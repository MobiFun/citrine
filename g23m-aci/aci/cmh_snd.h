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
|  Purpose :  Definitions for the protocol stack adapter 
|             SNDCP.
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef CMH_SND_H
#define CMH_SND_H


typedef struct
{
  UBYTE         srcId;
} T_SNDCP_SHRD_PRM;


EXTERN BOOL NULL_UPN_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type);
EXTERN BOOL PKTIO_SNDCP_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type);

#ifdef CO_UDP_IP
EXTERN BOOL IP_SNDCP_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type);
#endif

#ifdef FF_GPF_TCPIP
EXTERN BOOL TCPIP_SNDCP_connect_dti_cb(UBYTE dti_id,
                                       T_DTI_CONN_STATE result_type) ;
#endif  /* FF_GPF_TCPIP */
/*==== EXPORT =====================================================*/
#ifdef CMH_SNDS_C

GLOBAL T_SNDCP_SHRD_PRM sndcpShrdPrm;
#else

EXTERN T_SNDCP_SHRD_PRM sndcpShrdPrm;
#endif /* CMH_SNDS_C */


#endif /* CMH_SND_H */
                 
#endif  /* GPRS */
/*==== EOF =======================================================*/
