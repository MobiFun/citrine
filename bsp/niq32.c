/******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename       	: niq32.c

   Description    	: Nucleus IQ initializations

   Project        	: Drivers

   Author         	: proussel@ti.com  Patrick Roussel.

   Version number   : 1.25

   Date             : 08/22/03

   Previous delta 	: 12/19/00 14:24:51

*******************************************************************************/

#include "../include/config.h"
#include "../include/sys_types.h"

#include "inth.h"
#include "mem.h"
#include "iq.h"
#include "ulpd.h"
#include "armio.h"

#include "../serial/serialswitch.h"
#include "rtc/rtc_config.h"

#if CONFIG_INCLUDE_SIM
#include "sim.h"
#endif

#if 0

/* original maze of includes */

#if(OP_L1_STANDALONE == 0)
  #include "debug.cfg"
  #include "rv/rv_defined_swe.h"    
  #include "rtc/board/rtc_config.h"
#else
  #include "l1_macro.h"
  #include "l1_confg.h"
#endif

#if(OP_L1_STANDALONE == 0)
#include "swconfig.cfg"
#ifdef BLUETOOTH_INCLUDED
#include "btemobile.cfg"
#ifdef BT_CLK_REQ_INT
#include "board/bth_drv.h"
#endif
#endif
#endif


#if(L1_DYN_DSP_DWNLD == 1)
  #include "l1_api_hisr.h"
#endif
  
#if (OP_L1_STANDALONE == 0)
  #include "main/sys_types.h"
#else
  #include "sys_types.h"
#endif

#if (CHIPSET == 12)
  #include "sys_inth.h"
#else
  #include "inth/inth.h"
  #include "memif/mem.h"
  #if (OP_L1_STANDALONE == 1)
    #include "serialswitch_core.h"
  #else
    #include "uart/serialswitch.h"
  #endif

  #if (OP_L1_STANDALONE == 0)
    #include "sim/sim.h"
  #endif
#endif

#include "abb/abb_core_inth.h"	  // for External Interrupt
#define IQ_H
#include "inth/iq.h"
#include "ulpd/ulpd.h"
#if (BOARD == 34)
  #include "csmi/csmi.h"
#endif

#if (defined RVM_DAR_SWE) && (defined _GSM)
  extern void dar_watchdog_reset(void);
#endif

#if ((BOARD == 8) || (BOARD == 9) || (BOARD == 40) || (BOARD == 41) || (BOARD == 42) || (BOARD == 43) || (BOARD == 45))
#include "armio/armio.h"
  #if (OP_L1_STANDALONE == 0)
    #include "uart/uartfax.h"
  #endif
#endif

/* end of original include maze */
#endif

/* External declaration */
extern void GAUGING_Handler(void);
extern void TMT_Timer_Interrupt(void);
#if 0 //(OP_L1_STANDALONE == 1)
  extern void TM_Timer1Handler(void);
#endif
extern void kpd_key_handler(void);
extern void TP_FrameIntHandler(void);

#if 1 //(OP_L1_STANDALONE == 0)
  #if (defined RVM_MPM_SWE)
   extern void MPM_InterruptHandler(void);
  #endif

  #if (TI_PROFILER == 1)
    extern void ti_profiler_tdma_action(void);
  #endif

  #if(RF_FAM==35)
    extern void TSP_RxHandler(void);
  #endif

  extern void RTC_GaugingHandler(void);
  extern void RTC_ItTimerHandle(void);
  extern void RTC_ItAlarmHandle(void);
#endif

/* Global variables */
unsigned IQ_TimerCount1;   /* Used to check if timer is incrementing */
unsigned IQ_TimerCount2;   /* Used to check if timer is incrementing */
unsigned IQ_TimerCount;    /* Used to check if timer is incrementing */
unsigned IQ_DummyCount;    /* Used to check if dummy IT */
unsigned IQ_FrameCount;    /* Used to check if Frame IT TPU*/
unsigned IQ_GsmTimerCount; /* Used to check if GSM Timer IT */

/*--------------------------------------------------------------*/
/*  	irqHandlers                                             */
/*--------------------------------------------------------------*/
/* Parameters :none                                             */
/* Return     :	none                                            */
/* Functionality :  Table of interrupt handlers                 */
/* These MUST be 32-bit entries                                 */
/*--------------------------------------------------------------*/

SYS_FUNC irqHandlers[IQ_NUM_INT] = 
{
   IQ_TimerHandler,        /* Watchdog timer */
   IQ_TimerHandler1,       /* timer 1 */
   IQ_TimerHandler2,       /* timer 2 */
   IQ_Dummy,               /* AIRQ 3 */   
   IQ_FrameHandler,        /* TPU Frame It AIRQ 4 */
   IQ_Dummy,               /* AIRQ 5 */
#if CONFIG_INCLUDE_SIM //(OP_L1_STANDALONE == 0)
   SIM_IntHandler,         /* AIRQ 6 */
#else
   IQ_Dummy,               /* AIRQ 6 */
#endif
#if ((CHIPSET == 2) || (CHIPSET == 3))
   SER_uart_handler,       /* AIRQ 7 */
#elif ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
   SER_uart_modem_handler, /* AIRQ 7 */
#endif
#if 1 //((BOARD == 8) || (BOARD == 9) || (BOARD == 40) || (BOARD == 41))
// CC test 0316
   IQ_KeypadGPIOHandler,   /* AIRQ 8 */
// end
#else
   IQ_KeypadHandler,       /* AIRQ 8 */
#endif
   IQ_Rtc_Handler,         /* AIRQ 9 RTC Timer*/
#if ((CHIPSET == 2) || (CHIPSET == 3))
   IQ_RtcA_GsmTim_Handler, /* AIRQ 10 RTC ALARM OR ULPD GSM TIMER*/
#elif ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
   IQ_RtcA_Handler,        /* AIRQ 10 RTC ALARM */
#endif
   IQ_Gauging_Handler,     /* AIRQ 11 ULPD GAUGING */
   IQ_External,            /* AIRQ 12 */
   IQ_Dummy,               /* AIRQ 13 */
   IQ_Dummy,               /* DMA interrupt */
#if (CHIPSET == 4)
   IQ_Dummy,               /* LEAD */
   IQ_Dummy,               /* SIM card-detect fast interrupt */
   IQ_Dummy,               /* External fast interrupt */
   SER_uart_irda_handler,  /* UART IrDA interrupt */
   IQ_GsmTim_Handler       /* ULPD GSM timer */
#elif ((CHIPSET == 5) || (CHIPSET == 6))
   IQ_Dummy,               /* LEAD */
   IQ_Dummy,               /* SIM card-detect fast interrupt */
   IQ_Dummy,               /* External fast interrupt */
   SER_uart_irda_handler,  /* UART IrDA interrupt */
   IQ_GsmTim_Handler,      /* ULPD GSM timer */
   #if (BOARD == 34)
     IQ_IcrHandler32,        
   #else
   IQ_Dummy,               /* Not mapped interrupt */
   #endif
   IQ_Dummy,               /* Not mapped interrupt */
   IQ_Dummy,               /* Not mapped interrupt */
   IQ_Dummy,               /* Not mapped interrupt */
   IQ_Dummy                /* GEA interrupt */
#elif ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
#if (L1_DYN_DSP_DWNLD == 1)
    IQ_ApiHandler,         /* LEAD */ 
#else
   IQ_Dummy,               /* LEAD */
#endif                     
   IQ_Dummy,               /* SIM card-detect fast interrupt */
   IQ_Dummy,               /* External fast interrupt */
   SER_uart_irda_handler,  /* UART IrDA interrupt */
   IQ_GsmTim_Handler,      /* ULPD GSM timer */
   IQ_Dummy                /* GEA interrupt */
#elif (CHIPSET == 9)
   IQ_Dummy,               /* LEAD */
   IQ_Dummy,               /* SIM card-detect fast interrupt */
   IQ_Dummy,               /* External fast interrupt */
   SER_uart_irda_handler,  /* UART IrDA interrupt */
   IQ_GsmTim_Handler,      /* ULPD GSM timer */
   IQ_Dummy,               /* Not mapped interrupt */
   IQ_Dummy,               /* Not mapped interrupt */
   IQ_Dummy,               /* Not mapped interrupt */
   IQ_Dummy,               /* Not mapped interrupt */
   IQ_Dummy                /* Reserved */
#else
   IQ_Dummy                /* LEAD */
#endif
};   

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
  /*--------------------------------------------------------------*/
  /*  	fiqHandlers                                               */
  /*--------------------------------------------------------------*/
  /* Parameters :none                                             */
  /* Return     :none                                             */
  /* Functionality :  Table of interrupt handlers                 */
  /* These MUST be 32-bit entries                                 */
  /*--------------------------------------------------------------*/

  SYS_FUNC fiqHandlers[IQ_NUM_INT] = 
  {
    IQ_Dummy,          /* Watchdog timer */
    IQ_Dummy,          /* timer 1 */
    IQ_Dummy,          /* timer 2 */
  #if ((OP_L1_STANDALONE == 0) && (RF_FAM == 35))
    TSP_RxHandler,     /* 3 TSP  */
  #else
    IQ_Dummy,          /* AIRQ 3 */   
  #endif
    IQ_Dummy,          /* TPU Frame It AIRQ 4 */
    IQ_Dummy,          /* AIRQ 5 */
    IQ_Dummy,          /* AIRQ 6 */
    IQ_Dummy,          /* AIRQ 7 */
    IQ_Dummy,          /* AIRQ 8 */
    IQ_Dummy,          /* AIRQ 9 RTC Timer */
    IQ_Dummy,          /* AIRQ 10 RTC ALARM */
    IQ_Dummy,          /* AIRQ 11 ULPD GAUGING */
    IQ_Dummy,          /* AIRQ 12 */
    IQ_Dummy,          /* AIRQ 13 Spi Tx Rx interrupt */
    IQ_Dummy,          /* DMA interrupt */
    IQ_Dummy,          /* LEAD */
  #if CONFIG_INCLUDE_SIM //(OP_L1_STANDALONE == 0)
      SIM_CD_IntHandler, /* SIM card-detect fast interrupt */
  #else
      IQ_Dummy,          /* SIM card-detect fast interrupt */
  #endif
    IQ_Dummy,          /* External fast interrupt */
    IQ_Dummy,          /* UART_IRDA interrupt */
  #if (CHIPSET == 4)
    IQ_Dummy           /* ULPD GSM timer */
  #elif ((CHIPSET == 5) || (CHIPSET == 6))
    IQ_Dummy,          /* ULPD GSM timer */
    IQ_Dummy,          /* Not mapped interrupt */
    IQ_Dummy,          /* Not mapped interrupt */
    IQ_Dummy,          /* Not mapped interrupt */
    IQ_Dummy,          /* Not mapped interrupt */
    IQ_Dummy           /* GEA interrupt */
  #elif ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
    IQ_Dummy,          /* ULPD GSM timer */
    IQ_Dummy           /* GEA timer */
  #elif (CHIPSET == 9)
    IQ_Dummy,        /* ULPD GSM timer */
    IQ_Dummy,        /* Not mapped interrupt */
    IQ_Dummy,        /* Not mapped interrupt */
    IQ_Dummy,        /* Not mapped interrupt */
    IQ_Dummy,        /* Not mapped interrupt */
    IQ_Dummy         /* Reserved */
  #endif
};   
#endif

/*--------------------------------------------------------------*/
/*  IQ_Gauging_Handler				                */
/*--------------------------------------------------------------*/
/* Parameters :none						*/
/* Return     :	none						*/
/* Functionality :   Handle unused interrupts 			*/
/*--------------------------------------------------------------*/
void IQ_Gauging_Handler(void)
{
#if CONFIG_INCLUDE_L1
   GAUGING_Handler();
#if CONFIG_GSM
   RTC_GaugingHandler();
#endif
#endif
} 


/*--------------------------------------------------------------*/
/*  IQ_External							                        */
/*--------------------------------------------------------------*/
/* Parameters : none						                    */
/* Return     :	none						                    */
/* Functionality : Handle External IRQ mapped on ABB.           */
/*--------------------------------------------------------------*/
void IQ_External(void)
{
   #if (CHIPSET == 12)
      // Mask external interrupt 12
      F_INTH_DISABLE_ONE_IT(C_INTH_ABB_IRQ_IT);
   #else
     // Mask external interrupt 12
     IQ_Mask(IQ_EXT);
   #endif

  // The external IRQ is mapped on the ABB interrupt.
  // The associated HISR ABB_Hisr is activated on reception on the external IRQ.
#if CONFIG_INCLUDE_L1
  if(Activate_ABB_HISR())
  {
   #if (CHIPSET == 12)
      F_INTH_ENABLE_ONE_IT(C_INTH_ABB_IRQ_IT);
   #else
     // Mask external interrupt 12
     IQ_Unmask(IQ_EXT);
   #endif
  }
#endif
}

/*--------------------------------------------------------------*/
/*  IQ_Dummy							*/
/*--------------------------------------------------------------*/
/* Parameters :none						*/
/* Return     :	none						*/
/* Functionality :   Handle unused interrupts 			*/
/*--------------------------------------------------------------*/
void IQ_Dummy(void)
{
    IQ_DummyCount++;
}   

/*--------------------------------------------------------------*/
/*  IQ_RTCHandler						*/
/*--------------------------------------------------------------*/
/* Parameters :none						*/
/* Return     :	none						*/
/* Functionality :    Handle RTC Time interrupts 		*/
/*--------------------------------------------------------------*/

void IQ_Rtc_Handler(void)
{
#if CONFIG_GSM //(OP_L1_STANDALONE == 0)
  RTC_ItTimerHandle();
#endif
}

/*--------------------------------------------------------------*/
/*  IQ_RtcA_GsmTim_Handler					*/
/*--------------------------------------------------------------*/
/* Parameters :none						*/
/* Return     :	none						*/
/* Functionality :    Handle RTC ALARM or GAUGING interrupts    */
/*--------------------------------------------------------------*/

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
void IQ_RtcA_Handler(void)
{
  #if 1 //(OP_L1_STANDALONE == 0)
    /* INTH_DISABLEONEIT(IQ_RTC_ALARM); *//* RTC ALARM IT  */
    if ( (* (SYS_WORD8 *) RTC_STATUS_REG) & RTC_ALARM )
      RTC_ItAlarmHandle();
  #endif
}

  void IQ_GsmTim_Handler(void)
  {

    if ( (* (SYS_UWORD16 *) ULPD_GSM_TIMER_IT_REG) & ULPD_IT_TIMER_GSM  )
    {
      // it is GSM Timer it.....
      IQ_GsmTimerCount++;
    }
  }
#else
void IQ_RtcA_GsmTim_Handler(void)
{
  #if (OP_L1_STANDALONE == 0)
   if ( (* (SYS_UWORD16 *) ULPD_GSM_TIMER_IT_REG) & ULPD_IT_TIMER_GSM  )
   {
     // it is GSM Timer it.....
     IQ_GsmTimerCount++;
   }
   else
   {
     /* INTH_DISABLEONEIT(IQ_RTC_ALARM); *//* RTC ALARM IT  */
     if ( (* (SYS_WORD8 *) RTC_STATUS_REG) & RTC_ALARM )
        RTC_ItAlarmHandle();
   }   
  #endif
}
#endif

/*--------------------------------------------------------------*/
/*  IQ_TimerHandler						*/
/*--------------------------------------------------------------*/
/* Parameters :none						*/
/* Return     :	none						*/
/* Functionality :    Handle Timer interrupts 			*/
/*--------------------------------------------------------------*/
 void IQ_TimerHandler(void)
{
   IQ_TimerCount++;
   TMT_Timer_Interrupt();
   #if 0 //(defined RVM_DAR_SWE) && (defined _GSM)
     dar_watchdog_reset();
   #endif
}
 
/*--------------------------------------------------------------*/
/*  IQ_FramerHandler						*/
/*--------------------------------------------------------------*/
/* Parameters :none						*/
/* Return     :	none						*/
/* Functionality :    Handle Timer interrupts 			*/
/*--------------------------------------------------------------*/
 void IQ_FrameHandler(void)
{
   IQ_FrameCount++;
   TMT_Timer_Interrupt();
#if CONFIG_INCLUDE_L1
   TP_FrameIntHandler();
#endif
   #if (OP_L1_STANDALONE == 0)
     #if (TI_PROFILER == 1)
       // TDMA treatment for profiling buffer
       ti_profiler_tdma_action();
     #endif
   #endif
}

/*--------------------------------------------------------------*/
/*  IQ_TimerHandler1						*/
/*--------------------------------------------------------------*/
/* Parameters :none						*/
/* Return     :	none						*/
/* Functionality :    Handle Timer 1 interrupts 			*/
/*--------------------------------------------------------------*/
void IQ_TimerHandler1(void)
{
  IQ_TimerCount1++;  
  #if (OP_L1_STANDALONE == 1)
    TM_Timer1Handler();
  #endif
}
 
/*--------------------------------------------------------------*/
/*  IQ_TimerHandler2						*/
/*--------------------------------------------------------------*/
/* Parameters :none						*/
/* Return     :	none						*/
/* Functionality :    Handle Timer 2 interrupts 			*/
/*--------------------------------------------------------------*/
 void IQ_TimerHandler2(void)
{
  IQ_TimerCount2++;  
  #if !CONFIG_INCLUDE_L1
    TMT_Timer_Interrupt();
  #endif
}

#if (L1_DYN_DSP_DWNLD == 1)
/*-------------------------------------------------------*/
/* IQ_ApiHandler()                                       */
/*-------------------------------------------------------*/
/* Parameters : none                                     */
/* Return     : none                                     */
/* Functionality : API int management                    */
/*-------------------------------------------------------*/
void IQ_ApiHandler(void)
{
  l1_api_handler();
} /* IQ_ApiHandler() */
#endif


/*--------------------------------------------------------------*/
/*  IQ_IRQ_isr							*/
/*--------------------------------------------------------------*/
/* Parameters :none						*/
/* Return     :	none						*/
/* Functionality :    HHandle IRQ interrupts  			*/
/*--------------------------------------------------------------*/
void IQ_IRQ_isr(void)
{
  irqHandlers[((* (SYS_UWORD16 *) INTH_B_IRQ_REG) & INTH_SRC_NUM)]();  /* ACK IT */
  * (SYS_UWORD16 *) INTH_CTRL_REG |= (1 << INTH_IRQ);	/* valid next IRQ */
}

/*--------------------------------------------------------------*/
/*  IQ_FIQ_isr							*/
/*--------------------------------------------------------------*/
/* Parameters :none						*/
/* Return     :	none						*/
/* Functionality :   Handle FIQ interrupts  			*/
/*--------------------------------------------------------------*/
void IQ_FIQ_isr(void)
{
  #if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
    fiqHandlers[((* (SYS_UWORD16 *) INTH_B_FIQ_REG) & INTH_SRC_NUM)]();  /* ACK IT */
  #endif
    * (SYS_UWORD16 *) INTH_CTRL_REG |= (1 << INTH_FIQ);	/* valid next FIQ */
}   

/*--------------------------------------------------------------*/
/*  IQ_KeypadGPIOHandler                                        */
/*--------------------------------------------------------------*/
/* Parameters    : none                                             */
/* Return        : none                                            */
/* Functionality : Handle keypad and GPIO interrupts          */
/*--------------------------------------------------------------*/
// CC test 0316
//#include "rvm/rvm_use_id_list.h"
//#include "rvf/rvf_api.h"
//static char debug_buffer[50];
// end

void IQ_KeypadGPIOHandler(void)
{

 #if 0 //(OP_L1_STANDALONE == 0)
    /*
     * GPIO interrupt must be checked before the keypad interrupt. The GPIO
     * status bit is reset when the register is read.
     */
     
    if (AI_CheckITSource (ARMIO_GPIO_INT))

// CC test 0315
{	       
        AI_MaskIT (ARMIO_MASKIT_GPIO);
//sprintf(debug_buffer, "GPIO_Interrupt");
//rvf_send_trace(debug_buffer, 40, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVT_USE_ID);
        AI_UnmaskIT(ARMIO_MASKIT_GPIO);       //0x0002  
// end
/*
    #ifdef RVM_MPM_SWE
      // check if the SWE has been started
       MPM_InterruptHandler ();
    #elif BT_CLK_REQ_INT

      BT_DRV_ClkReqInterruptHandler( );
    #else
      UAF_DTRInterruptHandler ();
    #endif
*/    
}
    if (AI_CheckITSource (ARMIO_KEYPAD_INT))
    {
// CC test 0316
//sprintf(debug_buffer, "Key_Interrupt");
//rvf_send_trace(debug_buffer, 40, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RVT_USE_ID);
// end
      kpd_key_handler ();
    }

 #endif
}   
