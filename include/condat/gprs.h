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
|  Purpose :  Contains common definitions for the GPRS Protocol Stack.
+-----------------------------------------------------------------------------
*/

#ifndef GPRS_H
#define GPRS_H

/*
 * The names of the GPRS entities
 */

#define ACI_NAME              "MMI"     /* according to custom.h */
#define PPP_NAME              "PPP"
#define UART_NAME             "UART"
#define PSI_NAME              "PSI"
#define SNDCP_NAME            "SND"
#define SM_NAME               "SM"
#define GMM_NAME              "GMM"
#define GSMS_NAME             "SMS"
#define SMS_NAME              "SMS"     /* according to custom.h */
#define MM_NAME               "MM"
#define LLC_NAME              "LLC"
#define GRR_NAME              "GRR"
#define GRLC_NAME             "GRLC"
#define CCI_NAME              "CCI"
#define L1_NAME               "L1"
#define RR_NAME               "RR"
#define PL_NAME               "PL"      /* according to custom.h */
#define SIM_NAME              "SIM"     /* according to custom.h */
#define TIL_NAME              "PL"
#define UPM_NAME              "UPM"
#define GDD_DIO_NAME          "GDDI"
#define APP_NAME              "APP"

#ifndef _TARGET_
  #define CLT_NAME            "CLT"   /* Used only during Simulation testing*/
#endif /*!_TARGET_*/

/*
 * TRACE_PRIMITVE is no more necessary when using NEW_FRAME.
 * Remove it sometime.
 */

#undef TRACE_PRIMITIVE
#define TRACE_PRIMITIVE(s)    /* s */


#endif /* !GPRS_H */
