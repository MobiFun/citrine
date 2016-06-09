/*
 * r32 hexaddr -- read a 32-bit register or memory location
 */

#include <sys/types.h>
#include "types.h"

void
lldbg_cmd_r32(argbulk)
	char *argbulk;
{
	char *argv[2];
	u_long addr;

	if (lldbg_parse_args(argbulk, 1, 1, argv, 0) < 0)
		return;
	if (lldbg_parse_hexarg(argv[0], 8, &addr) < 0) {
	  lldbg_printf("ERROR: argument must be a valid 32-bit hex address\n");
		return;
	}
	if (addr & 3) {
		lldbg_printf("ERROR: unaligned address\n");
		return;
	}
	lldbg_printf("%08X\n", *(volatile u32 *)addr);
}
