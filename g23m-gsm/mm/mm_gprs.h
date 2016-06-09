/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM
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
|  Purpose :  Definitions for the GPRS functionality of MM
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_GPRS_H
#define MM_GPRS_H

#ifdef  GPRS

/*==== CONSTANTS ===========================================================*/

/*==== TYPES ===============================================================*/
  
/*==== EXPORT ==============================================================*/

/* 
 * Prototypes for the primitive interface MMGMM (GMM to MM) 
 */
EXTERN void mm_mmgmm_reg_req  
              (const T_MMGMM_REG_REQ *mmgmm_reg_req);

EXTERN void mm_mmgmm_nreg_req 
              (const T_MMGMM_NREG_REQ *mmgmm_nreg_req);

EXTERN void mm_mmgmm_net_req  
              (const T_MMGMM_NET_REQ *mmgmm_net_req);

EXTERN void mm_mmgmm_plmn_res 
              (const T_MMGMM_PLMN_RES *mmgmm_plmn_res);

EXTERN void mm_mmgmm_plmn_mode_req 
              (const T_MMGMM_PLMN_MODE_REQ *mmgmm_plmn_mode_req);

EXTERN void mm_mmgmm_auth_rej_req 
              (const T_MMGMM_AUTH_REJ_REQ *mmgmm_auth_rej_req);

EXTERN void mm_mmgmm_cm_establish_res 
              (const T_MMGMM_CM_ESTABLISH_RES *est_res);

EXTERN void mm_mmgmm_attach_started_req 
              (const T_MMGMM_ATTACH_STARTED_REQ *start_req);

EXTERN void mm_mmgmm_attach_acc_req 
              (const T_MMGMM_ATTACH_ACC_REQ *mmgmm_attach_acc_req);

EXTERN void mm_mmgmm_attach_rej_req 
              (const T_MMGMM_ATTACH_REJ_REQ *mmgmm_attach_rej_req);

EXTERN void mm_mmgmm_detach_started_req 
              (const T_MMGMM_DETACH_STARTED_REQ *det_start_req);

EXTERN void mm_mmgmm_start_t3212_req 
              (const T_MMGMM_START_T3212_REQ *start_t3212_req);

EXTERN void mm_mmgmm_cm_emergency_res 
              (const T_MMGMM_CM_EMERGENCY_RES *emerge_res);

EXTERN void mm_mmgmm_allowed_req
              (const T_MMGMM_ALLOWED_REQ *allowed_req);

EXTERN void mm_mmgmm_trigger_req
              (const T_MMGMM_TRIGGER_REQ *trigger_req);

/* 
 * Prototypes from mm_gprs.c. This is the functional interface in GMM to MM 
 * direction. Here all the function prototypes which are needed for GPRS 
 * only are defined.
 */
EXTERN void mm_func_mmgmm_auth_rej_req       (void);
EXTERN void mm_func_mmgmm_cm_establish_res   (UBYTE cm_establish_res);
EXTERN void mm_func_mmgmm_attach_started_req (void);
EXTERN void mm_func_mmgmm_attach_acc_req     (const T_plmn *plmn,
                                              USHORT lac,
                                              UBYTE v_mobile_identity,
                                              ULONG mobile_identity,
                                              UBYTE v_equ_plmn_list,
                                              const T_equ_plmn_list *equ_plmn_list);
EXTERN void mm_func_mmgmm_attach_rej_req     (USHORT cs);
EXTERN void mm_func_mmgmm_detach_started_req (void);
EXTERN void mm_func_mmgmm_start_t3212_req    (void);
EXTERN void mm_func_mmgmm_cm_emergency_res   (UBYTE cm_establish_res);
EXTERN void mm_func_mmgmm_allowed_req        (UBYTE v_equ_plmn_list, 
                                              const T_equ_plmn_list *equ_plmn_list);
EXTERN void mm_func_mmgmm_trigger_req        (void);

/* 
 * Other prototypes from mm_gprs.c
 */
EXTERN void mm_gprs_update_req               (void);
EXTERN void mm_network_initiated_detach      (USHORT cs);
EXTERN void mm_sim_removed_gprs_active       (void);

/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define hCommGMM        _ENTITY_PREFIXED(hCommGMM)
#endif /* OPTION_MULTITHREAD */

EXTERN T_HANDLE  hCommGMM;        /* GMM  Communication       */

#endif /* GPRS */

#endif /* MM_GPRS_H */
