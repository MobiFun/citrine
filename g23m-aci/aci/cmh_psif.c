/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_PSIF
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
|  Purpose :  This module defines the functions used by the command
|             handler for the PSI module.
+----------------------------------------------------------------------------- 
*/ 
#ifdef FF_PSI

#define CMH_PSIF_C

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "aci_lst.h"
#include "aci_io.h"


#ifdef DTI
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#endif /* DTI */
#include "psa_psi.h"
#include "cmh_psi.h"

#include "aci_mem.h"
#ifdef FAX_AND_DATA
#include "psa.h"
#include "psa_l2r.h"
#endif /* FAX_AND_DATA */


GLOBAL T_ACI_LIST *psi_src_params = NULL;
GLOBAL T_ACI_LIST *psi_dcb_par = NULL;


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  :             |
| STATE   : code                        ROUTINE : cmhPSItest_srcId  |
+--------------------------------------------------------------------+

  PURPOSE : search function for source Id in the structure T_ACI_DTI_PRC_PSI.

*/

GLOBAL BOOL cmhPSItest_srcId( UBYTE srcId, void *elem)
{
  T_ACI_DTI_PRC_PSI *compared = (T_ACI_DTI_PRC_PSI *)elem;
  
  if (compared -> srcId EQ srcId )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

#ifdef DTI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  :                    |
| STATE   : code                        ROUTINE : cmhPSI_lst_init    |
+--------------------------------------------------------------------+

  PURPOSE : initializing of src params list of PSI entity

*/
GLOBAL void cmhPSI_lst_init (void)
{
  TRACE_FUNCTION ("cmhPSI_lst_init()");
  psi_src_params = new_list ();

  if ( !psi_dcb_par )
    psi_dcb_par = new_list ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : cmhPSItest_device |
+--------------------------------------------------------------------+

  PURPOSE : search function for device number in the structure T_ACI_DTI_PRC_PSI.
*/

GLOBAL BOOL cmhPSItest_device (U32 devId, void *elem)
{
  T_ACI_DTI_PRC_PSI *compared = (T_ACI_DTI_PRC_PSI *)elem;
  
  if (compared ->devId EQ devId )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PSI               |
|                                 ROUTINE : cmhPSI_getdata         |
+-------------------------------------------------------------------+

  PURPOSE : tell DTI that ACI is ready to receive more datas.
*/

GLOBAL void cmhPSI_getdata ( T_ACI_CMD_SRC srcId )
{
  TRACE_FUNCTION ("cmhPSI_getdata");
  psa_psi_DTI_getdata ( (UBYTE)srcId, DTI_ENTITY_PSI);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  :            |
| STATE   : code                        ROUTINE : cmhPSI_find_dlci  |
+--------------------------------------------------------------------+

  PURPOSE : returns element with corresponding dlci and device.
  
            If called dlci=UART_DLCI_NOT_MULTIPLEXED (UART entity 
            value for device not multiplexed) then function returns 
            element with corresponding device...

*/
GLOBAL T_ACI_DTI_PRC_PSI *cmhPSI_find_dlci (T_ACI_LIST *search_list, 
                                         U32 devId, UBYTE dlci)
{
  T_ACI_DTI_PRC_PSI *content = NULL;

  if (search_list EQ NULL)
    return NULL;
  
  do
  {
    content = psi_find_next_element (search_list, content, devId, 
                                 cmhPSItest_device);
    if (content EQ NULL)
      return NULL;

    if( dlci EQ UART_DLCI_NOT_MULTIPLEXED OR
        content->dlci EQ UART_DLCI_NOT_MULTIPLEXED OR 
        content->dlci EQ dlci)
      return content;
  }
  while (content NEQ NULL);

  return NULL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)          MODULE  :                        |
| STATE   : code                    ROUTINE : cmhPSI_Line_State_Ind  |
+--------------------------------------------------------------------+

  PURPOSE : evaluates and processes  LINE STATE indications sent by PSI: 
            Escape Sequence Detection
            DTR_DTR Line drop

*/
GLOBAL void cmhPSI_Line_State_Ind (U32 devId, U16 line_state)
{
#ifdef DTI
  T_DTI_ENTITY_ID  peer_id;
  T_ACI_DTI_PRC_PSI *src_infos = NULL;
  T_DTI_ENTITY_ID  entity_list[] = {DTI_ENTITY_ACI};

  TRACE_FUNCTION("cmhPSI_Line_State_Ind()");


  psiShrdPrm.dtr_clearcall = FALSE;

  src_infos = cmhPSI_find_dlci (psi_src_params, devId, UART_DLCI_NOT_MULTIPLEXED);


  if (src_infos EQ NULL)
  {
    TRACE_EVENT("[ERR] Wrong dlci");
    return;
  }

  /* escape sequence detected */
  if ((line_state EQ LINE_STD_ESCD_IND) OR
     ((line_state EQ LINE_STD_DTR_LDR) AND
     ((psiShrdPrm.dtr_behaviour EQ DTR_BEHAVIOUR_CommandMode) OR
     (psiShrdPrm.dtr_behaviour EQ DTR_BEHAVIOUR_ClearCall)) ))
  {
    peer_id = dti_cntrl_get_peer( DTI_ENTITY_PSI, (UBYTE)(devId&DIO_DEVICE_MASK), UART_DLCI_NOT_MULTIPLEXED );

    if ( (psiShrdPrm.dtr_behaviour EQ DTR_BEHAVIOUR_ClearCall) AND
         (line_state EQ LINE_STD_DTR_LDR) AND
         (   (peer_id EQ DTI_ENTITY_TRA)
          OR (peer_id EQ DTI_ENTITY_L2R)
          /*OR (peer_id EQ DTI_ENTITY_T30) also changed in cmhUART_DetectedESC_DTR (cmh_uartr.c) by KJF */
          OR (peer_id EQ DTI_ENTITY_ACI)  /* even disconnect if we are currently in CMD-Mode */
          OR (peer_id EQ DTI_ENTITY_PPPS) /* also drop PPP sessions */
          OR (peer_id EQ DTI_ENTITY_PPPC)
          ) )

    {
      /* this is only for CSD (TRA, L2R, T30) */
      psiShrdPrm.dtr_clearcall = TRUE;
    }

#ifdef FAX_AND_DATA
    if (peer_id EQ DTI_ENTITY_L2R)
    {
      psaL2R_ESC ( src_infos->srcId );
    }
    else
#endif
    {
      dti_cntrl_est_dpath_indirect ( src_infos->srcId,
                                     entity_list,
                                     1,
                                     SPLIT,
                                     atiPSI_dti_cb,
                                     DTI_CPBLTY_CMD,
                                     DTI_CID_NOTPRESENT);
    }
  }

  /* DTR line of serial link drops */
  else if (line_state EQ LINE_STD_DTR_LDR)
  {
    if (psiShrdPrm.dtr_behaviour EQ DTR_BEHAVIOUR_Ignore)
    {
      TRACE_EVENT("DCE ignores DTR");
    }
    else
    {
      TRACE_EVENT("[ERR] Wrong dtr_behaviour value");
    }
  }

  else
  {
    TRACE_EVENT("[ERR] Wrong cause value in PSI_LINE_STATE_IND");
  }
#else
  TRACE_EVENT("[ERR] DTI DISABLED");
#endif /* DTI */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : cmhPSI_erase_elem_received_cmd  |
+--------------------------------------------------------------------+

  PURPOSE : erase an entry element from the psi_src_params list.

*/

GLOBAL void cmhPSI_erase_src_elem(UBYTE srcId)
{
  T_ACI_DTI_PRC_PSI *content;

  content = remove_element (psi_src_params, (UBYTE)srcId, cmhPSItest_srcId);
  ACI_MFREE (content);
}


/*
+--------------------------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_URT                                    |
| STATE   : code                        ROUTINE : cmhPSI_translateBD_RATE_to_DIO4_BAUD  |
+--------------------------------------------------------------------------------------------+

  PURPOSE: 
*/

LOCAL UBYTE cmhPSI_translateBD_RATE_to_DIO4_BAUD    ( T_ACI_BD_RATE rate )
{

  switch ( rate )
  {
    case BD_RATE_AUTO:   return (UBYTE) DIO4_BAUD_AUTO;
    case BD_RATE_75:     return (UBYTE) DIO4_BAUD_75;
    case BD_RATE_150:    return (UBYTE) DIO4_BAUD_150;
    case BD_RATE_300:    return (UBYTE) DIO4_BAUD_300;
    case BD_RATE_600:    return (UBYTE) DIO4_BAUD_600;
    case BD_RATE_1200:   return (UBYTE) DIO4_BAUD_1200;
    case BD_RATE_2400:   return (UBYTE) DIO4_BAUD_2400;
    case BD_RATE_4800:   return (UBYTE) DIO4_BAUD_4800;
    case BD_RATE_7200:   return (UBYTE) DIO4_BAUD_7200;
    case BD_RATE_9600:   return (UBYTE) DIO4_BAUD_9600;
    case BD_RATE_14400:  return (UBYTE) DIO4_BAUD_14400;
    case BD_RATE_19200:  return (UBYTE) DIO4_BAUD_19200;
    case BD_RATE_28800:  return (UBYTE) DIO4_BAUD_28800;
    case BD_RATE_33900:  return (UBYTE) DIO4_BAUD_33900;
    case BD_RATE_38400:  return (UBYTE) DIO4_BAUD_38400;
    case BD_RATE_57600:  return (UBYTE) DIO4_BAUD_57600;
    case BD_RATE_115200: return (UBYTE) DIO4_BAUD_115200;
    case BD_RATE_203125: return (UBYTE) DIO4_BAUD_203125;
    case BD_RATE_406250: return (UBYTE) DIO4_BAUD_406250;
    case BD_RATE_812500: return (UBYTE) DIO4_BAUD_812500;
    default:             return (UBYTE) DIO4_BAUD_NOT_VALID; 
  }
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                    |
| STATE   : code                        ROUTINE : cmhPSI_dcbParFindDevice    |
+-----------------------------------------------------------------------------+

  PURPOSE: Finding the device entry in the 'psi_dcb_par' table 

*/
LOCAL BOOL  cmhPSI_dcbParFindDevice (U32 criterium, void *elem)
{
  if ( criterium EQ  ((T_ACI_DEVICE_DCB_ENTRY *)elem)->devId )
    return TRUE;
  
  return FALSE;
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                    |
| STATE   : code                        ROUTINE : cmhPSI_FindAll             |
+-----------------------------------------------------------------------------+

  PURPOSE: Find every entry 

*/
LOCAL BOOL  cmhPSI_FindAll (U32 criterium, void *elem)
{
  return TRUE;
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)             MODULE  : CMH_PSI                       |
| STATE   : code                      ROUTINE : cmhPSI_getDefaultPSIParameter |
+-----------------------------------------------------------------------------+

  PURPOSE: copy default configuration parameter of serial, packet or 
           MUX device in dcb par structure (T_ACI_PSI_DCB_PAR)
*/
LOCAL void cmhPSI_getDefaultPSIParameter(U32 dio_driver_type, T_ACI_PSI_DCB_PAR *dcbPar)
{
  T_DIO_DCB_SER_MUX dcbPar1 = 
         {
            DIO_TYPE_SER_MUX,                    /*device_type*/
            DIO_SLEEP_NOT_CHAN,         /*sleep_mode*/
            (UBYTE)DIO4_BAUD_115200,            /*baudrate*/
            DIO_CF_8N1,                    /*char_frame*/
            (UBYTE)DIO_FLOW_RTS_CTS,        /*flow_control*/
            XON_DEFAULT,                    /*xon*/
            XOFF_DEFAULT,                  /*xoff*/
            ESC_CHAR_DEFAULT,        /*esc_char*/
            DIO_GUARD_PER_NOT_VALID, /*guard_per*/
            DIO_MODE_SER,                     /*device_mode*/
            (UBYTE)DIO_MUX_MODE_NOT_VALID,         /*mux_mode*/
         };     
 T_DIO_DCB_SER dcbPar2 = 
         {
            DIO_TYPE_SER,                        /*device_type*/
            DIO_SLEEP_NOT_CHAN,         /*sleep_mode*/
            (UBYTE)DIO4_BAUD_115200,            /*baudrate*/
            DIO_CF_8N1,                    /*char_frame*/
            (UBYTE)DIO_FLOW_RTS_CTS,        /*flow_control*/
            XON_DEFAULT,                    /*xon*/
            XOFF_DEFAULT,                  /*xoff*/
            ESC_CHAR_DEFAULT,        /*esc_char*/
            DIO_GUARD_PER_NOT_VALID, /*guard_per*/
         };   
 T_DIO_DCB_PKT dcbPar3 = 
         {
            DIO_TYPE_PKT,                    /*device_type*/
            DIO_SLEEP_NOT_CHAN,         /*sleep_mode*/
         }; 
      
    if (dio_driver_type EQ DIO_DATA_SER)
    {
      memcpy ((T_DIO_DCB_SER*)&dcbPar->dio_dcb_ser, &dcbPar2, sizeof (T_DIO_DCB_SER));
    }
    else if (dio_driver_type EQ DIO_DATA_MUX)
    {
      memcpy ((T_DIO_DCB_SER_MUX*)&dcbPar->dio_dcb_ser_mux, &dcbPar1, sizeof (T_DIO_DCB_SER_MUX));
    }
    else if (dio_driver_type EQ DIO_DATA_PKT)
    {
      memcpy ((T_DIO_DCB_PKT*)&dcbPar->dio_dcb_pkt, &dcbPar3, sizeof (T_DIO_DCB_PKT));
    }
}

/*
+----------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                         |
| STATE   : code                        ROUTINE : cmhPSI_AddDeviceToDcbParameter  |
+----------------------------------------------------------------------------------+

  PURPOSE: add DCB parameter for a new device 

*/
GLOBAL void cmhPSI_AddDeviceToDcbParameter( U32 devId,U32 dio_driver_type )
{
  T_ACI_DEVICE_DCB_ENTRY *device_entry;

  TRACE_FUNCTION("cmhPSI_AddDeviceToDcbParameter");

  /*
   *    try to find the device
   */
  device_entry = psi_find_element(psi_dcb_par, devId, cmhPSI_dcbParFindDevice);

  /*
   *    if the device is new add it to the list
   */
  if ( device_entry EQ NULL )
  {
    ACI_MALLOC (device_entry, sizeof (T_ACI_DEVICE_DCB_ENTRY));

    device_entry->devId = devId;
    /*
     *    Default values by adding a new device
     */
    cmhPSI_getDefaultPSIParameter(dio_driver_type, &(device_entry->dcbPar));

    insert_list (psi_dcb_par, device_entry);
  }
}

/*
+---------------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                              |
| STATE   : code                        ROUTINE : cmhPSI_RemoveDeviceFromDcbParameter  |
+---------------------------------------------------------------------------------------+

  PURPOSE: remove DCB parameter for a device from 'psi_dcb_par' table

*/
GLOBAL void cmhPSI_RemoveDeviceFromDcbParameter( U32 devId )
{
  T_ACI_DEVICE_DCB_ENTRY *device_entry;

  TRACE_FUNCTION("cmhPSI_RemoveDeviceFromDcbParameter()");

  /*
   *  Find the device and remove it.
   */
  device_entry = psi_remove_element(psi_dcb_par, devId, cmhPSI_dcbParFindDevice);

  if ( device_entry NEQ NULL )
  {
    ACI_MFREE ( device_entry );
  }
}

/*
+---------------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                              |
| STATE   : code                        ROUTINE : cmhPSI_CleanDcbParameterList  |
+---------------------------------------------------------------------------------------+

  PURPOSE: clean the 'psi_dcb_par' table 

*/
GLOBAL void cmhPSI_CleanDcbParameterList( void )
{
  T_ACI_DEVICE_DCB_ENTRY *device_entry;

  TRACE_FUNCTION("cmhPSI_CleanDcbParameterList ()");

  /*
   *  Find the device and remove it.
   */
  while ( NULL NEQ (device_entry = psi_remove_element(psi_dcb_par, 0, cmhPSI_FindAll)) )
  {
    ACI_MFREE ( device_entry );
  }
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                         |
| STATE   : code                        ROUTINE : cmhPSI_GetDcbParOverDevice  |
+---------------------------------------------------------------------------------+

  PURPOSE: returns the DCB parameter for this device

*/
GLOBAL T_ACI_DEVICE_DCB_ENTRY* cmhPSI_GetDcbParOverDevice( U32 devId )
{
  T_ACI_DEVICE_DCB_ENTRY  *device_entry;

  TRACE_FUNCTION("cmhPSI_GetDcbParOverDevice ()");

  /*
   *  return DCB parameter for device
   */
  device_entry = psi_find_element(psi_dcb_par, devId, cmhPSI_dcbParFindDevice);

  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhPSI_GetDcbParOverDevice: device=%d not found", devId) ;
    return NULL;
  }

  return device_entry;
}




/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                         |
| STATE   : code                        ROUTINE : cmhPSI_GetDeviceParOverSrcID    |
+---------------------------------------------------------------------------------+

  PURPOSE: returns the DCB parameter for this source ID

*/
GLOBAL T_ACI_DEVICE_DCB_ENTRY* cmhPSI_GetDeviceParOverSrcID( UBYTE srcId )
{
  T_ACI_DEVICE_DCB_ENTRY  *device_entry;
  T_ACI_DTI_PRC_PSI       *cmd_struct;

  TRACE_FUNCTION("cmhPSI_GetDeviceParOverSrcID ()");

  if( !ati_is_src_type(srcId, ATI_SRC_TYPE_PSI) )
  {
    /* not a PSI source */
    return(NULL);
  }
  
  /*
   *  find the device connected to this source ID
   */
  cmd_struct = find_element (psi_src_params, srcId, cmhPSItest_srcId);
  
  if (cmd_struct EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] cmhPSI_GetDeviceParOverSrcID: srcId=%d not found", srcId) ;
    return NULL;
  }    

  /*
   *  return DCB parameter for device
   */
  device_entry = psi_find_element(psi_dcb_par, cmd_struct->devId, cmhPSI_dcbParFindDevice);

  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhPSI_GetDeviceParOverSrcID: device=%d not found", cmd_struct->devId) ;
    return NULL;
  }

  return device_entry;
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                         |
| STATE   : code                        ROUTINE : cmhPSI_GetParityOverSrcID       |
+---------------------------------------------------------------------------------+

  PURPOSE: returns parity for this source ID
*/
GLOBAL UBYTE cmhPSI_GetParityOverSrcID( UBYTE srcId )
{
  T_ACI_DEVICE_DCB_ENTRY *device_entry = cmhPSI_GetDeviceParOverSrcID( srcId );
  UBYTE parity=UART_IO_PA_UNDEF;
  U32 character_frame;
  U32 device_type;

  if ( device_entry EQ NULL )
  {
    return UART_IO_PA_NONE;
  }
  else 
  {
    device_type = device_entry->devId & DIO_TYPE_DAT_MASK;
    if (device_type EQ DIO_DATA_SER)
    {
       character_frame = device_entry->dcbPar.dio_dcb_ser.char_frame;
    }
    else if (device_type EQ DIO_DATA_MUX)
    {
       character_frame = device_entry->dcbPar.dio_dcb_ser_mux.char_frame;
    }
    else /*if (device_type EQ DIO_DATA_PKT)*/
    {
        return(parity);
    }
     
     switch (character_frame)
     {
       case DIO_CF_8N2:
       case DIO_CF_8N1:
       case DIO_CF_7N2:
       case DIO_CF_7N1:
       default:
            parity = UART_IO_PA_NONE;
            break;
       case DIO_CF_8O1:
       case DIO_CF_7O1:
            parity = UART_IO_PA_ODD;
            break;
       case DIO_CF_8E1:
       case DIO_CF_7E1:
            parity = UART_IO_PA_EVEN;
            break;
       case DIO_CF_8S1:
       case DIO_CF_7S1:
       case DIO_CF_8M1:
       case DIO_CF_7M1:
            parity = UART_IO_PA_SPACE;
            break;
     }
  }
  return parity;  
}
/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                         |
| STATE   : code                        ROUTINE : cmhPSI_GetDataBitOverSrcID      |
+---------------------------------------------------------------------------------+

  PURPOSE: returns data bits for this source ID
*/
GLOBAL UBYTE cmhPSI_GetDataBitOverSrcID( UBYTE srcId )
{
  T_ACI_DEVICE_DCB_ENTRY *device_entry = cmhPSI_GetDeviceParOverSrcID( srcId );
  UBYTE data_bit =UART_IO_BPC_UNDEF;
  U32 character_frame;
  U32 device_type;

  if ( device_entry EQ NULL )
  {
    return UART_IO_BPC_8;
  }
 else 
  {
     device_type = device_entry->devId & DIO_TYPE_DAT_MASK;
     if (device_type EQ DIO_DATA_SER)
     {
       character_frame = device_entry->dcbPar.dio_dcb_ser.char_frame;
     }
     else if (device_type EQ DIO_DATA_MUX)
     {
       character_frame = device_entry->dcbPar.dio_dcb_ser_mux.char_frame;
     }
     else /*if (device_type EQ DIO_DATA_PKT)*/
     {
        return(data_bit);
     }
     switch (character_frame)
     {
       case DIO_CF_8N2:
       case DIO_CF_8N1:
       case DIO_CF_8S1:
       case DIO_CF_8O1:
       case DIO_CF_8E1:
       case DIO_CF_8M1:
       default:
            data_bit = UART_IO_BPC_8;
            break;
       case DIO_CF_7N2:
       case DIO_CF_7N1:
       case DIO_CF_7O1:
       case DIO_CF_7E1:
       case DIO_CF_7S1:
       case DIO_CF_7M1:
            data_bit = UART_IO_BPC_7;
            break;
     }
  }
  return data_bit;  
}
/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                         |
| STATE   : code                        ROUTINE : cmhPSI_GetStopBitOverSrcID      |
+---------------------------------------------------------------------------------+

  PURPOSE: returns stop bits for this source ID
*/
GLOBAL UBYTE cmhPSI_GetStopBitOverSrcID( UBYTE srcId )
{
  T_ACI_DEVICE_DCB_ENTRY *device_entry = cmhPSI_GetDeviceParOverSrcID( srcId );
  UBYTE stop_bit=UART_IO_SB_UNDEF;
  U32 character_frame;
  U32 device_type;
  if ( device_entry EQ NULL )
  {
    return UART_IO_SB_1;
  }
 else 
  {
    device_type = device_entry->devId & DIO_TYPE_DAT_MASK;
    if (device_type EQ DIO_DATA_SER)
    {
       character_frame = device_entry->dcbPar.dio_dcb_ser.char_frame;
    }
    else if (device_type EQ DIO_DATA_MUX)
    {
       character_frame = device_entry->dcbPar.dio_dcb_ser_mux.char_frame;
    }
    else /*if (device_type EQ DIO_DATA_PKT)*/
    {
        return(stop_bit);
    }
     switch (character_frame)
     {
       case DIO_CF_8N1:
       case DIO_CF_8S1:
       case DIO_CF_8O1:
       case DIO_CF_8E1:
       case DIO_CF_8M1:
       case DIO_CF_7N1:
       case DIO_CF_7O1:
       case DIO_CF_7E1:
       case DIO_CF_7S1:
       default:
            stop_bit = UART_IO_SB_1;
            break;
       case DIO_CF_8N2:
       case DIO_CF_7N2:
            stop_bit = UART_IO_SB_2;
            break;
     }
  }
  return stop_bit;  
}


/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                     |
| STATE   : code                        ROUTINE : cmhPSI_SetDcbParToUnchanged |
+------------------------------------------------------------------------------+

  PURPOSE: set the PSI DCB parameter to unchanged

*/
GLOBAL void cmhPSI_SetDcbParToUnchanged( T_ACI_DEVICE_DCB_ENTRY *dcbntry )
{
    U32 device_type;
    device_type = dcbntry->devId & DIO_TYPE_DAT_MASK;
    if (device_type EQ DIO_DATA_SER)
    {
       dcbntry->dcbPar.dio_dcb_ser.baudrate       = DIO4_BAUD_NOT_VALID;    /* baud rate                                */
       dcbntry->dcbPar.dio_dcb_ser.char_frame         = DIO_CF_NOT_VALID;      /* bits per character                       */
       dcbntry->dcbPar.dio_dcb_ser.flow_control     = DIO_FLOW_NOT_VALID;    /* flow control mode RX                     */

       dcbntry->dcbPar.dio_dcb_ser.xon         = XON_NOT_VALID;
       dcbntry->dcbPar.dio_dcb_ser.xoff        = XOFF_NOT_VALID;
       /*
       * Set escape parameters
       */
       dcbntry->dcbPar.dio_dcb_ser.esc_char    = ESC_CHAR_NOT_VALID;
       dcbntry->dcbPar.dio_dcb_ser.guard_period      = DIO_GUARD_PER_NOT_VALID;
    }
    else if (device_type EQ DIO_DATA_MUX)
    {
       dcbntry->dcbPar.dio_dcb_ser_mux.baudrate       = DIO4_BAUD_NOT_VALID;    /* baud rate                                */
       dcbntry->dcbPar.dio_dcb_ser_mux.char_frame         = DIO_CF_NOT_VALID;      /* bits per character                       */
       dcbntry->dcbPar.dio_dcb_ser_mux.flow_control     = DIO_FLOW_NOT_VALID;    /* flow control mode RX                     */

       dcbntry->dcbPar.dio_dcb_ser_mux.xon         = XON_NOT_VALID;
       dcbntry->dcbPar.dio_dcb_ser_mux.xoff        = XOFF_NOT_VALID;
       /*
       * Set escape parameters
       */
       dcbntry->dcbPar.dio_dcb_ser_mux.esc_char    = ESC_CHAR_NOT_VALID;
       dcbntry->dcbPar.dio_dcb_ser_mux.guard_period      = DIO_GUARD_PER_NOT_VALID;
    }
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                     |
| STATE   : code                        ROUTINE : cmhPSI_SetDataRate          |
+------------------------------------------------------------------------------+

  PURPOSE: set the PSI data rate

*/
GLOBAL T_ACI_RETURN cmhPSI_SetDataRate ( UBYTE srcId, T_ACI_BD_RATE  rate)
{
  T_ACI_DEVICE_DCB_ENTRY *device_entry = cmhPSI_GetDeviceParOverSrcID( srcId );
  U32 device_type;

  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhPSI_SetDataRate: device for source ID = %d not found", srcId);
    return AT_FAIL;
  }
  device_type = device_entry->devId & DIO_TYPE_DAT_MASK;
  cmhPSI_SetDcbParToUnchanged( device_entry );
  if (device_type EQ DIO_DATA_SER)
  {
    device_entry->dcbPar.dio_dcb_ser.baudrate = cmhPSI_translateBD_RATE_to_DIO4_BAUD( rate );
  }
  else if (device_type EQ DIO_DATA_MUX)
  {
    device_entry->dcbPar.dio_dcb_ser_mux.baudrate = cmhPSI_translateBD_RATE_to_DIO4_BAUD( rate );
  }
  else  /*if (device_type EQ DIO_DATA_PKT)*/
  {
    return AT_FAIL;
  }
  psaPSI_SetConfReq(device_entry);
  return AT_EXCT;
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                     |
| STATE   : code                        ROUTINE : cmhPSI_SetCharacterFraming  |
+------------------------------------------------------------------------------+

  PURPOSE: set the PSI character framing

*/
GLOBAL T_ACI_RETURN cmhPSI_SetCharacterFraming ( UBYTE srcId, 
                                                  T_ACI_BS_FRM format, 
                                                  T_ACI_BS_PAR parity )
{
  T_ACI_DEVICE_DCB_ENTRY *device_entry = cmhPSI_GetDeviceParOverSrcID( srcId );
  U32 device_type;
  
  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhPSI_SetCharacterFraming: device for source ID = %d not found", srcId);
    return AT_FAIL;
  }
  device_type = device_entry->devId & DIO_TYPE_DAT_MASK;
  cmhPSI_SetDcbParToUnchanged( device_entry );

  /*
   *  translate AT command parameter to PSI parameter
   */
 if (device_type EQ DIO_DATA_SER)
 {
  switch (format)
  {
     case BS_FRM_NotPresent:
        break;
     case BS_FRM_Dat8_Par0_St2:
        device_entry->dcbPar.dio_dcb_ser.char_frame = DIO_CF_8N2;
        break;
     case BS_FRM_Dat8_Par0_St1:
        device_entry->dcbPar.dio_dcb_ser.char_frame = DIO_CF_8N1;
        break;
     case BS_FRM_Dat7_Par0_St2:
        device_entry->dcbPar.dio_dcb_ser.char_frame = DIO_CF_7N2;
        break;
     case BS_FRM_Dat7_Par0_St1:
        device_entry->dcbPar.dio_dcb_ser.char_frame = DIO_CF_7N1;
        break;
     case BS_FRM_Dat8_Par1_St1:
        break;
     case BS_FRM_Dat7_Par1_St1:
        break;
     default:
        return AT_FAIL;
  }
  if (format EQ BS_FRM_Dat8_Par1_St1 OR format EQ BS_FRM_Dat7_Par1_St1)
  {
     switch (parity)
     {
         case BS_PAR_NotPresent:
            /* this case is handled by format translation */
            break;
         case BS_PAR_Odd:
             device_entry->dcbPar.dio_dcb_ser.char_frame = 
                 (format EQ BS_FRM_Dat8_Par1_St1 ? DIO_CF_8O1: DIO_CF_7O1);
             break;
         case BS_PAR_Even:
             device_entry->dcbPar.dio_dcb_ser.char_frame = 
                 (format EQ BS_FRM_Dat8_Par1_St1 ? DIO_CF_8E1: DIO_CF_7E1);
             break;
         case BS_PAR_Mark: 
             device_entry->dcbPar.dio_dcb_ser.char_frame = 
                 (format EQ BS_FRM_Dat8_Par1_St1 ? DIO_CF_8M1: DIO_CF_7M1);
             break;
         case BS_PAR_Space:
             device_entry->dcbPar.dio_dcb_ser.char_frame = 
                 (format EQ BS_FRM_Dat8_Par1_St1 ? DIO_CF_8S1: DIO_CF_7S1);
             break;
           default:
              return AT_FAIL;
       }
    }
  }
  else if (device_type EQ DIO_DATA_MUX)
  {
    switch (format)
    {
       case BS_FRM_NotPresent:
          break;
       case BS_FRM_Dat8_Par0_St2:
          device_entry->dcbPar.dio_dcb_ser_mux.char_frame = DIO_CF_8N2;
          break;
       case BS_FRM_Dat8_Par0_St1:
          device_entry->dcbPar.dio_dcb_ser_mux.char_frame = DIO_CF_8N1;
          break;
       case BS_FRM_Dat7_Par0_St2:
          device_entry->dcbPar.dio_dcb_ser_mux.char_frame = DIO_CF_7N2;
          break;
       case BS_FRM_Dat7_Par0_St1:
          device_entry->dcbPar.dio_dcb_ser_mux.char_frame = DIO_CF_7N1;
          break;
       case BS_FRM_Dat8_Par1_St1:
          break;
       case BS_FRM_Dat7_Par1_St1:
          break;
       default:
          return AT_FAIL;
    }
    if (format EQ BS_FRM_Dat8_Par1_St1 OR format EQ BS_FRM_Dat7_Par1_St1)
    {
       switch (parity)
       {
           case BS_PAR_NotPresent:
              /* this case is handled by format translation */
              break;
           case BS_PAR_Odd:
               device_entry->dcbPar.dio_dcb_ser_mux.char_frame = 
                   (format EQ BS_FRM_Dat8_Par1_St1 ? DIO_CF_8O1: DIO_CF_7O1);
               break;
           case BS_PAR_Even:
               device_entry->dcbPar.dio_dcb_ser_mux.char_frame = 
                   (format EQ BS_FRM_Dat8_Par1_St1 ? DIO_CF_8E1: DIO_CF_7E1);
               break;
           case BS_PAR_Mark: 
               device_entry->dcbPar.dio_dcb_ser_mux.char_frame = 
                   (format EQ BS_FRM_Dat8_Par1_St1 ? DIO_CF_8M1: DIO_CF_7M1);
               break;
           case BS_PAR_Space:
               device_entry->dcbPar.dio_dcb_ser_mux.char_frame = 
                   (format EQ BS_FRM_Dat8_Par1_St1 ? DIO_CF_8S1: DIO_CF_7S1);
               break;
           default:
              return AT_FAIL;
     }
  }
}
  psaPSI_SetConfReq(device_entry);
  return AT_EXCT;
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                     |
| STATE   : code                        ROUTINE : cmhPSI_SetFlowControl  |
+------------------------------------------------------------------------------+

  PURPOSE: set the PSI flow control

*/
GLOBAL T_ACI_RETURN cmhPSI_SetFlowControl (UBYTE srcId, T_ACI_RX_FLOW_CTRL DCE_by_DTE)
{
  U32 flow_control = DIO_FLOW_NONE;
  T_ACI_DEVICE_DCB_ENTRY *device_entry = cmhPSI_GetDeviceParOverSrcID( srcId );
  U32 device_type;
  
  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhPSI_SetFlowControl: device for source ID = %d not found", srcId);
    return AT_FAIL;
  }
  device_type = device_entry->devId & DIO_TYPE_DAT_MASK;
    /*
   *  translate AT command parameter to PSI parameter
   */
  switch ( DCE_by_DTE )
  {
    case   RX_FLOW_NONE:
      flow_control = DIO_FLOW_NONE;
      break;
    case   RX_FLOW_SOFTWARE:
      flow_control = DIO_FLOW_XON_XOFF;
      break;
    case   RX_FLOW_HARDWARE:
      flow_control = DIO_FLOW_RTS_CTS;
      break;
    case RX_FLOW_NotPresent:
      return AT_CMPL;
  }
  cmhPSI_SetDcbParToUnchanged( device_entry );

  /*
   *  translate AT command parameter to PSI parameter
   */

  if (device_type EQ DIO_DATA_SER)
  {
     device_entry->dcbPar.dio_dcb_ser.flow_control= flow_control;
  }
  else if (device_type EQ DIO_DATA_MUX)
  {
     device_entry->dcbPar.dio_dcb_ser_mux.flow_control= flow_control;
  }
  else /*if (device_type EQ DIO_DATA_PKT)*/
  {
     return AT_FAIL;
  }

   psaPSI_SetConfReq(device_entry);
 
  return AT_EXCT;
}


/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                     |
| STATE   : code                        ROUTINE : cmhPSI_SetEscape  |
+------------------------------------------------------------------------------+

  PURPOSE: set the escape request on or off

*/

GLOBAL T_ACI_RETURN cmhPSI_SetEscape( UBYTE srcId, UBYTE detection )
{
    T_ACI_DEVICE_DCB_ENTRY *device_entry = cmhPSI_GetDeviceParOverSrcID( srcId );
    U32 device_type;
    
    TRACE_FUNCTION("cmhPSI_SetEscape()");

    if ( device_entry EQ NULL )
    {
      TRACE_EVENT_P1("[ERR] cmhPSI_SetEscape: device for source ID = %d not found", srcId);
      return AT_FAIL;
    }
    device_type = device_entry->devId & DIO_TYPE_DAT_MASK;
    cmhPSI_SetDcbParToUnchanged( device_entry );

    if( detection EQ ESC_DETECTION_OFF)
    {
       if (device_type EQ DIO_DATA_SER)
       {
           device_entry->dcbPar.dio_dcb_ser.guard_period= DIO_ESC_OFF;
       }
       else if (device_type EQ DIO_DATA_MUX)
       {
           device_entry->dcbPar.dio_dcb_ser_mux.guard_period= DIO_ESC_OFF;
       }
       else /*if (device_type EQ DIO_DATA_PKT)*/
       {
           return AT_FAIL;
       }
    }
    psaPSI_SetConfReq(device_entry);

    return AT_EXCT;
  }

/*
+--------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSIF                      |
| STATE   : code                        ROUTINE : cmhPSI_GetFktInterfaceFormat  |
+--------------------------------------------------------------------------------+

  PURPOSE: translate the PSI DCB parameter to the AT command format parameter

  ATTENTION: internal function, DCB paramter is not checking of validity

*/
GLOBAL T_ACI_BS_FRM cmhPSI_GetFktInterfaceFormat( T_ACI_DEVICE_DCB_ENTRY *dcbntry )
{
  T_ACI_BS_FRM format = BS_FRM_NotPresent; 
  U32 char_frame = DIO_CF_NOT_VALID;
  U32 device_type;

  device_type = dcbntry->devId & DIO_TYPE_DAT_MASK;
  if (device_type EQ DIO_DATA_SER)
  {
     char_frame = dcbntry->dcbPar.dio_dcb_ser.char_frame;
  }
  else  if (device_type EQ DIO_DATA_MUX)
  {
     char_frame = dcbntry->dcbPar.dio_dcb_ser_mux.char_frame;
  }
     switch (char_frame)
     {
        case DIO_CF_8N2: 
            format = BS_FRM_Dat8_Par0_St2; 
            break;
        case DIO_CF_8N1: 
            format = BS_FRM_Dat8_Par0_St1;
            break;
        case DIO_CF_7N2: 
            format = BS_FRM_Dat7_Par0_St2;
            break;
        case DIO_CF_7N1:  
            format = BS_FRM_Dat7_Par0_St1; 
            break;
        case DIO_CF_8O1:
        case DIO_CF_8E1:
        case DIO_CF_8M1:
        case DIO_CF_8S1: 
            format = BS_FRM_Dat8_Par1_St1;
            break;
        case DIO_CF_7O1:
        case DIO_CF_7E1:
        case DIO_CF_7M1:
        case DIO_CF_7S1: 
            format = BS_FRM_Dat7_Par1_St1;
            break;
    default:
      break;
     }
  return format;
}

/*
+--------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PSI                        |
| STATE   : code                        ROUTINE : cmhUART_GetFktInterfaceParity  |
+--------------------------------------------------------------------------------+

  PURPOSE: translate the PSI COM parameter to the AT command parity parameter

  ATTENTION: internal function, COM paramter is is not checking of validity

*/
GLOBAL T_ACI_BS_PAR cmhPSI_GetFktInterfaceParity( T_ACI_DEVICE_DCB_ENTRY *dcbntry )
{
   T_ACI_BS_PAR parity = BS_PAR_NotPresent; 
   U32 char_frame = DIO_CF_NOT_VALID;
   U32 device_type;
   
   device_type = dcbntry->devId & DIO_TYPE_DAT_MASK;
   if (device_type EQ DIO_DATA_SER)
   {
     char_frame = dcbntry->dcbPar.dio_dcb_ser.char_frame;
   }
   else  if (device_type EQ DIO_DATA_MUX)
   {
     char_frame = dcbntry->dcbPar.dio_dcb_ser_mux.char_frame;
   }
     switch (char_frame)
     {
        case DIO_CF_8O1:
        case DIO_CF_7O1:
            parity = BS_PAR_Odd;
            break;
        case DIO_CF_8E1:
        case DIO_CF_7E1:
            parity = BS_PAR_Even;
            break;
        case DIO_CF_8M1:
        case DIO_CF_7M1:
            parity = BS_PAR_Even;
            break;
        case DIO_CF_8S1: 
        case DIO_CF_7S1: 
            parity = BS_PAR_Space;
        break;
      default:
            break;
     }
    return parity;
}
#endif /* DTI */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : cmhPSI_clean_all_elem  |
+--------------------------------------------------------------------+

  PURPOSE : clean the psi_src_params list.

*/

GLOBAL void cmhPSI_clean_all_elem( )
{
  T_ACI_DTI_PRC_PSI *psi_src_infos = NULL;

  while (1)
  {
    psi_src_infos = (T_ACI_DTI_PRC_PSI *)remove_first_element(psi_src_params);

    if (psi_src_infos EQ NULL)
    {
      break;
    }
    
    if (psi_src_infos->MuxParms NEQ NULL)
    {
       ACI_MFREE (psi_src_infos->MuxParms);
    }
    memset (psi_src_infos, 0x00, sizeof(T_ACI_DTI_PRC_PSI));
    ACI_MFREE (psi_src_infos);
  }
   
  

}

#endif /*FF_PSI*/
/*==== EOF =======================================================*/

