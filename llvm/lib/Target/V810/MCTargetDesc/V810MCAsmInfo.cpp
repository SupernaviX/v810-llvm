#include "V810MCAsmInfo.h"

using namespace llvm;

void V810AsmInfo::anchor() {}

V810AsmInfo::V810AsmInfo(const Triple &TheTriple) {
  CodePointerSize = 4;
  MinInstAlignment = 2;
  MaxInstLength = 4;
  IsLittleEndian = true;
  SupportsDebugInformation = true;
  UsesCFIWithoutEH = true;
}
