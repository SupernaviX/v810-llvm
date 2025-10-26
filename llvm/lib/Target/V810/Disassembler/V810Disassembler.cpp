#include "MCTargetDesc/V810MCTargetDesc.h"
#include "TargetInfo/V810TargetInfo.h"
#include "llvm/MC/MCDecoder.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::MCD;

#define DEBUG_TYPE "v810-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

class V810Disassembler : public MCDisassembler {
public:
  V810Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  virtual ~V810Disassembler() = default;

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
  uint64_t suggestBytesToSkip(ArrayRef<uint8_t> Bytes,
                              uint64_t Address) const override;
};
}

static MCDisassembler *createV810Disassembler(const Target &T,
                                              const MCSubtargetInfo &STI,
                                              MCContext &Ctx) {
  return new V810Disassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810Disassembler() {
  TargetRegistry::RegisterMCDisassembler(getTheV810Target(),
                                         createV810Disassembler);
}

static const unsigned GenRegDecoderTable[] = {
  V810::R0,   V810::R1,   V810::R2,   V810::R3,
  V810::R4,   V810::R5,   V810::R6,   V810::R7,
  V810::R8,   V810::R9,   V810::R10,  V810::R11,
  V810::R12,  V810::R13,  V810::R14,  V810::R15,
  V810::R16,  V810::R17,  V810::R18,  V810::R19,
  V810::R20,  V810::R21,  V810::R22,  V810::R23,
  V810::R24,  V810::R25,  V810::R26,  V810::R27,
  V810::R28,  V810::R29,  V810::R30,  V810::R31 };

static const unsigned SysRegDecoderTable[] = {
  V810::SR0,  V810::SR1,  V810::SR2,  V810::SR3,
  V810::SR4,  V810::SR5,  V810::SR6,  V810::SR7,
  0,          0,          0,          0,
  0,          0,          0,          0,
  0,          0,          0,          0,
  0,          0,          0,          0,
  V810::SR24, V810::SR25, 0,          0,
  0,          V810::SR29, V810::SR30, V810::SR31 };

static DecodeStatus DecodeGenRegsRegisterClass(MCInst &Inst, unsigned RegNo,
                                               uint64_t Address,
                                               const MCDisassembler *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;
  unsigned Reg = GenRegDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeClobberRegisterClass(MCInst &Inst, const MCDisassembler *Decoder) {
  Inst.addOperand(MCOperand::createReg(V810::R30));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeSysRegsRegisterClass(MCInst &Inst, unsigned RegNo,
                                               uint64_t Address,
                                               const MCDisassembler *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;
  unsigned Reg = SysRegDecoderTable[RegNo];
  if (Reg == 0)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus DecodeSIMM(MCInst &Inst, unsigned insn, uint64_t Address,
                               const MCDisassembler *Decoder);
template <unsigned N>
static DecodeStatus DecodeUIMM(MCInst &Inst, unsigned insn, uint64_t Address,
                               const MCDisassembler *Decoder);

static DecodeStatus DecodeCall(MCInst &Inst, unsigned insn, uint64_t Address,
                               const MCDisassembler *Decoder);

#include "V810GenDisassemblerTables.inc"

uint64_t V810Disassembler::suggestBytesToSkip(ArrayRef<uint8_t> Bytes,
                                              uint64_t Address) const {
  return 2;
}

DecodeStatus V810Disassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                              ArrayRef<uint8_t> Bytes,
                                              uint64_t Address,
                                              raw_ostream &CStream) const {
  CommentStream = &CStream;
  
  if (Bytes.size() < 2) {
    Size = 0;
    return DecodeStatus::Fail;
  }
  uint16_t SmallInsn = support::endian::read16le(Bytes.data());
  DecodeStatus Result = decodeInstruction(DecoderTableV81016, Instr, SmallInsn, Address,
                                          this, STI);
  if (Result != DecodeStatus::Fail) {
    Size = 2;
    return Result;
  }

  if (Bytes.size() < 4) {
    Size = 0;
    return DecodeStatus::Fail;
  }
  uint32_t LargeInsn = support::endian::read32le(Bytes.data());
  Result = decodeInstruction(DecoderTableV81032, Instr, LargeInsn, Address,
                             this, STI);
  if (Result != DecodeStatus::Fail) {
    Size = 4;
    return Result;
  }

  return DecodeStatus::Fail;
}

template <unsigned N>
static DecodeStatus DecodeSIMM(MCInst &MI, unsigned insn, uint64_t Address,
                               const MCDisassembler *Decoder) {
  int64_t tgt = SignExtend64<N>(insn);
  MI.addOperand(MCOperand::createImm(tgt));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus DecodeUIMM(MCInst &MI, unsigned insn, uint64_t Address,
                               const MCDisassembler *Decoder) {
  uint64_t tgt = insn & maskTrailingOnes<uint64_t>(N);
  MI.addOperand(MCOperand::createImm(tgt));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeCall(MCInst &MI, unsigned insn, uint64_t Address,
                                const MCDisassembler *Decoder) {
  int64_t tgt = SignExtend64<26>(insn);
  uint64_t CalleeAddress = Address + tgt;
  if (!Decoder->tryAddingSymbolicOperand(MI, CalleeAddress, false, Address, 0, 0, 32))
    MI.addOperand(MCOperand::createImm(tgt));
  return MCDisassembler::Success;
}