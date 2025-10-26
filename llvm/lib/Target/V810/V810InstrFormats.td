class InstV810<dag outs, dag ins, string asmstr, list<dag> pattern> : Instruction {
  let Namespace = "V810";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;

  let DecoderNamespace = "V810";

  bit V810_ZFlag = 0;
  bit V810_SFlag = 0;
  bit V810_OVFlag = 0;
  bit V810_CYFlag = 0;

  let TSFlags{0} = V810_ZFlag;
  let TSFlags{1} = V810_SFlag;
  let TSFlags{2} = V810_OVFlag;
  let TSFlags{3} = V810_CYFlag;
}

// register-to-register
class FormatI<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, list<SchedReadWrite> schedrw>
    : InstV810<outs, ins, asmstr, pattern>,
      Sched<schedrw> {
  field bits<16> Inst;
  let Size = 2;

  // Prefer 16-bit instructions by increasing the match priority.
  let AddedComplexity = 1;

  bits<5> r2;
  bits<5> r1;

  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = r1;
}

// immediate-to-register
class FormatII<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, list<SchedReadWrite> schedrw>
    : InstV810<outs, ins, asmstr, pattern>,
      Sched<schedrw> {
  field bits<16> Inst;
  let Size = 2;

  // Prefer 16-bit instructions by increasing the match priority.
  let AddedComplexity = 1;

  bits<5> r2;
  bits<5> imm;

  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = imm;
}

// conditional branch
class FormatIII<bits<3> op, dag outs, dag ins, string asmstr, list<dag> pattern, list<SchedReadWrite> schedrw>
    : InstV810<outs, ins, asmstr, pattern>,
      Sched<schedrw> {
  field bits<16> Inst;
  let Size = 2;

  // Prefer 16-bit instructions by increasing the match priority.
  let AddedComplexity = 1;

  bits<4> cond;
  bits<9> disp;

  let Inst{15-13} = op;
  let Inst{12-9} = cond;
  let Inst{8-0} = disp;
}

// middle-distance jump
class FormatIV<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, list<SchedReadWrite> schedrw>
    : InstV810<outs, ins, asmstr, pattern>,
      Sched<schedrw> {
  field bits<32> Inst;
  let Size = 4;

  bits<26> disp;

  let Inst{31-16} = disp{15-0};
  let Inst{15-10} = op;
  let Inst{9-0} = disp{25-16};
}

// three-operand
class FormatV<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, list<SchedReadWrite> schedrw>
    : InstV810<outs, ins, asmstr, pattern>,
      Sched<schedrw> {
  field bits<32> Inst;
  let Size = 4;

  bits<5> r2;
  bits<5> r1;
  bits<16> imm;

  let Inst{31-16} = imm;
  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = r1;
}

// load/store
class FormatVI<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, list<SchedReadWrite> schedrw>
    : InstV810<outs, ins, asmstr, pattern>,
      Sched<schedrw> {
  field bits<32> Inst;
  let Size = 4;

  bits<0> rswap; // needed for CAXI to consume r30
  bits<5> r2;
  bits<5> r1;
  bits<16> disp;

  let Inst{31-16} = disp;
  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = r1;
}

class FormatVII<bits<6> subop, dag outs, dag ins, string asmstr, list<dag> pattern, list<SchedReadWrite> schedrw>
    : InstV810<outs, ins, asmstr, pattern>,
      Sched<schedrw> {
  field bits<32> Inst;
  let Size = 4;

  bits<6> op = 0b111110;
  bits<5> r2;
  bits<5> r1;

  let Inst{31-26} = subop;
  let Inst{25-16} = 0;
  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = r1;
}
