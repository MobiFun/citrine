/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  WAP
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
|  Purpose :  Definitions for the WAP module
+-----------------------------------------------------------------------------
*/

#ifndef WAP_ACI_H
#define WAP_ACI_H

/*==== TYPES ======================================================*/

typedef enum
{
  Wap_Not_Init = -1 ,
#ifdef FF_GPF_TCPIP
  TCPIP_Initialization,
  TCPIP_Initialized,
  TCPIP_Activation ,
  TCPIP_Activated ,
#endif /*FF_GPF_TCPIP*/
  UDPA_Activation ,
  UDPA_Activated ,
  IPA_Activation ,
  IPA_Activated ,
  IPA_Configuration ,
  IPA_Configurated ,
  UDPA_Configuration  ,
  UDPA_Configurated ,
#ifdef FF_GPF_TCPIP
  TCPIP_Configuration ,
  TCPIP_Configurated  ,
#endif /*FF_GPF_TCPIP*/
  IPA_Deconfiguration,
  IPA_Deconfigurated,
  UDPA_Deconfiguration,
  UPDA_Deconfigurated,
#ifdef FF_GPF_TCPIP
  TCPIP_Deconfiguration,
  TCPIP_Deconfigurated,
  TCPIP_Deactivation,
  TCPIP_Deactivated,
#endif /*FF_GPF_TCPIP*/
  UDPA_Deactivation,
  UDPA_Deactivated,
  IPA_Deactivation,
  IPA_Deactivated
} T_ACI_WAP_STATES;


/*
 *  Indicate is IP connected or disconnected
 */

typedef enum
{
  IPA_CONN,
  IPA_DSC
}
T_ACI_IPA_DIRC;


/*==== EXPORT =====================================================*/

/*
 *  Prototypes for IP and UDP.
 */

EXTERN void psaIPA_Config ( ULONG ip, USHORT max_trans_unit, T_ACI_IPA_DIRC dirc );
EXTERN void psaUDPA_Config ( UBYTE dirc );

EXTERN T_ACI_RETURN cmhIPA_Configurated ( void );
EXTERN T_ACI_RETURN cmhIPA_Deconfigurated ( void );

EXTERN T_ACI_RETURN cmhUDPA_Activate ( T_ACI_CMD_SRC srcId, SHORT cId );
EXTERN T_ACI_RETURN cmhUDPA_Configurated ( void );
EXTERN T_ACI_RETURN cmhUDPA_Deconfigurated ( void );
EXTERN T_ACI_RETURN cmhUDPA_Deactivate ( UBYTE src_id );

EXTERN T_ACI_RETURN cmhTCPIP_Activate ( T_ACI_CMD_SRC srcId, SHORT cId );
EXTERN T_ACI_RETURN cmhTCPIP_Configurated ( void );
EXTERN T_ACI_RETURN cmhTCPIP_Deconfigurated ( void );
EXTERN T_ACI_RETURN cmhTCPIP_Deactivate ( UBYTE src_id );

EXTERN void psaIPA_Dti_Req(ULONG dti_id, UBYTE peer_to_connect_to, UBYTE dti_conn);
EXTERN void psaUDPA_Dti_Req(ULONG dti_id, UBYTE peer_to_connect_to, UBYTE dti_conn);
EXTERN void psaWAP_Dti_Req(ULONG dti_id, UBYTE peer_to_connect_to, UBYTE dti_conn);


/*
 *  Variables for WAP, state, id and CALL
 */

#ifdef PSA_PPPP_C
GLOBAL T_ACI_WAP_STATES wap_state  = Wap_Not_Init;
GLOBAL SHORT            wapId      = NO_ENTRY;
GLOBAL BOOL             Wap_Call   = FALSE;
GLOBAL UBYTE            wap_dti_id = DTI_DTI_ID_NOTPRESENT;
GLOBAL ULONG            peer_link_id  = DTI_LINK_ID_NOTPRESENT;
GLOBAL ULONG            prot_link_id = DTI_LINK_ID_NOTPRESENT;
GLOBAL UBYTE            peer_entity_id = DTI_ENTITY_MAX;
GLOBAL UBYTE            prot_entity_id = DTI_ENTITY_MAX;

#else

EXTERN T_ACI_WAP_STATES wap_state;
EXTERN SHORT            wapId;
EXTERN BOOL             Wap_Call;
EXTERN UBYTE            wap_dti_id;
EXTERN ULONG            peer_link_id;
EXTERN ULONG            prot_link_id;
EXTERN UBYTE            peer_entity_id;
EXTERN UBYTE            prot_entity_id;

#endif /* of #ifdef CMH_UDPAS_C */


#endif /* of #ifndef WAP_ACI_H */
