# Check that we resolve constant fixups (which don't need the linker) correctly

# RUN: llvm-mc -filetype=obj -triple=v810-unknown-vb %s -o - \
# RUN:   | llvm-objdump --disassemble - | FileCheck %s

    movhi hi(0x00100020), r0, r8
    movea lo(0x00100020), r8, r8
    movhi hi(address_ - address), r0, r9
    movea lo(address_ - address), r9, r9
    jr address_
address:
    .ascii "abcd"
address_:

# CHECK: movhi 0x10, r0, r8
# CHECK: movea 0x20, r8, r8
# CHECK: movhi 0x0, r0, r9
# CHECK: movea 0x4, r9, r9
# CHECK: jr 0x18
