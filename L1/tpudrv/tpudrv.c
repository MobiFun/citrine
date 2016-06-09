/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 *
 *        Filename tpudrv.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/
/*
 * TPUDRV.C
 *
 * TPU driver for Pole Star
 *
 *
 * Copyright (c) Texas Instruments 1996
 *
 */

#include "config.h"
#include "l1_confg.h"
#include "l1_macro.h"
#include "../../bsp/iq.h"
#include "l1_const.h"
#include "l1_types.h"

#if (AUDIO_TASK == 1)
  #include "l1audio_const.h"
  #include "l1audio_cust.h"
  #include "l1audio_defty.h"
#endif

#if (L1_GTT == 1)
  #include "l1gtt_const.h"
  #include "l1gtt_defty.h"
#endif

#if (L1_MIDI == 1)
  #include "l1midi_defty.h"
#endif

#include "sys_types.h"

#if TESTMODE
  #include "l1tm_defty.h"
#endif

#if (L1_MP3 == 1)
  #include "l1mp3_defty.h"
#endif

#if (L1_MIDI == 1)
  #include "l1midi_defty.h"
#endif

#if (L1_AAC == 1)
  #include "l1aac_defty.h"
#endif
#include "l1_defty.h"
#include "tpudrv.h"
#include "sys_types.h"
#include "../../bsp/clkm.h"
#include "l1_time.h"
#include "l1_varex.h"
#include "l1_trace.h"

#if (L1_MADC_ON == 1)
#if (RF_FAM == 61)
#include "drp_api.h"
#include "l1_rf61.h"
#include "drp_drive.h"
#include "tpudrv61.h"
extern T_DRP_REGS_STR  *drp_regs;
#endif
#endif //L1_MADC_ON

/* RFTime environment */
#if defined (HOST_TEST)
  #include "hostmacros.h"
#endif


/*
 * VEGA and OMEGA  receive windows - Defined in Customer-specific file
 */

extern UWORD32 debug_tpu;

#if ( OP_WCP == 1 ) && ( OP_L1_STANDALONE != 1 )
  // WCS Patch : ADC during RX or TX
  extern inline void GC_SetAdcInfo(unsigned char bTxBasedAdc);
#endif

/*
 * Global Variables
 */
// GSM1.5 : TPU MEMORY is 16-bit instead of 32 in Gemini/Polxx
//------------------------------------------------------------
SYS_UWORD16 *TP_Ptr = (SYS_UWORD16 *) TPU_RAM;


/*--------------------------------------------------------------*/
/*  TPU_Reset : Reset the TPU                                   */
/*--------------------------------------------------------------*/
/* Parameters : on/off(1/0)                                     */
/* Return : none                                                */
/* Functionality : ) Reset the TPU                              */
/*--------------------------------------------------------------*/
void TPU_Reset(SYS_UWORD16 on)
{
  if (on)
  {
    * ((volatile SYS_UWORD16 *) TPU_CTRL) |= TPU_CTRL_RESET;
    // WA for read/modify/write access problem with REG_TPU_CTRL present on Ulysse/Samson/Calypso
    while (!((*(volatile SYS_UWORD16 *) TPU_CTRL) &  TPU_CTRL_RESET));
  }
  else
  {
    * ((volatile SYS_UWORD16 *) TPU_CTRL) &= ~TPU_CTRL_RESET;
    // WA for read/modify/write access problem with REG_TPU_CTRL present on Ulysse/Samson/Calypso
    while (((*(volatile SYS_UWORD16 *) TPU_CTRL) &  TPU_CTRL_RESET));
  }
}

/*--------------------------------------------------------------*/
/*  TSP_Reset : Reset the TSP                                   */
/*--------------------------------------------------------------*/
/* Parameters : on/off(1/0)                                     */
/* Return : none                                                */
/* Functionality : ) Reset the TSP                              */
/*--------------------------------------------------------------*/
void TSP_Reset(SYS_UWORD16 on)
{
  if (on)
  {
    * ((volatile SYS_UWORD16 *) TPU_CTRL) |= TSP_CTRL_RESET;
    // WA for read/modify/write access problem with REG_TPU_CTRL present on Ulysse/Samson/Calypso
    while (!((*(volatile SYS_UWORD16 *) TPU_CTRL) &  TSP_CTRL_RESET));
  }
  else
  {
    * ((volatile SYS_UWORD16 *) TPU_CTRL) &= ~TSP_CTRL_RESET;
    // WA for read/modify/write access problem with REG_TPU_CTRL present on Ulysse/Samson/Calypso
    while (((*(volatile SYS_UWORD16 *) TPU_CTRL) &  TSP_CTRL_RESET));
  }
}

/*--------------------------------------------------------------*/
/*  TPU_SPIReset : Reset the SPI of the TPU                     */
/*--------------------------------------------------------------*/
/* Parameters : on/off(1/0)                                     */
/* Return : none                                                */
/* Functionality : ) the SPI of the TPU                         */
/*--------------------------------------------------------------*/

void TPU_SPIReset(SYS_UWORD16 on)
{
  if (on)
  {
    * ((volatile SYS_UWORD16 *) TPU_CTRL) |= TPU_CTRL_SPI_RST;
    // WA for read/modify/write access problem with REG_TPU_CTRL present on Ulysse/Samson/Calypso
    while (!((*(volatile SYS_UWORD16 *) TPU_CTRL) &  TPU_CTRL_SPI_RST));
  }
  else
  {
    * ((volatile SYS_UWORD16 *) TPU_CTRL) &= ~TPU_CTRL_SPI_RST;
    // WA for read/modify/write access problem with REG_TPU_CTRL present on Ulysse/Samson/Calypso
    while (((*(volatile SYS_UWORD16 *) TPU_CTRL) &  TPU_CTRL_SPI_RST));
  }
}

/*--------------------------------------------------------------*/
/*  TPU_ClkEnable :                                             */
/*--------------------------------------------------------------*/
/* Parameters : on/off(1/0)                                     */
/* Return : none                                                */
/* Functionality :  Enable the TPU clock                        */
/*--------------------------------------------------------------*/

void TPU_ClkEnable(SYS_UWORD16 on)
{
  if (on)
  {
    * ((volatile SYS_UWORD16 *) TPU_CTRL) |= TPU_CTRL_CLK_EN;
    // WA for read/modify/write access problem with REG_TPU_CTRL present on Ulysse/Samson/Calypso
    while (!((*(volatile SYS_UWORD16 *) TPU_CTRL) &  TPU_CTRL_CLK_EN));
  }
  else
  {
    * ((volatile SYS_UWORD16 *) TPU_CTRL) &= ~TPU_CTRL_CLK_EN;
    // WA for read/modify/write access problem with REG_TPU_CTRL present on Ulysse/Samson/Calypso
    while (((*(volatile SYS_UWORD16 *) TPU_CTRL) &  TPU_CTRL_CLK_EN));
  }
}

/*--------------------------------------------------------------*/
/*   TPU_Frame_ItOn :                                           */
/*--------------------------------------------------------------*/
/* Parameters :  bit  of it to enable                           */
/* Return : none                                                */
/* Functionality :  Enable frame it                             */
/*--------------------------------------------------------------*/

/*-----------------------------------------------------------*/
/* Warning read modify write access to TPU_INT_CTRL register */
/* may generate problems using Hyperion.                     */
/*-----------------------------------------------------------*/

void TPU_FrameItOn(SYS_UWORD16 it)
{
      * ((volatile SYS_UWORD16 *) TPU_INT_CTRL) &= ~it;
}

void TPU_FrameItEnable(void)
{
  #if W_A_ITFORCE
    (*(volatile SYS_UWORD16 *)TPU_INT_CTRL) |= TPU_INT_ITD_F;
  #else
    // enable IT_DSP generation on next frame
    // reset by DSP when IT occurs
    (*(volatile SYS_UWORD16 *) TPU_CTRL) |=  TPU_CTRL_D_ENBL;
    // WA for read/modify/write access problem with REG_TPU_CTRL present on Ulysse/Samson/Calypso
    while (!((*(volatile SYS_UWORD16 *) TPU_CTRL) &  TPU_CTRL_D_ENBL));
  #endif
}

/*--------------------------------------------------------------*/
/*   TPU_check_IT_DSP :                                         */
/*--------------------------------------------------------------*/
/* Parameters : none                                            */
/* Return : none                                                */
/* Functionality :  check if an IT DSP still pending                               */
/*--------------------------------------------------------------*/
BOOL TPU_check_IT_DSP(void)
{ // return TRUE if an IT DSP still pending.
  return( (((*(volatile SYS_UWORD16 *) TPU_CTRL) &  TPU_CTRL_D_ENBL) == TPU_CTRL_D_ENBL));
}

/*--------------------------------------------------------------*/
/*   TPU_DisableAllIt :                                         */
/*--------------------------------------------------------------*/
/* Parameters : none                                            */
/* Return : none                                                */
/* Functionality :  Disabl all it                               */
/*--------------------------------------------------------------*/
void  TPU_DisableAllIt()
{
        * ((volatile SYS_UWORD16 *) TPU_INT_CTRL) |= TPU_INT_ITF_M | TPU_INT_ITP_M | TPU_INT_ITD_M;

}





/*
 * TP_Program
 *
 * Write a null-terminated scenario into TPU memory at a given start address
 * (Do not write terminating 0)
 *
 */
void *TP_Program(const SYS_UWORD16 *src)
{
   /* Write TPU instructions until SLEEP */
   while (*src)
   {
      *TP_Ptr++ = *src++;
   }
   #if 1 //(TOOL_CHOICE == 3)   // 2.54 Migration
   return((void *)NULL);
   #endif // TOOL_CHOICE == 3
    //  return((void *)NULL);//ompas00090550

}



void TP_Reset(SYS_UWORD16 on)
{
   if (on) {
      * ((volatile SYS_UWORD16 *) TPU_CTRL) |= (TPU_CTRL_RESET | TSP_CTRL_RESET);
      while (!((*(volatile SYS_UWORD16 *) TPU_CTRL) &  (TPU_CTRL_RESET | TSP_CTRL_RESET)));
   }
   else {
      * ((volatile SYS_UWORD16 *) TPU_CTRL) &= ~(TPU_CTRL_RESET | TSP_CTRL_RESET);
      while (((*(volatile SYS_UWORD16 *) TPU_CTRL) &  (TPU_CTRL_RESET | TSP_CTRL_RESET)));
   }
}

void TP_Enable(SYS_UWORD16 on)
{
   if(on)
   {
     * ((volatile SYS_UWORD16 *) TPU_CTRL) |= TPU_CTRL_T_ENBL;

     // Some time shall be wait before leaving the function to ensure that bit has been taken
     // in account by the TPU. A while loop such as in function TP_reset can't be used as the
     // ARM can be interrupted within this loop and in that case the pulse will be missed (CQ20781).
     // The bit is updated in the worst case 24 cycles of 13MHz later it as been written by the MCU.
     // 24 ticks of 13MHz = 1.84us. Lets take 3us to keep some margin.
     wait_ARM_cycles(convert_nanosec_to_cycles(3000)); // wait 3us
   }
   else
   {
     * ((volatile SYS_UWORD16 *) TPU_CTRL) &= ~TPU_CTRL_T_ENBL;
     // Some time shall be wait before leaving the function to ensure that bit has been taken
     // in account by the TPU. A while loop such as in function TP_reset can't be used as the
     // ARM can be interrupted within this loop and in that case the pulse will be missed (CQ20781).
     // The bit is updated in the worst case 24 cycles of 13MHz later it as been written by the MCU.
     // 24 ticks of 13MHz = 1.84us. Lets take 3us to keep some margin.
     wait_ARM_cycles(convert_nanosec_to_cycles(3000)); // wait 3us
   }
}


/*-----------------------------------------------------------------------*/
/*   Function name: TPU_wait_idle                                        */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/*   Parameters:    None                                                 */
/*                                                                       */
/*   Return:        None                                                 */
/*                                                                       */
/*-----------------------------------------------------------------------*/
/*   Description:   Wait until TPU scenario execution is complete        */
/*                                                                       */
/*-----------------------------------------------------------------------*/
void TPU_wait_idle(void)
{
  while( ((*(volatile SYS_UWORD16 *) (TPU_CTRL)) & TPU_CTRL_TPU_IDLE) == TPU_CTRL_TPU_IDLE)
  {
    wait_ARM_cycles(convert_nanosec_to_cycles(3000));
  }
}


/*
 * l1dmacro_idle
 *
 * Write SLEEP instruction, start TPU and reset pointer
 */
void l1dmacro_idle (void)
{
   *TP_Ptr++ = TPU_SLEEP;

   /* start TPU */
   TP_Ptr = (SYS_UWORD16 *) TPU_RAM;
   TP_Enable(1);
}

/*
 * l1dmacro_offset
 *
 * Set OFFSET register
 *
 */
void l1dmacro_offset    (UWORD32 offset_value, WORD32 relative_time)
{
   // WARNING: 'relative time' and 'offset_value' must always be comprised
   // between 0 and TPU_CLOCK_RANGE !!!

   if (relative_time != IMM)      // IMM indicates to set directly without AT
   {
     *TP_Ptr++ = TPU_FAT(relative_time);
   }
   *TP_Ptr++ = TPU_OFFSET(offset_value);
}

/*
 * l1dmacro_synchro
 *
 * Set synchro register
 */
void l1dmacro_synchro (UWORD32 when, UWORD32 value)
{
   // WARNING: 'when' must always be comprised between 0 and TPU_CLOCK_RANGE !!!
   #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
     trace_fct(CST_L1DMACRO_SYNCHRO, 1);//omaps00090550
   #endif

   if (value != IMM)      // IMM indicates to set directly without AT
   {
     *TP_Ptr++ = TPU_FAT(when);
   }

   *TP_Ptr++ = TPU_SYNC(value);
   l1s.tpu_offset_hw = value;  // memorize the offset set into the TPU.
}


/*
 * l1dmacro_adc_read
 *
 */
void l1dmacro_adc_read_rx(void)
{

  #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
     // TSP needs to be configured in order to send serially to Omega

//   *TP_Ptr++ = TPU_MOVE  (TSP_SPI_SET1, TSP_CLK_RISE); // Clock configuration
     *TP_Ptr++ = TPU_WAIT  (5);
     *TP_Ptr++ = TPU_MOVE  (TSP_CTRL1,6);                // Device and Nb of bits configuration
     *TP_Ptr++ = TPU_MOVE  (TSP_TX_REG_1,STARTADC);      // Load data to send
     *TP_Ptr++ = TPU_MOVE  (TSP_CTRL2, TC2_WR);          // Start serialization command and adc conversion
     *TP_Ptr++ = TPU_WAIT  (5);
     *TP_Ptr++ = TPU_MOVE  (TSP_TX_REG_1,0x00);
     *TP_Ptr++ = TPU_MOVE  (TSP_CTRL2, TC2_WR);          // Reset startadc pulse

     #if (TRACE_TYPE==1)||(TRACE_TYPE ==4)
       #if (GSM_IDLE_RAM == 0)
         l1_trace_ADC(0);
       #else
         l1_trace_ADC_intram(0);
       #endif
     #endif
  #endif

#if (L1_MADC_ON == 1)
  #if (ANALOG == 11)

     #if (TRACE_TYPE==1)||(TRACE_TYPE ==4)
       #if (GSM_IDLE_RAM == 0)
         l1_trace_ADC(0);
       #else
         l1_trace_ADC_intram(0);
       #endif
     #endif
  #endif

  #if (OP_WCP == 1) && (OP_L1_STANDALONE != 1)
    // WCS patch: ADC during RX
    GC_SetAdcInfo(0);
  #endif
#endif
}


#if (CODE_VERSION != SIMULATION)
#if (L1_MADC_ON ==1)
/*
 * l1dmacro_adc_read_rx_cs_mode0
 *
 * Purpose:
 * ======
 * MADC is not enabled during CS_MODE0 periodically. MADC is enabled in CS_MODE0
 * when Layer 1 receives MPHC_RXLEV_REQ from L23. However in CS_MODE0, MPHC_RXLEV_REQ
 * is not received periodically. In case network is not found, the period between 2 MPHC_RXLEV_REQ
 * increases and can be as high as 360 seconds (Maximum Value)
 * This can result in battery related issues like phone powering off without MMI indication.
 */


void l1dmacro_adc_read_rx_cs_mode0(void)
{
          *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U,START_ADC);
          *TP_Ptr++ = TPU_WAIT  (2);
          *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U,0);

#if (L1_MADC_ON == 1)
  #if (ANALOG == 11)

     #if (TRACE_TYPE==1)||(TRACE_TYPE ==4)
       #if (GSM_IDLE_RAM == 0)
         l1_trace_ADC(0);
       #else
         l1_trace_ADC_intram(0);
       #endif
     #endif
  #endif

  #if (OP_WCP == 1) && (OP_L1_STANDALONE != 1)
    // WCS patch: ADC during RX
    GC_SetAdcInfo(0);
  #endif
#endif
}


#endif //If MADC is enabled
#endif //If Not Simulation

/*
 * l1dmacro_adc_read_tx
 *
 */


#if (ANALOG != 11)
void l1dmacro_adc_read_tx(UWORD32 when)
#else
void l1dmacro_adc_read_tx(UWORD32 when, UWORD8 tx_up_state)
#endif
{

  #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))

     *TP_Ptr++ = TPU_FAT (when);
     *TP_Ptr++ = TPU_MOVE  (TSP_CTRL1,6);                // Device and Nb of bits configuration
     *TP_Ptr++ = TPU_MOVE  (TSP_TX_REG_1, STARTADC|BULON|BULENA);      // Load data to send
     *TP_Ptr++ = TPU_MOVE  (TSP_CTRL2, TC2_WR);          // Start serialization command and adc conversion
     *TP_Ptr++ = TPU_WAIT  (5);
     *TP_Ptr++ = TPU_MOVE  (TSP_TX_REG_1,          BULON|BULENA);
     *TP_Ptr++ = TPU_MOVE  (TSP_CTRL2, TC2_WR);          // Reset startadc pulse

     #if (TRACE_TYPE==1)||(TRACE_TYPE ==4)
       l1_trace_ADC(1);
     #endif
  #endif

#if (L1_MADC_ON == 1)
  #if (ANALOG == 11)
     *TP_Ptr++ = TPU_FAT (when);
     *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U,tx_up_state | START_ADC);
     *TP_Ptr++ = TPU_WAIT  (2);
     *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U,tx_up_state);

#if 1 // TEMP MEASUREMENT - uncomment and test after MADC
#if (RF_FAM == 61)
     *TP_Ptr++ = TPU_MOVE(OCP_DATA_MSB, ((START_SCRIPT(DRP_TEMP_CONV))>>8) & 0xFF);      \
	 *TP_Ptr++ = TPU_MOVE(OCP_DATA_LSB, (START_SCRIPT(DRP_TEMP_CONV))     & 0xFF);      \
	 *TP_Ptr++ = TPU_MOVE(OCP_ADDRESS_MSB, (((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF)>>8) & 0xFF));      \
	 *TP_Ptr++ = TPU_MOVE(OCP_ADDRESS_LSB, ((UWORD16)( ((UWORD32)(&drp_regs->SCRIPT_STARTL))&0xFFFF))     & 0xFF);     \
	 *TP_Ptr++ = TPU_MOVE(OCP_ADDRESS_START,     0x01);                \
       //TEMP_MEAS: Call TEMP Conv Script in DRP
     //MOVE_REG_TSP_TO_RF(START_SCRIPT(DRP_TEMP_CONV),(UWORD16)(&drp_regs->SCRIPT_STARTL));
#endif

#endif

     #if (TRACE_TYPE==1)||(TRACE_TYPE ==4)
       l1_trace_ADC(1);
     #endif

  #endif
#endif //L1_MADC_ON

     #if (OP_WCP == 1) && (OP_L1_STANDALONE != 1)
      // WCS patch: ADC during TX
      GC_SetAdcInfo(1);
     #endif


}


/*
#if (RF_FAM == 61)

void l1dmacro_adc_read_tx(UWORD32 when, UWORD8 tx_up_state)
{
    int i;

	 *TP_Ptr++ = TPU_FAT (when);
	 *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U,tx_up_state | START_ADC);
	 *TP_Ptr++ = TPU_WAIT  (2);
         *TP_Ptr++ = TPU_MOVE(REG_SPI_ACT_U,tx_up_state);

     #if (TRACE_TYPE==1)||(TRACE_TYPE ==4)
       l1_trace_ADC(1);
     #endif


  #if (OP_WCP == 1) && (OP_L1_STANDALONE != 1)
   // WCS patch: ADC during TX
   GC_SetAdcInfo(1);
  #endif
}

#endif
*/

/*
 * l1dmacro_set_frame_it
 *
 */
void l1dmacro_set_frame_it(void)
{
  TPU_FrameItEnable();
}
