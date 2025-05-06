#ifndef LLVM_LIB_TARGET_V810_V810TARGETMACHINE_H
#define LLVM_LIB_TARGET_V810_V810TARGETMACHINE_H

#include "V810Subtarget.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Target/TargetMachine.h"
#include <optional>

namespace llvm {

class V810TargetMachine : public CodeGenTargetMachineImpl {
private:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  mutable StringMap<std::unique_ptr<V810Subtarget>> SubtargetMap;
public:
  V810TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                    bool JIT);
  ~V810TargetMachine() override;

  const V810Subtarget *getSubtargetImpl() const;
  const V810Subtarget *getSubtargetImpl(const Function &F) const override;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  ScheduleDAGInstrs *createMachineScheduler(MachineSchedContext *C) const override;
  ScheduleDAGInstrs *createPostMachineScheduler(MachineSchedContext *C) const override;
};

} // end namespace llvm

#endif
