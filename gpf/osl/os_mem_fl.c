/*
 * This C module is a reconstruction based on the disassembly of
 * os_mem.obj in frame_na7_db_fl.lib from the Leonardo package.
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
extern T_OS_MEM_POOL_TABLE_ENTRY MemPoolTable[];
extern T_OS_POOL_BORDER PoolBorder[];

OS_HANDLE os_ext_pool_handle;
OS_HANDLE os_int_pool_handle;

static USHORT NumOfMemoryPools;
static NU_SEMAPHORE MemSemCB;
static NU_MEMORY_POOL mem_pool_head;

GLOBAL LONG
os_SetPoolHandles(OS_HANDLE ext_pool_handle, OS_HANDLE int_pool_handle)
{
	os_ext_pool_handle = ext_pool_handle;
	os_int_pool_handle = int_pool_handle;
	return(OS_OK);
}

static int
os_GetPartitionPoolEntry(USHORT Index, T_OS_PART_POOL **pool)
{
	static T_OS_PART_POOL *part_pool;
	static int grp_hndl;

	switch (Index) {
	case FIRST_ENTRY:
		grp_hndl = 0;
		*pool = part_pool = PartGrpTable[0].grp_head;
		return(OS_OK);
	case NEXT_ENTRY:
		if (part_pool->next) {
			*pool = part_pool = part_pool->next;
			return(OS_OK);
		}
		grp_hndl++;
		if (PartGrpTable[grp_hndl].grp_head) {
			*pool = part_pool = PartGrpTable[grp_hndl].grp_head;
			return(OS_OK);
		} else
			return(OS_ERROR);
	default:
		return(OS_ERROR);
	}
}

GLOBAL LONG
os_PartitionInformation(USHORT Handle, char *Buffer)
{
	T_OS_PART_POOL *pool;
	OPTION SuspendType;
	UNSIGNED PoolSize;
	UNSIGNED PartitionSize;
	UNSIGNED Available;
	UNSIGNED Waiting;
	UNSIGNED Allocated;
	VOID *pStartAddress;
	NU_TASK *First;
	CHAR Name[NU_MAX_NAME];

	if (os_GetPartitionPoolEntry(Handle, &pool) == OS_ERROR)
		return(OS_ERROR);
	if (NU_Partition_Pool_Information(&pool->pcb, Name, &pStartAddress,
					  &PoolSize, &PartitionSize, &Available,
					  &Allocated, &SuspendType, &Waiting,
					  &First)
			!= NU_SUCCESS)
		return(OS_ERROR);
	sprintf(Buffer,
		"Name:%s Addr:%lx PoolSize:%ld PartSize:%ld Free:%ld Used:%ld",
		Name, (UNSIGNED) pStartAddress, PoolSize, PartitionSize,
		Available, Allocated);
	return(OS_OK);
}

static int
os_GetMemoryPoolEntry(USHORT Index, OS_HANDLE *Handle)
{
	static USHORT Idx;

	switch (Index) {
	case FIRST_ENTRY:
		Idx = 0;
		break;
	case NEXT_ENTRY:
		Idx++;
		break;
	default:
		Idx = Index;
	}
	if (Idx == NumOfMemoryPools)
		return(OS_ERROR);
	*Handle = Idx;
	return(OS_OK);
}

GLOBAL LONG
os_MemoryInformation(USHORT Index, char *Buffer)
{
	OS_HANDLE Handle;
	OPTION SuspendType;
	UNSIGNED Size, Min, Available, Waiting;
	VOID *pStartAddress;
	NU_TASK *First;
	CHAR Name[NU_MAX_NAME];

	if (os_GetMemoryPoolEntry(Index, &Handle) == OS_ERROR)
		return(OS_ERROR);
	if (NU_Memory_Pool_Information(MemPoolTable[Handle].pcb, Name,
					&pStartAddress, &Size, &Min,
					&Available, &SuspendType, &Waiting,
					&First)
			!= NU_SUCCESS)
		return(OS_ERROR);
	sprintf(Buffer,
		"Heapname:%s Addr:%lx Size:%ld Min:%ld Free:%ld Suspend:%d",
		Name, (UNSIGNED) pStartAddress, Size, Min, Available,
		SuspendType);
	return(OS_OK);
}

GLOBAL LONG
os_MemInit(void)
{
	USHORT i;

	if (NU_Create_Semaphore(&MemSemCB, "MEMSEM", 1, NU_PRIORITY)
			!= NU_SUCCESS)
		return(OS_ERROR);
	for (i = 0; i <= MaxPoolGroups; i++) {
		PoolBorder[i].Start = (char *)0xFFFFFFFF;
		PoolBorder[i].End   = (char *)0;
		PartGrpTable[i].grp_head = 0;
		PartGrpTable[i].name[0] = 0;
	}
	MemPoolTable[0].pcb = &mem_pool_head;
	return(OS_OK);
}

void
os_InitPartitionCheck(T_OS_PART_POOL *pool)
{
	unsigned **Buffer, offset;
	USHORT i, k;

	NU_Allocate_Memory(MemPoolTable[0].pcb, (VOID **) &Buffer,
			   pool->pcb.pm_available * sizeof(unsigned *),
			   NU_NO_SUSPEND);
	offset = pool->pcb.pm_partition_size / sizeof(unsigned) - 1;
	for (i = 0; ; i++) {
		if (NU_Allocate_Partition(&pool->pcb, (VOID **)(Buffer + i),
					  NU_NO_SUSPEND)
				!= NU_SUCCESS)
			break;
		Buffer[i][offset] = GUARD_PATTERN;
	}
	for (k = 0; k < i; k++)
		if (NU_Deallocate_Partition(Buffer[k]) != NU_SUCCESS)
			break;
	NU_Deallocate_Memory(Buffer);
}

GLOBAL const ULONG *
os_GetPrimpoolCB(int grp, int id)
{
	T_OS_PART_POOL *pool;
	int i;

	pool = PartGrpTable[grp].grp_head;
	if (!pool)
		return(0);
	if (id < 0)
		return(0);
	for (i = 0; i < id; i++) {
		pool = pool->next;
		if (!pool)
			return(0);
	}
	return (const ULONG *) &pool->pcb;
}

GLOBAL LONG
os_GetPartitionPoolStatus(ULONG size, OS_HANDLE gr_hndl,
			  USHORT *m_free, USHORT *m_alloc)
{
	T_OS_PART_POOL *pool;
	UNSIGNED dummy, allocated, available;
	CHAR Name[NU_MAX_NAME];

	for (pool = PartGrpTable[gr_hndl].grp_head; pool; pool = pool->next) {
		if (!size)
			break;
		if (size > pool->size)
			continue;
		if (NU_Partition_Pool_Information(&pool->pcb, Name,
						  (VOID **)&dummy, &dummy,
						  &dummy, &available,
						  &allocated, (OPTION *)&dummy,
						  &dummy, (NU_TASK **)&dummy)
				!= NU_SUCCESS)
			break;
		*m_alloc = allocated;
		*m_free = available;
		return(OS_OK);
	}
	*m_alloc = 0;
	*m_free = 0;
	return(OS_ERROR);
}

GLOBAL LONG
os_GetPartitionGroupHandle(OS_HANDLE Caller, char *Name, OS_HANDLE *GroupHandle)
{
	int i;

	for (i = 0; i <= MaxPoolGroups; i++) {
		if (!PartGrpTable[i].grp_head)
			continue;
		if (strncmp(Name, PartGrpTable[i].name, RESOURCE_NAMELEN-1))
			continue;
		*GroupHandle = i;
		return(OS_OK);
	}
	return(OS_ERROR);
}

GLOBAL LONG
os_DeallocateMemory(OS_HANDLE TaskHandle, T_VOID_STRUCT *Buffer)
{
	if (NU_Deallocate_Memory(Buffer) == NU_SUCCESS)
		return(OS_OK);
	else
		return(OS_ERROR);
}

GLOBAL LONG
os_AllocateMemory(OS_HANDLE TaskHandle, T_VOID_STRUCT **Buffer, ULONG Size,
		  ULONG Suspend, OS_HANDLE PoolHandle)
{
	int ret, sts;

	if (Suspend == 0xFFFFFFFF)
		Suspend = 1;
	ret = OS_OK;
	for (;;) {
		sts = NU_Allocate_Memory(MemPoolTable[PoolHandle].pcb, Buffer,
					 Size, Suspend);
		switch (sts) {
		case NU_SUCCESS:
			return(ret);
		case NU_INVALID_SUSPEND:
			Suspend = 0;
			continue;
		case NU_NO_MEMORY:
		case NU_TIMEOUT:
			if (Suspend == 1) {
				Suspend = 0xFFFFFFFF;
				ret = OS_WAITED;
				continue;
			} else {
				*Buffer = 0;
				return(OS_TIMEOUT);
			}
		default:
			/*
			 * Disassembly reveals that the original code
			 * has an endless loop here, the equivalent
			 * of continue.  My guess is that they simply
			 * forgot the default case, and so control
			 * falls onto the closing brace of the switch
			 * and then onto the closing brace of the for
			 * loop.  But I prefer better error handling,
			 * hence the present addition. - Space Falcon
			 */
			*Buffer = 0;
			return(OS_ERROR);
		}
	}
}

GLOBAL LONG
os_CreatePartitionPool(OS_HANDLE TaskHandle, char *GroupName, void *Addr,
			USHORT Num, ULONG Size, OS_HANDLE *GroupHandle)
{
	STATUS sts;
	T_OS_PART_POOL *part_group_head, *opool, *npool;
	USHORT part_group;
	USHORT i, j;
	char PoolName[8], *cp;

	sts = NU_Obtain_Semaphore(&MemSemCB, NU_SUSPEND);
	j = 0;
	part_group_head = 0;
	for (i = 0; i <= MaxPoolGroups; i++) {
		if (!PartGrpTable[i].grp_head || !PartGrpTable[i].name[0])
			break;
		if (!strncmp(GroupName, PartGrpTable[i].name,
			     RESOURCE_NAMELEN - 1)) {
			part_group_head = PartGrpTable[i].grp_head;
			opool = part_group_head;
			j++;
			while (opool->next) {
				opool = opool->next;
				j++;
			}
			break;
		}
	}
	/*
	 * This error check logic has been modified from the original
	 * faithful reconstruction by Space Falcon.  In the original code
	 * if MaxPoolGroups had been reached and the for loop above
	 * never broke, the code would proceed to overwrite pool #0
	 * instead of catching the error.
	 */
	if (i > MaxPoolGroups) {
release_sem_return_err:
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&MemSemCB);
		return(OS_ERROR);
	}
	part_group = i;
	if (!part_group_head) {
		strncpy(PartGrpTable[part_group].name, GroupName,
			RESOURCE_NAMELEN);
		PartGrpTable[part_group].name[RESOURCE_NAMELEN-1] = 0;
	}
	if (os_AllocateMemory(OS_NOTASK, (T_VOID_STRUCT **) &npool,
			      sizeof(T_OS_PART_POOL), OS_NO_SUSPEND,
			      os_ext_pool_handle) != OS_OK)
		goto release_sem_return_err;
	sprintf(PoolName, "POOL%1d%1d", part_group + 1, j);
	Size &= ~3;
	npool->pool_mem = Addr;
	/*
	 * FreeCalypso: we need to bzero the PM_PCB before calling
	 * NU_Create_Partition_Pool() to prevent the possibility of
	 * Nucleus error checker failing the call because the
	 * signature word happens to be there already.
	 */
	bzero(&npool->pcb, sizeof(NU_PARTITION_POOL));
	if (NU_Create_Partition_Pool(&npool->pcb, PoolName, npool->pool_mem,
				     POOL_SIZE(Num, Size), Size + 4, NU_FIFO)
			!= NU_SUCCESS)
		goto release_sem_return_err;
	if (!part_group_head)
		PartGrpTable[part_group].grp_head = npool;
	else
		opool->next = npool;
	npool->size = Size;
	npool->next = 0;
	*GroupHandle = part_group;
	cp = (char *) npool->pool_mem;
	if (PoolBorder[part_group].Start >= cp)
		PoolBorder[part_group].Start = cp;
	cp += POOL_SIZE(Num, Size);
	if (PoolBorder[part_group].End < cp)
		PoolBorder[part_group].End = cp;
	os_InitPartitionCheck(npool);
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&MemSemCB);
	return(OS_OK);
}

GLOBAL LONG
os_CreatePartitionPool_fixed_pool_size(OS_HANDLE TaskHandle, char *GroupName,
					void *Addr, USHORT PoolSize,
					ULONG PartSize, OS_HANDLE *GroupHandle,
					ULONG *NumCreated)
{
	USHORT num;

	num = PoolSize / (PartSize + PT_CHKOVERHEAD + PT_OVERHEAD);
	*NumCreated = num;
	return os_CreatePartitionPool(TaskHandle, GroupName, Addr, num,
					PartSize, GroupHandle);
}

GLOBAL LONG
os_CreateMemoryPool(OS_HANDLE TaskHandle, char *Name, void *Addr,
		    ULONG PoolSize, OS_HANDLE *PoolHandle)
{
	STATUS sts;
	USHORT i;

	sts = NU_Obtain_Semaphore(&MemSemCB, NU_SUSPEND);
	for (i = 0; i < NumOfMemoryPools; i++)
		if (!strncmp(Name, MemPoolTable[i].name, RESOURCE_NAMELEN-1)) {
			*PoolHandle = i;
			if (sts == NU_SUCCESS)
				NU_Release_Semaphore(&MemSemCB);
			return(OS_OK);
		}
	if (i >= MaxMemoryPools) {
release_sem_return_err:
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&MemSemCB);
		return(OS_ERROR);
	}
	if (i) {
		if (os_AllocateMemory(OS_NOTASK,
				      (T_VOID_STRUCT **) &MemPoolTable[i].pcb,
				      sizeof(NU_MEMORY_POOL), OS_NO_SUSPEND,
				      os_ext_pool_handle) != OS_OK)
			goto release_sem_return_err;
		/*
		 * FreeCalypso: we need to bzero the DM_PCB before calling
		 * NU_Create_Memory_Pool() to prevent the possibility of
		 * Nucleus error checker failing the call because the
		 * signature word happens to be there already.
		 */
		bzero(MemPoolTable[i].pcb, sizeof(NU_MEMORY_POOL));
	}
	if (NU_Create_Memory_Pool(MemPoolTable[i].pcb, Name, Addr, PoolSize,
				  4, NU_FIFO) != NU_SUCCESS)
		goto release_sem_return_err;
	strncpy(MemPoolTable[i].name, Name, RESOURCE_NAMELEN);
	MemPoolTable[i].name[RESOURCE_NAMELEN-1] = 0;
	*PoolHandle = i;
	NumOfMemoryPools++;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&MemSemCB);
	return(OS_OK);
}
