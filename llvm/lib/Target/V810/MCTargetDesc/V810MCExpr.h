#ifndef LLVM_LIB_TARGET_V810_MCTARGETDESC_V810MCEXPR_H
#define LLVM_LIB_TARGET_V810_MCTARGETDESC_V810MCEXPR_H

#include "V810FixupKinds.h"
#include "llvm/MC/MCExpr.h"

namespace llvm {

class StringRef;
class V810MCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_V810_None,
    VK_V810_LO,
    VK_V810_HI,
    VK_V810_SDAOFF,
    VK_V810_9_PCREL,
    VK_V810_26_PCREL
  };

private:
  const VariantKind Kind;
  const MCExpr *Expr;

  explicit V810MCExpr(VariantKind Kind, const MCExpr *Expr)
      : Kind(Kind), Expr(Expr) {}

public:
  static const V810MCExpr *create(VariantKind Kind, const MCExpr *Expr,
                                MCContext &Ctx);

  VariantKind getKind() const { return Kind; }

  const MCExpr *getSubExpr() const { return Expr; }

  V810::Fixups getFixupKind() const { return getFixupKind(Kind); }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res,
                                 const MCAssembler *Asm) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  static bool classof(const V810MCExpr *) { return true; }

  static bool printVariantKind(raw_ostream &OS, VariantKind Kind);
  static V810::Fixups getFixupKind(VariantKind Kind);
};

} // end namespace llvm

#endif