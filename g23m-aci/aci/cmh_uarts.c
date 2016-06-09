/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_UARTS
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

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef UART

#ifndef CMH_UARTS_C
#define CMH_UARTS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci.h"
#include "psa.h"
#include "aci_fd.h"
#include "cmh.h"

#include "aci_lst.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#include "psa_uart.h"
#include "cmh_uart.h"
#include "sap_dti.h"
#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#endif /*FF_PSI*/
#include "aci_mem.h"
#include "aci_io.h"
#include "l4_tim.h"

#include "psa_cc.h"

typedef struct      /* + IPR parameter */
{
  T_ACI_BD_RATE rate;
  T_ACI_CMD_SRC srcId;
}
T_ACI_IPR;

EXTERN T_ACI_LIST *ati_src_list;

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/
EXTERN char  *cmdErrStr;          /* Error Message            */
static T_ACI_IPR uart_ipr_param;
EXTERN CHAR  *cmdAtError (AtErrCode e);

/*==== VARIABLES ==================================================*/

GLOBAL T_ACI_LIST *uart_src_params = NULL;
GLOBAL T_ACI_UART_MUX_PARMS holdMuxParms;

/*
 *  uart_com_par: Lists all COM parameter for every device.
 *
 *  This list is only used so long as the UART SAP don't support parameter getting.
 *
 */
GLOBAL T_ACI_LIST *uart_com_par = NULL;

/*==== FUNCTIONS ==================================================*/

GLOBAL void cmhUART_lst_init (void)
{
  TRACE_FUNCTION ("cmhUART_lst_init()");
  uart_src_params = new_list ();
  /*psi_src_params = new_list();*/

  if ( !uart_com_par )
    uart_com_par = new_list ();
 /* if (!psi_dev_list)
    psi_dev_list = new_list();*/
}


GLOBAL T_ACI_BD_RATE convert_mux_port_speed (UBYTE mux_port_speed)
{
  switch (mux_port_speed)
  {
    case 1:
      return (BD_RATE_9600);

    case 2:
      return (BD_RATE_19200);

    case 3:
      return (BD_RATE_38400);

    case 4:
      return (BD_RATE_57600);

    case 5:
      return (BD_RATE_115200);

    default:
      return (BD_RATE_NotPresent);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBS                     |
| STATE   : code             ROUTINE : sAT_PlusCMUX                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMUX
            AT command which is responsible for starting multiplexing transactions.

*/
GLOBAL T_ACI_RETURN sAT_PlusCMUX ( T_ACI_CMD_SRC srcId,
                                   UBYTE mode,
                                   UBYTE subset,
                                   UBYTE port_speed,
                                   USHORT N1,
                                   UBYTE T1,
                                   UBYTE N2,
                                   UBYTE T2,
                                   UBYTE T3 )
{

#ifdef DTI
  T_ACI_DTI_PRC *src_infos = NULL;
  T_ACI_UART_MUX_PARMS *MuxParms = NULL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, (UBYTE)srcId, search_ati_src_id); 

  TRACE_FUNCTION ("sAT_PlusCMUX()");

  if(!cmh_IsVldCmdSrc (srcId))
  {
    return (AT_FAIL);
  }

  src_infos = find_element (uart_src_params, (UBYTE)srcId, cmhUARTtest_srcId);
  if (src_infos EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] sAT_PlusCMUX(): srcId=%d not found", srcId) ;
    return (AT_FAIL);
  }
 
  /*
   * AT+CMUX needs to get the answer OK before it sends the 
   * primitives to UART, because channel is then already closed.
   */ 
  io_sendConfirm ( (UBYTE)srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
  cmdErrStr = NULL;

  ACI_MALLOC( src_infos ->MuxParms, sizeof(T_ACI_UART_MUX_PARMS) );
  
  MuxParms = src_infos -> MuxParms;
  /* memorize parameters */
  MuxParms->mode       = mode;
  MuxParms->subset     = subset;
  MuxParms->port_speed = port_speed;
  MuxParms->N1         = N1;
  MuxParms->T1         = T1;
  MuxParms->N2         = N2;
  MuxParms->T2         = T2;
  MuxParms->T3         = T3;

  if ((src_infos->data_buffer.data_list EQ NULL)
    AND (aci_src_dti_params[srcId].dtxState EQ READY) )
  {
    uartEntcurCmd[srcId] = AT_CMD_NONE;
    if ((cmhUART_CMUX (srcId)) EQ FALSE)
    {
      /*
       * we already sent an OK according to GSM 27.010, but now the muxer is not ready !
       * this AT_FAIL results in a ATI_FAIL_NO_OUTPUT at function setatPlusCMUX().
       * so, what will the source do in this case ?
       */       
      TRACE_EVENT("[ERR] sAT_PlusCMUX()(): UART muxer not ready");
      return (AT_FAIL);
    }
  }
  else
  {
      /*Now we wait for the dtx state to be ready and the multiplexer will be started 
       from function sig_dti_tx_buffer_ready_ind() after the data buffer of the source is 
       emptied*/
   uartEntcurCmd[srcId] = AT_CMD_CMUX;
  }  
  return ( AT_EXCT );
#else 
  return ( AT_FAIL );
#endif /* DTI */
  
}

GLOBAL T_ACI_RETURN qAT_PlusCMUX ( T_ACI_CMD_SRC srcId,
                                   UBYTE *mode,
                                   UBYTE *subset,
                                   UBYTE *port_speed,
                                   USHORT *N1,
                                   UBYTE *T1,
                                   UBYTE *N2,
                                   UBYTE *T2,
                                   UBYTE *T3 )
{
  TRACE_FUNCTION ("qAT_PlusCMUX()");

  if ( (holdMuxParms.mode EQ 0) AND (holdMuxParms.subset EQ 0) AND
       (holdMuxParms.port_speed EQ 0) AND (holdMuxParms.N1 EQ 0) AND
       (holdMuxParms.T1 EQ 0) AND (holdMuxParms.N2 EQ 0) AND
       (holdMuxParms.T2 EQ 0) AND (holdMuxParms.T3 EQ 0) )
  {
    TRACE_FUNCTION("No Mux is enabled");
    return (AT_FAIL);
  }
  *mode       = holdMuxParms.mode;
  *subset     = holdMuxParms.subset;
  *port_speed = holdMuxParms.port_speed;
  *N1         = holdMuxParms.N1;
  *T1         = holdMuxParms.T1;
  *N2         = holdMuxParms.N2;
  *T2         = holdMuxParms.T2;
  *T3         = holdMuxParms.T3;

  return ( AT_CMPL );
}

GLOBAL BOOL cmhUART_CMUX (T_ACI_CMD_SRC srcId)
{
  T_ACI_DTI_PRC *src_infos = NULL;
  T_ACI_BD_RATE speed = BD_RATE_NotPresent;
  
  TRACE_FUNCTION ("cmhUART_CMUX()");
    
  src_infos = find_element (uart_src_params, (UBYTE)srcId, cmhUARTtest_srcId);

  if (src_infos EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] sAT_PlusCMUX(): srcId=%d not found", srcId) ;
    return (FALSE);
  }
  
  if (cmhUART_stopConnection ((UBYTE)srcId))
  {
    if (src_infos->MuxParms)
    {
      speed = convert_mux_port_speed (src_infos->MuxParms->port_speed);
      
      if ( speed NEQ BD_RATE_NotPresent )
      {
        /*
         * at first we have to change the baud rate (primitive to UART)
         */
        if (cmhUART_SetDataRate ((UBYTE)srcId, speed) EQ AT_EXCT)
        {
          return (TRUE);
        }
        else
        {
          TRACE_EVENT("[ERR] cmhUART_CMUX(): not able to change baud rate");
          return (FALSE);
        }
      }
      /*
       * void function, sends the UART_MUX_START_REQ primitive
       * return value is handled by the confirmation
       */
      psaUART_StartMux( src_infos->device,
                        src_infos->MuxParms->mode,
                        src_infos->MuxParms->subset,
                        src_infos->MuxParms->N1,
                        src_infos->MuxParms->T1,
                        src_infos->MuxParms->N2,
                        src_infos->MuxParms->T2,
                        src_infos->MuxParms->T3);
      
      memcpy( (CHAR *)&holdMuxParms, (CHAR *)src_infos->MuxParms, sizeof(T_ACI_UART_MUX_PARMS));
      ACI_MFREE( src_infos->MuxParms );
      src_infos->MuxParms = NULL;
      return (TRUE);
    }
    else   
    {
      TRACE_EVENT("[ERR] cmhUART_CMUX(): no muxer parameters available");
    }
  }
  else   
  {
    TRACE_EVENT("[ERR] cmhUART_CMUX(): not able to stop UART connection");
  }
  return (FALSE);  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DTIS                |
|                                 ROUTINE : cmhUART_getdata         |
+-------------------------------------------------------------------+

  PURPOSE : tell DTI that ACI is ready to receive more datas.
*/
GLOBAL void cmhUART_getdata ( T_ACI_CMD_SRC srcId )
{
  TRACE_FUNCTION ("cmhUART_getdata");

#ifdef RMV_01_04_03
#ifdef FF_TWO_UART_PORTS
  if (srcId NEQ UART_DATA_CHANNEL)
  {
    psaDTI_getdata ( (UBYTE)srcId, DTI_ENTITY_UART);
  }
#else
  psaDTI_getdata ( (UBYTE)srcId, DTI_ENTITY_UART);
#endif
#endif

  psaDTI_getdata ( (UBYTE)srcId, DTI_ENTITY_UART);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DTIS                |
|                                 ROUTINE : cmhUART_startConnection |
+-------------------------------------------------------------------+

  PURPOSE : start a DTI connection with UART.
*/

GLOBAL void cmhUART_startConnection (UBYTE srcId,
                                     T_ACI_DEVICE_TYPE device_type)
{
  T_ACI_DTI_PRC         *src_infos;
  T_ACI_DEVICE_ENTRY    *device_entry;
  T_ACI_DEVICE_TYPE curr_device_type;
  T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};

  TRACE_FUNCTION ("cmhUART_startConnection ()");

  src_infos = find_element (uart_src_params, (UBYTE)srcId, cmhUARTtest_srcId);
  if (src_infos EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] cmhUART_startConnection: srcId=%d not found",
                   srcId) ;
    return ;
  }

  device_entry = cmhUART_GetDeviceParOverDevice( src_infos->device );

  if ( device_entry EQ NULL )
  {
    TRACE_EVENT_P1("[ERR] cmhUART_startConnection: device=%d not found", src_infos->device) ;
    return;
  }

  curr_device_type = cmhUART_GetDeviceType (srcId);

  switch (curr_device_type)
  {
    case DEVICE_TYPE_UNKNOWN:
      /* register device to the DTI Control Manager */
#ifdef DTI
      dti_cntrl_new_device ( srcId,                             /* ATI source ID       */
                             DTI_ENTITY_UART,                   /* device ID           */
                             src_infos->device,                 /* device number       */
                             src_infos->dlci,                   /* sub-channel number  */
                             DTI_PORT_NUMBER_NOTPRESENT,        /* ATI-RIV port number */
                             DTI_CPBLTY_CMD | DTI_CPBLTY_SER,   /* capabilities        */
                             DTI_DRIVER_ID_NOTPRESENT,
                             DTI_DIO_ID_NOTPRESENT); 

#endif /* DTI */
      /* set device type */
      cmhUART_ChangeDeviceType  (srcId, device_type);
      break;

    default:
      break;
  } /* switch */

  if (device_type EQ DEVICE_TYPE_URT)
  {
    /* initialize UART */
    psaUART_SetParameters (src_infos->device, &device_entry->comPar);
    /* has to wait for confirmation before setting DTI channel */
    return;
  }
#ifdef DTI
  /* request of a DTI channel to communicate with UART */
  dti_cntrl_est_dpath_indirect ( src_infos->srcId,
                                 entity_list,
                                 1,
                                 SPLIT,
                                 atiUART_dti_cb,
                                 DTI_CPBLTY_CMD,
                                 DTI_CID_NOTPRESENT);
#endif
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DTIS                |
|                                 ROUTINE : cmhUART_stopConnection  |
+-------------------------------------------------------------------+

  PURPOSE : stop a DTI connection with UART.
*/
GLOBAL BOOL cmhUART_stopConnection (UBYTE srcId)
{
  T_ACI_DTI_PRC         *src_infos;

  TRACE_FUNCTION ("cmhUART_stopConnection()");

  src_infos = find_element (uart_src_params, (UBYTE)srcId, cmhUARTtest_srcId);
  if (src_infos EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] cmhUART_stopConnection: srcId=%d not found",
                   srcId) ;
    return (FALSE);
  }

  src_infos->LineState = LINE_DISCONNECTED;

  if (cmhUART_ChangeDeviceType  (src_infos->srcId, DEVICE_TYPE_UNKNOWN))
  {
#ifdef DTI
    if (dti_cntrl_close_dpath_from_src_id( srcId ))
    {
      return (TRUE);
    }
#endif
  }
  
  return (FALSE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UARTS                |
| STATE   : code                  ROUTINE : sAT_PlusIPR              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +IPR AT command
            which is responsible to set the data rate.
*/

GLOBAL void cmhUART_init_ipr_params(void)
{  
  TRACE_FUNCTION ("cmhUART_init_ipr_params()");
  
  uart_ipr_param.srcId = CMD_SRC_NONE;
  uart_ipr_param.rate  = BD_RATE_NotPresent;
}

GLOBAL T_ACI_RETURN sAT_PlusIPR(T_ACI_CMD_SRC srcId, T_ACI_BD_RATE rate)
{
  ULONG ipr_timer = 0;

  TRACE_FUNCTION ("sAT_PlusIPR()");

  /* check command source */
  if(srcId EQ CMD_SRC_LCL)
  {
    TRACE_ERROR("This command is not supported for local command source");
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp );
    return AT_FAIL;
  }
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return AT_FAIL;
  }

  /* parameter check */
  if ( rate < BD_RATE_AUTO OR rate > BD_RATE_115200 ) /* maximum supported value by UART */
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp );
    return AT_FAIL;
  }

  /* We need to sent the OK prior we switch to the new baudrate */
  io_sendConfirm((UBYTE)srcId, cmdAtError(atOk), ATI_NORMAL_OUTPUT);
  cmdErrStr = NULL;
  
  uart_ipr_param.rate  = rate;
  uart_ipr_param.srcId = srcId;

  ipr_timer = 600; /* worst case calculation: 4 characters:OK<CR><LF> at 75bit/s
                      with 1 startbit, 1 paritybit and 1 stopbit (2 stopbits are only used for longer words)
                      4*(8+1+1+1)= 44 bits => 44/75 seconds = 586 msec*/

  /* time for output "OK" then send changed parameter to UART */
  uartEntcurCmd[srcId] = AT_CMD_IPR;
  TIMERSTART(ipr_timer, ACI_IPR);

  return AT_CMPL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UARTS                |
| STATE   : code                  ROUTINE : timeout_PlusIPR          |
+--------------------------------------------------------------------+

  PURPOSE : This function send the changed baudrate to UART after 
            the timer ACI_IPR is expired
*/

GLOBAL void timeout_PlusIPR ( void)
{  
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *src_infos = find_element (psi_src_params, 
                               (UBYTE)uart_ipr_param.srcId, cmhPSItest_srcId);
#endif /*FF_PSI*/
  TRACE_FUNCTION ("timeout_PlusIPR()");
#ifdef FF_PSI
  if ( src_infos NEQ NULL )
  {
     cmhPSI_SetDataRate((UBYTE)uart_ipr_param.srcId, uart_ipr_param.rate);
  }
  /* force parameter request */
  else
#endif /*FF_PSI*/
     cmhUART_SetDataRate((UBYTE)uart_ipr_param.srcId, uart_ipr_param.rate);

  cmhUART_init_ipr_params();
  uartEntcurCmd[uart_ipr_param.srcId] = AT_CMD_NONE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UARTS                |
| STATE   : code                  ROUTINE : qAT_PlusIPR              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +IPR? AT command
            which returns the current setting for the data rate.

*/
GLOBAL T_ACI_RETURN qAT_PlusIPR   ( T_ACI_CMD_SRC  srcId, T_ACI_BD_RATE  *rate)
{
  T_ACI_DEVICE_ENTRY *device_entry = cmhUART_GetDeviceParOverSrcID( (UBYTE) srcId );
#ifdef FF_PSI
  T_ACI_DEVICE_DCB_ENTRY *device_dcb_entry;
  U32 device_type;/* RM 26-05-04 Nice */
#endif /*FF_PSI*/
  TRACE_FUNCTION ("qAT_PlusIPR()");

  if ( device_entry EQ NULL )
  {
#ifdef FF_PSI
     device_dcb_entry = cmhPSI_GetDeviceParOverSrcID( (UBYTE) srcId );
     if (device_dcb_entry EQ NULL)
#endif /*FF_PSI*/
        return AT_FAIL;
#ifdef FF_PSI
     else
    {
       device_type = device_dcb_entry->devId & DIO_TYPE_DAT_MASK;/* RM 26-05-04 Nice */
       if (device_type EQ DIO_DATA_SER)/* RM 26-05-04 Nice */
          *rate = (T_ACI_BD_RATE)device_dcb_entry->dcbPar.dio_dcb_ser.baudrate;
       if (device_type EQ DIO_DATA_MUX)/* RM 26-05-04 Nice */
          *rate = (T_ACI_BD_RATE)device_dcb_entry->dcbPar.dio_dcb_ser_mux.baudrate;
      else  if (device_type EQ DIO_DATA_PKT)/* RM 26-05-04 Nice */
          return AT_FAIL;
       return( AT_CMPL );
    }
#endif /*FF_PSI*/
  }
/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  *rate = (T_ACI_BD_RATE) device_entry->comPar.speed;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UARTS                |
| STATE   : code                  ROUTINE : sAT_PlusICF              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +ICF AT command
            which is responsible to set the character framing.
*/

GLOBAL T_ACI_RETURN sAT_PlusICF   ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_BS_FRM     format,
                                    T_ACI_BS_PAR     parity)
{
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *src_infos = find_element (psi_src_params,
                                 (UBYTE) srcId, cmhPSItest_srcId);
#endif /*FF_PSI*/
  TRACE_FUNCTION ("sAT_PlusICF()");

/*
 *-------------------------------------------------------------------
 * parameter check
 *-------------------------------------------------------------------
 */
  switch( format )
  {
    case( BS_FRM_NotPresent ):
    case( BS_FRM_Dat8_Par0_St2 ):
    case( BS_FRM_Dat8_Par1_St1 ):
    case( BS_FRM_Dat8_Par0_St1 ):
    case( BS_FRM_Dat7_Par0_St2 ):
    case( BS_FRM_Dat7_Par1_St1 ):
    case( BS_FRM_Dat7_Par0_St1 ):

      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  switch( parity )
  {
    case( BS_PAR_NotPresent ):
    case( BS_PAR_Odd ):
    case( BS_PAR_Even ):
    case( BS_PAR_Mark ):
    case( BS_PAR_Space ):

      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * force parameter request
 *-------------------------------------------------------------------
 */
#ifdef FF_PSI
  if ( src_infos NEQ NULL )
  {
     if ( AT_EXCT EQ cmhPSI_SetCharacterFraming((UBYTE) srcId, format, parity) )
     {
        uartEntcurCmd[srcId] = AT_CMD_ICF;
        return AT_EXCT;
     }
  }
  /* force parameter request */
  else
#endif /*FF_PSI*/
  {
    if ( AT_EXCT EQ cmhUART_SetCharacterFraming((UBYTE) srcId, format, parity) )
    {
      uartEntcurCmd[srcId] = AT_CMD_ICF;
      return AT_EXCT;
    }
  }
  return AT_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UARTS                |
| STATE   : code                  ROUTINE : qAT_PlusICF              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +ICF? AT command
            which returns the current setting for character framing.

*/

GLOBAL T_ACI_RETURN qAT_PlusICF   ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_BS_FRM    *format,
                                    T_ACI_BS_PAR    *parity)
{
  T_ACI_DEVICE_ENTRY *device_entry = 
                               cmhUART_GetDeviceParOverSrcID( (UBYTE) srcId );
#ifdef  FF_PSI
  T_ACI_DEVICE_DCB_ENTRY *device_dcb_entry = 
                               cmhPSI_GetDeviceParOverSrcID( (UBYTE) srcId );
#endif /*FF_PSI*/
  TRACE_FUNCTION ("qAT_PlusICF()");
#ifdef FF_PSI
  if (device_entry EQ NULL AND device_dcb_entry EQ NULL)
#else
  if (device_entry EQ NULL)
#endif /*FF_PSI*/
  {
      return AT_FAIL;
  }
#ifdef FF_PSI
  if ( device_entry EQ NULL AND device_dcb_entry NEQ NULL )
  {
     *format = cmhPSI_GetFktInterfaceFormat(device_dcb_entry);
     *parity = cmhPSI_GetFktInterfaceParity(device_dcb_entry);
     return( AT_CMPL );
  }
#endif /*FF_PSI*/

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  /*lint -e613 (Warning --Possible use of null pointer) */
  *format = cmhUART_GetFktInterfaceFormat(&(device_entry->comPar));
  
  *parity = cmhUART_GetFktInterfaceParity(&(device_entry->comPar));
  /*lint +e613 (Warning --Possible use of null pointer) */
  
  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UARTS                |
| STATE   : code                  ROUTINE : sAT_PlusIFC              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +IFC AT command
            which is responsible to set the flow control.
*/

GLOBAL T_ACI_RETURN sAT_PlusIFC   ( T_ACI_CMD_SRC  srcId,
                                    T_ACI_RX_FLOW_CTRL DCE_by_DTE,
                                    T_ACI_RX_FLOW_CTRL DTE_by_DCE   )
{
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *src_infos = find_element (psi_src_params, (UBYTE) srcId, cmhPSItest_srcId);
#endif /*FF_PSI*/
  T_ACI_RETURN result = AT_FAIL;
  TRACE_FUNCTION ("sAT_PlusIFC()");

/*
 *-------------------------------------------------------------------
 * parameter check
 *-------------------------------------------------------------------
 */
  TRACE_EVENT_P2("sAT_PlusIFC: DCE_by_DTE=%d DTE_by_DCE=%d", DCE_by_DTE, DTE_by_DCE);

  if ( DCE_by_DTE < RX_FLOW_NotPresent OR DCE_by_DTE > RX_FLOW_HARDWARE )
  {
    TRACE_EVENT("sAT_PlusIFC: err1");
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

  if ( DTE_by_DCE < RX_FLOW_NotPresent OR DTE_by_DCE > RX_FLOW_HARDWARE)
  {
    TRACE_EVENT("sAT_PlusIFC: err2");
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

  if ( DTE_by_DCE NEQ RX_FLOW_NotPresent AND DCE_by_DTE NEQ DTE_by_DCE )
  {
    TRACE_EVENT("sAT_PlusIFC: err3");
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

/*
 *-------------------------------------------------------------------
 * force parameter request
 *-------------------------------------------------------------------
 */
#ifdef FF_PSI
 if (src_infos NEQ NULL)
   result = cmhPSI_SetFlowControl((UBYTE) srcId, DCE_by_DTE);
 else
#endif /*FF_PSI*/
   result = cmhUART_SetFlowControl((UBYTE) srcId, DCE_by_DTE);
 switch (result)
 {
   case AT_EXCT:
     uartEntcurCmd[srcId] = AT_CMD_IFC;
     return AT_EXCT;
   case AT_CMPL:
     return AT_CMPL;
 }
 return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UARTS                |
| STATE   : code                  ROUTINE : qAT_PlusIFC              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +ICF? AT command
            which returns the current setting for character framing.

*/

GLOBAL T_ACI_RETURN qAT_PlusIFC   ( T_ACI_CMD_SRC       srcId,
                                    T_ACI_RX_FLOW_CTRL *DCE_by_DTE,
                                    T_ACI_RX_FLOW_CTRL *DTE_by_DCE   )
{
  S32 flow_cntrl = RX_FLOW_NotPresent;
  T_ACI_DEVICE_ENTRY *device_entry = cmhUART_GetDeviceParOverSrcID( (UBYTE) srcId );
#ifdef FF_PSI
  T_ACI_DEVICE_DCB_ENTRY *device_dcb_entry = NULL;
  U32 device_type;/* RM 26-05-04 Nice */
#endif /*FF_PSI*/
  TRACE_FUNCTION ("qAT_PlusIFC()");

  if ( device_entry EQ NULL )
  {
#ifdef FF_PSI
    if ( (device_dcb_entry = cmhPSI_GetDeviceParOverSrcID( (UBYTE) srcId )) EQ NULL)
#endif /*FF_PSI*/
      return AT_FAIL;
#ifdef FF_PSI
    device_type = device_dcb_entry->devId & DIO_TYPE_DAT_MASK;/*  RM 26-05-04 Nice */
    if (device_type EQ DIO_DATA_PKT)/* RM 26-05-04 Nice */
      return AT_FAIL;
   if (device_type EQ DIO_DATA_SER)/* RM 26-05-04 Nice */
      flow_cntrl = device_dcb_entry->dcbPar.dio_dcb_ser.flow_control;
   if (device_type EQ DIO_DATA_MUX)/* RM 26-05-04 Nice */
      flow_cntrl = device_dcb_entry->dcbPar.dio_dcb_ser_mux.flow_control;

    switch ( flow_cntrl )
    {
       case   (DIO_FLOW_NONE):
       default:
         *DCE_by_DTE = *DTE_by_DCE = RX_FLOW_NONE;
         break;
       case   (DIO_FLOW_XON_XOFF):
         *DCE_by_DTE = *DTE_by_DCE = RX_FLOW_SOFTWARE;
         break;
       case  ( DIO_FLOW_RTS_CTS):
         *DCE_by_DTE = *DTE_by_DCE = RX_FLOW_HARDWARE;
         break;
    }
    return( AT_CMPL );
#endif /*FF_PSI*/
  }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  switch ( device_entry->comPar.flow_rx )
  {
    case UART_IO_FC_RX_NONE:
    default:
      *DCE_by_DTE = *DTE_by_DCE = RX_FLOW_NONE;
      break;
    case UART_IO_FC_RX_XOFF:
      *DCE_by_DTE = *DTE_by_DCE = RX_FLOW_SOFTWARE;
      break;
    case UART_IO_FC_RX_RTS:
      *DCE_by_DTE = *DTE_by_DCE = RX_FLOW_HARDWARE;
      break;
  }

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCS                  |
| STATE   : code                  ROUTINE : sAT_AndC                 |
+--------------------------------------------------------------------+

  PURPOSE : set DCD behaviour
*/

GLOBAL T_ACI_RETURN sAT_AndC      ( T_ACI_CMD_SRC srcId,
                                    T_ACI_DCD_MOD value )
{
  BOOL  no_data_call_on_this_line = TRUE;
  UBYTE i;

  TRACE_FUNCTION ("sAT_AndC()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* check parameter value */
  switch(value)
  {
    case(DCD_ALWAYS_ON):
    case(DCD_DISABLE_AFTER_CALL):
      break;
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  /* set DCD behaviour */
  cmhPrm[srcId ].ccCmdPrm.DCDmode = value;

  /* set DCD */
  if(value EQ DCD_ALWAYS_ON)
  {
    io_setDCD (srcId, IO_DCD_ON);
  }
  else if(value EQ DCD_DISABLE_AFTER_CALL)
  {
    for (i = 0; i < MAX_CALL_NR; i++)
    {
      T_CC_CALL_TBL *ctbx = ccShrdPrm.ctb[i];

      if (ctbx NEQ NULL AND
          ctbx->curSrc EQ srcId)
      {
        switch(ctbx->BC[ctbx->curBC].bearer_serv)
        {
          case(MNCC_BEARER_SERV_NOT_PRES):
          case(MNCC_BEARER_SERV_SPEECH):
          case(MNCC_BEARER_SERV_AUX_SPEECH):
            break;
          default:
            /* data call */
            no_data_call_on_this_line = FALSE;
            break;
        }
      }
    }

    if( no_data_call_on_this_line )
    {
      io_setDCD (srcId, IO_DCD_OFF);
    }
  }
  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_AndC                 |
+--------------------------------------------------------------------+

  PURPOSE : query DCD behaviour setting.
*/

GLOBAL T_ACI_RETURN qAT_AndC      ( T_ACI_CMD_SRC srcId,
                                    T_ACI_DCD_MOD *value)
{

  TRACE_FUNCTION ("qAT_AndC()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* check parameter value */
  if( value EQ NULL )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  /* query DCD behaviour */
  *value = cmhPrm[srcId ].ccCmdPrm.DCDmode;

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UARTS                |
| STATE   : code                  ROUTINE : sAT_AndD                 |
+--------------------------------------------------------------------+

  PURPOSE : set DTR behaviour.
*/

GLOBAL T_ACI_RETURN sAT_AndD ( T_ACI_CMD_SRC srcId, UBYTE value)
{
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *dti_prc=find_element (psi_src_params, 
                                         (UBYTE)srcId, cmhPSItest_srcId);
#endif /*FF_PSI*/
  TRACE_FUNCTION ("sAT_AndD()");
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  switch( value )
  {
    case( DTR_BEHAVIOUR_Ignore ):
    case( DTR_BEHAVIOUR_CommandMode ):
    case( DTR_BEHAVIOUR_ClearCall ):
#ifdef FF_PSI
      if (dti_prc NEQ NULL)
      {
        psiShrdPrm.dtr_behaviour = (T_ACI_DTR_BEHAVIOUR)value;
        TRACE_EVENT("device entry is in PSI");
      }
      else
#endif /*FF_PSI*/
      {
        uartShrdPrm.dtr_behaviour = (T_ACI_DTR_BEHAVIOUR)value;
        TRACE_EVENT("device entry is in UART");
      }
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }
  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UARTS                |
| STATE   : code                  ROUTINE : qAT_AndD                 |
+--------------------------------------------------------------------+

  PURPOSE : query DTR behaviour.
*/

GLOBAL T_ACI_RETURN qAT_AndD ( T_ACI_CMD_SRC srcId, UBYTE *value)
{
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *dti_prc=find_element (psi_src_params, (UBYTE)srcId, cmhPSItest_srcId);
#endif /*FF_PSI*/
  T_ACI_DTR_BEHAVIOUR dtr_behavior;
  TRACE_FUNCTION ("qAT_AndD()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }
#ifdef FF_PSI
  if (dti_prc NEQ NULL)
    dtr_behavior = psiShrdPrm.dtr_behaviour;
  else 
#endif /*FF_PSI*/
    dtr_behavior = uartShrdPrm.dtr_behaviour;
  switch( dtr_behavior )
  {
    case( DTR_BEHAVIOUR_Ignore ):
    case( DTR_BEHAVIOUR_CommandMode ):
    case( DTR_BEHAVIOUR_ClearCall ):
      *value = dtr_behavior;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }
  return( AT_CMPL );
}

GLOBAL void cmhUART_lst_exit(void)
{
  T_ACI_LIST *src_infos = NULL;

  while (1)
  {
    src_infos = (T_ACI_LIST *)remove_first_element (uart_src_params);

    if (src_infos EQ NULL)
    {
      break;
    }
    ACI_MFREE (src_infos);
    
  }
}
#endif /* ifdef UART */
