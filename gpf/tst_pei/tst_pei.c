/* 
+------------------------------------------------------------------------------
|  File:       tst_pei.c
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
|  Purpose :  This Modul contains the PEI interface of TST
+----------------------------------------------------------------------------- 
*/ 

#ifndef __TST_PEI_C__
#define __TST_PEI_C__
#endif
 
#define ENTITY_TST

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gpfconf.h"
#define	_FF_RV_EXIST_	1

#ifdef _TOOLS_
 #include "frame_const.h" 
 #include <stdlib.h>
#endif /* _TOOLS_ */

#ifndef _TARGET_ 
 #include "printtofile.h"
#endif

#include "typedefs.h"
#include "vsi.h"
#include "../frame/frame.h"
#include "pei.h"
#include "os.h"
#include "tools.h"
#include "gdi.h"
#include "os.h"
#include "frm_defs.h" 
#include "frm_types.h"
#include "frm_glob.h"
#include "../frame/route.h"
#include "drvconf.h"
#include "tstdriver.h"
#include "tstheader.h"
#include "tst_mux.h"
#include "pcon.h"

#ifdef CTB
 #include "tst_primitives.h"
#endif

#ifdef _PSOS_
 #include "pna.h"
#endif

#ifdef _FF_RV_EXIST_
 #include "../../services/ffs/ffs_api.h"
#endif

/*==== TYPES ======================================================*/

#undef VSI_CALLER
#ifdef _TOOLS_
 #define VSI_CALLER TST_Handle,
#else
 #define VSI_CALLER e_running[os_MyHandle()],
#endif

/*==== CONSTANTS ==================================================*/

#ifdef _VXWORKS_
  #define TST_STACKSIZE  8192
  #define TST_PRIORITY    190             /* priority (1->high, 255->low) */
#else
  #ifdef _TOOLS_
    #define TST_STACKSIZE  4096
    #define TST_PRIORITY     15             
    #define TST_SND_PRIORITY 15		          /* priority (1->low, 255->high) */
    #define TST_RCV_PRIORITY 15		          /* priority (1->low, 255->high) */
  #else
    #define TST_STACKSIZE  1024
    #define TST_PRIORITY      2             
    #define TST_SND_PRIORITY  6             /* priority (1->low, 255->high) */
    #define TST_RCV_PRIORITY  7             /* priority (1->low, 255->high) */
  #endif
#endif

#ifdef _TOOLS_
 #define TST_TIMERS                        2
 #define GET_STACK_TIME_TIMER_INDEX        0
 #define GET_STACK_TIME_TIMER_DURATION 60000
 #define TST_SYNC_TIMER_INDEX              1
 #define TST_SYNC_TIMER_DURATION        2000
#else
 #define TST_TIMERS         0
#endif

#ifdef _FF_RV_EXIST_
#define TRACEMASKFILE "/var/dbg/tracemask"
#endif

#define DRV_CALLBACK_OPC     0

#ifdef _TOOLS_
  #define TR_RCV_BUF_SIZE    (MAX_PRIM_PARTITION_SIZE)
  #define TR_MAX_IND         (TR_RCV_BUF_SIZE-1)
  USHORT MaxEntities = MAX_ENTITIES;
#endif /* _TOOLS_ */

/*==== EXTERNALS ==================================================*/

#ifndef _TOOLS_
extern const T_PCON_PROPERTIES *pcon;
extern const USHORT TST_SndQueueEntries;
extern const USHORT TST_RcvQueueEntries;
extern const USHORT TST_SndStacksize;
extern const USHORT TST_RcvStacksize;
extern UBYTE FrameEnv;
#endif

#ifdef _TOOLS_
__declspec (dllimport) UBYTE SuppressOK;
#else
extern char *str2ind_version;
extern UBYTE SuppressOK;
#endif

#ifdef CTB
  static T_HANDLE hCommIDLE = VSI_ERROR;
  extern short idle_pei_create (T_PEI_INFO const ** info);
#endif

#ifdef _FF_RV_EXIST_
  extern char TaskName[];
#endif

/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM

T_HANDLE TST_Handle;
T_HANDLE RCV_Handle;
T_HANDLE TIF_Handle;

UBYTE TST_DrvState = TST_DRV_DISCONNECTED;
static char const *ok_string = "OK";
static UBYTE frmenv;

#ifdef _TOOLS_ 
ULONG TR_RcvBufferSize = TR_RCV_BUF_SIZE;
ULONG TR_MaxInd = TR_MAX_IND;
int tst_syncronized = 0;
int tst_message_received = 0;
int tst_sync_timeout_cnt = 0;
int tst_max_sync_timeout = 0;
int tst_sync_mode = 0;
int tst_sync_sucess = 1;
int tst_sync_started = 0;
int tst_status_received = 0;
T_HANDLE tst_sync_req_handle = VSI_ERROR;
char sync_req_name[RESOURCE_NAMELEN];
char sync_req_time[16];

#endif /* _TOOLS_ */

GLOBAL USHORT tst_task_priority = TST_PRIORITY;
GLOBAL ULONG tst_task_stack_size = TST_STACKSIZE;

extern SHORT tst_pei_primitive (void *primitive);

#endif /* RUN_INT_RAM */

#ifdef RUN_INT_RAM

extern T_HANDLE TST_Handle;
extern T_HANDLE RCV_Handle;
extern T_HANDLE TIF_Handle;
extern UBYTE TST_DrvState;

#endif

#ifdef CTB
  static char ctb_rcv[4];
  static U32 ctb_remaining_tick_time=0;
  BOOL ctb_tick_enabled = FALSE;
  BOOL ctb_sent_to_tap = FALSE;
  T_HANDLE idle_handle;
#endif

/*==== FUNCTIONS ==================================================*/

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : TST_PEI             |
| STATE   : code                       ROUTINE : pei_primitive       |
+--------------------------------------------------------------------+ 
*/

SHORT tst_pei_primitive (void *primitive)
{
  T_PRIM_HEADER *prim = (T_PRIM_HEADER*)primitive;
#ifndef _TOOLS_
  T_VOID_STRUCT *prim_ptr = NULL;
#endif
  SHORT ret = PEI_OK;
  
#ifdef _TOOLS_
  if ( ((SAP_NR(prim->opc)==TRACE_SAP) || (prim->opc==TRACE_OPC)) && (P_SHO(prim)!=0) )
  {
    prim->opc = SYS_MASK;
  }
#endif

  if ( (SAP_NR(prim->opc)!=TRACE_SAP) && (prim->opc!=TRACE_OPC) && !(prim->opc & SYS_MASK) )
  {
#ifndef _TOOLS_
    prim_ptr = (T_VOID_STRUCT*)(((T_PRIM_X*)prim)->prim_ptr);
#endif
    VSI_PPM_RCV(prim_ptr);
    PTRACE_OUT(prim->opc );
  }

  if ( TST_DrvState == TST_DRV_CONNECTED )
  {
    if ( vsi_d_write ( TST_Handle, TIF_Handle, primitive, prim->len ) != VSI_OK )
      ret = PEI_ERROR;
#ifdef _TOOLS_
    vsi_t_sleep(TST_Handle,10);
#endif
  }

#ifndef _TOOLS_
  if ( prim_ptr != NULL )
  {
    FREE(P2D(prim_ptr));
  }
#endif
  vsi_trc_free (0, (T_VOID_STRUCT**)&prim);

  return ( ret );
}
#endif
    
#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : TST_PEI             |
| STATE   : code                       ROUTINE : pei_init            |
+--------------------------------------------------------------------+ 
*/

LOCAL SHORT pei_init ( T_HANDLE handle )
{
#ifdef _TOOLS_
  T_HANDLE h_pco;
#endif

#ifdef _FF_RV_EXIST_
  T_HANDLE entityTraceMask;
  ULONG actual;
  T_FFS_FD fd;
  T_RV_RETURN retPath;
  BOOL gotAll = TRUE;
#endif

  TST_Handle = handle;
#ifdef _TOOLS_
  while (vsi_c_open (0, FRM_RCV_NAME) < VSI_OK) 
  {
    vsi_t_sleep(0,100);
  }
  frmenv = (UBYTE) pf_get_frameenv ();
  if ( os_OpenQueue (0, FRM_PCO_NAME, &h_pco) == OS_OK ||
       os_create_extq (FRM_PCO_NAME, &h_pco) == OS_OK)
  {
    vsi_o_set_htrace (h_pco);
#ifdef _DEBUG
    fprintf (stdout,"TST: %s commH set to %d\n", FRM_PCO_NAME,h_pco);
#endif
  }
  vsi_t_start ( handle, GET_STACK_TIME_TIMER_INDEX, GET_STACK_TIME_TIMER_DURATION );
#else
  if ( vsi_c_open (TST_Handle, FRM_RCV_NAME) < VSI_OK ) 
    return PEI_ERROR;
  RCV_Handle = vsi_e_handle ( TST_Handle, FRM_RCV_NAME );
  frmenv = FrameEnv;
#endif
#ifndef _TARGET_ 
  initPrintToFile();
#endif
  if ( vsi_d_init ( TST_Handle ) != VSI_OK )
  {
#ifndef _TARGET_
    printf("SYSTEM ERROR: Driver initialization failed\n");
    vsi_o_assert ( TST_Handle, OS_SYST_ERR, __FILE__, __LINE__, "SYSTEM ERROR: Testinterface driver initialization failed" );
#else
    ;
#endif
  }
  TIF_Handle = vsi_d_open ( TST_Handle, (char*)TIF_NAME );
  vsi_d_setsignal ( TST_Handle, 0, DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT|DRV_SIGTYPE_DISCONNECT); 
  vsi_d_setconfig ( TST_Handle, 0, NULL ); 
  vsi_trcsuspend ( TST_Handle, TST_Handle, OS_NO_SUSPEND );
  SuppressOK = FALSE;
  tst_mux_init();

#ifdef _FF_RV_EXIST_
  fd = ffs_open(TRACEMASKFILE,  FFS_O_RDONLY);
  if (fd < 0)
  {
    // could not open nor create /var/dbg/tracemask warning
    vsi_o_ttrace(NO_TASK, TC_SYSTEM, "pei_init: no trace mask in FFS");
  }
  else
  {
    InitializeTrace();
    TraceMask[0] = 0;
    TraceMask[0] |= TC_SYSTEM|TC_ERROR;
    for (entityTraceMask = 0; entityTraceMask < (MaxEntities + 1); entityTraceMask++)
    {
      actual = TraceMask[entityTraceMask];
      if (ffs_read(fd, (void*)&TraceMask[entityTraceMask], sizeof(ULONG) ) != sizeof(ULONG) )
      {
        gotAll = FALSE;
      }
      else
      {
        if (entityTraceMask == 0) // first element
        {
           TraceMask[0] = 0;
           TraceMask[0] |= TC_SYSTEM|TC_ERROR;
        }
        if (actual != TraceMask[entityTraceMask])
        {
          // not the default trace mask, generate warning
          if ( vsi_e_name ( 0, entityTraceMask, TaskName ) == VSI_OK )
          {
            vsi_o_ttrace(NO_TASK, TC_SYSTEM, "pei_init: FFS trace mask gave 0x%08x for %s (default 0x%08x)", TraceMask[entityTraceMask], TaskName, actual);
          }
        }
      }
    }  
  }
  ffs_close(fd);
  if (!gotAll)
  {
    vsi_o_ttrace(NO_TASK, TC_SYSTEM, "pei_init: some trace masks can't be read from FFS");
  }
#endif

  return PEI_OK;
}
#endif



#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : TST_PEI             |
| STATE   : code                       ROUTINE : pei_exit            |
+--------------------------------------------------------------------+ 
*/

static SHORT pei_exit (void)
{
#ifdef _TOOLS_
  T_HANDLE rcvh=vsi_p_handle (0, FRM_RCV_NAME);
  if (rcvh > VSI_OK) 
  {
    /* exit RCV process */
    vsi_p_exit ( TST_Handle, vsi_p_handle (0, FRM_RCV_NAME));
  }
  /* exit all drivers */
  vsi_d_exit ( TST_Handle, 0); 
#endif /* _TOOLS_ */

  return PEI_OK;
}
#endif

#ifndef RUN_INT_RAM
#ifdef _TOOLS_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : TST_PEI             |
| STATE   : code                       ROUTINE : pei_timeout         |
+--------------------------------------------------------------------+ 
*/

LOCAL SHORT pei_timeout (USHORT index)
{

  switch ( index )
  {
    case GET_STACK_TIME_TIMER_INDEX:
#if 0
      if ( tst_message_received == 1 )
      {
        tst_drv_write ( NO_TASK, SYS_MASK, FRM_TST_NAME, (char*)SYSPRIM_GET_STACK_TIME );
      }
      vsi_t_start ( VSI_CALLER GET_STACK_TIME_TIMER_INDEX, GET_STACK_TIME_TIMER_DURATION );
#endif
      break;
    case TST_SYNC_TIMER_INDEX:
      if ( tst_sync_timeout_cnt++ >= tst_max_sync_timeout-1 )
      {
        T_HANDLE tif_handle;
        T_VOID_STRUCT *ptr;
        
        tst_status_received = 0;
        tst_sync_timeout_cnt = 0;
        tst_sync_mode = 0;
        tst_sync_sucess = 0;
        tif_handle = vsi_d_open ( TST_Handle, (char*)TIF_NAME );
        vsi_d_setconfig ( TST_Handle, tif_handle, DISABLE_SYNC_MODE );
        vsi_t_stop ( TST_Handle, TST_SYNC_TIMER_INDEX );
        ptr = vsi_c_pnew ( sizeof(T_PRIM_HEADER)+strlen(SYSPRIM_TST_SYNC_REJ), 0x8000 FILE_LINE );
        strcpy ( (char*)ptr, SYSPRIM_TST_SYNC_REJ );
        vsi_c_psend ( tst_sync_req_handle, ptr );
      }
      else
      {
        if ( tst_syncronized == 0 )
        {
          T_HANDLE tif_handle;
          tst_sync_sucess = 1;
          tst_status_received = 0;
          tif_handle = vsi_d_open ( TST_Handle, (char*)TIF_NAME );
          vsi_d_setconfig ( TST_Handle, tif_handle, ENABLE_SYNC_MODE );
          tst_drv_write ( NO_TASK, SYS_MASK, FRM_RCV_NAME, (char*)"TRACECLASS 0x10" );
          vsi_t_sleep ( TST_Handle, 100 );
          tst_drv_write ( NO_TASK, SYS_MASK, FRM_RCV_NAME, (char*)"STATUS TASK" );
          vsi_t_start ( TST_Handle, TST_SYNC_TIMER_INDEX, TST_SYNC_TIMER_DURATION );
          vsi_t_sleep ( TST_Handle, 200 );
          tst_drv_write ( NO_TASK, SYS_MASK, FRM_RCV_NAME, (char*)"ROUTING" );
        }
      }
      break;
    default:
      break;
  }
  return PEI_OK;
}
#endif
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : TST_PEI             |
| STATE   : code                       ROUTINE : pei_signal          |
+--------------------------------------------------------------------+ 
*/
LOCAL SHORT pei_signal (ULONG SignalType, void *ptr)
{
/*
 * the following line of code causes a warning on tms470 compiler, that cannot be avoided
 * without changing the PEI interface. Warning will not cause a problem 
 */
T_VOID_STRUCT *sig_ptr = (T_VOID_STRUCT*)ptr;
T_PRIM_HEADER *prim;
T_S_HEADER *s_hdr;
T_HANDLE DestTaskHandle, DestComHandle;
T_HANDLE min, max, i;
unsigned int Length;
ULONG Suspend, TraceMask, OldTraceMask;
unsigned int Offset = 0;
char token[81];
UBYTE FreePrim = 1;
SHORT ret = PEI_OK;
USHORT processed = FALSE;

  switch ( SignalType )
  {
    case DRV_SIGTYPE_READ:
    VSI_PPM_RCV(sig_ptr);
    if ( (prim = ((T_PRIM_X*)sig_ptr)->prim_ptr) != NULL )
    {
      VSI_PPM_RCV (prim);
      s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);
      DestTaskHandle = vsi_e_handle ( TST_Handle, s_hdr->rcv );
      if ( DestTaskHandle == VSI_ERROR )
      {
        if ( !strcmp ( "IRQ", s_hdr->rcv ) )
        {
          DestTaskHandle = 0;
          OldTraceMask = TC_SYSTEM;
          TraceMask = 0;
        }
        else 
        {
          if ( !strcmp ( "SYST", s_hdr->rcv ) )
          {
            DestTaskHandle = 0;
            vsi_gettracemask ( DestTaskHandle, 0, &OldTraceMask);
            TraceMask = TC_SYSTEM;
          }
          else
          {
            OldTraceMask = 0;
            TraceMask = 0;
          }
        }
      }
      else
      {
        /* to satisfy LINT */
        OldTraceMask = 0;
        TraceMask = 0;
      }
      if ( DestTaskHandle != VSI_ERROR )
      {
        if ( prim->opc & SYS_MASK )
        {
          Length = GetNextToken ((char*)(P2D(prim)), token, "  #");
          Offset = Length+1;
          if ( frmenv == ENV_STACK )
          {
            /* Set Traceclass for non-frame tasks/HISRs */
            if ( DestTaskHandle == 0 && !strcmp (token, SYSPRIM_TRACECLASS_TOKEN) )
            {
              Length = GetNextToken ((char *)(P2D(prim))+Offset, token, " #");
              TraceMask = ASCIIToHex(token, CHARS_FOR_32BIT);
              vsi_settracemask ( DestTaskHandle, 0, TraceMask|OldTraceMask); /* it is not allowed to switch of system traces */
              sprintf ( token, "%s (%s %s)", ok_string, (char*)(P2D(prim)), s_hdr->rcv );
              tst_drv_write ( NO_TASK, 0, NULL, token );
              processed = TRUE;
            }
            if (!strcmp (token, SYSPRIM_TRC_SUSPEND))
            {
              processed = TRUE;
              /*
               * set suspend for traces to SUSPEND or NO_SUSPEND
               */
              Length = GetNextToken ((char *)(P2D(prim))+Offset, token, " #");
              Offset += (Length+1);
 
              if ( !strcmp ( token, "ALL" ) )
              {  
                min = 1; 
                max = MaxEntities;
                GetNextToken ((char *)(P2D(prim))+Offset, token, " #");
              }
              else
              { 
                min = DestTaskHandle; 
                max = min; 
              }
              if ( !strcmp ( token, "YES" ) )
                Suspend = 0xffffffff;
              else
                Suspend = 0;
     
              for ( i = min; i <= max; i++ ) 
                vsi_trcsuspend ( TST_Handle, i, Suspend );
              sprintf ( token, "%s (%s %s)", ok_string, (char*)(P2D(prim)), s_hdr->rcv );
              tst_drv_write ( NO_TASK, 0, NULL, token );
            }
            else if (!strcmp (token, SYSPRIM_MEMCHECK_TOKEN))
            {
              processed = TRUE;
              /*
               * Memory Check
               */
              for ( i = 1; i <= MaxEntities; i++ )
              {
                 if ( vsi_object_info (TST_Handle, OS_OBJTASK, (USHORT)i, token, sizeof(token)) != VSI_ERROR )
                 {
                   tst_drv_write ( NO_TASK, 0, NULL, token );
                 }
              }
            }
#ifdef CTB
#ifndef _TOOLS_
            else if (!strcmp (token, SYSPRIM_TIMER_TICK_REQ)) 
            {
               processed = TRUE;
               ctb_sent_to_tap = FALSE;
               if(strlen(ctb_rcv)<3)
               {
                  strcpy(ctb_rcv, P_SND(prim));
               }
               ctb_remaining_tick_time = P_TIME(prim);  //The time parameter is sent in the time stap.
               /*sprintf ( token, "Requesting ticking for %d from %s", ctb_remaining_tick_time, ctb_rcv);
               tst_drv_write ( NO_TASK, 0, NULL, token );*/
               PSIGNAL(hCommIDLE, IDLE_REQ, NULL);
            }
            else if (!strcmp (token, SYSPRIM_INT_TICK_MODE_REQ)) 
            {
              processed = TRUE;
              if(ctb_tick_enabled)
              {               
                vsi_p_delete (TST_Handle,idle_handle); 
                ctb_tick_enabled = FALSE;
                ctb_sent_to_tap = FALSE;
                sprintf ( token, "Disabling Common Timer Base");
                ctb_rcv[0]='\0';
                os_StartTicking();
              }
              else
                sprintf ( token, "Common Timer Base already disabled");
              tst_drv_write ( NO_TASK, 0, NULL, token );
              if(!strcmp(P_SND(prim), "TAP")) 
              {
                tst_drv_write ( NO_TASK, SYS_MASK, "TAP", "INT_TICK_MODE_CNF");
              }
            }
            else if (!strcmp (token, SYSPRIM_EXT_TICK_MODE_REQ)) 
            {
              processed = TRUE;
              if(!ctb_tick_enabled)
              {
                if ( (idle_handle = vsi_p_create (TST_Handle, idle_pei_create, NULL, 1)) == VSI_ERROR )
                {
                  vsi_o_assert ( TST_Handle, OS_SYST_ERR, __FILE__, __LINE__, "CTB: Cannot create IDLE task" );
                }
                vsi_p_start (TST_Handle, idle_handle);
                while ( (hCommIDLE = vsi_e_handle ( TST_Handle, "IDLE" ))<VSI_OK)
                {
	                vsi_t_sleep(0,1000);
                }
                os_StopTicking();
                sprintf ( token, "Enabling Common Timer Base");
     	          ctb_tick_enabled = TRUE;
              }
              else
                sprintf ( token, "Common Timer Base already enabled");
   			      tst_drv_write ( NO_TASK, 0, NULL, token );
				      if(!strcmp(P_SND(prim), "TAP")) 
              {
                char send_str[50];
                char tmp_str[8];
                //Get process id and put in this syst primitive.
                strcpy(send_str,"EXT_TICK_MODE_CNF#");
                _itoa(os_GetProcessId(), tmp_str, 10);
                strcat(send_str, tmp_str);
                tst_drv_write ( NO_TASK, SYS_MASK, "TAP", send_str);
              }
	          }
#endif
#endif //CTB

#ifndef _TARGET_
            else if (!strcmp (token, SYSPRIM_SUPPRESS_OK))
            {
              processed = TRUE;
              SuppressOK = TRUE;
            }  
#endif
            else if (!strcmp (token, SYSPRIM_GET_STACK_TIME))
            {
              processed = TRUE;
              tst_drv_write ( NO_TASK, SYS_MASK, FRM_TST_NAME, (char*)SYSPRIM_IS_STACK_TIME );
            }  
            else if (!strcmp (token, SYSPRIM_READ_ROUTING))
            {
              processed = TRUE;
              i = 0;
              while ( rt_RouteRead ( DestTaskHandle, token ) != RT_ERROR )
              {
                tst_drv_write ( NO_TASK, 0, NULL, token );
                i++;
              }
              if ( !i )
              {
                sprintf ( token, "NO %s ROUTINGS STORED", s_hdr->rcv );
                tst_drv_write ( NO_TASK, 0, NULL, token );
              }
            }
#ifndef _TOOLS_
            else if (!strcmp (token, SYSPRIM_STR2IND_VERSION))
            {
              processed = TRUE;
              tst_drv_write ( NO_TASK, 0, NULL, str2ind_version );
            }
#endif
          }
        }
      }
#ifdef _TOOLS_
      else
      {
        /* synchronization with protocol stack */
        if ( tst_sync_mode == 1 )
        {
          char *pos;
          char task_status;
          T_VOID_STRUCT *cmd_ptr;

          pos = strstr ( (char*)(P2D(prim)), "Name:" );
          if ( pos != NULL )
          {
            tst_sync_started = 1;
            GetNextToken ((char *)(P2D(prim))+strlen("Name:"), token, " #");
            if ( strcmp(token,FRM_RCV_NAME) && strcmp(token,FRM_TST_NAME) && strcmp(token,"EXTR") )
            {
              pos = strstr ( (char*)(P2D(prim)), "Stat:" );
              if ( pos != NULL )
              {
                tst_status_received = 1;
                task_status = atoi(pos+strlen("Stat:"));
                if ( task_status != 6 )
                {
                  tst_sync_sucess = 0;
                }
              }
            }
          }
          else
          {
            if ( tst_sync_started == 1 )
            {
              pos = strstr ( (char*)(P2D(prim)), "ROUTING" );
              if ( pos != NULL )
              {
                if ( tst_sync_sucess == 1 && tst_status_received == 1 )
                {
                  T_HANDLE tif_handle = vsi_d_open ( TST_Handle, (char*)TIF_NAME );
                  vsi_d_setconfig ( TST_Handle, tif_handle, DISABLE_SYNC_MODE );
                  vsi_t_stop ( TST_Handle, TST_SYNC_TIMER_INDEX );
                  cmd_ptr = vsi_c_pnew ( sizeof(T_PRIM_HEADER)+strlen(SYSPRIM_TST_SYNC_CNF), 0x8000 FILE_LINE );
                  strcpy ( (char*)cmd_ptr, SYSPRIM_TST_SYNC_CNF );
                  vsi_o_ttrace ( TST_Handle, TC_TIMER, "SYNC DONE") ;
                  vsi_c_psend ( tst_sync_req_handle, cmd_ptr );
                  tst_sync_mode = 0;
                  tst_sync_timeout_cnt = 0;
                  tst_syncronized = 1;
                }
              }
            }
          }
        }
      }
#endif /* _TOOLS_ */
      if ( processed == FALSE )
      {
        if ( ( DestComHandle = vsi_c_open ( TST_Handle, s_hdr->rcv ) ) != VSI_ERROR )
        {
          /* free carrier */
          PFREE(P2D(sig_ptr));
          FreePrim = 0;
          vsi_c_psend (DestComHandle, (T_VOID_STRUCT*)P2D(prim) FILE_LINE_MACRO);
        }
        else 
        {
#ifndef _TOOLS_
          if ( frmenv == ENV_STACK )
          {
            sprintf ( token, "SYSTEM WARNING: Receiver Process '%s' unknown", s_hdr->rcv );
            tst_drv_write ( NO_TASK, 0, NULL, token );
            ret = PEI_ERROR;
          }
#endif /* _TOOLS_ */
          /* free dyn_ptr if unknown receiver */
          FREE(P2D(prim));
        }
      }
      else
      {
        /* free dyn_ptr if processed in TST */
        FREE(P2D(prim));
      }
    }

    if ( FreePrim ) 
    {
      /* free carrier */
      PFREE(P2D(sig_ptr));
    }
    break;
    case DRV_SIGTYPE_CONNECT:
      TST_DrvState = TST_DRV_CONNECTED;
#ifndef _TOOLS_
      tst_drv_write ( NO_TASK, SYS_MASK, FRM_TST_NAME, (char*)SYSPRIM_IS_STACK_TIME );
      tst_drv_write ( NO_TASK, 0, NULL, str2ind_version );
#endif
    break;
    case DRV_SIGTYPE_DISCONNECT:
      TST_DrvState = TST_DRV_DISCONNECTED;
#ifdef _PSOS_
      /* for pSOS: send empty message to TST */
      if ( vsi_d_open ( TST_Handle, "SOCKET" ) != VSI_ERROR )
      {
        T_QMSG Message;
        static T_HANDLE tst_handle = 0;

        if( TST_Handle > 0 ) 
        {
          Message.MsgType = MSG_PRIMITIVE;
          Message.Msg.Primitive.Prim = NULL;
          Message.Msg.Primitive.PrimLen = 0;
#ifdef MEMORY_SUPERVISION
          vsi_c_send (TST_Handle, TST_Handle, &Message, __FILE__, __LINE__);
#else
          vsi_c_send (TST_Handle, TST_Handle, &Message);
#endif
        }
      }
#endif
    break;
#ifdef CTB
    case IDLE_CNF:
     if(ctb_remaining_tick_time > 0 && ctb_tick_enabled && !ctb_sent_to_tap) 
     {
       os_Tick();
       PSIGNAL(hCommIDLE, IDLE_REQ, NULL);
       ctb_remaining_tick_time = ctb_remaining_tick_time-50;
     }
     else
     {
       /*sprintf ( token, "Ticking finished - remaining time %d", ctb_remaining_tick_time);
       tst_drv_write ( NO_TASK, 0, NULL, token );*/
       if(!strcmp(ctb_rcv, "TAP"))
         tst_drv_write ( NO_TASK, SYS_MASK, FRM_TST_NAME, SYSPRIM_IS_STACK_TIME ); //Synchronize time with tools
       tst_drv_write ( NO_TASK, SYS_MASK, ctb_rcv, "TIMER_TICK_CNF");
     }
    break;
#endif
    default:
        sprintf ( token, "Unhandled PSIGNAL");
        tst_drv_write ( NO_TASK, 0, NULL, token );
    break;
  }
  return ( ret );
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : TST_PEI             |
| STATE   : code                       ROUTINE : pei_config          |
+--------------------------------------------------------------------+ 
*/

LOCAL SHORT pei_config (char * inString)
{
T_HANDLE drv_handle;
char token[80];
unsigned int length, offset;
BOOL select = 0,config = 0;
T_TST_DRV_ENTRY *tst_drv_info;

  length = GetNextToken (inString, token, " #");
  offset = length+1;
  if ( !strcmp ( token, "DRIVER") )
  {
    length = GetNextToken (inString+offset, token, " #");
    offset += (length+1);
    if ( !strcmp ( token, "FLUSH") )
    {
     vsi_d_flush ( TST_Handle, 0 );
    }
    if ( !strcmp ( token, "SELECT") )
    {
     select = 1;
    }
    if ( !strcmp ( token, "CONFIG") )
    {
      config = 1;
#ifdef _TOOLS_
      tst_syncronized = 0;
#endif
    }
    length = GetNextToken (inString+offset, token, " #");
    offset += (length+1);
    if ( select )
    {
#ifdef _TOOLS_
      if ( !strcmp(NODRV_NAME,token) )
      {
        if ( tst_drv_open ( token, &tst_drv_info ) == VSI_ERROR ||
             vsi_d_create ( TST_Handle, tst_drv_info ) == VSI_ERROR)
        {
          return PEI_ERROR;
        }
        PrintToFile("TST: all drivers unloaded\n");
      }
      else
#endif
      {
        /* check if driver is already loaded */
        if ( (drv_handle = vsi_d_open ( TST_Handle, token )) == VSI_ERROR )
        {
          if ( tst_drv_open ( token, &tst_drv_info ) == VSI_ERROR ||
               vsi_d_create ( TST_Handle, tst_drv_info ) == VSI_ERROR )
          {
            return PEI_ERROR;
          }
        } 
        else 
        {
#ifdef _TOOLS_
#ifdef _DEBUG
          PrintToFile("TST: keeping %s loaded\n",token);
#endif
#endif
          vsi_d_close( TST_Handle, drv_handle );
        }
      }
    }
    if ( config )
    {
      if ( ( drv_handle = vsi_d_open ( TST_Handle, token ) ) == VSI_ERROR )
      {
        return PEI_ERROR;
      }
      if (vsi_d_setconfig ( TST_Handle, drv_handle, inString+offset) != VSI_OK)
      {
#ifdef _TOOLS_
        char text[99];
        _snprintf(text,98,"TST: Error configuring driver %s with \"%s\" :-(",token, inString+offset);
        vsi_o_ttrace(NO_TASK, TC_SYSTEM, text);
#endif /* _TOOLS_ */
      }
      vsi_d_close( TST_Handle, drv_handle );
    }
  }  
  else if ( !strcmp ( token, "THIF") )
  {
    T_HANDLE tif_handle;
    tif_handle = vsi_d_open ( TST_Handle, (char*)TIF_NAME );
    length = GetNextToken (inString+offset, token, " #");
    offset += (length+1);

    if ( !strcmp ( token, "OPEN") )
    {
      vsi_d_setconfig ( TST_Handle, tif_handle, token );
    }
    else if ( !strcmp ( token, "CLOSE") )
    {
      vsi_d_setconfig ( TST_Handle, tif_handle, token );
    }
  }
#ifdef _TOOLS_
  else if ( !strcmp ( token, SYSPRIM_TST_SYNC_REQ) )
  {
    T_HANDLE tif_handle;
    unsigned int len;

    len = GetNextToken (inString+offset, sync_req_name, " #");
    offset += len;
    len = GetNextToken (inString+offset, sync_req_time, " #"); 
    tst_max_sync_timeout = (atoi(sync_req_time) - 1000)/TST_SYNC_TIMER_DURATION;

    tst_sync_req_handle = vsi_c_open ( TST_Handle, sync_req_name );
    if ( tst_sync_req_handle != VSI_ERROR )
    {
      if ( tst_syncronized == 0 )
      {
        tst_sync_started = 0;
        tst_status_received = 0;
        tst_sync_mode = 1;
        tst_sync_sucess = 1;
        tif_handle = vsi_d_open ( TST_Handle, (char*)TIF_NAME );
        vsi_d_setconfig ( TST_Handle, tif_handle, ENABLE_SYNC_MODE );
        tst_drv_write ( NO_TASK, SYS_MASK, FRM_RCV_NAME, (char*)"TRACECLASS 0x10" );
        vsi_t_sleep ( TST_Handle, 100 );
        tst_drv_write ( NO_TASK, SYS_MASK, FRM_RCV_NAME, (char*)"STATUS TASK" );
        vsi_t_start ( TST_Handle, TST_SYNC_TIMER_INDEX, TST_SYNC_TIMER_DURATION );
        vsi_t_sleep ( TST_Handle, 200 );
        tst_drv_write ( NO_TASK, SYS_MASK, FRM_RCV_NAME, (char*)"ROUTING" );
      }
    }
  }
#endif
#ifdef _FF_RV_EXIST_
  else if ( !strcmp ( token, "TRACEMASK_IN_FFS") )
  {
    int amount;
    T_FFS_FD fd;
    T_FFS_RET ret;
    T_FFS_SIZE written;
    T_FFS_DIR dir;
    BOOL writeFailed = FALSE;
    if (vsi_e_handle ( TST_Handle, FRM_RCV_NAME ) == e_running[os_MyHandle()]) // config prim processing only allowed in RCV, not TST!
    {
      fd = ffs_open(TRACEMASKFILE,  FFS_O_RDWR | FFS_O_CREATE | FFS_O_TRUNC);
      if (fd < 0)
      {
        // could not open nor create /var/dbg/tracemask warning
        vsi_o_ttrace(NO_TASK, TC_SYSTEM, "pei_config: could not open/create FFS trace mask, reason is 0x%x", fd);
      }
      else
      {
        TraceMask[0] = 0;
        TraceMask[0] |= TC_SYSTEM|TC_ERROR;
        amount = sizeof(ULONG) * (MaxEntities + 1);
        written = ffs_write(fd, (void*)&TraceMask[0], amount);
        if (written != amount)
        {
          if (written >= 0)
          {
            amount -= written;
            vsi_o_ttrace(NO_TASK, TC_SYSTEM, "pei_config: writing to  FFS, second try");
            written = ffs_write(fd, (void*)&TraceMask[0], amount);
            if (written != amount) 
            {
              writeFailed = TRUE;
            }
          }
          else
          {
            writeFailed = TRUE;
          } 
        }
        if (writeFailed)
        {
          vsi_o_ttrace(NO_TASK, TC_SYSTEM, "pei_config: ffs_write to FFS failed with 0x%x, did open with", written, fd);
        }
        else
        {
          if (ffs_fdatasync(fd) == EFFS_OK)
          {
            vsi_o_ttrace(NO_TASK, TC_SYSTEM, "pei_config: successfully written trace mask to FFS");
          }
          else
          {
            vsi_o_ttrace(NO_TASK, TC_SYSTEM, "pei_config: flushing FFS trace mask failed!");
          }
        }
        ffs_close(fd);
      }
    }
  }
  else if ( !strcmp ( token, "NO_TRACEMASK_IN_FFS") )
  {
    T_FFS_RET ret;

    InitializeTrace();
    TraceMask[0] = 0;
    TraceMask[0] |= TC_SYSTEM|TC_ERROR;
    if (vsi_e_handle ( TST_Handle, FRM_RCV_NAME ) == e_running[os_MyHandle()]) // config prim processing only allowed in RCV, not TST!
    {
      ret = ffs_remove(TRACEMASKFILE);
      if (ret != EFFS_OK)
      {
       vsi_o_ttrace(NO_TASK, TC_SYSTEM, "pei_config: failed to remove FFS trace mask, reason is 0x%x", ret);
      }
      else
      {
        vsi_o_ttrace(NO_TASK, TC_SYSTEM, "pei_config: successfully removed FFS trace mask");
      }
    }
  }
#endif
  return PEI_OK;
}
#endif

#ifndef RUN_INT_RAM
GLOBAL SHORT tstsnd_pei_create ( T_PEI_INFO ** info)
{
  static T_PEI_INFO data = 
  { "TST",  
    { 
      pei_init,
      pei_exit,
      tst_pei_primitive,
#ifdef _TOOLS_
      pei_timeout,
#else
      NULL,
#endif
      NULL,
      NULL,
      pei_config,
      NULL 
    },
    TST_STACKSIZE, 
    0,
    TST_SND_PRIORITY,
    TST_TIMERS,
    (PASSIVE_BODY|COPY_BY_REF|SYSTEM_PROCESS|TRC_NO_SUSPEND)
  };

#ifdef _TOOLS_
  data.QueueEntries = TST_QUEUE_ENTRIES;
#else
  data.QueueEntries = TST_SndQueueEntries;
  /* 
   * This way of setting the TST and RCV stacksize is chosen to keep it backwardscompatible,
   * i.e. not change the behavior if the stacksizes are not define in the configuration
   * file xxxconst.h.
   */
  if ( TST_SndStacksize > 0 )
  {
    data.StackSize = TST_SndStacksize;
  }
  if ( pcon != NULL )
  {
    data.StackSize += pcon->stack_offset;
  }
#endif
  *info = &data;
  return PEI_OK;
} 
#endif

#ifndef RUN_INT_RAM
GLOBAL SHORT tstrcv_pei_create ( T_PEI_INFO ** info)
{
  static T_PEI_INFO data = 
  { "RCV",  
    { 
      NULL,
      NULL,
      NULL,
      NULL,
      pei_signal,
      NULL,
      pei_config,
      NULL 
    },
    TST_STACKSIZE, 
    0,
    TST_RCV_PRIORITY,
    TST_TIMERS,
    (PASSIVE_BODY|COPY_BY_REF|SYSTEM_PROCESS|TRC_NO_SUSPEND)
  };

#ifdef _TOOLS_
  data.QueueEntries = TST_QUEUE_ENTRIES;
#else
  data.QueueEntries = TST_RcvQueueEntries;
  /* 
   * This way of setting the TST and RCV stacksize is chosen to keep it backwardscompatible,
   * i.e. not change the behavior if the stacksizes are not define in the configuration
   * file xxxconst.h.
   */
  if ( TST_RcvStacksize > 0 )
  {
    data.StackSize = TST_RcvStacksize;
  }
  if ( pcon != NULL )
  {
    data.StackSize += pcon->stack_offset;
  }
#endif
  *info = &data;
  return PEI_OK;
} 
#endif

