/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ATI
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
|  Purpose :  contains prototypes of functions or declaration of variables
|             which are INTERNAL to ATI.
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_INT_H
#define ATI_INT_H

//#undef TRACE_FUNCTION
//#define TRACE_FUNCTION(a)

EXTERN CHAR  *cmdCmsError ( T_ACI_CMS_ERR e );
EXTERN CHAR  *cmdCmeError ( T_ACI_CME_ERR err );
EXTERN CHAR  *cmdAtError (AtErrCode e);

EXTERN CHAR   *resp_disp(UBYTE srcId, CHAR *cl,CHAR *type, ...);
EXTERN CHAR   *strupper (CHAR *s);
EXTERN USHORT compact   (CHAR *s, USHORT len);
EXTERN void   ati_get_cmds_key(T_ACI_AT_CMD cmd_id, CHAR **cmd_key, CHAR **testcmd_output);
EXTERN void   ati_stop_ring(void);


EXTERN CHAR          *cmdErrStr;          /* Error Message            */
EXTERN T_ACI_AT_CMD  curAbrtCmd;
EXTERN char          g_sa[MAX_CMD_LEN];
EXTERN UBYTE         srcId_cb;
EXTERN T_ACI_LIST    *ati_src_list;
#endif
