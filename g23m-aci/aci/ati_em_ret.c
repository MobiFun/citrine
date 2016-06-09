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
|  Purpose :  This module provides the data preparation for the at-cmd output. 
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_EM_RET_C
#define ATI_EM_RET_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*===== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_mem.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "ati_cmd.h"

#include "aci.h"
#include "psa.h"

#ifdef GPRS
#include "gprs.h"
#endif 

#if (defined(FAX_AND_DATA) || defined(UART) || defined(GPRS))
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#include "cmh.h"
#include "cmh_snd.h"
#include "aci_em.h"
#include "cmh_em.h"
#ifdef GPRS
#include "gaci.h"
#include "gaci_cmh.h"
#include "cmh_sm.h"
#endif

#include "aci_lst.h"
#include "ati_int.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/
EXTERN void  em_aci_sem (UBYTE entity, UBYTE *buffer, UBYTE buf_index_tmp);

EXTERN SHORT em_relcs;

#if !defined (WIN32)
EXTERN CHAR* l1_version(void);
EXTERN CHAR* dl_version(void);
EXTERN CHAR* rr_version(void);
EXTERN CHAR* mm_version(void);
EXTERN CHAR* cc_version(void);
EXTERN CHAR* ss_version(void);
EXTERN CHAR* sim_version(void);
EXTERN CHAR* sms_version(void);
//EXTERN CHAR* aci_version(void);
#endif

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_OK             |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_OK call back

*/

GLOBAL void rCI_PercentEM ( T_EM_VAL * val_tmp )
{
  char *me="%EM: ";
  char *ver="%VER: ";

  int i=0, j=0;
  UBYTE srcId = srcId_cb;
#ifdef GPRS
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  UBYTE c_state,cid;
#endif

  TRACE_FUNCTION("rCI_PercentEM ()");

  switch(val_tmp->em_utype)
  {
     case EM_SUBCLASS_SC:
       sprintf(g_sa, "%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
               me, 
               val_tmp->em_u.em_sc_val.arfcn,    val_tmp->em_u.em_sc_val.c1, 
               val_tmp->em_u.em_sc_val.c2,       val_tmp->em_u.em_sc_val.rxlev, 
               val_tmp->em_u.em_sc_val.bsic,     val_tmp->em_u.em_sc_val.cell_id,
               val_tmp->em_u.em_sc_val.dsc, 
               val_tmp->em_u.em_sc_val.txlev,    val_tmp->em_u.em_sc_val.tn, 
               val_tmp->em_u.em_sc_val.rlt,      val_tmp->em_u.em_sc_val.tav, 
               val_tmp->em_u.em_sc_val.rxlev_f,  val_tmp->em_u.em_sc_val.rxlev_s, 
               val_tmp->em_u.em_sc_val.rxqual_f, val_tmp->em_u.em_sc_val.rxqual_s, 
               val_tmp->em_u.em_sc_val.lac,      val_tmp->em_u.em_sc_val.cba, 
               val_tmp->em_u.em_sc_val.cbq,      val_tmp->em_u.em_sc_val.cell_type_ind,
               val_tmp->em_u.em_sc_val.vocoder);
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);                           
       break;
#ifdef GPRS
     case EM_SUBCLASS_SC_GPRS:
       sprintf(g_sa, "%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
               me, 
               val_tmp->em_u.em_sc_gprs_val.tn,    val_tmp->em_u.em_sc_gprs_val.nmo, 
               val_tmp->em_u.em_sc_gprs_val.net_ctrl.spgc_ccch_sup,       
               val_tmp->em_u.em_sc_gprs_val.net_ctrl.priority_access_thr, 
               val_tmp->em_u.em_sc_gprs_val.cba,   val_tmp->em_u.em_sc_gprs_val.rac, 
               val_tmp->em_u.em_sc_gprs_val.tav,   val_tmp->em_u.em_sc_gprs_val.dsc, 
               val_tmp->em_u.em_sc_gprs_val.c31,   val_tmp->em_u.em_sc_gprs_val.c32,
               val_tmp->em_u.em_sc_gprs_val.nco);
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);                           
       break;
#endif /* GPRS */       
     case EM_SUBCLASS_NC:
       sprintf(g_sa, "%s%d", me, val_tmp->em_u.em_nc_val.no_ncells);
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);    

       sprintf(g_sa, "%d,%d,%d,%d,%d,%d", 
               val_tmp->em_u.em_nc_val.arfcn_nc[0],     val_tmp->em_u.em_nc_val.arfcn_nc[1],              
               val_tmp->em_u.em_nc_val.arfcn_nc[2],     val_tmp->em_u.em_nc_val.arfcn_nc[3],              
               val_tmp->em_u.em_nc_val.arfcn_nc[4],     val_tmp->em_u.em_nc_val.arfcn_nc[5]); 
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);   

       sprintf(g_sa, "%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.c1_nc[0],        val_tmp->em_u.em_nc_val.c1_nc[1],
               val_tmp->em_u.em_nc_val.c1_nc[2],        val_tmp->em_u.em_nc_val.c1_nc[3],
               val_tmp->em_u.em_nc_val.c1_nc[4],        val_tmp->em_u.em_nc_val.c1_nc[5]);
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);                           

       sprintf(g_sa, "%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.c2_nc[0],        val_tmp->em_u.em_nc_val.c2_nc[1],                 
               val_tmp->em_u.em_nc_val.c2_nc[2],        val_tmp->em_u.em_nc_val.c2_nc[3],                 
               val_tmp->em_u.em_nc_val.c2_nc[4],        val_tmp->em_u.em_nc_val.c2_nc[5]); 
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);                           

       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.rxlev_nc[0],     val_tmp->em_u.em_nc_val.rxlev_nc[1],
               val_tmp->em_u.em_nc_val.rxlev_nc[2],     val_tmp->em_u.em_nc_val.rxlev_nc[3],
               val_tmp->em_u.em_nc_val.rxlev_nc[4],     val_tmp->em_u.em_nc_val.rxlev_nc[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);                           

       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.bsic_nc[0],      val_tmp->em_u.em_nc_val.bsic_nc[1],
               val_tmp->em_u.em_nc_val.bsic_nc[2],      val_tmp->em_u.em_nc_val.bsic_nc[3],
               val_tmp->em_u.em_nc_val.bsic_nc[4],      val_tmp->em_u.em_nc_val.bsic_nc[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);                           

       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.cell_id_nc[0],   val_tmp->em_u.em_nc_val.cell_id_nc[1],
               val_tmp->em_u.em_nc_val.cell_id_nc[2],   val_tmp->em_u.em_nc_val.cell_id_nc[3],
               val_tmp->em_u.em_nc_val.cell_id_nc[4],   val_tmp->em_u.em_nc_val.cell_id_nc[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);   

       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.lac_nc[0],       val_tmp->em_u.em_nc_val.lac_nc[1],
               val_tmp->em_u.em_nc_val.lac_nc[2],       val_tmp->em_u.em_nc_val.lac_nc[3],
               val_tmp->em_u.em_nc_val.lac_nc[4],       val_tmp->em_u.em_nc_val.lac_nc[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);                           

       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.frame_offset[0], val_tmp->em_u.em_nc_val.frame_offset[1],
               val_tmp->em_u.em_nc_val.frame_offset[2], val_tmp->em_u.em_nc_val.frame_offset[3],
               val_tmp->em_u.em_nc_val.frame_offset[4], val_tmp->em_u.em_nc_val.frame_offset[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);                           

       sprintf(g_sa,"%ld,%ld,%ld,%ld,%ld,%ld",
               val_tmp->em_u.em_nc_val.time_alignmt[0], val_tmp->em_u.em_nc_val.time_alignmt[1],
               val_tmp->em_u.em_nc_val.time_alignmt[2], val_tmp->em_u.em_nc_val.time_alignmt[3],
               val_tmp->em_u.em_nc_val.time_alignmt[4], val_tmp->em_u.em_nc_val.time_alignmt[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);                           

       sprintf(g_sa,"%ld,%ld,%ld,%ld,%ld,%ld",
               val_tmp->em_u.em_nc_val.cba_nc[0],       val_tmp->em_u.em_nc_val.cba_nc[1],
               val_tmp->em_u.em_nc_val.cba_nc[2],       val_tmp->em_u.em_nc_val.cba_nc[3],
               val_tmp->em_u.em_nc_val.cba_nc[4],       val_tmp->em_u.em_nc_val.cba_nc[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);                           

       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.cbq_nc[0],       val_tmp->em_u.em_nc_val.cbq_nc[1],
               val_tmp->em_u.em_nc_val.cbq_nc[2],       val_tmp->em_u.em_nc_val.cbq_nc[3],
               val_tmp->em_u.em_nc_val.cbq_nc[4],       val_tmp->em_u.em_nc_val.cbq_nc[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);

       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.cell_type_ind[0], val_tmp->em_u.em_nc_val.cell_type_ind[1],
               val_tmp->em_u.em_nc_val.cell_type_ind[2], val_tmp->em_u.em_nc_val.cell_type_ind[3],
               val_tmp->em_u.em_nc_val.cell_type_ind[4], val_tmp->em_u.em_nc_val.cell_type_ind[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);   
	   
       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.rac[0],       val_tmp->em_u.em_nc_val.rac[1],
               val_tmp->em_u.em_nc_val.rac[2],       val_tmp->em_u.em_nc_val.rac[3],
               val_tmp->em_u.em_nc_val.rac[4],       val_tmp->em_u.em_nc_val.rac[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);                           
	   
       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.cell_resel_offset[0], val_tmp->em_u.em_nc_val.cell_resel_offset[1],
               val_tmp->em_u.em_nc_val.cell_resel_offset[2], val_tmp->em_u.em_nc_val.cell_resel_offset[3],
               val_tmp->em_u.em_nc_val.cell_resel_offset[4], val_tmp->em_u.em_nc_val.cell_resel_offset[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
	   
       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.temp_offset[0], val_tmp->em_u.em_nc_val.temp_offset[1],
               val_tmp->em_u.em_nc_val.temp_offset[2], val_tmp->em_u.em_nc_val.temp_offset[3],
               val_tmp->em_u.em_nc_val.temp_offset[4], val_tmp->em_u.em_nc_val.temp_offset[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);   

       sprintf(g_sa,"%d,%d,%d,%d,%d,%d",
               val_tmp->em_u.em_nc_val.rxlev_acc_min[0], val_tmp->em_u.em_nc_val.rxlev_acc_min[1],
               val_tmp->em_u.em_nc_val.rxlev_acc_min[2], val_tmp->em_u.em_nc_val.rxlev_acc_min[3],
               val_tmp->em_u.em_nc_val.rxlev_acc_min[4], val_tmp->em_u.em_nc_val.rxlev_acc_min[5]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);   
       break;
     case EM_SUBCLASS_LOC_PAG:
       sprintf(g_sa,"%s%d,%d,%d%d%d,%d%d%d,%ld",
               me, 
               val_tmp->em_u.em_loc_val.bs_pa_mfrms, val_tmp->em_u.em_loc_val.t3212, 
               val_tmp->em_u.em_loc_val.mcc[0], val_tmp->em_u.em_loc_val.mcc[1], val_tmp->em_u.em_loc_val.mcc[2], 
               val_tmp->em_u.em_loc_val.mnc[0], val_tmp->em_u.em_loc_val.mnc[1], val_tmp->em_u.em_loc_val.mnc[2], 
               val_tmp->em_u.em_loc_val.tmsi);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       break;
     case EM_SUBCLASS_PLMN:
       sprintf(g_sa,"%s%d,%d,%d,%d,%d",
               me, 
               val_tmp->em_u.em_plmn_val.no_creq_max, val_tmp->em_u.em_plmn_val.reest_flag,
               val_tmp->em_u.em_plmn_val.txpwr_max,   val_tmp->em_u.em_plmn_val.rxlev_min,
               em_relcs);   
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);       
       break;
     case EM_SUBCLASS_CIPH_HOP_DTX:   
       sprintf(g_sa,"%s%d,%d,%d,%d,%d,%d", 
               me,
               val_tmp->em_u.em_cip_val.ciph_stat,
               val_tmp->em_u.em_cip_val.hop,
               val_tmp->em_u.em_cip_val.arfcn,
               val_tmp->em_u.em_cip_val.hsn,
               val_tmp->em_u.em_cip_val.dtx_stat,
               val_tmp->em_u.em_cip_val.v_start);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);

       if(val_tmp->em_u.em_cip_val.hop)
       {
         sprintf(g_sa,"%d,%d", 
                 val_tmp->em_u.em_cip_val.hop_chn.maio,
                 val_tmp->em_u.em_cip_val.hop_chn.nr_arfcns);
         io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);

         i=0;
         j=0;
         /* The while loop exits when the content of ma[] equals 0xFFFF and the index exceeds the value 63 */
         while(val_tmp->em_u.em_cip_val.hop_chn.ma[j] NEQ NOT_PRESENT_16BIT AND j<EM_MAX_MA_CHANNELS)
         {
           i+=sprintf(g_sa+i, "%d,", val_tmp->em_u.em_cip_val.hop_chn.ma[j++]);
           if (i>(int)sizeof(g_sa)-20)  /* Check if we reach the buffer limit */
           {
             io_sendMessage(srcId, g_sa,ATI_ECHO_OUTPUT);  /* and send the data in chunks w/o CRLF */
             i=0;
             g_sa[0]='\0';
           }
         }
         if(i>0)
         {
           g_sa[i-1]='\0'; 
           io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
         } 

         if(val_tmp->em_u.em_cip_val.v_start)
         { 
           sprintf(g_sa,"%d,%d", 
                   val_tmp->em_u.em_cip_val.hop_chn2.maio,
                   val_tmp->em_u.em_cip_val.hop_chn2.nr_arfcns);
           io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);

           i=0;
           j=0;
           /* The while loop exits when the content of ma[] equals 0xFFFF and the index exceeds the value 63 */
           while(val_tmp->em_u.em_cip_val.hop_chn2.ma[j] NEQ NOT_PRESENT_16BIT AND j<EM_MAX_MA_CHANNELS) 
           { 
             i+=sprintf(g_sa+i, "%d,", val_tmp->em_u.em_cip_val.hop_chn2.ma[j++]);
             if (i>(int)sizeof(g_sa)-20)
             {
               io_sendMessage(srcId, g_sa,ATI_ECHO_OUTPUT);
               i=0;
               g_sa[0]='\0';
             } 
           } 
           if(i>0)
           { 
             g_sa[i-1]='\0';
             io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
           } 
         } 
       }  /* hopping is configured */

     break;
     case EM_SUBCLASS_POWER:
       sprintf(g_sa,"%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
               me,
               val_tmp->em_u.em_power_val.classm2.rev_lev,  val_tmp->em_u.em_power_val.classm2.es_ind,
               val_tmp->em_u.em_power_val.classm2.a5_1,     val_tmp->em_u.em_power_val.classm2.rf_pow_cap,
               val_tmp->em_u.em_power_val.classm2.ps_cap,   val_tmp->em_u.em_power_val.classm2.ss_screen,
               val_tmp->em_u.em_power_val.classm2.sm_cap,   val_tmp->em_u.em_power_val.classm2.freq_cap,
               val_tmp->em_u.em_power_val.classm2.class_3,  val_tmp->em_u.em_power_val.classm2.cmsp,
               val_tmp->em_u.em_power_val.classm2.a5_3,     val_tmp->em_u.em_power_val.classm2.a5_2);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       
       sprintf(g_sa,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
               val_tmp->em_u.em_power_val.classm3.mb_sub,           val_tmp->em_u.em_power_val.classm3.a5_7,
               val_tmp->em_u.em_power_val.classm3.a5_6,             val_tmp->em_u.em_power_val.classm3.a5_5,
               val_tmp->em_u.em_power_val.classm3.a5_4,             val_tmp->em_u.em_power_val.classm3.v_radio_cap_2,
               val_tmp->em_u.em_power_val.classm3.radio_cap_2,      val_tmp->em_u.em_power_val.classm3.v_radio_cap_1,
               val_tmp->em_u.em_power_val.classm3.radio_cap_1,      val_tmp->em_u.em_power_val.classm3.v_r_support,
               val_tmp->em_u.em_power_val.classm3.r_support,        val_tmp->em_u.em_power_val.classm3.v_m_s_class,
               val_tmp->em_u.em_power_val.classm3.m_s_class,        val_tmp->em_u.em_power_val.classm3.ucs2_treat,
               val_tmp->em_u.em_power_val.classm3.ext_meas_cap,     val_tmp->em_u.em_power_val.classm3.v_meas_cap,
               val_tmp->em_u.em_power_val.classm3.meas_cap.sw_time, val_tmp->em_u.em_power_val.classm3.meas_cap.sws_time);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       break;
     case EM_SUBCLASS_ID:
       i=0;
       i=sprintf(g_sa,"%s%d,%d,%d,%d,",
                 me, 
                 val_tmp->em_u.em_id_val.em_imeisv.ident_type,
                 val_tmp->em_u.em_id_val.em_imeisv.odd_even, 
                 val_tmp->em_u.em_id_val.em_imeisv.v_ident_dig, 
                 val_tmp->em_u.em_id_val.em_imeisv.c_ident_dig); 
       for(j=0; j<15; j++) {
         i+=sprintf(g_sa+i,"%d,", val_tmp->em_u.em_id_val.em_imeisv.ident_dig[j]);
       }
         i+=sprintf(g_sa+i,"%d", val_tmp->em_u.em_id_val.em_imeisv.ident_dig[15]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT); 
       
       i=0;
       i=sprintf(g_sa,"%d,%d,%d,%d,",
                 val_tmp->em_u.em_id_val.em_imsi.ident_type,
                 val_tmp->em_u.em_id_val.em_imsi.odd_even, 
                 val_tmp->em_u.em_id_val.em_imsi.v_ident_dig, 
                 val_tmp->em_u.em_id_val.em_imsi.c_ident_dig); 
       for(j=0; j<15; j++) {
         i+=sprintf(g_sa+i,"%d,", val_tmp->em_u.em_id_val.em_imsi.ident_dig[j]);
       }
         i+=sprintf(g_sa+i,"%d", val_tmp->em_u.em_id_val.em_imsi.ident_dig[15]);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT); 

       sprintf(g_sa,"%ld", val_tmp->em_u.em_id_val.tmsi);
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       break;
     case EM_SUBCLASS_SW_VERSION:
#if !defined (WIN32)
       sprintf(g_sa,"%s%s",ver,l1_version());
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,dl_version());
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,rr_version());
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,mm_version());
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,cc_version());
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,ss_version());
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,sim_version());
       io_sendMessage(srcId, g_sa,ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,sms_version());
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
#else
       sprintf(g_sa,"%s%s",ver,"ALR");
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,"DL");
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,"RR");
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,"MM");
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,"CC");
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,"SMS");
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,"SS");
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
       sprintf(g_sa,"%s%s",ver,"ACI");
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
#endif /* WIN 32 */
       break;
#ifdef GPRS
     case EM_SUBCLASS_GMM: /*SAP needed*/
       sprintf(g_sa, "%s%d,%u,%u,%u,%u,%d,%d,%d", 
               me, 
               val_tmp->em_u.em_gmm_val.ready_state,    
               val_tmp->em_u.em_gmm_val.tlli, 
               val_tmp->em_u.em_gmm_val.ptmsi,
               val_tmp->em_u.em_gmm_val.ptmsi_sig,
               val_tmp->em_u.em_gmm_val.ready_timer, 
               val_tmp->em_u.em_gmm_val.ciphering_algorithm,
               val_tmp->em_u.em_gmm_val.t3312.t3312_deactivated,
               val_tmp->em_u.em_gmm_val.t3312.t3312_val);
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);                           
       break;
      case EM_SUBCLASS_GRLC:
       sprintf(g_sa, "%s%d,%d,%d,%d,%d,%d,%d,%d,%d", 
               me, 
               val_tmp->em_u.em_grlc_val.grlc_state,
               val_tmp->em_u.em_grlc_val.tbf_mod, 
               val_tmp->em_u.em_grlc_val.ul_tbf_par.tfi,
               val_tmp->em_u.em_grlc_val.ul_tbf_par.mac_mod, 
               val_tmp->em_u.em_grlc_val.ul_tbf_par.ul_nb_block,
               val_tmp->em_u.em_grlc_val.ul_tbf_par.cv, 
               val_tmp->em_u.em_grlc_val.ul_tbf_par.cs,
               val_tmp->em_u.em_grlc_val.dl_tbf_par.tfi, 
               val_tmp->em_u.em_grlc_val.dl_tbf_par.mac_mod);
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);                           
       break;
#endif /*GPRS*/
      case EM_SUBCLASS_AMR:
       sprintf(g_sa, "%s%d,%d,%d,%d,%d,%d,", 
               me, 
               val_tmp->em_u.em_amr_val.amr_vocoder,
               val_tmp->em_u.em_amr_val.amr_icmi, 
               val_tmp->em_u.em_amr_val.amr_icm,
               val_tmp->em_u.em_amr_val.amr_acs,
               val_tmp->em_u.em_amr_val.amr_first_codec,
               val_tmp->em_u.em_amr_val.amr_nr_modes);

#ifdef _SIMULATION_
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT); /* Simulation needs CRLF */
#else
       io_sendMessage(srcId, g_sa, ATI_ECHO_OUTPUT); /* send without CRLF */
#endif

       sprintf(g_sa, "%d,%d,%d,%d,%d,%d", 
               val_tmp->em_u.em_amr_val.amr_cod_prop[0].amr_codec_thr,  val_tmp->em_u.em_amr_val.amr_cod_prop[0].amr_codec_hyst,              
               val_tmp->em_u.em_amr_val.amr_cod_prop[1].amr_codec_thr,  val_tmp->em_u.em_amr_val.amr_cod_prop[1].amr_codec_hyst,
               val_tmp->em_u.em_amr_val.amr_cod_prop[2].amr_codec_thr,  val_tmp->em_u.em_amr_val.amr_cod_prop[2].amr_codec_hyst); 
       io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
       break;
#ifdef GPRS
      case EM_SUBCLASS_PDP:
        sprintf(g_sa, "%s%d", me,MAX_CID);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

        for(cid=1;cid<=MAX_PDP_CTXT;cid++)
        {
          c_state = get_state_over_cid( cid );
         
          p_pdp_context_node = pdp_context_find_node_from_cid( cid );
         
          if((c_state EQ PDP_CONTEXT_STATE_INVALID) OR (p_pdp_context_node EQ NULL))

            sprintf(g_sa, "%d", c_state);
          else
          {
            if ( p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ NAS_is_ipv4 )
            {
              sprintf(g_sa, "%d,%s,%s,%s", 
                     c_state,
                     p_pdp_context_node->attributes.pdp_type,
                     p_pdp_context_node->attributes.pdp_apn, 
                     ((c_state EQ PDP_CONTEXT_STATE_INVALID) OR (c_state EQ PDP_CONTEXT_STATE_DATA_LINK))
                     ? p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv4_addr.a4 : 
                       p_pdp_context_node->attributes.pdp_addr.ip_address.ipv4_addr.a4);
          
            }
            else if ( p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ NAS_is_ipv6 )
            {
              sprintf(g_sa, "%d,%s,%s,%s", 
                     c_state,
                     p_pdp_context_node->attributes.pdp_type,
                     p_pdp_context_node->attributes.pdp_apn, 
                     ((c_state EQ PDP_CONTEXT_STATE_INVALID) OR (c_state EQ PDP_CONTEXT_STATE_DATA_LINK))
                     ? p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv6_addr.a6 : 
                       p_pdp_context_node->attributes.pdp_addr.ip_address.ipv6_addr.a6);
            }
          } 
          io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
          
        } /* MAX_CID */
              
      break;
#endif
     default:
       break;
  } /* switch */
}

GLOBAL void rCI_PercentEMET ( /*UBYTE srcId,*/ T_DRV_SIGNAL_EM_EVENT * Signal ) 
{
  int i=0, j=0;
  UBYTE srcId = srcId_cb;
  
  TRACE_FUNCTION("rCI_PercentEMET ()");

  if (Signal->Data.alr.length NEQ 0)
  {
    char *me="%EMET L1: ";

    i+=sprintf(g_sa+i, "%s",me);
    for(j=0; j<Signal->Data.alr.length; j++) {
      i+=sprintf(g_sa+i,"%d,", *(Signal->Pointer++));
    }

    i+=sprintf(g_sa+i,"%s", "FF");
  }
  if (Signal->Data.dl.length NEQ 0)
  {
    char *me="%EMET DL: ";
    
    i+=sprintf(g_sa+i, "%s",me);
    for(j=0; j<Signal->Data.dl.length; j++) {
      i+=sprintf(g_sa+i,"%d,", *(Signal->Pointer++));
    }

    i+=sprintf(g_sa+i,"%s", "FF");
  }
  if (Signal->Data.rr.length NEQ 0)
  {
    char *me="%EMET RR: ";
    
    i+=sprintf(g_sa+i, "%s",me);
    for(j=0; j<Signal->Data.rr.length; j++) {
      i+=sprintf(g_sa+i,"%d,", *(Signal->Pointer++));
    }
    
    i+=sprintf(g_sa+i,"%s", "FF");
  }
  if (Signal->Data.mm.length NEQ 0)
  {
    char *me="%EMET MM: ";
    
    i+=sprintf(g_sa+i, "%s",me);
    for(j=0; j<Signal->Data.mm.length; j++) {
      i+=sprintf(g_sa+i,"%d,", *(Signal->Pointer++));
    }

    i+=sprintf(g_sa+i,"%s", "FF");
  }
  if (Signal->Data.cc.length NEQ 0)
  {
    char *me="%EMET CC: ";
    
    i+=sprintf(g_sa+i, "%s",me);
    for(j=0; j<Signal->Data.cc.length; j++) {
      i+=sprintf(g_sa+i,"%d,", *(Signal->Pointer++));
    }

    i+=sprintf(g_sa+i,"%s", "FF");
  }
  if (Signal->Data.ss.length NEQ 0)
  {
    char *me="%EMET SS: ";
      
    i+=sprintf(g_sa+i, "%s",me);
    for(j=0; j<Signal->Data.ss.length; j++) {
      i+=sprintf(g_sa+i,"%d,", *(Signal->Pointer++));
    }

    i+=sprintf(g_sa+i,"%s", "FF");
  }
  if (Signal->Data.sms.length NEQ 0)
  {
    char *me="%EMET SMS: ";
      
    i+=sprintf(g_sa+i, "%s",me);
    for(j=0; j<Signal->Data.sms.length; j++) {
      i+=sprintf(g_sa+i,"%d,", *(Signal->Pointer++));
    }

    i+=sprintf(g_sa+i,"%s", "FF");
  }
  if (Signal->Data.sim.length NEQ 0)
  {
    char *me="%EMET SIM: ";
      
    i+=sprintf(g_sa+i, "%s",me);
    for(j=0; j<Signal->Data.sim.length; j++) {
      i+=sprintf(g_sa+i,"%d,", *(Signal->Pointer++));
    }

    i+=sprintf(g_sa+i,"%s", "FF");
  }
  if (i EQ 0) 
  {
    char *me="%EMET END";
    i=sprintf(g_sa, "%s",me);
  }
  
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);                               
  
} /* rCI_PercentEMET */


GLOBAL void rCI_PercentEMETS ( UBYTE entity )
{
  TRACE_FUNCTION("rCI_PercentEMETS ()");
}

/*+++++++++++++++++++++++++++++++++++++++++ E O F +++++++++++++++++++++++++++++++++++++++++*/

