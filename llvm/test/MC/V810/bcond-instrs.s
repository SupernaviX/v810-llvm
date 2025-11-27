# RUN: llvm-mc %s -arch=v810 | FileCheck %s

targetlbl:

# CHECK: bv targetlbl
bv targetlbl
# CHECK: setf v, r0
setf v, r0
# CHECK: setf v, r1
setfv r1

# CHECK: bc targetlbl
bc targetlbl
# CHECK: setf c, r0
setf c, r0
# CHECK: setf c, r1
setfc r1

# CHECK: bc targetlbl
bl targetlbl
# CHECK: setf c, r0
setf l, r0
# CHECK: setf c, r1
setfl r1

# CHECK: be targetlbl
be targetlbl
# CHECK: setf e, r0
setf e, r0
# CHECK: setf e, r1
setfe r1

# CHECK: be targetlbl
bz targetlbl
# CHECK: setf e, r0
setf z, r0
# CHECK: setf e, r1
setfz r1

# CHECK: bnh targetlbl
bnh targetlbl
# CHECK: setf nh, r0
setf nh, r0
# CHECK: setf nh, r1
setfnh r1

# CHECK: bn targetlbl
bn targetlbl
# CHECK: setf n, r0
setf n, r0
# CHECK: setf n, r1
setfn r1

# CHECK: br targetlbl
br targetlbl
# CHECK: setf t, r0
setf t, r0
# CHECK: setf t, r1
setft r1

# CHECK: blt targetlbl
blt targetlbl
# CHECK: setf lt, r0
setf lt, r0
# CHECK: setf lt, r1
setflt r1

# CHECK: ble targetlbl
ble targetlbl
# CHECK: setf le, r0
setf le, r0
# CHECK: setf le, r1
setfle r1

# CHECK: bnv targetlbl
bnv targetlbl
# CHECK: setf nv, r0
setf nv, r0
# CHECK: setf nv, r1
setfnv r1

# CHECK: bnc targetlbl
bnc targetlbl
# CHECK: setf nc, r0
setf nc, r0
# CHECK: setf nc, r1
setfnc r1

# CHECK: bnc targetlbl
bnl targetlbl
# CHECK: setf nc, r0
setf nl, r0
# CHECK: setf nc, r1
setfnl r1

# CHECK: bne targetlbl
bne targetlbl
# CHECK: setf ne, r0
setf ne, r0
# CHECK: setf ne, r1
setfne r1

# CHECK: bne targetlbl
bnz targetlbl
# CHECK: setf ne, r0
setf nz, r0
# CHECK: setf ne, r1
setfnz r1

# CHECK: bh targetlbl
bh targetlbl
# CHECK: setf h, r0
setf h, r0
# CHECK: setf h, r1
setfh r1

# CHECK: bp targetlbl
bp targetlbl
# CHECK: setf p, r0
setf p, r0
# CHECK: setf p, r1
setfp r1

# CHECK: nop
nop
# CHECK: setf f, r0
setf f, r0
# CHECK: setf f, r1
setff r1

# CHECK: bge targetlbl
bge targetlbl
# CHECK: setf ge, r0
setf ge, r0
# CHECK: setf ge, r1
setfge r1

# CHECK: bgt targetlbl
bgt targetlbl
# CHECK: setf gt, r0
setf gt, r0
# CHECK: setf gt, r1
setfgt r1
