# RUN: llvm-mc %s -arch=v810 -filetype=obj | llvm-objdump -dr - | FileCheck %s

    /* this one on its own initial line */
    /* also, this one */
    # CHECK: st.h r0, 0x0[r6]
    st.h r0, /* ignore this comment */ [r6] /* and this one */
    /* and this one on its own line */
