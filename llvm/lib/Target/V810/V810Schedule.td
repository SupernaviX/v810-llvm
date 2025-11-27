
// Below "Itineraries" are only used by the ScoreboardHazardRecognizer
def MemoryUnit : FuncUnit;

def IIC_Load : InstrItinClass;
def IIC_Store : InstrItinClass;

def V810Itineraries : ProcessorItineraries<
  [MemoryUnit],
  [],
  [
    InstrItinData<IIC_Load, [InstrStage<0, [MemoryUnit]>]>,
    InstrItinData<IIC_Store, [InstrStage<2, [MemoryUnit]>]>,
  ]>;

// The Schedule Machine Model itself

def V810Model : SchedMachineModel {
  let MicroOpBufferSize = 0; // In-order
  let IssueWidth = 1; // One thing happens per cycle
  let PostRAScheduler = 1;

  let Itineraries = V810Itineraries;
}

// Resources used by scheduler definitions
// (the 1 means there's only one of each)
let BufferSize = 0 in {
def V810UnitMemory  : ProcResource<1>;
def V810UnitBranch  : ProcResource<1>;
def V810UnitFPU     : ProcResource<1>;
}

// Scheduler Definitions
// (the result/operands of different instructions use these)
def WriteI        : SchedWrite; // math-free MOVs
def WriteALU      : SchedWrite; // most integer math can be treated the same
def WriteIMul     : SchedWrite; // MUL/MULU
def WriteIDiv     : SchedWrite; // DIV/DIVU
def WriteFPU      : SchedWrite; // being lazy and treating FP math the same too
def WriteLoad     : SchedWrite; // Load from memory
def WriteStore    : SchedWrite; // Store to memory
def WriteBranch   : SchedWrite; // jump jump

// Tie it all together
let SchedModel = V810Model in {

def : WriteRes<WriteI, []>;
def : WriteRes<WriteALU, []>;
def : WriteRes<WriteIMul, []>;
def : WriteRes<WriteIDiv, []>;
def : WriteRes<WriteFPU, [V810UnitFPU]>;
def : WriteRes<WriteLoad, [V810UnitMemory]>; 
def : WriteRes<WriteStore, [V810UnitMemory]> { let ReleaseAtCycles = [2]; }
def : WriteRes<WriteBranch, [V810UnitBranch]>; // TODO: how do I say "this flushes the pipeline"

// the COPY pseudo-instr needs scheduling info
def : InstRW<[WriteI], (instrs COPY)>;

}