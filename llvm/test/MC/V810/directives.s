# RUN: llvm-mc %s -arch=v810 -filetype=obj | llvm-objdump -dr - | FileCheck %s

.set R_ram_base,r20
.set R_ram_head,R_ram_base

    #CHECK: st.b r0, 0x4[r20]
    st.b r0, 4[R_ram_base]

    #CHECK: st.b r0, 0x8[r20]
    st.b r0, 8[R_ram_head]