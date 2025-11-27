#ifndef LLVM_LIB_TARGET_V810_V810MACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_V810_V810MACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

  class V810MachineFunctionInfo : public MachineFunctionInfo {
  private:
    int VarArgsFrameIndex = 0;
    int FPOffset = 0;
    virtual void anchor();
  public:
    V810MachineFunctionInfo(const Function &f, const TargetSubtargetInfo *STI) {}
 
    MachineFunctionInfo *
    clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
          const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
        const override;

    int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
    void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

    int getFPOffset() const { return FPOffset; }
    void setFPOffset(int Offset) { FPOffset = Offset; }
  };
}

#endif