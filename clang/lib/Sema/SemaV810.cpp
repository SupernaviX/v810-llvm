#include "clang/Sema/SemaV810.h"
#include "clang/AST/DeclBase.h"
#include "clang/Basic/DiagnosticSema.h"
#include "clang/Sema/Attr.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h" 

namespace clang {
SemaV810::SemaV810(Sema &S) : SemaBase(S) {}

void SemaV810::handleInterruptAttr(Decl *D, const ParsedAttr &AL) {
  if (!isFuncOrMethodForAttrSubject(D)) {
    Diag(D->getLocation(), diag::warn_attribute_wrong_decl_type)
        << AL << AL.isRegularKeywordAttribute() << ExpectedFunction;
    return;
  }

  if (hasFunctionProto(D) && getFunctionOrMethodNumParams(D) != 0) {
    Diag(D->getLocation(), diag::warn_interrupt_signal_attribute_invalid)
        << /*V810*/ 4 << /*interrupt*/ 0 << 0;
    return;
  }

  if (!getFunctionOrMethodResultType(D)->isVoidType()) {
    Diag(D->getLocation(), diag::warn_interrupt_signal_attribute_invalid)
        << /*V810*/ 4 << /*interrupt*/ 0 << 1;
    return;
  }

  if (!AL.checkExactlyNumArgs(SemaRef, 0))
    return;

  handleSimpleAttribute<V810InterruptAttr>(*this, D, AL);
}

} // namespace clang