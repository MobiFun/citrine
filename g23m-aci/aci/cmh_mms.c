/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MMS
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
|             protocol stack adapter for mobility management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_MMS_C
#define CMH_MMS_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"

#include "rtc.h"

#ifdef DTI
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#endif

#include "aci_mem.h"

#ifdef FAX_AND_DATA
/* #include "aci_fd.h" */
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_sim.h"
#include "psa_mm.h"
#include "cmh.h"
#include "cmh_sim.h"
#include "cmh_mm.h"

#include "psa_cc.h"
#include "cmh_cc.h"

#ifdef GPRS
  #include "gaci.h"
  #include "gaci_cmh.h"
  #include "psa_gmm.h"
  #include "cmh_gmm.h"
#endif

#ifndef _SIMULATION_
/***************************/
/* TEMPORARY until NVM     */
/* just to get FFs !!!! */
#ifndef GPRS
#define DONT_LET_FFSH_DEF_GPRS
#endif

#include "../../services/ffs/ffs.h"

#include "rx.h"

#include "rtc.h"

#ifdef FF_TIMEZONE
#include "rv/rv_general.h"
#include "rtc/rtc_tz.h"
#endif

/* check whether latter has defined GPRS !!! */
#ifdef DONT_LET_FFSH_DEF_GPRS
#undef GPRS
#endif

#undef DONT_LET_FFSH_DEF_GPRS
/***************************/
#endif /* _SIMULATION_ */


/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
EXTERN void wake_up_rr (void);

#ifdef TI_PS_FF_AT_P_CMD_CTREG
EXTERN BOOL cl_shrd_set_treg_val(T_TREG *treg);
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

#ifndef _SIMULATION_
LOCAL void rx_Cb (drv_SignalID_Type *signal_params);
LOCAL UCHAR rx_sequence_Cb (void* arg);
#endif /* _SIMULATION_ */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PercentBAND          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %BAND AT command
            which is responsible for configuring the way the mobile
            deals with multiband.

            <bandMode>: band switch mode.
            <bandType>: band selection by user: coded in bitfield as following.            
            Bit set to 1 if band is set, 0 otherwise...
       
     BITS:  |    5   |   4   |    3    |    2    |    1   |      
            | GSM850 | E_GSM | GSM1900 | GSM1800 | GSM900 |
*/

GLOBAL T_ACI_RETURN sAT_PercentBAND(T_ACI_CMD_SRC   srcId,
                                    T_ACI_BAND_MODE bandMode,
                                    UBYTE           bandType)
{
  UBYTE ManufacturerBands;
  UBYTE ubyte_dummy;

  TRACE_FUNCTION("sAT_PercentBAND()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* Check Parameter bandMode */
  switch( bandMode )
  {
  case( BAND_MODE_Auto ):
    bandType = 0x00;    /* scan all */
    break;

  case( BAND_MODE_Manual ):
    /* Check Parameter bandType */
    if(cmhMM_getBandSettings(&ubyte_dummy, &ManufacturerBands))
    {
      TRACE_FUNCTION("cmhMM_getBandSettings: data reading from FFS successful");
    }
    else
    {
      TRACE_FUNCTION("cmhMM_getBandSettings: data reading from FFS failed");
    }


    if( !cmhMM_isBandAllowed(bandType, ManufacturerBands) )
    {
      TRACE_EVENT_P1("Band combination %d is not supported by this hardware", bandType);
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
    break;
  
  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  if( !cmhMM_writeSetBand(bandType) )
    return(AT_FAIL);

  /* switch off radio */
  if( CFUNfun EQ CFUN_FUN_Full )
  {
    /* if mobile is on, proceed a soft switch off
     and then switch back on */
    mmEntStat.curCmd = AT_CMD_BAND;

    return( sAT_PlusCFUN( srcId,
                          CFUN_FUN_Disable_TX_RX_RF,
                          CFUN_RST_NotPresent) );
  }
  else
  {
    /* new configuration has been written to Flash and that's it ! */
    return( AT_CMPL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PlusCOPS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +COPS AT command
            which is responsible for selection and deselection of an
            network operator.

            <mode>:   registration mode.
            <format>: format of operator selection
            <oper>:   operator string
*/

GLOBAL T_ACI_RETURN sAT_PlusCOPS ( T_ACI_CMD_SRC srcId,
                                   T_ACI_COPS_MOD mode,
                                   T_ACI_COPS_FRMT format,
                                   CHAR * oper )
{
  return(sat_Plus_Percent_COPS(srcId,
                               mode,
                               format,
                               oper,
                               AT_CMD_COPS));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PercentCOPS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %COPS AT command
            which is responsible for selection and deselection of an
            network operator. %COPS will add, in principle, 
            the following functionality to the standard AT command +COPS: 
           
            -Possibility to query the "service status" in which the phone is currently registered; 
              the possible values will be "full service", "limited service" or  "no service".
            -Possibility to select the "last registered" operator using  %COPS=1. 


            <mode>:   registration mode.
            <format>: format of operator selection
            <oper>:   operator string
*/

GLOBAL T_ACI_RETURN sAT_PercentCOPS ( T_ACI_CMD_SRC srcId,
                                   T_ACI_COPS_MOD mode,
                                   T_ACI_COPS_FRMT format,
                                   CHAR * oper )
{
  return(sat_Plus_Percent_COPS(srcId,
                               mode,
                               format,
                               oper,
                               AT_CMD_P_COPS));
}

#ifdef TI_PS_FF_AT_CMD_WS46
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PlusWS46             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +WS46 AT
            command which is responsible to select wireless network.

            <mode>: network mode.
*/

GLOBAL T_ACI_RETURN sAT_PlusWS46   (T_ACI_CMD_SRC srcId,
                                    T_ACI_WS46_MOD mode )
{

  TRACE_FUNCTION ("sAT_PlusWS46()");

/*
 *-------------------------------------------------------------------
 * process the value parameter
 *-------------------------------------------------------------------
 */
  if( mode NEQ ACI_NumParmNotPresent )
  {
    if( mode NEQ WS46_MOD_Gsm )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

  return( AT_CMPL );
}
#endif /* TI_PS_FF_AT_CMD_WS46 */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : tAT_PercentALS           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %ALS AT command
            which is responsible for testing if ALS mode is present.
            (always if an E-Plus Sim-card is detected or the if the CSP
            field is present on the inserted Sim-card and enables
            ALS mode).

            
*/
GLOBAL T_ACI_RETURN tAT_PercentALS  ( T_ACI_CMD_SRC srcId, T_ACI_ALS_MOD *ALSmode )

{ 
  TRACE_FUNCTION ("tAT_PercentALS()");

  if( !cmh_IsVldCmdSrc( srcId ) )
  {
    return( AT_FAIL );
  }

#if defined(FF_BAT) && defined(_SIMULATION_)
  /* test command %ALS is not used anymore by BAT.lib, change CMD MODE only for host tests */
  if(aci_cmd_src_mode_get(srcId) EQ CMD_MODE_BAT)
  {
    aci_cmd_src_mode_set((UBYTE)srcId, CMD_MODE_ATI);
  }
#endif /* FF_BAT && _SIMULATION_ */  

  /* E-Plus SIM-Card inserted (mcc=0x262, mnc=0x03) ? */
  if (cmhSIM_plmn_is_hplmn(0x262, 0x03F))
  {
    *ALSmode = (T_ACI_ALS_MOD)(ALS_MOD_SPEECH | ALS_MOD_AUX_SPEECH);
    return (AT_CMPL);
  }

  simEntStat.curCmd = AT_CMD_ALS;
  simShrdPrm.owner = (T_OWN)srcId; 
  simEntStat.entOwn =  srcId;

  ccShrdPrm.als_cmd = ALS_CMD_TEST;
  *ALSmode = ALS_MOD_NOTPRESENT;

  cmhCC_checkALS_Support();
  return (AT_EXCT);
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PercentNRG           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %NRG AT command
            which is responsible for the setting of registration mode
            and service mode.

            <regMode>: registration mode.
            <srvMode>: service mode.
*/

GLOBAL T_ACI_RETURN sAT_PercentNRG( T_ACI_CMD_SRC  srcId,
                                    T_ACI_NRG_RGMD regMode,
                                    T_ACI_NRG_SVMD srvMode,
                                    T_ACI_NRG_FRMT oprFrmt,
                                    CHAR          *opr)
{
  T_MM_CMD_PRM * pMMCmdPrm;  /* points to MM command parameters */

 
  TRACE_FUNCTION ("sAT_PercentNRG()");


  pMMCmdPrm = &cmhPrm[srcId].mmCmdPrm;


/*
 *-------------------------------------------------------------------
 * check MM entity status
 *-------------------------------------------------------------------
 */
  if( mmEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );




 /*
 *-------------------------------------------------------------------
 * process the oprFrmt,svrMode and regMode parameters
 *-------------------------------------------------------------------
 */
  switch( oprFrmt )
  {
    case( NRG_FRMT_NotPresent ):
      oprFrmt = pMMCmdPrm -> NRGoprFrmt;
      break;
    case( NRG_FRMT_Long  ):
    case( NRG_FRMT_Short ):
    case( NRG_FRMT_Numeric ):
      break;
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }   

  if( srvMode EQ NRG_SVMD_NotPresent ) 
    srvMode=(T_ACI_NRG_SVMD)pMMCmdPrm->NRGregMode;

  if( regMode EQ NRG_RGMD_NotPresent ) 
    regMode=pMMCmdPrm->NRGregMode;



/* Set the value of the current command executed */
  mmEntStat.curCmd  = AT_CMD_NRG;  


  return cmhMM_OperatorSelect(srcId,regMode,srvMode,oprFrmt,opr);
}

/*
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI                 MODULE  : CMH_MMS                  |
| STATE   : code                    ROUTINE : sAT_PercentCSQ             |
+--------------------------------------------------------------------+

  PURPOSE : This is the function for Signal Quality 
  Param:
  srcId:    command ID;
  CSQmode:  enable(1) or disable(0) CSQ

  Shen,Chao
  Juni.13th, 2003
*/

GLOBAL T_ACI_RETURN sAT_PercentCSQ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_CSQ_MODE CSQmode)
{

  T_MM_CMD_PRM  * pMMCmdPrm;  /* points to MM command parameters */

  if( !cmh_IsVldCmdSrc( srcId ) )
  {
    return( AT_FAIL );
  }

  pMMCmdPrm = &cmhPrm[srcId].mmCmdPrm;

  switch(CSQmode)
  {
    case( CSQ_Enable):
    {
      if ( pMMCmdPrm->CSQworkStat EQ CSQ_Enable )
      {
        TRACE_EVENT("CSQ is already enabled");
      }
      else
      {
        cmhPrm[srcId].mmCmdPrm.CSQworkStat = CSQ_Enable;
        if( (pMMCmdPrm->sIndicationParam.sMmCINDSettings.sCindSignalParam <= CIND_SIGNAL_INDICATOR_LVL0) AND
            (pMMCmdPrm->sIndicationParam.sMmCMERSettings.sCmerIndParam NEQ CMER_INDICATOR_2) )
        { /* rx_Init() not yet called from CIND service */
#ifndef _SIMULATION_
          rx_Init(rx_Cb);
#endif /* _SIMULATION_ */
        }
      }
      break;
    }

    case(CSQ_Disable):
    {
      if ( pMMCmdPrm->CSQworkStat EQ CSQ_Enable )
      {
        cmhPrm[srcId].mmCmdPrm.CSQworkStat = CSQ_Disable;
        if( (pMMCmdPrm->sIndicationParam.sMmCINDSettings.sCindSignalParam <= CIND_SIGNAL_INDICATOR_LVL0) &&
            (pMMCmdPrm->sIndicationParam.sMmCMERSettings.sCmerIndParam NEQ CMER_INDICATOR_2)  )
        { /* CIND service inactive -> no more reading the signal strength necessary */
#ifndef _SIMULATION_
        rx_Init(NULL);
#endif /* _SIMULATION_ */
        }
      }
      break;
    }
    default:
      return(AT_FAIL);
  }
  return( AT_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI                 MODULE  : CMH_MMS                |
| STATE   : code                    ROUTINE : sAT_Plus CIND          |
+--------------------------------------------------------------------+

  PURPOSE : This is the function to setup the indications toward
            terminal
*/

GLOBAL T_ACI_RETURN sAT_PlusCIND( T_ACI_CMD_SRC  srcId,
                                       T_ACI_CIND_SIGNAL_TYPE  sCindSgnalSettings,
                                       T_ACI_CIND_SMSFULL_TYPE sCindSmsFullSettings )
{
  T_ACI_MM_CIND_VAL_TYPE  *pMMCindPrm;
  T_ACI_MM_CMER_VAL_TYPE  *pMMCmerPrm;
  T_ACI_CIND_SIGNAL_TYPE   sOldSignalSetting;

  TRACE_EVENT( "sAT_PlusCIND()" );

  if( !cmh_IsVldCmdSrc( srcId ) )
  {
    return( AT_FAIL );
  }

  pMMCindPrm = &(cmhPrm[srcId].mmCmdPrm.sIndicationParam.sMmCINDSettings);
  pMMCmerPrm = &(cmhPrm[srcId].mmCmdPrm.sIndicationParam.sMmCMERSettings);

  if( sCindSgnalSettings   EQ CIND_SIGNAL_INDICATOR_INVALID OR 
      sCindSmsFullSettings EQ CIND_SMSFULL_INDICATOR_INVALID )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  sOldSignalSetting = pMMCindPrm->sCindSignalParam;
  pMMCindPrm->sCindSignalParam  = sCindSgnalSettings;
  pMMCindPrm->sCindSmsFullParam = sCindSmsFullSettings;

  /* ----------------- signal strength setting part ------------------------ */
  if( sOldSignalSetting <= CIND_SIGNAL_INDICATOR_LVL0 )
  { /* the old (previews) signal setting is off */
    if( (sCindSgnalSettings > CIND_SIGNAL_INDICATOR_LVL0) AND 
        (cmhPrm[srcId].mmCmdPrm.CSQworkStat EQ CSQ_Disable) AND 
        (pMMCmerPrm->sCmerIndParam NEQ CMER_INDICATOR_2) )
    { /* greather LVL0 --> rx_Init must be initialize... */
#ifndef _SIMULATION_
      rx_Init(rx_Cb);
#endif /* _SIMULATION_ */
    }
  }
  else
  { /* the old (previews) signal setting is on */
    if( (sCindSgnalSettings EQ CIND_SIGNAL_INDICATOR_LVL0) AND
        (cmhPrm[srcId].mmCmdPrm.CSQworkStat EQ CSQ_Disable) AND
        (pMMCmerPrm->sCmerIndParam NEQ CMER_INDICATOR_2) )
    { /* LVL0 --> rx_Init must be reset... */
#ifndef _SIMULATION_
        rx_Init(NULL);
#endif /* _SIMULATION_ */
    }
  }
  return(AT_CMPL);
}

/* 
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI                 MODULE  : CMH_MMS                |
| STATE   : code                    ROUTINE : sAT_Plus CMER          |
+--------------------------------------------------------------------+

  PURPOSE : This is the function to setup the buffer control for the 
            indications toward terminal equipment
*/

GLOBAL T_ACI_RETURN sAT_PlusCMER( T_ACI_CMD_SRC  srcId,
                                       T_ACI_CMER_MODE_TYPE sCmerModeSettings,
                                       T_ACI_CMER_IND_TYPE sCmerIndicationSettings,
                                       T_ACI_CMER_BFR_TYPE sCmerBfrSettings )
{
  T_ACI_MM_CMER_VAL_TYPE  *pMMCmerPrm;
  T_ACI_MM_CIND_VAL_TYPE  *pMMCindPrm;
  T_ACI_CMER_IND_TYPE      sOldCmerIndSetting;

  TRACE_EVENT( "sAT_PlusCMER()" );

  if( !cmh_IsVldCmdSrc( srcId ) )
  {
    return( AT_FAIL );
  }

  pMMCmerPrm = &(cmhPrm[srcId].mmCmdPrm.sIndicationParam.sMmCMERSettings);
  pMMCindPrm = &(cmhPrm[srcId].mmCmdPrm.sIndicationParam.sMmCINDSettings);

  sOldCmerIndSetting        = pMMCmerPrm->sCmerIndParam;
  if( sCmerModeSettings NEQ CMER_MODE_INVALID )
  {
    pMMCmerPrm->sCmerModeParam = sCmerModeSettings;
  }
  if( sCmerIndicationSettings NEQ CMER_INDICATOR_INVALID )
  {
    pMMCmerPrm->sCmerIndParam  = sCmerIndicationSettings;
  }
  if( sCmerBfrSettings NEQ CMER_BFR_INVALID )
  {
    pMMCmerPrm->sCmerBfrParam  = sCmerBfrSettings;
  }

  /* ---------------- maybe switch on all indication settings ------------- */
  if( sOldCmerIndSetting NEQ CMER_INDICATOR_2 )
  { /* the old (previews) indicator setting is 'off' */
    if( (sCmerIndicationSettings EQ CMER_INDICATOR_2) AND
        (cmhPrm[srcId].mmCmdPrm.CSQworkStat EQ CSQ_Disable) AND 
        (pMMCindPrm->sCindSignalParam <= CIND_SIGNAL_INDICATOR_LVL0) )
    { /* all indications must be show --> rx_Init must be initialize... */
#ifndef _SIMULATION_
      rx_Init(rx_Cb);
#endif /* _SIMULATION_ */
    }
  }
  else
  { /* the old (previews) signal setting is on */
    if( (sCmerIndicationSettings NEQ CMER_INDICATOR_2) AND
        (cmhPrm[srcId].mmCmdPrm.CSQworkStat EQ CSQ_Disable) AND
        (pMMCindPrm->sCindSignalParam <= CIND_SIGNAL_INDICATOR_LVL0) )
    { /* LVL0 --> rx_Init must be reset... */
#ifndef _SIMULATION_
        rx_Init(NULL);
#endif /* _SIMULATION_ */
    }
  }

  return(AT_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI                 MODULE  : CMH_MMS                  |
| STATE   : code                    ROUTINE : sAT_PercentCSQ             |
+--------------------------------------------------------------------+

  PURPOSE : This is the callback function for Signal Quality 
  Param:
  signal_params   callback function name

  Shen,Chao
  Juni.13th, 2003
*/

#ifndef _SIMULATION_
LOCAL void rx_Cb (drv_SignalID_Type *signal_params)
{
    rx_Status_Type *para = NULL;

    TRACE_EVENT("rx_Cb()");

    ACI_MALLOC(para,sizeof(rx_Status_Type));

    para->actLevel = ((rx_Status_Type *) signal_params->UserData)->actLevel;
    para->gsmLevel = ((rx_Status_Type *) signal_params->UserData)->gsmLevel;
    para->rxQuality = ((rx_Status_Type *) signal_params->UserData)->rxQuality;
#ifdef FF_PS_RSSI
    para->min_access_level = ((rx_Status_Type *) signal_params->UserData)->min_access_level;
#endif
        
    if (!cmh_set_delayed_call (rx_sequence_Cb, (void *)para))
    {
      /* Implements Measure#32: Row 990  */
      TRACE_EVENT("Error, can not set the delayed resequence callback for AT_CMD_CSQ");
      ACI_MFREE(para);
      return ;
    }

    TRACE_EVENT("delayed requence call back requested: 100 ms");
    cmh_start_delayed_call (20);

    return;   
    
}
#endif /* _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI          MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PercentCSQ             |
+--------------------------------------------------------------------+

  PURPOSE : This is the delayed callback function for Signal Quality 
  Shen,Chao
  Juni.13th, 2003
*/
#ifndef _SIMULATION_
LOCAL UCHAR rx_sequence_Cb (void* arg)
{
  int  i;
  UBYTE rssi;
  UBYTE ber;
#ifdef FF_PS_RSSI
  UBYTE min_access_level;
#endif
  T_ACI_CIND_SIGNAL_TYPE sSignalStrength;
  T_ACI_MM_CIND_VAL_TYPE sIndValues;

  struct rx_Status_Type* tmp_p = (struct rx_Status_Type*) arg;
  
  TRACE_EVENT("rx_sequence_Cb()");

  sIndValues.sCindSmsFullParam = CIND_SMSFULL_INDICATOR_INVALID;

  if ( tmp_p->gsmLevel EQ 0xFF OR tmp_p->gsmLevel EQ 0 )
  {
    rssi = ACI_RSSI_FAULT;
    sSignalStrength = CIND_SIGNAL_INDICATOR_LVL0;
  }
  else if ( tmp_p->gsmLevel > 59 )
  {
    rssi = 31;
  }
  else
  {
    rssi = ( tmp_p->gsmLevel / 2 ) + 2;
  }

  if ( tmp_p->rxQuality EQ RX_QUAL_UNAVAILABLE )
  {
    ber = ACI_BER_FAULT;
  }
  else
  {
    ber = tmp_p->rxQuality;
  }  

#ifdef FF_PS_RSSI
  if ( tmp_p->min_access_level EQ RX_ACCE_UNAVAILABLE)
  {
    min_access_level = ACI_MIN_RXLEV_FAULT;
  }
  else
  {
    min_access_level = tmp_p->min_access_level;
  }
#endif

  for( i = CMD_SRC_LCL; i < CMD_SRC_MAX; i++)
  {
    if (cmhPrm[i].mmCmdPrm.CSQworkStat EQ CSQ_Enable)
    {
#ifdef FF_PS_RSSI
      R_AT (RAT_CSQ, (T_ACI_CMD_SRC)i) (rssi, ber, tmp_p->actLevel, min_access_level);
#else
      R_AT (RAT_CSQ, (T_ACI_CMD_SRC)i) (rssi,ber,tmp_p->actLevel);
#endif
    }
    /* ==================== process the service of +CIEV ========================== */
    sSignalStrength = CIND_SIGNAL_INDICATOR_LVL0;
    if( tmp_p->gsmLevel >= 11 ) { sSignalStrength = CIND_SIGNAL_INDICATOR_LVL1; };
    if( tmp_p->gsmLevel >= 21 ) { sSignalStrength = CIND_SIGNAL_INDICATOR_LVL2; };
    if( tmp_p->gsmLevel >= 32 ) { sSignalStrength = CIND_SIGNAL_INDICATOR_LVL3; };
    if( tmp_p->gsmLevel >= 42 ) { sSignalStrength = CIND_SIGNAL_INDICATOR_LVL4; };
    if( tmp_p->gsmLevel >= 52 ) { sSignalStrength = CIND_SIGNAL_INDICATOR_LVL5; };
    sIndValues.sCindSignalParam = sSignalStrength;
    sIndValues.sCindSmsFullParam = CIND_SMSFULL_INDICATOR_INVALID;
    if( ((cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCINDSettings.sCindSignalParam > CIND_SIGNAL_INDICATOR_LVL0) AND
         (cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCINDSettings.sCindSignalParam >= sIndValues.sCindSignalParam) AND
          (cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCMERSettings.sCmerIndParam EQ CMER_INDICATOR_2)) OR 
        ((cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCINDSettings.sCindSignalParam < sIndValues.sCindSignalParam) AND
          (cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCMERSettings.sCmerIndParam EQ CMER_INDICATOR_1)) )
    {
      TRACE_EVENT("send +CIEV");
      R_AT (RAT_CIEV, (T_ACI_CMD_SRC)i) ( sIndValues, cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCMERSettings );
    }
  }

  if (!cmh_set_delayed_call (NULL, NULL))
  { /* reset this callback pointer -> will be set again via rx_Cb */
    /* Implements Measure#32: Row 991  */
    TRACE_EVENT("Can not reset the delayed resequence callback for AT_CMD_CSQ");
    return FALSE;
  }
  ACI_MFREE(tmp_p);

  return TRUE;    
}
#endif /* _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PercentCHPL          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CHPL AT command
            which will return the list of entries in EF_HPLMNwAcT.

            <mode>:   registration mode.
            <format>: format of operator selection
            <oper>:   operator string
*/

GLOBAL T_ACI_RETURN sAT_PercentCHPL ( T_ACI_CMD_SRC srcId,
                                      T_ACI_OPER_NTRY *oper )
{
  T_OPER_ENTRY   plmnDesc;
  BOOL           found;
  SHORT          mccBuf;
  SHORT          mncBuf;

  TRACE_FUNCTION ("sAT_PercentCHPL()");

/*
 *-------------------------------------------------------------------
 * check whether SIM is ready 
 *-------------------------------------------------------------------
 */
  if (!cmhSIM_CheckSimStatus ())
  {
    return AT_FAIL;
  }

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("sAT_PercentCHPL(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }
  if(!cmhMM_GetActingHPLMN(&mccBuf, &mncBuf))/* Enhancement Acting HPLMN*/
  {
    cmhSIM_GetHomePLMN (&mccBuf, &mncBuf);
  }

  found = cmhMM_FindPLMN (&plmnDesc, mccBuf, mncBuf, NOT_PRESENT_16BIT, FALSE);
  if (!found)
  {
    TRACE_EVENT_P2("sAT_PercentCHPL(): MCC/MN not found in database (0x%X, 0x%X", mccBuf, mncBuf);
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal);
    return( AT_FAIL );
  }

  strcpy (oper->longName, plmnDesc.longName);
  strcpy (oper->shrtName, plmnDesc.shrtName);
  oper->mcc = plmnDesc.mcc;
  oper->mnc = plmnDesc.mnc;
  oper->pnn = plmnDesc.pnn;
  oper->long_len = plmnDesc.long_len;
  oper->shrt_len = plmnDesc.shrt_len;

  return (AT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PlusCTZR             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CTZR AT command
            which will set the status of CTZRmode, indicating whether time
            zone change rep[orting is enabled or disabled.

            <on/off>:   Indicates whether time zone reporting is enabled or disabled.
*/

GLOBAL T_ACI_RETURN sAT_PlusCTZR ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CTZR_MODE mode)
{
  TRACE_FUNCTION ("sAT_PlusCTZR()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("sAT_PlusCTZR(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the mode parameter
 *-------------------------------------------------------------------
 */
  switch (mode)
  {
    case CTZR_MODE_OFF:
    case CTZR_MODE_ON:
       cmhPrm[srcId].mmCmdPrm.CTZRMode = mode;
       break;

    default:
       TRACE_EVENT_P1("sAT_PlusCTZR(): Invalid mode: %d", mode);
       ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
       return( AT_FAIL );
  }
  
  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PlusCTZU             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CTZU AT command
            which will set the status of CTZUmode, indicating whether automatic time
            zone update is enabled or disabled.

            <on/off>:   Indicates whether automatic time zone update is enabled or disabled.
*/

GLOBAL T_ACI_RETURN sAT_PlusCTZU ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CTZU_MODE mode)
{
  TRACE_FUNCTION ("sAT_PlusCTZU()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("sAT_PlusCTZU(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the mode parameter
 *-------------------------------------------------------------------
 */
  switch (mode)
  {
    case CTZU_MODE_OFF:
    case CTZU_MODE_ON:
       cmhPrm[srcId].mmCmdPrm.CTZUMode = mode;
       break;

    default:
       TRACE_EVENT_P1("sAT_PlusCTZU(): Invalid mode: %d", mode);
       ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
       return( AT_FAIL );
  }
  
  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PlusCCLK             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CCLK AT command
            which will set the real time clock of the ME
*/

GLOBAL T_ACI_RETURN sAT_PlusCCLK ( T_ACI_CMD_SRC srcId,
                                   T_ACI_RTC_DATE *date_s,
                                   T_ACI_RTC_TIME *time_s,
                                   int timeZone
                                 )
{
#ifndef _SIMULATION_
  UBYTE  ret;
#endif /* _SIMULATION_ */

  TRACE_FUNCTION ("sAT_PlusCCLK()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("sAT_PlusCCLK(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * Set the time in the RTC 
 *-------------------------------------------------------------------
 */
#ifndef _SIMULATION_
  ret = rtc_set_time_date( (T_RTC_DATE *) date_s, (T_RTC_TIME *) time_s);

  switch (ret)
  {
    case 0:                     /* RVF_OK. Date and time set ok*/
#ifndef FF_TIMEZONE
      return( AT_CMPL );
#else  /* FF_TIMEZONE */
      ret = RTC_SetCurrentTZ((T_RTC_TZ)timeZone); /* Set current timezone now time and date are set.*/
      switch (ret)
      {
        case 0:                /* RVF_OK. RTC current TZ set ok*/
          return( AT_CMPL );

        default:
          TRACE_EVENT_P1("sAT_PlusCCLK(): ERROR1: %d", ret);  /* RTC setting failed */
          return( AT_FAIL );
      }
#endif /* FF_TIMEZONE */

    default:     /* RVF_NOT_READY or RVF_INTERNAL ERROR or RVF_INVALID_PARAMETER */
      TRACE_EVENT_P1("sAT_PlusCCLK(): ERROR2: %d", ret);  /* Date and time setting failed */
      return( AT_FAIL );
  }
#else /* _SIMULATION_ */
  return( AT_CMPL );
#endif /* _SIMULATION_ */

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PercentCTZV             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CTZV AT command
            which will set the status of PCTZVmode, indicating whether time and date report
            is enabled or disabled.

            <on/off>:   Indicates whether time and date report is enabled or disabled.
*/

GLOBAL T_ACI_RETURN sAT_PercentCTZV ( T_ACI_CMD_SRC srcId,
                                   T_ACI_PCTZV_MODE mode)
{
  TRACE_FUNCTION ("sAT_PercentCTZV()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("sAT_PercentCTZV(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the mode parameter
 *-------------------------------------------------------------------
 */
  switch (mode)
  {
    case PCTZV_MODE_OFF:
    case PCTZV_MODE_ON:
       cmhPrm[srcId].mmCmdPrm.PCTZVMode = mode;
       break;

    default:
       TRACE_EVENT_P1("sAT_PercentCTZV(): Invalid mode: %d", mode);
       ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
       return( AT_FAIL );
  }
  
  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PercentCNIV                |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CNIV AT command
            which will set the status of CNIVmode, indicating whether time and date report
            is enabled or disabled.

            <on/off>:   Indicates whether time and date report is enabled or disabled.
*/

GLOBAL T_ACI_RETURN sAT_PercentCNIV ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CNIV_MODE mode)
{
  TRACE_FUNCTION ("sAT_PercentCNIV()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("sAT_PercentCNIV(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the mode parameter
 *-------------------------------------------------------------------
 */
  switch (mode)
  {
    case CNIV_MODE_OFF:
    case CNIV_MODE_ON:
       cmhPrm[srcId].mmCmdPrm.CNIVMode = mode;
       break;

    default:
       TRACE_EVENT_P1("sAT_PercentCNIV(): Invalid mode: %d", mode);
       ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
       return( AT_FAIL );
  }
  
  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PercentCWUP          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CWUP AT command
                   which will force RR to trigger a power campaign.

*/

GLOBAL T_ACI_RETURN sAT_PercentCWUP ( T_ACI_CMD_SRC srcId, T_ACI_CWUP_TYPE type )
{
  TRACE_FUNCTION ("sAT_PercentCWUP()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("sAT_PercentCWUP(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( mmEntStat.curCmd NEQ AT_CMD_NONE )
  {
    TRACE_EVENT("sAT_PercentCWUP(): MM is busy");    
    return( AT_BUSY );
  }

  wake_up_rr();
  
  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PlusCOPS             |
+--------------------------------------------------------------------+

  PURPOSE : This function encapsulates the functionality of +COPS and %COPS command
             
            <srcId>:  Source Id
            <mode>:   registration mode.
            <format>: format of operator selection
            <oper>:   operator string
            <cmd>:    command
*/

GLOBAL T_ACI_RETURN sat_Plus_Percent_COPS ( T_ACI_CMD_SRC srcId,
                                   T_ACI_COPS_MOD mode,
                                   T_ACI_COPS_FRMT format,
                                   CHAR * oper,
                                   T_ACI_AT_CMD cmd)
{
  T_MM_CMD_PRM * pMMCmdPrm;  /* points to MM command parameters */

  T_ACI_NRG_RGMD regMode; /*NRG Registration mode */
  T_ACI_NRG_FRMT regFormat; 
  T_ACI_NRG_SVMD  svrMode;

  TRACE_FUNCTION ("sat_Plus_Percent_COPS()");

  pMMCmdPrm = &cmhPrm[srcId].mmCmdPrm;
  mmShrdPrm.COPSmodeBeforeAbort = mmShrdPrm.COPSmode;
  
 /*
  *  check MM entity status
  */
  if( mmEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

 /*
  *convert  format to NRG type and set the COPS format value.
  */

  if( format EQ COPS_FRMT_NotPresent )
    format = pMMCmdPrm -> COPSfrmt;

  switch( format )
  {
    case(    COPS_FRMT_NotPresent): 
      regFormat = NRG_FRMT_NotPresent; 
      break;
    case(   COPS_FRMT_Numeric ): 
      regFormat =NRG_FRMT_Numeric ; 
      break;
    case( COPS_FRMT_Short):
      regFormat = NRG_FRMT_Short; 
      break;
    case( COPS_FRMT_Long):
      regFormat = NRG_FRMT_Long; 
      break;
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  pMMCmdPrm -> COPSfrmt = format;

 /*
  * The function cmhMM_OperatorSelect() is using the NRG types. Therefore the following conversion is required.
  * Also the appropiate value for Service Mode, required by cmhMM_OperatorSelect(),  is set. 
  */

  if( mode EQ COPS_MOD_NotPresent )
    mode = mmShrdPrm.COPSmode;

  switch( mode )
  {
    case( COPS_MOD_NotPresent   ): 
      regMode = NRG_RGMD_NotPresent; 
      svrMode = NRG_SVMD_NotPresent;
      break;
    case(  COPS_MOD_Auto  ): 
      regMode =NRG_RGMD_Auto ; 
      svrMode = NRG_SVMD_Full;    /* Only Full Service makes sense for Automatic Mode */
      break;
    case(  COPS_MOD_Man):
      regMode = NRG_RGMD_Manual; 
      svrMode = NRG_SVMD_Full;   /* Only Full Service makes sense for Manual Mode */ 
      break;
    case(  COPS_MOD_Dereg):
      regMode = NRG_RGMD_Dereg; 
      svrMode = NRG_SVMD_Limited;    
      break;
    case(   COPS_MOD_Both ): 
      regMode = NRG_RGMD_Both; 
      svrMode = NRG_SVMD_Full;  /* Only Full Service makes sense for the Both Mode */
      break;
    case(COPS_MOD_SetOnly):  /*This mode can be fully handled in this function */
      /* DO NOT SET mmShrdPrm.COPSmode !!! */
      pMMCmdPrm -> COPSfrmt = format;
      return AT_CMPL;
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  mmShrdPrm.COPSmode = mode;

  /*Set the value of the current command executed */
  mmEntStat.curCmd  = cmd;  

  return cmhMM_OperatorSelect(srcId,regMode,svrMode,regFormat,oper);
}

#ifdef TI_PS_FF_AT_P_CMD_CTREG
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : sAT_PercentCTREG         |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CTREG AT command
            which will update the two tables present in common shared
            location.

*/

GLOBAL T_ACI_RETURN sAT_PercentCTREG ( T_ACI_CMD_SRC srcId, T_TREG *treg )
{
  BOOL ret;

  TRACE_FUNCTION ("sAT_PercentCTREG()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("sAT_PercentCTREG(): Invalid source: %d", srcId);
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  ret = cl_shrd_set_treg_val(treg);

  if(ret EQ TRUE)
  {
    return( AT_CMPL );
  }
  return(AT_FAIL);
}
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

/*==== EOF ========================================================*/
