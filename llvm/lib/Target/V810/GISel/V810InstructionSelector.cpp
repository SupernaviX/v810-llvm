#include "V810InstructionSelector.h"
#include "V810TargetObjectFile.h"
#include "MCTargetDesc/V810MCExpr.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/GISelKnownBits.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/GlobalISel/MIPatternMatch.h"

using namespace llvm;

#define DEBUG_TYPE "v810-isel"

namespace {
  
#define GET_GLOBALISEL_PREDICATE_BITSET
#include "V810GenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATE_BITSET

class V810InstructionSelector : public InstructionSelector {
public:
  V810InstructionSelector(const V810TargetMachine &TM,
                          V810Subtarget &STI,
                          V810RegisterBankInfo &RBI);
  bool select(MachineInstr &MI) override;
  static const char *getName() { return DEBUG_TYPE; }

  ComplexRendererFns selectGlobalRI(MachineInstr *Global, int64_t Offset) const;
  ComplexRendererFns selectAddrRI(MachineOperand &Root) const;
private:
  const V810TargetMachine &TM;
  const V810Subtarget &STI;
  const V810InstrInfo &TII;
  const V810RegisterInfo &TRI;
  const V810RegisterBankInfo &RBI;

  bool IsGPRelative(const GlobalValue *GVal) const;

  bool selectZext(MachineInstr &MI) const;
  bool selectSext(MachineInstr &MI) const;
  bool selectTrunc(MachineInstr &MI) const;
  // Given a register which contains a boolean "condition" value,
  // generates a CMP instruction which set CC flags appropriately,
  // and returns the predicate which a Bcond/Setf should use.
  // This will "fold" as many redundant operations as it can into the CMP.
  // The CMP may be removed later by optimizeCompareInstr, if it turns out to be unnecessary.
  bool buildCmpForCond(MachineIRBuilder &MIB, Register Cond, CmpInst::Predicate &Pred) const;
  bool selectIcmp(MachineInstr &MI) const;
  bool selectBrcond(MachineInstr &MI) const;
  bool selectGlobalValue(MachineInstr &MI, int64_t Offset = 0) const;
  bool selectPtrAdd(MachineInstr &MI) const;

  template <int Bits>
  bool matchImm(Register Reg, int64_t &Value) const;

  /// tblgen-erated 'select' implementation, used as the initial selector for
  /// the patterns that don't require complex C++.
  bool selectImpl(MachineInstr &MI, CodeGenCoverage &CoverageInfo) const;
  
#define GET_GLOBALISEL_PREDICATES_DECL
#include "V810GenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_DECL

#define GET_GLOBALISEL_TEMPORARIES_DECL
#include "V810GenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_DECL
};

} // namespace

#define GET_GLOBALISEL_IMPL
#include "V810GenGlobalISel.inc"
#undef GET_GLOBALISEL_IMPL

V810InstructionSelector::V810InstructionSelector(const V810TargetMachine &TM,
                                                       V810Subtarget &STI,
                                                       V810RegisterBankInfo &RBI)
    : TM(TM), STI(STI), TII(*STI.getInstrInfo()), TRI(*STI.getRegisterInfo()), RBI(RBI),
#define GET_GLOBALISEL_PREDICATES_INIT
#include "V810GenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_INIT
#define GET_GLOBALISEL_TEMPORARIES_INIT
#include "V810GenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_INIT
{
}

bool V810InstructionSelector::selectZext(MachineInstr &MI) const {
  // TODO: need u64 support?
  const Register Dst = MI.getOperand(0).getReg();
  const Register Src = MI.getOperand(1).getReg();
  auto &MRI = MF->getRegInfo();
  MRI.replaceRegWith(Dst, Src);
  MI.eraseFromParent();
  return true;
}

bool V810InstructionSelector::selectSext(MachineInstr &MI) const {
  const Register Dst = MI.getOperand(0).getReg();
  const Register Src = MI.getOperand(1).getReg();
  auto &MRI = MF->getRegInfo();
  const LLT DstType = MRI.getType(Dst);
  const LLT SrcType = MRI.getType(Src);
  MachineIRBuilder MIB(MI);

  const int64_t ShiftAmount = DstType.getScalarSizeInBits() - SrcType.getScalarSizeInBits();

  const Register Tmp = MRI.createVirtualRegister(&V810::GenRegsRegClass);
  MRI.setType(Tmp, SrcType);
  MIB.buildInstr(V810::SHLi)
    .addDef(Tmp)
    .addUse(Src)
    .addImm(ShiftAmount);
  MRI.setType(Tmp, DstType);
  MIB.buildInstr(V810::SARi)
    .addDef(Dst)
    .addUse(Tmp)
    .addImm(ShiftAmount);
  MRI.setRegClass(Dst, &V810::GenRegsRegClass);

  MI.eraseFromParent();
  return true;
}

bool V810InstructionSelector::selectTrunc(MachineInstr &MI) const {
  const Register Dst = MI.getOperand(0).getReg();
  const Register Src = MI.getOperand(1).getReg();
  auto &MRI = MF->getRegInfo();
  const LLT DstType = MRI.getType(Dst);
  const LLT SrcType = MRI.getType(Src);

  MachineIRBuilder MIB(MI);

  int64_t MaskValue = (1 << DstType.getScalarSizeInBits()) - 1;

  MRI.setType(Dst, SrcType);
  MIB.buildInstr(V810::ANDI)
    .addDef(Dst)
    .addUse(Src)
    .addImm(MaskValue);
  MRI.setType(Dst, DstType);
  MRI.setRegClass(Dst, &V810::GenRegsRegClass);

  MI.eraseFromParent();
  return true;
}

static V810CC::CondCodes PredicateToCC(CmpInst::Predicate Pred) {
  // Treat ordered and unordered identically on the VB, because we only have the one set of cond codes.
  // NaNs cause hardware exceptions, so it shouldn't matter anyway. 
  switch (Pred) {
  default: llvm_unreachable("Unknown condition code!");
  case CmpInst::Predicate::FCMP_FALSE:
    return V810CC::CC_NOP;
  case CmpInst::Predicate::FCMP_OEQ:
  case CmpInst::Predicate::FCMP_UEQ:
  case CmpInst::Predicate::ICMP_EQ:
    return V810CC::CC_E;
  case CmpInst::Predicate::FCMP_ONE:
  case CmpInst::Predicate::FCMP_UNE:
  case CmpInst::Predicate::ICMP_NE:
    return V810CC::CC_NE;
  case CmpInst::Predicate::ICMP_UGT:
    return V810CC::CC_H;
  case CmpInst::Predicate::ICMP_UGE:
    return V810CC::CC_NC;
  case CmpInst::Predicate::ICMP_ULT:
    return V810CC::CC_C;
  case CmpInst::Predicate::ICMP_ULE:
    return V810CC::CC_NH;
  case CmpInst::Predicate::FCMP_OGT:
  case CmpInst::Predicate::FCMP_UGT:
  case CmpInst::Predicate::ICMP_SGT:
    return V810CC::CC_GT;
  case CmpInst::Predicate::FCMP_OGE:
  case CmpInst::Predicate::FCMP_UGE:
  case CmpInst::Predicate::ICMP_SGE:
    return V810CC::CC_GE;
  case CmpInst::Predicate::FCMP_OLT:
  case CmpInst::Predicate::FCMP_ULT:
  case CmpInst::Predicate::ICMP_SLT:
    return V810CC::CC_LT;
  case CmpInst::Predicate::FCMP_OLE:
  case CmpInst::Predicate::FCMP_ULE:
  case CmpInst::Predicate::ICMP_SLE:
    return V810CC::CC_LE;
  case CmpInst::Predicate::FCMP_TRUE:
    return V810CC::CC_BR;
  }
}

template <int Bits>
bool V810InstructionSelector::matchImm(Register Reg, int64_t &Value) const {
  using namespace llvm::MIPatternMatch;

  MachineRegisterInfo &MRI = MF->getRegInfo();
  if (mi_match(Reg, MRI, m_ICst(Value))) {
    return isInt<Bits>(Value);
  }
  return false;
}

static bool buildCmpNotZero(MachineIRBuilder &MIB, Register Cond, CmpInst::Predicate &Pred) {
  MIB.buildInstr(V810::CMPri, {}, {Cond, (int64_t) 0});
  Pred = CmpInst::Predicate::ICMP_NE;
  return true;
}

bool V810InstructionSelector::buildCmpForCond(MachineIRBuilder &MIB, Register Cond, CmpInst::Predicate &Pred) const {
  using namespace llvm::MIPatternMatch;

  MachineRegisterInfo &MRI = MF->getRegInfo();
  if (!MRI.hasOneUse(Cond)) {
    // If the condition variable is used in more than one place, we can't fold it into a CMP.
    return buildCmpNotZero(MIB, Cond, Pred);
  }

  MachineInstr *MI = MRI.getVRegDef(Cond);

  Register LHS;
  Register RHS;
  if (mi_match(Cond, MRI, m_GICmp(m_Pred(Pred), m_Reg(LHS), m_Reg(RHS)))) {
    // If we are straight-up comparing two values, generate a CMP and be done with it.
    int64_t Imm;
    if (matchImm<5>(RHS, Imm)) {
      MIB.buildInstr(V810::CMPri, {}, {LHS, Imm});
    } else if (matchImm<5>(LHS, Imm)) {
      Pred = CmpInst::getSwappedPredicate(Pred);
      MIB.buildInstr(V810::CMPri, {}, {RHS, Imm});
    } else {
      MIB.buildInstr(V810::CMPrr, {}, {LHS, RHS});
    }
    MI->eraseFromParent();
    return true;
  } else if (mi_match(Cond, MRI,
              m_any_of(
                m_GZExt(m_Reg(LHS)),
                m_GSExt(m_Reg(LHS)),
                m_GAnyExt(m_Reg(LHS)),
                m_GFPExt(m_Reg(LHS)),
                m_GTrunc(m_Reg(LHS)),
                m_GFPTrunc(m_Reg(LHS))))) {
    // ignore truncations and extensions, they don't affect the boolean value
    if (!buildCmpForCond(MIB, LHS, Pred))
      return false;
    MI->eraseFromParent();
    return true;
  } else if (mi_match(Cond, MRI, m_GAnd(m_Reg(LHS), m_Reg(RHS)))) {
    KnownBits LowBit = KB->getKnownBits(RHS).trunc(1);
    if (LowBit.isZero()) {
      // If we clear the low bit, this is an "always false" condition
      MI->eraseFromParent();
      Pred = CmpInst::Predicate::FCMP_FALSE;
      return true;
    } else if (LowBit.isAllOnes()) {
      // If we don't clear the low bit, this is either a truncation or a no-op.
      // Either way, it doesn't affect the boolean value, so throw it out.
      if (!buildCmpForCond(MIB, LHS, Pred))
        return false;
      MI->eraseFromParent();
      return true;
    } else {
      // This is a logical AND. Don't try to optimize it out.
      return buildCmpNotZero(MIB, Cond, Pred);
    }
  } else if (mi_match(Cond, MRI, m_GOr(m_Reg(LHS), m_Reg(RHS)))){
    KnownBits LowBit = KB->getKnownBits(RHS).trunc(1);
    if (LowBit.isZero()) {
      // If we don't set the low bit, this is a no-op.
      if (!buildCmpForCond(MIB, LHS, Pred))
        return false;
      MI->eraseFromParent();
      return true;
    } else if (LowBit.isAllOnes()) {
      // If we set the low bit, this is an "always true" condition.
      MI->eraseFromParent();
      Pred = CmpInst::Predicate::FCMP_TRUE;
      return true;
    } else {
      // this is a logical OR. Don't try to optimize it out.
      return buildCmpNotZero(MIB, Cond, Pred);
    }
  } else if (mi_match(Cond, MRI, m_GXor(m_Reg(LHS), m_Reg(RHS)))) {
    KnownBits LowBit = KB->getKnownBits(RHS).trunc(1);
    if (LowBit.isZero()) {
      // If we don't toggle the low bit, this is a no-op.
      if (!buildCmpForCond(MIB, LHS, Pred))
        return false;
      MI->eraseFromParent();
      return true;
    } else if (LowBit.isAllOnes()) {
      // If we toggle the low bit, this is a NOT. Optimize it out, but invert the predicate.
      if (!buildCmpForCond(MIB, LHS, Pred))
        return false;
      Pred = CmpInst::getInversePredicate(Pred);
      MI->eraseFromParent();
      return true;
    } else {
      // this is a logical XOR. Don't try to optimize it out.
      return buildCmpNotZero(MIB, Cond, Pred);
    }
  } else {
    // This operand isn't a logical operation. We don't know what it is.
    KnownBits LowBit = KB->getKnownBits(Cond).trunc(1);
    if (LowBit.isZero()) {
      // Literal false
      Pred = CmpInst::Predicate::FCMP_FALSE;
      MI->eraseFromParent();
      return true;
    } else if (LowBit.isAllOnes()) {
      // Literal true
      Pred = CmpInst::Predicate::FCMP_TRUE;
      MI->eraseFromParent();
      return true;
    } else {
      // unknown value
      return buildCmpNotZero(MIB, Cond, Pred);
    }
  }
}

bool V810InstructionSelector::selectIcmp(MachineInstr &MI) const {
  const Register Dst = MI.getOperand(0).getReg();
  CmpInst::Predicate Pred = (CmpInst::Predicate)MI.getOperand(1).getPredicate();
  const Register LHS = MI.getOperand(2).getReg();
  const Register RHS = MI.getOperand(3).getReg();

  MachineIRBuilder MIB(MI);

  int64_t Imm;
  if (matchImm<5>(RHS, Imm)) {
    MIB.buildInstr(V810::CMPri, {}, {LHS, Imm});
  } else if (matchImm<5>(LHS, Imm)) {
    Pred = CmpInst::getSwappedPredicate(Pred);
    MIB.buildInstr(V810::CMPri, {}, {RHS, Imm});
  } else {
    MIB.buildInstr(V810::CMPrr, {}, {LHS, RHS});
  }

  V810CC::CondCodes CC = PredicateToCC(Pred);
  auto Setf = MIB.buildInstr(V810::SETF, {Dst}, {(int64_t) CC});
  MI.eraseFromParent();
  return constrainSelectedInstRegOperands(*Setf, TII, TRI, RBI);
}

bool V810InstructionSelector::selectBrcond(MachineInstr &MI) const {
  Register Cond = MI.getOperand(0).getReg();
  MachineBasicBlock *MBB = MI.getOperand(1).getMBB();

  MachineIRBuilder MIB(MI);
  CmpInst::Predicate Pred;
  if (!buildCmpForCond(MIB, Cond, Pred)) {
    return false;
  }

  V810CC::CondCodes CC = PredicateToCC(Pred);
  MIB.buildInstr(V810::Bcond).addImm(CC).addMBB(MBB);
  MI.eraseFromParent();
  return true;
}

bool V810InstructionSelector::IsGPRelative(const GlobalValue *GVal) const {
  if (!STI.enableGPRelativeRAM()) {
    return false;
  }
  const V810TargetObjectFile *TLOF =
      static_cast<const V810TargetObjectFile *>(
          TM.getObjFileLowering());
  return TLOF->isGlobalInSmallSection(GVal->getAliaseeObject(), TM);
}

bool V810InstructionSelector::selectGlobalValue(MachineInstr &MI, int64_t Offset) const {
  const Register Dst = MI.getOperand(0).getReg();
  const GlobalValue *GVal = MI.getOperand(1).getGlobal();
  MachineIRBuilder MIB(MI);
  MachineRegisterInfo &MRI = MF->getRegInfo();

  MachineInstr *NewMI;
  if (IsGPRelative(GVal)) {
    NewMI = MIB.buildInstr(V810::MOVEA)
      .addDef(Dst)
      .addUse(V810::R4)
      .addGlobalAddress(GVal, Offset, V810MCExpr::VK_V810_SDAOFF);
  } else {
    const Register HiReg = MRI.createVirtualRegister(&V810::GenRegsRegClass);
    MRI.setType(HiReg, MRI.getType(Dst));

    MIB.buildInstr(V810::MOVHI)
      .addDef(HiReg)
      .addUse(V810::R0)
      .addGlobalAddress(GVal, Offset, V810MCExpr::VK_V810_HI);
    NewMI = MIB.buildInstr(V810::MOVEA)
      .addDef(Dst)
      .addUse(HiReg)
      .addGlobalAddress(GVal, Offset, V810MCExpr::VK_V810_LO);
  }

  MI.eraseFromParent();
  return constrainSelectedInstRegOperands(*NewMI, TII, TRI, RBI);
}

bool V810InstructionSelector::selectPtrAdd(MachineInstr &MI) const {
  using namespace llvm::MIPatternMatch;
  MachineRegisterInfo &MRI = MF->getRegInfo();

  const Register Dst = MI.getOperand(0).getReg();
  const Register Ptr = MI.getOperand(1).getReg();
  const Register Offset = MI.getOperand(2).getReg();

  int64_t ConstOffset;
  if (!matchImm<16>(Offset, ConstOffset)) {
    MI.setDesc(TII.get(V810::ADDrr));
    return constrainSelectedInstRegOperands(MI, TII, TRI, RBI);
  }

  MachineInstr *PtrDef = MRI.getVRegDef(Ptr);
  if (PtrDef->getOpcode() == TargetOpcode::G_GLOBAL_VALUE) {
    if (!selectGlobalValue(*PtrDef, ConstOffset)) {
      return false;
    }
    MRI.replaceRegWith(Dst, Ptr);
    MI.eraseFromBundle();
    return true;
  }

  MachineIRBuilder MIB(MI);
  unsigned Opcode = isInt<5>(ConstOffset) ? V810::ADDri : V810::ADDI;
  auto NewMI = MIB.buildInstr(Opcode)
    .addDef(Dst)
    .addUse(Ptr)
    .addImm(ConstOffset);

  if (!NewMI.constrainAllUses(TII, TRI, RBI)) {
    return false;
  }
  MI.eraseFromBundle();
  return true;
}

bool V810InstructionSelector::select(MachineInstr &MI) {
  MachineRegisterInfo &MRI = MF->getRegInfo();

  if (!MI.isPreISelOpcode()) {
    for (MachineOperand &Op : MI.all_defs()) {
      if (Op.getReg().isPhysical() || MRI.getRegClassOrNull(Op.getReg())) {
        continue;
      }
      Register Reg = constrainOperandRegClass(*MF, TRI, MRI, TII, RBI, MI, V810::GenRegsRegClass, Op);
      Op.setReg(Reg);
    }
    return true;
  }

  if (selectImpl(MI, *CoverageInfo))
    return true;

  switch (MI.getOpcode()) {
    default:
      return false;
    case TargetOpcode::G_ZEXT:
    case TargetOpcode::G_ANYEXT:
      return selectZext(MI);
    case TargetOpcode::G_SEXT:
      return selectSext(MI);
    case TargetOpcode::G_TRUNC:
      return selectTrunc(MI);
    case TargetOpcode::G_ICMP:
      return selectIcmp(MI);
    case TargetOpcode::G_BRCOND:
      return selectBrcond(MI);
    case TargetOpcode::G_GLOBAL_VALUE:
      return selectGlobalValue(MI);
    case TargetOpcode::G_PTR_ADD:
      return selectPtrAdd(MI);
  }
}

InstructionSelector::ComplexRendererFns
V810InstructionSelector::selectGlobalRI(MachineInstr *Global, int64_t Offset) const {
  assert(Global->getOpcode() == TargetOpcode::G_GLOBAL_VALUE);

  const Register Dst = Global->getOperand(0).getReg();
  const GlobalValue *GVal = Global->getOperand(1).getGlobal();
  if (IsGPRelative(GVal)) {
    return {{
      [=](MachineInstrBuilder &MIB) { MIB.addReg(V810::R4); },
      [=](MachineInstrBuilder &MIB) { MIB.addGlobalAddress(GVal, Offset, V810MCExpr::VK_V810_SDAOFF); },
    }};
  }

  MachineRegisterInfo &MRI = MF->getRegInfo();
  const Register HiReg = MRI.createVirtualRegister(&V810::GenRegsRegClass);
  MRI.setType(HiReg, MRI.getType(Dst));
  const Register LoReg = MRI.createVirtualRegister(&V810::GenRegsRegClass);
  MRI.setType(LoReg, MRI.getType(Dst));

  return {{
    [=](MachineInstrBuilder &MIB) {
      MachineIRBuilder MIRB(*MIB.getInstr());
      MIRB.buildInstr(V810::MOVHI)
        .addDef(HiReg)
        .addUse(V810::R0)
        .addGlobalAddress(GVal, 0, V810MCExpr::VK_V810_HI);
      MIRB.buildInstr(V810::MOVEA)
        .addDef(LoReg)
        .addUse(HiReg)
        .addGlobalAddress(GVal, 0, V810MCExpr::VK_V810_LO);
      MIB.addReg(LoReg);
    },
    [=](MachineInstrBuilder &MIB) { MIB.addImm(Offset); }
  }};
}

InstructionSelector::ComplexRendererFns
V810InstructionSelector::selectAddrRI(MachineOperand &Root) const {
  using namespace llvm::MIPatternMatch;

  MachineFunction &MF = *Root.getParent()->getParent()->getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  if (!Root.isReg())
    return std::nullopt;

  MachineInstr *RootI = MRI.getVRegDef(Root.getReg());

  if (RootI->getOpcode() == TargetOpcode::G_FRAME_INDEX) {
    return {{
      [=](MachineInstrBuilder &MIB) { MIB.add(RootI->getOperand(1)); },
      [=](MachineInstrBuilder &MIB) { MIB.addImm(0); },
    }};
  }

  if (RootI->getOpcode() == TargetOpcode::G_GLOBAL_VALUE) {
    return selectGlobalRI(RootI, 0);
  }

  Register BaseReg;
  APInt Offset;
  if (mi_match(RootI, MRI, m_GPtrAdd(m_Reg(BaseReg), m_ICst(Offset)))
      && Offset.isSignedIntN(16)) {
    MachineInstr *LHS = MRI.getVRegDef(BaseReg);
    if (LHS->getOpcode() == TargetOpcode::G_FRAME_INDEX) {
      return {{
        [=](MachineInstrBuilder &MIB) { MIB.add(LHS->getOperand(1)); },
        [=](MachineInstrBuilder &MIB) { MIB.addImm(Offset.getSExtValue()); }
      }};
    }

    if (LHS->getOpcode() == TargetOpcode::G_GLOBAL_VALUE) {
      return selectGlobalRI(LHS, Offset.getSExtValue());
    }

    return {{
      [=](MachineInstrBuilder &MIB) { MIB.addReg(BaseReg); },
      [=](MachineInstrBuilder &MIB) { MIB.addImm(Offset.getSExtValue()); }
    }};
  }

  APInt ConstAddress;
  if (mi_match(Root.getReg(), MRI, m_ICst(ConstAddress))) {
    assert(ConstAddress.isSignedIntN(32));
    // When working with constant addresses, MOVHI the high bits and stick the low bits in the offset
    uint64_t lo = EvalLo(ConstAddress.getSExtValue());
    uint64_t hi = EvalHi(ConstAddress.getSExtValue());
    if (hi == 0) {
      return {{
        [=](MachineInstrBuilder &MIB) { MIB.addReg(V810::R0); },
        [=](MachineInstrBuilder &MIB) { MIB.addImm(lo); }
      }};
    }
    Register HiReg = MRI.createVirtualRegister(&V810::GenRegsRegClass);
    return {{
      [=](MachineInstrBuilder &MIB) {
        MachineIRBuilder(*MIB.getInstr()).buildInstr(V810::MOVHI, {HiReg}, {hi});
        MIB.addReg(HiReg);
      },
      [=](MachineInstrBuilder &MIB) { MIB.addImm(lo); }
    }};
  }

  return {{
    [=](MachineInstrBuilder &MIB) { MIB.add(Root); },
    [=](MachineInstrBuilder &MIB) { MIB.addImm(0); },
  }};
}

InstructionSelector *llvm::createV810InstructionSelector(
    const V810TargetMachine &TM, V810Subtarget &STI, V810RegisterBankInfo &RBI) {
  return new V810InstructionSelector(TM, STI, RBI);
}
