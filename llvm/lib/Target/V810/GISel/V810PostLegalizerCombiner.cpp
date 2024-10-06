#include "V810Subtarget.h"
#include "llvm/CodeGen/GlobalISel/CSEInfo.h"
#include "llvm/CodeGen/GlobalISel/Combiner.h"
#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/GISelKnownBits.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/TargetPassConfig.h"

#define GET_GICOMBINER_DEPS
#include "V810GenPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_DEPS

#define DEBUG_TYPE "v810-postlegalizer-combiner"

using namespace llvm;

namespace {

#define GET_GICOMBINER_TYPES
#include "V810GenPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_TYPES

class V810PostLegalizerCombinerImpl : public Combiner {
protected:
  // TODO: Make CombinerHelper methods const.
  mutable CombinerHelper Helper;
  const V810PostLegalizerCombinerImplRuleConfig &RuleConfig;
  const V810Subtarget &STI;

public:
  V810PostLegalizerCombinerImpl(
      MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
      GISelKnownBits &KB, GISelCSEInfo *CSEInfo,
      const V810PostLegalizerCombinerImplRuleConfig &RuleConfig,
      const V810Subtarget &STI, MachineDominatorTree *MDT,
      const LegalizerInfo *LI);

  static const char *getName() { return "V810PostLegalizerCombiner"; }

  bool tryCombineAll(MachineInstr &I) const override;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "V810GenPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "V810GenPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_IMPL

V810PostLegalizerCombinerImpl::V810PostLegalizerCombinerImpl(
    MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
    GISelKnownBits &KB, GISelCSEInfo *CSEInfo,
    const V810PostLegalizerCombinerImplRuleConfig &RuleConfig,
    const V810Subtarget &STI, MachineDominatorTree *MDT,
    const LegalizerInfo *LI)
    : Combiner(MF, CInfo, TPC, &KB, CSEInfo),
      Helper(Observer, B, /*IsPreLegalize*/ false, &KB, MDT, LI),
      RuleConfig(RuleConfig), STI(STI),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "V810GenPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

// Pass boilerplate
// ================

class V810PostLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  V810PostLegalizerCombiner();

  StringRef getPassName() const override { return "V810PostLegalizerCombiner"; }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  V810PostLegalizerCombinerImplRuleConfig RuleConfig;
};
} // end anonymous namespace

void V810PostLegalizerCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
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

V810PostLegalizerCombiner::V810PostLegalizerCombiner()
    : MachineFunctionPass(ID) {
  initializeV810PostLegalizerCombinerPass(*PassRegistry::getPassRegistry());

  if (!RuleConfig.parseCommandLineOption())
    report_fatal_error("Invalid rule identifier");
}

bool V810PostLegalizerCombiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasProperty(
          MachineFunctionProperties::Property::FailedISel))
    return false;
  assert(MF.getProperties().hasProperty(
             MachineFunctionProperties::Property::Legalized) &&
         "Expected a legalized function?");
  auto *TPC = &getAnalysis<TargetPassConfig>();
  const Function &F = MF.getFunction();
  bool EnableOpt =
      MF.getTarget().getOptLevel() != CodeGenOptLevel::None && !skipFunction(F);

  const V810Subtarget &ST = MF.getSubtarget<V810Subtarget>();
  const auto *LI = ST.getLegalizerInfo();

  GISelKnownBits *KB = &getAnalysis<GISelKnownBitsAnalysis>().get(MF);
  MachineDominatorTree *MDT =
      &getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();
  GISelCSEAnalysisWrapper &Wrapper =
      getAnalysis<GISelCSEAnalysisWrapperPass>().getCSEWrapper();
  auto *CSEInfo = &Wrapper.get(TPC->getCSEConfig());

  CombinerInfo CInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, EnableOpt, F.hasOptSize(),
                     F.hasMinSize());
  V810PostLegalizerCombinerImpl Impl(MF, CInfo, TPC, *KB, CSEInfo,
                                        RuleConfig, ST, MDT, LI);
  return Impl.combineMachineInstrs();
}

char V810PostLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(V810PostLegalizerCombiner, DEBUG_TYPE,
                      "Combine V810 machine instrs after legalization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelKnownBitsAnalysis)
INITIALIZE_PASS_DEPENDENCY(GISelCSEAnalysisWrapperPass)
INITIALIZE_PASS_END(V810PostLegalizerCombiner, DEBUG_TYPE,
                    "Combine V810 machine instrs after legalization", false,
                    false)

namespace llvm {
FunctionPass *createV810PostLegalizerCombiner() {
  return new V810PostLegalizerCombiner();
}
} // end namespace llvm
