/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_AFUNC.C
 *
 *        Filename l1audio_afunc.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

/************************************/
/* Include files...                 */
/************************************/

#include "l1_macro.h"
#include "l1_confg.h"

#if (AUDIO_TASK == 1)

  #include "l1_types.h"
  #include "sys_types.h"

  #if (CODE_VERSION == SIMULATION) && (AUDIO_SIMULATION)


    #include <stdlib.h>
    #include <string.h>

    #include "iq.h"             // Debug / Init hardware  ("eva3.lib")
    #include "l1_ver.h"
    #include "l1_const.h"
    #include "l1_signa.h"

    #if TESTMODE
      #include "l1tm_defty.h"
    #endif

    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_signa.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif
//added from e-sample for AAC
    #if (L1_DYN_DSP_DWNLD == 1)
      #include "l1_dyn_dwl_const.h"
      #include "l1_dyn_dwl_defty.h"
    #endif
    #if (L1_MP3 == 1)
      #include "l1mp3_defty.h"
    #endif

    #if (L1_MIDI == 1)
      #include "l1midi_defty.h"
    #endif
//added from e-sample for AAC
    #if (L1_AAC == 1)
      #include "l1aac_defty.h"
    #endif

    #include "l1_defty.h"
    #include "cust_os.h"
    #include "l1_msgty.h"
    #include "l1_varex.h"

    #include "l1_mftab.h"
    #include "l1_tabs.h"
    #include "l1_ctl.h"

    #include "l1_time.h"
    #include "l1_scen.h"

    #include "l1audio_proto.h"

  #else
  // Layer1 and debug include files.

    #include <ctype.h>
    #include <math.h>
    #include "l1_ver.h"
    #include "l1_const.h"
    #include "l1_signa.h"
    #include "cust_os.h"

    #if TESTMODE
      #include "l1tm_defty.h"
    #endif

    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_signa.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"
    #include "l1audio_proto.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif
//added from e-sample for AAC
    #if (L1_DYN_DSP_DWNLD == 1)
      #include "l1_dyn_dwl_const.h"
      #include "l1_dyn_dwl_defty.h"
    #endif
    #if (L1_MP3 == 1)
      #include "l1mp3_defty.h"
    #endif

    #if (L1_MIDI == 1)
      #include "l1midi_defty.h"
    #endif
//added from e-smaple for AAC
    #if (L1_AAC == 1)
      #include "l1aac_defty.h"
    #endif
#if (RF_FAM == 61)
#include "l1_rf61.h"
#endif 

    #include "l1_defty.h"
    #include "l1_msgty.h"
    #include "tpudrv.h"       // TPU drivers.           ("eva3.lib")
    #include "l1_varex.h"

    #include "l1_proto.h"
    #include "l1_mftab.h"
    #include "l1_tabs.h"
    #include "mem.h"
    #include "armio.h"
    #include "timer.h"
    #include "timer1.h"
    #include "dma.h"
    #include "inth.h"
    #include "ulpd.h"
    #include "rhea_arm.h"
    #include "clkm.h"         // Clockm  ("eva3.lib")
    #include "l1_ctl.h"

    #include "l1_time.h"
    #if L2_L3_SIMUL
      #include "l1_scen.h"
    #endif

    #if (OP_RIV_AUDIO == 1)
      #include "rv_general.h"
      #include "audio_api.h"
      #include "audio_structs_i.h"
      #include "audio_var_i.h"
      #include "audio_macro_i.h"
      #include "audio_const_i.h"
    #endif
 #endif
//      #include "audio_api.h"  //Thukaram Test
#if (OP_L1_STANDALONE == 0)
   
      #include "typedefs.h"
      #include "vsi.h"
      #include "os_const.h"
      #include "os.h"
      #include "frm_glob.h"

      #include "typedefs.h"
      #include "vsi.h"
      #include "os_const.h"
      #include "os.h"
      #include "frm_glob.h"

   extern T_HANDLE mm_ext_data_pool_handle;
   extern T_HANDLE mm_int_data_pool_handle; 
 #endif
   UWORD8 dma_src_port;
  // This macro avoids to duplicate the code.
  #if (OP_RIV_AUDIO == 1)
    #define AUDIO_MSG (p_message)
  #else
    #define AUDIO_MSG (msg->SigP)
  #endif

  /*-------------------------------------------------------*/
  /* l1a_audio_send_confirmation()                         */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters : Signal Code ofthe message                */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description :  Send a confirmation message to the MMI */
  /*                for an audio task.                     */
  /*                                                       */
  /*-------------------------------------------------------*/
  void l1a_audio_send_confirmation(UWORD32 SignalCode)
  {
  #if (OP_RIV_AUDIO == 1)

    void *p_message;
    T_RVF_MB_STATUS mb_status; //omaps00090550

    mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                              sizeof (T_RV_HDR),
                              (T_RVF_BUFFER **) (&p_message));

     /* If insufficient resources, then report a memory error and abort.               */
     if (mb_status == RVF_RED)
     {
       /* the memory is insufficient to continue the non regression test */
       AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
       return;
     }

    /* fill the message id */
    ((T_RV_HDR *)p_message)->msg_id    = SignalCode;

    /* send the messsage to the audio entity */
    rvf_send_msg (p_audio_gbl_var->addrId,
                  p_message);

  #else // OP_RIV_AUDIO

    xSignalHeaderRec *msg;

    msg = os_alloc_sig(0);
    DEBUGMSG(status,NU_ALLOC_ERR)
    msg->SignalCode = SignalCode;

    #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
      l1_trace_message(msg);
    #endif

    os_send_sig(msg, MMI_QUEUE);
    DEBUGMSG(status,NU_SEND_QUEUE_ERR)

  #endif // OP_RIV_AUDIO
  }

/*-------------------------------------------------------*/
/* l1a_audio_send_result()                               */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : Send a result message to the MMI.    */
/*-------------------------------------------------------*/
void l1a_audio_send_result(UWORD32 SignalCode, xSignalHeaderRec *msg, UWORD8 queue)
{
#if (OP_RIV_AUDIO == 1)

  void *p_message = NULL; //omaps00090550
  T_RVF_MB_STATUS mb_status; //omaps00090550

    switch (SignalCode)
    {
    #if (L1_AUDIO_DRIVER == 1)
      case AUDIO_DRIVER_NOTIFICATION_MSG:
      {
        T_AUDIO_DRIVER_SESSION *p_session;
        T_RV_RETURN *return_path;
        UINT8 channel_id;

        /* get driver session */
        channel_id = ((T_L1_AUDIO_DRIVER_IND *)msg->SigP)->channel_id;
        p_session = &(p_audio_gbl_var->audio_driver_session[channel_id]);
        return_path = &(p_session->session_req.return_path);

        /* Translate the l1 message into the Riviera compliant message */
        /* Allocate the Riviera buffer */
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_AUDIO_DRIVER_NOTIFICATION),
                                  (T_RVF_BUFFER **) (&p_message));
        /* If insufficient resources, then report a memory error and abort. */
        if (mb_status == RVF_RED)
        {
          /* the memory is insufficient to continue the non regression test */
          AUDIO_SEND_TRACE("AUDIO entity has no memory for driver notification",RV_TRACE_LEVEL_ERROR);
          return;
        }

        /* Fill the message ID + parameters */
        ((T_AUDIO_DRIVER_NOTIFICATION *)p_message)->header.msg_id = SignalCode;
        ((T_AUDIO_DRIVER_NOTIFICATION *)p_message)->channel_id = channel_id;
        ((T_AUDIO_DRIVER_NOTIFICATION *)p_message)->p_buffer = ((T_L1_AUDIO_DRIVER_IND *)(msg->SigP))->p_buffer;

        /* send answer */
        if (return_path->callback_func == NULL)
          rvf_send_msg (return_path->addr_id, p_message);
        else
        {
          (*return_path->callback_func)((void *)(p_message));
          rvf_free_buf((T_RVF_BUFFER *)p_message);
        }

        // This return is important because there is another rvf_send_msg at the end of the switch
        return;
      }
//omaps00090550      break;
    #endif
    #if (L1_VOICE_MEMO_AMR)
      case MMI_VM_AMR_RECORD_STOP_CON:
      {
        /* Translate the l1 message into the Riviera compliant message */
        /* Allocate the Riviera buffer */
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_VM_AMR_RECORD_CON),
                                  (T_RVF_BUFFER **) (&p_message));

        /* If insufficient resources, then report a memory error and abort. */
        if (mb_status == RVF_RED)
        {
          /* the memory is insufficient to continue the non regression test */
          AUDIO_SEND_TRACE("AUDIO entity has no memory for MMI_VM_AMR_RECORD_STOP_CON",RV_TRACE_LEVEL_ERROR);
          return;
        }

        // recorded size includes the end mask
        ((T_MMI_VM_AMR_RECORD_CON *)p_message)->recorded_size = ((T_L1_VM_AMR_RECORD_CON *)(msg->SigP))->recorded_size;
      }
      break;
    #endif
      #if (VOICE_MEMO)
        case MMI_VM_RECORD_STOP_CON:
        {
          /* Translate the l1 message into the Riviera compliant message */

          /* Allocate the Riviera buffer */
          mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_VM_RECORD_CON),
                                  (T_RVF_BUFFER **) (&p_message));

          /* If insufficient resources, then report a memory error and abort. */
          if (mb_status == RVF_RED)
          {
            /* the memory is insufficient to continue the non regression test */
            AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
            return;
          }

          ((T_MMI_VM_RECORD_CON *)p_message)->recorded_size =
            ((T_L1_VM_RECORD_CON *)(msg->SigP))->recorded_size;

          break;
        }
      #endif
        #if (L1_PCM_EXTRACTION)
	          case MMI_PCM_UPLOAD_STOP_CON:
	          {
	            /* Translate the l1 message into the Riviera compliant message */

	            /* Allocate the Riviera buffer */
          mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
	                                    sizeof (T_MMI_PCM_UPLOAD_STOP_CON),
	                                    (T_RVF_BUFFER **) (&p_message));

	            /* If insufficient resources, then report a memory error and abort. */
	            if (mb_status == RVF_RED)
	            {
	              /* the memory is insufficient to continue the non regression test */
	              AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
	              return;
	            }

	            ((T_MMI_PCM_UPLOAD_STOP_CON *)p_message)->uploaded_size =
	              ((T_L1_PCM_UPLOAD_STOP_CON *)(msg->SigP))->uploaded_size;

	            break;
	          }
	        #endif    /* L1_PCM_EXTRACTION */

      #if (SPEECH_RECO)
        case MMI_SR_ENROLL_STOP_CON:
        {
          /* Translate the l1 message into the Riviera compliant message */

          /* Allocate the Riviera buffer */
          mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_SR_ENROLL_STOP_CON),
                                  (T_RVF_BUFFER **) (&p_message));

          /* If insufficient resources, then report a memory error and abort. */
          if (mb_status == RVF_RED)
          {
            /* the memory is insufficient to continue the non regression test */
            AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
            return;
          }

          /* Fill the message parameter */
          ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->error_id =
            ((T_L1_SR_ENROLL_STOP_CON *)(msg->SigP))->error_id;

          break;
        }
        case MMI_SR_UPDATE_STOP_CON:
        {
          /* Translate the l1 message into the Riviera compliant message */

          /* Allocate the Riviera buffer */
          mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_SR_UPDATE_STOP_CON),
                                  (T_RVF_BUFFER **) (&p_message));

          /* If insufficient resources, then report a memory error and abort. */
          if (mb_status == RVF_RED)
          {
            /* the memory is insufficient to continue the non regression test */
            AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
            return;
          }

          /* Fill the message parameter */
          ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->error_id =
            ((T_L1_SR_UPDATE_STOP_CON *)(msg->SigP))->error_id;

          break;
        }
        case MMI_SR_RECO_STOP_CON:
        {
          /* Translate the l1 message into the Riviera compliant message */

          /* Allocate the Riviera buffer */
          mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_SR_RECO_STOP_CON),
                                  (T_RVF_BUFFER **) (&p_message));

          /* If insufficient resources, then report a memory error and abort. */
          if (mb_status == RVF_RED)
          {
            /* the memory is insufficient to continue the non regression test */
            AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
            return;
          }

          /* Fill the message parameter */
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->error_id                 =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->best_word_index          =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->best_word_score          =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_score;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->second_best_word_index   =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->second_best_word_index;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->second_best_word_score   =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->second_best_word_score;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->third_best_word_index    =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->third_best_word_index;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->third_best_word_score    =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->third_best_word_score;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->fourth_best_word_index   =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->fourth_best_word_index;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->fourth_best_word_score   =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->fourth_best_word_score;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->d_sr_model_size          =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->d_sr_model_size;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->d_sr_db_level            =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->d_sr_db_level;
          ((T_MMI_SR_RECO_STOP_CON *)(p_message))->d_sr_db_noise            =
            ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->d_sr_db_noise;

          break;
        }

        case MMI_SR_UPDATE_CHECK_STOP_CON:
        {
          /* Translate the l1 message into the Riviera compliant message */

          /* Allocate the Riviera buffer */
          mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_SR_UPDATE_CHECK_STOP_CON),
                                  (T_RVF_BUFFER **) (&p_message));

          /* If insufficient resources, then report a memory error and abort. */
          if (mb_status == RVF_RED)
          {
            /* the memory is insufficient to continue the non regression test */
            AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
            return;
          }

          /* Fill the message parameter */
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->error_id                 =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->error_id;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->best_word_index          =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->best_word_index;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->best_word_score          =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->best_word_score;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->second_best_word_index   =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->second_best_word_index;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->second_best_word_score   =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->second_best_word_score;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->third_best_word_index    =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->third_best_word_index;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->third_best_word_score    =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->third_best_word_score;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->fourth_best_word_index   =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->fourth_best_word_index;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->fourth_best_word_score   =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->fourth_best_word_score;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->d_sr_model_size          =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_model_size;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->d_sr_db_level            =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_db_level;
          ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->d_sr_db_noise            =
            ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_db_noise;

          break;
        }

      #endif /* SPEECH_RECO */


    #if (L1_CPORT == 1)
      case MMI_CPORT_CONFIGURE_CON:
      {
        /* Translate the l1 message into the Riviera compliant message */
        /* Allocate the Riviera buffer */
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_CPORT_CONFIGURE_CON),
                                  (T_RVF_BUFFER **) (&p_message));

        /* If insufficient resources, then report a memory error and abort. */
        if (mb_status == RVF_RED)
        {
          /* the memory is insufficient to continue the non regression test */
          AUDIO_SEND_TRACE("AUDIO entity has no memory for MMI_CPORT_CONFIGURE_CON",RV_TRACE_LEVEL_ERROR);
          return;
        }

        // recorded size includes the end mask
        ((T_MMI_CPORT_CONFIGURE_CON *)p_message)->register_id    = ((T_L1_CPORT_CONFIGURE_CON *)(msg->SigP))->register_id;
        ((T_MMI_CPORT_CONFIGURE_CON *)p_message)->register_value = ((T_L1_CPORT_CONFIGURE_CON *)(msg->SigP))->register_value;
      }
      break;
    #endif

    #if(L1_ANR == 2)
      case MMI_AQI_ANR_CON:
      {
        /* Translate the l1 message into the Riviera compliant message */
        /* Allocate the Riviera buffer */
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_AQI_ANR_CON),
                                  (T_RVF_BUFFER **) (&p_message));

        /* If insufficient resources, then report a memory error and abort. */
        if (mb_status == RVF_RED)
        {
          /* the memory is insufficient to continue the non regression test */
          AUDIO_SEND_TRACE("AUDIO entity has no memory for MMI_AQI_ANR_CON",RV_TRACE_LEVEL_ERROR);
          return;
        }

        // recorded size includes the end mask
        ((T_MMI_AQI_ANR_CON *)p_message)->anr_ul_action = ((T_L1_AQI_ANR_CON *)(msg->SigP))->anr_ul_action;
      }
      break;
    #endif

    #if(L1_IIR == 2)
      case MMI_AQI_IIR_DL_CON:
      {
        /* Translate the l1 message into the Riviera compliant message */
        /* Allocate the Riviera buffer */
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_AQI_IIR_DL_CON),
                                  (T_RVF_BUFFER **) (&p_message));

        /* If insufficient resources, then report a memory error and abort. */
        if (mb_status == RVF_RED)
        {
          /* the memory is insufficient to continue the non regression test */
          AUDIO_SEND_TRACE("AUDIO entity has no memory for MMI_AQI_IIR_DL_CON",RV_TRACE_LEVEL_ERROR);
          return;
        }

        // recorded size includes the end mask
        ((T_MMI_AQI_IIR_DL_CON *)p_message)->iir_dl_action = ((T_L1_AQI_IIR_DL_CON *)(msg->SigP))->iir_dl_action;
      }
      break;
    #endif

   #if(L1_AGC_UL == 1)
      case MMI_AQI_AGC_UL_CON:
      {
        /* Translate the l1 message into the Riviera compliant message */
        /* Allocate the Riviera buffer */
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_AQI_AGC_UL_CON),
                                  (T_RVF_BUFFER **) (&p_message));

        /* If insufficient resources, then report a memory error and abort. */
        if (mb_status == RVF_RED)
        {
          /* the memory is insufficient to continue the non regression test */
          AUDIO_SEND_TRACE("AUDIO entity has no memory for MMI_AQI_AGC_UL_CON",RV_TRACE_LEVEL_ERROR);
          return;
        }

        // recorded size includes the end mask
        ((T_MMI_AQI_AGC_UL_CON *)p_message)->agc_ul_action   = ((T_L1_AQI_AGC_UL_CON *)(msg->SigP))->agc_ul_action;
      }
      break;
    #endif

   #if(L1_AGC_DL == 1)
      case MMI_AQI_AGC_DL_CON:
      {
        /* Translate the l1 message into the Riviera compliant message */
        /* Allocate the Riviera buffer */
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_AQI_AGC_DL_CON),
                                  (T_RVF_BUFFER **) (&p_message));

        /* If insufficient resources, then report a memory error and abort. */
        if (mb_status == RVF_RED)
        {
          /* the memory is insufficient to continue the non regression test */
          AUDIO_SEND_TRACE("AUDIO entity has no memory for MMI_AQI_AGC_DL_CON",RV_TRACE_LEVEL_ERROR);
          return;
        }

        // recorded size includes the end mask
        ((T_MMI_AQI_AGC_DL_CON *)p_message)->agc_dl_action   = ((T_L1_AQI_AGC_DL_CON *)(msg->SigP))->agc_dl_action;
      }
      break;
    #endif

    #if(L1_WCM == 1)
      case MMI_AQI_WCM_CON:
      {
        /* Translate the l1 message into the Riviera compliant message */
        /* Allocate the Riviera buffer */
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_AQI_WCM_CON),
                                  (T_RVF_BUFFER **) (&p_message));

        /* If insufficient resources, then report a memory error and abort. */
        if (mb_status == RVF_RED)
        {
          /* the memory is insufficient to continue the non regression test */
          AUDIO_SEND_TRACE("AUDIO entity has no memory for MMI_AQI_WCM_CON",RV_TRACE_LEVEL_ERROR);
          return;
        }

        // recorded size includes the end mask
        ((T_MMI_AQI_WCM_CON *)p_message)->wcm_action   = ((T_L1_AQI_WCM_CON *)(msg->SigP))->wcm_action;
      }
      break;
    #endif


   #if(L1_DRC == 1)
      case MMI_AQI_DRC_CON:
      {
        /* Translate the l1 message into the Riviera compliant message */
        /* Allocate the Riviera buffer */
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_AQI_DRC_CON),
                                  (T_RVF_BUFFER **) (&p_message));

        /* If insufficient resources, then report a memory error and abort. */
        if (mb_status == RVF_RED)
        {
          /* the memory is insufficient to continue the non regression test */
          AUDIO_SEND_TRACE("AUDIO entity has no memory for MMI_AQI_DRC_CON",RV_TRACE_LEVEL_ERROR);
          return;
        }

        // recorded size includes the end mask
        ((T_MMI_AQI_DRC_CON *)p_message)->drc_dl_action = ((T_L1_AQI_DRC_CON *)(msg->SigP))->drc_dl_action;
      }
      break;
    #endif

    #if(L1_AEC == 2)
      case MMI_AQI_AEC_CON:
      {
        /* Translate the l1 message into the Riviera compliant message */
        /* Allocate the Riviera buffer */
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_AQI_AEC_CON),
                                  (T_RVF_BUFFER **) (&p_message));

        /* If insufficient resources, then report a memory error and abort. */
        if (mb_status == RVF_RED)
        {
          /* the memory is insufficient to continue the non regression test */
          AUDIO_SEND_TRACE("AUDIO entity has no memory for MMI_AQI_AEC_CON",RV_TRACE_LEVEL_ERROR);
          return;
        }

        // recorded size includes the end mask
        ((T_MMI_AQI_AEC_CON *)p_message)->aec_action = ((T_L1_AQI_AEC_CON *)(msg->SigP))->aec_action;
      }
      break;
    #endif


default:
        {
          AUDIO_SEND_TRACE("AUDIO entity: wrong L1 confirmation message",RV_TRACE_LEVEL_ERROR);
          break;
        }
    }

    if(p_message != NULL)
{
    /* Fill the message ID */
    ((T_RV_HDR *)p_message)->msg_id    = SignalCode;
}
    /* send the messsage to the audio entity */
    rvf_send_msg (p_audio_gbl_var->addrId,
                  p_message);

#else // OP_RIV_AUDIO

  // Set flag to avoid the FREE(msg) in L1ASYNC.
  l1a.l1_msg_forwarded = TRUE;

  msg->SignalCode = SignalCode;

  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
    l1_trace_message(msg);
  #endif

  os_send_sig(msg, queue);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)

#endif // OP_RIV_AUDIO
}

#if (SPEECH_RECO)
  /*-------------------------------------------------------*/
  /* l1_send_sr_background_msg()                           */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters : Signal Code ofthe message                */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description :  Send a message to the SR background    */
  /*                task.                                  */
  /*                                                       */
  /*-------------------------------------------------------*/
  void l1_send_sr_background_msg(UWORD32 SignalCode)
  {
  #if (OP_RIV_AUDIO == 1)
    void *p_message;
    T_RVF_MB_STATUS mb_status; //omaps00090550
  #else
    xSignalHeaderRec *msg;
  #endif

    switch (SignalCode)
    {
      case L1_SRBACK_SAVE_DATA_REQ:
      {
        // Allocate the partition
        #if (OP_RIV_AUDIO == 1)
          mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                    sizeof (T_L1_SRBACK_SAVE_DATA_REQ),
                                    (T_RVF_BUFFER **) (&p_message));

          /* If insufficient resources, then report a memory error and abort.               */
          if (mb_status == RVF_RED)
          {
            /* the memory is insufficient to continue the non regression test */
            AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
            return;
          }
        #else // OP_RIV_AUDIO
          msg = os_alloc_sig(sizeof(T_L1_SRBACK_SAVE_DATA_REQ));
          DEBUGMSG(status,NU_ALLOC_ERR)
        #endif // OP_RIV_AUDIO

        // Fill the message
        ((T_L1_SRBACK_SAVE_DATA_REQ *)(AUDIO_MSG))->database_id         = l1a_l1s_com.speechreco_task.parameters.database_id;
        ((T_L1_SRBACK_SAVE_DATA_REQ *)(AUDIO_MSG))->model_index         = l1a_l1s_com.speechreco_task.parameters.word_index;
        ((T_L1_SRBACK_SAVE_DATA_REQ *)(AUDIO_MSG))->model_RAM_address   = l1a_l1s_com.speechreco_task.parameters.model_address;
        ((T_L1_SRBACK_SAVE_DATA_REQ *)(AUDIO_MSG))->speech              = l1a_l1s_com.speechreco_task.parameters.speech;
        ((T_L1_SRBACK_SAVE_DATA_REQ *)(AUDIO_MSG))->start_buffer        = l1a_l1s_com.speechreco_task.parameters.speech_address;
        ((T_L1_SRBACK_SAVE_DATA_REQ *)(AUDIO_MSG))->stop_buffer         = l1a_l1s_com.speechreco_task.parameters.speech_address + SC_SR_MMI_2_L1_SPEECH_SIZE;
        ((T_L1_SRBACK_SAVE_DATA_REQ *)(AUDIO_MSG))->start_address       = l1a_l1s_com.speechreco_task.parameters.start_address;
        ((T_L1_SRBACK_SAVE_DATA_REQ *)(AUDIO_MSG))->stop_address        = l1a_l1s_com.speechreco_task.parameters.stop_address;
      }
      break;

      case L1_SRBACK_LOAD_MODEL_REQ:
      {
        // Allocate the partition
        #if (OP_RIV_AUDIO == 1)
          mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                    sizeof (T_L1_SRBACK_LOAD_MODEL_REQ),
                                    (T_RVF_BUFFER **) (&p_message));

          /* If insufficient resources, then report a memory error and abort.               */
          if (mb_status == RVF_RED)
          {
            /* the memory is insufficient to continue the non regression test */
            AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
            return;
          }
        #else // OP_RIV_AUDIO
          msg = os_alloc_sig(sizeof(T_L1_SRBACK_LOAD_MODEL_REQ));
          DEBUGMSG(status,NU_ALLOC_ERR)
        #endif // OP_RIV_AUDIO

        // Fill the message
        ((T_L1_SRBACK_LOAD_MODEL_REQ *)(AUDIO_MSG))->database_id         = l1a_l1s_com.speechreco_task.parameters.database_id;
        ((T_L1_SRBACK_LOAD_MODEL_REQ *)(AUDIO_MSG))->model_index         = l1a_l1s_com.speechreco_task.parameters.word_index;
        ((T_L1_SRBACK_LOAD_MODEL_REQ *)(AUDIO_MSG))->model_RAM_address   = l1a_l1s_com.speechreco_task.parameters.model_address;
        ((T_L1_SRBACK_LOAD_MODEL_REQ *)(AUDIO_MSG))->CTO_enable          = l1a_l1s_com.speechreco_task.parameters.CTO_algorithm;
      }
      break;

      case L1_SRBACK_TEMP_SAVE_DATA_REQ:
      {
        // Allocate the partition
        #if (OP_RIV_AUDIO == 1)
          mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                    sizeof (T_L1_SRBACK_TEMP_SAVE_DATA_REQ),
                                    (T_RVF_BUFFER **) (&p_message));

          /* If insufficient resources, then report a memory error and abort.               */
          if (mb_status == RVF_RED)
          {
            /* the memory is insufficient to continue the non regression test */
            AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
            return;
          }
        #else // OP_RIV_AUDIO
          msg = os_alloc_sig(sizeof(T_L1_SRBACK_TEMP_SAVE_DATA_REQ));
          DEBUGMSG(status,NU_ALLOC_ERR)
        #endif // OP_RIV_AUDIO

        // Fill the message
        ((T_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(AUDIO_MSG))->model_RAM_address_input = l1a_l1s_com.speechreco_task.parameters.model_address;
        ((T_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(AUDIO_MSG))->model_RAM_address_output = l1a_l1s_com.speechreco_task.parameters.model_temp_address;
      }
      break;
    }

    #if (OP_RIV_AUDIO == 1)
      /* fill the message id */
      ((T_RV_HDR *)p_message)->msg_id    = SignalCode;

      /* send the messsage to the audio entity */
      rvf_send_msg (p_audio_gbl_var->addrId,
                    p_message);
    #else // OP_RIV_AUDIO
      msg->SignalCode = SignalCode;

      #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
        l1_trace_message(msg);
      #endif

      os_send_sig(msg, SRBACK_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)
    #endif // OP_RIV_AUDIO
 }
#endif
#if (MELODY_E2)
  /*-------------------------------------------------------*/
  /* l1_send_melody_e2_background_msg()                    */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters : Signal Code of the message               */
  /*              melody ID (0 or 1)                       */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description :  Send a message to the audio background */
  /*                for the melody E2 feature.             */
  /*                                                       */
  /*-------------------------------------------------------*/
  void l1_send_melody_e2_background_msg(UWORD32 SignalCode, UWORD8  melody_id)
  {
  #if (OP_RIV_AUDIO == 1)
    void    *p_message;
    T_RVF_MB_STATUS mb_status; //omaps00090550
  #else
    xSignalHeaderRec *msg;
  #endif

    UWORD8           i;

    if ( SignalCode == L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ )
    {
      // Allocate the partition
    #if (OP_RIV_AUDIO == 1)
      mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                sizeof (T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ),
                                (T_RVF_BUFFER **) (&p_message));

      /* If insufficient resources, then report a memory error and abort.               */
      if (mb_status == RVF_RED)
      {
        /* the memory is insufficient to continue the non regression test */
        AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
        return;
      }
    #else // OP_RIV_AUDIO
      msg = os_alloc_sig(sizeof(T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ));
      DEBUGMSG(status,NU_ALLOC_ERR)
    #endif // OP_RIV_AUDIO

      ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(AUDIO_MSG))->melody_id =
        melody_id;

      if (melody_id == 0)
      {
        // Load instrument for the melody 0
        // Fill the message
        ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(AUDIO_MSG))->number_of_instrument =
          l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument;
        for(i=0; i<SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT; i++)
        {
          ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(AUDIO_MSG))->waves_table_id[i] =
            l1a_l1s_com.melody0_e2_task.parameters.waves_table_id[i];
        }
      }
      if (melody_id == 1)
      {
        // Load instrument for the melody 1
        // Fill the message
        ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(AUDIO_MSG))->number_of_instrument =
          l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument;
        for(i=0; i<SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT; i++)
        {
          ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(AUDIO_MSG))->waves_table_id[i] =
            l1a_l1s_com.melody1_e2_task.parameters.waves_table_id[i];
        }
      }
    }
    else
    if (SignalCode == L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ)
    {
      // Allocate the partition
    #if (OP_RIV_AUDIO == 1)
      mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                sizeof (T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ),
                                (T_RVF_BUFFER **) (&p_message));

      /* If insufficient resources, then report a memory error and abort.               */
      if (mb_status == RVF_RED)
      {
        /* the memory is insufficient to continue the non regression test */
        AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
        return;
      }
    #else // OP_RIV_AUDIO
      msg = os_alloc_sig(sizeof(T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ));
      DEBUGMSG(status,NU_ALLOC_ERR)
    #endif // OP_RIV_AUDIO

      ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(AUDIO_MSG))->melody_id =
        melody_id;

      if (melody_id == 0)
      {
        ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(AUDIO_MSG))->number_of_instrument =
          l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument;
      }
      else
      if (melody_id == 1)
      {
        ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(AUDIO_MSG))->number_of_instrument =
          l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument;
      }
    }

    #if (OP_RIV_AUDIO == 1)
      ((T_RV_HDR *)p_message)->msg_id    = SignalCode;

      /* send the messsage to the audio entity */
      rvf_send_msg (p_audio_gbl_var->addrId,
                    p_message);
    #else // OP_RIV_AUDIO
      msg->SignalCode = SignalCode;

      #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
        l1_trace_message(msg);
      #endif

      os_send_sig(msg, SRBACK_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)
    #endif // OP_RIV_AUDIO
  }
#endif // MELODY_E2

#if (OP_L1_STANDALONE == 0)
void *mem_Allocate(UWORD16 size)
{

void* p_buffer;

      T_HANDLE hTask;

    hTask = e_running[os_MyHandle()];

          if(os_AllocateMemory( hTask , (T_VOID_STRUCT **)&p_buffer,size,OS_NO_SUSPEND, mm_int_data_pool_handle) != 0)
          { 
		if(os_AllocateMemory ( hTask, (T_VOID_STRUCT**)&p_buffer,size,OS_NO_SUSPEND, mm_ext_data_pool_handle) != 0)
        			return NULL;
		else
		{
			   dma_src_port =AUDIO_SP_SOURCE_EMIF;
                     return p_buffer;
		}
	   }
          else
          {
		  	   dma_src_port =AUDIO_SP_SOURCE_IMIF;
		       return p_buffer;
           }
                

}


void mem_Deallocate(void *memoryBlock)
{
   T_HANDLE hTask;

    hTask = e_running[os_MyHandle()];

	if (memoryBlock)
	{
		os_DeallocateMemory(hTask, (T_VOID_STRUCT *) memoryBlock);
	}
  }
#endif//op_l1_standalone=0
#endif // AUDIO_TASK
