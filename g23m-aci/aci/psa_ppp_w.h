/* 
+----------------------------------------------------------------------------- 
|  Project :  WAP
|  Modul   :  PSA_PPP
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
|  Purpose :  Definitions for PPP
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_PPP_H
#define PSA_PPP_H

/*==== CONSTANTS ==================================================*/

/* 
 *   25 is the maximal size a login 
 *   and a password should have
 */

/*   Value of MAX_PPP_LOGIN_LEN is changed from 25 to 33
 *   Issue: OMAPS00067767
 */

#define MAX_PPP_LOGIN_LEN (33)   
#define MAX_PPP_PASSW_LEN (25)

/*
 *  States for PPP
 */

typedef enum
{
  PPP_UNDEFINED = -1,
  PPP_ESTABLISH = 0,	/* PPP is in establish state to server */
  PPP_ESTABLISHED,    /* PPP client is established with server */
  PPP_TERMINATE,			/* PPP start termination */
  PPP_TERMINATED			/* PPP is terminated */
} T_PPP_STATE;

/*
 *  Indicate is lower layer activ
 */

typedef enum
{
	UNDEFINED = -1,     /* Not definied */
    UP,					/* Lower layer, is activ. (Example: L2R, RLP, CC).*/
    DWN					/* Loer layer is down, (Example: CC has disconnected */
}
T_ACI_PPP_LOWER_LAYER;


/*==== TYPES ======================================================*/

typedef struct PPPShrdParm
{
  UBYTE  owner;							 /* owner */
  SHORT  cId;							 /* related call id */
  T_PPP_STATE  state;					 /* state */
  CHAR	 ppp_login[MAX_PPP_LOGIN_LEN];   /* login */
  CHAR	 ppp_password[MAX_PPP_PASSW_LEN];/* password */ 
  BOOL   is_PPP_CALL;     /* should "ATD" connect AAA-PPP-L2R */ 
  T_ACI_PPP_PROT auth_prot;
#if defined(FF_PPP) || defined(FF_WAP) || defined(FF_GPF_TCPIP) || defined (FF_SAT_E)
  /* PPP connection parameters supplied by the network.  */
  ULONG  ipaddr ;               /* Our own IP address.  */
  ULONG  dns1 ;                 /* First DNS server address. */
  ULONG  dns2 ;                 /* Second DNS server address. */
#endif /* FF_PPP || FF_WAP || FF_GPF_TCPIP || FF_SAT_E*/
} T_PPP_SHRD_PRM;

/*==== PROTOTYPES =================================================*/

void  psaPPP_Init();			                                         /* Initsialisation of PPP psa variables */

#ifdef DTI
void	psaPPP_Establish ( T_DTI_CONN_LINK_ID link_id, UBYTE peer ); /* Handle the establishment from ACI-PPP */
#endif /* DTI */
void  psaPPP_Terminate ( T_ACI_PPP_LOWER_LAYER ppp_lower_layer);   /* Handle the hang up */
			                 

/*==== EXPORT =====================================================*/

#ifdef PSA_PPPF_C

GLOBAL T_PPP_SHRD_PRM pppShrdPrm;

#else

EXTERN T_PPP_SHRD_PRM pppShrdPrm;

#endif /* PSA_PPPF_C */

#endif /* PSA_PPP_H */

/*==== EOF =======================================================*/
