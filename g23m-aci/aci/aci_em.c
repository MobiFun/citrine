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
|  Purpose :  This Module defines the engineering mode (EM) device driver for the 
|             G23 protocol stack. This driver is used to control all engineering 
|             mode related functions.
+----------------------------------------------------------------------------- 
*/ 

#ifndef ACI_EM_C
#define ACI_EM_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*===== INCLUDES ===================================================*/

#include "aci_cmh.h"

#ifdef UART
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#ifdef _SIMULATION_
#ifdef UART
#include "psa_uart.h"
#endif
#endif

#include "ati_cmd.h"
#include "aci_mem.h"

#ifdef FF_ATI
#include "aci_io.h"
#endif

#include "aci.h"

#include "aci_em.h"

#ifdef GPRS
#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "cmh.h"
#include "cmh_sm.h"
#endif


/*==== CONSTANTS ==================================================*/
/*==== PRIVAT =====================================================*/
/*==== EXPORT =====================================================*/

EXTERN UBYTE em_l1_sem_buffer [];
EXTERN UBYTE em_l1_sem_index;
EXTERN UBYTE em_dl_sem_buffer [];
EXTERN UBYTE em_dl_sem_index;
EXTERN UBYTE em_rr_sem_buffer [];
EXTERN UBYTE em_rr_sem_index;

EXTERN void  em_l1_sem_read (void);
EXTERN void  em_l1_sem_reset(void);
EXTERN void  em_dl_sem_read (void);
EXTERN void  em_dl_sem_reset(void);
EXTERN void  em_rr_sem_read (void);
EXTERN UBYTE em_rr_sem_reset(void);

EXTERN UBYTE em_mm_event_buffer[EM_MM_BUFFER_SIZE];
EXTERN UBYTE em_mm_buffer_write;
EXTERN UBYTE em_cc_event_buffer[EM_CC_BUFFER_SIZE];
EXTERN UBYTE em_cc_buffer_write;
EXTERN UBYTE em_ss_event_buffer[EM_SS_BUFFER_SIZE];
EXTERN UBYTE em_ss_buffer_write;
EXTERN UBYTE em_sms_event_buffer[EM_SMS_BUFFER_SIZE];
EXTERN UBYTE em_sms_buffer_write;
EXTERN UBYTE em_sim_event_buffer[EM_SIM_BUFFER_SIZE];
EXTERN UBYTE em_sim_buffer_write;

EXTERN void  em_aci_sem (UBYTE entity, UBYTE *buffer, UBYTE buf_index_tmp);

#ifndef WIN32
EXTERN const CHAR* l1_version(void);
EXTERN const CHAR* dl_version(void);
EXTERN const CHAR* rr_version(void);
EXTERN const CHAR* mm_version(void);
EXTERN const CHAR* cc_version(void);
EXTERN const CHAR* ss_version(void);
EXTERN const CHAR* sim_version(void);
EXTERN const CHAR* sms_version(void);
/*EXTERN const CHAR* aci_version(void);*/
#endif

/*==== VARIABLES ==================================================*/

/*  
*  callback for the single infrastructure and mobile data functions.
*/
static drv_SignalCB_Type_EM  em_para_signal_callback         = NULL;
static drv_SignalCB_Type_EM  em_para_signal_sc_callback      = NULL;
static drv_SignalCB_Type_EM  em_para_signal_sc_gprs_callback = NULL;
static drv_SignalCB_Type_EM  em_para_signal_nc_callback      = NULL;
static drv_SignalCB_Type_EM  em_para_signal_loc_callback     = NULL;
static drv_SignalCB_Type_EM  em_para_signal_plmn_callback    = NULL;
static drv_SignalCB_Type_EM  em_para_signal_cip_callback     = NULL;
static drv_SignalCB_Type_EM  em_para_signal_pow_callback     = NULL;
static drv_SignalCB_Type_EM  em_para_signal_id_callback      = NULL;
static drv_SignalCB_Type_EM  em_para_signal_ver_callback     = NULL;
static drv_SignalCB_Type_EM  em_para_signal_gmm_callback     = NULL;
static drv_SignalCB_Type_EM  em_para_signal_grlc_callback    = NULL;
static drv_SignalCB_Type_EM  em_para_signal_amr_callback     = NULL;
static drv_SignalCB_Type_EM  em_para_signal_pdp_callback     = NULL;

/*  
*  callback for the event trace function.
*/
static drv_SignalCB_Type_EM_EVENT  em_event_signal_callback  = NULL;

/*  
*  These flags indicates if the first event for the corresponding entity occurred. It is only used to
*  prevent unnecessary semaphor copying.
*/
static UBYTE em_l1_trace;
static UBYTE em_dl_trace;
static UBYTE em_rr_trace;
static UBYTE em_mm_trace;
static UBYTE em_cc_trace;
static UBYTE em_ss_trace;
static UBYTE em_sms_trace;
static UBYTE em_sim_trace;

/*  
*  The buffer keeps all information about index and length of the single event trace data until it is
*  passed to the originator of the em_Read_Event_Parameter() call.
*/
static T_EM_EVENT_BUF aci_em_buf;

static UBYTE   drv_enabled;

static UBYTE em_aci_buf_index;

GLOBAL USHORT em_relcs = 0;

/*==== FUNCTIONS ==================================================*/
LOCAL UBYTE em_class_infra_data       (UBYTE em_subclass, UBYTE em_type);
LOCAL UBYTE em_class_mobile_data      (UBYTE em_subclass, UBYTE em_type);
LOCAL UBYTE em_class_event_tracing    (UBYTE em_subclass, ULONG bitmask_h, ULONG bitmask_l);

LOCAL UBYTE em_subclass_sc            (UBYTE em_type);
LOCAL UBYTE em_subclass_sc_gprs       (UBYTE em_type);
LOCAL UBYTE em_subclass_nc            (UBYTE em_type);
LOCAL UBYTE em_subclass_loc_pag       (UBYTE em_type);
LOCAL UBYTE em_subclass_plmn          (UBYTE em_type);
LOCAL UBYTE em_subclass_ciph_hop_dtx  (UBYTE em_type);
LOCAL UBYTE em_subclass_power         (UBYTE em_type);
LOCAL UBYTE em_subclass_id            (UBYTE em_type);
LOCAL UBYTE em_subclass_version       (UBYTE em_type);
LOCAL UBYTE em_subclass_gmm           (UBYTE em_type);
LOCAL UBYTE em_subclass_grlc          (UBYTE em_type);
LOCAL UBYTE em_subclass_amr           (UBYTE em_type);
#ifdef GPRS
LOCAL UBYTE em_subclass_pdp           (UBYTE em_type);
LOCAL void  em_pco_pdp_trace_req      (void );
#endif

LOCAL UBYTE em_event_l1               (USHORT bitmask_h, ULONG bitmask_l);
LOCAL UBYTE em_event_dl               (USHORT bitmask_h);
LOCAL UBYTE em_event_rr               (USHORT bitmask_h, ULONG bitmask_l);
LOCAL UBYTE em_event_mm               (ULONG  bitmask_h);
LOCAL UBYTE em_event_cc               (ULONG  bitmask_h, ULONG bitmask_l);
LOCAL UBYTE em_event_ss               (USHORT bitmask_h);
LOCAL UBYTE em_event_sms              (ULONG  bitmask_h, ULONG bitmask_l);
LOCAL UBYTE em_event_sim              (ULONG  bitmask);

LOCAL void em_sw_ver_info_cnf (T_EM_SW_VER *version);


/*==== FUNCTIONS ==================================================*/

/*
+------------------------------------------------------------------------------
|  Function     :  em_Init
+------------------------------------------------------------------------------
|  Description  :  The function initializes the driver´s internal data. The 
|                  function returns DRV_OK in case of a successful completition. 
|                  The function returns DRV_INITIALIZED if the driver has already 
|                  been initialized and is ready to be used or is already in use. 
|                  In case of an initialization failure, which means that the 
|                  driver cannot be used, the function returns DRV_INITFAILURE.
|                  This function handles unsupported primitives.
|
|  Parameters   :  Callback function
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_Init (drv_SignalCB_Type_EM in_SignalCBPtr, drv_SignalCB_Type_EM_EVENT in_SignalEventCBPtr)
{
 TRACE_FUNCTION("em_Init ()");

 em_para_signal_callback  = in_SignalCBPtr;      /* store call-back function - Data */
 em_event_signal_callback = in_SignalEventCBPtr; /* store call-back function - Event */

 em_relcs = 0;
 
 if (drv_enabled EQ FALSE) {                  /* EM not initialized yet */
   drv_enabled = TRUE;
   return DRV_OK;
 }
 else {
   return DRV_INITIALIZED;
 }
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_Exit
+------------------------------------------------------------------------------
|  Description  :  The function is used to indicate that the driver and its 
|                  functionality isn´t needed anymore.
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void em_Exit (void)
{
 TRACE_FUNCTION("em_Exit ()");

 em_para_signal_callback = NULL;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_Read_Data_Parameter
+------------------------------------------------------------------------------
|  Description  :  The function is used to indicate that the driver and its 
|                  functionality isn´t needed anymore.
|
|  Parameters   :  UBYTE em_class              
|                  UBYTE em_subclass
|                  UBYTE em_type
|                  void (*cbfunc)(T_DRV_SIGNAL_EM * Signal)
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_Read_Data_Parameter (UBYTE em_class, UBYTE em_subclass, UBYTE em_type, 
                                     void (*cbfunc)(T_DRV_SIGNAL_EM * Signal))
{
 TRACE_FUNCTION("em_Read_Data_Parameter ()");

  /*
   Used to store the callback-address for the different sub-functions. If the same infrastructure data is 
   requested a second time before the first one is served only the latest one is handled.
  */
  switch (em_subclass)
  {
    case EM_SUBCLASS_SC:
      em_para_signal_sc_callback      = cbfunc;
      break;
    case EM_SUBCLASS_SC_GPRS:
      em_para_signal_sc_gprs_callback = cbfunc;
      break;
    case EM_SUBCLASS_NC:
      em_para_signal_nc_callback      = cbfunc;
      break;
    case EM_SUBCLASS_LOC_PAG:
      em_para_signal_loc_callback     = cbfunc;
      break;
    case EM_SUBCLASS_PLMN:
      em_para_signal_plmn_callback    = cbfunc;
      break;
    case EM_SUBCLASS_CIPH_HOP_DTX:
      em_para_signal_cip_callback     = cbfunc;
      break;
    case EM_SUBCLASS_POWER:
      em_para_signal_pow_callback     = cbfunc;
      break;
    case EM_SUBCLASS_ID:
      em_para_signal_id_callback      = cbfunc;
      break;
    case EM_SUBCLASS_SW_VERSION:
      em_para_signal_ver_callback     = cbfunc;
      break;
    case EM_SUBCLASS_GMM:
      em_para_signal_gmm_callback     = cbfunc;
      break;
    case EM_SUBCLASS_GRLC:
      em_para_signal_grlc_callback    = cbfunc;
      break;
    case EM_SUBCLASS_AMR:
      em_para_signal_amr_callback     = cbfunc;
      break;
    case EM_SUBCLASS_PDP:
      em_para_signal_pdp_callback     = cbfunc;
      break;
    default:
      em_para_signal_callback         = NULL;
      break;
  }

  switch (em_class)
  {
    case EM_CLASS_INFRA_DATA:
      return(em_class_infra_data(em_subclass, em_type));
    case EM_CLASS_MOBILE_DATA:
      return(em_class_mobile_data(em_subclass, em_type));
    default:
      return EM_INVALID_CLASS;
  }
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_Read_Event_Parameter
+------------------------------------------------------------------------------
|  Description  :  The function is used to indicate that the driver and its 
|                  functionality isn´t needed anymore.
|
|  Parameters   :  UBYTE entity 
|                  (*cbfunc)(T_DRV_SIGNAL_EM_EVENT * Signal)
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_Read_Event_Parameter (UBYTE entity, void (*cbfunc)(T_DRV_SIGNAL_EM_EVENT * Signal))
{
  T_DRV_SIGNAL_EM_EVENT signal_params;
  
  UBYTE  emet1=0,emet2=0,emet3=0,emet4=0,emet5=0,emet6=0,emet7=0,emet8=0;
  UBYTE *event_buf;
  USHORT length_event_buf;
  UBYTE  alr_tmp, dl_tmp, rr_tmp, mm_tmp, cc_tmp, ss_tmp, sms_tmp, sim_tmp;

  TRACE_FUNCTION("em_Read_Event_Parameter ()");

  /*
  *  Used to store the callback-address for the different sub-functions. If the same event trace is 
  *  requested a second time before the first one is served only the latest one is handled.
  */
  em_event_signal_callback = cbfunc;

  em_aci_buf_index = 0;

  /*
  *  length_event_buf indicates the length of all event traces and is used for dynamic
  *  memory allocation. The single values are from the correspondent entities and therefor
  *  defined as global. To ensure that no buffer overflow happens, the actual index is stored
  *  in a temp variable - a new event could occure after the memory is allocated.
  */
  length_event_buf = (alr_tmp=em_l1_sem_index) + (dl_tmp=em_dl_sem_index) +  (rr_tmp=em_rr_sem_index) + 
                     (mm_tmp=em_mm_buffer_write) + (cc_tmp=em_cc_buffer_write) + (ss_tmp=em_ss_buffer_write) + 
                     (sms_tmp=em_sms_buffer_write) + (sim_tmp=em_sim_buffer_write);

  #ifdef WIN32
  length_event_buf = 100;
  #endif /* WIN32 */

  memset(&aci_em_buf, 0, sizeof(T_EM_EVENT_BUF));

  ACI_MALLOC(event_buf, length_event_buf);

  /*
  *  This checks if the entity is set in the bitmask and at least one event
  *  occurred in the corresponding entity. The event flag (em_xx_trace) protects
  *  unnecessary buffer operations.
  */
  emet1 = ( (((entity & 0x0001) > 0) ? TRUE : FALSE) AND em_l1_trace ) ;  /* L1  */
  emet2 = ( (((entity & 0x0002) > 0) ? TRUE : FALSE) AND em_dl_trace ) ;  /* DL  */
  emet3 = ( (((entity & 0x0004) > 0) ? TRUE : FALSE) AND em_rr_trace ) ;  /* RR  */
  emet4 = ( (((entity & 0x0008) > 0) ? TRUE : FALSE) AND em_mm_trace ) ;  /* MM  */
  emet5 = ( (((entity & 0x0010) > 0) ? TRUE : FALSE) AND em_cc_trace ) ;  /* CC  */
  emet6 = ( (((entity & 0x0020) > 0) ? TRUE : FALSE) AND em_ss_trace ) ;  /* SS  */
  emet7 = ( (((entity & 0x0040) > 0) ? TRUE : FALSE) AND em_sms_trace );  /* SMS */
  emet8 = ( (((entity & 0x0080) > 0) ? TRUE : FALSE) AND em_sim_trace );  /* SIM */

  if(emet1)
  {
      em_aci_sem(EM_L1, event_buf, alr_tmp); 
      em_l1_trace = FALSE;
  }
  if(emet2)
  {
      em_aci_sem(EM_DL, event_buf, dl_tmp); 
      em_dl_trace = FALSE;
  }
  if(emet3)
  {
      em_aci_sem(EM_RR, event_buf, rr_tmp);
      em_rr_trace = FALSE;
  }
  if(emet4)
  {
      em_aci_sem(EM_MM, event_buf, mm_tmp); 
      em_mm_trace = FALSE;
  }
  if(emet5)
  {
      em_aci_sem(EM_CC, event_buf, cc_tmp); 
      em_cc_trace = FALSE;
  }
  if(emet6)
  {
      em_aci_sem(EM_SS, event_buf, ss_tmp);
      em_ss_trace = FALSE;
  }
  if(emet7)
  {
      em_aci_sem(EM_SMS, event_buf, sms_tmp); 
      em_sms_trace = FALSE;
  }
  if(emet8)
  {
      em_aci_sem(EM_SIM, event_buf, sim_tmp);
      em_sim_trace = FALSE;
  } 

  memcpy(&signal_params.Data, &aci_em_buf, sizeof(T_EM_EVENT_BUF));
  signal_params.DataLength = length_event_buf;
  signal_params.Pointer    = event_buf;

  if (em_event_signal_callback NEQ NULL) {
  (*em_event_signal_callback)(&signal_params);
  }
  
  ACI_MFREE(event_buf);

  return TRUE;
}
                                
/*
+------------------------------------------------------------------------------
|  Function     :  em_Set_EventTrace
+------------------------------------------------------------------------------
|  Description  :  Set the event flags and the callback function for the subclass
|
|  Parameters   :  subclass  - defines the subclass the data is coming from
|                  bitmask_h - bitmask 
|                  bitmask_l - bitmask 
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/
GLOBAL UBYTE em_Set_EventTrace (UBYTE em_subclass, ULONG bitmask_h, ULONG bitmask_l)
{
 TRACE_FUNCTION("em_Set_EventTrace ()");

 return(em_class_event_tracing(em_subclass, bitmask_h, bitmask_l));
}


/*
+------------------------------------------------------------------------------
|  Function     :  em_Received_Data
+------------------------------------------------------------------------------
|  Description  :  Compose the callback function
|
|  Parameters   :  data - requested data
          subclass - defines the subclass the data is coming from
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void em_Received_Data (void *data, UBYTE subclass)
{
 /*lint -e813*//* info about auto variable size of drv_SignalID_Type_EM*/
 drv_SignalID_Type_EM signal_params; 

 TRACE_FUNCTION("em_Received_Data ()");

 signal_params.SignalType = subclass;

 switch (subclass) {
   case EM_SUBCLASS_SC: {
     memcpy(&signal_params.UserData.sc, data, sizeof (T_EM_SC_INFO_CNF));
     signal_params.DataLength    = (sizeof (signal_params.UserData.sc) + 6); 
     em_para_signal_callback = em_para_signal_sc_callback;
     break; } 
   case EM_SUBCLASS_SC_GPRS: {        
     memcpy(&signal_params.UserData.sc_gprs, data, sizeof (T_EM_SC_GPRS_INFO_CNF));
     signal_params.DataLength    = (sizeof (signal_params.UserData.sc_gprs) + 6); 
     em_para_signal_callback = em_para_signal_sc_gprs_callback;
     break; } 
   case EM_SUBCLASS_NC: {             
     memcpy(&signal_params.UserData.nc, data, sizeof (T_EM_NC_INFO_CNF));
     signal_params.DataLength    = (sizeof (signal_params.UserData.nc) + 6); 
     em_para_signal_callback = em_para_signal_nc_callback;   
     break; } 
   case EM_SUBCLASS_LOC_PAG: {         
     memcpy(&signal_params.UserData.log_pag, data, sizeof (T_EM_LOC_PAG_INFO_CNF));
     signal_params.DataLength    = (sizeof (signal_params.UserData.log_pag) + 6); 
     em_para_signal_callback = em_para_signal_loc_callback;   
     break; } 
   case EM_SUBCLASS_PLMN: {           
     memcpy(&signal_params.UserData.plmn, data, sizeof (T_EM_PLMN_INFO_CNF));
     signal_params.UserData.plmn.rel_cause = em_relcs;
     signal_params.DataLength    = (sizeof (signal_params.UserData.plmn) + 6); 
     em_para_signal_callback = em_para_signal_plmn_callback;   
     break; } 
   case EM_SUBCLASS_CIPH_HOP_DTX: {   
     memcpy(&signal_params.UserData.cip, data, sizeof (T_EM_CIP_HOP_DTX_INFO_CNF));
     signal_params.DataLength    = (sizeof (signal_params.UserData.cip) + 6); 
     em_para_signal_callback = em_para_signal_cip_callback;
     break; } 
   case EM_SUBCLASS_POWER: {          
     memcpy(&signal_params.UserData.power, data, sizeof (T_EM_POWER_INFO_CNF));
     signal_params.DataLength    = (sizeof (signal_params.UserData.power) + 6); 
     em_para_signal_callback = em_para_signal_pow_callback;
     break; } 
   case EM_SUBCLASS_ID: {             
     memcpy(&signal_params.UserData.id, data, sizeof (T_EM_IDENTITY_INFO_CNF));
     signal_params.DataLength    = (sizeof (signal_params.UserData.id) + 6); 
     em_para_signal_callback = em_para_signal_id_callback;
     break; } 
   case EM_SUBCLASS_SW_VERSION: {
     em_sw_ver_info_cnf(&signal_params.UserData.version);
     signal_params.DataLength    = (sizeof (signal_params.UserData.version) + 6); 
     em_para_signal_callback = em_para_signal_ver_callback;
     break; } 
   case EM_SUBCLASS_GMM: {        
     memcpy(&signal_params.UserData.gmm, data, sizeof (T_EM_GMM_INFO_CNF));
     signal_params.DataLength    = (sizeof (signal_params.UserData.gmm) + 6); 
     em_para_signal_callback = em_para_signal_gmm_callback;
     break; } 
   case EM_SUBCLASS_GRLC: {        
     memcpy(&signal_params.UserData.grlc, data, sizeof (T_EM_GRLC_INFO_CNF));
     signal_params.DataLength    = (sizeof (signal_params.UserData.grlc) + 6); 
     em_para_signal_callback = em_para_signal_grlc_callback;
     break; } 
   case EM_SUBCLASS_AMR: {        
     memcpy(&signal_params.UserData.amr, data, sizeof (T_EM_AMR_INFO_CNF));
     signal_params.DataLength    = (sizeof (signal_params.UserData.amr) + 6); 
     em_para_signal_callback = em_para_signal_amr_callback;
     break; }
   case EM_SUBCLASS_PDP: {        
     em_para_signal_callback = em_para_signal_pdp_callback;
     break; }
   default: {
     signal_params.UserData.defaulT  = 0xff; 
     signal_params.DataLength    = (sizeof (signal_params.UserData.defaulT) + 6); 
     em_para_signal_callback = NULL;
     break; } 
  }  
  if (em_para_signal_callback NEQ NULL) {
   (*em_para_signal_callback)(&signal_params);
   }

  if(data NEQ NULL)
    PFREE(data);  
}

LOCAL void em_sw_ver_info_cnf (T_EM_SW_VER *version)
{
  TRACE_FUNCTION ("em_sw_ver_info_cnf()");

  memset (version, 0, sizeof (T_EM_SW_VER));

#ifndef WIN32
  /* The respective functions are auto-generated by the target build only */

  /* Copy the static const strings into the structure */
  strncpy (version->alr,  l1_version(),   MAX_VER-1);
  strncpy (version->dl,   dl_version(),   MAX_VER-1);
  strncpy (version->rr,   rr_version(),   MAX_VER-1);
  strncpy (version->mm,   mm_version(),   MAX_VER-1);
  strncpy (version->cc,   cc_version(),   MAX_VER-1);
  strncpy (version->ss,   ss_version(),   MAX_VER-1);
  strncpy (version->sms,  sms_version(),  MAX_VER-1);
  strncpy (version->sim,  sim_version(),  MAX_VER-1);
#endif
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_class_infra_data
+------------------------------------------------------------------------------
|  Description  :  This function calls the appropriate subfunction for the 
|          requested data (infrastructure data).
|
|  Parameters   :  em_subclass - defines the subclass the data is requested from
|          em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_class_infra_data (UBYTE em_subclass, UBYTE em_type)
{
 TRACE_FUNCTION("em_class_infra_data ()");

  switch (em_subclass)
  {
    case EM_SUBCLASS_SC:
      return (em_subclass_sc(em_type));
    case EM_SUBCLASS_SC_GPRS:
#ifdef GPRS
      return (em_subclass_sc_gprs(em_type));
#else
      return EM_INVALID_SUBCLASS;   
#endif /* GPRS */
    case EM_SUBCLASS_NC:
      return (em_subclass_nc(em_type));      
    case EM_SUBCLASS_LOC_PAG:
      return (em_subclass_loc_pag(em_type));
    case EM_SUBCLASS_PLMN:
      return (em_subclass_plmn(em_type));
    case EM_SUBCLASS_CIPH_HOP_DTX:
      return (em_subclass_ciph_hop_dtx(em_type));
    case EM_SUBCLASS_GMM:
#ifdef GPRS
      return (em_subclass_gmm(em_type));
#else /* GPRS */
      return EM_INVALID_SUBCLASS;   
#endif /* GPRS */
    case EM_SUBCLASS_GRLC:
#ifdef GPRS
      return (em_subclass_grlc(em_type));
#else /* GPRS */
      return EM_INVALID_SUBCLASS;   
#endif /* GPRS */
    case EM_SUBCLASS_AMR:
      return (em_subclass_amr(em_type));
    case EM_SUBCLASS_PDP:
#ifdef GPRS
      return (em_subclass_pdp(em_type));
#else
      return EM_INVALID_SUBCLASS;
#endif
    default:
      return EM_INVALID_SUBCLASS;
  }
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_class_mobile_data
+------------------------------------------------------------------------------
|  Description  :  This function calls the appropriate subfunction for the 
|          requested data (mobile data).
|
|  Parameters   :  em_subclass - defines the subclass the data is requested from
|          em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_class_mobile_data (UBYTE em_subclass, UBYTE em_type)
{
 TRACE_FUNCTION("em_class_mobile_data ()");

 switch (em_subclass)
  {
  case EM_SUBCLASS_POWER:
   return (em_subclass_power(em_type));
  case EM_SUBCLASS_ID:
   return (em_subclass_id(em_type));
  case EM_SUBCLASS_SW_VERSION:
   return (em_subclass_version(em_type)); /* not implemented yet */
  default:
   return EM_INVALID_SUBCLASS;
 }
}


/*
+------------------------------------------------------------------------------
|  Function     :  em_class_event_tracing
+------------------------------------------------------------------------------
|  Description  :  This function calls the appropriate subfunction for the 
|                  requested data (event trace).
|
|  Parameters   :  em_subclass - defines the event subclass
|                  bitmask - defines the single events
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_class_event_tracing (UBYTE em_subclass, ULONG bitmask_h, ULONG bitmask_l)
{
 TRACE_FUNCTION("em_class_event_tracing ()");

  switch (em_subclass)
  {
    case EM_SUBCLASS_LAYER_1:
       return em_event_l1((USHORT)bitmask_h, bitmask_l);   
    case EM_SUBCLASS_DL:
       return em_event_dl((USHORT)bitmask_l);   
    case EM_SUBCLASS_RR:
       return em_event_rr((USHORT)bitmask_h, bitmask_l);   
    case EM_SUBCLASS_MM:
       return em_event_mm(bitmask_l);   
    case EM_SUBCLASS_CC:
       return em_event_cc(bitmask_h, bitmask_l);   
    case EM_SUBCLASS_SS:
       return em_event_ss((USHORT)bitmask_l); 
    case EM_SUBCLASS_SMS:
       return em_event_sms(bitmask_h, bitmask_l);   
    case EM_SUBCLASS_SIM:
       return em_event_sim(bitmask_l);         
    default:
      return EM_INVALID_SUBCLASS;
  }
} 

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_sc
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_subclass_sc (UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_sc()");

  /*
   *-------------------------------------------------------------------
   * create and send primitive
   *-------------------------------------------------------------------
   */
  {
  PALLOC(em_sc_info_req, EM_SC_INFO_REQ); /* T_EM_SC_INFO_REQ */

  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_sc_info_req->data = em_type;
    
  /* the primitive is send to RR via MM */
  PSENDX (RR, em_sc_info_req);
  }
 return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_sc_gprs
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

#ifdef GPRS
LOCAL UBYTE em_subclass_sc_gprs (UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_sc_gprs()");

  /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
  PALLOC(em_sc_gprs_info_req, EM_SC_GPRS_INFO_REQ); /* T_EM_SC_GPRS_INFO_REQ */

  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_sc_gprs_info_req->data = em_type;
    
  /* the primitive is send to GRR via GMM */
  PSEND (hCommGMM, em_sc_gprs_info_req);
  }
 return TRUE;
}
#endif /* GPRS */

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_nc
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_subclass_nc (UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_nc()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
 {
  PALLOC(em_nc_info_req, EM_NC_INFO_REQ); /* T_EM_NC_INFO_REQ */

  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_nc_info_req->data = em_type;
    
  /* the primitive is send to RR via MM */
  PSENDX (RR, em_nc_info_req);
  }
 return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_loc_pag
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_subclass_loc_pag (UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_loc_pag()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
 {
  PALLOC(em_loc_pag_info_req, EM_LOC_PAG_INFO_REQ); /* T_EM_LOC_PAG_INFO_REQ */

  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_loc_pag_info_req->data = em_type;
    
  /* the primitive is send to RR via MM */
  PSENDX (RR, em_loc_pag_info_req);
  }
 return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_plmn
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_subclass_plmn (UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_plmn()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
 {
  PALLOC(em_plmn_info_req, EM_PLMN_INFO_REQ); /* T_EM_PLMN_INFO_REQ */

  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_plmn_info_req->data = em_type;
    
  /* the primitive is send to RR via MM */
  PSENDX (RR, em_plmn_info_req);
  }
 return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_ciph_hop_dtx
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_subclass_ciph_hop_dtx (UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_ciph_hop_dtx()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
 {
  PALLOC(em_cip_hop_dtx_info_req, EM_CIP_HOP_DTX_INFO_REQ); /* T_EM_CIP_HOP_DTX_INFO_REQ */

  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_cip_hop_dtx_info_req->data = em_type;
    
  /* the primitive is send to RR via MM */
  PSENDX (RR, em_cip_hop_dtx_info_req);
  }
 return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_power
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_subclass_power (UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_power()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
 {
  PALLOC(em_power_info_req, EM_POWER_INFO_REQ); /* T_EM_POWER_INFO_REQ */

  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_power_info_req->data = em_type;
    
  /* the primitive is send to RR via MM */
  PSENDX (RR, em_power_info_req);
  }
 return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_id
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_subclass_id (UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_id()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
 {
  PALLOC(em_identity_info_req, EM_IDENTITY_INFO_REQ); /* T_EM_IDENTITY_INFO_REQ */
  
  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_identity_info_req->data = em_type;
    
  /* the primitive is send to RR via MM */
  PSENDX (RR, em_identity_info_req);
  }
 return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_version
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_subclass_version (UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_version()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
 {
  PALLOC(em_sw_version_info_req, EM_SW_VERSION_INFO_REQ); /* T_EM_SW_VERSION_INFO_REQ */

  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_sw_version_info_req->data = em_type;
    
  /* the primitive is send to RR via MM */
  PSENDX (RR, em_sw_version_info_req);
  }
 return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_pco_bitmap
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_pco_bitmap - defines the actual requested data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

UBYTE em_subclass_pco_bitmap (U32 em_pco_bitmap)
{
  TRACE_FUNCTION ("em_subclass_pco_bitmap()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */

#ifdef GPRS
  /*check for requested GMM, GRLC, GRR info*/
  if(em_pco_bitmap & (EM_PCO_GPRS_INFO | EM_PCO_GMM_INFO | EM_PCO_GRLC_INFO))
  {
    PALLOC(em_pco_trace_req, EM_PCO_TRACE_REQ); /* T_EM_PCO_TRACE_REQ */

    em_pco_trace_req->pco_bitmap = em_pco_bitmap;

    /* the primitive is send to GMM*/
    PSENDX(GMM, em_pco_trace_req);
  }
#endif /*GPRS*/
  
  /*check for requested RR info*/
  if(em_pco_bitmap & (EM_PCO_SC_INFO | EM_PCO_NC_INFO | EM_PCO_LOC_PAG_INFO | EM_PCO_IDENTITY_INFO |
          EM_PCO_CIPH_HOP_DTX_INFO | EM_PCO_POWER_INFO | EM_PCO_PLMN_INFO | EM_PCO_SW_VERSION_INFO |
          EM_PCO_AMR_INFO))
  {
    PALLOC(em_pco_trace_req, EM_PCO_TRACE_REQ); /* T_EM_PCO_TRACE_REQ */

    em_pco_trace_req->pco_bitmap = em_pco_bitmap;

    /* the primitive is send to RR*/
    PSENDX(RR, em_pco_trace_req);
  }

#ifdef GPRS
  if(em_pco_bitmap & EM_PCO_PDP_INFO)
  {
    em_pco_pdp_trace_req();
  }
#endif
 return TRUE;
}/*em_subclass_pco_bitmap*/

/*
+------------------------------------------------------------------------------
|  Function     :  em_pco_pdp_trace_req
+------------------------------------------------------------------------------
|  Description  :  This function prints PDP configuration on PCO 
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
#ifdef GPRS
LOCAL void em_pco_pdp_trace_req(void)
{
  UBYTE cid;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;


  TRACE_FUNCTION ("em_pco_pdp_trace_req()");

  TRACE_EVENT_EM_P1("EM_PDP_INFO_REQ: num_ctxts : %d",PDP_CONTEXT_CID_MAX);
        
  for(cid=1;cid<=PDP_CONTEXT_CID_MAX;cid++)
  {
    
    p_pdp_context_node = pdp_context_find_node_from_cid( cid );

    if( p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_INVALID )
    {
      TRACE_EVENT_EM_P1("EM_PDP_INFO_REQ: state : %d",p_pdp_context_node->internal_data.state);
    }
    else
    {
      if ( p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ NAS_is_ipv4 )
      {
        TRACE_EVENT_EM_P4("EM_PDP_INFO_REQ: state:%d,pdp_type:%s,apn:%s,pdp address:%s", 
               p_pdp_context_node->internal_data.state,
               p_pdp_context_node->attributes.pdp_type,
               p_pdp_context_node->attributes.pdp_apn,
               (( p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_ACTIVATED ) OR 
               ( p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_DATA_LINK )) 
               ? p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv4_addr.a4 : 
                 p_pdp_context_node->attributes.pdp_addr.ip_address.ipv4_addr.a4);
      }
      else if ( p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ NAS_is_ipv6 )
      {
        TRACE_EVENT_EM_P4("EM_PDP_INFO_REQ: state:%d,pdp_type:%s,apn:%s,pdp address:%s", 
               p_pdp_context_node->internal_data.state,
               p_pdp_context_node->attributes.pdp_type,
               p_pdp_context_node->attributes.pdp_apn,
               (( p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_ACTIVATED ) OR 
               ( p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_DATA_LINK )) 
               ? p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv6_addr.a6 : 
                 p_pdp_context_node->attributes.pdp_addr.ip_address.ipv6_addr.a6);
      }
    }
  }

}
#endif /* GPRS */

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_gmm
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

#ifdef GPRS
LOCAL UBYTE em_subclass_gmm(UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_gmm()");

  /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
  PALLOC(em_gmm_info_req, EM_GMM_INFO_REQ); /* T_EM_SC_GPRS_INFO_REQ */

  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_gmm_info_req->data = em_type;
    
  /* the primitive is send to GMM */
  PSEND (hCommGMM, em_gmm_info_req);
  }
 return TRUE;
}
#endif /* GPRS */

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_gmm
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

#ifdef GPRS
LOCAL UBYTE em_subclass_grlc(UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_grlc()");

  /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
  PALLOC(em_subclass_grlc, EM_GRLC_INFO_REQ); /* T_EM_SC_GPRS_INFO_REQ */

  /* fill in primitive parameter:  */
  /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
  em_subclass_grlc->data = em_type;
    
  /* the primitive is send to GRLC via GMM */
  PSEND (hCommGMM, em_subclass_grlc);
  }
 return TRUE;
}
#endif /* GPRS */

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_amr
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_subclass_amr (UBYTE em_type)
{
  TRACE_FUNCTION ("em_subclass_amr()");

  /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
    PALLOC(em_amr_info_req, EM_AMR_INFO_REQ); /* T_EM_AMR_INFO_REQ */

    /* fill in primitive parameter:  */
    /* the bitmask em_type is only used to prevent unnecesary use of primitives*/
    em_amr_info_req->data = em_type;
    
    /* the primitive directly to RR */
    PSENDX (RR, em_amr_info_req);
  }
  return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_subclass_pdp
+------------------------------------------------------------------------------
|  Description  :  This function displays PDP output directly since the 
|                  data is available in ACI itself.                   
|
|  Parameters   :  em_type - defines the actual data
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

#ifdef GPRS
LOCAL UBYTE em_subclass_pdp (UBYTE em_type)
{
  
  TRACE_FUNCTION ("em_subclass_pdp()");

  em_Received_Data ((void *)NULL, EM_SUBCLASS_PDP);

  return TRUE;

}
#endif

/*
+------------------------------------------------------------------------------
|  Function     :  em_event_l1
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  bitmask - defines the single events
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_event_l1 (USHORT bitmask_h, ULONG bitmask_l)
{
  TRACE_FUNCTION ("em_event_l1()");

 /* check if only specified events are set in the bitmask */
 if (!(bitmask_h & ~EM_BITMASK_L1_H)) 
 {
  /*
   *-------------------------------------------------------------------
   * create and send primitive
   *-------------------------------------------------------------------
   */
   PALLOC(em_l1_event_req, EM_L1_EVENT_REQ); /* T_EM_L1_EVENT_REQ */

     /* fill in primitive parameter:  */
   em_l1_event_req->bitmask_l1_h = bitmask_h;
   em_l1_event_req->bitmask_l1_l = bitmask_l;

   /* additional information for the mscviewer */
   TRACE_PRIM_TO("PL");
          
   /* the primitive is send to PL */
   PSENDX (PL, em_l1_event_req);
   return TRUE;
 }
 else                      /* wrong bitmask */
  return EM_DATA_NOT_AVAIL;                 
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_event_dl
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  bitmask - defines the single events
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_event_dl (USHORT bitmask)
{
  TRACE_FUNCTION ("em_event_dl()");

 /* check if only specified events are set in the bitmask */
 if (!(bitmask & ~EM_BITMASK_DL)) {
 /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
   PALLOC(em_dl_event_req, EM_DL_EVENT_REQ); /* T_EM_DL_EVENT_REQ */

   /* fill in primitive parameter:  */
   em_dl_event_req->bitmask_dl = bitmask;

   /* additional information for the mscviewer */
   TRACE_PRIM_TO("DL");
          
   /* the primitive is send to DL via MM */
   PSENDX (MM, em_dl_event_req);
   }
  return TRUE;
 }
 else                      /* wrong bitmask */
  return EM_DATA_NOT_AVAIL;                 
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_event_rr
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  bitmask - defines the single events
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_event_rr (USHORT bitmask_h, ULONG bitmask_l)
{
  TRACE_FUNCTION ("em_event_rr()");

 /* check if only specified events are set in the bitmask */
 if (!(bitmask_h & ~EM_BITMASK_RR_H)) {
 /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
   PALLOC(em_rr_event_req, EM_RR_EVENT_REQ); /* EM_RR_EVENT_REQ */

   /* fill in primitive parameter:  */
   em_rr_event_req->bitmask_rr_h = bitmask_h;
   em_rr_event_req->bitmask_rr_l = bitmask_l;

   /* no additional information for the mscviewer is needed here because 
    * RR is the default entity inside the mscview.tbl
   TRACE_PRIM_TO("RR");
   */
       
   /* the primitive is send to RR */
   PSENDX (RR, em_rr_event_req);
   }
  return TRUE;
 }
 else                      /* wrong bitmask */
  return EM_DATA_NOT_AVAIL;                 
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_event_mm
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  bitmask - defines the single events
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_event_mm (ULONG bitmask)
{
  TRACE_FUNCTION ("em_event_mm()");

 /* check if only specified events are set in the bitmask */
 if (!(bitmask & ~EM_BITMASK_MM)) {
 /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
   PALLOC(em_mm_event_req, EM_MM_EVENT_REQ); /* T_EM_MM_EVENT_REQ */

   /* fill in primitive parameter:  */
   em_mm_event_req->bitmask_mm = bitmask;

   /* additional information for the mscviewer */
   TRACE_PRIM_TO("MM");
          
   /* the primitive is send to MM */
   PSENDX (MM, em_mm_event_req);
   }
  return TRUE;
 }
 else                      /* wrong bitmask */
  return EM_DATA_NOT_AVAIL;                 
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_event_cc
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  bitmask - defines the single events
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_event_cc (ULONG bitmask_h, ULONG bitmask_l)
{
  TRACE_FUNCTION ("em_event_cc()");

 /* check if only specified events are set in the bitmask */
 if (!(bitmask_h & ~EM_BITMASK_CC_H)) {
 /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
   PALLOC(em_cc_event_req, EM_CC_EVENT_REQ); /* T_EM_CC_EVENT_REQ */

   /* fill in primitive parameter:  */
   em_cc_event_req->bitmask_cc_h = bitmask_h;
   em_cc_event_req->bitmask_cc_l = bitmask_l;          

   /* additional information for the mscviewer */
   TRACE_PRIM_TO("CC");
          
   /* the primitive is send to CC */
   PSENDX (CC, em_cc_event_req);
   }
  return TRUE;
 }
 else                      /* wrong bitmask */
  return EM_DATA_NOT_AVAIL;                 
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_event_ss
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  bitmask - defines the single events
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/


LOCAL UBYTE em_event_ss (USHORT bitmask)
{
  TRACE_FUNCTION ("em_event_ss()");

 /* check if only specified events are set in the bitmask */
 if (!(bitmask & ~EM_BITMASK_SS)) {
 /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
   PALLOC(em_ss_event_req, EM_SS_EVENT_REQ); /* T_EM_SS_EVENT_REQ */

   /* fill in primitive parameter:  */
   em_ss_event_req->bitmask_ss = bitmask;

   /* additional information for the mscviewer */
   TRACE_PRIM_TO("SS");
          
   /* the primitive is send to SS */
   PSENDX (SS, em_ss_event_req);
   }
  return TRUE;
 }
 else                      /* wrong bitmask */
  return EM_DATA_NOT_AVAIL;                 
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_event_sms
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  bitmask - defines the single events
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_event_sms (ULONG bitmask_h, ULONG bitmask_l)
{
  TRACE_FUNCTION ("em_event_sms()");

 /* check if only specified events are set in the bitmask */
 if (!(bitmask_h & ~EM_BITMASK_SMS_H)) {
 /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
   PALLOC(em_sms_event_req, EM_SMS_EVENT_REQ); /* T_EM_SMS_EVENT_REQ */

   /* fill in primitive parameter:  */
   em_sms_event_req->bitmask_sms_h = bitmask_h;
   em_sms_event_req->bitmask_sms_l = bitmask_l;

   /* additional information for the mscviewer */
   TRACE_PRIM_TO("SMS");
          
   /* the primitive is send to SMS */
   PSENDX (SMS, em_sms_event_req);
   }
  return TRUE;
 }
 else                      /* wrong bitmask */
  return EM_DATA_NOT_AVAIL;                 
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_event_sim
+------------------------------------------------------------------------------
|  Description  :  This function send the appropriate primitive to the involved 
|                  entity. 
|
|  Parameters   :  bitmask - defines the single events
|
|  Return       :  UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE em_event_sim (ULONG bitmask)
{
  TRACE_FUNCTION ("em_event_sim()");

 /* check if only specified events are set in the bitmask */
 if (!(bitmask & ~EM_BITMASK_SIM)) {
 /*
  *-------------------------------------------------------------------
  * create and send primitive
  *-------------------------------------------------------------------
  */
  {
   PALLOC(em_sim_event_req, EM_SIM_EVENT_REQ); /* T_EM_SIM_EVENT_REQ */

   /* fill in primitive parameter:  */
   em_sim_event_req->bitmask_sim = bitmask;

   /* additional information for the mscviewer */
   TRACE_PRIM_TO("SIM");
          
   /* the primitive is send to SIM */
   PSENDX (SIM, em_sim_event_req);
   }
  return TRUE;
 }
 else                      /* wrong bitmask */
  return EM_DATA_NOT_AVAIL;                 
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_aci_sem
+------------------------------------------------------------------------------
|  Description  :  Clear all entries inside the semaphor for event tracing 
|
|  Parameters   :  UBYTE entity 
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void em_aci_sem (UBYTE entity, UBYTE *buffer, UBYTE buf_index_tmp)
{
 TRACE_FUNCTION ("em_aci_sem()");

 /*
 *  aci_em_buf.xxx.index keeps the position of the data inside the buffer
 *  aci_em_buf.xxx.length indicates the length of the data to be copied
 *  both values are used for later processing of the data 
 */
 switch (entity) 
 {
   case (EM_L1):
      em_l1_sem_read();
      memcpy(buffer + em_aci_buf_index, em_l1_sem_buffer, buf_index_tmp);
      aci_em_buf.alr.index  = em_aci_buf_index;
      aci_em_buf.alr.length = buf_index_tmp;
      em_aci_buf_index += buf_index_tmp;
      em_l1_sem_reset ();
      break;
    case (EM_DL):
      em_dl_sem_read();
      memcpy(buffer + em_aci_buf_index, em_dl_sem_buffer, buf_index_tmp);
      aci_em_buf.dl.index  = em_aci_buf_index;
      aci_em_buf.dl.length = buf_index_tmp;         
      em_aci_buf_index += buf_index_tmp;
      em_dl_sem_reset ();
      break;
    case (EM_RR):
      em_rr_sem_read();
      memcpy(buffer + em_aci_buf_index, em_rr_sem_buffer, buf_index_tmp);
      aci_em_buf.rr.index  = em_aci_buf_index;
      aci_em_buf.rr.length = buf_index_tmp;
      em_aci_buf_index += buf_index_tmp;
      em_rr_sem_reset ();
      break;
   case (EM_MM):
      memcpy(buffer + em_aci_buf_index, em_mm_event_buffer, buf_index_tmp);
      aci_em_buf.mm.index  = em_aci_buf_index;
      aci_em_buf.mm.length = buf_index_tmp;
      em_aci_buf_index += buf_index_tmp;
      em_mm_buffer_write = 0;
      break;
   case (EM_CC):
      memcpy(buffer + em_aci_buf_index, em_cc_event_buffer, buf_index_tmp);
      aci_em_buf.cc.index  = em_aci_buf_index;
      aci_em_buf.cc.length = buf_index_tmp;
      em_aci_buf_index += buf_index_tmp;
      em_cc_buffer_write = 0;
      break;
   case (EM_SS):
      memcpy(buffer + em_aci_buf_index, em_ss_event_buffer, buf_index_tmp);
      aci_em_buf.ss.index  = em_aci_buf_index;
      aci_em_buf.ss.length = buf_index_tmp;
      em_aci_buf_index += buf_index_tmp;
      em_ss_buffer_write = 0;
      break;
   case (EM_SMS):
      memcpy(buffer + em_aci_buf_index, em_sms_event_buffer, buf_index_tmp);
      aci_em_buf.sms.index  = em_aci_buf_index;
      aci_em_buf.sms.length = buf_index_tmp;
      em_aci_buf_index += buf_index_tmp;
      em_sms_buffer_write = 0;
      break;
   case (EM_SIM):
      memcpy(buffer + em_aci_buf_index, em_sim_event_buffer, buf_index_tmp);
      aci_em_buf.sim.index  = em_aci_buf_index;
      aci_em_buf.sim.length = buf_index_tmp;
      em_aci_buf_index += buf_index_tmp;
      em_sim_buffer_write = 0;
      break;
   default:
      break;
 } /* switch */
} /* endfunc em_aci_sem */

/*
+------------------------------------------------------------------------------
|  Function     :  em_event_trace_ind
+------------------------------------------------------------------------------
|  Description  :  When event tracing is enabled, the entity sends a notification 
|                  that the first event occurred. The event flag (em_xx_trace) 
|                  protects unnecessary buffer operations.
|
|  Parameters   :  Entity 
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void em_event_trace_ind (T_EM_DATA_IND * em_data_ind)
{
 TRACE_FUNCTION ("em_event_trace_ind()");

 switch (em_data_ind->entity)
 {
  case EM_L1:
   em_l1_trace = TRUE;
   break;
  case EM_DL:
   em_dl_trace = TRUE;
   break;
  case EM_RR:
   em_rr_trace = TRUE;
   break;
  case EM_MM:
   em_mm_trace = TRUE;
   break;
  case EM_CC:
   em_cc_trace = TRUE;
   break;
  case EM_SS:
   em_ss_trace = TRUE;
   break;
  case EM_SMS:
   em_sms_trace = TRUE;
   break;
  case EM_SIM:
   em_sim_trace = TRUE;
   break;
  default:
   break;
 } /* switch */
 PFREE(em_data_ind);
}


/*+++++++++++++++++++++++++++++++++++++++++ E O F +++++++++++++++++++++++++++++++++++++++++*/
