#ifndef LLVM_LIB_TARGET_V810_V810INSTRINFO_H
#define LLVM_LIB_TARGET_V810_V810INSTRINFO_H

#include "V810RegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "V810GenInstrInfo.inc"

namespace llvm {

class V810Subtarget;

namespace V810II {
// V810 Instruction Flags. Keep this in sync with V810InstrFormats.td
enum CCFlags {
  V810_NoFlags = 0x0,
  V810_ZFlag = 0x1,
  V810_SFlag = 0x2,
  V810_CYFlag = 0x4,
  V810_OVFlag = 0x8,
  V810_AllFlags = 0xf
};

inline CCFlags operator|(CCFlags a, CCFlags b) {
  return static_cast<CCFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline CCFlags operator&(CCFlags a, CCFlags b) {
  return static_cast<CCFlags>(static_cast<int>(a) & static_cast<int>(b));
}
}

class V810InstrInfo : public V810GenInstrInfo {
  const V810RegisterInfo RI;
  virtual void anchor();
  /// isLoadFromStackSlot - If the specified machine instruction is a direct
  /// load from a stack slot, return the virtual or physical register number of
  /// the destination along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than loading from the stack slot.
  Register isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;

  /// isStoreToStackSlot - If the specified machine instruction is a direct
  /// store to a stack slot, return the virtual or physical register number of
  /// the source reg along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than storing to the stack slot.
  Register isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  virtual void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                           const DebugLoc &DL, Register DestReg, Register SrcReg,
                           bool KillSrc, bool RenamableDest = false,
                           bool RenamableSrc = false) const override;
  virtual void storeRegToStackSlot(
              MachineBasicBlock &MBB, MachineBasicBlock::iterator HI, Register SrcReg,
              bool isKill, int FrameIndex, const TargetRegisterClass *RC,
              Register VReg,
              MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;
  virtual void loadRegFromStackSlot(
              MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
              Register DestReg, int FrameIndex, const TargetRegisterClass *RC,
              Register VReg,
              MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  bool isBranchOffsetInRange(unsigned BranchOpc, int64_t Offset) const override;

  bool analyzeCompare(const MachineInstr &MI, Register &SrcReg,
                      Register &SrcReg2, int64_t &CmpMask,
                      int64_t &CmpValue) const override;
  bool optimizeCompareInstr(MachineInstr &CmpInstr, Register SrcReg,
                            Register SrcReg2, int64_t CmpMask, int64_t CmpValue,
                            const MachineRegisterInfo *MRI) const override;

  bool shouldClusterMemOps(ArrayRef<const MachineOperand *> BaseOps1,
                           int64_t Offset1, bool OffsetIsScalable1,
                           ArrayRef<const MachineOperand *> BaseOps2,
                           int64_t Offset2, bool OffsetIsScalable2,
                           unsigned ClusterSize,
                           unsigned NumBytes) const override;
  bool getMemOperandsWithOffsetWidth(
      const MachineInstr &MI, SmallVectorImpl<const MachineOperand *> &BaseOps,
      int64_t &Offset, bool &OffsetIsScalable, LocationSize &Width,
      const TargetRegisterInfo *TRI) const override;

  bool isUnpredicatedTerminatorBesidesNop(const MachineInstr &MI) const;

public:
  explicit V810InstrInfo(const V810Subtarget &ST);
  const V810RegisterInfo &getRegisterInfo() const { return RI; }

  virtual unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

};

}

#endif