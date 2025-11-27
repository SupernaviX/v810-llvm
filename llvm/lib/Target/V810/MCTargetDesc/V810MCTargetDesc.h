#ifndef LLVM_LIB_TARGET_V810_MCTARGETDESC_V810MCTARGETDESC_H
#define LLVM_LIB_TARGET_V810_MCTARGETDESC_V810MCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;

MCCodeEmitter *createV810MCCodeEmitter(const MCInstrInfo &MCII,
                                       MCContext &Ctx);
MCAsmBackend *createV810AsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                   const MCRegisterInfo &MRI,
                                   const MCTargetOptions &Options);
std::unique_ptr<MCObjectTargetWriter> createV810ObjectWriter(uint8_t OSABI);

} // End llvm namespace

// Defines symbolic names for V810 registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "V810GenRegisterInfo.inc"

// Defines symbolic names for the Sparc instructions.
//
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "V810GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "V810GenSubtargetInfo.inc"

#endif