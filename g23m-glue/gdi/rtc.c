/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS
|  Modul   :  DRV_RTC
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
|  Purpose :  This Module defines the real time clock
|             driver interface for the G23 protocol stack.
			MC SPR 1725, re-wrote for new driver
+----------------------------------------------------------------------------- 
*/ 

#ifndef DRV_RTC_C
#define DRV_RTC_C

#define ENTITY_CST
/*==== INCLUDES ===================================================*/

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#include <string.h>
#include "typedefs.h"
#include "vsi.h"
#include "custom.h"
#include "prim.h"
#include "gsm.h"
#include "tok.h"
#include "../cst/cst.h"
#include "gdi.h"
#include "rtc.h"

#ifdef _SIMULATION_
#define _WINDOWS
#endif

#include "../../riviera/rvf/rvf_api.h"

#ifndef _SIMULATION_
#include "../../bsp/rtc/rtc_api.h"
#endif
#include "rtcdrv.h"
/*==== EXPORT =====================================================*/
/*==== VARIABLES ==================================================*/



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)   MODULE  : DRV_RTC                        |
| STATE   : code            ROUTINE : rtc_clock_cleared                 |
+--------------------------------------------------------------------+

  PURPOSE : returns true if clcok has been reset

            
*/
BOOL rtc_clock_cleared()
{
#ifndef _SIMULATION_
	return RTC_RtcReset();
#endif /* _SIMULATION_ */
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)   MODULE  : DRV_RTC                        |
| STATE   : code            ROUTINE : rtc_set_time_date                  |
+--------------------------------------------------------------------+

  PURPOSE : set the time and date

            
*/
UBYTE rtc_set_time_date(T_RTC_DATE* date, T_RTC_TIME* time )
{
#ifndef _SIMULATION_
	T_RTC_DATE_TIME time_and_date;

	time_and_date.second = 0;
	time_and_date.minute = time->minute;
	if (time->format== RTC_TIME_FORMAT_12HOUR)  
		time_and_date.hour = (time->hour)% 12;
	else
		time_and_date.hour = time->hour;
	time_and_date.second= time->second;
	time_and_date.day = date->day;
	time_and_date.month = date->month;
	time_and_date.wday = 3;
	time_and_date.year = (date->year - 2000) %100;
	if (time->format== RTC_TIME_FORMAT_12HOUR)
		time_and_date.mode_12_hour = TRUE;
	else
		time_and_date.mode_12_hour = FALSE;
	
	time_and_date.PM_flag = time->PM_flag % 2;
	
		
	return RTC_SetDateTime(time_and_date);
#endif /* _SIMULATION_ */

}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)   MODULE  : DRV_RTC                        |
| STATE   : code            ROUTINE : rtc_get_time_date                  |
+--------------------------------------------------------------------+

  PURPOSE : get the time and date

            
*/
UBYTE rtc_get_time_date(T_RTC_DATE* date , T_RTC_TIME* time)
{
#ifndef _SIMULATION_
	T_RTC_DATE_TIME time_and_date;
	
	if (RTC_GetDateTime(&time_and_date) != RVF_OK)
		DRV_INTERNAL_ERROR; /*error*/
	date->year = 2000+ time_and_date.year;
	date->month = time_and_date.month;
	date->day = time_and_date.day;

	time->minute = time_and_date.minute;
	time->hour = time_and_date.hour;
	time->second = time_and_date.second;
	if ( time_and_date.mode_12_hour == TRUE)
		time->format = RTC_TIME_FORMAT_12HOUR;
	else
		time->format = RTC_TIME_FORMAT_24HOUR;
	time->PM_flag = time_and_date.PM_flag;
	return DRV_OK;
#endif /* _SIMULATION_ */
}

void empty_callback_func(void* data)
{
	TRACE_EVENT("empty_callback_func");

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)   MODULE  : DRV_RTC                        |
| STATE   : code            ROUTINE : rtc_set_alarm                 |
+--------------------------------------------------------------------+

  PURPOSE : set the alarm

            
*/
UBYTE rtc_set_alarm(T_RTC_DATE* date , T_RTC_TIME* time, RtcCallback callback_func )
{
#ifndef _SIMULATION_
  T_RTC_DATE_TIME time_and_date;
	T_RV_RETURN return_path;

	memset(&return_path, 0, sizeof(T_RV_RETURN));

	
	time_and_date.second = time->second;
	time_and_date.minute = time->minute;
	if (time->format== RTC_TIME_FORMAT_12HOUR)  
		time_and_date.hour = (time->hour)% 12;
	else
		time_and_date.hour = time->hour;
	time_and_date.day = date->day;
	time_and_date.month = date->month;
	time_and_date.wday = 3;
	time_and_date.year = (date->year - 2000) %100;
	if (time->format== RTC_TIME_FORMAT_12HOUR)
		time_and_date.mode_12_hour = TRUE;
	else
		time_and_date.mode_12_hour = FALSE;
	
	time_and_date.PM_flag = time->PM_flag % 2;
	return_path.addr_id = RVF_INVALID_ADDR_ID;
	return_path.callback_func = (RtcCallback)callback_func;
	return RTC_SetAlarm(time_and_date, return_path);
#endif /* _SIMULATION_ */
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)   MODULE  : DRV_RTC                        |
| STATE   : code            ROUTINE : rtc_get_alarm            |
+--------------------------------------------------------------------+

  PURPOSE : retrieve alarm setting

            
*/
UBYTE rtc_get_alarm(T_RTC_DATE* date, T_RTC_TIME* time )
{	
#ifndef _SIMULATION_
  T_RTC_DATE_TIME time_and_date;

	if (RTC_GetAlarm(&time_and_date) != RVF_OK)
		DRV_INTERNAL_ERROR; /*ERROR*/
	date->year = 2000+ time_and_date.year;
	date->month = time_and_date.month;
	date->day = time_and_date.day;
	
	time->second = time_and_date.second;
	time->minute = time_and_date.minute;
	time->hour = time_and_date.hour;
	if ( time_and_date.PM_flag == TRUE)
		time->format = RTC_TIME_FORMAT_12HOUR;
	else
		time->format = RTC_TIME_FORMAT_24HOUR;
	time->PM_flag = time_and_date.PM_flag;
	return DRV_OK;
#endif /* _SIMULATION_ */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)   MODULE  : DRV_RTC                        |
| STATE   : code            ROUTINE : rtc_unset_alarm            |
+--------------------------------------------------------------------+

  PURPOSE : unset alarm

            
*/
UBYTE rtc_unset_alarm()
{
#ifndef _SIMULATION_
	return RTC_UnsetAlarm();
#endif /* _SIMULATION_ */
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)   MODULE  : DRV_RTC                        |
| STATE   : code            ROUTINE : rtc_set_time_format            |
+--------------------------------------------------------------------+

  PURPOSE : unset alarm

            
*/
UBYTE rtc_set_time_format(T_RTC_TIME_FORMAT format)
{	BOOL twelve_hour;

#ifndef _SIMULATION_
	if (format == RTC_TIME_FORMAT_12HOUR)
		twelve_hour = TRUE;
	else
		twelve_hour = FALSE;

	RTC_Set12HourMode(twelve_hour);
#endif /* _SIMULATION_ */

	return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)   MODULE  : DRV_RTC                        |
| STATE   : code            ROUTINE : vmd_primitive                  |
+--------------------------------------------------------------------+

  PURPOSE : This function get the date and time.
			and is backawrd compatible
            if year >= 70 then 19year
            if year <  70 then 20year
*/

GLOBAL UBYTE rtc_read_time ( rtc_time_type *rtc_time )
{	
#ifndef _SIMULATION_
  T_RTC_DATE_TIME time_and_date;
  if ( rtc_time EQ NULL )
    return ( FALSE );

  
	
	if (RTC_GetDateTime(&time_and_date) != RVF_OK)
		return FALSE;
	rtc_time->year =  time_and_date.year;
	rtc_time->month = time_and_date.month;
	rtc_time->day = time_and_date.day;

	rtc_time->minute = time_and_date.minute;
	rtc_time->hour = time_and_date.hour;
	rtc_time->second = time_and_date.second;

#endif /* _SIMULATION_ */

  return ( TRUE );
}

#endif /* #ifndef DRV_RTC_C */
