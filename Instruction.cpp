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
			snprintf(t, 128, "0x%x", (uint_t)inst.operands[i].imm);
			break;
		case MIPS_OP_MEM:
			snprintf(t, 128, "[r%d+0x%x]", inst.operands[i].mem.base-1, (uint_t)inst.operands[i].mem.disp);
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
				"%32s "
#endif
				"type=0x%04x size=0x%04x op_count=%d",
#ifdef CONFIG_WITH_MNEMONIC
				temp,
#endif
				inst.type,
				inst.size,
				inst.op_count
			);
	return buffer_for_str_of_instruction;
}
