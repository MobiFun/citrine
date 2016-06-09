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
|  Purpose :  Definitions for the protocol stack adapter SNDCP.
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef PSA_UPM_H
#define PSA_UPM_H



/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== PROTOTYPES =================================================*/

GLOBAL void psa_upm_count_req ( UBYTE c_id, BOOL reset );
GLOBAL void psa_upm_count_cnf ( T_SN_COUNT_CNF *upm_count_cnf );

#endif /* PSA_UPM_H */

#endif /* GPRS */
/*==== EOF =======================================================*/
