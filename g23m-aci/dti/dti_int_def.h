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
|  Purpose :  Definitions for the internal Data Transmission Interface
|             Library, DTI.
+-----------------------------------------------------------------------------
*/
/*
 *  Version 1.6
 */

#ifndef DTI_INT_DEF_H
#define DTI_INT_DEF_H

/*==== Constants ==================================================================*/

/* definition disappeared from vsi.h */
#ifdef MEMORY_SUPERVISION
#define FILE_AND_LINE ,__FILE__,__LINE__
#else  /* MEMORY_SUPERVISION */
#define FILE_AND_LINE
#endif /* else MEMORY_SUPERVISION */

/*
 *  Please do NOT use D_FREE_LINK_ID as your link_id.
 *  This constant indicates a free link in the link structure and in the dti_data_ind queue.
 */

#define  D_FREE_LINK_ID    65000    /* Indicate a free link ID */
#define  NO_DATA_PACKET    NULL     /* Indicate that is no data packet in the link
                                       structure */

/*
 * Default parameter
 */

#define  D_HANDLE            0
#define  D_CHANNEL          0
#define  D_LINK             ((U32)NULL)
#define  D_LINK_HANDLE      0
#define  D_QUEUE_SIZE       0
#define  D_DIRECTION        DTI_CHANNEL_TO_LOWER_LAYER
#define  D_VERSION          DTI_VERSION_10
#define  D_INSTANCE         0
#define  D_INTERFACE        0
#define  D_LINK_OPTIONS     DTI_QUEUE_UNUSED
#define  D_CONNECT_STATE    DTI_CLOSED
#define  D_MAX_LINKS        0
#define  D_ATP_SW_ENTITY_ID 0       /* target ID within Riviera */

/*
 *  RX states
 */

#define DTI_RX_IDLE     0        /* No Flow Control primitive sent (initial state) */
#define DTI_RX_READY    1        /* Flow Control primitive sent expect data primitive */
#define DTI_RX_STOPPED  2        /* Flow Control primitive sent
                                    but data flow was stopped by the entity */
/*
 *  TX states
 */

#define DTI_TX_IDLE        1 /* Waiting for Flow Control primitive (initial state) */
#define DTI_TX_FLOW        2 /* Flow Control primitive received */
#define DTI_TX_BUFFER_FULL 3 /* TX buffer buffer full signal has been sent */

/*
 *  Connect states
 */
#if 0
/* already defined in dti.h */
#define DTI_CLOSED      0   /* DTI connection is not established */
#define DTI_SETUP       1   /* Waiting for Connect Confirm primitive */
#define DTI_IDLE        2   /* DTI connection is opened */
#endif /* 0 */
#define DTI_CONNECTING  3   /* Connect primitive received,
                               but no appropriate dti_open() was called */
#define DTI_CLOSING     4   /* DTI connection is to be closed
                               when the send queue is empty */

/*
 * Bitoffset for encoding/decoding
 */
#define ENCODE_OFFSET   0

/*============ TYPES ======================================================*/

/*
 * Return values
 */
typedef enum {
  DTI_E_FAIL = 0,
  DTI_S_OK = 1,
  DTI_S_FOUND,
  DTI_S_CREATED_NEW
} DTI_RESULT;

/*============ MACROS =====================================================*/

/*
+------------------------------------------------------------------------------
| Purpose:    Additions to general definitions in vsi.h
|             for execution without traces (no matter which flags are set).
|
|             This is needed for MTST - please be aware of possible code
|             changes in vsi.h, however...
| $Identity:$
+------------------------------------------------------------------------------
*/

#define  PSEND_NTRACE(E,D)     vsi_c_psend_ntrace ( VSI_CALLER E, \
                                 (T_VOID_STRUCT*)D, PSIZE(D) FILE_LINE_MACRO );



#define PPASS_NTRACE(D0,D,T)   T_##T  *D    = (T_##T*)D0; \
                                 D_OPC(D)     = (T);

/*============ DATA STRUCTURES ============================================*/

#ifdef FF_TCP_IP
/*
 * Link reference table, references links in ANY entity data base
 *  towards ATP
 */
#ifdef DTI_ATP_C
  DTI_LINK* atp_links[MAX_ATP_LINKS];
#else  /* DTI_ATP_C */
  extern DTI_LINK* atp_links[] ;
#endif /* DTI_ATP_C */

#endif /* FF_TCP_IP */




#endif /* DTI_INT_DEF_H */
