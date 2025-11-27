class CCIfArgIsVarArg<CCAction A>
  : CCIf<"State.isVarArg() && "
         "ValNo >= static_cast<V810CCState&>(State).getNumFixedParams()",
         A>;

def CC_V810 : CallingConv<[
  CCIfArgIsVarArg<CCAssignToStack<4, 4>>,
  CCIfType<[i8, i16, i32, f32], CCAssignToReg<[R6, R7, R8, R9]>>,
  CCAssignToStack<4, 4>
]>;

def RetCC_V810 : CallingConv<[
  CCIfType<[i8, i16, i32, f32], CCAssignToReg<[R10, R11]>>
]>;

// Don't mark R3 as callee-saved because it messes up stack frames
// This is Fine because only stack frame code ever edits that
def CSR : CalleeSavedRegs<(add R1, R4, R5, (sequence "R%u", 20, 29), R31, R2)>;
def CSR_NoRegs : CalleeSavedRegs<(add)>;
def CSR_Interrupt : CalleeSavedRegs<(add R1, (sequence "R%u", 4, 31), R2)>;