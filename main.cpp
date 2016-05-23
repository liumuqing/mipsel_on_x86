#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <vector>
#include <unordered_map>

#include "Instruction.h"
#include "Cpu.h"
#include "ElfLoader.h"

ElfLoader loader;
std::unordered_map<uint_t, std::string> got_trans_map;
Cpu cpu;
uint_t host_stack_highest;
Instruction ** inst_cache;
size_t inst_cache_base;
size_t inst_cache_size;

uint_t fake__stack_stack_guard = 0x12345678;

uint_t create_fake_stack(int argc, char ** argv, char **env, uint_t stack_size = 0x10000)
{
	ASSERT(stack_size % 0x1000 == 0);
	uint_t stack = (uint_t)mmap((void *)0x22220000, stack_size ,PROT_WRITE|PROT_READ, MAP_FIXED|MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	if (stack != 0x22220000) ERROR("mmap error in create_fake_stack");
	INFO("[STACK] fake_stack_address: %p ~ %p", stack + stack_size-1, stack);
	stack += stack_size;
/*
        #http://www.phrack.org/issues.html?issue=58&id=5#article
        # position            content                     size (bytes) + comment
        # ----------------------------------------------------------------------
        # stack pointer ->  [ argc = number of args ]     4
        #                 [ argv[0] (pointer) ]         4   (program name)
        #                 [ argv[1] (pointer) ]         4
        #                 [ argv[..] (pointer) ]        4 * x
        #                 [ argv[n - 1] (pointer) ]     4
        #                 [ argv[n] (pointer) ]         4   (= NULL)
        #
        #                 [ envp[0] (pointer) ]         4
        #                 [ envp[1] (pointer) ]         4
        #                 [ envp[..] (pointer) ]        4
        #                 [ envp[term] (pointer) ]      4   (= NULL)
        #
        #                 [ auxv[0] (Elf32_auxv_t) ]    8
        #                 [ auxv[1] (Elf32_auxv_t) ]    8
        #                 [ auxv[..] (Elf32_auxv_t) ]   8
        #                 [ auxv[term] (Elf32_auxv_t) ] 8   (= AT_NULL vector)
        #
        #                 [ padding ]                   0 - 16
        #
        #                 [ argument ASCIIZ strings ]   >= 0
        #                 [ environment ASCIIZ str. ]   >= 0
        #
        # (0xbffffffc)      [ end marker ]                4   (= NULL)
        #
        # (0xc0000000)      < top of stack >              0   (virtual)
        # ----------------------------------------------------------------------
*/
	// here we just put our(the simulator's) argv, env, auxv into the stack of simulated mipsel process;
	// I ignore auxv....
	size_t count_argv = 0;
	size_t count_env = 0;
	for (;argv[count_argv]; count_argv++);
	for (;env[count_env]; count_env++);


	stack -= 4;
	*(uint_t *) stack = 0xbffffffc;
	stack -= 12;

	*(uint_t *) (stack-=4) = (uint_t) nullptr;
	for (;count_env--;)
		*(uint_t *) (stack-=4) = (uint_t) env[count_env];
	*(uint_t *) (stack-=4) = (uint_t) nullptr;
	for (;count_argv--;)
		*(uint_t *) (stack-=4) = (uint_t) argv[count_argv];
	*(uint_t *) (stack-=4) = (uint_t) argc;
	return stack;
}
/* execute until goto x86 area */
void simulation_loop(Cpu& cpu)
{
	while (true)
	{
		int i;
		/*
		for (i =0; i < loader._MAPs.size(); i++)
		{
			if (cpu._regs[MIPS_REG_PC] >= loader._MAPs[i].addr && cpu._regs[MIPS_REG_PC] < loader._MAPs[i].size  + loader._MAPs[i].addr)
				 break;
		}
		if (i == loader._MAPs.size()) return;
		*/
		if (loader.pageTable[cpu._regs[MIPS_REG_PC] >> COMMON_PAGE_BIT_LEN] == 0) return;
		//TODO check whethet PC is excutable
		ASSERT(cpu._regs[MIPS_REG_PC] % 4 == 0);
		if (!inst_cache[(cpu._regs[MIPS_REG_PC]-inst_cache_base)/4])
			inst_cache[(cpu._regs[MIPS_REG_PC]-inst_cache_base)/4] = new Instruction(cpu.fetch(cpu._regs[MIPS_REG_PC]));
		const Instruction& inst = *inst_cache[(cpu._regs[MIPS_REG_PC]-inst_cache_base)/4];
		if (inst.type >= INS_BEQ && inst.type<=INS_JAL)
		{
			uint_t old_PC = cpu._regs[MIPS_REG_PC];
			INFO("%08x %s", cpu._regs[MIPS_REG_PC], str_of_instruction(inst));
			cpu.execute(inst);
			uint_t new_PC = cpu._regs[MIPS_REG_PC];
			//if (inst.op_count) INFO("first operand:%08x", cpu.readOperand<uint32_t>(inst.operands[0]));
			if (false)
			{
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[1], cpu._regs[2], cpu._regs[3], cpu._regs[4],
						cpu._regs[5], cpu._regs[6], cpu._regs[7], cpu._regs[8]);
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[9], cpu._regs[10], cpu._regs[11], cpu._regs[12],
						cpu._regs[13], cpu._regs[14], cpu._regs[15], cpu._regs[16]);
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[17], cpu._regs[18], cpu._regs[19], cpu._regs[20],
						cpu._regs[21], cpu._regs[22], cpu._regs[23], cpu._regs[24]);
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[25], cpu._regs[26], cpu._regs[27], cpu._regs[28],
						cpu._regs[29], cpu._regs[30], cpu._regs[31], cpu._regs[32]);
			}
			ASSERT((old_PC+4)% 4 == 0);
			if (!inst_cache[(old_PC + 4 - inst_cache_base)/4])
				inst_cache[(old_PC + 4 - inst_cache_base)/4] = new Instruction(cpu.fetch(old_PC + 4));
			const Instruction& nextinst = *inst_cache[(old_PC + 4 - inst_cache_base)/4];
			cpu._regs[MIPS_REG_PC] = old_PC + 4;
			INFO("%08x %s", cpu._regs[MIPS_REG_PC], str_of_instruction(nextinst));

			cpu.execute(nextinst);
			//if (nextinst.op_count) INFO("first operand:%08x", cpu.readOperand<uint32_t>(nextinst.operands[0]));

			if (false)
			{
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[1], cpu._regs[2], cpu._regs[3], cpu._regs[4],
						cpu._regs[5], cpu._regs[6], cpu._regs[7], cpu._regs[8]);
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[9], cpu._regs[10], cpu._regs[11], cpu._regs[12],
						cpu._regs[13], cpu._regs[14], cpu._regs[15], cpu._regs[16]);
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[17], cpu._regs[18], cpu._regs[19], cpu._regs[20],
						cpu._regs[21], cpu._regs[22], cpu._regs[23], cpu._regs[24]);
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[25], cpu._regs[26], cpu._regs[27], cpu._regs[28],
						cpu._regs[29], cpu._regs[30], cpu._regs[31], cpu._regs[32]);
			}

			cpu._regs[MIPS_REG_PC] = new_PC;

		}
		else
		{
			INFO("%08x %s", cpu._regs[MIPS_REG_PC], str_of_instruction(inst));
			cpu.execute(inst);
			//if (inst.op_count) INFO("first operand:%08x", cpu.readOperand<uint32_t>(inst.operands[0]));
			if (false)
			{
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[1], cpu._regs[2], cpu._regs[3], cpu._regs[4],
						cpu._regs[5], cpu._regs[6], cpu._regs[7], cpu._regs[8]);
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[9], cpu._regs[10], cpu._regs[11], cpu._regs[12],
						cpu._regs[13], cpu._regs[14], cpu._regs[15], cpu._regs[16]);
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[17], cpu._regs[18], cpu._regs[19], cpu._regs[20],
						cpu._regs[21], cpu._regs[22], cpu._regs[23], cpu._regs[24]);
				INFO("%08x %08x %08x %08x %08x %08x %08x %08x", 
						cpu._regs[25], cpu._regs[26], cpu._regs[27], cpu._regs[28],
						cpu._regs[29], cpu._regs[30], cpu._regs[31], cpu._regs[32]);
			}
		}

	}
}
void start_simulate()
{
	while (true)
	{
		simulation_loop(cpu);
		if (got_trans_map.find(cpu._regs[MIPS_REG_PC]) != got_trans_map.end())
		{
			if (got_trans_map[cpu._regs[MIPS_REG_PC]] == "exit")
			{
				INFO("HOOKED exit");
				//INFO("total execute: 0x%x %d\n", cpu._icount, cpu._icount);
				_exit(cpu._regs[MIPS_REG_A0]);
			}
			INFO("prepare to go into x86 library to addr 0x%08x", cpu._regs[MIPS_REG_PC]);
			uint_t _func = cpu._regs[MIPS_REG_PC];
			INFO("library target address: %p %s", _func, got_trans_map[_func].c_str());
			if (!_func) ERROR("try to call a got func which has not been loaded");
			uint_t (*func)(uint_t, uint_t, uint_t, uint_t, uint_t, uint_t, uint_t, uint_t, uint_t, uint_t, uint_t) = 
				(uint_t (*)(uint_t, uint_t, uint_t, uint_t, uint_t, uint_t, uint_t, uint_t, uint_t, uint_t, uint_t))_func;

			uint_t retaddr = cpu._regs[MIPS_REG_RA];

			if (got_trans_map[cpu._regs[MIPS_REG_PC]] == "open64")
			{
				uint_t temp = cpu._regs[MIPS_REG_A1];
				uint_t a1 = 0;
				a1 = temp & 0x3;
				if (temp & 0x0008) a1 |= O_APPEND;
				if (temp & 0x0010) a1 |= O_SYNC;
				if (temp & 0x0080) a1 |= O_NONBLOCK;
				if (temp & 0x0100) a1 |= O_CREAT;
				if (temp & 0x0200) a1 |= O_TRUNC;
				if (temp & 0x0400) a1 |= O_EXCL;
				if (temp & 0x0800) a1 |= O_NOCTTY;
				cpu._regs[MIPS_REG_A1] = a1;

			}
			uint_t retv = func(
					cpu._regs[MIPS_REG_A0],
					cpu._regs[MIPS_REG_A1],
					cpu._regs[MIPS_REG_A2],
					cpu._regs[MIPS_REG_A3],
					*(uint_t *)(cpu._regs[MIPS_REG_SP] + 16),
					*(uint_t *)(cpu._regs[MIPS_REG_SP] + 20),
					*(uint_t *)(cpu._regs[MIPS_REG_SP] + 24),
					*(uint_t *)(cpu._regs[MIPS_REG_SP] + 28),
					*(uint_t *)(cpu._regs[MIPS_REG_SP] + 32),
					*(uint_t *)(cpu._regs[MIPS_REG_SP] + 36),
					*(uint_t *)(cpu._regs[MIPS_REG_SP] + 40)
					);
			/*
			if (got_trans_map[cpu._regs[MIPS_REG_PC]] == "__fxstat64" || got_trans_map[cpu._regs[MIPS_REG_PC]] == "__lxstat64"|| got_trans_map[cpu._regs[MIPS_REG_PC]] == "__xstat64" )
			{
				struct stat64 s64 = *(struct stat64 *)cpu._regs[MIPS_REG_A2];
				memcpy((char*)cpu._regs[MIPS_REG_A2]+16, &s64.st_ino, 8);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+24, &s64.st_mode, 4);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+28, &s64.st_nlink, 4);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+32, &s64.st_uid, 4);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+36, &s64.st_gid, 4);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+40, &s64.st_rdev, 4);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+56, &s64.st_size, 8);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+88, &s64.st_blksize, 4);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+96, &s64.st_blocks, 8);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+64, &s64.st_atim, 8);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+72, &s64.st_mtim, 8);
				memcpy((char*)cpu._regs[MIPS_REG_A2]+80, &s64.st_ctim, 8);
			}
			*/
			cpu._regs[MIPS_REG_PC] = retaddr;
			cpu._regs[MIPS_REG_V0] = retv;
			continue;
		}
		else
		{
			break;
		}
	}
}

void handler(int sig, siginfo_t * siginfo, void * ucontext)
{
#undef REG
#define REG(x) ((uint_t) (((ucontext_t *) ucontext)->uc_mcontext.gregs[REG_##x]))
	int i;
	INFO("callback from library, target address: %p", REG(EIP));
	for (i =0; i < loader._MAPs.size(); i++)
	{
		if (REG(EIP) >= loader._MAPs[i].addr && REG(EIP) < loader._MAPs[i].size  + loader._MAPs[i].addr)
			break;
	}
	if (i == loader._MAPs.size()) ERROR("not a exception!");

	uint_t pc = cpu._regs[MIPS_REG_PC], a0 = cpu._regs[MIPS_REG_A0], a1 = cpu._regs[MIPS_REG_A1], a2 = cpu._regs[MIPS_REG_A2], a3 = cpu._regs[MIPS_REG_A3], sp = cpu._regs[MIPS_REG_SP], ra = cpu._regs[MIPS_REG_RA], t9 = cpu._regs[MIPS_REG_T9];
	cpu._regs[MIPS_REG_RA] = *(uint_t *)REG(ESP);
	cpu._regs[MIPS_REG_A0] = *(uint_t *)(REG(ESP)+4);
	cpu._regs[MIPS_REG_A1] = *(uint_t *)(REG(ESP)+8);
	cpu._regs[MIPS_REG_A2] = *(uint_t *)(REG(ESP)+12);
	cpu._regs[MIPS_REG_A3] = *(uint_t *)(REG(ESP)+16);


	cpu._regs[MIPS_REG_SP] -= 40;
	for (int i = 0; i < 40; i +=4)
	{
		if (REG(ESP) >= host_stack_highest - 20 - i) break;
		*(uint_t *)(cpu._regs[MIPS_REG_SP] + i) = *(uint_t * )(REG(ESP) + 20 + i);
	}
	cpu._regs[MIPS_REG_PC] = REG(EIP);
	cpu._regs[MIPS_REG_T9] = REG(EIP);

	start_simulate();

	((ucontext_t *) ucontext)->uc_mcontext.gregs[REG_EIP] = cpu._regs[MIPS_REG_PC];
	((ucontext_t *) ucontext)->uc_mcontext.gregs[REG_EAX] = cpu._regs[MIPS_REG_V0];
	((ucontext_t *) ucontext)->uc_mcontext.gregs[REG_ESP] += 4;

	cpu._regs[MIPS_REG_SP] += 40;
	ASSERT(cpu._regs[MIPS_REG_SP] == sp);
	cpu._regs[MIPS_REG_PC] = pc;
   	cpu._regs[MIPS_REG_A0] = a0;
   	cpu._regs[MIPS_REG_A1] = a1;
   	cpu._regs[MIPS_REG_A2] = a2;
   	cpu._regs[MIPS_REG_A3] = a3;
   	cpu._regs[MIPS_REG_SP] = sp;
   	cpu._regs[MIPS_REG_RA] = ra;
   	cpu._regs[MIPS_REG_T9] = t9;
#undef REG
	return;

}
void * load_symbol(const char *name, const std::vector<std::string> libraries)
{
	void * trans_addr = nullptr;
	for (int j = 0; j < libraries.size(); j++)
	{
		void * handle = dlopen(libraries[j].c_str(), RTLD_NODELETE|RTLD_NOW);
		if (!handle) continue;

		void * func_addr = dlsym(handle, name);
		dlclose(handle);
		if (func_addr)
		{
			trans_addr = func_addr;
			break;
		}
	}
	if (!trans_addr)
	{
		trans_addr = dlsym(RTLD_NEXT, name);
	}
	return trans_addr;
}

int main(int argc, char ** argv, char ** env)
{
	setbuf(stdout, nullptr);
	for (host_stack_highest = (uint_t)&env[0]; *(uint_t *)host_stack_highest; host_stack_highest += 4);


	struct sigaction sigaction_x86_to_mipsel;
	sigaction_x86_to_mipsel.sa_handler = NULL;
	sigaction_x86_to_mipsel.sa_sigaction = handler;
	sigemptyset(&sigaction_x86_to_mipsel.sa_mask);
	sigaction_x86_to_mipsel.sa_flags = SA_SIGINFO;
	sigaction_x86_to_mipsel.sa_restorer = NULL;
	sigaction(SIGSEGV, &sigaction_x86_to_mipsel, NULL);

	if (argc >= 6 && (argc-3) % 3 == 0 && !strcmp(argv[1], "--"))
	{
		uint_t base, size, entry;

		sscanf(argv[2], "0x%08x", &entry);
		loader.entry_point = entry;
		INFO("entry: %08x", entry);

		uint_t prot = PROT_WRITE|PROT_READ|PROT_EXEC;
		for (int i = 3; i < argc; i +=3)
		{
			sscanf(argv[i+1], "0x%08x", &base);
			sscanf(argv[i+2], "0x%08x", &size);
			loader.load(argv[i], true, base, size, prot);
			INFO("%s %08x %08x", argv[i], base, size);
			prot = PROT_WRITE|PROT_READ;
		}
		cpu._regs[MIPS_REG_SP] = create_fake_stack(argc-1, &argv[1], env);
		cpu._regs[MIPS_REG_A1] = 1088820;
		//cpu._regs[MIPS_REG_A1] = 100;
		cpu._regs[MIPS_REG_A0] = 0x90000000;
	}
	else if (argc >= 2 && strcmp(argv[1], "--"))
	{
		loader.load(argv[1]);
		cpu._regs[MIPS_REG_SP] = create_fake_stack(argc-1, &argv[1], env);
	}
	else
	{
		printf("./loader -- 0xentry file 0xbase 0xsize [file 0xbase 0xsize]\n");
		printf("./loader elf_file\n");
		_exit(0);
	}
	/*
	else
	{
		printf(	"Usage: \n"
				"       ./main elfpath arg1 arg2 ...\n"
				"       ./main -- rawpath base size entry      (all in hex format))\n"
			 );
		return 0;
	}
	*/


	cpu._regs[MIPS_REG_PC] = loader.entry_point;
	cpu._regs[MIPS_REG_T9] = loader.entry_point;
	//cpu._regs[MIPS_REG_SP] = create_fake_stack(argc-1, &argv[1], env);


	for (uint_t i = 0; i < loader.FUNCs.size(); i++)
	{
		void * trans_addr = load_symbol(loader.FUNCs[i].name.c_str(), loader.needed_libraries);
		if (trans_addr) //没找到的不要随便乱改。。。got。原来那个地方的值就是可以用的。。
		{
			*(uint_t*) loader.FUNCs[i].addr = (uint_t) trans_addr;
			got_trans_map[(uint_t)trans_addr] = loader.FUNCs[i].name;
			INFO("[LOAD FUNC] addr:%08x value:%08x %s", loader.FUNCs[i].addr, (uint_t)trans_addr, loader.FUNCs[i].name.c_str());
		}
	}
	INFO("LOAD FUNC finish");
	for (uint_t i = 0; i < loader.OBJECTs.size(); i++)
	{
		void * symbol = load_symbol(loader.OBJECTs[i].name.c_str(), loader.needed_libraries);
		if (!symbol && loader.OBJECTs[i].name == "__stack_chk_guard")
		{
			symbol = (void *)&fake__stack_stack_guard; 
		}
		if (symbol) //没找到的不要随便乱改。。。got。原来那个地方的值就是可以用的。。
		{
			INFO("[OBJECT] %s %08x %08x", loader.OBJECTs[i].name.c_str(), loader.OBJECTs[i].addr, symbol);
			//memcpy((void *)loader.OBJECTs[i].addr, &symbol, sizeof(uint_t));
			*(uint_t *) loader.OBJECTs[i].addr = (uint_t)symbol;
		}
	}

	uint_t lower_address = 0xffffffff;
	uint_t upper_address = 0;
	for (uint_t i = 0; i < loader._MAPs.size(); i++)
	{
		uint_t base = loader._MAPs[i].addr;
		uint_t size = loader._MAPs[i].size;
		uint_t prot = loader._MAPs[i].prot;

		if (prot & PROT_EXEC)
		{
			if (lower_address > base) lower_address = base;
			if (upper_address < base + size) upper_address = base + size;
		}
	}
	if (lower_address < upper_address)
	{
		inst_cache_base = lower_address;
		inst_cache_size = (upper_address + 3 - lower_address)/4;
		inst_cache = (Instruction **)malloc(inst_cache_size * sizeof(Instruction*));
		if (inst_cache == nullptr) ERROR("can not malloc inst_cache");
		memset(inst_cache, 0, inst_cache_size * 4);
	}

	INFO("LOAD OBJECTS finish");
	INFO("start simulation!!!!");
	start_simulate();
	ERROR("[ERROR] execution uncontrolable...with v0:%08x", cpu._regs[MIPS_REG_V0]);
	return 0;
}
