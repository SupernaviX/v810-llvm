#include "V810ISelLowering.h"
#include "V810.h"
#include "V810MachineFunctionInfo.h"
#include "MCTargetDesc/V810MCExpr.h"
#include "V810CallingConv.h"
#include "V810RegisterInfo.h"
#include "V810Subtarget.h"
#include "V810TargetObjectFile.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/KnownBits.h"

using namespace llvm;

V810TargetLowering::V810TargetLowering(const TargetMachine &TM,
                                       const V810Subtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {

  setBooleanContents(BooleanContent::ZeroOrOneBooleanContent);  
  setMaxDivRemBitWidthSupported(32);

  // Set up the register classes.
  addRegisterClass(MVT::i32, &V810::GenRegsRegClass);
  addRegisterClass(MVT::f32, &V810::GenRegsRegClass);

  // Handle addresses specially to make constants
  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::GlobalTLSAddress, MVT::i32, Custom);
  setOperationAction(ISD::BlockAddress, MVT::i32, Custom);
  setOperationAction(ISD::ConstantPool, MVT::i32, Custom);
  // Handle branching specially
  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::BR_CC, MVT::f32, Custom);
  setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  setOperationAction(ISD::BRIND, MVT::Other, Expand);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);

  // SELECT is just a SELECT_CC with hardcoded cond, expand it to that 
  setOperationAction(ISD::SELECT, MVT::i32, Expand);
  setOperationAction(ISD::SELECT, MVT::f32, Expand);
  // Need to branch to handle SELECT_CC
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::f32, Custom);
  // SETCC reduces nicely to CMP + SETF, so do that
  setOperationAction(ISD::SETCC, MVT::i32, Custom);
  setOperationAction(ISD::SETCC, MVT::f32, Custom);

  // maths and bits
  setOperationAction(ISD::UADDO, MVT::i32, Expand);
  setOperationAction(ISD::USUBO, MVT::i32, Expand);
  setOperationAction(ISD::SADDO, MVT::i32, Expand);
  setOperationAction(ISD::SSUBO, MVT::i32, Expand);
  setOperationAction(ISD::UADDO_CARRY, MVT::i32, Expand);
  setOperationAction(ISD::USUBO_CARRY, MVT::i32, Expand);
  setOperationAction(ISD::SHL_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i32, Expand);

  // all of these expand to our native MUL_LOHI and DIVREM opcodes
  setOperationAction(ISD::SMUL_LOHI, MVT::i32, Custom);
  setOperationAction(ISD::UMUL_LOHI, MVT::i32, Custom);
  setOperationAction(ISD::MULHU, MVT::i32, Expand);
  setOperationAction(ISD::MULHS, MVT::i32, Expand);
  setOperationAction(ISD::MUL,   MVT::i32, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i32, Custom);
  setOperationAction(ISD::UDIVREM, MVT::i32, Custom);
  setOperationAction(ISD::SDIV,  MVT::i32, Expand);
  setOperationAction(ISD::UDIV,  MVT::i32, Expand);
  setOperationAction(ISD::SREM,  MVT::i32, Expand);
  setOperationAction(ISD::UREM,  MVT::i32, Expand);

  setMaxAtomicSizeInBitsSupported(32);
  setMinCmpXchgSizeInBits(32);
  setOperationAction(ISD::ATOMIC_CMP_SWAP, MVT::i32, Custom);
  setOperationAction(ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS, MVT::i32, Custom);
  setOperationAction(ISD::ATOMIC_FENCE, MVT::Other, Custom);

  setOperationAction(ISD::TRAP, MVT::Other, Legal);
  setOperationAction(ISD::DEBUGTRAP, MVT::Other, Legal);

  // Sign-extend smol types in registers with bitshifts
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);

  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAEND,   MVT::Other, Expand);
  setOperationAction(ISD::VAARG,   MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,  MVT::Other, Expand);

  setOperationAction(ISD::CTPOP, MVT::i32, Expand);
  setOperationAction(ISD::CTTZ , MVT::i32, Expand);
  setOperationAction(ISD::CTLZ , MVT::i32, Expand);
  setOperationAction(ISD::ROTL, MVT::i32, Expand);
  setOperationAction(ISD::ROTR, MVT::i32, Expand);
  // If we have native bit twiddlers, use em
  if (Subtarget->isNintendo()) {
    setOperationAction(ISD::BITREVERSE, MVT::i32, Legal);
    setOperationAction(ISD::BSWAP, MVT::i32, Legal);
  } else {
    setOperationAction(ISD::BITREVERSE, MVT::i32, Expand);
    setOperationAction(ISD::BSWAP, MVT::i32, Expand);
  }

  setOperationAction(ISD::ConstantFP, MVT::f32, Custom);
  setOperationAction(ISD::UINT_TO_FP, MVT::i32, Expand);
  setOperationAction(ISD::FP_TO_UINT, MVT::i32, Expand);
  setOperationAction(ISD::FABS, MVT::f32, Expand);
  setOperationAction(ISD::FCOS, MVT::f32, Expand);
  setOperationAction(ISD::FCOPYSIGN, MVT::f32, Expand);
  setOperationAction(ISD::FNEG, MVT::f32, Expand);
  setOperationAction(ISD::FREM, MVT::f32, Expand);
  setOperationAction(ISD::FSIN, MVT::f32, Expand);
  setOperationAction(ISD::FSQRT, MVT::f32, Expand);

  setLoadExtAction(ISD::EXTLOAD, MVT::f32, MVT::f16, Expand);
  setTruncStoreAction(MVT::f32, MVT::f16, Expand);
  setOperationAction(ISD::FP16_TO_FP, MVT::f32, Expand);
  setOperationAction(ISD::FP_TO_FP16, MVT::f32, Expand);

  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32, Expand);

  setStackPointerRegisterToSaveRestore(V810::R3);

  setMinStackArgumentAlignment(Align(4));

  setTargetDAGCombine({ISD::LOAD, ISD::STORE});

  MaxStoresPerMemcpy = 3;

  computeRegisterProperties(Subtarget->getRegisterInfo());
}

const char *V810TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((V810ISD::NodeType)Opcode) {
  case V810ISD::FIRST_NUMBER: break;
  case V810ISD::HI:           return "V810ISD::HI";
  case V810ISD::LO:           return "V810ISD::LO";
  case V810ISD::REG_RELATIVE: return "V810ISD::REG_RELATIVE";
  case V810ISD::CMP:          return "V810ISD::CMP";
  case V810ISD::FCMP:         return "V810ISD::FCMP";
  case V810ISD::BCOND:        return "V810ISD::BCOND";
  case V810ISD::SETF:         return "V810ISD::SETF";
  case V810ISD::SELECT_CC:    return "V810ISD::SELECT_CC";
  case V810ISD::CALL:         return "V810ISD::CALL";
  case V810ISD::TAIL_CALL:    return "V810ISD::TAIL_CALL";
  case V810ISD::RET_GLUE:     return "V810ISD::RET_GLUE";
  case V810ISD::RETI_GLUE:    return "V810ISD::RETI_GLUE";
  case V810ISD::SMUL_LOHI:    return "V810ISD::SMUL_LOHI";
  case V810ISD::UMUL_LOHI:    return "V810ISD::UMUL_LOHI";
  case V810ISD::SDIVREM:      return "V810ISD::SDIVREM";
  case V810ISD::UDIVREM:      return "V810ISD::UDIVREM";
  case V810ISD::CAXI:         return "V810ISD::CAXI";
  }
  return nullptr;
}

void V810TargetLowering::computeKnownBitsForTargetNode(
                                       const SDValue Op,
                                       KnownBits &Known,
                                       const APInt &DemandedElts,
                                       const SelectionDAG &DAG,
                                       unsigned Depth) const {
  KnownBits Known2(32);
  Known.resetAll();

  switch (Op.getOpcode()) {
  default: break;
  case V810ISD::HI:
    Known = DAG.computeKnownBits(Op.getOperand(0), DemandedElts, Depth + 1);
    Known2 = KnownBits::makeConstant(APInt(32, 16));
    Known = KnownBits::shl(Known, Known2);
    break;
  case V810ISD::LO:
    Known = DAG.computeKnownBits(Op.getOperand(0), DemandedElts, Depth + 1);
    Known2 = DAG.computeKnownBits(Op.getOperand(1), DemandedElts, Depth + 1);
    Known = KnownBits::computeForAddSub(true, false, false, Known, Known2);
    break;
  case V810ISD::REG_RELATIVE: {
    // The first operand has the exact value that this node should return
    SDValue Value = Op.getOperand(0);
    Known = DAG.computeKnownBits(Value, DemandedElts, Depth + 1);

    // If it's relative to GP, the value is 0x0500****
    Register Reg = cast<RegisterSDNode>(Op.getOperand(1))->getReg();
    if (Reg == V810::R4 && Subtarget->enableGPRelativeRAM()) {
      Known2 = KnownBits::makeConstant(APInt(16, 0x0500));
      Known.insertBits(Known2, 16);
    }
    break;
  }
  case V810ISD::SETF: {
    auto CC = (V810CC::CondCodes) cast<ConstantSDNode>(Op.getOperand(0).getNode())->getZExtValue();
    if (CC == V810CC::CC_BR) // always true
      Known = KnownBits::makeConstant(APInt(32, 1));
    else if (CC == V810CC::CC_NOP) // always false
      Known = KnownBits::makeConstant(APInt(32, 0));
    else // dunno the bottom bit, but everything else is 0
      Known.Zero.setBitsFrom(1);
    break;
  }
  case V810ISD::SELECT_CC: {
    auto CC = (V810CC::CondCodes) cast<ConstantSDNode>(Op.getOperand(2).getNode())->getZExtValue();
    if (CC == V810CC::CC_BR) // always true
      Known = DAG.computeKnownBits(Op.getOperand(0), DemandedElts, Depth + 1);
    else if (CC == V810CC::CC_NOP) // always false
      Known = DAG.computeKnownBits(Op.getOperand(1), DemandedElts, Depth + 1);
    else {
      Known = DAG.computeKnownBits(Op.getOperand(0), DemandedElts, Depth + 1);
      Known2 = DAG.computeKnownBits(Op.getOperand(1), DemandedElts, Depth + 1);
      Known = Known.intersectWith(Known2);
    }
    break;
  }
  case V810ISD::SMUL_LOHI:
  case V810ISD::UMUL_LOHI: {
    assert((Op.getResNo() == 0 || Op.getResNo() == 1) && "Unknown result");
    Known = DAG.computeKnownBits(Op.getOperand(0), DemandedElts, Depth + 1);
    Known2 = DAG.computeKnownBits(Op.getOperand(1), DemandedElts, Depth + 1);
    if (Op.getResNo() == 0) {
      bool SelfMultiply = Op.getOperand(0) == Op.getOperand(1);
      Known = KnownBits::mul(Known, Known2, SelfMultiply);
    } else {
      if (Op.getOpcode() == V810ISD::UMUL_LOHI)
        Known = KnownBits::mulhu(Known, Known2);
      else
        Known = KnownBits::mulhs(Known, Known2);
    }
    break;
  }
  case V810ISD::SDIVREM: {
    assert((Op.getResNo() == 0 || Op.getResNo() == 1) && "Unknown result");
    Known = DAG.computeKnownBits(Op.getOperand(0), DemandedElts, Depth + 1);
    Known2 = DAG.computeKnownBits(Op.getOperand(1), DemandedElts, Depth + 1);
    if (Op.getResNo() == 0)
      Known = KnownBits::sdiv(Known, Known2, Op->getFlags().hasExact());
    else
      Known = KnownBits::srem(Known, Known2);
    break;
  }
  case V810ISD::UDIVREM: {
    assert((Op.getResNo() == 0 || Op.getResNo() == 1) && "Unknown result");
    Known = DAG.computeKnownBits(Op.getOperand(0), DemandedElts, Depth + 1);
    Known2 = DAG.computeKnownBits(Op.getOperand(1), DemandedElts, Depth + 1);
    if (Op.getResNo() == 0)
      Known = KnownBits::udiv(Known, Known2, Op->getFlags().hasExact());
    else
      Known = KnownBits::urem(Known, Known2);
    break;
  }
  }
}

SDValue V810TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins,
    const SDLoc &DL, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  V810MachineFunctionInfo *FuncInfo = MF.getInfo<V810MachineFunctionInfo>();

  // Figure out where the calling convention sez all the arguments live
  SmallVector<CCValAssign, 16> ArgLocs;
  V810CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext(),
                     MF.getFunction().getFunctionType()->getNumParams());
  CCInfo.AnalyzeFormalArguments(Ins, CC_V810);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      // This argument is passed in a register.
      // The registers are all 32 bits, so...

      // Create a virtual register for the promoted live-in value.
      Register VReg = MF.addLiveIn(VA.getLocReg(),
                                   getRegClassFor(VA.getLocVT()));
      SDValue Arg = DAG.getCopyFromReg(Chain, DL, VReg, VA.getLocVT());

      // If needed, truncate the register down to the argument type
      if (VA.isExtInLoc()) {
        Arg = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), Arg);
      }
      InVals.push_back(Arg);
      continue;
    }

    // If it wasn't passed as a register, it's on the stack
    assert(VA.isMemLoc());
    unsigned Offset = VA.getLocMemOffset();
    unsigned ValSize = VA.getValVT().getSizeInBits() / 8;
    int FI = MF.getFrameInfo().CreateFixedObject(ValSize, Offset, true);

    // So, read it from there
    InVals.push_back(
      DAG.getLoad(VA.getValVT(), DL, Chain,
                  DAG.getFrameIndex(FI, getPointerTy(MF.getDataLayout())),
                  MachinePointerInfo::getFixedStack(MF, FI)));
  }

  // If this function is variadic, the varargs are on the stack, after any fixed arguments.
  if (IsVarArg) {
    int VarFI = MFI.CreateFixedObject(4, CCInfo.getStackSize(), true);
    FuncInfo->setVarArgsFrameIndex(VarFI);
  }

  return Chain;
}

bool
V810TargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                   MachineFunction &MF,
                                   bool IsVarArg,
                                   const SmallVectorImpl<ISD::OutputArg> &Outs,
                                   LLVMContext &Context) const {
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, Context);
  return CCInfo.CheckReturn(Outs, RetCC_V810);
}

SDValue
V810TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                bool IsVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                const SDLoc &DL, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  bool IsInterrupt = MF.getFunction().hasFnAttribute("interrupt");

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_V810);

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue OutVal = OutVals[i];
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVal, Glue);

    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain;
  if (Glue.getNode()) {
    RetOps.push_back(Glue);
  }

  V810ISD::NodeType RetOp = IsInterrupt
    ? V810ISD::RETI_GLUE
    : V810ISD::RET_GLUE;

  return DAG.getNode(RetOp, DL, MVT::Other, RetOps);
}

SDValue
V810TargetLowering::LowerCall(CallLoweringInfo &CLI,
                              SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  MachineFunction &MF = DAG.getMachineFunction();
  SDLoc DL = CLI.DL;
  SDValue Chain = CLI.Chain;
  MVT PtrVT = getPointerTy(DAG.getDataLayout());
  unsigned NumParams = CLI.CB ? CLI.CB->getFunctionType()->getNumParams() : 0;

  SmallVector<CCValAssign, 16> ArgLocs;
  V810CCState CCInfo(CLI.CallConv, CLI.IsVarArg, MF, ArgLocs, *DAG.getContext(), NumParams);
  CCInfo.AnalyzeCallOperands(CLI.Outs, CC_V810);

  CLI.IsTailCall = CLI.IsTailCall && IsEligibleForTailCallOptimization(CCInfo, CLI, MF, ArgLocs);

  if (!CLI.IsTailCall) {
    Chain = DAG.getCALLSEQ_START(Chain, CCInfo.getStackSize(), 0, DL);
  }

  // Collect the registers to pass in.
  // Keys are the register, values are the instructions to pass em
  SmallVector<std::pair<Register, SDValue>, 8> RegsToPass;

  // Collect all the stores that have to happen before the call
  SmallVector<SDValue, 8> MemOpChains;

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    const CCValAssign &VA = ArgLocs[i];
    SDValue Arg = CLI.OutVals[i];
    // TODO: may have to do extension things here

    if (VA.isRegLoc()) {
      Register Reg = VA.getLocReg();
      RegsToPass.push_back(std::make_pair(Reg, Arg));
      continue;
    }

    // guess it was on the stack
    assert(VA.isMemLoc());
    // so pull the stack pointer,
    SDValue StackPtr = DAG.getRegister(V810::R3, PtrVT);
    // add our offset to it,
    SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset(), DL);
    PtrOff = DAG.getNode(ISD::ADD, DL, PtrVT, StackPtr, PtrOff);
    // and track that we need to store the value there
    MemOpChains.push_back(
        DAG.getStore(Chain, DL, Arg, PtrOff, MachinePointerInfo()));
  }

  // Emit all the stores, make sure they occur before the call
  if (!MemOpChains.empty()) {
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);
  }

  // Emit all the CopyToReg nodes, together
  SDValue InGlue;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, DL,
                             RegsToPass[i].first, RegsToPass[i].second, InGlue);
    InGlue = Chain.getValue(1);
  }

  // convert this into a target type now, so that legalization doesn't mess it up
  SDValue Callee = CLI.Callee;
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, PtrVT, 0, V810MCExpr::VK_V810_26_PCREL);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), PtrVT, V810MCExpr::VK_V810_26_PCREL);

  // Now build the ops for the call
  SmallVector<SDValue, 8> Ops;
  // first op is the chain, representing prereqs
  Ops.push_back(Chain);
  // second op is the callee, typically the GlobalAddress of the calling func
  Ops.push_back(Callee);
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    // next ops are the registers to pass,
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));
  }
  // then a mask describing the call-preserved registers
  const V810RegisterInfo *TRI = Subtarget->getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(DAG.getMachineFunction(),
                                                   CLI.CallConv);
  assert(Mask && "Need to specify which registers are reserved for this calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InGlue.getNode()) {
    Ops.push_back(InGlue);
  }

  if (CLI.IsTailCall) {
    DAG.getMachineFunction().getFrameInfo().setHasTailCall();
    return DAG.getNode(V810ISD::TAIL_CALL, DL, MVT::Other, Ops);
  }
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(V810ISD::CALL, DL, NodeTys, Ops);
  InGlue = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, CCInfo.getStackSize(), 0, InGlue, DL);
  InGlue = Chain.getValue(1);

  // Now after all that ceremony, extract the return values.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState RVInfo(CLI.CallConv, CLI.IsVarArg, MF, RVLocs, *DAG.getContext());
  RVInfo.AnalyzeCallResult(CLI.Ins, RetCC_V810);

  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    // We are returning values in regs (up to one reg, really).
    // Copy frmo that register to a virtual register.
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers");
    unsigned Reg = VA.getLocReg();

    Chain = DAG.getCopyFromReg(Chain, DL, Reg, VA.getLocVT(), InGlue).getValue(1);
    InGlue = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }
  
  return Chain;
}

bool V810TargetLowering::IsEligibleForTailCallOptimization(
    CCState &CCInfo, CallLoweringInfo &CLI, MachineFunction &MF, const SmallVector<CCValAssign, 16> &ArgLocs) const {
  if (MF.getFunction().hasFnAttribute("interrupt"))
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
  for (auto &Arg : CLI.Outs)
    if (Arg.Flags.isByVal())
      return false;

  return true;
}

TargetLowering::AtomicExpansionKind V810TargetLowering::shouldExpandAtomicRMWInIR(AtomicRMWInst *AI) const {
  return AtomicExpansionKind::CmpXChg;
}

static V810CC::CondCodes IntCondCodeToCC(ISD::CondCode CC) {
  switch (CC) {
  default: llvm_unreachable("Unknown integer condition code!");
  case ISD::SETEQ:  return V810CC::CC_E;
  case ISD::SETNE:  return V810CC::CC_NE;
  case ISD::SETLT:  return V810CC::CC_LT;
  case ISD::SETLE:  return V810CC::CC_LE;
  case ISD::SETGT:  return V810CC::CC_GT;
  case ISD::SETGE:  return V810CC::CC_GE;
  case ISD::SETULT: return V810CC::CC_C;
  case ISD::SETULE: return V810CC::CC_NH;
  case ISD::SETUGT: return V810CC::CC_H;
  case ISD::SETUGE: return V810CC::CC_NC;
  }
}

static V810CC::CondCodes FloatCondCodeToCC(ISD::CondCode CC) {
  switch (CC) {
  default: llvm_unreachable("Unknown float condition code!");
  // NB: the hardware throws an exception if either operand is a NaN,
  // so... the "ordered" and "unordered" conditions can just match
  case ISD::SETFALSE:
  case ISD::SETUO:
  case ISD::SETFALSE2:
    return V810CC::CC_NOP;
  case ISD::SETOEQ:
  case ISD::SETUEQ:
  case ISD::SETEQ:
    return V810CC::CC_E;
  case ISD::SETOGT:
  case ISD::SETUGT:
  case ISD::SETGT:
    return V810CC::CC_GT;
  case ISD::SETOGE:
  case ISD::SETUGE:
  case ISD::SETGE:
    return V810CC::CC_GE;
  case ISD::SETOLT:
  case ISD::SETULT:
  case ISD::SETLT:
    return V810CC::CC_LT;
  case ISD::SETOLE:
  case ISD::SETULE:
  case ISD::SETLE:
    return V810CC::CC_LE;
  case ISD::SETONE:
  case ISD::SETUNE:
  case ISD::SETNE:
    return V810CC::CC_NE;
  case ISD::SETO:
  case ISD::SETTRUE:
  case ISD::SETTRUE2:
    return V810CC::CC_BR;
  }
}

bool V810TargetLowering::IsGPRelative(const GlobalValue *GVal) const {
  if (!Subtarget->enableGPRelativeRAM()) {
    return false;
  }
  const V810TargetObjectFile *TLOF =
      static_cast<const V810TargetObjectFile *>(
          getTargetMachine().getObjFileLowering());
  return TLOF->isGlobalInSmallSection(GVal->getAliaseeObject(), getTargetMachine());
}

static SDValue BuildMovhiMoveaPair(SelectionDAG &DAG, const GlobalValue *GV, SDLoc DL, EVT VT, int64_t Offset) {
  SDValue HiTarget = DAG.getTargetGlobalAddress(GV, DL, VT, Offset, V810MCExpr::VK_V810_HI);
  SDValue LoTarget = DAG.getTargetGlobalAddress(GV, DL, VT, Offset, V810MCExpr::VK_V810_LO);

  SDValue Hi = DAG.getNode(V810ISD::HI, DL, VT, HiTarget);
  return DAG.getNode(V810ISD::LO, DL, VT, Hi, LoTarget);
}

// Convert a global address into a GP-relative offset or a HI/LO pair
static SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG, const V810TargetLowering *TLI) {
  GlobalAddressSDNode *GN = cast<GlobalAddressSDNode>(Op);
  SDLoc DL(Op);
  EVT VT = Op.getValueType();

  const GlobalValue *GV = GN->getGlobal();
  if (TLI->IsGPRelative(GV)) {
    // The address of every global variable in the "small data" area 
    // can be expressed by a 16-bit signed offset from the GP register (R4).
    SDValue RelTarget = DAG.getTargetGlobalAddress(GV, DL, GN->getValueType(0), GN->getOffset(), V810MCExpr::VK_V810_SDAOFF);
    SDValue Reg = DAG.getRegister(V810::R4, GN->getValueType(0));
    return DAG.getNode(V810ISD::REG_RELATIVE, DL, VT, RelTarget, Reg);
  }

  // Fall back to a MOVHI/MOVEA pair for any other addresses
  return BuildMovhiMoveaPair(DAG, GN->getGlobal(), DL, VT, GN->getOffset());
}

static SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) {
  BlockAddressSDNode *BA = cast<BlockAddressSDNode>(Op);

  SDLoc DL(Op);
  SDValue HiTarget = DAG.getTargetBlockAddress(BA->getBlockAddress(), BA->getValueType(0),
                                               BA->getOffset(), V810MCExpr::VK_V810_HI);
  SDValue LoTarget = DAG.getTargetBlockAddress(BA->getBlockAddress(), BA->getValueType(0),
                                               BA->getOffset(), V810MCExpr::VK_V810_LO);

  EVT VT = Op.getValueType();
  SDValue Hi = DAG.getNode(V810ISD::HI, DL, VT, HiTarget);
  return DAG.getNode(V810ISD::LO, DL, VT, Hi, LoTarget);  
}

static SDValue LowerConstantPool(SDValue Op, SelectionDAG &DAG) {
  ConstantPoolSDNode *CP = cast<ConstantPoolSDNode>(Op);

  SDLoc DL(Op);
  SDValue HiTarget = DAG.getTargetConstantPool(CP->getConstVal(), CP->getValueType(0),
                                               CP->getAlign(), CP->getOffset(), V810MCExpr::VK_V810_HI);
  SDValue LoTarget = DAG.getTargetConstantPool(CP->getConstVal(), CP->getValueType(0),
                                               CP->getAlign(), CP->getOffset(), V810MCExpr::VK_V810_LO);

  EVT VT = Op.getValueType();
  SDValue Hi = DAG.getNode(V810ISD::HI, DL, VT, HiTarget);
  return DAG.getNode(V810ISD::LO, DL, VT, Hi, LoTarget);
}

static SDValue LowerConstantFP(SDValue Op, SelectionDAG &DAG) {
  EVT ResType = Op.getValueType();
  assert(ResType == MVT::f32);
  ConstantFPSDNode *C = dyn_cast<ConstantFPSDNode>(Op);
  assert(C);

  SDLoc DL(Op);

  uint64_t Value = C->getValueAPF().bitcastToAPInt().getZExtValue();

  if (Value == 0) {
    // TODO: why does this need to return a virtual register?
    // `return x > y ? x - y : 0.0;` seems to induce a need for it
    return DAG.getCopyFromReg(DAG.getEntryNode(), DL, V810::R0, ResType);
  }

  if (isUInt<16>(Value) && !isInt<16>(Value)) {
    SDValue R0 = DAG.getRegister(V810::R0, ResType);
    SDValue SDValueLo = DAG.getTargetConstant(Value, DL, MVT::i32);
    return SDValue(DAG.getMachineNode(V810::ADDI, DL, ResType, R0, SDValueLo), 0);
  }

  uint64_t ValueHi = EvalHi(Value);
  uint64_t ValueLo = EvalLo(Value);

  SDValue Result = DAG.getRegister(V810::R0, ResType);
  if (ValueHi) {
    SDValue SDValueHi = DAG.getTargetConstant(ValueHi, DL, MVT::i32);
    Result = SDValue(DAG.getMachineNode(V810::MOVHI, DL, ResType, Result, SDValueHi), 0);
  }
  if (ValueLo) {
    SDValue SDValueLo = DAG.getTargetConstant(ValueLo, DL, MVT::i32);
    Result = SDValue(DAG.getMachineNode(V810::MOVEA, DL, ResType, Result, SDValueLo), 0);
  }
  return Result;
}

// Convert a BR_CC into a cmp+bcond pair
static SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) {
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Dest = Op.getOperand(4);

  SDLoc DL(Op);
  EVT VT = Op.getValueType();

  SDVTList VTWithGlue = DAG.getVTList(VT, MVT::Glue);
  SDValue Cond;
  SDValue Cmp;
  if (LHS.getValueType().isFloatingPoint()) {
    Cond = DAG.getConstant(FloatCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::FCMP, DL, VTWithGlue, Chain, LHS, RHS);
  } else {
    Cond = DAG.getConstant(IntCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::CMP, DL, VTWithGlue, Chain, LHS, RHS);
  }

  return DAG.getNode(V810ISD::BCOND, DL, VT, Cmp, Cond, Dest, Cmp.getValue(1));
}

static SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueVal = Op.getOperand(2);
  SDValue FalseVal = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();

  SDLoc DL(Op);
  EVT VT = Op.getValueType();

  SDVTList VTWithGlue = DAG.getVTList(VT, MVT::Glue);
  SDValue Cond;
  SDValue Cmp;
  if (LHS.getValueType().isFloatingPoint()) {
    Cond = DAG.getConstant(FloatCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::FCMP, DL, VTWithGlue, DAG.getEntryNode(), LHS, RHS);
  } else {
    Cond = DAG.getConstant(IntCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::CMP, DL, VTWithGlue, DAG.getEntryNode(), LHS, RHS);
  }

  return DAG.getNode(V810ISD::SELECT_CC, DL, VT, TrueVal, FalseVal, Cond, Cmp.getValue(1));
}

static SDValue LowerSETCC(SDValue Op, SelectionDAG &DAG) {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();

  SDLoc DL(Op);
  EVT VT = Op.getValueType(); // This could return a bool if that's useful?

  SDVTList VTWithGlue = DAG.getVTList(VT, MVT::Glue);
  SDValue Cond;
  SDValue Cmp;
  if (LHS.getValueType().isFloatingPoint()) {
    Cond = DAG.getConstant(FloatCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::FCMP, DL, VTWithGlue, DAG.getEntryNode(), LHS, RHS);
  } else {
    Cond = DAG.getConstant(IntCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::CMP, DL, VTWithGlue, DAG.getEntryNode(), LHS, RHS);
  }

  return DAG.getNode(V810ISD::SETF, DL, VT, Cond, Cmp.getValue(1));
}

static SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG, const V810TargetLowering &TLI) {
  MachineFunction &MF = DAG.getMachineFunction();
  V810MachineFunctionInfo *FuncInfo = MF.getInfo<V810MachineFunctionInfo>();

  // vastart just stores the address of the VarArgsFrameIndex slot into the
  // memory location argument.
  SDLoc DL(Op);
  MVT PtrVT = TLI.getPointerTy(DAG.getDataLayout());
  SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(), PtrVT);

  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), DL, FI, Op.getOperand(1),
                      MachinePointerInfo(SV));
}

static SDValue LowerIntBinHiLo(SDValue Op, SelectionDAG &DAG, unsigned Opc) {
  SDLoc DL(Op);
  SDVTList RealVT = DAG.getVTList(MVT::i32, MVT::Glue);
  SDValue Ops[] = {Op.getOperand(0), Op.getOperand(1)};
  SDValue MAD = DAG.getNode(Opc, DL, RealVT, Ops);

  SDValue NormalRes = MAD.getValue(0);
  SDValue Glue = MAD.getValue(1);

  SDValue R30Res = DAG.getCopyFromReg(DAG.getEntryNode(), DL, V810::R30, MVT::i32, Glue);

  SDValue Vals[] = { NormalRes, R30Res };
  return DAG.getMergeValues(Vals, DL);
}

static SDValue LowerSMUL_LOHI(SDValue Op, SelectionDAG &DAG) {
  return LowerIntBinHiLo(Op, DAG, V810ISD::SMUL_LOHI);
}

static SDValue LowerUMUL_LOHI(SDValue Op, SelectionDAG &DAG) {
  return LowerIntBinHiLo(Op, DAG, V810ISD::UMUL_LOHI);
}

static SDValue LowerSDIVREM(SDValue Op, SelectionDAG &DAG) {
  return LowerIntBinHiLo(Op, DAG, V810ISD::SDIVREM);
}

static SDValue LowerUDIVREM(SDValue Op, SelectionDAG &DAG) {
  return LowerIntBinHiLo(Op, DAG, V810ISD::UDIVREM);
}

static SDValue LowerATOMIC_CMP_SWAP(SDValue Op, SelectionDAG &DAG) {
  SDValue Chain = Op.getOperand(0);
  SDValue Ptr = Op.getOperand(1);
  SDValue Cmp = Op.getOperand(2);
  SDValue Swap = Op.getOperand(3);
  SDLoc DL(Op);

  Chain = DAG.getCopyToReg(Chain, DL, V810::R30, Swap);
  SDValue Reg30 = DAG.getRegister(V810::R30, MVT::i32);

  SDVTList ValAndChainVT = DAG.getVTList(MVT::i32, MVT::Other);
  return DAG.getNode(V810ISD::CAXI, DL, ValAndChainVT, Chain, Ptr, Cmp, Reg30);
}

static SDValue LowerATOMIC_CMP_SWAP_WITH_SUCCESS(SDValue Op, SelectionDAG &DAG) {
  SDValue CmpSwap = LowerATOMIC_CMP_SWAP(Op, DAG);
  SDValue Val = CmpSwap.getValue(0);
  SDValue Chain = CmpSwap.getValue(1);

  SDLoc DL(Op);

  SDValue Cond = DAG.getConstant(V810CC::CC_E, DL, MVT::i32);
  SDValue Success = DAG.getNode(V810ISD::SETF, DL, MVT::i32, Cond);

  SDValue Vals[] = { Val, Success, Chain };
  return DAG.getMergeValues(Vals, DL);
}

static SDValue LowerATOMIC_FENCE(SDValue Op, SelectionDAG &DAG) {
  // Lower it to nothing, just return the input chain.
  // Don't need fences cuz we're single threaded bay-bee
  return Op.getOperand(0);
}

SDValue V810TargetLowering::
LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default: llvm_unreachable("Should not custom lower this!");

  case ISD::GlobalAddress:    return LowerGlobalAddress(Op, DAG, this);
  case ISD::GlobalTLSAddress: return LowerGlobalAddress(Op, DAG, this); // luckily no threads
  case ISD::BlockAddress:     return LowerBlockAddress(Op, DAG);
  case ISD::ConstantPool:     return LowerConstantPool(Op, DAG);
  case ISD::ConstantFP:       return LowerConstantFP(Op, DAG);
  case ISD::BR_CC:            return LowerBR_CC(Op, DAG);
  case ISD::SELECT_CC:        return LowerSELECT_CC(Op, DAG);
  case ISD::SETCC:            return LowerSETCC(Op, DAG);
  case ISD::VASTART:          return LowerVASTART(Op, DAG, *this);
  case ISD::SMUL_LOHI:        return LowerSMUL_LOHI(Op, DAG);
  case ISD::UMUL_LOHI:        return LowerUMUL_LOHI(Op, DAG);
  case ISD::SDIVREM:          return LowerSDIVREM(Op, DAG);
  case ISD::UDIVREM:          return LowerUDIVREM(Op, DAG);
  case ISD::ATOMIC_CMP_SWAP:  return LowerATOMIC_CMP_SWAP(Op, DAG);
  case ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS:
                              return LowerATOMIC_CMP_SWAP_WITH_SUCCESS(Op, DAG);
  case ISD::ATOMIC_FENCE:     return LowerATOMIC_FENCE(Op, DAG);
  }
}

// Instead of generating code like:
//    movhi hi(<globalvar> + 4), r0, r6
//    movea lo(<globalvar> + 4), r6, r6
//    ld.w  0[r6], r7
//    movhi hi(<globalvar> + 8), r0, r6
//    movea lo(<globalvar> + 8), r6, r6
//    ld.w  0[r6], r8
// fold those offsets into the load instruction:
//    movhi hi(<globalvar>), r0, r6
//    movea lo(<globalvar>), r6, r6
//    ld.w  4[r6], r7
//    ld.w  8[r6], r8
static SDValue PerformLoadCombine(SDNode *N,
                                  TargetLowering::DAGCombinerInfo &DCI,
                                  const V810TargetLowering *TLI) {
  SelectionDAG &DAG = DCI.DAG;
  SDLoc DL(N);
  LoadSDNode *LD = cast<LoadSDNode>(N);
  
  const GlobalValue *GA;
  int64_t Offset = 0;
  if (!TLI->isGAPlusOffset(LD->getBasePtr().getNode(), GA, Offset)) {
    return SDValue();
  }
  if (Offset == 0 || TLI->IsGPRelative(GA)) {
    return SDValue();
  }

  EVT PtrType = LD->getBasePtr().getValueType();

  SDValue GlobalAddr = BuildMovhiMoveaPair(DAG, GA, DL, PtrType, 0);
  SDValue ConstOffset = DAG.getConstant(APInt(32, Offset, true), DL, PtrType);
  SDValue BasePtr = DAG.getMemBasePlusOffset(GlobalAddr, ConstOffset, DL, LD->getFlags());
  return DAG.getLoad(LD->getAddressingMode(), LD->getExtensionType(),
    LD->getValueType(0), DL, LD->getChain(), BasePtr, LD->getOffset(),
    LD->getMemoryVT(), LD->getMemOperand());
}

// Instead of generating code like:
//    movhi hi(<globalvar> + 4), r0, r6
//    movea lo(<globalvar> + 4), r6, r6
//    st.w  r7, 0[r6]
//    movhi hi(<globalvar> + 8), r0, r6
//    movea lo(<globalvar> + 8), r6, r6
//    ld.w  r8, 0[r6]
// fold those offsets into the store instruction:
//    movhi hi(<globalvar>), r0, r6
//    movea lo(<globalvar>), r6, r6
//    ld.w  r7, 4[r6]
//    ld.w  r8, 8[r6]
static SDValue PerformStoreCombine(SDNode *N,
                                   TargetLowering::DAGCombinerInfo &DCI,
                                   const V810TargetLowering *TLI) {
  SelectionDAG &DAG = DCI.DAG;
  SDLoc DL(N);
  StoreSDNode *ST = cast<StoreSDNode>(N);
  
  const GlobalValue *GA;
  int64_t Offset = 0;
  if (!TLI->isGAPlusOffset(ST->getBasePtr().getNode(), GA, Offset)) {
    return SDValue();
  }
  if (Offset == 0 || TLI->IsGPRelative(GA)) {
    return SDValue();
  }

  EVT PtrType = ST->getBasePtr().getValueType();

  SDValue GlobalAddr = BuildMovhiMoveaPair(DAG, GA, DL, PtrType, 0);
  SDValue ConstOffset = DAG.getConstant(APInt(32, Offset, true), DL, PtrType);
  SDValue BasePtr = DAG.getMemBasePlusOffset(GlobalAddr, ConstOffset, DL, ST->getFlags());
  SDValue NewST = DAG.getTruncStore(ST->getChain(), DL, ST->getValue(), BasePtr, ST->getMemoryVT(), ST->getMemOperand());
  if (ST->isIndexed()) {
    NewST = DAG.getIndexedStore(NewST, DL, BasePtr, ST->getOffset(), ST->getAddressingMode());
  }
  return NewST;
}

SDValue V810TargetLowering::
PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const {
  switch (N->getOpcode()) {
  default: return SDValue();
  case ISD::LOAD:
    return PerformLoadCombine(N, DCI, this);
  case ISD::STORE:
    return PerformStoreCombine(N, DCI, this);
  }
}

EVT V810TargetLowering::getSetCCResultType(const DataLayout &DL, LLVMContext &Context, EVT VT) const {
  if (!VT.isVector())
    return getPointerTy(DL);
  return VT.changeVectorElementTypeToInteger();
}

EVT V810TargetLowering::getOptimalMemOpType(const MemOp &Op,
                                            const AttributeList &FuncAttributes) const {
  return MVT::i32;
}

// In case we ever use GISel
LLT V810TargetLowering::getOptimalMemOpLLT(const MemOp &Op,
                                           const AttributeList &FuncAttributes) const {
  return LLT::scalar(32);
}

bool V810TargetLowering::allowsMemoryAccess(
    LLVMContext &Context, const DataLayout &DL, EVT VT, unsigned AddrSpace,
    Align Alignment, MachineMemOperand::Flags Flags, unsigned *Fast) const {
  // We specify an alignment for 64-bit accesses, but don't actually support loading/storing them directly.
  if (VT.bitsGT(MVT::i32)) {
    return false;
  }
  return TargetLoweringBase::allowsMemoryAccess(
              Context, DL, VT, AddrSpace, Alignment, Flags, Fast);
}

MachineBasicBlock *
V810TargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                                MachineBasicBlock *BB) const {
  switch (MI.getOpcode()) {
  default: llvm_unreachable("No custom inserter found for instruction");
  case V810::SELECT_CC_Int:
  case V810::SELECT_CC_Float:
    return ExpandSelectCC(MI, BB);
  }  
}

// This is called after LowerSELECT_CC.
// It turns the target-specific SELECT_CC instr into a set of branches.
MachineBasicBlock *
V810TargetLowering::ExpandSelectCC(MachineInstr &MI, MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *Subtarget->getInstrInfo();
  DebugLoc dl = MI.getDebugLoc();

  Register Dest = MI.getOperand(0).getReg();
  Register TrueSrc = MI.getOperand(1).getReg();
  Register FalseSrc = MI.getOperand(2).getReg();
  unsigned CC = (V810CC::CondCodes)MI.getOperand(3).getImm();

  /*
    Split this block into the following control flow structure:
    ThisMBB
    |  \
    |   IfFalseMBB
    |  /
    SinkMBB

    ThisMBB ends with a Bcond; it goes to SinkMBB if true and IfFalseMBB if false.
    IfFalseMBB falls through to SinkMBB.
    SinkMBB uses a phi node to track the result.
  */
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator It = ++BB->getIterator();

  MachineBasicBlock *ThisMBB = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *IfFalseMBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *SinkMBB = F->CreateMachineBasicBlock(LLVM_BB);
  F->insert(It, IfFalseMBB);
  F->insert(It, SinkMBB);

  // Every node after the SELECT_CC in ThisMBB moves to SinkMBB
  SinkMBB->splice(SinkMBB->begin(), ThisMBB,
                  std::next(MachineBasicBlock::iterator(MI)), ThisMBB->end());
  SinkMBB->transferSuccessorsAndUpdatePHIs(ThisMBB);

  // Add the two successors of ThisMBB
  ThisMBB->addSuccessor(IfFalseMBB);
  ThisMBB->addSuccessor(SinkMBB);

  // ThisMBB ends with a Bcond
  BuildMI(ThisMBB, dl, TII.get(V810::Bcond))
    .addImm(CC)
    .addMBB(SinkMBB);

  // IfFalseMBB has no logic, it just falls through to SinkMBB...
  IfFalseMBB->addSuccessor(SinkMBB);

  // because the logic is handled with a phi node at SinkMBB's start.
  // %Result = phi [ %TrueValue, ThisMBB ], [ %FalseValue, IfFalseMBB ]
  BuildMI(*SinkMBB, SinkMBB->begin(), dl, TII.get(V810::PHI), Dest)
    .addReg(TrueSrc).addMBB(ThisMBB)
    .addReg(FalseSrc).addMBB(IfFalseMBB);

  MI.eraseFromParent(); // The pseudo instruction is gone.
  return SinkMBB;
}

V810TargetLowering::ConstraintType
V810TargetLowering::getConstraintType(StringRef Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:  break;
    case 'r':
      return C_RegisterClass;
    }
  }

  return TargetLowering::getConstraintType(Constraint);
}

std::pair<unsigned, const TargetRegisterClass *>
V810TargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                  StringRef Constraint,
                                                  MVT VT) const {
  if (Constraint.empty())
    return std::make_pair(0U, nullptr);
  if (Constraint == "r") {
    return std::make_pair(0U, &V810::GenRegsRegClass);
  }
  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}
