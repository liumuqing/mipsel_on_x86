#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>


//#define BIGENDIAN
#define ERROR(fmt, ...) (printf("[ERROR]" fmt "\n", ##__VA_ARGS__), exit(1))
#ifdef DEBUG
	#define INFO(fmt, ...) printf("[INFO]" fmt "\n", ##__VA_ARGS__)
#else
	#define INFO(fmt, ...) 
#endif

#define WARNING(fmt, ...) printf("[WARNING]" fmt "\n", ##__VA_ARGS__)
//#define ASSERT(cond) if (!(cond)) (printf("[ASSERT] %s", #cond), exit(1))
#define ASSERT(cond)

#define MAX_OPERAND_COUNT (4)

#define CONFIG_WITH_MNEMONIC
#ifdef CONFIG_WITH_MNEMONIC
	#define MNEMONIC_MAX_LEN 16
#endif
typedef uint32_t uint_t;

/*
 * for MAC OSX
 */
//#define MIPS_REG_ENDING MIPS_REG_MAX
#ifndef MAP_ANONYMOUS
	#define MAP_ANONYMOUS MAP_ANON
#endif

#define COMMON_PAGE_BIT_LEN 12
#define COMMON_PAGE_SIZE 0x1000 // should be the bigger one of x86 and mipsel


#ifdef BIGENDIAN
template <typename T> T change_endian(T value)
{
	T retv = 0;
	for (int i = 0; i < sizeof(T); i++)
	{
		uint8_t b = value & 0xff;
		value = value >> 8;
		retv <<= 8;
		retv |= b;
	}
	return retv;
}
template <> double change_endian<double>(double value)
{
	uint64_t retv = change_endian<uint64_t>(reinterpret_cast<uint64_t&>(value));
	return reinterpret_cast<double&>(retv);
}
template <> float change_endian<float>(float value)
{
	uint32_t retv = change_endian<uint32_t>(reinterpret_cast<uint32_t&>(value));
	return reinterpret_cast<float&>(retv);
}
#else
template <typename T> T change_endian(T value){return value;}
#endif
