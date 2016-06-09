/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  
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
|             protocol stack adapter for GPRS mobility management ( GMM ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef CMH_GMMQ_C
#define CMH_GMMQ_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti_conn_mng.h"

#include "pcm.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_gmm.h"

#include "cmh.h"
#include "cmh_gmm.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMMQ                 |
| STATE   : finished              ROUTINE : qAT_PlusCGATT            |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGATT? AT
            command which returns the current state of GPRS attachment.
*/

GLOBAL T_ACI_RETURN qAT_PlusCGATT  ( T_ACI_CMD_SRC srcId, T_CGATT_STATE *state )
{
  TRACE_FUNCTION ("qAT_PlusCGATT()");

  if ( !cmh_IsVldCmdSrc (srcId) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  switch ( cmhGMM_Get_state() )
  {
    case AS_ATTACHED:
    case AS_SUSPENTED:
      *state = CGATT_STATE_ATTACHED;
      break;
    case AS_INVALID_STATE:
    case AS_MOBILE_OFF:
    case AS_DETACHED:
    case AS_ATTACHING:
    case AS_DETACHING:
    default:
      *state = CGATT_STATE_DETACHED;
      break;
   }

   return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMMQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCGCLASS          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGCLASS? AT
            command which returns current GACI mobile class.
*/

GLOBAL T_ACI_RETURN qAT_PlusCGCLASS  ( T_ACI_CMD_SRC srcId, T_CGCLASS_CLASS *m_class )
{
   TRACE_FUNCTION ("qAT_PlusCGCLASS()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
   if ( !cmh_IsVldCmdSrc (srcId) ) {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
   }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  switch ( gmmShrdPrm.mobile_class )
  {
    case GMMREG_CLASS_A:
      *m_class = CGCLASS_CLASS_A;     /* mobile class A  (highest) */
      break;
    case GMMREG_CLASS_B:
    case GMMREG_CLASS_BC:
    case GMMREG_CLASS_BG:
      *m_class = CGCLASS_CLASS_B;     /* mobile class B */
      break;
    case GMMREG_CLASS_CG:
      *m_class = CGCLASS_CLASS_CG;    /* mobile class CG (GPRS only mode) */
      break;
    case GMMREG_CLASS_CC:
      *m_class = CGCLASS_CLASS_CC;    /* mobile class CC (circuit switched only mode - lowest) */
      break;
    default: /*bad user command */
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return AT_FAIL;
  }

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMMQ                 |
| STATE   : finished              ROUTINE : qAT_PercentCGAATT        |
+--------------------------------------------------------------------+

  PURPOSE : This command get the mode of automatic attach / detach.
*/

GLOBAL T_ACI_RETURN qAT_PercentCGAATT  ( T_ACI_CMD_SRC srcId, T_CGAATT_ATTACH_MODE *att_m, T_CGAATT_DETACH_MODE *det_m )

{
  TRACE_FUNCTION ("qAT_PercentCGAATT()");  

  *att_m = automatic_attach_mode;
  *det_m = automatic_detach_mode;

  return( AT_CMPL );   
}

GLOBAL T_ACI_RETURN qAT_PlusCGREG( T_ACI_CMD_SRC srcId, T_CGREG_STAT *stat, USHORT *lac, USHORT *ci )
{
  TRACE_FUNCTION ("qAT_PlusCGREG()");

  *stat = gmmShrdPrm.cgreg_stat;
  *lac  = gmmShrdPrm.lac;
  *ci   = gmmShrdPrm.cid;

  return( AT_CMPL );
}

GLOBAL T_ACI_RETURN qAT_PercentCGREG   ( T_ACI_CMD_SRC srcId, T_P_CGREG_STAT *stat, USHORT *lac, USHORT *ci )
{
  TRACE_FUNCTION ("qAT_PercentCGREG()");

  *stat = gmmShrdPrm.p_cgreg_stat;
  *lac  = gmmShrdPrm.lac;
  *ci   = gmmShrdPrm.cid;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMMQ                 |
| STATE   : code                  ROUTINE : qAT_PercentCGCLASS       |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CGCLASS? AT
            command which returns current GPRS mobile class.
*/
GLOBAL T_ACI_RETURN qAT_PercentCGCLASS  ( T_ACI_CMD_SRC srcId, T_PERCENT_CGCLASS *m_class, T_PERCENT_CGCLASS *currentClass )
{
   TRACE_FUNCTION ("qAT_PercentCGCLASS()");

  *m_class = *currentClass = gaciMobileClass.current;
  if( PERCENT_CGCLASS_BX EQ gaciMobileClass.current)
  {
    switch ( gmmShrdPrm.mobile_class )
    {
      case GMMREG_CLASS_BC:
        *currentClass = PERCENT_CGCLASS_BC;
        break;
      case GMMREG_CLASS_BG:
        *currentClass = PERCENT_CGCLASS_BG;
        break;
    }
  }

  return( AT_CMPL );
}
#endif  /* GPRS */
/*==== EOF ========================================================*/
