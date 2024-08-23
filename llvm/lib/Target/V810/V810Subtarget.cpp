#include "V810Subtarget.h"
#include "GISel/V810CallLowering.h"
#include "GISel/V810InstructionSelector.h"
#include "GISel/V810LegalizerInfo.h"
#include "GISel/V810RegisterBankInfo.h"
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
                             const std::string &FS, const V810TargetMachine &TM)
    : V810GenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      InstrInfo(), TLInfo(TM, initializeSubtargetDependencies(TT, CPU, FS)), FrameLowering(),
      InstrItins(getInstrItineraryForCPU(getCPUName(TT, CPU))) {
  CallLoweringInfo.reset(new V810CallLowering(*getTargetLowering()));
  RegBankInfo.reset(new V810RegisterBankInfo());
  Legalizer.reset(new V810LegalizerInfo(*this));
  V810RegisterBankInfo *RBI = static_cast<V810RegisterBankInfo*>(RegBankInfo.get());
  InstSelector.reset(createV810InstructionSelector(TM, *this, *RBI));
}

const CallLowering *V810Subtarget::getCallLowering() const {
  return CallLoweringInfo.get();
}

InstructionSelector *V810Subtarget::getInstructionSelector() const {
  return InstSelector.get();
}

const LegalizerInfo *V810Subtarget::getLegalizerInfo() const {
  return Legalizer.get();
}

const RegisterBankInfo *V810Subtarget::getRegBankInfo() const {
  return RegBankInfo.get();
}
