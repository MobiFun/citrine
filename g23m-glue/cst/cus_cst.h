/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        SOURCE : CUS_CST.H            |
| AUTHOR  : XXX                        VERSION: 1.0                  |
| CREATED : 01.02.99                   STATE  : code                 |
+--------------------------------------------------------------------+

   MODULE  : CUS_CST

   PURPOSE : Custom dependent definitions for the CST entity of the
             mobile station.

             Use this header for definitions to integrate the protocol
             stack entity CST in your target system.
*/

#ifndef CUS_CST_H
#define CUS_CST_H


/*==== CONSTANTS ==================================================*/
/*
 * TIMER_VALUES
 *
 * Description :  The constants define the timer values for the CST
 *                timer TXXX depending on the various channel types
 *                and service access point identifiers.
 *
 *                If your target system uses other units please
 *                change the values.
 */

/*
 * Unit is 1 ms 
 */
#define T_RX_VALUE              1000
#define TICS_PER_DECIHOURS      (1000*60*6)


/*
 * MAX_CST_TIMER
 *
 * Description :  The constant define the number of timer available
 *                in the timer pool.
 */
#define MAX_CST_TIMER 4

/*
 * VERSION
 *
 * Description :  The constants define the type and the value of a
 *                version identification. The version is part of the
 *                monitor struct.
 */
#define T_VERSION   char
#define VERSION_CST  "CST 1.0"

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
#define VSI_CALLER cst_handle,
#define VSI_CALLER_SINGLE cst_handle
#else
#define VSI_CALLER
#define VSI_CALLER_SINGLE
#endif

#endif // CUS_CST_H
