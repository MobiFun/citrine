/*
 * l1audio_stereo.c
 *
 * Control audio
 *
 *        Filename l1audio_stereo.c
 *  Copyright 2003 (C) Texas Instruments
 *
 *
 */

#include "l1_macro.h"
#include "l1_confg.h"

#define _L1AUDIO_STEREO_C_

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

#if (L1_STEREOPATH == 1)
#include "sys_dma.h"
#include "sys_inth.h"
#include "abb.h"
#if TESTMODE
#include "l1tm_msgty.h"
#endif
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

#if (L1_STEREOPATH == 1)
#include "sys_dma.h"
#include "sys_inth.h"
#include "abb.h"
#if TESTMODE
#include "l1tm_msgty.h"
#endif
#include "l1audio_stereo.h"
#endif

#include "l1audio_abb.h"
#endif

#include "l1audio_macro.h"

//add the extern reference of abb_write_done
#if (ANALOG == 11)
#include "bspTwl3029_I2c.h"
#include "bspTwl3029_Aud_Map.h"
#endif

#if (ANALOG == 11)
Bsp_Twl3029_I2cTransReqArray l1audio_i2cTransArray;
#endif

#if (L1_STEREOPATH == 1) && (CODE_VERSION == NOT_SIMULATION)

#if (ANALOG == 11)
 #include "bspTwl3029_Int_Map.h"


//Add the call back function of the stereo path.
void l1s_stereopath_callback(UWORD8 cbvalue)
{
  BspTwl3029_ReturnCode returnVal = BSP_TWL3029_RETURN_CODE_FAILURE;
  UWORD16 count = 0;
  UINT8 triton_classD = 0;

  /* callback function info pointer */
  BspTwl3029_I2C_Callback i2c_callback;
  BspTwl3029_I2C_CallbackPtr callbackPtr= &i2c_callback;

  /* I2C array */
  Bsp_Twl3029_I2cTransReqArray i2cTransArray;
  Bsp_Twl3029_I2cTransReqArrayPtr i2cTransArrayPtr= &i2cTransArray;

  /* twl3029 I2C reg info struct */
  BspTwl3029_I2C_RegisterInfo regInfo[10] ;
  BspTwl3029_I2C_RegisterInfo* regInfoPtr = regInfo;

  BspTwl3029_I2C_RegData tmpAudioHFTest1RegData=0;
  BspTwl3029_I2C_RegData tmpCtrl3RegData=0;

  bspTwl3029_Audio_getClassD_mode(&triton_classD);

  //Set the valud of abb_write_done to 1
  l1s.abb_write_done = 1;
  switch(cbvalue)
  {
    case L1S_TWL3029_STEROPATH_START:
    {
      l1a_l1s_com.outen_cfg_task.command_commited = l1a_l1s_com.outen_cfg_task.command_requested;

      returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_CTRL3_OFFSET,
                                              &tmpCtrl3RegData);
      returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_HFTEST1_OFFSET,
                                              &tmpAudioHFTest1RegData);

      returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN1_OFFSET,
                                              l1a_l1s_com.outen_cfg_task.outen1,  regInfoPtr++);
      count++;

      returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
                                              l1a_l1s_com.outen_cfg_task.outen2,  regInfoPtr++);
      count++;

      if(l1a_l1s_com.outen_cfg_task.classD == 0x01) // User wants to configure classD
      {
        if(triton_classD == 0x00) // User wants to switch on and Triton not configured for classD
        {
          returnVal= BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_CKG_TESTUNLOCK_OFFSET,
                                              0xb6,  regInfoPtr++);
          count++;

          returnVal= BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
                                              l1a_l1s_com.outen_cfg_task.outen3,  regInfoPtr++);
          count++;

          tmpCtrl3RegData |= 0x80;   // AUDIO_CTRL3_SPKDIGON
          tmpAudioHFTest1RegData = 0x01; // AUDIO_HFTEST1_SPKALLZB

          BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_CTRL3_OFFSET,
                                               tmpCtrl3RegData,  regInfoPtr++);
          count++;

          BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_HFTEST1_OFFSET,
                                               tmpAudioHFTest1RegData,  regInfoPtr++);
          count++;
          }
        }
        else if(l1a_l1s_com.outen_cfg_task.classD == 0x00)
        {
          if(triton_classD != 0x00) // User wants no to classD and Triton configured for classD
          {
            tmpCtrl3RegData &= 0x7F;   // AUDIO_CTRL3_SPKDIGON
            tmpAudioHFTest1RegData = 0x00; // AUDIO_HFTEST1_SPKALLZB

            BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_HFTEST1_OFFSET,
                                            tmpAudioHFTest1RegData,  regInfoPtr++);
            count++;

            BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_CTRL3_OFFSET,
                                                 tmpCtrl3RegData,  regInfoPtr++);
            count++;

            returnVal= BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
                                                 l1a_l1s_com.outen_cfg_task.outen3,  regInfoPtr++);
            count++;

            BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_CKG_TESTUNLOCK_OFFSET,
                                                 0x00,  regInfoPtr++);
            count++;
          }
          else // User no classD & Triton also no classD
          {
            returnVal= BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
                                                l1a_l1s_com.outen_cfg_task.outen3,  regInfoPtr++);
            count++;
            returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
	                                            l1a_l1s_com.outen_cfg_task.outen2,  regInfoPtr++);
            count++;
            returnVal= BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
                                                l1a_l1s_com.outen_cfg_task.outen3,  regInfoPtr++);
            count++;
          }
        }
                /*Switch OFF all pop modes unconditionally , it is turned on before turning on STON*/
                returnVal=BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_POPMAN_OFFSET, 0x00,regInfoPtr++);
                count++;

        callbackPtr->callbackFunc =  l1s_stereopath_callback;
        callbackPtr->callbackVal = L1S_TWL3029_STEROPATH_OUTEN_CONFIG;

        if (returnVal != BSP_TWL3029_RETURN_CODE_FAILURE)
        {
          regInfoPtr = regInfo;
          /* now request to I2C manager to write to Triton registers */

         returnVal = BspTwl3029_I2c_regInfoSend(regInfo,count,callbackPtr,
                                             (BspI2c_TransactionRequest*)i2cTransArrayPtr);
        }
        break;
        }
    case L1S_TWL3029_STEROPATH_CONFIG:
    case L1S_TWL3029_STEROPATH_OUTEN_CONFIG:
    case L1S_TWL3029_STEROPATH_STOP:
        break;
    default:
        break;
    }/* end switch */
}  /* end function l1s_stereopath_callback */

#endif

/*-------------------------------------------------------*/
/* l1s_stereopath_drv_config_ABB()                       */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters : mono_stereo: indicates if buffer is made */
/*              of mono or stereo samples                */
/*              sampling_frequency: sampling freq        */
/*                                                       */
/* Return     : none                                     */
/*                                                       */
/* Description : ABB configuration function              */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_stereopath_drv_config_ABB(UWORD8 mono_stereo,UWORD8 sampling_frequency)
{
#if (ANALOG == 3)
    UWORD16 regist;

    // Get the VAUDCTRL register
    regist = l1s_dsp_com.dsp_ndb_ptr->d_vaud_cfg >> 6;

    // reset sampling frequency and stereo/mono conversion
    regist &= 0x319;

    // stereo/mono conversion ?
    if (mono_stereo == AUDIO_SP_MONO_OUTPUT)
	regist |= 6;

    // apply the request sampling frequency
    regist |= (sampling_frequency << 5);

    // update DSP API
    l1s_dsp_com.dsp_ndb_ptr->d_vaud_cfg = ABB_L1_WRITE(VAUDCTRL, regist);

    // Get the VBCTRL2 register
    regist = l1s_dsp_com.dsp_ndb_ptr->d_vbctrl2 >> 6;
    // activate HSOVMID and VMIDFBYP
    regist |= 0x90;
    l1s_dsp_com.dsp_ndb_ptr->d_vbctrl2 = ABB_L1_WRITE(VBCTRL2, regist);

    // Get the VAUDPLL register
    regist = l1s_dsp_com.dsp_ndb_ptr->d_vaud_pll >> 6;
    // reset PLL
    regist &= 0x3fd;
    // switch PLL on
    regist |= 0x2;

    // update DSP API
    l1s_dsp_com.dsp_ndb_ptr->d_vaud_pll = ABB_L1_WRITE(VAUDPLL, regist);

    // Get the VBPOP register
    regist = l1s_dsp_com.dsp_ndb_ptr->d_vbpop >> 6;
    // deactivate vbpop for HSO
    regist &= 0x3F8;

    // update DSP API
    l1s_dsp_com.dsp_ndb_ptr->d_vbpop = ABB_L1_WRITE(VBPOP, regist);

#endif

#if (ANALOG == 11)
    // Call to the Triton API which would configure the Audio path.
    UWORD8 monostereo;
    UWORD8 pll;
    //Call back function
    BspTwl3029_I2C_Callback stereo_callbackFunc;

    //Switch on the Stereo path PLL - Set the STPLLON to 1
    pll = 0x02;
    //Check if stereo2mono is required
    if (mono_stereo == AUDIO_SP_MONO_OUTPUT)
	monostereo = 0x03;
    else
	monostereo = 0x00;

    //Set the call back function to be called after the Audio configuration
    stereo_callbackFunc.callbackFunc = l1s_stereopath_callback ;
    //stereo_callbackFunc.callbackVal  = L1S_TWL3029_STEROPATH_CONFIG;
    stereo_callbackFunc.callbackVal  = L1S_TWL3029_STEROPATH_START;
    stereo_callbackFunc.i2cTransArrayPtr = &l1audio_i2cTransArray;

    //Call the Triton audio path configuration function
    if(BSP_TWL3029_RETURN_CODE_SUCCESS == bspTwl3029_Audio_Configure_Stereopath(&stereo_callbackFunc,
		pll,
		monostereo,
		sampling_frequency ))
    {
	//Set the valud of abb_write_done to 1
	l1s.abb_write_done = 1;
    }

#endif
}


/*-------------------------------------------------------*/
/* l1s_stereopath_drv_start_ABB()                        */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters : none                                     */
/*                                                       */
/* Return     : none                                     */
/*                                                       */
/* Description : ABB start function                      */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_stereopath_drv_start_ABB(void)
{
#if (ANALOG == 3)
    // Get VAUDPLL register
    UWORD16 regist = l1s_dsp_com.dsp_ndb_ptr->d_vaud_pll >> 6;

    // Reset I2S
    regist &= 0x2ff;
    // I2S on
    regist |= 0x100;

    // update DSP API
    l1s_dsp_com.dsp_ndb_ptr->d_vaud_pll = ABB_L1_WRITE(VAUDPLL, regist);

    // Set AUDON bit of the PWRDNRG register
    regist = 0x100;
    // update DSP API
    l1s_dsp_com.dsp_ndb_ptr->d_togbr2 = ABB_L1_WRITE(TOGBR2, regist);
#endif
#if (ANALOG == 11)
    //Call back function
    BspTwl3029_I2C_Callback stereo_callbackFunc;
    //Set the call back function to be called after the Audio start
    stereo_callbackFunc.callbackFunc = l1s_stereopath_callback ;
    stereo_callbackFunc.callbackVal  = L1S_TWL3029_STEROPATH_CONFIG;
    stereo_callbackFunc.i2cTransArrayPtr = &l1audio_i2cTransArray;

    //call the Triton audio path start function
    if (BSP_TWL3029_RETURN_CODE_SUCCESS == bspTwl3029_Audio_Start_Stereopath(&stereo_callbackFunc))
    {
	l1s.abb_write_done = 1;
    };
#endif

}

/*-------------------------------------------------------*/
/* l1s_stereopath_drv_stop_ABB()                         */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters : none                                     */
/*                                                       */
/* Return     : none                                     */
/*                                                       */
/* Description : ABB stop function                       */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_stereopath_drv_stop_ABB(void)
{
#if (ANALOG == 3)
    UWORD16 regist;

    // Reset AUDON bit of the PWRDNRG register
    regist = 0x80;
    // update DSP API
    l1s_dsp_com.dsp_ndb_ptr->d_togbr2 = ABB_L1_WRITE(TOGBR2, regist);

    // Get VAUDPLL register
    regist = l1s_dsp_com.dsp_ndb_ptr->d_vaud_pll >> 6;
    // PLL/I2S off
    regist &= 0x2fd;
    ABB_Write_Register_on_page(PAGE1, VAUDPLL, regist);
    // update DSP API
    l1s_dsp_com.dsp_ndb_ptr->d_vaud_pll = ABB_L1_WRITE(VAUDPLL, regist);
#endif
#if (ANALOG == 11)
    //Call back function
    BspTwl3029_I2C_Callback stereo_callbackFunc;

    UWORD8  dl_control = 1;  // OUTEN_DISABLE

    // Set the call back function to be called after the Audio start
    stereo_callbackFunc.callbackFunc = l1s_stereopath_callback ;
    stereo_callbackFunc.callbackVal = L1S_TWL3029_STEROPATH_STOP;
    stereo_callbackFunc.i2cTransArrayPtr = &l1audio_i2cTransArray;

    if( (l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_ON)   ||
	    (l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_SWITCHON_STARTED))
    {
	dl_control = 0;   // OUTEN_ENABLE
    }

    //call the Triton audio path start function
    if (BSP_TWL3029_RETURN_CODE_SUCCESS == bspTwl3029_Audio_Stop_Stereopath(&stereo_callbackFunc, dl_control))
    {
	l1s.abb_write_done = 1;
    };
#endif
}


/*-------------------------------------------------------*/
/* l1s_stereopath_drv_start_DMA()                        */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters : d_dma_channel_parameter: DMA parameters  */
/*              DMA_allocation: allocation of the DMA    */
/*                              (MCU/DSP)                */
/*                                                       */
/* Return     : none                                     */
/*                                                       */
/* Description : DMA config and start function           */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_stereopath_drv_start_DMA(T_DMA_TYPE_CHANNEL_PARAMETER d_dma_channel_parameter,UWORD8 DMA_allocation)
{
#if ((CHIPSET == 12) || (CHIPSET == 15))
#if (L1_MP3_SIX_BUFFER == 1)
	// allocate the DMA to the MCU
    f_dma_channel_allocation_set(d_dma_channel_parameter.d_dma_channel_number, C_DMA_CHANNEL_ARM);

    // set parameters
    f_dma_channel_parameter_set((T_DMA_TYPE_CHANNEL_PARAMETER *)&d_dma_channel_parameter);
#else 
	// set parameters
    f_dma_channel_parameter_set((T_DMA_TYPE_CHANNEL_PARAMETER *)&d_dma_channel_parameter);

    // allocate the DMA to the MCU
    f_dma_channel_allocation_set(d_dma_channel_parameter.d_dma_channel_number, C_DMA_CHANNEL_ARM);

#endif 
    // Enable the DMA channel
    f_dma_channel_enable(d_dma_channel_parameter.d_dma_channel_number);


    // DMA allocation ?
    if (DMA_allocation == AUDIO_SP_DMA_ALLOC_MCU)
    {
	// DMA is allocate to MCU, just unmask DMA IT
	F_INTH_ENABLE_ONE_IT(C_INTH_DMA_IT);
    }
    else
    {
	// DMA is allocate to DSP, unmask API IT
	F_INTH_ENABLE_ONE_IT(C_INTH_API_IT);

	// Re-allocate DMA to the DSP
	f_dma_channel_allocation_set(d_dma_channel_parameter.d_dma_channel_number, C_DMA_CHANNEL_DSP);
    }
#endif
}

/*-------------------------------------------------------*/
/* l1s_stereopath_drv_reset_DMA()                        */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters : d_dma_channel_parameter: DMA parameters  */
/*                                                       */
/* Return     : none                                     */
/*                                                       */
/* Description : DMA reset function                      */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_stereopath_drv_reset_DMA(T_DMA_TYPE_CHANNEL_PARAMETER d_dma_channel_parameter)
{
#if ((CHIPSET == 12) || (CHIPSET == 15))
    // Allocate DMA to MCU
    f_dma_channel_allocation_set(d_dma_channel_parameter.d_dma_channel_number, C_DMA_CHANNEL_ARM);

    // Disable DMA channel
    f_dma_channel_disable(d_dma_channel_parameter.d_dma_channel_number);

    // Reset DMA channel
    f_dma_channel_soft_reset(d_dma_channel_parameter.d_dma_channel_number);
#endif
}


/*-------------------------------------------------------*/
/* l1s_stereopath_drv_reset_CPORT()                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters : none                                     */
/*                                                       */
/* Return     : none                                     */
/*                                                       */
/* Description : Cport reset function                    */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_stereopath_drv_reset_CPORT(void)
{
#if ((CHIPSET == 12) || (CHIPSET == 15))
    // init = 1 --> write ctrl register
    l1s_dsp_com.dsp_ndb_ptr->d_cport_init = (API) 0x0001;
    // set SW_RESET field to 1 to generate a software reset
    l1s_dsp_com.dsp_ndb_ptr->d_cport_ctrl = (API) 0x0FFD;
#endif
}

/*-------------------------------------------------------*/
/* l1s_stereopath_drv_stop_CPORT()                       */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters : none                                     */
/*                                                       */
/* Return     : none                                     */
/*                                                       */
/* Description : Cport stop function                     */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_stereopath_drv_stop_CPORT(void)
{
#if (CHIPSET == 12) || (CHIPSET == 15)
    // init = 1 --> write ctrl register
    l1s_dsp_com.dsp_ndb_ptr->d_cport_init = (API) 0x0001;
    // Set EXT_MCLK_EN field to 1 to enable external clock
    l1s_dsp_com.dsp_ndb_ptr->d_cport_ctrl = (API) 0x0000;
#endif
}


/*-------------------------------------------------------*/
/* l1s_stereopath_drv_config_CPORT()                     */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters : none                                     */
/*                                                       */
/* Return     : none                                     */
/*                                                       */
/* Description : Cport config function                   */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_stereopath_drv_config_CPORT(void)
{
#if (CHIPSET == 12) || (CHIPSET == 15)
    // init = 0x281F --> write cfr1,2,3,4 and ctrl register
    l1s_dsp_com.dsp_ndb_ptr->d_cport_init   = (API) 0x281F;
    // 2 timeslots per frame, I2S mode
    // 20 CLK_BIT cycles, 16 data bits per time slot
    l1s_dsp_com.dsp_ndb_ptr->a_cport_cfr[0] = (API) 0x0C0B;
    // one cycle delay, enable data serial output
    // CSYNC signal generate with the negative edge of CSCLK
    // clk direction set to input
    // l1s_dsp_com.dsp_ndb_ptr->a_cport_cfr[1] = (API) 0xEB00;
    l1s_dsp_com.dsp_ndb_ptr->a_cport_cfr[1] = (API) 0xFB00;    // L1_DSP-SPR-18866
    // mask receive/transmit interrupt request
    // Set threshold to 2 (nb of elements = 2)
    l1s_dsp_com.dsp_ndb_ptr->d_cport_ctrl   = (API) 0x012C;
#endif
}


/*-------------------------------------------------------*/
/* l1s_stereopath_drv_start_CPORT()                      */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters : none                                     */
/*                                                       */
/* Return     : none                                     */
/*                                                       */
/* Description : Cport start function                    */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_stereopath_drv_start_CPORT(void)
{
#if (CHIPSET == 12) || (CHIPSET == 15)
    // init = 0x20 --> write tcl_tadt register
    l1s_dsp_com.dsp_ndb_ptr->d_cport_init     = (API) 0x0020;
    // CPEN = 1 --> cport enable
    l1s_dsp_com.dsp_ndb_ptr->d_cport_tcl_tadt = (API) 0x0800;
#endif
}

#endif // L1_STEREOPATH && CODE_VERSION

#endif // AUDIO_TASK
