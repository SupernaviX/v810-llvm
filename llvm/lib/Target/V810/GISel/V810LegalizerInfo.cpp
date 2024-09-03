#include "V810LegalizerInfo.h"
#include "V810Subtarget.h"

using namespace llvm;

V810LegalizerInfo::V810LegalizerInfo(const V810Subtarget &STI) {
  using namespace TargetOpcode;
  const LLT p0 = LLT::pointer(0, 32);
  const LLT s32 = LLT::scalar(32);
  
  getActionDefinitionsBuilder({G_ADD, G_SUB, G_MUL})
    .legalFor({p0, s32})
    .widenScalarToNextPow2(0)
    .clampScalar(0, s32, s32);

  getActionDefinitionsBuilder({G_UMULH, G_SMULH})
    .legalFor({s32, s32});
  
  getActionDefinitionsBuilder({G_UADDE, G_UADDO, G_USUBE, G_USUBO}).lower();

  getActionDefinitionsBuilder(G_ICMP)
    .legalForCartesianProduct({p0, s32}, {p0, s32})
    .widenScalarToNextPow2(0)
    .clampScalar(1, s32, s32)
    .clampScalar(0, s32, s32);

  getLegacyLegalizerInfo().computeTables();
  verify(*STI.getInstrInfo());
}