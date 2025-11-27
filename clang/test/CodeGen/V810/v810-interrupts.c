// RUN: %clang_cc1 -triple v810 -emit-llvm %s -o - -O2 | FileCheck %s

__attribute__((interrupt))
void some_interrupt(void) {

}

// CHECK: attributes #0
// CHECK-SAME: "interrupt"
