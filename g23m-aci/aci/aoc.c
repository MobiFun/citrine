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
|  Purpose :  This module defines the functions for the
|             advice of charge handling of ACI.
+-----------------------------------------------------------------------------
*/

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "l4_tim.h"

#include "pcm.h"

#include "aci_cmh.h"

#include "psa.h"
#include "psa_sim.h"
#include "psa_mm.h"

#include "aoc.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_cc.h"

#ifdef SIM_TOOLKIT
#include "psa_sat.h"
#endif

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifdef UART
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#include "cmh_sim.h"

#define MC_AOC_ROUND_UP(X)  ((((X)%100) EQ 0) ? ((X)/100) : (((X)/100) + 1))
#define E_IN_MS(X)          ((X)*100)

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

/********* current define *********************************************/
static   UBYTE   aoc_state = AOC_NULL;      /* actual AoC state       */
static   UBYTE   sim_data[10];              /* SIM data exchange      */
static   UBYTE   currency [4];              /* used for currency      */
static   ULONG   ccm;                       /* current call meter     */
static   ULONG   ccm_already_incremented;   /* value of CCM increm.   */
static   ULONG   acm;                       /* accumulated call meter */
static   UBYTE   acm_increment_flag;        /* counter for incr. ACM  */
static   ULONG   acmmax;                    /* maximum of ACM         */
static   ULONG   eppu;                      /* elementary unit price  */
static   ULONG   sexp;                      /* sign of expression     */
static   ULONG   exp;                       /* expression value       */
static   UBYTE   pwd [9];                   /* password               */
static   UBYTE   act_upd_op;                /* actual update operation*/
static   ULONG   act_value;                 /* new value for ACM(Max) */
static   T_ACI_CMD_SRC act_src_id;          /* source of AT command   */

GLOBAL   T_CC_AOC_TBL cc_aoc_table[MAX_CALL_NR];  /* AoC Call Table   */
EXTERN T_PCEER causeMod;
EXTERN SHORT causeCeer;

static   ULONG          cct;                /* current call timer     */
/*static   ULONG          act;                *//* accumulated call timer */  /* never used */
static   USHORT         ct_running;         /* call timer is running  */
static   UBYTE          limit_reached = FALSE; /* ACM over limit ?    */

static const ULONG ppu_values [16][2] =
{
    /*    exp,          sexp            index        */
          1L,           0L,          /* 0            */
          1L,           0L,          /* 1            */
          10L,          0L,          /* 2            */
          10L,          1L,          /* 3            */
          100L,         0L,          /* 4            */
          100L,         1L,          /* 5            */
          1000L,        0L,          /* 6            */
          1000L,        1L,          /* 7            */
          10000L,       0L,          /* 8            */
          10000L,       1L,          /* 9            */
          100000L,      0L,          /* 10           */
          100000L,      1L,          /* 11           */
          1000000L,     0L,          /* 12           */
          1000000L,     1L,          /* 13           */
          10000000L,    0L,          /* 14           */
          10000000L,    1L           /* 15           */
};

#ifdef SIM_TOOLKIT
BOOL aoc_update (int ref, T_SIM_FILE_UPDATE_IND *fu);
#endif
static   UBYTE          ccwv_charging = CCWV_CHRG_Termination;

/********* function prototypes **************************************/
void aoc_calc_acm_wrn_evnt  ( ULONG           charge,
                              BOOL            aoc_running  );

void aoc_set_time_ut_charge ( SHORT           cId,
                              T_TIME          time         );
void aoc_calc_expct_charge  ( UBYTE           mode         );
void aoc_ntfy_acm_wrn_evnt  ( T_ACI_CCWV_CHRG charging     );

T_TIME       aoc_calc_time_ut_charge ( SHORT  cId,
                                       UBYTE  e_value_flag );

void aoc_start_info_newcall (SHORT Cid);
void aoc_start_newinfo_existingcall (SHORT Cid);

static void aoc_calculate_charging_parameter_part1 (SHORT Cid);
static void aoc_calculate_charging_parameter_part2 (SHORT Cid);
/********* functions ************************************************/
LOCAL void aoc_update_sim_datafield_cb(SHORT table_id, T_ACI_AT_CMD at_cmd_idn);
LOCAL void aoc_read_sim_datafield (UBYTE accType,
                                   USHORT reqDataFld,
                                   void (*rplyCB)(SHORT),
                                   UBYTE dataLen);
/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_init            |
+--------------------------------------------------------------------+


   PURPOSE : Initialisation of the Advice of charge module. The function
             is called after PIN entering. The actual phase of the SIM
             card, the SIM service table and the PCM entry is checked.
             The return value indicates whether AoC is supported.

*/

UBYTE aoc_init (UBYTE phase, UBYTE * sim_service_table)
{
  EF_MSSUP mssup;
  UBYTE    version;

  TRACE_FUNCTION ("aoc_init()");

  aoc_state = AOC_DISABLE;
  /* Check Phase. It must be at least Phase 2 */
  if (phase <= PHASE_1_SIM)
    return FALSE;
  TRACE_EVENT ("AOC: Card >= Phase 2");

  /* Check SIM Service Table */
  if (aoc_ssc(SRV_AOC,sim_service_table) NEQ ALLOCATED_AND_ACTIVATED)
    return FALSE;
  TRACE_EVENT ("AOC: Card supports AoC");

  /* check PCM entry in MSCAP field */
  pcm_Init ();
/* Implements Measure#32: Row 60 & 1039  */
  pcm_ReadFile ((UBYTE *)ef_mssup_id, SIZE_EF_MSSUP, (UBYTE *)&mssup, &version);
  if (FldGet(mssup.feat1, AoC))
  {
    /* start reading ACM, ACMMax and PUCT */
    TRACE_EVENT ("AOC: MS supports AoC");
    aoc_state = AOC_ENABLE;
    /* Implements Measure 156 and 157 */
    aoc_read_sim_datafield ( ACT_RD_REC, SIM_ACM, aoc_read_acm_cb, 3 );
    return TRUE;
  }

#ifdef _SIMULATION_
  aoc_state = AOC_ENABLE;
  return TRUE;
#else
  return FALSE;
#endif

}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_reset           |
+--------------------------------------------------------------------+

   PURPOSE : Deactivation of AoC Service. Necessary, when SIM card
             becomes unavailable
*/

void aoc_reset (void)
{
  TRACE_FUNCTION ("aoc_reset()");
  aoc_state = AOC_DISABLE;
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_sms             |
+--------------------------------------------------------------------+


   PURPOSE : Indication that a SMS has received. This can be a trigger
             for changed ACM or ACMMax on the SIM card.
*/

void aoc_sms (void)
{
  TRACE_FUNCTION ("aoc_sms()");
  if (aoc_state EQ AOC_ENABLE)
  {
    /* start reading ACM, ACMMax and PUCT again */
    /* Implements Measure 156 and 157 */
    aoc_read_sim_datafield ( ACT_RD_REC, SIM_ACM, aoc_read_acm_cb, 3 );
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_update          |
+--------------------------------------------------------------------+

   PURPOSE : Evaluation of File Change Notification and update AOC,
             if required.
*/

#ifdef SIM_TOOLKIT
BOOL aoc_update (int ref, T_SIM_FILE_UPDATE_IND *fu)
{
  BOOL found = FALSE;
  int i;

  TRACE_FUNCTION ("aoc_update ()");

  if (aoc_state NEQ AOC_ENABLE)
    return TRUE;   /* not used at all! */  /* AOC not supported */

  for (i = 0; i < (int)fu->val_nr; i++)
  {
    /* Check the full path as well */
    if (fu->file_info[i].v_path_info EQ TRUE AND
        fu->file_info[i].path_info.df_level1 EQ SIM_DF_GSM AND
        fu->file_info[i].path_info.v_df_level2 EQ FALSE AND
        ( fu->file_info[i].datafield EQ SIM_ACM OR
          fu->file_info[i].datafield EQ SIM_ACMMAX OR
          fu->file_info[i].datafield EQ SIM_PUCT ) )
    {
      found = TRUE;
      break;
    }
  }

  if (found)
  {
    simShrdPrm.fuRef = ref;
    /* Implements Measure 156 and 157 */
    aoc_read_sim_datafield ( ACT_RD_REC, SIM_ACM, aoc_read_acm_cb, 3 );
    return FALSE;
  }
  else
  {
    simShrdPrm.fuRef = -1;            /* no update needed */
    return TRUE;
  }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_info            |
+--------------------------------------------------------------------+

   PURPOSE : Information of the Advice of charge module. The following
             causes are defined:
             AOC_START_TIME     Start call time measurements.
             AOC_STOP_TIME      Stop call timer measurements.
             AOC_START_AOC      Advice of Charge is started.
             AOC_SUSPEND_AOC    Suspension of Call.
             AOC_RESUME_AOC     Resumption of Call.
*/

UBYTE aoc_info (SHORT Cid, UBYTE cause)
{
  BOOL calFlg;                 /* call flag */

  TRACE_FUNCTION ("aoc_info()");

  switch (cause)
  {
    case AOC_START_TIME: /* Indication of a new call. */
#ifdef _SIMULATION_
      TRACE_EVENT ("AOC_START_TIME");
#endif
      /* Call Time Measurements shall be
       * started if it is the first call (ct_runnning = 0) */
      if (ct_running EQ 0)
      {
        /* this is the first call. Then start call timer (periodic)
         * and clear current calltimer value and current call meter. */
        cct = ccm = ccm_already_incremented = 0L;
        TIMER_PSTART (aci_handle,  AOC_CALLTIMER, AOC_THOUSAND_MILLISECONDS, AOC_THOUSAND_MILLISECONDS);

        simShrdPrm.synCs = SYNC_START_CALL;
        psaSIM_SyncSIM();
      }

      cmhCC_flagCall( Cid, &ct_running );

      /* Clear aoc table parameter */
      memset (&cc_aoc_table[Cid], 0, sizeof(T_CC_AOC_TBL));
      break;

    case AOC_STOP_TIME: /* Indication of the end of a call. */
#ifdef _SIMULATION_
      TRACE_EVENT ("AOC_STOP_TIME");
#endif
      TRACE_EVENT_P1("CT Running = %4X", ct_running);
      calFlg = cmhCC_tstAndUnflagCall( Cid, &ct_running );

      /*Call Time Measurements shall be
       * stopped if it is the last call (ct_runnning <= 1) */
      if (ct_running EQ 0 AND calFlg)
      {
        /* this is the last call. Then stop call timer. */
        TRACE_EVENT ("Last Call Stop Call Timer");
        TIMER_STOP (aci_handle,  AOC_CALLTIMER);


        simShrdPrm.synCs = SYNC_STOP_CALL;
        psaSIM_SyncSIM();

        aoc_ntfy_acm_wrn_evnt ( CCWV_CHRG_Termination );
      }

      if (cc_aoc_table[Cid].aoc_timer_running) /* stop AoC timer if running */
      {
        TIMER_STOP (aci_handle,  (USHORT)(AOC_AOCTIMER+Cid));
        cc_aoc_table[Cid].aoc_timer_running = FALSE;
        aoc_increment_charge (0L, TRUE);  /* add rest to SIM */
      }
      break;

    case AOC_START_AOC: /* Advice of Charge information has received for the indicated call. */
#ifdef _SIMULATION_
      TRACE_EVENT ("*** AOC_START_AOC");
#endif
      TRACE_EVENT_P2("AOC started Cid = %u State = %u", Cid, aoc_state);

      if (aoc_state EQ AOC_ENABLE) /* AoC is supported */
      {
        limit_reached = FALSE;

        if (cc_aoc_table[Cid].aoc_timer_running)
          aoc_start_newinfo_existingcall(Cid);
        else
          aoc_start_info_newcall(Cid);

        return TRUE;
      }
      else   /* AoC not supported */
      {
        TRACE_EVENT ("AOC not supported");
        return FALSE;
      }

    case AOC_SUSPEND_AOC: /* Suspension of Call is indicated. */
#ifdef _SIMULATION_
      TRACE_EVENT ("*** AOC_SUSPEND_AOC");
#endif
      if (aoc_state EQ AOC_ENABLE AND
          cc_aoc_table[Cid].aoc_timer_running)  /* AoC timer is running, then stop it. */
      {
        /* save remaining timeslice prior stopping the timer */
        TIMER_STATUS (aci_handle,  (USHORT)(AOC_AOCTIMER+Cid), &cc_aoc_table[Cid].remaining_time);
        TIMER_STOP (aci_handle,  (USHORT)(AOC_AOCTIMER+Cid));

        cc_aoc_table[Cid].aoc_timer_running = FALSE;
      }
      break;

    case AOC_RESUME_AOC: /* Resumption of Call is indicated. */
#ifdef _SIMULATION_
      TRACE_EVENT ("*** AOC_RESUME_AOC");
#endif
      if (aoc_state EQ AOC_ENABLE AND
          aoc_non_zero_cai (Cid))
      {
        /* AoC info is available, then start again. calculate the remaining time */
        TIMER_PSTART (aci_handle,  (USHORT)(AOC_AOCTIMER + Cid),
                      cc_aoc_table[Cid].remaining_time,
                      cc_aoc_table[Cid].next_interval);
        cc_aoc_table[Cid].aoc_timer_running = TRUE;

        aoc_set_time_ut_charge ( Cid, cc_aoc_table[Cid].remaining_time );
      }
      break;

    case AOC_CALL_CONNECTED: /* Call active state of a Call is indicated. */
#ifdef _SIMULATION_
      TRACE_EVENT ("*** AOC_CALL_CONNECTED");
#endif
      if (aoc_state EQ AOC_ENABLE AND
          aoc_non_zero_cai (Cid))
      {
        /* AoC info is available, then start send initial CCM value to MMI. */
        aoc_send_ccm ();
      }
      break;

    default:
#ifdef _SIMULATION_
      TRACE_EVENT ("*** Wrong cause ***");
#endif
      break;

  }
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_get_values      |
+--------------------------------------------------------------------+

   PURPOSE : Request of Advice of Charge Values.
*/

void aoc_get_values (UBYTE value_type, void * value)
{
  ULONG * longPtr = value;
  T_puct_raw * raw_puct;

  TRACE_FUNCTION ("aoc_get_values()");

  switch (value_type)
  {
    case AOC_CTV: /* Current Timer Value */
      *longPtr = cct;
      break;

    case AOC_CCM: /* Current Call Meter Value */
#if defined _SIMULATION_
      TRACE_EVENT_P1("CCM = %u", MC_AOC_ROUND_UP(ccm));
#endif
      *longPtr = MC_AOC_ROUND_UP(ccm);
      break;

    case AOC_ACM: /* Accumulated Call Meter Value */
      *longPtr = MC_AOC_ROUND_UP(acm);
      break;

    case AOC_ACMMAX: /* Maximum Accumulated Call Meter Value */
      *longPtr = MC_AOC_ROUND_UP(acmmax);
      break;

    case AOC_PUCT: /* Price per Unit and Currency */
      aoc_calculate_puct (1L, (T_puct *)value);
      break;

    case AOC_PUCT_RAW: /* Price per Unit and Currency as RAW data */
      raw_puct       = (T_puct_raw *)value;
      raw_puct->exp  = exp;
      raw_puct->sexp = sexp;
      raw_puct->eppu = eppu;
      memcpy (raw_puct->currency, currency, 4);
      break;

    case AOC_CCM_PUCT: /* CCM in Price per Unit and Currency */
      aoc_calculate_puct (MC_AOC_ROUND_UP(ccm), (T_puct *)value);
      break;

    case AOC_ACM_PUCT: /* ACM in Price per Unit and Currency */
      aoc_calculate_puct (MC_AOC_ROUND_UP(acm), (T_puct *)value);
      break;

    case AOC_ACMMAX_PUCT: /* ACMMax in Price per Unit and Currency */
      aoc_calculate_puct (MC_AOC_ROUND_UP(acmmax), (T_puct *)value);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_set_values      |
+--------------------------------------------------------------------+

   PURPOSE : Setting of Advice of charge values.
*/

T_ACI_RETURN aoc_set_values (T_ACI_CMD_SRC srcId,
                             UBYTE         value_type,
                             void         *value,
                             UBYTE        *password)
{
  TRACE_FUNCTION ("aoc_set_values()");

  /* Store Password, if available and start writing to the SIM Card. */
  memcpy (pwd, password, 9);
  act_src_id = srcId;

  /* AOC not supported in SIM, Check for aoc_state return AT_FAIL */
  if( aoc_state NEQ AOC_ENABLE )
  {
    return( AT_FAIL );
  }
  
  switch (value_type)
  {
    case AOC_ACM:
      /* Implements Measure 152 */
      act_value  = 0L;
      aoc_update_sim_datafield ( FIRST_UPDATE, ACT_WR_REC,
                                 SIM_ACM, aoc_update_acm_cb );
      break;

    case AOC_ACMMAX:
      /* Implements Measure 152 */
      act_value  = (ULONG)value;
      aoc_update_sim_datafield ( FIRST_UPDATE, ACT_WR_DAT,
                                 SIM_ACMMAX, aoc_update_acmmax_cb );
      break;

    case AOC_PUCT:
      aoc_update_puct (FIRST_UPDATE, (T_puct *)value);
      break;
  }

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_check_moc       |
+--------------------------------------------------------------------+

   PURPOSE : The function checks whether ACM is greater than ACMMax.
             In this case only emergency calls are allowed for mobile
             originated call direction.
*/

UBYTE aoc_check_moc (void)
{
  TRACE_FUNCTION ("aoc_check_moc()");

  return aoc_check_acm();
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_ssc             |
+--------------------------------------------------------------------+

  PURPOSE : Check SIM service status. The value of service nr is
            extracted from the SIM service table.
*/

UBYTE aoc_ssc (UBYTE nr, UBYTE * serv_table)
{
  TRACE_FUNCTION ("aoc_ssc()");

  if (nr > MAX_SRV_TBL*4)
  {
    TRACE_ERROR ("serv_table overflow in pb_ssc()");
    return NO_ALLOCATED;
  }

  return ( *(serv_table+(nr-1)/4) >> (((nr-1)&3)*2)  & 0x03);
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_read_acm_cb     |
+--------------------------------------------------------------------+

  PURPOSE : Callback function for reading ACM.
*/

void aoc_read_acm_cb(SHORT table_id)
{
  TRACE_FUNCTION ("aoc_read_acm_cb()");

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[table_id].errCode NEQ SIM_NO_ERROR)
  {
    /* ACM is not readable, disable set to default values */
    acm=0L;
    TRACE_EVENT ("AOC: Card has no ACM field");
#ifdef SIM_TOOLKIT
    if (simShrdPrm.fuRef >= 0)
    {
      psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_ERROR);
    }
#endif
  }
  else
  {
    /* calculate ACM and start reading ACMMax  */
    acm  = (sim_data[0]<<16) + (sim_data[1]<<8) + sim_data[2];
    acm *= 100;    /* internal unit is 1/100 */
    TRACE_EVENT_P1("ACM  value = %u", (USHORT)acm);
    /* Implements Measure 156 and 157 */
    aoc_read_sim_datafield ( ACT_RD_DAT, SIM_ACMMAX, aoc_read_acmmax_cb, 3 );

  }
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_read_acmmax_cb  |
+--------------------------------------------------------------------+

  PURPOSE : Callback function for reading ACMMax.
*/

void aoc_read_acmmax_cb(SHORT table_id)
{
  TRACE_FUNCTION ("aoc_read_acmmax_cb()");

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR)
  {
    /* calculate ACMMAX and start reading PUCT */
    acmmax  = (sim_data[0]<<16) + (sim_data[1]<<8) + sim_data[2];
    acmmax *= 100;    /* internal unit is 1/100 */
    TRACE_EVENT_P1("ACMMAX  value = %u", (USHORT)acmmax);
    /* Implements Measure 156 and 157 */
    aoc_read_sim_datafield ( ACT_RD_DAT, SIM_PUCT, aoc_read_puct_cb, 5 );
  }
  else
  {
    /* ACMMAX is not readable, disable set to default values */
    acmmax=0;
    TRACE_EVENT ("AOC: Card has no ACMmax field");
#ifdef SIM_TOOLKIT
    if (simShrdPrm.fuRef >= 0)
    {
      psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_ERROR);
    }
#endif
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_read_puct_cb    |
+--------------------------------------------------------------------+

  PURPOSE : Callback function for reading PUCT.
*/

void aoc_read_puct_cb(SHORT table_id)
{
  UBYTE index;

  TRACE_FUNCTION ("aoc_read_puct_cb()");
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR)
  {
    /* calculate PUCT */
#if defined WIN32
    TRACE_EVENT_P2("PUCT %x %x",sim_data[3],sim_data[4]);
#endif

    currency [0] = sim_data[0];
    currency [1] = sim_data[1];
    currency [2] = sim_data[2];
    currency [3] = 0;
    eppu         = (sim_data[3]<<4 & 0xFF0) + (sim_data[4] & 0x0F);
    index        =  sim_data[4]>>4 & 0x0F;
    if (index > 15)  /* only 0 to 15 */
    {
      index = 15;
    }
    exp          = ppu_values[index][0];
    sexp         = ppu_values[index][1];
  }
  else
  {
    /* PUCT is not readable, disable set to default values */
    eppu = exp = sexp = 0;
    TRACE_EVENT ("AOC: Card has no PUCT field");
  }

#ifdef SIM_TOOLKIT
  if (simShrdPrm.fuRef >= 0)
  {
    psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_SUCC_ADD);
  }
#endif
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_init_calltable  |
+--------------------------------------------------------------------+

  PURPOSE : Initialisation of the AoC parameters of the call table.

*/

void aoc_init_calltable (void)
{
  TRACE_FUNCTION ("aoc_init_calltable()");

  /* Initialize AoC parameters and open
   * the call dependent AoC timer */
  memset (cc_aoc_table, 0, sizeof (cc_aoc_table));

  /* Initialize AoC Parameter */
  ccm                = 0L;
  ccm_already_incremented = 0L;
  acm_increment_flag = 0;

  /* Initialize the Call Timer Variables */
  cct        = 0L;
  /*  act        = 0L; */
  ct_running = 0;

#ifdef SIM_TOOLKIT
  simShrdPrm.fuRef = -1;
  if (!psaSAT_FURegister (aoc_update))
  {
    TRACE_EVENT ("FAILED to register the handler aoc_update() for FU");
  }

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : A timeout has occured for a timer. The funtion returns
            TRUE, if it is a call timer or a AoC timer, else FALSE
            is returned to indicate that the timer has not been
            processed.

*/
UBYTE aoc_timeout (USHORT index)
{
  if (index EQ AOC_CALLTIMER)
  {
    /* timeout call timer */
    aoc_timeout_call_timer ();
    return TRUE;
  }
  else if (index >= AOC_AOCTIMER AND index < AOC_AOCTIMER+MAX_CALL_NR)
  {
    /* Check Advice of Charge Timer */
    aoc_timeout_aoc_timer ((USHORT)(index - AOC_AOCTIMER));
    return TRUE;
  }
  else
  {
    /* the timeout is not for AoC */
    return FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)             MODULE : AOC                    |
| STATE  : code                      ROUTINE: aoc_timeout_call_timer |
+--------------------------------------------------------------------+

  PURPOSE : the call timer timeout has occured. The CCT and ACT are
            increased.

*/

void aoc_timeout_call_timer (void)
{
  int Cid;
  T_ACI_CMD_SRC idx;
  UBYTE five_second_have_just_elapsed = FALSE;

  /* TRACE_FUNCTION ("aoc_timeout_call_timer()"); */

  cct++;
  /*  act++; */

  if (acm_increment_flag EQ 1)
    five_second_have_just_elapsed = TRUE;

  /* Flag to realize 5 second delay for incrementing ACM on the SIM card. */
  if (acm_increment_flag)
    acm_increment_flag--;

  /* flush the charge on the SIM, 5 seconds after the last writing,
   * if there some units to add */
  if (five_second_have_just_elapsed)
    aoc_increment_charge (0L, FALSE);

  /* Update the remaining time for running AoC timer */
  for (Cid=0; Cid<MAX_CALL_NR; Cid++)
  {
    if (cc_aoc_table[Cid].aoc_timer_running AND
        cc_aoc_table[Cid].remaining_time > 0)
    {
      TIMER_STATUS (aci_handle,  (USHORT)(AOC_AOCTIMER + Cid), &cc_aoc_table[Cid].remaining_time);
    }
  }

  for( idx = CMD_SRC_LCL; idx < CMD_SRC_MAX; idx++ )
  {
    R_AT( RAT_CTV, idx )( );
  }

  aoc_set_time_ut_charge ( ACI_NumParmNotPresent, 0L );
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)             MODULE : AOC                    |
| STATE  : code                      ROUTINE: aoc_timeout_aoc_timer  |
+--------------------------------------------------------------------+

  PURPOSE : An AoC timer timeout has occured.

*/

void aoc_timeout_aoc_timer (SHORT Cid)
{
  SHORT dummy_waitId;          /* holds call waiting id */
  T_ACI_CMD_SRC src_dummy = CMD_SRC_NONE;

  TRACE_FUNCTION ("aoc_timeout_aoc_timer()");

  if ((aoc_check_acm () EQ FALSE)              AND
      (cc_aoc_table[Cid].aoci_active EQ FALSE) AND
      (cc_aoc_table[Cid].next_unit)                ) /* check if the next intervall would charge (time related charge > 0)*/
  {
    /* ACM exceeds ACMMax, and call is not free, so disconnect call */
    TRACE_EVENT ("ACM > ACMMax");
    cmhCC_ClearCall (Cid,
                     CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY, MNCC_CAUSE_ACM_MAX),
                     src_dummy, AT_CMD_NONE, &dummy_waitId);
  }
//TISH, patch for OMAPS00128043
//start
	else if (ccShrdPrm.ctb[Cid]->prio EQ 1) //PRIO_EMERG_CALL
		return;
//end
  else
  {
    /* e3 may have changed during the current intervall but recalc is already done */
    aoc_increment_charge (cc_aoc_table[Cid].next_unit, FALSE);
    if (cc_aoc_table[Cid].new_data_avail)
    {
      /* New Parameter available */
      /* 4.3.e) bring parameters held in abeyance into operation */
      if (cc_aoc_table[Cid].e_next_bitmap & E1_CHANGED) cc_aoc_table[Cid].e1 = cc_aoc_table[Cid].e1_next;
      if (cc_aoc_table[Cid].e_next_bitmap & E2_CHANGED) cc_aoc_table[Cid].e2 = cc_aoc_table[Cid].e2_next;
      if (cc_aoc_table[Cid].e_next_bitmap & E3_CHANGED) cc_aoc_table[Cid].e3 = cc_aoc_table[Cid].e3_next;
      if (cc_aoc_table[Cid].e_next_bitmap & E7_CHANGED) cc_aoc_table[Cid].e7 = cc_aoc_table[Cid].e7_next;

      aoc_calculate_charging_parameter_part2 (Cid);

      /* timing has changed? */
      if (cc_aoc_table[Cid].e_next_bitmap & (E2_CHANGED | E7_CHANGED))
      {
        /* reschedule timer */
        TIMER_STOP (aci_handle,  (USHORT)(AOC_AOCTIMER + Cid));
        TIMER_PSTART (aci_handle,  (USHORT)(AOC_AOCTIMER + Cid), cc_aoc_table[Cid].first_interval,
                      cc_aoc_table[Cid].next_interval);
        cc_aoc_table[Cid].remaining_time = cc_aoc_table[Cid].first_interval;
      }
      else
        /* ask the timer since we could already be ahead */
        /* cc_aoc_table[Cid].remaining_time = cc_aoc_table[Cid].next_interval; */
        TIMER_STATUS (aci_handle,  (USHORT)(AOC_AOCTIMER + Cid), &cc_aoc_table[Cid].remaining_time);

      cc_aoc_table[Cid].new_data_avail = FALSE;
      cc_aoc_table[Cid].e_next_bitmap &= ~(E1_CHANGED | E2_CHANGED | E3_CHANGED | E7_CHANGED);
    }
    else
    {
      /* re-initialise remaining time counter */
      /* ask the timer since we could already be ahead */
      /* cc_aoc_table[Cid].remaining_time = cc_aoc_table[Cid].next_interval; */
      TIMER_STATUS (aci_handle,  (USHORT)(AOC_AOCTIMER + Cid), &cc_aoc_table[Cid].remaining_time);
    }
    
    /* check if timer had e2=0 and e7>= */
    if (cc_aoc_table[Cid].next_interval == 0)
    {
      cc_aoc_table[Cid].aoc_timer_running = FALSE;
       /* cc_aoc_table[Cid].aoci_active = FALSE;*/
      cc_aoc_table[Cid].remaining_time = 0L;
      aoc_set_time_ut_charge ( Cid, (ULONG)ACI_NumParmNotPresent );
    }
    else
      aoc_set_time_ut_charge ( Cid, cc_aoc_table[Cid].remaining_time );
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)             MODULE : AOC                    |
| STATE  : code                      ROUTINE: aoc_start_aoc_timer    |
+--------------------------------------------------------------------+

  PURPOSE : AoC is started for the indicated call or
            new AoC parameter are received.
*/

void aoc_start_newinfo_existingcall (SHORT Cid)
{
  UBYTE         no_more_timer_running = FALSE;
  T_ACI_CMD_SRC src_dummy = CMD_SRC_NONE;
  SHORT         dummy_waitId;

  TRACE_FUNCTION ("aoc_start_newinfo_existingcall()");

  if(!aoc_non_zero_cai (Cid))
  {
    return;
  }

  if (cc_aoc_table[Cid].aoc_timer_running EQ FALSE)
  {
      /* New Parameter available */
      /* 4.3.e) bring parameters held in abeyance into operation */
      if (cc_aoc_table[Cid].e_next_bitmap & E1_CHANGED) cc_aoc_table[Cid].e1 = cc_aoc_table[Cid].e1_next;
      if (cc_aoc_table[Cid].e_next_bitmap & E2_CHANGED) cc_aoc_table[Cid].e2 = cc_aoc_table[Cid].e2_next;
      if (cc_aoc_table[Cid].e_next_bitmap & E3_CHANGED) cc_aoc_table[Cid].e3 = cc_aoc_table[Cid].e3_next;
      if (cc_aoc_table[Cid].e_next_bitmap & E7_CHANGED) cc_aoc_table[Cid].e7 = cc_aoc_table[Cid].e7_next;

      aoc_calculate_charging_parameter_part2 (Cid);

      /*    vsi_t_status (VSI_CALLER (USHORT)(AOC_AOCTIMER + Cid), &cc_aoc_table[Cid].remaining_time);*/

      cc_aoc_table[Cid].new_data_avail = FALSE;
      cc_aoc_table[Cid].e_next_bitmap &= ~(E1_CHANGED | E2_CHANGED | E3_CHANGED | E7_CHANGED);
      no_more_timer_running = TRUE;
  }


  /* charging information available, else wait for end of interval.*/
  if ( (cc_aoc_table[Cid].next_interval EQ 0) AND
       ((cc_aoc_table[Cid].first_interval EQ 0) OR (cc_aoc_table[Cid].remaining_time EQ 0)) )
  {
    /* if CDUR is not actively timing (i.e. due to e2 being zero,
     * e7 being zero or the processing of e7 has been completed),
     * then a new value of e2 and/or e7 is applied immediately as per a normal call. */
    no_more_timer_running = TRUE;
  }

  /* charging information available, else wait for end of interval. */
  if ((aoc_check_acm () EQ TRUE)            OR
      (cc_aoc_table[Cid].aoci_active EQ TRUE))
  {
    /* ACM has not exceeded ACMMax then calculate charging parameters and increment inital charge */
    aoc_calculate_charging_parameter_part1 (Cid);
    aoc_increment_initial_charge (Cid);
    cc_aoc_table[Cid].new_data_avail = TRUE;
  }
  else
  {
    /*
     * ACM has reached ACM max, the call is kept till the next interval elapses
     * but if there is no timer running, for a new reception of CAI non zero,
     * we behave as on reception of a first CAI for a call, if acm has reached ACM max,
     * we disconnect the call.
     * "Rec 2.24 : 4.2.2 ACM : If the ACM max is valid and the ACM is equal to or greater
     * than the value of ACM max, and an incoming call is received and subsequently
     * a non-zero CAI is received for that call, then the call shall be terminated by
     * the ME with an appropriate indication given to the user."
     */
    if (no_more_timer_running)
    {
      /* if we are here, it means that it's a non-zero CAI, so no need to test it again */
      TRACE_EVENT("no more timer running clear call");
      cmhCC_ClearCall (Cid,
                       CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY, MNCC_CAUSE_ACM_MAX),
                       src_dummy, AT_CMD_NONE, &dummy_waitId);
     }
  }

  if (no_more_timer_running)
  {
    /* New Parameter available */
    aoc_calculate_charging_parameter_part2 (Cid);

    cc_aoc_table[Cid].new_data_avail = FALSE;
    TIMER_STOP (aci_handle,  (USHORT)(AOC_AOCTIMER + Cid));
    TIMER_PSTART (aci_handle,  (USHORT)(AOC_AOCTIMER + Cid), cc_aoc_table[Cid].first_interval,
                  cc_aoc_table[Cid].next_interval);
    /* re-initialise remaining time counter for first interval */
    cc_aoc_table[Cid].remaining_time = cc_aoc_table[Cid].first_interval;
  }
}

void aoc_start_info_newcall (SHORT Cid)
{
  T_ACI_CMD_SRC src_dummy = CMD_SRC_NONE;
  SHORT         dummy_waitId;

  TRACE_FUNCTION ("aoc_start_info_newcall()");

  if( !aoc_non_zero_cai(Cid) )
  {
    return;
  }
  /* charging information available */

  if( (aoc_check_acm() EQ FALSE)           AND
      (cc_aoc_table[Cid].aoci_active EQ FALSE) )
  {
    /* ACM exceeds ACMMax, disconnect call */
    TRACE_EVENT("clear call on initial call");
    cmhCC_ClearCall (Cid,
                     CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY, MNCC_CAUSE_ACM_MAX),
                     src_dummy,
                     AT_CMD_NONE,
                     &dummy_waitId);
    return;
  }

  /* ACM has not exceeded ACMMax then calculate charging parameters,
   * increment inital charge and start AoC Timer. */
  {
    /* bring new parameters into operation */

      if (cc_aoc_table[Cid].e_next_bitmap & E1_CHANGED) cc_aoc_table[Cid].e1 = cc_aoc_table[Cid].e1_next;
      if (cc_aoc_table[Cid].e_next_bitmap & E2_CHANGED) cc_aoc_table[Cid].e2 = cc_aoc_table[Cid].e2_next;
      if (cc_aoc_table[Cid].e_next_bitmap & E3_CHANGED) cc_aoc_table[Cid].e3 = cc_aoc_table[Cid].e3_next;
      if (cc_aoc_table[Cid].e_next_bitmap & E7_CHANGED) cc_aoc_table[Cid].e7 = cc_aoc_table[Cid].e7_next;

   /*    cc_aoc_table[Cid].new_data_avail = FALSE;*/
    cc_aoc_table[Cid].e_next_bitmap &= ~(E1_CHANGED | E2_CHANGED | E3_CHANGED | E7_CHANGED);
  }

  aoc_calculate_charging_parameter (Cid);
  aoc_increment_initial_charge (Cid);

  if(cc_aoc_table[Cid].first_interval)
  {
    TIMER_PSTART (aci_handle,  (USHORT)(AOC_AOCTIMER + Cid),  cc_aoc_table[Cid].first_interval,
                  cc_aoc_table[Cid].next_interval);

    cc_aoc_table[Cid].aoc_timer_running = TRUE;
  }

  cc_aoc_table[Cid].new_data_avail = FALSE;
  cc_aoc_table[Cid].remaining_time = cc_aoc_table[Cid].first_interval;

  aoc_set_time_ut_charge ( Cid, cc_aoc_table[Cid].remaining_time );
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)             MODULE : AOC                    |
| STATE  : code                      ROUTINE: aoc_non_zero_cai       |
+--------------------------------------------------------------------+

  PURPOSE : Checks whether the Advice of Charge parameter indicate
            a free call (parameters are not available or equal zero).
*/

UBYTE aoc_non_zero_cai (SHORT Cid)
{
  TRACE_FUNCTION ("aoc_non_zero_cai()");

  /* AoC = e3      * { e4       + e1*INT(CDUR/(e7,e2)) + e5*INT(SEG/e6) }
   *     = scaling * { constant + time related         + data related}
   *
   * Ref. GSM 2.24 Section 4 Functional operation in MS */

  if ((cc_aoc_table[Cid].e3 EQ 0) OR
     ((cc_aoc_table[Cid].e1 EQ 0) AND (cc_aoc_table[Cid].e4 EQ 0) /* AND (cc_aoc_table[Cid].e5 EQ 0) */ ))
     /* e5, e6 is currently not supported */
    return FALSE;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)             MODULE : AOC                    |
| STATE  : code                      ROUTINE: aoc_check_acm          |
+--------------------------------------------------------------------+

  PURPOSE : The function indicates whether ACM exceeds ACMMax.

*/

UBYTE aoc_check_acm (void)
{
  TRACE_FUNCTION ("aoc_check_acm()");

  TRACE_EVENT_P2("ACM=%u ACMMax=%u", acm, acmmax);

  if (acmmax NEQ 0)
  {
    /*
     * ACMMax is valid
     */
    if (acm >= acmmax)
    {
      causeMod  =  P_CEER_sim;                       /* Set the module to sim to report ceer */
      causeCeer =  P_CEER_ACMMaxReachedOrExceeded;   /* Set proprietary cause */
      return FALSE;      /* ACM exceeds ACMMax */
    }
    else
    {
      causeMod  =  P_CEER_mod;          /* Clear module which is set */
      causeCeer =  P_CEER_NotPresent;   /* Clear proprietary cause */
    }
  }
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)      MODULE : AOC                           |
| STATE  : code               ROUTINE: aoc_calculate_charging_params_part1 |
+--------------------------------------------------------------------+

  PURPOSE : The function calculates an initial rime related charge

*/

static void aoc_calculate_charging_parameter_part1 (SHORT Cid)
{
  TRACE_FUNCTION ("aoc_calculate_charging_parameter_part1()");

  /*
   * Calculation for inital charge
   */
  cc_aoc_table[Cid].first_unit =
      ((ULONG)cc_aoc_table[Cid].e3 * (ULONG)cc_aoc_table[Cid].e4) / 10;
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)      MODULE : AOC                           |
| STATE  : code               ROUTINE: aoc_calculate_charging_params_part2 |
+--------------------------------------------------------------------+

  PURPOSE : The function calculates the interval length and the
            number of units which will be incremented at interval
            end.

*/

static void aoc_calculate_charging_parameter_part2 (SHORT Cid)
{

  TRACE_FUNCTION ("aoc_calculate_charging_parameter_part2()");

  /* Calculation of first time interval */
  cc_aoc_table[Cid].first_interval = (T_TIME) E_IN_MS(cc_aoc_table[Cid].e7);

  /* Calculation of next time interval */
  cc_aoc_table[Cid].next_interval = (T_TIME) E_IN_MS(cc_aoc_table[Cid].e2);


  /* 4.3.a) E7 is not available or E7 is equal zero then use E2 */
  if (cc_aoc_table[Cid].first_interval EQ 0L)
    cc_aoc_table[Cid].first_interval = cc_aoc_table[Cid].next_interval;

  /*
   * Calculation for charge of next units
   */
  cc_aoc_table[Cid].next_unit =
      ((ULONG)cc_aoc_table[Cid].e3 * (ULONG)cc_aoc_table[Cid].e1) / 10;
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)      MODULE : AOC                           |
| STATE  : code               ROUTINE: aoc_calculate_charging_params |
+--------------------------------------------------------------------+

  PURPOSE : The function calculates the interval length and the
            number of units which will be incremented at interval
            end.

*/

void aoc_calculate_charging_parameter (SHORT Cid)
{
  TRACE_FUNCTION ("aoc_calculate_charging_parameter()");

  aoc_calculate_charging_parameter_part1 (Cid);
  aoc_calculate_charging_parameter_part2 (Cid);
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)      MODULE : AOC                           |
| STATE  : code               ROUTINE: aoc_increment_initial_charge  |
+--------------------------------------------------------------------+

  PURPOSE : The function increments the inital charge to CCM and ACM.

*/

void aoc_increment_initial_charge (SHORT Cid)
{
  TRACE_FUNCTION ("aoc_increment_inital_charge()");

  if (cc_aoc_table[Cid].first_unit)
  {
    /*
     * if initial charge is available, charge it and
     * inform MMI about it although it has no charge.
     */
    aoc_increment_charge (cc_aoc_table[Cid].first_unit, FALSE);
    cc_aoc_table[Cid].first_unit = 0L;
  }
  aoc_send_ccm ();
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)      MODULE : AOC                           |
| STATE  : code               ROUTINE: aoc_increment_charge          |
+--------------------------------------------------------------------+

  PURPOSE : The function increments charge to CCM and ACM. If necessary
            the ACM is incremented on the SIM card.

*/

void aoc_increment_charge (ULONG charge, UBYTE ever)
{
  SHORT table_id;
  ULONG acm_for_sim;

  TRACE_FUNCTION ("aoc_increment_charge()");

  /*
   * Both CCM is incremented
   */
  ccm += charge;

  if (ccm > (0xFFFFFF * 100))  /* limit to 0xffffff since datafeld is only 3 bytes long.*/
    ccm = (0xFFFFFF * 100);



  TRACE_EVENT_P2("NewCCM %u charge %u", ccm, charge);

  /*
   * the update on the SIM is the difference between the rounded up value of the current CCM
   * and the old CCM (already rounded up)
   */
  acm_for_sim = MC_AOC_ROUND_UP(ccm) - MC_AOC_ROUND_UP(ccm_already_incremented);

  if (acm_for_sim)
  {
    if (acm_increment_flag EQ 0 OR ever)
    {
      /*
       * Nothing stored in the last five seconds on the SIM card
       * and now something to store
       */
      acm_increment_flag = 5;

      ccm_already_incremented += acm_for_sim * 100;

      acm += acm_for_sim * 100;

      if (acm > (0xFFFFFF * 100))
        acm = (0xFFFFFF * 100);

      if (limit_reached EQ FALSE)
      {
        /*
         * request table id for SIM SAP access
         */
        table_id = psaSIM_atbNewEntry();

        if(table_id NEQ NO_ENTRY)
        {
          /*
           * Fill formular for access
           *
           * set datafield type = Binary field
           * set datafield      = SIM_ACM
           * set offset         = 0 Bytes
           * set length         = 3 Bytes
           */
          simShrdPrm.atb[table_id].accType      = ACT_INC_DAT;
          simShrdPrm.atb[table_id].v_path_info  = FALSE;
          simShrdPrm.atb[table_id].reqDataFld   = SIM_ACM;
          simShrdPrm.atb[table_id].dataOff      = 0;
          simShrdPrm.atb[table_id].dataLen      = 3;
          simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;
          simShrdPrm.atb[table_id].exchData     = sim_data;
          simShrdPrm.atb[table_id].rplyCB       = aoc_increment_cb;
          simShrdPrm.aId = table_id;
          sim_data[0] = (UBYTE)(acm_for_sim >> 16);
          sim_data[1] = (UBYTE)(acm_for_sim >> 8);
          sim_data[2] = (UBYTE)(acm_for_sim & 0xFF);

          TRACE_EVENT_P1("ACM increment = %u", acm_for_sim);

          if(psaSIM_AccessSIMData() < 0)
          {
            TRACE_EVENT("FATAL ERROR");
          }
        }
      }
    }
    aoc_send_ccm ();
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_increment_cb    |
+--------------------------------------------------------------------+

  PURPOSE : Callback function for incrementing ACM.

*/

void aoc_increment_cb(SHORT table_id)
{
  UBYTE i;
  SHORT dummy_waitId;          /* holds call waiting id */
  T_ACI_CMD_SRC src_dummy = CMD_SRC_NONE;


  TRACE_FUNCTION ("aoc_increment_cb()");

  switch (simShrdPrm.atb[table_id].errCode)
  {
    case SIM_NO_ERROR:
      break;
    case SIM_CAUSE_MAX_INCREASE:
      /*
       * ACM has reached limit 0xFFFFFF,
       * If ACMMAX is zero, ACM shall be cleared
       * by MMI. Only an indication is forwarded
       * to MMI, else the call is released.
       */
      if (acmmax EQ 0)
      {
        limit_reached = TRUE;
        R_AT( RAT_CME, simEntStat.entOwn )
              ( AT_CMD_CACM, CME_ERR_AcmResetNeeded );
        break;
      }
       /*lint -fallthrough */
    default:
      /*
       * ACM increment is not successfull
       */
      aoc_state = AOC_DISABLE;

      /*
       * release all chargeable calls
       */
      for (i=0;i<MAX_CALL_NR;i++)
      {
        if (aoc_non_zero_cai(i))
        {
          cmhCC_ClearCall (i,
                           CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS, MNCC_ACI_ORIGINATING_ENTITY, MNCC_CAUSE_ACM_MAX),
                           src_dummy, AT_CMD_NONE,
                           &dummy_waitId);
        }
      }
      break;
  }
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_calculate_puct  |
+--------------------------------------------------------------------+

  PURPOSE : Calculate counter in PUCT.

*/

void aoc_calculate_puct (ULONG value, T_puct * result)
{
  TRACE_FUNCTION ("aoc_calculate_puct()");
#if defined WIN32
  {
    TRACE_EVENT_P1("EPPU %u",eppu);
    TRACE_EVENT_P1("SEXP %u",sexp);
    TRACE_EVENT_P1("EXP  %u",exp);
    TRACE_EVENT_P1("VAL  %u",value);
  }
#endif
  /*
   * copy currency
   */
  memcpy (result->currency, currency, 4);

  /*
   * Multiply counter value with elementary price per unit
   */
  value *= eppu;
  /*
   * If sexp is set divide by logarithm of ten, else multiply
   */
  if (sexp)
  {
    value *= 1000;    /* internal calculation is 1/1000 unit */
    value /= exp;     /* to support e-3 to e-1 */
    sprintf ((char *) result->value, "%u.%03u", value / 1000, value % 1000);
  }
  else
  {
    value *= exp;
    sprintf ((char *) result->value, "%u", value );
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)      MODULE : AOC                           |
| STATE  : code               ROUTINE: aoc_update_acm                |
+--------------------------------------------------------------------+

  PURPOSE : The function tries to reset the ACM field on the SIM card.

*/

void aoc_update_acm (UBYTE operation, ULONG value)
{
  SHORT table_id;

  TRACE_FUNCTION ("aoc_update_acm()");

  /*
   * request table id for SIM SAP access
   */
  table_id   = psaSIM_atbNewEntry();
  act_upd_op = operation;

  if(table_id NEQ NO_ENTRY)
  {
    switch (operation)
    {
      case FIRST_UPDATE:
        /*
         * This is the first access to ACM. It may fail,
         * because PIN2 is needed.
         */
        act_value  = 0L;
        /*lint -fallthrough*/
      case SECOND_UPDATE:
        /*
         * This is the second access to ACM after PIN entering.
         */
        simShrdPrm.atb[table_id].accType      = ACT_WR_REC;
        simShrdPrm.atb[table_id].v_path_info  = FALSE;
        simShrdPrm.atb[table_id].reqDataFld   = SIM_ACM;
        simShrdPrm.atb[table_id].dataOff      = 0;
        simShrdPrm.atb[table_id].dataLen      = 3;
        simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;
        simShrdPrm.atb[table_id].exchData     = sim_data;
        simShrdPrm.atb[table_id].rplyCB       = aoc_update_acm_cb;
        simShrdPrm.aId = table_id;
        sim_data[0] = (UBYTE)(act_value >> 16);
        sim_data[1] = (UBYTE)(act_value >> 8);
        sim_data[2] = (UBYTE)(act_value & 0xFF);

        if(psaSIM_AccessSIMData() < 0)
        {
          TRACE_EVENT("FATAL ERROR");
        }
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_update_acm_cb   |
+--------------------------------------------------------------------+

  PURPOSE : Callback function for updating ACM.

*/

void aoc_update_acm_cb(SHORT table_id)
{

  TRACE_FUNCTION ("aoc_update_acm_cb()");
  /* Implements Measure 213 */
  aoc_update_sim_datafield_cb( table_id, AT_CMD_CACM );
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)      MODULE : AOC                           |
| STATE  : code               ROUTINE: aoc_update_acmmax             |
+--------------------------------------------------------------------+

  PURPOSE : The function tries to set the ACMMAX field on the SIM card.

*/

void aoc_update_acmmax (UBYTE operation, ULONG value)
{
  SHORT table_id;

  TRACE_FUNCTION ("aoc_update_acmmax()");

  /*
   * request table id for SIM SAP access
   */
  table_id   = psaSIM_atbNewEntry();
  act_upd_op = operation;

  if(table_id NEQ NO_ENTRY)
  {
    switch (operation)
    {
      case FIRST_UPDATE:
        /*
         * This is the first access to ACMMAX. It may fail,
         * because PIN2 is needed.
         */
        act_value  = value;
        /*lint -fallthrough*/
      case SECOND_UPDATE:
        /*
         * This is the second access to ACMMAX after PIN entering.
         */
        simShrdPrm.atb[table_id].accType      = ACT_WR_DAT;
        simShrdPrm.atb[table_id].v_path_info  = FALSE;
        simShrdPrm.atb[table_id].reqDataFld   = SIM_ACMMAX;
        simShrdPrm.atb[table_id].dataOff      = 0;
        simShrdPrm.atb[table_id].dataLen      = 3;
        simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;
        simShrdPrm.atb[table_id].exchData     = sim_data;
        simShrdPrm.atb[table_id].rplyCB       = aoc_update_acmmax_cb;
        simShrdPrm.aId = table_id;
        sim_data[0] = (UBYTE)(act_value >> 16);
        sim_data[1] = (UBYTE)(act_value >> 8);
        sim_data[2] = (UBYTE)(act_value & 0xFF);

        if(psaSIM_AccessSIMData() < 0)
        {
          TRACE_EVENT("FATAL ERROR");
        }
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)               MODULE:  AOC                  |
| STATE  : code                        ROUTINE: aoc_update_acmmax_cb |
+--------------------------------------------------------------------+

  PURPOSE : Callback function for updating ACMMax.

*/

void aoc_update_acmmax_cb(SHORT table_id)
{

  TRACE_FUNCTION ("aoc_update_acmmax_cb()");
  /* Implements Measure 213 */
  aoc_update_sim_datafield_cb( table_id, AT_CMD_CAMM );
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)      MODULE : AOC                           |
| STATE  : code               ROUTINE: aoc_update_puct               |
+--------------------------------------------------------------------+

  PURPOSE : The function tries to modify the PUCT field on the SIM card.

*/

void aoc_update_puct (UBYTE operation, T_puct * value)
{
  SHORT table_id;

  TRACE_FUNCTION ("aoc_update_puct()");

  /*
   * request table id for SIM SAP access
   */
  table_id   = psaSIM_atbNewEntry();
  act_upd_op = operation;

  if(table_id NEQ NO_ENTRY)
  {
    switch (operation)
    {
      case FIRST_UPDATE:
        /*
         * This is the first access to PUCT. It may fail,
         * because PIN2 is needed.
         */
        if (aoc_set_puct_values (value) EQ FALSE)
        {
          R_AT( RAT_CME, simEntStat.entOwn )
               ( AT_CMD_CPUC, CME_ERR_OpNotAllow );

          simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
          return;
        }
        /*lint -fallthrough*/
      case SECOND_UPDATE:
        /*
         * This is the second access to PUCT after PIN entering.
         */
        simShrdPrm.atb[table_id].accType      = ACT_WR_DAT;
        simShrdPrm.atb[table_id].v_path_info  = FALSE;
        simShrdPrm.atb[table_id].reqDataFld   = SIM_PUCT;
        simShrdPrm.atb[table_id].dataOff      = 0;
        simShrdPrm.atb[table_id].dataLen      = 5;
        simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;
        simShrdPrm.atb[table_id].exchData     = sim_data;
        simShrdPrm.atb[table_id].rplyCB       = aoc_update_puct_cb;
        simShrdPrm.aId = table_id;
        if(psaSIM_AccessSIMData() < 0)
        {
          TRACE_EVENT("FATAL ERROR");
        }
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_update_puct_cb  |
+--------------------------------------------------------------------+

  PURPOSE : Callback function for updating PUCT.

*/

void aoc_update_puct_cb(SHORT table_id)
{
  T_SIM_SET_PRM * pSIMSetPrm;  /* points to MM parameter set */
  UBYTE         index;

  TRACE_FUNCTION ("aoc_update_puct_cb()");

  switch (simShrdPrm.atb[table_id].errCode)
  {
    case SIM_NO_ERROR:
      /*
       * No error has occured, read ACM etc. again from SIM Card
       */
      currency [0] = sim_data[0];
      currency [1] = sim_data[1];
      currency [2] = sim_data[2];
      currency [3] = 0;
      eppu         = (sim_data[3]<<4 & 0xFF0) + (sim_data[4] & 0x0F);
      index        =  sim_data[4]>>4 & 0x0F;

      if (index > 15)  /* only 0 to 15 */
      {
        index = 15;
      }

      exp          = ppu_values[index][0];
      sexp         = ppu_values[index][1];

      simEntStat.curCmd     = AT_CMD_NONE;
      simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
      R_AT( RAT_OK, simEntStat.entOwn) (AT_CMD_CPUC );
      aoc_sms ();
      break;
    case SIM_CAUSE_PIN2_EXPECT:
      /*
       * error has occured, maybe PIN2 is needed
       */
      simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
      if (strlen ((char *) pwd) AND
          act_upd_op EQ FIRST_UPDATE)
      {
        /*
         * Password is available and
         * it is the first update, then
         * try to verify PIN2
         */
        act_upd_op = VERIFY_PWD;
        pSIMSetPrm = &simShrdPrm.setPrm[act_src_id];

        cmhSIM_FillInPIN ( (char *) pwd, pSIMSetPrm -> curPIN, PIN_LEN);
        pSIMSetPrm -> PINType = PHASE_2_PIN_2;
        simEntStat.curCmd     = AT_CMD_CPUC;
        simShrdPrm.owner = (T_OWN)act_src_id;
        simEntStat.entOwn = act_src_id;

        if ( psaSIM_VerifyPIN() < 0 )  /* verify PIN */
        {
          TRACE_EVENT( "FATAL RETURN psaSIM in +CAMM" );
        }
      }
      else
      {
        /*
         * PIN2 not available or second attempt
         */
        simEntStat.curCmd     = AT_CMD_NONE;
        simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
        if (act_upd_op EQ FIRST_UPDATE)
        {
          R_AT( RAT_CME, simEntStat.entOwn )
               ( AT_CMD_CPUC, CME_ERR_SimPin2Req );
        }
        else
        {
          R_AT( RAT_CME, simEntStat.entOwn )
                ( AT_CMD_CPUC, CME_ERR_WrongPasswd );
        }
      }
      break;
    default:
      /*
       * Any other error, respective error code is returned 
       */
      simEntStat.curCmd     = AT_CMD_NONE;
      simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
      R_AT( RAT_CME, simEntStat.entOwn )
        (
          AT_CMD_CPUC, 
          cmhSIM_GetCmeFromSim ( simShrdPrm.atb[table_id].errCode )
        );
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_set_puct_values |
+--------------------------------------------------------------------+

  PURPOSE : Converts the PUCT values from string to SIM format.
*/

UBYTE aoc_set_puct_values(T_puct * puct)
{
  UBYTE first_digit  = FALSE;
  UBYTE colon_found  = FALSE;
  UBYTE position_dot = 0;
  SHORT temp_pos     = 0;
  SHORT position     = 0;
  SHORT length       = 0;
  ULONG eppu         = 0L;
  ULONG temp_eppu    = 0L;
  UBYTE i;

  TRACE_FUNCTION ("aoc_set_puct_values()");

  sim_data[0] = puct->currency [0];
  sim_data[1] = puct->currency [1];
  sim_data[2] = puct->currency [2];

  for (i=0;i<20;i++)
  {
    if (puct->value[i] EQ '\0')
      break;

    if (first_digit)
    {
      /* at least one digit detected */
      if (colon_found)
      {
        /* checking the digits after the colon */
        switch (puct->value[i])
        {
          case '0':
            /* zeros after the colon are counted */
            temp_eppu = temp_eppu * 10 + (puct->value[i] - '0');
            temp_pos++;
            break;

          default:
            /* digits available before the colon */
            eppu = (temp_eppu * 10) + (puct->value[i] - '0');
            temp_eppu = eppu;
            length = length + temp_pos + 1;
            temp_pos = 0;
            if (position_dot)
            {
              position = position_dot;
              position_dot = 0;
            }
            break;
        }
      }
      else
      {
        /* checking the digits before the colon  */
        switch (puct->value[i])
        {
          case '0':
            /* zeros before the colon are counted */
            temp_eppu = temp_eppu * 10 + (puct->value[i] - '0');
            temp_pos++;
            break;

          case '.':
            colon_found = TRUE;
            position_dot = position + temp_pos;
            length += temp_pos;
            temp_pos = 0;
            break;

          default:
            /* digits available before the colon */
            if (temp_pos)
              eppu = (temp_eppu * 10) + (puct->value[i] - '0');
            else
              eppu = eppu * 10 + (puct->value[i] - '0');
            temp_eppu = eppu;
            length = length + temp_pos + 1;
            position = position + temp_pos + 1;
            temp_pos = 0;
            break;
        }
      }
    }
    else
    {
      /* no digit found */
      if (colon_found)
      {
        /* searching for the first digit after the colon
         * e.g.  0.0034 */
        switch (puct->value[i])
        {
          case '0':
            /* count the number of zeros after the colon */
            temp_pos++;
            break;

          default:
            /* digits available before the colon */
            first_digit = TRUE;
            position -= temp_pos;
            temp_pos  = 0;
            temp_eppu = eppu = puct->value[i]-'0';
            length++;
            break;
        }
      }
      else
      {
        /* checking the digits before the colon
         * e.g 234.56  looking for the 2 */
        switch (puct->value[i])
        {
          case '0':
            /* leading zeros are ignored */
            break;
          case '.':
            /* no digits before the colon, e.g.  0.23 */
            colon_found = TRUE;
            break;
          default:
            /* digits available before the colon */
            first_digit = TRUE;
            temp_eppu = eppu = puct->value[i]-'0';
            position++;
            length++;
            break;
        }
      }
    }

    if (puct->value[i] EQ 0)
      break;
  }

#if defined WIN32
  {
    TRACE_EVENT_P2("PUCT POS=%d LEN=%d", position, length);
    TRACE_EVENT_P1("EPPU=%d", eppu);
  }
#endif

  /*
   * check the maximum of EPPU
   */
  if (eppu > 0xFFF)
    return FALSE;

  /*
   * set the EPPU, SEXP and EXP for the SIM Card
   */

  sim_data[3] = (UBYTE)(eppu >> 4);
  sim_data[4] = (UBYTE)(eppu & 0xF);


  /*
   * for the case : reset PUCT
   */
  if (!first_digit)
  {
    /*
     * set the first 4 bits of the fifth bytes to 0 (exp2, exp1, exp0, sexp)
     */
    sim_data[4] = sim_data[4] & 0x0F;
  }
  else
  {
    if (!colon_found)
    {
      if (temp_pos)
      {
        sim_data[4] += (temp_pos << 5);
      }
      else
      {
        sim_data[4] += ((length - position) << 5);
      }
    }
    else
    {
      if (position_dot NEQ 0)
        sim_data[4] += ((length - position) << 5);
      else
        sim_data[4] += 0x10 + ((length - position) << 5);
    }
  }
  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_parameter       |
+--------------------------------------------------------------------+

  PURPOSE : Copies the e-parameters of the facility IE.

*/
void aoc_parameter(SHORT Cid, T_FWD_CHG_ADVICE_INV * aoc_para)
{
  T_chargingInformation * charge;

  TRACE_FUNCTION ("aoc_parameter()");

  switch (aoc_para->forwardChargeAdviceArg.ssCode)
  {
    case(SS_CD_AOCI):
      cc_aoc_table[Cid].aoci_active=TRUE;
      break;
    case(SS_CD_AOCC):   
      cc_aoc_table[Cid].aoci_active=FALSE;
      break;
    default:
      TRACE_EVENT_P1("UNEXPECTED SS_CODE in FWD_AOC %d, assume AOCC", aoc_para->forwardChargeAdviceArg.ssCode);
      cc_aoc_table[Cid].aoci_active=FALSE;
  }   

  charge = &aoc_para->forwardChargeAdviceArg.chargingInformation;

  cc_aoc_table[Cid].new_data_avail = TRUE; 

  if (charge->v_e1)
  {
    if (cc_aoc_table[Cid].aoc_timer_running)
    {
      /* E1 is available, 4.3.e) special */
      cc_aoc_table[Cid].e1_next        = aoc_getEVal(&charge->e1);
      cc_aoc_table[Cid].e_next_bitmap |= E1_CHANGED;
      TRACE_EVENT_P1("E1 = %u", cc_aoc_table[Cid].e1_next);
    }
    else
    {
      cc_aoc_table[Cid].e1             = aoc_getEVal(&charge->e1);
      TRACE_EVENT_P1("E1 = %u", cc_aoc_table[Cid].e1);
    }

  }

  if (charge->v_e2)
  {
    if (cc_aoc_table[Cid].aoc_timer_running)
    {
      /* E2 is available, 4.3.e) special */
      cc_aoc_table[Cid].e2_next        = aoc_getEVal(&charge->e2);
      cc_aoc_table[Cid].e_next_bitmap |= E2_CHANGED;
      TRACE_EVENT_P1("E2 = %u", cc_aoc_table[Cid].e2_next);
    }
    else
    {
      cc_aoc_table[Cid].e2             = aoc_getEVal(&charge->e2);
      TRACE_EVENT_P1("E2 = %u", cc_aoc_table[Cid].e2);
    }
  }

  if (charge->v_e3)
  {
    /* Special handling of E3, see  AOC08226 */
    cc_aoc_table[Cid].e3_next        = aoc_getEVal(&charge->e3);
    cc_aoc_table[Cid].e_next_bitmap |= E3_CHANGED;
    TRACE_EVENT_P1("E3 = %u", cc_aoc_table[Cid].e3_next);
  }

  if (charge->v_e4)
  {
    cc_aoc_table[Cid].e4        = aoc_getEVal(&charge->e4);
    cc_aoc_table[Cid].e_next_bitmap |= E4_CHANGED;
    TRACE_EVENT_P1("E4 = %u", cc_aoc_table[Cid].e4);
  }

  /* e5 and e6 are not (yet?) supported */
  if (charge->v_e5)
  {
    cc_aoc_table[Cid].e5        = aoc_getEVal(&charge->e5);
    TRACE_EVENT_P1("E5 = %u", cc_aoc_table[Cid].e5);
  }

  if (charge->v_e6)
  {
    cc_aoc_table[Cid].e6        = aoc_getEVal(&charge->e6);
    TRACE_EVENT_P1("E6 = %u", cc_aoc_table[Cid].e6);
  }

  if (charge->v_e7)
  {
    if (cc_aoc_table[Cid].aoc_timer_running)
    {
      /* E7 is available, 4.3.e) special */
      cc_aoc_table[Cid].e7_next        = aoc_getEVal(&charge->e7);
      cc_aoc_table[Cid].e_next_bitmap |= E7_CHANGED;
      TRACE_EVENT_P1("E7 = %u", cc_aoc_table[Cid].e7_next);
    }
    else
    {
      cc_aoc_table[Cid].e7             = aoc_getEVal(&charge->e7);
      TRACE_EVENT_P1("E7 = %u", cc_aoc_table[Cid].e7);
    }

  }

  if(cc_aoc_table[Cid].e_next_bitmap & (E1_CHANGED | E2_CHANGED | E7_CHANGED))
  {
    /* e3 should be applied to the parameters held in abeyance, see  AOC08226 */
  }
  else
  {
    /* bring e3 immediately into operation */
    cc_aoc_table[Cid].e3 = cc_aoc_table[Cid].e3_next;
  }



  if(cc_aoc_table[Cid].e_next_bitmap & E3_CHANGED)
  {
    /* e3 is updated recalc is needed now */
    aoc_calculate_charging_parameter_part2( Cid );
  }

/*
  if(cc_aoc_table[Cid].e_next_bitmap & E4_CHANGED)
  {
    if e4 is updated while charging running, charge amount now 
    if (cc_aoc_table[Cid].aoc_timer_running EQ TRUE)
    {
      aoc_calculate_charging_parameter_part1( Cid );
      aoc_increment_initial_charge( Cid );
      cc_aoc_table[Cid].e_next_bitmap &= ~E4_CHANGED;
    }
  }
*/

}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_getEVal         |
+--------------------------------------------------------------------+

  PURPOSE : Assembles e-value out of the facility IE.
*/

USHORT aoc_getEVal( void * eBuf )
{
  T_e1    *pE  = (T_e1*)eBuf;
  USHORT  val = 0;
  UBYTE   len;

  for( len = 0; len < pE->c_e_val; len++ )
  {
    val<<= 8;
    val += pE->e_val[len];
  }

  return( val );
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  AOC                 |
| STATE  : code                         ROUTINE: aoc_send_ccm        |
+--------------------------------------------------------------------+

  PURPOSE : The ccm is forwarded if at least one of the calls is
            active.
*/

void aoc_send_ccm ()
{
  ULONG ccm_output;
  T_ACI_CMD_SRC idx;

  if (qAT_CallActive())
  {
    /* call back function to notify the MMI */
    for( idx = CMD_SRC_LCL; idx < CMD_SRC_MAX; idx++ )
    {
      /*
       * forward ccm in whole units to MMI
       */
      ccm_output = MC_AOC_ROUND_UP(ccm);

      R_AT( RAT_CCCM, idx )( &ccm_output );
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)             MODULE:  AOC                    |
| STATE  : code                      ROUTINE: aoc_set_time_ut_charge |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to set the period of time until
            the next charging occur.
*/

void aoc_set_time_ut_charge ( SHORT cId, T_TIME      time )
{
  UBYTE mode = AOC_CALL_TIMER_ELAPSED; /* indicates the elapsed */
                                       /* timer                 */
  /*
   * in case an AoC timer is elapsed the time until
   * next charging will be fixed to the new value
   */
  if ( cId NEQ ACI_NumParmNotPresent )
  {
    cc_aoc_table[cId].time_ut_charge = time;
    mode                             = AOC_AOC_TIMER_ELAPSED;
  }

  aoc_calc_expct_charge ( mode );
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)             MODULE:  AOC                    |
| STATE  : code                      ROUTINE: aoc_calc_expct_charge  |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to calculate the amount of charge
            which is going to be consumed during the period of time
            fixed by AOC_MAX_REMAIN_CALL_TIME.
*/

void aoc_calc_expct_charge ( UBYTE mode )
{
  ULONG         expct_charge  = 0;   /* expected charge            */
  T_TIME        remain_time;         /* remaining time until next  */
  ULONG         num_chrg_pts;        /* number of charging points  */
  UBYTE         idx;                 /* used for counting          */
  BOOL          aoc_running = FALSE; /* indicates whether at least */
                                     /* one AoC timer is running   */

  for ( idx = 0; idx < MAX_CALL_NR; idx++ )
  {
    if ( cc_aoc_table[idx].aoc_timer_running AND cc_aoc_table[idx].e1 )
    {
      aoc_running = TRUE;

      if ( mode EQ AOC_CALL_TIMER_ELAPSED )
      {
#if defined (WIN32)
        TRACE_EVENT_P1("time to charge: %d ms",
                       cc_aoc_table[idx].time_ut_charge - AOC_THOUSAND_MILLISECONDS);
#endif

        /* in case the call timer is elapsed the actual
         * time until next charging will be calculated */
        if (cc_aoc_table[idx].time_ut_charge >= AOC_THOUSAND_MILLISECONDS)
        {
          cc_aoc_table[idx].time_ut_charge -= AOC_THOUSAND_MILLISECONDS;
        }
        else
        {
          /* calculate the expected charging intervals based on the E parameter */
          if ( cc_aoc_table[idx].new_data_avail EQ TRUE AND
               cc_aoc_table[idx].e_next_bitmap      & E7_CHANGED )
          {
            cc_aoc_table[idx].time_ut_charge = aoc_calc_time_ut_charge ( idx, E7_CHANGED );
          }
          else if ( cc_aoc_table[idx].e_next_bitmap & E2_CHANGED )
          {
            cc_aoc_table[idx].time_ut_charge = aoc_calc_time_ut_charge ( idx, E2_CHANGED );
          }
          else
          {
            cc_aoc_table[idx].time_ut_charge = ~0L;
          }

          /* calculate the expected initial charge and
           * add to the expected charge */
          /* The fixed amount is charged immediately and not somewhen later
          if ( cc_aoc_table[idx].new_data_avail EQ TRUE         AND
               cc_aoc_table[idx].e_bitmap       &  E4_AVAILABLE     )
          {
            expct_charge += ( ( ULONG ) cc_aoc_table[idx].e3   *
                              ( ULONG ) cc_aoc_table[idx].e4 ) / 10;
          }
          */
        }
      }

      /* calculate expected charge for next AOC_MAX_REMAIN_CALL_TIME milliseconds */
      num_chrg_pts = 1;

      if ( cc_aoc_table[idx].time_ut_charge <= AOC_MAX_REMAIN_CALL_TIME )
      {
        if (cc_aoc_table[idx].e2)
        {
          remain_time = AOC_MAX_REMAIN_CALL_TIME -
                                     cc_aoc_table[idx].time_ut_charge;
          num_chrg_pts +=
              (ULONG)(remain_time / E_IN_MS ( cc_aoc_table[idx].e2 ));
        }

        expct_charge += (   num_chrg_pts *
                          ( cc_aoc_table[idx].e1 * cc_aoc_table[idx].e3 ) / 10 );
      }
    }
  }

  aoc_calc_acm_wrn_evnt ( expct_charge, aoc_running );
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)              MODULE:  AOC                   |
| STATE  : code                       ROUTINE: aoc_calc_acm_wrn_evnt |
+--------------------------------------------------------------------+

  PURPOSE : In case the remaining credit is low a call timer warning
            event is generated.
*/

void aoc_calc_acm_wrn_evnt ( ULONG charge, BOOL aoc_running )
{
  ULONG           credit;      /* remaining credit             */
  ULONG           acm_for_sim; /* lack of the actual ACM value */
  T_ACI_CCWV_CHRG charging;    /* CCWV mode indicated to MMI   */

  /*
   * the update on the SIM is the difference between the rounded up
   * value of the current CCM and the old CCM (already rounded up)
   */
  acm_for_sim = MC_AOC_ROUND_UP(ccm) - MC_AOC_ROUND_UP(ccm_already_incremented);
  credit = acmmax - acm - ( acm_for_sim * 100 );

#if defined (WIN32)
  {
    TRACE_EVENT_P2("AoC charge for next %u ms = %u", AOC_MAX_REMAIN_CALL_TIME, charge );
  }
#endif

  if ( aoc_running EQ TRUE )
  {
    if ( charge >= credit )
      charging = CCWV_CHRG_Shortage;
    else
      charging = CCWV_CHRG_Abundance;
  }
  else
    charging = CCWV_CHRG_Termination;

  aoc_ntfy_acm_wrn_evnt ( charging );
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)              MODULE:  AOC                   |
| STATE  : code                       ROUTINE: aoc_ntfy_acm_wrn_evnt |
+--------------------------------------------------------------------+

  PURPOSE : Notifies the MMI about the call meter warning event.

*/
void aoc_ntfy_acm_wrn_evnt ( T_ACI_CCWV_CHRG charging )
{
  T_ACI_CMD_SRC idx;

  if ( ccwv_charging NEQ charging )
  {
    ccwv_charging = charging;

    for( idx=CMD_SRC_LCL; idx<CMD_SRC_MAX; idx++ )
    {
      R_AT( RAT_CCWV, idx )( charging );
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)            MODULE:  AOC                     |
| STATE  : code                     ROUTINE: aoc_calc_time_ut_charge |
+--------------------------------------------------------------------+

  PURPOSE : This function calculates the new time to charge with
            respect to the previous value.

*/
T_TIME aoc_calc_time_ut_charge       ( SHORT cId,
                                       UBYTE e_value_flag )
{
  T_TIME       new_time = ~0L; /* new time until next charging */
  T_TIME       e_value;        /* interval value               */

  switch ( e_value_flag )
  {
    case (E2_CHANGED): e_value = E_IN_MS ( cc_aoc_table[cId].e2 );
      break;
    case (E7_CHANGED): e_value = E_IN_MS ( cc_aoc_table[cId].e7 );
      break;
    default            : e_value = 0L;
      break;
  }

  if ( e_value <= ( AOC_THOUSAND_MILLISECONDS - cc_aoc_table[cId].time_ut_charge ) )
  {
    new_time = e_value;
  }
  else if ( e_value NEQ 0L )
  {
    new_time = e_value - ( AOC_THOUSAND_MILLISECONDS - cc_aoc_table[cId].time_ut_charge );
  }

  return new_time;
}

/* Implements Measure 213 */
/*
+------------------------------------------------------------------------------
|  Function    : aoc_update_sim_datafield_cb
+------------------------------------------------------------------------------
|  Purpose     : Callback function for updating ACM.or ACMMAX Field 
|                on the SIM Card.
|
|  Parameters  : table_id   - 
|                at_cmd_idn - AT COMMAND Identifier 
|                             (AT_CMD_CAMM or AT_CMD_CACM)
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void aoc_update_sim_datafield_cb(SHORT table_id, T_ACI_AT_CMD at_cmd_idn)
{
  T_SIM_SET_PRM * pSIMSetPrm;  /* points to MM parameter set */

  TRACE_FUNCTION ("aoc_update_sim_datafield_cb()");

  simEntStat.curCmd     = AT_CMD_NONE;
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  switch (simShrdPrm.atb[table_id].errCode)
  {
    case SIM_NO_ERROR:
      /*
       * No error has occured, read ACM etc. again from SIM Card
       */
      R_AT( RAT_OK, simEntStat.entOwn) (at_cmd_idn);
      aoc_sms ();

      if (at_cmd_idn EQ AT_CMD_CAMM)
      {
        aoc_set_time_ut_charge ( ACI_NumParmNotPresent, 0L );
      }
      break;
    case SIM_CAUSE_PIN2_EXPECT:
      /*
       * error has occured, maybe PIN2 is needed
       */
      if (strlen ((char *) pwd) AND
          act_upd_op EQ FIRST_UPDATE)
      {
        /*
         * Password is available and
         * it is the first update, then
         * try to verify PIN2
         */
        act_upd_op = VERIFY_PWD;
        pSIMSetPrm = &simShrdPrm.setPrm[act_src_id];

        cmhSIM_FillInPIN ( (char *) pwd, pSIMSetPrm -> curPIN, PIN_LEN);
        pSIMSetPrm -> PINType = PHASE_2_PIN_2;
        simEntStat.curCmd     = at_cmd_idn;
        simShrdPrm.owner      = (T_OWN)act_src_id;
        simEntStat.entOwn     = act_src_id;

        if ( psaSIM_VerifyPIN() < 0 )  /* verify PIN */
        {
          TRACE_EVENT( "FATAL RETURN psaSIM in +CACM or +CAMM" );
        }
      }
      else
      {
        /*
         * PIN2 not available or second attempt
         */
        if (act_upd_op EQ FIRST_UPDATE)
        {
          R_AT( RAT_CME, simEntStat.entOwn )
               ( at_cmd_idn, CME_ERR_SimPin2Req );
        }
        else
        {
          R_AT( RAT_CME, simEntStat.entOwn )
                ( at_cmd_idn, CME_ERR_WrongPasswd );
        }
      }
      break;
    default:
      /*
       * Any other error, respective error code is returned 
       */
      R_AT( RAT_CME, simEntStat.entOwn )
        (
          at_cmd_idn, 
          cmhSIM_GetCmeFromSim ( simShrdPrm.atb[table_id].errCode )
        );
      break;
  }
}

/* Implements Measure 156 and 157 */
/*
+------------------------------------------------------------------------------
|  Function    : aoc_read_sim_datafield
+------------------------------------------------------------------------------
|  Purpose     : Reads the SIM_ACM , SIM_ACMMAX and SIM_PUCT field 
|                on the SIM card
|
|  Parameters  : accType    - ACCESS TYPE, (ACT_RD_REC or ACT_RD_DAT)
|                reqDataFld - Requested DataField Identifier
|                             (SIM_ACM/SIM_ACMMAX/SIM_PUCT)
|                rplyCB     - Pointer to the Reply Call Back Function
|                             (aoc_read_acm_cb or aoc_read_acmmax_cb)
|                dataLen    - Data Length depending upon the which
|                             field to be read.
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void aoc_read_sim_datafield ( UBYTE accType, USHORT reqDataFld,
                                    void (*rplyCB)(SHORT), UBYTE dataLen)
{
  SHORT table_id;

  TRACE_FUNCTION ("aoc_read_sim_datafield()");

  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].accType      = accType;
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].reqDataFld   = reqDataFld;
    if (reqDataFld EQ SIM_ACM)
    {
      simShrdPrm.atb[table_id].recNr        = 1;
    }
    else
    {
      simShrdPrm.atb[table_id].dataOff      = 0;
    }
    simShrdPrm.atb[table_id].dataLen      = dataLen;
    simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;
    simShrdPrm.atb[table_id].exchData     = sim_data;
    simShrdPrm.atb[table_id].rplyCB       = rplyCB;

    simShrdPrm.aId = table_id;
    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR");
    }
  }
}

/* Implements Measure 152 */
/*
+------------------------------------------------------------------------------
|  Function    : aoc_update_sim_datafield
+------------------------------------------------------------------------------
|  Purpose     : Updates the ACM field on the SIM card
|
|  Parameters  : operation  - FIRST_UPDATE or SECOND_UPDATE
|                accType    - ACCESS TYPE, (ACT_WR_REC or ACT_WR_DAT)
|                reqDataFld - Requested DataField Identifier
|                             (SIM_ACM or SIM_ACMMAX)
|                rplyCB     - Pointer to the Reply Call Back Function
|                             (aoc_update_acm_cb or aoc_update_acmmax_cb)
|
|  Return      : void
+------------------------------------------------------------------------------
*/

void aoc_update_sim_datafield ( UBYTE operation, UBYTE accType,
                                USHORT reqDataFld, void (*rplyCB)(SHORT) )
{
  SHORT table_id;

  TRACE_FUNCTION ("aoc_update_sim_datafield()");

  /*
   * request table id for SIM SAP access
   */
  table_id   = psaSIM_atbNewEntry();
  act_upd_op = operation;

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].accType      = accType;
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].reqDataFld   = reqDataFld;
    simShrdPrm.atb[table_id].dataOff      = 0;
    simShrdPrm.atb[table_id].dataLen      = 3;
    simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;
    simShrdPrm.atb[table_id].exchData     = sim_data;
    simShrdPrm.atb[table_id].rplyCB       = rplyCB;
    simShrdPrm.aId = table_id;

    sim_data[0] = (UBYTE)(act_value >> 16);
    sim_data[1] = (UBYTE)(act_value >> 8);
    sim_data[2] = (UBYTE)(act_value & 0xFF);

    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR");
    }
  }
}

