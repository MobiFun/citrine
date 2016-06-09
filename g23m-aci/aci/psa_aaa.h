/*
+-----------------------------------------------------------------------------
|  Project :  ACI
|  Modul   :  psa_aaa
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
|  Purpose :  This modul ...
+-----------------------------------------------------------------------------
*/
#ifndef PSA_AAA_H
#define PSA_AAA_H

/*
 *  belongs to psa_aaas.c
 */
EXTERN void psaAAA_open_port_cnf	    (UBYTE	 port_number,
                                             UBYTE   sub_no,
 	 					                                 UBYTE	 dti_id);

EXTERN void psaAAA_close_port_cnf	  (UBYTE	 port_number,
                                             UBYTE   sub_no);

EXTERN void psaAAA_cmd_cnf	          (UBYTE	 port_number,
                                             UBYTE   sub_no,
                                             USHORT	 result_len,
                                             UBYTE	*cmd_result);

EXTERN void psaAAA_pres_ind	        (UBYTE	 port_number,
                                             UBYTE   sub_no,
 	                                           USHORT	 result_len,
 	                                           UBYTE	*cmd_result);

EXTERN void psaAAA_ures_ind	        (UBYTE	 port_number,
                                             UBYTE   sub_no,
 	                                           USHORT	 result_len,
 	                                           UBYTE	*cmd_result);

EXTERN void psaAAA_dti_ind	          (ULONG            link_id, 
                                             T_DTI_ENTITY_ID  peer_ent_id);


EXTERN void psaAAA_disconnect_ind	  (ULONG  link_id);



#endif


