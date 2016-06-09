/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_L2RQ
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
|             protocol stack adapter for L2R.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_L2RQ_C
#define CMH_L2RQ_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "dti_conn_mng.h"

#include "cmh_ra.h"
#include "psa.h"
#include "psa_l2r.h"
#include "cmh.h"
#include "cmh_l2r.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_L2RQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCRLP             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CRLP? AT command
            which returns the current setting of iws, mws, t1 and
            n2.

            <iws>:   IWF to MS window size.
            <mws>:   MS to IWF window size
            <t1> :   acknowledge timer
            <n2> :   retransmission attemps
*/

GLOBAL T_ACI_RETURN qAT_PlusCRLP ( T_ACI_CMD_SRC srcId,
                                   SHORT* iws, SHORT* mws,
                                   SHORT* t1,  SHORT* n2 )
{
  T_L2R_CMD_PRM * pL2RCmdPrm; /* points to L2R command parameters */

  TRACE_FUNCTION ("qAT_PlusCRLP()");

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

  pL2RCmdPrm = &fnd_cmhPrm[srcId].l2rCmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *iws   = pL2RCmdPrm -> CRLPiws;
  *mws   = pL2RCmdPrm -> CRLPmws;
  *t1    = pL2RCmdPrm -> CRLPt1;
  *n2    = pL2RCmdPrm -> CRLPn2;


  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_L2RQ                 |
| STATE   : code                  ROUTINE : qAT_PlusDS               |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +DS? AT command
            which returns the current setting of dir, comp, maxDict
            and maxStr.

            <dir>     :   desired direction of data compression.
            <comp>    :   mode of continue
            <maxDict> :   maximum number of dictionary entries
            <maxStr>  :   maximum string length
*/

GLOBAL T_ACI_RETURN qAT_PlusDS    ( T_ACI_CMD_SRC srcId,
                                    T_ACI_DS_DIR* dir,
                                    T_ACI_DS_COMP* comp,
                                    LONG* maxDict,
                                    SHORT* maxStr )
{
  T_L2R_CMD_PRM * pL2RCmdPrm; /* points to L2R command parameters */

  TRACE_FUNCTION ("qAT_PlusDS()");

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

  pL2RCmdPrm = &fnd_cmhPrm[srcId].l2rCmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *dir     = pL2RCmdPrm -> DSdir;
  *comp    = pL2RCmdPrm -> DScomp;
  *maxDict = pL2RCmdPrm -> DSmaxDict;
  *maxStr  = pL2RCmdPrm -> DSmaxStr;


  return( AT_CMPL );
}


/*==== EOF ========================================================*/
