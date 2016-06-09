/* 
+------------------------------------------------------------------------------
|  File:       ser.c
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

#ifndef __SER_C__
#define __SER_C__
#endif

#include <windows.h>
#include <stdio.h>
#include "usart.h"
#include "stdlib.h"
#include "string.h"
#include "tools.h"
#include "typedefs.h"
#include "tstheader.h"
#include "os.h"
#include "gdi.h"

/*==== TYPES ======================================================*/

typedef struct
{
  USHORT Handle;
  USHORT EnabledSignalType;
  T_DRV_CB_FUNC Callback;
#ifndef _TARGET_
  OS_HANDLE TaskHandle;
#endif
  char Connected;
} T_SER_DATA;

/*==== CONSTANTS ==================================================*/

#define ALLOWED_SER_SIGNALS   (DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT)
#define MAX_CONFIGSTR_LEN 50

/*==== EXTERNALS ==================================================*/

#ifndef _TARGET_
extern OS_HANDLE ext_data_pool_handle;
#endif

/*==== VARIABLES ==================================================*/

T_SER_DATA SER_Data;
static T_DRV_SIGNAL Signal;
GLOBAL int            extPort     = 1;

#ifndef _TARGET_
  static char m_last_config[MAX_CONFIGSTR_LEN+1];
#endif

#ifdef _TOOLS_
  OS_TIME extr_creation_time = 0;
#endif

/*==== FUNCTIONS ==================================================*/

#ifndef RUN_INT_RAM
#ifdef _TOOLS_
OS_TIME tst_get_init_time ( void )
{
  return extr_creation_time;
}
#endif
#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : Callback            |
+--------------------------------------------------------------------+

  PURPOSE : callback function of the driver

*/
void Callback ( void )
{
	if ( SER_Data.EnabledSignalType & DRV_SIGTYPE_READ )
  {
    Signal.SignalType = DRV_SIGTYPE_READ;
	  Signal.DrvHandle = SER_Data.Handle;

    (SER_Data.Callback)( &Signal );
  }
}

#ifndef _TARGET_

GLOBAL void SER_Receiver (void)
{

  for(;;)
  {
    UT_IsChar ();
    Callback();
  }
}

#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_Exit            |
+--------------------------------------------------------------------+

  PURPOSE : exit a driver

*/
void SER_Exit ( void )
{
  os_DestroyTask( 0, SER_Data.TaskHandle );
  SER_Data.TaskHandle = OS_ERROR;
  UT_Close();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_Read            |
+--------------------------------------------------------------------+

  PURPOSE : read data from driver

*/
USHORT SER_Read ( void *Buffer, ULONG *BytesToRead )
{
  *BytesToRead = (USHORT)UT_ReadNChars (UT_DEVICE_0, (BYTE *)Buffer, *BytesToRead);
  return DRV_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_Write           |
+--------------------------------------------------------------------+

  PURPOSE : write data to driver

*/
USHORT SER_Write ( void *Buffer, ULONG *BytesToWrite )
{
ULONG ToWrite = *BytesToWrite & ~PRIM_FLAG_MASK;

  UT_WriteNChars(UT_DEVICE_0, (void*)Buffer, ToWrite );
  return ( DRV_OK );
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_SetSignal       |
+--------------------------------------------------------------------+

  PURPOSE : enable signal for the driver

*/
USHORT SER_SetSignal ( USHORT SignalType )
{
	if ( !(SignalType & ALLOWED_SER_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    SER_Data.EnabledSignalType |= SignalType;

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_ResetSignal     |
+--------------------------------------------------------------------+

  PURPOSE : disable signal for the driver

*/
USHORT SER_ResetSignal ( USHORT SignalType )
{
	if ( !(SignalType & ALLOWED_SER_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    SER_Data.EnabledSignalType &= ~SignalType;

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_SetConfig       |
+--------------------------------------------------------------------+

  PURPOSE : set configuration for the driver

*/
USHORT SER_SetConfig ( char *Buffer )
{
/*lint -e813, suppress Info 813: auto variable 'osver' has size '148' */ 
char token [MAX_PATH+1];
/*lint +e813 */
unsigned int baudrate=0;
int mode;
char flow_ctrl='N';
unsigned int len;
const char *fname=NULL;

  if ( !SER_Data.Connected )
  {
    Signal.SignalType = DRV_SIGTYPE_CONNECT;
	  Signal.DrvHandle = SER_Data.Handle;
    Signal.UserData = NULL;
    (SER_Data.Callback)( &Signal );
    SER_Data.Connected = TRUE;
    extPort=0;
    return DRV_OK;
  }

  if (strlen(m_last_config))
  {
    /* check if we are already configured right */
    if (strcmp(m_last_config,Buffer)==0)
    {
#ifdef _DEBUG
      fprintf(stdout,"SER: keeping configuration\n");
#endif
      return DRV_OK;
    }
  }

  if (!(strcmp(Buffer,"EXIT")))
  {
    SER_Exit();
    return DRV_OK;
  }
  /* save configuration string */
  strncpy(m_last_config,Buffer,MAX_CONFIGSTR_LEN);

  /* interprete configuration string */
  if ( (len = GetNextToken (Buffer, token, " #")) == 0 )
    return DRV_INVALID_PARAMS;
  else
    Buffer += (len+1);

  /* set mode */
  if (!strcmp(token,"SIM")) 
  {
    mode=UT_MODE_SIM;
    printf("TST: simulated USART mode selected\n");
  } 
  else if (!strcmp(token,"FILE")) 
  {
    if ( (len = GetNextToken (Buffer, token, "\"")) == 0 ) 
    {
      return DRV_INVALID_PARAMS;
    } 
    mode=UT_MODE_FILE;
    fname=token;
    printf("TST: file mode selected\n");
    strcpy(m_last_config,""); /* don't store file-mode */
  } 
  else 
  {
    if (!strcmp(token,"NT")) {
      mode=UT_MODE_NT;
    } else if (!strcmp(token,"95")) {
      mode=UT_MODE_95;
    } else if (!strcmp(token,"REAL")) {
      /*lint -e813, suppress Info 813: auto variable 'osver' has size '148' */ 
      OSVERSIONINFO osver;
      /*lint +e813 */
      osver.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
      GetVersionEx( &osver);
      if (osver.dwPlatformId==VER_PLATFORM_WIN32_NT) {
        mode=UT_MODE_NT;
      } else {
        mode=UT_MODE_95;
      }
    } else {
      return DRV_INVALID_PARAMS;
    }

    if (mode==UT_MODE_NT) {
      printf("TST: real USART mode for winNT selected\n");
    } else {
      printf("TST: real USART mode for win95 selected\n");
    }

    /* com port setting */
    if ( (len = GetNextToken (Buffer, token, " #")) == 0 ) {
      return DRV_INVALID_PARAMS;
    } else {
      Buffer += (len+1);
    }

    extPort=atoi(&token[3]);

    /*
     * Get baudrate from configuration string
     */
    if ( (len = GetNextToken (Buffer, token, " #")) == 0 ) {
      return DRV_INVALID_PARAMS;
    } else {
      Buffer += (len+1);
    }

    printf("SER: requested baudrate: %s\n",token);
    switch ( atoi(token) )
    {
      case 38400:
        baudrate = UT_BAUD_38400;
        break;
      case 19200:
        baudrate = UT_BAUD_19200;
        break;
      case 9600:
        baudrate = UT_BAUD_9600;
        break;
      case 57600:
        baudrate = UT_BAUD_57600;
        break;
      case 115200:
        baudrate = UT_BAUD_115200;
        break;
      case 128000:
        baudrate = UT_BAUD_128000;
        break;
      case 256000:
        baudrate = UT_BAUD_256000;
        break;
      default:
        printf("SER: no CBR_xxx constant found for requested baudrate\n");
        baudrate = atoi(token);
        break;
    }

    /*
     * Get flow control from configuration string
     */
    if ( (len = GetNextToken (Buffer, token, " #")) == 0 ) {
      return DRV_INVALID_PARAMS;
    } else {
      Buffer += (len+1);
    }

    flow_ctrl=token[0];
    printf("SER: requested flow control: %c\n",flow_ctrl);
  }

  /* exit receiver task */ 
  if ( SER_Data.TaskHandle != OS_ERROR )
  {
    os_DestroyTask( 0, SER_Data.TaskHandle );
  }
#ifdef _TOOLS_
  os_GetTime ( 0, &extr_creation_time );
#endif
  UT_Close();
  UT_set_mode(mode);
  if (UT_Init (baudrate, 100, flow_ctrl, NULL, fname)!=0)
  {
    strcpy(m_last_config,"");
    return DRV_INITFAILURE;
  };
  /* restart receiver task */
  if ( os_CreateTask ( 0, (char*)"EXTR", (void (*)(OS_HANDLE, ULONG))(SER_Receiver), 2048, 1,
                       &SER_Data.TaskHandle, ext_data_pool_handle) != OS_OK  ||
                       os_StartTask ( 0, SER_Data.TaskHandle, 0 ) != OS_OK)
  {
    return DRV_INITFAILURE;
  }

  return DRV_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : SER                 |
| STATE   : code                       ROUTINE : SER_Init            |
+--------------------------------------------------------------------+

  PURPOSE : initialize driver

*/
USHORT SER_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo )
{
static const T_DRV_EXPORT SER_Info =
{
  "SER",
  CALLED_FROM_ISR,
  {
#ifdef _TOOLS_
    SER_Init,
#endif
    SER_Exit,
    SER_Read,
    SER_Write,
    NULL,
    NULL,
    NULL,
    SER_SetSignal,
    SER_ResetSignal,
    SER_SetConfig,
    NULL,
    NULL,
  }
};

  SER_Data.Handle = DrvHandle;

  SER_Data.EnabledSignalType = 0;

  SER_Data.Callback = CallbackFunc;

  SER_Data.Connected = FALSE;

  SER_Data.TaskHandle = OS_ERROR;

#ifndef _TOOLS_
  UT_set_mode(UT_MODE_SIM);
  UT_Init (2, 100, 'N', NULL, NULL);
  UT_SetFlowCtrl ('N');
  if ( os_CreateTask ( 0, (char*)"EXTR", (void (*)(OS_HANDLE, ULONG))(SER_Receiver), 2048, 1,
                       &SER_Data.TaskHandle, ext_data_pool_handle) == OS_OK )
  {
    if ( os_StartTask ( 0, SER_Data.TaskHandle, 0 ) != OS_OK)
      printf ( "SYSTEM ERROR: Cannot start task EXTR" );
  }
  else
    printf ( "SYSTEM ERROR: Cannot create task EXTR" );
#endif

  m_last_config[0]='\0';

  *DrvInfo = &SER_Info;

  return DRV_OK;
}

