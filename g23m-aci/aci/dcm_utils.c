/*
+-----------------------------------------------------------------------------
|  Project :  DCM and TCPIP
|  Modul   :  ACI
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
|  Description :  This file contains Useful DCM (Data connection manager)functions.
+-----------------------------------------------------------------------------
*/

/************************************  INCLUDES ********************************************/
#include "aci_all.h"

#include "aci.h"
#include "aci_cmh.h"
#include "Gaci_cmh.h"
#include "dcm.h"
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#include "dcm_f.h"
#include "psa_dcm.h"
#include "socket_api.h"

#include "wap_aci.h"

/**************************** LOCAL VARIABLE DEFINITION  ************************************/

/**************************** EXTERN VARIABLE DEFINITION  ***********************************/
EXTERN T_DCM_ENV_CTRL_BLK *dcm_env_ctrl_blk_p;


/**************************** LOCAL FUCNTION DEFINITION  ************************************/
LOCAL T_DCM_RET dcm_send_cgdcont_get_cmd(U8 row);
LOCAL T_DCM_RET dcm_send_cgdcont_cmd(U8 row);
LOCAL T_DCM_RET dcm_send_cgatt_cmd(U8 row);
LOCAL T_DCM_RET dcm_send_cgdeact_cmd(U8 row);
LOCAL T_DCM_RET dcm_send_cgpco_auth_cmd(U8 row);
LOCAL T_DCM_RET dcm_send_cgpco_get_cmd(U8 row);
LOCAL T_DCM_RET dcm_send_cgerep_cmd(U8 row);
LOCAL T_DCM_RET dcm_send_cgpaddr_cmd(U8 current_row);
LOCAL T_DCM_RET dcm_send_percentppp_cmd(U8 row);
LOCAL T_DCM_RET dcm_send_sat_dn_cmd(U8 row);
LOCAL T_DCM_RET dcm_send_percentppp_get_cmd(U8 row);
LOCAL T_DCM_RET dcm_send_percentcal_get_cmd(UBYTE row);
LOCAL T_DCM_RET dcm_send_sat_abort_cmd(U8 row);

LOCAL ULONG dcm_ipaddr_htonl(ULONG horder_ipaddr);

LOCAL void set_state_from_ctrl_blk();

/************************** EXTERN FUCNTION DEFINITION  *********************************/
EXTERN T_DCM_RET dcm_handle_message (T_DCM_HDR *msg_p);
EXTERN void psaTCPIP_Shutdown_Req(void);


BOOL is_netdrop = FALSE  ;  //pinghua added patch for DCM open/close

/***************************************************************************************
*   Function       :  dcm_process_unwaited_events_state_intermediate_conn
*   Parameter      :  T_DCM_HDR *
*                     -Pointer on the header of the message.
*   Return         :  T_DCM_RET
*                     DCM_OK or DCM errors
*   Description    :  Function used in all intermediate states where events is unwaited,
*                     but must be processed.
***************************************************************************************/
T_DCM_RET dcm_process_unwaited_events_state_intermediate_conn(T_DCM_HDR * msg_p)
{
  T_DCM_OPEN_CONN_REQ_MSG * dcm_open_conn_req_msg_p;
  T_DCM_CLOSE_CONN_REQ_MSG * dcm_close_conn_req_msg_p;

  TRACE_FUNCTION("DCM: dcm_process_unwaited_events_state_intermediate_conn()");

  switch(msg_p->msg_id)
  {
    case DCM_ERROR_IND_MSG:
      return dcm_process_event_error_reception(msg_p);
      /*lint -e527 suppress Warning -- Unreachable */
      /* break is removed ,as case is returning before break so it is not needed */
      /*lint +e527 */
    case DCM_OPEN_CONN_REQ_MSG:
      dcm_open_conn_req_msg_p = (T_DCM_OPEN_CONN_REQ_MSG*)msg_p;
      psaDCM_open_conn_cnf(DCM_BUSY,
                                 dcm_open_conn_req_msg_p->conn_req.api_instance);
      return (DCM_BUSY);  /* It is replaced with return in next line to avoid warning */	  
      /*lint -e527 suppress Warning -- Unreachable */
      /* break is removed ,as case is returning before break so it is not needed */	  
      /*lint +e527 */
    case DCM_CLOSE_CONN_REQ_MSG:
      dcm_close_conn_req_msg_p = (T_DCM_CLOSE_CONN_REQ_MSG*)msg_p;
      /*  DCM CLOSE REQ have to be accepted in ACTVATING STATE  */
      if(dcm_env_ctrl_blk_p->state[0] EQ DCM_ACTIVATING_CONN)
      {
        dcm_process_close_conn_event(dcm_close_conn_req_msg_p);
      }
      else
      {
        psaDCM_close_conn_cnf(DCM_BUSY,
                              dcm_close_conn_req_msg_p->close_req.api_instance);
      }
      return DCM_OK;
      /*lint -e527 suppress Warning -- Unreachable */
      /* break is removed ,as case is returning before break so it is not needed */	  
      /*lint +e527 */
    default:
      /* Ignore event - Stay in the same state. */
      return DCM_UNKNOWN_EVENT;
  }
}


/*
 * Function used to close a connection (with PS and later with api_instance)
 *
 * The closing is always on demand of the IPU_id, or may be internally launched.
 * The closing of the connection begin by the closing of the port with the PS.
 * - Close the connection with the PS.
 *
 * @param   T_DCM_CLOSE_CONN_REQ_MSG*
 * @return  DCM_OK or DCM errors
 */
/***************************************************************************************
*   Function       :  dcm_process_close_conn_event
*   Parameter      :  T_APPLI_USER
*                     -Pointer on the header of the message.
*   Return         :  T_DCM_RET
*                     DCM_OK or DCM errors
*   Description    :  Function used in all intermediate states where events is unwaited,
*                     but must be processed.
***************************************************************************************/
T_DCM_RET dcm_process_close_conn_event(T_DCM_CLOSE_CONN_REQ_MSG *close_conn_p )
{
  U8 i;
  U8 current_bearer_count;
//pinghua DCM_OPEN_CLOSE patch 20080429  start 
  T_P_CGREG_STAT status = (T_P_CGREG_STAT)0;
  USHORT lac, ci;


  TRACE_FUNCTION("DCM: dcm_process_close_conn_event()");

  qAT_PercentCGREG(CMD_SRC_LCL, &status, &lac, &ci);
  TRACE_EVENT_P1("NOW NET STATA is %d", status);
  if(status>=P_CGREG_STAT_LIMITED )
  {
		psaDCM_close_conn_cnf(DCM_NO_NETWORK, close_conn_p->close_req.api_instance);
       	return DCM_NO_NETWORK;

  }
// pinghua DCM_OPEN_CLOSE patch 20080429  end

  for (i=0; i < DCM_MAX_NUMBER_IPU; i++)
  {
    if( (dcm_env_ctrl_blk_p->ipu_list[i].api_instance EQ
         close_conn_p->close_req.api_instance) AND
         (dcm_env_ctrl_blk_p->ipu_list[i].row_state) )
    {
      if(dcm_env_ctrl_blk_p->ipu_list[i].bearer_type EQ DCM_BEARER_GPRS)
      {
        current_bearer_count = dcm_env_ctrl_blk_p->gprs_current_total_row;
      }
      else
      {
        current_bearer_count = dcm_env_ctrl_blk_p->gsm_current_total_row;
      }
      /* raise the flag that indicates a disconnection */
      dcm_env_ctrl_blk_p->ipu_list[i].row_state = ROW_CLOSING;
      dcm_new_state(DCM_CLOSING_CONN , DCM_SUB_NO_ACTION);

      if(current_bearer_count EQ 1)
      {
        if(dcm_env_ctrl_blk_p->ipu_list[i].bearer_type EQ DCM_BEARER_GPRS)
        {
          dcm_send_cgdeact_cmd(i);
        }
        else
        {
          dcm_send_percentcal_get_cmd(i);
        }
      }
      else
      {
        psaDCM_close_conn_cnf(DCM_OK,close_conn_p->close_req.api_instance);
        /* free the row used */
        dcm_free_row(i);
        set_state_from_ctrl_blk();
      }
    }
  }
  return DCM_OK;
}

// pinghua DCM_OPEN_CLOSE patch 20080429  start here  
T_DCM_RET dcm_process_network_drop(T_DCM_CLOSE_CONN_REQ_MSG *close_conn_p )
{
  U8 i;
  U8 current_bearer_count;
  

  TRACE_FUNCTION("DCM: dcm_process_network_drop()");

  for (i=0; i < DCM_MAX_NUMBER_IPU; i++)
  {
    if( (dcm_env_ctrl_blk_p->ipu_list[i].api_instance EQ
         close_conn_p->close_req.api_instance) AND
         (dcm_env_ctrl_blk_p->ipu_list[i].row_state) )
    {
      if(dcm_env_ctrl_blk_p->ipu_list[i].bearer_type EQ DCM_BEARER_GPRS)
      {
        current_bearer_count = dcm_env_ctrl_blk_p->gprs_current_total_row;
      }
      else
      {
        current_bearer_count = dcm_env_ctrl_blk_p->gsm_current_total_row;
      }
      /* raise the flag that indicates a disconnection */
      dcm_env_ctrl_blk_p->ipu_list[i].row_state = ROW_CLOSING;
      dcm_new_state(DCM_CLOSING_CONN , DCM_SUB_NO_ACTION);

      if(current_bearer_count EQ 1)
      {
        if(dcm_env_ctrl_blk_p->ipu_list[i].bearer_type EQ DCM_BEARER_GPRS)
        {
          dcm_send_cgdeact_cmd(i);
          is_netdrop = TRUE  ; 	 // pinghua added DCM_OPEN_CLOSE patch	  
        }
        else
        {
          dcm_send_percentcal_get_cmd(i);
        }
      }
      else
      {
        psaDCM_close_conn_cnf(DCM_OK,close_conn_p->close_req.api_instance);
        /* free the row used */
        dcm_free_row(i);
        set_state_from_ctrl_blk();
      }
    }
  }
  return DCM_OK;
}
//pinghua DCM_OPEN_CLOSE patch 20080429  end

/*******************************************************************************
 * Function used to open a connection (with PS)
 *
 * The opening is always on demand of the IPU_id.
 * - Open the connection with the PS.
 *
 * @return  DCM_OK or DCM errors
*******************************************************************************/

EXTERN T_ACI_WAP_STATES wap_state;
T_DCM_RET dcm_process_open_conn_event(T_DCM_OPEN_CONN_REQ_MSG *open_conn_p)
{
  U8 row_id, row_id_free;
// pinghua DCM_OPEN_CLOSE patch 20080429  start 
  T_P_CGREG_STAT status = (T_P_CGREG_STAT)0;
  USHORT lac, ci;
  TRACE_FUNCTION("DCM: dcm_process_open_conn_event()");


  qAT_PercentCGREG(CMD_SRC_LCL, &status, &lac, &ci);
  TRACE_EVENT_P1("NOW NET STATA is %d", status);
  if(status>=P_CGREG_STAT_LIMITED )
  {
		psaDCM_open_conn_cnf(DCM_NO_NETWORK, open_conn_p->conn_req.api_instance);
       	return DCM_NO_NETWORK;

  }
if( wap_state != Wap_Not_Init  )
  {   
//       if(dcm_env_ctrl_blk_p->state[0]==DCM_CONN_ACTIVATED ||
//	   dcm_env_ctrl_blk_p->state[0]==DCM_CLOSING_CONN  || dcm_env_ctrl_blk_p->state[0] ==DCM_ACTIVATING_CONN )
	{
		psaDCM_open_conn_cnf(DCM_NOT_READY, open_conn_p->conn_req.api_instance);
       	return DCM_NOT_READY;
       }
	   
  }

// pinghua DCM_OPEN_CLOSE patch 20080429 end


  /* check if the max number of IPU is reached */
  if (( dcm_env_ctrl_blk_p->gsm_current_total_row +
        dcm_env_ctrl_blk_p->gprs_current_total_row ) >= DCM_MAX_NUMBER_IPU )
  {
    /* Too many IPU_id opened */
    /* send the negative confirmation to the IPU */
    psaDCM_open_conn_cnf(DCM_NOT_READY,
                                open_conn_p->conn_req.api_instance);
    return (DCM_NOT_READY);  /* It is replaced with return in next line to avoid warning */
  }

  /* if possible, get the next row */
  for (row_id = 0, row_id_free = DCM_MAX_NUMBER_IPU; row_id < DCM_MAX_NUMBER_IPU; row_id++)
  {
    if (dcm_env_ctrl_blk_p->ipu_list[row_id].row_state)
    {
      /* send a negative confirmation whether the IPU already exists */
      if (dcm_env_ctrl_blk_p->ipu_list[row_id].api_instance EQ
          open_conn_p->conn_req.api_instance)
      {
        /* send the negative confirmation to the IPU */
        psaDCM_open_conn_cnf(DCM_ALREADY_ACTIVATED,
                                    open_conn_p->conn_req.api_instance);
        return( DCM_ALREADY_ACTIVATED );  /* It is replaced with return in next line to avoid warning */
      }
    }
    else
    {
      /* get the first entry */
      if (row_id_free EQ DCM_MAX_NUMBER_IPU)
      {
        row_id_free = row_id;
      }
    }
  }

  if (row_id_free EQ DCM_MAX_NUMBER_IPU)
  {
    /* send the negative confirmation to the IPU */
    psaDCM_open_conn_cnf(DCM_NOT_READY,
                                open_conn_p->conn_req.api_instance);
    return( DCM_NOT_READY );  /* It is replaced with return in next line to avoid warning */
  }
  /* Check the bearer type */
  /* check best one bearer */
  if(open_conn_p->conn_req.bearer_select EQ DCM_BEARER_ANY)
  {
    T_CGATT_STATE  cgatt_state;
    qAT_PlusCGATT(CMD_SRC_LCL,&cgatt_state);

    if(cgatt_state EQ CGATT_STATE_ATTACHED)
      open_conn_p->conn_req.bearer_select = DCM_BEARER_GPRS;
    else
      open_conn_p->conn_req.bearer_select = DCM_BEARER_GSM;
  }

  /* If application doesn't give any connection parameters than use default */
  if(open_conn_p->conn_req.bearer_select EQ DCM_BEARER_GSM   OR
     open_conn_p->conn_req.bearer_select EQ DCM_BEARER_GPRS)
  {
    psaDCM_open_conn_cnf(DCM_INVALID_PARAMETER,open_conn_p->conn_req.api_instance);
    return DCM_INVALID_PARAMETER;
  }

  /* If application gives the necessary parameters for a connection use these */
  else if(open_conn_p->conn_req.bearer_select EQ DCM_BEARER_AS_SPECIFIED)
  {
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].bearer_handle =
      open_conn_p->conn_req.dcm_info_conn.bearer_handle;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].app_handle    =
      open_conn_p->conn_req.dcm_info_conn.app_handle;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].bearer_type   =
      open_conn_p->conn_req.dcm_info_conn.bearer_type;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].apn_valid     =
      open_conn_p->conn_req.dcm_info_conn.apn_valid;
    if(dcm_env_ctrl_blk_p->ipu_list[row_id_free].apn_valid)
    {
      strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row_id_free].apn,
             (char*)open_conn_p->conn_req.dcm_info_conn.apn);
    }
    else
    {
      strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row_id_free].apn,"");
    }
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].phone_number_valid =
      open_conn_p->conn_req.dcm_info_conn.phone_number_valid;
    if(dcm_env_ctrl_blk_p->ipu_list[row_id_free].phone_number_valid)
    {
      strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row_id_free].phone_number,
             (char*)open_conn_p->conn_req.dcm_info_conn.phone_number);
    }
    else
    {
      strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row_id_free].phone_number,"");
    }
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].user_id_valid =
      open_conn_p->conn_req.dcm_info_conn.user_id_valid;
    if(dcm_env_ctrl_blk_p->ipu_list[row_id_free].user_id_valid)
    {
      strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row_id_free].user_id,
             (char*)open_conn_p->conn_req.dcm_info_conn.user_id);
    }
    else
    {
      strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row_id_free].user_id,"");
    }
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].password_valid =
      open_conn_p->conn_req.dcm_info_conn.password_valid;
    if(dcm_env_ctrl_blk_p->ipu_list[row_id_free].password_valid)
    {
      strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row_id_free].password,
             (char*)open_conn_p->conn_req.dcm_info_conn.password);
    }
    else
    {
      strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row_id_free].password,"");
    }
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].cid             = open_conn_p->conn_req.dcm_info_conn.cid;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].ip_address      = open_conn_p->conn_req.dcm_info_conn.ip_address;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].dns1            = open_conn_p->conn_req.dcm_info_conn.dns1;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].dns2            = open_conn_p->conn_req.dcm_info_conn.dns2;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].gateway         = open_conn_p->conn_req.dcm_info_conn.gateway;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].auth_type       = open_conn_p->conn_req.dcm_info_conn.auth_type;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].data_compr      = open_conn_p->conn_req.dcm_info_conn.data_compr;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].header_compr    = open_conn_p->conn_req.dcm_info_conn.header_compr;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].precedence      = open_conn_p->conn_req.dcm_info_conn.precedence;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].delay           = open_conn_p->conn_req.dcm_info_conn.delay;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].reliability     = open_conn_p->conn_req.dcm_info_conn.reliability;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].peak_throughput = open_conn_p->conn_req.dcm_info_conn.peak_throughput;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].mean_throughput = open_conn_p->conn_req.dcm_info_conn.mean_throughput;
    dcm_env_ctrl_blk_p->ipu_list[row_id_free].shareable       = open_conn_p->conn_req.dcm_info_conn.shareable;
  }

  dcm_env_ctrl_blk_p->ipu_list[row_id_free].api_instance =
    open_conn_p->conn_req.api_instance;

  /* keep the curretn row */
  dcm_env_ctrl_blk_p->current_row = row_id_free;

  /* mark the row as used */
  dcm_env_ctrl_blk_p->ipu_list[row_id_free].row_state = ROW_ASSIGNED;

  /* sum the total of actual rows */
  if(dcm_env_ctrl_blk_p->ipu_list[row_id_free].bearer_type EQ DCM_BEARER_GPRS)
  {
    dcm_env_ctrl_blk_p->gprs_current_total_row++;
  }
  else
  {
    dcm_env_ctrl_blk_p->gsm_current_total_row++;
  }

  if(dcm_env_ctrl_blk_p->gprs_current_total_row > 1 OR
     dcm_env_ctrl_blk_p->gsm_current_total_row > 1)
  {
    psaDCM_open_conn_cnf(DCM_ALREADY_ACTIVATED,open_conn_p->conn_req.api_instance);
    return DCM_ALREADY_ACTIVATED;
  }

  set_gpf_tcpip_call();

  /* DCM state change */
  dcm_new_state(DCM_ACTIVATING_CONN,DCM_SUB_NO_ACTION);

  /* if GPRS: send first GPRS AT Command qAT_CGDCONT  */
  if(dcm_env_ctrl_blk_p->ipu_list[row_id_free].bearer_type EQ DCM_BEARER_GPRS)
  {
    dcm_send_cgdcont_get_cmd(dcm_env_ctrl_blk_p->current_row);
  }
  else /*send first GSM AT Command sAT_PercentPPP */
  {
    dcm_send_percentppp_cmd(dcm_env_ctrl_blk_p->current_row);
  }

  return DCM_OK;
}


/******************************************************************************/
T_DCM_RET dcm_process_get_current_conn_event(T_DCM_GET_CURRENT_CONN_REQ_MSG *current_conn_p)
{
  TRACE_FUNCTION("DCM: dcm_process_get_current_conn_event()");

  if(dcm_env_ctrl_blk_p->ipu_list[dcm_env_ctrl_blk_p->current_row].row_state EQ
     ROW_ASSIGNED) {
    psaDCM_get_current_conn_cnf(DCM_OK,
                                current_conn_p->current_conn_req.api_instance,
                                dcm_env_ctrl_blk_p);
  }
  else {
    psaDCM_get_current_conn_cnf(DCM_NOT_READY,
                                current_conn_p->current_conn_req.api_instance,
                                dcm_env_ctrl_blk_p);
  }
  return DCM_OK;
}


/******************************************************************************/
T_DCM_RET dcm_process_unknown_event_in_idle(T_DCM_HDR* msg_p)
{
  T_DCM_CLOSE_CONN_REQ_MSG *close_conn_req_p;
  T_DCM_RET ret;

  TRACE_FUNCTION("DCM: dcm_process_unknown_event_in_idle()");

  if(msg_p EQ NULL)
    return DCM_INVALID_PARAMETER;

  switch(msg_p->msg_id)
  {
    case DCM_CLOSE_CONN_REQ_MSG :
      close_conn_req_p =(T_DCM_CLOSE_CONN_REQ_MSG *)msg_p;
      psaDCM_close_conn_cnf(DCM_UNKNOWN_EVENT,
                            close_conn_req_p->close_req.api_instance);
      set_state_from_ctrl_blk();
      ret = DCM_OK;
      break;
    default:
      ret = DCM_UNKNOWN_EVENT;
      break;
  }
  return ret;
}


/******************************************************************************/
T_DCM_RET dcm_free_row(U8 current_row)
{
  TRACE_FUNCTION("DCM: dcm_free_row()");

  /* Decrease the current number of IPU */
  dcm_env_ctrl_blk_p->ipu_list[current_row].row_state = ROW_FREE;
  if(dcm_env_ctrl_blk_p->ipu_list[current_row].bearer_type EQ DCM_BEARER_GPRS)
    dcm_env_ctrl_blk_p->gprs_current_total_row--;
  else
    dcm_env_ctrl_blk_p->gsm_current_total_row--;

  /* clear the row in the structure of IP Users */
  dcm_clear_ipu_info(current_row);
  return DCM_OK;
}


/*
 * Function used to store some IPU informations
 *
 * @param   row to access [0, 256],
 * @param   IPU id
 * @param   bearer type
 * @param   apn, mtu, pdp@, cid, user, password, dns1, dns2, gateway
 * @return    DCM_OK or DCM errors
 */
T_DCM_RET dcm_store_ipu_info(U8 row, T_BEARER_TYPE bearer_type, char *apn,
                             char *number, char *pdp_addr, U8 cid_used,
                             char *user, char *password, U32 dns1, U32 dns2,
                             U32 gateway)
{
  TRACE_FUNCTION("DCM: dcm_store_ipu_info()");

  if(bearer_type EQ DCM_BEARER_GPRS)
    strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row].apn, apn);
  else if (bearer_type EQ DCM_BEARER_GSM)
    strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row].phone_number,number);
  else
    return DCM_INVALID_PARAMETER;
  dcm_env_ctrl_blk_p->ipu_list[row].bearer_type = bearer_type;
  dcm_env_ctrl_blk_p->ipu_list[row].cid = cid_used;
  strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row].user_id, user);
  strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row].password, password);
  dcm_env_ctrl_blk_p->ipu_list[row].dns1 = dns1;
  dcm_env_ctrl_blk_p->ipu_list[row].dns2 = dns2;
  dcm_env_ctrl_blk_p->ipu_list[row].gateway = gateway;

  return DCM_OK;
}


/* resets parameters of a row */
T_DCM_RET dcm_clear_ipu_info(U8 row)
{
  char empty[] = "";
  TRACE_FUNCTION("DCM: dcm_clear_ipu_info()");

  dcm_env_ctrl_blk_p->ipu_list[row].bearer_type = DCM_BEARER_NO;
  strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row].apn, empty);
  strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row].phone_number,empty);
  dcm_env_ctrl_blk_p->ipu_list[row].cid = 0;
  strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row].user_id, empty);
  strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row].password, empty);
  dcm_env_ctrl_blk_p->ipu_list[row].dns1 = 0;
  dcm_env_ctrl_blk_p->ipu_list[row].dns2 = 0;
  dcm_env_ctrl_blk_p->ipu_list[row].gateway = 0;

  return DCM_OK;
}

/*
 * Function used to send the <AT+CGDCONT=?> command
 *
 * This command is used to get the next Context IDentifier available in the PS.
 * This cid will be used very often in the suite
 *
 * @param   row in the IPU structure related to the actual command
 * @return  DCM_OK or DCM errors
 */
LOCAL T_DCM_RET dcm_send_cgdcont_get_cmd(U8 row)
{
  /*T_GPRS_CONT_REC defCtxts[MAX_CID_PLUS_EINS]; */
  T_PDP_CONTEXT defCtxts[PDP_CONTEXT_CID_MAX];
  SHORT cid_array[PDP_CONTEXT_CID_MAX];
  UBYTE i;

  TRACE_FUNCTION("DCM: dcm_send_cgdcont_get_cmd()");

  if(qAT_PlusCGDCONT(CMD_SRC_LCL,defCtxts,cid_array) NEQ AT_CMPL)
  {
    psaDCM_open_conn_cnf( DCM_NOT_READY, dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    set_state_from_ctrl_blk();
    return DCM_OK;
  }
  else
  {
    for( i= 0; i < MAX_CID_PLUS_EINS ; i++)
    {
      if(cid_array[i] EQ dcm_env_ctrl_blk_p->ipu_list[row].cid)
      {
        TRACE_EVENT_P2("DCM: dcm cid is the same %d=%d",
                       cid_array[i],
                       dcm_env_ctrl_blk_p->ipu_list[row].cid);
      }
    }
    return  dcm_send_cgdcont_cmd(row);
  }

}


/*
 * Function used to send the <AT+CGDCONT=cid, "IP", "apn", "", 0, 0> command
 *
 * This command is used to declare the PDP Context.
 *
 * @param   row in the IPU structure related to the actual command
 * @return  DCM_OK or DCM errors
 */
LOCAL T_DCM_RET dcm_send_cgdcont_cmd(U8 row)
{
  /*T_GPRS_CONT_REC input_txt; */
  T_PDP_CONTEXT input_txt;

  TRACE_FUNCTION("DCM: dcm_send_cgdcont_cmd()");
  TRACE_EVENT_P1("DCM: ipu_list[row].apn = %s",dcm_env_ctrl_blk_p->ipu_list[row].apn);

  strcpy(input_txt.pdp_apn,(char*)dcm_env_ctrl_blk_p->ipu_list[row].apn);
  strcpy(input_txt.pdp_type,"IP");
  memset(&input_txt.pdp_addr,0,sizeof(T_NAS_ip));
  input_txt.d_comp = PDP_CONTEXT_D_COMP_OMITTED;
  input_txt.h_comp = PDP_CONTEXT_H_COMP_OMITTED;

  if(sAT_PlusCGDCONT(CMD_SRC_LCL,dcm_env_ctrl_blk_p->ipu_list[row].cid, &input_txt) NEQ AT_CMPL)
  {
    psaDCM_open_conn_cnf(DCM_NOT_READY, dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    set_state_from_ctrl_blk();
    return DCM_OK;
  }
  else
  {
    dcm_send_cgatt_cmd(row);
    return DCM_OK;
  }
}


/*
 * Function used to send the <AT+CGATT=cid> command
 *
 * This command is used to force attachment to the network.
 *
 * @param   row in the IPU structure related to the actual command
 * @return  DCM_OK or DCM errors
 */
LOCAL T_DCM_RET dcm_send_cgatt_cmd(U8 row)
{
  TRACE_FUNCTION("DCM: dcm_send_cgatt_cmd()");

  /* prepare the AT command including some dynamic info like cid */
  switch(sAT_PlusCGATT(CMD_SRC_LCL, CGATT_STATE_ATTACHED))
  {
    case AT_FAIL:
    case AT_BUSY:
      psaDCM_open_conn_cnf(DCM_NOT_READY, dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
      dcm_free_row(row);
      set_state_from_ctrl_blk();
      break;

    case AT_CMPL:
      dcm_send_cgpco_auth_cmd(row);
      break;

    case AT_EXCT:
      dcm_env_ctrl_blk_p->dcm_call_back = dcm_handle_message;
      dcm_new_state(DCM_ACTIVATING_CONN, DCM_SUB_WAIT_CGATT_CNF);
      break;

    default :
      break;
  }
  return DCM_OK;
}


/*
 * Function used to send the <AT%CGPCO=0, cid, "PAP, username,
 * password, 0.0.0.0, 0.0.0.0"> command
 *
 * This command is used to configure the PS to TCPIP over SNDCP
 *
 * @param   row in the IPU structure related to the actual command
 * @return  DCM_OK or DCM errors
 */
LOCAL T_DCM_RET dcm_send_cgpco_auth_cmd(U8 row)
{
  CHAR dns[2];
  strcpy(dns, "");

  TRACE_FUNCTION("DCM: dcm_send_cgpco_auth_cmd()");
  TRACE_EVENT_P2("DCM: user=%s, password=%s",
                 dcm_env_ctrl_blk_p->ipu_list[row].user_id,
                 dcm_env_ctrl_blk_p->ipu_list[row].password);

  if(sAT_PercentCGPCO(CMD_SRC_LCL,dcm_env_ctrl_blk_p->ipu_list[row].cid,
                      ACI_PCO_AUTH_PROT_PAP,
                      (char*)dcm_env_ctrl_blk_p->ipu_list[row].user_id,
                      (char*)dcm_env_ctrl_blk_p->ipu_list[row].password,
                      dns,dns) NEQ AT_CMPL)
  {
    psaDCM_open_conn_cnf(DCM_NOT_READY, dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    set_state_from_ctrl_blk();
    return DCM_OK;
  }
  else
  {
    dcm_send_cgerep_cmd(row);
    return DCM_OK;
  }
}


/*
 * Function used to send the <AT+CGEREP=cid, 0> command
 *
 * This command is used to configure the PS to send EVENT to us
 *
 * @param   row in the IPU structure related to the actual command
 * @return  DCM_OK or DCM errors
 */
LOCAL T_DCM_RET dcm_send_cgerep_cmd(U8 row)
{
  TRACE_FUNCTION("DCM: dcm_send_cgerep_cmd()");

  /* prepare the AT command including some dynamic info like cid */
  if (sAT_PlusCGEREP(CMD_SRC_LCL,CGEREP_MODE_BUFFER,CGEREP_BFR_CLEAR) NEQ AT_CMPL)
  {
    psaDCM_open_conn_cnf(DCM_NOT_READY, dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    set_state_from_ctrl_blk();
    return DCM_OK;
  }
  else
  {
    strcpy((char*)dcm_env_ctrl_blk_p->ipu_list[row].phone_number,"*98*1#");
    /*  reset the WAP-data if not done at call termination before */
    sAT_PercentWAP(CMD_SRC_LCL,0);
    sAT_PercentWAP(CMD_SRC_LCL,1);
    dcm_send_sat_dn_cmd(row);
    return DCM_OK;
  }
}


/*
 * Function used to send the <AT+CGACT=cid, 0> command
 *
 * This command is used to deactivate the PDP decontext.
 *
 * @param   row in the IPU structure related to the actual command
 * @return  DCM_OK or DCM errors
 */
LOCAL T_DCM_RET dcm_send_cgdeact_cmd(U8 row)
{
  T_CGATT_STATE gprs_attach_state;
  SHORT cids[MAX_CID_PLUS_EINS] = {GPRS_CID_1,PDP_CONTEXT_CID_INVALID};


  TRACE_FUNCTION("DCM: dcm_send_cgdeact_cmd()");

  /* Packet Counter */
  sAT_PercentSNCNT(CMD_SRC_LCL,NAS_RESET_YES);

  qAT_PlusCGATT(CMD_SRC_LCL,&gprs_attach_state);
  TRACE_EVENT_P1("DCM: Attatch State %d",gprs_attach_state);

#if 0   //// pinghua DCM_OPEN_CLOSE patch 20080429
 
  if(gprs_attach_state NEQ CGATT_STATE_ATTACHED)
  {
    psaDCM_close_conn_cnf(DCM_OK,dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    set_state_from_ctrl_blk();
    return DCM_OK;
  }
 #endif  // // pinghua DCM_OPEN_CLOSE patch 20080429 end

  /* GPRS BEARER CLOSING */
  switch(sAT_PlusCGACT(CMD_SRC_LCL,CGACT_STATE_DEACTIVATED,cids))
  {
    case AT_FAIL:
    case AT_BUSY:
      psaDCM_close_conn_cnf(DCM_NOT_READY,
                            dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
      set_state_from_ctrl_blk();
      break;

    case AT_CMPL:
      psaDCM_close_conn_cnf(DCM_OK, dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
      dcm_free_row(row);
      set_state_from_ctrl_blk();
      break;

    case AT_EXCT:
      dcm_new_state(DCM_CLOSING_CONN, DCM_SUB_WAIT_CGDEACT_CNF);
      dcm_env_ctrl_blk_p->dcm_call_back = dcm_handle_message;
      break;

    default :
      break;
  }
  return DCM_OK;
}


LOCAL T_DCM_RET dcm_send_sat_h_cmd(U8 row)
{
  TRACE_FUNCTION("DCM: dcm_send_sat_h_cmd()");

  switch(sAT_H(CMD_SRC_LCL))
  {
    case AT_FAIL:
    case AT_BUSY:
      psaDCM_close_conn_cnf(DCM_NOT_READY, dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
      set_state_from_ctrl_blk();
      break;

    case AT_CMPL:
      break;

    case AT_EXCT:
      dcm_new_state(DCM_CLOSING_CONN, DCM_SUB_WAIT_SATH_CNF);
      dcm_env_ctrl_blk_p->dcm_call_back = dcm_handle_message;
      break;

    default :
      break;
  }
  return DCM_OK;
}


/*
 * Function used to send the <AT+CGPADDR=cid> command
 *
 * This command is used to get back the pdp@ of the module
 *
 * @param   row in the IPU structure related to the actual command
 * @return  DCM_OK or DCM errors
 */
LOCAL T_DCM_RET dcm_send_cgpaddr_cmd(U8 row)
{
  T_NAS_ip pdp_address[MAX_CID];
  SHORT cid_array[MAX_CID];
  cid_array[0] = dcm_env_ctrl_blk_p->ipu_list[row].cid;
  cid_array[1] = PDP_CONTEXT_CID_INVALID;

  TRACE_FUNCTION("DCM: dcm_send_cgpaddr_cmd()");

  memset(pdp_address , 0x00, sizeof(T_NAS_ip)*MAX_CID);

  /* prepare the AT command including some dynamic info like cid */
  if(sAT_PlusCGPADDR(CMD_SRC_LCL,cid_array,pdp_address) NEQ AT_CMPL)
  {
    psaDCM_open_conn_cnf(DCM_NOT_READY,
                         dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    set_state_from_ctrl_blk();
    return DCM_OK;
  }
  else
  {
    memcpy(dcm_env_ctrl_blk_p->ipu_list[row].pdp_addr,
           pdp_address[0].ip_address.ipv4_addr.a4,
           sizeof(NAS_SIZE_IPv4_ADDR));
    TRACE_EVENT_P1("DCM: PDP addr=%s",dcm_env_ctrl_blk_p->ipu_list[row].pdp_addr);
    dcm_send_cgpco_get_cmd(row);
    return DCM_OK;
  }
}


/*
 * Function used to send the <AT%CGPCO=1,1,,cid> command
 *
 * This command is used to get back the dns1, dns2 and gateway @
 *
 * @param   row in the IPU structure related to the actual command
 * @return  DCM_OK or DCM errors
 */
LOCAL T_DCM_RET dcm_send_cgpco_get_cmd(U8 row)
{
  TRACE_FUNCTION("DCM: dcm_send_cgpco_get_cmd()");

  psaDCM_open_conn_cnf(DCM_OK, dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
  dcm_env_ctrl_blk_p->dcm_call_back = dcm_handle_message;
  dcm_new_state(DCM_CONN_ACTIVATED, DCM_SUB_NO_ACTION);

  return DCM_OK;
}


T_DCM_RET dcm_send_percentppp_cmd(U8 row)
{
  TRACE_FUNCTION("DCM: dcm_send_percentppp_cmd()");
  if(sAT_PercentPPP(CMD_SRC_LCL,
                    A_PAP,
                    (char*)dcm_env_ctrl_blk_p->ipu_list[row].user_id,
                    (char*)dcm_env_ctrl_blk_p->ipu_list[row].password,
                    USE_NO_PPP_FOR_AAA) NEQ AT_CMPL)
  {
    psaDCM_open_conn_cnf(DCM_NOT_READY,
                         dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    set_state_from_ctrl_blk();  
  }
  else
  {
    /*  reset the WAP-data if not done at call termination before */
    sAT_PercentWAP(CMD_SRC_LCL,0);
    sAT_PercentWAP(CMD_SRC_LCL,1);
    dcm_send_sat_dn_cmd(row);
  }

  return DCM_OK;
}


LOCAL T_DCM_RET dcm_send_sat_dn_cmd(U8 row)
{
  TRACE_FUNCTION("DCM: dcm_send_sat_dn_cmd()");

  switch(sAT_Dn(CMD_SRC_LCL,
                (char*)dcm_env_ctrl_blk_p->ipu_list[row].phone_number,
                D_CLIR_OVRD_Default,
                D_CUG_CTRL_NotPresent,
                D_TOC_Data))
  {
    case AT_FAIL:
    case AT_BUSY:
    case AT_CMPL:
      psaDCM_open_conn_cnf(DCM_NOT_READY,
                           dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
      dcm_free_row(row);
      set_state_from_ctrl_blk();
      break;

    case AT_EXCT:
      dcm_env_ctrl_blk_p->dcm_call_back = dcm_handle_message;
      if(dcm_env_ctrl_blk_p->ipu_list[row].bearer_type EQ DCM_BEARER_GPRS) {
        dcm_new_state(DCM_ACTIVATING_CONN, DCM_SUB_WAIT_CGACT_CNF);
      }
      else {
        dcm_new_state(DCM_ACTIVATING_CONN, DCM_SUB_WAIT_SATDN_CNF);
      }
      break;

    default:
      break;
  }

  return DCM_OK;
}


LOCAL T_DCM_RET dcm_send_percentppp_get_cmd(U8 row)
{
  ULONG dns1=0;
  ULONG dns2 =0;
  ULONG ipaddr = 0 ;

  TRACE_FUNCTION("DCM: dcm_send_percentppp_get_cmd()");

  if(qAT_PercentPPP(CMD_SRC_LCL, &ipaddr,&dns1,&dns2) NEQ AT_CMPL)
  {
    psaDCM_open_conn_cnf(DCM_NOT_READY,
                         dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    set_state_from_ctrl_blk();
  }
  else
  {
    psaDCM_open_conn_cnf(DCM_OK,
                         dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_env_ctrl_blk_p->dcm_call_back = dcm_handle_message;
    dcm_new_state(DCM_CONN_ACTIVATED,DCM_SUB_NO_ACTION);
  }

  return DCM_OK;
}


/*
 * Function used to process the events and errors received from PS
 *
 * @param   received message
 * @return  DCM_OK or DCM errors
 */
T_DCM_RET dcm_process_event_error_reception(T_DCM_HDR * msg_p)
{
  U8 row = dcm_env_ctrl_blk_p->current_row;;

  TRACE_FUNCTION("DCM: dcm_process_event_error_reception()");

  /* check if this port number is really used by DCM */
  if(dcm_env_ctrl_blk_p->ipu_list[row].row_state)
  {
    psaDCM_error_ind((T_DCM_STATUS_IND_MSG*)msg_p,
                      dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    /* We cannot use this function call here (set_state_from_ctrl_blk();)
       since this will execute reset_gpf_tcpip_call() immediately, but we need
       to evaulate this flag later on */
    if((dcm_env_ctrl_blk_p->gsm_current_total_row +
       dcm_env_ctrl_blk_p->gprs_current_total_row ) > 0)
    {
      /* another active connection */
      dcm_new_state(DCM_CONN_ACTIVATED, DCM_SUB_NO_ACTION);
    }
    else
    {
      dcm_new_state(DCM_IDLE, DCM_SUB_NO_ACTION);
    }
  }
  return DCM_OK;
}


/*
 * Function used to process the reception of the answer to AT+CGATT=...
 *
 * @param   received message, and row in the IPU structure related to the actual command
 * @return  DCM_OK or DCM errors
 */
T_DCM_RET dcm_process_cgatt_ans(T_DCM_HDR * msg_p, U8 row)
{
  TRACE_FUNCTION("DCM: dcm_process_cgatt_ans()");

  if(msg_p->msg_id EQ DCM_NEXT_CMD_READY_MSG)
  {
    /* send next AT command */
    return dcm_send_cgpco_auth_cmd(row);
  }
  else
  {
    psaDCM_open_conn_cnf(DCM_NOT_READY,
                         dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    set_state_from_ctrl_blk();
    return DCM_OK;
  }
}


/*
 * Function used to process the reception of the answer to AT+CGACT=...
 *
 * @param   received message, and row in the IPU structure related to the actual command
 * @return  DCM_OK or DCM errors
 */
T_DCM_RET dcm_process_cgact_ans(T_DCM_HDR * msg_p, U8 row)
{
  TRACE_FUNCTION("DCM: dcm_process_cgact_ans()");

  if (msg_p->msg_id EQ DCM_NEXT_CMD_READY_MSG)
  {
    return dcm_send_cgpaddr_cmd(row);
  }
  else
  {
    psaDCM_open_conn_cnf(DCM_NOT_READY,
                         dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    /*set_state_from_ctrl_blk();*/
    /* We cannot use this function call here (set_state_from_ctrl_blk();)
       since this will execute reset_gpf_tcpip_call() immediately, but we need
       to evaulate this flag later on */

    if((dcm_env_ctrl_blk_p->gsm_current_total_row +
       dcm_env_ctrl_blk_p->gprs_current_total_row ) > 0)
    {
      /* another active connection */
      dcm_new_state(DCM_CONN_ACTIVATED, DCM_SUB_NO_ACTION);
    }
    else
    {
      dcm_new_state(DCM_IDLE, DCM_SUB_NO_ACTION);
    }
    return DCM_OK;
  }
}


T_DCM_RET dcm_process_cgdeact_ans(T_DCM_HDR * msg_p, U8 row)
{
  TRACE_FUNCTION("DCM: dcm_process_cgdeact_ans()");

  if(msg_p->msg_id EQ DCM_NEXT_CMD_READY_MSG)
  {
//pinghua add one broken messaged indiction for net drop !
   if(is_netdrop == TRUE )
   {

	psaDCM_error_ind((T_DCM_STATUS_IND_MSG*)msg_p,
                         dcm_env_ctrl_blk_p->ipu_list[row].api_instance);

   }	
  else    	
    	psaDCM_close_conn_cnf(DCM_OK,
                          dcm_env_ctrl_blk_p->ipu_list[row].api_instance); 
//end 
   
    dcm_free_row(row);
  }
  else
  {
    psaDCM_close_conn_cnf(DCM_NOT_READY,
                          dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_env_ctrl_blk_p->ipu_list[row].row_state = ROW_ASSIGNED;
  }
  set_state_from_ctrl_blk();
  return DCM_OK;
}


T_DCM_RET dcm_process_sat_dn_ans(T_DCM_HDR * msg_p, U8 row)
{
  TRACE_FUNCTION("DCM: dcm_process_sat_dn_ans()");

  if(msg_p->msg_id EQ DCM_NEXT_CMD_READY_MSG)
  {
    return dcm_send_percentppp_get_cmd(row);
  }
  else
  {
    T_DCM_STATUS_IND_MSG * message = (T_DCM_STATUS_IND_MSG *)msg_p;
    /* We need to check if TCPIP has been already initialised successfully or not*/
    if ( wap_state EQ TCPIP_Activation)
    {
      /* Make shure we shutdown TCPIP properly */
      wap_state = TCPIP_Deactivation;
      psaTCPIP_Shutdown_Req();
    }
    psaDCM_open_conn_cnf(message->result, dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
    set_state_from_ctrl_blk();
    return DCM_OK;
  }
}


T_DCM_RET dcm_process_sat_h_ans(T_DCM_HDR * msg_p, U8 row)
{
  TRACE_FUNCTION("DCM: dcm_process_sat_h_ans()");

  if(msg_p->msg_id EQ DCM_NEXT_CMD_READY_MSG)
  {
    psaDCM_close_conn_cnf(DCM_OK,dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
    dcm_free_row(row);
  }
  else
  {
    psaDCM_close_conn_cnf(DCM_NOT_READY,
                          dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
  }
  set_state_from_ctrl_blk();
  return DCM_OK;
}


GLOBAL ULONG bytes2ipv4addr(UBYTE *host)
{
  UBYTE c1;
  ULONG addr = 0;
  char *tmp;
  if (!host OR host[0]>'9' OR host[0]<'0') return((ULONG)-1);

  tmp=(char *)host;
  c1 = atoi(tmp);
  addr = addr | c1 << 24;
  tmp = strstr(tmp, ".");
  if(!tmp) return((ULONG)-1);
  tmp++;
  c1 = atoi(tmp);
  addr = addr | c1 << 16;
  tmp = strstr(tmp, ".");
  if(!tmp) return((ULONG)-1);
  tmp++;
  c1 = atoi(tmp);
  addr = addr | c1 <<8 ;
  tmp = strstr(tmp, ".");
  if(!tmp) return((ULONG)-1);
  tmp++;
  c1 = atoi(tmp);
  addr = addr | c1 ;
  return dcm_ipaddr_htonl(addr);
}


LOCAL ULONG dcm_ipaddr_htonl(ULONG horder_ipaddr)
{
  return((U32)((((U32)(horder_ipaddr) & 0x000000ffU) << 24) |
          (((U32)(horder_ipaddr) & 0x0000ff00U) <<  8) |
          (((U32)(horder_ipaddr) & 0x00ff0000U) >>  8) |
          (((U32)(horder_ipaddr) & 0xff000000U) >> 24)));
}


LOCAL T_DCM_RET dcm_send_percentcal_get_cmd(UBYTE row)
{
  T_ACI_CAL_ENTR call_tab[MAX_CALL_NR];
  UBYTE i;
  UBYTE count = 0;

  TRACE_FUNCTION("DCM: dcm_send_percentcal_get_cmd()");

  if(qAT_PercentCAL(CMD_SRC_LCL, call_tab) EQ AT_CMPL)
  {
    for(i=0; i<MAX_CALL_NR; i++)
    {
      if(call_tab[i].index EQ -1)
      {
        count++;
        /* in other  words no active call*/
        if(count EQ (MAX_CALL_NR -1) )
        {
          TRACE_EVENT("DCM: No active call");
          psaDCM_close_conn_cnf(DCM_OK, dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
          dcm_free_row(row);
          set_state_from_ctrl_blk();
        }
        break;
      }
      switch(call_tab[i].status)
      {
        case CAL_STAT_Active:
          dcm_send_sat_h_cmd(row);
          break;
        case CAL_STAT_Dial:
        case CAL_STAT_Alerting:
          dcm_send_sat_abort_cmd(row);
          break;

        default:
          TRACE_EVENT("DCM: dcm_send_percentcal_get_cmd DEFAULT call status");
          break;
      }
    }
    return DCM_OK;
  }
  return DCM_OK;
}


LOCAL T_DCM_RET dcm_send_sat_abort_cmd(U8 row)
{
  TRACE_FUNCTION("DCM: dcm_send_sat_h_cmd()");

  switch(sAT_Abort(CMD_SRC_LCL,AT_CMD_D))
  {
    case AT_FAIL:
    case AT_BUSY:
      psaDCM_close_conn_cnf(DCM_NOT_READY,
                            dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
      set_state_from_ctrl_blk();
      break;

    case AT_CMPL:
      psaDCM_close_conn_cnf(DCM_OK,dcm_env_ctrl_blk_p->ipu_list[row].api_instance);
      dcm_free_row(row);
      set_state_from_ctrl_blk();
      break;

    case AT_EXCT:
      dcm_new_state(DCM_CLOSING_CONN, DCM_SUB_WAIT_SATH_CNF);
      dcm_env_ctrl_blk_p->dcm_call_back = dcm_handle_message;
      break;

    default :
      break;
  }
  return DCM_OK;
}


/* This functions checks if antother conneciton is active and changes the
   DCM state corresponding */
LOCAL void set_state_from_ctrl_blk()
{
  if((dcm_env_ctrl_blk_p->gsm_current_total_row +
      dcm_env_ctrl_blk_p->gprs_current_total_row ) > 0)
  {
    /* another active connection */
    dcm_new_state(DCM_CONN_ACTIVATED, DCM_SUB_NO_ACTION);
  }
  else
  {
    dcm_new_state(DCM_IDLE, DCM_SUB_NO_ACTION);
    reset_gpf_tcpip_call();
  }
}
