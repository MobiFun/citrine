/*
+------------------------------------------------------------------------------
|  File:       cmh_dti.h
+------------------------------------------------------------------------------
|                 Copyright Condat AG 1999-2000, Berlin
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Condat AG.
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Condat AG.
+------------------------------------------------------------------------------
| Purpose:     Definitions for the DTI managment. 
|              
| $Identity:$
+------------------------------------------------------------------------------
*/


#ifndef CMH_DTI_H
#define CMH_DTI_H



/*==== ENUMS =====================================================*/

typedef enum              /* DTI redirection mode */
{
  REDIR_DELETE = 0,
  REDIR_ONCE,
  REDIR_ALWAYS    
} T_ACI_REDIR_MODE;

typedef enum              /* DINF mode */
{
  SHOW_CURR_CHAN_INF = 0,
  SHOW_ALL_CHAN_INF
} T_ACI_DINF_MODE;

typedef enum               /* to map*/
{
  DEV_CPBLTY_CMD = 0,      /* AT cmd capability                    */
  DEV_CPBLTY_SER,          /* serial capability                    */
  DEV_CPBLTY_PKT,          /* packet capability                    */
  DEV_CPBLTY_CMD_SER,      /* AT cmd and serial capability         */
  DEV_CPBLTY_CMD_PKT,      /* AT cmd and packet capability         */
  DEV_CPBLTY_PKT_SER,      /* packet and serial capability         */
  DEV_CPBLTY_CMD_PKT_SER,  /* AT cmd, packet and serial capability */
  DEV_CPBLTY_NONE          /* sentinel for %DINF command           */
} T_CAP_ID;

/*==== STRUCTS =====================================================*/

typedef struct
{
  CHAR     *name;
  T_CAP_ID  id;
} T_CAP_NAME;

/*==== VARIABLE====================================================*/


/*==== EXPORT =====================================================*/


#endif /* CMH_DTI_H */
                 
/*==== EOF =======================================================*/
