/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SIMQ
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
|  Purpose :  This module provides the query functions related to the 
|             protocol stack adapter for subscriber identity module.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_SIMQ_C
#define CMH_SIMQ_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_cmh.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifdef UART
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#include "psa.h"
#include "psa_sim.h"
#include "cmh.h"
#include "cmh_sim.h"
#include "cmh_sms.h"

/* To include AciSLockShrd */
#include "aci_ext_pers.h"
#include "aci_slock.h"


#include "aoc.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
#ifdef SIM_PERS
#include "general.h" // inluded for UINT8 compilation error in sec_drv.h
#include "sec_drv.h"
EXTERN T_SEC_DRV_CONFIGURATION *cfg_data;
#endif

/*==== FUNCTIONS ==================================================*/


#ifdef TI_PS_FF_AT_P_CMD_SECS
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : qAT_PercentSECS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SECS? AT command
            which is responsible to query the status of the Security Code.

*/
                             

GLOBAL T_ACI_RETURN qAT_PercentSECS ( T_ACI_CMD_SRC srcId,T_ACI_SECS_STA *status)
{
    T_SIMLOCK_STATUS result = SIMLOCK_FAIL;

    TRACE_FUNCTION ("qAT_PercentSECS()");

    result = aci_ext_personalisation_CS_get_status();

    switch (result)
    {
      case SIMLOCK_ENABLED:
        *status = SECS_STA_Enable;
        return AT_CMPL;
      case SIMLOCK_DISABLED:
       *status = SECS_STA_Disable;
        return AT_CMPL;
      default:
       *status = SECS_STA_NotPresent;
        return AT_FAIL;
    }
}
#endif /* TI_PS_FF_AT_P_CMD_SECS */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCFUN             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CFUN? AT command
            which returns the current setting for phone functionality.

            <fun>:   phone functionality.
*/

GLOBAL T_ACI_RETURN qAT_PlusCFUN  ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_CFUN_FUN *fun )
{
  TRACE_FUNCTION ("qAT_PlusCFUN()");

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
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *fun = CFUNfun;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCPIN             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPIN? AT command
            which returns the current PIN status.

            <code>:   PIN status.
*/

GLOBAL T_ACI_RETURN qAT_PlusCPIN   (T_ACI_CMD_SRC srcId, 
                                    T_ACI_CPIN_RSLT *code)
{
  T_SIM_SET_PRM * pSIMSetPrm; /* points to SIM parameter set */
  #ifdef SIM_PERS
   T_SIMLOCK_STATUS retSlStatus; /* holds return code */
  #endif

  TRACE_FUNCTION ("qAT_PlusCPIN()");

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
 * check SIM status
 *-------------------------------------------------------------------
 */
  if( simShrdPrm.SIMStat NEQ SS_OK AND
      simShrdPrm.SIMStat NEQ SS_BLKD   )
  {
    pSIMSetPrm -> actProc = SIM_INITIALISATION;

    simEntStat.curCmd = AT_CMD_CPIN;
    simShrdPrm.owner = (T_OWN)srcId;
    simEntStat.entOwn = srcId;
    simShrdPrm.PINQuery = 1;

    if( psaSIM_ActivateSIM() < 0 )   /* activate SIM card */
    {
      TRACE_EVENT( "FATAL RETURN psaSIM in +CPIN" );
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
      simShrdPrm.PINQuery = 0;
      return( AT_FAIL );
    }
    return( AT_EXCT );
  }

#ifdef SIM_PERS
/*
 *-------------------------------------------------------------------
 * check PIN status
 *-------------------------------------------------------------------
 */
 if ( simShrdPrm.PINStat EQ PS_RDY)
  {
     simEntStat.curCmd     = AT_CMD_CPIN;
     if(!aci_slock_set_CFG())
     {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext,EXT_ERR_NoMEPD); 
        return( AT_FAIL );
     }
     aci_slock_init();
  	      
     retSlStatus = SIMLOCK_ENABLED;
     AciSLockShrd.check_lock = SIMLOCK_CHECK_PERS;
     AciSLockShrd.cpin_query = SEND_CPIN_REQ_CODE; 
     retSlStatus = aci_slock_checkpersonalisation(AciSLockShrd.current_lock);
     switch(retSlStatus)
      {
        case  SIMLOCK_ENABLED  :
             *code = CPIN_RSLT_SimReady;
             break; 
        case  SIMLOCK_BLOCKED :
              aci_set_cpin_code(AciSLockShrd.current_lock,code); 
              break; 
  			   	
       case  SIMLOCK_WAIT :
               AciSLockShrd.cpin_query = SEND_CPIN_REQ_CODE_RAT;
               return (AT_EXCT);
                           
     }    
   /* OVK Check first if any Personalisations are active */
   TRACE_EVENT_P3("qAT_PlusCPIN: Curr Lock = %d, Status = %d %s", AciSLockShrd.current_lock, AciSLockShrd.status[AciSLockShrd.current_lock],"" );
   return (AT_CMPL);
  
    
  }
 else
#endif
  {
    switch( simShrdPrm.PINStat )
    {
      case( PS_RDY ):

        *code = CPIN_RSLT_SimReady;
        break;

      case( PS_PIN1 ):

        *code = CPIN_RSLT_SimPinReq;
        break;

      case( PS_PUK1 ):

        *code = CPIN_RSLT_SimPukReq;
        break;

      case( PS_PIN2 ):

        *code = CPIN_RSLT_SimPin2Req;
        break;

      case( PS_PUK2 ):

        *code = CPIN_RSLT_SimPuk2Req;
        break;

      default:

        TRACE_EVENT("UNEXPECTED PIN STATUS");
        *code = CPIN_RSLT_NotPresent;
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_DataCorrupt );
        return( AT_FAIL );
    }


  }

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCAOC             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CAOC AT command
            which is responsible to query the current call meter value.
            
            <ccm>   : CCM value.
*/

GLOBAL T_ACI_RETURN qAT_PlusCAOC  ( T_ACI_CMD_SRC srcId,
                                    LONG * ccm)
{
  TRACE_FUNCTION ("qAT_PlusCAOC ()");

/*
 *-------------------------------------------------------------------
 * request value from advice of charge module.
 *-------------------------------------------------------------------
 */  
  aoc_get_values (AOC_CCM, (void *)ccm);
  
  return( AT_CMPL );        
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCACM             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CACM AT command
            which is responsible to query the accumulated call meter value.
            
            <acm>   : ACM value.
*/

GLOBAL T_ACI_RETURN qAT_PlusCACM  ( T_ACI_CMD_SRC srcId,
                                    LONG * acm)
{
  TRACE_FUNCTION ("qAT_PlusCACM ()");

/*
 *-------------------------------------------------------------------
 * request value from advice of charge module.
 *-------------------------------------------------------------------
 */  
  aoc_get_values (AOC_ACM, (ULONG *)acm);
  
  return( AT_CMPL );        
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCAMM             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CAMM AT command
            which is responsible to query the maximum of the 
            accumulated call meter value.
            
            <acmmax>   : ACMMax value.
*/

GLOBAL T_ACI_RETURN qAT_PlusCAMM  ( T_ACI_CMD_SRC srcId,
                                    LONG * acmmax)
{
  TRACE_FUNCTION ("qAT_PlusCAMM ()");

/*
 *-------------------------------------------------------------------
 * request value from advice of charge module.
 *-------------------------------------------------------------------
 */  
  aoc_get_values (AOC_ACMMAX, (ULONG *)acmmax);
  
  return( AT_CMPL );        
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCPUC             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPUC AT command
            which is responsible to query the price per unit and 
            currency.
            
            <cuurency>   : Currency
            <ppu>        : Price per Unit
*/

GLOBAL T_ACI_RETURN qAT_PlusCPUC  ( T_ACI_CMD_SRC srcId,
                                    CHAR          *currency,
                                    CHAR          *ppu)
{
  T_puct puct;

  TRACE_FUNCTION ("qAT_PlusCPUC ()");

/*
 *-------------------------------------------------------------------
 * request value from advice of charge module.
 *-------------------------------------------------------------------
 */  
  aoc_get_values (AOC_PUCT, (void *)&puct);
  
  strcpy ((char *) currency, (char *) puct.currency);
  strcpy ((char *) ppu, (char *) puct.value);

  return( AT_CMPL );        
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : sAT_PlusCIMI             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CIMI AT command
            which is responsible to request the IMSI.

*/

GLOBAL T_ACI_RETURN qAT_PlusCIMI ( T_ACI_CMD_SRC  srcId,
                                   CHAR * imsi )
{
  T_SIM_SET_PRM * pSIMSetPrm; /* points to SIM parameter set */

  TRACE_FUNCTION ("qAT_PlusCIMI()");

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
  switch( simShrdPrm.SIMStat )
  {
    case( SS_OK ):
    /*
     *-----------------------------------------------------------------
     * check if command executable
     *-----------------------------------------------------------------
     */
    if((simShrdPrm.PINStat NEQ PS_RDY) AND (!cmhSMS_checkSIM ()))
    {
      return AT_FAIL;
    }

    /*
     * Check required for CIMI after SIM_REMOVE_IND and SIM_ACTIVATE_IND
     * with SIMStat as SS_OK and PINStat as PS_RDY
     * but before receiving SIM_MMI_INSERT_IND
     */
    else if(simShrdPrm.imsi.c_field EQ 0)
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
      return AT_FAIL; 
    }
    else
    {
      psaSIM_cnvrtIMSI2ASCII( imsi );
      return( AT_CMPL );
    }

    case( NO_VLD_SS ):

      if( simEntStat.curCmd NEQ AT_CMD_NONE ) return( AT_BUSY );

      pSIMSetPrm -> actProc = SIM_INITIALISATION;

      simEntStat.curCmd = AT_CMD_CIMI;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn =  srcId;

      if( psaSIM_ActivateSIM() < 0 )   /* activate SIM card */
      {
        simEntStat.curCmd = AT_CMD_NONE;
        TRACE_EVENT( "FATAL RETURN psaSIM in +CIMI" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
        return( AT_FAIL );
      }

      return( AT_EXCT );

    default:                     /* SIM failure */
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimFail );
      return( AT_FAIL );
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCACM          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CACM AT command
            which is responsible to query the accumulated call meter value
            using PUCT.
            
            <cur>   : currency.
            <val>   : ACM value.
*/

GLOBAL T_ACI_RETURN qAT_PercentCACM( T_ACI_CMD_SRC    srcId,
                                     CHAR            *cur,
                                     CHAR            *val)
{
  T_puct puct;

  TRACE_FUNCTION ("qAT_PercentCACM ()");

/*
 *-------------------------------------------------------------------
 * request value from advice of charge module.
 *-------------------------------------------------------------------
 */  
  aoc_get_values (AOC_ACM_PUCT, (T_puct *)&puct);
  
  strcpy (cur, (char *) puct.currency);
  strcpy (val, (char *) puct.value);

  return( AT_CMPL );        
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCAOC          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CAOC AT command
            which is responsible to query the current call meter value
            using PUCT.
            
            <cur>   : currency.
            <val>   : CCM value.
*/

GLOBAL T_ACI_RETURN qAT_PercentCAOC( T_ACI_CMD_SRC    srcId,
                                     CHAR            *cur,
                                     CHAR            *val)
{
  T_puct puct;

  TRACE_FUNCTION ("qAT_PercentCAOC ()");

/*
 *-------------------------------------------------------------------
 * request value from advice of charge module.
 *-------------------------------------------------------------------
 */  
  aoc_get_values (AOC_CCM_PUCT, (T_puct *)&puct);
  
  strcpy (cur, (char *) puct.currency);
  strcpy (val, (char *) puct.value);

  return( AT_CMPL );        
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCTV           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CTV AT command
            which is responsible to query the current call timer value.
            
            <ctv>   : CTV value.
*/

GLOBAL T_ACI_RETURN qAT_PercentCTV  ( T_ACI_CMD_SRC srcId,
                                      LONG * ctv)
{
  TRACE_FUNCTION ("qAT_PercentCTV ()");

/*
 *-------------------------------------------------------------------
 * request value from advice of charge module.
 *-------------------------------------------------------------------
 */  
  aoc_get_values (AOC_CTV, (void *)ctv);
  
  return( AT_CMPL );        
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PercentRPCT          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %RPCT AT command
            which is responsible to query the raw SIM data for PUCT.
            
            <rpuct>   : PUCT values.
*/

GLOBAL T_ACI_RETURN qAT_PercentRPCT( T_ACI_CMD_SRC    srcId,
                                     T_ACI_RPCT_VAL  *rpuct)
{
  T_puct_raw raw_puct;

  TRACE_FUNCTION ("qAT_PercentRPCT ()");

/*
 *-------------------------------------------------------------------
 * request value from advice of charge module.
 *-------------------------------------------------------------------
 */  
  aoc_get_values (AOC_PUCT_RAW, (T_puct_raw *)&raw_puct);
  
  memcpy(rpuct->currency, raw_puct.currency, MAX_CUR_LEN);
  rpuct->eppu = raw_puct.eppu;
  rpuct->exp  = raw_puct.exp;
  rpuct->sexp = raw_puct.sexp;

  return( AT_CMPL );        
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PercentPVRF          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %PVRF AT command
            which is responsible to query the current counter for
            PIN and PUK.
            
            <pn1Cnt>   : PIN 1 counter.
            <pn2Cnt>   : PIN 2 counter.
            <pk1Cnt>   : PUK 1 counter.
            <pk2Cnt>   : PUK 2 counter.
            <ps1>      : PIN 1 status.
            <ps2>      : PIN 2 status.
*/

GLOBAL T_ACI_RETURN qAT_PercentPVRF( T_ACI_CMD_SRC srcId, 
                                     SHORT        *pn1Cnt,
                                     SHORT        *pn2Cnt,
                                     SHORT        *pk1Cnt,
                                     SHORT        *pk2Cnt,
                                     T_ACI_PVRF_STAT *ps1,
                                     T_ACI_PVRF_STAT *ps2 )
{
  TRACE_FUNCTION ("qAT_PercentPVRF ()");

/*
 *-------------------------------------------------------------------
 * read PIN/PUK counter values
 *-------------------------------------------------------------------
 */  
  if( simShrdPrm.SIMStat NEQ SS_OK AND
      simShrdPrm.SIMStat NEQ SS_BLKD   )
  {
    *pn1Cnt = ACI_NumParmNotPresent;
    *pn2Cnt = ACI_NumParmNotPresent;
    *pk1Cnt = ACI_NumParmNotPresent;
    *pk2Cnt = ACI_NumParmNotPresent;
    *ps1    = PVRF_STAT_NotPresent;
    *ps2    = PVRF_STAT_NotPresent;
  }
  else
  {
    *pn1Cnt = simShrdPrm.pn1Cnt;
    *pn2Cnt = simShrdPrm.pn2Cnt;
    *pk1Cnt = simShrdPrm.pk1Cnt;
    *pk2Cnt = simShrdPrm.pk2Cnt;

    switch( simShrdPrm.pn1Stat )
    {
      case( PS_RDY  ): *ps1 = PVRF_STAT_NotRequired; break;
      case( PS_PIN1 ): *ps1 = PVRF_STAT_Required;    break;
      default:         *ps1 = PVRF_STAT_NotPresent;
    }

    switch( simShrdPrm.pn2Stat )
    {
      case( PS_RDY  ): *ps2 = PVRF_STAT_NotRequired; break;
      case( PS_PIN2 ): *ps2 = PVRF_STAT_Required;    break;
      default:         *ps2 = PVRF_STAT_NotPresent;
    }
  }

  return( AT_CMPL );        
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : qAT_PlusCNUM             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CNUM AT command
            which is responsible for reading the  subscriber number.

            <mode>:   indicates whether reading starts or continues
*/
GLOBAL T_ACI_RETURN qAT_PlusCNUM ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_CNUM_MOD mode )
{
  T_SIM_CMD_PRM * pSIMCmdPrm; /* points to SIM command parameters */

  UBYTE i;                    /* used for counting                */

  TRACE_FUNCTION ("qAT_PlusCNUM()");

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
 * check entity status
 *-------------------------------------------------------------------
 */  
  if( simEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check whether there are more EF to read
 *-------------------------------------------------------------------
 */  
  if ( mode                     EQ CNUM_MOD_NextRead AND
       pSIMCmdPrm -> CNUMActRec EQ CNUMMaxRec            )
  {
    return ( AT_CMPL );
  }

/*
 *-------------------------------------------------------------------
 * process parameter <mode>
 *-------------------------------------------------------------------
 */  
  switch ( mode )
  {
    case ( CNUM_MOD_NewRead  ): 
      pSIMCmdPrm -> CNUMActRec = 1;
      pSIMCmdPrm -> CNUMOutput = 0;
      break;

    case ( CNUM_MOD_NextRead ): 
      pSIMCmdPrm -> CNUMActRec++;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * reset to start of MSISDN list
 *-------------------------------------------------------------------
 */  
  CNUMMsisdnIdx = 0;

/*
 *-------------------------------------------------------------------
 * invalidate contents of MSISDN list
 *-------------------------------------------------------------------
 */  
  for ( i = 0; i < MAX_MSISDN; i++ )
    
    CNUMMsisdn[i].vldFlag = FALSE;

/*
 *-------------------------------------------------------------------
 * request EF MSISDN from SIM
 *-------------------------------------------------------------------
 */  
  return cmhSIM_ReqMsisdn ( srcId, pSIMCmdPrm -> CNUMActRec );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMS           |
| STATE   : code                        ROUTINE : cmhSIM_ReqMsisdn   |
+--------------------------------------------------------------------+

  PURPOSE : This function starts reading of EF MSISDN from SIM.
*/

GLOBAL T_ACI_RETURN cmhSIM_ReqMsisdn ( T_ACI_CMD_SRC srcId,
                                       UBYTE         record )
{
  UBYTE length;

  TRACE_FUNCTION ("cmhSIM_ReqMsisdn()");

  if (record EQ 1)
    length = NOT_PRESENT_8BIT;
  else
    length = CNUMLenEfMsisdn;

  return cmhSIM_ReadRecordEF (srcId, AT_CMD_CNUM, FALSE, NULL, SIM_MSISDN,
                              record, length, NULL, cmhSIM_CnfMsisdn);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMS           |
| STATE   : code                        ROUTINE : cmhSIM_ReqCcp      |
+--------------------------------------------------------------------+

  PURPOSE : This function starts reading of EF CCP from SIM.
*/

GLOBAL T_ACI_RETURN cmhSIM_ReqCcp ( T_ACI_CMD_SRC srcId, 
                                    UBYTE         record )
{
  TRACE_FUNCTION ("cmhSIM_ReqCcp()");

  return cmhSIM_ReadRecordEF (srcId, AT_CMD_CNUM, FALSE, NULL, SIM_CCP, record,
                              ACI_SIZE_EF_CCP, NULL, cmhSIM_CnfCcp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCPOL             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPOL AT command
            which is responsible for reading the preferred operator
            list.
            
            <startIdx>: start index to read from
            <lastIdx>:  buffer for last index read
            <operLst>:  buffer for operator list
            <mode>:     supplemental read mode
*/

GLOBAL T_ACI_RETURN qAT_PlusCPOL  ( T_ACI_CMD_SRC srcId,
                                    SHORT              startIdx,
                                    SHORT             *lastIdx,
                                    T_ACI_CPOL_OPDESC *operLst,
                                    T_ACI_CPOL_MOD     mode )

{
  T_SIM_CMD_PRM * pSIMCmdPrm; /* points to SIM command parameters */

  TRACE_FUNCTION ("qAT_PlusCPOL()");

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
 * check <mode> parameter
 *-------------------------------------------------------------------
 */  
  switch( mode )
  {
    case( CPOL_MOD_NotPresent ):
    case( CPOL_MOD_CompactList ):

      break;

    case( CPOL_MOD_Insert ):
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check if PLMNsel EF has to be read
 *-------------------------------------------------------------------
 */  
  if( startIdx EQ 0 OR EfPLMNselStat EQ EF_STAT_UNKNWN )
  {
    /*
     *-------------------------------------------------------------
     * check entity status
     *-------------------------------------------------------------
     */  
    if( simEntStat.curCmd NEQ AT_CMD_NONE )

      return( AT_BUSY );

    pSIMCmdPrm -> CPOLidx  = (startIdx EQ 0)?1:startIdx;
    pSIMCmdPrm -> CPOLmode = mode;
    pSIMCmdPrm -> CPOLact  = CPOL_ACT_Read;

    /*
     *-------------------------------------------------------------
     * request EF PLMN SEL from SIM
     *-------------------------------------------------------------
     */  
    /* Implements Measure 150 and 159 */
    return cmhSIM_Req_or_Write_PlmnSel( srcId, ACT_RD_DAT );
  }

  /*
   *-------------------------------------------------------------
   * fill PLMN SEL list
   *-------------------------------------------------------------
   */
  if( startIdx * ACI_LEN_PLMN_SEL_NTRY > CPOLSimEfDataLen )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
    return( AT_FAIL );
  }

  if( mode EQ CPOL_MOD_CompactList ) 
  {  
    cmhSIM_CmpctPlmnSel( CPOLSimEfDataLen, CPOLSimEfData );
  }

  *lastIdx = cmhSIM_FillPlmnSelList((UBYTE)startIdx,
                                    pSIMCmdPrm->CPOLfrmt,
                                    operLst,
                                    CPOLSimEfDataLen,
                                    CPOLSimEfData);
  return ( AT_CMPL );

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSQ                  |
| STATE   : code                  ROUTINE : tAT_PlusCPOL             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPOL AT command
            which is responsible for testing the supported preferred 
            operator list length.
            
            <lastIdx>:   maximum number of entries
            <usdNtry>:   number of used entries
*/

GLOBAL T_ACI_RETURN tAT_PlusCPOL  ( T_ACI_CMD_SRC srcId,
                                    SHORT * lastIdx,
                                    SHORT * usdNtry      )
{
  T_SIM_CMD_PRM * pSIMCmdPrm; /* points to SIM command parameters */

  TRACE_FUNCTION ("tAT_PlusCPOL()");

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
 * check if PLMNsel EF has to be read
 *-------------------------------------------------------------------
 */  
  if( EfPLMNselStat EQ EF_STAT_UNKNWN )
  {
    /*
     *-------------------------------------------------------------
     * check entity status
     *-------------------------------------------------------------
     */  
    if( simEntStat.curCmd NEQ AT_CMD_NONE )

      return( AT_BUSY );

    pSIMCmdPrm -> CPOLact = CPOL_ACT_Test;

    /*
     *-------------------------------------------------------------
     * request EF PLMN SEL from SIM
     *-------------------------------------------------------------
     */  
    /* Implements Measure 150 and 159 */
    return cmhSIM_Req_or_Write_PlmnSel( srcId, ACT_RD_DAT );
  }

  /*
   *-------------------------------------------------------------
   * return number of supported entries
   *-------------------------------------------------------------
   */
  *lastIdx = CPOLSimEfDataLen / ACI_LEN_PLMN_SEL_NTRY;

  *usdNtry = cmhSIM_UsdPlmnSelNtry( CPOLSimEfDataLen, CPOLSimEfData );

  return ( AT_CMPL );

}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMQ                |
|                                 ROUTINE : qAT_PercentCPRI         |
+-------------------------------------------------------------------+

 PURPOSE : This is the functional counterpart to the %CPRI AT command
           which is responsible for reading the ciphering indicator 
           mode.
*/
GLOBAL T_ACI_RETURN qAT_PercentCPRI( T_ACI_CMD_SRC srcId,
                                     UBYTE *mode  )
{
  TRACE_FUNCTION ("qAT_PercentCPRI()");

  if( !cmh_IsVldCmdSrc( srcId ) ) 
  { 
    return( AT_FAIL );
  }
  *mode = simShrdPrm.ciSIMEnabled;
  return( AT_CMPL );
}

#ifdef TI_PS_FF_AT_P_CMD_ATR
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMQ                |
|                                 ROUTINE : qAT_PercentATR          |
+-------------------------------------------------------------------+

 PURPOSE : This is the functional counterpart to the %ATR AT command
           which is responsible for reading the SIM phase and ATR 
           (answer to reset information).
*/
GLOBAL T_ACI_RETURN qAT_PercentATR( T_ACI_CMD_SRC  srcId,
                                     UBYTE        *phase,
                                     UBYTE        *atr_len,
                                     UBYTE        *atr_info)
{
  TRACE_FUNCTION ("qAT_PercentATR()");

  if( !cmh_IsVldCmdSrc( srcId ) ) 
  { 
    return( AT_FAIL );
  }

  *phase = simShrdPrm.crdPhs; /* SIM Phase... value is 0xFF if no phase is available*/
  

  *atr_len = simShrdPrm.atr.len; /* ATR length... 0 if no ATR data is available*/
  if (simShrdPrm.atr.len)
  {
    memcpy(atr_info, simShrdPrm.atr.data, simShrdPrm.atr.len);
  }
  
  return( AT_CMPL );
}
#endif /* TI_PS_FF_AT_P_CMD_ATR */

#ifdef FF_DUAL_SIM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMQ                  |
| STATE   : code                  ROUTINE : qAT_PercentSIM             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SIM? AT command
            which returns the currently powered on SIM Number.

            <sim_num>:   sim_number.
*/

GLOBAL T_ACI_RETURN qAT_PercentSIM  ( T_ACI_CMD_SRC  srcId,
                                    UBYTE *sim_num )
{
  TRACE_FUNCTION ("qAT_PercentSIM()");

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
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *sim_num = simShrdPrm.SIM_Powered_on;

  return( AT_CMPL );
}
#endif /*FF_DUAL_SIM*/


#ifdef FF_CPHS_REL4
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : qAT_PercentCFIS          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CNUM AT command
            which is responsible for reading the  subscriber number.

            <mode>:   indicates whether reading starts or continues
*/
GLOBAL T_ACI_RETURN qAT_PercentCFIS ( T_ACI_CMD_SRC  srcId,
                                      UBYTE index )
{


  TRACE_FUNCTION ("qAT_PercntCFIS()");

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

  /* Make sure everytime query is called ,CFISIndex 
     must be initialised to ZERO 
  */
  CFISIndex = 0;

  if(index EQ NOT_PRESENT_8BIT)
  {
    CFISIndex = 1;
    index =1;
  }
  return cmhSIM_ReadRecordEF (srcId, AT_CMD_P_CFIS, SIM_CFIS,
                     index, NOT_PRESENT_8BIT, NULL, cmhSIM_RdCnfCfis);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMS                 |
| STATE   : code                  ROUTINE : qAT_PercentMWIS          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %MWIS AT command
            which is responsible for reading the  MWIS records.
*/

GLOBAL T_ACI_RETURN qAT_PercentMWIS ( T_ACI_CMD_SRC  srcId,
                                      UBYTE mspId )
{

  TRACE_FUNCTION ("qAT_PercntMWIS()");

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

  /* Make sure everytime query is called ,MWISIndex 
     must be initialised to ZERO 
  */
  MWISIndex = 0;

  if(mspId EQ NOT_PRESENT_8BIT)
  {
    MWISIndex = 1;
    mspId =1;
  }
  return cmhSIM_ReadRecordEF (srcId, AT_CMD_P_MWIS, SIM_MWIS,
                     mspId, NOT_PRESENT_8BIT, NULL, cmhSIM_RdCnfMwis);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMQ                 |
| STATE   : code                  ROUTINE : qAT_PercentMBI           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %MBI AT command
            which is responsible for reading the  Mailbox Identifier.

*/
GLOBAL T_ACI_RETURN qAT_PercentMBI ( T_ACI_CMD_SRC  srcId,
                                     UBYTE index )
{

  MBI_Index = 0;  /* Intialise the index value */

  TRACE_FUNCTION ("qAT_PercntMBI()");

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

  /* If index not present then start reading from the first record
     till the maximum record */
  if (index EQ NOT_PRESENT_8BIT)
  {
    MBI_Index = 1;
    index = 1;
  }

  return cmhSIM_ReadRecordEF (srcId, AT_CMD_P_MBI, SIM_MBI,
                     index, NOT_PRESENT_8BIT, NULL, cmhSIM_RdCnfMbi);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIMQ                 |
| STATE   : code                  ROUTINE : qAT_PercentMBDN          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %MBDN AT command
            which is responsible for reading the  Mailbox number.

*/
GLOBAL T_ACI_RETURN qAT_PercentMBDN ( T_ACI_CMD_SRC  srcId,
                                      UBYTE index )
{


  TRACE_FUNCTION ("qAT_PercntMBDN()");

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
  return cmhSIM_ReadRecordEF (srcId, AT_CMD_P_MBDN, SIM_MBDN,
                     index, NOT_PRESENT_8BIT, NULL, cmhSIM_RdCnfMbdn);
}
#endif/* FF_CPHS_REL4 */

/*==== EOF ========================================================*/
