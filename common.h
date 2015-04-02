#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ERROR(fmt, ...) (printf("[ERROR]" fmt "\n", ##__VA_ARGS__), exit(1))
#ifdef DEBUG
	#define INFO(fmt, ...) printf("[INFO]" fmt "\n", ##__VA_ARGS__)
#else
	#define INFO(fmt, ...) 
#endif
#define WARNING(fmt, ...) printf("[WARNING]" fmt "\n", ##__VA_ARGS__)
#define ASSERT(cond) if (!(cond)) (printf("[ASSERT] %s", #cond), exit(1))

//TODO:try make fast?
#define TO_HOST(x) ((x<<24) | ((x<<8)&0x00ff0000) | ((x>>8)&0x0000ff00) | ((x>>24)&0xff))

#define MAX_OPERAND_COUNT (4)

#define CONFIG_WITH_MNEMONIC
#ifdef CONFIG_WITH_MNEMONIC
	#define MNEMONIC_MAX_LEN 16
#endif
typedef uint32_t uint_t;

/*
 * for macos
 */
#ifndef MAP_ANONYMOUS
	#define MAP_ANONYMOUS MAP_ANON
#endif

#define COMMON_PAGE_SIZE 0x1000 // should be the bigger one of x86 and mipsel
