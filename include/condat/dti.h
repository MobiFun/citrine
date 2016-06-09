/* 
+----------------------------------------------------------------------------- 
|  Project :  DTILIB
|  Modul   :  DTI
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
|  Purpose :  Definitions for the Data Transmission Interface, DTI.
|             Includefile for every entity using DTILIB.
+----------------------------------------------------------------------------- 
*/ 
/*
 *  Version 1.6
 */

/**********************************************************************************/

/* NOTE:
 * there is a MACRO definition for "interface" in objbase.h
 * which says
 * #define interface struct
 * and thus would collide with the name "interface"
 * as a quick fix, all "interface" have been changed to "interfac"
 * in the dtilib project. This should not affect any external files
 */

/**********************************************************************************/

#ifndef DTI_H
#define DTI_H

/**********************************************************************************/

#if defined (DTI2)
/*
 *  Use SAP DTI2.DOC
 */
#include "p_dti2.h"
#else
/*
 *  Make this code compatible to SAP dti.doc.
 */
#include "p_dti.h"

#define T_DTI2_DATA_IND      T_DTI_DATA_IND
#define T_DTI2_DATA_REQ      T_DTI_DATA_REQ
#define T_DTI2_READY_IND     T_DTI_READY_IND
#define T_DTI2_GETDATA_REQ   T_DTI_GETDATA_REQ
#define T_DTI2_DATA_TEST_REQ T_DTI_DATA_TEST_REQ
#define T_DTI2_DATA_TEST_IND T_DTI_DATA_TEST_IND

/*
 * By use the SAP DTI.DOC and the DTI2.DOC there could be double
 * defines. So an undef is used.
 */

#ifdef DTI2_DATA_IND
  #undef DTI2_DATA_IND
#endif
#ifdef DTI2_DATA_REQ
  #undef DTI2_DATA_REQ
#endif
#ifdef DTI2_READY_IND
  #undef DTI2_READY_IND
#endif
#ifdef DTI2_GETDATA_REQ
  #undef DTI2_GETDATA_REQ
#endif
#ifdef DTI2_DATA_TEST_REQ
  #undef DTI2_DATA_TEST_REQ
#endif
#ifdef DTI2_DATA_TEST_IND
  #undef DTI2_DATA_TEST_IND
#endif

/*
 *  For calling the primitives over the macros.
 */
#define DTI2_DATA_IND      DTI_DATA_IND
#define DTI2_DATA_REQ      DTI_DATA_REQ
#define DTI2_READY_IND     DTI_READY_IND
#define DTI2_GETDATA_REQ   DTI_GETDATA_REQ
#define DTI2_DATA_TEST_REQ DTI_DATA_TEST_REQ
#define DTI2_DATA_TEST_IND DTI_DATA_TEST_IND

/*
 * Version related to DTI SAP 1.
 */

#define T_desc2 T_desc
#define T_desc_list2 T_desc_list
#define desc_list2 desc_list

#endif

/**********************************************************************************
 * Externally visible dtilib defines
 **********************************************************************************/
/*
 *  Reason parameter for the callback function
 */
#define DTI_NO_SIGNAL                 0 /* No signal shall be sended */
#define DTI_REASON_CONNECTION_OPENED  1 /* DTI connection opened or reseted */
#define DTI_REASON_CONNECTION_CLOSED  2 /* DTI connection closed */
#define DTI_REASON_DATA_RECEIVED      3 /* UL Data primitive received */
#define DTI_REASON_TX_BUFFER_FULL     4 /* DTI send-queue full */
#define DTI_REASON_TX_BUFFER_READY    5 /* DTI DTI send-queue no longer full */

/*
 * Connection states
 */
#define DTI_CLOSED      0   /* DTI connection is not established */
#define DTI_SETUP       1   /* Waiting for connection_opened signal from DTILIB */
#define DTI_IDLE        2   /* DTI connection is opened */

/*
 * dti_open - direction parameter
 *
 * to lower layer: send request primitives (_REQ / _CNF)
 * to higher layer: send indications       (_IND / _RES)
 * null link: do not send anything
 */
#define DTI_CHANNEL_TO_HIGHER_LAYER 0x00
#define DTI_CHANNEL_TO_LOWER_LAYER 0x01
#define DTI_NULL_LINK 0x02    /* null link (null device) */

/*
 * dti_open - link_options parameter
 *
 * the 0 is legacy from previous versions - upon reception of this value
 * the queue-length parameter has to be used to find out about desired
 * behaviour for the send queue
 *   #define FLOW_CNTRL_ENABLED    0     -- Flow control enabled
 */
#define DTI_FLOW_CNTRL_DISABLED 1 /* Flow control entirely disabled */
/*
 * Modes for handling of the send queue
 *
 * Queue is not used
 */
#define DTI_QUEUE_UNUSED 2
/*
 * Queue without size limitation
 */
#define DTI_QUEUE_UNBOUNDED 3
/*
 * Notify entity when queue is full,
 * afterwards discard oldest piece of data upon arrival of new data packets
 */
#define DTI_QUEUE_RM_FIFO 4
/*
 * Notify entity when queue is full,
 * afterwards discard newly arriving data packets
 */
#define DTI_QUEUE_RM_LIFO 5
/*
 * Notify entity when queue is full but keep all buffered data
 */
#define DTI_QUEUE_WATERMARK 6

/*
 * dti_init - entity_options (flags!)
 */
#define DTI_DEFAULT_OPTIONS 0x0
#define DTI_NO_TRACE    (1<<0)

/*
 * Some parameter which are used for initialization.
 */
#define  D_NO_DATA_BASE     NULL
#define  D_NO_INSTANCE      0xFF
#define  D_NO_INTERFACE     0xFF
#define  D_NO_CHANNEL_NR    0xFF

/**********************************************************************************
 * Obsolete dtilib defines
 *
 * NOTE: use of these defines is STRONGLY DISCOURAGED since their functionality
 *       is no longer needed and they will be removed some time in the future
 *
 **********************************************************************************/

/*
 * NOTE: this is _obsolete_ and maintained only for compatibility reasons!!
 * (see link_options parameter)
 */
#define DTI_QUEUE_DISABLED    0 /* Queue not used */
#define DTI_QUEUE_UNLIMITED 255 /* Queue without limitations in size */

/*
 * Home and neighbor entity.
 * NOTE: this is _obsolete_ and maintained only for compatibility reasons!!
 *       instead use the defines
 *       DTI_CHANNEL_TO_HIGHER_LAYER and
 *       DTI_CHANNEL_TO_LOWER_LAYER
 */
#define HOME        TRUE    /* Indicate the entity or instance which send dti_connect_req */
#define NEIGHBOR    FALSE   /* Indicate the entity or instance which send dti_connect_ind */

/*
 *  Type of the link
 */

#define ENTITY_LINK           0x01   /* normal link with an entity */
#define NULL_LINK              0x02   /* null link (null device) */
#define RIVIERA_BT_LINK   0x03    /* link to BT riviera entity */
#define RIVIERA_ATP_LINK  0x04    /* link to RNET riviera entity */

/*
 * Communication type
 * NOTE: this is _obsolete_ and maintained only for compatibility reasons!!
 * (see link_options parameter)
 */
#define FLOW_CNTRL_ENABLED    0     /* Flow control enabled */
#define FLOW_CNTRL_DISABLED   1     /* Flow control disabled */

/*
 * Connection states
 * NOTE: this is _obsolete_ and maintained only for compatibility reasons!!
 *       these states should be defined in the entities or the prefixec versiones
 *       above should be used
 */
#define CLOSED  0   /* DTI connection is not established */
#define SETUP   1   /* Waiting for connection_opened signal from DTILIB */
#define IDLE    2   /* DTI connection is opened */

/*
 * Entity states for sending
 * NOTE: this is _obsolete_ and maintained only for compatibility reasons!!
 *       these states should be defined in the entity itself
 *       to ensure proper prefixing
 */
#define TX_IDLE      3  /* The entity must not send data primitives (initial state) */
#define TX_READY     4  /* The entity can send data primitives */

/*
 * Entity states for receiving
 * NOTE: this is _obsolete_ and maintained only for compatibility reasons!!
 *       these states should be defined in the entity itself
 *       to ensure proper prefixing
 */
#define RX_IDLE     5   /* Data reception stopped (initial state) */
#define RX_READY    6   /* Ready to receive data */

#define ENTITY_NAME_LEN    0x6         

#define MAX_ATP_LINKS       0x4   /* max. number of links towards the RIV/ATP environment,
                                                       set by AAA*/
/*
 * Value constants for capability
 */
#define DTI_CPBLTY_NO                  0x0         /* used as initial value  = no capabilities */
#define DTI_CPBLTY_CMD                 0x1         /* AT cmd capability              */
#define DTI_CPBLTY_PKT                 0x2         /* packet capability              */
#define DTI_CPBLTY_SER                 0x4         /* serial capability              */

/**********************************************************************************
 * typedefs
 **********************************************************************************/

/*
 * Data Base for each entity.
 */

typedef struct
{
  T_HANDLE  handle;                   /* GPF task handle */ 
  U8 max_links;                       /* Number of links in the link list
                                        - started from the pointer first_link */
  U32 entity_options;                 /* user specified entity_options */
  void ((*sig_callback)               /* Callback function */
    (U8 instance,                     /* Instance of the entity */
     U8 interfac,                     /* Interface of the entity */
     U8 channel,                      /* Channel */
     U8 reason,                       /* Reason for the callback */
     T_DTI2_DATA_IND *dti_data_ind)); /* Data primitive ul */
#ifdef FF_TCP_IP
  char* own_name;                     /* name of the own entity, held by AAA */
  U16 entity_id_p;                    /* entity id within ATP, the same for all 
                                         entities and AAA */ 
#endif
  U32 first_link;                     /* Pointer to link table */
} DTI_DATA_BASE;

typedef DTI_DATA_BASE * DTI_HANDLE;   /* For calling the data base */


/*
 * Link Table, parameter for each DTI connection.
 * For detail information - please see the MSC DTILIB.DOC
 */
typedef struct
{
  U32             link_id;        /* Identity for the link communication */
  U8              direction;      /* Direction for the link communication */
  U32             version;        /* DTI Version */
  U8              instance;       /* Instance of the entity */
  U8              interfac;       /* Selected interface */
  U8              channel;        /* Channel number */
  U32             link_options;   /* type of flow control, queueing, .. */
  T_HANDLE        link_handle;    /* Handle for the communication channel */
  U8              queue_size;     /* DTI queue size */
  U8              queue_len;      /* length of the queue */
  T_DTI2_DATA_IND *dti_data_ind;  /* DTI data primitive */
  U8              connect_state;  /* State for connect */
  U8              rx_state;       /* State for receive */
  U8              tx_state;       /* State for send */
  U32             next_link;      /* Pointer to next DTI_LINK struct, last */
                                          /*  linkpointer = NULL */
  U8              link_type;      /* indicate Riviera, NULL or GPF link */
#ifdef FF_TCP_IP
  U16             ul_next_atp_data; /* data to be got from ATP */
  U8              atp_tx_state;   /* flow state of ATP */
  U8              dti_id;         /* ACI identity for the link */
  DTI_DATA_BASE*  entity_db;      /* pointer back to home database */
  UINT16          port_nb;        /* port number, if linked towards RIV */
#endif
                                      
} DTI_LINK;

/**********************************************************************************
 * Prototypes for DTILIB
 **********************************************************************************/

EXTERN DTI_HANDLE dti_init(
  U8 maximum_links,
  T_HANDLE handle,
  U32 entity_options,
  void (sig_callback(
    U8 instance,
    U8 interfac,
    U8 channel,
    U8 reason,
    T_DTI2_DATA_IND *dti_data_ind
    ) )
  );

EXTERN void dti_deinit( DTI_HANDLE hDTI);

EXTERN BOOL dti_open(
  DTI_HANDLE hDTI,
  U8 instance,
  U8 interfac,
  U8 channel,
  U8 queue_size,
  U8 direction,
  U32 link_options,
  U32 version,
  U8 *neighbor_entity,
  U32 link_id
  );

EXTERN BOOL dti_resolve_link_id(
  DTI_HANDLE hDTI,
  U8 instance,
  U8 interfac,
  U8 channel,
  U32 *link_id
  );

EXTERN void dti_close(
  DTI_HANDLE hDTI,
  U8 instance,
  U8 interfac,
  U8 channel,
  BOOL flush
  );

EXTERN void dti_start( DTI_HANDLE hDTI, U8 instance, U8 interfac, U8 channel);

EXTERN void dti_stop( DTI_HANDLE hDTI, U8 instance, U8 interfac, U8 channel);

EXTERN void dti_send_data(
  DTI_HANDLE hDTI,
  U8 instance,
  U8 interfac,
  U8 channel,
  T_DTI2_DATA_IND *dti_data_ind
  );

EXTERN void mfree_desc(DTI_HANDLE hDTI, T_desc_list2 * desc_list2);

EXTERN BOOL dti_tx_buffer_status(
  DTI_HANDLE hDTI,
  U8 instance,
  U8 interfac,
  U8 channel,
  U8 *queue_len
  );

/**********************************************************************************
 * Functions in ip_kerp.c
 **********************************************************************************/

#if defined (DTI2)
EXTERN GLOBAL  void dti_dti_connect_req (DTI_HANDLE hDTI,
                            T_DTI2_CONNECT_REQ *dti_connect_req);

EXTERN GLOBAL  void dti_dti_disconnect_req (DTI_HANDLE hDTI,
                            T_DTI2_DISCONNECT_REQ *dti_disconnect_req);

EXTERN GLOBAL  void dti_dti_disconnect_ind (DTI_HANDLE hDTI,
                            T_DTI2_DISCONNECT_IND *dti_disconnect_ind);

EXTERN GLOBAL  void dti_dti_connect_cnf (DTI_HANDLE hDTI,
                            T_DTI2_CONNECT_CNF *dti_connect_cnf);

EXTERN GLOBAL  void dti_dti_connect_res (DTI_HANDLE hDTI,
                            T_DTI2_CONNECT_RES *dti_connect_res);

EXTERN GLOBAL  void dti_dti_connect_ind (DTI_HANDLE hDTI,
                            T_DTI2_CONNECT_IND *dti_connect_ind);
#endif

EXTERN GLOBAL  void dti_dti_ready_ind (DTI_HANDLE hDTI,
                            T_DTI2_READY_IND *dti_ready_ind);

EXTERN GLOBAL  void dti_dti_data_req (DTI_HANDLE hDTI,
                            T_DTI2_DATA_REQ *dti_data_req);

EXTERN GLOBAL  void dti_dti_getdata_req (DTI_HANDLE hDTI,
                            T_DTI2_GETDATA_REQ *dti_getdata_req);

EXTERN GLOBAL  void dti_dti_data_ind (DTI_HANDLE hDTI,
                            T_DTI2_DATA_IND *dti_data_ind);

/**********************************************************************************
 * Test functions used in the windows operating system.
 **********************************************************************************/

#if defined (_SIMULATION_)
EXTERN  void dti_dti_data_test_ind (DTI_HANDLE hDTI,
                    T_DTI2_DATA_TEST_IND *dti_data_test_ind);

EXTERN  void dti_dti_data_test_req (DTI_HANDLE hDTI,
                    T_DTI2_DATA_TEST_REQ *dti_data_test_req);

EXTERN BOOL dti_make_new_desc (DTI_HANDLE hDTI,
                               T_desc2 **p_desc_new,
                               U16 malloc_len,
                               BOOL buff_init_0);

#endif /* _SIMULATION_ */
#endif /* DTI_H */
