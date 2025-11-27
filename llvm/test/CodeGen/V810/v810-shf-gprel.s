# Check that .sdata and .sbss sections have SHF_V810_GPREL flags
# and proper section types.

# RUN: llvm-mc -filetype=obj -triple=v810-unknown-vb %s -o - \
# RUN:   | llvm-readobj -S - | FileCheck %s

  .sdata
  .word 0

  .sbss
  .zero 4

# CHECK:      Name: .sdata
# CHECK-NEXT: Type: SHT_PROGBITS
# CHECK-NEXT: Flags [ (0x10000003)
# CHECK-NEXT:   SHF_ALLOC
# CHECK-NEXT:   SHF_V810_GPREL
# CHECK-NEXT:   SHF_WRITE
# CHECK-NEXT: ]

# CHECK:      Name: .sbss
# CHECK-NEXT: Type: SHT_NOBITS
# CHECK-NEXT: Flags [ (0x10000003)
# CHECK-NEXT:   SHF_ALLOC
# CHECK-NEXT:   SHF_V810_GPREL
# CHECK-NEXT:   SHF_WRITE
# CHECK-NEXT: ]
