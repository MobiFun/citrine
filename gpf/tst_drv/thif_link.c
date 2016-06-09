/* 
+------------------------------------------------------------------------------
|  File:       thif_link.c
+------------------------------------------------------------------------------
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
|  Purpose :  This Modul contains the serial driver adaptation
+----------------------------------------------------------------------------- 
*/ 

#ifndef __THIF_LINK_C__
#define __THIF_LINK_C__
#endif

/* these types are used in thif_socket.h */
#define Char char
#define Bool8 char
#define Int8 signed char
#define Uint8 unsigned char
#define UInt16 unsigned short int 
#define Uint16 unsigned short int 
#define Uint32 unsigned long int
#define Int32  long int


#ifdef _TARGET_ 
  #include "thif_link.h"
  #include "\global_af\thif\thif_socket.h" 
  #include "string.h"
#else
 #include <windows.h>
 #include <stdio.h>
 #include "usart.h"
 #include "stdlib.h"
 #include "string.h"
 #include "tools.h"
#endif

#include "vsi.h"
#include "drvconf.h"
#include "tstheader.h"
#include "os.h"





/*==== TYPES ======================================================*/

typedef struct
{
  USHORT Handle;
  USHORT EnabledSignalType;
  T_DRV_CB_FUNC Callback;
  char Connected;
  Int8 thif_link;
} T_THIF_LINK_DATA;


typedef union {
  USHORT scalar;
  Uint8 array[2];
} SWAPPER;

/*==== CONSTANTS ==================================================*/

#define ALLOWED_THIF_LINK_SIGNALS   (DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT)
#define MAX_CONFIGSTR_LEN 50
#define NO_GOTDATA_CALLBACK 0
#define rxBufSz 0x800
#define txBufSz  0x800
#define WAIT_FOR_RECONNECT 500

/*==== EXTERNALS ==================================================*/


/*==== VARIABLES ==================================================*/

T_THIF_LINK_DATA THIF_LINK_Data;
static T_DRV_SIGNAL Signal;


Uint8 rxBuffer[rxBufSz];
Uint8 txBuffer[txBufSz];
Uint8 *prxBuffer = &rxBuffer[0];
Uint8 *ptxBuffer = &txBuffer[0];

GLOBAL OS_TIME WT;
GLOBAL OS_TIME RT;

/* static unsigned long int foolishSizeArgument = 0; */


/*==== FUNCTIONS ==================================================*/

#ifndef MIN
#  define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : swapUSHORT            |
+--------------------------------------------------------------------+

  PURPOSE : byte sex handling

*/
void swapUSHORT(USHORT* candidate)
{
  SWAPPER source;
  SWAPPER dest;
  
  source.scalar = *candidate;
  dest.array[0] = source.array[1];
  dest.array[1] = source.array[0];
  *candidate = dest.scalar;
  
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : beSureToBeConnected            |
+--------------------------------------------------------------------+

  PURPOSE : callback function of the driver

*/

void beSureToBeConnected()
{
  if (THIF_LINK_Data.thif_link < 0)
  {
    thif_link_Open();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_binding_Callback            |
+--------------------------------------------------------------------+

  PURPOSE : callback function of the driver

*/
void thif_binding_Callback (Int8 sockId) /* this is a bridge to connect the THIF binding callback to the TR-defined callback (see thif_link_Open) */
{
  if ( THIF_LINK_Data.EnabledSignalType & DRV_SIGTYPE_READ )
  {
    Signal.SignalType = DRV_SIGTYPE_READ;
    Signal.DrvHandle = THIF_LINK_Data.Handle;
    (THIF_LINK_Data.Callback)( &Signal );
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_Exit            |
+--------------------------------------------------------------------+

  PURPOSE : exit a driver

*/
void thif_link_Exit ( void )
{
  thif_link_Close();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_Open            |
+--------------------------------------------------------------------+

  PURPOSE : open connection to driver

*/
USHORT thif_link_Open(void)
{
  LONG result = -1;

  while (result < 0)
  {
    result = thif_socket_open(THIF_LINK_NAME, thif_binding_Callback, prxBuffer, rxBufSz, ptxBuffer, txBufSz );
    if (result < 0)
    {
      //os_SuspendTask ( 0, WAIT_FOR_RECONNECT);
    }
  }
  THIF_LINK_Data.thif_link = result;
  THIF_LINK_Data.EnabledSignalType = DRV_SIGTYPE_READ;

  Signal.SignalType = DRV_SIGTYPE_CONNECT;
  Signal.DrvHandle = THIF_LINK_Data.Handle;
  Signal.UserData = NULL;
  (THIF_LINK_Data.Callback)( &Signal );
  
  THIF_LINK_Data.Connected = TRUE;
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_Close            |
+--------------------------------------------------------------------+

  PURPOSE : exit a driver

*/
USHORT thif_link_Close(void )
{
  if (thif_socket_close(THIF_LINK_Data.thif_link)  < 0)
  {
    return DRV_INTERNAL_ERROR;
  }
  THIF_LINK_Data.thif_link = -1;
  THIF_LINK_Data.EnabledSignalType = DRV_SIGTYPE_DISCONNECT;

  Signal.DrvHandle = THIF_LINK_Data.Handle;
  Signal.UserData = NULL;
  (THIF_LINK_Data.Callback)( &Signal );

  THIF_LINK_Data.Connected = FALSE;
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_Read            |
+--------------------------------------------------------------------+

  PURPOSE : read data from driver

*/
USHORT thif_link_Read ( void *Buffer, ULONG *SizeInBytes)
{  

  OS_TIME timeBefore;
  OS_TIME timeAfter;
  T_HANDLE caller;
  USHORT retVal;

/*  
  ULONG to_read = *SizeInBytes;
  Uint8* buff = (Uint8*) Buffer;
  
  beSureToBeConnected();
  caller = os_MyHandle();

  if (foolishSizeArgument == 0)
  {
    to_read = thif_socket_read(THIF_LINK_Data.thif_link, buff, 2);
    foolishSizeArgument = buff[0]*0xff + buff[1];
  }
  to_read= foolishSizeArgument;

  //os_GetTime (caller,  &timeBefore);
  to_read = MIN(to_read, *SizeInBytes);
  *SizeInBytes = thif_socket_read(THIF_LINK_Data.thif_link, buff, to_read);
 
  if (*SizeInBytes <= foolishSizeArgument)
  {
    foolishSizeArgument -= *SizeInBytes;
    retVal = DRV_OK;
  }
  else
  {
    foolishSizeArgument = 0;
    retVal = DRV_INTERNAL_ERROR;
  }
  
  //os_GetTime (caller,  &timeAfter);
  //RT= timeAfter - timeBefore;
  return retVal;
*/
  *SizeInBytes = thif_socket_read( THIF_LINK_Data.thif_link, Buffer, *SizeInBytes ); 
  return DRV_OK;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_Write_to_socket           |
+--------------------------------------------------------------------+

  PURPOSE : write data to driver

*/
void thif_link_Write_to_socket (void *Buffer, ULONG sizeInBytes )
{
  T_HANDLE caller;
  Uint8* buff;

  OS_TIME timeBefore;
  OS_TIME timeAfter;
  ULONG to_write;
    
  LONG written = -1;


  buff = (Uint8*) Buffer;
  to_write = sizeInBytes;
  caller = os_MyHandle();
  while (to_write > 0)
  {
    //os_GetTime (caller,  &timeBefore);
    written = thif_socket_write(THIF_LINK_Data.thif_link, buff, to_write);
    //os_GetTime (caller,  &timeAfter);
    //WT = timeAfter - timeBefore;
    if (written < 0)
    {
      //os_SuspendTask (caller, WAIT_FOR_RECONNECT);
    }
    else if (written == 0)
    {
      os_SuspendTask (caller, 2000);
      //break;
    }
    else
    {
      to_write -= written;
      buff += written;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_Write           |
+--------------------------------------------------------------------+

  PURPOSE : relay 2 byte header & data to thif_link_Write_to_socket

*/
USHORT thif_link_Write ( void *Buffer, ULONG *SizeInBytes )
{
  USHORT TSTSocketMarker;
  Uint8* buff;

  beSureToBeConnected();

  /* TST's socket interface expects a 2-byte header containing the length of the real message  to receive
      in funtion socket_Read.  We just send that header, converting it to little endian in advance 
  TSTSocketMarker = (USHORT) *SizeInBytes;
  swapUSHORT(&TSTSocketMarker);

  buff = (Uint8*) &TSTSocketMarker;
  thif_link_Write_to_socket( buff, 2);
  */
  
  buff = (Uint8*) Buffer;
  thif_link_Write_to_socket(buff, *SizeInBytes);
  
  //thif_socket_flush(THIF_LINK_Data.thif_link);
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_Flush           |
+--------------------------------------------------------------------+

  PURPOSE : flush  data to driver

*/
USHORT thif_link_Flush (void)
{
  beSureToBeConnected();
  return thif_socket_flush(THIF_LINK_Data.thif_link);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_SetSignal       |
+--------------------------------------------------------------------+

  PURPOSE : enable signal for the driver

*/
USHORT thif_link_SetSignal ( USHORT SignalType )
{
  if ( !(SignalType & ALLOWED_THIF_LINK_SIGNALS) )
  {
    return DRV_INVALID_PARAMS;
  }
  else
  {
    THIF_LINK_Data.EnabledSignalType |= SignalType;
  }

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_ResetSignal     |
+--------------------------------------------------------------------+

  PURPOSE : disable signal for the driver

*/
USHORT thif_link_ResetSignal ( USHORT SignalType )
{
  if ( !(SignalType & ALLOWED_THIF_LINK_SIGNALS) )
  {
    return DRV_INVALID_PARAMS;
  }
  else
  {
    THIF_LINK_Data.EnabledSignalType &= ~SignalType;
  }

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_SetConfig       |
+--------------------------------------------------------------------+

  PURPOSE : set configuration for the driver

*/
USHORT thif_link_SetConfig ( char *buffer )
{
  char token [64];
  unsigned int len;
  USHORT result;

  result = DRV_OK;

  if ( (len = GetNextToken (buffer, token, " #")) == 0) 
  {
    result = thif_link_Open();
  }
  else
  {
    buffer += (len+1);
  }

  if ( !strcmp ( "OPEN", token ) )
  {
    result = thif_link_Open();
  }
  else if ( !strcmp ( "CLOSE", token ) )
  {
    result = thif_link_Close();
  }
  else if ( !strcmp ( "EXIT", token ) )
  {
    thif_link_Exit();
  }
  else
  {
    result = DRV_INVALID_PARAMS;
  }

  if ((result != DRV_OK) && (result != DRV_INVALID_PARAMS))
  {
    result = DRV_NOTCONFIGURED;
  }
  
  return  result;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : thif_link                 |
| STATE   : code                       ROUTINE : thif_link_Init            |
+--------------------------------------------------------------------+

  PURPOSE : initialize driver

*/
USHORT thif_link_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo )
{
static const T_DRV_EXPORT thif_link_Info =
{
  THIF_LINK_NAME,
  0,
  {
#ifdef _WIN32_
    thif_link_Init,
#endif
    thif_link_Exit,
    thif_link_Read,
    thif_link_Write,
    NULL, /* Look */
    NULL, /* Clear */
    thif_link_Flush,
    thif_link_SetSignal,
    thif_link_ResetSignal,
    thif_link_SetConfig,
    NULL,
    NULL,
  }
};

  THIF_LINK_Data.Handle = DrvHandle;

  THIF_LINK_Data.EnabledSignalType = 0;

  THIF_LINK_Data.Callback = CallbackFunc;

  THIF_LINK_Data.Connected = FALSE;

  *DrvInfo = &thif_link_Info;

  return DRV_OK;
}

