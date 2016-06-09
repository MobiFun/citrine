/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SSP
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
|  Purpose :  This module defines the processing functions for the
|             primitives send to the protocol stack adapter by 
|             supplementary service.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_SSP_C
#define PSA_SSP_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "ksd.h"
#include "aci.h"
#include "psa.h"
#include "psa_ss.h"

#ifdef SIM_TOOLKIT
#include "psa_cc.h"
#include "psa_sat.h"
#endif

#include "cmh_ss.h"
#include "aci_mem.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSP                 |
|                                 ROUTINE : psa_mnss_begin_ind      |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSS_BEGIN_IND primitive send by SS.
            this indicates an incoming service transaction.

*/

GLOBAL void psa_mnss_begin_ind 
                                 ( T_MNSS_BEGIN_IND *mnss_begin_ind )
{
  SHORT sId;                /* holds service id */        
  T_SS_SRV_TBL * pStbNtry;  /* holds pointer to service table entry */

  TRACE_FUNCTION ("psa_mnss_begin_ind()");

/*
 *-------------------------------------------------------------------
 * check for new entry in service table
 *-------------------------------------------------------------------
 */  
 psaSS_DumpFIE ( &mnss_begin_ind -> fac_inf );
#ifdef TI_PS_FF_AT_P_CMD_CSCN
 cmhSS_sendFie( CSCN_FACILITY_DIRECTION_IN,
                CSCN_FACILITY_TRANS_TYPE_BEGIN,
                &mnss_begin_ind -> fac_inf);
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

  sId = psaSS_stbNewEntry();

  if( sId < 0 )
  {
    TRACE_EVENT ("MTS rejected due to full service table");

    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */ 
  pStbNtry = &ssShrdPrm.stb[sId];
  
  pStbNtry -> ntryUsdFlg = TRUE;
  pStbNtry -> ti         = mnss_begin_ind -> ti;

  pStbNtry -> srvStat = SSS_ACT;
  pStbNtry -> srvType = ST_MTS;

  psaSS_chngSrvTypCnt( sId, +1 );

/*
 *-------------------------------------------------------------------
 * decode component type
 *-------------------------------------------------------------------
 */  
  CCD_START;
  {
    UBYTE ccdRet;

    MCAST( com, COMPONENT );  /* T_COMPONENT *com = (T_COMPONENT *)_decodedMsg; */
    memset( com, 0, sizeof( T_COMPONENT ));
    
    ccdRet = ccd_decodeMsg (CCDENT_FAC,
                            DOWNLINK,
                            (T_MSGBUF *) &mnss_begin_ind -> fac_inf,
                            (UBYTE    *) _decodedMsg,
                            COMPONENT);
    if( ccdRet NEQ ccdOK )
    {
      TRACE_EVENT_P1( "CCD Decoding Error: %d",ccdRet );
      ssShrdPrm.stb[sId].failType = SSF_CCD_DEC;
      cmhSS_TransFail(sId);
    }
    else
    {
      /* use prim buffer as further decoding buffer */
      ssFIEDecodeBuf = mnss_begin_ind -> fac_inf.fac;

      if( com->v_inv_comp )
      {
        T_COMPONENT *psCopyOfCom;
        ACI_MALLOC( psCopyOfCom, sizeof(T_COMPONENT) );
        /* now copy the com content for second CCD process */
        memcpy( psCopyOfCom, com, sizeof(T_COMPONENT) );
        psaSS_dasmInvokeCmp( sId, &(psCopyOfCom->inv_comp) );
        ACI_MFREE(psCopyOfCom);
      }
      else
      {
        TRACE_EVENT( "WRONG COMPONENT TYPE RECEIVED" );
      }
    }
  }
  CCD_END;
/*
 *-------------------------------------------------------------------
 * free primitive
 *-------------------------------------------------------------------
 */
  ssFIEDecodeBuf = NULL;
  PFREE (mnss_begin_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSP                 |
|                                 ROUTINE : psa_mnss_end_ind        |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSS_END_IND primitive send by SS.
            this indicates the end of a service transaction.

*/

GLOBAL void psa_mnss_end_ind( T_MNSS_END_IND *mnss_end_ind )
{
  SHORT        sId;          /* holds service id */
  T_SS_SRV_TBL *pStbNtry;    /* holds pointer to service table entry */
  SHORT        pending_sId;  /* holds second id if there is one */
  BOOL         this_is_a_ussd_string;
  T_ACI_RETURN ss_return;

  TRACE_FUNCTION ("psa_mnss_end_ind()");

  /* trace facility */
 psaSS_DumpFIE ( &mnss_end_ind -> fac_inf );
#ifdef TI_PS_FF_AT_P_CMD_CSCN
 cmhSS_sendFie ( CSCN_FACILITY_DIRECTION_IN,
                 CSCN_FACILITY_TRANS_TYPE_END,
                 &mnss_end_ind -> fac_inf);
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

#ifdef SIM_TOOLKIT
  /* reinitalize. */
  satShrdPrm.sId_pwd_requested = NO_ENTRY;
#endif /* SIM_TOOLKIT */

  /* find service in service table */  
  sId = psaSS_stbFindTi( mnss_end_ind -> ti );

  if( sId < 0 )
  {
    /* ignore primitive, due to not found transaction identifier. */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mnss_end_ind);
    return;
  }

  /* update shared parameter */  
  pStbNtry = &ssShrdPrm.stb[sId];

  pStbNtry -> failType = SSF_SS_ENT;
  pStbNtry -> entCs    = mnss_end_ind -> cause;

  pStbNtry -> srvStat = SSS_IDL;

  psaSS_chngSrvTypCnt( sId, -1 );
  psaSS_retMOSTi( sId );

  /****** Is this a USSD operation ? ****/
  if( ssShrdPrm.stb[sId].curCmd EQ AT_CMD_CUSD   OR
      ssShrdPrm.stb[sId].curCmd EQ (T_ACI_AT_CMD)KSD_CMD_USSD )
  {
    this_is_a_ussd_string = TRUE;
  }
#ifdef SIM_TOOLKIT
  else if( ssShrdPrm.stb[sId].srvOwn EQ OWN_SRC_SAT  AND
           sId EQ satShrdPrm.SentUSSDid )
  {
    /* SAT send USSD case */
    this_is_a_ussd_string = TRUE;
  }
#endif /* SIM_TOOLKIT */
  else
    this_is_a_ussd_string = FALSE;
  /************************************/


  /****** check error cause ***********/
  TRACE_EVENT_P1 ("SS end indication cause: %d", mnss_end_ind -> cause);

  if( mnss_end_ind->cause EQ MNSS_CAUSE_FACILITY_REJECT )
  {
    TRACE_EVENT("Facility has been rejected");

    /* check for protocol incompatibility for USSD */
    if( this_is_a_ussd_string  AND
        psaSS_asmUSSDProt1(sId) )
    {   
      /* facility with Protocole 1 format has been sent: wait for answer */
    }
    else
    {
      cmhSS_TransFail(sId);
    }
    PFREE (mnss_end_ind);
    return;
  }
  /************************************/

#ifdef SIM_TOOLKIT
  /* if SIM TOOLKIT and empty facility send send error to SAT 
  (but should have checked USSD compatibility BEFORE) */
  if( (ssShrdPrm.stb[sId].srvOwn EQ OWN_SRC_SAT) AND
      (mnss_end_ind -> fac_inf.l_fac EQ 0) )
  {
    cmhSS_TransFail(sId);
    PFREE (mnss_end_ind);
    return;
  }
#endif /* SIM_TOOLKIT */


  /*************************/
  /* decode component type */
  CCD_START;
  {
    UBYTE        ccdRet;
    T_COMPONENT *psCopyOfCom;
    BOOL        is_critical_warning = FALSE;

    MCAST( com, COMPONENT );    /* T_COMPONENT *com = (T_COMPONENT *)_decodedMsg; */
    memset( com, 0, sizeof( T_COMPONENT ));
    

    ccdRet = ccd_decodeMsg (CCDENT_FAC,
                            DOWNLINK,
                            (T_MSGBUF *) &mnss_end_ind -> fac_inf,
                            (UBYTE    *) _decodedMsg,
                            COMPONENT);
    if( ccdRet NEQ ccdOK )
    {
      if (ccdRet EQ ccdWarning)
      {
        UBYTE ccd_err;
        USHORT parlist [6];

        ccd_err = ccd_getFirstError (CCDENT_SS, parlist);

        do
        {
          switch (ccd_err)
          {
            case ERR_LEN_MISMATCH:     /* recoverable warnings */
            break;                       
            default:
            is_critical_warning = TRUE;  /* critical warning */
          }
          ccd_err = ccd_getNextError (CCDENT_SS, parlist);
        }while (ccd_err NEQ ERR_NO_MORE_ERROR);
      }

      if((ccdRet EQ ccdError) OR is_critical_warning)
      {
        TRACE_EVENT_P1("CCD Decoding Error: %d", ccdRet);
        ssShrdPrm.stb[sId].failType = SSF_CCD_DEC;
        cmhSS_TransFail(sId);
        CCD_END;
        PFREE (mnss_end_ind);
        return;
      }
    }

    /* use prim buffer as further decoding buffer */
    ssFIEDecodeBuf = mnss_end_ind -> fac_inf.fac;

    ACI_MALLOC( psCopyOfCom, sizeof(T_COMPONENT) );
    /* now copy the com content for second CCD process */
    memcpy( psCopyOfCom, com, sizeof(T_COMPONENT) );

#ifdef SIM_TOOLKIT
    if( ssShrdPrm.stb[sId].srvOwn EQ OWN_SRC_SAT )
    {
      ss_return = psaSAT_ss_end_ind( sId, psCopyOfCom, mnss_end_ind, this_is_a_ussd_string );
    }
    else
#endif /* SIM_TOOLKIT */
    {
      ss_return = psaSS_ss_end_ind( sId, psCopyOfCom, this_is_a_ussd_string );
    }

    ACI_MFREE(psCopyOfCom);

    switch(ss_return)
    {
      case(AT_CMPL):
        /* facility with Protocole 1 format has been sent: wait for answer */
        CCD_END;
        PFREE (mnss_end_ind);
        return;

      case(AT_EXCT):
        /* if more than one transactions were intended, send next one */
        if ( ssShrdPrm.mltyTrnFlg )
        {
          /* no error occured, then send next one */
          pending_sId = psaSS_GetPendingTrn( );
          /* unsent id flag */
          ssShrdPrm.mltyTrnFlg &= ~( 1u << pending_sId );
          /* send pending primitive */
          PSENDX (SS, ssShrdPrm.stb[pending_sId].save_prim ); 
        }
        break;

      default:
        cmhSS_TransFail(sId);
        break;
    }
  }
  CCD_END;
  PFREE (mnss_end_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSP                 |
|                                 ROUTINE : psa_mnss_facility_ind   |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSS_FACILITY_IND primitive send by SS.
            this indicates the receiving of a facility information 
            element.

*/

GLOBAL void psa_mnss_facility_ind 
                           ( T_MNSS_FACILITY_IND *mnss_facility_ind )
{
  SHORT sId;                 /* holds call id */

  TRACE_FUNCTION ("psa_mnss_facility_ind()");

/*
 *-------------------------------------------------------------------
 * find call in call table
 *-------------------------------------------------------------------
 */
 psaSS_DumpFIE ( &mnss_facility_ind -> fac_inf );
#ifdef TI_PS_FF_AT_P_CMD_CSCN
 cmhSS_sendFie ( CSCN_FACILITY_DIRECTION_IN,
                 CSCN_FACILITY_TRANS_TYPE,
                 &mnss_facility_ind -> fac_inf);
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

  sId = psaSS_stbFindTi( mnss_facility_ind -> ti );

  if( sId < 0 )
  {
   /* 
    * ignore primitive, due to not found transaction identifier.
    */
    TRACE_EVENT ("primitive rejected due to unused ti");
    PFREE(mnss_facility_ind);
    return;
  }

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */  

/*
 *-------------------------------------------------------------------
 * decode component type
 *-------------------------------------------------------------------
 */ 
  CCD_START;
  {
    UBYTE        ccdRet;

    MCAST( com, COMPONENT ); /* T_COMPONENT *com = (T_COMPONENT *)_decodedMsg; */
    memset( com, 0, sizeof( T_COMPONENT ));

    ccdRet = ccd_decodeMsg (CCDENT_FAC,
                            DOWNLINK,
                            (T_MSGBUF *) &mnss_facility_ind -> fac_inf,
                            (UBYTE    *) _decodedMsg,
                            COMPONENT);    
    if( ccdRet NEQ ccdOK )
    {
      TRACE_EVENT_P1( "CCD Decoding Error: %d",ccdRet );
      ssShrdPrm.stb[sId].failType = SSF_CCD_DEC;
      cmhSS_TransFail(sId);
    }
    else
    {
      T_COMPONENT *psCopyOfCom;
      ACI_MALLOC( psCopyOfCom, sizeof(T_COMPONENT) );
      /* now copy the com content for second CCD process */
      memcpy( psCopyOfCom, com, sizeof(T_COMPONENT) );

      /* use prim buffer as further decoding buffer */
      ssFIEDecodeBuf = mnss_facility_ind -> fac_inf.fac;

      if( com->v_inv_comp)
      {
        psaSS_dasmInvokeCmp( sId, &(psCopyOfCom->inv_comp) );
      }
      else if (com->v_res_comp)
      {
        psaSS_dasmResultCmp( sId, &(psCopyOfCom->res_comp) );
      }
      else
      {
        TRACE_EVENT( "WRONG COMPONENT TYPE RECEIVED" );
      }

      ACI_MFREE(psCopyOfCom);
    }
  }
  CCD_END;
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (mnss_facility_ind);
}


/*==== EOF =========================================================*/
