/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_UART
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
|             Data Transmission Interface ( DTI )
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_UART_H
#define PSA_UART_H


/*==== CONSTANTS ==================================================*/

#define UART_DTI_FLOW_OFF     1
#define UART_DTI_SB_BIT       2
#define UART_DTI_SA_BIT       4
#define UART_RING_RUNNING     8

/*==== TYPES ======================================================*/

typedef enum
{
  NOT_INITIALIZED = -1,
  READY_REC,           /* ACI is ready to receive datas from DTI */
  RECEIVING,           /* ACI is currently receiving a command line */
  PROCESSING           /* command line is to be processed - no receiving from DTI */
}
T_ACI_DTI_REC_STATE;

typedef enum
{
  LINE_CONNECTING = 0,        /* only during initializing */
  LINE_CONNECTED,             /* enabling communication through DTI */
  LINE_DISCONNECTED,          /* during change uart to mux uart */
  LINE_TMP_CONNECTED,
  LINE_TMP_DISCONNECTED,       /* temporary disconnected */
  LINE_TMP_DISCONNECTING       /* temporary disconnecting */
}
T_ACI_DTI_LINE_STATE;

typedef enum
{
  DTR_BEHAVIOUR_NotPresent = -1,
  DTR_BEHAVIOUR_Ignore,
  DTR_BEHAVIOUR_CommandMode,
  DTR_BEHAVIOUR_ClearCall
}
T_ACI_DTR_BEHAVIOUR;

typedef enum {
  DEVICE_TYPE_UNKNOWN = -1,
  DEVICE_TYPE_URT,
  DEVICE_TYPE_MUX,
  DEVICE_TYPE_BT,
  DEVICE_TYPE_OTHER
} T_ACI_DEVICE_TYPE;

typedef struct
{
  T_ACI_DTR_BEHAVIOUR dtr_behaviour;
  BOOL                dtr_clearcall;
  BOOL                reconnect_to_aci;
  UBYTE               escape_seq;
}
T_UART_SHRD_PRM;


typedef struct
{
  UBYTE  mode;
  UBYTE  subset;
  UBYTE  port_speed;
  USHORT N1;
  UBYTE  T1;
  UBYTE  N2;
  UBYTE  T2;
  UBYTE  T3;  

} T_ACI_UART_MUX_PARMS;

#define MAX_DATA_BUFFER_LIFE_TIME   30000     /* 30 sec live time */
/* buffer for data buffer, if not possible to send data via DTI */
typedef struct
{
  T_desc2 *data_list;           /* list of data buffers       */
  USHORT  max_buffer_size;      /* size of the current buffer */
  T_TIME  create_time;
} T_ACI_DTI_DATA_BUF;

typedef struct
{
  UBYTE device;
  UBYTE dlci;
  UBYTE srcId;   /* = c_id */
  T_ACI_DTI_REC_STATE  RecState;
  T_ACI_DTI_LINE_STATE LineState;
  BOOL                 run_cmd;
  BOOL                 first_output;
  BYTE                 data_cntr;         /* DTI line flags (eg. SB-BIT) */
  T_ACI_UART_MUX_PARMS *MuxParms;
  BOOL                 large_type;        /* is it a large output ? */
  T_ACI_DTI_DATA_BUF   data_buffer;
  T_ACI_DEVICE_TYPE    device_type;
} T_ACI_DTI_PRC;


/*==== PROTOTYPES =================================================*/

EXTERN void psaUART_DCDreq   ( UBYTE c_Id, UBYTE line_state );
EXTERN void psaUART_ESCAPEreq ( UBYTE c_Id, UBYTE line_state );
EXTERN void psaUART_RINGreq   ( UBYTE c_Id, UBYTE line_state );

#ifdef DTI
EXTERN BOOL       atiUART_dti_cb   ( UBYTE dti_id, T_DTI_CONN_STATE result_type );
EXTERN void psaDTI_getdata   ( UBYTE srcId, T_DTI_ENTITY_ID peer_id );
EXTERN void psaUART_SetDTIReq( T_DTI_CONN_LINK_ID link_id, T_DTI_ENTITY_ID conn_peer_Id );
EXTERN void psaDTI_data_req (
                               T_desc2 *data, 
                               T_ACI_DTI_PRC *src_infos, 
                               T_DTI_ENTITY_ID peer_id
                               );
#endif /* DTI */
EXTERN void psaUART_StartMux ( UBYTE device,
                                     UBYTE mode,
                                     UBYTE frame_type,
                                     USHORT N1,
                                     UBYTE T1,
                                     UBYTE N2,
                                     UBYTE T2,
                                     UBYTE T3 );
EXTERN void psaUART_MuxRes  ( UBYTE device,
                                    UBYTE dlci,
                                    USHORT N1 );

EXTERN void psaUART_SetParameters( UBYTE device,
                                         T_comPar *comPar );


#ifdef PSA_UARTP_C

GLOBAL T_UART_SHRD_PRM uartShrdPrm;
#else

EXTERN T_UART_SHRD_PRM uartShrdPrm;
#endif /* PSA_UARTP_C */

/*==== EXPORT =====================================================*/

#endif /* PSA_UART_H */

/*==== EOF =======================================================*/
