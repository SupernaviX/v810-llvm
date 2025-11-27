add_llvm_component_library(LLVMV810Desc
  V810AsmBackend.cpp
  V810InstPrinter.cpp
  V810MCAsmInfo.cpp
  V810MCCodeEmitter.cpp
  V810MCExpr.cpp
  V810MCTargetDesc.cpp
  V810ObjectWriter.cpp
  V810TargetStreamer.cpp

  LINK_COMPONENTS
  MC
  V810Info
  Support
  TargetParser

  ADD_TO_COMPONENT
  V810
  )