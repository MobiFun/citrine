/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_GPRSP.C
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
|  Purpose :  This module defines the functions which are necessary 
|             to bind the GPRS Primitives (MMGMM SAP) to the 
|             MM entity's internal functional interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_GPRSP_C
#define MM_GPRSP_C

#define ENTITY_MM

/*==== INCLUDES ===================================================*/
#include <string.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"                     
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_mm.h"
#include "mon_mm.h"
#include "pei.h"                     
#include "tok.h"
#include "mm.h"

#ifdef  GPRS

/*==== EXPORT ==============================================================*/

/*==== TEST ================================================================*/

/*==== PRIVAT ==============================================================*/

/*==== VARIABLES ===========================================================*/

/*==== FUNCTIONS ===========================================================*/

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_reg_req                   |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_REG_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void mm_mmgmm_reg_req (const T_MMGMM_REG_REQ *mmgmm_reg_req)
{
  TRACE_FUNCTION ("mm_mmgmm_reg_req()");
  mm_func_mmgmm_reg_req (mmgmm_reg_req->service_mode, 
                         mmgmm_reg_req->reg_type,
                         mmgmm_reg_req->mobile_class,
                         mmgmm_reg_req->bootup_act);

  PFREE (mmgmm_reg_req);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_nreg_req                  |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_NREG_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void mm_mmgmm_nreg_req (const T_MMGMM_NREG_REQ *mmgmm_nreg_req)
{
  TRACE_FUNCTION ("mm_mmgmm_nreg_req()");

  mm_func_mmgmm_nreg_req (mmgmm_nreg_req->detach_cause, 
                          mmgmm_nreg_req->detach_done,
                          mmgmm_nreg_req->cause);

  PFREE (mmgmm_nreg_req);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_net_req                   |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_NREG_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void mm_mmgmm_net_req (const T_MMGMM_NET_REQ *mmgmm_net_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_net_req()");

 /* Mark the network search as beeing for the MMI */
  mm_data->plmn_scan_mmi = TRUE;

  /* Start scanning for available PLMNs */
  mm_func_mmgmm_net_req ();

  PFREE (mmgmm_net_req);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_plmn_res                  |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_PLMN_RES primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void mm_mmgmm_plmn_res (const T_MMGMM_PLMN_RES *mmgmm_plmn_res)
{
  TRACE_FUNCTION ("mm_mmgmm_plmn_res()");
  
#ifdef WIN32
  vsi_o_ttrace (VSI_CALLER TC_EVENT,
                "  plmn = %x%x%x %x%x%x", 
                mmgmm_plmn_res->plmn.mcc[0],
                mmgmm_plmn_res->plmn.mcc[1],
                mmgmm_plmn_res->plmn.mcc[2],
                mmgmm_plmn_res->plmn.mnc[0],
                mmgmm_plmn_res->plmn.mnc[1],
                mmgmm_plmn_res->plmn.mnc[2]);
  vsi_o_ttrace (VSI_CALLER TC_EVENT,
                "  reg_type = %x", 
                mmgmm_plmn_res->reg_type);
#endif /* #ifdef WIN32 */  

  mm_func_mmgmm_plmn_res (&mmgmm_plmn_res->plmn,
                          mmgmm_plmn_res->reg_type,
                          mmgmm_plmn_res->mobile_class);
  
  PFREE (mmgmm_plmn_res);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_plmn_mode_req             |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_PLMN_MODE_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void 
mm_mmgmm_plmn_mode_req (const T_MMGMM_PLMN_MODE_REQ *mmgmm_plmn_mode_req)
{
  TRACE_FUNCTION ("mm_mmgmm_plmn_mode_req()");

  mm_func_mmgmm_plmn_mode_req (mmgmm_plmn_mode_req->mode);

  PFREE (mmgmm_plmn_mode_req);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_auth_rej_req              |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_AUTH_REG_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void 
mm_mmgmm_auth_rej_req (const T_MMGMM_AUTH_REJ_REQ *mmgmm_auth_rej_req)
{
  TRACE_FUNCTION ("mm_mmgmm_auth_rej_req()");

  mm_func_mmgmm_auth_rej_req ();

  PFREE (mmgmm_auth_rej_req);
}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_cm_establish_res          |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_CM_ESTABLISH_RES primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void 
mm_mmgmm_cm_establish_res (const T_MMGMM_CM_ESTABLISH_RES *est_res)
{
  TRACE_FUNCTION ("mm_mmgmm_cm_establish_res()");

  mm_func_mmgmm_cm_establish_res (est_res->cm_establish_res);

  PFREE (est_res);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_attach_started_req        |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_ATTACH_STARTED_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void
mm_mmgmm_attach_started_req (const T_MMGMM_ATTACH_STARTED_REQ *start_req)
{
  TRACE_FUNCTION ("mm_mmgmm_attach_started_req()");

  mm_func_mmgmm_attach_started_req ();

  PFREE (start_req);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_attach_acc_req            |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_ATTACH_ACC_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void 
mm_mmgmm_attach_acc_req (const T_MMGMM_ATTACH_ACC_REQ *mmgmm_attach_acc_req)
{
  TRACE_FUNCTION ("mmgmm_attach_acc_req()");

  mm_func_mmgmm_attach_acc_req (&mmgmm_attach_acc_req->plmn,
                                mmgmm_attach_acc_req->lac,
                                mmgmm_attach_acc_req->v_tmsi,
                                mmgmm_attach_acc_req->tmsi,
                                mmgmm_attach_acc_req->v_equ_plmn_list,
                                &mmgmm_attach_acc_req->equ_plmn_list);
  
  PFREE (mmgmm_attach_acc_req);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_attach_rej_req            |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_ATTACH_REJ_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void
mm_mmgmm_attach_rej_req (const T_MMGMM_ATTACH_REJ_REQ *mmgmm_attach_rej_req)
{
  TRACE_FUNCTION ("mm_mmgmm_attach_rej_req()");

  mm_func_mmgmm_attach_rej_req (mmgmm_attach_rej_req->cause);

  PFREE (mmgmm_attach_rej_req);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_detach_started_req        |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_DETACH_STARTED_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void
mm_mmgmm_detach_started_req (const T_MMGMM_DETACH_STARTED_REQ *det_start_req)
{
  TRACE_FUNCTION ("mm_mmgmm_detach_started_req()");

  mm_func_mmgmm_detach_started_req ();

  PFREE (det_start_req);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_start_t3212_req           |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_START_T3212_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void
mm_mmgmm_start_t3212_req (const T_MMGMM_START_T3212_REQ *start_t3212_req)
{
  TRACE_FUNCTION ("mmgmm_start_t3212_req()");

  mm_func_mmgmm_start_t3212_req ();

  PFREE (start_t3212_req);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_cm_emergency_res          |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_CM_EMERGENCY_RES primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void 
mm_mmgmm_cm_emergency_res (const T_MMGMM_CM_EMERGENCY_RES *emerge_res)
{
  TRACE_FUNCTION ("mm_mmgmm_cm_emergency_res()");

  mm_func_mmgmm_cm_emergency_res (emerge_res->cm_establish_res);

  PFREE (emerge_res);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_allowed_req               |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_ALLOWED_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void 
mm_mmgmm_allowed_req (const T_MMGMM_ALLOWED_REQ *allowed_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_allowed_req()");

  /* 
   * It can be safely assumed that the PLMN and the LAC is the 
   * PLMN and the LAC where RR is currently camped on.
   */
  assert (mm_check_lai_from_RR (&mm_data->mm.lai,
                                &allowed_req->plmn,
                                allowed_req->lac));
  
  mm_func_mmgmm_allowed_req (allowed_req->v_equ_plmn_list, &allowed_req->equ_plmn_list);

  PFREE (allowed_req);
}

#if 0
/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_mmgmm_trigger_req               |
+----------------------------------------------------------------------------+

  PURPOSE : This function receives the MMGMM_TRIGGER_REQ primitive and
            forwards it to its appropriate handler.
  
*/

GLOBAL void mm_mmgmm_trigger_req (const T_MMGMM_TRIGGER_REQ *trigger_req)
{
  TRACE_FUNCTION ("mm_mmgmm_trigger_req()");

  mm_func_mmgmm_trigger_req ();

  PFREE (trigger_req);
}
#endif

#endif /* GPRS */

#endif /* MM_GPRSP_C */
