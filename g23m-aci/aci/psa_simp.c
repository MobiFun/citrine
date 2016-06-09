/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SIMP
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
|  Purpose :  This module defines the processing functions for the
|             primitives send to the protocol stack adapter by the
|             subscriber identity module.
+-----------------------------------------------------------------------------
*/

#ifndef PSA_SIMP_C
#define PSA_SIMP_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifdef DTI
#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#endif

#include "aci.h"
#include "psa.h"
#include "psa_sim.h"
#include "psa_sms.h"
#include "psa_mmi.h"
#include "psa_mm.h"
#include "cmh.h"
#include "cmh_mm.h"
#include "cmh_sim.h"
#include "phb.h"
#include "aoc.h"

#ifdef SIM_TOOLKIT
#include "psa_cc.h"
#include "aci_mem.h"
#include "psa_sat.h"
#include "cmh_sat.h"
#endif /* SIM_TOOLKIT */

#include "aci_ext_pers.h"       /* for SIM locking constants and prototypes. */
#include "aci_slock.h"    /* for SIM locking constants and prototypes. */

#ifdef SIM_PERS
#include "general.h" // included for compilation error UINT8 in sec_drv.h
#include "sec_drv.h" 
#endif
/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/* Remember the last catched T_SIM_MMI_INSERT_IND, null if the last one as not remembered or freed */
T_SIM_MMI_INSERT_IND *last_sim_mmi_insert_ind = NULL;
#ifdef SIM_PERS
 EXTERN T_SEC_DRV_CONFIGURATION *cfg_data ;
 EXTERN  T_ACI_SIM_CONFIG aci_slock_sim_config;
#endif
 EXTERN void psaSIM_Insert_Continued(T_SIM_MMI_INSERT_IND *sim_mmi_insert_ind);


LOCAL void psaSIM_process_sim_upd_rec_cnf ( U8 req_id, U16   cause, 
                                            UBYTE type );
LOCAL void psaSIM_update_simShrdPrm  ( U16 cause, U8 pin_id);

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_read_cnf        |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_READ_CNF primitive send by SIM.
            this is the confirmation to the SIM read data operation.

*/
GLOBAL void psa_sim_read_cnf ( T_SIM_READ_CNF *sim_read_cnf )
{
  SHORT aId;              /* holds access id */

  TRACE_FUNCTION ("psa_sim_read_cnf()");

  aId = sim_read_cnf -> req_id;

  if( simShrdPrm.atb[aId].ntryUsdFlg AND 
      simShrdPrm.atb[aId].accType EQ ACT_RD_DAT )
  {
    /*
     *---------------------------------------------------------------
     * update access parameter and notify caller
     *---------------------------------------------------------------
     */
    simShrdPrm.atb[aId].errCode = sim_read_cnf -> cause;
    simShrdPrm.atb[aId].dataLen = sim_read_cnf -> length;

    if( simShrdPrm.atb[aId].exchData )
      memcpy (simShrdPrm.atb[aId].exchData, sim_read_cnf -> trans_data,
              sim_read_cnf->length);
    else
      simShrdPrm.atb[aId].exchData = sim_read_cnf -> trans_data;

    if( simShrdPrm.atb[aId].rplyCB )
      simShrdPrm.atb[aId].rplyCB( aId );
    else
      simShrdPrm.atb[aId].ntryUsdFlg = FALSE;
  }

    /*
     *-------------------------------------------------------------------
     * free the primitive buffer
     *-------------------------------------------------------------------
     */
  PFREE (sim_read_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_update_cnf      |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_READ_CNF primitive send by SIM.
            this is the confirmation to the SIM update data operation.

*/

GLOBAL void psa_sim_update_cnf ( T_SIM_UPDATE_CNF *sim_update_cnf )
{


  TRACE_FUNCTION ("psa_sim_update_cnf()");

  /* Implements Measure # 48 */
  psaSIM_process_sim_upd_rec_cnf ( sim_update_cnf -> req_id,
                                   sim_update_cnf -> cause,
                                   ACT_WR_DAT);
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_update_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_read_record_cnf |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_READ_RECORD_CNF primitive send by SIM.
            this is the confirmation to the SIM read absolute record
            operation.

*/

GLOBAL void psa_sim_read_record_cnf
                       ( T_SIM_READ_RECORD_CNF *sim_read_record_cnf )
{
  SHORT aId;              /* holds access id */

  TRACE_FUNCTION ("psa_sim_read_record_cnf()");

  aId = sim_read_record_cnf -> req_id;  

  if( simShrdPrm.atb[aId].ntryUsdFlg AND 
      simShrdPrm.atb[aId].accType EQ ACT_RD_REC )
  {
    /*
     *---------------------------------------------------------------
     * update access parameter and notify caller
     *---------------------------------------------------------------
     */
    simShrdPrm.atb[aId].errCode = sim_read_record_cnf -> cause;
    simShrdPrm.atb[aId].recMax  = sim_read_record_cnf -> max_record;
    if(simShrdPrm.atb[aId].check_dataLen                       AND
       simShrdPrm.atb[aId].dataLen < sim_read_record_cnf->length)
    {
      TRACE_EVENT_P1("Read record is too big for buffer !!! size: %d", sim_read_record_cnf->length);
    }
    else
    {
      simShrdPrm.atb[aId].dataLen = sim_read_record_cnf -> length;
    }

    if( simShrdPrm.atb[aId].exchData )

      memcpy (simShrdPrm.atb[aId].exchData, sim_read_record_cnf -> linear_data,
            sim_read_record_cnf->length);
    else

      simShrdPrm.atb[aId].exchData = sim_read_record_cnf -> linear_data;

    if( simShrdPrm.atb[aId].rplyCB )
      simShrdPrm.atb[aId].rplyCB( aId );
    else
      simShrdPrm.atb[aId].ntryUsdFlg = FALSE;
  }
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_read_record_cnf);

}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)        MODULE  : PSA_SIMP                 |
|                                ROUTINE : psa_sim_update_record_cnf|
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_READ_CNF primitive send by SIM.
            this is the confirmation to the SIM update absolute
            record operation.

*/

GLOBAL void psa_sim_update_record_cnf
                    ( T_SIM_UPDATE_RECORD_CNF *sim_update_record_cnf )
{

  TRACE_FUNCTION ("psa_sim_update_record_cnf()");

  /* Implements Measure # 48 */
  psaSIM_process_sim_upd_rec_cnf ( sim_update_record_cnf -> req_id,
                                   sim_update_record_cnf -> cause, 
                                   ACT_WR_REC);
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_update_record_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_increment_cnf   |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_READ_CNF primitive send by SIM.
            this is the confirmation to the SIM increment data
            operation.

*/

GLOBAL void psa_sim_increment_cnf
                          ( T_SIM_INCREMENT_CNF *sim_increment_cnf )
{
  SHORT aId;              /* holds access id */

  TRACE_FUNCTION ("psa_sim_increment_cnf()");

  aId = sim_increment_cnf -> req_id;  

  if( simShrdPrm.atb[aId].ntryUsdFlg AND 
      simShrdPrm.atb[aId].accType EQ ACT_INC_DAT )
  {
    /*
     *---------------------------------------------------------------
     * update access parameter and notify caller
     *---------------------------------------------------------------
     */
    simShrdPrm.atb[aId].errCode   = sim_increment_cnf -> cause;
    simShrdPrm.atb[aId].dataLen   = sim_increment_cnf -> length;

    if( simShrdPrm.atb[aId].exchData )

      memcpy( simShrdPrm.atb[aId].exchData,
              sim_increment_cnf -> linear_data,
              sim_increment_cnf -> length );
    else

      simShrdPrm.atb[aId].exchData = sim_increment_cnf -> linear_data;

    if( simShrdPrm.atb[aId].rplyCB )
      simShrdPrm.atb[aId].rplyCB( aId );
    else
      simShrdPrm.atb[aId].ntryUsdFlg = FALSE;
  }
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_increment_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_verify_pin_cnf  |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_VERIFY_PIN_CNF primitive send by SIM.
            this is the confirmation to the PIN verify operation.

*/

GLOBAL void psa_sim_verify_pin_cnf
                         ( T_SIM_VERIFY_PIN_CNF *sim_verify_pin_cnf )
{

  TRACE_FUNCTION ("psa_sim_verify_pin_cnf()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  simShrdPrm.rslt   = sim_verify_pin_cnf -> cause;
  simShrdPrm.pn1Cnt = sim_verify_pin_cnf -> pin_cnt;
  simShrdPrm.pn2Cnt = sim_verify_pin_cnf -> pin2_cnt;
  simShrdPrm.pk1Cnt = sim_verify_pin_cnf -> puk_cnt;
  simShrdPrm.pk2Cnt = sim_verify_pin_cnf -> puk2_cnt;

  TRACE_EVENT_P1("SIM answered with 0x%4.4X", sim_verify_pin_cnf -> cause);

  /* Implements Measure # 179 */
  psaSIM_update_simShrdPrm ( sim_verify_pin_cnf -> cause, 
                             sim_verify_pin_cnf -> pin_id);

  cmhSIM_PINVerified();

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_verify_pin_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_change_pin_cnf  |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_CHANGE_PIN_CNF primitive send by SIM.
            this is the confirmation to the PIN change operation.

*/

GLOBAL void psa_sim_change_pin_cnf
                         ( T_SIM_CHANGE_PIN_CNF *sim_change_pin_cnf )
{

  TRACE_FUNCTION ("psa_sim_change_pin_cnf()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  simShrdPrm.rslt   = sim_change_pin_cnf -> cause;
  simShrdPrm.pn1Cnt = sim_change_pin_cnf -> pin_cnt;
  simShrdPrm.pn2Cnt = sim_change_pin_cnf -> pin2_cnt;
  simShrdPrm.pk1Cnt = sim_change_pin_cnf -> puk_cnt;
  simShrdPrm.pk2Cnt = sim_change_pin_cnf -> puk2_cnt;

  /* Implements Measure # 179 */
  psaSIM_update_simShrdPrm ( sim_change_pin_cnf -> cause, 
                             sim_change_pin_cnf -> pin_id);
  /* Implements Measure 97 */
  cmhSIM_CardUnblocked_PINChanged( TRUE );

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_change_pin_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_disable_pin_cnf |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_DISABLE_PIN_CNF primitive send by SIM.
            this is the confirmation to the PIN disable operation.

*/

GLOBAL void psa_sim_disable_pin_cnf
                      ( T_SIM_DISABLE_PIN_CNF *sim_disable_pin_cnf )
{

  TRACE_FUNCTION ("psa_sim_disable_pin_cnf()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  simShrdPrm.rslt   = sim_disable_pin_cnf -> cause;
  simShrdPrm.pn1Cnt = sim_disable_pin_cnf -> pin_cnt;
  simShrdPrm.pn2Cnt = sim_disable_pin_cnf -> pin2_cnt;
  simShrdPrm.pk1Cnt = sim_disable_pin_cnf -> puk_cnt;
  simShrdPrm.pk2Cnt = sim_disable_pin_cnf -> puk2_cnt;
  
  switch( sim_disable_pin_cnf -> cause )
  {
    case( SIM_CAUSE_PUK1_EXPECT ):
    case( SIM_CAUSE_PIN1_BLOCKED):
      simShrdPrm.PINStat = PS_PUK1;
      break;
    /* Implements Measure # 101 */
    case( SIM_NO_ERROR):
      simShrdPrm.PEDStat = PEDS_DIS;
      break;
  }

  /* Implements Measure 183 */
  cmhSIM_PINEnabledDisabled();

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_disable_pin_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_enable_pin_cnf  |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_ENABLE_PIN_CNF primitive send by SIM.
            this is the confirmation to the PIN enable operation.

*/

GLOBAL void psa_sim_enable_pin_cnf
                         ( T_SIM_ENABLE_PIN_CNF *sim_enable_pin_cnf )
{

  TRACE_FUNCTION ("psa_sim_enable_pin_cnf()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  simShrdPrm.rslt   = sim_enable_pin_cnf -> cause;
  simShrdPrm.pn1Cnt = sim_enable_pin_cnf -> pin_cnt;
  simShrdPrm.pn2Cnt = sim_enable_pin_cnf -> pin2_cnt;
  simShrdPrm.pk1Cnt = sim_enable_pin_cnf -> puk_cnt;
  simShrdPrm.pk2Cnt = sim_enable_pin_cnf -> puk2_cnt;

  switch( sim_enable_pin_cnf -> cause )
  {
    case( SIM_CAUSE_PUK1_EXPECT ):
    case( SIM_CAUSE_PIN1_BLOCKED):
      simShrdPrm.PINStat = PS_PUK1;
     break;
    /* Implements Measure # 101 */
    case( SIM_NO_ERROR):
      simShrdPrm.PEDStat = PEDS_ENA;
      break;
  }

  /* Implements Measure 183 */
  cmhSIM_PINEnabledDisabled();
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_enable_pin_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_unblock_cnf     |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_UNBLOCK_CNF primitive send by SIM.
            this is the confirmation to the card unblock operation.

*/

GLOBAL void psa_sim_unblock_cnf
                              ( T_SIM_UNBLOCK_CNF *sim_unblock_cnf )
{

  TRACE_FUNCTION ("psa_sim_unblock_cnf()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  simShrdPrm.rslt   = sim_unblock_cnf -> cause;
  simShrdPrm.pn1Cnt = sim_unblock_cnf -> pin_cnt;
  simShrdPrm.pn2Cnt = sim_unblock_cnf -> pin2_cnt;
  simShrdPrm.pk1Cnt = sim_unblock_cnf -> puk_cnt;
  simShrdPrm.pk2Cnt = sim_unblock_cnf -> puk2_cnt;

  TRACE_EVENT_P1("SIM answered with 0x%4.4X", sim_unblock_cnf -> cause);

  switch( sim_unblock_cnf -> cause )
  {
    case( SIM_NO_ERROR ):
      simShrdPrm.PINStat = PS_RDY;
    
      if( sim_unblock_cnf -> pin_id EQ PHASE_2_PUK_1 )
      {
        simShrdPrm.pn1Stat = PS_RDY;
        /* 11.11/8.13 "After a successful unblocking attempt the CHV is enabled..." */
        simShrdPrm.PEDStat = PEDS_ENA;
      }
      else if( sim_unblock_cnf -> pin_id EQ PHASE_2_PUK_2 )
        simShrdPrm.pn2Stat = PS_RDY;
      break;

    case( SIM_CAUSE_PUK1_EXPECT ):
      simShrdPrm.PINStat = PS_PUK1;
      break;

    case( SIM_CAUSE_PUK2_EXPECT ):
      simShrdPrm.PINStat = PS_PUK2;
      break;
  }
  /* Implements Measure 97 */
  cmhSIM_CardUnblocked_PINChanged( FALSE );

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_unblock_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_sync_cnf        |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_SYNC_CNF primitive send by SIM.
            this is the confirmation to the data synchronization
            operation.

*/

GLOBAL void psa_sim_sync_cnf ( T_SIM_SYNC_CNF *sim_sync_cnf )
{

  TRACE_FUNCTION ("psa_sim_sync_cnf()");

  /* disable SMS */
  if (simShrdPrm.synCs EQ SYNC_DEACTIVATE)
  {
    cmhSMS_disableAccess();
    psaSIM_Init (ACI_INIT_TYPE_SOFT_OFF);
    percentCSTAT_indication(STATE_MSG_PBOOK, ENTITY_STATUS_NotReady);
  }

  /* notify ACI */
  cmhSIM_SIMSync();

  /* free the primitive buffer */
  PFREE (sim_sync_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_activate_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_ACTIVATE_CNF primitive send by SIM.
            this is the confirmation to the SIM activate operation.

*/

GLOBAL void psa_sim_activate_cnf
                            ( T_SIM_ACTIVATE_CNF *sim_activate_cnf )
{
  TRACE_FUNCTION ("psa_sim_activate_cnf()");

#ifdef FF_DUAL_SIM
  if(simShrdPrm.SIM_Selection)
  {
    simShrdPrm.rslt = sim_activate_cnf -> cause;
    simShrdPrm.SIM_Powered_on = sim_activate_cnf->sim_num;

    cmhSIM_SIMSelected();
    simShrdPrm.SIM_Selection = FALSE;

    PFREE (sim_activate_cnf);

    return;

  }
#endif /*FF_DUAL_SIM*/
/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */

  if (simShrdPrm.setPrm[simEntStat.entOwn].actProc EQ SIM_INITIALISATION)
  {
    simShrdPrm.pn1Cnt = sim_activate_cnf -> pin_cnt;
    simShrdPrm.pn2Cnt = sim_activate_cnf -> pin2_cnt;
    simShrdPrm.pk1Cnt = sim_activate_cnf -> puk_cnt;
    simShrdPrm.pk2Cnt = sim_activate_cnf -> puk2_cnt;
  }

  /*
   * Build emergency call phonebook
   */
#ifdef TI_PS_FFS_PHB
  pb_set_sim_ecc (sim_activate_cnf->cause,
                  MAX_ECC,
                  sim_activate_cnf->ec_code);
#else
  pb_init();
  pb_read_ecc(sim_activate_cnf -> cause, MAX_ECC,
              sim_activate_cnf->ec_code);
#endif

  simShrdPrm.rslt = sim_activate_cnf -> cause;
  TRACE_EVENT_P1("SIM answered with 0x%4.4X", sim_activate_cnf -> cause);

#ifdef TI_PS_FF_AT_P_CMD_ATR
  simShrdPrm.atr.len = MINIMUM(sim_activate_cnf->c_atr, MAX_SIM_ATR);
  memcpy (simShrdPrm.atr.data, sim_activate_cnf -> atr, MINIMUM(sim_activate_cnf->c_atr, MAX_SIM_ATR));
#endif /* TI_PS_FF_AT_P_CMD_ATR */

  switch( simShrdPrm.rslt )
  {
    case( SIM_NO_ERROR ):
      simShrdPrm.SIMStat = SS_OK;
      simShrdPrm.PINStat = simShrdPrm.pn1Stat = PS_RDY;
      if (simShrdPrm.setPrm[simEntStat.entOwn].actProc EQ SIM_INITIALISATION)
        simShrdPrm.PEDStat = PEDS_DIS;
      break;

    case( SIM_CAUSE_PIN1_EXPECT ):

      simShrdPrm.SIMStat = SS_OK;
      simShrdPrm.PINStat = simShrdPrm.pn1Stat = PS_PIN1;
      simShrdPrm.PEDStat = PEDS_ENA;
      break;

    case( SIM_CAUSE_PIN2_EXPECT ):

      simShrdPrm.SIMStat = SS_OK;
      simShrdPrm.PINStat = simShrdPrm.pn2Stat = PS_PIN2;
      break;

    case( SIM_CAUSE_PIN1_BLOCKED ):
    case( SIM_CAUSE_PUK1_EXPECT ):

      simShrdPrm.SIMStat = SS_BLKD;
      simShrdPrm.PINStat = PS_PUK1;
      simShrdPrm.PEDStat = PEDS_ENA;
      break;

    case( SIM_CAUSE_PIN2_BLOCKED ):
    case( SIM_CAUSE_PUK2_EXPECT ):

      simShrdPrm.SIMStat = SS_BLKD;
      simShrdPrm.PINStat = PS_PUK2;
      break;

    case( SIM_CAUSE_PUK1_BLOCKED ):
      simShrdPrm.SIMStat = SS_INV;
      simShrdPrm.pn1Stat = NO_VLD_PS;
      break;

    case( SIM_CAUSE_PUK2_BLOCKED ):
      simShrdPrm.SIMStat = SS_INV;
      simShrdPrm.pn2Stat = NO_VLD_PS;
      break;

    default:
      if (GET_CAUSE_DEFBY(simShrdPrm.rslt) NEQ DEFBY_CONDAT AND
          GET_CAUSE_ORIGSIDE(simShrdPrm.rslt) NEQ ORIGSIDE_MS)
      {
        /* unexpected result */
        simShrdPrm.SIMStat = NO_VLD_SS;
        TRACE_ERROR("psa_sim_activate_cnf: NO_VLD_SS");
        break;
      } /* no break: SIM driver error */
      /*lint -fallthrough*/
    case( SIM_CAUSE_CARD_REMOVED ):
      simShrdPrm.SIMStat = SS_URCHB;
      simShrdPrm.pn1Stat = simShrdPrm.pn2Stat = NO_VLD_PS;
      break;
  }

  cmhSIM_SIMActivated();

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_activate_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_activate_ind    |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_ACTIVATE_IND primitive send by SIM.
            This is the indication about the recognition of a SIM
            card after a previous SIM_REMOVE_IND or failed
            SIM_ACTIVATE_REQ
*/

GLOBAL void psa_sim_activate_ind
                            ( T_SIM_ACTIVATE_IND *sim_activate_ind )
{

  T_ACI_CMD_SRC idx;

  TRACE_FUNCTION ("psa_sim_activate_ind()");

  /* Check for the cause value and inform MMI accordingly with %SIMINS */ 
  if (sim_activate_ind->cause EQ SIM_CAUSE_SIM_REINSERTED)
  {
    for( idx = CMD_SRC_LCL; idx < CMD_SRC_MAX; idx++ )
    {
      R_AT( RAT_SIMINS, idx )(CME_ERR_SimResetNeeded);
    }
  }
  else
  {
  simShrdPrm.pn1Cnt = sim_activate_ind -> pin_cnt;
  simShrdPrm.pn2Cnt = sim_activate_ind -> pin2_cnt;
  simShrdPrm.pk1Cnt = sim_activate_ind -> puk_cnt;
  simShrdPrm.pk2Cnt = sim_activate_ind -> puk2_cnt;

  /*
   * Build emergency call phonebook
   */
#ifdef TI_PS_FFS_PHB
  pb_set_sim_ecc (sim_activate_ind->cause,
                  MAX_ECC,
                  sim_activate_ind->ec_code);
#else
  pb_read_ecc(sim_activate_ind->cause, MAX_ECC,
              sim_activate_ind->ec_code);
#endif

  simShrdPrm.rslt = sim_activate_ind -> cause;

#ifdef TI_PS_FF_AT_P_CMD_ATR
  simShrdPrm.atr.len = MINIMUM(sim_activate_ind->c_atr, MAX_SIM_ATR);
  memcpy (simShrdPrm.atr.data, sim_activate_ind -> atr, MINIMUM(sim_activate_ind->c_atr, MAX_SIM_ATR));
#endif /* TI_PS_FF_AT_P_CMD_ATR */

  switch( simShrdPrm.rslt )
  {
    case( SIM_NO_ERROR ):
      simShrdPrm.rslt    = SIM_NO_ERROR;
      simShrdPrm.SIMStat = SS_OK;
      simShrdPrm.PINStat = simShrdPrm.pn1Stat = PS_RDY;
      simShrdPrm.PEDStat = PEDS_DIS;
      break;

    case( SIM_CAUSE_PIN1_EXPECT ):
      simShrdPrm.SIMStat = SS_OK;
      simShrdPrm.PINStat = simShrdPrm.pn1Stat = PS_PIN1;
      simShrdPrm.PEDStat = PEDS_ENA;
      break;

    case( SIM_CAUSE_PIN2_EXPECT ):
      simShrdPrm.SIMStat = SS_OK;
      simShrdPrm.PINStat = simShrdPrm.pn2Stat = PS_PIN2;
      break;

    case( SIM_CAUSE_PIN1_BLOCKED ):
    case( SIM_CAUSE_PUK1_EXPECT ):
      simShrdPrm.SIMStat = SS_BLKD;
      simShrdPrm.PINStat = PS_PUK1;
      simShrdPrm.PEDStat = PEDS_ENA;
      break;

    case( SIM_CAUSE_PIN2_BLOCKED ):
    case( SIM_CAUSE_PUK2_EXPECT ):
      simShrdPrm.SIMStat = SS_BLKD;
      simShrdPrm.PINStat = PS_PUK2;
      break;

    default:            /* unexpected error */
      simShrdPrm.SIMStat = NO_VLD_SS;
      simShrdPrm.pn1Stat = simShrdPrm.pn2Stat = NO_VLD_PS;
  }

  cmhSIM_SIMActivated();
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_activate_ind);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_mmi_insert_ind  |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_MMI_INSERT_IND primitive send by SIM.
            This is an indication that the SIM card was inserted
            and verified.
*/

GLOBAL void psa_sim_mmi_insert_ind
                         ( T_SIM_MMI_INSERT_IND *sim_mmi_insert_ind )
{

  TRACE_FUNCTION ("psa_sim_mmi_insert_ind()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */

  if(simShrdPrm.PINStat EQ PS_PUK1)
  {
    TRACE_ERROR("Simcard is blocked !!! sim_mmi_insert_ind should no come before PUK");
    PFREE (sim_mmi_insert_ind);
    return;
  }

  simShrdPrm.SIMStat = SS_OK;      /* moved here */ /* ACI-SPR-10214 */
  simShrdPrm.crdPhs  = sim_mmi_insert_ind -> phase;
  simShrdPrm.crdFun  = sim_mmi_insert_ind -> func;
  simShrdPrm.imsi    = sim_mmi_insert_ind -> imsi_field;

  memcpy( simShrdPrm.srvTab, sim_mmi_insert_ind -> sim_serv,SRV_TAB_LEN );

  /* disable SMS access (can be SIM Re-Initialosation) */
  cmhSMS_disableAccess();

  psaSMS_InitParams();      /* Reset SIM Parameters */
#ifdef SIM_PERS_OTA
  aci_slock_ota_init();
#endif

   /* Initialize Advice of Charge */
  aoc_init (sim_mmi_insert_ind->phase,
            sim_mmi_insert_ind->sim_serv);

  /*
   * Start to build phonebook
   */
  pb_reset();

#ifndef TI_PS_FFS_PHB  
  pb_init();
  pb_update_ecc();
#endif
  
  last_sim_mmi_insert_ind = sim_mmi_insert_ind ;
  
  /* Access the AD from sim_mmi_insert_ind and inform MMI */ 
  cmhSIM_AD_Updated(last_sim_mmi_insert_ind->c_ad, last_sim_mmi_insert_ind->ad);

if(last_sim_mmi_insert_ind NEQ NULL)
{
 #ifdef SIM_PERS
   aci_slock_sim_config.sim_read_ad_first_byte = last_sim_mmi_insert_ind->ad[0] ; 
   aci_slock_sim_init(last_sim_mmi_insert_ind);
   if(aci_slock_is_timer_support() EQ TRUE)
   {
      if(aci_slock_check_timer() EQ TIMER_RUNNING)
      {
         aci_slock_start_timer(); 
      }
     
    }
  /* To set the global variable config data */
 
   if(!aci_slock_set_CFG())
   {
     AciSLockShrd.blocked = TRUE;
     cmhSIM_SIMInserted();
     PFREE (last_sim_mmi_insert_ind); /* 11_Apr_05 */
     last_sim_mmi_insert_ind= NULL;
     return;     
   }
   aci_slock_init();
   AciSLockShrd.pb_load = FALSE; 
   AciSLockShrd.check_lock = SIMLOCK_CHECK_PERS;
   aci_slock_checkpersonalisation(SIMLOCK_NETWORK);

   #else
    /*
     * Start to build phonebook
     */
      pb_reset();

#ifdef TI_PS_FFS_PHB
      pb_inserted_sim (MAX_SRV_TBL,
                       last_sim_mmi_insert_ind->sim_serv,
                       &last_sim_mmi_insert_ind->imsi_field,
                       last_sim_mmi_insert_ind->func,
                       last_sim_mmi_insert_ind->phase);
#else
      pb_build_req(last_sim_mmi_insert_ind);
#endif
     /* Request the Customer Service Profile  from the SIM (EF_CPHS_CSP) */
      cmhSIM_Get_CSP();

      #ifdef SIM_TOOLKIT
      cmhSMS_ReadCbDtaDwnl (last_sim_mmi_insert_ind);
      #endif

      #ifdef FF_MMI_RIV
       rAT_PlusCFUNP (last_sim_mmi_insert_ind);
      #endif /* FF_MMI_RIV */
      PFREE (last_sim_mmi_insert_ind); /* 11_Apr_05 */
      last_sim_mmi_insert_ind= NULL;

      cmhSIM_SIMInserted();
#endif  
} 
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_remove_ind      |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_REMOVE_IND primitive send by SIM.
            this is an indication that the SIM card was removed.

*/

GLOBAL void psa_sim_remove_ind
                         ( T_SIM_REMOVE_IND *sim_remove_ind )
{

  TRACE_FUNCTION ("psa_sim_remove_ind()");

  /*
   *-------------------------------------------------------------------
   * disable SMS access
   *-------------------------------------------------------------------
   */
  cmhSMS_disableAccess();

  smsShrdPrm.cbmPrm.cbmHndl = BM0;         /* switch off CBCH */
  smsShrdPrm.cbmPrm.cbchOwner = OWN_SRC_SAT;
  psaMMI_Cbch();
  smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)CMD_SRC_NONE;
  /*
   * reset phonebook, AOC, ...
   */
  pb_reset();
  aoc_reset();

  #ifdef SIM_PERS
 /*
   * reset aci_slock
   */
  aci_slock_reset(); 
  
  #endif

  /* reset the MM shared parameters */
  psaMM_Init();

  /*
   * erase event list for SAT event download
   */
#ifdef SIM_TOOLKIT
   satShrdPrm.event.list = 0L;
   satShrdPrm.event.temp_list = 0L; /* in case a setup list was in process */
#endif /* SIM_TOOLKIT */

  /*
   * close all open SIM accesses
   */
  psaSIM_CloseAtb ((USHORT)((sim_remove_ind->cause EQ SIM_NO_ERROR)?
                    SIM_CAUSE_SAT_BUSY: sim_remove_ind->cause));

  /* Issue OMAPS00058768: Reset ONSDesc */
  cmhMM_Reset_ONSDesc();

  /*
   *-------------------------------------------------------------------
   * Notification is sent for reason of SIM remove ind and SIM shared 
   * parameters are reset
   *-------------------------------------------------------------------
   */
  simShrdPrm.rslt    = sim_remove_ind->cause;
  cmhSIM_SIMRemoved();
  psaSIM_Init(ACI_INIT_TYPE_SOFT_OFF);

  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */
  PFREE (sim_remove_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_access_cnf          |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_ACCESS_IND primitive send by SIM.
            this is a confirmation to a previous SIM access request.

*/

GLOBAL void psa_sim_access_cnf
                         ( T_SIM_ACCESS_CNF *sim_access_cnf )
{
  T_SIM_TRNS_RSP_PRM rsp;

  TRACE_FUNCTION ("psa_sim_access_cnf()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */
  simShrdPrm.rslt = sim_access_cnf -> cause;

  rsp.sw1    = sim_access_cnf -> sw1;
  rsp.sw2    = sim_access_cnf -> sw2;
  rsp.rspLen = MINIMUM(sim_access_cnf -> c_trans_data, MAX_SIM_CMD);
  rsp.rsp    = sim_access_cnf -> trans_data;

  switch( simShrdPrm.rslt )
  {
    case( SIM_CAUSE_PIN1_EXPECT ):
      simShrdPrm.SIMStat = SS_OK;
      simShrdPrm.PINStat = simShrdPrm.pn1Stat = PS_PIN1;
      simShrdPrm.PEDStat = PEDS_ENA;
      break;

    case( SIM_CAUSE_PIN2_EXPECT ):
      simShrdPrm.SIMStat = SS_OK;
      simShrdPrm.PINStat = simShrdPrm.pn2Stat = PS_PIN2;
      break;

    case( SIM_CAUSE_PIN1_BLOCKED ):
    case( SIM_CAUSE_PUK1_EXPECT ):
      simShrdPrm.SIMStat = SS_BLKD;
      simShrdPrm.PINStat = PS_PUK1;
      simShrdPrm.PEDStat = PEDS_ENA;
      break;

    case( SIM_CAUSE_PIN2_BLOCKED ):
    case( SIM_CAUSE_PUK2_EXPECT ):
      simShrdPrm.SIMStat = SS_BLKD;
      simShrdPrm.PINStat = PS_PUK2;
      break;
  }

  cmhSIM_SIMResponseData( &rsp );

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */

  PFREE (sim_access_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS ()             MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_bip_config_cnf  |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_BIP_CONFIG_CNF primitive sent by SIM.
            Signal application about (un-)successful processing of AT_A 
            command. Clean-up the temporal parameters.
            The primitive has no content!
*/

#ifdef FF_SAT_E 
#ifdef DTI
GLOBAL void psa_sim_bip_config_cnf(T_SIM_BIP_CONFIG_CNF *sim_bip_config_cnf)
{

  T_ACI_SAT_TERM_RESP resp_data;      /* holds terminal response parms */
  UBYTE res;

  TRACE_FUNCTION("psa_sim_bip_config_cnf()");
  
  /* Since there is no content within this primitive, free it */
  PFREE(sim_bip_config_cnf);

  /* check for OPEN CHANNEL command context, immediate channel */
  if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND
      satShrdPrm.cmdDet.cmdType EQ SAT_CMD_OPEN_CHANNEL )
  {
    /* command: OPEN CHANNEL IMMEDIATELY */
    
    /* init terminal response */
    psaSAT_InitTrmResp( &resp_data );
    resp_data.chnStat  = TRUE;
    resp_data.bufSize  = TRUE;
    resp_data.bearDesc = TRUE;

    /* check for modification of bearer parameters */
    res = (satShrdPrm.opchPrmMdf)?RSLT_PERF_MDFIED:
          (satShrdPrm.opchCCMdfy)?RSLT_PERF_MDFY_SIM:RSLT_PERF_SUCCESS;

    /* send terminal response to SAT */
    psaSAT_SendTrmResp( res, &resp_data );

    /* finish command AT_A command */
    R_AT( RAT_OK, (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc ) ( AT_CMD_A );

    /* log result */
    cmh_logRslt ( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, RAT_OK, AT_CMD_A,
                            -1,BS_SPEED_NotPresent,CME_ERR_NotPresent );

    /* reset/free satShrdPrm.opch* parameters */
    cmhSAT_cleanupOpChnPrms();    
  }
  /* check for SEND DATA command context, on-demand channel */
  else if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND
           satShrdPrm.cmdDet.cmdType EQ SAT_CMD_SEND_DATA )
  {
    /* command: SEND DATA immediately */

    /* NO terminal response and no response to Application needed */

    /* reset/free satShrdPrm.opch* parameters */
    cmhSAT_cleanupOpChnPrms();
  }

  /* FREE channel parameter */
  if (simShrdPrm.sim_dti_chPrm NEQ NULL)
  {
    ACI_MFREE(simShrdPrm.sim_dti_chPrm);
    simShrdPrm.sim_dti_chPrm = NULL;
    TRACE_EVENT("FREE sim_dti_chPrm");
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS ()             MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_bip_cnf         |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_BIP_CNF primitive sent by SIM.

*/

GLOBAL void psa_sim_bip_cnf(T_SIM_BIP_CNF *sim_bip_cnf)
{
  /* The callback function is used to proceed the action that has been triggered, 
   * for instance, OPEN BIP channel. The result given by the bip connection 
   * qualifier may differ from the intended action, because this is actually 
   * the indicator whether SIM has processed it correctly! */

  TRACE_FUNCTION ("psa_sim_bip_cnf()");

  if((sim_bip_cnf->bip_conn & simShrdPrm.sim_dti_chPrm->sat_chn_prm.bipConn) > 0)
  {
    TRACE_EVENT("psa_sim_bip_cnf: BIP operation successful");
  }
  else
  {
    TRACE_EVENT("psa_sim_bip_cnf: BIP operation not successful");
  }

  /* 
   * go on with requested action           
   */
  if( simShrdPrm.sim_dti_chPrm->bip_cb )
  {
    /* FREE BIP channel parameter */
    simShrdPrm.sim_dti_chPrm->bip_cb(sim_bip_cnf->bip_conn, 
                                     sim_bip_cnf->bip_ch_id);
  }
  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS ()             MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_dti_cnf         |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_DTI_CNF primitive sent by SIM.

*/

GLOBAL void psa_sim_dti_cnf (T_SIM_DTI_CNF *sim_dti_cnf)
{

  TRACE_FUNCTION ("psa_sim_dti_cnf()");

  if(sim_dti_cnf->dti_conn EQ simShrdPrm.sim_dti_chPrm->sat_chn_prm.dtiConn)
  {
    /* REQUEST SUCCESSFUL */ 
    if( sim_dti_cnf->dti_conn EQ (SIM_DTI_CONNECT) )
    { 
      /* SUCCESSFUL DTI CONNECTION */
      /* 
       * inform dti manager about successfull connection, 
       * call cb of DTI initiator 
       */
      dti_cntrl_entity_connected (sim_dti_cnf->link_id, DTI_ENTITY_SIM, DTI_OK);
    }
    else if( sim_dti_cnf->dti_conn EQ SIM_DTI_DISCONNECT )
    { 
      /* SUCCESSFUL DTI DISCONNECTON */
      /* check whether the BIP channel has to be dropped */
      if( simShrdPrm.sim_dti_chPrm->sat_chn_prm.bipConn 
          EQ SIM_BIP_CLOSE_CHANNEL )
      {
        /* resetting stored SIM DTI ID */
        psaSIM_Bip_Req();
      }
      /* 
       *inform dti manager about successfull disconnection, 
       * call cb of DTI function 
       */
      
      dti_cntrl_entity_disconnected (sim_dti_cnf->link_id, DTI_ENTITY_SIM);

    }
    /* callback to SAT Source */ 
    if( simShrdPrm.sim_dti_chPrm->dti_cb )
    {
       simShrdPrm.sim_dti_chPrm->dti_cb(sim_dti_cnf->dti_conn,
                                        sim_dti_cnf->link_id);
    }
  }
  else
  {
    /* UNSUCCESSFUL DTI CONNECTION OR DISCONNECTION*/
    if(sim_dti_cnf->dti_conn EQ SIM_DTI_DISCONNECT)
    {
      /* UNSUCCESSFUL CONNECTION */
      /* inform dti manager that connection request failed */
      dti_cntrl_entity_connected (sim_dti_cnf->link_id, DTI_ENTITY_SIM, DTI_ERROR);
    }
    /* 
     * error --> callback to SAT Source but not ACI_FREE of 
     * simShrdPrm.sim_dti_chPrm 
     */
    if( simShrdPrm.sim_dti_chPrm->dti_cb )
    {
       simShrdPrm.sim_dti_chPrm->dti_cb(sim_dti_cnf->dti_conn,
                                        sim_dti_cnf->link_id);
    }
  }
  
  PFREE(sim_dti_cnf);
}

#endif /* DTI */
#endif /* #ifdef FF_SAT_E */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_eventlist_cnf   |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_EVENTLIST_CNF primitive sent by SIM.
            This is the confirmation of the changed
      data available event status(send in SIM_EVENTLIST_REQ)
*/

GLOBAL void psa_sim_eventlist_cnf ( T_SIM_EVENTLIST_CNF *sim_eventlist_cnf )
{

  TRACE_FUNCTION ("psa_sim_eventlist_cnf()");

  switch(sim_eventlist_cnf->event_data_avail)
  {
    case SIM_EVENT_DISABLE:
    case SIM_EVENT_ENABLE:
      break;
    default:
      TRACE_EVENT("psa_sim_eventlist_cnf:not valid event_data_avail");
  }
 /*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_eventlist_cnf);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_dti_ind         |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_DTI_IND primitive sent by SIM
            indicating closing of BIP channel and dti connection
      (because the release timer is expired)
*/
#ifdef FF_SAT_E
#ifdef DTI
GLOBAL void psa_sim_dti_bip_ind ( T_SIM_DTI_BIP_IND *sim_dti_bip_ind )
{

  TRACE_FUNCTION ("psa_sim_dti_bip_ind()");

/* inform SAT Source about bip_ch_id and dti_conn*/

  if((sim_dti_bip_ind->dti_conn NEQ  SIM_DTI_DISCONNECT) OR 
     (sim_dti_bip_ind->dti_conn NEQ SIM_BIP_CLOSE_CHANNEL)) 
  {
    TRACE_FUNCTION ("psa_sim_dti_bip_ind: bip/dti parameter combination received!");
  }

  cmhSAT_OpChnSIMFail( sim_dti_bip_ind->dti_conn, 
                       sim_dti_bip_ind->bip_conn, 
                       sim_dti_bip_ind->bip_ch_id );

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */
  PFREE (sim_dti_bip_ind);

}
#endif /* DTI */
#endif /* #ifdef FF_SAT_E */

/*
+------------------------------------------------------------------------------
|  Function    : psaSIM_Insert_Continued
+------------------------------------------------------------------------------
|  Description : For loading phone book 
|
|  Parameters  :sim_mmi_insert_ind - Primitive  
|
|  Return      :     Void  
|
+------------------------------------------------------------------------------
*/


void psaSIM_Insert_Continued(T_SIM_MMI_INSERT_IND *sim_mmi_insert_ind )
{
#ifdef TI_PS_FFS_PHB
  /* Inform the phonebook module about all SIM parameters except ECC */
  pb_inserted_sim (MAX_SRV_TBL,
                   sim_mmi_insert_ind->sim_serv,
                   &sim_mmi_insert_ind->imsi_field,
                   sim_mmi_insert_ind->func,
                   sim_mmi_insert_ind->phase);
#else
  pb_build_req(sim_mmi_insert_ind);
#endif
  cmhSIM_SIMInserted();
}

/* Implements Measure # 48 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMP                |
|                                 ROUTINE : psa_sim_update_cnf      |
+-------------------------------------------------------------------+

  PURPOSE : processes the SIM_READ_CNF primitive send by SIM.
            this is the confirmation to the SIM update data operation.
            or SIM update absolute record operation.
*/

LOCAL void psaSIM_process_sim_upd_rec_cnf ( U8 req_id, U16 cause, UBYTE type )
{
  SHORT aId;              /* holds access id */

  TRACE_FUNCTION ("psaSIM_process_sim_upd_rec_cnf()");

  /*
   *-----------------------------------------------------------------
   * find entry in access parameter table
   *-----------------------------------------------------------------
   */
  aId = req_id; 

 if( simShrdPrm.atb[aId].ntryUsdFlg AND 
      simShrdPrm.atb[aId].accType EQ type )
  {
    /*
     *---------------------------------------------------------------
     * update access parameter and notify caller
     *---------------------------------------------------------------
     */
    simShrdPrm.atb[aId].errCode = cause;

    if( simShrdPrm.atb[aId].rplyCB )
      simShrdPrm.atb[aId].rplyCB( aId );
    else
      simShrdPrm.atb[aId].ntryUsdFlg = FALSE;
  }

}

/* Implements Measure # 179 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PSA_SIMP                  |
|                               ROUTINE : psaSIM_update_simShrdPrm  |
+-------------------------------------------------------------------+

  PURPOSE : The function updates simShrdPrm when SIM_VERIFY_PIN_CNF 
            or SIM_CHANGE_PIN_CNF is received.
*/

LOCAL void psaSIM_update_simShrdPrm ( U16 cause, U8 pin_id)
{
  TRACE_FUNCTION ("psaSIM_update_simShrdPrm()");

  switch( cause )
  {
    case( SIM_NO_ERROR ):
      simShrdPrm.PINStat = PS_RDY;
      if( pin_id EQ PHASE_2_PIN_1 )
      {
        simShrdPrm.pn1Stat = PS_RDY;
      }
      else if( pin_id EQ PHASE_2_PIN_2 )
      {
        simShrdPrm.pn2Stat = PS_RDY;
      }
      break;

    case( SIM_CAUSE_PIN1_EXPECT ):
      simShrdPrm.PINStat = simShrdPrm.pn1Stat = PS_PIN1;
      break;

    case( SIM_CAUSE_PIN2_EXPECT ):
      simShrdPrm.PINStat = simShrdPrm.pn2Stat = PS_PIN2;
      break;

    case( SIM_CAUSE_PUK1_EXPECT ):
    case( SIM_CAUSE_PIN1_BLOCKED):
      simShrdPrm.PINStat = PS_PUK1;
      break;

    case( SIM_CAUSE_PUK2_EXPECT ):
    case( SIM_CAUSE_PIN2_BLOCKED):
      simShrdPrm.PINStat = PS_PUK2;
      break;
    default:
      break;
  }
}


/*==== EOF =========================================================*/


