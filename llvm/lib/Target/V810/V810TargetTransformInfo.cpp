#include "V810TargetTransformInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
using namespace llvm;

#define DEBUG_TYPE "v810tti"

void V810TTIImpl::getUnrollingPreferences(Loop *L, ScalarEvolution &SE,
                                          TTI::UnrollingPreferences &UP,
                                          OptimizationRemarkEmitter *ORE) const {
  // Only unroll loops with VERY small bodies
  UP.Threshold = 3;
}

InstructionCost V810TTIImpl::getCmpSelInstrCost(unsigned Opcode, Type *ValTy,
                                                 Type *CondTy,
                                                 CmpInst::Predicate VecPred,
                                                 TTI::TargetCostKind CostKind,
                                                 TTI::OperandValueInfo Op1Info,
                                                 TTI::OperandValueInfo Op2Info,
                                                 const Instruction *I) const {
  switch (Opcode) {
  case Instruction::ICmp:
    return 1;
  case Instruction::FCmp:
    return 10;
  case Instruction::Select:
    // This is compiled into an awkward set of jumps
    return 20;
  default:
    llvm_unreachable("unknown instruction");
  }
}

InstructionCost V810TTIImpl::getIntImmCost(const APInt &Imm, Type *Ty,
                              TTI::TargetCostKind CostKind) const {
  // Zero register is always free
  if (Imm == 0)
    return TTI::TCC_Free;
  // Can be loaded in a two-byte MOV
  if (Imm.isSignedIntN(5))
    return TTI::TCC_Basic;
  // Can be loaded in a four-byte MOVEA
  if (Imm.isSignedIntN(16))
    return TTI::TCC_Basic;
  // Can be loaded in a four-byte ORI
  if (Imm.isIntN(16))
    return TTI::TCC_Basic;
  // Can be loaded in a four-byte MOVHI
  if (Imm.isSingleWord() && isShiftedInt<16, 16>(Imm.getSExtValue()))
    return TTI::TCC_Basic;
  // gotta be an 8-byte MOVHI/MOVEA pair
  return TTI::TCC_Expensive;
}

InstructionCost V810TTIImpl::getIntImmCostInst(unsigned Opcode, unsigned Idx,
                                               const APInt &Imm, Type *Ty,
                                               TTI::TargetCostKind CostKind,
                                               Instruction *Inst) const {
  if (Imm == 0)
    return TTI::TCC_Free;

  // Check if the immediate fits into the instruction
  switch (Opcode) {
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
    if (Imm.isIntN(5))
      return TTI::TCC_Free;
    break;
  case Instruction::ICmp:
    if (Imm.isSignedIntN(5))
      return TTI::TCC_Free;
    break;
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor:
    if (Imm.isIntN(16))
      return TTI::TCC_Free;
    break;
  case Instruction::Add:
    if (Imm.isSignedIntN(16))
      return TTI::TCC_Free;
    break;
  }

  // We have to load it into an immediate
  return getIntImmCost(Imm, Ty, CostKind);
}

InstructionCost V810TTIImpl::getIntImmCostIntrin(Intrinsic::ID IID, unsigned Idx,
                                                 const APInt &Imm, Type *Ty,
                                                 TTI::TargetCostKind CostKind) const {
  // no hoist plz
  return TTI::TCC_Free;
}

InstructionCost V810TTIImpl::getArithmeticInstrCost(
    unsigned Opcode, Type *Ty, TTI::TargetCostKind CostKind,
    TTI::OperandValueInfo Op1Info, TTI::OperandValueInfo Op2Info,
    ArrayRef<const Value *> Args, const Instruction *CxtI) const {
  switch (Opcode) {
  // Floats
  case Instruction::FDiv:
  case Instruction::FRem:
    return 44;
  case Instruction::FMul:
    return 30;
  case Instruction::FSub:
  case Instruction::FAdd:
    return 28;
  case Instruction::FPToSI:
    return 14;
  case Instruction::SIToFP:
    return 16;
  case Instruction::FPToUI:
  case Instruction::UIToFP:
    return 100; // who knows what those libcalls do

  // Ints
  case Instruction::SDiv:
  case Instruction::SRem:
    return 38;
  case Instruction::UDiv:
  case Instruction::URem:
    return 36;
  case Instruction::Mul:
    return 13;

  default:
    return 1;
  }
}