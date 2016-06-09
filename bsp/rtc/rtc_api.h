/******************************************************************************/
/*                                                                            */
/*    File Name:   rtc_functions.h                                            */
/*                                                                            */
/*    Purpose:   This file contains prototypes of RTC's internal functions.   */
/*                                                                            */
/*    Note:      None.                                                        */
/*                                                                            */
/*    Revision History:                                                       */
/*    03/20/01   Laurent Sollier      Create.                                 */
/*                                                                            */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved  */
/*                                                                            */
/******************************************************************************/

#ifndef _RTC_FUNCTIONS_H_
#define _RTC_FUNCTIONS_H_

#include "../../riviera/rvf/rvf_api.h"


/******************************************************************************/
/*                                                                            */
/*   Generic functions declarations.                                          */
/*                                                                            */
/******************************************************************************/



/* Structure containing date and time */
typedef struct {
	UINT8   second;      /* seconds after the minute   - [0,59]  */
	UINT8   minute;      /* minutes after the hour      - [0,59]  */
	UINT8   hour;        /* hours after the midnight   - [0,23]  */
	UINT8   day;         /* day of the month            - [1,31]  */
	UINT8   month;       /* months                     - [01,12] */
	UINT8   year;        /* years                        - [00,99] */
	UINT8   wday;        /* days in a week               - [0,6] */
	BOOL   mode_12_hour; /* TRUE->12 hour mode ; FALSE-> 24 hour mode */
	BOOL   PM_flag;      /* if 12 hour flag = TRUE: TRUE->PM ; FALSE->AM */
} T_RTC_DATE_TIME;


/*******************************************************************************
 *
 *                               RTC_RtcReset
 * 
 * Purpose  : Indicate if aRTC reset occured.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns: TRUE if a RTC reset occurred
 *            FALSE if RTC didn't reset.
 *
 *
 ******************************************************************************/

BOOL RTC_RtcReset(void);


/*******************************************************************************
 *
 *                               RTC_GetDateTime
 * 
 * Purpose  : Get date and time information.
 *
 * Arguments: In : none.
 *            Out: date_time : contain date and time data if return value is
 *                               RVF_OK
 *
 * Returns: RVF_OK if date and time available in date_time variable
 *            RVF_NOT_READY if date and time is temporary not accessible for
 *            30 micro seconds
 *            RVF_INTERNAL_ERROR else
 *
 ******************************************************************************/

T_RVF_RET RTC_GetDateTime(T_RTC_DATE_TIME* date_time);


/*******************************************************************************
 *
 *                               RTC_SetDateTime
 * 
 * Purpose  : Set date and time in RTC module.
 *
 * Arguments: In : date_time : date and time to set in RTC module
 *            Out: none
 *
 * Returns: RVF_OK if date and time have been set in RTC module
 *            RVF_NOT_READY if date and time cannot be saved before
 *            30 micro seconds max
 *            RVF_INVALID_PARAMETER if date or/and timeformat is incorrect
 *            RVF_INTERNAL_ERROR else
 *
 *
 ******************************************************************************/

T_RVF_RET RTC_SetDateTime(T_RTC_DATE_TIME date_time);


/*******************************************************************************
 *
 *                               RTC_GetAlarm
 * 
 * Purpose  : Get date and time alarm.
 *
 * Arguments: In : none
 *            Out: date_time : contain date and time data if return value is
 *                               RVF_OK
 *
 * Returns: RVF_OK if date and time is available in date_time data
 *            RVF_INTERNAL_ERROR else
 *
 *
 ******************************************************************************/

T_RVF_RET RTC_GetAlarm(T_RTC_DATE_TIME* date_time);


/*******************************************************************************
 *
 *                               RTC_SetAlarm
 * 
 * Purpose  : Set date and time alarmin RTC module.
 *
 * Arguments: In : date_time : Date and time alarm to set in RTC module
 *                   return_path : return path used when date and time alarm is
 *                   reached. This path can be a callback function or both
 *                   task id and mailbox number
 *            Out: none
 *
 * Returns: RVF_OK if date and time alarm have been set in RTC module
 *            RVF_NOT_READY if date and time cannot be saved before
 *            30 micro seconds max
 *            RVF_INVALID_PARAMETER if date or/and time format is incorrect
 *            RVF_INTERNAL_ERROR else
 *
 *
 ******************************************************************************/

T_RVF_RET RTC_SetAlarm(T_RTC_DATE_TIME date_time, T_RV_RETURN return_path);


/*******************************************************************************
 *
 *                               RTC_UnsetAlarm
 * 
 * Purpose  : Unset alarm function.
 *
 * Arguments: In : none 
 *            Out: none
 *
 * Returns: RVF_OK if date and time alarm have been set in RTC module
 *            RVF_NOT_READY if date and time cannot be saved before
 *            30 micro seconds max
 *            RVF_INVALID_PARAMETER if date or/and time format is incorrect
 *            RVF_INTERNAL_ERROR else
 *
 *
 ******************************************************************************/

T_RVF_RET RTC_UnsetAlarm(void);


/*******************************************************************************
 *
 *                               RTC_Rounding30s
 * 
 * Purpose  : Round time to the closest minute.
 *
 * Arguments: In : none 
 *            Out: none
 *
 * Returns: none
 *
 *
 ******************************************************************************/

void RTC_Rounding30s(void);


/*******************************************************************************
 *
 *                               RTC_Set12HourMode
 * 
 * Purpose  : Set the 12 or 24 hour mode for time get by RTC_GetDateTime and
 *              RTC_GetAlarm.
 *
 * Arguments: In : 12HourMode :   TRUE if current time is on 12 hour mode
 *                                 FALSE  if current time is on 24 hour mode
 *            Out: none
 *
 * Returns: none
 *
 *
 ******************************************************************************/

void RTC_Set12HourMode(BOOL Mode12Hour);


/*******************************************************************************
 *
 *                               RTC_Is12HourMode
 * 
 * Purpose  : Define if the time given by RTC_GetAlarm and RTC_GetDateTime is in
 *               the 12 or 24 hour mode
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns: TRUE if current time is on 12 hour mode
 *            FALSE if current time is on 24 hour mode
 *
 *
 ******************************************************************************/

BOOL RTC_Is12HourMode(void);


#endif /* #ifndef _RTC_FUNCTIONS_H_ */
