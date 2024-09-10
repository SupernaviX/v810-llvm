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

  getActionDefinitionsBuilder({G_CONSTANT, G_FCONSTANT})
    .legalFor({p0, s1, s8, s16, s32, s64})
    .widenScalarToNextPow2(0);

  getActionDefinitionsBuilder({G_ADD, G_SUB, G_AND, G_OR, G_XOR})
    .legalFor({p0, s32})
    .customFor({s64})
    .minScalar(0, s32)
    .widenScalarToNextPow2(0);

  getActionDefinitionsBuilder({G_SHL, G_LSHR, G_ASHR})
    .legalFor({{p0, s32}, {s32, s32}})
    .customFor({s64, s32})
    .minScalar(0, s32)
    .widenScalarToNextPow2(0)
    .clampScalar(1, s32, s32);

  getActionDefinitionsBuilder({G_MUL, G_UMULH, G_SMULH})
    .legalFor({s32})
    .widenScalarToNextPow2(0)
    .clampScalar(0, s32, s32);

  // No need to support wider types, ExpandLargeDivRem will take care of that
  getActionDefinitionsBuilder({G_SDIV, G_SREM, G_SDIVREM, G_UDIV, G_UREM, G_UDIVREM})
    .legalFor({s32})
    .widenScalarToNextPow2(0)
    .minScalar(0, s32);

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

  getActionDefinitionsBuilder({G_LOAD, G_SEXTLOAD, G_ZEXTLOAD, G_STORE})
    .clampScalar(0, s32, s32)
    .legalForTypesWithMemDesc({
        {s32, p0, s8, 1},
        {s32, p0, s16, 2},
        {s32, p0, s32, 4},
        {p0, p0, s32, 4}
    })
    .lower();
  
  getActionDefinitionsBuilder({G_PTR_ADD, G_PTRMASK}).legalFor({{p0, s32}});
  getActionDefinitionsBuilder(G_PTRTOINT).legalFor({{s32, p0}}).minScalar(0, s32);
  getActionDefinitionsBuilder(G_INTTOPTR).legalFor({{p0, s32}}).minScalar(1, s32);

  getActionDefinitionsBuilder(G_GLOBAL_VALUE).legalFor({p0});

  getLegacyLegalizerInfo().computeTables();
  verify(*STI.getInstrInfo());
}

static bool narrowToS32(LegalizerHelper &Helper, MachineRegisterInfo &MRI, MachineInstr &MI) {
  // We only try to legalize s64
  assert(MRI.getType(MI.getOperand(0).getReg()).getSizeInBits() > 32);

  LLT s32 = LLT::scalar(32);
  return Helper.narrowScalar(MI, 0, s32) != LegalizerHelper::UnableToLegalize;
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
  case G_AND:
  case G_OR:
  case G_XOR:
  case G_SHL:
  case G_LSHR:
  case G_ASHR:
    return narrowToS32(Helper, MRI, MI);
  }
}