#ifndef LLVM_LIB_TARGET_V810_GISEL_V810CALLLOWERING_H
#define LLVM_LIB_TARGET_V810_GISEL_V810CALLLOWERING_H

#include "V810ISelLowering.h"
#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/CallingConv.h"

namespace llvm {

class V810TargetLowering;

class V810CallLowering : public CallLowering {
public:
  V810CallLowering(const V810TargetLowering &TLI);

  bool canLowerReturn(MachineFunction &MF, CallingConv::ID CallConv,
                      SmallVectorImpl<BaseArgInfo> &Outs,
                      bool IsVarArg) const override;
  bool lowerReturn(MachineIRBuilder &MIRBuilder, const Value *Val,
                   ArrayRef<Register> VRegs, FunctionLoweringInfo &FLI,
                   Register SwiftErrorVReg) const override;
  bool lowerFormalArguments(MachineIRBuilder &MIRBuilder, const Function &F,
                            ArrayRef<ArrayRef<Register>> VRegs,
                            FunctionLoweringInfo &FLI) const override;
  bool lowerCall(MachineIRBuilder &MIRBuilder,
                 CallLoweringInfo &Info) const override;
  bool
  isEligibleForTailCallOptimization(MachineIRBuilder &MIRBuilder,
                                    CCState &CCInfo,
                                    SmallVectorImpl<CCValAssign> &ArgLocs,
                                    SmallVectorImpl<ArgInfo> &OutArgs) const;
private:
  bool doDetermineAndHandleAssignments(
    ValueHandler &Handler, ValueAssigner &Assigner,
    SmallVectorImpl<ArgInfo> &Args, MachineIRBuilder &MIRBuilder,
    CallingConv::ID CallConv, bool IsVarArg, unsigned NumFixedParams) const;
};

class V810IncomingValueHandler : public CallLowering::IncomingValueHandler {
public:
  V810IncomingValueHandler(MachineIRBuilder &MIRBuilder,
                           MachineRegisterInfo &MRI)
      : CallLowering::IncomingValueHandler(MIRBuilder, MRI) {}

private:
  void assignValueToReg(Register ValVReg, Register PhysReg,
                        const CCValAssign &VA) override;
  
  void assignValueToAddress(Register ValueAndVReg, Register Addr, LLT MemTy,
                            const MachinePointerInfo &MPO,
                            const CCValAssign &VA) override;

  Register getStackAddress(uint64_t Size, int64_t Offset,
                           MachinePointerInfo &MPO,
                           ISD::ArgFlagsTy Flags) override;

  virtual void markPhysRegUsed(unsigned PhysReg) = 0;
};

class FormalArgHandler : public V810IncomingValueHandler {

  void markPhysRegUsed(unsigned PhysReg) override;

public:
  FormalArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI)
      : V810IncomingValueHandler(MIRBuilder, MRI) {}
};

class CallReturnHandler : public V810IncomingValueHandler {

  void markPhysRegUsed(unsigned PhysReg) override;

  MachineInstrBuilder MIB;

public:
  CallReturnHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI,
                    MachineInstrBuilder MIB)
      : V810IncomingValueHandler(MIRBuilder, MRI), MIB(MIB) {}
};

} // end namespace llvm

#endif