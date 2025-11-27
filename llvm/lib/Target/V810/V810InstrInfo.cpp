#include "V810InstrInfo.h"
#include "V810.h"
#include "V810Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineScheduler.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "V810GenInstrInfo.inc"

void V810InstrInfo::anchor() {}

V810InstrInfo::V810InstrInfo(const V810Subtarget &ST)
  : V810GenInstrInfo(ST, RI, V810::ADJCALLSTACKDOWN, V810::ADJCALLSTACKUP), RI() {}

void V810InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I, const DebugLoc &DL,
                                Register DestReg, Register SrcReg,
                                bool KillSrc, bool RenamableDest, bool RenamableSrc) const {
  assert(V810::GenRegsRegClass.contains(DestReg, SrcReg));
  if (DestReg == SrcReg) {
    return;
  }
  BuildMI(MBB, I, DL, get(V810::MOVr), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc) | getRenamableRegState(RenamableSrc));
}

Register V810InstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                            int &FrameIndex) const {
  if (MI.getOpcode() == V810::LD_W) {
    if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
        MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
  }
  return Register();
}

Register V810InstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                            int &FrameIndex) const {
  if (MI.getOpcode() == V810::ST_W) {
    if (MI.getOperand(0).isFI() && MI.getOperand(1).isImm() &&
        MI.getOperand(1).getImm() == 0) {
      FrameIndex = MI.getOperand(0).getIndex();
      return MI.getOperand(2).getReg();
    }
  }
  return Register();
}

void V810InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator I,
                                        Register SrcReg, bool isKill, int FrameIndex,
                                        const TargetRegisterClass *RC,
                                        Register VReg,
                                        MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  const MachineFrameInfo &MFI = MF->getFrameInfo();
  MachineMemOperand *MMO = MF->getMachineMemOperand(
    MachinePointerInfo::getFixedStack(*MF, FrameIndex), MachineMemOperand::MOStore,
    MFI.getObjectSize(FrameIndex), MFI.getObjectAlign(FrameIndex));

  BuildMI(MBB, I, DL, get(V810::ST_W)).addFrameIndex(FrameIndex).addImm(0)
    .addReg(SrcReg, getKillRegState(isKill)).addMemOperand(MMO)
    .setMIFlag(MachineInstr::FrameSetup);
}

void V810InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator I,
                                         Register DestReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         Register VReg,
                                         MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  const MachineFrameInfo &MFI = MF->getFrameInfo();
  MachineMemOperand *MMO = MF->getMachineMemOperand(
    MachinePointerInfo::getFixedStack(*MF, FrameIndex), MachineMemOperand::MOLoad,
    MFI.getObjectSize(FrameIndex), MFI.getObjectAlign(FrameIndex));

  BuildMI(MBB, I, DL, get(V810::LD_W), DestReg).addFrameIndex(FrameIndex).addImm(0)
    .addMemOperand(MMO);
}

MachineBasicBlock *V810InstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("unexpected opcode!");
  case V810::Bcond:
    return MI.getOperand(1).getMBB();
  case V810::JAL:
  case V810::JR:
    return MI.getOperand(0).getMBB();
  }
}

static bool isUncondBranch(MachineInstr &MI) {
  switch (MI.getOpcode()) {
  default:        return false;
  case V810::JR:  return true;
  case V810::Bcond:
    return MI.getOperand(0).getImm() == V810CC::CC_BR;
  }
}

static bool isCondBranch(const MachineInstr &MI) {
  return MI.getOpcode() == V810::Bcond
    && MI.getOperand(0).getImm() != V810CC::CC_BR
    && MI.getOperand(0).getImm() != V810CC::CC_NOP;
}

static bool isIndirectBranch(const MachineInstr &MI) {
  return MI.getOpcode() == V810::JMP;
}

static bool isNop(const MachineInstr &MI) {
  return MI.getOpcode() == V810::Bcond
    && MI.getOperand(0).getImm() == V810CC::CC_NOP;
}

static bool isUnintentionalNop(const MachineInstr &MI) {
  // The canonical nop is a BCond where the condition is "never" and the target is 0.
  // Any nop which doesn't look like that is just a branch-thatll-never-happen,
  // and can be deleted as an optimization.
  return isNop(MI) && MI.getOperand(1).isMBB();
}

static void stripUnintentionalNops(MachineBasicBlock &MBB) {
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  while (I != MBB.begin()) {
    MachineInstr *Inst = &*I;
    I--;
    if (isUnintentionalNop(*Inst))
      Inst->removeFromParent();
  }
}

bool V810InstrInfo::isUnpredicatedTerminatorBesidesNop(const MachineInstr &MI) const {
  return isUnpredicatedTerminator(MI) && !isNop(MI);
}

bool V810InstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                  MachineBasicBlock *&TBB,
                                  MachineBasicBlock *&FBB,
                                  SmallVectorImpl<MachineOperand> &Cond,
                                  bool AllowModify) const {
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end())
    return false;

  if (AllowModify)
    stripUnintentionalNops(MBB);

  if (!isUnpredicatedTerminatorBesidesNop(*I))
    return false;
  
  MachineInstr *LastInst = &*I;

  // function ends with only one terminator
  if (I == MBB.begin() || !isUnpredicatedTerminatorBesidesNop(*--I)) {
    if (isUncondBranch(*LastInst)) {
      TBB = getBranchDestBlock(*LastInst);
      return false;
    }
    if (isCondBranch(*LastInst)) {
      TBB = getBranchDestBlock(*LastInst);
      Cond.push_back(LastInst->getOperand(0));
      return false;
    }
    // can't handle indirect branches
    return true;
  }

  MachineInstr *SecondLastInst = &*I;

  // If we're allowed to, remove any unconditional branches after other unconditional branches
  if (AllowModify && isUncondBranch(*LastInst)) {
    while (isUncondBranch(*SecondLastInst)) {
      LastInst->eraseFromParent();
      LastInst = SecondLastInst;
      if (I == MBB.begin() || !isUnpredicatedTerminatorBesidesNop(*--I)) {
        // NOW it just ends with one unconditional branch
        TBB = getBranchDestBlock(*LastInst);
        return false;
      } else {
        SecondLastInst = &*I;
      }
    }
  }

  // Give up if there are more than three terminators
  if (SecondLastInst && I != MBB.begin() && isUnpredicatedTerminatorBesidesNop(*--I))
    return true;
  
  // conditional branch followed by unconditional branch
  if (isCondBranch(*SecondLastInst) && isUncondBranch(*LastInst)) {
    TBB = getBranchDestBlock(*SecondLastInst);
    FBB = getBranchDestBlock(*LastInst);
    Cond.push_back(SecondLastInst->getOperand(0));
    return false;
  }

  // indirect branch followed by anything, clear out the anything
  if (isIndirectBranch(*SecondLastInst) && isUncondBranch(*LastInst)) {
    I = LastInst;
    if (AllowModify)
      I->eraseFromParent();
  }

  return true;
}

unsigned V810InstrInfo::insertBranch(MachineBasicBlock &MBB,
                                     MachineBasicBlock *TBB,
                                     MachineBasicBlock *FBB,
                                     ArrayRef<MachineOperand> Cond,
                                     const DebugLoc &DL,
                                     int *BytesAdded) const {
  assert(TBB);
  // NB: if these branches are too short, we will fix them later

  if (Cond.empty()) {
    // Unconditional branch
    assert(!FBB && "Unconditional branch with multiple successors");
    BuildMI(&MBB, DL, get(V810::JR)).addMBB(TBB);
    if (BytesAdded)
      *BytesAdded = 4;
    return 1;
  }

  // Conditional branch
  unsigned CC = Cond[0].getImm();
  BuildMI(&MBB, DL, get(V810::Bcond)).addImm(CC).addMBB(TBB);

  if (!FBB) {
    if (BytesAdded)
      *BytesAdded = 2;
    return 1;
  }

  BuildMI(&MBB, DL, get(V810::JR)).addMBB(FBB);
  if (BytesAdded)
    *BytesAdded = 6;
  return 2;
}

unsigned V810InstrInfo::removeBranch(MachineBasicBlock &MBB,
                                     int *BytesRemoved) const {
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;
  int Removed = 0;
  while (I != MBB.begin()) {
    --I;

    if (I->isDebugInstr())
      continue;
    if (!isCondBranch(*I) && !isUncondBranch(*I))
      break; // Not a branch
    
    Removed += getInstSizeInBytes(*I);
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  if (BytesRemoved)
    *BytesRemoved = Removed;
  return Count;
}

bool V810InstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  if (Cond.size() != 1) {
    llvm_unreachable(std::to_string(Cond.size()).c_str());
  }
  assert(Cond.size() == 1);
  V810CC::CondCodes CC = static_cast<V810CC::CondCodes>(Cond[0].getImm());
  Cond[0].setImm(InvertV810CondCode(CC));
  return false;
}

bool V810InstrInfo::isBranchOffsetInRange(unsigned BranchOpc, int64_t Offset) const {
  switch(BranchOpc) {
  default:
    llvm_unreachable("Unknown branch instruction!");
  case V810::Bcond:
    return isInt<9>(Offset);
  case V810::JR:
  case V810::JAL:
    return isInt<26>(Offset);
  }
}

bool V810InstrInfo::analyzeCompare(const MachineInstr &MI, Register &SrcReg,
                                   Register &SrcReg2, int64_t &CmpMask,
                                   int64_t &CmpValue) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("Unknown compare instruction!");
  case V810::CMPrr:
  case V810::CMPF_S:
    SrcReg = MI.getOperand(0).getReg();
    SrcReg2 = MI.getOperand(1).getReg();
    CmpMask = 0;
    CmpValue = 0;
    return true;
  case V810::CMPri:
    SrcReg = MI.getOperand(0).getReg();
    SrcReg2 = Register();
    CmpMask = 0x1f;
    CmpValue = MI.getOperand(1).getImm();
    return true;
  }
}

static V810II::CCFlags PSWFlagsRequiredForCondCode(V810CC::CondCodes CC) {
  switch (CC) {
  default: llvm_unreachable("Unrecognized condition code");
  case V810CC::CC_V:
  case V810CC::CC_NV:
    return V810II::V810_OVFlag;
  case V810CC::CC_C:
  case V810CC::CC_NC:
    return V810II::V810_CYFlag;
  case V810CC::CC_E:
  case V810CC::CC_NE:
    return V810II::V810_ZFlag;
  case V810CC::CC_NH:
  case V810CC::CC_H:
    return V810II::V810_CYFlag | V810II::V810_ZFlag;
  case V810CC::CC_N:
  case V810CC::CC_P:
    return V810II::V810_SFlag;
  case V810CC::CC_BR:
  case V810CC::CC_NOP:
    return V810II::V810_NoFlags;
  case V810CC::CC_LT:
  case V810CC::CC_GE:
    return V810II::V810_OVFlag | V810II::V810_SFlag;
  case V810CC::CC_LE:
  case V810CC::CC_GT:
    return V810II::V810_OVFlag | V810II::V810_SFlag | V810II::V810_ZFlag;
  }
}

static bool tryGetCondCode(MachineInstr &MI, V810CC::CondCodes &CC) {
  switch (MI.getOpcode()) {
  default: return false;
  case V810::Bcond:
    CC = (V810CC::CondCodes) MI.getOperand(0).getImm();
    return true;
  case V810::SETF:
    CC = (V810CC::CondCodes) MI.getOperand(1).getImm();
    return true;
  }
}

bool V810InstrInfo::optimizeCompareInstr(MachineInstr &MI, Register SrcReg,
                                         Register SrcReg2, int64_t CmpMask,
                                         int64_t CmpValue, const MachineRegisterInfo *MRI) const {
  // We only optimize CMP 0, <reg>
  if (CmpMask == 0 || CmpValue != 0) {
    return false;
  }

  const TargetRegisterInfo *TRI = &getRegisterInfo();

  // Find the operation which set <reg>. 
  MachineInstr *SrcRegDef = NULL;
  auto From = std::next(MachineBasicBlock::reverse_iterator(MI));
  for (MachineBasicBlock *BB = MI.getParent();;) {
    for (MachineInstr &Inst : make_range(From, BB->rend())) {
      if (Inst.definesRegister(SrcReg, TRI)) {
        SrcRegDef = &Inst;
        break;
      }
      if (Inst.definesRegister(V810::SR5, TRI)) {
        // Something besides the operation we cared about has messed with PSW.
        // This compare-with-zero is not a no-op after all.
        // TODO: maybe possible to rearrange instructions to MAKE it a no-op
        return false;
      }
    }
    if (SrcRegDef) {
      break;
    }
    // <reg> was set in an earlier basic block.
    // If control flow is linear, keep trying to find it.
    if (BB->pred_size() != 1) {
      return false;
    }
    BB = *BB->pred_begin();
    From = BB->rbegin();
  }

  // Look up which PSW flags that operation sets.
  assert(SrcRegDef);
  V810II::CCFlags FlagsDefined = V810II::V810_AllFlags & (V810II::CCFlags) SrcRegDef->getDesc().TSFlags;

  if (FlagsDefined != V810II::V810_AllFlags) {
    // This CMP isn't a complete no-op; it sets PSW flags which the original operation hadn't.
    // Don't remove it unless we're sure that nothing uses those extra flags.
    bool FlagsMayLiveOut = true;
    for (MachineInstr &Instr : make_range(std::next(MachineBasicBlock::iterator(MI)), MI.getParent()->end())) {
      V810CC::CondCodes CC;
      if (tryGetCondCode(Instr, CC)) {
        V810II::CCFlags FlagsRequired = PSWFlagsRequiredForCondCode(CC);
        if ((FlagsRequired & FlagsDefined) != FlagsRequired) {
          // We care about a flag which doesn't get set without this CMP.
          return false;
        }
      }
      if (Instr.definesRegister(V810::SR5, TRI)) {
        // PSW got clobbered, so the old value doesn't matter beyond this point.
        FlagsMayLiveOut = false;
        break;
      }
    }

    // If PSW is live-out, some later basic block might on those extra flags.
    if (FlagsMayLiveOut) {
      for (MachineBasicBlock *Successor : MI.getParent()->successors()) {
        if (Successor->isLiveIn(V810::SR5)) {
          // Figuring out HOW this successor uses PSW would take graph traversal.
          // Just skip the optimization instead of doing that.
          return false;
        }
      }
    }
  }

  // Finally, we're sure that this cmp is unnecessary.
  MI.eraseFromParent();
  return true;
}

bool V810InstrInfo::shouldClusterMemOps(
    ArrayRef<const MachineOperand *> BaseOps1,
    int64_t Offset1, bool OffsetIsScalable1,
    ArrayRef<const MachineOperand *> BaseOps2,
    int64_t Offset2, bool OffsetIsScalable2,
    unsigned ClusterSize,
    unsigned NumBytes) const {
  assert(BaseOps1.size() == 1 && BaseOps2.size() == 1);
  const MachineOperand *BaseOp1 = BaseOps1.front();
  const MachineOperand *BaseOp2 = BaseOps2.front();
  
  // Cluster loads, not stores
  return BaseOp1->getParent()->mayLoad() && BaseOp2->getParent()->mayLoad();
}

bool V810InstrInfo::getMemOperandsWithOffsetWidth(
    const MachineInstr &MI, SmallVectorImpl<const MachineOperand *> &BaseOps,
    int64_t &Offset, bool &OffsetIsScalable, LocationSize &Width,
    const TargetRegisterInfo *TRI) const {
  OffsetIsScalable = false;
  switch (MI.getOpcode()) {
  case V810::IN_B:
  case V810::LD_B:
  case V810::OUT_B:
  case V810::ST_B:
    Width = LocationSize::precise(1);
    break;
  case V810::IN_H:
  case V810::LD_H:
  case V810::OUT_H:
  case V810::ST_H:
    Width = LocationSize::precise(2);
    break;
  case V810::CAXI:
  case V810::IN_W:
  case V810::LD_W:
  case V810::OUT_W:
  case V810::ST_W:
    Width = LocationSize::precise(4);
    break;
  default:
    return false;
  }
  unsigned BasePos = MI.mayLoad() ? 1 : 0;
  unsigned OffsetPos = BasePos + 1;
  if (!MI.getOperand(OffsetPos).isImm()) {
    return false;
  }
  BaseOps.push_back(&MI.getOperand(BasePos));
  Offset = MI.getOperand(OffsetPos).getImm();
  return true;
}

unsigned V810InstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  if (MI.isInlineAsm()) {
    const MachineFunction *MF = MI.getParent()->getParent();
    const char *AsmStr = MI.getOperand(0).getSymbolName();
    return getInlineAsmLength(AsmStr, *MF->getTarget().getMCAsmInfo());
  }

  unsigned Opcode = MI.getOpcode();
  return get(Opcode).getSize();
}
