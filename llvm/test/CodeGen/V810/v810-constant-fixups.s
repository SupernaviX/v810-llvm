# Check that we resolve constant fixups (which don't need the linker) correctly

# RUN: llvm-mc -filetype=obj -triple=v810-unknown-vb %s -o - \
# RUN:   | llvm-objdump --disassemble - | FileCheck %s

    movhi hi(-21845), r0, r1
    movea lo(-21845), r1, r1
    andi hi(0x0005f820), r0, r6
    ori lo(0x0005f820), r6, r6
    movhi hi(0x0005f820), r0, r7
    movea lo(0x0005f820), r7, r7
    movhi hi(0x00100020), r0, r8
    movea lo(0x00100020), r8, r8
    movhi hi(address_ - address), r0, r9
    movea lo(address_ - address), r9, r9
    movhi 0x10 + 3, r0, r10
    movea 0x37 * 0x100, r10, r10
    movea address_ - address, r0, r11
    in.b address_ - address[r12], r13
    jr address_
address:
    .ascii "abcd"
address_:

# CHECK: movhi 0x0, r0, r1
# CHECK: movea 0xaaab, r1, r1
# CHECK: andi 0x6, r0, r6
# CHECK: ori 0xf820, r6, r6
# CHECK: movhi 0x6, r0, r7
# CHECK: movea 0xf820, r7, r7
# CHECK: movhi 0x10, r0, r8
# CHECK: movea 0x20, r8, r8
# CHECK: movhi 0x0, r0, r9
# CHECK: movea 0x4, r9, r9
# CHECK: movhi 0x13, r0, r10
# CHECK: movea 0x3700, r10, r10
# CHECK: movea 0x4, r0, r11
# CHECK: in.b 0x4[r12], r13
# CHECK: jr 0x40
