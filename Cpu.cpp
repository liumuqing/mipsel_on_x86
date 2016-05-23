#include <stdint.h>
#include <string.h>
#include <capstone/capstone.h>
#include <capstone/mips.h>
#include "common.h"
#include "Cpu.h"

//(void (*)(Cpu& cpu, cs_mips_op * operands)) instruction_handles[INS_ENDING];
bool Cpu::instruction_handles_inited = false;
void * Cpu::instruction_handles[INS_ENDING];
#ifdef BIGENDIAN
Cpu::Cpu():_disassembler("mips")
#else
Cpu::Cpu():_disassembler("mipsel")
#endif
{
	if (!instruction_handles_inited)
	{
		instruction_handles_init();
	}
	for (int i = 0; i< MIPS_REG_ENDING+1; i++)
	{
		_regs[i] = 0;
	}
	//_icount = 0;
}
inline Instruction Cpu::fetch()
{
	return fetch(_regs[MIPS_REG_PC]);
}

Instruction Cpu::fetch(uint_t pc)
{
	return _disassembler.disassemble((const uint8_t*) pc);
}



void Cpu::load(uint_t addr, size_t len, uint8_t* buf)
{
	memcpy(buf, (void *)addr, len);
}
void Cpu::store(uint_t addr, size_t len, const uint8_t* buf)
{
	memcpy((void *)addr, buf, len);
}


#define DEFINE(NAME) __fastcall static inline void handle_##NAME (Cpu& cpu, const cs_mips_op* operands)
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

#include "InstructionDefineList.cc"
#undef READ
#undef WRITE


void Cpu::instruction_handles_init()
{
	for (int i = 0; i< INS_ENDING; i++)
		instruction_handles[i] = nullptr;
	instruction_handles_inited = true;
}

void Cpu::execute(const Instruction& inst)
{
	//_icount += 1;
	if (inst.type >= INS_ENDING || inst.type == INS_INVALID)
	{
		ERROR("no bind instruction type :%x", inst.type);
	}
	switch (inst.type)
	{
#define BIND(INS_NAME, HANDLE_NAME) case INS_NAME: handle_##HANDLE_NAME(*this, inst.operands); break;
		//the following included file is lines like BIND(MIPS_INS_NOP, NOP);
		#include "InstructionBindList.cc"
#undef BIND
		default: ERROR("havent bind InstructionType:0x%04x(%04d) %s", inst.type, inst.type, str_of_instruction(inst)); break;
	}
}
