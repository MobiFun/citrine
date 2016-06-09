/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6302)
|  Modul   :  CNF_SIM
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
|  Purpose :  Dynamic Configuration for SIM Application
+----------------------------------------------------------------------------- 
*/ 

#ifndef CNF_SIM_H
#define CNF_SIM_H

/*==== CONSTANTS ==================================================*/
/*
 * CONFIGURATION PARAMETER
 *
 * Description :  The constants define the commands for dynamic
 *                configuration proposals.
 */
#define SIM_MODE            "MODE"
#define SIM_REMOVE          "REMOVE"
#define SIM_NORM_POLL       "STD_POLLING"
#define SIM_TEST_POLL       "TEST_POLLING"
#define SIM_EXTD_POLL       "EXTD_POLLING"

/*
 * dynamic configuration constants
 */
#define CFG_MODE            1
#define CFG_REMOVE          2
#define CFG_NORM_POLL       3
#define CFG_TEST_POLL       4
#define CFG_EXTD_POLL       5

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

#endif
