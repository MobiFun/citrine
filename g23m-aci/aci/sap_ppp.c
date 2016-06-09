/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS
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
|  Purpose :  MUX for PPP primitives
|             
|             Different cases occur for same PPP primitives. In this
|             file the correct PSA function will be called
+----------------------------------------------------------------------------- 
*/ 

/*==== INCLUDES ===================================================*/

/* All this is only needed if we have GPRS, an internal WAP application, or
 * Riviera data. */
#if defined(FF_WAP) || defined(GPRS) || defined(FF_PPP) || defined(FF_SAT_E)

#ifndef SAP_PPPP_C
#define SAP_PPPP_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
#include "wap_aci.h"
#endif /* WAP */

#include "aci.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#include "psa_ppp_w.h"
/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/
#if defined (GPRS) AND defined (DTI)
EXTERN void psa_gppp_establish_cnf ( T_PPP_ESTABLISH_CNF *ppp_establish_cnf );
EXTERN void psa_gppp_terminate_ind ( T_PPP_TERMINATE_IND *ppp_terminate_ind );
EXTERN void psa_gppp_pdp_activate_ind ( T_PPP_PDP_ACTIVATE_IND *ppp_pdp_activate_ind );
EXTERN void psa_gppp_modification_cnf ( T_PPP_MODIFICATION_CNF *ppp_modification_cnf );
EXTERN void psa_gppp_dti_connected_ind ( T_PPP_DTI_CONNECTED_IND *ppp_dti_connected_ind );
#endif /* GPRS */

#if defined(FF_WAP) || defined(FF_PPP) || defined (FF_GPF_TCPIP) || defined(FF_SAT_E) 
EXTERN void psa_wppp_establish_cnf(T_PPP_ESTABLISH_CNF *ppp_establish_cnf);
EXTERN void psa_wppp_terminate_ind(T_PPP_TERMINATE_IND *ppp_terminate_ind);
EXTERN void psa_wppp_dti_connected_ind(T_PPP_DTI_CONNECTED_IND *ppp_dti_connected_ind);
#endif /* WAP or FF_PPP or SAT E */
/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM                   MODULE  : SAP_DTI                 |
|                                 ROUTINE : psa_ppp_establish_cnf   |
+-------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void psa_ppp_establish_cnf ( T_PPP_ESTABLISH_CNF *ppp_establish_cnf )
{
  TRACE_FUNCTION("psa_ppp_establish_cnf()...") ;
#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
  if( Wap_Call EQ TRUE )
  {
    psa_wppp_establish_cnf(ppp_establish_cnf);
    return;
  }
  else
#endif /* WAP or SAT E */
#ifdef FF_PPP
  if (pppShrdPrm.is_PPP_CALL EQ TRUE)
  {
    psa_wppp_establish_cnf(ppp_establish_cnf);
    return;
  }
  else
#endif /* FF_PPP */
  {
#if defined (GPRS) AND defined (DTI)
    psa_gppp_establish_cnf ( ppp_establish_cnf );
    return;
#endif /* GPRS */
  }

  /* The primitive is PFREEd in the functions called. */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM                   MODULE  : SAP_DTI                 |
|                                 ROUTINE : psa_ppp_terminate_ind   |
+-------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void psa_ppp_terminate_ind ( T_PPP_TERMINATE_IND *ppp_terminate_ind )
{
#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
  if( Wap_Call EQ TRUE )
  {
    psa_wppp_terminate_ind(ppp_terminate_ind);
    return;
  }
  else
#endif /* WAP or SAT E */
#ifdef FF_PPP
  if (pppShrdPrm.is_PPP_CALL EQ TRUE)
  {
    psa_wppp_terminate_ind(ppp_terminate_ind);
    return;
  }
  else
#endif /* FF_TCP_IP */
  {
#if defined (GPRS) AND defined (DTI)
    psa_gppp_terminate_ind ( ppp_terminate_ind );
    return;
#endif /* GPRS */
  }

  /* The primitive is PFREEd in the function called. */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM                   MODULE  : SAP_DTI                 |
|                                 ROUTINE : psa_ppp_pdp_activate_ind|
+-------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void psa_ppp_pdp_activate_ind ( T_PPP_PDP_ACTIVATE_IND *ppp_pdp_activate_ind )
{
#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)

  if( Wap_Call EQ TRUE )
  {
    PFREE (ppp_pdp_activate_ind);
    return;
  }
  else
#endif /* WAP or SAT E */
  {
#if defined (GPRS) AND defined (DTI)
    psa_gppp_pdp_activate_ind ( ppp_pdp_activate_ind );
    return;
#endif /* GPRS */
  }

  /* The primitive is PFREEd in the function called. */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM                   MODULE  : SAP_DTI                 |
|                                 ROUTINE : psa_ppp_modification_cnf|
+-------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void psa_ppp_modification_cnf ( T_PPP_MODIFICATION_CNF *ppp_modification_cnf )
{
#if defined (GPRS) AND defined (DTI)
    psa_gppp_modification_cnf ( ppp_modification_cnf );
    return;
#endif /* GPRS */

  /* The primitive is PFREEd in the function called. */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM                   MODULE  : SAP_DTI                  |
|                                 ROUTINE : psa_ppp_dti_connected_ind|
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void psa_ppp_dti_connected_ind ( T_PPP_DTI_CONNECTED_IND *ppp_dti_connected_ind )
{

  TRACE_FUNCTION("psa_ppp_dti_connected_ind()");

#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
  if( Wap_Call EQ TRUE )
  {
    psa_wppp_dti_connected_ind(ppp_dti_connected_ind);
    return;
  }
  else
#endif /* WAP or SAT E */
#ifdef FF_PPP
  if (pppShrdPrm.is_PPP_CALL EQ TRUE)
  {
    psa_wppp_dti_connected_ind(ppp_dti_connected_ind);
    return;
  }
  else
#endif /* FF_PPP */
  {
#if defined (GPRS) AND defined (DTI)
    psa_gppp_dti_connected_ind ( ppp_dti_connected_ind );
    return;
#endif /* GPRS */
  }

  /* The primitive is PFREEd in the function called. */
}



#endif /* WAP or GPRS or FF_PPP or SAT E */
