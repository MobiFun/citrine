/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DYN_DWL_SYNC.C
 *
 *        Filename l1_dyn_dwl_sync.c
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#include <stdio.h>
#include <string.h>

#include "nucleus.h"
#include "l1_confg.h"
#include "l1_types.h"
#include "sys_types.h"
#include "cust_os.h"
#include "l1audio_signa.h"
#include "l1audio_const.h"
#include "l1audio_cust.h"
#include "l1audio_defty.h"
#include "l1_const.h"
#include "l1tm_defty.h"
#if (L1_GTT == 1)
  #include "l1gtt_const.h"
  #include "l1gtt_defty.h"
#endif
#if (L1_DYN_DSP_DWNLD==1)
  #include "l1_dyn_dwl_defty.h"
  #include "l1_dyn_dwl_msgty.h"
  #include "l1_dyn_dwl_const.h"
  #include "l1_dyn_dwl_signa.h"
  #include "l1_dyn_dwl_error.h"
  #include "l1_dyn_dwl_proto.h"
  #include "l1_api_hisr.h"
#endif

#if (L1_MP3 == 1)
  #include "l1mp3_defty.h"
#endif //L1_MP3
#if (L1_MIDI == 1)
  #include "l1midi_defty.h"
#endif

#include "l1_macro.h"
#include "l1_defty.h"
#include "l1_varex.h"
#include "l1_trace.h"
#include "sys_dma.h"

#if (L1_DYN_DSP_DWNLD == 1)

extern T_DYN_DWNLD_MCU_DSP *dyn_dwl_ndb;


/*-------------------------------------------------------*/
/* l1s_dyn_dsp_dwnld_manager()                           */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters :  n/a                                     */
/*                                                       */
/* Return     :  n/a                                     */
/*                                                       */
/* Description : dsp dynamic download L1S manager task.  */
/*                                                       */
/*-------------------------------------------------------*/

void l1s_dyn_dwnld_manager(void)
{
  enum states
  {
    WAIT_INIT = 0,
    WAIT_RESULT,
    WAIT_STOP
  };

  UWORD8 *state = &l1s.dyn_dwnld_state;
  xSignalHeaderRec *conf_msg;
  
  switch(*state)
  {
    case WAIT_INIT:
    {
      if(l1a_l1s_com.dyn_dwnld_task.start==TRUE)
      {
        // make sure the DSP is able to receive API interrupt
        // for this, we wait 1 TDMA frame
        // change state
        *state=WAIT_RESULT;
      }
    }
    break;

    case WAIT_RESULT:
    {
      // reset the command
      l1a_l1s_com.dyn_dwnld_task.start=FALSE;
      // send start background to the DSP
      dyn_dwl_ndb->d_api_dwl_download_ctrl = (API) C_DWL_DOWNLOAD_CTRL_INIT;
      l1s_dsp_com.dsp_ndb_ptr->a_background_tasks[C_BGD_DSP_DYN_DWNLD] = (API)((C_BGD_DSP_DYN_DWNLD<<11) | 1);
      l1s_dsp_com.dsp_ndb_ptr->d_background_enable|=(API)(1<<C_BGD_DSP_DYN_DWNLD);
      l1_trigger_api_interrupt();

      // change state
      *state=WAIT_STOP;
    }
    break;

    case WAIT_STOP:
    {
      if(l1a_l1s_com.dyn_dwnld_task.stop==TRUE)
      {
        // reset the command
        l1a_l1s_com.dyn_dwnld_task.stop=FALSE;

        // send stop background to the DSP
        l1s_dsp_com.dsp_ndb_ptr->a_background_tasks[C_BGD_DSP_DYN_DWNLD] = (API)((C_BGD_DSP_DYN_DWNLD<<11) | 0);
        l1s_dsp_com.dsp_ndb_ptr->d_background_enable&=(API)(~(1<<C_BGD_DSP_DYN_DWNLD));
        l1_trigger_api_interrupt(); 
        
        // send confirmation to the L1A
        conf_msg=os_alloc_sig(0);
        DEBUGMSG(status,NU_ALLOC_ERR)
        conf_msg->SignalCode=L1_DYN_DWNLD_STOP_CON;
        os_send_sig(conf_msg,L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)

        // change state
        *state=WAIT_INIT;
      }
    }
    break;
  }
}

#endif    // L1_DYN_DSP_DWNLD==1

