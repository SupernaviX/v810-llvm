#include "TargetInfo/V810TargetInfo.h"
#include "V810.h"
#include "V810MachineFunctionInfo.h"
#include "V810TargetMachine.h"
#include "V810TargetObjectFile.h"
#include "V810TargetTransformInfo.h"
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810Target() {
  // Register the target.
  RegisterTargetMachine<V810TargetMachine> X(getTheV810Target());
  auto PR = PassRegistry::getPassRegistry();
  initializeGlobalISel(*PR);
  initializeV810PreLegalizerCombinerPass(*PR);
  initializeV810O0PreLegalizerCombinerPass(*PR);
  initializeV810PostLegalizerCombinerPass(*PR);
}

static std::string computeDataLayout(const Triple &T) {
  return "e-p:32:32-i32:32-i64:32-f32:32-a:0:32-n32:32-S32";
}

V810TargetMachine::V810TargetMachine(
    const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL, bool JIT)
    : LLVMTargetMachine(
        T, computeDataLayout(TT), TT, CPU, FS, Options,
        RM.value_or(Reloc::Static),
        CM.value_or(CodeModel::Small),
        OL),
      TLOF(std::make_unique<V810TargetObjectFile>()) {
  initAsmInfo();
}

const V810Subtarget *
V810TargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU =
      CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  std::string FS =
      FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;
  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<V810Subtarget>(TargetTriple, CPU, FS, *this);
  }
  return I.get();
}

MachineFunctionInfo *V810TargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return V810MachineFunctionInfo::create<V810MachineFunctionInfo>(Allocator, F, STI);
}

TargetTransformInfo
V810TargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(V810TTIImpl(this, F));
}

namespace {
class V810PassConfig : public TargetPassConfig {
public:
  V810PassConfig(V810TargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {
      substitutePass(&PostRASchedulerID, &PostMachineSchedulerID);
    }

  V810TargetMachine &getV810TargetMachine() const {
    return getTM<V810TargetMachine>();
  }

  ScheduleDAGInstrs *createMachineScheduler(MachineSchedContext *C) const override;
  ScheduleDAGInstrs *createPostMachineScheduler(MachineSchedContext *C) const override;

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
  void addPreEmitPass2() override;
  bool addIRTranslator() override;
  void addPreLegalizeMachineIR() override;
  bool addLegalizeMachineIR() override;
  void addPreRegBankSelect() override;
  bool addRegBankSelect() override;
  bool addGlobalInstructionSelect() override;
};
} // namespace

TargetPassConfig *V810TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new V810PassConfig(*this, PM);
}

ScheduleDAGInstrs *V810PassConfig::createMachineScheduler(MachineSchedContext *C) const {
  ScheduleDAGMILive *DAG = new ScheduleDAGMILive(C, std::make_unique<GenericScheduler>(C));
  DAG->addMutation(createLoadClusterDAGMutation(DAG->TII, DAG->TRI));
  return DAG;
}

ScheduleDAGInstrs *V810PassConfig::createPostMachineScheduler(MachineSchedContext *C) const {
  ScheduleDAGMI *DAG = new ScheduleDAGMI(C, std::make_unique<PostGenericScheduler>(C), true);
  DAG->addMutation(createLoadClusterDAGMutation(DAG->TII, DAG->TRI));
  return DAG;
}

void V810PassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());

  TargetPassConfig::addIRPasses();
}

bool V810PassConfig::addInstSelector() {
  addPass(createV810ISelDag(getV810TargetMachine(), getOptLevel()));
  return false;
}

void V810PassConfig::addPreEmitPass() {
  addPass(&BranchRelaxationPassID);
  addPass(createV810BranchSelectionPass());
}

void V810PassConfig::addPreEmitPass2() {
  // addPass(createV810BranchSelectionPass());
}

bool V810PassConfig::addIRTranslator() {
  addPass(new IRTranslator(getOptLevel()));
  return false;
}

void V810PassConfig::addPreLegalizeMachineIR() {
  if (getOptLevel() == CodeGenOptLevel::None) {
    addPass(createV810O0PreLegalizerCombiner());
  } else {
    addPass(createV810PreLegalizerCombiner());
  }
}

bool V810PassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  return false;
}

void V810PassConfig::addPreRegBankSelect() {
  if (getOptLevel() != CodeGenOptLevel::None)
    addPass(createV810PostLegalizerCombiner());
}

bool V810PassConfig::addRegBankSelect() {
  addPass(new RegBankSelect());
  return false;
}

bool V810PassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect(getOptLevel()));
  return false;
}

V810TargetMachine::~V810TargetMachine() {}
