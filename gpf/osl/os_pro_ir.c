/*
 * This C module is a reconstruction based on the disassembly of
 * os_pro.obj in frame_na7_db_ir.lib from the Leonardo package.
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

extern VOID *TCD_Current_Thread;
extern T_OS_TASK_TABLE_ENTRY TaskTable[];
extern unsigned os_time_to_tick_multiplier;

#define	OS_NU_TASK_MAGIC	0xdeafbeef

VOID
os_TaskEntry(UNSIGNED TaskHandle, VOID *argv)
{
	TaskTable[TaskHandle].TaskEntry(TaskHandle, 0);
}

GLOBAL LONG
os_SuspendTask(OS_HANDLE Caller, ULONG Time)
{
	UNSIGNED SuspendTicks;

	SuspendTicks = TIME_TO_SYSTEM_TICKS(Time);
	if (!SuspendTicks)
		SuspendTicks = 1;
	NU_Sleep(SuspendTicks);
	return (OS_OK);
}

GLOBAL LONG
os_ResumeTask(OS_HANDLE task_handle)
{
	if (NU_Resume_Task(&TaskTable[task_handle].TaskCB.TCB) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}

GLOBAL LONG
os_Relinquish(void)
{
	NU_Relinquish();
	return(OS_OK);
}

GLOBAL OS_HANDLE
os_MyHandle(void)
{
	OS_NU_TASK *os_nu_task = (OS_NU_TASK *) TCD_Current_Thread;

	if (os_nu_task && os_nu_task->magic_nr == OS_NU_TASK_MAGIC)
		return(os_nu_task->handle);
	else
		return(OS_NOTASK);
}

GLOBAL LONG
os_GetTaskName(OS_HANDLE Caller, OS_HANDLE TaskHandle, char *Name)
{
	if (TaskHandle) {
		if (TaskHandle > MaxTasks || !TaskTable[TaskHandle].Name[0])
			return(OS_ERROR);
		strcpy(Name, TaskTable[TaskHandle].Name);
	} else if (TCD_Current_Thread) {
		NU_TASK *curtask = TCD_Current_Thread;
		strcpy(Name, curtask->tc_name);
	} else
		strcpy(Name, "ROOT");
	return(OS_OK);
}

GLOBAL LONG
os_DeferTask(OS_HANDLE task_handle, OS_TIME time)
{
	if (NU_Suspend_Task(&TaskTable[task_handle].TaskCB.TCB) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}

GLOBAL LONG
os_CheckTaskStack(OS_HANDLE Handle)
{
	register void *sp asm("sp");

	if (*TaskTable[Handle].Stack != GUARD_PATTERN)
		return(OS_ERROR);
	if (TCD_Current_Thread) {
		NU_TASK *curtask = TCD_Current_Thread;
		if (sp < curtask->tc_stack_start)
			return(OS_ERROR);
	}
	return(OS_OK);
}
