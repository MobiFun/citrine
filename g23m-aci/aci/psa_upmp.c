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
|  Purpose :  This module defines the processing functions for the
|             primitives send to the protocol stack adapter by upm.
+----------------------------------------------------------------------------- 
*/ 

#if defined GPRS

#ifndef PSA_SNDP_C
#define PSA_SNDP_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "aci.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"
#include "cmh.h"
#include "cmh_sm.h"

#ifdef FF_WAP
#include "wap_aci.h"
#include "psa_tcpip.h"
#endif /* FF_WAP */

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== IMPORT =====================================================*/

EXTERN void cmhUPM_Counted( UBYTE nsapi,
                            ULONG octets_uplink,
                            ULONG octets_downlink,
                            ULONG packets_uplink,
                            ULONG packets_downlink );

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------+
| PROJECT : UPM                   MODULE  : PSA_UPM           |
| STATE   : finished              ROUTINE : psa_upm_count_cnf |
+-------------------------------------------------------------+

  PURPOSE : processes the UPM_COUNT_CNF primitive send by UPM.
*/
GLOBAL  void psa_upm_count_cnf ( T_UPM_COUNT_CNF *upm_count_cnf )
{

  TRACE_FUNCTION ("psa_upm_count_cnf()");

  cmhUPM_Counted( upm_count_cnf->nsapi,
                  upm_count_cnf->octets_uplink,
                  upm_count_cnf->octets_downlink,
                  upm_count_cnf->packets_uplink,
                  upm_count_cnf->packets_downlink );

  PFREE (upm_count_cnf);
}

#endif /* GPRS */
