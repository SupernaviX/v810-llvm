#include "V810.h"
#include "V810MCAsmInfo.h"
#include "V810MCExpr.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"

using namespace llvm;

#define DEBUG_TYPE "v810mcexpr"

const V810MCExpr*
V810MCExpr::create(Specifier S, const MCExpr *Expr,
                     MCContext &Ctx) {
    return new (Ctx) V810MCExpr(S, Expr);
}

V810::Fixups V810MCExpr::getFixupKind(Specifier S) {
  switch (S) {
  default: llvm_unreachable("Unhandled Specifier");
  case V810::S_V810_LO:        return V810::fixup_v810_lo;
  case V810::S_V810_HI:        return V810::fixup_v810_hi;
  case V810::S_V810_16:        return V810::fixup_v810_16;
  case V810::S_V810_SDAOFF:    return V810::fixup_v810_sdaoff;
  case V810::S_V810_9_PCREL:   return V810::fixup_v810_9_pcrel;
  case V810::S_V810_26_PCREL:  return V810::fixup_v810_26_pcrel;
  }
}

bool V810MCExpr::printSpecifier(raw_ostream &OS, uint16_t S) {
  switch (S) {
  case V810::S_V810_LO:     OS << "lo"; return true;
  case V810::S_V810_HI:     OS << "hi"; return true;
  case V810::S_V810_SDAOFF: OS << "sdaoff"; return true;
  default: return false;
  }
}
