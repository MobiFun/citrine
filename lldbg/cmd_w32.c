/*
 * w32 hexaddr xxxxxxxx -- write a 32-bit register or memory location
 */

#include <sys/types.h>
#include "types.h"

void
lldbg_cmd_w32(argbulk)
	char *argbulk;
{
	char *argv[3];
	u_long addr, data;

	if (lldbg_parse_args(argbulk, 2, 2, argv, 0) < 0)
		return;
	if (lldbg_parse_hexarg(argv[0], 8, &addr) < 0) {
	      lldbg_printf("ERROR: arg1 must be a valid 32-bit hex address\n");
		return;
	}
	if (addr & 3) {
		lldbg_printf("ERROR: unaligned address\n");
		return;
	}
	if (lldbg_parse_hexarg(argv[1], 8, &data) < 0) {
		lldbg_printf("ERROR: arg2 must be a valid 32-bit hex value\n");
		return;
	}
	*(volatile u32 *)addr = data;
}
