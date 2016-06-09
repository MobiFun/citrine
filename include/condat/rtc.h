/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM (6301)
|  Modul   :  
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
|  Purpose :  Type definitions and function prototypes for the real time clock driver
|             SPR 1725, re-wrote file for new RTC driver implementation.
+----------------------------------------------------------------------------- 
*/ 

#ifndef DEF_RTC__H
#define DEF_RTC__H
/*==== INCLUDES ===================================================*/
#include <string.h>
#include "typedefs.h"
#include "gdi.h"
#include "kbd.h"
/*==== EXPORT =====================================================*/
/*
 * type definitions
 */


 typedef enum 
{
	RTC_TIME_FORMAT_12HOUR,
	RTC_TIME_FORMAT_24HOUR
} T_RTC_TIME_FORMAT;

typedef struct {
	UBYTE	day;
	UBYTE	month;
	USHORT	year;
} 	T_RTC_DATE;


typedef struct
{	UBYTE	minute;
	UBYTE	hour;
	UBYTE   second;
	T_RTC_TIME_FORMAT	format;
	BOOL	PM_flag;
} T_RTC_TIME;

typedef void (*RtcCallback) (void*);  /* RTC event handler        */

/*
 * Prototypes
 */


BOOL rtc_clock_cleared();// wrapper for RTC_RtcReset();

UBYTE rtc_set_time_date(T_RTC_DATE* date, T_RTC_TIME* time); // wrapper for  RTC_setTimeDate();

UBYTE rtc_get_time_date(T_RTC_DATE* date, T_RTC_TIME* time); // wrapper for RTC_getTimeDate();

UBYTE rtc_set_alarm(T_RTC_DATE* date , T_RTC_TIME* time, RtcCallback callback_func );//wrapper for RTC_setAlarm();

UBYTE rtc_get_alarm(T_RTC_DATE* date, T_RTC_TIME* time);//wrapper for RTC_getAlarm();

UBYTE rtc_unset_alarm();//wrapper for RTC_UnsetAlarm();

UBYTE rtc_set_time_format(T_RTC_TIME_FORMAT format);//wrapper for RTC_Set12HourMode();





#endif /* #ifndef DEF_RTC_H */
