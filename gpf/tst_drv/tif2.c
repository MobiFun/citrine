/*
+------------------------------------------------------------------------------
|  File:       tif.c
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
|  Purpose :  This Modul contains the TIF driver
+-----------------------------------------------------------------------------
*/

#ifndef __TIF_C__
#define __TIF_C__
#endif

#include <string.h>
#include "gpfconf.h"
#include "typedefs.h"
#include "gdi.h"
#include "vsi.h"
#include "tools.h"
#include "glob_defs.h"
#if 0	// FreeCalypso
#include "ccdtable.h"
#include "ccddata.h"
#endif
#include "pcon.h"
#include "p_mem.h"
#include "drvconf.h"
#include "tstheader.h"

#ifdef _TOOLS_
  #include "frame_const.h"
  #include <stdio.h>
#endif /* _TOOLS_ */
#include "printtofile.h"

#ifndef _TOOLS_
#if !defined (_SOLARIS_) && !defined (_LINUX_)
#pragma pack(4)
#endif
#endif

/*lint -e545 */
/*==== TYPES ======================================================*/

typedef struct
{
  USHORT Handle;
  USHORT EnabledSignalType;
  T_DRV_CB_FUNC Callback;
  USHORT Config;
} T_TIF_DATA;

#ifdef _TOOLS_
typedef struct
{
  unsigned int use_id;
  char *name;
} T_RIV_USEID_TO_NAME;
#endif

/*==== CONSTANTS ==================================================*/

#define ALLOWED_TIF_SIGNALS   (DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT|DRV_SIGTYPE_DISCONNECT)

#define NAME_LENGTH           4

#define TIF_RCV_CMD_SIZE    32
#define TIF_MAX_CMD         (TIF_RCV_CMD_SIZE-1)

#define STX                 0x02
#define LF                  0x0a

#define PCON_ENABLED   0x0001
#define SYNC_MODE      0x0002

#undef VSI_CALLER
#define VSI_CALLER TST_Handle,

#ifdef _TOOLS_
const T_RIV_USEID_TO_NAME riv_use_id_to_name[] =
{
  { 0x00010001, "RVM" },
  { 0x00010002, "RVT" },
  { 0x000A0001, "R2D" },
  { 0x000A0002, "RTC" },
  { 0x000A0004, "FFS" },
  { 0x000A0008, "KPD" },
  { 0x000A0010, "SPI" },
  { 0x000A0020, "PWR" },
  { 0x000A0040, "RGUI" },
  { 0x00140001, "HCI" },
  { 0x00140002, "L2CA" },
  { 0x00140004, "BTCT" },
  { 0x00140008, "RFC" },
  { 0x00140010, "SPP" },
  { 0x00140020, "HS" },
  { 0x00140040, "HSG" },
  { 0x00140080, "SDP" },
  { 0x00140100, "DUN" },
  { 0x00140200, "FAX" },
  { 0x00140400, "OBX" },
  { 0x00140800, "OPP" },
  { 0x00141000, "FTP" },
  { 0x00142000, "SYN" },
  { 0x001E0001, "EXPL" },
  { 0x001E0010, "AUDIO" },
  { 0x001E0020, "ETM" },
  { 0x001E0040, "DAR" },
  { 0x001E0080, "MKS" },
  { 0x001E0100, "MPM" },
  { 0x001E0200, "LLS" },
  { 0x001E0400, "ATP" },
  { 0x001E0800, "ATPUART" },
  { 0x001E2000, "MDC" },
  { 0x001E4000, "TTY" },
  { 0x001E8000, "DCM" },
  { 0x00280001, "TEST" },
  { 0x00280002, "TUT" },
  { 0x00320001, "KIL" },
  { 0x00320002, "KGC" },
  { 0x00320004, "KCL" },
  { 0x00320008, "KMM" },
  { 0x00320010, "KNM" },
  { 0x00320020, "UVM" },
  { 0x00320040, "KZP" },
  { 0x00320080, "KPG" },
  { 0x00320100, "JTM" },
  { 0x003C0001, "DEV1" },
  { 0x003C0002, "DEV2" },
  { 0x003C0003, "DEV3" },
  { 0x00460001, "RNET" },
  { 0x00460002, "RNET_WS" },
  { 0x00460004, "RNET_RT" },
  { 0x00460008, "RNET_BR" },
  { 0x00640001, "MSME" },
  { 0x00640002, "MSFE" },
  { 0x00640004, "STKE" },
  { 0x00640008, "BRSE" },
  { 0x00640010, "BRAE" },
  { 0x00640020, "PHSE" },
  { 0x00640040, "IBSE" },
  { 0x00640080, "MMSE" },
  { 0x00640100, "SLSE" },
  { 0x00640200, "SMAE" },
  { 0x00640400, "MEAE" },
  { 0x00640800, "SECE" },
  { 0x00641000, "SELE" },
  { 0x00642000, "PRSE" },
  { 0x00644000, "JAAE" },
  { 0x00648000, "JASE" },
  { 0x006E0001, "IT1E" },
  { 0x006E0002, "IT2E" },
  { 0x006E0004, "IT0E" },
  { 0x0,        NULL}
};
#endif

/*==== EXTERNALS ==================================================*/

extern T_HANDLE TST_Handle;

#ifndef _TOOLS_
extern const T_PCON_PROPERTIES *pcon;
extern ULONG MaxPrimPartSize;
extern const T_MEM_PROPERTIES *mem;
#endif

#ifdef CTB
  extern BOOL ctb_sent_to_tap;
  extern BOOL ctb_tick_enabled;
#endif

/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM
char corruptWarning[] = "corrupt prim received: ";
char wrongTSTWarning[] = "wrong (old) TST header used on toolside! ";
T_TIF_DATA TIF_Data;
static ULONG TIF_WrInd = 0;
BOOL socket_flush = TRUE;
#else
extern T_TIF_DATA TIF_Data;
extern BOOL socket_flush;
#endif /* RUN_INT_RAM */

#ifdef _TOOLS_
char buffer[80];

const T_PCON_PROPERTIES pcon_export =
{
  pcon_init_prim_coding,
  pcon_decodePrim,
  pcon_codePrim,
  pcon_make_filter,
  PCON_STACK_OFFSET
};

T_PCON_PROPERTIES const *pcon = &pcon_export;
#endif

/*==== FUNCTIONS ==================================================*/

#ifndef RUN_INT_RAM
USHORT TIF_Write ( void *Buffer, ULONG *BytesToWrite );
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TIF                 |
| STATE   : code                       ROUTINE : TIF_Exit            |
+--------------------------------------------------------------------+

  PURPOSE : exit a driver

*/
LOCAL void TIF_Exit ( void )
{
#ifdef _TOOLS_
  ccddata_exit();
#endif /* _TOOLS_ */
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TIF                 |
| STATE   : code                       ROUTINE : TIF_Write           |
+--------------------------------------------------------------------+

  PURPOSE : write data to driver

*/
/* Lint is not able to analyze the program flow through if...else constructions
   and therefor bring a bunch of warnings.*/
/*lint -e645 suppress warning -- Symbol '...' may not have been initialized */
/*lint -e644 suppress warning -- Symbol '...' may not have been initialized */
/*lint -e613 suppress warning -- possibly use of NULL pointer '...' */
USHORT TIF_Write ( void *Buffer, ULONG *BytesToWrite )
{
T_PRIM_HEADER *prim = (T_PRIM_HEADER*)Buffer;
T_PRIM_HEADER *prim_ptr = NULL;
TST_SMALL_HEADER *tst_header_ptr;
T_S_HEADER *s_hdr;
unsigned int bytes_to_write;
unsigned int size;
unsigned int name_len;
unsigned int length;
unsigned int org_rcv_len = 0; 
unsigned int opc_len = 0;
unsigned int bytes_written = 0;
unsigned int write_flag = 0;
char *write_ptr;
char *tst_data_ptr;
char trace_opc=0;
char Sender[RESOURCE_NAMELEN] = {0};
void *coded_prim;
void *decoded_prim;
int pcon_called = 0;
int is_primitive = TRUE;
int ret = DRV_OK;
int sts;
#ifdef _TOOLS_
int is_generated_primitive = FALSE;
#endif /* _TOOLS_ */


  s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);
  if ( prim->opc & SYS_MASK )
  {
    size = prim->len - sizeof(T_PRIM_HEADER);
    tst_data_ptr = (char*)P2D(prim);
    tst_header_ptr = (TST_SMALL_HEADER*) tst_data_ptr - 1;
    tst_header_ptr->combined[INFO_BYTE] = (~HDR_IDENT_MASK | IDENT_SYS_PRIM);
    InsertString(s_hdr->rcv, (char*)&tst_header_ptr->receiver, 4);
    is_primitive = FALSE;
#ifdef _TOOLS_
    if ( (length = GetNextToken (tst_data_ptr, buffer, " #")) > 0)
    {
      if ( !strcmp (buffer, FILTER) )
      {
        void *filter_prim;
        if ( (pcon != NULL) && pcon->make_filter(tst_data_ptr+length+1, &filter_prim) == PCON_OK )
        {
          prim = D2P(filter_prim);
          s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);
          is_primitive = TRUE;
          is_generated_primitive = TRUE;
        }
      }
    }
#endif
  }
  else if ( (SAP_NR(prim->opc)==TRACE_SAP) || (prim->opc==TRACE_OPC) )
  {
    is_primitive = FALSE;
    size = prim->len - sizeof(T_PRIM_HEADER);
    tst_data_ptr = (char*)P2D(prim);
    if (prim->opc!=TRACE_OPC)
    {
      /* we have a traceclass id */
      trace_opc=(char)PRIM_NR(prim->opc);
    }
    tst_header_ptr = (TST_SMALL_HEADER*) tst_data_ptr - 1;
    tst_header_ptr->combined[INFO_BYTE] = (~HDR_IDENT_MASK | IDENT_TRACE);
    InsertString(FRM_PCO_NAME, (char*)&tst_header_ptr->receiver, 4);
  }
  if ( is_primitive == TRUE )
  {
    TST_BIG_HEADER tst_big_header;
    TST_BIG_HEADER *tst_big_header_ptr;
#ifdef _TOOLS_
    /* on the tool side the primitive to be duplicated is copied when sent to TST */
    prim_ptr = prim;
#else
    /* in the target a pointer to the primitive to be duplicated is transported in a carrier of type T_PRIM_X */
    prim_ptr = (T_PRIM_HEADER*)((T_PRIM_X*)(prim))->prim_ptr;
#endif

    size = prim_ptr->len - sizeof(T_PRIM_HEADER);
    tst_data_ptr = (char*)P2D(prim_ptr);
    tst_big_header_ptr = &tst_big_header;
    opc_len = 4;
    org_rcv_len = 4;
    if ( (pcon != NULL) && (TIF_Data.Config & PCON_ENABLED) && !(prim_ptr->opc & VIRTUAL_OPC) )
    {
      decoded_prim = P2D(prim_ptr);
      if ( (sts = (int)(pcon->code_prim( prim_ptr->opc, decoded_prim, &coded_prim, (ULONG*)&size, 0, s_hdr->rcv ))) != PCON_OK )
      {
        vsi_o_ttrace (NO_TASK, TC_SYSTEM,"PCON Code Error %d, TIF_Write() aborted", sts );
        return DRV_INTERNAL_ERROR;
      }
      else
      {
        prim_ptr = D2P(coded_prim);
        tst_data_ptr = coded_prim;
        pcon_called = 1;
      }
    }

    EXCHANGE_4BYTES_ENDIANESS(&prim_ptr->opc);

    tst_big_header_ptr->opc = prim_ptr->opc;
    tst_big_header_ptr->combined[INFO_BYTE] = (~HDR_IDENT_MASK | IDENT_PS_PRIM);
    InsertString(s_hdr->rcv, (char*) &tst_big_header_ptr->receiver, 4);
    InsertString(s_hdr->org_rcv, (char*) &tst_big_header_ptr->orgreceiver, 4);
    socket_flush = TRUE;
    tst_header_ptr = (TST_SMALL_HEADER*) tst_big_header_ptr;
  }

  tst_header_ptr->combined[INFO_BYTE] = ((tst_header_ptr->combined[INFO_BYTE] & (~HDR_VERSION_MASK)) | HDR_VALID_VERSION_1);
  tst_header_ptr->combined[INFO_BYTE] = ((tst_header_ptr->combined[INFO_BYTE] & (~HDR_TIME_MASK)) | HDR_TIME_MS);
  tst_header_ptr->time = s_hdr->time;
  EXCHANGE_4BYTES_ENDIANESS(&tst_header_ptr->time);
  tst_header_ptr->sender[0] = 0;

  if ( s_hdr->snd[0] == 0 )
  {
    if ( *tst_data_ptr == '~')
    {
      name_len = GetNextToken ((char*)(tst_data_ptr), Sender, "~");
      InsertString(Sender, (char*) &tst_header_ptr->sender, 4);
      size -= 2+name_len;
      /* rub out a leading ~NAM~ in data field: */
      memcpy ( tst_data_ptr, tst_data_ptr + 2 + name_len, size );
    }
    else
    {
      InsertString(FRM_SYST_NAME, (char*)&tst_header_ptr->sender, 4);
    }
  }
  else
  {
    T_HANDLE TaskHandle;
    if ( s_hdr->snd[0] & ~HANDLE_MASK )
    {
      TaskHandle = (T_HANDLE)(s_hdr->snd[0]&HANDLE_MASK);
      vsi_e_name ( TST_Handle, TaskHandle, Sender );
      InsertString(Sender, (char*)&tst_header_ptr->sender, 4);
    }
    else
    {
      InsertString(s_hdr->snd, (char*)&tst_header_ptr->sender, 4);
    }
  }

  length = size + TST_HEADER_TRAILING_FIELDS + org_rcv_len + opc_len + (trace_opc ? 1 : 0);
  tst_header_ptr->combined[FIRST_BYTE] = (UBYTE) (LOW_MASK & length);
  tst_header_ptr->combined[SECOND_BYTE] = (UBYTE) (LOW_MASK & (length >> 8));
  bytes_to_write = length + TST_HEADER_LEADING_FIELDS;
  write_ptr = (char*)&tst_header_ptr->combined[INFO_BYTE];
  if (trace_opc)
  {
    /* move small header 1 byte to the left and insert trace class id */
    char *trace_opc_ptr=tst_data_ptr-1;
    write_ptr = (char*)tst_header_ptr;
    memcpy(write_ptr,(char*)tst_header_ptr+1,sizeof(TST_SMALL_HEADER)-1);
    *trace_opc_ptr=trace_opc;
    length++;
  }
  else if ( prim_ptr != NULL )
  {
    bytes_written = TST_BIG_HEADER_SIZE;
    if ( vsi_d_write ( TIF_Data.Handle, 0, write_ptr, bytes_written | PRIM_HEADER_FLAG ) != VSI_OK )
    {
      ret = DRV_INTERNAL_ERROR;
      *BytesToWrite = 0;
    }
    write_ptr = tst_data_ptr;
    write_flag = PRIM_DATA_FLAG;
  }

  if (ret==DRV_OK)
  {
#if defined (_TOOLS_) || defined (_LINUX_) || defined (_SOLARIS_)
    socket_flush = TRUE;  /* flush socket always on tool side */
#endif

    if ( vsi_d_write ( TIF_Data.Handle, 0, write_ptr, (bytes_to_write - bytes_written) | write_flag ) != VSI_OK )
    {
      ret = DRV_INTERNAL_ERROR;
      *BytesToWrite = 0;
    }
  }
  if ( pcon_called )
  {
    PFREE(P2D((T_VOID_STRUCT*)prim_ptr));
  }
#ifdef _TOOLS_
  if ( is_generated_primitive == TRUE )
  {
    vsi_c_free ( TST_Handle, (T_VOID_STRUCT**)&prim );
  }
#else /* _TOOLS_ */
  if (prim->opc & MEMHANDLE_OPC)
  {
    M_FREE(P_MEMHANDLE_SDU(((T_PRIM_X*)(prim))->prim_ptr));
  }
#endif /* _TOOLS_ */
#if defined _NUCLEUS_ && !defined _TARGET_
#ifdef CTB
  if(ctb_tick_enabled && !ctb_sent_to_tap && !strncmp(tst_header_ptr->receiver,"TAP",3)) 
  {
    ctb_sent_to_tap = TRUE;
  }
#endif
#endif

  return ( (USHORT)ret );
}
/*lint +e645 */
/*lint +e613 */
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TIF                 |
| STATE   : code                       ROUTINE : TIF_SetSignal       |
+--------------------------------------------------------------------+

  PURPOSE : enable signal for the driver

*/
LOCAL USHORT TIF_SetSignal ( USHORT SignalType )
{
  if ( !(SignalType & ALLOWED_TIF_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    TIF_Data.EnabledSignalType |= SignalType;

  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TIF                 |
| STATE   : code                       ROUTINE : TIF_ResetSignal     |
+--------------------------------------------------------------------+

  PURPOSE : disable signal for the driver

*/
LOCAL USHORT TIF_ResetSignal ( USHORT SignalType )
{
  if ( !(SignalType & ALLOWED_TIF_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    TIF_Data.EnabledSignalType &= ~SignalType;

  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TIF                 |
| STATE   : code                       ROUTINE : TIF_SetConfig       |
+--------------------------------------------------------------------+

  PURPOSE : set configuration for the driver

*/
/*lint -esym(715,Buffer), suppress Info -- Symbol 'Buffer' not referenced) */
LOCAL USHORT TIF_SetConfig ( char *Buffer )
{
  if ( !strcmp ( TIF_PCON_ENABLE, Buffer ) )
  {
    if ( TIF_Data.Config & PCON_ENABLED)
    {
      /* already in PCON mode */
      return DRV_OK;
    }

#ifdef _TOOLS_
    switch (ccddata_init(NULL,0,NULL,NULL))
    {
      case CCDDATA_DLL_OK:
      case CCDDATA_DLL_ALREADY:
        break;
      default:
        return DRV_INTERNAL_ERROR;
    }
#endif
    TIF_Data.Config |= PCON_ENABLED;

#ifdef _TOOLS_
    printf("TIF: PCON mode selected\n");
#endif
    return DRV_OK;
  }
  else if ( !strcmp ( DRV_DEFAULT, Buffer ) )
  {
    if ( !(TIF_Data.Config & PCON_ENABLED))
    {
      /* already in default mode */
      return DRV_OK;
    }

#ifdef _TOOLS_
    ccddata_exit();
#endif
    TIF_Data.Config &= ~PCON_ENABLED;

#ifdef _TOOLS_
    printf("TIF: default mode selected\n");
#endif
    return DRV_OK;
  }
#ifdef _TOOLS_
  else if ( !strcmp ( ENABLE_SYNC_MODE, Buffer ) )
  {
    TIF_Data.Config |= SYNC_MODE;
    PrintToFile("TIF: sync mode enabled\n");
    return DRV_OK;
  }
  else if ( !strcmp ( DISABLE_SYNC_MODE, Buffer ) )
  {
    TIF_Data.Config &= ~SYNC_MODE;
    PrintToFile("TIF: sync mode disabled\n");
    return DRV_OK;
  }
  return DRV_INVALID_PARAMS;
#else
  return DRV_OK;
#endif
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TIF                 |
| STATE   : code                       ROUTINE : Callback            |
+--------------------------------------------------------------------+

  PURPOSE : callback function of the driver

*/
/* Lint is not able to analyze the program flow through if...else constructions
   and therefor bring a bunch of warnings.*/
/*lint -esym(645,opc_str) suppress warning -- Symbol 'opc_str' may not have been initialized */
LOCAL void TIF_Callback ( T_DRV_SIGNAL *Signal )
{
T_PRIM_HEADER *prim;
T_S_HEADER *s_hdr;
T_PRIM_X *sys_prim;
T_DRV_SIGNAL TIF_Signal;
TST_BIG_HEADER tst_header;
unsigned int CorruptByteOffset = 0;
unsigned int length;
unsigned int BytesToRead = 0;
unsigned int BytesRead;
unsigned int Bytes = 0;
unsigned int i;
unsigned int opc_len = 0;
unsigned int org_rcv_len = 0;
unsigned int alloc_size;
int sts;
int WrongTSTHeader = FALSE;
char *target;
void *decoded_prim;
char opc_str[4];
#ifdef _TOOLS_
int l1_sigtype = 0;
#endif

  switch ( Signal->SignalType )
  {
#ifdef _TOOLS_
    case DRV_SIGTYPE_READ_L1:
      l1_sigtype = 1;
      /*lint -fallthrough*/
    case DRV_SIGTYPE_READ_RIV:

      if ( vsi_d_read ( TIF_Data.Handle, 0, NULL, &BytesToRead ) != DRV_OK )
        return;

      alloc_size = S_ALLOC_SIZE(BytesToRead + 1);
      if ( ( prim = (T_PRIM_HEADER*)vsi_c_new ( TST_Handle, alloc_size, 0 FILE_LINE_MACRO )) != NULL )
      {
        TIF_WrInd = 0;
        Bytes = 0;
        BytesRead = BytesToRead;
        do
        {
          vsi_d_read ( TIF_Data.Handle, 0, (void*)((char*)P2D(prim)+TIF_WrInd), &BytesRead );
          Bytes += BytesRead;
          if ( Bytes < BytesToRead )
          {
            TIF_WrInd += BytesRead;
            BytesRead = BytesToRead - BytesRead;
          }
        } while ( Bytes < BytesToRead );
        prim->sh_offset = S_HDR_OFFSET(alloc_size - sizeof(T_S_HEADER));
        s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);
        s_hdr->time = 0;
        s_hdr->org_rcv[0] = 0;
        if ( l1_sigtype == 1 )
        {
          strcpy ( s_hdr->snd, "L1" );
        }
        else
        {
          int i = 0;
          unsigned int use_id = 0;
          unsigned char *p = (unsigned char*)P2D(prim);
          use_id = use_id | *p++ << 24;
          use_id = use_id | *p++ << 16;
          use_id = use_id | *p++ << 8;
          use_id = use_id | *p;
          do
          {
            if ( riv_use_id_to_name[i].use_id == use_id )
            {
              strcpy ( s_hdr->snd, riv_use_id_to_name[i].name );
              break;
            }
            if ( riv_use_id_to_name[i+1].use_id == 0 )
              strcpy ( s_hdr->snd, "RIV" );
          } while ( riv_use_id_to_name[++i].use_id != 0 );
          memcpy ( (char*)P2D(prim), (char*)P2D(prim)+5, BytesToRead);
        }
        strcpy ( s_hdr->rcv, FRM_PCO_NAME );

        sys_prim = (T_PRIM_X*) vsi_c_new ( TST_Handle, alloc_size, 0 FILE_LINE_MACRO );
        sys_prim->prim_ptr = prim;
        prim->opc = 0;
        prim->len = BytesToRead  + sizeof(T_PRIM_HEADER);

        if ( TIF_Data.EnabledSignalType & DRV_SIGTYPE_READ )
        {
          VSI_PPM_SEND ( (T_PRIM_HEADER*)sys_prim, TST_Handle );
          VSI_PPM_SEND ( (T_PRIM_HEADER*)sys_prim->prim_ptr, TST_Handle );
          TIF_Signal.SignalType = DRV_SIGTYPE_READ;
          TIF_Signal.DrvHandle = TIF_Data.Handle;
          TIF_Signal.UserData = (T_VOID_STRUCT*)sys_prim;
          (TIF_Data.Callback)( &TIF_Signal );
        }
      }
      break;
#endif
    case DRV_SIGTYPE_READ:
      /*
       * Get the size of the needed buffer to store the data
       */

      if ( vsi_d_read ( TIF_Data.Handle, 0, NULL, (ULONG*)&BytesToRead ) != DRV_OK )
        return;

      if ( BytesToRead )
      {
        if ( BytesToRead >= TST_SMALL_HEADER_SIZE )
          BytesToRead -= TST_SMALL_HEADER_SIZE;
        Bytes = 0;
        TIF_WrInd = 0;
        BytesRead = TST_SMALL_HEADER_SIZE;
        do
        {
          vsi_d_read ( TIF_Data.Handle, 0, (void*)((char*)&tst_header.combined[INFO_BYTE]+TIF_WrInd), (ULONG*)&BytesRead );
          Bytes += BytesRead;
          if ( Bytes < TST_SMALL_HEADER_SIZE )
          {
            TIF_WrInd += BytesRead;
            BytesRead = TST_SMALL_HEADER_SIZE - BytesRead;
          }
        } while ( Bytes < TST_SMALL_HEADER_SIZE );

        alloc_size = ALIGN(sizeof(T_PRIM_X));
        sys_prim = (T_PRIM_X*) vsi_c_new ( TST_Handle, alloc_size, 0 FILE_LINE_MACRO );

        switch (tst_header.combined[INFO_BYTE] &  HDR_IDENT_MASK)
        {
          case IDENT_ABORT:
            tst_header.combined[INFO_BYTE] = ((tst_header.combined[INFO_BYTE] & ~HDR_IDENT_MASK) | IDENT_TRACE);
#ifdef _TOOLS_
            /* TR did loose byte(s)! Primitive is corrupt, no header val can be
            guaranteed to be valid.
            Create a HUGE trace prim with zero time, sender tst, receiver pco,
            org_receiver tst
            TRACE DATA will contain: a "corrupt prim received: " string,
            the already received part of the prim and all the rest */
            CorruptByteOffset = TST_SMALL_HEADER_SIZE + strlen(corruptWarning) + 1;
#else
            /* TR did receive a prim from tool side old TST header format and swiched to new.
            Now we have to TRACE the wrong tool side format, because the prim is lost. */
            WrongTSTHeader = TRUE;
            BytesToRead = 0;
            CorruptByteOffset = strlen(wrongTSTWarning) + 1;
#endif
            break;
          case IDENT_PS_PRIM:
            opc_len = 4;
            org_rcv_len = 4;
            break;
          default:
            break;
        }

        if ( (BytesRead = org_rcv_len) > 0 )
        {
          if ( BytesToRead >= org_rcv_len )
            BytesToRead -= org_rcv_len;
          Bytes = 0;
          TIF_WrInd = 0;
          do
          {
            vsi_d_read ( TIF_Data.Handle, 0, (void*)((char*)&tst_header.orgreceiver+TIF_WrInd), (ULONG*)&BytesRead );
            Bytes += BytesRead;
            if ( Bytes < org_rcv_len )
            {
              TIF_WrInd += BytesRead;
              BytesRead = org_rcv_len - BytesRead;
            }
          } while ( Bytes < org_rcv_len );
        }

        if ( (BytesRead = opc_len) > 0 )
        {
          if ( BytesToRead >= opc_len )
            BytesToRead -= opc_len;
          Bytes = 0;
          TIF_WrInd = 0;
          do
          {
            vsi_d_read ( TIF_Data.Handle, 0, (void*)(opc_str+TIF_WrInd), (ULONG*)&BytesRead );
            Bytes += BytesRead;
            if ( Bytes < opc_len )
            {
              TIF_WrInd += BytesRead;
              BytesRead = opc_len - BytesRead;
            }
          } while ( Bytes < opc_len );
        }

        alloc_size = S_ALLOC_SIZE(CorruptByteOffset + BytesToRead + 1);
#ifdef _TOOLS_
        if ( alloc_size > MAX_PRIM_PARTITION_SIZE - sizeof(T_PRIM_HEADER) - sizeof(T_S_HEADER))
          alloc_size = MAX_PRIM_PARTITION_SIZE - sizeof(T_PRIM_HEADER) - sizeof(T_S_HEADER);
#else
        if ( alloc_size > MaxPrimPartSize - sizeof(T_PRIM_HEADER) - sizeof(T_S_HEADER) )
          alloc_size = MaxPrimPartSize - sizeof(T_PRIM_HEADER) - sizeof(T_S_HEADER);
#endif
        if ( ( prim = (T_PRIM_HEADER*)vsi_c_new ( TST_Handle, alloc_size, 0 FILE_LINE_MACRO )) != NULL )
        {
          TIF_WrInd = 0;
          Bytes = 0;
          if ((BytesToRead + CorruptByteOffset) > alloc_size)
          {
#ifdef _TOOLS_
            PrintToFile("TIF: Reduced a new Prim's size\n");
#endif
            BytesToRead = alloc_size - CorruptByteOffset;
          }
          BytesRead = BytesToRead;

          target = (char*) P2D(prim);

          if (WrongTSTHeader)
          {
            memcpy(target, wrongTSTWarning, strlen(wrongTSTWarning));
            target += strlen(wrongTSTWarning);
          }
          else if (CorruptByteOffset)
          {
#ifdef _TOOLS_
            PrintToFile("TIF: Byte(s) lost\n");
#endif
            memcpy(target, corruptWarning, strlen(corruptWarning));
            target += strlen(corruptWarning);
            /*lint -e420, suppress Warning -- Apparent access beyond array for function 'memcpy(...*/
            memcpy(target,&tst_header.combined[INFO_BYTE], TST_SMALL_HEADER_SIZE-1);
            /*lint +e420 */
            target += TST_SMALL_HEADER_SIZE - 1;
          }


          if (WrongTSTHeader == FALSE && BytesToRead)
          {
            if ( (tst_header.combined[INFO_BYTE] &  HDR_IDENT_MASK)==IDENT_TRACE )
            {
              /* we read the first bytes in advance in case the trace class is carried */
              ULONG bread=1;
              vsi_d_read ( TIF_Data.Handle, 0, (void*)((char*)target+TIF_WrInd), &bread );

              if (bread==1 && *((char*)target)<0x20) /* no character -> assume traceclass id */
              {
                opc_str[0]=*((char*)target);
                opc_len=1;

                BytesToRead -= bread;
              }
              else
              {
                /* ok, we have no traceclass info :-( */
                TIF_WrInd += bread;
                Bytes += bread;
              }
              BytesRead = BytesToRead - Bytes;
            }

            do
            {
              vsi_d_read ( TIF_Data.Handle, 0, (void*)((char*)target+TIF_WrInd), (ULONG*)&BytesRead );
              Bytes += BytesRead;
              if ( Bytes < BytesToRead )
              {
                TIF_WrInd += BytesRead;
                BytesRead = BytesToRead - Bytes;
              }
            } while ( Bytes < BytesToRead );
          }

          *((char*)target + Bytes) = 0;
          prim->sh_offset = S_HDR_OFFSET(alloc_size - sizeof(T_S_HEADER));
          s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);
          switch (tst_header.combined[INFO_BYTE] &  HDR_IDENT_MASK)
          {
            case IDENT_PS_PRIM:

              EXCHANGE_4BYTES_ENDIANESS(opc_str);

              memcpy((char*)&prim->opc, opc_str, 4);
              if ( (pcon != NULL) && (TIF_Data.Config & PCON_ENABLED) )
              {
                if ( (sts = (int)pcon->decode_prim ( prim->opc, &decoded_prim, (void*)((char*)prim+sizeof(T_PRIM_HEADER)), (ULONG*)&length, 0 )) != PCON_OK )
                {
                  if ( sts != PCON_CONFIG_PRIM )
                  {
                    vsi_o_ttrace (NO_TASK, TC_SYSTEM,"PCON Deode Error %d, TIF_Callback() aborted", sts );
                  }
                  PFREE(P2D(sys_prim));
                  PFREE(P2D(prim));
                  return;
                }
                PFREE(P2D((T_VOID_STRUCT*)prim));
                sys_prim->prim_ptr = D2P(decoded_prim);
              }
              else
              {
                sys_prim->prim_ptr = prim;
                length = BytesToRead;
              }
              sys_prim->prim_ptr->sh_offset = S_HDR_OFFSET(length+sizeof(T_PRIM_HEADER));
              sys_prim->prim_ptr->len = length + sizeof(T_PRIM_HEADER);
              s_hdr = (T_S_HEADER*)((ULONG*)sys_prim->prim_ptr + sys_prim->prim_ptr->sh_offset);
#ifndef _TOOLS_
              /* evtl. convert aux sdu to mem handle */
              if (sys_prim->prim_ptr->opc & MEMHANDLE_OPC && mem!=NULL)
              {
                if (P_MEMHANDLE_SDU(sys_prim->prim_ptr))
                {
                  U32 mem_handle;
                  U8 buffer_handle;
                  U8 user_handle;
                  U8 *user_data;
                  T_sdu *sdu;
                  U16 len;

                  sdu=(T_sdu *)P_MEMHANDLE_SDU(sys_prim->prim_ptr);
                  len=(sdu->l_buf + 7)/8;

                  buffer_handle=mem->create_buffer(MEM_UNORDERED_BUFFER, len);
                  user_handle = mem->create_user(buffer_handle, 23, "TST");

                  user_data=mem->alloc(user_handle,len,&mem_handle);
                  memcpy(user_data,sdu->buf+(sdu->o_buf+7)/8,len);
                  P_MEMHANDLE(sys_prim->prim_ptr)=mem_handle;

                  mem->delete_user(user_handle);
                  mem->delete_buffer(buffer_handle);
                }
              }
#endif
            break;

            case IDENT_SYS_PRIM:
              sys_prim->prim_ptr = prim;
              prim->opc = SYS_MASK;
              prim->len = BytesToRead  + sizeof(T_PRIM_HEADER);
            break;

            case IDENT_TRACE:
              sys_prim->prim_ptr = prim;
              if (opc_len)
              {
                /* nice, we found a traceclass info */
                prim->opc = opc_str[0];
                prim->opc = prim->opc << 16;
                prim->opc |= EXTENDED_OPC | VIRTUAL_OPC | TRACE_SAP;
              }
              else
              {
                prim->opc = TRACE_OPC;
              }
              prim->len = CorruptByteOffset + BytesToRead  + sizeof(T_PRIM_HEADER);
              break;

            default:
            break;
          }

          /*
           * Extract Sender and Receiverfrom header
           */
          if (CorruptByteOffset)
          {
            s_hdr->time = 0;	/* FreeCalypso: changed from NULL */
#ifndef _TOOLS_
            memcpy ( s_hdr->snd, "~TST", 4 );
            memcpy ( s_hdr->rcv, FRM_TST_NAME, 4 );
#else
            memcpy ( s_hdr->snd, FRM_TST_NAME, 4 );
            memcpy ( s_hdr->rcv, FRM_PCO_NAME, 4 );
#endif
            memcpy ( s_hdr->org_rcv, "TST", 4 );
          }
          else
          {
            s_hdr->time = tst_header.time;

            i = 0;
            do
            {
              s_hdr->snd[i] = tst_header.sender[i];
              i++;
            } while ( tst_header.sender[i] != 0x20 && i < 4 );
            s_hdr->snd[i] = 0;

            i = 0;
            do
            {
              s_hdr->rcv[i] = tst_header.receiver[i];
              i++;
            } while ( tst_header.receiver[i] != 0x20 && i < 4 );
            s_hdr->rcv[i] = 0;

            if (org_rcv_len)
            {
              i = 0;
              do
              {
                s_hdr->org_rcv[i] = tst_header.orgreceiver[i];
                i++;
              } while ( tst_header.orgreceiver[i] != 0x20 && i < 4 );
              s_hdr->org_rcv[i] = 0;
            }
          }

          if ( TIF_Data.EnabledSignalType & DRV_SIGTYPE_READ )
          {
            VSI_PPM_SEND ( (T_PRIM_HEADER*)sys_prim, TST_Handle );
            VSI_PPM_SEND ( (T_PRIM_HEADER*)sys_prim->prim_ptr, TST_Handle );
            TIF_Signal.SignalType = DRV_SIGTYPE_READ;
            TIF_Signal.DrvHandle = TIF_Data.Handle;
            TIF_Signal.UserData = (T_VOID_STRUCT*)sys_prim;
            (TIF_Data.Callback)( &TIF_Signal );
          }
        }
      } /* if ( BytesToRead ) */
    break;
    case DRV_SIGTYPE_CONNECT:
    case DRV_SIGTYPE_DISCONNECT:
      if ( TIF_Data.EnabledSignalType & Signal->SignalType )
      {
        TIF_Signal.SignalType = Signal->SignalType;
        TIF_Signal.DrvHandle = TIF_Data.Handle;
        TIF_Signal.UserData = NULL;
        (TIF_Data.Callback)( &TIF_Signal );
      }
    break;
    default:
    break;
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TIF                 |
| STATE   : code                       ROUTINE : TIF_Init            |
+--------------------------------------------------------------------+

  PURPOSE : initialize driver

*/
GLOBAL USHORT TIF_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo )
{
static const T_DRV_EXPORT TIF_Info =
{
  TIF_NAME,
  0,
  {
#ifdef _TOOLS_
    TIF_Init,
#endif
    TIF_Exit,
    NULL,
    TIF_Write,
    NULL,
    NULL,
    NULL,
    TIF_SetSignal,
    TIF_ResetSignal,
    TIF_SetConfig,
    NULL,
    TIF_Callback,
  }
};

#ifndef _TOOLS_
union
{
  USHORT s;
  UBYTE b[2];
} test;
#endif /* !_TOOLS_ */

  TIF_Data.Handle = DrvHandle;
  TIF_Data.EnabledSignalType = 0;
  TIF_Data.Config = 0;
  TIF_Data.Callback = CallbackFunc;

#ifdef _TOOLS_
  switch (ccddata_init(NULL,0,NULL,NULL))
  {
    case CCDDATA_DLL_OK:
    case CCDDATA_DLL_ALREADY:
      break;
    default:
      return DRV_INITFAILURE;
  }
#else /* _TOOLS_ */
  test.s = 1;
  if ( pcon != NULL )
  {
    ULONG sts = pcon->init_prim_coding ( TST_Handle,(UBYTE)((test.b[0]==1) ? PCON_LITTLE : PCON_BIG));
#ifndef _TARGET_ 
    vsi_pcheck_register (pcon->pcheck, PCON_OK);
#endif /* !_TARGET_ */
  }
#endif /* !_TOOLS_ */

  *DrvInfo = &TIF_Info;

  return DRV_OK;
}
#endif


