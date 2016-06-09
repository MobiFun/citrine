/******************************************************************************/
/*          TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION            */
/*                                                                            */
/* Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only    */
/* Unauthorized reproduction and/or distribution is strictly prohibited.  This*/
/* product  is  protected  under  copyright  law  and  trade  secret law as an*/
/* unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All*/
/* rights reserved.                                                           */
/*                                                                            */
/*                                                                            */
/* Filename          : abb_inth.c                                             */
/*                                                                            */
/* Description       : Functions to manage the ABB device interrupt.          */
/*                     The Serial Port Interface is used to connect the TI    */
/*	               Analog BaseBand (ABB).				      */
/*	               It is assumed that the ABB is connected as the SPI     */
/*                     device 0, and ABB interrupt is mapped as external IT.  */
/*                                                                            */
/* Author            : Pascal PUEL                                            */
/*                                                                            */
/* Version number   : 1.2                                                     */
/*                                                                            */
/* Date and time    : 07/02/03                                                */
/*                                                                            */
/* Previous delta   : Creation                                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* 17/12/03                                                                   */
/* The original abb_inth.c has been splitted between the actual abb_inth.c    */
/* located in drv_apps directory and abb_inth_core.c located in drv_core      */
/* directory.                                                                 */
/*                                                                            */
/******************************************************************************/

#include "../../include/config.h"
#include "../../include/sys_types.h"
#include "../../riviera/rv/general.h"
#include "../../nucleus/nucleus.h"      // for NUCLEUS functions and types

#include "../../L1/include/l1_confg.h"
#include "../../L1/include/l1_macro.h"

#include <string.h>          
#include "abb_inth.h"

#include "../../riviera/rv/rv_defined_swe.h"     // for RVM_PWR_SWE

#if (CHIPSET == 12)
    #include "sys_inth.h"
#else
    #include "../iq.h"
#endif

#include "../../gpf/inc/cust_os.h"
#include "../../L1/include/l1_signa.h"
#include "abb.h"

#if defined (OP_WCP)
  #include "ffs/ffs.h"
  #include "ffs/board/ffspcm.h"
#endif

#include "../../riviera/rvm/rvm_use_id_list.h"   // for SPI_USE_ID
#include "spi_env.h"
#include "spi_process.h"           // for ABB_EXT_IRQ_EVT
#if 0	// FreeCalypso
#include "kpd/kpd_power_api.h"     // for kpd_power_key_pressed()
#include "power/power.h"	 
#endif

#ifdef RVM_LCC_SWE
  #include "lcc/lcc_api.h"
  #include "lcc/lcc_cfg_i.h"
  #include "lcc/lcc.h"
  #include "lcc/lcc_env.h"
#endif
/******************************************************************************/
/*                                                                            */
/*  Function Name: spi_abb_read_int_reg_callback                              */
/*                                                                            */
/*  Purpose:      Callback function                                           */
/*                Called when an external interrupt has occured and the       */
/*                ABB interrupt register has been read.                       */
/*                                                                            */
/******************************************************************************/
void spi_abb_read_int_reg_callback(SYS_UWORD16 *read_value)
{
   SYS_UWORD16 loop_count;
   SYS_UWORD16 status_value;
   xSignalHeaderRec *adc_msg;
   volatile SYS_UWORD8 i;

#ifdef RVM_LCC_SWE
   struct pwr_adc_ind_s *addr;
   extern T_PWR_CTRL_BLOCK *pwr_ctrl;
#endif

/*
 * FreeCalypso: the following logic, which makes sense for complete
 * phones but not for Openmoko-style modems, has not been integrated
 * yet.
 */
#if 0
   // check all the possible causes of the ABB IT
   if (*read_value & PUSHOFF_IT_STS)
   {
      /* Push Button from ON to OFF */
      if (SPI_GBL_INFO_PTR->is_gsm_on == TRUE)
      {
         NU_Sleep(SHORT_OFF_KEY_PRESSED);

       // WCP Patch
       #if (OP_WCP == 1)
         // Backup of GSM FFS is remotely handled by MPU-S
         // we trigger the backup upon each ON->OFF transition
         ffs_backup ();
       #else
         /* Since this callback function is called from the SPI task
         it can't be interrupted by another task
         so we can directly access the SPI through the low-level driver */

         #if ((ANLG_FAM == 1) || (ANLG_FAM == 2))
         status_value = (ABB_Read_Status() & ONREFLT);
         #elif (ANLG_FAM == 3)
         status_value = (ABB_Read_Register_on_page(PAGE1, VRPCCFG) & PWOND);
         #endif

         if (status_value == PWR_OFF_KEY_PRESSED)
         {
            /* Inform keypad that key ON/OFF has been pressed */
            kpd_power_key_pressed();

            loop_count = 0;
            /* Wait loop for Power-OFF */
            while ((loop_count < OFF_LOOP_COUNT) &&
                   (status_value == PWR_OFF_KEY_PRESSED))
            {
               NU_Sleep(SHORT_OFF_KEY_PRESSED);
               #if ((ANLG_FAM == 1) || (ANLG_FAM == 2))
               status_value = (ABB_Read_Status() & ONREFLT);
               #elif (ANLG_FAM == 3)
               status_value = (ABB_Read_Register_on_page(PAGE1, VRPCCFG) & PWOND);
               #endif
               loop_count++;
            }

            if (status_value == PWR_OFF_KEY_PRESSED) /* Power-OFF request detected */
            {
               rvf_send_trace("IQ EXT: Power Off request",25, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);

               Power_OFF_Button();
            }
         }
       #endif //WCP
      }
      else  /* GSM OFF */
      { 
         rvf_send_trace("IQ EXT: Power On request",24, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);

         Power_ON_Button();
      }
   }

   else if (*read_value & REMOT_IT_STS)
   {
      rvf_send_trace("IQ EXT: Power Off remote request",32, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);

      /* 'Remote Power' from ON to OFF */
      Power_OFF_Remote();
   }
#else
   /* dummy to satisfy C */
   if (0)
      ;
#endif
        
   else if (*read_value & ADCEND_IT_STS)
   {
      rvf_send_trace("IQ EXT: ADC End",15, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);

      /* ADC end of conversion */
      ABB_Read_ADC(&SPI_GBL_INFO_PTR->adc_result[0]);
      adc_msg = os_alloc_sig(sizeof(T_CST_ADC_RESULT));
     if(adc_msg != NULL)
     {
        adc_msg->SignalCode = CST_ADC_RESULT;

        for(i=0;i<MADC_NUMBER_OF_MEAS;i++)
        {
          ((T_CST_ADC_RESULT *)(adc_msg->SigP))->adc_result[i] = SPI_GBL_INFO_PTR->adc_result[i];
        }
        os_send_sig(adc_msg, RRM1_QUEUE);
#ifdef RVM_LCC_SWE
        // Send ADC measurement to PWR (LCC) task
        // NOTE that memory is allocated externally in the PWR task
        if (rvf_get_buf(pwr_ctrl->prim_id, sizeof(struct pwr_adc_ind_s), (void *)&addr) == RVF_RED) {
            rvf_send_trace("rvf_get_buf failed",18, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, LCC_USE_ID);
            /* Unmask External interrupt */
            IQ_Unmask(IQ_EXT);
//            rvf_dump_mem();
            return;
        }
        addr->header.msg_id        = PWR_ADC_IND;
        addr->header.src_addr_id   = SPI_GBL_INFO_PTR->addr_id;
        addr->header.dest_addr_id  = pwr_ctrl->addr_id;
        addr->header.callback_func = NULL;
        // FIXME: memcpy from SPI_GBL_INFO_PTR->adc_result - make sure it has not been de-allocated
        memcpy(addr->data, SPI_GBL_INFO_PTR->adc_result, 8*2);
        addr->data[9] = ABB_Read_Status();; // Read & assign ITSTATREG status so we save the polling in PWR task!!
        if (rvf_send_msg(pwr_ctrl->addr_id, addr) != RV_OK) {
            rvf_send_trace("SPI FATAL: Send failed!",23, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, LCC_USE_ID);
        }
#endif
      }
   }

#if (defined(RVM_PWR_SWE) || defined(RVM_LCC_SWE))
   else if (*read_value & CHARGER_IT_STS)
   {
      /* Charger plug IN or OUT */
#if ((ANLG_FAM == 1) || (ANLG_FAM == 2))
      status_value = ABB_Read_Status();
#elif (ANLG_FAM == 3)
      status_value = ABB_Read_Register_on_page(PAGE1, VRPCCFG);
#endif
      if (status_value & CHGPRES)
      {
         rvf_send_trace("IQ EXT: Charger Plug",20, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);
#ifdef RVM_PWR_SWE
         PWR_Charger_Plug();  /* charger plugged IN */
#endif
#ifdef RVM_LCC_SWE
          // Forward charger plug indication to PWR (LCC) task
          // NOTE that memory is allocated externally in the PWR task
          if (rvf_get_buf(pwr_ctrl->prim_id, sizeof(struct pwr_req_s), (void *)&addr) == RVF_RED) {
              rvf_send_trace("rvf_get_buf failed#1",20, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, LCC_USE_ID);
              rvf_dump_mem();
          }
          addr->header.msg_id        = PWR_CHARGER_PLUGGED_IND;
          addr->header.src_addr_id   = SPI_GBL_INFO_PTR->addr_id;
          addr->header.dest_addr_id  = pwr_ctrl->addr_id;
          addr->header.callback_func = NULL;
          if (rvf_send_msg(pwr_ctrl->addr_id, addr) != RV_OK) {
              rvf_send_trace("SPI FATAL: Send failed!",23, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, LCC_USE_ID);
          }
#endif
      }
      else
      {
         rvf_send_trace("IQ EXT: Charger Unplug",22, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);

#ifdef RVM_PWR_SWE
         PWR_Charger_Unplug();   /* charger plugged OUT */
#endif
#ifdef RVM_LCC_SWE
          // Forward charger unplug indication to PWR (LCC) task
          // NOTE that memory is allocated externally in the PWR task
          if (rvf_get_buf(pwr_ctrl->prim_id, sizeof(struct pwr_req_s), (void *)&addr) == RVF_RED) {
              rvf_send_trace("rvf_get_buf failed#2",20, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, LCC_USE_ID);
              rvf_dump_mem();
          }
          addr->header.msg_id        = PWR_CHARGER_UNPLUGGED_IND;
          addr->header.src_addr_id   = SPI_GBL_INFO_PTR->addr_id;
          addr->header.dest_addr_id  = pwr_ctrl->addr_id;
          addr->header.callback_func = NULL;
          if (rvf_send_msg(pwr_ctrl->addr_id, addr) != RV_OK) {
              rvf_send_trace("SPI FATAL: Send failed!",23, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, LCC_USE_ID);
      }
#endif
      }
   }

#endif /* RVM_PWR_SWE || RVM_LCC_SWE */

   /* Unmask External interrupt */
   #if (CHIPSET == 12)
     // Unmask ABB ext interrupt
     F_INTH_ENABLE_ONE_IT(C_INTH_ABB_IRQ_IT);
   #else
     // Unmask external (ABB) interrupt
     IQ_Unmask(IQ_EXT);
   #endif
}
