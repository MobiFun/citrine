/* 
+----------------------------------------------------------------------------- 
|  Project :  BAT L2P ()
|  Modul   :  BAT_L2P
+----------------------------------------------------------------------------- 
|  Copyright 2005 Texas Instruments Berlin, AG 
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
|  Purpose :  L2P external interface definition
+----------------------------------------------------------------------------- 
*/ 

#ifndef L2P_EXTERN_H
#define L2P_EXTERN_H

#include "l2p_types.h"

/**********************************************
**
** Function Prototypes
**
***********************************************/

T_L2P_STATUS L2P_Configure(U8 batid, void *mem,
                           T_L2P_SUPPORTED_PROTOCOLS protocolId,
                           U16 maxMTU,
                           L2P_Get_Tx_Buf_CB  Get_Tx_Buf_Cb,
                           L2P_Get_Rx_Buf_CB Get_Rx_Buf_Cb,
                           L2P_Get_Next_Seg_CB Get_Next_Seg_Cb,
                           L2P_Send_Frame_CB Send_Frame_Cb,
                           L2P_Msg_Rxd_CB Msg_Rxd_Cb);

T_L2P_STATUS L2P_Remove(U8 batid);

T_L2P_STATUS L2P_Send(U8 batId, U8 muxId, U32 dataTag, void *dataBuf, U16 dataLen, int *errVal);

T_L2P_STATUS L2P_Receive(U8 batId, void *dataBuf, void *seg_hdr_ptr, U16 totalsize, U16 segSize);

#endif

