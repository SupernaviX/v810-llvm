#include "V810MCAsmInfo.h"
#include "V810MCExpr.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"

using namespace llvm;

#define DEBUG_TYPE "v810mcexpr"

const V810MCExpr*
V810MCExpr::create(VariantKind Kind, const MCExpr *Expr,
                     MCContext &Ctx) {
    return new (Ctx) V810MCExpr(Kind, Expr);
}

void V810MCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  bool parens = printVariantKind(OS, Kind);
  
  if (parens) OS << '(';
  MAI->printExpr(OS, *Expr);
  if (parens) OS << ')';
}

bool V810MCExpr::printVariantKind(raw_ostream &OS, VariantKind Kind) {
  switch (Kind) {
  case VK_V810_None:      return false;
  case VK_V810_LO:        OS << "lo"; return true;
  case VK_V810_HI:        OS << "hi"; return true;
  case VK_V810_SDAOFF:    OS << "sdaoff"; return true;
  case VK_V810_9_PCREL:   return false;
  case VK_V810_26_PCREL:  return false;
  }
  llvm_unreachable("Unhandled V810MCExpr::VariantKind");
}

V810::Fixups V810MCExpr::getFixupKind(V810MCExpr::VariantKind Kind) {
  switch (Kind) {
  default: llvm_unreachable("Unhandled V810MCExpr::VariantKind");
  case VK_V810_LO:        return V810::fixup_v810_lo;
  case VK_V810_HI:        return V810::fixup_v810_hi;
  case VK_V810_SDAOFF:    return V810::fixup_v810_sdaoff;
  case VK_V810_9_PCREL:   return V810::fixup_v810_9_pcrel;
  case VK_V810_26_PCREL:  return V810::fixup_v810_26_pcrel;
  }
}

bool
V810MCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                      const MCAssembler *Asm) const {
  return getSubExpr()->evaluateAsRelocatable(Res, Asm);  
}

void V810MCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}