#include <string.h>
#include <capstone/capstone.h>

#include "common.h"
#include "Instruction.h"

char buffer_for_str_of_instruction[128];
const char * str_of_instruction(const Instruction& inst)
{
#ifdef CONFIG_WITH_MNEMONIC
	char temp[128];
	snprintf(temp, 128, "%s ", inst.mnemonic);
	for (int i = 0; i < inst.op_count; i++)
	{
		char t[128];
		switch (inst.operands[i].type)
		{
		case MIPS_OP_REG:
			snprintf(t, 128, "r%d", inst.operands[i].reg-1);
			break;
		case MIPS_OP_IMM:
			snprintf(t, 128, "0x%lx", (uint_t)inst.operands[i].imm);
			break;
		case MIPS_OP_MEM:
			snprintf(t, 128, "[r%d+0x%lx]", inst.operands[i].mem.base-1, (uint_t)inst.operands[i].mem.disp);
			break;
		default:
			ERROR("str_of_instruction unkonwn operand type %d", inst.operands[i].type);
			break;
		}
		if (i != 0) strcat(temp, ", ");
		strcat(temp, t);
	}
#endif
	snprintf(	buffer_for_str_of_instruction,
				128,
#ifdef CONFIG_WITH_MNEMONIC
				"%02x%02x%02x%02x %32s "
#endif
				"type=0x%04x size=0x%04x op_count=%d",
#ifdef CONFIG_WITH_MNEMONIC
				inst.buf[0], inst.buf[1], inst.buf[2], inst.buf[3], 
				temp,
#endif
				inst.type,
				inst.size,
				inst.op_count
			);
	return buffer_for_str_of_instruction;
}
