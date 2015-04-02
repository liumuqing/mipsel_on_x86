#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <capstone/capstone.h>

#include "common.h"
#include "Disassembler.h"
#include "Instruction.h"
MipsDisassembler::MipsDisassembler(const char * type)
{
	cs_err err_code;
	if (strcmp(type, "mipsel") == 0) err_code  = cs_open(CS_ARCH_MIPS, (cs_mode)(CS_MODE_MIPS32 + CS_MODE_LITTLE_ENDIAN), &handle);
	else if (strcmp(type, "mips") == 0) err_code  = cs_open(CS_ARCH_MIPS, (cs_mode)(CS_MODE_MIPS32 + CS_MODE_BIG_ENDIAN), &handle);
	else
		ERROR("Unrecongized Architecture %s", type);

	if (err_code !=  CS_ERR_OK)
		ERROR("Disassembler initialize fails: %s", cs_strerror(err_code));


	err_code = cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
	if (err_code !=  CS_ERR_OK)
		ERROR("Detail Mode of Capstion fails: %s", cs_strerror(err_code));

	INFO("Disassembler initalize success!");
}
/*
 * disassemble the bytes from code_buf, the address assemes is code_buf..., if not use offset
 */
Instruction MipsDisassembler::disassemble(const uint8_t * code_buf, uint_t offset)
{
	cs_insn *all_insn;
	size_t count = cs_disasm(handle, code_buf, 8, (uint64_t)(code_buf) + offset, 1, &all_insn);
	//							handle, code_buf, code_size, address, count, cs_insn*

	if (count == 0) ERROR("Can't disassemble at %p", code_buf);

	Instruction retv;
	retv.type = INS_INVALID;
	retv.op_count = all_insn->detail->mips.op_count;
	retv.size = (uint_t) all_insn->size;
	memcpy(retv.operands, all_insn->detail->mips.operands, retv.op_count * sizeof(retv.operands[0]));
#ifdef CONFIG_WITH_MNEMONIC
	strncpy(retv.mnemonic ,  all_insn->mnemonic, MNEMONIC_MAX_LEN);
#endif
#define BIND_MNEMONIC_TO_INSTRUCTION_TYPE(mn, new_t)\
	else if (0 == strcmp(retv.mnemonic, mn)) ({retv.type = new_t;})
	if (false);

	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("nop", INS_NOP);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("move", INS_MOVE);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("add", INS_ADD);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("addi", INS_ADDI);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("addu", INS_ADDU);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("addiu", INS_ADDIU);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("sub", INS_SUB);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("subi", INS_SUBI);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("subu", INS_SUBU);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("subiu", INS_SUBIU);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("mult", INS_MULT);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("multu", INS_MULTU);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("div", INS_DIV);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("divu", INS_DIVU);

	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("lw", INS_LW);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("lh", INS_LH);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("lhu", INS_LHU);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("lb", INS_LB);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("lbu", INS_LBU);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("sw", INS_SW);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("sh", INS_SH);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("sb", INS_SB);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("lui", INS_LUI);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("mfhi", INS_MFHI);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("mflo", INS_MFLO);

	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("and", INS_AND);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("andi", INS_ANDI);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("or", INS_OR);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("ori", INS_ORI);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("xor", INS_XOR);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("xori", INS_XORI);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("nor", INS_NOR);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("nori", INS_NORI);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("slt", INS_SLT);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("slti", INS_SLTI);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("sltu", INS_SLTU);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("sltiu", INS_SLTIU);

	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("sll", INS_SLL);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("sllv", INS_SLLV);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("srl", INS_SRL);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("srlv", INS_SRLV);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("sra", INS_SRA);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("srav", INS_SRAV);

	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("beq", INS_BEQ);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("bne", INS_BNE);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("beqz", INS_BEQZ);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("bnez", INS_BNEZ);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("blez", INS_BLEZ);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("bltz", INS_BLTZ);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("bgez", INS_BGEZ);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("bgtz", INS_BGTZ);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("b", INS_B);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("bal", INS_BAL);

	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("j", INS_J);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("jr", INS_JR);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("jalr", INS_JALR);
	BIND_MNEMONIC_TO_INSTRUCTION_TYPE("jal", INS_JAL);





	else
	{
		ERROR("[Disassembler] can not dissemble (%s) mips_insn:0x%04x(%04d) %s", all_insn->mnemonic, all_insn->id, all_insn->id, str_of_instruction(retv));
	}





#undef BIND_MNEMONIC_TO_INSTRUCTION_TYPE
	
	cs_free(all_insn, count);
	return retv;
}
MipsDisassembler::~MipsDisassembler()
{
	cs_close(&handle);
}
