/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_CMD
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
|  Purpose :  Definitions for the AT Command Interpreter
+----------------------------------------------------------------------------- 
*/ 

#ifndef ACI_IO_H
#define ACI_IO_H


typedef enum {
    IO_DCD_OFF = 0,
    IO_DCD_ON
} T_IO_DCD_STAT;

typedef enum {
    IO_RING_OFF = 0,
    IO_RING_ON
} T_IO_RING_STAT;

typedef enum{
    ESC_DETECTION_OFF=0,
    ESC_DETECTION_ON
} T_IO_ESC_STAT;

typedef struct {
  T_IO_RING_STAT  ring_stat;
  UBYTE           b_cap_1;
  UBYTE           b_cap_2;
} T_IO_RING_PARAMS;


EXTERN void io_sendBreak    (
                              USHORT         brk_len
                            );

EXTERN void io_sendMessage  (
                              UBYTE             srcId,
                              char              *a,
                              T_ATI_OUTPUT_TYPE output_type
                            );

EXTERN void io_sendMessageEx (
                               UBYTE             srcId,
                               char              *a,
                               T_ATI_OUTPUT_TYPE output_type
                             );
#ifndef LOCOSTO_LITE_2_54_COMPILER_ERROR
EXTERN void io_sendIndication  (
                                 UBYTE             srcId,
                                 char              *a,
                                 T_ATI_OUTPUT_TYPE output_type
                               );
#endif


EXTERN void io_sendConfirm  (
                              UBYTE             srcId,
                              char              *a,
                              T_ATI_OUTPUT_TYPE output_type
                            );

EXTERN BOOL check_should_buffer_ind( T_ATI_SRC_PARAMS *src_params );
EXTERN void send_buffered_indication( T_ATI_SRC_PARAMS *src_params );

EXTERN void io_setRngInd (T_IO_RING_STAT state, T_ACI_CRING_SERV_TYP bt1, T_ACI_CRING_SERV_TYP bt2);

EXTERN void io_setDCD (T_ACI_CMD_SRC src_id, T_IO_DCD_STAT state);

EXTERN void io_setCommandState(T_ATI_SRC_PARAMS *src_params, T_ATI_LINE_STATE_TYPE line_state_type);

#endif
