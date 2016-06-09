/************************************************************************************* 
 *																			         * 
 *  Name        rv_trace.h													         * 
 *																			         * 
 *  Function    this file contains trace definitions for basic layers		         * 
 *																			         * 
 *  Date        Modification                                                         * 
 *  ------------------------												         * 
 *  10/12/00	 - Create Cristian Livadiotti - c-livadiotti@ti.com			         * 
 *																			         * 
 *																			         * 
 * (C) Copyright 1999 by Texas Instruments Incorporated, All Rights Reserved         * 
 *                                                                                   *
 * --------------------------------------------------------------------------------- *
 *                                                                                   *
 *   History:                                                                        *
 *                                                                                   *
 *   10/18/2001 - Updated for R2D by Christophe Favergeon		                     * 
 *                                                                                   *
 *************************************************************************************/

#ifndef _RV_TRACE_H_
#define _RV_TRACE_H_



/* Define trace levels.                                                          */
#define RV_TRACE_LEVEL_ERROR		(1)		/* Error condition trace messages.   */
											/* Used when an unrecoverable error is found */
#define RV_TRACE_LEVEL_WARNING		(2)		/* Warning condition trace messages. */
											/* Used when an error is found but is handled properly by the code*/
#define RV_TRACE_LEVEL_DEBUG_HIGH	(3)		/* Debug messages (high debugging).  */
                                            /* high = important debug message */
#define RV_TRACE_LEVEL_DEBUG_MEDIUM	(4)		/* Debug messages.                   */
#define RV_TRACE_LEVEL_DEBUG_LOW	(5)		/* Debug messages (low debugging).   */
                                            /* low = not important debug message */

/* temporary definition of this old flag for backward compatibility */
#define TRACE_RVTEST				(0x001E0004)

#endif /* _RV_GENERAL_H_*/
