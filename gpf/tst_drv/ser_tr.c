/* 
+------------------------------------------------------------------------------
|  File:       ser_tr.c
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

#ifndef __SER_TR_C__
#define __SER_TR_C__
#endif

#include "gpfconf.h"
#include "typedefs.h"
#include "../../serial/traceswitch.h"
#include "../../serial/serialswitch.h"
#include "gdi.h"
#include "tstheader.h"

/*==== TYPES ======================================================*/

typedef struct
{
  USHORT Handle;
  USHORT EnabledSignalType;
  T_DRV_CB_FUNC Callback;
  char Connected;
} T_SER_TR_DATA;

/*==== CONSTANTS ==================================================*/

#define ALLOWED_SER_TR_SIGNALS   (DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT)

/*==== EXTERNALS ==================================================*/

#ifdef RUN_FLASH
 USHORT SER_TR_Write ( void *Buffer, ULONG *BytesToWrite );
#endif

/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM
T_SER_TR_DATA SER_TR_Data;
#endif
static T_DRV_SIGNAL Signal;

/*==== FUNCTIONS ==================================================*/

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : Callback            |
+--------------------------------------------------------------------+

  PURPOSE : callback function of the driver

*/
void Callback ( void )
{
  if ( SER_TR_Data.EnabledSignalType & DRV_SIGTYPE_READ )
  {
    Signal.SignalType = DRV_SIGTYPE_READ;
    Signal.DrvHandle = SER_TR_Data.Handle;

    (SER_TR_Data.Callback)( &Signal );
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_TR_Exit         |
+--------------------------------------------------------------------+

  PURPOSE : exit a driver

*/
void SER_TR_Exit ( void )
{
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_TR_Read         |
+--------------------------------------------------------------------+

  PURPOSE : read data from driver

*/
USHORT SER_TR_Read ( void *Buffer, ULONG *BytesToRead )
{
  *BytesToRead = (USHORT)SER_tr_ReadNChars ( SER_LAYER_1, Buffer, *BytesToRead );
  return DRV_OK;
}
#endif


#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_TR_Write        |
+--------------------------------------------------------------------+

  PURPOSE : write data to driver

*/
USHORT SER_TR_Write ( void *Buffer, ULONG *BytesToWrite )
{
ULONG ToWrite = *BytesToWrite & ~PRIM_FLAG_MASK;
ULONG BytesWritten;
char *ptr = (char*)Buffer;

  ptr--;
  *ptr = 0x13;
  ToWrite += 1;
  BytesWritten = (ULONG)SER_tr_WriteNBytes (SER_LAYER_1, (SYS_UWORD8*)ptr, (SYS_UWORD32)ToWrite);
  *BytesToWrite = BytesWritten;

  return ( DRV_OK );
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_TR_SetSignal    |
+--------------------------------------------------------------------+

  PURPOSE : enable signal for the driver

*/
USHORT SER_TR_SetSignal ( USHORT SignalType )
{
  if ( !(SignalType & ALLOWED_SER_TR_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    SER_TR_Data.EnabledSignalType |= SignalType;

  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_TR_ResetSignal  |
+--------------------------------------------------------------------+

  PURPOSE : disable signal for the driver

*/
USHORT SER_TR_ResetSignal ( USHORT SignalType )
{
  if ( !(SignalType & ALLOWED_SER_TR_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    SER_TR_Data.EnabledSignalType &= ~SignalType;

  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_TR_SetConfig    |
+--------------------------------------------------------------------+

  PURPOSE : set configuration for the driver

*/
USHORT SER_TR_SetConfig ( char *Buffer )
{

  if ( !SER_TR_Data.Connected )
  {
    Signal.SignalType = DRV_SIGTYPE_CONNECT;
    Signal.DrvHandle = SER_TR_Data.Handle;
    Signal.UserData = NULL;
    (SER_TR_Data.Callback)( &Signal );
    SER_TR_Data.Connected = TRUE;
    return DRV_OK;
  }
  return DRV_OK;
}
#endif


#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_TR_Init         |
+--------------------------------------------------------------------+

  PURPOSE : initialize driver

*/
USHORT SER_TR_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo )
{
static const T_DRV_EXPORT SER_TR_Info =
{
  "SER",
  CALLED_FROM_ISR,
  {
#ifdef _TOOLS_
    SER_TR_TR_Init,
#endif
    SER_TR_Exit,
    SER_TR_Read,
    SER_TR_Write,
    NULL,
    NULL,
    NULL,
    SER_TR_SetSignal,
    SER_TR_ResetSignal,
    SER_TR_SetConfig,
    NULL,
    NULL,
  }
};

  SER_TR_Data.Handle = DrvHandle;
  SER_TR_Data.EnabledSignalType = 0;
  SER_TR_Data.Callback = CallbackFunc;
  SER_TR_Data.Connected = FALSE;

  SER_tr_Init ( SER_LAYER_1, TR_BAUD_115200, Callback ); 
  *DrvInfo = &SER_TR_Info;

  return DRV_OK;
}
#endif
