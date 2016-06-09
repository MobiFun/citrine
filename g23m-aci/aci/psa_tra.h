/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_TRA
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
|  Purpose :  Definitions for the protocol stack adapter TRA
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_TRA_H
#define PSA_TRA_H

EXTERN void psaTRA_Activate (void);
EXTERN void psaTRA_Deactivate (void);

#ifdef DTI
EXTERN void psaTRA_Dti_Req (T_DTI_CONN_LINK_ID link_id, UBYTE dti_conn, UBYTE peer);
#endif /* DTI */

#ifdef PSA_TRAS_C
GLOBAL BOOL TRA_is_activated = FALSE;
#else
EXTERN BOOL TRA_is_activated;
#endif

#endif /* PSA_TRA_H */

/*==== EOF =======================================================*/
