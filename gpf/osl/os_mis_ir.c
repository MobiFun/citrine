/*
 * This C module is a reconstruction based on the disassembly of
 * os_mis.obj in frame_na7_db_ir.lib from the Leonardo package.
 */

/* subset of included headers from COFF symtab: */
#include "gpfconf.h"	/* FreeCalypso addition */
#include "../../nucleus/nucleus.h"
#include "typedefs.h"
#include "os.h"
#include "gdi.h"
#include "os_types.h"
#include "os_glob.h"

extern unsigned os_tick_to_time_multiplier;

GLOBAL LONG
os_GetTime(OS_HANDLE Caller, OS_TIME *Time)
{
	UNSIGNED Value;

	Value = NU_Retrieve_Clock();
	*Time = SYSTEM_TICKS_TO_TIME(Value);
	return(OS_OK);
}
