#ifndef LLVM_LIB_TARGET_V810_V810TARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_V810_V810TARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/MC/MCSectionELF.h"

namespace llvm {

class MCContext;
class TargetMachine;

class V810TargetObjectFile : public TargetLoweringObjectFileELF {
public:
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

  MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                    const TargetMachine &MT) const override;

  MCSection *getExplicitSectionGlobal(const GlobalObject *GO,
                                      SectionKind Kind,
                                      const TargetMachine &TM) const override;

  bool isGlobalInSmallSection(const GlobalObject *GO, const TargetMachine &TM) const;
private:
  MCSectionELF *SmallDataSection;
  MCSectionELF *SmallBSSSection;

  MCSection *selectSmallSectionForGlobal(const GlobalObject *GO,
                                         SectionKind Kind,
                                         const TargetMachine &TM) const;
};

} // end namespace llvm

#endif