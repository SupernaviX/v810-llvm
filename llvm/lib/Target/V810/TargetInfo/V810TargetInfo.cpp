#include "TargetInfo/V810TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheV810Target() {
  static Target TheV810Target;
  return TheV810Target;
}

Target &llvm::getTheV830Target() {
  static Target TheV830Target;
  return TheV830Target;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810TargetInfo() {
  RegisterTarget<Triple::v810, /*HasJIT=*/false> X(getTheV810Target(), "v810", "V810", "V810");
  RegisterTarget<Triple::v830, /*HasJIT=*/false> Y(getTheV830Target(), "v830", "V830", "V810");
}