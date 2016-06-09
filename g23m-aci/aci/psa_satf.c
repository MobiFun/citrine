/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SATF
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
|             stack adapter for the SIM application toolkit.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef SIM_TOOLKIT

#ifndef PSA_SATF_C
#define PSA_SATF_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "aci_mem.h"

#ifdef GPRS
#include "gaci_cmh.h"
#endif

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */


#include "aci.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_sat.h"
#include "psa_util.h"
#include "cmh.h"
#include "cmh_sat.h"

#include "psa_mm.h"
#include "psa_ss.h"
#include "psa_sim.h"
#include "cmh_cc.h"

#include "ksd.h"

/*==== CONSTANTS ==================================================*/

#define ITM_WDT         (14)    /* item width in chars */
#define HDR_WDT         (10)    /* header width in chars */

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/
LOCAL UBYTE ssResPrms[MAXIMUM( sizeof(BUF_res_params_sat),
                               MAXIMUM(sizeof(BUF_rej_params_sat),
                                       sizeof(BUF_err_params_sat)))];

/*==== FUNCTIONS ==================================================*/

LOCAL void psaSAT_getTonNpi(CHAR *inSeq, BOOL call_active, 
                            UBYTE *ton, UBYTE *npi);



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : psaSAT_BuildEnvCC       |
+-------------------------------------------------------------------+

  PURPOSE : Build envelope call control command.

*/

GLOBAL void psaSAT_BuildEnvCC ( SHORT          cId,
                                T_CLPTY_PRM   *ss_cldPty,
                                T_sat_ussd    *ussd,
                                T_MNCC_bcconf *ccp1,
                                T_MNCC_bcconf *ccp2)
{
  T_CC_CALL_TBL *pCtbNtry = ccShrdPrm.ctb[cId];
  UBYTE ton;
  UBYTE npi;
  
/*
 *-------------------------------------------------------------------
 *  build envelope call control command
 *-------------------------------------------------------------------
 */
  {
    MCAST( env, ENV_CMD );

    memset( env, 0, sizeof( T_ENV_CMD ));

    env->v_cc_cmd = TRUE;

    /* device identities */
    env->cc_cmd.v_dev_ids = TRUE;
    env->cc_cmd.dev_ids.src_dev  = DEV_SRC_ME;
    env->cc_cmd.dev_ids.dest_dev = DEV_DST_SIM;

    if( cId NEQ NO_ENTRY )
    {
      /* address */
      env->cc_cmd.v_addr = TRUE;

      if( (pCtbNtry->SATinv & SAT_REDIAL_ECCBE) AND 
           (rdlPrm.rdlState) )
      {
        memcpy(&env->cc_cmd.addr, &satRdlCCEnv.addr, sizeof(T_addr));
      }
      else
      {
        /* address */
        env->cc_cmd.addr.v_noa = TRUE;
        env->cc_cmd.addr.noa   = pCtbNtry->cldPty.ton;

        env->cc_cmd.addr.v_npi = TRUE;
        env->cc_cmd.addr.npi   = pCtbNtry->cldPty.npi;

        env->cc_cmd.addr.c_bcdDigit = pCtbNtry->cldPty.c_called_num;
        memcpy( env->cc_cmd.addr.bcdDigit, 
                pCtbNtry->cldPty.called_num,
                pCtbNtry->cldPty.c_called_num );
      }
    
#ifdef SIM_TOOLKIT
      if ( !rdlPrm.rdlState )
      {
        memcpy(&satRdlCCEnv.addr, &env->cc_cmd.addr, sizeof(T_addr));
      }
#endif


      /* capability configuration parameters 1*/
      if( ccp1 )
      {
        env->cc_cmd.v_cap_cnf_parms = TRUE;
        env->cc_cmd.cap_cnf_parms.l_cap_cnf_parms = ccp1->bc_len<<3;

        memcpy( env->cc_cmd.cap_cnf_parms.b_cap_cnf_parms, ccp1->bc,
                ccp1->bc_len );
      }

      /* capability configuration parameters 2*/
      if( ccp2 )
      {
        env->cc_cmd.v_cap_cnf_parms_2 = TRUE;
        env->cc_cmd.cap_cnf_parms_2.l_cap_cnf_parms_2 = ccp2->bc_len<<3;

        memcpy( env->cc_cmd.cap_cnf_parms_2.b_cap_cnf_parms_2, ccp2->bc,
                ccp2->bc_len );
      }

      /* If ccp is present in the setUpCall */
      if ((satShrdPrm.capParm.cntxt EQ CTX_SAT_SETUP) AND satShrdPrm.stk_ccp.l_cap_cnf_parms )
      {
        env->cc_cmd.v_cap_cnf_parms = TRUE;
        /* Storing the ccp if ECCBE is enabled */
        if( (ccShrdPrm.ctb[cId]->SATinv & SAT_REDIAL_ECCBE) AND 
             (rdlPrm.rdlState) )
        {
          memcpy(&env->cc_cmd.cap_cnf_parms, &satRdlCCEnv.ccp, sizeof(BUF_cap_cnf_parms));
        }
        else
        {
          env->cc_cmd.cap_cnf_parms.l_cap_cnf_parms = satShrdPrm.stk_ccp.l_cap_cnf_parms;
          memcpy( env->cc_cmd.cap_cnf_parms.b_cap_cnf_parms, &satShrdPrm.stk_ccp.b_cap_cnf_parms,
                satShrdPrm.stk_ccp.l_cap_cnf_parms );
        }
#ifdef SIM_TOOLKIT
      if ( !rdlPrm.rdlState )
      {
        memcpy( &satRdlCCEnv.ccp, &env->cc_cmd.cap_cnf_parms, sizeof(BUF_cap_cnf_parms));
      }
#endif
      }

      /* subaddress */
      if( pCtbNtry->cldPtySub.c_subaddr )
      {
        env->cc_cmd.v_subaddr = TRUE;

        if( (pCtbNtry->SATinv & SAT_REDIAL_ECCBE) AND 
           (rdlPrm.rdlState) )
        {
          memcpy( &env->cc_cmd.subaddr, &satRdlCCEnv.sub_addr, sizeof(T_subaddr) );
        }
        else
        {

          env->cc_cmd.subaddr.v_tos = TRUE;
          env->cc_cmd.subaddr.tos   = pCtbNtry->cldPtySub.tos;

          env->cc_cmd.subaddr.v_oei = TRUE;
          env->cc_cmd.subaddr.oei   = pCtbNtry->cldPtySub.odd_even;

          env->cc_cmd.subaddr.c_subadr_str = pCtbNtry->cldPtySub.c_subaddr;
          memcpy( env->cc_cmd.subaddr.subadr_str, pCtbNtry->cldPtySub.subaddr,
                  pCtbNtry->cldPtySub.c_subaddr );
        }
      
#ifdef SIM_TOOLKIT
        if ( !rdlPrm.rdlState )
        {
          memcpy( &satRdlCCEnv.sub_addr, &env->cc_cmd.subaddr, sizeof(T_subaddr));  
        }
#endif

      }
    }

    else if( ss_cldPty NEQ NULL )
    {
      /* SS string */
      env->cc_cmd.v_ss_string = TRUE;

      psaSAT_getTonNpi(ss_cldPty->num, FALSE, &ton, &npi);
      
      env->cc_cmd.ss_string.v_noa = TRUE;
      env->cc_cmd.ss_string.noa   = ton;
      env->cc_cmd.ss_string.v_npi = TRUE;
      env->cc_cmd.ss_string.npi   = npi;

      env->cc_cmd.ss_string.c_ss_ctrl_string =
                (UBYTE)utl_dialStr2BCD (ss_cldPty->num,        /*ssStr,*/
                                        env->cc_cmd.ss_string.ss_ctrl_string,
                                        MAX_SS_STRING_LEN);
    }

    else if( ussd NEQ NULL )
    {
      /* USSD string */
      env->cc_cmd.v_ussd_string = TRUE;

      env->cc_cmd.ussd_string.dcs        = ussd->dcs;
      env->cc_cmd.ussd_string.c_ussd_str = MINIMUM (ussd->c_ussd_str, MAX_SAT_USSD_LEN);

      memcpy( env->cc_cmd.ussd_string.ussd_str,
              ussd->ussd_str,
              env->cc_cmd.ussd_string.c_ussd_str);
    }

    /* location information */

       /* code MCC */
    env->cc_cmd.cc_smc_loc_info.mnc_mcc |= ((mmShrdPrm.usedPLMN.mcc[1]<<4) + mmShrdPrm.usedPLMN.mcc[0]) <<16;
    env->cc_cmd.cc_smc_loc_info.mnc_mcc |= ((mmShrdPrm.usedPLMN.mnc[2]<<4) + mmShrdPrm.usedPLMN.mcc[2]) << 8;

    /* code MNC */
    env->cc_cmd.cc_smc_loc_info.mnc_mcc |= (mmShrdPrm.usedPLMN.mnc[1]<<4)+mmShrdPrm.usedPLMN.mnc[0];

    /* code LAC */
    env->cc_cmd.cc_smc_loc_info.lac = mmShrdPrm.lac;

    /* code CID */
    env->cc_cmd.cc_smc_loc_info.cid = mmShrdPrm.cid;

    env->cc_cmd.v_cc_smc_loc_info = TRUE;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : psa_sat                  |
| STATE   : code                  ROUTINE : psaSAT_getTonNpi      |
+--------------------------------------------------------------------+

  PURPOSE : Get TON and NPI. The SS string in inSeq will be 
            decoded to check if it contains a phone number. If  
            SS String contains a number, its TON and NPI will be 
            returned. Otherwise TON and NPI will be set to FF as  
            required in GSM 11.14/GSM 11.11.
*/
LOCAL void psaSAT_getTonNpi(CHAR *inSeq, BOOL call_active, 
                            UBYTE *ton, UBYTE *npi)
{
  T_KSD_SEQGRP     grp;
  CHAR            *rest;
  T_KSD_SEQPARAM   para;
  CHAR             seq[MAX_DIAL_LEN];

  *ton = MNCC_TON_NOT_PRES;
  *npi = MNCC_NPI_NOT_PRES;
  
  strcpy(seq, inSeq);

  /* decode sequence */
  ksd_decode(seq, call_active, &grp, &rest, &para);
  
  /* analyse TON and NPI */
  switch( grp )
  {
    case SEQGRP_CF: 

      if ( (para.cf.num NEQ NULL) AND (para.cf.num[0] NEQ 0) )
      {
        *ton = para.cf.ton;
        *npi = para.cf.npi;
      }
      break;
      
    case SEQGRP_DIAL: 

      if ( ( para.dial.number NEQ NULL ) AND
           ( para.dial.number[0] EQ 0 ) )
      {
        if ( para.dial.number[0] EQ '+' )
        {
          *ton = TON_International;
        }
        else
        {
          *ton = TON_Unknown;
        }
        *npi = NPI_IsdnTelephony;
      }
        
      break;

#ifdef SMI
    case SEQGRP_SET_ABBR_DIAL:

      if ( (para.abbrDial.number NEQ NULL) AND 
           ( para.abbrDial.number[0] EQ 0 ) )
      {
        if ( para.abbrDial.number[0] EQ '+' )
        {
          *ton = TON_International;
        }
        else
        {
          *ton = TON_Unknown;
        }
        *npi = NPI_IsdnTelephony;
      }
      break;
#endif /* SMI */      

    default:
      /* Sequence group does not contain number. TON and NPI still set to FF */
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : psaSAT_BuildEnvCB       |
+-------------------------------------------------------------------+

  PURPOSE : Build envelope cell broadcast data download command.

*/

GLOBAL void psaSAT_BuildEnvCB ( UBYTE *cbMsg, SHORT cbLen )
{

/*
 *-------------------------------------------------------------------
 *  build envelope cell broadcast data download command
 *-------------------------------------------------------------------
 */
  {
    MCAST( env, ENV_CMD );

    memset( env, 0, sizeof( T_ENV_CMD ));

    env->v_cbd_cmd = TRUE;

    /* device identities */
    env->cbd_cmd.v_dev_ids = TRUE;
    env->cbd_cmd.dev_ids.src_dev  = DEV_SRC_NTW;
    env->cbd_cmd.dev_ids.dest_dev = DEV_DST_SIM;

    /* CB message */
    env->cbd_cmd.v_cb_page = TRUE;
    env->cbd_cmd.cb_page.l_cb_page = cbLen<<3;
    env->cbd_cmd.cb_page.o_cb_page = 0;

    memcpy( env->cbd_cmd.cb_page.b_cb_page, cbMsg, cbLen );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : psaSAT_BuildEnvMoSmCntr |
+-------------------------------------------------------------------+

  PURPOSE : Build envelope cell broadcast data download command.

*/

GLOBAL void psaSAT_BuildEnvMoSmCntr ( T_rp_addr sc_addr,
                                      T_tp_da   dest_addr )
{

/*
 *-------------------------------------------------------------------
 *  build envelope
 *-------------------------------------------------------------------
 */
  {
    MCAST( env, ENV_CMD );

    memset( env, 0, sizeof( T_ENV_CMD ));

    env->v_smc_cmd = TRUE;

    /* device identities */
    env->smc_cmd.v_dev_ids = TRUE;
    env->smc_cmd.dev_ids.src_dev  = DEV_DST_ME;
    env->smc_cmd.dev_ids.dest_dev = DEV_DST_SIM;

    /* short message address */
    env->smc_cmd.v_sm_addr = TRUE;

    env->smc_cmd.sm_addr.v_noa = TRUE;
    env->smc_cmd.sm_addr.noa = sc_addr.ton;

    env->smc_cmd.sm_addr.v_npi = TRUE;
    env->smc_cmd.sm_addr.npi = sc_addr.npi;

    env->smc_cmd.sm_addr.c_bcdDigit = sc_addr.c_num;
    memcpy(&(env->smc_cmd.sm_addr.bcdDigit[0]),
           &(sc_addr.num[0]),
           sc_addr.c_num /*MAX_SMS_DIGIT_LEN*/);

    /* address 2 */
    env->smc_cmd.v_sm_addr_2 = TRUE;

    env->smc_cmd.sm_addr_2.v_noa = TRUE;
    env->smc_cmd.sm_addr_2.noa = dest_addr.ton;

    env->smc_cmd.sm_addr_2.v_npi = TRUE;
    env->smc_cmd.sm_addr_2.npi = dest_addr.npi;

    env->smc_cmd.sm_addr_2.c_bcdDigit = dest_addr.c_num;
    memcpy(&(env->smc_cmd.sm_addr_2.bcdDigit[0]),
           &(dest_addr.num[0]),
           dest_addr.c_num /*MAX_SMS_DIGIT_LEN*/);

    /* location information */ 

    /* code MCC */
    env->smc_cmd.cc_smc_loc_info.mnc_mcc |= ((mmShrdPrm.usedPLMN.mcc[1]<<4) + mmShrdPrm.usedPLMN.mcc[0]) <<16;
    env->smc_cmd.cc_smc_loc_info.mnc_mcc |= ((mmShrdPrm.usedPLMN.mnc[2]<<4) + mmShrdPrm.usedPLMN.mcc[2]) << 8;

    /* code MNC */
    env->smc_cmd.cc_smc_loc_info.mnc_mcc |= (mmShrdPrm.usedPLMN.mnc[1]<<4)+mmShrdPrm.usedPLMN.mnc[0];

    /* code LAC */
    env->smc_cmd.cc_smc_loc_info.lac = mmShrdPrm.lac;

    /* code CID */
    env->smc_cmd.cc_smc_loc_info.cid = mmShrdPrm.cid;

    env->smc_cmd.v_cc_smc_loc_info = TRUE;

  }
}



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : psaSAT_BuildEnvEventDwn |
+-------------------------------------------------------------------+

  PURPOSE : Build envelope event download command.

*/

GLOBAL BOOL psaSAT_BuildEnvEventDwn ( UBYTE event, SHORT callId, T_CC_INITIATER actionSrc )
{
  T_CC_CALL_TBL * pCtbNtry;  /* holds pointer to call table entry */
  BOOL            MT_flag;
  UBYTE           trans_id = 0xFF;
  UBYTE           ti_flag = 0xFF; /* lint: initialization warning */

  TRACE_FUNCTION("psaSAT_BuildEnvEventDwn()");

  pCtbNtry = ccShrdPrm.ctb[callId];

/*
 *-------------------------------------------------------------------
 *  build envelope
 *-------------------------------------------------------------------
 */
  {
    MCAST( env, ENV_CMD );

    memset( env, 0, sizeof( T_ENV_CMD ));

    env->v_evd_cmd = TRUE;

    /* event */
    env->evd_cmd.v_ev_list = TRUE;
    env->evd_cmd.ev_list.c_event = 1;
    env->evd_cmd.ev_list.event[0] = event;

    /* device identities */
    env->evd_cmd.v_dev_ids = TRUE;
    env->evd_cmd.dev_ids.dest_dev = DEV_DST_SIM;
    switch( actionSrc )
    {
    case( NEAR_END ):
      env->evd_cmd.dev_ids.src_dev  = DEV_SRC_ME;
      break;

    case( FAR_END ):
      env->evd_cmd.dev_ids.src_dev  = DEV_SRC_NTW;
      break;

    default:
      env->evd_cmd.v_dev_ids = FALSE;
    }

    if ( event EQ EVENT_MT_CALL OR
         event EQ EVENT_CALL_CONN OR
         event EQ EVENT_CALL_DISC )
    {
      /* process ti identifier */
      env->evd_cmd.v_ti_list = TRUE;
      env->evd_cmd.ti_list.c_ti_oct = 1;

      MT_flag  = pCtbNtry -> ti & 0x08;             /* TRUE if MS terminated transaction */
      trans_id = ( pCtbNtry -> ti) & 0x07;    /* see GSM 04.07 ?1.2.3.1.3 */
    }

    switch( event )
    {
    case( EVENT_MT_CALL ):

      /* ti identifier */
      /* MNCC_SETUP_IND means message sent from the side that originated the TI */
      env->evd_cmd.ti_list.ti_oct[0].ti = /*lint -e(644) */ trans_id;

      /* address */
     if (pCtbNtry->clgPty.c_num NEQ 0)
     {
       env->evd_cmd.v_addr = TRUE;

       env->evd_cmd.addr.v_noa = TRUE;
       env->evd_cmd.addr.noa = pCtbNtry->clgPty.ton;

       env->evd_cmd.addr.v_npi = TRUE;
       env->evd_cmd.addr.npi = pCtbNtry->clgPty.npi;

       env->evd_cmd.addr.c_bcdDigit = pCtbNtry->clgPty.c_num;
       memcpy(&(env->evd_cmd.addr.bcdDigit[0]),
              &(pCtbNtry->clgPty.num[0]),
              pCtbNtry->clgPty.c_num);
     }

     /* called party subaddress */
     if (pCtbNtry->clgPtySub.c_subaddr NEQ 0)
     {
       env->evd_cmd.v_subaddr = TRUE;

       env->evd_cmd.subaddr.v_tos = TRUE;
       env->evd_cmd.subaddr.tos = pCtbNtry -> clgPtySub.tos;

       env->evd_cmd.subaddr.v_oei = TRUE;
       env->evd_cmd.subaddr.oei = pCtbNtry -> clgPtySub.odd_even;

       env->evd_cmd.subaddr.c_subadr_str = pCtbNtry->clgPtySub.c_subaddr;
       memcpy(&(env->evd_cmd.subaddr.subadr_str[0]),
              &(pCtbNtry->clgPtySub.subaddr[0]),
              pCtbNtry->clgPtySub.c_subaddr);
     }
     break;


    case( EVENT_CALL_CONN ):
      /* ti identifier */
      /* MNCC_SETUP_CNF context with MO Call
      or: MNCC_SETUP_RES context with MT Call
      means: message sent to the side that originates the TI */
      ti_flag = 0x08;
      env->evd_cmd.ti_list.ti_oct[0].ti = trans_id | ti_flag;
      break;


    case( EVENT_CALL_DISC ):
      /* ti identifier */
      /*
      means: message sent to the side that originates the TI */
      switch (actionSrc)
      {
      case( NEAR_END ):
        /* MNCC_REJ_IND / MNCC_DISC_REQ / MNCC_REL_REQ */
        if ( /*lint -e(644) */ MT_flag )
          ti_flag = 0x08;
        else
          ti_flag = 0x00;
        break;

      case( FAR_END ):
        /* MNCC_DISC_IND / MNCC_REL_IND / MNCC_REL_REQ */
        if ( MT_flag )
          ti_flag = 0x00;
        else
          ti_flag = 0x08;
        break;
      }
      env->evd_cmd.ti_list.ti_oct[0].ti = trans_id | ti_flag;

      /* Cause */
      env->evd_cmd.v_cause = TRUE;
      if (pCtbNtry -> numRawCauseBytes EQ 0)
      {
        if (GET_CAUSE_VALUE(pCtbNtry -> rejCs) NEQ NOT_PRESENT_8BIT)             /* radio-link failure */
        {
          env->evd_cmd.cause.c_cs = 0;
        }
        else
        {
          if (GET_CAUSE_VALUE(pCtbNtry -> nrmCs) NEQ NOT_PRESENT_8BIT)
          {
            env->evd_cmd.cause.c_cs = 2;

            /*Location is user : 0 and coding standard defined for the GSM PLMNS : 3 */ 
            env->evd_cmd.cause.cs[0] =0xE0;
		  
            env->evd_cmd.cause.cs[1] = GET_CAUSE_VALUE(pCtbNtry -> nrmCs) |0x80 ;  
          }
          else
          {
            env->evd_cmd.v_cause = FALSE;
          }
        }
      }
      else
      {
        env->evd_cmd.cause.c_cs = pCtbNtry -> numRawCauseBytes;
        memcpy( env->evd_cmd.cause.cs, 
                pCtbNtry -> rawCauseBytes,
                pCtbNtry -> numRawCauseBytes );
      }
      break;


    case( EVENT_LOC_STATUS ):
      env->evd_cmd.v_loc_state = TRUE;

      switch( satShrdPrm.locInfo.regStatus )
      {
      case( NO_VLD_RS ):
        env->evd_cmd.v_loc_state = FALSE;
        break;

      case( RS_NO_SRV ):
        env->evd_cmd.loc_state = 0x02;
        break;

      case( RS_LMTD_SRV ):
        env->evd_cmd.loc_state = 0x01;
        break;

      case( RS_FULL_SRV ):
        env->evd_cmd.loc_state = 0x00;

        /*---------------------------------------------------
             only in this case would location info be included
        ----------------------------------------------------*/

        /* location information */

         /* code MCC */
        env->evd_cmd.cc_smc_loc_info.mnc_mcc |= 
          ((satShrdPrm.locInfo.currPLMN.mcc[1]<<4) + satShrdPrm.locInfo.currPLMN.mcc[0]) <<16;
        env->evd_cmd.cc_smc_loc_info.mnc_mcc |= 
          ((satShrdPrm.locInfo.currPLMN.mnc[2]<<4) + satShrdPrm.locInfo.currPLMN.mcc[2]) << 8;

        /* code MNC */
        env->evd_cmd.cc_smc_loc_info.mnc_mcc |= 
          (satShrdPrm.locInfo.currPLMN.mnc[1]<<4)+satShrdPrm.locInfo.currPLMN.mnc[0];

        /* code LAC */
        env->evd_cmd.cc_smc_loc_info.lac = satShrdPrm.locInfo.lac;

        /* code CID */
        env->evd_cmd.cc_smc_loc_info.cid = satShrdPrm.locInfo.cid;

        env->evd_cmd.v_cc_smc_loc_info = TRUE;

        break;
      }

      break;
      
#ifdef FF_SAT_E
    case( EVENT_CHAN_STAT ):
      env->evd_cmd.v_chan_stat = TRUE;
      env->evd_cmd.chan_stat.chan_id = CHANNEL_ID_1;
      env->evd_cmd.chan_stat.chan_stat_link = LINK_NO_ESTABL;
      env->evd_cmd.chan_stat.chan_stat_inf2 = LINK_DROPPED;
      break;
#endif /* FF_SAT_E */      

    default:
      env->v_evd_cmd = FALSE;
      TRACE_EVENT("wrong event id for SAT event download");
      break;
    }
/*
 *-------------------------------------------------------------------
 *  check if a call control request is already in progress
 *-------------------------------------------------------------------
 */
    if( satShrdPrm.SIMCCParm.busy EQ TRUE )
    {
      /* event has to be queued */
      T_SAT_QUEUE *p_queue;

      if (satShrdPrm.event.c_queued >= MAX_EVENT_QUEUED - 1 )
      {
        TRACE_EVENT("Too many events have been queued: failed to queue this new event");
        return FALSE;
      }

      p_queue = &satShrdPrm.event.queued[satShrdPrm.event.c_queued - 1];
      p_queue->owner = (T_OWN)satShrdPrm.owner;
      
      if (p_queue->stk_cmd NEQ NULL)
      {
        MFREE (p_queue->stk_cmd); /* Should not happen */
      }
      ACI_MALLOC (p_queue->stk_cmd, sizeof (T_stk_cmd));
      if (psaSAT_STKBuildCmd (p_queue->stk_cmd) < 0)
      {
        ACI_MFREE (p_queue->stk_cmd);
        p_queue->stk_cmd = NULL;
        TRACE_ERROR ("problem encoding SAT cmd");
        return FALSE;
      }
      satShrdPrm.event.c_queued++;
    }
    return TRUE;
  }
}




/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : psaSAT_SendTrmResp      |
+-------------------------------------------------------------------+

  PURPOSE : Send a terminal response for a previous SAT command.

*/

GLOBAL void psaSAT_InitTrmResp ( T_ACI_SAT_TERM_RESP *init_resp )
{
  init_resp->addLen        = 0;
  init_resp->add           = NULL;
  init_resp->add_content   = 0;
  init_resp->resCC         = NULL;
  init_resp->text          = NULL;
  init_resp->at_resp       = NULL;
  init_resp->at_resp_count = 0;
#ifdef FF_SAT_E  
  init_resp->chnStat       = FALSE;
  init_resp->bearDesc      = FALSE;
  init_resp->bufSize       = FALSE;
#endif /* FF_SAT_E */  
  memset (init_resp->dtt_buf, 0xFF, TIME_STAMP_LENGTH);
  memset (init_resp->lang, 0xFF, CLAN_CODE_LEN);
}

LOCAL void sat_build_cc_action_resp(UBYTE rspId,
                                    UBYTE addLen,
                                    UBYTE *add,
                                    UBYTE *resCC,
                                    T_TERM_RESP *res)
{
  UBYTE byteLen;            /* holds byte length */
  UBYTE *p;                 /* byte pointer */

  if( satShrdPrm.stkCnfPrim OR resCC )
  {
    p = (resCC)? resCC : satShrdPrm.stkCnfPrim->stk_cmd.cmd;

    if( p[1] EQ 0x81 )
    {
      byteLen = p[2];
      p = &p[3];
    }
    else
    {
      byteLen = p[1];
      p = &p[2];
    }

    res->v_cc_req_act = TRUE;
    res->cc_req_act.l_cc_req_act = byteLen<<3;
    memcpy( res->cc_req_act.b_cc_req_act, p, byteLen );

    /* result 2 */
    res->v_res_2   = TRUE;
    res->res_2.gen = res->res.gen;

    /* adjust first result according to second result (GSM 11.14 / 9.1) */
    if( rspId <= 0x1F )

      res->res.gen = RSLT_PERF_MDFY_SIM;

    else if( rspId >= 0x20 AND rspId <= 0x2F )

      res->res.gen = RSLT_CC_SIM_TMP;

    else if( rspId >= 0x30 AND rspId <= 0x3F )
    {
      res->res.gen = RSLT_CC_SIM_PRM;
      res->res.v_add = TRUE;
      res->res.add.l_add = 8;
      res->res.add.b_add[0] = ADD_CC_REQ_CHNG;
    }
    if( addLen )
    {
      if( res->res.gen NEQ RSLT_CC_SIM_PRM ) res->res.v_add = FALSE;
      res->res_2.v_add = TRUE;
      res->res_2.add.l_add = addLen<<3;
      memcpy( res->res_2.add.b_add,
              add,
              addLen );
    }

    /* free the primitive */
    if( satShrdPrm.stkCnfPrim )
    {
      PFREE( satShrdPrm.stkCnfPrim );
      satShrdPrm.stkCnfPrim = NULL;
    }
  }
}

GLOBAL BOOL psaSAT_SendTrmResp ( UBYTE rspId,
                                 T_ACI_SAT_TERM_RESP *data_for_term_resp)
{
  TRACE_FUNCTION("psaSAT_SendTrmResp()");

/*
 *-------------------------------------------------------------------
 *  build terminal response
 *-------------------------------------------------------------------
 */
  CCD_START;
  {
    MCAST( res, TERM_RESP );

    memset( res, 0, sizeof( T_TERM_RESP ));

    /* init of data_for_term_resp */
    /* return network error cause GSM 11.14 / 12.12.3 (it's mandatory) */
    if( ( data_for_term_resp->add_content NEQ 0 ) OR
        ( rspId EQ RSLT_NTW_UNAB_PROC ) )
    {
      data_for_term_resp->add = &(data_for_term_resp->add_content);
      data_for_term_resp->addLen = 1;
    }

    /* device identities */
    res->v_cmd_details = TRUE;
    res->cmd_details.cmd_nr  = satShrdPrm.cmdDet.cmdNr;
    res->cmd_details.cmd_typ = satShrdPrm.cmdDet.cmdType;
    res->cmd_details.cmd_qlf = satShrdPrm.cmdDet.cmdQlf;

    /* device identities */
    res->v_dev_ids = TRUE;
    res->dev_ids.src_dev  = DEV_SRC_ME;
    res->dev_ids.dest_dev = DEV_DST_SIM;

    /* result */
    res->v_res = TRUE;
    res->res.gen = (rspId EQ RSLT_PERF_SUCCESS AND cmpFlg)?
                    RSLT_PERF_PART_CMPR:rspId;
    if( data_for_term_resp->addLen )
    {
      res->res.v_add = TRUE;
      res->res.add.l_add = data_for_term_resp->addLen<<3;
      memcpy( res->res.add.b_add, data_for_term_resp->add, data_for_term_resp->addLen );
    }

    /* call control requested action */
    sat_build_cc_action_resp( rspId,
                              data_for_term_resp->addLen,
                              data_for_term_resp->add,
                              data_for_term_resp->resCC,
                              res );

    /* USSD related terminal response */
    if( data_for_term_resp->text NEQ NULL )
    {
      res->v_text = TRUE;
      res->text.dcs = data_for_term_resp->text->dcs;
      res->text.c_text_str = data_for_term_resp->text->c_text_str;
      memcpy( &(res->text.text_str),
              data_for_term_resp->text->text_str,
              data_for_term_resp->text->c_text_str);
    }

    /* Run AT related terminal response */
    if( data_for_term_resp->at_resp NEQ NULL )
    {
      res->v_at_resp = TRUE;

      memcpy( res->at_resp.text_str,
              data_for_term_resp->at_resp,
              data_for_term_resp->at_resp_count );

      res->at_resp.c_text_str = (UBYTE)data_for_term_resp->at_resp_count;
    }

    /* 
      Provide Local Information (date, time and time zone)
      related terminal response 
    */
    if( data_for_term_resp->dtt_buf[0] NEQ 0xFF)
    {
      res->v_dtt = TRUE;

      memcpy( res->dtt.text_str,
              data_for_term_resp->dtt_buf,
              TIME_STAMP_LENGTH );

      res->dtt.c_text_str = (UBYTE)TIME_STAMP_LENGTH;
    }
    
    /* 
      Provide Local Information (Language Setting)
    */
    
    if( data_for_term_resp->lang[0] NEQ 0xFF)
    {
      res->v_lang = TRUE;

      memcpy( res->lang.lang_buf,
              data_for_term_resp->lang,
              CLAN_CODE_LEN );

      res->lang.c_lang_buf = (UBYTE)CLAN_CODE_LEN;
    }

#ifdef FF_SAT_E
    /* add channel status to terminal response */
    if( data_for_term_resp->chnStat NEQ FALSE )
    {
      /* restricted to only one channel for class e */
      res->v_chan_stat = TRUE;
      res->c_chan_stat = 1;
      res->chan_stat[0].chan_id = CHANNEL_ID_1;

      res->chan_stat[0].chan_stat_inf2 = NO_FURTH_INFO;
      res->chan_stat[0].chan_stat_link = LINK_NO_ESTABL;

      if( satShrdPrm.chnTb.chnUsdFlg )
      {
        switch( satShrdPrm.chnTb.lnkStat )
        {
          case( SIM_LINK_CNCT ):

            res->chan_stat[0].chan_stat_link = LINK_ESTABL;
            break;

          case( SIM_LINK_DROP ):

            res->chan_stat[0].chan_stat_inf2 = LINK_DROPPED;
            break;

          default: /* already handled by pre-setting */

            break;
        }
      }
    }
    /* add bearer description to terminal response */
    if( data_for_term_resp->bearDesc NEQ FALSE AND satShrdPrm.opchPrm )
    {
      if( satShrdPrm.opchType EQ B_CSD )
      {
        res->v_bear_desc = TRUE;
        res->bear_desc.bear_type = BT_CSD;
        res->bear_desc.v_csd_bear_prm = TRUE;
        res->bear_desc.csd_bear_prm = ((T_SAT_CSD_PRM*)satShrdPrm.opchPrm)->csd_bear_prm;
      }
      if( satShrdPrm.opchType EQ B_GPRS )
      {
        res->v_bear_desc = TRUE;
        res->bear_desc.bear_type = BT_GPRS;
        res->bear_desc.v_gprs_bear_prm = TRUE;
        res->bear_desc.gprs_bear_prm = ((T_SAT_GPRS_PRM*)satShrdPrm.opchPrm)->gprs_bear_prm;
      }
    }
    /* add buffer size to terminal response */
    if( data_for_term_resp->bufSize NEQ FALSE )
    {
      res->v_buffer_size = TRUE;
      res->buffer_size = satShrdPrm.buffer_size;
    }
#endif /* FF_SAT_E */

    cmpFlg = FALSE;
  }

/*
 *-------------------------------------------------------------------
 *  send terminal response
 *-------------------------------------------------------------------
 */
  satShrdPrm.owner = OWN_SRC_INV;

  satShrdPrm.ownSAT = FALSE;
  memset( &satShrdPrm.stk_ccp, 0, sizeof(BUF_cap_cnf_parms));


#ifdef TI_PS_FF_AT_P_CMD_CUST
  if (satShrdPrm.cust1StkCmd != (void *)0)
  {
      ACI_MFREE(satShrdPrm.cust1StkCmd); /* Free the dynamically allocated Cust1 Stk Cmd memory */

      satShrdPrm.cust1StkCmd = (void *)0;
      satShrdPrm.cust1StkCmdLen = 0;
  }
#endif /* TI_PS_FF_AT_P_CMD_CUST */

  if( psaSAT_STKResponse() < 0 )  /* STK command response */
  {
    TRACE_EVENT( "FATAL RETURN SAT in send term resp" );
    CCD_END;
    return( FALSE );
  }

  CCD_END;
  return (TRUE);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SATF                |
|                                 ROUTINE : psaSAT_dasmMECmd        |
+-------------------------------------------------------------------+

  PURPOSE : disassemble ME proactive SIM command.
*/

GLOBAL BOOL psaSAT_dasmMECmd ( BUF_cmd_prms *cmdPrm )
{
  UBYTE               ccdRet, err;
  T_ACI_SAT_TERM_RESP resp_data;

  BOOL aciOnlyEvents; /* holds information whether only ACI events are in SET UP 
                         EVENT LIST */
  BOOL sendRes;           /* holds temporaly return result whether to send response
                         or not */ 

  /* response message in the case of SET UP EVENT LIST, only ACI events */
  static UBYTE TRSetEventOk [12]  =
  {
   0x81,           /* command details tag                */
   0x03,           /* command details length             */
   0x00,           /* will be updated to current cmd num */
   0x05,           /* command SETUP EVENT LIST           */
   0x00,           /* not used                           */
   0x82,           /* device details tag                 */
   0x02,           /* device details length              */
   0x82,           /* source ME                          */
   0x81,           /* destination SIM                    */
   0x83,           /* result tag                         */
   0x01,           /* result length                      */
   0x00            /* result OK                          */
  };

  TRACE_FUNCTION("psaSAT_dasmMECmd");
  
  psaSAT_InitTrmResp( &resp_data );

#ifdef TI_PS_FF_AT_P_CMD_CUST
  if ((satShrdPrm.cmdDet.cmdType NEQ SAT_CMD_EVENT_LIST) AND /* ACI has to process the event before */
      (simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_NORMAL_BEHAVIOUR))     /* and NOT Cust1 behaviour */
#else
  if (satShrdPrm.cmdDet.cmdType NEQ SAT_CMD_EVENT_LIST)
#endif /* TI_PS_FF_AT_P_CMD_CUST */
    cmhSAT_STKUsrNtfy();

  if( cmdPrm EQ NULL ) /* no parameter */
  {
    memset(&satShrdPrm.stkCmdPrm, 0,
           sizeof(satShrdPrm.stkCmdPrm));
  }
  else
  {
    memcpy( &satShrdPrm.stkCmdPrm, cmdPrm,
            sizeof(satShrdPrm.stkCmdPrm));

    ccdRet = ccd_decodeMsg (CCDENT_SAT,
                            DOWNLINK,
                            (T_MSGBUF *) &satShrdPrm.stkCmdPrm,
                            (UBYTE    *) _decodedMsg,
                            satShrdPrm.cmdDet.cmdType );

    /* check for CCD error */
    if( ccdRet NEQ ccdOK )
    {
      TRACE_EVENT_P1( "CCD Decoding Error: %d", ccdRet );

      err = psaSAT_ccdErrChk();
      if( err NEQ 0 )
      {
        psaSAT_SendTrmResp( err, &resp_data );
        return( FALSE );
      }
    }
  }

  /* continue processing */
  switch( satShrdPrm.cmdDet.cmdType )
  {
    case( SAT_CMD_SETUP_CALL ):
      {
        MCAST( calCmd, SETUP_CALL );

        return( cmhSAT_setupCall( calCmd ));
      }

    case( SAT_CMD_SEND_SS ):
      {
        MCAST( ssCmd, SEND_SS );

        return( cmhSAT_sendSS( ssCmd ));
      }

    case( SAT_CMD_SEND_USSD ):
      {
        MCAST( ussdCmd, SEND_USSD );

        return( cmhSAT_sendUSSD( ussdCmd ));
      }

    case( SAT_CMD_SEND_SMS ):
      {
        MCAST( smCmd, SEND_SM );

        return( cmhSAT_sendSM( smCmd ));
      }

    case( SAT_CMD_SEND_DTMF ):
      {
        MCAST( dtmfCmd, SEND_DTMF );

        return( cmhSAT_sendDTMF( dtmfCmd ));
      }

    case( SAT_CMD_EVENT_LIST ):
      {
        MCAST( setEvent, SETUP_EVENT );

        if (cmhSAT_setupEvent_Test( setEvent, &aciOnlyEvents ))
        {
          /* ACI-SPR-18200: If there are no Events to be processed by MMI
             ACI bypasses the MMI functionality and returns TR by itself */

          /* something went wrong, temp event list is empty --> TR(30) */
          if ( satShrdPrm.event.temp_list EQ 0L )
          {
            psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
            return FALSE;
          }

          if ( aciOnlyEvents ) 
          {
            /* prepare response to signal OK to SATK */
            TRSetEventOk[2] = satShrdPrm.cmdDet.cmdNr;
            satShrdPrm.setPrm[satShrdPrm.owner].stkCmd = TRSetEventOk;
            satShrdPrm.setPrm[satShrdPrm.owner].stkCmdLen = 12;

            /* inform SIM about acknowledged event list */
            sendRes = cmhSAT_CheckSetEventResp( );    

            if (sendRes) 
            {
              if( psaSAT_STKResponse() < 0 )  /* respond to STK command */
              {
                TRACE_EVENT( "FATAL RETURN - TR could not be send" );
              }
            }
          }
          else /* signal MMI to process further events */
          {
            cmhSAT_STKCmdInd();
          }
          return TRUE;
        }
        else
        {
          if ( satShrdPrm.event.temp_list NEQ 0L )
          {
            satShrdPrm.event.temp_list = 0L; /* reinitialize temp_event_list */
            psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
          }
        }
      }
      break;

    case( SAT_CMD_RUN_AT ):
      {
        MCAST( calCmd, RUN_AT );

        return( cmhSAT_runAt( calCmd ));
      }

    case( SAT_CMD_LAUNCH_BROWSER ):
      {
        MCAST( launchBrowserCmd, LAUNCH_BROWSER );

        return( cmhSAT_launchBrowser( launchBrowserCmd ));
      }

    case( SAT_CMD_PROV_LOC_INFO):
      {
        return( cmhSAT_provLocalInfo());
      }
#ifdef FF_SAT_E
    case( SAT_CMD_OPEN_CHANNEL ):
      {
        MCAST( opchCmd, OPEN_CHANNEL );

        return( cmhSAT_OpenChannelReq( opchCmd ));
      }
#endif /* FF_SAT_E */    
}
  return( FALSE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SATF                |
|                                 ROUTINE : psaSAT_SSResComp        |
+-------------------------------------------------------------------+

  PURPOSE : SS result component handling.
*/

GLOBAL void psaSAT_SSResComp ( T_res_comp* resCmp )
{
  UBYTE  addLen = 0;
  T_ACI_SAT_TERM_RESP resp_data;

  psaSAT_InitTrmResp( &resp_data );

  /* Add the operation code and parameters to terminal response
  (reference: section 12.12.1 "Additional information for SEND SS" of 11.14)
  */
  if( resCmp -> v_sequence         AND
      resCmp -> sequence.v_op_code )
  {
    ssResPrms[0] = resCmp -> sequence.op_code;

    if( resCmp -> sequence.params.l_params )
    {
      addLen = resCmp -> sequence.params.l_params >> 3;

      memcpy( &ssResPrms[1], resCmp -> sequence.params.b_params, addLen);
    }
    ++addLen;
  }
  resp_data.add = ssResPrms;
  resp_data.addLen = addLen;

  psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SATF                |
|                                 ROUTINE : psaSAT_SSErrComp        |
+-------------------------------------------------------------------+

  PURPOSE : SS error result component handling.
*/

GLOBAL void psaSAT_SSErrComp ( T_fac_inf* errCmp, BOOL is_fac_ussd )
{
  UBYTE  ccdRet;
  UBYTE  addLen = 1;
  T_ACI_SAT_TERM_RESP resp_data;

  MCAST( com, COMP_SAT );
  
  TRACE_FUNCTION("psaSAT_SSErrComp()");

  psaSAT_InitTrmResp( &resp_data );
  *ssResPrms = ADD_NO_CAUSE;

  memset( com, 0, sizeof( T_COMP_SAT ));

  ccdRet = ccd_decodeMsg (CCDENT_FAC,
                          DOWNLINK,
                          (T_MSGBUF *) errCmp,
                          (UBYTE    *) _decodedMsg,
                          COMP_SAT);

  if( ccdRet NEQ ccdOK )
  {
    TRACE_EVENT_P1("CCD Decoding Error: %d",ccdRet );
  }
  else
  {
    if( com->err_comp_sat.err_desc_sat.err_params_sat.
        l_err_params_sat )
    {
      addLen = com->err_comp_sat.err_desc_sat.err_params_sat.
               l_err_params_sat >> 3;

      memcpy( ssResPrms,com->err_comp_sat.err_desc_sat.err_params_sat.
              b_err_params_sat, addLen );
    }
  }
  resp_data.add = ssResPrms;
  resp_data.addLen = addLen;

  if( is_fac_ussd )
  {
    psaSAT_SendTrmResp( RSLT_USSD_ERR, &resp_data );
  }
  else
  {
    psaSAT_SendTrmResp( RSLT_SS_ERR, &resp_data );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SATF                |
|                                 ROUTINE : psaSAT_SSRejComp        |
+-------------------------------------------------------------------+

  PURPOSE : SS reject component handling.
*/

GLOBAL void psaSAT_SSRejComp ( UBYTE cRejectInfo )
{
  UBYTE               addLen    = 1;
  T_ACI_SAT_TERM_RESP resp_data;

  TRACE_FUNCTION("psaSAT_SSRejComp()");

  psaSAT_InitTrmResp( &resp_data );
  *ssResPrms = ADD_NO_CAUSE;

  resp_data.add = ssResPrms;
  resp_data.addLen = addLen;

  psaSAT_SendTrmResp( cRejectInfo, &resp_data );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SATF                |
|                                 ROUTINE : psaSAT_ss_end_ind       |
+-------------------------------------------------------------------+

  PURPOSE : handles SAT responses to SAT after a SAT originated SS.
*/

GLOBAL T_ACI_RETURN psaSAT_ss_end_ind ( SHORT sId,
                                        T_COMPONENT    *com, 
                                        T_MNSS_END_IND *mnss_end_ind, 
                                        BOOL           is_fac_ussd )
{
  TRACE_FUNCTION("psaSAT_ss_end_ind()");

  if( com->v_res_comp )
  {
    if( is_fac_ussd EQ FALSE ) /* not for USSD !! */
    {
//TISH, patch for ASTec20762
//start
      psaSS_dasmResultCmp( sId, &com->res_comp );
//end  
      psaSAT_SSResComp( &com->res_comp );
    }
    else
    {
      psaSS_dasmResultCmp( sId, &com->res_comp );
    }
    return( AT_EXCT );
  }

  if( com->v_err_comp )
  {
    ssShrdPrm.stb[sId].failType = SSF_SS_ERR;
    psaSAT_SSErrComp( &mnss_end_ind -> fac_inf, is_fac_ussd );
    return( AT_FAIL );
  }

  if( com->v_rej_comp )
  {
    UBYTE cRejectInfo = RSLT_NTW_UNAB_PROC;

    /* check for protocol incompatibility for USSD */
    if ( (is_fac_ussd) AND
         (com->rej_comp.v_inv_problem) )
    {
      switch( com->rej_comp.inv_problem )
      {
        case 0x01 :  /* invoke problem code #1 */
        {
          if (psaSS_asmUSSDProt1(sId))
          {
            /* facility with Protocole 1 format has been sent: wait for answer */
            return( AT_CMPL );
          }
          cRejectInfo = RSLT_NTW_UNAB_PROC;
          break;
        }
        case 0x02 : /* invoke problem code #2 */
        {
          cRejectInfo = RSLT_USSD_ERR;
          break;
        }
        default :
        {
          TRACE_EVENT_P1("psaSAT_ss_end_ind(): unknown invoke problem: %x", com->rej_comp.inv_problem );
          break;
        }        
      }
      ssShrdPrm.stb[sId].failType = SSF_INV_PRB;
    }
    
    /* check for protocol incompatibility for USSD */
    if ((is_fac_ussd EQ FALSE) AND (com->rej_comp.v_gen_problem))
    {
      ssShrdPrm.stb[sId].failType = SSF_GEN_PRB;
      /* for 27.22.4.11.1 Seq. 1.3 SEND SS */
      if (com->rej_comp.gen_problem EQ GEN_PROB_UNRECOG_CMP) 
      {
        cRejectInfo = RSLT_SS_ERR;
      }
    }

    psaSAT_SSRejComp( cRejectInfo );
    return( AT_FAIL );
  }
  return(AT_FAIL);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SATF                |
|                                 ROUTINE : psaSAT_ccdErrChk        |
+-------------------------------------------------------------------+

  PURPOSE : CCD error checking. The function returns a SAT result
            value unequal to 0 if the operation is must be terminated.
            Otherwise the condition is only a warning and the operation
            should be proceeded.
*/

GLOBAL UBYTE psaSAT_ccdErrChk ( void )
{
  USHORT parLst[MAX_ERR_PAR];
  UBYTE  ccdErr;
  UBYTE  fstFlg = TRUE;

  TRACE_FUNCTION("psaSAT_ccdErrChk()");

/*
 *-------------------------------------------------------------------
 *  handling of first error
 *-------------------------------------------------------------------
 */
  do
  {
    memset (parLst,0, sizeof (parLst));

    if( fstFlg )
    {
      ccdErr = ccd_getFirstError (CCDENT_SAT, parLst);
      fstFlg = FALSE;
    }
    else

      ccdErr = ccd_getNextError  (CCDENT_SAT, parLst);

    switch (ccdErr)
    {
      case( ERR_IE_NOT_EXPECTED ):     /* unexpected element */
        if( !((UBYTE)parLst[0] & 0x80) )
        {
          cmpFlg = TRUE;
          break;
        }
        /* otherwise continue, no break */
        /*lint -fallthrough*/
      case( ERR_COMPREH_REQUIRED ):    /* comprehension required     */

        return( RSLT_UNKN_DATA );

      case( ERR_MAND_ELEM_MISS ):      /* mandatory elements missing */

        return( RSLT_ERR_REQ_VAL );
    }
  } while( ccdErr NEQ ERR_NO_MORE_ERROR );

  return( 0 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SAT                 |
|                                 ROUTINE : psaSAT_DumpCmd          |
+-------------------------------------------------------------------+

  PURPOSE : Dump SAT command to debug output.

*/

GLOBAL void psaSAT_DumpCmd ( T_stk_cmd * cmd )
{
#define SAT_TRC_LEN 32

  CHAR  strBuf[2*SAT_TRC_LEN+1];  /* holds string buffer */
  ULONG idx, cnt, mcnt;           /* buffer index */
  CHAR  *pDest;                   /* points to destination */

  TRACE_FUNCTION("psaSAT_DumpCmd()");

/*
 *-------------------------------------------------------------------
 * format command
 *-------------------------------------------------------------------
 */
  TRACE_EVENT( "SAT CMD SENT/RECEIVED:" );

  mcnt = (ULONG)cmd->l_cmd >> 3;

  if( !mcnt )
  {
    TRACE_EVENT_P1("an empty envelope. (%d)", (ULONG)cmd->l_cmd);
    return;
  }

  for( cnt = 0; cnt < mcnt; cnt += SAT_TRC_LEN )
  {
    pDest = strBuf;

    for( idx = 0; idx < SAT_TRC_LEN AND (idx + cnt) < mcnt; idx++ )
    {
/* Implements Measure#32: Row 1277 */
      pDest += sprintf( pDest, format_2X_str, (UBYTE)cmd->cmd[idx + cnt] );
    }

    *pDest = 0x0;

    TRACE_EVENT_P1("%s", strBuf );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SAT                 |
|                                 ROUTINE : psaSAT_FURegister       |
+-------------------------------------------------------------------+

  PURPOSE : Register a handler for FILE UPDATE.

*/

GLOBAL BOOL psaSAT_FURegister ( T_SAT_FU_FUNC fu_func )
{
  int i;

  TRACE_FUNCTION("psaSAT_FURegister()");

  for (i = 0; i < MAX_FU_OPS; i++)
  {
    if (satShrdPrm.fu_func[i] EQ fu_func)
    {
      TRACE_EVENT ("FU func already registered");
      return TRUE;
    }
    if (satShrdPrm.fu_func[i] EQ NULL)
    {
      satShrdPrm.fu_func[i] = fu_func;
      return TRUE;
    }
  }
  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SAT                 |
|                                 ROUTINE : psaSAT_FUConfirm        |
+-------------------------------------------------------------------+

  PURPOSE : Confirm a handled FILE UPDATE. The parameter is the same
            with which the handler was called and refers to the entry
            of the callback table.

*/

GLOBAL void psaSAT_FUConfirm (int currFU, USHORT resFU)
{
  BOOL fu_active;
  BOOL aci_sent_no_primitive;

  TRACE_EVENT_P2("psaSAT_FUConfirm(): currFU: %d, resFU: %d", currFU, resFU);

  if (resFU EQ SIM_FU_ERROR) /*If there is one error, the rest of the procedure is to be cancelled */
  {
      PALLOC (fu_res, SIM_FILE_UPDATE_RES);

      fu_res->source = SRC_MMI;
      fu_res->fu_rsc = SIM_FU_ERROR;

      if  (satShrdPrm.fu_func_notif)          
        (void)satShrdPrm.fu_func_notif((int)fu_res->fu_rsc,satShrdPrm.fu_ind); /*Sends the final result  to MMI */

      PSENDX (SIM, fu_res);

      return;
  }

  TRACE_EVENT_P1("satShrdPrm.fu_ind: %d", satShrdPrm.fu_ind);

  if (satShrdPrm.fu_ind NEQ NULL AND currFU < MAX_FU_OPS)
  {
    if (currFU < 0)
    {
      if  (satShrdPrm.fu_func_notif)
      {         /*Sends the file update request notification to MMI */
        if  (!satShrdPrm.fu_func_notif(-1,satShrdPrm.fu_ind))  
        {     
          PALLOC (fu_res, SIM_FILE_UPDATE_RES);    /* If rejected, send negative response to SIM */

          fu_res->source = SRC_MMI;
          fu_res->fu_rsc = SIM_FU_ERROR;
          PSENDX (SIM, fu_res);
        }

      }
      currFU = 0;
      /*satShrdPrm.fu_rsc = SIM_FU_ERROR;*/ /* default*/
      fu_active = FALSE;
    }
    else
    {
      ++currFU;
      
      if ( (resFU EQ SIM_FU_SUCC_ADD) AND (satShrdPrm.fu_rsc NEQ SIM_FU_ERROR) )
      {
        satShrdPrm.fu_rsc = resFU;
      }

/*
      switch (resFU)
      {
      case SIM_FU_SUCCESS:
        if (satShrdPrm.fu_rsc EQ SIM_FU_SUCC_ADD)
          break;
      case SIM_FU_SUCC_ADD:
        satShrdPrm.fu_rsc = resFU;
      }
*/
      fu_active = TRUE;
    }

    for ( ; currFU < MAX_FU_OPS; currFU++)
    {
      if (satShrdPrm.fu_func[currFU] EQ NULL)
        continue;
/*      else
        fu_active = TRUE;*/

      aci_sent_no_primitive = satShrdPrm.fu_func[currFU](currFU, satShrdPrm.fu_ind);

      if (!aci_sent_no_primitive)
      {
        return;  /* otherwise, result would be sent directly, and not at the end */
      }
    }

    {
      PALLOC (fu_res, SIM_FILE_UPDATE_RES);

      fu_res->source = SRC_MMI;
      fu_res->fu_rsc = (fu_active)? satShrdPrm.fu_rsc: SIM_FU_SUCCESS;

      if  (satShrdPrm.fu_func_notif)          
        (void)satShrdPrm.fu_func_notif((int)fu_res->fu_rsc,satShrdPrm.fu_ind); /*Sends the final result  to MMI */

      TRACE_EVENT("FREE satShrdPrm.fu_ind");
      PFREE (satShrdPrm.fu_ind);
      satShrdPrm.fu_ind = NULL;

      PSENDX (SIM, fu_res);
      simShrdPrm.fuRef = -1;
    }
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SAT                 |
|                                 ROUTINE : psaSAT_FUNotifyRegister    |
+-------------------------------------------------------------------+

  PURPOSE : Register a handler for FILE UPDATE.

*/

GLOBAL BOOL psaSAT_FUNotifyRegister ( T_SAT_FU_FUNC fu_func )
{

  TRACE_FUNCTION("psaSAT_FUNotifyRegister()");

  if (fu_func)
  {
      satShrdPrm.fu_func_notif = fu_func;
    return TRUE;
  }
  else
    return FALSE;
}



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SAT                 |
|                                 ROUTINE : psaSAT_Init             |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for SAT.

*/

/* MACRO: initializer for set parameter */
#define INIT_SAT_SET_PARM( dest, def )\
  for( LpCnt = 0; LpCnt < OWN_SRC_MAX; LpCnt++ )\
    satShrdPrm.setPrm[LpCnt].dest = def

GLOBAL void psaSAT_Init ( void )
{

  TRACE_FUNCTION("psaSAT_Init()");
/*
 *-------------------------------------------------------------------
 * set default parms
 *-------------------------------------------------------------------
 */
  memset( &satShrdPrm, 0, sizeof(T_SAT_SHRD_PRM));
  satShrdPrm.dur = -1;
  satShrdPrm.SentUSSDid = NO_ENTRY;
  satShrdPrm.fu_rsc = SIM_FU_SUCCESS;
  satShrdPrm.fu_ind = NULL;
  satShrdPrm.sId_pwd_requested = NO_ENTRY;
#ifdef TI_PS_FF_AT_P_CMD_CUST
  satShrdPrm.cust1SimRefreshRespRqd = FALSE;
#endif /* TI_PS_FF_AT_P_CMD_CUST */
  satShrdPrm.locInfo.currPLMN.v_plmn = INVLD_PLMN;
  satShrdPrm.Cbch_EvtDnl = FALSE;
  satShrdPrm.ownSAT = FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : psaSAT_ChkEventList     |
+-------------------------------------------------------------------+

  PURPOSE : Check if the event is monitored and shall alert SAT.

*/

GLOBAL BOOL psaSAT_ChkEventList( UBYTE eventNr )
{
  BOOL event_listed = FALSE;

  TRACE_FUNCTION("psaSAT_ChkEventList()");

  event_listed = (satShrdPrm.event.list >> eventNr ) & 0x01;

  if( event_listed )
    return( TRUE );

  return( FALSE );
}

#ifdef FF_SAT_E 

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : psaSAT_ctbFindActCall   |
+-------------------------------------------------------------------+

  PURPOSE : Return the call id of a call which is in an active state.
            If no call could be found the function returns NO_ENTRY.

*/

GLOBAL SHORT psaSAT_ctbFindActCall( void )
{
  UBYTE ctbIdx;
  SHORT actId = NO_ENTRY;

  TRACE_FUNCTION("psaSAT_ctbFindActCall()");

  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if (ccShrdPrm.ctb[ctbIdx] NEQ NULL)
    {
      switch( psaCC_ctb(ctbIdx)->calStat )
      {
        case( CS_ACT ):
        case( CS_ACT_REQ ):
        case( CS_MDF_REQ ):

          actId = ctbIdx;
          break;
      }
    }
  }

  return( actId );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : psaSAT_gprsFindFreeCntxt|
+-------------------------------------------------------------------+

  PURPOSE : Return a free context id, if busy return NO_ENTRY.

*/
#ifdef GPRS
GLOBAL SHORT psaSAT_gprsFindFreeCntxt( void )
{
  BOOL  cntxtStat[MAX_CID+1];
  SHORT cntxtId[MAX_CID+1];
  SHORT cid = NO_ENTRY;
  UBYTE idx;

  TRACE_FUNCTION("psaSAT_gprsFindFreeCntxt()");

  if( qAT_PlusCGACT( CMD_SRC_LCL, cntxtStat, cntxtId ) NEQ AT_CMPL )

    return( NO_ENTRY );

  for( idx = 0; idx < MAX_CID; idx++ )
  {
    if( cntxtId[idx] NEQ INVALID_CID AND cntxtStat[idx] EQ TRUE ) continue;
    
    cid = (cntxtId[idx] EQ INVALID_CID)? idx+1:cntxtId[idx];
    break;
  }

  return( cid );

}
#endif  /* GPRS */

#endif /* FF_SAT_E */

#endif /* #ifdef SIM_TOOLKIT */

/*==== EOF ========================================================*/
