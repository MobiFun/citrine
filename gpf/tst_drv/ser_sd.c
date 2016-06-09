/* 
+------------------------------------------------------------------------------
|  File:       ser_sd.c
+------------------------------------------------------------------------------
|  Copyright 2004 Texas Instruments Deutschland, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Deutschland, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  This Modul contains the serial driver adaptation
+----------------------------------------------------------------------------- 
*/ 

#ifndef __SER_SD_C__
#define __SER_SD_C__
#endif

#include "typedefs.h"
#include "stack1_serial.h"
#include "gdi.h"
#include "tstheader.h"

/*==== TYPES ======================================================*/

typedef struct
{
  USHORT Handle;
  USHORT EnabledSignalType;
  T_DRV_CB_FUNC Callback;
  char Connected;
} T_SER_SD_DATA;

/*==== CONSTANTS ==================================================*/

#define ALLOWED_SER_SD_SIGNALS   (DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT)

/*==== EXTERNALS ==================================================*/


/*==== VARIABLES ==================================================*/

T_SER_SD_DATA SER_SD_Data;
static T_DRV_SIGNAL Signal;

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : Callback            |
+--------------------------------------------------------------------+

  PURPOSE : callback function of the driver

*/
void Callback ( void )
{
	if ( SER_SD_Data.EnabledSignalType & DRV_SIGTYPE_READ )
  {
    Signal.SignalType = DRV_SIGTYPE_READ;
	  Signal.DrvHandle = SER_SD_Data.Handle;

    (SER_SD_Data.Callback)( &Signal );
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_SD_Exit         |
+--------------------------------------------------------------------+

  PURPOSE : exit a driver

*/
void SER_SD_Exit ( void )
{
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_SD_Read         |
+--------------------------------------------------------------------+

  PURPOSE : read data from driver

*/
USHORT SER_SD_Read ( void *Buffer, ULONG *BytesToRead )
{
  *BytesToRead = stack1_Serial_receiveData ( (unsigned char*)Buffer, *BytesToRead ); 
  return DRV_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_SD_Write        |
+--------------------------------------------------------------------+

  PURPOSE : write data to driver

*/
USHORT SER_SD_Write ( void *Buffer, ULONG *BytesToWrite )
{
ULONG ToWrite = *BytesToWrite & ~PRIM_FLAG_MASK;

   /* stack1_Serial_sendData() returns TRUE/FALSE but not the number of written bytes */
  stack1_Serial_sendData( (unsigned char*)Buffer, ToWrite );
  return ( DRV_OK );
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_SD_SetSignal    |
+--------------------------------------------------------------------+

  PURPOSE : enable signal for the driver

*/
USHORT SER_SD_SetSignal ( USHORT SignalType )
{
	if ( !(SignalType & ALLOWED_SER_SD_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    SER_SD_Data.EnabledSignalType |= SignalType;

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_SD_ResetSignal  |
+--------------------------------------------------------------------+

  PURPOSE : disable signal for the driver

*/
USHORT SER_SD_ResetSignal ( USHORT SignalType )
{
	if ( !(SignalType & ALLOWED_SER_SD_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    SER_SD_Data.EnabledSignalType &= ~SignalType;

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_SD_SetConfig    |
+--------------------------------------------------------------------+

  PURPOSE : set configuration for the driver

*/
USHORT SER_SD_SetConfig ( char *Buffer )
{

  if ( !SER_SD_Data.Connected )
  {
    Signal.SignalType = DRV_SIGTYPE_CONNECT;
	  Signal.DrvHandle = SER_SD_Data.Handle;
    Signal.UserData = NULL;
    (SER_SD_Data.Callback)( &Signal );
    SER_SD_Data.Connected = TRUE;
    return DRV_OK;
  }
  return DRV_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_SD_Init         |
+--------------------------------------------------------------------+

  PURPOSE : initialize driver

*/
USHORT SER_SD_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo )
{
static const T_DRV_EXPORT SER_SD_Info =
{
  "SER",
  CALLED_FROM_ISR,
  {
#ifdef _TOOLS_
    SER_SD_Init,
#endif
    SER_SD_Exit,
    SER_SD_Read,
    SER_SD_Write,
    NULL,
    NULL,
    NULL,
    SER_SD_SetSignal,
    SER_SD_ResetSignal,
    SER_SD_SetConfig,
    NULL,
    NULL,
  }
};

  SER_SD_Data.Handle = DrvHandle;
  SER_SD_Data.EnabledSignalType = 0;
  SER_SD_Data.Callback = CallbackFunc;
  SER_SD_Data.Connected = FALSE;
   
  stack1_Serial_PowerUp ( Callback );

  *DrvInfo = &SER_SD_Info;

  return DRV_OK;
}

