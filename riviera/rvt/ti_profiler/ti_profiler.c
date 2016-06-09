/************************************************************************************
* ti_profiler.c :  contains ti profiling module function                            *
*																					*	
*																					*
*	Author: Philippe Martinez														*
*																					*
*	version: 1.0																	*
*																					*
*	Date: 07/16/2001																*
*   (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved		*		
************************************************************************************/

#include "config/debug.cfg"

#if (TI_PROFILER == 1) || (TI_NUC_MONITOR == 1)

  /*-------------------------------------------------------*/
  /* include files                                         */
  /*-------------------------------------------------------*/

  #include <string.h>

  #include "main/sys_types.h"

  #include "nucleus.h" 
  #include "qu_defs.h" 
  #include "tc_defs.h" 

  #include "rvf/rvf_api.h"
  #include "rvt/rvt_gen.h"
  #include "rvt/rvt_def_i.h"

  #include "mem.h"
  #include "iq.h"

  #include "rvt/ti_profiler/ti_profiler.h"

  #include "timer.h"       
  #include "inth.h"

  /*-------------------------------------------------------*/
  /* Global instances                                      */
  /*-------------------------------------------------------*/

  // task variable instance
  T_RVF_MB_ID   prof_mb_id;
  T_RVT_USER_ID prof_trace_id;
  T_RVF_ADDR_ID prof_addr_id;
  
  /*-------------------------------------------------------*/
  /* internal prototypes                                   */
  /*-------------------------------------------------------*/

  void     ti_prf_init( void );
  T_RV_RET ti_prf_core(void);
  void     ti_profiler_trace_rx_callback( T_RVT_BUFFER trace_msg, UINT16 trace_msg_size );
  #if (L1_TRACER == 1)
    extern void SendTraces(char *s);
  #endif
   
#endif


#if (TI_PROFILER == 1)

  /*-------------------------------------------------------*/
  /* Global instances                                      */
  /*-------------------------------------------------------*/

  // Internal state of profiling
  T_PROFILER_STATE profiler_state = PROFILER_STOPPED;


  // Buffer that contains the Profiling samples.
  //  Structure (SYS_UWORD32 buffer):
  //     C_OP_CODE_TDMA_BEGIN
  //     PC Samples ......
  //     C_OP_CODE_TDMA_END
  //     ......
  //     C_OP_CODE_TDMA_BEGIN
  //     PC Samples ......
  //     C_OP_CODE_TDMA_END

  SYS_UWORD32   pr_buffer[PR_BUFFER_SIZE];
  // pointer on pr_buffer
  SYS_UWORD32   pos_pr_buffer    = 0;
  SYS_UWORD32   pr_buffer_length = PR_BUFFER_SIZE;

  T_PROFILER_BUFFER_STATE ti_profiler_buffer_state = LOGGING;
  T_PROFILER_HANDLER      ti_profiler_active       = NOT_ACTIVE;
 
  // hisr variable instance
  NU_HISR       ti_profiler_cb;
  SYS_UWORD8    ti_profiler_hisr_stack[500];

   // Event variables
   // NO_SLEEP_EVENT
   SYS_UWORD32   ti_profiler_nb_sleep_call         = 0;
   SYS_UWORD32   ti_profiler_nb_sleep_counter_ref  = 0;
 
   T_PROFILER_EVENTS    ti_profiler_event_state = NO_EVENT;
  
  /*-------------------------------------------------------*/
  /* internal prototypes                                   */
  /*-------------------------------------------------------*/

  void ti_profiler_start( BOOL enable );
  void ti_profiler_handler( SYS_UWORD32 programCounter );
  void ti_profiler_freeze( void );
  void ti_profiler_unfreeze( void );
  void ti_profiler_tdma_action( void );
  void ti_profiler_init_profiler_module( void );
  void ti_profiler_hisr( void );

  void l1_profiler_open( void );
  void l1_profiler_close( void );
  void l1_profiler_flush( void );

  /*-------------------------------------------------------*/
  /* external prototypes                                   */
  /*-------------------------------------------------------*/
  
  SYS_UWORD32 l1_get_next_absolute_fn( void );
   
  /*-------------------------------------------------------*/
  /* ti_profiler_start()                                   */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description: Start or Stop the timer                  */
  /* ------------                                          */
  /* parameters                                            */
  /*  enable = 1 => Unmask + start the timer 1             */
  /*    Initialize buffer.								   */
  /*  enable = 0 => mask + stop the timer 1                */
  /*    Close Buffer.									   */
  /*-------------------------------------------------------*/
 
  void ti_profiler_start( BOOL enable )
  {
    if( enable ) 
    {   
      TM_EnableTimer( PR_TIMER_NUM );
      IQ_Unmask( IQ_TIM1 );
      IQ_InitLevel( IQ_TIM1, FIQ, 0, 1 );

      TM_ResetTimer( PR_TIMER_NUM, PR_TIMER_RESET_VALUE, 1, 0 );
      TM_StartTimer( PR_TIMER_NUM );
    }
    else
    {
      IQ_Mask( IQ_TIM1 );
      IQ_InitLevel( IQ_TIM1, 0, 0, 1 );
      TM_StopTimer( PR_TIMER_NUM );
    }
  } // ti_profiler_start


  /*-------------------------------------------------------*/
  /* ti_profiler_handler()                                 */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  Store the Program counter in a static buffer	       */
  /*  If buffer is full, deactivate the profiling and      */
  /*  activate the ti_profiler_hisr();                     */
  /*-------------------------------------------------------*/
 
  void ti_profiler_handler( SYS_UWORD32 programCounter )
  {
 
    // Store the program Counter in log buffer 
    pr_buffer[ pos_pr_buffer++ ] = programCounter; 
 
  } // ti_profiler_handler


  /*-------------------------------------------------------*/
  /* ti_profiler_freeze()  ti_profiler_unfreeze()          */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  Function to suspend/resume the profiling             */
  /*-------------------------------------------------------*/
 
  void ti_profiler_freeze()
  {
    TM_StopTimer( PR_TIMER_NUM );
  } // ti_profiler_freeze

  void ti_profiler_unfreeze()
  {
    TM_StartTimer( PR_TIMER_NUM );  
  } // ti_profiler_unfreeze

  /*-------------------------------------------------------*/
  /* ti_profiler_init_profiler_module()                    */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description: Initialize profiling task and hisr       */
  /* ------------                                          */
  /* Note : this init will changed                         */
  /*                                                       */
  /*-------------------------------------------------------*/

  void ti_profiler_init_profiler_module( void )
  {
    // Fill the entire stack with the pattern 0xFE
    memset( ti_profiler_hisr_stack, 0xFE, sizeof( ti_profiler_hisr_stack ) );

    // Create the HISR which is called from power interrupts
    NU_Create_HISR( &ti_profiler_cb, 
                    "TI PROF", 
                    ti_profiler_hisr, 
                    2,
                    ti_profiler_hisr_stack, 
                    sizeof( ti_profiler_hisr_stack ) ); // lowest prty

  } // ti_profiler_init_profiler_module


  /*-------------------------------------------------------*/
  /* ti_profiler_hisr() High Interrupt service routine     */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  This hisr stands as relay for the profiler LISR.     */
  /*  It sends a message to the profiler task to start the */
  /*   buffer extraction.                                  */
  /*-------------------------------------------------------*/

  void ti_profiler_hisr( void )
  {	
    T_PROFILER_MSG * msg;
	 
    //send a message to the ti profiler task
	if( rvf_get_buf( prof_mb_id, sizeof( T_PROFILER_MSG ), ( T_RVF_BUFFER** ) &msg ) == RVF_GREEN )
	{
	  msg->msg_id = TI_PROFILER_OUTPUT_BUFFER;
	  rvf_send_msg( prof_addr_id, msg );
	}
  } // ti_profiler_hisr

  /*-------------------------------------------------------*/
  /* ti_profiler_tdma_action()                              */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*                                                       */
  /*                                                       */
  /*-------------------------------------------------------*/

  void ti_profiler_tdma_action(void)
  {
 
    switch( profiler_state )
    {
      case PROFILER_RUNNING : 
      {
	    SYS_UWORD8  output_buffer_condition = 0;

        // suspend the profiler
        //ti_profiler_freeze();
		ti_profiler_start( FALSE );

        if( ti_profiler_event_state == NO_SLEEP_EVENT )
		{
		  // Need to reach the event to output the buffer
		  if( ti_profiler_nb_sleep_call++ >= ti_profiler_nb_sleep_counter_ref )
		  {
			  output_buffer_condition   = 1;
			  ti_profiler_nb_sleep_call = 0;
			 // ti_profiler_buffer_state = FLUSH;			  
		  } // End if
		}
		else
		{
		  output_buffer_condition = 1;  
		} // End if 
 

        // Check if buffer threshold reached
   	    if( pos_pr_buffer >= PR_BUFFER_THRESHOLD )
	    {
          // change the status of the buffer
		  ti_profiler_buffer_state = LOGGING;

	      // Insert the End of Buffer OP code
	      pr_buffer[ pos_pr_buffer++ ] = C_OP_CODE_END_BUFFER;
          
		  // Specifie used length of buffer in SYS_UWORD32
          pr_buffer_length = pos_pr_buffer;
           
          if( output_buffer_condition == 1 )
          { 
            // Change profiler state to freezed
	        profiler_state = PROFILER_FREEZED;

	        // Activate the hisr in order to start the buffer output
	        NU_Activate_HISR( &ti_profiler_cb );
          }
	      else
	      {
		    // insert TDMA begin OP Code
            pr_buffer[ 0 ] = C_OP_CODE_BEGIN_BUFFER;	           
            pr_buffer[ 1 ] = C_OP_CODE_TDMA_BEGIN;	           
		    // Insert current fn
	        pr_buffer[ 2 ] = l1_get_next_absolute_fn();

		    // initialize buffer pointer
	        pos_pr_buffer = 3;

			ti_profiler_start( TRUE );
		  } // End if

        }
	    else
	    {
	      // insert OP Code begining of TDMA
	      pr_buffer[pos_pr_buffer++] = C_OP_CODE_TDMA_BEGIN;
		  // Insert current fn
	      pr_buffer[pos_pr_buffer++] = l1_get_next_absolute_fn();

	      // resume profiler
	      //ti_profiler_unfreeze();
		  ti_profiler_start( TRUE );
        } // End if


        break;
      }	// PROFILER_RUNNING

      case PROFILER_TO_BE_RAN : 
      {
		// insert TDMA begin OP Code
        pr_buffer[ 0 ] = C_OP_CODE_BEGIN_BUFFER;	           
        pr_buffer[ 1 ] = C_OP_CODE_TDMA_BEGIN;	           
		// Insert current fn
	    pr_buffer[ 2 ] = l1_get_next_absolute_fn();

		// initialize buffer pointer
	    pos_pr_buffer = 3;
        // start the profiler
        profiler_state = PROFILER_RUNNING;

	    // start the profiler
	    ti_profiler_start( TRUE );

        break;
      }	// PROFILER_TO_BE_RAN

      case PROFILER_TO_BE_STOPPED : 
      {
	    // stop the profiler
	    ti_profiler_start( FALSE );

        // Insert End Buffer Op Code.
        pr_buffer[pos_pr_buffer++] = C_OP_CODE_END_BUFFER; 
        profiler_state             = PROFILER_STOPPED;

	    // Activate the hisr in order to start the buffer output
	    NU_Activate_HISR( &ti_profiler_cb );

        // Returns to inactive
		ti_profiler_active = NOT_ACTIVE;

        break;
      } // PROFILER_TO_BE_STOPPED

      case PROFILER_TO_BE_UNFREEZED : 
      {
		// Init profiling buffer
		// insert TDMA begin OP Code
        pr_buffer[ 0 ] = C_OP_CODE_BEGIN_BUFFER;	           
        pr_buffer[ 1 ] = C_OP_CODE_TDMA_BEGIN;	           
		// Insert current fn
	    pr_buffer[ 2 ] = l1_get_next_absolute_fn();

		// initialize buffer pointer
	    pos_pr_buffer = 3;
        // Insert the TDMA Start Op Code
        profiler_state = PROFILER_RUNNING;

	    // resume profiler
	    //ti_profiler_unfreeze();
		ti_profiler_start( TRUE );

        break;
      }	// PROFILER_TO_BE_UNFREEZED

      default :
      {
        // nothing to do
        break;
      }	// default

    } // End switch
  
  } // ti_profiler_tdma_action

  /*-------------------------------------------------------*/
  /* ti_profiler_open()                                    */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  Start the profiling on the next TDMA occurence       */
  /*                                                       */
  /*-------------------------------------------------------*/

  void ti_profiler_open( void )
  {
    if( ti_profiler_active == NOT_ACTIVE )
	{

	} // End if
  } // ti_profiler_open

  /*-------------------------------------------------------*/
  /* ti_profiler_close()                                   */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  Stop the profiling on the next TDMA occurence        */
  /*                                                       */
  /*-------------------------------------------------------*/

  void ti_profiler_close( void )  
  {
    if( ti_profiler_active == NOT_ACTIVE )
	{
	   l1_profiler_close();
	} // End if
  } // ti_profiler_close

  /*-------------------------------------------------------*/
  /* l1_profiler_flush()                                   */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  freeze the profiling and flush the current profiling */
  /*   buffer. Profiling will restart after output of the  */
  /*   buffer.                                             */ 
  /*-------------------------------------------------------*/

  void ti_profiler_flush( void )
  {
    if( ti_profiler_active == NOT_ACTIVE )
	{
	  l1_profiler_flush();
	} // End if
  } // ti_profiler_flush

  // To be used only TI internal modules

  /*-------------------------------------------------------*/
  /* ti_profiler_open()                                    */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  Start the profiling on the next TDMA occurence       */
  /*                                                       */
  /*-------------------------------------------------------*/

  void l1_profiler_open( void )
  {

     if( ti_profiler_active != NOT_ACTIVE )
	 {
	   // stop profiler timer 
	   ti_profiler_start( FALSE );
       // go to idle
	   profiler_state = PROFILER_STOPPED;
     } // Endif

   	 // insert TDMA begin OP Code
     pr_buffer[ 0 ] = C_OP_CODE_BEGIN_BUFFER;	           
     pr_buffer[ 1 ] = C_OP_CODE_TDMA_BEGIN;	           

	 // Insert current fn
	 pr_buffer[ 2 ] = l1_get_next_absolute_fn();
								
	 // initialize buffer pointer
	 pos_pr_buffer = 3;

     // change state of profiler
     profiler_state = PROFILER_TO_BE_RAN; 

  } // ti_profiler_open

  /*-------------------------------------------------------*/
  /* ti_profiler_close()                                   */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  Stop the profiling on the next TDMA occurence        */
  /*                                                       */
  /*-------------------------------------------------------*/

  void l1_profiler_close( void )  
  {
	// stop the profiler
	ti_profiler_start( FALSE );

    // Insert TDMA_BOUNDARY Op Code.
    pr_buffer[pos_pr_buffer++] = C_OP_CODE_END_BUFFER; 
    profiler_state             = PROFILER_STOPPED;

	// Activate the hisr in order to start the buffer output
	NU_Activate_HISR( &ti_profiler_cb );

  } // ti_profiler_close

  /*-------------------------------------------------------*/
  /* l1_profiler_flush()                                   */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  freeze the profiling and flush the current profiling */
  /*   buffer. Profiling will restart after output of the  */
  /*   buffer.                                             */ 
  /*-------------------------------------------------------*/

  void l1_profiler_flush( void )
  {
    ti_profiler_buffer_state = FLUSH;
  } // ti_profiler_flush

#endif // PROFILER

#if (TI_NUC_MONITOR == 1)

  /*-------------------------------------------------------*/
  /* Global instances                                      */
  /*-------------------------------------------------------*/
  // management of monitoring buffer
   
  //    Internal state of nucleus monitoring
   
  T_NUC_MONITOR_STATE ti_nuc_monitor_state       = NUC_MONITOR_STOPPED;

  // pointers on current buffer
  SYS_UWORD8*    ti_nuc_monitor_current_buffer   = NULL;
  SYS_UWORD8     ti_nuc_monitor_current_position = 0;

  // Create static doubled bufferised LISR buffers (max 10 LISR by TDMA)
  SYS_UWORD8     ti_nuc_monitor_LISR_Buffer[C_NUC_LISR_BUF_PG_NB][C_NUC_MAX_LISR];
  SYS_UWORD8     ti_nuc_monitor_LISR_current_page = 0;
  SYS_UWORD8     ti_nuc_monitor_LISR_current_pos  = 0;
  SYS_UWORD8     ti_nuc_monitor_LISR_next_page    = 0;
  SYS_UWORD8     ti_nuc_monitor_LISR_next_pos     = 0;

  // Create static doubled bufferised LISR buffers (max 10 LISR by TDMA)
  SYS_UWORD8     ti_nuc_monitor_Thread_Buffer[C_NUC_THREAD_BUF_PG_NB][C_NUC_MAX_THREAD];
  SYS_UWORD8     ti_nuc_monitor_Thread_current_page = 0;
  SYS_UWORD8     ti_nuc_monitor_Thread_current_pos  = 0;
  SYS_UWORD8     ti_nuc_monitor_Thread_next_page    = 0;
  SYS_UWORD8     ti_nuc_monitor_Thread_next_pos     = 0;

  // hisr variable instance
  NU_HISR        ti_nuc_monitor_cb;
  SYS_UWORD8     ti_nuc_monitor_hisr_stack[ 500 ];

  // Array that store the ID <-> pointer of the Nucleus Components
  //SYS_UWORD32*   ti_nuc_monitor_thread_id[ NUC_MONITOR_MAX_ID ];
  SYS_UWORD32    ti_nuc_monitor_thread_id[ NUC_MONITOR_MAX_ID ];
  SYS_UWORD8     ti_nuc_monitor_thread_pos      = 0;

  //SYS_UWORD32*   ti_nuc_monitor_queue_id[ NUC_MONITOR_MAX_ID ];
  SYS_UWORD32    ti_nuc_monitor_queue_id[ NUC_MONITOR_MAX_ID ];
  SYS_UWORD8     ti_nuc_monitor_queue_pos       = 0;

  // LISR IRQ value
  SYS_UWORD8     IRQ_value;

  // Indicates the nucleus monitor part to switch Thread buffer page
  SYS_UWORD8     ti_nuc_monitor_change_thread_page = 0;

  // Indicates that the IDLE mode has been reached
  // (to be reset in LISR or Thread monitor function)
  SYS_UWORD8     ti_nuc_monitor_idle_reached  = 0;

  // debug
  SYS_UWORD32    ti_nuc_monitor_lost_messages = 0;

  /*-------------------------------------------------------*/
  /* internal prototypes                                   */
  /*-------------------------------------------------------*/

  void ti_nuc_monitor_tdma_action( void );
  void ti_nuc_monitor_init_module( void );
  void ti_nuc_monitor_hisr( void );
  void ti_nuc_monitor_check_ID_buffer( void );
  void ti_nuc_monitor_LISR_log( void );
  void ti_nuc_monitor_LISR_log_end( void );
  void ti_nuc_monitor_Thread_log( void );
  void ti_nuc_monitor_Queue_log( SYS_UWORD32* queue_pointer, SYS_UWORD8 status, SYS_UWORD8 Direction );
  void ti_nuc_monitor_sleep( void );

  T_RVT_RET ti_nuc_monitor_mem_alloc(T_RVT_USER_ID user_id, 
                                     T_RVT_MSG_LG buffer_lenght, 
                                     T_RVT_BUFFER * buff);

  /*-------------------------------------------------------*/
  /* external prototypes                                   */
  /*-------------------------------------------------------*/
  
  extern SYS_UWORD16  l1_get_actual_fn_mod42432( void );
  extern SYS_UWORD32* TCD_Current_Thread;

  /*-------------------------------------------------------*/
  /* ti_nuc_monitor_init_module()                          */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description: Initialize hisr                          */
  /* ------------                                          */
  /* Create hisr and initialize thread/queue structures.   */
  /*                                                       */
  /*-------------------------------------------------------*/

  void ti_nuc_monitor_init_module( void )
  {
    SYS_UWORD8	i;

    // Fill the entire stack with the pattern 0xFE
    memset( ti_nuc_monitor_hisr_stack, 0xFE, sizeof( ti_nuc_monitor_hisr_stack ) );

    // Create the HISR 
        NU_Create_HISR( &ti_nuc_monitor_cb, 
                    "TI MON", 
                    ti_nuc_monitor_hisr, 
                    2,
                    ti_nuc_monitor_hisr_stack, 
                    sizeof( ti_nuc_monitor_hisr_stack ) ); // lowest prty

  } // ti_nuc_monitor_init_module


  /*-------------------------------------------------------*/
  /* ti_nuc_monitor_hisr() High Interrupt service routine  */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  It sends a message to the profiler task to start the */
  /*   buffer extraction.                                  */
  /*-------------------------------------------------------*/

  void ti_nuc_monitor_hisr( void )
  {	
    SYS_UWORD8     remaining_bytes; 
	SYS_UWORD16    fn;
    T_PROFILER_MSG *msg;
	SYS_UWORD8     i;

    // Allocate a buffer first time
    if(( ti_nuc_monitor_state == NUC_MONITOR_TO_BE_RUN ) || 
	   ( ti_nuc_monitor_state == NUC_MONITOR_TO_BE_STARTED ))
	{
	  if((ti_nuc_monitor_mem_alloc (prof_trace_id, 
	                     NUC_MONITOR_BUFFER_SIZE, 
			             (T_RVT_BUFFER*) &ti_nuc_monitor_current_buffer)) 
		            == RVT_OK)
	  {
	    
		ti_nuc_monitor_current_position = 0;

		// Insert the interrupt OP Code (0x00 0x00 0x00) 
		if( ti_nuc_monitor_state == NUC_MONITOR_TO_BE_RUN )
		{
		  for( i=0; i < 3; i++)
		  {
		    ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] = 0x00;			     
		  } // End for
	    } // Endif

        // Reset intermediate buffer pointers
		ti_nuc_monitor_Thread_current_pos  = 0;
		ti_nuc_monitor_LISR_current_pos    = 0;
	  }
	  else
	  {
	    return;
	  } // End if
	  
	  // Write FN OP CODE  : (0x00 | xx)

	  ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] =
	    (SYS_UWORD8) ( C_OP_CODE_FN_LOG );

      // Insert the fn%42432 
      fn = l1_get_actual_fn_mod42432();

      // insert LSB
	  ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] =
	    (SYS_UWORD8) (fn & 0x00ff);
	 
	  // insert MSB 
	  ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] =
	    (SYS_UWORD8) ((fn & 0xff00) >> 8);

      ti_nuc_monitor_state            = NUC_MONITOR_RUNNING;
      	  
	  return; 
	} // End if
	
	// Copy the LISR and Thread buffer into current buffer.

    //   - LISR array

    // Check the number of byte remaining in the current 
    // log buffer vs LISR buffer.

    remaining_bytes =  NUC_MONITOR_BUFFER_SIZE - ti_nuc_monitor_current_position;

	if( remaining_bytes > ti_nuc_monitor_LISR_next_pos )
	{
	  // Copy ti_nuc_monitor_LISR_next_pos bytes to Nucleus log buffer

      memcpy( &ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position ], 
              &ti_nuc_monitor_LISR_Buffer[ ti_nuc_monitor_LISR_next_page ][ 0 ],
              ti_nuc_monitor_LISR_next_pos);

	  ti_nuc_monitor_current_position += ti_nuc_monitor_LISR_next_pos;

	}
	else
	{

	  // Copy remaining_bytes bytes to Nucleus log buffer

      memcpy( &ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position ], 
              &ti_nuc_monitor_LISR_Buffer[ ti_nuc_monitor_LISR_next_page ][ 0 ],
              remaining_bytes);

      ti_nuc_monitor_current_position += remaining_bytes;

      // Send current buffer

      // send a message to the ti profiler task

	  if( rvf_get_buf( prof_mb_id, sizeof( T_PROFILER_MSG ), ( T_RVF_BUFFER** ) &msg ) == RVF_GREEN )
	  {
	    msg->msg_id                     = TI_NUC_MONITOR_OUTPUT_BUFFER;
	    msg->p_buffer                   = ti_nuc_monitor_current_buffer;
	    msg->buffer_size                = ti_nuc_monitor_current_position;

	    ti_nuc_monitor_current_position = 0;

	    if( rvf_send_msg( prof_addr_id, msg ) != RV_OK )
		{
          ti_nuc_monitor_state = NUC_MONITOR_TO_BE_RUN;
	      return;		  
		} // End if
	  }
	  else
	  {
	    ti_nuc_monitor_state = NUC_MONITOR_TO_BE_RUN;
	    return; 
	  } // End if

      // Change Nucleus log buffer

	  if((ti_nuc_monitor_mem_alloc (prof_trace_id, 
	                     NUC_MONITOR_BUFFER_SIZE, 
			             (T_RVT_BUFFER*) &ti_nuc_monitor_current_buffer)) 
		            != RVT_OK)
	  {
	    ti_nuc_monitor_state = NUC_MONITOR_TO_BE_RUN;
	    return;
	  } // End if 

	  // Copy remaining_bytes bytes to Nucleus log buffer
      memcpy( &ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position ], 
              &ti_nuc_monitor_LISR_Buffer[ ti_nuc_monitor_LISR_next_page ][ remaining_bytes ],
              ( ti_nuc_monitor_LISR_next_pos - remaining_bytes ));

	  ti_nuc_monitor_current_position += ( ti_nuc_monitor_LISR_next_pos - remaining_bytes );

	} // End if
         
    //   - Thread Array

    // Check the number of byte remaining in the current 
    // log buffer vs Thread buffer.

    remaining_bytes =  NUC_MONITOR_BUFFER_SIZE - ti_nuc_monitor_current_position;

	if( remaining_bytes > ti_nuc_monitor_Thread_next_pos )
	{
	  // Copy ti_nuc_monitor_Thread_next_pos bytes to Nucleus log buffer

      memcpy( &ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position ], 
              &ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_next_page ][ 0 ],
              ti_nuc_monitor_Thread_next_pos);

	  ti_nuc_monitor_current_position += ti_nuc_monitor_Thread_next_pos;

	}
	else
	{

	  // Copy remaining_bytes bytes to Nucleus log buffer

      memcpy( &ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position ], 
              &ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_next_page ][ 0 ],
              remaining_bytes);

      ti_nuc_monitor_current_position += remaining_bytes;

      // Send current buffer

      // send a message to the ti profiler task

	  if( rvf_get_buf( prof_mb_id, sizeof( T_PROFILER_MSG ), ( T_RVF_BUFFER** ) &msg ) == RVF_GREEN )
	  {
	    msg->msg_id                     = TI_NUC_MONITOR_OUTPUT_BUFFER;
	    msg->p_buffer                   = ti_nuc_monitor_current_buffer;
	    msg->buffer_size                = ti_nuc_monitor_current_position;

	    ti_nuc_monitor_current_position = 0;

	    if( rvf_send_msg( prof_addr_id, msg ) != RV_OK )
		{
          ti_nuc_monitor_state = NUC_MONITOR_TO_BE_RUN;
	      return;		  
		} // End if
	  }
	  else
	  {
	    ti_nuc_monitor_state = NUC_MONITOR_TO_BE_RUN;
	    return; 
	  } // End if

      // Change Nucleus log buffer

	  if((ti_nuc_monitor_mem_alloc (prof_trace_id, 
	                     NUC_MONITOR_BUFFER_SIZE, 
			             (T_RVT_BUFFER*) &ti_nuc_monitor_current_buffer)) 
		            != RVT_OK)
	  {
	    ti_nuc_monitor_state = NUC_MONITOR_TO_BE_RUN;
	    return;
	  } // End if 

	  // Copy remaining_bytes bytes to Nucleus log buffer
      memcpy( &ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position ], 
              &ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_next_page ][ remaining_bytes ],
              ( ti_nuc_monitor_Thread_next_pos - remaining_bytes ));

	  ti_nuc_monitor_current_position += ( ti_nuc_monitor_Thread_next_pos - remaining_bytes );

	} // End if
    
	// Exit if monitoring must be stopped

    if( ti_nuc_monitor_state == NUC_MONITOR_TO_BE_STOP )
	{
	  ti_nuc_monitor_state = NUC_MONITOR_STOPPED;
	  return;
	} // End if

    // Insert actual Frame number 

    // Check if enough space in current buffer
	if( ti_nuc_monitor_current_position >= (NUC_MONITOR_BUFFER_SIZE - 3) )
	{
      // send a message to the ti profiler task

	  if( rvf_get_buf( prof_mb_id, sizeof( T_PROFILER_MSG ), ( T_RVF_BUFFER** ) &msg ) == RVF_GREEN )
	  {
	    msg->msg_id                     = TI_NUC_MONITOR_OUTPUT_BUFFER;
	    msg->p_buffer                   = ti_nuc_monitor_current_buffer;
	    msg->buffer_size                = ti_nuc_monitor_current_position;

	    ti_nuc_monitor_current_position = 0;

	    if( rvf_send_msg( prof_addr_id, msg ) != RV_OK )
		{
          ti_nuc_monitor_state = NUC_MONITOR_TO_BE_RUN;
	      return;		  
		} // End if
	  }
	  else
	  {
	    ti_nuc_monitor_state = NUC_MONITOR_TO_BE_RUN;
	    return; 
	  } // End if

      // Change Nucleus log buffer

	  if((ti_nuc_monitor_mem_alloc (prof_trace_id, 
	                     NUC_MONITOR_BUFFER_SIZE, 
			             (T_RVT_BUFFER*) &ti_nuc_monitor_current_buffer)) 
		            != RVT_OK)
	  {
	    ti_nuc_monitor_state = NUC_MONITOR_TO_BE_RUN;
	    return;
	  } // End if 
	} // End if

	// Write FN OP CODE  : (0x00 | xx)

	ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] =
	  (SYS_UWORD8) ( C_OP_CODE_FN_LOG );

    // Insert the fn%42432 
    fn = l1_get_actual_fn_mod42432();

    // insert LSB
	ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] =
	  (SYS_UWORD8) (fn & 0x00ff);
	 
	// insert MSB 
	ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] =
	  (SYS_UWORD8) ((fn & 0xff00) >> 8);

  } // ti_nuc_monitor_hisr

  /*-------------------------------------------------------*/
  /* ti_nuc_monitor_check_ID_buffer()                      */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  During the Thread/Queue ID -> names buffer           */
  /*  transmission, change current buffer.                 */
  /*  Send the full buffer and allocate a new one.         */
  /*-------------------------------------------------------*/

  void ti_nuc_monitor_check_ID_buffer( void )
  {
    // Reset pointer
	ti_nuc_monitor_current_position = 0; 

	// Send a message to the Riviera Tracer to Output the current buffer.
    if( rvt_send_trace_no_cpy( ( T_RVT_BUFFER ) ti_nuc_monitor_current_buffer, 
                               prof_trace_id,
                               NUC_MONITOR_BUFFER_SIZE,   // Length in bytes  
                               RVT_BINARY_FORMAT ) != RVT_OK )
    {
	  // code to insert a breakpoint if failed. 
      ti_nuc_monitor_state = NUC_MONITOR_STOPPED; 		     
    } // End if


    // Allocate one buffer.
     do
     {
	   ti_nuc_monitor_state = NUC_MONITOR_STOPPED;
			  
	   if ((ti_nuc_monitor_mem_alloc (prof_trace_id, 
	                       NUC_MONITOR_BUFFER_SIZE, 
		   	             (T_RVT_BUFFER*) &ti_nuc_monitor_current_buffer)) 
		     	!= RVT_OK)
  	   {
	     ti_nuc_monitor_state = NUC_MONITOR_WAITING;			  
	   } // End if
      } while (ti_nuc_monitor_state == NUC_MONITOR_WAITING);
         	
  } // ti_nuc_monitor_check_ID_buffer

  /*-------------------------------------------------------*/
  /* ti_nuc_monitor_tdma_action()                          */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  this function activate the HISR monitor              */
  /*-------------------------------------------------------*/

  void ti_nuc_monitor_tdma_action( void )
  {

    if(( ti_nuc_monitor_state == NUC_MONITOR_RUNNING )   ||
	   ( ti_nuc_monitor_state == NUC_MONITOR_TO_BE_RUN ) ||
	   ( ti_nuc_monitor_state == NUC_MONITOR_TO_BE_STOP ) ||
	   ( ti_nuc_monitor_state == NUC_MONITOR_TO_BE_STARTED ))
	{

      // Activate the hisr in order to start the buffer	fill
	  NU_Activate_HISR( &ti_nuc_monitor_cb );
       	  
	} // End if

  } // ti_nuc_monitor_tdma_action

  /*-------------------------------------------------------*/
  /* ti_nuc_monitor_LISR_log()                             */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  This function permits to include the LISR IRQ and    */
  /*   timer value in the LISR log buffer                  */
  /* Warning : To save cpu, no overflow check is done	   */
  /*-------------------------------------------------------*/

  void ti_nuc_monitor_LISR_log( void )
  {
  	SYS_UWORD16 timer_value;

    if( ti_nuc_monitor_state == NUC_MONITOR_RUNNING )
	{ 

      // Exit Idle mode if reached

	  ti_nuc_monitor_idle_reached  = 0;

	  IRQ_value = (SYS_UWORD8) ((* (volatile SYS_UWORD16 *) INTH_B_IRQ_REG) & 0x001f);

	  // Write LISR OP CODE and IRQ : (0xC0 | IRQ)

	  ti_nuc_monitor_LISR_Buffer[ ti_nuc_monitor_LISR_current_page ]
	                            [ ti_nuc_monitor_LISR_current_pos ] =
	    (SYS_UWORD8) (C_OP_CODE_LISR_FLAG | IRQ_value);
	  
	  // Write the timer value

	  timer_value = TM_ReadTimer(2); // (* (SYS_UWORD16 *) TIMER2_READ_TIM)

	  // insert LSB
	   
	  ti_nuc_monitor_LISR_Buffer[ ti_nuc_monitor_LISR_current_page ]
	                            [ ti_nuc_monitor_LISR_current_pos + 1 ] =
	    (SYS_UWORD8) (timer_value & 0x00ff);
	  
	  // insert MSB

	  ti_nuc_monitor_LISR_Buffer[ ti_nuc_monitor_LISR_current_page ]
	                            [ ti_nuc_monitor_LISR_current_pos + 2 ] =
	    (SYS_UWORD8) ((timer_value & 0xff00) >> 8);

      ti_nuc_monitor_LISR_current_pos += 3;

	} // End if

  } // ti_nuc_monitor_LISR_log

  /*-------------------------------------------------------*/
  /* ti_nuc_monitor_LISR_log_end()                         */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  This function permits to include the LISR end        */
  /*   timer value in the LISR log buffer                  */
  /* Warning : To save cpu, no overflow check is done	   */
  /*-------------------------------------------------------*/

  void ti_nuc_monitor_LISR_log_end( void )
  {
	SYS_UWORD16 timer_value;

    if( ti_nuc_monitor_state == NUC_MONITOR_RUNNING )
	{ 	  
	  // Write the timer value

	  timer_value = TM_ReadTimer(2);

	  // insert LSB
	   
	  ti_nuc_monitor_LISR_Buffer[ ti_nuc_monitor_LISR_current_page ]
	                            [ ti_nuc_monitor_LISR_current_pos ] =
	    (SYS_UWORD8) (timer_value & 0x00ff);
	  
	  // insert MSB

	  ti_nuc_monitor_LISR_Buffer[ ti_nuc_monitor_LISR_current_page ]
	                            [ ti_nuc_monitor_LISR_current_pos + 1 ] =
	    (SYS_UWORD8) ((timer_value & 0xff00) >> 8);

      ti_nuc_monitor_LISR_current_pos += 2;

      // Switch LISR & thread buffer page if IRQ4 (TDMA)
	  if( IRQ_value == 4 )
	  {
        // copy current to next 
        ti_nuc_monitor_LISR_next_pos    = ti_nuc_monitor_LISR_current_pos;
	    ti_nuc_monitor_LISR_next_page   = ti_nuc_monitor_LISR_current_page;

        // instead of changing the Thread page here, set a boolean
		ti_nuc_monitor_change_thread_page = 1;

        //ti_nuc_monitor_Thread_next_pos  = ti_nuc_monitor_Thread_current_pos;
	    //ti_nuc_monitor_Thread_next_page = ti_nuc_monitor_Thread_current_page;

        // Reset current pointer
        ti_nuc_monitor_LISR_current_pos   = 0;
        //ti_nuc_monitor_Thread_current_pos = 0;
      
        // Change page
        if( ti_nuc_monitor_LISR_current_page == 0 )
        {
	       ti_nuc_monitor_LISR_current_page   = 1;
	    //   ti_nuc_monitor_Thread_current_page = 1;
	    }
        else
        {
	       ti_nuc_monitor_LISR_current_page   = 0;
		//   ti_nuc_monitor_Thread_current_page = 0;
        } // End if

		// reset IRQ_value
		IRQ_value = 0;

        // reset timer 
        TM_ResetTimer (2, 0xFFFF, 1, 0);
        TM_StartTimer (2);

        // Doesn't work
	    // * (volatile SYS_UWORD16 *) TIMER2_LOAD_TIM = 0xFFFF;
		// * (volatile SYS_UWORD16 *) TIMER2_CNTL     = 0x0003;

	  } // End if  
  	  
	} // End if
    
  } // ti_nuc_monitor_LISR_log_end
   
  /*-------------------------------------------------------*/
  /* ti_nuc_monitor_Thread_log()                           */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  This function permits to include the Thread and      */
  /*   timer value in the log buffer                       */
  /*-------------------------------------------------------*/

  void ti_nuc_monitor_Thread_log( void )
  {
	SYS_UWORD16	timer_value;
	SYS_UWORD8  i;

    if( ti_nuc_monitor_state == NUC_MONITOR_RUNNING )
	{ 

      // Exit Idle mode if reached

	  ti_nuc_monitor_idle_reached  = 0;

      // Check if page must be changed 
	  if( ti_nuc_monitor_change_thread_page == 1 )
	  {

	    ti_nuc_monitor_change_thread_page = 0;

        // copy current to next 
        ti_nuc_monitor_Thread_next_pos  = ti_nuc_monitor_Thread_current_pos;
	    ti_nuc_monitor_Thread_next_page = ti_nuc_monitor_Thread_current_page;

        // Reset current pointer
        ti_nuc_monitor_Thread_current_pos = 0;
      
        // Change page
        if( ti_nuc_monitor_Thread_current_page == 0 )
        {
	       ti_nuc_monitor_Thread_current_page = 1;
	    }
        else
        {
		   ti_nuc_monitor_Thread_current_page = 0;
        } // End if
        
	  } // End if

	  // Search the ID considering the Nucleus_Current_thread Pointer

      for( i = 0; i < ti_nuc_monitor_thread_pos ; i++ )
	  {
		if( ti_nuc_monitor_thread_id[ i ] == (SYS_UWORD32) TCD_Current_Thread )
		{
		  // insert the OP Code Thread | ID
	      ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_current_page ]
	                                  [ ti_nuc_monitor_Thread_current_pos ] =
	                       (SYS_UWORD8) (C_OP_CODE_THREAD_FLAG | i);
          // exit loop
	      break;	      		  
		} // End if
      } // End for
	  
	  // Write the timer value

	  timer_value = TM_ReadTimer(2); // (* (SYS_UWORD16 *) TIMER2_READ_TIM)

	  // insert LSB
	   
	  ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_current_page ]
	                            [ ti_nuc_monitor_Thread_current_pos + 1 ] =
	    (SYS_UWORD8) (timer_value & 0x00ff);
	  
	  // insert MSB

	  ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_current_page ]
	                            [ ti_nuc_monitor_Thread_current_pos + 2 ] =
	    (SYS_UWORD8) ((timer_value & 0xff00) >> 8);

      ti_nuc_monitor_Thread_current_pos += 3;

	} // End if

  } // ti_nuc_monitor_Thread_log

  /*-------------------------------------------------------*/
  /* ti_nuc_monitor_Queue_log()                            */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  This function permits to include the Queue ID and    */
  /*   status value in the log buffer                      */
  /*  Direction = 0 => Send message						   */
  /*  Direction = 1 => Receive message					   */
  /*  status    = 0 => OK       						   */
  /*  status    = 1 => Queue full  					       */
  /*-------------------------------------------------------*/

  void ti_nuc_monitor_Queue_log( SYS_UWORD32* queue_pointer, SYS_UWORD8 status, SYS_UWORD8 Direction )
  {
	SYS_UWORD8	status_word;
	SYS_UWORD8  i;



    if( ti_nuc_monitor_state == NUC_MONITOR_RUNNING )
	{ 

      // Exit Idle mode if reached

	  ti_nuc_monitor_idle_reached  = 0;

      // Check if page must be changed 
	  if( ti_nuc_monitor_change_thread_page == 1 )
	  {

	    ti_nuc_monitor_change_thread_page = 0;

        // copy current to next 
        ti_nuc_monitor_Thread_next_pos  = ti_nuc_monitor_Thread_current_pos;
	    ti_nuc_monitor_Thread_next_page = ti_nuc_monitor_Thread_current_page;

        // Reset current pointer
        ti_nuc_monitor_Thread_current_pos = 0;
      
        // Change page
        if( ti_nuc_monitor_Thread_current_page == 0 )
        {
	       ti_nuc_monitor_Thread_current_page = 1;
	    }
        else
        {
		   ti_nuc_monitor_Thread_current_page = 0;
        } // End if
        
	  } // End if

	  // Search the ID considering the Nucleus_Current_thread Pointer
      for( i = 0; i < ti_nuc_monitor_queue_pos ; i++ )
	  {
		if( ti_nuc_monitor_queue_id[ i ] == (SYS_UWORD32) queue_pointer)
		{
		  // insert the OP Code Thread | ID
	      ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_current_page ]
	                                  [ ti_nuc_monitor_Thread_current_pos ] =
	                       (SYS_UWORD8) (C_OP_CODE_QUEUE_FLAG | i);
	      
		  // exit loop
	      break;	      		  
	      		  
		} // End if
      } // End for

	  // Write the status value and Receive/send flag
	  
      status_word =  (Direction << 8) | (status << 7) | i; // Direction | Status | ID

	  ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_current_page ]
	                            [ ti_nuc_monitor_Thread_current_pos + 1 ] =
	    (SYS_UWORD8) (status_word);
	  
	  ti_nuc_monitor_Thread_current_pos += 2;
	  	  	  
	} // End if

  } // ti_nuc_monitor_Queue_log



  /*-------------------------------------------------------*/
  /* ti_nuc_monitor_sleep()                                */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  This function permits to include the sleep and      */
  /*   timer value in the log buffer                       */
  /*-------------------------------------------------------*/

  void ti_nuc_monitor_sleep( void )
  {

	SYS_UWORD16	timer_value;
	SYS_UWORD8  i;

    if(( ti_nuc_monitor_state        == NUC_MONITOR_RUNNING ) &&
	   ( ti_nuc_monitor_idle_reached == 0 ))
	{ 

      ti_nuc_monitor_idle_reached = 1;

      // insert the OP Code Thread | ID
	  ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_current_page ]
	                              [ ti_nuc_monitor_Thread_current_pos ] =
	                    (SYS_UWORD8) (0xBF);
	  
	  // Write the timer value

	  timer_value = TM_ReadTimer(2); // (* (SYS_UWORD16 *) TIMER2_READ_TIM)

	  // insert LSB
	   
	  ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_current_page ]
	                            [ ti_nuc_monitor_Thread_current_pos + 1 ] =
	    (SYS_UWORD8) (timer_value & 0x00ff);
	  
	  // insert MSB

	  ti_nuc_monitor_Thread_Buffer[ ti_nuc_monitor_Thread_current_page ]
	                            [ ti_nuc_monitor_Thread_current_pos + 2 ] =
	    (SYS_UWORD8) ((timer_value & 0xff00) >> 8);

      ti_nuc_monitor_Thread_current_pos += 3;
	} // End if

  } // ti_nuc_monitor_sleep

// -------------- RVT Copied function --------------------------------
//    --- Modified Memory Bank ----

/********************************************************************************/
/*                                                                              */
/*    Function Name:   ti_nuc_monitor_mem_alloc									*/
/*                                                                              */
/*    Purpose:         this function creates a memory bank for the profiler task*/
/*																				*/
/*    Input Parameters:                                                         */
/*        refer to t_rvt_type.h													*/
/*																				*/
/*    Output Parameters:                                                        */
/*        refer to t_rvt_type.h													*/
/*                                                                              */
/*    Global Parameters:                                                        */
/*        None.                                                                 */
/*                                                                              */
/*    Note:                                                                     */
/*        None.                                                                 */
/*                                                                              */
/********************************************************************************/


T_RVT_RET ti_nuc_monitor_mem_alloc(T_RVT_USER_ID user_id, T_RVT_MSG_LG buffer_lenght, T_RVT_BUFFER * buff)
{
	T_RVF_MB_STATUS return_value;	

	return_value = rvf_get_buf (prof_mb_id, (UINT32) buffer_lenght + RVT_HEADER_SIZE, (T_RVF_BUFFER**) buff);

	if (return_value == RVF_RED)
	{
		*buff = NULL;
		return RVT_MEMORY_ERR;
	}
	else
	{
		*buff = *buff + RVT_HEADER_SIZE;
		return RVT_OK;
	}
}  // ti_nuc_monitor_mem_alloc

// -------------------------------------------------------------------

#endif // TI_NUC_MONITOR

#if (TI_PROFILER == 1) || (TI_NUC_MONITOR == 1)

  /*-------------------------------------------------------*/
  /* ti_prf_init()                                         */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description: Initialize profiling hisrs               */
  /* ------------                                          */
  /*  Register the profiling to rvt                        */
  /*                                                       */
  /*-------------------------------------------------------*/

  void ti_prf_init( void )
  {
    #if (TI_PROFILER == 1)
      ti_profiler_init_profiler_module();
	#endif
    #if (TI_NUC_MONITOR == 1)
      ti_nuc_monitor_init_module();
	#endif
    // Register to Trace task module
	rvt_register_id( "PROF", &prof_trace_id, ti_profiler_trace_rx_callback );

  } // ti_prf_init

  /*-------------------------------------------------------*/
  /* ti_prf_core()                                         */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /*  this function :                                      */
  /*    - start the profiler on PC side message reception  */
  /*    - stop the profiler on PC side message reception   */
  /*    - cut the profiler buffer less than 256 bytes      */
  /*      messages to rvt                                  */
  /*													   */
  /*-------------------------------------------------------*/

  T_RV_RET ti_prf_core(void)
  {

    SYS_UWORD16      i = 0;

    SYS_UWORD16      event;
	SYS_UWORD16      message_length;
    T_PROFILER_MSG   *msg;

    while( INFINITE_LOOP )
    {	

	  // Suspend the task on empty queue
	  event = rvf_wait( RVF_TASK_MBOX_0_EVT_MASK, 0 );
	  msg   = ( T_PROFILER_MSG* ) rvf_read_mbox( RVF_TASK_MBOX_0 );
      
      // Check the msg ID code 
      switch( msg->msg_id )
	   {

       #if (TI_PROFILER == 1)
	     case TI_PROFILER_OUTPUT_BUFFER : 
	     // Cut the buffer in small element to be sent to 
		 // PC by the RVT.
		 {
		   message_length = TRACE_MESSAGE_SIZE;
		   pos_pr_buffer  = 0;

		   // loop until end of buffer
		   while( pos_pr_buffer < pr_buffer_length )
		   {
              // Update message length for next loop turn
			  if ( pos_pr_buffer <= ( pr_buffer_length - TRACE_MESSAGE_SIZE ) )
			  {
			    message_length = TRACE_MESSAGE_SIZE;
			  }
			  else
			  {
				message_length = pr_buffer_length - pos_pr_buffer;
			  }

              if( rvt_send_trace_cpy( ( T_RVT_BUFFER ) &pr_buffer[ pos_pr_buffer ], 
                                      prof_trace_id,
                                      message_length * 4,   // Length in bytes  
                                      RVT_BINARY_FORMAT ) == RVT_OK )
              {		     
				// Update pr_buffer pointer
                pos_pr_buffer += message_length;  
                 
			  } // End if 

		   } // End while

           // Restart the profiling at vitam aeternam unless stop command received.
		   if( profiler_state == PROFILER_FREEZED )
		   {
		     profiler_state = PROFILER_TO_BE_UNFREEZED;
		   } // End if 

		   break;
		 } // TI_PROFILER_OUTPUT_BUFFER

		 case TI_PROFILER_START_PROFILING :
		 { 
		   #if (L1_TRACER == 1)
		     SendTraces("TI_PROF: msg TI_PROFILER_START\n\r");
		   #endif

 		   ti_profiler_event_state     = (T_PROFILER_EVENTS) (msg->event_number);
 
 		   if( ti_profiler_event_state == NO_SLEEP_EVENT )
 		   {
 			 ti_profiler_nb_sleep_counter_ref = msg->arg1;	// nb_tdma 
 		   } // End if

           // Source of Activation
 		   ti_profiler_active = ACTIVE_BY_HOST;

           // change state of profiler
           profiler_state = PROFILER_TO_BE_RAN; 

		   break;
		 } // TI_PROFILER_START_PROFILING

		 case TI_PROFILER_END_PROFILING :
		 {
		   #if (L1_TRACER == 1)
		     SendTraces("TI_PROF: msg TI_PROFILER_STOP\n\r");
		   #endif

           profiler_state = PROFILER_TO_BE_STOPPED; 
		   break;
		 } // TI_PROFILER_END_PROFILING

	   #endif

       #if (TI_NUC_MONITOR == 1)

		 case TI_NUC_MONITOR_START :
		 {
		   SYS_UWORD8   i = 0;
		   SYS_UWORD8   j = 0;
		   #if (L1_TRACER == 1)
		     SendTraces("TI_PROF: msg TI_NUC_MONITOR_START\n\r");
		   #endif

		   if( ti_nuc_monitor_state == NUC_MONITOR_STOPPED )
		   {

             // Output the task and queues create info.
			 // buffer format is following : 
			 //   C_OP_CODE_NUC_MON_THREAD_IDS | (1 byte)
			 //   < Thread ID0 > 			   | (1 byte)
             //   < Thread name >              | (8 bytes)
             //   < Thread ID1 >			   | (1 byte)
			 //   < Thread name >              | (8 bytes)
			 //    ..............
			 //   C_OP_CODE_NUC_MON_THREAD_IDS | (1 byte)
			 //   < Queue ID0 > 			   | (1 byte)
             //   < Queue name >               | (8 bytes)
             //   < Queue ID1 >			       | (1 byte)
			 //   < Queue name >               | (8 bytes)
			 //    ..............
			 //   C_OP_CODE_NUC_MON_THREAD_IDS | (1 byte)
			       
			 // Allocate one buffer.

	         while ((ti_nuc_monitor_mem_alloc (prof_trace_id, 
	                                 NUC_MONITOR_BUFFER_SIZE, 
			                         (T_RVT_BUFFER*) &ti_nuc_monitor_current_buffer)) 
			          != RVT_OK);


             // Output the ID (thread and Queue array)

			 ti_nuc_monitor_current_buffer[ti_nuc_monitor_current_position++] = C_OP_CODE_NUC_MON_THREAD_IDS;

             // Output the Thread IDs and corresponding Thread name.

             for( i = 0 ; i < ti_nuc_monitor_thread_pos ; i++ )
			 {
			   // distinghuish Task and HISR
			   if( (( TC_TCB * ) ( ti_nuc_monitor_thread_id[ i ]))->tc_id == TC_HISR_ID)
			   {
			     ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] = (0x80 | i); // ID
			   }
			   else
			   {
			     ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] = i;  // ID
			   } // End if

			   if( ti_nuc_monitor_current_position >= NUC_MONITOR_BUFFER_SIZE )
			   {
			     ti_nuc_monitor_check_ID_buffer();
			   } // End if

               // Write task priority
			   ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] =  
				   (SYS_UWORD8) ((( TC_TCB * ) ( ti_nuc_monitor_thread_id[ i ]))->tc_priority);

			   if( ti_nuc_monitor_current_position >= NUC_MONITOR_BUFFER_SIZE )
			   {
			     ti_nuc_monitor_check_ID_buffer();
			   } // End if
  
               // Write name in the Log buffer.
			   for( j = 0 ; j < NU_MAX_NAME ; j++ )
			   {
			     ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] = 
			       (SYS_UWORD8) ((( TC_TCB * ) ( ti_nuc_monitor_thread_id[ i ]))->tc_name[ j ]);	// Thread_name
			     if( ti_nuc_monitor_current_position >= NUC_MONITOR_BUFFER_SIZE )
			     {
			       ti_nuc_monitor_check_ID_buffer();
			     } // End if
			   } // End for

			 } // End for 

             // Output the ID (thread and Queue array)
			 if( ti_nuc_monitor_queue_pos != 0 )
			 {
			   ti_nuc_monitor_current_buffer[ti_nuc_monitor_current_position++] = C_OP_CODE_NUC_MON_THREAD_IDS;

               // Output the Queue IDs and corresponding Queue name.

		       if( ti_nuc_monitor_current_position >= NUC_MONITOR_BUFFER_SIZE )
			   {
			     ti_nuc_monitor_check_ID_buffer();
			   } // End if

               for( i = 0 ; i < ti_nuc_monitor_queue_pos ; i++ )
			   {
			     ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] = i;  // ID

			     if( ti_nuc_monitor_current_position >= NUC_MONITOR_BUFFER_SIZE )
			     {
			       ti_nuc_monitor_check_ID_buffer();
			     } // End if

			     // Write name in the Log buffer.	   
			     for( j = 0 ; j < NU_MAX_NAME ; j++ )
			     {
			       ti_nuc_monitor_current_buffer[ ti_nuc_monitor_current_position++ ] = 
			         (SYS_UWORD8) ((( QU_QCB * ) ( ti_nuc_monitor_queue_id[ i ] ))->qu_name[ j ]);	// Thread_name
			       if( ti_nuc_monitor_current_position >= NUC_MONITOR_BUFFER_SIZE )
			       {
			         ti_nuc_monitor_check_ID_buffer();
			       } // End if
			     } // End for

			   } // End for 
			 } // End if

			 // Insert end op code (same as begin op code)

			 ti_nuc_monitor_current_buffer[ti_nuc_monitor_current_position++] = C_OP_CODE_NUC_MON_THREAD_IDS;
              
			 // flush the last buffer.

		     // Send a message to the Riviera Tracer to Output the current buffer.
             if( rvt_send_trace_no_cpy( ( T_RVT_BUFFER ) ti_nuc_monitor_current_buffer, 
                                        prof_trace_id,
                                        ti_nuc_monitor_current_position,   // Length in bytes  
                                        RVT_BINARY_FORMAT ) != RVT_OK )
             {
			   // Code to insert break point.
               ti_nuc_monitor_state = NUC_MONITOR_STOPPED;		     
             } // End if

             // Configuration buffer has been logged out.

	         /*
	          ** Allocate a buffer for Nucleus profiling
	          */

	         while ((ti_nuc_monitor_mem_alloc (prof_trace_id, 
	                                 NUC_MONITOR_BUFFER_SIZE, 
			                        (T_RVT_BUFFER*) &ti_nuc_monitor_current_buffer)) 
			        != RVT_OK);

             // reset the current pointer.
		     ti_nuc_monitor_current_position = 0;

			 // Set the new Monitoring state.
             ti_nuc_monitor_state            = NUC_MONITOR_TO_BE_STARTED;
		   } // End if

		  break;

		 } // TI_NUC_MONITOR_START

		 case TI_NUC_MONITOR_OUTPUT_BUFFER :
		 {
		   // The buffer is output only if it is full.

		   // Send a message to the Riviera Tracer to Output the current buffer.
           if( rvt_send_trace_no_cpy( ( T_RVT_BUFFER ) msg->p_buffer, 
                                      prof_trace_id,
                                      msg->buffer_size,   // Length in bytes  
                                      RVT_BINARY_FORMAT ) != RVT_OK )
           {
             // increment lost messages variable
			 ti_nuc_monitor_lost_messages++; 	     
           } // End if

		   break;

		 } // TI_NUC_MONITOR_OUTPUT_BUFFER

		 case TI_NUC_MONITOR_STOP :
		 {
		   #if (L1_TRACER == 1)
		     SendTraces("TI_PROF: msg TI_NUC_MONITOR_STOP\n\r");
		   #endif

		   ti_nuc_monitor_state = NUC_MONITOR_TO_BE_STOP; 

		   break;

		 } // TI_NUC_MONITOR_STOP

       #endif

         default :
		 {
		   #if (L1_TRACER == 1)		   
		     SendTraces("TI_PROF: msg default\n\r");
		   #endif

		   break;
		 } // default

	   } // End switch

      
      // Free the msg alloc 
	  rvf_free_buf( msg );

	} // End while ( INFINITE_LOOP )

  } // ti_profiler_task


  /*-------------------------------------------------------*/
  /* ti_profiler_trace_rx_callback()                       */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------ 										   */
  /* callback function called when the trace               */
  /* module receives a msg for the profiler                */
  /*  or the nucleus monitor.                              */
  /*                                                       */
  /*-------------------------------------------------------*/

  void ti_profiler_trace_rx_callback(T_RVT_BUFFER trace_msg, UINT16 trace_msg_size)
  {	 
    T_PROFILER_MSG * msg;

    // send a message to the ti profiler received from Rvt
	if( rvf_get_buf( prof_mb_id, sizeof( T_PROFILER_MSG ), ( T_RVF_BUFFER** ) &msg ) == RVF_GREEN )
	{
	/*
	  if( trace_msg[0] == 1 )
	    msg->msg_id = TI_PROFILER_START_PROFILING;
	  else if ( trace_msg[0] == 0 )
	    msg->msg_id = TI_PROFILER_END_PROFILING;
	  else if ( trace_msg[0] == 2 )
	    msg->msg_id = TI_NUC_MONITOR_START;
	  else if ( trace_msg[0] == 3 )
	    msg->msg_id = TI_NUC_MONITOR_STOP;
	 */
 	  msg->msg_id        = ((T_PROFILER_MSG_UART*) trace_msg)->msg_id;
 	  msg->event_number  = ((T_PROFILER_MSG_UART*) trace_msg)->event_number;
 	  msg->arg1          = ((T_PROFILER_MSG_UART*) trace_msg)->arg1;

	  rvf_send_msg( prof_addr_id, msg );
	}

  } // ti_profiler_trace_rx_callback

#endif // NUC_MONITOR || PROFILER

