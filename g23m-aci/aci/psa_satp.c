/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SATP
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
|             primitives send to the protocol stack adapter by the
|             SIM application toolkit.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef SIM_TOOLKIT

#ifndef PSA_SATP_C
#define PSA_SATP_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "aci_mem.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_sim.h" /* for simShrdPrm declaration */
#include "psa_sat.h"
#include "psa_sms.h"
#include "cmh.h"
#include "cmh_sat.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/
UBYTE temp ;

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SATP                |
|                                 ROUTINE : psa_sim_toolkit_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_TOOLKIT_IND primitive send by SIM.
            this is an indication that a STK command was received.

*/

GLOBAL void psa_sim_toolkit_ind
                         ( T_SIM_TOOLKIT_IND *sim_toolkit_ind )
{
  T_ACI_SAT_TERM_RESP resp_data;

  TRACE_FUNCTION ("psa_sim_toolkit_ind()");

/*
 *-------------------------------------------------------------------
 * update shared parameter
 *-------------------------------------------------------------------
 */

  satShrdPrm.stkCmdLen = sim_toolkit_ind -> stk_cmd.l_cmd;
  satShrdPrm.stkCmd    = sim_toolkit_ind -> stk_cmd.cmd;

  TRACE_EVENT_P1("%x", satShrdPrm.stkCmd[44] );

  psaSAT_DumpCmd ( &sim_toolkit_ind -> stk_cmd );

/*
 *-------------------------------------------------------------------
 * decode SIM toolkit command
 *-------------------------------------------------------------------
 */
  CCD_START;
  {
    UBYTE ccdRet;

    MCAST( cmd, STK_CMD );
    memset( cmd, 0, sizeof( T_STK_CMD ));

    ccdRet = ccd_decodeMsg (CCDENT_SAT,
                            DOWNLINK,
                            (T_MSGBUF *) &sim_toolkit_ind -> stk_cmd,
                            (UBYTE    *) _decodedMsg,
                            STK_CMD);

    if( ccdRet NEQ ccdOK OR !cmd->v_pas_cmd)
    {
      cmhSAT_STKCmdInd();
    }
    else if( cmd->pas_cmd.v_cmd_details AND cmd->pas_cmd.v_dev_ids )
     {
       /* store command details */
       satShrdPrm.cmdDet.cmdNr   = cmd->pas_cmd.cmd_details.cmd_nr;
       satShrdPrm.cmdDet.cmdType = cmd->pas_cmd.cmd_details.cmd_typ;
       satShrdPrm.cmdDet.cmdQlf  = cmd->pas_cmd.cmd_details.cmd_qlf;
 
 #ifdef TI_PS_FF_AT_P_CMD_CUST
       if (simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1)
       {
           /*
           ** Customised behaviour for the Cust1 MMI
           */
           switch (cmd->pas_cmd.cmd_details.cmd_typ)
           {
               case SAT_CMD_SETUP_CALL:
               case SAT_CMD_SEND_SMS:
               case SAT_CMD_SEND_SS:
               case SAT_CMD_SEND_USSD:
                   if (satShrdPrm.cust1StkCmd != (void *)0)
                   {
                        /*
                        ** The previous command has not been deleted.
                        */
                        ACI_MFREE(satShrdPrm.cust1StkCmd);
 
                        satShrdPrm.cust1StkCmd = (void *)0;
                        satShrdPrm.cust1StkCmdLen = 0;
                   }
 
                   /*
                   ** Allocate memory to store the command
                   */
                   ACI_MALLOC(satShrdPrm.cust1StkCmd, MAX_STK_CMD);
                   if (satShrdPrm.cust1StkCmd EQ (void *)0)
                   {
                       /*
                       ** Memory allocation has failed, return ME Unable to Process Cmd
                       */
                       TRACE_EVENT("Memory Allocation Failure");
                       psaSAT_InitTrmResp( &resp_data );
                       psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
                       break;
                   }
 
                   /*
                   ** Save the received data before proceeding
                   */
                   memset(satShrdPrm.cust1StkCmd, 0, MAX_STK_CMD);
                   /*
                   ** stkCmdLen is the BIT length of the received data, so shift right 3 (divide by 8)
                   ** in order to get the BYTE length
                   */
                   memcpy(satShrdPrm.cust1StkCmd, satShrdPrm.stkCmd, satShrdPrm.stkCmdLen >> 3);
                   satShrdPrm.cust1StkCmdLen = satShrdPrm.stkCmdLen;
 
                   /*
                   ** Continue the processing as for normal behaviour,
                   ** special processing will be applied later
                   **
                   ** No Break!
                   */
                   /*lint -fallthrough */
               case SAT_CMD_EVENT_LIST:
                   /*
                   ** This command is processed normally
                   */
                   if( !cmd->pas_cmd.v_cmd_prms OR !cmd->pas_cmd.cmd_prms.l_cmd_prms )
                   {
                       TRACE_EVENT("no parameters in envelope");
                       psaSAT_dasmMECmd( NULL );
                   }
                   else
                       psaSAT_dasmMECmd( &cmd->pas_cmd.cmd_prms );
                   break;
 
               case SAT_CMD_REFRESH:
                   /*
                   ** This command is to be forwarded to the MMI, for the user to be given the
                   ** chance to accept or reject the Refresh action. The user response will be
                   ** contained in a SATR Response indication, and we must be prepared to
                   ** process that indication in a different manner.
                   */
                   satShrdPrm.cust1SimRefreshRespRqd = TRUE;
 
                   /*
                   ** Forward the command to the MMI
                   */
                   cmhSAT_STKCmdInd();
                   break;
 
               default:
                   /*
                   ** If the command is handled by the MMI pass it up in a %SATI indication
                   */
                   if (cmhSAT_IsStkCmdForMmi(cmd->pas_cmd.cmd_details.cmd_typ,
                                                                    cmd->pas_cmd.cmd_details.cmd_qlf) EQ FALSE)
                   {
                       /*
                       ** Otherwise Determine whether the command is handled in the Modem Part
                       */
                       switch (cmd->pas_cmd.cmd_details.cmd_typ)
                       {
                           case( SAT_CMD_SEND_DTMF ):
                           case( SAT_CMD_RUN_AT ):
                           case( SAT_CMD_LAUNCH_BROWSER ):
                           case (SAT_CMD_PROV_LOC_INFO):
 #ifdef FF_SAT_E           
                           case( SAT_CMD_OPEN_CHANNEL ):
 #endif /* FF_SAT_E */          
                               if( !cmd->pas_cmd.v_cmd_prms OR !cmd->pas_cmd.cmd_prms.l_cmd_prms )
                               {
                                   TRACE_EVENT("no parameters in envelope");
                                   psaSAT_dasmMECmd( NULL );
                               }
                               else
                                   psaSAT_dasmMECmd( &cmd->pas_cmd.cmd_prms );
                               break;
 #ifdef FF_SAT_E 
                           /* special handling for CLOSE CHANNEL, no further decoding required.
                           all that is needed can be found in device identities */
                           case( SAT_CMD_CLOSE_CHANNEL ):
 				             
                                 cmhSAT_CloseChannel();
 			     
                               break;
       
                           /* special handling for SEND DATA, no further decoding required.
                           all that is needed can be found in command details */
                           case( SAT_CMD_SEND_DATA ):
                               cmhSAT_SendData();
                               break;
       
                           /* special handling for GET CHANNEL STATUS, no further decoding required */
                           case( SAT_CMD_GET_CHANNEL_STATUS ):
                               cmhSAT_GetChannelStatus();
                               break;
       
                           /* special handling for RECEIVE DATA, no further decoding required */
                           case( SAT_CMD_RECEIVE_DATA ):
                               cmhSAT_STKUsrNtfy();
                               break;
 #endif /* FF_SAT_E */ 
 
                           /*
                           ** If the command is not handled by the Modem Part, return a "Beyond ME
                           ** Capabilities" response to the SIM Entity.
                           */
                           default:
                               TRACE_EVENT("Unable to handle command");
                               psaSAT_InitTrmResp( &resp_data );
                               psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
                       }
                   }
           }
       }
       else
 #endif /* TI_PS_FF_AT_P_CMD_CUST */
       {
           /* if command is addressed to ACI */
           switch( cmd->pas_cmd.cmd_details.cmd_typ )
           {
             case( SAT_CMD_SETUP_CALL ):
             case( SAT_CMD_SEND_SS ):
             case( SAT_CMD_SEND_SMS ):
             case( SAT_CMD_SEND_USSD ):
             case( SAT_CMD_SEND_DTMF ):
             case( SAT_CMD_EVENT_LIST ):
             case( SAT_CMD_RUN_AT ):
             case( SAT_CMD_LAUNCH_BROWSER ):
             case (SAT_CMD_PROV_LOC_INFO):
 #ifdef FF_SAT_E           
             case( SAT_CMD_OPEN_CHANNEL ):
 #endif /* FF_SAT_E */          
         if( !cmd->pas_cmd.v_cmd_prms OR !cmd->pas_cmd.cmd_prms.l_cmd_prms )
               {
                 TRACE_EVENT("no parameters in envelope");
                 psaSAT_dasmMECmd( NULL );
               }
               else
                 psaSAT_dasmMECmd( &cmd->pas_cmd.cmd_prms );
               break;
 #ifdef FF_SAT_E 
             /* special handling for CLOSE CHANNEL, no further decoding required.
                all that is needed can be found in device identities */
             case( SAT_CMD_CLOSE_CHANNEL ):
               /* Channel Id invalid */ 
        	 temp = 	 (UBYTE) cmd->pas_cmd.dev_ids.dest_dev;            
               if ( (CHANNEL_ID_1 NEQ (UBYTE)(temp & 0x0f)) OR ( satShrdPrm.chnTb.lnkStat  EQ SIM_NO_LINK ) )
               {
                    /* Close Channel with an invalid channel identifier */
                   psaSAT_InitTrmResp( &resp_data );
	            resp_data.chnStat = FALSE; 
                    resp_data.add_content = ADD_BIP_CHANID_NT_VLD;
                    psaSAT_SendTrmResp( RSLT_BEARIND_PERR, &resp_data );
                }
                else
                {
                   cmhSAT_CloseChannel();
                }
               break;
 
             /* special handling for SEND DATA, no further decoding required.
                all that is needed can be found in command details */
             case( SAT_CMD_SEND_DATA ):
			 	
	          /* Channel Id invalid */ 
        	 temp = 	 (UBYTE) cmd->pas_cmd.dev_ids.dest_dev;            
               if ( (CHANNEL_ID_1 NEQ (UBYTE)(temp & 0x0f)) OR ( satShrdPrm.chnTb.lnkStat  EQ SIM_NO_LINK ) )
               {
                    /* Close Channel with an invalid channel identifier */
                   psaSAT_InitTrmResp( &resp_data );
                   resp_data.chnStat = FALSE; 
                   resp_data.add_content = ADD_BIP_CHANID_NT_VLD;
                    psaSAT_SendTrmResp( RSLT_BEARIND_PERR, &resp_data );
                }
             else
              {
               
                cmhSAT_SendData();
               }
               break;
 
             /* special handling for GET CHANNEL STATUS, no further decoding required */
             case( SAT_CMD_GET_CHANNEL_STATUS ):
               
               cmhSAT_GetChannelStatus();
               break;
 
             /* special handling for RECEIVE DATA, no further decoding required */
             case( SAT_CMD_RECEIVE_DATA ):
               
               cmhSAT_STKUsrNtfy();
               break;
 #endif /* FF_SAT_E */ 
             /* otherwise pass command to user */
             default:
               cmhSAT_STKCmdInd();
           }
       }
     }
    else
    {
      /* respond with "error, required values are missing" */
      TRACE_EVENT("parameters are missing");
      psaSAT_InitTrmResp( &resp_data );
      psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
    }
  }
  CCD_END;

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_toolkit_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SATP                |
|                                 ROUTINE : psa_sim_toolkit_cnf     |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_TOOLKIT_CNF primitive send by SIM.
            this is a confirmation to a previous STK command.

*/

GLOBAL void psa_sim_toolkit_cnf( T_SIM_TOOLKIT_CNF *sim_toolkit_cnf )
{
  void   *p             = NULL;
  BOOL   SIM_error      = FALSE;
  BOOL   not_allw       = FALSE;
  BOOL   allw           = FALSE;
  void   *p_allw        = NULL;
  UBYTE  *p_v_allw      = NULL;
  void   *p_not_allw    = NULL;
  UBYTE  *p_v_not_allw  = NULL;
  void   *p_allw_mdfy   = NULL;
  UBYTE  *p_v_allw_mdfy = NULL;
  SHORT  StructSize     = 0;
  UBYTE  DynEnv         = '\0';

  TRACE_FUNCTION ("psa_sim_toolkit_cnf()");
/*
 *-------------------------------------------------------------------
 * update shared parameter
 *-------------------------------------------------------------------
 */
  satShrdPrm.stkCmdLen = sim_toolkit_cnf -> stk_cmd.l_cmd;
  satShrdPrm.stkCmd    = sim_toolkit_cnf -> stk_cmd.cmd;
  satShrdPrm.stkError  = sim_toolkit_cnf -> cause;

  psaSAT_DumpCmd ( &sim_toolkit_cnf -> stk_cmd );

/*
 *-------------------------------------------------------------------
 * if request was invoked by MMI pass reply to MMI
 *-------------------------------------------------------------------
 */
  if( sim_toolkit_cnf -> req_id EQ SRQ_MMI )
  {
    cmhSAT_STKCmdCnf();
  }

/*
 *-------------------------------------------------------------------
 * if request was invoked by ACI decode envelope command result
 *-------------------------------------------------------------------
 */
  else if( sim_toolkit_cnf -> req_id EQ SRQ_ACI )
  {
    if( sim_toolkit_cnf -> stk_cmd.l_cmd EQ 0 OR
        sim_toolkit_cnf -> cause EQ SIM_CAUSE_SAT_BUSY)
    {
      SIM_error = TRUE;

      switch ( sim_toolkit_cnf -> cause )
      {
      case SIM_CAUSE_OTHER_ERROR:
      case SIM_CAUSE_SAT_BUSY:
      case SIM_CAUSE_CARD_REMOVED:
      case SIM_CAUSE_PIN1_EXPECT:
      case SIM_CAUSE_PIN1_BLOCKED:
      case SIM_CAUSE_PUK1_EXPECT:
      case SIM_CAUSE_PUK1_BLOCKED:
      case SIM_CAUSE_PIN2_EXPECT:
      case SIM_CAUSE_PIN2_BLOCKED:
      case SIM_CAUSE_PUK2_EXPECT:
      case SIM_CAUSE_PUK2_BLOCKED:
         /* in case of certain errors, assume not allowed */
        not_allw = TRUE;
        sim_toolkit_cnf -> stk_cmd.cmd[0] = CCR_NOT_ALLW;
        break;

      default:
         /* in case of a positive result without further info,
                     assume allowed not modified */
        allw = TRUE;
        sim_toolkit_cnf -> stk_cmd.cmd[0] = CCR_ALLW_NO_MDFY;
        break;
      }
      sim_toolkit_cnf -> stk_cmd.cmd[1] = 0; /* no extra TLVs */
      satShrdPrm.stkCmdLen = 2<<3;  /* 2 bytes result */
    }

    else
    {
      SIM_error = FALSE;
    }


    {
      CCD_START;
      {
        UBYTE ccdRet;
        MCAST( env, ENV_RES );
        MCAST( smc_env, ENV_RES_SMC );

        switch( satShrdPrm.SIMCCParm.ccAct )
        {
          case( CC_ACT_CAL ):
          case( CC_ACT_SS ):
          case( CC_ACT_USSD ):
            memset( env, 0, sizeof( T_ENV_RES ));
            p_v_not_allw  = &env->v_ccr_not_allw;
            p_v_allw      = &env->v_ccr_allw;
            p_v_allw_mdfy = &env->v_ccr_allw_mdfy;
            p_not_allw    = &env->ccr_not_allw;
            p_allw        = &env->ccr_allw;
            p_allw_mdfy   = &env->ccr_allw_mdfy;
            DynEnv        = ENV_RES;
            break;
          case( SMC_ACT_MO ):
            memset( smc_env, 0, sizeof( T_ENV_RES_SMC ));
            p_v_not_allw  = &smc_env->v_smcr_not_allw;
            p_v_allw      = &smc_env->v_smcr_allw;
            p_v_allw_mdfy = &smc_env->v_smcr_allw_mdfy;
            p_not_allw    = &smc_env->smcr_not_allw;
            p_allw        = &smc_env->smcr_allw;
            p_allw_mdfy   = &smc_env->smcr_allw_mdfy;
            DynEnv        = ENV_RES_SMC;
            break;
        }

        if (SIM_error)
        {
          if(p_v_not_allw AND p_v_allw)   /* To avoid the NULL poiner usage, 613 lint warning */
          {
            *p_v_not_allw = not_allw;
            *p_v_allw     = allw;
          }
        }
         /* otherwise decode message */
        else
        {
          ccdRet = ccd_decodeMsg (CCDENT_SAT,
                                  DOWNLINK,
                                  (T_MSGBUF *) &sim_toolkit_cnf -> stk_cmd,
                                  (UBYTE    *) _decodedMsg,
                                  DynEnv );

          if( ccdRet NEQ ccdOK )
          {
            /* in case of a result failure, pretend allowed not modified */
            if(p_v_not_allw AND p_v_allw AND p_v_allw_mdfy) /* To avoid the NULL poiner usage, 613 lint warning */

            {
              if( psaSAT_ccdErrChk() NEQ 0 AND
                (!(*p_v_allw) AND !(*p_v_not_allw) AND !(*p_v_allw_mdfy)))
              {
                 memset( env, 0, StructSize );
                *p_v_allw = TRUE;
                sim_toolkit_cnf -> stk_cmd.cmd[0] = CCR_ALLW_NO_MDFY;
                sim_toolkit_cnf -> stk_cmd.cmd[1] = 0;
                satShrdPrm.stkCmdLen = 2<<3;  /* 2 bytes result */
              }
            }
          }
        }

        /* process result */
        if(p_v_not_allw AND p_v_allw AND p_v_allw_mdfy) /* To avoid the NULL poiner usage, 613 lint warning */

        {
          if     ( *p_v_allw      ) p = p_allw;
          else if( *p_v_not_allw  ) p = p_not_allw;
          else if( *p_v_allw_mdfy ) p = p_allw_mdfy;


          satShrdPrm.ntfy = USR_NTF_CC_SIM;
          cmhSAT_STKUsrNtfy();

          switch( satShrdPrm.SIMCCParm.ccAct )
          {
            case( CC_ACT_CAL ):

              if( cmhSAT_ResCalCntrlBySIM( sim_toolkit_cnf -> stk_cmd.cmd, p ))
              {
                /* keep primitive for terminal response */
                satShrdPrm.stkCnfPrim = sim_toolkit_cnf;
                CCD_END;
                return;
              }
              break;

            case( CC_ACT_SS ):

              if( cmhSAT_ResSSCntrlBySIM( sim_toolkit_cnf -> stk_cmd.cmd, p ))
              {
                /* keep primitive for terminal response */
                satShrdPrm.stkCnfPrim = sim_toolkit_cnf;
                CCD_END;
                return;
              }
              break;

            case( CC_ACT_USSD ):

              if( cmhSAT_ResUSSDCntrlBySIM( sim_toolkit_cnf -> stk_cmd.cmd, p ))
              {
                /* keep primitive for terminal response */
                /*satShrdPrm.stkCnfPrim = sim_toolkit_cnf;*/
                CCD_END;
                return;
              }
              break;

            case( SMC_ACT_MO ):
              if( cmhSAT_ResSMCntrlBySIM( sim_toolkit_cnf -> stk_cmd.cmd, p ))
              {
                /* keep primitive for terminal response */
                /*satShrdPrm.stkCnfPrim = sim_toolkit_cnf;*/
                CCD_END;
                return;
              }
              break;

            default:
              TRACE_EVENT("UNEXP CC ACTION IN SAT PARMS");
              break;
          }
        }
      }
      satShrdPrm.SIMCCParm.ccAct = NO_CC_ACT;
      CCD_END;
    }
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_toolkit_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SATP                |
|                                 ROUTINE : psa_sim_file_update_ind |
+-------------------------------------------------------------------+

  PURPOSE : processes the FILE_UPDATE_IND primitive send by SIM.
            It provides a list of updated EFs.

*/

GLOBAL void psa_sim_file_update_ind
                      ( T_SIM_FILE_UPDATE_IND *sim_file_update_ind )
{
  SHORT i;
  BOOL send_confirm_now=TRUE;
    
  TRACE_FUNCTION ("psa_sim_file_update_ind()");
  
/*
 *-------------------------------------------------------------------
 * store the primitive and start processing
 *-------------------------------------------------------------------
 */
  satShrdPrm.fu_ind = sim_file_update_ind;

 /*
  *   Broadcast %SIMEF unsolicited event to all interested parties.
  */
  for(i=CMD_SRC_LCL;i<CMD_SRC_MAX;i++)
  {    
    if (simShrdPrm.SIMEFMode[i] EQ SIMEF_MODE_ON)
    {
      /*
      *   As we have reported the EF update, it is not now the
      *   responsibility of this function to send the response
      *   immediately.
      */
      send_confirm_now=FALSE;

      R_AT(RAT_P_SIMEF,(T_ACI_CMD_SRC)i)(sim_file_update_ind);

    }
  }

 /*
  *   If we didn't report the EF update to anyone we must send
  *   the response now.
  */
  if (send_confirm_now NEQ FALSE)
    psaSAT_FUConfirm (-1, NOT_PRESENT_16BIT);
}

#endif /* #ifdef SIM_TOOLKIT */

/*==== EOF =========================================================*/
