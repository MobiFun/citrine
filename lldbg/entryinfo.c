#include "types.h"

u32 lldbg_entry_cpsr;
u32 lldbg_entry_sp;

void
lldbg_cmd_entry()
{
	lldbg_printf("CPSR on entry: %08X\n", lldbg_entry_cpsr);
	lldbg_printf("SP on entry after register save: %08X\n", lldbg_entry_sp);
}
