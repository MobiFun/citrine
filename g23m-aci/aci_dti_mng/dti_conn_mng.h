/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  dti_conn_mng.h
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
|  Purpose :  Header file for DTI Connection Manager
+-----------------------------------------------------------------------------
*/

#ifndef DTI_CONN_H
#define DTI_CONN_H

#define DTI_DTI_ID_NOTPRESENT NOT_PRESENT_8BIT

#define DTI_TUPLE_NO_NOTPRESENT NOT_PRESENT_8BIT

#define DTI_LINK_ID_NOTPRESENT NOT_PRESENT_32BIT


#define NUM_OF_PEERS 2

#define MAX_DTI_CONN_TUPLES 5 /* max. number of tuples in an end-to-end connection */

#define MAX_DTI_CONN_ENTIES_PER_CHANNEL (MAX_DTI_CONN_TUPLES+1)

#define MAX_DTI_CONN_CHANNELS 32 /* can be max 32 because of tst_id, max number of end-to-end connections */

#define MAX_DTI_CONN_LINK_IDS (MAX_DTI_CONN_TUPLES*MAX_DTI_CONN_CHANNELS) /* max number of link ids */



#define EXTRACT_TUPLE_NO( link_id )\
  (UBYTE)(link_id & 0x000000FF)

#define EXTRACT_DTI_ID( link_id )\
  (UBYTE)((link_id & 0x0000FF00) >> 8)

#define DTI_CONNECT    0x00
#define DTI_DISCONNECT 0x01

typedef ULONG T_DTI_CONN_LINK_ID;


typedef enum
{
  DTI_ENTITY_INVALID = -1,
#ifdef GPRS
  DTI_ENTITY_SNDCP,
#endif
  DTI_ENTITY_ACI,
  DTI_ENTITY_UART,
  DTI_ENTITY_PPPS,
  DTI_ENTITY_PPPC,
  DTI_ENTITY_L2R,
  DTI_ENTITY_T30,
  DTI_ENTITY_IP,
  DTI_ENTITY_TRA,
  DTI_ENTITY_UDP,
#ifdef FF_GPF_TCPIP
  DTI_ENTITY_TCPIP,
#endif

  DTI_ENTITY_WAP,
  DTI_ENTITY_MTST,
  DTI_ENTITY_BLUETOOTH,
  DTI_ENTITY_SIM,
  DTI_ENTITY_NULL,
  DTI_ENTITY_AAA,
#ifdef GPRS
  DTI_ENTITY_PKTIO,
#endif
  DTI_ENTITY_PSI,
  DTI_ENTITY_MAX
} T_DTI_ENTITY_ID;

typedef struct
{
  CHAR            *name;
  T_DTI_ENTITY_ID  id;
} T_DTI_ENTITY_NAME;
/*
 * the XXX_NAME are defined in custom.h and gprs.h
 */
 /*lint -e528 (Warning -- Symbol not referenced*/
static const T_DTI_ENTITY_NAME dti_entity_name[] =
{
#ifdef GPRS
  {SNDCP_NAME,  DTI_ENTITY_SNDCP     },
#endif
  {ACI_NAME,    DTI_ENTITY_ACI       },
  {UART_NAME,   DTI_ENTITY_UART      },
  {PPP_NAME,    DTI_ENTITY_PPPS      },
  {PPP_NAME,    DTI_ENTITY_PPPC      },
  {L2R_NAME,    DTI_ENTITY_L2R       },
  {T30_NAME,    DTI_ENTITY_T30       },
  {IP_NAME,     DTI_ENTITY_IP        },
  {TRA_NAME,    DTI_ENTITY_TRA       },
  {UDP_NAME,    DTI_ENTITY_UDP       },
  #ifdef FF_GPF_TCPIP
  {TCPIP_NAME,  DTI_ENTITY_TCPIP     },
  #endif   


  {WAP_NAME,    DTI_ENTITY_WAP       },
  {MTST_NAME,   DTI_ENTITY_MTST      },
  {BTI_NAME,    DTI_ENTITY_BLUETOOTH },
  {SIM_NAME,    DTI_ENTITY_SIM       },
  {NULL_NAME,   DTI_ENTITY_NULL      },
  {RIV_NAME,    DTI_ENTITY_AAA       }, /* entity AAA is an AT cmd gate only, but for DTI it is "RIV" */
#ifdef GPRS
  {PKTIO_NAME,  DTI_ENTITY_PKTIO     },
#endif
  {PSI_NAME,  DTI_ENTITY_PSI},
  {"",          DTI_ENTITY_MAX       }
};


typedef enum
{
  SPLIT = 0,
  APPEND
} T_DTI_CONN_MODE;


typedef enum {
  DTI_CONN_STATE_UNKNOWN = -1,
  DTI_CONN_STATE_CONNECTING,
  DTI_CONN_STATE_CONNECTED,
  DTI_CONN_STATE_DISCONNECTING,
  DTI_CONN_STATE_DISCONNECTED,
  DTI_CONN_STATE_ERROR
} T_DTI_CONN_STATE;

typedef enum
{
  DTI_OK,
  DTI_ERROR
} T_DTI_CONN_RESULT;

typedef struct {
  T_DTI_CONN_STATE      state;
  T_DTI_ENTITY_ID       ent_id;
} T_DTI_CONN_PEER;


typedef struct {
  UBYTE                 tuple_no;
  T_DTI_CONN_STATE      state;
  T_DTI_CONN_PEER       peers[NUM_OF_PEERS];
} T_DTI_CONN_TUPLE;


typedef BOOL  T_DTI_CONN_CB (UBYTE              dti_id,
                             T_DTI_CONN_STATE   result_type );

typedef BOOL  T_DTI_CONN_MNG_ENT_CB( T_DTI_CONN_LINK_ID  link_id, 
                                     T_DTI_ENTITY_ID     entity_id,
                                     T_DTI_ENTITY_ID     peer_entity_id,
                                     UBYTE               dti_conn);


typedef struct {
  BOOL                  erase_channel;
  UBYTE                 dti_id;
  UBYTE                 num_of_conns;
  T_DTI_CONN_CB*        conn_cb;
  T_DTI_CONN_TUPLE      tuple_list[MAX_DTI_CONN_TUPLES];
  T_DTI_CONN_STATE      state;
} T_DTI_CONN_CHANNEL;


typedef struct {
  T_DTI_CONN_MNG_ENT_CB* mng_ent_cb;
  T_DTI_CONN_CB*         conn_cb;
  UBYTE                  num_entities;
  T_DTI_ENTITY_ID        entity_list_buf[MAX_DTI_CONN_ENTIES_PER_CHANNEL];
} T_DTI_CONN_PARAMS;



EXTERN T_DTI_CONN_LINK_ID dti_conn_compose_link_id ( UBYTE dummy, 
                                                     UBYTE assoc, 
                                                     UBYTE dti_id, 
                                                     UBYTE tuple_no);

EXTERN void dti_conn_init( T_DTI_CONN_MNG_ENT_CB* mng_ent_cb );

EXTERN UBYTE dti_conn_new( UBYTE dti_id );

EXTERN void dti_conn_erase_entry( UBYTE dti_id );


EXTERN BOOL dti_conn_est_dpath( UBYTE               dti_id,
                                T_DTI_ENTITY_ID*    entity_list,
                                UBYTE               num_entities,
                                T_DTI_CONN_MODE     mode,
                                T_DTI_CONN_CB*      cb );

EXTERN BOOL dti_conn_close_dpath( UBYTE dti_id );


EXTERN BOOL dti_conn_is_dti_channel_connected( T_DTI_ENTITY_ID ent_id, UBYTE dti_id );

EXTERN BOOL dti_conn_is_dti_channel_disconnected( UBYTE dti_id );


EXTERN void dti_conn_entity_connected( T_DTI_CONN_LINK_ID link_id, 
                                       T_DTI_ENTITY_ID    ent_id,
                                       T_DTI_CONN_RESULT  result );

EXTERN void dti_conn_entity_disconnected( T_DTI_CONN_LINK_ID link_id, 
                                          T_DTI_ENTITY_ID    ent_id );

EXTERN void dti_conn_close_all_connections ( );

#endif /* DTI_CONN_H */
