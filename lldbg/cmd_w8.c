/*
 * w8 hexaddr xx -- write an 8-bit register or memory location
 */

#include <sys/types.h>
#include "types.h"

void
lldbg_cmd_w8(argbulk)
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
	if (lldbg_parse_hexarg(argv[1], 2, &data) < 0) {
		lldbg_printf("ERROR: arg2 must be a valid 8-bit hex value\n");
		return;
	}
	*(volatile u8 *)addr = data;
}
