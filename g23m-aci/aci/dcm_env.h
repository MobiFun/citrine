/* 
+----------------------------------------------------------------------------- 
|  Project :  $Workfile::
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
|  Purpose :  This file contains Data connection manager definitions
+----------------------------------------------------------------------------- 
*/ 

#ifndef __DCM_ENV_H__
#define __DCM_ENV_H__



/*
* Internal structure of the descriptor of IPU
*/
typedef struct
{
	U8  row_state;
  U32 api_instance;                        /*  api instance                    */
  U16 bearer_handle;                       /*  Bearer handle                   */
  U16 app_handle;                          /*  Comm handle of application      */
  U8  bearer_type;                         /*  Used bearer type                */
  U8  apn_valid;                           /*  Flag for apn validation         */
  U8  apn[CDCM_APN_MAX_LEN + 1];           /*  Access Point Name               */
  U8  phone_number_valid;                  /*  Flag for phone number validation*/
  U8  phone_number[CDCM_PHONE_NR_LEN + 1]; /*  CSD dial up phone number        */
  U8  user_id_valid;                       /*  Flag for user ID validation     */
  U8  user_id[CDCM_USER_MAX_LEN + 1];      /*  User ID                         */
  U8  password_valid;                      /*  Flag for password validation    */
  U8  password[CDCM_PASSWORD_MAX_LEN + 1]; /*  Password                        */
  U16 cid;                                 /*  GPRS context ID                 */
  U32 ip_address;                          /*  Used IP address                 */
  U32 dns1;                                /*  First domain name server        */
  U32 dns2;                                /*  Second domain name server       */
  U32 gateway;                             /*  Gateway address                 */
  U16 auth_type;                           /*  Type of authentication          */
  U8  data_compr;                          /*  Flag for data compression       */
  U8  header_compr;                        /*  Flag for header compression     */
  U16 precedence;                          /*  GPRS precedence class           */
  U16 delay;                               /*  GPRS delay class                */
  U16 reliability;                         /*  GPRS reliability class          */
  U16 peak_throughput;                     /*  GPRS peak throughput            */
  U16 mean_throughput;                     /*  GPRS mean throughput            */
  U8  shareable;                           /*  Flag for sharing requested bearer conn */
  U8  pdp_addr[CDCM_PDP_MAX_LEN + 1];
  U8  ipaddr[16];
}T_DCM_IPU_LIST;


typedef  T_DCM_RET (*T_DCM_CALLBACK )(T_DCM_HDR *msg_p);

/**
 * The Control Block buffer of DCM, which gathers all 'Global variables'
 * used by DCM instance.
 *
 * A structure should gathers all the 'global variables' of DCM instance.
 * Indeed, global variable must not be defined in order to avoid using static memory.
 * A T_DCM_ENV_CTRL_BLK buffer is allocated when initializing DCM instance and is
 * then always refered by DCM instance when access to 'global variable'
 * is necessary.
 */
typedef struct
{
	/** Store the current state of the DCM instance */
	T_DCM_INTERNAL_STATE state[2];

	/** Store the current substate of the DCM instance */
	T_DCM_INTERNAL_SUBSTATE substate[2];

	/* current row of IPU list in use */
	U8 current_row;

	/* gsm current total of used rows */
	U8 gsm_current_total_row;

	/* gprs current total of used rows */
	U8 gprs_current_total_row;

    /* current network_state */
	U32 network_state;

	/* structure of IP Users */
	T_DCM_IPU_LIST ipu_list[DCM_MAX_NUMBER_IPU];

    /* dcm_call_back */
	T_DCM_CALLBACK  dcm_call_back;
} T_DCM_ENV_CTRL_BLK;


void dcm_init (void);
void dcm_send_message(T_DCM_STATUS_IND_MSG msg, T_DCM_INTERNAL_SUBSTATE sub_state);
BOOL dcm_check_data_call(U32 event);

EXTERN void dcm_display_message(U8 msg_id);


#endif /* __DCM_ENV_H__ */
