/****************************************************************************/
/*                                                                          */
/*  Name        rvf_target.h                                                */
/*                                                                          */
/*  Function    this file contains definitions that will probably			*/
/*              change for each target system. This includes				*/
/*              such things number of tasks,			                    */
/*              little endian/big endian conversions, etc...				*/
/*																			*/
/*                                                                          */
/*	NOTE        This file should always be included first.					*/
/*																			*/
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  4/19/1999	Create														*/
/*  10/27/1999  remove declaration of ntohs, htons, ntohl, htonl			*/
/*				in order to avoid conflict with winsock.h					*/
/*  12/23/1999	add buffer and memory bank related definition				*/
/*  07/12/2000	implement dynamic memory allocation.						*/
/*  01/16/2004  Corrected conversion from seconds/milliseconds to ticks.    */
/*																			*/
/*	Author		David Lamy-Charrier (dlamy@tif.ti.com)						*/
/*																			*/
/* (C) Copyright 1999 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/
#ifndef _RVF_TARGET_H
#define _RVF_TARGET_H

#if 0 //#ifndef _WINDOWS
	#include "config/rv.cfg"
	#include "config/swconfig.cfg"
#endif

#include "../../include/config.h"

/* optimizations */
#ifdef _WINDOWS
	#define INLINE __inline
#else
	#define INLINE inline
#endif


/*----------------------------------------------------------------------*/
/*                      RVF Definitions                                 */
/*----------------------------------------------------------------------*/


/* the maximum number of memory bank created by users */
#if (!GSMLITE)
#define RVF_MAX_TOTAL_MB			70
#else // GSMLITE
  #define RVF_MAX_TOTAL_MB			20
#endif // GSMLITE

/* the maximum number of real memory bank managed */
#if (!GSMLITE)
#define RVF_MAX_REAL_MB				60
#else // GSMLITE
  #define RVF_MAX_REAL_MB			20
#endif // GSMLITE

/* max length of memory bank name */
#define RVF_MAX_MB_LEN				10

/* max length of task name */
#define RVF_MAX_TASK_LEN			10


/* RVF_MB_MAPPING defines the correspondance between mb_name and mb_id */
/* WARNING!!!! When adding new memory banks in this default list, take care */
/* to always finish with the {0,0} !!! */
#if (!GSMLITE)
#define RVF_MB_MAPPING {{ "RVM_PRIM", 0},{ "TEST1", 1}, { "TEST2", 2},\
	{ "TEST3", 3}, { "TEST4", 3},\
	{"HCI_RX", 4},{"HCI_TX", 5},\
	{"ATP_PRIM",27},{"ATP_UART",27},\
	{"RV_TRACE", TRACE_MB_ID},\
	{"UWIRE_PRIM",37},{0,0}}
#else // GSMLITE
  #define RVF_MB_MAPPING {{ "RVM_PRIM", 0},{ "TEST1", 1}, { "TEST2", 2},\
						  { "TEST3", 3}, { "TEST4", 3}, {0,0}}
#endif // GSMLITE

/* set the following variable to 1 to check for buffer corruption*/
#define RVF_ENABLE_BUF_CORRUPTION_CHECK	1

/* set the following variable to 1 to check for buffer linkage*/
#define RVF_ENABLE_BUF_LINKAGE_CHECK	1

/* enable or disable memory statistics functions */
#define RVF_ENABLE_STATS				1	


/* The maximum number of RVF tasks allowed in the system*/
#if (!GSMLITE)
#define MAX_RVF_TASKS	            30
#else // GSMLITE
  #define MAX_RVF_TASKS	            11
#endif // GSMLITE

/* max number of callback functions for buffers waiting to be counted on another memory bank */
#if (!GSMLITE)
#define RVF_MAX_WAITING_BUF		300 
#else // GSMLITE
  #define RVF_MAX_WAITING_BUF		 20 
#endif // GSMLITE
/* sum of RVF_BUFi_MAX */







/***********************************************************************
** Timer related definitions. These should be edited per system. The
** macros should convert milliseconds and seconds to system ticks.
** Applications should use these to calculate the number of ticks to
** sleep for.
*/

#ifdef _WINDOWS
#define RVF_MS_TO_TICKS(_x)   ((_x) / 50)
#define RVF_SECS_TO_TICKS(_x) (((_x) * 1000) / 50)
#define RVF_TICKS_TO_MS(_x)   ((_x) * 50)
#else
/*
 * To avoid floating point computation, the tick duration 4.615 ms can be
 * resolved to the fraction : 923/200.
 *
 * Due to the multiplication with 200, the maximum usable value for
 * milliseconds is 2^32 / 200 = 21,474,836. The maximum usable value
 * for seconds is 2^32 / (200 * 1000) = 21,474.
 * Both values correspond approximately to 5h 57m 54s.
 */
#define RVF_MS_TO_TICKS(_x)   ((((_x) * 200) + 462) / 923) 
#define RVF_SECS_TO_TICKS(_x) ((((_x) * 1000 * 200) + 462) / 923)
#define RVF_TICKS_TO_MS(_x)   ((((_x) * 923) + 100) / 200)
#endif

#endif /* _RVF_TARGET_H */
