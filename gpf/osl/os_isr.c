/*
 * This C module is a reconstruction based on the disassembly of
 * os_isr.obj in frame_na7_db_fl.lib from the Leonardo package.
 */

/* set of included headers from COFF symtab: */
#include <string.h>
#include "gpfconf.h"	/* FreeCalypso addition */
#include "../../nucleus/nucleus.h"
#include "typedefs.h"
#include "os.h"
#include "gdi.h"
#include "os_types.h"
#include "os_glob.h"

typedef unsigned char u_char;

extern T_OS_OSISR_TABLE_ENTRY OSISRTable[];
extern OS_HANDLE os_int_pool_handle;

GLOBAL LONG
os_isr_init(void)
{
	USHORT i;

	for (i = 1; i <= MaxOSISRs; i++)
		OSISRTable[i].name[0] = 0;
	return(OS_OK);
}

GLOBAL LONG
os_SetInterruptState(OS_INT_STATE new_state, OS_INT_STATE *old_state)
{
	INT state;

	if (new_state)
		state = NU_ENABLE_INTERRUPTS;
	else
		state = NU_DISABLE_INTERRUPTS;
	state = NU_Control_Interrupts(state);
	if (state & 0xFF)
		*old_state = 0;
	else
		*old_state = 1;
	return(OS_OK);
}

GLOBAL LONG
os_EnableInterrupts(OS_INT_STATE *old_state)
{
	INT state;

	state = NU_Control_Interrupts(NU_ENABLE_INTERRUPTS);
	if (state & 0xFF)
		*old_state = 0;
	else
		*old_state = 1;
	return(OS_OK);
}

GLOBAL LONG
os_DisableInterrupts(OS_INT_STATE *old_state)
{
	INT state;

	state = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);
	if (state & 0xFF)
		*old_state = 0;
	else
		*old_state = 1;
	return(OS_OK);
}

GLOBAL LONG
os_DeleteOSISR(OS_HANDLE hisr_handle)
{
	OS_INT_STATE old_state, state;

	if (hisr_handle <= 0 || hisr_handle > MaxOSISRs)
		return(OS_ERROR);
	if (OSISRTable[hisr_handle].hisr_cb.tc_activation_count)
		return(OS_ERROR);
	os_DisableInterrupts(&old_state);
	if (os_DeallocateMemory(os_MyHandle(), OSISRTable[hisr_handle].stack)
	    == OS_ERROR) {
error:		os_SetInterruptState(old_state, &state);
		return(OS_ERROR);
	}
	if (NU_Delete_HISR(&OSISRTable[hisr_handle].hisr_cb) != NU_SUCCESS)
		goto error;
	OSISRTable[hisr_handle].name[0] = 0;
	os_SetInterruptState(old_state, &state);
	return(OS_OK);
}

GLOBAL LONG
os_CreateOSISR(char *name, void (*OSISR_entry)(void),
		int stacksize, int priority,
		int flags, OS_HANDLE *hisr_handle)
{
	OS_HANDLE handle;
	T_VOID_STRUCT *hisr_stack;
	OS_INT_STATE old_state, state;

	if (priority < 0 || priority > 2)
		return(OS_ERROR);
	priority = 2 - priority;
	os_DisableInterrupts(&old_state);
	for (handle = 1; handle <= MaxOSISRs; handle++)
		if (!strncmp(OSISRTable[handle].name, name,
				RESOURCE_NAMELEN - 1)) {
error:			os_SetInterruptState(old_state, &state);
			return(OS_ERROR);
		}
	for (handle = 1; handle <= MaxOSISRs; handle++)
		if (!OSISRTable[handle].name[0])
			break;
	if (handle > MaxOSISRs)
		goto error;
	if (os_AllocateMemory(os_MyHandle(), &hisr_stack, stacksize,
				0xFFFFFFFF, os_int_pool_handle) == OS_ERROR)
		goto error;
	memset((u_char *)hisr_stack, INITIAL_STACK_VALUE, stacksize);
	*hisr_stack = GUARD_PATTERN;
	if (NU_Create_HISR(&OSISRTable[handle].hisr_cb, name, OSISR_entry,
				priority, (VOID *)hisr_stack, stacksize)
			!= NU_SUCCESS)
		goto error;
	strncpy(OSISRTable[handle].name, name, RESOURCE_NAMELEN);
	OSISRTable[handle].name[RESOURCE_NAMELEN-1] = 0;
	OSISRTable[handle].stack = hisr_stack;
	*hisr_handle = handle;
	os_SetInterruptState(old_state, &state);
	return(OS_OK);
}

GLOBAL LONG
os_ActivateOSISR(OS_HANDLE hisr_handle)
{
	if (hisr_handle <= 0 || hisr_handle > MaxOSISRs)
		return(OS_ERROR);
	if (NU_Activate_HISR(&OSISRTable[hisr_handle].hisr_cb) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}
