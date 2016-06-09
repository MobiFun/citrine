/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CUS_CC
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
|  Purpose :  Custom dependent definitions for CC of the
|             mobile station
|             
|             Use this header for definitions to integrate the
|             protocol stack entity CC in your target system !
+----------------------------------------------------------------------------- 
*/ 

#ifndef CUS_CC_H
#define CUS_CC_H

/*==== CONSTANTS ==================================================*/
/*
 * DTMF_BUF_SIZE
 *
 * Description :  The constant defines the maximum of
 *                temporary stored DTMF digits -1 in CC.
 */
#if defined (WIN32)
#define DTMF_BUF_SIZE            3
#else
#define DTMF_BUF_SIZE            20
#endif

/* 
 * MAX_SETUP_ATTEMPTS
 *
 * Number of setup attempts until upper layers are informed. 
 * This may be useful for networks which aren't very stable.
 * All values other than 1 are regarded as dirty trick.
 */
#ifdef  WIN32 // Change this to suit your needs
#define MAX_SETUP_ATTEMPTS       2
#else
#define MAX_SETUP_ATTEMPTS       1 
#endif
 
/*
 * TIMER_VALUES
 *
 * Description :  The constants define the timer values
 *                for the CC timers
 */

/* unit for the timer values is 1 ms */
#define T303_VALUE             30000
#define T305_VALUE             30000
#define T308_VALUE             30000
#define T310_VALUE             30000
#define T313_VALUE             30000
#define T323_VALUE             30000
#define T332_VALUE             30000
#define T335_VALUE             30000
#define T336_VALUE             10000
#define T337_VALUE             10000

/*
 * MAX_CC_CALLS
 *
 * Description :  The constant define the number of parallel
 *                cc call instances per subscriber.
 */

#if defined (WIN32)
#define MAX_CC_CALLS  4
#else
#define MAX_CC_CALLS  7
#endif

/*
 * MAX_CC_TIMER
 *
 * Description :  The constant define the number of timer
 *                available in the timer pool.
 * Note        :  There are only two timer necessary for each CC instance, 
 *                one for the supervision of the CC states and 
 *                one for the supervision of DTMF (T336, T337).
 */

#define MAX_CC_TIMER (MAX_CC_CALLS * 2)

/*
 * FRAME_OFFSET_ZERO
 *
 * Description :  If the constant is set, PL ensures that all frames
 *                are send with offset zero to DL. This enables a
 *                faster access on frame header parameter.
 */

#undef FRAME_OFFSET_ZERO

/*
 * MAX_ERROR_TAGS
 *
 * Description :  The constant define the maximum number of
 *                stored information elements for the
 *                diagnostic field of the cc cause element.
 */

#define MAX_ERROR_TAGS    5

/*
 * MAX_STORED_PRIM
 *
 * Description :  The constant define the maximum number of
 *                stored primiitives in call control.
 */

#define MAX_STORED_PRIM    4

/*
 * VERSION
 *
 * Description :  The constants define the type and the value
 *                of a version identification. The version
 *                is part of the monitor struct.
 */

#define T_VERSION   char
#define VERSION_CC  "CC 1.0"

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
#define VSI_CALLER cc_handle,
#define VSI_CALLER_SINGLE cc_handle
#else
#define VSI_CALLER
#define VSI_CALLER_SINGLE
#endif

#endif

