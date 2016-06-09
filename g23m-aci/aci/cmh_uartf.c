/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_UARTF
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
|             handler for the UART module.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef UART

#ifndef CMH_UARTF_C
#define CMH_UARTF_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci_lst.h"

#include "dti_conn_mng.h"
#include "psa_uart.h"
#include "cmh_uart.h"
#include "aci_mem.h"

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : cmhUARTtest_srcId  |
+--------------------------------------------------------------------+

  PURPOSE : search function for source Id in the structure T_ACI_DTI_PRC.

*/

GLOBAL BOOL cmhUARTtest_srcId( UBYTE srcId, void *elem)
{
  T_ACI_DTI_PRC *compared = (T_ACI_DTI_PRC *)elem;
  
  if (compared -> srcId EQ srcId )
    return TRUE;
  else
    return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : cmhUARTtest_device |
+--------------------------------------------------------------------+

  PURPOSE : search function for device number in the structure T_ACI_DTI_PRC.

*/

GLOBAL BOOL cmhUARTtest_device (UBYTE device, void *elem)
{
  T_ACI_DTI_PRC *compared = (T_ACI_DTI_PRC *)elem;
  
  if (compared -> device EQ device )
    return TRUE;
  else
    return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : cmhUART_find_dlci  |
+--------------------------------------------------------------------+

  PURPOSE : returns element with corresponding dlci and device.
  
            If called dlci=UART_DLCI_NOT_MULTIPLEXED (UART entity 
            value for device not multiplexed) then function returns 
            element with corresponding device...

*/
GLOBAL T_ACI_DTI_PRC *cmhUART_find_dlci (T_ACI_LIST *search_list, 
                                         UBYTE device, UBYTE dlci)
{
  T_ACI_DTI_PRC *content = NULL;

  if (search_list EQ NULL)
    return NULL;
  
  do
  {
    content = find_next_element (search_list, content, device, 
                                 cmhUARTtest_device);
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
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : cmhUART_erase_elem_received_cmd  |
+--------------------------------------------------------------------+

  PURPOSE : erase an entry element from the uart_src_params list.

*/

GLOBAL void cmhUART_erase_elem_received_cmd (UBYTE srcId)
{
  T_ACI_DTI_PRC *content;

  /* find element to be erased */
  content = remove_element (uart_src_params, (UBYTE)srcId, cmhUARTtest_srcId);

  ACI_MFREE (content);
}

/*********************************************************
//////////////////////////////////////////////////////////
  TEMP: rests from aci_urt.c

  ready for remove? */

GLOBAL BOOL atiViaTif = FALSE;

/********************************************************
/////////////////////////////////////////////////////////  */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_URT            |
| STATE   : code                        ROUTINE : urt_init           |
+--------------------------------------------------------------------+

  PURPOSE : initialize the module and the UART

*/

GLOBAL void urt_init (void)
{
  TRACE_FUNCTION ("urt_init()");

}

/*
+--------------------------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_URT                                    |
| STATE   : code                        ROUTINE : cmhUART_translateBD_RATE_to_UART_IO_SPEED  |
+--------------------------------------------------------------------------------------------+

  PURPOSE: 
*/

LOCAL UBYTE cmhUART_translateBD_RATE_to_UART_IO_SPEED    ( T_ACI_BD_RATE rate )
{

  switch ( rate )
  {
    case BD_RATE_AUTO:   return UART_IO_SPEED_AUTO;
    case BD_RATE_75:     return UART_IO_SPEED_75;
    case BD_RATE_150:    return UART_IO_SPEED_150;
    case BD_RATE_300:    return UART_IO_SPEED_300;
    case BD_RATE_600:    return UART_IO_SPEED_600;
    case BD_RATE_1200:   return UART_IO_SPEED_1200;
    case BD_RATE_2400:   return UART_IO_SPEED_2400;
    case BD_RATE_4800:   return UART_IO_SPEED_4800;
    case BD_RATE_7200:   return UART_IO_SPEED_7200;
    case BD_RATE_9600:   return UART_IO_SPEED_9600;
    case BD_RATE_14400:  return UART_IO_SPEED_14400;
    case BD_RATE_19200:  return UART_IO_SPEED_19200;
    case BD_RATE_28800:  return UART_IO_SPEED_28800;
    case BD_RATE_33900:  return UART_IO_SPEED_33900;
    case BD_RATE_38400:  return UART_IO_SPEED_38400;
    case BD_RATE_57600:  return UART_IO_SPEED_57600;
    case BD_RATE_115200: return UART_IO_SPEED_115200;
    case BD_RATE_203125: return UART_IO_SPEED_203125;
    case BD_RATE_406250: return UART_IO_SPEED_406250;
    case BD_RATE_812500: return UART_IO_SPEED_812500;
    default:             return UART_IO_SPEED_UNDEF; 
  }
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                    |
| STATE   : code                        ROUTINE : cmhUART_comParFindDevice    |
+-----------------------------------------------------------------------------+

  PURPOSE: Finding the device entry in the 'uart_com_par' table 

*/
LOCAL BOOL  cmhUART_comParFindDevice (UBYTE criterium, void *elem)
{
  if ( criterium EQ  ((T_ACI_DEVICE_ENTRY*)elem)->device )
    return TRUE;
  
  return FALSE;
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                    |
| STATE   : code                        ROUTINE : cmhUART_FindAll             |
+-----------------------------------------------------------------------------+

  PURPOSE: Find every entry 

*/
LOCAL BOOL  cmhUART_FindAll (UBYTE criterium, void *elem)
{
  return TRUE;
}

LOCAL T_ACI_UART_COM_PAR cmhUART_getDefaultUARTParameter(void)
{
  T_ACI_UART_COM_PAR comPar =
  {
    UART_IO_SPEED_115200,
    UART_IO_BPC_8,
    UART_IO_SB_1,
    UART_IO_PA_NONE,
    UART_IO_FC_RX_RTS,
    UART_IO_FC_TX_RTS,
    UART_IO_XON_VALID,
    UART_IO_XON_DEFAULT,
    UART_IO_XOFF_VALID,
    UART_IO_XOFF_DEFAULT,
  };

  return comPar;
}
/*
+----------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                         |
| STATE   : code                        ROUTINE : cmhUART_AddDeviceToComParameter  |
+----------------------------------------------------------------------------------+

  PURPOSE: add COM parameter for a new device 

*/
GLOBAL void cmhUART_AddDeviceToComParameter( UBYTE device )
{
  T_ACI_DEVICE_ENTRY *device_entry;

  TRACE_FUNCTION("cmhUART_AddDeviceToComParameter");

  /*
   *    try to find the device
   */
  device_entry = find_element(uart_com_par, device, cmhUART_comParFindDevice);

  /*
   *    if the device is new add it to the list
   */
  if ( device_entry EQ NULL )
  {
    ACI_MALLOC (device_entry, sizeof (T_ACI_DEVICE_ENTRY));

    device_entry->device = device;
    /*
     *    Default values by adding a new device
     */
    device_entry->comPar = cmhUART_getDefaultUARTParameter();

    insert_list (uart_com_par, device_entry);
  }
}

/*
+---------------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                              |
| STATE   : code                        ROUTINE : cmhUART_RemoveDeviceFromComParameter  |
+---------------------------------------------------------------------------------------+

  PURPOSE: remove COM parameter for a device from 'uart_com_par' table

*/
GLOBAL void cmhUART_RemoveDeviceFromComParameter( UBYTE device )
{
  T_ACI_DEVICE_ENTRY *device_entry;

  TRACE_FUNCTION("cmhUART_RemoveDeviceFromComParameter");

  /*
   *  Find the device and remove it.
   */
  device_entry = remove_element(uart_com_par, device, cmhUART_comParFindDevice);

  if ( device_entry NEQ NULL )
  {
    ACI_MFREE ( device_entry );
  }
}

/*
+---------------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                              |
| STATE   : code                        ROUTINE : cmhUART_RemoveDeviceFromComParameter  |
+---------------------------------------------------------------------------------------+

  PURPOSE: clean the 'uart_com_par' table 

*/
GLOBAL void cmhUART_CleanComParameterList( void )
{
  T_ACI_DEVICE_ENTRY *device_entry;

  TRACE_FUNCTION("cmhUART_CleanComParameterList");

  /*
   *  Find the device and remove it.
   */
  while ( NULL NEQ (device_entry = remove_element(uart_com_par, 0, cmhUART_FindAll)) )
  {
    ACI_MFREE ( device_entry );
  }
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                        |
| STATE   : code                        ROUTINE : cmhUART_GetDeviceParOverDevice  |
+---------------------------------------------------------------------------------+

  PURPOSE: returns the COM parameter for this device

*/
GLOBAL T_ACI_DEVICE_ENTRY* cmhUART_GetDeviceParOverDevice( UBYTE device )
{
  T_ACI_DEVICE_ENTRY  *device_entry;

  TRACE_FUNCTION("cmhUART_GetDeviceParOverDevice");

  /*
   *  return COM parameter for device
   */
  device_entry = find_element(uart_com_par, device, cmhUART_comParFindDevice);

  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhUART_GetDeviceParOverDevice: device=%d not found", device) ;
    return NULL;
  }

  return device_entry;
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                        |
| STATE   : code                        ROUTINE : cmhUART_GetDeviceParOverDevice  |
+---------------------------------------------------------------------------------+

  PURPOSE: 

*/
GLOBAL T_ACI_DEVICE_TYPE cmhUART_GetDeviceType( UBYTE srcId )
{
  T_ACI_DTI_PRC *src_infos;

  TRACE_FUNCTION("cmhUART_GetDeviceType");

  src_infos = find_element (uart_src_params, (UBYTE)srcId, cmhUARTtest_srcId);

  if (src_infos NEQ NULL)
  {
    return src_infos->device_type;
  }
  else
  {
    return DEVICE_TYPE_UNKNOWN;
  }
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : CMH_UART                           |
| STATE   : code                     ROUTINE : cmhUART_ChangeDeviceParOverDevice  |
+---------------------------------------------------------------------------------+

  PURPOSE: 

*/
GLOBAL BOOL cmhUART_ChangeDeviceType( UBYTE             srcId, 
                                      T_ACI_DEVICE_TYPE device_type )
{
  T_ACI_DTI_PRC *src_infos;

  TRACE_FUNCTION("cmhUART_ChangeDeviceType");

  src_infos = find_element (uart_src_params, (UBYTE)srcId, cmhUARTtest_srcId);
  
  if (src_infos NEQ NULL)
  {
    src_infos->device_type = device_type;

    switch (device_type)
    {
    case DEVICE_TYPE_UNKNOWN:
      TRACE_EVENT_P1("Device type of src %u is DEVICE_TYPE_UNKNOWN", srcId);
      break;
    case DEVICE_TYPE_URT:
      TRACE_EVENT_P1("Device type of src %u is DEVICE_TYPE_URT", srcId);
      break;
    case DEVICE_TYPE_MUX:
      TRACE_EVENT_P1("Device type of src %u is DEVICE_TYPE_MUX", srcId);
      break;
    default:
      TRACE_EVENT_P1("Device type of src %u has invalid value", srcId);
      break;
    }

    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                        |
| STATE   : code                        ROUTINE : cmhUART_GetDeviceParOverSrcID   |
+---------------------------------------------------------------------------------+

  PURPOSE: returns the COM parameter for this source ID

*/
GLOBAL T_ACI_DEVICE_ENTRY* cmhUART_GetDeviceParOverSrcID( UBYTE srcId )
{
  T_ACI_DEVICE_ENTRY  *device_entry;
  T_ACI_DTI_PRC       *cmd_struct;

  TRACE_FUNCTION("cmhUART_GetDeviceParOverSrcID()");

  if( !ati_is_src_type(srcId, ATI_SRC_TYPE_UART) )
  {
    /* not a UART source */
    return(NULL);
  }
  
  /*
   *  find the device connected to this source ID
   */
  cmd_struct = find_element (uart_src_params, srcId, cmhUARTtest_srcId);
  
  if (cmd_struct EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] cmhUART_GetDeviceParOverSrcID: srcId=%d not found", srcId) ;
    return NULL;
  }    

  /*
   *  return COM parameter for device
   */
  device_entry = find_element(uart_com_par, cmd_struct->device, cmhUART_comParFindDevice);

  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhUART_GetDeviceParOverSrcID: device=%d not found", cmd_struct->device) ;
    return NULL;
  }

  return device_entry;
}

GLOBAL UBYTE cmhUART_GetParityOverSrcID( UBYTE srcId )
{
  T_ACI_DEVICE_ENTRY *device_entry = cmhUART_GetDeviceParOverSrcID( (UBYTE) srcId );

  if ( device_entry EQ NULL )
  {
    return cmhUART_getDefaultUARTParameter().parity;
  }

  return device_entry->comPar.parity;  
}

GLOBAL UBYTE cmhUART_GetDataBitOverSrcID( UBYTE srcId )
{
  T_ACI_DEVICE_ENTRY *device_entry = cmhUART_GetDeviceParOverSrcID( (UBYTE) srcId );

  if ( device_entry EQ NULL )
  {
    return cmhUART_getDefaultUARTParameter().bpc;
  }

  return device_entry->comPar.bpc;  
}

GLOBAL UBYTE cmhUART_GetStopBitOverSrcID( UBYTE srcId )
{
  T_ACI_DEVICE_ENTRY *device_entry = cmhUART_GetDeviceParOverSrcID( (UBYTE) srcId );

  if ( device_entry EQ NULL )
  {
    return cmhUART_getDefaultUARTParameter().nsb;
  }

  return device_entry->comPar.nsb;  
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                     |
| STATE   : code                        ROUTINE : cmhUART_SetComParToUnchanged |
+------------------------------------------------------------------------------+

  PURPOSE: set the UART COM parameter to unchanged

*/
GLOBAL void cmhUART_SetComParToUnchanged( T_comPar *comPar )
{
    comPar->speed       = UART_IO_SPEED_UNDEF;    /* baud rate                                */
    comPar->bpc         = UART_IO_BPC_UNDEF;      /* bits per character                       */
    comPar->nsb         = UART_IO_SB_UNDEF;       /* stop bits                                */
    comPar->parity      = UART_IO_PA_UNDEF;       /* parity of serial link                    */
    comPar->flow_rx     = UART_IO_FC_RX_UNDEF;    /* flow control mode RX                     */
    comPar->flow_tx     = UART_IO_FC_TX_UNDEF;    /* flow control mode TX                     */
    comPar->xon_valid   = UART_IO_XON_UNDEF;      /* indicator whether xon is valid           */
    comPar->xoff_valid  = UART_IO_XOFF_UNDEF;     /* indicator whether xoff is valid          */

    comPar->xon         = UART_IO_XON_DEFAULT;
    comPar->xoff        = UART_IO_XOFF_DEFAULT;

    /*
     * Set escape parameters
     */
    comPar->esc_valid   = UART_IO_ESC_VALID;
    comPar->esc_char    = UART_IO_ESC_CHAR_DEFAULT;
    comPar->esc_gp      = UART_IO_ESC_GP_DEFAULT;
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                     |
| STATE   : code                        ROUTINE : cmhUART_SetDataRate          |
+------------------------------------------------------------------------------+

  PURPOSE: set the UART data rate

*/
GLOBAL T_ACI_RETURN cmhUART_SetDataRate ( UBYTE srcId, T_ACI_BD_RATE  rate)
{
  T_ACI_DEVICE_ENTRY *device_entry = cmhUART_GetDeviceParOverSrcID( srcId );
  T_comPar  newComPar;

  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhUART_SetDataRate: device for source ID = %d not found", srcId);
    return AT_FAIL;
  }

  cmhUART_SetComParToUnchanged( &newComPar );

  device_entry->comPar.speed = newComPar.speed = cmhUART_translateBD_RATE_to_UART_IO_SPEED( rate );

  psaUART_SetParameters(device_entry->device, &newComPar);

  return AT_EXCT;
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                     |
| STATE   : code                        ROUTINE : cmhUART_SetCharacterFraming  |
+------------------------------------------------------------------------------+

  PURPOSE: set the UART character framing

*/
GLOBAL T_ACI_RETURN cmhUART_SetCharacterFraming ( UBYTE srcId, 
                                                  T_ACI_BS_FRM format, 
                                                  T_ACI_BS_PAR parity )
{
  T_ACI_DEVICE_ENTRY *device_entry = cmhUART_GetDeviceParOverSrcID( srcId );
  T_comPar            comPar;
  
  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhUART_SetCharacterFraming: device for source ID = %d not found", srcId);
    return AT_FAIL;
  }

  cmhUART_SetComParToUnchanged( &comPar );

  /*
   *  translate AT command parameter to UART parameter
   */
  if ( format NEQ BS_FRM_NotPresent )
  {
    if ( format & 4 )
    {
      device_entry->comPar.bpc = comPar.bpc = UART_IO_BPC_7;
      format++;
    }
    else
    {
      device_entry->comPar.bpc = comPar.bpc = UART_IO_BPC_8;
    }

    device_entry->comPar.nsb = comPar.nsb = format & 2 ? UART_IO_SB_1 : UART_IO_SB_2;
  
    if ( format & 1 )
    {
      device_entry->comPar.parity = comPar.parity = UART_IO_PA_NONE;
      parity = BS_PAR_NotPresent;
    }
    else
    {
      if ( parity EQ BS_PAR_NotPresent )
      {
        if ( device_entry->comPar.parity NEQ UART_IO_PA_NONE )
        {
          if ( device_entry->comPar.parity EQ NOT_SUPPORTED_UART_IO_PA_MARK )
          {
            comPar.parity = UART_IO_PA_NONE;
            comPar.nsb    = UART_IO_SB_2;
          }
          else
          {
            comPar.parity = device_entry->comPar.parity;
          }
        }
        else
        {
          /* the type of parity is not defined */
          return AT_FAIL; 
        }
      }
    }
  }

  switch (parity)
  {
    case BS_PAR_NotPresent:
      /* this case is handled by format translation */
      break;
    case BS_PAR_Odd:
      device_entry->comPar.parity = comPar.parity = UART_IO_PA_ODD;
      break;
    case BS_PAR_Even:
      device_entry->comPar.parity = comPar.parity = UART_IO_PA_EVEN;
      break;
    case BS_PAR_Mark: 
      /* 
       *  parity mark is not supported by the UART SAP, 
       *  but it is the same as no parity and 2 stop bit 
       */
      device_entry->comPar.parity = NOT_SUPPORTED_UART_IO_PA_MARK;
      comPar.parity               = UART_IO_PA_NONE;
      comPar.nsb                  = UART_IO_SB_2;
      break;
    case BS_PAR_Space:
      device_entry->comPar.parity = comPar.parity = UART_IO_PA_SPACE;
      break;  
  }

  psaUART_SetParameters(device_entry->device, &comPar);
  
  return AT_EXCT;
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                     |
| STATE   : code                        ROUTINE : cmhUART_SetFlowControl  |
+------------------------------------------------------------------------------+

  PURPOSE: set the UART flow control

*/
GLOBAL T_ACI_RETURN cmhUART_SetFlowControl(UBYTE srcId, T_ACI_RX_FLOW_CTRL DCE_by_DTE)
{
  T_ACI_DEVICE_ENTRY *device_entry = cmhUART_GetDeviceParOverSrcID( srcId );
  T_comPar            comPar;
  
  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhUART_SetFlowControl: device for source ID = %d not found", srcId);
    return AT_FAIL;
  }

  cmhUART_SetComParToUnchanged( &comPar );

  /*
   *  translate AT command parameter to UART parameter
   */
  switch ( DCE_by_DTE )
  {
    case   RX_FLOW_NONE:
      comPar.flow_rx = UART_IO_FC_RX_NONE;
      comPar.flow_tx = UART_IO_FC_TX_NONE;
      break;
    case   RX_FLOW_SOFTWARE:
      comPar.flow_rx = UART_IO_FC_RX_XOFF;
      comPar.flow_tx = UART_IO_FC_TX_XOFF;
      break;
    case   RX_FLOW_HARDWARE:
      comPar.flow_rx = UART_IO_FC_RX_RTS;
      comPar.flow_tx = UART_IO_FC_TX_RTS;
      break;
    case RX_FLOW_NotPresent:
      return AT_CMPL;
  }
  
  device_entry->comPar.flow_rx = comPar.flow_rx;
  device_entry->comPar.flow_tx = comPar.flow_tx;

  psaUART_SetParameters(device_entry->device, &comPar);
  
  return AT_EXCT;
}

/*
+--------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                       |
| STATE   : code                        ROUTINE : cmhUART_GetFktInterfaceFormat  |
+--------------------------------------------------------------------------------+

  PURPOSE: translate the UART COM parameter to the AT command format parameter

  ATTENTION: internal function, COM paramter is is not checking of validity

*/
GLOBAL T_ACI_BS_FRM cmhUART_GetFktInterfaceFormat( T_ACI_UART_COM_PAR *comPar )
{
  UBYTE format = 1;  /* 8 Data 2 Stop */
  
  if ( comPar->bpc EQ UART_IO_BPC_7 )
    format += 3;

  if ( comPar->nsb NEQ UART_IO_SB_2 )
    format += 2;

  if ( comPar->parity NEQ UART_IO_PA_NONE )
    format -= 1;

  return (T_ACI_BS_FRM) format;
}

/*
+--------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_UART                       |
| STATE   : code                        ROUTINE : cmhUART_GetFktInterfaceParity  |
+--------------------------------------------------------------------------------+

  PURPOSE: translate the UART COM parameter to the AT command parity parameter

  ATTENTION: internal function, COM paramter is is not checking of validity

*/
GLOBAL T_ACI_BS_PAR cmhUART_GetFktInterfaceParity( T_ACI_UART_COM_PAR *comPar )
{
  switch ( comPar->parity )
  {
    case UART_IO_PA_ODD:
      return (T_ACI_BS_PAR) BS_PAR_Odd;
    case UART_IO_PA_EVEN:
      return (T_ACI_BS_PAR) BS_PAR_Even;
    case NOT_SUPPORTED_UART_IO_PA_MARK:
      return (T_ACI_BS_PAR) BS_PAR_Mark;
    case UART_IO_PA_SPACE:
      return (T_ACI_BS_PAR) BS_PAR_Space;
  }
  return BS_PAR_NotPresent;
}
#endif /* UART */
