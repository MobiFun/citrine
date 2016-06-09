/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  
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
|             Point-to-Point Protocol ( PPP ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) AND defined (DTI)

#ifndef PSA_GPPP_H
#define PSA_GPPP_H



/*==== CONSTANTS ==================================================*/
/* length is out of the SAP from PPP */
#define CHANNEL_NAME_LENGTH           6


/*==== TYPES ======================================================*/
typedef struct pppSetParm
{
  T_PPP_PDP_ACTIVATE_RES  *pdp_res; /* parameter for PDP context activation */
  T_PPP_PDP_ACTIVATE_REJ  *pdp_rej; /* parameter for PDP context activation failed */
  UBYTE   ppp_hc;                   /* 0, 1: header compression of PPP connection */
  UBYTE   msid;                     /* ... : max slot identifier */

} T_PPP_SET_PRM;

typedef struct pppShrdParm
{
  /* set parameter */
  UBYTE  owner;                     /* identifies the used set */
  T_PPP_SET_PRM setPrm[OWN_SRC_MAX];    /* possible sets */

  /* global set parameter */
  UBYTE   ppp_authentication_protocol;  /* PPP authentication protocol */
  UBYTE   accm;                         /* default:  0 */
  UBYTE   restart_timer;                /* default:  3 */
  UBYTE   max_configure;                /* default: 10 */
  UBYTE   max_terminate;                /* default:  2 */
  UBYTE   max_failure;                  /* default:  5 */

  /* answer parameter */
  T_PPP_ESTABLISH_CNF      est;     /* parameter from establishment */
  T_PPP_PDP_ACTIVATE_IND  *pdp;     /* parameter from PDP context */
  U16     ppp_cause;                /* 1 ... 111 cause of PPP termination */
  UBYTE   instance;                 /* future parameter for the PPP instance */
  UBYTE   tui;
  UBYTE   ppp_hc;                   /* 0, 1: header compression of PPP connection */
  UBYTE   msid;                     /* ... : max slot identifier */

} T_GPPP_SHRD_PRM;

/*==== PROTOTYPES =================================================*/

EXTERN void  psaGPPP_Establish (  T_PPP_ESTABLISH_REQ *est_req );
EXTERN void  psaGPPP_PDP_Activate ( T_NAS_ip * ip, UBYTE *pco_buf, UBYTE pco_length, U8 nsapi);

EXTERN SHORT psaGPPP_PDP_Reject   ( void );
//needed ?? EXTERN SHORT psaGPPP_Modification ( void );

EXTERN void psaGPPP_Init ( UBYTE accm, UBYTE restart_timer, 
                           UBYTE max_configure, UBYTE max_terminate, UBYTE max_failure );


EXTERN void  psaGPPP_Terminate  ( UBYTE lower_layer );

EXTERN void psaGPPPS_Dti_Req         ( T_DTI_CONN_LINK_ID link_id, UBYTE peer );

EXTERN BOOL PPP_UART_connect_dti_cb  ( UBYTE dti_id, T_DTI_CONN_STATE result_type);


/*==== EXPORT =====================================================*/

#ifdef PSA_GPPPF_C

GLOBAL T_GPPP_SHRD_PRM gpppShrdPrm;

#else

EXTERN T_GPPP_SHRD_PRM gpppShrdPrm;

#endif /* PSA_GPPPF_C */

#endif /* PSA_PPP_H */

#endif  /* GPRS */
/*==== EOF =======================================================*/
