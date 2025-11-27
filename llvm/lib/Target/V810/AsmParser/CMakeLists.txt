add_llvm_component_library(LLVMV810AsmParser
  V810AsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  V810Desc
  V810Info
  Support
  TargetParser

  ADD_TO_COMPONENT
  V810
  )
