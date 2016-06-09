/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_PSIS
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

#ifdef FF_PSI
#define PSA_PSIS_C


#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci.h"
#include "aci_lst.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#include "psa_psi.h"
#include "cmh_psi.h"

#include "aci_mem.h"

#include "aci_io.h"

#include "sap_dti.h"



/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

LOCAL USHORT psi_get_desc_size (T_desc2 *data)
{
  USHORT size = 0;
  T_desc2 *next_data;

  next_data = data;
  while (next_data NEQ NULL)
  {
    size += next_data->len;
    next_data = (T_desc2*)next_data->next;
  }

  return (size);
}

#ifdef DTI

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PSIS              |
| STATE   : finished              ROUTINE : psaPSI_ConnectRej       |
+-------------------------------------------------------------------+

  PURPOSE : Indicates that ACI can not handle a DTI connection with PSI

*/
GLOBAL void psaPSI_ConnRej ( U32 devId)
{
  TRACE_FUNCTION ("psaPSI_ConnRej()");
  {
    PALLOC ( psi_conn_rej, PSI_CONN_REJ);
    psi_conn_rej->devId = devId;
    PSENDX (PSI, psi_conn_rej);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PSIS              |
| STATE   : finished              ROUTINE : psaPSI_ConnectRes       |
+-------------------------------------------------------------------+

  PURPOSE : Response to PSI_CONN_IND sent by PSI with confirmed
            DIO capabilities

*/
GLOBAL void psaPSI_ConnRes ( U32 devId)
{
  TRACE_FUNCTION ("psaPSI_ConnRes()");
  {
    PALLOC ( psi_conn_res, PSI_CONN_RES);
    psi_conn_res->devId = devId;
    PSENDX (PSI, psi_conn_res);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PSIS              |
| STATE   : finished              ROUTINE : psaPSI_CloseReq      |
+-------------------------------------------------------------------+

  PURPOSE : Request closing of DTI connection with PSI

*/
GLOBAL void psaPSI_CloseReq ( U32 devId)
{
  TRACE_FUNCTION ("psaPSI_DTICloseReq()");
  {
    PALLOC ( psi_close_req, PSI_CLOSE_REQ);
    psi_close_req->devId = devId;
    PSENDX (PSI, psi_close_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PSIS              |
| STATE   : finished              ROUTINE : psaPSI_DTICloseReq      |
+-------------------------------------------------------------------+

  PURPOSE : Request closing of DTI connection with PSI

*/
GLOBAL void psaPSI_DTICloseReq ( U32 devId, U32 link_id)
{
  TRACE_FUNCTION ("psaPSI_DTICloseReq()");
  {
    PALLOC ( psi_dti_close_req, PSI_DTI_CLOSE_REQ);
    psi_dti_close_req->devId = devId;
    psi_dti_close_req->link_id = link_id;
    PSENDX (PSI, psi_dti_close_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PKTIOS              |
| STATE   : finished              ROUTINE : psaPSI_DTIOpenReq       |
+-------------------------------------------------------------------+

  PURPOSE : Request opening of DTI connection with PSI

*/
GLOBAL void psaPSI_DTIOpenReq ( U32 devId, char* peer_name,
                                ULONG link_id, UBYTE dti_direction)
{
  TRACE_FUNCTION ("psaPSI_DTIOpenReq()");

 /* if (dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_PSI, (UBYTE)(devId&DIO_DEVICE_MASK), 
             UART_DLCI_NOT_MULTIPLEXED) EQ FALSE)
  {
    return;
  }*/
  {
    PALLOC ( psi_dti_open_req, PSI_DTI_OPEN_REQ);
    psi_dti_open_req->devId = devId;
    memcpy( &(psi_dti_open_req->peer.name), peer_name, strlen(peer_name) + 1 );    
    psi_dti_open_req->link_id = link_id;
    psi_dti_open_req->dti_direction = dti_direction;

    PSENDX (PSI, psi_dti_open_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)            MODULE  : PSA_PSIS               |
| STATE   : finished               ROUTINE : psaPSI_Dti_Req         |
+-------------------------------------------------------------------+

  PURPOSE : Request for DTI connection by DTI managment

*/
GLOBAL void psaPSI_Dti_Req ( ULONG  link_id, UBYTE  peer, 
                             T_DTI_MNG_PSI_MODE con_mode)
{
  UBYTE device_no;
  U32  devId;
  char*  peer_name;
  T_DTI_CNTRL  device_info;
  U32 cpblty = DTI_CPBLTY_NO;
  
  TRACE_FUNCTION ("psaPSI_Dti_Req()"); 
      
  dti_cntrl_get_info_from_dti_id(EXTRACT_DTI_ID(link_id), &device_info);

  device_no = device_info.dev_no;
  switch (device_info.capability-DTI_CPBLTY_CMD )
  {
    case DTI_CPBLTY_CMD: 
       cpblty = DTI_CPBLTY_CMD;
       break;
    case DTI_CPBLTY_SER: 
       if (device_info.sub_no EQ 0)
       {
          cpblty = DIO_DATA_SER;
       }
       else
       {
          cpblty = DIO_DATA_SER_MUX;
       }
       break;
    case DTI_CPBLTY_PKT: 
       cpblty = DIO_DATA_PKT;
       break;
    default: 
       break;       
  }
/*the mapping of capability between DTI and PSI should be done here, since devId relates 
    to the device type */
  devId = ((device_info.driver_id & 0xff)<<24|
            cpblty|
            (device_info.dio_ctrl_id & 0xff)<<8|
            device_no);
  peer_name = dti_entity_name[peer].name;
 
  if(con_mode EQ PSI_CONNECT_DTI)
  {
    dti_cntrl_set_conn_parms((T_DTI_CONN_LINK_ID)link_id, DTI_ENTITY_PSI,
                              device_info.dev_no, UART_DLCI_NOT_MULTIPLEXED);

    psaPSI_DTIOpenReq (devId, peer_name, link_id, DTI_CHANNEL_TO_HIGHER_LAYER);
  }
  else
  { 
    psaPSI_DTICloseReq(devId, link_id);                           
  }
}
#endif /* DTI */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_PSIS                     |
|                            ROUTINE : psa_psi_DTI_data_req         |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psa_psi_DTI_data_req (
                               T_desc2 *data,
                               T_ACI_DTI_PRC_PSI *src_infos,
                               T_DTI_ENTITY_ID peer_id
                               )
{
  USHORT data_len = 0;

  TRACE_FUNCTION("psa_psi_DTI_data_req()");

  {
    PALLOC_DESC2(dti_data_ind, DTI2_DATA_IND);

    data_len = psi_get_desc_size (data);

    dti_data_ind->desc_list2.list_len  = data_len;
    dti_data_ind->desc_list2.first     = (ULONG)data;

    dti_data_ind->parameters.p_id               = 0; /*dummy_ubyte; */

    if (BITFIELD_CHECK (src_infos->data_cntr, PSI_DTI_FLOW_OFF))
    {
      dti_data_ind->parameters.st_lines.st_flow = DTI_FLOW_OFF;
    }
    else
    {
      dti_data_ind->parameters.st_lines.st_flow = DTI_FLOW_ON;
    }

    if (BITFIELD_CHECK (src_infos->data_cntr, PSI_DTI_SB_BIT))
    {
      TRACE_EVENT("SB is ON");
      dti_data_ind->parameters.st_lines.st_line_sb = DTI_SB_ON;
    }
    else
    {
      TRACE_EVENT("SB is OFF");
      dti_data_ind->parameters.st_lines.st_line_sb = DTI_SB_OFF;
    }

    dti_data_ind->parameters.st_lines.st_line_sa = DTI_SA_ON;
    dti_data_ind->parameters.st_lines.st_break_len = DTI_BREAK_OFF;

    dti_send_data
    (
      aci_hDTI,
      src_infos->srcId,
      (UBYTE)peer_id,
      ACI_DTI_DN_CHANNEL,
      dti_data_ind
    );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_PSI                      |
|                            ROUTINE : psa_psi_DTI_getdata          |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psa_psi_DTI_getdata( UBYTE src_id, T_DTI_ENTITY_ID peer_id)
{
  T_ACI_DTI_PRC_PSI *src_infos = NULL;

  TRACE_FUNCTION("psaDTI_getdata");

  src_infos = find_element (psi_src_params, src_id, cmhPSItest_srcId);
  if (src_infos EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] psaDTI_getdata: link_id=%d not found", src_id) ;
    return ;
  }

  if( src_infos->RecState NEQ RECEIVING)
  {
    src_infos->RecState = READY_REC;
  }

  {
    dti_start
    (
      aci_hDTI,
      src_id,
      (UBYTE)peer_id,
      ACI_DTI_DN_CHANNEL
    );
  }
}

#ifdef DTI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_PSI                     |
|                            ROUTINE : psaPSI_DCDreq               |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaPSI_DCDreq( UBYTE c_Id, UBYTE line_state )
{
  T_ACI_DTI_PRC_PSI *cmd  = NULL;

  TRACE_FUNCTION("psaPSI_DCDreq()");

  cmd = find_element (psi_src_params, c_Id, cmhPSItest_srcId);
  if (cmd EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] psaPSI_DCDreq: c_Id=%d not found", c_Id) ;
    return ;
  }

  {
    PALLOC( psi_line_state_req, PSI_LINE_STATE_REQ);
    psi_line_state_req -> devId = cmd->devId;
    if( line_state EQ IO_DCD_ON )
    {
      psi_line_state_req -> line_state = LINE_STD_DCD_ON;
    }
    else if ( line_state EQ IO_DCD_OFF )
    {
      psi_line_state_req -> line_state = LINE_STD_DCD_OFF;
    }

    PSENDX( PSI, psi_line_state_req );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_PSI                      |
|                            ROUTINE : psaPSI_ESCAPEreq               |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL BOOL psaPSI_ESCAPEreq( UBYTE c_Id, UBYTE detection )
{
  T_ACI_DEVICE_DCB_ENTRY *device_entry = cmhPSI_GetDeviceParOverSrcID( c_Id );
  U32 device_type;
  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhPSI_SetDataRate: device for source ID = %d not found", c_Id);
    return (FALSE);
  }

  TRACE_FUNCTION("psaPSI_ESCAPEreq()");
  
  device_type = device_entry->devId & DIO_TYPE_DAT_MASK;
  cmhPSI_SetDcbParToUnchanged( device_entry );
  if (device_type EQ DIO_DATA_SER)
  {
    if( detection EQ ESC_DETECTION_ON)
    {
      device_entry->dcbPar.dio_dcb_ser.guard_period = DIO_GUARD_PER_DEFAULT;
    }
    else
    {
      device_entry->dcbPar.dio_dcb_ser.guard_period = 0;
    }
  }      
  else if (device_type EQ DIO_DATA_MUX)
  {
    if( detection EQ ESC_DETECTION_ON)
    {
      device_entry->dcbPar.dio_dcb_ser_mux.guard_period = DIO_GUARD_PER_DEFAULT;
    }
    else
    {
      device_entry->dcbPar.dio_dcb_ser_mux.guard_period = 0;  
    }
  }

  else /* if (device_type EQ DIO_DATA_PKT)*/
  {
    return (FALSE);
  }
  
  psaPSI_SetConfReq(device_entry);
  
  return (TRUE);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_PSI                      |
|                            ROUTINE : psaPSI_RINGreq              |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaPSI_RINGreq( UBYTE c_Id, UBYTE line_state )
{
  T_ACI_DTI_PRC_PSI *cmd  = NULL;

  TRACE_FUNCTION("psaPSI_RINGreq");

  cmd = find_element (psi_src_params, c_Id, cmhPSItest_srcId);
  if (cmd EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] psaPSI_RINGreq: c_Id=%d not found", c_Id) ;
    return ;
  }

  if (BITFIELD_CHECK (cmd->data_cntr, PSI_RING_RUNNING))
  {
    TRACE_EVENT_P2 ("[WRN] psaPSI_RINGreq(): no CNF for previous REQ (device: %d; dlci=%d)",
                    cmd->devId, cmd->dlci);
    return;
  }

  /*
     store that PSI_RING_REQ was send
     this is important so that the primitive queue in the PSI entity not
     overloaded if no PC is connected
  */
  BITFIELD_SET (cmd->data_cntr, PSI_RING_RUNNING);

  {
    PALLOC( psi_line_state_req, PSI_LINE_STATE_REQ);
    psi_line_state_req -> devId = cmd->devId;

    if( line_state EQ IO_RING_ON)
    {
      psi_line_state_req -> line_state = LINE_STD_RING_ON;
    }
    else if( line_state EQ IO_RING_OFF )
    {
      psi_line_state_req -> line_state = LINE_STD_RING_OFF;
    }

    PSENDX( PSI, psi_line_state_req);
  }
}

/*
+------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_PSI                           |
|                            ROUTINE : psaPSI_SetConfReq                 |
+------------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaPSI_SetConfReq( T_ACI_DEVICE_DCB_ENTRY *dcbPar )
{
  U32 device_type;
#ifdef _SIMULATION_
  PALLOC(psi_setconf_req_test, PSI_SETCONF_REQ_TEST);

  device_type = dcbPar->devId & DIO_TYPE_DAT_MASK;
  psi_setconf_req_test->devId = dcbPar->devId;
  switch (device_type)
  {
     case DIO_DATA_SER:
        memcpy(&psi_setconf_req_test->DIO_DCB_UN.DIO_DCB_SER,&dcbPar->dcbPar.dio_dcb_ser,sizeof(T_DIO_DCB_SER));
        break;
     case DIO_DATA_PKT:
        memcpy(&psi_setconf_req_test->DIO_DCB_UN.DIO_DCB_PKT,&dcbPar->dcbPar.dio_dcb_pkt,sizeof(T_DIO_DCB_PKT));
        break;
     case DIO_DATA_SER_MUX:
        memcpy(&psi_setconf_req_test->DIO_DCB_UN.DIO_DCB_SER_MUX,&dcbPar->dcbPar.dio_dcb_ser_mux,sizeof(T_DIO_DCB_SER_MUX));
        break;  
     default:
         break;
  } 
  PSENDX( PSI, psi_setconf_req_test );
#else
  PALLOC(psi_setconf_req, PSI_SETCONF_REQ);

  device_type = dcbPar->devId & DIO_TYPE_DAT_MASK;
  psi_setconf_req->devId = dcbPar->devId;
  switch (device_type)
  {
     case DIO_DATA_SER:
        psi_setconf_req->ptr_DIO_DCB   = (T_DIO_DCB *) &dcbPar->dcbPar.dio_dcb_ser;
         break;
       case DIO_DATA_PKT:
         psi_setconf_req->ptr_DIO_DCB   =(T_DIO_DCB *)  &dcbPar->dcbPar.dio_dcb_pkt;
         break;
       case DIO_DATA_SER_MUX:
        psi_setconf_req->ptr_DIO_DCB   =(T_DIO_DCB *)  &dcbPar->dcbPar.dio_dcb_ser_mux;
         break;
     default:
         break;
  } 
  PSENDX( PSI, psi_setconf_req );
#endif /* _SIMULATION_ */
}
#endif /* DTI */
#endif /*FF_PSI*/
/*==== EOF =======================================================*/

