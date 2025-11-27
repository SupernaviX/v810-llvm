#include "MCTargetDesc/V810MCExpr.h"
#include "V810.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

static MCOperand LowerSymbolOperand(const MachineOperand &MO,
                                    const MCSymbol *Symbol,
                                    const int64_t Offset,
                                    AsmPrinter &AP) {
  V810MCExpr::VariantKind Kind =
    (V810MCExpr::VariantKind)MO.getTargetFlags();

  const MCExpr *InnerExpr = MCSymbolRefExpr::create(Symbol,
                                                    AP.OutContext);
  if (Offset != 0) {
    const MCExpr *OffsetExpr = MCConstantExpr::create(Offset, AP.OutContext);
    InnerExpr = MCBinaryExpr::create(MCBinaryExpr::Add, InnerExpr,
                                     OffsetExpr, AP.OutContext);
  }

  const V810MCExpr *expr = V810MCExpr::create(Kind, InnerExpr,
                                              AP.OutContext);
  return MCOperand::createExpr(expr);
}

static MCOperand LowerGlobalOperand(const MachineInstr *MI,
                                    const MachineOperand &MO,
                                    AsmPrinter &AP) {
  const MCSymbol *Symbol = AP.getSymbol(MO.getGlobal());
  return LowerSymbolOperand(MO, Symbol, MO.getOffset(), AP);
}

static MCOperand LowerExternalSymbol(const MachineInstr *MI,
                                     const MachineOperand &MO,
                                     AsmPrinter &AP) {
  const MCSymbol *Symbol = AP.GetExternalSymbolSymbol(MO.getSymbolName());
  return LowerSymbolOperand(MO, Symbol, MO.getOffset(), AP);
}

static MCOperand LowerBlockAddress(const MachineInstr *MI,
                                   const MachineOperand &MO,
                                   AsmPrinter &AP) {
  const MCSymbol *Symbol = AP.GetBlockAddressSymbol(MO.getBlockAddress());
  return LowerSymbolOperand(MO, Symbol, MO.getOffset(), AP);
}

static MCOperand LowerConstantPoolIndex(const MachineInstr *MI,
                                          const MachineOperand &MO,
                                          AsmPrinter &AP) {
  const MCSymbol *Symbol = AP.GetCPISymbol(MO.getIndex());
  return LowerSymbolOperand(MO, Symbol, MO.getOffset(), AP);
}

static MCOperand LowerMBBAddress(const MachineInstr *MI,
                                   const MachineOperand &MO,
                                   AsmPrinter &AP) {
  const MCSymbol *Symbol = MO.getMBB()->getSymbol();
  return LowerSymbolOperand(MO, Symbol, 0, AP);
}

static MCOperand LowerOperand(const MachineInstr *MI,
                              const MachineOperand &MO,
                              AsmPrinter &AP) {
  switch (MO.getType()) {
  default: llvm_unreachable("unknown operand type");
  case MachineOperand::MO_Register:
    if (MO.isImplicit())
      break;
    return MCOperand::createReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm());
  case MachineOperand::MO_MCSymbol:
    return LowerSymbolOperand(MO, MO.getMCSymbol(), MO.getOffset(), AP);
  case MachineOperand::MO_GlobalAddress:
    return LowerGlobalOperand(MI, MO, AP);
  case MachineOperand::MO_ExternalSymbol:
    return LowerExternalSymbol(MI, MO, AP);
  case MachineOperand::MO_BlockAddress:
    return LowerBlockAddress(MI, MO, AP);
  case MachineOperand::MO_ConstantPoolIndex:
    return LowerConstantPoolIndex(MI, MO, AP);
  case MachineOperand::MO_MachineBasicBlock:
    return LowerMBBAddress(MI, MO, AP);
  case MachineOperand::MO_RegisterMask:
    break;
  }

  return MCOperand();
}

void llvm::LowerV810MachineInstrToMCInst(const MachineInstr *MI,
                                         MCInst &OutMI,
                                         AsmPrinter &AP)
{
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp = LowerOperand(MI, MO, AP);

    if (MCOp.isValid()) {
      OutMI.addOperand(MCOp);
    }
  }
}