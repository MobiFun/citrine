/*
+-----------------------------------------------------------------------------
|  Project :  ...
|  Modul   :  sap_dti.h
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
|  Purpose :  This modul ...
+-----------------------------------------------------------------------------
*/


#define ACI_CONNECT_DTI                0x00
#define ACI_DISCONNECT_DTI             0x01

#define NOT_READY                      0x00
#define READY                          0x01

typedef struct
{
  BOOL  isDtiConnected;
  UBYTE dtxState;
} T_ACI_SRC_DTI_PARAMS;


/* process functions for DTI Lib */
EXTERN void dti_lib_dti_dti_connect_cnf (
                    T_DTI2_CONNECT_CNF   *dti_connect_cnf );


EXTERN void dti_lib_dti_dti_connect_ind (
                    T_DTI2_CONNECT_IND   *dti_connect_ind );

EXTERN void dti_lib_dti_dti_disconnect_ind (
                    T_DTI2_DISCONNECT_IND   *dti_disconnect_ind );

EXTERN void dti_lib_dti_dti_data_ind (
                    T_DTI2_DATA_IND   *dti_data_ind );

EXTERN void dti_lib_dti_dti_ready_ind (
                    T_DTI2_READY_IND   *dti_ready_ind );

#ifdef _SIMULATION_
EXTERN const void dti_lib_dti_dti_data_test_ind (
                    T_DTI2_DATA_TEST_IND   *dti_data_test_ind );
#endif




EXTERN void psaACI_Init();

EXTERN void aci_pei_sig_callback(U8               instance,
                                 U8               interfac, 
                                 U8               channel,
                                 U8               reason, 
                                 T_DTI2_DATA_IND* dti_data_ind);


/* function for "ACI PSA" */
EXTERN BOOL psaACI_Dti_Req (T_DTI_CONN_LINK_ID link_id, 
                            T_DTI_ENTITY_ID    peer_ent_id, 
                            UBYTE              dti_conn);



#ifdef DTI_CNTRL_MNG_C
T_ACI_SRC_DTI_PARAMS aci_src_dti_params[CMD_SRC_MAX];
DTI_HANDLE aci_hDTI;
#else
EXTERN T_ACI_SRC_DTI_PARAMS aci_src_dti_params[];
EXTERN DTI_HANDLE aci_hDTI;
#endif


/* OLD DTI Manager stuff */
#define ACI_DTI_DN_CHANNEL             0

#define DTI_MNG_ID_NOTPRESENT          0
