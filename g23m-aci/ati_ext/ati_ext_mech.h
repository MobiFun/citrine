/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci_ext\aci_ext_pers.h
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
|  Purpose : This is the header file of the AT command extension mechanism.
+----------------------------------------------------------------------------- 
*/ 
#ifndef ATI_EXT_MECH_H
#define ATI_EXT_MECH_H

/* ACI extension definitions */

/* Globals ------------------------------------------------------------------------------ */

typedef enum
{
  ATI_EXT_FAIL	= -1,		/* execution failed, error occurred 			*/
  ATI_EXT_CMPL,					/* execution completed				*/
  ATI_EXT_EXCT,					/* execution is in progress				*/
  ATI_EXT_BUSY					/* execution is rejected due to a busy extension mechanism*/
} T_ATI_EXT_RETURN;


/* prototypes of functions.-------------------------------------- */


EXTERN char *parse (char *b, char *f, ...);
EXTERN void ext_OK_v2 ( T_ACI_AT_CMD cmdId );
EXTERN T_ATI_EXT_RETURN rEXT_Init ();
EXTERN T_ATI_EXT_RETURN rEXT_Execute	(UBYTE src_id, CHAR *cmd);
EXTERN T_ATI_EXT_RETURN rEXT_Abort (UBYTE src_id);
EXTERN T_ATI_EXT_RETURN rEXT_Signal (T_ACI_EXT_IND *aci_ext_ind);

#ifdef FF_BAT
#include "p_bat.h"
EXTERN T_ATI_EXT_RETURN rEXT_Response_BAT (UBYTE src_id, T_BAT_cmd_response *resp);
#endif


#endif /* ATI_EXT_MECH_H */
