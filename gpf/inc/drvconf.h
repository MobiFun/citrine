/* 
+------------------------------------------------------------------------------
|  File:       drvconf.h
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
|  Purpose :  Definitions for the dynamic driver configuration.
+----------------------------------------------------------------------------- 
*/ 

#ifndef DRVCONF_H
#define DRVCONF_H

/*==== INCLUDES =============================================================*/
#include "typedefs.h"
#include "gpfconf.h"
#include "gdi.h"
#include "vsi.h"

/*==== CONSTS ===============================================================*/

#if defined _TARGET_
 #define MAX_AVAILABLE_DRV 1
#else
 #define MAX_AVAILABLE_DRV 6
#endif

#define TIF_NAME              "TIF"
#define TR_NAME               "TR"
#define EMI_NAME              "EMI"
#define SOCKET_NAME           "SOCKET"
#define MTST_NAME             "MTST"
#define SER_NAME              "SER"
#define THIF_LINK_NAME        "THIFLINK"
#define USERSPACE_LINK_NAME   "USSPLINK"
#define TITRC_NAME            "TITRC"
#define NODRV_NAME            "NODRV"

#define DRV_DEFAULT       "DEFAULT"
#define DRV_TI_MODE       "TI_MODE"
#define DRV_RAW_TI_MODE   "RAW_TI_MODE"
#define TIF_PCON_ENABLE   "ENABLE_PCON"
#define TR_STX_LF         "STX_LF"
#define ENABLE_SYNC_MODE  "ENA_SYNC_MODE"
#define DISABLE_SYNC_MODE "DIS_SYNC_MODE"

#define FILTER            "FILTER"

#define DRV_SIGTYPE_READ_L1   0x100
#define DRV_SIGTYPE_READ_RIV  0x200

/*==== TYPES ================================================================*/

typedef struct
{
  T_DRV_LIST_ENTRY entry;  
  USHORT drv_pos;
} T_TST_DRV_ENTRY;

/*==== EXPORTS ==============================================================*/

SHORT tst_drv_open (char *drv_name, T_TST_DRV_ENTRY **drv_info );


#endif /* DRVCONF_H */
