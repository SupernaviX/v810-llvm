#ifndef LLVM_LIB_TARGET_V810_V810FRAMELOWERING_H
#define LLVM_LIB_TARGET_V810_V810FRAMELOWERING_H

#include "V810.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/TypeSize.h"

namespace llvm {

class V810FrameLowering : public TargetFrameLowering {
public:
  explicit V810FrameLowering();

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF,
                                MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;
  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;
  StackOffset getFrameIndexReferencePreferSP(const MachineFunction &MF, int FI,
                                     Register &FrameReg, bool IgnoreSPUpdates) const override;
  bool assignCalleeSavedSpillSlots(MachineFunction &MF,
      const TargetRegisterInfo *TRI, std::vector<CalleeSavedInfo> &CSI)
      const override;
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;
protected:
  bool hasFPImpl(const MachineFunction &FM) const override;
private:
  void moveStackPointer(MachineFunction &MF, MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MBBI, int bytes) const;
};

} // End llvm namespace

#endif