add_llvm_component_group(V810)

set(LLVM_TARGET_DEFINITIONS V810.td)

llvm_tablegen(V810GenAsmMatcher.inc -gen-asm-matcher)
llvm_tablegen(V810GenAsmWriter.inc -gen-asm-writer)
llvm_tablegen(V810GenCallingConv.inc -gen-callingconv)
llvm_tablegen(V810GenDAGISel.inc -gen-dag-isel)
llvm_tablegen(V810GenDisassemblerTables.inc -gen-disassembler)
llvm_tablegen(V810GenInstrInfo.inc -gen-instr-info)
llvm_tablegen(V810GenMCCodeEmitter.inc -gen-emitter)
llvm_tablegen(V810GenRegisterInfo.inc -gen-register-info)
llvm_tablegen(V810GenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(V810CommonTableGen)

add_llvm_target(V810CodeGen
  V810AsmPrinter.cpp
  V810BranchSelector.cpp
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