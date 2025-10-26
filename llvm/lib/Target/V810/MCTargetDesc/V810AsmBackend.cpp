#include "V810.h"
#include "MCTargetDesc/V810FixupKinds.h"
#include "MCTargetDesc/V810MCTargetDesc.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

// The v810 is little-endian, but stores 32-bit instructions with their high halfword first.
// We need to exchange halfwords after adjusting fixups to match this.
static uint64_t xh(uint64_t value) {
  return (value >> 16) | (value << 16);
}

template <unsigned N>
static void checkOffsetInRange(const MCFixup &Fixup, int64_t Value,
                               MCContext &Ctx) {
  if (isInt<N>(Value)) return;
  Ctx.reportError(Fixup.getLoc(),
    Twine("offset value ") + Twine(Value) + " out of range [" +
    Twine(minIntN(N)) + ", " + Twine(maxIntN(N)) + "]");
}

static unsigned adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext &Ctx) {
  switch (Fixup.getKind()) {
  default:
    llvm_unreachable("Unknown fixup kind!");
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
    return Value;
  case V810::fixup_v810_lo:
    return xh(EvalLo(Value));
  case V810::fixup_v810_hi:
    return xh(EvalHi(Value));
  case V810::fixup_v810_sdaoff:
    return 0; // can't compute this here
  case V810::fixup_v810_9_pcrel:
    checkOffsetInRange<9>(Fixup, Value, Ctx);
    return Value & 0x01ff;
  case V810::fixup_v810_26_pcrel:
    checkOffsetInRange<26>(Fixup, Value, Ctx);
    return xh(Value & 0x03ffffff);
  }
}

namespace {
  class V810AsmBackend : public MCAsmBackend {
  private:
    Triple::OSType OSType;
  public:
    V810AsmBackend(Triple::OSType OSType) : MCAsmBackend(endianness::little), OSType(OSType) {}

    std::optional<MCFixupKind> getFixupKind(StringRef Name) const override {
      unsigned Type;
      Type = llvm::StringSwitch<unsigned>(Name)
#define ELF_RELOC(X, Y) .Case(#X, Y)
#include "llvm/BinaryFormat/ELFRelocs/V810.def"
#undef ELF_RELOC
                 .Case("BFD_RELOC_NONE", ELF::R_V810_NONE)
                 .Case("BFD_RELOC_8", ELF::R_V810_8)
                 .Case("BFD_RELOC_16", ELF::R_V810_16)
                 .Case("BFD_RELOC_32", ELF::R_V810_32)
                 .Default(-1u);
      if (Type == -1u)
        return std::nullopt;
      return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
    }

    MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override {
      const static MCFixupKindInfo Infos[V810::NumTargetFixupKinds] = {
        // name                  offset bits  flags
        { "fixup_v810_lo",       16,    16,   0 },
        { "fixup_v810_hi",       16,    16,   0 },
        { "fixup_v810_sdaoff",   16,    16,   0 },
        { "fixup_v810_9_pcrel",  0,     16,   0 },
        { "fixup_v810_26_pcrel", 0,     32,   0 }
      };

      if (mc::isRelocation(Kind))
        return MCAsmBackend::getFixupKindInfo(FK_NONE);
      if (Kind < FirstTargetFixupKind)
        return MCAsmBackend::getFixupKindInfo(Kind);
      assert(unsigned(Kind - FirstTargetFixupKind) < V810::NumTargetFixupKinds &&
             "Invalid kind!");
      return Infos[Kind - FirstTargetFixupKind];
    }

    void applyFixup(const MCFragment &F, const MCFixup &Fixup,
                    const MCValue &Target, uint8_t *Data,
                    uint64_t Value, bool IsResolved) override {
      maybeAddReloc(F, Fixup, Target, Value, IsResolved);
      if (!IsResolved)
        return;
      Value = adjustFixupValue(Fixup, Value, getContext());
      if (!Value) return; // Doesn't change encoding
      
      MCFixupKindInfo Info = getFixupKindInfo(Fixup.getKind());

      // The number of bits in the fixup mask
      auto NumBits = Info.TargetSize + Info.TargetOffset;
      auto NumBytes = (NumBits / 8) + ((NumBits % 8) == 0 ? 0 : 1);

      // Shift the value into position.
      Value <<= Info.TargetOffset;

      // For each byte of the fragment that the fixup touches, mask in the
      // bits from the fixup value.
      for (int i = 0; i < NumBytes; ++i) {
        uint8_t mask = (((Value >> (i * 8)) & 0xff));
        Data[i] |= mask;
      }
    }

    bool writeNopData(raw_ostream &OS, uint64_t Count,
                      const MCSubtargetInfo *STI) const override {
      // Cannot emit NOP with size not multiple of 16 bits.
      if (Count % 2 != 0) {
        return false;
      }

      uint64_t NumNops = Count / 2;
      for (uint64_t i = 0; i != NumNops; ++i) {
        // all 0s is MOV r0 r0, which takes one cycle and does nothing
        support::endian::write<uint16_t>(OS, 0x0000, endianness::little);
      }
      return true;
    }

    std::unique_ptr<MCObjectTargetWriter>
    createObjectTargetWriter() const override {
      uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
      return createV810ObjectWriter(OSABI);
    }
  };

} // end anonymous namespace

MCAsmBackend *llvm::createV810AsmBackend(const Target &T,
                                         const MCSubtargetInfo &STI,
                                         const MCRegisterInfo &MRI,
                                         const MCTargetOptions &Options) {
  return new V810AsmBackend(STI.getTargetTriple().getOS());
}