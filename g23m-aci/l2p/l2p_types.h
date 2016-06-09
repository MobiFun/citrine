/* 
+----------------------------------------------------------------------------- 
|  Project :  BAT L2P ()
|  Modul   :  L2P Types
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
|  Purpose :  L2P type definitions definitions
+----------------------------------------------------------------------------- 
*/ 

#ifndef L2P_TYPES_H
#define L2P_TYPES_H

#include "typedefs.h"

/**********************************************************
**
** Enumerated Type definitions
**
***********************************************************/
typedef enum {
   L2P_STAT_SUCCESS = 0,
   L2P_STAT_UNKNOWN_ERROR = -1,
   L2P_STAT_MSG_SEND_FAIL = -2,
   L2P_STAT_INVALID_PARAMETER = -3,
   L2P_STAT_NO_SUCH_BAT_ID = -4,
   L2P_STAT_NO_SUCH_MUX_ID = -5,
   L2P_STAT_UNABLE_TO_GET_BUFFER = -6,
   L2P_STAT_BAT_ID_ALREADY_EXISTS = -7,
   L2P_STAT_UNEXPECTED_FRAME_RXD = -8,
   L2P_STAT_CRC_FAIL = -9
} T_L2P_STATUS;

typedef enum {
    L2P_FT_UNKNOWN_FRAME_TYPE=0,
    L2P_FT_START_FRAME,
    L2P_FT_CONTINUATION_FRAME,
    L2P_FT_END_FRAME,
    L2P_FT_COMPLETE_FRAME
} T_L2P_FRAME_TYPE;

typedef enum {
    L2P_SP_PSI,
    L2P_SP_MAX_NUM
}T_L2P_SUPPORTED_PROTOCOLS;


/**********************************************************
**
** Callback function type definitions
**
***********************************************************/
typedef void *(* L2P_Get_Rx_Buf_CB)(U8 batId);  /* Data Size not needed - The user will always return a maximum size buffer */
typedef void *(* L2P_Get_Tx_Buf_CB)(U8 batId, U16 dataSize, void **seg_hdr_ptr,  U16 *totalSize, U16 *segSize);
typedef void *(* L2P_Get_Next_Seg_CB)(U8 batId, void *last_seg_hdr, void **seg_hdr_ptr,  U16 *segSize);
typedef int   (* L2P_Send_Frame_CB)(U8 batId);
typedef void  (* L2P_Msg_Rxd_CB)(U8 batId , U8 muxId, U32 dataTag, void *dataPtr, U16 dataSize);


/**********************************************************
**
** L2P Structure definitions
**
***********************************************************/

typedef struct {
    L2P_Get_Tx_Buf_CB  Get_TxBuf_Cb;
    L2P_Get_Rx_Buf_CB  Get_RxBuf_Cb;
    L2P_Get_Next_Seg_CB Get_Next_Seg_Cb;
    L2P_Send_Frame_CB  Send_Frame_Cb;
    L2P_Msg_Rxd_CB Msg_Rxd_Cb;
} T_L2P_CALLBACK_BLOCK;

/*
** NDH : 20/06/05 : Modified the order of the fields in the L2P Control Block Structure
**                  to ensure that the compiler Byte Alignment is efficient so that the
**                  compiler does not add unnecessary padding bytes.
**
**                  In the following Comments the Bytes Alignment is show with the notation bN
**                  where N is 0 - 3.
**                  8bit entries can be on any byte
**                  16bit entries should be on b0 or b2 only
**                  32bit entries should be on b0 only
*/
typedef struct l2p_control_block_tag {
    U8 batId;                               /* b0 : Bat Instance Id */
    U8 frameId;                             /* b1 : Frame Id of last transmitted packet*/
    U16 maxMTU;                             /* b2 : Maximum Transport Unit size */
    T_L2P_SUPPORTED_PROTOCOLS protocolId;   /* b0 : protocol used for link */
    T_L2P_CALLBACK_BLOCK callbacks;         /* b0 : Call back functions from calling entity */
    char *rxBuf;                            /* b0 : Pointer to the current receive buffer */
    U8 muxId;                               /* b0 : Client Id of the sender/receipient */
    U8 cmdTagReceived;                      /* b1 : Flag to indicate whether the Cmd Tag has been fully received */
    U16 dataSize;                           /* b2 : Amount of data expected in total (length of the Cmd Data excl the Cmd Tag) */
    U32 receivedCmdTag;                     /* b0 : The value of the Cmd Tag received in the latest message*/
    U16  bytesToBeRead;                     /* b0 : byte read counter must be persistent over several calls to L2P_Receive */
    U8   pad_byte1;                         /* b2 : Padding byte - not used */
    U8   pad_byre2;                         /* b3 : Padding byte - not used */
    char *dataToBeRead;                     /* b0 : data read pointer must be persistent over several calls to L2P_Receive */
    struct  l2p_control_block_tag *next;    /* b0 : Pointer to the next block in the L2P Linked-List */
} T_L2P_CONTROL_BLOCK;

typedef struct {
    U8 frame_type;
    U8 frame_id;
    U8 protocol_id;
    U8 mux_id;
    UINT16 dataLen;         /* length of the Command Data */
    U16 frameLen;           /* length of the frame, including L2 Frame Control Bytes */
} T_L2P_FRAME_CONTROL_BYTES;

/**********************************************************
**
** L2P Size definitions
**
***********************************************************/
#define L2P_CONTROL_BLOCK_HDR_SIZE sizeof(T_L2P_CONTROL_BLOCK)
#define L2P_FRAME_CONTROL_SIZE sizeof(T_L2P_FRAME_CONTROL_BYTES)

#endif

