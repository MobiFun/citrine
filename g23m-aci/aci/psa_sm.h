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
|             GPRS session management ( SM ).
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef PSA_SM_H
#define PSA_SM_H



/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/
typedef struct SMSetParm
{
  T_SMREG_PDP_ACTIVATE_REQ *pact;    /* primitive parameter */
  T_SMREG_PDP_ACTIVATE_REQ act;     /* primitive parameter */
} T_SM_SET_PRM;

typedef struct SMShrdParm
{
  /* state parameter */
  UBYTE             direc;          /* parameter for primitive SMREG_PDP_ACTIVATE_REQ */

  /* set parameter */
  UBYTE             owner;          /* identifies the used set */
  T_SM_SET_PRM setPrm[OWN_SRC_MAX];     /* possible sets */

  /* answer parameter */
  T_SMREG_PDP_ACTIVATE_CNF *pdp_cnf;       /* primitive parameter                  */
  T_SMREG_PDP_ACTIVATE_REJ *pdp_rej;       /* primitive parameter                  */
  T_SMREG_PDP_ACTIVATE_IND  act_ind;       /* primitive parameter                  */
  T_CAUSE_ps_cause         mod_rej_cause; /* smreg_pdp_modify_rej cause parameter */
  USHORT                    nsapi_set;     /* 1 - 15 N-SAPI - ...                  */
  T_PS_ctrl_qos             ctrl_mod_qos;  /*                                      */
  T_PS_qos                  mod_qos;       /* QOS - modify indication              */
} T_SM_SHRD_PRM;

/*==== PROTOTYPES =================================================*/

void  psaSM_Init            ( void );

#ifdef DTI
EXTERN void psaSM_smreg_configure_req(U8 rat, U8 cn_release);
EXTERN void psaSM_PDP_Deactivate        ( USHORT nsapi_set, 
                                          UBYTE  smreg_local );
EXTERN void psaSM_smreg_pdp_activate_req( U8  cid,                                          
                                          U8  hcomp,
                                          U8  dcomp);
#ifdef REL99
EXTERN void psaSM_smreg_pdp_activate_sec_req( U8  cid );
#endif /* REL99 */

EXTERN void psaSM_PDP_Modify( void );
EXTERN void psaSM_PDP_No_activate ( UBYTE smreg_ti, USHORT smreg_cause );
#endif /*DTI*/
EXTERN void psaSM_ActivateReq ( SHORT cid,
                                UBYTE hcomp,
                                ULONG dti_linkid, 
                                ULONG dti_neighbor, 
                                UBYTE dti_direction );

/*==== EXPORT =====================================================*/

#ifdef PSA_SMF_C

GLOBAL T_SM_SHRD_PRM smShrdPrm;

#else

EXTERN T_SM_SHRD_PRM smShrdPrm;

#endif /* PSA_SMF_C */

#endif /* PSA_SM_H */

#endif  /* GPRS */
/*==== EOF =======================================================*/
