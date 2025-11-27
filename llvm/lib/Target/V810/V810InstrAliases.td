multiclass int_cond_alias<string bcond, string setf, int condVal> {
  // <bcond> $imm
  def : InstAlias<!strconcat(bcond, " $disp"),
                  (Bcond condVal, bcondtarget:$disp)>;
  // <setf> $condVal $imm
  def : InstAlias<!strconcat("setf", setf, " $r2"),
                  (SETF GenRegs:$r2, condVal),
                  0>;
}

defm : int_cond_alias<"bv",  "v",  0b0000>;
defm : int_cond_alias<"bc",  "c",  0b0001>;
defm : int_cond_alias<"bl",  "l",  0b0001>;
defm : int_cond_alias<"be",  "e",  0b0010>;
defm : int_cond_alias<"bz",  "z",  0b0010>;
defm : int_cond_alias<"bnh", "nh", 0b0011>;
defm : int_cond_alias<"bn",  "n",  0b0100>;
defm : int_cond_alias<"br",  "t",  0b0101>;
defm : int_cond_alias<"blt", "lt", 0b0110>;
defm : int_cond_alias<"ble", "le", 0b0111>;
defm : int_cond_alias<"bnv", "nv", 0b1000>;
defm : int_cond_alias<"bnc", "nc", 0b1001>;
defm : int_cond_alias<"bnl", "nl", 0b1001>;
defm : int_cond_alias<"bne", "ne", 0b1010>;
defm : int_cond_alias<"bnz", "nz", 0b1010>;
defm : int_cond_alias<"bh",  "h",  0b1011>;
defm : int_cond_alias<"bp",  "p",  0b1100>;
// defm : int_cond_alias<"nop", "f",  0b1101>;
defm : int_cond_alias<"bge", "ge", 0b1110>;
defm : int_cond_alias<"bgt", "gt", 0b1111>;

// nop is a special case, it does not take an offset
def : InstAlias<"nop",
                (Bcond 0b1101, 0)>;
  // <setf> $condVal $imm
def : InstAlias<"setff $r2",
                (SETF GenRegs:$r2, 0b1101),
                0>;