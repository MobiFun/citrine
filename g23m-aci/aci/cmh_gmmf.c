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
|  Purpose :  This module defines the functions used by the command
|             handler for GPRS mobility management ( GMM ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef CMH_GMMF_C
#define CMH_GMMF_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "pcm.h"

#include "dti_conn_mng.h"

#include "aci.h"
#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_mm.h"
#include "psa_gmm.h"
#include "psa_sm.h"
#include "psa_gppp.h"
#include "psa_sim.h"

#include "cmh.h"
#include "cmh_mm.h"
#include "cmh_gmm.h"
#include "cmh_sm.h"
#include "cmh_gppp.h"
#include "cmh_sim.h"

#include "aci_ext_pers.h"    
#include "aci_slock.h"       
/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
static  T_ATTACH_STATE      attach_state;
static  T_plmn              _plmn;

#ifdef SIM_PERS
EXTERN T_ACI_SLOCK_SHARED AciSLockShrd;
#endif

LOCAL BOOL  frstFlg = TRUE;  /* flags first attempt */
/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMM                  |
| STATE   : finished              ROUTINE : cmhGMM_Init              |
+--------------------------------------------------------------------+

  PURPOSE : Fill variables for the own use with default values.
*/
GLOBAL void cmhGMM_Init (void)
{
  gmmEntStat.curCmd = AT_CMD_NONE;
  gmmEntStat.entOwn = CMD_SRC_NONE;

  frstFlg = TRUE;
  cmhGMM_Set_state ( AS_MOBILE_OFF );
  _plmn.v_plmn = INVLD_PLMN;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMM                  |
| STATE   : finished              ROUTINE : cmhGMM_Get_state         |
+--------------------------------------------------------------------+

  PURPOSE : Get state of attachment.
*/
GLOBAL T_ATTACH_STATE cmhGMM_Get_state ( void )
{

  return attach_state;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMM                  |
| STATE   : finished              ROUTINE : cmhGMM_Set_state         |
+--------------------------------------------------------------------+

  PURPOSE : Set state of attachment.
*/
GLOBAL void cmhGMM_Set_state ( T_ATTACH_STATE state )
{

  if ( state > AS_INVALID_STATE AND state <= AS_SUSPENTED )
    attach_state = state;

}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMM                    |
| STATE   : finished              ROUTINE : cmhGMM_attach_if_necessary |
+----------------------------------------------------------------------+

  PURPOSE : Set state of attachment.
*/
GLOBAL T_ACI_RETURN cmhGMM_attach_if_necessary ( T_ACI_CMD_SRC srcId, T_ACI_AT_CMD curCmd )
{
  UBYTE        mobileClass = PERCENT_CGCLASS_MAX;

  if( AS_MOBILE_OFF EQ attach_state )
  {
    return AT_FAIL;
  }

  /* normal case: if mobile not GPRS attached */
  if ( AS_DETACHED EQ attach_state )
  {
    mobileClass = gmmShrdPrm.mobile_class;
  }

  /* case for NMOIII: if class BX -> BG requested */
  if( PERCENT_CGCLASS_BX EQ  gaciMobileClass.current AND
      GMMREG_CLASS_BG    NEQ gmmShrdPrm.mobile_class    )
  {
    mobileClass = GMMREG_CLASS_BG;
  }

  if( PERCENT_CGCLASS_MAX NEQ mobileClass )
  {
    gmmShrdPrm.last_attach_type = gmmShrdPrm.current_attach_type;
    gmmEntStat.entOwn = srcId;
    gmmEntStat.curCmd = curCmd;

    return cmhGMM_attach(mobileClass);  
  }
  
  return AT_CMPL;
}

/*
+--------------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMM                        |
| STATE   : finished              ROUTINE : cmhGMM_inform_other_psa        |
+--------------------------------------------------------------------------+

  PURPOSE : inform other PSA's if a automatic attach is passed or failed
*/
GLOBAL SHORT cmhGMM_inform_other_psa ( SHORT state )
{
  /* possible commands */
  switch ( gmmEntStat.curCmd )
  {
    case( AT_CMD_CGCLASS ):
    case( AT_CMD_CGATT ):
    default:
      /* no information necessary */
      return 0;
    case( AT_CMD_CGDATA ):
    case( AT_CMD_CGACT ):
      cmhSM_GprsAttached((T_GPRS_ATTACH_STATE)state);
      return 1;
  }

}

/*
+--------------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMM                        |
| STATE   : finished              ROUTINE : cmhGMM_isClassCG               |
+--------------------------------------------------------------------------+

  PURPOSE : return true if the 
*/
GLOBAL BOOL cmhGMM_isClassCG ( void )
{
  if (gmmShrdPrm.requested_mobile_class EQ GMMREG_CLASS_CG)
    return TRUE;
  
  return FALSE;
}

GLOBAL SHORT psaG_MM_CMD_REG ( void )
{
  UBYTE attach_type;
/*
 * Set automatic registration mode in MM
 */
  psaG_MM_CMD_SET_REGMD ( MODE_AUTO );

/*
 *-------------------------------------------------------------------
 * call the corresponding SAP subroutine
 *-------------------------------------------------------------------
 */
  if ( gmmShrdPrm.mobile_class NEQ GMMREG_CLASS_CG )
  {
    gmmShrdPrm.requested_mobile_class = gmmShrdPrm.mobile_class;

    attach_type = cmhGMM_requested_attach_type(GMMU_AT_GSM, gmmShrdPrm.requested_mobile_class);  

    /* OVK Set proper service mode dependent on Sim lock state */
#ifdef SIM_PERS
    if (AciSLockShrd.blocked EQ TRUE)
    {
       gmmShrdPrm.service_mode = SERVICE_MODE_LIMITED;   
    }
    else
 #endif
    if ( simShrdPrm.imei_blocked EQ TRUE )
    {
      gmmShrdPrm.service_mode = SERVICE_MODE_LIMITED; 
    }
    else
    {
       gmmShrdPrm.service_mode = SERVICE_MODE_FULL;
    }

    psaGMM_Attach( gmmShrdPrm.requested_mobile_class, 
                   attach_type,
                   gmmShrdPrm.service_mode);

    frstFlg = FALSE;
  }
  else
  {
    PALLOC (mmr_nreg_ind, MMR_NREG_IND);

    mmr_nreg_ind->service = NREG_NO_SERVICE;
    mmr_nreg_ind->search_running = SEARCH_NOT_RUNNING;
    psa_mmr_nreg_ind(mmr_nreg_ind);
  }


  return 0;
}

GLOBAL SHORT psaG_MM_CMD_DEREG ( UBYTE detach_type )
{
/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  frstFlg = TRUE;

  psaGMM_Detach( detach_type );

  return 0;
}

GLOBAL SHORT psaG_MM_CMD_NET_SRCH ( void )
{

/*
 * Set manual registration mode in MM
 */
  psaG_MM_CMD_SET_REGMD ( MODE_MAN );

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  if( frstFlg EQ TRUE
#ifdef FF_DUAL_SIM
    AND CFUNfun EQ CFUN_FUN_Full
#endif /*FF_DUAL_SIM*/
    )
  {
    gmmShrdPrm.requested_mobile_class = gmmShrdPrm.mobile_class;

    psaGMM_Attach( gmmShrdPrm.requested_mobile_class,
                   cmhGMM_requested_attach_type(GMMU_AT_GSM, gmmShrdPrm.requested_mobile_class),
                   SERVICE_MODE_FULL );
    frstFlg = FALSE;
  }
  else
  {
    psaGMM_Net_Req();
  }

  return 0;
}

GLOBAL SHORT psaG_MM_CMD_NET_SEL   ( void )
{
/*
 *-------------------------------------------------------------------
 * process parameter for both calls
 *-------------------------------------------------------------------
 */

/*
 * Set manual registration mode in MM
 */
  psaG_MM_CMD_SET_REGMD ( MODE_MAN );

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  gmmShrdPrm.requested_mobile_class  = gmmShrdPrm.mobile_class;

/*
 *-------------------------------------------------------------------
 * call the corresponding SAP subroutine
 *-------------------------------------------------------------------
 */
  psaGMM_Plmn_res(  gmmShrdPrm.requested_mobile_class,
                    cmhGMM_requested_attach_type(GMMU_AT_GSM, gmmShrdPrm.requested_mobile_class),
                    &mmShrdPrm.slctPLMN );

  frstFlg = FALSE;

  return 0;
}

GLOBAL SHORT psaG_MM_CMD_SET_REGMD ( UBYTE mode )
{

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  switch (mode)
  {
    case MODE_AUTO:
      psaGMM_Plmn_mode_req( GMMREG_NET_SEL_MODE_AUTO );
      break;
    case MODE_MAN:
      psaGMM_Plmn_mode_req( GMMREG_NET_SEL_MODE_MAN );
      break;
    default:
      return -1;
  }

/*
 *-------------------------------------------------------------------
 * call the corresponding SAP subroutine
 *-------------------------------------------------------------------
 */
  return 0;
}

GLOBAL SHORT gprs_sim_inserted ( void )
{

  /* OVK Set proper service mode dependent on Sim lock state */
  #ifdef SIM_PERS
  if (AciSLockShrd.blocked EQ TRUE)
  {
       gmmShrdPrm.service_mode = SERVICE_MODE_LIMITED;   
  }
  else
 #endif
  if ( simShrdPrm.imei_blocked EQ TRUE )
  {
    gmmShrdPrm.service_mode = SERVICE_MODE_LIMITED; 
  }
  else
  {
       gmmShrdPrm.service_mode = SERVICE_MODE_FULL;
  }
  gmmShrdPrm.mobile_class = default_mobile_class;
 
  cmhGMM_Set_state( AS_DETACHED );

  return 0;
}

GLOBAL T_ACI_RETURN cmhGMM_attach ( UBYTE mobile_class )
{

  TRACE_FUNCTION ("cmhGMM_attach()");

  if ( cmhGMM_Get_state() EQ AS_MOBILE_OFF )
  {
    gmmEntStat.curCmd = AT_CMD_NONE;
    gmmShrdPrm.requested_mobile_class = gmmShrdPrm.mobile_class = default_mobile_class = mobile_class;

    return AT_CMPL;
  }

  gmmShrdPrm.requested_mobile_class = mobile_class;
  gmmShrdPrm.last_attach_type       = gmmShrdPrm.current_attach_type;
  
  gmmShrdPrm.requested_attach_type = cmhGMM_requested_attach_type(GMMU_AT_GPRS, gmmShrdPrm.requested_mobile_class);

  psaGMM_Attach( gmmShrdPrm.requested_mobile_class,
                 gmmShrdPrm.requested_attach_type,
                 gmmShrdPrm.service_mode);

  return AT_EXCT;
}

/*
+--------------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_GMM                        |
| STATE   : finished              ROUTINE : cmhGMM_allContextsDeactivated  |
+--------------------------------------------------------------------------+

  PURPOSE : For automatic detaching the mobile after context deactivation.
*/
GLOBAL void cmhGMM_allContextsDeactivated ( void )
{

  TRACE_FUNCTION ("cmhGMM_allContextsDeactivated()");

  /* case for NMOIII: if class BX -> BC requested */
/* in this particular case an attach would be first sent just to inform
GMM of the class change.
It is OK if an attach_req is sent and then right after a detach_req. */
  if( PERCENT_CGCLASS_BX EQ  gaciMobileClass.current AND
      GMMREG_CLASS_BC    NEQ gmmShrdPrm.mobile_class    )
  {
    gmmShrdPrm.last_attach_type = gmmShrdPrm.current_attach_type;

    cmhGMM_attach((UBYTE)GMMREG_CLASS_BC);  
  }

  /* GPRS detach req will be sent only when the automatic detach mode is ON and
  the curCmd of mmEntStat should not be equal to CFUN.If user uses CFUN to deregister 
  from the network or to switch the mobile to power off mode, though automatic detach
  mode is ON it will not send one more gmmreg_detach_req. */
  if ( automatic_detach_mode EQ CGAATT_DETACH_MODE_ON AND mmEntStat.curCmd NEQ AT_CMD_CFUN)
  {
    psaGMM_Detach( GMMREG_DT_GPRS );
  }
}

GLOBAL UBYTE cmhGMM_requested_attach_type ( T_GMM_USER usr, UBYTE m_class )
{
  UBYTE attach_type = GMMREG_AT_COMB;

  switch ( m_class )
  {
    case GMMREG_CLASS_A:
    case GMMREG_CLASS_B:
    case GMMREG_CLASS_BG:
    case GMMREG_CLASS_BC:
      switch ( usr )
      {
        case GMMU_AT_GSM:
          if (  automatic_attach_mode NEQ CGAATT_ATTACH_MODE_AUTOMATIC         AND
                ( AS_DETACHED EQ attach_state OR AS_MOBILE_OFF EQ attach_state ))
                 
          {
            attach_type = GMMREG_AT_IMSI;
          }
          break;
        case GMMU_AT_GPRS:
          if (  mmShrdPrm.creg_status EQ CREG_STAT_NoSearch OR
                mmShrdPrm.creg_status EQ CREG_STAT_Denied   OR
                mmShrdPrm.creg_status EQ CREG_STAT_Unknown  OR
                mmShrdPrm.creg_status EQ CREG_STAT_NotPresent)
          {
            attach_type = GMMREG_AT_GPRS;
          }
          else if ( mmShrdPrm.creg_status EQ CREG_STAT_Search AND  
                    mmEntStat.curCmd EQ AT_CMD_NONE )
          {
            attach_type = GMMREG_AT_GPRS;     /* Check when the registration is initiated */
          }
          break;
      }
      break;
    case GMMREG_CLASS_CG:
      attach_type = GMMREG_AT_GPRS;
      break;
    case GMMREG_CLASS_CC:
      attach_type = GMMREG_AT_IMSI;
      break;
  }

  gmmShrdPrm.requested_attach_type = attach_type;

  return attach_type;
}

GLOBAL UBYTE cmhGMM_translate_gmm_cause_to_nreg_cs ( USHORT cause )
{
  switch( cause )
  {
    case GMMCS_POWER_OFF:
    case GMMCS_NO_SERVICE:
      return NREG_NO_SERVICE;

    /* ####!!!! Translated by GMM? */
    /* case RRCS_ABORT_CEL_SEL_FAIL: 
      return NREG_NO_SERVICE; */

    case GMMCS_LIMITED_SERVICE:
      return NREG_LIMITED_SERVICE;
    
    /* 
     * This catches all remaining GMM originated causes and 
     * all MM originated causes.
     */
    default:
      return NREG_LIMITED_SERVICE;
  }
}

/*
 *    COPS will be request an only GSM registration.
 *    If the ME is an only GPRS mobile, then it is impossible to request only GSM.
 *    This patch will be eliminate an error in this situation.
 *
 *    brz, 02.07.01
 */
GLOBAL BOOL  cmhGMM_class_eq_CG       ( void )
{

  if ( gmmShrdPrm.mobile_class EQ GMMREG_CLASS_CG )
  {
    /*
     *    If the mobile class is CG than the protocol stack
     *    will be attached automaticly.
     *
     *    patch brz, 090701
     */
    /*if ( automatic_attach_mode EQ CGAATT_ATTACH_MODE_AUTOMATIC )*/
    {
      gmmShrdPrm.requested_mobile_class = GMMREG_CLASS_CG;

      psaGMM_Attach( GMMREG_CLASS_CG, GMMREG_AT_GPRS, gmmShrdPrm.service_mode);
    }

    return TRUE;
  }

  return FALSE;
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_GMMF                              |
|                                 ROUTINE : cmhGMM_send_NetworkRegistrationStatus |
+---------------------------------------------------------------------------------+

  PURPOSE : report function
*/
GLOBAL void cmhGMM_send_NetworkRegistrationStatus( T_CGREG_STAT cgreg, T_P_CGREG_STAT p_cgreg )
{
  USHORT src;

  TRACE_FUNCTION("cmhGMM_send_NetworkRegistrationStatus");

  /* +CGREG */
  gmmShrdPrm.cgreg_stat = cgreg;

  for( src = 0 ; src < CMD_SRC_MAX; src++ )
  {
    R_AT( RAT_CGREG, (T_ACI_CMD_SRC)src ) (cgreg, gmmShrdPrm.lac, gmmShrdPrm.cid);
  }

  /* %CGREG */
  cmhGMM_send_percentNetworkRegistrationStatus( p_cgreg );
}

/*
+----------------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_GMMF                                     |
|                                 ROUTINE : cmhGMM_send_percentNetworkRegistrationStatus |
+----------------------------------------------------------------------------------------+

  PURPOSE : report function
*/
GLOBAL void cmhGMM_send_percentNetworkRegistrationStatus( T_P_CGREG_STAT p_cgreg )
{
  USHORT src;
  BOOL bActiveContext = FALSE;

  TRACE_FUNCTION("cmhGMM_send_percentNetworkRegistrationStatus");

  /* %CGREG */
  gmmShrdPrm.p_cgreg_stat = p_cgreg;

  bActiveContext = cmhSM_isContextActive();

  for( src = 0 ; src < CMD_SRC_MAX; src++ )
  {
    R_AT( RAT_P_CGREG, (T_ACI_CMD_SRC)src ) (p_cgreg, gmmShrdPrm.lac, gmmShrdPrm.cid, bActiveContext );
  }
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_GMMF                              |
|                                 ROUTINE : cmhGMM_setPLMN                        |
+---------------------------------------------------------------------------------+

  PURPOSE : report function
*/
GLOBAL void cmhGMM_setPLMN(T_plmn *plmn)
{
  memcpy(&_plmn, plmn, sizeof(T_plmn));
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_GMMF                              |
|                                 ROUTINE : cmhGMM_isPLMN_Home                    |
+---------------------------------------------------------------------------------+

  PURPOSE : report function
*/
GLOBAL UBYTE cmhGMM_isPLMN_Home( void )
{
  SHORT mccCur, mncCur;   /* holds MCC and MCC of current PLMN */

  if ( _plmn.v_plmn EQ VLD_PLMN )
  {
    cmhMM_CnvrtPLMN2INT( _plmn.mcc, 
                         _plmn.mnc, 
                         &mccCur, &mncCur );
    if (cmhSIM_plmn_is_hplmn (mccCur, mncCur)) 
    {
      return 1; /* yes, it is */
    }
  }
  
  return 0;     /* no, it isn't */
}

GLOBAL T_ACI_RETURN cmhGMM_contextActivation( T_ACI_CMD_SRC srcId, T_ACI_AT_CMD curCmd )
{

  /* case for NMOIII: if class BX -> BG requested */
  if( PERCENT_CGCLASS_BX EQ  gaciMobileClass.current AND
      GMMREG_CLASS_BG    NEQ gmmShrdPrm.mobile_class    )
  {
    gmmEntStat.entOwn = srcId;
    gmmEntStat.curCmd = curCmd;

    gmmShrdPrm.last_attach_type = gmmShrdPrm.current_attach_type;

    return cmhGMM_attach((UBYTE)GMMREG_CLASS_BG);  
  }
  
  return AT_CMPL;
  
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_GMMF                              |
|                                 ROUTINE : cmhGMM_isPLMN_Valid                   |
+---------------------------------------------------------------------------------+

  PURPOSE : Checks if _plmn contains a valid PLMN
*/
GLOBAL UBYTE cmhGMM_isPLMN_Valid (void)
{
  if ( _plmn.v_plmn EQ VLD_PLMN )
  {
    return 1;
  }
  return 0;
}

#endif  /* GPRS */
/*==== EOF ========================================================*/
