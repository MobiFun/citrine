/*  
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  BAT library
+-----------------------------------------------------------------------------
|  Copyright 2005 Texas Instruments Berlin, AG
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
|  Purpose :  API for binary AT commands
+-----------------------------------------------------------------------------
*/

#ifndef BAT_H 
#define BAT_H

#ifndef __TYPEDEFS_H__
#include "typedefs.h"     /* for customers without busybe */
#endif

#include "p_bat.h"        /* to include the p_bat.h */
#include "bat_types.h"    /* to include the types for internal maintainance */
#include "bat_cfg.h"      /* to include the configuration settings */


/*********************************************************************************
 *
 * defines
 *
 *********************************************************************************/

#define BAT_INSTANCE_HEADER_SIZE ((unsigned short)sizeof(void*))
#define BAT_INSTANCE_SIZE ((unsigned short)sizeof(T_BAT_instance_maintain) + (unsigned short)L2P_MAINTAIN_SIZE)                
#define BAT_CLIENT_SIZE ((unsigned short)sizeof(T_BAT_client_maintain))


/*********************************************************************************
 *
 * enumerations
 *
 *********************************************************************************/

typedef enum
{
  BAT_OK = 0,
  BAT_BUSY_RESOURCE,
  BAT_ERROR
}T_BAT_return;

typedef enum
{
  BAT_ABORT = 0,
  BAT_APP_READY_RESOURCE
}T_BAT_event;


/*********************************************************************************
 *
 * structures
 *
 *********************************************************************************/

typedef struct
{
  T_BAT_event          event; 
} T_BAT_ctrl;


/*********************************************************************************
 *
 * function declarations
 *
 *********************************************************************************/

extern T_BAT_return bat_init   (void            *mem, 
                                unsigned char   num);
extern T_BAT_return bat_deinit (void);
extern T_BAT_return bat_new    (T_BAT_instance  *instance,
                                void            *mem,
                                unsigned char   num,
                                T_BAT_config    *config,
                                void            (*instance_signal_cb)(T_BAT_signal signal));
extern T_BAT_return bat_delete (T_BAT_instance  instance);
extern T_BAT_return bat_open   (T_BAT_instance  instance, 
                                T_BAT_client    *client,
                                int             (*response_cb)(T_BAT_client client, 
                                                               T_BAT_cmd_response *response),
                                void            (*signal_cb)  (T_BAT_client client, 
                                                               T_BAT_signal signal));
extern T_BAT_return bat_uns_open(T_BAT_instance instance, 
                                 T_BAT_client   *client,  
                                 int   (*unsolicited_result_cb)( T_BAT_client client, 
                                                                 T_BAT_cmd_response *response));
extern T_BAT_return bat_close  (T_BAT_client    client);
extern T_BAT_return bat_send   (T_BAT_client    client, 
                                T_BAT_cmd_send  *cmd);
extern T_BAT_return bat_ctrl   (T_BAT_client    client, 
                                T_BAT_ctrl      *ctrl);


#endif /*BAT_H*/








