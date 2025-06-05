#include "V810RegisterBankInfo.h"

#include "MCTargetDesc/V810MCTargetDesc.h"
#include "llvm/CodeGen/RegisterBank.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#define GET_TARGET_REGBANK_IMPL
#include "V810GenRegisterBank.inc"

using namespace llvm;

const RegisterBankInfo::InstructionMapping &
V810RegisterBankInfo::getInstrMapping(const MachineInstr &MI) const {
  const auto &Mapping = getInstrMappingImpl(MI);
  if (Mapping.isValid())
    return Mapping;

  const auto &MRI = MI.getMF()->getRegInfo();
  unsigned NumOperands = MI.getNumOperands();

  SmallVector<const ValueMapping *, 8> ValMappings(NumOperands);
  for (const auto &I : enumerate(MI.operands())) {
    if (!I.value().isReg())
      continue;
    // Only the destination is expected for PHIs.
    if (MI.isPHI() && I.index() == 1) {
      NumOperands = 1;
      break;
    }
    LLT Ty = MRI.getType(I.value().getReg());
    if (!Ty.isValid())
      continue;
    ValMappings[I.index()] =
        &getValueMapping(0, Ty.getSizeInBits(), V810::AnyRegBank);
  }
  return getInstructionMapping(/*ID=*/1, /*Cost=*/1,
                               getOperandsMapping(ValMappings), NumOperands);
}

void V810RegisterBankInfo::applyMappingImpl(
    MachineIRBuilder &Builder, const OperandsMapper &OpdMapper) const {
  applyDefaultMapping(OpdMapper);
}

const RegisterBank &
V810RegisterBankInfo::getRegBankFromRegClass(const TargetRegisterClass &RC,
                                             LLT Ty) const {
  return V810::AnyRegBank;
}