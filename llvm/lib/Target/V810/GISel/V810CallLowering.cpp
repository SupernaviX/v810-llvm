#include "V810CallLowering.h"
#include "V810CallingConv.h"
#include "V810ISelLowering.h"
#include "V810Subtarget.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFrameInfo.h"

#define DEBUG_TYPE "v810-call-lowering"

using namespace llvm;

namespace {
struct OutgoingArgHandler : public CallLowering::OutgoingValueHandler {
  OutgoingArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI,
                     MachineInstrBuilder MIB)
      : OutgoingValueHandler(MIRBuilder, MRI), MIB(MIB) {}

  void assignValueToReg(Register ValVReg, Register PhysReg,
                        const CCValAssign &VA) override;
  void assignValueToAddress(Register ValVReg, Register Addr, LLT MemTy,
                            const MachinePointerInfo &MPO,
                            const CCValAssign &VA) override;
  Register getStackAddress(uint64_t Size, int64_t Offset,
                           MachinePointerInfo &MPO,
                           ISD::ArgFlagsTy Flags) override;

  MachineInstrBuilder MIB;
};
} // namespace

void OutgoingArgHandler::assignValueToReg(Register ValVReg, Register PhysReg,
                                          const CCValAssign &VA) {
  MIB.addUse(PhysReg, RegState::Implicit);
  Register ExtReg = extendRegister(ValVReg, VA);
  MIRBuilder.buildCopy(PhysReg, ExtReg);
}

void OutgoingArgHandler::assignValueToAddress(Register ValVReg, Register Addr,
                                              LLT MemTy,
                                              const MachinePointerInfo &MPO,
                                              const CCValAssign &VA) {
  llvm_unreachable("unimplemented");
}

Register OutgoingArgHandler::getStackAddress(uint64_t Size, int64_t Offset,
                                             MachinePointerInfo &MPO,
                                             ISD::ArgFlagsTy Flags) {
  llvm_unreachable("unimplemented");
}

V810CallLowering::V810CallLowering(const V810TargetLowering &TLI)
    : CallLowering(&TLI) {}

bool V810CallLowering::canLowerReturn(MachineFunction &MF,
                                      CallingConv::ID CallConv,
                                      SmallVectorImpl<BaseArgInfo> &Outs,
                                      bool IsVarArg) const {
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs,
                     MF.getFunction().getContext());
  return checkReturn(CCInfo, Outs, RetCC_V810);
}

bool V810CallLowering::lowerReturn(MachineIRBuilder &MIRBuilder,
                                   const Value *Val, ArrayRef<Register> VRegs,
                                   FunctionLoweringInfo &FLI,
                                   Register SwiftErrorVReg) const {
  bool Success = true;
  MachineFunction &MF = MIRBuilder.getMF();
  const Function &F = MF.getFunction();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  auto &DL = F.getDataLayout();

  bool IsInterrupt = MF.getFunction().hasFnAttribute("interrupt");
  unsigned RetOp = IsInterrupt ? V810::RETI : V810::RET;
  auto MIB = MIRBuilder.buildInstrNoInsert(RetOp);

  if (!FLI.CanLowerReturn) {
    insertSRetStores(MIRBuilder, Val->getType(), VRegs, FLI.DemoteRegister);
  } else if (!VRegs.empty()) {
    ArgInfo OrigArg{VRegs, Val->getType(), 0};
    setArgFlags(OrigArg, AttributeList::ReturnIndex, DL, F);
    SmallVector<ArgInfo, 8> SplitArgs;
    splitToValueTypes(OrigArg, SplitArgs, DL, F.getCallingConv());
    OutgoingValueAssigner ArgAssigner(RetCC_V810);
    OutgoingArgHandler ArgHandler(MIRBuilder, MRI, MIB);
    Success = determineAndHandleAssignments(ArgHandler, ArgAssigner, SplitArgs,
                                            MIRBuilder, F.getCallingConv(),
                                            F.isVarArg());
  }
  MIRBuilder.insertInstr(MIB);
  return Success;
}

bool V810CallLowering::lowerFormalArguments(MachineIRBuilder &MIRBuilder,
                                            const Function &F,
                                            ArrayRef<ArrayRef<Register>> VRegs,
                                            FunctionLoweringInfo &FLI) const {
  MachineFunction &MF = MIRBuilder.getMF();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const auto &DL = F.getDataLayout();

  SmallVector<ArgInfo, 8> SplitArgs;
  unsigned i = 0;

  if (!FLI.CanLowerReturn)
    insertSRetIncomingArgument(F, SplitArgs, FLI.DemoteRegister, MRI, DL);

  for (const auto &Arg : F.args()) {
    ArgInfo OrigArg{VRegs[i], Arg, i};
    setArgFlags(OrigArg, i + AttributeList::FirstArgIndex, DL, F);
    splitToValueTypes(OrigArg, SplitArgs, DL, F.getCallingConv());
    ++i;
  }

  IncomingValueAssigner ArgAssigner(CC_V810);
  FormalArgHandler ArgHandler(MIRBuilder, MRI);
  return determineAndHandleAssignments(ArgHandler, ArgAssigner, SplitArgs,
                                       MIRBuilder, F.getCallingConv(),
                                       F.isVarArg());
}

bool V810CallLowering::isEligibleForTailCallOptimization(
    MachineIRBuilder &MIRBuilder, CCState &CCInfo,
    SmallVectorImpl<CCValAssign> &ArgLocs, SmallVectorImpl<ArgInfo> &OutArgs) const {
  if (MIRBuilder.getMF().getFunction().hasFnAttribute("interrupt"))
    return false;
  if (CCInfo.getStackSize() > 0)
    return false; // can't tail call if we pass things on the stack

  // Do not tail call opt if any parameters need to be passed indirectly.
  for (auto &VA : ArgLocs)
    if (VA.getLocInfo() == CCValAssign::Indirect)
      return false;

  // Do not tail call if any parameters are passed `byval`, which actually
  // passes the value via the stack.
  // This bug was observed when passing a bitfield struct to a function with C
  // linkage.
  for (auto &Arg : OutArgs)
    if (Arg.Flags[0].isByVal())
      return false;

  return true;
}

bool V810CallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                                 CallLoweringInfo &Info) const {
  MachineFunction &MF = MIRBuilder.getMF();
  const Function &F = MF.getFunction();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const DataLayout &DL = F.getDataLayout();
  const V810Subtarget &Subtarget =
      MIRBuilder.getMF().getSubtarget<V810Subtarget>();
  
  SmallVector<ArgInfo, 8> OutArgs;
  for (auto &OrigArg : Info.OrigArgs)
    splitToValueTypes(OrigArg, OutArgs, DL, Info.CallConv);

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(Info.CallConv, Info.IsVarArg, MF, ArgLocs, F.getContext());

  OutgoingValueAssigner ArgAssigner(CC_V810);
  if (!determineAssignments(ArgAssigner, OutArgs, CCInfo))
    return false;

  Info.IsTailCall = Info.IsTailCall &&
      isEligibleForTailCallOptimization(MIRBuilder, CCInfo, ArgLocs, OutArgs);

  if (!Info.IsTailCall & Info.IsMustTailCall) {
    LLVM_DEBUG(dbgs() << "Failed to lower musttail call as tail call\n");
    return false;
  }

  // For non-tail calls, adjust the call stack first
  if (!Info.IsTailCall)
    MIRBuilder.buildInstr(V810::ADJCALLSTACKDOWN)
      .addImm(ArgAssigner.StackSize)
      .addImm(0);

  // Build a call instruction. We use different instructions based on whether
  // it's a normal or tail call, and whether the function is "indirect" (determined at runtime).
  // All of these opcodes are either copies of a real opcode, or pseudos to be expanded later.
  unsigned CallOpcode;
  if (Info.Callee.isReg()) {
    CallOpcode = Info.IsTailCall ? V810::TAIL_CALL_INDIRECT : V810::TAIL_CALL;
  } else {
    CallOpcode = Info.IsTailCall ? V810::TAIL_CALL : V810::CALL;
  }
  MachineInstrBuilder Call = MIRBuilder.buildInstrNoInsert(CallOpcode).add(Info.Callee);

  // Pass the call preserved mask
  const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
  Call.addRegMask(TRI->getCallPreservedMask(MF, Info.CallConv));

  // Then pass all the arguments
  OutgoingArgHandler Handler(MIRBuilder, MRI, Call);
  if (!handleAssignments(Handler, OutArgs, CCInfo, ArgLocs, MIRBuilder))
    return false;

  MIRBuilder.insertInstr(Call);

  // If we made a tail call, we're already done!
  if (Info.IsTailCall) return true;

  // For normal functions, we have to read values from the right registers/addresses
  if (Info.CanLowerReturn && !Info.OrigRet.Ty->isVoidTy()) {
    SmallVector<ArgInfo, 8> InArgs;
    splitToValueTypes(Info.OrigRet, InArgs, DL, Info.CallConv);

    IncomingValueAssigner RetAssigner(RetCC_V810);
    CallReturnHandler RetHandler(MIRBuilder, MRI, Call);
    if (!determineAndHandleAssignments(RetHandler, RetAssigner, InArgs, MIRBuilder,
                                       Info.CallConv, Info.IsVarArg))
      return false;

  }

  // And then fix the stack
  MIRBuilder.buildInstr(V810::ADJCALLSTACKUP)
    .addImm(ArgAssigner.StackSize)
    .addImm(0);

  // oh also, if we couldn't handle returning whatever value, insert an SRET load for it
  if (!Info.CanLowerReturn) {
    insertSRetLoads(MIRBuilder, Info.OrigRet.Ty, Info.OrigRet.Regs,
                    Info.DemoteRegister, Info.DemoteStackIndex);
  }

  return true;
}

void V810IncomingValueHandler::assignValueToReg(Register ValVReg,
                                        Register PhysReg,
                                        const CCValAssign &VA) {
  markPhysRegUsed(PhysReg);
  IncomingValueHandler::assignValueToReg(ValVReg, PhysReg, VA);
}

void V810IncomingValueHandler::assignValueToAddress(
    Register ValVReg, Register Addr, LLT MemTy, const MachinePointerInfo &MPO,
    const CCValAssign &VA) {
  MachineFunction &MF = MIRBuilder.getMF();
  auto *MMO = MF.getMachineMemOperand(MPO, MachineMemOperand::MOLoad, MemTy,
                                      inferAlignFromPtrInfo(MF, MPO));
  MIRBuilder.buildLoad(ValVReg, Addr, *MMO);
}

Register V810IncomingValueHandler::getStackAddress(
    uint64_t Size, int64_t Offset,
    MachinePointerInfo &MPO, ISD::ArgFlagsTy Flags) {
  MachineFunction &MF = MIRBuilder.getMF();
  const bool IsImmutable = !Flags.isByVal();
  int FI = MF.getFrameInfo().CreateFixedObject(Size, Offset, IsImmutable);
  MPO = MachinePointerInfo::getFixedStack(MF, FI);

  LLT FramePtr = LLT::pointer(0, MF.getDataLayout().getPointerSizeInBits());
  MachineInstrBuilder AddrReg = MIRBuilder.buildFrameIndex(FramePtr, FI);
  return AddrReg.getReg(0);
}

void FormalArgHandler::markPhysRegUsed(unsigned PhysReg) {
  MIRBuilder.getMBB().addLiveIn(PhysReg);
}

void CallReturnHandler::markPhysRegUsed(unsigned PhysReg) {
  MIB.addDef(PhysReg, RegState::Implicit);
}