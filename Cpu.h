#pragma once
/* NOTE: never modifiy any instance variable after initialized...
 * as we need support multithread..
 * */
#include <map>
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

	static inline void load(uint_t addr, size_t len, uint8_t *buf);
	static inline void store(uint_t addr, size_t len, const uint8_t *buf);

	uint_t _regs[MIPS_REG_ENDING+1];
	//uint_t _icount;

	//as all readOperand and writeOperand are used in on cpp file(i.e Cpu file, we can use a template methed decalare here, other wise we have to define the function)
	template <typename T> T readOperand(const cs_mips_op&);
	template <typename T> void writeOperand(const cs_mips_op&, T value);

	bool cond1;
	
private:
	static void *instruction_handles[INS_ENDING];
	static bool instruction_handles_inited;
	static void instruction_handles_init();

	std::map<uint_t, Instruction *> _inst_cache;
	MipsDisassembler _disassembler;

	Cpu(const Cpu& cpu);
};

template <typename T>
inline void Cpu::writeOperand(const cs_mips_op& operand, T value)
{
	switch (operand.type)
	{
		case MIPS_OP_REG:
			*(T*)&_regs[operand.reg] = value;
			break;
		case MIPS_OP_IMM:
			ERROR("try to store a value to IMM operand");
			break;
		case MIPS_OP_MEM:
			{
				T vv = change_endian<T>(value);
				//store(_regs[operand.mem.base] + operand.mem.disp, sizeof(T), (const uint8_t *)&vv);
				*(T*)(_regs[operand.mem.base] + operand.mem.disp) = vv;
				break;
			}
		default:
			ERROR("Unkonw operand type:%d", operand.type);
	}
}
template <typename T>
inline T Cpu::readOperand(const cs_mips_op& operand)
{
	switch (operand.type)
	{
	case MIPS_OP_REG:
		return *(T*)&_regs[operand.reg];
	case MIPS_OP_IMM:
		return operand.imm;
	case MIPS_OP_MEM:
		{
			T retv = T();//retv must be initialize as zero
			ASSERT(sizeof(T)>0);
			//load(_regs[operand.mem.base] + (uint_t)operand.mem.disp, sizeof(T), (uint8_t *)&retv);
			retv = *(T*)(_regs[operand.mem.base] + operand.mem.disp);
			return change_endian<T>(retv);
		}
	default:
		ERROR("Unkonw operand type:%d", operand.type);
	}
}
