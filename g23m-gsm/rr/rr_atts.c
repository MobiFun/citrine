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
|  Purpose :  Handling of BCCH System Information Message and BSIC
+-----------------------------------------------------------------------------
*/

#ifndef RR_ATTS_C
#define RR_ATTS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>     /* offsetof */
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

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
LOCAL void att_sys_info_read(USHORT mess_bit);
LOCAL void att_copy_sys_info_2_2bis_par(UBYTE index,
                                        T_VOID_STRUCT *sys_info_2_2bis,
                                        T_LIST         *new_2_2bis_list,
                                        UBYTE           ncell_ext,
                                        T_SI_TYPE        si_type,
                                        UBYTE     indicate_changes);
/*
 * -------------------------------------------------------------------
 * SIGNAL Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_dat_dedicated          |
+--------------------------------------------------------------------+

  PURPOSE : The data transfer process indicates that the dedicated
            state is reached.

*/

GLOBAL void att_dat_dedicated (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("att_dat_dedicated()");

  /*
   * align attachment process state
   */
  SET_STATE (STATE_ATT, ATT_DEDICATED);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_dat_con_est            |
+--------------------------------------------------------------------+

  PURPOSE : The data transfer process indicates that the connection
            establishment state is reached.

*/
GLOBAL void att_dat_con_est (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_dat_con_est()");

  /*
   * stop possibly parallel PLMN search
   */
  if ((rr_data->ms_data.rr_service EQ LIMITED_SERVICE) OR 
      (CS_GET_CURRENT_SEARCH_MODE EQ BLACK_LIST_SEARCH_MODE))
  {
    att_notify_stop_plmn_search (FALSE);
    TRACE_EVENT(" PLMN_SEARCH_STOPPED");
  }

  /*
   * If network search is ongoing, abort
   * and allow cell reselection.
   */
  if (rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
  {
    /*
     * send Please Retry to the MMI
     */
    rr_data->sc_data.found_entries = 0;
    att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
  }
  /*
   * reset multiband parameter
   */
  rr_data->ncell_mb = 0;

  /*
   * save the neighbour cell list valid in idle mode
   * (derived from SI 2x) to restore it after
   * dedicated mode shorter than 30 seconds
   * Note: SI 5x may contain other neighbour cells than SI 2x
   */
  memcpy(&rr_data->sc_data.ncell_list_idle,
         &rr_data->sc_data.cd.ncell_list,
         sizeof(T_LIST));

  /*
   * align attachment process state
   */
  SET_STATE (STATE_ATT, ATT_CON_EST);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_get_index              |
+--------------------------------------------------------------------+

  PURPOSE : Calculates from the arfcn of the cell the index in the
            data structures:

            0-5          neighbour cell
            6            serving cell (SC_INDEX)
            7            cell reselection cell (CR_INDEX)
            255          not present

*/

GLOBAL UBYTE att_get_index (USHORT arfcn)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("att_get_index()");

  /*
   * check first cell reselection index
   */
  if (arfcn EQ rr_data->nc_data[CR_INDEX].arfcn)
    return CR_INDEX;

  /*
   * then check serving cell index
   */
  if (arfcn EQ rr_data->nc_data[SC_INDEX].arfcn)
    return SC_INDEX;

  /*
   * third step is to check the neighbourcells
   * if available
   */
  for (i=0;i<6;i++)
  {
    if (rr_data->nc_data[i].bcch_status NEQ EMPTY AND
        rr_data->nc_data[i].arfcn EQ arfcn)
      return i;
  }

  /*
   * nothing found
   */
  return NOT_PRESENT_8BIT;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_for_sysinfo_type1      |
+--------------------------------------------------------------------+

  PURPOSE : A system information message one has received.

*/

GLOBAL void att_for_sysinfo_type1 (USHORT         arfcn,
                                   T_D_SYS_INFO_1 *sys_info_1,
                                   T_LIST         *cell_chan_desc)
{
  GET_INSTANCE_DATA;
  /*
   * get the storage area by checking the channel number
   */
  UBYTE index = att_get_index (arfcn);

  TRACE_FUNCTION ("att_for_sysinfo_type1()");

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS2:
      /*
       * during cell selection only CR_INDEX is taken in account
       */
      if (index EQ CR_INDEX)
      {
        /*
         * copy relevant parameters and check bcch carrier
         * if the BCCH is read completely.
         */
        att_copy_sys_info_1_par (CR_INDEX,
                                 sys_info_1,
                                 cell_chan_desc);
        att_check_bcch_carrier_si ();
      }
      break;

    case ATT_CS3:
      /*
       * during cell reselection only CR_INDEX is taken in account
       */

      /*
       * copy relevant parameters and check neighbourcell bcch carrier
       * if the BCCH is read completely.
       */
      att_copy_sys_info_1_par (index,
                               sys_info_1,
                               cell_chan_desc);

      if (index EQ CR_INDEX)
      {
        att_check_neighbourcell_si_reestab();
      }
      break;

    case ATT_IDLE:
    case ATT_CON_EST:
      switch (index)
      {
        case SC_INDEX:
          /*
           * changed system information type 1 message received for
           * the serving cell. Store relevant parameter and reconfigure
           * Layer 1 and MM if needed
           */
          att_copy_sys_info_1_par (SC_INDEX,
                                   sys_info_1,
                                   cell_chan_desc);
          att_check_barred_status_in_idle ();
          break;

        case CR_INDEX:
          att_copy_sys_info_1_par (CR_INDEX,
                                   sys_info_1,
                                   cell_chan_desc);
          break;

        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Implements RR Clone findings #19 */
/*unify functions: att_for_sysinfo_type2 , att_for_sysinfo_type2bis by adding a parameter si_type.*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_for_sysinfo_type2_2bis |
+--------------------------------------------------------------------+

  PURPOSE :   This function handles the SI type 2 and 2Ter common parameters.

*/
GLOBAL void att_for_sysinfo_type2_2bis (USHORT          arfcn,
                                        T_VOID_STRUCT   *sys_info_2_2bis,
                                        T_LIST         *new_2_2bis_list,
                                        UBYTE           ncell_ext,
                                        T_SI_TYPE        si_type)

{
  GET_INSTANCE_DATA;
  /*
   * get the storage area by checking the channel number
   */
  UBYTE index = att_get_index (arfcn);

  TRACE_FUNCTION ("att_for_sysinfo_type2_2bis()");
    
  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS2:
      /*
       * during cell selection only CR_INDEX is taken in account
       */
      if (index EQ CR_INDEX)
      {
        /*
         * copy relevant parameters and check bcch carrier
         * if the BCCH is read completely.
         */

        att_copy_sys_info_2_2bis_par (CR_INDEX,
                                      sys_info_2_2bis,
                                      new_2_2bis_list,
                                      ncell_ext,
                                      si_type,
                                      FALSE);     

        att_check_bcch_carrier_si ();
      }
      break;

    case ATT_CS3:
      /*
       * during cell reselection only CR_INDEX is taken in account
       */
       
      /*      
       * copy relevant parameters and check neighbourcell bcch carrier
       * if the BCCH is read completely.
       */
      att_copy_sys_info_2_2bis_par (index,
                                    sys_info_2_2bis,
                                    new_2_2bis_list,
                                    ncell_ext,
                                    si_type,
                                    FALSE);

      if (index EQ CR_INDEX)
      {        
        att_check_neighbourcell_si_reestab();
      }
      break;

    case ATT_IDLE:
    case ATT_CON_EST:
      switch (index)
      {
        case SC_INDEX:
          /*
           * changed system information type 2/2bis message received for
           * the serving cell. Store relevant parameter and reconfigure
           * Layer 1 and MM if needed
           */
          att_copy_sys_info_2_2bis_par (SC_INDEX,
                                        sys_info_2_2bis,
                                        new_2_2bis_list,
                                        ncell_ext,
                                        si_type,
                                        TRUE);
         
          att_check_barred_status_in_idle ();
#ifdef GPRS          
          if(att_gprs_is_avail() AND (rr_data->sc_data.cd.sys_info_read EQ ALL_SYS_INFO_READ))
          {
            TRACE_EVENT ("SI 2 or 2bis received after SI 13");

            if(rr_data->sc_data.selection_type EQ CELL_RESELECTION_ON_GPRS_ACT)
            {
              TIMERSTOP(T_RESELECT);
            }

            switch (GET_STATE(STATE_GPRS))
            {
            case GPRS_PIM_PBCCH:
              att_gprs_stop_pl();
              break;
            case GPRS_PIM_BCCH:
              att_build_idle_req(SC_INDEX, MODE_SYS_INFO_CHANGE);
              break;
            default: 
              break;
            }
            att_signal_gprs_support();
          }
#endif          
          break;

        case CR_INDEX:
          /*
           * for 2 case:system information 2 is only important to speed up the
           * PLMN available search.
           * for 2bis case:system information type 2bis message is stored.
	   * This is needed
           * for PLMN available search. Really taken in account after
           * reception of system info 2,3 or 4.
           */
          att_copy_sys_info_2_2bis_par (CR_INDEX,
                                         sys_info_2_2bis,
                                         new_2_2bis_list,
                                         ncell_ext,
                                         si_type,
                                         FALSE);
          if (si_type EQ SI_TYPE_2)
          {
            /*
             * if all is read, add the PLMN to the found list
             */
            switch (rr_data->ms_data.req_mm_service)
            {
              case FUNC_NET_SRCH_BY_MMI:
               att_check_bcch_carrier_si ();
               break;
              default:
               TRACE_EVENT_P1 ( "req_mm_service:%d", rr_data->ms_data.req_mm_service );
               break;
            }
          }
          break;
      }
      break;
    default:
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_for_sysinfo_type2ter   |
+--------------------------------------------------------------------+

  PURPOSE :   A system information message 2ter has received.

*/
GLOBAL void att_for_sysinfo_type2ter (USHORT             arfcn,
                                      T_D_SYS_INFO_2TER *sys_info_2ter,
                                      T_LIST            *new_2ter_list)
{
  GET_INSTANCE_DATA;
  /*
   * get the storage area by checking the channel number
   */
  UBYTE index = att_get_index (arfcn);

  TRACE_FUNCTION ("att_for_sysinfo_type2ter()");

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS2:
      /*
       * during cell selection only CR_INDEX is taken in account
       */
      if (index EQ CR_INDEX)
      {
        /*
         * copy relevant parameters and check bcch carrier
         * if the BCCH is read completely.
         */
        att_copy_sys_info_2ter_par (CR_INDEX,
                                    sys_info_2ter,
                                    new_2ter_list,
                                    FALSE);

        att_check_bcch_carrier_si ();
      }
      break;

    case ATT_CS3:
      /*
       * during cell selection only CR_INDEX is taken in account
       */

      /*
       * copy relevant parameters and check bcch carrier
       * if the BCCH is read completely.
       */
      att_copy_sys_info_2ter_par (index,
                                  sys_info_2ter,
                                  new_2ter_list,
                                  FALSE);

      if (index EQ CR_INDEX)
      {
        att_check_neighbourcell_si_reestab();
      }
      break;

    case ATT_IDLE:
    case ATT_CON_EST:
      switch (index)
      {
        case SC_INDEX:
          /*
           * changed system information type 2bis message received for
           * the serving cell. Store relevant parameter and reconfigure
           * Layer 1 and MM if needed
           */
          att_copy_sys_info_2ter_par (SC_INDEX,
                                      sys_info_2ter,
                                      new_2ter_list,
                                      TRUE);
#ifdef GPRS          
          if(att_gprs_is_avail() AND (rr_data->sc_data.cd.sys_info_read EQ ALL_SYS_INFO_READ))
          {
            TRACE_EVENT ("SI 2ter received after SI 13");

            if(rr_data->sc_data.selection_type EQ CELL_RESELECTION_ON_GPRS_ACT)
            {
              TIMERSTOP(T_RESELECT);
            }

            switch (GET_STATE(STATE_GPRS))
            {
            case GPRS_PIM_PBCCH:
              att_gprs_stop_pl();
              break;
            case GPRS_PIM_BCCH:
              att_build_idle_req(SC_INDEX, MODE_SYS_INFO_CHANGE);
              break;
            default: 
              break;
            }
            att_signal_gprs_support();
          }
#endif          
          break;
        case CR_INDEX:
          /*
           * system information type 2bis message is stored. This is needed
           * for PLMN available search. Really taken in account after
           * reception of system info 2,3 or 4.
           */
          att_copy_sys_info_2ter_par (CR_INDEX,
                                      sys_info_2ter,
                                      new_2ter_list,
                                      FALSE);
          break;
      }
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_for_sysinfo_type3      |
+--------------------------------------------------------------------+

  PURPOSE :   A system information message 3 has received.

*/

GLOBAL void att_for_sysinfo_type3 (USHORT         arfcn,
                                   T_D_SYS_INFO_3 *sys_info_3)
{
  GET_INSTANCE_DATA;
  /*
   * get the storage area by checking the channel number
   */
  UBYTE index = att_get_index (arfcn);

  TRACE_FUNCTION ("att_for_sysinfo_type3()");

  TRACE_EVENT_P8 ("SI3: MCC=%X%X%X MNC=%X%X%X LAC=%04X CID=%04X",
                  sys_info_3->loc_area_ident.mcc[0],
                  sys_info_3->loc_area_ident.mcc[1],
                  sys_info_3->loc_area_ident.mcc[2],
                  sys_info_3->loc_area_ident.mnc[0],
                  sys_info_3->loc_area_ident.mnc[1],
                  sys_info_3->loc_area_ident.mnc[2],
                  sys_info_3->loc_area_ident.lac,
                  sys_info_3->cell_ident);

  if ( index EQ NOT_PRESENT_8BIT )
    return;

  #ifdef GPRS
  rr_data->nc_data[index].si13_loc_ind = SI13_NOT_PRESENT;
  if(sys_info_3->si3_rest_oct.v_gprs_indic)
    if(sys_info_3->si3_rest_oct.gprs_indic.si13_pos EQ SI13_ON_BCCH_EXT)
      rr_data->nc_data[index].si13_loc_ind = EXTENDED_BCCH;
    else
      rr_data->nc_data[index].si13_loc_ind = NORMAL_BCCH;
  #endif
  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS2:
      /*
       * during cell selection only CR_INDEX is taken in account
       */
      if (index EQ CR_INDEX)
      {
        /*
         * copy relevant parameters and check bcch carrier
         * if the BCCH is read completely.
         */
        att_copy_sys_info_3_par (CR_INDEX,
                                 sys_info_3,
                                 FALSE);
        /* 
         * During Cell Selection if received SI3 from a PLMN which doesn't match
         * requested PLMN then no need to wait for remaining SI messages
         * The First Scan First Attempt is for Full Service search and only
         * the carriers with Normal Priority  is scanned and for this if the
	 * PLMN doesnot match, then no need to read remaining SIs. In the
	 * successive scans i.e First Scan Second Attempt and Second scan all
	 * the carriers are scanned irrespective of the priority and all the SI
	 * messages need to be read.
         */
        if ((rr_data->cs_data.scan_mode EQ CS_FIRST_SCAN_FIRST_ATTEMPT) AND
            (rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH) AND 
            (!dat_plmn_equal_req (sys_info_3->loc_area_ident.mcc,
                                  sys_info_3->loc_area_ident.mnc,
                                  rr_data->ms_data.plmn.mcc,
                                  rr_data->ms_data.plmn.mnc)))
        {
          att_set_sys_info_read ( SYS_INFO_4_READ | 
                                  SYS_INFO_1_READ | 
                                  SYS_INFO_2_READ | 
                                  SYS_INFO_2BIS_READ | 
                                  SYS_INFO_2TER_READ, index);
#ifdef GPRS
          att_set_sys_info_read (SYS_INFO_13_READ, index);
#endif    
        }
#ifdef GPRS
        else
        {
        att_check_gprs_supp (sys_info_3->si3_rest_oct.v_gprs_indic,
                             &sys_info_3->si3_rest_oct.gprs_indic);
        }
#endif
        att_check_bcch_carrier_si ();
      }
      break;

    case ATT_CS3:
      /*
       * during cell reselection only CR_INDEX is taken in account
       */

      /*
       * copy relevant parameters and check neighbourcell bcch carrier
       * if the BCCH is read completely.
       */
      att_copy_sys_info_3_par (index,
                               sys_info_3,
                               FALSE); 

      if (index EQ CR_INDEX)
      {
#ifdef GPRS
        att_check_gprs_supp (sys_info_3->si3_rest_oct.v_gprs_indic,
                             &sys_info_3->si3_rest_oct.gprs_indic);
#endif

        att_check_neighbourcell_si_reestab();
      }
      else
      {
        /* We have received some SYS INFO which is not the CR candidate */
        rr_data->nc_data[index].bcch_status  = DECODED;
        rr_data->nc_data[index].bcch_counter = 0;
        rr_data->nc_data[index].c1_counter   = 0;
      }
      break;

    case ATT_IDLE:
    case ATT_CON_EST:
      switch (index)
      {
        case SC_INDEX:
          /*
           * changed system information type 3 message received for
           * the serving cell. Store relevant parameter and reconfigure
           * Layer 1 and MM if needed
           */
#ifdef GPRS
         att_check_gprs_supp (sys_info_3->si3_rest_oct.v_gprs_indic,
                              &sys_info_3->si3_rest_oct.gprs_indic);
#endif

          att_copy_sys_info_3_par (SC_INDEX,
                                   sys_info_3,
                                   TRUE);

          att_check_barred_status_in_idle ();
          break;

        case CR_INDEX:
          /*
           * it is for a cell which is read in parallel during idle mode
           */
          att_copy_sys_info_3_par (CR_INDEX,
                                   sys_info_3,
                                   FALSE);


          /*
           * depending on the requested MM service
           */

/* Implements RR Clone findings #30 */
          att_sys_info_read(SYS_INFO_4_READ);
          break;

        case NOT_PRESENT_8BIT:
          /*
           * unexpected channel number
           */
          break;

        default:
          {
            BOOL plmn_ok = TRUE;

            if(rr_data->ms_data.rr_service EQ FULL_SERVICE)
            {
              /*
               * Sometimes the BA list contains cells from other PLMN's
               */
              plmn_ok = dat_plmn_equal_req (sys_info_3->loc_area_ident.mcc,
                                 sys_info_3->loc_area_ident.mnc,
                                 rr_data->ms_data.plmn.mcc,
                                 rr_data->ms_data.plmn.mnc);
            }
            if(plmn_ok)
            {
              /*
               * neighbour cell, copy parameters and set status to decoded.
               */
              att_copy_sys_info_3_par (index,sys_info_3,FALSE);
              att_bcch_status_to_decoded (index);
            }
            else
              TRACE_EVENT_P2("[%u]i%u Ignoring SI3 as plmn_ok = FALSE", rr_data->nc_data[index].arfcn, index);
          }
        break;
      }
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_for_sysinfo_type4      |
+--------------------------------------------------------------------+

  PURPOSE : A system information message 4 has received.

*/

GLOBAL void att_for_sysinfo_type4 (USHORT          arfcn,
                                   T_D_SYS_INFO_4 *sys_info_4)
{
  GET_INSTANCE_DATA;
  /*
   * get the storage area by checking the channel number
   */
  UBYTE index = att_get_index (arfcn);

  TRACE_FUNCTION ("att_for_sysinfo_type4()");

  TRACE_EVENT_P7 ("SI4: MCC=%X%X%X MNC=%X%X%X LAC=%04X",
                  sys_info_4->loc_area_ident.mcc[0],
                  sys_info_4->loc_area_ident.mcc[1],
                  sys_info_4->loc_area_ident.mcc[2],
                  sys_info_4->loc_area_ident.mnc[0],
                  sys_info_4->loc_area_ident.mnc[1],
                  sys_info_4->loc_area_ident.mnc[2],
                  sys_info_4->loc_area_ident.lac);

  if ( index EQ NOT_PRESENT_8BIT )
    return;

  #ifdef GPRS
  rr_data->nc_data[index].si13_loc_ind = SI13_NOT_PRESENT;
  if(sys_info_4->si4_rest_oct.v_gprs_indic)
    if(sys_info_4->si4_rest_oct.gprs_indic.si13_pos EQ SI13_ON_BCCH_EXT)
      rr_data->nc_data[index].si13_loc_ind = EXTENDED_BCCH;
    else
      rr_data->nc_data[index].si13_loc_ind = NORMAL_BCCH;
  #endif
  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS2:
      /*
       * during cell selection only CR_INDEX is taken in account
       */
      if (index EQ CR_INDEX)
      {
        /*
         * copy relevant parameters and check bcch carrier
         * if the BCCH is read completely.
         */
        att_copy_sys_info_4_par (CR_INDEX,
                                 sys_info_4,
                                 FALSE);
        /* 
         * During Cell Selection if received SI4 from a PLMN which doesn't match
         * requested PLMN then no need to wait for remaining SI messages
         * The First Scan First Attempt is for Full Service search and only
         * the carriers with Normal Priority  is scanned and for this if the
	 * PLMN doesnot match, then no need to read remaining SIs. In the
	 * successive scans i.e First Scan Second Attempt and Second scan all
	 * the carriers are scanned irrespective of the priority and all the SI
	 * messages need to be read.
         */
        if ((rr_data->cs_data.scan_mode EQ CS_FIRST_SCAN_FIRST_ATTEMPT) AND
            (rr_data->ms_data.req_mm_service EQ FUNC_PLMN_SRCH) AND 
            (!dat_plmn_equal_req (sys_info_4->loc_area_ident.mcc,
                                  sys_info_4->loc_area_ident.mnc,
                                  rr_data->ms_data.plmn.mcc,
                                  rr_data->ms_data.plmn.mnc)))
        {
          att_set_sys_info_read ( SYS_INFO_3_READ | 
                                  SYS_INFO_1_READ | 
                                  SYS_INFO_2_READ | 
                                  SYS_INFO_2BIS_READ | 
                                  SYS_INFO_2TER_READ, index);
#ifdef GPRS
          att_set_sys_info_read (SYS_INFO_13_READ, index);
#endif    
        }
#ifdef GPRS
        else
        {
          if (!sys_info_4->cell_select.acs)
          {
            att_check_gprs_supp(sys_info_4->si4_rest_oct.v_gprs_indic,
                                &sys_info_4->si4_rest_oct.gprs_indic);
          }
        }
#endif
        att_check_bcch_carrier_si ();
      }
      break;

    case ATT_CS3:
      /*
       * during cell reselection only CR_INDEX is taken in account
       */

      /*
       * copy relevant parameters and check neighbourcell bcch carrier
       * if the BCCH is read completely.
       */
      att_copy_sys_info_4_par (index,
                               sys_info_4,
                               FALSE);

      if (index EQ CR_INDEX)
      {        
#ifdef GPRS
       if (!sys_info_4->cell_select.acs)
       {
         att_check_gprs_supp(sys_info_4->si4_rest_oct.v_gprs_indic,
                             &sys_info_4->si4_rest_oct.gprs_indic);
       }
#endif
        att_check_neighbourcell_si_reestab();
      }
      else
      {
        if (!sys_info_4->cell_select.acs)
        {
          /*
           * if system information type 4 contains all information
           * for calculation C2, copy this parameter and declare channel
           * as decoded, that means as a candidate for cell reselection.
           */
          att_copy_c2_parameter_si4 (index, &sys_info_4->si4_rest_oct);
          rr_data->nc_data[index].bcch_status  = DECODED;
          rr_data->nc_data[index].bcch_counter = 0;
          rr_data->nc_data[index].c1_counter   = 0;
        }
      }
      break;

    case ATT_IDLE:
    case ATT_CON_EST:
      switch (index)
      {
        case SC_INDEX:
          /*
           * changed system information type 4 message received for
           * the serving cell. Store relevant parameter and reconfigure
           * Layer 1 and MM if needed
           */
          att_copy_sys_info_4_par (SC_INDEX,
                                   sys_info_4,
                                   TRUE);
          att_check_barred_status_in_idle ();
          break;

        case NOT_PRESENT_8BIT:
          /*
           * unexpected channel number, ignore
           */
          break;

        case CR_INDEX:
          /*
           * channel for which RR searches in parallel
           */
          att_copy_sys_info_4_par (CR_INDEX,
                                   sys_info_4,
                                   FALSE);
          if (!sys_info_4->cell_select.acs)
          {
            att_copy_c2_parameter_si4 (index, &sys_info_4->si4_rest_oct);
          }

          /*
           * depending on the service requested by MM
           */

/* Implements RR Clone findings #30 */
          att_sys_info_read(SYS_INFO_3_READ);
          break;

        default:
          {
            BOOL plmn_ok = TRUE;

            if(rr_data->ms_data.rr_service EQ FULL_SERVICE)
            {
            /*
             * Sometimes the BA list contains cells from other PLMN's
             */
            plmn_ok = dat_plmn_equal_req (sys_info_4->loc_area_ident.mcc,
                               sys_info_4->loc_area_ident.mnc,
                               rr_data->ms_data.plmn.mcc,
                               rr_data->ms_data.plmn.mnc);
            }
            if(plmn_ok)
            {
              /*
               * Neighbour cell BCCH
               */
              att_copy_sys_info_4_par (index, sys_info_4, FALSE);

              if (!sys_info_4->cell_select.acs)
              {
                /*
                 * if system information type 4 contains all information
                 * for calculation C2, copy this parameter and declare channel
                 * as decoded, that means as a candidate for cell reselection.
                 */
                att_copy_c2_parameter_si4 (index, &sys_info_4->si4_rest_oct);
                att_bcch_status_to_decoded (index);
              }
            }
            else
              TRACE_EVENT_P2("[%u]i%u Ignoring SI4 as plmn_ok = FALSE", rr_data->nc_data[index].arfcn, index);
         }
         break;
      }
      break;

    default:
      break;
  }
}

/* Implements RR Clone findings #2 */
/*unify functions: att_for_sysinfo_type5, att_for_sysinfo_type5bis by adding a parameter si_type*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_for_sysinfo_type5_5bis |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the system information type 5 and 5bis
  common parameters.

*/
GLOBAL  void  att_for_sysinfo_type5_5bis(USHORT             arfcn,
                                         T_LIST             *new_5_5bis_list,
                                         UBYTE              old_index,
                                         UBYTE              ncell_ext,
                                         T_SI_TYPE          si_type)

{
  GET_INSTANCE_DATA;
  /*
   * calculate the index for the channel
   */
  UBYTE index = att_get_index (arfcn);

  USHORT cur_si_type_read; /*Current SI type*/
  USHORT oth_si_type_read; /*The other SI type in this function*/
  USHORT oth_si_to_clean;/*The other SI to clean in this function*/

  TRACE_FUNCTION("att_for_sysinfo_type5_5bis()");

  /*
   * only messages for the serving cell are taken in account
   */
  if (index NEQ SC_INDEX)
  {
    return;
  }

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_DEDICATED:
    case ATT_CON_EST:
      if (dat_check_error_flag (SEND_NO_RR_STATUS))
      {
        /*
         * receiving a modified BA indicator forces discarding the current BA
         * receiving a SI5 while the bit SYS_INFO_5_READ is set   or
         * receiving a SI5Bis while the bit  SYS_INFO_5Bis_READ is set
         * indicates a change of the BA list
         * => clear old channel list
         * => delete other SI5x read bit mask
         */
        if(si_type EQ SI_TYPE_5)
        {
          cur_si_type_read = SYS_INFO_5_READ;
          oth_si_type_read = SYS_INFO_5BIS_READ;
          oth_si_to_clean = IND_SI_5BIS;
        }
        else
        {
          cur_si_type_read = SYS_INFO_5BIS_READ;
          oth_si_type_read = SYS_INFO_5_READ;
          oth_si_to_clean = IND_SI_5;
        }

        if (old_index NEQ rr_data->sc_data.ba_index OR
               (rr_data->sc_data.cd.sys_info_read &  ( cur_si_type_read) )EQ
               (cur_si_type_read))
        {
              srv_copy_list (&rr_data->sc_data.cd.ncell_list, new_5_5bis_list,
                          sizeof (T_LIST));
              srv_clear_list(&rr_data->sc_data.five_ter_list);
              rr_data->sc_data.cd.sys_info_read &=
              ~((oth_si_type_read) | SYS_INFO_5TER_READ);
              att_clean_buf( (USHORT)((oth_si_to_clean) | IND_SI_5TER));
        }
        else if ( (rr_data->sc_data.cd.sys_info_read &
                    (cur_si_type_read | oth_si_type_read)) EQ 0 )
        {
            /* neither SI5 nor SI5bis received before */
              srv_copy_list (&rr_data->sc_data.cd.ncell_list,
                             new_5_5bis_list,
                             sizeof (T_LIST));
        }
        else
        {
           /*
            * SI5bis/SI5 received before
            * merge list of sys info 5/sys info 5bis with the old neighbour
	    * cell list
            */
           srv_merge_list (&rr_data->sc_data.cd.ncell_list, new_5_5bis_list);
        }
        /* CSI-LLD section:4.1.1.11
         * This function Updates the black list with the BA list received in
	 * si5/si5bis
         */
        cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,new_5_5bis_list);

        /*
         * set concerning bit mask
         * if the extension indication not set,
         * the sys info carries the complete BA
         */
        att_set_sys_info_read ((cur_si_type_read), SC_INDEX);
        if (!ncell_ext) /* in case of 5bis, is't possible, that 5bis carries the complete BA???*/ 
        {
          att_set_sys_info_read ((oth_si_type_read), SC_INDEX);
        }

        att_code_mph_ncell_req_dedicated ();
      }
      break;

    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_for_sysinfo_type5ter   |
+--------------------------------------------------------------------+

  PURPOSE : A system information message 5ter has received.

*/

GLOBAL void att_for_sysinfo_type5ter (USHORT             arfcn,
                                      T_LIST            *new_5ter_list)
{
  GET_INSTANCE_DATA;
  /*
   * calculate the index for the channel
   */
  UBYTE index = att_get_index (arfcn);
  TRACE_FUNCTION ("att_for_sysinfo_type5ter()");

  /*
   * only messages for the serving cell are taken in account
   */
  if (index NEQ SC_INDEX)
    return;

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_DEDICATED:
    case ATT_CON_EST:
      if (dat_check_error_flag (SEND_NO_RR_STATUS))
      {
        /*
         * if there is received a new SI5ter, it replaces the old
         * five_ter_list
         */
        att_set_sys_info_read (SYS_INFO_5TER_READ, SC_INDEX);
        srv_copy_list (&rr_data->sc_data.five_ter_list, new_5ter_list,
                       sizeof (T_LIST));

        /* CSI-LLD section:4.1.1.11
         * This function Updates the black list with the BA list received in si5ter
         */
        cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,new_5ter_list);

        att_code_mph_ncell_req_dedicated ();
      }
      break;

    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_for_sysinfo_type6      |
+--------------------------------------------------------------------+

  PURPOSE : A system information type 6 message has received.

*/

/*
 * two tables to convert air-interface coding for dtx (depending on full
 * or halfrate) to the internal coding
 */
static const UBYTE   dtx_halfrate [8] =
                     { FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE };
static const UBYTE   dtx_fullrate [8] =
                     { TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, FALSE, TRUE };


GLOBAL void att_for_sysinfo_type6 (USHORT          arfcn,
                                   T_D_SYS_INFO_6 *sys_info_6)
{
  GET_INSTANCE_DATA;
  /*
   * calculate the index for the channel
   */
  UBYTE     index    = att_get_index (arfcn);
  UBYTE     transfer = FALSE;
  UBYTE     new_dtx;

  TRACE_FUNCTION ("att_for_sysinfo_type6()");

  /*
   * only messages for the serving cell are taken in account
   */
  if (index NEQ SC_INDEX)
    return;

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_DEDICATED:
    case ATT_CON_EST:
      if (dat_check_error_flag (SEND_NO_RR_STATUS))
      {
        /*
         * store location information for the serving cell
         */
        rr_data->nc_data[SC_INDEX].cell_id = sys_info_6->cell_ident;
        memcpy (&rr_data->nc_data[SC_INDEX].lai,
                &sys_info_6->loc_area_ident,
                sizeof (T_loc_area_ident));

        /*
         * set new dtx values
         */
        rr_data->sc_data.cd.dtx_half = dtx_halfrate [(sys_info_6->cell_opt_sacch.dtx2_s << 2) +
                                                   sys_info_6->cell_opt_sacch.dtx_s];
        rr_data->sc_data.cd.dtx_full = dtx_fullrate [(sys_info_6->cell_opt_sacch.dtx2_s << 2) +
                                                   sys_info_6->cell_opt_sacch.dtx_s];
        if (rr_data->sc_data.chan_desc.chan_type EQ 2 OR
            rr_data->sc_data.chan_desc.chan_type EQ 3)
          new_dtx = rr_data->sc_data.cd.dtx_half;
        else
          new_dtx = rr_data->sc_data.cd.dtx_full;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
        rr_data->sc_data.cd.band_indicator = sys_info_6->si6_rest_oct.band_indicator;
        att_update_std_band_indicator (rr_data->sc_data.cd.band_indicator);
#endif

        /*
         * Check change of DTX value
         */
        if (rr_data->sc_data.cd.dtx NEQ new_dtx)
        {
            transfer = TRUE;
            rr_data->sc_data.cd.dtx = new_dtx;
        }

        /*
         * Check Power Control Indicator
         */
        if (rr_data->sc_data.cd.cell_options.pow_ctrl NEQ sys_info_6->cell_opt_sacch.pow_ctrl)
        {
            transfer = TRUE;
            rr_data->sc_data.cd.cell_options.pow_ctrl = sys_info_6->cell_opt_sacch.pow_ctrl;
        }

        /*
         * Check Radio Link Timeout
         */
        if (rr_data->sc_data.cd.cell_options.rlt NEQ sys_info_6->cell_opt_sacch.rlt)
        {
            transfer = TRUE;
            rr_data->sc_data.cd.cell_options.rlt = sys_info_6->cell_opt_sacch.rlt;
        }

        /*
         * Copy NCC Permitted
         */
        rr_data->sc_data.cd.ncc_permitted = sys_info_6->ncc_permit;

        /*
         * transfer changed values to layer 1 if available
         */
        if (transfer)
          dat_code_sys_info_change (rr_data->sc_data.cd.dtx,
                                    rr_data->sc_data.cd.cell_options.pow_ctrl,
                                    rr_data->sc_data.cd.cell_options.rlt);
      }
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_for_sysinfo_type7_8    |
+--------------------------------------------------------------------+

  PURPOSE : a system information message type 7 or 8 has received.

*/

GLOBAL void att_for_sysinfo_type7_8 (USHORT          arfcn,
                                     T_D_SYS_INFO_8 *sys_info_8)
{
  GET_INSTANCE_DATA;
  /*
   * calculate the index for the channel
   */
  UBYTE index = att_get_index (arfcn);

  TRACE_FUNCTION ("att_for_sysinfo_type7_8()");

  if ( index EQ NOT_PRESENT_8BIT )
    return;

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS2:
    case ATT_IDLE:
    case ATT_CON_EST:
      switch (index)
      {
        case SC_INDEX:
          att_copy_sys_info_7_8_par (SC_INDEX,
                                     sys_info_8);
          break;
        case CR_INDEX:
          att_copy_sys_info_7_8_par (CR_INDEX,
                                     sys_info_8);
          break;
        case NOT_PRESENT_8BIT:
          break;

        default:
          /*
           * Neighbour cell BCCH requested
           */
          att_copy_sys_info_7_8_par (index,
                                     sys_info_8);
          att_bcch_status_to_decoded (index);
          break;
      }
      break;

    case ATT_CS3:
      att_copy_sys_info_7_8_par (index, sys_info_8);
      rr_data->nc_data[index].bcch_status  = DECODED;
      rr_data->nc_data[index].bcch_counter = 0;
      rr_data->nc_data[index].c1_counter   = 0;
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_bsic_ind               |
+--------------------------------------------------------------------+

  PURPOSE : Handling of the signal att_bsic_ind from the cell selection
            process. The signal is the positive result of a bsic request
            to layer 1.

*/

GLOBAL void att_bsic_ind (USHORT arfcn, UBYTE rxlev, UBYTE bsic)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_bsic_ind()");

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS1:
    case ATT_CS2:
      /*
       * during cell selection
       *
       * store parameters of the carrier
       */
      rr_data->nc_data[CR_INDEX].arfcn = arfcn;
      rr_data->nc_data[CR_INDEX].rxlev = rxlev;
      rr_data->nc_data[CR_INDEX].bsic  = bsic;

      /*
       * prepare reading of the BCCH, clear bitmap
       * and start timer to control the end of BCCH reading.
       */
      att_init_cr_data();
      TIMERSTART (T_RESELECT, TRESELECT_VALUE);
      rr_data->nc_data[CR_INDEX].bcch_error = 0;
      srv_clear_list (&rr_data->cr_data.cd.ncell_list);
      SET_STATE (STATE_ATT, ATT_CS2);
      srv_use_stored_prim ();
      break;

    case ATT_IDLE:
    case ATT_CON_EST:
      /*
       *  A search in parallel to the idle mode is ongoing.
       */
      if (arfcn EQ rr_data->nc_data[SC_INDEX].arfcn)
      {
        /*
         * short cut if this is the current serving cell
         */
        PALLOC (mph_sync_req, MPH_SYNC_REQ);

        mph_sync_req->cs = CS_STOP_BCCH_READING;

        PSENDX (PL, mph_sync_req);

        /*
         * simulate BCCH reading by copying serving cell data
         */
        memcpy (&rr_data->nc_data[CR_INDEX],
                &rr_data->nc_data[SC_INDEX],
                sizeof (T_NC_DATA));
        /* Copy the neighbour cell list also */
        memcpy (&rr_data->cr_data.cd, &rr_data->sc_data.cd,
                sizeof (T_CELL_DATA));

        rr_data->nc_data[CR_INDEX].arfcn = arfcn;
        rr_data->nc_data[CR_INDEX].rxlev = rxlev;
        rr_data->nc_data[CR_INDEX].bsic  = bsic;

        att_check_bcch_carrier ();
      }
      else
      {
        /*
         * store the data from the BSIC confirmation
         */
        rr_data->nc_data[CR_INDEX].arfcn = arfcn;
        rr_data->nc_data[CR_INDEX].rxlev = rxlev;
        rr_data->nc_data[CR_INDEX].bsic  = bsic;

        /*
         * initialize parameters for BCCH reading
         */
        att_init_cr_data();
        TIMERSTART (T_RESELECT, TRESELECT_VALUE);
        srv_use_stored_prim ();
        rr_data->nc_data[CR_INDEX].bcch_error = 0;
        srv_clear_list (&rr_data->cr_data.cd.ncell_list);
      }
      break;

    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_no_bsic_ind            |
+--------------------------------------------------------------------+

  PURPOSE : no bsic indication from cell selection process indicates
            the end of the search for a carrier by BSIC requests.

*/

GLOBAL void att_no_bsic_ind (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_no_bsic_ind()");

  switch (GET_STATE (STATE_ATT))
  {
    case ATT_CS1:
    case ATT_CS2:
      /*
       * depending on the service requested by MM
       */
      switch (rr_data->ms_data.req_mm_service)
      {
        case FUNC_NET_SRCH_BY_MMI:
          /*
           * MM has requested the list of available
           * PLMNs. Allow cell reselection decision
           * again and send list to MM.
           */
          rr_data->ms_data.rr_service = NO_SERVICE;
          att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
          SET_STATE (STATE_ATT, ATT_NO_SERVICE);
          att_start_registration_timer ();
          srv_use_stored_prim ();
          break;

        default:
          /*
           * limited or full serivce requested by MM
           */
          switch (rr_data->ms_data.req_mm_service)
          {
            case FUNC_LIM_SERV_ST_SRCH:
              /*
               * If MM has requested FUNC_PLMN_SRCH
               * but no suitable cell was found and
               * RR tried to reach limited service
               * or MM only requested limited service
               * but RR could not find an acceptable cell
               * return no service
               */
              rr_data->ms_data.rr_service = NO_SERVICE;
              att_reset_old_lai_rac();
              att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
              SET_STATE (STATE_ATT, ATT_NO_SERVICE);
              att_start_registration_timer ();
              srv_use_stored_prim ();
              break;

            case FUNC_PLMN_SRCH:
              /*
               * no BCCH for full service found
               */
              if(rr_data->cs_data.scan_mode EQ CS_FIRST_SCAN_FIRST_ATTEMPT)
              {

                rr_data->cs_data.scan_mode = CS_FIRST_SCAN_SECOND_ATTEMPT;

                TRACE_EVENT("First Scan First Attempt done");

                /*
                 * first attempt then look for low priority cells
                 */
                if (cs_def_list (LOW_PRIORITY_CELL))
                {

                  TRACE_EVENT("First Scan Second attempt started");
                  /*
                   * low priority cells available, then try it again
                   */
                  cs_start_sync ();
                  break;
                }
              }

              /*
               * no BCCH for full service available and the low
               * priority cells have been tried. Then try
               * to reach limited service by checking
               * for emergency cells
               */
              if (cs_def_list (EMERGENCY_CELL) AND
                  rr_data->cs_data.scan_mode EQ CS_FIRST_SCAN_SECOND_ATTEMPT)
              {
                /*
                 * emergency cells available
                 */
                rr_data->cs_data.scan_mode      = CS_SECOND_SCAN;
                rr_data->ms_data.rr_service     = LIMITED_SERVICE;
                TRACE_EVENT ("no suitable cell found, find acceptable cell");
                /*
                 * clear any pending RR_ESTABLISH_REQ
                 * MM would retransmit RR_ESTABLISH_REQ for emergency calls
                 */
                srv_clear_stored_prim (RR_ESTABLISH_REQ);
                cs_start_sync ();
              }
              else
              {
                /*
                 * no emergency cells availabe, then no service.
                 */
                srv_clear_stored_prim (RR_ESTABLISH_REQ);
                rr_data->ms_data.rr_service = NO_SERVICE;
                att_reset_old_lai_rac();
                att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
                SET_STATE (STATE_ATT, ATT_NO_SERVICE);
             
                /* CSI-LLD section:4.1.1.10  
                 * This function updates the black list after the first successful 
                 * FUNC_PLMN_SEARCH 
                 */
                if(rr_data->cs_data.initial_plmn_search EQ INITIAL_PLMN_SEARCH_ACTIVE)
                { 
                  cs_update_black_list();
                  rr_data->cs_data.initial_plmn_search = INITIAL_PLMN_SEARCH_DONE;
                }
             
                att_start_registration_timer ();
                TRACE_EVENT ("no acceptable cell found");
                srv_use_stored_prim ();
              }
              break;
            default:
              break;
          }
          break;
      }
      break;

    case ATT_IDLE:
    case ATT_CON_EST:
      switch (rr_data->ms_data.req_mm_service)
      {
        case FUNC_NET_SRCH_BY_MMI:
          /*
           * allow cell reselection decision
           */
          att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
          att_start_registration_timer ();
          srv_use_stored_prim ();
          break;

        case FUNC_PLMN_SRCH:
          /*
           * no BCCH for full service found
           */
          TRACE_EVENT ("Full Requested, end of Search");
          if (rr_data->ms_data.rr_service EQ LIMITED_SERVICE)
          {
            /* we trap here if RR tried to reach the from MM requested
               FUNC_PLMN_SRCH service
               but could not find a suitable cell
               The acceptable cell will be the serving cell
               we don't look for another one.
             */
            att_reset_old_lai_rac();
            att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
          }
          else
          {
            /* this was a search for the HPLMN don't inform
             * MM because we have FULL_SERVICE on a VPLMN
             */
            att_notify_stop_plmn_search (FALSE);
#if 0
            if (rr_data->gprs_data.gprs_indic)
            {
              att_signal_gprs_support();
            }
#endif
            EM_HPLMN_SEARCH_FAILED;
          }

          att_start_registration_timer ();
          break;
        case FUNC_LIM_SERV_ST_SRCH:
          /*
           * no BCCH for limited service found
           */
          rr_data->ms_data.rr_service = NO_SERVICE;
          att_reset_old_lai_rac();
          att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
          SET_STATE (STATE_ATT, ATT_NO_SERVICE);
          att_start_registration_timer ();
          srv_use_stored_prim ();
          break;

        default:
          break;
      }
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_full_service_found     |
+--------------------------------------------------------------------+

  PURPOSE : Performs all tasks required after reaching Full Service
            CSI-LLD section:4.1.3.4.1.7

*/

GLOBAL void att_full_service_found(void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION("att_full_service_found()");
  
  /* Stop search mode timers */
  TIMERSTOP(T_FAST_CS);
  TIMERSTOP(T_NORMAL_CS);
  
  /* Update the white list */
  dat_convert_white_list();

  /* Send SI2 info to MM */
  dat_send_bcchinfo_mm (rr_data->cr_data.cr_white_list.si2);

  TRACE_EVENT("RR SERVICE = FULL SERVICE");

}


/*convert into a function (si_type {SI3,SI4})*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_ATT                     |
| STATE   : code                ROUTINE : att_sys_info_read          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the sys_info_read flag based on the passed
  system information type

*/
LOCAL void att_sys_info_read(USHORT mess_bit)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION("att_sys_info_read()");

  switch (rr_data->ms_data.req_mm_service)
  {
    case FUNC_NET_SRCH_BY_MMI:
     /*
      * PLMN available search
      *
      * it is useless to read system info 1, 2ter and 4
      */
      att_set_sys_info_read ( mess_bit | 
                              SYS_INFO_1_READ | 
                              SYS_INFO_2_READ | 
                              SYS_INFO_2BIS_READ | 
                              SYS_INFO_2TER_READ, CR_INDEX);
#ifdef GPRS
      att_set_sys_info_read (SYS_INFO_13_READ, CR_INDEX);
#endif

      /*
       * enough info available for PLMN available search
       */
      att_check_bcch_carrier_si ();
      break;

    case FUNC_PLMN_SRCH:
      switch (rr_data->ms_data.rr_service)
      {
        case LIMITED_SERVICE:
        /*
         * RR is in limited service. So this search triggered by
         * RR shall reach full service again.
         */
          att_check_fplmn_cell ();
          break;

        default:
          break;
      }
      break;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)     MODULE  : RR_ATT                       |
| STATE   : code              ROUTINE : att_copy_sys_info_2_2bis_par |
+--------------------------------------------------------------------+

  PURPOSE : This function handles system information type 2 and 2bis common
  parameters

*/
LOCAL void att_copy_sys_info_2_2bis_par(UBYTE index,
                                        T_VOID_STRUCT *sys_info_2_2bis,
                                        T_LIST         *new_2_2bis_list,
                                        UBYTE           ncell_ext,
                                        T_SI_TYPE        si_type,
                                        UBYTE     indicate_changes)
{
  TRACE_FUNCTION("att_copy_sys_info_2_2bis_par()");

  if (si_type EQ SI_TYPE_2)
  {
    att_copy_sys_info_2_par (index,
                             (T_D_SYS_INFO_2 *)sys_info_2_2bis,
                             new_2_2bis_list,
                             ncell_ext,
                             indicate_changes);     
  }
  else if (si_type EQ SI_TYPE_2BIS)
  {
    att_copy_sys_info_2bis_par (index,
                                (T_D_SYS_INFO_2BIS *)sys_info_2_2bis,
                                new_2_2bis_list,
                                ncell_ext,
                                indicate_changes);
  }

}

#endif 
