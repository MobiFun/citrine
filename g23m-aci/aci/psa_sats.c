/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SATS
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
|             protocol stack adapter for the SIM application toolkit
|             module.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef SIM_TOOLKIT

#ifndef PSA_SATS_C
#define PSA_SATS_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "aci.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_cc.h"
#include "psa_sat.h"
#include "cmh.h"
#include "cmh_sat.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SATS                     |
|                            ROUTINE : psaSAT_STKBuildCmd           |
+-------------------------------------------------------------------+

  PURPOSE : build envelope of a SIM toolkit command

*/

GLOBAL SHORT psaSAT_STKBuildCmd ( T_stk_cmd *stk_cmd )
{
  UBYTE ccdRet;

  TRACE_FUNCTION ("psaSAT_STKBuildCmd()");

  stk_cmd->l_cmd = MAX_STK_CMD << 3;
  stk_cmd->o_cmd = 0;

  ccdRet = ccd_codeMsg (CCDENT_SAT,
                        UPLINK,
                        (T_MSGBUF *) stk_cmd,
                        (UBYTE    *) _decodedMsg,
                        ENV_CMD);

  if (ccdRet NEQ ccdOK)
  {
    TRACE_EVENT_P1 ("CCD Coding Error: %d", ccdRet); 
    return -1;
  }
  return 0;
} 


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SATS                     |
|                            ROUTINE : psaSAT_STKResponse           |
+-------------------------------------------------------------------+

  PURPOSE : respond to SIM toolkit command

*/

GLOBAL SHORT psaSAT_STKResponse ( void )
{
  T_SAT_SET_PRM * pPrmSet;       /* points to used parameter set */

  TRACE_FUNCTION ("psaSAT_STKResponse()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_toolkit_res, SIM_TOOLKIT_RES);

    if(psa_IsVldOwnId((T_OWN)satShrdPrm.owner) AND 
       satShrdPrm.owner NEQ OWN_SRC_INV      ) 
    {
      pPrmSet = &satShrdPrm.setPrm[satShrdPrm.owner]; 

      /* fill in primitive parameter: STK envelope */
      sim_toolkit_res -> stk_cmd.l_cmd = pPrmSet -> stkCmdLen << 3;
      sim_toolkit_res -> stk_cmd.o_cmd = 0;
      memcpy( sim_toolkit_res -> stk_cmd.cmd, pPrmSet -> stkCmd, 
              pPrmSet -> stkCmdLen );
    }
    
    else
    {
      UBYTE ccdRet;

      sim_toolkit_res -> stk_cmd.l_cmd = MAX_STK_CMD<<3;
      sim_toolkit_res -> stk_cmd.o_cmd = 0;

      ccdRet = ccd_codeMsg (CCDENT_SAT,
                            UPLINK,
                            (T_MSGBUF *) &sim_toolkit_res -> stk_cmd,
                            (UBYTE    *) _decodedMsg,
                            TERM_RESP);

      if( ccdRet NEQ ccdOK )
      {
        TRACE_EVENT_P1("CCD Coding Error: %d",ccdRet ); 
        PFREE( sim_toolkit_res );
        return( -1 );
      }

      satShrdPrm.stkCmdLen = sim_toolkit_res -> stk_cmd.l_cmd;
      satShrdPrm.stkCmd    = sim_toolkit_res -> stk_cmd.cmd;
      satShrdPrm.ntfy      = USR_NTF_TRM_RSP;

      cmhSAT_STKUsrNtfy();
    }

    psaSAT_DumpCmd ( &sim_toolkit_res -> stk_cmd );

    PSENDX (SIM, sim_toolkit_res);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SATS                     |
|                            ROUTINE : psaSAT_STKEnvelope           |
+-------------------------------------------------------------------+

  PURPOSE : send envelope of a SIM toolkit command

*/

GLOBAL SHORT psaSAT_STKEnvelope ( const T_stk_cmd *stk_cmd )
{
  T_SAT_SET_PRM * pPrmSet;       /* points to used parameter set */

  TRACE_FUNCTION ("psaSAT_STKEnvelope()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_toolkit_req, SIM_TOOLKIT_REQ); /* T_SIM_TOOLKIT_REQ */

    sim_toolkit_req -> source = SRC_MMI;

    if(psa_IsVldOwnId((T_OWN)satShrdPrm.owner) AND 
       satShrdPrm.owner NEQ OWN_SRC_INV      ) 
    {
      pPrmSet = &satShrdPrm.setPrm[satShrdPrm.owner]; 

      /* fill in primitive parameter: STK envelope */
      sim_toolkit_req -> req_id        = SRQ_MMI;
      sim_toolkit_req -> stk_cmd.l_cmd = pPrmSet -> stkCmdLen << 3;
      sim_toolkit_req -> stk_cmd.o_cmd = 0;
      memcpy( sim_toolkit_req -> stk_cmd.cmd, pPrmSet -> stkCmd, 
              pPrmSet -> stkCmdLen );
    }
    
    else
    {
      /* If ccAct is valid then set the req_id to SRQ_ACI to process the same
         in the confirmation. Otherwise set the req_id to NO_VALID_SRQ */
      if (satShrdPrm.SIMCCParm.ccAct NEQ NO_CC_ACT)
      {
        if (satShrdPrm.Cbch_EvtDnl)   /* Will be TRUE for CBCH and Event Downlaod */
        {
          sim_toolkit_req->req_id = NO_VALID_SRQ;
        }
        else
        {
          sim_toolkit_req->req_id = SRQ_ACI;
        }
      }
      else
      {
        sim_toolkit_req->req_id = NO_VALID_SRQ;
      }
      if (stk_cmd NEQ NULL)
      {
        sim_toolkit_req->stk_cmd = *stk_cmd; /* Struct copy */
      }
      else
      {
        if (psaSAT_STKBuildCmd (&sim_toolkit_req->stk_cmd) < 0)
        {
          PFREE( sim_toolkit_req );
          return( -1 );
        }
      }
    }

    psaSAT_DumpCmd ( &sim_toolkit_req -> stk_cmd );

    PSENDX (SIM, sim_toolkit_req);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA                     |
|                                 ROUTINE : checkRefreshUserResp|
+-------------------------------------------------------------------+

  PURPOSE : check whether the user has accepted the Refresh Request or Not
                   'p' is a pointer to the StkCmd
*/

LOCAL BOOL checkRefreshUserResp ( UBYTE *p )
{
  const UBYTE TermRespRefreshOk [12]  =
  {
   0x81,           /* 0: command details tag            */
   0x03,           /* 1: command details length         */
   0x00,           /* 2: not used                 */
   0x01,           /* 3: command REFRESH       */
   0x00,           /* 4: not used                       */
   0x82,           /* 5: device details tag             */
   0x02,           /* 6: device details length          */
   0x82,           /* 7: source ME                      */
   0x81,           /* 8: destination SIM                */
   0x83,           /* 9: result tag                     */
   0x00,           /* 10: not used                  */
   0x00            /* 11: result OK                      */
  };


  TRACE_FUNCTION("checkRefreshUserResp");

    if (!memcmp((const UBYTE*) p, TermRespRefreshOk,2))  /* Command Details Tag and Length*/
    {
      if ((*(p+3) EQ TermRespRefreshOk[3]) AND  /* Is this the REFRESH Command ... 'no' here would be trouble */
           (!memcmp((const UBYTE*)(p+5), TermRespRefreshOk+5, 5)) AND /* Device Details and Result Tag are correct */
           (*(p+11) EQ TermRespRefreshOk[11])) /* The result is OK */
      {
        return TRUE; /* The User has accepted the Refresh Request */ 
      }
      else
      {
        return FALSE; /* The User has rejected the Refresh Request */
      }
    }
    else
    {
      return FALSE; /* The User has rejected the Refresh Request */
    }
}

#ifdef TI_PS_FF_AT_P_CMD_CUST
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SATS                     |
|                            ROUTINE : psaSAT_SendRefreshUserRes           |
+-------------------------------------------------------------------+

  PURPOSE : Send the User Response to a SIM Toolkit Refresh Request

*/

GLOBAL void psaSAT_SendRefreshUserRes( SHORT len, UBYTE *satCmd )
{

  TRACE_FUNCTION ("psaSAT_SendRefreshUserRes()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (sim_refresh_user_res, SIM_REFRESH_USER_RES);

    /* fill in primitive parameter: STK envelope */
    sim_refresh_user_res->user_accepts = checkRefreshUserResp(satCmd);

    sim_refresh_user_res -> stk_cmd.l_cmd = len << 3;
    sim_refresh_user_res -> stk_cmd.o_cmd = 0;
    memcpy( sim_refresh_user_res -> stk_cmd.cmd, satCmd, len );

    psaSAT_DumpCmd ( &sim_refresh_user_res -> stk_cmd );

    PSENDX (SIM, sim_refresh_user_res);
  }

  return;
}
#endif /* TI_PS_FF_AT_P_CMD_CUST */
#endif /* #ifdef SIM_TOOLKIT */
/*==== EOF ========================================================*/
 
