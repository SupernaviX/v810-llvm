#include "V810InstructionSelector.h"
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

  ComplexRendererFns selectAddrRI(MachineOperand &Root) const;
private:
  const V810Subtarget &STI;
  const V810InstrInfo &TII;
  const V810RegisterInfo &TRI;
  const V810RegisterBankInfo &RBI;

  bool selectZext(MachineInstr &MI) const;
  bool selectIcmp(MachineInstr &MI) const;

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
    : STI(STI), TII(*STI.getInstrInfo()), TRI(*STI.getRegisterInfo()), RBI(RBI),
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
  MI.removeFromParent();
  return true;
}

static V810CC::CondCodes PredicateToCC(CmpInst::Predicate Pred) {
  switch (Pred) {
  default: llvm_unreachable("Unknown integer condition code!");
  case CmpInst::Predicate::ICMP_EQ:   return V810CC::CC_E;
  case CmpInst::Predicate::ICMP_NE:   return V810CC::CC_NE;
  case CmpInst::Predicate::ICMP_UGT:  return V810CC::CC_H;
  case CmpInst::Predicate::ICMP_UGE:  return V810CC::CC_NC;
  case CmpInst::Predicate::ICMP_ULT:  return V810CC::CC_C;
  case CmpInst::Predicate::ICMP_ULE:  return V810CC::CC_NH;
  case CmpInst::Predicate::ICMP_SGT:  return V810CC::CC_GT;
  case CmpInst::Predicate::ICMP_SGE:  return V810CC::CC_GE;
  case CmpInst::Predicate::ICMP_SLT:  return V810CC::CC_LT;
  case CmpInst::Predicate::ICMP_SLE:  return V810CC::CC_LE;
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

  switch (MI.getOpcode()) {
    default: break;
    case TargetOpcode::G_ZEXT:
    case TargetOpcode::G_ANYEXT:
      return selectZext(MI);
    case TargetOpcode::G_ICMP:
      return selectIcmp(MI);
  }
  return selectImpl(MI, *CoverageInfo);
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
