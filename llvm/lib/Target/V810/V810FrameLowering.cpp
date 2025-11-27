#include "V810.h"
#include "V810FrameLowering.h"
#include "V810MachineFunctionInfo.h"
#include "V810RegisterInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

V810FrameLowering::V810FrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), 0, Align(4), true) {}

static bool isFPSave(MachineBasicBlock::iterator I) {
  return I->getOpcode() == V810::ST_W
    && I->getOperand(0).isFI()
    && I->getOperand(2).isReg()
    && I->getOperand(2).getReg() == V810::R2;
}

static bool isFPLoad(MachineBasicBlock::iterator I) {
  return I->getOpcode() == V810::LD_W
    && I->getOperand(1).isFI()
    && I->getOperand(0).isReg()
    && I->getOperand(0).getReg() == V810::R2;
}

void
V810FrameLowering::emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const {
  int bytes = (int) MF.getFrameInfo().getStackSize();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  moveStackPointer(MF, MBB, MBBI, -bytes);

  DebugLoc dl;
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  if (hasFP(MF)) {
    // Find the instruction where we store FP...
    while (MBBI != MBB.getFirstTerminator() && !isFPSave(MBBI)) {
      ++MBBI;
      if (MBBI == MBB.getFirstTerminator()) {
        // If we don't store FP, don't worry about it.
        return;
      }
    }
    // Grab the frame index where we stored it...
    int FPIndex = MBBI->getOperand(0).getIndex();

    // and set FP to the address of that frame index.
    ++MBBI;

    BuildMI(MBB, MBBI, dl, TII.get(V810::MOVEA), V810::R2)
      .addFrameIndex(FPIndex).addImm(0).setMIFlag(MachineInstr::FrameSetup);
  }

  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();
  if (RegInfo->shouldRealignStack(MF) && MF.getFrameInfo().getMaxAlign() > Align(4)) {
    // align the dang stack
    // this will clobber r31, but we already saved it so that's fine
    int BitsToClear = MF.getFrameInfo().getMaxAlign().value() - 1;

    BuildMI(MBB, MBBI, dl, TII.get(V810::ANDI), V810::R31)
      .addReg(V810::R3).addImm(BitsToClear);
    BuildMI(MBB, MBBI, dl, TII.get(V810::SUB), V810::R3)
      .addReg(V810::R3).addReg(V810::R31, RegState::Kill);
  }
}

void
V810FrameLowering::emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  if (!TRI->shouldRealignStack(MF) && !MF.getFrameInfo().hasVarSizedObjects()) {
    // If we know the exact size of the stack, just increase the SP by that amount
    int bytes = (int) MF.getFrameInfo().getStackSize();
    moveStackPointer(MF, MBB, MBBI, bytes);
    return;
  }

  // We don't know the size of the stack, so we need to use special logic to restore fp (r2) and sp (r3)
  DebugLoc dl;
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  // Find the existing "restore fp" instruction and delete it
  while (!isFPLoad(MBBI)) --MBBI;
  MBBI->eraseFromParent();

  // Restore r3 by just adding the right offset to r2
  int FPOffset = MF.getInfo<V810MachineFunctionInfo>()->getFPOffset();
  MBBI = MBB.getLastNonDebugInstr();
  BuildMI(MBB, MBBI, dl, TII.get(V810::MOVEA), V810::R3)
    .addReg(V810::R2).addImm(-FPOffset);
  // And restore r2 by loading the old value... from the address stored in r2!
  BuildMI(MBB, MBBI, dl, TII.get(V810::LD_W), V810::R2)
    .addReg(V810::R2).addImm(0);
}

MachineBasicBlock::iterator V810FrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  if (!hasReservedCallFrame(MF)) {
    MachineInstr &MI = *I;
    int Size = 0;
    switch (MI.getOpcode()) {
    default: llvm_unreachable("Unrecognized call frame pseudo");
    case V810::ADJCALLSTACKDOWN:
      Size = -MI.getOperand(0).getImm();
      break;
    case V810::ADJCALLSTACKUP:
      Size = MI.getOperand(0).getImm();
      break;
    }
    moveStackPointer(MF, MBB, I, Size);
  }
  return MBB.erase(I);
}

bool
V810FrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  // Reserve call frame if there are no variable sized objects on the stack.
  return !MF.getFrameInfo().hasVarSizedObjects();
}

bool
V810FrameLowering::hasFPImpl(const MachineFunction &MF) const {
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         RegInfo->hasStackRealignment(MF) || MFI.hasVarSizedObjects() ||
         MFI.isFrameAddressTaken();
}

StackOffset
V810FrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                          Register &FrameReg) const {
  const TargetRegisterInfo *RI = MF.getSubtarget().getRegisterInfo();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  if ((RI->hasStackRealignment(MF) ||  MFI.hasVarSizedObjects()) && MFI.isFixedObjectIndex(FI)) {
    FrameReg = V810::R2;
    int FPOffset = MF.getInfo<V810MachineFunctionInfo>()->getFPOffset();
    return StackOffset::getFixed(MFI.getObjectOffset(FI) - FPOffset);
  }
  return TargetFrameLowering::getFrameIndexReference(MF, FI, FrameReg);
}

StackOffset
V810FrameLowering::getFrameIndexReferencePreferSP(const MachineFunction &MF, int FI,
                                                  Register &FrameReg,
                                                  bool IgnoreSPUpdates) const {
  // Fall back to the default logic, which always uses SP
  return TargetFrameLowering::getFrameIndexReference(MF, FI, FrameReg);
}

bool
V810FrameLowering::assignCalleeSavedSpillSlots(MachineFunction &MF,
                                               const TargetRegisterInfo *TRI,
                                               std::vector<CalleeSavedInfo> &CSI) const {
  if (!TRI->shouldRealignStack(MF) && !MF.getFrameInfo().hasVarSizedObjects()) {
    return false;
  }
  int Offset = -4;
  for (auto &CS : CSI) {
    if (CS.isSpilledToReg()) {
      continue;
    }
  
    // Spilled registers should be spilled to fixed offsets,
    // so that we can reference them relative to FP
    int FrameIdx = MF.getFrameInfo().CreateFixedSpillStackObject(4, Offset);
    CS.setFrameIdx(FrameIdx);
    if (CS.getReg() == V810::R2) {
      MF.getInfo<V810MachineFunctionInfo>()->setFPOffset(Offset);
    }
    Offset -= 4;
  }

  return true;
}

void
V810FrameLowering::determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs, RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  if (hasFP(MF)) {
    // If we need to use FP, we always store the old value
    SavedRegs.set(V810::R2);
    // And even in leaf functions, store LP for ease of debugging.
    // (In non-leaf functions, it already had to be saved anyway)
    SavedRegs.set(V810::R31);
  }
}

void
V810FrameLowering::moveStackPointer(MachineFunction &MF, MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MBBI, int bytes) const {
  while (bytes > 32764) {
    moveStackPointer(MF, MBB, MBBI, 32764);
    bytes -= 32764;
  }
  while (bytes < -32768) {
    moveStackPointer(MF, MBB, MBBI, -32768);
    bytes += 32768;
  }
  if (bytes == 0) {
    return;
  }

  DebugLoc dl;
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  if (isInt<5>(bytes)) {
    BuildMI(MBB, MBBI, dl, TII.get(V810::ADDri), V810::R3)
      .addReg(V810::R3).addImm(bytes);
  } else {
    assert(isInt<16>(bytes));
    BuildMI(MBB, MBBI, dl, TII.get(V810::MOVEA), V810::R3)
      .addReg(V810::R3).addImm(bytes);
  }
}
