/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  SMS_TLP
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
|  Purpose :  This Modul defines the functions for the transfer layer
|             capability of the module Short Message Service.
+-----------------------------------------------------------------------------
*/
#ifndef SMS_TLP_C
#define SMS_TLP_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SMS

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "message.h"
#include "ccdapi.h"
#include "prim.h"
#include "cus_sms.h"
#include "cnf_sms.h"
#include "mon_sms.h"
#include "pei.h"
#include "tok.h"
#include "sms.h"
#include "gdi.h"
#include "sms_em.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== TEST =======================================================*/

/*==== FUNCTIONS ==================================================*/

/* Implements Measure# 7 */
/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                              |
| STATE   : code                ROUTINE : tl_sim_rd_record_update_state        |
+------------------------------------------------------------------------------+

  PURPOSE : This routine process primitive SIM_READ_RECORD_CONF for the 
            cases MMI_FIND_FIRST and MMI_FIND_NEXT
            
*/

LOCAL BOOL tl_sim_rd_record_update_state(T_SIM_READ_RECORD_CNF *read_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION("tl_sim_rd_record_update_state()");  
   
  if ((GET_STATE(STATE_MMI) EQ MMI_FIND_FIRST 
       AND sms_data->sim_mem_update)
       OR (GET_STATE(STATE_MMI) EQ MMI_STATE_UPDATE 
       AND sms_data->sim_mem_update))
  {
    /*
     * Either the SMS record has been changed or both the SMS record
     * and the SMS status have been changed 
     */
     PFREE (read_cnf);
     sms_data->stored_mmi_state = MMI_IDLE;
     return TRUE; /* return; */
  }
  else if (GET_STATE(STATE_MMI) EQ MMI_STATE_UPDATE)
  {
    /*
     * Only SMS record's status has been changed. 
     */
    sms_data->stored_read_cnf = read_cnf;
    return TRUE;  /* return; */
  }
  sms_data->stored_mmi_state = MMI_IDLE;
  
  return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : tl_init                    |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the transfer layer.

*/

GLOBAL void tl_init ()
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_init()");

  SMS_ENT_STATE(sms_data) = SMS_STATE_NOT_AVAILABLE;

  memset (sms_data->data, 0, sizeof (sms_data->data));

  /* This is only for simulation tests*/
  sms_data->data[INST_MO].ti = 6; 

  SMS_SIM_MEM_AVAIL(sms_data)   = TRUE;
  SMS_MEM_CFG(sms_data)         = MEM_SM;
  SMS_MT_HANDLING(sms_data)     = MT_DEF;
  SMS_ST_REPORT(sms_data)       = DS0;
  SMS_SIM_PHASE(sms_data)       = PHASE_2_SIM;
  SMS_MT_ACK_MODE(sms_data)     = SMS_MHC_DEF;
  SMS_NETWORK(sms_data)         = NW_SIM_NONE;

  sms_data->init_done      = FALSE;

  SMS_SIM_PROP(sms_data).status_field = sms_data->sim_status_field;
  SMS_SIM_PROP(sms_data).pid_field  = sms_data->sim_pid_field;
  SMS_SIM_PROP(sms_data).mem_type   = MEM_SM;
  SMS_SIM_PROP(sms_data).max_record = 0;
  SMS_SIM_PROP(sms_data).any_valid  = FALSE;
  /* SMS_ME_PROP(sms_data).status_and_pid = sms_data->me_status_field; */
  /* SMS_ME_PROP(sms_data).status_and_pid = sms_data->me_pid_field; */
  SMS_ME_PROP(sms_data).status_field = NULL;
  SMS_ME_PROP(sms_data).pid_field   = NULL;
  SMS_ME_PROP(sms_data).mem_type    = MEM_ME;
  SMS_ME_PROP(sms_data).max_record  = 0;
  SMS_ME_PROP(sms_data).any_valid   =  FALSE;
  sms_data->sim_mem_update        = FALSE;
#ifdef SIM_TOOLKIT
  sms_data->download_sms    = FALSE;
  sms_data->file_update_ind = NULL;
  sms_data->stored_mmi_state = MMI_IDLE;
  sms_data->stored_read_cnf = NULL;
#endif
  SMS_RP_RCVD(sms_data) = NULL;
#ifdef FF_CPHS
  SMS_CPHS(sms_data) = FALSE;
#endif
#ifdef GPRS
  SMS_ROUTE_PREF(sms_data) = GPRS_SMS_CCT_ONLY;
#endif
  sms_data->pr_cntrl.delivery_state    = SMS_DELIVER_STATUS_RESUME;
  sms_data->concat_cntrl.concatenation = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_sim_abort_to_mmi        |
+--------------------------------------------------------------------+

  PURPOSE : Abort a SIM related SMS command to MMI.

*/

LOCAL void tl_sim_abort_to_mmi (USHORT error)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_MMI))
  {
  case MMI_REPLACE:
#ifdef REL99
    if(SMS_INST.failed_msg_retx EQ FALSE)
    {
#endif

      tl_mnsms_submit_cnf (SMS_INST.act_mem, SMS_INST.act_record,
                           SMS_INST.tp_mr_ret, error, NULL);
#ifdef REL99
    }
    else
    {
      tl_mnsms_retrans_cnf (SMS_INST.act_mem, SMS_INST.act_record,
                            SMS_INST.tp_mr_ret, error, NULL);
    }
#endif
    break;
  case MMI_READ:
    tl_mnsms_read_cnf (SMS_INST.act_mem,
                       SMS_INST.act_record, NULL, error);
    break;
  case MMI_WRITE:
    tl_mnsms_store_cnf (SMS_INST.act_mem, SMS_INST.act_record, error);
    break;
  case MMI_DELETE:
    tl_mnsms_delete_cnf (SMS_INST.act_mem, SMS_INST.act_record, 0, error);
    break;
  default:
    break;
  }
  if (sms_data->access_fifo[1] EQ ACCESS_BY_MMI)
  {
    sms_data->access_fifo[1] = ACCESS_EMPTY;
  }
  else if (sms_data->access_fifo[0] EQ ACCESS_BY_MMI)
  {
    sms_data->access_fifo[0] = ACCESS_EMPTY;
  }
  SET_STATE (STATE_MMI, MMI_IDLE);
#ifdef SIM_TOOLKIT
  if (sms_data->file_update_ind NEQ NULL)
  {
     T_SIM_FILE_UPDATE_IND *file_update_ind = sms_data->file_update_ind;
     sms_data->file_update_ind = NULL;
     tl_sim_file_update_ind (file_update_ind);  
  }
  if (sms_data->stored_read_cnf NEQ NULL)
  {
     /*
      * Resets the stored MMI state & read_cnf primitive
      */
     sms_data->stored_mmi_state = MMI_IDLE;  
     PFREE (sms_data->stored_read_cnf);
     sms_data->stored_read_cnf = NULL;
  }
#endif
}

/*---- PRIMITIVES --------------------------------------------------*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_configure_req     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_CONFIGURE_REQ.

*/

GLOBAL void tl_mnsms_configure_req (T_MNSMS_CONFIGURE_REQ *configure_req)
{
  register T_SMS_DATA *sms_data = GET_SMS_INSTANCE (0);

  TRACE_FUNCTION ("tl_mnsms_configure_req()");

  if (sms_data)
  {
     sms_data->mem3 = configure_req->pref_mem_3;
     sms_data->mt   = (configure_req->mt < MT_DEF)? configure_req->mt:
                                                MT_DEF;
     sms_data->ds   = configure_req->ds;
     sms_data->mhc  = configure_req->mhc;

     /*
      * If CMMS mode is enabled, store the current value of cmms_mode in sms_data
      */
     if(configure_req->v_cmms_mode)
     {
        sms_data->cmms_mode = configure_req->cmms_mode;
     }

     /*
      * For CMMS if user has set the value for TMMS timer then store that
      * value in sms_data
      */
     if(configure_req->v_tmms_val)
     {
        sms_data->timer_values[TMMS] = configure_req->tmms_val;
     }
  }
  PFREE (configure_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_mo_serv_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_MO_SERV_REQ.
            (set preferred destination (GPRS/GSM) for MO SM)

*/
#if defined GPRS
GLOBAL void tl_mnsms_mo_serv_req (T_MNSMS_MO_SERV_REQ *mo_serv_req)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_mnsms_mo_serv_req()");
  /*
   * send MNSMS_MO_SERV_CNF with configured pref. dst
   */
  {
    PALLOC (mo_serv_cnf, MNSMS_MO_SERV_CNF);
    memset (mo_serv_cnf, 0, sizeof (T_MNSMS_MO_SERV_CNF));

    if (sms_data) /*lint !e774 always True*/
    {
      sms_data->mo_dst_pref = mo_serv_req->mo_sms_serv;
      mo_serv_cnf->mo_sms_serv = sms_data->mo_dst_pref;

      TRACE_EVENT_P1("MO Path: %d", sms_data->mo_dst_pref);
    }
    else
    {
      mo_serv_cnf->mo_sms_serv = NOT_PRESENT_8BIT;
    }
    PSENDX (MMI, mo_serv_cnf);
  }
  PFREE (mo_serv_req);
}
#endif /* GPRS */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_command_cnf       |
+--------------------------------------------------------------------+

  PURPOSE : Generate a response to the primitive MNSMS_COMMAND_REQ.
*/

GLOBAL void tl_mnsms_command_cnf (UBYTE tp_mr,
                                  USHORT cause,
                                  T_sms_sdu *sms_sdu)
{
  PALLOC (command_cnf, MNSMS_COMMAND_CNF);

  command_cnf->tp_mr = tp_mr;
  command_cnf->cause = cause;

  if (sms_sdu NEQ NULL)
    memcpy (&command_cnf->sms_sdu, sms_sdu, sizeof (T_sms_sdu));
  else
    memset (&command_cnf->sms_sdu, 0, sizeof (T_sms_sdu));

  PSENDX (MMI, command_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_command_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_COMMAND_REQ.

*/

GLOBAL void tl_mnsms_command_req (T_MNSMS_COMMAND_REQ *command_req)
{
  register T_SMS_DATA *sms_data = GET_SMS_INSTANCE (0);

  TRACE_FUNCTION ("tl_mnsms_command_req()");

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_TL))
    {
      case TL_IDLE:
        if (sms_timer_check(TRAM))
        {
           tl_mnsms_command_cnf (NOT_PRESENT_8BIT, SMS_CAUSE_ENTITY_BUSY, NULL);
        }
        else
        {
#ifdef REL99
           SMS_INST.failed_msg_retx = FALSE;
           if(command_req->auto_rep_flag EQ AUTO_REP_FLAG_ENABLED)
           {
             SMS_INST.tl_retx = 0;
           }
           else
           {
             /* Set to the max value so that retransmission will not be done */
             SMS_INST.tl_retx = TL_MAX_RETANS;
           }
#endif
          /*
           * save sdu
           */
           if (SMS_SDU(sms_data) NEQ NULL)
           {
              MFREE (SMS_SDU(sms_data));
           }
           MALLOC (SMS_SDU(sms_data), sizeof(T_sms_sdu));
           memcpy (SMS_SDU(sms_data), &command_req->sms_sdu, sizeof(T_sms_sdu));
          /*
           * TL state transition TL_ESTABLISH
           * EST state transition EST_CMD
           */
           SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
           SET_STATE (STATE_EST, EST_CMD);
          /*
           * establish connection
           */
           tl_establish_connection(TRUE);
        }
        break;

      default:
        tl_mnsms_command_cnf (NOT_PRESENT_8BIT, SMS_CAUSE_ENTITY_BUSY, NULL);
    }
  }
  else
  {
    tl_mnsms_command_cnf (NOT_PRESENT_8BIT, SMS_CAUSE_ENTITY_BUSY, NULL);
  }
  PFREE (command_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_delete_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : Generate a response to the primitive MNSMS_DELETE_REQ.
*/

GLOBAL void tl_mnsms_delete_cnf (UBYTE mem_type,
                                 UBYTE record,
                                 UBYTE next_rec,
                                 USHORT error)
{
  PALLOC (delete_cnf, MNSMS_DELETE_CNF);

  delete_cnf->mem_type = mem_type;
  delete_cnf->rec_num = record;
  delete_cnf->cause = error;
  delete_cnf->delete_rec_next = next_rec;

  SMS_EM_DELETE_SHORT_MESSAGE;

  PSENDX (MMI, delete_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_delete_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_DELETE_REQ.

*/

GLOBAL void tl_mnsms_delete_req (T_MNSMS_DELETE_REQ *delete_req)
{
  register T_SMS_DATA *sms_data = GET_SMS_INSTANCE(0);
  UBYTE       rec_num;  

  TRACE_FUNCTION ("tl_mnsms_delete_req()");

#ifdef REL99
  /* Check if the last failed message is getting  deleted */
  if( ( delete_req->rec_num  EQ SMS_INST.failed_msg_rec_num) AND
      ( delete_req->mem_type EQ SMS_INST.failed_msg_mem ) AND 
       SMS_DATA_REQ(sms_data) NEQ NULL )
  {    
    PFREE (SMS_DATA_REQ(sms_data));
    SMS_DATA_REQ(sms_data) = NULL;
  }
#endif

  if (sms_data NEQ NULL)
  {
    switch (SMS_INST_GET_STATE (STATE_TL))
    {
    case TL_IDLE:
      switch (delete_req->mem_type)
      {
        case MEM_ME:
          /*
           * Mobile Equipment Memory
           */
          if(delete_req->delete_status NEQ CMGD_DEL_INDEX)
          {
            if(delete_req->rec_num EQ 0)
            {
              /* Status value is given. Find the first record satisfying this status value */
              rec_num = tl_search_record_for_delete (&SMS_ME_PROP(sms_data), 1,
                                                     delete_req->delete_status);
              if(rec_num EQ SMS_RECORD_NOT_EXIST)
              {
                tl_mnsms_delete_cnf (delete_req->mem_type, 0, 0, SIM_NO_ERROR);
                break;
              }
            } 
            else
            {
              if(delete_req->rec_num > sms_data->me_backup.max_record)
              {
                tl_mnsms_delete_cnf (delete_req->mem_type, delete_req->rec_num, 0, 
                                     (USHORT)((SMS_ME_PROP(sms_data).max_record EQ 0)?
                                     SMS_CAUSE_MEM_FAIL: SMS_CAUSE_INV_INDEX));
                break;
              }
              else
              {
                /* Use the record number that ACI has given */
                rec_num = delete_req->rec_num;
              }
            }
          }
          else
          {
            /* There should be a valid rec_num */
            if(delete_req->rec_num EQ 0 || delete_req->rec_num > sms_data->me_backup.max_record)
            {          
              tl_mnsms_delete_cnf (delete_req->mem_type, delete_req->rec_num, 0, 
                                   (USHORT)((SMS_ME_PROP(sms_data).max_record EQ 0)?
                                   SMS_CAUSE_MEM_FAIL: SMS_CAUSE_INV_INDEX));
              break;
            }
            else
            {
             rec_num = delete_req->rec_num;
            }
          }
          
          if (tl_get_status (&SMS_ME_PROP(sms_data), rec_num-1) & 1)
          {
            /*
             * entry is used
             */
            UBYTE data[SIZE_EF_SMS];
            UBYTE next_rec_num = 0;

            data[0] = 0;
            memset (&data[1], 0xFF, SIZE_EF_SMS-1);
            tl_store_status (&SMS_ME_PROP(sms_data), rec_num-1, SIM_SMS_FREE);
/* Implements Measure#32: Row 95 */
            pcm_WriteRecord ((UBYTE *)ef_sms_id, rec_num,
                             SIZE_EF_SMS, data);

            TRACE_EVENT_P1 ("mem cap avail %d", sms_data->mem_cap_avail);
            
            if (!sms_data->mem_cap_avail)
            {
              SMS_SEL_MEM(sms_data) = delete_req->mem_type;
              SMS_SEL_REC(sms_data) = delete_req->rec_num;

              GET_MO_INSTANCE(sms_data);
               /*
                * TL  state transition TL_ESTABLISH
                * EST state transition EST_SMMA
                * MMI state transition MMI_DELETE
                * 
                */
              SET_STATE (STATE_MMI, MMI_DELETE);
              SET_STATE (STATE_EST, EST_SMMA);
              SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
                 /*
                  * 1st shot
                  */
              SMS_INST.retrans  = FALSE;
                 /*
                  * establish connection
                  */
              tl_establish_connection(FALSE);
                 /*
                  * wait for processing of RP-SMMA
                  */
              PFREE (delete_req);    
              return;
            }
            
            if(delete_req->delete_status NEQ CMGD_DEL_INDEX) 
            {            
              /* Status value is given. Find the next record satisfying this status value */
              next_rec_num = tl_search_record_for_delete (&SMS_ME_PROP(sms_data), rec_num,
                                                          delete_req->delete_status);
            }

            tl_mnsms_delete_cnf (delete_req->mem_type, rec_num, next_rec_num, SIM_NO_ERROR);
          }
          else
          {         
            tl_mnsms_delete_cnf (delete_req->mem_type, rec_num, 0, SMS_CAUSE_INV_INDEX);
          }
                    
          break;
        
        case MEM_SM:
          /*
           * SIM memory
           */
          if(delete_req->delete_status NEQ CMGD_DEL_INDEX)
          {
            if(delete_req->rec_num EQ 0)
            {
              /* Status value is given. Find the first record satisfying this status value */
              rec_num = tl_search_record_for_delete (&SMS_SIM_PROP(sms_data), 1,
                                                     delete_req->delete_status);

              if(rec_num EQ SMS_RECORD_NOT_EXIST)
              {
                tl_mnsms_delete_cnf (delete_req->mem_type, 0, 0, SIM_NO_ERROR);
                break;
              }
 
            }
            else
            {
              if(delete_req->rec_num > sms_data->sim_backup.max_record)
              {
                tl_mnsms_delete_cnf (delete_req->mem_type, delete_req->rec_num, 0,
                                     SMS_CAUSE_INV_INDEX);
                break;
              }
              else
              {
                /* Use the record number that he has given */
                rec_num = delete_req->rec_num;
              }
            }            
          }
          else
          {
            /* There should be a valid rec_num */
            if(delete_req->rec_num EQ 0 || delete_req->rec_num > sms_data->sim_backup.max_record)
            {
              tl_mnsms_delete_cnf (delete_req->mem_type, delete_req->rec_num, 0,
                                   SMS_CAUSE_INV_INDEX);
              break;
            }
            else
            {
              rec_num = delete_req->rec_num;
            }
          }
          
          if (tl_get_status (&SMS_SIM_PROP(sms_data), rec_num-1) & 1)
          {
            /*
             * entry is used
             */
            SMS_SEL_MEM(sms_data) = delete_req->mem_type;
            SMS_SEL_REC(sms_data) = rec_num;
            SMS_REC_STATUS(sms_data) = delete_req->delete_status;

            tl_set_access_fifo (ACCESS_BY_MMI);
            SET_STATE (STATE_MMI, MMI_DELETE);
            SMS_INST_SET_STATE (STATE_TL, TL_OTHER);
            tl_sim_update_req (rec_num, SMS_RECORD_FREE, NULL);
          }
          else
          {         
            tl_mnsms_delete_cnf (delete_req->mem_type, rec_num, 0, SMS_CAUSE_INV_INDEX);
          }
          break;

        case MEM_SR:
          tl_mnsms_delete_cnf (delete_req->mem_type, delete_req->rec_num, 0,
                               SMS_CAUSE_MEM_FAIL);
          break;

        default:
          tl_mnsms_delete_cnf (delete_req->mem_type, delete_req->rec_num, 0,
                               SMS_CAUSE_PARAM_WRONG);
          break;
      }
      break;

    default:
      tl_mnsms_delete_cnf (delete_req->mem_type, delete_req->rec_num, 0,
                           SMS_CAUSE_ENTITY_BUSY);
      break;
    }
  }
  else
    tl_mnsms_delete_cnf (delete_req->mem_type, delete_req->rec_num, 0,
                         SMS_CAUSE_ENTITY_BUSY);
  PFREE (delete_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_read_cnf          |
+--------------------------------------------------------------------+

  PURPOSE : Generate an error response to the primitive
            MNSMS_READ_REQ.
*/

GLOBAL void tl_mnsms_read_cnf (UBYTE mem_type,
                               UBYTE record,
                               UBYTE *data,
                               USHORT error)
{
  GET_INSTANCE_DATA;
  PALLOC (read_cnf, MNSMS_READ_CNF); /* T_MNSMS_READ_CNF */

  read_cnf->mem_type = mem_type;
  read_cnf->rec_num = record;
  switch (mem_type)
  {
  case MEM_SM:
    if (sms_data->init_done)
    {    
      read_cnf->rec_next = tl_search_record (&SMS_SIM_PROP(sms_data),
                                             (USHORT)(record + 1),
                                             SMS_REC_STATUS(sms_data));
      read_cnf->rec_max = SMS_SIM_PROP(sms_data).max_record;
    }
    else
    {
      read_cnf->rec_next = SMS_RECORD_NOT_EXIST;
      read_cnf->rec_max = SMS_RECORD_NOT_EXIST;
    }
    break;
  case MEM_ME:
    read_cnf->rec_next = tl_search_record (&SMS_ME_PROP(sms_data),
                                           (USHORT)(record + 1),
                                           SMS_REC_STATUS(sms_data));
    read_cnf->rec_max = SMS_ME_PROP(sms_data).max_record;
    break;
  default:
    read_cnf->rec_next = SMS_RECORD_NOT_EXIST;
    read_cnf->rec_max = SMS_RECORD_NOT_EXIST;
    break;
  }
  read_cnf->cause = error;
  read_cnf->rec_status = SMS_RECORD_NOT_EXIST;  /* ffs */

  if (data NEQ NULL)
  {
    read_cnf->status = data[0];
    read_cnf->sms_sdu.l_buf = SIM_PDU_BIT_LEN;
    read_cnf->sms_sdu.o_buf = 0;
    memcpy (read_cnf->sms_sdu.buf, &data[1], SIM_PDU_LEN);
    tl_adjust_message_len (SMS_VT_SIM_PDU, (BUF_tpdu *)&read_cnf->sms_sdu);
  }
  else
  {
    if (record EQ SMS_RECORD_NOT_EXIST)
    {
      read_cnf->status = SMS_RECORD_FREE;
    }
    else
    {
      switch (mem_type)
      {
        case MEM_ME:
          read_cnf->status = tl_get_status (&SMS_ME_PROP(sms_data), record-1);
          break;
        case MEM_SM:
          read_cnf->status = tl_get_status (&SMS_SIM_PROP(sms_data), record-1);
          break;
        default: 
          read_cnf->status = SMS_RECORD_FREE;
          break;
      }
    }
    memset (&read_cnf->sms_sdu, 0, sizeof(T_sms_sdu));
  }
  PSENDX (MMI, read_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_read_req          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_READ_REQ.

*/

GLOBAL void tl_mnsms_read_req (T_MNSMS_READ_REQ *read_req)
{
  register T_SMS_DATA *sms_data = GET_SMS_INSTANCE(0);

  TRACE_FUNCTION ("tl_mnsms_read_req()");

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_TL))
    {
      case TL_IDLE:
        SMS_READ_MODE(sms_data) = read_req->read_mode;
        switch (read_req->mem_type)
        {
          case MEM_ME:     /* Mobile Equipment Memory */
            tl_read_me_memory (read_req->rec_num,
                               read_req->status);
            break;
          case MEM_SM:     /* SIM memory */
            if (sms_data->init_done)
            {
              tl_read_sim_memory (read_req->rec_num,
                                  read_req->status);
            }
            else
            {
              tl_mnsms_read_cnf (read_req->mem_type,
                       read_req->rec_num, NULL, SMS_CAUSE_ENTITY_BUSY);
            }
            break;
          case MEM_SR:
            tl_mnsms_read_cnf (read_req->mem_type,
                               read_req->rec_num, NULL,
                               SMS_CAUSE_MEM_FAIL);
            break;
          default:
            tl_mnsms_read_cnf (read_req->mem_type,
                               read_req->rec_num, NULL,
                               SMS_CAUSE_PARAM_WRONG);
            break;
        }
        break;

      default:
        tl_mnsms_read_cnf (read_req->mem_type,
                           read_req->rec_num, NULL, SMS_CAUSE_ENTITY_BUSY);
        break;
    }
  }
  else
    tl_mnsms_read_cnf (read_req->mem_type,
                       read_req->rec_num, NULL, SMS_CAUSE_ENTITY_BUSY);

  PFREE (read_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_store_cnf         |
+--------------------------------------------------------------------+

  PURPOSE : Generate a response to the primitive MNSMS_STORE_REQ.
*/

GLOBAL void tl_mnsms_store_cnf (UBYTE mem_type,
                                UBYTE record,
                                USHORT error)
{
  PALLOC (store_cnf, MNSMS_STORE_CNF);

  store_cnf->mem_type = mem_type;
  store_cnf->rec_num = record;
  store_cnf->cause = error;

  PSENDX (MMI, store_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_store_req         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_STORE_REQ.

*/

GLOBAL void tl_mnsms_store_req (T_MNSMS_STORE_REQ *store_req)
{
  USHORT index;
  UBYTE dummy;
  int byte_len;
  BOOL mem_full = FALSE;
  register T_SMS_DATA *sms_data = GET_SMS_INSTANCE(0);

  TRACE_FUNCTION ("tl_mnsms_store_req()");

#ifdef REL99
  /* Check if the last failed message is getting replaced */
  if( ( store_req->condx    NEQ SMS_CONDX_OVR_NON    ) AND
      ( store_req->rec_num  EQ SMS_INST.failed_msg_rec_num ) AND
      ( store_req->mem_type EQ SMS_INST.failed_msg_mem ) AND 
       SMS_DATA_REQ(sms_data) NEQ NULL )
  {    
    PFREE (SMS_DATA_REQ(sms_data));
    SMS_DATA_REQ(sms_data) = NULL;
  }  
#endif

  if (sms_data NEQ NULL)
  {
    switch (SMS_INST_GET_STATE (STATE_TL))
    {
    case TL_IDLE:
      if (tl_check_status_value (store_req->status) EQ NOT_PRESENT_8BIT)
      {                    /* invalid status value */
        tl_mnsms_store_cnf (store_req->mem_type, store_req->rec_num,
                            SMS_CAUSE_PARAM_WRONG);
        break;
      }
      switch (store_req->mem_type)
      {
      case MEM_SM:
        if (store_req->rec_num EQ SMS_RECORD_NOT_EXIST)
        {
          if ((index = tl_get_free_space (MEM_SM)) EQ 0)
            mem_full = TRUE;

        SMS_EM_SET_CHANGE_FLAG_1;

        }
        else
        {
          if (store_req->rec_num > SMS_SIM_PROP(sms_data).max_record)
            index = 0;
          else if (store_req->condx EQ SMS_CONDX_OVR_NON)
          {
            if ((tl_get_status (&SMS_SIM_PROP(sms_data), store_req->rec_num-1) & 1)
                EQ SMS_RECORD_FREE)
              index = store_req->rec_num;
            else
              index = 0;
          }
          else if (store_req->condx EQ SMS_CONDX_OVR_MO)
          {
            switch (tl_get_status (&SMS_SIM_PROP(sms_data), store_req->rec_num-1) & 7)
            {
            case SMS_RECORD_STO_UNSENT:
            case SMS_RECORD_STO_SENT:
              index = store_req->rec_num;
              break;

            default:
              index = 0;
              break;
            }
          }
          else
            index = store_req->rec_num;

        SMS_EM_SET_CHANGE_FLAG_2;

        }
        break;

      case MEM_ME:
        if (store_req->rec_num EQ SMS_RECORD_NOT_EXIST)
        {
          if ((index = tl_get_free_space (MEM_ME)) EQ 0)
            mem_full = TRUE;


          SMS_EM_SET_CHANGE_FLAG_1;

        }
        else
        {
          if (store_req->condx EQ SMS_CONDX_OVR_NON)
          {
            if ((tl_get_status (&SMS_ME_PROP(sms_data), store_req->rec_num-1) & 1)
                EQ SMS_RECORD_FREE)
              index = store_req->rec_num;
            else
              index = 0;
          }
          else if (store_req->condx EQ SMS_CONDX_OVR_MO)
          {
            switch (tl_get_status (&SMS_ME_PROP(sms_data), store_req->rec_num-1) & 7)
            {
            case SMS_RECORD_STO_UNSENT:
            case SMS_RECORD_STO_SENT:
              index = store_req->rec_num;
              break;

            default:
              index = 0;
              break;
            }
          }
          else
            index = store_req->rec_num;

        SMS_EM_SET_CHANGE_FLAG_2;

        }
        break;

      case MEM_SR:
        tl_mnsms_store_cnf (store_req->mem_type, store_req->rec_num,
                            SMS_CAUSE_MEM_FAIL);
        PFREE (store_req);
        return;

      default:
        tl_mnsms_store_cnf (store_req->mem_type, store_req->rec_num,
                            SMS_CAUSE_PARAM_WRONG);
        PFREE (store_req);
        return;
      }
      if (index > 0)
      {
        /*
         * storage available
         */
        byte_len = (int)store_req->sms_sdu.l_buf >> 3;

        if (store_req->mem_type EQ MEM_SM)
        {
          /*
           * SIM card memory
           */
          tl_store_status (&SMS_SIM_PROP(sms_data), index-1, SIM_SMS_PENDING);
          SMS_REC_STATUS(sms_data) = store_req->status;
          tl_get_pid_dcs (store_req->status,
                          &store_req->sms_sdu,
                          &SMS_PID(sms_data),
                          &dummy);

          memset (&store_req->sms_sdu.buf[byte_len],
                  NOT_PRESENT_8BIT,
                  (SIM_LENGTH_SMS_RECORD-1) - byte_len);

          tl_set_access_fifo (ACCESS_BY_MMI);
          SET_STATE (STATE_MMI, MMI_WRITE);
          SMS_INST_SET_STATE (STATE_TL, TL_OTHER);
          tl_sim_update_req ((UBYTE)index, store_req->status,
                             store_req->sms_sdu.buf);
          break;
        }
        else
        {
          /*
           * memory type is mobile memory
           */
          UBYTE data[SIZE_EF_SMS];

          data[0] =  store_req->status;
          memcpy (&data[1], store_req->sms_sdu.buf, byte_len);
          memset (&data[byte_len + 1],
                  NOT_PRESENT_8BIT,
                  (SIM_LENGTH_SMS_RECORD-1) - byte_len);

/* Implements Measure#32: Row 97 */
          if (pcm_WriteRecord ((UBYTE *)ef_sms_id, index,
                               SIZE_EF_SMS, data) EQ PCM_OK)
          {
            UBYTE pid;

            SMS_EM_STORE_MO_SHORT_MESSAGE;
            
            SMS_EM_CHANGE_SHORT_MESSAGE;

            tl_store_status (&SMS_ME_PROP(sms_data), index-1, store_req->status);
            tl_get_pid_dcs (store_req->status,
                            &store_req->sms_sdu,
                            &pid,
                            &dummy);
            tl_store_pid (&SMS_ME_PROP(sms_data), index-1, pid);
            tl_mnsms_store_cnf (store_req->mem_type, (UBYTE)index,
                                SMS_NO_ERROR);
          }
          else
            tl_mnsms_store_cnf (store_req->mem_type, (UBYTE)index,
                                SMS_CAUSE_MEM_FAIL);
        }
      }
      else
        tl_mnsms_store_cnf (store_req->mem_type, store_req->rec_num,
                            (USHORT)((mem_full)? SMS_CAUSE_MEM_FULL:
                                                 SMS_CAUSE_INV_INDEX));
      break;

    default:
      tl_mnsms_store_cnf (store_req->mem_type, store_req->rec_num,
                          SMS_CAUSE_ENTITY_BUSY);
      break;
    }
  }
  else
    tl_mnsms_store_cnf (store_req->mem_type, store_req->rec_num,
                        SMS_CAUSE_ENTITY_BUSY);

  PFREE (store_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_submit_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : Generate a response to the primitive MNSMS_SUBMIT_REQ.
*/

GLOBAL void tl_mnsms_submit_cnf (UBYTE mem_type,
                                 UBYTE record,
                                 UBYTE tp_mr,
                                 USHORT cause,
                                 T_sms_sdu *sms_sdu)
{
  PALLOC (submit_cnf, MNSMS_SUBMIT_CNF);

  TRACE_EVENT_P1 ("cause = %04x", cause);

  submit_cnf->mem_type = mem_type;
  submit_cnf->rec_num = record;
  submit_cnf->tp_mr = tp_mr;
  submit_cnf->cause = cause;

  if (sms_sdu NEQ NULL)
    memcpy (&submit_cnf->sms_sdu, sms_sdu, sizeof (T_sms_sdu));
  else
    memset (&submit_cnf->sms_sdu, 0, sizeof (T_sms_sdu));

  PSENDX (MMI, submit_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_submit_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_SUBMIT_REQ.
*/


GLOBAL void tl_mnsms_submit_req (T_MNSMS_SUBMIT_REQ *submit_req)
{
  UBYTE                      ti;
  register T_SMS_DATA                 *sms_data = GET_SMS_INSTANCE(0);

  TRACE_FUNCTION ("tl_mnsms_submit_req()");

  TRACE_EVENT_P2 ("mem_type = %02X, rec_num = %02X",
                  submit_req->mem_type, submit_req->rec_num);

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_TL))
    {
      case TL_SEND_CONTD:
       /*
        * stop timer TLCT
        */
        sms_timer_stop(TLCT);
       /*
        * stop timer TMMS
        */
        sms_timer_stop(TMMS);

      case TL_IDLE:
        if (sms_timer_check(TRAM))
        {
          /*
           * MNSMS_SUBMIT_CNF
           */
           tl_mnsms_submit_cnf (submit_req->mem_type, submit_req->rec_num,
                               NOT_PRESENT_8BIT, SMS_CAUSE_ENTITY_BUSY, NULL);
           break;
        }
       /*
        * save submit request with sms sdu
        */
        SMS_SEL_MEM(sms_data) = submit_req->mem_type;
        SMS_SEL_REC(sms_data) = submit_req->rec_num;
        SMS_CONDX(sms_data)   = submit_req->condx;
        SMS_MODIFY(sms_data)  = submit_req->modify;

        if (SMS_SDU(sms_data) NEQ NULL)
        {
           MFREE (SMS_SDU(sms_data));
        }
        MALLOC (SMS_SDU(sms_data), sizeof(T_sms_sdu));
        memcpy (SMS_SDU(sms_data), &submit_req->sms_sdu, sizeof(T_sms_sdu));
       /*
        * check whether concatenated
        */
        if (!tl_concat_check(SMS_SDU(sms_data)))
        {
           /*
            * error understanding the sdu
            */
            TRACE_ERROR ("error with concat sim pdu");
           /*
            * negative response MNSMS_SUBMIT_CNF
            */
            tl_mnsms_submit_cnf (submit_req->mem_type, submit_req->rec_num,
                             NOT_PRESENT_8BIT, SMS_CAUSE_PARAM_WRONG, NULL);
           /*
            * Send MNSMS_REPORT_IND if cmms_mode is equal 1or 2
            */
            if(CMMS_ACTIVE)
            {
               tl_mnsms_cmms_end_ind();
            }
           /*
            * give up
            */
            MFREE (SMS_SDU(sms_data));
            PFREE (submit_req);
            return;
        }
#ifdef REL99
        /* Check whether MO msg is sent thru retransmission request. Initially False */ 
        SMS_INST.failed_msg_retx = FALSE;
        if(submit_req->auto_rep_flag EQ AUTO_REP_FLAG_ENABLED)
        {
          SMS_INST.tl_retx = 0;
        }
        else
        {
          /* Set to the max value so that retransmission will not be done */
          SMS_INST.tl_retx = TL_MAX_RETANS;
        }
#endif

        
       /*
        * TL  state transition TL_ESTABLISH
        * EST state transition EST_SEND
        */
        SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
        SET_STATE (STATE_EST, EST_SEND);
       /*
        * establish connection
        */
        ti = SMS_INST.ti;
        tl_establish_connection(TRUE);
       /*
        * concat control: if needed, release previous connection
        */
        if ((sms_data->concat_cntrl.concatenation EQ TRUE) AND 
            (sms_data->concat_cntrl.release_pending EQ TRUE))
        {
           sms_data->concat_cntrl.release_pending = FALSE;
          /*
           * RL_RELEASE_REQ ==>
           */
           rl_release_req(ti);
          /*
           * check for end of concatenation
           */
           if (sms_data->concat_cntrl.end EQ TRUE)
           {
              /*
               * end of concatenation
               */
               sms_data->concat_cntrl.concatenation = FALSE;
           }
           break;
        }
       /*
        * cmms control: if needed, release previous connection
        */
        if(sms_data->cmms_release_pending EQ TRUE)
        {
           sms_data->cmms_release_pending = FALSE;
           /*
            * Release the previous active MM connection 
            */
           rl_release_req(ti);
        }
        break;

      default:
       /*
        * MNSMS_SUBMIT_CNF
        */
        tl_mnsms_submit_cnf (submit_req->mem_type, submit_req->rec_num,
                             NOT_PRESENT_8BIT, SMS_CAUSE_ENTITY_BUSY, NULL);
    }
  }
  else
  {
   /*
    * MNSMS_SUBMIT_CNF
    */
    tl_mnsms_submit_cnf (submit_req->mem_type, submit_req->rec_num,
                         NOT_PRESENT_8BIT, SMS_CAUSE_ENTITY_BUSY, NULL);
  }
  PFREE (submit_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_error_ind         |
+--------------------------------------------------------------------+

  PURPOSE : Build and send the primitive MNSMS_ERROR_IND.
*/

GLOBAL void tl_mnsms_error_ind (USHORT error)
{
  PALLOC (error_ind, MNSMS_ERROR_IND);

  error_ind->cause = error;

  PSENDX (MMI, error_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_ack_res           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_ACK_RES.
*/

GLOBAL void tl_mnsms_ack_res (T_MNSMS_ACK_RES *ack_res)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_mnsms_ack_res()");

  if (sms_data) /*lint !e774 always True*/
  {
    GET_MT_INSTANCE(sms_data);  /* MT operation */

    if (SMS_MT_ACK_MODE(sms_data) NEQ SMS_MHC_PH2PLUS)
    {
      tl_mnsms_error_ind (SMS_CAUSE_OPER_NOT_ALLW);
    }
    else switch (SMS_INST_GET_STATE (STATE_TL))
    {
      case TL_RECEIVE:
        if (sms_timer_check(TR2M))
        {
          T_rp_user_data *rp_ud;
          int sdu_ofs;
          MALLOC (rp_ud, sizeof(T_rp_user_data));

          CCD_START;

          sdu_ofs = ack_res->sms_sdu.buf[ack_res->sms_sdu.o_buf >> 3] + 1;
          rp_ud->tpdu.o_tpdu = 0;
          if (ack_res->sms_sdu.l_buf >= (sdu_ofs << 3) + 2) /* TP-MTI present */
          {
            rp_ud->tpdu.l_tpdu = ack_res->sms_sdu.l_buf - (sdu_ofs << 3);
            memcpy (rp_ud->tpdu.b_tpdu, &ack_res->sms_sdu.buf[sdu_ofs],
                    rp_ud->tpdu.l_tpdu >> 3);
            rp_ud->tp_mti = rp_ud->tpdu.b_tpdu[0] & 3;
            rp_ud->v_tpdu = TRUE;
          }
          else
          {
            rp_ud->tpdu.l_tpdu = 0;
            rp_ud->tp_mti = NOT_PRESENT_8BIT;
            rp_ud->v_tpdu = FALSE;
          }
          if (ack_res->resp EQ SMS_RP_ACK)
            rl_report_req_ack (rp_ud);
          else
          {
            if (!rp_ud->v_tpdu)
            {
              rp_ud->tp_mti = rp_ud->tpdu.b_tpdu[0] = SMS_DELIVER_REPORT;
              rp_ud->tpdu.b_tpdu[1] = SMS_FCS_UNSPECIFIED;  /* TP-FCS */
              rp_ud->tpdu.b_tpdu[2] = 0;                    /* TP-PI */
              rp_ud->tpdu.l_tpdu = 24;                      /* 3 bytes */
              rp_ud->v_tpdu = TRUE;
            }
            rl_report_req_error (SMS_RP_CS_PROTOCOL_ERROR, rp_ud);
           /*
            * RL_RELEASE_REQ ==>
            */
            rl_release_req(SMS_INST.ti);
          }
          CCD_END;
          MFREE (rp_ud);

          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
          break;
        }
        /*FALLTHROUGH*/ /*lint -fallthrough*/
      default:
        tl_mnsms_error_ind (SMS_CAUSE_UNEXP_CNMA);
        break;
    }
  }
  PFREE (ack_res);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_pause_req         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_PAUSE_REQ.
*/
GLOBAL void tl_mnsms_pause_req (T_MNSMS_PAUSE_REQ *pause_req)
{

  TRACE_FUNCTION ("tl_mnsms_pause_req()");

  tl_pause();

  PFREE (pause_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_resume_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_RESUME_REQ.
*/
GLOBAL void tl_mnsms_resume_req (T_MNSMS_RESUME_REQ *resume_req)
{

  TRACE_FUNCTION ("tl_mnsms_resume_req()");

  tl_resume();

  PFREE (resume_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_query_req         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_QUERY_REQ.
*/
GLOBAL void tl_mnsms_query_req (T_MNSMS_QUERY_REQ *query_req)
{

  TRACE_FUNCTION ("tl_mnsms_query_req()");

  switch (query_req->query_type)
  {
  case SMS_QUERY_DELIVER_STATUS:
     tl_query_deliver_status();
     break;
  default:
     TRACE_ERROR("sms query type unknown!");
  }

  PFREE (query_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_resume_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : Generate a response to the primitive MNSMS_RESUME_REQ.
*/

GLOBAL void tl_mnsms_resume_cnf ( USHORT cause )
{
  PALLOC (resume_cnf, MNSMS_RESUME_CNF);

  resume_cnf->cause = cause;

  PSENDX (MMI, resume_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_query_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : Generate a response to the primitive MNSMS_QUERY_REQ.
*/

GLOBAL void tl_mnsms_query_cnf ( U8 query_type, U8 status )
{

  switch (query_type)
  {
  case SMS_QUERY_DELIVER_STATUS:
     {
     PALLOC (query_cnf, MNSMS_QUERY_CNF);

     query_cnf->query_type       = SMS_QUERY_DELIVER_STATUS;
     query_cnf->v_deliver_status = 1;
     query_cnf->deliver_status   = status;

     PSENDX (MMI, query_cnf);
     }
     break;
  default:
     TRACE_ERROR("sms query type unknown!");
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_sim_sms_insert_ind      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_SMS_INSERT_IND.

*/

GLOBAL void tl_sim_sms_insert_ind (T_SIM_SMS_INSERT_IND *insert_ind)
{
  UBYTE      i;
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_sim_sms_insert_ind()");

  if (sms_data) /*lint !e774 always True*/
  {
    tl_sim_abort_to_mmi (SMS_CAUSE_SIM_BUSY);
    sms_data->init_done = FALSE;
    tl_mnsms_report_ind (SMS_ENT_STATE(sms_data) = SMS_STATE_INITIALISING);
    /*
     * copy SMS initialisation parameter
     */
    SMS_SIM_PHASE(sms_data)     = insert_ind->phase;
    SMS_INST.tp_mr              = insert_ind->tp_mr;
    /*
     * memory capacity exceeded notification flag
     */
    switch (insert_ind->mem_cap_avail)
    {
    case SIM_SMS_MEM_FULL:
       /* 
        * Notification Flag = TRUE
        */
       TRACE_EVENT("SIM_SMS_MEM_FULL => Notification Flag = TRUE");
       break;
    case SIM_SMS_MEM_AVAIL:
       /* 
        * Notification Flag = FALSE
        */
       TRACE_EVENT("SIM_SMS_MEM_AVAIL => Notification Flag = FALSE");
       break;
    case SIM_SMS_NO_MEM:
       /* 
        * SMS storage on SIM not present
        */
       TRACE_EVENT("SIM_SMS_NO_MEM");
       break;
    default:
       TRACE_ERROR("invalid mem cap avail information from sim");
    }
    SMS_SIM_MEM_AVAIL(sms_data) = insert_ind->mem_cap_avail & 1;
    SMS_SAT_DWNLD(sms_data)     = insert_ind->download_sms;

    /*
     * scan status and protocol identifier for all SMS entries
     * first one in tl_find_first, the rest with the for loop
     * in mobile memory
     */
    tl_find_first (MEM_ME);
    for (i = 2; i <= sms_data->me_backup.max_record; i++)
      tl_find_next (MEM_ME, i);

    /*
     * check for existing but empty memory
     */
    if (SMS_ME_PROP(sms_data).max_record > 0
         AND !SMS_ME_PROP(sms_data).any_valid)
      tl_message_ind_from_sim (MEM_ME, 0,
                               SMS_ME_PROP(sms_data).max_record,
                               NULL);
    /*
     * scan status and protocol identifier for all SMS entries
     * in sim memory
     */
#ifdef FF_CPHS
    SET_STATE (STATE_MMI, MMI_FIND_FIRST);
    tl_sim_read_req (SIM_CPHS_VMW, 1);
#elif  defined FF_SMS_NW_RCG_SIM
    SET_STATE (STATE_MMI, MMI_FIND_FIRST);
    tl_sim_read_req (SIM_IMSI, 9);
#else
    tl_find_first (MEM_SM);
#endif
  }
  PFREE (insert_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_sim_read_cnf            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_READ_CNF.
*/
#if defined(SIM_TOOLKIT) OR defined(FF_CPHS) OR defined(FF_SMS_NW_RCG_SIM)
GLOBAL void tl_sim_read_cnf (T_SIM_READ_CNF *read_cnf)
{
  GET_INSTANCE_DATA;
  USHORT  datafield;

  TRACE_FUNCTION ("tl_sim_file_update_ind()");

  datafield   = sms_data->sms_sim_access_info[read_cnf->req_id].datafield; 
  sms_data->sms_sim_access_info[read_cnf->req_id].entry_used = FALSE;

  if (sms_data)
  {
    switch (GET_STATE (STATE_MMI))
    {
    case MMI_STATE_UPDATE:
      /* 
       * EF_SMSS has been updated by SAT.
       */      
      if (read_cnf->cause EQ SIM_NO_ERROR)
      {
        /* Fetch TP-MR value out of EF_SMSS */
        SMS_INST.tp_mr = read_cnf->trans_data[0];

        if (sms_data->mem_cap_avail NEQ (read_cnf->trans_data[1] & 1))
        {
          /*
           * The memory capacity available flag has been changed.
           * Re-read EF_SMS also even in case this has not been announced
           * by the SIM application.
           */
          sms_data->mem_cap_avail = read_cnf->trans_data[1] & 1;
          sms_data->sim_mem_update = TRUE;
        }
        else
        {
          SET_STATE (STATE_MMI, MMI_IDLE);
        }
      }
      if (sms_data->sim_mem_update)
      {
        tl_mnsms_report_ind (SMS_ENT_STATE(sms_data) = SMS_STATE_INITIALISING);
        tl_find_first (MEM_SM);
#ifdef SIM_TOOLKIT
        if (sms_data->stored_read_cnf NEQ NULL)
        {
          /*
           * Resets the stored MMI state & read_cnf primitive
           */
          sms_data->stored_mmi_state = MMI_IDLE;
          PFREE (sms_data->stored_read_cnf);
          sms_data->stored_read_cnf = NULL;
        }
#endif /* SIM_TOOLKIT */
      }
      else
      {
        PALLOC (file_update_res, SIM_FILE_UPDATE_RES);
        file_update_res->fu_rsc = SIM_FU_SUCCESS;
        file_update_res->source = SRC_SMS;
        PSENDX (SIM, file_update_res);
      }
#ifdef SIM_TOOLKIT
      /*
       * If the SMS has not yet completed reading all the records, start from
       * the place where it has left earlier.
       */
      if( sms_data->stored_read_cnf NEQ NULL)
      {
        SET_STATE(STATE_MMI, sms_data->stored_mmi_state);
        tl_set_access_fifo (ACCESS_BY_MMI);
        tl_sim_read_record_cnf(sms_data->stored_read_cnf);
        sms_data->stored_mmi_state = MMI_IDLE;
        sms_data->stored_read_cnf = NULL;
      }
#endif /* SIM_TOOLKIT */

      break;
#ifdef FF_CPHS
    case MMI_FIND_FIRST:   /* CPHS Voice Message Waiting flag existent? */
#ifdef FF_SMS_NW_RCG_SIM
      if ( datafield EQ SIM_CPHS_VMW)
      {
        SMS_CPHS(sms_data) = read_cnf->cause EQ SIM_NO_ERROR;
        tl_sim_read_req (SIM_IMSI, 9);
        break;
      }
      else if (datafield EQ SIM_IMSI
               AND read_cnf->cause EQ SIM_NO_ERROR)
      {
        tl_check_network_on_sim (read_cnf->trans_data);
      }
      tl_find_first (MEM_SM);
#else
      SMS_CPHS(sms_data) = datafield EQ SIM_CPHS_VMW
                           AND read_cnf->cause EQ SIM_NO_ERROR;
      tl_find_first (MEM_SM);
#endif
#elif defined FF_SMS_NW_RCG_SIM
    case MMI_FIND_FIRST:   /* SIM operator determination required? */
      if (read_cnf->cause EQ SIM_NO_ERROR)
        tl_check_network_on_sim (read_cnf->trans_data);
      tl_find_first (MEM_SM);
      break;
#endif
    default:
      break;
    }
  }
  PFREE (read_cnf);
}
#endif  /* SIM_TOOLKIT OR FF_CPHS */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sim_update_rec_cnf_mmi  |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function TL_SIM_UPDATE_REC_CNF_MMI.

*/

LOCAL void tl_sim_update_rec_cnf_mmi (
                         T_SIM_UPDATE_RECORD_CNF *update_cnf)
{
  GET_INSTANCE_DATA;
  UBYTE next_rec_num=0,record;
  TRACE_FUNCTION ("tl_sim_update_rec_cnf_mmi()");

  record   = sms_data->sms_sim_access_info[update_cnf->req_id].rec_num;
  sms_data->sms_sim_access_info[update_cnf->req_id].entry_used = FALSE;

  switch (GET_STATE (STATE_MMI))
  {
    case MMI_READ_STATE_UPDATE:
      if (update_cnf->cause EQ SIM_NO_ERROR AND
          record EQ SMS_SEL_REC(sms_data))
      {
        /*
         * update status byte
         */
        tl_store_status (&SMS_SIM_PROP(sms_data), record-1, 
          SMS_RECORD_REC_READ);
      }
      if (SMS_READ_MODE(sms_data) EQ READ_STATUS_CHANGE)
      {
        tl_mnsms_read_cnf (MEM_SM, SMS_SEL_REC(sms_data),
                           NULL, SIM_NO_ERROR);
      }
      else
      {
        tl_mnsms_read_cnf (MEM_SM, SMS_SEL_REC(sms_data),
                           SMS_SIM_READ(sms_data)->linear_data,
                           SIM_NO_ERROR);
        PFREE (SMS_SIM_READ(sms_data));
        SMS_SIM_READ(sms_data) = NULL;
      }
      SMS_READ_MODE(sms_data) = READ_NORMAL;
      SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      break;

    case MMI_WRITE:
      if (update_cnf->cause EQ SIM_NO_ERROR)
      {
        /*
         * update status byte
         */
        tl_store_status (&SMS_SIM_PROP(sms_data), record-1, 
          SMS_REC_STATUS(sms_data));
        tl_store_pid (&SMS_SIM_PROP(sms_data), record-1,
          SMS_PID(sms_data));


        SMS_EM_STORE_MO_SHORT_MESSAGE_2;

        SMS_EM_CHANGE_SHORT_MESSAGE_2;

      }
      else
      {
        tl_store_status (&SMS_SIM_PROP(sms_data), record-1, SIM_SMS_FREE);
      }
      tl_mnsms_store_cnf (MEM_SM, record, update_cnf->cause);
      SMS_READ_MODE(sms_data) = READ_NORMAL;
      SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      break;

    case MMI_DELETE:
      if (record EQ SMS_SEL_REC(sms_data))
      {
        if (update_cnf->cause EQ SIM_NO_ERROR)
        {
          tl_store_status (&SMS_SIM_PROP(sms_data), record-1, SIM_SMS_FREE);

          TRACE_EVENT_P1 ("mem cap avail %d", sms_data->mem_cap_avail);

          if (!sms_data->mem_cap_avail)
          {
             GET_MO_INSTANCE(sms_data);
            /*
             * TL  state transition TL_ESTABLISH
             * EST state transition EST_SMMA
             */
             SET_STATE (STATE_EST, EST_SMMA);
             SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
            /*
             * 1st shot
             */
             SMS_INST.retrans  = FALSE;
            /*
             * establish connection
             */
             tl_establish_connection(FALSE);
            /*
             * wait for processing of RP-SMMA
             */
             return;
          }
          if(SMS_REC_STATUS(sms_data) NEQ CMGD_DEL_INDEX)
          {            
            /* Status value is given. Find the next record satisfying this status value */
            next_rec_num = tl_search_record_for_delete (&SMS_SIM_PROP(sms_data), 
                                                        record, 
                                                        SMS_REC_STATUS(sms_data));
                                            
            
            TRACE_EVENT_P1 ("tl_sim_update_rec_cnf_mmi :: next_rec_num=%d", next_rec_num);
          }          
          tl_mnsms_delete_cnf (MEM_SM, record, next_rec_num, SIM_NO_ERROR);
        }
        else
          tl_mnsms_delete_cnf (MEM_SM, record, 0, update_cnf->cause);
      }
      else
        tl_mnsms_delete_cnf (MEM_SM, record, 0, SMS_CAUSE_INV_INDEX);

      SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      break;

    case MMI_REPLACE:
      if (record EQ SMS_SEL_REC(sms_data))
      {
        if (update_cnf->cause EQ SIM_NO_ERROR)
        {
#ifdef REL99
          if(SMS_INST.failed_msg_retx EQ FALSE)
          {
#endif
            tl_mnsms_submit_cnf (MEM_SM, record,
                                     SMS_TP_REF_RET(sms_data),
                                     SMS_NO_ERROR, SMS_SDU(sms_data));
#ifdef REL99
          }
          else
          {
            tl_mnsms_retrans_cnf (MEM_SM, record,
                                     SMS_TP_REF_RET(sms_data),
                                     SMS_NO_ERROR, SMS_SDU(sms_data));
          } 
#endif

          /*
           * update status byte
           */
          tl_store_status (&SMS_SIM_PROP(sms_data), record-1, 
            SMS_RECORD_STO_SENT);
          tl_store_pid (&SMS_SIM_PROP(sms_data), record-1,
            SMS_PID_DEFAULT);
        }
        else
        {
#ifdef REL99
          if(SMS_INST.failed_msg_retx EQ FALSE)
          {
#endif
            tl_mnsms_submit_cnf (MEM_SM, SMS_RECORD_NOT_EXIST,
                               SMS_TP_REF_RET(sms_data),
                               update_cnf->cause, SMS_SDU(sms_data));
#ifdef REL99
          }
          else
          {
            tl_mnsms_retrans_cnf (MEM_SM, SMS_RECORD_NOT_EXIST,
                               SMS_TP_REF_RET(sms_data),
                               update_cnf->cause, SMS_SDU(sms_data));
          }    
#endif
          tl_store_status (&SMS_SIM_PROP(sms_data), record-1,
            SMS_RECORD_FREE);    /* reset reservation */
        }
      }
      else
        tl_mnsms_submit_cnf (MEM_SM, record,
                             SMS_TP_REF_RET(sms_data),
                             SMS_CAUSE_INV_INDEX, SMS_SDU(sms_data));

      if (SMS_SDU(sms_data) NEQ NULL)
      {
        MFREE (SMS_SDU(sms_data));
        SMS_SDU(sms_data) = NULL;
      }
      SMS_SEL_REC(sms_data) = SMS_RECORD_NOT_EXIST;
     /*
      * concat control
      */
      if (sms_data->concat_cntrl.concatenation EQ TRUE)
      {
        /*
         * start timer TLCT for next submit req supervision
         */
         sms_timer_start(TLCT);
        /*
         * TL State Transition TL_SEND_CONTD
         */
         SMS_INST_SET_STATE (STATE_TL, TL_SEND_CONTD);
        /*
         * remember to release connection
         */
         sms_data->concat_cntrl.release_pending = TRUE;
      }
      else
      {
        /*
         * TL State Transition TL_IDLE
         */
         SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      }
      break;

    default:
      assert (GET_STATE (STATE_MMI) EQ MMI_IDLE); /* illegal state */
      break;
  }
  SET_STATE (STATE_MMI, MMI_IDLE);
#ifdef SIM_TOOLKIT
  if (sms_data->file_update_ind NEQ NULL)
  {
    T_SIM_FILE_UPDATE_IND *file_update_ind = sms_data->file_update_ind;
    sms_data->file_update_ind = NULL;
    tl_sim_file_update_ind (file_update_ind);
  }
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sim_update_rec_cnf_net  |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function TL_SIM_UPDATE_REC_CNF_NET.

*/

LOCAL void tl_sim_update_rec_cnf_net (T_SIM_UPDATE_RECORD_CNF *update_cnf)
{
  GET_INSTANCE_DATA;
  UBYTE record;
  UBYTE data[SIZE_EF_SMS];

  TRACE_FUNCTION ("tl_sim_update_rec_cnf_net()");

  record   = sms_data->sms_sim_access_info[update_cnf->req_id].rec_num;
  sms_data->sms_sim_access_info[update_cnf->req_id].entry_used = FALSE;


  CCD_START;

  switch (GET_STATE (STATE_NET))
  {
    case NET_WRITE:
      if (update_cnf->cause EQ SIM_NO_ERROR)
      {
        /*
         * update status byte
         */
        tl_store_status (&SMS_SIM_PROP(sms_data), record-1,
          SMS_RECORD_REC_UNREAD);
        tl_store_pid (&SMS_SIM_PROP(sms_data), record-1, SMS_PID(sms_data));
        /*
         * send indication to MMI
         */
        tl_message_ind_from_net (MEM_SM, record,
                                 SMS_SIM_PROP(sms_data).max_record,
                                 SMS_RP_RCVD(sms_data));

        rl_report_req_ack (NULL);
        SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      }
      else switch (sms_data->use_mem_b)   // check alternative B
      {
      case DISPLAY:
        /*
         * Only display of the message
         */
        tl_message_ind_from_net (NOT_PRESENT_8BIT, 0, 0,
                                 SMS_RP_RCVD(sms_data));
        {
          if (sms_data->mhc NEQ SMS_MHC_PH2PLUS)
          {
            rl_report_req_ack (NULL);
            SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
          }
        }
        break;

      case MEM_ME:
        /*
         * memory type is mobile memory
         */
        record = tl_get_free_space (MEM_ME);
        if (record)
        {
          /*
           * store in mobile memory
           */
/* Implements Measure#32: Row 104 */
          if (pcm_WriteRecord ((UBYTE *)ef_sms_id, record,
                               SIZE_EF_SMS, data) EQ PCM_OK)
          {
            /*
             * update status byte
             */
            tl_store_status (&SMS_ME_PROP(sms_data), record-1, SMS_RECORD_REC_UNREAD);
            tl_store_pid (&SMS_ME_PROP(sms_data), record-1, SMS_PID(sms_data));
            /*
             * send indication to MMI
             */
            tl_message_ind_from_net (MEM_ME, record,
                                     SMS_ME_PROP(sms_data).max_record,
                                     SMS_RP_RCVD(sms_data));
            /*
             * acknowledge to the infrastructure
             */
            rl_report_req_ack (NULL);
            SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
            break;
          }
        }
        /*FALLTHROUGH*/ /*lint -fallthrough*/
      default:
        if (tl_get_free_space (MEM_ME) OR tl_get_free_space (MEM_SM))
        {
         /*
          * error to the infrastructure
          */
          rl_report_req_error (SMS_RP_CS_PROTOCOL_ERROR, NULL);
         /*
          * RL_RELEASE_REQ ==>
          */
          rl_release_req(SMS_INST.ti);
	  
        }
        else
        {
          if (update_cnf->cause NEQ SIM_CAUSE_DRV_TEMPFAIL)
          {
            /* change mem_cap_avail flag on SIM and return error */
            tl_sms_memo_exceeded (FALSE);
          }
          else
          {
           /*
            * RP_ERROR =>
            */
            rl_report_req_error (
                                 SMS_RP_CS_PROTOCOL_ERROR, NULL);
           /*
            * RL_RELEASE_REQ ==>
            */
            rl_release_req(SMS_INST.ti);
          }
        }
        tl_store_status (&SMS_SIM_PROP(sms_data), record-1, SIM_SMS_FREE);
        SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
        break;

      case IGNORE:
        rl_report_req_ack (NULL);
        SMS_INST_SET_STATE (STATE_TL, TL_IDLE);

        MFREE (SMS_RP_RCVD(sms_data));
        SMS_RP_RCVD(sms_data) = NULL;
        break;
      }
      sms_data->use_mem_b = NOTHING;

      MFREE (SMS_RP_RCVD(sms_data));
      SMS_RP_RCVD(sms_data) = NULL;
      break;

    case NET_23430_WRITE:
      if (update_cnf->cause EQ SIM_NO_ERROR)
      {
        CCD_END;

        tl_sim_update_req (record,
                           SMS_RECORD_FREE, NULL);

        tl_set_access_fifo (ACCESS_BY_NET);
        SET_STATE (STATE_NET, NET_23430_DELETE);
        return;
      }
      tl_store_status (&SMS_SIM_PROP(sms_data), record-1, SIM_SMS_FREE);

      tl_sms_memo_exceeded (FALSE);
      SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      break;

    case NET_23430_DELETE:
      tl_store_status (&SMS_SIM_PROP(sms_data), record-1, SIM_SMS_FREE);
      rl_report_req_ack (NULL);
      SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      break;

    default:
      assert (GET_STATE (STATE_NET) EQ NET_IDLE); /* illegal state */
//      SET_STATE (STATE_NET, NET_IDLE);
      break;
  }
  CCD_END;

  SET_STATE (STATE_NET, NET_IDLE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_sim_update_cnf          |
+--------------------------------------------------------------------+

  PURPOSE : After updating of EF(SMSS) the Entity state is checked
            and a MNSMS_REPORT_IND is sent to indicate the Ready
            State
*/

GLOBAL void tl_sim_update_cnf (T_SIM_UPDATE_CNF *update_cnf)
{
  GET_INSTANCE_DATA;
  UBYTE       next_rec_num = 0;

  TRACE_FUNCTION ("tl_sim_update_cnf()");
  sms_data->sms_sim_access_info[update_cnf->req_id].entry_used = FALSE;

  switch (tl_read_access_fifo ())
  {
    case ACCESS_BY_MMI:
      if (sms_data->sms_sim_access_info[update_cnf->req_id].datafield EQ SIM_SMSS AND
          SMS_ENT_STATE(sms_data) EQ SMS_STATE_INITIALISING)
      {
        tl_mnsms_report_ind (SMS_ENT_STATE(sms_data) = SMS_STATE_READY);
        SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
        PFREE (update_cnf);
        return;
      }
      if (GET_STATE (STATE_MMI) EQ MMI_DELETE)
      {
        GET_MO_INSTANCE(sms_data);
        /*
         * confirm deletion to user
         */
        if(SMS_REC_STATUS(sms_data) NEQ CMGD_DEL_INDEX)
        {
          /* Status value is given. Find the next record satisfying this status value */
          next_rec_num = tl_search_record_for_delete (((SMS_SEL_MEM(sms_data) EQ MEM_ME)? 
                                                   &SMS_ME_PROP(sms_data):&SMS_SIM_PROP(sms_data)),
                                                   SMS_SEL_REC(sms_data), 
                                                   SMS_REC_STATUS(sms_data));
        }
        tl_mnsms_delete_cnf (SMS_SEL_MEM(sms_data), SMS_SEL_REC(sms_data), next_rec_num, SIM_NO_ERROR);
        SET_STATE (STATE_MMI, MMI_IDLE);
        SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      }
      else if (GET_STATE (STATE_MMI) EQ MMI_RESUME)
      {
        /*
         * confirm resumption to user
         */
        if (sms_data->pr_cntrl.save_cause EQ SMS_NO_ERROR)
        {
          if (update_cnf->cause == SIM_NO_ERROR)
          {
            tl_mnsms_resume_cnf(SMS_NO_ERROR);
          }
          else
          {
            tl_mnsms_resume_cnf(update_cnf->cause);
          }
        }
        else
        {
          tl_mnsms_resume_cnf(sms_data->pr_cntrl.save_cause);
        }
        SET_STATE (STATE_MMI, MMI_IDLE);
        SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      }
      break;
      
    case ACCESS_BY_NET:
      TRACE_EVENT ("SIM_UPDATE_CNF ignored");
      break;
       
    default:      
      TRACE_ERROR ("Unexpected Error Value");
      break; 
  }   
#ifdef SIM_TOOLKIT
  if (sms_data->file_update_ind NEQ NULL)
  {
     T_SIM_FILE_UPDATE_IND *file_update_ind = sms_data->file_update_ind;
     sms_data->file_update_ind = NULL;
     tl_sim_file_update_ind (file_update_ind);
  }
#endif
  PFREE (update_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_sim_update_record_cnf   |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_UPDATE_RECORD_CNF.

*/

GLOBAL void tl_sim_update_record_cnf (T_SIM_UPDATE_RECORD_CNF *update_cnf)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_sim_update_record_cnf()");

  switch (tl_read_access_fifo ())
  {
    case ACCESS_BY_NET:
      GET_MT_INSTANCE(sms_data);    /* MT */
      tl_sim_update_rec_cnf_net (update_cnf);
      break;

    case ACCESS_BY_MMI:
      GET_MO_INSTANCE(sms_data);    /* MO */
      tl_sim_update_rec_cnf_mmi (update_cnf);
      break;

    default:
      assert (0);    /* impossible case */
      break;
  }
  PFREE (update_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_sim_read_record_cnf     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_READ_RECORD_CNF.

*/

GLOBAL void tl_sim_read_record_cnf (T_SIM_READ_RECORD_CNF *read_cnf)
{
  T_SIM_PDU *sim_pdu;
  GET_INSTANCE_DATA;
  UBYTE   record;
  TRACE_FUNCTION ("tl_sim_read_record_cnf()");

  record   = sms_data->sms_sim_access_info[read_cnf->req_id].rec_num;
  sms_data->sms_sim_access_info[read_cnf->req_id].entry_used = FALSE;

  if (sms_data) /*lint !e774 always True*/
  {
    switch (tl_read_access_fifo ())
    {
    case ACCESS_BY_MMI:
      GET_MO_INSTANCE(sms_data);    /* MO */
      switch (GET_STATE (STATE_MMI))
      {
      case MMI_READ:
        /*
         * read attempt from MMI
         */
        if (read_cnf->cause EQ SIM_NO_ERROR AND
            record EQ SMS_SEL_REC(sms_data))
        {
          tl_store_status (&SMS_SIM_PROP(sms_data), record-1,
            read_cnf->linear_data[0]);
          if (SMS_INST_GET_STATE (STATE_TL) EQ TL_SEND)
          {
            MCAST (cp_data, U_CP_DATA);

            MALLOC (sim_pdu, sizeof(T_SIM_PDU));
            CCD_START;
            if (tl_modify_submit (sim_pdu, SMS_MODIFY(sms_data),
                                  SMS_SDU(sms_data),
                                  read_cnf->linear_data) AND
                tl_prepare_submit (sim_pdu, cp_data))
            {
             /*
              * RL_DATA_REQ
              */
              if (!rl_data_req (SMS_INST.tp_mr, cp_data))
              {
                SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
               /*
                * release connection
                */
                rl_release_req(SMS_INST.ti);
               /*
                * send not successful but we confirm with no error ???!!
                */
                tl_mnsms_submit_cnf (MEM_SM, record,
                                     NOT_PRESENT_8BIT, SMS_NO_ERROR, NULL);
              }
            }
            else
            {
              SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
              tl_mnsms_submit_cnf (MEM_SM, record,
                                   NOT_PRESENT_8BIT, SMS_CAUSE_PARAM_WRONG, NULL);
            }
            CCD_END;
            MFREE (sim_pdu);
            MFREE (SMS_SDU(sms_data));
            SMS_SDU(sms_data) = NULL;

            SET_STATE (STATE_MMI, MMI_IDLE);
            break;
          }
          if ((read_cnf->linear_data[0] & 1) EQ SMS_RECORD_FREE)
          {
            tl_mnsms_read_cnf (MEM_SM, 0,
                               NULL, SMS_CAUSE_INV_INDEX);
          }
          else if (((read_cnf->linear_data[0] & 7) EQ SMS_RECORD_REC_UNREAD)
                    AND SMS_READ_MODE(sms_data) NEQ READ_PREVIEW)
          {
            tl_set_access_fifo (ACCESS_BY_MMI);
            SET_STATE (STATE_MMI, MMI_READ_STATE_UPDATE);
            SMS_INST_SET_STATE (STATE_TL, TL_OTHER);
            tl_sim_update_req (record, SMS_RECORD_REC_READ,
                               &read_cnf->linear_data[1]);

            if (SMS_READ_MODE(sms_data) NEQ READ_STATUS_CHANGE)
            {
//              read_cnf->linear_data[0] = SMS_RECORD_REC_READ;
              /*
               * keep read data until reception of UPDATE CONFIRM
               */
              if (SMS_SIM_READ(sms_data) NEQ 0)
                PFREE(SMS_SIM_READ(sms_data));
              SMS_SIM_READ(sms_data) = read_cnf;
              return;   /* read_cnf not to be deallocated */
            }
            PFREE (read_cnf);
            return;     /* keep states */
          }
          else
          {

            SMS_EM_READ_SHORT_MESSAGE_2;

            tl_mnsms_read_cnf (MEM_SM, record,
                               ((SMS_READ_MODE(sms_data) EQ READ_STATUS_CHANGE)?
                                NULL: read_cnf->linear_data),
                               SIM_NO_ERROR);
          }
        }
        else
        {
          if (SMS_INST_GET_STATE (STATE_TL) EQ TL_SEND)
          {
            if (read_cnf->cause EQ SIM_NO_ERROR)
              tl_mnsms_submit_cnf (MEM_SM, record,
                                   NOT_PRESENT_8BIT, SMS_CAUSE_INV_INDEX, NULL);
            else
              tl_mnsms_submit_cnf (MEM_SM, record,
                                   NOT_PRESENT_8BIT, read_cnf->cause, NULL);
          }
          else
            tl_mnsms_read_cnf (MEM_SM, 0,
                               NULL, SMS_CAUSE_INV_INDEX);
        }
        SET_STATE (STATE_MMI, MMI_IDLE);
        SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
        break;

      case MMI_FIND_FIRST:
#ifdef SIM_TOOLKIT
        if (sms_data->file_update_ind NEQ NULL)
        {
          T_SIM_FILE_UPDATE_IND *file_update_ind = sms_data->file_update_ind;
          /*
           * Take a backup of the current status of the MMI_STATE
           */
          sms_data->stored_mmi_state = GET_STATE(STATE_MMI);
          SET_STATE (STATE_MMI, MMI_IDLE);
          sms_data->file_update_ind = NULL;
          tl_sim_file_update_ind (file_update_ind);

          /* Implements Measure# 7 */
          if( tl_sim_rd_record_update_state(read_cnf) )
            return;
        }
#endif
        /*
         * first record is scanned
         */
        if (read_cnf->cause EQ SIM_NO_ERROR)
        {
          /* Store max. supported records of the SIM */
          SMS_SIM_PROP(sms_data).max_record = read_cnf->max_record;

          if (SMS_SIM_PROP(sms_data).max_record > MAX_RECORD_SIM)
          {
            /* The number of supported records is limited by SMS */
            SMS_SIM_PROP(sms_data).max_record = MAX_RECORD_SIM;
          }
          else if (SMS_SIM_PROP(sms_data).max_record EQ 0)
          {
            /* We have at least one record as we successfully read it
             * regardless of the returned value from the SIM */
            SMS_SIM_PROP(sms_data).max_record = 1;
          }
          /*
           * continue with MMI_FIND_NEXT
           */
        }
        else
        {
          /*
           * No EF_SMS field on the SIM.
           */          
          SMS_SIM_PROP(sms_data).max_record = 0;
          SET_STATE (STATE_MMI, MMI_IDLE);
          /* 
           * init done
           */
          tl_init_complete();
          break;
        }
      /*FALLTHROUGH*/ /*lint -fallthrough*/
      case MMI_FIND_NEXT:

#ifdef SIM_TOOLKIT
        /*
         * Take a backup of the current status of the MMI_STATE
         */
        sms_data->stored_mmi_state = GET_STATE(STATE_MMI);
#endif /* SIM_TOOLKIT */

        SET_STATE (STATE_MMI, MMI_IDLE); 

#ifdef SIM_TOOLKIT

        if (sms_data->file_update_ind NEQ NULL)
        {
          T_SIM_FILE_UPDATE_IND *file_update_ind = sms_data->file_update_ind;
          sms_data->file_update_ind = NULL;
          tl_sim_file_update_ind (file_update_ind);
          
         /* Implements Measure# 7 */ 
         if( tl_sim_rd_record_update_state(read_cnf) )
            return;
        }
#endif
        /*
         * next record is scanned
         */
        if (read_cnf->cause EQ SIM_NO_ERROR)
        {
          if (tl_find_status_pid (MEM_SM,
                                  record,
                                  read_cnf->linear_data,
                                  &SMS_SIM_PROP(sms_data)))
            tl_message_ind_from_sim (MEM_SM, record,
                                     read_cnf->max_record,
                                     read_cnf->linear_data);
          if (record < SMS_SIM_PROP(sms_data).max_record)
            tl_find_next (MEM_SM, (UBYTE)(record + 1));
          else
          {
#ifdef SIM_TOOLKIT
            if (sms_data->sim_mem_update)
            {
              PALLOC (file_update_res, SIM_FILE_UPDATE_RES);
              file_update_res->source = SRC_SMS;
              file_update_res->fu_rsc = SIM_FU_SUCCESS;
              PSENDX (SIM, file_update_res);
              sms_data->sim_mem_update = FALSE;
            }
#endif
            /*
             * check for existing but empty memory
             */
            if (SMS_SIM_PROP(sms_data).max_record > 0
                 AND !SMS_SIM_PROP(sms_data).any_valid)
              tl_message_ind_from_sim (MEM_SM, 0,
                                       SMS_SIM_PROP(sms_data).max_record,
                                       NULL);

           /* 
            * init done
            */
            tl_init_complete();
          }
        }
        else
        {
          /*
           * declare only successful read
           * entries as available
           */
          SMS_SIM_PROP(sms_data).max_record = record - 1;
          // must be doubled
          /* 
           * init done
           */
          tl_init_complete();
        }
        break;

      default:
        assert (GET_STATE (STATE_MMI) EQ MMI_IDLE); /* illegal state */
        break;
      }
#ifdef SIM_TOOLKIT
      if (sms_data->file_update_ind NEQ NULL)
      {
        T_SIM_FILE_UPDATE_IND *file_update_ind = sms_data->file_update_ind;
        sms_data->file_update_ind = NULL;
        tl_sim_file_update_ind (file_update_ind);
      }
#endif
      break;

    case ACCESS_BY_NET:
      GET_MT_INSTANCE(sms_data);    /* MT */
      switch (GET_STATE (STATE_NET))
      {
      case NET_READ:
        if (tl_check_replace_entry (&SMS_RP_RCVD(sms_data)->rp_user_data.tpdu.b_tpdu[1],
                                    &read_cnf->linear_data[1]))
        {
          /*
           *record shall be replaced
           */
          tl_sim_conv_update_req (record, SMS_RP_RCVD(sms_data));

          SMS_EM_REPLACE_SMS_IN_SIM;

          tl_set_access_fifo (ACCESS_BY_NET);
          SET_STATE (STATE_NET, NET_WRITE);
        }
        else
        {
          int i;

          i = tl_check_mt_pid (&SMS_SIM_PROP(sms_data),
                               record + 1,
                               SMS_PID(sms_data));
          if (i > 0)
          {
            tl_set_access_fifo (ACCESS_BY_NET);
            tl_sim_read_record_req((UBYTE)i);
          }
          else
          {
            MCAST (sms_deliver, TP_DELIVER);

            SET_STATE (STATE_NET, NET_IDLE);

            CCD_START;
            ccd_decodeMsg (CCDENT_SMS,
                           DOWNLINK,
                           (T_MSGBUF *)&SMS_RP_RCVD(sms_data)->rp_user_data.tpdu,
                           (UBYTE *)sms_deliver,
                           TP_DELIVER);
            tl_handle_message (sms_deliver);
            CCD_END;
          }
        }
        break;

      default:
        assert (GET_STATE (STATE_NET) EQ NET_IDLE); /* illegal state */
        break;
      }
      break;

    default:
      assert (0);    /* impossible case */
      break;
    }
  }
  PFREE (read_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_sim_remove_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_REMOVE_IND.

*/

GLOBAL void tl_sim_remove_ind (T_SIM_REMOVE_IND *remove_ind)
{
  USHORT error;
  register T_SMS_DATA *sms_data = GET_SMS_INSTANCE (0);

  TRACE_FUNCTION ("tl_sim_remove_ind()");
  if (sms_data)
  {
    error = (remove_ind->cause EQ SIM_NO_ERROR)?
            SMS_CAUSE_SIM_BUSY: remove_ind->cause;

    tl_sim_abort_to_mmi (error);

    sms_data->init_done = FALSE;
    SMS_SIM_PROP(sms_data).max_record = 0;
    sms_data->me_backup.max_record  = 0;
    sms_data->sim_mem_update = FALSE;
  }
  PFREE (remove_ind);
}

#if defined(SIM_TOOLKIT)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_sim_toolkit_cnf         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_TOOLKIT_CNF.

*/

GLOBAL void tl_sim_toolkit_cnf (T_SIM_TOOLKIT_CNF *toolkit_cnf)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_sim_toolkit_cnf()");

  switch (tl_read_access_fifo ())
  {
    case ACCESS_BY_NET:
      GET_MT_INSTANCE(sms_data);    /* MT */
      tl_sim_toolkit_confirm (toolkit_cnf);
      break;

    default:
      assert(0);
      break;
  }
  PFREE (toolkit_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_sim_file_update_ind     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_FILE_UPDATE_IND.

*/

GLOBAL void tl_sim_file_update_ind (T_SIM_FILE_UPDATE_IND *file_update_ind)
{
  USHORT i;
  BOOL found_sms = FALSE;   /* short messages changed by SAT */
  BOOL found_smss = FALSE;  /* short message status changed by SAT */
  register T_SMS_DATA *sms_data = GET_SMS_INSTANCE (0);

  TRACE_FUNCTION ("tl_sim_file_update_ind()");

  if (sms_data)
  {
    /*
     * avoid interference with active MMI procedure
     */
    switch (GET_STATE (STATE_MMI))
    {
      case MMI_IDLE:
        break;

      default:
        if (sms_data->file_update_ind NEQ NULL)
          PFREE (sms_data->file_update_ind);
        /* store primitive for delayed reaction */
        sms_data->file_update_ind = file_update_ind;
        return;
    }

    for (i = 0; i < file_update_ind->val_nr; i++)
    {
      if((file_update_ind->file_info[i].v_path_info EQ TRUE)  AND
         (file_update_ind->file_info[i].path_info.df_level1   EQ SIM_DF_TELECOM) AND
         (file_update_ind->file_info[i].path_info.v_df_level2 EQ FALSE))
      {
        if (file_update_ind->file_info[i].datafield EQ SIM_SMS)
        {
          /* short messages changed by SAT */
          found_sms = TRUE;
          sms_data->sim_mem_update = TRUE;
        }
        else if (file_update_ind->file_info[i].datafield EQ SIM_SMSS)
        {
          /* short message status changed by SAT */
          found_smss = TRUE;
        }
      }

      if (found_sms AND found_smss)
        break;
    }

    if (found_smss)
    {
      /* short message status changed by SAT */
      tl_sim_read_req (SIM_SMSS, 2);
      sms_data->sim_mem_update = found_sms NEQ FALSE;

      SET_STATE (STATE_MMI, MMI_STATE_UPDATE);
    }
    else if (found_sms)
    {
      /* short messages changed by SAT */
      sms_data->init_done = FALSE;
      SMS_SIM_PROP(sms_data).max_record = 0;
      tl_mnsms_report_ind (SMS_ENT_STATE(sms_data) = SMS_STATE_INITIALISING);
      sms_data->sim_mem_update = TRUE;
      tl_find_first (MEM_SM);
    }
    else
    {
      /* Nothing to do, indicate success to SAT */
      PALLOC (file_update_res, SIM_FILE_UPDATE_RES); /* T_SIM_FILE_UPDATE_RES */
      file_update_res->source = SRC_SMS;
      file_update_res->fu_rsc = SIM_FU_SUCCESS;
      PSENDX (SIM, file_update_res);
    }
  }
  PFREE (file_update_ind);
}

#endif  // SIM_TOOLKIT

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_mnsms_cmms_end_ind      |
+--------------------------------------------------------------------+

  PURPOSE : Function used for sending MNSMS_REPORT_IND.Primitive to ACI
            when cmms_mode is eqaul to 1.

*/

GLOBAL void tl_mnsms_cmms_end_ind(void)
{
  GET_INSTANCE_DATA;
  PALLOC (report_ind, MNSMS_REPORT_IND);

  TRACE_FUNCTION ("tl_mnsms_cmms_end_ind()");

  if(sms_data->cmms_mode EQ CMMS_MODE_ONE)
  {
    sms_data->cmms_mode = CMMS_MODE_DEF;
    report_ind->state = SMS_STATE_READY;
    report_ind->v_cmms_mode = TRUE;
    report_ind->cmms_mode = sms_data->cmms_mode;
    PSENDX (MMI, report_ind);
  }
}

#ifdef SIM_PERS_OTA
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_OTA_message_res    |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSMS_OTA_MESSAGE_RES.

*/

GLOBAL void tl_mnsms_OTA_message_res ( T_MNSMS_OTA_MESSAGE_RES *mnsms_OTA_message_res )
{
  
  T_SMS_DATA *sms_data = GET_INSTANCE(0);
  
  TRACE_FUNCTION ("tl_mnsms_OTA_message_res()");

  if (sms_data) /*lint !e774 always True*/
  { 
    GET_MT_INSTANCE(sms_data);  /* MT operation */	

    switch (SMS_INST_GET_STATE (STATE_TL))
     {
        case TL_RECEIVE:
         if (sms_timer_check(TR2M))
         {
           T_TP_DLVR_REP_ACK *rep_ack ;
           T_rp_user_data *rp_ud;
           MALLOC(rep_ack, sizeof(T_TP_DLVR_REP_ACK));
           MALLOC(rp_ud, sizeof(T_rp_user_data));
         
           rep_ack->tp_vt_mti = SMS_VT_DLVR_REP_ACK;
           rep_ack->tp_udhi = SMS_UDHI_NOT_INCLUDED;
           rep_ack->tp_mti = SMS_DELIVER_REPORT;
           rep_ack->v_tp_udh_inc = FALSE;
         
           memcpy(rep_ack->tp_ud.data, mnsms_OTA_message_res->data,mnsms_OTA_message_res->len);
         
           rep_ack->tp_ud.c_data =mnsms_OTA_message_res->len;  //No of octets
           rep_ack->tp_ud.length = mnsms_OTA_message_res->packedlen;  /*Since default alphabet is used as per 23.040 sec. 9.2.3.16
                           the length contains no of septets of data returned */
                         
           rep_ack->tp_ext = SMS_EXT_NOT_INCLUDED;
           rep_ack->v_tp_ud = TRUE;
           rep_ack->tp_udl_p = SMS_UD_INCLUDED;
           rep_ack->tp_dcs = mnsms_OTA_message_res->tp_dcs;
           rep_ack->v_tp_dcs = TRUE;
           rep_ack->tp_dcs_p = SMS_DCS_INCLUDED;
           rep_ack->tp_pid = mnsms_OTA_message_res->tp_pid;
           rep_ack->v_tp_pid = TRUE;
           rep_ack->tp_pid_p = SMS_PID_INCLUDED;
           rp_ud->tpdu.o_tpdu = 0;
           rp_ud->tpdu.l_tpdu = TPDU_BIT_LEN;
           rp_ud->v_tpdu = (ccd_codeMsg (CCDENT_SMS, UPLINK,
                                        (T_MSGBUF *)&rp_ud->tpdu,
                                        (UBYTE *)rep_ack,
                                         SMS_VT_DLVR_REP_ACK) EQ ccdOK);
           rp_ud->tp_mti = SMS_DELIVER_REPORT;
             
           MFREE (rep_ack);
           rl_report_req_ack (rp_ud);
           MFREE (rp_ud);           
           SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      }
     else
      {
           /*TR2M TIM EXPIRE -ACI NOT RESPONDED WITHIN TR2M */
     	}
     break;

    default:
        /* INVALID SMS TL STATE IN MNSMS_OTA_MESSAGE_RES */
        break;

   }
 }
  PFREE (mnsms_OTA_message_res);

}
#endif /* End OTA */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_retrans_req       |
+--------------------------------------------------------------------+

  PURPOSE : Handles the primitive MNSMS_RETRANS_REQ. This will support user 
  requested retransmission of the last failed message as per 
  23.040 R99 Section 9.2.3.6.
*/
#ifdef REL99
GLOBAL void tl_mnsms_retrans_req (T_MNSMS_RETRANS_REQ *mnsms_retrans_req)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_mnsms_retrans_req()");

  switch (SMS_INST_GET_STATE (STATE_TL))
  {
    case TL_SEND_CONTD:
     /*
      * stop timer TLCT
      */
      sms_timer_stop(TLCT);  
    /*FALLTHROUGH*/ /*lint -fallthrough*/
    case TL_IDLE:
      if (sms_timer_check(TRAM))
      {
        /*
         * MNSMS_SUBMIT_CNF
         */
         tl_mnsms_retrans_cnf (SMS_SEL_MEM(sms_data), SMS_SEL_REC(sms_data),
                             NOT_PRESENT_8BIT, SMS_CAUSE_ENTITY_BUSY, NULL);
         break;
      }

       /* Check if MO message is present(stored CPDU data available?) */
      if (SMS_DATA_REQ(sms_data) NEQ NULL)
      {    
        if(mnsms_retrans_req->auto_rep_flag EQ AUTO_REP_FLAG_ENABLED)
        {
          SMS_INST.tl_retx = 0;
        }
        else
        {
          /* Set to the max value so that retransmission will not be done */
          SMS_INST.tl_retx = TL_MAX_RETANS;
        }

        SMS_INST_SET_STATE (STATE_TL, TL_SEND); 

        /* Restore the record number and storage type */
        SMS_SEL_REC(sms_data) = SMS_INST.failed_msg_rec_num;
        SMS_SEL_MEM(sms_data) = SMS_INST.failed_msg_mem;
        SMS_INST.failed_msg_retx = TRUE;
        
        /*
         * TL  state transition TL_ESTABLISH
         * EST state transition EST_SEND
         */
         SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
         SET_STATE (STATE_EST, EST_RTX);
        /*
         * establish connection
         */
         tl_establish_connection(FALSE);       
      }
      else
      {
        tl_mnsms_retrans_cnf (SMS_SEL_MEM(sms_data),SMS_SEL_REC(sms_data), 
                              SMS_TP_REF_RET(sms_data),
                              SMS_CAUSE_FAILED_MSG_NOT_FOUND, NULL);
      }
      break;
    
    default:
       tl_mnsms_retrans_cnf (SMS_SEL_MEM(sms_data),SMS_SEL_REC(sms_data),
                              SMS_TP_REF_RET(sms_data),
                              SMS_CAUSE_ENTITY_BUSY, NULL);
       break;
  }

  PFREE(mnsms_retrans_req);

  return;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_retrans_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : Generate a response to the primitive MNSMS_RETRANS_REQ.
*/

GLOBAL void tl_mnsms_retrans_cnf ( UBYTE mem_type,
                                   UBYTE record,
                                   UBYTE tp_mr,
                                   USHORT cause,
                                   T_sms_sdu *sms_sdu )
{
  TRACE_FUNCTION ("tl_mnsms_retrans_cnf()");
  {
    PALLOC (retrans_cnf, MNSMS_RETRANS_CNF);
    retrans_cnf->mem_type = mem_type;
    retrans_cnf->rec_num = record;
    retrans_cnf->tp_mr = tp_mr;
    retrans_cnf->cause = cause;

    if (sms_sdu NEQ NULL)
    {
      memcpy (&retrans_cnf->sms_sdu, sms_sdu, sizeof (T_sms_sdu));
    }
    else
    {
      memset (&retrans_cnf->sms_sdu, 0, sizeof (T_sms_sdu)); 
    }

    PSENDX (MMI, retrans_cnf);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLP                    |
| STATE   : code                ROUTINE : tl_mnsms_send_prog_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Send the primitive MNSMS_SEND_PROG_IND.
*/
GLOBAL void tl_mnsms_send_prog_ind (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tl_mnsms_send_prog_ind ()");
  {
    PALLOC (send_prog_ind, MNSMS_SEND_PROG_IND);
    send_prog_ind->resend_count = sms_data->data[INST_MO].tl_retx;
    send_prog_ind->max_retrans = TL_MAX_RETANS;  
    PSENDX (MMI, send_prog_ind);
  }
}
#endif /* REL99 */

#endif
