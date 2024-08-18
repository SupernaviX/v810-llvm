#include "V810Subtarget.h"
#include "GISel/V810CallLowering.h"
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
  EnableGPRelativeRAM = false;
  EnableAppRegisters = false;

  StringRef CPUName = getCPUName(TT, CPU);
  ParseSubtargetFeatures(CPUName, /*TuneCPU*/ CPUName, FS);

  return *this;
}

V810Subtarget::V810Subtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
    : V810GenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      InstrInfo(), TLInfo(TM, initializeSubtargetDependencies(TT, CPU, FS)), FrameLowering(),
      InstrItins(getInstrItineraryForCPU(getCPUName(TT, CPU))) {
  CallLoweringInfo.reset(new V810CallLowering(*getTargetLowering()));
}

const CallLowering *V810Subtarget::getCallLowering() const {
  return CallLoweringInfo.get();
}