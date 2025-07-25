#include "V810.h"
#include "MCTargetDesc/V810MCAsmInfo.h"
#include "MCTargetDesc/V810MCExpr.h"
#include "MCTargetDesc/V810MCTargetDesc.h"
#include "TargetInfo/V810TargetInfo.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/SMLoc.h"

using namespace llvm;

namespace {

class V810Operand;

class V810AsmParser : public MCTargetAsmParser {
  MCAsmParser &Parser;
#define GET_ASSEMBLER_HEADER
#include "V810GenAsmMatcher.inc"

  // public interface of the MCTargetAsmParser.
  bool matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;
  bool parseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                     SMLoc &EndLoc) override;
  ParseStatus tryParseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;
  bool parseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;
  bool ParseDirective(AsmToken DirectiveID) override;
  bool parseSSectionDirective(StringRef Section, unsigned Type);

  ParseStatus parseMEMOperand(OperandVector &Operands);
  ParseStatus parseBranchTargetOperand(OperandVector &Operands);
  ParseStatus parseBcondTargetOperand(OperandVector &Operands);
  ParseStatus parseJumpTargetOperand(OperandVector &Operands, V810MCExpr::VariantKind Kind);
  ParseStatus parseCondOperand(OperandVector &Operands);

  ParseStatus MatchOperandParserCustomImpl(OperandVector &Operands, StringRef Mnemonic);
  ParseStatus parseOperand(OperandVector &Operands, StringRef Name);
  ParseStatus parseV810AsmOperand(std::unique_ptr<V810Operand> &Operand);

  bool parseImm16Expression(const MCExpr *&Res, SMLoc &EndLoc);

public:
  V810AsmParser(const MCSubtargetInfo &sti, MCAsmParser &parser,
                const MCInstrInfo &MII,
                const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, sti, MII), Parser(parser) {

    // Initialize the set of available features.
    setAvailableFeatures(ComputeAvailableFeatures(getSTI().getFeatureBits()));

    parser.addAliasForDirective(".hword", ".2byte");
    parser.addAliasForDirective(".word", ".4byte");
  }
};

} // end anonymous namespace

namespace {

// V810Operand - Instances of this class represent a parsed V810 instruction.
class V810Operand : public MCParsedAsmOperand {
private:
  enum KindTy {
    k_Token,
    k_Register,
    k_Immediate,
    k_Memory,
  } Kind;

  SMLoc StartLoc, EndLoc;

  struct Token {
    const char *Data;
    unsigned Length;
  };

  struct RegOp {
    unsigned RegNum;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  struct MemOp {
    unsigned Base;
    const MCExpr *Off;
  };

  union {
    struct Token Tok;
    struct RegOp Reg;
    struct ImmOp Imm;
    struct MemOp Mem;
  };

public:
  V810Operand(KindTy K) : Kind(K) {}

  bool isToken() const override { return Kind == k_Token; }
  bool isReg() const override { return Kind == k_Register; }
  bool isImm() const override { return Kind == k_Immediate; }
  bool isMem() const override { return Kind == k_Memory; }
  bool isMEMri() const { return isMem(); }
  bool isBranchTarget() const { return isJumpTarget<26>(); }
  bool isBcondTarget() const { return isJumpTarget<9>(); }
  template <unsigned N> bool isJumpTarget() const {
    if (!isImm()) return false;
    if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Imm.Val))
      return isInt<N>(CE->getValue());
    return true;
  }
  bool isCond() const {
    if (!isImm()) return false;
    if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Imm.Val))
      return 0 <= CE->getValue() && CE->getValue() < 16;
    return false;
  }

  MCRegister getReg() const override {
    assert((Kind == k_Register) && "Invalid access!");
    return Reg.RegNum;
  }

  const MCExpr *getImm() const {
    assert((Kind == k_Immediate) && "Invalid access!");
    return Imm.Val;
  }

  unsigned getMemBase() const {
    assert((Kind == k_Memory) && "Invalid access!");
    return Mem.Base;
  }

  const MCExpr *getMemOff() const {
    assert((Kind == k_Memory) && "Invalid access!");
    return Mem.Off;
  }

  SMLoc getStartLoc() const override {
    return StartLoc;
  }

  SMLoc getEndLoc() const override {
    return EndLoc;
  }

  void print(raw_ostream &OS, const MCAsmInfo &MAI) const override {
    switch (Kind) {
    case k_Token:     OS << "Token: " << getToken() << "\n"; break;
    case k_Register:  OS << "Reg: #" << getReg() << "\n"; break;
    case k_Immediate: OS << "Imm: " << getImm() << "\n"; break;
    case k_Memory:    assert(getMemOff() != nullptr);
      OS << "Mem: " << getMemBase() << "+";
      MAI.printExpr(OS, *getMemOff());
      OS << "\n"; break;
    }
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    const MCExpr *Expr = getImm();
    addExpr(Inst, Expr);
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    // Add as immediate when possible.  Null MCExpr = 0.
    if (!Expr)
      Inst.addOperand(MCOperand::createImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  void addMEMriOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands!");

    Inst.addOperand(MCOperand::createReg(getMemBase()));

    const MCExpr *Expr = getMemOff();
    addExpr(Inst, Expr);
  }

  void addBranchTargetOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");

    addExpr(Inst, getImm());
  }

  void addBcondTargetOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");

    addExpr(Inst, getImm());
  }

  void addCondOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");

    addExpr(Inst, getImm());
  }

  static std::unique_ptr<V810Operand> CreateToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<V810Operand>(k_Token);
    Op->Tok.Data = Str.data();
    Op->Tok.Length = Str.size();
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static std::unique_ptr<V810Operand> CreateReg(unsigned RegNum, SMLoc S,
                                                SMLoc E) {
    auto Op = std::make_unique<V810Operand>(k_Register);
    Op->Reg.RegNum = RegNum;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<V810Operand> CreateImm(const MCExpr *Val, SMLoc S,
                                                SMLoc E) {
    auto Op = std::make_unique<V810Operand>(k_Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<V810Operand> CreateMEMri(unsigned Base, const MCExpr *Off,
                                                  SMLoc S, SMLoc E) {
    auto Op = std::make_unique<V810Operand>(k_Memory);
    Op->Mem.Base = Base;
    Op->Mem.Off = Off;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  StringRef getToken() const {
    assert(Kind == k_Token && "Invalid access!");
    return StringRef(Tok.Data, Tok.Length);
  }

};

}

/// Maps from the set of all register names to a register number.
/// \note Generated by TableGen.
static MCRegister MatchRegisterName(StringRef Name);
static MCRegister MatchRegisterAltName(StringRef Alias);

bool V810AsmParser::matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out,
                                            uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned MatchResult =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);
  switch (MatchResult) {
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  case Match_MissingFeature:
    return Error(IDLoc,
                 "instruction requires a CPU feature not currently enabled");
  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0ULL) {
      if (ErrorInfo >= Operands.size())
        return Error(IDLoc, "too few operands for instruction");
      ErrorLoc = ((V810Operand &)*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }

    return Error(ErrorLoc, "invalid operand for instruction");
  }
  case Match_MnemonicFail:
    return Error(IDLoc, "invalid instruction mnemonic");
  }
  llvm_unreachable("Implement any new match types added!");
}

bool V810AsmParser::parseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                   SMLoc &EndLoc) {
  if (!tryParseRegister(RegNo, StartLoc, EndLoc).isSuccess())
    return Error(StartLoc, "invalid register name");
  return false;
}

ParseStatus V810AsmParser::tryParseRegister(MCRegister &RegNo,
                                            SMLoc &StartLoc,
                                            SMLoc &EndLoc) {
  const AsmToken Tok = Parser.getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  std::string RegName = Tok.getString().lower();
  RegNo = MatchRegisterName(RegName);
  if (RegNo) {
    Parser.Lex(); // eat the identifier
    return ParseStatus::Success;
  }
  RegNo = MatchRegisterAltName(RegName);
  if (RegNo) {
    Parser.Lex();
    return ParseStatus::Success;
  }
  return ParseStatus::NoMatch;
}

bool V810AsmParser::parseInstruction(ParseInstructionInfo &Info,
                                     StringRef Name, SMLoc NameLoc,
                                     OperandVector &Operands) {
  Operands.push_back(V810Operand::CreateToken(Name, NameLoc));

  // If there are no operands, we're done
  if (getLexer().is(AsmToken::EndOfStatement))
    return false;

  // read first operand  
  if (!parseOperand(Operands, Name).isSuccess()) {
    SMLoc Loc = getLexer().getLoc();
    return Error(Loc, "unexpected token");
  }

  // read other operands
  while (getLexer().is(AsmToken::Comma)) {
    getLexer().Lex();
    if (!parseOperand(Operands, Name).isSuccess()) {
      SMLoc Loc = getLexer().getLoc();
      return Error(Loc, "unexpected token");
    }
  }

  return getLexer().isNot(AsmToken::EndOfStatement);
}

bool V810AsmParser::ParseDirective(AsmToken DirectiveID) {
  StringRef IDVal = DirectiveID.getString();
  if (IDVal == ".sbss") {
    parseSSectionDirective(IDVal, ELF::SHT_NOBITS);
    return false;
  }
  if (IDVal == ".sdata") {
    parseSSectionDirective(IDVal, ELF::SHT_PROGBITS);
    return false;
  }
  // Let the MC layer handle everything else
  return true;
}

bool V810AsmParser::parseSSectionDirective(StringRef Section, unsigned Type) {
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    Error(getLexer().getLoc(), "unexpected token, expected end of statement");
    return false;
  }

  MCSection *ELFSection = getContext().getELFSection(
      Section, Type, ELF::SHF_WRITE | ELF::SHF_ALLOC | ELF::SHF_V810_GPREL);
  getParser().getStreamer().switchSection(ELFSection);

  getParser().Lex(); // Eat EndOfStatement token.
  return false;
}

// offset[reg]
// offset is an (optional) expression, reg is a register
ParseStatus
V810AsmParser::parseMEMOperand(OperandVector &Operands) {
  SMLoc S = getTok().getLoc();
  SMLoc E = getTok().getEndLoc();

  // Parse the offset (if it exists)
  const MCExpr *EVal;
  if (getLexer().is(AsmToken::LBrac)) {
    EVal = MCConstantExpr::create(0, getContext());
  } else {
    if (parseImm16Expression(EVal, E))
      return ParseStatus::Failure;
  }
  getLexer().Lex(); // eat the [

  // parse the register
  MCRegister Reg;
  if (parseRegister(Reg, S, E))
    return ParseStatus::Failure;

  // eat the ]
  E = getTok().getEndLoc();
  if (!getLexer().is(AsmToken::RBrac))
    return ParseStatus::Failure;
  getLexer().Lex();

  Operands.push_back(V810Operand::CreateMEMri(Reg, EVal, S, E));

  return ParseStatus::Success;
}

ParseStatus
V810AsmParser::parseBranchTargetOperand(OperandVector &Operands) {
  return parseJumpTargetOperand(Operands, V810MCExpr::VK_V810_26_PCREL);
}

ParseStatus
V810AsmParser::parseBcondTargetOperand(OperandVector &Operands) {
  return parseJumpTargetOperand(Operands, V810MCExpr::VK_V810_9_PCREL);
}

ParseStatus
V810AsmParser::parseJumpTargetOperand(OperandVector &Operands, V810MCExpr::VariantKind Kind) {
  SMLoc S = getTok().getLoc();
  SMLoc E = getTok().getEndLoc();

  const MCExpr *DispValue;
  if (getParser().parseExpression(DispValue))
    return ParseStatus::Failure;

  int64_t Value;
  if (DispValue->evaluateAsAbsolute(Value)) {
    Operands.push_back(V810Operand::CreateImm(DispValue, S, E));
  } else {
    const V810MCExpr *DispExpr = V810MCExpr::create(Kind, DispValue, getContext());
    Operands.push_back(V810Operand::CreateImm(DispExpr, S, E));
  }
  return ParseStatus::Success;
}

ParseStatus
V810AsmParser::parseCondOperand(OperandVector &Operands) {
  SMLoc S = getTok().getLoc();
  SMLoc E = getTok().getEndLoc();

  if (getTok().isNot(AsmToken::Identifier))
    return ParseStatus::Failure;

  unsigned Val = llvm::StringSwitch<unsigned>(getTok().getString())
    .CaseLower("v", V810CC::CC_V)
    .CasesLower("c", "l", V810CC::CC_C)
    .CasesLower("e", "z", V810CC::CC_E)
    .CaseLower("nh", V810CC::CC_NH)
    .CaseLower("n", V810CC::CC_N)
    .CaseLower("t", V810CC::CC_BR)
    .CaseLower("lt", V810CC::CC_LT)
    .CaseLower("le", V810CC::CC_LE)
    .CaseLower("nv", V810CC::CC_NV)
    .CasesLower("nc", "nl", V810CC::CC_NC)
    .CasesLower("ne", "nz", V810CC::CC_NE)
    .CaseLower("h", V810CC::CC_H)
    .CaseLower("p", V810CC::CC_P)
    .CaseLower("f", V810CC::CC_NOP)
    .CaseLower("ge", V810CC::CC_GE)
    .CaseLower("gt", V810CC::CC_GT)
    .Default(-1u);
  if (Val == -1u)
    return ParseStatus::Failure;
  Lex();
  
  const MCExpr *Expr = MCConstantExpr::create(Val, getContext());
  Operands.push_back(V810Operand::CreateImm(Expr, S, E));
  return ParseStatus::Success;
}

// We need some slightly unusual logic to parse registers, and that seems hard to express in tablegen.
// so, handle them here.
ParseStatus
V810AsmParser::MatchOperandParserCustomImpl(OperandVector &Operands, StringRef Mnemonic) {
  SMLoc Start = getTok().getLoc();

  if (Mnemonic == "jmp" && Operands.size() == 1) {
    // for some reason, JMP's target is enclosed in brackets
    if (Parser.parseToken(AsmToken::LBrac))
      return ParseStatus::Failure;
    MCRegister RegNo;
    SMLoc S, E;
    if (parseRegister(RegNo, S, E))
      return ParseStatus::Failure;
    SMLoc End = getTok().getEndLoc();
    if (Parser.parseToken(AsmToken::RBrac))
      return ParseStatus::Failure;
    Operands.push_back(V810Operand::CreateReg(RegNo, Start, End));
    return ParseStatus::Success;    
  }

  if ((Mnemonic == "stsr" && Operands.size() == 1) ||
      (Mnemonic == "ldsr" && Operands.size() == 2)) {
    // Handle parsing a system register. They can be referenced by either name or number.
    // (don't handle this in tryParseRegister because in general, we should not treat integers as registers)

    std::string RegName;
    if (getTok().is(AsmToken::Integer)) {
      RegName = "sr" + getTok().getString().lower();
    } else {
      RegName = getTok().getString().lower();
    }

    MCRegister RegNo = MatchRegisterName(RegName);
    if (!RegNo) {
      RegNo = MatchRegisterAltName(RegName);
      if (!RegNo) {
        return ParseStatus::Failure;
      }
    }
    SMLoc End = getTok().getEndLoc();
    Lex();
    Operands.push_back(V810Operand::CreateReg(RegNo, Start, End));
    return ParseStatus::Success;
  }
  return MatchOperandParserImpl(Operands, Mnemonic);
}

ParseStatus
V810AsmParser::parseOperand(OperandVector &Operands, StringRef Mnemonic) {
  ParseStatus ResTy = MatchOperandParserCustomImpl(Operands, Mnemonic);

  // If there wasn't a custom match, try the generic matcher below. Otherwise,
  // there was a match, but an error occurred, in which case, just return that
  // the operand parsing failed.
  if (ResTy.isSuccess() || ResTy.isFailure())
    return ResTy;

  std::unique_ptr<V810Operand> Op;
  ResTy = parseV810AsmOperand(Op);
  if (!ResTy.isSuccess() || !Op)
    return ParseStatus::Failure;
  
  Operands.push_back(std::move(Op));

  return ParseStatus::Success;
}

ParseStatus
V810AsmParser::parseV810AsmOperand(std::unique_ptr<V810Operand> &Op) {
  SMLoc Start = Parser.getTok().getLoc();
  SMLoc End = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
  const MCExpr *EVal;

  Op = nullptr;
  switch (getLexer().getKind()) {
  default:  break;

  case AsmToken::Plus:
  case AsmToken::Minus:
  case AsmToken::Integer:
  case AsmToken::LParen:
  case AsmToken::Tilde:
    if (getParser().parseExpression(EVal, End))
      break;
    Op = V810Operand::CreateImm(EVal, Start, End);
    break;
  case AsmToken::Identifier:
    MCRegister IndexReg;
    ParseStatus result = tryParseRegister(IndexReg, Start, End);
    if (result.isSuccess()) {
      Op = V810Operand::CreateReg(IndexReg, Start, End);
      break;
    }

    if (parseImm16Expression(EVal, End))
      break;

    Op = V810Operand::CreateImm(EVal, Start, End);
  }

  return (Op) ? ParseStatus::Success : ParseStatus::Failure;
}

static bool evalPseudoOp(V810MCExpr::VariantKind Kind, int64_t &Value) {
  switch (Kind) {
  case V810MCExpr::VK_V810_LO:
    Value = EvalLo(Value);
    return true;
  case V810MCExpr::VK_V810_HI:
    Value = EvalHi(Value);
    return true;
  default:
    return false;
  }
}

bool V810AsmParser::parseImm16Expression(const MCExpr *&Res, SMLoc &EndLoc) {
  SMLoc StartLoc = getLexer().getLoc();
  if (getTok().isNot(AsmToken::Identifier)) {
    return getParser().parseExpression(Res, EndLoc);
  }
  auto Kind = StringSwitch<V810MCExpr::VariantKind>(getTok().getString())
                  .Case("lo", V810MCExpr::VK_V810_LO)
                  .Case("hi", V810MCExpr::VK_V810_HI)
                  .Case("sdaoff", V810MCExpr::VK_V810_SDAOFF)
                  .Default(V810MCExpr::VK_V810_None);
  if (Kind == V810MCExpr::VK_V810_None)
    return getParser().parseExpression(Res, EndLoc);

  Lex();
  if (getTok().isNot(AsmToken::LParen))
    return Error(StartLoc, "expected parenthesized expression");
  if (getParser().parseExpression(Res, EndLoc))
    return true;
  
  // Try constant folding hi and lo
  int64_t Value;
  if (Res->evaluateAsAbsolute(Value)) {
    if (evalPseudoOp(Kind, Value)) {
      Res = MCConstantExpr::create(Value, getContext());
      return false;
    }
  }

  Res = V810MCExpr::create(Kind, Res, getContext());
  return false;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810AsmParser() {
  RegisterMCAsmParser<V810AsmParser> X(getTheV810Target());
}

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "V810GenAsmMatcher.inc"