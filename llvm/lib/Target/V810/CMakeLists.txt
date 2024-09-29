add_llvm_component_group(V810)

set(LLVM_TARGET_DEFINITIONS V810.td)

tablegen(LLVM V810GenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM V810GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM V810GenCallingConv.inc -gen-callingconv)
tablegen(LLVM V810GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM V810GenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM V810GenGlobalISel.inc -gen-global-isel)
tablegen(LLVM V810GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM V810GenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM V810GenPreLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="V810PreLegalizerCombiner")
tablegen(LLVM V810GenO0PreLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="V810O0PreLegalizerCombiner")
tablegen(LLVM V810GenPostLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="V810PostLegalizerCombiner")
tablegen(LLVM V810GenRegisterBank.inc -gen-register-bank)
tablegen(LLVM V810GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM V810GenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(V810CommonTableGen)

add_llvm_target(V810CodeGen
  V810AsmPrinter.cpp
  V810BranchSelector.cpp
  V810CallingConv.cpp
  V810FrameLowering.cpp
  V810InstrInfo.cpp
  V810ISelDAGToDAG.cpp
  V810ISelLowering.cpp
  V810MachineFunctionInfo.cpp
  V810MCInstLower.cpp
  V810RegisterInfo.cpp
  V810SelectionDAGInfo.cpp
  V810Subtarget.cpp
  V810TargetMachine.cpp
  V810TargetObjectFile.cpp
  V810TargetTransformInfo.cpp
  GISel/V810CallLowering.cpp
  GISel/V810InstructionSelector.cpp
  GISel/V810LegalizerInfo.cpp
  GISel/V810PreLegalizerCombiner.cpp
  GISel/V810O0PreLegalizerCombiner.cpp
  GISel/V810PostLegalizerCombiner.cpp
  GISel/V810RegisterBankInfo.cpp

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  MC
  SelectionDAG
  Support
  Target
  TargetParser
  V810Info

  ADD_TO_COMPONENT
  V810
)

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)