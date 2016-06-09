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

#ifndef RR_EM_C
#define RR_EM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>     /* offsetof */
#include <stdio.h>      /* sprintf */
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_rr.h"
#include "tok.h"
#include "rr.h"
#include "rr_em.h"

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

#ifdef FF_EM_MODE

/*
  These variables are used between entities. Even this is not a clean solution it is a straigth forward
  way to reduce the overhead to a minimum. A clean solution would be based on an only usage of primitives
  which would stress the os with no aditional advantage!!
*/
/* these are accessed by ACI */
GLOBAL UBYTE    em_rr_sem_buffer [EM_RR_SEM_SIZE]; /*lint -esym(552,em_rr_sem_buffer) -esym(765,em_rr_sem_buffer ) */
GLOBAL UBYTE    em_rr_sem_index;                  /*lint -esym(765,em_rr_sem_index)*/
GLOBAL UBYTE    em_act_dlt = 0; /*lint -esym(765,em_act_dlt) | used by ALR */
GLOBAL UBYTE    em_act_rlt = 0; /*lint -esym(765,em_act_rlt) | used by ALR */

static USHORT ma [MAX_MA_CHANNELS];     /* MA list after starting time */
static USHORT ma2[MAX_MA_CHANNELS];     /* MA list before starting time if available */
static UBYTE  v_start=0;                  /* starting time valid */
static UBYTE  maio2=0;

static T_HANDLE  sem_EM_RR;
static UBYTE em_rr_trace_occured;


/* Event tracing flags for EM */
GLOBAL BOOL rr_v[EM_MAX_RR_EVENTS];
GLOBAL USHORT em_assign_fail_rr_cause;
GLOBAL USHORT em_handover_fail_rr_cause;

LOCAL void em_rr_sem_clear (void);
LOCAL void rr_em_first_event_check(void);

#endif /* FF_EM_MODE */

/*==== FUNCTIONS ==================================================*/
#ifdef FF_EM_MODE


/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_get_hchn
+------------------------------------------------------------------------------
|  Description  :  This function stores the hopping channels after change occured.
|
|  Parameters   :  channel_array  - channel mode 1
|                  channel2_array - channel mode 2
|                  start          - valid flag for channel mode2
|                  maio_2         - MAIO for before time configuration
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void dat_em_get_hchn (USHORT* channel_array, USHORT* channel2_array, UBYTE start,UBYTE maio_2)
{
 TRACE_FUNCTION ("dat_em_get_hchn()");

 memset(ma, 0, 65);
 memset(ma2, 0, 65);
 memcpy(ma, channel_array, 65);
 v_start = start;
 if (v_start) {
  maio2 = maio_2;
  memcpy(ma2, channel2_array, 65); }
}
/*
+------------------------------------------------------------------------------
|  Function     :  em_init_get_hchn
+------------------------------------------------------------------------------
|  Description  :  initiates the hopping list. This is necessary, because a request
|                  of the hopping list immideatly after switch on causes a reset.
|
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL void em_init_get_hchn ()
{
 TRACE_FUNCTION ("em_init_get_hchn()");

 ma[0]  = NOT_PRESENT_16BIT;
 ma2[0] = NOT_PRESENT_16BIT;
}

/*
+------------------------------------------------------------------------------
|  Function     :  em_get_first_codec
+------------------------------------------------------------------------------
|  Description  :  Returns the highest bit rate AMR codec mode
|  
|  Parameters   :  acs - Active codec set
|                  
|  Return       :  void
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE em_get_first_codec(UBYTE acs)
{
  UBYTE i;
 
  /* Find Highest bit rate codec mode from ACS */
  for(i=0;i<8;i++)
  {
    if((0x80>>i) & acs )
      break;
  }

  /* First codec( Highest bit rate codec mode) */
  return (EM_AMR_MODE_12_2-i);
}

/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_sc_info_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_SC_INFO_REQ.
|
|  Parameters   :  Primitive from EM - T_EM_SC_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_sc_info_req (T_EM_SC_INFO_REQ *em_sc_info_req)
{
  GET_INSTANCE_DATA;
  PALLOC(em_sc_info_cnf, EM_SC_INFO_CNF);
  memset (em_sc_info_cnf, 0, sizeof (T_EM_SC_INFO_CNF));
 
  PFREE(em_sc_info_req);
  TRACE_FUNCTION ("dat_em_sc_info_req()");

  em_sc_info_cnf->vocoder = EM_VOC_NA;

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_IDLE:
      if (rr_data->nc_data[SC_INDEX].bcch_status EQ DECODED)
      {
        em_sc_info_cnf->arfcn   = rr_data->nc_data[SC_INDEX].arfcn;
        em_sc_info_cnf->c1      = rr_data->nc_data[SC_INDEX].c1;
        em_sc_info_cnf->c2      = rr_data->nc_data[SC_INDEX].c2;
        em_sc_info_cnf->rxlev   = rr_data->nc_data[SC_INDEX].rxlev;
        em_sc_info_cnf->bsic    = rr_data->nc_data[SC_INDEX].bsic;
        em_sc_info_cnf->dsc     = em_act_dlt;
        em_sc_info_cnf->txlev   = rr_data->sc_data.cd.cell_options.pow_ctrl;
        em_sc_info_cnf->tn      = rr_data->sc_data.chan_desc.tn;
        em_sc_info_cnf->lac     = rr_data->nc_data[SC_INDEX].lai.lac;
        em_sc_info_cnf->cba     = rr_data->nc_data[SC_INDEX].rach.cell_bar_access;
        em_sc_info_cnf->cbq     = rr_data->nc_data[SC_INDEX].c2_par.cbq;
        em_sc_info_cnf->cell_id = rr_data->nc_data[SC_INDEX].cell_id;
        em_sc_info_cnf->cell_type_ind = EM_CELL_GSM;
#ifdef GPRS
        if(rr_data->nc_data[SC_INDEX].rac NEQ NOT_PRESENT_8BIT)
          em_sc_info_cnf->cell_type_ind = EM_CELL_GPRS;
#endif
      }
      else if (rr_data->nc_data[SC_INDEX].bcch_status EQ NON_DECODED)
      {
        em_sc_info_cnf->arfcn   = rr_data->nc_data[SC_INDEX].arfcn;
        em_sc_info_cnf->rxlev   = rr_data->nc_data[SC_INDEX].rxlev;
        em_sc_info_cnf->bsic    = rr_data->nc_data[SC_INDEX].bsic;
      }
      break;

    case ATT_DEDICATED:
      if (!rr_data->ms_data.measurement_report.valid)
        break;

      em_sc_info_cnf->arfcn    = rr_data->ms_data.measurement_report.arfcn;
      em_sc_info_cnf->bsic     = rr_data->nc_data[SC_INDEX].bsic;
      em_sc_info_cnf->txlev    = rr_data->sc_data.cd.cell_options.pow_ctrl;
      em_sc_info_cnf->tn       = rr_data->sc_data.chan_desc.tn;
      em_sc_info_cnf->lac      = rr_data->nc_data[SC_INDEX].lai.lac;
      em_sc_info_cnf->cell_id  = rr_data->nc_data[SC_INDEX].cell_id;
      em_sc_info_cnf->rxlev_f  = rr_data->ms_data.measurement_report.rx_lev_full;
      em_sc_info_cnf->rxlev_s  = rr_data->ms_data.measurement_report.rx_lev_sub;
      em_sc_info_cnf->rlt      = em_act_rlt;
      em_sc_info_cnf->tav      = rr_data->sc_data.new_ta;
      em_sc_info_cnf->rxqual_f = rr_data->ms_data.measurement_report.rx_qual_full;
      em_sc_info_cnf->rxqual_s = rr_data->ms_data.measurement_report.rx_qual_sub;
      em_sc_info_cnf->vocoder  = rr_data->sc_data.ch_mode;
      if (rr_data->nc_data[SC_INDEX].bcch_status EQ DECODED)
      {
        em_sc_info_cnf->cell_type_ind = EM_CELL_GSM;
#ifdef GPRS
        if(rr_data->nc_data[SC_INDEX].rac NEQ NOT_PRESENT_8BIT)
          em_sc_info_cnf->cell_type_ind = EM_CELL_GPRS;
#endif
      }
      break;

    default:
      break;
  }/*switch*/
  PSENDX(MMI, em_sc_info_cnf);
} /*dat_em_sc_info_req*/

/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_nc_info_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_NC_INFO_REQ.
|
|  Parameters   :  Primitive from EM - T_EM_NC_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_nc_info_req (T_EM_NC_INFO_REQ *em_nc_info_req)
{
  GET_INSTANCE_DATA;
  UBYTE index = 0;
  UBYTE index2 = 0;
  PALLOC(em_nc_info_cnf, EM_NC_INFO_CNF);
  memset (em_nc_info_cnf, 0, sizeof (T_EM_NC_INFO_CNF));
  memset (em_nc_info_cnf->rac, 0xff, EM_MAX_NUM_NC);

  PFREE(em_nc_info_req);
  TRACE_FUNCTION ("dat_em_nc_info_req()");

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_IDLE:
      for (index = 0 , index2 = 0 ; index < EM_MAX_NUM_NC ; index++)
      {
        if (rr_data->nc_data[index].bcch_status EQ DECODED)
        {
          em_nc_info_cnf->arfcn_nc[index2]    = rr_data->nc_data[index].arfcn;
          em_nc_info_cnf->c1_nc[index2]       = rr_data->nc_data[index].c1;
          em_nc_info_cnf->c2_nc[index2]       = rr_data->nc_data[index].c2;
          em_nc_info_cnf->rxlev_nc[index2]    = rr_data->nc_data[index].rxlev;
          em_nc_info_cnf->bsic_nc[index2]     = rr_data->nc_data[index].bsic;
          em_nc_info_cnf->cell_id_nc[index2]  = rr_data->nc_data[index].cell_id;
          em_nc_info_cnf->lac_nc[index2]      = rr_data->nc_data[index].lai.lac;
          em_nc_info_cnf->frame_offset[index2]      = rr_data->ms_data.measurement_report.ncells.frame_offset[index];
          em_nc_info_cnf->time_alignmt[index2]      = rr_data->ms_data.measurement_report.ncells.time_alignmt[index];
          em_nc_info_cnf->cba_nc[index2]      = rr_data->nc_data[index].rach.cell_bar_access;
          em_nc_info_cnf->cbq_nc[index2]      = rr_data->nc_data[index].c2_par.cbq;
          em_nc_info_cnf->cell_type_ind[index2]     = EM_CELL_GSM;
#ifdef GPRS
          em_nc_info_cnf->rac[index2]               = rr_data->nc_data[index].rac;

          /* rac in nc_data is set only when v_gprs_ind is present in SI3/SI4 rest
             octets. Since v_gprs_ind is not stored,rac is used to check gprs 
             support in the cell */
          if(em_nc_info_cnf->rac[index2] NEQ NOT_PRESENT_8BIT)
            em_nc_info_cnf->cell_type_ind[index2]   = EM_CELL_GPRS; 
#endif
          em_nc_info_cnf->cell_resel_offset[index2] = rr_data->nc_data[index].c2_par.cell_reselect_offset;
          em_nc_info_cnf->temp_offset[index2]       = rr_data->nc_data[index].c2_par.temp_offset;
          em_nc_info_cnf->rxlev_acc_min[index2]     = rr_data->nc_data[index].select_para.rxlev_access_min;
          index2++;
        } /* if decoded */

        else if (rr_data->nc_data[index].bcch_status EQ NON_DECODED)
        {
          em_nc_info_cnf->arfcn_nc[index2]   = rr_data->nc_data[index].arfcn;
          em_nc_info_cnf->rxlev_nc[index2]   = rr_data->nc_data[index].rxlev;
          em_nc_info_cnf->bsic_nc[index2]    = rr_data->nc_data[index].bsic;
          index2++;
        } /* if non decoded */
      } /* for */
      em_nc_info_cnf->no_ncells = rr_data->ms_data.measurement_report.ncells.no_of_ncells;
      break;

    case ATT_DEDICATED:
      {
        if (!rr_data->ms_data.measurement_report.valid)
          break;

        em_nc_info_cnf->no_ncells = rr_data->ms_data.measurement_report.ncells.no_of_ncells;

        for (index = 0; index< rr_data->ms_data.measurement_report.ncells.no_of_ncells; index++)
        {
          /*in the measurement report the cells are ordered by fieldstrength*/
          em_nc_info_cnf->arfcn_nc[index]    = rr_data->ms_data.measurement_report.ncells.arfcn[index];
          em_nc_info_cnf->rxlev_nc[index]    = rr_data->ms_data.measurement_report.ncells.rx_lev[index];
          em_nc_info_cnf->bsic_nc[index]     = rr_data->ms_data.measurement_report.ncells.bsic[index];
          em_nc_info_cnf->frame_offset[index]= rr_data->ms_data.measurement_report.ncells.frame_offset[index];
          em_nc_info_cnf->time_alignmt[index]= rr_data->ms_data.measurement_report.ncells.time_alignmt[index];
        } /*for*/
      }/*case*/
      break;

    default:
      break;
  }/*switch*/
  PSENDX(MMI, em_nc_info_cnf);
}/*dat_em_nc_info_req*/

/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_loc_pag_info_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_LOC_PAG_INFO_REQ.
|
|  Parameters   :  Primitive from EM - T_EM_LOC_PAG_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_loc_pag_info_req (T_EM_LOC_PAG_INFO_REQ *em_loc_pag_info_req)
{
  GET_INSTANCE_DATA;
  PALLOC(em_loc_pag_info_cnf, EM_LOC_PAG_INFO_CNF);
  memset (em_loc_pag_info_cnf, 0, sizeof (T_EM_LOC_PAG_INFO_CNF));

  PFREE(em_loc_pag_info_req);
  TRACE_FUNCTION ("dat_em_loc_pag_info_req()");

  if (rr_data->nc_data[SC_INDEX].bcch_status EQ DECODED)
  {
    memcpy(em_loc_pag_info_cnf->mcc, rr_data->nc_data[SC_INDEX].lai.mcc, SIZE_MCC);

    /*check for MNC with 2 digits*/
    /* Fixed for Issue 21468 */
    if (rr_data->nc_data[SC_INDEX].lai.mnc[2] NEQ 0xF) /*defines a 3 digit NMC*/
    {
      memcpy(em_loc_pag_info_cnf->mnc, rr_data->nc_data[SC_INDEX].lai.mnc, SIZE_MNC);
    }
    else /*2 digit -> leave the first value empty*/
    {
      em_loc_pag_info_cnf->mnc[1] = rr_data->nc_data[SC_INDEX].lai.mnc[0];
      em_loc_pag_info_cnf->mnc[2] = rr_data->nc_data[SC_INDEX].lai.mnc[1];
    }

    em_loc_pag_info_cnf->bs_pa_mfrms  = rr_data->nc_data[SC_INDEX].control_descr.bs_pa_mfrms;
    em_loc_pag_info_cnf->t3212        = rr_data->nc_data[SC_INDEX].control_descr.t3212;
  }

  if (rr_data->ms_data.tmsi_available EQ TRUE)
    em_loc_pag_info_cnf->tmsi       = rr_data->ms_data.tmsi_binary;

  PSENDX(MMI, em_loc_pag_info_cnf);
}/*dat_em_loc_pag_info_req*/

/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_plmn_info_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_PLMN_INFO_REQ.
|
|  Parameters   :  Primitive from EM - T_EM_PLMN_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_plmn_info_req (T_EM_PLMN_INFO_REQ *em_plmn_info_req)
{
  GET_INSTANCE_DATA;
  PALLOC(em_plmn_info_cnf, EM_PLMN_INFO_CNF);
  memset (em_plmn_info_cnf, 0, sizeof (T_EM_PLMN_INFO_CNF));
  
  PFREE(em_plmn_info_req);
  TRACE_FUNCTION ("dat_em_plmn_info_req()");

  em_plmn_info_cnf->no_creq_max = rr_data->nc_data[SC_INDEX].rach.max_retrans;
  em_plmn_info_cnf->reest_flag  = rr_data->nc_data[SC_INDEX].rach.re;
  em_plmn_info_cnf->txpwr_max   = rr_data->nc_data[SC_INDEX].select_para.ms_txpwr_max_cch;
  em_plmn_info_cnf->rxlev_min   = rr_data->nc_data[SC_INDEX].select_para.rxlev_access_min;
  em_plmn_info_cnf->rel_cause   = 0xff; /* these info will be parsed by aci */

  PSENDX(MMI, em_plmn_info_cnf);
}/*dat_em_plmn_info_req*/
/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_cip_hop_dtx_info_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_CIP_HOP_DTX_INFO_REQ.
|
|  Parameters   :  Primitive from EM - T_EM_CIP_HOP_DTX_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_cip_hop_dtx_info_req (T_EM_CIP_HOP_DTX_INFO_REQ *em_cip_hop_dtx_info_req)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  PALLOC(em_cip_hop_dtx_info_cnf, EM_CIP_HOP_DTX_INFO_CNF);
  memset (em_cip_hop_dtx_info_cnf, 0, sizeof (T_EM_CIP_HOP_DTX_INFO_CNF));

  PFREE(em_cip_hop_dtx_info_req);
  TRACE_FUNCTION ("dat_em_cip_hop_dtx_info_req()");

  /* Intialize to 0xffff. No hopping channels */
  em_cip_hop_dtx_info_cnf->hop_chn.ma[0]  = NOT_PRESENT_16BIT;
  em_cip_hop_dtx_info_cnf->hop_chn2.ma[0] = NOT_PRESENT_16BIT;
  
  if ((GET_STATE(STATE_ATT) EQ ATT_DEDICATED) AND 
      (GET_STATE(STATE_DAT) EQ DAT_DEDICATED))
  {
    /* cipher status */
    em_cip_hop_dtx_info_cnf->ciph_stat = rr_data->sc_data.ciph_on;

    /* Hopping status */
    em_cip_hop_dtx_info_cnf->hop = rr_data->sc_data.chan_desc.hop;

    if (rr_data->sc_data.chan_desc.hop EQ H_NO)
    { /* Hopping is not configured */
      em_cip_hop_dtx_info_cnf->arfcn = rr_data->sc_data.chan_desc.arfcn;
    }
    else
    { /* hopping sequence no   */
      em_cip_hop_dtx_info_cnf->hsn    = rr_data->sc_data.chan_desc.hsn;
    }

    if(em_cip_hop_dtx_info_cnf->hop NEQ H_NO)
    {
      /* MAIO */
      em_cip_hop_dtx_info_cnf->hop_chn.maio = rr_data->sc_data.chan_desc.maio;

      i=0;
      while((i<MAX_MA_CHANNELS) AND (ma[i] NEQ NOT_PRESENT_16BIT))
      {
        em_cip_hop_dtx_info_cnf->hop_chn.ma[i] = ma[i];
        i++;
      }
    
      /* Number of hopping channels */
      em_cip_hop_dtx_info_cnf->hop_chn.nr_arfcns = i;
      if(i < MAX_MA_CHANNELS) 
      {
        em_cip_hop_dtx_info_cnf->hop_chn.ma[i] = NOT_PRESENT_16BIT;
      }

      /* starting time status */
      em_cip_hop_dtx_info_cnf->v_start = v_start;
  
      /* Hopping list after time */
      if (v_start)
      {
        /* MAIO */
        em_cip_hop_dtx_info_cnf->hop_chn2.maio = maio2;
      
        i=0;
        while((i<MAX_MA_CHANNELS) AND (ma2[i] NEQ NOT_PRESENT_16BIT))
        {
          em_cip_hop_dtx_info_cnf->hop_chn2.ma[i] = ma2[i];
          i++;
        }
        /* Number of hopping channels */
        em_cip_hop_dtx_info_cnf->hop_chn2.nr_arfcns = i;
        if(i < MAX_MA_CHANNELS)
        {
          em_cip_hop_dtx_info_cnf->hop_chn2.ma[i] = NOT_PRESENT_16BIT;
        }
      } /* v_start */
      
    } /* hop NEQ H_NO */
  
  } /* dedicated state */

  /* DTX status */
  em_cip_hop_dtx_info_cnf->dtx_stat  = rr_data->sc_data.cd.dtx; 

  PSENDX(MMI, em_cip_hop_dtx_info_cnf);
}/*dat_em_cip_hop_dtx_info_req*/

/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_mobdata_power_info_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_POWER_INFO_REQ.
|
|  Parameters   :  Primitive from EM - T_EM_POWER_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_mobdata_power_info_req (T_EM_POWER_INFO_REQ *em_power_info_req)
{
  GET_INSTANCE_DATA;
  PALLOC(em_power_info_cnf, EM_POWER_INFO_CNF);

  PFREE(em_power_info_req);

  TRACE_FUNCTION ("dat_em_mobdata_power_info_req()");

  memset (em_power_info_cnf, 0, sizeof (T_EM_POWER_INFO_CNF));
  memcpy(&em_power_info_cnf->classm2, &rr_data->ms_data.classmark2, sizeof (T_classm2));
  memcpy(&em_power_info_cnf->classm3, &rr_data->ms_data.classmark3, sizeof (T_classm3));

  PSENDX(MMI, em_power_info_cnf);
}/*dat_em_mobdata_power_info_req*/

/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_mobdata_id_info_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_IDENTITY_INFO_REQ.
|
|  Parameters   :  Primitive from EM - T_EM_IDENTITY_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_mobdata_id_info_req (T_EM_IDENTITY_INFO_REQ *em_identity_info_req)
{
  GET_INSTANCE_DATA;
  PALLOC(em_identity_info_cnf, EM_IDENTITY_INFO_CNF);
  memset (em_identity_info_cnf, 0, sizeof (T_EM_IDENTITY_INFO_CNF));

  PFREE(em_identity_info_req);
  TRACE_FUNCTION ("dat_em_mobdata_id_info_req()");

  /*
   *   The values for IMEI and IMEISV are identical up to the last 4bits. The 'ident_type' distinguish
   *   IMEI and IMEISV where ident_type = 3 is synonymously with IMEISV. In case only IMEI is needed,
   *   em_mm_mobdata_id_info_req->em_imei.ident_dig[16] should not considered.
   */

  em_identity_info_cnf->em_imeisv.ident_type   = rr_data->ms_data.imei.ident_type;
  em_identity_info_cnf->em_imeisv.v_ident_dig  = rr_data->ms_data.imei.v_ident_dig;
  em_identity_info_cnf->em_imeisv.c_ident_dig  = rr_data->ms_data.imei.c_ident_dig;
  em_identity_info_cnf->em_imeisv.odd_even     = rr_data->ms_data.imei.odd_even;
  memcpy (em_identity_info_cnf->em_imeisv.ident_dig, rr_data->ms_data.imei.ident_dig, 16);

  if (rr_data->ms_data.imsi_available) /* SIM available */
  {
    em_identity_info_cnf->em_imsi.ident_type    = rr_data->ms_data.imsi.ident_type;
    em_identity_info_cnf->em_imsi.v_ident_dig   = rr_data->ms_data.imsi.v_ident_dig;
    em_identity_info_cnf->em_imsi.c_ident_dig   = rr_data->ms_data.imsi.c_ident_dig;
    em_identity_info_cnf->em_imsi.odd_even      = rr_data->ms_data.imsi.odd_even;
    memcpy (em_identity_info_cnf->em_imsi.ident_dig, rr_data->ms_data.imsi.ident_dig, 16);
  }

  if (rr_data->ms_data.tmsi_available EQ TRUE)
    em_identity_info_cnf->tmsi     = rr_data->ms_data.tmsi_binary;

  PSENDX(MMI, em_identity_info_cnf);
} /*dat_em_mobdata_id_info_req*/

/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_mobdata_version_info_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_SW_VERSION_INFO_REQ.
|
|  Parameters   :  Primitive from EM - T_EM_SW_VERSION_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_mobdata_version_info_req (T_EM_SW_VERSION_INFO_REQ *em_sw_version_info_req)
{
  PALLOC(em_sw_version_info_cnf, EM_SW_VERSION_INFO_CNF);
  PFREE(em_sw_version_info_req);

  TRACE_FUNCTION ("dat_em_mobdata_version_info_req()");

  memset (em_sw_version_info_cnf, 0, sizeof (T_EM_SW_VERSION_INFO_CNF));

  em_sw_version_info_cnf->v_mmi  = 0xFF;
  em_sw_version_info_cnf->v_sim  = 0xFF;
  em_sw_version_info_cnf->v_cc   = 0xFF;
  em_sw_version_info_cnf->v_ss   = 0xFF;
  em_sw_version_info_cnf->v_sms  = 0xFF;
  em_sw_version_info_cnf->v_mm   = 0xFF;
  em_sw_version_info_cnf->v_rr   = 0xFF;
  em_sw_version_info_cnf->v_dl   = 0xFF;
  em_sw_version_info_cnf->v_l1   = 0xFF;

  PSENDX(MMI, em_sw_version_info_cnf);
}/*dat_em_mobdata_version_info_req*/


/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_amr_info_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_AMR_INFO_REQ.
|
|  Parameters   :  Primitive from EM - T_EM_AMR_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_amr_info_req (T_EM_AMR_INFO_REQ *em_amr_info_req)
{
  GET_INSTANCE_DATA;
  UBYTE i=0;
  PALLOC(em_amr_info_cnf, EM_AMR_INFO_CNF);
  memset (em_amr_info_cnf, 0, sizeof (T_EM_AMR_INFO_CNF));
 
  PFREE(em_amr_info_req);
  TRACE_FUNCTION ("dat_em_amr_info_req()");

  /* AMR information is valid only in dedicated state, when chan mode is AMR */
  if ((GET_STATE(STATE_ATT) EQ ATT_DEDICATED) AND 
      (GET_STATE(STATE_DAT) EQ DAT_DEDICATED))
  {
    if(rr_data->sc_data.ch_mode EQ CM_AMR)
    {
      /* AMR vocoder type */
      em_amr_info_cnf->amr_vocoder = rr_data->sc_data.chan_desc.chan_type;

      /* Initial codec mode indicator */
      em_amr_info_cnf->amr_icmi    = rr_data->sc_data.amr_conf.icmi;

      /* Initial codec mode. Valid if icmi = 1(Start mode signalled) */
      em_amr_info_cnf->amr_icm     = rr_data->sc_data.amr_conf.st_mode;

      /* Active codec set */
      em_amr_info_cnf->amr_acs     = rr_data->sc_data.amr_conf.set_amr;

      /* First codec( Highest bit rate codec mode) */
      em_amr_info_cnf->amr_first_codec = 
                   em_get_first_codec(rr_data->sc_data.amr_conf.set_amr);

      if(rr_data->sc_data.amr_conf.v_cod_prop)
      {
        /* Number of codec modes */
        em_amr_info_cnf->amr_nr_modes = rr_data->sc_data.amr_conf.c_cod_prop + 1;

        /* Threshold and Hysteresis properties between codec modes */
        for(i=0;i<rr_data->sc_data.amr_conf.c_cod_prop;i++)
        {
          memcpy(&em_amr_info_cnf->amr_cod_prop[i],&rr_data->sc_data.amr_conf.cod_prop[i],
                 sizeof(T_amr_cod_prop));
        }
      }  
      else
      {
        /* Number of codec modes */
        em_amr_info_cnf->amr_nr_modes = 1;
      }
    } /* CM_AMR */
  } /* ATT DEDICATED */

  PSENDX(MMI, em_amr_info_cnf);
} /*dat_em_amr_info_req*/

/*
+------------------------------------------------------------------------------
|  Function     : em_init_rr_event_trace
+------------------------------------------------------------------------------
|  Description  :  Initialize the event tracing flags for RR
|
|  Parameters   :  void
|
|  Return       :  void
|
+------------------------------------------------------------------------------
*/
GLOBAL void em_init_rr_event_trace(void)
{
 UBYTE i;

 TRACE_FUNCTION ("em_init_rr_event_trace()");

 for (i=0; i< EM_MAX_RR_EVENTS; i++)
   rr_v[i] = 0;
}

/*
+------------------------------------------------------------------------------
|  Function     : rr_em_rr_event_req
+------------------------------------------------------------------------------
|  Description  :  Set the event tracing flags according the bitmask
|
|  Parameters   :  Primitive - Bitmask(T_EM_RR_EVENT_REQ)
|
|  Return       :  void
|
+------------------------------------------------------------------------------
*/
GLOBAL void rr_em_rr_event_req (T_EM_RR_EVENT_REQ *em_rr_event_req)
{
 UBYTE i;

 TRACE_FUNCTION ("rr_em_rr_event_req()");

 /*
  *  The event tracing flags are set according the bitmask. rr_v[i] are
  *  the flags belonging to the event number described in 8443.601
 */
 for(i=1; i<33; i++)
   rr_v[i] = ((em_rr_event_req->bitmask_rr_l & (0x01<<(i-1))) > 0) ? TRUE : FALSE;

 for(i=33; i<(EM_MAX_RR_EVENTS); i++)
   rr_v[i] = ((em_rr_event_req->bitmask_rr_h & (0x01<<(i-1))) > 0) ? TRUE : FALSE;

 /*
   A new event trace is generated therefor the flag is set to 0.
 */
 em_rr_trace_occured = 0;

 PFREE(em_rr_event_req);
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_2
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define EVT_LEN_OFFSET     2  /* Event Number and Length value bytes */
#define BUFFER_2_LENGTH   2
GLOBAL UBYTE em_write_buffer_2 (UBYTE event_no)
{
 UBYTE em_rr_event_buffer[BUFFER_2_LENGTH];
 UBYTE em_rr_buffer_write = 0;

 TRACE_FUNCTION ("rr_em_write_buffer_2()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 em_rr_event_buffer[em_rr_buffer_write++] = event_no;                      /* Event number */
 em_rr_event_buffer[em_rr_buffer_write]   = 0x00; /* Value length - 0 equals no data */

 return (em_rr_sem (BUFFER_2_LENGTH, em_rr_event_buffer));              /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_3
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define BUFFER_3_LENGTH 3
GLOBAL UBYTE em_write_buffer_3  (UBYTE event_no, UBYTE value)
{
 UBYTE em_rr_event_buffer[BUFFER_3_LENGTH];
 UBYTE em_rr_buffer_write = 0;

 TRACE_FUNCTION ("rr_em_write_buffer_3()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 em_rr_event_buffer[em_rr_buffer_write++] = event_no;                /* Event number */
 em_rr_event_buffer[em_rr_buffer_write++] = BUFFER_3_LENGTH-EVT_LEN_OFFSET;  /* Value length - 0 equals no value */
 em_rr_event_buffer[em_rr_buffer_write]   = value;                   /* Data to be stored */

 return (em_rr_sem (BUFFER_3_LENGTH, em_rr_event_buffer));        /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_3a
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value (USHORT)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define BUFFER_3A_LENGTH 4
GLOBAL UBYTE em_write_buffer_3a (UBYTE event_no, USHORT value)
{
 UBYTE em_rr_event_buffer[BUFFER_3A_LENGTH];
 UBYTE em_rr_buffer_write = 0;

 TRACE_FUNCTION ("rr_em_write_buffer_3a()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 em_rr_event_buffer[em_rr_buffer_write++] = event_no;               /* Event number */
 em_rr_event_buffer[em_rr_buffer_write++] = BUFFER_3A_LENGTH-EVT_LEN_OFFSET; /* Value length - 0 equals no value */
 em_rr_event_buffer[em_rr_buffer_write++] = (UBYTE)(value >> 8);    /* Data to be stored - MSB first */
 em_rr_event_buffer[em_rr_buffer_write++] = (UBYTE)(value );        /* LSB second */

 return (em_rr_sem (BUFFER_3A_LENGTH, em_rr_event_buffer));      /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, length of data and value1 and value2
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define BUFFER_4_LENGTH 4
GLOBAL UBYTE em_write_buffer_4  (UBYTE event_no, UBYTE value1, UBYTE value2)
{
 UBYTE em_rr_event_buffer[BUFFER_4_LENGTH];
 UBYTE em_rr_buffer_write = 0;

 TRACE_FUNCTION ("rr_em_write_buffer_4()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 em_rr_event_buffer[em_rr_buffer_write++] = event_no;               /* Event number */
 em_rr_event_buffer[em_rr_buffer_write++] = BUFFER_4_LENGTH-EVT_LEN_OFFSET;  /* Value length - 0 equals no value */
 em_rr_event_buffer[em_rr_buffer_write++] = value1;                 /* Value 1 */
 em_rr_event_buffer[em_rr_buffer_write++] = value2;                 /* Value 2 */

 return (em_rr_sem (BUFFER_4_LENGTH, em_rr_event_buffer));       /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4a
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value, data cs (USHORT)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define BUFFER_4A_LENGTH 5
GLOBAL UBYTE em_write_buffer_4a (UBYTE event_no, UBYTE value, USHORT cs)
{
 UBYTE em_rr_event_buffer[BUFFER_4A_LENGTH];
 UBYTE em_rr_buffer_write = 0;

 TRACE_FUNCTION ("rr_em_write_buffer_4a()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 em_rr_event_buffer[em_rr_buffer_write++] = event_no;               /* Event number */
 em_rr_event_buffer[em_rr_buffer_write++] = BUFFER_4A_LENGTH-EVT_LEN_OFFSET;  /* Value length - 0 equals no value */
 em_rr_event_buffer[em_rr_buffer_write++] = value;                  /* first data info */
 em_rr_event_buffer[em_rr_buffer_write++] = (UBYTE)(cs >> 8);       /* Data to be stored - MSB first */
 em_rr_event_buffer[em_rr_buffer_write++] = (UBYTE)(cs);            /* LSB second */

 return (em_rr_sem (BUFFER_4A_LENGTH, em_rr_event_buffer));         /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4b
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value, data plmn (T_plmn)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define BUFFER_4B_LENGTH  9
GLOBAL UBYTE em_write_buffer_4b (UBYTE event_no, UBYTE value, T_plmn plmn)
{
 UBYTE em_rr_event_buffer[BUFFER_4B_LENGTH];
 UBYTE em_rr_buffer_write = 0;

 TRACE_FUNCTION ("rr_em_write_buffer_4b()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 memset(em_rr_event_buffer, 0, BUFFER_4B_LENGTH);
 em_rr_event_buffer[em_rr_buffer_write++] = event_no;                  /* Event number */
 em_rr_event_buffer[em_rr_buffer_write++] = BUFFER_4B_LENGTH-EVT_LEN_OFFSET;  /* Value length - 0 equals no value */
 em_rr_event_buffer[em_rr_buffer_write++] = value;
 if(plmn.v_plmn)
 {
  memcpy(&em_rr_event_buffer[em_rr_buffer_write], plmn.mcc, SIZE_MCC);
  em_rr_buffer_write += SIZE_MCC;
  memcpy(&em_rr_event_buffer[em_rr_buffer_write], plmn.mnc, SIZE_MNC);
 }

 return (em_rr_sem (BUFFER_4B_LENGTH, em_rr_event_buffer));        /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_4c
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value1, data plmn (T_plmn)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define BUFFER_4C_LENGTH ((MAX_PLMN*(SIZE_MCC+SIZE_MNC))+EVT_LEN_OFFSET)
GLOBAL UBYTE em_write_buffer_4c (UBYTE event_no, UBYTE value1, T_plmn plmn[MAX_PLMN])
{
 UBYTE em_rr_event_buffer[BUFFER_4C_LENGTH];
 UBYTE em_rr_buffer_write = 0;
 UBYTE length = 0;
 UBYTE i;

 TRACE_FUNCTION ("rr_em_write_buffer_4c()");
 /* Ensure value1 is within specified boundaries */
 if( value1 > MAX_PLMN )
 {
   value1 = MAX_PLMN;
 }

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 memset(em_rr_event_buffer, 0, BUFFER_4C_LENGTH);
 em_rr_event_buffer[em_rr_buffer_write++] = event_no;                     /* Event number */
 em_rr_event_buffer[em_rr_buffer_write++] = 1+value1*(SIZE_MCC + SIZE_MNC); /* Value length - 0 equals no value */
 for (i=0; i<value1; i++) {
   if(plmn[i].v_plmn)
   {
     memcpy(em_rr_event_buffer + em_rr_buffer_write, plmn[i].mcc, SIZE_MCC);
     em_rr_buffer_write += SIZE_MCC;
     memcpy(em_rr_event_buffer + em_rr_buffer_write, plmn[i].mnc, SIZE_MNC);
     em_rr_buffer_write += SIZE_MNC;
   }
 }
 length = (EVT_LEN_OFFSET + value1*(SIZE_MCC + SIZE_MNC));

 return (em_rr_sem (length, em_rr_event_buffer));                          /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_5
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value1, data value2, data cs (USHORT)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define BUFFER_5_LENGTH  5
GLOBAL UBYTE em_write_buffer_5 (UBYTE event_no, UBYTE value1, UBYTE value2, UBYTE value3)
{
 UBYTE em_rr_event_buffer[BUFFER_5_LENGTH];
 UBYTE em_rr_buffer_write = 0;

 TRACE_FUNCTION ("rr_em_write_buffer_5()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 em_rr_event_buffer[em_rr_buffer_write++] = event_no;               /* Event number */
 em_rr_event_buffer[em_rr_buffer_write++] = BUFFER_5_LENGTH-EVT_LEN_OFFSET; /* Value length - 0 equals no value */
 em_rr_event_buffer[em_rr_buffer_write++] = value1;                 /* first data info */
 em_rr_event_buffer[em_rr_buffer_write++] = value2;                 /* second data info */
 em_rr_event_buffer[em_rr_buffer_write++] = value3;                 /* Data */

 return (em_rr_sem (BUFFER_5_LENGTH, em_rr_event_buffer));       /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_5c
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value, data plmn (T_plmn)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define BUFFER_5C_LENGTH  10
GLOBAL UBYTE em_write_buffer_5c (UBYTE event_no, UBYTE value1, UBYTE value2, T_plmn plmn)
{
 UBYTE em_rr_event_buffer[BUFFER_5C_LENGTH];
 UBYTE em_rr_buffer_write = 0;

 TRACE_FUNCTION ("rr_em_write_buffer_5c()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 memset(em_rr_event_buffer, 0, BUFFER_5C_LENGTH);
 em_rr_event_buffer[em_rr_buffer_write++] = event_no;                 /* Event number */
 em_rr_event_buffer[em_rr_buffer_write++] = BUFFER_5C_LENGTH-EVT_LEN_OFFSET;  /* Value length - 0 equals no value */
 em_rr_event_buffer[em_rr_buffer_write++] = value1;
 em_rr_event_buffer[em_rr_buffer_write++] = value2;
 if(plmn.v_plmn)
 {
  memcpy(em_rr_event_buffer + em_rr_buffer_write, plmn.mcc, SIZE_MCC);
  em_rr_buffer_write += SIZE_MCC;
  memcpy(em_rr_event_buffer + em_rr_buffer_write, plmn.mnc, SIZE_MNC);
 }

 return (em_rr_sem (BUFFER_5C_LENGTH, em_rr_event_buffer));        /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_8
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value1, data value2, data cs (USHORT)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define BUFFER_8_LENGTH  8
GLOBAL UBYTE em_write_buffer_8 (UBYTE event_no, UBYTE value1, UBYTE value2, UBYTE value3, UBYTE value4,
                            UBYTE value5, UBYTE value6)
{
 UBYTE em_rr_event_buffer[BUFFER_8_LENGTH];
 UBYTE em_rr_buffer_write = 0;

 TRACE_FUNCTION ("rr_em_write_buffer_8()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 em_rr_event_buffer[em_rr_buffer_write++] = event_no;               /* Event number */
 em_rr_event_buffer[em_rr_buffer_write++] = BUFFER_8_LENGTH-EVT_LEN_OFFSET; /* Value length - 0 equals no value */
 em_rr_event_buffer[em_rr_buffer_write++] = value1;                 /* first data info */
 em_rr_event_buffer[em_rr_buffer_write++] = value2;                 /* second data info */
 em_rr_event_buffer[em_rr_buffer_write++] = value3;                 /* data */
 em_rr_event_buffer[em_rr_buffer_write++] = value4;                 /* data */
 em_rr_event_buffer[em_rr_buffer_write++] = value5;                 /* data */
 em_rr_event_buffer[em_rr_buffer_write++] = value6;                 /* data */

 return (em_rr_sem (BUFFER_8_LENGTH, em_rr_event_buffer));       /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : em_write_buffer_9
+------------------------------------------------------------------------------
|  Description  :  Perform buffer check and store corresponding data in it.
|
|  Parameters   :  Event number, data value1, data value2, data cs (USHORT)
|
|  Return       :  TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
#define BUFFER_9_LENGTH  9
GLOBAL UBYTE em_write_buffer_9 (UBYTE event_no, UBYTE value1, UBYTE value2, UBYTE value3,
                           UBYTE value4,   UBYTE value5, UBYTE value6, UBYTE value7)
{
 UBYTE em_rr_event_buffer[BUFFER_9_LENGTH];
 UBYTE em_rr_buffer_write = 0;

 TRACE_FUNCTION ("rr_em_write_buffer_9()");

 /*
   ACI is informed about the first event trace, used for later data processing.
 */
 rr_em_first_event_check();

 em_rr_event_buffer[em_rr_buffer_write++] = event_no;               /* Event number */
 em_rr_event_buffer[em_rr_buffer_write++] = BUFFER_9_LENGTH-EVT_LEN_OFFSET; /* Value length - 0 equals no value */
 em_rr_event_buffer[em_rr_buffer_write++] = value1;                 /* first data info */
 em_rr_event_buffer[em_rr_buffer_write++] = value2;                 /* second data info */
 em_rr_event_buffer[em_rr_buffer_write++] = value3;                 /* data */
 em_rr_event_buffer[em_rr_buffer_write++] = value4;                 /* data */
 em_rr_event_buffer[em_rr_buffer_write++] = value5;                 /* data */
 em_rr_event_buffer[em_rr_buffer_write++] = value6;                 /* data */
 em_rr_event_buffer[em_rr_buffer_write++] = value7;                 /* data */

 return (em_rr_sem (BUFFER_9_LENGTH, em_rr_event_buffer));       /* Data is stored inside buffer, reset flag */
}

/*
+------------------------------------------------------------------------------
|  Function     : rr_semaphore_err
+------------------------------------------------------------------------------
|  Description  :  Semaphor error
|
|
|  Parameters   :  void
|
|  Return       :  void
|
+------------------------------------------------------------------------------
*/
static void rr_semaphore_err (void)
{
  static UCHAR out = 0;

  if (!out)
  {
    out = 1;
/* Implements Measure#32: Row 224 */
    TRACE_EVENT ("semaphore error");
  }
}


/*
+------------------------------------------------------------------------------
|  Function     : rr_enter_critical_section
+------------------------------------------------------------------------------
|  Description  :
|                 Attempt to access critical section by taking control
|                 of the semaphore
|
|  Parameters   : void
|
|  Return       : int
|
+------------------------------------------------------------------------------
*/
static int rr_enter_critical_section (T_HANDLE sem)
{
  if (vsi_s_get (VSI_CALLER sem) NEQ VSI_OK)
  {
    SYST_TRACE ( "** Enter Critical Sec:semaphore NOT cleared **");
    rr_semaphore_err();
    return -1;
  }
  else
  {
    return 0;
  }
}

/*
+------------------------------------------------------------------------------
|  Function     : rr_leave_critical_section
+------------------------------------------------------------------------------
|  Description  :
|                 Relinquish control of semaphore and release access to
|                 critical section
|
|  Parameters   : void
|
|  Return       : int
|
+------------------------------------------------------------------------------
*/
static int rr_leave_critical_section (T_HANDLE sem)
{
  if (vsi_s_release (VSI_CALLER sem) NEQ VSI_OK)
  {
    SYST_TRACE ( "** Leave Critical Sec:semaphore NOT cleared **");
    rr_semaphore_err();
    return -1;
  }
  else
  {
    return 0;
  }
}

/*
+------------------------------------------------------------------------------
|  Function     : em_rr_sem_init
+------------------------------------------------------------------------------
|  Description  :
|                 Initialise Semaphore
|
|  Parameters   : void
|
|  Return       : int
|
+------------------------------------------------------------------------------
*/
GLOBAL void em_rr_sem_init (void)
{
  sem_EM_RR  = vsi_s_open (VSI_CALLER "EM_RR_SEM",1);

  if (sem_EM_RR NEQ VSI_ERROR)
    em_rr_sem_clear ();
  else
    SYST_TRACE ("RR:can't open semaphore \"EM_RR_SEM\"");
}

/*
+------------------------------------------------------------------------------
|  Function     : em_rr_sem_exit
+------------------------------------------------------------------------------
|  Description  :
|                 Close the semaphore
|
|  Parameters   : void
|
|  Return       : void
|
+------------------------------------------------------------------------------
*/
GLOBAL void em_rr_sem_exit (void)
{
  if (sem_EM_RR NEQ VSI_ERROR)
    vsi_s_close (VSI_CALLER sem_EM_RR);
}

/*
+------------------------------------------------------------------------------
|  Function     : em_rr_sem_clear
+------------------------------------------------------------------------------
|  Description  :
|                 Clear semaphore
|
|  Parameters   : void
|
|  Return       : void
|
+------------------------------------------------------------------------------
*/
LOCAL void em_rr_sem_clear (void)
{
  if (rr_enter_critical_section(sem_EM_RR))
  {
    return;
  }

  em_rr_sem_index = 0;
  if(rr_leave_critical_section(sem_EM_RR))
  {
    return;
  }
  SYST_TRACE ( "RR:em_rr_sem_index cleared");
}

/*
+------------------------------------------------------------------------------
|  Function     : em_rr_sem_reset
+------------------------------------------------------------------------------
|  Description  :
|                 Reset the semaphore
|
|  Parameters   : void
|
|  Return       : UBYTE
|
+------------------------------------------------------------------------------
*/
/*lint -esym(714,em_rr_sem_reset) | Symbol not referenced         | used by ACI */
/*lint -esym(765,em_rr_sem_reset) | external could be made static | used by ACI */

GLOBAL void em_rr_sem_reset (void)
{
  em_rr_sem_index = 0;
  if(rr_leave_critical_section(sem_EM_RR))
  {
    return;
  }

  SYST_TRACE ( "RR:em_rr_sem_index reset OK");
}

/*
+------------------------------------------------------------------------------
|  Function     : em_rr_sem_read
+------------------------------------------------------------------------------
|  Description  :
|                 Read state of the semaphore. The rr semaphor will be read by
|                 the engineering mode via aci.
|
|  Parameters   : void
|
|  Return       : UBYTE
|
+------------------------------------------------------------------------------
*/
/*lint -esym(714,em_rr_sem_read) | Symbol not referenced         | used by ACI */
/*lint -esym(765,em_rr_sem_read) | external could be made static | used by ACI */

GLOBAL void em_rr_sem_read (void)
{
  TRACE_FUNCTION ("em_rr_sem_read()");

  {
    USHORT    semCount;

    if (vsi_s_status (VSI_CALLER sem_EM_RR, &semCount) NEQ VSI_OK)
    {
      SYST_TRACE ( "** RR:sem status error **");
      rr_semaphore_err();
      return;
    }
    if (semCount EQ 0)
    {
      vsi_o_ttrace(VSI_CALLER TC_EVENT, "semCount = %d", semCount);
      SYST_TRACE ( "semCount EQ 0");
      return;
    }
  }

  rr_enter_critical_section(sem_EM_RR);
}

/*
+------------------------------------------------------------------------------
|  Function     : em_rr_sem
+------------------------------------------------------------------------------
|  Description  :
|
|
|  Parameters   : void
|
|  Return       : TRUE/FALSE - TRUE keeps the event flag valid,
|                              FALSE indicates a successful flag handle
|
+------------------------------------------------------------------------------
*/
/*lint -esym(714,em_rr_sem) | Symbol not referenced         | used by ACI */
/*lint -esym(765,em_rr_sem) | external could be made static | used by ACI */
GLOBAL UBYTE em_rr_sem (UBYTE length, UBYTE *data)
{
  UBYTE     i;

  TRACE_FUNCTION ("em_rr_sem()");

  {
    USHORT    semCount;

    if (vsi_s_status (VSI_CALLER sem_EM_RR, &semCount) NEQ VSI_OK)
    {
      rr_semaphore_err();
      return FALSE;
    }
    if (semCount EQ 0)
    {
      vsi_o_ttrace(VSI_CALLER TC_EVENT, "semCount = %d", semCount);
      SYST_TRACE ( "semCount EQ 0");
      return FALSE;
    }
  }

  /*
   *  buffer overflow protection
   */
  if (( em_rr_sem_index + length)  > EM_RR_SEM_SIZE )
  {
    TRACE_FUNCTION ("rr buffer overflow");
    return FALSE;
  }

  if(rr_enter_critical_section(sem_EM_RR))
    return FALSE;

  for (i=0; i<length; i++)
    em_rr_sem_buffer[em_rr_sem_index++] = *(data++);

  if(rr_leave_critical_section(sem_EM_RR))
  {
    SYST_TRACE ( "** Unable to clear semaphore **");
    return FALSE;
  }

  return TRUE;             /* indicates that flag was handled */
} /* endfunc em_rr_sem */


/*
+------------------------------------------------------------------------------
|  Function     : rr_em_first_event_check()
+------------------------------------------------------------------------------
|  Description  :  Checks if first EM-Event ocured
|
|  Parameters   :  None
|
|  Return       :  None
|
+------------------------------------------------------------------------------
*/

/*
   ACI is informed about the first event trace, used for later data processing.
 */

LOCAL void rr_em_first_event_check(void)
{
 TRACE_FUNCTION("rr_em_first_event_check()");

 if(em_rr_trace_occured EQ 0)
 {
  PALLOC(em_notification, EM_DATA_IND);
  em_notification->entity = EM_RR;
  PSENDX(MMI, em_notification);
  em_rr_trace_occured++;
 }
}

/*
+------------------------------------------------------------------------------
|  Function     :  rr_em_pco_trace_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_PCO_TRACE_REQ.
|
|  Parameters   :  UBYTE pco_bitmap
|
|  Return       :  void
|
|  Purpose      :  with this primitive the requested EM Data is traced in the PCO.
+------------------------------------------------------------------------------
*/
GLOBAL void rr_em_pco_trace_req (T_EM_PCO_TRACE_REQ *em_pco_trace_req)
{
  GET_INSTANCE_DATA;
  UBYTE em_cell_type = EM_CELL_NA;
  TRACE_FUNCTION("rr_em_pco_trace_req()");

  /*check for SC data*/
  if(em_pco_trace_req->pco_bitmap & EM_PCO_SC_INFO)
  {
    switch (GET_STATE (STATE_ATT))
    {
      case ATT_IDLE:

        if (rr_data->nc_data[SC_INDEX].bcch_status EQ DECODED)
        {
          em_cell_type = EM_CELL_GSM;
#ifdef GPRS
          if(rr_data->nc_data[SC_INDEX].rac NEQ NOT_PRESENT_8BIT)
            em_cell_type = EM_CELL_GPRS;	 
#endif
          
          TRACE_EVENT_EM_P8("EM_SC_INFO_REQ_IDLE_DECODED: arfcn:%d c1:%d c2:%d rxlev:%d bsic:%d cell_id:%d dsc:%d cell_type:%d",
            
            rr_data->nc_data[SC_INDEX].arfcn,
            rr_data->nc_data[SC_INDEX].c1,
            rr_data->nc_data[SC_INDEX].c2,
            rr_data->nc_data[SC_INDEX].rxlev,
            rr_data->nc_data[SC_INDEX].bsic,
            rr_data->nc_data[SC_INDEX].cell_id,
            em_act_dlt,
            em_cell_type);

          TRACE_EVENT_EM_P5("EM_SC_INFO_REQ_IDLE_DECODED: txlev:%d tn:%d lac:%d cba:%d cbq:%d",
            rr_data->sc_data.cd.cell_options.pow_ctrl,
            rr_data->sc_data.chan_desc.tn,
            rr_data->nc_data[SC_INDEX].lai.lac,
            rr_data->nc_data[SC_INDEX].rach.cell_bar_access,
            rr_data->nc_data[SC_INDEX].c2_par.cbq);
        }
        else if (rr_data->nc_data[SC_INDEX].bcch_status EQ NON_DECODED)
        {
          TRACE_EVENT_EM_P4("EM_SC_INFO_REQ_IDLE_NON_DECODED: cell_type:%d arfcn:%d rxlev:%d bsic:%d",
            em_cell_type,
            rr_data->nc_data[SC_INDEX].arfcn,
            rr_data->nc_data[SC_INDEX].rxlev,
            rr_data->nc_data[SC_INDEX].bsic);
        }
        break;

      case ATT_DEDICATED:
        if (!rr_data->ms_data.measurement_report.valid)
          break;

        em_cell_type = EM_CELL_GSM;

#ifdef GPRS
          if(rr_data->nc_data[SC_INDEX].rac NEQ NOT_PRESENT_8BIT)
            em_cell_type = EM_CELL_GPRS;
#endif

        TRACE_EVENT_EM_P1("EM_SC_INFO_REQ_DEDICATED: cell_type:%d", em_cell_type);

        TRACE_EVENT_EM_P6("EM_SC_INFO_REQ_DEDICATED: arfcn:%d bsic:%d cell_id:%d txlev:%d tn:%d rxlev_f:%d",
          rr_data->ms_data.measurement_report.arfcn,
          rr_data->nc_data[SC_INDEX].bsic,
          rr_data->nc_data[SC_INDEX].cell_id,
          rr_data->sc_data.cd.cell_options.pow_ctrl,
          rr_data->sc_data.chan_desc.tn,
          rr_data->ms_data.measurement_report.rx_lev_full);

        TRACE_EVENT_EM_P6("EM_SC_INFO_REQ_DEDICATED: rxlev_s:%d rlt:%d ta:%d rxqual_f:%d rxqual_s:%d vocoder:%d",
          rr_data->ms_data.measurement_report.rx_lev_sub,
          em_act_rlt,
          rr_data->sc_data.new_ta,
          rr_data->ms_data.measurement_report.rx_qual_full,
          rr_data->ms_data.measurement_report.rx_qual_sub,
          rr_data->sc_data.ch_mode);

        break;

      default:
        break;
    }/*switch - state*/
  }/*if - SC data*/

  /*check NC data*/
  if(em_pco_trace_req->pco_bitmap & EM_PCO_NC_INFO)
  {
    UBYTE index = 0;
    UBYTE rac   = 0xff;
    
    switch (GET_STATE (STATE_ATT))
    {
      case ATT_IDLE:
        
        TRACE_EVENT_EM_P1("EM_NC_INFO_REQ_IDLE: num_nc:%d", /*number of neighbour cells*/
          rr_data->ms_data.measurement_report.ncells.no_of_ncells);

        for (index = 0; index < EM_MAX_NUM_NC ; index++) /*report cells ordered by index in NC list*/
        {
          em_cell_type = EM_CELL_NA;

          if (rr_data->nc_data[index].bcch_status EQ DECODED)
          {
            em_cell_type = EM_CELL_GSM;
            rac = 0xff;
#ifdef GPRS
            rac = rr_data->nc_data[index].rac;
            if(rr_data->nc_data[index].rac NEQ NOT_PRESENT_8BIT)
              em_cell_type = EM_CELL_GPRS;
#endif

            TRACE_EVENT_EM_P8("EM_NC_INFO_REQ_IDLE_DECODED: cell_type:%d rac:%d index:%d arfcn:%d c1:%d c2:%d rxlev:%d bsic:%d",
              em_cell_type,
              rac,
              index,
              rr_data->nc_data[index].arfcn,
              rr_data->nc_data[index].c1,
              rr_data->nc_data[index].c2,
              rr_data->nc_data[index].rxlev,
              rr_data->nc_data[index].bsic);

            TRACE_EVENT_EM_P6("EM_NC_INFO_REQ_IDLE_DECODED: cid:%d lac:%d f_o:%ld t_a:%ld cba:%d cbq:%d",
              rr_data->nc_data[index].cell_id,
              rr_data->nc_data[index].lai.lac,
              rr_data->ms_data.measurement_report.ncells.frame_offset[index],
              rr_data->ms_data.measurement_report.ncells.time_alignmt[index],
              rr_data->nc_data[index].rach.cell_bar_access,
              rr_data->nc_data[index].c2_par.cbq);

            TRACE_EVENT_EM_P3("EM_NC_INFO_REQ_IDLE_DECODED: cell_resel_off:%d temp_off:%d rxlev_acc_min:%ld",
              rr_data->nc_data[index].c2_par.cell_reselect_offset,
              rr_data->nc_data[index].c2_par.temp_offset,
              rr_data->nc_data[index].select_para.rxlev_access_min);

          } /* if decoded */

          else if (rr_data->nc_data[index].bcch_status EQ NON_DECODED)
          {
            TRACE_EVENT_EM_P4("EM_NC_INFO_REQ_IDLE_NON_DECODED: cell_type:%d arfcn:%d rxlev:%d bsic:%d",
              em_cell_type,
              rr_data->nc_data[index].arfcn,
              rr_data->nc_data[index].rxlev,
              rr_data->nc_data[index].bsic);
          } /* if non decoded */
        } /* for */

        break;

      case ATT_DEDICATED:
        {
        if (!rr_data->ms_data.measurement_report.valid)
          break;

        TRACE_EVENT_EM_P1("EM_NC_INFO_REQ_DEDICATED: num_nc:%d",
          rr_data->ms_data.measurement_report.ncells.no_of_ncells);

        for (index = 0; index< rr_data->ms_data.measurement_report.ncells.no_of_ncells; index++)
        {
        /*in the measurement report the cells are ordered by fieldstrength -
          non valid measurements are at the end*/
          TRACE_EVENT_EM_P6("EM_NC_INFO_REQ_DEDICATED: index:%d arfcn:%d rx_lev:%d bsic:%d f_o:%ld t_a:%ld",
            index,
            rr_data->ms_data.measurement_report.ncells.arfcn[index],
            rr_data->ms_data.measurement_report.ncells.rx_lev[index],
            rr_data->ms_data.measurement_report.ncells.bsic[index],
            rr_data->ms_data.measurement_report.ncells.frame_offset[index],
            rr_data->ms_data.measurement_report.ncells.time_alignmt[index]);
        }/*for*/

        }/*case*/
        break;

      default:
        break;
    }/*switch - state*/
  }/*if - NC data*/

  /*check Location and Paging data*/
  if(em_pco_trace_req->pco_bitmap & EM_PCO_LOC_PAG_INFO)
  {
    if (rr_data->nc_data[SC_INDEX].bcch_status EQ DECODED)
    {
      TRACE_EVENT_EM_P8("EM_LOC_PAG_INFO_REQ_DECODED: mcc:%d %d %d mnc:%d %d %d bs_pa_mfrms:%d t3212:%d",
        rr_data->nc_data[SC_INDEX].lai.mcc[0],
        rr_data->nc_data[SC_INDEX].lai.mcc[1],
        rr_data->nc_data[SC_INDEX].lai.mcc[2],

        /*2 digits vs. 3 digits*/
        rr_data->nc_data[SC_INDEX].lai.mnc[2] NEQ 0xF ? rr_data->nc_data[SC_INDEX].lai.mnc[0] :
                                                        0 ,
        rr_data->nc_data[SC_INDEX].lai.mnc[2] NEQ 0xF ? rr_data->nc_data[SC_INDEX].lai.mnc[1] :
                                                        rr_data->nc_data[SC_INDEX].lai.mnc[0],
        rr_data->nc_data[SC_INDEX].lai.mnc[2] NEQ 0xF ? rr_data->nc_data[SC_INDEX].lai.mnc[2] :
                                                        rr_data->nc_data[SC_INDEX].lai.mnc[1],

        rr_data->nc_data[SC_INDEX].control_descr.bs_pa_mfrms,
        rr_data->nc_data[SC_INDEX].control_descr.t3212);
    }
    if (rr_data->ms_data.tmsi_available EQ TRUE)
      TRACE_EVENT_EM_P1("EM_LOC_PAG_INFO_REQ_TMSI: %d", rr_data->ms_data.tmsi_binary);
  }/*if - Location and Paging data*/
  
  /*check PLMN parameters*/
  if(em_pco_trace_req->pco_bitmap & EM_PCO_PLMN_INFO)
  {
    TRACE_EVENT_EM_P4("EM_PLMN_INFO_REQ: no_creq_max:%d re_flag:%d TXPOW_max:%d RXLEV_acc_min:%d",
      rr_data->nc_data[SC_INDEX].rach.max_retrans,
      rr_data->nc_data[SC_INDEX].rach.re,
      rr_data->nc_data[SC_INDEX].select_para.ms_txpwr_max_cch,
      rr_data->nc_data[SC_INDEX].select_para.rxlev_access_min);

  }/* if - PLMN parameters*/
  
  /*check Ciphering Hopping DTX data*/
  if(em_pco_trace_req->pco_bitmap & EM_PCO_CIPH_HOP_DTX_INFO)
  {
    UBYTE i,offset;
/* Implements Measure#32: Row 242 */
    char *basestr="EM_CIP_HOP_DTX_INFO_REQ_FREQ";
    
    /*to store the trace output*/
    char hop_freq_list[MAX_SPRINTF_STRING_LEN]; /*lint !e813 , info about length*/

    if ((GET_STATE(STATE_ATT) EQ ATT_DEDICATED) AND 
        (GET_STATE(STATE_DAT) EQ DAT_DEDICATED))
    {

      TRACE_EVENT_EM_P6("EM_CIP_HOP_DTX_INFO_REQ_DEDICATED: ciph:%d dtx:%d v_start:%d hop:%d hsn:%d arfcn:%d",
      rr_data->sc_data.ciph_on,
      rr_data->sc_data.cd.dtx,
      v_start,
      rr_data->sc_data.chan_desc.hop,
      rr_data->sc_data.chan_desc.hop NEQ H_NO ? rr_data->sc_data.chan_desc.hsn : NOT_PRESENT_16BIT,
      rr_data->sc_data.chan_desc.arfcn);

      if(rr_data->sc_data.chan_desc.hop NEQ H_NO)
      {
/* Implements Measure#32: Row 242 */
        offset = sprintf(hop_freq_list, "%s1:  ", basestr);


    for (i = 0; (i < MAX_MA_CHANNELS) AND (ma[i] NEQ NOT_PRESENT_16BIT); i++)
    {
      offset += sprintf(hop_freq_list+offset, "%u ", ma[i]);
      /*
       * 4 spaces for freq + 1 blank space + 1 space for '\0'
       */
      if(offset > MAX_SPRINTF_STRING_LEN - 6)
      {    
        TRACE_EVENT_EM (hop_freq_list);
/* Implements Measure#32: Row 245 */
        offset = sprintf(hop_freq_list, "%s1_CONT:  ", basestr);
      }
    }
    TRACE_EVENT_EM (hop_freq_list);
        TRACE_EVENT_EM_P2("EM_CIP_HOP_DTX_INFO_REQ_FREQ1: nr_arfcns:%d, maio:%d",
          i,rr_data->sc_data.chan_desc.maio);
     } /* If hopping valid */

    if (v_start)
    {
  
/* Implements Measure#32: Row 247 */
      offset = sprintf(hop_freq_list, "%s2:  ", basestr);/*before starting time*/

      for (i = 0; (i < MAX_MA_CHANNELS) AND (ma2[i] NEQ NOT_PRESENT_16BIT); i++)
      {
        offset += sprintf(hop_freq_list+offset, "%u ", ma2[i]);
        if(offset > MAX_SPRINTF_STRING_LEN - 6)
        {    
          TRACE_EVENT_EM (hop_freq_list);
/* Implements Measure#32: Row 248 */
          offset = sprintf(hop_freq_list, "%s2_CONT:  ", basestr);
        }
      }
      TRACE_EVENT_EM (hop_freq_list);
        TRACE_EVENT_EM_P2("EM_CIP_HOP_DTX_INFO_REQ_FREQ2: nr_arfcns:%d, maio:%d",i, maio2);
  
    }/*if - starting time valid*/
    } /* ATT dedicated state */
    else
    {
      TRACE_EVENT_EM_P1("EM_CIP_HOP_DTX_INFO_REQ_IDLE: dtx:%d", rr_data->sc_data.cd.dtx);
    } /* ATT idle state */
  }/*if - ciphering, hopping, dtx*/
  
  /*check Power (Classmark) data*/
  if(em_pco_trace_req->pco_bitmap & EM_PCO_POWER_INFO)
  {/*here the em_power_info_cnf struckture is used, because I couldn't find classmark definition*/
    PALLOC(em_power_info_cnf, EM_POWER_INFO_CNF);
    memset (em_power_info_cnf, 0, sizeof (T_EM_POWER_INFO_CNF));
    memcpy(&em_power_info_cnf->classm2, &rr_data->ms_data.classmark2, sizeof (T_classm2));
    memcpy(&em_power_info_cnf->classm3, &rr_data->ms_data.classmark3, sizeof (T_classm3));

    TRACE_EVENT_EM_P6("EM_POWER_INFO_REQ_CLASSMARK_2: rev_lev:%d es_ind:%d a5_1:%d rf_pow_cap:%d ps_cap:%d ss_screen:%d",
      em_power_info_cnf->classm2.rev_lev,  em_power_info_cnf->classm2.es_ind,
      em_power_info_cnf->classm2.a5_1,     em_power_info_cnf->classm2.rf_pow_cap,
      em_power_info_cnf->classm2.ps_cap,   em_power_info_cnf->classm2.ss_screen);
    TRACE_EVENT_EM_P6("EM_POWER_INFO_REQ_CLASSMARK_2: sm_cap:%d freq_cap:%d class_3:%d cmsp:%d a5_3:%d a5_2:%d",
      em_power_info_cnf->classm2.sm_cap,   em_power_info_cnf->classm2.freq_cap,
      em_power_info_cnf->classm2.class_3,  em_power_info_cnf->classm2.cmsp,
      em_power_info_cnf->classm2.a5_3,     em_power_info_cnf->classm2.a5_2);

    TRACE_EVENT_EM_P6("EM_POWER_INFO_REQ_CLASSMARK_3: mb_sub:%d a5_7:%d a5_6:%d a5_5:%d a5_4:%d v_radio_cap_2:%d",
      em_power_info_cnf->classm3.mb_sub,           em_power_info_cnf->classm3.a5_7,
      em_power_info_cnf->classm3.a5_6,             em_power_info_cnf->classm3.a5_5,
      em_power_info_cnf->classm3.a5_4,             em_power_info_cnf->classm3.v_radio_cap_2);
    TRACE_EVENT_EM_P6("EM_POWER_INFO_REQ_CLASSMARK_3: ra_cap_2:%d v_ra_cap_1:%d ra_cap_1:%d v_r_sup:%d r_sup:%d v_m_s_class:%d",
      em_power_info_cnf->classm3.radio_cap_2,      em_power_info_cnf->classm3.v_radio_cap_1,
      em_power_info_cnf->classm3.radio_cap_1,      em_power_info_cnf->classm3.v_r_support,
      em_power_info_cnf->classm3.r_support,        em_power_info_cnf->classm3.v_m_s_class);
    TRACE_EVENT_EM_P6("EM_POWER_INFO_REQ_CLASSMARK_3: m_s_class:%d ucs2_treat:%d ext_meas_cap:%d v_meas_cap:%d sw_time:%d sws_time:%d",
      em_power_info_cnf->classm3.m_s_class,        em_power_info_cnf->classm3.ucs2_treat,
      em_power_info_cnf->classm3.ext_meas_cap,     em_power_info_cnf->classm3.v_meas_cap,
      em_power_info_cnf->classm3.meas_cap.sw_time, em_power_info_cnf->classm3.meas_cap.sws_time);

    PFREE(em_power_info_cnf);

  }/* if - Power (Classmark) data*/
  
  /*check Identity data*/
  if(em_pco_trace_req->pco_bitmap & EM_PCO_IDENTITY_INFO)
  {   
    TRACE_EVENT_EM_P4("EM_IDENTITY_INFO_REQ_IMEI: id_type:%d v_id:%d c_id:%d odd_even:%d",
      rr_data->ms_data.imei.ident_type,
      rr_data->ms_data.imei.v_ident_dig,
      rr_data->ms_data.imei.c_ident_dig,
      rr_data->ms_data.imei.odd_even);

    TRACE_EVENT_EM_P8("EM_IDENTITY_INFO_REQ: imei_1: %d %d %d %d %d %d %d %d",
      rr_data->ms_data.imei.ident_dig[0], rr_data->ms_data.imei.ident_dig[1],
      rr_data->ms_data.imei.ident_dig[2], rr_data->ms_data.imei.ident_dig[3],
      rr_data->ms_data.imei.ident_dig[4], rr_data->ms_data.imei.ident_dig[5],
      rr_data->ms_data.imei.ident_dig[6], rr_data->ms_data.imei.ident_dig[7]);
    TRACE_EVENT_EM_P8("EM_IDENTITY_INFO_REQ: imei_2: %d %d %d %d %d %d %d %d",
      rr_data->ms_data.imei.ident_dig[8],  rr_data->ms_data.imei.ident_dig[9],
      rr_data->ms_data.imei.ident_dig[10], rr_data->ms_data.imei.ident_dig[11],
      rr_data->ms_data.imei.ident_dig[12], rr_data->ms_data.imei.ident_dig[13],
      rr_data->ms_data.imei.ident_dig[14], rr_data->ms_data.imei.ident_dig[15]);

    if (rr_data->ms_data.imsi_available) /* SIM available */
    {
      TRACE_EVENT_EM_P4("EM_IDENTITY_INFO_REQ_IMSI: id_type:%d v_id:%d c_id:%d odd_even:%d",
        rr_data->ms_data.imsi.ident_type,
        rr_data->ms_data.imsi.v_ident_dig,
        rr_data->ms_data.imsi.c_ident_dig,
        rr_data->ms_data.imsi.odd_even);
      TRACE_EVENT_EM_P8("EM_IDENTITY_INFO_REQ: imsi_1: %d %d %d %d %d %d %d %d",
        rr_data->ms_data.imsi.ident_dig[0], rr_data->ms_data.imsi.ident_dig[1],
        rr_data->ms_data.imsi.ident_dig[2], rr_data->ms_data.imsi.ident_dig[3],
        rr_data->ms_data.imsi.ident_dig[4], rr_data->ms_data.imsi.ident_dig[5],
        rr_data->ms_data.imsi.ident_dig[6], rr_data->ms_data.imsi.ident_dig[7]);
      TRACE_EVENT_EM_P8("EM_IDENTITY_INFO_REQ: imsi_2: %d %d %d %d %d %d %d %d",
        rr_data->ms_data.imsi.ident_dig[8],  rr_data->ms_data.imsi.ident_dig[9],
        rr_data->ms_data.imsi.ident_dig[10], rr_data->ms_data.imsi.ident_dig[11],
        rr_data->ms_data.imsi.ident_dig[12], rr_data->ms_data.imsi.ident_dig[13],
        rr_data->ms_data.imsi.ident_dig[14], rr_data->ms_data.imsi.ident_dig[15]);
      }/*if - SIM available*/

    if (rr_data->ms_data.tmsi_available EQ TRUE)
      TRACE_EVENT_EM_P1("EM_IDENTITY_INFO_REQ: tmsi_bin: %ld", rr_data->ms_data.tmsi_binary);
  }/*if - Identity data*/
  
   /*check AMR configuration data*/
  if(em_pco_trace_req->pco_bitmap & EM_PCO_AMR_INFO)
  {
    if ((GET_STATE(STATE_ATT) EQ ATT_DEDICATED) AND 
        (GET_STATE(STATE_DAT) EQ DAT_DEDICATED))
    {
      if(rr_data->sc_data.ch_mode EQ CM_AMR)
      {
        UBYTE amr_index,amr_nr_modes = 1;

        TRACE_EVENT_EM_P5("EM_AMR_INFO_REQ_DEDICATED: vocoder:%d icmi:%d icm:%d acs:%d first_codec:%d",
        rr_data->sc_data.chan_desc.chan_type,
        rr_data->sc_data.amr_conf.icmi,
        rr_data->sc_data.amr_conf.st_mode,
        rr_data->sc_data.amr_conf.set_amr,
        em_get_first_codec(rr_data->sc_data.amr_conf.set_amr));

        if(rr_data->sc_data.amr_conf.v_cod_prop) 
        {
          /* Number of codec modes */
          amr_nr_modes = rr_data->sc_data.amr_conf.c_cod_prop + 1;

          TRACE_EVENT_EM_P1("EM_AMR_INFO_REQ_DEDICATED: amr_num_modes:%d",
          amr_nr_modes);

          /* Threshold and Hysteresis properties between codec modes */
          for(amr_index=0;amr_index<amr_nr_modes;amr_index++)
          {
            TRACE_EVENT_EM_P2("EM_AMR_COD_PROP: thr:%d hyst:%d",
            rr_data->sc_data.amr_conf.cod_prop[amr_index].codec_thr,
            rr_data->sc_data.amr_conf.cod_prop[amr_index].codec_hyst);
          }
        } /* codec properties */
      } /* AMR configured */
      else
      {
        TRACE_EVENT_EM("EM_AMR_INFO_REQ_DEDICATED: AMR not configured");
      } /* AMR not configured in dedicated state */
    } /* ATT dedicated state */
    else
    {
      TRACE_EVENT_EM("EM_AMR_INFO_REQ_IDLE: Not Applicable");
    } /* ATT idle state */
  } /* EM_PCO_AMR_INFO */
  
#if !defined (WIN32)
  /*check SW Version data*/
  if(em_pco_trace_req->pco_bitmap & EM_PCO_SW_VERSION_INFO)
  {
    TRACE_EVENT_EM_P1("EM_SW_SIM: %s",sim_version());
    TRACE_EVENT_EM_P1("EM_SW_SMS: %s",sms_version());
    TRACE_EVENT_EM_P1("EM_SW_SS:  %s",ss_version()) ;
    TRACE_EVENT_EM_P1("EM_SW_CC:  %s",cc_version()) ;
    TRACE_EVENT_EM_P1("EM_SW_MM:  %s",mm_version()) ;
    TRACE_EVENT_EM_P1("EM_SW_RR:  %s",rr_version()) ;
    TRACE_EVENT_EM_P1("EM_SW_DL:  %s",dl_version()) ;
    TRACE_EVENT_EM_P1("EM_SW_L1:  %s",l1_version()) ;
  }/*if - SW Version data*/
#endif
 
  PFREE(em_pco_trace_req);
}/*rr_em_pco_trace_req*/


/* ------------------ Microtec Cooperation Functions ----------*/
#ifdef GPRS /* changes for later:  Include extra compile switch for fmm */
/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_fmm_sc_info_req
+------------------------------------------------------------------------------
|  Description  :  This function provides the data for microtec fmm - Program
|
|  Parameters   :  Primitive from EM - T_EM_FMM_SC_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
This function sends the latest stored data of the SC. It is only interesting
during a GPRS connection (PTM). Therefor the ATT_STATE is ATT_IDLE (or RE_SELECTION).

*/

GLOBAL void dat_em_fmm_sc_info_req (T_EM_FMM_SC_INFO_REQ *em_fmm_sc_info_req)
{ 
  /*GET_INSTANCE_DATA;*/
 PFREE(em_fmm_sc_info_req);

 TRACE_FUNCTION ("dat_fmm_em_sc_info_req()");

#ifdef FF_WAP
 {
  PALLOC(em_fmm_sc_info_cnf, EM_FMM_SC_INFO_CNF);

  memset (em_fmm_sc_info_cnf, 0, sizeof (T_EM_FMM_SC_INFO_CNF));

  switch (GET_STATE (STATE_ATT))
  {
   case ATT_IDLE:
     if (rr_data->nc_data[SC_INDEX].bcch_status EQ DECODED)
     {
      em_fmm_sc_info_cnf->arfcn			      = rr_data->nc_data[SC_INDEX].arfcn;
      em_fmm_sc_info_cnf->c1		  	      = rr_data->nc_data[SC_INDEX].c1;
      em_fmm_sc_info_cnf->c2			        = rr_data->nc_data[SC_INDEX].c2;
      em_fmm_sc_info_cnf->rac			        = rr_data->nc_data[SC_INDEX].rac;
      em_fmm_sc_info_cnf->hyst			      = rr_data->nc_data[SC_INDEX].select_para.cell_resel_hyst;
      em_fmm_sc_info_cnf->c32			        = rr_data->nc_data[SC_INDEX].c32;	/*c32_used abfragen ?*/
      em_fmm_sc_info_cnf->gprs_prio_class = rr_data->nc_data[SC_INDEX].cr_par.priority_class;
     }
     else if (rr_data->nc_data[SC_INDEX].bcch_status EQ NON_DECODED)
     {
      em_fmm_sc_info_cnf->arfcn   = rr_data->nc_data[SC_INDEX].arfcn;
     }
     break;
   case ATT_CS3:
     break; /* Cell reselection - if needed can be included later*/
   default:
     break;
  } /*switch*/
 
/* TRACE_EVENT_P7("arfcn:%d c1:%d c2:%d rac:%d hyst:%d c32:%d prio:%d" , em_fmm_sc_info_cnf->arfcn,\
      em_fmm_sc_info_cnf->c1 , em_fmm_sc_info_cnf->c2	,  em_fmm_sc_info_cnf->rac , em_fmm_sc_info_cnf->hyst,\
      em_fmm_sc_info_cnf->c32, em_fmm_sc_info_cnf->gprs_prio_class);*/

  PSENDX( WAP, em_fmm_sc_info_cnf);
 }

#endif /*FF_WAP*/

}


/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_fmm_nc_info_req
+------------------------------------------------------------------------------
|  Description  :  Process the primitive EM_FMM_NC_INFO_REQ.
|
|  Parameters   :  Primitive from EM - T_EM_FMM_NC_INFO_REQ
|
|  Return       :  void
+------------------------------------------------------------------------------
This function sends the latest stored data of the NC. It is only interesting
during a GPRS connection (PTM). Therefor the ATT_STATE is ATT_IDLE or RE_SELECTION.
So the measurement report results should also be available in the nc_data.
*/
GLOBAL void dat_em_fmm_nc_info_req (T_EM_FMM_NC_INFO_REQ *em_fmm_nc_info_req)
{
  /*GET_INSTANCE_DATA;*/
 PFREE(em_fmm_nc_info_req);

 TRACE_FUNCTION ("dat_em_fmm_nc_info_req()");

#ifdef FF_WAP
 {
  UBYTE index = 0, index2 = 0;
  PALLOC(em_fmm_nc_info_cnf, EM_FMM_NC_INFO_CNF);

  memset (em_fmm_nc_info_cnf, 0, sizeof (T_EM_FMM_NC_INFO_CNF));

  switch (GET_STATE (STATE_ATT))
  {
		case ATT_IDLE:
		  em_fmm_nc_info_cnf->no_ncells = rr_data->ms_data.measurement_report.ncells.no_of_ncells;
			for (index = index2 = 0; index< EM_MAX_NUM_NC; index++)
			{
				if (rr_data->nc_data[index].bcch_status EQ DECODED)
				{
          em_fmm_nc_info_cnf->arfcn_nc[index2]			= rr_data->nc_data[index].arfcn;
          em_fmm_nc_info_cnf->c2_nc[index2]			= rr_data->nc_data[index].c2;
          em_fmm_nc_info_cnf->rac_nc[index2]			= rr_data->nc_data[index].rac;
          em_fmm_nc_info_cnf->c31_nc[index2]			= rr_data->nc_data[index].c31;
          em_fmm_nc_info_cnf->c32_nc[index2]			= rr_data->nc_data[index].c32;
          em_fmm_nc_info_cnf->gprs_prio_class_nc[index2]	= rr_data->nc_data[index].cr_par.priority_class;
          index2++;
       	} /* if decoded */

				else if (rr_data->nc_data[index].bcch_status EQ NON_DECODED)
				{
          em_fmm_nc_info_cnf->arfcn_nc[index]   = rr_data->nc_data[index].arfcn;
				} /* if non decoded */
			} /* for */

			break;
		default:
			break;
	}/*switch*/

/* for (index = 0 ; index < EM_MAX_NUM_NC ; index++)
   TRACE_EVENT_P6("arfcn:%d c2:%d rac:%d c31:%d c32:%d prio:%d", em_fmm_nc_info_cnf->arfcn_nc[index],\
                    em_fmm_nc_info_cnf->c2_nc[index], em_fmm_nc_info_cnf->rac_nc[index],\
                    em_fmm_nc_info_cnf->c31_nc[index], em_fmm_nc_info_cnf->c32_nc[index],\
                    em_fmm_nc_info_cnf->gprs_prio_class_nc[index]);*/

  PSENDX( WAP, em_fmm_nc_info_cnf);
 }

#endif /*FF_WAP*/


}

/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_fmm_reselection_start_ind
+------------------------------------------------------------------------------
|  Description  :  Indicates the start of an cell reselection to FMM
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_fmm_reselection_start_ind (void)
{
  /*GET_INSTANCE_DATA;*/
  TRACE_FUNCTION("dat_em_fmm_reselection_start_ind()");
#ifdef FF_WAP
  {
    PALLOC(em_fmm_reselection_start_ind, EM_FMM_RESELECTION_START_IND);
    memset (em_fmm_reselection_start_ind, 0, sizeof (T_EM_FMM_RESELECTION_START_IND));
    em_fmm_reselection_start_ind->arfcn = rr_data->nc_data[rr_data->reselect_index].arfcn;
    em_fmm_reselection_start_ind->rac = rr_data->nc_data[rr_data->reselect_index].rac;
    em_fmm_reselection_start_ind->res_type = rr_data->sc_data.selection_type;

/*    TRACE_EVENT_P3("em_fmm_reselection_start_ind %d,%d,%d",em_fmm_reselection_start_ind->arfcn,
              em_fmm_reselection_start_ind->rac,em_fmm_reselection_start_ind->res_type );*/

    PSENDX(WAP, em_fmm_reselection_start_ind);
  }

#endif /*FF_WAP */

}

/*
+------------------------------------------------------------------------------
|  Function     :  dat_em_fmm_reselection_end_ind
+------------------------------------------------------------------------------
|  Description  :  Indicates the end of an cell reselection to FMM
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
GLOBAL void dat_em_fmm_reselection_end_ind (void)
{
  /*GET_INSTANCE_DATA;*/
  TRACE_FUNCTION("dat_em_fmm_reselection_end_ind()");
#ifdef FF_WAP
  {
    PALLOC(em_fmm_resel_end_ind, EM_FMM_RESELECTION_END_IND);
    memset (em_fmm_resel_end_ind, 0, sizeof (T_EM_FMM_RESELECTION_END_IND));
    em_fmm_resel_end_ind->arfcn = rr_data->nc_data[SC_INDEX].arfcn;
    em_fmm_resel_end_ind->rac = rr_data->nc_data[SC_INDEX].rac;
/*    TRACE_EVENT_P2("em_fmm_reselection_end_ind %d,%d",em_fmm_resel_end_ind->arfcn,
              em_fmm_resel_end_ind->rac);*/

    PSENDX(WAP, em_fmm_resel_end_ind);
  }

#endif /*FF_WAP*/
 
}

#endif /* GPRS */

#endif /* FF_EM_MODE */

#endif /* RR_EM_C */
