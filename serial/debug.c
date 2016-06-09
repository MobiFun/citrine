/*
 * This module is a FreeCalypso addition for very low-level debugging.
 * The functions implemented in this module produce "forced" serial output
 * on the RVTMUX UART without going through the normal RV trace mechanism.
 * Calls to these FC debug functions should be added during difficult
 * debug sessions, but should never remain in stable checked-in code:
 * these functions are too disruptive to be used in "production" fw images.
 */

#include "../include/config.h"
#include "../include/sys_types.h"

#include "serialswitch.h" 

#include <string.h>

freecalypso_raw_dbgout(char *string)
{
	char *p;
	int l, cc;

	p = string;
	l = strlen(p);
	while (l) {
		cc = SER_tr_WriteNBytes(SER_LAYER_1, p, l);
		p += cc;
		l -= cc;
	}
}

#if 0
freecalypso_lldbg_intinfo()
{
	char strbuf[128];
	extern unsigned IQ_TimerCount2;
	extern unsigned TMD_System_Clock;
	extern unsigned INT_Check_IRQ_Mask();

	sprintf(strbuf, "*CPSR=%08x, IQ_TimerCount2=%u, TMD_System_Clock=%u",
		INT_Check_IRQ_Mask(), IQ_TimerCount2, TMD_System_Clock);
	freecalypso_raw_dbgout(strbuf);
}

freecalypso_nucidle_dbghook()
{
	freecalypso_raw_dbgout("*In Nucleus idle loop");
	freecalypso_lldbg_intinfo();
}
#endif
