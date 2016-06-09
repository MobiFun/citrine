/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_SRBACK.C
 *
 *        Filename l1audio_back.c
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

/************************************/
/* Include files...                 */
/************************************/

#define L1_SRBACK_COM // switch to define the l1_srback_com variable
#define L1_AUDIOBACK_MELODYE2 // switch to define the audio background variable

#include "l1_macro.h"
#include "l1_confg.h"

#if (AUDIO_TASK == 1) && (L1_AUDIO_BACKGROUND_TASK)

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
    #include "l1audio_varex.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif
//added form e-sample for AAC
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
//added form e-sample for AAC
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

  #else
  // Layer1 and debug include files.

    #include <ctype.h>
    #include <math.h>
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
    #include "l1audio_varex.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif
//added form e-sample for AAC
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
//added form e-sample for AAC
    #if (L1_AAC == 1)
      #include "l1aac_defty.h"
    #endif

    #include "l1_defty.h"
    #include "cust_os.h"
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
  #endif

  /****************************************/
  /* Prototypes for audio background task */
  /****************************************/
  void l1_audio_background_task       (UWORD32 argc, void *argv);
  #if (SPEECH_RECO)
  void srback_CTO_algorithm         (API *RAM_address);
  void srback_save_model_temp       (API *RAM_address_input, UWORD16 *RAM_address_output);
  #endif
  #if (MELODY_E2 && FIR)
    void audio_background_melody_e2_download_instrument_manager(xSignalHeaderRec *msg);
  #endif
  /**************************************/
  /* External prototypes                */
  /**************************************/
  #if (SPEECH_RECO)
  extern void Cust_srback_save_model     (UWORD8 database, UWORD8 index, API *RAM_address);
  extern void Cust_srback_save_speech    (UWORD8 database, UWORD8 index, UWORD16 *start_buffer, UWORD16 *stop_buffer, UWORD16 *start_speech, UWORD16 *stop_speech);
  extern void Cust_srback_load_model     (UWORD8 database, UWORD8 index, API *RAM_address);
  #endif
  #if (MELODY_E2 && FIR)
    extern UWORD16 Cust_audio_melody_E2_load_instrument (UWORD8 customer_instrument_id,
                                                         API *API_address,
                                                         UWORD16 allowed_size);
  #endif

#if (OP_RIV_AUDIO == 0)
  /*--------------------------------------------------------*/
  /* l1_audio_background_task()                             */
  /*--------------------------------------------------------*/
  /*                                                        */
  /* Description:                                           */
  /* ------------                                           */
  /* This function is a state machine which handles the     */
  /* audio background feature.                              */
  /*                                                        */
  /* Starting messages:        L1_SRBACK_SAVE_DATA_REQ      */
  /*                           L1_SRBACK_TEMP_SAVE_DATA_REQ */
  /*                           L1_SRBACK_LOAD_MODEL_REQ     */
  /*                L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ   */
  /*                L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ */
  /*                                                        */
  /* Result messages (input):  none                         */
  /*                                                        */
  /* Result messages (output): L1_SRBACK_SAVE_DATA_CON      */
  /*                           L1_SRBACK_TEMP_SAVE_DATA_CON */
  /*                           L1_SRBACK_LOAD_MODEL_CON     */
  /*                L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON   */
  /*                L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON */
  /*                                                        */
  /* Reset messages (input):   none                         */
  /*                                                        */
  /* Stop message (input):     non                          */
  /*                                                        */
  /* Stop message (output):    L1_SRBACK_SAVE_DATA_CON      */
  /*                           L1_SRBACK_TEMP_SAVE_DATA_CON */
  /*                           L1_SRBACK_LOAD_MODEL_CON     */
  /*                L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON   */
  /*                L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON */
  /*                                                        */
  /* Rem: to stop immeditly the speech reco feature a flag  */
  /* ---- is created: l1_srback_com.emergency_stop          */
  /*                                                        */
  /*--------------------------------------------------------*/
  void l1_audio_background_task(UWORD32 argc, void *argv)
  {
    xSignalHeaderRec *receive_msg, *confirm_msg;
    UWORD8  index;

    while(1)
    {
      // Wait until a message is receive
      receive_msg = os_receive_sig(SRBACK_QUEUE);

      #if (SPEECH_RECO)
        if (receive_msg->SignalCode == L1_SRBACK_SAVE_DATA_REQ)
        {
          if (l1_srback_com.emergency_stop == FALSE)
          {
            // Call the customer function to save the model
            Cust_srback_save_model( ((T_L1_SRBACK_SAVE_DATA_REQ *)(receive_msg->SigP))->database_id,
                                    ((T_L1_SRBACK_SAVE_DATA_REQ *)(receive_msg->SigP))->model_index,
                                    ((T_L1_SRBACK_SAVE_DATA_REQ *)(receive_msg->SigP))->model_RAM_address );
          }
          if ( ((T_L1_SRBACK_SAVE_DATA_REQ *)(receive_msg->SigP))->speech )
          {
            if (l1_srback_com.emergency_stop == FALSE)
            {
              // Call the customer function to save the speech from a circular buffer to the database
              Cust_srback_save_speech( ((T_L1_SRBACK_SAVE_DATA_REQ *)(receive_msg->SigP))->database_id,
                                       ((T_L1_SRBACK_SAVE_DATA_REQ *)(receive_msg->SigP))->model_index,
                                       ((T_L1_SRBACK_SAVE_DATA_REQ *)(receive_msg->SigP))->start_buffer,
                                       ((T_L1_SRBACK_SAVE_DATA_REQ *)(receive_msg->SigP))->stop_buffer,
                                       ((T_L1_SRBACK_SAVE_DATA_REQ *)(receive_msg->SigP))->start_address,
                                       ((T_L1_SRBACK_SAVE_DATA_REQ *)(receive_msg->SigP))->stop_address);
            }
          }

          // Send the stop confirmation message
          confirm_msg = os_alloc_sig(0);
          DEBUGMSG(status,NU_ALLOC_ERR)
          confirm_msg->SignalCode = L1_SRBACK_SAVE_DATA_CON;
          os_send_sig(confirm_msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)
        } // L1_SRBACK_SAVE_DATA_REQ
        else
        if (receive_msg->SignalCode == L1_SRBACK_LOAD_MODEL_REQ)
        {
          if ( ( ((T_L1_SRBACK_LOAD_MODEL_REQ *)(receive_msg->SigP))->CTO_enable ) == FALSE )
          {
            if (l1_srback_com.emergency_stop == FALSE)
            {
              // Call the function to load a model
              Cust_srback_load_model( ((T_L1_SRBACK_LOAD_MODEL_REQ *)(receive_msg->SigP))->database_id,
                                      ((T_L1_SRBACK_LOAD_MODEL_REQ *)(receive_msg->SigP))->model_index,
                                      ((T_L1_SRBACK_LOAD_MODEL_REQ *)(receive_msg->SigP))->model_RAM_address );
            }
          }
          else
          {
            if (l1_srback_com.emergency_stop == FALSE)
            {
              // Calculate the good index
              index = ((T_L1_SRBACK_LOAD_MODEL_REQ *)(receive_msg->SigP))->model_index>>1;

              // Call the function to load a model with the good index
              Cust_srback_load_model( ((T_L1_SRBACK_LOAD_MODEL_REQ *)(receive_msg->SigP))->database_id,
                                      index,
                                      ((T_L1_SRBACK_LOAD_MODEL_REQ *)(receive_msg->SigP))->model_RAM_address );
            }

            // The CTO algorithm is used and the model index is odd
            if ( ((T_L1_SRBACK_LOAD_MODEL_REQ *)(receive_msg->SigP))->model_index & 0x01 )
            {
              if (l1_srback_com.emergency_stop == FALSE)
              {
                // Call the function to apply the CTO algorithm to the loaded model
                srback_CTO_algorithm( ((T_L1_SRBACK_LOAD_MODEL_REQ *)(receive_msg->SigP))->model_RAM_address );
              }
            }
          }

          // Send the stop confirmation message
          confirm_msg = os_alloc_sig(0);
          DEBUGMSG(status,NU_ALLOC_ERR)
          confirm_msg->SignalCode = L1_SRBACK_LOAD_MODEL_CON;
          os_send_sig(confirm_msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)
        } // L1_SRBACK_LOAD_MODEL_REQ
        else
        if (receive_msg->SignalCode == L1_SRBACK_TEMP_SAVE_DATA_REQ)
        {
          if (l1_srback_com.emergency_stop == FALSE)
          {
            // Call the function to save the model in a temporary buffer
            srback_save_model_temp( ((T_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(receive_msg->SigP))->model_RAM_address_input,
                                    ((T_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(receive_msg->SigP))->model_RAM_address_output );
          }

          // Send the stop confirmation message
          confirm_msg = os_alloc_sig(0);
          DEBUGMSG(status,NU_ALLOC_ERR)
          confirm_msg->SignalCode = L1_SRBACK_TEMP_SAVE_DATA_CON;
          os_send_sig(confirm_msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)
        } // L1_SRBACK_TEMP_SAVE_DATA_REQ
      #endif // SPEECH_RECO
      #if (MELODY_E2 && FIR)
        if ( (receive_msg->SignalCode == L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ) ||
             (receive_msg->SignalCode == L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ) )
        {
          audio_background_melody_e2_download_instrument_manager(receive_msg);
        } // L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ
      #endif // MELODY_E2

      // Deallocate the received message
      os_free_sig(receive_msg);
      DEBUGMSG(status,NU_ALLOC_ERR)
    } // while(1)
  }
#endif // OP_RIV_AUDIO

  #if (SPEECH_RECO)
    /*-------------------------------------------------------*/
    /* srback_CTO_algorithm()                                */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters : RAM_address                              */
    /*                                                       */
    /* Return     : none                                     */
    /*                                                       */
    /* Description : apply the CTO algorithm to the model.   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void srback_CTO_algorithm (API *RAM_address)
    {
      UWORD16 model_size, frame;
      UWORD8  frame_size;

      // This alogrithm changes the model:
      // |frame 0|frame 1|frame 2|frame 3|frame 4|frame 5|frame 6|frame 7|...
      // into a garbage model:
      // |0000000|frame 1|0000000|0000000|frame 4|0000000|0000000|frame 7|...

      // look the size of the model in model frame unit (16 words unit)
      model_size = *RAM_address++;

      frame = 0;

      while( (frame <= model_size) &&
             (l1_srback_com.emergency_stop == FALSE) )
      {
        if ((frame % 3) == 1)
        {
          // This frame is kept
          RAM_address += SC_SR_MODEL_FRAME_SIZE;
        }
        else
        {
          // This frame is set to 0
          frame_size = SC_SR_MODEL_FRAME_SIZE;
          while ( (frame_size != 0) &&
                  (l1_srback_com.emergency_stop == FALSE) )
          {
            *RAM_address++ = 0;
            frame_size--;
          }
        }

        frame++;
      }
    }

    /*-------------------------------------------------------*/
    /* srback_CTO_algorithm()                                */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters : RAM_address_input                        */
    /*              RAM_address_output                       */
    /*                                                       */
    /* Return     : none                                     */
    /*                                                       */
    /* Description : apply the CTO algorithm to the model.   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void srback_save_model_temp (API *RAM_address_input, UWORD16 *RAM_address_output)
    {
      UWORD16 model_size;
      UWORD8  frame_size;

      // look the size of the model in model frame unit (16 words unit)
      model_size = *RAM_address_input;

      // save the header of the model
      *RAM_address_output++ = *RAM_address_input++;

      while( (model_size != 0) &&
             (l1_srback_com.emergency_stop == FALSE) )
      {
        frame_size = SC_SR_MODEL_FRAME_SIZE;
        while ( (frame_size != 0) &&
                (l1_srback_com.emergency_stop == FALSE) )
        {
          *RAM_address_output++ = *RAM_address_input++;
          frame_size--;
        }
        model_size--;
      }
    }
  #endif // SPEECH_RECO

#if (OP_RIV_AUDIO == 0)
  #if (MELODY_E2 && FIR)
    /*--------------------------------------------------------*/
    /*audio_background_melody_e2_download_instrument_manager()*/
    /*--------------------------------------------------------*/
    /*                                                        */
    /* Description:                                           */
    /* ------------                                           */
    /* This function is used to load/unload the instrument of */
    /* the melodies E2.                                       */
    /*                                                        */
    /* Starting messages:                                     */
    /*                L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ   */
    /*                L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ */
    /*                                                        */
    /* Result messages (input):  none                         */
    /*                                                        */
    /* Result messages (output):                              */
    /*                L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON   */
    /*                L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON */
    /*                                                        */
    /* Reset messages (input):   none                         */
    /*                                                        */
    /* Stop message (input):     none.                        */
    /*                                                        */
    /* Stop message (output):                                 */
    /*                L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON   */
    /*                L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON */
    /*                                                        */
    /* Rem: to stop immediatly the instrument download flags  */
    /* ---- is created:                                       */
    /* l1a_l1s_com.melody0_e2_task.parameters.emergency_stop  */
    /* l1a_l1s_com.melody1_e2_task.parameters.emergency_stop  */
    /*                                                        */
    /*--------------------------------------------------------*/
    void audio_background_melody_e2_download_instrument_manager(xSignalHeaderRec *msg)
    {
      xSignalHeaderRec  *confirm_msg;
      UWORD8            instrument_number, max_number_of_instrument, instrument_id, id;
      UWORD16           size;
      UWORD32           address;

      if (msg->SignalCode == L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ)
      {
        // Load the instrument

        // Init the first address
        address = ( ((UWORD32)(l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_instrument_wave))
          - SC_AUDIO_MCU_API_BEGIN_ADDRESS );
        l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_instrument_ptr[0] =
            (API)( (address>>1) + SC_AUDIO_DSP_API_BEGIN_ADDRESS );

        // Download the instrument
        max_number_of_instrument = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->number_of_instrument;
        for(instrument_number=0; instrument_number < max_number_of_instrument ; instrument_number++)
        {
          // No instrument was previously download
          if (audioback_melody_e2.number_of_user[instrument_number] == 0)
          {
            // load the insturment ID
            instrument_id =
              ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[instrument_number];

            // Find if this instrument was already downloaded
            id = 0;
            while ( (id < SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT) &&
                    ((instrument_id != audioback_melody_e2.instrument_id[id]) ||
                     (audioback_melody_e2.number_of_user[id] == 0)) )
            {
              id++;
            }

            if (id < SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT)
            {
              // This insturment was already downloaded
              // copy the address of this instrument
              l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_instrument_ptr[instrument_number] =
                l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_instrument_ptr[id];

              // The size of this instrument is 0
              audioback_melody_e2.instrument_size[instrument_number] = 0;
            }
            else
            {
                // Load the customer instrument
              size = Cust_audio_melody_E2_load_instrument (
                      ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[instrument_number],
                      audioback_melody_e2.API_address,
                      audioback_melody_e2.allowed_size);

              // Added to stop the L1 in case of download error
              if (size == 0)
              {
                // Send a message to stop the L1
                /* send the stop command to the audio L1 */
                /* allocate the buffer for the message to the L1 */
                confirm_msg = os_alloc_sig(0);
                DEBUGMSG(status,NU_ALLOC_ERR)

                if (confirm_msg != NULL)
                {
                  /* send the stop command to the audio L1 */
                  if ( ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(confirm_msg->SigP))->melody_id == 0)
                  {
                    confirm_msg->SignalCode = MMI_MELODY0_E2_STOP_REQ;
                  }
                  else
                  {
                    confirm_msg->SignalCode = MMI_MELODY1_E2_STOP_REQ;
                  }
                  os_send_sig(confirm_msg, L1C1_QUEUE);
                  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
                }
              }

              // Save the size of this instrument
              audioback_melody_e2.instrument_size[instrument_number] = size;
            }

            // Update the Cust_audio_melody_E2_load_instrument argument
            audioback_melody_e2.API_address   += audioback_melody_e2.instrument_size[instrument_number];
            audioback_melody_e2.allowed_size  -= audioback_melody_e2.instrument_size[instrument_number];

            // Put the DSP address to the NDB API for the next instrument
            if (instrument_number < SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT-1)
            {
              address = ( ((UWORD32)(audioback_melody_e2.API_address))
                - SC_AUDIO_MCU_API_BEGIN_ADDRESS );

              l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_instrument_ptr[instrument_number + 1] =
                  (API)( (address>>1) + SC_AUDIO_DSP_API_BEGIN_ADDRESS );
            }

            // Save the instrument ID
            audioback_melody_e2.instrument_id[instrument_number] = instrument_id;
          }

          // Increase the number of user of this instrument number
          audioback_melody_e2.number_of_user[instrument_number]++;
        }

        // Send the load confirmation message
        confirm_msg = os_alloc_sig(sizeof(T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON));
        DEBUGMSG(status,NU_ALLOC_ERR)

        // Fill the parameter
        ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON *)(confirm_msg->SigP))->melody_id =
          ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id;

        confirm_msg->SignalCode = L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON;

        os_send_sig(confirm_msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      }
      else
      if (msg->SignalCode == L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ)
      {
        // Unload the instrument
        max_number_of_instrument = ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->number_of_instrument;
        for(instrument_number = max_number_of_instrument; instrument_number > 0 ; instrument_number--)
        {
          // Decrease the number of user of this instrument number
          audioback_melody_e2.number_of_user[instrument_number-1]--;

          // Check if the instrument must be removed
          if (audioback_melody_e2.number_of_user[instrument_number-1] == 0)
          {
            // Increase the size and decrease the pointer to the API with the size
            // of the removed instrument
            // Update the Cust_audio_melody_E2_load_instrument argument
            audioback_melody_e2.API_address   -= audioback_melody_e2.instrument_size[instrument_number-1];
            audioback_melody_e2.allowed_size  += audioback_melody_e2.instrument_size[instrument_number-1];
          }
        }

        // Send the unload confirmation message
        confirm_msg = os_alloc_sig(sizeof(T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON));
        DEBUGMSG(status,NU_ALLOC_ERR)

        // Fill the paramter
        ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON *)(confirm_msg->SigP))->melody_id =
          ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id;

        confirm_msg->SignalCode = L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON;

        os_send_sig(confirm_msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      }
    }
  #endif // MELODY_E2
#endif // OP_RIV_AUDIO
#endif // AUDIO_TASK == 1 && L1_AUDIO_BACKGROUND_TASK
