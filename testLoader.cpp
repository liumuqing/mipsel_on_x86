#include "ElfLoader.h"
int testmain(int argc, char ** argv)
{
	ElfLoader loader;
	loader.load(argv[1]);
	loader.mprotect();
	return 0;
}
