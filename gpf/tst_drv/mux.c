/* 
+------------------------------------------------------------------------------
|  File:       mux.c
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
|  Purpose :  This Module defines the adaptation of the MUX_tr driver
|             to the GDI.
+----------------------------------------------------------------------------- 
*/ 

#ifdef __MUX_C__
#define __MUX_C__
#endif

#include "stdlib.h"
#include <stdio.h>
#include "string.h"
#include "typedefs.h"

#include "tools.h"
#include "os.h"
#include "gdi.h"
#include "vsi.h"
#include "glob_defs.h"
#undef VSI_CALLER
#include "frame.h"
#include "prim.h"       /* to get the definitions of used SAP and directions */
#include "gsm.h"
#include "pei.h"
#include "route.h"
#include "dti.h"        /* functionality of the dti library */
#include "mux.h"        /* to get the global entity definitions */
#include "frame.h"

/*==== TYPES ======================================================*/

/*==== CONSTANTS ==================================================*/

#define VSI_CALLER        m_tst_handle,
#define VSI_CALLER_SINGLE m_tst_handle

#define ALLOWED_MUX_SIGNALS   (DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT) 

#define TST_NAME  "TST"
/*==== EXTERNALS ==================================================*/

extern T_HANDLE ext_data_pool_handle;
extern T_QMSG QueueMsg;

/*==== GLOBAL VARIABLES ==================================================*/

/*==== LOCAL VARIABLES ==================================================*/
static T_DRV_SIGNAL Signal;

T_HANDLE  m_tst_handle;

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : MUX                 |
| STATE   : code                       ROUTINE : mux_Exit            |
+--------------------------------------------------------------------+

  PURPOSE : exit a driver

*/
void mux_Exit ( void )
{
//vsi_p_delete (0, vsi_p_handle (0,"MTST"));
  mux_data.Connected = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : MUX                 |
| STATE   : code                       ROUTINE : mux_Read            |
+--------------------------------------------------------------------+

  PURPOSE : read data from driver

*/
USHORT mux_Read ( void *Buffer, ULONG *BytesToRead )
{
  T_desc2 *d;
  T_desc2 *d_new;

  /* copy data into buffer */
  *BytesToRead=0;
  d=mux_data.recv_data;
  while (d)
  {
    d_new=(T_desc2 *)d->next;
    *BytesToRead += d->len;
    memcpy(Buffer, d->buffer, d->len);
    /* free memory */
    MFREE(d);

    d=d_new;
  }

  if (mux_data.dti_state NEQ IDLE) /* not yet configured */
  {
    return DRV_NOTCONFIGURED;
  } 
  else
  {
    /* inform UART that we are ready to receive next data package */
    dti_start
    (
      mtst_hDTI,
      MTST_DTI_DN_INSTANCE,
      MTST_DTI_DN_INTERFACE,
      MTST_DTI_DN_CHANNEL
    );
  }

  return DRV_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : MUX                 |
| STATE   : code                       ROUTINE : mux_Write           |
+--------------------------------------------------------------------+

  PURPOSE : write data to driver

*/
USHORT mux_Write ( void *Buffer, ULONG *BytesToWrite )
{
  USHORT ret = DRV_OK;

  if (mux_data.h_comm_mtst==VSI_ERROR) /* not yet configured */
  {
    ret=DRV_NOTCONFIGURED;
  }
  else 
  {
    T_desc2 *d;
    T_QMSG *pMsg = &QueueMsg;

    PALLOC(localPrimPtr, DTI2_DATA_IND);

    /* fill in parameters */
    localPrimPtr->parameters.p_id=DTI_PID_UOS;  /* protocol identifier  */
       /* flow control state */
    localPrimPtr->parameters.st_lines.st_flow=DTI_FLOW_ON;   
       /* line state sa      */
    localPrimPtr->parameters.st_lines.st_line_sa=DTI_SA_ON;  
       /* line state sb      */       
    localPrimPtr->parameters.st_lines.st_line_sb=DTI_SB_ON;
       /* break length       */       
    localPrimPtr->parameters.st_lines.st_break_len=DTI_BREAK_OFF;

      /* list of generic data descriptors */
    MALLOC(d,(ULONG)(sizeof(T_desc2)+*BytesToWrite-1));
    localPrimPtr->desc_list2.first=(ULONG)d;
    localPrimPtr->desc_list2.list_len= (USHORT)*BytesToWrite;
    d->next   = 0;
    d->len    = (USHORT)*BytesToWrite;
    d->size   = *BytesToWrite;
    d->offset =  0;
    memcpy(d->buffer,Buffer,*BytesToWrite);

    /* PSEND without tracing */
    pMsg->Msg.Primitive.Prim = (T_VOID_STRUCT*)(D2P(localPrimPtr));
    pMsg->Msg.Primitive.PrimLen = PSIZE(localPrimPtr);
    pMsg->MsgType = MSG_PRIMITIVE;
#ifdef MEMORY_SUPERVISION
    vsi_c_send (VSI_CALLER mux_data.h_comm_mtst,pMsg,__FILE__,__LINE__);
#else
    vsi_c_send (VSI_CALLER mux_data.h_comm_mtst,pMsg);
#endif
  }

  return ( ret );
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : MUX                 |
| STATE   : code                       ROUTINE : mux_SetSignal       |
+--------------------------------------------------------------------+

  PURPOSE : enable signal for the driver

*/
USHORT mux_SetSignal ( USHORT SignalType )
{
	if ( !(SignalType & ALLOWED_MUX_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    mux_data.EnabledSignalType |= SignalType;
  
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : MUX                 |
| STATE   : code                       ROUTINE : mux_ResetSignal     |
+--------------------------------------------------------------------+

  PURPOSE : disable signal for the driver

*/
USHORT mux_ResetSignal ( USHORT SignalType )
{
	if ( !(SignalType & ALLOWED_MUX_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    mux_data.EnabledSignalType &= ~SignalType;
  
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : MUX                 |
| STATE   : code                       ROUTINE : mux_SetConfig       |
+--------------------------------------------------------------------+

  PURPOSE : set configuration for the driver

  PARAMS:   Buffer - configuration string ("<DTI-partner> <tui> <c_id>")
           

*/
USHORT mux_SetConfig ( char *Buffer )
{
  char token [10];
  char peer_name [10];
  USHORT len;
  ULONG link_id;

  if ( !mux_data.Connected )
  {
    Signal.SignalType = DRV_SIGTYPE_CONNECT;
	  Signal.DrvHandle = mux_data.Handle;
    (mux_data.Callback)( &Signal );
    mux_data.Connected = TRUE;

    return DRV_OK;
  }

  /* interprete configuration string */
  if (!(len = GetNextToken (Buffer, token, " #")))
  {
    return DRV_INVALID_PARAMS;
  }
  else
  {
    Buffer += (len+1);
  }
  if (strcmp(token,"STOP")==0) 
  {
    dti_close
    (
      mtst_hDTI,
      MTST_DTI_DN_INSTANCE,
      MTST_DTI_DN_INTERFACE,
      MTST_DTI_DN_CHANNEL,
      FALSE
    );

    mux_data.dti_state=CLOSED;
    mux_data.h_comm_mtst=VSI_ERROR;
    /* xxxxx set m_uart_ready = 0 ?! */
    if (mux_data.send_data_buf_count>0) {
      /* xxxxx clear send_buffer ?! */
      mux_data.send_data_buf_count=0;
    }

    return DRV_OK;
  }
  strcpy(peer_name, token);
  
  if (!(len = GetNextToken (Buffer, token, " #")))
  {
    return DRV_INVALID_PARAMS;
  }
  else
  {
    Buffer += (len+1);
  }
  link_id=atoi(token);

  if(
    dti_open
    (
      mtst_hDTI,
      MTST_DTI_DN_INSTANCE,
      MTST_DTI_DN_INTERFACE,
      MTST_DTI_DN_CHANNEL,
      DTI_QUEUE_DISABLED, /* DTI_QUEUE_UNLIMITED in order to queue data */
      DTI_CHANNEL_TO_LOWER_LAYER,
      FLOW_CNTRL_ENABLED,
      DTI_VERSION_10,
      (U8*)peer_name,
      link_id
    ) EQ FALSE)
    return DRV_INVALID_PARAMS;

  /* reset send_data_buf counter */
  mux_data.send_data_buf_count=0;

  /* set internal communication handle */
  while ( (mux_data.h_comm_mtst=vsi_c_open (0, "MTST")) == VSI_ERROR)
  {
    vsi_t_sleep(0,100);
  };

  /* inform UART that we are ready to receive next data package */
  {
    dti_start
    (
      mtst_hDTI,
      MTST_DTI_DN_INSTANCE,
      MTST_DTI_DN_INTERFACE,
      MTST_DTI_DN_CHANNEL
    );
  }

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : MUX                 |
| STATE   : code                       ROUTINE : mux_Init            |
+--------------------------------------------------------------------+

  PURPOSE : initialize driver

*/
USHORT mux_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo )
{ 
  T_HANDLE mux_handle;

static const T_DRV_EXPORT mux_Info =
{
  "MUX",
  CALLED_FROM_ISR,
  {
    /*mux_Init,*/
    mux_Exit,
    mux_Read,
    mux_Write,
    NULL,
    NULL,
    NULL,
    mux_SetSignal,
    mux_ResetSignal,
    mux_SetConfig,
    NULL,
    NULL,
  }
};

  mux_data.Handle = DrvHandle;

  mux_data.EnabledSignalType = 0;

  mux_data.Callback = CallbackFunc;

  mux_data.Connected = FALSE;

  mux_data.dti_state = CLOSED;
  mux_data.h_comm_mtst = VSI_ERROR;

  mux_handle = vsi_p_create (0, mux_pei_create, NULL, ext_data_pool_handle);
  if (vsi_p_start (0, mux_handle) != VSI_OK)
  {
    return DRV_INITFAILURE;
  };

  *DrvInfo = &mux_Info;

  m_tst_handle=vsi_c_open (0,TST_NAME);

  return DRV_OK;           
}

