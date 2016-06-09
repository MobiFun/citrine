/*
 * This C module is a reconstruction based on the disassembly of
 * os_evt.obj in frame_na7_db_fl.lib from the Leonardo package.
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

extern T_OS_EVTGRP_TABLE_ENTRY EvtGrpTable[];

GLOBAL LONG
os_SetEvents(OS_HANDLE evt_grp_handle, unsigned event_flags)
{
	if (!EvtGrpTable[evt_grp_handle].Name[0])
		return(OS_ERROR);
	if (NU_Set_Events(&EvtGrpTable[evt_grp_handle].EvtGrp, event_flags,
				NU_OR) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}

GLOBAL LONG
os_RetrieveEvents(OS_HANDLE evt_grp_handle, unsigned event_flags, char option,
		  unsigned *retrieved_events, unsigned suspend)
{
	if (!EvtGrpTable[evt_grp_handle].Name[0])
		return(OS_ERROR);
	if (NU_Retrieve_Events(&EvtGrpTable[evt_grp_handle].EvtGrp,
				event_flags, option, retrieved_events, suspend)
			== NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}

GLOBAL LONG
os_GetEventGroupHandle(char *evt_grp_name, OS_HANDLE *evt_grp_handle)
{
	int idx;

	for (idx = 0; idx <= MaxEventGroups; idx++) {
		if (!EvtGrpTable[idx].Name[0])
			break;
		if (!strncmp(EvtGrpTable[idx].Name, evt_grp_name,
				RESOURCE_NAMELEN))
			break;
	}
	if (idx > MaxEventGroups || !EvtGrpTable[idx].Name[0]) {
		*evt_grp_handle = -1;
		return(OS_ERROR);
	}
	*evt_grp_handle = idx;
	return(OS_OK);
}

GLOBAL LONG
os_EventGroupInformation(OS_HANDLE evt_grp_handle, char *Name,
			 unsigned *mask_evt, unsigned *tasks_waiting,
			 OS_HANDLE *first_task)
{
	if (!EvtGrpTable[evt_grp_handle].Name[0])
		return(OS_ERROR);
	if (NU_Event_Group_Information(&EvtGrpTable[evt_grp_handle].EvtGrp,
					Name, mask_evt, tasks_waiting,
					first_task) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}
