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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 
#ifndef PSA_DCMS_C
#define PSA_DCMS_C
#endif

/*==== INCLUDES ===================================================*/
#include "aci_all.h"
#include "psa_dcm.h"
#include "socket_int.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/***************************************************************************************
*   Function        :   psaDCM_open_conn_cnf
*   Parameter       :   T_DCM_RET
*                       - dcm result
*                       T_APPLI_USER
*                       - application user id
*   Return          :   BOOL
*                       if sending primitive is sucessful , return TREU  , otherwise return FALSE
*   Description     :   send dcm open conn cnf primitive to application
***************************************************************************************/
GLOBAL BOOL psaDCM_open_conn_cnf(T_DCM_RET result, T_SOCK_API_INSTANCE api_instance)
{
  T_SOCK_API_INSTANCE_DATA* api_data;

  TRACE_FUNCTION("DCM: psaDCM_open_conn_cnf()");
  TRACE_EVENT_P1("DCM: result  %d",result);

  /* set api_data  */
  api_data = (T_SOCK_API_INSTANCE_DATA*)api_instance;
  {
    PALLOC(dcm_open_conn_cnf, DCM_OPEN_CONN_CNF);
    dcm_open_conn_cnf->result = result;
    PSEND(api_data->hCommAPP, dcm_open_conn_cnf);
  }
  return TRUE;
}


/***************************************************************************************
*   Function        :   psaDCM_close_conn_cnf
*   Parameter       :   T_DCM_RET
*                       - dcm result
*                       T_APPLI_USER
*                       - application user id
*   Return          :  BOOL
                       if sending primitive is sucessful , return TREU  , otherwise return FALSE
*   Description     :  send dcm cloes conn cnf primitive to application
***************************************************************************************/
GLOBAL BOOL psaDCM_close_conn_cnf(T_DCM_RET result, T_SOCK_API_INSTANCE api_instance)
{
  T_SOCK_API_INSTANCE_DATA* api_data;

  TRACE_FUNCTION("DCM: psaDCM_close_conn_cnf()");

  /* set api_data  */
  api_data = (T_SOCK_API_INSTANCE_DATA*)api_instance;
  {
    PALLOC(dcm_close_conn_cnf, DCM_CLOSE_CONN_CNF);
    dcm_close_conn_cnf->result = result;
    PSEND(api_data->hCommAPP, dcm_close_conn_cnf);
  }
  return TRUE;

}


/***************************************************************************************
*   Function          :   psaDCM_get_current_conn_cnf
*   Parameter         :   T_DCM_RET
*                          - dcm result
*                         T_APPLI_USER
*                          - application user id
*   Return            :   BOOL
*                         if sending primitive is sucessful , return TREU  , otherwise return FALSE
*   Description       :   send dcm get current conn cnf primitive to application
***************************************************************************************/
GLOBAL BOOL psaDCM_get_current_conn_cnf(T_DCM_RET result,
                                        T_SOCK_API_INSTANCE api_instance,
                                        T_DCM_ENV_CTRL_BLK *dcm_evt_blk)
{
  U8 row;
  T_SOCK_API_INSTANCE_DATA* api_data;

  TRACE_FUNCTION("DCM: psaDCM_get_current_conn_cnf()");

  /* set api_data  */
  api_data = (T_SOCK_API_INSTANCE_DATA*)api_instance;

  row = dcm_evt_blk->current_row;
  {
    PALLOC(dcm_get_current_conn_cnf, DCM_GET_CURRENT_CONN_CNF);
    dcm_get_current_conn_cnf->result = result;

    if(result == DCM_OK)
    {
      /* current connected user infomation*/
      dcm_get_current_conn_cnf->dcm_info_conn.bearer_handle = 
        dcm_evt_blk->ipu_list[row].bearer_handle;
      dcm_get_current_conn_cnf->dcm_info_conn.app_handle = 
        dcm_evt_blk->ipu_list[row].app_handle;
      dcm_get_current_conn_cnf->dcm_info_conn.bearer_type = 
        dcm_evt_blk->ipu_list[row].bearer_type;
      dcm_get_current_conn_cnf->dcm_info_conn.apn_valid =
        dcm_evt_blk->ipu_list[row].apn_valid;
      memcpy(dcm_get_current_conn_cnf->dcm_info_conn.apn,
             dcm_evt_blk->ipu_list[row].apn , (CDCM_APN_MAX_LEN + 1) );
      dcm_get_current_conn_cnf->dcm_info_conn.phone_number_valid = 
        dcm_evt_blk->ipu_list[row].phone_number_valid;
      memcpy(dcm_get_current_conn_cnf->dcm_info_conn.phone_number,
             dcm_evt_blk->ipu_list[row].phone_number, (CDCM_PHONE_NR_LEN + 1) );
      dcm_get_current_conn_cnf->dcm_info_conn.user_id_valid = 
        dcm_evt_blk->ipu_list[row].user_id_valid;
      memcpy(dcm_get_current_conn_cnf->dcm_info_conn.user_id,
             dcm_evt_blk->ipu_list[row].user_id, (CDCM_USER_MAX_LEN + 1) );
      dcm_get_current_conn_cnf->dcm_info_conn.password_valid  = 
        dcm_evt_blk->ipu_list[row].password_valid;
      memcpy(dcm_get_current_conn_cnf->dcm_info_conn.password,
             dcm_evt_blk->ipu_list[row].password, (CDCM_PASSWORD_MAX_LEN + 1) );
      dcm_get_current_conn_cnf->dcm_info_conn.cid = 
        dcm_evt_blk->ipu_list[row].cid;
      dcm_get_current_conn_cnf->dcm_info_conn.ip_address = 
        dcm_evt_blk->ipu_list[row].ip_address;
      dcm_get_current_conn_cnf->dcm_info_conn.dns1 =
        dcm_evt_blk->ipu_list[row].dns1;
      dcm_get_current_conn_cnf->dcm_info_conn.dns2 = 
        dcm_evt_blk->ipu_list[row].dns2;
      dcm_get_current_conn_cnf->dcm_info_conn.gateway =
        dcm_evt_blk->ipu_list[row].gateway;
      dcm_get_current_conn_cnf->dcm_info_conn.auth_type = 
        dcm_evt_blk->ipu_list[row].auth_type;
      dcm_get_current_conn_cnf->dcm_info_conn.data_compr = 
        dcm_evt_blk->ipu_list[row].data_compr;
      dcm_get_current_conn_cnf->dcm_info_conn.header_compr = 
        dcm_evt_blk->ipu_list[row].header_compr;
      dcm_get_current_conn_cnf->dcm_info_conn.precedence = 
        dcm_evt_blk->ipu_list[row].precedence;
      dcm_get_current_conn_cnf->dcm_info_conn.delay =
        dcm_evt_blk->ipu_list[row].delay;
      dcm_get_current_conn_cnf->dcm_info_conn.reliability =
        dcm_evt_blk->ipu_list[row].reliability;
      dcm_get_current_conn_cnf->dcm_info_conn.peak_throughput =
        dcm_evt_blk->ipu_list[row].peak_throughput;
      dcm_get_current_conn_cnf->dcm_info_conn.mean_throughput = 
        dcm_evt_blk->ipu_list[row].mean_throughput;
      dcm_get_current_conn_cnf->dcm_info_conn.shareable =
        dcm_evt_blk->ipu_list[row].shareable;
    }
    PSEND(api_data->hCommAPP, dcm_get_current_conn_cnf);
  }
  return TRUE;
}


/***************************************************************************************
*   Function          :   psaDCM_error_ind
*   Parameter         :   T_DCM_STATUS_IND_MSG *
*                         - dcm error indication msg
*                         T_APPLI_USER
*                         - application user id
*   Return            :   BOOL
*                         if sending primitive is sucessful , return TRUE  , otherwise return FALSE
*   Description       :  send error ind primitive to application
***************************************************************************************/
extern  BOOL  is_netdrop   ;
GLOBAL BOOL psaDCM_error_ind(T_DCM_STATUS_IND_MSG* dcm_error_ind_msg, 
                               T_SOCK_API_INSTANCE api_instance )
{
  T_SOCK_API_INSTANCE_DATA* api_data;

  TRACE_FUNCTION("DCM: psaDCM_error_ind()");

  /* set api_data  */
  api_data = (T_SOCK_API_INSTANCE_DATA*)api_instance;
  {
    PALLOC(dcm_error_ind, DCM_ERROR_IND);
     if( is_netdrop==TRUE)                 //pinghua added 
      {
             dcm_error_ind->dcm_err = 99 ;
	      dcm_error_ind->result = 99; 
	        is_netdrop=FALSE   ;	  
    	}
     else 
     	{
       dcm_error_ind->dcm_err = dcm_error_ind_msg->error;

       dcm_error_ind->result = DCM_PS_CONN_BROKEN; 
     	}	             //pinghua add end 
    PSEND(api_data->hCommAPP, dcm_error_ind);
  }
  return TRUE;
}

