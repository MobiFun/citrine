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
|  Purpose :  GPRS Command handler interface definitions.
+----------------------------------------------------------------------------- 
*/ 

#ifndef GACI_CMH_H
#define GACI_CMH_H


/*==== MACROS =====================================================*/
#define GPRS_EVENT_REPORTING_BUFFER_SIZE  10

/*==== DEFINES ====================================================*/
/* the maximum length of strings to describe the PDP typ, PDP address, APN and L2P */
#define MAX_PDP_TYPE_LEN  7
#define MAX_APN_LEN       103
#define MAX_DNS_LEN       15
#define MAX_USER_LEN      63  /* this is a common value for Routers etc... */
#define MAX_PSWD_LEN      63  /* Name is changed as the same name is used in MFW with different value */
#define MAX_PDP_ADDR_LEN  63  /* IPv6 address n01.n02..n16 (16 * 3 + 15) */
#if defined (FF_PKTIO) OR defined (FF_TCP_IP) || defined(FF_GPF_TCPIP) OR defined (FF_PSI)
#define MAX_L2P_LENGTH    6
#else /* FF_PKTIO || FF_TCP_IP || FF_GPF_TCPIP || FF_PSI */
#define MAX_L2P_LENGTH    4
#endif /* FF_PKTIO || FF_TCP_IP || FF_GPF_TCPIP OR FF_PSI */
#ifdef REL99
#define QOS_R99_TC_OMITTED            0xff
#define QOS_R99_ORDER_OMITTED         0xff
#define QOS_R99_DEL_ERR_SDU_OMITTED   0xff
#define QOS_R99_MAX_SDU_OMITTED       0xffff
#define QOS_R99_MAX_BR_UL_OMITTED     0xffff
#define QOS_R99_MAX_BR_DL_OMITTED     0xffff
#define QOS_R99_XFER_DELAY_OMITTED    0xffff
#define QOS_R99_HANDLING_PRIO_OMITTED 0xff
#define QOS_R99_GUAR_BR_UL_OMITTED    0xffff
#define QOS_R99_GUAR_BR_DL_OMITTED    0xffff
#define QOS_R99_RATIO_MANT_OMITTED    0xff
#define QOS_R99_RATIO_EXP_OMITTED     0xff
#endif /* REL99 */
#define PDP_CONTEXT_CID_OMITTED   0
#define PDP_CONTEXT_CID_MIN       1
#define PDP_CONTEXT_CID_MAX       11
#define PDP_CONTEXT_CID_INVALID   (PDP_CONTEXT_CID_MAX + 1)
#define PDP_CONTEXT_TYPE_LEN_MAX  7
#define PDP_CONTEXT_APN_LEN_MAX   103 // ???
#define PDP_CONTEXT_ADDR_LEN_MAX  16

#define MAX_PDP_CONTEXT_TYPE_LEN 7
#define MAX_PDP_CONTEXT_APN_LEN  103
#define MAX_PDP_CONTEXT_ADDR_LEN 16
#define MAX_PDP_D_COMP_LEN       1
#define MAX_PDP_H_COMP_LEN       1

#define PDP_CONTEXT_QOS_OMITTED  0

#define TFT_PF_ID_MIN            1
#define TFT_PF_ID_MAX            4  /* Due to unclear definition in spec. it is decided to limit the max number of PF's to 4 instead of 8 */

#ifdef REL99
#define TFT_FLOW_LABEL_MIN           0x00000
#define TFT_FLOW_LABEL_MAX           0xfffff
#define TFT_FLOW_LABEL_INVALID      -1
#define TFT_PROT_OR_NXT_HDR_MIN      0x00
#define TFT_PROT_OR_NXT_HDR_MAX      0xff
#define TFT_PROT_OR_NXT_HDR_INVALID -1
#define TFT_PRM_LIST_MAX_BYT_LEN     254 /* TFT IE: 257-IEI-IEL-mandatoryPrms */
#endif


typedef enum                    /* CGDCONT command d_comp */
{
  GPRS_CID_OMITTED = 0,         /* value is omitted */
  GPRS_CID_1,                   /* PDP data compression off (default) */
  GPRS_CID_2,                   /* PDP data compression on */
#if defined (FF_PKTIO) OR defined (FF_PSI)
  GPRS_CID_3,
  GPRS_CID_4,
  GPRS_CID_5,
  GPRS_CID_6,
#endif
  GPRS_CID_INVALID              /* invalid value */

} T_GPRS_CID;

#define INVALID_CID         GPRS_CID_OMITTED

#if defined (FF_PKTIO) OR defined (FF_PSI)
#define MAX_CID             GPRS_CID_6
#else
#define MAX_CID             GPRS_CID_2
#endif

#define MAX_CID_PLUS_EINS   GPRS_CID_INVALID

/*==== TYPES ======================================================*/

//typedef char T_PDP_ADDRESS  [MAX_PDP_CONTEXT_ADDR_LEN];
//typedef T_NAS_ip T_PDP_ADDRESS  ;
typedef char T_PDP_TYPE     [MAX_PDP_CONTEXT_TYPE_LEN];
typedef char T_APN          [MAX_APN_LEN];
typedef char T_L2P          [MAX_L2P_LENGTH];

typedef struct
{
  T_APN               apn;
  T_PDP_TYPE          pdp_type;
  T_NAS_ip            pdp_addr;
  SHORT               d_comp;
  SHORT               h_comp;
  T_PS_ctrl_qos       ctrl_qos;
  T_PS_qos            qos;
  T_PS_ctrl_min_qos   ctrl_min_qos;
  T_PS_min_qos        min_qos;
} T_GPRS_CONT_REC;

/*---- Types of  -----------------------------------------*/
typedef enum                    /* CGDCONT command d_comp */
{
  CGDCONT_D_COMP_OMITTED = -1,  /* value is omitted */
  CGDCONT_D_COMP_OFF,           /* PDP data compression off (default) */
  CGDCONT_D_COMP_ON,            /* PDP data compression on */
  CGDCONT_D_COMP_INVALID        /* invalid value */

} T_CGDCONT_D_COMP;

typedef enum                    /* CGDCONT command h_comp */
{
  CGDCONT_H_COMP_OMITTED = -1,  /* value is omitted */
  CGDCONT_H_COMP_OFF,           /* PDP header compression off (default) */
  CGDCONT_H_COMP_ON,            /* PDP header compression on */
  CGDCONT_H_COMP_INVALID        /* invalid value */

} T_CGDCONT_H_COMP;

typedef enum                    /* CGATT command state */
{
  CGATT_STATE_OMITTED = -1,     /* value is omitted */
  CGATT_STATE_DETACHED,         /* GPRS detached */
  CGATT_STATE_ATTACHED,         /* GPRS attached */
  CGATT_STATE_INVALID           /* invalid value */

} T_CGATT_STATE;

typedef enum                    /* CGACT command state */
{
  CGACT_STATE_OMITTED = -1,     /* value is omitted */
  CGACT_STATE_DEACTIVATED,      /* PDP context detached */
  CGACT_STATE_ACTIVATED,        /* PDP context attached */
  CGACT_STATE_INVALID           /* invalid value */

} T_CGACT_STATE;

typedef enum                    /* CGAUTO command n */
{
  CGAUTO_N_OMITTED = -1,        /* value is omitted */
  CGAUTO_N_GPRS_RES_OFF,        /* turn off automatic response for GPRS only */
  CGAUTO_N_GPRS_RES_ON,         /* turn on automatic response for GPRS only */
  CGAUTO_N_MCM_GPRS_ONLY,       /* modem compabitbility mode, GPRS only */
  CGAUTO_N_MCM_GPRS_CSC,        /* modem compabitbility mode, GPRS and circuit switched calls (default) */
  CGAUTO_N_INVALID              /* invalid value */

} T_CGAUTO_N;

typedef enum                    /* CGANS command response */
{
  CGANS_RESPONSE_OMITTED = -1,  /* value is omitted */
  CGANS_RESPONSE_REJECT,        /* reject the request */
  CGANS_RESPONSE_ACCEPT,        /* accept and request that the PDP context be activated */
  CGANS_RESPONSE_INVALID        /* invalid value */

} T_CGANS_RESPONSE;

typedef enum                    /* CGANS command response */
{
  CGCLASS_CLASS_OMITTED = -1,   /* value is omitted */
  CGCLASS_CLASS_A,              /* mobile class A  (highest) */
  CGCLASS_CLASS_B,              /* mobile class B  (if necessary consider NET III) */
  CGCLASS_CLASS_CG,             /* mobile class CG (GPRS only mode) */
  CGCLASS_CLASS_CC,             /* mobile class CC (circuit switched only mode - lowest) */
  CGCLASS_CLASS_MAX             /* invalid value */

} T_CGCLASS_CLASS;

typedef enum                    /* CGANS command response */
{
  PERCENT_CGCLASS_OMITTED = -1,   /* value is omitted */
  PERCENT_CGCLASS_A,              /* mobile class A  (highest) */
  PERCENT_CGCLASS_B,              /* mobile class B   */
  PERCENT_CGCLASS_BG,             /* mobile class BG   */
  PERCENT_CGCLASS_BC,             /* mobile class BC   */
  PERCENT_CGCLASS_BX,             /* mobile class BX   */
  PERCENT_CGCLASS_CG,             /* mobile class CG (GPRS only mode) */
  PERCENT_CGCLASS_CC,             /* mobile class CC (circuit switched only mode - lowest) */
  PERCENT_CGCLASS_MAX             /* invalid value */

} T_PERCENT_CGCLASS;

typedef enum                    /* CGEREP command mode */
{
  CGEREP_MODE_OMITTED = -1,     /* value is omitted */
  CGEREP_MODE_BUFFER,           /* buffer unsolicited result codes */
  CGEREP_MODE_DICARD_RESERVED,  /* discard unsolicited result codes when MT-TE link is reserved */
  CGEREP_MODE_BUFFER_RESERVED,  /* buffer unsolicited result codes in the MT when MT-TE link is reserved */
  CGEREP_MODE_INVALID           /* invalid value */

} T_CGEREP_MODE;

typedef enum                    /* CGEREP command bfr */
{
  CGEREP_BFR_OMITTED = -1,      /* value is omitted */
  CGEREP_BFR_CLEAR,             /* buffer unsolicited result codes */
  CGEREP_BFR_FLUSH,             /* discard unsolicited result codes when MT-TE link is reserved */
  CGEREP_BFR_INVALID            /* invalid value */

} T_CGEREP_BFR;

typedef enum                    /* CGREG command read stat */
{
  CGREG_STAT_NOT_PRESENT = -1,  /* not present, last state is not indicated */
  CGREG_STAT_NOT_REG,           /* not registered, no searching */
  CGREG_STAT_REG_HOME,          /* registered, home network */
  CGREG_STAT_SEARCHING,         /* not registered, but searching */
  CGREG_STAT_REG_DEN,           /* registration denied */
  CGREG_STAT_UNKN,              /* unknown */
  CGREG_STAT_REG_ROAM           /* registered, roaming */

} T_CGREG_STAT;

typedef enum                    /* %CGREG command read stat */
{
  P_CGREG_STAT_NOT_PRESENT = -1,  /* not present, last state is not indicated */
  P_CGREG_STAT_NOT_REG,           /* not registered, no searching */
  P_CGREG_STAT_REG_HOME,          /* registered, home network */
  P_CGREG_STAT_SEARCHING,         /* not registered, but searching */
  P_CGREG_STAT_REG_DEN,           /* registration denied */
  P_CGREG_STAT_UNKN,              /* unknown */
  P_CGREG_STAT_REG_ROAM,          /* registered, roaming */
  P_CGREG_STAT_LIMITED,           /* limited service */
  P_CGREG_STAT_GSM_CALL,          /* GSM call is active */
  P_CGREG_STAT_NO_CELL,           /* no cell available */
  P_CGREG_STAT_TRY_TO_UPDATE      /* next attempt to update MS */

} T_P_CGREG_STAT;


typedef enum                    /* CGSMS command service */
{
  CGSMS_SERVICE_OMITTED = -1,   /* value is omitted */
  CGSMS_SERVICE_GPRS,           /* GPRS */
  CGSMS_SERVICE_CS,             /* circuit switched */
  CGSMS_SERVICE_GPRS_PREFERRED, /* GPRS preferred */
  CGSMS_SERVICE_CS_PREFERRED,   /* circuit switched preferred */
  CGSMS_SERVICE_INVALID         /* invalid value */

} T_CGSMS_SERVICE;

typedef enum                    /* CGAATT command automatic attach mode */
{
  CGAATT_ATTACH_MODE_OMITTED = -1, /* value is omitted */
  CGAATT_ATTACH_MODE_AUTOMATIC,    /* automatic attach */
  CGAATT_ATTACH_MODE_MANUAL,       /* manual attach */
  CGAATT_ATTACH_MODE_INVALID       /* invalid value */

} T_CGAATT_ATTACH_MODE;

typedef enum                    /* CGAATT command automatic detach after context deactivation */
{
  CGAATT_DETACH_MODE_OMITTED = -1, /* value is omitted */
  CGAATT_DETACH_MODE_ON,           /* on */
  CGAATT_DETACH_MODE_OFF,          /* off */
  CGAATT_DETACH_MODE_INVALID       /* invalid value */

} T_CGAATT_DETACH_MODE;

/*---- Types for event reporting-----------------------------------*/

typedef enum                /* Defined events for GPRS event reporting */
{
  CGEREP_EVENT_INVALID = -1, /* reserved value */
  CGEREP_EVENT_REJECT,      /* network request context activation */
  CGEREP_EVENT_NW_REACT,    /* network requested context reactivation */
  CGEREP_EVENT_NW_DEACT,    /* network forced context deactivation */
  CGEREP_EVENT_ME_DEACT,    /* mobile equipment forced context deactivation */
  CGEREP_EVENT_NW_DETACH,   /* network forced detach */
  CGEREP_EVENT_ME_DETACH,   /* mobile equipment forced detach */
  CGEREP_EVENT_NW_CLASS,    /* network forced class change */
  CGEREP_EVENT_ME_CLASS,    /* mobile equipment forced class change */
  CGEREP_EVENT_NW_ACT,    /* network requested context activation */
  CGEREP_EVENT_ME_ACT    /* mobile equipment initiated context activation */
} T_CGEREP_EVENT;

typedef struct              /* parameter of unsolicited event: REJECT */
{
  T_PDP_TYPE        pdp_type;
  T_NAS_ip          pdp_addr;

} T_EVENT_REJECT;

typedef struct              /* parameter of unsolicited event: NW REACT, NW DEACT, ME DEACT */
{
  T_PDP_TYPE        pdp_type;
  T_NAS_ip          pdp_addr;
  SHORT             cid;

} T_EVENT_ACT;

typedef union
{
  T_EVENT_REJECT    reject;
  T_EVENT_ACT       act;
  T_CGCLASS_CLASS   mobile_class;

} T_CGEREP_EVENT_REP_PARAM;


typedef enum
{
  PDP_CONTEXT_D_COMP_OFF     =  0,
  PDP_CONTEXT_D_COMP_INVALID =  3,
  PDP_CONTEXT_D_COMP_OMITTED =  255
} T_PDP_CONTEXT_D_COMP;


typedef enum
{
  PDP_CONTEXT_H_COMP_OFF     =  0,
  PDP_CONTEXT_H_COMP_INVALID =  3,
  PDP_CONTEXT_H_COMP_OMITTED =  255
} T_PDP_CONTEXT_H_COMP;


typedef enum
{
  PDP_CONTEXT_TYPE_UNKNOWN   = 0,
  PDP_CONTEXT_TYPE_PRIMARY   = 1,
  PDP_CONTEXT_TYPE_SECONDARY = 2
  
}T_PDP_CONTEXT_TYPE;

typedef enum
{
  PDP_CONTEXT_STATE_INVALID = 0,             /* 0x00: invalid value                                                                       */
  PDP_CONTEXT_STATE_DEFINED,                 /* 0x01: context defined                                                                     */
  PDP_CONTEXT_STATE_ATTACHING,               /* 0x02: Attaching before context activation (+CGDATA or +CGACT)                             */
  PDP_CONTEXT_STATE_ESTABLISH_1,             /* 0x03: PPP informed over estblish                                                          */
  PDP_CONTEXT_STATE_ESTABLISH_2,             /* 0x04: SM informed over estblish                                                           */
  PDP_CONTEXT_STATE_ESTABLISH_3,             /* 0x05: PPP informed over activated context                                                 */
  PDP_CONTEXT_STATE_ACTIVATING,              /* 0x06: SM context activating                                                               */
  PDP_CONTEXT_STATE_ABORT_ESTABLISH,         /* 0x07: abort establish                                                                     */
  PDP_CONTEXT_STATE_ACTIVATED,               /* 0x08: context activated                                                                   */
  PDP_CONTEXT_STATE_ACTIVATED_ESTABLISH_1,   /* 0x09: Context is activated. PPP informed over estblish                                    */
  PDP_CONTEXT_STATE_ACTIVATED_MODIFYING,     /* 0x0a: context activated, context modification in progress                                 */
  PDP_CONTEXT_STATE_DATA_LINK,               /* 0x0b: Data link                                                                           */
  PDP_CONTEXT_STATE_DATA_LINK_MODIFYING,     /* 0x0c: Data link, context modification in progress                                         */
  PDP_CONTEXT_STATE_DEACTIVATE_NORMAL,       /* 0x0d: Context activated/activating: Deactivating from appl. or SMREG_PDP_ACTIVATE_REJ     */
  PDP_CONTEXT_STATE_BREAKDOWN_LINK_NORMAL,   /* 0x0f: Data link deactivation: breakdown data link from PPP (PPP_TERMINATE_IND received) 
                                                      or network initiated context deactivation (SMREG_PDP_DEACTIVATE_IND)                */
  PDP_CONTEXT_STATE_REACTIVATION_1,          /* 0x10: Data link deactivation: PPP_TERMINATE_REQ and SMREG_DEACTIVATE_REQ are sent         */
  PDP_CONTEXT_STATE_REACTIVATION_2           /* 0x11: Data link deactivation: Response is received from either PPP or SM
                                                      e.g. PPP_TERMINATE_IND or SMREG_DEACTIVATE_CNF                                      */
} T_PDP_CONTEXT_STATE;
 

typedef struct _T_TFT_INTERNAL
{
  T_NAS_tft_pf            pf_attributes;
  struct _T_TFT_INTERNAL *p_next;

} T_TFT_INTERNAL;


typedef char T_PDP_CONTEXT_PDP_TYPE[MAX_PDP_CONTEXT_TYPE_LEN];
typedef char T_PDP_CONTEXT_APN[PDP_CONTEXT_APN_LEN_MAX];

typedef struct
{
  T_PDP_CONTEXT_PDP_TYPE pdp_type;
  T_PDP_CONTEXT_APN      pdp_apn;
  T_NAS_ip               pdp_addr;
  U8                     d_comp;  /* T_PDP_CONTEXT_D_COMP */
  U8                     h_comp;  /* T_PDP_CONTEXT_H_COMP */
  U8                     p_cid;   /* Primary PDP context id, used if the context is a secondary PDP context */

} T_PDP_CONTEXT;


typedef struct
{
  UBYTE   len;
  UBYTE   pco[251];

} T_PDP_CONTEXT_PCO;  /* user PCO */


/*---- %CGPCO -----------------------------------------------------*/

#define ACI_PCO_MAX_LEN                         251
#define ACI_PCO_CONTENTMASK_AUTH                0x00000001
#define ACI_PCO_CONTENTMASK_DNS1                0x00000002
#define ACI_PCO_CONTENTMASK_DNS2                0x00000004

#define ACI_PCO_CONFIG_PROT_PPP                 0

#define ACI_PCO_AUTH_PROT_PAP                   0xc023

#define ACI_PCO_PAP_OVERHEAD                    9
#define ACI_PCO_IPCP_OVERHEAD                   7
#define ACI_PCO_PAP_AUTH_REQ                    0x01
#define ACI_PCO_IPCP_PROT_MSB                   0x80
#define ACI_PCO_IPCP_PROT_LSB                   0x21
#define ACI_PCO_IPCP_CONF_REQ                   0x01
#define ACI_PCO_IPCP_CONF_ACK                   0x02
#define ACI_PCO_IPCP_CONF_NAK                   0x03
#define ACI_PCO_IPCP_TYPE_IP                    3
#define ACI_PCO_IPCP_TYPE_DNS1                  129
#define ACI_PCO_IPCP_TYPE_DNS2                  131
#define ACI_PCO_IPCP_LENGTH_IP                  6
#define ACI_PCO_IPCP_LENGTH_DNS1                6
#define ACI_PCO_IPCP_LENGTH_DNS2                6

typedef enum
{
  CGPCO_FORMAT_HEX = 0,
  CGPCO_FORMAT_ASCII
}T_ACI_CGPCO_FORMAT;

typedef enum
{
  CGPCO_MODE_SET_PCO = 0,
  CGPCO_MODE_QUERY_PCO
}T_ACI_CGPCO_MODE;


/*==== PROTOTYPES =================================================*/

/***  set commands  ***/
EXTERN T_ACI_RETURN sAT_PercentCGPCO ( T_ACI_CMD_SRC srcId, U8 cid, USHORT protocol, CHAR *user, CHAR *pwd, CHAR *dns1, CHAR *dns2);
EXTERN T_ACI_RETURN sAT_PercentCGPCO_HEX ( T_ACI_CMD_SRC srcId, UBYTE cid, UBYTE *pco_array, UBYTE pco_len);
EXTERN T_ACI_RETURN sAT_PlusCGDCONT  ( T_ACI_CMD_SRC srcId, U8 cid, T_PDP_CONTEXT *inputCtxt);
EXTERN T_ACI_RETURN sAT_PlusCGQREQ   ( T_ACI_CMD_SRC srcId, U8 cid ,T_PS_qos *qos);
EXTERN T_ACI_RETURN sAT_PlusCGQMIN   ( T_ACI_CMD_SRC srcId, U8 cid ,T_PS_min_qos *qos);
EXTERN T_ACI_RETURN sAT_PlusCGATT    ( T_ACI_CMD_SRC srcId, T_CGATT_STATE state );
EXTERN T_ACI_RETURN sAT_PlusCGACT    ( T_ACI_CMD_SRC srcId, T_CGACT_STATE state, SHORT *cids );
EXTERN T_ACI_RETURN sAT_PlusCGDATA   ( T_ACI_CMD_SRC srcId, char *L2P, U8 *p_cid_array );
EXTERN T_ACI_RETURN sAT_PlusCGPADDR  ( T_ACI_CMD_SRC srcId, SHORT *cids, T_NAS_ip *pdp_adress );
EXTERN T_ACI_RETURN sAT_PlusCGAUTO   ( T_ACI_CMD_SRC srcId, T_CGAUTO_N n );
EXTERN T_ACI_RETURN sAT_PlusCGANS    ( T_ACI_CMD_SRC srcId, USHORT response, char *l2p, U8 cid );
EXTERN T_ACI_RETURN sAT_PlusCGCLASS  ( T_ACI_CMD_SRC srcId, T_CGCLASS_CLASS m_class );
EXTERN T_ACI_RETURN sAT_PlusCGEREP   ( T_ACI_CMD_SRC srcId, T_CGEREP_MODE mode, T_CGEREP_BFR bfr );
EXTERN T_ACI_RETURN sAT_PlusCGSMS    ( T_ACI_CMD_SRC srcId, T_CGSMS_SERVICE service );



EXTERN T_ACI_RETURN sAT_PercentCGAATT ( T_ACI_CMD_SRC srcId, T_CGAATT_ATTACH_MODE att_m, T_CGAATT_DETACH_MODE det_m );
EXTERN T_ACI_RETURN sAT_PercentCGMM   ( T_ACI_CMD_SRC srcId, UBYTE cipher_on, UBYTE tlli_handling );
EXTERN T_ACI_RETURN sAT_PercentSNCNT  ( T_ACI_CMD_SRC srcId, BOOL reset_counter );

EXTERN T_ACI_RETURN sAT_PercentCGPPP   ( T_ACI_CMD_SRC srcId, T_ACI_PPP_PROT protocol );
EXTERN T_ACI_RETURN qAT_PercentCGPPP   ( T_ACI_CMD_SRC srcId, T_ACI_PPP_PROT *protocol );

EXTERN T_ACI_RETURN sAT_PercentCGCLASS ( T_ACI_CMD_SRC srcId, T_PERCENT_CGCLASS m_class );



/***  read commands  ***/
EXTERN T_ACI_RETURN qAT_PercentCGPCO  ( T_ACI_CMD_SRC srcId, ULONG *gateway, ULONG *dns1, ULONG *dns2, USHORT cid);
EXTERN T_ACI_RETURN qAT_PlusCGDCONT   ( T_ACI_CMD_SRC srcId, T_PDP_CONTEXT *p_pdp_context_array, SHORT *cid_array );
EXTERN T_ACI_RETURN qAT_PlusCGQREQ    ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *qos_valid, T_PS_qos *qos);
EXTERN T_ACI_RETURN qAT_PlusCGQMIN    ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *qos_valid, T_PS_qos *qos);
EXTERN T_ACI_RETURN qAT_PlusCGATT     ( T_ACI_CMD_SRC srcId, T_CGATT_STATE *state );
EXTERN T_ACI_RETURN qAT_PlusCGACT     ( T_ACI_CMD_SRC srcId, BOOL  *states, SHORT *cid );
EXTERN T_ACI_RETURN qAT_PlusCGAUTO    ( T_ACI_CMD_SRC srcId, T_CGAUTO_N *n);
EXTERN T_ACI_RETURN qAT_PlusCGCLASS   ( T_ACI_CMD_SRC srcId, T_CGCLASS_CLASS *m_class );
EXTERN T_ACI_RETURN qAT_PlusCGEREP    ( T_ACI_CMD_SRC srcId, T_CGEREP_MODE *mode, T_CGEREP_BFR *bfr );
EXTERN T_ACI_RETURN qAT_PlusCGREG     ( T_ACI_CMD_SRC srcId, T_CGREG_STAT *stat, USHORT *lac, USHORT *ci );
EXTERN T_ACI_RETURN qAT_PercentCGREG  ( T_ACI_CMD_SRC srcId, T_P_CGREG_STAT *stat, USHORT *lac, USHORT *ci );
EXTERN T_ACI_RETURN qAT_PlusCGSMS     ( T_ACI_CMD_SRC srcId, T_CGSMS_SERVICE *service );
EXTERN T_ACI_RETURN qAT_PercentCGAATT ( T_ACI_CMD_SRC srcId, T_CGAATT_ATTACH_MODE *att_m, T_CGAATT_DETACH_MODE *det_m );
EXTERN T_ACI_RETURN qAT_PercentCGCLASS  ( T_ACI_CMD_SRC srcId, T_PERCENT_CGCLASS *m_class, T_PERCENT_CGCLASS *currentClass );
#ifdef REL99
EXTERN T_ACI_RETURN qAT_PlusCGDSCONT( T_ACI_CMD_SRC srcId, T_PDP_CONTEXT *p_pdp_context_array, U8 *cid_array );
EXTERN T_ACI_RETURN qAT_PlusCGTFT   ( T_ACI_CMD_SRC srcId, U8 *cid_array );
#endif
/***  test commands  ***/
EXTERN T_ACI_RETURN tAT_PlusCGPADDR ( T_ACI_CMD_SRC srcId, U8 *cids);
EXTERN T_ACI_RETURN tAT_PlusCGSMS   ( T_ACI_CMD_SRC srcId, SHORT *service_list);

#ifdef REL99
EXTERN T_ACI_RETURN sAT_PlusCGDSCONT( T_ACI_CMD_SRC srcId, U8 cid, T_PDP_CONTEXT *pdp_context_input );
EXTERN T_ACI_RETURN sAT_PlusCGEQREQ ( T_ACI_CMD_SRC srcId, U8 cid, T_PS_qos *qos );
EXTERN T_ACI_RETURN sAT_PlusCGEQMIN ( T_ACI_CMD_SRC srcId, U8 cid, T_PS_qos *qos );
EXTERN T_ACI_RETURN sAT_PlusCGEQNEG ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *qos_valid, T_PS_qos *qos);
EXTERN T_ACI_RETURN sAT_PlusCGCMOD  ( T_ACI_CMD_SRC srcId, U8 *cid);
EXTERN T_ACI_RETURN sAT_PlusCGTFT   ( T_ACI_CMD_SRC srcId, U8 cid, T_NAS_tft_pf *tft_pf_input );

EXTERN T_ACI_RETURN qAT_PlusCGEQREQ ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *qos_valid, T_PS_qos *qos);
EXTERN T_ACI_RETURN qAT_PlusCGEQMIN ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *qos_valid, T_PS_min_qos *qos);

EXTERN T_ACI_RETURN tAT_PlusCGEQNEG_CGCMOD ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *context_activated);
EXTERN T_ACI_RETURN tAT_PlusCGDSCONT       ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *context_activated );
#endif /* REL99 */


/*--------------- extension mechansim ----------------------------*/

/*--------------- constants --------------------------------------*/

/*--------------- configuration ----------------------------------*/

/*--------------- Extension Handler -> AT Interpreter ------------*/

/*--------------- AT Interpreter -> Extension Handler ------------*/

/*------- call-backs for MMI --------------------------------------*/

#ifdef CMH_F_C /*lint -save -e18 */ /* Turn off Lint errors for this "construct" */
EXTERN void rAT_PlusCGACT            ( void );
EXTERN void rAT_PlusCGDATA           ( void );
EXTERN void rAT_PlusCGANS            ( void );
EXTERN void rAT_PlusCGEREP           ( void );
EXTERN void rAT_PlusCGREG            ( void );
EXTERN void rAT_changedQOS           ( void );
EXTERN void rAT_PercentSNCNT         ( void );
EXTERN void rAT_PercentCGREG         ( void );
#ifdef REL99
EXTERN void rAT_PlusCGCMOD           ( void ); /* added for rel99 */
#endif
EXTERN void rAT_PercentCGEV           ( void );
/*lint -restore */
#else
EXTERN void rAT_PlusCGACT            ( SHORT link_id );
EXTERN void rAT_PlusCGDATA           ( SHORT link_id );
EXTERN void rAT_PlusCGANS            ( SHORT link_id );
EXTERN void rAT_PlusCGEREP           ( T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param );
EXTERN void rAT_PlusCGREG            ( T_CGREG_STAT stat, USHORT lac, USHORT ci );
EXTERN void rAT_changedQOS           ( SHORT cid, T_PS_qos *qos );
EXTERN void rAT_PercentSNCNT         ( UBYTE c_id,
                                       ULONG octets_uplink,
                                       ULONG octets_downlink,
                                       ULONG packets_uplink,
                                       ULONG packets_downlink );
EXTERN void rAT_PercentCGREG         ( T_P_CGREG_STAT stat, USHORT lac, USHORT ci, BOOL bActiveContext );
#ifdef REL99
EXTERN void rAT_PlusCGCMOD           ( void ); /* added for rel99 */
#endif
EXTERN void rAT_PercentCGEV           ( T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param );
#endif

/*--------------- call-backs for AT CI ----------------------------*/

#ifdef CMH_F_C
/*lint -save -e18 */ /* Turn off Lint errors for this "construct" */
EXTERN void rCI_PlusCGACT            ( void );
EXTERN void rCI_PlusCGDATA           ( void );
EXTERN void rCI_PlusCGANS            ( void );
EXTERN void rCI_PlusCGEREP           ( void );
EXTERN void rCI_PlusCGREG            ( void );
EXTERN void rCI_changedQOS           ( void );
EXTERN void rCI_PercentSNCNT         ( void );
EXTERN void rCI_PercentCGREG         ( void );
#ifdef REL99
EXTERN void rCI_PlusCGCMOD           ( void ); /* rel99 */
#endif /* REL99 */
EXTERN void rCI_PercentCGEV           ( void );
/*lint -restore */
#else
EXTERN void rCI_PlusCGACT            ( SHORT link_id );
EXTERN void rCI_PlusCGDATA           ( SHORT link_id );
EXTERN void rCI_PlusCGANS            ( SHORT link_id );
EXTERN void rCI_PlusCGEREP           ( T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param );
EXTERN void rCI_PlusCGREG            ( T_CGREG_STAT stat, USHORT lac, USHORT ci );
EXTERN void rCI_changedQOS           ( SHORT cid, T_PS_qos *qos );
EXTERN void rCI_PercentSNCNT         ( UBYTE c_id,
                                       ULONG octets_uplink,
                                       ULONG octets_downlink,
                                       ULONG packets_uplink,
                                       ULONG packets_downlink );
EXTERN void rCI_PercentCGREG         ( T_P_CGREG_STAT stat, USHORT lac, USHORT ci, BOOL bActiveContext );
#ifdef REL99
EXTERN void rCI_PlusCGCMOD           ( void ); /* rel99 */
#endif /* REL99 */
EXTERN void rCI_PercentCGEV           ( T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param );
#endif

#endif
/*==== EOF ========================================================*/
