/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MMT
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
|  Purpose :  This module provides the test functions related to the
|             protocol stack adapter for mobility management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_MMT_C
#define CMH_MMT_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"


#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_mm.h"
#include "cmh.h"
#include "cmh_mm.h"


#ifdef GPRS 
#ifdef DTI
#include "dti_conn_mng.h"
#endif
#include "gaci.h"
#include "gaci_cmh.h"
#include "psa_gmm.h"
#include "cmh_gmm.h"
#endif

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
LOCAL T_ACI_RETURN get_available_network_list  ( T_ACI_CMD_SRC srcId,
                                                  T_ACI_AT_CMD cmd,
                                                  SHORT startIdx,
                                                  SHORT * lastIdx,
                                                  T_ACI_COPS_OPDESC * operLst);
LOCAL T_ACI_RETURN tAT_Plus_Percent_COPS  ( T_ACI_AT_CMD at_cmd_id,
                                            T_ACI_CMD_SRC srcId,
                                            SHORT startIdx,
                                            SHORT * lastIdx,
                                            T_ACI_COPS_OPDESC * operLst );

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMQ                  |
| STATE   : code                  ROUTINE : qAT_PercentBAND             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %BAND=? AT command
            which returns the current multiband configuration.

            <MaxBandMode>:  highest value of supported band switch modes.
            <AllowedBands>: bitfield of supported bands (manufacturer defined).
*/

GLOBAL T_ACI_RETURN tAT_PercentBAND(T_ACI_CMD_SRC    srcId,
                                    T_ACI_BAND_MODE  *MaxBandMode,
                                    UBYTE            *AllowedBands)
{
  UBYTE dummy;

  TRACE_FUNCTION ("tAT_PercentBAND()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* process MaxBandMode parameter */
  if(MaxBandMode NEQ NULL)
  {
    *MaxBandMode = BAND_MODE_Manual;  /* it is currently the highest value for BandMode */ 
  }
  
  /* process BandTypeList parameter */
  if( AllowedBands NEQ NULL )
  {
    if(cmhMM_getBandSettings(&dummy, AllowedBands))
    {
      TRACE_FUNCTION("cmhMM_getBandSettings: data reading from FFS successful");
    }
    else
    {
      TRACE_FUNCTION("cmhMM_getBandSettings: data reading from FFS failed");
    }
  }
  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMT                  |
| STATE   : code                  ROUTINE : tAT_PlusCOPS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +COPS=? AT command
            which is responsible to test for all available network
            operators.

            <startIdx>: Start index for reading the list.
                        Must be zero.
            <lastIdx> : Last index buffers the last read index of the list.
                        Not used, maintained for compatibility reasons.
            <operLst> : List buffer to copy MAX_OPER entries into.
                        Not used, maintained for compatibility reasons.

*/

GLOBAL T_ACI_RETURN tAT_PlusCOPS  ( T_ACI_CMD_SRC srcId,
                                    SHORT startIdx,
                                    SHORT * lastIdx,
                                    T_ACI_COPS_OPDESC * operLst)
{
  /* Implements Measure 29 */
  return ( tAT_Plus_Percent_COPS( AT_CMD_COPS, srcId, startIdx,
                                  lastIdx, operLst ) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMT                  |
| STATE   : code                  ROUTINE : tAT_PercentNRG             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %NRG=?
*/

GLOBAL T_ACI_RETURN tAT_PercentNRG  ( T_ACI_CMD_SRC srcId, T_ACI_NRG *NRG_options)
{
  
  TRACE_FUNCTION ("tAT_PercentNRG()");

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
  if( mmEntStat.curCmd NEQ AT_CMD_NONE )
  {
    TRACE_EVENT("mmEntStat.curCmd NEQ AT_CMD_NONE");
    return( AT_BUSY );
  }
/*
 * The response depends upon the value of the PLMN Mode bit.
 */

  NRG_options->reg_mode = NRG_REG_NotPresent;
  NRG_options->srv_mode = NRG_SRV_NotPresent;
  NRG_options->opr_frmt = NRG_OPR_NotPresent;

  NRG_options->srv_mode = (T_ACI_NRG_SRV)
  	      (NRG_options->srv_mode | NRG_SRV_Full | NRG_SRV_Limited | NRG_SRV_NoSrv | NRG_SRV_SetRegModeOnly);
  NRG_options->opr_frmt = (T_ACI_NRG_OPR)
  	      (NRG_options->opr_frmt | NRG_OPR_Long | NRG_OPR_Short | NRG_OPR_Numeric);
  NRG_options->reg_mode = (T_ACI_NRG_REG)(NRG_options->reg_mode | NRG_REG_Auto);
  
  if(cmhSIM_isplmnmodebit_set())
  {
    NRG_options->reg_mode = (T_ACI_NRG_REG)(NRG_options->reg_mode | NRG_REG_Manual | NRG_REG_Both);
  }
    
  return(AT_CMPL);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMT                  |
| STATE   : code                  ROUTINE : tAT_PercentCOPS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %COPS=? AT command
            which is responsible to test for all available network
            operators.

            <startIdx>: start index for reading the list.
            <lastIdx> : last index buffers the last read index of the
                        list
            <operLst> : list buffer to copy MAX_OPER entries into.
*/

GLOBAL T_ACI_RETURN tAT_PercentCOPS  ( T_ACI_CMD_SRC srcId,
                                    SHORT startIdx,
                                    SHORT * lastIdx,
                                    T_ACI_COPS_OPDESC * operLst)
{
  /* Implements Measure 29 */
  return ( tAT_Plus_Percent_COPS( AT_CMD_P_COPS, srcId, startIdx,
                                  lastIdx, operLst ) );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMT                  |
| STATE   : code                  ROUTINE : get_available_network_list|
+--------------------------------------------------------------------+

  PURPOSE : This is common function for +COPS and %COPS test functions.
*/

LOCAL T_ACI_RETURN get_available_network_list  ( T_ACI_CMD_SRC srcId,
                                                  T_ACI_AT_CMD cmd,
                                                  SHORT startIdx,
                                                  SHORT * lastIdx,
                                                  T_ACI_COPS_OPDESC * operLst)
{
  T_ACI_RETURN   retCd;        /* holds return code */

  TRACE_FUNCTION ("get_available_network_list()");

/*
 *-------------------------------------------------------------------
 * process the start index parameter in case of a network search
 *-------------------------------------------------------------------
 */
  if(!cmhSIM_isplmnmodebit_set())
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return( AT_FAIL );
  }
  
  if( startIdx EQ 0 )
  {
    mmEntStat.curCmd  = cmd;
    mmShrdPrm.owner = (T_OWN)srcId;	
    mmEntStat.entOwn = srcId;

#if defined (GPRS) AND defined (DTI) 
    if( psaG_MM_CMD_NET_SRCH ( ) < 0 )  /* search for network */
#else
    if( psaMM_NetSrch () < 0 )  /* search for network */
#endif
    {
      TRACE_EVENT( "FATAL RETURN psaMM_NetSrch in COPS" );
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
      return( AT_FAIL );
    }

    retCd = AT_EXCT;
  }
  else
  {
    /*
     * For g23m, it makes no sense to support a startIdx different from zero
     * as MM only delivers GMMREG_MAX_PLMN_ID PLMNs which is identical to
     * MAX_PLMN_ID. So the MMI already gets all the desired information 
     * by the callback rAT_PlusCOPS().
     * For other programs like Neptune this maybe different, this is for
     * further study.
     */
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  {
  T_ACI_CLOG     cmdLog;       /* holds logging info */
  cmdLog.atCmd                  = cmd;
  cmdLog.cmdType                = CLOG_TYPE_Test;
  cmdLog.retCode                = retCd;
  cmdLog.cId                    = ACI_NumParmNotPresent;
  cmdLog.sId                    = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.tCOPS.srcId     = srcId;
  cmdLog.cmdPrm.tCOPS.startIdx  = startIdx;
  cmdLog.cmdPrm.tCOPS.lastIdx   = lastIdx;
  cmdLog.cmdPrm.tCOPS.operLst   = operLst;

  rAT_PercentCLOG( &cmdLog );
  }
#endif
  return( retCd );
}

/* Implements Measure 29 */
/*
+------------------------------------------------------------------------------
|  Function    : tAT_Plus_Percent_COPS
+------------------------------------------------------------------------------
|  Description : This is the functional counterpart to the +COPS=? or %COPS=?
|                AT command which is responsible to test for all available 
|                network.
|                Argument at_cmd_id will be used to pass AT_CMD_COPS or 
|                AT_CMD_P_COPS in case of AT command +COPS=? or %COPS=?.
|
|
|  Parameters  : at_cmd_id  - AT command identifier 
|                srcId      - AT command source identifier
|                startIdx   - start index for reading the list.
|                lastIdx    - last index buffers the last read index of the
|                             list
|                operLst    - list buffer to copy MAX_OPER entries into.
|
|  Return      : ACI functional return codes 
+------------------------------------------------------------------------------
*/

LOCAL T_ACI_RETURN tAT_Plus_Percent_COPS  ( T_ACI_AT_CMD at_cmd_id,
                                            T_ACI_CMD_SRC srcId,
                                            SHORT startIdx,
                                            SHORT * lastIdx,
                                            T_ACI_COPS_OPDESC * operLst )
{
  TRACE_FUNCTION ( "tAT_Plus_Percent_COPS()" );

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
  if( mmEntStat.curCmd NEQ AT_CMD_NONE )
  {
    return( AT_BUSY );
  }

  /*
   *-------------------------------------------------------------------
   * process the start index parameter in case of a network search
   *-------------------------------------------------------------------
   */

  return( get_available_network_list( srcId, at_cmd_id, startIdx,
                                      lastIdx, operLst ) );

}

/*==== EOF ========================================================*/
