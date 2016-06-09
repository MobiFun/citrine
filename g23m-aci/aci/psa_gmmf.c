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
|  Purpose :  This module defines the functions for the protocol
|             stack adapter for the registration part of mobility
|             management ( GMM ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef PSA_GMMF_C
#define PSA_GMMF_C
#endif

#include "aci_all.h"

#undef TRACING
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci_cmh.h"
#include "gaci.h"
#include "aci.h"
#include "psa.h"
#include "psa_gmm.h"
#include "cmh.h"
#include "cmh_gmm.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMF                |
|                                 ROUTINE : psaGMM_Init             |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for GMM.

*/

/* MACRO: initializer for set parameter */
#ifdef  INIT_SET_PARM
  #undef  INIT_SET_PARM
#endif

#define INIT_SET_PARM( dest, def )\
  for( LpCnt = 0; LpCnt < OWN_SRC_MAX; LpCnt++ )\
    gmmShrdPrm.setPrm[LpCnt].dest = def

GLOBAL void psaGMM_Init (  UBYTE auto_attach, UBYTE auto_detach, UBYTE mobile_class )
{
  UBYTE LpCnt;            /* holds loop counter for macro */

/*
 *-------------------------------------------------------------------
 * set default parms
 *-------------------------------------------------------------------
 */  
  default_mobile_class               = mobile_class;
  gmmShrdPrm.mobile_class            = default_mobile_class;
  gmmShrdPrm.requested_mobile_class  = gmmShrdPrm.mobile_class;
  gmmShrdPrm.last_attach_type        = ATTACH_TYPE_DETACHED;
  gmmShrdPrm.current_attach_type     = ATTACH_TYPE_DETACHED;
  gmmShrdPrm.requested_attach_type   = ATTACH_TYPE_DETACHED;

  automatic_attach_mode              = (T_CGAATT_ATTACH_MODE)auto_attach;
  automatic_detach_mode              = (T_CGAATT_DETACH_MODE)auto_detach;

  /*
   * GlumPs
   * ANS 15-Mar-00
   * GMM knows that SIM is not inserted
   * gmmShrdPrm.service_mode = SERVICE_MODE_LIMITED; 
   */
  gmmShrdPrm.service_mode = SERVICE_MODE_FULL;

  for (LpCnt = 0; LpCnt < MAX_PLMN_ID; LpCnt++)
    gmmShrdPrm.gprs_status[LpCnt] = GMMREG_GSM_ONLY;

  gmmShrdPrm.gprs_indicator = P_CREG_GPRS_Support_Unknown; /* ACI-SPR-24473 */

  gmmShrdPrm.cgreg_stat   = CGREG_STAT_NOT_REG;    /* +CGREG status */
  gmmShrdPrm.p_cgreg_stat = P_CGREG_STAT_NOT_REG;  /* %CGREG status */
  gmmShrdPrm.lac          = NOT_PRESENT_16BIT;     /* current cell coordinates */
  gmmShrdPrm.cid          = NOT_PRESENT_16BIT;

  switch(gmmShrdPrm.mobile_class)
  {
  case GMMREG_CLASS_A :
     gaciMobileClass.current = PERCENT_CGCLASS_A;
     break;
  case GMMREG_CLASS_B :
     gaciMobileClass.current = PERCENT_CGCLASS_B;
     break;
  case GMMREG_CLASS_BC  :
     gaciMobileClass.current = PERCENT_CGCLASS_BC;
     break;
  case GMMREG_CLASS_BG  :
     gaciMobileClass.current = PERCENT_CGCLASS_BG;
     break;
  case GMMREG_CLASS_CC :
     gaciMobileClass.current = PERCENT_CGCLASS_CC;
     break;
  case GMMREG_CLASS_CG :
     gaciMobileClass.current = PERCENT_CGCLASS_CG;
     break;
  default:
     gaciMobileClass.current = PERCENT_CGCLASS_MAX;
     break;
  }
}

#ifdef GMM_PRIMITIVE_TRACES_ADVANCED

char* dbg_attachType(UBYTE attach_type)
{
  char* type[3]  ={"GPRS", "IMSI", "COMB"};
  return type[attach_type - 1];
}
char* dbg_detachType(UBYTE detach_type)
{
  char* type[8]  ={"GPRS", "IMSI", "COMB",
                   "SIM_REM", "POWER_OFF", "DISABLE_GPRS",
                   "LIM_SERV", "SOFT_OFF"};
  return type[detach_type - 1];
}
char* dbg_mobileClass(UBYTE mobile_class)
{
  char* running[6]  ={"A", "B", "BC",
                      "BG", "CC", "CG"};
  return running[mobile_class - 1];

}
char* dbg_searchRunning(UBYTE search_running)
{
  char* running[2]  ={"NOT_RUN", "SEARCH_RUN"};
  return running[search_running];
}
char* dbg_cellState(UBYTE cell_state)
{
  char* state[8]  ={"FULL_SERV", "LIM_SERV", "CS_CALL",
                   "GPRS_NOT_SUP", "TRY_TO_UPD"};
  return state[cell_state];
}

#endif /* GMM_PRIMITIVE_TRACES_ADVANCED */

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_GMMF                          |
|                                 ROUTINE : is_GMM_searching_stoped           |
+-----------------------------------------------------------------------------+

  PURPOSE : Answer the Question: "Is the automatic searching from GMM stoped?"
*/
LOCAL BOOL is_GMM_searching_stoped (USHORT cause)
{
  switch ( cause )
  {
    case MMCS_IMSI_IN_HLR:              /* Network cause #2 via MM */
    /* GMMCS_IMSI_UNKNOWN: */           /* Via GMM, this doesn't stop GPRS */

    case MMCS_ILLEGAL_MS:               /* Network cause #3 */
    case GMMCS_ILLEGAL_MS:
    
    case MMCS_ILLEGAL_ME:               /* Network cause #2 */
    case GMMCS_ILLEGAL_ME:
    
    /* No MMCS_GPRS_NOT_ALLOWED */      /* Doesn't exist in MM protocol */
    case GMMCS_GPRS_NOT_ALLOWED:        /* Network cause #7 */
    
    /* No MMCS_GSM_GPRS_NOT_ALLOWED */  /* Doesn't exist in MM protocol */
    case GMMCS_GSM_GPRS_NOT_ALLOWED:    /* Network cause #8 */

    case MMCS_SIM_REMOVED:              /* SIM has been removed/disabled */
    case GMMCS_SIM_REMOVED:

    case MMCS_AUTHENTICATION_REJECTED:  /* AUTHENTICATION REJECT received */
    case GMMCS_AUTHENTICATION_REJECTED:

    /* No MMCS_POWER_OFF */             /* Swiched off */
    case GMMCS_POWER_OFF:
      return(TRUE); /* yes, it is stoped */
  }

  return(FALSE); /* no, it isn't stoped */
}


typedef enum
{
  NRS_NOT_REGISTERED,
  NRS_SEARCHING,
  NRS_REGISTERED

} T_NETWORK_REGISTRATION_STATUS;

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_GMMF                          |
|                                 ROUTINE : is_GMM_searching_stoped           |
+-----------------------------------------------------------------------------+

  PURPOSE : Answer the Question: "Is the automatic searching from GMM stoped?"
*/
LOCAL T_NETWORK_REGISTRATION_STATUS 
GMM_detached ( T_NETWORK_REGISTRATION_STATUS stat, 
               BOOL suspended, 
               UBYTE detach_type, 
               USHORT cause, 
               ULONG caller )
{
  if ( detach_type NEQ GMMREG_DT_IMSI )
  {
    /* special causes stops in every state the GMM searching*/
    if ( is_GMM_searching_stoped(cause) )
    {
      if( stat NEQ NRS_NOT_REGISTERED )
      {
        if ( caller EQ GMMREG_ATTACH_REJ AND 
             (  gmmShrdPrm.last_attach_type EQ GMMREG_AT_IMSI       OR 
                gmmShrdPrm.last_attach_type EQ ATTACH_TYPE_DETACHED    ) )
        {
          cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_DEN, P_CGREG_STAT_REG_DEN );        
        }
        else
        {
          cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_NOT_REG, P_CGREG_STAT_NOT_REG );
        }
        return NRS_NOT_REGISTERED;
      }
    }
    else
    {
      if ( ( stat EQ NRS_REGISTERED AND !suspended )     OR   /* in this state always */
           ( stat EQ NRS_REGISTERED AND  suspended )     AND   /* in this state only with special detach_type  */
           ( detach_type EQ GMMREG_DT_LIMITED_SERVICE   OR 
             detach_type EQ GMMREG_DT_SIM_REMOVED          ) )
      {
        cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_SEARCHING, P_CGREG_STAT_SEARCHING );
        return NRS_SEARCHING;
      }
    }
  }
  else  /* detach_type EQ GMMREG_DT_IMSI -> attach_type EQ GPRS*/
  {
    if( stat NEQ NRS_REGISTERED )
    {
      if ( cmhGMM_isPLMN_Home())
        { cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_HOME, P_CGREG_STAT_REG_HOME ); }
      else
        { cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_ROAM, P_CGREG_STAT_REG_ROAM ); }

      return NRS_REGISTERED;
    }
   }

   return stat;
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_GMMF                          |
|                                 ROUTINE : psaGMM_NetworkRegistrationStatus  |
+-----------------------------------------------------------------------------+

  PURPOSE : this function reported the network registration status for GPRS
            (+CGREG, %CGREG).
*/
GLOBAL void psaGMM_NetworkRegistrationStatus ( ULONG prim, void* para)
{
  T_NETWORK_REGISTRATION_STATUS tmp_stat;                         /* only temporary in use*/
  static T_NETWORK_REGISTRATION_STATUS stat = NRS_NOT_REGISTERED; /* current GPRS NRS */
  static BOOL suspended = FALSE;                                  /* is GPRS suspended */
  
  TRACE_FUNCTION ("psaGMM_NetworkRegistrationStatus()");

  switch ( prim )
  {
    case SMREG_PDP_ACTIVATE_REQ:
      if ( stat EQ NRS_NOT_REGISTERED )
      {
        stat = NRS_SEARCHING;
        cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_SEARCHING, P_CGREG_STAT_SEARCHING );
      }      
      break;
    case GMMREG_ATTACH_REQ:
      if ( stat EQ NRS_NOT_REGISTERED AND
           ((T_GMMREG_ATTACH_REQ*)para)->attach_type NEQ GMMREG_AT_IMSI )
      {
        stat = NRS_SEARCHING;
        cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_SEARCHING, P_CGREG_STAT_SEARCHING );
      }
      break;
    case GMMREG_ATTACH_CNF:
      if ((stat EQ NRS_NOT_REGISTERED OR stat EQ NRS_SEARCHING)  AND !suspended AND
           ((T_GMMREG_ATTACH_CNF*)para)->attach_type NEQ GMMREG_AT_IMSI )
      {
        stat = NRS_REGISTERED;

        if ( cmhGMM_isPLMN_Home())
          { cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_HOME, P_CGREG_STAT_REG_HOME ); }
        else
          { cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_ROAM, P_CGREG_STAT_REG_ROAM ); }
      }
      else if ( (stat EQ NRS_REGISTERED) AND suspended AND
                ((T_GMMREG_ATTACH_CNF*)para)->attach_type NEQ GMMREG_AT_IMSI AND
                ((T_GMMREG_ATTACH_CNF*)para)->search_running EQ GMMREG_SEARCH_NOT_RUNNING )
      {
        if(((T_GMMREG_ATTACH_CNF*)para)->gprs_indicator NEQ GMM_GPRS_SUPP_YES)
        {
          cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_NOT_REG, P_CGREG_STAT_NOT_REG ); 
          break;
        } 
        suspended = FALSE;

        if ( cmhGMM_isPLMN_Home())
          { cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_HOME, P_CGREG_STAT_REG_HOME ); }
        else
          { cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_ROAM, P_CGREG_STAT_REG_ROAM ); }
      }

      else
      { /* answer to ATTACH_REQ->attach_type=IMSI */
        if (   stat                                      NEQ NRS_NOT_REGISTERED        AND
             ((T_GMMREG_ATTACH_CNF*)para)->search_running EQ GMMREG_SEARCH_NOT_RUNNING AND
             ((T_GMMREG_ATTACH_CNF*)para)->attach_type    EQ GMMREG_AT_IMSI )
        {
          stat      = NRS_NOT_REGISTERED;
          suspended = FALSE;
          cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_NOT_REG, P_CGREG_STAT_NOT_REG );
        }
      }
      break;
    case GMMREG_ATTACH_REJ:
      tmp_stat = GMM_detached(stat, 
                              suspended,
                              ((T_GMMREG_ATTACH_REJ*)para)->detach_type,
                              ((T_GMMREG_ATTACH_REJ*)para)->cause,
                              GMMREG_ATTACH_REJ );

      if( stat NEQ tmp_stat )
      {
        stat = tmp_stat;
        suspended = FALSE;
      }
      break;
    case GMMREG_DETACH_CNF:
      if ( stat NEQ NRS_NOT_REGISTERED AND
           ((T_GMMREG_DETACH_CNF*)para)->detach_type NEQ GMMREG_DT_IMSI )
      {
        stat      = NRS_NOT_REGISTERED;
        suspended = FALSE;
        cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_NOT_REG, P_CGREG_STAT_NOT_REG );
      }
      break;
    case GMMREG_DETACH_IND:
      tmp_stat = GMM_detached(stat,
                              suspended,
                              ((T_GMMREG_DETACH_IND*)para)->detach_type,
                              ((T_GMMREG_DETACH_IND*)para)->cause,
                              GMMREG_DETACH_IND );

      if( stat NEQ tmp_stat )
      {
        stat = tmp_stat;
        suspended = FALSE;
      }
      break;
    case GMMREG_SUSPEND_IND:
      if ( ( stat EQ NRS_REGISTERED AND !suspended ) OR stat EQ NRS_SEARCHING)
      {
        suspended = TRUE;
        switch ( ((T_GMMREG_SUSPEND_IND*)para)->cell_state )
        {
          case GMMREG_FULL_SERVICE:
            /* nobody knows why we are not resumed */
            suspended = FALSE;
            break;
          case GMMREG_LIMITED_SERVICE:
	     cmhGMM_Set_state(AS_DETACHED);   // pinghua DCM_OPEN_CLOSE patch 20080429 start
	      gmmShrdPrm.current_attach_type = ATTACH_TYPE_DETACHED;
	 
            cmhGMM_send_percentNetworkRegistrationStatus( P_CGREG_STAT_LIMITED );
            break;
          case GMMREG_CS_CALL:
            cmhGMM_send_percentNetworkRegistrationStatus( P_CGREG_STAT_GSM_CALL );
            break;
          case GMMREG_GPRS_NOT_SUPPORTED:
	     cmhGMM_Set_state(AS_DETACHED);
            gmmShrdPrm.current_attach_type = ATTACH_TYPE_DETACHED;  //// pinghua DCM_OPEN_CLOSE patch 20080429 end

            cmhGMM_send_percentNetworkRegistrationStatus( P_CGREG_STAT_NO_CELL );
            break;
          case GMMREG_TRY_TO_UPDATE:
            cmhGMM_send_percentNetworkRegistrationStatus( P_CGREG_STAT_TRY_TO_UPDATE );
            break;
        }
      }
      break;
    case GMMREG_RESUME_IND:
      if ( suspended )
      {
        suspended = FALSE;

        if ( stat NEQ NRS_REGISTERED )
        {
          if ( cmhGMM_isPLMN_Home())
            { cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_HOME, P_CGREG_STAT_REG_HOME ); }
          else
            { cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_ROAM, P_CGREG_STAT_REG_ROAM ); }
        }
        else
        {
          if ( cmhGMM_isPLMN_Home())
            { cmhGMM_send_percentNetworkRegistrationStatus( P_CGREG_STAT_REG_HOME ); }
          else
            { cmhGMM_send_percentNetworkRegistrationStatus( P_CGREG_STAT_REG_ROAM ); }
        }
        stat = NRS_REGISTERED;
      }
      break;
    case GMMREG_AHPLMN_IND:   /* Enhancement for ActingHPLMN*/
      if(cmhGMM_isPLMN_Valid())
      {
        if(cmhGMM_isPLMN_Home())
        { cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_HOME, P_CGREG_STAT_REG_HOME); }
        else
        { cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_REG_ROAM, P_CGREG_STAT_REG_ROAM ); }
      }
      break;
    case GMMREG_DETACH_REQ:
    case GMMREG_NET_REQ:
    case GMMREG_PLMN_IND:
    case GMMREG_PLMN_RES:
    case GMMREG_PLMN_MODE_REQ:
    case GMMREG_INFO_IND:
    case GMMREG_CONFIG_REQ:
    case GMMREG_CIPHERING_IND:
      break;
    default:
      TRACE_EVENT_P1("UNSUPPORTED PRIMITIVE: %d", prim);
  }

}


#if 0
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_GMMF                |
|                                 ROUTINE : psaGMM_shrPrmDump       |
+-------------------------------------------------------------------+

  PURPOSE : this function dumps the shared parameter to the debug
            output.
*/

GLOBAL void psaGMM_shrPrmDump ( void )
{
#ifdef TRACING

  char  lnBuf [80];             /* holds buffer for output line */ 
  char  mccBuf[SIZE_MCC + 1];   /* MCC converted to printable C-string */
  char  mncBuf[SIZE_MNC + 1];   /* MNC converted to printable C-string */
  SHORT chrNr;                  /* holds number of processed chars */
  SHORT cnt;                    /* holds a counter */

  /* --- PLMN list ------------------------------------------------*/
  for( cnt = 0; cnt<MAX_PLMN_ID AND 
                mmShrdPrm.PLMNLst[cnt].v_plmn NEQ INVLD_PLMN; cnt++ )
  {
    chrNr  = sprintf( lnBuf, "%*.*s[%2d]", HDR_WDT, HDR_WDT, " PLMN list",cnt );
    utl_BCD2String (mccBuf, mmShrdPrm.PLMNLst[cnt].mcc, SIZE_MCC);
    utl_BCD2String (mncBuf, mmShrdPrm.PLMNLst[cnt].mnc, SIZE_MNC);
    chrNr += sprintf( lnBuf+chrNr, "%*s %*s", 
                      ITM_WDT/2, ITM_WDT/2, mccBuf, mncBuf);
    TRACE_EVENT( lnBuf );
  }

  /* --- used PLMN ------------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " used PLMN" );
  if( mmShrdPrm.usedPLMN.v_plmn EQ VLD_PLMN )
  {
    utl_BCD2String (mccBuf, mmShrdPrm.usedPLMN.mcc, SIZE_MCC);
    utl_BCD2String (mncBuf, mmShrdPrm.usedPLMN.mnc, SIZE_MNC);
    chrNr += sprintf( lnBuf+chrNr, "%*s %*s", 
                      ITM_WDT/2, ITM_WDT/2, mccBuf, mncBuf);
  }
  else
  {
    chrNr += sprintf( lnBuf+chrNr, "%*s", ITM_WDT, "none" );
  }
  TRACE_EVENT( lnBuf );

  /* --- registration mode ----------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "rgstr mode" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT, 
                                         mmShrdPrm.setPrm[0].regMode );
  TRACE_EVENT( lnBuf );

  /* --- registration status --------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "rgstr stat" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         mmShrdPrm.regStat );
  TRACE_EVENT( lnBuf );

  /* --- search result --------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " srch rslt" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         mmShrdPrm.srchRslt );
  TRACE_EVENT( lnBuf );

  /* --- de-registration cause ------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "dereg caus" );
  chrNr += sprintf( lnBuf+chrNr, "%*X", ITM_WDT,
                                        mmShrdPrm.deregCs );
  TRACE_EVENT( lnBuf );

#endif  /* of #ifdef TRACING */
}
#endif  /* #if 0 */

#endif  /* GPRS */
/*==== EOF ========================================================*/
 
