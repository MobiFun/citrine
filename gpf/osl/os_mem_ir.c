/*
 * This C module is a reconstruction based on the disassembly of
 * os_mem.obj in frame_na7_db_ir.lib from the Leonardo package.
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

extern T_OS_PART_GRP_TABLE_ENTRY PartGrpTable[];
extern T_OS_POOL_BORDER PoolBorder[];

GLOBAL LONG
os_is_valid_partition(T_VOID_STRUCT *Buffer)
{
	int i;

	for (i = 0; i <= MaxPoolGroups; i++) {
		if (PoolBorder[i].End == 0)
			return(OS_ERROR);
		if ((char *)Buffer < PoolBorder[i].Start)
			continue;
		if ((char *)Buffer >= PoolBorder[i].End)
			continue;
		return(OS_OK);
	}
	return(OS_ERROR);
}

GLOBAL LONG
os_PartitionCheck(ULONG *ptr)
{
	PM_HEADER *phdr;
	PM_PCB *pool;

	phdr = (PM_HEADER *)(ptr - 2);
	if (phdr->pm_next_available)
		return(OS_PARTITION_FREE);
	pool = phdr->pm_partition_pool;
	if (ptr[(pool->pm_partition_size - 4) >> 2] == GUARD_PATTERN)
		return(OS_OK);
	else
		return(OS_PARTITION_GUARD_PATTERN_DESTROYED);
}

GLOBAL LONG
os_DeallocatePartition(OS_HANDLE TaskHandle, T_VOID_STRUCT *Buffer)
{
	if (os_is_valid_partition(Buffer) != OS_OK)
		return(OS_ERROR);
	if (NU_Deallocate_Partition(Buffer) != NU_SUCCESS)
		return(OS_ERROR);
	return(OS_OK);
}

GLOBAL LONG
os_AllocatePartition(OS_HANDLE TaskHandle, T_VOID_STRUCT **Buffer, ULONG Size,
		     ULONG Suspend, OS_HANDLE GroupHandle)
{
	T_OS_PART_POOL *pool, *requested_pool;
	ULONG nu_suspend;
	STATUS sts;
	int ret;

	for (pool = PartGrpTable[GroupHandle].grp_head; pool;
	     pool = pool->next)
		if (Size <= pool->size)
			break;
	if (!pool)
		return(OS_ERROR);
	requested_pool = pool;
	ret = OS_OK;
	nu_suspend = NU_NO_SUSPEND;
try_alloc:
	sts = NU_Allocate_Partition(&pool->pcb, (VOID **) Buffer, nu_suspend);
	switch (sts) {
	case NU_SUCCESS:
		return(ret);
	case NU_TIMEOUT:
	case NU_INVALID_SUSPEND:
		*Buffer = 0;
		return(OS_TIMEOUT);
	case NU_NO_PARTITION:
		pool = pool->next;
		if (pool) {
			ret = OS_ALLOCATED_BIGGER;
			goto try_alloc;
		}
		pool = requested_pool;
		if (Suspend) {
			nu_suspend = Suspend;
			ret = OS_WAITED;
			goto try_alloc;
		}
		return(OS_TIMEOUT);
	default:
		*Buffer = 0;
		return(OS_ERROR);
	}
}
