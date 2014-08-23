
#include "common.h"
#include <Windows.h>

#define JMP32_SZ 5 // the size of JMP <address>
#define NOP 0x90 // opcode for NOP
#define JMP 0xE9 // opcode for JUMP

/**
 * Found this snippet on http://www.progamercity.net/code-tut/744-c-writing-your-own-detour-functions.html
 */
void *detourFunction(uint8_t *orig, uint8_t *hook, int len) {

	DWORD dwProt = 0;
	uint8_t *jmp = (uint8_t*)malloc(len + JMP32_SZ);
	
	VirtualProtect(orig, len, PAGE_READWRITE, &dwProt);
	
	memcpy(jmp, orig, len);
	jmp += len; // increment to the end of the copied bytes
	jmp[0] = JMP;

	*(DWORD*)(jmp + 1) = (DWORD)(orig + len - jmp) - JMP32_SZ;
	memset(orig, NOP, len);

	orig[0] = JMP;
	*(DWORD*)(orig + 1) = (DWORD)(hook - orig) - JMP32_SZ;

	VirtualProtect(orig, len, dwProt, 0);
	return (jmp - len);
}
