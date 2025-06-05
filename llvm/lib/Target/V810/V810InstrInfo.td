include "V810InstrFormats.td"

def IsNintendo : Predicate<"Subtarget->isNintendo()">,
                 AssemblerPredicate<(all_of FeatureNintendo)>;

// split a 32-bit immediate into low and high bits
def IMM32_LO16 : SDNodeXForm<imm, [{
  int64_t lo = EvalLo(N->getSExtValue());
  return CurDAG->getTargetConstant(lo, SDLoc(N), MVT::i32);
}]>;
def IMM32_HI16 : SDNodeXForm<imm, [{
  int64_t hi = EvalHi(N->getSExtValue());
  return CurDAG->getTargetConstant(hi, SDLoc(N), MVT::i32);
}]>;

// grab the high 16 bits from a 32-bit immediate
def HI16 : SDNodeXForm<imm, [{
  int64_t hi = EvalHi0(N->getSExtValue());
  return CurDAG->getTargetConstant(hi, SDLoc(N), MVT::i32);
}]>;

def uimm5 : ImmLeaf<i32, [{ return isUInt<5>(Imm); }]>;
def simm5 : ImmLeaf<i32, [{ return isInt<5>(Imm); }]>;
def uimm16 : ImmLeaf<i32, [{ return isUInt<16>(Imm); }]>;
def simm16 : ImmLeaf<i32, [{ return isInt<16>(Imm); }]>;
def imm32 : ImmLeaf<i32, [{ return isUInt<32>(Imm) || isInt<32>(Imm); }]>;
def MOVHIimm : ImmLeaf<i32, [{
  return isShiftedUInt<16, 16>(Imm);
}], HI16>;

def ADDRri : ComplexPattern<i32, 2, "SelectADDRri", [frameindex], []>;
def gi_ADDRri : GIComplexOperandMatcher<s32, "selectAddrRI">,
                GIComplexPatternEquiv<ADDRri>;

def V810MEMriAsmOperand : AsmOperandClass {
  let Name = "MEMri";
  let ParserMethod = "parseMEMOperand";
}

def V810BranchTargetAsmOperand : AsmOperandClass {
  let Name = "BranchTarget";
  let ParserMethod = "parseBranchTargetOperand";
}

def V810BcondTargetAsmOperand : AsmOperandClass {
  let Name = "BcondTarget";
  let ParserMethod = "parseBcondTargetOperand";
}

def V810CondAsmOperand : AsmOperandClass {
  let Name = "Cond";
  let ParserMethod = "parseCondOperand";
}

def uimm5Op : Operand<i32> { let DecoderMethod = "DecodeUIMM<5>"; }
def simm5Op : Operand<i32> { let DecoderMethod = "DecodeSIMM<5>"; }
def uimm16Op : Operand<i32> { let DecoderMethod = "DecodeUIMM<16>"; }
def simm16Op : Operand<i32> { let DecoderMethod = "DecodeSIMM<16>"; }

def MEMri : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GenRegs, i16imm);
  let ParserMatchClass = V810MEMriAsmOperand;
}
def calltarget : Operand<iPTR> {
  let PrintMethod = "printBranchOperand<26>";
  let OperandType = "OPERAND_PCREL";
  let DecoderMethod = "DecodeCall";
  let EncoderMethod = "getBranchTargetOpValue";
  let ParserMatchClass = V810BranchTargetAsmOperand;
}
def brtarget : Operand<OtherVT> {
  let PrintMethod = "printBranchOperand<26>";
  let OperandType = "OPERAND_PCREL";
  let EncoderMethod = "getBranchTargetOpValue";
  let ParserMatchClass = V810BranchTargetAsmOperand;
}
def bcondtarget : Operand<OtherVT> {
  let PrintMethod = "printBranchOperand<9>";
  let OperandType = "OPERAND_PCREL";
  let EncoderMethod = "getBcondTargetOpValue";
  let ParserMatchClass = V810BcondTargetAsmOperand;
}
def condcode : Operand<i32> {
  let PrintMethod = "printCondOperand";
  let ParserMatchClass = V810CondAsmOperand;
}

def V810Hi  : SDNode<"V810ISD::HI", SDTIntUnaryOp>;
def V810Lo  : SDNode<"V810ISD::LO", SDTIntBinOp>;
def V810RegRelative : SDNode<"V810ISD::REG_RELATIVE", SDTIntBinOp>;

def V810Smullohi      : SDNode<"V810ISD::SMUL_LOHI", SDTIntBinOp, [SDNPCommutative, SDNPOutGlue]>;
def V810Umullohi      : SDNode<"V810ISD::UMUL_LOHI", SDTIntBinOp, [SDNPCommutative, SDNPOutGlue]>;
def V810Sdivrem       : SDNode<"V810ISD::SDIVREM", SDTIntBinOp, [SDNPOutGlue]>;
def V810Udivrem       : SDNode<"V810ISD::UDIVREM", SDTIntBinOp, [SDNPOutGlue]>;
def V810Caxi          : SDNode<"V810ISD::CAXI", SDTAtomic3, [SDNPHasChain, SDNPMayStore, SDNPMayLoad, SDNPMemOperand]>;

def SDT_V810Cmp       : SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>, SDTCisInt<0>]>;
def V810CMP           : SDNode<"V810ISD::CMP", SDT_V810Cmp, [SDNPHasChain, SDNPOutGlue]>;
def SDT_V810FCmp      : SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>, SDTCisFP<0>]>;
def V810FCMP          : SDNode<"V810ISD::FCMP", SDT_V810FCmp, [SDNPHasChain, SDNPOutGlue]>;
def SDT_V810Bcond     : SDTypeProfile<0, 2, [SDTCisInt<0>, SDTCisVT<1, OtherVT>]>;
def V810Bcond         : SDNode<"V810ISD::BCOND", SDT_V810Bcond, [SDNPHasChain, SDNPInGlue]>;
def SDT_V810Setf      : SDTypeProfile<1, 1, [SDTCisInt<0>, SDTCisInt<1>]>;
def V810Setf          : SDNode<"V810ISD::SETF", SDT_V810Setf, [SDNPInGlue]>;
def SDT_V810Selectcc  : SDTypeProfile<1, 3, [SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>, SDTCisVT<3, i32>]>;
def V810Selectcc      : SDNode<"V810ISD::SELECT_CC", SDT_V810Selectcc, [SDNPInGlue]>;

def SDT_V810Call  : SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>;
def call          : SDNode<"V810ISD::CALL", SDT_V810Call,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                            SDNPVariadic]>;
def tailcall      : SDNode<"V810ISD::TAIL_CALL", SDT_V810Call,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                            SDNPVariadic]>;
def SDT_V810Ret   : SDTypeProfile<0, 0, []>;
def retglue       : SDNode<"V810ISD::RET_GLUE", SDT_V810Ret,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def retiglue      : SDNode<"V810ISD::RETI_GLUE", SDT_V810Ret,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def SDT_SPCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_SPCallSeqEnd : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_SPCallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDT_SPCallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstV810<outs, ins, asmstr, pattern> {
  let isCodeGenOnly = 1;
  let isPseudo = 1;
  let hasNoSchedulingInfo = 1;
}

def V810_CMP :
      Pseudo<(outs),
             (ins type0:$r2, type0:$r1),
             "CMP_Pseudo $r2, $r1", []>;
def : GINodeEquiv<V810_CMP, V810CMP>;

// Conditions
// Keep this in sync with the values in V810.h
class CC_VAL<int N> : PatLeaf<(i32 N)>;
def CC_V   : CC_VAL<0>; // Overflow
def CC_C   : CC_VAL<1>; // Carry/Lower (unsigned)
def CC_E   : CC_VAL<2>; // Equal/Zero
def CC_NH  : CC_VAL<3>; // Not higher (unsigned)
def CC_N   : CC_VAL<4>; // Negative
def CC_BR  : CC_VAL<5>; // Always (unconditional branch)
def CC_LT  : CC_VAL<6>; // Less than (signed)
def CC_LE  : CC_VAL<7>; // Less than or equal (signed)
def CC_NV  : CC_VAL<8>; // Not overflow
def CC_NC  : CC_VAL<9>; // Not carry/lower (unsigned)
def CC_NE  : CC_VAL<10>; // Not equal/zero
def CC_H   : CC_VAL<11>; // Nigher (unsigned)
def CC_P   : CC_VAL<12>; // Positive
def CC_NOP : CC_VAL<13>; // Never (nop)
def CC_GE  : CC_VAL<14>; // Greater than or equal (signed)
def CC_GT  : CC_VAL<15>; // Greater than

let mayLoad = 1, Itinerary = IIC_Load in {
  def LD_W : FormatVI<0b110011,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "ld.w $addr, $r2",
    [(set i32:$r2, (load ADDRri:$addr))],
    [WriteLoad]
  >;
  def IN_W : FormatVI<0b111011,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "in.w $addr, $r2",
    [(set i32:$r2, (load ADDRri:$addr))],
    [WriteLoad]
  >;
  def LD_H : FormatVI<0b110001,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "ld.h $addr, $r2",
    [(set i32:$r2, (sextloadi16 ADDRri:$addr))],
    [WriteLoad]
  >;
  def IN_H : FormatVI<0b111001,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "in.h $addr, $r2",
    [(set i32:$r2, (zextloadi16 ADDRri:$addr))],
    [WriteLoad]
  >;
  def LD_B : FormatVI<0b110000,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "ld.b $addr, $r2",
    [(set i32:$r2, (sextloadi8 ADDRri:$addr))],
    [WriteLoad]
  >;
  def IN_B : FormatVI<0b111000,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "in.b $addr, $r2",
    [(set i32:$r2, (zextloadi8 ADDRri:$addr))],
    [WriteLoad]
  >;
}

let mayStore = 1, Itinerary = IIC_Store in {
  def ST_W : FormatVI<0b110111,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "st.w $r2, $addr",
    [(store i32:$r2, ADDRri:$addr)],
    [WriteStore]
  >;
  def OUT_W : FormatVI<0b111111,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "out.w $r2, $addr",
    [(store i32:$r2, ADDRri:$addr)],
    [WriteStore]
  >;
  def ST_H : FormatVI<0b110101,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "st.h $r2, $addr",
    [(truncstorei16 i32:$r2, ADDRri:$addr)],
    [WriteStore]
  >;
  def OUT_H : FormatVI<0b111101,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "out.h $r2, $addr",
    [(truncstorei16 i32:$r2, ADDRri:$addr)],
    [WriteStore]
  >;
  def ST_B : FormatVI<0b110100,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "st.b $r2, $addr",
    [(truncstorei8 i32:$r2, ADDRri:$addr)],
    [WriteStore]
  >;
  def OUT_B : FormatVI<0b111100,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "out.b $r2, $addr",
    [(truncstorei8 i32:$r2, ADDRri:$addr)],
    [WriteStore]
  >;
}

let isMoveImm = 1, isReMaterializable = 1, isAsCheapAsAMove = 1 in
def MOVi : FormatII<0b010000,
  (outs GenRegs:$r2), (ins simm5Op:$imm),
  "mov $imm, $r2",
  [(set i32:$r2, simm5:$imm)],
  [WriteI]
>;
let isMoveReg = 1, isReMaterializable = 1, isAsCheapAsAMove = 1  in
def MOVr : FormatI<0b000000,
  (outs GenRegs:$r2), (ins GenRegs:$r1),
  "mov $r1, $r2",
  [], // no pattern matching needed because copyPhysReg handles it
  [WriteI]
>;
let isMoveImm = 1, isMoveReg = 1, isReMaterializable = 1, isAsCheapAsAMove = 1 in {
  def MOVHI : FormatV<0b101111,
    (outs GenRegs:$r2), (ins GenRegs:$r1, simm16Op:$imm),
    "movhi $imm, $r1, $r2",
    [], // no pattern matching because this is very situational
    [WriteI]
  >;
  def MOVEA : FormatV<0b101000,
    (outs GenRegs:$r2), (ins GenRegs:$r1, simm16Op:$imm),
    "movea $imm, $r1, $r2",
    [(set i32:$r2, (add GenRegs:$r1, simm16:$imm))],
    [WriteI]
  >;
}

// Bit ops
let Defs = [SR5], V810_ZFlag = 1, V810_SFlag = 1 in {
  def ANDI : FormatV<0b101101,
    (outs GenRegs:$r2), (ins GenRegs:$r1, uimm16Op:$imm),
    "andi $imm, $r1, $r2",
    [(set i32:$r2, (and GenRegs:$r1, uimm16:$imm))],
    [WriteALU]
  >;
  let Constraints = "$r2 = $r2_" in
  def AND : FormatI<0b001101,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "and $r1, $r2",
    [(set i32:$r2, (and GenRegs:$r2_, GenRegs:$r1))],
    [WriteALU]
  >;

  def ORI : FormatV<0b101100,
    (outs GenRegs:$r2), (ins GenRegs:$r1, uimm16Op:$imm),
    "ori $imm, $r1, $r2",
    [(set i32:$r2, (or GenRegs:$r1, uimm16:$imm))],
    [WriteALU]
  >;
  let Constraints = "$r2 = $r2_" in
  def OR : FormatI<0b001100,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "or $r1, $r2",
    [(set i32:$r2, (or GenRegs:$r2_, GenRegs:$r1))],
    [WriteALU]
  >;

  def XORI : FormatV<0b101110,
    (outs GenRegs:$r2), (ins GenRegs:$r1, uimm16Op:$imm),
    "xori $imm, $r1, $r2",
    [(set i32:$r2, (xor GenRegs:$r1, uimm16:$imm))],
    [WriteALU]
  >;
  let Constraints = "$r2 = $r2_" in
  def XOR : FormatI<0b001110,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "xor $r1, $r2",
    [(set i32:$r2, (xor GenRegs:$r2_, GenRegs:$r1))],
    [WriteALU]
  >;

  def NOT : FormatI<0b001111,
    (outs GenRegs:$r2), (ins GenRegs:$r1),
    "not $r1, $r2",
    [(set i32:$r2, (not GenRegs:$r1))],
    [WriteALU]
  >;

  let Constraints = "$r2 = $r1" in
  def SHLi : FormatII<0b010100,
    (outs GenRegs:$r2), (ins GenRegs:$r1, uimm5Op:$imm),
    "shl $imm, $r2",
    [(set i32:$r2, (shl GenRegs:$r1, uimm5:$imm))],
    [WriteALU]
  >;
  let Constraints = "$r2 = $r2_" in
  def SHLr : FormatI<0b000100,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "shl $r1, $r2",
    [(set i32:$r2, (shl GenRegs:$r2_, GenRegs:$r1))],
    [WriteALU]
  >;

  let Constraints = "$r2 = $r1" in
  def SHRi : FormatII<0b010101,
    (outs GenRegs:$r2), (ins GenRegs:$r1, uimm5Op:$imm),
    "shr $imm, $r2",
    [(set i32:$r2, (srl GenRegs:$r1, uimm5:$imm))],
    [WriteALU]
  >;
  let Constraints = "$r2 = $r2_" in
  def SHRr : FormatI<0b000101,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "shr $r1, $r2",
    [(set i32:$r2, (srl GenRegs:$r2_, GenRegs:$r1))],
    [WriteALU]
  >;

  let Constraints = "$r2 = $r1" in
  def SARi : FormatII<0b010111,
    (outs GenRegs:$r2), (ins GenRegs:$r1, uimm5Op:$imm),
    "sar $imm, $r2",
    [(set i32:$r2, (sra GenRegs:$r1, uimm5:$imm))],
    [WriteALU]
  >;
  let Constraints = "$r2 = $r2_" in
  def SARr : FormatI<0b000111,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "sar $r1, $r2",
    [(set i32:$r2, (sra GenRegs:$r2_, GenRegs:$r1))],
    [WriteALU]
  >;
}

// Maths

let Defs = [SR5], V810_ZFlag = 1, V810_SFlag = 1, V810_CYFlag = 1, V810_OVFlag = 1 in {
  let Constraints = "$r2 = $r1" in
  def ADDri : FormatII<0b010001,
    (outs GenRegs:$r2), (ins GenRegs:$r1, simm5Op:$imm),
    "add $imm, $r2",
    [(set i32:$r2, (add GenRegs:$r1, simm5:$imm))],
    [WriteALU]
  >;
  def ADDI : FormatV<0b101001,
    (outs GenRegs:$r2), (ins GenRegs:$r1, simm16Op:$imm),
    "addi $imm, $r1, $r2",
    [(set i32:$r2, (add GenRegs:$r1, simm16:$imm))],
    [WriteALU]
  >;
  let Constraints = "$r2 = $r2_" in
  def ADDrr : FormatI<0b000001,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "add $r1, $r2",
    [(set i32:$r2, (add GenRegs:$r2_, GenRegs:$r1))],
    [WriteALU]
  >;

  let Constraints = "$r2 = $r2_" in
  def SUB : FormatI<0b000010,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "sub $r1, $r2",
    [(set i32:$r2, (sub GenRegs:$r2_, GenRegs:$r1))],
    [WriteALU]
  >;

  let isCompare = 1 in {
    def CMPri : FormatII<0b010011,
      (outs), (ins GenRegs:$r2, simm5Op:$imm),
      "cmp $imm, $r2",
      [(V810CMP GenRegs:$r2, simm5:$imm)],
      [WriteALU]
    >;
    def CMPrr : FormatI<0b000011,
      (outs), (ins GenRegs:$r2, GenRegs:$r1),
      "cmp $r1, $r2",
      [(V810CMP GenRegs:$r2, GenRegs:$r1)],
      [WriteALU]
    >;
  }

  // Support loading addresses into a register
  let isCodeGenOnly = 1 in
  def ADDImem : FormatV<0b101001,
    (outs GenRegs:$r2), (ins (MEMri $r1, $imm):$addr),
    "addi $addr, $r2",
    [(set iPTR:$r2, ADDRri:$addr)],
    [WriteALU]
  >;
}

// These hi+lo instructions have return values handled in code
let Defs = [R30, SR5], Constraints = "$r2 = $r2_", V810_ZFlag = 1, V810_SFlag = 1 in {
  def MUL : FormatI<0b001000,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "mul $r1, $r2",
    [(set i32:$r2, (V810Smullohi GenRegs:$r2_, GenRegs:$r1))],
    [WriteIMul]
  >;
  def MULU: FormatI<0b001010,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "mulu $r1, $r2",
    [(set i32:$r2, (V810Umullohi GenRegs:$r2_, GenRegs:$r1))],
    [WriteIMul]
  >;
}
def : GINodeEquiv<MUL, V810Smullohi>;
def : GINodeEquiv<MULU, V810Umullohi>;

let Defs = [R30, SR5], Constraints = "$r2 = $r2_", V810_ZFlag = 1, V810_SFlag = 1 in {
  def DIV : FormatI<0b001001,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "div $r1, $r2",
    [(set i32:$r2, (V810Sdivrem GenRegs:$r2_, GenRegs:$r1))],
    [WriteIDiv]
  >;
  def DIVU : FormatI<0b001011,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "divu $r1, $r2",
    [(set i32:$r2, (V810Udivrem GenRegs:$r2_, GenRegs:$r1))],
    [WriteIDiv]
  >;
}
def : GINodeEquiv<DIV, V810Sdivrem>;
def : GINodeEquiv<DIVU, V810Udivrem>;

// Float ops
let Defs = [SR5], V810_ZFlag = 1, V810_SFlag = 1 in {
  let Constraints = "$r2 = $r2_" in
  def ADDF_S : FormatVII<0b000100,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "addf.s $r1, $r2",
    [(set f32:$r2, (fadd GenRegs:$r2_, GenRegs:$r1))],
    [WriteFPU]
  >;

  let Constraints = "$r2 = $r2_" in
  def SUBF_S : FormatVII<0b000101,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "subf.s $r1, $r2",
    [(set f32:$r2, (fsub GenRegs:$r2_, GenRegs:$r1))],
    [WriteFPU]
  >;

  let isCompare = 1 in
  def CMPF_S : FormatVII<0b000000,
    (outs), (ins GenRegs:$r2, GenRegs:$r1),
    "cmpf.s $r1, $r2",
    [(V810FCMP GenRegs:$r2, GenRegs:$r1)],
    [WriteFPU]
  >;

  let Constraints = "$r2 = $r2_" in
  def MULF_S : FormatVII<0b000110,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "mulf.s $r1, $r2",
    [(set f32:$r2, (fmul GenRegs:$r2_, GenRegs:$r1))],
    [WriteFPU]
  >;

  let Constraints = "$r2 = $r2_" in
  def DIVF_S : FormatVII<0b000111,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "divf.s $r1, $r2",
    [(set f32:$r2, (fdiv GenRegs:$r2_, GenRegs:$r1))],
    [WriteFPU]
  >;

  def CVT_WS : FormatVII<0b000010,
    (outs GenRegs:$r2), (ins GenRegs:$r1),
    "cvt.ws $r1, $r2",
    [(set f32:$r2, (sint_to_fp GenRegs:$r1))],
    [WriteFPU]
  >;

  def CVT_SW : FormatVII<0b000011,
    (outs GenRegs:$r2), (ins GenRegs:$r1),
    "cvt.sw $r1, $r2",
    [(set i32:$r2, (fp_to_sint (fround GenRegs:$r1)))],
    [WriteFPU]
  >;

  def TRNC_SW : FormatVII<0b001011,
    (outs GenRegs:$r2), (ins GenRegs:$r1),
    "trnc.sw $r1, $r2",
    [(set i32:$r2, (fp_to_sint GenRegs:$r1))],
    [WriteFPU]
  >;
}
def : GINodeEquiv<CMPF_S, V810FCMP>;

let Uses = [SR5], usesCustomInserter = 1 in {
  def SELECT_CC_Int : Pseudo<
    (outs GenRegs:$dst), (ins GenRegs:$t, GenRegs:$f, condcode:$cond),
    "; SELECT_CC_Int PSEUDO!",
    [(set i32:$dst, (V810Selectcc i32:$t, i32:$f, imm:$cond))]
  >;
  def SELECT_CC_Float : Pseudo<
    (outs GenRegs:$dst), (ins GenRegs:$t, GenRegs:$f, condcode:$cond),
    "; SELECT_CC_Float PSEUDO!",
    [(set f32:$dst, (V810Selectcc f32:$t, f32:$f, imm:$cond))]
  >;
}
def : GINodeEquiv<SELECT_CC_Int, V810Selectcc>;
def : GINodeEquiv<SELECT_CC_Float, V810Selectcc>;

// conditionals
// V810ISelLowering generates the patterns which these instrs match, when lowering
// LLVM conditional forms. That's because CMP is an instruction which writes SR5
// flags that BCond/SETF read, and idk how to express that dependency in tablegen.
let Uses = [SR5] in {
  let isTerminator = 1, isBranch = 1 in
  def Bcond : FormatIII<0b100,
    (outs), (ins condcode:$cond, bcondtarget:$disp),
    "b$cond $disp",
    [(V810Bcond imm:$cond, bb:$disp)],
    [WriteBranch]
  >;

  def SETF : FormatII<0b010010,
    (outs GenRegs:$r2), (ins condcode:$imm),
    "setf $imm, $r2",
    [(set i32:$r2, (V810Setf imm:$imm))],
    [WriteALU]
  >;
}
def : GINodeEquiv<Bcond, V810Bcond>;
def : GINodeEquiv<SETF, V810Setf>;

let isTerminator = 1, isBarrier = 1, isBranch = 1 in {
  let isIndirectBranch = 1, r2 = 0 in
  def JMP : FormatI<0b000110,
    (outs), (ins GenRegs:$r1),
    "jmp $r1",
    [(brind GenRegs:$r1)],
    [WriteBranch]
  >;

  def JR : FormatIV<0b101010,
    (outs), (ins brtarget:$disp),
    "jr $disp",
    [(br bb:$disp)],
    [WriteBranch]
  >;

  let Defs=[R31] in
  def JAL : FormatIV<0b101011,
    (outs), (ins calltarget:$disp),
    "jal $disp",
    [],
    [WriteBranch]
  >;
}
def : GINodeEquiv<JMP, brind>;

// Function call tools

let isCodeGenOnly = 1 in {
  let isCall = 1, Defs = [R31] in
  def CALL : FormatIV<0b101011,
    (outs), (ins calltarget:$disp, variable_ops),
    "jal $disp",
    [],
    [WriteBranch]
  >;

  let isCall = 1, isReturn = 1, isTerminator = 1, isBarrier = 1, Uses = [R31] in
  def TAIL_CALL : FormatIV<0b101010,
    (outs), (ins calltarget:$disp, variable_ops),
    "jr $disp",
    [],
    [WriteBranch]
  >;

  // We can't jal from a register, so emit
  // code to set the LP before jumping
  let isCall = 1, Size = 8, Defs = [R31] in
  def CALL_INDIRECT: Pseudo<
    (outs), (ins GenRegs:$r1, variable_ops),
    "!CALL_INDIRECT",
    []
  >;

  let isCall = 1, isReturn = 1, isTerminator = 1, isBarrier = 1, Uses = [R31], r2 = 0 in
  def TAIL_CALL_INDIRECT: FormatI<0b000110,
    (outs), (ins GenRegs:$r1, variable_ops),
    "jmp [$r1]",
    [],
    [WriteBranch]
  >;

  let isReturn = 1, isTerminator = 1, isBarrier = 1, r2 = 0, r1 = 31 in
  def RET : FormatI<0b000110,
    (outs), (ins),
    "jmp [r31]",
    [(retglue)],
    [WriteBranch]
  >;

  let Defs = [R3], Uses = [R3], hasSideEffects = 1 in {
    def ADJCALLSTACKDOWN : Pseudo<
      (outs), (ins i32imm:$amt1, i32imm:$amt2),
      "!ADJCALLSTACKDOWN $amt1, $amt2",
      [(callseq_start timm:$amt1, timm:$amt2)]
    >;
    def ADJCALLSTACKUP : Pseudo<
      (outs), (ins i32imm:$amt1, i32imm:$amt2),
      "!ADJCALLSTACKUP $amt1, $amt2",
      [(callseq_end timm:$amt1, timm:$amt2)]
    >;
  }
}
def : GINodeEquiv<RET, retglue>;
def : GINodeEquiv<ADJCALLSTACKDOWN, callseq_start>;
def : GINodeEquiv<ADJCALLSTACKUP, callseq_end>;

def : Pat<(call tglobaladdr:$dst),
          (CALL tglobaladdr:$dst)>;
def : Pat<(call texternalsym:$dst),
          (CALL texternalsym:$dst)>;
def : Pat<(tailcall tglobaladdr:$dst),
          (TAIL_CALL tglobaladdr:$dst)>;
def : Pat<(tailcall texternalsym:$dst),
          (TAIL_CALL texternalsym:$dst)>;
def : Pat<(call GenRegs:$dst),
          (CALL_INDIRECT GenRegs:$dst)>;
def : Pat<(tailcall GenRegs:$dst),
          (TAIL_CALL_INDIRECT GenRegs:$dst)>;
def : GINodeEquiv<CALL, call>;
def : GINodeEquiv<CALL_INDIRECT, call>;
def : GINodeEquiv<TAIL_CALL, tailcall>;
def : GINodeEquiv<TAIL_CALL_INDIRECT, tailcall>; 

// System stuff
def LDSR : FormatII<0b011100,
  (outs SysRegs:$imm), (ins GenRegs:$r2),
  "ldsr $r2, $imm",
  [],
  [WriteALU]
>;
def STSR : FormatII<0b011101,
  (outs GenRegs:$r2), (ins SysRegs:$imm),
  "stsr $imm, $r2",
  [],
  [WriteALU]
>;

let r2 = 0, imm = 0 in {
  def HALT : FormatII<0b011010,
    (outs), (ins),
    "halt",
    [],
    [WriteBranch]
  >;
  let isReturn = 1, isTerminator = 1, isBarrier = 1 in 
  def RETI : FormatII<0b011001,
    (outs), (ins),
    "reti",
    [(retiglue)],
    [WriteBranch]
  >;
}
def : GINodeEquiv<RETI, retiglue>;

let isBarrier = 1, isTerminator = 1, r2 = 0 in
def TRAP : FormatII<0b011000,
  (outs), (ins uimm5Op:$imm),
  "trap $imm",
  [],
  [WriteBranch]
>;
def : Pat<(trap), (TRAP 0)>;
def : Pat<(debugtrap), (TRAP 1)>;

// bitstrings
class BitstringInst<bits<5> subop, string asmstr>
  : FormatII<0b011111, (outs), (ins), asmstr, [], [WriteLoad, WriteStore]> {
  let r2 = 0;
  let imm = subop;
  let Uses = [R26, R27, R28, R29, R30];
  let Defs = [R26, R27, R28, R29, R30];
  let Itinerary = IIC_Store;
  let mayLoad = 1;
  let mayStore = 1;
}

def ANDBSU  : BitstringInst<0b01001, "andbsu">;
def ANDNBSU : BitstringInst<0b01101, "andnbsu">;
def MOVBSU  : BitstringInst<0b01011, "movbsu">;
def NOTBSU  : BitstringInst<0b01111, "notbsu">;
def ORBSU   : BitstringInst<0b01000, "orbsu">;
def ORNBSU  : BitstringInst<0b01100, "ornbsu">;
def XORBSU  : BitstringInst<0b01010, "xorbsu">;
def XORNBSU : BitstringInst<0b01110, "xornbsu">;

// and bitstring searches
class BitstringSearchInst<bits<5> subop, string asmstr>
  : FormatII<0b011111, (outs), (ins), asmstr, [], [WriteLoad]> {
  let r2 = 0;
  let imm = subop;
  let Uses = [R27, R28, R29, R30];
  let Defs = [R27, R28, R29, R30, SR5];
  let mayLoad = 1;
  let Itinerary = IIC_Load;
}

def SCH0BSD : BitstringSearchInst<0b00001, "sch0bsd">;
def SCH0BSU : BitstringSearchInst<0b00000, "sch0bsu">;
def SCH1BSD : BitstringSearchInst<0b00011, "sch1bsd">;
def SCH1BSU : BitstringSearchInst<0b00010, "sch1bsu">;

// the one atomic operator
// $rcmp is always the same as r2, and rswap is always R30.
// The latter is guaranteed by using custom lowering for ATOMIC_CMP_SWAP.
let mayLoad = 1, mayStore = 1, Uses = [R30], Defs = [SR5],
    Constraints = "$r2 = $rcmp", DisableEncoding = "$rcmp,$rswap",
    Itinerary = IIC_Store in
def CAXI : FormatVI<0b111010,
  (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr, GenRegs:$rcmp, GenRegs:$rswap),
  "caxi $addr, $r2",
  [(set i32:$r2, (V810Caxi ADDRri:$addr, GenRegs:$rcmp, GenRegs:$rswap))],
  [WriteLoad, WriteStore]
>;
def : GINodeEquiv<CAXI, V810Caxi>;

// VB-specific instructions
let Predicates = [IsNintendo] in {
  let Defs = [SR5], r2 = 0, imm = 0 in {
    def CLI : FormatII<0b010110,
      (outs), (ins),
      "cli",
      [],
      [WriteI]
    >;
    def SEI : FormatII<0b011110,
      (outs), (ins),
      "sei",
      [],
      [WriteI]
    >;
  }

  let Constraints = "$r2 = $r2_" in
  def MPYHW : FormatVII<0b001100,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "mpyhw $r1, $r2",
    [],
    [WriteALU]
  >;

  def REV : FormatVII<0b001010,
    (outs GenRegs:$r2), (ins GenRegs:$r1),
    "rev $r1, $r2",
    [(set i32:$r2, (bitreverse GenRegs:$r1))],
    [WriteALU]
  >;

  let Constraints = "$r2 = $r2_", r1 = 0 in
  def XB : FormatVII<0b001000,
    (outs GenRegs:$r2), (ins GenRegs:$r2_),
    "xb $r2",
    [],
    [WriteALU]
  >;

  let Constraints = "$r2 = $r2_", r1 = 0 in
  def XH : FormatVII<0b001001,
    (outs GenRegs:$r2), (ins GenRegs:$r2_),
    "xh $r2",
    [],
    [WriteALU]
  >;

  def : Pat<(i32 (bswap GenRegs:$r2)),
            (XB (XH (XB GenRegs:$r2)))>;

}

// Non-instructions

// Zero immediate.
def : Pat<(i32 0), (COPY (i32 R0))>;
// Smol immediate (up to 5 bits).
def : Pat<(i32 simm5:$val),
          (MOVi imm:$val)>;
// Med immediate (up to 16 bits).
def : Pat<(i32 simm16:$val),
          (MOVEA (i32 R0), imm:$val)>;
// Med immediate (unsigned, up to 16 bits).
def : Pat<(i32 uimm16:$val),
          (ORI (i32 R0), imm:$val)>;
// Immediate that only uses the top 16 bits
def : Pat<(i32 MOVHIimm:$val),
          (MOVHI (i32 R0), (HI16 imm:$val))>;
// Lorg immediate (up to 32 bits).
def : Pat<(i32 imm32:$val),
          (MOVEA (MOVHI (i32 R0), (IMM32_HI16 imm:$val)), (IMM32_LO16 imm:$val))>;

// loads that aren't explicitly sign-extended or zero-extended will sign-extend
def : Pat<(i32 (extloadi16 ADDRri:$addr)), (LD_H ADDRri:$addr)>;
def : Pat<(i32 (extloadi8 ADDRri:$addr)), (LD_B ADDRri:$addr)>;

// bools (i1) can be loaded as bytes
def : Pat<(i32 (extloadi1 ADDRri:$addr)), (IN_B ADDRri:$addr)>;
def : Pat<(i32 (zextloadi1 ADDRri:$addr)), (IN_B ADDRri:$addr)>;
def : Pat<(i32 (sextloadi1 ADDRri:$addr)),
          (SHLi (SARi (IN_B ADDRri:$addr), 31), 31)>;

// i8, i16, and i32 loads ARE atomic loads
def : Pat<(i32 (atomic_load_32 ADDRri:$addr)), (LD_W ADDRri:$addr)>;
def : Pat<(i32 (atomic_load_16 ADDRri:$addr)), (LD_H ADDRri:$addr)>;
def : Pat<(i32 (atomic_load_8 ADDRri:$addr)), (LD_B ADDRri:$addr)>;

// i8, i16, and i32 stores ARE atomic stores
def : Pat<(atomic_store_32 i32:$r1, ADDRri:$addr), (ST_W ADDRri:$addr, GenRegs:$r1)>;
def : Pat<(atomic_store_16 i32:$r1, ADDRri:$addr), (ST_H ADDRri:$addr, GenRegs:$r1)>;
def : Pat<(atomic_store_8 i32:$r1, ADDRri:$addr), (ST_B ADDRri:$addr, GenRegs:$r1)>;

// floats can be stored just like ints, nothing special
def : Pat<(store f32:$r2, ADDRri:$addr), (ST_W ADDRri:$addr, GenRegs:$r2)>;

// floats can be loaded just like ints, nothing special
def : Pat<(f32 (load ADDRri:$addr)), (LD_W ADDRri:$addr)>;

// floats can be bitcast to ints for free and vice-versa
def : Pat<(i32 (bitconvert (f32 GenRegs:$val))),
          (i32 GenRegs:$val)>;
def : Pat<(f32 (bitconvert (i32 GenRegs:$val))),
          (f32 GenRegs:$val)>;

// It's not safe to convert u32 to f32, because a large enough value would become a negative float.
// But if it was zero-extended from a smaller type, that can't happen, so it is safe after all.
def : Pat<(uint_to_fp (assertzext GenRegs:$r1)),
          (CVT_WS GenRegs:$r1)>;

// Make addresses work
def : GINodeEquiv<MOVEA, V810Lo>;
def : GINodeEquiv<MOVHI, V810Hi>;
def : Pat<(V810Lo (V810Hi tglobaladdr:$inhi), tglobaladdr:$inlo),
          (MOVEA (MOVHI (i32 R0), tglobaladdr:$inhi), tglobaladdr:$inlo)>;
def : Pat<(V810Lo (V810Hi tglobaltlsaddr:$inhi), tglobaltlsaddr:$inlo),
          (MOVEA (MOVHI (i32 R0), tglobaltlsaddr:$inhi), tglobaltlsaddr:$inlo)>;
def : Pat<(V810Lo (V810Hi tblockaddress:$inhi), tblockaddress:$inlo),
          (MOVEA (MOVHI (i32 R0), tblockaddress:$inhi), tblockaddress:$inlo)>;
def : Pat<(V810Lo (V810Hi texternalsym:$inhi), texternalsym:$inlo),
          (MOVEA (MOVHI (i32 R0), texternalsym:$inhi), texternalsym:$inlo)>;
def : Pat<(V810Lo (V810Hi tconstpool:$inhi), tconstpool:$inlo),
          (MOVEA (MOVHI (i32 R0), tconstpool:$inhi), tconstpool:$inlo)>;
// Including register-relative addresses
def : Pat<(V810RegRelative tglobaladdr:$inrel, GenRegs:$reg),
          (MOVEA (i32 $reg), tglobaladdr:$inrel)>;
def : Pat<(V810RegRelative tglobaltlsaddr:$inrel, GenRegs:$reg),
          (MOVEA (i32 $reg), tglobaltlsaddr:$inrel)>;
def : Pat<(V810RegRelative texternalsym:$inrel, GenRegs:$reg),
          (MOVEA (i32 $reg), texternalsym:$inrel)>;
def : Pat<(V810RegRelative tconstpool:$inrel, GenRegs:$reg),
          (MOVEA (i32 $reg), tconstpool:$inrel)>;

include "V810InstrAliases.td"