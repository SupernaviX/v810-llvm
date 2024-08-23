#include "V810InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"

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

InstructionSelector *llvm::createV810InstructionSelector(
    const V810TargetMachine &TM, V810Subtarget &STI, V810RegisterBankInfo &RBI) {
  return new V810InstructionSelector(TM, STI, RBI);
}
