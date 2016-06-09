/* 
+------------------------------------------------------------------------------
|  File:       vsi_drv.c
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
|  Purpose :  This Module defines the virtual system interface part
|             for driver access.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __VSI_DRV_C__
#define __VSI_DRV_C__
#endif

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdio.h>

#include "gpfconf.h"
#include "typedefs.h"

#include "vsi.h"
#include "gdi.h"
#include "os.h"
#ifdef _TOOLS_
 #include "drvconf.h"
#endif
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"
#include "frame.h"

/*==== TYPES ======================================================*/


#ifdef _TOOLS_
typedef struct
{
  char Name[RESOURCE_NAMELEN];
  char Process[RESOURCE_NAMELEN];
  char DrvConfig[80];
} _T_DRV_LIST_ENTRY;

typedef struct
{
  _T_DRV_LIST_ENTRY DrvEntry [ 5 ];
} _T_DRV_LIST;

#endif

/*==== CONSTANTS ==================================================*/


/*==== EXTERNALS ==================================================*/

/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED") 
#endif

extern T_DRV_TABLE_ENTRY DrvTable [];

/*==== VARIABLES ==================================================*/

#undef EXTR_SEND_CONTROL
#ifndef RUN_INT_RAM
T_DRV_LIST *DriverConfigList;    /* pointer to start of driver cinfiguration list */
static T_DRV_LIST *DriverList;   /* pointer to selected driver list */
#ifdef EXTR_SEND_CONTROL
FILE *fp;
#endif
#endif

#ifdef _TOOLS_
_T_DRV_LIST _DrvList={0};
T_DRV_LIST DrvList={0};
#endif

#ifdef _TOOLS_
#pragma data_seg()
#endif /* _TOOLS_ */
/* -------------- S H A R E D - END ---------------- */


/*==== FUNCTIONS ==================================================*/

void ClearDriverTable ( void );

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_create        |
+--------------------------------------------------------------------+

  PURPOSE : enter a new driver in the driver list

*/
int vsi_d_create ( T_HANDLE caller, T_TST_DRV_ENTRY *drv_info )
{
T_HANDLE drv_handle;

  drv_handle = drv_info->drv_pos;
  vsi_d_exit ( caller, 0 );
  DriverList->DrvEntry[drv_handle].drv_Init = drv_info->entry.drv_Init;
  DriverList->DrvEntry[drv_handle].Name = drv_info->entry.Name;
  DriverList->DrvEntry[drv_handle].Process = drv_info->entry.Process;
  DriverList->DrvEntry[drv_handle].DrvConfig = drv_info->entry.DrvConfig;

  ClearDriverTable();
  vsi_d_init ( caller );
  vsi_d_setsignal ( caller, 0, DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT|DRV_SIGTYPE_DISCONNECT); 
  vsi_d_setconfig ( caller, 0, NULL ); 
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_callback      |
+--------------------------------------------------------------------+

  PURPOSE : callback from a driver

*/
void vsi_d_callback ( T_DRV_SIGNAL *Signal )
{
T_HANDLE Caller;
T_HANDLE DrvHandle;
int sts;
#ifdef EXTR_SEND_CONTROL
OS_TIME time;
T_PRIM_HEADER *p;
static int cnt = 0;
int bytes;
#endif

  Caller = Signal->DrvHandle;
  DrvHandle = DrvTable[Caller].UpperDrv;
  if ( DrvHandle )
  {
    if ( DrvTable[DrvHandle].DrvInfo->DrvFunc.drv_Callback != NULL )
    {
#if defined _TARGET_ && defined _NUCLEUS_
      if ( DrvTable[Caller].DrvInfo->Flags & CALLED_FROM_ISR )
        os_ExecuteCallback ( Caller, DrvTable[DrvHandle].DrvInfo->DrvFunc.drv_Callback, Signal ); 
      else
#endif
        (DrvTable[DrvHandle].DrvInfo->DrvFunc.drv_Callback)( Signal );
    }
  }
  else
  {
    if ( DrvTable[Caller].ProcessHandle )
    {
      OS_QDATA Msg;
      OS_TIME time;
      Msg.data16 = MSG_SIGNAL;
      Msg.data32 = Signal->SignalType;
      Msg.ptr = Signal->UserData;
      os_GetTime ( 0, &time );
      Msg.time = (ULONG)time;
      Msg.e_id = DrvTable[Caller].ProcessHandle;
#ifdef EXTR_SEND_CONTROL
      if ( Msg.ptr )
      {
        os_GetTime(0,&time);
        fp = fopen("test.txt", "a");
        p = (T_PRIM_HEADER*)((T_PRIM_X*)Msg.ptr)->prim_ptr;
        if ( p->opc == 0x8000 )
        {
          printf("EXTR: Start sending %s, time %d, %d\n", (char*)P2D(p),time, cnt & 1023  );
          bytes = fprintf(fp, "EXTR: Start sending %s, time %d, %d\n", (char*)P2D(p),time, cnt & 1023  );
        }
        else
        {
          printf("EXTR: Start sending primitive, time %d, %d\n", time, cnt & 1023);
          bytes = fprintf(fp, "EXTR: Start sending primitive, time %d, %d\n", time, cnt & 1023);
        }
        fclose(fp);
      }
#endif
#ifdef _TOOLS_
      sts = os_SendToQueue (NO_TASK, DrvTable[Caller].ProcessHandle, OS_NORMAL, OS_SUSPEND, &Msg );
#else
      sts = os_SendToQueue (NO_TASK, pf_TaskTable[DrvTable[Caller].ProcessHandle].QueueHandle, OS_NORMAL, OS_SUSPEND, &Msg );
#endif
#ifdef EXTR_SEND_CONTROL
      os_GetTime(0,&time);
      fp = fopen("test.txt", "a");
      printf("EXTR: Complete sending, time %d %d\n", time, cnt & 1023);
      bytes = fprintf(fp,"EXTR: Complete sending, time %d %d\n", time, cnt++ & 1023);
      fclose(fp);
#endif
      /*
       * This is a dirty patch, but due to the missing return value there is no other choice
       */
      if ( sts == OS_TIMEOUT || sts == OS_ERROR )
      {
        T_PRIM_X *sys_prim;

        sys_prim = (T_PRIM_X*)Signal->UserData;
        PFREE(P2D(sys_prim->prim_ptr));
        PFREE(P2D(sys_prim));
      }
    }
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_init          |
+--------------------------------------------------------------------+

  PURPOSE : initialize drivers

*/
/*lint -esym(644,DrvInfo) */
int vsi_d_init ( T_HANDLE Caller )
{
T_DRV_EXPORT const *DrvInfo;
USHORT i;
SHORT sts;

#ifdef EXTR_SEND_CONTROL
   fp = fopen("test.txt", "a");
   fprintf(fp,"=========================================================\n");
   fclose (fp);
#endif
   for ( i = 1; i < MAX_TST_DRV; i++ )
   {
     sts = DRV_NOTCONFIGURED;
#ifdef _TOOLS_
     if ( DrvTable[i].DrvInfo )
       sts = (SHORT)(DrvTable[i].DrvInfo->DrvFunc.drv_Init)(i,vsi_d_callback,&DrvInfo);
     else
#endif
       if ( DriverList->DrvEntry[i].drv_Init )
         sts = (SHORT)(DriverList->DrvEntry[i].drv_Init)(i,vsi_d_callback,&DrvInfo);
     if ( sts == DRV_OK )
     {
       if ( DriverList->DrvEntry[i].Process )
         DrvTable[i].ProcessHandle = vsi_c_open ( Caller, (char*)DriverList->DrvEntry[i].Process );
       DrvTable[i].UpperDrv = i-1;
       DrvTable[i-1].LowerDrv = i;
       DrvTable[i].DrvInfo = DrvInfo;
#if defined _TARGET_ && defined _NUCLEUS_
       if ( DrvTable[i].DrvInfo->Flags & CALLED_FROM_ISR )
         if ( os_CreateCallback() == OS_ERROR )
           return VSI_ERROR;
#endif
     }
     else
     {
       if ( sts != DRV_NOTCONFIGURED )
       {
         return VSI_ERROR;
       }
     }
   }
   return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_exit          |
+--------------------------------------------------------------------+

  PURPOSE : exit drivers

*/
int vsi_d_exit ( T_HANDLE Caller, T_HANDLE DrvHandle )
{
T_HANDLE Handle;
T_HANDLE min, max;

  if ( DrvHandle )
  {
    min = DrvHandle;
    max = DrvHandle+1;
  }
  else
  {
    min = 1;
    max = MAX_TST_DRV;
  }

  for ( Handle = min; Handle < max; Handle++ )
  {
    if ( DrvTable[Handle].DrvInfo )
    {
      if ( DrvTable[Handle].DrvInfo->DrvFunc.drv_Exit != NULL )
        (DrvTable[Handle].DrvInfo->DrvFunc.drv_Exit)();
    }
  }
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_open          |
+--------------------------------------------------------------------+

  PURPOSE : open a drivers

*/
int vsi_d_open ( T_HANDLE Caller, char *Name )
{
int i;

  for ( i = 1; i <= MAX_TST_DRV; i++ )
  {
    if ( DrvTable[i].DrvInfo && DrvTable[i].DrvInfo->Name )
      if ( !strcmp ( DrvTable[i].DrvInfo->Name, Name ) )
        return (i);
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_close         |
+--------------------------------------------------------------------+

  PURPOSE : close a driver

*/
/*lint -esym(715,DrvHandle) suppress Info -- Symbol 'DrvHandle' not referenced */
int vsi_d_close ( T_HANDLE Caller, T_HANDLE DrvHandle )
{
  
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_read          |
+--------------------------------------------------------------------+

  PURPOSE : read data from a driver

*/
int vsi_d_read ( T_HANDLE Caller, T_HANDLE DrvHandle, void *Buffer, ULONG *Size )
{
T_HANDLE Handle;

  if ( DrvHandle )
    Handle = DrvHandle;                  /* Caller TST: opened driver with vsi_d_open() */
  else
    Handle = DrvTable[Caller].LowerDrv;  /* Caller drv: handle defined by ConfigSring */

  if ( DrvTable[Handle].DrvInfo->DrvFunc.drv_Read != NULL )
    if ( (DrvTable[Handle].DrvInfo->DrvFunc.drv_Read)( (void*)Buffer, Size ) == DRV_OK )
      return VSI_OK;
  
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_write         |
+--------------------------------------------------------------------+

  PURPOSE : write data to a driver

*/
int vsi_d_write ( T_HANDLE Caller, T_HANDLE DrvHandle, void *Buffer, ULONG Size )
{
T_HANDLE Handle;
ULONG TotalBytesToWrite = Size;
ULONG BytesToWrite = Size;
ULONG TotalBytesWritten = 0;
ULONG BytesWritten = 0;
char *ptr = (char*)Buffer;

  if ( DrvHandle )
    Handle = DrvHandle;                  /* Caller TST: opened driver with vsi_d_open() */
  else
    Handle = DrvTable[Caller].LowerDrv;  /* Caller drv: handle defined by ConfigSring */

  if ( DrvTable[Handle].DrvInfo->DrvFunc.drv_Write != NULL )
  {
    while ( TotalBytesWritten < TotalBytesToWrite )
    {
      BytesWritten = BytesToWrite;
      if ( (DrvTable[Handle].DrvInfo->DrvFunc.drv_Write)( (void*)ptr, &BytesWritten ) != DRV_OK )
        return VSI_ERROR;
      ptr += BytesWritten;
      TotalBytesWritten += BytesWritten;
      BytesToWrite = TotalBytesToWrite - TotalBytesWritten; 
    }
  }
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_flush         |
+--------------------------------------------------------------------+

  PURPOSE : flush the internal buffers of a driver

*/
int vsi_d_flush ( T_HANDLE Caller, T_HANDLE DrvHandle )
{
T_HANDLE Handle;
T_HANDLE min, max;

  if ( DrvHandle )
  {
    min = DrvHandle;
    max = DrvHandle+1;
  }
  else
  {
    min = 1;
    max = MAX_TST_DRV;
  }

  for ( Handle = min; Handle < max; Handle++ )
  {
    if ( DrvTable[Handle].DrvInfo )
      if ( DrvTable[Handle].DrvInfo->DrvFunc.drv_Flush != NULL )
        if ( (DrvTable[Handle].DrvInfo->DrvFunc.drv_Flush)() != DRV_OK )
          return VSI_ERROR;
  }
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_setsignal     |
+--------------------------------------------------------------------+

  PURPOSE : enable a signal in a driver

*/
int vsi_d_setsignal ( T_HANDLE Caller, T_HANDLE DrvHandle, USHORT SignalType )
{
T_HANDLE Handle;
T_HANDLE min, max;

  if ( DrvHandle )
  {
    min = DrvHandle;
    max = DrvHandle+1;
  }
  else
  {
    min = 1;
    max = MAX_TST_DRV;
  }

  for ( Handle = min; Handle < max; Handle++ )
  {
    if ( DrvTable[Handle].DrvInfo )
      if ( DrvTable[Handle].DrvInfo->DrvFunc.drv_SetSignal != NULL )
        if ( (DrvTable[Handle].DrvInfo->DrvFunc.drv_SetSignal)( SignalType  ) != DRV_OK )
          return VSI_ERROR;
  }
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_resetsignal   |
+--------------------------------------------------------------------+

  PURPOSE : disable a signal in a driver

*/
int vsi_d_resetsignal ( T_HANDLE Caller, T_HANDLE DrvHandle, USHORT SignalType )
{
T_HANDLE Handle;
T_HANDLE min, max;

  if ( DrvHandle )
  {
    min = DrvHandle;
    max = DrvHandle+1;
  }
  else
  {
    min = 1;
    max = MAX_TST_DRV;
  }

  for ( Handle = min; Handle < max; Handle++ )
  {
    if ( DrvTable[Handle].DrvInfo )
      if ( DrvTable[Handle].DrvInfo->DrvFunc.drv_ResetSignal != NULL )
        if ( (DrvTable[Handle].DrvInfo->DrvFunc.drv_ResetSignal)( SignalType  ) != DRV_OK )
          return VSI_ERROR;
  }
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_setconfig     |
+--------------------------------------------------------------------+

  PURPOSE : configure  a driver

*/
int vsi_d_setconfig ( T_HANDLE Caller, T_HANDLE DrvHandle, char *Config )
{
T_HANDLE Handle;

  if ( Config && DrvHandle != 0)
  {
    if ( DrvTable[DrvHandle].DrvInfo->DrvFunc.drv_SetConfig != NULL )
      if ( (DrvTable[DrvHandle].DrvInfo->DrvFunc.drv_SetConfig)( Config  ) != DRV_OK )
        return VSI_ERROR;
  }
  else
  {
    T_HANDLE min, max;

    if ( DrvHandle )
    {
      min = DrvHandle;
      max = DrvHandle+1;
    }
    else
    {
      min = 1;
      max = MAX_TST_DRV;
    }

    for ( Handle = min; Handle < max; Handle++ )
    {
      if ( DriverList->DrvEntry[Handle].DrvConfig )
      {
        if ( DrvTable[Handle].DrvInfo->DrvFunc.drv_SetConfig != NULL )
          if ( (DrvTable[Handle].DrvInfo->DrvFunc.drv_SetConfig)( (char*)DriverList->DrvEntry[Handle].DrvConfig  ) != DRV_OK )
            return VSI_ERROR;
      }
    }
  }
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_DRV             |
| STATE   : code                       ROUTINE : vsi_d_getconfig     |
+--------------------------------------------------------------------+

  PURPOSE : read configuration data from a driver

*/

int vsi_d_getconfig ( T_HANDLE Caller, T_HANDLE DrvHandle, char *Config )
{
T_HANDLE Handle;
char Buffer[40];
char *ptr = Config;

  Handle = DrvHandle;
  while ( Handle )
  {
    if ( DrvTable[Handle].DrvInfo )
      if ( DrvTable[Handle].DrvInfo->DrvFunc.drv_GetConfig != NULL )
        if ( (DrvTable[Handle].DrvInfo->DrvFunc.drv_GetConfig)( Buffer ) != DRV_OK )
          return VSI_ERROR;

    sprintf ( ptr, "%s:%s;",DrvTable[Handle].DrvInfo->Name,Buffer );
    ptr = ptr + strlen(DrvTable[Handle].DrvInfo->Name) + strlen(Buffer) + 2;
    Handle = DrvTable[Handle].LowerDrv;
  }
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)         MODULE  : VSI_DRV               |
| STATE   : code                     ROUTINE : InitializeDriverTable |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the driver table

*/
void InitializeDriverConfig ( void )
{
#ifdef _TOOLS_
USHORT j;

  for ( j = 1; j < MAX_TST_DRV ; j++ )
  {
    if ( DriverConfigList->DrvEntry[j].Name )
    {
      strcpy ( _DrvList.DrvEntry[j].Name, DriverConfigList->DrvEntry[j].Name );
      DrvList.DrvEntry[j].Name = _DrvList.DrvEntry[j].Name;
    }
    
    if ( DriverConfigList->DrvEntry[j].drv_Init )
    {
      DrvList.DrvEntry[j].drv_Init = DriverConfigList->DrvEntry[j].drv_Init;
    }

    if ( DriverConfigList->DrvEntry[j].Process )
    {
      strcpy ( _DrvList.DrvEntry[j].Process, DriverConfigList->DrvEntry[j].Process );
      DrvList.DrvEntry[j].Process = _DrvList.DrvEntry[j].Process;
    }

    if ( DriverConfigList->DrvEntry[j].DrvConfig )
    {
      strcpy ( _DrvList.DrvEntry[j].DrvConfig, DriverConfigList->DrvEntry[j].DrvConfig );
      DrvList.DrvEntry[j].DrvConfig = _DrvList.DrvEntry[j].DrvConfig;
    }
  }

  DriverList = &DrvList;
#else
  DriverList = DriverConfigList;
#endif /* _TOOLS_ */
  ClearDriverTable();
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)         MODULE  : VSI_DRV               |
| STATE   : code                     ROUTINE : ClearDriverTable      |
+--------------------------------------------------------------------+

  PURPOSE : Clear the driver table

*/
void ClearDriverTable ( void )
{
char i;

  for ( i = 1; i <= MAX_TST_DRV; i++ )
  {
    memset ( &DrvTable[i], 0, sizeof(T_DRV_TABLE_ENTRY) );
  }

}
#endif
