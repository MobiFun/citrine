/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SIMS
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
|  Purpose :  This module provides the set functions related to the
|             protocol stack adapter for the subscriber identity
|             module.
+-----------------------------------------------------------------------------
*/

#ifndef CMH_SIMS_C
#define CMH_SIMS_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"

#ifdef UART
#include "dti.h"      /* functionality of the dti library */
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#include "ati_cmd.h"

#ifdef FF_ATI
#include "aci_io.h"
#endif /* of #ifdef FF_ATI */

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_sim.h"
#include "psa_mm.h"
#include "cmh.h"
#include "cmh_sim.h"
#include "cmh_mm.h"

#ifdef GPRS
  #include "dti_cntrl_mng.h"
  #include "gaci.h"
  #include "gaci_cmh.h"
  #include "psa_gmm.h"
  #include "cmh_gmm.h"
#endif

#include "aci.h"

/* #include "m_fac.h" */
#include "aoc.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "phb.h"

#include "aci_ext_pers.h"
#include "aci_slock.h"

#include "cl_imei.h"


#ifdef SIM_PERS
#include "general.h" // included for compilation error UNIT8 in sec_drv.h
#include "sec_drv.h" 
#include "cmh.h"
#include "aci_cmh.h"

EXTERN T_SIM_MMI_INSERT_IND *last_sim_mmi_insert_ind;
EXTERN T_SEC_DRV_CATEGORY *personalisation_nw;
EXTERN T_SEC_DRV_CATEGORY *personalisation_ns;
EXTERN T_SEC_DRV_CATEGORY *personalisation_sp;
EXTERN T_SEC_DRV_CATEGORY *personalisation_cp;
EXTERN T_SEC_DRV_CATEGORY *personalisation_sim;
EXTERN T_SEC_DRV_CONFIGURATION *cfg_data ;
#endif

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
LOCAL T_ACI_RETURN cmhSIM_analyze_SIMLOCK_STATUS( T_SIMLOCK_STATUS status );
LOCAL T_ACI_RETURN sAT_Plus_CAMM_CACM ( T_ACI_AT_CMD  at_cmd_id,
                                        T_ACI_CMD_SRC srcId,
                                        UBYTE         value_type,
                                        void          *value,
                                        UBYTE         *pwd );
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PlusCFUN             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CFUN AT command
            which is responsible to set the phone functionality.

            <fun>:    defines the level of functionality to switch to.
            <rst>:    reset mode
*/

GLOBAL T_ACI_RETURN sAT_PlusCFUN ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_CFUN_FUN fun,
                                   T_ACI_CFUN_RST rst )
{
  T_SIM_SET_PRM * pSIMSetPrm; /* points to MM parameter set */
  T_ACI_RETURN    retCd;      /* holds return code */
  UBYTE           idx;        /* holds index value */

  /* variables for IMEI control mechanism */
#ifndef _SIMULATION_
  BYTE         retVal;                     /* holds return value */
  UBYTE        dummyIMEIBuf[CL_IMEI_SIZE]; /* dummy IMEI buffer */
#endif /* if NOT defined windows simulation */

  TRACE_FUNCTION ("sAT_PlusCFUN()");

#ifndef _SIMULATION_
/*
 *-------------------------------------------------------------------
 * check IMEI
 *-------------------------------------------------------------------
 */

 retVal = cl_get_imeisv(CL_IMEI_SIZE, dummyIMEIBuf, CL_IMEI_CONTROL_IMEI);

 if( retVal NEQ CL_IMEI_OK )
 {
   ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_IMEICheck );
   TRACE_EVENT("IMEI not valid");
   simShrdPrm.imei_blocked = TRUE;
   /*return( AT_FAIL ); We dont return here to enable the stack to go to the state of "Limited Service' to enable emergency calls. */ 
 }
#endif /* if NOT defined windows simulation */

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pSIMSetPrm = &simShrdPrm.setPrm[srcId];

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( simEntStat.curCmd NEQ AT_CMD_NONE )
  {
    TRACE_EVENT("Entity SIM is busy: cannot proceed command...");
    return( AT_BUSY );
  }

/*
 *-------------------------------------------------------------------
 * process the <rst> parameter
 *-------------------------------------------------------------------
 */
  switch( rst )
  {
    case( CFUN_RST_NotPresent ):  /* default value */
    case( CFUN_RST_NoReset ):

    break;

    default:                      /* unexpected parameter */
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the <fun> parameter
 *-------------------------------------------------------------------
 */
  if( fun EQ CFUN_FUN_NotPresent ) fun = CFUNfun;

  switch( fun )
  {
    case( CFUN_FUN_Minimum ):    /* set ME to minimum functionality */
      CFUNfun           = fun;
      simEntStat.curCmd = AT_CMD_CFUN;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn = srcId;

    /* Turn off all possible ringing */
#ifdef FF_ATI
    io_setRngInd ( IO_RING_OFF, CRING_SERV_TYP_NotPresent, CRING_SERV_TYP_NotPresent ); /* V.24 Ring Indicator Line */
#endif
    for( idx = 0; idx < CMD_SRC_MAX; idx++ )
    {
      R_AT( RAT_CRING_OFF, (T_ACI_CMD_SRC)idx )( 0 );  /* Turn of all ringing */
    }


    /* We have to wait for both entities to finish in CFUN. So both EntStat are set to AT_CMD_CFUN and
       when a certain entity finished it also emits AT_OK if the other entity has already finished */

    simEntStat.curCmd = AT_CMD_CFUN;
    simShrdPrm.owner = (T_OWN)srcId;
    simEntStat.entOwn =  srcId;

    mmEntStat.curCmd  = AT_CMD_CFUN;
    mmShrdPrm.owner = (T_OWN)srcId;
    mmEntStat.entOwn  = srcId;

    pb_exit();

/*    simShrdPrm.synCs = SYNC_DEACTIVATE;  */   /* This is moved to pb_exit */
/*    psaSIM_SyncSIM(); */

    /* Reset ONSDesc */
     cmhMM_Reset_ONSDesc();


#if defined (GPRS) AND defined (DTI)
    mmShrdPrm.nrgCs = GMMREG_DT_POWER_OFF;
    if( psaG_MM_CMD_DEREG ( mmShrdPrm.nrgCs ) < 0 )  /* deregister from network */
#else
    mmShrdPrm.nrgCs = CS_POW_OFF;
    if( psaMM_DeRegistrate () < 0 )  /* deregister from network */
#endif
    {
      TRACE_EVENT( "FATAL RETURN psaMM_DeRegistrate in +COPS" );
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
      return( AT_FAIL );
    }

    percentCSTAT_indication(STATE_MSG_SMS,   ENTITY_STATUS_NotReady);

    retCd = AT_EXCT;
    break;

  case( CFUN_FUN_Full ):       /* set ME to full functionality */
    if ( (CFUNfun EQ CFUN_FUN_Minimum) OR (simShrdPrm.PINStat NEQ PS_RDY) )
    {
      CFUNfun               = fun;
      pSIMSetPrm -> actProc = SIM_INITIALISATION;

      simEntStat.curCmd = AT_CMD_CFUN;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn =  srcId;

      if( psaSIM_ActivateSIM() < 0 )   /* activate SIM card */
      {
        TRACE_EVENT( "FATAL RETURN psaSIM in +CFUN" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        return( AT_FAIL );
      }

      retCd = AT_EXCT;
#ifdef GPRS
     {
       PALLOC (gmmreg_attach_req, GMMREG_ATTACH_REQ);

       /* This is just a dummy req to trigger lower layer
          for power scanning. It will be done based on bootup_act.*/
       gmmreg_attach_req->bootup_act = QUICK_REG;

       PSEND (hCommGMM,gmmreg_attach_req);
     }
#else
     {
       PALLOC (mmr_reg_req, MMR_REG_REQ);

       /* This is just a dummy req to trigger lower layer
          for power scanning. It will be done based on bootup_act.
          Other parameters is filled for the simulation test cases to pass */
       mmr_reg_req->bootup_act = QUICK_REG;

       PSENDX (MM, mmr_reg_req);
     }
#endif
    }
#ifdef SIM_PERS
   else if (CFUNfun EQ CFUN_FUN_Full ) 
   {
   TRACE_EVENT("This is for if MMI calls sAT_PlusCFUN repetedly for verification of LOCKS");
     CFUNfun               = fun;
      pSIMSetPrm -> actProc = SIM_INITIALISATION;

      simEntStat.curCmd = AT_CMD_CFUN;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn = srcId;

      if( psaSIM_ActivateSIM() < 0 )   /* activate SIM card */
      {
        TRACE_EVENT( "FATAL RETURN psaSIM in +CFUN" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        return( AT_FAIL );
      }

      retCd = AT_EXCT;
   }
#endif
  else
    {
      TRACE_EVENT("Switch mobile back on after radio low (CFUN=4)");
      CFUNfun  = fun;
      retCd    = AT_CMPL;
    }
    break;

  case (CFUN_FUN_Disable_TX_RX_RF):
    if (CFUNfun EQ CFUN_FUN_Minimum)
    {
      /* User directly goes to flight mode from power off. Only initialise
         the SIM data */
      CFUNfun               = fun;
      pSIMSetPrm -> actProc = SIM_INITIALISATION;

      simEntStat.curCmd = AT_CMD_CFUN;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn =  srcId;

      if( psaSIM_ActivateSIM() < 0 )   /* activate SIM card */
      {
        TRACE_EVENT( "FATAL RETURN psaSIM in +CFUN" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        return( AT_FAIL );
      }

      retCd = AT_EXCT;
      break;
    }
    CFUNfun           = fun;

    /* Turn off all possible ringing */
#ifdef FF_ATI
    io_setRngInd ( IO_RING_OFF, CRING_SERV_TYP_NotPresent, CRING_SERV_TYP_NotPresent ); /* V.24 Ring Indicator Line */
#endif
    for( idx = 0; idx < CMD_SRC_MAX; idx++ )
    {
      R_AT( RAT_CRING_OFF, (T_ACI_CMD_SRC)idx )( 0 );  /* Turn of all ringing */
    }

    if( mmEntStat.curCmd NEQ AT_CMD_BAND )
    {
      /* If sAT_PlusCFUN has been called by sAT_PercentBAND !!! */
      mmEntStat.curCmd  = AT_CMD_CFUN;
    }
    mmShrdPrm.owner = (T_OWN)srcId;
    mmEntStat.entOwn  = srcId;

#if defined (GPRS) AND defined (DTI)
    mmShrdPrm.nrgCs = GMMREG_DT_SOFT_OFF;
    if( psaG_MM_CMD_DEREG ( mmShrdPrm.nrgCs ) < 0 )  /* deregister from network */
#else
    mmShrdPrm.nrgCs = CS_SOFT_OFF;
    if( psaMM_DeRegistrate () < 0 )  /* deregister from network */
#endif
    {
      TRACE_EVENT( "FATAL RETURN psaMM_DeRegistrate in +COPS" );
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
      return( AT_FAIL );
    }

    retCd = AT_EXCT;
    break;

  default:                     /* unexpected parameter */

    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  if( mmEntStat.curCmd NEQ AT_CMD_BAND
    AND simEntStat.curCmd NEQ AT_CMD_BAND )
  {
  T_ACI_CLOG      cmdLog;     /* holds logging info */

  cmdLog.atCmd                = AT_CMD_CFUN;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = retCd;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sCFUN.srcId   = srcId;
  cmdLog.cmdPrm.sCFUN.fun     = fun;
  cmdLog.cmdPrm.sCFUN.rst     = rst;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( retCd );

}

#ifdef TI_PS_FF_AT_P_CMD_SECP
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PercentSECP             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SEC AT command
            which is responsible to enter a PIN.

            <pin>:    string of PIN chars.
            <newpin>: string of PIN chars required if requested PIN is
                      SIM PUK
*/

GLOBAL T_ACI_RETURN sAT_PercentSECP ( T_ACI_CMD_SRC srcId,
                                      CHAR * pin,
                                      CHAR * newpin )
{
  T_SIMLOCK_STATUS  result;  
  
  TRACE_FUNCTION ("sAT_PercentSECP()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }
  
  /* Check validity of pin str*/
  if( pin EQ NULL)
  {
     ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
     return( AT_FAIL );
  }

  /* Try to set new pin */
  result = aci_ext_personalisation_CS_change_password( pin, newpin );     

  /* Implements Measure 50 */
  return ( cmhSIM_analyze_SIMLOCK_STATUS ( result ) );
  
}
#endif /* TI_PS_FF_AT_P_CMD_SECP */

#ifdef TI_PS_FF_AT_P_CMD_SECS
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PercentSECS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SECS? AT command
            which is responsible to set the status of the Security Code.

            <securityStatus>:   Status of the security code.
            <code>: Security code required to change the status.
*/


GLOBAL T_ACI_RETURN sAT_PercentSECS ( T_ACI_CMD_SRC srcId,
                                   T_ACI_SECS_STA securityState,
                                   CHAR * code )
{

  T_SIMLOCK_STATUS  result;

  TRACE_FUNCTION ("sAT_PercentSECS()");


/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check status value
 *-------------------------------------------------------------------
 */
  switch (securityState)
  {
      case( SECS_STA_Enable ):
        result = aci_ext_personalisation_CS_set_status(SIMLOCK_ENABLED, code);
        break;      
      case( SECS_STA_Disable ):
        result = aci_ext_personalisation_CS_set_status(SIMLOCK_DISABLED, code);
        break;      
      default:
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * analyze answer
 *-------------------------------------------------------------------
 */
  /* Implements Measure 50 */
  return ( cmhSIM_analyze_SIMLOCK_STATUS ( result ) );
}
#endif /* TI_PS_FF_AT_P_CMD_SECS */





/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PlusCPIN             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPIN AT command
            which is responsible to enter a PIN.

            <pin>:    string of PIN chars.
            <newpin>: string of PIN chars required if requested PIN is
                      SIM PUK
*/

GLOBAL T_ACI_RETURN sAT_PlusCPIN ( T_ACI_CMD_SRC srcId,
                                   CHAR * pin,
                                   CHAR * newpin )
{
  T_SIM_SET_PRM * pSIMSetPrm;  /* points to SIM parameter set */
  T_ACI_RETURN    retCd;              /* holds return code */
  T_SIMLOCK_STATUS retSlStatus; /* holds return code */

  TRACE_FUNCTION ("sAT_PlusCPIN()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pSIMSetPrm = &simShrdPrm.setPrm[srcId];

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( simEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check for PIN status
 *-------------------------------------------------------------------
 */
  switch( simShrdPrm.PINStat )
  {
    case( PS_RDY ):
      /*
       *---------------------------------------------------------------
        * Not a SIM PIN State, but a ME personalisation PIN
       *---------------------------------------------------------------
        */
  #ifdef SIM_PERS
      if (AciSLockShrd.blocked)
      {
        retSlStatus = aci_slock_authenticate(AciSLockShrd.current_lock, pin);
        if ( retSlStatus NEQ SIMLOCK_DISABLED)
        {
          TRACE_EVENT( "Wrong PIN given for SIM lock." );
          if (retSlStatus EQ SIMLOCK_BLOCKED)
          {
             if(!aci_slock_set_CFG())
             {
                 ACI_ERR_DESC( ACI_ERR_CLASS_Ext,EXT_ERR_NoMEPD); 
                 return( AT_FAIL );
             }
            aci_set_cme_error(AciSLockShrd.current_lock); 
            MFREE(cfg_data); 
          }
          else
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_WrongPasswd );
          return( AT_FAIL );
        }
        else
        {
             simEntStat.curCmd     = AT_CMD_CPIN;
	      if(!aci_slock_set_CFG())
             	{
             	   ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_NoMEPD); 
                 return( AT_FAIL );
             	}
             aci_slock_init();
	      
             retSlStatus = SIMLOCK_ENABLED;
	      if(AciSLockShrd.current_lock < SIMLOCK_SIM)
             {
                AciSLockShrd.current_lock=(T_SIMLOCK_TYPE)(AciSLockShrd.current_lock +SIMLOCK_NETWORK_SUBSET);
                 AciSLockShrd.check_lock = SIMLOCK_CHECK_PERS;
	          retSlStatus = aci_slock_checkpersonalisation(AciSLockShrd.current_lock);
		}
		switch(retSlStatus)
                {
                 case  SIMLOCK_ENABLED  :
	                return( AT_CMPL);
                 case  SIMLOCK_BLOCKED :
	                return( AT_FAIL);
                 case  SIMLOCK_WAIT :
                        return (AT_EXCT);
                         
               }           
          }

	 }

     #endif  
    /*
     *---------------------------------------------------------------
     * no PIN input is required
     *---------------------------------------------------------------
     */
      retCd = AT_FAIL;
      break;

    case( NO_VLD_PS ):
    /*
     *---------------------------------------------------------------
     * PIN Status is unknown
     *---------------------------------------------------------------
     */

    /* invoke SIM activate and enter PIN if needed ???? */
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return( AT_FAIL );

    case( PS_PIN1 ):
    /*
     *---------------------------------------------------------------
     * PIN 1 input is required
     *---------------------------------------------------------------
     */
      if( pin EQ NULL                 OR
          strlen( pin ) < MIN_PIN_LEN OR
          strlen( pin ) > PIN_LEN        )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      cmhSIM_FillInPIN ( pin, pSIMSetPrm -> curPIN, PIN_LEN );
      pSIMSetPrm -> PINType = PHASE_2_PIN_1;
      simEntStat.curCmd     = AT_CMD_CPIN;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn     = srcId;

      if( psaSIM_VerifyPIN() < 0 )  /* verify PIN */
      {
        TRACE_EVENT( "FATAL RETURN psaSIM in +CPIN" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        return( AT_FAIL );
      }

      retCd = AT_EXCT;
      break;

    case( PS_PIN2 ):
    /*
     *---------------------------------------------------------------
     * PIN 2 input is required
     *---------------------------------------------------------------
     */
      if( pin EQ NULL                 OR
          strlen( pin ) < MIN_PIN_LEN OR
          strlen( pin ) > PIN_LEN        )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      cmhSIM_FillInPIN ( pin, pSIMSetPrm -> curPIN, PIN_LEN );
      pSIMSetPrm -> PINType = PHASE_2_PIN_2;
      simEntStat.curCmd     = AT_CMD_CPIN;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn     = srcId;

      if( psaSIM_VerifyPIN() < 0 )  /* verify PIN */
      {
        TRACE_EVENT( "FATAL RETURN psaSIM in +CPIN" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        return( AT_FAIL );
      }

      retCd = AT_EXCT;
      break;

    case( PS_PUK1 ):
    /*
     *---------------------------------------------------------------
     * PUK 1 input is required
     *---------------------------------------------------------------
     */
      if( newpin EQ NULL                 OR
          strlen( newpin ) EQ 0)
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPukReq ); /* inform about needed PUK */
        return( AT_FAIL );
      }
      if( pin EQ NULL                    OR
          newpin EQ NULL                 OR
          strlen( pin ) NEQ PUK_LEN      OR
          strlen( newpin ) < MIN_PIN_LEN OR
          strlen( newpin ) > PIN_LEN        )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      cmhSIM_FillInPIN ( pin,    pSIMSetPrm -> unblkKey, PUK_LEN );
      cmhSIM_FillInPIN ( newpin, pSIMSetPrm -> curPIN,   PIN_LEN );
      pSIMSetPrm -> PINType = PHASE_2_PUK_1;
      simEntStat.curCmd     = AT_CMD_CPIN;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn     = srcId;

      if( psaSIM_UnblockCard( ) < 0 )  /* verify PIN */
      {
        TRACE_EVENT( "FATAL RETURN psaSIM in +CPIN" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        return( AT_FAIL );
      }

      retCd = AT_EXCT;
      break;

    case( PS_PUK2 ):
    /*
     *---------------------------------------------------------------
     * PUK 2 input is required
     *---------------------------------------------------------------
     */
      if( newpin EQ NULL                 OR
          strlen( newpin ) EQ 0)
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPuk2Req ); /* inform about needed PUK2 */
        return( AT_FAIL );
      }
      if( pin EQ NULL                    OR
          newpin EQ NULL                 OR
          strlen( pin ) NEQ PUK_LEN      OR
          strlen( newpin ) < MIN_PIN_LEN OR
          strlen( newpin ) > PIN_LEN        )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      cmhSIM_FillInPIN ( pin,    pSIMSetPrm -> unblkKey, PUK_LEN );
      cmhSIM_FillInPIN ( newpin, pSIMSetPrm -> curPIN,   PIN_LEN );
      pSIMSetPrm -> PINType = PHASE_2_PUK_2;
      simEntStat.curCmd     = AT_CMD_CPIN;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn     = srcId;

      if( psaSIM_UnblockCard( ) < 0 )  /* verify PIN */
      {
        TRACE_EVENT( "FATAL RETURN psaSIM in +CPIN" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        return( AT_FAIL );
      }

      retCd = AT_EXCT;
      break;

  default:
      /*
       *---------------------------------------------------------------
       * unexpected PIN state
       *---------------------------------------------------------------
       */
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_DataCorrupt );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  {
  T_ACI_CLOG      cmdLog;      /* holds logging info */

  cmdLog.atCmd                = AT_CMD_CPIN;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = retCd;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sCPIN.srcId   = srcId;
  cmdLog.cmdPrm.sCPIN.pin     = pin;
  cmdLog.cmdPrm.sCPIN.newpin  = newpin;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( retCd );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PlusCAMM             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CAMM AT command
            which is responsible to set the ACMMax.
*/

GLOBAL T_ACI_RETURN sAT_PlusCAMM ( T_ACI_CMD_SRC    srcId,
                                   LONG             acmmax,
                                   CHAR *           pwd)
{
  /* Implements Measure 40 */
  return ( sAT_Plus_CAMM_CACM( AT_CMD_CAMM, srcId, AOC_ACMMAX,
                               (void *) acmmax, (UBYTE *) pwd ));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PlusCPUC             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPUC AT command
            which is responsible to set the PUCT values.
*/

GLOBAL T_ACI_RETURN sAT_PlusCPUC ( T_ACI_CMD_SRC    srcId,
                                   CHAR *           currency,
                                   CHAR *           ppu,
                                   CHAR *           pwd)
{
  T_puct          puct;
  T_ACI_RETURN    ret = AT_FAIL;

  TRACE_FUNCTION ("sAT_PlusCPUC()");
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * send parameters to advice of charge module.
 *-------------------------------------------------------------------
 */
  strcpy ((char *) puct.currency, currency);
  strcpy ((char *) puct.value, ppu);
  simEntStat.curCmd     = AT_CMD_CPUC;
  simShrdPrm.owner = (T_OWN)srcId;
  simEntStat.entOwn     = srcId;
  ret = aoc_set_values (srcId,
                        AOC_PUCT,
                        (void *)&puct,
                        (UBYTE *) pwd);

/*
 *-------------------------------------------------------------------
 * Check return value of aoc_set_values() equal to AT_FAIL,
 * resets simEntStat.curcmd and simEntStat.entown.
 *-------------------------------------------------------------------
 */

  if( ret EQ AT_FAIL )
  {
    simEntStat.curCmd    = AT_CMD_NONE;
    simShrdPrm.owner     = (T_OWN)CMD_SRC_NONE;
    simEntStat.entOwn    = CMD_SRC_NONE;
  }

  return( ret );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PlusCACM             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CACM AT command
            which is responsible to reset the ACM value.
*/

GLOBAL T_ACI_RETURN sAT_PlusCACM ( T_ACI_CMD_SRC    srcId,
                                   CHAR *           pwd)
{
  /* Implements Measure 40 */
  return ( sAT_Plus_CAMM_CACM( AT_CMD_CACM, srcId, AOC_ACM,
                               (void *) NULL, (UBYTE *) pwd ));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PlusCPOL             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPOL AT command
            which is responsible to access the preferred PLMN list.

            <index>:  PLMN list index
            <format>: PLMN format
            <oper>:   PLMN name
            <index2>: second PLMN list index for exchange operation
            <mode>:   supplemental mode information
*/

GLOBAL T_ACI_RETURN sAT_PlusCPOL  ( T_ACI_CMD_SRC srcId,
                                    SHORT index,
                                    T_ACI_CPOL_FRMT format,
                                    CHAR * oper,
                                    SHORT index2,
                                    T_ACI_CPOL_MOD mode )
{
  T_SIM_CMD_PRM * pSIMCmdPrm;       /* points to SIM command parameters */
  UBYTE plmn[ACI_LEN_PLMN_SEL_NTRY];/* holds coded plmn id */

  TRACE_FUNCTION ("sAT_PlusCPOL()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }
  pSIMCmdPrm = &cmhPrm[srcId].simCmdPrm;

/*
 *-------------------------------------------------------------------
 * check mode parameter
 *-------------------------------------------------------------------
 */
  switch( mode )
  {
    case( CPOL_MOD_CompactList ):
    case( CPOL_MOD_Insert ):
    case( CPOL_MOD_NotPresent ):
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check format parameter
 *-------------------------------------------------------------------
 */
  switch( format )
  {
    case( CPOL_FRMT_Long ):
    case( CPOL_FRMT_Short ):
    case( CPOL_FRMT_Numeric ):

      if( index EQ ACI_NumParmNotPresent AND !oper )
      {
        pSIMCmdPrm->CPOLfrmt = format;
        return( AT_CMPL );
      }
      break;

    case( CPOL_FRMT_NotPresent ):

      format = pSIMCmdPrm->CPOLfrmt;
      break;

    default:
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * check for write entry
 *-------------------------------------------------------------------
 */
  if( oper )
  {
    /* code plmn id */
    if( ! cmhSIM_GetCodedPLMN( oper, format, plmn ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }

    /* if EF is already read */
    if( EfPLMNselStat EQ EF_STAT_READ )
    {
      if( mode EQ CPOL_MOD_CompactList )
      {
        cmhSIM_CmpctPlmnSel( CPOLSimEfDataLen, CPOLSimEfData );
      }

      if( index NEQ ACI_NumParmNotPresent )
      {
        if( index > (CPOLSimEfDataLen / ACI_LEN_PLMN_SEL_NTRY))
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
          return( AT_FAIL );
        }

        return cmhSIM_UpdPlmnSel( srcId, index, plmn, mode );
      }
      else

        return cmhSIM_FndEmptyPlmnSel( srcId, plmn );
    }
    else
    {
      if( simEntStat.curCmd NEQ AT_CMD_NONE )

        return( AT_BUSY );

      pSIMCmdPrm->CPOLidx  = (UBYTE)(index EQ ACI_NumParmNotPresent)?
                                    NOT_PRESENT_8BIT:index;
      pSIMCmdPrm->CPOLmode = mode;
      pSIMCmdPrm->CPOLact  = CPOL_ACT_Write;
      memcpy( pSIMCmdPrm->CPOLplmn, plmn, ACI_LEN_PLMN_SEL_NTRY );

      simEntStat.curCmd = AT_CMD_CPOL;
      simEntStat.entOwn = srcId;

    /* Implements Measure 150 and 159 */
    return cmhSIM_Req_or_Write_PlmnSel( srcId, ACT_RD_DAT );
    }
  }

/*
 *-------------------------------------------------------------------
 * check for delete entry
 *-------------------------------------------------------------------
 */
  else
  {
    /* check presence of index */
    if( index EQ ACI_NumParmNotPresent )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
      return( AT_FAIL );
    }

    /* if EF is already read */
    if( EfPLMNselStat EQ EF_STAT_READ )
    {
      if( mode EQ CPOL_MOD_CompactList )
      {
        cmhSIM_CmpctPlmnSel( CPOLSimEfDataLen, CPOLSimEfData );
      }

      if( index  > (CPOLSimEfDataLen / ACI_LEN_PLMN_SEL_NTRY) OR
         (index2 > (CPOLSimEfDataLen / ACI_LEN_PLMN_SEL_NTRY) AND
          index2 NEQ ACI_NumParmNotPresent))
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
        return( AT_FAIL );
      }

      if( index2 EQ ACI_NumParmNotPresent )

        return cmhSIM_DelPlmnSel( srcId, index, mode );

      else

        return cmhSIM_ChgPlmnSel( srcId, index, index2 );
    }
    else
    {
      if( simEntStat.curCmd NEQ AT_CMD_NONE )

        return( AT_BUSY );

      pSIMCmdPrm->CPOLidx  = (UBYTE)index;
      pSIMCmdPrm->CPOLidx2 = (index2 NEQ ACI_NumParmNotPresent)?
                             (UBYTE)index2:NOT_PRESENT_8BIT;
      pSIMCmdPrm->CPOLmode = mode;
      pSIMCmdPrm->CPOLact  = CPOL_ACT_Delete;

      simEntStat.curCmd = AT_CMD_CPOL;
      simEntStat.entOwn = srcId;

    /* Implements Measure 150 and 159 */
    return cmhSIM_Req_or_Write_PlmnSel( srcId, ACT_RD_DAT );
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PlusCRSM             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CRSM AT command
            which is responsible for restricted SIM access.

            <cmd>:     access command.
            <fileId>:  file identifier
            <p1>:      parameter 1
            <p2>:      parameter 2
            <p3>:      parameter 3
            <dataLen>: length of data
            <data>:    pointer to data
*/

T_ACI_RETURN sAT_PlusCRSM  ( T_ACI_CMD_SRC  srcId,
                             T_ACI_CRSM_CMD cmd,
                             SHORT          fileId,
                             SHORT          p1,
                             SHORT          p2,
                             SHORT          p3,
                             SHORT          dataLen,
                             UBYTE         *data   )
{
  T_SIM_TRNS_ACC_PRM prm;   /* holds access parameter */

  TRACE_FUNCTION ("sAT_PlusCRSM()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( simEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check command, data, p1, p2 and p3 parameters
 *-------------------------------------------------------------------
 */
  switch( cmd )
  {
    case( CRSM_CMD_UpdBin ):
    case( CRSM_CMD_UpdRec ):

      if( !data OR !dataLen )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }
      /*lint -fallthrough*/

    case( CRSM_CMD_ReadBin ):
    case( CRSM_CMD_ReadRec ):

      if( p1 EQ ACI_NumParmNotPresent OR
          p2 EQ ACI_NumParmNotPresent OR
          p3 EQ ACI_NumParmNotPresent )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }
      break;

    case( CRSM_CMD_GetResp ):
    case( CRSM_CMD_Status ):

      if( p3 EQ ACI_NumParmNotPresent )
        p3 = 0;
      break;

    case( CRSM_CMD_NotPresent ):
    default:
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * check fileId parameter
 *-------------------------------------------------------------------
 */
  if( fileId EQ ACI_NumParmNotPresent AND cmd NEQ CRSM_CMD_Status )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * convert command
 *-------------------------------------------------------------------
 */
  switch( cmd )
  {
    case( CRSM_CMD_UpdBin  ): prm.cmd = SIM_UPDATE_BINARY;break;
    case( CRSM_CMD_UpdRec  ): prm.cmd = SIM_UPDATE_RECORD;break;
    case( CRSM_CMD_ReadBin ): prm.cmd = SIM_READ_BINARY;break;
    case( CRSM_CMD_ReadRec ): prm.cmd = SIM_READ_RECORD;break;
    case( CRSM_CMD_GetResp ): prm.cmd = SIM_GET_RESPONSE;break;
    case( CRSM_CMD_Status  ): prm.cmd = SIM_STATUS;break;
  }

/*
 *-------------------------------------------------------------------
 * access SIM
 *-------------------------------------------------------------------
 */
  simEntStat.curCmd = AT_CMD_CRSM;
  simEntStat.entOwn = srcId;

  prm.reqDataFld = (USHORT)fileId;
  prm.p1         = (UBYTE)p1;
  prm.p2         = (UBYTE)p2;
  prm.p3         = (UBYTE)p3;
  prm.dataLen    = (UBYTE)dataLen;
  prm.transData  = data;

  psaSIM_TrnsSIMAccess( &prm );

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PlusCSIM             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CSIM AT command
            which is responsible for generic SIM access.

*/

T_ACI_RETURN sAT_PlusCSIM  ( T_ACI_CMD_SRC  srcId,
                             USHORT         dataLen,
                             UBYTE         *data    )
{
  T_SIM_TRNS_ACC_PRM prm;   /* holds access parameter */

  TRACE_FUNCTION ("sAT_PlusCSIM()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( simEntStat.curCmd NEQ AT_CMD_NONE )
  {
    return( AT_BUSY );
  }

  if (data[0] EQ GSM_CLASS)
  {
    /* GSM instruction class is not allowed */
    TRACE_EVENT("GSM instruction class is not allowed");
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }
#if 0 /* do we really need the check for the ATP source ? */
#ifdef _TARGET_
  if (ati_is_src_type((UBYTE)srcId, ATI_SRC_TYPE_RIV) EQ FALSE)
  {
    /* don't allow other source type than RIV */
    TRACE_EVENT("other src type than RIV is not allowed");
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }
#endif /*_TARGET_*/
#endif /* */
  if (dataLen > MAX_SIM_TRANSP)
  {
    /* wrong length value */
    TRACE_EVENT("wrong length value");
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * access SIM
 *-------------------------------------------------------------------
 */
  simEntStat.curCmd = AT_CMD_CSIM;
  simEntStat.entOwn = srcId;

  prm.cmd        = SIM_TRANSP_CMD;
  prm.dataLen    = dataLen;
  prm.transData  = data;

  psaSIM_TrnsSIMAccess( &prm );

  return( AT_EXCT );

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PercentPVRF          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %PVRF AT command
            which is responsible to verify a specific PIN.

            <pin>:    string of PIN chars.
            <newpin>: string of PIN chars required if requested PIN is
                      SIM PUK
*/

GLOBAL T_ACI_RETURN sAT_PercentPVRF( T_ACI_CMD_SRC   srcId,
                                     T_ACI_PVRF_TYPE type,
                                     CHAR * pin,
                                     CHAR * newpin )
{
  T_SIM_SET_PRM * pSIMSetPrm;  /* points to SIM parameter set */
  T_ACI_RETURN    retCd;       /* holds return code */

  TRACE_FUNCTION ("sAT_PercentPVRF()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pSIMSetPrm = &simShrdPrm.setPrm[srcId];

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( simEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check for PIN type
 *-------------------------------------------------------------------
 */
  switch( type )
  {
    case( PVRF_TYPE_Pin1 ):
    case( PVRF_TYPE_Pin2 ):
    /*
     *---------------------------------------------------------------
     * PIN 1/2 verify is required
     *---------------------------------------------------------------
     */
      if( pin EQ NULL                 OR
          strlen( pin ) < MIN_PIN_LEN OR
          strlen( pin ) > PIN_LEN        )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      cmhSIM_FillInPIN ( pin, pSIMSetPrm -> curPIN, PIN_LEN );
      pSIMSetPrm -> PINType = (type EQ PVRF_TYPE_Pin1)?
                                  PHASE_2_PIN_1:PHASE_2_PIN_2;
      simEntStat.curCmd     = AT_CMD_PVRF;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn     = srcId;

      if( psaSIM_VerifyPIN() < 0 )  /* verify PIN */
      {
        TRACE_EVENT( "FATAL RETURN psaSIM in %%PVRF" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        return( AT_FAIL );
      }

      retCd = AT_EXCT;
      break;

    case( PVRF_TYPE_Puk1 ):
    case( PVRF_TYPE_Puk2 ):
    /*
     *---------------------------------------------------------------
     * PUK 1/2 verify is required
     *---------------------------------------------------------------
     */
      if( pin EQ NULL                    OR
          newpin EQ NULL                 OR
          strlen( pin ) NEQ PUK_LEN      OR
          strlen( newpin ) < MIN_PIN_LEN OR
          strlen( newpin ) > PIN_LEN        )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      cmhSIM_FillInPIN ( pin,    pSIMSetPrm -> unblkKey, PUK_LEN );
      cmhSIM_FillInPIN ( newpin, pSIMSetPrm -> curPIN,   PIN_LEN );
      pSIMSetPrm -> PINType = (type EQ PVRF_TYPE_Puk1)?
                                  PHASE_2_PUK_1:PHASE_2_PUK_2;
      simEntStat.curCmd     = AT_CMD_PVRF;
      simShrdPrm.owner     = (T_OWN)srcId;
      simEntStat.entOwn     = srcId;

      if( psaSIM_UnblockCard( ) < 0 )  /* verify PIN */
      {
        TRACE_EVENT( "FATAL RETURN psaSIM in %%PVRF" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        return( AT_FAIL );
      }

      retCd = AT_EXCT;
      break;

  default:
    /*
     *---------------------------------------------------------------
     * unexpected PIN state
     *---------------------------------------------------------------
     */
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_DataCorrupt );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  {
  T_ACI_CLOG      cmdLog;      /* holds logging info */

  cmdLog.atCmd                = AT_CMD_PVRF;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = retCd;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sPVRF.srcId   = srcId;
  cmdLog.cmdPrm.sPVRF.type    = type;
  cmdLog.cmdPrm.sPVRF.pin     = pin;
  cmdLog.cmdPrm.sPVRF.newpin  = newpin;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( retCd );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                |
|                                 ROUTINE : sAT_PercentCPRI         |
+-------------------------------------------------------------------+

  PURPOSE : set the CPRI mode for displaying/not displaying
            ciphering indications
*/

GLOBAL T_ACI_RETURN sAT_PercentCPRI( T_ACI_CMD_SRC srcId,
                                     UBYTE mode   )
{
  TRACE_FUNCTION ("sAT_PercentCPRI()");

  if( !cmh_IsVldCmdSrc( srcId ) )
  {
    return( AT_FAIL );
  }

  if (simShrdPrm.ciSIMEnabled NEQ FALSE)   /* Since this function is used by MMI,it just returns AT_CMPL */
  {
    return( AT_CMPL );               /* If CPRI is enabled in the SIM,it just returns AT_CMPL */
  }

  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }
}

#ifdef FF_DUAL_SIM
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                |
|                                 ROUTINE : sAT_PercentSIM         |
+-------------------------------------------------------------------+

  PURPOSE : Select the SIM to be powered on
*/

GLOBAL T_ACI_RETURN sAT_PercentSIM( T_ACI_CMD_SRC srcId,
                                     UBYTE sim_num   )
{
  T_SIM_SET_PRM * pSIMSetPrm; /* points to SIM parameter set */
 
  TRACE_FUNCTION ("sAT_PercentSIM()");

  if( !cmh_IsVldCmdSrc( srcId ) )
  {
    return( AT_FAIL );
  }

  pSIMSetPrm = &simShrdPrm.setPrm[srcId];

  if( simEntStat.curCmd NEQ AT_CMD_NONE )
  {
    TRACE_EVENT("Entity SIM is busy: cannot proceed command...");
    return( AT_BUSY );
  }

  if(CFUNfun EQ CFUN_FUN_Full)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }

  if(sim_num < SIM_NUM_0 OR sim_num > SIM_NUM_2)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }

  pSIMSetPrm->SIM_Selected = sim_num;

  simEntStat.curCmd = AT_CMD_SIM;
  simShrdPrm.owner = (T_OWN)srcId;
  simEntStat.entOwn =  srcId;

  if( psaSIM_SelectSIM() < 0 )   /* select SIM card */
  {
    TRACE_EVENT( "FATAL RETURN psaSIM in %SIM" );
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    return( AT_FAIL );
  }
  return(AT_EXCT);
}
#endif /*FF_DUAL_SIM*/

#ifdef FF_CPHS_REL4
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                |
|                                 ROUTINE : sAT_PercentCFIS         |
+-------------------------------------------------------------------+

  PURPOSE : Set/Read the CFU staus in EF-CFIS file present in SIM
*/

GLOBAL T_ACI_RETURN sAT_PercentCFIS( T_ACI_CMD_SRC srcId,
                                    T_ACI_CFIS_MOD mode, 
                                    UBYTE index,
                                    UBYTE mspId,
                                    UBYTE cfuStat,
                                    CHAR* number, 
                                    T_ACI_TOA* type,
                                    UBYTE cc2_id)
{

  TRACE_FUNCTION ("sAT_PercentCFIS()");

 /*
  *-------------------------------------------------------------------
  * check command source
  *-------------------------------------------------------------------
  */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

 /*
  *-------------------------------------------------------------------
  * check entity status
  *-------------------------------------------------------------------
  */  
  if( simEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

 /*
  *-------------------------------------------------------------------
  * process parameter <mode>
  *-------------------------------------------------------------------
  */
  switch( mode )
  {
    case ( CFIS_MOD_Write ):
    case ( CFIS_MOD_Delete ):
      return cmhSIM_WrCfis (srcId,mode,index,mspId,cfuStat,number, 
                           type,cc2_id);
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                |
|                                 ROUTINE : sAT_PercentMWIS         |
+-------------------------------------------------------------------+

  PURPOSE :Set the Message Waiting status in EF-MWIS file in SIM
*/

GLOBAL T_ACI_RETURN sAT_PercentMWIS( T_ACI_CMD_SRC srcId,
                                     T_ACI_MWIS_MOD mode, 
                                     UBYTE mspId,
                                     T_ACI_MWIS_MWI *mwis)
{

  TRACE_FUNCTION ("sAT_PercentMWIS()");

 /*
  *-------------------------------------------------------------------
  * check command source
  *-------------------------------------------------------------------
  */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

 /*
  *-------------------------------------------------------------------
  * check entity status
  *-------------------------------------------------------------------
  */  
  if( simEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

 /*
  *-------------------------------------------------------------------
  * process parameter <mode>
  *-------------------------------------------------------------------
  */
  switch( mode )
  {
    case ( MWIS_MOD_Write ):
    case ( MWIS_MOD_Delete ):
      return cmhSIM_WrMwis (srcId,mode,mspId,mwis);
      break;
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                |
|                                 ROUTINE : sAT_PercentMBDN         |
+-------------------------------------------------------------------+

  PURPOSE : Set/Read the Mailbox Numbers in EF-MBDN file present in SIM
*/

GLOBAL T_ACI_RETURN sAT_PercentMBDN( T_ACI_CMD_SRC srcId,
                                     T_ACI_MBN_MODE mode, 
                                     UBYTE index,
                                     CHAR* number, 
                                     T_ACI_TOA* type,
                                     UBYTE cc2_id,
                                     T_ACI_PB_TEXT *text)
{

  TRACE_FUNCTION ("sAT_PercentMBDN()");

#ifndef NO_ASCIIZ
  /* convert Text */
  if ( text NEQ NULL )
  {
    UBYTE   tmpBuf[MAX_ALPHA_LEN];
    USHORT  len;
    text->cs = CS_Sim;
    cmh_cvtToDefGsm ( (CHAR*)text->data, (CHAR*)tmpBuf, &len );
    text->len = (UBYTE)len;
    memcpy ( text->data, tmpBuf, text->len );
  }
#endif /* #ifndef NO_ASCIIZ */
 /*
  *-------------------------------------------------------------------
  * check command source
  *-------------------------------------------------------------------
  */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

 /*
  *-------------------------------------------------------------------
  * check entity status
  *-------------------------------------------------------------------
  */  
  if( simEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

 /*
  *-------------------------------------------------------------------
  * process parameter <mode>
  *-------------------------------------------------------------------
  */
  switch( mode )
  {
    case ( MBN_Mode_Write ): 
      if (number EQ NULL)
      { 
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return (AT_FAIL);
      }
    /* lint-fallthrough */
    case ( MBN_Mode_Delete ): 
      return cmhSIM_WrMbdn (srcId, mode, index, number, 
                           type, cc2_id, text);
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }
}
#endif /* FF_CPHS_REL4 */


/* Implements Measure 50 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_analyze_SIMLOCK_STATUS
+------------------------------------------------------------------------------
|  Description : This Function will analyze the SIMLOCK STATUS, passed as an
|                argument to this function and accordingly returns  AT_CMPL
|                AT_FAIL
|
|  Parameters  : status  -  SIMLOCK STATUS that needs to be analyzed
|                 
|  Return      : ACI Return Codes (Either AT_CMPL  or  AT_FAIL)
+------------------------------------------------------------------------------
*/

LOCAL T_ACI_RETURN cmhSIM_analyze_SIMLOCK_STATUS( T_SIMLOCK_STATUS status )
{
  TRACE_FUNCTION ( "cmhSIM_analyze_SIMLOCK_STATUS()" );

  switch ( status )
  {
    case SIMLOCK_ENABLED:
    case SIMLOCK_DISABLED:
      /* success */
      return AT_CMPL;

    case SIMLOCK_BLOCKED:  /* password tried too many times, phone blocked */
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme, CME_ERR_PhoneFail);
      return AT_FAIL;

    case SIMLOCK_LOCKED: /* password wrong */
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme, CME_ERR_WrongPasswd);
      return AT_FAIL;

    default: /* other error */
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
      return AT_FAIL;
  }
}

/* Implements Measure 40 */
/*
+------------------------------------------------------------------------------
|  Function    : sAT_Plus_CAMM_CACM
+------------------------------------------------------------------------------
|  Description : This is the functional counterpart to the +CACM or +CAMM
|                AT commands.
|
|  Parameters  : at_cmd_id  - AT command identifier 
|                srcId      - AT command source identifier
|                value_type - AOC_ACM or AOC_ACMMAX 
|                value      - NULL in case of +CACM
|                             acmmax in case of +CAMM
|                pwd        - password
|
|  Return      : ACI functional return codes 
+------------------------------------------------------------------------------
*/

LOCAL T_ACI_RETURN sAT_Plus_CAMM_CACM ( T_ACI_AT_CMD  at_cmd_id,
                                        T_ACI_CMD_SRC srcId,
                                        UBYTE         value_type,
                                        void          *value,
                                        UBYTE         *pwd )
{
  T_ACI_RETURN  ret = AT_FAIL;

  TRACE_FUNCTION ( "sAT_Plus_CAMM_CACM()" );
  /*
   *-------------------------------------------------------------------
   * check command source
   *-------------------------------------------------------------------
   */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /*
   *-------------------------------------------------------------------
   * send parameters to advice of charge module.
   *-------------------------------------------------------------------
   */
  simEntStat.curCmd     = at_cmd_id;
  simShrdPrm.owner      = (T_OWN)srcId;
  simEntStat.entOwn     = srcId;
  ret = aoc_set_values ( srcId,
                         value_type,
                         value,
                         pwd );

  /*
   *-------------------------------------------------------------------
   * Check return value of aoc_set_values() equal to AT_FAIL,
   * resets simEntStat.curcmd and simEntStat.entown.
   *-------------------------------------------------------------------
   */

  if( ret EQ AT_FAIL )
  {
    simEntStat.curCmd     = AT_CMD_NONE;
    simShrdPrm.owner      = (T_OWN)srcId;
    simEntStat.entOwn     = srcId;
  }

  return( ret );

}




/*==== EOF ========================================================*/
