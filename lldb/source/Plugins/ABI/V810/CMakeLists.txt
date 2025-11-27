add_lldb_library(lldbPluginABIV810 PLUGIN
  ABIV810.cpp

  LINK_LIBS
    lldbCore
    lldbSymbol
    lldbTarget
  LINK_COMPONENTS
    Support
    TargetParser
)