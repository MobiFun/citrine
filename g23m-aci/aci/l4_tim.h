/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  L4_TIM
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
|  Purpose :  Definitions of timer indices
+----------------------------------------------------------------------------- 
*/ 


/*
 * Timer Indices
 */

#define SMI_TBACKLIGHT           0
#define SMI_TSLIDER              1
#define SMI_TSHIFTKEY            2
#define SMI_TUNIVERSAL           3

#define ACI_TRING                4
#define ACI_TMPTY                5
#define ACI_TFIT                 6
#define ACI_TDTMF                7

#if defined (FAX_AND_DATA)
#define FIT_RUN                  8
#endif

#define AOC_CALLTIMER            9
#define AOC_AOCTIMER             10
#define MAX_AOC_TIMER           7   /* should be changed to MAX_CALL_NR */


/* timers in between are reserved for AOC */
#define MFW_TIMER               (AOC_AOCTIMER+MAX_AOC_TIMER)

#define ACI_IPR                 (MFW_TIMER + 1)
#define ACI_TECT                (ACI_IPR + 1)
#define ACI_TDELAY              (ACI_TECT + 1)

#if defined FF_EOTD /*&& defined _SIMULATION_*/
#define ACI_LC_1                (ACI_TDELAY+1)
#define ACI_LC_2                (ACI_LC_1+1)
#define ACI_LC_3                (ACI_LC_1+2)
#define ACI_LC_4                (ACI_LC_1+3)
#define ACI_LC_5                (ACI_LC_1+4)
#endif

#ifndef FF_EOTD
#define ACI_REPEAT_HND          (ACI_TDELAY +1)
#else
#define ACI_REPEAT_HND          (ACI_LC_1+5)
#endif /* FF_EOTD */

#ifdef SIM_TOOLKIT
#define ACI_SAT_MAX_DUR_HND     (ACI_REPEAT_HND +1)
#define ACI_CNMA_TIMER_HANDLE   (ACI_SAT_MAX_DUR_HND+1)
#else 
#define ACI_CNMA_TIMER_HANDLE   (ACI_REPEAT_HND+1)
#endif /* SIM_TOOLKIT */


#ifdef VOCODER_FUNC_INTERFACE
#define ACI_VOCODER             (ACI_CNMA_TIMER_HANDLE+1)
#endif

#ifdef SIM_PERS
  #ifdef VOCODER_FUNC_INTERFACE
    #define ACI_UNLOCK_TIMER_HANDLE   (ACI_VOCODER+1)
  #else
    #define ACI_UNLOCK_TIMER_HANDLE   (ACI_CNMA_TIMER_HANDLE+1)
  #endif
#endif



#define ACI_REPEAT_1         5000   /* 5 sec for waiting for 1th redialling */
#define ACI_REPEAT_2_4       60000  /* 1 min for waiting for 2th to 4th redialling */
#define ACI_CNMA_TIMER_VALUE 15000  /* 15 sec. for waiting for the +CNMA */
#define ACI_DELAY_TIMER      5000   /* 5 sec delay for various REDIAL attempts */
#ifdef SIM_PERS
#define ACI_UNLOCK_TIMER      10000   /* 2 sec delay for various wrong unlock/unblock attempts */
#endif

