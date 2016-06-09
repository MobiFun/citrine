/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
+-----------------------------------------------------------------------------
|  Copyright 2002 Texas Instruments Berlin, AG
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Texas
|                 Instruments Berlin, AG
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Texas Instruments Berlin, AG.
+-----------------------------------------------------------------------------
|  Purpose :  Component Table for GSM
+-----------------------------------------------------------------------------
*/

#include "../../include/config.h"
#include "../../include/condat-features.h"
#include "gpfconf.h"

#define	CONFIG_MODULE	1

#ifndef NEW_ENTITY
#define NEW_ENTITY
#endif

/*==== INCLUDES ===================================================*/

#include "../../nucleus/nucleus.h"
#include "typedefs.h"
#include "os.h"
#include "vsi.h"
#include "pei.h"
#include "gsmconst.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"
#include "os_types.h"
#include "os_glob.h"

/*==== CONSTANTS ==================================================*/


/*==== EXTERNALS ==================================================*/

extern SHORT tstrcv_pei_create(T_PEI_INFO const **Info);
extern SHORT tstsnd_pei_create(T_PEI_INFO const **Info);
extern SHORT aci_pei_create   (T_PEI_INFO const **Info);
extern SHORT cst_pei_create   (T_PEI_INFO const **Info);
extern SHORT sim_pei_create   (T_PEI_INFO const **Info);
extern SHORT sms_pei_create   (T_PEI_INFO const **Info);
extern SHORT cc_pei_create    (T_PEI_INFO const **Info);
extern SHORT ss_pei_create    (T_PEI_INFO const **Info);
extern SHORT mm_pei_create    (T_PEI_INFO const **Info);
extern SHORT rr_pei_create    (T_PEI_INFO const **Info);
extern SHORT dl_pei_create    (T_PEI_INFO const **Info);
extern SHORT pl_pei_create    (T_PEI_INFO const **Info);
#ifdef FAX_AND_DATA
extern SHORT l2r_pei_create   (T_PEI_INFO const **Info);
extern SHORT rlp_pei_create   (T_PEI_INFO const **Info);
extern SHORT fad_pei_create   (T_PEI_INFO const **Info);
extern SHORT t30_pei_create   (T_PEI_INFO const **Info);
#endif /* FAX_AND_DATA */
#ifdef UART
extern SHORT uart_pei_create  (T_PEI_INFO const **Info);
#endif/*UART*/
#ifdef BT_ADAPTER
extern SHORT bti_pei_create ( T_PEI_INFO const **Info);
#endif /* BT_ADAPTER */

#ifdef FF_EOTD
extern SHORT lc_pei_create    (T_PEI_INFO const **Info);
extern SHORT rrlp_pei_create  (T_PEI_INFO const **Info);
#endif /* FF_EOTD */

#ifdef CO_UDP_IP
extern SHORT udp_pei_create   (T_PEI_INFO const **Info);
extern SHORT ip_pei_create    (T_PEI_INFO const **Info);
#endif

#if defined (FF_WAP) || defined (FF_SAT_E)
extern SHORT ppp_pei_create ( T_PEI_INFO const **Info);
#endif /* FF_WAP || FF_SAT_E */

#ifdef FF_WAP
#ifndef UDP_NO_WAP
extern SHORT wap_pei_create   (T_PEI_INFO const **Info);
#endif /* UDP_NO_WAP */
#endif /* FF_WAP */

#ifndef _TARGET_
extern SHORT psi_pei_create    (T_PEI_INFO const **Info);
#ifdef FAX_AND_DATA
extern SHORT ra_pei_create    (T_PEI_INFO const **Info);
#endif
#endif

#ifdef _TARGET_
extern SHORT l1_pei_create    (T_PEI_INFO const **Info);
#endif
/*==== VARIABLES ==================================================*/

#ifndef _TARGET_
BOOL newTstHeader = TRUE;
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

const T_COMPONENT_ADDRESS mmi_list[] =
{
  { aci_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS cst_list[] =
{
  { cst_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS sim_list[] =
{
  { sim_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS cm_list[] =
{
  { sms_pei_create,      NULL,   ASSIGNED_BY_TI },
  { cc_pei_create,       NULL,   ASSIGNED_BY_TI },
  { ss_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   (int)"CM" }
};

const T_COMPONENT_ADDRESS mm_list[] =
{
  { mm_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS rr_list[] =
{
  { rr_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS dl_list[] =
{
  { dl_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS pl_list[] =
{
  { pl_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

#ifdef FAX_AND_DATA
const T_COMPONENT_ADDRESS l2rt30_list[] =
{
  { l2r_pei_create,      NULL,   ASSIGNED_BY_TI },
  { t30_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   (int) "L2RT30" }
};

const T_COMPONENT_ADDRESS rlpfad_list[] =
{
  { rlp_pei_create,      NULL,   ASSIGNED_BY_TI },
  { fad_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   (int) "RLPFAD"}
};

#ifndef _TARGET_
const T_COMPONENT_ADDRESS ra_list[] =
{
  { ra_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /* !_TARGET_ */
#endif /* FAX_AND_DATA */

#ifndef _TARGET_
#ifdef FF_TCP_IP
const T_COMPONENT_ADDRESS aaa_list[] =
{
  { NULL,                "AAA",  ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /* FF_TCP_IP */
#endif /* !_TARGET_ */

#ifndef _TARGET_
#ifdef FF_PSI
const T_COMPONENT_ADDRESS psi_list[] =
{
  { psi_pei_create,      NULL,  ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /*FF_PSI*/
#endif /* !_TARGET_ */

#ifdef UART
const T_COMPONENT_ADDRESS uart_list[] =
{
  { uart_pei_create,     NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /*UART*/

#ifdef FF_EOTD
const T_COMPONENT_ADDRESS eotd_list[] =
{
  { lc_pei_create,       NULL,   ASSIGNED_BY_TI },
  { rrlp_pei_create,     NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   (int)"EOTD" }
};
#endif /* FF_EOTD */


#ifdef FF_WAP
#ifndef UDP_NO_WAP
const T_COMPONENT_ADDRESS wap_list[] =
{
  { wap_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /* UDP_NO_WAP */
#endif /* FF_WAP */

#ifdef CO_UDP_IP
const T_COMPONENT_ADDRESS udp_list[] =
{
  { udp_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS ip_list[] =
{
  { ip_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif

#if defined (FF_WAP) || defined (FF_SAT_E)
const T_COMPONENT_ADDRESS ppp_list[] =
{
  { ppp_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /* FF_WAP || FF_SAT_E */

#ifdef BT_ADAPTER
const T_COMPONENT_ADDRESS bti_list[] =
{
  { bti_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /* BT_ADAPTER */

const T_COMPONENT_ADDRESS l1_list[] =
{
#ifdef _TARGET_
  { l1_pei_create,       NULL,    ASSIGNED_BY_TI },
#else
  { NULL,                "L1",   ASSIGNED_BY_TI },
#endif
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS * const ComponentTables[]=
{
  tstrcv_list,
  tstsnd_list,
  mmi_list,
  cst_list,
  sim_list,
  cm_list,
  mm_list,
  rr_list,
  dl_list,
  pl_list,
#ifdef FAX_AND_DATA
  l2rt30_list,
  rlpfad_list,
#ifndef _TARGET_
  ra_list,
#endif
#endif /* FAX_AND_DATA */
#ifndef _TARGET_
#ifdef FF_TCP_IP
  aaa_list,
#endif /* FF_TCP_IP */
#ifdef FF_PSI
  psi_list,
#endif /*FF_PSI*/
#endif /* !_TARGET_ */
#ifdef UART
  uart_list,
#endif
#ifdef FF_EOTD
  eotd_list,
#endif
#ifdef FF_WAP
#ifndef UDP_NO_WAP
  wap_list,
#endif /* !UDP_NO_WAP */
#endif /* FF_WAP */

#ifdef CO_UDP_IP
  udp_list,
  ip_list,
#endif /* CO_UDP_IP */
#if defined (FF_WAP) || defined (FF_SAT_E)
  ppp_list,
#endif /* FF_WAP || FF_SAT_E */

#ifdef BT_ADAPTER
  bti_list,
#endif /* BT_ADAPTER */
  l1_list,
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
#ifdef FAX_AND_DATA

#define PRIM_PARTITION_0_SIZE     52
#define PRIM_PARTITION_1_SIZE    100
#define PRIM_PARTITION_2_SIZE    432
#define PRIM_PARTITION_3_SIZE   1600
#define PRIM_PARTITION_4_SIZE      0
#define PRIM_PARTITION_5_SIZE      0

#define PRIMPOOL_0_PARTITIONS     80
#define PRIMPOOL_1_PARTITIONS     20
#define PRIMPOOL_2_PARTITIONS     30
#define PRIMPOOL_3_PARTITIONS      3
#define PRIMPOOL_4_PARTITIONS      0
#define PRIMPOOL_5_PARTITIONS      0

#else
/* no fax and data */
#define PRIM_PARTITION_0_SIZE     52
#define PRIM_PARTITION_1_SIZE    100
#define PRIM_PARTITION_2_SIZE    216 /* New */
#define PRIM_PARTITION_3_SIZE    260 /* sizeof (T_stk_cmd), do we need this? */
#define PRIM_PARTITION_4_SIZE    432
#define PRIM_PARTITION_5_SIZE   1600 /* FreeCalypso raised from 900 */

#define PRIMPOOL_0_PARTITIONS     80
#define PRIMPOOL_1_PARTITIONS     20
#ifdef TI_PS_UICC_CHIPSET_15
#define PRIMPOOL_2_PARTITIONS      6 /*  0 */
#else
#define PRIMPOOL_2_PARTITIONS      5 /*  0 */
#endif

#define PRIMPOOL_3_PARTITIONS      6 /*  0 */ /* Number to be optimized */ 

/*
 * FreeCalypso change: bumping the number of 432-byte partitions in the
 * voice-only config from 5 or 6 to 10.  Our FreeCalypso code appears to
 * work with the original smaller config, but this warning is seen:

SYSTEM WARNING: Bigger partition allocated than requested, entity SIM, Size 288

 * SIM's request for size 288 will normally be allocated out of the 432 pool,
 * but the next size up is the rare big one (used to be 900, we bumped it up
 * to 1600), and wasting those is bad.  So let's give it some more 432-byte
 * partitions.  (See the FAX_AND_DATA config above for comparison.)
 */

#define PRIMPOOL_4_PARTITIONS      10

#define PRIMPOOL_5_PARTITIONS      3 /* FreeCalypso raised from 2 */

#endif /* else, #ifdef FAX_AND_DATA */


#ifdef FAX_AND_DATA
unsigned int MaxPrimPartSize = PRIM_PARTITION_3_SIZE;
#else
unsigned int MaxPrimPartSize = PRIM_PARTITION_5_SIZE;
#endif

char pool10 [ POOL_SIZE(PRIMPOOL_0_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_0_SIZE)) ] __attribute__ ((section ("int.ram")));

char pool11 [ POOL_SIZE(PRIMPOOL_1_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_1_SIZE)) ] __attribute__ ((section ("int.ram")));

char pool12 [ POOL_SIZE(PRIMPOOL_2_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_2_SIZE)) ] __attribute__ ((section ("int.ram")));

char pool13 [ POOL_SIZE(PRIMPOOL_3_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_3_SIZE)) ] __attribute__ ((section ("int.ram")));

#ifndef FAX_AND_DATA
char pool14 [ POOL_SIZE(PRIMPOOL_4_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_4_SIZE)) ] __attribute__ ((section ("int.ram")));

char pool15 [ POOL_SIZE(PRIMPOOL_5_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_5_SIZE)) ] __attribute__ ((section ("int.ram")));
#endif /* FAX_AND_DATA */


const T_FRM_PARTITION_POOL_CONFIG prim_grp_config[] =
{
  { PRIMPOOL_0_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_0_SIZE), &pool10 },
  { PRIMPOOL_1_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_1_SIZE), &pool11 },
  { PRIMPOOL_2_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_2_SIZE), &pool12 },
  { PRIMPOOL_3_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_3_SIZE), &pool13 },
#ifndef FAX_AND_DATA
  { PRIMPOOL_4_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_4_SIZE), &pool14 },
  { PRIMPOOL_5_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_5_SIZE), &pool15 },
#endif
  { 0                    , 0                    , NULL	  }
};

/*
 * Partitions pool configuration for test interface communication
 *
 * FreeCalypso: I bumped the configuration up from what TI had in their
 * (likely unmaintained) gsmcomp.c as I expect we'll be doing a lot of
 * debugging. - Space Falcon
 */
#define TESTPOOL_0_PARTITIONS     10	/* was 1 */
#define TESTPOOL_1_PARTITIONS     50	/* was 15 */
#define TESTPOOL_2_PARTITIONS      2	/* was 0 */

#define TSTSND_QUEUE_ENTRIES         (TESTPOOL_0_PARTITIONS+TESTPOOL_1_PARTITIONS+TESTPOOL_2_PARTITIONS)
#define TSTRCV_QUEUE_ENTRIES         50

#define TEST_PARTITION_0_SIZE     80	/* was 16 */
#ifdef _TARGET_
  #define TEST_PARTITION_1_SIZE  160
#else
  #define TEST_PARTITION_1_SIZE  260
#endif /* else, #ifdef _TARGET_ */

#define TEST_PARTITION_2_SIZE      1600

const USHORT TST_SndQueueEntries    = TSTSND_QUEUE_ENTRIES;
const USHORT TST_RcvQueueEntries    = TSTRCV_QUEUE_ENTRIES;
const USHORT TextTracePartitionSize = TEST_PARTITION_1_SIZE;

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

#define DMEM_PARTITION_0_SIZE     16 /* 100 */

/*
 * CQ 29245: Value of the DMEMPOOL_0_PARTITIONS has been modified from 30 to 50
 */

#define DMEMPOOL_0_PARTITIONS     50 /* 10 */ /* In GPRS this is 70 */

char pool30 [ POOL_SIZE(DMEMPOOL_0_PARTITIONS,ALIGN_SIZE(DMEM_PARTITION_0_SIZE)) ] __attribute__ ((section ("ext.ram")));

const T_FRM_PARTITION_POOL_CONFIG dmem_grp_config[] =
{
  { DMEMPOOL_0_PARTITIONS, ALIGN_SIZE(DMEM_PARTITION_0_SIZE), &pool30 },
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

/* Detecting GSM only lite to compile using minimal settings */
#if !defined(FAX_AND_DATA) && !defined(FF_TCP_IP) && !defined(FF_EOTD) &&\
    !defined(FF_WAP) && !defined(BT_ADAPTER)
#define GO_LITE 1
#endif

#if defined(_TARGET_) && defined(GO_LITE)
#define EXT_DATA_POOL_PS_BASE_SIZE 28000 /* FreeCalypso bumped up from 23075 */
#else /* _TARGET_ && GO_LITE */
#define EXT_DATA_POOL_PS_BASE_SIZE 50000
#endif /* _TARGET_ && GO_LITE */

#undef GO_LITE /* We are not going to use this definition globally */

#ifdef MEMORY_SUPERVISION
 #define EXT_DATA_POOL_PPS_ADDITION ((EXT_DATA_POOL_PS_BASE_SIZE>>3)+10000)
#else  /* MEMORY_SUPERVISION */
 #define EXT_DATA_POOL_PPS_ADDITION 0
#endif /* MEMORY_SUPERVISION */

#ifdef FAX_AND_DATA
 #define EXT_DATA_POOL_FD_ADDITION 13000
#else  /* FAX_AND_DATA */
 #define EXT_DATA_POOL_FD_ADDITION 0
#endif /* FAX_AND_DATA */

#if defined (FF_WAP) || defined (FF_SAT_E)
 #define EXT_DATA_POOL_WAP_ADDITION 5000
#else  /* FF_WAP || FF_SAT_E */
 #define EXT_DATA_POOL_WAP_ADDITION 0
#endif /* FF_WAP || FF_SAT_E */

#define EXT_DATA_POOL_PS_SIZE   (EXT_DATA_POOL_PS_BASE_SIZE + EXT_DATA_POOL_PPS_ADDITION + EXT_DATA_POOL_FD_ADDITION + EXT_DATA_POOL_WAP_ADDITION)
#define INT_DATA_POOL_PS_SIZE   1   /* no distinction between external/internal RAM for GO/GOLite builds */

#define EXT_DATA_POOL_GPF_SIZE  (2048 + OS_QUEUE_ENTRY_SIZE(TSTSND_QUEUE_ENTRIES) + OS_QUEUE_ENTRY_SIZE(TSTRCV_QUEUE_ENTRIES))

#define EXT_DATA_POOL_SIZE      (EXT_DATA_POOL_PS_SIZE + EXT_DATA_POOL_GPF_SIZE)
#define INT_DATA_POOL_SIZE      (INT_DATA_POOL_PS_SIZE)

char ext_data_pool              [ EXT_DATA_POOL_SIZE ]
					__attribute__ ((section ("ext.ram")));

const T_MEMORY_POOL_CONFIG memory_pool_config[MAX_MEMORY_POOLS+1] =
{
  { "EXTPOOL", EXT_DATA_POOL_SIZE, &ext_data_pool[0] },
  {  NULL }
};

extern T_HANDLE ext_data_pool_handle;

T_HANDLE *MemoryPoolHandle[MAX_MEMORY_POOLS+1] =
{
  &ext_data_pool_handle,
  NULL
};
