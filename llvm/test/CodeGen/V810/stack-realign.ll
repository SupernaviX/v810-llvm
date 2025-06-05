; RUN: llc < %s -mtriple=v810 -global-isel | FileCheck %s -check-prefix=V810

define void @test_stack_realign(i32 %value) nounwind {
entry:
  %overaligned = alloca i32, align 8
  store i32 %value, ptr %overaligned
  ret void
}

; V810-LABEL: test_stack_realign

; Test aligning the stack
; V810: add -16, r3
; V810: st.w r31, 12[r3]
; V810: st.w r2, 8[r3]
; V810: movea 8, r3, r2
; V810: andi 7, r3, r31
; V810: sub r31, r3

; Test saving to an aligned address
; V810: st.w r6, 0[r3]

; Test restoring the stack
; V810: ld.w 4[r2], r31
; V810: movea 8, r2, r3
; V810: ld.w 0[r2], r2
; V810: jmp [r31]

define void @test_stack_realign_with_params(i32 %p1, i32 %p2, i32 %p3, i32 %p4, i32 %p5) nounwind {
entry:
  %overaligned = alloca i32, align 8
  store i32 %p5, ptr %overaligned
  ret void
}

; V810-LABEL: test_stack_realign_with_params

; V810: ld.w 8[r2], r6
; V810: st.w r6, 0[r3]
