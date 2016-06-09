/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CC_REL
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
|  Purpose :  This Modul defines the functions for call release
|             of the component CC of the mobile station.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CC_REL_C
#define CC_REL_C

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

/*==== PROTOTYPES =================================================*/
/* Implements Measure#  8 */
LOCAL void cc_mncc_release_ind  (T_PRIM * prim);
/* Implements Measure#  9 */
LOCAL void cc_mncc_release_cnf (T_PRIM * prim);
/* Implements Measure#  41 */
LOCAL void cc_mncc_sync_ind ( UBYTE ti, 
                              UBYTE new_state);
/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_check_setup_reattempt   |
+--------------------------------------------------------------------+

  PURPOSE : This function checks whether a the mobile originated setup
            may be reattempted. This will be done for some certain
            reasons which may indicate network failure.
            This function is considered as a hack only
            for networks which tend to deliver a poor quality
            of service only (e.g. typically in China). 
            This behaviour or the algorithm is not specified / implied 
            by GSM but driven by customer requests and the desire
            to have a good, working product.
*/

LOCAL void cc_check_setup_reattempt ( const T_M_CC_cc_cause *cc_cause)
{
  GET_INSTANCE_DATA;
  EXTERN BOOL poor_quality_network;

  TRACE_FUNCTION ("cc_check_setup_reattempt()");

#ifdef  WIN32
  /*
   * Set poor_quality_network for test, pei_config() is the appropriate place,
   * but this seems not to be working for some strange reasons
   */
  poor_quality_network = TRUE;
#endif

  /* reattempt already "running", see cc_setup_reattempt for further reasoning */
  if (cc_data->setup_reattempt_ti NEQ NOT_PRESENT_8BIT)
    return;

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_1:  /* call initiated                     */
    case M_CC_CS_3:  /* mobile originating call proceeding */
      cc_data->setup_attempts++;
      if (poor_quality_network AND
          (cc_data->setup_attempts < MAX_SETUP_ATTEMPTS) AND
          (cc_cause->v_cause))
      {
        /*
         * Mobile originated call got disconnect,
         * max. number of setup attempts not reached.
         * New dial attempt for certain disconnect reasons
         */
        switch (cc_cause->cause)
        {
          case M_CC_CAUSE_NO_CHAN_AVAIL:       /* #34  */
          case M_CC_CAUSE_NETWORK_ORDER:       /* #38  */
          case M_CC_CAUSE_TEMP_FAIL:           /* #41  */
          case M_CC_CAUSE_SWITCH_CONGEST:      /* #42  */
          case M_CC_CAUSE_INFO_DISCARD:        /* #43  */
          case M_CC_CAUSE_REQ_CHAN_UNAVAIL:    /* #44  */
          case M_CC_CAUSE_RESOURCE_UNAVAIL:    /* #47  */
          case M_CC_CAUSE_QOS_UNAVAIL:         /* #49  */
          case M_CC_CAUSE_BEARER_CAP_UNAVAIL:  /* #58  */
          case M_CC_CAUSE_TIMER:               /* #102 */
          case M_CC_CAUSE_INTERWORKING:        /* #127 */
            /*
             * Maybe some more causes to added here
             * which indicate network failure
             */

            /* try setup reattempt, remember this by setting setup_reattempt_ti */
            cc_data->setup_reattempt_ti = cc_data->ti;
            break;

          default: /* Don't try setup reattempt, no appropriate cause */
            srv_free_stored_setup ();
            break;
        }
      }
      break;

    default: /* Don't try setup reattempt in other states */
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_setup_reattempt         |
+--------------------------------------------------------------------+

  PURPOSE : This function reattempts the mobile originated setup
            for some certain reasons which may indicate network
            failure. This function is considered as a hack only
            for networks which tend to deliver a poor quality
            of service only.

*/

LOCAL void cc_setup_reattempt (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_setup_reattempt()");

  if (cc_data->ti EQ cc_data->setup_reattempt_ti)
  {
    PALLOC (est, MMCM_ESTABLISH_REQ); /* T_MMCM_ESTABLISH_REQ */
    est->ti    = cc_data->ti;
    est->estcs = cc_data->estcs;
    for_est_req (est);

    cc_data->setup_reattempt_ti = NOT_PRESENT_8BIT;
    cc_data->progress_desc[cc_data->index_ti] = NOT_PRESENT_8BIT;
    cc_set_state (M_CC_CS_01);
    TIMERSTART (T303, T303_VALUE);
  }
}

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_mncc_disconnect_req     |
+--------------------------------------------------------------------+

  PURPOSE : Disconnection of call by the mobile side.

*/

GLOBAL void cc_mncc_disconnect_req (T_MNCC_DISCONNECT_REQ * disc)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_mncc_disconnect_req()");

  if ((cc_data->index_ti = srv_convert_ti (disc->ti))
       EQ NOT_PRESENT_8BIT)
  {
    PFREE (disc);
    return;
  }
  if (cc_data->ti EQ cc_data->setup_reattempt_ti)
  {
    srv_free_stored_setup ();  
  }
  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_03: /* WAIT FOR NW. INFO */
    case M_CC_CS_05: /* CC ESTABLISHMENT CONFIRMED */
    case M_CC_CS_06: /* RECALL PRESENT */
      /* Forget stored CCBS setup */
      if (cc_data->stored_ccbs_setup NEQ NULL)
      {
        PFREE (cc_data->stored_ccbs_setup);
        cc_data->stored_ccbs_setup = NULL;
      }

      /* Send a (faked) MNCC_RELEASE_IND to MMI */
      {
        PALLOC (rel, MNCC_RELEASE_IND);
        rel->ti    = disc->ti;
        rel->cause = disc->cause;
        rel->c_raw_cause = 0;
        PSENDX (MMI, rel);
      }

      /* Send RELEASE COMPLETE to network */
      CCD_START;
      {
        MCAST (rel_com, U_RELEASE_COMP);
        cc_build_release_complete (rel_com, disc->cause);
        for_release_complete (rel_com);
      }
      CCD_END;

      /* Release MM connection */
      for_rel_req ();

      PFREE (disc);

      /* Stop all running timers */
      TIMERSTOP (TIMER_CC);

      /* Next state is NULL */
      cc_set_state (M_CC_CS_0);
      break;

    case M_CC_CS_01:
    case CS_101:
    case CS_261:
      {
        TIMERSTOP (TIMER_CC);

        cc_reset_dtmf ();

        {
          PALLOC (rel, MNCC_RELEASE_IND);
          rel->ti    = disc->ti;
          rel->cause = disc->cause;
          rel->c_raw_cause = 0;
          PSENDX (MMI, rel);
        }

        for_rel_req ();
        cc_set_state (M_CC_CS_0);
        srv_use_stored_prim ();
        PFREE (disc);
      }
      break;

    case M_CC_CS_1:      
    case M_CC_CS_3:
    case M_CC_CS_4:
    case M_CC_CS_7:
    case M_CC_CS_8:
    case M_CC_CS_9:
    case M_CC_CS_10:
    case M_CC_CS_26:
      TIMERSTOP (TIMER_CC);

      CCD_START;
      {
        MCAST (disconnect, U_DISCONNECT);
        
        cc_build_disconnect (disconnect, disc->cause,
                             &disc->fac_inf, disc->ss_version);
        cc_set_state (M_CC_CS_11);
        for_disconnect (disconnect);
      }
      CCD_END;
      TIMERSTART (T305, T305_VALUE);
      PFREE (disc);
      break;

    case M_CC_CS_12: /* DISCONNECT INDICATION, this is clear collision here */
      CCD_START;
      {
        MCAST (release, U_RELEASE);

        cc_build_release (release, disc->cause,
                          NULL, MNCC_SS_VER_NOT_PRES);
        cc_set_state (M_CC_CS_19);
        for_release (release);

      }
      CCD_END;
      TIMERSTART (T308, T308_VALUE);
      PFREE (disc);
      break;

    default:
      PFREE (disc);
      break;

  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_mncc_reject_req         |
+--------------------------------------------------------------------+

  PURPOSE : Release of call by the mobile side by sending
            a RELEASE COMPLETE message.

*/

GLOBAL void cc_mncc_reject_req (T_MNCC_REJECT_REQ * rej)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_reject_req");

  if ((cc_data->index_ti = srv_convert_ti (rej->ti))
      EQ NOT_PRESENT_8BIT)
  {
    PFREE (rej);
    return;
  }

  if (cc_data->ti EQ cc_data->setup_reattempt_ti)
  {
    srv_free_stored_setup ();
  }

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_03: /* WAIT FOR NW. INFO */
    case M_CC_CS_05: /* CC ESTABLISHMENT CONFIRMED */
    case M_CC_CS_06: /* RECALL PRESENT */
      /* Forget stored CCBS setup */
      if (cc_data->stored_ccbs_setup NEQ NULL)
      {
        PFREE (cc_data->stored_ccbs_setup);
        cc_data->stored_ccbs_setup = NULL;
      }

      /* Send RELEASE COMPLETE */
      CCD_START;
      {
        MCAST (rel_com, U_RELEASE_COMP);
        cc_build_release_complete (rel_com, rej->cause);
        for_release_complete (rel_com);
      }
      CCD_END;

      /* Release MM connection */
      for_rel_req ();

      PFREE (rej);

      /* Stop all running timers */
      TIMERSTOP (TIMER_CC);

      /* Next state is NULL */
      cc_set_state (M_CC_CS_0);
      break;
    default:
      PFREE (rej);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_mncc_release_req        |
+--------------------------------------------------------------------+

  PURPOSE : Release of call by the mobile side.

*/
GLOBAL void cc_mncc_release_req (T_MNCC_RELEASE_REQ * rel)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_release_req()");

  if ((cc_data->index_ti = srv_convert_ti (rel->ti))
      EQ NOT_PRESENT_8BIT)
  {
    PFREE (rel);
    return;
  }

  if (cc_data->ti EQ cc_data->setup_reattempt_ti)
  {
    srv_free_stored_setup ();
  }

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_12: /* DISCONNECT INDICATION */

      CCD_START;
      {
        MCAST (release, U_RELEASE);

        cc_build_release (release, rel->cause, 
                          &rel->fac_inf, rel->ss_version);
        cc_set_state (M_CC_CS_19);
        for_release (release);

      }
      CCD_END;
      TIMERSTART (T308, T308_VALUE);
      PFREE (rel);
      break;

    default:
      PFREE (rel);
      break;

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_disconnect              |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming disconnect message.

*/

GLOBAL void cc_disconnect (T_D_DISCONNECT * disconnect)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_disconnect()");

  /* The cause is in DISCONNECT a mandatory IE (given intact message) */
  TRACE_EVENT_P1 ("DISCONNECT cause: %02x", disconnect->cc_cause.cause);

  EM_CC_DISCONNECT_RECEIVED;

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_11: /* DISCONNECT REQUEST, this is clear collision here */
      CCD_END;
      CCD_START;
      {
        MCAST (release, U_RELEASE);

        TIMERSTOP (TIMER_CC);
        /* PATCH LE 10.04.00
         * set disconnect collision flag
         */ 
        cc_data->disc_coll [cc_data->index_ti]    = TRUE;
        /* END PATCH LE 10.04.00 */

        switch (cc_data->error)
        {
          case M_CC_CAUSE_INVALID_MAND_INFO:
          case M_CC_CAUSE_COND_INFO_ELEM:
            cc_build_release (release, CAUSE_MAKE(DEFBY_STD, 
                                                  ORIGSIDE_MS, 
                                                  MNCC_CC_ORIGINATING_ENTITY,
                                                  cc_data->error),
                              NULL, MNCC_SS_VER_NOT_PRES);
            cc_set_state (M_CC_CS_19);
            for_release (release);
            TIMERSTART (T308, T308_VALUE);
            break;

          default:
            if (disconnect->v_progress) 
            {
              cc_data->progress_desc[cc_data->index_ti] = 
                disconnect->progress.progress_desc;
            }
            cc_build_release (release, CAUSE_MAKE(DEFBY_CONDAT, 
                                                  ORIGSIDE_MS, 
                                                  MNCC_CC_ORIGINATING_ENTITY,
                                                  NOT_PRESENT_8BIT),
                              NULL, MNCC_SS_VER_NOT_PRES);
            cc_set_state (M_CC_CS_19);
            for_release (release);
            TIMERSTART (T308, T308_VALUE);
            break;
        }
      }
      CCD_END;
      break;

    case M_CC_CS_12: /* DISCONNECT INDICATION */
    case M_CC_CS_19: /* RELEASE REQUEST */
      /* NULL (M_CC_CS_0) is handled by the formatter */
      CCD_END;
/* Implements Measure#  3 and streamline encoding */
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;

    default:
      /* any "normal" state */
      TIMERSTOP (TIMER_CC);

      switch (cc_data->error)
      {
        case M_CC_CAUSE_INVALID_MAND_INFO:
        case M_CC_CAUSE_COND_INFO_ELEM:
          CCD_END;
          CCD_START;
          {
            USHORT curr_cause; /* local variable to avoid repeated CAUSE_MAKEs for the same cause */

            MCAST (release, U_RELEASE);
            PALLOC (disc_ind, MNCC_DISCONNECT_IND);

            disc_ind->ti    = cc_data->ti;
            curr_cause = CAUSE_MAKE(DEFBY_STD, 
                                    ORIGSIDE_MS, 
                                    MNCC_CC_ORIGINATING_ENTITY,
                                    cc_data->error);
            disc_ind->cause = curr_cause; 
            /* Setting raw_cause to empty as this is a local release
             * of MM connection,CC is not receiving any cause value
             * from Network
             */
            disc_ind->c_raw_cause = 0;

            disc_ind->diagnostic = NOT_PRESENT_8BIT;
            disc_ind->progress_desc = MNCC_PROG_NOT_PRES;
            /* CQ 23619: get the ss diagnostic >> */
            disc_ind->ss_diag = MNCC_SS_DIAG_NOT_PROVIDED;
            /* CQ 23619 << */
            PSENDX (MMI, disc_ind);
            
            /* Patch HM 29-Jan-02 >>> */
            cc_build_facility_ind (MNCC_FAC_IN_DISCONNECT,
                                   disconnect->v_facility, &disconnect->facility);
            /* Patch HM 29-Jan-02 <<< */
            cc_build_release (release, cc_data->error,
                              NULL, MNCC_SS_VER_NOT_PRES);
            cc_set_state (M_CC_CS_19);
            for_release (release);
            TIMERSTART (T308, T308_VALUE);
          }
          CCD_END;
        break;

        default:
          if (disconnect->v_progress) 
          {
            cc_data->progress_desc[cc_data->index_ti] = 
              disconnect->progress.progress_desc;
          }
          cc_check_setup_reattempt (&disconnect->cc_cause);

          if (cc_data->ti NEQ cc_data->setup_reattempt_ti)
          {
            /*
             * We don't perform setup reattempt, so send disconnect to MMI
             */
            USHORT curr_cause; /* local variable to avoid repeated CAUSE_MAKEs for the same cause */             
            PALLOC (disc_ind, MNCC_DISCONNECT_IND); /* T_MNCC_DISCONNECT_IND */
            disc_ind->ti    = cc_data->ti;
            disc_ind->cause = curr_cause = CAUSE_MAKE(DEFBY_STD, 
                                                      ORIGSIDE_NET, 
                                                      MNCC_CC_ORIGINATING_ENTITY,
                                                      disconnect->cc_cause.cause);
            disc_ind->c_raw_cause = cc_build_cause (&disconnect->cc_cause,
                                                     disc_ind->raw_cause);
            disc_ind->diagnostic = NOT_PRESENT_8BIT;

            if (disconnect->v_progress)
              disc_ind->progress_desc = disconnect->progress.progress_desc;
            else
              disc_ind->progress_desc = MNCC_PROG_NOT_PRES;

            /* Handle CCBS possible flag */
            if ((cc_data->call_ctrl_cap.pcp NEQ 0) AND
                (disconnect->v_allowed_actions NEQ 0) AND
                (disconnect->allowed_actions.ccbs_act EQ M_CC_CCBS_YES))
            {
              /* CCBS indicated as possible by the network */
              disc_ind->diagnostic = MNCC_DIAG_CCBS_POSSIBLE;
            }
            /* CQ 23619: get the ss diagnostic */
            disc_ind->ss_diag = cc_get_ss_diag(curr_cause, disconnect);
            /* CQ 23619 << */
            PSENDX (MMI, disc_ind);
            /* Patch HM 29-Jan-02 >>> */
            cc_build_facility_ind (MNCC_FAC_IN_DISCONNECT,
                                   disconnect->v_facility, &disconnect->facility);            
            /* Patch HM 29-Jan-02 <<< */
          }
          if ((cc_data->ti NEQ cc_data->setup_reattempt_ti) AND
              (((disconnect->v_progress AND
                (disconnect->progress.progress_desc EQ M_CC_PROG_INBAND_AVAIL) AND
                (cc_data->channel_mode != NAS_CHM_SIG_ONLY)))
              OR
              ((cc_data->call_ctrl_cap.pcp NEQ 0) AND
               (disconnect->v_allowed_actions NEQ 0) AND
               (disconnect->allowed_actions.ccbs_act EQ M_CC_CCBS_YES))))
          {
            /*
             * Hook on tone is generated by the
             * infrastructure and signalled via TCH OR CCBS is possible
             */
            CCD_END;
            cc_set_state (M_CC_CS_12);
          }
          else
          {
            CCD_END;
            CCD_START;
            {
              MCAST (release, U_RELEASE);

              /*
               * Hook on tone is generated internally by the mobile station
               * and no CCBS possible, the connection is released
               */
              cc_build_release (release, CAUSE_MAKE(DEFBY_CONDAT, 
                                                    ORIGSIDE_MS, 
                                                    MNCC_CC_ORIGINATING_ENTITY,
                                                    NOT_PRESENT_8BIT),
                                NULL, MNCC_SS_VER_NOT_PRES);
              cc_set_state (M_CC_CS_19);
              for_release (release);
              TIMERSTART (T308, T308_VALUE);
            }
            CCD_END;
          }
          break;
      }
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_release                 |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming release message.

*/

GLOBAL void cc_release (T_D_RELEASE * release)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_release()");
 
  if (release->cc_cause.v_cause)
  {
    /* The cause is in RELEASE an optional IE */
    TRACE_EVENT_P1 ("RELEASE cause: %02x", release->cc_cause.cause);
  }
  else
  {
    TRACE_EVENT ("RELEASE cause: --");
  }

  switch (cc_data->state[cc_data->index_ti])
  {
    /*
     * M_CC_CS_0 is handled by the formatter
     */
    case M_CC_CS_19: /* Release Request */
    {
      cc_reset_dtmf ();
      /* PATCH LE 10.04.00
       * in disconnect collision case send
       * MNCC_RELEASE_IND instead of MNCC_RELEASE_CNF
....   */
      TIMERSTOP (TIMER_CC);
      if (cc_data->disc_coll [cc_data->index_ti])
      {
/* Implements Measure#  8 */
        cc_mncc_release_ind ((T_PRIM *)D2P(release));
      }
      else
      {
/* Implements Measure#  9 */
        cc_mncc_release_cnf ((T_PRIM *)D2P(release));
      }
      // END PATCH LE 10.04.00     

      for_rel_req ();
      cc_set_state (M_CC_CS_0);
      /* END PATCH LE 10.04.00 */
      CCD_END;
      break;
    }

    case M_CC_CS_1:  /* call initiated                     */
    case M_CC_CS_3:  /* mobile originating call proceeding */ 
      cc_check_setup_reattempt (&release->cc_cause);
      /*FALLTHROUGH*/ /*lint -fallthrough*/
    default:
      cc_reset_dtmf (); /* China change HM 11.07.00 */
      CCD_END;
      CCD_START;
      {
        MCAST (rel_com, U_RELEASE_COMP);
        
        TIMERSTOP (TIMER_CC);
        switch (cc_data->error)
        {
          case M_CC_CAUSE_INVALID_MAND_INFO:
          case M_CC_CAUSE_COND_INFO_ELEM:
            {
              /* local variable to avoid repeated CAUSE_MAKEs
		 for the same cause */
              USHORT curr_cause;
              curr_cause = CAUSE_MAKE(DEFBY_STD, 
                                      ORIGSIDE_MS, 
                                      MNCC_CC_ORIGINATING_ENTITY,
                                      cc_data->error);

              if (cc_data->ti NEQ cc_data->setup_reattempt_ti)
              {
                PALLOC (rel_ind, MNCC_RELEASE_IND);
                rel_ind->ti = cc_data->ti;
                rel_ind->cause = curr_cause;
                rel_ind->c_raw_cause = 0;
                PSENDX (MMI, rel_ind);
              }
              cc_build_release_complete (rel_com, curr_cause);
            }
            break;

          default:
            if (cc_data->ti NEQ cc_data->setup_reattempt_ti)
            {
/* Implements Measure#  8 */
              cc_mncc_release_ind ((T_PRIM *)D2P(release));
            }       
            cc_build_release_complete (rel_com, CAUSE_MAKE(DEFBY_CONDAT, 
                                                    ORIGSIDE_MS, 
                                                    MNCC_CC_ORIGINATING_ENTITY,
                                                    NOT_PRESENT_8BIT));
            break;
        }

        for_release_complete (rel_com);
      }
      CCD_END;
      for_rel_req ();
      cc_set_state (M_CC_CS_0);
      cc_setup_reattempt ();
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_release_complete        |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming release complete message.

*/

GLOBAL void cc_release_complete (T_D_RELEASE_COMP * rel_com)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_release_complete()");

  if (rel_com->cc_cause.v_cause)
  {
    /* The cause is in RELEASE COMPLETE an optional IE */
    TRACE_EVENT_P1 ("RELEASE COMPLETE cause: %02x", rel_com->cc_cause.cause);
  }
  else
  {
    TRACE_EVENT ("RELEASE COMPLETE cause: --");
  }

  EM_CC_RELEASE_COMPLETE_RECEIVED;

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_0: /* NULL */
      CCD_END;
      for_rel_req ();
      break;

    case M_CC_CS_03: /* WAIT FOR NETWORK INFO*/
      /* Stop all running CC timers */
      TIMERSTOP (TIMER_CC);

      /* Inform MMI */
      {
        PALLOC (release, MNCC_RELEASE_IND);
        release->ti  = cc_data->ti;
        if (rel_com->v_cc_cause AND rel_com->cc_cause.v_cause)
        {
          release->cause = CAUSE_MAKE(DEFBY_STD, 
                                      ORIGSIDE_NET, 
                                      MNCC_CC_ORIGINATING_ENTITY,
                                      rel_com->cc_cause.cause);
          release->c_raw_cause = cc_build_cause (&rel_com->cc_cause,
                                                  release->raw_cause);
        }
        else
        {
          release->cause = CAUSE_MAKE(DEFBY_CONDAT, 
                                      ORIGSIDE_NET, 
                                      MNCC_CC_ORIGINATING_ENTITY,
                                      NOT_PRESENT_8BIT);
          release->c_raw_cause = 0;
        }
        PSENDX (MMI, release);
      }

      CCD_END;

      /* Release MM connection */
      for_rel_req ();

      /* Next state is NULL */
      cc_set_state (M_CC_CS_0);
      break;

    case M_CC_CS_05: /* CC ESTABLISHMENT CONFIRMED */
    case M_CC_CS_06: /* RECALL PRESENT */
      /* Forget stored CCBS setup */
      PFREE (cc_data->stored_ccbs_setup);
      cc_data->stored_ccbs_setup = NULL;

      /* Stop all running CC timers */
      TIMERSTOP (TIMER_CC);

      /* Inform MMI */
      {
        PALLOC (release, MNCC_RELEASE_IND);
        release->ti  = cc_data->ti;
        if (rel_com->v_cc_cause AND rel_com->cc_cause.v_cause)
        {
          release->cause = CAUSE_MAKE(DEFBY_STD, 
                                      ORIGSIDE_NET, 
                                      MNCC_CC_ORIGINATING_ENTITY,
                                      rel_com->cc_cause.cause);
          release->c_raw_cause = cc_build_cause (&rel_com->cc_cause,
                                                  release->raw_cause);
        }
        else
        {
          release->cause = CAUSE_MAKE(DEFBY_CONDAT, 
                                      ORIGSIDE_NET, 
                                      MNCC_CC_ORIGINATING_ENTITY,
                                      NOT_PRESENT_8BIT);
          release->c_raw_cause = 0;
        }
        PSENDX (MMI, release);
      }

      CCD_END;

      /* Release MM connection */
      for_rel_req ();

      /* Next state is NULL */
      cc_set_state (M_CC_CS_0);
      break;

    case M_CC_CS_19: /* RELEASE REQUEST */
      cc_reset_dtmf ();
      /* PATCH LE 10.04.00
       * in disconnect collision case send
       * MNCC_RELEASE_IND instead of MNCC_RELEASE_CNF
       */
      TIMERSTOP (TIMER_CC);

      if (cc_data->ti NEQ cc_data->setup_reattempt_ti) 
      {
        /* Inform MMI only if no setup reattempt for this ti */
        if (cc_data->disc_coll [cc_data->index_ti])
        {
/* Implements Measure#  9 */
          cc_mncc_release_ind ((T_PRIM *)D2P(rel_com));
        }
        else
        {
/* Implements Measure#  9 */
          cc_mncc_release_cnf ((T_PRIM *)D2P(rel_com));
        }
        /* END PATCH LE 10.04.00 */

      }

      for_rel_req ();
      cc_set_state (M_CC_CS_0);
      CCD_END;
      cc_setup_reattempt ();
      break;

    case M_CC_CS_1:  /* call initiated                     */
    case M_CC_CS_3:  /* mobile originating call proceeding */
      cc_check_setup_reattempt (&rel_com->cc_cause);
      /*FALLTHROUGH*/ /*lint -fallthrough*/
    default:
      cc_reset_dtmf (); /* China change HM 11.07.00 */
      if (cc_data->ti NEQ cc_data->setup_reattempt_ti)
      {
        /* Implements Measure#  10 */
        cc_mncc_release_ind ((T_PRIM *)D2P(rel_com));
      }
      for_rel_req ();
      CCD_END;
      cc_set_state (M_CC_CS_0);
      cc_setup_reattempt ();
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_rel_ind                 |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming release indication from
            mobility management.

*/

GLOBAL void cc_rel_ind (USHORT cause)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_rel_ind()");

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_0:
      break;

    case M_CC_CS_05: /* CC ESTABLISHMENT CONFIRMED */
    case M_CC_CS_06: /* RECALL PRESENT */
      /* Stop all timers */
      TIMERSTOP (TIMER_CC);

      /* Forget stored CCBS setup message */
      PFREE (cc_data->stored_ccbs_setup);
      cc_data->stored_ccbs_setup = NULL;

      /* Inform MMI */
      {
        PALLOC (release, MNCC_RELEASE_IND);
        release->ti    = cc_data->ti;
        release->cause = cause; /* provide cause from lower layers transparently to higher layers */
        release->c_raw_cause = 0;
        PSENDX (MMI, release);
      }

      cc_set_state (M_CC_CS_0);
      srv_use_stored_prim ();
      break;

    case CS_101: /* Re-establishment request in state M_CC_CS_10 */\
    case CS_261: /* Re-establishment request in state CS_26 */\
      EM_CC_REESTABLISHED_FAILED;
      /*FALLTHROUGH*/ /*lint -fallthrough*/

    default:
      cc_reset_dtmf (); /* China change HM 11.07.00 */
      if (cc_data->ti NEQ cc_data->setup_reattempt_ti)
      {
        PALLOC (rej_ind, MNCC_REJECT_IND);
        rej_ind->ti  = cc_data->ti;
        rej_ind->cause = cause;
        PSENDX (MMI, rej_ind);
      }

      cc_set_state (M_CC_CS_0);
      if (cc_data->ti EQ cc_data->setup_reattempt_ti)
      {
        cc_setup_reattempt();
      }
      else
      {
        srv_use_stored_prim ();
      }
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_err_ind                 |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming error indication from
            mobility management.

*/

GLOBAL void cc_err_ind (T_MMCM_ERROR_IND * err_ind)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_err_ind()");

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_0:
      break;

    case M_CC_CS_10:
      {
/* Implements Measure#  41 */
        cc_mncc_sync_ind(err_ind->ti, CS_101);

        EM_CC_REESTABLISHED_STARTED;

      }
      break;

    case M_CC_CS_26:
      {
/* Implements Measure#  41 */
        cc_mncc_sync_ind(err_ind->ti, CS_261);

        EM_CC_REESTABLISHED_STARTED;

      }
      break;

    default:
      cc_reset_dtmf ();
      if (cc_data->ti NEQ cc_data->setup_reattempt_ti)
      {
        PALLOC (rel_ind, MNCC_RELEASE_IND);
        rel_ind->cause = err_ind->cause;
        rel_ind->ti    = cc_data->ti;
        rel_ind->c_raw_cause = 0;
        PSENDX (MMI, rel_ind);
      }
      cc_set_state (M_CC_CS_0);
      for_rel_req ();
      cc_setup_reattempt ();
      break;
  }
  PFREE (err_ind);
}
/* Implements Measure#  8 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_mncc_release_ind        |
+--------------------------------------------------------------------+

  PURPOSE : send mncc_release_ind

*/

LOCAL void cc_mncc_release_ind (T_PRIM * prim)
{
  GET_INSTANCE_DATA;
  UBYTE                     v_cc_cause;
  T_M_CC_cc_cause                *cc_cause;                             
  
  TRACE_FUNCTION ("cc_mncc_release_ind()");
  
  if (prim->custom.opc EQ D_RELEASE)
  {
    v_cc_cause = ((T_D_RELEASE *)P2D(prim))->v_cc_cause;
    cc_cause   = &((T_D_RELEASE *)P2D(prim))->cc_cause;
    cc_build_facility_ind (MNCC_FAC_IN_RELEASE, 
                           ((T_D_RELEASE *)P2D(prim))->v_facility,
                           &((T_D_RELEASE *)P2D(prim))->facility);
  }
  else
  {
    v_cc_cause = ((T_D_RELEASE_COMP *)P2D(prim))->v_cc_cause;
    cc_cause   = &((T_D_RELEASE_COMP *)P2D(prim))->cc_cause;
    cc_build_facility_ind (MNCC_FAC_IN_RELEASE_COMP,
                           ((T_D_RELEASE_COMP *)P2D(prim))->v_facility, 
                           &((T_D_RELEASE_COMP *)P2D(prim))->facility);
  }

  /* Patch HM 29-Jan-02 <<< */
  {
    PALLOC (rel_ind, MNCC_RELEASE_IND);
    rel_ind->ti    = cc_data->ti;
    if (v_cc_cause)
    {
      rel_ind->cause = CAUSE_MAKE(DEFBY_STD, 
                                   ORIGSIDE_NET, 
                                   MNCC_CC_ORIGINATING_ENTITY,
                                   cc_cause->cause);
      rel_ind->c_raw_cause = cc_build_cause (cc_cause,
                                             rel_ind->raw_cause);
    }
    else
    {
      rel_ind->cause = CAUSE_MAKE(DEFBY_CONDAT, 
                                   ORIGSIDE_NET, 
                                   MNCC_CC_ORIGINATING_ENTITY,
                                   NOT_PRESENT_8BIT);
      /* Setting raw_cause to empty as this is a local release
       * of MM connection,CC is not receiving any cause value
       * from Network
       */
       rel_ind->c_raw_cause = 0;
    }
    PSENDX (MMI, rel_ind);
  }
}

/* Implements Measure#  9 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_mncc_release_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : send mncc_release_cnf

*/

LOCAL void cc_mncc_release_cnf (T_PRIM * prim)
{
  GET_INSTANCE_DATA;
  UBYTE                     v_cc_cause;
  T_M_CC_cc_cause                *cc_cause;                             

  TRACE_FUNCTION ("cc_mncc_release_cnf()");
  
  if (prim->custom.opc EQ D_RELEASE)
  {
    v_cc_cause = ((T_D_RELEASE *)P2D(prim))->v_cc_cause;
    cc_cause   = &((T_D_RELEASE *)P2D(prim))->cc_cause;
    cc_build_facility_ind (MNCC_FAC_IN_RELEASE_COMP, /* A little bit dirty */
                           ((T_D_RELEASE *)P2D(prim))->v_facility,
                           &((T_D_RELEASE *)P2D(prim))->facility);
  }
  else
  {
    v_cc_cause = ((T_D_RELEASE_COMP *)P2D(prim))->v_cc_cause;
    cc_cause   = &((T_D_RELEASE_COMP *)P2D(prim))->cc_cause;
    cc_build_facility_ind (MNCC_FAC_IN_RELEASE_COMP,
                           ((T_D_RELEASE_COMP *)P2D(prim))->v_facility, 
                           &((T_D_RELEASE_COMP *)P2D(prim))->facility);
  }

  /* Patch HM 29-Jan-02  <<< */
  {
    PALLOC (rel_cnf, MNCC_RELEASE_CNF);
    rel_cnf->ti    = cc_data->ti;
    if (v_cc_cause)
    {
      rel_cnf->cause = CAUSE_MAKE(DEFBY_STD, 
                                  ORIGSIDE_NET, 
                                  MNCC_CC_ORIGINATING_ENTITY,
                                  cc_cause->cause);
      rel_cnf->c_raw_cause = cc_build_cause (cc_cause,
                                             rel_cnf->raw_cause);
    }
    else
    {
      rel_cnf->cause = CAUSE_MAKE(DEFBY_CONDAT, 
                                  ORIGSIDE_NET, 
                                  MNCC_CC_ORIGINATING_ENTITY,
                                  NOT_PRESENT_8BIT);
      /* Setting raw_cause to empty as this is a local release
       * of MM connection,CC is not receiving any cause value
       * from Network
       */
       rel_cnf->c_raw_cause = 0;
    }
    PSENDX (MMI, rel_cnf);
  }
}


/* Implements Measure#  41 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : cc_mncc_sync_ind           |
+--------------------------------------------------------------------+

  PURPOSE : send cc_mncc_sync_ind

*/

LOCAL void cc_mncc_sync_ind (UBYTE ti, UBYTE new_state)
{
  TRACE_FUNCTION ("cc_mncc_sync_ind()");
  {
    PALLOC (sync_ind, MNCC_SYNC_IND); /* T_MNCC_SYNC_IND */

    sync_ind->ti    = ti;
    sync_ind->cause = MNCC_CAUSE_REEST_STARTED;
    sync_ind->ch_info.ch_mode=NOT_PRESENT_8BIT;
    sync_ind->ch_info.ch_type=NOT_PRESENT_8BIT;
    
    PSENDX (MMI, sync_ind);
    
    for_reest_req ();
    cc_set_state (new_state);
  }

}
#endif
