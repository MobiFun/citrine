/*
 * This C module is a reconstruction based on the disassembly of
 * os_sem.obj in frame_na7_db_ir.lib from the Leonardo package.
 */

/* set of included headers from COFF symtab: */
#include <stdio.h>
#include <string.h>
#include "gpfconf.h"	/* FreeCalypso addition */
#include "../../nucleus/nucleus.h"
#include "typedefs.h"
#include "os.h"
#include "gdi.h"
#include "os_types.h"
#include "os_glob.h"

extern T_OS_SEM_TABLE_ENTRY SemTable[];
extern unsigned os_time_to_tick_multiplier;

int
ReleaseSemaphoreCB(NU_SEMAPHORE *SemCB)
{
	if (NU_Release_Semaphore(SemCB) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}

GLOBAL LONG
os_ReleaseSemaphore(OS_HANDLE TaskHandle, OS_HANDLE SemHandle)
{
	if (NU_Release_Semaphore(&SemTable[SemHandle].SemCB) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}

int
ObtainSemaphoreCB(NU_SEMAPHORE *SemCB, ULONG Timeout, USHORT wait_check)
{
	UNSIGNED nu_timeout;
	STATUS sts;
	int ret;

	ret = OS_OK;
	if (Timeout != OS_SUSPEND)
		nu_timeout = TIME_TO_SYSTEM_TICKS(Timeout);
	else if (wait_check == 1)
		nu_timeout = 1;
	else
		nu_timeout = NU_SUSPEND;
	for (;;) {
		sts = NU_Obtain_Semaphore(SemCB, nu_timeout);
		switch (sts) {
		case NU_SUCCESS:
			return(ret);
		case NU_INVALID_SEMAPHORE:
			return(OS_ERROR);
		case NU_INVALID_SUSPEND:
			nu_timeout = 0;
			continue;
		case NU_TIMEOUT:
		case NU_UNAVAILABLE:
			if (nu_timeout == 1 && wait_check == 1) {
				nu_timeout = NU_SUSPEND;
				ret = OS_WAITED;
				continue;
			}
			return(OS_TIMEOUT);
		default:
			/*
			 * Disassembly reveals that the original code
			 * has an endless loop here, the equivalent
			 * of continue.  My guess is that they simply
			 * forgot the default case, and so control
			 * falls onto the closing brace of the switch
			 * and then onto the closing brace of the for
			 * loop.  But I prefer better error handling,
			 * hence the present addition. - Space Falcon
			 */
			return(OS_ERROR);
		}
	}
}

GLOBAL LONG
os_ObtainSemaphore(OS_HANDLE TaskHandle, OS_HANDLE SemHandle, ULONG Timeout)
{
	if (SemHandle > MaxSemaphores)
		return(OS_ERROR);
	return ObtainSemaphoreCB(&SemTable[SemHandle].SemCB, Timeout, 0);
}
