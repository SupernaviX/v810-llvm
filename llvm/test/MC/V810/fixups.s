# RUN: llvm-mc %s -arch=v810 -filetype=obj | llvm-objdump -dr - | FileCheck %s

.set literal, 0x456789ab

    # CHECK: movhi 0x4568, r0, r2
    movhi hi(literal), r0, r2
    # CHECK: movea 0x89ab, r2, r2
    movea lo(literal), r2, r2

.global someobj

    # CHECK: movhi 0x0, r0, r2
    # CHECK: R_V810_HI_S someobj
    movhi hi(someobj), r0, r2
    # CHECK: movea 0x0, r2, r2
    # CHECK: R_V810_LO someobj
    movea lo(someobj), r2, r2

    # CHECK: ld.w 0x0[r4], r2
    # CHECK: R_V810_SDAOFF someobj
    ld.w sdaoff(someobj)[r4], r2

    # CHECK: br 0x
    # CHECK: R_V810_9_PCREL someobj
    br someobj

    # CHECK: jr 0x
    # CHECK: R_V810_26_PCREL someobj
    jr someobj