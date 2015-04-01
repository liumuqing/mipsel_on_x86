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
	retv.type = (mips_insn)all_insn -> id;
	retv.op_count = all_insn->detail->mips.op_count;
	retv.size = (uint_t) all_insn->size;
#ifdef CONFIG_WITH_MNEMONIC
	memcpy(retv.mnemonic ,  all_insn->mnemonic, MNEMONIC_MAX_LEN);
	retv.addr = (uint_t) code_buf + offset;
	retv.buf = code_buf;
#endif
	memcpy(retv.operands, all_insn->detail->mips.operands, retv.op_count * sizeof(retv.operands[0]));
	return retv;
}
MipsDisassembler::~MipsDisassembler()
{
	cs_close(&handle);
}
