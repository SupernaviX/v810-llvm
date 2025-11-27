add_llvm_component_library(LLVMV810Info
  V810TargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  V810
  )