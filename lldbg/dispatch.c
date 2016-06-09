/*
 * This module implements the dispatch of interactively entered
 * commands to their respective implementation functions via cmdtab.
 */

#include "cmdtab.h"

extern char lldbg_command[];
extern struct cmdtab lldbg_cmdtab[];

void
lldbg_command_dispatch()
{
	char *cp, *np;
	struct cmdtab *tp;

	for (cp = lldbg_command; *cp == ' '; cp++)
		;
	if (!*cp)
		return;
	for (np = cp; *cp && *cp != ' '; cp++)
		;
	if (*cp)
		*cp++ = '\0';
	for (tp = lldbg_cmdtab; tp->cmd; tp++)
		if (!strcmp(tp->cmd, np))
			break;
	if (tp->func)
		tp->func(cp);
	else
		lldbg_printf("ERROR: unknown or unimplemented command\n");
}
