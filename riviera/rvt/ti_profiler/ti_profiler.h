/************************************************************************************
* ti_profiler.h :  contains ti profiling and Nucleus Monitoring module header                              *
*																					*	
*																					*
*	Author: Philippe Martinez														*
*																					*
*	version: 1.1																	*
*																					*
*	Date: 07/16/2001																*
*   (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved		*		
************************************************************************************/


#ifndef _TI_PROFILER_H_
  #define _TI_PROFILER_H_

  #define INFINITE_LOOP               ( 1 )
  #define L1_TRACER		      ( 1 )  // Add debug traces into Layer 1 stream

  // Log buffer size in 32 bit words  (profiler only)
  #define PR_BUFFER_SIZE              ( 20000 )
  #define PR_BUFFER_THRESHOLD         ( PR_BUFFER_SIZE - 95 )


  // buffer OP code	(profiler only)
  #define C_OP_CODE_START_PR          ( 0xfffffff0 )
  #define C_OP_CODE_STOP_PR           ( 0xfffffffe )

  #define C_OP_CODE_BEGIN_BUFFER      ( 0xfffffff0 )
  #define C_OP_CODE_TDMA_BEGIN        ( 0xfffffff2 )
  #define C_OP_CODE_TDMA_END          ( 0xfffffff4 )
  #define C_OP_CODE_END_BUFFER        ( 0xfffffff8 )

  // Rx message ID code	(profiler only)
  #define TI_PROFILER_END_PROFILING   ( 0x0000 )
  #define TI_PROFILER_START_PROFILING ( 0x0001 )

  #define TI_PROFILER_OUTPUT_BUFFER   ( 0xffff )


  // Timer used	 (profiler only)
  #define PR_TIMER_NUM                ( 1 ) 

  #define PR_TIMER_RESET_VALUE        ( 21 )  // load value 50 us

  // define the length of cutted element buffer in order to
  // Generate rvt messages. (profiler only)
  #define TRACE_MESSAGE_SIZE	      ( 50 )  // x4 = n bytes

  // Nucleus monitoring buffer size (bytes) for rvt (Nucleus Monitoring only)
  #define NUC_MONITOR_BUFFER_SIZE     ( 240 )

  // Nucleus Monitoring Buffer (Nucleus Monitoring only)
  #define C_OP_CODE_NUC_MON_THREAD_IDS ( 0xff )
  #define C_OP_CODE_FN_LOG   		   ( 0x00 )
  #define C_OP_CODE_LISR_FLAG          ( 0xC0 )
  #define C_OP_CODE_THREAD_FLAG        ( 0x80 )
  #define C_OP_CODE_QUEUE_FLAG         ( 0x40 )

  // Rx message ID code (Nucleus Monitoring only)
  #define TI_NUC_MONITOR_OUTPUT_BUFFER ( 0xfff0 )
  #define TI_NUC_MONITOR_START         ( 0x0002 )
  #define TI_NUC_MONITOR_STOP          ( 0x0003 )

  // ID Thread NULL (Nucleus Monitoring only)
  #define NUC_MONITOR_ID_IDLE         ( 0xff )

  // Nuleus monitoring : Number max of component of same type
  // that can be monitored	(Nucleus Monitoring only)
  #define NUC_MONITOR_MAX_ID          ( 64 )

  // Define LISR doubled buffer Size (Nucleus Monitoring only)
  #define C_NUC_LISR_BUF_PG_NB        ( 2 )
  #define C_NUC_LISR_LOG_SZ           ( 5 )  // bytes

  // Must be strickly < NUC_MONITOR_BUFFER_SIZE	(Nucleus Monitoring only)
  #define C_NUC_MAX_LISR              ( 20 *  C_NUC_LISR_LOG_SZ )

  // Define LISR doubled buffer Size (Nucleus Monitoring only)
  #define C_NUC_THREAD_BUF_PG_NB        ( 2 )
  #define C_NUC_THREAD_LOG_SZ           ( 3 )  // bytes

  // Must be strickly < NUC_MONITOR_BUFFER_SIZE (Nucleus Monitoring only)
  #define C_NUC_MAX_THREAD              ( 30 *  C_NUC_THREAD_LOG_SZ )

  // Differents state of profiling
  typedef enum 
    {               
      PROFILER_STOPPED,
      PROFILER_RUNNING,
      PROFILER_FREEZED,
      PROFILER_TO_BE_RAN,
      PROFILER_TO_BE_UNFREEZED,
      PROFILER_TO_BE_STOPPED
    } T_PROFILER_STATE;

  // Differents state of nucleus monitoring	(Nucleus Monitoring only)
  typedef enum 
    {               
      NUC_MONITOR_STOPPED,
      NUC_MONITOR_RUNNING,
	  NUC_MONITOR_TO_BE_RUN,
	  NUC_MONITOR_TO_BE_STARTED,
	  NUC_MONITOR_TO_BE_STOP,
	  NUC_MONITOR_WAITING
	} T_NUC_MONITOR_STATE;

  typedef enum 
    {               
      LOGGING,
      FLUSH
    } T_PROFILER_BUFFER_STATE;

  typedef enum 
    {               
      NOT_ACTIVE,
      ACTIVE_BY_TI,
      ACTIVE_BY_EXT,
      ACTIVE_BY_HOST
    } T_PROFILER_HANDLER;

  typedef enum 
    {               
      NO_EVENT,
      NO_SLEEP_EVENT,
      EVENT2,
      EVENT3,
	  EVENT4
    } T_PROFILER_EVENTS;

  typedef struct 
    {
      SYS_UWORD16  msg_id;
      SYS_UWORD8   event_number;
	  SYS_UWORD8   arg1;
      SYS_UWORD8   *p_buffer;
      SYS_UWORD8   buffer_size;
    } T_PROFILER_MSG_UART;

  typedef struct 
    {
      T_RV_HDR     header;
      SYS_UWORD16  msg_id;
      SYS_UWORD8   event_number;
	  SYS_UWORD8   arg1;
      SYS_UWORD8   *p_buffer;
      SYS_UWORD8   buffer_size;
    } T_PROFILER_MSG;

#endif
                 
