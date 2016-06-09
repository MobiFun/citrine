/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (8410)
|  Modul   :  CUS_MM
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
|  Purpose :  Custom dependent definitions for MM of the
|             mobile station
|
|             Use this header for definitions to integrate the
|             protocol stack entity MM in your target system !
+-----------------------------------------------------------------------------
*/

#ifndef CUS_MM_H
#define CUS_MM_H

/*==== CONSTANTS ==================================================*/
/*
 * TIMER_VALUES
 *
 * Description :  The constants define the timer values
 *                for the MM timer TXXX depending on the various
 *                channel types and service access point identifiers.
 *                If your target system uses other units please
 *                change the values.
 */

#if defined (NEW_FRAME)
/*
 * all timer values in milliseconds
 */
#define T_3210_VALUE           20000
#define T_3211_VALUE           15000
#define T_3213_VALUE            4000
#define T_3220_VALUE            5000
#define T_3230_VALUE           15000
#define T_3240_VALUE           10000
#ifdef REL99
#define T_3241_VALUE           300000
#endif
#define T_REG_VALUE           360000
#define T_T3212_TIC_VALUE      10000

#define TICS_PER_DECIHOURS  (1000*60*6)

#else

#if defined (_TMS470)

/*
 * Unit is 4.615 ms (TDMA frame)
 */
#define T_3210_VALUE             4337
#define T_3211_VALUE             3250
#define T_3213_VALUE              867
#define T_3220_VALUE             1083
#define T_3230_VALUE             3250
#define T_3240_VALUE             2167
#ifdef REL99
#define T_3241_VALUE             65005
#endif
#define T_REG_VALUE             78006
#define T_T3212_TIC_VALUE        2168

#define TICS_PER_DECIHOURS      (216*60*6)

#else

/*
 * Unit is ca. 50 ms
 */
#define T_3210_VALUE             400
#define T_3211_VALUE             300
#define T_3213_VALUE              80
#define T_3220_VALUE             100
#define T_3230_VALUE             300
#define T_3240_VALUE             200
#ifdef REL99
#define T_3241_VALUE             6000
#endif
#define T_REG_VALUE             7200
#define T_T3212_TIC_VALUE        200

#define TICS_PER_DECIHOURS      (20*60*6)

#endif /* _TMS470 */

#endif /* NEW_FRAME */
/*
 * TIMER IDENTIFIER
 *
 * Description :  The constants define the identifier of the
 *                timer resources of MM.
 */

#define T_REG_NAME   "T_REG"
#define T3210_NAME   "T3210"
#define T3211_NAME   "T3211"
#define T3212_NAME   "T3212"
#define T3213_NAME   "T3213"
#define T3220_NAME   "T3220"
#define T3230_NAME   "T3230"
#define T3240_NAME   "T3240"
#ifdef REL99
#define T3241_NAME   "T3241"
#endif
#define T_HPLMN_NAME "T_HPLMN"

/*
 * MAX_MM_TIMER
 *
 * Description :  The constant define the number of timer
 *                available in the timer pool.
 *
 * NOTE: This constant is only used for old frame.
 */

#define MAX_MM_TIMER 5

/*
 * MAX_STORE_ENTRIES
 *
 * Description :  The constant defines the number of store
 *                entries of the call establishment and timeout
 *                administration in MM.
 *                available in the timer pool.
 */

#define MAX_STORE_ENTRIES 10

/*
 * INTERNAL REDIAL
 *
 * Description :  If no test SIM card is inserted, but a SIM card an
 *                internal redial is tried if the connection establishment
 *                has failed due to random access failed. The constant
 *                define the number of reattempts.
 */

#define INTERNAL_REDIAL 1

/*
 * VERSION
 *
 * Description :  The constants define the type and the value
 *                of a version identification. The version
 *                is part of the monitor struct.
 */


#define T_VERSION   char
#define VERSION_MM  "MM 1.0"

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

#if defined (NEW_FRAME)

#ifdef OPTION_MULTITHREAD
#define VSI_CALLER mm_handle,
#define VSI_CALLER_SINGLE mm_handle
#else
#define VSI_CALLER
#define VSI_CALLER_SINGLE
#endif

#else

#ifdef OPTION_MULTITHREAD
#define VSI_CALLER MM_NAME,
#define VSI_CALLER_SINGLE MM_NAME
#else
#define VSI_CALLER
#define VSI_CALLER_SINGLE
#endif
#endif /*NEW_FRAME */

#endif

