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
|  Purpose :  Types definitions for the real time clock driver
|             .
+----------------------------------------------------------------------------- 
*/ 

#ifndef DEF_RTC_H
#define DEF_RTC_H

/*
 * type definitions
 */
typedef struct rtc_time_type
{
  UBYTE   year;
  UBYTE   month;
  UBYTE   day;
  UBYTE   hour;
  UBYTE   minute;
  UBYTE   second;
} rtc_time_type;

/*
 * Prototypes
 */
EXTERN UBYTE rtc_read_time ( rtc_time_type *rtc_time );

#endif /* #ifndef DEF_RTC_H */
