#include "V810FixupKinds.h"
#include "V810MCExpr.h"
#include "V810MCTargetDesc.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

namespace {
class V810MCCodeEmitter : public MCCodeEmitter {
private:
  const MCInstrInfo &MCII;
  MCContext &Ctx;

public:
  V810MCCodeEmitter(const MCInstrInfo &MCII, MCContext &ctx)
      : MCII(MCII), Ctx(ctx) {}

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// getMachineOpValue - Return binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups,
                                  const MCSubtargetInfo &STI) const;
  unsigned getBcondTargetOpValue(const MCInst &MI, unsigned OpNo,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;
};

} // end anonymous namespace

void V810MCCodeEmitter::encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());

  unsigned Size = Desc.getSize();
  unsigned Bits = getBinaryCodeForInstr(MI, Fixups, STI);

  switch (Size) {
  default: llvm_unreachable("Instruction is missing size");
  case 2:
    support::endian::write(CB, (uint16_t) Bits, endianness::little);
    break;
  case 4:
    support::endian::write(CB, (uint32_t) Bits, endianness::little);
    break;
  }
}

static void addFixup(SmallVectorImpl<MCFixup> &Fixups, uint32_t Offset,
                     const MCExpr *Value, uint16_t Kind) {
  bool PCRel = false;
  switch (Kind) {
    case V810::fixup_v810_9_pcrel:
    case V810::fixup_v810_26_pcrel:
      PCRel = true;
  }
  Fixups.push_back(MCFixup::create(Offset, Value, Kind, PCRel));
}

unsigned V810MCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups,
                  const MCSubtargetInfo &STI) const {
  if (MO.isReg()) {
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
  }

  if (MO.isImm()) {
    return MO.getImm();
  }

  assert(MO.isExpr());
  const MCExpr *Expr = MO.getExpr();
  if (const V810MCExpr *VExpr = dyn_cast<V810MCExpr>(Expr)) {
    MCFixupKind Kind = (MCFixupKind)VExpr->getFixupKind();
    addFixup(Fixups, 0, Expr, Kind);
    return 0;
  }

  int64_t Res;
  if (Expr->evaluateAsAbsolute(Res))
    return Res;

  llvm_unreachable("Unhandled expression!");
  return 0;
}

unsigned V810MCCodeEmitter::
getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                       SmallVectorImpl<MCFixup> &Fixups,
                       const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);
  
  addFixup(Fixups, 0, MO.getExpr(), V810::fixup_v810_26_pcrel);
  return 0;
}

unsigned V810MCCodeEmitter::
getBcondTargetOpValue(const MCInst &MI, unsigned OpNo,
                      SmallVectorImpl<MCFixup> &Fixups,
                      const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);
  
  addFixup(Fixups, 0, MO.getExpr(), V810::fixup_v810_9_pcrel);
  return 0;
}

#include "V810GenMCCodeEmitter.inc"

MCCodeEmitter *llvm::createV810MCCodeEmitter(const MCInstrInfo &MCII,
                                             MCContext &Ctx) {
  return new V810MCCodeEmitter(MCII, Ctx);
}