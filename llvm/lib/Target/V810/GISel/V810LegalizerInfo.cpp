#include "V810LegalizerInfo.h"
#include "V810Subtarget.h"

using namespace llvm;

V810LegalizerInfo::V810LegalizerInfo(const V810Subtarget &STI) {
  getLegacyLegalizerInfo().computeTables();
  verify(*STI.getInstrInfo());
}