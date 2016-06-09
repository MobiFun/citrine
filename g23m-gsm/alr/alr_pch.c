/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  ALR_PCH
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
|  Purpose :  This Modul defines the SDL process PCH_Control.
+-----------------------------------------------------------------------------
*/

#ifndef ALR_PCH_C
#define ALR_PCH_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_PL

/*==== INCLUDES ===================================================*/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_alr.h"
#include "mon_alr.h"
#include "pei.h"
#include "tok.h"
#include "pcm.h"
#ifdef GPRS
#include "alr_gprs.h"
#endif

#include "alr.h"
#include "alr_em.h"

/*==== EXPORT =====================================================*/
typedef enum
{
  IMSI_TYPE_1=1,
  IMSI_TYPE_2
}T_IMSI_TYPE;
typedef enum
{
  TMSI_TYPE_1=4,  
  TMSI_TYPE_2=8
}T_TMSI_TYPE;
/*==== PRIVAT =====================================================*/
LOCAL BOOL pch_frm_chan_imsi    (UBYTE         *frame, 
                                 UBYTE          channel_needed, 
                                 T_IMSI_TYPE    imsi_type,
                                 UBYTE          index,
                                 UBYTE         *frame_start);
LOCAL BOOL pch_tmsi_type        (T_TMSI_TYPE    tmsi_type,
                                 UBYTE         *frame,
                                 UBYTE          channel_needed);
/*==== VARIABLES ==================================================*/
UBYTE page_mode_before_hplmn_search = PGM_NORMAL;
/*==== CONSTANTS ==================================================*/
#define IMSI_ODD_FLAG     8
#define IMSI_EVEN_FLAG    0
#define IDENT_TYPE_MON    0
#define IDENT_TYPE_IMSI   1
#define IDENT_TYPE_IMEI   2
#define IDENT_TYPE_IMEISV 3
#define IDENT_TYPE_TMSI   4
#define END_MARK          0xF0

/*==== FUNCTIONS ==================================================*/

#define TRACING

#if defined (TRACING)
#define ALR_TRACE_PCH(a)  ALR_TRACE(a)
#else
#define ALR_TRACE_PCH(a)
#endif

#ifdef TRACING
#define ALR_TRACE_PCH_CONFIG(b,a,c,t,p,m)  \
          TRACE_EVENT_P6 ("MFRMS: %d AG_RES: %d COMB: %d GRP: %d PGRP: %d PI: %d",b,a,c,t,p,m)
#define ALR_TRACE_PCH_PGM(p,x) \
          TRACE_EVENT_P2 ("new_pgm: %d cur_pgm: %d",p,x)
#define ALR_TRACE_PCH_IMSI() \
        { for (i=0; i<alr_data->pch_data.imsi[0]+1;i++) { \
          TRACE_EVENT_P2 ("imsi[%d]=%x",i,alr_data->pch_data.imsi[i]);} }
#else
#define ALR_TRACE_PCH_CONFIG(b,a,c,t,p,m)
#define ALR_TRACE_PCH_PGM(p,x)
#define ALR_TRACE_PCH_IMSI()
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_init                   |
+--------------------------------------------------------------------+

  PURPOSE : Initialize PCH Control Process.

*/
GLOBAL void pch_init (void)
{
  GET_INSTANCE_DATA;
  alr_data->pch_data.saved_page_mode = PGM_REORG;
  alr_data->pch_data.reorg_bcch_reading = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_start                  |
+--------------------------------------------------------------------+

  PURPOSE : Process signal pch_start from SDL process
            Main_Control.

*/
static const UBYTE PAG_BLOCK_TABLE [2][8] =
{
  /* not combined ccch */
  9,8,7,6,5,4,3,2,
  /* combined ccch     */
  3,2,1,1,1,1,1,1
};

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_configure              |
+--------------------------------------------------------------------+

  PURPOSE : Configutes L1 for paging.

*/
GLOBAL void pch_configure (T_MPH_IDLE_REQ *idle, UBYTE page_mode)
{
  GET_INSTANCE_DATA;
  UBYTE pag_blocks_per_mfr;

  if(idle NEQ NULL)
  {

    alr_data->pch_data.dlt                      = idle->dlt;
    alr_data->pch_data.act_dlt                  = idle->dlt;

    ALR_EM_SET_EM_ACT_DLT;

    pag_blocks_per_mfr                          = PAG_BLOCK_TABLE [idle->comb_ccch][idle->bs_ag_blocks_res];
    /*
     * pl_idle.bs_pa_mfrms has a range from 2-9.
     * MPH_IDLE_REQ codes them from 0-7
     */
    alr_data->pch_data.pl_idle.bs_pa_mfrms      = (UBYTE)(idle->bs_pa_mfrms + 2);
    alr_data->pch_data.pl_idle.bs_ag_blks_res   = idle->bs_ag_blocks_res;
    alr_data->pch_data.pl_idle.bcch_combined    = idle->comb_ccch;
    alr_data->pch_data.pl_idle.ccch_group       = (UBYTE)(idle->tn / 2);
    alr_data->pch_data.pl_idle.page_group       = idle->pg;
    alr_data->pch_data.pl_idle.page_block_index = (UBYTE)(idle->pg % pag_blocks_per_mfr);

    ALR_TRACE_PCH_CONFIG(idle->bs_pa_mfrms+2, idle->bs_ag_blocks_res,
                         idle->comb_ccch, idle->tn/2, idle->pg,
                         idle->pg % pag_blocks_per_mfr);
  }

  /*
   * During cell reselection reading of PCH is started hard coded with
   * page mode PGM_REORG because of the lack of parameters to calculate
   * the right paging group. Detection of SI3 during cell reselection
   * triggers the function pch_config_resel() to reconfigure PCH reading,
   * detection of a changed page mode during cell reselection is handled
   * by function pch_check_page_mode_cr() which needs to know whether SI3
   * is read
   */
  if (GET_STATE (STATE_MA) EQ MA_CELL_RESELECTION)
    alr_data->pch_data.si3_read = FALSE;


  /* Ensure that L1 does not get initialised with PAGING EXTENDED */
  switch( page_mode )
  {
    case SAVED_PGM:
      if( alr_data->pch_data.saved_page_mode EQ PGM_EXTENDED )
      {
        alr_data->pch_data.saved_page_mode = PGM_REORG;
      }
      page_mode = alr_data->pch_data.saved_page_mode;
      /*lint -fallthrough*/
    default:
      alr_data->pch_data.pl_idle.page_mode = page_mode;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_save_pgm               |
+--------------------------------------------------------------------+
  PURPOSE : Configure Paging
*/
GLOBAL void pch_save_pgm(UBYTE mode)
{
  GET_INSTANCE_DATA;
  if(mode)
    alr_data->pch_data.saved_page_mode = mode;
  else
    alr_data->pch_data.saved_page_mode = alr_data->pch_data.pl_idle.page_mode;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_start_ccch_req         |
+--------------------------------------------------------------------+

  PURPOSE : Configure Paging

*/

GLOBAL void pch_start_ccch_req (void)
{
  GET_INSTANCE_DATA;
  PALLOC(pl_idle, MPHC_START_CCCH_REQ);
  memset(pl_idle, 0, sizeof(T_MPHC_START_CCCH_REQ));

  ALR_EM_PAGE_MODE_CHANGE;

  switch (alr_data->pch_data.pl_idle.page_mode)
  {
    case PGM_REORG:
      ALR_TRACE_PCH ("config REORG");
      /* dummy values */
      pl_idle->bs_pa_mfrms      = 2;
      pl_idle->bs_ag_blks_res   = 7;
      pl_idle->bcch_combined    = 0;
      pl_idle->ccch_group       = 0;
      pl_idle->page_group       = 0;
      pl_idle->page_block_index = 0;
      pl_idle->page_mode        = PGM_REORG;
      break;
    case PGM_REORG_CS:
      ALR_TRACE_PCH ("config REORG_CS");

      memcpy (pl_idle, &alr_data->pch_data.pl_idle,
              sizeof (T_MPHC_START_CCCH_REQ));
      /*
       * if the page_mode is PGM_REORG_CS then
       * we have to change this to PGM_REORG
       * before we send it to L1
       */
      pl_idle->page_mode        = PGM_REORG;
     break;
    case PGM_EXTENDED:
      ALR_TRACE_PCH ("config EXT");

      memcpy (pl_idle, &alr_data->pch_data.pl_idle,
              sizeof (T_MPHC_START_CCCH_REQ));
      pl_idle->page_mode        = PGM_EXTENDED;
      break;
   case PGM_REORG_NC_SYNC:
     /*this case is the same as the default - except for the TRACE*/
      ALR_TRACE_PCH ("config PGM_REORG_NC_SYNC (NORMAL)");

      memcpy (pl_idle, &alr_data->pch_data.pl_idle,
              sizeof (T_MPHC_START_CCCH_REQ));
      pl_idle->page_mode        = PGM_NORMAL;
     break;
   default:
      ALR_TRACE_PCH ("config NORMAL");

      memcpy (pl_idle, &alr_data->pch_data.pl_idle,
              sizeof (T_MPHC_START_CCCH_REQ));
      pl_idle->page_mode        = PGM_NORMAL;
     break;
  }

  SET_STATE(STATE_PCH,PCH_ACTIVE);
  ma_pch_start_ccch_req(pl_idle);

  TRACE_EVENT_P1("reorg_bcch_reading = %d",alr_data->pch_data.reorg_bcch_reading);
  if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG OR
     alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_CS OR
     alr_data->pch_data.reorg_bcch_reading EQ TRUE)
  { /*
     * if we're going into reorg paging we also need to read the BCCH
     * to read possibly changed channel configuration
     * XXX but not at PGM_REORG_NC_SYNC (NORMAL)
     */
    ma_scell_full_nbcch();
#if defined (REL99) && defined (TI_PS_FF_EMR)
    if (alr_data->nc_data.si2quater_status EQ SI2QUATER_CONFIGURE )
    {
      if(alr_data->ma_data.sys_info_2quater[1] EQ D_SYS_INFO_2QUATER)
      {
        ma_send_stored_SI2qtr(&alr_data->ma_data.sys_info_2quater[0]);
        memset (alr_data->ma_data.sys_info_2quater,    0, 22);
      }
      alr_data->nc_data.si2quater_status = SI2QUATER_ACQ_FULL_BCCH;
    }
#endif
  }
  else
  { /*
     * otherwise we stop the reading of the BCCH and start the periodic read.
     * XXX but not if we have a PBCCH
     */
#ifdef GPRS
    if(alr_data->gprs_data.pbcch EQ FALSE)
#endif
#if defined (REL99) && defined (TI_PS_FF_EMR)
    {      
      TRACE_EVENT_P1("SI2qtr status: %d", alr_data->nc_data.si2quater_status);
      if ( (alr_data->nc_data.si2quater_status EQ SI2QUATER_ABSENT) OR
           (alr_data->nc_data.si2quater_status EQ SI2QUATER_ACQ_WRONG_BAIND) OR   
           (alr_data->nc_data.si2quater_status EQ SI2QUATER_ACQ_COMP) )                       
           sc_start_periodic();
      else if ((alr_data->nc_data.si2quater_status EQ SI2QUATER_CONFIGURE ) OR
        (alr_data->nc_data.si2quater_status EQ SI2QUATER_ACQ_FULL_BCCH))
      {
        if(alr_data->ma_data.sys_info_2quater[1] EQ D_SYS_INFO_2QUATER)
        {
          ma_send_stored_SI2qtr(&alr_data->ma_data.sys_info_2quater[0]);
          memset (alr_data->ma_data.sys_info_2quater,    0, 22);
        }
        if (alr_data->nc_data.si2quater_pos EQ SI2QUATER_ON_NBCCH)
          ma_scell_mon_si2quater_nbcch();      
        else
          ma_scell_mon_si2quater_ebcch();
        alr_data->nc_data.si2quater_status = SI2QUATER_ACQ_PENDING;
      }        
    } 
#else
      sc_start_periodic();
#endif
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_identity_req           |
+--------------------------------------------------------------------+

  PURPOSE : Get new mobile identity information from RR. Build IMSI
            pattern and store TMSI.

*/
GLOBAL void pch_identity_req (T_MPH_IDENTITY_REQ *mph_identity_req)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  /* the IMSI in the identity request is coded as a 15byte long array
     and is stored for later usage in pch_data as the message representation
     of the IMSI according to GSM4.08 10.5.1.4 */

  if (mph_identity_req->mid.len_imsi EQ 0)
  {
    /*
     * limited service, no paging
     */
    memset (alr_data->pch_data.imsi, 0, IMSI_LEN);
    alr_data->pch_data.v_tmsi   = FALSE;
    alr_data->pch_data.tmsi     = 0L;
  }
  else
  {
    /*
     * The IMSI is available
     */
    /* store length */
    alr_data->pch_data.imsi[0] = (UBYTE)((mph_identity_req->mid.len_imsi + 2) / 2);

    if (mph_identity_req->mid.len_imsi & 1)
    {
      ALR_TRACE_PCH ("IMSI is odd");
      /*
       * odd number of digits
       * first digit + odd flag + identity type IMSI
       */
      alr_data->pch_data.imsi[1] = (UBYTE)((mph_identity_req->mid.imsi[0] << 4) +
                                            IMSI_ODD_FLAG +
                                            IDENT_TYPE_IMSI);
    }
    else
    {
      ALR_TRACE_PCH ("IMSI is even");
      /*
       * even number of digits
       * first digit + even flag + identity type IMSI
       */
      alr_data->pch_data.imsi[1] = (UBYTE)((mph_identity_req->mid.imsi[0] << 4) +
                                            IMSI_EVEN_FLAG +
                                            IDENT_TYPE_IMSI);
    }
    /*
     * fill in the rest of digits
     */
    for (i=1;i<mph_identity_req->mid.len_imsi;i++)
    {
      if (i & 1)
        alr_data->pch_data.imsi[(i/2)+2] = (UBYTE)(END_MARK + mph_identity_req->mid.imsi[i]);
      else
      {
        alr_data->pch_data.imsi[(i/2)+1] &= ~END_MARK;  /* remove end mark */
        alr_data->pch_data.imsi[(i/2)+1] = (UBYTE)(alr_data->pch_data.imsi[(i/2)+1] +
                                                   (mph_identity_req->mid.imsi[i] << 4));
      }
    }
    alr_data->pch_data.imsi_mod_1000 = (SHORT)
      ( (mph_identity_req->mid.imsi[mph_identity_req->mid.len_imsi-1] +
         mph_identity_req->mid.imsi[mph_identity_req->mid.len_imsi-2] * 10 +
         mph_identity_req->mid.imsi[mph_identity_req->mid.len_imsi-3] * 100 ) % 1000);

    ALR_TRACE_PCH_IMSI();

    /*
     * copy TMSI
     */
    alr_data->pch_data.v_tmsi = mph_identity_req->mid.v_tmsi;
    alr_data->pch_data.tmsi   = mph_identity_req->mid.tmsi;
#ifdef GPRS
    gprs_alr_store_ptmsi(mph_identity_req->mid.v_ptmsi,
                         mph_identity_req->mid.ptmsi);
    gprs_alr_store_ptmsi2(mph_identity_req->mid.v_ptmsi2,
                          mph_identity_req->mid.ptmsi2);
#endif
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_increment_dlt          |
+--------------------------------------------------------------------+

  PURPOSE : Incrementation of downlink timeout counter after receiving
            a valid PCH block.

*/
GLOBAL void pch_increment_dlt (void)
{
  GET_INSTANCE_DATA;
  if (alr_data->pch_data.act_dlt <
      alr_data->pch_data.dlt)
  {
#if !defined NTRACE
    trc_mon_counter_idle (alr_data->pch_data.act_dlt,
                          alr_data->pch_data.dlt);
#endif /* (!defined NTRACE) */
    alr_data->pch_data.act_dlt++;

    ALR_EM_SET_EM_ACT_DLT;

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_decrement_dlt          |
+--------------------------------------------------------------------+

  PURPOSE : Decrementation of downlink timeout counter after receiving
            an invalid PCH block.

*/
GLOBAL void pch_decrement_dlt (void)
{
  GET_INSTANCE_DATA;
  if (alr_data->pch_data.act_dlt > 4)
  {
#if !defined NTRACE
    trc_mon_counter_idle (alr_data->pch_data.act_dlt,
                          alr_data->pch_data.dlt);
#endif /* (!defined NTRACE) */

    alr_data->pch_data.act_dlt -= 4;

    ALR_EM_SET_EM_ACT_DLT;

  }
  else
  {
    alr_data->pch_data.act_dlt = alr_data->pch_data.dlt;
#if !defined NTRACE
    trc_mon_counter_idle (alr_data->pch_data.act_dlt,
                          alr_data->pch_data.dlt);
#endif

    ALR_EM_SET_EM_ACT_DLT;

    ALR_TRACE_PCH ("downlink fail");

    ma_error_ind (CS_DOWN_LINK_FAIL, alr_data->serving_cell);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_check_page_mode        |
+--------------------------------------------------------------------+

  PURPOSE : Check the page mode of an incoming unacknowledged
            message.
*/
static const UBYTE PAGE_MODE_CHANGE [3][4] =
       {
         /* old mode = paging normal         */
         NONE,            SWAP_TO_EXTEND, SWAP_TO_REORG, NONE,
         /* old mode = extended paging       */
         SWAP_TO_NORMAL,  NONE,           SWAP_TO_REORG, NONE,
         /* old mode = paging reorganisation */
         SWAP_TO_NORMAL,  SWAP_TO_EXTEND, NONE,          NONE
       };


GLOBAL void pch_check_page_mode (T_MPHC_DATA_IND  *data_ind)
{
  GET_INSTANCE_DATA;
  UBYTE page_mode;
  UBYTE cur_page_mode = alr_data->pch_data.pl_idle.page_mode;

  page_mode = (UBYTE)(data_ind->l2_frame.content [3] & 3);

  ALR_TRACE_PCH_PGM(page_mode, alr_data->pch_data.pl_idle.page_mode);

  /*if HPLMN search is going on and we should be in REORG, we are really
  in REORG_NC_SYNC (NORMAL). Otherwise we cannot synchronize to the NC's*/
  if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_NC_SYNC)
    cur_page_mode = PGM_REORG;

  if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_CS)
    cur_page_mode = PGM_REORG;

  switch (PAGE_MODE_CHANGE [cur_page_mode][page_mode])
  {
    case SWAP_TO_NORMAL:
      ALR_TRACE_PCH ("SWAP_TO_NORMAL");
      alr_data->pch_data.pl_idle.page_mode = PGM_NORMAL;
      pch_start_ccch_req ();
      break;
    case SWAP_TO_EXTEND:
      ALR_TRACE_PCH ("SWAP_TO_EXT");
      alr_data->pch_data.pl_idle.page_mode = PGM_EXTENDED;
      pch_start_ccch_req ();
      break;
    case SWAP_TO_REORG:
      ALR_TRACE_PCH ("SWAP_TO_REORG");
      alr_data->pch_data.reorg_bcch_reading = TRUE;
      alr_data->pch_data.si_bitmap = 0;
      alr_data->pch_data.pl_idle.page_mode = PGM_REORG_CS;
      pch_start_ccch_req ();
      break;
    default:
      break;
  }
}

GLOBAL void pch_check_page_mode_cr (T_MPHC_DATA_IND  *data_ind)
{
  GET_INSTANCE_DATA;
  UBYTE page_mode;
  UBYTE cur_page_mode = alr_data->pch_data.saved_page_mode;
  UBYTE swap = TRUE;

  page_mode = (UBYTE)(data_ind->l2_frame.content [3] & 3);
  if(alr_data->pch_data.saved_page_mode EQ PGM_REORG_CS)
    cur_page_mode = PGM_REORG;

  switch (PAGE_MODE_CHANGE [cur_page_mode][page_mode])
  {
    case SWAP_TO_NORMAL:
      ALR_TRACE_PCH ("CR:SWAP_TO_NORMAL");
      alr_data->pch_data.saved_page_mode = PGM_NORMAL;
      break;

    case SWAP_TO_EXTEND:
      ALR_TRACE_PCH ("CR:SWAP_TO_EXT");
      alr_data->pch_data.saved_page_mode = PGM_EXTENDED;
      break;

    case SWAP_TO_REORG:
      ALR_TRACE_PCH ("CR:SWAP_TO_REORG");
      alr_data->pch_data.saved_page_mode = PGM_REORG_CS;
      break;

    default:
      swap = FALSE;
      break;
  }

  if (swap AND alr_data->pch_data.si3_read AND
      alr_data->pch_data.last_start_ccch_req.bs_pa_mfrms NEQ NOT_PRESENT_8BIT)
  {
    PALLOC(pl_idle, MPHC_START_CCCH_REQ);
    memcpy ( pl_idle,
             &(alr_data->pch_data.last_start_ccch_req),
             sizeof(T_MPHC_START_CCCH_REQ));
    pl_idle->page_mode = alr_data->pch_data.saved_page_mode;
    ma_pch_start_ccch_req (pl_idle);
  }

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_check_pag_1            |
+--------------------------------------------------------------------+

  PURPOSE : The function checks a paging request type 1 message.

*/
GLOBAL void pch_check_pag_1 (T_MPHC_DATA_IND *data_ind)
{
  GET_INSTANCE_DATA;
  UBYTE *frame  = data_ind->l2_frame.content;
  UBYTE  channel_needed;

  UBYTE *frame_start  = data_ind->l2_frame.content;

  /*ALR_TRACE_PCH ("p1");*/
  /*
   * Check only if IMSI available (len NEQ 0),
   * ti and pd = 0x06 and
   * l2 pseudolength is greater than 5 bytes
   */
  if (alr_data->pch_data.imsi[0] AND
      frame[1] EQ 0x06 AND
      frame[0] > 0x15)
  {
    /*
     * store channel needed type
     */
    channel_needed = frame[3];
    /*
     * check type of identity for mobile identity 1
     */
    if(pch_frm_chan_imsi( frame, channel_needed, IMSI_TYPE_1, 5,frame_start))
      return;
    /*
     * check type of identity for mobile identity 2
     * set frame pointer to start of mobile identity 2 (id tag)
     * old frame pointer (frame) +
     * offset pseudo length etc (4) +
     * length mobile identity 1 (frame [4]) +
     * plus 1 byte for length field
     */
    frame = frame + 4 + frame[4] + 1;
    if (frame[0] EQ 0x17)
    {
      /*
       * mobile identity 2 is available
       */
      if(pch_frm_chan_imsi( frame, channel_needed, IMSI_TYPE_2, 2, frame_start))
        return;
    }
  }
  //ALR_TRACE_PCH("end p1");
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_check_pag_2            |
+--------------------------------------------------------------------+

  PURPOSE : The function checks a paging request type 2 message.

*/

GLOBAL void pch_check_pag_2 (T_MPHC_DATA_IND *data_ind)
{
  GET_INSTANCE_DATA;
  UBYTE *frame  = data_ind->l2_frame.content;
  ULONG  tmsi;
  UBYTE  i;
  UBYTE  channel_needed;

  /*
   * Check only if IMSI available (len NEQ 0) and
   * ti and pd = 0x06 and
   * l2 pseudolength is greater than 5 bytes
   */
  if (alr_data->pch_data.imsi[0] AND
      frame[1] EQ 0x06 AND
      frame[0] > 0x15)
  {
    /*
     * store channel needed type
     */
    channel_needed = frame[3];
    /*
     * check mobile identity 1 only if TMSI is available
     */
    if(pch_tmsi_type(TMSI_TYPE_1, frame, channel_needed))
      return;
#ifdef GPRS
    if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
    {
      tmsi = 0L;
      for (i=0;i<4;i++)
        tmsi += frame[i+4] << (8*(3-i));
      if(gprs_alr_check_ptmsi(tmsi)) return;
    }
#endif
    /*
     * check mobile identity 2 only if TMSI is available
     */
    if(pch_tmsi_type(TMSI_TYPE_2, frame, channel_needed))
      return;
#ifdef GPRS
    if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
    {
      tmsi = 0L;
      for (i=0;i<4;i++)
        tmsi += frame[i+8] << (8*(3-i));
      if(gprs_alr_check_ptmsi(tmsi)) return;
    }
#endif

    if (frame[12] EQ 0x17)
    {
      /*
       * mobile identity 3 is available
       * calculation of channel needed for
       * mobile identity 3 from the rest octet.
       */
      channel_needed = frame [ frame[13]+14 ];
      if (channel_needed & 0x80)
        channel_needed = (UBYTE)((channel_needed >> 5) & 3);
      else
        channel_needed = 0;

      switch (frame [14] & 7)
      {
        case 1:
          /*
           * IMSI
           */
          if (!memcmp (alr_data->pch_data.imsi, &frame[13],
                       alr_data->pch_data.imsi[0]+1))
          {
            /*
             * IMSI matches
             */
#ifdef GPRS
     if(! gprs_alr_check_packet_paging_2(frame,3))
#endif
       ma_pch_paging_ind (1, (UBYTE) channel_needed);
#ifdef GPRS
     else if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
       ma_pch_paging_ind (ID_IMSI, CN_PACKET);
#endif
            return;
          }
          break;
        case 4:
          /*
           * TMSI
           */
          if (alr_data->pch_data.v_tmsi)
          {
            tmsi = 0L;
            for (i=0;i<frame[13]-1;i++)
              tmsi += frame[i+15] << ((8*(frame[13]-2-i)));
            if (alr_data->pch_data.tmsi EQ tmsi)
            {
              /*
               * TMSI matches
               */
              ma_pch_paging_ind (4, channel_needed);
              return;
            }
#ifdef GPRS
            if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
            {
              tmsi = 0L;
              for (i=0;i<frame[13]-1;i++)
                tmsi += frame[i+15] << ((8*(frame[13]-2-i)));
              if(gprs_alr_check_ptmsi(tmsi)) return;
            }
#endif
          }
          break;
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_check_pag_3            |
+--------------------------------------------------------------------+

  PURPOSE : The function checks a paging request type 3 message.

*/

GLOBAL void pch_check_pag_3 (T_MPHC_DATA_IND *data_ind)
{
  GET_INSTANCE_DATA;
  UBYTE *frame  = data_ind->l2_frame.content;
  ULONG  tmsi;
  UBYTE  i;
  UBYTE  channel_needed;

  /*
   * Check only if IMSI available (len NEQ 0) and
   * ti and pd = 0x06 and
   * l2 pseudolength is greater than 5 bytes
   */
  if (alr_data->pch_data.imsi[0] AND
      frame[1] EQ 0x06 AND
      frame[0] > 0x15)
  {
    /*
     * store channel needed type
     */
    channel_needed = frame[3];
    /*
     * check mobile identity 1 only if TMSI is available
     */
    if(pch_tmsi_type(TMSI_TYPE_1, frame, channel_needed))
      return;
#ifdef GPRS
    if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
    {
      tmsi = 0L;
      for (i=0;i<4;i++)
        tmsi += frame[i+4] << (8*(3-i));
      if(gprs_alr_check_ptmsi(tmsi)) return;
    }
#endif

    /*
     * check mobile identity 2 only if TMSI is available
     */
    if(pch_tmsi_type(TMSI_TYPE_2, frame, channel_needed))
      return;
#ifdef GPRS
    if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
    {
      tmsi = 0L;
      for (i=0;i<4;i++)
        tmsi += frame[i+8] << (8*(3-i));
      if(gprs_alr_check_ptmsi(tmsi)) return;
    }
#endif
    /*
     * calculation of channel needed for
     * mobile identity 3 and 4 from the rest octet.
     */
    channel_needed = frame [ 20 ];
    if (channel_needed & 0x80)
      channel_needed = (UBYTE)((channel_needed & 0x78) >> 3);
    else
      channel_needed = 0;
    /*
     * check mobile identity 3 only if TMSI is available
     */
    if (alr_data->pch_data.v_tmsi)
    {
      tmsi = 0L;
      for (i=0;i<4;i++)
        tmsi += frame[i+12] << (8*(3-i));
      if (alr_data->pch_data.tmsi EQ tmsi)
      {
        /*
         * TMSI matches
         */
        ma_pch_paging_ind (4, (UBYTE)(channel_needed >> 2));
        return;
      }
#ifdef GPRS
    if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
    {
      tmsi = 0L;
      for (i=0;i<4;i++)
        tmsi += frame[i+12] << (8*(3-i));
      if(gprs_alr_check_ptmsi(tmsi)) return;
    }
#endif
    }
    /*
     * check mobile identity 4 only if TMSI is available
     */
    if (alr_data->pch_data.v_tmsi)
    {
      tmsi = 0L;
      for (i=0;i<4;i++)
        tmsi += frame[i+16] << (8*(3-i));
      if (alr_data->pch_data.tmsi EQ tmsi)
      {
        /*
         * TMSI matches
         */
        ma_pch_paging_ind (4, (UBYTE)(channel_needed & 3));
        return;
      }
    }
#ifdef GPRS
    if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
    {
      tmsi = 0L;
      for (i=0;i<4;i++)
        tmsi += frame[i+16] << (8*(3-i));
      if(gprs_alr_check_ptmsi(tmsi)) return;
    }
#endif
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_stop                   |
+--------------------------------------------------------------------+

  PURPOSE : The function stops paging.

*/
GLOBAL void pch_stop(void)
{
  GET_INSTANCE_DATA;
  if(GET_STATE(STATE_PCH) EQ PCH_ACTIVE)
  {
    SET_STATE(STATE_PCH,PCH_NULL);
    if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG OR
       alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_CS)
    {
      ALR_TRACE_PCH ("stop sc for pch");
      ma_stop_scell_bcch_req();
    }

    ma_pch_stop();
  }
}

/*used for S13 - to stop BCCH reading*/

GLOBAL UBYTE pch_mode_reorg(void)
{
  GET_INSTANCE_DATA;
  if(alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG OR
     alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_CS OR
     alr_data->pch_data.pl_idle.page_mode EQ PGM_REORG_NC_SYNC)
    return TRUE;
  else
    return FALSE;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_no_of_paging_blocks    |
+--------------------------------------------------------------------+

  PURPOSE : The function configures the paging reading during cell
            reselection when reveived a SI3
*/

LOCAL SHORT pch_no_of_paging_blocks (UBYTE ccch_conf,
                                     UBYTE bs_ag_blks_res,
                                     UBYTE bs_pa_mfrms)
{
  /* in according to GSM 4.08 section 10.5.2.11, table 10.5.33 */
  if (ccch_conf EQ COMB_CCCH_COMB)
  {
    /*
     * combined CCCH,
     *
     * number of paging blocks = (3 - BS_AG_BLKS_RES) * BS_PA_MFRMS
     *
     * Maximum function only for security reasons, BCCH coding range is 0..7,
     * but allowed is only 0..2.
     */
    return (( (1 >  (UBYTE)(3 - bs_ag_blks_res) ? 1 : (UBYTE)(3 - bs_ag_blks_res)) ) *
            ((UBYTE)(2 + bs_pa_mfrms)));
  }
  else
  {
    /*
     * non-combined CCCH,
     *
     * number of paging blocks = (9 - BS_AG_BLKS_RES) * BS_PA_MFRMS
     */
    return ((9 - bs_ag_blks_res) *
            (2 + bs_pa_mfrms));
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_config_resel           |
+--------------------------------------------------------------------+

  PURPOSE : The function configures the L1 PCH reading during cell
            reselection when reveived an SI3
*/

GLOBAL void pch_config_resel (T_MPHC_DATA_IND *data_ind)
{
  GET_INSTANCE_DATA;
  UBYTE *p_ctrl = &(data_ind->l2_frame.content[SI_CONTENTS_MSG_T+8]);
  UBYTE ccch_conf,
        bs_ag_blks_res,
        bs_pa_mfrms,
        pg,
        pag_blocks_per_mfr;
  SHORT n, b;
  PALLOC(pl_idle, MPHC_START_CCCH_REQ);
  memset(pl_idle, 0, sizeof(T_MPHC_START_CCCH_REQ));

  bs_ag_blks_res = (UBYTE)(((*p_ctrl) & 0x38) >> 3);
  ccch_conf      = (((*p_ctrl) & 0x07) EQ 0x01) ? COMB_CCCH_COMB : COMB_CCCH_NOT_COMB;
  p_ctrl++;
  bs_pa_mfrms    = (UBYTE)(((*p_ctrl) & 0x07));
/*  TRACE_EVENT_P5("pch_config_resel IE: %02x %02x bs_ag_blks_res=%u ccch_conf=%u bs_pa_mfrms=%u",
                  data_ind->l2_frame.content[SI_CONTENTS_MSG_T+8],
                  *p_ctrl,
                  bs_ag_blks_res,
                  ccch_conf,
                  bs_pa_mfrms);*/

  n = pch_no_of_paging_blocks (ccch_conf, bs_ag_blks_res, bs_pa_mfrms);
  b = ((ccch_conf / 2) + 1) * n;
  pg = (UBYTE) ((alr_data->pch_data.imsi_mod_1000 % b) % n);
  pag_blocks_per_mfr = PAG_BLOCK_TABLE [ccch_conf][bs_ag_blks_res];

  pl_idle->bs_ag_blks_res   = bs_ag_blks_res;
  pl_idle->bs_pa_mfrms      = (UBYTE)(bs_pa_mfrms + 2);
  pl_idle->bcch_combined    = ccch_conf;
  pl_idle->ccch_group       = (UBYTE) ((alr_data->pch_data.imsi_mod_1000 % b) / n);
  pl_idle->page_group       = pg;
  pl_idle->page_block_index = (UBYTE)(pg % pag_blocks_per_mfr);
  pl_idle->page_mode        = (alr_data->pch_data.saved_page_mode EQ PGM_REORG_CS)
                              ? PGM_REORG
                              : alr_data->pch_data.saved_page_mode;

/*  TRACE_EVENT_P3("n=%u b=%u page_mode=%u", n, b, pl_idle->page_mode);*/
  ma_pch_start_ccch_req(pl_idle);

  alr_data->pch_data.si3_read = TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_frm_chan_imsi          |
+--------------------------------------------------------------------+

  PURPOSE : Function replacing the common switch-code in the function 
            "pch_check_pag_1". This function checks for the IMSI types.

*/
LOCAL BOOL pch_frm_chan_imsi (UBYTE          *frame,
                              UBYTE           channel_needed,
                              T_IMSI_TYPE     imsi_type,
                              UBYTE           index,
                              UBYTE          *frame_start)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  ULONG tmsi;  
  ALR_TRACE_PCH ("pch_frm_chan_imsi");
  switch (frame[index] & 7)
  {
    case 1:
      /*
       * IMSI
       */
    if (!memcmp (alr_data->pch_data.imsi, &frame[index-1],
                 alr_data->pch_data.imsi[0]+1))
    {
      /*ALR_TRACE_PCH ("p1 IMSI match");*/
      /*
       * IMSI matches
       */
      if(imsi_type EQ IMSI_TYPE_1)
      {
#ifdef GPRS
        if(! gprs_alr_check_packet_paging(frame_start,1))
#endif
        ma_pch_paging_ind (1, (UBYTE)((channel_needed & 0x30)>>4));
#ifdef GPRS
          else if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
            ma_pch_paging_ind (ID_IMSI, CN_PACKET);
#endif
      return TRUE;
      }
      else if(imsi_type EQ IMSI_TYPE_2)
      {
#ifdef GPRS
        if(! gprs_alr_check_packet_paging(frame_start,2))
#endif
        ma_pch_paging_ind (1, (UBYTE)((channel_needed & 0xC0)>>6));
#ifdef GPRS
      else if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
        ma_pch_paging_ind (ID_IMSI, CN_PACKET);
#endif
      return TRUE;
      }
     }
    break; /* for case-1 */

    case 4:
      /*ALR_TRACE_PCH ("p1 TMSI");*/
      /*
       * TMSI
       */
      if (alr_data->pch_data.v_tmsi)
      {
        tmsi = 0L;
        for (i=0; i<frame[index-1]-1; i++)
           tmsi += frame[i+index+1] << ((8*(frame[index-1]-2-i)));
        if (alr_data->pch_data.tmsi EQ tmsi)
        {
           /*ALR_TRACE_PCH ("p1 TMSI match");*/
           /*
            * TMSI matches
            */
           if(imsi_type EQ IMSI_TYPE_1)
              ma_pch_paging_ind (4, (UBYTE)((channel_needed & 0x30)>>4));
           else if(imsi_type EQ IMSI_TYPE_2)
             ma_pch_paging_ind (4, (UBYTE)((channel_needed & 0xC0)>>6));
           return TRUE;
        }
       }
#ifdef GPRS
        if(GET_STATE(STATE_MA) NEQ MA_CON_EST)
        {
          tmsi = 0L;
          for (i=0; i<frame[index-1]-1; i++)
            tmsi += frame[i+index+1] << ((8*(frame[index-1]-2-i)));
          if(gprs_alr_check_ptmsi(tmsi))
            return TRUE;
        }
#endif
    break;/* for case-4 */
  } /* end of switch-case */
  return FALSE;
}
/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : ALR_PCH                    |
| STATE   : code                ROUTINE : pch_tmsi_type              |
+--------------------------------------------------------------------+

  PURPOSE : Function replacing the common code from the function 
            "pch_check_pag_2"
*/

LOCAL BOOL pch_tmsi_type (T_TMSI_TYPE     tmsi_type,
                          UBYTE          *frame,
                          UBYTE           channel_needed)
{
  GET_INSTANCE_DATA;
  UBYTE  i;
  ULONG  tmsi;
  ALR_TRACE_PCH ("pch_tmsi_type");
  if (alr_data->pch_data.v_tmsi)
  {
    tmsi = 0L;
    if (tmsi_type EQ TMSI_TYPE_1)
      for (i=0; i<4; i++)
        tmsi += frame[i+4] << (8*(3-i));
    else if (tmsi_type EQ TMSI_TYPE_2)
      for (i=0; i<4; i++)
        tmsi += frame[i+8] << (8*(3-i));
    if (alr_data->pch_data.tmsi EQ tmsi)
    {
      /*
       * TMSI matches
       */
      if (tmsi_type EQ TMSI_TYPE_1)
        ma_pch_paging_ind (4, (UBYTE)((channel_needed & 0x30)>>4));
      else if (tmsi_type EQ TMSI_TYPE_2)
        ma_pch_paging_ind (4, (UBYTE)((channel_needed & 0xC0)>>6));
      return TRUE;
    }
  }
  return FALSE;
}
#endif
