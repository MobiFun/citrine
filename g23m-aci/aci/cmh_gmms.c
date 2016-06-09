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
|  Purpose :  This module implements the set fuinctions related to the
|             protocol stack adapter for GPRS mobility management ( GMM ).
+-----------------------------------------------------------------------------
*/

#if defined (GPRS) && defined (DTI)

#ifndef CMH_GMMS_C
#define CMH_GMMS_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"

#include "dti_conn_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_gmm.h"
#include "psa_sm.h"
#ifdef FAX_AND_DATA
/* #include "aci_fd.h" */
#endif
#include "phb.h"
#include "cmh.h"
#include "cmh_gmm.h"
#include "cmh_sm.h"
#include "psa_sim.h"
#include "cmh_sim.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
GLOBAL T_ACI_RETURN sAT_PlusCGATT   ( T_ACI_CMD_SRC srcId, T_CGATT_STATE state );
GLOBAL T_ACI_RETURN sAT_PlusCGCLASS ( T_ACI_CMD_SRC srcId, T_CGCLASS_CLASS m_class );

GLOBAL T_ACI_RETURN sAT_PercentCGAATT ( T_ACI_CMD_SRC srcId, T_CGAATT_ATTACH_MODE att_m, T_CGAATT_DETACH_MODE det_m);
GLOBAL T_ACI_RETURN sAT_PercentCGCLASS ( T_ACI_CMD_SRC srcId, T_PERCENT_CGCLASS m_class );

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMMQ                 |
| STATE   : finished              ROUTINE : sAT_PlusCGATT            |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGATT= GPRS AT
          command which sets current GACI service state.

         Fails if illegal new state requested. Returns complete if
         already in existing state, otherwise signals GACI SEM to
         attach or detach GACI as appropriate.

         (Un)succesful completion of attach/detach is signalled by
         means of a GACI callback.
*/

GLOBAL T_ACI_RETURN sAT_PlusCGATT ( T_ACI_CMD_SRC srcId, T_CGATT_STATE state )
{
  T_ACI_RETURN    retCd;         /* holds return code */

  TRACE_FUNCTION ("sAT_PlusCGATT()");

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
  if( gmmEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check cfun status - avoid activating radio in case of flight mode (+CFUN=4)
 *-------------------------------------------------------------------
 */
  if (CFUNfun EQ CFUN_FUN_Disable_TX_RX_RF)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
    return( AT_FAIL );
  }


/*
 *-------------------------------------------------------------------
 * check first command argument
 *-------------------------------------------------------------------
 */
  if ( state < CGATT_STATE_OMITTED OR state >= CGATT_STATE_INVALID )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
  if (pb_get_fdn_mode () EQ FDN_ENABLE AND state EQ CGATT_STATE_ATTACHED)
  {
    if (pb_check_fdn (0, (const UBYTE *)"*99#") NEQ PHB_OK)
    {
      TRACE_EVENT("sAT_PlusCGATT: Entry not found in FDN, GPRS not allowed.");
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
      return (AT_FAIL);
    }
    TRACE_EVENT("sAT_PlusCGATT: Entry found in FDN, GPRS allowed.");
  }

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
switch ( state )
  {
    case CGATT_STATE_OMITTED:
      switch ( cmhGMM_Get_state() )
      {
        case AS_ATTACHED:
        case AS_SUSPENTED:
          state = CGATT_STATE_DETACHED;
          break;
        case AS_DETACHED:
          state = CGATT_STATE_ATTACHED;
          break;
        case AS_ATTACHING:
        case AS_DETACHING:
          return AT_BUSY;
        case AS_INVALID_STATE:
        case AS_MOBILE_OFF:
        default:
          return AT_FAIL;
      }

      retCd = sAT_PlusCGATT(srcId, state);
      break;
    case CGATT_STATE_ATTACHED:
      /*
       *  A class CC mobile can not attach GPRS
       */
      if ( gmmShrdPrm.mobile_class EQ GMMREG_CLASS_CC )
      {
        return AT_FAIL;
      }

      switch(gmmShrdPrm.current_attach_type)
      {
        case GMMREG_AT_GPRS:
        case GMMREG_AT_COMB:
          TRACE_EVENT("Already attached, attach not necessary.");
          retCd = AT_CMPL;
          break;
        case GMMREG_AT_IMSI:
          TRACE_EVENT("GSM already attached, initiating combined attach.");
          gmmEntStat.curCmd = AT_CMD_CGATT;
          gmmEntStat.entOwn = srcId;
          gmmShrdPrm.last_attach_type = gmmShrdPrm.current_attach_type;
          gmmShrdPrm.requested_attach_type = GMMREG_AT_COMB;
          gmmShrdPrm.requested_mobile_class = gmmShrdPrm.mobile_class;

          psaGMM_Attach( gmmShrdPrm.requested_mobile_class,
                         gmmShrdPrm.requested_attach_type,
                         gmmShrdPrm.service_mode);

          retCd = AT_EXCT;
          break;
        case ATTACH_TYPE_DETACHED:
          TRACE_EVENT("Mobile is detached, initiating GPRS attach.");
          gmmEntStat.curCmd = AT_CMD_CGATT;
          gmmEntStat.entOwn = srcId;
          gmmShrdPrm.last_attach_type = gmmShrdPrm.current_attach_type;
          gmmShrdPrm.requested_attach_type = GMMREG_AT_GPRS;
          gmmShrdPrm.requested_mobile_class = gmmShrdPrm.mobile_class;

          psaGMM_Attach( gmmShrdPrm.requested_mobile_class,
                         gmmShrdPrm.requested_attach_type,
                         gmmShrdPrm.service_mode);

          retCd = AT_EXCT;
          break;
        default:
          retCd = AT_FAIL;
          break;
      }
      break;
    case CGATT_STATE_DETACHED:
      switch ( cmhGMM_Get_state() )
      {
        case AS_DETACHED:
          retCd = AT_CMPL;
          break;
        case AS_ATTACHING:
        case AS_DETACHING:
          retCd = AT_BUSY;
          break;
        default:
          gmmEntStat.curCmd = AT_CMD_CGATT;
          gmmEntStat.entOwn = srcId;

          psaGMM_Detach( GMMREG_DT_GPRS );

          retCd = AT_EXCT;
          break;
      }
      break;
    case CGATT_STATE_INVALID:
    default:
      retCd = AT_FAIL;
      break;
  }

  return retCd;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMMQ                 |
| STATE   : finished              ROUTINE : sAT_PlusCGCLASS          |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGCLASS= GPRS AT
          command which sets the specified GPRS mobile class.
*/

GLOBAL T_ACI_RETURN sAT_PlusCGCLASS ( T_ACI_CMD_SRC srcId, T_CGCLASS_CLASS m_class )
{
  USHORT          mobile_class;

  TRACE_FUNCTION ("sAT_PlusCGCLASS()");

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
  if( gmmEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

  if (pb_get_fdn_mode () EQ FDN_ENABLE AND m_class NEQ CGCLASS_CLASS_CC)
  {
    if (pb_check_fdn (0, (const UBYTE *)"*99#") NEQ PHB_OK)
    {
      TRACE_EVENT("sAT_PlusCGCLASS: Entry not found in FDN, GPRS not allowed.");
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
      return (AT_FAIL);
    }
    TRACE_EVENT("sAT_PlusCGCLASS: Entry found in FDN, GPRS allowed.");
  }


/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  switch ( m_class )
  {
    case CGCLASS_CLASS_A:             /* mobile class A  (highest) */
      mobile_class = GMMREG_CLASS_A;
      break;
    case CGCLASS_CLASS_B:             /* mobile class B  (if necessary consider NET III) */
      mobile_class = GMMREG_CLASS_BG;
      break;
    case CGCLASS_CLASS_CG:            /* mobile class CG (GPRS only mode) */
      mobile_class = GMMREG_CLASS_CG;
      break;
    case CGCLASS_CLASS_CC:            /* mobile class CC (circuit switched only mode - lowest) */
      mobile_class = GMMREG_CLASS_CC;
      break;
    case CGCLASS_CLASS_OMITTED:
      mobile_class = gmmShrdPrm.mobile_class;
      break;
    case CGCLASS_CLASS_MAX:
    default: /*bad user command */
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return AT_FAIL;
  }

  gmmEntStat.entOwn = srcId;
  gmmEntStat.curCmd = AT_CMD_CGCLASS;

  return cmhGMM_attach((UBYTE) mobile_class );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMMQ                 |
| STATE   : finished              ROUTINE : sAT_PercentCGAATT        |
+--------------------------------------------------------------------+

PURPOSE : This command set the mode of automatic attach / detach.
*/

GLOBAL T_ACI_RETURN sAT_PercentCGAATT ( T_ACI_CMD_SRC srcId, T_CGAATT_ATTACH_MODE att_m, T_CGAATT_DETACH_MODE det_m )
{

  TRACE_FUNCTION ("sAT_PercentCGAATT()");

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( gmmEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check first command argument
 *-------------------------------------------------------------------
 */
  if ( att_m < CGAATT_ATTACH_MODE_OMITTED OR att_m >= CGAATT_ATTACH_MODE_INVALID )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  if ( det_m < CGAATT_DETACH_MODE_OMITTED OR det_m >= CGAATT_DETACH_MODE_INVALID )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  if ( att_m EQ CGAATT_ATTACH_MODE_OMITTED )
    att_m = automatic_attach_mode;

  if ( det_m EQ CGAATT_DETACH_MODE_OMITTED )
    det_m = automatic_detach_mode;

/*
 *-------------------------------------------------------------------
 * set parameter
 *-------------------------------------------------------------------
 */

  automatic_attach_mode = att_m;
  automatic_detach_mode = det_m;

  return AT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMMQ                 |
| STATE   : finished              ROUTINE : sAT_PercentCGMM          |
+--------------------------------------------------------------------+

PURPOSE : This command configurates GMM.
          CIPHER_ON = 1 / OFF = 0
          TLLI_TWICE = 1 / STANDARD = 0
*/

GLOBAL T_ACI_RETURN sAT_PercentCGMM ( T_ACI_CMD_SRC srcId, UBYTE cipher_on,
                                                           UBYTE tlli_handling )
{
  TRACE_FUNCTION ("sAT_PercentCGMM()");

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( gmmEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check command arguments
 *-------------------------------------------------------------------
 */
  if ( cipher_on NEQ 1 /*GMMREG_CIPHER_ON*/
    AND cipher_on NEQ 0 /*GMMREG_CIPHER_OFF*/ )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  if ( tlli_handling NEQ GMMREG_TLLI_STANDARD
    AND tlli_handling NEQ GMMREG_TLLI_TWICE )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  /* default value for tlli_handling */
  if ( tlli_handling EQ NOT_PRESENT_8BIT )
    tlli_handling = GMMREG_TLLI_STANDARD;

  psaGMM_Config_req( cipher_on, tlli_handling );
  return AT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMMQ                 |
| STATE   : finished              ROUTINE : sAT_PlusCGCLASS          |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the %CGCLASS= GPRS AT
          command which sets the specified GPRS mobile class.
*/

GLOBAL T_ACI_RETURN sAT_PercentCGCLASS ( T_ACI_CMD_SRC srcId, T_PERCENT_CGCLASS m_class )
{
  USHORT          mobile_class;

  TRACE_FUNCTION ("sAT_PercentCGCLASS()");

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
  if( gmmEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

  if (pb_get_fdn_mode () EQ FDN_ENABLE AND m_class NEQ PERCENT_CGCLASS_CC)
  {
    if (pb_check_fdn (0, (const UBYTE *)"*99#") NEQ PHB_OK)
    {
      TRACE_EVENT("sAT_PercentCGCLASS: Entry not found in FDN, GPRS not allowed.");
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
      return (AT_FAIL);
    }
    TRACE_EVENT("sAT_PercentCGCLASS: Entry found in FDN, GPRS allowed.");
  }

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  switch ( m_class )
  {
    case PERCENT_CGCLASS_A:             /* mobile class A  (highest) */
      mobile_class = GMMREG_CLASS_A;
      break;
    case PERCENT_CGCLASS_B:             /* mobile class B */
      mobile_class = GMMREG_CLASS_B;
      break;
    case PERCENT_CGCLASS_BG:            /* mobile class BG */
      mobile_class = GMMREG_CLASS_BG;
      break;
    case PERCENT_CGCLASS_BC:            /* mobile class BC */
      mobile_class = GMMREG_CLASS_BC;
      break;
    case PERCENT_CGCLASS_BX:            /* mobile class BX */
      mobile_class = cmhSM_isContextActive() ? GMMREG_CLASS_BG : GMMREG_CLASS_BC;
      break;
    case PERCENT_CGCLASS_CG:            /* mobile class CG (GPRS only mode) */
      mobile_class = GMMREG_CLASS_CG;
      break;
    case PERCENT_CGCLASS_CC:            /* mobile class CC (circuit switched only mode - lowest) */
      mobile_class = GMMREG_CLASS_CC;
      break;
    case PERCENT_CGCLASS_OMITTED:
      mobile_class = gmmShrdPrm.mobile_class;
      break;
    case PERCENT_CGCLASS_MAX:
    default: /*bad user command */
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return AT_FAIL;
  }

  gmmEntStat.entOwn = srcId;
  gmmEntStat.curCmd = AT_CMD_CGCLASS;

  gaciMobileClass.requested = m_class;  
  
  return cmhGMM_attach((UBYTE) mobile_class );
}

#endif  /* GPRS */
/*==== EOF ========================================================*/
