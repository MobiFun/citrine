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
|  Purpose :  Definitions for the command handler of the
|             GPRS session management ( SM ).
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef CMH_SM_H
#define CMH_SM_H

/*==== CONSTANTS ==================================================*/

/* dynamic numbers of nsapis */
#define SMH_FIRST_FREE_NSAPIS   NAS_NSAPI_5
#define SMH_LAST_FREE_NSAPIS    NAS_NSAPI_15
#define SMH_NSAPI_MAX           16

/* invalid values */
#define INVALID_NSAPI  (0xFF)          /* marks an invalid nsapi */

/* for network requested context activation */
#define MAX_GPRS_CALL_TABLE_ENTRIES   2
  /* MAX_PDP_TYPE_LEN + MAX_PDP_ADDR_LEN + MAX_L2P_LENGTH = 7 + 21 + 6 = 34 */
#define MAX_CRING_INFORMATION_LENGTH  42

/* undefiend ti */
#define UNDEFINED_TI 0xFF

typedef enum
{
  PCO_USER,
  PCO_NETWORK

} T_PCO_TYPE;

typedef enum
{
  SNDCP_PEER_NORMAL,
  SNDCP_PEER_SWITCHED

} T_SNDCP_PEER;

typedef enum
{
  GCTT_NORMAL = 0,
  GCTT_REACTIVATION

} T_GPRS_CALL_TABLE_TYPE;

typedef enum
{
  GPRS_DETACH = 0,
  GPRS_ATTACH
} T_GPRS_ATTACH_STATE;


/*==== MACROS =====================================================*/
#define GET_NUM_OF_DTI_ENTITIES(A) (sizeof(A)/(sizeof(A##[0])))


/*==== TYPES ======================================================*/

typedef struct
{
  UBYTE   len;
  UBYTE   pco[251];
} T_GPRS_CONT_PCO;


typedef struct
{
  T_SMREG_PDP_ACTIVATE_IND    sm_ind;
  char                        L2P[MAX_L2P_LENGTH];
  SHORT                       cid;
  UBYTE                       reactivation;         /* T_GPRS_CALL_TABLE_TYPE */

} T_GPRS_CALL_TABLE;

typedef enum             /* GPRS Packet Data Protocol type              */
{
  PDP_T_NONE = 0,        /* no valid PDP type                           */
  PDP_T_PPP  = SMREG_PDP_PPP,  /* Point to Point Protocol (IETF STD 51)       */
  PDP_T_IP   = SMREG_PDP_IPV4, /* Internet Protocol, version 4, IETF STD 5    */
  PDP_T_IPV6 = SMREG_PDP_IPV6, /* Internet Protocol, version 6, IETF RFC 2460 */
  PDP_T_MAX              /* maximum GPRS PDP types                      */
  
} T_GACI_PDP_TYPE;

#ifdef CO_UDP_IP
typedef void  T_SM_IP_CB (T_ACI_RETURN result);
#endif /* WCO_UDP_IP */

/*==== PROTOTYPES =================================================*/
EXTERN void   cmhSM_NetDeactivate       ( void );
EXTERN void   cmhSM_Deactivated         ( void );
EXTERN void   cmhSM_NetActivate         ( void );
EXTERN SHORT  cmhSM_NoActivate          ( void );
EXTERN SHORT  cmhSM_Activated           ( T_SMREG_PDP_ACTIVATE_CNF *pdp_cnf );
EXTERN void   cmhSM_NetModify           ( T_SMREG_PDP_MODIFY_IND       *smreg_pdp_modify_ind );

#ifdef REL99
EXTERN SHORT  cmhSM_Modified            ( T_SMREG_PDP_MODIFY_CNF       *smreg_pdp_modify_cnf );
EXTERN SHORT  cmhSM_NoModify            ( T_SMREG_PDP_MODIFY_REJ       *smreg_pdp_nodify_rej );
EXTERN SHORT  cmhSM_ActivatedSecondary  ( T_SMREG_PDP_ACTIVATE_SEC_CNF *p_smreg_pdp_activate_sec_cnf );
EXTERN SHORT  cmhSM_NoActivateSecondary ( T_SMREG_PDP_ACTIVATE_SEC_REJ *p_smreg_pdp_activate_sec_rej );
#endif


EXTERN void   cmhSM_Init                ( void );
EXTERN void   cmhSM_Reset               ( void );
EXTERN void   cmhSM_ResetNonWorkingContexts( void );
EXTERN void   cmhSM_empty_call_table    ( void );
EXTERN void   cmhSM_GiveNSapiFree       ( USHORT cid );
EXTERN void   cmhSM_contextDeactivated  ( void );
EXTERN USHORT cmhSM_pdp_typ_to_string   ( UBYTE pdp_typ_no, char* string );
EXTERN UBYTE  cmhSM_Get_pdp_type        ( void );
EXTERN void   cmhSM_Get_pdp_address     ( T_NAS_ip_address *pdp_address, T_NAS_ctrl_ip_address * ctrl_ip_address );
EXTERN UBYTE  cmhSM_pdp_address_to_ip   ( T_PDP_TYPE pdp_type, T_NAS_ip *pdp_addr_str, U8 *ip );
EXTERN void   cmhSM_ip_to_pdp_address   ( UBYTE *ip, T_NAS_ip pdp_address );
EXTERN void   cmhSM_Get_smreg_apn       ( T_SMREG_apn   *smreg_apn );
EXTERN UBYTE  cmhSM_Get_h_comp          ( void );
EXTERN UBYTE  cmhSM_Get_d_comp          ( void );
EXTERN void   cmhSM_change_def_QOS      ( T_PS_qos     *qos, T_PS_ctrl_qos     ctrl_qos );
EXTERN void   cmhSM_change_def_QOS_min  ( T_PS_min_qos *qos, T_PS_ctrl_min_qos ctrl_min_qos );
EXTERN void   cmhSM_Set_default_QOS     ( U8 cid );
EXTERN void   cmhSM_Set_default_QOS_min ( U8 cid );
EXTERN void   cmhSM_Get_QOS             ( T_PS_qos     *dest_qos );
EXTERN void   cmhSM_Get_QOS_min         ( T_PS_min_qos *dest_qos_min );
EXTERN char*  cmhSM_ring_gprs_par       ( void );
EXTERN BOOL   cmhSM_call_answer         ( UBYTE ring_counter, T_ACI_CRING_MOD mode );
EXTERN BOOL   cmhSM_call_reject         ( UBYTE ring_counter, T_ACI_CRING_MOD mode );
EXTERN BOOL   cmhSM_sAT_H               ( T_ACI_CMD_SRC srcId, T_ACI_RETURN *aci_ret );
EXTERN BOOL   cmhSM_sAT_A               ( T_ACI_CMD_SRC srcId, T_ACI_RETURN *aci_ret );
EXTERN USHORT cmhSM_Give_nsapi_set      ( U8 cid );
EXTERN BOOL   cmhSM_apn_well_formed     ( T_APN apn );
EXTERN USHORT cmhSM_transform_pdp_type  ( char *pdp_type );
EXTERN void   cmhSM_cgerep_buffer       ( void );
EXTERN void   cmhSM_save_event          ( T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param );
EXTERN void   cmhSM_set_sms_service     ( T_CGSMS_SERVICE service );
EXTERN void   cmhSM_sms_service_changed ( UBYTE service );
EXTERN T_ACI_RETURN cmhSM_activate_context_For_WAP(void);

EXTERN BOOL   cmhSM_next_work_cid       ( T_ACI_AT_CMD curCmd );
EXTERN T_PDP_CONTEXT_STATE pdp_context_get_state_for_cid( U8 cid );
EXTERN BOOL   pdp_context_type_omitted  ( char *p_type );
EXTERN BOOL   pdp_context_type_valid    ( char *p_type );
EXTERN BOOL   pdp_context_apn_omitted   ( char *p_apn );
EXTERN BOOL   pdp_context_apn_valid     ( char *p_apn );
EXTERN BOOL   pdp_context_addr_omitted  ( T_NAS_ip *p_addr );
EXTERN BOOL   pdp_context_addr_valid    ( T_NAS_ip *p_addr );

EXTERN BOOL pdp_context_cid_used_by_other( U8 cid );

EXTERN SHORT cmhSM_make_active_cid_list ( T_ACI_CMD_SRC srcId, U8 *cids );
EXTERN void  cmhSM_Set_pdp_type( U8 cid,  char *pdp_type );

EXTERN U8     cmhSM_get_pdp_addr_for_CGPADDR  ( U8 cid, T_NAS_ip * pdp_adress );
EXTERN void   cmhSM_GprsAttached              ( T_GPRS_ATTACH_STATE state );
EXTERN void cmhSM_activate_context    (void);
EXTERN void   cmhSM_data_link_context         (void);

#ifdef DTI
EXTERN SHORT  cmhSM_connect_context           ( U8 cid, T_DTI_ENTITY_ID peer );
#endif /* DTI */

EXTERN SHORT  cmhSM_context_connected         ( USHORT nsapi);
EXTERN BOOL   cmhSM_is_smreg_ti_used          ( UBYTE ti, U8 *cid );
EXTERN void   cmhSM_context_reactivation      ( void );
EXTERN void   cmhSM_stop_context_reactivation ( void );
EXTERN void   cmhSM_next_call_table_entry     ( void );
EXTERN SHORT  cmhSM_connect_working_cid       ( void );
EXTERN void   cmhSM_set_PCO                   ( U8 cid, T_PCO_TYPE pco_type, UBYTE* buf_addr, UBYTE length );

#if defined (CO_UDP_IP) || defined (FF_GPF_TCPIP)
EXTERN void   cmhSM_IP_activate_cb            ( T_ACI_RETURN result );
EXTERN void   cmhSM_IP_Enable                 (T_DTI_CONN_LINK_ID link_id );
EXTERN void   cmhSM_IP_Disable                (void);
#endif /* (CO_UDP_IP) || defined (FF_GPF_TCPIP) */

/* execution AT set command without parameter check */
EXTERN void sAT_PlusCGDCONT_exec               ( U8 cid, T_PDP_CONTEXT *inputCtxt);
EXTERN BOOL sAT_PlusCGDSCONT_exec              ( U8 cid, T_PDP_CONTEXT *p_pdp_context_input);

EXTERN T_ACI_RETURN cmhSM_CGPCO_HEX       ( U8 cid, UBYTE *pco_array, UBYTE pco_len );
EXTERN ULONG  cmhSM_get_link_id_UPM_peer       ( U8 cid );
EXTERN void   cmhSM_context_deactivated        ( USHORT nsapi_set );
EXTERN void   cmhSM_connection_down            ( UBYTE dti_id );

EXTERN T_ACI_RETURN cmhSM_deactivateContexts   ( T_ACI_CMD_SRC srcId, SHORT *cids);
EXTERN T_ACI_RETURN cmhSM_deactivateAContext   ( T_ACI_CMD_SRC srcId, U8 cid );

EXTERN BOOL isContextDeactivationRequestedByCGACT(SHORT cid);


//EXTERN void      cmhSM_ip_to_pdp_address       ( UBYTE *ip, T_NAS_ip pdp_address );
EXTERN T_PS_qos* cmhSM_getCurQOS               ( U8 cid );

EXTERN BOOL   cmhSM_isContextActive            ( void );

EXTERN void   cmhSM_connection_down           ( UBYTE dti_id );


#ifdef DTI
EXTERN  void  set_conn_param_on_working_cid ( UBYTE owner, T_DTI_ENTITY_ID entity_id );
EXTERN  void  set_conn_param_on_all_working_cids ( UBYTE owner, T_DTI_ENTITY_ID entity_id );
#endif /* DTI */

EXTERN BOOL PKTIO_SNDCP_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type);

EXTERN void cmhSM_free_pdpcontext_list(void);
EXTERN UBYTE cmhSM_mapSM2ACI_Cause(U16 cause_value);
EXTERN void  cmhSM_clear_work_cids(U8 cid);

/*==== EXPORT =====================================================*/
#ifdef CMH_SMF_C

GLOBAL T_ENT_STAT      smEntStat;

U8                     work_cids[PDP_CONTEXT_CID_MAX + 1];
U8                     cid_pointer;

T_PDP_CONTEXT_INTERNAL pdp_context_default;
T_PS_qos               pdp_context_default_qos;
T_PS_min_qos           pdp_context_default_qos_min;

SHORT                  automatic_response_mode;

/* network requested context activation */
T_GPRS_CALL_TABLE      gprs_call_table[MAX_GPRS_CALL_TABLE_ENTRIES];
SHORT                  current_gprs_ct_index;
SHORT                  gprs_ct_index;

/* command values */
T_CGEREP_MODE          sm_cgerep_mode;
T_CGEREP_BFR           sm_cgerep_bfr;
T_CGSMS_SERVICE        sm_cgsms_service;
T_ACI_CMD_SRC       sm_cgerep_srcId;

#else

EXTERN T_ENT_STAT      smEntStat;

EXTERN U8              work_cids[PDP_CONTEXT_CID_MAX + 1];
EXTERN U8              cid_pointer;
EXTERN T_GPRS_CONT_REC defaultCtx;

EXTERN T_PDP_CONTEXT_INTERNAL pdp_context_default;
EXTERN T_PS_qos               pdp_context_default_qos;
EXTERN T_PS_min_qos           pdp_context_default_qos_min;
EXTERN T_PDP_CONTEXT_PCO      pdp_context_default_pco;

EXTERN SHORT                  automatic_response_mode;

/* network requested context activation */
EXTERN T_GPRS_CALL_TABLE gprs_call_table[MAX_GPRS_CALL_TABLE_ENTRIES];
EXTERN SHORT             current_gprs_ct_index;
EXTERN SHORT             gprs_ct_index;

/* command values */
EXTERN T_CGEREP_MODE     sm_cgerep_mode;
EXTERN T_CGEREP_BFR      sm_cgerep_bfr;
EXTERN T_CGSMS_SERVICE   sm_cgsms_service;

EXTERN T_ACI_CMD_SRC cmhSM_getSrcIdOfRunningCGACTDeactivation(U8 cid);
EXTERN T_ACI_CMD_SRC       sm_cgerep_srcId;

#endif /* CMH_SMF_C */

#endif /* CMH_SM_H */

#endif  /* GPRS */

/*==== EOF =======================================================*/
