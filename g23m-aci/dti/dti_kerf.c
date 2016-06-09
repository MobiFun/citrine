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
|  Purpose :  Definitions for the Protocol Stack Library
|             DTI
+-----------------------------------------------------------------------------
*/

/*
 *  Version 1.6
 */

#ifndef DTI_KERF_C
#define DTI_KERF_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

/*==== CONST =======================================================*/

#include <string.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "dti.h"
#include "dti_int_def.h"
#include "dti_int_prot.h"

#ifdef FF_TCP_IP
#include "atp/atp_api.h"
#include "atp/atp_messages.h"
#include "aaa.h"
#include "dti_atp.h"
#include "gprs.h"
#include "dti_conn_mng.h"     /* for getting EXTRACT_DTI_ID */
#endif

/*==== LOCALS ================================================================*/

#ifdef _SIMULATION_
LOCAL void send_data_test_req_or_ind
           (
             DTI_HANDLE hDTI,
             DTI_LINK *link,
             T_DTI2_DATA_IND *dti_data_ind
           );
#endif /* _SIMULATION_ */


#ifdef FF_TCP_IP
static BOOL ATP_used_flag;
#endif

/*==== GLOBALS ===============================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : dti_init                 |
+--------------------------------------------------------------------+
 *
 *  Malloc and set default parameters and specified entity_options for the database
 *
 */

GLOBAL DTI_HANDLE dti_init(
  U8 maximum_links,
  T_HANDLE handle,
  U32 entity_options,
  void (sig_callback(
    U8 instance,
    U8 interfac,
    U8 channel,
    U8 reason,
    T_DTI2_DATA_IND *dti_data_ind)
    )
  )
{
  U16 i;
  DTI_HANDLE hDTI;
  DTI_LINK *link, *last_link;

  trace_function(handle, "dti_init()", entity_options);

#if defined ALLOCATE_DATABASE_BLOCK

   /*
    *  Allocate all needed memory for the DTI Database in one block.
    */
  MALLOC( hDTI, (U16)
    /* Database */
    (sizeof(temp_mem_1) - sizeof(DTI_LINK) +
    /* Every links */
    ((sizeof(temp_mem_2) - sizeof(temp_mem_1))* maximum_links)));

   /*
    *   Set defaults to the Database.
    */
  set_default_para_data_base(hDTI);

  /*
   *   Number of links in the Data Base (DTI_DATA_BASE).
   */
  hDTI->max_links = maximum_links;
  hDTI->handle    = handle;
  hDTI->entity_options   = entity_options;

  /*
   *   Cast the links and init them.
   */
  {
    U32 link_addr_offset = 0;
    for(i = 0; i < maximum_links; i++)
    {
      if(i == 0)
      {
        hDTI->first_link = ((U32) &((temp_mem_1*) hDTI)->tmp_link[i]);
        link =  (DTI_LINK*)hDTI->first_link;
        init_link_table(link);
        link_addr_offset = ((U32) &((temp_mem_2*) hDTI)->tmp_link[1]) - ((U32) link);
      }
      else
      {
        link = link + link_addr_offset;
        init_link_table(link);
      }
    }
  }
#else

  /*
   *  Allocate the memory over separate blocks.
   */
  MALLOC (hDTI, (U16)(sizeof(DTI_DATA_BASE) - 1));

  /*
   *   Set defaults
   */
  set_default_para_data_base(hDTI);

  /*
   *   Number of links in the Data Base (DTI_DATA_BASE).
   */
  hDTI->max_links = maximum_links;
  hDTI->handle    = handle;
  hDTI->entity_options   = entity_options;

  /*
   *   Allocate LINK_TABLE for maximum_links and set default parameters
   */
  if(maximum_links > 0)
  {
    MALLOC (link, (U16) sizeof(DTI_LINK));
    init_link_table(link);
    hDTI->first_link = (U32) link;
    for(i = 1; i < maximum_links; i++)
    {
      last_link = link;
      MALLOC (link, (U16) sizeof(DTI_LINK));
      init_link_table(link);
      last_link->next_link = (U32) link;
    }
  }
#endif

#ifdef FF_TCP_IP
    if (!ATP_used_flag)               /* initialise array of references to ATP links */
      {
        for ( i=0; i<MAX_ATP_LINKS; i++)
        {
          atp_links[i]= D_LINK;
        }
      }
#endif

  /*
   *  Set the callback function for the entity / instance
   */
  hDTI->sig_callback = sig_callback;
  return hDTI;
} /* dti_init() */
/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : dti_deinit               |
+--------------------------------------------------------------------+

 *
 *  Malloc and set default parameter for the Data Base
 *
 */

GLOBAL void dti_deinit( DTI_HANDLE hDTI)
{
  trace_function(hDTI->handle, "dti_deinit()", hDTI->entity_options);

  if(hDTI EQ D_NO_DATA_BASE)
    return;

#if defined ALLOCATE_DATABASE_BLOCK

  /*
   *  Free the datablock as a block.
   */

  MFREE (hDTI);

#else

  /*
   *  Free the links of the link table with DTI queue
   */

  free_dti_link_structure(hDTI);

  /*
   *   Free the DTI_DATA_BASE
   */

  MFREE (hDTI);

#endif
} /* dti_deinit() */

/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                      MODULE  : DTI_KERF           |
| STATE   : code                        ROUTINE : acquire_link       |
+--------------------------------------------------------------------+

 *
 *  This function acquires a link
 */
LOCAL DTI_RESULT acquire_link(DTI_HANDLE hDTI,
                              U32 link_id,
                              U8 direction,
                              DTI_LINK **link)
{
  trace_function(hDTI->handle,
                 "acquire_link()",
                 hDTI->entity_options);

  if((*link = get_pointer_link_table(hDTI, link_id, direction)) NEQ NULL)
    return DTI_S_FOUND;

  if((*link=get_pointer_free_link(hDTI)) NEQ NULL)
  {
    set_default_para_link_table (hDTI, *link, link_id, direction);
    return DTI_S_CREATED_NEW;
  }

  /*
   * No free link and so no connection possible.
   */
  trace_message_link_id(hDTI->handle,
    "DTI ERROR: No link free in dtilib",
    link_id,
    hDTI->entity_options);
  return DTI_E_FAIL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB          MODULE  : dti_kerf.c                     |
| STATE   : code            ROUTINE : send_connect_req_or_ind        |
+--------------------------------------------------------------------+

 * PURPOSE: Send a Connect primitive depending on the direction.
 */

LOCAL void send_connect_req_or_ind (DTI_HANDLE hDTI, DTI_LINK *link)
{
#define VSI_CALLER hDTI->handle,
  /*
   * Check if the link is upwards or downwards
   * and send the according connect primitive.
   */
  switch(link->direction)
  {
    case DTI_CHANNEL_TO_HIGHER_LAYER:
      {
        PALLOC (dti_connect_ind, DTI2_CONNECT_IND);
        dti_connect_ind->link_id = link->link_id;
        dti_connect_ind->version = link->version;
        if(hDTI->entity_options & DTI_NO_TRACE)
        {
          PSEND_NTRACE(link->link_handle, dti_connect_ind);
        }
        else
        {
          PSEND(link->link_handle, dti_connect_ind);
          trace_message_link_id(hDTI->handle,
                                "OUT: DTI2_CONNECT_IND",
                                link->link_id,
                                hDTI->entity_options);
        }
      }
      break;

    case DTI_CHANNEL_TO_LOWER_LAYER:
      {
        PALLOC (dti_connect_req, DTI2_CONNECT_REQ);
        dti_connect_req->link_id = link->link_id;
        dti_connect_req->version = link->version;
        if(hDTI->entity_options & DTI_NO_TRACE)
        {
          PSEND_NTRACE(link->link_handle, dti_connect_req);
        }
        else
        {
          PSEND(link->link_handle, dti_connect_req);
          trace_message_link_id(hDTI->handle,
                                "OUT: DTI2_CONNECT_REQ",
                                link->link_id,
                                hDTI->entity_options);
        }
      }
      break;

    default:
      /*
       * no known channel type open so do not send anything
       */
      break;
  }
#undef VSI_CALLER
} /* send_connect_req_or_ind() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB          MODULE  : dti_kerf.c                     |
| STATE   : code            ROUTINE : send_connect_res_or_cnf        |
+--------------------------------------------------------------------+

 * PURPOSE: Send a Connect Confirm primitive depending on the direction.
 */

LOCAL void send_connect_res_or_cnf (DTI_HANDLE hDTI, DTI_LINK *link)
{
#define VSI_CALLER hDTI->handle,
  /*
   * Check if the link is upwards or downwards
   * and send the according connect primitive.
   */
  switch(link->direction)
  {
    case DTI_CHANNEL_TO_HIGHER_LAYER:
      {
        PALLOC (dti_connect_cnf, DTI2_CONNECT_CNF);
        dti_connect_cnf->link_id = link->link_id;
        dti_connect_cnf->version = link->version;
        if(hDTI->entity_options & DTI_NO_TRACE)
        {
          PSEND_NTRACE(link->link_handle, dti_connect_cnf);
        }
        else
        {
          PSEND(link->link_handle, dti_connect_cnf);
          trace_message_link_id(hDTI->handle,
                                "OUT: DTI2_CONNECT_CNF",
                                link->link_id,
                                hDTI->entity_options);
        }
      }
      break;

    case DTI_CHANNEL_TO_LOWER_LAYER:
      {
        PALLOC (dti_connect_res, DTI2_CONNECT_RES);
        dti_connect_res->link_id = link->link_id;
        dti_connect_res->version = link->version;
        if(hDTI->entity_options & DTI_NO_TRACE)
        {
          PSEND_NTRACE(link->link_handle, dti_connect_res);
        }
        else
        {
          PSEND(link->link_handle, dti_connect_res);
          trace_message_link_id(hDTI->handle,
                                "OUT: DTI2_CONNECT_RES",
                                link->link_id,
                                hDTI->entity_options);
        }
      }
      break;

    default:
      /*
       * no known channel type open so do not send anything
       */
      break;
  }
#undef VSI_CALLER
} /* send_connect_res_or_cnf() */



/*
+-------------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                      |
| STATE   : code                  ROUTINE : send_open_ready_callback      |
+-------------------------------------------------------------------------+

 *
 *  Activate connection_opened and tx_buffer_ready callback funktions.
 */

LOCAL void send_open_ready_callback(DTI_HANDLE hDTI, DTI_LINK *link)
{
#ifdef FF_TCP_IP
  /*
   * trigger DTI Primitive transmission separatly, because it can not be done by ATP
   */
  if(link->link_type EQ RIVIERA_ATP_LINK)
  {
    PALLOC(dti_ready_ind, DTI2_READY_IND);
    dti_ready_ind->link_id = link->link_id;
#define VSI_CALLER 0,
    PSEND_NTRACE(link->link_handle, dti_ready_ind);
#undef VSI_CALLER
    /*
     * stop ATP reception Flow Control
     */
    TRACE_EVENT("initial: ATP_RX_FLOW_OFF");
    atp_set_signal(hDTI->entity_id_p,
                   link->port_nb,
                   ATP_RX_FLOW_OFF,
                   ATP_RX_FLOW_UNMASK);
  }
#endif /* FF_TCP_IP */
  /*
   * send indication for the open connection
   */
  hDTI->sig_callback(
    link->instance,
    link->interfac,
    link->channel,
    DTI_REASON_CONNECTION_OPENED,
    NULL);

  /*
   * Call the callback function with the signal tx_buffer_ready
   * if the queue is used and not full
   */
  if((link->direction EQ DTI_NULL_LINK) OR
     ((link->queue_len < link->queue_size) OR
      (link->link_options EQ DTI_QUEUE_UNBOUNDED)))
  {
    link->tx_state = DTI_TX_IDLE;
    hDTI->sig_callback(
      link->instance,
      link->interfac,
      link->channel,
      DTI_REASON_TX_BUFFER_READY,
      NULL
      );
  }
} /* send_open_ready_callback() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : dti_open                 |
+--------------------------------------------------------------------+

 *
 *  This function opens or resets a DTI connection.
 *
 */

GLOBAL BOOL dti_open (DTI_HANDLE hDTI,
                      U8 instance,
                      U8 interfac,
                      U8 channel,
                      U8 queue_size,
                      U8 direction,
                      U32 link_options,
                      U32 version,
                      U8 *neighbor_entity,
                      U32 link_id)
{
  DTI_LINK *link;
  BOOL send_req_or_ind  = FALSE;
  BOOL send_res_or_cnf  = FALSE;
  BOOL open_vsi_channel = FALSE;
  BOOL riviera_atp_channel  = FALSE;               /* came by DAA hack */
#ifdef BT_ADAPTER
  BOOL riviera_bt_channel  = FALSE;                /* came by dirty BT interface hack */
#endif  /* riviera_bt_channel */
  U8 signal             = DTI_NO_SIGNAL;

  trace_function(hDTI->handle,
                 "dti_open()",
                 hDTI->entity_options);

  trace_message_l_e(hDTI->handle,
    "open connection",
    link_id,
    (char*)neighbor_entity,
    hDTI->entity_options);

#ifdef BT_ADAPTER
 /*
   *  Check if open with a riviera entity.
   */
  if (strcmp ((CHAR*)neighbor_entity, BTI_NAME) EQ 0)
  {
    riviera_bt_channel = TRUE;
  }
#endif /* BT_ADAPTER */

#ifdef FF_TCP_IP
  TRACE_EVENT_P5 ("dti_open: QS:%d DIR:%d LOPT:%d NE:%s LID:%d V:4",  queue_size, direction, link_options, neighbor_entity, link_id);
  if (strcmp ((CHAR*)neighbor_entity, RIV_NAME) EQ 0)
  {
    riviera_atp_channel = TRUE;
    TRACE_EVENT("Riviera link to be opened!");
  }
#endif /* FF_TCP_IP */

 /*
   * Validate and correct passed parameters
   */
  if(validate_open_parameters(hDTI,
                              link_id,
                              &queue_size,
                              &direction,
                              &link_options,
                              version,
                              neighbor_entity) EQ DTI_E_FAIL)
  {
    return FALSE;
  }

  /*
   * Find a link with the given link_id and direction
   */
  switch (acquire_link (hDTI, link_id, direction, &link) )
  {
  case DTI_S_FOUND:
    /*
     * Found already existing link
     */
    switch (link->connect_state)
    {
    case DTI_CLOSING:
      link->connect_state = DTI_IDLE;
      /* fall through */

    case DTI_IDLE:
      /*
       *  Reset the link and set the parameter.
       */
          trace_message(hDTI->handle,
            "existing DTI connection is being reset!",
            hDTI->entity_options);

          set_open_para_link_table(hDTI,
                                   link,
                                   version,
                                   link_options,
                                   instance,
                                   interfac,
                                   channel,
                                   queue_size,
                                   DTI_IDLE);
      /*
       *  Send a confirm primitive if we use SAP DTI2.DOC. Then also set the parameter.
       */
      send_req_or_ind     = TRUE;
      signal              = DTI_REASON_CONNECTION_OPENED;
      if (link->direction NEQ DTI_NULL_LINK)
      {
       /*
        * Close the old channel.
        */
        vsi_c_close (hDTI->handle, link->link_handle);
      }

      /*
       *  Set the flag to open a new channel.
       */
          open_vsi_channel    = TRUE;

      break;

    case DTI_SETUP:
      /*
       *  Collision of DTI2_CONNECT_REQ and DTI2_CONNECT_IND. The parameters
       *  have been set in dti_open before. They stay in this state.
       */
      break;

    case DTI_CLOSED:
    /*
     *  Start with connecting
     */

      set_open_para_link_table
        (
        hDTI, link, version, link_options,
        instance, interfac, channel, queue_size, DTI_SETUP
        );

      link->connect_state     = DTI_SETUP;
      send_req_or_ind         = TRUE;
          open_vsi_channel        = TRUE;
      break;

    case DTI_CONNECTING:
      /*
       *  Got a connecting primitive, send a confirm primitive and set the parameter.
       */
      set_open_para_link_table
        (
        hDTI, link, version, link_options,
        instance, interfac, channel, queue_size, DTI_IDLE
        );

      send_res_or_cnf     = TRUE;
      signal              = DTI_REASON_CONNECTION_OPENED;
          open_vsi_channel    = TRUE;
      break;

    default:
      /*
       *  Wrong state
       */
          trace_message_link_id(hDTI->handle,
            "DTI ERROR: Wrong state of dtilib",
        link_id,
        hDTI->entity_options);
      break;
    }
    break;

  /*
   * Created new link
   */
  case DTI_S_CREATED_NEW:
    /*
     *  Open the requested channel and send a req/ind primitive.
     */
      open_vsi_channel    = TRUE;

    /*
     *  DTILIB can handle DTI SAP DTI.DOC and DTI2.DOC. By using DTI.DOC (the old
     *  SAP) the parameter are set but the connect primitives are not supported.
     *
     *  SAP DTI.DOC does also not support a queue. So the queue len is set to
     *  0 in the function set_link_parameter().
     */
    set_open_para_link_table
      (
      hDTI, link, version, link_options,
      instance, interfac, channel, queue_size, DTI_SETUP
      );
      send_req_or_ind     = TRUE;
    break;

  default:
    /*
     * No free link - no connection possible.
     */
      return FALSE;
  } /*lint !e788 enum constant not used */

/* in case of RIV link, get parameters from AAA */
#ifdef FF_TCP_IP
  if( riviera_atp_channel)
  {
    link->link_type     = RIVIERA_ATP_LINK;
    link->entity_db     = hDTI;                 /* remember the entity the link belongs to */
    link->dti_id          =  EXTRACT_DTI_ID(link_id);
/*    link->link_options = FLOW_CNTRL_DISABLED;  */ /* to avoid the full DTI state machine */

    /* get the parameters valid within RIV environment from AAA */
    if(!aaa_get_connection_data(  link->dti_id, &(link->port_nb), &(hDTI->entity_id_p), &(hDTI->own_name)))
    {
      TRACE_ERROR("aaa_get_connection_data() failed!");
    }
    else
    {
      TRACE_EVENT_P2("got connection_data: name: %s, link_id: %d", (char*) (hDTI->own_name), link->link_id);
    }

    if(hDTI->handle EQ PEI_ERROR)
    {
      TRACE_ERROR("error getting my own entity handle!");
    }
  }
#endif

/* mark links connection type */
  if(    !riviera_atp_channel
#ifdef BT_ADAPTER
     AND !riviera_bt_channel
#endif
    )  /*lint !e774  (Info -- Boolean within 'if' always evaluates to True), only used for FF_TCP_IP/BT */
  {
    link->link_type     = ENTITY_LINK;
  }

  /*
   * link to BlueTooth
   */
#ifdef BT_ADAPTER
  if(riviera_bt_channel)
  {
    link->link_type = RIVIERA_BT_LINK;
  }
#endif

  /*
   *  Open a channel to VSI. If it is not possible to open, return FALSE to the entity
   *  so that the entity can try again.
   */
  if(open_vsi_channel)
  {
    /*
     * If NULL device then disable flow control
     */
    if(direction EQ DTI_NULL_LINK)
    {
      trace_message(hDTI->handle,
                    "DTI connection is to be opened for NULL device",
                    hDTI->entity_options);
      /*
       * it is not exactly necessary to set this here,
       * but only a logical consequence
       */
      link->link_options  = DTI_FLOW_CNTRL_DISABLED;
      link->link_type     = NULL_LINK;
      link->connect_state = DTI_IDLE;
      /*
       *  Activate callback function with reason_connection_opened
       *  and, additionally, signal tx_buffer_ready if the queue is used.
       */
      send_open_ready_callback(hDTI, link);
      return TRUE;
    }

  if (link->link_type EQ ENTITY_LINK) /* check for connection within GPF */
  {
      open_comm_channel(hDTI->handle,
        &link->link_handle,
        (char *) neighbor_entity,
        hDTI->entity_options);
  }

#ifdef FF_TCP_IP
  else if (link->link_type EQ RIVIERA_ATP_LINK) /* check for connection towards Riv */
  {
      link->dti_data_ind  =  NULL;
      link->connect_state =  DTI_IDLE;     /* we don't use connect prims here, */
                                          /* so set it by hand */
      signal              = DTI_REASON_CONNECTION_OPENED;
      send_res_or_cnf     = FALSE;
      send_req_or_ind     = FALSE;

      open_comm_channel(  hDTI->handle,  /* open com handle to hDTI-entity */
                                      &link->link_handle,
                                      (char *) hDTI->own_name,
                                        hDTI->entity_options);

      /* the port has already been opened by AAA, so only remember link as leading to ATP,
        * for sending data or events 'backwards' to that entity
        */
      if(link->link_handle >= VSI_OK)
      {
        if(atp_links[link->port_nb] EQ D_LINK)
        {
            atp_links[link->port_nb] = link;
            ATP_used_flag = TRUE;
         }
        else
        {
          TRACE_ERROR("link reference could not be saved for DAA");
         }
      }
  }
#endif

    if(link->link_handle < VSI_OK)
    {
      trace_message_link_id(hDTI->handle,
        "DTI ERROR: open a comm channel not possible",
        link_id,
        hDTI->entity_options);

      set_default_para_link_table(hDTI, link, D_FREE_LINK_ID, D_DIRECTION);
      return FALSE;
    }

#ifdef BT_ADAPTER
  if(link->link_type EQ RIVIERA_BT_LINK)  /* connection links into RIV environment */
  {
      /*
      xxxx: handle new BT link
      */
  }
#endif
  }   /* if(open_channel) */

  if(send_res_or_cnf)                                 /* shouldn't happen for ATP links! */
  {
    /* Send DTI2_CONNECT_RES or DTI2_CONNECT_CNF
    */
    send_connect_res_or_cnf (hDTI, link);
  }
  else if(send_req_or_ind)                          /* is supposed to happen for ATP links??? */
  {
    /* Send DTI2_CONNECT_REQ or DTI_CONNECT_IND.
     */
    if(link->link_type EQ ENTITY_LINK)
    {
      send_connect_req_or_ind (hDTI, link);
    }
  }
  /*
   *  Activate the callback function to the entity.
   */
  if(signal EQ DTI_REASON_CONNECTION_OPENED)
  {
    if (!(hDTI->entity_options & DTI_NO_TRACE) )
      {
        trace_message_l_e(hDTI->handle,
        "DTI connection opened", /*lint !e605 Increase in pointer capability */
        link_id,
        (char*)neighbor_entity,
         hDTI->entity_options);
      }

    /*
     *  Activate callback function with reason_connection_opened
     *  and, additionally, signal tx_buffer_ready if the queue is used.
     */
    send_open_ready_callback (hDTI, link);
  }
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB          MODULE  : dti_kerf.c                     |
| STATE   : code            ROUTINE : send_disconnect_req_or_ind     |
+--------------------------------------------------------------------+

 * PURPOSE: Send a Disconnect primitive depending on the direction.
 */

LOCAL void send_disconnect_req_or_ind (DTI_HANDLE hDTI,
                                       DTI_LINK* link,
                                       U8 cause)
{
#define VSI_CALLER hDTI->handle,
  /*
   * Check if the link is upwards or downwards
   * and send the according disconnect primitive.
   */
  switch (link->direction)
  {
    case DTI_CHANNEL_TO_HIGHER_LAYER:
      {
        PALLOC (dti_disconnect_ind, DTI2_DISCONNECT_IND);
        dti_disconnect_ind->link_id = link->link_id;
        dti_disconnect_ind->cause   = cause;
        if(hDTI->entity_options & DTI_NO_TRACE)
        {
          PSEND_NTRACE(link->link_handle, dti_disconnect_ind);
        }
        else
        {
          PSEND(link->link_handle, dti_disconnect_ind);
          trace_message_link_id(hDTI->handle,
                                "OUT: DTI2_DISCONNECT_IND",
                                link->link_id,
                                hDTI->entity_options);
        }
      }
      break;

    case DTI_CHANNEL_TO_LOWER_LAYER:
      {
        PALLOC (dti_disconnect_req, DTI2_DISCONNECT_REQ);
        dti_disconnect_req->link_id = link->link_id;
        dti_disconnect_req->cause   = cause;
        if(hDTI->entity_options & DTI_NO_TRACE)
        {
          PSEND_NTRACE(link->link_handle, dti_disconnect_req);
        }
        else
        {
          PSEND(link->link_handle, dti_disconnect_req);
          trace_message_link_id(hDTI->handle,
                                "OUT: DTI2_DISCONNECT_REQ",
                                link->link_id,
                                hDTI->entity_options);
        }
      }
      break;

    default:
      /*
       * no known channel type open so do not send anything
       */
      break;
  }
#undef VSI_CALLER
} /* send_disconnect_req_or_ind() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB          MODULE  : dti_kerf.c                     |
| STATE   : code            ROUTINE : send_ready_req_or_ind          |
+--------------------------------------------------------------------+

 * PURPOSE: Send a Flow Control primitive depending on the direction.
 */

LOCAL void send_ready_req_or_ind (DTI_HANDLE hDTI, DTI_LINK *link)
{
#define VSI_CALLER hDTI->handle,

#ifdef FF_TCP_IP
  /*
   * do not send flow control primitives in case of riviera link
   * but send a Data primitive to trigger next data reception
   * this is needed because ATP may has sent data
   * where DTILIB was not initialized yet
   */
  if(link->link_type EQ RIVIERA_ATP_LINK)
  {
    PALLOC(dti_data_ind, DTI2_DATA_IND);
    dti_data_ind->link_id             = link->link_id;
    dti_data_ind->desc_list2.list_len = 0;
    dti_data_ind->desc_list2.first    = (U32)NULL;
    PSEND(link->link_handle, dti_data_ind);
    return;
  }
#endif /* FF_TCP_IP */
  /*
   * Check if the link is upwards or downwards
   * and send the according connect primitive.
   */
  switch (link->direction)
  {
    case DTI_CHANNEL_TO_HIGHER_LAYER:
#ifdef BT_ADAPTER
      if(link->link_type EQ RIVIERA_BT_LINK)
      {
        btidti_getdata_req(link_id);
      }
      else
#endif /* BT_ADAPTER */
      {
        if(link->link_type EQ ENTITY_LINK)
        {
          PALLOC (dti_ready_ind, DTI2_READY_IND);
          dti_ready_ind->link_id = link->link_id;
          if(hDTI->entity_options & DTI_NO_TRACE)
          {
            PSEND_NTRACE(link->link_handle, dti_ready_ind);
          }
          else
          {
            PSEND(link->link_handle, dti_ready_ind);
            trace_message_link_id(hDTI->handle,
                                  "OUT: DTI2_READY_IND",
                                  link->link_id,
                                  hDTI->entity_options);
          }
        }
      }
      break;

    case DTI_CHANNEL_TO_LOWER_LAYER:
#ifdef BT_ADAPTER
      if(link->link_type EQ RIVIERA_BT_LINK)
      {
        btidti_getdata_req(link_id);
      }
      else
#endif /* BT_ADAPTER */
      {
        if(link->link_type EQ ENTITY_LINK)
        {
          PALLOC (dti_getdata_req, DTI2_GETDATA_REQ);
          dti_getdata_req->link_id = link->link_id;
          if(hDTI->entity_options & DTI_NO_TRACE)
          {
            PSEND_NTRACE(link->link_handle, dti_getdata_req);
          }
          else
          {
            PSEND(link->link_handle, dti_getdata_req);
            trace_message_link_id(hDTI->handle,
                                  "OUT: DTI2_GETDATA_REQ",
                                  link->link_id,
                                  hDTI->entity_options);
          }
        }
      }
      break;

    default:
      /*
       * no known channel type open so do not send anything
       */
      break;
  }
#undef VSI_CALLER
} /* send_ready_req_or_ind() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : dti_close                |
+--------------------------------------------------------------------+

 *
 *  The function searchs a link in the databank link list and closes it.
 *  Then it calls the callback function with the signal DTI_REASON_CONNECTION_CLOSED.
 */

GLOBAL void dti_close (DTI_HANDLE hDTI,
                       U8 instance,
                       U8 interfac,
                       U8 channel,
                       BOOL flush)
{
  DTI_LINK *link;

  trace_function( hDTI->handle,
    "dti_close()",
    hDTI->entity_options);

  /*
   *  Find the link in the database
   */

  if((link = get_pointer_link_table_channel(hDTI,
                                            instance,
                                            interfac,
                                            channel)) NEQ NULL)
  {

      trace_message_link_id(hDTI->handle,
        "closing DTI connection", /*lint !e605 Increase in pointer capability */
        link->link_id,
        hDTI->entity_options);

    if (link->direction EQ DTI_NULL_LINK)
    {
      set_default_para_link_table(hDTI, link, D_FREE_LINK_ID, D_DIRECTION);
      return;
    }

    switch (link->connect_state)
    {
      /*
       *  The link is already closed
       */

      case DTI_CLOSED:
          trace_message_link_id(hDTI->handle,
            "Link is already closed for this entity",
          link->link_id,
          hDTI->entity_options);
        return;

        /*
         * In all other states the entity sends a disconnect primitive
         */
      default:
      /*
       * exit only after send queue has been emptied?
       */
        if(flush EQ TRUE)
        {
       /*
        * if it is not empty, wait for data flow primitives
        * from the peer entity
        */
          if(link->queue_len NEQ 0)
          {
            link->connect_state = DTI_CLOSING;
            return;
          }
          else
          {
          /*
           * call the callback function right now.
           */
            hDTI->sig_callback(
              link->instance,
              link->interfac,
              link->channel,
              DTI_REASON_CONNECTION_CLOSED,
              NULL
              );
          }
        }

        if (link->link_type NEQ RIVIERA_ATP_LINK)
          /* check for connection towards ATP. The NULL_LINK case is handled
           * above, already.
           */
        {
          send_disconnect_req_or_ind (hDTI, link, DTI_CAUSE_NORMAL_CLOSE);
        }
#ifdef FF_TCP_IP
        else  /* the links goes towards Riviera */
        {
          U8 i;
          atp_links[link->port_nb] = D_LINK;
          ATP_used_flag = FALSE;
          for (i=0; i<MAX_ATP_LINKS; i++)
          {
            if(atp_links[i] NEQ D_LINK)
            {
                ATP_used_flag = TRUE;
                TRACE_EVENT("there are still open ATP links!");
                break;
            }
          }
        }
#endif /* FF_TCP_IP */

        /*
         * close the communication channel
         */
        vsi_c_close (hDTI->handle, link->link_handle);
        /*
         *  Set the default parameter. The channel is closed in the neighbour entity.
         */
        set_default_para_link_table(hDTI,link, D_FREE_LINK_ID, D_DIRECTION);

        /*
         * Note: Flow control and data primitives are silently discarded.
         */

        break;
    }
  }
  else
  {
  /*
   * there is no link to be found in the table
   */
    trace_message_iic(hDTI->handle,
      "DTI link is alredy closed",
      instance,
      interfac,
      channel,
      hDTI->entity_options);
  }
}



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : dti_start                |
+--------------------------------------------------------------------+

 *
 * If the entity wants to receive data primitives, this function must be
 * called.
 */

GLOBAL void dti_start( DTI_HANDLE hDTI, U8 instance, U8 interfac, U8 channel)
{
  DTI_LINK *link;

  trace_function( hDTI->handle,
    "dti_start()",
    hDTI->entity_options);

  /*
   *  Find the link in the database.
   */
  link = get_pointer_link_table_channel(hDTI, instance, interfac, channel);
  if(link EQ NULL)
  {
    /*
     * Link id is not in the table.
     */
    trace_message_iic(hDTI->handle,
      "DTI ERROR: dti_start() - No link id in the database",
      instance,
      interfac,
      channel,
      hDTI->entity_options);
    return;
  }
  /*
   *  Is the entity connected ?
   */
  if(link->connect_state NEQ DTI_IDLE)
  {
    trace_message_link_id(hDTI->handle,
      "DTI ERROR: link is not connected",
      link->link_id,
      hDTI->entity_options);
    return;
  }
  /*
   *  Check if the flow control is not used but do nothing.
   */
  if(link->link_options EQ DTI_FLOW_CNTRL_DISABLED)
    return;
  /*
   *  Handle the states
   */
  switch(link->rx_state)
  {
    case DTI_RX_IDLE:
      /*
       *  Change the state to indicate ready to receive data.
       */
      link->rx_state = DTI_RX_READY;
      /*
       * No flow control primitive was sent. So send one now.
       * NOTE: The parameter link->direction gives information about the
       * direction for sending the data.
       */
      send_ready_req_or_ind (hDTI, link);
      break;

    case DTI_RX_STOPPED:
      /*
       * The entity has stopped the flow control.
       * The flow control was sent already.
       * So change state to DTI_RX_READY.
       */
      link->rx_state = DTI_RX_READY;
      break;

    case DTI_RX_READY:
      /*
       *  dti_start() was already called.
       */
      break;

    default:
      /*
       * Unexpected state - set ERROR.
       */
      trace_message_link_id(hDTI->handle,
        "DTI ERROR: wrong state",
      link->link_id,
      hDTI->entity_options);
      break;
  }
} /* dti_start() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : dti_stop                 |
+--------------------------------------------------------------------+

 *
 * This function is called if the entity wants to stop receiving of data
 * primitives.
 */

GLOBAL void dti_stop( DTI_HANDLE hDTI, U8 instance, U8 interfac, U8 channel)
{
  DTI_LINK *link;

  trace_function( hDTI->handle,
    "dti_stop()",
    hDTI->entity_options);

  /*
   *  Find the link in the databank.
   */
  link = get_pointer_link_table_channel(hDTI, instance, interfac, channel);
  /*
   * It is link id in the table ?
   */
  if(link EQ NULL)
  {
    trace_message_iic(hDTI->handle,
      "DTI ERROR: dti_stop() - No link id in the database",
      instance,
      interfac,
      channel,
      hDTI->entity_options);
    return;
  }
  /*
   *  Is the entity connected ?
   */
  if(link->connect_state NEQ DTI_IDLE)
  {
    trace_message_link_id(hDTI->handle,
      "DTI ERROR: dti_stop() - link is not connected",
      link->link_id,
      hDTI->entity_options);
    return;
  }
  /*
   *  Check if the flow control is not used - then do nothing
   */
  if(link->link_options EQ DTI_FLOW_CNTRL_DISABLED)
    return;
  /*
   *  Handle the states
   */
  switch(link->rx_state)
  {
    case DTI_RX_READY:
      /*
       *  The flow control was already sent therefor change to stop state.
       */
      link->rx_state = DTI_RX_STOPPED;
      break;

    case DTI_RX_STOPPED:
    case DTI_RX_IDLE:
      /*
       *  dti_stop() was already called.
       *  So there is no need to change state.
       */
      break;

    default:
      /*
       * Other state - ERROR
       */
      trace_message_link_id(hDTI->handle,
        "DTI ERROR: dti_stop() - wrong state",
        link->link_id,
        hDTI->entity_options);
      break;
  }
} /* dti_stop() */



#ifdef _SIMULATION_
/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB               MODULE  : dti_kerf.c                |
| STATE   : code                 ROUTINE : send_data_test_req_or_ind |
+--------------------------------------------------------------------+

 * PURPOSE: Get a DTI_DATA_IND and translate it to DTI_DATA_TEST_IND
 *          and send it.
 */

LOCAL void send_data_test_req_or_ind (DTI_HANDLE hDTI,
                                      DTI_LINK* link,
                                      T_DTI2_DATA_IND* dti_data_ind)
{
#define VSI_CALLER hDTI->handle,
  U16 len_buf_bits;
  U16 i;
  U16 len;
  U16 j;
  T_desc2 *p_desc;

  trace_function(hDTI->handle,
                 "send_data_test_req_or_ind()",
                 hDTI->entity_options);

  len_buf_bits = dti_data_ind->desc_list2.list_len * 8;

  /*
   *  Build the SDU primitive and send it
   */
  {
    PALLOC_SDU (dti_data_test_ind, DTI2_DATA_TEST_IND, len_buf_bits);
    memset (dti_data_test_ind, 0, sizeof (T_DTI2_DATA_TEST_IND));

    dti_data_test_ind->link_id    =  dti_data_ind->link_id;
    dti_data_test_ind->parameters =  dti_data_ind->parameters;

    dti_data_test_ind->sdu.l_buf = len_buf_bits;
    dti_data_test_ind->sdu.o_buf = 0;

    /*
     *  Copy the descs into sdu structure.
     */
    if(len_buf_bits > 0)
    {
      j = 0;
      p_desc = (T_desc2*)(dti_data_ind->desc_list2.first);
      while(p_desc NEQ NULL)
      {
        len = p_desc->len;
        for(i=0; i < len; i++)
        {
          dti_data_test_ind->sdu.buf[j] = p_desc->buffer[i];
          j++;
        }
        p_desc = (T_desc2*)(p_desc->next);
      }
    }

    /*
     * Check if the link is upwards or downwards
     * and send the according disconnect primitive.
     */
    switch (link->direction)
    {
      case DTI_CHANNEL_TO_HIGHER_LAYER:
        if(hDTI->entity_options & DTI_NO_TRACE)
        {
          PSEND_NTRACE(link->link_handle, dti_data_test_ind);
        }
        else
        {
          PSEND(link->link_handle, dti_data_test_ind);
          trace_message_l_dl(hDTI->handle,
                             "OUT: DTI2_DATA_TEST_IND",
                             dti_data_ind->link_id,
                             dti_data_ind->desc_list2.list_len,
                             hDTI->entity_options);
        }
        break;

      case DTI_CHANNEL_TO_LOWER_LAYER:
        if(hDTI->entity_options & DTI_NO_TRACE)
        {
          PPASS_NTRACE(dti_data_test_ind,
                       dti_data_test_req,
                       DTI2_DATA_TEST_REQ);
          PSEND_NTRACE(link->link_handle, dti_data_test_req);
        }
        else
        {
          PPASS(dti_data_test_ind, dti_data_test_req, DTI2_DATA_TEST_REQ);
          PSEND(link->link_handle, dti_data_test_req);
          trace_message_l_dl(hDTI->handle,
                             "OUT: DTI2_DATA_TEST_REQ",
                             dti_data_ind->link_id,
                             dti_data_ind->desc_list2.list_len,
                             hDTI->entity_options);
        }
        break;

      default:
        /*
         * no known channel type open so do not send anything
         */
        break;
    }
  }
  /*
   *   Free the dti_data_ind primitive and the descs in the linked list.
   */
  mfree_desc(hDTI, &dti_data_ind->desc_list2);
  PFREE (dti_data_ind);
#undef VSI_CALLER
} /* send_data_test_req_or_ind() */
#endif /* _SIMULATION_ */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : dti_kerf.c               |
| STATE   : code                  ROUTINE : send_data_req_or_ind     |
+--------------------------------------------------------------------+

 * PURPOSE: Send a Data primitive depending on the direction.
 */

LOCAL void send_data_req_or_ind (DTI_HANDLE hDTI,
                                 DTI_LINK* link,
                                 T_DTI2_DATA_IND* dti_data_ind)
{
#ifndef _SIMULATION_
  U32 link_id;
  U16 list_len;
#endif /* !_SIMULATION_ */
#ifdef BT_ADAPTER
  if(link->link_type EQ RIVIERA_BT_LINK)
  {
    btidti_data_req(dti_data_ind);
  }
  else
#endif /* BT_ADAPTER */
/*
 *  Check if the primitive is directed to a GPF or ATP entity, and send it.
 */
#ifdef FF_TCP_IP
  if(link->link_type EQ RIVIERA_ATP_LINK)
  {
    dti_send_data_to_atp(hDTI, link, dti_data_ind);
    return;
  }
#endif /* FF_TCP_IP */

#ifdef _SIMULATION_
  send_data_test_req_or_ind(hDTI, link, dti_data_ind);
#else  /* _SIMULATION_ */
#define VSI_CALLER hDTI->handle,
  /*
   * Check if the link is upwards or downwards
   * and send the according disconnect primitive.
   */
  switch (link->direction)
  {
    case DTI_CHANNEL_TO_HIGHER_LAYER:
#ifdef BT_ADAPTER
      if(link->link_type EQ RIVIERA_LINK)
      {
        btidti_data_req(dti_data_ind);
      }
      else
#endif /* BT_ADAPTER */
      {
        if(hDTI->entity_options & DTI_NO_TRACE)
        {
          PSEND_NTRACE(link->link_handle, dti_data_ind);
        }
        else
        {
          link_id  = dti_data_ind->link_id;
          list_len = dti_data_ind->desc_list2.list_len;
          PSEND(link->link_handle, dti_data_ind);
          trace_message_l_dl(hDTI->handle,
                             "OUT: DTI2_DATA_IND",
                             link_id,
                             list_len,
                             hDTI->entity_options);
        }
      }
      break;

    case DTI_CHANNEL_TO_LOWER_LAYER:
      if(hDTI->entity_options & DTI_NO_TRACE)
      {
        PPASS_NTRACE(dti_data_ind, dti_data_req, DTI2_DATA_REQ);
        PSEND_NTRACE(link->link_handle, dti_data_req);
      }
      else
      {
        PPASS(dti_data_ind, dti_data_req, DTI2_DATA_REQ);
        link_id  = dti_data_req->link_id;
        list_len = dti_data_req->desc_list2.list_len;
        PSEND(link->link_handle, dti_data_req);
        trace_message_l_dl(hDTI->handle,
                           "OUT: DTI2_DATA_REQ",
                           link_id,
                           list_len,
                           hDTI->entity_options);
      }
      break;

    default:
      /*
       * no known channel type open so do not send anything
       */
      break;
  }
#undef VSI_CALLER
#endif /* else _SIMULATION_ */
} /* send_data_req_or_ind() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : dti_send_data            |
+--------------------------------------------------------------------+

 *
 *  This function sends data. If the database direction is set to
 *  DTI_UPLINK then it sends a dti_data_req primitive otherwise a DTI_DATA_IND
 *  primitive.
 */

GLOBAL void dti_send_data (DTI_HANDLE hDTI,
                           U8 instance,
                           U8 interfac,
                           U8 channel,
                           T_DTI2_DATA_IND *dti_data_ind)
{
  DTI_LINK *link;
  T_DTI2_DATA_IND *hlp_data_ind;

  trace_function(hDTI->handle,
                 "dti_send_data()",
                 hDTI->entity_options);

  /*
   *  Find the link in the database.
   */
  link = get_pointer_link_table_channel(hDTI, instance, interfac, channel);
  if(link EQ NULL)
  {
    /*
     * link_id is not in the table.
     */
    trace_message_iic(hDTI->handle,
      "DTI ERROR: dti_send_data() - No link id in the database",
      instance,
      interfac,
      channel,
      hDTI->entity_options);
    mfree_desc(hDTI, &dti_data_ind->desc_list2);
    PFREE (dti_data_ind);
    return;
  }

  /*
   *  Is the entity connected ?
   */
  if(link->connect_state NEQ DTI_IDLE)
  {
    trace_message_link_id(hDTI->handle,
      "DTI ERROR: dti_send_data() - link is not connected",
      link->link_id,
      hDTI->entity_options);
    mfree_desc(hDTI, &dti_data_ind->desc_list2);
    PFREE (dti_data_ind);
    return;
  }

  /*
   * If link is a NULL device then just free the message
   */
  if (link->direction EQ DTI_NULL_LINK)
  {
    mfree_desc (hDTI, &dti_data_ind->desc_list2);
    PFREE (dti_data_ind);
    return;
  }

  /*
   * There is no flow control, so the primitive doesn't get into the queue
   * and has to get sent at once.
   */
  if(link->link_options EQ DTI_FLOW_CNTRL_DISABLED)
  {
    send_data_req_or_ind(hDTI, link, dti_data_ind);
    return;
  }

  switch (link->tx_state)
  {
    case DTI_TX_IDLE:
    case DTI_TX_BUFFER_FULL:
      /*
       * While waiting for a flow control primitive no sending is possible.
       * Put the primitive dti_data_ind in the queue.
       */
      put_dti_data_ind_in_queue_managed (hDTI, link, dti_data_ind);
      break;

    case DTI_TX_FLOW:
      /*
       * The flow control primitive is already received.
       * So just send Data primitive and change state.
       * Because of ATP links there might be still a prim in the queue
       * therefore we have to use the queue for this
       */
      put_dti_data_ind_in_queue(hDTI, link, dti_data_ind);
      /*
       *  Read the last packet from the queue and send it.
       */
      {
        hlp_data_ind =  get_dti_data_ind_from_queue(hDTI, link);
        hlp_data_ind->link_id = link->link_id;
        /*
         * The packet will be sent. Now change the state to DTI_TX_IDLE.
         */
        link->tx_state = DTI_TX_IDLE;
        /*
         *  Send the primitive depending on its direction.
         */
        send_data_req_or_ind(hDTI, link, hlp_data_ind);
      }
      break;

    default:
      /*
       * Unknown state..
       */
      trace_message_link_id(hDTI->handle,
        "DTI ERROR: dti_send_data() - wrong state",
        link->link_id,
        hDTI->entity_options);
      mfree_desc(hDTI, &dti_data_ind->desc_list2);
      PFREE (dti_data_ind);
      break;
  }
  /*
   * if queue is full now, send a warning callback
   */
  if((link->queue_len >= link->queue_size) AND
     (link->link_options NEQ DTI_QUEUE_UNBOUNDED))
  {
    link->tx_state = DTI_TX_BUFFER_FULL;
    hDTI->sig_callback(link->instance,
                       link->interfac,
                       link->channel,
                       DTI_REASON_TX_BUFFER_FULL,
                       NULL);
  }
} /* dti_send_data() */



/*
+--------------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERF                   |
| STATE   : code                      ROUTINE : flow_control_prim_received |
+--------------------------------------------------------------------------+

  PURPOSE : Process primitives DTI_GETDATA_REQ and DTI_READY_IND
            received from neighbour DTI
  */

GLOBAL void flow_control_prim_received (DTI_HANDLE hDTI,
                                        U32 link_id,
                                        U8 direction)
{
  DTI_LINK*         link;
  T_DTI2_DATA_IND*  dti_data_ind;

  trace_function(hDTI->handle,
                 "flow_control_prim_received()",
                 hDTI->entity_options);

  /*
   * get link pointer
   */
  link = get_pointer_link_table(hDTI, link_id, direction);
  if(link EQ NULL)
  {
    /*
     *  There is no link_id which requested to the link_id in the link_table.
     */
    trace_message_link_id(hDTI->handle,
      "DTI ERROR: fc_prim - No link in data base",
      link_id,
      hDTI->entity_options);
    return;
  }

  /*
   * check for valid link
   * the link is valid if it is in IDLE or CLOSING state
   * Flow Control primitives are unexpected if Flow Control is disabled
   */
  if(((link->connect_state NEQ DTI_IDLE) AND
      (link->connect_state NEQ DTI_CLOSING)) OR
     (link->link_options EQ DTI_FLOW_CNTRL_DISABLED))
  {
    TRACE_EVENT_P4("HORST=7, weil: connect_state %d, link_options %d, link_id %u, direction %d",
                   link->connect_state, link->link_options, link_id, direction);
    return;
  }
  /*
   * get next prim from queue
   */
  dti_data_ind = get_dti_data_ind_from_queue(hDTI, link);
  /*
   * Select the state.
   */
  switch (link->tx_state)
  {
    case DTI_TX_IDLE:
    case DTI_TX_FLOW:
      /*
       * A flow control primitive is received. Send a data packet
       * if there is any in the queue or change the state.
       * In case of an RIVIERA_ATP_LINK it is necessary to check the send queue
       * also in DTI_TX_FLOW state
       */
      if(dti_data_ind NEQ NULL)
      {
        /*
         * Select link_id and send data.
         */
        dti_data_ind->link_id = link->link_id;
        send_data_req_or_ind(hDTI, link, dti_data_ind);

        /*
         * Stay in this state.
         */
      }
      else
      {
        /*
         * Change the state because there is a flow control primitive
         * and no packet has been sent.
         */
        link->tx_state = DTI_TX_FLOW;
      }
      break;

    case DTI_TX_BUFFER_FULL:
      /*
       * The buffer had been full. Send packets from queue and signal ready
       */
      if(dti_data_ind NEQ NULL)
      {
        /*
         * Select link_id and send data.
         */
        dti_data_ind->link_id = link->link_id;
        send_data_req_or_ind(hDTI, link, dti_data_ind);
        /*
         * Change the state if the queue is ready
         * to get the next data from the entity
         */
        if(link->queue_len < link->queue_size)
        {
          link->tx_state = DTI_TX_IDLE;
        }
      }
      else
      {
        /*
         * Change the state because there is a flow control primitive
         * and no packet has been sent.
         */
        link->tx_state = DTI_TX_FLOW;
      }
      /*
       * Signal to the callback function that the buffer is ready.
       */
      if((link->connect_state NEQ DTI_CLOSING) AND
         (link->tx_state NEQ DTI_TX_BUFFER_FULL))
      {
        hDTI->sig_callback(link->instance,
                           link->interfac,
                           link->channel,
                           DTI_REASON_TX_BUFFER_READY,
                           NULL);
      }
      break;

    default:
      trace_message_link_id(hDTI->handle,
        "DTI ERROR: Wrong state for flow control primitive",
        link->link_id,
        hDTI->entity_options);
      /*
       * free whole prim, incl. descs
       */
      if(dti_data_ind NEQ NULL)
      {
        mfree_desc(hDTI, (T_desc_list2*) &(dti_data_ind->desc_list2));
        PFREE(dti_data_ind);
      }
      break;
  }

  /*
   * if the connection is to be closed and the send queue is empty
   * then close the connection now
   */
  if((link->queue_len EQ 0) AND
     (link->connect_state EQ DTI_CLOSING))
  {
    send_disconnect_req_or_ind(hDTI, link, DTI_CAUSE_NORMAL_CLOSE);
    close_link_with_signal(hDTI, link);
  }
} /* flow_control_prim_received() */



/*
+--------------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERF                   |
| STATE   : code                      ROUTINE : connect_init_prim_received |
+--------------------------------------------------------------------------+

PURPOSE : Process primitives DTI_CONNECT_REQ and DTI_CONNECT_IND
          received from neighbour DTI
*/

GLOBAL void connect_init_prim_received (DTI_HANDLE hDTI,
                                        U32 link_id,
                                        U32 version,
                                        U8 direction)
{
  DTI_LINK* link;
  BOOL      send_cnf  = FALSE;
  BOOL      send_disc = FALSE;
  U8        signal    = DTI_NO_SIGNAL;

  trace_function( hDTI->handle,
    "connect_init_prim_received()",
    hDTI->entity_options);

  /*
   *  Check version of peer dtilib
   */
  if(check_dti_version(hDTI, version) EQ FALSE)
  {
    trace_message_link_id(hDTI->handle,
      "DTI ERROR: init_prim - Wrong DTI version",
      link_id,
      hDTI->entity_options);
    /*
     * Inform peer dtilib that connection failed
     */
    link = get_pointer_link_table(hDTI, link_id, direction);
    if(link NEQ NULL)
    {
      send_disconnect_req_or_ind (hDTI, link, DTI_CAUSE_UNSUPPORTED_VERSION);
      close_link_with_signal(hDTI, link);
    }
    return;
  }

  switch (acquire_link (hDTI, link_id, direction, &link) )
  {
    case DTI_S_FOUND:
      /*
       *  Entry for link_id found, continue connecting procedure
       */
      break;

    case DTI_S_CREATED_NEW:
      /*
       *  There is no entry with the requested link_id in the link_table yet.
       *  Wait for call of dti_open().
       *  The remaining parameters are set in dti_open(). Then the response
       *  primitive will be sent.
       */
      link->connect_state   = DTI_CONNECTING;
      return;

    default:
      /*
       * No free link
       */
      return;
  }

  /*
   *  Start up connecting.
   */
  switch (link->connect_state)
  {
    case DTI_IDLE:
      /*
       *  Reset the link and send a response primitive, free the data packets,
       *  and call the callback funktion.
       */
      set_reset_req_para_link_table(hDTI, link);
      send_cnf = TRUE;
      signal   = DTI_REASON_CONNECTION_OPENED;
      trace_message_link_id(hDTI->handle,
        "DTI connection opened",
        link->link_id,
        hDTI->entity_options);
      break;

    case DTI_SETUP:
      /*
       *  Collision ! The neighbour entity has sent a dti_connect_ind
       *  primitive as well which means the neighbor enitiy is willing to connect as well.
       *  So we send a response and open the connection.
       */
      link->connect_state = DTI_IDLE;
      send_cnf            = TRUE;
      signal              = DTI_REASON_CONNECTION_OPENED;
      link->rx_state      = DTI_RX_IDLE;
      trace_message_link_id(hDTI->handle,
        "DTI connection opened",
        link->link_id,
        hDTI->entity_options);
      break;

    case DTI_CLOSING:
      /*
       * because of the connect request, internal buffers
       * are being reset. Thus, the send queue is empty now
       * and the connection can be closed down.
       * there has to be a confirmation for the connect
       * primitive anyway
       */
      send_disc = TRUE;
      send_cnf  = TRUE;
      break;

    case DTI_CLOSED:
      /*
       *  dti_open() is not called yet. The confirm primitive will
       *  be sent then and the parameter will be set.
       */
      link->connect_state = DTI_CONNECTING;
      break;

    default:
      trace_message_link_id(hDTI->handle,
        "DTI ERROR: init_prim - Wrong state dtilib",
        link_id,
        hDTI->entity_options);
      break;
  }

  /*
   *  Send the confirm primitive.
   */
  if (send_cnf)
  {
    send_connect_res_or_cnf (hDTI, link);
  }

  /*
   *  Send the disconnect primitive.
   */
  if (send_disc)
  {
    send_disconnect_req_or_ind (hDTI, link, DTI_CAUSE_NORMAL_CLOSE);
    close_link_with_signal(hDTI, link);
    /*
     * No more signals to be sent in this case..
     */
    return;
  }

  /*
   *  Activate callback function with reason_connection_opened
   *  and, additionally, signal tx_buffer_ready if the queue is used.
   */
  if(signal EQ DTI_REASON_CONNECTION_OPENED)
  {
    send_open_ready_callback (hDTI, link);
  }
} /* connect_init_prim_received() */



/*
+--------------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                       |
| STATE   : code                  ROUTINE : connect_confirm_prim_received  |
+--------------------------------------------------------------------------+

PURPOSE : Process primitives DTI_CONNECT_RES and DTI_CONNECT_CNF
          received from neighbour DTI
*/

GLOBAL void connect_confirm_prim_received (DTI_HANDLE hDTI,
                                           U32 link_id,
                                           U32 version,
                                           U8 direction)
{
  DTI_LINK* link;
  U8        signal = DTI_NO_SIGNAL;

  trace_function(hDTI->handle,
                 "connect_confirm_prim_received()",
                 hDTI->entity_options);

  /*
   *  Note: No need to check the version because this has already been done
   *        by the primitives dti_connect_req and dti_connect_ind.
   */
  if((link = get_pointer_link_table(hDTI, link_id, direction)) EQ NULL)
  {
    trace_message_link_id(hDTI->handle,
      "DTI ERROR: cnf_prim - No link in dtilib",
      link_id,
      hDTI->entity_options);
    return;
  }

  /*
   *  The link is in the list so check the state.
   */
  switch (link->connect_state)
  {
    case DTI_SETUP:
      /*
       *  The entity can now enter the final state. The entity connection
       *  is established.
       */
      link->connect_state = DTI_IDLE;
      signal              = DTI_REASON_CONNECTION_OPENED;
      link->rx_state      = DTI_RX_IDLE;
      trace_message_link_id(hDTI->handle,
        "DTI connection opened",
        link->link_id,
        hDTI->entity_options);
      break;

    case DTI_CLOSED:
    case DTI_IDLE:
      /*
       * We are already in the final state. So there is nothing to do here.
       */
      break;

    default:
      trace_message_link_id(hDTI->handle,
        "DTI ERROR: cnf_prim - Wrong state dtilib",
        link_id,
        hDTI->entity_options);
      break;
  }

  /*
   *  Activate callback function with reason_connection_opened
   *  and, additionally, signal tx_buffer_ready if the queue is used.
   */
  if(signal EQ DTI_REASON_CONNECTION_OPENED)
  {
    send_open_ready_callback (hDTI, link);
  }
} /* connect_confirm_prim_received() */



/*
+--------------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                       |
| STATE   : code                  ROUTINE : disconnect_prim_received       |
+--------------------------------------------------------------------------+

PURPOSE : Process primitives DTI_DISCONNECT_IND and DTI_DISCONNECT_REQ
          received from neighbour DTI
*/

GLOBAL void disconnect_prim_received (DTI_HANDLE hDTI,
                                      U32 link_id,
                                      U8 direction)
{

  DTI_LINK *link;

  trace_function(hDTI->handle,
                 "disconnect_prim_received()",
                 hDTI->entity_options);

  /*
   *  Is the link in the link list ?
   */
  if((link = get_pointer_link_table(
    hDTI,
    link_id,
    direction)
    ) NEQ NULL)
  {
    switch (link->connect_state)
    {
    /*
     *  Link is already closed.
     */
    case DTI_CLOSED:
      trace_message_link_id(hDTI->handle,
        "DTI link alredy closed",
        link->link_id,
        hDTI->entity_options);
      break;

    /*
     * Close the link.
     */
    default:
      close_link_with_signal(hDTI, link);

      break;
    }
  }

}



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERP             |
| STATE   : code                      ROUTINE : data_prim_received   |
+--------------------------------------------------------------------+

 *
 * Process primitives DTI_DATA_REQ and DTI_DATA_IND received from DTI peer
 */

GLOBAL void data_prim_received(DTI_HANDLE hDTI,
                               T_DTI2_DATA_IND *dti_data_ind,
                               U8 direction)
{
  DTI_LINK *link;
  U32 link_id;

  trace_function(hDTI->handle,
                 "data_prim_received()",
                 hDTI->entity_options);

  /*
   *  Check if old or new SAP is supported.
   */
  link_id = dti_data_ind->link_id;

  /*
   *  Is the link in the link list ?
   */
  link = get_pointer_link_table(hDTI, link_id, direction);
  if(link EQ NULL)
  {
    /*
     *  The link_id is not in the list of ids.
     */
    trace_message_link_id(hDTI->handle,
      "DTI ERROR: data_prim - No link in data base",
      link_id,
      hDTI->entity_options);
    /*
     * Just ignore the received data primitive.
     */
    mfree_desc(hDTI, &dti_data_ind->desc_list2);
    PFREE(dti_data_ind);
    return;
  }
  /*
   *  If there is no connection silently discard primitive.
   */
  if(link->connect_state NEQ DTI_IDLE)
  {
    trace_message_link_id(hDTI->handle,
      "DTI ERROR: data_prim_received() - link is not connected",
      link->link_id,
      hDTI->entity_options);
    /*
     * Just ignore the received data primitive.
     */
    mfree_desc(hDTI, &dti_data_ind->desc_list2);
    PFREE(dti_data_ind);
    return;
  }
  /*
   * Check if no flow control should be supported. Pass the primitive at once
   * to the entity over the callback function.
   */
  if(link->link_options EQ DTI_FLOW_CNTRL_DISABLED)
  {
    hDTI->sig_callback(link->instance,
                       link->interfac,
                       link->channel,
                       DTI_REASON_DATA_RECEIVED,
                       dti_data_ind);
    return;
  }

#ifdef FF_TCP_IP
  /*
   * Get primitive content in case it is a RIVIERA_ATP_LINK
   */
  if(link->link_type EQ RIVIERA_ATP_LINK)
  {
    if((link->rx_state EQ DTI_RX_READY) OR
       (link->rx_state EQ DTI_RX_STOPPED))
    {
      /*
       * DTI does not really know if there is data available
       * In case there is no ATP data the list_len value is set to 0
       */
      get_data_from_atp(hDTI, link, dti_data_ind);

      if(dti_data_ind->desc_list2.list_len EQ 0)
      {
        /*
         * there is no data in the ATP buffer any more
         * so release the primitive and start ATP flow control again
         */
        mfree_desc(hDTI, &dti_data_ind->desc_list2);
        PFREE (dti_data_ind);
        TRACE_EVENT("atp_set_signal: ATP_RX_FLOW_ON");
        atp_set_signal(hDTI->entity_id_p,
                       link->port_nb,
                       ATP_RX_FLOW_ON,
                       ATP_RX_FLOW_UNMASK);
        return;
      }
    }
    else
    {
      /*
       * the entity has stopped data flow
       * currently it is not allowed to receive data
       */
      mfree_desc(hDTI, &dti_data_ind->desc_list2);
      PFREE (dti_data_ind);
      return;
    }
  }
#endif /* FF_TCP_IP */

  /*
   * Handle the states.
   */
  switch(link->rx_state)
  {
    case DTI_RX_READY:
      /*
       *  Receive dti_data_ind, give a signal and send flow control primitive.
       */
      hDTI->sig_callback(link->instance,
                         link->interfac,
                         link->channel,
                         DTI_REASON_DATA_RECEIVED,
                         dti_data_ind);
      /*
       *  Check if the entity has stoped during the callback.
       *  If yes do not send a flow control.
       */
      if(link->rx_state EQ DTI_RX_READY)
      {
        send_ready_req_or_ind(hDTI, link);
      }
      else
      {
        link->rx_state = DTI_RX_IDLE;
      }
      break;

    case DTI_RX_STOPPED:
      /*
       *  The entity has stoped the communication but the flow control primitive to
       *  the neighbour entity was sent. The last dti_data_ind signal must be sended
       *  to the entity. DTILIB change to DTI_RX_IDLE.
       */
      link->rx_state = DTI_RX_IDLE;
      hDTI->sig_callback(link->instance,
                         link->interfac,
                         link->channel,
                         DTI_REASON_DATA_RECEIVED,
                         dti_data_ind);
      break;

    default:
      trace_message_link_id(hDTI->handle,
        "DTI ERROR: Wrong state for dti_data_ind primitive",
        link_id,
        hDTI->entity_options);
      mfree_desc(hDTI, &dti_data_ind->desc_list2);
      PFREE (dti_data_ind);
      break;
  }
} /* data_prim_received() */



#ifdef _SIMULATION_
/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                 MODULE  : DTI_KERP                |
| STATE   : code                   ROUTINE : data_test_prim_received |
+--------------------------------------------------------------------+

PURPOSE : Process primitives DTI_DATA_TEST_REQ and DTI_DATA_TEST_IND
          received from neighbour DTI
*/

GLOBAL void data_test_prim_received (DTI_HANDLE hDTI,
                                     T_DTI2_DATA_TEST_IND *dti_data_test_ind,
                                     U8 direction)
{
  USHORT len_buff, offset, i;
  T_desc2 *test_desc;

  trace_function(hDTI->handle,
                 "data_test_prim_received()",
                 hDTI->entity_options);

  /*
   *  Fill in dti_data_ind the structure.
   */
  {
    PALLOC (dti_data_ind, DTI2_DATA_IND);

    dti_data_ind->link_id    =  dti_data_test_ind->link_id;
    dti_data_ind->parameters =  dti_data_test_ind->parameters;

    len_buff = dti_data_test_ind->sdu.l_buf>>3;
    offset   = dti_data_test_ind->sdu.o_buf>>3;

    /*
     * Build a new desc and fill in the parameter.
     */
    dti_make_new_desc(hDTI, &test_desc, len_buff, TRUE);

    for(i=0; i < len_buff; i++)
      test_desc->buffer[i]  =  dti_data_test_ind->sdu.buf[i+offset];

    dti_data_ind->desc_list2.list_len   = len_buff;
    dti_data_ind->desc_list2.first      = (ULONG) test_desc;

    /*
     *  Handle the primitive.
     */
    PFREE (dti_data_test_ind);

    data_prim_received (hDTI, dti_data_ind, direction);
  }
}

#endif /* _SIMULATION_ */


/*
+-------------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                      |
| STATE   : code                  ROUTINE : vsi_c_psend_ntrace            |
+-------------------------------------------------------------------------+

 *
 *  vsi_c_psend without traces
 */

#ifdef MEMORY_SUPERVISION
GLOBAL SHORT vsi_c_psend_ntrace ( T_HANDLE Caller, T_HANDLE ComHandle,
  T_VOID_STRUCT *ptr, ULONG MsgLen, const char *file, int line )
#else  /* MEMORY_SUPERVISION */
  GLOBAL SHORT vsi_c_psend_ntrace ( T_HANDLE Caller, T_HANDLE ComHandle,
  T_VOID_STRUCT *ptr, ULONG MsgLen )
#endif /* MEMORY_SUPERVISION */
{
  T_QMSG QMsg;

  QMsg.Msg.Primitive.Prim = (T_VOID_STRUCT*)(D2P(ptr));
  QMsg.Msg.Primitive.PrimLen = MsgLen;
  QMsg.MsgType = MSG_PRIMITIVE;

#ifdef MEMORY_SUPERVISION
  return ( vsi_c_send ( Caller, ComHandle, &QMsg, file, line) );
#else
  return ( vsi_c_send ( Caller, ComHandle, &QMsg) );
#endif /* MEMORY_SUPERVISION */
}
