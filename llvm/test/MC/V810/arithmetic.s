# RUN: llvm-mc %s -arch=v810 -filetype=obj | llvm-objdump -dr - | FileCheck %s

.lbl_start:
    nop
    nop
    nop
.lbl_end:
    # CHECK: movea 0x6, r0, r6
    movea lo(.lbl_end - .lbl_start), r0, r6