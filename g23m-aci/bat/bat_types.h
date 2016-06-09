/*  
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  BAT library
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
|  Purpose :  types for binary AT commands
+-----------------------------------------------------------------------------
*/

#ifndef BAT_TYPES_H 
#define BAT_TYPES_H

#include "p_bat.h"
#include "bat_cfg.h"

/*********************************************************************************
 *
 * defines
 *
 *********************************************************************************/
typedef unsigned char  T_BAT_instance;
typedef unsigned short T_BAT_client;

#define BAT_RCV_BUF_SIZE 800 /* response of %SIMEF comes with 784 bytes ! */

/*********************************************************************************
 *
 * enumerations
 *
 *********************************************************************************/
typedef enum
{
  BAT_INSTANCE_IDLE = 0,
  BAT_INSTANCE_ACTIVATING,
  BAT_INSTANCE_READY,
  BAT_INSTANCE_BUSY     /* the busy state for direction from APP to GDD */
}T_BAT_instance_state;

typedef enum
{
  BAT_CLIENT_IDLE = 0,
  BAT_CLIENT_ACTIVATING,
  BAT_CLIENT_READY,
  BAT_CLIENT_BUSY,       /* the busy state for direction from APP to GDD */
  BAT_CLIENT_SENDING,
  BAT_CLIENT_SENDING_AND_BUSY /* cmd sending + abort busy */
}T_BAT_client_state;


typedef enum
{
  BAT_BUF_EMPTY,
  BAT_BUF_FILLING,
  BAT_BUF_FILLED
}T_BAT_buf_state;

typedef enum
{
  BAT_NEW_INSTANCE_SUCCEED = 0,
  BAT_NEW_INSTANCE_FAIL,
  BAT_OPEN_CLIENT_SUCCEED,
  BAT_OPEN_CLIENT_FAIL,
  BAT_ABORT_COMMAND_SUCCEED,
  BAT_ABORT_COMMAND_FAIL,
  BAT_READY_RESOURCE
}T_BAT_signal;

typedef struct
{
  T_GDD_BUF            *gdd_buf; /* to memorize the address of the sent GDD buffer*/
  T_GDD_BUF            *gdd_buf_rcv; /* to memorize the address of the received GDD buffer*/
  T_BAT_client         dest;
  T_BAT_buf_state      buf_st;
  T_BAT_cmd_response   rsp;
  char                 data[BAT_RCV_BUF_SIZE]; 
} T_BAT_buffer;
 
typedef struct
{
  T_BAT_client_state   client_state;
  void                 (*signal_cb)           (T_BAT_client client, 
                                               T_BAT_signal   signal);
  int                  (*response_cb)         (T_BAT_client client, 
                                               T_BAT_cmd_response *response);
}T_BAT_client_maintain;


typedef struct
{
  T_GDD_SEGMENT* desc;
  U16         length;
} T_BAT_l2p_maintain;


typedef struct 
{
  unsigned long        con_handle;
  T_BAT_instance_state instance_state;
  unsigned char        max_client_num;
  T_BAT_l2p_maintain   l2p_mt;
  T_BAT_buffer         buffer;
  T_BAT_config         *config;
  int                  sem_BAT; /* should be T_HANDLE, but we don't want to include "vsi.h" */
  void                (*instance_signal_cb)   (T_BAT_signal signal);
  int                 (*unsolicited_result_cb)(T_BAT_client client, 
                                               T_BAT_cmd_response *response);
}T_BAT_instance_maintain;

#endif /*BAT_TYPES_H*/


