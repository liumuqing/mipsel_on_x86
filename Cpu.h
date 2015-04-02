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

	void load(uint_t addr, size_t len, uint8_t *buf);
	void store(uint_t addr, size_t len, const uint8_t *buf);

	uint_t _regs[MIPS_REG_ENDING+1];
	uint_t _icount;

	//as all readOperand and writeOperand are used in on cpp file(i.e Cpu file, we can use a template methed decalare here, other wise we have to define the function)
	template <typename T> T readOperand(const cs_mips_op&);
	template <typename T> void writeOperand(const cs_mips_op&, T value);
	
private:
	static void *instruction_handles[INS_ENDING];
	static bool instruction_handles_inited;
	static void instruction_handles_init();


	std::map<uint_t, Instruction *> _inst_cache;
	MipsDisassembler _disassembler;

	Cpu(const Cpu& cpu);
};

