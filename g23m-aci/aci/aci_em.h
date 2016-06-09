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
|  Purpose :  Handles all instances for the Engineering mode (EM).
+----------------------------------------------------------------------------- 
*/ 

#ifndef ACI_EM_H
#define ACI_EM_H

/*
* Return Values
*/
#define EM_INVALID_CLASS           10
#define EM_INVALID_SUBCLASS        11
#define EM_INVALID_TYPE            12
#define EM_INVALID_ACCESS          13
#define EM_NO_MORE_DATA            14
#define EM_DATA_NOT_AVAIL          15

#define MAX_EM_LENGTH              80
 
/*
 * EM classes
 */
#define EM_CLASS_EVENT_TRACING      1
#define EM_CLASS_INFRA_DATA         3
#define EM_CLASS_MOBILE_DATA        4

/*
 * EM Subclasses Event Tracing / Counter
 */
#define EM_SUBCLASS_LAYER_1         1
#define EM_SUBCLASS_DL              2
#define EM_SUBCLASS_RR              3
#define EM_SUBCLASS_MM              4
#define EM_SUBCLASS_CC              5
#define EM_SUBCLASS_SS              6
#define EM_SUBCLASS_SMS             7
#define EM_SUBCLASS_SIM             8

/*
 * EM Subclasses Infrastructure data
 */
#define EM_SUBCLASS_SC              9
#define EM_SUBCLASS_SC_GPRS        10
#define EM_SUBCLASS_NC             11
#define EM_SUBCLASS_LOC_PAG        12
#define EM_SUBCLASS_PLMN           13
#define EM_SUBCLASS_CIPH_HOP_DTX   14
#define EM_SUBCLASS_GMM            15
#define EM_SUBCLASS_GRLC           16
#define EM_SUBCLASS_AMR            20
#define EM_SUBCLASS_PDP            21


/*
 * EM Subclasses Mobile Data   
 */
#define EM_SUBCLASS_POWER          17
#define EM_SUBCLASS_ID             18
#define EM_SUBCLASS_SW_VERSION     19

/*
 * EM bitmask check
*/

#define EM_BITMASK_L1_H            0x007F        /* 39 events */
#define EM_BITMASK_L1_L            0xFFFFFFFFL
#define EM_BITMASK_DL              0x001F        /*  5 events */
#define EM_BITMASK_RR_H            0x001F        /* 37 events */
#define EM_BITMASK_RR_L            0xFFFFFFFFL
#define EM_BITMASK_MM              0x0003FFFFL   /* 18 events */
#define EM_BITMASK_CC_H            0x0001FFFFL   /* 49 events */
#define EM_BITMASK_CC_L            0xFFFFFFFFL
#define EM_BITMASK_SS              0x03FF        /* 10 events */
#define EM_BITMASK_SMS_H           0x00000001L   /* 33 events */
#define EM_BITMASK_SMS_L           0xFFFFFFFFL    
#define EM_BITMASK_SIM             0x01FFFFFFL   /* 25 events */

/*
#define EM_STARTED                 1
#define EM_ESTABLISHED             2
#define EM_FAILED                  3
#define EM_SEND                    4
#define EM_RECEIVED                5
#define EM_ABORTED                 6
#define EM_RELEASED                7
*/

#define EM_DATA_IND_OPC            32267

#define EM_MM_BUFFER_SIZE            168
#define EM_CC_BUFFER_SIZE            185
#define EM_SS_BUFFER_SIZE             31
#define EM_SMS_BUFFER_SIZE           110
#define EM_SIM_BUFFER_SIZE            67
#define EM_MAX_MA_CHANNELS            64

#define MAX_VER 50

#ifdef TRACE_PRIM
  #define TRACE_PRIM_FROM(s)              vsi_o_ttrace(VSI_CALLER TC_PRIM, "Pdir: " s)
  #define TRACE_PRIM_TO(s)                vsi_o_ttrace(VSI_CALLER TC_PRIM, "Pdir: " s)
#else
  #define TRACE_PRIM_FROM(s)           
  #define TRACE_PRIM_TO(s)             
#endif


#define TRACE_EVENT_EM_P1(s,a1)                       TRACE_USER_CLASS_P1(TC_USER8,s,a1)
#define TRACE_EVENT_EM_P4(s,a1,a2,a3,a4)              TRACE_USER_CLASS_P4(TC_USER8,s,a1,a2,a3,a4)

/*
/ *************************************************************************************
 *
 * Structures according to EM Functional interface description (8443.601.01.002)
 *
 *************************************************************************************
 */

/*
 * Unit ID
 */
typedef enum
{
 EM_UNIT_INVALID = -1,
 EM_UNIT_L1      =  1,
 EM_UNIT_DL,
 EM_UNIT_RR,
 EM_UNIT_MM,
 EM_UNIT_CC,
 EM_UNIT_SS,
 EM_UNIT_SMS,
 EM_UNIT_SIM
} T_EM_UNIT_ID;

typedef struct
{
  UBYTE index;
  UBYTE length;
} T_EVENT_PTR;

typedef struct
{
  T_EVENT_PTR alr;
  T_EVENT_PTR dl;
  T_EVENT_PTR rr;
  T_EVENT_PTR mm;
  T_EVENT_PTR cc;
  T_EVENT_PTR ss;
  T_EVENT_PTR sms;
  T_EVENT_PTR sim;
} T_EM_EVENT_BUF;

typedef struct
{
  CHAR alr[MAX_VER];
  CHAR  dl[MAX_VER];
  CHAR  rr[MAX_VER];
  CHAR  mm[MAX_VER];
  CHAR  cc[MAX_VER];
  CHAR  ss[MAX_VER];
  CHAR sms[MAX_VER];
  CHAR sim[MAX_VER];  
} T_EM_SW_VER;


/*------------------------------------------------------------------------
   drv_SignalID_Type - driver signal identification
 
   The type defines the signal information data used to identify a signal. 
   This data type is used to define and to report a signal. A signal is 
   defined by a process calling the driver function drv_SetSignal. An 
   event is signalled by driver by calling the pre-defined signal call-
   back function.
  -------------------------------------------------------------------------*/
typedef struct
{
  UBYTE               SignalType;
  USHORT              DataLength;
  union
  {
    T_EM_SC_INFO_CNF          sc;
    T_EM_SC_GPRS_INFO_CNF     sc_gprs;
    T_EM_NC_INFO_CNF          nc;
    T_EM_LOC_PAG_INFO_CNF     log_pag;
    T_EM_PLMN_INFO_CNF        plmn;
    T_EM_CIP_HOP_DTX_INFO_CNF cip;
    T_EM_POWER_INFO_CNF       power;
    T_EM_IDENTITY_INFO_CNF    id;
    T_EM_SW_VER               version;
    T_EM_GMM_INFO_CNF         gmm;
    T_EM_GRLC_INFO_CNF        grlc;
    T_EM_AMR_INFO_CNF         amr;
    UBYTE                     defaulT;
  } UserData;
} T_DRV_SIGNAL_EM;

typedef struct
{
  UBYTE *             Pointer;
  USHORT              DataLength;
  T_EM_EVENT_BUF      Data;
} T_DRV_SIGNAL_EM_EVENT;


/*------------------------------------------------------------------------
   drv_SignalCB_Type - driver signal device control block
  
   This type defines a call-back function used to signal driver events,
   e.g. driver is ready to accept data. The driver calls the signal
   call-back function when a specific event occurs and the driver has
   been instructed to signal the event to a specific process. A process
   can set or reset event signalling by calling one of the driver 
   functions drv_SetSignal or drv_ResetSignal. Event signalling can only
   be performed when a call-back function has been installed at driver
   initialization.
  -------------------------------------------------------------------------*/
typedef void (*T_DRV_CB_FUNC_EM )      (T_DRV_SIGNAL_EM * Signal);
typedef void (*T_DRV_CB_FUNC_EM_EVENT) (T_DRV_SIGNAL_EM_EVENT * Event);

/*
*  callback functionality for em_Read_Data_Parameter 
*/
#define drv_SignalCB_Type_EM         T_DRV_CB_FUNC_EM
#define drv_SignalID_Type_EM         T_DRV_SIGNAL_EM

/*
*  callback functionality for em_Read_Event_Parameter 
*/
#define drv_SignalCB_Type_EM_EVENT   T_DRV_CB_FUNC_EM_EVENT
#define drv_SignalID_Type_EM_EVENT   T_DRV_SIGNAL_EM_EVENT

#define T_VSI_THANDLE                USHORT
 
/*
 * Prototypes
 */
EXTERN UBYTE em_Read_Data_Parameter  (UBYTE em_class, UBYTE em_subclass, UBYTE em_type, 
                                      void (*cbfunc)(T_DRV_SIGNAL_EM * Signal));
EXTERN UBYTE em_Read_Event_Parameter (UBYTE entity, 
                                      void (*cbfunc)(T_DRV_SIGNAL_EM_EVENT * Signal));
EXTERN void  em_Received_Data   (void *data, UBYTE subclass);
EXTERN UBYTE em_Set_EventTrace  (UBYTE em_subclass, ULONG bitmask_h, ULONG bitmask_l);

EXTERN UBYTE em_Init            (drv_SignalCB_Type_EM in_SignalCBPtr, 
                                 drv_SignalCB_Type_EM_EVENT in_SignalEventCBPtr);
EXTERN void  em_Exit            (void);

EXTERN void  em_aci_sem_clear (void);
EXTERN void  em_aci_sem_init  (void);
EXTERN void  em_aci_sem_exit  (void);

EXTERN UBYTE em_subclass_pco_bitmap (U32 em_pco_bitmap); /* en- or disable PCO-trace*/

/*
EXTERN UBYTE em_SetSignal   (drv_SignalID_Type * in_SignalIDPtr);
EXTERN UBYTE em_ResetSignal (drv_SignalID_Type * in_SignalIDPtr);
EXTERN UBYTE em_SetConfig   (pwr_DCB_Type      * in_DCBPtr);
EXTERN UBYTE em_GetConfig   (pwr_DCB_Type      * out_DCBPtr);
EXTERN UBYTE em_GetStatus   (pwr_Status_Type   * out_StatusPtr);
*/



#endif  /* ACI_EM_H */

/*+++++++++++++++++++++++++++++++++++++++++ E O F +++++++++++++++++++++++++++++++++++++++++*/
