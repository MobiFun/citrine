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
|  Purpose :  Custom dependent definitions for RR of the
|             mobile station
+-----------------------------------------------------------------------------
*/

#ifndef CUS_RR_H
#define CUS_RR_H

/*==== CONSTANTS ==================================================*/
/*
 * TIMER_VALUES
 *
 * Description :  The constants define the timer values
 *                for the RR timer TXXX depending on the various
 *                channel types and service access point identifiers.
 *                If your target system uses other units please
 *                change the values.
 */

/*
 * all values in milliseconds independent from the target
 */
#define TRESELECT_VALUE         10000
#define TABORT_VALUE            10000
#define T3110_VALUE             500
#define T3110_SDCCH_VALUE       900
#define T3126_VALUE             2500
#define T3122_VALUE(a)          (1000*(a))
#define T_PLMN_SEARCH_VALUE     60000

/* CSI-LLD section:4.1.3.2  
 * Search mode timer values in seconds
 */
#define TFAST_CS_VALUE          240000
#define TNORMAL_CS_VALUE        240000 


#define ONE_SEC                 1000
#define TEN_SEC                 10000
#define TWENTY_SEC              20000
#define THIRTY_SEC              30000
#define SIXTY_SEC               60000
#define TWO_MIN                 120000
#define SIX_MIN                 360000

#ifdef TI_PS_FF_AT_P_CMD_CTREG
#define T_TEN_SEC                 1
#define T_TWENTY_SEC              2
#define T_THIRTY_SEC              3
#define T_SIXTY_SEC               6
#define T_TWO_MIN                 12
#define T_SIX_MIN                 36
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
/*
 * TIMER IDENTIFIER
 *
 * Description :  The constants define the identifier of the
 *                timer resources of RR.
 */

#define FOUR_FRAMES             4

#define T3110_NAME     "T3110"
#define T3122_NAME     "T3122"
#define T3126_NAME     "T3126"
#define TRESELECT_NAME "TRESELECT"
#define TREG_NAME      "TREG"
#define TABORT_NAME    "TABORT"

/*
 * MAX_RR_TIMER
 *
 * Description :  The constant define the number of timer
 *                available in the timer pool.
 */

#define MAX_RR_TIMER 6

/*
 * PERIODS OF MEASUREMENT REPORTS
 *
 * Description :  The constants defines the time between
 *                measurement reports in idle or dedicated
 *                mode as a multiple of 480 ms.
 */
#define PERIODS_OF_480MS_DEDICATED 1
#define PERIODS_OF_480MS_IDLE      2

/*
 * MEDIUM_RXLEV_THRESHOLD
 *
 * Description :  The constant defines the RxLev threshold for reasonably
 *                strong carriers
 */

#define MEDIUM_RXLEV_THRESHOLD   10

/*
 * UPPER_RXLEV_THRESHOLD
 *
 * Description :  The constant defines the Upper RxLev threshold for
 *                strong carriers
 */

#define UPPER_RXLEV_THRESHOLD   20

/*
 * MAX_LAI
 *
 * Description :  The constant defines the length of the forbidden
 *                LAI list.
 */

#define MAX_LAI   10

/*
 * MAX_RR_STORED_PRIM
 *
 * Description :  The constant defines the number of primitives stored
 *                in radio resource in parallel.
 */


/*
 * DG 1.8.2000
 * #define MAX_RR_STORED_PRIM 5
 * Forum G23M-0087, more than 5 prims to store
 */

#define MAX_RR_STORED_PRIM 10


/*
 * FRAME_OFFSET_ZERO
 *
 * Description :  If the system ensures that the frame offset
 *                for RR message is 24 bits, that means DL
 *                start at offset zero, this option can be used
 *                for an optimized access to the SDU.
 */

#undef FRAME_OFFSET_ZERO

/*
 * C1 OFFSET
 *
 * Description :  If no test sim card is inserted, this offset
 *                is added to the calculated c1 value. This
 *                ensures coverage for mobiles with a RF
 *                at the lower border of sensitivity.
 */

#define C1_OFFSET    4

/*
 * DELAY BEFORE RESET IN CASE NO CONFIGURATION DATA AVAILABLE
 *
 * Description :  At system start the software tries to read configuration
 *                data from the flash [file system (FFS)] to allow behaviour
 *                fitting to a customer's configuration, e.g. in terms of
 *                HW suport (supported frequency bands, power class etc...)
 *                If no such data can be found the system will reset after
 *                the dalay specified below (unit is milliseconds). A customer
 *                might use this time to download the configuration to the
 *                flash.
 *                Introduced with ConQuest Issue 7510, project G23M.
 */

#define DELAY_RESET_NO_CONFIG_DATA 30000

/*
 * DELAY BETWEEN NON PARALLEL SEARCH IN LIMITED SERVICE IN ALL FREQ AREA
 *
 * Description :  When MS enters Limited Service in all freqqurncy are, 
 *                Non Parallel search must be used to get back to Full Service.
 *                The following constant identifies the time gap between Non 
 *                Parallel search attempts in such a situation following TREG 
 *                timer expiry. This time gap is used to allow time for emegency
 *                calls to the user. 
 */

#define DELAY_NON_PAR_SEARCH_LIM_SER TWO_MIN

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
#define VSI_CALLER rr_handle,
#define VSI_CALLER_SINGLE rr_handle
#else
#define VSI_CALLER
#define VSI_CALLER_SINGLE
#endif

#endif
