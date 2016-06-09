/* 
+------------------------------------------------------------------------------
|  File:       tstdriver.h
+------------------------------------------------------------------------------
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
|  Purpose :  Prototypes for direct driver access.
+----------------------------------------------------------------------------- 
*/ 

#ifndef TSTDRIVER_H
#define TSTDRIVER_H

/*==== INCLUDES =============================================================*/

#include "typedefs.h"
#include "vsi.h"

/*==== CONSTS ===============================================================*/

#define TST_DRV_CONNECTED    0x01
#define TST_DRV_DISCONNECTED 0x02

/*==== TYPES ================================================================*/


/*==== EXPORTS ==============================================================*/

SHORT tst_drv_write ( T_HANDLE caller, ULONG opc, char *dest, char *Buffer );


#endif /* TSTDRIVER_H */
