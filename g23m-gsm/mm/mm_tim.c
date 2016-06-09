/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_TIM
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
|  Purpose :  This Modul defines the timer handling functions
|             for the component MM of the mobile station
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_TIM_C
#define MM_TIM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_MM

/*==== INCLUDES ===================================================*/
#if defined (NEW_FRAME)

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_mm.h"
#include "mon_mm.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"

#else

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "stddefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_mm.h"
#include "mon_mm.h"
#include "vsi.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"

#endif
/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

#if defined (OPTION_TIMER)
LOCAL T_TIMER_CONFIG       config_table[NUM_OF_MM_TIMERS];
#endif


/*==== FUNCTIONS ==================================================*/
#if defined (OPTION_TIMER)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : MM_TIM                   |
| STATE   : code                  ROUTINE : tim_init_timer           |
+--------------------------------------------------------------------+

  PURPOSE : Initialise Time-out FIFO and configuration data.

*/


GLOBAL BOOL tim_init_timer (void)
{
  USHORT i;

  TRACE_FUNCTION ("tim_init_timer()");

  for (i = 0; i < NUM_OF_MM_TIMERS; i++)
  {
    config_table[i].t_mode = TIMER_RESET;
    config_table[i].t_val  = 0L;
  }

  return TRUE;
}
#endif /* #if defined (OPTION_TIMER) */

#if defined (OPTION_TIMER)

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : MM_TIM                   |
| STATE   : code                  ROUTINE : tim_config_timer         |
+--------------------------------------------------------------------+

  PURPOSE : Configure Timer

*/

GLOBAL void tim_config_timer (UBYTE t_num, UBYTE t_mod, ULONG t_val)
{
  TRACE_FUNCTION ("tim_config_timer()");

  assert (t_num < NUM_OF_MM_TIMERS);

  if (t_num < NUM_OF_MM_TIMERS)
  {
    config_table[t_num].t_mode = t_mod;
    config_table[t_num].t_val  = t_val;
  }
  else
  {
    TRACE_ERROR ("tim_config_timer(): index out of range");
  }
}
#endif /* #if defined (OPTION_TIMER) */

#if defined (NEW_FRAME)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : MM_TIM                   |
| STATE   : code                  ROUTINE : tim_exec_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : execute timeout

*/

GLOBAL void tim_exec_timeout (USHORT index)
{
  GET_INSTANCE_DATA;
  /* typedef void (*T_VOID_FUNC)(); */ /* Already defined */

  static const T_VOID_FUNC timer_jump_table[NUM_OF_MM_TIMERS] = 
  {
    tim_t_reg,    /* T_REGISTRATION */
    tim_t3210,    /* T3210 */
    tim_t3211,    /* T3211 */
    tim_t3212,    /* T3212 */
    tim_t3213,    /* T3213 */
    tim_t3220,    /* T3220 */
    tim_t3230,    /* T3230 */
    tim_t3240,    /* T3240 */

    tim_t_hplmn   /* T_HPLMN */
#ifdef REL99
    , tim_t3241    /* T3241 */
#endif
  };

  if (index < NUM_OF_MM_TIMERS)
  {
    /* 
     * Timeout is handled in SDL like the reception of a primitive, 
     * so enable also this trace if primitive traces are enabled only
     */
/* Implements Measure#36 */
#if defined(NCONFIG)
    /* partab is not defined when NCONFIG is defined */
    TRACE_EVENT_P1 ("tim_exec_timeout: index (%d)", index);
#else /* not (NCONFIG) */
#if defined (TRACE_PRIM) AND defined(OPTION_TIMER)
    TRACE_EVENT_P1 ("tim_exec_timeout (%s)", partab[index].keyword);
#endif
#endif /* NCONFIG */

    mm_data->t_running[index] = FALSE;
    timer_jump_table[index]();  
  }
  else
  {
    TRACE_ERROR ("tim_exec_timeout(): index out of range");
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : MM_TIM                   |
| STATE   : code                  ROUTINE : tim_stop_timer           |
+--------------------------------------------------------------------+

  PURPOSE : stop timer

*/

GLOBAL void tim_stop_timer (USHORT index)
{
  GET_INSTANCE_DATA;
  assert (index < NUM_OF_MM_TIMERS);

  if (index < NUM_OF_MM_TIMERS)
  {
    /* Implements Measure#36 */
  #if defined(NCONFIG)
    /* partab is not defined when NCONFIG is defined */
    TRACE_EVENT_P1 ("tim_stop_timer: index (%d)", index);
  #else /* not (NCONFIG) */
  #if defined (TRACE_PRIM) AND defined(OPTION_TIMER)
    TRACE_EVENT_P1 ("tim_stop_timer (%s)", partab[index].keyword);
  #endif
  #endif /* NCONFIG */

    mm_data->t_running[index] = FALSE;
    TIMER_STOP (mm_handle, index);
  }
  else
  {
    TRACE_ERROR ("tim_stop_timer(): index out of range");
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : MM_TIM                   |
| STATE   : code                  ROUTINE : tim_start_timer          |
+--------------------------------------------------------------------+

  PURPOSE : start timer

*/

GLOBAL void tim_start_timer (USHORT index, T_TIME value)
{
  GET_INSTANCE_DATA;
  assert (index < NUM_OF_MM_TIMERS);

  if (index < NUM_OF_MM_TIMERS)
  {
    /* Implements Measure#36 */
  #if defined(NCONFIG)
    /* partab is not defined when NCONFIG is defined */
    TRACE_EVENT_P1 ("tim_start_timer: index (%d)", index);
  #else /* not (NCONFIG) */
  #if defined (TRACE_PRIM) AND defined(OPTION_TIMER)
    TRACE_EVENT_P1 ("tim_start_timer (%s)", partab[index].keyword);
  #endif
  #endif /* NCONFIG */
  #if defined (OPTION_TIMER)
    switch (config_table[index].t_mode)
    {
      case TIMER_SET:
        value = config_table[index].t_val;
        break;

      case TIMER_RESET:
        value = value;
        break;

      case TIMER_SPEED_UP:
        value = value / config_table[index].t_val;
        if (value == 0)
          value = 1;
        TRACE_EVENT_P1 ("timer_speed_up (%d)", value);
        break;

      case TIMER_SLOW_DOWN:
        value = value * config_table[index].t_val;
        TRACE_EVENT_P1 ("timer_speed_down (%d)", value);
        break;

      default:
        TRACE_FUNCTION ("ERROR: UNKNOWN MODE");
        return;
    }
  #endif

    mm_data->t_running[index] = TRUE;

    TIMER_START (mm_handle, index, value);
  }
  else
  {
    TRACE_ERROR ("tim_start_timer(): index out of range");
  }
}

#endif /* #if defined (NEW_FRAME) */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_TIM                     |
| STATE   : code                ROUTINE : tim_t3210                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T3210

*/

GLOBAL void tim_t3210 (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t3210()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED:
    case MM_WAIT_FOR_RR_CONN_LUP:
      TIMERSTOP (T3240);
      mm_abort_connection (ABCS_NORM);
      /* 
       * The RR connection is aborted normally. RR guarantees that this will
       * be answered by RR_RELEASE_IND.
       * This has the advange that GMM gets the MMGMM_NREG_IND after the
       * channel release of layer 2 if GPRS present without any disadvantage 
       * for a GSM only protocol stack. No state change here.
       */
      break;

    default: /* Ignore event */
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_TIM                     |
| STATE   : code                ROUTINE : tim_t3211                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T3211

*/

// T3211 handling routine does the same as T3213 handling routine now ...

GLOBAL void tim_t3211 (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t3211()");

  switch (GET_STATE (STATE_MM))
  {
// If we leave these states and reenter a full service IDLE state,
// mm_release_rr_connection() 
// for MM_WAIT_FOR_OUTG_MM_CONN and MM_WAIT_FOR_RR_CONN_MM will 
// handle the update.
// mm_rr_activate_cnf() for MM_WAIT_FOR_RR_ACTIVE will also 
// handle the an outstanding update if coming back to full 
// service IDLE state.
// mm_rr_abort_ind() will be called if the state was 
// MM_PLMN_SEARCH_NORMAL_SERVICE, if there is an outstanding
// updating procedure and the new service is full service, 
// this will be checked there after state transition.
// MM_IDLE_NO_CELL_AVAILABLE / RR_ACTIVATE_IND is handled also.
// No need to store the timer anymore into the queue.

/*
    // case MM_WAIT_FOR_OUTG_MM_CONN:
    // case MM_WAIT_FOR_RR_CONN_MM:
    // case MM_WAIT_FOR_RR_ACTIVE:
    case MM_PLMN_SEARCH_NORMAL_SERVICE:
    // case MM_IDLE_NO_CELL_AVAILABLE:
      mm_write_entry (TIMEOUT, T3211, 0);
      break;
*/

    case MM_IDLE_ATTEMPT_TO_UPDATE:
    case MM_IDLE_NORMAL_SERVICE:
      mm_continue_running_update ();
      break;

    default: /* Ignore event */
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_TIM                     |
| STATE   : code                ROUTINE : tim_t3212                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of counter timer for timer T3212

*/

GLOBAL void tim_t3212 (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t3212()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:
    case MM_IDLE_NO_IMSI:
      break; /* Forget the event */

    case MM_IDLE_NORMAL_SERVICE:
      mm_data->t3212_timeout = TRUE;
      mm_data->attempt_cnt = 0; /* Expiry of timer T3212 */
      if (!mm_normal_upd_needed())
      {
        /* MM is updated on the cell, no Imm Ass Rej, no cell barred */
        if (mm_lup_allowed_by_gmm())
        {
          mm_periodic_loc_upd ();
        }
        else
        {
          mm_mmgmm_lup_needed_ind (MMGMM_T3212);
          /* No state change, remains in MM_IDLE_NORMAL_SERVICE */
        }
      }
      break;

    case MM_IDLE_ATTEMPT_TO_UPDATE:
      if (mm_data->mm.mm_info.t3212 NEQ T3212_NO_PRD_UPDAT)
      {
        mm_data->t3212_timeout = TRUE;
        mm_data->attempt_cnt = 0; /* Expiry of timer T3212 */
        if (mm_lup_allowed_by_gmm())
        {
          mm_normal_loc_upd ();
        }
        else
        {
          mm_mmgmm_lup_needed_ind (MMGMM_T3212);
          /* No state change, remains in MM_IDLE_ATTEMPT_TO_UPDATE */
        }
      }
      break;

    default: /* Store the event until it is possible to handle it */
      mm_data->t3212_timeout = TRUE;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_TIM                     |
| STATE   : code                ROUTINE : tim_t3213                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T3213

*/

GLOBAL void tim_t3213 (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t3213()");

  switch (GET_STATE (STATE_MM))
  {
/* 
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_RR_ACTIVE:
    case MM_PLMN_SEARCH_NORMAL_SERVICE:
      mm_write_entry (TIMEOUT, T3213, 0);
      break;
*/

    case MM_IDLE_ATTEMPT_TO_UPDATE:
    case MM_IDLE_NORMAL_SERVICE:
      /* 
       * if something is received from RR or T3213 was already restarted 2 times --> delay of additional 8 seconds
       * continue the LUP attempts
       */
      mm_data->t3213_restart++;
      if (mm_data->t3213_restart > MAX_REST_T3213)
        mm_continue_running_update ();
      else
        TIMERSTART (T3213, T_3213_VALUE);
      break;

    default: /* Ignore event */
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_TIM                     |
| STATE   : code                ROUTINE : tim_t3220                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T3220

*/

GLOBAL void tim_t3220 (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t3220()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_IMSI_DETACH_INIT:
    case MM_WAIT_FOR_RR_CONN_DETACH:
      /* 
       * The RR connection is aborted normally. RR guarantees that this will
       * be answered by RR_RELEASE_IND. If MM receives the RR_RELEASE_IND,
       * the IMSI DETACH procedure ends and appropriate actions are taken.
       * This has the advange that GMM gets the MMGMM_NREG_CNF after the 
       * channel release of layer 2 if GPRS present without any disadvantage 
       * for a GSM only protocol stack. No state change here.
       */
      mm_abort_connection (ABCS_NORM);
      break;

    default: /* Ignore event */
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_TIM                     |
| STATE   : code                ROUTINE : tim_t3230                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T3230

*/

GLOBAL void tim_t3230 (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t3230()");

  /* 
   * If T3230 expires (i.e. no response is given but a RR connection is 
   * available) the MM connection establishment is aborted and the requesting
   * CM sublayer is informed. If no other MM connection exists then the mobile
   * station shall proceed as described in section 4.5.3.1 for release of the 
   * RR connection. Otherwise the mobile station shall return to the MM 
   * sublayer state where the request of an MM connection was received,
   * i.e. to MM sublayer state MM connection active. Other ongoing
   * MM connections (if any) shall not be affected.
   * [GSM 04.08 subclause 4.5.1.2 b)]
   * 
   * If all MM connections are released by their CM entities, the
   * mobile station shall set timer T3240 and enter the state
   * WAIT FOR NETWORK COMMAND, expecting the release of the RR connection.
   *  [Excerpt from GSM 04.08 subclause 4.5.3.1]
   */
  
  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_REESTABLISH:
    case MM_WAIT_FOR_OUTG_MM_CONN:
      mm_mmxx_rel_ind (MMCS_TIMER_RECOVERY, CM_ACTIVE);
      mm_mmxx_rel_ind (MMCS_TIMER_RECOVERY, CM_PENDING);
      mm_data->wait_for_accept = FALSE;
      TIMERSTART (T3240, T_3240_VALUE);
      SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
      break;

    case MM_CONN_ACTIVE: /* wait_for_accept expected to be TRUE */
      mm_mmxx_rel_ind (MMCS_TIMER_RECOVERY, CM_PENDING);
      mm_data->wait_for_accept = FALSE;
      if ((mm_count_connections (CM_ACTIVE) EQ 0) AND
          (mm_count_connections (CM_STORE)  EQ 0))
      {
        TIMERSTART (T3240, T_3240_VALUE);
        SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
      }
      USE_STORED_ENTRIES();
      break;

    default: /* Ignore event */
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_TIM                     |
| STATE   : code                ROUTINE : tim_t3240                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T3240

*/

GLOBAL void tim_t3240 (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t3240()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
      mm_abort_connection (ABCS_NORM);
      mm_mmxx_rel_ind (MMCS_TIMER_RECOVERY, CM_ACTIVE);
      mm_mmxx_rel_ind (MMCS_TIMER_RECOVERY, CM_PENDING);
      
      /* 
       * The RR connection is aborted normally. RR guarantees that this will
       * be answered by RR_RELEASE_IND.
       * This has the advange that GMM gets the MMGMM_CM_RELEASE_IND after the
       * channel release of layer 2 if GPRS present without any disadvantage 
       * for a GSM only protocol stack. No state change here.
       */
      break;

    case MM_LUP_REJECTED:
      mm_abort_connection (ABCS_NORM);
      /* 
       * The RR connection is aborted normally. RR guarantees that this will
       * be answered by RR_RELEASE_IND.
       * This has the advange that GMM gets the MMGMM_NREG_IND after the
       * channel release of layer 2 if GPRS present without any disadvantage 
       * for a GSM only protocol stack. No state change here.
       */
      break;

    default: /* Ignore event */
      break;
  }
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_TIM                     |
| STATE   : code                ROUTINE : tim_t3241                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T3241

*/

GLOBAL void tim_t3241 (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t3241()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
      mm_abort_connection (ABCS_NORM);
      /*
       * The RR connection is aborted normally. RR guarantees that this will
       * be answered by RR_RELEASE_IND.
       * This has the advange that GMM gets the MMGMM_CM_RELEASE_IND after the
       * channel release of layer 2 if GPRS present without any disadvantage
       * for a GSM only protocol stack. No state change here.
       */
      break;
    default: /* Ignore event */
      break;
  }
}
#endif


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_TIM                             |
| STATE   : code                ROUTINE : tim_t_hplmn                        |
+----------------------------------------------------------------------------+

  PURPOSE : Timeout of timer T_HPLMN. 
            This timer allows control of the PPLMN rescan and
            national roaming procedure to recover the HPLMN.
*/

GLOBAL void tim_t_hplmn (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t_hplmn()");

  /*
   * Launch the scan procedure if it still makes sense to do it according to
   * MM state.
   */
  if (mm_full_service_pplmn_scan())
  {
    mm_data->plmn_scan_mm = TRUE;
    mm_data->first_attach_mem = FALSE;	
    mm_func_mmgmm_net_req();
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_TIM                     |
| STATE   : code                ROUTINE : tim_t_reg                  |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T_REG. 
            This timer is not foreseen by the recommendations. 
            It is MM's health monitor timer, checking 
            the conditions whether an update has been missed.

*/

GLOBAL void tim_t_reg (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t_reg()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:
      break; /* No timer activity in this state */
  
    case MM_IDLE_NORMAL_SERVICE: /* 19.1 */
      /* Restart the registration timer */
      TIMERSTART (T_REGISTRATION, T_REG_VALUE);      
    
      /* 
       * As it is not expected that the timer catches in state 
       * MM_IDLE_NORMAL_SERVICE in FTA, there is no test 
       * here for a test SIM. The goal is to have an MM
       * where the timer never catches in MM_IDLE_NORMAL_SERVICE.
       */
      
      /* Check whether T3211, T3213 are running retriggering update anyway */
      if ((TIMERACTIVE (T3211) OR TIMERACTIVE (T3213)) AND 
          (mm_data->loc_upd_type.lut NEQ NOT_RUNNING))
        return;

      /* Check whether MM is temporary barred and cannot update now */
      if ((mm_data->idle_entry EQ RRCS_ACCESS_BARRED) OR
          (mm_data->idle_entry EQ RRCS_RND_ACC_DELAY))
        return; 
        
      /* 
       * Check whether we are in an ATTACH update procedure, but there is no
       * T3211, T3213 timer running and MM is not temporary barred.
       */
      if (mm_attach_upd_needed() OR mm_normal_upd_needed())
      {
        TRACE_ERROR ("Recover ATTACH/NORMAL");
        
        mm_mmgmm_lup_needed_ind (MMGMM_REG_TIMER);

        if (mm_normal_upd_needed ())
        {
          mm_data->attempt_cnt = 0; /* New location area */
          mm_normal_loc_upd ();
        }
        else
        {
          mm_attach_loc_upd ();
        }
        return;
      }
 
      /* 
       * Check whether T3212 should run, but is not running.
       * If so, something irregular has happened and 
       * we have to start the update immediately.
       */
      if (mm_data->mm.mm_info.t3212 NEQ T3212_NO_PRD_UPDAT AND 
          (!TIMERACTIVE (T3212) OR mm_data->t3212_timeout))
      {
        /* 
         * The networks says we have periodic updating, 
         * but unexpectedly T3212 is not running or T3212 timed out.
         */
        TRACE_ERROR ("Recover PERIODIC");
        
        if (mm_lup_allowed_by_gmm())
        {
          mm_periodic_loc_upd ();
        }
        else
        {
          // Don't add recovery code now for GPRS, maybe more has to be done.
          // mm_mmgmm_lup_needed_ind (MMGMM_REG_TIMER);
        }
        return;
      }
      break;

    case MM_IDLE_ATTEMPT_TO_UPDATE: /* 19.2 */
      /* Restart the registration timer */
      TIMERSTART (T_REGISTRATION, T_REG_VALUE);      

      /* Timer only handled in this state if a normal SIM is present */
      if (mm_data->reg.op.sim_ins EQ SIM_NO_INSRT OR 
          mm_data->reg.op.ts EQ TS_AVAIL)
        return;

      /* Check whether T3211, T3213 are running retriggering update anyway */
      if ((TIMERACTIVE (T3211) OR TIMERACTIVE (T3213)) AND 
          (mm_data->loc_upd_type.lut NEQ NOT_RUNNING))
        return;

      if (TIMERACTIVE (T3212))
        return;


      /* Check whether MM is temporary barred and cannot update now */
      if ((mm_data->idle_entry EQ RRCS_ACCESS_BARRED) OR
          (mm_data->idle_entry EQ RRCS_RND_ACC_DELAY))
        return; 
        
      if (mm_gsm_alone ())
      {
        mm_normal_loc_upd ();
      }
      else
      {
        mm_mmgmm_lup_needed_ind (MMGMM_REG_TIMER);
        /* No state change, remains in MM_IDLE_ATTEMPT_TO_UPDATE */
      }
      break;

    default:
      /* Restart the registration timer */
      TIMERSTART (T_REGISTRATION, T_REG_VALUE);      
      break;
  }
}

#endif
