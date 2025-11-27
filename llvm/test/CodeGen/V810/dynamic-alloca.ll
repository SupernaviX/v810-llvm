; RUN: llc < %s | FileCheck %s -check-prefix=V810

define ptr @test_dynamic_alloca(i32 %N) {
    %P = alloca i32, i32 %N
    ret ptr %P
}

; V810-LABEL: test_dynamic_alloca

; Get the prolog right
; V810: add -8, r3
; V810: st.w r31, 4[r3]
; V810: st.w r2, 0[r3]
; V810: mov r3, r2

; Get the epilog right
; V810: ld.w 4[r2], r31
; V810: movea 8, r2, r3
; V810: ld.w 0[r2], r2
; V810: jmp [r31]