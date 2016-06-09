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
|  Purpose :  Component Table for GPRS
+-----------------------------------------------------------------------------
*/

#ifndef _TARGET_
#define NEW_ENTITY
#endif

/*==== INCLUDES ===================================================*/

#include "nucleus.h"
#include "typedefs.h"
#include "os.h"
#include "vsi.h"
#include "pei.h"
#include "gprsconst.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"
#include "os_types.h"
#include "os_glob.h"
#include "gprsconst.h"

/*==== CONSTANTS ==================================================*/


/*==== EXTERNALS ==================================================*/

extern SHORT tstrcv_pei_create(T_PEI_INFO const **Info);
extern SHORT tstsnd_pei_create(T_PEI_INFO const **Info);
extern SHORT aci_pei_create   (T_PEI_INFO const **Info);
extern SHORT cst_pei_create   (T_PEI_INFO const **Info);
#ifdef FF_ESIM
extern SHORT esim_pei_create  (T_PEI_INFO const **Info); /* esim module */
#endif
extern SHORT sim_pei_create   (T_PEI_INFO const **Info);
extern SHORT sms_pei_create   (T_PEI_INFO const **Info);
extern SHORT cc_pei_create    (T_PEI_INFO const **Info);
extern SHORT sm_pei_create    (T_PEI_INFO const **Info);
extern SHORT ss_pei_create    (T_PEI_INFO const **Info);
extern SHORT mm_pei_create    (T_PEI_INFO const **Info);
extern SHORT gmm_pei_create   (T_PEI_INFO const **Info);
extern SHORT rr_pei_create    (T_PEI_INFO const **Info);
extern SHORT grr_pei_create   (T_PEI_INFO const **Info);
extern SHORT grlc_pei_create  (T_PEI_INFO const **Info);
extern SHORT dl_pei_create    (T_PEI_INFO const **Info);
extern SHORT pl_pei_create    (T_PEI_INFO const **Info);
#ifdef FAX_AND_DATA
extern SHORT l2r_pei_create   (T_PEI_INFO const **Info);
extern SHORT rlp_pei_create   (T_PEI_INFO const **Info);
#ifdef FF_FAX
extern SHORT fad_pei_create   (T_PEI_INFO const **Info);
extern SHORT t30_pei_create   (T_PEI_INFO const **Info);
#endif
#endif /* FAX_AND_DATA */
extern SHORT llc_pei_create   (T_PEI_INFO const **Info);
extern SHORT sndcp_pei_create (T_PEI_INFO const **Info);
extern SHORT ppp_pei_create   (T_PEI_INFO const **Info);
extern SHORT uart_pei_create  (T_PEI_INFO const **Info);

#ifdef FF_MUX
extern SHORT mux_pei_create (T_PEI_INFO const **Info);
#endif /* MUX */

#ifdef FF_PKTIO
extern SHORT pktio_pei_create (T_PEI_INFO const **Info);
#endif /* #ifdef FF_PKTIO */

#ifdef FF_PSI
extern SHORT psi_pei_create (T_PEI_INFO const **Info);
#endif

#ifdef FF_EOTD
extern SHORT lc_pei_create    (T_PEI_INFO const **Info);
extern SHORT rrlp_pei_create  (T_PEI_INFO const **Info);
#endif /* FF_EOTD */

#ifdef CO_UDP_IP
extern SHORT udp_pei_create   (T_PEI_INFO const **Info);
extern SHORT ip_pei_create    (T_PEI_INFO const **Info);
#endif /* CO_UDP_IP */

#ifdef FF_WAP
extern SHORT wap_pei_create   (T_PEI_INFO const **Info);
#endif /* FF_WAP */

#ifdef _TARGET_
extern SHORT l1_pei_create    (T_PEI_INFO const **Info);
#ifdef FF_TCP_IP
extern SHORT aaa_pei_create   (T_PEI_INFO const **Info);
#endif /* FF_TCP_IP */
#endif /* _TARGET_ */


#ifdef FF_GPF_TCPIP
extern SHORT tcpip_pei_create (T_PEI_INFO const **Info);
#endif /* FF_TCP_IP */

#if defined (CO_TCPIP_TESTAPP) || defined (CO_BAT_TESTAPP)
extern SHORT app_pei_create   (T_PEI_INFO const **Info);
#endif /* CO_TCPIP_TESTAPP */
#ifndef _TARGET_
extern SHORT ra_pei_create    (T_PEI_INFO const **Info);
#endif

#ifndef FF_ATI_BAT
  #ifdef  FF_BAT 
extern SHORT gdd_dio_pei_create(T_PEI_INFO const **Info);
  #endif
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

#ifndef DATA_EXT_RAM

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

#ifdef FF_ESIM
const T_COMPONENT_ADDRESS esim_list[] =
{
  { esim_pei_create,     NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif

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
  { sm_pei_create,       NULL,   ASSIGNED_BY_TI },
  { ss_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   (int)"CM" }
};

const T_COMPONENT_ADDRESS mmgmm_list[] =
{
  { mm_pei_create,       NULL,   ASSIGNED_BY_TI },
  { gmm_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   (int)"MMGMM" }
};

const T_COMPONENT_ADDRESS rr_list[] =
{
  { rr_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS grr_list[] =
{
  { grr_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS grlc_list[] =
{
  { grlc_pei_create,     NULL,   ASSIGNED_BY_TI },
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
#ifdef FF_FAX
  { t30_pei_create,      NULL,   ASSIGNED_BY_TI },
#endif
  { NULL,                NULL,   (int)"L2RT30" }
};

const T_COMPONENT_ADDRESS rlpfad_list[] =
{
  { rlp_pei_create,      NULL,   ASSIGNED_BY_TI },
#ifdef FF_FAX
  { fad_pei_create,      NULL,   ASSIGNED_BY_TI },
#endif
  { NULL,                NULL,   (int)"RLPFAD" }
};
#endif /* FAX_AND_DATA */


const T_COMPONENT_ADDRESS llc_list[] =
{
  { llc_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS sndcp_list[] =
{
  { sndcp_pei_create,    NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS ppp_list[] =
{
  { ppp_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

const T_COMPONENT_ADDRESS uart_list[] =
{
  { uart_pei_create,     NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};

#ifdef FF_MUX
const T_COMPONENT_ADDRESS mux_list[] =
{
  { mux_pei_create,    NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /* MUX */

#ifdef FF_PKTIO
const T_COMPONENT_ADDRESS pktio_list[] =
{
  { pktio_pei_create,    NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /* #ifdef FF_PKTIO */

#ifdef FF_PSI
const T_COMPONENT_ADDRESS psi_list[] =
{
  { psi_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /* #ifdef FF_PSI */

#ifdef FF_EOTD
const T_COMPONENT_ADDRESS eotd_list[] =
{
  { lc_pei_create,       NULL,   ASSIGNED_BY_TI },
  { rrlp_pei_create,     NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   (int)"EOTD" }
};
#endif /* FF_EOTD */

#ifdef FF_WAP
const T_COMPONENT_ADDRESS wap_list[] =
{
  { wap_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
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
#endif /* CO_UDP_IP */

#ifndef _TARGET_
const T_COMPONENT_ADDRESS ra_list[] =
{
  { ra_pei_create,       NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /* !_TARGET_ */

#ifdef FF_TCP_IP
const T_COMPONENT_ADDRESS aaa_list[] =
{
#ifdef _TARGET_
  { aaa_pei_create,      NULL,   ASSIGNED_BY_TI },
#else /* _TARGET_ */
  { NULL,               "AAA",   ASSIGNED_BY_TI },
#endif /* else _TARGET_ */
  { NULL,                NULL,   0 }
};
#endif /* FF_TCP_IP */

#ifdef FF_GPF_TCPIP
const T_COMPONENT_ADDRESS tcpip_list[] =
{
  { tcpip_pei_create,    NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif

#if defined (CO_TCPIP_TESTAPP) || defined (CO_BAT_TESTAPP)
const T_COMPONENT_ADDRESS app_list[] =
{
  { app_pei_create,      NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
#endif /* CO_TCPIP_TESTAPP */

const T_COMPONENT_ADDRESS l1_list[] =
{
#ifdef _TARGET_
  { l1_pei_create,       NULL,   ASSIGNED_BY_TI },
#else
  { NULL,                "L1",   ASSIGNED_BY_TI },
#endif
  { NULL,                NULL,   0 }
};

#ifndef FF_ATI_BAT
  #ifdef  FF_BAT
const T_COMPONENT_ADDRESS gdd_dio_list[] =
{
  { gdd_dio_pei_create,    NULL,   ASSIGNED_BY_TI },
  { NULL,                NULL,   0 }
};
  #endif
#endif

const T_COMPONENT_ADDRESS *ComponentTables[]=
{
  tstrcv_list,
  tstsnd_list,
  mmi_list,
  cst_list,
  sim_list,
  cm_list,
  mmgmm_list,
  rr_list,
  grr_list,
  grlc_list,
  dl_list,
  pl_list,
#ifdef FAX_AND_DATA
  l2rt30_list,
  rlpfad_list,
#endif /* FAX_AND_DATA */
  llc_list,
  sndcp_list,
  ppp_list,
  uart_list,
#ifdef FF_MUX
  mux_list,
#endif
#ifdef FF_PKTIO
  pktio_list,
#endif
#ifdef FF_PSI
  psi_list,
#endif
#ifndef FF_ATI_BAT
  #ifdef  FF_BAT
  gdd_dio_list,
  #endif /* FF_BAT */
#endif
#ifdef FF_EOTD
  eotd_list,
#endif
#ifdef FF_WAP
  wap_list,
#endif

#ifdef CO_UDP_IP
  udp_list,
  ip_list,
#endif
#if defined _SIMULATION_ && defined FF_FAX
  ra_list,
#endif
#ifdef FF_TCP_IP
  aaa_list,
#endif /* FF_TCP_IP */

#ifdef FF_GPF_TCPIP
  tcpip_list,
#endif

#if defined (CO_TCPIP_TESTAPP) || defined (CO_BAT_TESTAPP)
  app_list,
#endif /* CO_TCPIP_TESTAPP */

#ifdef FF_ESIM
  esim_list, /* needed for esim module */
#endif
  l1_list,
  NULL
};

/*==== VERSIONS ===================================================*/
#ifndef CTRACE
  char * str2ind_version = "&0";
#endif

#endif /* DATA_EXT_RAM */

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

#if (!defined DATA_EXT_RAM && defined PRIM_0_INT_RAM) || (!defined DATA_INT_RAM && !defined PRIM_0_INT_RAM)
char pool10 [ POOL_SIZE(PRIMPOOL_0_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_0_SIZE)) ];
#else
extern char pool10 [];
#endif

#if (!defined DATA_EXT_RAM && defined PRIM_1_INT_RAM) || (!defined DATA_INT_RAM && !defined PRIM_1_INT_RAM)
char pool11 [ POOL_SIZE(PRIMPOOL_1_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_1_SIZE)) ];
#else
extern char pool11 [];
#endif

#if (!defined DATA_EXT_RAM && defined PRIM_2_INT_RAM) || (!defined DATA_INT_RAM && !defined PRIM_2_INT_RAM)
char pool12 [ POOL_SIZE(PRIMPOOL_2_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_2_SIZE)) ];
#else
extern char pool12 [];
#endif

#if (!defined DATA_EXT_RAM && defined PRIM_3_INT_RAM) || (!defined DATA_INT_RAM && !defined PRIM_3_INT_RAM)
char pool13 [ POOL_SIZE(PRIMPOOL_3_PARTITIONS,ALIGN_SIZE(PRIM_PARTITION_3_SIZE)) ];
#else
extern char pool13 [];
#endif

#ifndef DATA_INT_RAM
const T_FRM_PARTITION_POOL_CONFIG prim_grp_config[] =
{
  { PRIMPOOL_0_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_0_SIZE), &pool10 },
  { PRIMPOOL_1_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_1_SIZE), &pool11 },
  { PRIMPOOL_2_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_2_SIZE), &pool12 },
  { PRIMPOOL_3_PARTITIONS, ALIGN_SIZE(PRIM_PARTITION_3_SIZE), &pool13 },
  { 0                    , 0                    , NULL	  }
};
#endif /* !DATA_INT_RAM */

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

#if (!defined DATA_EXT_RAM && defined TEST_0_INT_RAM) || (!defined DATA_INT_RAM && !defined TEST_0_INT_RAM)
char pool20 [ POOL_SIZE(TESTPOOL_0_PARTITIONS,ALIGN_SIZE(TEST_PARTITION_0_SIZE)) ];
#else
extern char pool20 [];
#endif

#if (!defined DATA_EXT_RAM && defined TEST_1_INT_RAM) || (!defined DATA_INT_RAM && !defined TEST_1_INT_RAM)
char pool21 [ POOL_SIZE(TESTPOOL_1_PARTITIONS,ALIGN_SIZE(TEST_PARTITION_1_SIZE)) ];
#else
extern char pool21 [];
#endif

#if (!defined DATA_EXT_RAM && defined TEST_2_INT_RAM) || (!defined DATA_INT_RAM && !defined TEST_2_INT_RAM)
char pool22 [ POOL_SIZE(TESTPOOL_2_PARTITIONS,ALIGN_SIZE(TEST_PARTITION_2_SIZE)) ];
#else
extern char pool22 [];
#endif

#ifndef DATA_INT_RAM
const T_FRM_PARTITION_POOL_CONFIG test_grp_config[] =
{
  { TESTPOOL_0_PARTITIONS, ALIGN_SIZE(TEST_PARTITION_0_SIZE), &pool20 },
  { TESTPOOL_1_PARTITIONS, ALIGN_SIZE(TEST_PARTITION_1_SIZE), &pool21 },
  { TESTPOOL_2_PARTITIONS, ALIGN_SIZE(TEST_PARTITION_2_SIZE), &pool22 },
  { 0                    , 0                    , NULL	  }
};
#endif /* !DATA_INT_RAM */

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

#if (!defined DATA_EXT_RAM && defined DMEM_0_INT_RAM) || (!defined DATA_INT_RAM && !defined DMEM_0_INT_RAM)
char pool30 [ POOL_SIZE(DMEMPOOL_0_PARTITIONS,ALIGN_SIZE(DMEM_PARTITION_0_SIZE)) ];
#else
extern char pool30 [];
#endif

#if (!defined DATA_EXT_RAM && defined DMEM_1_INT_RAM) || (!defined DATA_INT_RAM && !defined DMEM_1_INT_RAM)
char pool31 [ POOL_SIZE(DMEMPOOL_1_PARTITIONS,ALIGN_SIZE(DMEM_PARTITION_1_SIZE)) ];
#else
extern char pool31 [];
#endif

#ifndef DATA_INT_RAM
const T_FRM_PARTITION_POOL_CONFIG dmem_grp_config[] =
{
  { DMEMPOOL_0_PARTITIONS, ALIGN_SIZE(DMEM_PARTITION_0_SIZE), &pool30 },
  { DMEMPOOL_1_PARTITIONS, ALIGN_SIZE(DMEM_PARTITION_1_SIZE), &pool31 },
  { 0                    , 0                    , NULL	  }
};
#endif /* !DATA_INT_RAM */

/*
 * Partitions group list
 */

#ifndef DATA_INT_RAM
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
#endif /* !DATA_INT_RAM */

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

#ifndef DATA_INT_RAM
char ext_data_pool              [ EXT_DATA_POOL_SIZE ];
#endif

#ifndef DATA_EXT_RAM
char int_data_pool              [ INT_DATA_POOL_SIZE ];
#else
extern char int_data_pool       [ ];
#endif

#ifndef DATA_INT_RAM

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

#endif /* !DATA_INT_RAM */
