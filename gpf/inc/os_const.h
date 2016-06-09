/* 
+------------------------------------------------------------------------------
|  File:       os_const.h
+------------------------------------------------------------------------------
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
|  Purpose :  Constants for the Nucleus OS-Layer
+----------------------------------------------------------------------------- 
*/ 

#ifndef OS_CONST_H
#define OS_CONST_H

/*==== INCLUDES =============================================================*/

#include "gpfconf.h"

/*==== CONSTS ===============================================================*/

#define PTR_OVERHEAD       8

#ifdef MEMORY_SUPERVISION
 #define PPM_CHKOVERHEAD  4
 #define PPM_OVERHEAD     (4+PPM_CHKOVERHEAD)
 #define PPM_IDX_OFFSET   0
#else
 #define PPM_CHKOVERHEAD  0
 #define PPM_OVERHEAD     0
#endif

#ifdef NU_DEBUG
 #define PT_CHKOVERHEAD   4
#else
 #define PT_CHKOVERHEAD   0
#endif

#define PT_OVERHEAD      (PTR_OVERHEAD+PPM_OVERHEAD)
#define POOL_OVERHEAD    0

#define PPM_OFFSET       ((PPM_OVERHEAD-PPM_CHKOVERHEAD)/sizeof(ULONG))

#define SUSPEND_ONE_TICK           1
#define TDMA_FRAME_DURATION    4.615
#define WIN32_TIMER_TICK          50

#define NO_WAIT_CHECK    0
#define WAIT_CHECK       1

#define OS_SUSPEND       0xffffffffUL  /*NU_SUSPEND*/
#define OS_NO_SUSPEND    0             /*NU_NO_SUSPEND*/

#define OS_FOREVER       0xffffffffUL  /*NU_SUSPEND*/

#define TIME_TO_TICK_TDMA_FRAME_MULTIPLIER   14199
#define TICK_TO_TIME_TDMA_FRAME_MULTIPLIER   302483
#define TIME_TO_TICK_10MS_MULTIPLIER         6554
#define TICK_TO_TIME_10MS_MULTIPLIER         655319

#ifdef _TARGET_
 #define TIME_TO_SYSTEM_TICKS(time)  (((((time)&0xffff)*os_time_to_tick_multiplier+0x8000)>>16)\
                                      +(((time)>>16)*os_time_to_tick_multiplier))
 #define SYSTEM_TICKS_TO_TIME(ticks) (((((ticks)&0xfff)*os_tick_to_time_multiplier+0x8000)>>16)\
                                      +((((ticks)>>12)*os_tick_to_time_multiplier)>>4))
#else
 #define TIME_TO_SYSTEM_TICKS(Time)   ((Time)/WIN32_TIMER_TICK)
 #define SYSTEM_TICKS_TO_TIME(Ticks)  ((Ticks)*WIN32_TIMER_TICK)
#endif

#define OS_QUEUE_ENTRY_SIZE(E)      (((E)*(sizeof(OS_QDATA)+sizeof(void*)) + (OS_MAX_PRIORITY * (((E)+1)*sizeof(void*)))))

#define POOL_SIZE(n,s)              ((n*(s+PT_CHKOVERHEAD+PT_OVERHEAD)))

/* to avoid changes in all xxxconst.h files even if they do not use events */
#ifndef MAX_EVENT_GROUPS
  #define MAX_EVENT_GROUPS  1
#endif

/*==== TYPES =================================================================*/


/*==== EXPORTS ===============================================================*/


#endif /* OS_CONST_H */
