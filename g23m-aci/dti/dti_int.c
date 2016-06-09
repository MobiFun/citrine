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

  MODULE  : DTI

  PURPOSE : Internal functions of the Protocol Stack Library DTI
*/

#ifndef DTI_INT_C
#define DTI_INT_C
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
#endif /* FF_TCP_IP */

/*
+---------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                  |
| STATE   : code                  ROUTINE : set_default_para_data_base|
+---------------------------------------------------------------------+

 *
 *  Set the default parameter for the DTI data base
 *
 */

GLOBAL void set_default_para_data_base(DTI_HANDLE hDTI)
{
  hDTI->first_link = D_LINK;
  hDTI->handle     = D_LINK_HANDLE;
  hDTI->max_links  = D_MAX_LINKS;

#ifdef FF_TCP_IP
  hDTI->entity_id_p = D_ATP_SW_ENTITY_ID;
#endif /* FF_TCP_IP */
}

/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : free_dti_packets_queue   |
+--------------------------------------------------------------------+

 *
 *  Free dti packets in the queue
 *
 */

LOCAL void free_dti_data_ind_packets_queue(DTI_HANDLE hDTI, T_DTI2_DATA_IND * p_data_ind)
{
  T_DTI2_DATA_IND *p_ind;

  BOOL go = TRUE;

  trace_function( hDTI->handle,
    "free_dti_data_ind_packets_queue()",
    hDTI->entity_options);

   /*
    * Are there any packets in the queue ?
    */

  if (p_data_ind EQ NO_DATA_PACKET )
    return;

  p_ind = p_data_ind;

  /*
   *  Check if DTILIB is supported
   */

  /*
   *  Free the packets
   */

  do {
    T_DTI2_DATA_IND *p_last_ind;
    p_last_ind = p_ind;

    if(p_ind->link_id NEQ 0)
      p_ind = (T_DTI2_DATA_IND *) p_ind->link_id;
    else
      go = FALSE;

    /*
     *  Free the descs at first
     */
    mfree_desc(hDTI, &p_last_ind->desc_list2);

    /*
     *  Free the primitive
     */
    PFREE (p_last_ind);

  } while( go );
} /* free_dti_data_ind_packets_queue() */



/*
+-----------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                    |
| STATE   : code                  ROUTINE : set_default_para_link_table |
+-----------------------------------------------------------------------+
 *
 *  Set the default parameter for DTI_LINK
 *
 */

GLOBAL void set_default_para_link_table (DTI_HANDLE hDTI,
                                         DTI_LINK *link,
                                         U32 link_id,
                                         U8 direction)
{
  /*
   * Free primitives and descs if there are any in the queue
   */
  free_dti_data_ind_packets_queue(hDTI, link->dti_data_ind);
  link->dti_data_ind = NULL;
  /*
   * Set the other parameter to default
   */
  link->link_id       = link_id;
  link->direction     = direction;
  link->version       = D_VERSION;
  link->link_options  = D_LINK_OPTIONS;
  link->instance      = D_INSTANCE;
  link->interfac      = D_INTERFACE;
  link->channel       = D_CHANNEL;
  link->link_handle   = D_LINK_HANDLE;
  link->queue_size    = D_QUEUE_SIZE;
  link->queue_len     = 0;
  link->connect_state = D_CONNECT_STATE;
  link->rx_state      = DTI_RX_IDLE;
  link->tx_state      = DTI_TX_IDLE;
} /* set_default_para_link_table() */



/*
+-----------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_int										                    |
| STATE   : code                  ROUTINE : init_link_table             									|
+-----------------------------------------------------------------------+
 *
 *  Set the init parameter for DTI_LINK
 *
 */

GLOBAL void init_link_table (DTI_LINK *link)
{
  link->link_id          = D_FREE_LINK_ID;
  link->direction        = D_DIRECTION;
  link->version          = D_VERSION;
  link->link_options     = D_LINK_OPTIONS;
  link->instance         = D_INSTANCE;
  link->interfac         = D_INTERFACE;
  link->link_handle      = D_LINK_HANDLE;
  link->queue_size       = D_QUEUE_SIZE;
  link->queue_len        = 0;
  link->channel          = D_CHANNEL;
  link->dti_data_ind     = NULL;
  link->connect_state    = D_CONNECT_STATE;
  link->rx_state         = DTI_RX_IDLE;
  link->tx_state         = DTI_TX_IDLE;
  link->next_link        = D_LINK;
#ifdef FF_TCP_IP
  link->ul_next_atp_data = 0;
  link->atp_tx_state     = ATP_TX_FLOW_ON;
  link->dti_id           = 0;
  link->entity_db        = (U32) NULL;
  link->port_nb          = 0;
#endif /* FF_TCP_IP */
} /* init_link_table() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : free_dti_link_structure  |
+--------------------------------------------------------------------+

 *
 *  Free DTI link structures, DTI_LINK.
 *
 */

GLOBAL void free_dti_link_structure(DTI_HANDLE hDTI)
{
  DTI_LINK *link_last, *link;
  BOOL go = TRUE;

  trace_function(hDTI->handle,
                 "free_dti_link_structure()", /*lint !e605 Increase in pointer capability */
                 hDTI->entity_options);

  /*
   *    Check if there are any packets to free
   */
  if(hDTI->first_link EQ D_LINK)
    return;

  link = (DTI_LINK *) hDTI->first_link;

  do {
    link_last = link;

    if(link->next_link NEQ D_LINK)
      link = (DTI_LINK *) link->next_link;
    else
      go = FALSE;

     /*
      *  Free the dti_queue
      */
    free_dti_data_ind_packets_queue(hDTI, link_last->dti_data_ind);

    /*
     *  Free a link structure
     */
    MFREE (link_last);

  } while( go );

} /* free_dti_link_structure() */

/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : trace_message            |
+--------------------------------------------------------------------+

 *
 *  Create a string
 *
 */

GLOBAL void trace_message (T_HANDLE handle,
                          char *str,
                          U32 entity_options)
{
  if(! (entity_options & DTI_NO_TRACE) )
  {
    TRACE_EVENT(str);
  }
} /*lint !e715 handle not referenced trace_message() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : trace_message_link_id    |
+--------------------------------------------------------------------+

 *
 *  Create a string and a number
 *
 */

GLOBAL void trace_message_link_id (T_HANDLE handle,
                                  char *str,
                                  U32 link_id,
                                  U32 entity_options)
{
  if(! (entity_options & DTI_NO_TRACE) )
  {
    TRACE_EVENT_P2("%s - link_id=%d", str, link_id);
  }
} /*lint !e715 handle not referenced trace_message_link_id() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : trace_message_l_dl       |
+--------------------------------------------------------------------+

 *
 *  Create a string with link_id and data length
 *
 */

GLOBAL void trace_message_l_dl (T_HANDLE handle,
                                char *str,
                                U32 link_id,
                                U16 length,
                                U32 entity_options)
{
  if(! (entity_options & DTI_NO_TRACE) )
  {
    TRACE_EVENT_P3("%s - link_id=%d length=%d", str, link_id, length);
  }
} /* trace_message_l_dl() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : trace_message_iic        |
+--------------------------------------------------------------------+

 *
 *  Create a string and a number
 *
 */

GLOBAL void trace_message_iic (T_HANDLE handle,
                              char *str,
                              U8 instance,
                              U8 interfac,
                              U8 channel,
                              U32 entity_options)
{
  if(!(entity_options & DTI_NO_TRACE))
  {
    TRACE_EVENT_P4("%s - instance=%d interface=%d channel=%d",
                   str,
                   instance,
                   interfac,
                   channel);
  }
} /*lint !e715 handle not referenced trace_message_iic() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : trace_message_l_e        |
+--------------------------------------------------------------------+

 *
 *  Create a string and a number
 *
 */

GLOBAL void trace_message_l_e (T_HANDLE handle,
                              char *str,
                              U32 link_id,
                              char* entity,
                              U32 entity_options)
{
  if(!(entity_options & DTI_NO_TRACE))
  {
    TRACE_EVENT_P3("%s - link_id=%d neighbor_entity=%s",
                   str,
                   link_id,
                   entity);
  }
} /*lint !e715 handle not referenced trace_message_l_e() */

/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : trace_function           |
+--------------------------------------------------------------------+

 *
 * Call macro TRACE_FUNKTION if traces are enabled
 *
 */

GLOBAL void trace_function( T_HANDLE handle, char *str, U32 entity_options)
{
  if(! (entity_options & DTI_NO_TRACE) )
  {
    TRACE_FUNCTION (str);
  }
} /*lint !e715 handle not referenced trace_function() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB       MODULE  :    DTI_KERF                       |
| STATE   : code         ROUTINE :  get_pointer_link_table_channel   |
+--------------------------------------------------------------------+

 *
 *  The function searchs an instance and a channel in the database link list
 *  and returns a pointer to the link list. If there is no link with the same channel and
 *  the same interface it returns NULL.
 */

GLOBAL DTI_LINK * get_pointer_link_table_channel (DTI_HANDLE hDTI,
                                                 U8 instance,
                                                 U8 interfac,
                                                 U8 channel)
{
  DTI_LINK *link_last, *link;
  BOOL go = TRUE;

  trace_function(hDTI->handle,
                 "get_pointer_link_table_channel()", /*lint !e605 Increase in pointer capability */
                 hDTI->entity_options);

  link = (DTI_LINK *) hDTI->first_link;

  /*
   *  Check if the link is not set
   */

  if(link EQ NULL)
    return NULL;

  do {
    link_last = link;

    /*
     *  Search on instance, interface and channal
     */

    if((link_last->instance EQ instance) AND (link_last->channel EQ channel) AND
      (link_last->interfac EQ interfac) AND
      (link_last->link_id NEQ D_FREE_LINK_ID))

      return link_last;

    if(link->next_link NEQ D_LINK)
      link = (DTI_LINK *) link->next_link;
    else
      go = FALSE;

  } while( go );

  return NULL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : get_pointer_link_table   |
+--------------------------------------------------------------------+

 *
 *  The function searchs a link id in the databank link list and returns a
 *  pointer to the link list. If there is no link_id with the selected direction it
 *  returns NULL. Note: the direction is used to select between to equal
 *  links in a link list because it could be 2 channels are connected on one
 *  instance.
 */

GLOBAL DTI_LINK * get_pointer_link_table (DTI_HANDLE hDTI,
                                          U32 select_link_id,
                                          U8 select_direction)
{
  DTI_LINK* link;

  trace_function(hDTI->handle,
                 "get_pointer_link_table()",
                 hDTI->entity_options);

  link = (DTI_LINK *) hDTI->first_link;

  /*
   *  Find the link and return it
   */
  while(link NEQ NULL)
  {
    if(link->link_id EQ select_link_id)
      return link;

    link = (DTI_LINK *) link->next_link;
  }

  return NULL;
} /* get_pointer_link_table() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : get_pointer_free_link    |
+--------------------------------------------------------------------+

PURPOSE : The function searches a free link in the database link list
          and returns a pointer to the link structure. If there is no
          linkin the database then it returns NULL.
*/

GLOBAL DTI_LINK * get_pointer_free_link (DTI_HANDLE hDTI)
{
  DTI_LINK* link;

  trace_function( hDTI->handle,
    "get_pointer_free_link()",
    hDTI->entity_options);

  /*
   * Search for unused link
   */
  link = (DTI_LINK *) hDTI->first_link;
  while(link)
  {
    if(link->connect_state EQ DTI_CLOSED)
      return link;
    link = (DTI_LINK*)link->next_link;
  }

  /*
   * Search for a link where only DTI_CONNECT_REQ has been received.
   * This solves the situation when dti_open() is called for a new link
   * and all links are used by existing links
   * which are going to be closed and reopened with an other link_id
   */
  link = (DTI_LINK *) hDTI->first_link;
  while(link)
  {
    if(link->connect_state EQ DTI_CONNECTING)
      return link;
    link = (DTI_LINK*)link->next_link;
  }

  return NULL;
} /* get_pointer_free_link() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : open_comm_channel        |
+--------------------------------------------------------------------+

 *
 *  This function opens a VSI communication channel. If it's open it will be closed
 *  and opened new again. The opened handle is returned in the parameter com_handle.
 */

GLOBAL void open_comm_channel(T_HANDLE entity_handle,
                             T_HANDLE *com_handle,
                             char *name,
                             U32 entity_options)
{

  trace_function( entity_handle,
    "open_comm_channel()",
    entity_options);

  /* Be sure the channel is closed if it was opened before */
  vsi_c_close (entity_handle, *com_handle);

  /* Open */
  *com_handle = vsi_c_open(entity_handle, name);
}

/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : check_dti_version        |
+--------------------------------------------------------------------+

 *
 *  This function checks the version in a link table.
 *  and returns TRUE if the same version is found as in the request_version.
 */

GLOBAL BOOL check_dti_version (DTI_HANDLE hDTI, U32 request_version)
{
  trace_function(hDTI->handle,
                 "check_dti_version()", /*lint !e605 Increase in pointer capability */
                 hDTI->entity_options);

  /*
   *  DTILIB has to be able to use older versions too.
   *  NOTE: this has been D_VERSION <= request_version until Aug-2002
   *        just an error or some sort of intricate reasoning??
   */
  if(D_VERSION >= request_version)
    return TRUE;
  else
    return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : validate_open_parameters |
+--------------------------------------------------------------------+

 *
 *  This function validates the open-parameters
 */
GLOBAL DTI_RESULT validate_open_parameters(DTI_HANDLE hDTI,
                                          U32 link_id,
                                          U8 *queue_size,
                                          U8 *direction,
                                          U32 *link_options,
                                          U32 version,
                                          U8 *neighbor_entity)
{
  trace_function(hDTI->handle,
                 "validate_open_parameters()", /*lint !e605 Increase in pointer capability */
                 hDTI->entity_options);

  /*
   *  Check version of peer dtilib
   */
  if (
    check_dti_version
      (
      hDTI,
      version
      ) EQ FALSE
    )
  {
    trace_message_link_id(hDTI->handle,
      "DTI ERROR: Wrong DTI version", /*lint !e605 Increase in pointer capability */
      link_id,
      hDTI->entity_options);
    return DTI_E_FAIL;
  }

  /*
   * Legacy
   */

  /*
   *  Check if open with the NULL device.
   */
  if (strcmp ((CHAR*)neighbor_entity, NULL_NAME) EQ 0)
  {
    *direction = DTI_NULL_LINK;
  }
  if (*link_options EQ FLOW_CNTRL_ENABLED)
  {
    if (*queue_size EQ DTI_QUEUE_DISABLED)
    {
      *link_options = DTI_QUEUE_UNUSED;
    }
    else if (*queue_size EQ DTI_QUEUE_UNLIMITED)
    {
      *link_options = DTI_QUEUE_UNBOUNDED;
    }
    else
    {
      /*
       * Set default behaviour
       */
      *link_options = DTI_QUEUE_RM_LIFO;
    }
  }

  /*
   * Ensure consistency
   */
  if((*link_options EQ DTI_QUEUE_UNUSED) OR
     (*link_options EQ DTI_FLOW_CNTRL_DISABLED) OR
     (*link_options EQ DTI_QUEUE_UNBOUNDED))
  {
    *queue_size = 0;
  }
  else if(*queue_size EQ 0)
  {
    *link_options = DTI_QUEUE_UNUSED;
  }

  return DTI_S_OK;
}


/*
+-----------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERF                |
| STATE   : code                      ROUTINE : set_link_parameter      |
+-----------------------------------------------------------------------+

 *
 *  Set parameter in a link desc.
 */

GLOBAL void set_open_para_link_table (DTI_HANDLE hDTI,
                                      DTI_LINK *link,
                                      U32   version,
                                      U32   link_options,
                                      U8    instance,
                                      U8    interfac,
                                      U8    channel,
                                      U8    queue_size,
                                      U8    connect_state)
{
  trace_function(hDTI->handle,
                 "set_open_para_link_table()",
                 hDTI->entity_options);

  /*
   * Free primitives and descs if there are any in the queue.
   */
  free_dti_data_ind_packets_queue(hDTI, link->dti_data_ind);
  link->dti_data_ind = NULL;
  /*
   * set parameters as requested
   */
  link->version       = version;
  link->instance      = instance;
  link->interfac      = interfac;
  link->channel       = channel;
  link->link_handle   = D_LINK_HANDLE;
  link->queue_len     = 0;
  link->rx_state      = DTI_RX_IDLE;
  link->tx_state      = DTI_TX_BUFFER_FULL;
  link->connect_state = connect_state;
  link->queue_size    = queue_size;
  link->link_options  = link_options;
  /*
   * the following structure is needed for ATP links
   */
#ifdef FF_TCP_IP
  link->entity_db     = D_HANDLE;
  link->port_nb       = 0;
#endif /* FF_TCP_IP */
} /* set_open_para_link_table() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : dti_resolve_link_id      |
+--------------------------------------------------------------------+

 *
 * This function returns the link_id for the given
 * instance/interface/channel.
 *
 * This is to be used to identify primitives sent from the entity to ACI
 *
 * return value:  TRUE if associated link_id is found
 *                FALSE, otherwise
 */

GLOBAL BOOL dti_resolve_link_id (DTI_HANDLE hDTI,
                                 U8 instance,
                                 U8 interfac,
                                 U8 channel,
                                 U32 *link_id)
{
  DTI_LINK *link;

  trace_function(hDTI->handle,
                 "dti_resolve_link_id()", /*lint !e605 Increase in pointer capability */
                 hDTI->entity_options);

  if((link = get_pointer_link_table_channel(hDTI,
                                            instance,
                                            interfac,
                                            channel)) EQ NULL)
  {
    return FALSE;
  }

  *link_id = link->link_id;
  return TRUE;
} /* dti_resolve_link_id() */

/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                 |
| STATE   : code                  ROUTINE : put_dti_data_ind_in_queue|
+--------------------------------------------------------------------+

 *
 *  This function puts a dti_data_req primitive in queue.
 */

GLOBAL void put_dti_data_ind_in_queue( DTI_HANDLE hDTI, DTI_LINK *link,
  T_DTI2_DATA_IND *dti_data_ind)
{

  if (hDTI->handle NEQ 0)
  {
    trace_function( hDTI->handle,
      "put_dti_data_ind_in_queue()",
      hDTI->entity_options);
  }

  dti_data_ind->link_id = 0;

  /*
   * Put in the first data packet.
   * When using the old SAP the link->queue len can be 0 or 1
   */
  {
    T_DTI2_DATA_IND *p_ind;
    U8 chk     = 1;

    if(link->queue_len EQ 0)
    {
      link->dti_data_ind = dti_data_ind;
      link->queue_len++;
      return;
    }


    /* Now put more packets in the queue and put the new one on the last position */
    p_ind = link->dti_data_ind;

    /*
     *    Search the last packet in queue.
     */
    while(p_ind->link_id NEQ 0)
    {
      p_ind = (T_DTI2_DATA_IND *) p_ind->link_id;
      chk++;
    }

    p_ind->link_id = (U32) dti_data_ind;

    /*
     * Check the queue length.
     */
    if(chk != link->queue_len)
    {
      trace_message_link_id(hDTI->handle,
        "DTI ERROR: Write to queue error", /*lint !e605 Increase in pointer capability */
        link->link_id,
        hDTI->entity_options);
      return;
    }
    link->queue_len++;
  }
}



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB             MODULE  : DTI_KERF                    |
| STATE   : code               ROUTINE : get_dti_data_ind_from_queue |
+--------------------------------------------------------------------+

 *
 *  This function gets a dti_data_ind primitive from queue. If there is
 *  no packet in the queue it returns NULL.
 */

GLOBAL T_DTI2_DATA_IND * get_dti_data_ind_from_queue(DTI_HANDLE hDTI,
                                                     DTI_LINK *link)
{
  T_DTI2_DATA_IND *ret_dti_data_ind;

  trace_function( hDTI->handle,
    "get_dti_data_ind_from_queue()",
    hDTI->entity_options);

  /*
   *    Is there any packet in the queue ?
   */
  if(link->queue_len EQ 0)
  {
    return NULL;
  }
  /*
   *    Set the second data packet on the first position.
   */
  ret_dti_data_ind = link->dti_data_ind;
  /*
   *    Set the queue length
   */
  link->queue_len--;
  /*
   *  Put the next data packet in position.
   */
  link->dti_data_ind = (T_DTI2_DATA_IND *)link->dti_data_ind->link_id;

  return (ret_dti_data_ind);
} /* get_dti_data_ind_from_queue() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB        MODULE  : DTI_KERF                         |
| STATE   : code          ROUTINE : get_last_dti_data_ind_from_queue |
+--------------------------------------------------------------------+

 *
 * This function gets a dti_data_ind primitive from queue. If there is
 * no packet in the queue it returns NULL.
 */

LOCAL T_DTI2_DATA_IND * get_last_dti_data_ind_from_queue(DTI_HANDLE hDTI,
                                                         DTI_LINK *link)
{
  T_DTI2_DATA_IND **last_dti_data_ind;
  T_DTI2_DATA_IND *ret_dti_data_ind;

  trace_function(hDTI->handle,
    "get_last_dti_data_ind_from_queue()",
    hDTI->entity_options);

  /*
   * Is there any packet in the queue?
   */
  if(link->queue_len EQ 0)
  {
    return NULL;
  }
  /*
   * Get the last data packet
   */
  last_dti_data_ind = &(link->dti_data_ind);
  while((*last_dti_data_ind)->link_id NEQ 0)
  {
    last_dti_data_ind = (T_DTI2_DATA_IND **) &((*last_dti_data_ind)->link_id);
  }
  ret_dti_data_ind = *last_dti_data_ind;
  /*
   * Set the queue length
   */
  link->queue_len--;
  /*
   * remove the data packet from queue.
   */
  *last_dti_data_ind = 0;

  return (ret_dti_data_ind);
} /* get_last_dti_data_ind_from_queue() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB        MODULE  : DTI_KERF                         |
| STATE   : code          ROUTINE : put_dti_data_ind_in_queue_managed|
+--------------------------------------------------------------------+

 *
 *  This function puts a dti_data_req primitive in queue.
 *  If the queue is full, it acts accordingly
 */

GLOBAL void put_dti_data_ind_in_queue_managed (DTI_HANDLE hDTI,
                                              DTI_LINK *link,
                                              T_DTI2_DATA_IND *dti_data_ind)
{
  trace_function(hDTI->handle,
                 "put_dti_data_ind_in_queue_managed()", /*lint !e605 Increase in pointer capability */
                 hDTI->entity_options);

  /*
   * Put new data packet into the queue.
   */
  if(link->link_options NEQ DTI_QUEUE_UNUSED)
  {
    put_dti_data_ind_in_queue(hDTI, link, dti_data_ind);
  }

  /*
   * If queue full - send signal to entity.
   */
  switch (link->link_options)
  {
  case DTI_QUEUE_UNBOUNDED:
  case DTI_QUEUE_WATERMARK:
    break;

  case DTI_QUEUE_UNUSED:
    /* free data primitive since we cannot handle it */

    trace_message_link_id(hDTI->handle,
      "DTI ERROR: No send queue available, discarding data packet", /*lint !e605 Increase in pointer capability */
      link->link_id,
      hDTI->entity_options);

    mfree_desc (hDTI, &dti_data_ind->desc_list2);
    PFREE (dti_data_ind);
    break;

  case DTI_QUEUE_RM_FIFO:
    if (link->queue_len > link->queue_size)
    {
      T_DTI2_DATA_IND *free_dti_data_ind
        = get_dti_data_ind_from_queue(hDTI, link);

      trace_message_link_id(hDTI->handle,
        "Send queue full, discarding oldest data packet", /*lint !e605 Increase in pointer capability */
        link->link_id,
        hDTI->entity_options);

      mfree_desc (hDTI, &free_dti_data_ind->desc_list2);
      PFREE (free_dti_data_ind);
    }
    break;

  case DTI_QUEUE_RM_LIFO:
    if (link->queue_len > link->queue_size)
    {
      T_DTI2_DATA_IND *free_dti_data_ind
        = get_last_dti_data_ind_from_queue(hDTI, link);

      trace_message_link_id(hDTI->handle,
        "Send queue full, discarding newest data packet", /*lint !e605 Increase in pointer capability */
        link->link_id,
        hDTI->entity_options);

      mfree_desc (hDTI, &free_dti_data_ind->desc_list2);
      PFREE (free_dti_data_ind);
    }
    break;

  default:
    trace_message_link_id(hDTI->handle,
      "DTI ERROR: Unknown state for link_options", /*lint !e605 Increase in pointer capability */
      link->link_id,
      hDTI->entity_options);
    break;
  }
} /* put_dti_data_ind_in_queue_managed() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERF             |
| STATE   : code                      ROUTINE : dti_make_new_desc    |
+--------------------------------------------------------------------+

 *
 *  Malloc a knew desc. Init a buffer 0 if buff_init_0 is TRUE.
 *
 */

BOOL dti_make_new_desc( DTI_HANDLE hDTI, T_desc2 **p_desc_new,
  U16 malloc_len, BOOL buff_init_0)
{
#define VSI_CALLER hDTI->handle,

  trace_function( hDTI->handle,
    "dti_make_new_desc()",
    hDTI->entity_options);

  MALLOC ((*p_desc_new), ((U16)(sizeof(T_desc2)-1 + malloc_len)));

  if(*p_desc_new EQ 0)
    return(FALSE);

  if(buff_init_0)
  {
    U16 i;
    for(i=0; i<malloc_len; i++)
      (*p_desc_new)->buffer[i] = 0;
  }
  (*p_desc_new)->len    = malloc_len;
  (*p_desc_new)->next   = 0;
  (*p_desc_new)->offset = 0;
  (*p_desc_new)->size   = (*p_desc_new)->len + (*p_desc_new)->offset;
  return(TRUE);

#undef VSI_CALLER
}


/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERF             |
| STATE   : code                      ROUTINE : mfree_desc           |
+--------------------------------------------------------------------+

 *
 *  Free the descs in a desc list.
 *
 */


GLOBAL void mfree_desc(DTI_HANDLE hDTI, T_desc_list2 * desc_list2)
{
  BOOL go = TRUE;
  T_desc2 *p_desc_last, *p_desc;

  trace_function( hDTI->handle,
    "mfree_desc()",
    hDTI->entity_options);

  if (desc_list2 EQ NULL)
    return;

  if (desc_list2->first EQ 0)
    return;

  p_desc = (T_desc2 *) desc_list2->first;

  do {
    p_desc_last = p_desc;

    if(p_desc->next >0)
      p_desc = (T_desc2 *) p_desc->next;
    else
      go = FALSE;

    MFREE (p_desc_last);

  } while( go );
}


/*
+-----------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERF                |
| STATE   : code                      ROUTINE : close_link_with_signal  |
+-----------------------------------------------------------------------+

 *
 *  Close a link.
 */

GLOBAL void close_link_with_signal(DTI_HANDLE hDTI, DTI_LINK *link)
{
  U8 hlp_instance, hlp_interface;
  U8 hlp_channel;

  trace_function( hDTI->handle,
    "close_link_with_signal()",
    hDTI->entity_options);

  /*
   *  Close communication channel
   */
  vsi_c_close (hDTI->handle, link->link_handle);

  hlp_instance   =   link->instance;
  hlp_interface  =   link->interfac;
  hlp_channel    =   link->channel;

  /*
   *  Set the default parameter and call the callback function.
   */

  set_default_para_link_table(hDTI,link, D_FREE_LINK_ID, D_DIRECTION);

  /*
   *  Call the callback function.
   */

  hDTI->sig_callback(hlp_instance, hlp_interface, hlp_channel,
    DTI_REASON_CONNECTION_CLOSED, NULL);

  /*
   * Note: Any flow control and any data primitive are now silently discarded.
   */
}



/*
+-------------------------------------------------------------------------+
| PROJECT : DTILIB                MODULE  : DTI_KERF                      |
| STATE   : code                  ROUTINE : set_reset_req_para_link_table |
+-------------------------------------------------------------------------+
 *
 *  Set parameter by reset.
 *
 */

GLOBAL void set_reset_req_para_link_table(DTI_HANDLE hDTI, DTI_LINK *link)
{
  /*
   * Free primitives and descs if there are any in the queue.
   */
  free_dti_data_ind_packets_queue(hDTI, link->dti_data_ind);
  link->dti_data_ind = NULL;
  /*
   * Set the other parameters
   */
  link->queue_len     = 0;
  link->connect_state = DTI_IDLE;
} /* set_reset_req_para_link_table() */
