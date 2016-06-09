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

#ifndef PSA_SNDCP_H
#define PSA_SNDCP_H



/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== PROTOTYPES =================================================*/

GLOBAL void psa_sn_dti_req( T_DTI_CONN_LINK_ID  link_id, T_DTI_ENTITY_ID peer, U8 dti_conn );
GLOBAL void psa_sn_dti_cnf ( T_SN_DTI_CNF *sn_dti_cnf );

#endif /* PSA_SNDCP_H */

#endif /* GPRS */
/*==== EOF =======================================================*/
