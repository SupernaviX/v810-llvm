add_llvm_component_library(LLVMV810Disassembler
  V810Disassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  MC
  V810Info
  Support

  ADD_TO_COMPONENT
  V810
  )
