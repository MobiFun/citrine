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
/* Filename          : abb_core_inth.c                                        */
/*                                                                            */
/* Description       : Functions to manage the ABB device interrupt.          */
/*                     The Serial Port Interface is used to connect the TI    */
/*                     Analog BaseBand (ABB).    			      */
/*		       It is assumed that the ABB is connected as the SPI     */
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
#include "abb_core_inth.h"

#if (OP_L1_STANDALONE == 0)
  #include "../../riviera/rv/rv_defined_swe.h"     // for RVM_PWR_SWE
#endif

#if (OP_L1_STANDALONE == 1)
  #include "../../L1/include/l1_types.h"
#endif

#include "../iq.h"

#include "../../gpf/inc/cust_os.h"
#include "../../L1/include/l1_signa.h"
#include "abb.h"

#if(OP_L1_STANDALONE == 0)
  #include "../../riviera/rvm/rvm_use_id_list.h"   // for SPI_USE_ID
  #include "spi_env.h"
  #include "spi_process.h"       // for ABB_EXT_IRQ_EVT
#if 0	// FreeCalypso
  #include "power/power.h"	 
#endif
#endif	/* (OP_L1_STANDALONE == 0) */



// Size of the HISR stack associated to the ABB interrupt
#define ABB_HISR_STACK_SIZE   (512)

static NU_HISR ABB_Hisr;
static char ABB_HisrStack[ABB_HISR_STACK_SIZE];



/*-----------------------------------------------------------------------*/
/* Create_ABB_HISR()                                                     */
/*                                                                       */
/* This function is called from Layer1 during initialization process 	 */
/* to create the HISR associated to the ABB External Interrupt.		 */
/*                                                                       */
/*-----------------------------------------------------------------------*/    
void Create_ABB_HISR(void)
{
  // Fill the entire stack with the pattern 0xFE
  memset (ABB_HisrStack, 0xFE, sizeof(ABB_HisrStack));

  // Create the HISR which is called when an ABB interrupt is received.
  NU_Create_HISR(&ABB_Hisr, "EXT_HISR", EXT_HisrEntry, 2, ABB_HisrStack, sizeof(ABB_HisrStack)); // lowest prty
}




/*-----------------------------------------------------------------------*/
/* Activate_ABB_HISR()                                                   */
/*                                                                       */
/* This function is called from the interrupt handler to activate        */
/* the HISR associated to the ABB External Interrupt.  			 */
/*                                                                       */
/*-----------------------------------------------------------------------*/    
SYS_BOOL Activate_ABB_HISR(void)
{
  if(NU_SUCCESS != NU_Activate_HISR(&ABB_Hisr))
  {
     return 1;
  }
  return 0;
}



/*-----------------------------------------------------------------------*/
/* EXT_HisrEntry()                                                       */
/*                                                                       */
/* This function is called when an ABB interrupt (external interrupt) 	 */
/* is received. 							 */
/* In a "L1_STANDALONE" environment, this IT is related to ADC.          */
/* In a complete system, this IT can have several causes. In that case,  */
/* it sends a message to the SPI task, to handle the IT in calling a	 */
/* callback function.                                                    */
/*                                                                       */
/*-----------------------------------------------------------------------*/    
void EXT_HisrEntry(void)
{
#if (OP_L1_STANDALONE == 1)
  // New code in order to test the ADC with the L1 standalone 
  UWORD16 data ;
  xSignalHeaderRec *adc_msg;

  // Call to the low-level driver function to read the interrupt status register.
  data = ABB_Read_Register_on_page(PAGE1, ITSTATREG);

  if(data & ADCEND_IT_STS)	// end of ADC
  {
    adc_msg = os_alloc_sig(sizeof(T_CST_ADC_RESULT));
    if(adc_msg != NU_NULL)
    {
      adc_msg->SignalCode = CST_ADC_RESULT;
	  ABB_Read_ADC(&((T_CST_ADC_RESULT *)(adc_msg->SigP))->adc_result[0]);
      os_send_sig(adc_msg, RRM1_QUEUE);
    }
  }

  #if (CHIPSET == 12)
    // Unmask ABB ext interrupt
    F_INTH_ENABLE_ONE_IT(C_INTH_ABB_IRQ_IT);
  #else
    // Unmask external (ABB) interrupt
    IQ_Unmask(IQ_EXT);
  #endif
#endif

#if (OP_L1_STANDALONE == 0)
  T_RV_HDR *msgPtr;

  if(SPI_GBL_INFO_PTR != NULL)
  {
     if(SPI_GBL_INFO_PTR->SpiTaskReady != FALSE)
     {
        if(rvf_get_buf (SPI_GBL_INFO_PTR->prim_id, sizeof (T_RV_HDR),(void **) &msgPtr) == RVF_RED)
        {
           rvf_send_trace ("SPI ERROR: ABB IQ External not possible. Reason: Not enough memory",
                           66,
                           NULL_PARAM,
                           RV_TRACE_LEVEL_ERROR,
                           SPI_USE_ID);

           /* Unmask External interrupt */
           #if (CHIPSET == 12)
             // Unmask ABB ext interrupt
             F_INTH_ENABLE_ONE_IT(C_INTH_ABB_IRQ_IT);
           #else
             // Unmask external (ABB) interrupt
             IQ_Unmask(IQ_EXT);
           #endif
        }
        else	 // enough memory => normal processing : a message is sent to the SPI task.
        {
           msgPtr->msg_id        = ABB_EXT_IRQ_EVT;
           msgPtr->dest_addr_id  = SPI_GBL_INFO_PTR->addr_id;
           msgPtr->callback_func = (CALLBACK_FUNC) spi_abb_read_int_reg_callback;

           rvf_send_msg (SPI_GBL_INFO_PTR->addr_id, msgPtr);
        }
     }
     else	  //  SpiTaskReady is false
     {
        rvf_send_trace("ABB IQ External not possible. Reason: SPI Task not ready",56, NULL_PARAM, 
                       RV_TRACE_LEVEL_ERROR, SPI_USE_ID);

        /* Unmask External interrupt */
        #if (CHIPSET == 12)
          // Unmask ABB ext interrupt
          F_INTH_ENABLE_ONE_IT(C_INTH_ABB_IRQ_IT);
        #else
          // Unmask external (ABB) interrupt
          IQ_Unmask(IQ_EXT);
        #endif
     }
  }
  else	  // SPI_GBL_INFO_PTR = NULL
  {
     rvf_send_trace("ABB IQ External ERROR. Reason: SPI task not started",51, NULL_PARAM, 
                    RV_TRACE_LEVEL_ERROR, SPI_USE_ID);

     /* Unmask External interrupt */
     #if (CHIPSET == 12)
       // Unmask ABB ext interrupt
       F_INTH_ENABLE_ONE_IT(C_INTH_ABB_IRQ_IT);
     #else
       // Unmask external (ABB) interrupt
       IQ_Unmask(IQ_EXT);
     #endif
  }
#endif
}
