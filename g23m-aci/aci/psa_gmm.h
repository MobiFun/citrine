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
|             GPRS Mobility Management ( GMM ).
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef PSA_GMM_H
#define PSA_GMM_H


/************************************************************************
**                       GMM PRIMITIVE TRACE                           **
************************************************************************/

#define GMM_PRIMITIVE_TRACES GMM_PRIMITIVE_TRACES
//#define GMM_PRIMITIVE_TRACES_ADVANCED GMM_PRIMITIVE_TRACES_ADVANCED

/************************************************************************
**       switching on of and mode over above mentioned defines         **
**       -----------------------------------------------------         **
**       advanced means: a short string output of the parameter        **
**                                                                     **
**       without advanced: only numbers                                **
************************************************************************/

#ifdef GMM_PRIMITIVE_TRACES_ADVANCED

#define PARAMETER_STRING_1 "%s: %s"
#define PARAMETER_STRING_2 "%s: %s %s"

extern char* dbg_attachType(UBYTE attach_type);
extern char* dbg_detachType(UBYTE detach_type);
extern char* dbg_mobileClass(UBYTE mobile_class);
extern char* dbg_searchRunning(UBYTE search_running)
extern char* dbg_cellState(UBYTE cell_state)

#else

# define PARAMETER_STRING_1 "%d"
# define PARAMETER_STRING_2 "%d %d"

# define  dbg_attachType
# define  dbg_detachType
# define  dbg_mobileClass
# define  dbg_searchRunning
# define  dbg_cellState

#endif /* GMM_PRIMITIVE_TRACES_ADVANCED */

#if defined GMM_PRIMITIVE_TRACES
#   define GMM_PRIM_TRACE_1(a,b)    TRACE_EVENT_P1(PARAMETER_STRING_1, b)
#   define GMM_PRIM_TRACE_2(a,b,c)  TRACE_EVENT_P2(PARAMETER_STRING_2, b, c)

#elif defined GMM_PRIMITIVE_TRACES_ADVANCED
#   define GMM_PRIM_TRACE_1(a,b)    TRACE_EVENT_P2(PARAMETER_STRING_1, a, b)
#   define GMM_PRIM_TRACE_2(a,b,c)  TRACE_EVENT_P3(PARAMETER_STRING_2, a, b, c)

#else
# define GMM_PRIM_TRACE_1(a,b)
# define GMM_PRIM_TRACE_2(a,b,c)
#endif  /* GMM_PRIMITIVE_TRACES || GMM_PRIMITIVE_TRACES_ADVANCED */

/************************************************************************
**                       GMM PRIMITIVE TRACE                           **
************************************************************************/


/*==== CONSTANTS ==================================================*/
#define INVLD_PLMN  (0xFF)          /* marks an invalid PLMN */
#define ATTACH_TYPE_DETACHED  0

typedef enum              /* PSA notification events */
{
  GMM_NTF_ATT_CNF = 0,    /* the attach was successful */
  GMM_NTF_ATT_REJ,        /* the attach has failed */
  GMM_NTF_DET_CNF,        /* the network confirmed the detach */
  GMM_NTF_DET_IND,        /* A network initiated detach */
  GMM_NTF_PLMN_IND,       /* indicate a PLMN list */
  GMM_NTF_SUSP_IND,       /*  */
  GMM_NTF_RES_IND,        /*  */
  GMM_NTF_INFO_IND,       /* transmit information */
  GMM_NTF_MAX              /* maximum MM notification event */

} T_GMM_NTF;  /* finished */

typedef enum              /* test parameter identifier */
{
  GMM_OFF = 0,            /* the  */
  GMM_DETACHED,           /* detached */
  GMM_ATTACHED,           /* attached */
  GMM_ATTACHING,          /* will be attached */
  GMM_DETACHING,          /* will be detached */
  GMM_SUSPENTED           /* suspented */

} T_GMM_ATTACH_STATE;

/*==== TYPES ======================================================*/

typedef struct GMMShrdParm
{
  /* state parameter */
  UBYTE   mobile_class;             /* 0: A       1: B      2: C           3: CG      4: CC */
  UBYTE   service_mode;

  /* set parameter */
  UBYTE   requested_mobile_class;   /* 0: A       1: B      2: C           3: CG      4: CC */

  /* answer parameter */
  UBYTE   gprs_status[MAX_PLMN_ID]; /* 0 - 2: which the PLMN supports */
  UBYTE   last_attach_type;
  UBYTE   current_attach_type;
  UBYTE   requested_attach_type;

  /* GPRS registration status parameters */
  T_CGREG_STAT    cgreg_stat;    /* +CGREG status */
  T_P_CGREG_STAT  p_cgreg_stat;  /* %CGREG status */
  USHORT          lac;           /* current cell coordinates */
  USHORT          cid;
  T_ACI_P_CREG_GPRS_IND gprs_indicator; /* %CREG gprs indicator */
  UBYTE           gprs_call_killer; /* This paramter indicates whether the detach is initiated 
                                       by user / not. */
  UBYTE           rt;
} T_GMM_SHRD_PRM;

/*==== PROTOTYPES =================================================*/

EXTERN void psaGMM_Init (UBYTE auto_attach, UBYTE auto_detach, UBYTE mobile_class );
EXTERN void psaGMM_NetworkRegistrationStatus ( ULONG prim, void* para);

EXTERN void psaGMM_Attach         ( UBYTE mobile_class, UBYTE attach_type, UBYTE service_mode );
EXTERN void psaGMM_Detach         ( UBYTE detach_type );
EXTERN void psaGMM_Net_Req        ( void );
EXTERN void psaGMM_Plmn_res       ( UBYTE mobile_class, UBYTE attach_type, T_plmn *plmn );
EXTERN void psaGMM_Plmn_mode_req  ( UBYTE net_selection_mode );
EXTERN void psaGMM_Config_req( UBYTE cipher_on, UBYTE tlli_handling );

/*==== EXPORT =====================================================*/

#ifdef PSA_GMMF_C

GLOBAL T_GMM_SHRD_PRM gmmShrdPrm;

GLOBAL UBYTE                default_mobile_class  = GMMREG_CLASS_BG;
GLOBAL T_CGAATT_ATTACH_MODE automatic_attach_mode;
GLOBAL T_CGAATT_DETACH_MODE automatic_detach_mode;

#else

EXTERN T_GMM_SHRD_PRM gmmShrdPrm;

EXTERN UBYTE  default_mobile_class;
EXTERN T_CGAATT_ATTACH_MODE automatic_attach_mode;
EXTERN T_CGAATT_DETACH_MODE automatic_detach_mode;

#endif /* PSA_GMMF_C */

#endif /* PSA_GMM_H */

#endif  /* GPRS */
/*==== EOF =======================================================*/
