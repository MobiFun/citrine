#include "cmdtab.h"

extern void lldbg_cmd_dump();
extern void lldbg_cmd_entry();
extern void lldbg_cmd_r8();
extern void lldbg_cmd_r16();
extern void lldbg_cmd_r32();
extern void lldbg_cmd_w8();
extern void lldbg_cmd_w16();
extern void lldbg_cmd_w32();

const struct cmdtab lldbg_cmdtab[] = {
	{"dump", lldbg_cmd_dump},
	{"entry", lldbg_cmd_entry},
	{"r8", lldbg_cmd_r8},
	{"r16", lldbg_cmd_r16},
	{"r32", lldbg_cmd_r32},
	{"w8", lldbg_cmd_w8},
	{"w16", lldbg_cmd_w16},
	{"w32", lldbg_cmd_w32},
	{0, 0}
};
