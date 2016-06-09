/* 
+------------------------------------------------------------------------------
|  File:       tst_mux.h
+------------------------------------------------------------------------------
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
|  Purpose :  Prototypes for TST multiplexer access 
+----------------------------------------------------------------------------- 
*/ 

#ifndef TST_MUX_H
#define TST_MUX_H

/*==== INCLUDES =============================================================*/


/*==== CONSTS ===============================================================*/

/* channels for ETM */
#define MAX_TST_CHANNEL   2

#define MAX_TST_MUX_CMD_LEN  512

/*==== TYPES ================================================================*/

typedef struct
{
  U8 channel_id;
  void * rcv_data_ptr;
  int rcv_data_size;
  void (*rcv_callback)(void *,int);
  T_HANDLE drv_handle;
  void * send_data;
} T_TST_MUX_CHANNEL;

/*==== PROTOTYPES ===========================================================*/

int tst_mux_send     ( U8 id, void * buffer, int size );
int tst_mux_register ( U8 id, void (*callback)(void * buffer, int size));
int tst_mux_init     ( void );
void tst_mux_callback( U8 id, void * buffer, int size );


#endif /* !TST_MUX_H.H */
