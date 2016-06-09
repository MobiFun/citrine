/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI
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
|  Purpose :  This Modul holds the functions
|             for the binary AT command library at APPlication side
+-----------------------------------------------------------------------------
*/
#ifndef BAT_CTRL_H
#define BAT_CTRL_H



/*********************************************************************************
 *
 * enums
 *
 *********************************************************************************/

typedef enum
{
  BATC_MAX_CLIENTS = 0x8000,/*bit 15 is 1 */
  BATC_OPEN_CLIENT,
  BATC_CLOSE_CLIENT,
  BATC_ABORT_CMD
} T_BATC_ctrl_params;


typedef enum
{
  BATC_MAX_CLIENTS_CNF = 0x8000, /* bit 15 is 1 */
  BATC_MAX_CLIENTS_REJ,
  BATC_OPEN_CLIENT_CNF,
  BATC_OPEN_CLIENT_REJ,
  BATC_ABORT_COMMAND_CNF,
  BATC_ABORT_COMMAND_REJ
} T_BATC_rsp_param;


/*********************************************************************************
 *
 * structures
 *
 *********************************************************************************/

typedef struct
{
  U32  batc_dummy;
} T_BATC_no_parameter;  /* actually never used */

typedef struct
{
  U32  num_clients;
} T_BATC_max_clients;   /* max number of clients of an instance */

typedef struct
{
  U32  client_id;
} T_BATC_open_client;   /* client to open */

typedef struct
{
  U32  client_id;
} T_BATC_close_client;   /* client to close */

typedef struct
{
  U32  client_id;
} T_BATC_abort_cmd;   /* client which runs the cmd */

typedef union
{
  T_BATC_max_clients    *ptr_max_clients;
  T_BATC_open_client    *ptr_open_client;
  T_BATC_close_client   *ptr_close_client;
  T_BATC_abort_cmd      *ptr_abort_cmd;
} T_BATC_params;

typedef union
{
  T_BATC_open_client   *ptr_bat_open_client_cnf; /* confirmation from BAT to BAT Lib */
  T_BATC_abort_cmd     *ptr_bat_abort_command_cnf; /* confirmation from BAT to BAT Lib */
} T_BATC_rsp;

typedef struct
{
  T_BATC_ctrl_params   ctrl_params;
  T_BATC_params        params;
}T_BATC_signal;

typedef struct
{
  T_BATC_rsp_param     rsp_params;              
  T_BATC_rsp           rsp;                   
}T_BATC_confirm;



#endif  /* BAT_CTRL_H */



