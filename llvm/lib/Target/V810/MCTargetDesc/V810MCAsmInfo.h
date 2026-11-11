#ifndef LLVM_LIB_TARGET_V810_MCTARGETDESC_V810MCASMINFO_H
#define LLVM_LIB_TARGET_V810_MCTARGETDESC_V810MCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/MC/MCExpr.h"

namespace llvm {

class Triple;

class V810AsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit V810AsmInfo(const Triple &TheTriple,
                       const MCTargetOptions &Options);

  void printSpecifierExpr(raw_ostream &O, const MCSpecifierExpr &S) const override;
};

namespace V810 {
using Specifier = uint16_t;
enum {
  S_None,

  S_V810_NONE = MCSymbolRefExpr::FirstTargetSpecifier,
  S_V810_LO,
  S_V810_HI,
  S_V810_16,
  S_V810_SDAOFF,
  S_V810_9_PCREL,
  S_V810_26_PCREL
};
} // namespace V810

} // end namespace llvm

#endif // LLVM_LIB_TARGET_V810_MCTARGETDESC_V810MCASMINFO_H