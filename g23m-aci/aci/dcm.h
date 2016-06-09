/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :  DCM
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
|  Purpose :  This modul provides the types used by DCM (Data Connection Manager).
+-----------------------------------------------------------------------------
*/

#ifndef __DCM_H__
#define __DCM_H__


/*==== Includes =============================================================*/


#include "typedefs.h" /* to get general TI types */
#include "gsm.h"      /* to get Protocol Stack types */


/*
 * DCM return values
 */
typedef enum
{
  DCM_OK                = DCM_RET_OK,
  DCM_NOT_READY         = DCM_RET_NOT_READY,
  DCM_ALREADY_ACTIVATED = DCM_RET_ALREADY_ACTIVATED,
  DCM_UNKNOWN_EVENT     = DCM_RET_UNKNOWN_EVENT,
  DCM_INVALID_PARAMETER = DCM_RET_INVALID_PARAMETER,
  DCM_BUSY              = DCM_RET_CMD_PENDING,
  DCM_PS_CONN_BROKEN    = DCM_RET_PS_CONN_BROKEN,
  DCM_NO_NETWORK
}T_DCM_RET;


/*
 * bearer type
 */
typedef enum
{
   DCM_BEARER_NO,
   DCM_BEARER_ANY          = DCM_SOCK_BEARER_ANY,
   DCM_BEARER_GPRS         = DCM_SOCK_BEARER_GPRS,
   DCM_BEARER_GSM          = DCM_SOCK_BEARER_GSM,
   DCM_BEARER_USE_PROFILE  = DCM_SOCK_BEARER_USE_PROFILE,
   DCM_BEARER_AS_SPECIFIED = DCM_SOCK_BEARER_AS_SPECIFIED
}T_BEARER_TYPE;


#endif /* !__DCM_H__ */
