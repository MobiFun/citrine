/****************************************************************************/
/*                                                                          */
/*  Name        rvf_trace_adapt.c                                           */
/*                                                                          */
/*  Function    this file is used to trace messages if TRACE module is      */
/*              activated or not                                            */
/*                                                                          */
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  03/19/2001		Create													*/
/*																			*/
/*	Author		Pascal Puel (p-puel@tif.ti.com)								*/
/*																			*/
/* (C) Copyright 1999 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../include/config.h"

#include "../rv/general.h"
#include "../rv/rv.h"
#include "../rv/rv_general.h"
#include "rvf_api.h"

#include "../rvt/rvt_gen.h"

#include "../rvm/rvm_use_id_list.h"

#include <string.h>

/* DAR files used to redirect trace to DAR entity */
//#ifdef RVM_DAR_SWE
//	#include "dar_api.h"
//	#include "dar_gen.h"
//	#include "dar_structs_i.h"
//	#include "dar_diagnose_i.h"

	/* Define a pointer to the DAR Global Environment Control block   */
//	extern T_DAR_ENV_CTRL_BLK *dar_gbl_var_p;
//#endif

/*
 * FreeCalypso: the configuration we've got with the Sotomodem semi-src
 * has this in g23m/__out__/gsm_<blah>/config/trace.cfg:
 */

#define LAYER_DBG 0xFFFFFFFF
#define TRACE_LEVEL_FILTER 5

/*
 * The above configuration corresponds to all trace output
 * being enabled by default.
 */

UINT8 rvf_trace_level = TRACE_LEVEL_FILTER;
UINT32 rvf_layer_mask = LAYER_DBG;

/* Decimal to hexadecimal conversion table */
static const char Num2Char[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

extern T_RVT_USER_ID rv_trace_user_id;

#ifndef FRAMING_PROTOCOL
void rvf_send_trace1 (char * msg, UINT8 msg_length, UINT32 val, UINT8 TRACE_LEVEL, UINT32 swe_use_id)
{	
	UINT32 trace_type = swe_use_id;

	/* Apply the appropriate compilation flags to filter trace messages based on
	their type and level */
	if ((TRACE_LEVEL < RV_TRACE_LEVEL_WARNING) ||
		((TRACE_LEVEL <= rvf_trace_level) &&
		 (((trace_type & rvf_layer_mask & 0x0000FFFF) == (trace_type & 0x0000FFFF)) ||
		  ((trace_type & rvf_layer_mask & 0xFFFF0000) != (trace_type & 0xFFFF0000)))))
		{
			char * buff;

			if (val == NULL_PARAM)
			{
				rvt_mem_alloc (rv_trace_user_id, msg_length, (T_RVT_BUFFER *)&buff);
				if (buff != NULL) /* Check if there is enough memory for the buffer */
				{
					/* Copy the body of the message */
					memcpy( buff, msg, msg_length);
					rvt_send_trace_no_cpy( buff, rv_trace_user_id, msg_length, RVT_ASCII_FORMAT);
				}
			}
			else
			{
				rvt_mem_alloc (rv_trace_user_id, msg_length +11, (T_RVT_BUFFER *)&buff);
				if (buff != NULL) /* Check if there is enough memory for the buffer */
				{	
					UINT8 i;
				
					/* Copy the body of the message */
					memcpy( buff, msg, msg_length);

					((char *)buff)[msg_length]		= ' ';
					((char *)buff)[msg_length + 1]	= '0'; 
					((char *)buff)[msg_length + 2]	= 'x';
					
					for (i=0; i<8; i++)
					{
						((char *)buff)[msg_length+3+i] = Num2Char[(UINT8)((val<<(i<<2))>>28)];
					}
					rvt_send_trace_no_cpy(buff, rv_trace_user_id, msg_length + 11, RVT_BINARY_FORMAT);
				}
			}
		}
}
#else
/********************************* VERSION WITH TRACE MUX **************************/

void rvf_send_trace1 (char * msg, UINT8 msg_length, UINT32 val, UINT8 TRACE_LEVEL, UINT32 swe_use_id)
{	
	UINT32 trace_type = swe_use_id;

	/* Apply the appropriate compilation flags to filter trace messages based
	on their type and level */
	if ((TRACE_LEVEL < RV_TRACE_LEVEL_WARNING) ||
		((TRACE_LEVEL <= rvf_trace_level) &&
		 (((trace_type & rvf_layer_mask & 0x0000FFFF) == (trace_type & 0x0000FFFF)) ||
		  ((trace_type & rvf_layer_mask & 0xFFFF0000) != (trace_type & 0xFFFF0000)))))
		{
			char * buff;

			if (val == NULL_PARAM)
			{
				rvt_mem_alloc (rv_trace_user_id, msg_length+5, (T_RVT_BUFFER *)&buff);

				if (buff != NULL) /* Check if there is enough memory for the buffer */
				{
					/* Add the trace type (MSB and LSB) and Trace level	*/
					buff [0] = (char) (trace_type >> 24);
					buff [1] = (char) (trace_type >> 16);
					buff [2] = (char) (trace_type >> 8);
					buff [3] = (char) (trace_type & 0xff);
					buff [4] = (char) TRACE_LEVEL;

					/* Copy the message in the new buffer */
					memcpy(buff+5, msg, msg_length);
					
					/* Send the trace message */
					rvt_send_trace_no_cpy ((T_RVT_BUFFER) buff, rv_trace_user_id, msg_length+5, RVT_BINARY_FORMAT);
				}
			}
			else
			{
				rvt_mem_alloc (rv_trace_user_id, msg_length+16, (T_RVT_BUFFER *)&buff);

				if (buff != NULL) /* Check if there is enough memory for the buffer */
				{	
					UINT8 i;

					/* Add the trace type (MSB and LSB) and Trace level	*/
					buff [0] = (char) (trace_type >> 24);
					buff [1] = (char) (trace_type >> 16);
					buff [2] = (char) (trace_type >> 8);
					buff [3] = (char) (trace_type & 0xff);
					buff [4] = (char) TRACE_LEVEL;

					/* Copy the message in the new buffer */
					memcpy( buff + 5, msg, msg_length);
					
					buff[5+msg_length]		= ' ';
					buff[5+msg_length + 1]	= '0'; 
					buff[5+msg_length + 2]	= 'x';
					
					for (i=0; i<8; i++)
					{
						((char *)buff)[8+msg_length+i] = Num2Char[(UINT8)((val<<(i<<2))>>28)];
					}

					/* Send the trace message */
					rvt_send_trace_no_cpy ((T_RVT_BUFFER) buff, rv_trace_user_id, msg_length+16, RVT_BINARY_FORMAT);
				}
			}
		}
}
#endif
