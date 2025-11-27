#ifndef LLVM_CLANG_SEMA_SEMAV810_H
#define LLVM_CLANG_SEMA_SEMAV810_H

#include "clang/Sema/SemaBase.h"

namespace clang {
class Decl;
class ParsedAttr;

class SemaV810 : public SemaBase {
public:
  SemaV810(Sema &S);

  void handleInterruptAttr(Decl *D, const ParsedAttr &AL);
};

} // namespace clang

#endif // LLVM_CLANG_SEMA_SEMAV810_H