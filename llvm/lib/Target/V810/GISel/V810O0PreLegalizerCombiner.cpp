#include "V810Subtarget.h"
#include "llvm/CodeGen/GlobalISel/CSEInfo.h"
#include "llvm/CodeGen/GlobalISel/Combiner.h"
#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/GISelKnownBits.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/TargetPassConfig.h"

#define GET_GICOMBINER_DEPS
#include "V810GenO0PreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_DEPS

#define DEBUG_TYPE "v810-O0-prelegalizer-combiner"

using namespace llvm;

namespace {

#define GET_GICOMBINER_TYPES
#include "V810GenO0PreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_TYPES

class V810O0PreLegalizerCombinerImpl : public Combiner {
protected:
  // TODO: Make CombinerHelper methods const.
  mutable CombinerHelper Helper;
  const V810O0PreLegalizerCombinerImplRuleConfig &RuleConfig;
  const V810Subtarget &STI;

public:
  V810O0PreLegalizerCombinerImpl(
      MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
      GISelKnownBits &KB, GISelCSEInfo *CSEInfo,
      const V810O0PreLegalizerCombinerImplRuleConfig &RuleConfig,
      const V810Subtarget &STI, MachineDominatorTree *MDT,
      const LegalizerInfo *LI);

  static const char *getName() { return "V810O0PreLegalizerCombiner"; }

  bool tryCombineAll(MachineInstr &I) const override;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "V810GenO0PreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "V810GenO0PreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_IMPL

V810O0PreLegalizerCombinerImpl::V810O0PreLegalizerCombinerImpl(
    MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
    GISelKnownBits &KB, GISelCSEInfo *CSEInfo,
    const V810O0PreLegalizerCombinerImplRuleConfig &RuleConfig,
    const V810Subtarget &STI, MachineDominatorTree *MDT,
    const LegalizerInfo *LI)
    : Combiner(MF, CInfo, TPC, &KB, CSEInfo),
      Helper(Observer, B, /*IsPreLegalize*/ true, &KB, MDT, LI),
      RuleConfig(RuleConfig), STI(STI),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "V810GenO0PreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

// Pass boilerplate
// ================

class V810O0PreLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  V810O0PreLegalizerCombiner();

  StringRef getPassName() const override { return "V810O0PreLegalizerCombiner"; }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  V810O0PreLegalizerCombinerImplRuleConfig RuleConfig;
};
} // end anonymous namespace

void V810O0PreLegalizerCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetPassConfig>();
  AU.setPreservesCFG();
  getSelectionDAGFallbackAnalysisUsage(AU);
  AU.addRequired<GISelKnownBitsAnalysis>();
  AU.addPreserved<GISelKnownBitsAnalysis>();
  AU.addRequired<MachineDominatorTreeWrapperPass>();
  AU.addPreserved<MachineDominatorTreeWrapperPass>();
  AU.addRequired<GISelCSEAnalysisWrapperPass>();
  AU.addPreserved<GISelCSEAnalysisWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

V810O0PreLegalizerCombiner::V810O0PreLegalizerCombiner()
    : MachineFunctionPass(ID) {
  initializeV810O0PreLegalizerCombinerPass(*PassRegistry::getPassRegistry());

  if (!RuleConfig.parseCommandLineOption())
    report_fatal_error("Invalid rule identifier");
}

bool V810O0PreLegalizerCombiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasProperty(
          MachineFunctionProperties::Property::FailedISel))
    return false;
  auto &TPC = getAnalysis<TargetPassConfig>();

  // Enable CSE.
  GISelCSEAnalysisWrapper &Wrapper =
      getAnalysis<GISelCSEAnalysisWrapperPass>().getCSEWrapper();
  auto *CSEInfo = &Wrapper.get(TPC.getCSEConfig());

  const V810Subtarget &ST = MF.getSubtarget<V810Subtarget>();
  const auto *LI = ST.getLegalizerInfo();

  const Function &F = MF.getFunction();
  bool EnableOpt =
      MF.getTarget().getOptLevel() != CodeGenOptLevel::None && !skipFunction(F);
  GISelKnownBits *KB = &getAnalysis<GISelKnownBitsAnalysis>().get(MF);
  MachineDominatorTree *MDT =
      &getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();
  CombinerInfo CInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, EnableOpt, F.hasOptSize(),
                     F.hasMinSize());
  V810O0PreLegalizerCombinerImpl Impl(MF, CInfo, &TPC, *KB, CSEInfo, RuleConfig,
                                     ST, MDT, LI);
  return Impl.combineMachineInstrs();
}

char V810O0PreLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(V810O0PreLegalizerCombiner, DEBUG_TYPE,
                      "Combine V810 machine instrs before legalization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelKnownBitsAnalysis)
INITIALIZE_PASS_DEPENDENCY(GISelCSEAnalysisWrapperPass)
INITIALIZE_PASS_END(V810O0PreLegalizerCombiner, DEBUG_TYPE,
                    "Combine V810 machine instrs before legalization", false,
                    false)

namespace llvm {
FunctionPass *createV810O0PreLegalizerCombiner() {
  return new V810O0PreLegalizerCombiner();
}
} // end namespace llvm
