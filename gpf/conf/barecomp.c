/*
 * If we are building a configuration with just "bare" GPF, plus maybe L1,
 * but no GSM or GPRS L23 stack, then this bare{comp.c,const.h} config
 * takes the place of gsm* or gprs*.
 *
 * This barecomp.c configuration has been made from gprscomp.c, the
 * one actually used in the working leo2moko reference.
 */

#include "../../include/config.h"
#include "gpfconf.h"

#define	CONFIG_MODULE	1

#ifndef _TARGET_
#define NEW_ENTITY
#endif

/*==== INCLUDES ===================================================*/

#include "../../nucleus/nucleus.h"
#include "typedefs.h"
#include "os.h"
#include "vsi.h"
#include "pei.h"
#include "bareconst.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"
#include "os_types.h"
#include "os_glob.h"

/*==== CONSTANTS ==================================================*/


/*==== EXTERNALS ==================================================*/

extern SHORT tstrcv_pei_create(T_PEI_INFO const **Info);
extern SHORT tstsnd_pei_create(T_PEI_INFO const **Info);

#if CONFIG_L1_STANDALONE
extern SHORT l1_pei_create          (T_PEI_INFO const **Info);
extern SHORT l1stand_fwd_pei_create (T_PEI_INFO const **info);
#endif

/*==== VARIABLES ==================================================*/

//EF For normal Test Definition Language (TDL) TAP usage set newTstHeader = FALSE
//EF For multiple entity (TCSL) Test Case Script Lang.   set newTstHeader = TRUE
#ifndef _TARGET_

#ifdef TDL_TAP
BOOL newTstHeader = FALSE;
#else
BOOL newTstHeader = TRUE;
#endif

#endif

const T_COMPONENT_ADDRESS tstrcv_list[] =
{
  { tstrcv_pei_create,   NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS tstsnd_list[] =
{
  { tstsnd_pei_create,   NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

#if CONFIG_L1_STANDALONE
const T_COMPONENT_ADDRESS l1fwd_list[] =
{
  { l1stand_fwd_pei_create, NULL,   ASSIGNED_BY_TI },
  { NULL,                   NULL,   0 }
};

const T_COMPONENT_ADDRESS l1_list[] =
{
  { l1_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif

const T_COMPONENT_ADDRESS *ComponentTables[]=
{
  tstrcv_list,
  tstsnd_list,
#if CONFIG_L1_STANDALONE
  l1fwd_list,
  l1_list,
#endif
  NULL
};

/*==== VERSIONS ===================================================*/
#ifndef CTRACE
  char * str2ind_version = "&0";
#endif

/*==== MEMORY CONFIGURATION =======================================*/

/*
 * Partitions pool configuration for primitive communication
 */

/*
 * Memory extensions for multiplexer
 */
#ifdef FF_MUX
#define PRIMPOOL_0_MUX_ADDITION 30
#define PRIMPOOL_2_MUX_ADDITION 10
#else /* FF_MUX */
#define PRIMPOOL_0_MUX_ADDITION  0
#define PRIMPOOL_2_MUX_ADDITION  0
#endif /* else FF_MUX */

/*
 * Memory extensions for multiple PDP contexts
 */
#ifdef FF_PKTIO
#define PRIMPOOL_2_MPDP_ADDITION 30
#else /* FF_PKTIO */
#define PRIMPOOL_2_MPDP_ADDITION  0
#endif /* else FF_PKTIO */
#ifdef WIN32
/*
 * Required for testing LLC acknowledged mode.
 */
#define PRIMPOOL_0_PARTITIONS		200
#define PRIMPOOL_1_PARTITIONS		100
#define PRIMPOOL_2_PARTITIONS		 20
#define PRIMPOOL_3_PARTITIONS		 20

#else /*WIN32*/

#define PRIMPOOL_0_PARTITIONS   (190 + PRIMPOOL_0_MUX_ADDITION)
#define PRIMPOOL_1_PARTITIONS		 110
#define PRIMPOOL_2_PARTITIONS   ( 50 + PRIMPOOL_2_MPDP_ADDITION + PRIMPOOL_2_MUX_ADDITION)
#define PRIMPOOL_3_PARTITIONS		   7

#endif /*WIN32*/

#define PRIM_PARTITION_0_SIZE	    60
#define PRIM_PARTITION_1_SIZE	   128
#define PRIM_PARTITION_2_SIZE	   632
#define PRIM_PARTITION_3_SIZE	  1600

#ifndef DATA_INT_RAM
unsigned int MaxPrimPartSize = PRIM_PARTITION_3_SIZE;
#endif /* !DATA_INT_RAM */

char pool10 [ POOL_SIZE(PRIMPOOL_0_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_0_SIZE)) ] __attribute__ ((section ("int.ram")));

char pool11 [ POOL_SIZE(PRIMPOOL_1_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_1_SIZE)) ] __attribute__ ((section ("int.ram")));

char pool12 [ POOL_SIZE(PRIMPOOL_2_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_2_SIZE)) ] __attribute__ ((section ("int.ram")));

char pool13 [ POOL_SIZE(PRIMPOOL_3_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_3_SIZE)) ] __attribute__ ((section ("int.ram")));

const T_FRM_PARTITION_POOL_CONFIG prim_grp_config[] =
{
  { PRIMPOOL_0_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_0_SIZE), &pool10 },
  { PRIMPOOL_1_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_1_SIZE), &pool11 },
  { PRIMPOOL_2_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_2_SIZE), &pool12 },
  { PRIMPOOL_3_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_3_SIZE), &pool13 },
  { 0                    , 0                    , NULL	  }
};

/*
 * Partitions pool configuration for test interface communication
 */
#define TESTPOOL_0_PARTITIONS		 10
#define TESTPOOL_1_PARTITIONS		200
#define TESTPOOL_2_PARTITIONS		  2

#define TSTSND_QUEUE_ENTRIES         (TESTPOOL_0_PARTITIONS+TESTPOOL_1_PARTITIONS+TESTPOOL_2_PARTITIONS)
#define TSTRCV_QUEUE_ENTRIES         50

#define TEST_PARTITION_0_SIZE	    80
#ifdef _TARGET_
 #define TEST_PARTITION_1_SIZE   160
#else
 #define TEST_PARTITION_1_SIZE   260
#endif
#define TEST_PARTITION_2_SIZE	  1600

#ifndef DATA_INT_RAM
const USHORT TST_SndQueueEntries    = TSTSND_QUEUE_ENTRIES;
const USHORT TST_RcvQueueEntries    = TSTRCV_QUEUE_ENTRIES;
const USHORT TextTracePartitionSize = TEST_PARTITION_1_SIZE;
#endif /* !DATA_INT_RAM */

char pool20 [ POOL_SIZE(TESTPOOL_0_PARTITIONS,ALIGN_SIZE(TEST_PARTITION_0_SIZE)) ] __attribute__ ((section ("ext.ram")));

char pool21 [ POOL_SIZE(TESTPOOL_1_PARTITIONS,ALIGN_SIZE(TEST_PARTITION_1_SIZE)) ] __attribute__ ((section ("ext.ram")));

char pool22 [ POOL_SIZE(TESTPOOL_2_PARTITIONS,ALIGN_SIZE(TEST_PARTITION_2_SIZE)) ] __attribute__ ((section ("ext.ram")));

const T_FRM_PARTITION_POOL_CONFIG test_grp_config[] =
{
  { TESTPOOL_0_PARTITIONS, ALIGN_SIZE(TEST_PARTITION_0_SIZE), &pool20 },
  { TESTPOOL_1_PARTITIONS, ALIGN_SIZE(TEST_PARTITION_1_SIZE), &pool21 },
  { TESTPOOL_2_PARTITIONS, ALIGN_SIZE(TEST_PARTITION_2_SIZE), &pool22 },
  { 0                    , 0                    , NULL	  }
};

/*
 * Partitions pool configuration for general purpose allocation
 */

#define DMEMPOOL_0_PARTITIONS		70
#define DMEMPOOL_1_PARTITIONS		 2

#define DMEM_PARTITION_0_SIZE		16
#ifdef _TARGET_
#define DMEM_PARTITION_1_SIZE		1600 /* for non tracing ccd arm7 */
#else
#define DMEM_PARTITION_1_SIZE		2800 /* for non tracing ccd pc */
#endif

char pool30 [ POOL_SIZE(DMEMPOOL_0_PARTITIONS,ALIGN_SIZE(DMEM_PARTITION_0_SIZE)) ] __attribute__ ((section ("ext.ram")));

char pool31 [ POOL_SIZE(DMEMPOOL_1_PARTITIONS,ALIGN_SIZE(DMEM_PARTITION_1_SIZE)) ] __attribute__ ((section ("ext.ram")));

const T_FRM_PARTITION_POOL_CONFIG dmem_grp_config[] =
{
  { DMEMPOOL_0_PARTITIONS, ALIGN_SIZE(DMEM_PARTITION_0_SIZE), &pool30 },
  { DMEMPOOL_1_PARTITIONS, ALIGN_SIZE(DMEM_PARTITION_1_SIZE), &pool31 },
  { 0                    , 0                    , NULL	  }
};

/*
 * Partitions group list
 */

const T_FRM_PARTITION_GROUP_CONFIG partition_grp_config[MAX_POOL_GROUPS+1] =
{
  { "PRIM", &prim_grp_config[0] },
  { "TEST", &test_grp_config[0] },
  { "DMEM", &dmem_grp_config[0] },
  { NULL,   NULL                }
};

extern T_HANDLE PrimGroupHandle;
extern T_HANDLE DmemGroupHandle;
extern T_HANDLE TestGroupHandle;

T_HANDLE *PoolGroupHandle[MAX_POOL_GROUPS+1] =
{
  &PrimGroupHandle,
  &TestGroupHandle,
  &DmemGroupHandle,
  NULL
};

/*
 * Dynamic Memory Pool Configuration
 */

#ifdef _TARGET_
#ifdef FF_ESIM
 #define EXT_DATA_POOL_PS_BASE_SIZE 45000
#else
 #define EXT_DATA_POOL_PS_BASE_SIZE 35000
#endif
#define INT_DATA_POOL_PS_BASE_SIZE 25000
#else /* _TARGET_ */
#define EXT_DATA_POOL_TCPIP_ADDTIION 120000
#define EXT_DATA_POOL_PS_BASE_SIZE  80000 + EXT_DATA_POOL_TCPIP_ADDTIION
#define INT_DATA_POOL_PS_BASE_SIZE  1000
#endif /* _TARGET_ */

#ifdef MEMORY_SUPERVISION
 #define EXT_DATA_POOL_PPS_ADDITION ((EXT_DATA_POOL_PS_BASE_SIZE>>3)+25000)
 #define INT_DATA_POOL_PPS_ADDITION ((INT_DATA_POOL_PS_BASE_SIZE>>3))
#else  /* MEMORY_SUPERVISION */
 #define EXT_DATA_POOL_PPS_ADDITION 0
 #define INT_DATA_POOL_PPS_ADDITION 0
#endif /* MEMORY_SUPERVISION */

#if defined (FF_WAP) || defined (FF_SAT_E)
 #define EXT_DATA_POOL_WAP_ADDITION 15000
#else
 #define EXT_DATA_POOL_WAP_ADDITION 0
#endif /* FF_WAP OR SAT E */

#ifdef GRR_PPC_IF_PRIM
 #define INT_DATA_POOL_GRR_PPC_IF_PRIM_ADDITION  3000
#else  /* #ifdef GRR_PPC_IF_PRIM */
 #define INT_DATA_POOL_GRR_PPC_IF_PRIM_ADDITION  0
#endif /* #ifdef GRR_PPC_IF_PRIM */

#define EXT_DATA_POOL_PS_SIZE   (EXT_DATA_POOL_PS_BASE_SIZE + EXT_DATA_POOL_WAP_ADDITION             + EXT_DATA_POOL_PPS_ADDITION)
#define INT_DATA_POOL_PS_SIZE   (INT_DATA_POOL_PS_BASE_SIZE + INT_DATA_POOL_GRR_PPC_IF_PRIM_ADDITION + INT_DATA_POOL_PPS_ADDITION)

#define EXT_DATA_POOL_GPF_SIZE  (2048 + OS_QUEUE_ENTRY_SIZE(TSTSND_QUEUE_ENTRIES) + OS_QUEUE_ENTRY_SIZE(TSTRCV_QUEUE_ENTRIES))

#define EXT_DATA_POOL_SIZE      (EXT_DATA_POOL_PS_SIZE + EXT_DATA_POOL_GPF_SIZE)
#define INT_DATA_POOL_SIZE      (INT_DATA_POOL_PS_SIZE)

char ext_data_pool              [ EXT_DATA_POOL_SIZE ]
					__attribute__ ((section ("ext.ram")));

char int_data_pool              [ INT_DATA_POOL_SIZE ]
					__attribute__ ((section ("int.ram")));

const T_MEMORY_POOL_CONFIG memory_pool_config[MAX_MEMORY_POOLS+1] =
{
  { "INTPOOL", INT_DATA_POOL_SIZE, &int_data_pool[0] },
  { "EXTPOOL", EXT_DATA_POOL_SIZE, &ext_data_pool[0] },
  {  NULL }
};

extern T_HANDLE ext_data_pool_handle;
extern T_HANDLE int_data_pool_handle;

T_HANDLE *MemoryPoolHandle[MAX_MEMORY_POOLS+1] =
{
  &int_data_pool_handle,
  &ext_data_pool_handle,
  NULL
};
