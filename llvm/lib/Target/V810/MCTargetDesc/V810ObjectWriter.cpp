#include "MCTargetDesc/V810FixupKinds.h"
#include "MCTargetDesc/V810MCExpr.h"
#include "MCTargetDesc/V810MCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"

using namespace llvm;

namespace {
  class V810ObjectWriter : public MCELFObjectTargetWriter {
  public:
    V810ObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(false, OSABI, ELF::EM_V810, true) {}
    ~V810ObjectWriter() override = default;
  protected:
    unsigned getRelocType(const MCFixup &Fixup, const MCValue &Target,
                          bool IsPCRel) const override;
  };
}

unsigned V810ObjectWriter::getRelocType(const MCFixup &Fixup,
                                        const MCValue &Target,
                                        bool IsPCRel) const {
  MCFixupKind Kind = Fixup.getKind();
  if (mc::isRelocation(Kind))
    return Kind;

  if (IsPCRel) {
    switch(Fixup.getKind()) {
    default:
      llvm_unreachable("Unimplemented fixup -> relocation");
    case FK_NONE:                   return ELF::R_V810_NONE;
    case FK_Data_1:                 return ELF::R_V810_DISP8;
    case FK_Data_2:                 return ELF::R_V810_DISP16;
    case FK_Data_4:                 return ELF::R_V810_DISP32;
    case V810::fixup_v810_9_pcrel:  return ELF::R_V810_9_PCREL;
    case V810::fixup_v810_26_pcrel: return ELF::R_V810_26_PCREL;
    }

  }
  
  switch(Fixup.getKind()) {
  default:
    llvm_unreachable("Unimplemented fixup -> relocation");
  case FK_NONE:                   return ELF::R_V810_NONE;
  case FK_Data_1:                 return ELF::R_V810_8;
  case FK_Data_2:                 return ELF::R_V810_16;
  case FK_Data_4:                 return ELF::R_V810_32;
  case V810::fixup_v810_lo:       return ELF::R_V810_LO;
  case V810::fixup_v810_hi:       return ELF::R_V810_HI_S;
  case V810::fixup_v810_sdaoff:   return ELF::R_V810_SDAOFF;
  }

  return ELF::R_V810_NONE;
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createV810ObjectWriter(uint8_t OSABI) {
  return std::make_unique<V810ObjectWriter>(OSABI);
}
