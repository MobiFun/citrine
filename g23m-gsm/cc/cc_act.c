/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CC_ACT
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
|  Purpose :  This Modul defines the primitive processing functions
|             of the SDL process Call Control for the call active
|             phase of the component CC of the mobile station
+----------------------------------------------------------------------------- 
*/ 

#ifndef CC_ACT_C
#define CC_ACT_C

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
LOCAL void cc_mncc_hold_retrieve_req (T_PRIM * prim);

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_dtmf_full               |
+--------------------------------------------------------------------+

  PURPOSE : Check whether DTMF buffer is full

*/

LOCAL BOOL cc_dtmf_full (const T_DTMF *p_dtmf)
{
  TRACE_FUNCTION ("cc_dtmf_full()");
  
  return ((p_dtmf->read EQ p_dtmf->write + 1) OR
          (p_dtmf->read EQ 0 AND p_dtmf->write EQ DTMF_BUF_SIZE - 1));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_dtmf_empty              |
+--------------------------------------------------------------------+

  PURPOSE : Check whether DTMF buffer is empty

*/

LOCAL BOOL cc_dtmf_empty (const T_DTMF *p_dtmf)
{
  TRACE_FUNCTION ("cc_dtmf_empty()");
 
  return (p_dtmf->read EQ p_dtmf->write);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_dtmf_write              |
+--------------------------------------------------------------------+

  PURPOSE : Write next entry into DTMF buffer. 
            No checks are done (e.g. buffer full)

*/

LOCAL void cc_dtmf_write (T_DTMF *p_dtmf, UBYTE val)
{
  TRACE_FUNCTION ("cc_dtmf_write()");
  
  p_dtmf->buf[p_dtmf->write++] = val;
  if (p_dtmf->write EQ DTMF_BUF_SIZE) 
    p_dtmf->write = 0; 
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_dtmf_write              |
+--------------------------------------------------------------------+

  PURPOSE : Read next entry from DTMF buffer. 
            No checks are done (e.g. buffer empty)

*/

LOCAL UBYTE cc_dtmf_read (T_DTMF *p_dtmf)
{
  UBYTE val;
  
  TRACE_FUNCTION ("cc_dtmf_read()");

  val = p_dtmf->buf[p_dtmf->read++];
  if (p_dtmf->read EQ DTMF_BUF_SIZE)
    p_dtmf->read = 0;
  return val;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_dtmf_allowed            |
+--------------------------------------------------------------------+

  PURPOSE : This functions returns TRUE if DTMF is allowed in the
            current state, otherwise FALSE.

*/

LOCAL BOOL cc_dtmf_allowed (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_dtmf_allowed()");

  /* 
   * "The mobile station shall be capable of transmitting DTMF messages if
   * and only if the mobile station has the user connection for speech 
   * attached and an appropriate channel is available.
   * 
   * ".."
   * 
   * NOTE 1: This specification means that DTMF messages can generally be 
   *         sent in the active state of a call in speech transmission 
   *         mode or when a traffic channel is available during setup or 
   *         release and the progress indicator IE has been received."
   *
   * [TS 24.008 subclause 5.5.7]
   * 
   * As of GSM 04.08 the progress indicator IE can be received by 
   * CALL PROCEEDING, PROGRESS, CONNECT, SETUP, ALERTING and DISCONNECT.
   */ 
  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_3:   /* mobile originating call proceeding */
    case M_CC_CS_4:   /* call delivered */
    case M_CC_CS_8:   /* connect request */
    case M_CC_CS_11:  /* disconnect request */ 
    case M_CC_CS_12:  /* disconnect indication */
    case M_CC_CS_19:  /* release request */ 
    case CS_101: /* substate of CS_10, reestablish requested */     
    case M_CC_CS_26:  /* mobile originating modify */
    case CS_261: /* substate of CS_26, reestablish requested */
      if (cc_data->progress_desc[cc_data->index_ti] EQ NOT_PRESENT_8BIT)
        return FALSE; /* Progress indicator IE has not been received */
      /*FALLTHROUGH*/ /*lint -fallthrough*/
    case M_CC_CS_10: /* active */
      if (cc_data->channel_type EQ NAS_CH_SDCCH)
        return FALSE; /* No TCH assigned */
  
      if ((cc_data->channel_mode NEQ NAS_CHM_SPEECH) AND
          (cc_data->channel_mode NEQ NAS_CHM_SPEECH_V2) AND
          (cc_data->channel_mode NEQ NAS_CHM_SPEECH_V3))
        return FALSE; /* Channel mode is not speech */

      return TRUE;
    
    default:
      return FALSE;
  }
}

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mncc_start_dtmf_req     |
+--------------------------------------------------------------------+

  PURPOSE : Sending of DTMF tones to the infrastructure side.

*/

GLOBAL void cc_mncc_start_dtmf_req (T_MNCC_START_DTMF_REQ * dtmf)
{
  GET_INSTANCE_DATA;
  T_DTMF         * p_dtmf;

  TRACE_FUNCTION ("cc_mncc_start_dtmf_req()");


  if ((cc_data->index_ti = srv_convert_ti (dtmf->ti))
       EQ NOT_PRESENT_8BIT)
  {
    PFREE (dtmf);
    return;
  }
  
  switch (cc_data->state[cc_data->index_ti])
  {
    default:
      if (!cc_dtmf_allowed ())
      {
        /* Tone dropped. Confirm DTMF request with negative cause */
        PALLOC (dtmf_cnf, MNCC_START_DTMF_CNF);
        dtmf_cnf->ti       = dtmf->ti;
        dtmf_cnf->key      = dtmf->key;
        dtmf_cnf->cause    = MNCC_CAUSE_DTMF_NOT_ALLOWED;
        dtmf_cnf->dtmf_mod = dtmf->dtmf_mod;
        PSENDX (MMI, dtmf_cnf);

        PFREE (dtmf); 
        return;
      }

      p_dtmf = &cc_data->dtmf [cc_data->index_ti];

      if ( dtmf->dtmf_mod EQ MNCC_DTMF_MOD_AUTO OR 
           dtmf->dtmf_mod EQ MNCC_DTMF_MOD_MAN_START )
      {
        if (p_dtmf->state EQ DTMF_IDLE)
        {
          /*
           * If sending of DTMF tones is not in
           * progress send the tone.
           */
          CCD_START;
          {
            MCAST (start_dtmf, U_START_DTMF);

            cc_build_start_dtmf (dtmf->key, start_dtmf);
            for_start_dtmf (start_dtmf);
          }
          CCD_END;
          
          /* On sending a START DTMF message the MS shall start T336 */
          TIMERSTART (T336, T336_VALUE);

          p_dtmf->state = DTMF_SEND_REQUEST;
          p_dtmf->mode  = (dtmf->dtmf_mod EQ MNCC_DTMF_MOD_AUTO)?DTMF_AUTO:DTMF_MAN;
          p_dtmf->key   = dtmf->key;
        }
        else
        {
          /*
           * Not DTMF_IDLE.
           */

          if (dtmf->dtmf_mod EQ MNCC_DTMF_MOD_AUTO)
          {
            /* 
             * If DTMF_MOD_AUTO, store the tone if the buffer is not full.
             */ 
            if (!cc_dtmf_full (p_dtmf))
              cc_dtmf_write (p_dtmf, (UBYTE)SET_DTMF_MOD(dtmf->key, DTMF_AUTO));
            // PATCH HM 13.04.00 >>>
            else
            {
              /* Tone dropped. Confirm DTMF request with negative cause */
              PALLOC (dtmf_cnf, MNCC_START_DTMF_CNF);
              dtmf_cnf->ti       = dtmf->ti;
              dtmf_cnf->key      = dtmf->key;
              dtmf_cnf->cause    = MNCC_CAUSE_DTMF_BUFFER_FULL;
              dtmf_cnf->dtmf_mod = dtmf->dtmf_mod;
              PSENDX(MMI, dtmf_cnf);
            }
            // PATCH HM 13.04.00 <<<
          }
          else
          {
            /* 
             * If DTMF_MOD_MAN_START, two conditions have to be fullfilled 
             * that the tone is stored and not dropped:
             * 
             * - The buffer is not full.
             * - There is enough room to store the stop which 
             *   will follow the tone.
             */
            if (!cc_dtmf_full (p_dtmf) AND
                 (p_dtmf->read NEQ (p_dtmf->write + 2) % DTMF_BUF_SIZE))
            {
              cc_dtmf_write (p_dtmf, (UBYTE)SET_DTMF_MOD(dtmf->key, DTMF_MAN));
            }
            // PATCH HM 13.04.00 >>>
            else
            {
              /* Tone dropped. Confirm DTMF request with negative cause */
              PALLOC (dtmf_cnf, MNCC_START_DTMF_CNF);
              dtmf_cnf->ti       = dtmf->ti;
              dtmf_cnf->key      = dtmf->key;
              dtmf_cnf->cause    = MNCC_CAUSE_DTMF_BUFFER_FULL;
              dtmf_cnf->dtmf_mod = dtmf->dtmf_mod;
              PSENDX (MMI, dtmf_cnf);
            }
            // PATCH HM 13.04.00 <<<
          }
        }
      }
      else
      {
        /* 
         * This is DTMF_MOD_MAN_STOP 
         */
        if ( p_dtmf->state EQ DTMF_SEND_ACKNOWLEDGE AND 
             cc_dtmf_empty (p_dtmf))
        {
          /*
           * If sending of DTMF tones is in
           * progress stop the tone.
           */
          for_stop_dtmf ();

          /* On sending a STOP DTMF message the MS shall start T337 */
          TIMERSTART (T337, T337_VALUE);

          p_dtmf->state = DTMF_STOP_REQUEST;
        }
        else
        {
          /* 
           * If DTMF_MOD_MAN_STOP, the stop mark is stored if 
           * the buffer is not full. If DTMF STOP ACK is received
           * and there are unexpected stops in the buffer due to 
           * a loss of the respective tones, the stops are dropped
           * until a tone is found.
           */
          if (!cc_dtmf_full(p_dtmf))
          {
            cc_dtmf_write (p_dtmf, NOT_PRESENT_8BIT);
          }
        }
      }
      PFREE (dtmf) 
      break;

    case M_CC_CS_26: /* mobile originating modify */
    case CS_101:
    case CS_261:
      srv_store_prim ((T_PRIM *)D2P(dtmf));
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_reset_dtmf              |
+--------------------------------------------------------------------+

  PURPOSE : Reset DTMF machine. If used before CC leaves any state in 
            which DTMF is possible MMI is informed about DTMF failure 
            if there was still a DTMF request outstanding.

*/

GLOBAL void cc_reset_dtmf (void)
{
  GET_INSTANCE_DATA;
  T_DTMF * p_dtmf;
  BOOL dtmf_aborted;

  TRACE_FUNCTION ("cc_reset_dtmf");

  p_dtmf = &cc_data->dtmf[cc_data->index_ti];

  if (cc_dtmf_allowed())
  {
    /* Stop all possibly running DTMF timers */
    TIMERSTOP (TIMER_DTMF);

    /*  
     * A DTMF acknowledge maybe outstanding for ACI either if the 
     * DTMF state is DTMF_SEND_REQ or 
     * a tone was found in the DTMF send buffer
     */
    dtmf_aborted = (p_dtmf->state EQ DTMF_SEND_REQUEST);
    while (!dtmf_aborted AND !cc_dtmf_empty(p_dtmf))
      dtmf_aborted = (cc_dtmf_read (p_dtmf) NEQ NOT_PRESENT_8BIT);

    if (dtmf_aborted)
    {
      /* Tone dropped. Confirm DTMF request with negative cause */
      PALLOC (dtmf_cnf, MNCC_START_DTMF_CNF);
      dtmf_cnf->ti       = cc_data->ti;
      dtmf_cnf->key      = NOT_PRESENT_8BIT;
      dtmf_cnf->cause    = MNCC_CAUSE_DTMF_ABORTED;
      dtmf_cnf->dtmf_mod = NOT_PRESENT_8BIT;
      PSENDX (MMI, dtmf_cnf);
    }

  }
  p_dtmf->state = DTMF_IDLE;
  p_dtmf->write = 0;
  p_dtmf->read  = 0;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_start_dtmf_ack          |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming start dtmf acknowledge message.

*/

GLOBAL void cc_start_dtmf_ack (T_D_START_DTMF_ACK * start_dtmf_ack)
{
  GET_INSTANCE_DATA;
  T_DTMF               * p_dtmf;

  TRACE_FUNCTION ("cc_start_dtmf_ack()");

  if (cc_dtmf_allowed ())
  {
    if (cc_check_error_flag ())
    {
      p_dtmf = &cc_data->dtmf[cc_data->index_ti];
      
      /* Stop T336 */
      TIMERSTOP (TIMER_DTMF);
      
      if (p_dtmf->state EQ DTMF_SEND_REQUEST)
      {
        PALLOC (start, MNCC_START_DTMF_CNF);
        
        start->ti       = cc_data->ti;
        start->key      = start_dtmf_ack->key_facility.key;
        start->cause    = MNCC_CAUSE_DTMF_START_SUCCESS;
        start->dtmf_mod = (p_dtmf->mode EQ DTMF_AUTO) ?
                          MNCC_DTMF_MOD_AUTO : MNCC_DTMF_MOD_MAN_START;
        PSENDX (MMI, start);

        EM_CC_START_DTMF_ACKNOWLEDGE;

        if( p_dtmf->mode EQ DTMF_AUTO )
        {
          for_stop_dtmf ();
          
          /* On sending a STOP DTMF message the MS shall start T337 */
          TIMERSTART (T337, T337_VALUE);
          
          p_dtmf->state = DTMF_STOP_REQUEST;
        }
        else
        {
          /* This is DTMF_MAN */
          if (!cc_dtmf_empty (p_dtmf))
          {
            UBYTE tone;
            
            tone = cc_dtmf_read (p_dtmf);
            if (tone EQ NOT_PRESENT_8BIT)
            {
              /* Should be always true */
              p_dtmf->mode = GET_DTMF_MOD(tone);
              for_stop_dtmf ();
              
              /* On sending a STOP DTMF message the MS shall start T337 */
              TIMERSTART (T337, T337_VALUE);
              
              p_dtmf->state = DTMF_STOP_REQUEST;
            }
            while (!cc_dtmf_empty(p_dtmf) AND
              p_dtmf->buf[p_dtmf->read] EQ NOT_PRESENT_8BIT)
            {
              /* Consume all unwanted stop marks */
              (void)cc_dtmf_read(p_dtmf);
            }
          }              
          else
            p_dtmf->state = DTMF_SEND_ACKNOWLEDGE;
        }
/* Implements Measure#  3 and streamline encoding */
        CCD_END;
      }
      else
      {

/* Implements Measure#  3 and streamline encoding */
        CCD_END;
        cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      }
/* Implements Measure#  3 and streamline encoding */
    } /* if (cc_check_error_flag (cc_data)) */
  } /* if (cc_dtmf_allowed (cc_data)) */
  else
  {
    /* 
     * DTMF not allowed for some reason, e.g. wrong state,
     * call is not a speech call, no TCH assigned etc.
     */
    CCD_END;
/* Implements Measure#  3 and streamline encoding */
    cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_start_dtmf_rej          |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming start dtmf reject message.

*/

GLOBAL void cc_start_dtmf_rej (T_D_START_DTMF_REJ * start_dtmf_rej)
{
  GET_INSTANCE_DATA;
  T_DTMF               * p_dtmf;

  TRACE_FUNCTION ("cc_start_dtmf_rej()");

  if (cc_dtmf_allowed ())
  {
    if (cc_check_error_flag ())
    {
      /* Stop T336 */
      TIMERSTOP (TIMER_DTMF);
      p_dtmf = &cc_data->dtmf[cc_data->index_ti];
      if (p_dtmf->state EQ DTMF_SEND_REQUEST)
      {
        PALLOC (start, MNCC_START_DTMF_CNF);

        start->ti       = cc_data->ti;
        start->key      = NOT_PRESENT_8BIT;
        start->dtmf_mod = NOT_PRESENT_8BIT;
        /* 
         * cause IE is mandatory and the check for presence has been done already 
         * thus there is no need to check for validity of cause IE here anymore
         */
        start->cause = CAUSE_MAKE(DEFBY_STD, 
                                  ORIGSIDE_NET, 
                                  MNCC_CC_ORIGINATING_ENTITY, 
                                  start_dtmf_rej->cc_cause.cause);
        PSENDX (MMI, start);

        p_dtmf->state = DTMF_IDLE;
        p_dtmf->write = 0;
        p_dtmf->read  = 0;
/* Implements Measure#  3 and streamline encoding */
       CCD_END; 
      }
      else
      {
/* Implements Measure#  3 and streamline encoding */
        CCD_END;
        cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      }
/* Implements Measure#  3 and streamline encoding */
    }
  }
  else
  {
    /* 
     * DTMF not allowed for some reason, e.g. wrong state,
     * call is not a speech call, no TCH assigned etc.
     */
    CCD_END;
/* Implements Measure#  3 and streamline encoding */
    cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_stop_dtmf               |
+--------------------------------------------------------------------+

  PURPOSE : Stops DTMF, back to DTMF_IDLE state.
            Reasons for call: 
            1.) Reception of STOP DTMF ACK message
            2.) Expiry of T336
            3.) Expiry of T337

*/

GLOBAL void cc_stop_dtmf (void)
{
  GET_INSTANCE_DATA;
  T_DTMF               * p_dtmf;

  TRACE_FUNCTION ("cc_stop_dtmf()");

  p_dtmf = &cc_data->dtmf[cc_data->index_ti];

  while (!cc_dtmf_empty(p_dtmf))
  {
    UBYTE tone;
    
    tone = cc_dtmf_read (p_dtmf);
    if (tone NEQ NOT_PRESENT_8BIT)
    {
      /*
       * another DTMF tone has to be sent
       */
      p_dtmf->mode = GET_DTMF_MOD (tone);
      p_dtmf->key  = tone & 0x7f;
      CCD_START;
      {
        MCAST (start, U_START_DTMF);

        cc_build_start_dtmf (p_dtmf->key, start);
        for_start_dtmf (start);
      }
      CCD_END;
      
      /* On sending a START DTMF message the MS shall start T336 */
      TIMERSTART (T336, T336_VALUE);
      
      p_dtmf->state = DTMF_SEND_REQUEST;
      return;
    }
  }
  p_dtmf->state = DTMF_IDLE;
  srv_use_stored_prim ();
}    


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_stop_dtmf_ack           |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming stop dtmf ack message.

*/

GLOBAL void cc_stop_dtmf_ack (void)
{
  GET_INSTANCE_DATA;
  T_DTMF               * p_dtmf;

  TRACE_FUNCTION ("cc_stop_dtmf_ack()");

  if (cc_dtmf_allowed ())
  {
    if (cc_check_error_flag ()) 
    {
      CCD_END;
      p_dtmf = &cc_data->dtmf[cc_data->index_ti];
      if (p_dtmf->state EQ DTMF_STOP_REQUEST)
      {
        if (p_dtmf->mode EQ DTMF_MAN)
        {
          PALLOC (start, MNCC_START_DTMF_CNF);
          start->ti       = cc_data->ti;
          start->key      = NOT_PRESENT_8BIT;
          start->cause    = MNCC_CAUSE_DTMF_STOP_SUCCESS;
          start->dtmf_mod = MNCC_DTMF_MOD_MAN_STOP;
          PSENDX (MMI, start);
        }
        /* Stop T337 */
        TIMERSTOP (TIMER_DTMF);
        cc_stop_dtmf ();
      }
      else
      {
        /* 
         * The STOP DTMF ACK message was not expected as we did not sent
         * previously a START DTMF request.
         */
/* Implements Measure#  3 and streamline encoding */
        cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      }
    }
  }
  else
  {
    /* 
     * DTMF not allowed in the current state 
     */
    CCD_END;    
/* Implements Measure#  3 and streamline encoding */
    cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mncc_facility_req       |
+--------------------------------------------------------------------+

  PURPOSE : Sending of Facility Information by the mobile side.

*/

GLOBAL void cc_mncc_facility_req (T_MNCC_FACILITY_REQ * facility)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_facility_req()");

  if ((cc_data->index_ti = srv_convert_ti (facility->ti))
       EQ NOT_PRESENT_8BIT)
  {
    PFREE (facility);
    return;
  }

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_19:
      PFREE (facility);
      break;

    case M_CC_CS_01:
    case CS_101:
    case CS_261:
      srv_store_prim ((T_PRIM *)D2P(facility));
      break;

    default:
      CCD_START;
      {
        MCAST (facility_msg, U_FACILITY);
        cc_build_facility (facility, facility_msg);
        for_facility (facility_msg);

        EM_CC_FACILITY_TO_THE_NETWORK;

      }
      CCD_END;
      PFREE (facility);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mncc_modify_req         |
+--------------------------------------------------------------------+

  PURPOSE : Start of a mobile originated in-call modification.

*/

GLOBAL void cc_mncc_modify_req (T_MNCC_MODIFY_REQ * modify)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_modify_req()");

  if ((cc_data->index_ti = srv_convert_ti (modify->ti))
      EQ NOT_PRESENT_8BIT)
  {
    PFREE (modify);
    return;
  }

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_10:
      if (((cc_data->hold_state[cc_data->index_ti] EQ M_CC_HLD_IDLE)) AND
           (cc_data->dtmf[cc_data->index_ti].state EQ DTMF_IDLE))
      {
        /*
         * If no other procedure is running
         */
        CCD_START;
        {
          MCAST (modify_msg, B_MODIFY);
          cc_build_modify (modify, modify_msg);
          for_modify (modify_msg);
        }
        CCD_END;
        PFREE (modify);
        cc_set_state (M_CC_CS_26);
        TIMERSTART (T323, T323_VALUE);

        EM_CC_MO_IN_CALL_MODIFICATION;

      }
      else
        srv_store_prim ((T_PRIM *)D2P(modify));
      break;

    default:
      PFREE (modify);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mncc_user_req           |
+--------------------------------------------------------------------+

  PURPOSE : Send user information or congestion control.

*/

GLOBAL void cc_mncc_user_req (T_MNCC_USER_REQ * user)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_user_req()");

  if ((cc_data->index_ti = srv_convert_ti (user->ti))
       EQ NOT_PRESENT_8BIT)
  {
    PFREE (user);
    return;
  }

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_10:
      switch (user->congest_lev)
      {
        case M_CC_CL_RECEIVER_READY:
        case M_CC_CL_RECEIVER_NOT_READY:
          /*
           * Send Congestion Control
           */
          CCD_START;
          {
            MCAST (cong_ctrl, B_CONGEST_CTRL);
            
            cc_build_congestion_control (user, cong_ctrl);
            for_congestion_control (cong_ctrl);
          }
          CCD_END;
          PFREE (user);
          break;
        default:
          /*
           * Send User Information
           */
          CCD_START;
          {
            MCAST (user_msg, B_USER_INFO);

            cc_build_user_information (user, user_msg);
            for_user_information (user_msg);
          }
          CCD_END;
          PFREE (user);
          break;
      }
      break;

    case CS_101:
      srv_store_prim ((T_PRIM *)D2P(user));
      break;

    default:
      PFREE (user);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mncc_hold_req           |
+--------------------------------------------------------------------+

  PURPOSE : Send hold message.

*/

GLOBAL void cc_mncc_hold_req (T_MNCC_HOLD_REQ * hold)
{

  TRACE_FUNCTION ("cc_mncc_hold_req()");

/* Implements Measure# 18 */
  cc_mncc_hold_retrieve_req ((T_PRIM *)D2P(hold));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mncc_retrieve_req       |
+--------------------------------------------------------------------+

  PURPOSE : Send retrieve message.

*/
GLOBAL void cc_mncc_retrieve_req (T_MNCC_RETRIEVE_REQ * retrieve)
{

  TRACE_FUNCTION ("cc_mncc_retrieve_req()");

/* Implements Measure# 18 */
  cc_mncc_hold_retrieve_req ((T_PRIM *)D2P(retrieve));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_call_is_active          |
+--------------------------------------------------------------------+

  PURPOSE : Return TRUE if the respective CC instance is in an active
            state, otherwise return FALSE.

*/

LOCAL BOOL cc_call_is_active (UBYTE index_ti)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_call_is_active()");
  
  switch (cc_data->state[index_ti])
  {
    case M_CC_CS_10:
    case M_CC_CS_26:
    case CS_101:
    case CS_261:
      return TRUE;

    default:
      return FALSE;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mncc_sync_req           |
+--------------------------------------------------------------------+

  PURPOSE : Processing a MNCC_SYNC_REQ primitive. To be able to 
            understand  some details of this function, see first 
            GSM 03.84, GSM 04.84 and GSM 11.10 chapters 31.4.1 and 31.4.2.

*/

GLOBAL void cc_mncc_sync_req (T_MNCC_SYNC_REQ * mncc_sync_req)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("cc_mncc_sync_req()");

  if ((cc_data->index_ti = srv_convert_ti (mncc_sync_req->ti))
      EQ NOT_PRESENT_8BIT)
  {
    PFREE (mncc_sync_req);
    return;
  }

  switch (cc_data->state[cc_data->index_ti])
  {
    /* 
     * Note: MPTY and HOLD only has a meaning in the following states, 
     *       in other states these aux states have absolutely no meaning.
     */
    case M_CC_CS_10:
    case M_CC_CS_26:
    case CS_101:
    case CS_261:
      switch (mncc_sync_req->synccs)
      {
        case MNCC_SYNCCS_MPTY_EVENT:

          TRACE_EVENT_P2 ("  ti = %d, mpty_event = %d", 
                          mncc_sync_req->ti, 
                          mncc_sync_req->mpty_event);

          switch (mncc_sync_req->mpty_event)
          {
            case MNCC_MPTY_BUILD_SENT:
              for (i = 0; i < MAX_CC_CALLS; i++)
              {
                if (cc_call_is_active(i) AND
                    cc_data->mpty_state[i] EQ M_CC_MPTY_IDLE)
                {
                  cc_data->mpty_state[i] = M_CC_MPTY_REQ;
                }
              }
              break;

            case MNCC_MPTY_BUILD_FAIL:
              for (i = 0; i < MAX_CC_CALLS; i++)
              {
                if (cc_call_is_active(i) AND 
                    cc_data->mpty_state[i] EQ M_CC_MPTY_REQ)
                {
                  cc_data->mpty_state[i] =  M_CC_MPTY_IDLE;
                }
              }
              break;

            case MNCC_MPTY_BUILD_SUCCESS:
              for (i = 0; i < MAX_CC_CALLS; i++)
              {
                if (cc_call_is_active(i))
                {
                  cc_data->mpty_state[i] = M_CC_MPTY_CALL;
                  cc_data->hold_state[i] = M_CC_HLD_IDLE;
                }
              }
              break;

            /* -- */
            
            case MNCC_MPTY_SPLIT_SENT:
              if (cc_data->mpty_state[cc_data->index_ti] EQ M_CC_MPTY_CALL)
              {              
                cc_data->mpty_state[cc_data->index_ti] = M_CC_MPTY_SPLIT;
              }
              break;

            case MNCC_MPTY_SPLIT_FAIL:
              if (cc_data->mpty_state[cc_data->index_ti] EQ M_CC_MPTY_SPLIT)
              {              
                cc_data->mpty_state[cc_data->index_ti] = M_CC_MPTY_CALL;
              }
              break;

            case MNCC_MPTY_SPLIT_SUCCESS:
              cc_data->mpty_state[cc_data->index_ti] = M_CC_MPTY_IDLE;
              cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_IDLE;
              for (i = 0; i < MAX_CC_CALLS; i++)
              {
                if (cc_call_is_active(i) AND 
                    cc_data->mpty_state[i] NEQ M_CC_MPTY_IDLE)
                {
                  cc_data->hold_state[i] = M_CC_HLD_CALL_HELD;
                }
              }
              break;

            /* -- */

            case MNCC_MPTY_HOLD_SENT:
              for (i = 0; i < MAX_CC_CALLS; i++)
              {
                if (cc_call_is_active(i) AND 
                    cc_data->mpty_state[i] NEQ M_CC_MPTY_IDLE)
                {
                  cc_data->hold_state[i] = M_CC_HLD_HOLD_REQ;
                }
              }
              break;

            case MNCC_MPTY_HOLD_FAIL:
              for (i = 0; i < MAX_CC_CALLS; i++)
              {
                if (cc_call_is_active(i) AND 
                    cc_data->mpty_state[i] NEQ M_CC_MPTY_IDLE)
                {
                  cc_data->hold_state[i] = M_CC_HLD_IDLE;
                }
              }
              break;

            case MNCC_MPTY_HOLD_SUCCESS:
              for (i = 0; i < MAX_CC_CALLS; i++)
              {
                if (cc_call_is_active(i) AND 
                    cc_data->mpty_state[i] NEQ M_CC_MPTY_IDLE)
                {
                  cc_data->hold_state[i] = M_CC_HLD_CALL_HELD;
                }
              }
              break;

            /* -- */

            case MNCC_MPTY_RETRIEVE_SENT:
              for (i = 0; i < MAX_CC_CALLS; i++)
              {
                if (cc_call_is_active(i) AND 
                    cc_data->mpty_state[i] NEQ M_CC_MPTY_IDLE)
                {
                  cc_data->hold_state[i] = M_CC_HLD_RETRIEVE_REQ;
                }
              }
              break;

            case MNCC_MPTY_RETRIEVE_FAIL:
              for (i = 0; i < MAX_CC_CALLS; i++)
              {
                if (cc_call_is_active(i) AND 
                    cc_data->mpty_state[i] NEQ M_CC_MPTY_IDLE)
                {
                  cc_data->hold_state[i] = M_CC_HLD_CALL_HELD;
                }
              }
              break;

            case MNCC_MPTY_RETRIEVE_SUCCESS:
              for (i = 0; i < MAX_CC_CALLS; i++)
              {
                if (cc_call_is_active(i) AND 
                    cc_data->mpty_state[i] NEQ M_CC_MPTY_IDLE)
                {
                  cc_data->hold_state[i] = M_CC_HLD_IDLE;
                }
              }
              break;

            /* -- */

            default: 
              TRACE_ERROR ("Illegal MPTY event");
              break;
          } /* switch (mncc_sync_req->mpty_event) */
          break; 
          
        default:  
          TRACE_ERROR ("Illegal SYNCCS");
      } /* switch (mncc_sync_req->synccs) */
      break;
      
    default:
      break; /* Ignore the event */
  } /* switch (cc_data->state[cc_data->index_ti]) */

#ifndef NTRACE
  for (i = 0; i < MAX_CC_CALLS; i++)
  {
    if (cc_call_is_active(i))
    {
      TRACE_EVENT_P4 ("  hold[%d] = %d, mpty[%d] = %d", 
                      cc_data->stored_ti_values[i],
                      cc_data->hold_state[i],
                      cc_data->stored_ti_values[i],
                      cc_data->mpty_state[i]);
    }
  }
#endif /* #ifndef NTRACE */

  PFREE (mncc_sync_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mncc_status_res         |
+--------------------------------------------------------------------+

  PURPOSE : Processing a MNCC_STATUS_RES primitive. There is an
            outstanding STATUS message which still has to be sent to 
            the network after having received a STATUS ENQUIRY in U10.

*/

GLOBAL void cc_mncc_status_res (T_MNCC_STATUS_RES * mncc_status_res)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_mncc_status_res()");

  if ((cc_data->index_ti = srv_convert_ti (mncc_status_res->ti))
      EQ NOT_PRESENT_8BIT)
  {
    PFREE (mncc_status_res);
    return;
  }

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_01:  /* MM connection pending */
    case CS_101: /* No network connection, reestablishment in progress */
    case CS_261:
      break;

    default:
/* Implements Measure#  3 and streamline encoding */    
      cc_send_status (M_CC_CAUSE_STATUS_ENQUIRY);
      break;
  }

  PFREE (mncc_status_res);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_facility                |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming facility message.

*/

GLOBAL void cc_facility (T_D_FACILITY * facility)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_facility()");

  /* 
   * A facility message can be received in every state where CC has a 
   * MM connection, no need to check states here.
   */
  if (cc_check_error_flag ())
  {
    cc_build_facility_ind (MNCC_FAC_IN_FACILITY, TRUE, &facility->facility);
    CCD_END;
  }

  EM_CC_FACILITY_FROM_THE_NETWORK;

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_hold_ack                |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming hold ack message.

*/

GLOBAL void cc_hold_ack (void)
{
  GET_INSTANCE_DATA;
  CCD_END;

  TRACE_FUNCTION ("cc_hold_ack()");

  switch (cc_data->state[cc_data->index_ti])
  {
      /*
       * CS_0 is handled by the formatter
       */
    case M_CC_CS_10:
      if (cc_check_error_flag ())
      {
        if (cc_data->hold_state[cc_data->index_ti] EQ M_CC_HLD_HOLD_REQ)
        {
          PALLOC (hold_cnf, MNCC_HOLD_CNF);

          cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_CALL_HELD;
          hold_cnf->ti = cc_data->ti;
          hold_cnf->cause = MNCC_CAUSE_HOLD_SUCCESS;
          PSENDX (MMI, hold_cnf);

          EM_CC_CALL_HOLD_ACKNOWLEDGE;

        }
        else
        {
/* Implements Measure#  3 and streamline encoding */
          cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
        }
      }
      break;

    default:
/* Implements Measure#  3 and streamline encoding */
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_hold_rej                |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming hold rej message.

*/

GLOBAL void cc_hold_rej (T_D_HOLD_REJ * hold_rej)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_hold_rej()");

  switch (cc_data->state[cc_data->index_ti])
  {
      /*
       * CS_0 is handled by the formatter
       */
    case M_CC_CS_10:
      if (cc_data->hold_state[cc_data->index_ti] EQ M_CC_HLD_HOLD_REQ)
      {
        PALLOC (hold_cnf, MNCC_HOLD_CNF);

        cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_IDLE;
        hold_cnf->ti = cc_data->ti;

        /*
         * It is possible for the network to send a REJ without a Cause IE.
         * This causes the decoder to flag a Madatory IE error. According to
         * 04.08 Sec: 8.5.3.e, the REJ should still be managed as if a correctly
         * formatted REJ had been received. 
         */
        if (!hold_rej->cc_cause.v_cause)
        {
          hold_cnf->cause = MNCC_CAUSE_NO_NET_CAUSE;
        }
        else
        {
          hold_cnf->cause = CAUSE_MAKE(DEFBY_STD, 
                                       ORIGSIDE_NET, 
                                       MNCC_CC_ORIGINATING_ENTITY, 
                                       hold_rej->cc_cause.cause);
        }
        PSENDX (MMI, hold_cnf);
        srv_use_stored_prim ();

        EM_CC_CALL_HOLD_REJECT;

        CCD_END;
      }
      else
      {
        CCD_END;
/* Implements Measure#  3 and streamline encoding  */
        cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      }
      break;

    default:
      CCD_END;
/* Implements Measure#  3 and streamline encoding */
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_retrieve_ack            |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming retrieve ack message.

*/

GLOBAL void cc_retrieve_ack (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_retrieve_ack()");

  switch (cc_data->state[cc_data->index_ti])
  {
    /*
     * CS_0 is handled by the formatter
     */
    case M_CC_CS_10:
      if (cc_check_error_flag ())
      {
        CCD_END;
        if (cc_data->hold_state[cc_data->index_ti] EQ M_CC_HLD_RETRIEVE_REQ)
        {
          PALLOC (retrieve_cnf, MNCC_RETRIEVE_CNF);

          cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_IDLE;
          retrieve_cnf->ti = cc_data->ti;
          retrieve_cnf->cause = MNCC_CAUSE_RETRIEVE_SUCCESS;
          PSENDX (MMI, retrieve_cnf);

           EM_CC_CALL_RETRIEVE_ACKNOWLEDGE;

        }
        else
        {
/* Implements Measure#  3 and streamline encoding */
          cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
        }
      }
      break;

    default:
      CCD_END;
/* Implements Measure#  3 and streamline encoding */
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_retrieve_rej            |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming retrieve rej message.

*/

GLOBAL void cc_retrieve_rej (T_D_RETRIEVE_REJ * retrieve_rej)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_retrieve_rej()");

  switch (cc_data->state[cc_data->index_ti])
  {
    /*
     * CS_0 is handled by the formatter
     */
    case M_CC_CS_10:
      if (cc_data->hold_state[cc_data->index_ti] EQ M_CC_HLD_RETRIEVE_REQ)
      {
        PALLOC (retrieve_cnf, MNCC_RETRIEVE_CNF);

        cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_CALL_HELD;
        retrieve_cnf->ti = cc_data->ti;

        /*
         * It is possible for the network to send a REJ without a Cause IE.
         * This causes the decoder to flag a Madatory IE error. According to
         * 04.08 Sec: 8.5.3.e, the REJ should still be managed as if a correctly
         * formatted REJ had been received. 
         */
        if (!retrieve_rej->cc_cause.v_cause)
        {
          retrieve_cnf->cause = MNCC_CAUSE_NO_NET_CAUSE;
        }
        else
        {
          retrieve_cnf->cause = CAUSE_MAKE(DEFBY_STD, 
                                           ORIGSIDE_NET, 
                                           MNCC_CC_ORIGINATING_ENTITY, 
                                           retrieve_rej->cc_cause.cause);
        }
        PSENDX (MMI, retrieve_cnf);
        srv_use_stored_prim ();

        EM_CC_CALL_RETRIEVE_REJECT;

        CCD_END;
      }
      else
      {
        CCD_END;    
/* Implements Measure#  3 and streamline encoding */
        cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      }
      break;

    default:
      CCD_END;
/* Implements Measure#  3 and streamline encoding */
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_congestion_control      |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming congestion control message.

*/

GLOBAL void cc_congestion_control (T_B_CONGEST_CTRL * cong)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_congestion_control()");

  switch (cc_data->state[cc_data->index_ti])
  {
      /*
       * CS_0 is handled by the formatter
       */
    case M_CC_CS_10:
      if (cc_check_error_flag ())
      {
        PALLOC (user, MNCC_USER_IND);

        cc_build_user_ind_from_cong (user, cong);
        PSENDX (MMI, user);
        CCD_END;
      }
      break;

    default:
      CCD_END;
/* Implements Measure#  3 and streamline encoding*/
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_user_information        |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming user information message.

*/

GLOBAL void cc_user_information ( T_B_USER_INFO * user)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_user_information()");

  switch (cc_data->state[cc_data->index_ti])
  {
    /*
     * CS_0 is handled by the formatter
     */
    case M_CC_CS_10:
      if (cc_check_error_flag ())
      {
        PALLOC (user_ind, MNCC_USER_IND);
        
        cc_build_user_ind_from_user (user_ind, user);
        CCD_END;
        PSENDX (MMI, user_ind);
 
        EM_CC_USER_TO_USER_DATA_RECEIVED;

      }
      break;

    default:
      CCD_END;
/* Implements Measure#  4 and streamline encoding*/
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_modify                  |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming modify message.

*/

GLOBAL void cc_modify (T_B_MODIFY * modify)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cc_modify()");

  switch (cc_data->state[cc_data->index_ti])
  {
      /*
       * CS_0 is handled by the formatter
       */
    case M_CC_CS_10:
      if (cc_check_error_flag ())
      {
        switch (cc_data->neg_ri)
        {
          case M_CC_REPEAT_CIRCULAR:
            if (cc_data->active_service EQ cc_data->neg_serv1)
            {
              if (cc_bcs_compatible (&modify->bearer_cap,
                                     &cc_data->neg_bc2, TRUE) EQ TRUE)
              {
                CCD_END;
                cc_mod_complete ();
              }
              else
              {
                CCD_END;
                cc_mod_reject ();
              }
            }
            else
            {
              if (cc_bcs_compatible (&modify->bearer_cap,
                                     &cc_data->neg_bc1, TRUE) EQ TRUE)
              {
                CCD_END;
                cc_mod_complete ();
              }
              else
              {
                CCD_END;
                cc_mod_reject ();
              }
            }
            break;
#ifndef REL99
          case M_CC_REPEAT_SEQUENTIAL:
            if (cc_data->active_service EQ cc_data->neg_serv1)
            {
              if (cc_bcs_compatible (&modify->bearer_cap,
                                     &cc_data->neg_bc2, TRUE) EQ TRUE)
              {
                CCD_END;
                cc_mod_complete ();
              }
              else
              {
                CCD_END;
                cc_mod_reject ();
              }
            }
            else
            {
              CCD_END;
              cc_mod_reject ();
            }
            break;
#endif /* #ifndef REL99 */
          default:
            CCD_END;
            cc_mod_reject ();
            break;
        }
      }
      break;

    default:
      CCD_END;
/* Implements Measure#  5 and streamline encoding*/
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mod_reject              |
+--------------------------------------------------------------------+

  PURPOSE : Negative end of a mobile terminated in-call modification.

*/

GLOBAL void cc_mod_reject (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_mod_reject()");

  CCD_START;
  {
    MCAST (modify_rej, B_MODIFY_REJ);


    cc_build_modify_reject (modify_rej,
                            CAUSE_MAKE(DEFBY_STD, 
                                       ORIGSIDE_MS, 
                                       MNCC_CC_ORIGINATING_ENTITY,
                                       M_CC_CAUSE_BEARER_CAP_AUTHORIZ));
    for_modify_reject (modify_rej);
  }

  EM_CC_MT_IN_CALL_MODIFICATION_FAILED;

  CCD_END;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mod_complete            |
+--------------------------------------------------------------------+

  PURPOSE : Positive end of a mobile terminated in-call modification.

*/

GLOBAL void cc_mod_complete (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_mod_complete()");
  
  CCD_START;
  {
    MCAST (modify_com, B_MODIFY_COMP);
    PALLOC (mod_ind, MNCC_MODIFY_IND);


    if (cc_data->active_service EQ cc_data->neg_serv1)
      cc_data->active_service = cc_data->neg_serv2;
    else
      cc_data->active_service = cc_data->neg_serv1;

    mod_ind->ti = cc_data->ti;
    mod_ind->serv = cc_data->active_service;
    PSENDX (MMI, mod_ind);

    EM_CC_MT_IN_CALL_MODIFICATION_PASSED;

    cc_build_modify_complete (modify_com);
    for_modify_complete (modify_com);
  }
  CCD_END;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_modify_complete         |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming modify complete message.

*/

GLOBAL void cc_modify_complete ( T_B_MODIFY_COMP * mod_com)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_modify_complete()");

  switch (cc_data->state[cc_data->index_ti])
  {
    /*
     * CS_0 is handled by the formatter
     */
    case M_CC_CS_26:
      if (cc_check_error_flag ())
      {
        if (cc_data->new_itc EQ mod_com->bearer_cap.trans_cap)
        {
          PALLOC (mod_cnf, MNCC_MODIFY_CNF);

          TIMERSTOP (TIMER_CC);
          if (cc_data->active_service EQ cc_data->serv1)
            cc_data->active_service = cc_data->serv2;
          else
            cc_data->active_service = cc_data->serv1;
          mod_cnf->ti = cc_data->ti;
          mod_cnf->cause = MNCC_CAUSE_MODIFY_SUCCESS;
          /* Setting raw_cause to empty as we are not sending any cause from network*/ 
          mod_cnf->c_raw_cause = 0;

          EM_CC_MO_IN_CALL_MODIFICATION_PASSED;

          PSENDX (MMI, mod_cnf);
          cc_set_state (M_CC_CS_10);
        }
        CCD_END;
      }
      break;

    default:
      CCD_END;
/* Implements Measure#  6 and streamline encoding*/
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_modify_reject           |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming modify reject message.

*/

GLOBAL void cc_modify_reject (T_B_MODIFY_REJ * mod_rej)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_modify_reject()");

  switch (cc_data->state[cc_data->index_ti])
  {
    /*
     * CS_0 is handled by the formatter
     */
    case M_CC_CS_26:
      if (cc_check_error_flag ())
      {
        if (cc_data->old_itc EQ mod_rej->bearer_cap.trans_cap)
        {
          PALLOC (mod_cnf, MNCC_MODIFY_CNF);

          TIMERSTOP (TIMER_CC);
          mod_cnf->ti = cc_data->ti;
          /* 
           * cause IE is mandatory and the check for presence has been done
           * already thus there is no need to check for validity of cause IE
           * here anymore
           */
          mod_cnf->cause = CAUSE_MAKE(DEFBY_STD, 
                                      ORIGSIDE_NET, 
                                      MNCC_CC_ORIGINATING_ENTITY, 
                                      mod_rej->cc_cause.cause);
          mod_cnf->c_raw_cause = cc_build_cause (&mod_rej->cc_cause,
                                                  mod_cnf->raw_cause);
          PSENDX (MMI, mod_cnf);
          cc_set_state (M_CC_CS_10);
        }

        EM_CC_MO_IN_CALL_MODIFICATION_FAILED;

        CCD_END;
      }
      break;

    default:
      CCD_END;
/* Implements Measure#  7 and streamline encoding*/
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_status                  |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming status message.

*/

GLOBAL void cc_status (T_B_STATUS * status)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cc_status()");

  /* The cause is in STATUS a mandatory IE (given intact message) */
  TRACE_EVENT_P1 ("STATUS cause: %02x", status->cc_cause.cause);

  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_0:
      CCD_END;
      if (status->call_state.state NEQ M_CC_CS_0)
      {
        CCD_START;
        {
          MCAST (rel_com, U_RELEASE_COMP);
 
          cc_build_release_complete (rel_com,
                                     CAUSE_MAKE(DEFBY_STD, 
                                                ORIGSIDE_MS, 
                                                MNCC_CC_ORIGINATING_ENTITY,
                                                M_CC_CAUSE_MESSAGE_INCOMPAT));
          for_release_complete (rel_com);
        }
        CCD_END;
      }
      for_rel_req ();
      break;

    case M_CC_CS_1:
    case M_CC_CS_3:
    case M_CC_CS_4:
    case M_CC_CS_6:
    case M_CC_CS_7:
    case M_CC_CS_8:
    case M_CC_CS_9:
    case M_CC_CS_10:
    case M_CC_CS_26:
      if (cc_check_error_flag ())
      {
        /* check for incompatible state */
        if (status->call_state.state NEQ cc_data->state[cc_data->index_ti])
        {
          PALLOC (rel_ind, MNCC_RELEASE_IND);

          cc_reset_dtmf ();  // China change HM 11.07.00
          
          TIMERSTOP (TIMER_CC);

          rel_ind->ti    = cc_data->ti;

          rel_ind->cause = CAUSE_MAKE(DEFBY_STD, 
                                      ORIGSIDE_MS, 
                                      MNCC_CC_ORIGINATING_ENTITY, 
                                      M_CC_CAUSE_MESSAGE_INCOMPAT);
          rel_ind->c_raw_cause = 0;
          PSENDX (MMI, rel_ind);
 
          CCD_END;
          CCD_START;
          {
            MCAST (rel_com, U_RELEASE_COMP);

            cc_build_release_complete (rel_com,
                                       CAUSE_MAKE(DEFBY_STD, 
                                                  ORIGSIDE_MS, 
                                                  MNCC_CC_ORIGINATING_ENTITY,
                                                  M_CC_CAUSE_MESSAGE_INCOMPAT));
            for_release_complete (rel_com);
          }
          CCD_END;
          for_rel_req ();
          cc_set_state (M_CC_CS_0);
        }
        else 
        /* 
         * compatible, perform checks/ actions of 04.08, ch. 5.5.3.2.2;  
         * Condat action (option!) is to initiate normal call clearing 
         * (with DISCONNECT)
         */
        {
          if ((status->cc_cause.cause >= M_CC_CAUSE_INCORRECT_MESSAGE)
              AND
              (status->cc_cause.cause <= M_CC_CAUSE_COND_INFO_ELEM))
          {
            /* 
             * Block entered for 
             * CAUSE_INCORRECT_MESSAGE, CAUSE_INVALID_MAND_INFO,
             * CAUSE_MESSAGE_TYPE_NOT_IMPLEM, CAUSE_MESSAGE_TYPE_INCOMPAT,
             * CAUSE_INFO_ELEM_NOT_IMPLEM, CAUSE_COND_INFO_ELEM. (0x5f..0x64)
             */
            if ((cc_data->state[cc_data->index_ti] EQ M_CC_CS_10) AND
                (cc_data->hold_state[cc_data->index_ti] EQ M_CC_HLD_HOLD_REQ))
            {
              /* 
               * Compatible state, last CC message was a hold request which is
               * not supported by the network. Handled like HOLD REJECT.
               */
              PALLOC (hold_cnf, MNCC_HOLD_CNF); /* T_MNCC_HOLD_CNF */
              cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_IDLE;
              hold_cnf->ti = cc_data->ti;
              hold_cnf->cause = CAUSE_MAKE(DEFBY_STD, 
                                           ORIGSIDE_NET, 
                                           MNCC_CC_ORIGINATING_ENTITY, 
                                           status->cc_cause.cause);
              PSENDX (MMI, hold_cnf);
              CCD_END;
              srv_use_stored_prim ();
            }
            else if (status->cc_cause.cause NEQ M_CC_CAUSE_INFO_ELEM_NOT_IMPLEM)
            {
              /* 
               * Compatible state, special condition for HOLD REQUEST not met.
               * The network is also not complaining about an optional 
               * information element which can be simply ignored.
               * Disconnect the call.
               */              
              PALLOC (disc_ind, MNCC_DISCONNECT_IND);

              TIMERSTOP (TIMER_CC);

              disc_ind->ti    = cc_data->ti;
              disc_ind->cause = CAUSE_MAKE(DEFBY_STD, 
                                           ORIGSIDE_NET, 
                                           MNCC_CC_ORIGINATING_ENTITY, 
                                           status->cc_cause.cause);
              disc_ind->c_raw_cause = cc_build_cause (&status->cc_cause,
                                                       disc_ind->raw_cause);
              disc_ind->diagnostic = NOT_PRESENT_8BIT;
              disc_ind->progress_desc = MNCC_PROG_NOT_PRES;
              PSENDX (MMI, disc_ind);

              CCD_END;
              CCD_START;
              {
                MCAST (disconnect, U_DISCONNECT);
 
                cc_build_disconnect (disconnect, 
                                     CAUSE_MAKE(DEFBY_STD, 
                                                ORIGSIDE_MS, 
                                                MNCC_CC_ORIGINATING_ENTITY,
                                                M_CC_CAUSE_CALL_CLEAR),
                                     NULL, MNCC_SS_VER_NOT_PRES);
                for_disconnect (disconnect);
              }
              TIMERSTART (T305, T305_VALUE);
              cc_set_state (M_CC_CS_11);
            }
          }
          else 
          {
            /* 
             * STATUS message but neither incompatible state nor indicating 
             * incorrect message / IE; check if we received a STATUS with cause
             * "message not compatible with protocol state". GSM 04.08 does not
             * define what to do and also does not define that this case has to 
             * be handled at all;
             * Condat (HM) decided to "clear DTMF" if it was running to try to 
             * recover from the misaligned states scenario.
             */
            T_DTMF *p_dtmf;

            p_dtmf = &cc_data->dtmf[cc_data->index_ti];
            if ((p_dtmf->state EQ DTMF_SEND_REQUEST) AND
                (status->cc_cause.cause EQ M_CC_CAUSE_MESSAGE_INCOMPAT))
            {
              /* Send negative acknowledge for DTMF send request to MMI */
              PALLOC (mncc_start_dtmf_cnf, MNCC_START_DTMF_CNF);
              mncc_start_dtmf_cnf->ti       = cc_data->ti;
              mncc_start_dtmf_cnf->key      = NOT_PRESENT_8BIT;
              mncc_start_dtmf_cnf->dtmf_mod = NOT_PRESENT_8BIT;
              mncc_start_dtmf_cnf->cause = CAUSE_MAKE(DEFBY_STD, 
                                                      ORIGSIDE_NET, 
                                                      MNCC_CC_ORIGINATING_ENTITY, 
                                                      M_CC_CAUSE_MESSAGE_INCOMPAT);
              PSENDX (MMI, mncc_start_dtmf_cnf);
            }

            /* Bring screwed up network back to work */
            for_stop_dtmf ();

            /* Reset screwed up DTMF state machine */
            p_dtmf->state = DTMF_IDLE;
            p_dtmf->read  = 0;             
            p_dtmf->write = 0;
          }
          CCD_END;
        }
      }
      break;

    case M_CC_CS_11:
    case M_CC_CS_12:
      if (cc_check_error_flag ())
      {
        if (status->call_state.state NEQ cc_data->state[cc_data->index_ti])
        {
          /* 
           * STATUS message with incompatible state
           * On receipt of a STATUS message reporting an incompatible call
           * control state, the receiving entity shall clear the call by 
           * sending a RELEASE COMPLETE message with cause # 101 "message 
           * not compatible with protocol state". The reported call control
           * state is incompatible if the combination of call control states
           * at the sender and receiver side cannot occur, do not match or cannot
           * be aligned by actions of the receiver; the exact definition is 
           * implementation dependent. [04.08 clause 5.5.3.2.1] 
           */
          cc_reset_dtmf ();
          
          TIMERSTOP (TIMER_CC);
          
          /* Send MNCC_RELEASE_IND to MMI */
          {
            PALLOC (rel_ind, MNCC_RELEASE_IND); /* T_MNCC_RELEASE_IND */
            rel_ind->ti    = cc_data->ti;
            rel_ind->cause = CAUSE_MAKE(DEFBY_STD, 
                                        ORIGSIDE_MS, 
                                        MNCC_CC_ORIGINATING_ENTITY, 
                                        M_CC_CAUSE_MESSAGE_INCOMPAT);
            rel_ind->c_raw_cause = 0;
            PSENDX (MMI, rel_ind);
          }

          CCD_END;
          CCD_START;
          {
            MCAST (rel_com, U_RELEASE_COMP); /* T_U_RELEASE_COMP */
            cc_build_release_complete (rel_com, 
                                       CAUSE_MAKE(DEFBY_STD, 
                                                  ORIGSIDE_MS, 
                                                  MNCC_CC_ORIGINATING_ENTITY, 
                                                  M_CC_CAUSE_MESSAGE_INCOMPAT));
            for_release_complete (rel_com);
            for_rel_req ();
            cc_set_state (M_CC_CS_0);
          }
        }
        CCD_END;
      }
      break;

    case M_CC_CS_19:
      if (cc_check_error_flag ())
      {
        if (status->call_state.state NEQ cc_data->state[cc_data->index_ti])
        {
          TIMERSTOP (TIMER_CC);

          cc_reset_dtmf ();

          {
            PALLOC (rel_cnf, MNCC_RELEASE_CNF); /* T_MNCC_RELEASE_CNF */
            rel_cnf->ti    = cc_data->ti;
            rel_cnf->cause = CAUSE_MAKE(DEFBY_STD, 
                                        ORIGSIDE_MS, 
                                        MNCC_CC_ORIGINATING_ENTITY, 
                                        M_CC_CAUSE_MESSAGE_INCOMPAT);
            /* Setting raw_cause to empty as this is a local release
             * of MM connection,CC is not receiving any cause value
             * from Network
             */
            rel_cnf->c_raw_cause = 0;
            PSENDX (MMI, rel_cnf);
          }

          CCD_END;
          CCD_START;
          {
            MCAST (rel_com, U_RELEASE_COMP); /* T_U_RELEASE_COMP */
            cc_build_release_complete (rel_com, 
                                       CAUSE_MAKE(DEFBY_STD, 
                                                  ORIGSIDE_MS, 
                                                  MNCC_CC_ORIGINATING_ENTITY, 
                                                  M_CC_CAUSE_MESSAGE_INCOMPAT));
            for_release_complete (rel_com);
            for_rel_req ();
            cc_set_state (M_CC_CS_0);
          }
        }
        CCD_END;
      }
      break;

    default:
      CCD_END;
      break;
  }

  EM_CC_STATUS_RECEIVED;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_status_enquiry          |
+--------------------------------------------------------------------+

  PURPOSE : Processing an incoming status enquiry message.

*/

GLOBAL void cc_status_enquiry (void)
{
  GET_INSTANCE_DATA;
 
  TRACE_FUNCTION ("cc_status_enquiry()");

  /*
   * CS_0 is handled by the formatter
   */
  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_10:
      /*
       * Ensure synchronization of auxiliary states with ACI.
       */
      if (cc_check_error_flag ())
      {
        CCD_END;
        {
          PALLOC (mncc_status_ind, MNCC_STATUS_IND);
          mncc_status_ind->ti = cc_data->ti;
          PSENDX (MMI, mncc_status_ind);
        }

      EM_CC_STATUS_ENQUIRY_RECEIVED;

      }
      break;

    default:
      /*
       * No auxiliary states in non-active states. Immediate response is safe.
       */
      if (cc_check_error_flag ())
      {
        CCD_END;
        /* EM call moved above cc_send_status should not have any impact */
        EM_CC_STATUS_ENQUIRY_RECEIVED;

/* Implements Measure#  7 and streamline encoding*/
        cc_send_status (M_CC_CAUSE_STATUS_ENQUIRY);  
      }
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_unknown_message         |
+--------------------------------------------------------------------+

  PURPOSE : Processing of an incoming unknown message.

*/

GLOBAL void cc_unknown_message (void)
{

  CCD_END;
  CCD_START;
  {
    MCAST (status, B_STATUS);

    TRACE_FUNCTION ("cc_unknown_message()");

    cc_build_status (status, 
                     CAUSE_MAKE(DEFBY_STD,
                                ORIGSIDE_MS,
                                MNCC_CC_ORIGINATING_ENTITY,
                                M_CC_CAUSE_MESSAGE_TYPE_NOT_IMPLEM));
    for_status (status);
  }
  CCD_END;
}

/* Implements Measure#  18 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_ACT                     |
| STATE   : code                ROUTINE : cc_mncc_hold_retrieve_req  |
+--------------------------------------------------------------------+

  PURPOSE : Send hold REQ/retrieve REQ.

*/

LOCAL void cc_mncc_hold_retrieve_req (T_PRIM * prim)
{ 
  GET_INSTANCE_DATA;
  UBYTE      ti;

  TRACE_FUNCTION ("cc_mncc_hold_retrieve_req()");

  if(prim->custom.opc EQ MNCC_HOLD_REQ)
  {
    ti = ((T_MNCC_HOLD_REQ *)P2D(prim))->ti;    
  }
  else
  {
    ti = ((T_MNCC_RETRIEVE_REQ *)P2D(prim))->ti;
  }
  if ((cc_data->index_ti = srv_convert_ti (ti)) EQ NOT_PRESENT_8BIT)
  {
    PFREE (P2D(prim));
    return;
  }
  
  switch (cc_data->state[cc_data->index_ti])
  {
    case M_CC_CS_10:
      if(prim->custom.opc EQ MNCC_HOLD_REQ)
      {
        /* We should think about storing the hold request 
           until DTMF idle state reached. ...
           This causes some other changes, we have to add some 
           calls to serv_use_stored_prim() at some places after
           state transition of the DTMF state machine to DTMF idle.*/
        cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_HOLD_REQ;
        for_hold ();
        EM_CC_CALL_HOLD;
      }
      else
      {
        cc_data->hold_state[cc_data->index_ti] = M_CC_HLD_RETRIEVE_REQ;
        for_retrieve ();
        EM_CC_CALL_RETRIEVE;
      }
      PFREE (P2D(prim));     

      break;

    case CS_101:
    case M_CC_CS_26:
    case CS_261:
      srv_store_prim (prim);
      break;

    default:
      PFREE (P2D(prim));
      break;
  }
}

#endif
