#include "V810LegalizerInfo.h"
#include "V810Subtarget.h"

using namespace llvm;

V810LegalizerInfo::V810LegalizerInfo(const V810Subtarget &STI) {
  using namespace TargetOpcode;
  const LLT p0 = LLT::pointer(0, 32);
  const LLT s1 = LLT::scalar(1);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);
  const LLT s64 = LLT::scalar(64);

  getActionDefinitionsBuilder(G_MERGE_VALUES).legalFor({{s64, s32}});
  getActionDefinitionsBuilder(G_UNMERGE_VALUES).legalFor({{s32, s64}});

  getActionDefinitionsBuilder({G_ADD, G_SUB})
    .legalFor({p0, s32})
    .customFor({s64})
    .minScalar(0, s32)
    .widenScalarToNextPow2(0);

  getActionDefinitionsBuilder({G_MUL, G_UMULH, G_SMULH})
    .legalFor({s32, s64})
    .minScalar(0, s32)
    .widenScalarToNextPow2(0);

  getActionDefinitionsBuilder({G_UADDE, G_UADDO, G_USUBE, G_USUBO}).lower();

  getActionDefinitionsBuilder(G_ICMP)
    .legalForCartesianProduct({s1}, {s32, p0})
    .minScalar(1, s32)
    .widenScalarToNextPow2(1);

  getActionDefinitionsBuilder({G_ANYEXT, G_SEXT, G_ZEXT}).legalFor({
    {s64, s32}, {s64, s16}, {s64, s8}, {s64, s1},
    {s32, s16}, {s32, s8}, {s32, s1},
    {s16, s8}, {s16, s1},
    {s8, s1},
  });

  getActionDefinitionsBuilder(G_TRUNC).legalFor({
    {s32, s64}, {s16, s64}, {s8, s64}, {s1, s64},
    {s16, s32}, {s8, s32}, {s1, s32},
    {s8, s16}, {s1, s16},
    {s1, s8},
  });

  getLegacyLegalizerInfo().computeTables();
  verify(*STI.getInstrInfo());
}

static bool legalizeAddSub(LegalizerHelper &Helper, MachineRegisterInfo &MRI, MachineInstr &MI) {
  // We only try to legalize s64
  assert(MRI.getType(MI.getOperand(0).getReg()).getSizeInBits() > 32);

  LLT s32 = LLT::scalar(32);
  return Helper.narrowScalarAddSub(MI, 0, s32) != LegalizerHelper::UnableToLegalize;
}

bool V810LegalizerInfo::legalizeCustom(LegalizerHelper &Helper, MachineInstr &MI,
                                       LostDebugLocObserver &LocObserver) const {
  using namespace TargetOpcode;

  MachineRegisterInfo &MRI = MI.getMF()->getRegInfo();
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("Invalid opcode for custom legalization");
  case G_ADD:
  case G_SUB:
    return legalizeAddSub(Helper, MRI, MI);
  }
}