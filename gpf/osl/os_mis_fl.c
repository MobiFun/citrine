/*
 * This C module is a reconstruction based on the disassembly of
 * os_mis.obj in frame_na7_db_fl.lib from the Leonardo package.
 */

/* set of included headers from COFF symtab, slightly reordered: */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../../riviera/rv/general.h"
#include "../../riviera/rv/rv_general.h"
#include "../../riviera/rvf/rvf_api.h"
#include "../../services/ffs/ffs.h"
#include "gpfconf.h"	/* FreeCalypso addition */
#include "../../nucleus/nucleus.h"
#include "typedefs.h"
#include "os.h"
#include "gdi.h"
#include "os_types.h"
#include "os_glob.h"
#include "header.h"
#include "vsi.h"
#include "drvconf.h"
#include "../tst_pei/tstdriver.h"
#include "dar_func.h"
#include "tools.h"

typedef unsigned char u_char;

char *NU_State[13] = {
	"NU_READY",
	"",
	"NU_SLEEP_SUSPEND",
	"",
	"",
	"NU_QUEUE_SUSPEND",
	"NU_SEMAPHORE_SUSPEND",
	"",
	"NU_PARTITION_SUSPEND",
	"NU_MEMORY_SUSPEND",
	"",
	"NU_FINISHED",
	"NU_TERMINATED",
};

char *msg_type[4] = {
	"",
	"PRIMITIVE",
	"SIGNAL",
	"TIMEOUT",
};

char *warning = "SYSTEM WARNING: ";

T_GPF_DAR_PROPERTIES *dar;
T_GPF_DAR_STRUCT os_dar;
NU_PROTECT os_mis_Protect;
u_char SystemErrorBuffer[256];
u_char DARErrorBuffer[256];

static u_char state;
static int fd;
static unsigned bytes_read;
static u_char *msg;
static unsigned count;
static unsigned data_len;
static unsigned dar_wrap_around;
static unsigned dar_buffer_count;
static unsigned dar_wrap_around_detected;

LONG
os_read_dar_ffs_data(USHORT entry, char *buffer, USHORT len)
{
	/* dummy for now */
	return(OS_ERROR);
}

LONG
os_dar_set_filter(void)
{
	if (dar)
		dar->diagnose_swe_filter(dar->gpf_use_id, dar->warning);
	return(OS_OK);
}

LONG
os_dar_register(const void *dar_properties)
{
	if (dar_properties) {
		os_dar.properties = (T_GPF_DAR_PROPERTIES *) dar_properties;
		os_dar.magic_nr = DAR_INITIALIZED;
		dar = (T_GPF_DAR_PROPERTIES *) dar_properties;
	}
	return(OS_OK);
}

void
os_dar_init(void)
{
	if (os_dar.magic_nr != DAR_INITIALIZED)
		dar = 0;
}

void
os_SystemError(OS_HANDLE Caller, USHORT cause, char *buffer)
{
	/*
	 * The implementation of this function in the binary blob
	 * which we are reversing is very complex.  I am leaving it
	 * as an empty stub for now. - Space Falcon
	 */
}

GLOBAL LONG
os_ObjectInformation(OS_HANDLE Caller, USHORT Id, USHORT Handle, USHORT len,
			void *Buffer)
{
	switch (Id) {
	case OS_OBJTASK:
		return os_TaskInformation(Handle, Buffer);
	case OS_OBJQUEUE:
		return os_QueueInformation(Handle, Buffer);
	case OS_OBJPARTITIONGROUP:
		return os_PartitionInformation(Handle, Buffer);
	case OS_OBJMEMORYPOOL:
		return os_MemoryInformation(Handle, Buffer);
	case OS_OBJTIMER:
		return os_TimerInformation(Handle, Buffer);
	case OS_OBJSEMAPHORE:
		return os_SemaphoreInformation(Handle, Buffer);
	default:
		return(OS_ERROR);
	}
}

GLOBAL LONG
os_Initialize(void)
{
	if (os_SemInit() == OS_ERROR)
		return(OS_ERROR);
	if (os_ProInit() == OS_ERROR)
		return(OS_ERROR);
	if (os_ComInit() == OS_ERROR)
		return(OS_ERROR);
	if (os_MemInit() == OS_ERROR)
		return(OS_ERROR);
	if (os_TimInit() == OS_ERROR)
		return(OS_ERROR);
	if (os_isr_init() == OS_ERROR)
		return(OS_ERROR);
#if 0
	if (os_EvGrpInit() == OS_ERROR)
		return(OS_ERROR);
#endif
	/*
	 * The original code zeroes out the first 32-bit word of
	 * os_mis_Protect here.  We don't really need to do that,
	 * as it is a zeroed-on-boot bss var.
	 */
	os_dar_init();	/* inlined in the original */
	return(OS_OK);
}
