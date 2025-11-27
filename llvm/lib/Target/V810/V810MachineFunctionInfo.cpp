#include "V810MachineFunctionInfo.h"

using namespace llvm;

void V810MachineFunctionInfo::anchor() { }

MachineFunctionInfo *V810MachineFunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  return DestMF.cloneInfo<V810MachineFunctionInfo>(*this);
}
