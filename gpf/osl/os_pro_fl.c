/*
 * This C module is a reconstruction based on the disassembly of
 * os_pro.obj in frame_na7_db_fl.lib from the Leonardo package.
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

typedef unsigned char u_char;

extern VOID *TCD_Current_Thread;
extern T_OS_TASK_TABLE_ENTRY TaskTable[];
extern VOID os_TaskEntry(UNSIGNED, VOID *);

static NU_SEMAPHORE ProSemCB;

#define	OS_NU_TASK_MAGIC	0xdeafbeef

static int
os_GetTaskEntry(USHORT Index, OS_HANDLE *Handle)
{
	static USHORT Idx;

	if (Index == FIRST_ENTRY)
		Idx = 0;
	if (Index == FIRST_ENTRY || Index == NEXT_ENTRY) {
		while (++Idx <= MaxTasks && !TaskTable[Idx].Name[0])
			;
	} else
		Idx = Index;
	if (Idx <= MaxTasks && TaskTable[Idx].Name[0]) {
		*Handle = Idx;
		return(0);
	} else
		return(-1);
}

GLOBAL LONG
os_TaskInformation(USHORT Index, char *Buffer)
{
	DATA_ELEMENT TaskStatus;
	OPTION Prio, Preempt;
	UNSIGNED Count, TimeSlice, Size, MinStack;
	OS_HANDLE Handle;
	CHAR Name[NU_MAX_NAME];
	u_char *StackBase, *sp;
	USHORT Untouched;

	if (os_GetTaskEntry(Index, &Handle) < 0)
		return(OS_ERROR);
	if (NU_Task_Information(&TaskTable[Handle].TaskCB.TCB, Name,
				&TaskStatus, &Count, &Prio, &Preempt,
				&TimeSlice, (VOID **) &StackBase,
				&Size, &MinStack) != NU_SUCCESS)
		return(OS_ERROR);
	Untouched = 0;
	for (sp = StackBase; sp < StackBase + Size; sp++) {
		if (*sp != INITIAL_STACK_VALUE)
			break;
		Untouched++;
	}
	sprintf(Buffer,
	"Name:%s Stat:%d Count:%ld Prio:%d Stack:%lx Size:%ld Untouched:%d",
		Name, TaskStatus, Count, 255 - Prio, (ULONG) StackBase,
		(LONG) Size, Untouched);
	return(OS_OK);
}

GLOBAL LONG
os_StopTask(OS_HANDLE Caller, OS_HANDLE TaskHandle)
{
	if (NU_Suspend_Task(&TaskTable[TaskHandle].TaskCB.TCB) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}

GLOBAL LONG
os_StartTask(OS_HANDLE Caller, OS_HANDLE TaskHandle, ULONG Value)
{
	if (NU_Resume_Task(&TaskTable[TaskHandle].TaskCB.TCB) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}

GLOBAL LONG
os_ProInit(void)
{
	USHORT i;

	if (NU_Create_Semaphore(&ProSemCB, "PROSEM", 1, NU_PRIORITY)
			!= NU_SUCCESS)
		return(OS_ERROR);
	for (i = 1; i <= MaxTasks; i++)
		bzero(&TaskTable[i], sizeof(T_OS_TASK_TABLE_ENTRY));
	return(OS_OK);
}

GLOBAL unsigned char
os_GetTaskState(OS_HANDLE Caller, OS_HANDLE Handle)
{
	if (TaskTable[Handle].Name[0])
		return(TaskTable[Handle].TaskCB.TCB.tc_status);
	else
		return(255);
}

GLOBAL LONG
os_GetTaskHandle(OS_HANDLE Caller, char *Name, OS_HANDLE *TaskHandle)
{
	USHORT i;

	if (!Name) {
		OS_NU_TASK *os_nu_task = (OS_NU_TASK *) TCD_Current_Thread;

		if (os_nu_task && os_nu_task->magic_nr == OS_NU_TASK_MAGIC)
			*TaskHandle = os_nu_task->handle;
		else
			*TaskHandle = OS_NOTASK;
		return(OS_OK);
	}
	for (i = 1; i <= MaxTasks; i++)
		if (TaskTable[i].Name[0] &&
		    !strncmp(Name, TaskTable[i].Name, RESOURCE_NAMELEN - 1)) {
			*TaskHandle = i;
			return(OS_OK);
		}
	return(OS_ERROR);
}

GLOBAL LONG
os_GetTaskData(OS_HANDLE Handle, unsigned **tcb,
		u_char **stackbegin, u_char **stackend)
{
	NU_TASK *task;

	if (!TaskTable[Handle].Name[0])
		return(OS_ERROR);
	task = &TaskTable[Handle].TaskCB.TCB;
	*tcb = (unsigned *) task;
	*stackbegin = (u_char *) task->tc_stack_start;
	*stackend = (u_char *) task->tc_stack_end;
	return(OS_OK);
}

GLOBAL LONG
os_GetScheduleCount(OS_HANDLE task_handle, int *schedule_count)
{
	NU_TASK *task_cb;

	if (task_handle > MaxTasks)
		return(OS_ERROR);
	if (!TaskTable[task_handle].Name[0])
		return(OS_ERROR);
	if (task_handle == OS_NOTASK)
		task_cb = (NU_TASK *) TCD_Current_Thread;
	else
		task_cb = &TaskTable[task_handle].TaskCB.TCB;
	*schedule_count = task_cb->tc_scheduled;
	return(OS_OK);
}

GLOBAL LONG
os_DestroyTask(OS_HANDLE Caller, OS_HANDLE TaskHandle)
{
	STATUS sts;

	if (NU_Terminate_Task(&TaskTable[TaskHandle].TaskCB.TCB) != NU_SUCCESS)
		return(OS_ERROR);
	if (NU_Delete_Task(&TaskTable[TaskHandle].TaskCB.TCB) != NU_SUCCESS)
		return(OS_ERROR);
	if (os_DeallocateMemory(Caller, TaskTable[TaskHandle].Stack) != OS_OK)
		return(OS_ERROR);
	sts = NU_Obtain_Semaphore(&ProSemCB, NU_SUSPEND);
	TaskTable[TaskHandle].Name[0] = 0;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&ProSemCB);
	return(OS_OK);
}

GLOBAL LONG
os_CreateTask(OS_HANDLE Caller, char *Name,
		void (*TaskEntry)(OS_HANDLE, ULONG), ULONG StackSize,
		USHORT Priority, OS_HANDLE *TaskHandle, OS_HANDLE MemPoolHandle)
{
	STATUS sem_sts;
	USHORT Handle;
	u_char *stack;

	sem_sts = NU_Obtain_Semaphore(&ProSemCB, NU_SUSPEND);
	for (Handle = 1; Handle <= MaxTasks; Handle++)
		if (!TaskTable[Handle].Name[0])
			break;
	if (Handle > MaxTasks) {
return_error:	if (sem_sts == NU_SUCCESS)
			NU_Release_Semaphore(&ProSemCB);
		return(OS_ERROR);
	}
	if (os_AllocateMemory(Handle, &TaskTable[Handle].Stack, StackSize + 4,
				0, MemPoolHandle) != OS_OK)
		goto return_error;
	stack = (u_char *)TaskTable[Handle].Stack + 4;
	memset(stack, INITIAL_STACK_VALUE, StackSize);
	*TaskTable[Handle].Stack = GUARD_PATTERN;
	if (NU_Create_Task(&TaskTable[Handle].TaskCB.TCB, Name, os_TaskEntry,
				Handle, 0, stack, StackSize,
				255 - Priority, 0, NU_PREEMPT, NU_NO_START)
			!= NU_SUCCESS)
		goto return_error;
	strncpy(TaskTable[Handle].Name, Name, RESOURCE_NAMELEN);
	TaskTable[Handle].Name[RESOURCE_NAMELEN-1] = 0;
	TaskTable[Handle].TaskEntry = TaskEntry;
	TaskTable[Handle].TaskCB.magic_nr = OS_NU_TASK_MAGIC;
	TaskTable[Handle].TaskCB.handle = Handle;
	*TaskHandle = Handle;
	if (sem_sts == NU_SUCCESS)
		NU_Release_Semaphore(&ProSemCB);
	return(OS_OK);
}

GLOBAL LONG
os_ChangePreemption(char preempt)
{
	if (NU_Change_Preemption(preempt) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}
