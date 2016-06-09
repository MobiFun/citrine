/*
 * This C module is a reconstruction based on the disassembly of
 * os_com.obj in frame_na7_db_fl.lib from the Leonardo package.
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

extern T_OS_COM_TABLE_ENTRY ComTable[];

static NU_SEMAPHORE ComSemCB;

static int
os_GetQueueEntry(USHORT Index, OS_HANDLE *Handle)
{
	static USHORT Idx;

	if (Index == FIRST_ENTRY)
		Idx = 0;
	if (Index == FIRST_ENTRY || Index == NEXT_ENTRY) {
		while (++Idx <= MaxCommunications && !ComTable[Idx].Name[0])
			;
	} else
		Idx = Index;
	if (Idx <= MaxCommunications && ComTable[Idx].Name[0]) {
		*Handle = Idx;
		return(0);
	} else
		return(-1);
}

GLOBAL LONG
os_QueueInformation(USHORT Index, char *Buffer)
{
	OS_HANDLE Handle;
	T_OS_COM_TABLE_ENTRY *ent;
	UNSIGNED Used;
	OPTION SuspendType;
	UNSIGNED TasksWaiting;
	NU_TASK *First;
	CHAR Name[NU_MAX_NAME];

	if (os_GetQueueEntry(Index, &Handle) < 0)
		return(OS_ERROR);
	ent = ComTable + Handle;
	if (NU_Semaphore_Information(&ent->UsedSemCB, Name, &Used, &SuspendType,
					&TasksWaiting, &First) != NU_SUCCESS)
		return(OS_ERROR);
	sprintf(Buffer, "Name:%s Startadr:%lx Entries:%d Used:%ld MaxUsed:%d",
		ent->Name, (ULONG)ent->pQueueMemory, ent->Entries, (LONG)Used,
		ent->MaxUsed);
	return(OS_OK);
}

GLOBAL LONG
os_OpenQueue(OS_HANDLE TaskHandle, char *Name, OS_HANDLE *ComHandle)
{
	USHORT i;

	if (!Name)
		return(OS_ERROR);
	for (i = 1; i <= MaxCommunications; i++)
		if (ComTable[i].Name[0] &&
		    !strncmp(ComTable[i].Name, Name, RESOURCE_NAMELEN - 1)) {
			*ComHandle = i;
			return(OS_OK);
		}
	return(OS_ERROR);
}

GLOBAL LONG
os_GetQueueState(OS_HANDLE Caller, OS_HANDLE Handle, ULONG *Used, ULONG *Free)
{
	if (ComTable[Handle].Name[0]) {
		*Used = ComTable[Handle].UsedSemCB.sm_semaphore_count;
		*Free = ComTable[Handle].FreeSemCB.sm_semaphore_count;
		return(OS_OK);
	} else
		return(OS_ERROR);
}

GLOBAL LONG
os_GetQueueName(OS_HANDLE Caller, OS_HANDLE ComHandle, char *Name)
{
	if (ComHandle > MaxCommunications)
		return(OS_ERROR);
	if (!ComTable[ComHandle].Name[0])
		return(OS_ERROR);
	strcpy(Name, ComTable[ComHandle].Name);
	return(OS_OK);
}

GLOBAL LONG
os_GetQueueHandle(OS_HANDLE Caller, char *Name, OS_HANDLE *ComHandle)
{
	USHORT i;

	for (i = 1; i <= MaxCommunications; i++)
		if (ComTable[i].Name[0] &&
		    !strncmp(Name, ComTable[i].Name, RESOURCE_NAMELEN - 1)) {
			*ComHandle = i;
			return(OS_OK);
		}
	return(OS_ERROR);
}

GLOBAL LONG
os_GetQueueData(OS_HANDLE Caller, OS_HANDLE Handle, USHORT Index, USHORT *Type,
		ULONG *opc, ULONG *ptr, ULONG *time)
{
	static USHORT entry;
	static T_QDATA_ELEMENT *p;

	if (!ComTable[Handle].Name[0])
		return(OS_ERROR);
	if (Index == FIRST_ENTRY) {
		*Type = ComTable[Handle].current_msg.type;
		*opc  = ComTable[Handle].current_msg.opc;
		*time = ComTable[Handle].current_msg.time;
		*ptr  = (ULONG) ComTable[Handle].current_msg.ptr;
		p = ComTable[Handle].pQueueMemory;
		entry = 0;
		return(OS_OK);
	}
	if (entry >= ComTable[Handle].Entries)
		return(OS_ERROR);
	entry++;
	*Type = p->Data.data16;
	*ptr  = (ULONG) p->Data.ptr;
	*opc  = p->Data.data32;
	*time = p->Data.time;
	p++;
	return(OS_OK);
}

GLOBAL unsigned char *
os_FindSuspendingQueue(unsigned int *tcb)
{
	USHORT i;
	SM_SUSPEND *susp, *susp2;

	for (i = 1; i <= MaxCommunications; i++) {
		if (!ComTable[i].Name[0])
			continue;
		if (susp = ComTable[i].FreeSemCB.sm_suspension_list) {
			if (susp->sm_suspended_task == (NU_TASK*)tcb)
				return(ComTable[i].FreeSemCB.sm_name + 1);
			susp = (SM_SUSPEND *) susp->sm_suspend_link.cs_next;
			for (susp2 = susp; ; ) {
				if (susp2->sm_suspended_task == (NU_TASK*)tcb)
					return(ComTable[i].FreeSemCB.sm_name+1);
				susp2 = (SM_SUSPEND *)
						susp2->sm_suspend_link.cs_next;
				if (susp2 == susp)
					break;
			}
		}
		if (susp = ComTable[i].UsedSemCB.sm_suspension_list) {
			if (susp->sm_suspended_task == (NU_TASK*)tcb)
				return(ComTable[i].UsedSemCB.sm_name + 1);
			susp = (SM_SUSPEND *) susp->sm_suspend_link.cs_next;
			for (susp2 = susp; ; ) {
				if (susp2->sm_suspended_task == (NU_TASK*)tcb)
					return(ComTable[i].UsedSemCB.sm_name+1);
				susp2 = (SM_SUSPEND *)
						susp2->sm_suspend_link.cs_next;
				if (susp2 == susp)
					break;
			}
		}
	}
	return(0);
}

GLOBAL LONG
os_DestroyQueue(OS_HANDLE TaskHandle, OS_HANDLE ComHandle)
{
	STATUS sts;

	sts = NU_Obtain_Semaphore(&ComSemCB, NU_SUSPEND);
	if (NU_Delete_Semaphore(&ComTable[ComHandle].FreeSemCB) != NU_SUCCESS) {
return_error:	if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&ComSemCB);
		return(OS_ERROR);
	}
	if (NU_Delete_Semaphore(&ComTable[ComHandle].UsedSemCB) != NU_SUCCESS)
		goto return_error;
	if (os_DeallocateMemory(TaskHandle, ComTable[ComHandle].QueueData)
			== OS_ERROR)
		goto return_error;
	ComTable[ComHandle].Name[0] = 0;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&ComSemCB);
	return(OS_OK);
}

static short
InitQueueMemory(OS_HANDLE TaskHandle, OS_HANDLE ComHandle, USHORT Entries,
		OS_HANDLE MemPoolHandle)
{
	T_QDATA_ELEMENT *pElem;
	OS_QDATA **ptrs;
	USHORT i;

	if (os_AllocateMemory(TaskHandle, &ComTable[ComHandle].QueueData,
				sizeof(T_QDATA_ELEMENT) * Entries +
					sizeof(OS_QDATA *) * (Entries + 1)
						* OS_MAX_PRIORITY,
				0, MemPoolHandle) == OS_TIMEOUT)
		return(OS_ERROR);
	pElem = (T_QDATA_ELEMENT *) ComTable[ComHandle].QueueData;
	ComTable[ComHandle].pQueueMemory = pElem;
	ComTable[ComHandle].pFreeElement = pElem;
	for (i = 0; i < Entries; i++) {
		if (i < Entries - 1)
			pElem->pNext = pElem + 1;
		else
			pElem->pNext = 0;
		pElem++;
	}
	ptrs = (OS_QDATA **) pElem;
	for (i = 0; i < OS_MAX_PRIORITY; i++) {
		ComTable[ComHandle].Queue[i].pStart = ptrs;
		ComTable[ComHandle].Queue[i].pRead = ptrs;
		ComTable[ComHandle].Queue[i].pWrite = ptrs;
		ptrs += Entries + 1;
	}
	return(OS_OK);
}

GLOBAL LONG
os_CreateQueue(OS_HANDLE TaskHandle, OS_HANDLE ComHandle, char *Name,
		USHORT Entries, OS_HANDLE *ActHandle, OS_HANDLE MemPoolHandle)
{
	STATUS sts;
	OS_HANDLE i;
	char Buffer[RESOURCE_NAMELEN + 1];

	if (os_OpenQueue(TaskHandle, Name, ActHandle) == OS_OK)
		return(OS_ERROR);
	if (!Entries)
		return(OS_ERROR);
	sts = NU_Obtain_Semaphore(&ComSemCB, NU_SUSPEND);
	if (!ComHandle) {
		for (i = 1; i <= MaxCommunications; i++)
			if (!ComTable[i].Name[0])
				goto good_slot;
release_sem_error:
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&ComSemCB);
		return(OS_ERROR);
	} else {
		i = ComHandle;
		if (i > MaxCommunications)
			goto release_sem_error;
		if (ComTable[i].Name[0])
			goto release_sem_error;
	}
good_slot:
	if (InitQueueMemory(TaskHandle, i, Entries, MemPoolHandle) == OS_ERROR)
		goto release_sem_error;
	strncpy(Buffer + 1, Name, RESOURCE_NAMELEN - 1);
	Buffer[RESOURCE_NAMELEN] = 0;
	Buffer[0] = 'U';
	if (NU_Create_Semaphore(&ComTable[i].UsedSemCB, Buffer, 0, NU_PRIORITY)
			!= NU_SUCCESS)
		goto release_sem_error;
	Buffer[0] = 'F';
	if (NU_Create_Semaphore(&ComTable[i].FreeSemCB, Buffer, Entries,
				NU_PRIORITY) != NU_SUCCESS)
		goto release_sem_error;
	strncpy(ComTable[i].Name, Name, RESOURCE_NAMELEN);
	ComTable[i].Name[RESOURCE_NAMELEN-1] = 0;
	*ActHandle = i;
	ComTable[i].Entries = Entries;
	ComTable[i].MaxUsed = 0;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&ComSemCB);
	return(OS_OK);
}

GLOBAL LONG
os_ComInit(void)
{
	USHORT i;

	if (NU_Create_Semaphore(&ComSemCB, "COMSEM", 1, NU_PRIORITY)
			!= NU_SUCCESS)
		return(OS_ERROR);
	for (i = 1; i <= MaxCommunications; i++)
		bzero(&ComTable[i], sizeof(T_OS_COM_TABLE_ENTRY));
	return(OS_OK);
}

GLOBAL LONG
os_CloseQueue(OS_HANDLE TaskHandle, OS_HANDLE ComHandle)
{
	return(OS_OK);
}
