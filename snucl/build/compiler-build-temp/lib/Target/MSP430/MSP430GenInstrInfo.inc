//===- TableGen'erated file -------------------------------------*- C++ -*-===//
//
// Target Instruction Descriptors
//
// Automatically generated file, do not edit!
//
//===----------------------------------------------------------------------===//

namespace llvm {

static const unsigned ImplicitList1[] = { MSP430::SRW, 0 };
static const unsigned ImplicitList2[] = { MSP430::SPW, 0 };
static const unsigned ImplicitList3[] = { MSP430::SPW, MSP430::SRW, 0 };
static const unsigned ImplicitList4[] = { MSP430::R12W, MSP430::R13W, MSP430::R14W, MSP430::R15W, MSP430::SRW, 0 };

static const TargetOperandInfo OperandInfo2[] = { { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo3[] = { { -1, 0, 0 }, { -1, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo4[] = { { -1, 0, 0 }, { -1, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, { -1, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo5[] = { { -1, 0, 0 }, { -1, 0, 0 }, { -1, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo6[] = { { -1, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo7[] = { { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo8[] = { { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo9[] = { { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, };
static const TargetOperandInfo OperandInfo10[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo11[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo12[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, { MSP430::GR16RegClassID, 0, 0 }, };
static const TargetOperandInfo OperandInfo13[] = { { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, { MSP430::GR8RegClassID, 0, 0 }, };
static const TargetOperandInfo OperandInfo14[] = { { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo15[] = { { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo16[] = { { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, { MSP430::GR8RegClassID, 0, 0 }, };
static const TargetOperandInfo OperandInfo17[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, { MSP430::GR16RegClassID, 0, ((1 << 16) | (1 << TOI::TIED_TO)) }, };
static const TargetOperandInfo OperandInfo18[] = { { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, { MSP430::GR16RegClassID, 0, ((1 << 16) | (1 << TOI::TIED_TO)) }, };
static const TargetOperandInfo OperandInfo19[] = { { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo20[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo21[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, };
static const TargetOperandInfo OperandInfo22[] = { { MSP430::GR8RegClassID, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo23[] = { { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo24[] = { { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, 0 }, };
static const TargetOperandInfo OperandInfo25[] = { { MSP430::GR16RegClassID, 0, 0 }, };
static const TargetOperandInfo OperandInfo26[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, ((1 << 16) | (1 << TOI::TIED_TO)) }, };
static const TargetOperandInfo OperandInfo27[] = { { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, ((1 << 16) | (1 << TOI::TIED_TO)) }, };
static const TargetOperandInfo OperandInfo28[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, 0 }, };
static const TargetOperandInfo OperandInfo29[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, };
static const TargetOperandInfo OperandInfo30[] = { { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, ((0 << 16) | (1 << TOI::TIED_TO)) }, };
static const TargetOperandInfo OperandInfo31[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo32[] = { { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, 0 }, { -1, 0, 0 }, };
static const TargetOperandInfo OperandInfo33[] = { { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR16RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, 0 }, };
static const TargetOperandInfo OperandInfo34[] = { { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, 0 }, { MSP430::GR8RegClassID, 0, 0 }, };

static const TargetInstrDesc MSP430Insts[] = {
  { 0,	0,	0,	0,	"PHI", 0|(1<<TID::Variadic)|(1<<TID::UnmodeledSideEffects), 0x0ULL, NULL, NULL, NULL, 0 },  // Inst #0 = PHI
  { 1,	0,	0,	0,	"INLINEASM", 0|(1<<TID::Variadic), 0x0ULL, NULL, NULL, NULL, 0 },  // Inst #1 = INLINEASM
  { 2,	1,	0,	0,	"PROLOG_LABEL", 0|(1<<TID::NotDuplicable)|(1<<TID::UnmodeledSideEffects), 0x0ULL, NULL, NULL, NULL, OperandInfo2 },  // Inst #2 = PROLOG_LABEL
  { 3,	1,	0,	0,	"EH_LABEL", 0|(1<<TID::NotDuplicable)|(1<<TID::UnmodeledSideEffects), 0x0ULL, NULL, NULL, NULL, OperandInfo2 },  // Inst #3 = EH_LABEL
  { 4,	1,	0,	0,	"GC_LABEL", 0|(1<<TID::NotDuplicable)|(1<<TID::UnmodeledSideEffects), 0x0ULL, NULL, NULL, NULL, OperandInfo2 },  // Inst #4 = GC_LABEL
  { 5,	0,	0,	0,	"KILL", 0|(1<<TID::Variadic), 0x0ULL, NULL, NULL, NULL, 0 },  // Inst #5 = KILL
  { 6,	3,	1,	0,	"EXTRACT_SUBREG", 0, 0x0ULL, NULL, NULL, NULL, OperandInfo3 },  // Inst #6 = EXTRACT_SUBREG
  { 7,	4,	1,	0,	"INSERT_SUBREG", 0, 0x0ULL, NULL, NULL, NULL, OperandInfo4 },  // Inst #7 = INSERT_SUBREG
  { 8,	1,	1,	0,	"IMPLICIT_DEF", 0|(1<<TID::Rematerializable)|(1<<TID::CheapAsAMove), 0x0ULL, NULL, NULL, NULL, OperandInfo2 },  // Inst #8 = IMPLICIT_DEF
  { 9,	4,	1,	0,	"SUBREG_TO_REG", 0, 0x0ULL, NULL, NULL, NULL, OperandInfo5 },  // Inst #9 = SUBREG_TO_REG
  { 10,	3,	1,	0,	"COPY_TO_REGCLASS", 0|(1<<TID::CheapAsAMove), 0x0ULL, NULL, NULL, NULL, OperandInfo3 },  // Inst #10 = COPY_TO_REGCLASS
  { 11,	0,	0,	0,	"DBG_VALUE", 0|(1<<TID::Variadic), 0x0ULL, NULL, NULL, NULL, 0 },  // Inst #11 = DBG_VALUE
  { 12,	1,	1,	0,	"REG_SEQUENCE", 0|(1<<TID::Variadic)|(1<<TID::CheapAsAMove), 0x0ULL, NULL, NULL, NULL, OperandInfo2 },  // Inst #12 = REG_SEQUENCE
  { 13,	2,	1,	0,	"COPY", 0|(1<<TID::CheapAsAMove), 0x0ULL, NULL, NULL, NULL, OperandInfo6 },  // Inst #13 = COPY
  { 14,	3,	0,	0,	"ADC16mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, ImplicitList1, ImplicitList1, NULL, OperandInfo7 },  // Inst #14 = ADC16mi
  { 15,	4,	0,	0,	"ADC16mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, ImplicitList1, ImplicitList1, NULL, OperandInfo8 },  // Inst #15 = ADC16mm
  { 16,	3,	0,	0,	"ADC16mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo9 },  // Inst #16 = ADC16mr
  { 17,	3,	1,	0,	"ADC16ri", 0, 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo10 },  // Inst #17 = ADC16ri
  { 18,	4,	1,	0,	"ADC16rm", 0|(1<<TID::MayLoad), 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo11 },  // Inst #18 = ADC16rm
  { 19,	3,	1,	0,	"ADC16rr", 0|(1<<TID::Commutable), 0xaULL, ImplicitList1, ImplicitList1, NULL, OperandInfo12 },  // Inst #19 = ADC16rr
  { 20,	3,	0,	0,	"ADC8mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, ImplicitList1, ImplicitList1, NULL, OperandInfo7 },  // Inst #20 = ADC8mi
  { 21,	4,	0,	0,	"ADC8mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, ImplicitList1, ImplicitList1, NULL, OperandInfo8 },  // Inst #21 = ADC8mm
  { 22,	3,	0,	0,	"ADC8mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo13 },  // Inst #22 = ADC8mr
  { 23,	3,	1,	0,	"ADC8ri", 0, 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo14 },  // Inst #23 = ADC8ri
  { 24,	4,	1,	0,	"ADC8rm", 0|(1<<TID::MayLoad), 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo15 },  // Inst #24 = ADC8rm
  { 25,	3,	1,	0,	"ADC8rr", 0|(1<<TID::Commutable), 0xaULL, ImplicitList1, ImplicitList1, NULL, OperandInfo16 },  // Inst #25 = ADC8rr
  { 26,	3,	0,	0,	"ADD16mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #26 = ADD16mi
  { 27,	4,	0,	0,	"ADD16mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #27 = ADD16mm
  { 28,	3,	0,	0,	"ADD16mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo9 },  // Inst #28 = ADD16mr
  { 29,	3,	1,	0,	"ADD16ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo10 },  // Inst #29 = ADD16ri
  { 30,	4,	1,	0,	"ADD16rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo11 },  // Inst #30 = ADD16rm
  { 31,	4,	2,	0,	"ADD16rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo17 },  // Inst #31 = ADD16rm_POST
  { 32,	3,	1,	0,	"ADD16rr", 0|(1<<TID::Commutable), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo12 },  // Inst #32 = ADD16rr
  { 33,	3,	0,	0,	"ADD8mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #33 = ADD8mi
  { 34,	4,	0,	0,	"ADD8mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #34 = ADD8mm
  { 35,	3,	0,	0,	"ADD8mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo13 },  // Inst #35 = ADD8mr
  { 36,	3,	1,	0,	"ADD8ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo14 },  // Inst #36 = ADD8ri
  { 37,	4,	1,	0,	"ADD8rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo15 },  // Inst #37 = ADD8rm
  { 38,	4,	2,	0,	"ADD8rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo18 },  // Inst #38 = ADD8rm_POST
  { 39,	3,	1,	0,	"ADD8rr", 0|(1<<TID::Commutable), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo16 },  // Inst #39 = ADD8rr
  { 40,	1,	0,	0,	"ADJCALLSTACKDOWN", 0, 0x4ULL, ImplicitList2, ImplicitList3, NULL, OperandInfo2 },  // Inst #40 = ADJCALLSTACKDOWN
  { 41,	2,	0,	0,	"ADJCALLSTACKUP", 0, 0x4ULL, ImplicitList2, ImplicitList3, NULL, OperandInfo6 },  // Inst #41 = ADJCALLSTACKUP
  { 42,	3,	0,	0,	"AND16mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #42 = AND16mi
  { 43,	4,	0,	0,	"AND16mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #43 = AND16mm
  { 44,	3,	0,	0,	"AND16mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo9 },  // Inst #44 = AND16mr
  { 45,	3,	1,	0,	"AND16ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo10 },  // Inst #45 = AND16ri
  { 46,	4,	1,	0,	"AND16rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo11 },  // Inst #46 = AND16rm
  { 47,	4,	2,	0,	"AND16rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo17 },  // Inst #47 = AND16rm_POST
  { 48,	3,	1,	0,	"AND16rr", 0|(1<<TID::Commutable), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo12 },  // Inst #48 = AND16rr
  { 49,	3,	0,	0,	"AND8mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #49 = AND8mi
  { 50,	4,	0,	0,	"AND8mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #50 = AND8mm
  { 51,	3,	0,	0,	"AND8mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo13 },  // Inst #51 = AND8mr
  { 52,	3,	1,	0,	"AND8ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo14 },  // Inst #52 = AND8ri
  { 53,	4,	1,	0,	"AND8rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo15 },  // Inst #53 = AND8rm
  { 54,	4,	2,	0,	"AND8rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo18 },  // Inst #54 = AND8rm_POST
  { 55,	3,	1,	0,	"AND8rr", 0|(1<<TID::Commutable), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo16 },  // Inst #55 = AND8rr
  { 56,	4,	0,	0,	"BIC16mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #56 = BIC16mm
  { 57,	3,	0,	0,	"BIC16mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo9 },  // Inst #57 = BIC16mr
  { 58,	4,	1,	0,	"BIC16rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo11 },  // Inst #58 = BIC16rm
  { 59,	3,	1,	0,	"BIC16rr", 0, 0xaULL, NULL, ImplicitList1, NULL, OperandInfo12 },  // Inst #59 = BIC16rr
  { 60,	4,	0,	0,	"BIC8mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #60 = BIC8mm
  { 61,	3,	0,	0,	"BIC8mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo13 },  // Inst #61 = BIC8mr
  { 62,	4,	1,	0,	"BIC8rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo15 },  // Inst #62 = BIC8rm
  { 63,	3,	1,	0,	"BIC8rr", 0, 0xaULL, NULL, ImplicitList1, NULL, OperandInfo16 },  // Inst #63 = BIC8rr
  { 64,	3,	0,	0,	"BIT16mi", 0|(1<<TID::MayLoad), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #64 = BIT16mi
  { 65,	4,	0,	0,	"BIT16mm", 0|(1<<TID::MayLoad), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #65 = BIT16mm
  { 66,	3,	0,	0,	"BIT16mr", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo9 },  // Inst #66 = BIT16mr
  { 67,	2,	0,	0,	"BIT16ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo19 },  // Inst #67 = BIT16ri
  { 68,	3,	0,	0,	"BIT16rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo20 },  // Inst #68 = BIT16rm
  { 69,	2,	0,	0,	"BIT16rr", 0|(1<<TID::Commutable), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo21 },  // Inst #69 = BIT16rr
  { 70,	3,	0,	0,	"BIT8mi", 0|(1<<TID::MayLoad), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #70 = BIT8mi
  { 71,	4,	0,	0,	"BIT8mm", 0|(1<<TID::MayLoad), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #71 = BIT8mm
  { 72,	3,	0,	0,	"BIT8mr", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo13 },  // Inst #72 = BIT8mr
  { 73,	2,	0,	0,	"BIT8ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo22 },  // Inst #73 = BIT8ri
  { 74,	3,	0,	0,	"BIT8rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo23 },  // Inst #74 = BIT8rm
  { 75,	2,	0,	0,	"BIT8rr", 0|(1<<TID::Commutable), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo24 },  // Inst #75 = BIT8rr
  { 76,	1,	0,	0,	"Bi", 0|(1<<TID::Branch)|(1<<TID::IndirectBranch)|(1<<TID::Barrier)|(1<<TID::Terminator), 0xeULL, NULL, NULL, NULL, OperandInfo2 },  // Inst #76 = Bi
  { 77,	2,	0,	0,	"Bm", 0|(1<<TID::Branch)|(1<<TID::IndirectBranch)|(1<<TID::Barrier)|(1<<TID::MayLoad)|(1<<TID::Terminator), 0xeULL, NULL, NULL, NULL, OperandInfo19 },  // Inst #77 = Bm
  { 78,	1,	0,	0,	"Br", 0|(1<<TID::Branch)|(1<<TID::IndirectBranch)|(1<<TID::Barrier)|(1<<TID::Terminator), 0xaULL, NULL, NULL, NULL, OperandInfo25 },  // Inst #78 = Br
  { 79,	1,	0,	0,	"CALLi", 0|(1<<TID::Call)|(1<<TID::Variadic), 0xdULL, ImplicitList2, ImplicitList4, NULL, OperandInfo2 },  // Inst #79 = CALLi
  { 80,	2,	0,	0,	"CALLm", 0|(1<<TID::Call)|(1<<TID::MayLoad)|(1<<TID::Variadic), 0xdULL, ImplicitList2, ImplicitList4, NULL, OperandInfo19 },  // Inst #80 = CALLm
  { 81,	1,	0,	0,	"CALLr", 0|(1<<TID::Call)|(1<<TID::Variadic), 0x9ULL, ImplicitList2, ImplicitList4, NULL, OperandInfo25 },  // Inst #81 = CALLr
  { 82,	3,	0,	0,	"CMP16mi", 0|(1<<TID::MayLoad), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #82 = CMP16mi
  { 83,	3,	0,	0,	"CMP16mr", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo9 },  // Inst #83 = CMP16mr
  { 84,	2,	0,	0,	"CMP16ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo19 },  // Inst #84 = CMP16ri
  { 85,	3,	0,	0,	"CMP16rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo20 },  // Inst #85 = CMP16rm
  { 86,	2,	0,	0,	"CMP16rr", 0, 0xaULL, NULL, ImplicitList1, NULL, OperandInfo21 },  // Inst #86 = CMP16rr
  { 87,	3,	0,	0,	"CMP8mi", 0|(1<<TID::MayLoad), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #87 = CMP8mi
  { 88,	3,	0,	0,	"CMP8mr", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo13 },  // Inst #88 = CMP8mr
  { 89,	2,	0,	0,	"CMP8ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo22 },  // Inst #89 = CMP8ri
  { 90,	3,	0,	0,	"CMP8rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo23 },  // Inst #90 = CMP8rm
  { 91,	2,	0,	0,	"CMP8rr", 0, 0xaULL, NULL, ImplicitList1, NULL, OperandInfo24 },  // Inst #91 = CMP8rr
  { 92,	2,	0,	0,	"JCC", 0|(1<<TID::Branch)|(1<<TID::Terminator), 0xbULL, ImplicitList1, NULL, NULL, OperandInfo6 },  // Inst #92 = JCC
  { 93,	1,	0,	0,	"JMP", 0|(1<<TID::Branch)|(1<<TID::Barrier)|(1<<TID::Terminator), 0xbULL, NULL, NULL, NULL, OperandInfo2 },  // Inst #93 = JMP
  { 94,	3,	0,	0,	"MOV16mi", 0|(1<<TID::MayStore), 0x12ULL, NULL, NULL, NULL, OperandInfo7 },  // Inst #94 = MOV16mi
  { 95,	4,	0,	0,	"MOV16mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, NULL, NULL, OperandInfo8 },  // Inst #95 = MOV16mm
  { 96,	3,	0,	0,	"MOV16mr", 0|(1<<TID::MayStore), 0xeULL, NULL, NULL, NULL, OperandInfo9 },  // Inst #96 = MOV16mr
  { 97,	2,	1,	0,	"MOV16ri", 0|(1<<TID::Rematerializable)|(1<<TID::CheapAsAMove), 0xeULL, NULL, NULL, NULL, OperandInfo19 },  // Inst #97 = MOV16ri
  { 98,	3,	1,	0,	"MOV16rm", 0|(1<<TID::FoldableAsLoad)|(1<<TID::MayLoad)|(1<<TID::Rematerializable), 0xeULL, NULL, NULL, NULL, OperandInfo20 },  // Inst #98 = MOV16rm
  { 99,	3,	2,	0,	"MOV16rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, NULL, NULL, OperandInfo26 },  // Inst #99 = MOV16rm_POST
  { 100,	2,	1,	0,	"MOV16rr", 0, 0xaULL, NULL, NULL, NULL, OperandInfo21 },  // Inst #100 = MOV16rr
  { 101,	3,	0,	0,	"MOV8mi", 0|(1<<TID::MayStore), 0x12ULL, NULL, NULL, NULL, OperandInfo7 },  // Inst #101 = MOV8mi
  { 102,	4,	0,	0,	"MOV8mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, NULL, NULL, OperandInfo8 },  // Inst #102 = MOV8mm
  { 103,	3,	0,	0,	"MOV8mr", 0|(1<<TID::MayStore), 0xeULL, NULL, NULL, NULL, OperandInfo13 },  // Inst #103 = MOV8mr
  { 104,	2,	1,	0,	"MOV8ri", 0|(1<<TID::Rematerializable)|(1<<TID::CheapAsAMove), 0xeULL, NULL, NULL, NULL, OperandInfo22 },  // Inst #104 = MOV8ri
  { 105,	3,	1,	0,	"MOV8rm", 0|(1<<TID::FoldableAsLoad)|(1<<TID::MayLoad)|(1<<TID::Rematerializable), 0xeULL, NULL, NULL, NULL, OperandInfo23 },  // Inst #105 = MOV8rm
  { 106,	3,	2,	0,	"MOV8rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, NULL, NULL, OperandInfo27 },  // Inst #106 = MOV8rm_POST
  { 107,	2,	1,	0,	"MOV8rr", 0, 0xaULL, NULL, NULL, NULL, OperandInfo24 },  // Inst #107 = MOV8rr
  { 108,	3,	1,	0,	"MOVZX16rm8", 0|(1<<TID::MayLoad), 0xeULL, NULL, NULL, NULL, OperandInfo20 },  // Inst #108 = MOVZX16rm8
  { 109,	2,	1,	0,	"MOVZX16rr8", 0, 0xaULL, NULL, NULL, NULL, OperandInfo28 },  // Inst #109 = MOVZX16rr8
  { 110,	0,	0,	0,	"NOP", 0, 0x4ULL, NULL, NULL, NULL, 0 },  // Inst #110 = NOP
  { 111,	3,	0,	0,	"OR16mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #111 = OR16mi
  { 112,	4,	0,	0,	"OR16mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #112 = OR16mm
  { 113,	3,	0,	0,	"OR16mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo9 },  // Inst #113 = OR16mr
  { 114,	3,	1,	0,	"OR16ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo10 },  // Inst #114 = OR16ri
  { 115,	4,	1,	0,	"OR16rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo11 },  // Inst #115 = OR16rm
  { 116,	4,	2,	0,	"OR16rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo17 },  // Inst #116 = OR16rm_POST
  { 117,	3,	1,	0,	"OR16rr", 0|(1<<TID::Commutable), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo12 },  // Inst #117 = OR16rr
  { 118,	3,	0,	0,	"OR8mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #118 = OR8mi
  { 119,	4,	0,	0,	"OR8mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #119 = OR8mm
  { 120,	3,	0,	0,	"OR8mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo13 },  // Inst #120 = OR8mr
  { 121,	3,	1,	0,	"OR8ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo14 },  // Inst #121 = OR8ri
  { 122,	4,	1,	0,	"OR8rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo15 },  // Inst #122 = OR8rm
  { 123,	4,	2,	0,	"OR8rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo18 },  // Inst #123 = OR8rm_POST
  { 124,	3,	1,	0,	"OR8rr", 0|(1<<TID::Commutable), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo16 },  // Inst #124 = OR8rr
  { 125,	1,	1,	0,	"POP16r", 0|(1<<TID::MayLoad), 0xaULL, ImplicitList2, ImplicitList2, NULL, OperandInfo25 },  // Inst #125 = POP16r
  { 126,	1,	0,	0,	"PUSH16r", 0|(1<<TID::MayStore), 0x9ULL, ImplicitList2, ImplicitList2, NULL, OperandInfo25 },  // Inst #126 = PUSH16r
  { 127,	0,	0,	0,	"RET", 0|(1<<TID::Return)|(1<<TID::Barrier)|(1<<TID::Terminator), 0xaULL, NULL, NULL, NULL, 0 },  // Inst #127 = RET
  { 128,	0,	0,	0,	"RETI", 0|(1<<TID::Return)|(1<<TID::Barrier)|(1<<TID::Terminator), 0x9ULL, NULL, NULL, NULL, 0 },  // Inst #128 = RETI
  { 129,	2,	1,	0,	"SAR16r1", 0, 0x9ULL, NULL, ImplicitList1, NULL, OperandInfo29 },  // Inst #129 = SAR16r1
  { 130,	2,	1,	0,	"SAR16r1c", 0, 0x4ULL, NULL, ImplicitList1, NULL, OperandInfo29 },  // Inst #130 = SAR16r1c
  { 131,	2,	1,	0,	"SAR8r1", 0, 0x9ULL, NULL, ImplicitList1, NULL, OperandInfo30 },  // Inst #131 = SAR8r1
  { 132,	2,	1,	0,	"SAR8r1c", 0, 0x4ULL, NULL, ImplicitList1, NULL, OperandInfo30 },  // Inst #132 = SAR8r1c
  { 133,	3,	0,	0,	"SBC16mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, ImplicitList1, ImplicitList1, NULL, OperandInfo7 },  // Inst #133 = SBC16mi
  { 134,	4,	0,	0,	"SBC16mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, ImplicitList1, ImplicitList1, NULL, OperandInfo8 },  // Inst #134 = SBC16mm
  { 135,	3,	0,	0,	"SBC16mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo9 },  // Inst #135 = SBC16mr
  { 136,	3,	1,	0,	"SBC16ri", 0, 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo10 },  // Inst #136 = SBC16ri
  { 137,	4,	1,	0,	"SBC16rm", 0|(1<<TID::MayLoad), 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo11 },  // Inst #137 = SBC16rm
  { 138,	3,	1,	0,	"SBC16rr", 0, 0xaULL, ImplicitList1, ImplicitList1, NULL, OperandInfo12 },  // Inst #138 = SBC16rr
  { 139,	3,	0,	0,	"SBC8mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, ImplicitList1, ImplicitList1, NULL, OperandInfo7 },  // Inst #139 = SBC8mi
  { 140,	4,	0,	0,	"SBC8mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, ImplicitList1, ImplicitList1, NULL, OperandInfo8 },  // Inst #140 = SBC8mm
  { 141,	3,	0,	0,	"SBC8mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo13 },  // Inst #141 = SBC8mr
  { 142,	3,	1,	0,	"SBC8ri", 0, 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo14 },  // Inst #142 = SBC8ri
  { 143,	4,	1,	0,	"SBC8rm", 0|(1<<TID::MayLoad), 0xeULL, ImplicitList1, ImplicitList1, NULL, OperandInfo15 },  // Inst #143 = SBC8rm
  { 144,	3,	1,	0,	"SBC8rr", 0, 0xaULL, ImplicitList1, ImplicitList1, NULL, OperandInfo16 },  // Inst #144 = SBC8rr
  { 145,	2,	1,	0,	"SEXT16r", 0, 0x9ULL, NULL, ImplicitList1, NULL, OperandInfo29 },  // Inst #145 = SEXT16r
  { 146,	2,	1,	0,	"SHL16r1", 0, 0xaULL, NULL, ImplicitList1, NULL, OperandInfo29 },  // Inst #146 = SHL16r1
  { 147,	2,	1,	0,	"SHL8r1", 0, 0xaULL, NULL, ImplicitList1, NULL, OperandInfo30 },  // Inst #147 = SHL8r1
  { 148,	3,	0,	0,	"SUB16mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #148 = SUB16mi
  { 149,	4,	0,	0,	"SUB16mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #149 = SUB16mm
  { 150,	3,	0,	0,	"SUB16mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo9 },  // Inst #150 = SUB16mr
  { 151,	3,	1,	0,	"SUB16ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo10 },  // Inst #151 = SUB16ri
  { 152,	4,	1,	0,	"SUB16rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo11 },  // Inst #152 = SUB16rm
  { 153,	4,	2,	0,	"SUB16rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo17 },  // Inst #153 = SUB16rm_POST
  { 154,	3,	1,	0,	"SUB16rr", 0, 0xaULL, NULL, ImplicitList1, NULL, OperandInfo12 },  // Inst #154 = SUB16rr
  { 155,	3,	0,	0,	"SUB8mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #155 = SUB8mi
  { 156,	4,	0,	0,	"SUB8mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #156 = SUB8mm
  { 157,	3,	0,	0,	"SUB8mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo13 },  // Inst #157 = SUB8mr
  { 158,	3,	1,	0,	"SUB8ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo14 },  // Inst #158 = SUB8ri
  { 159,	4,	1,	0,	"SUB8rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo15 },  // Inst #159 = SUB8rm
  { 160,	4,	2,	0,	"SUB8rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo18 },  // Inst #160 = SUB8rm_POST
  { 161,	3,	1,	0,	"SUB8rr", 0, 0xaULL, NULL, ImplicitList1, NULL, OperandInfo16 },  // Inst #161 = SUB8rr
  { 162,	2,	1,	0,	"SWPB16r", 0, 0x9ULL, NULL, NULL, NULL, OperandInfo29 },  // Inst #162 = SWPB16r
  { 163,	4,	1,	0,	"Select16", 0|(1<<TID::UsesCustomInserter), 0x4ULL, NULL, NULL, NULL, OperandInfo31 },  // Inst #163 = Select16
  { 164,	4,	1,	0,	"Select8", 0|(1<<TID::UsesCustomInserter), 0x4ULL, NULL, NULL, NULL, OperandInfo32 },  // Inst #164 = Select8
  { 165,	3,	1,	0,	"Shl16", 0|(1<<TID::UsesCustomInserter), 0x4ULL, NULL, ImplicitList1, NULL, OperandInfo33 },  // Inst #165 = Shl16
  { 166,	3,	1,	0,	"Shl8", 0|(1<<TID::UsesCustomInserter), 0x4ULL, NULL, ImplicitList1, NULL, OperandInfo34 },  // Inst #166 = Shl8
  { 167,	3,	1,	0,	"Sra16", 0|(1<<TID::UsesCustomInserter), 0x4ULL, NULL, ImplicitList1, NULL, OperandInfo33 },  // Inst #167 = Sra16
  { 168,	3,	1,	0,	"Sra8", 0|(1<<TID::UsesCustomInserter), 0x4ULL, NULL, ImplicitList1, NULL, OperandInfo34 },  // Inst #168 = Sra8
  { 169,	3,	1,	0,	"Srl16", 0|(1<<TID::UsesCustomInserter), 0x4ULL, NULL, ImplicitList1, NULL, OperandInfo33 },  // Inst #169 = Srl16
  { 170,	3,	1,	0,	"Srl8", 0|(1<<TID::UsesCustomInserter), 0x4ULL, NULL, ImplicitList1, NULL, OperandInfo34 },  // Inst #170 = Srl8
  { 171,	3,	0,	0,	"XOR16mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #171 = XOR16mi
  { 172,	4,	0,	0,	"XOR16mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #172 = XOR16mm
  { 173,	3,	0,	0,	"XOR16mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo9 },  // Inst #173 = XOR16mr
  { 174,	3,	1,	0,	"XOR16ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo10 },  // Inst #174 = XOR16ri
  { 175,	4,	1,	0,	"XOR16rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo11 },  // Inst #175 = XOR16rm
  { 176,	4,	2,	0,	"XOR16rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo17 },  // Inst #176 = XOR16rm_POST
  { 177,	3,	1,	0,	"XOR16rr", 0|(1<<TID::Commutable), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo12 },  // Inst #177 = XOR16rr
  { 178,	3,	0,	0,	"XOR8mi", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo7 },  // Inst #178 = XOR8mi
  { 179,	4,	0,	0,	"XOR8mm", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0x12ULL, NULL, ImplicitList1, NULL, OperandInfo8 },  // Inst #179 = XOR8mm
  { 180,	3,	0,	0,	"XOR8mr", 0|(1<<TID::MayLoad)|(1<<TID::MayStore), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo13 },  // Inst #180 = XOR8mr
  { 181,	3,	1,	0,	"XOR8ri", 0, 0xeULL, NULL, ImplicitList1, NULL, OperandInfo14 },  // Inst #181 = XOR8ri
  { 182,	4,	1,	0,	"XOR8rm", 0|(1<<TID::MayLoad), 0xeULL, NULL, ImplicitList1, NULL, OperandInfo15 },  // Inst #182 = XOR8rm
  { 183,	4,	2,	0,	"XOR8rm_POST", 0|(1<<TID::MayLoad)|(1<<TID::UnmodeledSideEffects)|(1<<TID::ExtraDefRegAllocReq), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo18 },  // Inst #183 = XOR8rm_POST
  { 184,	3,	1,	0,	"XOR8rr", 0|(1<<TID::Commutable), 0xaULL, NULL, ImplicitList1, NULL, OperandInfo16 },  // Inst #184 = XOR8rr
  { 185,	2,	1,	0,	"ZEXT16r", 0, 0xaULL, NULL, NULL, NULL, OperandInfo29 },  // Inst #185 = ZEXT16r
};
} // End llvm namespace 