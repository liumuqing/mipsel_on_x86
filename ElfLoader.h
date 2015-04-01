#pragma once
#include <vector>
#include <string>

#include "common.h"
/* ElfLoader 帮你把ELF加载到内存中，但是不加载library，不处理got等，并且内存是可读可写，加载完成后你做完需要完成的工作之后，调用mprotect()，Loader帮你把ELF对应的内存的权限设置好。*/
struct FUNC_t
{
	std::string name;
	uint_t addr;
};
struct OBJECT_t
{
	std::string name;
	uint_t addr;
	uint_t size;
};
struct MAP_t
{
	uint_t addr;
	uint_t size;
	int prot;
};
class ElfLoader
{
public:
	ElfLoader (){};
	std::vector<std::string> needed_libraries;
	std::vector<FUNC_t> FUNCs;//里面是一个一个需要填充的got 
	std::vector<OBJECT_t> OBJECTs;
	uint_t entry_point;
	void load(const char *);
	void mprotect();
	std::vector<MAP_t> _MAPs;

private:
	ElfLoader (const ElfLoader &);
	ElfLoader (const ElfLoader &&);
};
