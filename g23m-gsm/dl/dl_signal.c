/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  dl_signal
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
|  Purpose :  This Modul defines all functions which invoke signals or are
|             called by signal.
+-----------------------------------------------------------------------------
*/

#ifndef DL_SIGNAL_C
#define DL_SIGNAL_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_DL

/*==== INCLUDES ===================================================*/

#include "typedefs.h"
#include <string.h>
#include "pconst.cdg"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "mon_dl.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "ccdapi.h"
#include "dl.h"
#include "dl_trc.h"

#if defined(INVOKE_SIGNAL)
/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
/*
+------------------------------------------------------------------------------
| Function    : sig_get_signal_data
+------------------------------------------------------------------------------
| Description : Get the signal data back depend on parameter write
+------------------------------------------------------------------------------
*/
GLOBAL void sig_init_signal_data (void)
{
  GET_INSTANCE_DATA;
  memset (&dl_data->signal_struct, 0, sizeof (T_DL_SIGNAL));
}

/*
+------------------------------------------------------------------------------
| Function    : sig_get_signal_data
+------------------------------------------------------------------------------
| Description : Get the signal data back depend on parameter write
+------------------------------------------------------------------------------
*/
LOCAL T_DL_SIGNAL_DATA * sig_get_signal_data (void)
{
  GET_INSTANCE_DATA;
  T_DL_SIGNAL      *sig = &dl_data->signal_struct;
  T_DL_SIGNAL_DATA *sd = &sig->sig_data[sig->sig_idx];

  /*TRACE_EVENT_WIN_P2 ("sig_get_signal_data(): idx=%u %s", sig->sig_idx, sd->busy ? "BUSY": "use it");*/

  /* first approach without semaphore */
  if (sd->busy EQ FALSE)
  {
    sd->busy = TRUE;
    #if defined(_SIMULATION_)
      sd->idx = sig->sig_idx;
    #endif  /* _SIMULATION_ */
    if (++sig->sig_idx EQ DL_SIGNAL_DATA_ELEMENTS)
      sig->sig_idx = 0;/* index of ring buffer */
  }
  else
  {
    TRACE_ERROR ("dl_signal_data BUSY !!!");
    TRACE_ASSERT (!sd->busy);
  }
  return sd;
}

#if 0 /* only for debug purposes */
GLOBAL int sig_get_busy_signals_counter (T_DL_DATA_STORE * dl_data)
{
  T_DL_SIGNAL      *sig = &dl_data->signal_struct;
  T_DL_SIGNAL_DATA *sd = &sig->sig_data[0];
  int n, busy;

  for (n=busy=0; n < DL_SIGNAL_DATA_ELEMENTS; n++, sd++)
  {
    if (sd->busy EQ TRUE)
      busy++;
  }
  return busy;
}
#endif /* 0|1 */

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_drr_dl_establish_ind
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_ESTABLISH_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_drr_dl_establish_ind (UBYTE ch_type,
                                             UBYTE sapi, UBYTE indication)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_drr_dl_establish_ind()");
  signal->u.establish_ind.ch_type = ch_type;
  signal->u.establish_ind.sapi = sapi;
  signal->u.establish_ind.indication = indication;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_ESTABLISH_IND, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_ESTABLISH_IND, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_drr_dl_establish_ind
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_ESTABLISH_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_drr_dl_establish_ind (T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_drr_dl_establish_ind()");
  drr_dl_establish_ind (signal->u.establish_ind.ch_type,
                signal->u.establish_ind.sapi, signal->u.establish_ind.indication);
}

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_drr_dl_establish_cnf
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_ESTABLISH_CNF
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_drr_dl_establish_cnf (UBYTE ch_type, UBYTE sapi)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_drr_dl_establish_cnf()");
  signal->u.establish_cnf.ch_type = ch_type;
  signal->u.establish_cnf.sapi = sapi;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_ESTABLISH_CNF, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_ESTABLISH_CNF, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_drr_dl_establish_cnf
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_ESTABLISH_CNF
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_drr_dl_establish_cnf (T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_drr_dl_establish_cnf()");
  drr_dl_establish_cnf (signal->u.establish_cnf.ch_type,
    signal->u.establish_cnf.sapi);
}

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_drr_dl_data_cnf
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_DATA_CNF
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_drr_dl_data_cnf (UBYTE sapi)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_drr_dl_data_cnf()");
  signal->u.data_cnf.sapi = sapi;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_DATA_CNF, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_DATA_CNF, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_drr_dl_data_cnf
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_DATA_CNF
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_drr_dl_data_cnf (T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_drr_dl_data_cnf()");
  drr_dl_data_cnf (signal->u.data_cnf.sapi);
}

#if 0 /* happens in primitive context only */
/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_drr_dl_unitdata_ind
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_UNITDATA_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_drr_dl_unitdata_ind (UBYTE error_flag, UBYTE * layer1head, 
                                            UBYTE * layer3msg,UBYTE length, 
                                            ULONG fn)
{
  GET_INSTANCE_DATA;
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_drr_dl_unitdata_ind()");
  signal->u.unitdata_ind.error_flag = error_flag;
#ifdef TI_PS_16BIT_CPY
  api_memcpy(signal->u.unitdata_ind.layer1head,  layer1head,
    sizeof(signal->u.unitdata_ind.layer1head));
  api_memcpy(signal->u.unitdata_ind.layer3msg,  layer3msg, length);
#else
  memcpy(signal->u.unitdata_ind.layer1head,  layer1head,
    sizeof(signal->u.unitdata_ind.layer1head));
  memcpy(signal->u.unitdata_ind.layer3msg,  layer3msg, length);
#endif
  signal->u.unitdata_ind.length = length;
  signal->u.unitdata_ind.fn = fn;
  PSIGNAL(hCommDL, DL_SIGNAL_UNITDATA_IND, signal);
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_drr_dl_unitdata_ind
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_UNITDATA_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_drr_dl_unitdata_ind (T_DL_DATA_STORE *dl_data,
                                     T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_drr_dl_unitdata_ind()");
  drr_dl_unitdata_ind (dl_data,
    signal->u.unitdata_ind.error_flag,
    signal->u.unitdata_ind.layer1head,
    signal->u.unitdata_ind.layer3msg,
    signal->u.unitdata_ind.length,
    signal->u.unitdata_ind.fn);
}
#endif /* 0 */

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_drr_dl_short_unitdata_ind
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_SHORT_UNITDATA_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_drr_dl_short_unitdata_ind (UBYTE ch_type,
                                    UBYTE error_flag, UBYTE * layer1head,
                                    UBYTE * layer3msg, UBYTE length, ULONG fn)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();	  
  TRACE_EVENT_WIN ("sig_invoke_drr_dl_short_unitdata_ind()");
  signal->u.short_unitdata_ind.ch_type = ch_type;
  signal->u.short_unitdata_ind.error_flag = error_flag;
#ifdef TI_PS_16BIT_CPY
  api_memcpy(signal->u.short_unitdata_ind.layer1head,  layer1head,
    sizeof(signal->u.short_unitdata_ind.layer1head));
  api_memcpy(signal->u.short_unitdata_ind.layer3msg,  layer3msg, length);
#else
  memcpy(signal->u.short_unitdata_ind.layer1head,  layer1head,
    sizeof(signal->u.short_unitdata_ind.layer1head));
  memcpy(signal->u.short_unitdata_ind.layer3msg,  layer3msg, length);
#endif
  signal->u.short_unitdata_ind.length = length;
  signal->u.short_unitdata_ind.fn = fn;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_SHORT_UNITDATA_IND, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_SHORT_UNITDATA_IND, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_drr_dl_short_unitdata_ind
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_SHORT_UNITDATA_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_drr_dl_short_unitdata_ind (
                                         T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_drr_dl_short_unitdata_ind()");
  drr_dl_short_unitdata_ind (
    signal->u.short_unitdata_ind.ch_type,
    signal->u.short_unitdata_ind.error_flag,
    signal->u.short_unitdata_ind.layer1head,
    signal->u.short_unitdata_ind.layer3msg,
    signal->u.short_unitdata_ind.length,
    signal->u.short_unitdata_ind.fn);
}

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_drr_dl_release_ind
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_RELEASE_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_drr_dl_release_ind (UBYTE ch_type, UBYTE sapi, 
                                           UBYTE cs, BOOL init)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_drr_dl_release_ind()");
  signal->u.release_ind.ch_type = ch_type;
  signal->u.release_ind.sapi = sapi;
  signal->u.release_ind.cs = cs;
  signal->u.release_ind.init = init;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_RELEASE_IND, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_RELEASE_IND, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_drr_dl_release_ind
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_RELEASE_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_drr_dl_release_ind (T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_drr_dl_release_ind()");
  drr_dl_release_ind (signal->u.release_ind.ch_type,
    signal->u.release_ind.sapi, signal->u.release_ind.cs,
    signal->u.release_ind.init);

}

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_drr_dl_release_cnf
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_RELEASE_CNF
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_drr_dl_release_cnf (UBYTE ch_type,
                                           UBYTE sapi, BOOL init)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_drr_dl_release_cnf()");
  signal->u.release_cnf.ch_type = ch_type;
  signal->u.release_cnf.sapi = sapi;
  signal->u.release_cnf.init = init;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_RELEASE_CNF, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_RELEASE_CNF, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_drr_dl_release_cnf
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_RELEASE_CNF
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_drr_dl_release_cnf ( T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_drr_dl_release_cnf()");
  drr_dl_release_cnf (signal->u.release_cnf.ch_type,
    signal->u.release_cnf.sapi, signal->u.release_cnf.init);
}

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_drr_error_ind
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_ERROR_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_drr_error_ind (UBYTE ch_type, UBYTE sapi)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_drr_error_ind()");
  signal->u.error_ind.ch_type = ch_type;
  signal->u.error_ind.sapi = sapi;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_ERROR_IND, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_ERROR_IND, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_drr_error_ind
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_ERROR_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_drr_error_ind (T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_drr_error_ind()");
  drr_error_ind (signal->u.error_ind.ch_type,
    signal->u.error_ind.sapi);
}  

#if defined(FF_EM_MODE)
/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_dl_em_write
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_EM_WRITE
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_dl_em_write (UBYTE length, UBYTE * data)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_dl_em_write()");
  signal->u.em_write.length = length;
#ifdef TI_PS_16BIT_CPY
  api_memcpy(signal->u.em_write.data,  data, length);
#else
  memcpy(signal->u.em_write.data,  data, length);
#endif
  PSIGNAL(hCommDL, DL_SIGNAL_EM_WRITE, signal);
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_dl_em_write
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_EM_WRITE
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_dl_em_write (T_DL_DATA_STORE *dl_data,
                                    T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_dl_em_write()");
  em_dl_write (signal->u.em_write.length, signal->u.em_write.data);
}

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_dl_em_first_event_check
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_EM_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_dl_em_first_event_check (void)
{
  TRACE_EVENT_WIN ("sig_invoke_dl_em_first_event_check()");
  PSIGNAL(hCommDL, DL_SIGNAL_EM_IND, NULL);
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_dl_em_first_event_check
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_EM_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_dl_em_first_event_check ()
{
  TRACE_EVENT_WIN ("sig_handle_dl_em_first_event_check()");
  dl_em_first_event_check();
}
#endif  /* FF_EM_MODE */

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_com_concatenate
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_DATA_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_com_concatenate ( T_DL_DATA_IND ** in_msg, 
                                         UBYTE * new_data_in)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_com_concatenate()");
  signal->u.concatenate.in_msg = in_msg;
#ifdef TI_PS_16BIT_CPY
  api_memcpy (signal->u.concatenate.new_data_in, new_data_in, MAX_L2_FRAME_SIZE);
#else
  memcpy (signal->u.concatenate.new_data_in, new_data_in, MAX_L2_FRAME_SIZE);
#endif
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_CONCATENATE, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_CONCATENATE, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_com_concatenate
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_CONCATENATE
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_com_concatenate (T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_com_concatenate()");
  com_concatenate (signal->u.concatenate.in_msg,
                            signal->u.concatenate.new_data_in);
}

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_com_free_pointer
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_FREE_POINTER
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_com_free_pointer (void *pointer)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_com_free_pointer()");
  signal->u.free_pointer.pointer = pointer;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_FREE_POINTER, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_FREE_POINTER, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_drr_error_ind
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_ERROR_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_com_free_pointer(T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_com_free_pointer()");
  COM_FREE_POINTER (signal->u.free_pointer.pointer);
  signal->u.free_pointer.pointer = NULL;
}

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_com_data_ind
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_CONCATENATE
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_com_data_ind (UBYTE ch_type,
                                     UBYTE sapi, ULONG fn)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_com_data_ind()");
  signal->u.data_ind.ch_type = ch_type;
  signal->u.data_ind.sapi = sapi;
  signal->u.data_ind.fn = fn;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_DATA_IND, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_DATA_IND, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_com_data_ind
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_DATA_IND
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_com_data_ind (T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_com_data_ind()");
  com_data_ind(signal->u.data_ind.ch_type, signal->u.data_ind.sapi,
                        signal->u.data_ind.fn);
}

/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_com_l3trace
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_L3TRACE
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_com_l3trace (UBYTE type, UBYTE ch_type, UBYTE *frame)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_com_l3_trace()");
  signal->u.l3trace.type = type;
  signal->u.l3trace.ch_type = ch_type;
  /*
   * Attention: The down- or uplinked message will not copy. The pointer
   * of the frame is stored only. Please be aware that the frame have to trace
   * before it will free.
   */
  signal->u.l3trace.frame = frame;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_L3TRACE, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_L3TRACE, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_com_l3trace
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_L3TRACE
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_com_l3trace( T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_com_l3trace()");
  com_l3trace (signal->u.l3trace.type, signal->u.l3trace.ch_type, signal->u.l3trace.frame);
}

#if defined(DL_TRACE_ENABLED) && defined(DL_IMMEDIATE_TRACE)
/*
+------------------------------------------------------------------------------
| Function    : sig_invoke_com_l2trace
+------------------------------------------------------------------------------
| Description : Invokes the signal DL_SIGNAL_L2TRACE
+------------------------------------------------------------------------------
*/
GLOBAL void sig_invoke_com_l2trace (UBYTE trace_type,
                               UBYTE channel, UBYTE ch_type, T_TIME trace_time,
                               UBYTE* data)
{
  T_DL_SIGNAL_DATA * signal = sig_get_signal_data ();
  TRACE_EVENT_WIN ("sig_invoke_com_l2trace()");
  signal->u.l2trace.trace_type = trace_type;
  signal->u.l2trace.channel = channel;
  signal->u.l2trace.ch_type = ch_type;
  signal->u.l2trace.trace_time = trace_time;
  if (data)
  {
    signal->u.l2trace.data_len = MAX_L2_FRAME_SIZE;
    /*
     * lint Warning 420: Apparent access beyond array ...
     * The argument 2 (data) may be smaller than MAX_L2_FRAME_SIZE,
     * but it makes no sense to detect the kind and length of the data first
     * (frame of SACCH or DCCH, string -> strlen) to avoid a read beyond array.
     */
    memcpy (signal->u.l2trace.data, data, MAX_L2_FRAME_SIZE);
  }
  else
    signal->u.l2trace.data_len = 0;
#ifdef TI_PS_HCOMM_CHANGE
  PSIGNAL(_hCommDL, DL_SIGNAL_L2TRACE, signal);
#else
  PSIGNAL(hCommDL, DL_SIGNAL_L2TRACE, signal);
#endif
}

/*
+------------------------------------------------------------------------------
| Function    : sig_handle_com_l2trace
+------------------------------------------------------------------------------
| Description : Handles the signal DL_SIGNAL_L2TRACE
+------------------------------------------------------------------------------
*/
GLOBAL void sig_handle_com_l2trace(T_DL_SIGNAL_DATA * signal)
{
  TRACE_EVENT_WIN ("sig_handle_com_l2trace()");
  com_l2trace (signal->u.l2trace.trace_type, signal->u.l2trace.channel,
    signal->u.l2trace.ch_type,
    signal->u.l2trace.trace_time,
    signal->u.l2trace.data_len ? signal->u.l2trace.data : NULL);
}
#endif  /* DL_TRACE_ENABLED && DL_IMMEDIATE_TRACE */

#endif  /* INVOKE_SIGNAL */
#endif /* DL_SIGNAL_C */
