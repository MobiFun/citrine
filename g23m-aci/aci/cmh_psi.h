/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_PKTIO
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
|             psi managment.
+----------------------------------------------------------------------------- 
*/ 
#ifdef FF_PSI

#ifndef CMH_PSI_H
#define CMH_PSI_H


#include "aci_lst.h"

/*==== CONSTANTS ==================================================*/
  
#define ACI_DIO_ID_MASK 0x0000FF00  /* bit 08-15: device combination id from DIO Interface layer, v4 */

/*==== TYPES ======================================================*/
typedef enum{
   PSI_DTI_CLS_CNF,
   PSI_DTI_CLS_IND
}T_PSI_DTI_CLS_TYP;

typedef union
{
  T_DIO_CAP_SER             dio_cap_ser;    
  T_DIO_CAP_SER_MUX         dio_cap_ser_mux;   
  T_DIO_CAP_PKT             dio_cap_pkt;       
} T_DIO_CAP_PSI;
typedef T_DIO_CAP_PSI T_ACI_PSI_CAP_PAR;

typedef union 
{
  T_DIO_DCB_SER  dio_cap_ser;
  T_DIO_DCB_SER_MUX  dio_cap_ser_mux;
  T_DIO_DCB_PKT  dio_cap_pkt;
} T_ACI_PSI_CAP;

typedef union 
{
  T_DIO_DCB_SER  dio_dcb_ser;
  T_DIO_DCB_SER_MUX  dio_dcb_ser_mux;
  T_DIO_DCB_PKT  dio_dcb_pkt;
} T_ACI_PSI_DCB_PAR; 

typedef struct {
  U32               devId;
 T_ACI_PSI_DCB_PAR  dcbPar;
} T_ACI_DEVICE_DCB_ENTRY;

typedef struct
{
    U32 devId;
    T_ACI_PSI_CAP_PAR psi_cap;
    U8 psi_data_mode;
} T_ACI_PSI;

typedef struct
{
  UBYTE               src_id;
  T_DTI_ENTITY_ID     entity_to_conn;
  UBYTE               num_entities;
  T_DTI_CONN_MODE     mode;
  T_DTI_CONN_CB*      cb;
  UBYTE               capability;
  UBYTE               cid;
  T_ACI_AT_CMD        last_cmd;
}T_ACI_PSI_CALL_TYPE;

/*==== PROTOTYPES =================================================*/
EXTERN void cmhPSI_DTI_OpenCnf (U32 devId, U32 link_id, T_DTI_CONN_RESULT result);
EXTERN void cmhPSI_DTI_Close (U32 devId, T_PSI_DTI_CLS_TYP  state, U32 link_id);
EXTERN void cmhPSI_Line_State_Ind (U32 devId, U16 line_state);
EXTERN BOOL cmhPSItest_srcId( UBYTE srcId, void *elem);
EXTERN void cmhPSI_getdata ( T_ACI_CMD_SRC srcId );

EXTERN BOOL cmhPSItest_device (U32 devId, void *elem);
EXTERN void cmhPSI_AddDeviceToDcbParameter( U32 devId, U32 dio_driver_type);
EXTERN void cmhPSI_erase_src_elem(UBYTE srcId);
EXTERN void cmhPSI_RemoveDeviceFromDcbParameter( U32 devId );
EXTERN void cmhPSI_CleanDcbParameterList( void );
EXTERN T_ACI_DEVICE_DCB_ENTRY* cmhPSI_GetDcbParOverDevice( U32 devId );
EXTERN T_ACI_DEVICE_DCB_ENTRY* cmhPSI_GetDeviceParOverSrcID( UBYTE srcId );
EXTERN UBYTE cmhPSI_GetParityOverSrcID( UBYTE srcId );
EXTERN UBYTE cmhPSI_GetStopBitOverSrcID( UBYTE srcId );
EXTERN UBYTE cmhPSI_GetDataBitOverSrcID( UBYTE srcId );
EXTERN void cmhPSI_SetDcbParToUnchanged( T_ACI_DEVICE_DCB_ENTRY *dcbntry );
EXTERN T_ACI_RETURN cmhPSI_SetDataRate ( UBYTE srcId, T_ACI_BD_RATE  rate);
EXTERN T_ACI_RETURN cmhPSI_SetCharacterFraming ( UBYTE srcId, 
                                                  T_ACI_BS_FRM format, 
                                                  T_ACI_BS_PAR parity );
EXTERN void cmhPSI_Ind (T_ACI_PSI *psi_indic_msg, UBYTE src_id,U32 dio_cap);
EXTERN T_ACI_RETURN cmhPSI_SetFlowControl (UBYTE srcId, T_ACI_RX_FLOW_CTRL DCE_by_DTE);
EXTERN T_ACI_RETURN cmhPSI_SetEscape( UBYTE srcId, UBYTE detection );
EXTERN T_ACI_BS_FRM cmhPSI_GetFktInterfaceFormat( T_ACI_DEVICE_DCB_ENTRY *dcbntry );
EXTERN T_ACI_BS_PAR cmhPSI_GetFktInterfaceParity( T_ACI_DEVICE_DCB_ENTRY *dcbntry );
EXTERN T_ACI_DTI_PRC_PSI *cmhPSI_find_dlci (T_ACI_LIST *search_list, U32 devId, UBYTE dlci );
EXTERN void cmhPSI_lst_init (void);
EXTERN void cmhPSI_clean_all_elem(void);
EXTERN T_ACI_LIST *psi_src_params;
EXTERN T_ACI_LIST *psi_dcb_par;

EXTERN  void psaPSI_SetConfReq( T_ACI_DEVICE_DCB_ENTRY *dcbPar );
/*==== EXPORT =====================================================*/
#ifdef CMH_PSIF_C
GLOBAL T_ACI_PSI_CALL_TYPE psi_ato;
#else
EXTERN T_ACI_PSI_CALL_TYPE psi_ato;
#endif  /* CMH_PSIF_C */

#endif /*CMH_PSI_H*/

#endif
/*==== EOF =======================================================*/
