#include <stdio.h>

#include "../elfio/elfio.hpp"
using namespace ELFIO;
int main(int argc, char * argv[])
{
	if (argc!=2){printf("args!\n"); return 1;}
	elfio reader;
	if (!reader.load(argv[1])) {printf("parsing error!"); return 1;}

	printf("PT_LOAD:%08x\n", PT_LOAD);
	for ( int i = 0; i < reader.segments.size(); ++i )
	{
		const segment* pseg = reader.segments[i];
		printf("[%02d] type:%08x rwx:%08x vaddr:%08llxx paddr:%08llx vsize:%08llx\n", i, pseg->get_type(), pseg->get_flags(), pseg->get_virtual_address(), pseg->get_file_size(), pseg->get_memory_size());
	}

	Elf_Half sec_num = reader.sections.size();
	printf("Number of sections: %d\n", sec_num);
	for ( int i = 0; i < sec_num; ++i )
	{
		const section* psec = reader.sections[i]; 
		printf("[%02d] %30s addr:%08llx size:%08llx type:%08x\n", i, psec->get_name().c_str(), psec->get_address(), psec->get_size(), psec->get_type());
	}
	printf("DYNAMIC: SHT_DYNAMIC:%08x\n", SHT_DYNAMIC);
	for (int i = 0; i< sec_num; ++i)
	{
		section* psec = reader.sections[i]; 
		if ( psec->get_type() == SHT_DYNAMIC)
		{
			printf("Section Name: %s\n", psec->get_name().c_str());
			const dynamic_section_accessor symbols( reader, psec );
			for ( unsigned int j = 0; j < symbols.get_entries_num(); ++j )
			{
				Elf_Xword   tag;
				Elf_Xword   value;
				std::string str;
				symbols.get_entry( j, tag, value, str);
				printf("(%03d) tag:%08llx value:%08llx %s\n", j, tag, value, str.c_str());
			}
		}
	}
	printf("Symbols: SHT_DYNSYM:%08x\n", SHT_DYNSYM);
	for (int i = 0; i< sec_num; ++i)
	{
		section* psec = reader.sections[i]; 
		if ( psec->get_type() == SHT_DYNSYM)
		{
			printf("Section Name: %s\n", psec->get_name().c_str());
			const symbol_section_accessor symbols( reader, psec );
			for ( unsigned int j = 0; j < symbols.get_symbols_num(); ++j )
			{
				std::string name;
				Elf64_Addr value;
				Elf_Xword size;
				unsigned char bind;
				unsigned char type;
				Elf_Half      section_index;
				unsigned char other;
				symbols.get_symbol( j, name, value, size, bind, type, section_index, other );
				printf("(%03d) %20s value:%08llx size:%08llx bind:%02x type:%02x other:%llx\n", j, name.c_str(), value, size, bind, type, other);
			}
		}
	}

	printf("\nSymbols: SHT_REL:%08x\n", SHT_REL);
	for (int i = 0; i< sec_num; ++i)
	{
		section* psec = reader.sections[i]; 
		if ( psec->get_type() == SHT_REL)
		{
			printf("Section Name: %s\n", psec->get_name().c_str());
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
				printf("(%03d) %20s offset:%08llx symbolValue:%08llx type:%08x addend:%08llx calcValue:%08llx\n", j, symbolName.c_str(), offset, symbolValue, type, addend, calcValue);
			}
		}
	}
}
