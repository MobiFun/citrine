/*
+-----------------------------------------------------------------------------
|  Project :  ...
|  Modul   :  dti_conn_mng.c
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
|  Purpose :  Implementation of DTI Connection Manager
+-----------------------------------------------------------------------------
*/


#ifndef DTI_CONN_MNG_C
#define DTI_CONN_MNG_C
#endif


/*===== INCLUDES ===================================================*/

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "pconst.cdg"
#include "mconst.cdg"
#include "cnf_aci.h"
#include "mon_aci.h"
#include "pei.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"

#include "aci_cmh.h"
#include "aci_lst.h"
#include "aci_mem.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"



LOCAL ULONG        used_dti_channels = 0;
LOCAL T_ACI_LIST  *dti_channel_list = NULL; /* list_begin */

LOCAL T_DTI_CONN_PARAMS glob_params;


/********************** LOCAL FUNCTIONS *****************************/


LOCAL UBYTE dti_conn_get_new_dti_id (void)
{
  ULONG tst_id;
  UBYTE i;

  TRACE_FUNCTION("dti_conn_get_new_dti_id()");

  /* create a new DTI ID */
  for (i = 0; i <= MAX_DTI_CONN_CHANNELS; i++)
  {
    tst_id = (0x01 << i) & used_dti_channels;
    if (!tst_id)
    {
      break;
    }
  }
  if (tst_id)
  {
    TRACE_EVENT("No DTI ID available");
    return (DTI_DTI_ID_NOTPRESENT);
  }
  used_dti_channels |= (0x01 << i);

  TRACE_EVENT_P1("DTI ID %d created", i);

  return (i);
}


LOCAL void dti_conn_remove_dti_id (UBYTE dti_id)
{
  ULONG tst_id;
  
  TRACE_FUNCTION("dti_conn_remove_id()");
  
  if (dti_id >= MAX_DTI_CONN_CHANNELS)
    return;

  tst_id = 0x01 << (dti_id);
  used_dti_channels &= ~tst_id;

  TRACE_EVENT_P1("DTI ID %d removed", dti_id);

}


LOCAL BOOL dti_conn_error_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  /* theorically should not be called... Yet helpfull for testing */
  TRACE_FUNCTION("dti_conn_error_cb()");

  TRACE_EVENT("ERROR: DTI connection callback has not been initialized !");

  return FALSE;
}


LOCAL void dti_conn_reset_conn_parms( T_DTI_CONN_CHANNEL *dti_channel )
{

  UBYTE i,j;

  TRACE_FUNCTION("dti_conn_reset_conn_parms()");

  dti_channel->erase_channel = FALSE;
  dti_channel->num_of_conns  = 0;
  dti_channel->conn_cb       = dti_conn_error_cb;
  dti_channel->state         = DTI_CONN_STATE_DISCONNECTED;

  for (i=0; i<MAX_DTI_CONN_TUPLES; i++)
  {
    dti_channel->tuple_list[i].state = DTI_CONN_STATE_DISCONNECTED;
    for (j=0; j<NUM_OF_PEERS; j++)
    {
      dti_channel->tuple_list[i].peers[j].state  = DTI_CONN_STATE_DISCONNECTED;
      dti_channel->tuple_list[i].peers[j].ent_id = DTI_ENTITY_INVALID;
    }
  }
}


LOCAL BOOL DTItest_dti_id( UBYTE dti_id, void *elem)
{
  T_DTI_CONN_CHANNEL *dti_channel = (T_DTI_CONN_CHANNEL *)elem;

  if (dti_channel EQ NULL)
    return FALSE;
  if (dti_channel -> dti_id EQ dti_id )
    return TRUE;
  else
    return FALSE;
}

LOCAL T_DTI_CONN_TUPLE *dti_conn_find_dti_tuple( T_DTI_CONN_CHANNEL *dti_channel, UBYTE tuple_no)
{
   return &(dti_channel->tuple_list[tuple_no]);
}


LOCAL T_DTI_CONN_CHANNEL *dti_conn_find_dti_conn( UBYTE dti_id )
{
  T_DTI_CONN_CHANNEL *dti_channel;

  TRACE_FUNCTION("dti_conn_find_dti_conn()");
  
  /*find channel in list */
  dti_channel = find_element (dti_channel_list, dti_id, DTItest_dti_id);

  return dti_channel;
}


LOCAL BOOL dti_conn_connect( T_DTI_CONN_CHANNEL* dti_channel, 
                              T_DTI_ENTITY_ID*   entity_list, 
                              UBYTE              num_entities, 
                              T_DTI_CONN_MODE    mode )
{
  ULONG link_id;
  UBYTE tuple_no;
  UBYTE i;

  TRACE_FUNCTION("dti_conn_connect()");

  dti_channel->state = DTI_CONN_STATE_CONNECTING;

  if (dti_channel->conn_cb)
  {
    (void )dti_channel->conn_cb(dti_channel->dti_id, DTI_CONN_STATE_CONNECTING);
  }
  else
  {
    TRACE_EVENT("conn_cb is NULL");
  }


  if (mode EQ APPEND)
  {
    tuple_no = dti_channel->num_of_conns;
  }
  else /* SPLIT */
  {
    tuple_no = 0;
    dti_channel->num_of_conns = 0;
  }

  for (i=0; i<num_entities-1; i++)
  {
    dti_channel->tuple_list[tuple_no].peers[0].ent_id = entity_list[i];
    dti_channel->tuple_list[tuple_no].peers[1].ent_id = entity_list[i+1];

    if ( (dti_channel->tuple_list[tuple_no].state NEQ DTI_CONN_STATE_DISCONNECTING) AND
         (dti_channel->tuple_list[tuple_no].state NEQ DTI_CONN_STATE_CONNECTING) )
    {
      dti_channel->tuple_list[tuple_no].state = DTI_CONN_STATE_CONNECTING;
      dti_channel->tuple_list[tuple_no].tuple_no = tuple_no;
      dti_channel->state = DTI_CONN_STATE_CONNECTING;

      link_id = dti_conn_compose_link_id(0, 0, dti_channel->dti_id, tuple_no);

      dti_channel->tuple_list[tuple_no].peers[0].state = DTI_CONN_STATE_CONNECTING;
      dti_channel->tuple_list[tuple_no].peers[1].state = DTI_CONN_STATE_CONNECTING;

      /* connect a tuple */
     (void) glob_params.mng_ent_cb(link_id, entity_list[i], entity_list[i+1], DTI_CONNECT);
      (void)glob_params.mng_ent_cb(link_id, entity_list[i+1], entity_list[i], DTI_CONNECT);
    }

    tuple_no++;
  }

  dti_channel->num_of_conns = tuple_no;

  return TRUE;
}


LOCAL BOOL dti_conn_disconnect( T_DTI_CONN_CHANNEL* dti_channel )
{
  ULONG link_id;
  UBYTE i;

  TRACE_FUNCTION("dti_conn_disconnect()");

  dti_channel->state = DTI_CONN_STATE_DISCONNECTING;

  if (dti_channel->conn_cb)
  {
    (void)dti_channel->conn_cb(dti_channel->dti_id, DTI_CONN_STATE_DISCONNECTING);
  }
  else
  {
    TRACE_EVENT("conn_cb is NULL");
  }

  for (i=0; i < dti_channel->num_of_conns; i++)
  {

    /* set tuple state only if it is not already DISCONNECTED */
    if (dti_channel->tuple_list[i].state NEQ DTI_CONN_STATE_DISCONNECTED)
    {
      dti_channel->tuple_list[i].state = DTI_CONN_STATE_DISCONNECTING;
    }

    link_id = dti_conn_compose_link_id(0, 0, dti_channel->dti_id, i);

    /* disconnect only a connected entity */
    if (dti_channel->tuple_list[i].peers[0].state EQ DTI_CONN_STATE_CONNECTED  OR
		dti_channel->tuple_list[i].peers[0].state EQ DTI_CONN_STATE_CONNECTING    )
    {
      dti_channel->tuple_list[i].peers[0].state = DTI_CONN_STATE_DISCONNECTING;
      (void)glob_params.mng_ent_cb(link_id, dti_channel->tuple_list[i].peers[0].ent_id, 
                           dti_channel->tuple_list[i].peers[1].ent_id, DTI_DISCONNECT);
    }

    /* disconnect only a connected entity */
    if (dti_channel->tuple_list[i].peers[1].state EQ DTI_CONN_STATE_CONNECTED  OR
		dti_channel->tuple_list[i].peers[1].state EQ DTI_CONN_STATE_CONNECTING    )
    {
      dti_channel->tuple_list[i].peers[1].state = DTI_CONN_STATE_DISCONNECTING;
      (void)glob_params.mng_ent_cb(link_id, dti_channel->tuple_list[i].peers[1].ent_id, 
                            dti_channel->tuple_list[i].peers[0].ent_id, DTI_DISCONNECT);
    }
  }

  return TRUE;
}







/********************** GLOBAL FUNCTIONS *****************************/


GLOBAL T_DTI_CONN_LINK_ID dti_conn_compose_link_id(UBYTE dummy, UBYTE assoc, UBYTE dti_id, UBYTE tuple_no)
{
  T_DTI_CONN_LINK_ID link_id = 0;

  link_id += dummy;
  link_id <<= 8;
  link_id += assoc;
  link_id <<= 8;
  link_id += dti_id;
  link_id <<= 8;
  link_id += tuple_no;

  return link_id;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                             MODULE  : DTI_CONN           |
| STATE   : code                        ROUTINE : dti_conn_init      |
+--------------------------------------------------------------------+

  PURPOSE : initialise the DTI Connection Manager

*/
GLOBAL void dti_conn_init( T_DTI_CONN_MNG_ENT_CB* mng_ent_cb )
{
  TRACE_FUNCTION("dti_conn_init()");

  dti_channel_list = new_list ();
 
  glob_params.conn_cb      = NULL;
  glob_params.mng_ent_cb   = mng_ent_cb;
  glob_params.num_entities = 0;

}


/*
+--------------------------------------------------------------------+
| PROJECT :                             MODULE  : DTI_CONN           |
| STATE   : code                        ROUTINE : dti_conn_new       |
+--------------------------------------------------------------------+

  PURPOSE : register new DTI channel

*/
GLOBAL UBYTE dti_conn_new(UBYTE dti_id )
{
  T_DTI_CONN_CHANNEL *dti_channel;

  TRACE_FUNCTION("dti_conn_new()");


  if (dti_id EQ DTI_DTI_ID_NOTPRESENT)
  {
    dti_id = dti_conn_get_new_dti_id ();
  }

  if (dti_id EQ DTI_DTI_ID_NOTPRESENT)
  {
    return (DTI_DTI_ID_NOTPRESENT);
  }

  /* search for an existing entry with correspondant dti_id */
  dti_channel = dti_conn_find_dti_conn (dti_id);

  if( dti_channel )
  {
    return (DTI_DTI_ID_NOTPRESENT); /* couldn't create a new entry */
  }

  ACI_MALLOC(dti_channel, sizeof(T_DTI_CONN_CHANNEL));

  dti_conn_reset_conn_parms (dti_channel);
  dti_channel->dti_id = dti_id;
  insert_list(dti_channel_list, dti_channel);

  return (dti_channel->dti_id);
}


/*
+---------------------------------------------------------------------+
| PROJECT :                            MODULE  : DTI_CONN             |
| STATE   : code                       ROUTINE : dti_conn_erase_entry |
+---------------------------------------------------------------------+

  PURPOSE :  erase entry from DTI channel list 

*/
GLOBAL void dti_conn_erase_entry(UBYTE dti_id)
{
  T_DTI_CONN_CHANNEL *dti_channel;

  TRACE_FUNCTION("dti_conn_erase_entry");

  /* find element to be erased */
  dti_channel = remove_element (dti_channel_list, dti_id, DTItest_dti_id);

  if( dti_channel NEQ NULL ) /* entry not erased yet */
    ACI_MFREE (dti_channel);

  dti_conn_remove_dti_id(dti_id);
}


/*
+-----------------------------------------------------------------------+
| PROJECT :               MODULE  : DTI_CONN                            |
| STATE   : code          ROUTINE : dti_conn_is_dti_channel_connected   |
+-----------------------------------------------------------------------+ 

  PURPOSE : returns TRUE if end-to-end DTI connection is connected and 
            the given entity is in this connection

*/
GLOBAL BOOL dti_conn_is_dti_channel_connected( T_DTI_ENTITY_ID ent_id, UBYTE dti_id )
{
  UBYTE count = 0;
  UBYTE i;
  UBYTE entity_found = FALSE;
  T_DTI_CONN_CHANNEL *dti_channel = dti_conn_find_dti_conn( dti_id );

  TRACE_FUNCTION("dti_conn_is_dti_channel_connected()");

  if (dti_channel EQ NULL)
  {
    TRACE_EVENT_P1("dti_channel for dti_id %d not found", dti_id);
    return FALSE;
  }

  for (i=0; i<dti_channel->num_of_conns; i++)
  {
    if ((dti_channel->tuple_list[i].peers[0].ent_id EQ ent_id) OR
        (dti_channel->tuple_list[i].peers[1].ent_id EQ ent_id))
    {
      entity_found = TRUE;
      break;
    }
  }

  if (entity_found EQ FALSE)
    return FALSE;

  if (dti_channel->state EQ DTI_CONN_STATE_CONNECTED)
  {
    return TRUE;
  }
  else
  {
    for (i=0; i<dti_channel->num_of_conns; i++)
    {
      if (dti_channel->tuple_list[i].state EQ DTI_CONN_STATE_CONNECTED)
      {
        count++;
      }
    }

    if (count EQ dti_channel->num_of_conns)
    {
      return TRUE;
    }
  }

  return FALSE;
}


/*
+--------------------------------------------------------------------------+
| PROJECT :               MODULE  : DTI_CONN                               |
| STATE   : code          ROUTINE : dti_conn_is_dti_channel_disconnected   |
+--------------------------------------------------------------------------+ 

  PURPOSE : returns TRUE if end-to-end DTI connection is disconnected

*/
GLOBAL BOOL dti_conn_is_dti_channel_disconnected( UBYTE dti_id )
{
  UBYTE count = 0;
  UBYTE i;
  T_DTI_CONN_CHANNEL *dti_channel = dti_conn_find_dti_conn( dti_id );

  TRACE_FUNCTION("dti_conn_is_dti_channel_disconnected()");


  if (dti_channel EQ NULL)
  {
    return TRUE;
  }

  if (dti_channel->state EQ DTI_CONN_STATE_DISCONNECTED)
  {
    return TRUE;
  }
  else
  {
    for (i=0; i<dti_channel->num_of_conns; i++)
    {
      if (dti_channel->tuple_list[i].state EQ DTI_CONN_STATE_DISCONNECTED)
      {
        count++;
      }
    }

    if (count EQ dti_channel->num_of_conns)
    {
      return TRUE;
    }
  }

  return FALSE;
}


/*
+---------------------------------------------------------------------+
| PROJECT :                             MODULE  : DTI_CONN            |
| STATE   : code                        ROUTINE : dti_conn_est_dpath  |
+---------------------------------------------------------------------+

  PURPOSE : establishes data path.

*/
GLOBAL BOOL dti_conn_est_dpath( UBYTE               dti_id,
                                T_DTI_ENTITY_ID*    entity_list,
                                UBYTE               num_entities,
                                T_DTI_CONN_MODE     mode,
                                T_DTI_CONN_CB*      cb)
{
  T_DTI_CONN_CHANNEL *dti_channel;

  TRACE_FUNCTION("dti_conn_est_dpath()");


  /*find channel in list */
  dti_channel = dti_conn_find_dti_conn (dti_id);

  if (dti_channel EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] dti_conn_est_dpath: dti_channel dti_id=%x not found",
                   dti_id);
    /* DTI ID not found */
    return FALSE;
  }
  
  if ( entity_list[0] EQ DTI_ENTITY_INVALID OR entity_list[1] EQ DTI_ENTITY_INVALID)
  {
    /* improper entity list */
    TRACE_EVENT("[ERR] improper entity list ");
    return FALSE;
  }

  /* if 'SPLIT' then the whole DTI channel must be disconnected before a 
   * new connection is established 
   */
  if ((mode EQ SPLIT) AND (dti_channel->state NEQ DTI_CONN_STATE_DISCONNECTED))
  {
    dti_conn_disconnect (dti_channel);
    
    /* save new entity list to entity list buffer */
    memcpy(glob_params.entity_list_buf, entity_list, sizeof(T_DTI_ENTITY_ID)*num_entities);
    glob_params.num_entities = num_entities;
    glob_params.conn_cb = cb;
  }
  else
  {
    dti_channel->conn_cb = cb;
    dti_conn_connect (dti_channel, entity_list, num_entities, mode);
  }

  return TRUE;
}


/*
+---------------------------------------------------------------------+
| PROJECT :                           MODULE  : DTI_CONN              |
| STATE   : code                      ROUTINE : dti_conn_close_dpath  |
+---------------------------------------------------------------------+

  PURPOSE : closes a DTI connection

*/
GLOBAL BOOL dti_conn_close_dpath( UBYTE dti_id )
{
  T_DTI_CONN_CHANNEL *dti_channel;

  TRACE_FUNCTION("dti_conn_close_dpath()");


  dti_channel = dti_conn_find_dti_conn( dti_id );
  if (dti_channel EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] dti_conn_close_dpath: dti_channel dti_id=%x not found",
                   dti_id);
    return FALSE;
  }

  if (dti_channel->state NEQ DTI_CONN_STATE_DISCONNECTED)
  {
    dti_channel->state = DTI_CONN_STATE_DISCONNECTING;
        
    dti_conn_disconnect (dti_channel);
  }
  else
  {
    TRACE_EVENT("[ERR] dti_conn_close_dpath: try to close dpath which was DISCONNECTED");
    return FALSE;
  }

  return TRUE;
}


/*
+-----------------------------------------------------------------------+
| PROJECT :                        MODULE  : DTI_CONN                   |
| STATE   : code                   ROUTINE : dti_conn_entity_connected  |
+-----------------------------------------------------------------------+

  PURPOSE : response after a DTI connect request. This function is called 
            to inform the DTI Connection Manager that a connection is 
            established.

*/
GLOBAL void dti_conn_entity_connected( T_DTI_CONN_LINK_ID link_id, 
                                       T_DTI_ENTITY_ID    ent_id, 
                                       T_DTI_CONN_RESULT  result )
{
  T_DTI_CONN_CHANNEL    *dti_channel;
  T_DTI_CONN_TUPLE      *tuple;
  UBYTE                 i;
  UBYTE                 dti_id = EXTRACT_DTI_ID(link_id);
  BOOL                  both_entities_connected = FALSE;
  UBYTE                 count = 0;

  TRACE_FUNCTION("dti_conn_entity_connected()");


  dti_channel = dti_conn_find_dti_conn( dti_id );
  if (dti_channel EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] dti_conn_entity_connected: dti_channel link_id=%x not found",
                   link_id);
    return;
  }

  if (result EQ DTI_ERROR)
  {
    /* mark entity as disconnected */
    dti_conn_entity_disconnected( link_id, ent_id );

    /* close the whole DTI channel */
    dti_conn_close_dpath( dti_id );
    return;
  }

  if (dti_channel->state EQ DTI_CONN_STATE_CONNECTED)
  {
    TRACE_EVENT_P1("dti channel with dti_id=%x already connected", dti_id);
    return;
  }

  tuple = dti_conn_find_dti_tuple( dti_channel, EXTRACT_TUPLE_NO(link_id ) );

  for (i=0; i<NUM_OF_PEERS; i++)
  {
    if (tuple->peers[i].state EQ DTI_CONN_STATE_CONNECTED)
    {
      count++;
    }

    if (tuple->peers[i].ent_id EQ ent_id)
    {
      if (tuple->peers[i].state EQ DTI_CONN_STATE_CONNECTING)
      {
        tuple->peers[i].state = DTI_CONN_STATE_CONNECTED;
        count++;
      }
    }
    if (count EQ NUM_OF_PEERS)
    {
      both_entities_connected = TRUE;
    }
  }

  /* if both entities are CONNECTED */
  if (both_entities_connected)
  {
    tuple->state = DTI_CONN_STATE_CONNECTED;

    /* if all other tuples CONNECTED */
    if (dti_conn_is_dti_channel_connected(ent_id, dti_id))
    {
      dti_channel->state = DTI_CONN_STATE_CONNECTED;

      TRACE_EVENT_P1("DTI ID %d connected", dti_id);
      
      /* call connect_cb here */
      if (dti_channel->conn_cb)
      {
        (void)dti_channel->conn_cb(dti_id, DTI_CONN_STATE_CONNECTED);
      }
      else
      {
        TRACE_EVENT("conn_cb is NULL");
      }
    }
  }
}


/*
+-----------------------------------------------------------------------+
| PROJECT :                     MODULE  : DTI_CONN                      |
| STATE   : code                ROUTINE : dti_conn_entity_disconnected  |
+-----------------------------------------------------------------------+

  PURPOSE : response after a DTI connect request. This function is called 
            to inform the DTI Connection Manager that a connection is 
            closed.

*/
GLOBAL void dti_conn_entity_disconnected( T_DTI_CONN_LINK_ID link_id, T_DTI_ENTITY_ID ent_id )
{
  T_DTI_CONN_CHANNEL    *dti_channel;
  T_DTI_CONN_TUPLE      *tuple;
  UBYTE                 i;
  UBYTE                 dti_id = EXTRACT_DTI_ID(link_id);
  UBYTE                 count = 0;

  TRACE_FUNCTION("dti_conn_entity_disconnected()");


  dti_channel = dti_conn_find_dti_conn( dti_id );
  if (dti_channel EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] dti_conn_entity_disconnected: dti_channel link_id=%x not found",
                   link_id);
    return;
  }

  if (dti_channel->state EQ DTI_CONN_STATE_DISCONNECTED)
  {
    TRACE_EVENT_P1("dti channel with dti_id=%x already diconnected", dti_id);
    return;
  }


  tuple = dti_conn_find_dti_tuple( dti_channel, EXTRACT_TUPLE_NO(link_id) );

  for (i=0; i<NUM_OF_PEERS; i++)
  {
    if (tuple->peers[i].state EQ DTI_CONN_STATE_DISCONNECTED)
    {
      count++;
    }

    /* find disconnected entity */
    if (tuple->peers[i].ent_id EQ ent_id)
    {
      tuple->peers[i].state  = DTI_CONN_STATE_DISCONNECTED;
      tuple->peers[i].ent_id = DTI_ENTITY_INVALID;
      count++;

      tuple->state = DTI_CONN_STATE_DISCONNECTING;

      if (dti_channel->state EQ DTI_CONN_STATE_CONNECTED)
      {
        /* set DTI channel state */
        dti_channel->state = DTI_CONN_STATE_DISCONNECTING;
      }
    }
  }

  /* if both entities are DISCONNECTED */
  if (count EQ NUM_OF_PEERS)
  {
    /* set tuple state */
    tuple->state = DTI_CONN_STATE_DISCONNECTED;

    /* if all other tuples DISCONNECTED */
    if (dti_conn_is_dti_channel_disconnected(dti_id))
    {
      dti_channel->state = DTI_CONN_STATE_DISCONNECTED;

      TRACE_EVENT_P1("DTI ID %d disconnected", dti_id);

      /* reset number of conns */
      dti_channel->num_of_conns = 0;
      
      /* call disconnect_cb here */
      if (dti_channel->conn_cb)
      {
        (void)dti_channel->conn_cb(dti_id, DTI_CONN_STATE_DISCONNECTED);
      }
      else
      {
        TRACE_EVENT("conn_cb is NULL");
      }

      if (glob_params.num_entities NEQ 0)
      {
        dti_channel->conn_cb = glob_params.conn_cb;
        dti_conn_connect( dti_channel, glob_params.entity_list_buf, glob_params.num_entities, SPLIT );
        glob_params.num_entities = 0;
      }
      else
      {
        if (dti_channel->erase_channel EQ TRUE)
        {
          /* erase entry in DTI Conn Mng */
          dti_conn_erase_entry(dti_id);
        }
      }
    }
  }
}


/*
+-----------------------------------------------------------------------+
| PROJECT :                     MODULE  : DTI_CONN                      |
| STATE   : code                ROUTINE : dti_conn_close_all_connections|
+-----------------------------------------------------------------------+

  PURPOSE : This function is called to close all established DTI 
            connections.

*/
GLOBAL void dti_conn_close_all_connections()
{
  T_DTI_CONN_CHANNEL *dti_channel=NULL;

  TRACE_FUNCTION("dti_conn_close_all_connections()");
  
  while(TRUE)
  {
    dti_channel = get_next_element (dti_channel_list, dti_channel);
    if (dti_channel EQ NULL)
    {
      return;
    }

    if (dti_channel->state EQ DTI_CONN_STATE_CONNECTED)
    {
      dti_conn_close_dpath(dti_channel->dti_id);
      dti_channel->erase_channel = TRUE;
    }
    else
    {
      dti_conn_erase_entry(dti_channel->dti_id);
    }
   }
}
