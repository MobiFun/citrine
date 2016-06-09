/*
 * This C module is a reconstruction based on the disassembly of
 * os_drv.obj in frame_na7_db_fl.lib from the Leonardo package.
 */

/* set of included headers from COFF symtab: */
#include "gpfconf.h"	/* FreeCalypso addition */
#include "../../nucleus/nucleus.h"
#include "typedefs.h"
#include "gdi.h"
#include "os.h"

typedef unsigned char u_char;

extern OS_HANDLE os_ext_pool_handle;

static u_char *HISR_Stack;
static T_DRV_SIGNAL *SignalID;
static void (*DrvCallback)(T_DRV_SIGNAL *);
static NU_HISR CallBack_HISR;

#define	CB_HISR_STACK_SIZE	1024

GLOBAL LONG
os_ExecuteCallback(OS_HANDLE Caller, void (*Callback)(T_DRV_SIGNAL *),
			T_DRV_SIGNAL *Signal)
{
	DrvCallback = Callback;
	SignalID = Signal;
	NU_Activate_HISR(&CallBack_HISR);
	return(OS_OK);
}

static void
CallbackFunc(void)
{
	DrvCallback(SignalID);
}

GLOBAL LONG
os_CreateCallback(void)
{
	if (os_AllocateMemory(OS_NOTASK, (T_VOID_STRUCT **) &HISR_Stack,
				CB_HISR_STACK_SIZE, 0xFFFFFFFF,
				os_ext_pool_handle) == OS_ERROR)
		return(OS_ERROR);
	if (NU_Create_HISR(&CallBack_HISR, "CB_HISR", CallbackFunc, 2,
				HISR_Stack, CB_HISR_STACK_SIZE) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}
