#ifndef LLVM_V810_FIXUP_KINDS_H
#define LLVM_V810_FIXUP_KINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace V810 {
// Keep this list in order with MCFixupKindInfo Infos in V810AsmBackend.cpp
enum Fixups {
  // The low 16 bits of a MOVHI/MOVEA pair
  fixup_v810_lo = FirstTargetFixupKind,
  // The high 16 bits of a MOVHI/MOVEA pair
  fixup_v810_hi,
  // A 16-bit GP-relative fixup
  fixup_v810_sdaoff, 
  // A 9-bit PC-relative fixup.
  fixup_v810_9_pcrel,
  // A 26-bit PC-relative fixup.
  fixup_v810_26_pcrel,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
}
}

#endif