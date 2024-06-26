; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py UTC_ARGS: --version 2
; RUN: llc < %s -march=v810 -frame-pointer=all | FileCheck %s

declare i32 @otherFunc(i32 %input)

define i32 @callingConv(i32 %input) {
; CHECK-LABEL: callingConv:
; CHECK:       # %bb.0:
; CHECK-NEXT:    add -8, r3
; CHECK-NEXT:    st.w r31, 4[r3] # 4-byte Folded Spill
; CHECK-NEXT:    st.w r2, 0[r3] # 4-byte Folded Spill
; CHECK-NEXT:    mov r3, r2
; CHECK-NEXT:    jal otherFunc
; CHECK-NEXT:    add 3, r10
; CHECK-NEXT:    ld.w 0[r3], r2 # 4-byte Folded Reload
; CHECK-NEXT:    ld.w 4[r3], r31 # 4-byte Folded Reload
; CHECK-NEXT:    add 8, r3
; CHECK-NEXT:    jmp [r31]
    %out = call i32 @otherFunc(i32 %input)
    %res = add i32 %out, 3
    ret i32 %res
}

