#ifndef LLVM_LIB_TARGET_V810_V810INSTRUCTIONSELECTOR_H
#define LLVM_LIB_TARGET_V810_V810INSTRUCTIONSELECTOR_H

#include "V810RegisterBankInfo.h"
#include "V810Subtarget.h"
#include "V810TargetMachine.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"

namespace llvm {

InstructionSelector *
createV810InstructionSelector(const V810TargetMachine &TM,
                                    V810Subtarget &STI,
                                    V810RegisterBankInfo &RBI);

}

#endif