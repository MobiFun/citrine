/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  AOC
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
|  Purpose :  Definitions for advice of charge
+----------------------------------------------------------------------------- 
*/ 

#ifndef AOC_H
#define AOC_H

/*
 * States of Module
 */
#define AOC_NULL                  0
#define AOC_DISABLE               1
#define AOC_ENABLE                2

/*
 * Info Causes
 */
#define AOC_START_TIME            1
#define AOC_STOP_TIME             2
#define AOC_START_AOC             3
#define AOC_SUSPEND_AOC           4
#define AOC_RESUME_AOC            5
#define AOC_CALL_CONNECTED        6

/*
 * Request Types
 */
#define AOC_CTV                   0
#define AOC_CCM                   1
#define AOC_ACM                   2
#define AOC_ACMMAX                3
#define AOC_PUCT                  4
#define AOC_CCM_PUCT              5
#define AOC_ACM_PUCT              6
#define AOC_ACMMAX_PUCT           7
#define AOC_PUCT_RAW              8

/*
 * Status of update operation
 */
#define FIRST_UPDATE              0
#define SECOND_UPDATE             1
#define VERIFY_PWD                2
 
/*
 * maximum remaining call time in milliseconds
 */
#define AOC_MAX_REMAIN_CALL_TIME  30000L
#define AOC_THOUSAND_MILLISECONDS 1000L

/*
 * indicates the processing mode for calculating remaining call time
 */
#define AOC_AOC_TIMER_ELAPSED    0
#define AOC_CALL_TIMER_ELAPSED   1

/*
 * Structure for requesting PUCT Values
 */
typedef struct 
{
  UBYTE currency [4];
  UBYTE value [20];
} T_puct;

typedef struct 
{
  UBYTE currency [4];
  ULONG eppu;
  ULONG exp;
  ULONG sexp;
} T_puct_raw;

/*
 * e_bitmap parameter
 */

#define E1_CHANGED              1
#define E2_CHANGED              2
#define E3_CHANGED              4
#define E4_CHANGED              8
#define E7_CHANGED             64

typedef struct
{
  USHORT         e1;               /* e1 parameter            */
  USHORT         e2;               /* e2 parameter            */
  USHORT         e3;               /* e3 parameter            */
  USHORT         e4;               /* e4 parameter            */ /* will be cleared after charged */
  USHORT         e5;               /* e5 parameter            */
  USHORT         e6;               /* e6 parameter            */
  USHORT         e7;               /* e7 parameter            */ /* will be cleared when reached */

  UBYTE          e_next_bitmap;    /* validity of e-parameter */
  USHORT         e1_next;          /* e1 parameter in abeyance*/
  USHORT         e2_next;          /* e2 parameter in abeyance*/
  USHORT         e3_next;          /* e3 parameter in abeyance*/ /* See  AOC08226 !!! */
  USHORT         e7_next;          /* e7 parameter in abeyance*/

  ULONG          first_unit;       /* ccm increment first unit*/
  ULONG          next_unit;        /* ccm increment next unit */
  T_TIME         first_interval;   /* time first interval     */
  T_TIME         next_interval;    /* time next interval      */
  
  T_TIME         remaining_time;   /* remaining time for timer*/
  UBYTE          aoc_timer_running;/* timer is running ?      */
  UBYTE          new_data_avail;   /* E-Parameter have changed*/
  T_TIME         time_ut_charge;   /* time in sec until next  */
                                   /* charging occur          */
  UBYTE          aoci_active;      /* indicate charge only    */
} T_CC_AOC_TBL;

EXTERN UBYTE        aoc_init                (UBYTE phase, UBYTE *sim_service_table);
EXTERN void         aoc_reset               (void);
EXTERN void         aoc_sms                 (void);
EXTERN UBYTE        aoc_info                (SHORT Cid, UBYTE cause);
EXTERN void         aoc_get_values          (UBYTE value_type, void * value);
EXTERN T_ACI_RETURN aoc_set_values          (T_ACI_CMD_SRC srcId,
                                             UBYTE         value_type, 
                                             void *        value, 
                                             UBYTE *       password);
EXTERN UBYTE        aoc_check_moc           (void);
EXTERN void         aoc_parameter           (SHORT Cid, 
                                             T_FWD_CHG_ADVICE_INV * aoc_para);
EXTERN void         aoc_send_ccm            (void);
EXTERN UBYTE        aoc_ssc                 (UBYTE nr, UBYTE * serv_table);
EXTERN void         aoc_read_acm_cb         (SHORT table_id);
EXTERN void         aoc_read_acmmax_cb      (SHORT table_id);
EXTERN void         aoc_read_puct_cb        (SHORT table_id);
EXTERN void         aoc_init_calltable      (void);
EXTERN UBYTE        aoc_timeout             (USHORT handle);
EXTERN void         aoc_timeout_call_timer  (void);
EXTERN void         aoc_timeout_aoc_timer   (SHORT Cid);
EXTERN UBYTE        aoc_non_zero_cai        (SHORT Cid);
EXTERN UBYTE        aoc_check_acm           (void);
EXTERN void         aoc_calculate_charging_parameter (SHORT Cid);
EXTERN void         aoc_increment_initial_charge (SHORT Cid);
EXTERN void         aoc_increment_charge    (ULONG increment, UBYTE ever);
EXTERN void         aoc_increment_cb        (SHORT table_id);
EXTERN void         aoc_calculate_puct      (ULONG value, T_puct * result);
EXTERN void aoc_update_sim_datafield ( UBYTE operation,
                                       UBYTE accType,
                                       USHORT reqDataFld,
                                       void (*rplyCB)(SHORT));
EXTERN void         aoc_update_puct         (UBYTE operation, T_puct * value);
EXTERN void         aoc_update_acm_cb       (SHORT table_id);
EXTERN void         aoc_update_acmmax_cb    (SHORT table_id);
EXTERN void         aoc_update_puct_cb      (SHORT table_id);
EXTERN UBYTE        aoc_set_puct_values     (T_puct * puct);
EXTERN USHORT       aoc_getEVal             ( void * eBuf );
#endif
