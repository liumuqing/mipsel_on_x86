
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

DEFINE(NEGU){WRITEU(0, 0-READU(1)); cpu._regs[MIPS_REG_PC] +=4;}
DEFINE(NEG){WRITE(0, 0-READ(1)); cpu._regs[MIPS_REG_PC] +=4;}

DEFINE(LW){WRITE(0, READ(1)); cpu._regs[MIPS_REG_PC] += 4; }
DEFINE(LH){WRITE(0, ((int32_t)READ_2(1))<<16>>16); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(LHU){WRITEU(0, (uint32_t)READU_2(1)&0xffff); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(LB){WRITE(0, ((int32_t)READ_1(1))<<24>>24); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(LBU){WRITEU(0, (uint32_t)READU_1(1)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SW){WRITE(1, READ(0)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SH){WRITE_2(1, READ_2(0)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(SB){WRITE_1(1, READ_1(0)); cpu._regs[MIPS_REG_PC] += 4;}
//FIXME:unaligned load/store
//NOTE: although mostly, LWL and LWR are used together to load a word on a unaligned address i.e `lwl $1, (1)$0; lwr $1 (1)$0`
//the two instruction so the following definitions are  wrong...
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

DEFINE(TEQ){if (READ(0) == READ(1)) ERROR("trap %d", READ(2)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(TGE){if (READ(0) >= READ(1)) ERROR("trap %d", READ(2)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(TGEU){if (READU(0) >= READU(1)) ERROR("trap %d", READ(2)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(TLT){if (READ(0) < READ(1)) ERROR("trap %d", READ(2)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(TLTU){if (READU(0) <= READU(1)) ERROR("trap %d", READ(2)); cpu._regs[MIPS_REG_PC] += 4;}
DEFINE(TNE){if (READ(0) != READ(1)) ERROR("trap %d", READ(2)); cpu._regs[MIPS_REG_PC] += 4;}
