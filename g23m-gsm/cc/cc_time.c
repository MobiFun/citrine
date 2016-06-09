/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CC_TIME
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
|             for the component CC of the mobile station
+----------------------------------------------------------------------------- 
*/ 

#ifndef CC_TIM_C
#define CC_TIM_C

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
/*==== PRIVAT =====================================================*/
/*==== PROTOTYPES =====================================================*/
/* Implements Measure#  30, 31 */
LOCAL void tim_mncc_rel_ind (void);
/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CC_TIM                   |
| STATE   : code                  ROUTINE : tim_exec_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : execute timeout

*/

GLOBAL void tim_exec_timeout (USHORT index)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_exec_timeout()");

  /*
   * Processing of Timeout Function
   */
  if (index < MAX_CC_CALLS)
  {
    /* 
     * Not DTMF (T336, T337), any other CC timer 
     */
    cc_data->ti        = cc_data->stored_ti_values[index];
    cc_data->index_ti  = (UBYTE)index;
    
    switch (cc_data->state[cc_data->index_ti])
    {
      case M_CC_CS_01:
        tim_t303_u0_1 ();
        break;
      case M_CC_CS_03:
        tim_t332_u0_3 ();
        break;
      case M_CC_CS_05:
        tim_t335_u0_5 ();
        break;
      case M_CC_CS_1:
        tim_t303_u1 ();
        break;
      case M_CC_CS_3:
        tim_t310_u3 ();
        break;
      case M_CC_CS_8:
        tim_t313_u8 ();
        break;
      case M_CC_CS_11:
        tim_t305_u11 ();
        break;
      case M_CC_CS_19:
        tim_t308_u19 ();
        break;
      case M_CC_CS_26:
        tim_t323_u26 ();
        break;
    }
  }
  else
  {
    /* 
     * DTMF timer, either T336 or T337
     */
    T_DTMF * p_dtmf;

    cc_data->index_ti  = (UBYTE)(index - MAX_CC_CALLS);
    cc_data->ti        = cc_data->stored_ti_values[cc_data->index_ti];
    p_dtmf = &cc_data->dtmf[cc_data->index_ti];

    switch (p_dtmf->state)
    {
      case DTMF_SEND_REQUEST:
        tim_t336 ();
        break;
      case DTMF_STOP_REQUEST:
        tim_t337 ();
        break;
      default: /* Not expected */
        break;
    }
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t303_u0_1              |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T303 in state U0.1

*/

GLOBAL void tim_t303_u0_1 ()
{
  PALLOC (rej_ind, MNCC_REJECT_IND);

  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_t303_u0_1()");
 
  EM_CC_TIMEOUT_T303;        

  rej_ind->ti  = cc_data->ti;
  rej_ind->cause = CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, M_CC_CAUSE_TIMER);
  PSENDX (MMI, rej_ind);
  for_rel_req ();
  cc_set_state (M_CC_CS_0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t303_u1                |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T303 in state U1

*/

GLOBAL void tim_t303_u1 ()
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_t303_u1()");

  EM_CC_TIMEOUT_T303;  

  cc_data->timer [cc_data->index_ti] = T303;
  cc_disconnect_after_timeout ();
  cc_set_state (M_CC_CS_11);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t305_u11               |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T305 in state U11

*/

GLOBAL void tim_t305_u11 ()
{
  PALLOC (disc_ind, MNCC_DISCONNECT_IND);

  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_t305_u11()");

  cc_data->timer [cc_data->index_ti] = T305;
  CCD_START;
  {
    MCAST (release, U_RELEASE);

    cc_build_release (release, 
                      cc_data->cc_cause[cc_data->index_ti], 
                      NULL, MNCC_SS_VER_NOT_PRES);
    for_release (release);
  }
  CCD_END;

  disc_ind->ti    = cc_data->ti;
  disc_ind->cause = cc_data->cc_cause [cc_data->index_ti];
  /* Setting raw_cause to empty as this is a local release
   * of MM connection,CC is not receiving any cause value
   * from Network
   */
  disc_ind->c_raw_cause = 0;
  disc_ind->diagnostic = NOT_PRESENT_8BIT;
  disc_ind->progress_desc = MNCC_PROG_NOT_PRES;
  PSENDX (MMI, disc_ind);

  TIMERSTART (T308, T308_VALUE);
  cc_set_state (M_CC_CS_19);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t308_u19               |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T308 in state U19

*/

GLOBAL void tim_t308_u19 ()
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_t308_u19()");

  cc_data->timer [cc_data->index_ti] = T308;

  if (cc_data->t308_counter [cc_data->index_ti] EQ 0)
  {
    CCD_START;
    {
      MCAST (release, U_RELEASE);

      cc_data->t308_counter[cc_data->index_ti]++;
      cc_build_release (release,
                        cc_data->cc_cause [cc_data->index_ti], 
                        NULL, MNCC_SS_VER_NOT_PRES);
      for_release (release);
    }
    CCD_END;
    TIMERSTART (T308, T308_VALUE);
  }
  else
  {
    cc_reset_dtmf ();

    {
      PALLOC (rel_cnf, MNCC_RELEASE_CNF);
      rel_cnf->ti    = cc_data->ti;
      rel_cnf->cause = CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, M_CC_CAUSE_TIMER);
      rel_cnf->c_raw_cause = 0;
      PSENDX (MMI, rel_cnf);
    }
    
    for_rel_req ();
    cc_set_state (M_CC_CS_0);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t310_u3                |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T310 in state U3

*/

GLOBAL void tim_t310_u3 ()
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_t310_u3()");

  cc_data->timer [cc_data->index_ti] = T310;
  cc_disconnect_after_timeout ();
  cc_set_state (M_CC_CS_11);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t313_u8                |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T313 in state U8

*/

GLOBAL void tim_t313_u8 ()
{
  GET_INSTANCE_DATA;

  PALLOC (setup_comp, MNCC_SETUP_COMPL_IND);

  TRACE_FUNCTION ("tim_t313_u8()");

  EM_CC_DISCONNECT_SENT;   

  cc_data->timer [cc_data->index_ti] = T313;
  CCD_START;
  {
    MCAST (disconnect, U_DISCONNECT);

    cc_build_disconnect (disconnect, 
                         CAUSE_MAKE(DEFBY_STD, 
                                    ORIGSIDE_MS, 
                                    MNCC_CC_ORIGINATING_ENTITY,
                                    M_CC_CAUSE_TIMER), 
                                    NULL, MNCC_SS_VER_NOT_PRES);
    for_disconnect (disconnect);
  }
  CCD_END;

  setup_comp->ti  = cc_data->ti;
  setup_comp->cause = CAUSE_MAKE(DEFBY_STD, 
                                 ORIGSIDE_MS, 
                                 MNCC_CC_ORIGINATING_ENTITY,
                                 M_CC_CAUSE_TIMER);
  PSENDX (MMI, setup_comp);
  TIMERSTART (T305, T305_VALUE);
  cc_set_state (M_CC_CS_11);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t323_u26               |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T323 in state U26

*/

GLOBAL void tim_t323_u26 ()
{
  GET_INSTANCE_DATA;

  PALLOC (modify_cnf, MNCC_MODIFY_CNF);

  TRACE_FUNCTION ("tim_t323_u26()");

  cc_data->timer [cc_data->index_ti] = T323;

  CCD_START;
  {
    MCAST (disconnect, U_DISCONNECT);

    cc_build_disconnect (disconnect,
                         CAUSE_MAKE(DEFBY_STD, 
                                    ORIGSIDE_MS, 
                                    MNCC_CC_ORIGINATING_ENTITY,
                                    M_CC_CAUSE_TIMER),
                                    NULL, MNCC_SS_VER_NOT_PRES);
    for_disconnect (disconnect);
  }
  CCD_END;

  modify_cnf->ti = cc_data->ti;
  modify_cnf->cause = CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, M_CC_CAUSE_TIMER);
  /* Setting raw_cause to empty as we are not sending any cause from network*/ 
  modify_cnf->c_raw_cause = 0;

  PSENDX (MMI, modify_cnf);

  TIMERSTART (T305, T305_VALUE);
  cc_set_state (M_CC_CS_11);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t332_u0_3              |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T332 in state U0.3

*/

GLOBAL void tim_t332_u0_3 ()
{
  GET_INSTANCE_DATA;
  
  TRACE_FUNCTION ("tim_t332_u0_3()");

  cc_data->timer [cc_data->index_ti] = T332;

  /* Inform MMI */
  /* Implements Measure#  30, 31 */
  tim_mncc_rel_ind ();

  /* Send RELEASE COMPLETE to network */
  CCD_START;
  {
    MCAST (rel_com, U_RELEASE_COMP);
    cc_build_release_complete (rel_com, CAUSE_MAKE(DEFBY_STD, 
                                                   ORIGSIDE_MS, 
                                                   MNCC_CC_ORIGINATING_ENTITY,
                                                   M_CC_CAUSE_TIMER));
    for_release_complete (rel_com);
  }
  CCD_END;

  /* Release MM connection */
  for_rel_req ();

  /* Next state is NULL */
  cc_set_state (M_CC_CS_0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t335_u0_5              |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T335 in state U0.5

*/

GLOBAL void tim_t335_u0_5 ()
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tim_t335_u0_5()");

  cc_data->timer [cc_data->index_ti] = T335;

  /* Forget stored setup container */
  PFREE (cc_data->stored_ccbs_setup);
  cc_data->stored_ccbs_setup = NULL;
  
  /* Inform MMI */
  /* Implements Measure#  30, 31 */
  tim_mncc_rel_ind ();

  /* Send RELEASE COMPLETE to network */
  CCD_START;
  {
    MCAST (rel_com, U_RELEASE_COMP);
    cc_build_release_complete (rel_com, CAUSE_MAKE(DEFBY_STD, 
                                                   ORIGSIDE_MS, 
                                                   MNCC_CC_ORIGINATING_ENTITY,
                                                   M_CC_CAUSE_TIMER));
    for_release_complete (rel_com);
  }
  CCD_END;

  /* Release MM connection */
  for_rel_req ();

  /* Next state is NULL */
  cc_set_state (M_CC_CS_0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t336                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T336

*/

GLOBAL void tim_t336 ()
{
  GET_INSTANCE_DATA;
  
  T_DTMF * p_dtmf;

  TRACE_FUNCTION ("tim_t336()");

  cc_data->timer [cc_data->index_ti] = T336;
  p_dtmf = &cc_data->dtmf[cc_data->index_ti];

  /* 
   * Send error indication to MMI. DTMF transmission maybe has failed.
   * This point may be discussed, but TS 24.008 doesn't say that a
   * positive indication as a feedback for a successful transmission
   * should be generated.
   */
  {
    PALLOC (dtmf_cnf, MNCC_START_DTMF_CNF);
    dtmf_cnf->ti       = cc_data->ti;
    dtmf_cnf->key      = p_dtmf->key;
    dtmf_cnf->cause    = CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, M_CC_CAUSE_TIMER);
    dtmf_cnf->dtmf_mod = p_dtmf->mode;
    PSENDX (MMI, dtmf_cnf);
  }

  cc_stop_dtmf ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_t337                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer T337

*/

GLOBAL void tim_t337 ()
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_t337()");
  
  cc_data->timer [cc_data->index_ti] = T337;

  cc_stop_dtmf ();
}


/* Implements Measure#  30, 31 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_TIM                     |
| STATE   : code                ROUTINE : tim_mncc_rel_ind                   |
+--------------------------------------------------------------------+

  PURPOSE : Build and send MNCC_RELEASE_IND

*/

LOCAL void tim_mncc_rel_ind (void)
{ 
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_mncc_rel_ind");
  { 
    PALLOC (release, MNCC_RELEASE_IND); 
    release->ti    = cc_data->ti;
    release->cause = CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_CC_ORIGINATING_ENTITY, M_CC_CAUSE_TIMER);
    release->c_raw_cause = 0;
    PSENDX (MMI, release);
  }
}

#endif /* #ifndef CC_TIM_C */
