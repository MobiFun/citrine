/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  SIM_GPRS
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
|  Purpose :  This modul defines the GPRS Upgrade.
+----------------------------------------------------------------------------- 
*/ 

#ifndef SIM_GPRS_C
#define SIM_GPRS_C

#define ENTITY_SIM

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "cnf_sim.h"
#include "mon_sim.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "sim.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== TYPES ======================================================*/

/*==== CONSTANTS ==================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)     MODULE  : SIM_GPRS                     |
| STATE   : code              ROUTINE : gprs_check_pcm_data          |
+--------------------------------------------------------------------+

  PURPOSE : Checks the validation of GPRS data stored in PCM.

*/

GLOBAL BOOL gprs_check_pcm_data (T_imsi_field *sim_imsi)
{
  EF_IMSIGPRS imsi;
  UBYTE       version;


  return pcm_ReadFile((UBYTE *)EF_IMSIGPRS_ID, SIZE_EF_IMSIGPRS,
                      (UBYTE *)&imsi, &version) EQ PCM_OK
                       AND
                      (sim_imsi->c_field EQ imsi.len)
                       AND
                       !memcmp(imsi.IMSI, sim_imsi->field, sim_imsi->c_field);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)     MODULE  : SIM_GPRS                     |
| STATE   : code              ROUTINE : gprs_gmm_insert_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Collects the initial data for GMM.

*/

GLOBAL void gprs_gmm_insert_ind (T_SIM_MM_INSERT_IND * sim_mm_insert_ind)
{
  UBYTE  kc_n[MAX_KC_N];
  int    i;
  /*
   * Read Parameters for GPRS mobility management
   */
  PALLOC (sim_gmm_insert_ind, SIM_GMM_INSERT_IND);

  TRACE_FUNCTION ("gprs_sim_gmm_insert_ind()");

  /*
   * administrative data
   */
  sim_gmm_insert_ind->op_mode = sim_mm_insert_ind->ad[0];

  /*
   * IMSI
   */
  memcpy (&sim_gmm_insert_ind->imsi_field, &sim_mm_insert_ind->imsi_field,
          sizeof (T_imsi_field));
  /*
   * Location Information
   */
  memcpy (&sim_gmm_insert_ind->loc_info, &sim_mm_insert_ind->loc_info,
          sizeof (T_loc_info));
  /*
   * access control classes
   */
  memcpy (&sim_gmm_insert_ind->acc_ctrl, &sim_mm_insert_ind->acc_ctrl,
          sizeof (T_acc_ctrl));
   /*
    * phase
    */
  sim_gmm_insert_ind->phase = sim_data.sim_phase;

  if (SIM_IS_FLAG_SET (SERVICE_38_SUPPORT))
  {
    /*
     * SIM card supports GPRS
     *
     * Read GPRS Location Information
     */
    TRACE_EVENT ("SIM supports GPRS");
  
    if (FKT_Select (SIM_LOCGPRS, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
    {
      sim_gmm_insert_ind->gprs_loc_info.c_loc = MAX_LOCIGPRS;
      if (FKT_ReadBinary ((UBYTE *)&sim_gmm_insert_ind->gprs_loc_info.loc,
                          0, MAX_LOCIGPRS) NEQ SIM_NO_ERROR)
      {
        PFREE (sim_gmm_insert_ind);
        return;
      }
    }
    /*
     * Read GPRS KC
     */
    if (FKT_Select (SIM_KCGPRS, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
    {
      if (FKT_ReadBinary ((UBYTE *)kc_n, 0, MAX_KC_N) NEQ
          SIM_NO_ERROR)
      {
        PFREE (sim_gmm_insert_ind);
        return;
      }
      else
      {
        sim_gmm_insert_ind->kc_n.c_kc = MAX_KC_N;
        /*
         * Store KC in opposite order
         */
        for (i = 0; i < MAX_KC; i++)
          sim_gmm_insert_ind->kc_n.kc[(MAX_KC-1)-i] = kc_n[i];
        /*
         * Store cipher key sequence number
         */
        sim_gmm_insert_ind->kc_n.kc[MAX_KC] = kc_n[MAX_KC];
      }
    }
  }
  else
  {
    /*
     * Use PCM instead
     */
    TRACE_EVENT ("SIM does not support GPRS");

    /*
     * Only use ME data, when it is marked with IMSI
     * Note : No storage of the changed IMSI here!
     */
    if (gprs_check_pcm_data (&sim_mm_insert_ind->imsi_field))
    {
      UBYTE version;
      /*
       * then read the fields
       */
      pcm_ReadFile((UBYTE *) EF_LOCGPRS_ID,SIZE_EF_LOCGPRS,
                   (UBYTE *) &sim_gmm_insert_ind->gprs_loc_info.loc, &version);
      sim_gmm_insert_ind->gprs_loc_info.c_loc = MAX_LOCIGPRS;
      pcm_ReadFile((UBYTE *) EF_KCGPRS_ID,SIZE_EF_KCGPRS,
                   (UBYTE *) &sim_gmm_insert_ind->kc_n, &version);
    }
    else
    {
      /*
       * reading of IMSI failed, set values to defaults
       */
      memset (&sim_gmm_insert_ind->gprs_loc_info.loc, 0xFF, MAX_LOCIGPRS);
      sim_gmm_insert_ind->gprs_loc_info.loc[11] = 0xFE;
      sim_gmm_insert_ind->gprs_loc_info.c_loc = 0;
      memset (&sim_gmm_insert_ind->kc_n, 0xFF, 9);
    }
  }

  /*
   * send information to GPRS mobility management
   */
  PSENDX (GMM, sim_gmm_insert_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_GPRS                   |
| STATE   : code                ROUTINE : gprs_gmm_update_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process of the primitive SIM_GMM_UPDATE_REQ.

*/

GLOBAL void gprs_sim_gmm_update_req (T_SIM_GMM_UPDATE_REQ * sim_gmm_update_req)
{
  int    i;
  T_kc_n kc_n;

  TRACE_FUNCTION ("gprs_sim_gmm_update_req()");

  /*
   * prepare kc and cksn for storing
   */
  kc_n.c_kc = MAX_KC_N;
  kc_n.kc[MAX_KC] = sim_gmm_update_req->cksn;
  for (i = 0; i < MAX_KC; i++)
    kc_n.kc[(MAX_KC-1)-i] = sim_gmm_update_req->kc[i];

  /*
   * SIM with GPRS service activated?
   */
  if (SIM_IS_FLAG_SET (SERVICE_38_SUPPORT))
  {
  /*
   * check location information
   */
    if (sim_gmm_update_req->gprs_loc_info.c_loc > 0)
    {
      if (FKT_Select (SIM_LOCGPRS, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
        FKT_UpdateBinary (sim_gmm_update_req->gprs_loc_info.loc,
                          MAX_LOCIGPRS, 0);
    }
  /*
   * store kc and cksn
   */
    if (FKT_Select (SIM_KCGPRS, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
    {
      FKT_UpdateBinary (kc_n.kc, kc_n.c_kc, 0);
    }
  }
  else
  {
    /*
     * SIM with no GPRS service: store in ME memory
     */
    T_imsi_field sim_imsi;
  
    if (FKT_Select (SIM_IMSI, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR AND
        FKT_ReadBinary ((UBYTE *)&sim_imsi, 0, MAX_IMSI)
         EQ SIM_NO_ERROR)
    {
      /*
       * Compare IMSI on SIM with IMSI in ME memory
       */
      if (!gprs_check_pcm_data (&sim_imsi))
      {
        /*
         * Check GPRS attach status of current IMSI (from SIM)
         */
        if (sim_gmm_update_req->att_status)
          /*
           * Update IMSI in ME memory, when attached
           */
          pcm_WriteFile((UBYTE *)EF_IMSIGPRS_ID, SIZE_EF_IMSIGPRS,
                        (UBYTE *)&sim_imsi);
        else
        {
          /*
           * do not update GPRS data
           */
          PFREE (sim_gmm_update_req);
          return;
        }
      }
      /*
       * Update GPRS data in ME memory
       */
      if (sim_gmm_update_req->gprs_loc_info.c_loc > 0)
      {
        pcm_WriteFile((UBYTE *)EF_LOCGPRS_ID,SIZE_EF_LOCGPRS,
                      (UBYTE *)&sim_gmm_update_req->gprs_loc_info.loc);
      }
      pcm_WriteFile((UBYTE *)EF_KCGPRS_ID,SIZE_EF_KCGPRS,
                    (UBYTE *)kc_n.kc);
    }
  }
  PFREE (sim_gmm_update_req);
}

#endif
