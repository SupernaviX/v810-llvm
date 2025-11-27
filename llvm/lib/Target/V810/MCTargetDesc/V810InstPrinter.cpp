#include "V810InstPrinter.h"
#include "V810MCAsmInfo.h"
#include "V810.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define GET_INSTRUCTION_NAME
#define PRINT_ALIAS_INSTR
#include "V810GenAsmWriter.inc"

void V810InstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) {
  OS << StringRef(getRegisterName(Reg));
}

void V810InstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &O) {
  if (!printAliasInstr(MI, Address, O)) {
    if (MI->getOpcode() == V810::Bcond)
      printBcondInstruction(MI, Address, O);
    else if (MI->getOpcode() == V810::JMP)
      printJmpInstruction(MI, Address, O);
    else
      printInstruction(MI, Address, O);
  }
  printAnnotation(O, Annot);
}

void V810InstPrinter::printBcondInstruction(const MCInst *MI, uint64_t Address,
                                            raw_ostream &O) {
  assert(MI->getNumOperands() == 2);
  int64_t cond = MI->getOperand(0).getImm();

  if (cond == V810CC::CC_NOP) {
    O << "\tnop";
  } else {
    printInstruction(MI, Address, O);
  }
}

void V810InstPrinter::printJmpInstruction(const MCInst *MI, uint64_t Address,
                                          raw_ostream &O) {
  O <<"\tjmp [";
  printOperand(MI, 0, O);
  O << "]";
}

void V810InstPrinter::printOperand(const MCInst *MI, int opNum,
                                   raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    int64_t ImmValue = MO.getImm();
    if (PrintImmHex && (MI->getOpcode() == V810::MOVHI || MI->getOpcode() == V810::MOVEA)) {
      ImmValue &= 0xffff;
    }
    O << formatImm(ImmValue);
    return;
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MAI.printExpr(O, *MO.getExpr());
}

template <unsigned N>
void V810InstPrinter::printBranchOperand(const MCInst *MI, uint64_t Address,
                                         unsigned opNum, raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);
  if (MO.isImm()) {
    int64_t Val = SignExtend64<N>(MO.getImm());
    if (PrintBranchImmAsAddress) {
      uint64_t Target = Address + Val;
      O << formatHex(Target);
    } else {
      O << ".";
      if (Val > 0) O << '+';
      O << formatImm(Val);
    }
    return;
  }

  assert(MO.isExpr() && "not expr?");
  MAI.printExpr(O, *MO.getExpr());
}

void V810InstPrinter::printMemOperand(const MCInst *MI, int opNum,
                                      raw_ostream &O) {
  if (MI->getOpcode() == V810::ADDImem) {
    // This is compiled to an ADDI, print it like "$imm, $r1"
    printOperand(MI, opNum + 1, O);
    O << ", ";
    printOperand(MI, opNum, O);
  } else {
    printOperand(MI, opNum + 1, O);
    O << "[";
    printOperand(MI, opNum, O);
    O << "]";
  }
}

void V810InstPrinter::printCondOperand(const MCInst *MI, int opNum,
                                       raw_ostream &O) {
  V810CC::CondCodes Cond = (V810CC::CondCodes)MI->getOperand(opNum).getImm();
  if (Cond & (~0x0f)) {
    O << "<invalid>";
  } else {
    O << V810CondCodeToString(Cond);
  }
}