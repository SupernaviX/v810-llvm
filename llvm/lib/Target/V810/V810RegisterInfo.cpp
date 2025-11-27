#include "V810RegisterInfo.h"
#include "V810FrameLowering.h"
#include "V810Subtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "V810GenRegisterInfo.inc"

V810RegisterInfo::V810RegisterInfo() : V810GenRegisterInfo(V810::R31) {}

const MCPhysReg*
V810RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  if (MF->getFunction().hasFnAttribute("interrupt")) {
    // An interrupt function needs to preserve essentially every register
    return CSR_Interrupt_SaveList;
  }
  return CSR_SaveList;
}

const uint32_t *
V810RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const {
  return CSR_RegMask;
}

const uint32_t *
V810RegisterInfo::getNoPreservedMask() const {
  return CSR_NoRegs_RegMask;
}

BitVector
V810RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const V810Subtarget &Subtarget = MF.getSubtarget<V810Subtarget>();
  bool reserveAppRegs = !Subtarget.enableAppRegisters();

  Reserved.set(V810::R0); // zero register
  if (reserveAppRegs) {
    Reserved.set(V810::R1); // useful for ASM things
  }
  if (reserveAppRegs || getFrameLowering(MF)->hasFP(MF)) {
    Reserved.set(V810::R2); // frame pointer
  }
  Reserved.set(V810::R3); // stack pointer
  if (reserveAppRegs || Subtarget.enableGPRelativeRAM()) {
    Reserved.set(V810::R4); // global pointer
  }
  if (reserveAppRegs) {
    Reserved.set(V810::R5); // text pointer
  }
  Reserved.set(V810::R31); // return address
  return Reserved;
}

bool
V810RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS) const {
  MachineInstr &MI = *II;
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const V810FrameLowering *TFI = getFrameLowering(MF);
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

  Register FrameReg;
  int Offset = MI.getFlag(MachineInstr::FrameSetup)
    ? TFI->getFrameIndexReferencePreferSP(MF, FrameIndex, FrameReg, true).getFixed()
    : TFI->getFrameIndexReference(MF, FrameIndex, FrameReg).getFixed();
  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  if (MI.getOpcode() == V810::MOVEA && Offset == 0) {
    assert(FIOperandNum == 1);
    DebugLoc dl;
    BuildMI(MBB, II, dl, TII->get(V810::MOVr), MI.getOperand(0).getReg())
      .addReg(FrameReg);
    II->removeFromParent();
    return true;
  } else {
    MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false);
    MI.getOperand(FIOperandNum + 1).setImm(Offset);
    return false;
  }
}

Register
V810RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return V810::R3;
}



