/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_cgrlc.h                                                      |
| SOURCE  : "sap\cgrlc.pdf"                                                |
| LastModified : "2004-05-17"                                              |
| IdAndVersion : "8010.119.008.04"                                         |
| SrcFileTime  : "Thu Nov 29 09:38:02 2007"                                |
| Generated by CCDGEN_2.5.5A on Thu Sep 25 09:52:55 2014                   |
|           !!DO NOT MODIFY!!DO NOT MODIFY!!DO NOT MODIFY!!                |
+--------------------------------------------------------------------------+
*/

/* PRAGMAS
 * PREFIX                 : CGRLC
 * COMPATIBILITY_DEFINES  : NO
 * ALWAYS_ENUM_IN_VAL_FILE: NO
 * ENABLE_GROUP: NO
 * CAPITALIZE_TYPENAME: NO
 */


#ifndef P_CGRLC_H
#define P_CGRLC_H


#define CDG_ENTER__P_CGRLC_H

#define CDG_ENTER__FILENAME _P_CGRLC_H
#define CDG_ENTER__P_CGRLC_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_CGRLC_H__LAST_MODIFIED _2004_05_17
#define CDG_ENTER__P_CGRLC_H__ID_AND_VERSION _8010_119_008_04

#define CDG_ENTER__P_CGRLC_H__SRC_FILE_TIME _Thu_Nov_29_09_38_02_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_CGRLC_H

#undef CDG_ENTER__FILENAME


#include "p_cgrlc.val"

#ifndef __T_CGRLC_fix_alloc_struct__
#define __T_CGRLC_fix_alloc_struct__
/*
 * Fixed Allocation structure
 * CCDGEN:WriteStruct_Count==1637
 */
typedef struct
{
  U8                        bitmap_len;               /*<  0:  1> Bitmap length                                      */
  U8                        bitmap_array[127];        /*<  1:127> Bitmap array                                       */
  U32                       end_fn;                   /*<128:  4> End of bitmap framenumber                          */
  U8                        final_alloc;              /*<132:  1> Final allocation                                   */
  U8                        _align0;                  /*<133:  1> alignment                                          */
  U8                        _align1;                  /*<134:  1> alignment                                          */
  U8                        _align2;                  /*<135:  1> alignment                                          */
} T_CGRLC_fix_alloc_struct;
#endif

#ifndef __T_CGRLC_freq_param__
#define __T_CGRLC_freq_param__
/*
 * Frequency Parameters
 * CCDGEN:WriteStruct_Count==1638
 */
typedef struct
{
  U16                       bcch_arfcn;               /*<  0:  2> ARFCN of the BCCH                                  */
  U8                        pdch_hopping;             /*<  2:  1> Hopping or no hopping is used on the assigned PDCH */
  U8                        pdch_band;                /*<  3:  1> PDCH band                                          */
} T_CGRLC_freq_param;
#endif

#ifndef __T_CGRLC_pwr_ctrl_param__
#define __T_CGRLC_pwr_ctrl_param__
/*
 * Power Control Parameters
 * CCDGEN:WriteStruct_Count==1639
 */
typedef struct
{
  U8                        alpha;                    /*<  0:  1> Alpha                                              */
  U8                        gamma_ch[CGRLC_MAX_TIMESLOTS]; /*<  1:  8> Gamma                                              */
  U8                        _align0;                  /*<  9:  1> alignment                                          */
  U8                        _align1;                  /*< 10:  1> alignment                                          */
  U8                        _align2;                  /*< 11:  1> alignment                                          */
} T_CGRLC_pwr_ctrl_param;
#endif

#ifndef __T_CGRLC_c_value__
#define __T_CGRLC_c_value__
/*
 * C-Value
 * CCDGEN:WriteStruct_Count==1640
 */
typedef struct
{
  S32                       c_lev;                    /*<  0:  4> C-value raw data level                             */
  U16                       c_idx;                    /*<  4:  2> C-value raw data index                             */
  U16                       c_acrcy;                  /*<  6:  2> C-value raw data accuracy                          */
} T_CGRLC_c_value;
#endif

#ifndef __T_CGRLC_pan_struct__
#define __T_CGRLC_pan_struct__
/*
 * Pan Structure
 * CCDGEN:WriteStruct_Count==1641
 */
typedef struct
{
  U8                        inc;                      /*<  0:  1> Pan increment                                      */
  U8                        dec;                      /*<  1:  1> Pan decrement                                      */
  U8                        pmax;                     /*<  2:  1> Pan maximum                                        */
  U8                        _align0;                  /*<  3:  1> alignment                                          */
} T_CGRLC_pan_struct;
#endif

#ifndef __T_CGRLC_glbl_pwr_ctrl_param__
#define __T_CGRLC_glbl_pwr_ctrl_param__
/*
 * Global Power Control Parameters
 * CCDGEN:WriteStruct_Count==1642
 */
typedef struct
{
  U8                        alpha;                    /*<  0:  1> Alpha                                              */
  U8                        t_avg_t;                  /*<  1:  1> T_AVG_T                                            */
  U8                        pb;                       /*<  2:  1> Power reduction value                              */
  U8                        pc_meas_chan;             /*<  3:  1> PC_MEAS_CHAN                                       */
  U8                        pwr_max;                  /*<  4:  1> Maximum output power of the MS.                    */
  U8                        _align0;                  /*<  5:  1> alignment                                          */
  U8                        _align1;                  /*<  6:  1> alignment                                          */
  U8                        _align2;                  /*<  7:  1> alignment                                          */
} T_CGRLC_glbl_pwr_ctrl_param;
#endif

#ifndef __T_CGRLC_pwr_ctrl__
#define __T_CGRLC_pwr_ctrl__
/*
 * Power Control Information
 * CCDGEN:WriteStruct_Count==1643
 */
typedef struct
{
  U8                        _align0;                  /*<  0:  1> alignment                                          */
  U8                        _align1;                  /*<  1:  1> alignment                                          */
  U8                        _align2;                  /*<  2:  1> alignment                                          */
  U8                        v_pwr_ctrl_param;         /*<  3:  1> valid-flag                                         */
  T_CGRLC_pwr_ctrl_param    pwr_ctrl_param;           /*<  4: 12> Power Control Parameters                           */
  U8                        _align3;                  /*< 16:  1> alignment                                          */
  U8                        _align4;                  /*< 17:  1> alignment                                          */
  U8                        _align5;                  /*< 18:  1> alignment                                          */
  U8                        v_glbl_pwr_ctrl_param;    /*< 19:  1> valid-flag                                         */
  T_CGRLC_glbl_pwr_ctrl_param glbl_pwr_ctrl_param;    /*< 20:  8> Global Power Control Parameters                    */
  U8                        _align6;                  /*< 28:  1> alignment                                          */
  U8                        _align7;                  /*< 29:  1> alignment                                          */
  U8                        _align8;                  /*< 30:  1> alignment                                          */
  U8                        v_freq_param;             /*< 31:  1> valid-flag                                         */
  T_CGRLC_freq_param        freq_param;               /*< 32:  4> Frequency Parameters                               */
  U8                        _align9;                  /*< 36:  1> alignment                                          */
  U8                        _align10;                 /*< 37:  1> alignment                                          */
  U8                        _align11;                 /*< 38:  1> alignment                                          */
  U8                        v_c_value;                /*< 39:  1> valid-flag                                         */
  T_CGRLC_c_value           c_value;                  /*< 40:  8> C-Value                                            */
} T_CGRLC_pwr_ctrl;
#endif


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_CGRLC_ENABLE_REQ__
#define __T_CGRLC_ENABLE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1644
 */
typedef struct
{
  U8                        enable_cause;             /*<  0:  1> Enable Cause                                       */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  U32                       ul_tlli;                  /*<  4:  4> Uplink TLLI value.                                 */
  U32                       dl_tlli;                  /*<  8:  4> Downlink TLLI value.                               */
  U8                        _align3;                  /*< 12:  1> alignment                                          */
  U8                        _align4;                  /*< 13:  1> alignment                                          */
  U8                        _align5;                  /*< 14:  1> alignment                                          */
  U8                        v_pan_struct;             /*< 15:  1> valid-flag                                         */
  T_CGRLC_pan_struct        pan_struct;               /*< 16:  4> Pan Structure                                      */
  U8                        queue_mode;               /*< 20:  1> Type of Queue Mode.                                */
  U8                        burst_type;               /*< 21:  1> Default burst type                                 */
  U8                        ab_type;                  /*< 22:  1> Default access burst type                          */
  U8                        t3168_val;                /*< 23:  1> T3168 Value                                        */
  U8                        cu_cause;                 /*< 24:  1> Cell update cause                                  */
  U8                        ac_class;                 /*< 25:  1> Access control class                               */
  U8                        change_mark;              /*< 26:  1> Change mark value                                  */
// ELEM-FF: REL99 
//   U8                        nw_rel;                   /*<  0:  0> Network Release Flag                               */
// ELEM-FF: REL99 
//   U8                        pfi_support;              /*<  0:  0> Basic Element                                      */
  U8                        _align6;                  /*< 27:  1> alignment                                          */
} T_CGRLC_ENABLE_REQ;
#endif

#ifndef __T_CGRLC_DISABLE_REQ__
#define __T_CGRLC_DISABLE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1645
 */
typedef struct
{
  U8                        disable_class;            /*<  0:  1> Disable class.                                     */
  U8                        prim_status;              /*<  1:  1> Primitive Queue Handler.                           */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_CGRLC_DISABLE_REQ;
#endif

#ifndef __T_CGRLC_UL_TBF_RES__
#define __T_CGRLC_UL_TBF_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==1646
 */
typedef struct
{
  U32                       starting_time;            /*<  0:  4> TBF starting time.                                 */
  U8                        tbf_mode;                 /*<  4:  1> Type of TBF.                                       */
  U8                        prim_status;              /*<  5:  1> Primitive Queue Handler.                           */
  U8                        polling_bit;              /*<  6:  1> Polling bit                                        */
  U8                        cs_mode;                  /*<  7:  1> Type of Coding Scheme.                             */
  U8                        mac_mode;                 /*<  8:  1> Type of MAC mode.                                  */
  U8                        nts_max;                  /*<  9:  1> Number of Timeslots.                               */
  U8                        tn_mask;                  /*< 10:  1> timeslot mask                                      */
  U8                        tfi;                      /*< 11:  1> TFI value.                                         */
  U8                        ti;                       /*< 12:  1> TLLI indicator.                                    */
  U8                        bs_cv_max;                /*< 13:  1> Maximum Countdown value.                           */
  U8                        tlli_cs_mode;             /*< 14:  1> Type of Coding Scheme in Contention Resolution.    */
  U8                        r_bit;                    /*< 15:  1> R bit                                              */
  T_CGRLC_fix_alloc_struct  fix_alloc_struct;         /*< 16:136> Fixed Allocation structure                         */
  U16                       rlc_db_granted;           /*<152:  2> RLCdata block granted                              */
  U8                        _align0;                  /*<154:  1> alignment                                          */
  U8                        _align1;                  /*<155:  1> alignment                                          */
  T_CGRLC_pwr_ctrl          pwr_ctrl;                 /*<156: 48> Power Control Information                          */
} T_CGRLC_UL_TBF_RES;
#endif

#ifndef __T_CGRLC_DL_TBF_REQ__
#define __T_CGRLC_DL_TBF_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1647
 */
typedef struct
{
  U32                       starting_time;            /*<  0:  4> TBF starting time.                                 */
  U8                        rlc_mode;                 /*<  4:  1> Type of RLC mode.                                  */
  U8                        cs_mode;                  /*<  5:  1> Type of Coding Scheme.                             */
  U8                        mac_mode;                 /*<  6:  1> Type of MAC mode.                                  */
  U8                        nts_max;                  /*<  7:  1> Number of Timeslots.                               */
  U8                        tn_mask;                  /*<  8:  1> timeslot mask                                      */
  U8                        tfi;                      /*<  9:  1> TFI value.                                         */
  U8                        t3192_val;                /*< 10:  1> Value of T3192.                                    */
  U8                        ctrl_ack_bit;             /*< 11:  1> Ctrl ack bit                                       */
  U8                        polling_bit;              /*< 12:  1> Polling bit                                        */
  U8                        _align0;                  /*< 13:  1> alignment                                          */
  U8                        _align1;                  /*< 14:  1> alignment                                          */
  U8                        _align2;                  /*< 15:  1> alignment                                          */
  T_CGRLC_pwr_ctrl          pwr_ctrl;                 /*< 16: 48> Power Control Information                          */
} T_CGRLC_DL_TBF_REQ;
#endif

#ifndef __T_CGRLC_TBF_REL_REQ__
#define __T_CGRLC_TBF_REL_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1648
 */
typedef struct
{
  U8                        tbf_mode;                 /*<  0:  1> Type of TBF.                                       */
  U8                        tbf_rel_cause;            /*<  1:  1> TBF Release Cause.                                 */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  U32                       rel_fn;                   /*<  4:  4> Release after Poll with fn.                        */
} T_CGRLC_TBF_REL_REQ;
#endif

#ifndef __T_CGRLC_TBF_REL_IND__
#define __T_CGRLC_TBF_REL_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1649
 */
typedef struct
{
  U8                        tbf_mode;                 /*<  0:  1> Type of TBF.                                       */
  U8                        tbf_rel_cause;            /*<  1:  1> TBF Release Cause.                                 */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        v_c_value;                /*<  3:  1> valid-flag                                         */
  T_CGRLC_c_value           c_value;                  /*<  4:  8> C-Value                                            */
  U8                        dl_trans_id;              /*< 12:  1> DL Assignmnet ID                                   */
  U8                        _align1;                  /*< 13:  1> alignment                                          */
  U8                        _align2;                  /*< 14:  1> alignment                                          */
  U8                        _align3;                  /*< 15:  1> alignment                                          */
} T_CGRLC_TBF_REL_IND;
#endif

#ifndef __T_CGRLC_TBF_REL_RES__
#define __T_CGRLC_TBF_REL_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==1650
 */
typedef struct
{
  U8                        tbf_mode;                 /*<  0:  1> Type of TBF.                                       */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_CGRLC_TBF_REL_RES;
#endif

#ifndef __T_CGRLC_UL_TBF_IND__
#define __T_CGRLC_UL_TBF_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1651
 */
typedef struct
{
  U8                        access_type;              /*<  0:  1> Access Type.                                       */
  U8                        ra_prio;                  /*<  1:  1> Radio priority                                     */
  U8                        nr_blocks;                /*<  2:  1> Number of blocks                                   */
  U8                        llc_prim_type;            /*<  3:  1> LLC Primitive type                                 */
  U16                       peak;                     /*<  4:  2> Peak value                                         */
  U16                       rlc_oct_cnt;              /*<  6:  2> Number of bytes for TBF                            */
// ELEM-FF: REL99 AND TI_PS_FF_TBF_EST_PACCH 
//   U8                        tbf_est_pacch;            /*<  0:  0> TBF establishment on PACCH                         */
} T_CGRLC_UL_TBF_IND;
#endif

#ifndef __T_CGRLC_DATA_REQ__
#define __T_CGRLC_DATA_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1652
 */
typedef struct
{
  U8                        blk_owner;                /*<  0:  1> Block owner.                                       */
  U8                        data_array[CGRLC_MAX_CTRL_MSG_SIZE]; /*<  1: 23> Data Array.                                        */
} T_CGRLC_DATA_REQ;
#endif

#ifndef __T_CGRLC_DATA_IND__
#define __T_CGRLC_DATA_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1653
 */
typedef struct
{
  U32                       fn;                       /*<  0:  4> Received frame number.                             */
  U8                        tn;                       /*<  4:  1> Timeslot number                                    */
  U8                        data_array[CGRLC_MAX_CTRL_MSG_SIZE]; /*<  5: 23> Data Array.                                        */
} T_CGRLC_DATA_IND;
#endif

#ifndef __T_CGRLC_POLL_REQ__
#define __T_CGRLC_POLL_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1654
 */
typedef struct
{
  U32                       poll_fn;                  /*<  0:  4> Poll frame number.                                 */
  U8                        tn;                       /*<  4:  1> Timeslot number                                    */
  U8                        poll_b_type;              /*<  5:  1> Poll burst type                                    */
  U8                        ctrl_ack;                 /*<  6:  1> Ctrl_ack                                           */
  U8                        _align0;                  /*<  7:  1> alignment                                          */
} T_CGRLC_POLL_REQ;
#endif

#ifndef __T_CGRLC_ACCESS_STATUS_REQ__
#define __T_CGRLC_ACCESS_STATUS_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1655
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CGRLC_ACCESS_STATUS_REQ;
#endif

#ifndef __T_CGRLC_CTRL_MSG_SENT_IND__
#define __T_CGRLC_CTRL_MSG_SENT_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1656
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CGRLC_CTRL_MSG_SENT_IND;
#endif

#ifndef __T_CGRLC_STARTING_TIME_IND__
#define __T_CGRLC_STARTING_TIME_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1657
 */
typedef struct
{
  U8                        tbf_mode;                 /*<  0:  1> Type of TBF.                                       */
  U8                        tfi;                      /*<  1:  1> TFI value.                                         */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_CGRLC_STARTING_TIME_IND;
#endif

#ifndef __T_CGRLC_T3192_STARTED_IND__
#define __T_CGRLC_T3192_STARTED_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1658
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CGRLC_T3192_STARTED_IND;
#endif

#ifndef __T_CGRLC_CONT_RES_DONE_IND__
#define __T_CGRLC_CONT_RES_DONE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1659
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CGRLC_CONT_RES_DONE_IND;
#endif

#ifndef __T_CGRLC_TA_VALUE_IND__
#define __T_CGRLC_TA_VALUE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1660
 */
typedef struct
{
  U8                        ta_value;                 /*<  0:  1> Timing Advance Value.                              */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_CGRLC_TA_VALUE_IND;
#endif

#ifndef __T_CGRLC_STATUS_IND__
#define __T_CGRLC_STATUS_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1661
 */
typedef struct
{
  U8                        failure;                  /*<  0:  1> Lower layer failure.                               */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_CGRLC_STATUS_IND;
#endif

#ifndef __T_CGRLC_TEST_MODE_REQ__
#define __T_CGRLC_TEST_MODE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1662
 */
typedef struct
{
  U16                       no_of_pdus;               /*<  0:  2> Number of PDUs.                                    */
  U8                        dl_timeslot_offset;       /*<  2:  1> Downlink Timeslot Offset.                          */
  U8                        test_mode_flag;           /*<  3:  1> Test mode flag.                                    */
} T_CGRLC_TEST_MODE_REQ;
#endif

#ifndef __T_CGRLC_TEST_MODE_CNF__
#define __T_CGRLC_TEST_MODE_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1663
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CGRLC_TEST_MODE_CNF;
#endif

#ifndef __T_CGRLC_TEST_END_REQ__
#define __T_CGRLC_TEST_END_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1664
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CGRLC_TEST_END_REQ;
#endif

#ifndef __T_CGRLC_TRIGGER_IND__
#define __T_CGRLC_TRIGGER_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1665
 */
typedef struct
{
  U8                        prim_type;                /*<  0:  1> Type of primitive.                                 */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_CGRLC_TRIGGER_IND;
#endif

#ifndef __T_CGRLC_STANDBY_STATE_IND__
#define __T_CGRLC_STANDBY_STATE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1666
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CGRLC_STANDBY_STATE_IND;
#endif

#ifndef __T_CGRLC_READY_STATE_IND__
#define __T_CGRLC_READY_STATE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1667
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CGRLC_READY_STATE_IND;
#endif

#ifndef __T_CGRLC_TA_VALUE_REQ__
#define __T_CGRLC_TA_VALUE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1668
 */
typedef struct
{
  U8                        ta_value;                 /*<  0:  1> Timing Advance Value.                              */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_CGRLC_TA_VALUE_REQ;
#endif

#ifndef __T_CGRLC_INT_LEVEL_REQ__
#define __T_CGRLC_INT_LEVEL_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1669
 */
typedef struct
{
  U8                        ilev[CGRLC_MAX_TIMESLOTS]; /*<  0:  8> Interference level                                 */
} T_CGRLC_INT_LEVEL_REQ;
#endif

#ifndef __T_CGRLC_TEST_MODE_IND__
#define __T_CGRLC_TEST_MODE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1670
 */
typedef struct
{
  U8                        test_mode_flag;           /*<  0:  1> Test mode flag.                                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_CGRLC_TEST_MODE_IND;
#endif

#ifndef __T_CGRLC_READY_TIMER_CONFIG_REQ__
#define __T_CGRLC_READY_TIMER_CONFIG_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1671
 */
typedef struct
{
  U32                       t3314_val;                /*<  0:  4> Value of T3314.                                    */
} T_CGRLC_READY_TIMER_CONFIG_REQ;
#endif

#ifndef __T_CGRLC_FORCE_TO_STANDBY_REQ__
#define __T_CGRLC_FORCE_TO_STANDBY_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1672
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CGRLC_FORCE_TO_STANDBY_REQ;
#endif

#ifndef __T_CGRLC_PWR_CTRL_REQ__
#define __T_CGRLC_PWR_CTRL_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1673
 */
typedef struct
{
  T_CGRLC_pwr_ctrl          pwr_ctrl;                 /*<  0: 48> Power Control Information                          */
} T_CGRLC_PWR_CTRL_REQ;
#endif

#ifndef __T_CGRLC_PWR_CTRL_CNF__
#define __T_CGRLC_PWR_CTRL_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1674
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CGRLC_PWR_CTRL_CNF;
#endif


#include "CDG_LEAVE.h"


#endif
