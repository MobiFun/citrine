/*
 * This C module is a reconstruction based on the disassembly of
 * os_sem.obj in frame_na7_db_fl.lib from the Leonardo package.
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

static NU_SEMAPHORE SemSemCB;

static int
os_GetSemaphoreEntry(USHORT Index, OS_HANDLE *Handle)
{
	static USHORT Idx;

	if (Index == FIRST_ENTRY)
		Idx = 0;
	if (Index == FIRST_ENTRY || Index == NEXT_ENTRY) {
		for (;;) {
			Idx++;
			if (Idx > MaxSemaphores)
				return(OS_ERROR);
			if (SemTable[Idx].Name[0])
				break;
		}
	} else
		Idx = Index;
	if (Idx > MaxSemaphores)
		return(OS_ERROR);
	if (SemTable[Idx].Name[0]) {
		*Handle = Idx;
		return(OS_OK);
	} else
		return(OS_ERROR);
}

GLOBAL LONG
os_SemaphoreInformation(USHORT Index, char *Buffer)
{
	OS_HANDLE Handle;
	OPTION SuspendType;
	UNSIGNED Current, TasksWaiting;
	NU_TASK *First;
	CHAR Name[NU_MAX_NAME];

	if (os_GetSemaphoreEntry(Index, &Handle) < 0)
		return(OS_ERROR);
	if (NU_Semaphore_Information(&SemTable[Handle].SemCB, Name, &Current,
				     &SuspendType, &TasksWaiting, &First)
			!= NU_SUCCESS)
		return(OS_ERROR);
	sprintf(Buffer, "Semname:%s Count:%ld Suspend:%d Waiting:%ld", Name,
		Current, SuspendType, TasksWaiting);
	return(OS_OK);
}

GLOBAL LONG
os_SemInit(void)
{
	USHORT i;

	if (NU_Create_Semaphore(&SemSemCB, "SEMSEM", 1, NU_PRIORITY)
			!= NU_SUCCESS)
		return(OS_ERROR);
	for (i = 1; i <= MaxSemaphores; i++)
		bzero(&SemTable[i], sizeof(T_OS_SEM_TABLE_ENTRY));
	return(OS_OK);
}

GLOBAL LONG
os_ResetSemaphore(OS_HANDLE TaskHandle, OS_HANDLE SemHandle,
		  USHORT init_counter)
{
	STATUS sts;

	sts = NU_Obtain_Semaphore(&SemSemCB, NU_SUSPEND);
	if (!SemTable[SemHandle].Name[0]) {
error_out:	if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&SemSemCB);
		return(OS_ERROR);
	}
	if (NU_Reset_Semaphore(&SemTable[SemHandle].SemCB, init_counter)
			!= NU_SUCCESS)
		goto error_out;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&SemSemCB);
	return(OS_OK);
}

GLOBAL LONG
os_QuerySemaphore(OS_HANDLE TaskHandle, OS_HANDLE SemHandle, USHORT *Count)
{
	OPTION SuspendType;
	UNSIGNED SemCount, TasksWaiting;
	NU_TASK *First;
	CHAR Name[NU_MAX_NAME];

	if (NU_Semaphore_Information(&SemTable[SemHandle].SemCB, Name,
				     &SemCount, &SuspendType, &TasksWaiting,
				     &First) != NU_SUCCESS)
		return(OS_ERROR);
	*Count = SemCount;
	return(OS_OK);
}

GLOBAL LONG
os_OpenSemaphore(OS_HANDLE TaskHandle, char *Name, OS_HANDLE *SemHandle)
{
	USHORT i;

	for (i = 1; i <= MaxSemaphores; i++) {
		if (!SemTable[i].Name[0])
			continue;
		if (strncmp(Name, SemTable[i].Name, RESOURCE_NAMELEN-1))
			continue;
		*SemHandle = i;
		return(OS_OK);
	}
	return(OS_ERROR);
}

GLOBAL unsigned char *
os_FindSuspendingSema(unsigned int *tcb)
{
	USHORT i;
	SM_SUSPEND *susp, *susp_loopchk;

	for (i = 1; i <= MaxSemaphores; i++) {
		if (!SemTable[i].Name[0])
			continue;
		susp = SemTable[i].SemCB.sm_suspension_list;
		if (!susp)
			continue;
		if (susp->sm_suspended_task == (NU_TASK *)tcb)
			return(SemTable[i].SemCB.sm_name);
		susp = (SM_SUSPEND *)susp->sm_suspend_link.cs_next;
		for (susp_loopchk = susp; susp != susp_loopchk;
		     susp = (SM_SUSPEND *)susp->sm_suspend_link.cs_next)
			if (susp->sm_suspended_task == (NU_TASK *)tcb)
				return(SemTable[i].SemCB.sm_name);
	}
	return(0);
}

GLOBAL LONG
os_DestroySemaphore(OS_HANDLE TaskHandle, OS_HANDLE SemHandle)
{
	STATUS sts;

	sts = NU_Obtain_Semaphore(&SemSemCB, NU_SUSPEND);
	if (!SemTable[SemHandle].Name[0]) {
error_out:	if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&SemSemCB);
		return(OS_ERROR);
	}
	if (NU_Delete_Semaphore(&SemTable[SemHandle].SemCB) != NU_SUCCESS)
		goto error_out;
	SemTable[SemHandle].Name[0] = 0;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&SemSemCB);
	return(OS_OK);
}

GLOBAL LONG
os_CreateSemaphore(OS_HANDLE TaskHandle, char *Name, USHORT Count,
		   OS_HANDLE *SemHandle, OS_HANDLE MemPoolHandle)
{
	USHORT i;
	STATUS sts;

	if (os_OpenSemaphore(TaskHandle, Name, SemHandle) == OS_OK)
		return(OS_ERROR);
	sts = NU_Obtain_Semaphore(&SemSemCB, NU_SUSPEND);
	for (i = 1; i <= MaxSemaphores; i++) {
		if (SemTable[i].Name[0])
			continue;
		if (NU_Create_Semaphore(&SemTable[i].SemCB, Name, Count,
					NU_PRIORITY) != NU_SUCCESS)
			break;
		strncpy(SemTable[i].Name, Name, RESOURCE_NAMELEN);
		SemTable[i].Name[RESOURCE_NAMELEN-1] = 0;
		*SemHandle = i;
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&SemSemCB);
		return(OS_OK);
	}
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&SemSemCB);
	return(OS_ERROR);
}

GLOBAL LONG
os_CloseSemaphore(OS_HANDLE TaskHandle, OS_HANDLE SemHandle)
{
	return(OS_OK);
}
