#include "V810TargetStreamer.h"

using namespace llvm;

V810TargetStreamer::V810TargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

MCELFStreamer &V810TargetStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer); 
}