/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MMIS
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
|             protocol stack adapter for the man machine interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_MMIQ_C
#define CMH_MMIQ_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "psa_mmi.h"
#include "cmh_mmi.h"
#include "pcm.h"

#ifdef UART
#include "dti.h"
#include "dti_conn_mng.h"
#endif


#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */
#include "psa.h"
#include "psa_sim.h"
#include "cmh.h"
#include "cmh_sim.h"
#include "aoc.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/* Implements Measure#32: Row 971, 976, 1023 & 1072 */
const char * const ef_clng_id = EF_CLNG_ID;
/* Implements Measure#32: Row 972, 1024 & 1041 */
char * const au_str = "au";

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMIQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCLAN             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CLAN AT command
            which is responsible for query supporetd language code in 
            ME.
*/

GLOBAL T_ACI_RETURN qAT_PlusCLAN ( T_ACI_CMD_SRC srcId,
                                   T_ACI_LAN_SUP* lngCode )              
{
/* Implements Measure#32: Row 971 */
  pcm_FileInfo_Type fileInfo;
  EF_CLNG lng;
  T_SIM_CMD_PRM * pSIMCmdPrm; /* points to SIM command parameters  */
/* Implements Measure#32: Row 972 */

  TRACE_FUNCTION ("qAT_PlusCLAN()");

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
 *   read supported language from ME 
 *-------------------------------------------------------------------
 */   
/* Implements Measure#32: Row 971 */
   if (pcm_GetFileInfo ( ( UBYTE* ) ef_clng_id, &fileInfo) NEQ PCM_OK)
   {
     ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
     return( AT_FAIL );
   }
   else      
   { 
     
/* Implements Measure#32: Row 971 */
     if ( pcm_ReadFile ( (UBYTE*)ef_clng_id,
                         fileInfo.FileSize,
                         (UBYTE*) &lng,
                         &fileInfo.Version) EQ PCM_OK )
     {
       memcpy(lngCode->str, &lng.data[0], 2);
     }
     else
     {
       ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_MemFail );
       return( AT_FAIL );
     } 
   }
/*
 *-------------------------------------------------------------------
 *  Read EF ELP or LP from the sim if Automatic language is selected 
 *-------------------------------------------------------------------
 */  
 
/* Implements Measure#32: Row 972 */
  if (!strcmp(lngCode->str, au_str))
  {
    /*
     *-------------------------------------------------------------
     * check entity status
     *-------------------------------------------------------------
     */  
    if( simEntStat.curCmd NEQ AT_CMD_NONE )
      return( AT_BUSY );

    pSIMCmdPrm -> CLANact  = CLAN_ACT_Read;

    /*
     *-------------------------------------------------------------
     * request EF ELP from SIM
     *-------------------------------------------------------------
     */  
    /* Implements Measure 150 and 159 */
    return cmhSIM_ReqLanguage_LP_or_ELP ( srcId, SIM_ELP );
  }     

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMIQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCLAE             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CLAE? AT command
            which returns the current setting of mode .

            <mode>: Enable or Disable the unsolicited result code,  
                    when the language in the ME is changend.
           
*/

GLOBAL T_ACI_RETURN qAT_PlusCLAE (T_ACI_CMD_SRC  srcId,
                                  T_ACI_CLAE_MOD *mode)
                                
{
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameter */ 
  

  TRACE_FUNCTION ("qAT_PlusCLAE()");
 
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;

/*
 *-------------------------------------------------------------------
 * fill parameter mode
 *-------------------------------------------------------------------
 */  
  *mode= pPHBCmdPrm->CLAEmode;

  return( AT_CMPL );
}

#ifdef TI_PS_FF_AT_P_CMD_CUST
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMIQ                |
|                                 ROUTINE : qAT_PercentCUST  |
+-------------------------------------------------------------------+

PURPOSE :    This function will set the customisation mode for the ACI and
                    other required entities
*/ 

GLOBAL T_ACI_RETURN qAT_PercentCUST( T_ACI_CMD_SRC srcId,
                                          T_CUST_MOD *customisation_mode)
{
    /*
    *-------------------------------------------------------------------
    * check command source
    *-------------------------------------------------------------------
    */  
    if(!cmh_IsVldCmdSrc (srcId)) 
    { 
    return( AT_FAIL );
    }

    *customisation_mode = (T_CUST_MOD)simShrdPrm.setPrm[srcId].cust_mode;

    return (AT_CMPL);
}
#endif /* TI_PS_FF_AT_P_CMD_CUST */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMIQ                |
|                                 ROUTINE : qAT_PercentSATCC  |
+-------------------------------------------------------------------+

PURPOSE :    This function will query the Call Control mode from the ACI
*/ 

GLOBAL T_ACI_RETURN qAT_PercentSATCC( T_ACI_CMD_SRC srcId,
                                          T_SAT_CC_MOD *sat_cc_mode)
{
    /*
    *-------------------------------------------------------------------
    * check command source
    *-------------------------------------------------------------------
    */  
    if(!cmh_IsVldCmdSrc (srcId)) 
    { 
    return( AT_FAIL );
    }

    *sat_cc_mode = (T_SAT_CC_MOD)simShrdPrm.setPrm[srcId].sat_cc_mode;

    return (AT_CMPL);
}

/*==== EOF ========================================================*/
