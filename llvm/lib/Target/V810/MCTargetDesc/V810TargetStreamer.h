#ifndef LLVM_LIB_TARGET_V810_MCTARGETDESC_V810TARGETSTREAMER_H
#define LLVM_LIB_TARGET_V810_MCTARGETDESC_V810TARGETSTREAMER_H

#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {
class V810TargetStreamer : public MCTargetStreamer {
public:
  V810TargetStreamer(MCStreamer &S);
  MCELFStreamer &getStreamer();
};

}

#endif