#include <stdint.h>
#include <string.h>
#include <capstone/capstone.h>
#include <capstone/mips.h>
#include "common.h"
#include "Cpu.h"

//(void (*)(Cpu& cpu, cs_mips_op * operands)) instruction_handles[INS_ENDING];
bool Cpu::instruction_handles_inited = false;
void * Cpu::instruction_handles[INS_ENDING];
Cpu::Cpu()
{
	if (!instruction_handles_inited)
	{
		instruction_handles_init();
	}
	for (int i = 0; i< MIPS_REG_ENDING+1; i++)
	{
		_regs[i] = 0;
	}
	_icount = 0;
}
inline Instruction Cpu::fetch()
{
	return fetch(_regs[MIPS_REG_PC]);
}

Instruction Cpu::fetch(uint_t pc)
{
	return _disassembler.disassemble((const uint8_t*) pc);
}
void Cpu::execute(const Instruction& inst)
{
	_icount += 1;
	if (inst.type >= INS_ENDING || inst.type == INS_INVALID)
	{
		ERROR("no bind instruction type :%x", inst.type);
	}
	if (!instruction_handles[inst.type]) ERROR("havent bind InstructionType:0x%04x(%04d) %s", inst.type, inst.type, str_of_instruction(inst));
	((void (*)(Cpu& cpu, const cs_mips_op *))(instruction_handles[inst.type]))(*this, inst.operands);
}



void Cpu::load(uint_t addr, size_t len, uint8_t* buf)
{
	memcpy(buf, (void *)addr, len);
}
void Cpu::store(uint_t addr, size_t len, const uint8_t* buf)
{
	memcpy((void *)addr, buf, len);
}


#define DEFINE(NAME) void handle_##NAME (Cpu& cpu, const cs_mips_op* operands)

	
#define READS(i) (cpu.readOperand<float>(operands[i]))
#define READD(i) (cpu.readOperand<double>(operands[i]))
#define READ(i) (cpu.readOperand<int32_t>(operands[i]))
#define READL(i) (cpu.readOperand<int64_t>(operands[i]))
#define READU(i) (cpu.readOperand<uint32_t>(operands[i]))
#define READUL(i) (cpu.readOperand<uint64_t>(operands[i]))

#define WRITES(i, v) ({auto t = v; cpu.writeOperand<float>(operands[i], reinterpret_cast<float&>(t));})
#define WRITED(i, v) ({auto t = v; cpu.writeOperand<double>(operands[i], reinterpret_cast<double&>(t));})
#define WRITE(i, v) ({auto t = v; cpu.writeOperand<int32_t>(operands[i], reinterpret_cast<int32_t>(t));})
#define WRITEL(i, v) ({auto t = v; cpu.writeOperand<uint64_t>(operands[i], reinterpret_cast<uint64_t>(t));})
#define WRITEU(i, v) ({auto t = v; cpu.writeOperand<uint32_t>(operands[i], reinterpret_cast<uint32_t>(t));})
#define WRITEUL(i, v) ({auto t = v; cpu.writeOperand<uint64_t>(operands[i], reinterpret_cast<uint64_t>(t));})
/*
#define WRITES(i, v) cpu.writeOperand<double>(operands[i], (v))
#define WRITED(i, v) cpu.writeOperand<double>(operands[i], (v))
#define WRITEU(i, v) cpu.writeOperand<uint32_t>(operands[i], (v))
#define WRITE(i, v) cpu.writeOperand<int32_t>(operands[i], (v))
#define WRITEL(i, v) cpu.writeOperand<uint64_t>(operands[i], (v))
*/

#define READ_1(i) (cpu.readOperand<int8_t>(operands[i]))
#define READ_2(i) (cpu.readOperand<int16_t>(operands[i]))
#define READ_4(i) (cpu.readOperand<int32_t>(operands[i]))
#define READU_1(i) cpu.readOperand<uint8_t>(operands[i])
#define READU_2(i) cpu.readOperand<uint16_t>(operands[i])
#define READU_4(i) cpu.readOperand<uint32_t>(operands[i])


#define WRITE_1(i, v) ({auto t = v; cpu.writeOperand<int8_t>(operands[i], reinterpret_cast<int8_t>(t));})
#define WRITE_2(i, v) ({auto t = v; cpu.writeOperand<int16_t>(operands[i], reinterpret_cast<int16_t>(t));})
#define WRITE_4(i, v) ({auto t = v; cpu.writeOperand<int32_t>(operands[i], reinterpret_cast<int32_t>(t));})
#define WRITEU_1(i, v) ({auto t = v; cpu.writeOperand<uint8_t>(operands[i], reinterpret_cast<uint8_t>(t));})
#define WRITEU_2(i, v) ({auto t = v; cpu.writeOperand<uint16_t>(operands[i], reinterpret_cast<uint16_t>(t));})
#define WRITEU_4(i, v) ({auto t = v; cpu.writeOperand<uint32_t>(operands[i], reinterpret_cast<uint32_t>(t));})
/*
#define WRITE_1(i, v) cpu.writeOperand<int8_t>(operands[i], (v))
#define WRITE_2(i, v) cpu.writeOperand<int16_t>(operands[i], (v))
#define WRITE_4(i, v) cpu.writeOperand<int32_t>(operands[i], (v))
*/

DEFINE(ADD){ WRITE(0, READ(1) + READ(2));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(ADDU){ WRITEU(0, READU(1) + READU(2));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SUB){ WRITE(0, READ(1) - READ(2));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SUBU){ WRITEU(0, READU(1) - READU(2));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(MULT)
{
	uint64_t v = (uint64_t)(((int64_t)READ(0)) * (int64_t)READ(1));
	cpu._regs[MIPS_REG_LO] = (uint32_t)(v << 32 >> 32);
	cpu._regs[MIPS_REG_HI] = (uint32_t)(v >> 32);
	cpu._regs[MIPS_REG_PC] += 4;
}
DEFINE(MULTU)
{
	uint64_t v = (uint64_t)((uint64_t)READU(0)) * (uint64_t)READU(1);
	cpu._regs[MIPS_REG_LO] = (uint32_t)(v << 32 >> 32);
	cpu._regs[MIPS_REG_HI] = (uint32_t)(v >> 32);
	cpu._regs[MIPS_REG_PC] += 4;
}
DEFINE(DIV)
{
	cpu._regs[MIPS_REG_LO] = READ(0)/READ(1);
	cpu._regs[MIPS_REG_HI] = READ(0)%READ(1);
	cpu._regs[MIPS_REG_PC] += 4;
}
DEFINE(DIVU)
{
	cpu._regs[MIPS_REG_LO] = READU(0)/READU(1);
	cpu._regs[MIPS_REG_HI] = READU(0)%READU(1);
	cpu._regs[MIPS_REG_PC] += 4;
}
DEFINE(LW){WRITE(0, READ(1)); cpu._regs[MIPS_REG_PC] += 4; }
DEFINE(LH){WRITE(0, ((int32_t)READ_2(1))<<16>>16); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(LHU){WRITEU(0, (uint32_t)READU_2(1)&0xffff); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(LB){WRITE(0, ((int32_t)READ_1(1))<<24>>24); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(LBU){WRITEU(0, (uint32_t)READU_1(1)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SW){WRITE(1, READ(0)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SH){WRITE_2(1, READ_2(0)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SB){WRITE_1(1, READ_1(0)); cpu._regs[MIPS_REG_PC] += 4;}
//unaligned load/store
DEFINE(LWL){WRITE(0, READ(1));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(LWR){WRITE(0, READ(1));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SWL){WRITE(1, READ(0));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SWR){WRITE(1, READ(0));cpu._regs[MIPS_REG_PC] += 4;}

DEFINE(LUI){WRITE(0, READ(1)<<16); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(MFHI){WRITEU(0, cpu._regs[MIPS_REG_HI]); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(MFLO){WRITEU(0, cpu._regs[MIPS_REG_LO]); cpu._regs[MIPS_REG_PC] += 4;}

DEFINE(AND){WRITEU(0, READU(1) & READU(2)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(OR){WRITEU(0, READU(1) | READU(2)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(XOR){WRITEU(0, READU(1) ^ READU(2)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(NOR){WRITEU(0, ~(READU(1) | READU(2))); cpu._regs[MIPS_REG_PC] += 4;}

DEFINE(SLT){WRITEU(0, (uint32_t)(READ(1) < READ(2))); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SLTU){WRITEU(0, (uint32_t)(READU(1) < READU(2))); cpu._regs[MIPS_REG_PC] += 4;}

DEFINE(SLL){WRITEU(0, READU(1) << READU(2)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SRL){WRITEU(0, READU(1) >> READU(2)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SRA){WRITE(0, READ(1) >> READU(2)); cpu._regs[MIPS_REG_PC] += 4;}


DEFINE(BEQ){if (READ(0) == READ(1)) cpu._regs[MIPS_REG_PC] = READU(2); else cpu._regs[MIPS_REG_PC] += 8;}
DEFINE(BNE){if (READ(0) != READ(1)) cpu._regs[MIPS_REG_PC] = READU(2); else cpu._regs[MIPS_REG_PC] += 8;}
DEFINE(BEQZ){if (READ(0) == 0) cpu._regs[MIPS_REG_PC] = READU(1); else cpu._regs[MIPS_REG_PC] += 8;}
DEFINE(BNEZ){if (READ(0) != 0) cpu._regs[MIPS_REG_PC] = READU(1); else cpu._regs[MIPS_REG_PC] += 8;}
DEFINE(BLEZ){if (READ(0) <= 0) cpu._regs[MIPS_REG_PC] = READU(1); else cpu._regs[MIPS_REG_PC] += 8;}
DEFINE(BLTZ){if (READ(0) < 0) cpu._regs[MIPS_REG_PC] = READU(1); else cpu._regs[MIPS_REG_PC] += 8;}
DEFINE(BGEZ){if (READ(0) >= 0) cpu._regs[MIPS_REG_PC] = READU(1); else cpu._regs[MIPS_REG_PC] += 8;}
DEFINE(BGTZ){if (READ(0) > 0) cpu._regs[MIPS_REG_PC] = READU(1); else cpu._regs[MIPS_REG_PC] += 8;}

DEFINE(BC1T){if (cpu.cond1) cpu._regs[MIPS_REG_PC] = READU(0); else cpu._regs[MIPS_REG_PC] += 8;}
DEFINE(BC1F){if (!cpu.cond1) cpu._regs[MIPS_REG_PC] = READU(0); else cpu._regs[MIPS_REG_PC] += 8;}

DEFINE(J){cpu._regs[MIPS_REG_PC] = READU(0);}
DEFINE(JAL){cpu._regs[MIPS_REG_RA] = cpu._regs[MIPS_REG_PC] + 8; cpu._regs[MIPS_REG_PC] = READU(0); }

DEFINE(NOP){cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(MOVE){WRITE(0, READ(1));cpu._regs[MIPS_REG_PC] += 4;}

DEFINE(MTC1){WRITES(0, READS(1));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(LWC1){WRITES(0, READS(1));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(LDC1){WRITED(0, READD(1));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SWC1){WRITES(1, READS(0));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SDC1){WRITED(1, READD(0));cpu._regs[MIPS_REG_PC] += 4;}



DEFINE(CVT_D_W){WRITED(0, (double)READ(1));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(CVT_D_S){WRITED(0, (double)READS(1));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(CVT_S_D){WRITES(0, (float)READD(1));cpu._regs[MIPS_REG_PC] += 4;}


DEFINE(DIV_S){WRITES(0, (float)(READD(1)/READS(2)));cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(DIV_D){WRITED(0, (double)(READD(1)/READD(2)));cpu._regs[MIPS_REG_PC] += 4;}

DEFINE(C_LE_S){cpu.cond1 = (READD(0)<=READD(1));cpu._regs[MIPS_REG_PC] += 4;}

#undef READ
#undef WRITE







void Cpu::instruction_handles_init()
{
	for (int i = 0; i< INS_ENDING; i++)
		instruction_handles[i] = nullptr;

#define BIND(INS_NAME, HANDLE_NAME) ({instruction_handles[INS_NAME] = (void *)handle_##HANDLE_NAME;})
	BIND(INS_NOP, NOP);

	BIND(INS_MOVE, MOVE);
	BIND(INS_ADD, ADD);	
	BIND(INS_ADDU, ADDU);
	BIND(INS_ADDIU, ADDU);	
	BIND(INS_SUB, SUB);
	BIND(INS_SUBU, SUBU);
	BIND(INS_SUBIU, SUBU);
	BIND(INS_MULT, MULT);
	BIND(INS_MULTU, MULTU);
	BIND(INS_DIV, DIV);
	BIND(INS_DIVU, DIVU);

	BIND(INS_LW, LW);
	BIND(INS_LH, LH);
	BIND(INS_LHU, LHU);
	BIND(INS_LB, LB);
	BIND(INS_LBU, LBU);
	BIND(INS_SW, SW);
	BIND(INS_SH, SH);
	BIND(INS_SB, SB);

	BIND(INS_LWL, LWL);
	BIND(INS_LWR, LWR);
	BIND(INS_SWL, SWL);
	BIND(INS_SWR, SWR);

	BIND(INS_LUI, LUI);
	BIND(INS_MFHI, MFHI);
	BIND(INS_MFLO, MFLO);

	BIND(INS_AND, AND);
	BIND(INS_ANDI, AND);
	BIND(INS_OR, OR);
	BIND(INS_ORI, OR);
	BIND(INS_XOR, XOR);
	BIND(INS_XORI, XOR);
	BIND(INS_NOR, NOR);
	BIND(INS_NORI, NOR);
	BIND(INS_SLT, SLT);
	BIND(INS_SLTI, SLT);
	BIND(INS_SLTU, SLTU);
	BIND(INS_SLTIU, SLTU);

	BIND(INS_SLL, SLL);
	BIND(INS_SLLV, SLL);
	BIND(INS_SRL, SRL);
	BIND(INS_SRLV, SRL);
	BIND(INS_SRA, SRA);
	BIND(INS_SRAV, SRA);

	BIND(INS_BEQ, BEQ);
	BIND(INS_BNE, BNE);
	BIND(INS_BEQZ, BEQZ);
	BIND(INS_BNEZ, BNEZ);
	BIND(INS_BLEZ, BLEZ);
	BIND(INS_BLTZ, BLTZ);
	BIND(INS_BGEZ, BGEZ);
	BIND(INS_BGTZ, BGTZ);
	BIND(INS_B, J);
	BIND(INS_BAL, JAL);

	BIND(INS_BC1T, BC1T);
	BIND(INS_BC1F, BC1F);

	BIND(INS_J, J);
	BIND(INS_JR, J);
	BIND(INS_JALR, JAL);
	BIND(INS_JAL, JAL);



	BIND(INS_MTC1, MTC1);
	BIND(INS_LWC1, LWC1);
	BIND(INS_LDC1, LDC1);
	BIND(INS_SWC1, SWC1);
	BIND(INS_SDC1, SDC1);


	BIND(INS_CVT_D_W, CVT_D_W);
	BIND(INS_CVT_D_S, CVT_D_S);
	BIND(INS_CVT_S_D, CVT_S_D);


	BIND(INS_DIV_S, DIV_S);
	BIND(INS_DIV_D, DIV_D);

	BIND(INS_C_LE_S, C_LE_S);




#undef BIND
	instruction_handles_inited = true;
}
