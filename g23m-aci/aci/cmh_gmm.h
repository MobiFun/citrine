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
|             GPRS Mobility Mangement ( GMM ).
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef CMH_GMM_H
#define CMH_GMM_H

/*==== CONSTANTS ==================================================*/
 
 
/*==== TYPES ======================================================*/
typedef enum
{
  GMMH_CMD_OK,
  GMMH_CMD_AT_NOT_HANDLED

} T_GMMH_RETURN;

typedef enum 
{
  GMMU_AT_GPRS,
  GMMU_AT_GSM

} T_GMM_USER;

typedef struct
{
  T_PERCENT_CGCLASS current;
  T_PERCENT_CGCLASS requested;

} T_GACI_MOBILE_CLASS;

/*==== PROTOTYPES =================================================*/
EXTERN void  cmhGMM_Init      ( void );
EXTERN void  cmhGMM_Attached  ( UBYTE attach_type, T_plmn *plmn, UBYTE search_running );
EXTERN void  cmhGMM_NoAttach  ( UBYTE detach_type, USHORT cause, UBYTE search_running );
EXTERN void  cmhGMM_Detached  ( UBYTE detach_type );
EXTERN void  cmhGMM_NetDetach ( UBYTE detach_type, USHORT cause, UBYTE search_running );
EXTERN SHORT cmhGMM_Plmn      ( void );
EXTERN SHORT cmhGMM_Suspend   ( UBYTE cell_state );
EXTERN SHORT cmhGMM_Resume    ( void );
EXTERN SHORT cmhGMM_Info      ( void );
EXTERN SHORT cmhGMM_CipheringInd ( UBYTE gsm_ciph, UBYTE gprs_ciph );


EXTERN T_ATTACH_STATE cmhGMM_Get_state  ( void );
EXTERN void           cmhGMM_Set_state  ( T_ATTACH_STATE state );
EXTERN T_ACI_RETURN   cmhGMM_attach_if_necessary ( T_ACI_CMD_SRC srcId, T_ACI_AT_CMD curCmd );
EXTERN SHORT          cmhGMM_inform_other_psa    ( SHORT state );

EXTERN UBYTE          cmhGMM_requested_attach_type ( T_GMM_USER usr, UBYTE m_class );
EXTERN UBYTE          cmhGMM_translate_gmm_cause_to_nreg_cs ( USHORT cause );
EXTERN BOOL           cmhGMM_isClassCG ( void );


EXTERN SHORT psaG_MM_CMD_REG            ( void );
EXTERN SHORT psaG_MM_CMD_DEREG          ( UBYTE  detach_type );
EXTERN SHORT psaG_MM_CMD_NET_SRCH       ( void );
EXTERN SHORT psaG_MM_CMD_NET_SEL        ( void );
EXTERN SHORT psaG_MM_CMD_SET_REGMD      ( UBYTE mode );

EXTERN SHORT gprs_sim_inserted          ( void );
EXTERN BOOL  cmhGMM_class_eq_CG         ( void );
EXTERN T_ACI_RETURN cmhGMM_attach       ( UBYTE mobile_class );

EXTERN void cmhGMM_send_percentNetworkRegistrationStatus( T_P_CGREG_STAT p_cgreg );
EXTERN void cmhGMM_send_NetworkRegistrationStatus( T_CGREG_STAT cgreg, T_P_CGREG_STAT p_cgreg );
EXTERN UBYTE cmhGMM_isPLMN_Home( void );
EXTERN UBYTE cmhGMM_isPLMN_Valid (void);
EXTERN void cmhGMM_setPLMN(T_plmn *plmn);


EXTERN void  cmhGMM_allContextsDeactivated  ( void );
EXTERN T_ACI_RETURN cmhGMM_contextActivation( T_ACI_CMD_SRC srcId, T_ACI_AT_CMD curCmd );

/*==== EXPORT =====================================================*/
#ifdef CMH_GMMF_C

GLOBAL T_ENT_STAT gmmEntStat;
GLOBAL T_GACI_MOBILE_CLASS gaciMobileClass;

#else

EXTERN T_ENT_STAT gmmEntStat;
EXTERN T_GACI_MOBILE_CLASS gaciMobileClass;

#endif /* CMH_GMMF_C */

#endif /* CMH_GMM_H */


#endif  /* GPRS */
/*==== EOF =======================================================*/
