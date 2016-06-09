/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_FUNC.C
 *
 *        Filename l1audio_func.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/


/************************************/
/* Include files...                 */
/************************************/
#include "nucleus.h"   //omaps00090550
#include "l1_macro.h"

#include "l1_macro.h"
#include "l1_confg.h"
//#include "l1audio_defty.h"

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
    #include "l1audio_abb.h"
    #include "l1audio_btapi.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif
//added here from e-sample for AAC
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
//added here from e-sample for AAC
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

    #if TESTMODE
      #include "l1tm_msgty.h"
      #include "l1tm_signa.h"
      #include "l1tm_varex.h"
    #endif // TESTMODE

    #if (L1_STEREOPATH == 1)
      #include "sys_dma.h"
      #include "sys_inth.h"
      #include "abb.h"
      #include "l1audio_stereo.h"
    #endif

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
      #include "l1audio_abb.h"

      #if (L1_GTT == 1)
        #include "l1gtt_const.h"
        #include "l1gtt_defty.h"
      #endif
//added here from e-sample for AAC
      #if (L1_DYN_DSP_DWNLD == 1)
        #include "l1_dyn_dwl_const.h"
        #include "l1_dyn_dwl_defty.h"
      #endif
      #if (L1_MP3 == 1)
        #include "l1mp3_defty.h"
	 #include"l1mp3_const.h"
      #endif

      #if (L1_MIDI == 1)
        #include "l1midi_defty.h"
      #endif
//added here from e-sample for AAC
      #if (L1_AAC == 1)
        #include "l1aac_defty.h"
	 #include"l1aac_const.h"	
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

      #if TESTMODE
        #include "l1tm_msgty.h"
        #include "l1tm_signa.h"
        #include "l1tm_varex.h"
      #endif // TESTMODE

      #if (L1_STEREOPATH == 1)
        #include "sys_dma.h"
        #include "sys_inth.h"
        #include "abb.h"
        #include "l1audio_stereo.h"
      #endif

      #include "l1_time.h"
      #if L2_L3_SIMUL
        #include "l1_scen.h"
      #endif
  #endif
#if (OP_RIV_AUDIO == 1)
      #include "rv_general.h"
      #include "audio_api.h"
      #include "audio_structs_i.h"
      #include "audio_var_i.h"
      #include "audio_macro_i.h"
      #include "audio_const_i.h"
    #endif

  #include "l1audio_macro.h"

  /**************************************/
  /* Prototypes for L1S audio function  */
  /**************************************/
  UWORD8  copy_data_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD16 **ptr_buf, UWORD16 data_size, API *ptr_dst);
  UWORD8  copy_data_to_buffer   (UWORD8 session_id, UWORD16 *buffer_size, UWORD16 **ptr_buf, UWORD16 data_size, API *ptr_src);
  #if (MELODY_E2)
    UWORD16 audio_twentyms_to_TDMA_convertion(UWORD16 twentyms_value);
  #endif
  #if (MELODY_E2) || (L1_VOICE_MEMO_AMR)
    UWORD8  copy_byte_data_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, UWORD8 *ptr_dst);
    UWORD8  copy_byte_data_to_buffer   (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, UWORD8 *ptr_src);
  #endif
  #if (L1_VOICE_MEMO_AMR)
    UWORD8  copy_byte_data_le_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, API *ptr_dst);
    UWORD8  copy_byte_data_le_to_buffer   (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, API *ptr_src);
  #endif
  #if (L1_EXT_AUDIO_MGT == 1)
    void l1_ext_audio_mgt_dma_handler(SYS_UWORD16 dma_status);
  #endif
  #if (L1_IIR == 2)
  void l1_audio_iir4x_copy_params();
  #endif

  #if (L1_AGC_UL== 1)
  void l1_audio_agc_ul_copy_params();
  #endif

  #if (L1_AGC_DL== 1)
  void l1_audio_agc_dl_copy_params();
  #endif

  #if (L1_DRC == 1)
    void l1_audio_drc1x_copy_params();
  #endif

#if(L1_BT_AUDIO ==1)||(L1_WCM ==1)
void l1_audio_manager(UWORD8 *src, UWORD16 size);
void l1_audio_bt_init(UINT16 media_buf_size);
extern void l1mp3_dma_it_handler(SYS_UWORD16 d_dma_channel_it_status);
extern void l1aac_dma_it_handler(SYS_UWORD16 d_dma_channel_it_status);
#endif
#if(L1_BT_AUDIO ==1)
T_L1_BT_AUDIO bt_audio;

extern T_MP3_DMA_PARAM *mp3_dma;
extern T_AAC_DMA_PARAM *aac_dma;
extern void l1a_bt_audio_noti_process();
extern UWORD16 pending_dec_req;

#endif
  /**************************************/
  /* External prototypes                */
  /**************************************/
  extern  UWORD8 Cust_get_pointer  (UWORD16 **ptr, UWORD16 *buffer_size, UWORD8 session_id);

  #if ((L1_STEREOPATH == 1) && (OP_L1_STANDALONE == 1))
    extern void l1tm_stereopath_DMA_handler(SYS_UWORD16 dma_status);
    #if TESTMODE
      extern T_STP_DRV_MCU_DSP *stp_drv_ndb;
    #endif
  #endif

  #if (L1_EXT_AUDIO_MGT == 1)
    extern  NU_HISR  EXT_AUDIO_MGT_hisr;
    extern  T_MIDI_DMA_PARAM midi_buf;
  #endif

  #if (L1_DRC == 1)
	extern T_DRC_MCU_DSP *drc_ndb;
  #endif

  /*-------------------------------------------------------*/
  /* copy_data_from_buffer()                               */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters : session_id                               */
  /*              buffer_size                              */
  /*              data_size                                */
  /*              ptr_dst                                  */
  /*              ptr_src                                  */
  /*                                                       */
  /* Return     : error_id                                 */
  /*                                                       */
  /* Description :                                         */
  /* This function copies the data from the buffer         */
  /* (buffer size: buffer_size, start address: ptr_buf) to */
  /* the destination indicated by ptr_dst. The size of the */
  /* data to download is data_size. The session_id         */
  /* indicates to the custom flash manager the type of     */
  /* data.                                                 */
  /*                                                       */
  /*-------------------------------------------------------*/
  UWORD8 copy_data_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD16 **ptr_buf, UWORD16 data_size, API *ptr_dst)
  {
    UWORD8  error_id = 0;

    while (data_size != 0)
    {
      while( (data_size !=0) && (*buffer_size != 0) )
      {
        *ptr_dst++ = *(*ptr_buf)++;
        data_size--;
        (*buffer_size)--;
      }
      // A new buffer is requested in order to finish to copy the data
      if ( data_size != 0)
      {
        *buffer_size = data_size;
        error_id = Cust_get_pointer( ptr_buf, buffer_size, session_id);

        // An error is occured
        if (error_id)
          return(error_id);
      }
    }

    return(error_id);
  }

  /*-------------------------------------------------------*/
  /* copy_data_to_buffer()                                 */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters : session_id                               */
  /*              buffer_size                              */
  /*              data_size                                */
  /*              ptr_dst                                  */
  /*              ptr_src                                  */
  /*                                                       */
  /* Return     : error_id                                 */
  /*                                                       */
  /* Description :                                         */
  /* This function copies the data to the buffer           */
  /* (buffer size: buffer_size, start address: ptr_buf)    */
  /* from the source indicated by ptr_src. The size of the */
  /* data to save is data_size. The session_id             */
  /* indicates to the custom flash manager the type of     */
  /* data.                                                 */
  /*                                                       */
  /*-------------------------------------------------------*/
  UWORD8 copy_data_to_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD16 **ptr_buf, UWORD16 data_size, API *ptr_src)
  {
    UWORD8 error_id = 0;

    while (data_size != 0)
    {
      while( (data_size !=0) && (*buffer_size != 0) )
      {
        *(*ptr_buf)++ = *ptr_src++;
        data_size--;
        (*buffer_size)--;
      }
      // A new buffer is requested in order to finish to copy the data
      if (data_size != 0)
      {
        *buffer_size = data_size;
        error_id = Cust_get_pointer(ptr_buf, buffer_size, session_id);

        // An error is occured
        if (error_id)
          return(error_id);
      }
    }

    return(error_id);
  }

  #if (MELODY_E2) || (L1_VOICE_MEMO_AMR)
    /*-------------------------------------------------------*/
    /* copy_byte_data_from_buffer()                          */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters : session_id                               */
    /*              buffer_size                              */
    /*              data_size                                */
    /*              ptr_dst                                  */
    /*              ptr_src                                  */
    /*                                                       */
    /* Return     : error_id                                 */
    /*                                                       */
    /* Description :                                         */
    /* This function copies byte per byte the data from      */
    /* the buffer (buffer size (in byte): buffer_size,       */
    /* start address: ptr_buf) to the destination indicated  */
    /* by ptr_dst. The size of the data to download is       */
    /* data_size(in byte). The session_id indicates to the   */
    /* custom flash manager the type of data.                */
    /*                                                       */
    /*-------------------------------------------------------*/
    UWORD8 copy_byte_data_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, UWORD8 *ptr_dst)
    {
      UWORD8  error_id = 0;
      UWORD16 requested_size;

      while (data_size != 0)
      {
        while( (data_size !=0) && (*buffer_size != 0) )
        {
          *ptr_dst++ = *(*ptr_buf)++;
          data_size--;
          (*buffer_size)--;
        }
        // A new buffer is requested in order to finish to copy the data
        if ( data_size != 0)
        {
          // Calculate the size of the data to request in 16-bit word
          if (*buffer_size & 0x0001)
          {
            // The size is a odd value
            requested_size = (data_size >> 1) + 1;
          }
          else
          {
            // the size is an even value
            requested_size = (data_size >> 1);
          }

          error_id = Cust_get_pointer( (UWORD16 **)ptr_buf, &requested_size, session_id);

          // An error is occured
          if (error_id)
            return(error_id);

          *buffer_size = (requested_size << 1);
        }
      }

      return(error_id);
    }

    /*-------------------------------------------------------*/
    /* copy_data_to_buffer()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters : session_id                               */
    /*              buffer_size                              */
    /*              data_size                                */
    /*              ptr_dst                                  */
    /*              ptr_src                                  */
    /*                                                       */
    /* Return     : error_id                                 */
    /*                                                       */
    /* Description :                                         */
    /* This function copies byte per byte the data to the    */
    /* buffer (buffer size in byte: buffer_size, start       */
    /* address: ptr_buf) from the source indicated by        */
    /* ptr_src. The size in byte of the data to save is      */
    /* data_size. The session_id indicates to the custom     */
    /* flash manager the type of data.                       */
    /*                                                       */
    /*-------------------------------------------------------*/
    UWORD8 copy_byte_data_to_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, UWORD8 *ptr_src)
    {
      UWORD8 error_id = 0;
      UWORD16 requested_size;

      while (data_size != 0)
      {
        while( (data_size !=0) && (*buffer_size != 0) )
        {
          *(*ptr_buf)++ = *ptr_src++;
          data_size--;
          (*buffer_size)--;
        }
        // A new buffer is requested in order to finish to copy the data
        if (data_size != 0)
        {
          // Calculate the size of the data to request in 16-bit word
          if (*buffer_size & 0x0001)
          {
            // The size is a odd value
            requested_size = (data_size >> 1) + 1;
          }
          else
          {
            // the size is an even value
            requested_size = (data_size >> 1);
          }

          error_id = Cust_get_pointer((UWORD16 **)ptr_buf, &requested_size, session_id);

          // An error is occured
          if (error_id)
            return(error_id);

          *buffer_size = (requested_size << 1);
        }
      }

      return(error_id);
    }
  #endif //#if (MELODY_E2) || (L1_VOICE_MEMO_AMR)

  #if (MELODY_E2)

    /*-------------------------------------------------------*/
    /* audio_twentyms_to_TDMA_convertion()                   */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters : 20ms value                               */
    /*                                                       */
    /* Return     : TDMA value                               */
    /*                                                       */
    /* Description :                                         */
    /* This function convert a duration from 20ms unit to    */
    /* TDMA unit.                                            */
    /*                                                       */
    /*-------------------------------------------------------*/
    UWORD16 audio_twentyms_to_TDMA_convertion(UWORD16 twentyms_value)
    {
      UWORD16 TDMA_value;

      // 20ms # 4 TDMA
      TDMA_value = twentyms_value << 2;

      // Compensation factor: 1 TDMA each 60ms.
      TDMA_value += (twentyms_value/3);

      return(TDMA_value);
    }
  #endif // MELODY_E2

  #if (L1_VOICE_MEMO_AMR)
    /*-------------------------------------------------------*/
    /* copy_byte_data_le_from_buffer()                       */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters : session_id                               */
    /*              buffer_size                              */
    /*              data_size                                */
    /*              ptr_dst                                  */
    /*              ptr_src                                  */
    /*                                                       */
    /* Return     : error_id                                 */
    /*                                                       */
    /* Description :                                         */
    /* This function copies byte per byte the data from      */
    /* the buffer (buffer size (in byte): buffer_size,       */
    /* start address: ptr_buf) to the destination indicated  */
    /* by ptr_dst. The size of the data to download is       */
    /* data_size(in byte). The session_id indicates to the   */
    /* custom flash manager the type of data.                */
    /* Data in buffer is expected to be big-endian and will  */
    /* be copied in order to retrieve little-endian order in */
    /* ptr_dst                                               */
    /*                                                       */
    /*-------------------------------------------------------*/
    UWORD8 copy_byte_data_le_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, API *ptr_dst)
    {
      UWORD8  error_id = 0;
      UWORD16 requested_size;
      UWORD16 data_size_inv = 0;
      UWORD16 temp_uword16 = 0 ; //omaps00090550

      while (data_size_inv != data_size)
      {
        /* we go from 0 to data_size copying to MSB (even numbers) then LSB (odd numbers) of DSP */
        while( (data_size_inv != data_size) && (*buffer_size != 0) )
        {
          if (data_size_inv & 0x0001)
          {
            temp_uword16 |= *(*ptr_buf)++;
            *(ptr_dst)++ = temp_uword16;
          }
          else
          {
            temp_uword16 = (*(*ptr_buf)++ << 8);
          }
          data_size_inv++;
          (*buffer_size)--;
        }
        // A new buffer is requested in order to finish to copy the data
        if ( data_size_inv != data_size)
        {
          // Calculate the size of the data to request in 16-bit word
          if ((data_size - data_size_inv) & 0x0001)
          {
            // The size is an odd value
            requested_size = ((data_size - data_size_inv) >> 1) + 1;
          }
          else
          {
            // the size is an even value
            requested_size = ((data_size - data_size_inv) >> 1);
          }

          error_id = Cust_get_pointer((UWORD16 **)ptr_buf, &requested_size, session_id);

          // An error is occured
          if (error_id)
            return(error_id);

          *buffer_size = (requested_size << 1);
        }
      }

      if (data_size & 0x0001)
      {
        *(ptr_dst)++ = temp_uword16;
      }

      return(error_id);
    }

    /*-------------------------------------------------------*/
    /* copy_byte_data_le_to_buffer()                         */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters : session_id                               */
    /*              buffer_size                              */
    /*              data_size                                */
    /*              ptr_dst                                  */
    /*              ptr_src                                  */
    /*                                                       */
    /* Return     : error_id                                 */
    /*                                                       */
    /* Description :                                         */
    /* This function copies byte per byte the data to the    */
    /* buffer (buffer size in byte: buffer_size, start       */
    /* address: ptr_buf) from the source indicated by        */
    /* ptr_src. The size in byte of the data to save is      */
    /* data_size. The session_id indicates to the custom     */
    /* flash manager the type of data.                       */
    /* Data is expected to be little-endian in ptr_src and   */
    /* will be copied in order to retrieve big-endian        */
    /* order in buffer                                       */
    /*                                                       */
    /*-------------------------------------------------------*/
    UWORD8 copy_byte_data_le_to_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, API *ptr_src)
    {
      UWORD8 error_id = 0;
      UWORD8 data_size_inv = 0;
      UWORD16 requested_size;
      UWORD16 temp_uword16=0; //omaps00090550

      /* we go from 0 to data_size copying MSB (even numbers) then LSB (odd numbers) from DSP */
      while (data_size_inv != data_size)
      {
        while( (data_size_inv != data_size) && (*buffer_size != 0) )
        {
          /* if data_size_inv is odd, we need the LSB of the DSP word */
          if (data_size_inv & 0x0001)
          {
            *(*ptr_buf)++ = (UWORD8)(temp_uword16 & 0x00FF);
          }
          /* if data_size_inv is even, we need the MSB of the DSP word */
          else
          {
            temp_uword16 = *(ptr_src)++;
            *(*ptr_buf)++ = (UWORD8)(temp_uword16 >> 8);
          }
          data_size_inv++;
          (*buffer_size)--;
        }
        // A new buffer is requested in order to finish to copy the data
        if ( data_size_inv != data_size)
        {
          // Calculate the size of the data to request in 16-bit word
          if ((data_size - data_size_inv) & 0x0001)
          {
            // The size is a odd value
            requested_size = ((data_size - data_size_inv) >> 1) + 1;
          }
          else
          {
            // the size is an even value
            requested_size = ((data_size - data_size_inv) >> 1);
          }

          error_id = Cust_get_pointer((UWORD16 **)ptr_buf, &requested_size, session_id);

          // An error occured
          if (error_id)
            return(error_id);

          *buffer_size = (requested_size << 1);
        }
      }

      return(error_id);
    }
  #endif

 #if (L1_STEREOPATH == 1) && (CODE_VERSION == NOT_SIMULATION)
  /*-------------------------------------------------------*/
  /* l1_audio_api_handler()                                */
  /*-------------------------------------------------------*/
  /* Parameters :                                          */
  /* Return     :                                          */
  /* Functionality : API int management                    */
  /*-------------------------------------------------------*/
  void l1_audio_api_handler(void)
  {
   UWORD16 rootcause;

    if (l1a_l1s_com.stereopath_drv_task.parameters.feature_identifier == AUDIO_SP_TESTS_ID)
    {
     #if (TESTMODE && (OP_L1_STANDALONE == 1))
      rootcause = stp_drv_ndb->d_cport_api_dma_rootcause;

      l1tm_stereopath_DMA_handler((SYS_UWORD16) rootcause);
     #endif
    }
  } /* l1_audio_api_handler() */
 #endif   // (L1_STEREOPATH == 1) && (CODE_VERSION == NOT_SIMULATION)

 #if (L1_EXT_AUDIO_MGT == 1)
   /*-------------------------------------------------------*/
   /* l1_ext_audio_mgt_dma_handler()                        */
   /*-------------------------------------------------------*/
   /*                                                       */
   /* Parameters : dma_status                               */
   /*                                                       */
   /* Return     : none                                     */
   /*                                                       */
   /* Description :                                         */
   /* This function is used to handle a DMA interrupt       */
   /* that will notify to the external midi play process    */
   /* that a new buffer is needed.                          */
   /*                                                       */
   /*-------------------------------------------------------*/
   void l1_ext_audio_mgt_dma_handler(SYS_UWORD16 dma_status)
   {
    NU_Activate_HISR(&EXT_AUDIO_MGT_hisr);          // activate external MIDI HISR
   }
 #endif

  #if (L1_LIMITER == 1)
    /*-------------------------------------------------------*/
    /* l1_audio_lim_update_mul_low_high()                    */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description :                                         */
    /* -------------                                         */
    /* This function is used to update mul_low and mul_high  */
    /* Limiter parameters in function to volume gain (Q15)   */
    /* First the function read the last controlled volume    */
    /* in the MCU/DSP API then it processed mul_low[0/1]     */
    /* and mul_high[0/1] according to following equations:   */
    /*                                                       */
    /*  thr_low = -  thr_low_slope x volume + thr_low_0      */
    /* thr_high = - thr_high_slope x volume + thr_high_0     */
    /*                                                       */
    /*    mul_low[0] = 32767 / thr_low                       */
    /*    mul_low[1] = 32767 x thr_low                       */
    /*   mul_high[0] = 32767 / thr_high                      */
    /*   mul_high[1] = 32767 x thr_high                      */
    /*-------------------------------------------------------*/
    void l1_audio_lim_update_mul_low_high()
    {
#if (CODE_VERSION != SIMULATION)
  #if (ANALOG == 3)
      WORD16 volume_q15 = ABB_Read_DLGain();
  #endif
  #if (ANALOG == 11)
    WORD16 volume_q15 = (WORD16)(l1_audio_abb_Read_DLGain());
  #endif
#else
      WORD16 volume_q15 = 0x4000; // -6 dB for example in simulation
#endif
      WORD16 thr_low, thr_high;

      /* Process Thr_low */
      thr_low  = (- (l1a_l1s_com.limiter_task.parameters.thr_low_slope  * volume_q15)>>15) +
                 l1a_l1s_com.limiter_task.parameters.thr_low_0;
//      if (thr_low > 32767) //OMAPS00090550
//        thr_low = 32767;

      /* Process Thr_high */
      thr_high = (- (l1a_l1s_com.limiter_task.parameters.thr_high_slope * volume_q15)>>15) +
                 l1a_l1s_com.limiter_task.parameters.thr_high_0;
//      if (thr_high > 32767) //OMAPS00090550
//        thr_high = 32767;

      /* Process amd store mul_low[0/1] */
      if (thr_low != 0)
        l1s_dsp_com.dsp_ndb_ptr->a_lim_mul_low[0]  = 32767 / thr_low;  // Q0
      else
        l1s_dsp_com.dsp_ndb_ptr->a_lim_mul_low[0]  = 32767;  // should never happen
      l1s_dsp_com.dsp_ndb_ptr->a_lim_mul_low[1]  = thr_low;          // Q15

      /* Process and store mul_high[0/1] */
      if (thr_high != 0)
        l1s_dsp_com.dsp_ndb_ptr->a_lim_mul_high[0] = 32767 / thr_high; // Q0
      else
        l1s_dsp_com.dsp_ndb_ptr->a_lim_mul_high[0]  = 32767;  // should never happen
      l1s_dsp_com.dsp_ndb_ptr->a_lim_mul_high[1] = thr_high;         // Q15
    }

    /*-------------------------------------------------------*/
    /* l1_audio_lim_update_mul_low_high()                    */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description :                                         */
    /* -------------                                         */
    /* This function MUST be called at each volume change in */
    /* order to request a limiter partial update.            */
    /*-------------------------------------------------------*/
    void l1_audio_lim_partial_update()
    {
      // Set partial update command
      l1a_l1s_com.limiter_task.command.partial_update = TRUE;
      // Force L1S execution
      l1a_l1s_com.time_to_next_l1s_task = 0;
    }
  #endif // L1_LIMITER == 1



  #if (L1_AGC_UL == 1)
    /*-------------------------------------------------------*/
    /* l1_audio_agc_ul_copy_params                           */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description :                                         */
    /* -------------                                         */
    /* This function is used to copy the AGC UL 1x parameter */
    /* to API memory using a pointer                         */
    /*-------------------------------------------------------*/

    void l1_audio_agc_ul_copy_params()
	{

      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_control                = l1a_l1s_com.agc_ul_task.parameters.control;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_frame_size             = l1a_l1s_com.agc_ul_task.parameters.frame_size;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_targeted_level         = l1a_l1s_com.agc_ul_task.parameters.targeted_level;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_signal_up              = l1a_l1s_com.agc_ul_task.parameters.signal_up;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_signal_down            = l1a_l1s_com.agc_ul_task.parameters.signal_down;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_max_scale              = l1a_l1s_com.agc_ul_task.parameters.max_scale;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_alpha      = l1a_l1s_com.agc_ul_task.parameters.gain_smooth_alpha;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_alpha_fast = l1a_l1s_com.agc_ul_task.parameters.gain_smooth_alpha_fast;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_beta       = l1a_l1s_com.agc_ul_task.parameters.gain_smooth_beta;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_beta_fast  = l1a_l1s_com.agc_ul_task.parameters.gain_smooth_beta_fast;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_intp_flag         = l1a_l1s_com.agc_ul_task.parameters.gain_intp_flag;

	}
  #endif

  #if (L1_AGC_DL == 1)
    /*-------------------------------------------------------*/
    /* l1_audio_agc_dl_copy_params                           */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description :                                         */
    /* -------------                                         */
    /* This function is used to copy the AGC DL 1x parameter */
    /* to API memory using a pointer                         */
    /*-------------------------------------------------------*/

    void l1_audio_agc_dl_copy_params()
	{
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_control                = l1a_l1s_com.agc_dl_task.parameters.control;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_frame_size             = l1a_l1s_com.agc_dl_task.parameters.frame_size;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_targeted_level         = l1a_l1s_com.agc_dl_task.parameters.targeted_level;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_signal_up              = l1a_l1s_com.agc_dl_task.parameters.signal_up;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_signal_down            = l1a_l1s_com.agc_dl_task.parameters.signal_down;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_max_scale              = l1a_l1s_com.agc_dl_task.parameters.max_scale;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_alpha      = l1a_l1s_com.agc_dl_task.parameters.gain_smooth_alpha;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_alpha_fast = l1a_l1s_com.agc_dl_task.parameters.gain_smooth_alpha_fast;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_beta       = l1a_l1s_com.agc_dl_task.parameters.gain_smooth_beta;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_beta_fast  = l1a_l1s_com.agc_dl_task.parameters.gain_smooth_beta_fast;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_intp_flag         = l1a_l1s_com.agc_dl_task.parameters.gain_intp_flag;

	}
  #endif


  #if (L1_IIR == 2)
    /*-------------------------------------------------------*/
    /* l1_audio_iir4x_copy_params                            */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description :                                         */
    /* -------------                                         */
    /* This function is used to copy the IIR 4x parameter    */
    /* to API memory using a pointer                         */
    /*-------------------------------------------------------*/

    void l1_audio_iir4x_copy_params()
	{
      UWORD8 i;
      UWORD8 j;

      // Set IIR parameters
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_control         = l1a_l1s_com.iir_task.parameters->parameters.control;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_frame_size      = l1a_l1s_com.iir_task.parameters->parameters.frame_size;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_swap        = l1a_l1s_com.iir_task.parameters->parameters.fir_swap;

      // Set parameter os FIR part
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_enable      = l1a_l1s_com.iir_task.parameters->parameters.fir_filter.fir_enable;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_length      = l1a_l1s_com.iir_task.parameters->parameters.fir_filter.fir_length;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_shift       = l1a_l1s_com.iir_task.parameters->parameters.fir_filter.fir_shift;

      for (i=0; i < (l1a_l1s_com.iir_task.parameters->parameters.fir_filter.fir_length); i++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_fir_taps[i]   = l1a_l1s_com.iir_task.parameters->parameters.fir_filter.fir_taps[i];
		}

      // Set parameters for IIR part
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_enable      = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_enable;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_number      = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_number;

      // Set parameters for IIR part - SOS 1
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_1      = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[0].sos_fact;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_1 = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[0].sos_fact_form;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_1[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[0].sos_den[j];
		}
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_1[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[0].sos_num[j];
		}
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_1  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[0].sos_num_form;


      // Set parameters for IIR part - SOS 2
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_2      = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[1].sos_fact;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_2 = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[1].sos_fact_form;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_2[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[1].sos_den[j];
		}
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_2[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[1].sos_num[j];
		}
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_2  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[1].sos_num_form;


      // Set parameters for IIR part - SOS 3
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_3      = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[2].sos_fact;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_3 = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[2].sos_fact_form;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_3[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[2].sos_den[j];
		}
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_3[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[2].sos_num[j];
		}
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_3  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[2].sos_num_form;


      // Set parameters for IIR part - SOS 4
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_4      = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[3].sos_fact;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_4 = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[3].sos_fact_form;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_4[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[3].sos_den[j];
		}
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_4[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[3].sos_num[j];
		}
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_4  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[3].sos_num_form;


      // Set parameters for IIR part - SOS 5
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_5      = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[4].sos_fact;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_5 = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[4].sos_fact_form;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_5[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[4].sos_den[j];
		}
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_5[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[4].sos_num[j];
		}
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_5  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[4].sos_num_form;


      // Set parameters for IIR part - SOS 6
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_6      = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[5].sos_fact;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_6 = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[5].sos_fact_form;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_6[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[5].sos_den[j];
		}
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
		{
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_6[j]  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[5].sos_num[j];
		}
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_6  = l1a_l1s_com.iir_task.parameters->parameters.sos_filter.sos_filter[5].sos_num_form;



      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_gain            = l1a_l1s_com.iir_task.parameters->parameters.gain;

	  }

  #endif // L1_IIR == 2


#if (L1_DRC == 1)
    /*-------------------------------------------------------*/
    /* l1_audio_drc1x_copy_params                            */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description :                                         */
    /* -------------                                         */
    /* This function is used to copy the DRC 1x parameter    */
    /* to API memory using a pointer                         */
    /*-------------------------------------------------------*/

    void l1_audio_drc1x_copy_params()
	{
      UWORD8 i;

      // Set DRC parameters
      drc_ndb->d_drc_speech_mode_samp_f              = l1a_l1s_com.drc_task.parameters->parameters.speech_mode_samp_f;
      drc_ndb->d_drc_num_subbands                    = l1a_l1s_com.drc_task.parameters->parameters.num_subbands;
      drc_ndb->d_drc_frame_len                       = l1a_l1s_com.drc_task.parameters->parameters.frame_len;
      drc_ndb->d_drc_expansion_knee_fb_bs            = l1a_l1s_com.drc_task.parameters->parameters.expansion_knee_fb_bs;
      drc_ndb->d_drc_expansion_knee_md_hg            = l1a_l1s_com.drc_task.parameters->parameters.expansion_knee_md_hg;
      drc_ndb->d_drc_expansion_ratio_fb_bs           = l1a_l1s_com.drc_task.parameters->parameters.expansion_ratio_fb_bs;
      drc_ndb->d_drc_expansion_ratio_md_hg           = l1a_l1s_com.drc_task.parameters->parameters.expansion_ratio_md_hg;
      drc_ndb->d_drc_max_amplification_fb_bs         = l1a_l1s_com.drc_task.parameters->parameters.max_amplification_fb_bs;
      drc_ndb->d_drc_max_amplification_md_hg         = l1a_l1s_com.drc_task.parameters->parameters.max_amplification_md_hg;
      drc_ndb->d_drc_compression_knee_fb_bs          = l1a_l1s_com.drc_task.parameters->parameters.compression_knee_fb_bs;
      drc_ndb->d_drc_compression_knee_md_hg          = l1a_l1s_com.drc_task.parameters->parameters.compression_knee_md_hg;
      drc_ndb->d_drc_compression_ratio_fb_bs         = l1a_l1s_com.drc_task.parameters->parameters.compression_ratio_fb_bs;
      drc_ndb->d_drc_compression_ratio_md_hg         = l1a_l1s_com.drc_task.parameters->parameters.compression_ratio_md_hg;
      drc_ndb->d_drc_energy_limiting_th_fb_bs        = l1a_l1s_com.drc_task.parameters->parameters.energy_limiting_th_fb_bs;
      drc_ndb->d_drc_energy_limiting_th_md_hg        = l1a_l1s_com.drc_task.parameters->parameters.energy_limiting_th_md_hg;
      drc_ndb->d_drc_limiter_threshold_fb            = l1a_l1s_com.drc_task.parameters->parameters.limiter_threshold_fb;
      drc_ndb->d_drc_limiter_threshold_bs            = l1a_l1s_com.drc_task.parameters->parameters.limiter_threshold_bs;
      drc_ndb->d_drc_limiter_threshold_md            = l1a_l1s_com.drc_task.parameters->parameters.limiter_threshold_md;
      drc_ndb->d_drc_limiter_threshold_hg            = l1a_l1s_com.drc_task.parameters->parameters.limiter_threshold_hg;
      drc_ndb->d_drc_limiter_hangover_spect_preserve = l1a_l1s_com.drc_task.parameters->parameters.limiter_hangover_spect_preserve;
      drc_ndb->d_drc_limiter_release_fb_bs           = l1a_l1s_com.drc_task.parameters->parameters.limiter_release_fb_bs;
      drc_ndb->d_drc_limiter_release_md_hg           = l1a_l1s_com.drc_task.parameters->parameters.limiter_release_md_hg;
      drc_ndb->d_drc_gain_track_fb_bs                = l1a_l1s_com.drc_task.parameters->parameters.gain_track_fb_bs;
      drc_ndb->d_drc_gain_track_md_hg                = l1a_l1s_com.drc_task.parameters->parameters.gain_track_md_hg;

      for (i=0; i < DRC_LPF_LENGTH; i++)
		{
          drc_ndb->a_drc_low_pass_filter[i]          = l1a_l1s_com.drc_task.parameters->parameters.low_pass_filter[i];
		}
      for (i=0; i < DRC_BPF_LENGTH; i++)
		{
          drc_ndb->a_drc_mid_band_filter[i]          = l1a_l1s_com.drc_task.parameters->parameters.mid_band_filter[i];
		}

	}

  #endif // L1_DRC == 1

#if(L1_BT_AUDIO == 1)
BOOL L1Audio_InformBtAudioPathState (BOOL connected)
{
    if(connected==bt_audio.connected_status)
	return BT_STATUS_OK;
    else
	return BT_STATUS_ERROR;
}

void L1Audio_RegisterBthal (L1AudioPcmCallback pcmCallback, L1AudioConfigureCallback configCallback)
{
      bt_audio.audio_configure_callback=configCallback;
      bt_audio.audio_pcmblock_callback=pcmCallback;
}
UWORD8 bt_flag=1;


L1AudioPcmStatus L1Audio_PullPcmBlock (L1AudioPcmBlock *pcmBlock)
{
	UWORD8 status=0;
    if(bt_flag == 0)
    	{
    	AUDIO_SEND_TRACE("pull back bef init",RV_TRACE_LEVEL_ERROR);	
	     bt_audio.pcm_data_ready = 0;
		// return L1_PCM_PENDING;
    	}
//	AUDIO_SEND_TRACE("pull back called",RV_TRACE_LEVEL_ERROR);	   

	if(bt_audio.pcm_data_end == 1)
		{
     AUDIO_SEND_TRACE("abnormal BT request mp3/aac",RV_TRACE_LEVEL_ERROR);  	

	  if(bt_audio.pcmblock.lengthInBytes==2*C_MP3_OUTPUT_BUFFER_SIZE)
	  	l1mp3_dma_it_handler(0);
      else if(bt_audio.pcmblock.lengthInBytes==2*C_AAC_OUTPUT_BUFFER_SIZE)
        l1aac_dma_it_handler(0);
	  
	  bt_audio.pcm_data_end = 0;
      bt_audio.pcm_data_ready = 0;
	  bt_flag = 0;
	  return L1_PCM_MEDIA_ENDED;
		}
	
  
  if(bt_audio.pcm_data_failed==0)
  {
       if(bt_audio.pcm_data_ready>0)
     { 
	//AUDIO_SEND_TRACE("Data ready for BT-Pull event",RV_TRACE_LEVEL_ERROR);
       pcmBlock->pcmBuffer= bt_audio.pcmblock.pcmBuffer;
	pcmBlock->lengthInBytes=bt_audio.pcmblock.lengthInBytes;
	
	bt_audio.pcm_data_ready=0;
      
	if( bt_audio.pcm_data_end==1)
	{
              AUDIO_SEND_TRACE("Data ended for BT-Pull event",RV_TRACE_LEVEL_ERROR);
		 return L1_PCM_MEDIA_ENDED;
	}
	else	
	   if(bt_audio.pcmblock.lengthInBytes==2*AUDIO_EXT_MIDI_BUFFER_SIZE)   
             return L1_PCM_READY;
	else	
	       status= L1_PCM_READY;
      }
    else
     {
	 	if( bt_audio.pcm_data_end==1)
	{
              AUDIO_SEND_TRACE("Data ended for BT-Pull event",RV_TRACE_LEVEL_ERROR);
		 return L1_PCM_MEDIA_ENDED;
	}
	//  AUDIO_SEND_TRACE("Data pending for BT-Pull event",RV_TRACE_LEVEL_ERROR);
        bt_audio.pcm_data_pending=1;
	 status= L1_PCM_PENDING;
      }
  }
  else
  {
  	AUDIO_SEND_TRACE("Data failed for BT-Pull event",RV_TRACE_LEVEL_ERROR);
  	return L1_PCM_FAILED;
  }
if((bt_audio.pcmblock.lengthInBytes==2*C_MP3_OUTPUT_BUFFER_SIZE)&& bt_flag==1)
{
	if(l1a_apihisr_com.mp3.command.stop==TRUE)
  	 {
	  bt_flag=0;	
         l1mp3_dma_it_handler(0);
		 bt_audio.pcm_data_end = 0;
	  return L1_PCM_MEDIA_ENDED;
	 }
	else
	   l1mp3_dma_it_handler(0);
	
}
else if(bt_audio.pcmblock.lengthInBytes==2*C_AAC_OUTPUT_BUFFER_SIZE&& bt_flag==1)
{
       if(l1a_apihisr_com.aac.command.stop==TRUE)
  	 {
	   bt_flag=0;
	   AUDIO_SEND_TRACE("Media ended for BT-Pull event",RV_TRACE_LEVEL_ERROR);
	   l1aac_dma_it_handler(0);
  		 bt_audio.pcm_data_end = 0;
	   return L1_PCM_MEDIA_ENDED;
	 }
	else
	   l1aac_dma_it_handler(0); 
}
else if(bt_audio.pcmblock.lengthInBytes==2*AUDIO_EXT_MIDI_BUFFER_SIZE && bt_flag==1)
	if( bt_audio.pcm_data_end==1)
  	 {
	  bt_flag=0;	
          return L1_PCM_MEDIA_ENDED;
	 }
	else
	 l1a_bt_audio_noti_process();
return status;
  

}

void l1_audio_bt_init(UINT16 media_buf_size)
{
     bt_audio.pcm_data_pending    =0;
     bt_audio.pcm_data_end          =0;
     bt_audio.pcm_data_ready       =1; 
     bt_audio.pcm_data_failed        =0; 

     bt_flag=1;
 
if(media_buf_size==C_MP3_OUTPUT_BUFFER_SIZE)
{
	bt_audio.pcmblock.pcmBuffer=(UWORD8 *)&mp3_dma->a_mp3_dma_input_buffer[0][0];
       bt_audio.pcmblock.lengthInBytes=2*media_buf_size;
}
else if(media_buf_size==C_AAC_OUTPUT_BUFFER_SIZE)
{
	bt_audio.pcmblock.pcmBuffer=(UWORD8 *)&aac_dma->a_aac_dma_input_buffer[0][0];
       bt_audio.pcmblock.lengthInBytes=2*media_buf_size;
}
else if(media_buf_size==AUDIO_EXT_MIDI_BUFFER_SIZE)
{
	bt_audio.pcmblock.pcmBuffer=(UWORD8 *)&midi_buf.audio_play_buffer[0];
       bt_audio.pcmblock.lengthInBytes=2*media_buf_size;
}
}
#endif//L1_BT_AUDIO == 1

#if(L1_BT_AUDIO==1) ||(L1_WCM==1)
void l1_audio_manager(UWORD8 *src, UWORD16 size)
{
      UWORD16 i ;
#if(L1_BT_AUDIO==1)
      bt_audio.pcmblock.pcmBuffer=src;
     bt_audio.pcmblock.lengthInBytes=2*size;
#endif
    
    #if (OP_L1_STANDALONE == 0)
    #if 0
      // l1_audio_wcm(src,size);
    if( wcm_enable == 1)
    {
      switch(size)
      {
      case 2048:
	     
        arm_wcm_module( AAC, (T_SINT16 *)src , &wcm_output[0]); 
	break;
      case 1152:

        arm_wcm_module( MP3, (T_SINT16 *)src , &wcm_output[0]); 
	break;

      
      }

        for (i=0 ; i< size ; i++) 
  while(size>0)
	      {
    *src++=(API)*wcm_output++;
    size--;
  }

       /* for (i=0 ; i< size ; i++) 
	      {
                *src++ = (UINT8 )wcm_output[i]&0x00ff;
		*src++ = (UINT8 )(wcm_output[i] & 0xff00) >> 8;
	      }	      
	      */

    }


     #endif
    #endif

     #if(L1_BT_AUDIO==1)
          if(bt_audio.connected_status==TRUE)
             {
	          if(bt_audio.pcm_data_pending>0)
	              {
	                  bt_audio.pcm_data_pending = 0;
				AUDIO_SEND_TRACE("Pending callback",RV_TRACE_LEVEL_ERROR);
                	    bt_audio.audio_pcmblock_callback(&bt_audio.pcmblock);
	             
                     }
		else  
                 bt_audio.pcm_data_ready = 1;
             }
    #endif
}
#endif

UWORD16  l1_ext_audio_get_frequencyrate(UWORD16 frequency_index)
{
  UWORD16 sampling_frequency=0;

  switch(frequency_index)
  {
    case 1:
      sampling_frequency=8000;
      break;
    case 2:
      sampling_frequency=11025;
      break;
    case 3:
      sampling_frequency=16000;
      break;
    case 4:
      sampling_frequency=22050;
      break;
    case 5:
      sampling_frequency=32000;
      break;
    case 6:
      sampling_frequency=44100;
      break;
    case 7:
      sampling_frequency=48000;
      break;
    }

  // Sampling frequency should never be zero
  return sampling_frequency;
}
#endif // AUDIO_TASK
