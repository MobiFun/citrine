/* 
+----------------------------------------------------------------------------- 
|  Project :  COMLIB
|  Modul   :  RLCMAC
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
|  Purpose :  Definitions of global types used by common library functions
|             and the prototypes of those functions: RLC/MAC layer.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CL_RLCMAC_H
#define CL_RLCMAC_H

/*==== CONST ================================================================*/
/*
 * Start defintions from entity RR
 */

/*
 ******************************************************************************
 * The following declarations shall be identical with the corresponding 
 * declarations located in RR. 
 ******************************************************************************
 */

EXTERN UBYTE rr_csf_get_radio_access_capability( T_ra_cap *ra_cap      );
EXTERN void  RX_SetRxQual                      ( UBYTE     new_rx_qual );

/*
 ******************************************************************************
 * The above declarations shall be identical with the corresponding 
 * declarations located in RR. 
 ******************************************************************************
 */

/*
 * End defintions from entity RR
 */

/*
 * Used for tracing purposes, see also function cl_rlcmac_get_msg_name
 */

#define RLC_MAC_MAX_LEN_CHANNEL_REQ  2

#define D_MSG_TYPE_CRC_ERROR_c       0xFD
#define D_MSG_TYPE_2ND_SEGMENT_c     0xFE
#define D_MSG_TYPE_UNKNOWN_c         0xFF

#define U_MSG_TYPE_CHANNEL_REQ_c     0xFE
#define U_MSG_TYPE_UNKNOWN_c         0xFF

typedef enum /* T_RLC_MAC_ROUTE */
{
  RLC_MAC_ROUTE_UL,
  RLC_MAC_ROUTE_DL
} T_RLC_MAC_ROUTE;

/*==== VARS =================================================================*/

/*==== TYPES =================================================================*/

/*==== FUNCTIONS ============================================================*/

#if !defined (NTRACE)

EXTERN char*  cl_rlcmac_get_msg_name   ( UBYTE           msg_type,
                                         T_RLC_MAC_ROUTE route                );

#endif /* #if !defined (NTRACE) */

#endif /* #ifndef CL_RLCMAC_H */

