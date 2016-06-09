/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_MMF
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
|  Purpose :  This Modul defines the functions for the mob. management
|             capability of the module Mobility Management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_MMF_C
#define MM_MMF_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_MM

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
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
#ifdef _SIMULATION_
  #include "ffs_pc_api.h"
#else
  #include "../../services/ffs/ffs.h"
#endif
#include "mm_em.h"

/*==== EXPORT =====================================================*/
BOOL poor_quality_network = FALSE; /* Hack, maybe needed by CC */

/*==== TEST =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
GLOBAL T_FFS_RET ffs_mkdir(const char *name);

/*==== PRIVAT =====================================================*/
LOCAL BOOL mm_handle_ffs_read_result  (T_FFS_SIZE status_read);
LOCAL BOOL mm_handle_ffs_write_result (T_FFS_RET status_write);
LOCAL BOOL mm_create_ffs_dirs         (const char *name);
LOCAL BOOL mm_check_ffs_dirs          (void);


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_fill_identity           |
+--------------------------------------------------------------------+

  PURPOSE : This function fills the mobile identity structure with
            the desired identity, this maybe the IMSI, the IMSISV,
            the IMEI or the TMSI.

*/

LOCAL void mm_fill_identity (T_mob_id *mob_ident, UBYTE id_type)
{
  GET_INSTANCE_DATA;
  T_imsi_struct imei_struct;
  TRACE_FUNCTION ("mm_fill_identity()");

  memset (mob_ident, 0, sizeof (T_mob_id));

  switch (id_type)
  {
    case ID_TYPE_IMSI:
      mob_ident->ident_type  = id_type;
      mob_ident->v_ident_dig = TRUE;
      mob_ident->c_ident_dig = mm_calculate_digits (mm_data->reg.imsi_struct.id);
      mob_ident->odd_even    = mob_ident->c_ident_dig & 1;
      memcpy (mob_ident->ident_dig, mm_data->reg.imsi_struct.id, 16);
      break;
    
    case ID_TYPE_IMEI:
      csf_read_imei (&imei_struct);
      mob_ident->ident_type  = id_type;
      mob_ident->v_ident_dig = TRUE;
      mob_ident->c_ident_dig = 15;
      mob_ident->odd_even    = 1; 
      memcpy (mob_ident->ident_dig, imei_struct.id, 14);
      mob_ident->ident_dig[14] = 0;
      break;
    
    case ID_TYPE_IMEISV:
      csf_read_imei (&imei_struct);
      mob_ident->ident_type  = id_type;
      mob_ident->v_ident_dig = TRUE;
      mob_ident->c_ident_dig = 16;
      mob_ident->odd_even    = 0;
      memcpy (mob_ident->ident_dig, imei_struct.id, 16);
      break;
    
    case ID_TYPE_TMSI:
      mob_ident->ident_type  = id_type;
      mob_ident->v_ident_dig = FALSE;
      mob_ident->tmsi.l_tmsi = 32;
      mob_ident->v_tmsi      = TRUE;
      ccd_codeByte (mob_ident->tmsi.b_tmsi, 0, 8, 
        (UBYTE)(mm_data->reg.tmsi >> 24));
      ccd_codeByte (mob_ident->tmsi.b_tmsi, 8, 8, 
        (UBYTE)(mm_data->reg.tmsi >> 16));
      ccd_codeByte (mob_ident->tmsi.b_tmsi, 16, 8, 
        (UBYTE)(mm_data->reg.tmsi >> 8));
      ccd_codeByte (mob_ident->tmsi.b_tmsi, 24, 8, 
        (UBYTE)(mm_data->reg.tmsi));
      break;

    default: 
      TRACE_ERROR ("No such mobile identity");
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_fill_ident_for_est      |
+--------------------------------------------------------------------+

  PURPOSE : This functinon fills the mobile identity for an 
            establishment message and uses the appropriate identity
            for this purpose. The function returns the used mobile
            identity type.

*/

LOCAL UBYTE mm_fill_ident_for_est (T_mob_id *mob_ident)
{
  GET_INSTANCE_DATA;
  UBYTE id_type;

  TRACE_FUNCTION ("mm_fill_ident_for_est()");

  if (mm_data->reg.op.sim_ins EQ SIM_NO_INSRT)
  {
    /* No SIM present */
    id_type = ID_TYPE_IMEI;
  }
  else
  {
    /* SIM present */
    if (mm_data->reg.tmsi EQ TMSI_INVALID_VALUE)
    {
      /* No TMSI present */
      id_type = ID_TYPE_IMSI;
    }
    else
    {
      /* TMSI present */
      id_type = ID_TYPE_TMSI;
    }
  }

  mm_fill_identity (mob_ident, id_type);

  return id_type;
}

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * Procedures
 * -------------------------------------------------------------------
 */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_abort_connection        |
+--------------------------------------------------------------------+

  PURPOSE : This function sends the RR_ABORT_REQ primitive.

*/

GLOBAL void mm_abort_connection (UBYTE abcs)
{
  TRACE_FUNCTION ("mm_abort_connection ()");
  
  assert (abcs EQ ABCS_SIM_REM OR 
          abcs EQ ABCS_NORM);
  
  {
    PALLOC ( rr_abort_req, RR_ABORT_REQ); /* T_RR_ABORT_REQ */
    rr_abort_req->abcs = abcs;
    PSENDX (RR, rr_abort_req);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_attach_loc_upd          |
+--------------------------------------------------------------------+

  PURPOSE : This function starts an IMSI ATTACH location update.

*/

GLOBAL void mm_attach_loc_upd (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_attach_loc_upd ()");

  if (mm_lup_allowed_by_gmm())
  {
    mm_start_loc_upd (IMSI_ATTACH_LUP);
  }
  else
  {
    SET_STATE (STATE_MM, MM_IDLE_LUP_NEEDED);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_normal_loc_upd          |
+--------------------------------------------------------------------+

  PURPOSE : This function starts a normal location update procedure.

*/

GLOBAL void mm_normal_loc_upd (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_normal_loc_upd ()");

  if (mm_lup_allowed_by_gmm())
  {
    mm_start_loc_upd (NORMAL_LUP);
  }
  else
  {
    SET_STATE (STATE_MM, MM_IDLE_LUP_NEEDED);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_periodic_loc_upd        |
+--------------------------------------------------------------------+

  PURPOSE : This function starts a periodic location update.

*/

GLOBAL void mm_periodic_loc_upd (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_periodic_loc_upd ()");
  if (mm_lup_allowed_by_gmm())
  {
    mm_start_loc_upd (PERIODIC_LUP);
  }
  else
  {
    SET_STATE (STATE_MM, MM_IDLE_LUP_NEEDED);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_build_auth_res          |
+--------------------------------------------------------------------+

  PURPOSE : In this function the AUTHENTICATION RESPONSE message is 
            built.

*/

GLOBAL void mm_build_auth_res (T_SIM_AUTHENTICATION_CNF *sim_auth_cnf,
                               T_U_AUTH_RES             *auth_res)
{
  TRACE_FUNCTION ("mm_build_auth_res ()");
  auth_res->msg_type = U_AUTH_RES;
  auth_res->auth_sres.l_auth_sres = MAX_SRES << 3;
  auth_res->auth_sres.o_auth_sres = 0;
  memcpy (auth_res->auth_sres.b_auth_sres, sim_auth_cnf->sres, MAX_SRES);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_build_ident_res         |
+--------------------------------------------------------------------+

  PURPOSE : In this function the IDENTITY RESPONSE message is built.

*/

GLOBAL void mm_build_ident_res (UBYTE         id_type, 
                                T_U_IDENT_RES *ident_res)
{
  TRACE_FUNCTION ("mm_build_ident_res ()");
  ident_res->msg_type = U_IDENT_RES;
  mm_fill_identity (&ident_res->mob_id, id_type);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_build_mm_status         |
+--------------------------------------------------------------------+

  PURPOSE : In this function the MM STATUS message is built.

*/

GLOBAL void mm_build_mm_status (UBYTE         cause,
                                T_B_MM_STATUS *mm_status)
{
  TRACE_FUNCTION ("mm_build_mm_status ()");
  mm_status->msg_type  = B_MM_STATUS;
  mm_status->rej_cause = cause;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_build_rr_sync_req_cause |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to synchronize RR information 
            with changed information in MM.
            The function mm_build_rr_sync_req_cause() is used especially
            if the registration has failed for some reason, e.g. 
            authentication failure. Cause value is given, 
            ciphering information and tmsi are not present if this 
            function is called.

*/

GLOBAL void mm_build_rr_sync_req_cause (USHORT cause)
{
  GET_INSTANCE_DATA;
  PALLOC (rr_sync_req, RR_SYNC_REQ); /* T_RR_SYNC_REQ */

  TRACE_FUNCTION ("mm_build_rr_sync_req_cause ()");

  rr_sync_req->op.v_op    = V_OP_NOT_PRES;
  rr_sync_req->cksn       = CKSN_NOT_PRES;
  rr_sync_req->kcv.v_kc   = V_KC_NOT_PRES;
  rr_sync_req->tmsi_struct.v_mid = V_MID_NOT_PRES;
  rr_sync_req->plmn.v_plmn = V_PLMN_NOT_PRES;
  rr_sync_req->synccs     = cause;
  rr_sync_req->accc       = mm_data->reg.acc_class;
  rr_sync_req->eq_plmn_list.v_eq_plmn = FALSE;

  TRACE_EVENT_P1 ("sync_cause = %04x", cause);

  switch (cause)
  {
    case SYNCCS_LAI_ALLOW:
    case SYNCCS_LAI_NOT_ALLOW:
    case SYNCCS_LAI_NOT_ALLOW_FOR_ROAMING:
      rr_sync_req->plmn.v_plmn = V_PLMN_PRES;
      memcpy (rr_sync_req->plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
      memcpy (rr_sync_req->plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);
      rr_sync_req->lac = mm_data->mm.lai.lac;
      break;
    case SYNCCS_EPLMN_LIST:
      rr_sync_req->eq_plmn_list.v_eq_plmn = TRUE;
      memcpy(&rr_sync_req->eq_plmn_list.eq_plmn, 
             &mm_data->reg.eqv_plmns.eqv_plmn_list, 
             EPLMNLIST_SIZE*UBYTES_PER_PLMN);
      break;
    default: /* eg. SYNCCS_ACCC */
      break;
  }
  PSENDX (RR, rr_sync_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_build_rr_sync_req       |
+--------------------------------------------------------------------+

  PURPOSE : The function unifies the functions mm_build_rr_sync_req_mode()
            and mm_build_rr_sync_req_ciph() and is used to synchronize 
            RR information with changed information in MM. The parameter
            'ciph' decides whether the ciphering parameters are to be 
            set.
*/

GLOBAL void mm_build_rr_sync_req (T_MSG_TYPE ciph)
{
  GET_INSTANCE_DATA;
  PALLOC (rr_sync_req, RR_SYNC_REQ); /* T_RR_SYNC_REQ */

  TRACE_FUNCTION ("mm_build_rr_sync_req()");
  rr_sync_req->tmsi_struct.v_mid   = V_MID_NOT_PRES;
  rr_sync_req->synccs              = NOT_PRESENT_16BIT;
  rr_sync_req->accc                = mm_data->reg.acc_class;
  rr_sync_req->plmn.v_plmn         = V_PLMN_NOT_PRES;

  if( ciph EQ MSG_MM_CIPH)
  {
    rr_sync_req->op.v_op           = V_OP_NOT_PRES;
    rr_sync_req->cksn              = mm_data->reg.cksn;
    rr_sync_req->kcv.v_kc          = V_KC_PRES;
    memcpy (rr_sync_req->kcv.kc, mm_data->reg.kc, MAX_KC);
  }
  else
  {
    memcpy (&rr_sync_req->op, &mm_data->reg.op, sizeof (T_op));
    rr_sync_req->cksn              = NOT_PRESENT_8BIT;
    rr_sync_req->kcv.v_kc          = V_KC_NOT_PRES;
  }
  PSENDX (RR, rr_sync_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_build_rr_sync_req_tmsi  |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to synchronize RR information 
            with changed information in MM.
            The function mm_build_rr_sync_req_tmsi() is used especially
            to inform RR about a change in the ciphering parameter 
            tmsi.
            
*/

GLOBAL void mm_build_rr_sync_req_tmsi (void)
{
  GET_INSTANCE_DATA;
  PALLOC (rr_sync_req, RR_SYNC_REQ); /* T_RR_SYNC_REQ */

  TRACE_FUNCTION ("mm_build_rr_sync_req_tmsi ()");

  rr_sync_req->op.v_op           = V_OP_NOT_PRES;
  rr_sync_req->cksn              = CKSN_RES;
  rr_sync_req->kcv.v_kc          = V_KC_NOT_PRES;
  rr_sync_req->plmn.v_plmn       = V_PLMN_NOT_PRES;
  rr_sync_req->synccs            = NOT_PRESENT_16BIT;
  rr_sync_req->accc              = mm_data->reg.acc_class;

  memset (rr_sync_req->tmsi_struct.id, 0, 
          sizeof (rr_sync_req->tmsi_struct.id));  /* IMSI digits */
  rr_sync_req->tmsi_struct.v_mid    = V_MID_PRES;
  rr_sync_req->tmsi_struct.id_type  = TYPE_TMSI;
  rr_sync_req->tmsi_struct.tmsi_dig = mm_data->reg.tmsi;

  PSENDX (RR, rr_sync_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_build_rr_sync_hplmn_req |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to synchronize RR information 
            with changed AHPLMN in MM.
            The function mm_build_rr_sync_hplmn_req() is used especially
            to inform RR about the change of the status of RPLMN to AHPLMN.            
            
*/

GLOBAL void mm_build_rr_sync_hplmn_req (void)
{
  GET_INSTANCE_DATA;

  PALLOC (rr_sync_hplmn_req, RR_SYNC_HPLMN_REQ); /* T_RR_SYNC_HPLMN_REQ */

  TRACE_FUNCTION ("mm_build_rr_sync_hplmn_req()");

  rr_sync_hplmn_req->plmn.v_plmn = mm_data->reg.acting_hplmn.v_plmn;
  memcpy(&rr_sync_hplmn_req->plmn.mcc, &mm_data->reg.acting_hplmn.mcc, SIZE_MCC);
  memcpy(&rr_sync_hplmn_req->plmn.mnc, &mm_data->reg.acting_hplmn.mnc, SIZE_MNC);

  PSENDX(RR,rr_sync_hplmn_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_check_critical_error    |
+--------------------------------------------------------------------+

  PURPOSE : This function checks wheter a critical error has been 
            detected in the air message. Critical errors which prevent 
            the treatment of an air message are 
            - invalid Message ID
            - mandatory IE missing
            - IE coded as comprehension required missing
*/

GLOBAL BOOL mm_check_critical_error (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_check_critical_error ()");
  if (mm_data->error EQ RC_INVALID_MAND_MESSAGE     OR 
      mm_data->error EQ RC_MESSAGE_TYPE_NOT_IMPLEM  OR
      mm_data->error EQ RC_CONDITIONAL_IE           OR
      mm_data->error EQ RC_SERVICE_NOT_SUPPORTED    OR
      mm_data->error EQ RC_MESSAGE_INCOMPAT         OR
      mm_data->error EQ RC_MESSAGE_TYPE_INCOMPAT)
  {
    return (TRUE);
  }
  return (FALSE);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_send_status             |
+--------------------------------------------------------------------+

  PURPOSE : This function checks wheter a critical error has been 
            detected requiring 
            - to abort the AIM processing and
            - to send a STATUS message.
            The error can be detected by CCD or within the AIM processing itself.
            If any, the message is send, using the value found in mm_data->error as cause.
*/

GLOBAL BOOL mm_send_status_on_error (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_send_status_on_error ()");
  if (mm_check_critical_error())
  {

/* Implements Measure 29 and streamline encoding */
    mm_send_status(mm_data->error);
    mm_for_set_error (0);
    return (TRUE);
  }
  return (FALSE);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_check_error_flag        |
+--------------------------------------------------------------------+

  PURPOSE : This function checks wheter a critical error has been 
            detected in the air message. Critical errors which prevent 
            the treatment of an air message are all errors except 
            an error in an optional information element.
            [In calls of this function it is checked whether the 
             error is an RC_INVALID_MAND_MESSAGE, only this case is 
             treated as error in the caller's code. The code could be 
             simplified here if mm_check_error_flag() was a function 
             returning a BOOLEAN, indicating whether the message is valid
             or not. This is done this way in the CC entitity.]

*/

GLOBAL void mm_check_error_flag (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_check_error_flag ()");
  if (mm_data->error NEQ 0 AND 
      mm_data->error NEQ OPTIONAL_INFO_ERROR)
  {
/* Implements Measure 29 and streamline encoding */
    mm_send_status(mm_data->error);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_clear_mob_ident         |
+--------------------------------------------------------------------+

  PURPOSE : This function zeroes out the mobile identity. 


*/

GLOBAL void mm_clear_mob_ident (T_imsi_struct*mob_ident)
{
  TRACE_FUNCTION ("mm_clear_mob_ident ()");
  
  memset (mob_ident, 0, sizeof (T_imsi_struct));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_clear_reg_data          |
+--------------------------------------------------------------------+

  PURPOSE : This function clears some variables in the registration 
            data. This is done in a way that the SIM data is not lost, 
            the SIM data is only disabled by setting a software flag
            indicating MM shall not use the SIM data until it is 
            requested to do so by upper layers.

*/

GLOBAL void mm_clear_reg_data (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_clear_reg_data ()");
 
  mm_data->reg.op.func = FUNC_LIM_SERV_ST_SRCH;
  mm_data->reg.op.sim_ins = SIM_NO_INSRT;
  mm_data->first_attach = TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_copy_rr_act_cnf_data    |
+--------------------------------------------------------------------+

  PURPOSE : This function copies the data delivered by RR_ACTIVATE_CNF
            or RR_ACTIVATE_IND into the internal MM data structures.

*/

GLOBAL void mm_copy_rr_act_cnf_data (T_RR_ACTIVATE_CNF *rr_activate_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_copy_rr_act_cnf_data ()");

  mm_data->mm.mm_info = rr_activate_cnf->mm_info; /* Stucture copy */
  memcpy (mm_data->mm.lai.mcc, rr_activate_cnf->plmn.mcc, SIZE_MCC);
  memcpy (mm_data->mm.lai.mnc, rr_activate_cnf->plmn.mnc, SIZE_MNC);
  mm_data->mm.lai.lac = rr_activate_cnf->lac;
  mm_data->mm.cid = rr_activate_cnf->cid;

  EM_CELL_SELECTION_RESELECTION;

#ifdef GPRS
  mm_data->mm.gprs_indication= rr_activate_cnf->gprs_indication;
#endif /* GPRS */

  // Write-only variable, eliminated HM 20.07.00
  // mm_data->mm.lac = rr_activate_cnf->lac;

  /*
   * Until now we have only knowledge that chinese networks
   * seem to have special problems we have to deal with,
   * but maybe other countries have these special problems, too.
   */
  poor_quality_network =
    ((rr_activate_cnf->plmn.mcc[0] EQ 4) AND
     (rr_activate_cnf->plmn.mcc[1] EQ 6) AND
     (rr_activate_cnf->plmn.mcc[2] EQ 0));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_count_connections       |
+--------------------------------------------------------------------+

  PURPOSE : Count the number of connections which connection state 
            equals to the parameter conn_type. In case conn_type 
            is CM_NOT_IDLE, all connections not idle are counted.

*/

GLOBAL SHORT mm_count_connections (UBYTE conn_type)
{
  GET_INSTANCE_DATA;
  UBYTE cm;
  UBYTE ti;
  SHORT count = 0;
    
  TRACE_FUNCTION ("mm_count_connections ()");
  
  for (cm = 0; cm < NUM_OF_CM_ENT; cm++)
  {
    for (ti = 0; ti < NUM_OF_CONN_PER_CM_ENT; ti++)
    {
      if (((conn_type EQ CM_NOT_IDLE) AND (CMSTATE(cm, ti) NEQ CM_IDLE)) OR
          (CMSTATE(cm, ti) EQ conn_type))
      {
#if defined (WIN32)
        (void)CM_GET_STATE(cm, ti);
#endif
        count++;
      }
    }
  }
  return (count);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_create_est_message      |
+--------------------------------------------------------------------+

  PURPOSE : This functinon creates the establishment message, 
            either for MM services or for CM services. 
            If the entitity requiring service is a CM component, 
            the CM entitity will be remembered in mm_data->last_comp.

*/

GLOBAL void mm_create_est_message (USHORT    est_cause,
                                   UBYTE     service,
                                   UBYTE     ti,
                                   USHORT    *bit_size_message)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_create_est_message ()");

  TRACE_EVENT_P1 ("  ESTCS = %x", est_cause);
  TRACE_EVENT_P7 ("  REG: MCC=%X%X%X MNC=%X%X%X LAC=%04X",
                  mm_data->reg.lai.mcc[0],
                  mm_data->reg.lai.mcc[1],
                  mm_data->reg.lai.mcc[2],
                  mm_data->reg.lai.mnc[0],
                  mm_data->reg.lai.mnc[1],
                  mm_data->reg.lai.mnc[2],
                  mm_data->reg.lai.lac);
  TRACE_EVENT_P7 ("  MM: MCC=%X%X%X MNC=%X%X%X LAC=%04X",
                  mm_data->mm.lai.mcc[0],
                  mm_data->mm.lai.mcc[1],
                  mm_data->mm.lai.mcc[2],
                  mm_data->mm.lai.mnc[0],
                  mm_data->mm.lai.mnc[1],
                  mm_data->mm.lai.mnc[2],
                  mm_data->mm.lai.lac);

  if (est_cause NEQ ESTCS_SERV_REQ_BY_MM)
  {
    UBYTE comp;

    /*
     * Set the CM component requiring CM service 
     */
    switch (service)
    {
      case CALL_SERVICE:
      case EMERGENCY_SERVICE:
        comp = CC_COMP;
        break;

      case SS_SERVICE:
        comp = SS_COMP;
        break;

      case SMS_SERVICE:
        comp = SMS_COMP;
        break;

      default: /* Request for MM service */
        TRACE_ERROR ("Illegal service");
        comp = CC_COMP; /* Just to say something */
        break;
    }

    mm_data->pend_conn.ti   = ti;
    mm_data->pend_conn.comp = comp;
    mm_data->pend_conn.service = service;
    mm_data->pend_conn.cause = est_cause;
    if (est_cause EQ ESTCS_CAL_REEST)
    {
      CM_SET_STATE (comp, ti, CM_REEST_PENDING);
    }
    else
    {
      CM_SET_STATE (comp, ti, CM_PENDING);
    }
  }

  mm_data->rej_cause = 0;

  switch (est_cause)
  {
    case ESTCS_CAL_REEST:
      {
        MCAST (cm_reestab_req, U_CM_REESTAB_REQ); /* T_U_CM_REESTAB_REQ */

        *bit_size_message = BSIZE_U_CM_REESTAB_REQ;
        cm_reestab_req->msg_type = U_CM_REESTAB_REQ;
        cm_reestab_req->ciph_key_num.key_seq = mm_data->reg.cksn;
        cm_reestab_req->v_loc_area_ident = FALSE;

        csf_read_mobile_class_2 (&cm_reestab_req->mob_class_2);
        cm_reestab_req->mob_class_2.rf_pow_cap = mm_data->rf_power;

        if (mm_fill_ident_for_est (&cm_reestab_req->mob_id) EQ ID_TYPE_TMSI)
        {
          /* TMSI used. Include also location area to make this unambiguous */
          cm_reestab_req->v_loc_area_ident = TRUE;
          cm_reestab_req->loc_area_ident = mm_data->reg.lai; /* Struct copy */
        }
      }
      break;

    case ESTCS_SERV_REQ_BY_MM:
      {
        MCAST (loc_upd_req, U_LOC_UPD_REQ); /* T_U_LOC_UPD_REQ */

        *bit_size_message = BSIZE_U_LOC_UPD_REQ;
        loc_upd_req->msg_type = U_LOC_UPD_REQ;
        loc_upd_req->loc_upd_type = mm_data->loc_upd_type; /* Struct copy */
        loc_upd_req->ciph_key_num.key_seq = mm_data->reg.cksn;
        loc_upd_req->loc_area_ident = mm_data->reg.lai; /* Struct copy */
        if (loc_upd_req->loc_area_ident.mnc[2] EQ 0xF)
          loc_upd_req->loc_area_ident.c_mnc = 2;
        csf_read_mobile_class_1 (&loc_upd_req->mob_class_1);
        loc_upd_req->mob_class_1.rf_pow_cap = mm_data->rf_power;
        mm_fill_ident_for_est (&loc_upd_req->mob_id);
      }
      break;

    default:
      {
        MCAST (cm_serv_req, U_CM_SERV_REQ); /* T_U_CM_SERV_REQ */

        EM_CM_SERVICE_REQUESTED;

#if defined (WIN32)
        {
          char buf [40];
          sprintf (buf, "detected power = %d", mm_data->rf_power);
          TRACE_FUNCTION (buf);
        }
#endif

        *bit_size_message = BSIZE_U_CM_SERV_REQ;
        cm_serv_req->msg_type = U_CM_SERV_REQ;
        cm_serv_req->cm_serv_type = service;
        cm_serv_req->ciph_key_num.key_seq = mm_data->reg.cksn;

        csf_read_mobile_class_2 (&cm_serv_req->mob_class_2);
        cm_serv_req->mob_class_2.rf_pow_cap = mm_data->rf_power;

        mm_fill_ident_for_est (&cm_serv_req->mob_id);
      }
      break;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : MM_MM                         |
| STATE   : code             ROUTINE : mm_create_imsi_detach_message |
+--------------------------------------------------------------------+

  PURPOSE : This function creates the IMSI DETACH message. This may 
            either be forwarded to the network by the means of 
            RR_ESTABLISH_REQ or RR_DATA_REQ if a network connection 
            already exists.

*/

GLOBAL void mm_create_imsi_detach_message (void)
{
  GET_INSTANCE_DATA;
  MCAST (imsi_detach, U_IMSI_DETACH_IND);

  TRACE_FUNCTION ("mm_create_imsi_detach_message ()");

  imsi_detach->msg_type = U_IMSI_DETACH_IND;
  csf_read_mobile_class_1 (&imsi_detach->mob_class_1);
  imsi_detach->mob_class_1.rf_pow_cap = mm_data->rf_power;
  mm_fill_ident_for_est (&imsi_detach->mob_id);

  EM_IMSI_DETACH;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_delete_entry            |
+--------------------------------------------------------------------+

  PURPOSE :   The purpose of this function is to delete a stored entry
              previously written by the mm_write_entry() function.
              There are some types of entries which may be stored in
              the T_STORE data structure.
              - Establish attempts by the CM sublayer (MMXX_ESTABLISH_REQ)
              - Net requests by MMI (MMXXX_NET_REQ, AT+COPS=?)
              - Expired timers (TIMEOUT)
              - primitives incoming
              In case the content type is set to PRIMITIVE_ENTRY currently
              nothing is done, because no delete case was necessary so far.

*/

GLOBAL void mm_delete_entry (UBYTE comp, UBYTE ti)
{
  GET_INSTANCE_DATA;
  USHORT  i;
  USHORT  j;

  TRACE_FUNCTION ("mm_delete_entry ()");

  /*
   * Find stored entry in table
   */
  for (i = 0; i < MAX_STORE_ENTRIES; i++)
  {
    if (mm_data->store[i].content_type EQ EVENT_ENTRY)
    {

      if (mm_data->store[i].use NEQ ENTRY_FREE AND
          mm_data->store[i].content.event.comp EQ comp AND
          mm_data->store[i].content.event.ti   EQ ti)
      {
        TRACE_EVENT_P2 ("Delete entry: comp=%d, ti=%d", comp, ti);
        
        for (j = i; j < MAX_STORE_ENTRIES - 1; j++)
        {
          mm_data->store[j] = mm_data->store[j + 1]; /* Struct copy */
        }        
        mm_data->store[MAX_STORE_ENTRIES - 1].use = ENTRY_FREE;
        mm_data->store[MAX_STORE_ENTRIES - 1].content_type = NO_ENTRY;
          
        if (comp < NUM_OF_CM_ENT)
        {
            /* Establish request for any CM entity has been deleted */
            CM_SET_STATE(comp, ti, CM_IDLE);
        } 
        return;
      } /*IF comp ti*/
    } /*IF content_type*/
  } /*FOR*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_set_follow_on_request   |
+--------------------------------------------------------------------+

  PURPOSE : set the follow on request if at least one connection 
            attempt is stored.

*/

GLOBAL BOOL mm_set_follow_on_request (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_set_follow_on_request()");

  if (mm_count_connections (CM_STORE) NEQ 0 AND mm_cm_est_allowed())
  {
    mm_data->loc_upd_type.follow = FOR_PENDING_YES;
    return TRUE;
  }
  else
  {
    mm_data->loc_upd_type.follow = FOR_PENDING_NO;
    return FALSE;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_init                    |
+--------------------------------------------------------------------+

  PURPOSE : This function initializes the MM data structures.

*/

GLOBAL void mm_init (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_init()");

  TIMERSTOP (T_REGISTRATION);
  TIMERSTOP (T3210);
  TIMERSTOP (T3211);
  TIMERSTOP (T3212);  
  mm_data->t3212_timeout = FALSE;
  TIMERSTOP (T3213);
  mm_data->t3213_restart = 0;
  TIMERSTOP (T3220);
  TIMERSTOP (T3230);
  TIMERSTOP (T3240);
#ifdef REL99
  TIMERSTOP (T3241);
#endif

  memset (mm_data, 0, sizeof (T_MM_DATA));
  mm_data->limited_cause = MMCS_SIM_REMOVED; /* MMCS_SIM_INVAL_NOSIM */

  mm_data->last_auth_req_id = NOT_PRESENT_8BIT;
  mm_data->first_attach = TRUE;
  mm_data->first_attach_mem = FALSE;

  mm_data->t3212_cfg_counter = 0;
  mm_data->idle_entry = RRCS_INT_NOT_PRESENT;
  mm_data->ciphering_on = CIPH_NOT_PRES; /* Initialize ciphering indicator */

  
  mm_data->reg.length_mnc = 2;
  mm_data->net_search_count = 0;
  mm_data->ef_indicator = 0; /* Initialize for EF indication to SIM */
  mm_data->reg.is_cingular_sim = FALSE; /*This flag is specially for cingular n/w*/
  
  reg_init ();

  poor_quality_network = FALSE;

#ifdef GPRS
  SET_STATE (STATE_REG_TYPE, REG_GPRS_INACTIVE);
  SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_IDLE);
#endif /* GPRS */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_calculate_digits        |
+--------------------------------------------------------------------+

  PURPOSE : This function calculates the length of the IMSI. 
            (The digits delivered to this function are the digits of
            an IMSI, but this doesn't become obvious from the function's
            or parameter's name.)

*/

GLOBAL UBYTE mm_calculate_digits (const UBYTE *digits)
{
  UBYTE i = 0;

  TRACE_FUNCTION ("mm_calculate_digits()");

  while (digits[i] < 0x0A AND i < 16)
    i++;

  return i;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_loc_upd_rej             |
+--------------------------------------------------------------------+

  PURPOSE : This function is called if LOCATION UPDATING REJECT has 
            been received and then after the RR connection has been 
            released or aborted, after leaving MM_LUP_REJECTED state.
            It is also used if MMGMM_ATTACH_REJ_REQ is received by GMM.
            It is also used if MMGMM_NREG_REQ is received with
            cs EQ CS_DISABLE and certain mmgmm_error_cause values.
            Maybe the name "mm_network_rejected()" would be a 
            better name for the function now.

            NOTE: For MM's own location updating, every non-defined 
                  cause is mapped into RC_SERVICE_ORDER (0x22). 
                  If the network sends some GPRS only defined cause 
                  by means of the LOCATION UPDATING REJECT message, 
                  we catch this in the default.

*/

GLOBAL void mm_loc_upd_rej (void)
{
  GET_INSTANCE_DATA;
  T_plmn hplmn;

  TRACE_FUNCTION ("mm_loc_upd_rej ()");

  reg_extract_hplmn (&hplmn);

  TRACE_EVENT_P1 ("rej_cause = %04X", mm_data->rej_cause);

  /* 
   * T3212 is stopped if a LOCATION UPDATING ACCEPT or 
   * LOCATION UPDATING REJECT message is received.
   * [GSM 04.08 subclause 4.4.2]
   * Exception: MMGMM_ERRCS_GPRS_NOT_ALLOWED_IN_PLMN as this shall not
   * have any influence on T3212.
   */
#ifdef GPRS
  if (mm_data->rej_cause NEQ GMMCS_GPRS_NOT_ALLOWED_IN_PLMN)
#endif /* #ifdef GPRS */
  {
    TIMERSTOP (T3212);
    mm_data->t3212_timeout = FALSE;
  }

  switch (mm_data->rej_cause)
  {
    case MMCS_PLMN_NOT_ALLOWED: /* #11 + Ofs */
#ifdef GPRS
    case GMMCS_PLMN_NOT_ALLOWED:
#endif /* #ifdef GPRS */
      mm_data->attempt_cnt = 0;
      mm_data->loc_upd_type.lut = NOT_RUNNING;
      if (reg_plmn_equal_hplmn (&mm_data->reg.actual_plmn) EQ FALSE)
      {
        /*
         * The PLMN is only added to forbidden list if not the HPLMN.
         * An internal error of the HPLMN would otherwise invalidate
         * the SIM for the HPLMN even after SIM removal/SIM insertion.
         */
        reg_plmn_add_bad (mm_data->reg.forb_plmn, 
                          MAX_FORB_PLMN_ID,
                          &mm_data->reg.actual_plmn);
        mm_build_rr_sync_req_cause (SYNCCS_LIMITED_SERVICE);
        mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL);

        /* Delete EPLMN List */
        if (reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE))
          mm_build_rr_sync_req_cause(SYNCCS_EPLMN_LIST);
#ifdef REL99
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
#else
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED);
#endif
        /* Set the new MM state before calling reg_mm_failure() */
        SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
        reg_mm_failure (FORB_PLMN_INCLUDED);
      }
      else
      {
        /* 
         * Cause #11 for the HPLMN. This is a network failure.
         */

        // Call mm_lup_restart() instead? Makes no sense this way...

        mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL);

        /* Delete EPLMN List */
        if (reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE))
          mm_build_rr_sync_req_cause(SYNCCS_EPLMN_LIST);
        
#ifdef REL99
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
#else
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED);
#endif
        /* Set the new MM state before calling reg_mm_failure() */
        SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
        reg_mm_failure (FORB_PLMN_NOT_INCLUDED);
      }

      USE_STORED_ENTRIES();
      break;

    case MMCS_LA_NOT_ALLOWED: /* #12 + Ofs */
#ifdef GPRS
    case GMMCS_LA_NOT_ALLOWED:
#endif /* #ifdef GPRS */
      /* 
       * There is no list of "forbidden location areas for regional
       * provision of service" supported by G23. Compare this with the 
       * requirements of GSM 04.08 subclause 4.4.4.7.
       * The non-implementation of this list should not cause any 
       * harm for cause #12.
       */
      mm_data->attempt_cnt = 0;
      mm_build_rr_sync_req_cause (SYNCCS_LAI_NOT_ALLOW);
      mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL);

      mm_data->loc_upd_type.lut = NOT_RUNNING;
#ifdef REL99
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
#else
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED);
#endif
      /* Set the new MM state before calling reg_mm_failure() */
      SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
      reg_mm_failure (FORB_PLMN_NOT_INCLUDED);
      USE_STORED_ENTRIES();
      break;

    
#ifdef GPRS
    case GMMCS_ROAMING_NOT_ALLOWED:
#ifdef REL99
      /* Invalidate the TMSI, CKSN, KC in RR */
      mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL);
      case GMMCS_ROAMING_NOT_ALLOWED_WITH_RAU_REJ: /* #13 + Ofs GMM sent for GPRS RAU rejected case*/
      /* GMM sent cause value 13 becauseof 'Normal/periodic RAU Rejected' or 
       * 'Combined RAU Rejected by the network'.*/
      /*FALLTHROUGH*/
#endif
      /*FALLTHROUGH*/
      //lint -fallthrough
#endif /* #ifdef GPRS */
    case MMCS_ROAMING_NOT_ALLOWED: /* #13 + Ofs */
      /* MM received cause value 13 because of 'LU rejected' by the network*/
      /*
       * For the cause 13 handling as per the specification 24.008 v3.15.0,
       * If Cause value 13 received because of 'LU rejected' or 'Normal/periodic RAU
       * Rejected' or 'Combined RAU Rejected by the network' MM will
       *   - store the LAI in the list of "forbidden location areas for roaming".
       *   - set the update status to U3 ROAMING NOT ALLOWED and shall reset the
       *     location update attempt counter
       *   - The mobile station shall perform a PLMN selection instead of a cell
       *     selection when back to the MM IDLE state according to 3GPP TS 23.122.
       *
       * If cause value 13 received because of 'GPRS attach rejected' or 'Combined
       * GPRS rejected' or 'Network initiated GPRS detach' MM will
       *   - Do the above steps
       *   - Additionally MM will any TMSI and ciphering key sequence number.
       */
      mm_data->attempt_cnt = 0;
      mm_data->loc_upd_type.lut = NOT_RUNNING;

      /* Remember the PLMN which sent this reject cause, it's low prio now */
      mm_data->reg.low_prio_plmn = mm_data->reg.actual_plmn; /* Struct copy */
      
      /* Enter the location area in RR into the list of 
       * "forbidden location areas for roaming" for reject cause #13.
       * This list is maintained in RR */
      mm_build_rr_sync_req_cause (SYNCCS_LAI_NOT_ALLOW_FOR_ROAMING);

      /* Inform RR about the limited service condition */
      mm_build_rr_sync_req_cause (SYNCCS_LIMITED_SERVICE);

      /* As RR_SYNC_REQ (SYNCCS_LIMITED_SERVICE) clears the PLMN in RR, 
       * we do the same with the actual PLMN in MM. */
      reg_clear_plmn (&mm_data->reg.actual_plmn);
#ifndef REL99
      /* Invalidate the TMSI, CKSN, KC in RR */
      mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL);
#endif
      /* Delete EPLMN List */
      if (reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE))
        mm_build_rr_sync_req_cause(SYNCCS_EPLMN_LIST);

#ifdef REL99
#ifdef GPRS
      /* Following will check if the TMSI, CKSN, KC needs to be deleted */
      if(mm_data->rej_cause EQ GMMCS_ROAMING_NOT_ALLOWED)
      {
        /*
         * Update the MM state with invalidate the TMSI, CKSN, KC in MM
         * and send indication to SIM.
         */
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
      }
      else
#endif /* #ifdef GPRS */
      {
        /*
         * update the MM state but don't delete the TMSI, CKSN, KC 
         * and send indication to SIM.
         */
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, TRUE);
      }

#else
      /* Inform the SIM */
      reg_invalidate_upd_state (MS_LA_NOT_ALLOWED);
#endif
      /* Set the new MM state before calling reg_mm_failure() */
      SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
      reg_mm_failure (FORB_PLMN_NOT_INCLUDED);
      USE_STORED_ENTRIES();
      break;

#ifdef REL99
#ifdef GPRS
    case GMMCS_NO_SUITABLE_CELL_IN_LA: /* #15 + Ofs GMM sent for GPRS attach rejected case*/
      /* GMM sent cause value 15 because of 'GPRS attach rejected' or 'Combined GPRS rejected'
       * or 'Network initiated GPRS detach'.*/
      /* Invalidate the TMSI, CKSN, KC in RR */
      mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL);
      /*FALLTHROUGH*/
    case GMMCS_NO_SUITABLE_CELL_IN_LA_WITH_RAU_REJ: /* #15 + Ofs GMM sent for GPRS RAU rejected case*/
      /* GMM sent cause value 15 because of 'Normal/periodic RAU Rejected' or 
       * 'Combined RAU Rejected by the network'.*/
      /*FALLTHROUGH*/
      //lint -fallthrough
#endif
    case MMCS_NO_SUITABLE_CELL_IN_LA: /* #15 + Ofs MM received in LU rejected message*/
      /*MM received cause value 15 because of 'LU rejected' by the network*/
      /*
       * For cause 15 handling as per the specification 24.008 v3.15.0,
       * If Cause value 15 received because of 'LU rejected' or 'Normal/periodic RAU
       * Rejected' or 'Combined RAU Rejected by the network' MM will
       *   - store the LAI in the list of "forbidden location areas for roaming".
       *   - set the update status to U3 ROAMING NOT ALLOWED and shall reset the
       *     location update attempt counter
       *   - The MS shall search for a suitable cell in another location area in
       *     the same PLMN according to 3GPP TS 03.22 and 3GPP TS 25.304.
       * If cause value 15 received because of 'GPRS attach rejected' or 'Combined
       * GPRS rejected' or 'Network initiated GPRS detach' MM will
       *   - Do the above steps
       *   - Additionally will delete any TMSI and ciphering key sequence number.
       */
      mm_data->attempt_cnt = 0;
      mm_data->loc_upd_type.lut = NOT_RUNNING;
      /*
       * Enter the location area in RR into the list of "forbidden location areas for roaming"
       * for reject cause #15. This list is maintained in RR.
       */
      mm_build_rr_sync_req_cause (SYNCCS_LAI_NOT_ALLOW_FOR_ROAMING);

      /* Set the new MM state MM_IDLE_LIMITED_SERVICE*/
      SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
      /* Following will check if the TMSI, CKSN, KC needs to be deleted */
#ifdef GPRS
      if(mm_data->rej_cause EQ GMMCS_NO_SUITABLE_CELL_IN_LA)
      {
        /*
         * Update the MM state with invalidate the TMSI, CKSN, KC in MM
         * and send indication to SIM.
         */
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
      }
      else
#endif /* #ifdef GPRS */
      {
        /*
         * update the MM state but don't delete the TMSI, CKSN, KC
         * and send indication to SIM.
         */
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, TRUE);
      }

      /* MM should send primitive MMGMM_REG_REJ to GMM only in case if cause 15 is received
       * because of its own failure i.e if there was either a RR failure or a MM failure
       * (e.g. LOCATION UPDATING reject). It should then inform GMM about failure. Upon
       * reception of MMGMM_REG_REJ primitive, GMM send failure information to MMI.
       */
      if(GET_CAUSE_ORIGIN_ENTITY (mm_data->rej_cause) EQ MM_ORIGINATING_ENTITY)
      {
        /* MM sends primitive MMGMM_REG_REJ to GMM */
        mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE,
                          SEARCH_RUNNING,
                          FORB_PLMN_NOT_INCLUDED);
      }
      else
      {
        /*Do not send NREG Indication. GMM is already unregistered*/
      }
      /*
       * Start PLMN serch, try the PLMN in the list i.e. search
       * another location area in the same PLMN.
       */
      mm_mmr_reg_req (FUNC_PLMN_SRCH);

      /*Responce back with confirmation if MMGMM_NREG_REQ was received.*/
      if (mm_data->nreg_request)
      {
        mm_mmgmm_nreg_cnf (mm_data->nreg_cause);
      }

      USE_STORED_ENTRIES();
      break;
#endif

    case MMCS_IMSI_IN_HLR: /* #2 + Ofs */
#ifdef GPRS
    case GMMCS_IMSI_UNKNOWN:
      if (!mm_lup_allowed_by_gmm())
      {
        /* 
         * The "IMSI unknown in HLR" was received by a GMM request and 
         * not by a LOCATION UPDATING REJECT message.
         * MM must not enter MM_IDLE_NO_IMSI here, as the SIM 
         * is still valid for the GPRS side and MM still has to deliver
         * some services for GMM which it cannot deliver in service state 
         * "NO IMSI". The relevant testcases here are (Release 1999):
         * GSM 11.10 subclause 44.2.1.2.2, GSM 11.10 subclause 44.2.3.2.3.
         * What is tested there is that after an ATTACH ACCEPT/
         * ROUTING AREA UPDATING ACCEPT message the mobile doesn't 
         * respond to packet paging with IMSI anymore and that no DETACH 
         * is performed for GSM.
         * What is not tested is that the mobile station doesn't respond to 
         * paging with IMSI on the old channels after entering an area where 
         * we have network mode II/III or that the mobile rejects a manual 
         * switch to mobile class CS without switch off. 
         * What is done here may not be perfect, but should be sufficient.
         * In fact, #2 may never be seen in field, as the network providers
         * may not want to sell GPRS subscriptions without GSM subscriptions.
         */

        /* Release all waiting CM requests */
        mm_mmxx_rel_ind (mm_data->rej_cause, CM_NOT_IDLE);
        
        /* 
         * Invalidate TMSI and ciphering parameters in RR. 
         * RR remains in full service for GPRS.
         */
        mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL);

        /* Delete EPLMN List */
        if (reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE))
          mm_build_rr_sync_req_cause(SYNCCS_EPLMN_LIST);

        /* Invalidate the update state */
#ifdef REL99
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
#else
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED);
#endif

        /* If the deregistration was caused by a MMGMM_NREG_REQ, 
         * confirm it properly */
        if (mm_data->nreg_request) 
          mm_mmgmm_nreg_cnf (mm_data->nreg_cause);

        /* Next state is a parking state */
        SET_STATE (STATE_MM, MM_IDLE_LUP_NEEDED);
        break;
      }
      /* 
       * However, if #2 is received by LOCATION UPDATING REJECT message,
       * GSM 04.08 subclause 4.4.4.7 claims that the SIM shall be considered
       * as invalid for "the mobile station" (and not only for the GSM part).
       * There is no distinction between causes #2, #3 and #6 there.
       */
      /*FALLTHROUGH*/
      //lint -fallthrough
#endif /* #ifdef GPRS */
    case MMCS_ILLEGAL_MS:  /* #3 + Ofs */
    case MMCS_ILLEGAL_ME:  /* #6 + Ofs */
#ifdef GPRS
    case GMMCS_ILLEGAL_MS:
    case GMMCS_ILLEGAL_ME:
    case GMMCS_GSM_GPRS_NOT_ALLOWED: /* #8 + Ofs */
#endif /* #ifdef GPRS */
      mm_clear_mob_ident (&mm_data->reg.imsi_struct);
      mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL_NO_PAG);

      /* Delete EPLMN List */
      if (reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE))
        mm_build_rr_sync_req_cause(SYNCCS_EPLMN_LIST);

      // Debug patch >>>
      if (mm_data->mm_idle_no_imsi_marker EQ 0)
        mm_data->mm_idle_no_imsi_marker = 19;
      // End debug patch

      mm_data->loc_upd_type.lut = NOT_RUNNING;
#ifdef REL99
      reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
#else
      reg_invalidate_upd_state (MS_LA_NOT_ALLOWED);
#endif
      /* Invalidate SIM data after indirect call to reg_build_sim_update() */
      mm_clear_reg_data ();
      mm_data->reg.plmn_cnt = 0; /* Delete list of available PLMNs */
      /* Set the new MM state before calling reg_mm_failure */
      SET_STATE (STATE_MM, MM_IDLE_NO_IMSI);
      reg_mm_failure (FORB_PLMN_NOT_INCLUDED);
      USE_STORED_ENTRIES();
      break;

#ifdef GPRS
    case GMMCS_GPRS_NOT_ALLOWED_IN_PLMN: /* #14 + Ofs */
      /* 
       * We assume this cause will not be sent by the HPLMN.
       *
       * "The MS shall store the PLMN identity in the "forbidden PLMNs for
       * GPRS service" list. A GPRS MS operating in MS operation mode C 
       * shall perform a PLMN selection instead of a cell selection."
       * [3GPP TS 04.08 version 7.13.0 and others]
       */
      reg_plmn_add_bad (mm_data->reg.gprs_forb_plmn,
                        MAX_GPRS_FORB_PLMN_ID,
                        &mm_data->reg.actual_plmn);

      if (mm_data->reg.op.m EQ MODE_AUTO AND
          mm_data->gprs.mobile_class EQ MMGMM_CLASS_CG AND
          !reg_plmn_equal_hplmn (&mm_data->reg.actual_plmn))
      {
        if (mm_data->reg.plmn_cnt EQ 0)
        {
          /* A PLMN list is not present */
          reg_extract_hplmn (&mm_data->reg.actual_plmn);
          mm_data->reg.plmn_cnt = 0; /* Delete list of available PLMNs */
          mm_mmr_reg_req (FUNC_PLMN_SRCH);
        }
        else
        {
          /* A PLMN list is present */
          if (mm_data->reg.plmn_cnt > mm_data->reg.plmn_index)
          {
            /* 
             * There are still untried entries in the PLMN list. 
             * Try the next PLMN in the list.
             */
            reg_unpack_plmn (&mm_data->reg.actual_plmn,
                             mm_data->reg.plmn, mm_data->reg.plmn_index);
            mm_data->reg.plmn_index++;
            mm_data->attempt_cnt = 0;
            mm_mmr_reg_req (FUNC_PLMN_SRCH);
          }
          else
          {
            /* 
             * There is no signalization to signal the end of search to 
             * GMM here. Maybe this has to be introduced.
             */
          }
        }
      }
      break;
#endif /* #ifdef GPRS */

    case MMCS_NETWORK_FAILURE: /* #17 + Ofs */
#ifdef GPRS
    case GMMCS_NET_FAIL:
#endif /* #ifdef GPRS */
      /*FALLTHROUGH*/
    default:

      /* Delete EPLMN List */
      if (reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE))
        mm_build_rr_sync_req_cause(SYNCCS_EPLMN_LIST);

      mm_lup_restart ();
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_lup_restart             |
+--------------------------------------------------------------------+

  PURPOSE : The purpose of this function is to perform the actions
            required after a location updating attempt has failed.
            Either T3211 or T3212 are started by this function,
            so a new location updating attempt will be started
            after expiry.

*/

GLOBAL void mm_lup_restart (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_lup_restart ()");

  mm_data->last_rej_cause = 0;
  if (mm_data->attempt_cnt < 4)
    mm_data->attempt_cnt++;
  if (mm_data->attempt_cnt < 4 AND
      mm_check_lai (&mm_data->reg.lai, &mm_data->mm.lai) AND
      mm_data->reg.update_stat EQ MS_UPDATED)
  {
    /*
     * - The update status is UPDATED, and the stored LAI is equal to the one
     *   received on the BCCH from the current serving cell and the
     *   attempt counter is smaller than 4:
     *   
     *   The mobile station shall keep the update status to UPDATED, the MM IDLE
     *   sub-state after the RR connection release is NORMAL SERVICE. 
     * (GSM 04.08 4.4.4.9)
     */ 
    TIMERSTART (T3211, T_3211_VALUE);
    SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);

#ifdef GPRS
    if (mm_data->gprs.sim_physically_removed)
    {
      mm_sim_removed_gprs_active ();
      return;
    }
#endif /* #ifdef GPRS */

  }
  else
  {
    /* 
     * - Either the update status is different from UPDATED, or the stored 
     *   LAI is different from the one received by the on the BCCH from the 
     *   serving cell, or the attempt counter is greater or equal to 4:
     * 
     *   The mobile station shall delete any LAI, TMSI, ciphering key sequence
     *   number stored in the SIM, set the update status to NOT UPDATED and 
     *   enter the MM IDLE sub-state ATTEMPTING TO UPDATE or optionally the 
     *   MM IDLE sub-state PLMN SEARCH in order to perform a PLMN selection 
     *   according to 3GPP TS 23.122 when the RR connection is released. If 
     *   the attempt counter is smaller than 4, the mobile station shall 
     *   memorize that timer T3211 is to be started when the RR connection is 
     *   released, otherwise it shall memorize that timer T3212 is to be started
     *   when the RR connection is released. 
     * (GSM 24.008 4.4.4.9)
     */
    mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL);
#ifdef REL99
    reg_invalidate_upd_state (MS_NOT_UPDATED, FALSE);
#else
    reg_invalidate_upd_state (MS_NOT_UPDATED);
#endif

    /* Set the new state now, vital for reg_plmn_select() */
    SET_STATE (STATE_MM, MM_IDLE_ATTEMPT_TO_UPDATE);

#ifdef GPRS
    if (mm_data->gprs.sim_physically_removed)
    {
      mm_sim_removed_gprs_active ();
      return;
    }
#endif /* #ifdef GPRS */

    if (mm_data->attempt_cnt < 4)
    {
      TIMERSTART (T3211, T_3211_VALUE);
    }
    else
    {
      mm_mmxx_rel_ind (MMCS_NO_REGISTRATION, CM_NOT_IDLE);
      mm_start_t3212_bcch ();
      mm_data->loc_upd_type.lut = NOT_RUNNING;

      if (mm_lup_allowed_by_gmm())
      {
        if (mm_data->reg.plmn_cnt > mm_data->reg.plmn_index)
        {
          /*
           * Select the next PLMN in the list, if available.
           * This is recommended in GSM 03.22 subclause 4.4.4.
           * "When in Automatic Network Selection mode and the MS is in the 
           * "not updated" state with one or more suitable cells to camp on;
           * then after the maximum allowed unsuccessful LR requests (controlled
           * by the specific attempt counters) the MS may continue (or start if 
           * not running) the user reselection procedure of 4.4.3.2 A."
           */
          reg_plmn_select (FORB_PLMN_NOT_INCLUDED);
        }
        else if (mm_data->reg.op.m EQ MODE_AUTO AND
                 mm_data->reg.op.sim_ins EQ SIM_INSRT AND
                 mm_data->reg.quick_hplmn_search EQ TRUE)
        {
          /* The state PLMN SEARCH is also entered in the following cases:
           * - optionally, when the mobile station is in the ATTEMPTING TO UPDATE 
           * state and is in Automatic Network Selection mode and location update 
           * attempt counter is greater than or equal to 4.
           * (GSM 24.008 4.2.1.2)
           */
          /* optionally the MM IDLE sub-state PLMN SEARCH in order to perform a 
           * PLMN selection according to 3GPP TS 23.122
           * (GSM 24.008 4.4.4.9)
           */
          mm_data->plmn_scan_mm = TRUE;
          mm_data->reg.quick_hplmn_search = FALSE;
          mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                       SEARCH_RUNNING,
                       FORB_PLMN_NOT_INCLUDED);
          mm_func_mmgmm_net_req();
        }
        else
        {
          mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                             SEARCH_NOT_RUNNING,
                             FORB_PLMN_NOT_INCLUDED);
          mm_data->reg.quick_hplmn_search = TRUE;
        }
      }

      if (mm_data->reg.op.sim_ins EQ SIM_INSRT AND 
          mm_data->reg.op.ts EQ TS_NO_AVAIL)
      {
        /*
         * If there is no test SIM inserted and after 4 unsuccessfull 
         * registration attempts, the registration timer is started.
         * On timeout of the registration timer, *one* normal location 
         * updating is started.
         * Without this, MM may stay in MM_IDLE_ATTEMPT_TO_UPDATE 
         * for an infinite time. (No call attempt, no cell selection, 
         * no periodic location updating)
         */
        TIMERSTART (T_REGISTRATION, T_REG_VALUE);
      }
    }
  }
  reg_check_hplmn_tim (mm_data->reg.thplmn);
  USE_STORED_ENTRIES();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mdl_rel_req             |
+--------------------------------------------------------------------+

  PURPOSE : This function sends an MDL_RELEASE_REQ for SAPI 0 to the 
            data link layer. Usually this is done after RR_RELEASE_IND 
            or RR_ABORT_IND is received by RR. 
            One really good question is why this is handled in MM and
            not in RR. But this doesn't matter as it works the way it
            is implemented.

*/

GLOBAL void mm_mdl_rel_req (void)
{
  PALLOC (mdl_release_req, MDL_RELEASE_REQ); /* T_MDL_RELEASE_REQ */
  TRACE_FUNCTION ("mm_mdl_rel_req ()");

  mdl_release_req->sapi = SAPI_0;
  mdl_release_req->ch_type = NOT_PRESENT_8BIT;

  PSENDX (DL, mdl_release_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mdl_rel_req_sapi_3      |
+--------------------------------------------------------------------+

  PURPOSE : This function sends an MDL_RELEASE_REQ for SAPI 3 to the 
            data link layer. Usually this is done after RR_RELEASE_IND 
            or RR_ABORT_IND is received by RR. 
            One really good question is why this is handled in MM and
            not in RR. But this doesn't matter as it works the way it
            is implemented.

*/

GLOBAL void mm_mdl_rel_req_sapi_3 (void)
{
  PALLOC (mdl_release_req, MDL_RELEASE_REQ);

  TRACE_FUNCTION ("mm_mdl_rel_req_sapi_3 ()");

  mdl_release_req->sapi = SAPI_3;
  mdl_release_req->ch_type = NOT_PRESENT_8BIT;

  PSENDX (DL, mdl_release_req);
}
   

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmsms_rel_ind           |
+--------------------------------------------------------------------+

  PURPOSE : This functions releases the SMS connections (SAPI 3) 
            of the required connection type to the SMS entitity. This 
            function is used if MM receives a RR_RELEASE_IND primitive
            for SAPI 3.

*/

GLOBAL void mm_mmsms_rel_ind (USHORT cause, UBYTE conn_typ)
{
  GET_INSTANCE_DATA;
  UBYTE ti;

  TRACE_FUNCTION ("mm_mmsms_rel_ind ()");
    
  for (ti = 0; ti < NUM_OF_CONN_PER_CM_ENT; ti++)
  {
    switch (conn_typ)
    {
      case CM_PENDING:
      case CM_ACTIVE:
      case CM_REEST_PENDING:
        if (mm_data->conn_state[SMS_COMP][ti] NEQ conn_typ)
          continue;
        /*FALLTHROUGH*/
        //lint -fallthrough
      default: /* all connections */
        if (mm_data->conn_state[SMS_COMP][ti] EQ CM_IDLE)
          continue;
        CM_SET_STATE (SMS_COMP, ti, CM_IDLE);
        mm_mmxx_release_ind (SMS_COMP, ti, cause);
        mm_delete_entry (SMS_COMP, ti);
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmxx_err_ind            |
+--------------------------------------------------------------------+

  PURPOSE : This function is called if a radio link error occurs, 
            this means, the RR_ABORT_IND primitive was received 
            while active connections existed. Upper layers are 
            notificated about the failure. The function returns the 
            number of active connections found (CM_ACTIVE).

*/

GLOBAL SHORT mm_mmxx_err_ind (USHORT cause)
{
  GET_INSTANCE_DATA;
  UBYTE count = 0;
  UBYTE cm;
  UBYTE ti;

  TRACE_FUNCTION ("mm_mmxx_err_ind ()");
  for (cm = 0; cm < NUM_OF_CM_ENT; cm++)
  {
    for ( ti = 0; ti < NUM_OF_CONN_PER_CM_ENT; ti++)
    {
      if (CMSTATE(cm, ti) EQ CM_ACTIVE)
      {
        count++;
        switch (cm)
        {
          case CC_COMP:
          {
            PALLOC (mmcm_error_ind, MMCM_ERROR_IND);
            mmcm_error_ind->ti    = ti;
            mmcm_error_ind->cause = cause;
            PSENDX (CC, mmcm_error_ind);
            break;
          }
          case SS_COMP:
          {
            PALLOC (mmss_error_ind, MMSS_ERROR_IND);
            mmss_error_ind->ti    = ti;
            mmss_error_ind->cause = cause;
            PSENDX (SS, mmss_error_ind);
            break;
          }
          case SMS_COMP:
          {
            PALLOC (mmsms_error_ind, MMSMS_ERROR_IND);
            mmsms_error_ind->ti    = ti;
            mmsms_error_ind->cause = cause;
            PSENDX (SMS, mmsms_error_ind);
            break;
          }
          default:
            TRACE_ERROR ("Illegal cm comp");
            break;
        }
      }
    }
  }
  return (count);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmxx_est_cnf            |
+--------------------------------------------------------------------+

  PURPOSE : Handle CM SERVICE ACCEPT / completion of ciphering mode
            setting for connections requested by CM entities. 
            Two different cases, call establishment and
            call reestablishment are handled here.

*/

GLOBAL void mm_mmxx_est_cnf (void)
{
  GET_INSTANCE_DATA;
  UBYTE ti;

  TRACE_FUNCTION ("mm_mmxx_est_cnf ()");
  {
    if (mm_count_connections (CM_REEST_PENDING) EQ 0)
    {
      /* 
       * No connections waiting for reestablishment. The establishment
       * was confirmed for a pending connection. Set the state of the 
       * pending connection to CM_ACTIVE as it is now not pending anymore.
       * Notify the CM entitity which requested the connection
       * about the successful establishment. 
       * (There can only be one pending connection at a given time).
       */

      CM_SET_STATE (mm_data->pend_conn.comp,
                    mm_data->pend_conn.ti, CM_ACTIVE);

      switch (mm_data->pend_conn.comp)
      {
        case CC_COMP:
          {
            PALLOC (mmcm_establish_cnf, MMCM_ESTABLISH_CNF);
            mmcm_establish_cnf->ti = mm_data->pend_conn.ti;
            PSENDX (CC, mmcm_establish_cnf);
          }
          break;
        case SS_COMP:
          {
            PALLOC (mmss_establish_cnf, MMSS_ESTABLISH_CNF);
            mmss_establish_cnf->ti = mm_data->pend_conn.ti;
            PSENDX (SS, mmss_establish_cnf);
          }
          break;
        case SMS_COMP:
          {
            PALLOC (mmsms_establish_cnf, MMSMS_ESTABLISH_CNF);
            mmsms_establish_cnf->ti = mm_data->pend_conn.ti;
            PSENDX (SMS, mmsms_establish_cnf);
          }
          break;
        default:
          TRACE_ERROR (UNEXPECTED_DEFAULT); /* Cannot happen */
          break;
      }
    }
    else
    {
      /* 
       * Reestablishment pending. Only the CM entitiy CC can perform call
       * reestablishment, so only CC needs to be treated here.
       */
      for (ti = 0; ti < NUM_OF_CONN_PER_CM_ENT; ti++)
      {
        if (mm_data->conn_state[CC_COMP][ti] EQ CM_REEST_PENDING)
        {
          PALLOC (mmcm_reestablish_cnf, MMCM_REESTABLISH_CNF);
          CM_SET_STATE (CC_COMP, ti, CM_ACTIVE);
          mmcm_reestablish_cnf->ti = ti;
          PSENDX (CC, mmcm_reestablish_cnf);
        }
      }
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmxx_release_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Send either MMCM_RELEASE_IND, MMSS_RELEASE_IND or
            MMSMS_RELEASE_IND, dependend of the CM component.

*/

GLOBAL void mm_mmxx_release_ind (UBYTE comp, UBYTE ti, USHORT relcs)
{
  TRACE_FUNCTION ("mm_mmxx_release_ind()");

  switch (comp)
  {
    case CC_COMP:
      {
        PALLOC (mmcm_release_ind, MMCM_RELEASE_IND); /* T_MMCC_RELEASE_IND */
        mmcm_release_ind->ti    = ti;
        mmcm_release_ind->cause = relcs;
        PSENDX (CC, mmcm_release_ind);
      }
      break;

    case SS_COMP:
      {
        PALLOC (mmss_release_ind, MMSS_RELEASE_IND); /* T_MMSS_RELEASE_IND */
        mmss_release_ind->ti    = ti;
        mmss_release_ind->cause = relcs;
        PSENDX (SS, mmss_release_ind);
      }
      break;

    case SMS_COMP:
      {
        PALLOC (mmsms_release_ind, MMSMS_RELEASE_IND); /* T_MMSMS_RELEASE_IND */
        mmsms_release_ind->ti    = ti;
        mmsms_release_ind->cause = relcs;
        PSENDX (SMS, mmsms_release_ind);
      }
      break;

    default: 
      TRACE_ERROR (UNEXPECTED_PARAMETER);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmxx_rel_ind            |
+--------------------------------------------------------------------+

  PURPOSE : The mm_mmxx_rel_ind() function releases all CM connections 
            of a given ´given type. Type may be CM_PENDING, 
            CM_ACTIVE, CM_REEST_PENDING and CM_NOT_IDLE.

*/

GLOBAL void mm_mmxx_rel_ind (USHORT cause, UBYTE conn_typ)
{
  GET_INSTANCE_DATA;
  UBYTE comp;
  UBYTE ti;

  TRACE_FUNCTION ("mm_mmxx_rel_ind ()");

  for (comp = 0; comp < NUM_OF_CM_ENT; comp++)
  {
    for (ti = 0; ti < NUM_OF_CONN_PER_CM_ENT; ti++)
    {
      switch (conn_typ)
      {
        case CM_PENDING:
        case CM_ACTIVE:
        case CM_REEST_PENDING:
          if (CMSTATE(comp, ti) NEQ conn_typ)
            continue;
          /*FALLTHROUGH*/
          //lint -fallthrough
        default: /* CM_NOT_IDLE */
          if (CMSTATE(comp, ti) EQ CM_IDLE)
            continue;
          CM_SET_STATE (comp, ti, CM_IDLE);
          mm_mmxx_release_ind (comp, ti, cause);
          mm_delete_entry (comp, ti);
      }
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_power_off               |
+--------------------------------------------------------------------+

  PURPOSE : This function deactivates the lower layers 
            and enters the MM NULL state.

*/

GLOBAL void mm_power_off (void)
{
  GET_INSTANCE_DATA;
  USHORT index;

  TRACE_FUNCTION ("mm_power_off ()");

  /* Send RR_DEACTIVATE_REQ */
  {
    PALLOC (rr_deactivate_req, RR_DEACTIVATE_REQ);
    PSENDX (RR, rr_deactivate_req);
  }
  
  /* Invalidate the registration data temporary only without deleting it. */
  mm_clear_reg_data ();

  /* Stop all MM timers */
  for (index = 0; index < NUM_OF_MM_TIMERS; index++)
  {
    TIMERSTOP (index);
  }

  mm_data->t3212_timeout = FALSE;

  /* MM enters MM_NULL state, but the important MM data is still alive. */
  SET_STATE (STATE_MM, MM_NULL);
#ifdef GPRS
  SET_STATE (STATE_REG_TYPE, REG_GPRS_INACTIVE);
  SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_OK);
#endif /* GPRS */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_random                  |
+--------------------------------------------------------------------+

  PURPOSE : A random number generator.

*/

GLOBAL USHORT mm_random (USHORT    n)
{
  GET_INSTANCE_DATA;
  T_TIME actual_time;

  TRACE_FUNCTION ("mm_random ()");

  vsi_t_time (VSI_CALLER &actual_time);
  mm_data->rand_base += actual_time;
  return ((USHORT) mm_data->rand_base % n);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_read_entry              |
+--------------------------------------------------------------------+

  PURPOSE : This functions reads the entries which are flagged as 
            "USE" and executes them. This is done by:
            - rebuilding the original primitives and sending them to 
            the MM entitity in case of content_type "EVENT_ENTRY".
            - calling the respective timeout handling routine for 
            timers (content_type "EVENT_ENTRY"),
            - resending the stored primitive (content_type "PRIMITIVE_ENTRY")

*/

GLOBAL void mm_read_entry (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_read_entry ()");
                          
  if (mm_data->store[0].use EQ USE)
  {
    USHORT i;
    T_STORE current;

    current = mm_data->store[0]; /* Struct copy */

    for (i = 0; i < MAX_STORE_ENTRIES - 1; i++)
    {
      mm_data->store[i] = mm_data->store[i + 1]; /* Struct copy */
    }
    mm_data->store[MAX_STORE_ENTRIES - 1].use = ENTRY_FREE;
    mm_data->store[MAX_STORE_ENTRIES - 1].content_type = NO_ENTRY;

    TRACE_EVENT_P2 ("Read entry: comp=%d, ti=%d", current.content.event.comp, current.content.event.ti);

    if (current.content_type EQ EVENT_ENTRY)
    {
      switch (current.content.event.comp)
      {
        case TIMEOUT:
          switch (current.content.event.ti) /* TI used as timer index here */
          {
            case T3210:  tim_t3210 ();  break;
            case T3211:  tim_t3211 ();  break;
            case T3212:  tim_t3212 ();  break;
            case T3213:  tim_t3213 ();  break;
            case T3220:  tim_t3220 ();  break;
            case T3230:  tim_t3230 ();  break;
            case T3240:  tim_t3240 ();  break;
#ifdef REL99
            case T3241:  tim_t3241 ();  break;
#endif
            default: 
              TRACE_EVENT (UNEXPECTED_DEFAULT);
              break;
          }
          break;

        case REG_COMP:
          mm_func_mmgmm_net_req ();
          break;

        case SS_COMP:
        case SMS_COMP:
          current.content.event.estcs = 0; /* Override estcs */
          /*FALLTHROUGH*/
          //lint -fallthrough
        case CC_COMP:
          CM_SET_STATE (current.content.event.comp, current.content.event.ti, CM_IDLE);
          mm_mmxx_establish_req (current.content.event.comp, current.content.event.ti, current.content.event.estcs, current.info);
          break;

        default:
          TRACE_ERROR (UNEXPECTED_DEFAULT);
          break;
      } /* switch comp */
    } /* if content_type */
    else
    {
      /* the following code will be used if additional primitives
       * will be stored this way or a generic handling for storage
       * and retrieval of elements is requested.
       */
      switch (((T_PRIM_HEADER *)current.content.primitive)->opc)
      {
        case RR_ACTIVATE_IND:
       mm_rr_activate_ind((T_RR_ACTIVATE_IND *) P2D(current.content.primitive));
          break;

        case RR_ABORT_IND:
          mm_rr_abort_ind((T_RR_ABORT_IND *) P2D(current.content.primitive));
          break;

        default:
          TRACE_ERROR (UNEXPECTED_DEFAULT);
          break;
      }
    }/* if content_type */
  } /* if use */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_reest                   |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL void mm_reest (UBYTE ti)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_reest ()");
  if (mm_data->reg.op.func EQ FUNC_LIM_SERV_ST_SRCH)
  {
    mm_mmxx_rel_ind (MMCS_NO_REGISTRATION, CM_NOT_IDLE);
    mm_sim_set_imsi_marker( MSG_MM_REE);
  }
  else
  {
    mm_rr_est_req (ESTCS_CAL_REEST, CALL_SERVICE, ti);
    SET_STATE (STATE_MM, MM_WAIT_FOR_RR_CONN_MM);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_release_rr_connection   |
+--------------------------------------------------------------------+

  PURPOSE : This functions releases the RR connection after a 
            CM connection / CM connection attempt or after a 
            successfull location updating procedure.
            It can be assumed that if this function is entered MM 
            has better service than no service at all (no state 19.5). 
            It can also be assumed that the mobile is not currently 
            performing a network search, so the following IDLE state 
            will not be one of the IDLE search states 19.7 and 19.8.

*/

GLOBAL void mm_release_rr_connection (UBYTE resumption)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_release_rr_connection ()");

  /* 
   * We are here after either an CM service (attempt) or 
   * after a successfull location updating procedure.
   * The possible remaining states from this point are:
   * 
   * - MM_IDLE_NORMAL_SERVICE, 19.1
   * - MM_IDLE_ATTEMPT_TO_UPDATE, 19.2
   * - MM_IDLE_LIMITED_SERVICE, 19.3
   * - MM_IDLE_NO_IMSI, 19.4 
   * - MM_IDLE_LUP_NEEDED, 19.6 (GPRS only)
   */

#ifdef GPRS
  if (mm_data->gprs.reg_cnf_on_idle_entry AND
      mm_count_connections (CM_NOT_IDLE) EQ 0)
  {
    /* Inform GMM about successful end of registration procedure */
    mm_data->reg.full_service_indicated = FALSE; /* Force MMGMM_REG_CNF */
    mm_mmgmm_reg_cnf ();
  }

  switch (GET_STATE (STATE_REG_TYPE))
  {
    case REG_GPRS_INACTIVE:
      break; /* No further signalling for GMM needed. GPRS is off */

    case REG_REMOTE_CONTROLLED:
      if (mm_data->gprs.sim_physically_removed AND mm_normal_upd_needed())
      {
        /* 
         * Indicate end of RC update to GMM after SIM removal while also
         * engaged in a call, the MS is not updated in the location area.
         */
        mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                           SEARCH_NOT_RUNNING,
                           FORB_PLMN_NOT_INCLUDED);
      }
      break;

    case REG_CELL_SEARCH_ONLY:
      /* Inform GMM about release of last CM connection, if any */
      mm_mmgmm_cm_release_ind (resumption);
      break;

    default:
      TRACE_ERROR (UNEXPECTED_DEFAULT);
      break;
  }
#endif /* GPRS */

  if (mm_data->reg.op.sim_ins EQ SIM_NO_INSRT)
  {
    /* 
     * The SIM is considered invalid or is not present at all.
     */
    if (mm_data->idle_substate NEQ MM_IDLE_NO_IMSI)
    {
      /* 
       * The SIM has recently been invalidated,
       * Indicate the new service to MMI 
       */   
      mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                         SEARCH_NOT_RUNNING,
                         FORB_PLMN_NOT_INCLUDED);
    }
    
    // Find original place where MM entered MM_IDLE_NO_IMSI state >>>
    if (mm_data->mm_idle_no_imsi_marker EQ 0)
      mm_data->mm_idle_no_imsi_marker = 4;
    // End of debugging patch <<<
    SET_STATE (STATE_MM, MM_IDLE_NO_IMSI);
    USE_STORED_ENTRIES();
    return;
  }

  /* 
   * If we are here the next state is not MM_IDLE_NO_IMSI.
   */
  
  if (mm_data->idle_substate EQ MM_IDLE_LIMITED_SERVICE)
  {
    /* 
     * If we are here we come from an emergency call (attempt) 
     * and had previously limited service only. So we have to 
     * re-enter limited service state here.
     */
    SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
    USE_STORED_ENTRIES();
    return;
  }

#ifdef GPRS 
  if (!mm_lup_allowed_by_gmm () AND
      (mm_normal_upd_needed () OR mm_attach_upd_needed ()))
  {
    /* 
     * MM needs location updating, but is not allowed by GMM to start it.
     */
    if (mm_data->rej_cause EQ MMCS_IMSI_IN_VLR)
    {
      /* 
       * Mobile has no registration in VLR. 
       * Normal update after CM SERVICE REJECT with RC_IMSI_IN_VLR needed.
       */

      /* Throw out all calls to force sending of MMGMM_CM_RELEASE_IND. */
      mm_mmxx_rel_ind (mm_data->rej_cause, CM_NOT_IDLE);
      mm_mmgmm_cm_release_ind (resumption);

      /* Reset the exceptional condition */
      mm_data->rej_cause = 0;

      /* Inform GMM about lost registration in network */
      mm_mmgmm_lup_needed_ind (MMGMM_IMSI_IN_VLR);
    }

    SET_STATE (STATE_MM, MM_IDLE_LUP_NEEDED);
    USE_STORED_ENTRIES();
    return;
  }
#endif /* GPRS */

  /* 
   * Here the remaining possible states are:
   * 
   * - MM_IDLE_NORMAL_SERVICE, 19.1
   * - MM_IDLE_ATTEMPT_TO_UPDATE, 19.2
   * - MM_IDLE_LIMITED_SERVICE, 19.3 (?) // This is not possible
   */

  /* After CM connection (attempt), restart T3212 if necessary */
  mm_start_t3212_bcch ();

  /* 
   * Find appropriate IDLE state:
   * Updated anywhere => MM_IDLE_NORMAL_SERVICE 
   * Updated nowhere  => MM_IDLE_ATTEMPT_TO_UPDATE
   */
  if (mm_data->reg.update_stat EQ MS_UPDATED)
  {
    SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
  }
  else
  {
    SET_STATE (STATE_MM, MM_IDLE_ATTEMPT_TO_UPDATE);
  }

  // There may be still some problems if we have a clash 
  // RR_ESTABLISH_REQ -> RR_ACTIVATE_IND -> RR_ESTABLISH_CNF -> RR_RELEASE_IND
  // It could be a good idea to repeat the RR_ACTIVATE_IND in this case at 
  // exactly this place and return. To be checked...
  
  /* 
   * Check whether T3211 or T3213 are running, if so, the updating
   * will be delayed until the respective timer expires.
   */
  if (!TIMERACTIVE (T3211) AND !TIMERACTIVE(T3213))
  {
    mm_continue_running_update ();
  }
  USE_STORED_ENTRIES();
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_act_req              |
+--------------------------------------------------------------------+

  PURPOSE : This function activates the RR sublayer

*/

GLOBAL void mm_rr_act_req (void)
{
  GET_INSTANCE_DATA;

  PALLOC (rr_activate_req, RR_ACTIVATE_REQ); /* T_RR_ACTIVATE_REQ */

  TRACE_FUNCTION ("mm_rr_act_req()");

  TIMERSTOP (T3211);
  TIMERSTOP (T3213);
  mm_data->t3213_restart = 0;
  
  memcpy (&rr_activate_req->op, &mm_data->reg.op, sizeof (T_op));
  rr_activate_req->op.service = 0;  /* not used */
  rr_activate_req->cell_test  = mm_data->reg.cell_test;
  rr_activate_req->check_hplmn = CHECK_PLMN_NOT_PRES;

  switch (rr_activate_req->op.func)
  {
    case FUNC_LIM_SERV_ST_SRCH:
      mm_data->loc_upd_type.lut = NOT_RUNNING;

      //lint -fallthrough
    case FUNC_ST_PWR_SCAN:
    case FUNC_NET_SRCH_BY_MMI:
      rr_activate_req->cksn = CKSN_RES;
      rr_activate_req->accc = 0;
      memset (&rr_activate_req->plmn, 0x0F,
              sizeof (T_plmn));
      rr_activate_req->plmn.v_plmn = V_PLMN_NOT_PRES;
      memset (&rr_activate_req->kcv, 0,
              sizeof (T_kcv));
      memset (&rr_activate_req->imsi_struct, 0,
              sizeof (T_imsi_struct));
      memset (&rr_activate_req->tmsi_struct, 0,
              sizeof (T_tmsi_struct));
      memset (&rr_activate_req->bcch_info, 0,
              sizeof (T_bcch_info));
      break;

    case FUNC_PLMN_SRCH:
      rr_activate_req->cksn = mm_data->reg.cksn;
      rr_activate_req->accc = mm_data->reg.acc_class;
      memcpy (&rr_activate_req->plmn, &mm_data->reg.actual_plmn,
              sizeof (T_plmn));
      rr_activate_req->kcv.v_kc = V_KC_PRES;
      memcpy (rr_activate_req->kcv.kc, mm_data->reg.kc, 8);
      rr_activate_req->accc = mm_data->reg.acc_class;
      memcpy (&rr_activate_req->imsi_struct, &mm_data->reg.imsi_struct,
              sizeof (T_imsi_struct));
      
      memset (rr_activate_req->tmsi_struct.id, 0, 
              sizeof (rr_activate_req->tmsi_struct.id));
      if (mm_data->reg.tmsi EQ TMSI_INVALID_VALUE)
      {
        rr_activate_req->tmsi_struct.v_mid    = V_MID_NOT_PRES;
        rr_activate_req->tmsi_struct.id_type  = TYPE_NO_ID;
        rr_activate_req->tmsi_struct.tmsi_dig = 0;
      }
      else
      {
        rr_activate_req->tmsi_struct.v_mid    = V_MID_PRES;
        rr_activate_req->tmsi_struct.id_type  = TYPE_TMSI;
        rr_activate_req->tmsi_struct.tmsi_dig = mm_data->reg.tmsi;
      }
       	

      // Question: Did these testcases fail only because there was a bug?
      // This has still to be tested on the Anite! ...
      // And bcch_encode is suspect here. Maybe the variable could be 
      // eliminated and instead the expression 
      // "(mm_data->reg.update_stat EQ MS_UPDATED AND 
      //  !reg_plmn_empty (&last_plmn))" be used. ...

      /*
       * stored cell selection only in the field
       * else 20.4 and 20.5 on the Anite will fail
       * Exception: the boolean variable 'config_use_stored_bcch' is set to a 
       * value of TRUE by the config command: CONFIG USE_STORED_BCCH
       * or the AT command: AT%CPRIM="MM","CONFIG USE_STORED_BCCH"
       */
      if (mm_data->reg.bcch_encode AND
          ((mm_data->reg.op.ts EQ TS_NO_AVAIL) OR 
           (mm_data->config_use_stored_bcch)))
      {
        /*
         * use SIM data only the first time.
         */
        mm_data->reg.bcch_encode = FALSE;

        // Patch HM 14.03.01 >>>
        // memcpy (&rr_activate_req->bcch_info, &mm_data->reg.bcch, 17);
        // rr_activate_req->bcch_info.v_bcch = TRUE;
        rr_activate_req->bcch_info.v_bcch = V_BCCH_PRES;
        memcpy (rr_activate_req->bcch_info.bcch, mm_data->reg.bcch, SIZE_BCCH);
        // Patch HM 14.03.01 <<<
        
        TRACE_EVENT ("with BCCH information");
      }
      else
        memset (&rr_activate_req->bcch_info, 0, 
                sizeof (T_bcch_info));

      /* restart HPLMN timer for 6 min if we change to a PLMN not the HPLMN */
      if (!reg_best_plmn_in_country(&rr_activate_req->plmn))
      {
        reg_check_hplmn_tim(1);
      }
      /* reg_stop_hplmn_tim (); */

      /* Not Power ON Scenario AND Not a Test SIM */
      if (mm_data->first_attach EQ FALSE)
      {
          /* set the hplmn flag to true only if MM is in substate 
          Limited service or No cell available*/
          if (((GET_STATE(STATE_MM)) EQ MM_IDLE_NO_CELL_AVAILABLE) OR
              ((GET_STATE(STATE_MM)) EQ MM_IDLE_LIMITED_SERVICE))
          {
              /* check_hplmn set to true */
              rr_activate_req->check_hplmn = CHECK_PLMN_PRES;
          }
      }
      break;

    default:
      TRACE_ERROR (UNEXPECTED_DEFAULT);
      break;
  }

  /* Tell RR whether we are interested in GPRS */
  if (mm_gsm_alone())
    rr_activate_req->gprs_indication = GPRS_NO;
  else 
    rr_activate_req->gprs_indication = GPRS_YES;

  if (mm_data->reg.op.sim_ins EQ SIM_NO_INSRT) 
  {
    /* 
     * Without valid SIM, some informations are not present.
     * Ensure SIM specific data will be deleted from RR_ACTIVATE_REQ 
     * if SIM data present, but not considered as valid by MM.
     */
    memset (&rr_activate_req->plmn, 0x0F, sizeof (T_plmn));
    rr_activate_req->plmn.v_plmn = V_PLMN_NOT_PRES;
    rr_activate_req->op.sim_ins = SIM_NO_INSRT;
    rr_activate_req->op.ts      = TS_NO_AVAIL;
    rr_activate_req->cksn       = CKSN_RES;
    memset (&rr_activate_req->kcv, 0x00, sizeof (T_kcv));
    rr_activate_req->accc       = 0x0000;
    mm_clear_mob_ident (&rr_activate_req->imsi_struct);
    mm_clear_mob_ident ((T_imsi_struct*) &rr_activate_req->tmsi_struct);
    memset (&rr_activate_req->bcch_info, 0, sizeof (T_bcch_info));
    rr_activate_req->cell_test  = CELL_TEST_DISABLE;
    /* rr_activate_req->gprs_indic = GPRS_NO; */
  }

  /* Add Equiavlent PLMN list to the activation data */
  memcpy (&rr_activate_req->eq_plmn_list, 
             &mm_data->reg.eqv_plmns.eqv_plmn_list,
              sizeof (EPLMNLIST_SIZE*UBYTES_PER_PLMN));
  /*added by jennifer for 134 sim card issue */
      if((rr_activate_req->plmn.mcc[0]==4)
     && (rr_activate_req->plmn.mcc[1]==6)
     &&(rr_activate_req->plmn.mcc[2]==0)
     &&(rr_activate_req->plmn.mnc[0]==0)
     &&(rr_activate_req->plmn.mnc[1]==2))
     {
     rr_activate_req->plmn.mnc[1]=0;
     }
  /*added by jennifer for 134 sim card issue*/

  TRACE_EVENT_P7 ("RR_ACT_REQ (%d) MCC=%X%X%X MNC=%X%X%X",
                  rr_activate_req->op.func,
                  rr_activate_req->plmn.mcc[0],
                  rr_activate_req->plmn.mcc[1],
                  rr_activate_req->plmn.mcc[2],
                  rr_activate_req->plmn.mnc[0],
                  rr_activate_req->plmn.mnc[1],
                  rr_activate_req->plmn.mnc[2]);

  PSENDX (RR, rr_activate_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_data_req             |
+--------------------------------------------------------------------+

  PURPOSE : This function sends the CM_SERVICE_REQUEST message to the 
            network if a CM entity is requesting an additional connection.

*/

GLOBAL void mm_rr_data_req (USHORT est_cause, UBYTE service, UBYTE ti)
{
  GET_INSTANCE_DATA;
  USHORT bit_size_message = 0;

  TRACE_FUNCTION ("mm_rr_data_req ()");

  mm_create_est_message (est_cause, 
                         service, 
                         ti,
                         &bit_size_message);
  for_data_req (bit_size_message);
  mm_data->wait_for_accept = TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_est_req              |
+--------------------------------------------------------------------+

  PURPOSE : This functions creates and sends the RR_ESTABLISH_REQ 
            primitive. A RR connection is requested.

*/

GLOBAL void mm_rr_est_req (USHORT    est_cause,
                           UBYTE     service,
                           UBYTE     ti)
{
  GET_INSTANCE_DATA;
  USHORT bit_size_message = 0;
  MCAST (loc_upd_req, U_LOC_UPD_REQ);

  TRACE_FUNCTION ("mm_rr_est_req ()");

  if (est_cause EQ ESTCS_SERV_REQ_BY_MM)
  {
    /* LOCATION UPDATING only, IMSI DETACH not handled here */
    
    mm_create_est_message (est_cause, service, ti,
                           &bit_size_message);

    if (mm_data->reg.update_stat EQ MS_NOT_UPDATED)
      loc_upd_req->ciph_key_num.key_seq = CKSN_RES;

    for_est_req (est_cause, bit_size_message);
  }
  else
  {
    /* All the CM stuff */

#ifdef  GPRS
    assert (GET_STATE (STATE_GPRS_CM_EST) EQ REG_GPRS_INACTIVE OR
            GET_STATE (STATE_GPRS_CM_EST) EQ CM_GPRS_EST_OK);
#endif

    mm_create_est_message (est_cause, service, ti,
                           &bit_size_message);
    
    for_est_req (est_cause, bit_size_message);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_start_loc_upd           |
+--------------------------------------------------------------------+

  PURPOSE : This function starts a location update. 
            The type of the location update will be given in 
            loc_upd_type and is one of NORMAL_LUP, PERIODIC_LUP or 
            IMSI_ATTACH_LUP.

*/
GLOBAL void mm_start_loc_upd (UBYTE loc_upd_type)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_start_loc_upd ()");

  TRACE_EVENT_P1 ("attempt #%x", mm_data->attempt_cnt + 1);

  /* Set the actual plmn to the PLMN selected by RR */
  // Added here by HM 13.11.00
  mm_data->reg.actual_plmn.v_plmn = V_PLMN_PRES;
  memcpy (mm_data->reg.actual_plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
  memcpy (mm_data->reg.actual_plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);

  mm_set_follow_on_request ();
  // mm_data->t3212_timeout = FALSE;
  mm_data->idle_entry  = RRCS_INT_NOT_PRESENT;
  mm_data->loc_upd_type.lut = loc_upd_type;
  mm_data->limited_cause = MMCS_INT_NOT_PRESENT;

  EM_RR_CONECTION_REQUESTED;

  /* 
   * Remember the idle substate MM may have to enter afterwards. 
   * 
   * This implementation of MM lacks the implementation of MM_IDLE_LUP_NEEDED
   * for the GSM only protocol stack.
   * So, we have to use MM_IDLE_NORMAL_SERVICE and MM_IDLE_ATTEMPT_TO_UPDATE 
   * not as foreseen by ETSI in some certain situations as replacement for 
   * the missing state MM_IDLE_LUP_NEEDED.
   */

#if 0  /* Bodybag 2004-01-15; remove latest 2004-07-15 */
  // The old code: 
  // Problem: Remembers MM_IDLE_ATTEMPT_TO_UPDATE even if IMSI ATTACH.
  // This is wrong, compare this with GSM 04.08 subclause 4.2.3.
  // Problem 2.) Didn't handle GPRS.
  switch (GET_STATE(STATE_MM))
  {
    case MM_IDLE_ATTEMPT_TO_UPDATE:
    case MM_WAIT_FOR_RR_ACTIVE:
      mm_data->idle_substate = MM_IDLE_ATTEMPT_TO_UPDATE;
      break;

    default:
      mm_data->idle_substate = MM_IDLE_NORMAL_SERVICE;
      break;
  }
#endif

  if (mm_data->reg.update_stat EQ MS_UPDATED)
    mm_data->idle_substate = MM_IDLE_NORMAL_SERVICE;
  else
    mm_data->idle_substate = MM_IDLE_ATTEMPT_TO_UPDATE;

  if (mm_data->rej_cause NEQ RRCS_RND_ACC_FAIL)
    mm_data->last_rej_cause = mm_data->rej_cause;

  mm_rr_est_req (ESTCS_SERV_REQ_BY_MM, NO_CM_SERVICE, 0);
  
  TIMERSTOP (T3211);
  TIMERSTOP (T3213);
  mm_data->t3213_restart = 0;
  
  SET_STATE (STATE_MM, MM_WAIT_FOR_RR_CONN_LUP);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_continue_running_update |
+--------------------------------------------------------------------+

  PURPOSE : This function continues a running location update.

*/
GLOBAL void mm_continue_running_update (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_continue_running_update()");

  switch (mm_data->loc_upd_type.lut)
  {
    case NORMAL_LUP:
      mm_normal_loc_upd ();
      break;
    case PERIODIC_LUP:
      mm_periodic_loc_upd ();
      break;
    case IMSI_ATTACH_LUP:
      mm_attach_loc_upd ();
      break;
    default: /* No updating procedure in progress */
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_start_net_req           |
+--------------------------------------------------------------------+

  PURPOSE : This function starts a network request.

*/

GLOBAL void mm_start_net_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_start_net_req ()");

  mm_data->reg.op.func = FUNC_NET_SRCH_BY_MMI;
  mm_rr_act_req ();

  /* 
   * 19.7 PLMN SEARCH 
   * The mobile station is searching for PLMNs, and the conditions for 
   * state 19.8 are not met. This state is ended when either a cell is 
   * selected (the new state is 19.1, 19.3 or 19.6), 
   * or when it is concluded that no cell is available for the moment 
   * (the new state is 19.5). [GSM 04.08 subclause 4.1.2.1.2]
   */
  
  /* 
   * 19.8 PLMN SEARCH, NORMAL SERVICE
   * Valid subscriber data are available, update status is U1, a cell is
   * selected which belongs to the LA where the subscriber is registered,
   * and the mobile station is searching for PLMNs. This state is ended
   * when either a cell is selected (the new state is 19.1, 19.3 or 19.6),
   * or when it is concluded that no cell is available for the moment 
   * (the new state is 19.5). [GSM 04.08 subclause 4.1.2.1.2]
   */
 
  /* 
   * MM will enter MM_PLMN_SEARCH_NORMAL_SERVICE in the current implementation 
   * also if update state is different from U1 e.g. if MM is in state 
   * MM_IDLE_ATTEMPT_TO_UPDATE. This makes sense, as the update state can be
   * queried to find this condition.
   */

  switch (GET_STATE (STATE_MM))
  {
    case MM_IDLE_NORMAL_SERVICE:        /* 19.1 */
    case MM_IDLE_ATTEMPT_TO_UPDATE:     /* 19.2 */
#ifdef GPRS
    case MM_IDLE_LUP_NEEDED:            /* 19.6 */
#endif /* GPRS */
      /* Remember the type of IDLE state MM had before network search */
      mm_data->idle_substate = GET_STATE (STATE_MM);
      SET_STATE (STATE_MM, MM_PLMN_SEARCH_NORMAL_SERVICE);
      break;
    
    case MM_IDLE_PLMN_SEARCH:           /* 19.7 */
      break; /* No state change */

    case MM_PLMN_SEARCH_NORMAL_SERVICE: /* 19.8 */
      break; /* No state change */
    
    case MM_IDLE_LIMITED_SERVICE:       /* 19.3 */
    case MM_IDLE_NO_IMSI:               /* 19.4 */
      /* Remember the type of IDLE state MM had before network search */
      mm_data->idle_substate = GET_STATE (STATE_MM);
      SET_STATE (STATE_MM, MM_IDLE_PLMN_SEARCH);
      break; 

    case MM_NULL:
    case MM_WAIT_FOR_RR_ACTIVE:         /* 18 */
    case MM_IDLE_NO_CELL_AVAILABLE:     /* 19.5 */
      mm_data->idle_substate = MM_IDLE_NO_CELL_AVAILABLE;
      SET_STATE (STATE_MM, MM_IDLE_PLMN_SEARCH);
      break;
    
#ifdef GPRS
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
      /* What is happening here? Re-think again about this ... */
      TRACE_EVENT ("Interesting protocol sequence");
      
      /* Back to last IDLE substate */
      SET_STATE (STATE_MM, mm_data->idle_substate);
      
      /* Repeat this in old IDLE state */
      mm_start_net_req ();
      return;
#endif /* GPRS */
    
    default:
      TRACE_EVENT ("No IDLE state at network search. Observe this"); // ...
      mm_data->idle_substate = MM_IDLE_NO_CELL_AVAILABLE;
      SET_STATE (STATE_MM, MM_IDLE_PLMN_SEARCH);
      break; 
  }    
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_start_t3212_bcch        |
+--------------------------------------------------------------------+

  PURPOSE : Start the periodic location updating timer T3212 with the
            broadcasted value. This is done in two situations: First, 
            after power on and no IMSI ATTACH / NORMAL UPDATE is needed
            and second at return to IDLE state after network contact.
            The case where no periodic update timer exists is 
            handled also, in this case T3212 is stopped.
            If T3212 is running it is left alone and not restarted, 
            so it is ensured that no periodic location updating is 
            missed if a dedicated connection to the network is released 
            in e.g. state MM_WAIT_FOR_NW_COMMAND, but the conditions in 
            GSM 04.08 subclause 4.4.2 were not met to stop the timer (e.g.
            no MM message received or emergeny call and the previous 
            IDLE state was MM_IDLE_LIMITED_SERVICE.

*/

GLOBAL void mm_start_t3212_bcch (void)
{
  GET_INSTANCE_DATA;
  T_TIME t3212_time;
  
  TRACE_FUNCTION ("mm_start_t3212_bcch()");

#ifdef GPRS 
  if ((mm_data->mm.mm_info.t3212 EQ 0) OR mm_data->gprs.combined_procedures)
#else
  if (mm_data->mm.mm_info.t3212 EQ 0)
#endif /* GPRS */
  {
    /* 
     * No periodic location updating timer
     */
    TIMERSTOP (T3212); 
    mm_data->t3212_timeout = FALSE;
  }
  else
  {
    /*
     * Start T3212 with broadcasted value only if not already running
     */
    if (!TIMERACTIVE (T3212))
    {
      TRACE_EVENT ("T3212 start BCCH");
      if (mm_data->t3212_cfg_counter NEQ 0)
        t3212_time = T_T3212_TIC_VALUE * mm_data->t3212_cfg_counter;
      else
        t3212_time = T_T3212_TIC_VALUE * 36 * mm_data->mm.mm_info.t3212;
      TIMERSTART (T3212, t3212_time);
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_change_t3212            |
+--------------------------------------------------------------------+

  PURPOSE : This function starts/restarts the periodic updating timer
            with a new value if a change of the broadcasted value 
            was detected, either by receiving RR_ACTIVATE_CNF, 
            RR_ACTIVATE_IND or RR_SYNC_IND.

*/

GLOBAL void mm_change_t3212 (void)
{
  GET_INSTANCE_DATA;
  T_TIME t3212_time;
 
  TRACE_FUNCTION ("mm_change_t3212 ()");

#ifdef GPRS 
  if ((mm_data->mm.mm_info.t3212 EQ 0) OR mm_data->gprs.combined_procedures)
#else
  if (mm_data->mm.mm_info.t3212 EQ 0)
#endif /* GPRS */
  {
    /*
     * No periodic location updating timer
     */
    TIMERSTOP (T3212);
    mm_data->t3212_timeout = FALSE;
  }
  else
  {
    if (TIMERACTIVE(T3212) OR mm_data->t3212_timeout)
    {
      if (!mm_data->t3212_timeout)
      {
        T_TIME remaining_time;
        /*
         * T3212 is running and there maybe changes.
         * Restart T3212 with remaining time modulo BCCH value.
         * Nothing will happen here if the BCCH time is 
         * greater or equal the remaining time (modulo operator), 
         * so we avoid TIMERSTOP / TIMERSTART in this case also.
         */
  #if defined (NEW_FRAME)
        TIMER_STATUS (mm_handle, T3212, &remaining_time);
  #else
        TIMER_STATUS (mm_handle, mm_data->t_handle[T3212], &remaining_time);
  #endif
        t3212_time = 
          remaining_time % (mm_data->mm.mm_info.t3212 * 36 * T_T3212_TIC_VALUE);
        if (t3212_time NEQ remaining_time)
        {
          TRACE_EVENT ("T3212 restart modulo");
          TIMERSTOP (T3212); // Neccessary ?
  #ifdef WIN32
          vsi_o_ttrace(VSI_CALLER TC_EVENT,
                       "Modulo time = %d", t3212_time);
  #endif
          TIMERSTART (T3212, t3212_time);
        }
      }
    }
    else
    {
      if (mm_data->first_attach) 
      {
        /* 
         * First activation after power on and no IMSI ATTACH, 
         * start T3212 with broadcasted value.
         */
        mm_start_t3212_bcch ();
      }
      else
      {
        /*
         * changes detected, T3212 is not running and it is not the first
         * activation. Restart T3212 with random value between 0 and BCCH.
         */
        TRACE_EVENT ("T3212 restart random");
        if (mm_data->t3212_cfg_counter NEQ 0)
        {
          t3212_time = mm_data->t3212_cfg_counter;
        }
        else
        {
          t3212_time = 
            T_T3212_TIC_VALUE * 
              mm_random ((USHORT)(mm_data->mm.mm_info.t3212 * 36));
        }
        TIMERSTART (T3212, t3212_time);
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_use_entry               |
+--------------------------------------------------------------------+

  PURPOSE : This functions marks all entries in the stored event table
            which are marked as "STORED" as "USE". Then these events 
            are processed by mm_read_entry(). mm_read_entry() will 
            re-build the respective primitive for each event which
            is marked as "USE" and sent it to MM, giving it a new 
            chance to be processed by MM. For timers, the respective
            handling function will be called.

*/

GLOBAL void mm_use_entry (void)
{
  GET_INSTANCE_DATA;
  SHORT i;
  
  TRACE_FUNCTION ("mm_use_entry ()");

  for (i = 0; i < MAX_STORE_ENTRIES; i++)
  {
    if (mm_data->store[i].use EQ STORED)
      mm_data->store[i].use = USE;
  }

  while (mm_data->store[0].use EQ USE)
    mm_read_entry ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_write_entry             |
+--------------------------------------------------------------------+

  PURPOSE : SDL SAVE
            This function saves a primitive which cannot be used
            in the current state, but may be used after the next 
            state transition. If content_type is given as EVENT_ENTRY
            storing is done for the data component, TI and establ. cause.
            If another primitive is to be stored, the pointer is set to 
            the T_PRIM_HEADER and stored.
*/

GLOBAL void mm_write_entry (UBYTE     comp,
                            UBYTE     ti,
                            USHORT    estcs,
                            U8        content_type,
                            void      *primitive,
                            U8        info)
{
  GET_INSTANCE_DATA;
  USHORT i;
  
  TRACE_FUNCTION ("mm_write_entry ()");

  for (i = 0; i < MAX_STORE_ENTRIES; i++)
  {
    if (mm_data->store[i].use EQ ENTRY_FREE)
    {
      TRACE_EVENT_P2 ("Write entry: comp=%d, ti=%d", comp, ti);
      
      mm_data->store[i].use                 = STORED;
      mm_data->store[i].content_type        = content_type;
      if (content_type EQ EVENT_ENTRY)
      {
        mm_data->store[i].content.event.ti    = ti;
        mm_data->store[i].content.event.estcs = estcs;
        mm_data->store[i].content.event.comp  = comp;
        mm_data->store[i].info  = info;
        if (comp < NUM_OF_CM_ENT)
        {
          CM_SET_STATE (comp, ti, CM_STORE);
        }
      }
      else
      {
        mm_data->store[i].content.primitive   = D2P(primitive);
      }
      return;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_check_lai               |
+--------------------------------------------------------------------+

  PURPOSE : This function checks whether two given location areas
            identifications (MCC, MNC and LAC) are identical. 
            Returns TRUE if LAIs are identical, otherwise FALSE.

*/

GLOBAL BOOL mm_check_lai (const T_loc_area_ident * lai1,
                          const T_loc_area_ident * lai2)
{
  TRACE_FUNCTION ("mm_check_lai ()");

  return ((memcmp (lai1->mcc, lai2->mcc, SIZE_MCC) EQ 0) AND
          (memcmp (lai1->mnc, lai2->mnc, SIZE_MNC) EQ 0) AND
          lai1->lac EQ lai2->lac);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_check_lai_from_RR       |
+--------------------------------------------------------------------+

  PURPOSE : This function compares the PLMN and location area in the 
            form delivered by RR and in and T_plmn structure 
            for equality. Returns TRUE if the PLMNs and location areas
            are equal, otherwise FALSE.

*/

GLOBAL BOOL mm_check_lai_from_RR (const T_loc_area_ident * lai1,
                                  const T_plmn           * plmn2,
                                  USHORT             lac2)
{
  TRACE_FUNCTION ("mm_check_lai_from_RR ()");

  return (memcmp (lai1->mcc, plmn2->mcc, SIZE_MCC) EQ 0 AND
          memcmp (lai1->mnc, plmn2->mnc, SIZE_MNC) EQ 0 AND
          lai1->lac EQ lac2);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_normal_upd_needed       |
+--------------------------------------------------------------------+

  PURPOSE : This function returns TRUE if a normal updating is needed.

*/

GLOBAL BOOL mm_normal_upd_needed (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_normal_upd_needed()");

  if (mm_data->reg.update_stat EQ MS_UPDATED AND
      mm_check_lai (&mm_data->reg.lai, &mm_data->mm.lai))
    return FALSE;
  return TRUE;
}


/* 
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_attach_upd_needed       |
+--------------------------------------------------------------------+

  PURPOSE : This function returns TRUE if an IMSI ATTACH is needed.

*/

GLOBAL BOOL mm_attach_upd_needed (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_attach_upd_needed()");

  if (mm_data->mm.mm_info.att EQ ATT_ALLOW AND mm_data->first_attach) 
    return TRUE;
  return FALSE;
}


/* 
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_periodic_upd_needed     |
+--------------------------------------------------------------------+

  PURPOSE : This function returns TRUE if a periodic location update 
            is immediately needed. This can happen if T3212 expired 
            in a no coverage situation.

*/

GLOBAL BOOL mm_periodic_upd_needed (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_periodic_upd_needed()");

  if (mm_data->t3212_timeout AND mm_data->mm.mm_info.t3212 NEQ 0)
    return TRUE;
  return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_end_of_detach           |
+--------------------------------------------------------------------+

  PURPOSE : This function performs all actions which are done 
            at the end of a detach operation. This function may 
            be called due reception of the primitives RR_RELEASE_IND or 
            RR_ABORT_IND. In case the end of the IMSI DETACH procedure 
            was caused by a timeout of T3220, the RR connection has been 
            aborted normally and this function is called after the 
            release has been acknowledged by RR_RELEASE_IND from RR.

*/          

GLOBAL void mm_end_of_detach (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_end_of_detach()");
  
  TIMERSTOP (T3220);
  
  switch (mm_data->nreg_cause)
  {
    case CS_SIM_REM:
      /* Send RR_ABORT_REQ with ABCS_SIM_REM to invalidate SIM in RR */
      mm_abort_connection (ABCS_SIM_REM);
      
      // This may be done instead RR_ABORT_REQ
      // mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL_NO_PAG);
      
      mm_clear_reg_data ();
        
      // Find original place where MM entered MM_IDLE_NO_IMSI state >>>
      if (mm_data->mm_idle_no_imsi_marker EQ 0)
        mm_data->mm_idle_no_imsi_marker = 5;
      // End of debugging patch <<<
      SET_STATE (STATE_MM, MM_IDLE_NO_IMSI);

      reg_end_of_deregistration (mm_data->nreg_cause, LIMITED_SERVICE);
      break;

    case CS_POW_OFF:
    case CS_SOFT_OFF:
      mm_power_off ();
      reg_end_of_deregistration (mm_data->nreg_cause, NO_SERVICE);
      break;

#ifdef GPRS 
    case CS_DISABLE:
      /* Remember MM may have to perform IMSI ATTACH if reactivated */
      mm_data->first_attach = TRUE;
        
      /* 
       * Find appropriate IDLE state after disabling of CS GSM.
       * If there was really an IMSI DETACH before, the new state 
       * will be MM_IDLE_LUP_NEEDED as the old state was 
       * MM_IDLE_LIMITED_SERVICE and now at least an IMSI ATTACH 
       * is needed again, but if not really a DETACH was performed, 
       * also another IDLE state may be entered.
       */
      if (mm_data->reg.op.sim_ins EQ SIM_NO_INSRT)
      {
        SET_STATE (STATE_MM, MM_IDLE_NO_IMSI);
      }
      else if (mm_data->idle_substate EQ MM_IDLE_LIMITED_SERVICE)
      {
        SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
      }
      else if (mm_normal_upd_needed () OR mm_attach_upd_needed())
      {
        SET_STATE (STATE_MM, MM_IDLE_LUP_NEEDED);
      }
      else
      {
        SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
      }

      /* Confirm the requested deregistration */
      mm_mmgmm_nreg_cnf (mm_data->nreg_cause);
      break;
#endif /* GPRS */    
  
    default: 
      TRACE_ERROR (UNEXPECTED_DEFAULT);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_get_service_state       |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to get the service state from 
            the MM main state and/or the service state. 
            The return value of this function is expected to be in the
            range of states 19.1..19.6 for GPRS and 19.1..19.5 for GSM.
            The function shall only be used if leaving idle state and
            entering some sort of dedicated state.

*/

GLOBAL UBYTE mm_get_service_state (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_get_service_state()");
  
  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:                       /* 0 */
      /*  
       * In MM_NULL state, the mobile is switched off, so there is no cell.
       */
      return MM_IDLE_NO_CELL_AVAILABLE;
    
    case MM_WAIT_FOR_RR_CONN_LUP:       /* 13 */
    case MM_WAIT_FOR_RR_ACTIVE:         /* 18 */
      /* 
       * MM has a cell which may be good for full service.
       * MM has also a valid SIM (IMSI).
       *
       * This implementation of MM lacks the implementation of 
       * state MM_IDLE_LUP_NEEDED for the GSM only protocol stack.
       * MM_IDLE_LUP_NEEDED is only used in this implementation if MM needs
       * an update with GPRS, but GPRS has not yet triggered the updating.
       * So we have to use MM_IDLE_NORMAL_SERVICE and MM_IDLE_ATTEMPT_TO_UPDATE
       * not as foreseen by ETSI in some certain situations as replacement for 
       * the missing state MM_IDLE_LUP_NEEDED.
       */
      if (mm_data->reg.update_stat EQ MS_UPDATED) 
        return MM_IDLE_NORMAL_SERVICE; 
      else
        return MM_IDLE_ATTEMPT_TO_UPDATE;

    case MM_IDLE_NORMAL_SERVICE:        /* 19.1 */
    case MM_IDLE_ATTEMPT_TO_UPDATE:     /* 19.2 */
    case MM_IDLE_LIMITED_SERVICE:       /* 19.3 */
    case MM_IDLE_NO_IMSI:               /* 19.4 */
    case MM_IDLE_NO_CELL_AVAILABLE:     /* 19.5 */
#ifdef GPRS
    case MM_IDLE_LUP_NEEDED:            /* 19.6, not used by GSM only stack */
#endif /* GPRS */
      /* 
       * In the cases of 19.1..19.5 (GSM only) or 19.1..19.6 (GPRS also)
       * the service state is simply the appropriate MM IDLE state.
       */
      return GET_STATE (STATE_MM);
            
    case MM_IDLE_PLMN_SEARCH:           /* 19.7 */
    case MM_PLMN_SEARCH_NORMAL_SERVICE: /* 19.8 */
      /* 
       * For the searching IDLE states, no respective service state exists.
       * This is due to the fact that the information that RR is searching
       * for a network can be obtained by reading the MM main state.
       * It is more interesting if CM wants to perform a call to know the
       * IDLE state before the mobile has started network search.
       */
      return mm_data->idle_substate;
    
    default:
      /* 
       * Some sort of dedicated state. This cannot/need not be used to 
       * determine the service state.
       */
      return mm_data->idle_substate;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_full_service_pplmn_scan         |
+----------------------------------------------------------------------------+

  PURPOSE : This states according to MM main state whether the PPLMN
            rescan procedure could be applicable.


*/

GLOBAL BOOL mm_full_service_pplmn_scan (void)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_MM))
  {
    /*
     * The 'normal' cases : idle updated states
     */
    case MM_IDLE_NORMAL_SERVICE:
    case MM_PLMN_SEARCH_NORMAL_SERVICE:
    /* new introduced
     * We also might be in full service but without a valid registration.
     */
    case MM_IDLE_ATTEMPT_TO_UPDATE:
    /*
     * RR connection active, which is either for a CM connection which imply
     * that we are updated (unless it is an emergency call!) or for a MM
     * procedure. If MM procedure fails consistently then we will lose the
     * registration data and the timer will be stopped anyway.
     */
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_REESTABLISH:
      
    /*
     * It is important not to stop the timer when in this state (check of 
     * conditions can be done during LUP procedure!) 
     */
    case MM_LUP_INITIATED:
      return TRUE;

    /*
     * If we currently have no or limited service then the standard PLMN 
     * reselection procedure will apply if automatic mode.
     */
    case MM_NULL:
    case MM_IDLE_LIMITED_SERVICE:
    case MM_IDLE_NO_CELL_AVAILABLE:
    case MM_IDLE_PLMN_SEARCH:
    case MM_WAIT_FOR_RR_ACTIVE:

    /*
     * We also might be in full service but without a valid registration. No
     * point to carry on as HPLMN timer would be started after succesfull GSM
     * attach or LUP procedure.
     */
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_LUP_REJECTED:
#if defined(GPRS)
    case MM_LOCATION_UPDATING_PENDING:
#endif

    /*
     * No IMSI, detached or in the process to do it. HPLMN rescan does not 
     * make sense so disregard it.
     */
    case MM_IMSI_DETACH_INIT:
#if defined(GPRS)
    case MM_IMSI_DETACH_PENDING:
#endif
    case MM_IDLE_NO_IMSI:
    case MM_WAIT_FOR_RR_CONN_DETACH:
      return FALSE;

    /* 
     * 3GPP 23.122 clause 4.4.3.4 may apply here and makes life easy:
     * "The investigation scan is restricted to automatic selection mode and 
     *  shall only be performed by an MS that is capable of both voice and 
     * packet data."
     */
#ifdef GPRS
    case MM_IDLE_LUP_NEEDED:
      return FALSE;
#endif /* #ifdef GPRS */

     default:
      TRACE_ERROR (UNEXPECTED_DEFAULT);
      return FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_read_ffs_init           |
+--------------------------------------------------------------------+

  PURPOSE : This function is the initial read of ffs by MM.
*/

const char eplmn_list_name[] = "/gsm/l3/eplmn";
/* added by jennifer to write simloci to FFS */
//int Writesimloci = 1;
//if(Writesimloci)
const char simloci_name[] = "/gsm/l3/simloci";
T_loc_info                loc_info_ffs;
/* added by jennifer to write simloci to FFS */

const char imsi_name[] = "/gsm/l3/imsi";
T_imsi_struct      imsi_in_ffs;


GLOBAL void mm_read_ffs_init(void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("mm_read_ffs_init()");

  /* If MM has to create the directories for FFS, there is no point in carry on with the */
  /* initialisation */
  if(mm_check_ffs_dirs())
    return;

  if(mm_read_ffs())
  {
    /*Successful read*/

    /*
     * Compare IMSIs...
     */
    if(!(reg_imsi_equal(&mm_data->reg.eqv_plmns.eqv_plmn_imsi, 
                        &mm_data->reg.imsi_struct)))
    {
      /*
       * Remove old EPLMN list
       */
      memset (&mm_data->reg.eqv_plmns.eqv_plmn_list, 0xFF, EPLMNLIST_SIZE*UBYTES_PER_PLMN);

      /*
       * Write the new IMSI
       */
      mm_data->reg.eqv_plmns.eqv_plmn_imsi = mm_data->reg.imsi_struct; /*Struct Copy*/
      mm_write_eplmn_to_ffs();
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_read_ffs                |
+--------------------------------------------------------------------+

  PURPOSE : This function reads the EPLMN from the ffs by MM.
*/

GLOBAL BOOL mm_read_ffs(void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_read_ffs()");

  return (mm_handle_ffs_read_result(ffs_file_read (eplmn_list_name, 
                              &mm_data->reg.eqv_plmns, 
                               sizeof(T_ffs_eplmn_imsi))));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_read_ffs                |
+--------------------------------------------------------------------+

  PURPOSE : This function resets the EPLMN data and writes the empty 
            data to the FFS
*/

GLOBAL void mm_reset_ffs(void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_reset_ffs()");

  memset (&mm_data->reg.eqv_plmns.eqv_plmn_list, 
           0xFF, 
           EPLMNLIST_SIZE*UBYTES_PER_PLMN);

  mm_write_eplmn_to_ffs();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_write_eplmn_to_ffs      |
+--------------------------------------------------------------------+

  PURPOSE : This function writes the current EPLMN list to the 
            FFS
*/

GLOBAL void mm_write_eplmn_to_ffs(void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_write_eplmn_to_ffs()");
 
  mm_check_ffs_dirs();
  mm_handle_ffs_write_result(ffs_file_write (eplmn_list_name, 
                                &mm_data->reg.eqv_plmns,
                                 sizeof(T_ffs_eplmn_imsi),
                                 FFS_O_CREATE | FFS_O_RDWR));
}
/*add by TISH 0418 to write imsi to FFS*/

GLOBAL void mm_write_imsi_to_ffs(void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_write_imsi_to_ffs()");
  mm_check_ffs_dirs();
  mm_handle_ffs_write_result(ffs_file_write (imsi_name,
  					&mm_data->reg.imsi_struct,
  					sizeof(T_imsi_struct),
            FFS_O_CREATE | FFS_O_RDWR));
}

GLOBAL BOOL mm_read_ffs_imsi(void)
{

  TRACE_FUNCTION ("mm_read_ffs_imsi");
  
  return (mm_handle_ffs_read_result(ffs_file_read (imsi_name, 
                              &imsi_in_ffs, 
                              sizeof(T_imsi_struct))));
}
/*add by TISH 0418 to write imsi to FFS*/


/* added by TISH 0418 to write simloci to FFS */

GLOBAL void mm_write_simloci_to_ffs(void)
{
  GET_INSTANCE_DATA;

  ULONG tmsi_binary;

  TRACE_FUNCTION ("mm_write_simloci_to_ffs()");
  
  tmsi_binary = mm_data->reg.tmsi;
  loc_info_ffs.c_loc = SIZE_LOC_INFO;
  loc_info_ffs.loc[0]  = (UBYTE)(tmsi_binary >> 24);
  loc_info_ffs.loc[1]  = (UBYTE)(tmsi_binary >> 16);
  loc_info_ffs.loc[2]  = (UBYTE)(tmsi_binary >> 8);
  loc_info_ffs.loc[3]  = (UBYTE)tmsi_binary;
  loc_info_ffs.loc[4]  = mm_data->reg.lai.mcc[1] << 4;
  loc_info_ffs.loc[4] += mm_data->reg.lai.mcc[0];
  loc_info_ffs.loc[5]  = mm_data->reg.lai.mnc[2] << 4;
  loc_info_ffs.loc[5] += mm_data->reg.lai.mcc[2];  
  loc_info_ffs.loc[6]  = mm_data->reg.lai.mnc[1] << 4;
  loc_info_ffs.loc[6] += mm_data->reg.lai.mnc[0];
  loc_info_ffs.loc[7]  = mm_data->reg.lai.lac >> 8;
  loc_info_ffs.loc[8]  = mm_data->reg.lai.lac & 0xff;
  loc_info_ffs.loc[9]  = 0;
  loc_info_ffs.loc[10] = mm_data->reg.update_stat;

  TRACE_EVENT_P7("writelocinfo:%d,%d,%d,%d,%d,%d,%d", loc_info_ffs.loc[4],
		 loc_info_ffs.loc[5], loc_info_ffs.loc[6], loc_info_ffs.loc[7],
		 loc_info_ffs.loc[8], loc_info_ffs.loc[9],
		 loc_info_ffs.loc[10]);
  mm_check_ffs_dirs();
  mm_handle_ffs_write_result(ffs_file_write (simloci_name, 
                                &loc_info_ffs,
                                 sizeof(T_loc_info),
                                 FFS_O_CREATE | FFS_O_RDWR));
  
}

GLOBAL BOOL mm_read_ffs_simloci(void)
{

  TRACE_FUNCTION ("mm_read_ffs_simloci");
  
  return (mm_handle_ffs_read_result(ffs_file_read (simloci_name, 
                              &loc_info_ffs, 
                                sizeof(T_loc_info))));
}
/* added by TISH 0418 to write simloci to FFS */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_check_ffs_dirs          |
+--------------------------------------------------------------------+

  PURPOSE : This routine creates the /gsm/l3 directory for the eplmn 
            file in the FFS. The return result indicates whether either 
            directories existed or not. There is no error handling.
*/

LOCAL BOOL mm_check_ffs_dirs( void )
{
  const char gsm_name[] = "/gsm";
  const char gsm_l3_name[] = "/gsm/l3";
  BOOL x, y;

  x = mm_create_ffs_dirs(gsm_name);
  y = mm_create_ffs_dirs(gsm_l3_name);
  return(x&&y);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_create_ffs_dirs         |
+--------------------------------------------------------------------+

  PURPOSE : This routine calls the ffs_mkdir() routine to create a 
            specified directory. It returns whether the directory 
            existed or not prior to creation.
*/
LOCAL BOOL mm_create_ffs_dirs(const char *dir_name)
{
  T_FFS_RET status;
  BOOL dirs_present = FALSE;

  status=ffs_mkdir(dir_name);
  switch(status)
  {
    case EFFS_EXISTS:
      dirs_present = TRUE;
      break;

    case EFFS_OK:
      break;

    default: /*Error*/
      TRACE_EVENT_P1("Create Dir error - %x",status);
      break;
  }
  return dirs_present;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_handle_ffs_read_result  |
+--------------------------------------------------------------------+

  PURPOSE : This function checks the status from FFS and returns
            TRUE if there is no error and FALSE if there is
*/
LOCAL BOOL mm_handle_ffs_read_result(T_FFS_SIZE status_read)
{
  if(status_read)
  {
    TRACE_EVENT ("FFS Read OK");
    return TRUE;
  }
  else
  {
    TRACE_EVENT_P1("EPLMN Read ERROR - %x", status_read);
    return FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_handle_ffs_write_result |
+--------------------------------------------------------------------+

  PURPOSE : This function checks the status from FFS and returns
            TRUE if there is no error and FALSE if there is
*/
LOCAL BOOL mm_handle_ffs_write_result(T_FFS_RET status_write)
{
  if(status_write >= EFFS_OK)
  {
    TRACE_EVENT("FFS Write OK");
    if(status_write)
      TRACE_EVENT_P1 ("Bytes written: %d",status_write);

    return(TRUE);
  }
  else
  {
    TRACE_EVENT_P1 ("FFS Write Error - Status: %x",status_write);
    return(FALSE);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_display_eplmn           |
+--------------------------------------------------------------------+

  PURPOSE : This function displays the current EPLMN list

*/

GLOBAL void mm_display_eplmn(void)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("EPLMN List");
  TRACE_EVENT_P3 ("%x, %x, %x", mm_data->reg.eqv_plmns.eqv_plmn_list[0],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[1],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[2]);
  TRACE_EVENT_P3 ("%x, %x, %x", mm_data->reg.eqv_plmns.eqv_plmn_list[3],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[4],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[5]);
  TRACE_EVENT_P3 ("%x, %x, %x", mm_data->reg.eqv_plmns.eqv_plmn_list[6],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[7],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[8]);
  TRACE_EVENT_P3 ("%x, %x, %x", mm_data->reg.eqv_plmns.eqv_plmn_list[9],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[10],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[11]);
  TRACE_EVENT_P3 ("%x, %x, %x", mm_data->reg.eqv_plmns.eqv_plmn_list[12],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[13],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[14]);
  TRACE_EVENT_P3 ("%x, %x, %x", mm_data->reg.eqv_plmns.eqv_plmn_list[15],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[16],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[17]);

  TRACE_EVENT_P3 ("%c, %c, %c", mm_data->reg.eqv_plmns.eqv_plmn_list[0],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[1],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[2]);
  TRACE_EVENT_P3 ("%c, %c, %c", mm_data->reg.eqv_plmns.eqv_plmn_list[3],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[4],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[5]);
  TRACE_EVENT_P3 ("%c, %c, %c", mm_data->reg.eqv_plmns.eqv_plmn_list[6],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[7],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[8]);
  TRACE_EVENT_P3 ("%c, %c, %c", mm_data->reg.eqv_plmns.eqv_plmn_list[9],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[10],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[11]);
  TRACE_EVENT_P3 ("%c, %c, %c", mm_data->reg.eqv_plmns.eqv_plmn_list[12],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[13],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[14]);
  TRACE_EVENT_P3 ("%c, %c, %c", mm_data->reg.eqv_plmns.eqv_plmn_list[15],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[16],
                                mm_data->reg.eqv_plmns.eqv_plmn_list[17]);

}

/* Implements Measure 29 and streamline encoding */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_send_status             |
+--------------------------------------------------------------------+

  PURPOSE : This function builts and sends status

*/
GLOBAL void mm_send_status (UBYTE  cause)
{
  TRACE_FUNCTION ("mm_send_status ()");
  
  {
    MSG_CAST (mm_status, T_B_MM_STATUS);
    mm_build_mm_status (cause, mm_status);
    for_data_req (BSIZE_B_MM_STATUS);
  }
}
#endif
