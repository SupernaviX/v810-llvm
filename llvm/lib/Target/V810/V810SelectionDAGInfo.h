#ifndef LLVM_LIB_TARGET_V810_V810SELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_V810_V810SELECTIONDAGINFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

namespace llvm {

class V810SelectionDAGInfo : public SelectionDAGTargetInfo {
public:
  explicit V810SelectionDAGInfo() = default;

  SDValue EmitTargetCodeForMemcpy(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Dst, SDValue Src,
                                  SDValue Size, Align Alignment,
                                  bool isVolatile, bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo,
                                  MachinePointerInfo SrcPtrInfo) const override;
};

}

#endif
