#pragma once
#include <capstone/capstone.h>
#include "common.h"

/* when simulator reache to native library memery, if will fetch a MIPS_INS_SWITCH */
#define MIPS_INS_SWITCH (MIPS_INS_ENDING)
#define MIPS_REG_PC (MIPS_REG_ENDING)
struct Instruction
{
	mips_insn type;
	uint16_t size;
#ifdef CONFIG_WITH_MNEMONIC
	char mnemonic[MNEMONIC_MAX_LEN];
	uint_t addr;
	const uint8_t * buf;
#endif
	uint8_t op_count;
	cs_mips_op operands[MAX_OPERAND_COUNT];
};

/* not mutlti-thread safe, but, will only used for debug, so...*/
extern const char * str_of_instruction(const Instruction&);



