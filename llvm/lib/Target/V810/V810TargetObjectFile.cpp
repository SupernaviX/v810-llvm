#include "V810Subtarget.h"
#include "V810TargetMachine.h"
#include "V810TargetObjectFile.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Target/TargetMachine.h"

#define DEBUG_TYPE "v810-sdata"

using namespace llvm;

void V810TargetObjectFile::Initialize(MCContext &Ctx, const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);

  SmallDataSection =
    getContext().getELFSection(".sdata", ELF::SHT_PROGBITS,
                               ELF::SHF_WRITE | ELF::SHF_ALLOC |
                               ELF::SHF_V810_GPREL);

  SmallBSSSection =
    getContext().getELFSection(".sbss", ELF::SHT_NOBITS,
                               ELF::SHF_WRITE | ELF::SHF_ALLOC |
                               ELF::SHF_V810_GPREL);
}

static bool isSmallDataSection(StringRef Sec) {
  return Sec.starts_with(".sdata") || Sec.starts_with(".sbss");
}

bool V810TargetObjectFile::isGlobalInSmallSection(const GlobalObject *GO, const TargetMachine &TM) const {
  const GlobalVariable *GVar = dyn_cast<GlobalVariable>(GO);
  if (!GVar) return false;
  if (GVar->isConstant()) return false;
  if (GVar->hasSection()) {
    return isSmallDataSection(GVar->getSection());
  }
  if (TM.getMCSubtargetInfo()->checkFeatures("+gprel")) {
    // Put ALL non-constant variables in a small section
    return true;
  }
  // It's all or nothing
  return false;
}

MCSection *V810TargetObjectFile::SelectSectionForGlobal(
    const GlobalObject *GO, SectionKind Kind, const TargetMachine &TM) const {
  
  if (isGlobalInSmallSection(GO, TM))
    return selectSmallSectionForGlobal(GO, Kind, TM);

  return TargetLoweringObjectFileELF::SelectSectionForGlobal(GO, Kind, TM);
}

MCSection *V810TargetObjectFile::getExplicitSectionGlobal(
    const GlobalObject *GO, SectionKind Kind, const TargetMachine &TM) const {

  if (isGlobalInSmallSection(GO, TM))
    return selectSmallSectionForGlobal(GO, Kind, TM);
  
  return TargetLoweringObjectFileELF::getExplicitSectionGlobal(GO, Kind, TM);
}

MCSection *V810TargetObjectFile::selectSmallSectionForGlobal(const GlobalObject *GO,
                                                             SectionKind Kind,
                                                             const TargetMachine &TM) const {
  if (Kind.isBSS())
    return SmallBSSSection;
  if (Kind.isData())
    return SmallDataSection;

  return TargetLoweringObjectFileELF::SelectSectionForGlobal(GO, Kind, TM);
}