#pragma once
#include <capstone/capstone.h>
#include "common.h"
#define MIPS_REG_PC (MIPS_REG_ENDING)
enum InstructionType
{
	INS_INVALID,
	INS_NOP,
	INS_MOVE,
	INS_ADD,
	INS_ADDI,
	INS_ADDU,
	INS_ADDIU,
	INS_SUB,
	INS_SUBI,
	INS_SUBU,
	INS_SUBIU,
	INS_MULT,
	INS_MULTU,
	INS_DIV,
	INS_DIVU,

	INS_LW,
	INS_LH,
	INS_LHU,
	INS_LB,
	INS_LBU,
	INS_SW,
	INS_SH,
	INS_SB,
	INS_LUI,
	INS_MFHI,
	INS_MFLO,

	INS_AND,
	INS_ANDI,
	INS_OR,
	INS_ORI,
	INS_XOR,
	INS_XORI,
	INS_NOR,
	INS_NORI,
	INS_SLT,
	INS_SLTI,
	INS_SLTU,
	INS_SLTIU,

	INS_SLL,
	INS_SLLV,
	INS_SRL,
	INS_SRLV,
	INS_SRA,
	INS_SRAV,

	INS_BEQ,
	INS_BNE,
	INS_BEQZ,
	INS_BNEZ,
	INS_BLEZ,
	INS_BLTZ,
	INS_BGEZ,
	INS_BGTZ,
	INS_B,
	INS_BAL,

	INS_J,
	INS_JR,
	INS_JALR,
	INS_JAL,
	INS_MTC1,
	INS_CVT_D_W,
	INS_CVT_D_S,
	INS_CVT_S_D,


	INS_ENDING
};

struct Instruction
{
	InstructionType type;
	uint16_t size;
#ifdef CONFIG_WITH_MNEMONIC
	char mnemonic[MNEMONIC_MAX_LEN];
#endif
	uint8_t op_count;
	cs_mips_op operands[MAX_OPERAND_COUNT];
};

/* not mutlti-thread safe, but, will only used for debug, so...*/
extern const char * str_of_instruction(const Instruction&);


