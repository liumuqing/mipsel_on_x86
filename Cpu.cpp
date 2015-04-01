#include <stdint.h>
#include <string.h>
#include <capstone/capstone.h>
#include "common.h"
#include "Cpu.h"
#include "Instruction.h"
Cpu::Cpu()
{
	for (int i = 0; i< MIPS_REG_ENDING+1; i++)
	{
		_regs[i] = 0;
	}
}
Instruction Cpu::fetch()
{
	return fetch(_regs[MIPS_REG_PC]);
}

Instruction Cpu::fetch(uint_t pc)
{
	return _disassembler.disassemble((const uint8_t*) pc);
}
void Cpu::execute(const Instruction& inst)
{
	switch (inst.type)
	{
#define BIND_HANDLE(type, handle) case type: handle(inst.operands); break
		BIND_HANDLE(MIPS_INS_ADD, ADD);
		BIND_HANDLE(MIPS_INS_ADDI, ADD);

		BIND_HANDLE(MIPS_INS_ADDU, ADDU);
		BIND_HANDLE(MIPS_INS_ADDIU, ADDU);

		BIND_HANDLE(MIPS_INS_SUB, SUB);
		//BIND_HANDLE(MIPS_INS_SUBI, SUB); NO subi, which is replaced by addi

		BIND_HANDLE(MIPS_INS_SUBU, SUBU);
		//BIND_HANDLE(MIPS_INS_SUBIU, SUBU);


		BIND_HANDLE(MIPS_INS_MULT, MULT);
		BIND_HANDLE(MIPS_INS_MULTU, MULTU);
		BIND_HANDLE(MIPS_INS_DIV, DIV);
		BIND_HANDLE(MIPS_INS_DIVU, DIVU);

		BIND_HANDLE(MIPS_INS_LW, LW);
		BIND_HANDLE(MIPS_INS_LH, LH);
		BIND_HANDLE(MIPS_INS_LHU, LHU);
		BIND_HANDLE(MIPS_INS_LB, LB);
		BIND_HANDLE(MIPS_INS_LBU, LBU);

		BIND_HANDLE(MIPS_INS_SW, SW);
		BIND_HANDLE(MIPS_INS_SH, SH);
		BIND_HANDLE(MIPS_INS_SB, SB);

		BIND_HANDLE(MIPS_INS_LUI, LUI);


		BIND_HANDLE(MIPS_INS_MFHI, MFHI);
		BIND_HANDLE(MIPS_INS_MFLO, MFLO);


		BIND_HANDLE(MIPS_INS_AND, AND);
		BIND_HANDLE(MIPS_INS_ANDI, AND);
		BIND_HANDLE(MIPS_INS_OR, OR);
		BIND_HANDLE(MIPS_INS_ORI, OR);
		BIND_HANDLE(MIPS_INS_XOR, XOR);
		BIND_HANDLE(MIPS_INS_XORI, XOR);
		BIND_HANDLE(MIPS_INS_NOR, NOR);
		BIND_HANDLE(MIPS_INS_NORI, NOR);
		BIND_HANDLE(MIPS_INS_SLT, SLT);
		BIND_HANDLE(MIPS_INS_SLTI, SLT);
		BIND_HANDLE(MIPS_INS_SLTU, SLTU);
		BIND_HANDLE(MIPS_INS_SLTIU, SLTU);

		BIND_HANDLE(MIPS_INS_SLL, SLL);
		BIND_HANDLE(MIPS_INS_SLLV, SLL);
		BIND_HANDLE(MIPS_INS_SRL, SRL);
		BIND_HANDLE(MIPS_INS_SRLV, SRL);
		BIND_HANDLE(MIPS_INS_SRA, SRA);
		BIND_HANDLE(MIPS_INS_SRAV, SRA);

		BIND_HANDLE(MIPS_INS_BAL, JAL);
		BIND_HANDLE(MIPS_INS_BEQ, BEQ);
		BIND_HANDLE(MIPS_INS_BNE, BNE);
		BIND_HANDLE(MIPS_INS_BEQZ, BEQZ);
		BIND_HANDLE(MIPS_INS_BNEZ, BNEZ);
		BIND_HANDLE(MIPS_INS_BLEZ, BLEZ);
		BIND_HANDLE(MIPS_INS_BLTZ, BLTZ);
		BIND_HANDLE(MIPS_INS_BGEZ, BGEZ);
		BIND_HANDLE(MIPS_INS_BGTZ, BGTZ);
		BIND_HANDLE(MIPS_INS_B, J);

		BIND_HANDLE(MIPS_INS_J, J);
		BIND_HANDLE(MIPS_INS_JR, J);//note, never bind hazard instruction such as MIPS_INS_JR_HB
		BIND_HANDLE(MIPS_INS_JAL, JAL);
		BIND_HANDLE(MIPS_INS_JALR, JAL);

		BIND_HANDLE(MIPS_INS_NOP, NOP);
		BIND_HANDLE(MIPS_INS_MOVE, MOVE);
		BIND_HANDLE(MIPS_INS_NEGU, SUBU);

		default:
			ERROR("Unimplemented Instruction %s", str_of_instruction(inst));
#undef BIND_HANDLE
	}
}


uint_t Cpu::readOperand(const cs_mips_op& operand, size_t len)
{
	switch (operand.type)
	{
	case MIPS_OP_REG:
		ASSERT(len == 4);
		return _regs[operand.reg];
	case MIPS_OP_IMM:
		return operand.imm;
	case MIPS_OP_MEM:
		{
			uint_t retv = 0;//retv must be initialize as zero
			ASSERT(len <= 4 && len > 0);
			load(_regs[operand.mem.base] + (uint_t)operand.mem.disp, len, (uint8_t *)&retv);
			return retv;
		}
	default:
		ERROR("Unkonw operand type:%d", operand.type);
	}
}

void Cpu::writeOperand(const cs_mips_op& operand, uint_t value, size_t len)
{
	switch (operand.type)
	{
	case MIPS_OP_REG:
		ASSERT(len == 4);
		_regs[operand.reg] = value;
		break;
	case MIPS_OP_IMM:
		ERROR("try to store a value to IMM operand");
		break;
	case MIPS_OP_MEM:
		ASSERT(len <= 4 && len > 0);
		ASSERT(operand.reg != MIPS_REG_ZERO);
		store(_regs[operand.mem.base] + operand.mem.disp, len, (const uint8_t *)&value);
		break;
	default:
		ERROR("Unkonw operand type:%d", operand.type);
	}

}

void Cpu::load(uint_t addr, size_t len, uint8_t* buf)
{
	memcpy(buf, (void *)addr, len);
}
void Cpu::store(uint_t addr, size_t len, const uint8_t* buf)
{
	memcpy((void *)addr, buf, len);
}

#define DEFINE(HANDLE) void Cpu:: HANDLE(const cs_mips_op* operands)

#define READU(i) readOperand(operands[i])
#define READ(i) ((int32_t)readOperand(operands[i]))
#define WRITE(i, v) writeOperand(operands[i], (uint_t)v)

#define READU_1(i) readOperand(operands[i], 1)
#define READU_2(i) readOperand(operands[i], 2)
#define READU_4(i) readOperand(operands[i], 4)

#define READ_1(i) ((int8_t)readOperand(operands[i], 1))
#define READ_2(i) ((int16_t)readOperand(operands[i], 2))
#define READ_4(i) ((int32_t)readOperand(operands[i], 4))

#define WRITE_1(i, v) writeOperand(operands[i], (uint_t)v, 1)
#define WRITE_2(i, v) writeOperand(operands[i], (uint_t)v, 2)
#define WRITE_4(i, v) writeOperand(operands[i], (uint_t)v, 4)

DEFINE(ADD){ WRITE(0, READ(1) + READ(2));_regs[MIPS_REG_PC] += 4;}
DEFINE(ADDU){ WRITE(0, READU(1) + READU(2));_regs[MIPS_REG_PC] += 4;}
DEFINE(SUB){ WRITE(0, READ(1) - READ(2));_regs[MIPS_REG_PC] += 4;}
DEFINE(SUBU){ WRITE(0, READU(1) - READU(2));_regs[MIPS_REG_PC] += 4;}
DEFINE(MULT)
{
	uint64_t v = (uint64_t)(((int64_t)READ(0)) * (int64_t)READ(1));
	_regs[MIPS_REG_LO] = (uint32_t)(v << 32 >> 32);
	_regs[MIPS_REG_HI] = (uint32_t)(v >> 32);
	_regs[MIPS_REG_PC] += 4;
}
DEFINE(MULTU)
{
	uint64_t v = (uint64_t)((uint64_t)READU(0)) * (uint64_t)READU(1);
	_regs[MIPS_REG_LO] = (uint32_t)(v << 32 >> 32);
	_regs[MIPS_REG_HI] = (uint32_t)(v >> 32);
	_regs[MIPS_REG_PC] += 4;
}
DEFINE(DIV)
{
	_regs[MIPS_REG_LO] = READ(0)/READ(1);
	_regs[MIPS_REG_HI] = READ(0)%READ(1);
	_regs[MIPS_REG_PC] += 4;
}
DEFINE(DIVU)
{
	_regs[MIPS_REG_LO] = READU(0)/READU(1);
	_regs[MIPS_REG_HI] = READU(0)%READU(1);
	_regs[MIPS_REG_PC] += 4;
}
DEFINE(LW){WRITE(0, READ(1)); _regs[MIPS_REG_PC] += 4; }
DEFINE(LH){WRITE(0, ((int32_t)READ_2(1))<<16>>16); _regs[MIPS_REG_PC] += 4;}
DEFINE(LHU){WRITE(0, READU_2(1)); _regs[MIPS_REG_PC] += 4;}
DEFINE(LB){WRITE(0, ((int32_t)READ_1(1))<<24>>24); _regs[MIPS_REG_PC] += 4;}
DEFINE(LBU){WRITE(0, READU_1(1)); _regs[MIPS_REG_PC] += 4;}
DEFINE(SW){WRITE(1, READ(0)); _regs[MIPS_REG_PC] += 4;}
DEFINE(SH){WRITE_2(1, READ(0)); _regs[MIPS_REG_PC] += 4;}
DEFINE(SB){WRITE_1(1, READ(0)); _regs[MIPS_REG_PC] += 4;}
DEFINE(LUI){WRITE(0, READ(1)<<16); _regs[MIPS_REG_PC] += 4;}
DEFINE(MFHI){WRITE(0, _regs[MIPS_REG_HI]); _regs[MIPS_REG_PC] += 4;}
DEFINE(MFLO){WRITE(0, _regs[MIPS_REG_LO]); _regs[MIPS_REG_PC] += 4;}

DEFINE(AND){WRITE(0, READU(1) & READU(2)); _regs[MIPS_REG_PC] += 4;}
DEFINE(OR){WRITE(0, READU(1) | READU(2)); _regs[MIPS_REG_PC] += 4;}
DEFINE(XOR){WRITE(0, READU(1) ^ READU(2)); _regs[MIPS_REG_PC] += 4;}
DEFINE(NOR){WRITE(0, ~(READU(1) | READU(2))); _regs[MIPS_REG_PC] += 4;}

DEFINE(SLT){WRITE(0, READ(1) < READ(2)); _regs[MIPS_REG_PC] += 4;}
DEFINE(SLTU){WRITE(0, READU(1) < READU(2)); _regs[MIPS_REG_PC] += 4;}

DEFINE(SLL){WRITE(0, READU(1) << READU(2)); _regs[MIPS_REG_PC] += 4;}
DEFINE(SRL){WRITE(0, READU(1) >> READU(2)); _regs[MIPS_REG_PC] += 4;}
DEFINE(SRA){WRITE(0, READ(1) >> READU(2)); _regs[MIPS_REG_PC] += 4;}


DEFINE(BEQ){if (READ(0) == READ(1)) _regs[MIPS_REG_PC] = READU(2); else _regs[MIPS_REG_PC] += 8;}
DEFINE(BNE){if (READ(0) != READ(1)) _regs[MIPS_REG_PC] = READU(2); else _regs[MIPS_REG_PC] += 8;}
DEFINE(BEQZ){if (READ(0) == 0) _regs[MIPS_REG_PC] = READU(1); else _regs[MIPS_REG_PC] += 8;}
DEFINE(BNEZ){if (READ(0) != 0) _regs[MIPS_REG_PC] = READU(1); else _regs[MIPS_REG_PC] += 8;}
DEFINE(BLEZ){if (READ(0) <= 0) _regs[MIPS_REG_PC] = READU(1); else _regs[MIPS_REG_PC] += 8;}
DEFINE(BLTZ){if (READ(0) < 0) _regs[MIPS_REG_PC] = READU(1); else _regs[MIPS_REG_PC] += 8;}
DEFINE(BGEZ){if (READ(0) >= 0) _regs[MIPS_REG_PC] = READU(1); else _regs[MIPS_REG_PC] += 8;}
DEFINE(BGTZ){if (READ(0) > 0) _regs[MIPS_REG_PC] = READU(1); else _regs[MIPS_REG_PC] += 8;}

DEFINE(J){_regs[MIPS_REG_PC] = READU(0);}
DEFINE(JAL){_regs[MIPS_REG_RA] = _regs[MIPS_REG_PC] + 8; _regs[MIPS_REG_PC] = READU(0); }

DEFINE(NOP){_regs[MIPS_REG_PC] += 4;}
DEFINE(MOVE){WRITE(0, READ(1));_regs[MIPS_REG_PC] += 4;}


#undef READ
#undef WRITE
#undef DEFINE
