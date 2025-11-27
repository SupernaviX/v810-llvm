#include "MCTargetDesc/V810InstPrinter.h"
#include "MCTargetDesc/V810MCExpr.h"
#include "V810.h"
#include "V810TargetMachine.h"
#include "TargetInfo/V810TargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
  class V810AsmPrinter : public AsmPrinter {
  public:
    explicit V810AsmPrinter(TargetMachine &TM,
                            std::unique_ptr<MCStreamer> Streamer)
        : AsmPrinter(TM, std::move(Streamer)) {}
    StringRef getPassName() const override { return "V810 Assembly Printer"; }

    void emitInstruction(const MachineInstr *MI) override;

    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                         const char *ExtraCode, raw_ostream &O) override;
    bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                               const char *ExtraCode, raw_ostream &O) override;
    void LowerCallIndirect(const MachineInstr *MI);
  private:
    void printOperandImpl(const MachineInstr *MI, unsigned OpNo, raw_ostream &O);
  };
} // end of anonymous namespace

void V810AsmPrinter::emitInstruction(const MachineInstr *MI) {
  V810_MC::verifyInstructionPredicates(MI->getOpcode(),
                                       getSubtargetInfo().getFeatureBits());

  if (MI->getOpcode() == V810::CALL_INDIRECT) {
    LowerCallIndirect(MI);
    return;
  }

  MCInst TmpInst;
  LowerV810MachineInstrToMCInst(MI, TmpInst, *this);
  EmitToStreamer(*OutStreamer, TmpInst);
}

/// PrintAsmOperand - Print out an operand for an inline asm expression.
///
bool V810AsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                     const char *ExtraCode,
                                     raw_ostream &O) {
  if (ExtraCode && ExtraCode[0]) {
    if (ExtraCode[1] != 0) return true; // Unknown modifier.

    switch (ExtraCode[0]) {
    default:
      // See if this is a generic print operand
      return AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, O);
    case 'r':
     break;
    }
  }

  printOperandImpl(MI, OpNo, O);
  return false;
}

bool V810AsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                                           const char *ExtraCode,
                                           raw_ostream &O) {
  if (ExtraCode && ExtraCode[0])
    return true; // Unknown modifier

  printOperandImpl(MI, OpNo + 1, O);
  O << '[';
  printOperandImpl(MI, OpNo, O);
  O << ']';

  return false;
}

void V810AsmPrinter::printOperandImpl(const MachineInstr *MI, unsigned OpNo,
                         raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(OpNo);
  V810MCExpr::VariantKind TF = (V810MCExpr::VariantKind) MO.getTargetFlags();

  bool Parens = V810MCExpr::printVariantKind(O, TF);
  if (Parens) O << '(';
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << StringRef(V810InstPrinter::getRegisterName(MO.getReg()));
    break;
  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    break;
  case MachineOperand::MO_GlobalAddress:
    PrintSymbolOperand(MO, O);
    break;
  case MachineOperand::MO_BlockAddress:
    O << GetBlockAddressSymbol(MO.getBlockAddress())->getName();
    break;
  case MachineOperand::MO_ExternalSymbol:
    O << MO.getSymbolName();
    break;
  default:
    errs() << "Could not print asm operand: ";
    MO.print(errs());
    llvm_unreachable("Unhandled ASM operand");
  }
  if (Parens) O << ')';
}

void V810AsmPrinter::LowerCallIndirect(const MachineInstr *MI) {
  //   jal .+4
  //   add 4, lp
  //   jmp [r?]

  assert(MI->getNumOperands() >= 1);
  assert(MI->getOperand(0).isReg());
  unsigned CalleeReg = MI->getOperand(0).getReg();

  MCInst LinkInst;
  LinkInst.setOpcode(V810::JAL);
  LinkInst.addOperand(MCOperand::createImm(4));
  OutStreamer->emitInstruction(LinkInst, getSubtargetInfo());

  MCInst AddInst;
  AddInst.setOpcode(V810::ADDri);
  AddInst.addOperand(MCOperand::createReg(V810::R31));
  AddInst.addOperand(MCOperand::createReg(V810::R31));
  AddInst.addOperand(MCOperand::createImm(4));
  OutStreamer->emitInstruction(AddInst, getSubtargetInfo());
  
  MCInst CallInst;
  CallInst.setOpcode(V810::JMP);
  CallInst.addOperand(MCOperand::createReg(CalleeReg));
  OutStreamer->emitInstruction(CallInst, getSubtargetInfo());
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810AsmPrinter() {
  RegisterAsmPrinter<V810AsmPrinter> X(getTheV810Target());
}