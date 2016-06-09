/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_L2R
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
|  Purpose :  Definitions for the protocol stack adapter
|             Layer 2 Relay ( L2R )
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_L2R_H
#define PSA_L2R_H

/*==== CONSTANTS ==================================================*/

typedef enum
{
  L2R_DEACTIVATED = 0,
  L2R_ACTIVATE,
  L2R_CONNECT,
  L2R_CONNECTED,
  L2R_ENABLE,
  L2R_ENABLED,
//  L2R_DISABLE,
  L2R_DETACH,
  L2R_DISCONNECT,
  L2R_DEACTIVATE,
  L2R_ESCAPE
} T_L2R_STATE;


/*==== TYPES ======================================================*/

typedef struct
{
  UBYTE  rlp_vers;                /* L2R_XID_IND */
  USHORT k_ms_iwf;                /* L2R_ACTIVATE_REQ, L2R_XID_IND */
  USHORT k_iwf_ms;
  UBYTE  t1;
  UBYTE  t2;
  UBYTE  n2;
  UBYTE  pt;
  UBYTE  p0;
  USHORT p1;
  UBYTE  p2;
  UBYTE  uil2p;
  UBYTE  rate;
  USHORT err_cause;               /* L2R_ERROR_IND */ 
  ULONG  error_rate;              /* L2R_STATISTIC_IND */
  USHORT reset;                   /* L2R_RESET_IND */
} T_L2R_SET_PRM;

typedef struct L2RShrdParm
{
  UBYTE         owner;
  UBYTE         state;
  UBYTE         uart_conn;
  T_L2R_SET_PRM set_prm[OWN_SRC_MAX];
  T_L2R_SET_PRM set_prm_use;
} T_L2R_SHRD_PRM;

typedef struct L2RTstPrmRef
{
  const char * key;               /* keyword string */
  UBYTE  id;                      /* corresponding id */
} L2R_TSTPRM_REF;

/*==== PROTOTYPES =================================================*/

#ifdef DTI
GLOBAL SHORT psaL2R_Enable( T_DTI_CONN_LINK_ID  link_id, UBYTE peer );
#endif /* DTI */

GLOBAL SHORT psaL2R_Activate( UBYTE srcId );
GLOBAL SHORT psaL2R_Deactivate( void );
void  psaL2R_Init (void);

GLOBAL SHORT psaL2R_ESC( UBYTE src_id );

/*==== EXPORT =====================================================*/

#ifdef PSA_L2RF_C

GLOBAL T_L2R_SHRD_PRM l2rShrdPrm;

#else

EXTERN T_L2R_SHRD_PRM l2rShrdPrm;

#endif /* PSA_L2RF_C */


#endif /* PSA_L2R_H */

/*==== EOF =======================================================*/

