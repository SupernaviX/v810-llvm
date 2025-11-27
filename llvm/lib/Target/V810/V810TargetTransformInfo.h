#ifndef LLVM_LIB_TARGET_V810_V810TARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_V810_V810TARGETTRANSFORMINFO_H

#include "V810Subtarget.h"
#include "V810TargetMachine.h"
#include "llvm/CodeGen/BasicTTIImpl.h"

namespace llvm {
class V810TTIImpl : public BasicTTIImplBase<V810TTIImpl> {
  using BaseT = BasicTTIImplBase<V810TTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  const V810Subtarget *ST;
  const V810TargetLowering *TLI;

  const V810Subtarget *getST() const { return ST; }
  const V810TargetLowering *getTLI() const { return TLI; }

public:
  explicit V810TTIImpl(const V810TargetMachine *TM, const Function &F)
    : BaseT(TM, F.getParent()->getDataLayout()), ST(TM->getSubtargetImpl(F)),
      TLI(ST->getTargetLowering()) {}

  void getUnrollingPreferences(Loop *L, ScalarEvolution &SE,
                               TTI::UnrollingPreferences &UP,
                               OptimizationRemarkEmitter *ORE) const override;

  InstructionCost getCmpSelInstrCost(unsigned Opcode, Type *ValTy, Type *CondTy,
                                     CmpInst::Predicate VecPred,
                                     TTI::TargetCostKind CostKind,
                                     TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
                                     TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
                                     const Instruction *I = nullptr) const override;

  InstructionCost getIntImmCost(const APInt &Imm, Type *Ty,
                                TTI::TargetCostKind CostKind) const override;
  InstructionCost getIntImmCostInst(unsigned Opcode, unsigned Idx, const APInt &Imm, Type *Ty,
                                    TTI::TargetCostKind CostKind, Instruction *Inst = nullptr) const override;
  InstructionCost getIntImmCostIntrin(Intrinsic::ID IID, unsigned Idx,
                                      const APInt &Imm, Type *Ty,
                                      TTI::TargetCostKind CostKind) const override;
  InstructionCost getArithmeticInstrCost(
      unsigned Opcode, Type *Ty, TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
      TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
      ArrayRef<const Value *> Args = ArrayRef<const Value *>(),
      const Instruction *CxtI = nullptr) const override;
};
}

#endif