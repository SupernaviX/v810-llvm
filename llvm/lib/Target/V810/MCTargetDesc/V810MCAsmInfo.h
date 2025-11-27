#ifndef LLVM_LIB_TARGET_V810_MCTARGETDESC_V810MCASMINFO_H
#define LLVM_LIB_TARGET_V810_MCTARGETDESC_V810MCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {

class Triple;

class V810AsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit V810AsmInfo(const Triple &TheTriple);
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_V810_MCTARGETDESC_V810MCASMINFO_H