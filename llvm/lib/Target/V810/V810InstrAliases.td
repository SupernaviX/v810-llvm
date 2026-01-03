
multiclass int_cond_alias<string abcond, string bcond, string setf, int condVal> {
  // <bcond> $imm
  def : InstAlias<!strconcat(bcond, " $disp"),
                  (Bcond condVal, bcondtarget:$disp)>;
  let Predicates = [IsV830] in {
  // <abcond> $imm
  def : InstAlias<!strconcat(abcond, " $disp"),
                  (ABcond condVal, bcondtarget:$disp)>;
  }
  // <setf> $condVal $imm
  def : InstAlias<!strconcat("setf", setf, " $r2"),
                  (SETF GenRegs:$r2, condVal),
                  0>;
}

defm : int_cond_alias<"abv",  "bv",  "v",  0b0000>;
defm : int_cond_alias<"abc",  "bc",  "c",  0b0001>;
defm : int_cond_alias<"abl",  "bl",  "l",  0b0001>;
defm : int_cond_alias<"abe",  "be",  "e",  0b0010>;
defm : int_cond_alias<"abz",  "bz",  "z",  0b0010>;
defm : int_cond_alias<"abnh", "bnh", "nh", 0b0011>;
defm : int_cond_alias<"abn",  "bn",  "n",  0b0100>;
defm : int_cond_alias<"abr",  "br",  "t",  0b0101>;
defm : int_cond_alias<"ablt", "blt", "lt", 0b0110>;
defm : int_cond_alias<"able", "ble", "le", 0b0111>;
defm : int_cond_alias<"abnv", "bnv", "nv", 0b1000>;
defm : int_cond_alias<"abnc", "bnc", "nc", 0b1001>;
defm : int_cond_alias<"abnl", "bnl", "nl", 0b1001>;
defm : int_cond_alias<"abne", "bne", "ne", 0b1010>;
defm : int_cond_alias<"abnz", "bnz", "nz", 0b1010>;
defm : int_cond_alias<"abh",  "bh",  "h",  0b1011>;
defm : int_cond_alias<"abp",  "bp",  "p",  0b1100>;
// defm : int_cond_alias<"nop", "f",  0b1101>;
defm : int_cond_alias<"abge", "bge", "ge", 0b1110>;
defm : int_cond_alias<"abgt", "bgt", "gt", 0b1111>;

// nop is a special case, it does not take an offset
def : InstAlias<"nop",
                (Bcond 0b1101, 0)>;
  // <setf> $condVal $imm
def : InstAlias<"setff $r2",
                (SETF GenRegs:$r2, 0b1101),
                0>;
// Interrupt control aliases
let Predicates = [IsV830] in {
  // CLI is EI
  def : InstAlias<"ei",
                  (CLI)>;
  // SEI is DI
  def : InstAlias<"di",
                  (SEI)>;
  }