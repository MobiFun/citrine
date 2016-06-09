/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  CUS_SMS
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
|  Purpose :  Custom dependent definitions for SMS of the
|             mobile station
|             
|             Use this header for definitions to integrate the
|             protocol stack entity SMS in your target system !
+----------------------------------------------------------------------------- 
*/ 

#ifndef CUS_SMS_H
#define CUS_SMS_H

/*==== CONSTANTS ==================================================*/

/*
 * Maximum number of CP-DATA retransmissions
 */
#define MAX_RETRANS         1

/*
 * Maximum number of TL layer retransmissions,
 * 1 retransmission => 2 transmissions.
 */
#define TL_MAX_RETANS       1

/*
 * TIMER_VALUES
 *
 * Description :  The constants define the timer values
 *                for the SMS timers in milliseconds
 *
 * TR1M = 40 Seconds, TRAM = 30 Seconds, TR2M = 15 Seconds
 */
#define TR1M_VALUE      40000
#define TR2M_VALUE      15000
#define TRAM_VALUE      30000
#define TC1M_OFFSET     10000   /* gap between retransm. TC1M and TR1M */
#define TLCT_VALUE      1000
#define TMMS_VALUE      5000

#define TC1M_VALUE  ((TR1M_VALUE-TC1M_OFFSET)/(MAX_RETRANS+1))

#define TC1M_NAME         "TC1M"
#define TR1M_NAME         "TR1M"
#define TR2M_NAME         "TR2M"
#define TRAM_NAME         "TRAM"
#define TLCT_NAME         "TLCT"
#define TMMS_NAME         "TMMS"

#define T_VERSION         char
#define VERSION_SMS       "SMS 2.0"

/*
 * MAX_SMS_RECORDS
 *
 * Description :  The constant define the maximum number of SMS
 *                messages handled by SMS.
 */

// Currently we should be very careful to set the MAX_RECORD_XX to 255 as some
// loop counters are of type UBYTE - carefully to be checked.
#ifndef WIN32
#define MAX_RECORD_ME    254
#define MAX_RECORD_SIM   254
#else
#define MAX_RECORD_ME    255
#define MAX_RECORD_SIM   255
#endif /* else, #ifndef WIN32 */

/*
 * VSI_CALLER
 *
 * Description :  For multithread applications the constant VSI_CALLER
 *                must be defined to identify the calling thread of the
 *                VSI-Interface. This must be done correponding to the
 *                type of T_VSI_CALLER in VSI.H. The comma symbol is
 *                neccessary because the vsi-functions are called
 *                like this vsi_xxx (VSI_CALLER par2, par3 ...)
 */
#ifdef OPTION_MULTITHREAD
#define VSI_CALLER sms_handle,
#define VSI_CALLER_SINGLE sms_handle
#else
#define VSI_CALLER
#define VSI_CALLER_SINGLE
#endif /* else, #ifdef OPTION_MULTITHREAD */

#endif /* #ifndef CUS_SMS_H */
