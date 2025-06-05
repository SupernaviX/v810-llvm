#ifndef LLVM_LIB_TARGET_V810_V810CALLINGCONV_H
#define LLVM_LIB_TARGET_V810_V810CALLINGCONV_H

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/IR/CallingConv.h"

namespace llvm {

struct V810CCState : public CCState {
  unsigned NumFixedParams = 0;

  V810CCState(CallingConv::ID CC, bool IsVarArg, MachineFunction &MF,
              SmallVectorImpl<CCValAssign> &locs, LLVMContext &C,
              unsigned NumFixedParams)
      : CCState(CC, IsVarArg, MF, locs, C),
        NumFixedParams(NumFixedParams) {}
  unsigned getNumFixedParams() const { return NumFixedParams; }
};

bool RetCC_V810(unsigned ValNo, MVT ValVT, MVT LocVT,
                CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                CCState &State);
bool CC_V810(unsigned ValNo, MVT ValVT, MVT LocVT,
             CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
             CCState &State);
} // End llvm namespace
#endif