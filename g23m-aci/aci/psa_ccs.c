/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_CCS
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
|  Purpose :  This module defines the signalling functions of the 
|             protocol stack adapter for call control.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_CCS_C
#define PSA_CCS_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "ccdapi.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_ss.h"
#include "aoc.h"
#include "l4_tim.h"

#if !defined (MFW)
#include "aci_io.h"
#endif

#include "aci_fd.h"
#include "cmh.h"

#include "cmh_cc.h"
#include "psa_cc.h"

#include "wap_aci.h"
#include "aci_mem.h"
/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== IMPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_CCS                      |
|                            ROUTINE : psaCC_NewCall                |
+-------------------------------------------------------------------+

  PURPOSE : setup a new call

*/

GLOBAL SHORT psaCC_NewCall ( SHORT cId )
{
  T_CC_CALL_TBL *pCtbNtry = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION ("psaCC_NewCall()");

  if (pCtbNtry EQ NULL)
    return -1;

  /* get a valid ti for a MOC, otherwise return */
  if (psaCC_ctb(cId)->calType NEQ CT_NI_MOC )
  {
    if( psaCC_getMOCTi( cId ) < 0 ) return ( -1 );
  }

#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
  /* if this call is a WAP call then set wapId to the current Call Id */
  if (Wap_Call EQ TRUE)
  {
    wapId = cId;
  }
#endif /* defined(WAP) OR defined(FF_GPF_TCPIP) */

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (mncc_setup_req, MNCC_SETUP_REQ);

    /*
     * fill in primitive parameter: setup request
     */
    mncc_setup_req -> ti               = pCtbNtry -> ti;
    mncc_setup_req -> prio             = pCtbNtry -> prio;
    mncc_setup_req -> ri               = pCtbNtry -> rptInd;

    mncc_setup_req->called_party.ton          = pCtbNtry->cldPty.ton;
    mncc_setup_req->called_party.npi          = pCtbNtry->cldPty.npi;
    mncc_setup_req->called_party.c_called_num = pCtbNtry->cldPty.c_called_num;
    memcpy (mncc_setup_req->called_party.called_num, 
            pCtbNtry->cldPty.called_num,
            pCtbNtry->cldPty.c_called_num);

    mncc_setup_req -> called_party_sub = pCtbNtry -> cldPtySub;
    mncc_setup_req -> clir_sup         = pCtbNtry -> CLIRsup;
    memcpy( &(mncc_setup_req->bcpara),&(pCtbNtry->BC[0]), 
          sizeof( T_MNCC_bcpara) ); 
    memcpy( &(mncc_setup_req->bcpara2),&(pCtbNtry->BC[1]), 
          sizeof( T_MNCC_bcpara) ); 

    /*
     * fill in CUG info facility
     */
    if( !(pCtbNtry -> CUGidx EQ NOT_PRESENT_8BIT AND
          pCtbNtry -> CUGprf EQ FALSE AND
          pCtbNtry -> OAsup  EQ FALSE    ))
    {
      CCD_START;
      psaCC_asmCUGInfo( cId );

      {
        UBYTE ccdRet;

        MCAST( com, COMPONENT );

        memset( com, 0, sizeof( T_COMPONENT ));

        com -> v_inv_comp = TRUE;
        com -> inv_comp.v_inv_id  = TRUE;
        com -> inv_comp.inv_id    = pCtbNtry -> iId 
                                  = ccShrdPrm.iIdNxt++;
        com -> inv_comp.v_op_code = TRUE;
        com -> inv_comp.op_code   = pCtbNtry -> opCode
                                  = ssFIECodeBuf.buf[0];
        com -> inv_comp.v_params  = TRUE;
        com -> inv_comp.params.l_params = ssFIECodeBuf.l_buf-8;
        com -> inv_comp.params.o_params = 8;
        memcpy( com -> inv_comp.params.b_params, 
                ssFIECodeBuf.buf, ssFIECodeBuf.l_buf );

        mncc_setup_req -> fac_inf.l_fac = MNCC_FACILITY_LEN<<3;
        mncc_setup_req -> fac_inf.o_fac = 0;
        ccdRet = ccd_codeMsg (CCDENT_FAC,
                              UPLINK,
                              (T_MSGBUF *) &mncc_setup_req -> fac_inf,
                              (UBYTE    *) _decodedMsg,
                              COMPONENT);

        if( ccdRet NEQ ccdOK )
        {
          TRACE_EVENT_P1("CCD Coding Error: %d",ccdRet ); 
          memset( &mncc_setup_req -> fac_inf, 0, sizeof( T_MNCC_fac_inf));
        }
      }
      CCD_END;
    }
    else
    {
      memset( &mncc_setup_req -> fac_inf, 0, sizeof( T_MNCC_fac_inf));
    }

    if(pCtbNtry -> calType NEQ CT_MOC_RDL)
    {
      pCtbNtry -> calType = CT_MOC;
    }
    pCtbNtry -> curBC   = 0;

    PSENDX (CC, mncc_setup_req);

    /*
     * update call status
     */
    pCtbNtry -> calStat = CS_ACT_REQ;
    /* Inform that a call is initiated */
    psaCC_chngCalTypCnt( cId, +1 );   
  }
#ifdef FF_TTY
  /* TTY notification */
  cmhCC_notifyTTY (((pCtbNtry->BC[0].bearer_serv EQ MNCC_BEARER_SERV_SPEECH_CTM OR
                     pCtbNtry->BC[0].bearer_serv EQ MNCC_BEARER_SERV_AUX_SPEECH_CTM)?
                    CTTY_NEG_Request: CTTY_NEG_None),
                   CTTY_TRX_Unknown);
#endif /* FF_TTY */
  if(cmhCC_atdsendok ( cId ))
  {
    R_AT( RAT_OK, (T_ACI_CMD_SRC)pCtbNtry->curSrc ) ( pCtbNtry->curCmd );
  }

  /* start call time measurement */
  aoc_info (cId, AOC_START_TIME);

  /* Disable voice path in case this is first call */
  psaCC_setSpeechMode ();

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_AcceptCall        |
+-------------------------------------------------------------------+
  PURPOSE : In addition to sending the setup response for an MT call
            this function enables the vocoder if the call is a voice
            call
*/

GLOBAL void psaCC_AcceptCall ( SHORT cId )
{
  T_CC_CALL_TBL * pCtbNtry;     /* holds pointer to call table entry */

  TRACE_FUNCTION ("psaCC_AcceptCall()");

  /* update call status and attach the user if not done previously */
  pCtbNtry = ccShrdPrm.ctb[cId];
  pCtbNtry -> calStat = CS_CPL_REQ;

  
  psaCC_setSpeechMode();
  
  /* create and send primitive */
  {
    PALLOC (mncc_setup_res, MNCC_SETUP_RES);
    mncc_setup_res -> ti = pCtbNtry -> ti;
    PSENDX (CC, mncc_setup_res);
    psaCC_send_satevent( EVENT_CALL_CONN, cId, NEAR_END, FALSE );
  }

  /* start call time measurement */
  aoc_info (cId, AOC_START_TIME);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_ClearCall         |
+-------------------------------------------------------------------+

  PURPOSE : clear a call
*/

GLOBAL void psaCC_ClearCall ( SHORT cId )
{
  T_CC_CALL_TBL *pCtbNtry = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION ("psaCC_ClearCall()");

  if (!psaCC_ctbIsValid (cId))
    return; /* Ensure we are not dereferencing NULL */

  psaCC_StopDTMF ( cId );

  /* inform advice of charge module */
  aoc_info (cId, AOC_STOP_TIME);

  /*
   *-----------------------------------------------------------------
   * relase call if in disconnect request state
   *-----------------------------------------------------------------
   */
  if( pCtbNtry -> calStat EQ CS_DSC_REQ )
  {
    PALLOC (mncc_release_req, MNCC_RELEASE_REQ);

    /* fill in primitive parameter: release request */
    mncc_release_req -> ti    = pCtbNtry -> ti;
    mncc_release_req -> cause = pCtbNtry -> nrmCs;
      
    /* fill in CCBS request facility */
    if( pCtbNtry -> CCBSstat EQ CCBSS_REQ )
    {
      CCD_START;
      {
        psaCC_asmCCBSReq( cId );
        psaCC_asmComponent( cId );
      }
      CCD_END;
      
      mncc_release_req -> fac_inf.l_fac = ssFIECodeBuf.l_buf;
      mncc_release_req -> fac_inf.o_fac = 0;
      memcpy (mncc_release_req->fac_inf.fac, 
              ssFIECodeBuf.buf + (ssFIECodeBuf.o_buf >> 3),
              ssFIECodeBuf.l_buf >> 3);

      mncc_release_req -> ss_version = MNCC_SS_VERSION_3;
    }
    else
    {
      memset( &mncc_release_req -> fac_inf, 0, sizeof( T_MNCC_fac_inf));
      mncc_release_req -> ss_version = MNCC_SS_VER_NOT_PRES;
    }

    PSENDX (CC, mncc_release_req);

    psaCC_send_satevent( EVENT_CALL_DISC, cId, NEAR_END, TRUE );
  }

  /*
   *-----------------------------------------------------------------
   * relase call if in activate request state, network initiated
   *-----------------------------------------------------------------
   */
  else if( pCtbNtry -> calStat EQ CS_ACT_REQ AND
           pCtbNtry -> calType EQ CT_NI_MOC      )
  {
    PALLOC (mncc_reject_req, MNCC_REJECT_REQ);

    /* fill in primitive parameter: release request */
    mncc_reject_req -> ti    = pCtbNtry -> ti;
    mncc_reject_req -> cause = pCtbNtry -> nrmCs;

    PSENDX (CC, mncc_reject_req);

    /* free call table entry */
    psaCC_retMOCTi( pCtbNtry -> ti );

    psaCC_FreeCtbNtry (cId);
  }
      
  /*
   *-----------------------------------------------------------------
   * disconnect all other calls
   *-----------------------------------------------------------------
   */
  else
  {
    PALLOC (disc_req, MNCC_DISCONNECT_REQ); /* T_MNCC_DISCONNECT_REQ */
    disc_req -> ti    = pCtbNtry -> ti;
    disc_req -> cause = pCtbNtry -> nrmCs;
    if (pCtbNtry -> curCmd EQ AT_CMD_CTFR)
    {
      /* We expect here an already built facility in ssFIECodeBuf */
      disc_req -> fac_inf.l_fac = ssFIECodeBuf.l_buf;
      disc_req -> fac_inf.o_fac = 0;
      memcpy (disc_req->fac_inf.fac, 
              ssFIECodeBuf.buf, 
              ssFIECodeBuf.l_buf >> 3);
      disc_req -> ss_version = MNCC_SS_VERSION_2;
    }
    else
    {
      disc_req -> fac_inf.l_fac = 0;
      disc_req -> ss_version    = MNCC_SS_VER_NOT_PRES;
    }
    PSENDX (CC, disc_req);

    /* update call status */
    pCtbNtry -> calStat = CS_DSC_REQ;

//TISH, hot fix for GCF 27.22.7.3,  added in 2007-03-23
//start 38.patch for GCF test case:	27.22.7.3
    if (pCtbNtry -> nrmCs EQ 0x510)
    {
	pCtbNtry->numRawCauseBytes=2;
	ACI_MALLOC(pCtbNtry -> rawCauseBytes,2);
	pCtbNtry -> rawCauseBytes[0]=0xe0;
	pCtbNtry -> rawCauseBytes[1]=0x90;
    }
//end


    /* monitoring for SAT */
    psaCC_send_satevent( EVENT_CALL_DISC, cId , NEAR_END, TRUE );

//TISH, hot fix for GCF 27.22.7.3,  added in 2007-03-23
//start 38.patch for GCF test case:	27.22.7.3
    if (pCtbNtry -> nrmCs EQ 0x510)
    {
	pCtbNtry->numRawCauseBytes=0;
	ACI_MFREE(pCtbNtry -> rawCauseBytes);
    }	
//end

  }
  /* Switch off audio path after user action (e.g. hangup) */
/* clb this breaks ACI056 (among others..) necessary ??  pCtbNtry -> inBndTns = FALSE; */
  pCtbNtry -> inBndTns = FALSE;
  psaCC_setSpeechMode ();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_HoldCall          |
+-------------------------------------------------------------------+

  PURPOSE : put a call into hold state

*/

GLOBAL void psaCC_HoldCall ( SHORT cId )
{
  T_CC_CALL_TBL *pCtbNtry = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION ("psaCC_HoldCall()");

  if (pCtbNtry EQ NULL)
    return; /* Ensure not to dereference NULL */

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  psaCC_StopDTMF ( cId );

  {
    PALLOC (mncc_hold_req, MNCC_HOLD_REQ);

    /*
     * fill in primitive parameter: hold request
     */

    mncc_hold_req -> ti = pCtbNtry -> ti;
      
    PSENDX (CC, mncc_hold_req);

    /* update call status */
    pCtbNtry -> calStat = CS_HLD_REQ;

  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_RetrieveCall      |
+-------------------------------------------------------------------+

  PURPOSE : retrieve a held call

*/

GLOBAL void psaCC_RetrieveCall ( SHORT cId )
{
  T_CC_CALL_TBL *pCtbNtry = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION ("psaCC_RetrieveCall()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (mncc_retrieve_req, MNCC_RETRIEVE_REQ);

    /*
     * fill in primitive parameter: retrieve request
     */

    mncc_retrieve_req -> ti = pCtbNtry -> ti;
      
    PSENDX (CC, mncc_retrieve_req);

    /*
     * update call status   ???
     */
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_ModifyCall        |
+-------------------------------------------------------------------+

  PURPOSE : modify an active call 

*/

GLOBAL SHORT psaCC_ModifyCall ( SHORT cId )
{
  T_CC_CALL_TBL *pCtbNtry = ccShrdPrm.ctb[cId];
  UBYTE nomBC;                  /* holds nominal bearer capabilities */

  TRACE_FUNCTION ("psaCC_ModifyCall()");

  if (pCtbNtry EQ NULL)
    return -1;

/*
 *-------------------------------------------------------------------
 * check if call is in a active state
 *-------------------------------------------------------------------
 */
 if(
     (pCtbNtry -> calStat NEQ CS_ACT)     AND
     (pCtbNtry -> calStat NEQ CS_ACT_REQ)
   )
   return( -1 );
/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  psaCC_StopDTMF ( cId );
  {
    PALLOC (mncc_modify_req, MNCC_MODIFY_REQ);

    /*
     * fill in primitive parameter: modify request
     */
    nomBC = (pCtbNtry -> curBC)?0:1;

    switch( pCtbNtry -> BC[nomBC].bearer_serv )
    {
    case( MNCC_BEARER_SERV_AUX_SPEECH    ):
    case( MNCC_BEARER_SERV_SPEECH        ):
    case( MNCC_BEARER_SERV_AUX_SPEECH_CTM):
    case( MNCC_BEARER_SERV_SPEECH_CTM    ):
      mncc_modify_req -> serv = MNCC_SERV_SPEECH;
      break;

    case( MNCC_BEARER_SERV_FAX           ):
    case( MNCC_BEARER_SERV_ASYNC         ):
    case( MNCC_BEARER_SERV_SYNC          ):
    case( MNCC_BEARER_SERV_PAD_ACCESS    ):
    case( MNCC_BEARER_SERV_PACKET_ACCESS ):
      mncc_modify_req -> serv = MNCC_SERV_DATA;

      /* check for TTY service */
      cmhCC_TTY_Control ( cId, TTY_PAUSE );
      break;

    default:
      mncc_modify_req -> serv = MNCC_SERV_NOT_PRES;
    }
    mncc_modify_req ->   ti = pCtbNtry -> ti;

    PSENDX (CC, mncc_modify_req);

    /*
     * update call status
     */
    pCtbNtry -> calStat = CS_MDF_REQ;
  }
  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_SendDTMF          |
+-------------------------------------------------------------------+

  PURPOSE : send DTMF tones

*/

GLOBAL SHORT psaCC_SendDTMF ( SHORT cId,
                              UBYTE digit,
                              UBYTE mode )
{

  TRACE_FUNCTION ("psaCC_SendDTMF()");

  if (ccShrdPrm.ctb[cId] EQ NULL)
    return -1; /* Ensure not to dereference NULL */

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (mncc_start_dtmf_req, MNCC_START_DTMF_REQ);

    /*
     * fill in primitive parameter: DTMF tone request
     */

    mncc_start_dtmf_req -> ti       = psaCC_ctb(cId)->ti;
    mncc_start_dtmf_req -> key      = digit;
    mncc_start_dtmf_req -> dtmf_mod = mode;
      
    PSENDX (CC, mncc_start_dtmf_req);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_Config            |
+-------------------------------------------------------------------+
  PURPOSE : configure CC entity
*/

GLOBAL void psaCC_Config ( void )
{
  T_MNCC_bcpara *mtcBC;

  TRACE_FUNCTION ("psaCC_Config()");

  /* create and send primitive */
  {
    PALLOC (mncc_configure_req, MNCC_CONFIGURE_REQ);

    /* fill in primitive parameter: configure request */
    memset(&(mncc_configure_req -> called_party_sub), 0,
            sizeof(T_MNCC_called_party_sub));

    /* MTC bearer caps */
    mtcBC = &mncc_configure_req -> bcpara;
#ifdef FAX_AND_DATA
    mtcBC->rate = cmhCC_SelRate( ccShrdPrm.CBSTspeed );
#else
    mtcBC->rate = MNCC_UR_NOT_PRES;
#endif /* FAX_AND_DATA */

    if( ccShrdPrm.snsMode EQ CSNS_MOD_Fax      OR
        ccShrdPrm.snsMode EQ CSNS_MOD_VAFVoice OR
        ccShrdPrm.snsMode EQ CSNS_MOD_VAFFax )
    {
      mtcBC->bearer_serv = MNCC_BEARER_SERV_FAX;
      mtcBC->modem_type  = MNCC_MT_NONE;
      mtcBC->conn_elem   = MNCC_CONN_ELEM_TRANS;
    }
    else
    {
#ifdef FAX_AND_DATA
      mtcBC->bearer_serv = cmhCC_SelServ( ccShrdPrm.CBSTname );
      mtcBC->modem_type  = cmhCC_SelMT  ( ccShrdPrm.CBSTspeed );
      mtcBC->conn_elem   = cmhCC_SelCE  ( ccShrdPrm.CBSTce );
      mtcBC->transfer_cap  = cmhCC_SelTransferCap ( ccShrdPrm.CBSTspeed );
      mtcBC->rate_adaption = cmhCC_SelRateAdaption( ccShrdPrm.CBSTspeed );
#else
      mtcBC->bearer_serv = MNCC_BEARER_SERV_NOT_PRES;
      mtcBC->modem_type  = MNCC_MT_NONE;
      mtcBC->conn_elem   = MNCC_CONN_ELEM_NOT_PRES;
#endif /* FAX_AND_DATA */
    }


    /* not sure whether this is taken into account by CC */
    mtcBC->stop_bits   = DEF_BC1_SB; /*cmhCC_SelStopBit( srcId );*/
    mtcBC->data_bits   = DEF_BC1_DB; /*cmhCC_SelDataBit( srcId );*/
    mtcBC->parity      = DEF_BC1_PR; /*cmhCC_SelParity ( srcId );*/
    mtcBC->flow_control = DEF_BC1_FC;
    /*****************************************************/

/*    mncc_configure_req -> bcpara   = *mtcBC; superfluous */
    mncc_configure_req -> sns_mode = ccShrdPrm.snsMode;
#ifdef FF_TTY
    mncc_configure_req -> ctm_ena = ccShrdPrm.ctmReq;
#else
    mncc_configure_req -> ctm_ena = MNCC_CTM_DISABLED;
#endif /* FF_TTY */
    PSENDX (CC, mncc_configure_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_SendSS            |
+-------------------------------------------------------------------+

  PURPOSE : send SS facility

*/

LOCAL SHORT psaCC_SendSS ( SHORT cId )
{
  T_CC_CALL_TBL *pCtbNtry = ccShrdPrm.ctb[cId];
  UBYTE           ccdRet;       /* holds CCD return value */

  TRACE_FUNCTION ("psaCC_SendSS()");

  if (pCtbNtry EQ NULL)
    return -1; /* Ensure not to dereference NULL */

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (mncc_facility_req, MNCC_FACILITY_REQ);

    /*
     * fill in primitive parameter: facility request
     */

    mncc_facility_req -> ti          = pCtbNtry -> ti;
    mncc_facility_req -> ss_version  = pCtbNtry -> SSver;

    CCD_START;
    {
      MCAST( com, COMPONENT );

      memset( com, 0, sizeof( T_COMPONENT ));
      switch( ccShrdPrm.cmpType )
      {
        case( CT_INV ):
          com -> v_inv_comp = TRUE;
          com -> inv_comp.v_inv_id  = TRUE;
          com -> inv_comp.inv_id    = pCtbNtry -> iId 
                                    = ccShrdPrm.iIdNxt++;
          com -> inv_comp.v_op_code = TRUE;
          com -> inv_comp.op_code   = pCtbNtry -> opCode
                                    = ssFIECodeBuf.buf[0];
          com -> inv_comp.v_params  = TRUE;
          com -> inv_comp.params.l_params = ssFIECodeBuf.l_buf-8;
          com -> inv_comp.params.o_params = 8;
          memcpy( com -> inv_comp.params.b_params, 
                  ssFIECodeBuf.buf, ssFIECodeBuf.l_buf );
          break;
        case( CT_RET_RSLT ):
          break;
        case( CT_RET_ERR ):
          break;
        case( CT_RET_REJ ):
          break;
      }
      mncc_facility_req -> fac_inf.l_fac = MNCC_FACILITY_LEN<<3;
      mncc_facility_req -> fac_inf.o_fac = 0;
      ccdRet = ccd_codeMsg (CCDENT_FAC,
                            UPLINK,
                            (T_MSGBUF *) &mncc_facility_req -> fac_inf,
                            (UBYTE    *) _decodedMsg,
                            COMPONENT);

      if( ccdRet NEQ ccdOK )
      {
        TRACE_EVENT_P1( "CCD Coding Error: %d",ccdRet ); 
        PFREE( mncc_facility_req );
        CCD_END;
        return( -1 );
      }
    }
    CCD_END;

    pCtbNtry -> srvType = ST_MOS;

    psaCC_DumpFIE ( &mncc_facility_req -> fac_inf );
    cmhCC_sendFie ( CSCN_FACILITY_DIRECTION_OUT,
                    cId,
                    &mncc_facility_req -> fac_inf );
    
    PSENDX (CC, mncc_facility_req);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_BuildMPTY         |
+-------------------------------------------------------------------+

  PURPOSE : build multiparty

*/

GLOBAL void psaCC_BuildMPTY ( SHORT cId )
{
  SHORT ctbIdx;     /* holds call table index */

  TRACE_FUNCTION ("psaCC_BuildMPTY()");

/*
 *-------------------------------------------------------------------
 * set multiparty status for held calls
 *-------------------------------------------------------------------
 */
  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if (ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
        psaCC_ctb(ctbIdx)->calStat EQ CS_HLD )
    {
      if( psaCC_ctb(ctbIdx)->mptyStat NEQ CS_ACT )
      
        psaCC_ctb(ctbIdx)->mptyStat = CS_ACT_REQ;
    }
  }

  /* set multiparty status for active call */
  if( psaCC_ctb(cId)->mptyStat NEQ CS_ACT )
    psaCC_ctb(cId)->mptyStat = CS_ACT_REQ;

/*
 *-------------------------------------------------------------------
 * send facility information element
 *-------------------------------------------------------------------
 */
  psaCC_asmBuildMPTY();

  if( psaCC_SendSS(cId) EQ 0 )
  {
    ccShrdPrm.cIdMPTY = cId;
    TIMERSTART( TMPTY_VALUE, ACI_TMPTY );
  }
    
  psaCC_MPTY (cId, MNCC_MPTY_BUILD_SENT);
}

/* Implements Measure # 86 */


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_CountMPTY         |
+-------------------------------------------------------------------+

  PURPOSE : counts the number of parties in a MPTY

*/

GLOBAL int psaCC_CountMPTY ( void )
{
  int i = 0;
  SHORT cId;
  TRACE_FUNCTION ("psaCC_CountMPTY()");
  
  for( cId = 0; cId < MAX_CALL_NR; cId++ )
  {
    if (ccShrdPrm.ctb[cId] NEQ NULL AND
        psaCC_ctb(cId)->mptyStat EQ CS_ACT)
    {
      i++;
    }
  }
  return i;
}



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_SplitMPTY         |
+-------------------------------------------------------------------+

  PURPOSE : split multiparty

*/

GLOBAL void psaCC_SplitMPTY ( SHORT cId )
{
  TRACE_FUNCTION ("psaCC_SplitMPTY()");

  /* set multiparty status for active call */
  psaCC_ctb(cId)->mptyStat = CS_DSC_REQ;

/*
 *-------------------------------------------------------------------
 * send facility information element
 *-------------------------------------------------------------------
 */
  psaCC_asmSplitMPTY();

  if( psaCC_SendSS(cId) EQ 0 )
  {
    ccShrdPrm.cIdMPTY = cId;
    TIMERSTART( TMPTY_VALUE, ACI_TMPTY );
  }

  psaCC_MPTY (cId, MNCC_MPTY_SPLIT_SENT);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_ECT               |
+-------------------------------------------------------------------+

  PURPOSE : explicit call transfer

*/

GLOBAL SHORT psaCC_ECT ( SHORT cId )
{
  SHORT psa_ret;
  
  TRACE_FUNCTION ("psaCC_ECT()");

/*
 *-------------------------------------------------------------------
 * send facility information element
 *-------------------------------------------------------------------
 */
  psaCC_asmECT();

  psa_ret = psaCC_SendSS(cId);
    
  if( psa_ret EQ 0 )
  {
    TIMERSTART( TECT_VALUE, ACI_TECT );
  }
 
  return(psa_ret);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_BCapCode          |
+-------------------------------------------------------------------+

  PURPOSE : code bearer capabilites according GSM 04.08

*/

GLOBAL SHORT psaCC_BCapCode ( UBYTE reqId, SHORT cId )
{
  TRACE_FUNCTION ("psaCC_BCapCode()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (mncc_bearer_cap_req, MNCC_BEARER_CAP_REQ);

    /*
     * fill in primitive parameter: bearer capability coding request
     */
    
    mncc_bearer_cap_req -> req_id  = reqId;
    mncc_bearer_cap_req -> bc_mod  = MNCC_BC_MOD_CODE;
   /* 
    * Bearer Service parameter is used by CC to determine whether coding 
    * has to be done or not. Hence that resetting that alone is sufficient. 
    */
    mncc_bearer_cap_req -> bcpara.bearer_serv = MNCC_BEARER_SERV_NOT_PRES;
    mncc_bearer_cap_req -> bcpara2.bearer_serv = MNCC_BEARER_SERV_NOT_PRES;
    
    if ( ccShrdPrm.BC0_send_flag )
    {
      mncc_bearer_cap_req -> bcpara  = ccShrdPrm.ctb[cId]->BC[0];
    }
    if ( ccShrdPrm.BC1_send_flag )
    {
      mncc_bearer_cap_req -> bcpara2 = ccShrdPrm.ctb[cId]->BC[1];
    }

    PSENDX (CC, mncc_bearer_cap_req);
  }

  return 0;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_BCapDecode        |
+-------------------------------------------------------------------+

  PURPOSE : decode bearer cpabilities according GSM 04.08 into struct
            bcpara.

*/

GLOBAL SHORT psaCC_BCapDecode ( UBYTE reqId, UINT16 bcLen1, UBYTE *bc1, UINT16 bcLen2, UBYTE *bc2 )
{
  TRACE_FUNCTION ("psaCC_BCapDecode()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  if ((  bcLen1 > MNCC_MAX_BC_LEN ) OR ( bcLen2 > MNCC_MAX_BC_LEN )) return -1;

  {
    PALLOC (mncc_bearer_cap_req, MNCC_BEARER_CAP_REQ);

    /*
     * fill in primitive parameter: bearer capability decoding request
     */
    mncc_bearer_cap_req -> req_id         = reqId;
    mncc_bearer_cap_req -> bc_mod         = MNCC_BC_MOD_DECODE;
    mncc_bearer_cap_req -> bcconf.bc_len  = (UBYTE)bcLen1;
    mncc_bearer_cap_req -> bcconf2.bc_len = (UBYTE)bcLen2;

    if ( bcLen1 )
    {
      memcpy( mncc_bearer_cap_req -> bcconf.bc, bc1, bcLen1 );
    }

    if ( bcLen2 )
    {
      memcpy( mncc_bearer_cap_req -> bcconf2.bc, bc2, bcLen2 );
    }

    PSENDX (CC, mncc_bearer_cap_req);
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_MPTY              |
+-------------------------------------------------------------------+

  PURPOSE : Inform CC about an event related to a multiparty call
            which may change the auxiliary state hold or mpty in CC.

*/

GLOBAL SHORT psaCC_MPTY ( SHORT cId, UBYTE mpty_event )
{
  TRACE_FUNCTION ("psaCC_MPTY()");
  
  /* Send the synchronization request to CC */
  {
    PALLOC (mncc_sync_req, MNCC_SYNC_REQ); /* T_MNCC_SYNC_REQ */
    mncc_sync_req->synccs     = MNCC_SYNCCS_MPTY_EVENT;
    mncc_sync_req->ti         = psaCC_ctb(cId)->ti;
    mncc_sync_req->mpty_event = mpty_event;
    PSENDX (CC, mncc_sync_req);
  }
  return 0;
}

/* Implements Measure # 86 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_CCS                 |
|                                 ROUTINE : psaCC_HoldMPTY          |
+-------------------------------------------------------------------+

  PURPOSE : Hold OR Retrieve multiparty

*/

GLOBAL void psaCC_Hold_RetrieveMPTY ( SHORT     cId, 
                                      T_CC_CLST call_stat,
                                      T_CC_CLST call_stat_new,
                                      UBYTE     cId_new,
                                      UBYTE     opc)
{
  SHORT ctbIdx;     /* holds call table index */

  TRACE_FUNCTION ("psaCC_Hold_RetrieveMPTY()");

  /*
   *-----------------------------------------------------------------
   * set call status for multiparty members
   *-----------------------------------------------------------------
   */
  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if (ccShrdPrm.ctb[ctbIdx] NEQ NULL AND
        ccShrdPrm.ctb[ctbIdx]->calStat  EQ call_stat AND 
        ccShrdPrm.ctb[ctbIdx]->mptyStat EQ CS_ACT)
    {
      ccShrdPrm.ctb[ctbIdx]->calStat = call_stat_new;
    }
  }

  /* set multiparty status for active call */
  ccShrdPrm.ctb[cId]->calStat = call_stat_new;

  /*
   *-----------------------------------------------------------------
   * send facility information element
   *-----------------------------------------------------------------
   */

  /* 
   *-----------------------------------------------------------------
   * assemble the hold multiparty SS facility
   * information element, invoke id
   *-----------------------------------------------------------------
   */

  memset( &ssFIECodeBuf, 0, sizeof( ssFIECodeBuf ));

  ssFIECodeBuf.l_buf  = 8;
  ssFIECodeBuf.o_buf  = 0;
  ssFIECodeBuf.buf[0] = opc;

  ccShrdPrm.cmpType = CT_INV;

  if( psaCC_SendSS(cId) EQ 0 )
  {
    ccShrdPrm.cIdMPTY = cId;
    TIMERSTART( TMPTY_VALUE, ACI_TMPTY );
  }
  psaCC_MPTY (cId, cId_new);
}

/*==== EOF ========================================================*/
 
