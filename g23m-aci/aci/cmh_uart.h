/*
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_UART
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

#ifndef CMH_UART_H
#define CMH_UART_H


EXTERN T_ACI_LIST *uart_src_params;
EXTERN T_ACI_LIST *uart_com_par;

/*==== CONSTANTS ==================================================*/
#define NOT_SUPPORTED_UART_IO_PA_MARK 4 /* the UART does not support parity mark directly */

/*==== TYPES ======================================================*/

typedef struct
{
  UBYTE         *data;
  USHORT        data_len;
  T_ACI_CMD_SRC srcId;
} T_ACI_DTI_SEND;

typedef T_comPar T_ACI_UART_COM_PAR;
typedef struct {
  UBYTE               device;
  T_ACI_UART_COM_PAR  comPar;
  
} T_ACI_DEVICE_ENTRY;



/*==== PROTOTYPES =================================================*/

/*==== EXPORT =====================================================*/
EXTERN void           timeout_PlusIPR (void);

EXTERN BOOL           cmhUART_CMUX (T_ACI_CMD_SRC srcId);

EXTERN void           cmhUART_lst_init (void);
EXTERN BOOL           cmhUARTtest_srcId (UBYTE srcId,
                                         void *elem);

EXTERN BOOL           cmhUARTtest_device (UBYTE device,
                                          void *elem);

EXTERN T_ACI_DTI_PRC *cmhUART_find_dlci (T_ACI_LIST *search_list,
                                         UBYTE device,
                                         UBYTE dlci );
EXTERN void cmhUART_startConnection (UBYTE srcId, 
                                      T_ACI_DEVICE_TYPE device_type);

EXTERN BOOL cmhUART_stopConnection (UBYTE srcId);
EXTERN void cmhUART_getdata ( T_ACI_CMD_SRC srcId );

EXTERN void cmhUART_ParmsUpdated( UBYTE device );
EXTERN void cmhUART_DetectedESC_DTR( UBYTE device, UBYTE dlci, UBYTE cause );
EXTERN void cmhUART_init_ipr_params(void);

/*
 *  functions for cotroling UART parameters (baud rate, character framing, flow control)
 */
EXTERN void cmhUART_AddDeviceToComParameter( UBYTE device );
EXTERN void cmhUART_CleanComParameterList( void );

EXTERN T_ACI_DEVICE_TYPE cmhUART_GetDeviceType( UBYTE srcId );
EXTERN BOOL cmhUART_ChangeDeviceType( UBYTE             srcId, 
                                      T_ACI_DEVICE_TYPE device_type );
EXTERN T_ACI_DEVICE_ENTRY* cmhUART_GetDeviceParOverDevice( UBYTE device );
EXTERN T_ACI_DEVICE_ENTRY* cmhUART_GetDeviceParOverSrcID( UBYTE srcId );
EXTERN void cmhUART_SetComParToUnchanged( T_comPar *comPar );
EXTERN T_ACI_RETURN cmhUART_SetDataRate ( UBYTE srcId, T_ACI_BD_RATE  rate);
EXTERN T_ACI_RETURN cmhUART_SetCharacterFraming ( UBYTE srcId, 
                                                  T_ACI_BS_FRM format, 
                                                  T_ACI_BS_PAR parity );
EXTERN T_ACI_RETURN cmhUART_SetFlowControl(UBYTE srcId, T_ACI_RX_FLOW_CTRL DCE_by_DTE);
EXTERN T_ACI_BS_FRM cmhUART_GetFktInterfaceFormat( T_ACI_UART_COM_PAR *comPar );
EXTERN T_ACI_BS_PAR cmhUART_GetFktInterfaceParity( T_ACI_UART_COM_PAR *comPar );

EXTERN UBYTE cmhUART_GetParityOverSrcID( UBYTE srcId );
EXTERN UBYTE cmhUART_GetDataBitOverSrcID( UBYTE srcId );
EXTERN UBYTE cmhUART_GetStopBitOverSrcID( UBYTE srcId );
EXTERN void  cmhUART_lst_exit (void);


#ifdef CMH_UARTF_C

GLOBAL T_ACI_AT_CMD uartEntcurCmd[CMD_SRC_MAX];

#else

EXTERN T_ACI_AT_CMD uartEntcurCmd[];

#endif /* CMH_UARTF_C */

#endif /* CMH_UART_H */

/*==== EOF =======================================================*/
