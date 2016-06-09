/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SIMR
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
|             the subscriber identity module.
+-----------------------------------------------------------------------------
*/

#ifndef CMH_SIMR_C
#define CMH_SIMR_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_mem.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "pcm.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifndef _SIMULATION_
#include "../../services/ffs/ffs.h"
#include "ffs_coat.h"
#endif

#ifdef DTI
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#endif /* DTI */

#include "phb.h"
#include "ksd.h"
#include "aci.h"
#include "psa.h"
#include "psa_sim.h"
#include "psa_cc.h"
#include "psa_sat.h"
#include "psa_mm.h"
#include "cmh.h"
#include "cmh_sim.h"
#include "cmh_mm.h"
#include "cmh_phb.h"

#include "aci_ext_pers.h"
#include "aci_slock.h"
#include "psa_sms.h"

#ifdef SIM_PERS
#include "general.h" // included for compilation error UNIT8 in sec_drv.h
#include "sec_drv.h"
#endif

#ifdef GPRS
  #include "gaci.h"
  #include "gaci_cmh.h"
  #include "psa_gmm.h"
  #include "cmh_gmm.h"
#endif

#include "p_mmcm.h"
#include "m_cc.h"

#include "aoc.h"

#ifdef DTI
#include "wap_aci.h"
#include "psa_tcpip.h"
#include "psa_l2r.h"
#endif

#ifdef SIM_PERS
EXTERN  T_ACI_SIM_CONFIG aci_slock_sim_config;     /* SIM configuration, initialised by a T_SIM_MMI_INSERT_IND */
EXTERN T_SEC_DRV_CONFIGURATION *cfg_data ;
#endif

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
LOCAL void cmhSIM_ProcessEvents ( T_ACI_AT_CMD at_cmd_id );
LOCAL void cmhSIM_Compare_CNUMMsisdnIdx ( void );
LOCAL void cmhSIM_SndError( T_ACI_CMD_SRC ownBuf,
                            T_ACI_AT_CMD cmdBuf,
                            T_ACI_CME_ERR cme_err );

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhSIM_SIMSync               |
+-------------------------------------------------------------------+

  PURPOSE : SIM data synchronized

*/

GLOBAL void cmhSIM_SIMSync ( void )
{
  TRACE_FUNCTION ("cmhSIM_SIMSync()");

  /* process event */
  switch( simEntStat.curCmd )
  {
    case( AT_CMD_CFUN ):
      if (mmEntStat.curCmd NEQ AT_CMD_CFUN)     /* Has MM already deregistered? */
      {
        R_AT( RAT_OK, simEntStat.entOwn )
          ( AT_CMD_CFUN );

        /* log result */
        cmh_logRslt ( simEntStat.entOwn, RAT_OK, AT_CMD_CFUN,
                          -1, BS_SPEED_NotPresent,CME_ERR_NotPresent );
      }
      /*lint -fallthrough*/
    case( AT_CMD_COPS ):
    case( AT_CMD_P_COPS ):

      simEntStat.curCmd = AT_CMD_NONE;
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : clck_fdn_accepted()          |
+-------------------------------------------------------------------+

  PURPOSE : PIN2 status allows CLCK changing the status concerning FDN.

*/

LOCAL void clck_fdn_accepted ( void )
{
  switch( simShrdPrm.setPrm[simEntStat.entOwn].actProc )
  {
    case( SIM_FDN_ENABLE ):
      if(  simShrdPrm.crdFun EQ SIM_ADN_ENABLED )
      {
        simShrdPrm.crdFun = SIM_FDN_ENABLED;
      }
      if(  simShrdPrm.crdFun EQ SIM_ADN_BDN_ENABLED )
      {
        simShrdPrm.crdFun = SIM_FDN_BDN_ENABLED;
      }

      pb_switch_adn_fdn( FDN_ENABLE, simShrdPrm.classFDN ); /* inform PHB */
      break;

    case( SIM_FDN_DISABLE ):
      if(  simShrdPrm.crdFun EQ SIM_FDN_ENABLED )
      {
        simShrdPrm.crdFun = SIM_ADN_ENABLED;
      }
      if(  simShrdPrm.crdFun EQ SIM_FDN_BDN_ENABLED )
      {
        simShrdPrm.crdFun = SIM_ADN_BDN_ENABLED;
      }
      pb_switch_adn_fdn( FDN_DISABLE, simShrdPrm.classFDN ); /* inform PHB */
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhsim_simactivated_clck     |
+-------------------------------------------------------------------+
  PURPOSE : SIM activated after AT+CLCK
*/

LOCAL void cmhsim_simactivated_clck( void )
{
  T_ACI_CME_ERR err = CME_ERR_Unknown;
   
  TRACE_FUNCTION ("cmhsim_simactivated_clck()");

  simEntStat.curCmd = AT_CMD_NONE;

  switch( simShrdPrm.rslt )
  {
    case( SIM_NO_ERROR ):
      clck_fdn_accepted();

      R_AT( RAT_OK, simEntStat.entOwn )
        ( AT_CMD_CLCK );
      cmh_logRslt ( simEntStat.entOwn, RAT_OK, AT_CMD_CLCK, -1,
	  	                     BS_SPEED_NotPresent,CME_ERR_NotPresent );
      return;

    case( SIM_CAUSE_PIN2_EXPECT ):
      TRACE_EVENT_P1("simShrdPrm.pn2Stat: %d", simShrdPrm.pn2Stat);

      if ( simShrdPrm.pn2Stat EQ PS_PIN2 )
      {
        if ( ( simShrdPrm.setPrm[simEntStat.entOwn].curPIN[0] NEQ 0x00 )           AND
             ( simShrdPrm.setPrm[simEntStat.entOwn].curPIN[0] NEQ NOT_PRESENT_CHAR ) )
        {
          /* if PIN2 is required and a pin is given, then check pin with PIN2 */
          simEntStat.curCmd = AT_CMD_CLCK; /* further processing */

          simShrdPrm.setPrm[simEntStat.entOwn].PINType = PHASE_2_PIN_2;
          psaSIM_VerifyPIN();  /* verify PIN */
          return;
        }
        else
        {
          /* if PIN2 is required and no pin is given, then send cme_err with PIN2
             required message code */
          if (  simShrdPrm.PINStat EQ PS_RDY )
          {
            simShrdPrm.PINStat = PS_PIN2;
          }
          err = CME_ERR_SimPin2Req;
        }
      }
      else
      {
        err = CME_ERR_WrongPasswd;
      }
      break;

   /*case( SIM_CAUSE_PIN1_EXPECT ):
   case( SIM_CAUSE_PUK1_EXPECT ):*/

/* Should be CME_ERR_SimPuk2Req, and it is done 
                         in the default case 
 case( SIM_CAUSE_PIN2_BLOCKED):
   case( SIM_CAUSE_PUK2_EXPECT ):
      err = CME_ERR_WrongPasswd;
      break;*/

   default:
      err = cmhSIM_GetCmeFromSim( simShrdPrm.rslt );
      break;
  }

  R_AT( RAT_CME, simEntStat.entOwn )
        ( AT_CMD_CLCK, err );
  cmh_logRslt ( simEntStat.entOwn, RAT_CME, AT_CMD_CLCK,
                -1, BS_SPEED_NotPresent, err );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhsim_simactivated_start    |
+-------------------------------------------------------------------+
  PURPOSE : SIM activated after AT+CFUN, +CPIN, +CIMI and %NRG
*/
LOCAL void simactivated_cpinresult( T_ACI_CPIN_RSLT cpin_result )
{
  R_AT( RAT_CPIN, simEntStat.entOwn )
    ( cpin_result );
  R_AT( RAT_OK, simEntStat.entOwn )
    ( AT_CMD_CPIN );
  cmh_logRslt ( simEntStat.entOwn, RAT_OK, AT_CMD_CPIN,
                -1, BS_SPEED_NotPresent,CME_ERR_NotPresent );
}

LOCAL void simactivated_errorresult( T_ACI_CME_ERR error_result, T_ACI_AT_CMD cmdBuf )
{
  R_AT( RAT_CME, simEntStat.entOwn )
  ( cmdBuf, error_result );

  if( cmdBuf NEQ AT_CMD_CIMI )
  {
    cmh_logRslt ( simEntStat.entOwn, RAT_CME, cmdBuf,
                  -1, BS_SPEED_NotPresent, error_result );
  }
}

LOCAL void cmhsim_simactivated_start( void )
{
  T_ACI_AT_CMD cmdBuf = simEntStat.curCmd;  /* buffers current command */

  TRACE_FUNCTION ("cmhsim_simactivated_start()");

  simEntStat.curCmd = AT_CMD_NONE;

  switch( simShrdPrm.SIMStat )
  {
    case( SS_INV   ):
      simactivated_errorresult( CME_ERR_SimWrong, cmdBuf );
      return;

    case( SS_URCHB ):
      simactivated_errorresult( CME_ERR_SimNotIns, cmdBuf );
      return;

    case( NO_VLD_SS ):
      simactivated_errorresult( CME_ERR_NotPresent, cmdBuf );
      return;

    case( SS_OK   ):
      switch( simShrdPrm.PINStat )
      {
        case( PS_PIN1 ):
          if( cmdBuf EQ AT_CMD_CPIN )
          {
            simactivated_cpinresult( CPIN_RSLT_SimPinReq );
          }
          else
          {
            simactivated_errorresult( CME_ERR_SimPinReq, cmdBuf );
          }
          break;

        case( PS_PIN2 ):
          if( cmdBuf EQ AT_CMD_CPIN )
          {
            simactivated_cpinresult( CPIN_RSLT_SimPin2Req );
          }
          else
          {
            simactivated_errorresult( CME_ERR_SimPin2Req, cmdBuf );
          }
          break;

        case( PS_RDY ):
          /* wait for SIM insert indication to complete command */
          simEntStat.curCmd = cmdBuf;
          break;
      }
      break;

    case( SS_BLKD  ):

      switch( simShrdPrm.PINStat )
      {
        case( PS_PUK1 ):
          if( cmdBuf EQ AT_CMD_CPIN )
          {
            simactivated_cpinresult( CPIN_RSLT_SimPukReq );
          }
          else
          {
            simactivated_errorresult( CME_ERR_SimPukReq, cmdBuf );
          }
          break;

          case( PS_PUK2 ):
          if( cmdBuf EQ AT_CMD_CPIN )
          {
            simactivated_cpinresult( CPIN_RSLT_SimPuk2Req );
          }
          else
          {
            simactivated_errorresult( CME_ERR_SimPuk2Req, cmdBuf );
          }
          break;
      }
      break;

  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhSIM_SIMActivated          |
+-------------------------------------------------------------------+

  PURPOSE : SIM activated

*/

GLOBAL void cmhSIM_SIMActivated ( void )
{
  T_ACI_CMD_SRC idx;

  TRACE_FUNCTION ("cmhSIM_SIMActivated()");

  /* check for command context */
  switch( simEntStat.curCmd )
  {
    case( AT_CMD_CFUN ):
    case( AT_CMD_CPIN ):
    case( AT_CMD_CIMI ):
    case( AT_CMD_NRG  ):
    case( AT_CMD_SIMRST):
      /* process event for +CFUN, +CPIN, +CIMI and for %NRG command */
      cmhsim_simactivated_start( );
      break;

    case( AT_CMD_CLCK ):
      /* process event for +CLCK command */
      cmhsim_simactivated_clck( );
      break;

    case( AT_CMD_NONE ):
      /* process event spontaneous insertion */
      if (simShrdPrm.rslt EQ SIM_NO_ERROR)
      {
        ; /* do nothing right now since the %SIMINS: -1 must be
           syncronized with SIM_MMI_INSERT_IND which will come later! */
      }
      else
      {
        for( idx = CMD_SRC_LCL; idx < CMD_SRC_MAX; idx++ )
        {
          R_AT( RAT_SIMINS, idx )(cmhSIM_GetCmeFromSim (simShrdPrm.rslt));
        }
      }
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhSIM_SIMInserted           |
+-------------------------------------------------------------------+

  PURPOSE : SIM inserted

*/

GLOBAL void cmhSIM_SIMInserted ( void )
{
  USHORT cmdBuf;                 /* buffers current command */
  CHAR  imsiBuf[MAX_IMSI_LEN+1];  /* buffer IMSI representation +1 for '\0'*/
  T_ACI_CME_ERR err_code = CME_ERR_NotPresent; /* code holding the correct error code calculated */
  #ifdef SIM_PERS
  T_ACI_CPIN_RSLT code = CPIN_RSLT_NotPresent;
  #endif
  BOOL return_rat_ok = 1;  /* return ok? */

  TRACE_FUNCTION ("cmhSIM_SIMInserted()");

  /* Check if the SIM is functional and the IMSI value is valid*/
  if ( simShrdPrm.crdFun       EQ SIM_NO_OPERATION OR
       simShrdPrm.imsi.c_field EQ 0 )
  {
    simShrdPrm.SIMStat = SS_INV;
    err_code = CME_ERR_SimWrong;
  }
 cmdBuf = simEntStat.curCmd;
  switch( cmdBuf )
  {
    case( AT_CMD_CFUN ):
    case( AT_CMD_CPIN ):
    case( AT_CMD_PVRF ):
    case( AT_CMD_SIMRST):
    case( KSD_CMD_UBLK):
      /*
        *----------------------------------------------------------------
        * process event for +CFUN, +CPIN, %PVRF commands and for the 
          SIM reset condition
        *----------------------------------------------------------------
        */

      simEntStat.curCmd = AT_CMD_NONE;
#ifdef SIM_PERS
      if((aci_slock_sim_config.sim_type EQ SIM_TYPEAPPROVAL) AND AciSLockShrd.blocked)
      {
        simShrdPrm.SIMStat = SS_INV;
   err_code = CME_ERR_SimWrong;
      }
     if(cfg_data EQ NULL)
     {
       simShrdPrm.SIMStat = SS_INV;
       ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_NoMEPD );
       err_code = CME_ERR_Unknown;
       aci_slock_send_RAT(cmdBuf,err_code); 
       simShrdPrm.PINQuery = 0;
       AciSLockShrd.cpin_query = SEND_CPIN_REQ_CODE_NONE; 
       return;
     }
#endif

      if( err_code EQ CME_ERR_SimWrong )
      {
        R_AT( RAT_CME, simEntStat.entOwn ) ((T_ACI_AT_CMD) cmdBuf, err_code );
        cmh_logRslt ( simEntStat.entOwn, RAT_CME, (T_ACI_AT_CMD)cmdBuf, -1, BS_SPEED_NotPresent, err_code );
        simShrdPrm.PINQuery = 0;
#ifdef SIM_PERS
        AciSLockShrd.cpin_query = SEND_CPIN_REQ_CODE_NONE;
#endif
        return;
      }

#ifdef SIM_PERS
    /* check for personalisation locking. If SIM was detected as not correct, do signal AT_CPIN */

      if ( AciSLockShrd.blocked)
      {
         return_rat_ok = 0;
         if(AciSLockShrd.cpin_query EQ SEND_CPIN_REQ_CODE_NONE)
         {
           /* @GBR: Alternativly CME_ERR_SimWrong might be returned, but this way is telling the MMI mor specific, what went wrong. */

           aci_set_cme_error_code_and_logRslt( cmdBuf );
         }
        else
        {  
          if(AciSLockShrd.cpin_query EQ SEND_CPIN_REQ_CODE_RAT )
          {
              aci_set_cpin_code(AciSLockShrd.current_lock,&code);
              R_AT( RAT_CPIN, simEntStat.entOwn )
                ( code );
          }
           AciSLockShrd.cpin_query  = SEND_CPIN_REQ_CODE_NONE;	
        }
      }

#endif
      if ( (cmdBuf EQ AT_CMD_CPIN) AND (simShrdPrm.PINQuery EQ 1) ) /* Case when PIN is disabled and CFUN=0 */
      {
        R_AT( RAT_CPIN, simEntStat.entOwn )
          ( CPIN_RSLT_SimReady );
        simShrdPrm.PINQuery = 0;
      }

#if defined (GPRS) AND defined (DTI)
      gprs_sim_inserted();
#endif  /* GPRS */

      if (return_rat_ok EQ 1) /* @gbr: only send ok, if everhing went okay. The +CFUN might not be ready here, so don't send something! */
      {

      R_AT( RAT_OK, simEntStat.entOwn ) ((T_ACI_AT_CMD) cmdBuf );
      cmh_logRslt ( simEntStat.entOwn, RAT_OK,(T_ACI_AT_CMD) cmdBuf, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
#ifdef SIM_PERS
      AciSLockShrd.cpin_query  = SEND_CPIN_REQ_CODE_NONE;
#endif
     }
      break;

    case( AT_CMD_CIMI ):
   /*
    *----------------------------------------------------------------
    * process event for +CIMI command
    *----------------------------------------------------------------
    */
      

      if( err_code EQ CME_ERR_SimWrong )
      {
        R_AT( RAT_CME, simEntStat.entOwn ) ( AT_CMD_CIMI, err_code );
        cmh_logRslt ( simEntStat.entOwn, RAT_CME, AT_CMD_CIMI, -1, BS_SPEED_NotPresent, err_code );
        return;
      }

      R_AT( RAT_CIMI, simEntStat.entOwn )
          ( psaSIM_cnvrtIMSI2ASCII( imsiBuf ));

      simEntStat.curCmd = AT_CMD_NONE;

      R_AT( RAT_OK, simEntStat.entOwn )
        ( AT_CMD_CIMI );
      break;

    case( AT_CMD_NRG ):
   /*
    *----------------------------------------------------------------
    * process event for %NRG
    *----------------------------------------------------------------
    */
      if( err_code EQ CME_ERR_SimWrong )
      {
        simEntStat.curCmd = AT_CMD_NONE;
        mmEntStat.curCmd  = AT_CMD_NONE;
        R_AT( RAT_CME, simEntStat.entOwn ) ( (T_ACI_AT_CMD)cmdBuf, err_code );
        cmh_logRslt ( simEntStat.entOwn, RAT_CME, (T_ACI_AT_CMD)cmdBuf, -1, BS_SPEED_NotPresent, err_code );
        return;
      }

      switch( cmhPrm[simShrdPrm.owner].mmCmdPrm.NRGregMode )
      {
        case( NRG_RGMD_Auto ):

          mmShrdPrm.regMode = MODE_AUTO;

          simEntStat.curCmd = AT_CMD_NONE;
          mmEntStat.curCmd  = AT_CMD_NRG;
          mmShrdPrm.owner = simShrdPrm.owner;
          mmEntStat.entOwn  = (T_ACI_CMD_SRC)simShrdPrm.owner;

#if defined (GPRS) AND defined (DTI)
          (void)psaG_MM_CMD_REG ();  /* register to network */
#else
          (void)psaMM_Registrate (); /* register to network */
#endif

          cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_Search);
          break;

        case( NRG_RGMD_Manual ):

          mmShrdPrm.regMode = MODE_MAN;

          simEntStat.curCmd = AT_CMD_NONE;
          mmEntStat.curCmd  = AT_CMD_NRG;
          mmShrdPrm.owner = simShrdPrm.owner;
          mmEntStat.entOwn  = (T_ACI_CMD_SRC) simShrdPrm.owner;

#if defined (GPRS) && defined (DTI) 
          psaG_MM_CMD_NET_SEL ( ); /* register to network */
#else
          psaMM_NetSel (); /* register to network */
#endif

          cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_Search);
          break;
      }
      break;

    case( AT_CMD_NONE ):
      {
        T_ACI_CMD_SRC idx;
//TISH, patch for ASTec31853
//start
/*modified for Roaming issue*/
	TRACE_EVENT("reset mm reg stat");
    	mmShrdPrm.regStat = NO_VLD_RS;
//end		
        /* process event spontaneous insertion */
        for( idx = CMD_SRC_LCL; idx < CMD_SRC_MAX; idx++ )
        {
          R_AT( RAT_SIMINS, idx )( err_code );
        }
      }
      break;

  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhSIM_SIMRemoved            |
+-------------------------------------------------------------------+

  PURPOSE : SIM removed

*/

GLOBAL void cmhSIM_SIMRemoved ( void )
{
  T_ACI_AT_CMD cmdBuf;             /* buffers command */
  T_ACI_CMD_SRC idx;
  T_ACI_CME_ERR cme_error;  /* CME error code */
  T_ACI_SIMREM_TYPE srt;    /* type of SIM remove */

  TRACE_FUNCTION ("cmhSIM_SIMRemoved()");

  if (simEntStat.curCmd NEQ AT_CMD_NONE)
  {
    cmdBuf = simEntStat.curCmd;
    simEntStat.curCmd = AT_CMD_NONE;

    if (simShrdPrm.rslt EQ SIM_NO_ERROR)
      cme_error = CME_ERR_SimBusy;
    else
      cme_error = cmhSIM_GetCmeFromSim (simShrdPrm.rslt);

    R_AT( RAT_CME, simEntStat.entOwn )
          ( cmdBuf, cme_error );
    cmh_logRslt ( simEntStat.entOwn, RAT_CME, cmdBuf,
                  -1, BS_SPEED_NotPresent, cme_error );
  }
  EfPLMNselStat = EF_STAT_UNKNWN;
  /*
   *----------------------------------------------------------------
   * send unsolicited result
   *----------------------------------------------------------------
   */
  TRACE_EVENT_P1("Result of SIM remove indication: %x", simShrdPrm.rslt);
  switch (simShrdPrm.rslt)
  {
    case SIM_NO_ERROR:
      /* In this case the AT command state is set to SIM RESET */
      /* NO, don't do this!
         This line would prevent a %SIMIND: 11 and instead leads to
         "+CME ERROR: SIM PIN required" without any command context which is wrong */
      /* simEntStat.curCmd = AT_CMD_SIMRST; */
      srt = SIMREM_RESET;
      break;
    case SIM_CAUSE_DRV_TEMPFAIL:
      srt = SIMREM_RETRY;
      break;
    default:
      srt = SIMREM_FAILURE;
      break;
  }

  for( idx = CMD_SRC_LCL; idx < CMD_SRC_MAX; idx++ )
  {
    R_AT( RAT_SIMREM, idx )(srt);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhSIM_PINVerified           |
+-------------------------------------------------------------------+

  PURPOSE : SIM verified

*/
LOCAL void pin_verified_clck (USHORT sim_result)
{
  TRACE_FUNCTION ("pin_verified_clck()");

  switch(sim_result)
  {
    case( SIM_NO_ERROR ):
      /* now do it ! */

      /* Check if we want to do FDN Lock or ALS Lock */
      switch (simShrdPrm.setPrm[simShrdPrm.owner].actProc)
      {
        case SIM_FDN_ENABLE:
        case SIM_FDN_DISABLE:
          /* FDN Lock */
          if( psaSIM_ActivateSIM() < 0 )   /* activate SIM card */
          {
            simEntStat.curCmd = AT_CMD_NONE;

            R_AT( RAT_CME, simEntStat.entOwn )
                  ( AT_CMD_CLCK, CME_ERR_Unknown );
            cmh_logRslt ( simEntStat.entOwn, RAT_CME, AT_CMD_CLCK,
                          -1, BS_SPEED_NotPresent, CME_ERR_Unknown );
          }
          break;

        case SIM_ALS_LOCK:
        case SIM_ALS_UNLOCK:
          /* ALS Lock to PIN2 */
          if (simShrdPrm.setPrm[simShrdPrm.owner].actProc EQ SIM_ALS_LOCK)
          {
            ALSlock = cmhPrm[simShrdPrm.owner].ccCmdPrm.ALSmode;
            /* pull all sources to the current ALSmode, otherwise a lock won't make sense */
            {
              int i;
              for (i=0; i<OWN_SRC_MAX; i++)
                cmhPrm[i].ccCmdPrm.ALSmode = ALSlock;
                /* and lock them all */
            }
  #ifndef _SIMULATION_
            /* write Lock to FFS */
            /* Implements Measure#32: Row 1071 */
            FFS_fwrite(gsm_com_alslock_path, &ALSlock, sizeof(ALSlock));
  #endif
          }
          else
          /* unlock ALS */
          {
            ALSlock = ALS_MOD_NOTPRESENT;
  #ifndef _SIMULATION_
            /* remove lock from FFS */
            /* Implements Measure#32: Row 1071 */
            FFS_remove(gsm_com_alslock_path);
  #endif
          }

          simEntStat.curCmd = AT_CMD_NONE;

          R_AT( RAT_OK, simEntStat.entOwn )
            ( AT_CMD_CLCK );
          cmh_logRslt ( simEntStat.entOwn, RAT_OK, AT_CMD_CLCK, -1, BS_SPEED_NotPresent,CME_ERR_NotPresent);
          break;

      }
      break; /* SIM_NO_ERROR */

    case( SIM_CAUSE_PIN1_EXPECT ):
    case( SIM_CAUSE_PIN2_EXPECT ):
    case( SIM_CAUSE_PIN1_BLOCKED ):
    case( SIM_CAUSE_PUK1_EXPECT ):
    case( SIM_CAUSE_PIN2_BLOCKED ):
    case( SIM_CAUSE_PUK2_EXPECT ):
      simEntStat.curCmd = AT_CMD_NONE;

      R_AT( RAT_CME, simEntStat.entOwn )
            ( AT_CMD_CLCK, cmhSIM_GetCmeFromSim ( sim_result ) /*CME_ERR_WrongPasswd*/ );
      cmh_logRslt ( simEntStat.entOwn, RAT_CME, AT_CMD_CLCK,
                    -1,BS_SPEED_NotPresent, CME_ERR_WrongPasswd );
      break;

    default:
      simEntStat.curCmd = AT_CMD_NONE;
      R_AT( RAT_CME, simEntStat.entOwn )
            ( AT_CMD_CLCK, CME_ERR_SimFail );
      cmh_logRslt ( simEntStat.entOwn, RAT_CME, AT_CMD_CLCK,
                    -1, BS_SPEED_NotPresent, CME_ERR_SimFail );
      break;
  }
}

GLOBAL void cmhSIM_PINVerified ( void )
{
  T_ACI_AT_CMD cmdBuf;         /* buffers command */
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameters */

  TRACE_FUNCTION ("cmhSIM_PINVerified()");

  pPHBCmdPrm = &cmhPrm[simShrdPrm.owner].phbCmdPrm;

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( simEntStat.curCmd )
  {
    case( AT_CMD_CLCK ):
     /* process event for +CLCK command */
      pin_verified_clck( simShrdPrm.rslt );
      return;

    case( AT_CMD_CPIN ):
    case( AT_CMD_PVRF ):
     /*
      *----------------------------------------------------------------
      * process event for +CPIN and %PVRF command
      *----------------------------------------------------------------
      */
      switch( simShrdPrm.rslt )
      {
        case( SIM_NO_ERROR ):

          /* if verified PIN was PIN 2 or PIN 1 was already entered */
          if( simShrdPrm.setPrm[simEntStat.entOwn].PINType EQ
              PHASE_2_PIN_2 OR
              simShrdPrm.crdPhs NEQ 0xFF )
          {
            cmdBuf = simEntStat.curCmd;
            simEntStat.curCmd = AT_CMD_NONE;

            R_AT( RAT_OK, simEntStat.entOwn )
            ( cmdBuf );
            cmh_logRslt ( simEntStat.entOwn, RAT_OK, cmdBuf, -1, BS_SPEED_NotPresent,CME_ERR_NotPresent);
          }
          /* otherwise wait for SIM insert indication to complete command  */
          break;

        case( SIM_CAUSE_PIN1_EXPECT ):
        case( SIM_CAUSE_PIN2_EXPECT ):
        case( SIM_CAUSE_PUK1_EXPECT ):
        case( SIM_CAUSE_PUK2_EXPECT ): 
          cmdBuf = simEntStat.curCmd;
          simEntStat.curCmd = AT_CMD_NONE;

          R_AT( RAT_CME, simEntStat.entOwn )
                ( cmdBuf, CME_ERR_WrongPasswd );
          cmh_logRslt ( simEntStat.entOwn, RAT_CME, cmdBuf,
                        -1, BS_SPEED_NotPresent, CME_ERR_WrongPasswd );
          break;
        case( SIM_CAUSE_PIN1_BLOCKED):       
          cmdBuf = simEntStat.curCmd;
          simEntStat.curCmd = AT_CMD_NONE;

          R_AT( RAT_CME, simEntStat.entOwn )
                ( cmdBuf, CME_ERR_SimPukReq );
          cmh_logRslt ( simEntStat.entOwn, RAT_CME, cmdBuf,
                        -1, BS_SPEED_NotPresent, CME_ERR_SimPukReq );
          break;
        case( SIM_CAUSE_PIN2_BLOCKED):
          cmdBuf = simEntStat.curCmd;
          simEntStat.curCmd = AT_CMD_NONE;

          R_AT( RAT_CME, simEntStat.entOwn )
                ( cmdBuf, CME_ERR_SimPuk2Req );
          cmh_logRslt ( simEntStat.entOwn, RAT_CME, cmdBuf,
                        -1,BS_SPEED_NotPresent, CME_ERR_SimPuk2Req );
          break;
        default:
          cmdBuf = simEntStat.curCmd;
          simEntStat.curCmd = AT_CMD_NONE;
          R_AT( RAT_CME, simEntStat.entOwn )
                ( cmdBuf, CME_ERR_SimFail );
          break;
      }
      break;

      /* Implements Measure 106 */
    case( AT_CMD_CAMM ):
    case( AT_CMD_CPUC ):
    case( AT_CMD_CACM ):
      cmhSIM_ProcessEvents( simEntStat.curCmd );
      break;
     /*----------------------------------------------------------* 
      * The below code is for CPBW SIM verification response     *
      *----------------------------------------------------------*/

    case( AT_CMD_CPBS ):
     /*
      *----------------------------------------------------------------
      * process event for +CPBW command
      *----------------------------------------------------------------
      */
      switch( simShrdPrm.rslt )
      {
        case( SIM_NO_ERROR ):

        /*-----------------------------------------------* 
         * if verified PIN was PIN 2 was already entered *
         *-----------------------------------------------*/

         cmdBuf = simEntStat.curCmd;
         simEntStat.curCmd = AT_CMD_NONE;

         pPHBCmdPrm->cmhStor = (T_ACI_PB_STOR)pPHBCmdPrm->temp_cmhStor;
         pPHBCmdPrm->phbStor = pPHBCmdPrm->temp_phbStor;

         R_AT( RAT_OK, simEntStat.entOwn )
              ( cmdBuf );

         cmh_logRslt ( simEntStat.entOwn, RAT_OK, cmdBuf, -1, BS_SPEED_NotPresent,CME_ERR_NotPresent );         

        /*---------------------------------------------------------------*
         * otherwise wait for SIM insert indication to complete command  *
         *---------------------------------------------------------------*/

         break;

        case( SIM_CAUSE_PUK2_EXPECT ):
        case( SIM_CAUSE_PIN2_BLOCKED):
        case( SIM_CAUSE_PIN2_EXPECT ):
          
         cmdBuf = simEntStat.curCmd;
         simEntStat.curCmd = AT_CMD_NONE;

         R_AT( RAT_CME, simEntStat.entOwn )
                ( cmdBuf, CME_ERR_WrongPasswd );
         cmh_logRslt ( simEntStat.entOwn, RAT_CME, cmdBuf,
                       -1, BS_SPEED_NotPresent, CME_ERR_WrongPasswd );
         break;

        default:
         cmdBuf = simEntStat.curCmd;
         simEntStat.curCmd = AT_CMD_NONE;
         R_AT( RAT_CME, simEntStat.entOwn )
               ( cmdBuf, CME_ERR_SimFail );
         break;
      }
      break;

    default:

      simEntStat.curCmd = AT_CMD_NONE;
      R_AT( RAT_CME, simEntStat.entOwn )
        ( AT_CMD_CPIN, CME_ERR_Unknown );
      cmh_logRslt ( simEntStat.entOwn, RAT_CME, AT_CMD_CPIN,
                    -1,BS_SPEED_NotPresent, CME_ERR_Unknown );
     break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhSIM_PINChanged            |
+-------------------------------------------------------------------+

  PURPOSE : SIM changed

*/

GLOBAL void cmhSIM_PINChanged ( void )
{
  T_ACI_CME_ERR err;
  USHORT         cmdBuf = simEntStat.curCmd;

  TRACE_FUNCTION ("cmhSIM_PINChanged()");

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( cmdBuf )
  {
    case( AT_CMD_CPWD ):
    /*lint -e{408} */ 
    case( KSD_CMD_PWD ):  /* KSD_CMD_* extends AT_CMD_* */
     /*
      *----------------------------------------------------------------
      * process event for +CPWD and KSD PWD command
      *----------------------------------------------------------------
      */
      switch( simShrdPrm.rslt )
      {
        case( SIM_NO_ERROR ):

          simEntStat.curCmd = AT_CMD_NONE;

          R_AT( RAT_OK, simEntStat.entOwn )
            ((T_ACI_AT_CMD) cmdBuf );
          cmh_logRslt ( simEntStat.entOwn, RAT_OK, (T_ACI_AT_CMD)cmdBuf, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
          return;

/* why not use a meaningful CME error code?
        case( SIM_CAUSE_PUK1_EXPECT ):
        case( SIM_CAUSE_PUK2_EXPECT ): */
        case( SIM_CAUSE_PIN1_EXPECT ):
        case( SIM_CAUSE_PIN2_EXPECT ):
          err = CME_ERR_WrongPasswd;
          break;

        default:
          err = cmhSIM_GetCmeFromSim ( simShrdPrm.rslt );
          break;
      }

      simEntStat.curCmd = AT_CMD_NONE;

      R_AT( RAT_CME, simEntStat.entOwn )
            ( (T_ACI_AT_CMD)cmdBuf, err );
      cmh_logRslt ( simEntStat.entOwn, RAT_CME, (T_ACI_AT_CMD)cmdBuf,
                    -1, BS_SPEED_NotPresent, err );
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhSIM_CardUnblocked         |
+-------------------------------------------------------------------+

  PURPOSE : Card unblocked

*/

GLOBAL void cmhSIM_CardUnblocked ( void )
{
  USHORT cmdBuf =simEntStat.curCmd ;
  T_ACI_CME_ERR err;

  TRACE_FUNCTION ("cmhSIM_CardUnblocked()");

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( cmdBuf )
  {
    case( AT_CMD_CPIN ):
    case( AT_CMD_PVRF ):
    /*lint -e{408} */ 
    case( KSD_CMD_UBLK ):
     /*
      *----------------------------------------------------------------
      * process event for +CPIN, %PVRF and KSD UBLK command
      *----------------------------------------------------------------
      */
      switch( simShrdPrm.rslt )
      {
        case( SIM_NO_ERROR ):

          if(simShrdPrm.crdPhs NEQ 0xFF)/* If SIM insert indication is already received*/
          {
            simEntStat.curCmd = AT_CMD_NONE;

            R_AT( RAT_OK, simEntStat.entOwn )
              ( (T_ACI_AT_CMD)cmdBuf );
            cmh_logRslt ( simEntStat.entOwn, RAT_OK,(T_ACI_AT_CMD) cmdBuf, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
          }/* else wait till SIM insert indication is received*/
          return;

/* why not use a meaningful CME error code?
        case( SIM_CAUSE_PUK1_EXPECT ):
        case( SIM_CAUSE_PUK2_EXPECT ): */
        case( SIM_CAUSE_PIN1_EXPECT ):
        case( SIM_CAUSE_PIN2_EXPECT ):
          err = CME_ERR_WrongPasswd;
          break;

        default:
          err = cmhSIM_GetCmeFromSim ( simShrdPrm.rslt );
          break;
      }

       simEntStat.curCmd = AT_CMD_NONE;

      R_AT( RAT_CME, simEntStat.entOwn )
            ( (T_ACI_AT_CMD)cmdBuf, err );
      cmh_logRslt ( simEntStat.entOwn, RAT_CME,(T_ACI_AT_CMD) cmdBuf,
                    -1, BS_SPEED_NotPresent, err );
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhSIM_SIMResponseData       |
+-------------------------------------------------------------------+

  PURPOSE : SIM response data

*/

GLOBAL void cmhSIM_SIMResponseData( T_SIM_TRNS_RSP_PRM* rsp )
{
  T_ACI_AT_CMD cmdBuf;            /* buffers command       */
  T_ACI_CMD_SRC ownBuf;    /* buffers current owner */
  UBYTE *rspData;

  TRACE_FUNCTION ("cmhSIM_SIMResponseData()");

  cmdBuf = simEntStat.curCmd;
  simEntStat.curCmd = AT_CMD_NONE;

  ownBuf = simEntStat.entOwn;
  simShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
  simEntStat.entOwn = CMD_SRC_NONE;

  /*
   *-------------------------------------------------------------------
   * check for command context
   *-------------------------------------------------------------------
   */
  switch( cmdBuf )
  {
    case( AT_CMD_CRSM ):
      if (simShrdPrm.rslt NEQ SIM_NO_ERROR       AND 
          GET_CAUSE_DEFBY(simShrdPrm.rslt) EQ DEFBY_CONDAT ) 
      {
     /*
      *----------------------------------------------------------------
      * error event for +CRSM
      *----------------------------------------------------------------
      */
        R_AT( RAT_CME, ownBuf )
          (
            cmdBuf,
            cmhSIM_GetCmeFromSim ( simShrdPrm.rslt )
          );
      }
      else
      {
     /*
      *----------------------------------------------------------------
      * process event for +CRSM
      *----------------------------------------------------------------
      */
        R_AT( RAT_CRSM, ownBuf )
          ( rsp->sw1, rsp->sw2, rsp->rspLen, ((rsp->rspLen)?rsp->rsp:NULL));

        R_AT( RAT_OK, ownBuf )
          ( cmdBuf );
      }
      break;

    case( AT_CMD_CSIM ):
     /*
      *----------------------------------------------------------------
      * error event for +CSIM
      *----------------------------------------------------------------
      */
      if (simShrdPrm.rslt NEQ SIM_NO_ERROR) 
      {
        R_AT( RAT_CME, ownBuf )
          (
            cmdBuf,
            cmhSIM_GetCmeFromSim ( simShrdPrm.rslt )
          );

        return;
      }
     /*
      *----------------------------------------------------------------
      * process event for +CSIM
      *----------------------------------------------------------------
      */
      ACI_MALLOC(rspData, MAX_SIM_CMD+2);  /* alloc 2 byte more for sw1 and sw2 */
      memcpy (rspData, rsp->rsp, rsp->rspLen);
      rspData[rsp->rspLen++] = rsp->sw1;
      rspData[rsp->rspLen++] = rsp->sw2;

      R_AT( RAT_CSIM, ownBuf )
        ( rsp->rspLen, rspData);

      ACI_MFREE(rspData);

      R_AT( RAT_OK, ownBuf )
        ( cmdBuf );
      break;
    
    default:
      TRACE_EVENT("wrong command context (+CRSM or +CSIM expected)");
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMR                 |
|                                 ROUTINE : cmhSIM_CnfMsisdn         |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            EF MSISDN from SIM.
*/
GLOBAL void cmhSIM_CnfMsisdn ( SHORT table_id )
{
  T_SIM_CMD_PRM* pSIMCmdPrm; /* points to SIM command parameters  */
  T_ACI_AT_CMD   cmdBuf;     /* buffers current command           */
  T_ACI_CMD_SRC  ownBuf;     /* buffers current owner             */

  UBYTE*  ptr;               /* pointer to decoded data           */
  UBYTE   alphaLen;          /* max. length of alpha identifier   */
  UBYTE*  pExchData;         /* points to exchange data buffer    */
  UBYTE   dataLen;           /* holds length of data */

  TRACE_FUNCTION ("cmhSIM_CnfMsisdn()");

  pSIMCmdPrm = &cmhPrm[simEntStat.entOwn].simCmdPrm;
  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  pExchData  = simShrdPrm.atb[table_id].exchData;
  dataLen    = simShrdPrm.atb[table_id].dataLen;


  switch ( simShrdPrm.atb[table_id].errCode )
  {
    case ( SIM_NO_ERROR ):
    {
      /*
       *-------------------------------------------------------------
       * check basic consistency of read data
       *-------------------------------------------------------------
       */
      if ( pExchData EQ NULL                   OR
           dataLen   <  ACI_MIN_SIZE_EF_MSISDN    )
      {
        /* Implements Measure 7, 17, 24, 35 */
        cmhSIM_SndError( ownBuf, cmdBuf, CME_ERR_Unknown );
      }
      else
      {
        ptr      = pExchData;
        alphaLen = dataLen - ACI_MIN_SIZE_EF_MSISDN;

        /*
         *-----------------------------------------------------------
         * process parameter <lenEfMsisdn>
         *-----------------------------------------------------------
         */
        CNUMLenEfMsisdn = dataLen;

        /*
         *-----------------------------------------------------------
         * process parameter <cnumMaxRec>
         *-----------------------------------------------------------
         */
        if ( pSIMCmdPrm -> CNUMActRec EQ 1 )
          CNUMMaxRec = simShrdPrm.atb[table_id].recMax;

        /*
         *-----------------------------------------------------------
         * process parameter <alpha>
         *-----------------------------------------------------------
         */
        cmhPHB_getTagNt ( ptr,
                          alphaLen,
                          CNUMMsisdn[CNUMMsisdnIdx].alpha,
                          MAX_ALPHA_LEN );

        ptr += alphaLen;

        /*
         *-----------------------------------------------------------
         * process parameter <number>
         *-----------------------------------------------------------
         */
        cmhPHB_getAdrStr ( CNUMMsisdn[CNUMMsisdnIdx].number,
                           MAX_PHB_NUM_LEN - 1,
                           ptr + 2,
                           *ptr );

        /*
         *-----------------------------------------------------------
         * validate entry of MSISDN list
         *-----------------------------------------------------------
         */
        /* VO patch 14.03.01
         * move validate entry to after decoding alpha and number
         * and set the vldFlag only when the alpha or the number exists. */
        if ( strlen(CNUMMsisdn[CNUMMsisdnIdx].number) OR
             strlen(CNUMMsisdn[CNUMMsisdnIdx].alpha)     )
        {
          CNUMMsisdn[CNUMMsisdnIdx].vldFlag = TRUE;
          pSIMCmdPrm -> CNUMOutput++;
        }

        /*
         *-----------------------------------------------------------
         * process parameter <type>
         *-----------------------------------------------------------
         */
        cmhPHB_toaDmrg ( *( ptr + 1 ),
                         &CNUMMsisdn[CNUMMsisdnIdx].type );

        ptr += 12;

        /*
         *-----------------------------------------------------------
         * invalidate parameter <speed>, <service>, <itc>
         *-----------------------------------------------------------
         */
        CNUMMsisdn[CNUMMsisdnIdx].speed   = BS_SPEED_NotPresent;
        CNUMMsisdn[CNUMMsisdnIdx].service = CNUM_SERV_NotPresent;
        CNUMMsisdn[CNUMMsisdnIdx].itc     = CNUM_ITC_NotPresent;

        if ( *ptr NEQ 0xFF )
        {
          /*
           *---------------------------------------------------------
           * bearer capability EF present
           *---------------------------------------------------------
           */
          if ( cmhSIM_ReqCcp ( ownBuf, *ptr++ ) NEQ AT_EXCT )
          {
            /* Implements Measure 7, 17, 24, 35 */
            cmhSIM_SndError( ownBuf, cmdBuf, CME_ERR_Unknown );
          }
        }
        else
        {
          /* Implements Measure 200 */

          if ( CNUMMsisdn[CNUMMsisdnIdx].vldFlag EQ FALSE )
          {
            CNUMMsisdnIdx--;
          }

          cmhSIM_Compare_CNUMMsisdnIdx();
        }
      }

      break;
    }

    default:
    {
      /* Implements Measure 7, 17, 24, 35 */
      cmhSIM_SndError( ownBuf, cmdBuf,
                   cmhSIM_GetCmeFromSim (simShrdPrm.atb[table_id].errCode) );
      break;
    }

  }

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMR           |
| STATE   : code                        ROUTINE : cmhSIM_CnfCcp      |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            EF CCP from SIM.
*/
GLOBAL void cmhSIM_CnfCcp ( SHORT table_id )
{
  T_ACI_AT_CMD   cmdBuf;      /* buffers current command          */
  T_ACI_CMD_SRC  ownBuf;      /* buffers current owner            */

  T_U_CALL_CONF* callCnf;     /* call confirm message             */
  T_M_CC_bearer_cap*  bearCap;     /* bearer capability                */
  UBYTE*         pExchData;   /* points to exchange data buffer   */
  UBYTE          dataLen;     /* holds length of data */

  TRACE_FUNCTION ("cmhSIM_CnfCcp()");

  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  dataLen    = simShrdPrm.atb[table_id].dataLen;
  pExchData  = simShrdPrm.atb[table_id].exchData;

  switch ( simShrdPrm.atb[table_id].errCode )
  {
    case ( SIM_NO_ERROR ):
    {
      /*
       *-------------------------------------------------------------
       * check basic consistency of read data
       *-------------------------------------------------------------
       */
      if ( pExchData EQ   NULL            OR
           dataLen   NEQ  ACI_SIZE_EF_CCP    )
      {
        /* Implements Measure 7, 17, 24, 35 */
        cmhSIM_SndError( ownBuf, cmdBuf, CME_ERR_Unknown );
      }
      else
      {
        /*
         *-----------------------------------------------------------
         * extracting the relevant data from bearer capability IE
         *-----------------------------------------------------------
         */
        UBYTE bearCapLen = pExchData[0];

        if (bearCapLen <= 13 )   /* do not decode empty or invalid entries,
                                    see 11.11/10.5.4.1 max 14 bytes */
        {
          /*lint -e415 -e416 -e669 (Waring: creation of out of bound ptr or data overrun)*/
          PALLOC_MSG ( msg, MMCM_DATA_REQ, U_CALL_CONF);

          msg -> sdu.l_buf  = bearCapLen * 0x08;
          msg -> sdu.o_buf  = 0x08;
          msg -> sdu.buf[0] = 0x83;
          msg -> sdu.buf[1] = U_CALL_CONF;

          memcpy ( &msg -> sdu.buf[2],
                   pExchData,
                   bearCapLen+1);
          /*lint +e415 +e416 +e669 (Waring: creation of out of bound ptr or data overrun)*/
          /*
           *-----------------------------------------------------------
           * use of CCD for decoding of bearer capability
           *-----------------------------------------------------------
           */
          CCD_START;

          memset (_decodedMsg, 0, sizeof (_decodedMsg));

          if ( ccd_decodeMsg ( CCDENT_CC,
                               UPLINK,
                               (T_MSGBUF *) &msg->sdu,
                               (UBYTE    *) _decodedMsg,
                               NOT_PRESENT_8BIT) EQ ccdOK
               AND

               _decodedMsg[0] EQ U_CALL_CONF )
          {
            callCnf = ( T_U_CALL_CONF * )&_decodedMsg[0];

            if ( callCnf -> v_bearer_cap )
            {
              bearCap = &callCnf -> bearer_cap;

              /*
               *-------------------------------------------------------
               * process parameter <speed> of MSISDN
               *-------------------------------------------------------
               */
              if ( bearCap -> v_user_rate )
              {
                CNUMMsisdn[CNUMMsisdnIdx].speed =
                           cmhSIM_GetUserRate ( bearCap -> user_rate );
              }

              /*
               *-------------------------------------------------------
               * process parameter <itc> and <service> of MSISDN
               *-------------------------------------------------------
               */
              if ( bearCap -> v_trans_cap )
              {
                CNUMMsisdn[CNUMMsisdnIdx].itc =
                                cmhSIM_GetItc ( bearCap -> trans_cap );
              }

              if ( bearCap -> v_user_inf_l2_prot                AND
                   bearCap -> user_inf_l2_prot   EQ M_CC_L2_X25      AND
                   bearCap -> v_sync_async                      AND
                   bearCap -> sync_async         EQ M_CC_SYNCHRONOUS     )
              {
                CNUMMsisdn[CNUMMsisdnIdx].service =
                                                 CNUM_SERV_PacketSynch;
              }
              else if
                 (   bearCap -> v_sig_access_prot
                     AND
                   ( bearCap -> sig_access_prot EQ M_CC_SIAP_X28_INDIV_NUI OR
                     bearCap -> sig_access_prot EQ M_CC_SIAP_X28_UNIV_NUI  OR
                     bearCap -> sig_access_prot EQ M_CC_SIAP_X28_NON_DEDIC
                   )

                     AND
                     bearCap -> v_sync_async
                     AND
                     bearCap -> sync_async        EQ M_CC_ASYNCHRONOUS
                 )
              {
                CNUMMsisdn[CNUMMsisdnIdx].service =
                                                   CNUM_SERV_PadAsynch;
              }
              else if ( bearCap -> v_sync_async )
              {
                CNUMMsisdn[CNUMMsisdnIdx].service =
                        cmhSIM_GetSrvFromSync ( bearCap -> sync_async );

              }
              else if ( bearCap -> v_trans_cap )
              {
                CNUMMsisdn[CNUMMsisdnIdx].service =
                         cmhSIM_GetSrvFromItc ( bearCap -> trans_cap );
              }
            }
          }
          CCD_END;

          PFREE ( msg );
        }

        /*
         *-----------------------------------------------------------
         * tmpActRec is used to prevent conflicts with global
         * variable pSIMCmdPrm -> CNUMActRec
         *-----------------------------------------------------------
         */
        /* Implements Measure 200 */
        cmhSIM_Compare_CNUMMsisdnIdx();
      }

      break;
    }

    default:
    {
      /* Implements Measure 7, 17, 24, 35 */
      cmhSIM_SndError( ownBuf, cmdBuf, 
                       cmhSIM_GetCmeFromSim(simShrdPrm.atb[table_id].errCode));
      break;
    }

  }

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMR           |
| STATE   : code                        ROUTINE : cmhSIM_RdCnfPlmnSel|
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            EF PLMN SEL from SIM.
*/
GLOBAL void cmhSIM_RdCnfPlmnSel ( SHORT table_id )
{
  T_SIM_CMD_PRM * pSIMCmdPrm; /* points to SIM command parameters  */
  T_ACI_AT_CMD    cmdBuf;     /* buffers current command           */
  T_ACI_CMD_SRC   ownBuf;     /* buffers current owner             */
  T_ACI_CPOL_LST  CPOLPlmnSelLst;   /* PLMN SEL list               */
  T_ACI_RETURN    ret;
  SHORT           lastIdx;    /* holds last index */
  SHORT           usdNtry;    /* holds number of used entries */

  TRACE_FUNCTION ("cmhSIM_RdCnfPlmnSel()");

  pSIMCmdPrm = &cmhPrm[simEntStat.entOwn].simCmdPrm;
  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;


  switch ( simShrdPrm.atb[table_id].errCode )
  {
    case ( SIM_NO_ERROR ):
    {
      /*
       *-------------------------------------------------------------
       * check basic consistency of read data
       *-------------------------------------------------------------
       */
      if ( simShrdPrm.atb[table_id].exchData EQ NULL  OR
           simShrdPrm.atb[table_id].dataLen <  ACI_MIN_SIZE_EF_PLMN_SEL)
      {
        pSIMCmdPrm->CPOLact = CPOL_ACT_None;
        /* Implements Measure 7, 17, 24, 35 */
        cmhSIM_SndError( ownBuf, cmdBuf, CME_ERR_Unknown );
        break;
      }

      EfPLMNselStat = EF_STAT_READ;
      CPOLSimEfDataLen = simShrdPrm.atb[table_id].dataLen;

      if( pSIMCmdPrm->CPOLmode EQ CPOL_MOD_CompactList )
      {
        cmhSIM_CmpctPlmnSel( simShrdPrm.atb[table_id].dataLen,
                             simShrdPrm.atb[table_id].exchData );
      }

      /*
       *-----------------------------------------------------------
       * what is to do ?
       *-----------------------------------------------------------
       */
      switch( pSIMCmdPrm->CPOLact )
      {
        /*
         *-----------------------------------------------------------
         * read out PLMN SEL list
         *-----------------------------------------------------------
         */
        case( CPOL_ACT_Read ):

          /* fill in buffer */
          lastIdx = cmhSIM_FillPlmnSelList(pSIMCmdPrm->CPOLidx,
                                           pSIMCmdPrm->CPOLfrmt,
                                           &CPOLPlmnSelLst[0],
                                           simShrdPrm.atb[table_id].dataLen,
                                           (UBYTE *)simShrdPrm.atb[table_id].exchData);
          /* notify about PLMN SEL list */
          simEntStat.curCmd = AT_CMD_NONE;
          simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
          simEntStat.entOwn =  CMD_SRC_NONE;

          R_AT( RAT_CPOL, ownBuf ) ( ACI_NumParmNotPresent,
                                     lastIdx,
                                     &CPOLPlmnSelLst[0],
                                     ACI_NumParmNotPresent );

          R_AT( RAT_OK, ownBuf )( AT_CMD_CPOL );
          break;

        /*
         *-----------------------------------------------------------
         * write PLMN SEL list
         *-----------------------------------------------------------
         */
        case( CPOL_ACT_Write ):

          if( pSIMCmdPrm->CPOLidx EQ NOT_PRESENT_8BIT )

            ret = cmhSIM_FndEmptyPlmnSel( simEntStat.entOwn,
                                          pSIMCmdPrm->CPOLplmn );
          else

            ret = cmhSIM_UpdPlmnSel( simEntStat.entOwn,
                                     pSIMCmdPrm->CPOLidx,
                                     pSIMCmdPrm->CPOLplmn,
                                     pSIMCmdPrm->CPOLmode);
          if( ret EQ AT_FAIL )
          {
            pSIMCmdPrm->CPOLact = CPOL_ACT_None;
            /* Implements Measure 7, 17, 24, 35 */

            /* T_ACI_CME_ERR is used for TypeCast as the 
             * ACI_ERR_DESC_NR() will retrun ULONG and the function expects
             * T_ACI_CME_ERR and there will no major harm as well.
             */
            cmhSIM_SndError( simEntStat.entOwn, AT_CMD_CPOL,
                             (T_ACI_CME_ERR) ACI_ERR_DESC_NR( aciErrDesc ));
          }
          break;

        /*
         *-----------------------------------------------------------
         * delete PLMN SEL entry
         *-----------------------------------------------------------
         */
        case( CPOL_ACT_Delete ):

          if( pSIMCmdPrm->CPOLidx2 EQ NOT_PRESENT_8BIT )

            ret = cmhSIM_DelPlmnSel( simEntStat.entOwn,
                                     pSIMCmdPrm->CPOLidx,
                                     pSIMCmdPrm->CPOLmode );
          else

            ret = cmhSIM_ChgPlmnSel( simEntStat.entOwn,
                                     pSIMCmdPrm->CPOLidx,
                                     pSIMCmdPrm->CPOLidx2 );

          if( ret EQ AT_FAIL )
          {
            pSIMCmdPrm->CPOLact = CPOL_ACT_None;
            /* Implements Measure 7, 17, 24, 35 */

            /* T_ACI_CME_ERR is used for TypeCast as the ACI_ERR_DESC_NR() 
             * will retrun ULONG and the function expects T_ACI_CME_ERR 
             * and there will no major harm as well
             */
            cmhSIM_SndError( simEntStat.entOwn, AT_CMD_CPOL, 
                             (T_ACI_CME_ERR) ACI_ERR_DESC_NR( aciErrDesc ));
          }
          break;

        /*
         *-----------------------------------------------------------
         * test PLMN SEL entry number
         *-----------------------------------------------------------
         */
        case( CPOL_ACT_Test ):

          /* notify about PLMN SEL entry number */
          simEntStat.curCmd = AT_CMD_NONE;
          simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
          simEntStat.entOwn =  CMD_SRC_NONE;

          lastIdx = CPOLSimEfDataLen / ACI_LEN_PLMN_SEL_NTRY;
          usdNtry = cmhSIM_UsdPlmnSelNtry( CPOLSimEfDataLen,
                                           (UBYTE *)simShrdPrm.atb[table_id].exchData );

          R_AT( RAT_CPOL, ownBuf ) ( ACI_NumParmNotPresent,
                                     lastIdx,
                                     NULL,
                                     usdNtry );

          R_AT( RAT_OK, ownBuf )( AT_CMD_CPOL );
          break;
      }
      break;
    }

    case ( SIM_CAUSE_UNKN_FILE_ID ):
      EfPLMNselStat = EF_STAT_NOT_AVAIL;
      /*lint -fallthrough*/
      /*lint -fallthrough*/
    default:
    {
      pSIMCmdPrm->CPOLact = CPOL_ACT_None;
      /* Implements Measure 7, 17, 24, 35 */
      cmhSIM_SndError( ownBuf, cmdBuf, 
                       cmhSIM_GetCmeFromSim(simShrdPrm.atb[table_id].errCode));
      break;
    }

  }

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMR           |
| STATE   : code                        ROUTINE : cmhSIM_WrCnfPlmnSel|
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of writing
            EF PLMN SEL to SIM.
*/
GLOBAL void cmhSIM_WrCnfPlmnSel ( SHORT table_id )
{
  T_SIM_CMD_PRM * pSIMCmdPrm; /* points to SIM command parameters  */
  UBYTE ownBuf;

  TRACE_FUNCTION ("cmhSIM_WrCnfPlmnSel()");

  pSIMCmdPrm = &cmhPrm[simEntStat.entOwn].simCmdPrm;
  ownBuf = simEntStat.entOwn;

  switch ( simShrdPrm.atb[table_id].errCode )
  {
    /* successful PLMN SEL list update */
    case ( SIM_NO_ERROR ):
    {
      pSIMCmdPrm->CPOLact = CPOL_ACT_None;
      simEntStat.curCmd = AT_CMD_NONE;
      simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
      simEntStat.entOwn = CMD_SRC_NONE;

      R_AT( RAT_OK, (T_ACI_CMD_SRC)ownBuf )( AT_CMD_CPOL );
      break;
    }
    case SIM_CAUSE_ACCESS_PROHIBIT:
    {
      pSIMCmdPrm->CPOLact = CPOL_ACT_None;
      /* Implements Measure 7, 17, 24, 35 */
      cmhSIM_SndError((T_ACI_CMD_SRC)ownBuf, AT_CMD_CPOL, CME_ERR_OpNotAllow );
      break;
    }

    /* unsuccessful PLMN SEL list update */
    default:
    {
      pSIMCmdPrm->CPOLact = CPOL_ACT_None;
      /* Implements Measure 7, 17, 24, 35 */
      cmhSIM_SndError((T_ACI_CMD_SRC)ownBuf, AT_CMD_CPOL, 
                       cmhSIM_GetCmeFromSim(simShrdPrm.atb[table_id].errCode));
      break;
    }
  }

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMR           |
| STATE   : code                        ROUTINE : cmhSIM_RdCnfLangELP|
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            from the SIM (EF ELP).
*/
GLOBAL void cmhSIM_RdCnfLangELP ( SHORT table_id )
{
  T_SIM_CMD_PRM *pSIMCmdPrm; /* points to SIM command parameters */
  T_PHB_CMD_PRM *pPHBCmdPrm; /* points to PHB command parameter  */

  T_ACI_AT_CMD  cmdBuf;     /* buffers current command           */
  T_ACI_CMD_SRC ownBuf;     /* buffers current owner             */
  CHAR          Clang_buffer[3]={0};/* Current language          */
  T_ACI_LAN_SUP clng;
  EF_CLNG       lng;
  BOOL          Suplng = FALSE;
  T_ACI_LAN_SUP LngPCMsupLst[MAX_LAN];
  SHORT         lastIdx;
  T_ACI_RETURN    ret ;

  TRACE_FUNCTION ("cmhSIM_RdCnfLangELP()");

  pSIMCmdPrm = &cmhPrm[simEntStat.entOwn].simCmdPrm;
  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  pPHBCmdPrm = &cmhPrm[ownBuf].phbCmdPrm;
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
  simEntStat.curCmd = AT_CMD_NONE;

  clng.str = Clang_buffer;

  switch ( simShrdPrm.atb[table_id].errCode )
  {
    case ( SIM_NO_ERROR ):
     /*
      *-------------------------------------------------------------
      * check basic consistency of read data
      *-------------------------------------------------------------
      */
      if ( simShrdPrm.atb[table_id].exchData EQ NULL  OR
           simShrdPrm.atb[table_id].dataLen <  ACI_MIN_SIZE_EF_LAN)

      {
        pSIMCmdPrm->CLANact = CLAN_ACT_None;
        simShrdPrm.owner    = (T_OWN)CMD_SRC_NONE;
        simEntStat.entOwn   = CMD_SRC_NONE;

        R_AT( RAT_CME, ownBuf ) ( cmdBuf, CME_ERR_Unknown );
        return;
      }

      memcpy(clng.str, (UBYTE *)simShrdPrm.atb[table_id].exchData, CLAN_CODE_LEN);
      ret=getSupLangFromPCM(&LngPCMsupLst[0], &lastIdx);
      if (ret EQ AT_FAIL)
      {
        return;
      }
      Suplng=checkSuppLang(&LngPCMsupLst[0],lastIdx, &clng);
      simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
      simEntStat.entOwn = CMD_SRC_NONE;

     /*
      *---------------------------------------------------------------------
      * check if the language,which was read from EF ELP is supported in PCM
      *---------------------------------------------------------------------
      */
      if(Suplng)
      {
        switch( pSIMCmdPrm->CLANact )
        {
          case( CLAN_ACT_Read ):
            R_AT( RAT_CLAN, ownBuf ) (&clng);
            R_AT( RAT_OK, ownBuf )( AT_CMD_CLAN );
            break;
          case( CLAN_ACT_Write ):
            memcpy(lng.data,clng.str ,CLAN_CODE_LEN);
            /* Implements Measure#32 Row 1072 */
            pcm_WriteFile (( UBYTE* )ef_clng_id,SIZE_EF_CLNG,( UBYTE*) &lng);

            if (pPHBCmdPrm->CLAEmode EQ CLAE_MOD_Enable)
              R_AT( RAT_CLAE, ownBuf ) (&clng);

          R_AT( RAT_OK, ownBuf )( AT_CMD_CLAN );
          break;
       }
      }
      else
     /*
      *-------------------------------------------------------------
      * read the EF LP if the language is not supported in PCM
      *-------------------------------------------------------------
      */
       /* Implements Measure 150 and 159 */
       cmhSIM_ReqLanguage_LP_or_ELP ( ownBuf, SIM_LP );
       break;

     /*
      *-------------------------------------------------------------
      * read the EF LP if the EF ELP not available
      *-------------------------------------------------------------
      */
    case ( SIM_CAUSE_UNKN_FILE_ID ):
       /* Implements Measure 150 and 159 */
       cmhSIM_ReqLanguage_LP_or_ELP ( ownBuf, SIM_LP );
      break;

    default:
      pSIMCmdPrm->CLANact = CLAN_ACT_None;
      simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
      simEntStat.entOwn = CMD_SRC_NONE;

      R_AT( RAT_CME, ownBuf) ( AT_CMD_CLAN, CME_ERR_Unknown );
      break;

  }

}


/*
+------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMR               |
| STATE   : code                        ROUTINE : cmhSIM_RdCnfLangPrfELP |
+------------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            from the SIM (EF ELP) for SAT feature LS.
*/
GLOBAL void cmhSIM_RdCnfLangPrfELP ( SHORT table_id )
{
  
  T_ACI_SAT_TERM_RESP resp_data;
  CHAR           Clang_buffer[3]={0};/* Current language          */
  T_ACI_LAN_SUP  clng;
  BOOL           Suplng = FALSE;
  T_ACI_LAN_SUP  LngPCMsupLst[MAX_LAN];
  SHORT          lastIdx;
  T_ACI_RETURN    ret ;
  

  TRACE_FUNCTION ("cmhSIM_RdCnfLangPrfELP()");
  
  psaSAT_InitTrmResp( &resp_data );
  
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  clng.str = Clang_buffer;

  switch ( simShrdPrm.atb[table_id].errCode )
  {
    case ( SIM_NO_ERROR ):
     /*
      *-------------------------------------------------------------
      * check basic consistency of read data
      *-------------------------------------------------------------
      */
      if ( simShrdPrm.atb[table_id].exchData EQ NULL  OR
           simShrdPrm.atb[table_id].dataLen <  ACI_MIN_SIZE_EF_LAN)

      {
        psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
        return;
      }

      memcpy(clng.str, (UBYTE *)simShrdPrm.atb[table_id].exchData, CLAN_CODE_LEN);
      ret=getSupLangFromPCM(&LngPCMsupLst[0], &lastIdx);
      if (ret EQ AT_FAIL)
      {
        psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
        return;
      }
      
      Suplng=checkSuppLang(&LngPCMsupLst[0],lastIdx, &clng);

      
     /*
      *---------------------------------------------------------------------
      * check if the language,which was read from EF ELP is supported in PCM
      *---------------------------------------------------------------------
      */
      if(Suplng)
      {
        /* Send Terminal resp*/
        memcpy(&resp_data.lang, (UBYTE *)simShrdPrm.atb[table_id].exchData, CLAN_CODE_LEN);
        psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );

      }
      else
      {
        /*
        *-------------------------------------------------------------
        * read the EF LP if the language is not supported in PCM
        *-------------------------------------------------------------
        */
        /* Implements Measure 119 */
        if ( cmhSIM_ReqLanguagePrf_LP_or_ELP( SIM_LP, ACI_MAX_LAN_LP_NTRY,
                                              CLANSimEfDataLP,
                                              cmhSIM_RdCnfLangPrfLP ) EQ FALSE )
        {
          psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
        }
      }
      break;

     /*
      *-------------------------------------------------------------
      * read the EF LP if the EF ELP not available
      *-------------------------------------------------------------
      */
    case ( SIM_CAUSE_UNKN_FILE_ID ):
        /* Implements Measure 119 */
        if (cmhSIM_ReqLanguagePrf_LP_or_ELP ( SIM_LP, ACI_MAX_LAN_LP_NTRY,
                                              CLANSimEfDataLP,
                                              cmhSIM_RdCnfLangPrfLP ) EQ FALSE )
      {
        psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
      }
      break;

    default:
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
      break;

  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMR           |
| STATE   : code                        ROUTINE : cmhSIM_RdCnfLangLP |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            EF LP from SIM.
*/
GLOBAL void cmhSIM_RdCnfLangLP ( SHORT table_id )
{
  T_SIM_CMD_PRM * pSIMCmdPrm; /* points to SIM command parameters  */
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameter */
  T_ACI_AT_CMD    cmdBuf;     /* buffers current command           */
  T_ACI_CMD_SRC   ownBuf;     /* buffers current owner             */
  T_ACI_LAN_SUP   clng;
  EF_CLNG lng;
  BOOL Suplng=    FALSE;
  SHORT           lastIdx;
  T_ACI_LAN_SUP LngPCMsupLst[MAX_LAN];


  TRACE_FUNCTION ("cmhSIM_RdCnfLangLP()");

  pSIMCmdPrm = &cmhPrm[simEntStat.entOwn].simCmdPrm;
  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  pPHBCmdPrm = &cmhPrm[ownBuf].phbCmdPrm;
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
  simEntStat.curCmd = AT_CMD_NONE;

  clng.lng=(T_ACI_CLAN_LNG)simShrdPrm.atb[table_id].exchData[0];

  getSupLangFromPCM(&LngPCMsupLst[0], &lastIdx);
  Suplng=checkSuppLangInLP(&LngPCMsupLst[0],lastIdx, &clng);
  
  if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR AND
       Suplng                                            )
  {
    simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
    simEntStat.entOwn = CMD_SRC_NONE;
   /*
    *-------------------------------------------------------------
    *    check basic consistency of read data
    *-------------------------------------------------------------
    */
    if ( simShrdPrm.atb[table_id].exchData EQ NULL  OR
         simShrdPrm.atb[table_id].dataLen <  ACI_LEN_LAN_LP_NTRY)

     {
        simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
        simEntStat.entOwn = CMD_SRC_NONE;
        pSIMCmdPrm->CLANact = CLAN_ACT_None;
        R_AT( RAT_CME, ownBuf ) ( cmdBuf, CME_ERR_Unknown );
        return;
      }

#ifdef SIM_TOOLKIT
      if (simShrdPrm.fuRef >= 0)
      {
        psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_SUCCESS);
      }
#endif

      /*
      *-----------------------------------------------------
      *    read the supported laguage or write it PCM
      *-----------------------------------------------------
      */
      switch( pSIMCmdPrm->CLANact )
      {
        case( CLAN_ACT_Read ):
          R_AT( RAT_CLAN, ownBuf ) (&clng);
          R_AT( RAT_OK, ownBuf )( AT_CMD_CLAN );
          break;
         case( CLAN_ACT_Write ):
           memcpy(lng.data,(UBYTE *) clng.str,CLAN_CODE_LEN);
           /* Implements Measure#32 Row 1072 */
           pcm_WriteFile (( UBYTE* )ef_clng_id,SIZE_EF_CLNG,( UBYTE*) &lng);
           if (pPHBCmdPrm->CLAEmode EQ CLAE_MOD_Enable)
           {  R_AT( RAT_CLAE, ownBuf ) (&clng);}

          R_AT( RAT_OK, ownBuf )( AT_CMD_CLAN );
          break;

       }
  }
  else
  {

#ifdef SIM_TOOLKIT
    if (simShrdPrm.fuRef >= 0)
    {
      psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_ERROR);
    }
#endif
   /*
    *---------------------------------------------------------------
    *          Preferred language is not supported in MS
    *---------------------------------------------------------------
    */
    if ( simShrdPrm.atb[table_id].errCode EQ SIM_CAUSE_UNKN_FILE_ID OR
         !Suplng                                                      )
    {
      TRACE_FUNCTION ("preferred language not supported");    

      switch( pSIMCmdPrm->CLANact )
      {
        case( CLAN_ACT_Read ):
          R_AT( RAT_CLAN, ownBuf ) (&LngPCMsupLst[0]);
          R_AT( RAT_OK, ownBuf )( AT_CMD_CLAN );
        break;
        case( CLAN_ACT_Write ):      /* Preferred Language not supported,so not change CLAN */
          R_AT( RAT_OK, ownBuf )( AT_CMD_CLAN );          
        break;
      }
    }
    else
    {
      pSIMCmdPrm->CLANact = CLAN_ACT_None;
      simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
      simEntStat.entOwn = CMD_SRC_NONE;
      R_AT( RAT_CME, ownBuf) ( AT_CMD_CLAN, CME_ERR_Unknown );
    }
  }
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMR              |
| STATE   : code                        ROUTINE : cmhSIM_RdCnfLangPrfLP |
+-----------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            EF LP from SIM for SAT feature LS.
*/
GLOBAL void cmhSIM_RdCnfLangPrfLP ( SHORT table_id )
{
  
  T_ACI_SAT_TERM_RESP resp_data;
  T_ACI_LAN_SUP   clng;
  BOOL Suplng=    FALSE;
  SHORT           lastIdx;
  T_ACI_LAN_SUP   lngPCMsupLst[MAX_LAN];
  T_ACI_RETURN    ret;

  TRACE_FUNCTION ("cmhSIM_RdCnfLangPrfLP()");

  psaSAT_InitTrmResp( &resp_data );
  
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  clng.lng=(T_ACI_CLAN_LNG)simShrdPrm.atb[table_id].exchData[0];
  ret = getSupLangFromPCM(&lngPCMsupLst[0], &lastIdx);
  if(ret EQ AT_FAIL)
  {
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return;
  }

  Suplng=checkSuppLangInLP(&lngPCMsupLst[0],lastIdx, &clng);

  if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR AND
       Suplng )
  {
   /*
    *-------------------------------------------------------------
    *    check basic consistency of read data
    *-------------------------------------------------------------
    */
    if ( simShrdPrm.atb[table_id].exchData EQ NULL  OR
         simShrdPrm.atb[table_id].dataLen <  ACI_LEN_LAN_LP_NTRY)

    {
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
      return;
    }

#ifdef SIM_TOOLKIT
    if (simShrdPrm.fuRef >= 0)
    {
      psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_SUCCESS);
    }
#endif

   /*
    *-----------------------------------------------------
    *    Send the Terminal Response
    *-----------------------------------------------------
    */
    memcpy(&resp_data.lang, clng.str, CLAN_CODE_LEN);
    psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
  }
  else
  {

#ifdef SIM_TOOLKIT
    if (simShrdPrm.fuRef >= 0)
    {
      psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_ERROR);
    }
#endif
   /*
    *---------------------------------------------------------------
    *          read default language from ME
    *---------------------------------------------------------------
    */

    if ( simShrdPrm.atb[table_id].errCode EQ SIM_CAUSE_UNKN_FILE_ID OR
         !Suplng )
    {
      TRACE_FUNCTION ("default language is selected");
      memcpy(&resp_data.lang, &lngPCMsupLst[0].str, CLAN_CODE_LEN);
      psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
            
    }
    else
    {
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    }

  }

}


/*
+--------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  : CMH_SIMR                  |
| STATE   : code                       ROUTINE : SIM_ENT_CSDconnect_dti_cb |
+--------------------------------------------------------------------------+

  PURPOSE : Callback for connection between SIM and TRA/L2R for CSD.

*/
#ifdef FF_SAT_E
#ifdef DTI
GLOBAL BOOL SIM_ENT_CSDconnect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  TRACE_FUNCTION("SIM_ENT_CSDconnect_dti_cb");

  switch( result_type)
  {
    case DTI_CONN_STATE_DISCONNECTING:
      break;
    case DTI_CONN_STATE_DISCONNECTED:
      /* wap_state is set to IPA_Deactivated in psaTCPIP_config_dispatch() */
      if (wap_state EQ UDPA_Deactivation)
      {
        wap_state = IPA_Deactivation;
        psaUDPIP_config_dispatch();
      }
      dti_cntrl_erase_entry(dti_id);
      dti_cntrl_clear_conn_parms(dti_id);
      if (simShrdPrm.sat_class_e_dti_id EQ dti_id)
      {
        simShrdPrm.sat_class_e_dti_id = DTI_DTI_ID_NOTPRESENT;
        TRACE_EVENT("sat_class_e_dti_id reset");
      }
      break;
    case DTI_CONN_STATE_CONNECTING:
      break;
    case DTI_CONN_STATE_CONNECTED:
      /* the SIM-SNDCP connection has been established, now send SIM_BIP_CONFIG
       * request to SIM */ 
      psaSIM_Bip_Config_Req();
      
      break;
    case DTI_CONN_STATE_ERROR:
      /* connection not possible: disconnect UDP, L2R or TRA */ 
      TRACE_EVENT("SIM_ENT_CSDconnect_dti_cb connection not possible: disconnect UDP,L2R or TRA");
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;
    case DTI_CONN_STATE_UNKNOWN:
    default:
      TRACE_EVENT("SIM_ENT_CSDconnect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}
#endif /* DTI */

#if defined (GPRS) AND defined (DTI)
/*
+---------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMR                  |
| STATE   : code                        ROUTINE : SIM_SNDCP_connect_dti_cb  |
+---------------------------------------------------------------------------+

  PURPOSE : Callback for connection between SIM and SNDCP.

*/
GLOBAL BOOL SIM_SNDCP_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  TRACE_FUNCTION("SIM_SNDCP_connect_dti_cb");

  switch( result_type)
  {
    case DTI_CONN_STATE_DISCONNECTING:
      break;
    case DTI_CONN_STATE_DISCONNECTED:
      dti_cntrl_erase_entry(dti_id);
      if (simShrdPrm.sat_class_e_dti_id EQ dti_id)
      {
        simShrdPrm.sat_class_e_dti_id = DTI_DTI_ID_NOTPRESENT;
        TRACE_EVENT("sat_class_e_dti_id reset");
      }
      break;
    case DTI_CONN_STATE_CONNECTING:
      break;
    case DTI_CONN_STATE_CONNECTED:
      /* the SIM-SNDCP connection has been established, now send SIM_BIP_CONFIG
       * request to SIM */ 
      psaSIM_Bip_Config_Req();
      break;
    case DTI_CONN_STATE_ERROR:
      /* connection not possible: disconnect SNDCP */
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;
    case DTI_CONN_STATE_UNKNOWN:
    default:
      TRACE_EVENT("SIM_SNDCP_connect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}


/*
+--------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  : CMH_SIMR                  |
| STATE   : code                       ROUTINE : SIM_ENT_GPRSconnect_dti_cb|
+--------------------------------------------------------------------------+

  PURPOSE : Callback for connection between SIM and UDP/SNDCP for GPRS.

*/

GLOBAL BOOL SIM_ENT_GPRSconnect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  TRACE_FUNCTION("SIM_ENT_GPRSconnect_dti_cb");

  switch( result_type)
  {
    case DTI_CONN_STATE_DISCONNECTING:
      break;
    case DTI_CONN_STATE_DISCONNECTED:
      dti_cntrl_erase_entry(dti_id);
      if (simShrdPrm.sat_class_e_dti_id EQ dti_id)
      {
        simShrdPrm.sat_class_e_dti_id = DTI_DTI_ID_NOTPRESENT;
        TRACE_EVENT("sat_class_e_dti_id reset");
      }
      break;
    case DTI_CONN_STATE_CONNECTING:
    case DTI_CONN_STATE_CONNECTED:
      break;
    case DTI_CONN_STATE_ERROR:
      /* connection not possible: disconnect UDP or SNDCP */
      TRACE_EVENT("SIM_ENT_GPRSconnect_dti_cb connection not possible: disconnect UDP or SNDCP");
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;
    case DTI_CONN_STATE_UNKNOWN:
    default:
      TRACE_EVENT("SIM_ENT_GPRSconnect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}

#endif /* GPRS */

#endif /* FF_SAT_E */

#ifdef FF_DUAL_SIM

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SIMR                     |
|                            ROUTINE : cmhSIM_SIMSelected           |
+-------------------------------------------------------------------+

  PURPOSE : Informs the upper layer the result of the SIM Selection process.

*/
GLOBAL void cmhSIM_SIMSelected ( void )
{
  T_ACI_AT_CMD cmdBuf;
  T_ACI_CME_ERR err;

  TRACE_FUNCTION("cmhSIM_SIMSelected()");

  switch(simEntStat.curCmd)
  {
    case(AT_CMD_SIM):

      switch( simShrdPrm.rslt )
      {
        case( SIM_NO_ERROR ):

          simEntStat.curCmd = AT_CMD_NONE;

          R_AT( RAT_OK, simEntStat.entOwn )
              ( AT_CMD_SIM );
          cmh_logRslt ( simEntStat.entOwn, RAT_OK, AT_CMD_SIM,
		  	     -1, BS_SPEED_NotPresent,CME_ERR_NotPresent);
          return;

        default:
          err = cmhSIM_GetCmeFromSim ( simShrdPrm.rslt );
          break;
      }

      cmdBuf = simEntStat.curCmd;
      simEntStat.curCmd = AT_CMD_NONE;

      R_AT( RAT_CME, simEntStat.entOwn )
            ( cmdBuf, err );
      cmh_logRslt ( simEntStat.entOwn, RAT_CME, cmdBuf,
                    -1, BS_SPEED_NotPresent, err );
      break;
  }
}
#endif /*FF_DUAL_SIM*/

#ifdef FF_CPHS_REL4
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMR                 |
|                                 ROUTINE : cmhSIM_RdCnfCfis         |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            EF CFIS from SIM.
*/
GLOBAL void cmhSIM_RdCnfCfis ( SHORT table_id )
{
  T_ACI_AT_CMD    cmdBuf;    /* buffers current command          */
  T_ACI_CMD_SRC   ownBuf;    /* buffers current owner            */
  T_ACI_CFIS_CFU  cfisData;  /* CFIS data                        */ 

  UBYTE   *pExchData;        /* points to exchange data buffer   */
  UBYTE   dataLen;           /* holds length of data             */
  UBYTE   CFISmaxRcd;        /* holds no. of records in EF-CFIS  */

  TRACE_FUNCTION ("cmhSIM_RdCnfCfis()");

  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  pExchData  = simShrdPrm.atb[table_id].exchData;
  dataLen    = simShrdPrm.atb[table_id].dataLen;

  switch ( simShrdPrm.atb[table_id].errCode )
  {
    case ( SIM_NO_ERROR ):
    {
      /*
       *-------------------------------------------------------------
       * check basic consistency of read data
       *-------------------------------------------------------------
       */
      if ( pExchData EQ NULL        OR
           dataLen < ACI_SIZE_EF_CFIS  OR *pExchData EQ 0xFF)
      {
        simEntStat.curCmd = AT_CMD_NONE;
        simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;

        R_AT( RAT_CME, ownBuf ) ( cmdBuf, CME_ERR_Unknown );
      }
      else
      {
        /*
         *-----------------------------------------------------------
         * process parameter <msp>
         *-----------------------------------------------------------
         */
        cfisData.mspId = *pExchData;

       /*
        *-----------------------------------------------------------
        * process parameter <cfustat>
        *-----------------------------------------------------------
        */
        cfisData.cfuStat = *(pExchData+1);

       /*
        *-----------------------------------------------------------
        * process parameter <type>
        *-----------------------------------------------------------
        */
        cmhPHB_toaDmrg ( *( pExchData + 3 ),
                         &cfisData.type );

       /*
        *-----------------------------------------------------------
        * process parameter <number>
        *-----------------------------------------------------------
        */
        cmhPHB_getAdrStr ( cfisData.number ,
                           MAX_PHB_NUM_LEN - 1,
                           pExchData + 4,
                           *(pExchData+2) );
        
        CFISmaxRcd = simShrdPrm.atb[table_id].recMax;

        R_AT( RAT_P_CFIS, ownBuf ) ( &cfisData );

        if( CFISIndex )
        {
          if( CFISIndex NEQ CFISmaxRcd )
          {
            CFISIndex++;
            if ( cmhSIM_ReadRecordEF (ownBuf, AT_CMD_P_CFIS, SIM_CFIS,
                     CFISIndex, NOT_PRESENT_8BIT, NULL, cmhSIM_RdCnfCfis) NEQ AT_EXCT )
            {
              simEntStat.curCmd = AT_CMD_NONE;
              simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;

              R_AT( RAT_CME, ownBuf ) ( cmdBuf, CME_ERR_Unknown );
            }
          }
          else
          {
            CFISIndex=0;
          }
        }
        if( CFISIndex EQ 0 )
        {
          simEntStat.curCmd = AT_CMD_NONE;
          simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;
          R_AT( RAT_OK, ownBuf ) ( cmdBuf );
        }
      }
      break;
    }
    default:
    {
      simEntStat.curCmd = AT_CMD_NONE;
      simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;

      R_AT( RAT_CME, ownBuf )
        (
          cmdBuf,
          cmhSIM_GetCmeFromSim ( simShrdPrm.atb[table_id].errCode )
        );
      break;
    }
  }
simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMR                 |
|                                 ROUTINE : cmhSIM_WrCnfCfis         |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of writing data
            into EF-CFIS in SIM.
*/
GLOBAL void cmhSIM_WrCnfCfis ( SHORT table_id )
{

  T_ACI_AT_CMD  cmdBuf;       /* buffers current command     */
  T_ACI_CMD_SRC ownBuf;       /* buffers current owner       */

  TRACE_FUNCTION ("cmhSIM_WrCnfCfis ()");

  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  simEntStat.curCmd = AT_CMD_NONE;
  simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;

  if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR)
  {
    R_AT( RAT_OK, ownBuf ) ( cmdBuf );
  }
  else
  {
    R_AT( RAT_CME, ownBuf )  /* unsuccessful SIM write */
    (
      cmdBuf,
      cmhSIM_GetCmeFromSim (  simShrdPrm.atb[table_id].errCode )
    );
  }
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMR                 |
|                                 ROUTINE : cmhSIM_RdCnfMbi          |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            EF-MBI from SIM.
*/
GLOBAL void cmhSIM_RdCnfMbi ( SHORT table_id )
{
  T_ACI_AT_CMD    cmdBuf;    /* buffers current command          */
  T_ACI_CMD_SRC   ownBuf;    /* buffers current owner            */
  T_ACI_MBI       mbiData;   /* MBT  data                        */ 

  UBYTE   *pExchData;        /* points to exchange data buffer   */
  UBYTE   dataLen;           /* holds length of data             */
  UBYTE   MBImaxRcd;         /* holds no. of records in EF-MBI   */


  TRACE_FUNCTION ("cmhSIM_RdCnfMbi()");

  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  simEntStat.curCmd = AT_CMD_NONE;
  simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;
  pExchData  = simShrdPrm.atb[table_id].exchData;
  dataLen    = simShrdPrm.atb[table_id].dataLen;
  MBImaxRcd = simShrdPrm.atb[table_id].recMax;

  switch ( simShrdPrm.atb[table_id].errCode )
  {
    case ( SIM_NO_ERROR ):
    {
      /*
       *-------------------------------------------------------------
       * check basic consistency of read data
       *-------------------------------------------------------------
       */
      if ( pExchData EQ NULL        OR
           dataLen < ACI_SIZE_EF_MBI )
      {
        R_AT( RAT_CME, ownBuf ) ( cmdBuf, CME_ERR_Unknown );
      }
      else
      {
        mbiData.mbdn_id_voice = *pExchData;
        mbiData.mbdn_id_fax   = *( pExchData + 1 );
        mbiData.mbdn_id_email = *( pExchData + 2 );
        mbiData.mbdn_id_other = *( pExchData + 3 );

        R_AT( RAT_P_MBI, ownBuf ) ( &mbiData );

        if( MBI_Index )
        {
          if( MBI_Index NEQ MBImaxRcd )
          {
            MBI_Index++;
            if ( cmhSIM_ReadRecordEF (ownBuf, AT_CMD_P_MBI, SIM_MBI,
                     MBI_Index, NOT_PRESENT_8BIT, NULL, cmhSIM_RdCnfMbi) NEQ AT_EXCT )
            {
              R_AT( RAT_CME, ownBuf ) ( cmdBuf, CME_ERR_Unknown );
            }
          }
          else
          {
            MBI_Index = 0;
          }
        }
        if( MBI_Index EQ 0 )
        {
          R_AT( RAT_OK, ownBuf ) ( cmdBuf );
        }
      }
      break;
    }
    default:
    {
      R_AT( RAT_CME, ownBuf )
        (
          cmdBuf,
          cmhSIM_GetCmeFromSim ( simShrdPrm.atb[table_id].errCode )
        );
      break;
    }
  }
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMR                 |
|                                 ROUTINE : cmhSIM_RdCnfMbdn         |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            EF-MBDN from SIM.
*/
GLOBAL void cmhSIM_RdCnfMbdn ( SHORT table_id )
{
  T_ACI_AT_CMD    cmdBuf;    /* buffers current command          */
  T_ACI_CMD_SRC   ownBuf;    /* buffers current owner            */
  T_ACI_MBDN      mbdnData;  /* MBDN data                        */ 

  UBYTE   *pExchData;        /* points to exchange data buffer   */
  UBYTE   dataLen;           /* holds length of data             */
  UBYTE   alphaLen;          /* max. length of alpha identifier   */

  TRACE_FUNCTION ("cmhSIM_RdCnfMbdn()");

  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  pExchData  = simShrdPrm.atb[table_id].exchData;
  dataLen    = simShrdPrm.atb[table_id].dataLen;
  simEntStat.curCmd = AT_CMD_NONE;
  simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;

  switch ( simShrdPrm.atb[table_id].errCode )
  {
    case ( SIM_NO_ERROR ):
    {
      /*
       *-------------------------------------------------------------
       * check basic consistency of read data
       *-------------------------------------------------------------
       */
      if ( pExchData EQ NULL        OR
           dataLen < ACI_SIZE_EF_MBDN )
      {
        R_AT( RAT_CME, ownBuf ) ( cmdBuf, CME_ERR_Unknown );
      }
      else
      {
        alphaLen = dataLen - ACI_MIN_SIZE_EF_MSISDN;
        /*
         *-----------------------------------------------------------
         * process parameter <alpha>
         *-----------------------------------------------------------
        */
        cmhPHB_getTagNt ( pExchData,
                          alphaLen,
                          mbdnData.alpha,
                          MAX_ALPHA_LEN );

        pExchData += alphaLen;
       /*
        *-----------------------------------------------------------
        * process parameter <type>
        *-----------------------------------------------------------
        */
        cmhPHB_toaDmrg ( *( pExchData + 1 ),
                         &mbdnData.type );

       /*
        *-----------------------------------------------------------
        * process parameter <number>
        *-----------------------------------------------------------
        */
        cmhPHB_getAdrStr ( mbdnData.number ,
                           MAX_MB_NUM_LEN - 1,
                           pExchData + 2,
                           *pExchData );

        R_AT( RAT_P_MBDN, ownBuf ) ( &mbdnData );
        R_AT( RAT_OK, ownBuf ) ( cmdBuf );
      }
      break;
    }
    default:
    {
      R_AT( RAT_CME, ownBuf )
        (
          cmdBuf,
          cmhSIM_GetCmeFromSim ( simShrdPrm.atb[table_id].errCode )
        );
      break;
    }
  }
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMR                 |
|                                 ROUTINE : cmhSIM_WrCnfMbdn         |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of writing data
            into EF-MBDN in SIM.
*/
GLOBAL void cmhSIM_WrCnfMbdn ( SHORT table_id )
{

  T_ACI_AT_CMD  cmdBuf;       /* buffers current command     */
  T_ACI_CMD_SRC ownBuf;       /* buffers current owner       */

  TRACE_FUNCTION ("cmhSIM_WrCnfMbdn ()");

  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  simEntStat.curCmd = AT_CMD_NONE;
  simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;

  if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR)
  {
    R_AT( RAT_OK, ownBuf ) ( cmdBuf );
  }
  else
  {
    R_AT( RAT_CME, ownBuf )  /* unsuccessful SIM write */
    (
      cmdBuf,
      cmhSIM_GetCmeFromSim (  simShrdPrm.atb[table_id].errCode )
    );
  }
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMR                 |
|                                 ROUTINE : cmhSIM_RdCnfMwis         |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of reading
            EF MWIS from SIM.
*/
GLOBAL void cmhSIM_RdCnfMwis ( SHORT table_id )
{
  T_ACI_AT_CMD    cmdBuf;    /* buffers current command          */
  T_ACI_CMD_SRC   ownBuf;    /* buffers current owner            */
  T_ACI_MWIS_MWI  mwisData;  /* MWIS data                        */

  UBYTE   *pExchData;        /* points to exchange data buffer   */
  UBYTE   dataLen;           /* holds length of data             */
  UBYTE   MWISmaxRcd;        /* holds no. of records in EF-MWIS  */

  TRACE_FUNCTION ("cmhSIM_RdCnfMwis()");

  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  pExchData  = simShrdPrm.atb[table_id].exchData;
  dataLen    = simShrdPrm.atb[table_id].dataLen;

  switch ( simShrdPrm.atb[table_id].errCode )
  {
    case ( SIM_NO_ERROR ):
    {
      /*
       *-------------------------------------------------------------
       * check basic consistency of read data
       *-------------------------------------------------------------
       */
      if ( pExchData EQ NULL        OR
           dataLen < ACI_SIZE_EF_MWIS  OR *pExchData EQ 0x00)
      {
        simEntStat.curCmd = AT_CMD_NONE;
        simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;

        R_AT( RAT_CME, ownBuf ) ( cmdBuf, CME_ERR_Unknown );
      }
      else
      {
        /*
         *-----------------------------------------------------------
         * process parameter <mwisStat>
         *-----------------------------------------------------------
         */
        mwisData.mwiStat = *pExchData;

       /*
        *-----------------------------------------------------------
        * process parameter <mwis_count_voice>
        *-----------------------------------------------------------
        */
        mwisData.mwis_count_voice = *(pExchData+1);

       /*
        *-----------------------------------------------------------
        * process parameter <mwis_count_fax>
        *-----------------------------------------------------------
        */
        mwisData.mwis_count_fax = *(pExchData+2);

       /*
        *-----------------------------------------------------------
        * process parameter <mwis_count_email>
        *-----------------------------------------------------------
        */
        mwisData.mwis_count_email = *(pExchData+3);

       /*
        *-----------------------------------------------------------
        * process parameter <mwis_count_other>
        *-----------------------------------------------------------
        */
        mwisData.mwis_count_other = *(pExchData+4);

        MWISmaxRcd = simShrdPrm.atb[table_id].recMax;

        R_AT( RAT_P_MWIS, ownBuf ) ( &mwisData );

        if( MWISIndex )
        {
          if( MWISIndex NEQ MWISmaxRcd )
          {
            MWISIndex++;
            if ( cmhSIM_ReadRecordEF (ownBuf, AT_CMD_P_MWIS, SIM_MWIS,
                 MWISIndex, NOT_PRESENT_8BIT, NULL, cmhSIM_RdCnfMwis) NEQ AT_EXCT )
            {
              simEntStat.curCmd = AT_CMD_NONE;
              simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;

              R_AT( RAT_CME, ownBuf ) ( cmdBuf, CME_ERR_Unknown );
            }
          }
          else
          {
            MWISIndex=0;
          }
        }
        if( MWISIndex EQ 0 )
        {
          simEntStat.curCmd = AT_CMD_NONE;
          simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;
          R_AT( RAT_OK, ownBuf ) ( cmdBuf );
        }
      }
      break;
    }
    default:
    {
      simEntStat.curCmd = AT_CMD_NONE;
      simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;

      R_AT( RAT_CME, ownBuf )
        (
          cmdBuf,
          cmhSIM_GetCmeFromSim ( simShrdPrm.atb[table_id].errCode )
        );
      break;
    }
  }
simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMR                 |
|                                 ROUTINE : cmhSIM_WrCnfMwis         |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the confirmation of writing data
            into EF-MWIS in SIM.
*/
GLOBAL void cmhSIM_WrCnfMwis ( SHORT table_id )
{

  T_ACI_AT_CMD  cmdBuf;       /* buffers current command     */
  T_ACI_CMD_SRC ownBuf;       /* buffers current owner       */
  U8 i = 0;
  T_ACI_MWIS_MWI  mwisData;   /* MWIS data                   */

  TRACE_FUNCTION ("cmhSIM_WrCnfMwis ()");

  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;
  simEntStat.curCmd = AT_CMD_NONE;
  simEntStat.entOwn = simShrdPrm.owner = OWN_NONE;

  if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR)
  {
    if( cmdBuf NEQ AT_CMD_P_MWIS )
    {
      memcpy(&mwisData,&smsShrdPrm.MWISdata,sizeof(T_ACI_MWIS_MWI));
      for(i; i < CMD_SRC_MAX; i++)
      {
        R_AT( RAT_P_MWI, i)(1,&mwisData);
      }
    }
    else
    {
      R_AT( RAT_OK, ownBuf ) ( cmdBuf );
    }
  }
  else
  {
    R_AT( RAT_CME, ownBuf )  /* unsuccessful SIM write */
    (
      cmdBuf,
      cmhSIM_GetCmeFromSim (  simShrdPrm.atb[table_id].errCode )
    );
  }
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}
#endif /* FF_CPHS_REL4 */

/* Implements Measure 97 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_CardUnblocked_PINChanged
+------------------------------------------------------------------------------
|  Purpose     : This function can check whether the Card is Unblocked
|                or PIN is Changed.
|                When this function will be called for checking whether
|                the PIN is changed then the argument PINChanged will be
|                TRUE otherwise it will be FALSE
|
|  Parameters  : PINChanged - TRUE or FALSE
|
|  Return      : void
+------------------------------------------------------------------------------
*/

GLOBAL void cmhSIM_CardUnblocked_PINChanged ( BOOL PINChanged )
{
  UBYTE cmdBuf;
  T_ACI_CME_ERR err;

  TRACE_FUNCTION ( "cmhSIM_CardUnblocked_PINChanged()" );

  /*
   *-------------------------------------------------------------------
   * check for command context
   *-------------------------------------------------------------------
   */

  switch( simEntStat.curCmd )
  {
    case( AT_CMD_CPIN ):
    case( AT_CMD_PVRF ):
      /*lint -e{408}*/
    case( KSD_CMD_UBLK ):
    case( AT_CMD_CPWD ):
      /*lint -e{408}*/
    case( KSD_CMD_PWD ):
      /*
       *----------------------------------------------------------------
       * process event for +CPIN, %PVRF and KSD UBLK command
       * also
       * process event for +CPWD and KSD PWD command
       *----------------------------------------------------------------
       */
      switch( simShrdPrm.rslt )
      {
        case( SIM_NO_ERROR ):
          if( (PINChanged EQ TRUE) OR (simShrdPrm.crdPhs NEQ 0xFF) )
          {
            cmdBuf = simEntStat.curCmd;
            simEntStat.curCmd = AT_CMD_NONE;

            R_AT( RAT_OK, simEntStat.entOwn )
              ( cmdBuf );
            cmh_logRslt ( simEntStat.entOwn, RAT_OK, (T_ACI_AT_CMD)cmdBuf, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
          }
          return;

        case( SIM_CAUSE_PIN1_EXPECT ):
        case( SIM_CAUSE_PIN2_EXPECT ):
          err = CME_ERR_WrongPasswd;
          break;

        default:
          err = cmhSIM_GetCmeFromSim ( simShrdPrm.rslt );
          break;
      }

      cmdBuf = simEntStat.curCmd;
      simEntStat.curCmd = AT_CMD_NONE;

      R_AT( RAT_CME, simEntStat.entOwn )
            ( cmdBuf, err );

      cmh_logRslt ( simEntStat.entOwn, RAT_CME, (T_ACI_AT_CMD)cmdBuf,
                    -1, BS_SPEED_NotPresent, err );
      break;
  }
}

/* Implements Measure 106 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_ProcessEvents
|------------------------------------------------------------------------------
|  Purpose     : Process Events for +CAMM, +CPUC, +CACM commands.
|
|  Parameters  : at_cmd_id - AT command identifier
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void cmhSIM_ProcessEvents ( T_ACI_AT_CMD at_cmd_id )
{
  TRACE_FUNCTION ( "cmhSIM_ProcessEvents()" );

  simEntStat.curCmd = AT_CMD_NONE;

  if( simShrdPrm.rslt EQ SIM_NO_ERROR )
  {
    /*
     * Try it again
     */
    switch ( at_cmd_id )
    {
      case ( AT_CMD_CAMM ):
        aoc_update_sim_datafield ( SECOND_UPDATE, ACT_WR_DAT,
                                   SIM_ACMMAX, aoc_update_acmmax_cb );
        break;

      case ( AT_CMD_CPUC ):
        aoc_update_puct (SECOND_UPDATE, 0L);
        break;

      case ( AT_CMD_CACM ):
        aoc_update_sim_datafield ( SECOND_UPDATE, ACT_WR_REC,
                                   SIM_ACM, aoc_update_acm_cb );
        break;
    }
  }
  else
  {
    R_AT( RAT_CME, simEntStat.entOwn )
          ( at_cmd_id, CME_ERR_WrongPasswd );
  }
}

/* Implements Measure 183 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_PINEnabledDisabled
|------------------------------------------------------------------------------
|  Purpose     : SIM Enabled or Disabled
|
|  Parameters  : void
|
|  Return      : void
+------------------------------------------------------------------------------
*/

GLOBAL void cmhSIM_PINEnabledDisabled ( void )
{
  T_ACI_CME_ERR err;

  TRACE_FUNCTION ( "cmhSIM_PINEnabledDisabled()" );

  /*
   *-------------------------------------------------------------------
   * check for command context
   *-------------------------------------------------------------------
   */
  if( simEntStat.curCmd EQ AT_CMD_CLCK )
  {
    /*
     *----------------------------------------------------------------
     * process event for +CLCK command
     *----------------------------------------------------------------
     */
    switch( simShrdPrm.rslt )
    {
      case( SIM_NO_ERROR ):

        simEntStat.curCmd = AT_CMD_NONE;

        R_AT( RAT_OK, simEntStat.entOwn )
              ( AT_CMD_CLCK );
        cmh_logRslt ( simEntStat.entOwn, RAT_OK, AT_CMD_CLCK, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
        return;

      case( SIM_CAUSE_PIN1_EXPECT ):
      case( SIM_CAUSE_PIN1_BLOCKED ):
        err = CME_ERR_WrongPasswd;
        break;

      default:
        err = cmhSIM_GetCmeFromSim ( simShrdPrm.rslt );
        break;
    }

    simEntStat.curCmd = AT_CMD_NONE;

    R_AT( RAT_CME, simEntStat.entOwn )
          ( AT_CMD_CLCK, err );
    cmh_logRslt ( simEntStat.entOwn, RAT_CME, AT_CMD_CLCK,
                  -1, BS_SPEED_NotPresent, err );      
  }
}

/* Implements Measure 200 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_Compare_CNUMMsisdnIdx
|------------------------------------------------------------------------------
|  Purpose     : Comapres CNUMMsisdnIdx
|
|  Parameters  : void
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void cmhSIM_Compare_CNUMMsisdnIdx ( void )
{
  T_SIM_CMD_PRM* pSIMCmdPrm;
  T_ACI_AT_CMD   cmdBuf;
  T_ACI_CMD_SRC  ownBuf;

  TRACE_FUNCTION ( "cmhSIM_Compare_CNUMMsisdnIdx()" );
  
  pSIMCmdPrm = &cmhPrm[simEntStat.entOwn].simCmdPrm;
  cmdBuf     = simEntStat.curCmd;
  ownBuf     = simEntStat.entOwn;

  if ( (CNUMMsisdnIdx EQ MAX_MSISDN - 1) OR 
       ((pSIMCmdPrm -> CNUMActRec) EQ CNUMMaxRec) )
  {
    simEntStat.curCmd = AT_CMD_NONE;
    simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
    simEntStat.entOwn = CMD_SRC_NONE;

    R_AT( RAT_CNUM, ownBuf ) ( &CNUMMsisdn[0], CNUMMaxRec );

    if ( (pSIMCmdPrm -> CNUMActRec) EQ CNUMMaxRec )
    {
      if (pSIMCmdPrm -> CNUMOutput)
      {
        R_AT( RAT_OK, ownBuf ) ( cmdBuf );
      }
      else
      {
        R_AT( RAT_CME, ownBuf ) ( cmdBuf, CME_ERR_NotFound );
      }
    }
  }
  else
  {
    CNUMMsisdnIdx++;
    pSIMCmdPrm -> CNUMActRec++;

    if ( cmhSIM_ReqMsisdn ( ownBuf, pSIMCmdPrm -> CNUMActRec ) NEQ AT_EXCT )
    {
      cmhSIM_SndError( ownBuf, cmdBuf, CME_ERR_Unknown );
    }
  }

}

/* Implements Measure 7, 17, 24, 35 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_SndError
|------------------------------------------------------------------------------
|  Purpose     : Reports Error
|
|  Parameters  : ownBuf  - AT Command Source Identifier 
|                cmdBuf  - AT Command Identifier
|                cme_err - +CME ERROR parameter
|
|  Return      : void
+------------------------------------------------------------------------------
*/
LOCAL void cmhSIM_SndError( T_ACI_CMD_SRC  ownBuf, T_ACI_AT_CMD   cmdBuf,
                            T_ACI_CME_ERR  cme_err )
{
  TRACE_FUNCTION ( "cmhSIM_SndError()" );

  simEntStat.curCmd = AT_CMD_NONE;
  simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;
  simEntStat.entOwn = CMD_SRC_NONE;

  R_AT( RAT_CME, ownBuf ) ( cmdBuf, cme_err );

}

/*==== EOF ========================================================*/

