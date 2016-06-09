/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_BAT
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
|  Purpose :  This header file consists of the prototype for aci_bat_ext.c
|
+----------------------------------------------------------------------------- 
*/ 
#ifndef ACI_BAT_EXT_H
#define ACI_BAT_EXT_H

#ifndef P_BAT_H
#include "p_bat.h"
#endif

#ifndef ACI_BAT_CMH_H
typedef enum
{
  ACI_BAT_FAIL = -1,
  ACI_BAT_CMPL,
  ACI_BAT_EXCT,
  ACI_BAT_BUSY
} T_ACI_BAT_RSLT;
#endif

#define BAT_BIT_13 0x2000  /* custom cmd/rsp between 0x2000 - 0x3FFF */

EXTERN T_ACI_BAT_RSLT aci_bat_send_custom_cmd (unsigned char src_id, T_BAT_cmd_send *cmd);
EXTERN T_ACI_BAT_RSLT aci_bat_rcv_custom (unsigned char src_id, 
                                          unsigned char client,
                                          unsigned int dataTag, 
                                          void *dataPtr, 
                                          unsigned short dataSize);

#endif

