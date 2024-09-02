#include "V810InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
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

bool V810InstructionSelector::select(MachineInstr &MI) {
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

  return std::nullopt;
}

InstructionSelector *llvm::createV810InstructionSelector(
    const V810TargetMachine &TM, V810Subtarget &STI, V810RegisterBankInfo &RBI) {
  return new V810InstructionSelector(TM, STI, RBI);
}
