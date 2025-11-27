#include "ABIInfoImpl.h"
#include "TargetInfo.h"

#include <optional>

using namespace clang;
using namespace clang::CodeGen;

namespace {

// r6-r9 could theoretically be used to pass the struct.
// This will still spill onto the stack if there are more arguments.
constexpr int MAX_FIELDS_PER_ARG = 4;

// r10+r11 for return values.
constexpr int MAX_FIELDS_PER_RETURN_VALUE = 2;

class V810ABIInfo : public DefaultABIInfo {
public:
  V810ABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}

  void computeInfo(CGFunctionInfo &FI) const override;

private:
  ABIArgInfo classifyArgumentType(QualType Ty) const;
  ABIArgInfo classifyReturnType(QualType Ty) const;
  bool mustBeIndirect(QualType Ty) const;

  // Returns std::nullopt if could not determine.
  std::optional<int> countLeafFields(QualType Ty) const;
};

void V810ABIInfo::computeInfo(CGFunctionInfo &FI) const {
  if (!getCXXABI().classifyReturnType(FI))
    FI.getReturnInfo() = classifyReturnType(FI.getReturnType());
  for (auto &Arg : FI.arguments())
    Arg.info = classifyArgumentType(Arg.type);
}

ABIArgInfo V810ABIInfo::classifyArgumentType(QualType Ty) const {
  if (Ty->isStructureOrClassType() && !mustBeIndirect(Ty)) {
    const std::optional<int> FieldCount = countLeafFields(Ty);
    if (FieldCount && *FieldCount <= MAX_FIELDS_PER_ARG) {
      return ABIArgInfo::getDirect(CGT.ConvertType(Ty));
    }
  }
  return DefaultABIInfo::classifyArgumentType(Ty);
}

ABIArgInfo V810ABIInfo::classifyReturnType(QualType Ty) const {
  if (Ty->isStructureOrClassType()) {
    const std::optional<int> FieldCount = countLeafFields(Ty);
    if (FieldCount && *FieldCount <= MAX_FIELDS_PER_RETURN_VALUE) {
      return ABIArgInfo::getDirect(CGT.ConvertType(Ty));
    }
  }
  return DefaultABIInfo::classifyReturnType(Ty);
}

bool V810ABIInfo::mustBeIndirect(QualType Ty) const {
  // Determine whether this must be indirect due to it being
  // a C++ move-constructed instance (among possibly other reasons).
  const CGCXXABI::RecordArgABI RAA = getRecordArgABI(Ty, getCXXABI());
  return RAA == CGCXXABI::RAA_Indirect;
}

// Adapted from CodeGen::isSingleElementStruct
std::optional<int> V810ABIInfo::countLeafFields(QualType T) const {
  const RecordType *RT = T->getAs<RecordType>();
  if (!RT)
    return std::nullopt;

  const RecordDecl *RD = RT->getDecl();
  if (RD->hasFlexibleArrayMember())
    return std::nullopt;

  int Count = 0;

  // If this is a C++ record, check the bases first.
  if (const CXXRecordDecl *CXXRD = dyn_cast<CXXRecordDecl>(RD)) {
    for (const auto &I : CXXRD->bases()) {
      // Ignore empty records.
      if (isEmptyRecord(getContext(), I.getType(), true))
        continue;

      const std::optional<int> BaseClassCount = countLeafFields(I.getType());
      if (!BaseClassCount)
        return std::nullopt;
      Count += *BaseClassCount;
    }
  }

  // Count immediate fields.
  for (const auto *FD : RD->fields()) {
    QualType FT = FD->getType();

    // Ignore empty fields.
    if (isEmptyField(getContext(), FD, true))
      continue;

    // Sum up elements in arrays.
    while (const ConstantArrayType *AT =
               getContext().getAsConstantArrayType(FT)) {
      Count += AT->getSize().getZExtValue();
      FT = AT->getElementType();
    }

    if (!isAggregateTypeForABI(FT)) {
      // Found a leaf field.
      ++Count;
    } else {
      const std::optional<int> NestedStructCount = countLeafFields(FT);
      if (!NestedStructCount)
        return std::nullopt;
      Count += *NestedStructCount;
    }
  }

  return Count;
}

class V810TargetCodeGenInfo : public TargetCodeGenInfo {
public:
  V810TargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<V810ABIInfo>(CGT)) {}

  void setTargetAttributes(const Decl *D, llvm::GlobalValue *GV,
                           CodeGen::CodeGenModule &CGM) const override {
    if (GV->isDeclaration())
      return;
    const auto *FD = dyn_cast_or_null<FunctionDecl>(D);
    if (!FD)
      return;
    auto *Fn = cast<llvm::Function>(GV);

    if (FD->getAttr<V810InterruptAttr>())
      Fn->addFnAttr("interrupt");
  }
};
} // namespace

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createV810TargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<V810TargetCodeGenInfo>(CGM.getTypes());
}
