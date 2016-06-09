/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_REGF
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
|  Purpose :  This Modul defines the functions for the registration
|             capability of the module Mobility Management.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define MM_REGF_C
#define ENTITY_MM

/*==== INCLUDES ===================================================*/
#if defined (NEW_FRAME)

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
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
#include "cnf_mm.h"
#include "mon_mm.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"

#else

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "stddefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_mm.h"
#include "mon_mm.h"
#include "vsi.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"

#endif

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/
/* added by TISH 0418 to write simloci to FFS */
extern T_loc_info                loc_info_ffs;
extern T_imsi_struct      imsi_in_ffs;
/* added by TISH 0418 to write simloci to FFS */

/*==== TEST =====================================================*/

/*==== FUNCTIONS ==================================================*/

LOCAL void reg_pack_plmn_fn            (USHORT                i,
                                        const T_RR_ABORT_IND *rr_abort_ind);

/*==== LOCALS ==================================================*/
/* first and last colums are dummies to staisf T_plmn structure*/
LOCAL const T_plmn cingular_plmn_list[]=
{ 
  { 0x1, {0x00, 0x00 ,0x00},  {0x01, 0x08 ,0x00}, 0x1},
  { 0x1, {0x03, 0x01 ,0x00},  {0x01, 0x05 ,0x00}, 0x1},
  { 0x1, {0x03, 0x01 ,0x00},  {0x01, 0x07 ,0x00}, 0x1},
  { 0x1, {0x03, 0x01 ,0x00},  {0x03, 0x08 ,0x00}, 0x1},
  { 0x1, {0x03, 0x01 ,0x00},  {0x04, 0x01 ,0x00}, 0x1},
  { 0x1, {0x03, 0x01 ,0x00},  {0x09, 0x08 ,0x00}, 0x1},
  { 0x1, {0x03, 0x04 ,0x02},  {0x08, 0x01 ,0x00}, 0x1},
  { 0x1, {0x03, 0x04 ,0x04},  {0x09, 0x03 ,0x00}, 0x1},
  { 0x1, {0x03, 0x05 ,0x00},  {0x01, 0x00 ,0x0F}, 0x1},
  { 0x1, {0x03, 0x05 ,0x02},  {0x03, 0x00 ,0x0F}, 0x1},
  { 0x1, {0x03, 0x05 ,0x08},  {0x03, 0x00 ,0x0F}, 0x1},
  { 0x1, {0x03, 0x06 ,0x00},  {0x01, 0x00 ,0x0F}, 0x1},
  { 0x1, {0x03, 0x06 ,0x06},  {0x02, 0x00 ,0x0F}, 0x1}
};

#if !defined(GPRS)

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_send_mmr_reg_cnf              |
+----------------------------------------------------------------------------+

  PURPOSE : This function builds and sends a MMR_REG_CNF primitive to
	    the MMR SAP. 
            This is only done if G23 is compiled as a GSM only protocol stack 
            without GPRS. Otherwise this function is deactivated.

*/

GLOBAL void reg_send_mmr_reg_cnf (UBYTE bootup_cause)
{
  GET_INSTANCE_DATA;
  PALLOC (mmr_reg_cnf, MMR_REG_CNF); /* T_MMR_REG_CNF */
  
  TRACE_FUNCTION ("reg_send_mmr_reg_cnf()");

  if (bootup_cause NEQ PWR_SCAN_START)
  {
    mmr_reg_cnf->plmn.v_plmn = V_PLMN_PRES;
    memcpy (mmr_reg_cnf->plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
    memcpy (mmr_reg_cnf->plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);
  }
  else
  {
    mmr_reg_cnf->plmn.v_plmn = V_PLMN_NOT_PRES;
  }
  mmr_reg_cnf->lac = mm_data->mm.lai.lac;
  mmr_reg_cnf->cid = mm_data->mm.cid;
  mmr_reg_cnf->bootup_cause = bootup_cause;
  PSENDX (MMI, mmr_reg_cnf);
}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_build_mmr_reg_cnf              |
+----------------------------------------------------------------------------+

  PURPOSE : This function indicates change in service or change in PLMN to
	    the MMR SAP. 
            This is only done if G23 is compiled as a GSM only protocol stack 
            without GPRS. Otherwise this function is deactivated.

*/

GLOBAL void reg_build_mmr_reg_cnf (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_build_mmr_reg_cnf()");

  if ((mm_data->reg.full_service_indicated EQ FALSE) OR
      (mm_data->reg.new_cell_ind EQ TRUE))
  {
    /* 
     * Either no full service was indicated to the MMI, 
     * or the PLMN has changed from that what was indicated before.
     */
    reg_send_mmr_reg_cnf (REG_END);

    mm_data->reg.full_service_indicated = TRUE;
    mm_data->reg.new_cell_ind = FALSE;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_build_mmr_nreg_ind     |
+--------------------------------------------------------------------+

  PURPOSE : This functions sends an MMR_NREG_IND to the MMR SAP.
            This is only done if G23 is compiled as a GSM only protocol stack 
            without GPRS. Otherwise this function is deactivated.

*/

GLOBAL void reg_build_mmr_nreg_ind (UBYTE service, 
                                    UBYTE search_running,
                                    UBYTE forb_ind)
{
  GET_INSTANCE_DATA;
  PALLOC (mmr_nreg_ind, MMR_NREG_IND); /* T_MMR_NREG_IND */

  TRACE_FUNCTION ("reg_build_mmr_nreg_ind");

  mmr_nreg_ind->service = service;
  mmr_nreg_ind->search_running = search_running;

  if (forb_ind EQ FORB_PLMN_INCLUDED)                  
  {
    mmr_nreg_ind->new_forb_plmn.v_plmn = V_PLMN_PRES;
    memcpy (mmr_nreg_ind->new_forb_plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
    memcpy (mmr_nreg_ind->new_forb_plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);
  }
  else
  {
    mmr_nreg_ind->new_forb_plmn.v_plmn = V_PLMN_NOT_PRES;
    memset (mmr_nreg_ind->new_forb_plmn.mcc, 0x0f, SIZE_MCC);
    memset (mmr_nreg_ind->new_forb_plmn.mnc, 0x0f, SIZE_MNC);
  }

  mmr_nreg_ind->cause = mm_data->limited_cause;

  PSENDX (MMI, mmr_nreg_ind);

  /* 
   * Delete the limited cause if it was not fatal, as 
   * on the next cell selection the reason may be another than 
   * the one now indicated.
   */
  if (mm_data->reg.op.sim_ins EQ SIM_INSRT)
    mm_data->limited_cause = MMCS_INT_NOT_PRESENT;

  mm_data->reg.full_service_indicated = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_build_mmr_nreg_cnf     |
+--------------------------------------------------------------------+

  PURPOSE : This functions sends an MMR_NREG_CNF to the MMR SAP.
            This is only done if G23 is compiled as a GSM only protocol stack 
            without GPRS. Otherwise this function is deactivated.

*/

GLOBAL void reg_build_mmr_nreg_cnf (UBYTE cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_build_mmr_nreg_cnf()");

  if (mm_data->nreg_request)
  {
    PALLOC (mmr_nreg_cnf, MMR_NREG_CNF); /* T_MMR_NREG_CNF */
    mmr_nreg_cnf->detach_cause = cause;
    PSENDX (MMI, mmr_nreg_cnf);

    mm_data->nreg_request = FALSE;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_build_mmr_plmn_ind             |
+----------------------------------------------------------------------------+

  PURPOSE : This function send a MMR_PLMN_IND primitive to the MMR SAP.
            This is only done if G23 is compiled as a GSM only protocol stack 
            without GPRS. Otherwise this function is deactivated.

*/

GLOBAL void reg_build_mmr_plmn_ind (USHORT cause,
                              const T_RR_ABORT_IND *rr_abort_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_build_mmr_plmn_ind()");

  if (mm_data->plmn_scan_mmi)
  {
    USHORT i;
    PALLOC (mmr_plmn_ind, MMR_PLMN_IND); /* T_MMR_PLMN_IND */

    mmr_plmn_ind->cause = cause;
  
    // This memset() sets all plmn.v_plmn fields to 0xff. 
    // For ACI, file PSA_MMF.C, function psaMM_CpyPLMNLst() 
    // this indicates that the PLMN is *not* valid. 
    // This convention is used at some places in MM, 
    // but it is against all usual conventions for G23.
    // By RR SAP definition 0xff is not a possible value in this field.
    // RR SAP definition says: 
    //   plmn.v_plmn = V_PLMN_PRES (0x01) PLMN valid
    //   plmn.v_plmn = V_PLMN_NOT_PRES (0x00) PLMN not valid

    memset (&mmr_plmn_ind->plmn, NOT_PRESENT_8BIT,
            sizeof (mmr_plmn_ind->plmn));
  
    if (cause EQ MMCS_SUCCESS)
    {
      /* Create the PLMN list for the MMI and send it */
      reg_create_plmn_list (rr_abort_ind, WITH_ALL_PLMNS);

      for (i = 0 ; i < mm_data->reg.plmn_cnt; i++)
      {
        reg_unpack_plmn (&mmr_plmn_ind->plmn[i], mm_data->reg.plmn, i);
        mmr_plmn_ind->forb_ind[i] = 
          reg_plmn_in_list (mm_data->reg.forb_plmn,
                            MAX_FORB_PLMN_ID,
                            &mmr_plmn_ind->plmn[i]);
        mmr_plmn_ind->rxlevel[i]  = mm_data->reg.plmn_rx [i];
        mmr_plmn_ind->lac_list[i]  = mm_data->reg.plmn_lac [i];
      }

      /* Do not consider the forbidden PLMNs for MM's internal operation */
      reg_create_plmn_list (rr_abort_ind, WITH_OTHER_PLMNS);
    }
    
    PSENDX (MMI, mmr_plmn_ind);
  
  }
   
  reg_check_plmn_search (cause, rr_abort_ind);
}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_build_mmr_ahplmn_ind             |
+----------------------------------------------------------------------------+

  PURPOSE : This function sends a MMR_AHPLMN_IND primitive to the MMR SAP.
            At poweron if valid AHPLMN or if MM receives valid AHPLMN request
            from SIM, MM should inform ACI of the changed AHPLMN.
            This is only done if G23 is compiled as a GSM only protocol stack 
            without GPRS. Otherwise this function is deactivated.
*/

GLOBAL void reg_build_mmr_ahplmn_ind (T_plmn   *acting_hplmn)
{
  TRACE_FUNCTION ("reg_build_mmr_ahplmn_ind()");
  {
    PALLOC(mmr_ahplmn_ind, MMR_AHPLMN_IND);  /* T_MMR_AHPLMN_IND */

    mmr_ahplmn_ind->ahplmn.v_plmn = acting_hplmn->v_plmn;
    memcpy (mmr_ahplmn_ind->ahplmn.mcc, acting_hplmn->mcc, SIZE_MCC); 
    memcpy (mmr_ahplmn_ind->ahplmn.mnc, acting_hplmn->mnc, SIZE_MNC);

    PSENDX(MMI, mmr_ahplmn_ind);	 
  }
}

#endif /* !defined(GPRS) */

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_build_sim_update               |
+----------------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void reg_build_sim_update (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_build_sim_update()");

  if (mm_data->reg.op.sim_ins EQ SIM_INSRT)
  {
    PALLOC (sim_mm_update_req, SIM_MM_UPDATE_REQ);

    mm_write_eplmn_to_ffs();

    reg_set_loc_info   (sim_mm_update_req);
    reg_set_bcch_info  (sim_mm_update_req);
    reg_set_forb_plmns (sim_mm_update_req);
    reg_set_kc         (sim_mm_update_req);

    /* Sending the final EF indicator Bit pattern to SIM */
    sim_mm_update_req->ef_indicator = mm_data->ef_indicator;

    PSENDX (SIM, sim_mm_update_req);
    /* Resetting to 0 after its info is conveyed to SIM */
    mm_data->ef_indicator = 0;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_copy_sim_data          |
+--------------------------------------------------------------------+

  PURPOSE : This function copies the registration data from the SIM
            provided by the SIM_MM_INSERT_IND primitive into the 
            registration data structures. A consistency check is 
            made, if data is not consistent (e.g. TMSI present, 
            but update state is different from U1 updated), this is
            corrected.

*/

GLOBAL void reg_copy_sim_data (const T_SIM_MM_INSERT_IND *sim_mm_insert_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_copy_sim_data()");

  mm_data->reg.op.ts          = TS_NO_AVAIL;
  mm_data->reg.op.sim_ins     = SIM_INSRT;

  /* Clear limited service cause for MMI information */
  mm_data->limited_cause = MMCS_INT_NOT_PRESENT;

#ifdef TM_SPECIAL
  mm_data->reg.op.ts = TS_AVAIL;
#else
  if (sim_mm_insert_ind->ad[0] >= 0x80)
    mm_data->reg.op.ts = TS_AVAIL;
  if (sim_mm_insert_ind->ad[0] EQ 4)
    mm_data->reg.cell_test = CELL_TEST_ENABLE;
  else
    mm_data->reg.cell_test = CELL_TEST_DISABLE;
#endif

  /* EF_AD in SIM has length of MNC as optional field. If the field
   * is present then Length of MNC is present and hence read it.
   */
  if (sim_mm_insert_ind->c_ad EQ 4)
  {
    mm_data->reg.length_mnc = sim_mm_insert_ind->ad[3] &0x0f;
  }

  TRACE_EVENT_P1("Length of MNC = %x",
                  sim_mm_insert_ind->c_ad);

  /* 
   * Copy the information of SIM_MM_INSERT_IND into the 
   * data fields of the registration 
   */
  reg_read_imsi     (&mm_data->reg.imsi_struct, &sim_mm_insert_ind->imsi_field);
  reg_read_loc_info (&sim_mm_insert_ind->loc_info);

  /*add by TISH 0418 to write imsi to FFS*/
  mm_read_ffs_imsi();
  if (reg_imsi_equal(&imsi_in_ffs, &mm_data->reg.imsi_struct))
  {
    /* added by TISH 0418 to write simloci to FFS */
    if( mm_data->reg.update_stat NEQ MS_UPDATED)
    {	//only use value in FFS in this case
      TRACE_EVENT("MS NOT UPDATED, readsimloci from FFS");
      if(mm_read_ffs_simloci())
      {
        /*Successful read*/
        TRACE_EVENT_P7("readlocinfo:%d,%d,%d,%d,%d,%d,%d", loc_info_ffs.loc[4],
			loc_info_ffs.loc[5], loc_info_ffs.loc[6],
			loc_info_ffs.loc[7], loc_info_ffs.loc[8],
			loc_info_ffs.loc[9], loc_info_ffs.loc[10]);
        reg_read_loc_info (&loc_info_ffs);
      }
    }
  }
  mm_data->reg.acc_class = sim_mm_insert_ind->acc_ctrl.acc[0] * 256 +
                           sim_mm_insert_ind->acc_ctrl.acc[1];
  reg_read_bcch_info (sim_mm_insert_ind);
  reg_read_kc_cksn   (sim_mm_insert_ind);

  reg_read_forb_plmn (&sim_mm_insert_ind->forb_plmn);
  reg_copy_sim_ahplmn (sim_mm_insert_ind);
  
  /* Delete the AHPLMN entry from the Forbidden list, if present*/
  if ( mm_data->reg.acting_hplmn.v_plmn)
  {
    /* Remove from forbidden PLMN list if stored */
    reg_plmn_bad_del (mm_data->reg.forb_plmn, 
                          MAX_FORB_PLMN_ID,
                          &mm_data->reg.acting_hplmn);
  }

#ifdef GPRS
  reg_clear_plmn_list (mm_data->reg.gprs_forb_plmn, MAX_GPRS_FORB_PLMN_ID);
#endif /* #ifdef GPRS */
  // sim_mm_insert_ind->phase is not needed by MM
  mm_data->reg.thplmn = sim_mm_insert_ind->hplmn;
  if (mm_data->reg.thplmn > HPLMN_MAX_SEARCH_PERIOD)
  {
    /* 3GPP 22.011 subclause 3.2.2.5 requires to use the default value
     * if the value delivered by the SIM exceeds the allowed limit. */
    mm_data->reg.thplmn = HPLMN_DEF_SEARCH_PERIOD;
  }

  /*
   * If file size is indicated in SIM MM insert indication,
   * MM shall read files from SIM.
   */
  if(reg_sim_files_to_be_read(sim_mm_insert_ind))
  {
    /*
     * Read indicated EFs in SIM_MM_INSERT_IND from SIM.
     */
    reg_read_next_sim_file();
  }

  /*Re-read any valid EPLMNs..*/
  mm_read_ffs_init();

  if (mm_data->reg.update_stat NEQ MS_UPDATED)
  {
    /* 
     * According to GSM 04.08 subclause 4.1.2.2 the SIM does not contain
     * any valid LAI, TMSI, ciphering key or ciphering key sequence number if 
     * the update status on the SIM not equals to U1.
     */
  
    /* Delete TMSI */
    mm_data->reg.tmsi = TMSI_INVALID_VALUE;
  
    /* Inform GMM about the TMSI change */
    mm_mmgmm_tmsi_ind (TMSI_INVALID_VALUE);

    /* Delete LAI */
    mm_data->reg.lai.lac = LAC_INVALID_VALUE;
    /* EF Indicator for EF LOCI - bit 1 */
    mm_data->ef_indicator|=0x01;
  
    /* Delete CKSN */
    mm_data->reg.cksn    = CKSN_RES;
  
    /* Delete also KC */
    memset (mm_data->reg.kc, 0xff, MAX_KC);
    /* Set bit 4 in ef_indicator to indicate kc change to SIM for next SIM_MM_UPDATE_REQ */
    mm_data->ef_indicator|=(0x01 << 3);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_create_plmn_list       |
+--------------------------------------------------------------------+

  PURPOSE : This functions creates the list of available PLMNs from
            the information given by the RR_ABORT_IND primitive. 
            The HPLMN is sorted to the top of the PLMN list, 
            than the PLMNs follow in th order defined in the 
            preferred PLMN list, than the PLMNs follow in the 
            order as delivered by the RR_ABORT_IND primitive.
              
            The parameter hplmn_flag controls whether the forbidden
            PLMNs are deleted from the list, this will be the case 
            for MM's internal operation or wheter all PLMNs found shall
            be present in the resulting list, this will be the case
            if the list was requested by the MMI for informational 
            purposes.
            
            hplmn_flag may also indicate whether the HPLMN shall be 
            present in the list, this functionality may become
            obsolete in the future.

            In case MM received a location updating reject #13, 
            we remember the PLMN where we received this cause. This 
            PLMN is in this case a low priority PLMN and sorted to the 
            end of the list.



*/

GLOBAL void reg_create_plmn_list (const T_RR_ABORT_IND *rr_abort_ind,
                                        UBYTE           include_flag)
{
  GET_INSTANCE_DATA;
  USHORT i;
  USHORT j;
  USHORT low_prio_index;
  BOOL copy_plmn[MAX_PLMN_ID];
  UBYTE plmn_avail;

  TRACE_FUNCTION ("reg_create_plmn_list()");

  reg_clear_plmn_list (mm_data->reg.plmn, MAX_PLMN_ID);
  mm_data->reg.plmn_cnt   = 0;
  mm_data->reg.plmn_index = 0;

  plmn_avail = rr_abort_ind->plmn_avail;

  if (plmn_avail > MAX_PLMN_ID)
    plmn_avail = MAX_PLMN_ID; /* Garbage protection, not expected to catch */
  
  for (i = 0; i < MAX_PLMN_ID; i++)
    copy_plmn[i] = TRUE;

  if (include_flag NEQ WITH_ALL_PLMNS)
  {
    /*
     * MS is in automatic mode and the list which shall be created is 
     * not for a PLMN available request,then remove the forbidden PLMNs.
     */
    for (i = 0; i < rr_abort_ind->plmn_avail; i++)
    {
      if (reg_plmn_in_list (mm_data->reg.forb_plmn, 
                            MAX_FORB_PLMN_ID,
                            &rr_abort_ind->plmn[i]))
      {
        copy_plmn[i] = FALSE;
      }

#ifdef GPRS
      /* 
       * Consider also the "forbidden PLMNs for GPRS services" list,
       * if GPRS is active.
       */
      if (!mm_gsm_alone () AND
          (mm_data->gprs.mobile_class EQ MMGMM_CLASS_CG) AND 
          reg_plmn_in_list (mm_data->reg.gprs_forb_plmn,
                            MAX_GPRS_FORB_PLMN_ID,
                            &rr_abort_ind->plmn[i]))
      {
        copy_plmn[i] = FALSE;
      }
#endif /* #ifdef GPRS */  
    }
  }

  /* 
   * In case RR delivered the PLMN where we recently received a location 
   * updating reject with cause #13, this PLMN has low priority now only.
   * This is a different handling than described in GSM 03.22 clause 4.4.3, 
   * but needed in the field.
   */
  low_prio_index = NOT_PRESENT_16BIT;
  if ((include_flag NEQ WITH_ALL_PLMNS) AND
      (include_flag NEQ WITH_RPLMN) AND
      !reg_plmn_empty (&mm_data->reg.low_prio_plmn))
  {
    for (i = 0; i < plmn_avail; i++)
    {
      if (copy_plmn[i] AND
          reg_plmn_equal_sim (&rr_abort_ind->plmn[i], 
                              &mm_data->reg.low_prio_plmn))
      {
        low_prio_index = i;
        copy_plmn[i] = FALSE;
      }
    }
    
    /* Clear the low priority PLMN */
    reg_clear_plmn (&mm_data->reg.low_prio_plmn);
  }
  
  /* 
   * In case RR delivered the actual PLMN, requested by FUNC_PLMN_SRCH, 
   * as a member of the list, we filter it out, as in this case no cell
   * of the PLMN has been recognized by RR as suitable for full service.
   */
  if ((include_flag NEQ WITH_ALL_PLMNS) AND
      (include_flag NEQ WITH_RPLMN))
  {
    for (i = 0; i < plmn_avail; i++)
    {
      if (reg_plmn_equal_sim (&rr_abort_ind->plmn[i], 
                              &mm_data->reg.actual_plmn))
      {
        /* 
         * In case of cause #12 - Location Area Not Allowed, MS shall be able
         * to select the same PLMN if suitable cell with other LAI is avaiable. 
         * Hence while preparing PLMN list MM shall not discard that PLMN.
         */
        if (((mm_data->rej_cause EQ MMCS_LA_NOT_ALLOWED) 
#ifdef GPRS
              OR (mm_data->rej_cause EQ GMMCS_LA_NOT_ALLOWED)
#endif
          )
          AND (GET_STATE(STATE_MM) EQ MM_IDLE_LIMITED_SERVICE))
        {
          TRACE_EVENT ("Don't ignore actual PLMN for cause #12");
        }
        else
        {
          TRACE_EVENT ("Ignore actual PLMN");
          copy_plmn[i] = FALSE;
        }
      }
    }
  }

  /* 
   * GSM 03.22 subclause 4.4.3 gives some rules in which way the PLMN list 
   * has to be sorted. These rules apply for manual and for automatic mode.
   * For further details, see the mentioned recommendation.
   */
  
  /* 
   * Find HPLMN in the list of found PLMNs. If present, set it on top 
   */
  for (i = 0; i < plmn_avail; i++)
  {
    if (copy_plmn[i] AND
        reg_plmn_equal_hplmn (&rr_abort_ind->plmn[i]))
    {
      reg_pack_plmn_fn (i, rr_abort_ind);
      copy_plmn[i] = FALSE;
      break;
    }
  }

  /* 
   * Add the found PLMNS into MMR_PLMN_IND according to their position in the
   * preferred PLMN list
   */
  for (j = 0; j < MAX_PREF_PLMN_ID; j++)
  {
    T_plmn pref_plmn;
    reg_unpack_plmn (&pref_plmn, mm_data->reg.pref_plmn, j);

    if (! reg_plmn_empty (&pref_plmn))
    {
      for (i = 0; i < plmn_avail; i++)
      {
        /* 
         * Use the comparison routines from GSM 03.22 Annex A (normative)
         * Reason: Even if it is not the HPLMN, we were not able to 
         * recognize a 3-digit NA MNC network as preferred if it would still
         * broadcast only 2-digit MNC.
         */
        if (copy_plmn[i] AND
            reg_plmn_equal_sim (&rr_abort_ind->plmn[i], &pref_plmn))
        {
          reg_pack_plmn_fn (i, rr_abort_ind);
          copy_plmn[i] = FALSE;
          break;
        }
      }
    }
  }

  /* 
   * Add the remaining PLMNS to MMR_PLMN_IND 
   */
  for (i = 0; i < plmn_avail; i++)
  {
    if (copy_plmn[i] AND
        ! reg_plmn_empty (&rr_abort_ind->plmn[i]))
    {
      reg_pack_plmn_fn (i, rr_abort_ind);
      copy_plmn[i] = FALSE;
    }
  }

  /* 
   * Add the low priority PLMN where we previously received LUP reject #13, 
   * if present, at the end of the list.
   */
  if (low_prio_index NEQ NOT_PRESENT_16BIT)
  {
    reg_pack_plmn_fn (low_prio_index, rr_abort_ind);
    copy_plmn[i] = FALSE;
  }

#ifndef NTRACE
  TRACE_EVENT ("PLMN list");

  for (i = 0; i < mm_data->reg.plmn_cnt; i++)
  {
    T_plmn plmn;

    reg_unpack_plmn (&plmn, mm_data->reg.plmn, i);
    TRACE_EVENT_P6 ("MCC=%x%x%x MNC=%x%x%x",
                    plmn.mcc[0],
                    plmn.mcc[1],
                    plmn.mcc[2],
                    plmn.mnc[0],
                    plmn.mnc[1],
                    plmn.mnc[2]);
  }
#endif /* #ifndef NTRACE */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_extract_hplmn          |
+--------------------------------------------------------------------+

  PURPOSE : Extracts the HPLMN out of the registration data.
            If valid AHPLMN is present then HPLMN should be
            read from the file, else it should be read from IMSI.

*/

GLOBAL void reg_extract_hplmn (T_plmn *plmn)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_extract_hplmn()");

  if (mm_data->reg.acting_hplmn.v_plmn)
  {
    plmn->v_plmn = V_PLMN_PRES;
    memcpy(plmn->mcc,mm_data->reg.acting_hplmn.mcc,SIZE_MCC);
    memcpy(plmn->mnc,mm_data->reg.acting_hplmn.mnc,SIZE_MNC);	  
  }
  else
  {
    plmn->v_plmn = V_PLMN_PRES;
    plmn->mcc[0] = mm_data->reg.imsi_struct.id[0];
    plmn->mcc[1] = mm_data->reg.imsi_struct.id[1];
    plmn->mcc[2] = mm_data->reg.imsi_struct.id[2];
    plmn->mnc[0] = mm_data->reg.imsi_struct.id[3];
    plmn->mnc[1] = mm_data->reg.imsi_struct.id[4];

    /* 
     * We cannot be sure that plmn->mnc[2] really belongs to the MNC, 
     * but the comparison routines for the HPLMN are done in a way that 
     * this doesn't matter anyway. See GSM 03.03 subclause 2.2, 
     * which is far away from being clear and GSM 03.22 version 7.1.0 
     * Release 1998 Annex A (normative). Figure A.2 in this annex makes 
     * it obvious that 3-digit-MNC isn't only a NA issue!
     */
    plmn->mnc[2] = mm_data->reg.imsi_struct.id[5];
  }/* end of mm_data->reg.acting_hplmn.v_plmn */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_init                   |
+--------------------------------------------------------------------+

  PURPOSE : This function initializes the registration data structures.

*/

GLOBAL void reg_init (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_init()");

  memset (&mm_data->reg, 0, sizeof (T_REG));

  mm_data->reg.update_stat = MS_NOT_UPDATED;
  mm_data->reg.cksn        = CKSN_RES;
  mm_data->reg.full_service_indicated = FALSE;
  mm_data->reg.sim_insert_info = NULL;
  mm_data->reg.sim_sync_req_pending = FALSE;
  mm_data->reg.quick_hplmn_search = TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_plmn_add_bad           |
+--------------------------------------------------------------------+

  PURPOSE : This function adds the given PLMN to the given list of 
            forbidden PLMNs. 
            In case the PLMN is already stored in this list, no action
            will be taken, otherwise the PLMN will be stored at the 
            first free entry of the list. If there is no free entry, 
            the first entry in the list will be deleted and the list 
            will be shifted so that the PLMN can be added at the end 
            of the list.

*/

GLOBAL void reg_plmn_add_bad (UBYTE *forb_plmn_list, 
                              USHORT list_size, 
                              const T_plmn *plmn)
{
  T_plmn forb_plmn;
  USHORT i;

  TRACE_FUNCTION ("reg_plmn_add_bad()");

  /* First look whether the PLMN is already stored */
  for (i = 0; i < list_size; i++)
  {
    reg_unpack_plmn (&forb_plmn, forb_plmn_list, i);
    if (reg_plmn_equal_sim (plmn, &forb_plmn))
      return;
  }

  /* Else look for an empty location and store the PLMN */
  for (i = 0; i < list_size; i++)
  {
    reg_unpack_plmn (&forb_plmn, forb_plmn_list, i);
    if (reg_plmn_empty (&forb_plmn))
    {
      reg_pack_plmn (forb_plmn_list, i, plmn);
      return;
    }
  }

  /* Else shift the entries down add the PLMN at the end of the list */
  memmove (&forb_plmn_list[0], 
           &forb_plmn_list[UBYTES_PER_PLMN],
           UBYTES_PER_PLMN * (list_size - 1)); /*lint !e807 This function is always called with constant 'list_size'*/
  reg_pack_plmn (forb_plmn_list, (USHORT)(list_size - 1), plmn);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_plmn_in_list           |
+--------------------------------------------------------------------+

  PURPOSE : This function checks whether the PLMN in question is a 
            member of the PLMN list provided. The function returns 
            TRUE if the PLMN can be found in the PLMN list, otherwise 
            the function returns FALSE.
*/

GLOBAL BOOL reg_plmn_in_list(const UBYTE *plmn_list, 
                                   USHORT list_size, 
                             const T_plmn *plmn)
{
  T_plmn member_plmn;
  USHORT i;

  TRACE_FUNCTION ("reg_in_forb_plmn()");

  for (i = 0; i < list_size; i++)
  {
    reg_unpack_plmn (&member_plmn, plmn_list, i);
    if (reg_plmn_equal_sim (plmn, &member_plmn))
      return TRUE;
  }
  return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_plmn_bad_del           |
+--------------------------------------------------------------------+

  PURPOSE : Remove the specified PLMN from the given list of forbidden
            PLMNs. In case the specified PLMN is found in the list, 
            it will be deleted from the list. 

*/

GLOBAL void reg_plmn_bad_del (UBYTE *forb_plmn_list, 
                              USHORT list_size, 
                              const T_plmn *plmn)
{
  GET_INSTANCE_DATA;
  T_plmn forb_plmn;
  USHORT i;
  USHORT j;

  TRACE_FUNCTION ("reg_plmn_bad_del()");

  for (i = 0; i < list_size; i++)
  {
    reg_unpack_plmn (&forb_plmn, forb_plmn_list, i);
    if (reg_plmn_equal_sim (&forb_plmn, plmn)) /* Argument order matters */
    {
      for (j = i; j < list_size - 1; j++)
      {
        memcpy (&forb_plmn_list[j * UBYTES_PER_PLMN],
                &forb_plmn_list[(j + 1) * UBYTES_PER_PLMN],
                UBYTES_PER_PLMN);
      }
      memset (&forb_plmn_list[(list_size - 1) * UBYTES_PER_PLMN], 
              0xff, UBYTES_PER_PLMN);
      /* Set bit 3 in ef_indicator to indicate forb_plmn change to SIM */
      mm_data->ef_indicator|=(0x01 << 2);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_plmn_empty             |
+--------------------------------------------------------------------+

  PURPOSE : This function tests whether plmn points to an empty PLMN.
            TRUE will be returned if plmn points to an empty PLMN, 
            otherwise FALSE.

*/

GLOBAL BOOL reg_plmn_empty (const T_plmn *plmn)
{
  /* TRACE_FUNCTION ("reg_plmn_empty ()"); */ /* Avoid too much output */

  return ((plmn->mcc[0] & 0x0F) EQ 0x0F);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_sim_ef_plmn_field_empty|
+--------------------------------------------------------------------+

  PURPOSE : This function tests whether plmn in EF points to an empty PLMN.
            TRUE will be returned if plmn points to an empty PLMN, 
            otherwise FALSE.

*/
GLOBAL BOOL reg_sim_ef_plmn_field_empty (UBYTE*     plmn)
{
  /*check if 1th digit of MCC is not present*/
  return ((plmn[0] & 0x0F) EQ 0x0F);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_plmn_is_NA_plmn        |
+--------------------------------------------------------------------+

  PURPOSE : This function tests whether the PLMN is a NA (North America)
            PLMN. The MCC code is checked to meet this decision.

*/

GLOBAL BOOL reg_plmn_is_NA_plmn (const T_plmn *plmn)
{
  /* TRACE_FUNCTION ("reg_plmn_is_NA_plmn()"); */ /* Avoid too much output */

 /* return ((plmn->mcc[0] EQ 3) AND
          (plmn->mcc[1] EQ 1) AND
          (plmn->mcc[2] >= 0) AND (plmn->mcc[2] <= 6)); lint !e568 never
                                                          less than zero*/
      return ((plmn->mcc[0] EQ 3) AND
          (plmn->mcc[1] EQ 1) AND
          (plmn->mcc[2] <= 6)); /*lint !e568 never
                                                          less than zero*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_plmn_equal_sim         |
+--------------------------------------------------------------------+

  PURPOSE : This function tests whether two PLMNs are contained in
            Equivalent PLMN list or not

*/

GLOBAL BOOL reg_plmn_equal_eqv (const T_plmn        *bcch_plmn,
                                const T_plmn        *req_plmn)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_plmn_equal_eqv()");  

  if (reg_plmn_equal_sim (bcch_plmn, req_plmn))    
    return TRUE; /* Equal without equivalent PLMN */  
  if (reg_plmn_in_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE, bcch_plmn) && 
      reg_plmn_in_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE, req_plmn))  
  {    
    return TRUE;
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_plmn_equal_sim         |
+--------------------------------------------------------------------+

  PURPOSE : This function tests whether two PLMNs are equal.

*/

GLOBAL BOOL reg_plmn_equal_sim (const T_plmn        *bcch_plmn,
                                const T_plmn        *sim_plmn)
{
  /* TRACE_FUNCTION ("reg_plmn_equal_sim()"); */ /* Avoid too much output */

  /* Check MCC */
  if (memcmp (sim_plmn->mcc, bcch_plmn->mcc, SIZE_MCC) NEQ 0)
    return FALSE;

  /* Check first 2 MNC digits */
  if (memcmp (sim_plmn->mnc, bcch_plmn->mnc, 2) NEQ 0)
    return FALSE;

  /* Check for full match */
  if (sim_plmn->mnc[2] EQ bcch_plmn->mnc[2])
    return TRUE;
  
  /* The 3rd digit of the MNC differs. */
  if (reg_plmn_is_NA_plmn (bcch_plmn))
  {
    return (((sim_plmn->mnc[2] EQ 0xf) AND (bcch_plmn->mnc[2] EQ 0x0)) OR
            ((sim_plmn->mnc[2] EQ 0x0) AND (bcch_plmn->mnc[2] EQ 0xf)));
  }
  return (bcch_plmn->mnc[2] EQ 0xf) OR (sim_plmn->mnc[2] EQ 0xf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_plmn_equal_hplmn       |
+--------------------------------------------------------------------+

  PURPOSE : This function tests whether the PLMN received on the 
            BCCH equals the HPLMN. TRUE will be returned if bcch_plmn 
            points to a PLMN description which describes the HPLMN,
            otherwise FALSE will be returned.

*/

GLOBAL BOOL reg_plmn_equal_hplmn (const T_plmn *bcch_plmn)
{
  T_plmn hplmn;

  TRACE_FUNCTION ("reg_plmn_equal_hplmn()");

  reg_extract_hplmn (&hplmn);
  return (reg_plmn_equal_sim (bcch_plmn, &hplmn));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_plmn_equal_rplmn       |
+--------------------------------------------------------------------+

  PURPOSE : This function tests whether the PLMN equals the RPLMN. 
            TRUE will be returned if PLMN points to a PLMN description 
            which describes the RPLMN,otherwise FALSE will be returned.        
*/

GLOBAL BOOL reg_plmn_equal_rplmn (T_plmn    *plmn)
{
  GET_INSTANCE_DATA;

  T_plmn last_plmn;

  TRACE_FUNCTION ("reg_plmn_equal_rplmn()");

  last_plmn.v_plmn = V_PLMN_PRES;
  memcpy (last_plmn.mcc, mm_data->reg.lai.mcc, SIZE_MCC);
  memcpy (last_plmn.mnc, mm_data->reg.lai.mnc, SIZE_MNC);

  return (reg_plmn_equal_sim (plmn, &last_plmn));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_imsi_equal             |
+--------------------------------------------------------------------+

  PURPOSE : Checks whether two given IMSIs are identical

*/

GLOBAL BOOL reg_imsi_equal (const T_imsi_struct *imsi_struct1, const T_imsi_struct *imsi_struct2)
{
  USHORT length;
  
  TRACE_FUNCTION ("reg_imsi_equal()");

  if (imsi_struct1->v_mid NEQ imsi_struct2->v_mid)
    return FALSE;
  
  if (imsi_struct1->id_type NEQ imsi_struct2->id_type)
    return FALSE;
  
  switch (imsi_struct1->id_type)
  {
    case TYPE_IMSI:
      length = mm_calculate_digits (imsi_struct1->id);
      if (length NEQ mm_calculate_digits (imsi_struct2->id))
        return FALSE;
      return (memcmp (imsi_struct1->id, imsi_struct2->id, length) EQ 0);
    
    case TYPE_TMSI:
      return (imsi_struct1->tmsi_dig EQ imsi_struct2->tmsi_dig);

    default:
      return TRUE;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_plmn_select            |
+--------------------------------------------------------------------+

  PURPOSE : This function is called if there was either a RR failure
            or a MM failure (e.g. LOCATION UPDATING reject).
            MM is in state LIMITED_SERVICE.

*/

GLOBAL void reg_plmn_select (UBYTE forb_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_plmn_select ()");

  if (mm_data->reg.plmn_cnt > mm_data->reg.plmn_index AND
      mm_data->reg.op.m EQ MODE_AUTO AND
      mm_data->reg.op.sim_ins EQ SIM_INSRT)
  {
    /*
     * Another PLMN available, automatic mode and SIM considered as valid
     */
    mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                       SEARCH_RUNNING,
                       forb_ind);
    reg_unpack_plmn (&mm_data->reg.actual_plmn,
                     mm_data->reg.plmn, mm_data->reg.plmn_index++);
    mm_data->attempt_cnt = 0;
    mm_mmr_reg_req (FUNC_PLMN_SRCH);
  }
  else
  {
    mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                       SEARCH_NOT_RUNNING,
                       forb_ind);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_read_bcch_info         |
+--------------------------------------------------------------------+

  PURPOSE : Read the BCCH information delivered by the SIM into the 
            registration data structures.

*/

GLOBAL void reg_read_bcch_info (const T_SIM_MM_INSERT_IND *sim_mm_insert_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_read_bcch_info ()");

  if(memcmp(mm_data->reg.bcch, sim_mm_insert_ind->bcch_inf.bcch, SIZE_BCCH))
  {
    /* Set bit 2 in ef_indicator to indicate bcch_info change to SIM */
    mm_data->ef_indicator|=(0x01 << 1);
  }
  memcpy (mm_data->reg.bcch, sim_mm_insert_ind->bcch_inf.bcch, SIZE_BCCH);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS              MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_sim_files_to_be_read   |
+--------------------------------------------------------------------+

  PURPOSE : Read the EFs size (EFu=EFPLMNwAcT, EFo=EFOPLMNwAcT, EFs=EFPLMNSEL)
	    information delivered 
            by the SIM into the registration data structures.

*/

GLOBAL BOOL reg_sim_files_to_be_read (const T_SIM_MM_INSERT_IND *sim_mm_insert_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_sim_files_to_be_read ()");
  mm_data->reg.sim_read_in_progress = FALSE;

#ifdef REL99
  /*
   * If the SIM insert indication and file size is indicated, it MM shall read files from SIM.
   */
  if (sim_mm_insert_ind->u_ctl_plmn_sel_actech_list_sz >0)
  {
    mm_data->reg.upd_sim_ucps_at = SAT_READ_FILE;
    /*Set indicatort sim reading is in progress to true*/
    mm_data->reg.sim_read_in_progress = TRUE;
    /*
     * MM cant decide here how many PLMNs are supported GSM access Technology so
     * MM should read maximum number of plmn as much it can read.
     * At present there is a limitation in SIM read req. It cant read more than 
     * 256 bytes in one request. May be PLMN reading can be extended using more
     * than one sim read req for the same EF in future. 
     */
    if (sim_mm_insert_ind->u_ctl_plmn_sel_actech_list_sz > 0xFF)
    {
      /*
       * In SIM_READ_REQ FF represents length not present. SIM will read actual EF length
       * and send it to MM. At present limitaion is SIM READ CNF can sent max 256 bytes data.
       */
      mm_data->reg.sim_ucps_at_len = 0xFF;
    }
    else
    {
      mm_data->reg.sim_ucps_at_len = sim_mm_insert_ind->u_ctl_plmn_sel_actech_list_sz;
    }
  }
  if(sim_mm_insert_ind->o_ctl_plmn_sel_actech_list_sz > 0)
  {
    mm_data->reg.upd_sim_ocps_at = SAT_READ_FILE;
    /*Set indicatort sim reading is in progress to true*/
    mm_data->reg.sim_read_in_progress = TRUE;
    /*
     * MM cant decide here how many PLMNs are supported GSM access Technology so
     * MM should read maximum number of plmn as much it can read.
     * At present there is a limitation in SIM read req. It cant read more than 
     * 256 bytes in one request. May be PLMN reading can be extended using more
     * than one sim read req for the same EF in future. 
     */
    if (sim_mm_insert_ind->o_ctl_plmn_sel_actech_list_sz > 0xFF)
    {
      /*
       * In SIM_READ_REQ FF represents length not present. SIM will read actual EF length
       * and send it to MM. At present limitaion is SIM READ CNF can sent max 256 bytes data.
       */
      mm_data->reg.sim_ocps_at_len = 0xFF;
    }
    else
    {
      mm_data->reg.sim_ocps_at_len = sim_mm_insert_ind->o_ctl_plmn_sel_actech_list_sz;
    }
  }
#endif

  if (sim_mm_insert_ind->pref_plmn_list_sz >0)
  {
    mm_data->reg.upd_sim_plmnsel = SAT_READ_FILE;
    /*Set indicatort sim reading is in progress to true*/
    mm_data->reg.sim_read_in_progress = TRUE;
    /* MM should read MAX PREF PLMNs because this EF does not contain any Access Technology info.*/
    if (sim_mm_insert_ind->pref_plmn_list_sz > MAX_PREF_PLMN)
    {
      mm_data->reg.sim_plmnsel_len = MAX_PREF_PLMN; /* Garbage protection */
    }
    else
    {
      mm_data->reg.sim_plmnsel_len = sim_mm_insert_ind->pref_plmn_list_sz;
    }
  }

  return mm_data->reg.sim_read_in_progress;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_read_forb_plmn         |
+--------------------------------------------------------------------+

  PURPOSE : Read the forbidden PLMN into registration data

*/

GLOBAL void reg_read_forb_plmn (const T_forb_plmn *forb_plmn)
{
  GET_INSTANCE_DATA;
  T_plmn plmn;

  TRACE_FUNCTION ("reg_read_forb_plmn ()");

  reg_clear_plmn_list (mm_data->reg.forb_plmn, MAX_FORB_PLMN_ID);
  memcpy (mm_data->reg.forb_plmn, forb_plmn->forb, 
          MAX_SIM_FORB_PLMN_ID * UBYTES_PER_PLMN);

  /* 
   * If the HPLMN is a member of the forbidden list, delete it from the list.
   */
  reg_extract_hplmn (&plmn);
  TRACE_EVENT_P6 ("HPLMN = %x%x%x %x%x%x",
                  plmn.mcc[0],
                  plmn.mcc[1],
                  plmn.mcc[2],
                  plmn.mnc[0],
                  plmn.mnc[1],
                  plmn.mnc[2]);

  reg_plmn_bad_del (mm_data->reg.forb_plmn, MAX_SIM_FORB_PLMN_ID, &plmn);

#ifndef NTRACE
  {
    USHORT i;

    for (i = 0; i < MAX_SIM_FORB_PLMN_ID; i++)
    {
      reg_unpack_plmn (&plmn, mm_data->reg.forb_plmn, i);
      TRACE_EVENT_P6 ("FORB = %x%x%x %x%x%x",
                      plmn.mcc[0],
                      plmn.mcc[1],
                      plmn.mcc[2],
                      plmn.mnc[0],
                      plmn.mnc[1],
                      plmn.mnc[2]);
    }
  }
#endif /* #ifndef NTRACE */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_copy_sim_ahplmn         |
+--------------------------------------------------------------------+

  PURPOSE : Read the AHPLMN information delivered by the SIM at poweron
            into the registration data structures.

*/

GLOBAL void reg_copy_sim_ahplmn (const T_SIM_MM_INSERT_IND *sim_mm_insert_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_copy_sim_ahplmn ()");

  TRACE_EVENT_P1("v_act_hplmn = %x",
                  sim_mm_insert_ind->v_act_hplmn);
                   

  if (!sim_mm_insert_ind->v_act_hplmn)
  {
    /*Do Nothing. AHPLMN Feature not supported by SIM. Hence, ignore*/
  }
  else 
  {
    reg_read_acting_hplmn(sim_mm_insert_ind->act_hplmn);
    /* Inform ACI & RR of the AHPLMN present in SIM */
    valid_acting_hplmn(&mm_data->reg.acting_hplmn);
    mm_build_rr_sync_hplmn_req();
    mm_mmgmm_ahplmn_ind(&mm_data->reg.acting_hplmn);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_read_imsi              |
+--------------------------------------------------------------------+

  PURPOSE : Reads IMSI delivered by SIM into T_imsi data structure

*/

GLOBAL void reg_read_imsi (T_imsi_struct *imsi_struct, const T_imsi_field *imsi_field)
{
  USHORT i;
  UBYTE  digit;
  UBYTE  length;

  TRACE_FUNCTION ("reg_read_imsi ()");
  
  imsi_struct->v_mid    = V_MID_PRES;
  imsi_struct->id_type  = TYPE_IMSI;
  imsi_struct->tmsi_dig = 0;

  length = (imsi_field->c_field - 1) * 2;
  if ((imsi_field->field[0] & 0x08) NEQ 0)
    length++;
  for (i = 0; i < length; i++)
  {
    digit = ((i & 1) NEQ 0) ?
        imsi_field->field[(i + 1) / 2] & 0x0f :
       (imsi_field->field[(i + 1) / 2] & 0xf0) >> 4;
    imsi_struct->id[i] = digit;
  }
  imsi_struct->id[i] = 0xff;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_read_kc_cksn           |
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void reg_read_kc_cksn (const T_SIM_MM_INSERT_IND *sim_mm_insert_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_read_kc_cksn ()");

  if (mm_data->reg.update_stat EQ MS_NOT_UPDATED)
  {
    /*
     * clear cipher key, Kc and location area code
     */
    mm_data->reg.cksn = CKSN_RES;
    // mm_data->reg.lac  = 0xfffe; // Write-only variable, deleted HM 20.07.00
    memset (mm_data->reg.kc, 0xFF, 8);
    /* Set bit 4 in ef_indicator to indicate kc change to SIM for next SIM_MM_UPDATE_REQ */
    mm_data->ef_indicator|=(0x01 << 3);
  }
  else
  {
    /*
     * copy parameter from SIM card
     */
    if(memcmp(mm_data->reg.kc, sim_mm_insert_ind->kc_n.kc, MAX_KC))
    {
    /* Set bit 4 in ef_indicator to indicate kc change to SIM for next SIM_MM_UPDATE_REQ */
    mm_data->ef_indicator|=(0x01 << 3);
    }
    memcpy (mm_data->reg.kc, sim_mm_insert_ind->kc_n.kc, MAX_KC);
    mm_data->reg.cksn = sim_mm_insert_ind->kc_n.kc[8];
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_read_loc_info          |
+--------------------------------------------------------------------+

  PURPOSE : Reads the location information delivered by the SIM card
            into the registration memory structures.

*/

GLOBAL void reg_read_loc_info (const T_loc_info *loc_info)
{
  GET_INSTANCE_DATA;
  ULONG tmsi_binary;

  TRACE_FUNCTION ("reg_read_loc_info ()");
  
  tmsi_binary = (((ULONG)loc_info->loc[0]) << 24) +
                (((ULONG)loc_info->loc[1]) << 16) +
                (((ULONG)loc_info->loc[2]) <<  8) +
                  (ULONG)loc_info->loc[3];

#ifdef GPRS
  mm_data->reg.indicated_tmsi = tmsi_binary;
#endif /* #ifdef GPRS */

  mm_data->reg.tmsi = tmsi_binary;
  mm_data->reg.lai.mcc[0] = loc_info->loc[4] & 0x0f;
  mm_data->reg.lai.mcc[1] = loc_info->loc[4] >> 4;
  mm_data->reg.lai.mcc[2] = loc_info->loc[5] & 0x0f;
  mm_data->reg.lai.mnc[2] = loc_info->loc[5] >> 4;
  
  mm_data->reg.lai.mnc[0] = loc_info->loc[6] & 0x0f;
  mm_data->reg.lai.mnc[1] = loc_info->loc[6] >> 4;
  mm_data->reg.lai.lac    = loc_info->loc[7] * 256 +
                            loc_info->loc[8];

  if (mm_data->reg.lai.mnc[2] EQ 0xF)
    mm_data->reg.lai.c_mnc = 2; /* 2-digit-MNC */
  else
    mm_data->reg.lai.c_mnc = 3; /* 3-digit-MNC */
  
  if (mm_data->reg.lai.lac EQ 0xffffL)
  {
    mm_data->reg.lai.lac = 0;
  }
  mm_data->reg.update_stat     = loc_info->loc[10];
  if (mm_data->reg.update_stat >= 0x07)
  {
    mm_data->reg.update_stat = MS_NOT_UPDATED;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_read_pref_plmn         |
+--------------------------------------------------------------------+

  PURPOSE : Reads the preferred PLMN list delivered by the SIM card
            into the registration memory structures.

*/

GLOBAL void reg_read_pref_plmn (UBYTE* data, USHORT length)
{
  GET_INSTANCE_DATA;
  USHORT index;
  USHORT plmnsel_plmn_count;
  USHORT plmn_count = 0;

  TRACE_FUNCTION ("reg_read_pref_plmn ()");
  /* 
   * If any additional which does not give complete
   * PLMN id(length MOD UBYTES_PER_PLMN > 0),
   * ignore these additional bytes at the end
   */
  mm_data->reg.sim_plmnsel_len = length-length%UBYTES_PER_PLMN;

  if (mm_data->reg.sim_plmnsel_len > MAX_PREF_PLMN_ID * UBYTES_PER_PLMN)
  {
    mm_data->reg.sim_plmnsel_len = MAX_PREF_PLMN_ID * UBYTES_PER_PLMN; /* Garbage protection */
  }
  plmnsel_plmn_count = mm_data->reg.sim_plmnsel_len/UBYTES_PER_PLMN;

  reg_clear_plmn_list (mm_data->reg.pref_plmn, MAX_PREF_PLMN_ID);
  
  /*Dont copy PLMN entry is empty in the EF*/
  for (index =0; index < plmnsel_plmn_count; index++)
  {
    /*Check if PLMN entry is empty in the EF*/
    if(!reg_sim_ef_plmn_field_empty(&data[index*UBYTES_PER_PLMN]))
    {
      memcpy (&mm_data->reg.pref_plmn[plmn_count*UBYTES_PER_PLMN], 
              &data[index*UBYTES_PER_PLMN], UBYTES_PER_PLMN);
      plmn_count++;
    }
  }
}

#ifdef REL99

/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                          |
| STATE   : code                ROUTINE : reg_read_ucps_acctec            |
+-------------------------------------------------------------------------+

  PURPOSE : Reads the user controlled PLMN selector with access technology
            list delivered by the SIM card. PLMN does not support GSM access 
            are ignored in the pref_plmn list.
*/
GLOBAL void reg_read_ucps_acctec(UBYTE* data, USHORT length)
{
  GET_INSTANCE_DATA;
  USHORT index;
  USHORT plmn_count=0;
  USHORT ucps_acctech_plmn_count;
  TRACE_FUNCTION ("reg_read_ucps_acctec ()");

  /* 
   * If any additional bytes at the end which does not give a complete PLMN id(length 
   * MOD UBYTES_PER_PLMN_WITH_ACC_TECH > 0), ignore these additional bytes at the end
   */
  mm_data->reg.sim_ucps_at_len = length-length%UBYTES_PER_PLMN_WITH_ACC_TECH;
  ucps_acctech_plmn_count = mm_data->reg.sim_ucps_at_len/UBYTES_PER_PLMN_WITH_ACC_TECH;
  reg_clear_plmn_list (mm_data->reg.pref_plmn, MAX_PREF_PLMN_ID);

  for( index=0; index < ucps_acctech_plmn_count; index++)
  {
    if(reg_read_plmn_support_acctec(&data[index*UBYTES_PER_PLMN_WITH_ACC_TECH]))
    {
      /*Check if PLMN entry is empty in the EF*/
      if(!reg_sim_ef_plmn_field_empty(&data[index*UBYTES_PER_PLMN_WITH_ACC_TECH]))
      {
        memcpy(&mm_data->reg.pref_plmn[plmn_count*UBYTES_PER_PLMN],
               &data[index*UBYTES_PER_PLMN_WITH_ACC_TECH], UBYTES_PER_PLMN);
        plmn_count++;
      }
    }
    if(plmn_count >= MAX_PREF_PLMN_ID)
    {
      /*MAX_PREF_PLMN_ID PLMN in pref_plmn list will be copied*/
      break;
    }
  }
  /*
   * Number of plmn copied in the pref_plmn list from the user 
   * controlled PLMN selector with access technology list 
   * delivered by the SIM card. This informaiont will be used during copy of 
   * PLMN from Operator controlled PLMN selector with access technology list 
   * delivered by the SIM card.
   */
   mm_data->reg.sim_ucps_at_len = plmn_count*UBYTES_PER_PLMN;
}

/*
+--------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                           |
| STATE   : code                ROUTINE : reg_read_plmn_support_acctec    |
+--------------------------------------------------------------------------+

  PURPOSE : This function tests whether plmn points to an access technology
            which is supported by the MS. Return TRUE if GSM access technology 
            is supported else FALSE.
*/

GLOBAL BOOL reg_read_plmn_support_acctec (UBYTE* plmn_bytes)
{
  /*
   * Check if GSM access technology is supported by the PLMN. Return TRUE 
   * if supported else FALSE
   * Spec 11.11 v8.9.1 For each User/Operator controlled PLMN Selector with Access Technology
   * Byte 5th: 8th bit = 1: GSM access technology selected;
   * Byte 5th: 8th bit = 0: GSM access technology selected;
   */
  
  return ((plmn_bytes[4] & 0x80) EQ 0x80);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_read_plmn_present    |
+--------------------------------------------------------------------+

  PURPOSE : This function tests whether plmn points to an PLMN already 
            exists in the pref_plmn list. TRUE will be returned if plmn
            already exists in the pref_plmn list, otherwise FALSE.
*/

GLOBAL BOOL reg_read_plmn_present (UBYTE* plmn_bytes)
{
  GET_INSTANCE_DATA;
  USHORT index;
  USHORT pref_plmn_count;
  pref_plmn_count = mm_data->reg.sim_ucps_at_len/UBYTES_PER_PLMN;
  for(index=0; index < pref_plmn_count; index++)
  {
    if(((mm_data->reg.pref_plmn[index*UBYTES_PER_PLMN] EQ plmn_bytes[0]) AND
         (mm_data->reg.pref_plmn[index*UBYTES_PER_PLMN+1] EQ plmn_bytes[1]) AND
         (mm_data->reg.pref_plmn[index*UBYTES_PER_PLMN+2] EQ plmn_bytes[2])))
    {
      return TRUE;
    }
  }
  return FALSE;
}


/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                               |
| STATE   : code                ROUTINE : reg_read_ocps_acctec                 |
+------------------------------------------------------------------------------+

  PURPOSE : Read Operator controlled PLMN selector with access technology list
	    delivered by the SIM card. This can happens only after SIM insert
	    indication indicates to read file from SIM.
*/
GLOBAL void reg_read_ocps_acctec(UBYTE* data, USHORT length)
{
  GET_INSTANCE_DATA;
  USHORT index;
  USHORT plmn_count;
  USHORT ocps_acctech_plmn_count;
  TRACE_FUNCTION ("reg_read_ocps_acctec ()");
  /* 
   * If any additional bytes at the end which does not give a complete PLMN id(length 
   * MOD UBYTES_PER_PLMN_WITH_ACC_TECH > 0), ignore these additional bytes at the end
   */
  mm_data->reg.sim_ocps_at_len = length-length%UBYTES_PER_PLMN_WITH_ACC_TECH;
  /*
   * Remember Number of plmn already copied in the pref_plmn list from the user
   * controlled PLMN selector with access technology list.
   */
  plmn_count = mm_data->reg.sim_ucps_at_len/UBYTES_PER_PLMN;
  ocps_acctech_plmn_count = mm_data->reg.sim_ocps_at_len/UBYTES_PER_PLMN_WITH_ACC_TECH;

  for( index=0; index < ocps_acctech_plmn_count; index++)
  {
    /*Check if the PLMN supports GSM access technology*/
    if (reg_read_plmn_support_acctec(&data[index*UBYTES_PER_PLMN_WITH_ACC_TECH]))
    { 
      /*Check if PLMN entry is empty in the EF*/
      if(!reg_sim_ef_plmn_field_empty(&data[index*UBYTES_PER_PLMN_WITH_ACC_TECH]))
      {
        /*Check if the PLMN is already present in the pref_plmn list*/
        if(!reg_read_plmn_present(&data[index*UBYTES_PER_PLMN_WITH_ACC_TECH]))
        {
          memcpy(&mm_data->reg.pref_plmn[plmn_count*UBYTES_PER_PLMN],
                 &data[index*UBYTES_PER_PLMN_WITH_ACC_TECH], UBYTES_PER_PLMN);
          plmn_count++;
        }
      }
      if(plmn_count >= MAX_PREF_PLMN_ID)
      {
        /*MAX_PREF_PLMN_ID PLMN in pref_plmn list will be copied*/
        break;
      }
    }
  }  
}

#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_clear_plmn             |
+--------------------------------------------------------------------+

  PURPOSE : Clears a given PLMN.

*/

GLOBAL void reg_clear_plmn (T_plmn *plmn)
{
  TRACE_FUNCTION ("reg_clear_plmn()");
  
  memset (plmn, 0x0F, sizeof (T_plmn));
  plmn->v_plmn = FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_clear_plmn_list        |
+--------------------------------------------------------------------+

  PURPOSE : Clears a given PLMN list. Returns TRUE if the function
            actually had to change data.

*/

GLOBAL BOOL reg_clear_plmn_list (UBYTE *plmn_list, USHORT list_size)
{
  USHORT i;
  USHORT byte_count;

  TRACE_FUNCTION ("reg_clear_plmn_list()");

  byte_count = UBYTES_PER_PLMN * list_size;

  for (i = 0; i < byte_count; i++)
  {
    if (plmn_list[i] NEQ NOT_PRESENT_8BIT)
    {
      memset (plmn_list, NOT_PRESENT_8BIT, UBYTES_PER_PLMN * list_size);
      TRACE_EVENT ("list actually deleted");
      return TRUE; /* List has been changed */
    }
  }
  return FALSE; /* Nothing changed */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_unpack_plmn            |
+--------------------------------------------------------------------+

  PURPOSE : Unpacks a PLMN from compressed form to uncompressed form.

*/

GLOBAL void reg_unpack_plmn (T_plmn *plmn, const UBYTE *packed, USHORT index)
{
  /* TRACE_FUNCTION ("reg_unpack_plmn()"); */ /* Avoid too much traces */

  index *= UBYTES_PER_PLMN;
  plmn->mcc[0] = packed[index] & 0x0f;
  plmn->mcc[1] = packed[index] >> 4;
  index++;
  plmn->mcc[2] = packed[index] & 0x0f;
  plmn->mnc[2] = packed[index] >> 4;
  index++;
  plmn->mnc[0] = packed[index] & 0x0f;
  plmn->mnc[1] = packed[index] >> 4;
  index++;
  if ((plmn->mcc[0] & 0x0F) EQ 0x0F)
    plmn->v_plmn = V_PLMN_NOT_PRES;
  else
    plmn->v_plmn = V_PLMN_PRES;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_pack_plmn              |
+--------------------------------------------------------------------+

  PURPOSE : Packs a PLMN from uncompressed form to compressed form.

*/

GLOBAL void reg_pack_plmn (UBYTE *packed, USHORT index, const T_plmn *plmn)
{
  /* TRACE_FUNCTION ("reg_pack_plmn()"); */ /* Avoid too much traces */

  index *= UBYTES_PER_PLMN;
  packed[index]  = plmn->mcc[1] << 4;
  packed[index] += plmn->mcc[0];
  index++;
  packed[index]  = plmn->mnc[2] << 4;
  packed[index] += plmn->mcc[2];
  index++;
  packed[index]  = plmn->mnc[1] << 4;
  packed[index] += plmn->mnc[0];
  index++;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_set_bcch_info          |
+--------------------------------------------------------------------+

  PURPOSE : Fill in the BCCH information in SIM_MM_UPDATE_REQ using 
            the new data in the registration data structures.

*/

GLOBAL void reg_set_bcch_info (T_SIM_MM_UPDATE_REQ *sim_mm_update_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_set_bcch_info ()");

  if (!mm_normal_upd_needed())
  {
    if(memcmp(mm_data->reg.bcch, mm_data->mm.bcch, SIZE_BCCH))
    {
      /* Set bit 2 in ef_indicator to indicate bcch_info change to SIM */
    mm_data->ef_indicator|=(0x01 << 1);
    }
    memcpy (mm_data->reg.bcch, mm_data->mm.bcch, SIZE_BCCH);
  }
  sim_mm_update_req->bcch_inf.c_bcch = SIZE_BCCH;
  memcpy (sim_mm_update_req->bcch_inf.bcch, mm_data->reg.bcch, SIZE_BCCH);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_set_forb_plmns         |
+--------------------------------------------------------------------+

  PURPOSE : Fill the forbidden PLMN field in the SIM_MM_UPDATE_REQ 
            primitive with the actual values in the MM data structures.

*/

GLOBAL void reg_set_forb_plmns (T_SIM_MM_UPDATE_REQ *sim_mm_update_req)
{
  GET_INSTANCE_DATA;
  T_plmn forb_plmn;
  UBYTE sim_forb_plmn_list[MAX_SIM_FORB_PLMN_ID * UBYTES_PER_PLMN];
  USHORT i;

  TRACE_FUNCTION ("reg_set_forb_plmns ()");

  memcpy (sim_forb_plmn_list, 
          mm_data->reg.forb_plmn, 
          MAX_SIM_FORB_PLMN_ID * UBYTES_PER_PLMN);

  for (i = MAX_SIM_FORB_PLMN_ID; i < MAX_FORB_PLMN_ID; i++)
  {
    reg_unpack_plmn (&forb_plmn, mm_data->reg.forb_plmn, i);
    if (!reg_plmn_empty (&forb_plmn))
    {
      reg_plmn_add_bad (sim_forb_plmn_list, 
                        MAX_SIM_FORB_PLMN_ID,
                        &forb_plmn);
    }
  }

  sim_mm_update_req->forb_plmn.c_forb = MAX_SIM_FORB_PLMN_ID * UBYTES_PER_PLMN;
  memcpy (sim_mm_update_req->forb_plmn.forb, 
          sim_forb_plmn_list,
          MAX_SIM_FORB_PLMN_ID * UBYTES_PER_PLMN);

#ifndef NTRACE
  for (i = 0; i < MAX_SIM_FORB_PLMN_ID; i++)
  {
    reg_unpack_plmn (&forb_plmn, sim_mm_update_req->forb_plmn.forb, i);
    TRACE_EVENT_P6 ("FORB = %x%x%x %x%x%x",
                    forb_plmn.mcc[0],
                    forb_plmn.mcc[1],
                    forb_plmn.mcc[2],
                    forb_plmn.mnc[0],
                    forb_plmn.mnc[1],
                    forb_plmn.mnc[2]);
  }
#endif /* #ifndef NTRACE */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_set_kc                 |
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void reg_set_kc (T_SIM_MM_UPDATE_REQ *sim_mm_update_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_set_kc ()");

  sim_mm_update_req->cksn = mm_data->reg.cksn;
  memcpy (sim_mm_update_req->kc, mm_data->reg.kc, MAX_KC);

  TRACE_EVENT_P1 ("CKSN = %d", mm_data->reg.cksn);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_set_loc_info           |
+--------------------------------------------------------------------+

  PURPOSE : Set the location information in the SIM_MM_UPDATE_REQ 
            primitive to the actual values of the MM data structures.

*/

GLOBAL void reg_set_loc_info (T_SIM_MM_UPDATE_REQ *sim_mm_update_req)
{
  GET_INSTANCE_DATA;
  ULONG tmsi_binary;
 
  TRACE_FUNCTION ("reg_set_loc_info ()");
  
  sim_mm_update_req->loc_info.c_loc = SIZE_LOC_INFO;

  tmsi_binary = mm_data->reg.tmsi;

  mm_mmgmm_tmsi_ind (tmsi_binary);

  sim_mm_update_req->loc_info.loc[0]  = (UBYTE)(tmsi_binary >> 24);
  sim_mm_update_req->loc_info.loc[1]  = (UBYTE)(tmsi_binary >> 16);
  sim_mm_update_req->loc_info.loc[2]  = (UBYTE)(tmsi_binary >> 8);
  sim_mm_update_req->loc_info.loc[3]  = (UBYTE)tmsi_binary;
  sim_mm_update_req->loc_info.loc[4]  = mm_data->reg.lai.mcc[1] << 4;
  sim_mm_update_req->loc_info.loc[4] += mm_data->reg.lai.mcc[0];
  sim_mm_update_req->loc_info.loc[5]  = mm_data->reg.lai.mnc[2] << 4;
  sim_mm_update_req->loc_info.loc[5] += mm_data->reg.lai.mcc[2];  
  sim_mm_update_req->loc_info.loc[6]  = mm_data->reg.lai.mnc[1] << 4;
  sim_mm_update_req->loc_info.loc[6] += mm_data->reg.lai.mnc[0];
  sim_mm_update_req->loc_info.loc[7]  = mm_data->reg.lai.lac >> 8;
  sim_mm_update_req->loc_info.loc[8]  = mm_data->reg.lai.lac & 0xff;
  sim_mm_update_req->loc_info.loc[9]  = 0;
  sim_mm_update_req->loc_info.loc[10] = mm_data->reg.update_stat;
  sim_mm_update_req->cell_identity    = mm_data->mm.cid;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_send_sim_read_req      |
+--------------------------------------------------------------------+

  PURPOSE : This functions sends SIM_READ_REQ for the requested field
            Currently only used by function reg_read_next_sim_file

*/

LOCAL void reg_send_sim_read_req ( USHORT datafield,
                                   T_path_info *path_info_ptr,
                                   UBYTE act_length,
                                   UBYTE max_length )
{
  GET_INSTANCE_DATA;
  PALLOC (read_req, SIM_READ_REQ);

  TRACE_FUNCTION ("mm_send_sim_read_req()");

  read_req->source = SRC_MM;
  /* req_id can be set to 0 as MM sends a SIM_READ_REQ only 
   * when it gets the SIM_READ_CNF to the previous request */
  read_req->req_id = 0;              

  read_req->offset = 0;

  if(path_info_ptr NEQ NULL)
  {
    read_req->v_path_info = TRUE;
    read_req->path_info = *path_info_ptr;
  }
  else
  {
    read_req->v_path_info = FALSE;
  }

  mm_data->sim_read_req_data_field = read_req->datafield = datafield;

  read_req->length = act_length;
  read_req->max_length = max_length;
  PSENDX (SIM, read_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_read_next_sim_file     |
+--------------------------------------------------------------------+

  PURPOSE : This functions requests the next changed, unread SIM file

  RETURN: TRUE if function call caused a pending SIM_READ_CNF
          FALSE otherwise

*/

GLOBAL BOOL reg_read_next_sim_file (void)
{
  GET_INSTANCE_DATA;
  /* Definition used to determine sizeof() */
  T_SIM_MM_INSERT_IND *insert_ind;
  
  TRACE_FUNCTION ("reg_read_next_sim_file()");

  if (mm_data->reg.upd_sim_fplmn EQ SAT_READ_FILE)
  {
    /* Change of forbidden PLMN indicated */

    reg_send_sim_read_req (SIM_FPLMN, NULL, NOT_PRESENT_8BIT, sizeof (insert_ind->forb_plmn.forb));
    mm_data->reg.upd_sim_fplmn = SAT_PEND_CNF;
    return TRUE;
  }

  if (mm_data->reg.upd_sim_hplmn EQ SAT_READ_FILE)
  {
    /* Change of HPLMN search timer indicated */

    reg_send_sim_read_req (SIM_HPLMN, NULL, NOT_PRESENT_8BIT, sizeof (insert_ind->hplmn));
    mm_data->reg.upd_sim_hplmn = SAT_PEND_CNF;
    return TRUE;
  }

#ifdef REL99
  /*updating status user controlled & operator controlled PLMN selection*/

  if (mm_data->reg.upd_sim_ucps_at EQ SAT_READ_FILE)
  {
    /* 
     * Change of user controlled PLMN selector with access technology list indicated.
     * Max length in SIM READ REQ is sent to FF(255 which is a limitation in SIM READ REQ)instead
     * of MAX_PREF_PLMN because MM does not know how many PLMN in the EF are supported GSM access
     * technology.So MM will try to read as many as possible.
     * In future if MM want to read more than 255 byte for the same EF, MM implementation can
     * be extented to send more than one SIM READ REQ for same EF.
     */
    reg_send_sim_read_req (SIM_UCPS_ACTEC, NULL, (UBYTE)mm_data->reg.sim_ucps_at_len, 0xFF);
    mm_data->reg.upd_sim_ucps_at = SAT_PEND_CNF;
    return TRUE;
  }

  if (mm_data->reg.upd_sim_ocps_at EQ SAT_READ_FILE)
  {
    /*
     * Read Operator controlled PLMN selector with access technology list.
     * Max length in SIM READ REQ is sent to FF(255 which is a limitation in SIM READ REQ)instead
     * of MAX_PREF_PLMN because MM does not know how many PLMN in the EF are supported GSM access
     * technology.So MM will try to read as much as possible.
     * In future if MM want to read more than 255 byte for the same EF, MM implementation can
     * be extented to send more than one SIM READ REQ for same EF.
     */
    reg_send_sim_read_req (SIM_OCPS_ACTEC, NULL, (UBYTE)mm_data->reg.sim_ocps_at_len, 0xFF);
    mm_data->reg.upd_sim_ocps_at = SAT_PEND_CNF;
    return TRUE;
  }
#endif

  if (mm_data->reg.upd_sim_acc EQ SAT_READ_FILE)
  {
    /* Change of access class indicated */

    reg_send_sim_read_req (SIM_ACC,NULL, NOT_PRESENT_8BIT, sizeof (insert_ind->acc_ctrl.acc));
    mm_data->reg.upd_sim_acc = SAT_PEND_CNF;
    return TRUE;
  }

  if (mm_data->reg.upd_sim_act_hplmn EQ SAT_READ_FILE)
  {
    /* Change of AHPLMN indicated */

    reg_send_sim_read_req (SIM_CING_AHPLMN,NULL,NOT_PRESENT_8BIT, sizeof (insert_ind->act_hplmn));
    mm_data->reg.upd_sim_act_hplmn = SAT_PEND_CNF;
    return TRUE;
  }
  if (mm_data->reg.upd_sim_plmnsel EQ SAT_READ_FILE)
  {
    /* Change of preferred PLMN list indicated */
    /*
     * EF PLMNsel will only be used if EFs EFPLMNwAcT & EFOPLMNwAcT are not used.
     * mm_data->reg.sim_uocps_at_used will only be true after successful sim read
     * cnf for EFs EFPLMNwAcT or EFOPLMNwAcT.
     */
#ifdef REL99
    if(mm_data->reg.sim_uocps_at_used EQ FALSE)
    {
      reg_send_sim_read_req (SIM_PLMNSEL,NULL, (UBYTE)mm_data->reg.sim_plmnsel_len, MAX_PREF_PLMN);
      mm_data->reg.upd_sim_plmnsel = SAT_PEND_CNF;
      return TRUE;
    }
    else
    {
      /*
       * No need to read EF PLMNsel because EFs EFPLMNwAcT and(or) EFOPLMNwAcT are being used.
       */
      mm_data->reg.upd_sim_plmnsel = SAT_UNCHANGED;
    }
#else

    reg_send_sim_read_req (SIM_PLMNSEL, NULL,(UBYTE)mm_data->reg.sim_plmnsel_len, MAX_PREF_PLMN);
    mm_data->reg.upd_sim_plmnsel = SAT_PEND_CNF;
    return TRUE;

#endif
  }
  return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : reg_end_of_deregistration  |
+--------------------------------------------------------------------+

  PURPOSE : Indicate or confirm negative registration to MMI/GMM. 
            The MM restart procedure may be performed.

*/

GLOBAL void reg_end_of_deregistration (UBYTE nreg_cause, UBYTE service)
{
  GET_INSTANCE_DATA;
  BOOL mm_restart;

  TRACE_FUNCTION ("reg_end_of_deregistration()");

  /* Remember the MM restart condition */
  mm_restart = ((mm_data->reg.sim_insert_info NEQ NULL) AND
                (mm_data->nreg_cause EQ CS_SIM_REM));

  if (mm_data->reg.sim_insert_info NEQ NULL) 
  {
    /* Insert the new SIM data and free primive */
    mm_clear_reg_data ();
    reg_copy_sim_data (mm_data->reg.sim_insert_info);
    PFREE (mm_data->reg.sim_insert_info);
    mm_data->reg.sim_insert_info = NULL;
  }

  if (mm_restart)
  {
    /* 
     * End of MM restart procedure, re-register
     */
    if (mm_data->reg.op.m EQ MODE_AUTO)
    {
      /* 
       * Register in automatic mode 
       */
      mm_auto_net_reg ();
    }
    else
    {
      /* 
       * Register in manual mode 
       */
      mm_data->reg.plmn_cnt = 0; /* Delete list of available PLMNs */
      mm_data->attempt_cnt = 0;
      mm_mmr_reg_req (FUNC_PLMN_SRCH);
    }
  }
  else
  {
    /* 
     * This was no MM RESTART
     */
    if (mm_data->nreg_request) 
    {
      /* 
       * The deregistration was requested by the MMI 
       */
      
      if (nreg_cause EQ CS_POW_OFF)
      {
        /* Hard switch off (AT+CFUN=0), delete SIM data physically */ 
        reg_init ();
        
        mm_data->limited_cause = MMCS_SIM_REMOVED; /* MMCS_SIM_INVAL_NOSIM */
      }

      mm_mmgmm_nreg_cnf (nreg_cause);
    }
    else
    {
      /* 
       * This was a real SIM remove, the SIM has been pulled
       */
      if (service EQ NO_SERVICE)
      {
        mm_mmgmm_nreg_ind (NREG_NO_SERVICE, 
                           SEARCH_NOT_RUNNING,
                           FORB_PLMN_NOT_INCLUDED);
      }
      else
      {
        mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                           SEARCH_NOT_RUNNING,
                           FORB_PLMN_NOT_INCLUDED);
      }
    }
  }

#ifdef GPRS
  if (mm_data->gprs.sim_physically_removed)
  {
    mm_data->gprs.sim_physically_removed = FALSE;
    
    /* Delete registration data */
    reg_init ();
  }
#endif /* GPRS */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_invalidate_upd_state   |
+--------------------------------------------------------------------+

  PURPOSE : This function invalidates the update state. 

*/          

#ifdef REL99
GLOBAL void reg_invalidate_upd_state (UBYTE new_update_state, BOOL tmsi_cksn_kc_not_deleted)
#else
GLOBAL void reg_invalidate_upd_state (UBYTE new_update_state)
#endif
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_invalidate_update_state()");

  /* No IMSI ATTACH neccessary anymore */
  if (mm_data->first_attach)
  {
    mm_data->first_attach_mem = mm_data->first_attach;
    mm_data->first_attach = FALSE;
  }

  /* No periodic update needed anymore, needing normal update now */
  mm_data->t3212_timeout = FALSE;

  /* Set new update state */
  mm_data->reg.update_stat = new_update_state;

#ifdef REL99
  if(tmsi_cksn_kc_not_deleted EQ TRUE)
  {
    /*Dont delete LAI CKSN CKSN KC*/
  }
  else
#endif
  {
    /* Delete TMSI */
    mm_data->reg.tmsi = TMSI_INVALID_VALUE;

    /* Delete LAI */
    mm_data->reg.lai.lac = LAC_INVALID_VALUE;

    /* Delete CKSN */
    mm_data->reg.cksn    = CKSN_RES;

    /* Delete also KC */
    memset (mm_data->reg.kc, 0xff, MAX_KC);
  }

  /* Delete BCCH information */
  memset (mm_data->reg.bcch, 0, SIZE_BCCH);
  
  /* Update all EFs on SIM */
  mm_data->ef_indicator = 0xFF;
  /* Write changed data to SIM */
  reg_build_sim_update ();
  /* added by TISH 0418 to write simloci to FFS */
  mm_write_simloci_to_ffs();
  mm_write_imsi_to_ffs();

  /* Check HPLMN timer state */
  reg_check_hplmn_tim (mm_data->reg.thplmn);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : reg_select_network                 |
+----------------------------------------------------------------------------+

  PURPOSE : This function starts the network registration for the given PLMN.

*/

GLOBAL void reg_select_network (const T_plmn *plmn)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_select_network()");
  
  mm_data->reg.actual_plmn = *plmn; /* Struct copy */

  if (reg_plmn_empty(plmn))
  {
    if (mm_data->reg.update_stat EQ MS_UPDATED)
    {
      mm_data->reg.actual_plmn.v_plmn = TRUE;
      memcpy(mm_data->reg.actual_plmn.mcc, mm_data->reg.lai.mcc, SIZE_MCC);
      memcpy(mm_data->reg.actual_plmn.mnc, mm_data->reg.lai.mnc, SIZE_MNC);
    }
    else
    {
      /* If PLMN sent is 0xFF (PLMN not present in FFS) and MM Update Status
       * is MS_NOT_UPDATED, We send a error message to ACI */
      mm_mmgmm_nreg_ind(NREG_LIMITED_SERVICE, 
                           SEARCH_NOT_RUNNING, 
                           FORB_PLMN_NOT_INCLUDED);
      return;
    }
  }
  mm_data->attempt_cnt = 0;
  mm_mmr_reg_req (FUNC_PLMN_SRCH);
}


/*

+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_best_plmn_in_country           |
+----------------------------------------------------------------------------+

  PURPOSE : This function checks whether the given PLMN is the best PLMN
            in the country where the mobile is roaming.

*/

GLOBAL BOOL reg_best_plmn_in_country (const T_plmn *bcch_plmn)
{
  GET_INSTANCE_DATA;
  T_plmn hplmn;
  USHORT i;

  TRACE_FUNCTION ("reg_best_plmn_in_country()");

  reg_extract_hplmn (&hplmn);

  /* fix for CT PTCRB- TC_26_7_4_5_4_6. HPLMN timer is not started only if 
   bcch PLMN and HPLMN belong to same country and both are in equivalent PLMN list.
   Detailed analysis present in OMAPS00150594*/
  if (reg_same_country_plmn (bcch_plmn, &hplmn))
  {
      if (reg_plmn_equal_eqv (bcch_plmn, &hplmn))
        return TRUE; /* The PLMN is the HPLMN */
      else
        return FALSE; /* National roaming */
  }
  /* International roaming: Check the preferred PLMN list */
  for (i = 0; i < MAX_PREF_PLMN_ID; i++)
  {
    T_plmn pref_plmn;

    reg_unpack_plmn (&pref_plmn, mm_data->reg.pref_plmn, i);

    if (reg_same_country_plmn (bcch_plmn, &pref_plmn))
      return reg_plmn_equal_eqv (bcch_plmn, &pref_plmn);
  }

  return TRUE; /* For this country no entry exists */
}


/*

+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_check_hplmn_tim                |
+----------------------------------------------------------------------------+

  PURPOSE : This function checks whether:
            - the HPLMN timer has to be started (if not running)
	      with the duration given as input parameter or 
            - stopped. 
            Time unit is decihour.
*/

GLOBAL void reg_check_hplmn_tim (UBYTE decihours)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_check_hplmn_tim()");

  /*
   * The HPLMN timer has to be started if all the conditions below are
   * fulfilled and is not running already. Otherwise, it has to be
   * stopped.
   * - Registration state is updated
   * - MS is roaming on a VPLMN
   * - Registration mode is automatic
   * - THPLMN Elementary File is not equal to zero
   */

  if (mm_full_service_pplmn_scan())
  {
    if (mm_data->reg.op.m EQ M_AUTO AND
        mm_data->reg.thplmn NEQ 0 AND
        mm_data->reg.update_stat NEQ MS_LA_NOT_ALLOWED AND
        !reg_best_plmn_in_country (&mm_data->reg.actual_plmn))
    {
      /*
       * Period of HPLMN is controlled by EF_HPLMN file of the SIM.
       * According to TS 11.11 chapter 10.3.5 :
       *  0- no search attempt
       *  N- search attempts every N*6 min intervals (6 min to 8 hours)
       * For phase 1 SIM wehereby this EF is not available, a default 
       * value of 1 hour shall be used according to TS 22.011 
       * chapter 3.2.2.5. In G23 this is managed by SIM entity that 
       * will force this default value within SIM_INSERT_IND.
       */
      if (!TIMERACTIVE(T_HPLMN))
      {
        if (mm_data->first_attach_mem)
        {
          TRACE_EVENT_P1 ("Start initial HPLMN timer: %d min", 2);
          TIMERSTART(T_HPLMN, HPLMN_INITIAL_DELAY);
        }
        else
        {
          TRACE_EVENT_P1 ("Start HPLMN timer: %d", decihours);
          TIMERSTART(T_HPLMN, decihours * 360000);
        }
      }/* if timeractive*/
    }/*end if mm_data->reg.op.m eq*/
    else
    {
      reg_stop_hplmn_tim ();
    mm_data->first_attach_mem = FALSE;
  }
    
  }/* end if mm_full_service_pplmn,,,*/
  else
  {
    reg_stop_hplmn_tim();
  }
  /* Issue 31179  This timer is started for foreign mcc only for cingular */
  if( mm_data->reg.is_cingular_sim AND !mm_data->first_attach)
  {   
    T_plmn   hplmn;
    reg_extract_hplmn (&hplmn);
    if(!TIMERACTIVE(T_HPLMN))
    {
      if(memcmp(&(mm_data->reg.actual_plmn.mcc[0]), &(hplmn.mcc[0]), SIZE_MCC) NEQ 0)
      {
        TRACE_EVENT_P1 ("Start HPLMN timer: %d", decihours);
        TIMERSTART(T_HPLMN, decihours * 360000);
      }
    }
  }
  return;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_stop_hplmn_tim                 |
+----------------------------------------------------------------------------+

  PURPOSE : This function stops the HPLMN timer.
*/

GLOBAL void reg_stop_hplmn_tim (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_stop_hplmn_tim()");

  TRACE_EVENT("Stop HPLMN timer");
  TIMERSTOP(T_HPLMN);
  mm_data->plmn_scan_mm = FALSE;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_plmn_in_pref_list              |
+----------------------------------------------------------------------------+

  PURPOSE : This function returns TRUE if the PLMN belongs to the 
            preferred list.
*/

GLOBAL BOOL reg_plmn_in_pref_list (const T_plmn *plmn)
{
  GET_INSTANCE_DATA;
  USHORT i;
  TRACE_FUNCTION ("reg_plmn_in_pref_list()");

  for (i = 0; i < MAX_PREF_PLMN_ID; i++)
  {
    T_plmn pref_plmn;

    reg_unpack_plmn (&pref_plmn, mm_data->reg.pref_plmn, i);
    if (!reg_plmn_empty (&pref_plmn))
    {
      if (reg_plmn_equal_sim (plmn, &pref_plmn))
        return TRUE;
    }
  }

  return FALSE;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_same_country_plmn              |
+----------------------------------------------------------------------------+

  PURPOSE : This function returns TRUE if both PLMN belongs to the 
            same country, handling the special case of NA (several
            MCC).
*/

GLOBAL BOOL reg_same_country_plmn (const T_plmn *plmn1,
                                   const T_plmn *plmn2)
{
  /* TRACE_FUNCTION ("reg_same_country_plmn()"); */ /* Avoid too much traces */

  if (reg_plmn_is_NA_plmn(plmn1))
    return (reg_plmn_is_NA_plmn(plmn2));
  else
    return (memcmp(&(plmn1->mcc[0]), &(plmn2->mcc[0]), SIZE_MCC) EQ 0);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_check_plmn_search              |
+----------------------------------------------------------------------------+

  PURPOSE : This function checks whether a better PLMN has been found and, 
            if so, starts automatic registration on this PLMN.
            In case the search has to be aborted for some reason (eg. 
            MM is not in IDLE mode), the HPLMN search timer is started 
            using a small value.
*/

GLOBAL void reg_check_plmn_search (USHORT cause,
                             const T_RR_ABORT_IND *rr_abort_ind)
{
  GET_INSTANCE_DATA;
  T_plmn plmn;
  BOOL success;

  TRACE_FUNCTION ("reg_check_plmn_search()");
  
  if (mm_data->plmn_scan_mm)
  {
    /* 
     * A MM search was ongoing
     */
    switch (cause)
    {
      case MMCS_PLMN_NOT_IDLE_MODE:
        TRACE_EVENT ("PLMN scan aborted");

        /* 
         * PLMN scan has been aborted because not compatible with the 
         * current activity of the MS. Restart timer with a small duration.
         * Bufferize / postpone this in a later implementation (Maybe).
         */
        reg_check_hplmn_tim (HPLMN_REARM_DELAY);
        break;

      case MMCS_SUCCESS:
        /* 
         * First we need to create the list of available PLMNs. We 
         * are not interested in the ones belonging to a forbidden list 
         * but we are still interested in the current RPLMN.
         * Criteria to start the PLMN selection will be:
         * 1) First PLMN in the list (higher priority) is not the RPLMN
         * 2) Either it is the HPLMN,
         *    Or it is an other PLMN, which belongs to the same country 
         *    than the current VPLMN, and it belongs to the preferred 
         *    list ie it has not been inserted here to the randomization
         *    of the PLMNs whose fieldstrength is higher than -85 dBm.
         */

        reg_create_plmn_list (rr_abort_ind, WITH_RPLMN);

        success = FALSE;
        
        while (mm_data->reg.plmn_index < mm_data->reg.plmn_cnt)
        {
          reg_unpack_plmn (&plmn, mm_data->reg.plmn, mm_data->reg.plmn_index);
          
          if (reg_plmn_equal_sim (&plmn, &mm_data->reg.actual_plmn))
          {
            TRACE_EVENT ("VPLMN hit - cancel");
            break;
          }
          else if (reg_plmn_equal_hplmn (&plmn))
          {
            /* fix for TC 26.7.4.5.4.4. Select a PLMN of Same country 
               and not HPLMN if in International roaming only for Test sim.*/
            if((mm_data->reg.op.ts EQ TS_NO_AVAIL) OR
               reg_same_country_plmn (&plmn, &mm_data->reg.actual_plmn))
            {
              TRACE_EVENT ("HPLMN found - success");
              success = TRUE;          
              break;
            }
            else
            {
              mm_data->reg.plmn_index++;
              continue;
            }
          }
          else if (!reg_plmn_in_pref_list (&plmn))
          {
            TRACE_EVENT ("PPLMN list end - cancel");
            break;
          }
          else if(mm_data->reg.is_cingular_sim)
          {
            TRACE_EVENT ("better PLMN from HPLMN list in Cingular - success");
            success = TRUE;
            break;
          }
          else if (reg_same_country_plmn (&plmn, &mm_data->reg.actual_plmn))
          {
            TRACE_EVENT ("better PLMN same country - success");
            success = TRUE;
            break;
          }
          else
            mm_data->reg.plmn_index++;
        }

        if (success)
        {
          TRACE_EVENT ("PPLMN rescan pass");

          /* 
           * An alternate network candidate has been found, so try to camp
           * on it. It the registration procedure fails, then the normal
	   * automatic network selection procedure will apply.
           */
          reg_select_network (&plmn); /*lint !e772 conceivably not initialized */
        }
        else
        {
          TRACE_EVENT ("PPLMN rescan failed");

          /* 
           * PLMN rescan did not provide any interesting results, 
           * so restart timer.
           */
          reg_check_hplmn_tim (mm_data->reg.thplmn);
        }

        /* 
         * Do not consider the forbidden PLMNs for MM's internal operation
         */
        reg_create_plmn_list (rr_abort_ind, WITH_OTHER_PLMNS);
        break;

      case MMCS_SIM_REMOVED:
        /* 
         * Not relevant for MM initiated search.
         */
        break;

      default: 
        TRACE_ERROR (UNEXPECTED_DEFAULT);
        break;
    }
  }

  mm_data->plmn_scan_mm  = FALSE;
  mm_data->plmn_scan_mmi = FALSE;

}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_store_eqv_plmns                |
+----------------------------------------------------------------------------+

  PURPOSE : This function checks whether a received equivalent PLMN list
            matches an already stored list. If no match then the new 
            equivalent PLMN list overwrites the currently stored list.
*/
BOOL reg_store_eqv_plmns(T_eqv_plmn_list *rx_eplmn_list, T_plmn *plmn)
{
  GET_INSTANCE_DATA;
  U8 i=0, j=0;
  T_plmn local_plmn;
  UBYTE local_store[EPLMNLIST_SIZE*UBYTES_PER_PLMN];
  
  TRACE_FUNCTION ("reg_store_eqv_plmns()");
  
  if (rx_eplmn_list->c_eqv_plmn > EPLMNLIST_SIZE)
  {  
    rx_eplmn_list->c_eqv_plmn = 0;
    TRACE_ERROR ("count out of range");
    return FALSE;
  }

  for (i=0; i < EPLMNLIST_SIZE - 1; i++)
  {
    /* Use 0xf as filler for 2 digit MNCs */
    if (rx_eplmn_list->eqv_plmn[i].c_mnc EQ (SIZE_MNC-1))
      rx_eplmn_list->eqv_plmn[i].mnc[SIZE_MNC-1] = 0xf;
  }
  
  /*Convert the new list into a MM-friendly format*/
  reg_pack_plmn(local_store, 0, plmn);

  for(i=0;i<EPLMNLIST_SIZE-1;i++)
  {
    memcpy(&local_plmn.mcc, &rx_eplmn_list->eqv_plmn[i].mcc, SIZE_MCC);
    memcpy(&local_plmn.mnc, &rx_eplmn_list->eqv_plmn[i].mnc, SIZE_MNC);

    reg_pack_plmn(local_store, i+1, &local_plmn);
  }

  if(memcmp(local_store, mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE*UBYTES_PER_PLMN))
  {
    /* The Equivalent PLMN list has changed */

    /*Initialise EPLMN storage*/
    memset (&mm_data->reg.eqv_plmns.eqv_plmn_list, 0xFF, EPLMNLIST_SIZE*UBYTES_PER_PLMN);

    /* Store Equivalent PLMNs */
    for(i=0,j=0;i<EPLMNLIST_SIZE;i++)
    {
      T_plmn new_plmn;

      reg_unpack_plmn(&new_plmn, local_store, i);

      /* Remove any forbidden PLMNs from the new EPLMN list */
      if(!reg_plmn_in_list(mm_data->reg.forb_plmn, MAX_FORB_PLMN_ID, &new_plmn))
        reg_pack_plmn(mm_data->reg.eqv_plmns.eqv_plmn_list, j++, &new_plmn);
    }
    return(TRUE);
  }
  return(FALSE);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_read_acting_hplmn                 |
+----------------------------------------------------------------------------+

  PURPOSE : This function copies the AHPLMN value read from the SIM at poweron
            or after REFRESH command from network.            
*/
GLOBAL void reg_read_acting_hplmn (const U8  acting_hplmn[])
{
  GET_INSTANCE_DATA;
  mm_data->reg.acting_hplmn.mcc[0] = acting_hplmn[0] &0x0f;
  mm_data->reg.acting_hplmn.mcc[1] = acting_hplmn[0] >> 4;
  mm_data->reg.acting_hplmn.mcc[2] = acting_hplmn[1] &0x0f;
  mm_data->reg.acting_hplmn.mnc[2] = acting_hplmn[1] >>4;
  mm_data->reg.acting_hplmn.mnc[0] = acting_hplmn[2] &0x0f;
  mm_data->reg.acting_hplmn.mnc[1] = acting_hplmn[2] >>4;
}

            
/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : valid_acting_hplmn                 |
+----------------------------------------------------------------------------+

  PURPOSE : This function checks whether a received AHPLMN has the
            same MCC as the True-HPLMN and if yes then checks the
            validity of the MNC digits. Thus it checks the validity
            of AHPLMN received.
*/

BOOL valid_acting_hplmn(T_plmn   *acting_hplmn)
{
  GET_INSTANCE_DATA;
  T_plmn   hplmn;

  UINT  temp1=0x0F0F0F;

  /*This will be set to TRUE if AHPLMN is FFFFFF*/
  mm_data->reg.acting_hplmn_invalid = FALSE;
 
  TRACE_FUNCTION ("valid_acting_hplmn()");

  /* Extract HPLMN from IMSI and compare MCC of HPLMN and AHPLMN */
  reg_extract_hplmn(&hplmn);

  /* Check If the AHPLMN contains FFFFFF */

  if (!(memcmp(acting_hplmn->mcc,&temp1,SIZE_MCC) AND
        memcmp(acting_hplmn->mnc,&temp1,SIZE_MNC)))
  {     
     mm_data->reg.acting_hplmn.v_plmn = V_PLMN_NOT_PRES;     
     mm_data->reg.acting_hplmn_invalid = TRUE;
     return FALSE;
  }
  
  /* If MCC of HPLMN and AHPLMN differs invalidate AHPLMN */
  if ( memcmp(acting_hplmn->mcc,mm_data->reg.imsi_struct.id,3) )
  {
    mm_data->reg.acting_hplmn.v_plmn = V_PLMN_NOT_PRES;         
    return FALSE;
  }

  if (acting_hplmn->mnc[0] EQ 0x0f OR
      acting_hplmn->mnc[1] EQ 0x0f )
  {
    mm_data->reg.acting_hplmn.v_plmn = V_PLMN_NOT_PRES;         
    return FALSE;
  }
  else
  {
    /* 1. If True HPLMN has only 2 mnc digits ignore 3rd digit of mnc in AHPLMN 
          even if exists 
       2. If True HPLMN has mnc as 3 digits but AHPLMN has only 2 OR if any
          of the mnc digits are 0xff,invalidate the AHPLMN
    */
    if (mm_data->reg.length_mnc EQ 2 ) 
    {
      acting_hplmn->mnc[2] = 0x0F;
    }
    else if(acting_hplmn->mnc[2] EQ 0xFF)
    {
      mm_data->reg.acting_hplmn.v_plmn = V_PLMN_NOT_PRES;         
      return FALSE;
    }
  }/* else acting_hplmn->mnc */

  mm_data->reg.acting_hplmn.v_plmn = V_PLMN_PRES;
  return TRUE;

}/* end of valid_acting_hplmn */

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                             |
| STATE   : code                ROUTINE : reg_pack_plmn_fn.....              |
+----------------------------------------------------------------------------+

  PURPOSE : This function packs a PLMN.
*/

LOCAL void reg_pack_plmn_fn (USHORT               i,
                             const T_RR_ABORT_IND *rr_abort_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_pack_plmn_fn()");
  reg_pack_plmn (mm_data->reg.plmn, mm_data->reg.plmn_cnt, 
                 &rr_abort_ind->plmn[i]);
  mm_data->reg.plmn_rx[mm_data->reg.plmn_cnt] = rr_abort_ind->rxlevel[i];
  mm_data->reg.plmn_lac[mm_data->reg.plmn_cnt] = rr_abort_ind->lac_list[i]; /* LOL 02.01.2003: added for EONS support */
  mm_data->reg.plmn_cnt++;
}/*reg_pack_plmn_fn*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : check_if_cingular_sim      |
+--------------------------------------------------------------------+

  PURPOSE : This function checks whether inserted sim belong to 
  Cingular Network.cingular_plmn_list contains the entire mcc and mnc of
  Cingular Network. Issue 31179

*/

GLOBAL void check_if_cingular_sim(void)
{
  GET_INSTANCE_DATA;
  USHORT plmn_index;
  T_plmn   hplmn;
  reg_extract_hplmn (&hplmn);
  for(plmn_index = 0; plmn_index < MAX_CINGULAR_PLMN; plmn_index++)
  {
    if(reg_plmn_equal_sim (&hplmn, &cingular_plmn_list[plmn_index]))
    {
      mm_data->reg.is_cingular_sim = TRUE;
      break;
    }
  }
}
