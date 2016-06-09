/*
+-----------------------------------------------------------------------------
|  Project :
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
|  Purpose :  Custom dependent definitions for DL of the
|             mobile station
|
|             Use this header for definitions to integrate the
|             protocol stack entity DL in your target system !
+-----------------------------------------------------------------------------
*/

#ifndef CUS_DL_H
#define CUS_DL_H

/*==== CONSTANTS ==================================================*/
/*
 * TIMER_VALUES
 *
 * Description :  The constants define the timer values
 *                for the DL timer T200 depending on the various
 *                channel types and service access point identifiers.
 *                Value 1 means timeout, Value 0 means timer is stopped
 */

#define T200_ACTIVE   2
#define T200_EXPIRED  1
#define T200_STOPPED  0

#define T200_SDCCH_SAPI_0_CNT     (T200_EXPIRED + 1)
#define T200_FACCH_SAPI_0_CNT_FR  (T200_EXPIRED + 9)
#define T200_FACCH_SAPI_0_CNT_HR  (T200_EXPIRED + 5)
#define T200_SDCCH_SAPI_3_CNT     (T200_EXPIRED + 3)
#define T200_SACCH_SAPI_3_CNT     (T200_EXPIRED + 4)


/*
 * PRIMITIVE SIZES
 *
 * Description :  The primitive size of PH_DATA_REQ depends on the
 *                used channel. It is defined separately to achieve
 *                an optimum of transmitted bytes to physical layer.
 *                The default value which is defined here is calculated
 *                without any alignment bits.
 *                E.g. SACCH   N201        =  18
 *                             l1 header   =   2
 *                             l2 header   =   3
 *                             sdu offset  =   2
 *                             sdu length  =   2
 *                             ch_type     =   1
 *                             dummy       =   1
 *                             -----------------
 *                             total       =  29
 *
 */

#define SACCH_PRIM_SIZE     29
#define SDCCH_PRIM_SIZE     29
#define FACCH_PRIM_SIZE     29


/*
 * MAX_QUEUED_MESSAGES
 *
 * Description :  The constant define the maximum number of
 *                stored layer 3 messages in data link layer.
 */

#define MAX_QUEUED_MESSAGES    10

/*
 * VERSION
 *
 * Description :  The constants define the type and the value
 *                of a version identification. The version
 *                is part of the monitor struct.
 */

#define T_VERSION   char
#define VERSION_DL  "DL 1.0"

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
#define VSI_CALLER dl_handle,
#define VSI_CALLER_SINGLE dl_handle
#else
#ifdef OPTION_MULTITHREAD
#define VSI_CALLER DL_NAME,
#define VSI_CALLER_SINGLE DL_NAME
#else
#define VSI_CALLER
#define VSI_CALLER_SINGLE
#endif
#endif
#endif

