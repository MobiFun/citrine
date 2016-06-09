/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  ALR
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
|  Purpose :  Definitions for the Protocol Stack Entity ALR
+-----------------------------------------------------------------------------
*/

#ifndef ALR_H
#define ALR_H

#ifdef TI_PS_HCOMM_CHANGE
#include "cl_hComm_handle.h"
#endif

#ifdef TI_GSP_STR2IND_S2I_STRING
typedef unsigned int T_S2I_STRING;
#else
typedef char * T_S2I_STRING;
#define S2I_STRING(x) (x)
#endif

#ifdef TI_PS_OP_VSI_NO_CALL_ID
#define TIMER_START(C,I,T)         vsi_t_start_nc(I,T)
#define TIMER_PSTART(C,I,T,R)      vsi_t_pstart_nc(I,T,R)
#define TIMER_STOP(C,I)            vsi_t_stop_nc(I)
#define TIMER_STATUS(C,I,T)        vsi_t_status_nc(I,T)
#define SUSPEND_SELF(C,T)          vsi_t_sleep_nc(T)
#define SYSTEM_TIME(C,T)           vsi_t_time_nc(T)               
#else /* TI_PS_OP_VSI_NO_CALL_ID */
#define TIMER_START(C,I,T)         vsi_t_start(C,I,T)
#define TIMER_PSTART(C,I,T,R)      vsi_t_pstart(C,I,T,R)
#define TIMER_STOP(C,I)            vsi_t_stop(C,I)
#define TIMER_STATUS(C,I,T)        vsi_t_status(C,I,T)
#define SUSPEND_SELF(C,T)          vsi_t_sleep(C,T)
#define SYSTEM_TIME(C,T)           vsi_t_time(C,T)               
#endif /* TI_PS_OP_VSI_NO_CALL_ID */


#define STOP_SYNC_TASK          /* stop always any sync tasks before start a new one */
#if 0
#define DL_TRACE_ENABLED        /* use DL offline trace */
#define TRACE_STATE_TRANSITION  /* trace some state transitions */
#endif /* 0 */

#if !defined(_SIMULATION_) && defined(WIN32)
#define _SIMULATION_
#endif  /* !_SIMULATION_ && WIN32 */

#if defined(_SIMULATION_)
#undef  DL_TRACE_ENABLED  /* no _SYST trace during simulation */
#endif  /* WIN32 */

#define SYST_TRACE(a) vsi_o_ttrace(0, 0xFFFF,a)
#define SYST           0, 0xffff
#define SYST_TRACE_P(a) vsi_o_ttrace a
/*
 * use it as showed next line...
 * SYST_TRACE_P((SYST, "e.g. two parameter: %d %d", p1, p2));
 */


#if !defined(ELEMENTS)
#define ELEMENTS(array) (sizeof(array)/sizeof(array[0]))
#endif  /* !ELEMENTS */


/* Bitoffset for Message Buffer allocation */
#define ENCODE_OFFSET             24

/*
 * Bitmasks
 */
#define BIT_0                     0x01
#define BIT_1                     0x02
#define BIT_2                     0x04
#define BIT_3                     0x08
#define BIT_4                     0x10
#define BIT_5                     0x20
#define BIT_6                     0x40
#define BIT_7                     0x80
#define BIT_012                   0x07

#include "p_ph.h"
#include "p_mph.h"

/* First 40 carriers per band will be added at top of the MPH_POWER_CNF
 * and the next 20 Carriers (40 to 60) will be added at the bottom.
 * Remaining carriers needs to be ignored.
 */

#define ADD_AT_THE_TOP       0x01   /* first 40 carrier */
#define ADD_AT_THE_BOTTOM    0x02   /* already 40 carriers were added */
#define REACHED_THE_MAXIMUM  0x04   /* already 60 carriers were added */
#define DO_NOT_ADD           0x08   /* wrong carrier */


/* Private modes */
#define MODE_CHAN_ASS_FAIL   16
#define MODE_HANDOVER_FAIL   17

/* Results of configuration dedicated mode */
#define DEDICATED_SUCCESS    0

#define  ALR_ALLOCATE_NEW_BA(v)             ((((v)+1) % (ALR_BA_HIGH-ALR_BA_LOW+1)) + ALR_BA_LOW)
#define  IS_EXT_MEAS_RUNNING                (alr_data->cs_data.mph_ext_meas_req NEQ NULL)

/*
 * Dynamic Configurations
 */
#define ID_STD               1
#define ID_CONFIG            2
#define ID_MON_COUNTER_IDLE  3
#define ID_MON_COUNTER_DEDI  4
#define ID_TRC_DATA_IND      5
#define ID_RACH_FAILURE      6
#define ID_EOTD              7
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
#define ID_MB_TESTING        8
#endif

#define ID_TRC_DATA_IND_ALL     0xff
#define ID_TRC_DATA_IND_VALID   0x7f
#define ID_TRC_DATA_IND_INVALID 0x80
#define ID_TRC_DATA_IND_BCCH    0x01  /* normal and extended BCCH */
#define ID_TRC_DATA_IND_PCH     0x02  /* normal and extended PCH */
#define ID_TRC_DATA_IND_CCCH    0x04
#define ID_TRC_DATA_IND_CBCH    0x08
#define ID_TRC_DATA_IND_SCCH    0x10  /* SACCH and SDCCH */
#define ID_TRC_DATA_IND_FCCH    0x20  /* full and half rate FACCH */
#define ID_TRC_DATA_IND_STATE   0x40  /* MA status */
#define ID_TRC_DATA_IND_INIT    0x3f  /* all valid frames without stati */
#define ID_TRC_DATA_IND_INIT2   0x1f  /* ... without FACCH */

EXTERN  UBYTE v_mon_trc_data_ind;
EXTERN  UBYTE v_cfg_rach_failure;

/* In addition to normal page modes defined in SAP */
#define PGM_REORG_CS         3
#define NO_PGM_CHANGE        4
#define SAVED_PGM            0xff

/*If page mode is set to REORG and a HPLMN search is performed, L1 cannot
synchronize to NC's. Therefore NORMAL page mode is started. This state
defines that the mobile should be in REORG, but actual is in NORMAL.*/
#define PGM_REORG_NC_SYNC    5
EXTERN  UBYTE page_mode_before_hplmn_search;/*to distinguish PGM_REORG and PGM_REORG_CS */

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
EXTERN UBYTE alr_multiband_std;
#endif

/* Page Mode Changes */
#define NONE                 0
#define SWAP_TO_EXTEND       1
#define SWAP_TO_REORG        2
#define SWAP_TO_NORMAL       3

#define MAX_GSM_CHANNEL_ATTEMPTS  40   /* GSM says 30, but this is done in the field */
#define MAX_DPCS_CHANNEL_ATTEMPTS  40

/* Band Limitations for multiband reporting */
#define NO_BAND_LIMITATION   0
#define EXCLUDE_SC_BAND      1
#define ONLY_SC_BAND         2

#define IMSI_LEN                  9
#define SI_CONTENTS_MSG_T         2
#define SI_CONTENTS_CS2           9
#define ONLY_ACS                  0x80
#define STOP_PCH_READING          1
#define DONT_STOP_PCH_READING     0
#define STOP_MEASUREMENTS         2

/*
 * Neighbour cell status
 */
/* #define DEFINE_OLD_NC_STATUS */
#if !defined(DEFINE_OLD_NC_STATUS)
/* neighbour cell base stati */
#define  NCS_INACTIVE                       0x00
#define  NCS_IDLE                           0x01
#define  NCS_FB_SB                          0x02
#define  NCS_SB                             0x03
/* neighbour cell bit masks */
#define  NCB_BCCH                           0x04
#define  NCB_READ                           0x08
#define  NCB_PENDING                        0x10
#define  NCB_SYNC                           0x20
#define  NCB_RR_NOT_INFORMED                0x40
#define  NCB_FAILED                         0x80

#if defined (TI_PS_FF_RTD) AND defined (REL99)
#define  NUM_OF_TDMA_FRAMES                 0x40
#define  RTD_12BIT_EXTRACT                  0x0FFF
#define  MAX_NUM_OF_QBITS                   0x1388
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */ 


/* neighbour cell stati */
/* 1of<n> n = 6,12 means that the cell belongs to the n strongest */
/* RLA_C means Received Level Averages according GSM 05.08 seciton 6.1 */

                                            /* (1) no PERIODIC measurement received; cell is excluded from all activities except rxlev measurement */
#define  INACTIVE                           (NCS_INACTIVE)

                                            /* (2) PERIODIC measurement received; not synchronized; BCCH data unknown */
#define  IDLE                               (NCS_IDLE)

                                            /* (11a) not a 1of12 cell any more; synchronized */
#define  IDLE_SYNC                          (NCS_IDLE|NCB_SYNC)

                                            /* (3) strong cell(1of12); not synchronized yet; need first FB and SB synchronization */
#define  READ_FB_SB                         (NCS_FB_SB|NCB_READ)

                                            /* (4) first FB and SB synchronization ongoing */
#define  READ_FB_SB_PENDING                 (NCS_FB_SB|NCB_READ|NCB_PENDING)

                                            /* (9) synchronized; need re-synchronization of SB (10/30 sec); no need to read BCCH */
#define  READ_SB                            (NCS_SB|NCB_READ)

                                            /* (10) re-synchronization of SB ongoing (10/30 sec); no reading of BCCH */
#define  READ_SB_PENDING                    (NCS_SB|NCB_READ|NCB_PENDING)

                                            /* (9a) synchronized; need re-synchronization of SB and re-reading of BCCH (5 min) */
#define  READ_SB_BCCH                       (NCS_SB|NCB_READ|NCB_BCCH)

                                            /* (10a) re-synchronization of SB ongoing (5 min) */
#define  READ_SB_BCCH_PENDING               (NCS_SB|NCB_READ|NCB_BCCH|NCB_PENDING)

                                            /* (11a) re-synchronization of SB finished; need re-reading of BCCH (5 min) */
#define  READ_BCCH                          (NCS_SB|NCB_READ|NCB_BCCH|NCB_SYNC)

                                            /* (12a) re-reading of BCCH ongoing (5 min) */
#define  READ_BCCH_PENDING                  (NCS_SB|NCB_READ|NCB_BCCH|NCB_SYNC|NCB_PENDING)

                                            /* (5) 1of6 cell; synchronized; BCCH data unknown; need to read BCCH, but to notify RR after a measurement ind only */
#define  READ_BCCH_RR_NOT_INFORMED          (NCS_SB|NCB_READ|NCB_BCCH|NCB_SYNC|NCB_RR_NOT_INFORMED)

                                            /* (6) 1of6 cell; reading of BCCH ongoing  */
#define  READ_BCCH_PENDING_RR_NOT_INFORMED  (NCS_SB|NCB_READ|NCB_BCCH|NCB_SYNC|NCB_RR_NOT_INFORMED|NCB_PENDING)

                                            /* (8) 1of12 cell; synchronized; in case 1of6: BCCH data known; RR is notified; subsequent RLA_Cs being computed */
#define  FB_SB_SYNC                         (NCS_FB_SB|NCB_SYNC)

                                            /* (7) 1of6 cell; synchronized; BCCH data known; RR not yet notified; first RLA_C being computed */
#define  FB_SB_SYNC_RR_NOT_INFORMED         (NCS_FB_SB|NCB_SYNC|NCB_RR_NOT_INFORMED)

                                            /* (13) 1of<n> cell state questionable; FB/SB sync or BCCH reading failed 1 thru 8 times */
#define  FB_SB_FAILED                       (NCS_FB_SB|NCB_FAILED)

                                            /* (14) 1of<n> cell; synchronization of FB/SB or reading of BCCH failed > 8 times; NCC check failed; */
#define  EXCLUDED                           (NCB_FAILED)

#else /* !DEFINE_OLD_NC_STATUS */
#define INACTIVE                            0
#define IDLE                                1
#define READ_BCCH                           2
#define FB_SB_SYNC                          3
#define FB_SB_FAILED                        4
#define READ_FB_SB                          5
#define READ_SB                             6
#define IDLE_SYNC                           7
#define EXCLUDED                            8
#define FB_SB_SYNC_RR_NOT_INFORMED          9
#define READ_SB_BCCH                       10
#define READ_BCCH_PENDING                  11
#define READ_FB_SB_PENDING                 12
#define READ_SB_PENDING                    13
#define READ_SB_BCCH_PENDING               14
#define READ_BCCH_RR_NOT_INFORMED          15
#define READ_BCCH_PENDING_RR_NOT_INFORMED  16
#endif /* !DEFINE_OLD_NC_STATUS */

/*
 * states of a neighbour cell with respect to the BA list
 * in the last MPH_NEIGHBOURCELL_REQ.
 */
#define IN_BA              0   /* NCell was in last MPH_NEIGHBOURCELL_REQ */
#define NOT_IN_BA_SHORT    1   /* NCell wasn't in last MPH_NEIGHBOURCELL_REQ but at last confirmation it had state IN_BA (not in BA for a short time) */
#define NOT_IN_BA_LONG     2   /* NCell wasn't in last MPH_NEIGHBOURCELL_REQ and at last confirmation it hadn't state IN_BA (not in BA for a long time) */
/*
*  Definitions for AMR
*/
#define CM_AMR             0x41

/*
 * The times are given in number of multiframes. Each multiframe is
 * equal to circa 217 milliseconds.
 */
#define THIRTY_SECONDS_SCELL_BCCH   128

/*
 * States Main Control
 */
#define MA_NULL              0
#define MA_CELL_SELECTION    1
#define MA_IDLE              2
#define MA_CON_EST           3
#define MA_DEDICATED         4
#define MA_CELL_RESELECTION  5

#ifdef GPRS
#define MA_PTM               6
#endif

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const 
        STATE_MA_NAME[];
#ifndef GPRS
/* N950 Memory Optimization - Implements Measure #39*/
#else  /* !GPRS */
/* N950 Memory Optimization - Implements Measure #39*/
#endif  /* !GPRS */
/*
 * States Idle Neighbour Cell
 */
#define NC_NULL              0
#define NC_IDLE              1
#define NC_DEDICATED         2
#define NC_CON_EST           3

#ifdef GPRS
#define NC_PIM_PBCCH         4
#define NC_PTM_PBCCH         5

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const 
        STATE_NC_NAME[];
#else
/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const 
        STATE_NC_NAME[];
#endif

/*
 * States Dedicated Control
 */
#define DEDI_INACTIVE        0
#define DEDI_IMM_ASS         1
#define DEDI_IMM_ASS_RECONN  2
#define DEDI_ACTIVE          3

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const 
        STATE_DEDI_NAME[];

typedef enum
{
  HO_TYPE_ASYNC=1,  
  HO_TYPE_PRE_SYNC,
  HO_TYPE_SYNC
}T_HO_TYPE;

/*
 * States Cell Selection
 */
#define CS_NULL              0
#define CS_INIT_L1           1  /* start with configuration of the radio band */
#define CS_INIT_ACTIVE       2  /* configuration of the radio band is active */
#define CS_INIT_DONE         3  /* configuration has done */
#define CS_START_MEASURE     4  /* start measurement with preceding configuration */
#define CS_ACTIVE_MEASURE    5  /* power measurement step is active */
#define CS_MEASURED          6  /* power measurement step has finished */
#define CS_INIT_SYNC         7  /* configuration of the radio band before SYNC */
#define CS_STOP_SYNC         8  /* stop any sync task */
#define CS_STOP_SYNC_DONE    9  /* no sync task active */
#define CS_ACTIVE_SYNC      10
#define CS_ACTIVE_BCCH      11
#define CS_NW_SYNC_TIMEOUT  12  /* ALR has timed out whilst attempting to sync to a nw */

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const 
        STATE_CS_NAME[];
/*
 * States PCH Control
 */
#define PCH_NULL           0
#define PCH_ACTIVE         1

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const 
        STATE_PCH_NAME[];
/*
 * States RACH Control
 */
#define RA_NULL            0
#define RA_ACTIVE          1
#define RA_LISTEN_TO_CCCH  2

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN T_S2I_STRING const 
        STATE_RA_NAME[];
/*
 * States CBCH Control
 */
#define CBCH_NULL          0
#define CBCH_IDLE          1
#define CBCH_NO_DRX        2
#define CBCH_DRX           3

#define CBCH_HZ_OFF        0
#define CBCH_HZ_IDLE        5
#define CBCH_HZ_PENDING     6
#define CBCH_HZ_RECEIVED    7

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const  STATE_NORMAL_CBCH_NAME[];
EXTERN  T_S2I_STRING const  STATE_EXT_CBCH_NAME[];

/*
 * States Idle Mode Serving Cell
 */
#define SC_NULL            0
#define SC_IDLE_INITIAL    1
#define SC_IDLE            2

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const 
        STATE_SC_NAME[];
/*
 * States Idle Mode Serving Cell
 */

typedef enum {
  NC_ACQUIRE=0,
  NC_CONFIRM
} T_NCELL_PROC;

/* N950 Memory Optimization - Implements Measure #39*/
EXTERN  T_S2I_STRING const 
        STATE_NC_PROC_NAME[];

/*
 * ALR manages 9 (MAX_STATE) states: CBCH,CS,MA,NC,PCH,RA,SC,DEDI,NC_PROC
 */

#define STATE_CS                  0
#define STATE_MA                  1
#define STATE_NC                  2
#define STATE_PCH                 3
#define STATE_RA                  4
#define STATE_SC                  5
#define STATE_DEDI                6
#define STATE_NC_PROC             7
#define STATE_NORMAL_CBCH         8
#define STATE_EXT_CBCH            9
#define MAX_STATE                 10

/*
 * A definition is provided in ALR_PEI.C
 */
#ifdef ALR_PEI_C
/* N950 Memory Optimization - Implements Measure #39*/
GLOBAL T_S2I_STRING const    STATE_EXT_CBCH_NAME[] = 
{    
  S2I_STRING("CBCH_NULL")         ,
  S2I_STRING("CBCH_IDLE")         ,
  S2I_STRING("CBCH_NO_DRX")       ,
  S2I_STRING("CBCH_DRX")
};

GLOBAL T_S2I_STRING const    STATE_NORMAL_CBCH_NAME[] = 
{    
  S2I_STRING("CBCH_NULL")         ,
  S2I_STRING("CBCH_IDLE")         ,
  S2I_STRING("CBCH_NO_DRX")       ,
  S2I_STRING("CBCH_DRX")
};

GLOBAL T_S2I_STRING const    STATE_CS_NAME  [] =
{
  S2I_STRING("CS_NULL")            ,
  S2I_STRING("CS_INIT_L1")         ,
  S2I_STRING("CS_INIT_ACTIVE")     ,
  S2I_STRING("CS_INIT_DONE")       ,
  S2I_STRING("CS_START_MEASURE")   ,
  S2I_STRING("CS_ACTIVE_MEASURE")  ,
  S2I_STRING("CS_MEASURED")        ,
  S2I_STRING("CS_INIT_SYNC")       ,
  S2I_STRING("CS_STOP_SYNC")       ,
  S2I_STRING("CS_STOP_SYNC_DONE")  ,
  S2I_STRING("CS_ACTIVE_SYNC")     ,
  S2I_STRING("CS_ACTIVE_BCCH")     ,
  S2I_STRING("CS_NW_SYNC_TIMEOUT")  
};

#ifdef GPRS
GLOBAL T_S2I_STRING const    STATE_MA_NAME  [] =
{
  S2I_STRING("MA_NULL")            ,
  S2I_STRING("MA_CELL_SELECTION")  ,
  S2I_STRING("MA_IDLE")            ,
  S2I_STRING("MA_CON_EST")         ,
  S2I_STRING("MA_DEDICATED")       ,
  S2I_STRING("MA_CELL_RESELECTION"),
  S2I_STRING("MA_PTM")  
};
#else
GLOBAL T_S2I_STRING const    STATE_MA_NAME  [] =
{
  S2I_STRING("MA_NULL")            ,
  S2I_STRING("MA_CELL_SELECTION")  ,
  S2I_STRING("MA_IDLE")            ,
  S2I_STRING("MA_CON_EST")         ,
  S2I_STRING("MA_DEDICATED")       ,
  S2I_STRING("MA_CELL_RESELECTION")  
};
#endif
#ifdef GPRS
GLOBAL T_S2I_STRING const    STATE_NC_NAME  [] = 
{    
  S2I_STRING("NC_NULL")            ,
  S2I_STRING("NC_IDLE")            ,
  S2I_STRING("NC_DEDICATED")       ,
  S2I_STRING("NC_CON_EST")         ,
  S2I_STRING("NC_PIM_PBCCH")       ,
  S2I_STRING("NC_PTM_PBCCH")
};
#else
GLOBAL T_S2I_STRING const    STATE_NC_NAME  [] = 
{    
  S2I_STRING("NC_NULL")            ,
  S2I_STRING("NC_IDLE")            ,
  S2I_STRING("NC_DEDICATED")       ,
  S2I_STRING("NC_CON_EST")
};
#endif
GLOBAL T_S2I_STRING const    STATE_PCH_NAME [] =
{    
  S2I_STRING("PCH_NULL")         ,
  S2I_STRING("PCH_ACTIVE")
};

GLOBAL T_S2I_STRING const    STATE_RA_NAME  [] = 
{    
  S2I_STRING("RA_NULL")          ,
  S2I_STRING("RA_ACTIVE")        ,
  S2I_STRING("RA_LISTEN_TO_CCCH")  
};

GLOBAL T_S2I_STRING const    STATE_SC_NAME  [] = 
{    
  S2I_STRING("SC_NULL")          ,
  S2I_STRING("SC_IDLE_INITIAL")  ,
  S2I_STRING("SC_IDLE")
};

GLOBAL T_S2I_STRING const    STATE_DEDI_NAME[] = 
{    
  S2I_STRING("DEDI_INACTIVE")      ,
  S2I_STRING("DEDI_IMM_ASS")       ,
  S2I_STRING("DEDI_IMM_ASS_RECONN"),
  S2I_STRING("DEDI_ACTIVE")
};

GLOBAL T_S2I_STRING const    STATE_NC_PROC_NAME[]=
{ 
  S2I_STRING("NC_ACQUIRE")          ,
  S2I_STRING("NC_CONFIRM")
};

#endif  /* ALR_PIC_C */

/*==== MACROS =====================================================*/

/*

   This macros convert channel numbers from and to the layer 1 specific
   representation for the target version.

   For the single bands GSM900, DCS1800 and PCS1900 the macro is removed
   at compile time.

   For dualband (STD==5,STD==6) a DCS1800 channel number is converted
   using the convert function located at the end of alr_main.c

   In the windows variant no change occurs.

*/

EXTERN USHORT convert_arfcn_to_l1  (USHORT arfcn, UBYTE local_std);
EXTERN USHORT convert_arfcn_to_g23 (USHORT arfcn, UBYTE local_std);

/* range check with the fastest way for ARM */
/* corresponds with ((min <= x) AND (x <= max)) */
#define INRANGE(min, x, max)  ((unsigned)(x-min) <= (max-min))

/*
 * target version
 */
#define ARFCN_TO_L1(a)          convert_arfcn_to_l1((USHORT)(a),std)
#define ARFCN_TO_G23(a)         convert_arfcn_to_g23((USHORT)(a),std)
#define ARFCN_STD_TO_L1(a,std)  convert_arfcn_to_l1((USHORT)(a),std)
#define ARFCN_STD_TO_G23(a,std) convert_arfcn_to_g23((USHORT)(a),std)

/* add resp. get the std value to resp. from arfcn */
#define STD_ADD_TO_ARFCN(a,std)  (((a)&~STD_BIT_MASK) | ((std)<<STD_BIT_SHIFT))
#define STD_GET_FROM_ARFCN(a)    ((((a)&STD_BIT_MASK)>>STD_BIT_SHIFT))

/* find the frequency band index to which the given arfcn belongs. */
#define get_band_index_from_arfcn(arfcn, idx, std)\
{\
   UBYTE local_std = STD_GET_FROM_ARFCN(arfcn);\
   if( !local_std )\
      	local_std = std;\
    idx = local_std;\
    if( local_std ==  STD_DUAL ) {\
            if (arfcn >= LOW_CHANNEL_1800)\
            idx = STD_1800;\
            else\
            idx = STD_900;\
    }\
    else if ( local_std EQ  STD_DUAL_EGSM ) {\
            if (arfcn >= LOW_CHANNEL_EGSM)\
                idx = STD_EGSM;\
            else if (arfcn >= LOW_CHANNEL_1800)\
                idx = STD_1800;\
            else if (arfcn EQ CHANNEL_0)\
                idx = STD_EGSM;\
            else\
                idx = STD_900;\
    }\
    else if( local_std EQ  STD_DUAL_US ) {\
            if (arfcn >= LOW_CHANNEL_1900)\
                idx = STD_1900;\
            else\
                idx = STD_850;\
    }\
    (local_std<=STD_DUAL_US)?(idx=array_band_index[idx-1]):(idx=MAX_NUM_BANDS) ;\
}

/*==== TYPES ======================================================*/

EXTERN UBYTE std;       									/*lint -esym(526,std)        : not defined | defined in other entity */
EXTERN UBYTE test_house;									/*lint -esym(526,test_house) : not defined | defined in other entity */

typedef struct {
  USHORT  radio_freq;
  SHORT   accum_power_result;
} T_POWER_ARRAY;

typedef struct {
  USHORT        power_array_size;
  T_POWER_ARRAY power_array[MAX_CARRIERS];
} T_POWER_MEAS;

typedef struct {
  USHORT        power_array_size;
  T_POWER_ARRAY power_array[MAX_CARRIERS_DUAL_EGSM];
} T_POWER_MEAS1;

typedef struct {
  USHORT        power_array_size;
  T_POWER_ARRAY power_array[MAX_CARRIERS_DUAL_US];
} T_POWER_MEAS2;

EXTERN T_POWER_MEAS1  alr_power_meas_result1;
EXTERN T_POWER_MEAS2  alr_power_meas_result2;

#define CS_SYNC_FAIL_COUNT_MAX 5
#define CS_BCCH_FAIL_COUNT_MAX 10

#define SI_1_READ           0x01
#define SI_2_READ           0x02
#define SI_3_READ           0x04
#define SI_4_READ           0x08
#define SI_2TER_READ        0x10
#define SI_2BIS_READ        0x20

typedef struct {
  T_POWER_MEAS    *p_results1;
  T_POWER_MEAS    *p_results2;
  T_MPH_POWER_CNF *p_power_cnf;
  T_MPH_POWER_REQ *p_power_req;           /*   CSI-LLD section: 4.1.3.1.2 */
  T_MPH_EXT_MEAS_REQ *mph_ext_meas_req;
  USHORT          arfcn;
  UBYTE           ext_meas_state_pend;    /* wait for pending MPHC_RXLEV_IND or MPHC_NCELL_SYNC_IND */
  UBYTE           sync_active;
  UBYTE           freq_bands;
  UBYTE           std;
  UBYTE           std12;
  UBYTE           freq_area;
  UBYTE           search_mode;
  UBYTE           sync_fail_count;
  UBYTE           bcch_fail_count;
  UBYTE           c_meas;
  UBYTE           c_max_meas;
  UBYTE           si_bitmap;
  T_TIME          c_tim_meas;
} T_CS_DATA;

#define MAX_OLD_CB  20
#define CBCH_BLOCK_SIZE      22
#define CBCH_BLOCKS           4
#define CBCH_MSG_SIZE      (CBCH_BLOCKS*CBCH_BLOCK_SIZE)

/* no alignment or gaps between members ! */
typedef struct cbmsg_header_t
{
  UBYTE   serial_no1;
  UBYTE   serial_no2;
  UBYTE   msg_id3;
  UBYTE   msg_id4;
  UBYTE   dcs;
  UBYTE   page;
} T_CBMSG_HEADER;

typedef struct
{
  UBYTE  cnt;
  T_CBMSG_HEADER header [MAX_OLD_CB];
} T_CB_MSG;

#define CBCH_CHANNELS 2
/*
 * index defines are in alr_cbch.c :
 * CBCH_NORM 0
 * CBCH_EXT  1
 */
typedef struct
{
  /* message which currently read */
  UBYTE      msg_is_extended;
  UBYTE      msg [CBCH_MSG_SIZE];
  USHORT     msg_id_sat [MAX_IDENTS_SAT];
  UBYTE      msg_type;
  UBYTE      sat_enabled;
  UBYTE      expected_seq_number[CBCH_CHANNELS];
  /* schedule support for normal and extended CBCH */
  UBYTE      begin_schedule[CBCH_CHANNELS];
  UBYTE      end_schedule[CBCH_CHANNELS];
  UBYTE      scheduled_sched_msg[CBCH_CHANNELS];
  UBYTE      schedule_length[CBCH_CHANNELS];
  UBYTE      last_slot_read[CBCH_CHANNELS];
  ULONG      schedule_map[CBCH_CHANNELS][2];
  /* configured by MMI: specifies which messages are wanted */
  UBYTE      modus;
  UBYTE      old_cid_plmn_lac [7];
  UBYTE      dcs_id [MAX_IDENTS];
  USHORT     msg_id [MAX_IDENTS];
  T_CB_MSG   old_cbch_msg;
#if defined(FF_HOMEZONE)
  /*
   * special handling for homezone CBCH message after cell reselecting
   */
  UBYTE      homezone;
  USHORT     msg_hz_id;
  UBYTE      dcs_hz_id;
  USHORT     hz_timeout;
 #endif  /* FF_HOMEZONE */
#if !defined (L1_STORES_CBCH_CONFIG)
  T_MPHC_CONFIG_CBCH_REQ     mphc_config_cbch;
#endif
} T_CBCH_DATA;

typedef struct
{
  UBYTE                  sys_info_1[22];
  UBYTE                  sys_info_2[22];
  UBYTE                  sys_info_2bis[22];
  UBYTE                  sys_info_2ter[22];
#if defined (REL99) && defined (TI_PS_FF_EMR)
  UBYTE                  sys_info_2quater[22];// we are storing only one instance.
#endif
  UBYTE                  sys_info_3[22];
  UBYTE                  sys_info_4[22];
  UBYTE                  sys_info_5[18];
  UBYTE                  sys_info_5bis[18];
  UBYTE                  sys_info_5ter[18];
  UBYTE                  sys_info_6[18];
#ifdef GPRS
  UBYTE                  sys_info_13[22];
#endif
} T_MA_DATA;

typedef struct
{
  /* information for using the cell */
  USHORT     ba_arfcn;
  UBYTE      last_rxlev;
  UBYTE      tim_valid;
  ULONG      frame_offset;
  ULONG      time_align;
  UBYTE      bsic;
  /* power measurements of the cell */
  UBYTE      rxlev[5];
  UBYTE      rxlev_average;
  UBYTE      c_rxlev;
  /* state of the cell */
  UBYTE      one_of_six;   /* this is a subset of one_of_twelve */
  UBYTE      rank_of_the_six; /*Nina add for calculate the rank of this cell*/
  UBYTE      syn_failure_times; /*Nina added to exclude the cells that failed to synch 6 times*/
  UBYTE      one_of_twelve;
  UBYTE      status;
  UBYTE      ba_status;
  UBYTE      new_strong_cell;
  /* counters for status handling */
  UBYTE      c_attempt;
  UBYTE      c_error;
  UBYTE      c_bcch;
  UBYTE      c_sync;
  USHORT     blocks_required;

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  UBYTE      c_sync_req;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

  /* Meas results for averaging when NC=1 or NC=2 */
#ifdef GPRS
  USHORT     nc_rxlev;
  UBYTE      c_nc_rxlev;
#endif
  /* temporary BCCH storage */
  T_MPH_UNITDATA_IND* mph_unitdata_ind;
  T_MPH_UNITDATA_IND* mph_unitdata_ind78;
  
} T_NC;

/* values for component tim_state */
#define NC_TIM_STOPPED       0
#define NC_CONF_ENABLED      1
#define NC_CONF_DISABLED     2
#define NC_CONF_PENDING      3

#if defined (REL99) && defined (TI_PS_FF_EMR)
typedef struct
{
  UBYTE     num_valid;
  UBYTE     index[MAX_NEIGHBOURCELLS];
  UBYTE     nc_index[MAX_NEIGHBOURCELLS];
  UBYTE     priority[MAX_NEIGHBOURCELLS];
} T_enh_bin;
#endif

typedef struct
{
  /* +1 because we use this for idle mode cell selection */
#define LAST_BSIC_REQ        BA_LIST_SIZE
  T_NC       cell[BA_LIST_SIZE+1];
  T_NC       cr_cell; /*  */
  UBYTE      c_ba_arfcn;
  UBYTE      ba_id;
  USHORT     channel;
  USHORT     rxlev_full;
  USHORT     rxlev_sub;
  UBYTE      rxqual_full;
  UBYTE      rxqual_sub;
  USHORT     tav;
  UBYTE      pwrc;
  UBYTE      dtx;
  UBYTE      act_dtx;
  UBYTE      update;
  UBYTE      sc_included;
  UBYTE      multiband;
  UBYTE      c_nc_timer;
  /*
   * Counter variable used to store 10sec timer interval 
   * in terms of  the number of 51 frame control multi-frames. 
   * This counter is decremented for every measurement indication
   * received from L1. This is initialized with TEN_SECONDS_ NCSYNC.
   */
  UBYTE      c_ncsync_tim; 
  /*
   *Variable to store the system time whenever 
   * the NCSYNC counter is re-assigned
   */
  T_TIME     ncsync_start_tim;
  /* for MPH_MEASUREMENT_IND */
  USHORT     fn_offset;
  UBYTE      max_reports;
  UBYTE      c_reports; 
  
#if defined (TI_PS_FF_RTD) AND defined (REL99)
  USHORT         nxt_ind;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */


  /*
   * counters to keep track of the send NCELL_SYNC_REQ's
   * and NCELL_BCCH_REQ's valid ranges are 0..12 and 0..6
   */
  UBYTE       c_sync_req;
  UBYTE       c_bcch_req;
  /* EOTD related */
  UBYTE       eotd_avail;
  UBYTE       tim_state;
  UBYTE       new_strong_cell_detect;
  UBYTE       c_sync_intrupted;
  T_MPH_NCELL_POS_IND* ppos_ind;
  T_MPH_NCELL_POS_REQ* ppos_req;
#if defined (REL99) && defined (TI_PS_FF_EMR)
/* EMR Related */
  T_MPH_ENHPARA_UPDATE_REQ  emr_data;  
  UBYTE       rxlev_val;
  UBYTE       nbr_rcvd_blks;
  UBYTE       m_bep;
  UBYTE       cv_bep;  
  UBYTE       si2_count; /*1 - SI2, 2 - SI-2/2bis, 3 - SI-2/2bis/2ter, 0 - none expected*/
  UBYTE       si2quater_status;
  UBYTE       si2quater_pos;
  UBYTE       si2q_per; /*counter for SI-2quater periodic configuration*/
  ULONG       rank_cell_bmp; /*The neighbor cell which has been ranked in sync req is marked in this bit map
                               at bit location corresponding to it's index in ALR's NC list*/
#endif
} T_NC_DATA;

#define SYSTEM_INFO_1_READ           0x01
#define SYSTEM_INFO_2_READ           0x02
#define SYSTEM_INFO_3_READ           0x04
#define SYSTEM_INFO_4_READ           0x08

#ifdef GPRS
#define SYSTEM_INFO_13_READ          0x10
#endif

#define ALL_SI_READ_EXCEPT_SI13      0x0F

typedef struct
{
  T_MPHC_START_CCCH_REQ  pl_idle, last_start_ccch_req;
  SHORT      imsi_mod_1000;
  UBYTE      dlt;
  UBYTE      act_dlt;
  UBYTE      si3_read;
  UBYTE      saved_page_mode;
  UBYTE      imsi [IMSI_LEN];
  UBYTE      v_tmsi;
  ULONG      tmsi;
  BOOL       reorg_bcch_reading;
  UBYTE      si_bitmap;
} T_PCH_DATA;

typedef struct
{
  UBYTE       ms_class;
  UBYTE       dcs_class;
  UBYTE       max_tx_pwr_ccch;
  UBYTE       delta [8];
  UBYTE       channel_request [8];
#ifdef GPRS
  UBYTE       t1 [8];
  UBYTE       t2 [8];
  UBYTE       t3 [8];
#endif
  UBYTE       max_bursts;
  UBYTE       c_burst;
} T_RACH_DATA;

typedef struct
{
  UBYTE             rlt;
  UBYTE             act_rlt;
  USHORT            act_bcch;
  UBYTE             act_mode;
  /*
   *   Description of the actual used
   *   channel, confirmed by layer 1
   */
  T_channel_desc            act_channel_desc;
  T_frequency_list          act_frequency_list;
  T_starting_time           act_starting_time;
  T_frequency_list_bef_sti  act_frequency_list_bef_sti;
  T_channel_desc            act_channel_desc_bef_sti;
  UBYTE                     act_channel_mode;
  BOOL                      act_pwrc;
  UBYTE                     act_cipher_mode;
  UBYTE                     act_a5_algorithm;
  T_cipher_key              act_cipher_key;
  T_amr_configuration       act_amr_configuration;
  UBYTE                     act_power;
  UBYTE                     act_dtx_allowed;
  /*
   *    Description of the old used
   *    channel.
   */
  USHORT                    old_bcch;
  UBYTE                     old_bsic;
  UBYTE                     old_channel;
  UBYTE                     old_channel_mode;
  T_channel_desc            old_channel_desc;
  T_frequency_list          old_frequency_list;
  T_starting_time           old_starting_time;
  T_frequency_list_bef_sti  old_frequency_list_bef_sti;
  T_channel_desc            old_channel_desc_bef_sti;
  BOOL                      old_pwrc;
  UBYTE                     old_cipher_mode;
  UBYTE                     old_a5_algorithm;
  T_cipher_key              old_cipher_key;
  T_amr_configuration       old_amr_configuration;
  UBYTE                     old_power;
  UBYTE                     old_dtx_allowed;

  /*
   *    Description of the new configured
   *    channel, not yet confirmed by layer 1.
   */
  USHORT                    temp_bcch;
  UBYTE                     temp_bsic;
  UBYTE                     temp_channel;
  UBYTE                     temp_channel_mode;
  T_channel_desc            temp_channel_desc;
  T_frequency_list          temp_frequency_list;
  T_starting_time           temp_starting_time;
  T_frequency_list_bef_sti  temp_frequency_list_bef_sti;
  T_channel_desc            temp_channel_desc_bef_sti;
  BOOL                      temp_pwrc;
  UBYTE                     temp_cipher_mode;
  UBYTE                     temp_a5_algorithm;
  T_cipher_key              temp_cipher_key;
  T_amr_configuration       temp_amr_configuration;
  UBYTE                     temp_power;
  UBYTE                     temp_dtx_allowed;

  /*
   *     Description of frequency redefinition
   *     parameter.
   */
  T_channel_desc    redef_channel_desc;
  T_frequency_list  redef_frequency_list;
  T_starting_time   redef_starting_time;
  /*
   *     Extended Measurmeent Order parameter.
   */
  UBYTE                     emo_ba_id;
#if defined (REL99) && defined (FF_BHO)
  UBYTE                     ho_type; /*To store Handover type during handover*/
  USHORT                    bho_cell_index; /*To store the index of the cell chosen for BHO, 
                                              NOT_PRESENT_8BIT indicates that cell is not in BA list*/
#endif
} T_DEDI_DATA;

#define TIM_NCSYNC_VALUE               10000
#define TIM_NW_SYNC_GUARD_VALUE         2500

/*
 *  timer
 */
#define TIMER_TRACE

#if defined (TIMER_TRACE)
#define TIMERSTART(i,v)                 tim_start_timer (i, v)
#define TIMERSTOP(i)                    tim_stop_timer (i)
#define IS_TIMER_ACTIVE(i)              tim_check_timer (i)
#else
#define TIMERSTART(i,v)
#define TIMERSTOP(i)
#define IS_TIMER_ACTIVE(i)
#endif /* TIMER_TRACE */

enum  tim_index_e
{
  TIM_POWERMEAS,
  TIM_NW_SYNC_GUARD,
#if defined(FF_HOMEZONE)
  TIM_HOMEZONE,
#endif  /* FF_HOMEZONE */
  NUM_OF_ALR_TIMERS
};

typedef  void (*T_TIMER_EXPIRY)(void);

typedef struct
{
  UBYTE             running[NUM_OF_ALR_TIMERS];
  T_TIMER_EXPIRY    expire[NUM_OF_ALR_TIMERS];
} T_TIM_DATA;

#ifdef OPTION_MULTITHREAD
  #define tim_init                  _ENTITY_PREFIXED(tim_init)
  #define tim_exec_timeout          _ENTITY_PREFIXED(tim_exec_timeout)
  #define tim_stop_timer            _ENTITY_PREFIXED(tim_stop_timer)
  #define tim_start_timer           _ENTITY_PREFIXED(tim_start_timer)
  #define tim_check_timer           _ENTITY_PREFIXED(tim_check_timer)
  #define trace_timer               _ENTITY_PREFIXED(trace_timer)
#endif

EXTERN void tim_init (void);
EXTERN void tim_exec_timeout (USHORT index);
EXTERN void tim_stop_timer (USHORT index);
EXTERN void tim_start_timer (USHORT index, T_TIME value);
EXTERN BOOL tim_check_timer(USHORT index);

/* all data is stored so, that it doesn't need
   to be converted if send to the upper layers.
   Exceptions are alr_data.cs_data.results,
                  alr_data.pch_data.pl_idle
                  alr_data.pch_data.imsi */

typedef struct
{
  UBYTE          state[MAX_STATE];
  /*
   * each of these elements is only
   * accessed by the appropiate
   * process
   */
  T_CBCH_DATA    cbch_data;
  T_CS_DATA      cs_data;
  T_MA_DATA      ma_data;
  T_NC_DATA      nc_data;
  T_PCH_DATA     pch_data;
  T_RACH_DATA    rach_data;
  T_DEDI_DATA    dedi_data;
#ifdef GPRS
  T_ALR_GPRS_DATA    gprs_data;
#endif  /* GPRS */
  T_TIM_DATA     tim_data;

  UBYTE          mmi;             /* mmi variant     */
  UBYTE          keypad;          /* keypad variant  */
  UBYTE          sc_band;         /* frequency band of the serving cell */
  BOOL           ext_display;     /* ext display     */
  EF_MSCAP       mscap;           /* MS capabilities (channel mode, data cap., features */
                                  /* is this component really needed ?? */
  /*
   * these parameters are used by more than one process
   * this is to avoid data inconsistencies between
   * the different processes
   */
  USHORT         serving_cell;
  USHORT         old_serving_cell;
  UBYTE          bsic;
  UBYTE          bs_pa_mfrms;
  UBYTE          ncc_permitted;
  UBYTE          plmn_search_running;
#ifdef GPRS
  UBYTE          nc_sync_with_grr;
  UBYTE          nwctrl_meas_active; /* indicates whether averaging when NC=1,2 is active */
#endif
#ifdef _SIMULATION_
  UBYTE          mb_testing; /* Dynamic configuration flag to indicate multiband testing */
#endif
} T_ALR_DATA;

/*==== EXPORT =====================================================*/
/*
 *  Main Control
 */
EXTERN void ma_init                    (void);
/*
 *  Primitive processing functions for upper layer (RR)
 */
EXTERN void ma_mph_idle_req                 (T_MPH_IDLE_REQ                 *mph_idle_req);
EXTERN void ma_mph_neighbourcell_req        (T_MPH_NEIGHBOURCELL_REQ        *mph_neighbourcell_req);
EXTERN void ma_mph_emo_req                  (T_MPH_EMO_REQ                  *mph_emo_req);
EXTERN void ma_mph_dedicated_req            (T_MPH_DEDICATED_REQ            *mph_dedicated_req);
EXTERN void ma_mph_dedicated_fail_req       (T_MPH_DEDICATED_FAIL_REQ       *mph_dedicated_fail_req);
EXTERN void ma_mph_ciphering_req            (T_MPH_CIPHERING_REQ            *mph_ciphering_req);
EXTERN void ma_mph_freq_redef_req           (T_MPH_FREQ_REDEF_REQ           *mph_freq_redef_req);
EXTERN void ma_mph_channel_mode_req         (T_MPH_CHANNEL_MODE_REQ         *mph_channel_mode_req);
EXTERN void ma_mph_deactivate_req           (T_MPH_DEACTIVATE_REQ           *mph_deactivate_req);
EXTERN void ma_mph_classmark_req            (T_MPH_CLASSMARK_REQ            *mph_classmark_req);
EXTERN void ma_mph_ext_meas_req             (T_MPH_EXT_MEAS_REQ             *mph_ext_meas_req);
EXTERN void ma_mph_tch_loop_req             (T_MPH_TCH_LOOP_REQ             *mph_tch_loop_req);
EXTERN void ma_mph_dai_req                  (T_MPH_DAI_REQ                  *mph_dai_req);
EXTERN void ma_mph_cbch_req                 (T_MPH_CBCH_REQ                 *mph_cbch_req);
EXTERN void ma_mph_identity_req             (T_MPH_IDENTITY_REQ             *mph_identity_req);
EXTERN void ma_mph_power_req                (T_MPH_POWER_REQ                *mph_power_req);
EXTERN void ma_mph_bsic_req                 (T_MPH_BSIC_REQ                 *mph_bsic_req);
EXTERN void ma_mph_random_access_req        (T_MPH_RANDOM_ACCESS_REQ        *mph_random_access_req);
EXTERN void ma_mph_sync_req                 (T_MPH_SYNC_REQ                 *mph_sync_req);
EXTERN void ma_mph_ncell_pos_req            (T_MPH_NCELL_POS_REQ            *mph_ncell_pos_req);
EXTERN void ma_mph_clean_buf_req            (T_MPH_CLEAN_BUF_REQ            *mph_clean_buf_req);
EXTERN void ma_mph_stop_dedi_req            (T_MPH_STOP_DEDICATED_REQ       *mph_stop_dedi_req);
#ifdef GPRS
EXTERN void ma_mph_meas_rep_req             (T_MPH_MEAS_REP_REQ             *mph_meas_rep_req);
#endif
/*
 *  Primitive processing functions for lower layer
 */
EXTERN void ma_mphc_rxlev_ind               (T_MPHC_RXLEV_IND               *rxlev_ind);
EXTERN void ma_mphc_stop_rxlev_cnf          (T_MPHC_STOP_RXLEV_CON          *rxlev_con);
EXTERN void ma_mphc_empty_cnf               (void                           *);
EXTERN void ma_mphc_network_sync_ind        (T_MPHC_NETWORK_SYNC_IND        *network_sync_ind);
EXTERN void ma_cs_stop_network_sync_req     (void);
EXTERN void ma_mphc_new_scell_cnf           (T_MPHC_NEW_SCELL_CON           *new_Scell_cnf);
EXTERN void ma_mphc_ncell_bcch_ind          (T_MPHC_NCELL_BCCH_IND          *ncell_bcch_data_ind);
EXTERN void ma_mphc_ncell_sync_ind          (T_MPHC_NCELL_SYNC_IND          *ncell_sync_ind);
GLOBAL void ma_mphc_stop_ncell_sync_cnf     (T_MPHC_STOP_NCELL_SYNC_CON     *ncell_sync_cnf );
EXTERN void ma_mphc_rxlev_periodic_ind      (T_MPHC_RXLEV_PERIODIC_IND      *rxlev_periodic_ind);
EXTERN void ma_mphc_ra_cnf                  (T_MPHC_RA_CON                  *ra_cnf);
EXTERN void ma_mphc_data_ind                (T_MPHC_DATA_IND                *data_ind);
EXTERN void ma_mphc_change_frequency_cnf    (T_MPHC_CHANGE_FREQUENCY_CON    *change_frequency_cnf);
EXTERN void ma_mphc_async_ho_cnf            (T_MPHC_ASYNC_HO_CON            *async_ho_cnf);
EXTERN void ma_mphc_channel_assign_cnf      (T_MPHC_CHANNEL_ASSIGN_CON      *channel_assign_cnf);
EXTERN void ma_mphc_handover_fail_cnf       (T_MPHC_HANDOVER_FAIL_CON       *handover_fail_cnf);
EXTERN void ma_mphc_immed_assign_cnf        (T_MPHC_IMMED_ASSIGN_CON        *immed_assign_cnf);
EXTERN void ma_mphc_pre_sync_ho_cnf         (T_MPHC_PRE_SYNC_HO_CON         *pre_sync_ho_cnf);
EXTERN void ma_mphc_sync_ho_cnf             (T_MPHC_SYNC_HO_CON             *sync_ho_cnf);
EXTERN void ma_mphc_ta_fail_ind             (T_MPHC_TA_FAIL_IND             *ta_fail_ind);
EXTERN void ma_mphc_handover_finished_ind   (T_MPHC_HANDOVER_FINISHED       *handover_finished);
EXTERN void ma_mphc_meas_report_ind         (T_MPHC_MEAS_REPORT             *meas_report);
EXTERN void ma_mphc_adc_ind                 (T_MPHC_ADC_IND                 *adc_results);
EXTERN void ma_mphc_init_l1_cnf             (T_MPHC_INIT_L1_CON             *init_l1_cnf);
EXTERN void ma_mphc_stop_dedi_con           (T_MPHC_STOP_DEDICATED_CON      *stop_cnf);
#if defined(STOP_SYNC_TASK)
EXTERN void ma_mphc_stop_network_sync_cnf   (T_MPHC_STOP_NETWORK_SYNC_CON   *stop_network_sync_cnf);
#endif /* STOP_SYNC_TASK */
/* Signal processing functions for process Cell Selection */
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
EXTERN void ma_cs_init_l1_req               (void);
#else
EXTERN void ma_cs_init_l1_req               (UBYTE                           radio_band_config);
#endif
EXTERN void ma_cs_stop_network_sync_req     (void);
EXTERN void ma_cs_network_sync_req          (USHORT                          channel);
EXTERN void ma_cs_rxlev_req                 (void);
EXTERN void ma_cs_power_cnf                 (T_MPH_POWER_CNF                *mph_power_cnf);

/* Signal processing functions for process Neigbour Cell */
EXTERN void ma_nc_rxlev_periodic_req        (T_MPHC_RXLEV_PERIODIC_REQ      *update);
EXTERN void ma_nc_stop_ncell_bcch_req       (USHORT                          arfcn);
EXTERN void ma_nc_stop_ncell_sync_req       (USHORT                          arfcn);
EXTERN void ma_nc_sync_req                  (T_MPHC_NCELL_SYNC_REQ          *sync_req);
EXTERN void ma_nc_list_sync_req             (T_MPHC_NCELL_LIST_SYNC_REQ     *list_sync_req);
EXTERN void ma_nc_bcch_req                  (T_MPHC_NCELL_BCCH_REQ          *bcch_req);
EXTERN void ma_nc_update_ba_list            (T_MPHC_UPDATE_BA_LIST          *update_ba_list);
EXTERN void nc_new_serving_cell             (USHORT                          serving_cell);
EXTERN void ma_nc_report_res                (T_MPH_MEASUREMENT_IND          *mph_measurement_ind);

/* Signal processing functions for process RACH_Contol */
EXTERN void ma_rach_stop_ra_req             (void);
EXTERN void ma_rach_ra_req                  (T_MPHC_RA_REQ                  *mph_ra_req);
EXTERN void ma_rach_random_cnf              (T_MPH_RANDOM_ACCESS_CNF        *mph_random_access_cnf);

/* Signal processing functions for process Dedi_Control */
EXTERN void ma_dedi_stop_req (void);
EXTERN void ma_dedi_cnf (UBYTE cause);
EXTERN void ma_dedi_chan_ass_req (T_MPHC_CHANNEL_ASSIGN_REQ *mphc_channel_assign_req);
EXTERN void ma_dedi_async_ho_req (T_MPHC_ASYNC_HO_REQ *async_ho_req);
EXTERN void ma_dedi_sync_ho_req (T_MPHC_SYNC_HO_REQ *sync_ho_req);
EXTERN void ma_dedi_pre_sync_ho_req (T_MPHC_PRE_SYNC_HO_REQ *pre_sync_ho_req);
EXTERN void ma_dedi_fail_cnf (void);
EXTERN void ma_dedi_ho_fail_req (void);
EXTERN void ma_dedi_imm_ass_req (T_MPHC_IMMED_ASSIGN_REQ *immed_assign_req);
EXTERN void ma_dedi_change_freq_req (T_MPHC_CHANGE_FREQUENCY *change_frequency);
EXTERN void ma_dedi_chan_mode_req (T_MPHC_CHANNEL_MODE_MODIFY_REQ *channel_mode_modify_req);
EXTERN void ma_dedi_ciph_req (T_MPHC_SET_CIPHERING_REQ *set_ciphering_req);
EXTERN void ma_dedi_start_dai_req (T_OML1_START_DAI_TEST_REQ *oml1_start_dai_test_req);
EXTERN void ma_dedi_close_tch_loop_req (T_OML1_CLOSE_TCH_LOOP_REQ *oml1_close_tch_loop_req);
EXTERN void ma_dedi_stop_dai_req (void);
EXTERN void ma_dedi_open_tch_loop_req (void);

/* Signal processing functions for process PCH_control */
EXTERN void ma_pch_start_ccch_req           (T_MPHC_START_CCCH_REQ          *pl_idle);
EXTERN void ma_pch_paging_ind               (UBYTE                           id_type,
                                             UBYTE                           channel_needed);
EXTERN void ma_pch_stop                     (void);

/* Signal processing functions for process MMI_control */
EXTERN void ma_mmi_cbch_req                 (T_MMI_CBCH_REQ                 *cbch_req);
EXTERN void ma_mmi_sat_cbch_dwnld_req       (T_MMI_SAT_CBCH_DWNLD_REQ        *cbch_req);
/* Signal processing functions for process CBCH_control */
EXTERN void ma_cb_stop_cbch_req             (void);
EXTERN void ma_cb_sched_req                 (T_MPHC_CBCH_SCHEDULE_REQ       *sched_req);
EXTERN void ma_cb_config_cbch               (T_MPHC_CONFIG_CBCH_REQ         *config_cbch);
EXTERN void ma_cb_mmi_cbch_ind              (T_MMI_CBCH_IND                 *mmi_cbch_ind);
EXTERN void ma_cb_info_req                  (UBYTE                           bitmap);

/* Signal processing functions used by more then one process */
EXTERN void ma_scell_nbcch_req              (T_MPHC_SCELL_NBCCH_REQ         *nbcch_req);
EXTERN void ma_scell_ebcch_req              (T_MPHC_SCELL_EBCCH_REQ         *ebcch_req);
EXTERN void ma_stop_scell_bcch_req          (void);
EXTERN void ma_stop_active_procs            (UBYTE flags);
EXTERN void ma_bsic_cnf                     (T_MPH_BSIC_CNF                 *mph_bsic_cnf);
EXTERN void ma_send_unitdata                (T_MPHC_DATA_IND                *data_ind);
EXTERN void ma_error_ind                    (UBYTE                           cause,
                                             USHORT                          arfcn);
EXTERN void ma_clean_dedi_sys_buffer        (void);
EXTERN void ma_new_scell_req                (T_MPHC_NEW_SCELL_REQ           *new_scell);
EXTERN void ma_scell_full_nbcch             (void);
#ifdef GPRS
EXTERN BOOL ma_is_ptm                       (void);
EXTERN void ma_nc_rxlev_sc_req              (T_TB_RXLEV_SC_REQ              *rxlev_sc_req);
#endif
EXTERN void ma_clean_sys_buffer             (USHORT                          si_mask);
#if defined (REL99) && defined (TI_PS_FF_EMR)
GLOBAL void ma_send_stored_SI2qtr           (UBYTE                          *si_2quater);
EXTERN void ma_mph_enhpara_update_req       (T_MPH_ENHPARA_UPDATE_REQ       * p_enh);
EXTERN void ma_mph_mon_ctrl_req             (T_MPH_MON_CTRL_REQ             * ctrl_req);
EXTERN void ma_scell_mon_si2quater_nbcch    (void);
EXTERN void ma_scell_mon_si2quater_ebcch    (void);
#endif
/* defined in dl_pei.c */
EXTERN void dl1_downlink_indication         (UBYTE                           chan,
                                             UBYTE                          *frame);

/*
 *  CBCH Control
 */
EXTERN void   cb_init                       (void);
EXTERN void   cb_stop                       (void);
EXTERN void   cb_start                      (void);
EXTERN void   cb_mmi_cbch_req               (T_MMI_CBCH_REQ                  *cbch);
EXTERN void   cb_mph_cbch_req               (T_cbch                          *cbch);
EXTERN void   cb_data_ind                   (T_MPHC_DATA_IND                 *cbch_ind);
EXTERN void   cb_mmi_sat_cbch_req           (T_MMI_SAT_CBCH_DWNLD_REQ        *cbch_req);
#if defined(FF_HOMEZONE)
EXTERN void   cb_tim_homezone (void);
#endif  /* FF_HOMEZONE */

/*
 *  Cell Selection
 */
EXTERN void   cs_init                       (void);
EXTERN void   cs_rxlev_ind                  (T_MPHC_RXLEV_IND               *rxlev_ind);
EXTERN void   cs_network_sync_ind           (T_MPHC_NETWORK_SYNC_IND        *network_sync_ind);
EXTERN void   cs_stop                       (void);
EXTERN T_POWER_MEAS* cs_prepare_power_req   (void);
EXTERN void   cs_power_req                  (UBYTE                           pch_interrupt);
EXTERN void   cs_bsic_req                   (T_MPH_BSIC_REQ                 *mph_bsic_req);
EXTERN void   cs_read_scell_bcch            (void);
EXTERN UBYTE  get_band                      (USHORT                          arfcn);
EXTERN void   cs_increment_bfc              (void);
EXTERN void   cs_decrement_bfc              (void);
EXTERN void   cs_set_wideband_sync          (void);

/*
 *  Dedicated Control
 */
EXTERN void   dedi_init                     (void);
EXTERN void   dedi_req                      (T_MPH_DEDICATED_REQ            *dedicated_req);
EXTERN void   dedi_fail_req                 (void);
EXTERN void   dedi_ta_fail_ind              (void);
EXTERN void   dedi_ho_finished              (T_MPHC_HANDOVER_FINISHED       *ho_finished);
EXTERN void   dedi_freq_redef_req           (T_MPH_FREQ_REDEF_REQ           *freq);
EXTERN void   dedi_chan_mode_req            (T_MPH_CHANNEL_MODE_REQ         *ch_mode);
EXTERN void   dedi_ciph_req                 (T_MPH_CIPHERING_REQ            *ciph_req);
EXTERN void   dedi_tch_loop_req             (T_MPH_TCH_LOOP_REQ             *tloop);
EXTERN void   dedi_dai_req                  (T_MPH_DAI_REQ                  *dai);
EXTERN void   dedi_change_freq_cnf          (void);
EXTERN void   dedi_ho_cnf                   (T_HO_TYPE                       ho_type);
EXTERN void   dedi_chan_ass_cnf             (void);
EXTERN void   dedi_ho_fail_cnf              (void);
EXTERN void   dedi_imm_ass_cnf              (void);
EXTERN void   dedi_stop                     (void);
EXTERN void   dedi_increment_rlt            (void);
EXTERN void   dedi_decrement_rlt            (void);

/*
 *  MMI Control
 */
EXTERN USHORT RX_GetValue                   (void);

/*
 *  Idle Mode Neighbour Cell
 */
EXTERN void   nc_init                       (void);
EXTERN void   nc_ncell_list                 (T_MPH_NEIGHBOURCELL_REQ        *mph_neighbourcell_req);
EXTERN void   nc_update_ba_list             (USHORT serving_cell, T_MPH_NEIGHBOURCELL_REQ *ncell_list);
EXTERN void   nc_stop                       (void);
EXTERN void   nc_bsic_req                   (T_MPH_BSIC_REQ                 *mph_bsic_req);
EXTERN void   nc_sync_ind                   (T_MPHC_NCELL_SYNC_IND          *sync_ind);
EXTERN void   nc_bcch_ind                   (T_MPHC_NCELL_BCCH_IND          *data_ind);
EXTERN void   nc_start_dedicated            (UBYTE                           pwrc,
                                             UBYTE                           dtx);
EXTERN void   nc_update_dedicated           (UBYTE                           dtx,
                                             UBYTE                           pwrc);
EXTERN void   nc_update_list                (USHORT                          channel);
EXTERN BOOL   nc_get_fn_time                (USHORT                          channel,
                                             ULONG                           *fn,
                                             ULONG                           *time);
EXTERN void   nc_suspend                    (void);
EXTERN void   nc_add_offset                 (void);
EXTERN void   nc_report                     (T_MPHC_RXLEV_PERIODIC_IND      *rxlev_periodic_ind);
EXTERN void   nc_start_reselect             (USHORT                          arfcn);
EXTERN void   nc_start_monitoring           (void);
EXTERN void   nc_report_dedicated           (T_MPHC_MEAS_REPORT             *meas_report);
EXTERN void   nc_fill_report_sc_dedi        (T_MPH_MEASUREMENT_IND          *rr_report,
                                             UBYTE                           ncells);
EXTERN void   nc_store_tav                  (USHORT                          tav);
EXTERN void   nc_check_new_ncc_permitted    (UBYTE                           new_ncc_permitted);
EXTERN void   nc_stop_rr_activity           (UBYTE                           stop);
EXTERN void   nc_stop_ext_meas_ind          (void);
EXTERN void   nc_resume                     (void);
EXTERN void   nc_check_activity             (void);
EXTERN void   nc_set_status                 (USHORT                          index,
                                             UBYTE                           new_status);
EXTERN USHORT  nc_get_index                 (USHORT arfcn);
EXTERN void    nc_ncsync_tim_expiry         (void);
EXTERN void    nc_ncell_pos_req             (T_MPH_NCELL_POS_REQ*            pos_req);
GLOBAL void nc_resume_dedicated(void);
GLOBAL void nc_suspend_handover (void);
EXTERN BOOL nc_is_in_ba(USHORT arfcn);
EXTERN USHORT nc_find_serving_cell_entry (USHORT serving_cell);

#ifdef GPRS
EXTERN void    nc_start_pbcch(void);
EXTERN void    nc_build_nwctrl_rr_report    (T_MPH_MEAS_REP_CNF             *rr_report);
#endif

/*
 *  PCH Control
 */
EXTERN void   pch_init                      (void);
EXTERN void   pch_configure                 (T_MPH_IDLE_REQ                 *idle,
                                             UBYTE page_mode);
EXTERN void   pch_start_ccch_req            (void);
EXTERN void   pch_save_pgm                  (UBYTE                           mode);
EXTERN void   pch_identity_req              (T_MPH_IDENTITY_REQ             *mph_identity_req);
EXTERN void   pch_check_pag_3               (T_MPHC_DATA_IND                *data_ind);
EXTERN void   pch_check_pag_2               (T_MPHC_DATA_IND                *data_ind);
EXTERN void   pch_check_pag_1               (T_MPHC_DATA_IND                *data_ind);
EXTERN void   pch_check_page_mode           (T_MPHC_DATA_IND                *data_ind);
EXTERN void   pch_check_page_mode_cr        (T_MPHC_DATA_IND                *data_ind);
EXTERN void   pch_decrement_dlt             (void);
EXTERN void   pch_increment_dlt             (void);
EXTERN void   pch_stop                      (void);
EXTERN UBYTE  pch_mode_reorg                (void);
EXTERN void   pch_config_resel              (T_MPHC_DATA_IND                *data_ind);
/*
 *  RACH Control
 */
EXTERN void   rach_init                     (void);
EXTERN void   rach_configure_power          (UBYTE                           power);
EXTERN void   rach_configure_class          (UBYTE                           class,
                                             UBYTE                           dcs_class);
EXTERN void   rach_stop                     (void);
EXTERN UBYTE  cut_power_value               (UBYTE                           power,
                                             T_MPH_DEDICATED_REQ            *mph_dedicated_req);

EXTERN void   rach_random_req               (T_MPH_RANDOM_ACCESS_REQ        *random_req);
EXTERN void   rach_ra_cnf                   (T_MPHC_RA_CON*                  ra_cnf);

/*
 *  Idle Mode Serving Cell
 */
EXTERN void   sc_start_periodic             (void);
EXTERN void csf_show_version (T_TST_TEST_HW_CON  *ver);
EXTERN void   tim_stop_sync_to_nw           (void);



EXTERN UBYTE  grlc_test_mode_active(void);				/*lint -esym(526,grlc_test_mode_active) : not defined | defined in other entity */
#if defined (_SIMULATION_)
EXTERN void rr_csf_check_rfcap (UBYTE init);			/*lint -esym(526,rr_csf_check_rfcap)   : not defined | defined in other entity */
#endif  /* _SIMULATION_ */
EXTERN CHAR* alr_version(void);

#if defined(ALR_CSF_C) && defined(_TMS470)
EXTERN void  Cust_Init_Layer1(void);
EXTERN void  SIM_PowerOff (void);
EXTERN void  TM_EnableWatchdog (void);
EXTERN void  TM_ResetWatchdog(SYS_UWORD16 count);
#endif  /* ALR_CSF_C && _TMS470 */

#if defined(WIN32)
#include <stdio.h>  /* prototype of sprintf() */
#endif  /* WIN32 */

#if !defined NTRACE
EXTERN void trc_mon_counter_idle (UBYTE act_dlt, UBYTE max_dlt);
EXTERN void trc_mon_counter_dedi (UBYTE act_rlt, UBYTE max_rlt);
#endif /* (!defined NTRACE ) */


EXTERN  T_ALR_DATA              alr_data_base;
#define GET_INSTANCE_DATA    register T_ALR_DATA *alr_data= &alr_data_base

#ifdef WIN32
  EXTERN T_POWER_MEAS tap_rxlev_response_european[];
  EXTERN T_POWER_MEAS tap_rxlev_response_american[];
#endif

#define ENTITY_DATA                alr_data

/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
#ifdef TI_PS_HCOMM_CHANGE
#else /* for hCommHandles backward compatibility */
  #define hCommL1         _ENTITY_PREFIXED(hCommL1)
  #define hCommPL         _ENTITY_PREFIXED(hCommPL)
  #define hCommDL         _ENTITY_PREFIXED(hCommDL)
  #define hCommRR         _ENTITY_PREFIXED(hCommRR)
  #define hCommMMI        _ENTITY_PREFIXED(hCommMMI)
  #define hCommCST        _ENTITY_PREFIXED(hCommCST)
#endif
#ifdef GPRS
  #define hCommGPL        _ENTITY_PREFIXED(hCommGPL)
#endif /* #ifdef GPRS */
#endif

#ifdef TI_PS_HCOMM_CHANGE
#else /* for hCommHandles backward compatibility */
EXTERN T_HANDLE  hCommPL;              /* PL   own handle          */
EXTERN T_HANDLE  hCommL1;              /* L1   Communication       */
EXTERN T_HANDLE  hCommDL;              /* DL   Communication       */
EXTERN T_HANDLE  hCommRR;              /* RR   Communication       */
EXTERN T_HANDLE  hCommMMI;             /* MMI  Communication       */
EXTERN T_HANDLE  hCommCST;             /* CST  Communication       */
#endif
#ifdef GPRS
EXTERN T_HANDLE  hCommGPL;             /* GPL  Communication       */
#endif /* GPRS */
EXTERN T_HANDLE  pl_handle;

EXTERN UBYTE          v_mon_counter_idle;
EXTERN UBYTE          v_mon_counter_dedi;
EXTERN UBYTE          v_eotd;

#if defined(DL_TRACE_ENABLED)
  EXTERN void dl_trace                   (UCHAR                    trace_type,
                                          UCHAR                    channel,
                                          UCHAR                    ch_type,
                                          UCHAR                  * data);
  #ifdef OPTION_MULTITHREAD
    #define dl_trace_buf    _ENTITY_PREFIXED(dl_trace_buf)
  #endif
  #if defined(ALR_MAIN_C)
    GLOBAL char dl_trace_buf[100];
  #else  /* ALR_MAIN_C */
    EXTERN char dl_trace_buf[100];
  #endif  /* ALR_MAIN_C */
  #define TRACE_PL_EVENT   4
  #define DOES_NOT_MATTER  0
  #define DL_OFFLINE_TRACE(s) \
    dl_trace (TRACE_PL_EVENT, DOES_NOT_MATTER, DOES_NOT_MATTER, (UCHAR*)s)
#else  /* DL_TRACE_ENABLED */
  #define DL_OFFLINE_TRACE(s)
#endif  /* DL_TRACE_ENABLED */

#if !defined(NCONFIG) && !defined(NTRACE) && defined(TRACE_STATE_TRANSITION)
void trc_state_transition(int line, UBYTE newstate);
#else
#define trc_state_transition(line,newstate)
#endif  /* !NCONFIG && !NTRACE && TRACE_STATE_TRANSITION */

#if defined(_SIMULATION_)
#define TRACING
#define TRACE_EVENT_WIN(s)                                TRACE_EVENT(s)
#define TRACE_EVENT_WIN_P1(s,a1)                          TRACE_EVENT_P1(s,a1)
#define TRACE_EVENT_WIN_P2(s,a1,a2)                       TRACE_EVENT_P2(s,a1,a2)
#define TRACE_EVENT_WIN_P3(s,a1,a2,a3)                    TRACE_EVENT_P3(s,a1,a2,a3)
#define TRACE_EVENT_WIN_P4(s,a1,a2,a3,a4)                 TRACE_EVENT_P4(s,a1,a2,a3,a4)
#define TRACE_EVENT_WIN_P5(s,a1,a2,a3,a4,a5)              TRACE_EVENT_P5(s,a1,a2,a3,a4,a5)
#define TRACE_EVENT_WIN_P6(s,a1,a2,a3,a4,a5,a6)           TRACE_EVENT_P6(s,a1,a2,a3,a4,a5,a6)
#define TRACE_EVENT_WIN_P7(s,a1,a2,a3,a4,a5,a6,a7)        TRACE_EVENT_P7(s,a1,a2,a3,a4,a5,a6,a7)
#define TRACE_EVENT_WIN_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)     TRACE_EVENT_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)
#define TRACE_EVENT_WIN_P9(s,a1,a2,a3,a4,a5,a6,a7,a8,a9)  TRACE_EVENT_P9(s,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#else  /* _SIMULATION_ */
#define TRACE_EVENT_WIN(s)
#define TRACE_EVENT_WIN_P1(s,a1)
#define TRACE_EVENT_WIN_P2(s,a1,a2)
#define TRACE_EVENT_WIN_P3(s,a1,a2,a3)
#define TRACE_EVENT_WIN_P4(s,a1,a2,a3,a4)
#define TRACE_EVENT_WIN_P5(s,a1,a2,a3,a4,a5)
#define TRACE_EVENT_WIN_P6(s,a1,a2,a3,a4,a5,a6)
#define TRACE_EVENT_WIN_P7(s,a1,a2,a3,a4,a5,a6,a7)
#define TRACE_EVENT_WIN_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)
#define TRACE_EVENT_WIN_P9(s,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#endif  /* _SIMULATION_ */

#if defined TRACING
#define ALR_TRACE(a) vsi_o_ttrace(VSI_CALLER TC_EVENT,a);
#else
#define ALR_TRACE(a)
#endif  /* TRACING */

#define TNNN_WITH_TIMER

/*
#define ALR_TRACE_ENABLED
*/
#if defined(ALR_TRACE_ENABLED)
  void alr_trc_init (void);
  void alr_trc_exit (void);
  void alr_trc_store (UCHAR event, UCHAR channel, UCHAR state, void* l2_data);
  void alr_trc_read_all (void);
  void alr_trc_read (int count);
#else  /* ALR_TRACE_ENABLED */
#define alr_trc_init()
#define alr_trc_exit()
#define alr_trc_store(e,c,s,p)
#define alr_trc_read(c)
#define alr_trc_read_all()
#endif  /* ALR_TRACE_ENABLED */

/*
 * ALR TRACE defines
 */
#define ALR_TRC_UPLINK        0
#define ALR_TRC_DOWNLINK      1
#define ALR_TRC_EVENT         2
#define ALR_TRC_STRING        3

#define ALR_TRC_CH_UNDEF      0
#define ALR_TRC_CH_FACCH_FR   1
#define ALR_TRC_CH_FACCH_HR   2
#define ALR_TRC_CH_SDCCH4     3
#define ALR_TRC_CH_SDCCH8     4
#define ALR_TRC_CH_SACCH      5
#define ALR_TRC_CH_UNKNOWN    6

#define ALR_TRC_DATA_SIZE     40

#define ALR_RXLEV_AVERAGE_MIN 0
#define ALR_RXLEV_AVERAGE_LWR_THR 4

#endif  /* ALR_H */
