/**************************************************************************
*
* rvt_task.c
*
* This contains the core of the Trace task.
*
* (C) Texas Instruments, all rights reserved
*
* Version number   : 0.1
*
* History		   : 0.1 (7/5/2000) - Created
*
* Date             : 7/5/2000
*
* Author           : Guido Pagana
*
* Update           : Pascal Puel
*                  : David Lamy-Charrier (changes for Riviera 1.6)
*
***************************************************************************/

#include "../../include/config.h" 

#include "../rv/general.h"
#include "../rv/rv_general.h"
#include "../rvf/rvf_api.h"
#include "rvt_gen.h"
#include "rvt_def_i.h"
#include "rvt_env.h"
#include "rvt_env_i.h"
#include "../rvm/rvm_use_id_list.h"

#include "../../serial/serialswitch.h"

#include "../../nucleus/nucleus.h"

#include <string.h>

extern NU_HISR        TI_rcv_HISR;

/* Time (in milliseconds) between two consecutive 'System Time' messages */
#define RVT_ALIVE_POLLING_TIME    (RVF_MS_TO_TICKS (20000))

#define RVT_WAIT_FOR_HEADER       (1)
#define RVT_WAIT_FOR_DATA         (2)


/******************************************************************************/
/*                                                                            */
/*    Function Name:   rvt_task_core                                          */
/*                                                                            */
/*    Purpose:         Core of Trace task.                                    */
/*                                                                            */
/*    Input Parameters:                                                       */
/*        None.                                                               */
/*                                                                            */
/*    Output Parameters:                                                      */
/*        None.                                                               */
/*                                                                            */
/*    Global Parameters:                                                      */
/*        None.                                                               */
/*                                                                            */
/*    Note:                                                                   */
/*        None.                                                               */
/*                                                                            */
/******************************************************************************/
T_RVM_RETURN rvt_task_core (void)
{
    UINT16  event         = 0;
    UINT32  nb_bytes_sent = 0;

    #ifdef FRAMING_PROTOCOL

	// Request for the level of filtering, as well as the 32-bit
	// mask related to the software entities to be monitored.
	{
	    UINT8  trace_level_request[] = {RVM_INVALID_USE_ID, 0, 0, 0, 0,
					(RV_TRACE_LEVEL_ERROR - 1), 0, 0, 0, 0};

		// Note that the level is defined as invalid
		trace_level_request[0] = (char) rv_trace_user_id;

		// Transmit an 'empty' message
		nb_bytes_sent = 0;
		while (nb_bytes_sent < sizeof (trace_level_request))
		{
		    nb_bytes_sent += SER_tr_WriteNBytes (SER_LAYER_1,
					trace_level_request + nb_bytes_sent,
					sizeof (trace_level_request) -
						nb_bytes_sent);
		}
	}

	// Start the 'Alive Polling Timer'
	#if (OP_WCP == 0)
		rvf_start_timer (RVF_TIMER_0,
				 RVT_ALIVE_POLLING_TIME,
				 TRUE);
	#endif

	for (; ; )
	{
	    // Infinite wait on 'Trace Task' mailbox or timer events
	    event = rvf_wait ((1 << RVT_TRACE_MAILBOX) | (RVF_TIMER_0_EVT_MASK),
				  0);

	    // Check for some messages lost
	    if (((rvt_lost_msg_cpt.bit_mask).count >= RVT_MAX_LOST_TRACE_MSG)
		&& (p_rvt_lost_msg))
	    {
		INT8    count           = 0;
		UINT8   lost_msg_length = RVT_HDR_LENGTH + RVT_LOST_MSG_LENGTH;
		UINT32  lost_msg_cpt    = rvt_lost_msg_cpt.overall_value;

		// Append with the number of messages lost
		rvt_lost_msg_cpt.overall_value = 0;
		for (count = 0;
		     count < RVT_HEX_VALUE_LENGTH;
		     count++)
		{
		    p_rvt_lost_msg[lost_msg_length + count] =
		      Num2Char[(UINT8) ((lost_msg_cpt << (count << 2)) >> 28)];	
		}
		lost_msg_length += RVT_HEX_VALUE_LENGTH;

		// Send message to the UART with byte stuffing
		nb_bytes_sent = 0;
		while (nb_bytes_sent < lost_msg_length)
		{
			nb_bytes_sent += SER_tr_WriteNBytes (SER_LAYER_1,
				 (UINT8 *) p_rvt_lost_msg + nb_bytes_sent,
					 lost_msg_length - nb_bytes_sent);
		}

	    } // End of if ((rvt_lost_msg_cpt.bit_mask).count >= RVT_MAX_LOST_TRACE_MSG)

	    if (event & EVENT_MASK (RVT_TRACE_MAILBOX))
	    {
		T_RV_HDR  *msg = NULL;

		// Read the message from the mailbox
		if ((msg = (T_RV_HDR *) rvf_read_mbox (RVT_TRACE_MAILBOX))
		    != NULL)
		{
		    if (msg->msg_id == RVT_TRACE_RQST_ID)
		    {
			UINT8   msg_format = 0;
			UINT32  msg_length = 0;

			// Get the length
			msg_length = ((T_RVT_TRACE_RQST *) msg)->msg_length;

			// Get the format
			msg_format = ((T_RVT_TRACE_RQST *) msg)->format;

			// Copy the 'User ID'
			((UINT8 *) msg + RVT_HEADER_SIZE - 1)[0] =
				((T_RVT_TRACE_RQST *) msg)->user_id;
			msg_length++;

			switch (msg_format)
			{
			    case RVT_ASCII_FORMAT:
			    {
				// Send message to the UART without byte stuffing
				nb_bytes_sent = 0;
				while (nb_bytes_sent < msg_length)
				{
				    nb_bytes_sent += SER_tr_EncapsulateNChars
						(SER_LAYER_1,
					(char *) msg + RVT_HEADER_SIZE - 1 +
						nb_bytes_sent,
					msg_length - nb_bytes_sent);
				}
				break;
			    }
			    case RVT_BINARY_FORMAT:
			    {
				// Send message to the UART with byte stuffing
				nb_bytes_sent = 0;
				while (nb_bytes_sent < msg_length)
				{
					nb_bytes_sent += SER_tr_WriteNBytes
							(SER_LAYER_1,
						(UINT8 *) msg + RVT_HEADER_SIZE
							- 1 + nb_bytes_sent,
						msg_length - nb_bytes_sent);
				}
				break;
			    }
			    default:
			    {
				// Increment the number of messages lost.
				// Is the buffer corrupted?
				(rvt_lost_msg_cpt.bit_mask).count++;
				(rvt_lost_msg_cpt.bit_mask).unknown_format = 1;
				break;
			    }
			}

			// Deallocate the buffer
			rvf_free_buf (msg);

		    } // End of if (msg->msg_id == RVT_TRACE_RQST_ID)
		    else
		    {
			// Increment the number of messages lost. Is the buffer
			// corrupted?
			(rvt_lost_msg_cpt.bit_mask).count++;
			(rvt_lost_msg_cpt.bit_mask).unknown_request = 1;
		    }

		} // End of if (msg != NULL)
		else
		{
			// Increment the number of messages lost. Is the buffer
			// corrupted?
			(rvt_lost_msg_cpt.bit_mask).count++;
			(rvt_lost_msg_cpt.bit_mask).message_empty = 1;
		}

	    }  // End of if (event & EVENT_MASK (RVT_TRACE_MAILBOX))

	    if ((event & (RVF_TIMER_0_EVT_MASK)) && (p_rvt_sys_time))
	    {
		UINT8   count           = 0;
		UINT8   sys_time_length = RVT_HDR_LENGTH + RVT_SYS_TIME_LENGTH;
		UINT32  current_time    = rvf_get_tick_count ();

		// Append with the system time
		for (count = 0;
		     count < RVT_HEX_VALUE_LENGTH;
		     count++)
		{
		    p_rvt_sys_time[sys_time_length + count] =
		      Num2Char[(UINT8) ((current_time << (count << 2)) >> 28)];	
		}
		sys_time_length += RVT_HEX_VALUE_LENGTH;

		// Send message to the UART with byte stuffing
		nb_bytes_sent = 0;
		while (nb_bytes_sent < sys_time_length)
		{
			nb_bytes_sent += SER_tr_WriteNBytes (SER_LAYER_1,
				 (UINT8 *) p_rvt_sys_time + nb_bytes_sent,
					 sys_time_length - nb_bytes_sent);
		}

	    } // End of if (event & (RVF_TIMER_0_EVT_MASK))
		//freecalypso_lldbg_intinfo();
	}

    #else

		// Start the 'Alive Polling Timer'
		#if (OP_WCP == 0)
		  rvf_start_timer (RVF_TIMER_0,
						   RVT_ALIVE_POLLING_TIME,
						   TRUE);
		#endif
		
		for (;
			 ;
			 )
		{
			// Infinite wait on 'Trace Task' mailbox or timer events
			event = rvf_wait ((1 << RVT_TRACE_MAILBOX) | (RVF_TIMER_0_EVT_MASK),
							  0);

			// Check for some messages lost
			if (((rvt_lost_msg_cpt.bit_mask).count >= RVT_MAX_LOST_TRACE_MSG) && (p_rvt_lost_msg))			
			{
				INT8    count           = 0;
				UINT8   lost_msg_length = RVT_LOST_MSG_LENGTH;
				UINT32  lost_msg_cpt    = rvt_lost_msg_cpt.overall_value;

				// Append with the number of messages lost
				rvt_lost_msg_cpt.overall_value = 0;
				for (count = 0;
					 count < RVT_HEX_VALUE_LENGTH;
					 count ++)
				{
					p_rvt_lost_msg[lost_msg_length + count] = Num2Char[(UINT8) ((lost_msg_cpt << (count << 2)) >> 28)];
				}
				rvt_lost_msg_length += RVT_HEX_VALUE_LENGTH;

				// Append with the '\n' and '\r' characters for the hyper terminal
				p_rvt_lost_msg[lost_msg_length++] = '\n';
				p_rvt_lost_msg[lost_msg_length++] = '\r';

				// Send the message to the UART without byte stuffing
				nb_bytes_sent = 0;
				while (nb_bytes_sent < lost_msg_length)
				{
					nb_bytes_sent += SER_tr_WriteNChars (SER_LAYER_1,
														 (UINT8 *) p_rvt_lost_msg + nb_bytes_sent,
														 lost_msg_length - nb_bytes_sent);
				}

			} // End of if ((rvt_lost_msg_cpt.bit_mask).count >= RVT_MAX_LOST_TRACE_MSG)

			if (event & EVENT_MASK (RVT_TRACE_MAILBOX))
			{
				T_RV_HDR  *msg = NULL;

				// Read the message from the mailbox
				if ((msg = (T_RV_HDR *) rvf_read_mbox (RVT_TRACE_MAILBOX)) != NULL)
				{
					if (msg->msg_id == RVT_TRACE_RQST_ID)
					{
						UINT32  msg_length = 0;

						// Get the length
						msg_length = ((T_RVT_TRACE_RQST *) msg)->msg_length;

						// Send message to the UART without byte stuffing
						nb_bytes_sent = 0;
						while (nb_bytes_sent < msg_length)
						{
							nb_bytes_sent += SER_tr_WriteNChars (SER_LAYER_1,
																 msg + RVT_HEADER_SIZE + nb_bytes_sent,
																 msg_length - nb_bytes_sent);
						}

						// Append with the '\n' and '\r' characters for the hyper terminal
						msg_length        = 0;
						msg[msg_length++] = '\n';
						msg[msg_length++] = '\r';

						// Send message to the UART without byte stuffing
						nb_bytes_sent = 0;
						while (nb_bytes_sent < msg_length)
						{
							nb_bytes_sent += SER_tr_WriteNChars (SER_LAYER_1,
																 msg + nb_bytes_sent,
																 msg_length - nb_bytes_sent);
						}

						// Deallocate the buffer
						rvf_free_buf (msg);

					} // End of if (msg->msg_id == RVT_TRACE_RQST_ID)

					else
					{
						// Increment the number of messages lost. Is the buffer
						// corrupted?
						(rvt_lost_msg_cpt.bit_mask).count++;
						(rvt_lost_msg_cpt.bit_mask).unknown_request = 1;
					}

				} // End of if (msg != NULL)

				else
				{
					// Increment the number of messages lost. Is the buffer
					// corrupted?
					(rvt_lost_msg_cpt.bit_mask).count++;
					(rvt_lost_msg_cpt.bit_mask).message_empty = 1;
				}

			}  // End of if (event & EVENT_MASK (RVT_TRACE_MAILBOX))

			if ((event & (RVF_TIMER_0_EVT_MASK)) && (p_rvt_sys_time))
			{
				UINT8   count           = 0;
				UINT8   sys_time_length = RVT_SYS_TIME_LENGTH;
				UINT32  current_time    = rvf_get_tick_count ();

				// Append with the system time
				for (count = 0;
					 count < RVT_HEX_VALUE_LENGTH;
					 count++)
				{
					p_rvt_sys_time[sys_time_length + count] = Num2Char[(UINT8) ((current_time << (count << 2)) >> 28)];	
				}
				sys_time_length += RVT_HEX_VALUE_LENGTH;

				// Append with the '\n' and '\r' characters for the hyper terminal
				p_rvt_sys_time[sys_time_length++] = '\n';
				p_rvt_sys_time[sys_time_length++] = '\r';

				// Send message to the UART without byte stuffing
				nb_bytes_sent = 0;
				while (nb_bytes_sent < sys_time_length)
				{
					nb_bytes_sent += SER_tr_WriteNChars (SER_LAYER_1,
														 (UINT8 *) p_rvt_sys_time + nb_bytes_sent,
														 sys_time_length - nb_bytes_sent);
				}

			} // End of if (event & (RVF_TIMER_0_EVT_MASK))
		}
    #endif
}	


/******************************************************************************/
/*                                                                            */
/*    Function Name:   rvt_RX_process                                         */
/*                                                                            */
/*    Purpose:         This function is called when characters are received   */
/*                     on the serial port on receive HISR.                    */
/*                                                                            */
/*    Input Parameters:                                                       */
/*        None.                                                               */
/*                                                                            */
/*    Output Parameters:                                                      */
/*        None.                                                               */
/*                                                                            */
/*    Global Parameters:                                                      */
/*        None.                                                               */
/*                                                                            */
/*    Note:                                                                   */
/*        None.                                                               */
/*                                                                            */
/******************************************************************************/
void rvt_RX_process (void)
{
	UINT32        bytesRead;
	static UINT8  inBuffer[255];

/*
	rvf_send_trace("Ser RX", 6, rvf_get_tick_count(),
			RV_TRACE_LEVEL_DEBUG_HIGH, RVT_USE_ID);
	freecalypso_raw_dbgout("*Ser_RX");
	freecalypso_lldbg_intinfo();
*/

    #ifdef FRAMING_PROTOCOL
	BOOL                      eof              = 0;
	static UINT8              rcv_state        = RVT_WAIT_FOR_HEADER;
	static UINT32             total_bytesRead  = 0;
	static RVT_CALLBACK_FUNC  rx_callback_func = NULL;

	// Get all bytes from the UART RX FIFO
	for (; ; )
	{
	    // Read and destuff the UART RX FIFO and fill inBuffer with received
	    // bytes
	    bytesRead = SER_tr_ReadNBytes (SER_LAYER_1,
					  (char *) (inBuffer + total_bytesRead),
					   sizeof (inBuffer) - total_bytesRead,
					   &eof);

	    // Check for the header. Hence, get the sendee
	    if ((rcv_state == RVT_WAIT_FOR_HEADER) && \
		(bytesRead) && \
		!(total_bytesRead))
	    {
		if ((inBuffer[0] >= RVT_RV_HEADER) &&
		    (inBuffer[0] <= RVT_KEEPALIVE_HEADER))
		{
		    UINT8  idtab = 0;

		    // Search for the ID in the table
		    for (idtab = 0;
		    	 rvt_user_db[idtab].user_id != RVT_INVALID_HEADER;
		    	 idtab++)
		    {
			if (rvt_user_db[idtab].user_id == inBuffer[0])
			{
			    rx_callback_func =
					rvt_user_db[idtab].rx_callback_func;
			    break;
			}
		    }
		    rcv_state = RVT_WAIT_FOR_DATA;
		}
	    }

	    // Update the total number of bytes read,
	    // regarding the current frame 
	    total_bytesRead += bytesRead;

	    // Call the corresponding callback function when a complete message
	    // is received (eof odd)
	    if (eof & 0x01)
	    {
		// Invoke the callback function
		if (rx_callback_func != NULL)
		{
		    rx_callback_func ((T_RVT_BUFFER) (inBuffer + 1),
					total_bytesRead - 1);
		    rx_callback_func = NULL;
		}

		// Wait for the next frame to come
		rcv_state       = RVT_WAIT_FOR_HEADER;
		total_bytesRead = 0;
	    }

	    // inBuffer may be full due to some synchro lost problems
	    else if (total_bytesRead == sizeof (inBuffer))
	    {
		// If still waiting for the header, discard received characters
		// and reset static variables for the next frame to come
		if (rcv_state == RVT_WAIT_FOR_HEADER)
		{
		    total_bytesRead = 0;
		    break;
		}

		// Just discard characters received as payload
		total_bytesRead = sizeof (inBuffer[0]);
	    }

	    // Proceed with the next concatenated frame whether more bytes left
	    if (eof > 1)
	    {
		continue;
	    }
	    break;
	}
    #else

	// Read the UART RX FIFO and fill inBuffer with received bytes
	bytesRead = SER_tr_ReadNChars (SER_LAYER_1,
					(char *) inBuffer,
					sizeof (inBuffer));

	// Invoke the Testmode callback function : this is the only one able,
	// for the moment, to send an external command !!  WARNING : This
	// should be the same name than the one already registered in
	// create_RVtasks.c.
	tm_receive ((T_RVT_BUFFER) inBuffer, bytesRead);
    #endif
}



/******************************************************************************/
/*                                                                            */
/*    Function Name:   rvt_activate_RX_HISR                                   */
/*                                                                            */
/*    Purpose:         This function is called when an RX interrupt occurs.   */
/*                                                                            */
/*    Input Parameters:                                                       */
/*        None.                                                               */
/*                                                                            */
/*    Output Parameters:                                                      */
/*        None.                                                               */
/*                                                                            */
/*    Global Parameters:                                                      */
/*        None.                                                               */
/*                                                                            */
/*    Note:                                                                   */
/*        None.                                                               */
/*                                                                            */
/******************************************************************************/
void rvt_activate_RX_HISR (void)                                    
{                                                         
	NU_Activate_HISR (&TI_rcv_HISR);
}
