/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CC_EST
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
|  Purpose :  This Modul defines the functions for the establishment
|             phase of the call control process of the component CC.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CC_EST_C
#define CC_EST_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_CC
/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "message.h"
#include "ccdapi.h"
#include "prim.h"
#include "cnf_cc.h"
#include "mon_cc.h"
#include "pei.h"
#include "tok.h"
#include "cc.h"
#include "cc_em.h"

/*==== EXPORT =====================================================*/
/*==== PROTOTYPE ==================================================*/
/* Implements Measure#  19 */                                       
LOCAL void cc_send_release_cmp (USHORT cause);
LOCAL void cc_send_mncc_release_ind (UBYTE  ti, USHORT cause);
/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_init                    |
+--------------------------------------------------------------------+

  PURPOSE : Initialisation of CC data.

*/

GLOBAL void cc_init (void)
{
#ifdef OPTION_MULTIPLE_INSTANCES

  USHORT i;

  TRACE_FUNCTION ("cc_init()");

  for (i=0;i<MAX_INSTANCES;i++)
    cc_init_data (&data_base[i]);
}
#else

  TRACE_FUNCTION ("cc_init()");
  cc_init_data ();
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : CC_EST                |
| STATE   : code                     ROUTINE : cc_init_data          |
+--------------------------------------------------------------------+

  PURPOSE : Initializes the data for one instance.

*/

GLOBAL void cc_init_data (void)
{
  GET_INSTANCE_DATA;
  USHORT i;

  TRACE_FUNCTION ("cc_init_data()");

  memset (cc_data, 0, sizeof (T_CC_DATA));

  /* No SETUP / EMERGENCY SETUP pending */
  cc_data->stored_setup = NULL;

  /* No CCBS SETUP pending */
  cc_data->stored_ccbs_setup = NULL;
  
  cc_data->channel_mode = NAS_CHM_SIG_ONLY;

  for (i=0;i<MAX_CC_CALLS;i++)
  {
    cc_data->stored_ti_values[i] = NOT_PRESENT_8BIT;
  }

  cc_csf_ms_cap ();

  /*
   * Initialise single numbering scheme
   */
  cc_data->sns_bcpara.bearer_serv = MNCC_BEARER_SERV_SPEECH;
  cc_data->sns_mode = MNCC_SNS_MODE_VOICE;

  /*
   * Initialise connection element for MTC
   */
  cc_data->conn_elem = MNCC_CONN_ELEM_NON_TRANS;

  /* 
   * Initialize setup_reattempt_ti to not present 
   */
  cc_data->setup_reattempt_ti = NOT_PRESENT_8BIT;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_mncc_configure_req      |
+--------------------------------------------------------------------+

  PURPOSE : Configuration of bearer capabilities and subaddress
            by man machine interface.

*/

static const UBYTE def_modem_type [8] = 
{
    MNCC_MT_V21,       /* user rate 300 Baud    ->  modem type V.21    */
    MNCC_MT_V22,       /* user rate 1200 Baud   ->  modem type V.22    */
    MNCC_MT_V22_BIS,   /* user rate 2400 Baud   ->  modem type V.22bis */
    MNCC_MT_V32,       /* user rate 4800 Baud   ->  modem type V32     */
    MNCC_MT_V32,       /* user rate 9600 Baud   ->  modem type V32     */
    MNCC_MT_V32,       /* 12k transparent, not supported now by ACI    */
    MNCC_MT_V23,       /* user rate 12k/75 Baud ->  modem type V23     */
    MNCC_MT_V21,       /* user rate 14.4 kBaud  ->  modem type V21 ??? */
};

GLOBAL void cc_mncc_configure_req (T_MNCC_CONFIGURE_REQ * config)
{

  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_configure_req()");

  /*
   * store own subadress if available
   * 
   * NOTE: T_called_party_sub is the wrong type here in MNCC_CONFIGURE_REQ, 
   *       this should have been T_calling_party_sub as this is that what
   *       the subaddress is used for in the uplink SETUP message later.
   *       Doesn't really matter here as the types are identical, it's 
   *       just somewhat confusing and could be occasionally be changed.
   */
  if (config->called_party_sub.c_subaddr)
  {
    cc_data->my_party_subaddr.v_tos      = TRUE;
    cc_data->my_party_subaddr.tos        = config->called_party_sub.tos;
    cc_data->my_party_subaddr.v_odd_even = TRUE;
    cc_data->my_party_subaddr.odd_even   = config->called_party_sub.odd_even;
    cc_data->my_party_subaddr.c_subaddr  = config->called_party_sub.c_subaddr;
    memcpy (cc_data->my_party_subaddr.subaddr,
            config->called_party_sub.subaddr,
            config->called_party_sub.c_subaddr);
  }
  else
    memset (&cc_data->my_party_subaddr, 0, sizeof (T_M_CC_called_subaddr));
  /*
   * Store CTM support
   */
  cc_data->ctm_ena = config->ctm_ena;
  /*
   * Store Single Numbering Scheme BC parameter and mode
   * if available
   */
  if (config->sns_mode NEQ NOT_PRESENT_8BIT)
  {
   /*
    * Store Connection Element for MTC
    * Note that setting conditionally on sns_mode NEQ NOT_PRESENT_8BIT is 
    * possible because ACI always sets sns_mode to something different from
    * NOT_PRESENT_8BIT when it sends an MNCC_CONFIGURE_REQ.
    * For future proof use of MNCC_CONFIGURE_REQ (e.g. only subaddress is valid!)
    * it is convenient and correct to consider parameters only if they
    * are valid.
    */

    cc_data->conn_elem = config->bcpara.conn_elem;

    cc_data->sns_mode = config->sns_mode;

    if (cc_data->sns_mode EQ MNCC_SNS_MODE_VOICE)
    {
      memset (&cc_data->sns_bcpara, 0 /*NOT_PRESENT_8BIT*/, sizeof (T_MNCC_bcpara));
      cc_data->sns_bcpara.bearer_serv = (cc_data->ctm_ena EQ MNCC_CTM_ENABLED)?
                                        MNCC_BEARER_SERV_SPEECH_CTM: MNCC_BEARER_SERV_SPEECH;
    }
    else
      memcpy (&cc_data->sns_bcpara, &config->bcpara, sizeof (T_MNCC_bcpara));
    /*
     * Set modem type to default values for single numbering scheme
     */
    switch (cc_data->sns_bcpara.bearer_serv)
    {
      case MNCC_BEARER_SERV_FAX:
        cc_data->sns_bcpara.modem_type = M_CC_MT_NONE;
        break;
      case MNCC_BEARER_SERV_SPEECH:
      case MNCC_BEARER_SERV_AUX_SPEECH:
      case MNCC_BEARER_SERV_SPEECH_CTM:
      case MNCC_BEARER_SERV_AUX_SPEECH_CTM:
        break;
      default:
        cc_data->sns_bcpara.modem_type = def_modem_type[config->bcpara.rate-1];
        if (config->bcpara.modem_type == MNCC_MT_V34)
        {
          cc_data->sns_bcpara.modem_type = MNCC_MT_V34;
        }
        break;
    }
  }

  /*
   * define connection element if a preferred one is set by MMI
   */
  if (cc_data->sns_bcpara.conn_elem EQ MNCC_CONN_ELEM_TRANS_PREF)
  {
    /*
     * check only if transparent asynchronous data services are supported
     */
    if (FldGet(cc_data->mscap.datCap1, AsySup))
      cc_data->sns_bcpara.conn_elem = M_CC_CE_TRANSPA;
    else
      cc_data->sns_bcpara.conn_elem = M_CC_CE_RLP;
  }

  /*
   * define connection element if a preferred one is set by MMI
   */
  if (cc_data->sns_bcpara.conn_elem EQ MNCC_CONN_ELEM_NON_TRANS_PREF)
  {
    /*
     * check only if non-transparent asynchronous data services are supported
     */
    if (FldGet (cc_data->mscap.datCap1, RLPSup))
      cc_data->sns_bcpara.conn_elem = M_CC_CE_RLP;
    else
      cc_data->sns_bcpara.conn_elem = M_CC_CE_TRANSPA;
  }

  PFREE (config);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_mncc_prompt_rsp         |
+--------------------------------------------------------------------+

  PURPOSE : A new transaction identifier during network initiated 
            mobile origination call establishment was assigned by 
            upper layers. Under normal conditions this causes the 
            emmission of an MMCC_DATA_REQ (START CC) 

*/

GLOBAL void cc_mncc_prompt_res (T_MNCC_PROMPT_RES * prompt)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_prompt_rsp()");

  /* Check parameters */
  if (prompt->ti >= 7)
  {
    /* Invalid transaction identifier */
/* Implements Measure#  36, 48 */
    cc_send_mncc_release_ind (prompt->ti, 
                                CAUSE_MAKE(DEFBY_STD, 
                                ORIGSIDE_MS, 
                                MNCC_CC_ORIGINATING_ENTITY, 
                                M_CC_CAUSE_INVALID_TI));    

    PFREE (prompt);
    return;
  }

  /* Assign ti and allocate a free entry in the call data */ 
  cc_data->ti = prompt->ti;
  cc_data->index_ti = srv_define_ti();
  if (cc_data->index_ti EQ NOT_PRESENT_8BIT)
  {
    /*
     * No call instance available
     */
/* Implements Measure#  36, 48 */
    cc_send_mncc_release_ind (prompt->ti, MNCC_CAUSE_MAX_NO_CALLS_REACHED);

    PFREE (prompt);
    return;
  }

  /* Send MMCC_PROMPT_RSP */
  {
    PALLOC (prompt_rsp, MMCM_PROMPT_RES); // T_MMCM_ESTABLISH_REQ
    prompt_rsp->ti = prompt->ti;
    PSENDX (MM, prompt_rsp);
  }
  
  /* Send MMCC_DATA_REQ (START CC) */
  
  CCD_START;
  {
    MCAST (start_cc, U_START_CC);
    cc_build_start_cc (start_cc);
    for_start_cc (start_cc);
  }
  CCD_END;

  /* Start Timer T332 */
  TIMERSTART (T332, T332_VALUE);

  PFREE (prompt);
  
  /* Next state is WAIT FOR NETWORK INFO (U0.3) */
  cc_set_state (M_CC_CS_03);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_mncc_prompt_rej         |
+--------------------------------------------------------------------+

  PURPOSE : A new transaction identifier during network initiated 
            mobile origination call establishment could not be 
            assigned by upper layers. MM is informed by
            MMCC_PROMPT_REJ.

*/

GLOBAL void cc_mncc_prompt_rej (T_MNCC_PROMPT_REJ * prompt)
{
  TRACE_FUNCTION ("cc_mncc_prompt_rej()");

  PFREE (prompt);
  
  {
    PALLOC (reject, MMCM_PROMPT_REJ);
    PSENDX (MM, reject);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_mncc_setup_req          |
+--------------------------------------------------------------------+

  PURPOSE : Starting of mobile originated call.

*/

GLOBAL void cc_mncc_setup_req (T_MNCC_SETUP_REQ * setup)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_setup_req()");
  cc_data->index_ti = srv_convert_ti (setup->ti);
  if (cc_data->index_ti NEQ NOT_PRESENT_8BIT)
  {
    /*
     * Transaction identifier already present, this means, we know this ti
     * and CC is not in M_CC_CS_0 state. This is okay for M_CC_CS_06 (CCBS), 
     * for every other state this is an internal failure.
     */
    switch (cc_data->state[cc_data->index_ti])
    {
      case M_CC_CS_06: /* RECALL present */
        /* 
         * Don't use the setup parametes from ACI, instead use 
         * the stored SETUP message in CC.
         */

        cc_data->progress_desc[cc_data->index_ti] = NOT_PRESENT_8BIT;

        /* No ugly CC setup reattempts here */
        cc_data->setup_attempts = MAX_SETUP_ATTEMPTS;
        cc_data->setup_reattempt_ti = NOT_PRESENT_8BIT;

        /* Send stored SETUP message to network and forget it */
        for_pd (cc_data->stored_ccbs_setup);
        cc_data->stored_ccbs_setup = NULL;

        /* Set parameters in cc_data */
        cc_data->call_type [cc_data->index_ti]    = CALL_TYPE_MOC;

        /* initialize disconnect collision flag */
        cc_data->disc_coll [cc_data->index_ti]    = FALSE;

        cc_data->t308_counter [cc_data->index_ti] = 0;
        cc_reset_dtmf ();

        /* Start T303 */
        TIMERSTART (T303, T303_VALUE);

        /* Next state is CALL INIT */
        cc_set_state (M_CC_CS_1);
        break;
    
      default: /* The ti is already in use and it is not CCBS recall */
        {
          PALLOC (rel, MNCC_RELEASE_IND);
          rel->ti = setup->ti;
          rel->cause = M_CC_CAUSE_INVALID_TI;
          rel->c_raw_cause = 0;
          PSENDX (MMI, rel);
        }
        break;
    }
    PFREE (setup);
    return;
  }

  if (setup->ti >= 7)
  {
    /*
     * Invalid transaction identifier
     */
/* Implements Measure#  36, 48 */
    cc_send_mncc_release_ind (setup->ti,
                                CAUSE_MAKE(DEFBY_STD, 
                                ORIGSIDE_MS, 
                                MNCC_CC_ORIGINATING_ENTITY, 
                                M_CC_CAUSE_INVALID_TI));    
    PFREE (setup);
    return;
  }

  /*
   * ti is valid here and not already in use
   */
  cc_data->ti = setup->ti;
  cc_data->index_ti = srv_define_ti ();
  if (cc_data->index_ti EQ NOT_PRESENT_8BIT)
  {
    /*
     * No call instance available
     */
/* Implements Measure#  36, 48 */
    cc_send_mncc_release_ind (setup->ti, MNCC_CAUSE_MAX_NO_CALLS_REACHED);
    PFREE (setup);
    return;
  }

  /*
   * Security check: if bcpara indicates not present,
   * set service to speech
   */
  if (setup->bcpara.bearer_serv EQ NOT_PRESENT_8BIT)
  {
    setup->bcpara.bearer_serv = (cc_data->ctm_ena EQ MNCC_CTM_ENABLED)?
                                MNCC_BEARER_SERV_SPEECH_CTM: MNCC_BEARER_SERV_SPEECH;
  }
  /*
   * Security check: if prio indicates emergency call,
   * set service to speech or speech with CTM
   */
  if (setup->prio NEQ MNCC_PRIO_NORM_CALL)
  {
    switch (setup->bcpara.bearer_serv)
    {
    case MNCC_BEARER_SERV_SPEECH:
    case MNCC_BEARER_SERV_SPEECH_CTM:
      break;
    case MNCC_BEARER_SERV_AUX_SPEECH_CTM:
      setup->bcpara.bearer_serv = MNCC_BEARER_SERV_SPEECH_CTM;
      break;
    default:
      setup->bcpara.bearer_serv = MNCC_BEARER_SERV_SPEECH;
      break;
    }
  }

  if (cc_check_capabilities (&setup->bcpara) EQ FALSE OR 
      cc_check_capabilities ((T_MNCC_bcpara *)&setup->bcpara2) EQ FALSE)
  {
    /*
     * MS doesn't support the requested services.
     */
    PALLOC ( rel, MNCC_RELEASE_IND);

    rel->ti    = setup->ti;
    rel->cause = CAUSE_MAKE(DEFBY_STD, 
                            ORIGSIDE_MS, 
                            MNCC_CC_ORIGINATING_ENTITY, 
                            M_CC_CAUSE_BEARER_NOT_IMPLEM);
    rel->c_raw_cause = 0;

    PSENDX (MMI, rel);

    srv_free_ti ();

    PFREE (setup);
  }
  else
  {
    PALLOC (est, MMCM_ESTABLISH_REQ); /* T_MMCM_ESTABLISH_REQ */
    cc_build_bc (&cc_data->bc1, &cc_data->serv1, &setup->bcpara);
    cc_data->ri = setup->ri;
    cc_data->bcpara1 = setup->bcpara;
    memcpy (&cc_data->bcpara2, &setup->bcpara2, sizeof (T_MNCC_bcpara)); 

    if (cc_data->ri EQ NOT_PRESENT_8BIT)
      cc_data->serv2 = NOT_PRESENT_8BIT;
    else
      cc_build_bc (&cc_data->bc2, &cc_data->serv2, (T_MNCC_bcpara *)&setup->bcpara2);
    memcpy (&cc_data->bcpara2, &setup->bcpara2, sizeof (T_MNCC_bcpara));
    est->org_entity = NAS_ORG_ENTITY_CC;
    est->ti = setup->ti;

    /* Set establ_serv according to chosen bearer capability */
    switch (cc_data->bcpara1.bearer_serv)
    {
      case MNCC_BEARER_SERV_SPEECH:
          est->estcs = MMCM_ESTCS_MOB_ORIG_SPCH;
        break;

      case MNCC_BEARER_SERV_ASYNC:
        if (cc_data->bcpara1.conn_elem == MNCC_CONN_ELEM_TRANS ||
          cc_data->bcpara1.conn_elem == MNCC_CONN_ELEM_TRANS_PREF)
        {
            est->estcs = MMCM_ESTCS_MOB_ORIG_DATA;
        }
        else
        {
            est->estcs = MMCM_ESTCS_MOB_ORIG_DATA;
        }
        break;

      case MNCC_BEARER_SERV_FAX:
          est->estcs = MMCM_ESTCS_MOB_ORIG_DATA;
        break;

      default:
          est->estcs = MMCM_ESTCS_MOB_ORIG_SPCH;
        break;
    }

    switch (cc_data->serv1)
    {
      case MNCC_SERV_DATA:
        if (FldGet(cc_data->mscap.datCap2, DHRSup))
        {
          /*
           * Halfrate support for data
           */
          switch (setup->bcpara.rate)
          {
            case M_CC_UR_0_3_KBIT:
            case M_CC_UR_1_2_KBIT:
            case M_CC_UR_2_4_KBIT:
            case M_CC_UR_4_8_KBIT:
            case M_CC_UR_1_2_KBIT_V23:
              est->estcs = MMCM_ESTCS_MOB_ORIG_DATA;
              break;
            default:
              est->estcs = MMCM_ESTCS_MOB_ORIG_DATA;
              break;
          }
        }
        else
          est->estcs = MMCM_ESTCS_MOB_ORIG_DATA;
        break;

      default: /* SERV_SPEECH */
        if (setup->prio EQ MNCC_PRIO_NORM_CALL)
          est->estcs = MMCM_ESTCS_MOB_ORIG_SPCH;
        else
          est->estcs = MMCM_ESTCS_EMERGE;
        break;
    }

    /* Data solely needed for possible redial attempt */
    cc_data->estcs              = est->estcs;
    cc_data->setup_attempts     = 0;

    srv_free_stored_setup ();
    cc_data->progress_desc[cc_data->index_ti] = NOT_PRESENT_8BIT;
    cc_data->call_type [cc_data->index_ti]    = CALL_TYPE_MOC;
    // PATCH LE 10.04.00
    // initialize disconnect collision flag
    cc_data->disc_coll [cc_data->index_ti]    = FALSE;
    // END PATCH LE 10.04.00
    cc_data->t308_counter [cc_data->index_ti] = 0;
    cc_reset_dtmf ();

    CCD_START;
    if (setup->prio EQ MNCC_PRIO_NORM_CALL)
    {
      MCAST (setup_msg, U_SETUP);

      cc_build_setup (setup_msg, setup);
      for_setup (setup_msg);
    }
    else
    {
      MCAST (emergency_setup_msg, U_EMERGE_SETUP);

      cc_build_emergency_setup (emergency_setup_msg);
      for_emergency_setup (emergency_setup_msg);
    }
    CCD_END;

    cc_set_state (M_CC_CS_01);
    for_est_req (est);
    TIMERSTART (T303, T303_VALUE);
    PFREE (setup);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_mncc_alert_req          |
+--------------------------------------------------------------------+

  PURPOSE : The mobile side indicates alerting.

*/

GLOBAL void cc_mncc_alert_req (T_MNCC_ALERT_REQ * alert)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_alert_req()");

  if ((cc_data->index_ti = srv_convert_ti (alert->ti))
      EQ NOT_PRESENT_8BIT)
  {
    PFREE (alert);
    return;
  }

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_9:
      CCD_START;
      {
        MCAST (alert_msg, U_ALERT);
        
        cc_build_alert (alert_msg);
        cc_set_state (M_CC_CS_7);
        for_alert (alert_msg);
      }
      CCD_END;

      EM_CC_ALERTING_SENT;

      PFREE (alert);
      break;

    default:
      PFREE (alert);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_mncc_setup_res          |
+--------------------------------------------------------------------+

  PURPOSE : The mobile side indicates call acceptance.

*/

GLOBAL void cc_mncc_setup_res (T_MNCC_SETUP_RES * setup_res)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_setup_res()");

  if ((cc_data->index_ti = srv_convert_ti (setup_res->ti))
      EQ NOT_PRESENT_8BIT)
  {
    PFREE (setup_res);
    return;
  }

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_7:
    case M_CC_CS_9:
      CCD_START;
      {
        MCAST (connect, U_CONNECT);
        cc_build_connect (connect);
        cc_set_state (M_CC_CS_8);
        TIMERSTART (T313, T313_VALUE);
        for_connect (connect);
      }
      CCD_END;

      EM_CC_CONNECT_SENT;
      
      PFREE (setup_res);
      break;

    default:
      PFREE (setup_res);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_alert                   |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming alert message.

*/

GLOBAL void cc_alert (T_D_ALERT * alert)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_alert()");

  EM_CC_ALERTING_RECEIVED;

  switch (cc_data->state[cc_data->index_ti])
  {
    /*
     * CS_0 is handled by the formatter
     */
    case M_CC_CS_1:
    case M_CC_CS_3:
      if (cc_check_error_flag ())
      {
        PALLOC (alert_ind, MNCC_ALERT_IND);
        TIMERSTOP (TIMER_CC);

        if (alert->v_progress)
        {
          cc_data->progress_desc[cc_data->index_ti] = 
            alert->progress.progress_desc;
        }

        cc_build_mncc_alert_ind (alert, alert_ind);
        PSENDX (MMI, alert_ind);

        cc_build_facility_ind (MNCC_FAC_IN_ALERT, alert->v_facility, &alert->facility);
        cc_build_user_user_ind (MNCC_USER_IN_ALERT, alert->v_user_user, 
                                &alert->user_user);
        CCD_END;
        srv_free_stored_setup ();
        cc_set_state (M_CC_CS_4);
      }
      break;

    default:
      CCD_END;
/* Implements Measure#  3 and streamline encoding */
      cc_send_status (MNCC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_call_proceeding         |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming call proceed message.

*/

GLOBAL void cc_call_proceeding (T_D_CALL_PROCEED * proceed)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_call_proceeding()");

  EM_CC_CALL_PROCEEDING_RECEIVED;

  switch (cc_data->state[cc_data->index_ti])
  {
    /*
     * CS_0 is handled by the formatter
     */
    case M_CC_CS_1:
      if (cc_check_error_flag ())
      {
        TIMERSTOP (TIMER_CC); /* timer T303 */
        if (cc_moc_compatibility (proceed) EQ OKAY)
        {
          PALLOC (proceed_ind, MNCC_CALL_PROCEED_IND);

          cc_build_mncc_proceed_ind (proceed, proceed_ind);
          PSENDX (MMI, proceed_ind);

          cc_build_facility_ind (MNCC_FAC_IN_CALL_PROCEED, 
                                   proceed->v_facility, &proceed->facility);
          if (proceed->v_progress)
          {
             cc_data->progress_desc[cc_data->index_ti] = 
               proceed->progress.progress_desc;
          }
    
          CCD_END;
          /* 
           * - start T310 unless the CALL PROCEEDING message contains a progress indicator IE 
           *   specifying progress description #1, #2, Ph2 handling is not done yet (#64), see
           *   also issue 4967
           * - start T310 unless a PROGRESS message with #1, 2, 64 has been 
           *   received, this is achieved by remembering the progress description in cc_data
           * - the case when 1. PROGRESS and 2. CALL PROCEEDING with e.g. #4 is handled by
           *   interpreting 5.2.1.1.3 of 04.08 in the manner that a 2nd progress description
           *   superseeds the first one. See also issue 4965 / CC-FIX-4965.
           */
          /* start T310 with Ph2 unless progress description value #1, #2 or #64 has been received */
          {
            UBYTE prog_desc = cc_data->progress_desc[cc_data->index_ti];
            if ((prog_desc NEQ MNCC_PROG_NO_END_TO_END_PLMN)
                  AND
               (prog_desc NEQ MNCC_PROG_DEST_NON_PLMN)
                  AND
               (prog_desc NEQ MNCC_PROG_QUEUEING))
            {
                TIMERSTART (T310, T310_VALUE);
            }
          }
          if (proceed->v_progress AND
              proceed->progress.v_progress_desc AND 
              proceed->progress.progress_desc EQ M_CC_PROG_INBAND_AVAIL)
          {
            /* 
             * Avoid more call establishment attempts if there was
             * inband announcement heard by the user. 
             * The user should have no way to realize that more than one
             * call attempt may be taken by CC.
             */
            cc_data->setup_attempts = MAX_SETUP_ATTEMPTS;
          }

          cc_set_state (M_CC_CS_3);
        }
        else /* bearer capability check on CALL PROCEEDING failed */
        {
          CCD_END;
          CCD_START;
          {
            MCAST (disconnect, U_DISCONNECT);
            PALLOC (rej_ind, MNCC_REJECT_IND);

            rej_ind->ti  = cc_data->ti;
            /* 
             * GSM 04.08 does not specify what to do / which cause to use;
             * Condat decided to use normal call clearing with cause #88
             */
            rej_ind->cause = CAUSE_MAKE(DEFBY_STD, 
                                        ORIGSIDE_MS, 
                                        MNCC_CC_ORIGINATING_ENTITY, 
                                        MNCC_CAUSE_INCOMPAT_DEST); 
            PSENDX (MMI, rej_ind);

            cc_build_disconnect (disconnect, 
                                 CAUSE_MAKE(DEFBY_STD, 
                                            ORIGSIDE_MS, 
                                            MNCC_CC_ORIGINATING_ENTITY,
                                            MNCC_CAUSE_INCOMPAT_DEST),
                                 NULL, MNCC_SS_VER_NOT_PRES);
            cc_set_state (M_CC_CS_11);
            for_disconnect (disconnect);
          }
          CCD_END;
          TIMERSTART (T305, T305_VALUE);
        }
      }
      break;

    default:
      CCD_END;
/* Implements Measure#  3 and streamline encoding */
      cc_send_status (MNCC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_connect                 |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming connect message.

*/

GLOBAL void cc_connect (T_D_CONNECT * connect)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_connect()");

  switch (cc_data->state[cc_data->index_ti])
  {  
    /*
     * CS_0 is handled by the formatter
     */
    case M_CC_CS_1:
    case M_CC_CS_3:
    case M_CC_CS_4:
      if (cc_check_error_flag ())
      {
        PALLOC (setup_cnf, MNCC_SETUP_CNF);

        TIMERSTOP (TIMER_CC);

        if (connect->v_progress)
        {
          cc_data->progress_desc[cc_data->index_ti] = 
            connect->progress.progress_desc;
        }

        cc_build_mncc_setup_cnf (connect, setup_cnf);
        PSENDX (MMI, setup_cnf);
        cc_build_facility_ind (MNCC_FAC_IN_CONNECT, connect->v_facility,
                               &connect->facility);
        cc_build_user_user_ind (MNCC_USER_IN_CONNECT, connect->v_user_user,
                                &connect->user_user);
        CCD_END;
        CCD_START;
        {
          MCAST (connect_ack, B_CONNECT_ACK);

          connect_ack->msg_type = B_CONNECT_ACK;
          for_connect_ack (connect_ack);
        }
        CCD_END;
        srv_free_stored_setup ();
        cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_IDLE;
        cc_data->mpty_state[cc_data->index_ti] = M_CC_MPTY_IDLE;
        cc_set_state (M_CC_CS_10);
      }

      EM_CC_CONNECT_RECEIVED;

      break;

    default:
/* Implements Measure#  3 and streamline encoding */
    CCD_END;
      cc_send_status (MNCC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_connect_ack             |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming connect acknowledge message.

*/

GLOBAL void cc_connect_ack (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_connect_ack()");

  switch (cc_data->state[cc_data->index_ti])
  {
    /*
     * CS_0 is handled by the formatter
     */
    case M_CC_CS_8:
      if (cc_check_error_flag ())
      {
        PALLOC (setup_comp, MNCC_SETUP_COMPL_IND);

        CCD_END;

        TIMERSTOP (TIMER_CC);

        setup_comp->ti  = cc_data->ti;
        setup_comp->cause = MNCC_CAUSE_SUCCESS;
        PSENDX (MMI, setup_comp);

        cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_IDLE;
        cc_data->mpty_state[cc_data->index_ti] = M_CC_MPTY_IDLE;
        cc_set_state (M_CC_CS_10);

        EM_CC_CONNECT_ACKNOWLEDGE_RECEIVED;

      }
      break;

    default:
      CCD_END;
/* Implements Measure#  3 and streamline encoding */
      cc_send_status (MNCC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_progress                |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming progress message.

*/

GLOBAL void cc_progress (T_D_PROGRESS * progress)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_progress()");

  switch (cc_data->state[cc_data->index_ti])
  {
    /*
     * CS_0 is handled by the formatter
     */
    case M_CC_CS_1:
    case M_CC_CS_3:
    case M_CC_CS_4:
    case M_CC_CS_6:
    case M_CC_CS_7:
    case M_CC_CS_8:
    case M_CC_CS_9:
    case M_CC_CS_11:
    case M_CC_CS_12:
    case M_CC_CS_19:
      if (cc_check_error_flag ())
      {
        PALLOC (progress_ind, MNCC_PROGRESS_IND);

        TIMERSTOP (TIMER_CC);
        
        /* Progress indicator IE here is mandatory IE, so no 
         * checks for presence need to be done here */
        cc_data->progress_desc[cc_data->index_ti] = 
          progress->progress.progress_desc;
       
        cc_build_mncc_progress_ind (progress, progress_ind);
        PSENDX (MMI, progress_ind);
        cc_build_user_user_ind (MNCC_USER_IN_PROGRESS, progress->v_user_user,
                                &progress->user_user);
        CCD_END;
      }

      EM_CC_PROGRESS_RECEIVED;

      break;

    default:
      CCD_END;
/* Implements Measure#  3 and streamline encoding */
      cc_send_status (MNCC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_cc_establishment        |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming CC ESTABLISHMENT message.
            Because the only interesting part in this message
            is the setup container, only the decoded SETUP message
            is delivered here.

*/

GLOBAL void cc_cc_establishment (T_U_SETUP * setup)
{
  GET_INSTANCE_DATA;	
  TRACE_FUNCTION ("cc_cc_establishment()");

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_03: /* WAIT FOR NW INFO */
      
      /* Check errors delivered by formatter */
      switch (cc_data->error) 
      {
        case M_CC_CAUSE_INVALID_MAND_INFO:
        case M_CC_CAUSE_COND_INFO_ELEM:
          CCD_END;
          /* Implements Measure#  7 and streamline encoding*/
          cc_send_status (cc_data->error);  
          return;

        default: /* No error until now */
          /* Stop TIMER T332 */
          TIMERSTOP (TIMER_CC);
   
          if (cc_data->stored_ccbs_setup NEQ NULL) 
          {
            /* 
             * There is only room for one stored CCBS SETUP message
             * for all instances. In case this is already occupied, 
             * the call is released with cause "user busy".
             * In this case the CCBS recall is not lost, but suspended
             * by the network until the mobile becomes idle.
             */
            CCD_END;
           
            /* Send RELEASE COMPLETE */
            CCD_START;
            {
              MCAST (rel_com, U_RELEASE_COMP);
              cc_build_release_complete (rel_com, CAUSE_MAKE(DEFBY_STD, 
                                                    ORIGSIDE_MS, 
                                                    MNCC_CC_ORIGINATING_ENTITY,
                                                    MNCC_CAUSE_USER_BUSY));
              for_release_complete (rel_com);
            }
            CCD_END;

            /* Inform MMI */
            {
              PALLOC (release, MNCC_RELEASE_IND);
              release->ti    = cc_data->ti;
              release->cause = MNCC_CAUSE_MAX_NO_RECALLS_REACHED;
              release->c_raw_cause = 0;
              PSENDX (MMI, release);
            }

            /* Release MM connection */
            for_rel_req ();

            /* Next state is NULL */
            cc_set_state (M_CC_CS_0);
            return;
          }

          /* Check bearer capabilities, do basic recall alignment */
          if (cc_basic_service_align (setup) EQ ERROR)
          {
            /* Incompatible bearer capabilities. Clear call. */
            CCD_END;
            /* Implements Measure#  19 */
            cc_send_release_cmp (CAUSE_MAKE(DEFBY_STD,
                                           ORIGSIDE_MS, 
                                           MNCC_CC_ORIGINATING_ENTITY, 
                                           MNCC_CAUSE_INCOMPAT_DEST));
            return;
          }

          /* 
           * "If the CC Capabilities in the Setup Container IE is different
           * to that supported by the mobile station, the mobile station 
           * shall modify the CC Capabilities in the SETUP message to indicate
           * the true capabilities of the mobile station" [GSM 04.08 5.2.3.2]
           */
          setup->call_ctrl_cap = cc_data->call_ctrl_cap; /* Struct copy */

          /* 
           * Handle facility alignment 
           * ("Simple recall alignment",
           *  "Advanced recall alignment" and 
           *  "Recall alignment not essential").
           */

          /* "Simple recall alignment" handled automatically, do nothing */

          if (setup->v_fac_adv) 
          {
            /* Advanced recall alignment not supported. Reject call. */
            CCD_END;
           /* Implements Measure#  19 */
            cc_send_release_cmp (CAUSE_MAKE(DEFBY_STD,
                                           ORIGSIDE_MS, 
                                           MNCC_CC_ORIGINATING_ENTITY,
                                           MNCC_CAUSE_FACILITY_REJECT));
           
            return;
          }

          /* 
           * The recall alignment not essential facility IE 
           * need not be handled directly here, it is simply 
           * an optional IE which was ignored by CCD.
           * The only handling we do is to delete the SS-Version 
           * indicator if there are no remaining facility elements.
           */
          if (!setup->v_facility)
            setup->v_ss_version = FALSE;

          /* Store coded setup message in cc_data */
          {
            /* No assumtions made about length of this foreign message */
            PALLOC_SDU (data, MMCM_DATA_REQ, M_CC_L3MAX << 3);
            data->sdu.o_buf = CC_ENCODE_OFFSET;
            ccd_codeMsg (CCDENT_CC,
                         UPLINK,
                         (T_MSGBUF *) &data->sdu,
                         (UBYTE    *) setup,
                          NOT_PRESENT_8BIT);
            cc_data->stored_ccbs_setup = data;
          }
          CCD_END;

          /* Send MMCC_DATA_REQ (CC EST. CONFIRMED) */
          CCD_START;
          {
            MCAST (est_cnf, U_CC_EST_CONF);
            if (cc_count_active_connections () EQ 0)
              cc_build_cc_est_confirm (est_cnf, CAUSE_MAKE(DEFBY_CONDAT, 
                                                    ORIGSIDE_MS, 
                                                    MNCC_CC_ORIGINATING_ENTITY,
                                                    NOT_PRESENT_8BIT));
            else
              cc_build_cc_est_confirm (est_cnf, CAUSE_MAKE(DEFBY_STD, 
                                                    ORIGSIDE_MS, 
                                                    MNCC_CC_ORIGINATING_ENTITY,
                                                    MNCC_CAUSE_USER_BUSY));
            for_cc_est_confirm (est_cnf);
          }
          CCD_END;

          /* Start TIMER T335 */
          TIMERSTART (T335, T335_VALUE);
      
          /* Enter the "CC-establishment confirmed" state */
          cc_set_state (M_CC_CS_05);
          break;
      }
      break;

    default: 
      /* Message not compatible with protocol state */
      CCD_END;
      
      /* Implements Measure#  3 and streamline encoding */
      cc_send_status (MNCC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_recall                  |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming recall message.

*/

GLOBAL void cc_recall (T_D_RECALL * recall)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_recall");

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_05: /* CC ESTABLISHMENT CONFIRMED */
      switch(cc_data->error)
      {
        case M_CC_CAUSE_INVALID_MAND_INFO:
        case M_CC_CAUSE_COND_INFO_ELEM:
          CCD_END;
          /* Implements Measure#  7 and streamline encoding*/
          cc_send_status (cc_data->error);  
          return;

        default:
          /* Stop Timer T335 */
          TIMERSTOP (TIMER_CC);
          {
            T_U_SETUP * setup;
            UBYTE result;
            PALLOC (fac_ind, MNCC_FACILITY_IND);

            /* Process facility element. may be not present */
            fac_ind->ti = cc_data->ti;
            fac_ind->fac_context = MNCC_FAC_IN_RECALL;
            fac_ind->fac_inf.l_fac = recall->facility.c_fac << 3;
            fac_ind->fac_inf.o_fac = 0;
            memcpy (fac_ind->fac_inf.fac,  
                    recall->facility.fac,
                    recall->facility.c_fac);

            {
              PALLOC (rec_ind, MNCC_RECALL_IND); /* T_MNCC_RECALL_IND */    

              rec_ind->ti =  cc_data->ti;
              rec_ind->rcl_type = recall->recall_type.rcl_type;
              CCD_END;

              /* 
               * Unpack coded setup container.
               * Conserving memory, so already processed parameters are 
               * processed for a second time now. No errors expected.
               */
              assert (cc_data->stored_ccbs_setup NEQ NULL);
              CCD_START;
              result = ccd_decodeMsg (CCDENT_CC,
                         UPLINK,
                         (T_MSGBUF *) &cc_data->stored_ccbs_setup->sdu,
                         (UBYTE    *) _decodedMsg,
                         NOT_PRESENT_8BIT);
              assert (result EQ ccdOK);
              setup = (T_U_SETUP *)_decodedMsg;
              result = cc_basic_service_align (setup);
              assert (result NEQ ERROR);
            
              /* Process repeat indicator, BC I and BC II */
              rec_ind->ri = cc_data->neg_ri;
              memcpy (&rec_ind->bcpara, &cc_data->neg_bcpara1, 
                      sizeof (T_MNCC_bcpara));
              memcpy (&rec_ind->bcpara2, &cc_data->neg_bcpara2, 
                      sizeof (T_MNCC_bcpara));
            
              /* Process called party address */   
              rec_ind->called_party.ton          = setup->ul_called_num.ton;
              rec_ind->called_party.npi          = setup->ul_called_num.npi;
              rec_ind->called_party.c_called_num = setup->ul_called_num.c_num;
              memcpy (rec_ind->called_party.called_num, 
                      setup->ul_called_num.num, 
                      setup->ul_called_num.c_num);

              /* Process called party subaddress */
              if (setup->v_called_subaddr)
              {
                rec_ind->called_party_sub.tos =  
                  setup->called_subaddr.tos;
                rec_ind->called_party_sub.odd_even = 
                  setup->called_subaddr.odd_even;
                rec_ind->called_party_sub.c_subaddr = 
                  setup->called_subaddr.c_subaddr;
                memcpy (rec_ind->called_party_sub.subaddr,
                        setup->called_subaddr.subaddr, MNCC_SUB_LENGTH);
              }
              else
              {
                rec_ind->called_party_sub.tos = MNCC_TOS_NOT_PRES;
                rec_ind->called_party_sub.c_subaddr = 0;
              }                                             
              CCD_END;
            
              /* Send MNCC_RECALL_IND */
              PSENDX (MMI, rec_ind);
            }

            /* Handle MNCC_FACILITY_IND */
            if (fac_ind->fac_inf.l_fac)
            {
              PSENDX (MMI, fac_ind);
            }
            else
              PFREE (fac_ind);
            
            /* Next state is RECALL PRESENT */
            cc_set_state (M_CC_CS_06);
          }
          break;
      }
      break; /* esac CS_05 */
    default: 
      CCD_END;
      /* Message not compatible with protocol state */
/* Implements Measure#  3 and streamline encoding*/
      cc_send_status (MNCC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_setup                   |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming setup message.

*/

GLOBAL void cc_setup (T_D_SETUP * setup)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_setup()");

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_0:
      if (cc_data->ti < 8)
      {
        /*
         * This values are reserved for mobile originated calls.
         */
        CCD_END;
        for_rel_req ();
        break;
      }

      switch (cc_data->error)
      {
        case M_CC_CAUSE_INVALID_MAND_INFO:
        case M_CC_CAUSE_COND_INFO_ELEM:
          CCD_END;
          CCD_START;
          {
            MCAST (rel_com, U_RELEASE_COMP);

            cc_build_release_complete (rel_com, CAUSE_MAKE(DEFBY_STD, 
                                                    ORIGSIDE_MS, 
                                                    MNCC_CC_ORIGINATING_ENTITY,
                                                    cc_data->error));
            for_release_complete (rel_com);
          }
          CCD_END;
          for_rel_req ();
          break;

        default:
          switch (cc_compatibility_check (setup))
          {
            case BAD_SUBADDRESS:
              CCD_END;
              CCD_START;
              {
                MCAST (rel_com, U_RELEASE_COMP);
                cc_build_release_complete (rel_com, CAUSE_MAKE(DEFBY_STD, 
                                                    ORIGSIDE_MS, 
                                                    MNCC_CC_ORIGINATING_ENTITY,
                                                    MNCC_CAUSE_NO_ROUTE));
                for_release_complete (rel_com);
              }
              CCD_END;
              for_rel_req ();
              cc_set_state (M_CC_CS_0);
              break;

            case ERROR:
              CCD_END;
              CCD_START;
              {
                MCAST (rel_com, U_RELEASE_COMP);
                cc_build_release_complete (rel_com, CAUSE_MAKE(DEFBY_STD, 
                                                    ORIGSIDE_MS, 
                                                    MNCC_CC_ORIGINATING_ENTITY,
                                                    MNCC_CAUSE_INCOMPAT_DEST));
                for_release_complete (rel_com);
              }
              CCD_END;
              for_rel_req ();
              cc_set_state (M_CC_CS_0);
              break;

            case OKAY:
            case NEGOTIATION:
            {
              PALLOC (setup_ind, MNCC_SETUP_IND);

              cc_data->progress_desc[cc_data->index_ti] = NOT_PRESENT_8BIT;

              if (setup->v_progress)
              {
                cc_data->progress_desc[cc_data->index_ti] = 
                  setup->progress.progress_desc;
              }

              cc_data->call_type [cc_data->index_ti] = CALL_TYPE_MTC;
              // PATCH LE 10.04.00
              // initialize disconnect collision flag
              cc_data->disc_coll [cc_data->index_ti] = FALSE;
              // END PATCH LE 10.04.00
              cc_build_mncc_setup_ind (setup, setup_ind);

              PSENDX (MMI, setup_ind);
              cc_build_facility_ind (MNCC_FAC_IN_SETUP, setup->v_facility,
                                     &setup->facility);
              cc_build_user_user_ind (MNCC_USER_IN_SETUP, setup->v_user_user,
                                      &setup->user_user);



              CCD_END;
              CCD_START;
              {
                MCAST (call_cnf, U_CALL_CONF);
  
                if (cc_count_active_connections () EQ 0)
                  cc_build_call_confirm (call_cnf, CAUSE_MAKE(DEFBY_CONDAT, 
                                                    ORIGSIDE_MS, 
                                                    MNCC_CC_ORIGINATING_ENTITY,
                                                    NOT_PRESENT_8BIT));
                else
                  cc_build_call_confirm (call_cnf, CAUSE_MAKE(DEFBY_STD, 
                                                    ORIGSIDE_MS, 
                                                    MNCC_CC_ORIGINATING_ENTITY,
                                                    MNCC_CAUSE_USER_BUSY));
                for_call_confirm (call_cnf);
              }
              CCD_END;
              cc_set_state (M_CC_CS_9);

              EM_CC_MM_CONNECTION_ESTABLISHED_MT;

              break;
            }

            default:
              CCD_END;
              break;
          }
          break;
      }
      break;

    default:
      CCD_END;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_sync_ind                |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming sync indication from
            mobility management.

*/

GLOBAL void cc_sync_ind (T_MMCM_SYNC_IND * mmcm_sync)
{
  GET_INSTANCE_DATA;	
  PALLOC ( mncc_sync, MNCC_SYNC_IND );

  TRACE_FUNCTION ("cc_sync_ind()");


  cc_data->channel_type = mmcm_sync->sync_info.ch_info.ch_type;
  cc_data->channel_mode = mmcm_sync->sync_info.ch_info.ch_mode;

  mncc_sync->ti         = mmcm_sync->ti;

  mncc_sync->ch_info.ch_mode = mmcm_sync->sync_info.ch_info.ch_mode;
  mncc_sync->ch_info.ch_type = mmcm_sync->sync_info.ch_info.ch_type;

  mncc_sync->cause      = MNCC_CAUSE_CHANNEL_SYNC;
  PSENDX (MMI, mncc_sync);
  PFREE (mmcm_sync);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_est_cnf                 |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming establish confirm from
            mobility management.

*/

GLOBAL void cc_est_cnf (void)
{
  GET_INSTANCE_DATA;	
  TRACE_FUNCTION ("cc_est_cnf()");

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_01:
      {
        /* 
         * As CC may potentially do some ugly things like 
         * MAX_SETUP_ATTEMPTS > 1, we cannot use the stored 
         * SETUP or EMERGENCY SETUP message, but have to copy.
         */
        PALLOC_SDU (data, MMCM_DATA_REQ, cc_data->stored_setup->sdu.l_buf);

        data->sdu.l_buf = cc_data->stored_setup->sdu.l_buf;
        data->sdu.o_buf = cc_data->stored_setup->sdu.o_buf;
        memcpy (&data->sdu.buf[data->sdu.o_buf >> 3],
                &cc_data->stored_setup->sdu.buf[data->sdu.o_buf >> 3],
                data->sdu.l_buf >> 3);

        /* Forward the SETUP or EMERGENCY SETUP message to MM */
        for_pd (data);
      }

      cc_set_state (M_CC_CS_1);
      srv_use_stored_prim ();
      break;

    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_reest_cnf               |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming re-establish confirm from
            mobility management.

*/

GLOBAL void cc_reest_cnf (void)
{
  GET_INSTANCE_DATA;
  
  PALLOC (sync, MNCC_SYNC_IND);

    TRACE_FUNCTION ("cc_reest_cnf()");

    switch (cc_data->state[cc_data->index_ti])
    {
      case CS_101:
        sync->ti    = cc_data->ti;
        sync->cause = MNCC_CAUSE_REEST_FINISHED;
        sync->ch_info.ch_mode = NOT_PRESENT_8BIT;
        sync->ch_info.ch_type = NOT_PRESENT_8BIT;        
        PSENDX (MMI, sync);

        cc_set_state (M_CC_CS_10);
        srv_use_stored_prim ();
        break;

      case CS_261:
        sync->ti    = cc_data->ti;
        sync->cause = MNCC_CAUSE_REEST_FINISHED;
        sync->ch_info.ch_mode = NOT_PRESENT_8BIT;
        sync->ch_info.ch_type = NOT_PRESENT_8BIT;
        PSENDX (MMI, sync);

        TIMERSTART (T323, T323_VALUE);
        cc_set_state (M_CC_CS_26); 
        srv_use_stored_prim ();
        break;

      default:
        PFREE (sync);
        break;
    }
}

#ifdef SIM_TOOLKIT
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_mncc_bearer_cap_req     |
+--------------------------------------------------------------------+

  PURPOSE:      Checks the compatibility of bearer capabilities. 
                Bearer capabilities in coded form are tranformed to 
                parameter form and vice versa.      
*/

GLOBAL void cc_mncc_bearer_cap_req (T_MNCC_BEARER_CAP_REQ * bc_req)
{
  PALLOC (bc_conf, MNCC_BEARER_CAP_CNF); 

  TRACE_FUNCTION ("cc_mncc_bearer_cap_req()");

  /* Copy input parameters */
  bc_conf->req_id = bc_req->req_id;
  bc_conf->bc_mod = bc_req->bc_mod;

  /* Set default values to no bcpara to set valid values in case cc_bearer_cap_decode returns OKAY */
  memset (&bc_conf->bcpara2, 0, sizeof (T_MNCC_bcpara));
  bc_conf->bcpara2.bearer_serv = MNCC_BEARER_SERV_NOT_PRES;

  if (bc_req->bc_mod EQ MNCC_BC_MOD_DECODE) 
  {
    /* Transform coded form into parameter form */
    
    /* Copy input parameters */
    bc_conf->bcconf = bc_req->bcconf;
    bc_conf->bcconf2 = bc_req->bcconf2;
    
    if (cc_bearer_cap_decode(&bc_req->bcconf, 
                             &bc_conf->bcpara) 
        EQ ERROR)
    {
      /* Bearer capabilities not supported */
      PFREE (bc_req);
      bc_conf->cause = CAUSE_MAKE(DEFBY_STD, 
                                ORIGSIDE_MS, 
                                MNCC_CC_ORIGINATING_ENTITY, 
                                MNCC_CAUSE_INCOMPAT_DEST);
      PSENDX (MMI, bc_conf);
      return;
    } 

    if (cc_bearer_cap_decode((T_MNCC_bcconf *)&bc_req->bcconf2, 
                             (T_MNCC_bcpara *)&bc_conf->bcpara2)
        EQ ERROR)
    {
      /* Bearer capabilities not supported */
      PFREE (bc_req);
      bc_conf->cause = CAUSE_MAKE(DEFBY_STD, 
                                ORIGSIDE_MS, 
                                MNCC_CC_ORIGINATING_ENTITY, 
                                MNCC_CAUSE_INCOMPAT_DEST);
      PSENDX (MMI, bc_conf);
      return;
    }
  }
  else
  {
    /* Transform parameter form into coded form */

    /* Copy input parameters */
    bc_conf->bcpara  = bc_req->bcpara;
    bc_conf->bcpara2 = bc_req->bcpara2;

    if (cc_bearer_cap_code(&bc_req->bcpara, 
                           &bc_conf->bcconf)
        EQ ERROR)
    {
      /* Bearer capabilities not supported */
      PFREE (bc_req);
      bc_conf->cause = CAUSE_MAKE(DEFBY_STD, 
                                ORIGSIDE_MS, 
                                MNCC_CC_ORIGINATING_ENTITY, 
                                MNCC_CAUSE_INCOMPAT_DEST);
      PSENDX (MMI, bc_conf);
      return;
    }

    if (cc_bearer_cap_code((T_MNCC_bcpara *)&bc_req->bcpara2,
                           (T_MNCC_bcconf *)&bc_conf->bcconf2)
        EQ ERROR)
    {
      /* Bearer capabilities not supported */
      PFREE (bc_req);
      bc_conf->cause = CAUSE_MAKE(DEFBY_STD, 
                                ORIGSIDE_MS, 
                                MNCC_CC_ORIGINATING_ENTITY, 
                                MNCC_CAUSE_INCOMPAT_DEST);
      PSENDX (MMI, bc_conf);
      return;
    }
  }

  bc_conf->cause = MNCC_CAUSE_SUCCESS; /* Positive result */
  PSENDX (MMI, bc_conf);
  PFREE (bc_req);
}

#endif /*SIM_TOOLKIT */


/* Implements Measure#  3 and streamline encoding*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_send_status             |
+--------------------------------------------------------------------+

  PURPOSE : Calls CCD Start and then cc_build_and_send_status 
            for sending status message
*/
GLOBAL void cc_send_status (USHORT cause)
{
  TRACE_FUNCTION ("cc_send_status()");

  CCD_START;
  {
    MCAST (status, B_STATUS);
    cc_build_status (status, CAUSE_MAKE(DEFBY_STD, 
                                      ORIGSIDE_MS, 
                                      MNCC_CC_ORIGINATING_ENTITY,
                                      cause));
    for_status (status);
  }
  CCD_END;
}

/* Implements Measure#  19 */ 
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                     |
| STATE   : code                ROUTINE : cc_send_release_cmp        |
+--------------------------------------------------------------------+

  PURPOSE : Send release complete message.

*/

LOCAL void cc_send_release_cmp (USHORT cause)
{
  GET_INSTANCE_DATA; 
  TRACE_FUNCTION ("cc_send_release_cmp()");
  
  /* Send RELEASE COMPLETE */
  CCD_START;
  {
    MCAST (rel_com, U_RELEASE_COMP);
    cc_build_release_complete (rel_com, cause);
    for_release_complete (rel_com);
  }
  CCD_END;

  /* Inform MMI */
  {
    PALLOC (release, MNCC_RELEASE_IND);
    release->ti    = cc_data->ti;
    release->cause = cause;
    release->c_raw_cause = cause;
    PSENDX (MMI, release);
  }

  /* Release MM connection */
  for_rel_req ();

  /* Next state is NULL */
  cc_set_state (M_CC_CS_0);

}

/* Implements Measure#  36, 48 */ 
/*
+--------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_EST                           |
| STATE   : code                ROUTINE : cc_send_mncc_release_ind         |
+--------------------------------------------------------------------------+

  PURPOSE : Send release complete message with cause invalid ti.

*/

LOCAL void cc_send_mncc_release_ind(UBYTE  ti, USHORT cause)
{
  TRACE_FUNCTION ("cc_send_mncc_release_ind");  
  {
    PALLOC (release, MNCC_RELEASE_IND); 
    release->ti          = ti;
    release->cause       = cause;
    release->c_raw_cause = 0;
    PSENDX (MMI, release);
  }
}
#endif
