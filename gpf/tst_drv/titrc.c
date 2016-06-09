/* 
+------------------------------------------------------------------------------
|  File:       titrc.c
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
|  Purpose :  This Module defines the functionality of the TITRC driver
|             to communicate with the TI trace task.
+----------------------------------------------------------------------------- 
*/ 


#ifndef __TITRC_C__
#define __TITRC_C__
#endif

#include <string.h>
#include "gpfconf.h"
#include "typedefs.h"
#include "tstheader.h"
#include "../../riviera/rvt/rvt_gen.h"
#include "gdi.h"
#include "../../serial/serialswitch.h"

/*==== TYPES ======================================================*/

typedef struct
{
  USHORT Handle;
  USHORT EnabledSignalType;
  T_DRV_CB_FUNC Callback;
  T_RVT_USER_ID trc_id;
  unsigned char connected;
  T_RVT_BUFFER read_ptr;
  unsigned int read_len;
} T_TITRC_DATA;

/*==== CONSTANTS ==================================================*/

#define ALLOWED_TITRC_SIGNALS   (DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT) 


/*==== EXTERNALS ==================================================*/

extern USHORT emergeny_trace;

/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM
T_TITRC_DATA TITRC_Data;
T_DRV_SIGNAL Signal;
#else
extern T_TITRC_DATA TITRC_Data;
#endif

#ifndef RUN_FLASH
char header_buffer[TST_BIG_HEADER_SIZE];
#endif

/*==== FUNCTIONS ==================================================*/

#ifndef RUN_INT_RAM
extern USHORT TITRC_Write ( void *Buffer, ULONG *BytesToWrite ) ;
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TITRC               |
| STATE   : code                       ROUTINE : TITRC_Read          |
+--------------------------------------------------------------------+

  PURPOSE : read data from driver

*/
USHORT TITRC_Read ( void *Buffer, ULONG *BytesToRead )
{

  if ( TITRC_Data.read_ptr )
  {
    if ( *BytesToRead != 0 )
    {
      if ( *BytesToRead > TITRC_Data.read_len )
        *BytesToRead = TITRC_Data.read_len;

      memcpy ( Buffer, TITRC_Data.read_ptr, *BytesToRead );

      if ( *BytesToRead < TITRC_Data.read_len )
      {
        TITRC_Data.read_ptr += *BytesToRead;
        TITRC_Data.read_len -= *BytesToRead;
      }
      else
        TITRC_Data.read_ptr = NULL;
    }
    else
    {
      *BytesToRead = TITRC_Data.read_len;
    }
  }
  else
  {
    *BytesToRead = 0;
  }

  return DRV_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TITRC               |
| STATE   : code                       ROUTINE : TITRC_Write         |
+--------------------------------------------------------------------+

  PURPOSE : write data to driver

*/
USHORT TITRC_Write ( void *Buffer, ULONG *BytesToWrite )
{
T_RVT_FORMAT msg_format;
char *ptr = (char*)Buffer;
ULONG BytesWritten;
ULONG ToWrite;
T_RVT_BUFFER rvt_buffer;

  msg_format = RVT_BINARY_FORMAT;

  if ( emergeny_trace )
  {
    ptr--;
    *ptr = 0x13;
    ToWrite = (ULONG)*BytesToWrite;
    ToWrite += 1;
    BytesWritten = (ULONG)SER_tr_WriteNBytes (1, (SYS_UWORD8*)ptr, (SYS_UWORD32)ToWrite);
    *BytesToWrite = BytesWritten;
  }
  else
  {
    if ( *BytesToWrite & PRIM_HEADER_FLAG )
    {
      memcpy ( header_buffer, (char*)Buffer, TST_BIG_HEADER_SIZE );
      return DRV_OK;
    }
    if ( *BytesToWrite & PRIM_DATA_FLAG )
    {
      ToWrite = *BytesToWrite & ~PRIM_DATA_FLAG;
      if ( rvt_mem_alloc(TITRC_Data.trc_id, ToWrite+TST_BIG_HEADER_SIZE, &rvt_buffer) != RVT_OK )
        return DRV_BUFFER_FULL;
      if ( rvt_buffer != NULL )
      {
        memcpy ( (char*)rvt_buffer, header_buffer, TST_BIG_HEADER_SIZE );
        memcpy ( (char*)rvt_buffer+TST_BIG_HEADER_SIZE, (char*)Buffer, ToWrite );
        if ( rvt_send_trace_no_cpy ( rvt_buffer, TITRC_Data.trc_id, (T_RVT_MSG_LG)ToWrite+TST_BIG_HEADER_SIZE, msg_format ) != RVT_OK )
          return DRV_BUFFER_FULL;
        else
          return DRV_OK;
      }
    }
    if ( rvt_send_trace_cpy ( (T_RVT_BUFFER)Buffer, TITRC_Data.trc_id, (T_RVT_MSG_LG)*BytesToWrite, msg_format ) != RVT_OK )
      return DRV_BUFFER_FULL;
  }
  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TITRC               |
| STATE   : code                       ROUTINE : TITRC_SetSignal     |
+--------------------------------------------------------------------+

  PURPOSE : enable signal for the driver

*/
USHORT TITRC_SetSignal ( USHORT SignalType )
{
  if ( !(SignalType & ALLOWED_TITRC_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    TITRC_Data.EnabledSignalType |= SignalType;
  
  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TITRC               |
| STATE   : code                       ROUTINE : TITRC_ResetSignal   |
+--------------------------------------------------------------------+

  PURPOSE : disable signal for the driver

*/
USHORT TITRC_ResetSignal ( USHORT SignalType )
{
  if ( !(SignalType & ALLOWED_TITRC_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    TITRC_Data.EnabledSignalType &= ~SignalType;
  
  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TITRC               |
| STATE   : code                       ROUTINE : TITRC_SetConfig     |
+--------------------------------------------------------------------+

  PURPOSE : set configuration for the driver

*/
USHORT TITRC_SetConfig ( char *Buffer )
{

  if ( TITRC_Data.connected == FALSE )
  {
    Signal.SignalType = DRV_SIGTYPE_CONNECT;
    Signal.DrvHandle = TITRC_Data.Handle;
    (TITRC_Data.Callback)( &Signal );
    TITRC_Data.connected = TRUE;
  }
  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TITRC               |
| STATE   : code                       ROUTINE : TITRC_Callback      |
+--------------------------------------------------------------------+

  PURPOSE : callback function of the driver

*/
int callback_busy = 0;
void TITRC_Callback ( T_RVT_BUFFER ptr, UINT16 len )
{
  if ( 	callback_busy == 1 )
    for (;;)
      ;

  callback_busy = 1;
  if ( TITRC_Data.EnabledSignalType & DRV_SIGTYPE_READ )
  {
    TITRC_Data.read_ptr = ptr;
    TITRC_Data.read_len = len;
    Signal.SignalType = DRV_SIGTYPE_READ;
    Signal.DrvHandle = TITRC_Data.Handle;
    (TITRC_Data.Callback)( &Signal );
  }
  callback_busy = 0;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TITRC               |
| STATE   : code                       ROUTINE : TITRC_Init          |
+--------------------------------------------------------------------+

  PURPOSE : initialize driver

*/
USHORT TITRC_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo )
{                
static const T_DRV_EXPORT TITRC_Info =
{
  "TITRC",
  0,
  {
#ifdef _TOOLS_
    TITRC_Init,
#endif
    NULL,
    TITRC_Read,
    TITRC_Write,
    NULL,
    NULL,
    NULL,
    TITRC_SetSignal,
    TITRC_ResetSignal,
    TITRC_SetConfig,
    NULL,
    NULL,
  }
};

  TITRC_Data.connected = FALSE;
    
  TITRC_Data.Handle = DrvHandle;

  TITRC_Data.EnabledSignalType = 0;

  TITRC_Data.Callback = CallbackFunc;

  if ( rvt_register_id ( (char*)"L23",&TITRC_Data.trc_id, TITRC_Callback ) == RVT_INVALID_PARAMETER )
    return DRV_INITFAILURE;

  *DrvInfo = &TITRC_Info;

  return DRV_OK;           
}
#endif
