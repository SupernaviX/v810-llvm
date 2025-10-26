class V810Reg<bits<16> Enc, string n, list<string> altNames = []> : Register<n, altNames> {
  let HWEncoding = Enc;
  let Namespace = "V810";
}

def R0  : V810Reg< 0, "r0">, DwarfRegNum<[0]> {
  let isConstant = true;
}
def R1  : V810Reg< 1, "r1">, DwarfRegNum<[1]>;
def R2  : V810Reg< 2, "r2", ["hp", "fp"]>, DwarfRegNum<[2]>;
def R3  : V810Reg< 3, "r3", ["sp"]>, DwarfRegNum<[3]>;
def R4  : V810Reg< 4, "r4", ["gp"]>, DwarfRegNum<[4]>;
def R5  : V810Reg< 5, "r5", ["tp"]>, DwarfRegNum<[5]>;
def R6  : V810Reg< 6, "r6">, DwarfRegNum<[6]>;
def R7  : V810Reg< 7, "r7">, DwarfRegNum<[7]>;
def R8  : V810Reg< 8, "r8">, DwarfRegNum<[8]>;
def R9  : V810Reg< 9, "r9">, DwarfRegNum<[9]>;
def R10 : V810Reg<10, "r10">, DwarfRegNum<[10]>;
def R11 : V810Reg<11, "r11">, DwarfRegNum<[11]>;
def R12 : V810Reg<12, "r12">, DwarfRegNum<[12]>;
def R13 : V810Reg<13, "r13">, DwarfRegNum<[13]>;
def R14 : V810Reg<14, "r14">, DwarfRegNum<[14]>;
def R15 : V810Reg<15, "r15">, DwarfRegNum<[15]>;
def R16 : V810Reg<16, "r16">, DwarfRegNum<[16]>;
def R17 : V810Reg<17, "r17">, DwarfRegNum<[17]>;
def R18 : V810Reg<18, "r18">, DwarfRegNum<[18]>;
def R19 : V810Reg<19, "r19">, DwarfRegNum<[19]>;
def R20 : V810Reg<20, "r20">, DwarfRegNum<[20]>;
def R21 : V810Reg<21, "r21">, DwarfRegNum<[21]>;
def R22 : V810Reg<22, "r22">, DwarfRegNum<[22]>;
def R23 : V810Reg<23, "r23">, DwarfRegNum<[23]>;
def R24 : V810Reg<24, "r24">, DwarfRegNum<[24]>;
def R25 : V810Reg<25, "r25">, DwarfRegNum<[25]>;
def R26 : V810Reg<26, "r26">, DwarfRegNum<[26]>;
def R27 : V810Reg<27, "r27">, DwarfRegNum<[27]>;
def R28 : V810Reg<28, "r28">, DwarfRegNum<[28]>;
def R29 : V810Reg<29, "r29">, DwarfRegNum<[29]>;
def R30 : V810Reg<30, "r30">, DwarfRegNum<[30]>;
def R31 : V810Reg<31, "r31", ["lp"]>, DwarfRegNum<[31]>;

def GenRegs : RegisterClass<"V810", [i32, f32], 32, (sequence "R%u", 0, 31)>;
def Clobber : RegisterClass<"V810", [i32, f32], 32, (add R30)>;

// Program status word is the V810's status register
def SR0  : V810Reg< 0, "sr0",  ["eipc"]>, DwarfRegNum<[32]>;
def SR1  : V810Reg< 1, "sr1",  ["eipsw"]>, DwarfRegNum<[33]>;
def SR2  : V810Reg< 2, "sr2",  ["fepc"]>, DwarfRegNum<[34]>;
def SR3  : V810Reg< 3, "sr3",  ["fepsw"]>, DwarfRegNum<[35]>;
def SR4  : V810Reg< 4, "sr4",  ["ecr"]>, DwarfRegNum<[36]>;
def SR5  : V810Reg< 5, "sr5",  ["cc", "psw"]>, DwarfRegNum<[37]>;
def SR6  : V810Reg< 6, "sr6",  ["pir"]>, DwarfRegNum<[38]>;
def SR7  : V810Reg< 7, "sr7",  ["tkcw"]>, DwarfRegNum<[39]>;
def SR24 : V810Reg<24, "sr24", ["chcw"]>, DwarfRegNum<[40]>;
def SR25 : V810Reg<25, "sr25", ["adtre"]>, DwarfRegNum<[41]>;
def SR29 : V810Reg<29, "sr29">, DwarfRegNum<[42]>;
def SR30 : V810Reg<30, "sr30">, DwarfRegNum<[43]>;
def SR31 : V810Reg<31, "sr31">, DwarfRegNum<[44]>;

def SysRegs : RegisterClass<"V810", [i32], 32,
  (add (sequence "SR%u", 0, 7),
       (sequence "SR%u", 24, 25),
       (sequence "SR%u", 29, 31))> {
  let CopyCost = -1;
  let isAllocatable = false;
}
