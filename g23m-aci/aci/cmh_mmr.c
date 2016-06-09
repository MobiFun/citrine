/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MMR
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
|  Purpose :  This module defines the functions which are responsible
|             for the responses of the protocol stack adapter for
|             mobility management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_MMR_C
#define CMH_MMR_C
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
#include "aci_mem.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_mm.h"
#include "psa_sim.h"
#include "cmh.h"
#include "cmh_mm.h"

#ifdef DTI
 #include "dti.h"      /* functionality of the dti library */
 #include "dti_conn_mng.h"
 #include "dti_cntrl_mng.h"
#endif

#include "cmh_sim.h"

#ifdef GPRS
  #include "gaci.h"
  #include "gaci_cmh.h"
  #include "psa_gmm.h"
  #include "cmh_gmm.h"
#endif

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
LOCAL void cmhMM_Registration_AutoMode( T_ACI_AT_CMD at_cmd_id );
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMR                      |
|                            ROUTINE : cmhMM_Registered             |
+-------------------------------------------------------------------+

  PURPOSE : ME is registered to the network

*/

GLOBAL SHORT cmhMM_Registered ( void )
{
  SHORT mncCur, mccCur;   /* holds mnc and mcc of current PLMN */
  UBYTE cmdBuf;           /* buffers current command */

  TRACE_FUNCTION ("cmhMM_Registered()");

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( mmEntStat.curCmd )
  {
    case( AT_CMD_NONE ):
    case( AT_CMD_COPS ):
    case (AT_CMD_P_COPS):
    case( AT_CMD_NRG  ):

      /* check for home PLMN or roaming */
      if( mmShrdPrm.usedPLMN.v_plmn EQ VLD_PLMN  )
      {
        cmhMM_CnvrtPLMN2INT( mmShrdPrm.usedPLMN.mcc,
                             mmShrdPrm.usedPLMN.mnc,
                             &mccCur, &mncCur );

        /* Store the new PLMN and the IMSI in the FFS */
        if (!cmhMM_OperatorStoreInFFS(mmShrdPrm.usedPLMN.mcc,
                                                        mmShrdPrm.usedPLMN.mnc,
                                                        simShrdPrm.imsi.field))
            TRACE_EVENT("Could not write PLMN and IMSI in FFS");                                            
                                                        

        if( cmhSIM_plmn_is_hplmn (mccCur, mncCur))
        {
          cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_Reg);
        }
        else
        {
          cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_Roam);
        }
      }

      if( mmEntStat.curCmd NEQ AT_CMD_NONE )
      {
#if defined (GPRS) AND defined (DTI)
        psaG_MM_CMD_SET_REGMD ( mmShrdPrm.regMode );   /* restore former registration mode*/
#else
        /* restore former registration mode */
        psaMM_SetRegMode ( mmShrdPrm.regMode );
#endif

        cmdBuf = mmEntStat.curCmd;
        mmEntStat.curCmd = AT_CMD_NONE;

        /* From the flight mode if the user registers directly
           change the variable to full functionality. */
        if (CFUNfun NEQ CFUN_FUN_Full)
        {
          CFUNfun = CFUN_FUN_Full;
        }

        R_AT( RAT_OK, mmEntStat.entOwn )
          ( cmdBuf );

        /* log result */
        cmh_logRslt ( mmEntStat.entOwn, RAT_OK, (T_ACI_AT_CMD)cmdBuf, 
                              -1, BS_SPEED_NotPresent ,CME_ERR_NotPresent );
      }
      break;
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMR                      |
|                            ROUTINE : cmhMM_Deregistered           |
+-------------------------------------------------------------------+

  PURPOSE : ME is deregistered from network

*/

GLOBAL SHORT cmhMM_Deregistered ( void )
{
  T_ACI_RETURN ret;

  TRACE_FUNCTION ("cmhMM_Deregistered()");

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( mmEntStat.curCmd )
  {
    case( AT_CMD_NONE ):
   /*
    *----------------------------------------------------------------
    * process event if no command was invoked
    *----------------------------------------------------------------
    */
      switch( mmShrdPrm.deregCs )
      {
        case( NREG_LIMITED_SERVICE ):
        case( NREG_NO_SERVICE ):

          if (mmShrdPrm.regModeAutoBack)
          {
            /*
             * Fallback to automatic mode is allowed, fallback.
             * If MM is not in full service and the registration mode
             * is switched from manual to automatic,
             * MM starts a network search immediately.
             */
            mmShrdPrm.regModeBeforeAbort = mmShrdPrm.regMode;
            mmShrdPrm.regMode = MODE_AUTO;
            mmShrdPrm.regModeAutoBack = FALSE;
#if defined (GPRS) AND defined (DTI)
            psaG_MM_CMD_SET_REGMD ( MODE_AUTO );
#else
            psaMM_SetRegMode (MODE_AUTO);
#endif
          }

          cmhMM_Ntfy_NtwRegistrationStatus(cmhMM_GetNregCREGStat());
          break;
      }
      break;

    case( AT_CMD_CFUN ):
   /*
    *----------------------------------------------------------------
    * process event for +CFUN command
    *----------------------------------------------------------------
    */
      mmEntStat.curCmd = AT_CMD_NONE;

      cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_NoSearch);

      if (simEntStat.curCmd NEQ AT_CMD_CFUN)     /* Has SIM already finished flushing of LDN entries? */
      {
        R_AT( RAT_OK, mmEntStat.entOwn )
          ( AT_CMD_CFUN );

        /* log result */
        cmh_logRslt ( mmEntStat.entOwn, RAT_OK, AT_CMD_CFUN, -1, 
                                  BS_SPEED_NotPresent,CME_ERR_NotPresent );
      }

      break;

    case( AT_CMD_BAND ):
   /*
    *----------------------------------------------------------------
    * process event for %BAND command
    *----------------------------------------------------------------
    */
      cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_NoSearch);

      /* switch back to full functionnality */
      ret = sAT_PlusCFUN( mmEntStat.entOwn,
                          CFUN_FUN_Full,
                          CFUN_RST_NotPresent );


     /* SPR#919 - SH - fix advised by CLB */
     mmEntStat.curCmd = AT_CMD_NONE;
      simEntStat.curCmd = AT_CMD_NONE;
      /* end SH */
      
      if( ret EQ AT_CMPL )
      {
        R_AT( RAT_OK, mmEntStat.entOwn )
                (AT_CMD_BAND);
      }
      else
      {
        TRACE_EVENT_P1
            ("Fatal Error in sAT_PlusCFUN switching on !!!, ret_type: %d", ret);
        R_AT( RAT_CME, mmEntStat.entOwn )
                        ( AT_CMD_BAND, CME_ERR_Unknown );
        
      }
      /*SPR#919 - SH - fix advised by CLB
      mmEntStat.curCmd = AT_CMD_NONE;
      simEntStat.curCmd = AT_CMD_NONE;
      */
      break;

    case( AT_CMD_COPS ):
    case( AT_CMD_P_COPS):
      /*
       *----------------------------------------------------------------
       * process event for +COPS command
       *----------------------------------------------------------------
       */
      switch( mmShrdPrm.COPSmode )
      {
        case( COPS_MOD_Auto   ):
        case( COPS_MOD_Man   ):

         cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_Denied);

         /* If the current running command is AT+COPS=?, we should not do the below 
            actions. pnn_read_cnt will be valid when AT+COPS=? is running. */
         if (mmShrdPrm.pnn_read_cnt EQ 0)
         {
           mmEntStat.curCmd = AT_CMD_NONE;
#if defined (GPRS) AND defined (DTI)
          psaG_MM_CMD_SET_REGMD ( mmShrdPrm.regMode );   /* restore former registration mode*/
#else
          /* restore former registration mode */
          psaMM_SetRegMode ( mmShrdPrm.regMode);
#endif

          R_AT( RAT_CME, mmEntStat.entOwn )
            ( mmEntStat.curCmd, cmhMM_GetNregCMEStat() );

          /* log result */
          cmh_logRslt ( mmEntStat.entOwn, RAT_CME, AT_CMD_COPS, -1, BS_SPEED_NotPresent ,
            cmhMM_GetNregCMEStat() );
         }
          break;
        case( COPS_MOD_Dereg   ):

          cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_NoSearch);

          R_AT( RAT_OK, mmEntStat.entOwn )
            ( mmEntStat.curCmd );

          /* log result */
          cmh_logRslt ( mmEntStat.entOwn, RAT_OK, mmEntStat.curCmd, -1, 
                                              BS_SPEED_NotPresent,CME_ERR_NotPresent );

          mmEntStat.curCmd = AT_CMD_NONE;
          break;

        case( COPS_MOD_SetOnly   ):

          cmhMM_Ntfy_NtwRegistrationStatus(cmhMM_GetNregCREGStat());
          break;

        case( COPS_MOD_Both   ):

            /* Implements Measure 87 */
            cmhMM_Registration_AutoMode( mmEntStat.curCmd );
          break;
      }
      break;

    case( AT_CMD_NRG ):
   /*
    *----------------------------------------------------------------
    * process event for %NRG command
    *----------------------------------------------------------------
    */
    switch( cmhPrm[mmEntStat.entOwn].mmCmdPrm.NRGsrvMode )
    {
    case( NRG_SVMD_Full    ):

      switch( cmhPrm[mmEntStat.entOwn].mmCmdPrm.NRGregMode )
      {
      case( NRG_RGMD_Auto   ):
      case( NRG_RGMD_Manual ):

        mmEntStat.curCmd = AT_CMD_NONE;

        cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_Denied);

#if defined (GPRS) AND defined (DTI)
        psaG_MM_CMD_SET_REGMD ( mmShrdPrm.regMode );   /* restore former registration mode*/
#else
        /* restore former registration mode */
        psaMM_SetRegMode ( mmShrdPrm.regMode );
#endif

        R_AT( RAT_CME, mmEntStat.entOwn )
          ( AT_CMD_NRG, cmhMM_GetNregCMEStat());

        /* log result */
        cmh_logRslt ( mmEntStat.entOwn, RAT_CME, AT_CMD_NRG, -1, 
                               BS_SPEED_NotPresent ,cmhMM_GetNregCMEStat() );
        break;

      case( NRG_RGMD_Both ):

          /* Implements Measure 87 */
          cmhMM_Registration_AutoMode( AT_CMD_NRG );

        break;
      }
      break;

    case( NRG_SVMD_Limited ):

      /* pending registration request */
      if( regReqPnd EQ TRUE )
      {
        mmShrdPrm.regMode = MODE_AUTO;

#if defined (GPRS) AND defined (DTI)
        (void)psaG_MM_CMD_REG ();  /* register to network */
#else
        (void)psaMM_Registrate (); /* register to network */
#endif

        regReqPnd = FALSE;
        break;
      }

      if( mmShrdPrm.regStat EQ RS_LMTD_SRV )
      {
        mmEntStat.curCmd = AT_CMD_NONE;

        cmhMM_Ntfy_NtwRegistrationStatus(cmhMM_GetNregCREGStat());

        R_AT( RAT_OK, mmEntStat.entOwn )
          ( AT_CMD_NRG );

        /* log result */
        cmh_logRslt ( mmEntStat.entOwn, RAT_OK, AT_CMD_NRG, -1, 
                               BS_SPEED_NotPresent, CME_ERR_NotPresent );
      }
      else
      {
        mmEntStat.curCmd = AT_CMD_NONE;

        cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_Denied);

#if defined (GPRS) AND defined (DTI)
        psaG_MM_CMD_SET_REGMD ( mmShrdPrm.regMode );   /* restore former registration mode*/
#else
        /* restore former registration mode */
        psaMM_SetRegMode ( mmShrdPrm.regMode );
#endif

        R_AT( RAT_CME, mmEntStat.entOwn )
          ( AT_CMD_NRG, cmhMM_GetNregCMEStat() );

        /* log result */
        cmh_logRslt ( mmEntStat.entOwn, RAT_CME, AT_CMD_NRG, -1, 
                               BS_SPEED_NotPresent ,cmhMM_GetNregCMEStat() );
      }
      break;

    case( NRG_SVMD_NoSrv   ):

      mmEntStat.curCmd = AT_CMD_NONE;

      cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_NoSearch);

      R_AT( RAT_OK, mmEntStat.entOwn )
        ( AT_CMD_NRG );

      /* log result */
      cmh_logRslt ( mmEntStat.entOwn, RAT_OK, AT_CMD_NRG, -1, 
                             BS_SPEED_NotPresent,CME_ERR_NotPresent );
      break;

    case( NRG_SVMD_SetRegModeOnly ):

      cmhMM_Ntfy_NtwRegistrationStatus(cmhMM_GetNregCREGStat());
      break;

    }
    break;
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMR                      |
|                            ROUTINE : cmhMM_NetworkLst             |
+-------------------------------------------------------------------+

  PURPOSE : List of networks available

*/

GLOBAL SHORT cmhMM_NetworkLst ( void )
{
  /* 
   * We need a structure which has to fit into one not too big partition.
   * sizeof (T_OP_NAME) = 36 (35 characters plus one alignment byte)
   * with MAX_LONG_OPER_LEN = 26 and MAX_SHRT_OPER_LEN = 9
   * => sizeof (T_OP_NAME) * MAX_OPER = 432
   * => This fits into one of the 432er partitions of which we have some.
   */
  typedef struct
  {
    char longNamBuf[MAX_LONG_OPER_LEN]; /* buffer long name */
    char shrtNamBuf[MAX_SHRT_OPER_LEN]; /* buffer short name*/
  } T_OP_NAME;

  T_OP_NAME    * op_name = NULL;      /* Holds operator names */
  T_OPER_ENTRY   plmnDesc;            /* PLMN description */
  T_ACI_COPS_OPDESC * operLst = NULL; /* operator list T_ACI_COPS_LST */
  SHORT          lstIdx;              /* holds list idx */
  SHORT          mcc, mnc;            /* holds converted mnc and mcc */
  BOOL           found;               /* Result of cmhMM_FindXXX() */
  T_ACI_AT_CMD   bufCmd;

  TRACE_FUNCTION ("cmhMM_NetworkLst()");

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( mmEntStat.curCmd )
  {
    case( AT_CMD_COPS ):
    case (AT_CMD_P_COPS):

    ACI_MALLOC (op_name, sizeof (T_OP_NAME) * MAX_OPER);
    ACI_MALLOC (operLst, sizeof (T_ACI_COPS_OPDESC) * MAX_OPER);

    /*
     *---------------------------------------------------------------
     * for every entry of the PLMN list
     *---------------------------------------------------------------
     */
    for( lstIdx = 0; lstIdx < MAX_OPER AND lstIdx < MAX_PLMN_ID; lstIdx++ )
    {
      /*
       *-------------------------------------------------------------
       * if last entry of PLMN list, close oper list and return
       *-------------------------------------------------------------
       */
      if( mmShrdPrm.PLMNLst[lstIdx].v_plmn EQ INVLD_PLMN )
      {
        operLst[lstIdx].status    = COPS_STAT_NotPresent;
        operLst[lstIdx].longOper  = NULL;
        operLst[lstIdx].shortOper = NULL;
        operLst[lstIdx].numOper[0]= '\0';
        break;
      }

      /*
       *-------------------------------------------------------------
       * find PLMN description
       *-------------------------------------------------------------
       */
      cmhMM_CnvrtPLMN2INT( mmShrdPrm.PLMNLst[lstIdx].mcc,
                           mmShrdPrm.PLMNLst[lstIdx].mnc,
                           &mcc, &mnc );

      found = cmhMM_FindPLMN (&plmnDesc, mcc, mnc, mmShrdPrm.LACLst[lstIdx], TRUE);

      if (!found)  /* no description found */
      {
        operLst[lstIdx].longOper  =  "";
        operLst[lstIdx].shortOper =  ""; 
      }
      else
      {
        /* Copy strings into dynamic buffer so that they don't get lost */
        memcpy (op_name[lstIdx].longNamBuf, plmnDesc.longName, MAX_LONG_OPER_LEN);
        memcpy (op_name[lstIdx].shrtNamBuf, plmnDesc.shrtName, MAX_SHRT_OPER_LEN);

        /* EONS PLMN name conding scheme and source flag */
        operLst[lstIdx].pnn          = plmnDesc.pnn;
        operLst[lstIdx].longOper     = op_name[lstIdx].longNamBuf;
        operLst[lstIdx].long_len     = plmnDesc.long_len;
        operLst[lstIdx].long_ext_dcs = plmnDesc.long_ext_dcs;
        operLst[lstIdx].shortOper    = op_name[lstIdx].shrtNamBuf;
        operLst[lstIdx].shrt_len     = plmnDesc.shrt_len;
        operLst[lstIdx].shrt_ext_dcs = plmnDesc.shrt_ext_dcs;
      }

      /*
       *-------------------------------------------------------------
       * build numeric representation
       *-------------------------------------------------------------
       */
      /* Implements Measure#32: Row 987 & 986 */
      cmhMM_mcc_mnc_print(&(operLst[lstIdx].numOper[0]), mcc, mnc);

      /*
       *-------------------------------------------------------------
       * set PLMN status
       *-------------------------------------------------------------
       */
      if (mmShrdPrm.PLMNLst[lstIdx].v_plmn EQ mmShrdPrm.usedPLMN.v_plmn AND
          memcmp (mmShrdPrm.PLMNLst[lstIdx].mcc,
                  mmShrdPrm.usedPLMN.mcc, 
                  SIZE_MCC) EQ 0 AND
          memcmp (mmShrdPrm.PLMNLst[lstIdx].mnc, 
                  mmShrdPrm.usedPLMN.mnc, SIZE_MNC) EQ 0)
      {
        operLst[lstIdx].status = COPS_STAT_Current;
      }
      else
      {
        switch( mmShrdPrm.FRBLst[lstIdx] )
        {
          case( FORB_PLMN_NOT_INCLUDED ):
            operLst[lstIdx].status = COPS_STAT_Available;
            break;
          case( 1 /*FORB_PLMN_INCLUDED*/ ):
            operLst[lstIdx].status = COPS_STAT_Forbidden;
            break;
          default:
            operLst[lstIdx].status = COPS_STAT_Unknown;
            break;
        }
      }
    }

#if defined (GPRS) AND defined (DTI)
    psaG_MM_CMD_SET_REGMD ( mmShrdPrm.regMode );   /* restore former registration mode*/
#else
    /* restore former registration mode */
    psaMM_SetRegMode ( mmShrdPrm.regMode );
#endif
    bufCmd = mmEntStat.curCmd;
    mmEntStat.curCmd = AT_CMD_NONE;
    if(bufCmd EQ AT_CMD_COPS)
    {
      R_AT( RAT_COPS, mmEntStat.entOwn )
      ( --lstIdx, &operLst[0]);
    }
    else
    {
      R_AT( RAT_P_COPS, mmEntStat.entOwn )
      ( --lstIdx, &operLst[0]);
    }
    R_AT( RAT_OK, mmEntStat.entOwn )
      ( mmEntStat.curCmd );

    /* log result */
    cmh_logRslt ( mmEntStat.entOwn, RAT_OK, mmEntStat.curCmd , -1, 
                           BS_SPEED_NotPresent,CME_ERR_NotPresent );

    ACI_MFREE (op_name);
    ACI_MFREE (operLst);

    break;
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMR                      |
|                            ROUTINE : cmhMM_SelNetwork             |
+-------------------------------------------------------------------+

  PURPOSE : Select a network, cause for failure from MM is passed as parameter
                   to report appropriate error.
*/

GLOBAL SHORT cmhMM_SelNetwork ( USHORT cause )
{
  TRACE_FUNCTION ("cmhMM_SelNetwork()");

  switch( mmEntStat.curCmd )
  {
    case( AT_CMD_COPS ):

#if defined (GPRS) AND defined (DTI)
      psaG_MM_CMD_SET_REGMD ( mmShrdPrm.regMode );   /* restore former registration mode*/
#else
      /* restore former registration mode */
      psaMM_SetRegMode ( mmShrdPrm.regMode );
#endif

      switch(cause)
      {
        case MMCS_SIM_REMOVED:
           /* If the SIM is not inserted or if the SIM requires a PIN, more meaningful error 
            * codes are returned*/
           if ( (cmhSIM_GetSIMError(mmEntStat.entOwn, mmEntStat.curCmd)) NEQ AT_CMPL)
           {
             R_AT( RAT_CME, mmEntStat.entOwn ) ( mmEntStat.curCmd , CME_ERR_NotPresent);
           }
           break;

        case MMCS_PLMN_NOT_IDLE_MODE:
          R_AT( RAT_CME, mmEntStat.entOwn ) ( mmEntStat.curCmd , CME_ERR_AbortedByNetwork );
          break;

        default:
          R_AT( RAT_CME, mmEntStat.entOwn ) ( mmEntStat.curCmd , CME_ERR_NotPresent);
          break;
      }

      /* log result */
      cmh_logRslt ( mmEntStat.entOwn, RAT_CME, mmEntStat.curCmd , -1, 
                             BS_SPEED_NotPresent, CME_ERR_NotPresent );

      mmEntStat.curCmd = AT_CMD_NONE;

      break;
  }

  return 0;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMR                      |
|                            ROUTINE : cmhMM_CipheringInd           |
+-------------------------------------------------------------------+

  PURPOSE : ciphering indication received

*/

GLOBAL SHORT cmhMM_CipheringInd ( UBYTE ciph )
{
  SHORT idx;                  /* holds index counter */

  TRACE_FUNCTION ("cmhMM_CipheringInd()");

  if (simShrdPrm.ciSIMEnabled NEQ TRUE)
  {
    return 1;
  }

  for (idx = 0; idx < CMD_SRC_MAX; idx++)
  {
     /*
     *-----------------------------------------------------------------
     * new message indication
     *-----------------------------------------------------------------
     */
#ifndef GPRS
  #define CIPH_NA 2
#endif
     R_AT(RAT_P_CPRI,(T_ACI_CMD_SRC)idx) ( ciph, CIPH_NA );
  }
  return 0;
}

/* Implements Measure 87 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhMM_Registration_AutoMode
+------------------------------------------------------------------------------
|  Purpose     : This Function will try to register into network in 
|                automatic mode.
|
|  Parameters  : void
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void cmhMM_Registration_AutoMode( T_ACI_AT_CMD at_cmd_id )
{
  TRACE_FUNCTION ("cmhMM_Registration_AutoMode()");

  if ( mmShrdPrm.regModeAutoBack )
  {
    /*
     * Tried register to network in manual mode. Attempt failed.
     * Now trying to register into network in automatic mode.
     */

    mmShrdPrm.regModeBeforeAbort = mmShrdPrm.regMode; 
    mmShrdPrm.regMode = MODE_AUTO;
    mmShrdPrm.regModeAutoBack = FALSE;

#if defined (GPRS) AND defined (DTI)
    if( psaG_MM_CMD_REG ( ) EQ 0 )
#else
    if( psaMM_Registrate () EQ 0 )
#endif
    {
      return;
    }

  }
  else
  {

    cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_Denied);

    /* If fallback to automatic mode is allowed, fall back */
    if ( at_cmd_id EQ AT_CMD_NRG )
    {
      if (mmShrdPrm.regModeAutoBack)
      {
        mmShrdPrm.regModeBeforeAbort = mmShrdPrm.regMode;
        mmShrdPrm.regModeAutoBack = FALSE;
        mmShrdPrm.regMode = MODE_AUTO;
      }
    }

#if defined (GPRS) AND defined (DTI)
    /* restore former registration mode*/
    psaG_MM_CMD_SET_REGMD ( mmShrdPrm.regMode );
#else
    /* restore former registration mode or set new one */
    psaMM_SetRegMode ( mmShrdPrm.regMode );
#endif
  }

  mmEntStat.curCmd = AT_CMD_NONE;

  R_AT( RAT_CME, mmEntStat.entOwn )
        ( at_cmd_id, cmhMM_GetNregCMEStat());

  /* log result */
  cmh_logRslt ( mmEntStat.entOwn, RAT_CME, mmEntStat.curCmd, -1, 
                                     BS_SPEED_NotPresent,cmhMM_GetNregCMEStat() ); 

}


/*==== EOF ========================================================*/

