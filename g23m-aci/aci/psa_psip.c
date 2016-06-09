/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_PSIP
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
|  Purpose :  This module defines the processing functions for the
|             primitives sent to the protocol stack adapter by the DTI
|             interface.
+----------------------------------------------------------------------------- 
*/ 
#if defined (FF_PSI) && defined (DTI)
#define PSA_PSIP_C


#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci.h"
#include "aci_lst.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"


#include "psa_psi.h"
#include "cmh_psi.h"
#include "ati_src_psi.h"
#include "ati_io.h"
#include "aci_mem.h"
#include "sap_dti.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/
GLOBAL T_ACI_LIST *psi_dev_list=NULL;

/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/* for tracing of establishing of CMD channels for dual port version */



/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)         MODULE  : PSA_PSIP                   |
| STATE   : finished            ROUTINE : find_psi_dev_no              |
+----------------------------------------------------------------------+

  PURPOSE : search psi device number in psi device list
*/
LOCAL BOOL find_psi_dev_id ( U32 devId, void * elem )
{
  T_ACI_PSI *compared = (T_ACI_PSI *)elem;

  if (compared NEQ NULL)
  {
    if (compared->devId EQ devId )
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
  }
  else
  {
     return FALSE;
  }  
}

#ifdef _SIMULATION_
/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)          MODULE  : PSA_PSIP                    |
| STATE   : finished             ROUTINE : mng_psi_dev_list_test       |
+----------------------------------------------------------------------+

  PURPOSE : manage psi device list
*/
LOCAL BOOL mng_psi_dev_list_test ( T_PSI_CONN_IND_TEST *psi_conn_ind_test  )
{
  T_ACI_PSI *msg_ptr = NULL;
  
  if(psi_dev_list EQ NULL)
  {/* there is no psi device list */
    psi_dev_list = new_list();
  }
  msg_ptr = psi_find_element(psi_dev_list, psi_conn_ind_test->devId, find_psi_dev_id);
  if(msg_ptr EQ NULL)
  {/* added new device */
    {
      ACI_MALLOC(msg_ptr,sizeof(T_ACI_PSI));
      msg_ptr->devId = psi_conn_ind_test->devId;
      msg_ptr->psi_data_mode = psi_conn_ind_test->psi_data_mode; 
      switch (psi_conn_ind_test->devId & DIO_TYPE_DAT_MASK)
      {
        case DIO_DATA_SER:
          memcpy(&msg_ptr->psi_cap.dio_cap_ser,&psi_conn_ind_test->DIO_CAP_UN.DIO_CAP_SER, 
                           sizeof(msg_ptr->psi_cap.dio_cap_ser));
          break;
        case DIO_DATA_MUX:
          memcpy(&msg_ptr->psi_cap.dio_cap_ser_mux, &psi_conn_ind_test->DIO_CAP_UN.DIO_CAP_SER_MUX, 
                           sizeof(msg_ptr->psi_cap.dio_cap_ser_mux));
          break;
        case DIO_DATA_PKT:
           memcpy(&msg_ptr->psi_cap.dio_cap_pkt, &psi_conn_ind_test->DIO_CAP_UN.DIO_CAP_PKT, 
                           sizeof(msg_ptr->psi_cap.dio_cap_pkt));
           break;
      }
      insert_list(psi_dev_list,msg_ptr);
      return (TRUE);
    }
  }
  else
  {/* new DIO capabilities for existing device */
    return (FALSE);
  }
}
#endif /* _SIMULATION_ */

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)          MODULE  : PSA_PSIP                  |
| STATE   : finished             ROUTINE : mng_psi_dev_list            |
+----------------------------------------------------------------------+

  PURPOSE : manage psi device list
*/
LOCAL BOOL mng_psi_dev_list ( T_PSI_CONN_IND *psi_conn_ind  )
{
  T_ACI_PSI *msg_ptr = NULL;
  
  if(psi_dev_list EQ NULL)
  {/* there is no psi device list */
    psi_dev_list = new_list();
  }
  msg_ptr = psi_find_element(psi_dev_list, psi_conn_ind->devId, find_psi_dev_id);
  if(msg_ptr EQ NULL)
  {    /* added new device */
       /*  if((psi_conn_ind->dio_cap->data_mode >= DTI_CPBLTY_CMD ) 
              AND 
       (psi_conn_ind->dio_cap->data_mode <=(DTI_CPBLTY_CMD|DTI_CPBLTY_PKT|DTI_CPBLTY_SER)))*/
    {
      ACI_MALLOC(msg_ptr,sizeof(T_ACI_PSI));
      msg_ptr->devId = psi_conn_ind->devId;
      msg_ptr->psi_data_mode = psi_conn_ind->psi_data_mode; 
      switch (psi_conn_ind->devId & DIO_TYPE_DAT_MASK)
      {
        case DIO_DATA_SER:
          if(psi_conn_ind->ptr_DIO_CAP NEQ NULL)
          {
            memcpy(&msg_ptr->psi_cap.dio_cap_ser,(T_DIO_CAP_SER *)&(psi_conn_ind->ptr_DIO_CAP), 
                           sizeof(msg_ptr->psi_cap.dio_cap_ser));
          }
          break;
        case DIO_DATA_MUX:
          if(psi_conn_ind->ptr_DIO_CAP NEQ NULL)
          {           
            memcpy(&msg_ptr->psi_cap.dio_cap_ser_mux, (T_DIO_CAP_SER_MUX *)&(psi_conn_ind->ptr_DIO_CAP), 
                           sizeof(msg_ptr->psi_cap.dio_cap_ser_mux));
          }
          break;
        case DIO_DATA_PKT:
          if(psi_conn_ind->ptr_DIO_CAP NEQ NULL)
          {
            memcpy(&msg_ptr->psi_cap.dio_cap_pkt, (T_DIO_CAP_PKT *)&(psi_conn_ind->ptr_DIO_CAP), 
                           sizeof(msg_ptr->psi_cap.dio_cap_pkt));
          }
          break;
      }
      insert_list(psi_dev_list,msg_ptr);
      return (TRUE);
    }
    /* else
    { neither CMD, SER nor PKT mode 
      return (FALSE);           
    }*/
  }
  else
  {/* new DIO capabilities for existing device */
    return (FALSE);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                MODULE  : PSA_PSIP            |
| STATE   : code                       ROUTINE : PSI_erase_dev_elem  |
+--------------------------------------------------------------------+

  PURPOSE : erase an entry element from the psi_dev_list list.

*/
LOCAL void PSI_erase_dev_elem(U32 devId)
{
  T_ACI_DTI_PRC_PSI *content;
  content = psi_remove_element (psi_dev_list, devId, find_psi_dev_id);
  ACI_MFREE (content);
}
#ifdef _SIMULATION_
/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PSI                    |
| STATE   : finished              ROUTINE : psa_psi_conn_ind_test      |
+----------------------------------------------------------------------+

  PURPOSE : processes the PSI_CONN_IND_TEST primitive send by PSI.
            add new device to psi device list, create the ATI source and 
            register the DIO capabilities in ACI
*/

GLOBAL const void psa_psi_conn_ind_test ( T_PSI_CONN_IND_TEST *psi_conn_ind_test )
{
  T_ACI_PSI *  msg_ptr;
  UBYTE src_id = 0;
  U32 dio_cap;

  TRACE_FUNCTION ("psa_psi_conn_ind_test()");

  dio_cap =psi_conn_ind_test->devId & DIO_TYPE_DAT_MASK;

  if(mng_psi_dev_list_test(psi_conn_ind_test) EQ TRUE )
  {
    msg_ptr = psi_find_element(psi_dev_list, psi_conn_ind_test->devId, find_psi_dev_id); 
    switch (dio_cap)
    {
      case DIO_DATA_SER:  /*lint -fallthrough*/        
      case DIO_DATA_MUX:  /*lint -fallthrough*/        
      case DIO_DATA_PKT:
        /* Create a new AT source if driver supports AT commands */
        if(psi_conn_ind_test->psi_data_mode EQ DRIVER_MODE_AT OR
           psi_conn_ind_test->psi_data_mode EQ DRIVER_MODE_AT_DATA)
        {
          src_id = psi_new_source(psi_conn_ind_test->devId, NOT_PRESENT_8BIT, dio_cap);
        }
        cmhPSI_Ind(msg_ptr, src_id,dio_cap);
        break;

      default:
        break;
    }
  }
  else 
  {/* second PSI_CONNECT_IND from same device or wrong convergence (mode) in 
        DIO capabilities */
    psaPSI_ConnRej(psi_conn_ind_test->devId);
  }
  /* free the primitive buffer */
  PFREE (psi_conn_ind_test);
}
#endif /* _SIMULATION_ */
/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PSI                   |
| STATE   : finished              ROUTINE : psa_psi_conn_ind        |
+----------------------------------------------------------------------+

  PURPOSE : processes the PSI_CONN_IND primitive send by PSI.
                   add new device to psi device list, create the ATI source and 
                   register the DIO capabilities in ACI
*/

GLOBAL void psa_psi_conn_ind ( T_PSI_CONN_IND *psi_conn_ind )
{
  T_ACI_PSI *  msg_ptr;
  UBYTE src_id = 0;
  U32 dio_cap;

  TRACE_FUNCTION ("psa_psi_conn_ind()");
  /* cmhPSI_SetDcbParToUnchanged( device_entry );*/

  dio_cap =psi_conn_ind->devId & DIO_TYPE_DAT_MASK;

  if(mng_psi_dev_list(psi_conn_ind) EQ TRUE )
  {
    msg_ptr = psi_find_element(psi_dev_list, psi_conn_ind->devId, find_psi_dev_id); 
    switch (dio_cap)
    {
      case DIO_DATA_SER:  /*lint -fallthrough*/        
      case DIO_DATA_MUX:  /*lint -fallthrough*/        
      case DIO_DATA_PKT:
        /* Create a new AT source if driver supports AT commands */
        if(psi_conn_ind->psi_data_mode EQ DRIVER_MODE_AT OR
           psi_conn_ind->psi_data_mode EQ DRIVER_MODE_AT_DATA)
        {
          src_id = psi_new_source(psi_conn_ind->devId, NOT_PRESENT_8BIT, dio_cap);
        }
        cmhPSI_Ind(msg_ptr, src_id,dio_cap);
        break;

      default:
        break;
    }
  }
  else 
  {/* second PSI_CONNECT_IND from same device or wrong convergence (mode) in 
        DIO capabilities */
    psaPSI_ConnRej(psi_conn_ind->devId);
  }
   /* free the primitive buffer */
  PFREE (psi_conn_ind);
}



/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PSI                 |
| STATE   : finished              ROUTINE : psa_psi_dti_close_ind      |
+----------------------------------------------------------------------+

  PURPOSE : processes the PSI_DTI_CLOSE_IND primitive send by PSI.
            this indicates the dti disconnection caused by the DIO driver
*/

GLOBAL void psa_psi_dti_close_ind( T_PSI_DTI_CLOSE_IND *psi_dti_close_ind )
{
  T_ACI_PSI * msg_ptr;
     
  TRACE_FUNCTION ("psa_psi_dti_close_ind()");

  msg_ptr = psi_find_element(psi_dev_list, psi_dti_close_ind->devId, find_psi_dev_id);
  /* if the device_no does not exist in the psi_dev_list
     the primitive is ignored */
  if(msg_ptr NEQ NULL)
  {            
    cmhPSI_DTI_Close(psi_dti_close_ind->devId, PSI_DTI_CLS_IND, psi_dti_close_ind->link_id);
  } 
  else
  {
     TRACE_EVENT ("ERROR: DEVICE NOT FOUND.");
  }
  /* free the primitive buffer */
  PFREE (psi_dti_close_ind);
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PSI                    |
| STATE   : finished              ROUTINE : psa_psi_disconn_ind        |
+----------------------------------------------------------------------+

  PURPOSE : processes the PSI_DISCONN_IND primitive send by PSI.
  this indicates the disconnection of data call caused by the DIO driver
*/
GLOBAL void psa_psi_disconn_ind ( T_PSI_DISCONN_IND *psi_disconn_ind )
{
   T_ACI_PSI * msg_ptr;
   T_ACI_DTI_PRC_PSI *cmd;   

   TRACE_FUNCTION ("psa_psi_disconn_ind()");    

   msg_ptr = psi_find_element(psi_dev_list, psi_disconn_ind->devId, find_psi_dev_id);
   cmd = cmhPSI_find_dlci (psi_src_params, psi_disconn_ind->devId,
                    UART_DLCI_NOT_MULTIPLEXED);
   /* if the device_no does not exist in the psi_dev_list the primitive is ignored */
   if(msg_ptr NEQ NULL)
   {
    /*erase the source*/
    psi_erase_source( cmd->srcId );
    /*erase the src element in the <psi_src_params>*/
    cmhPSI_erase_src_elem (cmd->srcId );
    /*erase the devie from the device list*/
    PSI_erase_dev_elem (psi_disconn_ind->devId);
   } 
   /* free the primitive buffer */
   PFREE (psi_disconn_ind);
}



/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PSI                 |
| STATE   : finished              ROUTINE : psa_psi_dti_close_cnf      |
+----------------------------------------------------------------------+

  PURPOSE : processes the PSI_DTI_CLOSE_CNF primitive sent by PSI.
     this confirms the dti disconnection requested by PSI_DTI_CLOSE_REQ 
*/
GLOBAL void psa_psi_dti_close_cnf ( T_PSI_DTI_CLOSE_CNF *psi_dti_close_cnf )
{
  T_ACI_PSI * msg_ptr;
        
  TRACE_FUNCTION ("psa_psi_dti_close_cnf()");    
  msg_ptr = psi_find_element(psi_dev_list, psi_dti_close_cnf->devId, find_psi_dev_id);
  /* if the devId does not exist in the psi_dev_list
       the primitive is ignored */
  if(msg_ptr NEQ NULL)
  {                    
    cmhPSI_DTI_Close(psi_dti_close_cnf->devId, PSI_DTI_CLS_CNF, psi_dti_close_cnf->link_id);          
  }
  /* psaPSI_CloseReq(psi_dti_close_cnf->devId);*/
  /* free the primitive buffer */
  PFREE (psi_dti_close_cnf);
}


/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PSI                 |
| STATE   : finished              ROUTINE : psa_psi_close_cnf      |
+----------------------------------------------------------------------+

  PURPOSE : processes the PSI_CLOSE_CNF primitive sent by PSI.
     this confirms the dti disconnection requested by PSI_CLOSE_REQ 
*/
GLOBAL void psa_psi_close_cnf ( T_PSI_CLOSE_CNF *psi_close_cnf )
{
   T_ACI_PSI * msg_ptr;
   T_ACI_DTI_PRC_PSI *cmd;
        
   TRACE_FUNCTION ("psa_psi_close_cnf()");    
   msg_ptr = psi_find_element(psi_dev_list, psi_close_cnf->devId, find_psi_dev_id);
   cmd = cmhPSI_find_dlci (psi_src_params, psi_close_cnf->devId,
                    UART_DLCI_NOT_MULTIPLEXED);
   /* if the devId does not exist in the psi_dev_list
       the primitive is ignored */
   if(msg_ptr NEQ NULL)
   {                    
      /*erase the source*/
      psi_erase_source( cmd->srcId );
      /*erase the src element in the <psi_src_params>*/
      cmhPSI_erase_src_elem (cmd->srcId );
      /*erase the devie from the device list*/
      PSI_erase_dev_elem (psi_close_cnf->devId);   
   }
   /* free the primitive buffer */
   PFREE (psi_close_cnf);
}




/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)          MODULE  : PSA_PSI                 |
| STATE   : finished             ROUTINE : psa_psi_dti_open_cnf        |
+----------------------------------------------------------------------+

  PURPOSE : processes the PSI_DTI_OPEN_CNF primitive send by PSI.
            this confirms the dti connection requested by PSI_DTI_OPEN_REQ
*/
GLOBAL void psa_psi_dti_open_cnf ( T_PSI_DTI_OPEN_CNF *psi_dti_open_cnf )
{
  T_ACI_PSI * msg_ptr;
     
  TRACE_FUNCTION ("psa_psi_dti_open_cnf()");

  msg_ptr = psi_find_element(psi_dev_list, psi_dti_open_cnf->devId, find_psi_dev_id);
  /* if the devId does not exist in the psi_dev_list
     the primitive is ignored */
  if(msg_ptr NEQ NULL)
  {
    switch(psi_dti_open_cnf->cause)
    {
      case PSICS_SUCCESS:
        cmhPSI_DTI_OpenCnf(psi_dti_open_cnf->devId, psi_dti_open_cnf->link_id, DTI_OK);
        break;
      case PSICS_INVALID_PARAMS:
      case PSICS_DISCONNECT:
      case PSICS_INTERNAL_DRV_ERROR:
      default:
        cmhPSI_DTI_OpenCnf(psi_dti_open_cnf->devId, psi_dti_open_cnf->link_id, DTI_ERROR);
        break;
    }              
 }
 /* free the primitive buffer */
 PFREE (psi_dti_open_cnf);
}


/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)          MODULE  : PSA_PSI                     |
| STATE   : finished             ROUTINE : psa_psi_setconf_cnf         |
+----------------------------------------------------------------------+

  PURPOSE : processes the PSI_DTI_SETCONF_CNF primitive send by PSI.
            this confirms the changed driver configuration requested 
            by PSI_SETCONF_REQ
*/

GLOBAL void psa_psi_setconf_cnf (T_PSI_SETCONF_CNF *psi_setconf_cnf)
{
  TRACE_FUNCTION ("psa_psi_setconf_cnf()");
  /* free the primitive buffer */
  PFREE (psi_setconf_cnf);
}


/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  :PSA_PSIP                        |
|                            ROUTINE : psa_psi_line_state_cnf              |
+---------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL void psa_psi_line_state_cnf(T_PSI_LINE_STATE_CNF *psi_line_state_cnf)
{
  T_ACI_DTI_PRC_PSI  *src_infos = NULL;

  TRACE_FUNCTION("psa_psi_line_state_cnf ()");
  
  src_infos = cmhPSI_find_dlci (psi_src_params, psi_line_state_cnf->devId, UART_DLCI_NOT_MULTIPLEXED);
    
  if (src_infos EQ NULL)      
  {
    TRACE_EVENT_P1 ("[ERR] psa_psi_line_state_cnf: not found: device=%d;",psi_line_state_cnf->devId);
  }
  else
  {
    BITFIELD_CLEAR (src_infos->data_cntr, PSI_RING_RUNNING);
  }

  PFREE(psi_line_state_cnf);
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_PSIP                       |
|                            ROUTINE : psa_psi_line_state_ind          |
+---------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL void psa_psi_line_state_ind(T_PSI_LINE_STATE_IND *psi_line_state_ind )
{
  TRACE_FUNCTION("psa_psi_line_state_ind()");

  cmhPSI_Line_State_Ind ( psi_line_state_ind->devId, psi_line_state_ind->line_state );

  PFREE(psi_line_state_ind);
}

#endif /*FF_PSI*/
/*==== EOF =======================================================*/

