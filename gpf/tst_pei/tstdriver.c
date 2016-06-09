/* 
+------------------------------------------------------------------------------
|  File:       tstdriver.c
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
|  Purpose :  This Modul contains a table of all the drivers that may be
|             used for the test interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __TST_DRV_C__
#define __TST_DRV_C__
#endif
 
/*==== INCLUDES ===================================================*/

#include <string.h>

#include "gpfconf.h"
#include "typedefs.h"
#include "os.h"
#include "vsi.h"
#include "gdi.h"
#include "drvconf.h"
#include "tstdriver.h"
#include "tst_mux.h"

/*==== TYPES ======================================================*/


/*==== VARIABLES ==================================================*/

T_TST_MUX_CHANNEL tst_mux_chan_struct[ MAX_TST_CHANNEL ];
T_HANDLE tst_mux_drv_handle;

/*
 * just a hack - clean up needed
 */
#define MAX_PROT_PRIM_SIZE 236
ULONG DrvSndData[(MAX_PROT_PRIM_SIZE + sizeof(T_PRIM_HEADER) + 3) / 4];
ULONG X_PrimData[(sizeof(T_PRIM_X) + sizeof(T_S_HEADER) + 3)  / 4 ]; 

/*==== EXTERNALS ==================================================*/

extern T_HANDLE TST_Handle;
extern T_HANDLE TIF_Handle;
extern UBYTE TST_DrvState;

#ifndef _TARGET_
extern USHORT TIF_Init      ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                              T_DRV_EXPORT const **DrvInfo );
extern USHORT TR_Init       ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                              T_DRV_EXPORT const **DrvInfo );
extern USHORT NODRV_Init    ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                              T_DRV_EXPORT const **DrvInfo );
extern USHORT socket_Init   ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                              T_DRV_EXPORT const **DrvInfo );
extern USHORT SER_Init      ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                              T_DRV_EXPORT const **DrvInfo );
#endif

/*==== CONSTANTS ==================================================*/
#ifdef _TARGET_
#ifndef RUN_INT_RAM
const T_TST_DRV_ENTRY tst_drv_list[ MAX_AVAILABLE_DRV ] =
{
  { { NULL,            NULL,           NULL,   NULL }, 0 }
};
#endif /* RUN_INT_RAM */
#else /* _TARGET_ */

const T_TST_DRV_ENTRY tst_drv_list[ MAX_AVAILABLE_DRV ] =
{
  { { TIF_NAME,        TIF_Init,       "TST",  NULL }, 1 },
  { { TR_NAME,         TR_Init,        NULL,   NULL }, 2 },
  { { SOCKET_NAME,     socket_Init,    NULL,   NULL }, 3 },
#if !defined (_LINUX_) && !defined (_SOLARIS_)
  { { SER_NAME,        SER_Init,       NULL,   ""   }, 3 },
#endif
  { { NODRV_NAME,      NODRV_Init,     NULL,   NULL }, 3 },
  { { NULL,            NULL,           NULL,   NULL }, 0 }
};
#endif /* _TARGET_ */


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/


#ifndef _TARGET_
/*
+--------------------------------------------------------------------+
| PROJECT : GPF                        MODULE  : TSTDRIVER           |
| STATE   : code                       ROUTINE : NODRV_Init          |
+--------------------------------------------------------------------+ 

  PURPOSE : initialize empty driver

*/
GLOBAL USHORT NODRV_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo )
{                
static const T_DRV_EXPORT NODRV_Info =
{
  NODRV_NAME,
  0,
  {
#ifdef _TOOLS_
    NULL,
#endif
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
  }
};

  *DrvInfo = &NODRV_Info;
  return DRV_OK;           
}
#endif /* ndef _TARGET_ */


#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GPF                        MODULE  : TSTDRIVER           |
| STATE   : code                       ROUTINE : tst_drv_open        |
+--------------------------------------------------------------------+ 
*/

SHORT tst_drv_open (char *drv_name, T_TST_DRV_ENTRY **drv_info )
{
USHORT i;

  for ( i = 0; i < MAX_AVAILABLE_DRV; i++ )
  {
    if ( tst_drv_list[i].entry.Name && drv_name 
      && !strcmp ( drv_name, tst_drv_list[i].entry.Name ) )
    {
      *drv_info = (T_TST_DRV_ENTRY*)&tst_drv_list[i];
      return VSI_OK;
    }
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : TSTDRIVER           |
| STATE   : code                       ROUTINE : tst_drv_write       |
+--------------------------------------------------------------------+ 

  PURPOSE:  Send a message via the test interface 

*/
GLOBAL SHORT tst_drv_write ( T_HANDLE caller, ULONG opc, char *dest, char *Buffer )
{
T_PRIM_HEADER *prim;
T_S_HEADER *s_hdr;
T_VOID_STRUCT *ptr = (T_VOID_STRUCT*)Buffer;  /* just to reduce number of alignment warnings */
T_VOID_STRUCT *snd_ptr;
T_PRIM_X *x_prim = (T_PRIM_X*)X_PrimData;

  x_prim->prim_ptr = NULL;
  prim = (T_PRIM_HEADER*)DrvSndData;
  prim->opc = opc;
  if ( opc == 0 || opc == SYS_MASK )          /* opc = 0 -> trace -> to PCO */
  {
    prim->len = sizeof(T_PRIM_HEADER);
    if ( Buffer != NULL )
    {
      prim->len += strlen(Buffer);
      strcpy ((char*)P2D(prim), Buffer );
    }
  }
  else
  {
    x_prim->prim_ptr = prim;
    x_prim->p_hdr.opc = opc;
    if ( D_LEN(ptr) <= MAX_PROT_PRIM_SIZE )
    {
      prim->len = D_LEN(ptr);
      memcpy ((char*)P2D(prim), Buffer, (D_LEN(ptr)-sizeof(T_PRIM_HEADER)));
    }
    else
    {
      /*
       * modify type to trace and send warning 
       */ 
      prim->opc = 0;
      strcpy ((char*)P2D(prim), "Error: DirectDrvWrite -> Primitive to large to be transmitted");
      prim->len = strlen((char*)P2D(prim)) + sizeof(T_PRIM_HEADER);
    }
  }
  prim->sh_offset = S_HDR_OFFSET(prim->len);
  s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);
  os_GetTime(TST_Handle,&s_hdr->time);
  s_hdr->snd[0] = (char)caller;
  if ( caller )
    s_hdr->snd[0] |= (char)HANDLE_BIT;
  if ( dest )
    strcpy (s_hdr->rcv, dest);

  if ( TST_DrvState == TST_DRV_CONNECTED )
  {
     if ( x_prim->prim_ptr != NULL )
     {
       /*lint -e419 suppress - Warning -- Apparent data overrun for function 'memcpy... */
       memcpy ( ((char*)x_prim) + sizeof(T_PRIM_X), s_hdr, sizeof(T_S_HEADER) );
       /*lint +e419 */
       x_prim->p_hdr.sh_offset = sizeof(T_PRIM_X)>>2;
       snd_ptr = (T_VOID_STRUCT*)x_prim;
     }
     else
       snd_ptr = (T_VOID_STRUCT*)prim;
     if ( vsi_d_write ( TST_Handle, TIF_Handle, (void*)snd_ptr, prim->len ) != VSI_OK )
       return DRV_BUFFER_FULL;
     else
       return DRV_OK;
  }
  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  tst_mux_send
+------------------------------------------------------------------------------
|  Description  :  send message via specified test interface channnel
|
|  Parameters   :  id     - channel ID
|                  buffer - pointer to message
|                  size   - message length 
|
|  Return       :  DRV_OK 
|                  DRV_BUFFER_FULL
|                  DRV_BUFFER_FULL
+------------------------------------------------------------------------------
*/
int tst_mux_send ( U8 id, void * buffer, int size )
{
int chan_id;
int i;
int snd_size;
char *p_dst;
char *p_src;
 
  snd_size = size + 3;  /* 3 additional bytes for framing and chan id */
  if ( size > MAX_TST_MUX_CMD_LEN-3)
  {
    return DRV_INVALID_PARAMS;
  }

  for ( chan_id = 0; chan_id < MAX_TST_CHANNEL; chan_id++ )
  {
    if ( tst_mux_chan_struct[chan_id].channel_id == id )
    {
      p_dst = (char*)tst_mux_chan_struct[chan_id].send_data;
      p_src = (char*)buffer;
      *p_dst++ = 0x02;
      *p_dst++ = id;
      for ( i = 0; i < size; i++ )
      {
        if ( *p_src == 0x10 || *p_src == 0x02 )
        {
          if ( snd_size < MAX_TST_MUX_CMD_LEN-1 )
          {
            *p_dst++ = 0x10;
            snd_size++;
          }
        }
        *p_dst++ = *p_src++;
      }
      *p_dst = 0x02;

      if ( vsi_d_write ( 0, tst_mux_drv_handle, tst_mux_chan_struct[chan_id].send_data, snd_size) != VSI_OK )
      {
        return DRV_BUFFER_FULL;
      }
      return DRV_OK;
    }
  }
  return DRV_INVALID_PARAMS;
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  tst_mux_register
+------------------------------------------------------------------------------
|  Description  :  register callback that is called if data is received on 
|                  specified test interface channnel
|
|  Parameters   :  id       - channel ID
|                  callback - callback function 
|
|  Return       :  DRV_OK 
|                  DRV_INITFAILURE
+------------------------------------------------------------------------------
*/
int tst_mux_register ( U8 id, void (*callback)(void * buffer, int size))
{
int i;

  for ( i = 0; i < MAX_TST_CHANNEL; i++ )
  {
    if ( tst_mux_chan_struct[i].channel_id == 0 )
    {
      tst_mux_chan_struct[i].channel_id = id;
      tst_mux_chan_struct[i].rcv_callback   = callback;
      MALLOC(tst_mux_chan_struct[i].send_data,MAX_TST_MUX_CMD_LEN);
      return DRV_OK;
    }
  }
  return DRV_INITFAILURE;
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  tst_mux_callback
+------------------------------------------------------------------------------
|  Description  :  callback that is called if data is received on 
|                  specified test interface channnel
|
|  Parameters   :  id       - channel ID
|                  buffer   - data 
|                  size     - number of received bytes
|
|  Return       :  DRV_OK 
|                  DRV_INITFAILURE
+------------------------------------------------------------------------------
*/
void tst_mux_callback ( U8 id, void * buffer, int size )
{
  char * rcv_ptr;
  char * dta_ptr;
  char * p_rd;
  char * p_wr;
  int rd_bytes       = size;
  int bytes_to_read  = size;
  int total_wr_bytes = size;
  int total_rd_bytes = 0;
  int i;
  int stuffed_byte = 0;
  

  MALLOC(rcv_ptr, size);
  p_rd = rcv_ptr;
  do
  {
    vsi_d_read ( 0, tst_mux_drv_handle, (void*)rcv_ptr, (ULONG*)&rd_bytes );
    total_rd_bytes += rd_bytes;
    if ( total_rd_bytes < bytes_to_read )
    {
      rcv_ptr += rd_bytes;
      rd_bytes = bytes_to_read - total_rd_bytes;
    }
  } while ( total_rd_bytes < bytes_to_read );

  
  MALLOC(dta_ptr, size);
  p_wr = dta_ptr;
  for ( i = 0; i < size; i++ )
  {
    if ( stuffed_byte == 1 )
    {
      stuffed_byte = 0;
      *p_wr++ = *p_rd++; 
    }
    if ( *p_rd == 0x10 )
    {
      stuffed_byte = 1;
      p_rd++;
      total_wr_bytes--;
    }
    else
    {
      *p_wr++ = *p_rd++; 
    }
  }
  MFREE(rcv_ptr);

  if ( tst_mux_chan_struct[id].rcv_callback != NULL )
  {
    (tst_mux_chan_struct[id].rcv_callback)(dta_ptr,total_wr_bytes); 
  }
  MFREE(dta_ptr);

}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  tst_mux_init
+------------------------------------------------------------------------------
|  Description  :  register callback that is called if data is received on 
|                  specified test interface channnel
|
|  Parameters   :  id       - channel ID
|                  callback - callback function 
|
|  Return       :  DRV_OK 
|                  DRV_INITFAILURE
+------------------------------------------------------------------------------
*/
int tst_mux_init ( void )
{
int i;

  if ( (tst_mux_drv_handle = vsi_d_open ( TST_Handle, (char*)TR_NAME )) == VSI_ERROR )
  {
    return DRV_INITFAILURE;
  }
  for ( i = 0; i < MAX_TST_CHANNEL; i++ )
  {
    tst_mux_chan_struct[i].channel_id = 0;
    tst_mux_chan_struct[i].rcv_callback   = NULL;
    tst_mux_chan_struct[i].rcv_data_ptr   = NULL;
    tst_mux_chan_struct[i].rcv_data_size  = 0;
  }
  return DRV_OK;
}
#endif
