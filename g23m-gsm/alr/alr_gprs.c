/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  ALR_GPRS
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
|  Purpose :  This module implements the necessary funtionality
|             for switching to and from GPRS mode and also
|             the handling if determining if GPRS is supported
|             by the cell.
+-----------------------------------------------------------------------------
*/
#ifndef ALR_GPRS_C
#define ALR_GPRS_C


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
#include "alr_gprs.h"
#define TRACING
#include "alr.h"
#define TRACING

#define TRACING

#if defined (TRACING)
#define ALR_TRACE_GPRS(a)  ALR_TRACE(a)
#else
#define ALR_TRACE_GPRS(a)
#endif

LOCAL BOOL gprs_read_si13                (UBYTE               si13_position);
LOCAL void gprs_alr_nc_enter_ptm         (void);
LOCAL void gprs_alr_tb_meas_ind          (T_TB_MEAS_IND*      report);

LOCAL const T_FUNC tb_table[] = {
  MAK_FUNC_0( gprs_alr_tb_meas_ind, TB_MEAS_IND ) /* 0 */
};

#define ONLY_BCC                 7
static UBYTE gprs_support = 0;

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_get_table         |
+--------------------------------------------------------------------+

  PURPOSE :

*/
void gprs_alr_get_table(const T_FUNC **tab, USHORT *n)
{
    *tab = tb_table;
    *n   = TAB_SIZE (tb_table);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_mon_ctrl_req      |
+--------------------------------------------------------------------+

  PURPOSE :

*/
/*
 * required for TESTMODE A/B
 */
EXTERN UBYTE grlc_test_mode_active(void);
void gprs_alr_mon_ctrl_req(T_MPH_MON_CTRL_REQ* ctrl_req)
{
  GET_INSTANCE_DATA;
  ALR_TRACE_GPRS("mon_ctrl_req");

  switch(GET_STATE(STATE_MA))
  {
    case MA_CELL_SELECTION:
      switch(ctrl_req->action)
      {
        case START_MON_EBCCH:
          if(ctrl_req->si_to_read EQ UPDATE_SI13)
          {
            PALLOC(req, MPHC_SCELL_EBCCH_REQ);
            req->schedule_array_size                 = 1;
            req->schedule_array[0].modulus           = 8;
            req->schedule_array[0].relative_position = 0;

            ma_scell_ebcch_req(req);
          }
          break;
        default:
          break;
      }
      break;
    case MA_CELL_RESELECTION:
      switch(ctrl_req->action)
      {
        case START_MON_EBCCH:
          /*for SI13 reading control*/
          if(ctrl_req->si_to_read EQ UPDATE_SI13)
          {
            PALLOC(req, MPHC_SCELL_EBCCH_REQ);
            req->schedule_array_size                 = 1;
            req->schedule_array[0].modulus           = 8;
            req->schedule_array[0].relative_position = 0;

            ma_scell_ebcch_req(req);
          }
          break;
        default:
          break;
      }
      break;
    case MA_IDLE:
      switch(ctrl_req->action)
      {
        case START_MON_EBCCH:
          /*for SI13 reading control*/
          if(ctrl_req->si_to_read EQ UPDATE_SI13)
          {
            PALLOC(req, MPHC_SCELL_EBCCH_REQ);
            req->schedule_array_size                 = 1;
            req->schedule_array[0].modulus           = 8;
            req->schedule_array[0].relative_position = 0;

            ma_scell_ebcch_req(req);
          }
          break;
        case START_MON_NBCCH:
          {
            USHORT  si_mask = 0;
            UBYTE   tc      = 0;  /* refer to GSM Spec 05.02, clause 6.3.1.3 Mapping of BCCH data */
            UBYTE   i,k;

            switch(ctrl_req->si_to_read)
            {
              case UPDATE_SI13:
                gprs_read_si13(SI13_ON_NBCCH);
                break;

              case UPDATE_SI13_GSM:
              {
                PALLOC(scell_bcch_req, MPHC_SCELL_NBCCH_REQ);

                scell_bcch_req->schedule_array_size                 = 1;
                scell_bcch_req->schedule_array[0].modulus           = 8;
                scell_bcch_req->schedule_array[0].relative_position = 4;
                ma_scell_nbcch_req(scell_bcch_req);
              }
              break;

              case UNSPECIFIED_SI:
              case COMPLETE_SI:
                ma_clean_sys_buffer (IND_ALL_IDLE_SI);
                ma_scell_full_nbcch();
                break;

              case UPDATE_SI1:
                si_mask = IND_SI_1;
                tc = (1<<0);
                break;
              case UPDATE_SI2_SI2BIS_OR_SI2TER:
                si_mask = IND_SI_2 | IND_SI_2BIS | IND_SI_2TER;
                tc = (1<<1) | (1<<4) | (1<<5);
#if defined (REL99) AND defined (TI_PS_FF_EMR)
                /*This update indication is for SI-2quater also, so we need
                  to configure it after SI-2/2bis/2ter are received*/
                alr_data->nc_data.si2_count = 1;
                /*check whether we have valid 2bis info */
                if (alr_data->ma_data.sys_info_2bis[1] EQ  D_SYS_INFO_2BIS)
                  alr_data->nc_data.si2_count++;
                /*check whether we have valid 2ter info */
                if (alr_data->ma_data.sys_info_2ter[1] EQ  D_SYS_INFO_2TER)
                  alr_data->nc_data.si2_count++;  
#endif
                break;
              case UPDATE_SI3_SI4_SI7_OR_SI8:
                si_mask = IND_SI_3 | IND_SI_4;
                tc = (1<<2) | (1<<3) | (1<<6) | (1<<7);
                break;
              case UPDATE_SI9:
                break;
            }

            if ( si_mask )
            {
              PALLOC(scell_bcch_req, MPHC_SCELL_NBCCH_REQ);
              k = 0;
              scell_bcch_req->schedule_array_size = k;
              for ( i = 0; i < 8; i++ )
              {
                if ( (tc & (1 << i)) != 0 )
                {
                  scell_bcch_req->schedule_array[k].modulus = 8;
                  scell_bcch_req->schedule_array[k].relative_position = i;
                  k++;
                  scell_bcch_req->schedule_array_size = k;
                }
              }
              ma_clean_sys_buffer ( si_mask );
              ma_scell_nbcch_req ( scell_bcch_req );
            }
          }
          break;
        case STOP_MON_CCCH:
          /* PBCCH is present, stop all activities */
          /*ma_stop_active_procs(STOP_PCH_READING | STOP_MEASUREMENTS);*/
          pch_stop();
          /*alr_data->gprs_data.pbcch = 1;*/
          break;
        case START_MON_CCCH:
          if(alr_data->gprs_data.pbcch EQ TRUE)
          {
            pch_configure (NULL, PGM_NORMAL);
            pch_save_pgm(PGM_NORMAL); /* reset saved pgm to REORG_CS */
            pch_start_ccch_req();
          }
          break;

        case LEAVING_PIM_PBCCH:
          /* we are about to enter PAM or PTM */
          if(alr_data->gprs_data.pbcch EQ TRUE)
          {
            TRACE_EVENT("leave pim");
            alr_data->gprs_data.pim = FALSE;
            pch_stop();
            cb_stop();
            nc_suspend();
          }
          else
          {
            TRACE_EVENT("huch pim");
          }
          break;
        case LEAVING_PAM_PBCCH:
          /* we are about to enter PTM or PIM */
          if(alr_data->gprs_data.pbcch EQ TRUE)
          {
            /* do nothing, nothing should be active */
            alr_data->gprs_data.pim = FALSE;
            TRACE_EVENT("leave pam");
          }
          else
          {
            TRACE_EVENT("huch pam");
          }
          break;
        case LEAVING_PTM_PBCCH:
          /* we are about to enter PIM */
          if(alr_data->gprs_data.pbcch EQ TRUE)
          {
            TRACE_EVENT("leave ptm");
            alr_data->gprs_data.pim = FALSE;
            pch_stop();
            nc_suspend();
            alr_data->gprs_data.ptm = FALSE;
          }
          else
          {
            TRACE_EVENT("huch ptm");
          }
          break;
        case ENTER_PTM_PBCCH:
          if(alr_data->gprs_data.pbcch EQ TRUE)
          {
            alr_data->gprs_data.ptm = TRUE;
            alr_data->gprs_data.pim = FALSE;
            nc_start_pbcch();
          }
          break;
        case ENTER_PIM_PBCCH:
          if(alr_data->gprs_data.pbcch EQ TRUE)
          {
            alr_data->gprs_data.pim = TRUE;
            alr_data->gprs_data.ptm = FALSE;
            nc_start_pbcch();
            cb_start();
          }
          break;
        case ENTER_PTM_BCCH:
          /*
           * convert counters and set NC process to IDLE
           */
          alr_data->gprs_data.pim = FALSE;
          gprs_alr_nc_enter_ptm();
          break;
        default:
          break;
      }
      break;
    case MA_PTM:
      switch(ctrl_req->action)
      {
        case START_MON_EBCCH:
          /*for SI13 reading control*/
          if(ctrl_req->si_to_read EQ UPDATE_SI13)
          {
            PALLOC(req, MPHC_SCELL_EBCCH_REQ);
            req->schedule_array_size                 = 1;
            req->schedule_array[0].modulus           = 8;
            req->schedule_array[0].relative_position = 0;

            ma_scell_ebcch_req(req);
          }
          break;
        case START_MON_NBCCH:
          gprs_read_si13(SI13_ON_NBCCH);
          break;
        case START_MON_CCCH:
          pch_configure (NULL, PGM_NORMAL);
          if(alr_data->gprs_data.pbcch EQ TRUE)
            pch_save_pgm(PGM_NORMAL); /* reset saved pgm to REORG_CS */
          else
          {
            pch_save_pgm(PGM_REORG_CS); /* reset saved pgm to REORG_CS */
            alr_data->pch_data.last_start_ccch_req.bs_pa_mfrms = NOT_PRESENT_8BIT;
          }

          pch_start_ccch_req();
          break;
         /*
          * This case is necessary to stop ccch monitoring in MA_PTM
          * This is required for Testmode A/B
          */
         case STOP_MON_CCCH:
           if(grlc_test_mode_active())
           {
             ma_stop_active_procs(STOP_PCH_READING | STOP_MEASUREMENTS);
             TRACE_EVENT("STOP_MON_CCCH: TESTMODE A/B is running !!!!");
           }
          break;
         case ENTER_PTM_BCCH:
          /*
           * convert counters and set NC process to IDLE
           */
          gprs_alr_nc_enter_ptm();
          break;
        default:
          break;
      }
      break;

    case MA_CON_EST:
      switch(ctrl_req->action)
      {
        case ENTER_PTM_BCCH:
          /*
           * convert counters and set NC process to IDLE
           */
          gprs_alr_nc_enter_ptm();
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  PFREE (ctrl_req);

}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_check_packet_paging
+--------------------------------------------------------------------+

  PURPOSE :

*/

#define P1_REST_OCTET_LEN     5
#define GSM408_SPARE_PADDING   0x2b

/*lint -e749 (Info -- local enumeration constant not referenced) */
enum t_p1_ie {  NLN = 0,
                PRIO1,
                PRIO2,
                GRCI,
                PPI1,
                PPI2
};
/*lint +e749 (Info -- local enumeration constant not referenced) */

#define NEXT_BIT(bit_pos,byte_pos) if( (bit_pos >>=1) == 0 ) { bit_pos=0x80; byte_pos++;}

LOCAL enum t_p1_ie p1_ie;
LOCAL UBYTE element_size[PPI2+1] = {
   3,
   3,
   3,
   0,
   1,
   1
 };



BOOL gprs_alr_check_packet_paging(UBYTE* frame, UBYTE which)
{
  UBYTE  byte_pos = 0;
  UBYTE  bit  = 0x80;
  UBYTE *frame_ptr = frame;

  ALR_TRACE_GPRS("check packet paging");
  /* point after Mobile Identity 1 */
  frame = frame + 4 + frame[4] + 1;
  /* check if Mobile Identity 2 is present */
  if (frame[0] EQ 0x17)
    /* skip Mobile Identity 2 */
    /* rest octets = start of mob 2 + len of mob 2 + TL */
    frame= frame + frame[1] + 2;

  if( frame - frame_ptr >= MAX_L2_FRAME_SIZE )
    return FALSE;

  /* decode packet paging */
  for(p1_ie = NLN; p1_ie < PPI2+1;p1_ie++)
  {
    if((frame[byte_pos] & bit) EQ
       (GSM408_SPARE_PADDING & bit))
    {
      /* L:
       *   - element is not used
       *   - check next bit for next element
       */
      if(p1_ie NEQ (which+GRCI))
      {
        NEXT_BIT(bit,byte_pos);/*lint!e720 (Info -- Boolean test of assignment) */
      }
      else
      {
        ALR_TRACE_GPRS("no packet");
        return FALSE;
      }
    }
    else
    {
      /* H:
       *   - element is used
       *   - skip the bits used by this element
       *     except PPIx
       */
      ALR_TRACE_GPRS("element used");
      if(p1_ie NEQ (which+GRCI))
      {
        UBYTE i;
        for(i=element_size[p1_ie]+1; i > 0; i--)
        {
          NEXT_BIT(bit,byte_pos);/*lint!e720 (Info -- Boolean test of assignment) */
        }
      }
      else
      {
        ALR_TRACE_GPRS("packet paging");
        return TRUE;
      }
    }
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_check_packet_paging_2
+--------------------------------------------------------------------+

  PURPOSE :

*/

/*lint -e749 (Info -- local enumeration constant not referenced) */
enum t_p2_ie {
  p2CN3 = 0,
  p2NLN,
  p2PRIO1,
  p2PRIO2,
  p2PRIO3,
  p2PPI3
};
/*lint -e749 (Info -- local enumeration constant not referenced) */

LOCAL enum t_p2_ie p2_ie;
LOCAL UBYTE p2_element_size[p2PPI3+1] = {
  2,
  3,
  3,
  3,
  3,
  1
};

BOOL gprs_alr_check_packet_paging_2(UBYTE* frame, UBYTE which)
{
  UBYTE byte_pos = 0;
  UBYTE bit  = 0x80;

  if(which NEQ 3) return FALSE;
  ALR_TRACE_GPRS("check packet paging 2");
  frame += frame[13]+14;
  /* decode packet paging */
  for(p2_ie = p2CN3; p2_ie < p2PPI3+1;p2_ie++)
  {
    if((frame[byte_pos] & bit) EQ
       (GSM408_SPARE_PADDING & bit))
    {
      /* L:
       *   - element is not used
       *   - check next bit for next element
       */
      if(p2_ie NEQ p2PPI3)
      {
        NEXT_BIT(bit,byte_pos);/*lint!e720 (Info -- Boolean test of assignment) */
      }
      else
      {
        ALR_TRACE_GPRS("no packet");
        return FALSE;
      }
    }
    else
    {
      /* H:
       *   - element is used
       *   - skip the bits used by this element
       *     except PPIx
       */
      ALR_TRACE_GPRS("element used");
      if(p2_ie NEQ p2PPI3)
      {
        UBYTE i;
        for(i=p2_element_size[p2_ie]+1; i > 0; i--)
        {
          NEXT_BIT(bit,byte_pos);/*lint!e720 (Info -- Boolean test of assignment) */
        }
      }
      else
      {
        ALR_TRACE_GPRS("packet paging");
        return TRUE;
      }
    }
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_store_ptmsi       |
+--------------------------------------------------------------------+

  PURPOSE :

*/
void gprs_alr_store_ptmsi(UBYTE indic, ULONG tmsi)
{
  GET_INSTANCE_DATA;
  alr_data->gprs_data.v_ptmsi = indic;
  alr_data->gprs_data.ptmsi   = tmsi;
  TRACE_EVENT_P2("v: %d ptmsi: %x", indic, tmsi);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_store_ptmsi       |
+--------------------------------------------------------------------+

  PURPOSE : Store the candidate PTMSI

*/
void gprs_alr_store_ptmsi2(UBYTE indic2, ULONG tmsi2)
{
  GET_INSTANCE_DATA;
  alr_data->gprs_data.v_ptmsi2 = indic2;
  alr_data->gprs_data.ptmsi2   = tmsi2;
  TRACE_EVENT_P2("v: %d ptmsi: %x", indic2, tmsi2);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_check_ptmsi       |
+--------------------------------------------------------------------+

  PURPOSE :

*/
BOOL gprs_alr_check_ptmsi(ULONG ptmsi_pag)
{
  GET_INSTANCE_DATA;

  if((alr_data->gprs_data.v_ptmsi AND
      alr_data->gprs_data.ptmsi EQ ptmsi_pag) OR
     (alr_data->gprs_data.v_ptmsi2 AND
      alr_data->gprs_data.ptmsi2 EQ ptmsi_pag))
  {
    ALR_TRACE_GPRS("ptmsi match");
    ma_pch_paging_ind (ID_PTMSI, CN_PACKET);
    return TRUE;
  }

  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_check_downlink_assign
+--------------------------------------------------------------------+

  PURPOSE :

*/
void gprs_alr_check_downlink_assign(T_MPHC_DATA_IND* data_ind)
{
  /* check dl bit */
  if(data_ind->l2_frame.content[3] & 0x20) {
    ma_send_unitdata (data_ind);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_read_si13             |
+--------------------------------------------------------------------+

  PURPOSE :

*/
BOOL gprs_check_read_si13_only(void)
{
  if(gprs_support)
    return FALSE;
  else
    return TRUE;
}

LOCAL BOOL gprs_read_si13(UBYTE si13_position)
{
  ALR_TRACE_GPRS("read si13");

  if(gprs_support AND
     si13_position EQ SI13_ON_NBCCH)
  {
    PALLOC(scell_bcch_req, MPHC_SCELL_NBCCH_REQ);
    /*
     * we want to read SI13 on TC=4 which has to be send at least
     * on every 4th consecutive occurence of TC=4
     */
    scell_bcch_req->schedule_array_size                 = 1;
    scell_bcch_req->schedule_array[0].modulus           = 8;
    scell_bcch_req->schedule_array[0].relative_position = 4;
    ma_scell_nbcch_req(scell_bcch_req);
    return FALSE;
  }
  else return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_stop                  |
+--------------------------------------------------------------------+

  PURPOSE :

*/
void gprs_alr_init(void)
{
  GET_INSTANCE_DATA;
  ALR_TRACE_GPRS("gprs_alr_init");
  alr_data->gprs_data.ign_pgm    = FALSE;
  alr_data->gprs_data.pbcch      = FALSE;
  alr_data->gprs_data.ptm        = FALSE;
  alr_data->gprs_data.check_bsic = FALSE;
  alr_data->gprs_data.sync_only  = FALSE;
  alr_data->gprs_data.pcco_active= FALSE;
  alr_data->nc_sync_with_grr     = FALSE;
  gprs_support = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_tb_meas_ind       |
+--------------------------------------------------------------------+

  PURPOSE :

*/
LOCAL void gprs_alr_tb_meas_ind (T_TB_MEAS_IND *report)
{
  GET_INSTANCE_DATA;
  ALR_TRACE_GPRS("tb_meas_ind");
  switch(GET_STATE(STATE_MA))
  {
    case MA_PTM:
       /*Measurement Report in Packet Transfer Mode*/
      {
        USHORT temp;
        USHORT index;
        UBYTE  diff;
        UBYTE  i,j,lim;
        T_NC*  pcell;

        if(report->tb_meas_result[0].arfcn EQ NOT_PRESENT_16BIT)
        {
          lim = alr_data->nc_data.c_ba_arfcn;
        }
        else
        {
          lim = 0;
          while( report ->tb_meas_result[lim].arfcn NEQ NOT_PRESENT_16BIT AND lim < TB_BA_LIST_SIZE )
          {
            lim++;
          }
        }

        /* 
         * reduce the time for next sync
         * Decrement the 10sec timer counter variable by 2
         */
        alr_data->nc_data.c_ncsync_tim = alr_data->nc_data.c_ncsync_tim-2;
        if ((signed char)( alr_data->nc_data.c_ncsync_tim) < 0)
          alr_data->nc_data.c_ncsync_tim = 0;

        if( alr_data->nc_data.c_ncsync_tim==0 )
        {
          /* 10 sec have elapsed. Perform all requisite tasks */
          nc_ncsync_tim_expiry();
        }
  
        for (pcell=&alr_data->nc_data.cell[0],i = 0; i < lim; i++,pcell++)
        {
          if(lim EQ alr_data->nc_data.c_ba_arfcn)
          {
            index = i;
          }
          else
          {
            /* For all cells look if it is in BA(BCCH)  */
            switch(index = nc_get_index(ARFCN_TO_G23(report->tb_meas_result[i].arfcn)))
            {
              case LAST_BSIC_REQ:
              case NOT_PRESENT_16BIT:
                continue;
              default:
                break;
            }
          }
          /*
           * the ncell is in the list
           * so store the rxlev values
           */
        if(report->tb_meas_result[0].arfcn NEQ NOT_PRESENT_16BIT)
        {
          for(j=0;j<lim;j++)
          {
            if(pcell->ba_arfcn EQ ARFCN_TO_G23(report->tb_meas_result[j].arfcn))
            {
              if (alr_data->nc_data.cell[index].c_rxlev EQ NOT_PRESENT_8BIT)
              {
               /*
                * if it is a new cell, build an average from the first value
                * to speed up fb sb read
                */
                if ((signed short) (report->tb_meas_result[j].rxlev) < 0)
                {
                  report->tb_meas_result[j].rxlev = 0;
                }
                
                alr_data->nc_data.cell[index].rxlev[0] = report->tb_meas_result[j].rxlev / report->tb_meas_result[j].num_meas;
                alr_data->nc_data.cell[index].rxlev[1] = report->tb_meas_result[j].rxlev / report->tb_meas_result[j].num_meas;
                alr_data->nc_data.cell[index].rxlev[2] = report->tb_meas_result[j].rxlev / report->tb_meas_result[j].num_meas;
                alr_data->nc_data.cell[index].rxlev[3] = report->tb_meas_result[j].rxlev / report->tb_meas_result[j].num_meas;
                alr_data->nc_data.cell[index].rxlev[4] = report->tb_meas_result[j].rxlev / report->tb_meas_result[j].num_meas;
                alr_data->nc_data.cell[index].c_rxlev = 0;
              }
              else
              {
                if ((signed short) (report->tb_meas_result[j].rxlev) < 0)
                  report->tb_meas_result[j].rxlev = 0;
                
                alr_data->nc_data.cell[index].rxlev[alr_data->nc_data.cell[index].c_rxlev++] =
                  report->tb_meas_result[j].rxlev / report->tb_meas_result[j].num_meas;
                
                if (alr_data->nc_data.cell[index].c_rxlev >= 5)
                  alr_data->nc_data.cell[index].c_rxlev = 0;
              }
#ifdef GPRS 
              /*
               * store the results seperately for averaging when NC=1 or NC=2
               */
              if(alr_data->nwctrl_meas_active)
              {
                pcell->nc_rxlev += (report->tb_meas_result[j].rxlev / report->tb_meas_result[j].num_meas);
                pcell->c_nc_rxlev++;
                
                TRACE_EVENT_P3("%d %d rx: %d ",i, alr_data->nc_data.cell[i].ba_arfcn,
                  report->tb_meas_result[j].rxlev / report->tb_meas_result[j].num_meas);
              }
#endif        
            break;  
            }
          }
        }
        else
        {
          if (alr_data->nc_data.cell[index].c_rxlev EQ NOT_PRESENT_8BIT)
          {
            /*
             * if it is a new cell, build an average from the first value
             * to speed up fb sb read
             */
            if ((signed short) (report->tb_meas_result[i].rxlev) < 0)
            {
              report->tb_meas_result[i].rxlev = 0;
            }

            alr_data->nc_data.cell[index].rxlev[0] = report->tb_meas_result[i].rxlev / report->tb_meas_result[i].num_meas;
            alr_data->nc_data.cell[index].rxlev[1] = report->tb_meas_result[i].rxlev / report->tb_meas_result[i].num_meas;
            alr_data->nc_data.cell[index].rxlev[2] = report->tb_meas_result[i].rxlev / report->tb_meas_result[i].num_meas;
            alr_data->nc_data.cell[index].rxlev[3] = report->tb_meas_result[i].rxlev / report->tb_meas_result[i].num_meas;
            alr_data->nc_data.cell[index].rxlev[4] = report->tb_meas_result[i].rxlev / report->tb_meas_result[i].num_meas;
            alr_data->nc_data.cell[index].c_rxlev = 0;
          }
          else
          {
            if ((signed short) (report->tb_meas_result[i].rxlev) < 0)
              report->tb_meas_result[i].rxlev = 0;

            alr_data->nc_data.cell[index].rxlev[alr_data->nc_data.cell[index].c_rxlev++] =
              report->tb_meas_result[i].rxlev / report->tb_meas_result[i].num_meas;

            if (alr_data->nc_data.cell[index].c_rxlev >= 5)
              alr_data->nc_data.cell[index].c_rxlev = 0;
          }
          /*
           * store the results seperately for averaging when NC=1 or NC=2 
           */
#ifdef GPRS
          if(alr_data->nwctrl_meas_active)
          {
            if(report->tb_meas_result[0].arfcn NEQ NOT_PRESENT_16BIT)
            {
              for(j=0;j<lim;j++)
              {
                if(pcell->ba_arfcn EQ ARFCN_TO_G23(report->tb_meas_result[j].arfcn))
                {
                  pcell->nc_rxlev += (report->tb_meas_result[j].rxlev / report->tb_meas_result[j].num_meas);
                  pcell->c_nc_rxlev++;
                                    
                  TRACE_EVENT_P3("%d %d rx: %d ",i, alr_data->nc_data.cell[i].ba_arfcn,
                                  report->tb_meas_result[j].rxlev / report->tb_meas_result[j].num_meas);
                  break;
                }
              }
            }
            else
            {
              
              pcell->nc_rxlev += (report->tb_meas_result[i].rxlev / report->tb_meas_result[i].num_meas);
              pcell->c_nc_rxlev++;

              TRACE_EVENT_P3("%d %d rx: %d ",i, alr_data->nc_data.cell[i].ba_arfcn,
              report->tb_meas_result[i].rxlev / report->tb_meas_result[i].num_meas);
            }
          }
#endif
        }
        
        temp = (USHORT)(alr_data->nc_data.cell[index].rxlev[0] +
                        alr_data->nc_data.cell[index].rxlev[1] +
                        alr_data->nc_data.cell[index].rxlev[2] +
                        alr_data->nc_data.cell[index].rxlev[3] +
                        alr_data->nc_data.cell[index].rxlev[4]);
        
        alr_data->nc_data.cell[index].rxlev_average = (UBYTE)(temp / 5);
     
        
          switch (alr_data->nc_data.cell[index].status)
          {
            case INACTIVE:
              nc_set_status (index, IDLE);
              break;
            case EXCLUDED:
              diff = (UBYTE)(alr_data->nc_data.cell[index].rxlev_average -
                             alr_data->nc_data.cell[index].last_rxlev);

              if (diff < 128 AND diff >= 6)
              {
                /* result is positive and more than 6 dBm */
                nc_set_status (index, IDLE);
              }
              break;
            default:
              break;
          }

        } /* for all */
        PFREE(report);

        nc_check_activity();
      }
      break;
    default:
      PFREE(report);
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_nc_enter_ptm      |
+--------------------------------------------------------------------+

  PURPOSE : After the Packet Access Phase we enter Packet Transfer
            Mode and have to do Measurements and Cell Reselection like
            in Idle Mode.
*/

LOCAL void gprs_alr_nc_enter_ptm (void)
{
  GET_INSTANCE_DATA;
  nc_resume();  /* set NC_STATE to IDLE */

  alr_data->nc_data.c_reports   = (alr_data->nc_data.c_reports*10)/alr_data->nc_data.max_reports;
  alr_data->nc_data.max_reports = 10;
  alr_data->nc_data.cell[LAST_BSIC_REQ].status = INACTIVE;

  TRACE_EVENT_P2("glob reps: %d max_report: %d", alr_data->nc_data.c_reports, alr_data->nc_data.max_reports);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_check_page_mode       |
+--------------------------------------------------------------------+

  PURPOSE : Do not change the page mode configuration in L1
            if ing_pgm is set. ie. we have to stay in REORG.

*/
void gprs_check_page_mode(T_MPHC_DATA_IND* data_ind)
{
  GET_INSTANCE_DATA;
  if(alr_data->gprs_data.ign_pgm EQ TRUE)
  {
    pch_check_page_mode_cr(data_ind);
  }
  else
  {
    pch_check_page_mode(data_ind);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_GPRS                   |
| STATE   : code                ROUTINE : gprs_alr_is_supported      |
+--------------------------------------------------------------------+

  PURPOSE : This function returns whether GPRS is supported in
            the cell.

*/
GLOBAL BOOL gprs_alr_is_supported( void )
{
  return( gprs_support EQ TRUE );
}

void set_gprs_support( UBYTE support )
{
  gprs_support = support;
}

#endif /* ALR_GPRS_C */


