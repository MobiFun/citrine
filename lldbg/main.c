#include "types.h"

lldbg_main()
{
	lldbg_printf("\2\2\2*Low Level Debug mode entered\2");
	for (;;) {
		lldbg_putchar('>');
		if (lldbg_command_entry())
			lldbg_command_dispatch();
	}
}
