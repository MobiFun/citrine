/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_MM
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
|             Mobility Management ( MM )
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_MM_H
#define PSA_MM_H


/*==== CONSTANTS ==================================================*/
#define VLD_PLMN    (0x01)          /* marks a valid PLMN */
#define INVLD_PLMN  (0xFF)          /* marks an invalid PLMN */
#define INVLD_TZ    (0xFF)          /* marks an invalid time zone */

#define DEF_REG_MODE (MODE_AUTO)  /* default registration mode */

#define TEST_STR_LEN  (80)        /* maximum length of test parameter string */

#define NO_ENTRY  (-1)    /* not a valid entry */

typedef enum              /* registration status */
{
  NO_VLD_RS = 0,          /* not a valid registration status */
  RS_NO_SRV,              /* registration status no service */
  RS_LMTD_SRV,            /* registration status limited service */
  RS_FULL_SRV             /* registration status full service */
} T_MM_RGST;

/*==== TYPES ======================================================*/

/*
 * Compare this with the similiar struct T_pnn in psa_sim.h
 */
typedef struct _T_pnn_name
{
  T_plmn plmn;                            /* serving network PLMN */
  USHORT lac;                             /* serving network LAC */
  UBYTE long_len;                         /* length of operator long name */
  UBYTE long_ext_dcs;                     /* octet 3 of IEI */
  UBYTE long_name [MAX_LONG_OPER_LEN-1];  /* long name for operator */
  UBYTE shrt_len;                         /* length of operator short name */
  UBYTE shrt_ext_dcs;                     /* octet 3 of IEI */
  UBYTE shrt_name [MAX_SHRT_OPER_LEN-1];  /* short name for operator */
  UBYTE pnn_rec_num;                      /* the PNN record this data is read from */
  struct _T_pnn_name* next;               /* pointer to next pnn in linked list */
} T_pnn_name;

typedef struct T_COPN_VAR
{
  SHORT Start_Count;
}T_COPN_VAR;

typedef struct MMShrdParm
{
  T_OWN  owner;                   /* identifies the used set */
  T_plmn slctPLMN;                /* selected PLMN */
  USHORT srchRslt;                /* network search result */
  USHORT deregCs;                 /* deregistration cause from NW */
  UBYTE  nrgCs;                   /* deregistration cause from ME */
  UBYTE  regStat;                 /* registration status */
  T_ACI_COPS_MOD  COPSmode;       /* registration mode in COPS type */
  UBYTE  regMode;                 /* registration mode */
  BOOL   regModeAutoBack;         /* TRUE if AT+COPS=4 and manual mode */
  T_plmn PLMNLst[MAX_PLMN_ID];    /* list of found PLMN */
  USHORT LACLst[MAX_PLMN_ID];     /* LAC list (for EONS)*/
  UBYTE  FRBLst[MAX_PLMN_ID];     /* list of forbidden status */
  T_plmn usedPLMN;                /* PLMN in use */
  T_plmn ActingHPLMN;             /* Acting HPLMN*/
  U16    lac;                     /* current lac */
  U16    cid;                     /* current cell id */ 
  T_ACI_CREG_STAT creg_status;    /* current registration status GSM */
  UBYTE  tz;                      /* time zone */ 
  T_pnn_name PNNLst;              /* PLMN Network Name Linked List (for EONS)*/
  BYTE   pnn_nr[MAX_PLMN_ID];     /* PLMN list and PNN record number assocation */
  BYTE   pnn_read_cnt;            /* counter for PNN SIM reading */
  UBYTE  regModeBeforeAbort;      /* This variable is added to keep track of actual registration mode. 
                                     This is used to reset mode in case AT+COPS command is aborted*/
  T_ACI_COPS_MOD  COPSmodeBeforeAbort;
  T_COPN_VAR COPN_VAR[OWN_SRC_MAX];   /* holds start count for various sources */
} T_MM_SHRD_PRM;

/*==== PROTOTYPES =================================================*/

#if !defined (DTI)|| !defined(GPRS)
GLOBAL SHORT psaMM_Registrate  ( void );
GLOBAL SHORT psaMM_DeRegistrate( void );
GLOBAL SHORT psaMM_NetSrch     ( void );
GLOBAL SHORT psaMM_NetSel      ( void );
GLOBAL SHORT psaMM_SetRegMode  ( UBYTE mode );

#endif  /* !DTI */

void  psaMM_Init        ( void );
void  psaMM_CpyPLMNLst  ( T_plmn * pPLMNLst, UBYTE * pFRBLst, USHORT * pLACLst );
void  psaMM_ClrPLMNLst  ( void );

#ifdef TRACING
void  psaMM_shrPrmDump  ( void );
#endif /* TRACING */

/*==== EXPORT =====================================================*/

#ifdef PSA_MMF_C

GLOBAL T_MM_SHRD_PRM mmShrdPrm;

#else

EXTERN T_MM_SHRD_PRM mmShrdPrm;

#endif /* PSA_MMF_C */

 

#endif /* PSA_MM_H */

/*==== EOF =======================================================*/
