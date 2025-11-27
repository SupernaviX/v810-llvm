#include "TargetInfo/V810TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheV810Target() {
  static Target TheV810Target;
  return TheV810Target;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810TargetInfo() {
  RegisterTarget<Triple::v810, /*HasJIT=*/false> X(getTheV810Target(), "v810", "V810", "V810");
}