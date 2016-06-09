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
|  Purpose :  Definitions for GPRS command handler and protocol stack adapter.
+----------------------------------------------------------------------------- 
*/ 

#include "dti.h"
#ifdef GPRS
#include "gaci_cmh.h"
#endif
#ifndef GACI_H
#define GACI_H



/*** T_CONTEXT_STATE changed to T_PDP_CONTEXT_STATE and moved to gaci_cmh.h ***/

#define NSAPI_TO_CID(A) ( (U8)((A)-4) )
#define CID_TO_NSAPI(A) ( (U16)((A)+4) )


typedef enum
{
  AS_INVALID_STATE = 0,       /* invalid value */
  AS_MOBILE_OFF,              /* the protocol stack is off before +CFUN=1 */
  AS_DETACHED,                /* GPRS detached */
  AS_ATTACHED,                /* GPRS attached */
  AS_ATTACHING,               /* attaching GPRS */
  AS_DETACHING,               /* detaching GPRS */
  AS_SUSPENTED                /* GPRS suspended */

} T_ATTACH_STATE;

typedef enum
{
  GC_TYPE_NONE      = -1,     /* no connection */
  GC_TYPE_DATA_LINK = 0,      /* Data connection over PPP */
  GC_TYPE_NULL,               /* Data connection with NULL Device */
  GC_TYPE_SIM,          /* Data connection with SIM */
  GC_TYPE_WAP,                /* Data connection for the WAP browser */
#if defined (FF_PKTIO) OR defined (FF_TCP_IP) || defined(FF_GPF_TCPIP) OR defined (FF_PSI)
  GC_TYPE_PKT,                 /* Data connection for PKTIO */
#endif /* FF_PKTIO OR FF_TCP_IP || FF_GPF_TCPIP OR FF_PSI  */
  GC_TYPE_EMAIL               /* Data connection for an Email - Client */

} T_GPRS_CONNECT_TYPE;

#ifdef GPRS
typedef struct
{
  T_PDP_CONTEXT_STATE      state;
  UBYTE                    smreg_ti;
  T_DTI_ENTITY_ID          entity_id;
  T_DTI_CONN_LINK_ID       link_id;            /* link_id for SNDCP */
  T_DTI_CONN_LINK_ID       link_id_uart;
  T_DTI_CONN_LINK_ID       link_id_new;
  USHORT                   upper_tui;
  T_ACI_CMD_SRC            owner;
  T_NAS_ip                 pdp_address_allocated;
  T_PDP_CONTEXT_PCO        user_pco;           /* setup with %CGPCO=... */
  T_PDP_CONTEXT_PCO        network_pco;
  T_SMREG_PDP_ACTIVATE_CNF *p_pdp_activate_cnf;

} T_PDP_CONTEXT_INTERNAL_DATA;


typedef struct _T_PDP_CONTEXT_INTERNAL
{
  T_PDP_CONTEXT_TYPE             type;
  U8                             cid;    /* PDP context id */
  T_PDP_CONTEXT_INTERNAL_DATA    internal_data;
  T_PDP_CONTEXT                  attributes;
  T_PS_ctrl_qos                  ctrl_qos;
  T_PS_qos                       qos;
  T_PS_ctrl_min_qos              ctrl_min_qos;
  T_PS_min_qos                   min_qos;
  T_PS_ctrl_qos                  ctrl_neg_qos; /* holds the negotiated qos, set with smreg_pdp_activate_cnf is received */
  T_PS_qos                       neg_qos;      /* holds the negotiated qos, set with smreg_pdp_activate_cnf is received */
  T_TFT_INTERNAL                 *p_tft_pf;
  BOOL                            tft_changed;  /* True if the TFT is changed while a PDP context is activated           */
//T_TFT_INTERNAL                 *p_tft_pf_active;
//T_TFT_MODIFICATION_ACTION       tft_modify;

  struct _T_PDP_CONTEXT_INTERNAL *p_next;

} T_PDP_CONTEXT_INTERNAL;



EXTERN T_PDP_CONTEXT_STATE get_state_over_cid      ( U8 cid );
EXTERN void                set_state_over_cid      ( U8 cid, T_PDP_CONTEXT_STATE c_state );
EXTERN T_PDP_CONTEXT_STATE get_state_working_cid   ( void );
EXTERN void                set_state_working_cid   ( T_PDP_CONTEXT_STATE c_state );
EXTERN T_ACI_CAL_OWN       get_owner_over_cid      ( U8 cid );
EXTERN T_PDP_CONTEXT_STATE get_state_over_nsapi_set( USHORT *nsapi_set, U8 *cid );


EXTERN T_PDP_CONTEXT_INTERNAL *pdp_context_create_node( U8 cid );
EXTERN int                     pdp_context_remove_node( U8 cid );
EXTERN T_PDP_CONTEXT_INTERNAL *pdp_context_find_node_from_cid( U8 cid );
EXTERN T_PDP_CONTEXT_INTERNAL *pdp_context_find_node_from_smreg_ti( U8 smreg_ti );
EXTERN T_PDP_CONTEXT_INTERNAL *pdp_context_find_node_from_dti_id( U8 dti_id );
EXTERN T_PDP_CONTEXT_INTERNAL *pdp_context_find_matching_node( T_SMREG_VAL_pdp_type smreg_pdp_type, T_NAS_ctrl_ip_address ctrl_ip_addr, T_NAS_ip_address *ip_addr );

EXTERN T_TFT_INTERNAL         *pdp_context_add_tft_pf ( U8 cid, U8 tft_pf_id );
EXTERN BOOL                    pdp_context_del_tft_pf ( U8 cid, U8 tft_pf_id );
EXTERN T_TFT_INTERNAL         *pdp_context_find_tft_pf( U8 cid, U8 tft_pf_id );
EXTERN U8                      pdp_context_get_no_of_tft_pfs( U8 cid );
EXTERN void                    pdp_context_copy_tft_to_active_list( U8 cid );
//EXTERN T_TFT_MODIFICATION      pdp_context_get_modification_action( U8 cid );
EXTERN void                    pdp_context_clear_tft_active_list( U8 cid );
//EXTERN T_TFT_MODIFICATION      pdp_context_compare_tft( U8 cid );
EXTERN U8                      pdp_context_check_if_nodes_exists( U8 *cid_array );
EXTERN U8                      pdp_context_validate_pdp_type    ( U8 *cid_array, T_PDP_TYPE pdp_type );

#endif /*GPRS */

EXTERN void   gaci_init   ( void );
EXTERN void   gaci_reset  ( void );
EXTERN void   gaci_finit  ( void );

#ifdef DTI
EXTERN SHORT  gaci_get_cid_over_link_id ( T_DTI_CONN_LINK_ID  link_id );
EXTERN SHORT  gaci_get_cid_over_dti_id  ( UBYTE  dti_id );
EXTERN T_DTI_CONN_LINK_ID  gaci_get_link_id_over_peer ( T_DTI_ENTITY_ID entity_id );
#endif

EXTERN  void  cmhSM_disconnect_cid ( SHORT cid, T_GPRS_CONNECT_TYPE type );
EXTERN  BOOL  uart_is_mt_te_link( void );

/* for ATZ */
EXTERN T_ACI_RETURN sGprsAT_Z     ( T_ACI_CMD_SRC srcId );
EXTERN BOOL         gaci_isATZcmd ( void );


EXTERN U8 pdp_context_get_free_cid( void );


EXTERN void gaci_RAT_caller ( SHORT rat_id, SHORT cid, UBYTE cmdBuf, UBYTE cme_err );
EXTERN void gaci_SAT_err(USHORT cause);

#endif /* GACI_H */

