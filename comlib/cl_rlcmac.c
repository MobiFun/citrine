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
|  Purpose :  Definitions of common library functions: RLC/MAC layer.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CL_RLCMAC_C
#define CL_RLCMAC_C
#endif /* #ifndef CL_RLCMAC_C */

#include "config.h"
#include "fixedconf.h"

#define ENTITY_GRLC
#define ENTITY_GRR

/*==== INCLUDES =============================================================*/

#include "typedefs.h"
#include "vsi.h"
#include "macdef.h"
#include "gprs.h"
#include "gsm.h"
#include "ccdapi.h"
#include "prim.h"
#include "message.h"
#include "cl_rlcmac.h"

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/
/*
+------------------------------------------------------------------------------
| Function    : cl_rlcmac_get_msg_name
+------------------------------------------------------------------------------
| Description : 
|
| Parameters  : 
|
+------------------------------------------------------------------------------
*/

#if !defined (NTRACE)


#define RETURN(msg)  return( #msg );

GLOBAL char* cl_rlcmac_get_msg_name ( UBYTE           msg_type,
                                      T_RLC_MAC_ROUTE route )
{
  if( route EQ RLC_MAC_ROUTE_UL )
  {
    switch( msg_type )
    {
      case U_RESOURCE_REQ_c:         RETURN(  U_RESOURCE_REQ          ); /*lint !e527*/
      case U_DL_ACK_c:               RETURN(  U_DL_ACK                ); /*lint !e527 !e825*/
      case U_CTRL_ACK_c:             RETURN(  U_CTRL_ACK              ); /*lint !e527 !e825*/
      case U_CELL_CHAN_FAILURE_c:    RETURN(  U_CELL_CHAN_FAILURE     ); /*lint !e527 !e825*/
      case U_UL_DUMMY_c:             RETURN(  U_UL_DUMMY              ); /*lint !e527 !e825*/
      case U_MEAS_REPORT_c:          RETURN(  U_MEAS_REPORT           ); /*lint !e527 !e825*/
      case U_MS_TBF_STATUS_c:        RETURN(  U_MS_TBF_STATUS         ); /*lint !e527 !e825*/
      case U_PKT_PSI_STATUS_MSG_c:   RETURN(  U_PKT_PSI_STATUS_MSG    ); /*lint !e527 !e825*/
      case U_MSG_TYPE_CHANNEL_REQ_c: return( "U_MSG_TYPE_CHANNEL_REQ" ); /*lint !e527 !e825*/
      case U_MSG_TYPE_UNKNOWN_c:                                         /*lint !e527 !e825*/
      default:                       return( "U_MSG_TYPE_UNKNOWN"     ); /*lint !e527 !e825*/
    }
  }
  else
  {
    switch( msg_type )
    {
      case D_ACCESS_REJ_c:           RETURN(  D_ACCESS_REJ            ); /*lint !e527*/
      case D_QUEUING_NOT_c:          RETURN(  D_QUEUING_NOT           ); /*lint !e527 !e825*/
      case D_UL_ASSIGN_c:            RETURN(  D_UL_ASSIGN             ); /*lint !e527 !e825*/
      case D_DL_ASSIGN_c:            RETURN(  D_DL_ASSIGN             ); /*lint !e527 !e825*/
      case D_TBF_RELEASE_c:          RETURN(  D_TBF_RELEASE           ); /*lint !e527 !e825*/
      case D_PAGING_REQ_c:           RETURN(  D_PAGING_REQ            ); /*lint !e527 !e825*/
      case D_UL_ACK_c:               RETURN(  D_UL_ACK                ); /*lint !e527 !e825*/
      case PSI_1_c:                  RETURN(  PSI_1                   ); /*lint !e527 !e825*/
      case PSI_2_c:                  RETURN(  PSI_2                   ); /*lint !e527 !e825*/
      case PSI_3_c:                  RETURN(  PSI_3                   ); /*lint !e527 !e825*/
      case PSI_3_BIS_c:              RETURN(  PSI_3_BIS               ); /*lint !e527 !e825*/
      case PSI_4_c:                  RETURN(  PSI_4                   ); /*lint !e527 !e825*/
      case PSI_5_c:                  RETURN(  PSI_5                   ); /*lint !e527 !e825*/
      case PSI_13_c:                 RETURN(  PSI_13                  ); /*lint !e527 !e825*/
      case D_CELL_CHAN_ORDER_c:      RETURN(  D_CELL_CHAN_ORDER       ); /*lint !e527 !e825*/
      case D_DL_DUMMY_c:             RETURN(  D_DL_DUMMY              ); /*lint !e527 !e825*/
      case D_MEAS_ORDER_c:           RETURN(  D_MEAS_ORDER            ); /*lint !e527 !e825*/
      case D_PDCH_RELEASE_c:         RETURN(  D_PDCH_RELEASE          ); /*lint !e527 !e825*/
      case D_POLLING_REQ_c:          RETURN(  D_POLLING_REQ           ); /*lint !e527 !e825*/
      case D_CTRL_PWR_TA_c:          RETURN(  D_CTRL_PWR_TA           ); /*lint !e527 !e825*/
      case D_PRACH_PAR_c:            RETURN(  D_PRACH_PAR             ); /*lint !e527 !e825*/
      case D_TS_RECONFIG_c:          RETURN(  D_TS_RECONFIG           ); /*lint !e527 !e825*/
      case D_MSG_TYPE_CRC_ERROR_c:   return( "D_MSG_TYPE_CRC_ERROR"   ); /*lint !e527 !e825*/
      case D_MSG_TYPE_2ND_SEGMENT_c: return( "D_MSG_TYPE_2ND_SEGMENT" ); /*lint !e527 !e825*/
      case D_MSG_TYPE_UNKNOWN_c:                                         /*lint !e527 !e825*/
      default:                       return( "D_MSG_TYPE_UNKNOWN"     ); /*lint !e527 !e825*/
    }
  }
} /* cl_rlcmac_get_msg_name */

#endif /* #if !defined (NTRACE) */
