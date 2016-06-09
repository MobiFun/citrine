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
|  Purpose :  This module holds the functions for handling primitives
|             sent from entity GRR to entity RR and vice versa.
+-----------------------------------------------------------------------------
*/

#ifndef RR_GPRS_C
#define RR_GPRS_C

#ifdef GPRS

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
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
#include "cnf_rr.h"
#include "pei.h"
#include "tok.h"
#include "rr_gprs.h"
#include "rr.h"

/*==== EXPORT =====================================================*/

/*==== PRIVATE =====================================================*/
LOCAL void pei_not_supported (void *data);

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

LOCAL const T_FUNC rrgrr_table[] = {
  MAK_FUNC_0( dat_rrgrr_channel_req,        RRGRR_CHANNEL_REQ             ), /* 0*/
  MAK_FUNC_0( dat_rrgrr_resumed_tbf_req,    RRGRR_RESUMED_TBF_REQ         ), /* 1*/
  MAK_FUNC_S( dat_rrgrr_data_req,           RRGRR_DATA_REQ                ), /* 2*/
  MAK_FUNC_0( att_rrgrr_start_mon_ccch_req, RRGRR_START_MON_CCCH_REQ      ), /* 3*/
  MAK_FUNC_0( att_rrgrr_stop_mon_ccch_req,  RRGRR_STOP_MON_CCCH_REQ       ), /* 4*/
  MAK_FUNC_0( att_rrgrr_cr_req,             RRGRR_CR_REQ                  ), /* 5*/
  MAK_FUNC_0( att_rrgrr_start_mon_bcch_req, RRGRR_START_MON_BCCH_REQ      ), /* 6*/
  MAK_FUNC_0( dat_rrgrr_rr_est_req,         RRGRR_RR_EST_REQ              ), /* 7*/
  MAK_FUNC_0( dat_rrgrr_rr_est_rsp,         RRGRR_RR_EST_RSP              ), /* 8*/
  MAK_FUNC_N( pei_not_supported,            0                             ), /* 9*/
  MAK_FUNC_0( dat_rrgrr_activate_req,       RRGRR_ACTIVATE_REQ            ), /* A*/
  MAK_FUNC_N( pei_not_supported,            0                             ), /* B*/
  MAK_FUNC_0( dat_rrgrr_gprs_data_req,      RRGRR_GPRS_DATA_REQ           ), /* C*/
  MAK_FUNC_0( dat_rrgrr_suspend_dcch_req,   RRGRR_SUSPEND_DCCH_REQ        ), /* D*/
  MAK_FUNC_0( dat_rrgrr_reconnect_dcch_req, RRGRR_RECONNECT_DCCH_REQ      ), /* E*/
  MAK_FUNC_0( att_check_bsic,               RRGRR_NCELL_SYNC_REQ          ), /* F*/
  MAK_FUNC_0( att_gprs_sync_req,            RRGRR_SYNC_REQ                ), /* 10*/
  MAK_FUNC_N( pei_not_supported,            0                             ), /* 11*/
  MAK_FUNC_0( att_rrgrr_update_ba_req,      RRGRR_UPDATE_BA_REQ           ), /* 12*/
  MAK_FUNC_N( att_rrgrr_meas_rep_req,       RRGRR_MEAS_REP_REQ            ), /* 13*/
  MAK_FUNC_N( pei_not_supported,            RRGRR_TBF_ESTABLISHED_REQ     ), /* 14*/
  MAK_FUNC_0( att_gprs_cr_rsp,              RRGRR_CR_RSP                  ), /* 15*/
  MAK_FUNC_0( att_rrgrr_ext_meas_req,       RRGRR_EXT_MEAS_REQ            ), /* 16*/
  MAK_FUNC_0( att_rrgrr_ext_meas_stop_req,  RRGRR_EXT_MEAS_STOP_REQ       ), /* 17*/
  MAK_FUNC_0( gprs_rrgrr_stop_task,         RRGRR_STOP_TASK_REQ           ), /* 18*/
  MAK_FUNC_0( att_gprs_start_task,          RRGRR_START_TASK_REQ          ), /* 19*/
  MAK_FUNC_0( att_rrgrr_standby_ind,        RRGRR_STANDBY_STATE_IND       ), /* 1A*/
  MAK_FUNC_0( att_rrgrr_ready_ind,          RRGRR_READY_STATE_IND         )  /* 1B*/
#ifdef REL99
  ,
  MAK_FUNC_0( att_rrgrr_cbch_info_ind,      RRGRR_CBCH_INFO_IND           )  /* 1C*/
#endif
};

GLOBAL const char * const    STATE_GPRS_NAME[] = {    STATE_GPRS_NAME_INIT };


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS ()         MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : gprs_init_gprs_data                 |
+-----------------------------------------------------------------------------+

  PURPOSE : initialises the RR related GPRS data.

*/

GLOBAL void gprs_init_gprs_data (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("gprs_init_gprs_data()");

  SET_STATE(STATE_GPRS, GPRS_NULL);

  memset(rr_data->gprs_data.rr_sdu,   NOT_PRESENT_8BIT, sizeof(rr_data->gprs_data.rr_sdu));
  memset(rr_data->gprs_data.si13_sdu, NOT_PRESENT_8BIT, sizeof(rr_data->gprs_data.si13_sdu));
  rr_data->gprs_data.tma_in_progress = 0;
  rr_data->gprs_data.fn              = 0;
  rr_data->gprs_data.req_ref_idx     = NOT_PRESENT_8BIT;
  rr_data->gprs_data.page_mode       = PAG_MODE_DEFAULT;
  rr_data->gprs_data.si13_received   = 0;
  rr_data->gprs_data.split_pg        = 0;

  rr_data->gprs_data.cr_type = NOT_PRESENT_8BIT;
  rr_data->gprs_data.cr_orig = NOT_PRESENT_8BIT;
  rr_data->gprs_data.start_proc = START_PROC_NOTHING;

  rr_data->gprs_data.is_nc2_used_in_si13 = FALSE;    /* NC0 is default */
  rr_data->gprs_data.nc_mode_of_pmo      = NC_MODE_RESET; /* Use NC of SI13 */
  rr_data->gprs_data.ready_state         = FALSE;    /* Standby */

  /*rr_data->gprs_data.cr_pbcch_active = 0;*/
  if ( rr_data->gprs_data.dl_data_ind NEQ NULL )
  {
    PFREE ( rr_data->gprs_data.dl_data_ind );
  }
  rr_data->gprs_data.dl_data_ind     = NULL;
#ifdef REL99
  rr_data->gprs_data.cbch_psi_valid = FALSE;
  rr_data->gprs_data.cbch_info_rxvd_in_ptm = FALSE;
  rr_data->gprs_data.nw_release = RR_GPRS_R97;
#endif
  rr_data->gprs_data.ba_bcch_modified= FALSE;
  
}

GLOBAL void gprs_init_data_cr (void)
{
  GET_INSTANCE_DATA;
  if(GET_STATE(STATE_GPRS) NEQ GPRS_NULL)
    SET_STATE(STATE_GPRS, GPRS_ACTIVATED);

  memset(rr_data->gprs_data.rr_sdu,   NOT_PRESENT_8BIT, sizeof(rr_data->gprs_data.rr_sdu));
  memset(rr_data->gprs_data.si13_sdu, NOT_PRESENT_8BIT, sizeof(rr_data->gprs_data.si13_sdu));
  rr_data->gprs_data.tma_in_progress = 0;
  rr_data->gprs_data.fn              = 0;
  rr_data->gprs_data.req_ref_idx     = NOT_PRESENT_8BIT;
  rr_data->gprs_data.page_mode       = PAG_MODE_DEFAULT;
  rr_data->gprs_data.si13_received   = 0;
  rr_data->gprs_data.split_pg        = 0;
  /*rr_data->gprs_data.cr_pbcch_active = 0;*/
  rr_data->gprs_data.cr_type = NOT_PRESENT_8BIT;
  rr_data->gprs_data.cr_orig = NOT_PRESENT_8BIT;
  rr_data->gprs_data.start_proc = START_PROC_NOTHING;
  rr_data->gprs_data.is_nc2_used_in_si13 = FALSE;    /* NC0 is default */
  rr_data->gprs_data.nc_mode_of_pmo      = NC_MODE_RESET; /* Use NC of SI13 */
  /* ready state variable should not be cleared know because it is still needed */ 

  if ( rr_data->gprs_data.dl_data_ind NEQ NULL )
  {
    PFREE ( rr_data->gprs_data.dl_data_ind );
  }
  rr_data->gprs_data.dl_data_ind     = NULL;
#ifdef REL99
  rr_data->gprs_data.cbch_psi_valid = FALSE;
  rr_data->gprs_data.cbch_info_rxvd_in_ptm = FALSE;
  rr_data->gprs_data.nw_release = RR_GPRS_R97;
#endif

}
/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (??6147)     MODULE  : RR_GPRS                           |
| STATE   : code                ROUTINE : gprs_get_table_n                    |
+-----------------------------------------------------------------------------+

  PURPOSE : process jumptable,
  IN      : adresses of table, n
  OUT     : values of rrgrr_table in table, n

*/
void gprs_get_table_n (const T_FUNC **table, USHORT *n)
{
  TRACE_FUNCTION ("gprs_get_table_n()");

  *table = rrgrr_table;
  *n     = TAB_SIZE ( rrgrr_table);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : gprs_rrgrr_store_sdu                |
+-----------------------------------------------------------------------------+

  PURPOSE :
  IN      :
  OUT     :

*/
void gprs_rrgrr_store_sdu(UBYTE* to, T_sdu* from)
{
   from->o_buf-=8;
   from->l_buf+=8;
   memcpy(to, &from->buf[from->o_buf/8], from->l_buf/8);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : gprs_rrgrr_fill_from_stored_sdu     |
+-----------------------------------------------------------------------------+

  PURPOSE :
  IN      :
  OUT     :

*/
void gprs_rrgrr_fill_from_stored_sdu(T_sdu* sdu, UBYTE* from)
{
        /*
         * copy message content
         */
        sdu->l_buf = 22*8;
#if defined (_SIMULATION_)
        sdu->o_buf = 0*8;
#else
        sdu->o_buf = 1*8;
#endif

        memcpy(sdu->buf, from, MAX_L2_FRAME_SIZE);/*lint !e419 Apparent data overrun for function*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : RR_PEI              |
| STATE   : code                       ROUTINE : pei_not_supported   |
+--------------------------------------------------------------------+

  PURPOSE : An unsupported primitive is received.

*/

LOCAL void pei_not_supported (void * data)
{
  TRACE_FUNCTION ("pei_not_supported()");

  PFREE (data);
}

#endif /* GPRS */
#endif /* RR_GPRS_C */
