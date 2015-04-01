#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <elf.h>
#include "elfio/elfio.hpp"


#include "common.h"
#include "ElfLoader.h"

using namespace ELFIO;

void ElfLoader::load(const char * file_path)
{
	elfio reader;
	if (!reader.load(file_path)) ERROR("");
	INFO("entry_point:%llx", reader.get_entry());
	entry_point = reader.get_entry();

	INFO("PT_LOAD:%08x\n", PT_LOAD);
	for ( int i = 0; i < reader.segments.size(); ++i)
	{
		const segment* pseg = reader.segments[i];
		INFO("[%02d] type:%08x rwx:%08x vaddr:%08llxx paddr:%08llx vsize:%08llx\n", i, pseg->get_type(), pseg->get_flags(), pseg->get_virtual_address(), pseg->get_file_size(), pseg->get_memory_size());
		if (pseg->get_type() == PT_LOAD)
		{
			//align
			uint64_t v_page_addr = pseg->get_virtual_address() - (pseg->get_virtual_address() & (COMMON_PAGE_SIZE-1));
			uint64_t v_page_size = ((((pseg->get_memory_size() + (pseg->get_virtual_address() - v_page_addr)) - 1) | (COMMON_PAGE_SIZE - 1))) + 1;

			if (v_page_addr != (uint64_t)mmap((void *)v_page_addr, v_page_size ,PROT_WRITE|PROT_READ, MAP_FIXED|MAP_ANONYMOUS|MAP_SHARED, -1, 0)) 
				ERROR("can't mmap at 0x%08llx with size 0x%08llx", v_page_addr, v_page_size);

			memcpy((void*) pseg->get_virtual_address(), pseg->get_data(), pseg->get_file_size());
			MAP_t t;
			t.addr = v_page_addr;
			t.size = v_page_size;
			t.prot = pseg->get_flags();
			_MAPs.push_back(t);
		}

	}

	Elf_Half sec_num = reader.sections.size();
	INFO("Number of sections: %d\n", sec_num);
	for ( int i = 0; i < sec_num; ++i )
	{
		const section* psec = reader.sections[i]; 
		INFO("[%02d] %30s addr:%08llx size:%08llx type:%08x\n", i, psec->get_name().c_str(), psec->get_address(), psec->get_size(), psec->get_type());
	}

	INFO("DYNAMIC: SHT_DYNAMIC:%08x\n", SHT_DYNAMIC);
	uint_t dt_pltgot, dt_mips_local_gotno, dt_mips_gotsym;
	for (int i = 0; i< sec_num; ++i)
	{
		section* psec = reader.sections[i]; 
		if ( psec->get_type() == SHT_DYNAMIC)
		{
			INFO("Section Name: %s\n", psec->get_name().c_str());
			const dynamic_section_accessor symbols( reader, psec );
			for ( unsigned int j = 0; j < symbols.get_entries_num(); ++j )
			{
				Elf_Xword   tag;
				Elf_Xword   value;
				std::string str;
				symbols.get_entry( j, tag, value, str);
				if (tag == DT_NEEDED)
				{
					INFO("(%03d) tag:%08llx value:%08llx %s\n", j, tag, value, str.c_str());
					needed_libraries.push_back(str);
				}
				//according to this url,https://www.cr0.org/paper/mips.elf.external.resolution.txt
				//got items should be placed to DT_PLTGOT + (DT_MIPS_LOCAL_GOTNO - DT_MIPS_GOTSYM) * 4  one by one
				if (tag == DT_PLTGOT) dt_pltgot = value;
				if (tag == DT_MIPS_LOCAL_GOTNO) dt_mips_local_gotno = value;
				if (tag == DT_MIPS_GOTSYM) dt_mips_gotsym = value;
			}
		}
	}

	INFO("Symbols: SHT_DYNSYM:%08x\n", SHT_DYNSYM);
	if (!(dt_pltgot && dt_mips_gotsym && dt_mips_local_gotno)) ERROR("con't find import information for got");
	for (int i = 0; i< sec_num; ++i)
	{
		section* psec = reader.sections[i]; 
		if ( psec->get_type() == SHT_DYNSYM)
		{
			INFO("Section Name: %s\n", psec->get_name().c_str());
			const symbol_section_accessor symbols( reader, psec );
			for ( unsigned int j = dt_mips_gotsym; j < symbols.get_symbols_num(); ++j )
			{
				std::string name;
				Elf64_Addr value;
				Elf_Xword size;
				unsigned char bind;
				unsigned char type;
				Elf_Half      section_index;
				unsigned char other;
				symbols.get_symbol( j, name, value, size, bind, type, section_index, other );
				INFO("(%03d) %20s value:%08llx size:%08llx bind:%02x type:%02x section_index:%08lx\n", j, name.c_str(), value, size, bind, type, section_index);
				if (section_index != 0) continue;
				if (type == STT_OBJECT)
				{
					if (bind == STB_GLOBAL || STB_WEAK)
					{
						OBJECT_t t;
						t.name = name;
						//t.addr = value;
						t.size = size;
						t.addr = dt_pltgot + (dt_mips_local_gotno - dt_mips_gotsym + j) * 4;
						OBJECTs.push_back(t);
					}
				}
				else if (type == STT_FUNC)
				{
					if ((bind == STB_GLOBAL|| bind == STB_WEAK))
					{
						FUNC_t t;
						t.name = name;
						t.addr = dt_pltgot + (dt_mips_local_gotno - dt_mips_gotsym + j) * 4;
						FUNCs.push_back(t);
					}
				}
			}
		}
	}
	for (int i = 0; i < FUNCs.size(); i++)
		INFO("[FUNC] %08x %s", FUNCs[i].addr, FUNCs[i].name.c_str());
	for (int i = 0; i < OBJECTs.size(); i++)
		INFO("[OBJECT] %08x %s", OBJECTs[i].addr, OBJECTs[i].name.c_str());

	/*
	INFO("\nSymbols: SHT_REL:%08x\n", SHT_REL);
	for (int i = 0; i< sec_num; ++i)
	{
		section* psec = reader.sections[i]; 
		if ( psec->get_type() == SHT_REL)
		{
			INFO("Section Name: %s\n", psec->get_name().c_str());
			const relocation_section_accessor symbols( reader, psec );
			for ( unsigned int j = 0; j < symbols.get_entries_num(); ++j )
			{
				Elf64_Addr  offset;
				Elf64_Addr  symbolValue;
				std::string symbolName;
				Elf_Word    type;
				Elf_Sxword  addend;
				Elf_Sxword  calcValue;
				symbols.get_entry( j, offset, symbolValue, symbolName, type, addend, calcValue);
				INFO("(%03d) %20s offset:%08llx symbolValue:%08llx type:%08x addend:%08llx calcValue:%08llx\n", j, symbolName.c_str(), offset, symbolValue, type, addend, calcValue);
			}
		}
	}
	*/
}

void ElfLoader::mprotect()
{
	for (int i = 0; i < _MAPs.size(); i++)
	{
		if(0 != ::mprotect((void *)_MAPs[i].addr, _MAPs[i].size, _MAPs[i].prot))
			ERROR("mprotect(%08x, %08x, %08x) failed", _MAPs[i].addr, _MAPs[i].size, _MAPs[i].prot);
	}
}
