#pragma once
#include <stdint.h>
#include "Instruction.h"
#include "common.h"
class MipsDisassembler
{
public:
	MipsDisassembler(const char * type="mipsel");
	Instruction disassemble(const uint8_t * code_buf, uint_t offset=0);
private:
	csh handle;

public:
	Instruction disassemble(const char * code_buf, uint_t offset=0){return disassemble((const uint8_t*) code_buf, offset);}
	~MipsDisassembler();
private:
	MipsDisassembler(const MipsDisassembler&){};
};

