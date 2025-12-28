#include "V810Subtarget.h"
#include "V810.h"

using namespace llvm;

#define DEBUG_TYPE "v810-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "V810GenSubtargetInfo.inc"

static StringRef getCPUName(const Triple &TT, StringRef CPU) {
  if (CPU.empty() && TT.getOSAndEnvironmentName() == "vb")
    return "vb";
  return CPU;
}

V810Subtarget &V810Subtarget::initializeSubtargetDependencies(const Triple &TT,
                                                              StringRef CPU,
                                                              StringRef FS) {
  IsNintendo = false;
  IsV830 = false;
  EnableGPRelativeRAM = false;
  EnableAppRegisters = false;

  StringRef CPUName = getCPUName(TT, CPU);
  ParseSubtargetFeatures(CPUName, /*TuneCPU*/ CPUName, FS);

  if (!IsV830 && TT.isV830()) {
    FeatureBitset Features = getFeatureBits();
    setFeatureBits(Features.set(V810::FeatureV830));
    IsV830 = true;
  }

  return *this;
}

V810Subtarget::V810Subtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
    : V810GenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      InstrInfo(*this), TLInfo(TM, initializeSubtargetDependencies(TT, CPU, FS)), FrameLowering(),
      InstrItins(getInstrItineraryForCPU(getCPUName(TT, CPU))) {}