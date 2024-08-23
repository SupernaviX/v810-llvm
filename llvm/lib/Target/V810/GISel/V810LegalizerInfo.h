#ifndef LLVM_LIB_TARGET_V810_V810LEGALIZERINFO_H
#define LLVM_LIB_TARGET_V810_V810LEGALIZERINFO_H

#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"

namespace llvm {

class V810Subtarget;

class V810LegalizerInfo : public LegalizerInfo {
public:
  V810LegalizerInfo(const V810Subtarget &STI);
};

}

#endif
