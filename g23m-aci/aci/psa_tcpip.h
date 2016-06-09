/*
+-----------------------------------------------------------------------------
|  Project :  WAPoverGPRS
|  Modul   :  PSA_TPCIP
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
|  Purpose :  TCP/IP-related definitions and declarations for the ACI
+-----------------------------------------------------------------------------
*/

#ifndef PSA_TCPIP_H
#define PSA_TCPIP_H

#include "wap_aci.h"

/*==== CONSTANTS ==================================================*/

/* Option bitflag constants for the activation of TCP/IP components;
 * this is meant to be sufficient even for times when there is no
 * longer only IPv4 and UDP. These constants are meant to be ORed to
 * the actual option value, which is then passed to
 * psaTCPIP_Activate().
 */
#define TCPIP_ACT_OPTION_V4    0x01  /* Activate IPv4. */
#define TCPIP_ACT_OPTION_V6    0x02  /* Activate IPv6. (Not yet implemented) */
#define TCPIP_ACT_OPTION_UDP   0x04  /* Activate UDP. */
#define TCPIP_ACT_OPTION_TCPIP 0x08  /* Activate TCPIP. */
#define TCPIP_ACT_OPTION_FU1   0x10  /* Reserved for future use. */
#define TCPIP_ACT_OPTION_FU2   0x20  /* Reserved for future use. */
#define TCPIP_ACT_OPTION_FU3   0x40  /* Reserved for future use. */
#define TCPIP_ACT_OPTION_FU4   0x80  /* Reserved for future use. */


#define TCPIP_CONNECTION_TYPE_UNKNOWN  0x00
#define TCPIP_CONNECTION_TYPE_GPRS_WAP 0x01
#define TCPIP_CONNECTION_TYPE_CSD_WAP  0x02
#define TCPIP_CONNECTION_TYPE_GPRS_SAT 0x04 /* SAT class c/e */
#define TCPIP_CONNECTION_TYPE_CSD_SAT  0x08 /* SAT class c/e */

#define TCPIP_CONNECTION_BUILDUP_UNKNOWN 0x00
#define TCPIP_CONNECTION_BUILDUP_UP      0x01
#define TCPIP_CONNECTION_BUILDUP_DOWN    0x02


/*==== TYPES ======================================================*/

/* Shared Parameters for TCP/IP-related stuff. There should be more in
 * here, for instance the wap_state, but to make changes as small as
 * possible at the moment, this is left as it is for now. The handling
 * of TCP/IP activation state will have to be changed in the future
 * anyway. [ni 2001-09-28]
 */
typedef struct tcpipShrdPrm
{
  S8    src_id ;
  UBYTE connection_type;        /* distinction of GPRS or CSD connection */
  UBYTE connection_buildup;     /* distinction of connection build up or
                                   build down */
  UBYTE options ;               /* Any ORed combination of the
                                 * IP_ACT_OPTION_* values above with
                                 * obvious semantics. */
  SHORT wap_call_id ;           /* ID of WAP call (if any). */
  UBYTE ipaddr[16] ;            /* Up to 16 bytes if IP address. */
  UBYTE peer_addr[16] ;         /* Address of peer. */
  UBYTE dns1[16];               /* Primary dns server address */
  UBYTE dns2[16];               /* Second dns server address */
  short mtu ;                   /* Size of maximum transfer unit. */
  void (*callback_function)(T_ACI_RETURN result) ;
                                /* Function to call after the current
                                 * operation is completed. */
} T_TCPIP_SHRD_PRM;

/*==== PROTOTYPES =================================================*/
EXTERN char* wap_state_to_string(T_ACI_WAP_STATES wap_state);

/* Initialize TCP/IP psa variables.
 */
void psaTCPIP_Init(void) ;

/* Activate TCP/IP-related entities.
 * Parameters:
 *   src_id
 *   dti_id             DTI channel to use.
 *   options            Any ORed combination of the IP_ACT_OPTION_* values
 *                      above with obvious semantics.
 *   callback_function  Function to call after activation is completed;
 *                      result can be AT_FAIL or AT_CMPL. May be null.
 */
void psaTCPIP_Activate(UBYTE src_id,
                       UBYTE dti_id,
                       SHORT wap_call_id,
                       UBYTE options,
                       UBYTE connection_type,
                       void (*callback_function)(T_ACI_RETURN result)) ;

/* Configure TCP/IP-related entities.
 * Parameters:
 *   ip_address         IP address to use when called for a circuit-switched
 *                      data (i. e. PPP) connection. A pointer to an
 *                      array of bytes containing the IP address in
 *                      network order. The address is assumed as being
 *                      4 or 16 bytes long, depending on whether IPv6
 *                      is used. The ``pdp_address'' value returned by
 *                      SMREG_PDP_ACTIVATE_CNF can be used here
 *                      directly. Must be null when called for IP over
 *                      GPRS.
 *   pdp_addrp          Pointer to PDP address to use when called for IP
 *                      over GPRS. Must be null when called for IP
 *                      over CSD/PPP.
 *   peer_address       IP address of the peer, or a null pointer to indicate
 *                      the peer address is unknown. Only valid with IP over PPP.
 *   mtu                Size of the maximum transfer unit or zero if unknown.
 *   callback_function  Function to call after configuration is completed;
 *                      result can be AT_FAIL or AT_CMPL. May be null.
 */
void psaTCPIP_Configure(UBYTE *ip_address,
                        void  *pdp_addrp,
                        UBYTE *peer_address,
                        UBYTE *dns1,
                        UBYTE *dns2,
                        short mtu,
                        void (*callback_function)(T_ACI_RETURN result)) ;

/* Deactivate TCP/IP-related entities.
 * Parameters:
 *   callback_function  Function to call after deactivation is completed;
 *                      result can be AT_FAIL or AT_CMPL. May be null.
 */
void psaTCPIP_Deactivate(void (*callback_function)(T_ACI_RETURN result)) ;


/* UDP/IP activation/configuration/deactivation dipatcher function. */
void psaUDPIP_config_dispatch(void);

/* TCP/IP activation/configuration/deactivation dipatcher function.
 */
void psaTCPIP_config_dispatch(void);


/* TCP/IP activation callback for circuit-switched data.
 */
void psaTCPIP_act_csd_callback(T_ACI_RETURN result) ;


/* TCP/IP configuration callback for circuit-switched data.
 */
void psaTCPIP_conf_csd_callback(T_ACI_RETURN result) ;


/* TCP/IP deactivation callback for circuit-switched data.
 */
void psaTCPIP_deact_csd_callback(T_ACI_RETURN result) ;


ULONG psaTCPIP_bytes2ipv4addr(UBYTE *ip_address) ;

void psaTCPIP_get_own_ipaddr(UBYTE *ipaddr);

/*==== EXPORT =====================================================*/

#ifdef PSA_TCPIPF_C

GLOBAL T_TCPIP_SHRD_PRM tcpipShrdPrm;

#else

EXTERN T_TCPIP_SHRD_PRM tcpipShrdPrm;

#endif /* PSA_TCPIPF_C */

#endif /* PSA_TCPIP_H */

/*==== EOF =======================================================*/
