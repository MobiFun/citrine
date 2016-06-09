/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM (6301)
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
|  Purpose :  Types definitions for the engineering mode driver
|             .
+----------------------------------------------------------------------------- 
*/ 

#ifndef EM_H
#define EM_H


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
 * Data Types
 */
typedef struct
{
  UBYTE em_class;
  UBYTE em_subclass;
  UBYTE em_type;
  UBYTE em_length;
  UBYTE em_parameter [MAX_EM_LENGTH];
} em_data_type;

/*
 * EM classes
 */

#define EM_CLASS_EVENT_TRACE        1
#define EM_CLASS_COUNTER            2
#define EM_CLASS_INFRA_DATA         3
#define EM_CLASS_MS_DATA            4

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
#define EM_SUBCLASS_SC_INFO         9
#define EM_SUBCLASS_NC_INFO         10
#define EM_SUBCLASS_LUP_AND_PAG     12
#define EM_SUBCLASS_PLMN_PARA       13
#define EM_SUBCLASS_CIPH_HOP_DTX    14
 
/*
 * EM Subclasses Mobile Data   
 */
#define EM_SUBCLASS_POWER           15
#define EM_SUBCLASS_IDENTITY        16
#define EM_SUBCLASS_VERSION         17

/*
 * EM types
 */

typedef struct
{
  USHORT            arfcn;               /* channel number              */
  SHORT             c1;                  /* C1 Path Loss Criterion      */
  SHORT             c2;                  /* C2 Reselection Criterion    */
  UBYTE             rxlev;               /* fieldstrength               */
  UBYTE             bsic;                /* BSIC                        */
  UBYTE             mcc[3];              /* Mobile Country Code         */
  UBYTE             mnc[3];              /* Mobile Network Code         */
  USHORT            lac;                 /* Location Area Code          */
  USHORT            cell_id;             /* cell identifier             */
  UBYTE             cba;                 /* Cell barred access          */
  UBYTE             cbq;                 /* Cell barred qualify         */
  UBYTE             til_state;           /* State of cell in TIL/ALR    */
  UBYTE             sync_cnt;            /* Pending L1 cells sync       */
  UBYTE             bcch_cnt;            /* Pending L1 cells bcch read  */
  UBYTE             rxlev_f;             /* RSSI full in dedicated mode */
  UBYTE             rxlev_s;             /* RSSI sub in dedicated mode  */
  UBYTE             rxqual_f;            /* Qual full in dedicated mode */
  UBYTE             rxqual_s;            /* Qual sub in dedicated mode  */
} T_EM_NC_DATA;


/*
 * internal prototypes for em driver
 */
EXTERN void    em_trace_single (UBYTE class, UBYTE subclass, UBYTE type);
EXTERN void    em_trace_ubyte  (UBYTE class, UBYTE subclass, UBYTE type, UBYTE value);
EXTERN void    em_trace_ushort (UBYTE class, UBYTE subclass, UBYTE type, USHORT value);
EXTERN void    em_trace_array  (UBYTE class, UBYTE subclass, UBYTE type, UBYTE * value, UBYTE len);

EXTERN UBYTE * em_get_sys_info (UBYTE sys_info_type);
EXTERN UBYTE   em_get_nc_data  (T_EM_NC_DATA * em_nc_data, UBYTE index);
EXTERN UBYTE   em_get_network_meas (UBYTE * chan_list);
EXTERN UBYTE   em_get_bcch_chan_list (em_data_type * out_em_data);
/*
 * Access Functions
 */
#if defined (EM_TRACE)

#define EM_EV_L1_TRACE(a)           em_trace_single(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_LAYER_1,a);
#define EM_EV_L1_TRACE_UBYTE(a,b)   em_trace_ubyte(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_LAYER_1,a,b);
#define EM_EV_L1_TRACE_USHORT(a,b)  em_trace_ushort(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_LAYER_1,a,b);
#define EM_EV_L1_TRACE_ARRAY(a,b,c) em_trace_array(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_LAYER_1,a,b,c);

#define EM_EV_DL_TRACE(a)           em_trace_single(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_DL,a);
#define EM_EV_DL_TRACE_UBYTE(a,b)   em_trace_ubyte(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_DL,a,b);
#define EM_EV_DL_TRACE_USHORT(a,b)  em_trace_ushort(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_DL,a,b);
#define EM_EV_DL_TRACE_ARRAY(a,b,c) em_trace_array(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_DL,a,b,c);

#define EM_EV_RR_TRACE(a)           em_trace_single(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_RR,a);
#define EM_EV_RR_TRACE_UBYTE(a,b)   em_trace_ubyte(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_RR,a,b);
#define EM_EV_RR_TRACE_USHORT(a,b)  em_trace_ushort(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_RR,a,b);
#define EM_EV_RR_TRACE_ARRAY(a,b,c) em_trace_array(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_RR,a,b,c);

#define EM_EV_MM_TRACE(a)           em_trace_single(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_MM,a);
#define EM_EV_MM_TRACE_UBYTE(a,b)   em_trace_ubyte(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_MM,a,b);
#define EM_EV_MM_TRACE_USHORT(a,b)  em_trace_ushort(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_MM,a,b);
#define EM_EV_MM_TRACE_ARRAY(a,b,c) em_trace_array(EM_CLASS_EVENT_TRACE,EM_SUBCLASS_MM,a,b,c);

#else

#define EM_EV_L1_TRACE(a)         
#define EM_EV_L1_TRACE_UBYTE(a,b) 
#define EM_EV_L1_TRACE_USHORT(a,b) 
#define EM_EV_L1_TRACE_ARRAY(a,b,c)

#define EM_EV_DL_TRACE(a)         
#define EM_EV_DL_TRACE_UBYTE(a,b) 
#define EM_EV_DL_TRACE_USHORT(a,b) 
#define EM_EV_DL_TRACE_ARRAY(a,b,c)

#define EM_EV_RR_TRACE(a)         
#define EM_EV_RR_TRACE_UBYTE(a,b) 
#define EM_EV_RR_TRACE_USHORT(a,b) 
#define EM_EV_RR_TRACE_ARRAY(a,b,c)

#define EM_EV_MM_TRACE(a)         
#define EM_EV_MM_TRACE_UBYTE(a,b) 
#define EM_EV_MM_TRACE_USHORT(a,b) 
#define EM_EV_MM_TRACE_ARRAY(a,b,c)

#endif

/*
 * Event trace, layer 1
 */
#define EM_EV_L1_POWER_MEAS_REQ      1
#define EM_EV_L1_RX_MEAS_REQ         2
#define EM_EV_L1_RX_MEAS_CNF         3
#define EM_EV_L1_POWER_MEAS_CNF      4
#define EM_EV_L1_BSIC_REQ            5
#define EM_EV_L1_BSIC_AVAIL          6
#define EM_EV_L1_BSIC_NOT_AVAIL      7
#define EM_EV_L1_BCCH_READ_ERR       8
#define EM_EV_L1_CONFIG_CLASSMARK    9
#define EM_EV_L1_CONFIG_IDLE_MODE    10
#define EM_EV_L1_CONFIG_CBCH         11
#define EM_EV_L1_CONFIG_NCELL        12
#define EM_EV_L1_CHANGE_EXT_PAG      13
#define EM_EV_L1_CHANGE_REORG_PAG    14
#define EM_EV_L1_CHANGE_NORM_PAG     15
#define EM_EV_L1_CHANGE_IDLE_BCCH    16
#define EM_EV_L1_DOWNLINK_ERROR      17
#define EM_EV_L1_PAGING_DETECTED     18
#define EM_EV_L1_READ_NCELL_BSIC     19
#define EM_EV_L1_NCELL_BSIC_AVAIL    20
#define EM_EV_L1_NCELL_BSIC_NO_AVAIL 21
#define EM_EV_L1_READ_NCELL_SB       22
#define EM_EV_L1_NCELL_SB_AVAIL      23
#define EM_EV_L1_NCELL_SB_NO_AVAIL   24
#define EM_EV_L1_READ_NCELL_BCCH     25
#define EM_EV_L1_NCELL_BCCH_AVAIL    26
#define EM_EV_L1_NCELL_BCCH_NO_AVAIL 27
#define EM_EV_L1_CONFIG_CELL_RESEL   28
#define EM_EV_L1_START_CON_EST       29
#define EM_EV_L1_CONFIG_IMM_ASS      30
#define EM_EV_L1_STOP_CON_EST        31
#define EM_EV_L1_CONFIG_CHN_ASS      32
#define EM_EV_L1_CONFIG_ASYNC_HO     33
#define EM_EV_L1_CONFIG_SYNC_HO      34
#define EM_EV_L1_CONFIG_PRE_HO       35
#define EM_EV_L1_CONFIG_PSEUDO_HO    36
#define EM_EV_L1_CONFIG_FREQ_REDEF   37
#define EM_EV_L1_CONFIG_CHN_MODIFY   38
#define EM_EV_L1_CONFIG_CIPH_SET     39
#define EM_EV_L1_START_CBCH_READ     40
#define EM_EV_L1_RECEIVE_CBCH        41
#define EM_EV_L1_CBCH_MSG_COMPLETE   42
#define EM_EV_L1_STOP_CBCH_READ      43
#define EM_EV_L1_RECEIVE_NULL        44
#define EM_EV_L1_RECEIVE_SCHEDULE    45
#define EM_EV_L1_CONFIG_TCH_LOOP     46
#define EM_EV_L1_CONFIG_DAT          47
#define EM_EV_L1_DEACTIVATE          48

/*
 * Event trace, data link layer 
 */
#define EM_EV_DL_SDCCH_EST_PASSED     1
#define EM_EV_DL_SDCCH_EST_FAILED     2
#define EM_EV_DL_FACCH_EST_PASSED     3
#define EM_EV_DL_FACCH_EST_FAILED     4
#define EM_EV_DL_SDCCH_3_EST_PASSED   5
#define EM_EV_DL_SDCCH_3_EST_FAILED   6
#define EM_EV_DL_SACCH_3_EST_PASSED   7
#define EM_EV_DL_SACCH_3_EST_FAILED   8
#define EM_EV_DL_SDCCH_REL            9
#define EM_EV_DL_FACCH_REL           10
#define EM_EV_DL_SDCCH_3_REL         11
#define EM_EV_DL_FACCH_3_REL         12
#define EM_EV_DL_SDCCH_SUSPEND       13
#define EM_EV_DL_FACCH_SUSPEND       14
#define EM_EV_DL_SDCCH_MINOR_ERR     15
#define EM_EV_DL_FACCH_MINOR_ERR     16
#define EM_EV_DL_SDCCH_3_MINOR_ERR   17
#define EM_EV_DL_SACCH_3_MINOR_ERR   18
#define EM_EV_DL_SDCCH_MAJOR_ERR     19
#define EM_EV_DL_FACCH_MAJOR_ERR     20
#define EM_EV_DL_SDCCH_3_MAJOR_ERR   21
#define EM_EV_DL_SACCH_3_MAJOR_ERR   22


/*
 * Event trace, radio resource management
 */
#define EM_EV_RR_SRCH_LIM_STARTED     1
#define EM_EV_RR_SRCH_LIM_PASSED      2
#define EM_EV_RR_SRCH_LIM_FAILED      3
#define EM_EV_RR_SRCH_FULL_START_MM   4
#define EM_EV_RR_SRCH_FULL_START_RR   5
#define EM_EV_RR_SRCH_FULL_PASSED     6
#define EM_EV_RR_SRCH_FULL_FAILED     7
#define EM_EV_RR_SRCH_HPLMN_START_RR  8
#define EM_EV_RR_SRCH_HPLMN_PASSED    9
#define EM_EV_RR_SRCH_HPLMN_FAILED   10
#define EM_EV_RR_SRCH_PLMN_START     11
#define EM_EV_RR_SRCH_PLMN_FINISHED  12
#define EM_EV_RR_IDLE_NO_SERVICE     13
#define EM_EV_RR_IDLE_LIM_SERVICE    14
#define EM_EV_RR_IDLE_FULL_SERVICE   15
#define EM_EV_RR_CELL_RESEL_START    16
#define EM_EV_RR_CELL_RESEL_END      17
#define EM_EV_RR_PAGING_DETECTED     18
#define EM_EV_RR_DOWNLINK_FAILURE    19
#define EM_EV_RR_CHANNEL_REQUEST     20
#define EM_EV_RR_IMM_ASS             21
#define EM_EV_RR_IMM_ASS_EXT         22
#define EM_EV_RR_IMM_ASS_REJ         23
#define EM_EV_RR_L2_CON_EST          24
#define EM_EV_RR_EARLY_CLASS_SEND    25
#define EM_EV_RR_CLASS_INTERROGATION 26
#define EM_EV_RR_ASS                 27
#define EM_EV_RR_ASS_FAIL_REJ        28
#define EM_EV_RR_ASS_FAIL_REC        29
#define EM_EV_RR_ASS_FAIL_LOSS       30
#define EM_EV_RR_ASS_COMPLETE        31
#define EM_EV_RR_HO_ASYNC            32
#define EM_EV_RR_HO_SYNC             33
#define EM_EV_RR_HO_PRE_SYNC         34
#define EM_EV_RR_HO_PSEUDO_SYNC      35
#define EM_EV_RR_HO_FAIL_REJ         36
#define EM_EV_RR_HO_FAIL_REC         37
#define EM_EV_RR_HO_FAIL_LOSS        38
#define EM_EV_RR_HO_COMPLETE         39
#define EM_EV_RR_FREQ_REDEF          40
#define EM_EV_RR_CIPH_SET            41
#define EM_EV_RR_CHAN_MOD_START      42
#define EM_EV_RR_CHAN_MOD_END        43
#define EM_EV_RR_CHAN_RELEASE        44
#define EM_EV_RR_RADIO_LINK_FAIL     45
#define EM_EV_RR_LAYER_2_LOSS        46
#define EM_EV_RR_STATUS_RECEIVED     47
#define EM_EV_RR_STATUS_SEND         48
#define EM_EV_RR_TCH_LOOP            49
#define EM_EV_RR_TEST_INTERFACE      50
#define EM_EV_RR_DEACTIVATION        51

/*
 * Event trace, mobility management
 */
#define EM_EV_MM_SIM_INSERT           1
#define EM_EV_MM_SIM_REMOVE           2
#define EM_EV_MM_SET_PLMN_MODE        3
#define EM_EV_MM_START_REG_LIM        4
#define EM_EV_MM_START_REG_FULL       5
#define EM_EV_MM_START_PLMN_AVAIL     6
#define EM_EV_MM_CS_NO_SERVICE        7
#define EM_EV_MM_CS_LIM_SERVICE       8
#define EM_EV_MM_CS_FULL_SERVICE      9
#define EM_EV_MM_CR_NO_SERVICE       10
#define EM_EV_MM_CR_LIM_SERVICE      11
#define EM_EV_MM_CR_FULL_SERVICE     12
#define EM_EV_MM_RESULT_PLMN_LIST    13
#define EM_EV_MM_IMSI_ATT_STARTED    14
#define EM_EV_MM_RR_CON_ESTABLISHED  15
#define EM_EV_MM_RR_CON_FAILED       16
#define EM_EV_MM_LOC_UPD_ACCEPT      17
#define EM_EV_MM_NORM_LUP_STARTED    18
#define EM_EV_MM_PERI_LUP_STARTED    19
#define EM_EV_MM_TMSI_REALLOC_START  20
#define EM_EV_MM_TMSI_REALLOC_CMP    21
#define EM_EV_MM_LOC_UPD_REJECT      22
#define EM_EV_MM_IMSI_DETACH         23
#define EM_EV_MM_ID_REQUEST          24
#define EM_EV_MM_ID_RESPONSE         25
#define EM_EV_MM_AUTH_REQUEST        26
#define EM_EV_MM_AUTH_RESPONSE       27
#define EM_EV_MM_AUTH_REJECT         28
#define EM_EV_MM_CM_SERV_REQ_CC      29
#define EM_EV_MM_CM_SERV_REQ_SS_SMS  30
#define EM_EV_MM_CM_SERV_ACC_CIPH    31
#define EM_EV_MM_CM_SERV_ACC_CMD     32
#define EM_EV_MM_CM_SERV_REJECT      33

/*
 * Infrastructure Data - Serving Cell Information
 */
#define EM_IN_SC_BCCH_ARFCN           1
#define EM_IN_SC_C1                   2
#define EM_IN_SC_RX                   3
#define EM_IN_SC_BSIC                 4
#define EM_IN_SC_DLT                  5
#define EM_IN_SC_TX_POWER             6
#define EM_IN_SC_TS                   7
#define EM_IN_SC_DEDI_ARFCN           8
#define EM_IN_SC_DEDI_RX_FULL         9
#define EM_IN_SC_DEDI_RX_SUB         10
#define EM_IN_SC_DEDI_RLT            11
#define EM_IN_SC_DEDI_TA             12
#define EM_IN_SC_DEDI_QUAL_FULL      13
#define EM_IN_SC_DEDI_QUAL_SUB       14
#define EM_IN_SC_NMR_RAW             15
#define EM_IN_SC_BCCH_LIST_RAW       16
#define EM_IN_SC_C2                  17
#define EM_IN_SC_LAC                 18
#define EM_IN_SC_BA                  19
#define EM_IN_SC_BQ                  20
#define EM_IN_SC_TIL_STATE           21

/*
 * Infrastructure Data - Neighbour Cell Information
 */
#define EM_IN_NC_NO_OF_NCELLS         1
#define EM_IN_NC_BCCH_1               2
#define EM_IN_NC_BCCH_2               3
#define EM_IN_NC_BCCH_3               4
#define EM_IN_NC_BCCH_4               5
#define EM_IN_NC_BCCH_5               6
#define EM_IN_NC_BCCH_6               7
#define EM_IN_NC_RX_1                 8
#define EM_IN_NC_RX_2                 9
#define EM_IN_NC_RX_3                10
#define EM_IN_NC_RX_4                11
#define EM_IN_NC_RX_5                12
#define EM_IN_NC_RX_6                13
#define EM_IN_NC_C1_1                14
#define EM_IN_NC_C1_2                15
#define EM_IN_NC_C1_3                16
#define EM_IN_NC_C1_4                17
#define EM_IN_NC_C1_5                18
#define EM_IN_NC_C1_6                19
#define EM_IN_NC_BSIC_1              20
#define EM_IN_NC_BSIC_2              21
#define EM_IN_NC_BSIC_3              22
#define EM_IN_NC_BSIC_4              23
#define EM_IN_NC_BSIC_5              24
#define EM_IN_NC_BSIC_6              25
#define EM_IN_NC_CID_1               26
#define EM_IN_NC_CID_2               27
#define EM_IN_NC_CID_3               28
#define EM_IN_NC_CID_4               29
#define EM_IN_NC_CID_5               30
#define EM_IN_NC_CID_6               31
#define EM_IN_NC_FN_OFFSET_1         32
#define EM_IN_NC_FN_OFFSET_2         33
#define EM_IN_NC_FN_OFFSET_3         34
#define EM_IN_NC_FN_OFFSET_4         35
#define EM_IN_NC_FN_OFFSET_5         36
#define EM_IN_NC_FN_OFFSET_6         37
#define EM_IN_NC_TA_OFFSET_1         38
#define EM_IN_NC_TA_OFFSET_2         39
#define EM_IN_NC_TA_OFFSET_3         40
#define EM_IN_NC_TA_OFFSET_4         41
#define EM_IN_NC_TA_OFFSET_5         42
#define EM_IN_NC_TA_OFFSET_6         43
#define EM_IN_NC_C2_1                44
#define EM_IN_NC_C2_2                45
#define EM_IN_NC_C2_3                46
#define EM_IN_NC_C2_4                47
#define EM_IN_NC_C2_5                48
#define EM_IN_NC_C2_6                49
#define EM_IN_NC_LAC_1               50
#define EM_IN_NC_LAC_2               51
#define EM_IN_NC_LAC_3               52
#define EM_IN_NC_LAC_4               53
#define EM_IN_NC_LAC_5               54
#define EM_IN_NC_LAC_6               55
#define EM_IN_NC_BA_1                56
#define EM_IN_NC_BA_2                57
#define EM_IN_NC_BA_3                58
#define EM_IN_NC_BA_4                59
#define EM_IN_NC_BA_5                60
#define EM_IN_NC_BA_6                61
#define EM_IN_NC_BQ_1                62
#define EM_IN_NC_BQ_2                63
#define EM_IN_NC_BQ_3                64
#define EM_IN_NC_BQ_4                65
#define EM_IN_NC_BQ_5                66
#define EM_IN_NC_BQ_6                67
#define EM_IN_NC_TIL_STATE_1         68
#define EM_IN_NC_TIL_STATE_2         69
#define EM_IN_NC_TIL_STATE_3         70
#define EM_IN_NC_TIL_STATE_4         71
#define EM_IN_NC_TIL_STATE_5         72
#define EM_IN_NC_TIL_STATE_6         73
#define EM_IN_NC_TIL_SYNC_CNT        74
#define EM_IN_NC_TIL_BCCH_CNT        75

/*
 * Infrastructure Data - Location and Paging Parameter
 */
#define EM_IN_LP_BS_PA_MFRMS          1
#define EM_IN_LP_T3212                2
#define EM_IN_LP_MCC                  3
#define EM_IN_LP_MNC                  4
#define EM_IN_LP_LAC                  5
#define EM_IN_LP_TMSI                 6
#define EM_IN_LP_LAC_RAW              7
#define EM_IN_LP_CI_RAW               8

#if defined (NEW_FRAME)
/*
 * to achieve backward compatibility with older definitions
 */
#define drv_SignalCB_Type           T_DRV_CB_FUNC
#define drv_SignalID_Type           T_DRV_SIGNAL
#define T_VSI_THANDLE               USHORT
#endif

/*
 * Prototypes
 */
#ifndef FF_EM_MODE 
EXTERN UBYTE em_Init                (void);
EXTERN void  em_Exit                (void);
#endif /* FF_EM_MODE */
EXTERN UBYTE em_Read_Parameter      (UBYTE               em_class, 
                                     UBYTE               em_subclass,
                                     UBYTE               em_type,
                                     em_data_type *      out_em_data);
EXTERN UBYTE em_Enable_Post_Mortem  (void);
EXTERN UBYTE em_Disable_Post_Mortem (void);
EXTERN UBYTE em_Read_Post_Mortem    (em_data_type *      out_em_data);

#endif
