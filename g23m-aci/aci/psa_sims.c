/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SIMS
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
|  Purpose :  This module defines the signalling functions of the
|             protocol stack adapter for the subscriber identity
|             module.
+-----------------------------------------------------------------------------
*/

#ifndef PSA_SIMS_C
#define PSA_SIMS_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "psa.h"
#include "psa_sim.h"

#include "aci_mem.h"
#ifdef SIM_TOOLKIT
#include "psa_cc.h"
#include "psa_sat.h"
#include "ati_src_sat.h"
#include "psa_tcpip.h"
#endif/* SIM_TOOLKIT */

#ifdef DTI
#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#endif

#if defined GPRS AND defined (DTI)
#include "gprs.h"
#include "gaci.h"
#include "gaci_cmh.h"
#include "gaci_srcc.h"
#include "psa_sm.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif /* FAX_AND_DATA */

#endif /* GPRS */

#include "cmh.h"

#if defined (GPRS) AND defined (DTI)
#include "cmh_sm.h"
#endif /* GPRS */

#include "cmh_sim.h"
#include "cmh_sat.h"

#include "psa_util.h"

#ifndef _SIMULATION_

/* temporary solution to get ffs.h included without GPRS to be set ! */
#ifdef GPRS
#include "../../services/ffs/ffs.h"
#else
#include "../../services/ffs/ffs.h"
#undef GPRS
#endif /* GPRS */
#include "ffs_coat.h"

#endif /* !_SIMULATION_ */


/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/
#ifdef TI_PS_FF_AT_P_CMD_CUST
LOCAL BOOL Cust_Mode_Set = FALSE;

/*==== FUNCTIONS ==================================================*/

GLOBAL BOOL psaSIM_hasCustModeBeenSet(void)
{
       return Cust_Mode_Set;
}
#endif /* TI_PS_FF_AT_P_CMD_CUST */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_AccessSIMData         |
+-------------------------------------------------------------------+

  PURPOSE : access SIM data

*/

GLOBAL SHORT psaSIM_AccessSIMData ( void )
{
  SHORT aId;            /* holds access id */

  TRACE_FUNCTION ("psaSIM_AccessSIMData()");

/*
 *-------------------------------------------------------------------
 * determine access type
 *-------------------------------------------------------------------
 */
  aId = simShrdPrm.aId;

  switch( simShrdPrm.atb[aId].accType )
  {
    /*
     *---------------------------------------------------------------
     * read datafield
     *---------------------------------------------------------------
     */
    case( ACT_RD_DAT ):
      {
        PALLOC (sim_read_req, SIM_READ_REQ);

        /* fill in primitive parameter: read data request */
        sim_read_req -> source     = SRC_MMI;
        sim_read_req -> req_id      = (UBYTE)aId;
        sim_read_req -> v_path_info = simShrdPrm.atb[aId].v_path_info;
        if(sim_read_req -> v_path_info)
        {
          sim_read_req -> path_info   = simShrdPrm.atb[aId].path_info;
        }
        sim_read_req -> datafield  = simShrdPrm.atb[aId].reqDataFld;
        sim_read_req -> offset     = simShrdPrm.atb[aId].dataOff;
        sim_read_req -> length     = simShrdPrm.atb[aId].dataLen;
        sim_read_req -> max_length = simShrdPrm.atb[aId].recMax;
        PSENDX (SIM, sim_read_req);
      }
      break;

    /*
     *---------------------------------------------------------------
     * write datafield
     *---------------------------------------------------------------
     */
    case( ACT_WR_DAT ):
      {
        PALLOC (sim_update_req, SIM_UPDATE_REQ);

        /* fill in primitive parameter: update data request */
        sim_update_req -> source    = SRC_MMI;
        sim_update_req -> req_id      = (UBYTE)aId;
        sim_update_req -> v_path_info = simShrdPrm.atb[aId].v_path_info; 
        if(sim_update_req -> v_path_info)
        {
          sim_update_req -> path_info   = simShrdPrm.atb[aId].path_info;   
        }
        sim_update_req -> datafield = simShrdPrm.atb[aId].reqDataFld;
        sim_update_req -> offset    = simShrdPrm.atb[aId].dataOff;
        sim_update_req -> length    = simShrdPrm.atb[aId].dataLen;
        memcpy (sim_update_req -> trans_data, simShrdPrm.atb[aId].exchData,
                simShrdPrm.atb[aId].dataLen);

        PSENDX (SIM, sim_update_req);
      }
      break;

    /*
     *---------------------------------------------------------------
     * read record
     *---------------------------------------------------------------
     */
    case( ACT_RD_REC ):
      {
        PALLOC (sim_read_record_req, SIM_READ_RECORD_REQ);

        /* fill in primitive parameter: read absolute record request */
        sim_read_record_req -> source    = SRC_MMI;
        sim_read_record_req -> req_id    = (UBYTE)aId;
        sim_read_record_req -> v_path_info = simShrdPrm.atb[aId].v_path_info;
        if(sim_read_record_req -> v_path_info)
        {
          sim_read_record_req -> path_info   = simShrdPrm.atb[aId].path_info;
        }
        sim_read_record_req -> datafield = simShrdPrm.atb[aId].reqDataFld;
        sim_read_record_req -> record    = simShrdPrm.atb[aId].recNr;
        if(simShrdPrm.atb[aId].check_dataLen)
        {
          sim_read_record_req -> length    = NOT_PRESENT_8BIT;
        }
        else
        {
          sim_read_record_req -> length    = simShrdPrm.atb[aId].dataLen;
        }
        PSENDX (SIM, sim_read_record_req);
      }
      break;

    /*
     *---------------------------------------------------------------
     * write record
     *---------------------------------------------------------------
     */
    case( ACT_WR_REC ):
      {
        PALLOC (sim_update_record_req, SIM_UPDATE_RECORD_REQ);

        /* fill in primitive parameter: update absolute record request */
        sim_update_record_req -> source    = SRC_MMI;
        sim_update_record_req -> req_id    = (U8)aId;
        sim_update_record_req -> v_path_info = simShrdPrm.atb[aId].v_path_info;
        if(sim_update_record_req -> v_path_info)
        {
          sim_update_record_req -> path_info   = simShrdPrm.atb[aId].path_info;
        }
        sim_update_record_req -> datafield = simShrdPrm.atb[aId].reqDataFld;
        sim_update_record_req -> record    = simShrdPrm.atb[aId].recNr;
        sim_update_record_req -> length    = simShrdPrm.atb[aId].dataLen;
        memcpy (sim_update_record_req -> linear_data,
                simShrdPrm.atb[aId].exchData, sim_update_record_req->length);

        PSENDX (SIM, sim_update_record_req);
      }
      break;

    /*
     *---------------------------------------------------------------
     * increment datafield
     *---------------------------------------------------------------
     */
    case( ACT_INC_DAT ):
      {
        PALLOC (sim_increment_req, SIM_INCREMENT_REQ);

        /* fill in primitive parameter: increment data request */
        sim_increment_req -> source    = SRC_MMI;
        sim_increment_req -> req_id    = (U8)aId;
        sim_increment_req -> v_path_info = simShrdPrm.atb[aId].v_path_info;
        if(sim_increment_req -> v_path_info)
        {
          sim_increment_req -> path_info   = simShrdPrm.atb[aId].path_info;
        }
        sim_increment_req -> datafield = simShrdPrm.atb[aId].reqDataFld;
        sim_increment_req -> length    = simShrdPrm.atb[aId].dataLen;
        memcpy (sim_increment_req -> linear_data, simShrdPrm.atb[aId].exchData,
                sim_increment_req->length);

        PSENDX (SIM, sim_increment_req);
      }
      break;

    /*
     *---------------------------------------------------------------
     * illegal access type
     *---------------------------------------------------------------
     */
    default:

      TRACE_EVENT("UNEXP ACCESS TYPE IN ATB");
      return( -1 );
  }
/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_VerifyPIN             |
+-------------------------------------------------------------------+

  PURPOSE : verify a PIN

*/

GLOBAL SHORT psaSIM_VerifyPIN ( void )
{
  T_SIM_SET_PRM * pPrmSet;       /* points to used parameter set */

  TRACE_FUNCTION ("psaSIM_VerifyPIN()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */
  if(!psa_IsVldOwnId(simShrdPrm.owner))

    return( -1 );

  pPrmSet = &simShrdPrm.setPrm[simShrdPrm.owner];

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_verify_pin_req, SIM_VERIFY_PIN_REQ);

    /* fill in primitive parameter: verify PIN request */
    sim_verify_pin_req -> source = SRC_MMI;
    sim_verify_pin_req -> pin_id = pPrmSet -> PINType;
    memcpy( sim_verify_pin_req -> pin, pPrmSet -> curPIN, PIN_LEN );

    PSENDX (SIM, sim_verify_pin_req);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_ChangePIN             |
+-------------------------------------------------------------------+

  PURPOSE : change a PIN

*/

GLOBAL SHORT psaSIM_ChangePIN ( void )
{
  T_SIM_SET_PRM * pPrmSet;       /* points to used parameter set */

  TRACE_FUNCTION ("psaSIM_ChangePIN()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */
  if(!psa_IsVldOwnId(simShrdPrm.owner))

    return( -1 );

  pPrmSet = &simShrdPrm.setPrm[simShrdPrm.owner];

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_change_pin_req, SIM_CHANGE_PIN_REQ);

    /* fill in primitive parameter: change PIN request */
    sim_change_pin_req -> source = SRC_MMI;
    sim_change_pin_req -> pin_id = pPrmSet -> PINType;
    memcpy( sim_change_pin_req -> old_pin, pPrmSet -> curPIN, PIN_LEN );
    memcpy( sim_change_pin_req -> new_pin, pPrmSet -> newPIN, PIN_LEN );

    PSENDX (SIM, sim_change_pin_req);
  }

  return 0;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_DisablePIN            |
+-------------------------------------------------------------------+

  PURPOSE : disable a PIN

*/

GLOBAL SHORT psaSIM_DisablePIN ( void )
{
  T_SIM_SET_PRM * pPrmSet;       /* points to used parameter set */

  TRACE_FUNCTION ("psaSIM_DisablePIN()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */
  if(!psa_IsVldOwnId(simShrdPrm.owner))

    return( -1 );

  pPrmSet = &simShrdPrm.setPrm[simShrdPrm.owner];

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_disable_pin_req, SIM_DISABLE_PIN_REQ);

    /* fill in primitive parameter: disable PIN request */
    sim_disable_pin_req -> source = SRC_MMI;
    memcpy( sim_disable_pin_req -> pin, pPrmSet -> curPIN, PIN_LEN );

    PSENDX (SIM, sim_disable_pin_req);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_EnablePIN             |
+-------------------------------------------------------------------+

  PURPOSE : enable a PIN

*/

GLOBAL SHORT psaSIM_EnablePIN ( void )
{
  T_SIM_SET_PRM * pPrmSet;       /* points to used parameter set */

  TRACE_FUNCTION ("psaSIM_EnablePIN()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */
  if(!psa_IsVldOwnId(simShrdPrm.owner))

    return( -1 );

  pPrmSet = &simShrdPrm.setPrm[simShrdPrm.owner];

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_enable_pin_req, SIM_ENABLE_PIN_REQ);

    /* fill in primitive parameter: enable PIN request */
    sim_enable_pin_req -> source = SRC_MMI;
    memcpy( sim_enable_pin_req -> pin, pPrmSet -> curPIN, PIN_LEN );

    PSENDX (SIM, sim_enable_pin_req);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_UnblockCard           |
+-------------------------------------------------------------------+

  PURPOSE : unblock the card

*/

GLOBAL SHORT psaSIM_UnblockCard ( void )
{
  T_SIM_SET_PRM * pPrmSet;       /* points to used parameter set */

  TRACE_FUNCTION ("psaSIM_UnblockCard()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */
  if(!psa_IsVldOwnId(simShrdPrm.owner))

    return( -1 );

  pPrmSet = &simShrdPrm.setPrm[simShrdPrm.owner];

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_unblock_req, SIM_UNBLOCK_REQ);

    /* fill in primitive parameter: unblock card request */
    sim_unblock_req -> source = SRC_MMI;
    sim_unblock_req -> pin_id = pPrmSet -> PINType;
    memcpy( sim_unblock_req -> pin, pPrmSet -> curPIN, PIN_LEN );
    memcpy( sim_unblock_req -> unblock_key, pPrmSet -> unblkKey, PUK_LEN );

    PSENDX (SIM, sim_unblock_req);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_SyncSIM               |
+-------------------------------------------------------------------+

  PURPOSE : synchronize SIM data

*/

GLOBAL void psaSIM_SyncSIM ( void )
{
  TRACE_FUNCTION ("psaSIM_SyncSIM()");

  /* create and send primitive */
  {
    PALLOC (sim_sync_req, SIM_SYNC_REQ);

    /* fill in primitive parameter: synchronize request */

    sim_sync_req -> synccs = simShrdPrm.synCs;

    PSENDX (SIM, sim_sync_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_ActivateSIM           |
+-------------------------------------------------------------------+

  PURPOSE : synchronize SIM data

*/

GLOBAL SHORT psaSIM_ActivateSIM ( void )
{
  T_SIM_SET_PRM * pPrmSet;       /* points to used parameter set */

  TRACE_FUNCTION ("psaSIM_ActivateSIM()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */
  if(!psa_IsVldOwnId(simShrdPrm.owner))
  {
    TRACE_EVENT("not a valid owner !!");
    return( -1 );
  }

  pPrmSet = &simShrdPrm.setPrm[simShrdPrm.owner];

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_activate_req, SIM_ACTIVATE_REQ);

    /* fill in primitive parameter: activate request */
    sim_activate_req->proc = pPrmSet -> actProc;

    /* sim toolkit profile */
    memcpy( sim_activate_req->stk_pro_file, pPrmSet -> STKprof,
            MAX_STK_PRF );
    sim_activate_req->mmi_pro_file = 0xE0; /*enable all*/

#ifdef TI_PS_FF_AT_P_CMD_CUST
    if (pPrmSet -> actProc EQ SIM_INITIALISATION)
    {
        if (psaSIM_hasCustModeBeenSet() == FALSE)
        {
            /*
            ** Customisation Mode is to be global, applying to ALL ACI channels
            */
            simShrdPrm.overall_cust_mode = pPrmSet->cust_mode;
            Cust_Mode_Set = TRUE;
        }

        sim_activate_req->cust_mode = simShrdPrm.overall_cust_mode;
    }
    else
    {
        /*
        ** If this isn't SIM_INITIALISATION then the cust_mode will have no relevance, but it should
        ** still not be left uninitialised.
        */
        sim_activate_req->cust_mode = (psaSIM_hasCustModeBeenSet() == FALSE) ? 
                                                         (UBYTE)CUST_NORMAL_BEHAVIOUR: simShrdPrm.overall_cust_mode;
    }
#else
    sim_activate_req->cust_mode = (UBYTE)CUST_NORMAL_BEHAVIOUR;
#endif /* TI_PS_FF_AT_P_CMD_CUST */

    if(sim_activate_req->proc EQ SIM_INITIALISATION)
    {
      sim_activate_req->v_trmst_file = 0x01;
      TRACE_EVENT("Read Terminal Support table from FFS");
#ifndef _SIMULATION_
      if(FFS_fread("/gsm/osf/ftrsprt", &sim_activate_req->trmst_file, MAX_TRMST) NEQ MAX_TRMST)
#endif /* _SIMULATION_ */
      {
        memset(&sim_activate_req->trmst_file, 0, MAX_TRMST);
      }
    }
    
    PSENDX (SIM, sim_activate_req);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_TrnsSIMAccess         |
+-------------------------------------------------------------------+

  PURPOSE : transparent access to SIM data

*/

GLOBAL SHORT psaSIM_TrnsSIMAccess ( T_SIM_TRNS_ACC_PRM * prm )
{

  TRACE_FUNCTION ("psaSIM_TrnsSIMAccess()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_access_req, SIM_ACCESS_REQ);

    /* fill in primitive parameter: SIM access request */
    sim_access_req -> source      = SRC_MMI;
    sim_access_req -> sim_command = prm -> cmd;

    if (prm->cmd NEQ SIM_TRANSP_CMD)
    {
      sim_access_req -> datafield   = prm -> reqDataFld;
      sim_access_req -> p1          = prm -> p1;
      sim_access_req -> p2          = prm -> p2;
      sim_access_req -> p3          = prm -> p3;
    }

    sim_access_req->c_trans_data = prm -> dataLen;
    memcpy( sim_access_req -> trans_data, prm -> transData,
            prm -> dataLen );

    PSENDX (SIM, sim_access_req);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_Bpi_Req               |
+-------------------------------------------------------------------+

  PURPOSE :  resume/suspend/connect/disconnect SAT BIP connection. 
*/
#if defined (FF_SAT_E) AND defined (DTI)
GLOBAL void psaSIM_Bip_Req ( void )
{
  T_SAT_CSD_PRM * csd_prm_ptr;

  TRACE_EVENT("psaSIM_Bip_Req()");

  if (simShrdPrm.sim_dti_chPrm EQ NULL)
  {
    TRACE_ERROR ("ERROR: simShrdPrm.sim_dti_chPrm uninitalized, aborting!");
  }
  else
  {
    PALLOC(sim_bip_req,SIM_BIP_REQ);
    memset(sim_bip_req,0,sizeof(sim_bip_req));

    /* copy connection qualifier */
    sim_bip_req->bip_conn = simShrdPrm.sim_dti_chPrm->sat_chn_prm.bipConn;
    /* copy channel id */
    sim_bip_req->bip_ch_id = simShrdPrm.sim_dti_chPrm->sat_chn_prm.chnId;
    /* copy general result */
    sim_bip_req->general_result = simShrdPrm.sim_dti_chPrm->sat_chn_prm.genRes;
    /* copy additional result info */
    sim_bip_req->add_info_result = simShrdPrm.sim_dti_chPrm->sat_chn_prm.addRes;

   /* copy release timer to primitive, if CSD and if duration defined */
   /* SAT_E_PATCH: Should be ported to SIM_BIP_CONFIG_REQ? */
    if(satShrdPrm.opchType EQ B_CSD)
    {
      csd_prm_ptr = (T_SAT_CSD_PRM *)(satShrdPrm.opchPrm);
      if(csd_prm_ptr NEQ NULL)
      {
        if(csd_prm_ptr->v_dur2)
        {
          switch(csd_prm_ptr->dur2.time_unit)
          {
            case TU_100_MSEC:
              sim_bip_req->release_time = (U32)csd_prm_ptr->dur2.time_ivl;
              break;
            case TU_MIN:
              sim_bip_req->release_time = (U32)(csd_prm_ptr->dur2.time_ivl * 600);
              break;
            case TU_SEC:
              sim_bip_req->release_time = (U32)(csd_prm_ptr->dur2.time_ivl * 10);
              break;
          } /* end switch */
        } /* end if */
      } /* end if csd_prm_ptr NEQ NULL */ 
    } /* end if */    

    /* SEND SIM_BIP_REQ */
    PSENDX(SIM,sim_bip_req);      
  } /* end else*/

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_Bip_Config_Req        |
+-------------------------------------------------------------------+

  PURPOSE : sends the BIP channel parameters to SIM. The type of 
            parameters depends on the transport unit.   
*/

GLOBAL void psaSIM_Bip_Config_Req ( )
{
  T_SAT_CSD_PRM * csd_prm_ptr; /* holds pointer to CSD open channel prms */ 
  T_SAT_GPRS_PRM * gprs_prm_ptr; /* holds pointer to GPRS open channel prms */ 

  /* allocate SIM_BIP_CONFIG_REQ primitive */
  PALLOC(sim_bip_config_req,SIM_BIP_CONFIG_REQ);

  TRACE_EVENT("psaSIM_Bip_Config_Req()");

  /* reset values of primitive */
  memset(sim_bip_config_req,0,sizeof(sim_bip_config_req));
  /* check kind of bearer */ 
  if(satShrdPrm.opchType EQ B_CSD)
  {
    /* BEARER: CSD */

    /* get connection type */ 
    switch(simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiUnit)
    {
      case DTI_ENTITY_UDP:
        sim_bip_config_req->con_type = SIM_CON_TYPE_UDP;
        break;
#ifdef UART
      case DTI_ENTITY_TRA:
        sim_bip_config_req->con_type = SIM_CON_TYPE_SERIAL;
        break;
      case DTI_ENTITY_L2R:
        sim_bip_config_req->con_type = SIM_CON_TYPE_SERIAL;
        break;
#endif
    }

    /* get addresses */    
    /* local_ip,destination_ip, destination port are only valid for UDP */
    if(simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiUnit EQ DTI_ENTITY_UDP)
    {
      csd_prm_ptr = (T_SAT_CSD_PRM *)(satShrdPrm.opchPrm);
      if(csd_prm_ptr->v_other_addr)
      {
        if(csd_prm_ptr->other_addr.oth_addr_type EQ IPv4)
        {
          if(csd_prm_ptr->other_addr.v_ipv4_addr)/* local IP address */
          {
            memcpy(&sim_bip_config_req->local_ip,
                   &csd_prm_ptr->other_addr.ipv4_addr,
                   sizeof(csd_prm_ptr->other_addr.ipv4_addr));
          }  
          else /* not valid */
          {
            sim_bip_config_req->local_ip = SIM_IP_LOCAL_DYNAMIC;
          }
        }
        else /* not supported IPv6*/
        {
        sim_bip_config_req->local_ip = SIM_IP_LOCAL_DYNAMIC;
        }
      }
      else
      {
        sim_bip_config_req->local_ip = SIM_IP_LOCAL_DYNAMIC;
      }

      /* data destination address */
      if(csd_prm_ptr->v_dda)
      {
        if(csd_prm_ptr->dda.oth_addr_type EQ IPv4)
        {
          if(csd_prm_ptr->dda.v_ipv4_addr)/* local IP address */
          {
            memcpy(&sim_bip_config_req->destination_ip,
                   &csd_prm_ptr->dda.ipv4_addr,
                   sizeof(csd_prm_ptr->dda.ipv4_addr));
          }
        }

        /* SIM/ME interface transport level:destination port number for UDP */
        if(csd_prm_ptr->v_itl)
        {
          sim_bip_config_req->destination_port = csd_prm_ptr->itl.port_number;
        }
      }
    }        
  }
  else if (satShrdPrm.opchType EQ B_GPRS)
  {
    /* BEARER: GPRS */
#ifdef GPRS
    /* get connecton type */
    switch(simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiUnit)
    {
      case DTI_ENTITY_UDP:
        sim_bip_config_req->con_type = SIM_CON_TYPE_UDP;
        break;
      case DTI_ENTITY_SNDCP:
        sim_bip_config_req->con_type = SIM_CON_TYPE_IP;
        break;
    }

    /* get port and addresses */
    gprs_prm_ptr = (T_SAT_GPRS_PRM *)(satShrdPrm.opchPrm);
    if(gprs_prm_ptr NEQ NULL)
    {
      /* local_ip,destination_ip, destination port are only valid for UDP */
      if(simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiUnit EQ DTI_ENTITY_UDP)
      {
        /* get local ip address */ 
        if(gprs_prm_ptr->v_other_addr)
        {
          if(gprs_prm_ptr->other_addr.oth_addr_type EQ IPv4)
          {
            if(gprs_prm_ptr->other_addr.v_ipv4_addr)/* local IP address */
            {
              memcpy(&sim_bip_config_req->local_ip,
                     &gprs_prm_ptr->other_addr.ipv4_addr,
                     sizeof(gprs_prm_ptr->other_addr.ipv4_addr));
            }
            else /* not valid */
            {
              sim_bip_config_req->local_ip = SIM_IP_LOCAL_DYNAMIC;
            }
          }
          else /* not supported IPv6*/
          {
            sim_bip_config_req->local_ip = SIM_IP_LOCAL_DYNAMIC;
          }
        }
        else /* not valid flag for other address */
        {
          /*
           * local IP4 address has been provided by Session Management; it was not provided by the OPEN CHANNEL command
           */
          if(tcpipShrdPrm.ipaddr)
          {
            sim_bip_config_req->local_ip = psaTCPIP_bytes2ipv4addr(tcpipShrdPrm.ipaddr);
          }
          else
          {
            sim_bip_config_req->local_ip = SIM_IP_LOCAL_DYNAMIC;
          }
        }
        
        /* get data destination address */
        if(gprs_prm_ptr->v_dda)
        {
          if(gprs_prm_ptr->dda.oth_addr_type EQ IPv4)
          {
            if(gprs_prm_ptr->dda.v_ipv4_addr)/* local IP address */
            {
              memcpy(&sim_bip_config_req->destination_ip,
                     &gprs_prm_ptr->dda.ipv4_addr,
                     sizeof(gprs_prm_ptr->dda.ipv4_addr));
            }
          }
        }

        /* get SIM/ME interface transport level:destination port number for UDP */
        if(gprs_prm_ptr->v_itl)
        {
          sim_bip_config_req->destination_port = gprs_prm_ptr->itl.port_number;
        }
      }
    }

#else /* GPRS */
    PFREE(sim_bip_config_req);
    TRACE_ERROR("psaSIM_Bip_Config_Req(): ERROR stack does not support GPRS");
#endif /* GPRS */
  }
  else /* BEARER: unknown */ 
  {
    TRACE_ERROR("psaSIM_Bip_Config_Req(): ERROR unkown bearer"); 
  }
  /* send primitive to SIM */
  PSENDX(SIM,sim_bip_config_req);  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_Dti_Req               |
+-------------------------------------------------------------------+

  PURPOSE :  connection SIM with UDP,TRA or L2R in context CSD
             connection SIM with UDP or SNDCP in context GPRS
             request by proactive SIM CARD, set feature SAT CLASS e
             sent parameter depends on kind of connection (dtiConn)

             This function sends the primitive SIM_DTI_REQ which is 
             only responsible for the establishment of the SIM DTI 
             channel!             
*/
GLOBAL void psaSIM_Dti_Req ( ULONG link_id )
{
#ifdef DTI
  CHAR *p_ent_name=NULL; /* hold peer name */

  TRACE_EVENT("psaSIM_Dti_Req");

  if (simShrdPrm.sim_dti_chPrm EQ NULL)
  {
    /* when disconnecting if previos state connecting; 
     * will otherwise crash the stack in ACISAT473H */
    TRACE_ERROR ("simShrdPrm.sim_dti_chPrm uninitalized, aborting!");
  }
  else
  {
    /* alloc primitive */
    PALLOC(sim_dti_req,SIM_DTI_REQ);
    /* reset values */
    memset(sim_dti_req,0,sizeof(sim_dti_req));
    /* copy dti connection qualifier */
    sim_dti_req->dti_conn = simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiConn;
    /* set dti direction */
    sim_dti_req->dti_direction = SIM_DTI_INVERTED;
    
    if( simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiConn EQ SIM_DTI_CONNECT )
    {
      /* CASE: CONNECT IMMEDIATELY */
      /* copy link id */
      sim_dti_req->link_id = link_id;

      if(satShrdPrm.opchType EQ B_CSD)
      {      
        /* BEARER: CSD */
        /* copy entity_name 
         * handle different transport protocols: L2R, RA and UDP */
        switch(simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiUnit)
        {
          case DTI_ENTITY_UDP:
            p_ent_name = &UDP_NAME[0];
            break;
          case DTI_ENTITY_TRA:
            p_ent_name = &TRA_NAME[0];
            break;
          case DTI_ENTITY_L2R:
            p_ent_name = &L2R_NAME[0];
            break;
        }
        sim_dti_req->entity_name = (ULONG)p_ent_name; 
      }
#ifdef GPRS
      else if(satShrdPrm.opchType EQ B_GPRS)
      {
        /* BEARER: GPRS */
        /* copy entity_name 
         * handle different transport protocols: L2R, RA and UDP */
        switch(simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiUnit)
        {
          case DTI_ENTITY_UDP:
            p_ent_name = &UDP_NAME[0];            
            break;
          case DTI_ENTITY_SNDCP:
            p_ent_name = &SNDCP_NAME[0];
          break;
        }
        sim_dti_req->entity_name = (ULONG)p_ent_name; 
      }
#endif /* GPRS */
      else
      {
        TRACE_EVENT("psaSIM_Dti_Req:error");
      }
      
      PSENDX(SIM,sim_dti_req);       
    } /* end DTI open channel */
    
    else if ( simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiConn EQ SIM_DTI_DISCONNECT )
    {
      /* CASE: CLOSE DTI CHANNEL */
      /* copy link id */
      sim_dti_req->link_id = link_id;
      
      PSENDX(SIM,sim_dti_req); 
    }

    else if ( simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiConn EQ SIM_DTI_UNKNOWN)
    {
      /* no primitive has to be send, free mem */
      PFREE(sim_dti_req);

      /* check whether a BIP channel has to be closed */
      if( simShrdPrm.sim_dti_chPrm->sat_chn_prm.bipConn 
          EQ SIM_BIP_CLOSE_CHANNEL )
      {
        /* No action has to be performed, DTI channel already disconnected by 
         * SIM */
        TRACE_EVENT("psaSIM_Dti_Req: SIM-DTI channel already disconnected");

        /* inform dti manager about successfull disconnection */

        TRACE_EVENT("psaSIM_Dti_Req: close open BIP channel");
        /* send SIM_BIP_REQ to close BIP channel */        
        psaSIM_Bip_Req();
      }
      else
      {
        TRACE_ERROR("psaSIM_Dti_Req: ERROR: Unkown parameter combination");
      }     
    }
    else
    {
      /* no primitive has to be send, free mem */
      PFREE(sim_dti_req);
      
      TRACE_ERROR("psaSIM_Dti_Req: ERROR: Unkown parameter");
    }
  }    
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_SATBIPChn             |
+-------------------------------------------------------------------+

  PURPOSE : connect/disconnect/suspend/resume of BIP 
            (Bearer Independent Protocol) connection for SAT 
            (new for SAT_E_PATCH)  
*/

GLOBAL void psaSIM_SATBIPChn ( T_SIM_SAT_CHN chnInf,
                               void (*cb)(UBYTE bipConn, UBYTE chnId))
{
  TRACE_EVENT("psaSIM_SATBIPChn()");

#ifdef UART

  /* ALLOC shared paramenters for BIP if not yet done */
  if(!simShrdPrm.sim_dti_chPrm)
  {
    ACI_MALLOC(simShrdPrm.sim_dti_chPrm,sizeof(T_SIM_DTI_CH_PRM));
    /* reset parameters */
    memset(simShrdPrm.sim_dti_chPrm,0,sizeof(T_SIM_DTI_CH_PRM)); 
    TRACE_EVENT("ALLOC sim_dti_chPrm");
  }

  /* copy bip related parameters */
  simShrdPrm.sim_dti_chPrm->sat_chn_prm.bipConn = chnInf.bipConn;
  simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiConn = chnInf.dtiConn;
  simShrdPrm.sim_dti_chPrm->sat_chn_prm.chnId  = chnInf.chnId;
  simShrdPrm.sim_dti_chPrm->sat_chn_prm.genRes = chnInf.genRes;
  simShrdPrm.sim_dti_chPrm->sat_chn_prm.addRes = chnInf.addRes;
  /* callback to process after of BIP connection request */
  simShrdPrm.sim_dti_chPrm->bip_cb = cb;

  switch( chnInf.bipConn ) 
  {
    case SIM_BIP_CLOSE_CHANNEL:

      if(chnInf.dtiConn EQ SIM_DTI_DISCONNECT)
      {
        cmhSAT_OpChnClose( chnInf.bipConn, 1 );        
      }
      else 
      {
        if (dti_cntrl_is_dti_channel_connected (DTI_ENTITY_SIM, simShrdPrm.sat_class_e_dti_id) EQ TRUE)
        {
          /* SIM entity has disconnected the dti connection via SIM_DTI_BIP_IND */
          if (dti_cntrl_is_dti_channel_connected (DTI_ENTITY_UDP, simShrdPrm.sat_class_e_dti_id) EQ TRUE)
            cmhSAT_OpChnClose( chnInf.bipConn, 1 );
          else
            dti_cntrl_close_dpath_from_dti_id(simShrdPrm.sat_class_e_dti_id);
        }
        else
        {
          /* up to now there was not a dti connection with SIM entity */
          psaSIM_Bip_Req();
        }
      }
      break;      
    case SIM_BIP_OPEN_CHANNEL:
    case SIM_BIP_CHANNEL_SUSPENDED:
    case SIM_BIP_CHANNEL_RESUMED:
      psaSIM_Bip_Req();
      break;
    default:
      TRACE_ERROR("ERROR: psaSIM_SATBIPChn() unknown BIP status"); 
  } /* end of switch */

/* old behavior: test */     
#endif /* UART */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_SATChn                |
+-------------------------------------------------------------------+

  PURPOSE : start of DTI connection SIM - UDP,L2R,SNDCP,TRA for CSD
            /GPRS

  SAT E Design: Now only used for SIM DTI connections to SNDCP, L2R and TRA 
        
*/
#ifdef DTI
GLOBAL void psaSIM_SATChn ( T_SIM_SAT_CHN chnInf,
                       void (*cb)(UBYTE dtiConn, UBYTE chnId))
{
#ifdef GPRS
  U8 cid_array[2] = { 0,PDP_CONTEXT_CID_INVALID };
  cid_array[0] = satShrdPrm.chnTb.chnRefId;
#endif

  TRACE_EVENT("psaSIM_SATChn");

  if(!simShrdPrm.sim_dti_chPrm)
  {
    ACI_MALLOC(simShrdPrm.sim_dti_chPrm,sizeof(T_SIM_DTI_CH_PRM));
    TRACE_EVENT("ALLOC sim_dti_chPrm");
  }
  memcpy(&simShrdPrm.sim_dti_chPrm->sat_chn_prm,&chnInf,sizeof(T_SIM_SAT_CHN));
  simShrdPrm.sim_dti_chPrm->dti_cb = cb;/* callback for results of DTI connection -> SatSource */

  if( chnInf.dtiConn EQ SIM_DTI_CONNECT )
  {
     if(psa_search_SATSrcId() >= 0)
    {
      T_DTI_ENTITY_ID entity_list[2];
      entity_list[0]= DTI_ENTITY_SIM;
      entity_list[1]= (T_DTI_ENTITY_ID)chnInf.dtiUnit;

      if(satShrdPrm.opchType EQ B_CSD)
      {/* case immediately connection of SIM entity with L2R or TRA */
       /* case on demand connection of SIM entity with L2R or TRA */

        /* create a SAT class E DTI ID if not present */
        if ( simShrdPrm.sat_class_e_dti_id EQ DTI_DTI_ID_NOTPRESENT )
        {
          simShrdPrm.sat_class_e_dti_id = dti_cntrl_new_dti(DTI_DTI_ID_NOTPRESENT);
          TRACE_EVENT_P1("sat_class_e_dti_id = %d", simShrdPrm.sat_class_e_dti_id);
        }
                 
        /* establish SIM-L2R or SIM-TRA DTI connection */
        dti_cntrl_est_dpath( simShrdPrm.sat_class_e_dti_id,
                             entity_list,
                             2,
                             APPEND,
                             SIM_ENT_CSDconnect_dti_cb);
      }
#ifdef GPRS
      else if(satShrdPrm.opchType EQ B_GPRS)
      {/* case immediately: open and connect bip channel with help of DTI connection between SIM entity and UDP or SNDCP */
       /* case on demand: connect bip channel with help of DTI connection between SIM entity and UDP or SNDCP */

        /* create a SAT class E DTI ID if not present */
        if ( simShrdPrm.sat_class_e_dti_id EQ DTI_DTI_ID_NOTPRESENT )
        {
          simShrdPrm.sat_class_e_dti_id = dti_cntrl_new_dti(DTI_DTI_ID_NOTPRESENT);
          TRACE_EVENT_P1("sat_class_e_dti_id = %d", simShrdPrm.sat_class_e_dti_id);
        }

        if(chnInf.dtiUnit NEQ DTI_ENTITY_SNDCP)
        {
          dti_cntrl_est_dpath( simShrdPrm.sat_class_e_dti_id,
                               entity_list,
                               2,
                               APPEND,
                               SIM_ENT_GPRSconnect_dti_cb);
        }
        else
        {
          if(!srcc_reserve_sources( SRCC_SIM_SNDCP_LINK, satShrdPrm.chnTb.chnRefId)  OR
          !cmhSM_make_active_cid_list((T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, cid_array)    ) 
          {
            TRACE_FUNCTION("psaSIM_SATChn: Error cid list ");
          }
          else
          {
            set_conn_param_on_working_cid( (UBYTE)smEntStat.entOwn, DTI_ENTITY_SIM );
            cmhSM_connect_working_cid();
          }
        }
      }
#endif
    }
    else
    {
      TRACE_ERROR("psaSIM_SATChn: fatal ERROR: SAT source id invalid!");
    }
  }
}
#endif /* DTI */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_EvDatAvail            |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_EVENTLIST_REQ primitive to SIM.
            MMI informs about a change of the status of the
            Data available event (because it is part of the SAT event list)
*/

GLOBAL void psaSIM_EvDatAvail ( BOOL evStat )
{
  TRACE_EVENT("psaSIM_EvDatAvail");
  {
    PALLOC(sim_eventlist_req,SIM_EVENTLIST_REQ);
    switch(evStat)
    {
      case SIM_EVENT_DISABLE:
      case SIM_EVENT_ENABLE:
        sim_eventlist_req->event_data_avail = evStat;
        break;
      default:
        TRACE_EVENT("psaSIM_EvDatAvail:not valid evStat");
        break;
    }
    PSENDX(SIM,sim_eventlist_req);
  }
}

#endif /* #ifdef FF_SAT_E */

#ifdef FF_DUAL_SIM
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SIMS                     |
|                            ROUTINE : psaSIM_SelectSIM             |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_ACTIVATE_REQ primitive to SIM to Select the required SIM
*/
GLOBAL SHORT psaSIM_SelectSIM ( void )
{
  T_SIM_SET_PRM * pPrmSet;       /* points to used parameter set */

  TRACE_FUNCTION ("psaSIM_SelectSIM()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */
  if(!psa_IsVldOwnId(simShrdPrm.owner))
  {
    TRACE_EVENT("not a valid owner !!");
    return( -1 );
  }

  pPrmSet = &simShrdPrm.setPrm[simShrdPrm.owner];
  simShrdPrm.SIM_Selection = TRUE;

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_activate_req, SIM_ACTIVATE_REQ);

    sim_activate_req->proc = SIM_SELECT;
    sim_activate_req->sim_num = pPrmSet->SIM_Selected;

    PSENDX (SIM, sim_activate_req);
  }
  return 0;
}
#endif /*FF_DUAL_SIM*/  


/*==== EOF ========================================================*/

