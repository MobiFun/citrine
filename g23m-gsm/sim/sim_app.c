/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  SIM_APP
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
|  Purpose :  This modul defines the SIM Application.
+----------------------------------------------------------------------------- 
*/ 

#ifndef SIM_APP_C
#define SIM_APP_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SIM

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "cnf_sim.h"
#include "mon_sim.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "sim.h"
#include "sim_em.h"

#ifdef TI_PS_UICC_CHIPSET_15
#include "8010_136_SIMDRV_SAP_inline.h"
#endif

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/
GLOBAL T_FIELD_STATUS field_status;

/*==== FUNCTIONS ==================================================*/


LOCAL UBYTE app_read_sim_service_table (T_SIM_MMI_INSERT_IND * sim_mmi_insert_ind);
LOCAL USHORT app_get_ef_size(USHORT service, USHORT ef_name, UBYTE* res);
#ifdef TI_PS_UICC_CHIPSET_15
GLOBAL void app_sim_insert( T_SIMDRV_atr_string_info     *atr_string_info,
                            U8       config_requested,
                            T_SIMDRV_config_characteristics     *config_characteristics);

#ifdef _SIMULATION_
GLOBAL UBYTE sim_command_type;
#endif

#endif /* TI_PS_UICC_CHIPSET_15 */

#ifndef TI_PS_UICC_CHIPSET_15
#define SIMDRV_MAX_RESULT 0x100
#endif


/* Implements Measure# 2 to 8 */
/* Table */
#ifdef TI_PS_HCOMM_CHANGE
T_HANDLE const hComm_mux[] = {_hCommMMI, _hCommMM, _hCommSMS}; 
#else
T_HANDLE* const hComm_mux[] = {&sim_hCommMMI, &sim_hCommMM, &sim_hCommSMS}; 
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS              MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_get_ef_size            |
+--------------------------------------------------------------------+

  PURPOSE : Return size of the Elementory file. If Elementory file is 
            not present return 0.

*/

LOCAL USHORT app_get_ef_size(USHORT service, USHORT ef_name, UBYTE *res)
{

  if (SIM_IS_FLAG_SET (service))
  {      
    T_FIELD_STATUS field_status;
    
    if (FKT_Select (ef_name, FALSE, NULL, res, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)
    {
      memcpy(&field_status, res, SIM_MIN_EF_ST_LEN);
      return((((USHORT)field_status.field_size[0]) << 8)
                   | (USHORT)field_status.field_size[1]);
    }
  }

  return 0;

}

/* Implements Measure# 18 */
/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)   MODULE  : SIM_APP                          |
| STATE   : code            ROUTINE : app_sim_read_n_update_req        |
+----------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_READ_REQ and SIM_UPDATE_REQ

*/
LOCAL void app_sim_read_n_update_req(USHORT *cause, USHORT *datafield, UBYTE rd_upd_access, U8 *v_path_info, T_path_info *path_info)
{
  UBYTE response[SIMDRV_MAX_RESULT];  
  
    
  TRACE_FUNCTION ("app_sim_read_n_update_req()");

  
  /* if SIM is inserted, try to select the SIM card.*/
  *cause = FKT_Select (*datafield, *v_path_info, path_info, response, SIM_MIN_EF_ST_LEN);

    if (*cause EQ SIM_NO_ERROR)
    {
      memcpy(&field_status, response, SIM_MIN_EF_ST_LEN);  
    
      sim_data.act_length = (USHORT)field_status.field_size[0] * 256 +
                              field_status.field_size[1];
  }

  if ((*cause EQ SIM_NO_ERROR) OR (*cause EQ SIM_NO_ERR_FILE_ALREADY_SELECTED))
  {
    /* field_status is global and has been updated either 
     * in "if" above or during previous operation on same file 
     */
      if (!(sim_data.act_access = app_check_access_conditions (rd_upd_access,
                                    &field_status)))
      {
      /* access not allowed */
         *cause = SIM_CAUSE_ACCESS_PROHIBIT;
      }
    else
    {
      *cause = SIM_NO_ERROR; 
    }
  }

}


/* Implements Measure# 14 */
/*
+-------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)   MODULE  : SIM_APP                                   |
| STATE   : code            ROUTINE : app_sim_activate_req_fdn_enable           |
+-------------------------------------------------------------------------------+

  PURPOSE : Process the SIM activate req for FDN ENABLE and FDN DISABLE cases

*/

LOCAL void app_sim_activate_req_fdn_enable(BOOL sim_fdn_en) 
{
  ULONG   support_flag, service_flag;
  USHORT  error, (*FKT_FuncPtr)(void);
  
  TRACE_FUNCTION ("app_sim_activate_req_fdn_enable()");
  
  if(sim_fdn_en EQ TRUE) 
  {
    support_flag = ADN_SUPPORT_BY_SIM;
    service_flag = SERVICE_3_SUPPORT;
    FKT_FuncPtr = FKT_Invalidate ;
  }
  else
  {
    support_flag = FDN_SUPPORT_BY_SIM;
    service_flag = SERVICE_2_SUPPORT;
    FKT_FuncPtr = FKT_Rehabilitate ;
  }
  
  if (SIM_IS_FLAG_SET (support_flag))
  {
    if (SIM_IS_FLAG_SET (service_flag))
    {
      sim_data.last_requested_pin_no = LRP_PIN_2;
      error = FKT_Select (SIM_ADN, FALSE, NULL, NULL, 0);
      if (error EQ SIM_NO_ERROR OR error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)
      { 
        error = FKT_FuncPtr();
        if (error EQ SIM_NO_ERROR)
        {
          if(sim_fdn_en EQ FALSE)  
          {
            if (!app_check_imsi_loci_validation())
            {
              if (!app_rehabilitate_imsi_loci())
              {
                app_sim_card_error (SIM_CAUSE_EF_INVALID);
                return ; /* return is replaced for break */
              }
            }
            SIM_SET_FLAG (ADN_SUPPORT_BY_SIM);
          }
          else
            SIM_SET_FLAG (FDN_SUPPORT_BY_SIM);
          SIM_CLEAR_FLAG (support_flag);   
          app_sim_card_error (SIM_NO_ERROR);
        }
        else
          app_sim_card_error (error);
      }
    else
      app_sim_card_error (error);
  
      if(sim_fdn_en EQ TRUE) 
      {
        SIM_EM_FDN_ENABLE;
      }
      else
      {
        SIM_EM_ADN_ENABLE;
      }
  
    }
    else
      app_sim_card_error (SIM_CAUSE_ACCESS_PROHIBIT);
  }
  else
    app_sim_card_error (SIM_NO_ERROR);
}
 



/*
 * -------------------------------------------------------------------
 * PRIMITIVE Processing functions
 * -------------------------------------------------------------------
 */
/*lint -e720 (boolean test of assignment) */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_init_sim_data          |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the SIM data for the module application.

*/

GLOBAL void app_init_sim_data (void)
{
#ifdef SIM_TOOLKIT
  int i;
#endif

  TRACE_FUNCTION ("app_init_sim_data()");
  /*
   * initialize all internal flags
   */
  sim_data.flags = 0;

  sim_data.last_requested_pin_no  = LRP_NONE;
  /* 
  sim_data.pin_no_puct            = NOT_PRESENT_8BIT;
  sim_data.pin_no_acm             = NOT_PRESENT_8BIT;
  sim_data.pin_no_acmmax          = NOT_PRESENT_8BIT;
  */

  sim_data.act_directory          = NOT_PRESENT_16BIT;
  sim_data.act_field              = NOT_PRESENT_16BIT;
  sim_data.remove_error           = SIM_CAUSE_CARD_REMOVED;

#ifdef SIM_TOOLKIT
  sim_data.sync_awaited           = 0;

  /* startup with idle polling, after 11.11[11.2.8] */
  sim_data.idle_polling           = TRUE;

  for (i = 0; i < MAX_SAT_TIMER; i++)
  {
    sim_data.timer[i].active = FALSE;
  }
#endif

#ifndef TI_PS_UICC_CHIPSET_15
  SIM_Init (app_sim_insert, app_sim_remove);
#else
  simdrv_register(app_sim_insert, app_sim_remove);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_read_req           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_READ_REQ.

*/

GLOBAL void app_sim_read_req (T_SIM_READ_REQ * sim_read_req)
{
  USHORT         source;
  
  PALLOC (sim_read_cnf, SIM_READ_CNF);

  TRACE_FUNCTION ("app_sim_read_req()");

  // initialize answer
  source                = sim_read_req->source;
  sim_read_cnf->req_id  = sim_read_req->req_id;

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    /*
     * If unknown length request, a Select must be
     * carried out in any case to determine real length
     */
    if (sim_read_req->length EQ NOT_PRESENT_8BIT)
    {
      sim_data.act_field = NOT_PRESENT_16BIT;
    }
  
    /* Implements Measure# 18 */
    app_sim_read_n_update_req(&sim_read_cnf->cause, &sim_read_req->datafield, ACCESS_READ,&sim_read_req->v_path_info ,&sim_read_req->path_info);

   
    if (sim_read_cnf->cause EQ SIM_NO_ERROR)
    {
      if (sim_read_req->length EQ NOT_PRESENT_8BIT)
      {
        // unknown length, must be calculated by the entity
        if (sim_read_req->offset < sim_data.act_length)
        {
          sim_read_cnf->length = (UBYTE)(sim_data.act_length - sim_read_req->offset);
        }
        else
        {
          // incorrect offset
          sim_read_cnf->cause = SIM_CAUSE_PARAM_WRONG;
        }
      }
      else
      {
        sim_read_cnf->length = sim_read_req->length;
        // check given length
        if (((USHORT)sim_read_req->offset + sim_read_req->length) <= sim_data.act_length)
        {
          sim_read_cnf->length = sim_read_req->length;
        }
        else
        {
          // incorrect offset and/or length
          sim_read_cnf->cause = SIM_CAUSE_PARAM_WRONG;
        }
      }
    }
  }
  else
  {
    // sim is not inserted
    sim_read_cnf->cause = SIM_CAUSE_CARD_REMOVED;
  }

  if (sim_read_cnf->cause EQ SIM_NO_ERROR)
  {
    // cut length if needed
    if ((sim_read_req->max_length > 0) AND
        (sim_read_cnf->length > sim_read_req->max_length))
      sim_read_cnf->length = sim_read_req->max_length;
    // length is available then read and start status timer again
    sim_read_cnf->cause = FKT_ReadBinary (sim_read_cnf->trans_data,
                                          sim_read_req->offset,
                                          (USHORT)sim_read_cnf->length);
    app_start_status_timer (FALSE);
  }
  else
  {
    sim_read_cnf->length = 0;
    memset (sim_read_cnf->trans_data, 0, sizeof (sim_read_cnf->trans_data));
  }

  SIM_EM_READ_BINARY_FILE;

  // free incoming primitive
  PFREE (sim_read_req);

  /* Implements Measure# 2 */
#ifdef TI_PS_HCOMM_CHANGE
  vsi_c_psend (hComm_mux[source], (T_VOID_STRUCT*) sim_read_cnf FILE_LINE_MACRO);
#else
  vsi_c_psend (*hComm_mux[source], (T_VOID_STRUCT*) sim_read_cnf FILE_LINE_MACRO);
#endif

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_read_record_req    |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_READ_RECORD_REQ.

*/

GLOBAL void app_sim_read_record_req (T_SIM_READ_RECORD_REQ * sim_read_record_req)
{
  USHORT         source;
  UBYTE response[SIMDRV_MAX_RESULT];
  
  PALLOC (sim_read_record_cnf, SIM_READ_RECORD_CNF);

  TRACE_FUNCTION ("app_sim_read_record_req()");

  source = sim_read_record_req->source;
  sim_read_record_cnf->req_id     = sim_read_record_req->req_id;

  sim_read_record_cnf->max_record = 0;

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    sim_read_record_cnf->cause = FKT_Select (sim_read_record_req->datafield,
                                             sim_read_record_req->v_path_info,
					     &sim_read_record_req->path_info,
                                             response, SIM_MIN_EF_ST_LEN);

      if (sim_read_record_cnf->cause EQ SIM_NO_ERROR)
    {
      USHORT total_length;
        memcpy(&field_status,response,SIM_MIN_EF_ST_LEN);
        sim_data.field_type = field_status.field_type;
        sim_data.act_length = (USHORT)field_status.record_length;
        total_length = (USHORT)field_status.field_size[0] * 256 +
                         field_status.field_size[1];
        if (field_status.record_length)
        {
            sim_read_record_cnf->max_record =
                               total_length / field_status.record_length;
        }
        else
        {
            sim_read_record_cnf->max_record = 1;  /* don't divide by zero */
        }
          sim_data.max_record = sim_read_record_cnf->max_record;
    }

    if ((sim_read_record_cnf->cause EQ SIM_NO_ERROR) OR
        (sim_read_record_cnf->cause EQ SIM_NO_ERR_FILE_ALREADY_SELECTED))
    {
      /* field_status is global and has been updated either 
       * in "if" above or during previous operation on same file 
       */
        if (!(sim_data.act_access = app_check_access_conditions (ACCESS_READ,
                                      &field_status)))
        {
          sim_read_record_cnf->cause = SIM_CAUSE_ACCESS_PROHIBIT;
        }
        else if ((sim_read_record_req->record EQ 0) OR
              (sim_read_record_req->record > sim_data.max_record))
        {
          sim_read_record_cnf->cause = SIM_CAUSE_ADDR_WRONG;
        }
        else
        {
        sim_read_record_cnf->cause = SIM_NO_ERROR;
         sim_read_record_cnf->max_record = sim_data.max_record;
         sim_read_record_cnf->length = (sim_read_record_req->length > (UBYTE)sim_data.act_length)?
                                    (UBYTE)sim_data.act_length: sim_read_record_req->length;
        }
      }
    }
  else
  {
    sim_read_record_cnf->cause = SIM_CAUSE_CARD_REMOVED;
  }

  if (sim_read_record_cnf->cause EQ SIM_NO_ERROR)
  {
    sim_read_record_cnf->cause =
          FKT_ReadRecord (sim_read_record_cnf->linear_data,
                          4,
                          sim_read_record_req->record,
                          (USHORT)sim_data.act_length);
    app_start_status_timer (FALSE);
  }
  else
  {
    sim_read_record_cnf->length = 0;
    memset (sim_read_record_cnf->linear_data, 0,
	    sizeof (sim_read_record_cnf->linear_data));
  }

  SIM_EM_READ_RECORD_FILE;

  PFREE (sim_read_record_req);


  /* Implements Measure# 2 */
#ifdef TI_PS_HCOMM_CHANGE
  vsi_c_psend (hComm_mux[source], (T_VOID_STRUCT*) sim_read_record_cnf FILE_LINE_MACRO);
#else
  vsi_c_psend (*hComm_mux[source], (T_VOID_STRUCT*) sim_read_record_cnf FILE_LINE_MACRO);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_update_req         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_UPDATE_REQ.

*/
GLOBAL void app_sim_update_req (T_SIM_UPDATE_REQ * sim_update_req)
{
  USHORT          source;
  PALLOC (sim_update_cnf, SIM_UPDATE_CNF);

  TRACE_FUNCTION ("app_sim_update_req()");

  source = sim_update_req->source;
  sim_update_cnf->req_id = sim_update_req->req_id;
  sim_update_cnf->cause = SIM_NO_ERROR;

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
  
    /* Implements Measure# 18 */
    app_sim_read_n_update_req(&sim_update_cnf->cause, &sim_update_req->datafield, ACCESS_UPDATE,&sim_update_req->v_path_info ,&sim_update_req->path_info) ;
  
    if (sim_update_cnf->cause EQ SIM_NO_ERROR)
    {
      if ((sim_update_req->offset + (USHORT)sim_update_req->length)
           > sim_data.act_length)
      {		    
        sim_update_cnf->cause = SIM_CAUSE_PARAM_WRONG;
      }
      else
      {
        sim_update_cnf->cause =
             FKT_UpdateBinary (sim_update_req->trans_data,
                           (USHORT)sim_update_req->length,
                           sim_update_req->offset);

        app_start_status_timer (FALSE);

        if ((sim_update_cnf->cause EQ SIM_NO_ERROR) AND (sim_update_req->v_path_info EQ FALSE))
	{
          switch(sim_update_req->datafield) 
	  {
            case SIM_PLMNSEL:
              if (SIM_IS_FLAG_SET (SERVICE_7_SUPPORT))
	      {
                /*
                 * MM should be notified about the changed file so that MM can
		 * read this file.
                 */
                PALLOC (sim_mm_info_ind, SIM_MM_INFO_IND);
                sim_mm_info_ind->datafield = sim_update_req->datafield;
                PSENDX (MM, sim_mm_info_ind);
	      }
              break;
#ifdef REL99
            case SIM_UCPS_ACTEC:
              if (SIM_IS_FLAG_SET (SERVICE_43_SUPPORT))
	      {
                /*
                 * MM should be notified about the changed file so that MM can
		 * read this file.
                 */
                PALLOC (sim_mm_info_ind, SIM_MM_INFO_IND);
                sim_mm_info_ind->datafield = sim_update_req->datafield;
                PSENDX (MM, sim_mm_info_ind);
	      }
              break;
#endif
            default:
              break;
	  }
	}
      }
    }
  }
  else
  {
    sim_update_cnf->cause = SIM_CAUSE_CARD_REMOVED;
  }

  SIM_EM_UPDATE_BINARY_FILE;

  PFREE (sim_update_req);

#ifdef TI_PS_HCOMM_CHANGE
  vsi_c_psend (hComm_mux[source], (T_VOID_STRUCT*) sim_update_cnf FILE_LINE_MACRO);
#else
  vsi_c_psend (*hComm_mux[source], (T_VOID_STRUCT*) sim_update_cnf FILE_LINE_MACRO);
#endif
  /* Implements Measure# 3 */
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_update_record_req  |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_UPDATE_RECORD_REQ.

*/

GLOBAL void app_sim_update_record_req (T_SIM_UPDATE_RECORD_REQ * sim_update_record_req)
{
  USHORT         source;
  UBYTE response[SIMDRV_MAX_RESULT];

  PALLOC (sim_update_record_cnf, SIM_UPDATE_RECORD_CNF);

  TRACE_FUNCTION ("app_sim_update_record_req()");

  source = sim_update_record_req->source;

  sim_update_record_cnf->req_id = sim_update_record_req->req_id;

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    sim_update_record_cnf->cause = FKT_Select (sim_update_record_req->datafield,
                                               sim_update_record_req->v_path_info, &sim_update_record_req->path_info,
                                               response,SIM_MIN_EF_ST_LEN);

      if (sim_update_record_cnf->cause EQ SIM_NO_ERROR)
      {
        USHORT total_length;
        memcpy(&field_status,response,SIM_MIN_EF_ST_LEN);
        sim_data.field_type = field_status.field_type;
        sim_data.act_length = (USHORT)field_status.record_length;
        total_length = (USHORT)field_status.field_size[0] * 256 +
                       (USHORT)field_status.field_size[1];
        if (field_status.record_length)
        {
            sim_data.max_record = (UBYTE)(total_length / field_status.record_length);
        }
        else
        {
          sim_data.max_record = 1;  /* don't divide by zero */
        }
    }

    if ((sim_update_record_cnf->cause EQ SIM_NO_ERROR) OR 
        (sim_update_record_cnf->cause EQ SIM_NO_ERR_FILE_ALREADY_SELECTED))
    {
      /* field_status is global and has been updated either 
       * in "if" above or during previous operation on same file 
       */
        if (!(sim_data.act_access = app_check_access_conditions (ACCESS_UPDATE,
                                    &field_status)))
        {
          sim_update_record_cnf->cause = SIM_CAUSE_ACCESS_PROHIBIT;
        }
        else if ((sim_update_record_req->length > sim_data.act_length) OR
        ((sim_data.field_type EQ LINEAR_FIXED) AND
         (sim_update_record_req->record EQ 0)) OR
         (sim_update_record_req->record > sim_data.max_record))
        {
          sim_update_record_cnf->cause = SIM_CAUSE_ADDR_WRONG;
        }
        else 
        {
        if (sim_data.field_type EQ LINEAR_FIXED)
          {
            sim_update_record_cnf->cause =
               FKT_UpdateRecord (sim_update_record_req->linear_data,
                            (USHORT)sim_update_record_req->length,
                            4,
                            sim_update_record_req->record);
          }
          else
          {
            /*
             * Cyclic file
             */
            sim_update_record_cnf->cause =
                  FKT_UpdateRecord (sim_update_record_req->linear_data,
                            (USHORT)sim_update_record_req->length,
                            3,
                            0);
         }
        app_start_status_timer (FALSE);
      }
    }		
  }
  else
  {
    sim_update_record_cnf->cause = SIM_CAUSE_CARD_REMOVED;
  }

  SIM_EM_UPDATE_RECORD;

  PFREE (sim_update_record_req);

  /* Implements Measure# 4 */
#ifdef TI_PS_HCOMM_CHANGE
  vsi_c_psend (hComm_mux[source], (T_VOID_STRUCT*) sim_update_record_cnf FILE_LINE_MACRO);
#else
  vsi_c_psend (*hComm_mux[source], (T_VOID_STRUCT*) sim_update_record_cnf FILE_LINE_MACRO);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_increment_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_INCREMENT_REQ.

*/

GLOBAL void app_sim_increment_req (T_SIM_INCREMENT_REQ * sim_increment_req)
{
  USHORT         source;
  UBYTE response[SIMDRV_MAX_RESULT];

  PALLOC (sim_increment_cnf, SIM_INCREMENT_CNF);

  TRACE_FUNCTION ("app_sim_increment_req()");

  source = sim_increment_req->source;
  sim_increment_cnf->req_id     = sim_increment_req->req_id;

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    sim_increment_cnf->cause = FKT_Select (sim_increment_req->datafield,
                                           sim_increment_req->v_path_info,
					   &sim_increment_req->path_info,
                                           response,SIM_MIN_EF_ST_LEN );

      if (sim_increment_cnf->cause EQ SIM_NO_ERROR)
    {
      memcpy(&field_status,response,SIM_MIN_EF_ST_LEN);
      sim_data.field_type = field_status.field_type;
    }
    if ((sim_increment_cnf->cause EQ SIM_NO_ERROR) OR 
        (sim_increment_cnf->cause EQ SIM_NO_ERR_FILE_ALREADY_SELECTED))
    {
      /* field_status is global and has been updated either 
       * in "if" above or during previous operation on same file 
       */
        if (!(sim_data.act_access = app_check_access_conditions (ACCESS_INCREASE, &field_status)))
        {
          sim_increment_cnf->cause = SIM_CAUSE_ACCESS_PROHIBIT;
        }
      else
      {
        sim_increment_cnf->cause = SIM_NO_ERROR;
      }
    }
  }
  else
  {
    sim_increment_cnf->cause = SIM_CAUSE_CARD_REMOVED;
  }

  if (sim_increment_cnf->cause EQ SIM_NO_ERROR)
  {
    sim_increment_cnf->cause = FKT_Increase (sim_increment_req->linear_data);
    sim_increment_cnf->length = 0; /* sim_increment_req ->length; */

    app_start_status_timer (FALSE);
  }
  else
  {
    memset (&sim_increment_cnf->linear_data, 0,
	    sizeof (sim_increment_cnf->linear_data));
    sim_increment_cnf->length = 0;
  }

  SIM_EM_INCREMENT_FILE;

  PFREE (sim_increment_req);

  /* Implements Measure# 5 */
#ifdef TI_PS_HCOMM_CHANGE
  vsi_c_psend (hComm_mux[source], (T_VOID_STRUCT*) sim_increment_cnf FILE_LINE_MACRO);
#else
  vsi_c_psend (*hComm_mux[source], (T_VOID_STRUCT*) sim_increment_cnf FILE_LINE_MACRO);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_verify_pin_req     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_VERIFY_PIN_REQ.

*/

GLOBAL void app_sim_verify_pin_req (T_SIM_VERIFY_PIN_REQ * sim_verify_pin_req)
{
  USHORT source;             /* which entity has requested */
  USHORT error;              /* result of operation        */
  /*
   * allocate buffer for answer
   */
  PALLOC (sim_verify_pin_cnf, SIM_VERIFY_PIN_CNF);

  TRACE_FUNCTION ("app_sim_verify_pin_req()");

  SIM_EM_VERIFY_PIN;

  /*
   * fill the answer
   */
  memset (sim_verify_pin_cnf, 0, sizeof(T_SIM_VERIFY_PIN_CNF));
  source = sim_verify_pin_req->source;
  sim_verify_pin_cnf->pin_id = sim_verify_pin_req->pin_id;
  sim_data.last_requested_pin_no = sim_verify_pin_req->pin_id;
  /*
   * use SIM driver call for verification
   */
  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    error = FKT_VerifyCHV (sim_verify_pin_req->pin,
                           sim_verify_pin_req->pin_id);
  }
  else
    error = SIM_CAUSE_CARD_REMOVED;

  /*
   * deallocate incoming primitive
   */
  PFREE (sim_verify_pin_req);

  /*
   * check actual pin/puk counts
   */
  FKT_Status (&sim_verify_pin_cnf->pin_cnt,
              &sim_verify_pin_cnf->pin2_cnt,
              &sim_verify_pin_cnf->puk_cnt,
              &sim_verify_pin_cnf->puk2_cnt);

  /*
   * fill results for answer
   */
  sim_verify_pin_cnf->cause = error;

  /* Implements Measure# 6 */
#ifdef TI_PS_HCOMM_CHANGE
  vsi_c_psend (hComm_mux[source], (T_VOID_STRUCT*) sim_verify_pin_cnf FILE_LINE_MACRO);
#else
  vsi_c_psend (*hComm_mux[source], (T_VOID_STRUCT*) sim_verify_pin_cnf FILE_LINE_MACRO);
#endif
  /*
   * during initialization start
   * remaining part of initialisation procedure
   *
   */
  if (error EQ SIM_NO_ERROR)
  {
    if (SIM_IS_FLAG_CLEARED (MM_KNOWS_FROM_SIM))
    {
      app_sim_read_parameters ();
      app_start_status_timer (TRUE);
    }
    else
      app_start_status_timer (FALSE);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_change_pin_req     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_CHANGE_PIN_REQ.

*/

GLOBAL void app_sim_change_pin_req (T_SIM_CHANGE_PIN_REQ * sim_change_pin_req)
{
  USHORT source;

  PALLOC (sim_change_pin_cnf, SIM_CHANGE_PIN_CNF);

  TRACE_FUNCTION ("app_sim_change_pin_req()");

  SIM_EM_CHANGE_PIN;

  source = sim_change_pin_req->source;
  sim_change_pin_cnf->pin_id = sim_change_pin_req->pin_id;
  sim_data.last_requested_pin_no = sim_change_pin_req->pin_id;

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    sim_change_pin_cnf->cause = FKT_ChangeCHV (sim_change_pin_req->old_pin,
                                               sim_change_pin_req->new_pin,
                                               sim_change_pin_req->pin_id);
    app_start_status_timer (FALSE);
  }
  else
    sim_change_pin_cnf->cause = SIM_CAUSE_CARD_REMOVED;

  PFREE (sim_change_pin_req);

  /*
   * check actual pin/puk counts
   */
  FKT_Status (&sim_change_pin_cnf->pin_cnt,
              &sim_change_pin_cnf->pin2_cnt,
              &sim_change_pin_cnf->puk_cnt,
              &sim_change_pin_cnf->puk2_cnt);

  /* Implements Measure# 7 */
#ifdef TI_PS_HCOMM_CHANGE
  vsi_c_psend (hComm_mux[source], (T_VOID_STRUCT*) sim_change_pin_cnf FILE_LINE_MACRO);
#else
  vsi_c_psend (*hComm_mux[source], (T_VOID_STRUCT*) sim_change_pin_cnf FILE_LINE_MACRO);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_disable_pin_req    |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_DISABLE_PIN_REQ.

*/

GLOBAL void app_sim_disable_pin_req (T_SIM_DISABLE_PIN_REQ * sim_disable_pin_req)
{
  USHORT source;

  PALLOC (sim_disable_pin_cnf, SIM_DISABLE_PIN_CNF);

  TRACE_FUNCTION ("app_sim_disable_pin_req()");

  SIM_EM_DISABLE_PIN;

  /*
   * store source of request
   */
  source = sim_disable_pin_req->source;

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    /*
     * only if a SIM card is inserted
     */
    if (SIM_IS_FLAG_SET (SIM_PIN_FLAG))
    {
      /*
       * CHV1 entering was necessary, that means not disabled
       */
      if (SIM_IS_FLAG_SET (SERVICE_1_SUPPORT))
      {
        /*
         * SIM card supports disabling of CHV1
         */
        sim_data.last_requested_pin_no = LRP_PIN_1;
        sim_disable_pin_cnf->cause = FKT_DisableCHV (sim_disable_pin_req->pin);
        app_start_status_timer (FALSE);
      }
      else
        sim_disable_pin_cnf->cause = SIM_CAUSE_ACCESS_PROHIBIT;
    }
    else
      sim_disable_pin_cnf->cause = SIM_NO_ERROR;
  }
  else
    sim_disable_pin_cnf->cause = SIM_CAUSE_CARD_REMOVED;

  if (sim_disable_pin_cnf->cause EQ SIM_NO_ERROR)
  {
    SIM_CLEAR_FLAG (SIM_PIN_FLAG);
  }

  PFREE (sim_disable_pin_req);

  /*
   * check actual pin/puk counts
   */
  FKT_Status (&sim_disable_pin_cnf->pin_cnt,
              &sim_disable_pin_cnf->pin2_cnt,
              &sim_disable_pin_cnf->puk_cnt,
              &sim_disable_pin_cnf->puk2_cnt);

  /* Implements Measure# 2 to 8 */
#ifdef TI_PS_HCOMM_CHANGE
  vsi_c_psend (hComm_mux[source], (T_VOID_STRUCT*) sim_disable_pin_cnf FILE_LINE_MACRO);
#else
  vsi_c_psend (*hComm_mux[source], (T_VOID_STRUCT*) sim_disable_pin_cnf FILE_LINE_MACRO);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_enable_pin_req     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_ENABLE_PIN_REQ.

*/

GLOBAL void app_sim_enable_pin_req (T_SIM_ENABLE_PIN_REQ * sim_enable_pin_req)
{
  USHORT source;

  PALLOC (sim_enable_pin_cnf, SIM_ENABLE_PIN_CNF);

  TRACE_FUNCTION ("app_sim_enable_pin_req()");

  SIM_EM_ENABLE_PIN;

  source = sim_enable_pin_req->source;

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    if (SIM_IS_FLAG_SET (SIM_PIN_FLAG))
      sim_enable_pin_cnf->cause = SIM_NO_ERROR;
    else
    {
      sim_data.last_requested_pin_no = LRP_PIN_1;
      sim_enable_pin_cnf->cause = FKT_EnableCHV (sim_enable_pin_req->pin);
      app_start_status_timer (FALSE);
    }
  }
  else
    sim_enable_pin_cnf->cause = SIM_CAUSE_CARD_REMOVED;

  if (sim_enable_pin_cnf->cause EQ SIM_NO_ERROR)
  {
    SIM_SET_FLAG (SIM_PIN_FLAG);
  }

  PFREE (sim_enable_pin_req);

  /*
   * check actual pin/puk counts
   */
  FKT_Status (&sim_enable_pin_cnf->pin_cnt,
              &sim_enable_pin_cnf->pin2_cnt,
              &sim_enable_pin_cnf->puk_cnt,
              &sim_enable_pin_cnf->puk2_cnt);

  /* Implements Measure# 2 to 8 */
#ifdef TI_PS_HCOMM_CHANGE
  vsi_c_psend (hComm_mux[source], (T_VOID_STRUCT*) sim_enable_pin_cnf FILE_LINE_MACRO);
#else
  vsi_c_psend (*hComm_mux[source], (T_VOID_STRUCT*) sim_enable_pin_cnf FILE_LINE_MACRO);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_unblock_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_UNBLOCK_REQ.

*/

GLOBAL void app_sim_unblock_req (T_SIM_UNBLOCK_REQ * sim_unblock_req)
{
  USHORT source;
  USHORT error;

  PALLOC (sim_unblock_cnf, SIM_UNBLOCK_CNF);

  TRACE_FUNCTION ("app_sim_unblock_req()");

  SIM_EM_UNBLOCK_PIN;

  /*
   * fill the answer
   */
  memset (sim_unblock_cnf, 0, sizeof(T_SIM_VERIFY_PIN_CNF));
  source = sim_unblock_req->source;
  sim_unblock_cnf->pin_id = sim_unblock_req->pin_id;

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    if (sim_unblock_req->pin_id EQ PHASE_2_PUK_2)
    {
      sim_unblock_req->pin_id = UNBL_CHV2;      /* PUK2 */
      sim_data.last_requested_pin_no = LRP_PUK_2;
    }
    else
    {
      sim_unblock_req->pin_id = UNBL_CHV1;      /* PUK1 */
      sim_data.last_requested_pin_no = LRP_PUK_1;
    }
    error = FKT_UnblockCHV (sim_unblock_req->unblock_key,
                            sim_unblock_req->pin,
                            sim_unblock_req->pin_id);
  }
  else
    error = SIM_CAUSE_CARD_REMOVED;

  /*
   * deallocate incoming primitive
   */
  PFREE (sim_unblock_req);

  /*
   * check actual pin/puk counts regardless the outcome of
   * of the UNBLOCK operation
   */
  FKT_Status (&sim_unblock_cnf->pin_cnt,
              &sim_unblock_cnf->pin2_cnt,
              &sim_unblock_cnf->puk_cnt,
              &sim_unblock_cnf->puk2_cnt);

  /*
   * after a successful UNBLOCK of CHV1 the PIN is enabled,
   * otherwise its state remains unchanged
   */
  if (error EQ SIM_NO_ERROR AND
      sim_data.last_requested_pin_no EQ LRP_PUK_1)
  {
    SIM_SET_FLAG (SIM_PIN_FLAG);
  }
  /*
   * fill results for answer
   */
  sim_unblock_cnf->cause = error;

  /* Implements Measure# 8 */
#ifdef TI_PS_HCOMM_CHANGE
  vsi_c_psend (hComm_mux[source], (T_VOID_STRUCT*) sim_unblock_cnf FILE_LINE_MACRO);
#else
  vsi_c_psend (*hComm_mux[source], (T_VOID_STRUCT*) sim_unblock_cnf FILE_LINE_MACRO);
#endif
  /*
   * during initialization start
   * remaining part of initialisation procedure
   *
   */
  if (error EQ SIM_NO_ERROR)
  {
    if (SIM_IS_FLAG_CLEARED (MM_KNOWS_FROM_SIM))
    {
      app_sim_read_parameters ();
      app_start_status_timer (TRUE);
    }
    else
      app_start_status_timer (FALSE);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_auth_req           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_AUTHENTICATION_REQ.

*/

GLOBAL void app_sim_auth_req (T_SIM_AUTHENTICATION_REQ * sim_authentication_req)
{
  USHORT     error;
  USHORT     source;
  T_SRES_KC  sres_kc;
  T_kc_n     kc_n;
  UBYTE response[SIMDRV_MAX_RESULT];

  PALLOC (sim_authentication_cnf, SIM_AUTHENTICATION_CNF);

  TRACE_FUNCTION ("app_sim_auth_req()");

  SIM_EM_AUTHENTICATION;

  memset(response, 0, SIMDRV_MAX_RESULT);
  source = sim_authentication_req->source;
  kc_n.kc[MAX_KC] = sim_authentication_req->cksn;

  sim_authentication_cnf->req_id = sim_authentication_req->req_id;

  if (SIM_IS_FLAG_SET (GSM_DATAFIELD))
    error = FKT_Select (SIM_DF_GSM, FALSE, NULL, NULL, 0);
  else
    error = FKT_Select (SIM_DF_1800, FALSE, NULL, NULL, 0);

  if ((error EQ SIM_NO_ERROR) OR (error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED))
    error = FKT_RunGSMAlgo (sim_authentication_req->rand, response, SIM_GSM_ALG_LEN);

  if (error EQ SIM_NO_ERROR)
  {
     memcpy(&sres_kc,response,SIM_GSM_ALG_LEN);
     app_start_status_timer (FALSE);
  }
  if (error EQ SIM_NO_ERROR)
  {
    int i;
    /*lint -e{645} (when if statement TRUE, then 'sres_kc' valid) */
    memcpy (sim_authentication_cnf->sres, sres_kc.sres, 4);

    for (i = 0; i < MAX_KC; i++)
      sim_authentication_cnf->kc[(MAX_KC-1)-i] = kc_n.kc[i] = sres_kc.kc[i];

    switch (source)
    {
#if defined (GPRS)
      case SRC_GMM:
        PSENDX (GMM, sim_authentication_cnf);
        if (SIM_IS_FLAG_SET (SERVICE_38_SUPPORT))
        {
          error = FKT_Select (SIM_KCGPRS, FALSE, NULL, NULL, 0);  /* use SIM */
          if(error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)
            error = SIM_NO_ERROR;
        }
        else
        {                                   /* use PCM */
          T_imsi_field sim_imsi;
            error = FKT_Select (SIM_IMSI, FALSE, NULL, NULL, 0);
          if ((error EQ SIM_NO_ERROR OR error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)
               AND
               FKT_ReadBinary ((UBYTE *)&sim_imsi, 0, MAX_IMSI)
               EQ SIM_NO_ERROR)
          {
            if (gprs_check_pcm_data (&sim_imsi))
              pcm_WriteFile((UBYTE *)EF_KCGPRS_ID, SIZE_EF_KCGPRS,
                            (UBYTE *)kc_n.kc);
          }
          PFREE (sim_authentication_req);
          return;
        }
        break;
#endif
       default:
         PSENDX (MM, sim_authentication_cnf);
         error = FKT_Select (SIM_KC, FALSE, NULL, NULL, 0);
         if(error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)
             error = SIM_NO_ERROR;
         break;
    }
  }

  PFREE (sim_authentication_req);

  if (error EQ SIM_NO_ERROR)
  {
    kc_n.c_kc = MAX_KC_N;
    FKT_UpdateBinary (kc_n.kc, (USHORT)kc_n.c_kc, 0);
  }
//TISH, OMAPS00133714
//start
  else
  {
    sim_data.remove_error = error;
    app_sim_remove ();
    sim_data.remove_error = SIM_CAUSE_CARD_REMOVED;
  } 
//end
}

#ifdef TI_PS_UICC_CHIPSET_15
LOCAL void app_require_uicc_characteristics(T_SIMDRV_config_characteristics *config_characteristics)
{
  UBYTE clock_stop = 0;
  UBYTE voltage_class = 0;

  TRACE_FUNCTION("app_require_uicc_characteristics()");
  TRACE_EVENT_P1 ("SIMDRV - GSM SIM File Characteristics : 0x%02X", config_characteristics->uicc_characteristics);

  clock_stop = (config_characteristics->uicc_characteristics & 0x0D);/*get bit 1,3 and 4*/
  voltage_class = (config_characteristics->uicc_characteristics & 0x30);/*get bit 5 and 6*/

  switch(voltage_class) 
  {
    case 0x00:/*8GSM 5V - No voltage class bits set.*/
      voltage_class = 0x10; /*UMTS CLASS A - Bit 5 set*/
      break;
    case 0x10:/*GSM 3V - Voltage class bit 5 set.*/
      voltage_class = 0x20;/*UMTS CLASS B - Bit 6 set*/
      break;
    case 0x30:/*GSM 1.8V - Voltage class bit 5 and 6 set.*/
      voltage_class = 0x40;/*UMTS CLASS C - Bit 7 set*/
      break;
    default:
      voltage_class = 0x00;
      break;
  }
  config_characteristics->uicc_characteristics = (voltage_class | clock_stop);
  TRACE_EVENT_P1 ("SIMDRV - SIM File Characteristics (Converted) : 0x%02X", config_characteristics->uicc_characteristics);
  /* Clear traces of the fact that we have already used the card */
  sim_data.act_directory = NOT_PRESENT_16BIT;
  sim_data.act_field = NOT_PRESENT_16BIT;
}
#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_insert             |
+--------------------------------------------------------------------+

  PURPOSE : Hook function for SIM driver after SIM insertion.

*/

#ifndef TI_PS_UICC_CHIPSET_15
GLOBAL void app_sim_insert (T_SIM_CARD *p_atr)
#else /*!TI_PS_UICC_CHIPSET_15*/
GLOBAL void app_sim_insert( T_SIMDRV_atr_string_info     *atr_string_info,
                            U8       config_requested,
                            T_SIMDRV_config_characteristics     *config_characteristics)
#endif /*!TI_PS_UICC_CHIPSET_15*/
{
  USHORT       error;
  USHORT       length;
  T_DIR_STATUS dir_status;
  T_FIELD_STATUS field_status;
  UBYTE response[SIMDRV_MAX_RESULT];

  TRACE_FUNCTION ("app_sim_insert()");

#ifdef TI_PS_UICC_CHIPSET_15
  if (atr_string_info NEQ NULL)
  {
    /*
     * stop card detection timer
    */
    vsi_t_stop (VSI_CALLER SIM_TIMER);
  }
  else
  {
    /* SIM reinsert has been automatically detect and hence
       inform ACI */
    T_SIM_ACTIVATE_CNF * sim_activate_cnf;

    PALLOC (sim_activate, SIM_ACTIVATE_IND);
    sim_activate_cnf = (T_SIM_ACTIVATE_CNF *)sim_activate;

    sim_activate_cnf->cause = SIM_CAUSE_SIM_REINSERTED;

    PSENDX (MMI, sim_activate_cnf);

    return;
  }
#else
  /*
   * stop card detection timer
   */
  vsi_t_stop (VSI_CALLER SIM_TIMER);
#endif


#ifdef FF_DUAL_SIM
  if(sim_data.SIM_Selection)
  {
    T_SIM_ACTIVATE_CNF * sim_activate_cnf;

    PALLOC (sim_activate, SIM_ACTIVATE_CNF);
    sim_activate_cnf = sim_activate;

    sim_data.sim_num = SIM_GetSIM();

    sim_activate_cnf->cause = SIM_NO_ERROR;
    sim_activate_cnf->sim_num = (UBYTE)sim_data.sim_num;

    TRACE_EVENT_P1("SIM Selected is %d",sim_activate_cnf->sim_num);

    PSENDX (MMI, sim_activate_cnf);

    SIM_SET_FLAG (SIM_INSERT);

    return;
  }
#endif /*FF_DUAL_SIM*/
  
  /*
   * As per the section 6.5 of 3GPP TS 11.11, After the Answer To Reset (ATR), 
   * the Master File (MF) is implicitly selected and becomes the Current Directory.
   * 
   * Set Current Dir to MF, this makes the selection hit the DF_GSM as it is supposed to !
   */
  sim_data.act_directory = SIM_MF;
    
  /*
   * Read Datafield GSM
   */
  error = FKT_Select (SIM_DF_GSM, FALSE, NULL, response, SIM_MIN_DMF_ST_LEN);
  if (error EQ SIM_NO_ERROR OR error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)
  {
      length = (sim_data.sim_data_len < SIM_MIN_DMF_ST_LEN)?
             sim_data.sim_data_len: SIM_MIN_DMF_ST_LEN;
      memcpy(&dir_status,response,SIM_MIN_DMF_ST_LEN);
      #ifdef TI_PS_UICC_CHIPSET_15
       if (config_requested EQ SIMDRV_REQUEST_CONFIG_CHARACTERISTICS)
       {
         config_characteristics->uicc_characteristics = dir_status.characteristics;
         app_require_uicc_characteristics(config_characteristics);
         memset ((UBYTE *)&dir_status + length, 0, SIM_MIN_DMF_ST_LEN - length);
         return;
       }
      #endif
      memset ((UBYTE *)&dir_status + length, 0, SIM_MIN_DMF_ST_LEN - length);
  }
  if (error NEQ SIM_NO_ERROR)
  {
    /*
     * If not possible read Datafield DCS1800
     * for backward compatibility reasons
     */
    error = FKT_Select (SIM_DF_1800, FALSE, NULL, response, SIM_MIN_DMF_ST_LEN);
    if (error EQ SIM_NO_ERROR OR error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)
    {
      length = (sim_data.sim_data_len < SIM_MIN_DMF_ST_LEN)?
               sim_data.sim_data_len: SIM_MIN_DMF_ST_LEN;
      memcpy(&dir_status,response,SIM_MIN_DMF_ST_LEN);
      #ifdef TI_PS_UICC_CHIPSET_15
       if (config_requested EQ SIMDRV_REQUEST_CONFIG_CHARACTERISTICS)
       {
         config_characteristics->uicc_characteristics = dir_status.characteristics;
         app_require_uicc_characteristics(config_characteristics);
         memset ((UBYTE *)&dir_status + length, 0, SIM_MIN_DMF_ST_LEN - length);
         return;
       }
      #endif
      memset ((UBYTE *)&dir_status + length, 0, SIM_MIN_DMF_ST_LEN - length);
    }
    SIM_CLEAR_FLAG (GSM_DATAFIELD);
  }
  else
    SIM_SET_FLAG (GSM_DATAFIELD);

  if (error NEQ SIM_NO_ERROR)
  {
    /*
     * datafields are not readable
     */
    if (SIM_IS_FLAG_SET (ACTIVATION_STARTED))
    {
      app_sim_card_error ((USHORT)((SIM_IS_FLAG_SET(DRV_FAILED_RETRY))?
                           SIM_CAUSE_DRV_TEMPFAIL: SIM_CAUSE_OTHER_ERROR));
      SIM_CLEAR_FLAG (ACTIVATION_STARTED);
    }
    return;
  }

  {
    T_SIM_ACTIVATE_CNF * sim_activate_cnf;

    if (SIM_IS_FLAG_SET (ACTIVATION_STARTED))
    {
      PALLOC (sim_activate, SIM_ACTIVATE_CNF);
      sim_activate_cnf = sim_activate;
    }
    else
    {
      PALLOC (sim_activate, SIM_ACTIVATE_IND);
      sim_activate_cnf = (T_SIM_ACTIVATE_CNF *)sim_activate;
    }
    /*
     * Read Emergency Call Codes
     */
    memset (sim_activate_cnf->ec_code, NOT_PRESENT_8BIT, MAX_ECC);
    if (FKT_Select (SIM_ECC, FALSE, NULL, response, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)
    {
      memcpy(&field_status,response,SIM_MIN_EF_ST_LEN);
      {
        length = (USHORT)field_status.field_size[0] * 256 +
                  field_status.field_size[1];
        if (length > MAX_ECC)
          length = MAX_ECC;
        FKT_ReadBinary (sim_activate_cnf->ec_code, 0, length);
      }
    }
    /*
     * Read Preferred Language
     */
    memset (sim_activate_cnf->pref_lang, NOT_PRESENT_8BIT, MAX_LNG_PREF);
    if (FKT_Select (SIM_LP, FALSE, NULL, response, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)
    {
      memcpy(&field_status,response,SIM_MIN_EF_ST_LEN);
      {
        length = (USHORT)field_status.field_size[0] * 256 +
                  field_status.field_size[1];
        if (length > MAX_LNG_PREF)
          length = MAX_LNG_PREF;
        FKT_ReadBinary (sim_activate_cnf->pref_lang, 0, length);
      }
    }
    sim_activate_cnf->cause = SIM_NO_ERROR;
    /*
     * get ATR data
     */
     
    #ifndef TI_PS_UICC_CHIPSET_15
    length = MINIMUM(p_atr->atr_size, MAX_SIM_ATR);
    sim_activate_cnf->c_atr = (UBYTE)length;
    memcpy (sim_activate_cnf->atr, p_atr->atr_data, length);
    #else /*!TI_PS_UICC_CHIPSET_15*/
    length = MINIMUM(atr_string_info->c_atr_string, MAX_SIM_ATR);
    sim_activate_cnf->c_atr = (UBYTE)length;
    memcpy (sim_activate_cnf->atr, atr_string_info->atr_string, length);
    #endif /*!TI_PS_UICC_CHIPSET_15*/

    /*
     * check PIN/PUK status
     */
    SIM_CLEAR_FLAG (SIM_PIN_FLAG);
    /*lint -e{644} (only reachable when SIM_NO_ERROR, then 'dir_status' valid) */
    sim_activate_cnf->pin_cnt  = FKT_check_pin_count (dir_status.pinstatus);
    sim_activate_cnf->puk_cnt  = FKT_check_pin_count (dir_status.unbstatus);
    sim_activate_cnf->pin2_cnt = FKT_check_pin_count (dir_status.pin2status);
    sim_activate_cnf->puk2_cnt = FKT_check_pin_count (dir_status.unb2status);

    
    if (sim_activate_cnf->pin_cnt > 0)
    {
      /*
       * card is not blocked
       */
      if ((dir_status.characteristics & 0x80) EQ 0)
      {
        /*
         * PIN is enabled
         */
        sim_activate_cnf->cause = SIM_CAUSE_PIN1_EXPECT;
        SIM_SET_FLAG (SIM_PIN_FLAG);
      }
    }
    else if (sim_activate_cnf->puk_cnt > 0)
    {
      /*
       * SIM card is blocked, unblock attempts available
       */
      sim_activate_cnf->cause = SIM_CAUSE_PUK1_EXPECT;
      SIM_SET_FLAG (SIM_PIN_FLAG);
    }
    else
    {
      /*
       * SIM card is blocked, no unblock attempts available
       */
      TRACE_EVENT ("Card blocked");
      sim_activate_cnf->cause = SIM_CAUSE_PUK1_BLOCKED;

      SIM_EM_SIM_ACTIVATION_RESULT;

      PSENDX (MMI, sim_activate_cnf);
      SIM_CLEAR_FLAG (ACTIVATION_STARTED);
      return;
    }
    

    SIM_EM_SIM_ACTIVATION_RESULT;

    PSENDX (MMI, sim_activate_cnf);
    SIM_CLEAR_FLAG (ACTIVATION_STARTED);
  }

  SIM_SET_FLAG (SIM_INSERT);

  if (SIM_IS_FLAG_CLEARED (SIM_PIN_FLAG))
    /*
     * PIN entering is not necessary
     */
  {
    TRACE_EVENT ("Read the rest of Parameters");
    app_sim_read_parameters ();
    app_start_status_timer (TRUE);
  }
  else
  {
    TRACE_EVENT ("Wait for PIN/PUK entering");
    SIM_CLEAR_FLAG (MM_KNOWS_FROM_SIM);
    TIMER_STOP (sim_handle, SIM_TIMER);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_read_parameters    |
+--------------------------------------------------------------------+

  PURPOSE : Start the rest of the initialisation procedure.

*/

GLOBAL void app_sim_read_parameters (void)
{
  UBYTE result;

  PALLOC (sim_mmi_insert_ind, SIM_MMI_INSERT_IND);

  TRACE_FUNCTION ("app_sim_read_parameters()");

  app_sim_phase ();

  switch (sim_data.sim_phase)
  {
    case 1:
      // try to read the SIM service table
      if (!app_read_sim_service_table(sim_mmi_insert_ind))
      {
        app_sim_mmi_insert_ind (sim_mmi_insert_ind, SIM_NO_OPERATION);
        return;
      }
      if (app_sim_mm_insert_ind (sim_mmi_insert_ind) EQ FALSE)
        app_sim_mmi_insert_ind (sim_mmi_insert_ind, SIM_NO_OPERATION);
      else
        app_sim_mmi_insert_ind (sim_mmi_insert_ind, SIM_ADN_ENABLED) ;

      PSENDX (MMI, sim_mmi_insert_ind);
      app_sim_sms_insert_ind ();
      break;
    case 3:
#if defined SIM_TOOLKIT
      stk_perform_profile_download ();

       /* Update the Terminal Support table*/
      {
        T_path_info tmp_path;
        tmp_path.df_level1 = SIM_DF_CING;
        tmp_path.v_df_level2 = FALSE;

        if(FKT_Select(SIM_CING_TRMST, TRUE, &tmp_path, NULL, 0) EQ SIM_NO_ERROR)
        {
          FKT_UpdateBinary (sim_data.trmst, MAX_TRMST, 0);
        }
      }
      SIM_EM_SIM_TOOLKIT_ACTIVATION;

#endif
    /* no break;*/
    /*lint -fallthrough*/
    case 2:
      /* includes reading of the SIM service table*/
      result = app_fdn_bdn_procedures (sim_mmi_insert_ind);
      if ((result EQ SIM_NO_OPERATION) OR
          !app_sim_mm_insert_ind (sim_mmi_insert_ind))
        app_sim_mmi_insert_ind (sim_mmi_insert_ind, SIM_NO_OPERATION);
      else
        app_sim_mmi_insert_ind (sim_mmi_insert_ind, result);

      PSENDX (MMI, sim_mmi_insert_ind);
      if (result NEQ SIM_NO_OPERATION)
        app_sim_sms_insert_ind ();
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_phase              |
+--------------------------------------------------------------------+

  PURPOSE : Read the phase identification.

*/

GLOBAL void app_sim_phase (void)
{
  USHORT error;

  TRACE_FUNCTION ("app_sim_phase()");
  error = FKT_Select (SIM_PHASE, FALSE, NULL, NULL, 0);
  if (error EQ SIM_NO_ERROR OR error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)
    error = FKT_ReadBinary (&sim_data.sim_phase, 0, 1);

  /*
   * Default is phase 1 SIM card
   * G23 interpretation is
   * 1: Phase 1
   * 2: Phase 2
   * 3: Phase 2+
   */
  if (error NEQ SIM_NO_ERROR)
    sim_data.sim_phase = 1;
  else
  {
    if (sim_data.sim_phase EQ 0)
      sim_data.sim_phase = 1;
//TISH, patch for OMAPS00122397
//start
     else if (sim_data.sim_phase EQ 0xFF)
          sim_data.sim_phase = 3;
//end
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_read_sim_service_table |
+--------------------------------------------------------------------+

  PURPOSE : Read the SIM Service Table.

*/

LOCAL UBYTE app_read_sim_service_table (T_SIM_MMI_INSERT_IND * sim_mmi_insert_ind)
{
  USHORT length;
  USHORT error;
  T_FIELD_STATUS field_status;
  UBYTE response[SIMDRV_MAX_RESULT];

  /*
   * read SIM service table
   * Currently selected EF information is reset to force the selection 
   */
  sim_data.act_field = NOT_PRESENT_16BIT;

  error = FKT_Select (SIM_SST, FALSE, NULL, response, SIM_MIN_EF_ST_LEN);
  if (error EQ SIM_NO_ERROR)
  {
     memcpy(&field_status,response,SIM_MIN_EF_ST_LEN);

     length = (USHORT)field_status.field_size[0] * 256
               + field_status.field_size[1];
     if (length > MAX_SRV_TBL)

       length = MAX_SRV_TBL;
     memset (sim_mmi_insert_ind->sim_serv, 0, MAX_SRV_TBL);
     if (FKT_ReadBinary (sim_mmi_insert_ind->sim_serv, 0, length) NEQ SIM_NO_ERROR)
       return FALSE;
  }
  else
    return FALSE;

 /*
  * set flags according to the allocated and activated services
  */
  if (SERVICE(1,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_1_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_1_SUPPORT);

  if (SERVICE(2,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_2_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_2_SUPPORT);

  if (SERVICE(3,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_3_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_3_SUPPORT);

  if (SERVICE(4,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_4_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_4_SUPPORT);

  if (SERVICE(7,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_7_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_7_SUPPORT);

  if (SERVICE(26,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_26_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_26_SUPPORT);

  if (SERVICE(31,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_31_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_31_SUPPORT);

  if (SERVICE(35,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_35_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_35_SUPPORT);
  
  if ((SERVICE(39,sim_mmi_insert_ind->sim_serv) & ALLOCATED)
       AND FKT_Select (SIM_DF_GRAPHICS, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
    SIM_SET_FLAG (DF_GRAPHICS_EXISTENT);
  else
    SIM_CLEAR_FLAG (DF_GRAPHICS_EXISTENT);
  
  if ((SERVICE(40,sim_mmi_insert_ind->sim_serv) & ALLOCATED)
       AND FKT_Select (SIM_DF_SOLSA, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
    SIM_SET_FLAG (DF_SOLSA_EXISTENT);
  else
    SIM_CLEAR_FLAG (DF_SOLSA_EXISTENT);

  /* @@TODO -- CHECK THE SERVICE NUMBER */
  if (FKT_Select (SIM_DF_ORANGE, FALSE,NULL, NULL, 0) EQ SIM_NO_ERROR)
    SIM_SET_FLAG (DF_ORANGE_EXISTENT);
  else
    SIM_CLEAR_FLAG (DF_ORANGE_EXISTENT);

#ifdef REL99
  /*
   * SET SERVICE FLAG for SERVICE 43 if "user controlled PLMN Selector with 
   * Access Technology" is supported
   */
  if (SERVICE(43,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_43_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_43_SUPPORT);

  /*
   * SET SERVICE FLAG for SERVICE 44 if "Operator controlled PLMN Selector with 
   * Access Technology" is supported
   */  
  if (SERVICE(44,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_44_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_44_SUPPORT);
#endif /* REl99 */

#if defined (GPRS)
  if (SERVICE(38,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (SERVICE_38_SUPPORT);
  else
    SIM_CLEAR_FLAG (SERVICE_38_SUPPORT);
#endif

#ifdef SIM_TOOLKIT
  if (SERVICE(29,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
    SIM_SET_FLAG (PRO_ACTIVE_SIM);
  else
    SIM_CLEAR_FLAG (PRO_ACTIVE_SIM);
#endif
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_mm_insert_ind      |
+--------------------------------------------------------------------+

  PURPOSE : Reads some fields and build SIM_MM_INSERT_IND.

*/

GLOBAL UBYTE app_sim_mm_insert_ind (T_SIM_MMI_INSERT_IND * sim_mmi_insert_ind)
{
  UBYTE  kc_n [MAX_KC_N];
  int    i;
  USHORT error;
  USHORT  length;
  T_FIELD_STATUS field_status;
  USHORT FileSelRes;
  UBYTE response[SIMDRV_MAX_RESULT];
  T_path_info  tmp_path;
  
  /*
   * Read remaining parameters for mobility management
   */
  PALLOC (sim_mm_insert_ind, SIM_MM_INSERT_IND);

  TRACE_FUNCTION ("app_sim_mm_insert_ind()");

  SIM_EM_READ_MM_PARAMETER;

  SIM_CLEAR_FLAG (TEST_SIM_INSERTED);
#if defined SIM_TOOLKIT
  SIM_CLEAR_FLAG (TEST_MODE_POLLING);
#endif
  /*
   * read administrative data
   */
  memset (&sim_mmi_insert_ind->ad, 0, MAX_AD);
  error = FKT_Select (SIM_AD, FALSE, NULL, response, SIM_MIN_EF_ST_LEN);

  if ( error EQ SIM_NO_ERROR OR error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)
  {
    memcpy(&field_status, response, SIM_MIN_EF_ST_LEN);
    {
      length = field_status.field_size[0] * 256 +
                  field_status.field_size[1];
      
      if(length > MAX_AD)
        length = MAX_AD;
      
      sim_mm_insert_ind->c_ad = (UBYTE)length;
      if(FKT_ReadBinary ((UBYTE *)&sim_mm_insert_ind->ad, 0, length) EQ SIM_NO_ERROR)
      {
        TRACE_EVENT_P1("The length of AD is %d", length);
      }
      else
      {
        PFREE (sim_mm_insert_ind);
        return FALSE;
      }
    }

    sim_mmi_insert_ind->c_ad = sim_mm_insert_ind->c_ad;
    memcpy (&sim_mmi_insert_ind->ad, &sim_mm_insert_ind->ad,sim_mmi_insert_ind->c_ad);


    if(sim_mm_insert_ind->ad[0] & 0x80)
    {
      SIM_SET_FLAG (TEST_SIM_INSERTED);
#if defined SIM_TOOLKIT
      TRACE_EVENT("REG POLLING");
      SIM_SET_FLAG (TEST_MODE_POLLING);
#endif
    }

  }
  else
  {
    PFREE (sim_mm_insert_ind);
    return FALSE;
  }

  /*
   * read IMSI (set to zero in case of error)
   */
  memset (&sim_mmi_insert_ind->imsi_field, 0, sizeof (T_imsi_field));
  if (FKT_Select (SIM_IMSI, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
  {
    if (FKT_ReadBinary ((UBYTE *)&sim_mm_insert_ind->imsi_field, 0, MAX_IMSI)
        NEQ SIM_NO_ERROR)
    {
      PFREE (sim_mm_insert_ind);
      return FALSE;
    }
    /*
     * Check length of IMSI for validity
     */
    if ((sim_mm_insert_ind->imsi_field.c_field <= 3) OR
        (sim_mm_insert_ind->imsi_field.c_field > MAX_IMSI-1))
    {
      PFREE (sim_mm_insert_ind);
      return FALSE;
    }
    memcpy (&sim_mmi_insert_ind->imsi_field, &sim_mm_insert_ind->imsi_field,
            sizeof (T_imsi_field));
    /*
     * Modify polling algorithm in case of HPLMN == Test Network
     */
    if ((sim_mm_insert_ind->imsi_field.c_field >= 3) AND
        ((sim_mm_insert_ind->imsi_field.field[0] & 0xF7) EQ 0x01) AND
        (sim_mm_insert_ind->imsi_field.field[1] EQ 0x10) AND
        (sim_mm_insert_ind->imsi_field.field[2] EQ 0x10))
    {
      SIM_SET_FLAG (TEST_SIM_INSERTED);
#if defined SIM_TOOLKIT
      TRACE_EVENT("REG POLLING");
      SIM_SET_FLAG (TEST_MODE_POLLING);
#endif
    }
  }
  else
  {
    PFREE (sim_mm_insert_ind);
    return FALSE;
  }

  /*
   * read location information
   */
  if (FKT_Select (SIM_LOCI, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
  {
    sim_mm_insert_ind->loc_info.c_loc = MAX_LOC_INFO;
    if (FKT_ReadBinary ((UBYTE *)sim_mm_insert_ind->loc_info.loc, 0,
                         MAX_LOC_INFO) NEQ SIM_NO_ERROR)
    {
      PFREE (sim_mm_insert_ind);
      return FALSE;
    }
  }
  else
  {
    PFREE (sim_mm_insert_ind);
    return FALSE;
  }

  /*
   * Access control classes
   */
  if (FKT_Select (SIM_ACC, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
  {
    sim_mm_insert_ind->acc_ctrl.c_acc = MAX_ACCESS_CONTROL;
    if (FKT_ReadBinary ((UBYTE *)sim_mm_insert_ind->acc_ctrl.acc, 0,
                         MAX_ACCESS_CONTROL) NEQ SIM_NO_ERROR)
    {
      PFREE (sim_mm_insert_ind);
      return FALSE;
    }
  }
  else
  {
    PFREE (sim_mm_insert_ind);
    return FALSE;
  }

  /*
   * BCCH information
   */
  if (FKT_Select (SIM_BCCH, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
  {
    sim_mm_insert_ind->bcch_inf.c_bcch = MAX_BCCH_INFO;
    if (FKT_ReadBinary ((UBYTE *)sim_mm_insert_ind->bcch_inf.bcch, 0,
                        MAX_BCCH_INFO) NEQ SIM_NO_ERROR)
    {
      PFREE (sim_mm_insert_ind);
      return FALSE;
    }
  }
  else
  {
    PFREE (sim_mm_insert_ind);
    return FALSE;
  }

  /*
   * KC and cipher key sequence number
   */
  if (FKT_Select (SIM_KC, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
  {
    if (FKT_ReadBinary ((UBYTE *)kc_n, 0, MAX_KC_N) NEQ SIM_NO_ERROR)
    {
      PFREE (sim_mm_insert_ind);
      return FALSE;
    }
    else
    {
      sim_mm_insert_ind->kc_n.c_kc = MAX_KC_N;
      /*
       * Store KC in opposite order
       */
      for (i = 0; i < MAX_KC; i++)
        sim_mm_insert_ind->kc_n.kc[(MAX_KC-1)-i] = kc_n[i];
      /*
       * Store cipher key sequence number
       */
      sim_mm_insert_ind->kc_n.kc[MAX_KC]   = kc_n[MAX_KC];
    }
  }
  else
  {
    PFREE (sim_mm_insert_ind);
    return FALSE;
  }
  
  /*
   * Read Preferred PLMN information
   * Initialize preferred plmn information is not present
   */
  sim_mm_insert_ind->pref_plmn_list_sz = 
     app_get_ef_size(SERVICE_7_SUPPORT, SIM_PLMNSEL,response);

  /*
   * Read forbidden PLMNs
   */
  if (FKT_Select (SIM_FPLMN, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
  {
    sim_mm_insert_ind->forb_plmn.c_forb = MAX_FORB_PLMN;
    if (FKT_ReadBinary ((UBYTE *)sim_mm_insert_ind->forb_plmn.forb, 0,
                        MAX_FORB_PLMN) NEQ SIM_NO_ERROR)
    {
      PFREE (sim_mm_insert_ind);
      return FALSE;
    }
  }
  else
  {
    PFREE (sim_mm_insert_ind);
    return FALSE;
  }

#ifdef REL99
  /*
   * Extract "User controlled PLMN Selector with Access Technology" Information
   * Initially set 'usr ctrl plmn selector with Access Technology informaiton' is not present
   */
  sim_mm_insert_ind->u_ctl_plmn_sel_actech_list_sz = 
    app_get_ef_size(SERVICE_43_SUPPORT, SIM_UCPS_ACTEC,response);

  /*
   * Extract "Operator controlled PLMN Selector with Access Technology" Information
   * Initailly set 'operator ctrl plmn with Access Technology informaiton' is not present
   */
  sim_mm_insert_ind->o_ctl_plmn_sel_actech_list_sz = 
    app_get_ef_size(SERVICE_44_SUPPORT, SIM_OCPS_ACTEC,response);

#endif /* REl99 */

  /*
   * Read Acting HPLMN 
   */
  tmp_path.df_level1   = SIM_DF_CING;
  tmp_path.v_df_level2 = TRUE;
  tmp_path.df_level2   = SIM_DF2_CING;

  FileSelRes = FKT_Select(SIM_CING_AHPLMN, TRUE, &tmp_path, NULL, 0);

  if( FileSelRes EQ SIM_NO_ERROR)
  {
    sim_mm_insert_ind->v_act_hplmn = TRUE;
    if(FKT_ReadBinary ((UBYTE *)sim_mm_insert_ind->act_hplmn, 0,
                       MAX_SIM_PLMN_SIZE) NEQ SIM_NO_ERROR)
    {
      sim_mm_insert_ind->v_act_hplmn = FALSE;
    }
  }
  else
  {
    sim_mm_insert_ind->v_act_hplmn = FALSE;
  }
  
  
  /*
   * set phase identification and
   * default value for hplmn search period
   * for phase 1 card.
   */
  sim_mm_insert_ind->phase = sim_data.sim_phase;
  /*
   * 3GPP 23.122 clause 4.4.3.3 states the default search period
   * is now 60 minutes. This is different from ETSI 03.22
   * clause 4.4.3.3 where the default value was only 30 minutes.
   */
  sim_mm_insert_ind->hplmn = 10;

  if (sim_data.sim_phase NEQ PHASE_1_SIM)
  {
    if (FKT_Select (SIM_HPLMN, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
    {
      if (FKT_ReadBinary (&sim_mm_insert_ind->hplmn, 0, 1)
          NEQ SIM_NO_ERROR)
        sim_mm_insert_ind->hplmn = 10;
    }
  }

#if defined (GPRS)
  gprs_gmm_insert_ind (sim_mm_insert_ind);
#endif

  /*
   * send information to mobility management
   */
  PSENDX (MM, sim_mm_insert_ind);
  SIM_SET_FLAG (MM_KNOWS_FROM_SIM);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_mmi_insert_ind     |
+--------------------------------------------------------------------+

  PURPOSE : Build the primitive SIM_MMI_INSERT_IND.
*/

GLOBAL void app_sim_mmi_insert_ind (T_SIM_MMI_INSERT_IND * sim_mmi_insert_ind, UBYTE func)
{
  T_FIELD_STATUS field_status;
  USHORT  length;
  UBYTE response[SIMDRV_MAX_RESULT];

  TRACE_FUNCTION ("app_sim_mmi_insert_ind()");

  SIM_EM_READ_MMI_PARAMETER;

  /*
   * copy parameters known from MM INSERT reading
   */

  sim_mmi_insert_ind->func = func;

  if (func & 1)
    SIM_SET_FLAG (ADN_SUPPORT_BY_SIM);
  else
    SIM_CLEAR_FLAG (ADN_SUPPORT_BY_SIM);

  if (func & 2)
    SIM_SET_FLAG (FDN_SUPPORT_BY_SIM);
  else
    SIM_CLEAR_FLAG (FDN_SUPPORT_BY_SIM);

  if (func & 4)
    SIM_SET_FLAG (BDN_SUPPORT_BY_SIM);
  else
    SIM_CLEAR_FLAG (BDN_SUPPORT_BY_SIM);

  sim_mmi_insert_ind->phase = sim_data.sim_phase;

  /*
   * check access conditions for AoC fields
   * Currently selected EF information is reset to force the selection
   */

  sim_data.act_field = NOT_PRESENT_16BIT;
  sim_mmi_insert_ind->access_acm = NOT_PRESENT_8BIT;
  if (FKT_Select (SIM_ACM, FALSE, NULL, response, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)
  {
    memcpy(&field_status, response, SIM_MIN_EF_ST_LEN);
    sim_mmi_insert_ind->access_acm = field_status.access_1 & 0x0F;
  }
  
  sim_mmi_insert_ind->access_acmmax = NOT_PRESENT_8BIT;
  if (FKT_Select (SIM_ACMMAX, FALSE, NULL, response, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)
  {
    memcpy(&field_status, response, SIM_MIN_EF_ST_LEN);
    sim_mmi_insert_ind->access_acmmax = field_status.access_1 & 0x0F;
  }
  
  sim_mmi_insert_ind->access_puct   = NOT_PRESENT_8BIT;
  if (FKT_Select (SIM_PUCT, FALSE, NULL, response, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)
  {
    memcpy(&field_status, response, SIM_MIN_EF_ST_LEN);
    sim_mmi_insert_ind->access_puct = field_status.access_1 & 0x0F;
  }

  TRACE_EVENT_P1 ("Access ACM    = %d", sim_mmi_insert_ind->access_acm);
  TRACE_EVENT_P1 ("Access ACMMAX = %d", sim_mmi_insert_ind->access_acmmax);
  TRACE_EVENT_P1 ("Access PUCT   = %d", sim_mmi_insert_ind->access_puct);

#ifdef SIM_TOOLKIT
  if ( (SERVICE(25,sim_mmi_insert_ind->sim_serv) EQ ALLOCATED_AND_ACTIVATED)
        AND  (sim_data.stk_profile[0] & SAT_TP1_CB_DNL) )
  {
    if (FKT_Select (SIM_CBMID, FALSE, NULL, response, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)
    {
      memcpy(&field_status, response, SIM_MIN_EF_ST_LEN);
      {
        length = field_status.field_size[0] * 256 +
                    field_status.field_size[1];
      
        length =  MINIMUM(length, MAX_CBMID_REC);

        sim_mmi_insert_ind->cbmid_rec.c_rec = (UBYTE)length;
      
        if(FKT_ReadBinary ((UBYTE *)sim_mmi_insert_ind->cbmid_rec.rec, 0, length) 
              EQ SIM_NO_ERROR)
        {
          TRACE_EVENT_P1("The length of CBMID is %d", length);
        }
      }
    }
  }
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_sms_insert_ind     |
+--------------------------------------------------------------------+

  PURPOSE : Build the primitive SIM_SMS_INSERT_IND.
*/

GLOBAL void app_sim_sms_insert_ind (void)
{
  PALLOC (sim_sms_insert_ind, SIM_SMS_INSERT_IND);

  TRACE_FUNCTION ("app_sim_sms_insert_ind()");

  SIM_EM_READ_SMS_PARAMETER;

  memset (sim_sms_insert_ind, 0, sizeof(T_SIM_SMS_INSERT_IND));
  sim_sms_insert_ind->mem_cap_avail = SIM_SMS_NO_MEM;
  /*
   * Read SMS Status
   */
  if (SIM_IS_FLAG_SET (SERVICE_4_SUPPORT))
  {
    USHORT     error;
    error = FKT_Select (SIM_SMSS, FALSE, NULL, NULL, 0);
    if (error EQ SIM_NO_ERROR OR error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)
    {
      if (FKT_ReadBinary (&sim_sms_insert_ind->tp_mr, 0, 2)
                           EQ SIM_NO_ERROR)
      {
        sim_sms_insert_ind->mem_cap_avail &= 1;
      }
      else
      {
        sim_sms_insert_ind->mem_cap_avail = SIM_SMS_MEM_AVAIL;
      }
    }
    else if (sim_data.sim_phase < PHASE_2_SIM)
    {
      sim_sms_insert_ind->mem_cap_avail = SIM_SMS_MEM_AVAIL;
    }
  }
  /*
   * check SIM phase and determine support of SMS data download
   */
  sim_sms_insert_ind->phase = sim_data.sim_phase;

  switch (sim_data.sim_phase)
  {
    case 1:
    case 2:
      sim_sms_insert_ind->download_sms = DOWNLOAD_SMS_NO;
      break;
    case 3:
#ifdef SIM_TOOLKIT
      /*
       * check service 26: data download via point-to-point SMS
       */
      if (SIM_IS_FLAG_SET (SERVICE_26_SUPPORT))
        sim_sms_insert_ind->download_sms = DOWNLOAD_SMS_YES;
      else
#endif
        sim_sms_insert_ind->download_sms = DOWNLOAD_SMS_NO;
      break;
  }
  /*
   * check service 35: store SM Status Reports
   */
  if (SIM_IS_FLAG_SET (SERVICE_35_SUPPORT))
    sim_sms_insert_ind->smsr_mem_cap = SIM_SMSR_ENABLE;
  else
    sim_sms_insert_ind->smsr_mem_cap = SIM_SMSR_DISABLE;

  /*
   * send information to short message service
   */
  PSENDX (SMS, sim_sms_insert_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_remove             |
+--------------------------------------------------------------------+

  PURPOSE : Hook function for SIM driver after SIM remove.

*/

GLOBAL void app_sim_remove (void)
{
  TRACE_FUNCTION ("app_sim_remove()");
#ifdef SIM_TOOLKIT
  stk_stop_all_sat_timers ();
#endif
  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    {
      PALLOC (sim_remove_ind_to_mm , SIM_REMOVE_IND);
      sim_remove_ind_to_mm->cause = sim_data.remove_error;
      PSENDX (MM, sim_remove_ind_to_mm);
    }
#ifdef GPRS
    {
      PALLOC (sim_remove_ind_to_gmm , SIM_REMOVE_IND);
      sim_remove_ind_to_gmm->cause = sim_data.remove_error;
      PSENDX (GMM, sim_remove_ind_to_gmm);
    }
#endif
    {
      PALLOC (sim_remove_ind_to_mmi, SIM_REMOVE_IND);
      sim_remove_ind_to_mmi->cause = sim_data.remove_error;
      PSENDX (MMI, sim_remove_ind_to_mmi);
    }
    {
      PALLOC (sim_remove_ind_to_sms, SIM_REMOVE_IND);
      sim_remove_ind_to_sms->cause = sim_data.remove_error;
      PSENDX (SMS, sim_remove_ind_to_sms);
    }
    SIM_EM_SIM_REMOVE;
  }
  SIM_CLEAR_FLAG (SIM_INSERT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_timeout            |
+--------------------------------------------------------------------+

  PURPOSE : A timeout has occured.

*/

GLOBAL void app_sim_timeout (U16 timer)
{
  USHORT error;
  UBYTE pin_cnt, pin2_cnt, puk_cnt, puk2_cnt;

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    /*
     * Presence check all thirty seconds
     */
//#ifdef SIM_TOOLKIT	// FreeCalypso failed attempt to unbreak !SIM_TOOLKIT
    if (timer EQ SLEEP_TIMER AND sim_data.sat_session EQ TRUE)
    {
      TRACE_EVENT("STK SESSION RUNNING");
      return;
    }
//#endif
     	
    if ((error = FKT_Status (&pin_cnt, &pin2_cnt, &puk_cnt, &puk2_cnt))
        NEQ SIM_NO_ERROR)
    {
    /* stop Poll Timer*/
      TIMER_STOP (sim_handle, SIM_TIMER);

      sim_data.remove_error = error;
      app_sim_remove ();
      sim_data.remove_error = SIM_CAUSE_CARD_REMOVED;
    }
  
#ifdef TI_PS_FF_SIM_POLL_ALWAYS
  
    else
    {
#if defined SIM_TOOLKIT
      if(SIM_IS_FLAG_SET (PRO_ACTIVE_SIM))
      {
      stk_proactive_polling();
      }
      else
#endif
      {
          TRACE_FUNCTION ("Restarting timer for polling non-proactive SIMs");
          app_start_status_timer(FALSE);
      }
    }
  
#else
  
#if defined SIM_TOOLKIT
    else
      stk_proactive_polling();
#endif
  
#endif
  
  }
  else if (SIM_IS_FLAG_SET (ACTIVATION_STARTED))
  {
    /*
     * Timeout indicates no recognition of
     * a SIM card by the driver
     */
    app_sim_card_error (SIM_CAUSE_CARD_REMOVED);
    SIM_CLEAR_FLAG (ACTIVATION_STARTED);
  }
  else if (SIM_IS_FLAG_SET(DRV_FAILED_RETRY))
  {
    /*
     * Timeout indicates failed recovery after
     * a SIM driver retry failure
     */
    app_sim_remove();
    SIM_CLEAR_FLAG(DRV_FAILED_RETRY);
  }
  else
  {
    /*
     * Timer may still run in case of auto-restart
     */
    TIMER_STOP (sim_handle, SIM_TIMER);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_card_error         |
+--------------------------------------------------------------------+

  PURPOSE : An error is signalled to MMI.

*/

GLOBAL void app_sim_card_error (USHORT error)
{
  PALLOC (sim_activate_cnf, SIM_ACTIVATE_CNF);

  memset (sim_activate_cnf, NOT_PRESENT_8BIT, sizeof (T_SIM_ACTIVATE_CNF));

#ifdef FF_DUAL_SIM
  if(sim_data.SIM_Selection)
  {
    sim_activate_cnf->sim_num = SIM_NUM_0;
    sim_activate_cnf->cause = error;

    PSENDX (MMI, sim_activate_cnf);

    return;
  }
#endif /*FF_DUAL_SIM*/

  sim_activate_cnf->pin_cnt = sim_activate_cnf->puk_cnt = 0;
  sim_activate_cnf->pin2_cnt = sim_activate_cnf->puk2_cnt = 0;
  sim_activate_cnf->cause = error;
  sim_activate_cnf->c_atr = 0;

  SIM_EM_SIM_ACTIVATION_RESULT;

  PSENDX (MMI, sim_activate_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_mm_update_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process of the primitive SIM_MM_UPDATE_REQ.

*/

GLOBAL void app_sim_mm_update_req (T_SIM_MM_UPDATE_REQ * sim_mm_update_req)
{
  int    i;
  USHORT error;
  T_kc_n kc_n;
  BOOL all_upd = TRUE;

  TRACE_FUNCTION ("app_sim_mm_update_req()");

  /*
   * check location information
   */
  if (sim_mm_update_req->loc_info.c_loc > 0)
  {
#if defined SIM_TOOLKIT
    memcpy (&sim_data.location_info, &sim_mm_update_req->loc_info,
            sizeof (sim_data.location_info));
#endif
    if(sim_mm_update_req->ef_indicator & 0x01)
    {
      error = FKT_Select (SIM_LOCI, FALSE, NULL, NULL, 0);
      if (error EQ SIM_NO_ERROR)
        error = FKT_UpdateBinary (sim_mm_update_req->loc_info.loc,
                                  (USHORT)sim_mm_update_req->loc_info.c_loc, 0);
      if (error NEQ SIM_NO_ERROR)
      {
  #ifdef REL99
        if (error EQ SIM_CAUSE_MEM_PROBLEM)
        {
          /*
           * In case when updating EF LOCI with data containing the 
           * TMSI value and the card reports the error '92 40' (Memory Problem),
           * the ME shall terminate GSM operation.
           */
          sim_data.remove_error = SIM_CAUSE_MEM_PROBLEM;
          app_sim_remove();
          PFREE (sim_mm_update_req);
          return;
        }
  #endif /* end of ifdef REL99 */
        all_upd = FALSE;
      }
    }
#if defined SIM_TOOLKIT
    sim_data.cell_identity = sim_mm_update_req->cell_identity;
    /*
     * Modify polling algorithm if connected to Test Network
     */
    if ((sim_mm_update_req->loc_info.c_loc >= 7) AND
        (sim_mm_update_req->loc_info.loc[4] EQ 0x00) AND
        ((sim_mm_update_req->loc_info.loc[5] & 0x0F) EQ 0x1) AND
        (sim_mm_update_req->loc_info.loc[6] EQ 0x10))
    {
      TRACE_EVENT("REG POLLING");
      SIM_SET_FLAG (TEST_MODE_POLLING);
    }
    else if (SIM_IS_FLAG_CLEARED (TEST_SIM_INSERTED))
    {
      TRACE_EVENT("STD POLLING");
      SIM_CLEAR_FLAG (TEST_MODE_POLLING);
    }
#endif
  }

  /*
   * check bcch information
   */
  if(sim_mm_update_req->ef_indicator & 0x02)
  {
    if (sim_mm_update_req->bcch_inf.c_bcch > 0)
    {
      error = FKT_Select (SIM_BCCH, FALSE, NULL, NULL, 0);
      if (error EQ SIM_NO_ERROR)
        error = FKT_UpdateBinary (sim_mm_update_req->bcch_inf.bcch,
                                  (USHORT)sim_mm_update_req->bcch_inf.c_bcch, 0);
      if (error NEQ SIM_NO_ERROR)
        all_upd = FALSE;
    }
  }

  /*
   * check forbidden PLMN
   */
  if(sim_mm_update_req->ef_indicator & 0x04)
  {
    if (sim_mm_update_req->forb_plmn.c_forb > 0)
    {
      error = FKT_Select (SIM_FPLMN, FALSE, NULL, NULL, 0);
      if (error EQ SIM_NO_ERROR)
        error = FKT_UpdateBinary (sim_mm_update_req->forb_plmn.forb,
                                  (USHORT)sim_mm_update_req->forb_plmn.c_forb, 0);
      if (error NEQ SIM_NO_ERROR)
        all_upd = FALSE;
    }
  }

  /*
   * check kc and cksn
   */
  if(sim_mm_update_req->ef_indicator & 0x08)
  {
    error = FKT_Select (SIM_KC, FALSE, NULL, NULL, 0);
    if (error EQ SIM_NO_ERROR)
    {
      kc_n.c_kc = 9;
      kc_n.kc[8] = sim_mm_update_req->cksn;
      for (i = 0; i < 8; i++)
        kc_n.kc[7-i] = sim_mm_update_req->kc[i];
      error = FKT_UpdateBinary (kc_n.kc, (USHORT)kc_n.c_kc, 0);
    }
    if (error NEQ SIM_NO_ERROR)
      all_upd = FALSE;
  }

  SIM_EM_PARAMETER_UPDATE;

  PFREE (sim_mm_update_req);

  if (all_upd)
    app_start_status_timer (FALSE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_sync_req           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_SYNC_REQ.

*/

GLOBAL void app_sim_sync_req (T_SIM_SYNC_REQ * sim_sync_req)
{
  #ifdef TI_PS_UICC_CHIPSET_15
  U8 reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
  #endif
  PALLOC (sim_sync_cnf, SIM_SYNC_CNF);
  sim_sync_cnf->cause = SIM_NO_ERROR;

  TRACE_FUNCTION ("app_sim_sync_req()");

  SIM_EM_PARAMETER_SYNCHRONISATION;

  switch (sim_sync_req->synccs)
  {
    case SYNC_START_CALL:
      SIM_SET_FLAG (CALL_ACTIVE);
      PSENDX (MMI, sim_sync_cnf);
      app_sim_timeout (SIM_TIMER);               /* checks SIM status!*/
      app_start_status_timer (TRUE);    /* restart SIM presence detection*/
      break;

    case SYNC_STOP_CALL:
      SIM_CLEAR_FLAG (CALL_ACTIVE);
      /*
       * only if SIM inserted
       */
      if (SIM_IS_FLAG_SET (SIM_INSERT))
      {
#if defined SIM_TOOLKIT
        /*
         * if SIM Toolkit is active, idle polling
         * might be needed, additionally
         */
        if (SIM_IS_FLAG_CLEARED (PRO_ACTIVE_SIM))
#endif
        {
        /*
         * Stop presence detection polling, after Call
         */
          TIMER_STOP (sim_handle, SIM_TIMER);
        }
#if defined SIM_TOOLKIT
        else if (sim_data.idle_polling)
        {
          app_start_status_timer (TRUE);
        }
#endif
      }
      else
        sim_sync_cnf->cause = SIM_CAUSE_CARD_REMOVED;

      PSENDX (MMI, sim_sync_cnf);
      break;


    case SYNC_MM_FINISHED_READING:
#if defined SIM_TOOLKIT
      if(sim_data.sync_awaited & SIM_SYNC_AWAIT_MM_READ)
      {
        sim_data.sync_awaited &= ~SIM_SYNC_AWAIT_MM_READ;
        /* Check if both MM and MMI have sent SYNC_REQ */
        if(sim_data.sync_awaited EQ 0 AND sim_data.stk_resp_len NEQ 0)
        {
          FKT_TerminalResponse (sim_data.stk_response, (USHORT)sim_data.stk_resp_len);
          sim_data.stk_resp_len = 0;
        }
      }
#endif /* SIM_TOOLKIT */
      PSENDX (MM, sim_sync_cnf);
      break;

    case SYNC_MMI_FINISHED_READING:
#if defined SIM_TOOLKIT
      if(sim_data.sync_awaited & SIM_SYNC_AWAIT_MMI_READ)
      {
        sim_data.sync_awaited &= ~SIM_SYNC_AWAIT_MMI_READ;
        /* Check if both MM and MMI have sent SYNC_REQ */
        if(sim_data.sync_awaited EQ 0 AND sim_data.stk_resp_len NEQ 0)
        {
          FKT_TerminalResponse (sim_data.stk_response, (USHORT)sim_data.stk_resp_len);
          sim_data.stk_resp_len = 0;
        }
      }
#endif /* SIM_TOOLKIT */
      PSENDX (MMI, sim_sync_cnf);
      break;

    case SYNC_DEACTIVATE:
      TIMER_STOP (sim_handle, SIM_TIMER);
#ifdef SIM_TOOLKIT
      stk_stop_all_sat_timers ();
#endif
      PSENDX (MMI, sim_sync_cnf);
      /*
       * switch off SIM driver
       */
#ifndef TI_PS_UICC_CHIPSET_15
      SIM_PowerOff ();
#else
      simdrv_poweroff( reader_id );
#endif
      SIM_CLEAR_FLAG (SIM_INSERT);
      /*
       * Initialize SIM for next Power On
       */
      app_init_sim_data ();
      break;

    default:
      sim_sync_cnf->cause = SIM_CAUSE_PARAM_WRONG;
      PSENDX (MMI, sim_sync_cnf);
      break;
  }
  PFREE (sim_sync_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_activate_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_ACTIVATE_REQ.

*/

GLOBAL void app_sim_activate_req (T_SIM_ACTIVATE_REQ * sim_activate_req)
{
  USHORT     retcode;

#ifdef TI_PS_UICC_CHIPSET_15
  U8 reader_id =SIMDRV_VAL_READER_ID__RANGE_MIN;
  U8 voltage_select = SIMDRV_REQ_VOLTAGE_SEL;
#endif  /* TI_PS_UICC_CHIPSET_15 */
#if defined  FF_DUAL_SIM OR (!defined TI_PS_UICC_CHIPSET_15)
  T_SIM_CARD sim_info;
#endif  /* FF_DUAL_SIM  OR TI_PS_UICC_CHIPSET_15*/

  TRACE_FUNCTION ("app_sim_activate_req()");

  switch (sim_activate_req->proc)
  {
    case SIM_INITIALISATION:
      SIM_SET_FLAG (ACTIVATION_STARTED);
      SIM_CLEAR_FLAG (CALL_ACTIVE);
      sim_data.act_directory = NOT_PRESENT_16BIT;
      sim_data.act_field     = NOT_PRESENT_16BIT;
      sim_data.status_time   = THIRTY_SECONDS;
//#ifdef SIM_TOOLKIT	// FreeCalypso failed attempt to unbreak !SIM_TOOLKIT
      sim_data.cust_mode     = sim_activate_req->cust_mode;
//#endif

      if (sim_activate_req->mmi_pro_file & SIM_MMI_BDN)
        SIM_SET_FLAG (BDN_SUPPORT_BY_MMI);
      else
        SIM_CLEAR_FLAG (BDN_SUPPORT_BY_MMI);

      if (sim_activate_req->mmi_pro_file & SIM_MMI_FDN)
        SIM_SET_FLAG (FDN_SUPPORT_BY_MMI);
      else
        SIM_CLEAR_FLAG (FDN_SUPPORT_BY_MMI);

      SIM_CLEAR_FLAG (CC_WITH_STK);

#if defined SIM_TOOLKIT
      stk_check_tp (sim_data.stk_profile, sim_activate_req->stk_pro_file,
                    MINIMUM(sizeof sim_data.stk_profile, sizeof sim_activate_req->stk_pro_file));
      if (sim_data.stk_profile[1] & SAT_TP2_CC)
        SIM_SET_FLAG (CC_WITH_STK);

      if(sim_activate_req->v_trmst_file)
      {
        memcpy (sim_data.trmst, sim_activate_req->trmst_file, sizeof(sim_activate_req->trmst_file));
      }
      else
      {
        memset (sim_data.trmst, FALSE, MAX_TRMST);
      }
#endif

      TIMER_START (sim_handle, SIM_TIMER, T_DETECT_VALUE);
  #ifndef TI_PS_UICC_CHIPSET_15
      retcode = SIM_Reset (&sim_info);   
  #else
      retcode = simdrv_reset( reader_id, voltage_select);
  #endif

      TRACE_EVENT_P1 ("Result SIM Reset = %d", (int)retcode);

      switch (retcode)
      {
        case 0:
          /*
           * No problem
           */
          break;
        case 1:
        case 3:
        case 8:
        case 9:
          /*
           * No card inserted
           */
          TIMER_STOP (sim_handle, SIM_TIMER);

          app_sim_card_error (SIM_CAUSE_CARD_REMOVED);
          SIM_CLEAR_FLAG (SIM_INSERT);
          SIM_CLEAR_FLAG (ACTIVATION_STARTED);
          break;
        default:
          /*
           * other card problems
           */
          TIMER_STOP (sim_handle, SIM_TIMER);
          app_sim_card_error (CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS,
                                         SIM_ORIGINATING_ENTITY, retcode));
          SIM_CLEAR_FLAG (SIM_INSERT);
          SIM_CLEAR_FLAG (ACTIVATION_STARTED);
          break;
      }

      break;

    case SIM_FDN_ENABLE:
       /* Implements Measure# 14 */
       app_sim_activate_req_fdn_enable(TRUE); 
      break;
    
    case SIM_FDN_DISABLE:
       /* Implements Measure# 14 */
       app_sim_activate_req_fdn_enable(FALSE); 
      break;

#ifdef FF_DUAL_SIM
    case SIM_SELECT:

      if(sim_activate_req->sim_num < 0 OR sim_activate_req->sim_num > 2)
      {
        app_sim_card_error (SIM_CAUSE_PARAM_WRONG);
        break;
      }
 
      sim_data.SIM_Selection = TRUE;

      if(SIM_IS_FLAG_SET (SIM_INSERT))
      {
        SIM_PowerOff ();
        app_init_sim_data ();
        SIM_CLEAR_FLAG (SIM_INSERT);
      }
      retcode = SIM_SwitchDualSIM(sim_activate_req->sim_num);
    
      if(!retcode)
      {
        sim_data.sim_num = sim_activate_req->sim_num;

        TIMER_START (sim_handle, SIM_TIMER, T_DETECT_VALUE);

        retcode = SIM_Reset (&sim_info);
        TRACE_EVENT_P1 ("Result SIM Reset = %d", (int)retcode);

        switch (retcode)
        {
          case 0:
            /*
             * No problem
             */
            break;
          case 1:
          case 3:
          case 8:
          case 9:
            /*
             * No card inserted
             */
            TIMER_STOP (sim_handle, SIM_TIMER);

            app_sim_card_error (SIM_CAUSE_CARD_REMOVED);
            SIM_CLEAR_FLAG (SIM_INSERT);
            break;
          default:
            /*
             * other card problems
             */
            TIMER_STOP (sim_handle, SIM_TIMER);

            app_sim_card_error (CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS,
                                       SIM_ORIGINATING_ENTITY, retcode));
            SIM_CLEAR_FLAG (SIM_INSERT);
            break;
        }
      }
      else
      {
        app_sim_card_error (CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS,
                                         SIM_ORIGINATING_ENTITY, retcode));
      }
      sim_data.SIM_Selection = FALSE;
      break;
#endif /*FF_DUAL_SIM*/

    default:
      app_sim_card_error (SIM_CAUSE_PARAM_WRONG);  /* wrong parameter */

      SIM_EM_SIM_ACTIVATION_STARTED;

      break;
  }

  PFREE (sim_activate_req);

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_check_service      |
+--------------------------------------------------------------------+

  PURPOSE : Checks a service status.

*/

GLOBAL UBYTE app_sim_check_service (UBYTE nr, UBYTE * serv_table)
{
  UBYTE value;

  TRACE_FUNCTION ("app_check_sim_service()");

  serv_table = serv_table + (nr-1)/4;
  value = * serv_table;

  value = value >> (((nr-1) & 3) * 2);
  value = value & 3;

  return value;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_fdn_bdn_procedures     |
+--------------------------------------------------------------------+

  PURPOSE : Processing of the FDN/BDN procedures according annex B
            of GSM 11.11.

*/

static const UBYTE op [9] =
{
    SIM_ADN_ENABLED,             /* no BDN, no FDN             */
    SIM_ADN_BDN_ENABLED,         /* BDN enabled, no FDN        */
    SIM_ADN_ENABLED,             /* BDN disabled, no FND       */
    SIM_FDN_ENABLED,             /* no BDN, FDN enabled        */
    SIM_FDN_BDN_ENABLED,         /* BDN enabled, FDN enabled   */
    SIM_FDN_ENABLED,             /* BDN disabled, FND enabled  */
    SIM_ADN_ENABLED,             /* no BDN, FDN disabled       */
    SIM_ADN_BDN_ENABLED,         /* BDN enabled, FDN disabled  */
    SIM_ADN_ENABLED              /* BDN disabled, FND disabled */
};

GLOBAL UBYTE app_fdn_bdn_procedures (T_SIM_MMI_INSERT_IND *sim_mmi_insert_ind)
{
  UBYTE bdn_capability;
  UBYTE fdn_capability;
  UBYTE check_imsi_loci;

  TRACE_FUNCTION ("app_fdn_bdn_procedures()");

  /*
   * IMSI and Location Information are
   * both not invalidated
   */
  check_imsi_loci = app_check_imsi_loci_validation ();

  if (!app_read_sim_service_table(sim_mmi_insert_ind))
    return SIM_NO_OPERATION;

  if (check_imsi_loci)
    return SIM_ADN_ENABLED;
  
  bdn_capability = app_bdn_capability_request ();
  fdn_capability = app_fdn_capability_request ();
  
  if (SIM_IS_FLAG_SET (CC_WITH_STK))
  {
    /*
     * ME supports Call control with SIM Toolkit
     */

    /*
     * if mobile has no BDN capability, but the SIM card
     */
    if (bdn_capability EQ BDN_ENABLED AND
        SIM_IS_FLAG_CLEARED (BDN_SUPPORT_BY_MMI))
       return SIM_NO_OPERATION;
    /*
     * if mobile has no FDN capability, but the SIM card
     */
    if (fdn_capability EQ FDN_ENABLED AND
        SIM_IS_FLAG_CLEARED (FDN_SUPPORT_BY_MMI))
      return SIM_NO_OPERATION;

    /*
     * Try rehabilitation of IMSI and Location Information
     */

    if (app_rehabilitate_imsi_loci ())
      return op [bdn_capability + 3 * fdn_capability];
    else
      return SIM_NO_OPERATION;
  }
  else
  {
    /*
     * ME doesn't support Call control with SIM Toolkit
     */

    /*
     * if mobile has no FDN capability or FDN is not enabled on the SIM card
     */
    if (fdn_capability NEQ FDN_ENABLED OR
        SIM_IS_FLAG_CLEARED (FDN_SUPPORT_BY_MMI))
      return SIM_NO_OPERATION;

    /*
     * Try rehabilitation of IMSI and Location Information
     */
    if (app_rehabilitate_imsi_loci ())
      return SIM_FDN_ENABLED;
    else
      return SIM_NO_OPERATION;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)   MODULE  : SIM_APP                        |
| STATE   : code            ROUTINE : app_check_imsi_loci_validation |
+--------------------------------------------------------------------+

  PURPOSE : Checks the validation flag of IMSI and Location information.

*/

GLOBAL UBYTE app_check_imsi_loci_validation (void)
{
  UBYTE response[SIMDRV_MAX_RESULT];
  T_FIELD_STATUS field_status;

  TRACE_FUNCTION ("app_check_imsi_loci_validation()");

  
  /* Currently selected EF information is reset to force the selection */
  sim_data.act_field = NOT_PRESENT_16BIT;

  /*
   * check IMSI
   */
  if (FKT_Select (SIM_LOCI, FALSE, NULL,response, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)
    memcpy(&field_status,response,SIM_MIN_EF_ST_LEN);
  else
    return FALSE;

  /*
   * check invalidation flag
   */
  if ((field_status.file_status & 1) EQ 0)
    return FALSE;

  /*
   * check Location Information
   */
  if (FKT_Select (SIM_IMSI, FALSE, NULL,response, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)
    memcpy(&field_status,response,SIM_MIN_EF_ST_LEN);
  else
    return FALSE;

  /*
   * check invalidation flag
   */
  if ((field_status.file_status & 1) EQ 0)
    return FALSE;

  /*
   * both fields are not invalidated
   */
  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)   MODULE  : SIM_APP                        |
| STATE   : code            ROUTINE : app_bdn_capability_request     |
+--------------------------------------------------------------------+

  PURPOSE : Checks the BDN capability of the SIM card.

*/

GLOBAL UBYTE app_bdn_capability_request (void)
{
  T_FIELD_STATUS field_status;
  UBYTE response[SIMDRV_MAX_RESULT];

  TRACE_FUNCTION ("app_bdn_capability_request()");

  /*
   * check service in sim service table
   */
  if (SIM_IS_FLAG_CLEARED (SERVICE_31_SUPPORT))
    return NO_BDN_SIM;

  /*
   * Select BDN field and check invalidation flag
   */
  /* Currently selected EF information is reset to force the selection */
  sim_data.act_field = NOT_PRESENT_16BIT;

  if (FKT_Select (SIM_BDN, FALSE, NULL, response, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)

    memcpy(&field_status, response, SIM_MIN_EF_ST_LEN);

  else
    return NO_BDN_SIM;

  if ((field_status.file_status & 1) EQ 0)
    return BDN_DISABLED;
  else
    return BDN_ENABLED;
   
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)   MODULE  : SIM_APP                        |
| STATE   : code            ROUTINE : app_fdn_capability_request     |
+--------------------------------------------------------------------+

  PURPOSE : Checks the FDN capability of the SIM card.

*/

GLOBAL UBYTE app_fdn_capability_request (void)
{
  T_FIELD_STATUS field_status;
  UBYTE response[SIMDRV_MAX_RESULT];
  
  TRACE_FUNCTION ("app_fdn_capability_request()");

  /*
   * check service in sim service table
   */
  if (SIM_IS_FLAG_CLEARED (SERVICE_3_SUPPORT))
    return NO_FDN_SIM;

  /*
   * FDN is allocated and activated. Check against ADN.
   * Only ADN or FDN is possible.
   */
  if (SIM_IS_FLAG_CLEARED (SERVICE_2_SUPPORT))
    return FDN_ENABLED;

  /*
   * Select ADN field and check invalidation flag
   */
 /* Currently selected EF information is reset to force the selection */
  sim_data.act_field = NOT_PRESENT_16BIT;

  if (FKT_Select (SIM_ADN, FALSE, NULL, response, SIM_MIN_EF_ST_LEN) EQ SIM_NO_ERROR)

    memcpy(&field_status, response, SIM_MIN_EF_ST_LEN);

  else
    return FDN_ENABLED;

  if ((field_status.file_status & 1) EQ 0)
    return FDN_ENABLED;
  else
    return FDN_DISABLED;
}
  

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)   MODULE  : SIM_APP                        |
| STATE   : code            ROUTINE : app_rehabilitate_imsi_loci     |
+--------------------------------------------------------------------+

  PURPOSE : Tries to rehabilitate IMSI and location information.

*/

GLOBAL UBYTE app_rehabilitate_imsi_loci (void)
{
  USHORT error;
  TRACE_FUNCTION ("app_rehabilitate_imsi_loci()");

  /*
   * rehabilitate IMSI
   */
  error = FKT_Select (SIM_LOCI, FALSE, NULL, NULL, 0);
  if (error EQ SIM_NO_ERROR OR error EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)

  {
    if (FKT_Rehabilitate () NEQ SIM_NO_ERROR)
      return FALSE;
  }
  else
    return FALSE;

  /*
   * rehabilitate Location Information
   */
  if (FKT_Select (SIM_IMSI, FALSE, NULL, NULL, 0) EQ SIM_NO_ERROR)
  {
    if (FKT_Rehabilitate () NEQ SIM_NO_ERROR)
      return FALSE;
  }
  else
    return FALSE;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)   MODULE  : SIM_APP                        |
| STATE   : code            ROUTINE : app_start_status_timer         |
+--------------------------------------------------------------------+

  PURPOSE : (Re-)Start the timer for periodical status requests to the
            SIM card. If parameter 'condx' is set TRUE, then the timer
            is (re-)started regardless of the SIM being a test SIM, or
            not. Also entry point for SIM toolkit commands.
*/

GLOBAL void app_start_status_timer (BOOL condx)
{
  T_TIME t_val;
  TRACE_FUNCTION ("app_start_status_timer()");
  /*
   * start status timer if call is active and SIM is inserted
   * for periodic status polling of SIM toolkit polling
   */
  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    if (SIM_IS_FLAG_SET (CALL_ACTIVE)
#if defined SIM_TOOLKIT
         OR (SIM_IS_FLAG_SET (PRO_ACTIVE_SIM) AND
         (SIM_IS_FLAG_CLEARED (TEST_MODE_POLLING) OR condx))
#endif
#ifdef TI_PS_FF_SIM_POLL_ALWAYS	 
         OR SIM_IS_FLAG_CLEARED (PRO_ACTIVE_SIM)
#endif	 
       )
    { /* when idle polling is enabled, use that timer value (as long as 
       * it's not more than 30s); else, we're only SIM presence 
       * detecting, during the call, with an interval of those 30s.
       * This will be disabled, at the end of the call, again.
       */
#if defined SIM_TOOLKIT
      if (sim_data.idle_polling)
      {
        t_val = (SIM_IS_FLAG_SET (CALL_ACTIVE) AND
               sim_data.status_time > THIRTY_SECONDS)?
              THIRTY_SECONDS: sim_data.status_time;
      }
      else
#endif
      {
        t_val = THIRTY_SECONDS;
      }
      /*
       * Start Status Polling during Call
       */
      TIMER_PSTART (sim_handle, SIM_TIMER, t_val, t_val);
    }
#if defined SIM_TOOLKIT
    sim_data.chk_sat_avail = TRUE;
    /* stk_proactive_polling();*/
#endif
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)   MODULE  : SIM_APP                        |
| STATE   : code            ROUTINE : app_check_access_conditions    |
+--------------------------------------------------------------------+

  PURPOSE : The function checks the access conditions. It sets the
            last requested pin number derived from the field status
            of the selected file. It returns whether access is
            possible or not.
*/

GLOBAL BOOL app_check_access_conditions (UBYTE proc, T_FIELD_STATUS *field_status)
{
  UBYTE  access;

  TRACE_FUNCTION ("app_check_access_conditions()");

  switch (proc)
  {
    case ACCESS_READ:
      access = field_status->access_1 >> 4;
      break;
    case ACCESS_UPDATE:
      access = field_status->access_1 & 0x0F;
      break;
    case ACCESS_INCREASE:
      access = field_status->access_2 >> 4;
      break;
    case ACCESS_REHABILITATE:
      access = field_status->access_3 >> 4;
      break;
    case ACCESS_INVALIDATE:
      access = field_status->access_3 & 0x0F;
      break;
    default:
      sim_data.last_requested_pin_no = LRP_NEVER;
      return FALSE;
  }

  switch (access)
  {
    case ALWAYS:
    case PIN_1:
    case PIN_2:
      sim_data.last_requested_pin_no = access;
      return TRUE;
    default:      /* ADM or NEVER */
      sim_data.last_requested_pin_no = LRP_NEVER;
      return FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_APP                    |
| STATE   : code                ROUTINE : app_sim_access_req         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_ACCESS_REQ.

*/

GLOBAL void app_sim_access_req (T_SIM_ACCESS_REQ * sim_access_req_org)
{
  USHORT result;
  USHORT offset;
  USHORT size = 0,length; 
  USHORT rcvLen = 0;

  #ifdef TI_PS_UICC_CHIPSET_15
  T_SIMDRV_cmd_header cmd_header;
  U8 reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
  T_SIMDRV_data_info  data_info;
  T_SIMDRV_result_info    result_info;
  U8 offset_high;
  U8 offset_low;
  #endif

  static UBYTE get_resp[SIM_TPDU_HEADER_LEN] = {0x00, 0xC0, 0x00, 0x00, 0x00};
  T_SIM_ACCESS_CNF *sim_access_cnf;
  PPASS (sim_access_req_org, sim_access_req, SIM_ACCESS_REQ);

  sim_access_cnf = P_ALLOC (SIM_ACCESS_CNF);

  TRACE_FUNCTION ("app_sim_access_req()");

  /*
   * initialize answer
   */
  sim_access_cnf->req_id = sim_access_req->req_id;
  sim_access_cnf->c_trans_data = 0;
  sim_access_cnf->sw1 = sim_access_cnf->sw2 = 0;
  memset (sim_access_cnf->trans_data, 0, sizeof (sim_access_cnf->trans_data));
  sim_data.sw1 = sim_data.sw2 = 0; /* delete previous result code */

  if (SIM_IS_FLAG_SET (SIM_INSERT))
  {
    /*
     * if SIM is inserted, try to select the SIM card.
     */
    switch (sim_access_req->sim_command)
    {
    case SIM_GET_RESPONSE:
      sim_data.act_directory = NOT_PRESENT_16BIT;
      sim_data.act_field = NOT_PRESENT_16BIT;
    case SIM_READ_BINARY:
    case SIM_READ_RECORD:
    case SIM_UPDATE_BINARY:
    case SIM_UPDATE_RECORD:
      length = (sim_access_req->p3 > 0)?
      sim_access_req->p3: NOT_PRESENT_16BIT; 
      /* Access request coming from AT Command is only for standard files. Hence 
         path info can be set as NULL */
      sim_access_cnf->cause = FKT_Select (sim_access_req->datafield, FALSE, NULL,sim_access_cnf->trans_data, length);
      if(sim_access_cnf->cause EQ SIM_NO_ERR_FILE_ALREADY_SELECTED)
        sim_access_cnf->cause = SIM_NO_ERROR;
      sim_access_cnf->c_trans_data = (USHORT)sim_data.sim_data_len - SIM_TI_DRV_X_BYTES;
      sim_access_cnf->sw1 = (UBYTE)(sim_access_cnf->cause >> 8);
      sim_access_cnf->sw2 = (UBYTE)sim_access_cnf->cause;
      break;
    case SIM_TRANSP_CMD:
      if (sim_access_req->c_trans_data < 4 OR sim_access_req->c_trans_data > 261
           OR sim_access_req->trans_data[0] EQ GSM_CLASS)
      {
        sim_access_cnf->cause = SIM_CAUSE_PARAM_WRONG;
        break;
      }
      /* no break */
    case SIM_STATUS:
      sim_access_cnf->cause = SIM_NO_ERROR;
      break;
    default:
      sim_access_cnf->cause = SIM_CAUSE_PARAM_WRONG;
      break;
    }
    if (sim_access_cnf->cause EQ SIM_NO_ERROR)
    {
      /*
       * switch depending on SIM command
       */
      switch (sim_access_req->sim_command)
      {
      case SIM_READ_BINARY:
      /*
       * calculate offset from P1 and P2
       */
        offset = (sim_access_req->p1 << 8) + sim_access_req->p2;
      /*
       * call SIM driver
       */
       
       #ifndef TI_PS_UICC_CHIPSET_15
        result= SIM_ReadBinary (sim_access_cnf->trans_data,
                                offset,
                                (USHORT)sim_access_req->p3,
                                &size);
       #else /* !TI_PS_UICC_CHIPSET_15 */
        offset_high = (U8)((offset &0x7F00)>>8); /* to make the 8th bit 0 as per 102.221 */ 
        offset_low = (U8)(offset & 0x00FF); 
        reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
        cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
        cmd_header.ins = SIMDRV_INS_READ_BINARY;
        cmd_header.p1 =offset_high ;
        cmd_header.p2 =offset_low ;
        data_info.data   = NULL;
        data_info.c_data = 0;
        result_info.result  = (U8 *)sim_access_cnf->trans_data;
        result_info.c_result = size;
        result_info.len  = (USHORT)sim_access_req->p3;
    
        result= simdrv_xch_apdu (reader_id,cmd_header,data_info,&result_info);
        size = result_info.c_result; /* c_result will be updated by SIMDRV */
       #endif  /* !TI_PS_UICC_CHIPSET_15 */

      /*
       * fill response primitive
       */
        sim_access_cnf->c_trans_data = (USHORT)size - SIM_TI_DRV_X_BYTES;
        sim_access_cnf->sw1 = (UBYTE)(result >> 8);
        sim_access_cnf->sw2 = (UBYTE)result;
        break;

      case SIM_READ_RECORD:
      /*
       * call SIM driver
       */
       #ifndef TI_PS_UICC_CHIPSET_15
        result = SIM_ReadRecord (sim_access_cnf->trans_data,
                                 sim_access_req->p2,
                                 sim_access_req->p1,
                                 (USHORT)sim_access_req->p3,
                                 &size);
       #else /* !TI_PS_UICC_CHIPSET_15 */
        reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
        cmd_header.cla = SIMDRV_GSM_CLASS_BYTE ;
        cmd_header.ins = SIMDRV_INS_READ_RECORD;
        cmd_header.p1 = (U8)sim_access_req->p1;
        cmd_header.p2 = (U8)sim_access_req->p2;
        data_info.data   = NULL;
        data_info.c_data = 0;
        result_info.result   = (U8 *)sim_access_cnf->trans_data ;
        result_info.c_result = size;
        result_info.len  =(USHORT)sim_access_req->p3;
  
        result= simdrv_xch_apdu (reader_id,cmd_header,data_info, &result_info);
        size = result_info.c_result; /* c_result will be updated by SIMDRV */
      #endif  /* !TI_PS_UICC_CHIPSET_15 */
      /*
       * fill response primitive
       */
        sim_access_cnf->c_trans_data = (USHORT)size - SIM_TI_DRV_X_BYTES;
        sim_access_cnf->sw1 = (UBYTE)(result >> 8);
        sim_access_cnf->sw2 = (UBYTE)result;
        break;

       case SIM_GET_RESPONSE:
      /*
       * call SIM driver
       */
       #ifndef TI_PS_UICC_CHIPSET_15
        length = (sim_access_req->p3 > 0)?
                  sim_access_req->p3: sim_data.sim_data_len;
    
        result = SIM_GetResponse (sim_access_cnf->trans_data,
                                  length,
                                  &size);
        sim_access_cnf->c_trans_data = (USHORT)size - SIM_TI_DRV_X_BYTES;
        sim_access_cnf->sw1 = (UBYTE)(result >> 8);
        sim_access_cnf->sw2 = (UBYTE)result;
       #endif
        break; 

      case SIM_UPDATE_BINARY:
      /*
       * calculate offset from P1 and P2
       */
        offset = (sim_access_req->p1 << 8) + sim_access_req->p2;
      /*
       * call SIM driver
       */
       #ifndef TI_PS_UICC_CHIPSET_15
        result= SIM_UpdateBinary (sim_access_cnf->trans_data,
                                  sim_access_req->trans_data,
                                  offset,
                                  (USHORT)sim_access_req->p3,
                                  &size);
   
       #else /* !TI_PS_UICC_CHIPSET_15 */
        offset_high = (U8)((offset &0x7F00)>>8);/*to make the 8th bit 0 as per 102.221*/
        offset_low = (U8)(offset & 0x00FF);
        reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
        cmd_header.cla = SIMDRV_GSM_CLASS_BYTE ;
        cmd_header.ins = SIMDRV_INS_UPDATE_BINARY;
        cmd_header.p1 = offset_high;
        cmd_header.p2 = offset_low;
        data_info.data  = (U8 *)sim_access_req->trans_data;
        data_info.c_data = (U8)sim_access_req->p3;
        result_info.result = sim_access_cnf->trans_data;
        result_info.c_result = size;
        result_info.len  = NOT_PRESENT_16BIT;
   
        result= simdrv_xch_apdu (reader_id,cmd_header,data_info,&result_info);
        size = result_info.c_result; /* c_result will be updated by SIMDRV */
       #endif  /* !TI_PS_UICC_CHIPSET_15 */
      /*
       * fill response primitive
       */
        sim_access_cnf->sw1 = (UBYTE)(result >> 8);
        sim_access_cnf->sw2 = (UBYTE)result;
        break;

      case SIM_UPDATE_RECORD:
      /*
       * call SIM driver
       */
       #ifndef TI_PS_UICC_CHIPSET_15
        result = SIM_UpdateRecord (sim_access_cnf->trans_data,
                                   sim_access_req->trans_data,
                                   sim_access_req->p2,
                                   sim_access_req->p1,
                                   (USHORT)sim_access_req->p3,
                                   &size);
  
       #else /* !TI_PS_UICC_CHIPSET_15 */
        reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
        cmd_header.cla = SIMDRV_GSM_CLASS_BYTE ;
        cmd_header.ins = SIMDRV_INS_UPDATE_RECORD;
        cmd_header.p1 = (U8)sim_access_req->p1;
        cmd_header.p2 = (U8)sim_access_req->p2;
        data_info.data   = (U8*)sim_access_req->trans_data;
        data_info.c_data = (U8)sim_access_req->p3;
        result_info.result = sim_access_cnf->trans_data;
        result_info.c_result = size;
        result_info.len  = NOT_PRESENT_16BIT;
  
        result= simdrv_xch_apdu (reader_id,cmd_header,data_info,&result_info);
        size = result_info.c_result; /* c_result will be updated by SIMDRV */
       #endif  /* !TI_PS_UICC_CHIPSET_15 */
      /*
       * fill response primitive
       */
        sim_access_cnf->sw1 = (UBYTE)(result >> 8);
        sim_access_cnf->sw2 = (UBYTE)result;
        break;

      case SIM_STATUS:
      /*
       * call SIM driver
       */
        length = (sim_access_req->p3 > 0)?
                 (USHORT)sim_access_req->p3: sim_data.dir_status_len;
    
      #ifndef TI_PS_UICC_CHIPSET_15
        result = SIM_Status_Extended (sim_access_cnf->trans_data,
                                      length,
                                      &size);
      #else /* !TI_PS_UICC_CHIPSET_15 */
        reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;

        cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
        cmd_header.ins = SIMDRV_INS_STATUS;
        cmd_header.p1 = 0;
        cmd_header.p2 = 0;

        data_info.data   = NULL;
        data_info.c_data = 0;
    
        result_info.result = (U8 *)sim_access_cnf->trans_data;
        result_info.len = length;
        result_info.c_result = size;

        result =  simdrv_xch_apdu(reader_id, cmd_header, data_info, &result_info);
        size = result_info.c_result; /* c_result will be updated by SIMDRV */
      #endif /* !TI_PS_UICC_CHIPSET_15 */

        sim_access_cnf->c_trans_data = (USHORT)size - SIM_TI_DRV_X_BYTES;
        sim_access_cnf->sw1 = (UBYTE)(result >> 8);
        sim_access_cnf->sw2 = (UBYTE)result;
        break;

      case SIM_TRANSP_CMD:
      /*
       * transparent SIM access
       */
       #ifdef  TI_PS_UICC_CHIPSET_15 
	  #ifdef  _SIMULATION_ 
             sim_command_type = SIM_TRANSP_CMD;
	  #endif
       #endif
        if (sim_access_req->c_trans_data EQ 4)
        {
          sim_access_req->trans_data[4] = '\0';
          sim_access_req->c_trans_data = 5;

          #ifndef TI_PS_UICC_CHIPSET_15
            result = SIM_XchTPDU (sim_access_req->trans_data,
                                sim_access_req->c_trans_data,
                                sim_access_cnf->trans_data,
                                0, &sim_access_cnf->c_trans_data);
         #else /* !TI_PS_UICC_CHIPSET_15 */
           cmd_header.cla = sim_access_req->trans_data[0];
           cmd_header.ins = sim_access_req->trans_data[1];
           cmd_header.p1 = sim_access_req->trans_data[2];
           cmd_header.p2 = sim_access_req->trans_data[3];

           data_info.data = &sim_access_req->trans_data[5];
           data_info.c_data = (U8)sim_access_req->c_trans_data;

           result_info.result = sim_access_cnf->trans_data;
           result_info.len = (USHORT)sim_access_req->trans_data[4];
           result_info.c_result = sim_access_cnf->c_trans_data;

           result = simdrv_xch_apdu (reader_id, cmd_header, data_info, &result_info);
           sim_access_cnf->c_trans_data = result_info.c_result;
         #endif   /* !TI_PS_UICC_CHIPSET_15 */

        }
        else if (sim_access_req->c_trans_data EQ 5)
        {
          rcvLen = (USHORT)sim_access_req->trans_data[4];
          if (rcvLen EQ 0)
            rcvLen = 256;
         #ifndef TI_PS_UICC_CHIPSET_15
          result = SIM_XchTPDU (sim_access_req->trans_data,
                                sim_access_req->c_trans_data,
                                sim_access_cnf->trans_data,
                                rcvLen, &sim_access_cnf->c_trans_data);
         #else  /* !TI_PS_UICC_CHIPSET_15 */
           cmd_header.cla = sim_access_req->trans_data[0];
           cmd_header.ins = sim_access_req->trans_data[1];
           cmd_header.p1 = sim_access_req->trans_data[2];
           cmd_header.p2 = sim_access_req->trans_data[3];

           data_info.data = &sim_access_req->trans_data[5];
           data_info.c_data = (U8)sim_access_req->c_trans_data;

           result_info.result = sim_access_cnf->trans_data;
           result_info.len = (USHORT)sim_access_req->trans_data[4];
           result_info.c_result = sim_access_cnf->c_trans_data;

           result = simdrv_xch_apdu (reader_id, cmd_header, data_info, &result_info);
           sim_access_cnf->c_trans_data = result_info.c_result;
         #endif  /* !TI_PS_UICC_CHIPSET_15 */
        }
        else
        {
          if (sim_access_req->c_trans_data EQ ((USHORT)sim_access_req->trans_data[4] + 6))
          {
            rcvLen = (USHORT)sim_access_req->trans_data[--sim_access_req->c_trans_data];
            if (rcvLen EQ 0)
              rcvLen = 256;
          }
          #ifndef TI_PS_UICC_CHIPSET_15
           result = SIM_XchTPDU (sim_access_req->trans_data,
                                sim_access_req->c_trans_data,
                                sim_access_cnf->trans_data,
                                0, &sim_access_cnf->c_trans_data);
          #else /* !TI_PS_UICC_CHIPSET_15 */
           cmd_header.cla = sim_access_req->trans_data[0];
           cmd_header.ins = sim_access_req->trans_data[1];
           cmd_header.p1 = sim_access_req->trans_data[2];
           cmd_header.p2 = sim_access_req->trans_data[3];

           data_info.data = &sim_access_req->trans_data[5];
           data_info.c_data = (U8)sim_access_req->c_trans_data;

           result_info.result = sim_access_cnf->trans_data;
           result_info.len = (USHORT)sim_access_req->trans_data[4];
           result_info.c_result = sim_access_cnf->c_trans_data;

           result = simdrv_xch_apdu (reader_id, cmd_header, data_info, &result_info);
           sim_access_cnf->c_trans_data = result_info.c_result;
          #endif /* !TI_PS_UICC_CHIPSET_15 */
          {
            USHORT sw1, sw2;

            sw1 = (result >> 8);
            sw2 = result & 0xFF;

            if (sw1 EQ 0x61)
            {
              if ((rcvLen <= 0) OR (sw2 < rcvLen))
                rcvLen = sw2;
            }
            else if (sw1 NEQ 0x90)
            {
              rcvLen = 0;
            }
            if (rcvLen > 0)
            {
              TRACE_EVENT_P2 ("SW1=%02X SW2=%02X", (int)sw1, (int)sw2);

              get_resp[0] = sim_access_req->trans_data[0];
              get_resp[4] = (UBYTE)rcvLen;

           #ifndef TI_PS_UICC_CHIPSET_15
              result = SIM_XchTPDU (get_resp, SIM_TPDU_HEADER_LEN,
                                    sim_access_cnf->trans_data,
                                    rcvLen,
                                    &sim_access_cnf->c_trans_data);
           #else /* !TI_PS_UICC_CHIPSET_15 */
             cmd_header.cla = get_resp[0];
             cmd_header.ins = get_resp[1];
             cmd_header.p1 = get_resp[2];
             cmd_header.p2 = get_resp[3];

             data_info.data = &get_resp[5];
             data_info.c_data = SIM_TPDU_HEADER_LEN;

             result_info.result = sim_access_cnf->trans_data;
             result_info.len = (USHORT)get_resp[4];
             result_info.c_result = sim_access_cnf->c_trans_data;

             result = simdrv_xch_apdu (reader_id, cmd_header, data_info, &result_info);
             sim_access_cnf->c_trans_data = result_info.c_result;
           #endif /* !TI_PS_UICC_CHIPSET_15 */

            }
          }
        }
        size = (sim_access_cnf->c_trans_data -= SIM_TI_DRV_X_BYTES);

        if ((result & 0xF000) EQ 0x6000 OR
            (result & 0xF000) EQ 0x9000)
        {
          sim_access_cnf->sw1 = (UBYTE)(result >> 8);
          sim_access_cnf->sw2 = (UBYTE)result;
        }
        else if ((result & 0xFF00) EQ 0)
        {
          sim_access_cnf->cause = CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS,
                                             SIM_ORIGINATING_ENTITY, result);
        }
        else
        {
          sim_access_cnf->cause = SIM_CAUSE_OTHER_ERROR;
        }
        break;

      default:
        sim_access_cnf->cause = SIM_CAUSE_PARAM_WRONG;
        break;
      }
      TRACE_EVENT_P3 ("SW1=%02X SW2=%02X SIZE=%d", sim_access_cnf->sw1,
                      sim_access_cnf->sw2, (int)size);
      /*
       * start status timer again
       */
      app_start_status_timer (FALSE);
    }
    else
    {
      sim_access_cnf->sw1 = sim_data.sw1;
      sim_access_cnf->sw2 = sim_data.sw2;
    }
  }
  else
  /*
   * sim is not inserted
   */
    sim_access_cnf->cause = SIM_CAUSE_CARD_REMOVED;

  /*
   * free incoming primitive
   */
  PFREE (sim_access_req);

  /*
   * send result to MMI
   */
  PSENDX (MMI, sim_access_cnf);
}

#endif
