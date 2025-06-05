#ifndef LLVM_LIB_TARGET_V810_GISEL_V810REGISTERBANKINFO_H
#define LLVM_LIB_TARGET_V810_GISEL_V810REGISTERBANKINFO_H

#include "llvm/CodeGen/RegisterBankInfo.h"

#define GET_REGBANK_DECLARATIONS
#include "V810GenRegisterBank.inc"

namespace llvm {

class V810GenRegisterBankInfo : public RegisterBankInfo {
protected:
#define GET_TARGET_REGBANK_CLASS
#include "V810GenRegisterBank.inc"
};


class V810RegisterBankInfo final : public V810GenRegisterBankInfo {
public:
  const InstructionMapping &
  getInstrMapping(const MachineInstr &MI) const override;

  void applyMappingImpl(MachineIRBuilder &Builder,
                        const OperandsMapper &OpdMapper) const override;

  const RegisterBank &getRegBankFromRegClass(const TargetRegisterClass &RC,
                                             LLT Ty) const override;
};

}

#endif