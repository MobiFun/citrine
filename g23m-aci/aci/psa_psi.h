/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_PSI
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
|  Purpose :  Definitions for the protocol stack adapter Protocol Stack Interface (PSI )
+----------------------------------------------------------------------------- 
*/ 
#ifdef FF_PSI

#ifndef PSA_PSI_H
#define PSA_PSI_H

#include "dti_conn_mng.h"
#include "psa_uart.h"

#ifndef P_BAT_H
#ifdef FF_BAT
#include "p_bat.h"
#endif /* FF_BAT */
#endif

/*==== CONSTANTS ==================================================*/

#define PSI_DTI_FLOW_OFF     1
#define PSI_DTI_SB_BIT       2
#define PSI_DTI_SA_BIT       4
#define PSI_RING_RUNNING     8


/*==== TYPES ======================================================*/
typedef enum{
   PSI_CONNECT_DTI = 0,
   PSI_DISCONNECT_DTI
}T_DTI_MNG_PSI_MODE;


typedef struct
{
  T_ACI_DTR_BEHAVIOUR dtr_behaviour;
  BOOL                dtr_clearcall;
  BOOL                reconnect_to_aci;
}
T_PSI_SHRD_PRM;


#ifdef FF_BAT
typedef struct aci_bat_queue_entry_s
{
  struct aci_bat_queue_entry_s *next;
  T_BAT_cmd_send                cmd;  /* defined in p_bat.h */
  U8                            client_id;
} T_aci_bat_queue_entry;

typedef struct
{
  T_aci_bat_queue_entry *first;
  T_aci_bat_queue_entry *last;
} T_aci_bat_queue;

typedef struct
{
  T_BAT_ctrl_params  curCmd;    /* what is the currently running command */
  BOOL               opened;    /* there was a bat_open() at application */
  BOOL               atd_live;  /* TRUE while waiting for a final response to ATD */
  T_desc2           *send_data; /* due to L2P makes our life hard        */
} T_bat_aci_client_maintain;

#ifdef _SIMULATION_
typedef unsigned short T_BAT_client;
typedef struct
{
  T_BAT_client         client;
  T_BAT_cmd_response  *response;
} T_ACI_BAT_RES;
#endif
#endif /* FF_BAT */


typedef struct
{
  U32 devId;
  UBYTE dlci;
  UBYTE srcId;   /* = c_id */
  T_ACI_DTI_REC_STATE  RecState;
  T_ACI_DTI_LINE_STATE LineState;
#ifdef FF_BAT
  BOOL                 l2p_configured;    /* BAT and L2P */
  void                *rcv_data;          /* L2P receive buffer pointer, must be freed after BAT cmd processing */
  T_desc2             *send_data;         /* for control channel (0xFE), due to L2P makes our life hard         */
  T_bat_aci_client_maintain  *bat_client; /* BAT client maintenance */
  U8                   active_client;     /* which client runs the current command */
  U8                   max_clients;       /* maximum # of BAT clients */
  T_aci_bat_queue     *queue;             /* pendig BAT commands    */
  U8                  *search_str_ptr;    /* CPBF and similiar candidates */
#ifdef _SIMULATION_
  T_ACI_BAT_RES        res;
#endif
#endif /* FF_BAT */
  BOOL                 run_cmd;
  BOOL                 first_output;
  BYTE                 data_cntr;         /* DTI line flags (eg. SB-BIT) */
  T_ACI_UART_MUX_PARMS *MuxParms;
  BOOL                 large_type;        /* is it a large output ? */
  T_ACI_DTI_DATA_BUF   data_buffer;
  T_ACI_DEVICE_TYPE    device_type;
  S16                  index1;            /* remember for multi line response cmds the current index */
  S16                  index2;            /* remember for multi line response cmds the current index */

} T_ACI_DTI_PRC_PSI;


/*==== PROTOTYPES =================================================*/
EXTERN void psaPSI_ConnRej ( U32 devId);
EXTERN void psaPSI_ConnRes ( U32 devId);
EXTERN void psaPSI_CloseReq ( U32 devId);
EXTERN void psaPSI_DTICloseReq ( U32 devId, U32 link_id);
EXTERN void psaPSI_DTIOpenReq ( U32 devId, char* peer_name,
                                ULONG link_id, UBYTE dti_direction);
EXTERN void psaPSI_Dti_Req ( ULONG  link_id, UBYTE  peer, T_DTI_MNG_PSI_MODE con_mode);

EXTERN void psa_psi_DTI_data_req (T_desc2 *data, T_ACI_DTI_PRC_PSI *src_infos,
                               T_DTI_ENTITY_ID peer_id);
EXTERN void psa_psi_DTI_getdata( UBYTE src_id, T_DTI_ENTITY_ID peer_id);
EXTERN void psaPSI_DCDreq( UBYTE c_Id, UBYTE line_state );
EXTERN BOOL psaPSI_ESCAPEreq( UBYTE c_Id, UBYTE detection );
EXTERN void psaPSI_RINGreq( UBYTE c_Id, UBYTE line_state );
EXTERN void psaPSI_SetDTIReq( T_DTI_CONN_LINK_ID link_id, T_DTI_ENTITY_ID conn_peer_Id );
EXTERN BOOL atiPSI_dti_cb( UBYTE dti_id, T_DTI_CONN_STATE result_type );

#ifdef PSA_PSIP_C
GLOBAL T_PSI_SHRD_PRM psiShrdPrm;
#else
EXTERN T_PSI_SHRD_PRM psiShrdPrm;
#endif /* PSA_PSI_C */


/*==== EXPORT =====================================================*/

#endif /* PSA_PSI_H */
#endif /*FF_PSI*/
/*==== EOF =======================================================*/
