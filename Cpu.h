#pragma once
/* NOTE: never modifiy any instance variable after initialized...
 * as we need support multithread..
 * */
#include "common.h"
#include "Instruction.h"
#include "Disassembler.h"
class Cpu
{
public:
	Cpu();
	Instruction fetch();
	Instruction fetch(uint_t addr);
	void execute(const Instruction&);
	uint_t readOperand(const cs_mips_op&, size_t len=4);
	void writeOperand(const cs_mips_op&, uint_t value, size_t len=4);

	void load(uint_t addr, size_t len, uint8_t *buf);
	void store(uint_t addr, size_t len, const uint8_t *buf);

	uint_t _regs[MIPS_REG_ENDING+1];
private:
	MipsDisassembler _disassembler;

	Cpu(const Cpu& cpu);
#define DECLARE(HANDLE) void HANDLE(const cs_mips_op * operands)
	DECLARE(ADD);
	DECLARE(ADDU);
	DECLARE(SUB);
	DECLARE(SUBU);
	DECLARE(MULT);
	DECLARE(MULTU);
	DECLARE(DIV);
	DECLARE(DIVU);
	DECLARE(LW);
	DECLARE(LH);
	DECLARE(LHU);
	DECLARE(LB);
	DECLARE(LBU);
	DECLARE(SW);
	DECLARE(SH);
	DECLARE(SB);
	DECLARE(LUI);
	DECLARE(MFHI);
	DECLARE(MFLO);

	DECLARE(AND);
	DECLARE(OR);
	DECLARE(XOR);
	DECLARE(NOR);
	DECLARE(SLT);
	DECLARE(SLTU);

	DECLARE(SLL);
	DECLARE(SRL);
	DECLARE(SRA);

	DECLARE(BEQ);
	DECLARE(BNE);
	DECLARE(BEQZ);
	DECLARE(BNEZ);
	DECLARE(BLEZ);
	DECLARE(BLTZ);
	DECLARE(BGEZ);
	DECLARE(BGTZ);
	DECLARE(BAL);

	DECLARE(J);
	DECLARE(JAL);

	DECLARE(NOP);
	DECLARE(MOVE);
#undef DECLARE
};

