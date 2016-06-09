/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS
|  Modul   :  CUS_ALR
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
|  Purpose :  Custom dependent definitions for TI-Add on of the
|             mobile station
|             
|             Use this header for definitions to integrate the
|             protocol stack entity TI-Add on in your target system !
+----------------------------------------------------------------------------- 
*/ 
#ifndef CUS_ALR_H
#define CUS_ALR_H

/*==== CONSTANTS ==================================================*/

/*
 * FAST_SEARCH_ATTEMPTS
 *
 * Description :  The constant defines the number of single shots
 *                to measure the fieldstrength of all channels.
 *                for Fast Search Mode.
 */
#define FAST_SEARCH_MODE_ATTEMPTS   1

/*
 * BLACK_LIST_SEARCH_ATTEMPTS
 *
 * Description :  The constant defines the number of single shots
 *                to measure the fieldstrength of all channels.
 *                for Black List Search Mode.
 */
#define BLACK_LIST_SEARCH_MODE_ATTEMPTS   1

/*
 * FAST_SEARCH_POWERMEAS_VALUE
 *
 * Description :  The constant defines the spreading time
 *                to measure the fieldstrength of all channels.
 *                for Fast Search Mode.
 */
#define TIM_FAST_SEARCH_POWERMEAS_VAL 800

/*
 * BLACK_LIST_SEARCH_POWERMEAS_VALUE
 *
 * Description :  The constant defines the spreading time
 *                to measure the fieldstrength of all channels.
 *                for Black List Search Mode.
 */
#define TIM_BLACK_LIST_SEARCH_POWERMEAS_VAL 800

/*
 * VERSION
 *
 * Description :  The constants define the type and the value
 *                of a version identification. The version
 *                is part of the monitor struct.
 */


#define T_VERSION   char
#define VERSION_ALR  "ALR 1.0"

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
#define VSI_CALLER pl_handle,
#define VSI_CALLER_SINGLE pl_handle
#else
#define VSI_CALLER
#define VSI_CALLER_SINGLE
#endif

#else

#ifdef OPTION_MULTITHREAD
#define VSI_CALLER PL_NAME,
#define VSI_CALLER_SINGLE PL_NAME
#else
#define VSI_CALLER
#define VSI_CALLER_SINGLE
#endif
#endif

#endif
