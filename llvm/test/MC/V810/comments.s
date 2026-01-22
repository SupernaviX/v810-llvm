# RUN: llvm-mc %s -arch=v810 -filetype=obj | llvm-objdump -dr - | FileCheck %s

    # CHECK: st.h r0, 0x0[r6]
    st.h r0, /* ignore this comment */ [r6] /* and this one */