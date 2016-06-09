/*
 * This C module is a reconstruction based on the disassembly of
 * os_com.obj in frame_na7_db_ir.lib from the Leonardo package.
 */

/* set of included headers from COFF symtab: */
#include <stdio.h>
#include <string.h>
#include "gpfconf.h"	/* FreeCalypso addition */
#include "../../nucleus/nucleus.h"
#include "../../nucleus/tc_extr.h"	/* not seen in original, but needed */
#include "typedefs.h"
#include "os.h"
#include "gdi.h"
#include "os_types.h"
#include "os_glob.h"

extern T_OS_COM_TABLE_ENTRY ComTable[];
extern unsigned os_tick_to_time_multiplier;

extern int ObtainSemaphoreCB(NU_SEMAPHORE *SemCB, ULONG Timeout,
				USHORT wait_check);
extern int ReleaseSemaphoreCB(NU_SEMAPHORE *SemCB);

GLOBAL LONG
os_SendToQueue(OS_HANDLE TaskHandle, OS_HANDLE ComHandle, USHORT Priority,
		ULONG Suspend, OS_QDATA *Msg)
{
	T_OS_COM_TABLE_ENTRY *pTable;
	T_QDATA_ELEMENT *elem;
	T_QUEUE *queue;
	int ret;
	NU_SEMAPHORE *CBPtr;
	USHORT watmark;

	if (ComHandle <= 0 || ComHandle > MaxCommunications)
		return(OS_INVALID_QUEUE);
	pTable = ComTable + ComHandle;
	if (!pTable->Name[0])
		return(OS_INVALID_QUEUE);
	CBPtr = &pTable->FreeSemCB;
	ret = ObtainSemaphoreCB(CBPtr, Suspend, 1);
	if (ret == OS_ERROR || ret == OS_TIMEOUT)
		return(ret);
	TCT_System_Protect();
	elem = pTable->pFreeElement;
	pTable->pFreeElement = elem->pNext;
	bcopy(Msg, &elem->Data, sizeof(OS_QDATA));
	queue = &pTable->Queue[Priority - OS_MIN_PRIORITY];
	*queue->pWrite++ = &elem->Data;
	if (queue->pWrite - queue->pStart >= pTable->Entries + 1)
		queue->pWrite = queue->pStart;
	watmark = pTable->Entries - CBPtr->sm_semaphore_count;
	if (pTable->MaxUsed < watmark)
		pTable->MaxUsed = watmark;
	TCT_System_Unprotect();
	ReleaseSemaphoreCB(&pTable->UsedSemCB);
	return(ret);
}

GLOBAL LONG
os_ReceiveFromQueue(OS_HANDLE TaskHandle, OS_HANDLE ComHandle,
			OS_QDATA *Msg, ULONG Timeout)
{
	T_QDATA_ELEMENT *pElem;
	UNSIGNED c_time;
	int ret;
	USHORT i;
	T_QUEUE *pQueue;
	T_OS_COM_TABLE_ENTRY *pTable;

	pTable = ComTable + ComHandle;
	if (!pTable->Name[0])
		return(OS_ERROR);
	pTable->current_msg.type = 0;
	ret = ObtainSemaphoreCB(&pTable->UsedSemCB, Timeout, 0);
	if (ret == OS_ERROR || ret == OS_TIMEOUT)
		return(ret);
	TCT_System_Protect();
	for (i = OS_MAX_PRIORITY; i >= OS_MIN_PRIORITY; i--) {
		pQueue = &pTable->Queue[i - OS_MIN_PRIORITY];
		if (pQueue->pWrite != pQueue->pRead)
			break;
	}
	if (i < OS_MIN_PRIORITY) {
		TCT_System_Unprotect();
		ReleaseSemaphoreCB(&pTable->FreeSemCB);
		return(OS_ERROR);
	}
	bcopy(*pQueue->pRead, Msg, sizeof(OS_QDATA));
	pElem = (T_QDATA_ELEMENT *)*pQueue->pRead++;
	pElem->Data.data16 = 0;
	pElem->pNext = pTable->pFreeElement;
	pTable->pFreeElement = pElem;
	if (pQueue->pRead - pQueue->pStart >= pTable->Entries + 1)
		pQueue->pRead = pQueue->pStart;
	pTable->current_msg.type = Msg->data16;
	pTable->current_msg.opc = Msg->data32;
	c_time = NU_Retrieve_Clock();
	pTable->current_msg.time = SYSTEM_TICKS_TO_TIME(c_time);
	pTable->current_msg.ptr = Msg->ptr;
	TCT_System_Unprotect();
	ReleaseSemaphoreCB(&pTable->FreeSemCB);
	return(OS_OK);
}
