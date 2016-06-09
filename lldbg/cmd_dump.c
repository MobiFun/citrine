/*
 * This is a human-oriented memory dump command.  The dump is given in
 * both hex and ASCII, with readable spacing.
 */

#include <sys/types.h>
#include "types.h"

void
lldbg_cmd_dump(argbulk)
	char *argbulk;
{
	char *argv[3];
	u_long start, length;
	u_long offset;
	u_char intbuf[16];
	int i, c;

	if (lldbg_parse_args(argbulk, 2, 2, argv, 0) < 0)
		return;
	if (lldbg_parse_hexarg(argv[0], 8, &start) < 0) {
	      lldbg_printf("ERROR: arg1 must be a valid 32-bit hex address\n");
		return;
	}
	if (lldbg_parse_hexarg(argv[1], 8, &length) < 0) {
    lldbg_printf("ERROR: arg2 must be a valid 32-bit hex value (length)\n");
		return;
	}
	if (start & 0xF || length & 0xF) {
    lldbg_printf("ERROR: implementation limit: 16-byte alignment required\n");
		return;
	}
	for (offset = 0; offset < length; offset += 0x10) {
		bcopy(start + offset, intbuf, 0x10);
		lldbg_printf("%08X: ", start + offset);
		for (i = 0; i < 16; i++) {
			lldbg_printf("%02X ", intbuf[i]);
			if ((i & 3) == 3)
				lldbg_putchar(' ');
		}
		for (i = 0; i < 16; i++) {
			c = intbuf[i];
			if (c >= ' ' && c <= '~')
				lldbg_putchar(c);
			else
				lldbg_putchar('.');
		}
		lldbg_putchar('\n');
	}
}
