#ifndef LLVM_LIB_TARGET_V810_V810_H
#define LLVM_LIB_TARGET_V810_V810_H

#include "MCTargetDesc/V810MCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class AsmPrinter;
class FunctionPass;
class MachineInstr;
class MCInst;
class PassRegistry;
class V810TargetMachine;

FunctionPass *createV810BranchSelectionPass();
FunctionPass *createV810ISelDag(V810TargetMachine &TM, CodeGenOptLevel OptLevel);

void LowerV810MachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                   AsmPrinter &AP);

void initializeV810BSelPass(PassRegistry &);
void initializeV810DAGToDAGISelLegacyPass(PassRegistry &);

FunctionPass *createV810PreLegalizerCombiner();
void initializeV810PreLegalizerCombinerPass(PassRegistry &);

FunctionPass *createV810O0PreLegalizerCombiner();
void initializeV810O0PreLegalizerCombinerPass(PassRegistry &);

FunctionPass *createV810PostLegalizerCombiner();
void initializeV810PostLegalizerCombinerPass(PassRegistry &);
}

namespace llvm {
  // Keep these in sync with V810InstrInfo.td
  namespace V810CC {
  enum CondCodes {
    CC_V = 0, // Overflow
    CC_C = 1, // Carry/Lower (unsigned)
    CC_E = 2, // Equal/Zero
    CC_NH = 3, // Not higher (unsigned)
    CC_N = 4, // Negative
    CC_BR = 5, // Always (unconditional branch)
    CC_LT = 6, // Less than (signed)
    CC_LE = 7, // Less than or equal (signed)
    CC_NV = 8, // Not overflow
    CC_NC = 9, // Not carry/lower (unsigned)
    CC_NE = 10, // Not equal/zero
    CC_H = 11, // Higher (unsigned)
    CC_P = 12, // Positive
    CC_NOP = 13, // Never (nop)
    CC_GE = 14, // Greater than or equal (signed)
    CC_GT = 15, // Greater than
  };
  }

  inline static V810CC::CondCodes InvertV810CondCode(V810CC::CondCodes CC) {
    return (V810CC::CondCodes)(CC ^ 0x8);
  }

  inline static const char *V810CondCodeToString(V810CC::CondCodes CC) {
    switch (CC) {
    case V810CC::CC_V:  return "v";
    case V810CC::CC_C:  return "c";
    case V810CC::CC_E:  return "e";
    case V810CC::CC_NH: return "nh";
    case V810CC::CC_N:  return "n";
    case V810CC::CC_BR:  return "t";
    case V810CC::CC_LT: return "lt";
    case V810CC::CC_LE: return "le";
    case V810CC::CC_NV: return "nv";
    case V810CC::CC_NC: return "nc";
    case V810CC::CC_NE: return "ne";
    case V810CC::CC_H:  return "h";
    case V810CC::CC_P:  return "p";
    case V810CC::CC_NOP: return "f";
    case V810CC::CC_GE: return "ge";
    case V810CC::CC_GT: return "gt";
    }
    llvm_unreachable("Invalid cond code");
  }

  // logic for HI and LO
  inline static int64_t EvalLo(int64_t Value) {
    return Value & 0xffff;
  }
  inline static int64_t EvalHi(int64_t Value) {
    // if LO is negative, increment HI to compensate
    return ((Value >> 16) & 0x0000ffff) + ((Value & 0x8000) == 0x8000);
  }

  // Just grab the high bits, without compensating for the low sign
  inline static int64_t EvalHi0(int64_t Value) {
    return (Value >> 16) & 0xffff;
  }
}

#endif