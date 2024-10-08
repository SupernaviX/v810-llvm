
# NOTE: Assertions have been autogenerated by utils/update_mca_test_checks.py
# RUN: llvm-mca -mtriple=riscv64 -mcpu=sifive-p470 -iterations=1 < %s | FileCheck %s

# These instructions only support e32

vsetvli zero, zero, e32, mf2, tu, mu
vaesef.vv v4, v8
vaesef.vs v4, v8
vaesem.vv v4, v8
vaesem.vs v4, v8
vaesdm.vv v4, v8
vaesdm.vs v4, v8
vaeskf1.vi v4, v8, 8
vaeskf2.vi v4, v8, 8
vaesz.vs v4, v8

vsetvli zero, zero, e32, m1, tu, mu
vaesef.vv v4, v8
vaesef.vs v4, v8
vaesem.vv v4, v8
vaesem.vs v4, v8
vaesdm.vv v4, v8
vaesdm.vs v4, v8
vaeskf1.vi v4, v8, 8
vaeskf2.vi v4, v8, 8
vaesz.vs v4, v8

vsetvli zero, zero, e32, m2, tu, mu
vaesef.vv v4, v8
vaesef.vs v4, v8
vaesem.vv v4, v8
vaesem.vs v4, v8
vaesdm.vv v4, v8
vaesdm.vs v4, v8
vaeskf1.vi v4, v8, 8
vaeskf2.vi v4, v8, 8
vaesz.vs v4, v8

vsetvli zero, zero, e32, m4, tu, mu
vaesef.vv v4, v8
vaesef.vs v4, v8
vaesem.vv v4, v8
vaesem.vs v4, v8
vaesdm.vv v4, v8
vaesdm.vs v4, v8
vaeskf1.vi v4, v8, 8
vaeskf2.vi v4, v8, 8
vaesz.vs v4, v8

vsetvli zero, zero, e32, m8, tu, mu
vaesef.vv  v8, v16
vaesef.vs  v8, v16
vaesem.vv  v8, v16
vaesem.vs  v8, v16
vaesdm.vv  v8, v16
vaesdm.vs  v8, v16
vaeskf1.vi v8, v16, 8
vaeskf2.vi v8, v16, 8
vaesz.vs   v8, v16

# CHECK:      Iterations:        1
# CHECK-NEXT: Instructions:      50
# CHECK-NEXT: Total Cycles:      142
# CHECK-NEXT: Total uOps:        50

# CHECK:      Dispatch Width:    3
# CHECK-NEXT: uOps Per Cycle:    0.35
# CHECK-NEXT: IPC:               0.35
# CHECK-NEXT: Block RThroughput: 144.0

# CHECK:      Instruction Info:
# CHECK-NEXT: [1]: #uOps
# CHECK-NEXT: [2]: Latency
# CHECK-NEXT: [3]: RThroughput
# CHECK-NEXT: [4]: MayLoad
# CHECK-NEXT: [5]: MayStore
# CHECK-NEXT: [6]: HasSideEffects (U)

# CHECK:      [1]    [2]    [3]    [4]    [5]    [6]    Instructions:
# CHECK-NEXT:  1      1     1.00                  U     vsetvli zero, zero, e32, mf2, tu, mu
# CHECK-NEXT:  1      2     1.00                        vaesef.vv v4, v8
# CHECK-NEXT:  1      2     1.00                        vaesef.vs v4, v8
# CHECK-NEXT:  1      2     1.00                        vaesem.vv v4, v8
# CHECK-NEXT:  1      2     1.00                        vaesem.vs v4, v8
# CHECK-NEXT:  1      2     1.00                        vaesdm.vv v4, v8
# CHECK-NEXT:  1      2     1.00                        vaesdm.vs v4, v8
# CHECK-NEXT:  1      2     1.00                        vaeskf1.vi  v4, v8, 8
# CHECK-NEXT:  1      2     1.00                        vaeskf2.vi  v4, v8, 8
# CHECK-NEXT:  1      2     1.00                        vaesz.vs  v4, v8
# CHECK-NEXT:  1      1     1.00                  U     vsetvli zero, zero, e32, m1, tu, mu
# CHECK-NEXT:  1      2     1.00                        vaesef.vv v4, v8
# CHECK-NEXT:  1      2     1.00                        vaesef.vs v4, v8
# CHECK-NEXT:  1      2     1.00                        vaesem.vv v4, v8
# CHECK-NEXT:  1      2     1.00                        vaesem.vs v4, v8
# CHECK-NEXT:  1      2     1.00                        vaesdm.vv v4, v8
# CHECK-NEXT:  1      2     1.00                        vaesdm.vs v4, v8
# CHECK-NEXT:  1      2     1.00                        vaeskf1.vi  v4, v8, 8
# CHECK-NEXT:  1      2     1.00                        vaeskf2.vi  v4, v8, 8
# CHECK-NEXT:  1      2     1.00                        vaesz.vs  v4, v8
# CHECK-NEXT:  1      1     1.00                  U     vsetvli zero, zero, e32, m2, tu, mu
# CHECK-NEXT:  1      2     2.00                        vaesef.vv v4, v8
# CHECK-NEXT:  1      2     2.00                        vaesef.vs v4, v8
# CHECK-NEXT:  1      2     2.00                        vaesem.vv v4, v8
# CHECK-NEXT:  1      2     2.00                        vaesem.vs v4, v8
# CHECK-NEXT:  1      2     2.00                        vaesdm.vv v4, v8
# CHECK-NEXT:  1      2     2.00                        vaesdm.vs v4, v8
# CHECK-NEXT:  1      2     2.00                        vaeskf1.vi  v4, v8, 8
# CHECK-NEXT:  1      2     2.00                        vaeskf2.vi  v4, v8, 8
# CHECK-NEXT:  1      2     2.00                        vaesz.vs  v4, v8
# CHECK-NEXT:  1      1     1.00                  U     vsetvli zero, zero, e32, m4, tu, mu
# CHECK-NEXT:  1      2     4.00                        vaesef.vv v4, v8
# CHECK-NEXT:  1      2     4.00                        vaesef.vs v4, v8
# CHECK-NEXT:  1      2     4.00                        vaesem.vv v4, v8
# CHECK-NEXT:  1      2     4.00                        vaesem.vs v4, v8
# CHECK-NEXT:  1      2     4.00                        vaesdm.vv v4, v8
# CHECK-NEXT:  1      2     4.00                        vaesdm.vs v4, v8
# CHECK-NEXT:  1      2     4.00                        vaeskf1.vi  v4, v8, 8
# CHECK-NEXT:  1      2     4.00                        vaeskf2.vi  v4, v8, 8
# CHECK-NEXT:  1      2     4.00                        vaesz.vs  v4, v8
# CHECK-NEXT:  1      1     1.00                  U     vsetvli zero, zero, e32, m8, tu, mu
# CHECK-NEXT:  1      2     8.00                        vaesef.vv v8, v16
# CHECK-NEXT:  1      2     8.00                        vaesef.vs v8, v16
# CHECK-NEXT:  1      2     8.00                        vaesem.vv v8, v16
# CHECK-NEXT:  1      2     8.00                        vaesem.vs v8, v16
# CHECK-NEXT:  1      2     8.00                        vaesdm.vv v8, v16
# CHECK-NEXT:  1      2     8.00                        vaesdm.vs v8, v16
# CHECK-NEXT:  1      2     8.00                        vaeskf1.vi  v8, v16, 8
# CHECK-NEXT:  1      2     8.00                        vaeskf2.vi  v8, v16, 8
# CHECK-NEXT:  1      2     8.00                        vaesz.vs  v8, v16

# CHECK:      Resources:
# CHECK-NEXT: [0]   - SiFiveP400Div
# CHECK-NEXT: [1]   - SiFiveP400FEXQ0
# CHECK-NEXT: [2]   - SiFiveP400FloatDiv
# CHECK-NEXT: [3]   - SiFiveP400IEXQ0
# CHECK-NEXT: [4]   - SiFiveP400IEXQ1
# CHECK-NEXT: [5]   - SiFiveP400IEXQ2
# CHECK-NEXT: [6]   - SiFiveP400Load
# CHECK-NEXT: [7]   - SiFiveP400Store
# CHECK-NEXT: [8]   - SiFiveP400VDiv
# CHECK-NEXT: [9]   - SiFiveP400VEXQ0
# CHECK-NEXT: [10]  - SiFiveP400VFloatDiv
# CHECK-NEXT: [11]  - SiFiveP400VLD
# CHECK-NEXT: [12]  - SiFiveP400VST

# CHECK:      Resource pressure per iteration:
# CHECK-NEXT: [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    [9]    [10]   [11]   [12]
# CHECK-NEXT:  -      -      -      -     5.00    -      -      -      -     144.00  -      -      -

# CHECK:      Resource pressure by instruction:
# CHECK-NEXT: [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    [9]    [10]   [11]   [12]   Instructions:
# CHECK-NEXT:  -      -      -      -     1.00    -      -      -      -      -      -      -      -     vsetvli  zero, zero, e32, mf2, tu, mu
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesef.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesef.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesem.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesem.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesdm.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesdm.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaeskf1.vi v4, v8, 8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaeskf2.vi v4, v8, 8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesz.vs v4, v8
# CHECK-NEXT:  -      -      -      -     1.00    -      -      -      -      -      -      -      -     vsetvli  zero, zero, e32, m1, tu, mu
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesef.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesef.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesem.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesem.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesdm.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesdm.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaeskf1.vi v4, v8, 8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaeskf2.vi v4, v8, 8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     1.00    -      -      -     vaesz.vs v4, v8
# CHECK-NEXT:  -      -      -      -     1.00    -      -      -      -      -      -      -      -     vsetvli  zero, zero, e32, m2, tu, mu
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     2.00    -      -      -     vaesef.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     2.00    -      -      -     vaesef.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     2.00    -      -      -     vaesem.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     2.00    -      -      -     vaesem.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     2.00    -      -      -     vaesdm.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     2.00    -      -      -     vaesdm.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     2.00    -      -      -     vaeskf1.vi v4, v8, 8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     2.00    -      -      -     vaeskf2.vi v4, v8, 8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     2.00    -      -      -     vaesz.vs v4, v8
# CHECK-NEXT:  -      -      -      -     1.00    -      -      -      -      -      -      -      -     vsetvli  zero, zero, e32, m4, tu, mu
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     4.00    -      -      -     vaesef.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     4.00    -      -      -     vaesef.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     4.00    -      -      -     vaesem.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     4.00    -      -      -     vaesem.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     4.00    -      -      -     vaesdm.vv  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     4.00    -      -      -     vaesdm.vs  v4, v8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     4.00    -      -      -     vaeskf1.vi v4, v8, 8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     4.00    -      -      -     vaeskf2.vi v4, v8, 8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     4.00    -      -      -     vaesz.vs v4, v8
# CHECK-NEXT:  -      -      -      -     1.00    -      -      -      -      -      -      -      -     vsetvli  zero, zero, e32, m8, tu, mu
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     8.00    -      -      -     vaesef.vv  v8, v16
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     8.00    -      -      -     vaesef.vs  v8, v16
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     8.00    -      -      -     vaesem.vv  v8, v16
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     8.00    -      -      -     vaesem.vs  v8, v16
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     8.00    -      -      -     vaesdm.vv  v8, v16
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     8.00    -      -      -     vaesdm.vs  v8, v16
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     8.00    -      -      -     vaeskf1.vi v8, v16, 8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     8.00    -      -      -     vaeskf2.vi v8, v16, 8
# CHECK-NEXT:  -      -      -      -      -      -      -      -      -     8.00    -      -      -     vaesz.vs v8, v16

