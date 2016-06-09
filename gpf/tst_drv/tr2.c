/* 
+------------------------------------------------------------------------------
|  File:       tr.c
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
|  Purpose :  This Modul contains the TR driver
+----------------------------------------------------------------------------- 
*/ 

#ifndef __TR_C__
#define __TR_C__
#endif

#include <string.h>
#include "gpfconf.h"
#include "typedefs.h"
#include "gdi.h"
#include "vsi.h"
#include "os.h"
#ifdef _TOOLS_
  #include <stdio.h>
  #include "glob_defs.h"
  #include "tstdriver.h"
  #include "frame.h"
  #include "frame_const.h"
  #include "printtofile.h"
#endif /* _TOOLS_ */
#include "tstheader.h"
#include "tst_mux.h"


/*==== TYPES ======================================================*/

typedef struct
{
  T_VOID_STRUCT *RcvBuffer;
  USHORT Handle;
  USHORT EnabledSignalType;
  T_DRV_CB_FUNC Callback;
  USHORT config;
  UBYTE rcv_state;
  UBYTE idle_state;
} T_TR_DATA;


/*==== CONSTANTS ==================================================*/

#define ALLOWED_TR_SIGNALS   (DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT|DRV_SIGTYPE_DISCONNECT) 

#define STX                 0x02
#define TI_RIV_ID           0x11
#define TI_L1_ID            0x12
#define TI_L23_ID           0x13
#define TI_STX              0x02
#define TI_ESC              0x10
#define CHAR_ABORT          'A'

#define DATA_SEGMENT_SIZE   223

#define WAIT_FOR_STX        0
#define WAIT_FOR_IDENT      1
#define WAIT_FOR_TIMESTAMP  2
#define WAIT_FOR_LENGTH     3
#define WAIT_FOR_SND        4
#define WAIT_FOR_RCV        5
#define WAIT_FOR_DATA       6
#define WAIT_FOR_CR         7
#define WAIT_FOR_TI_ID      8
#define WAIT_FOR_TI_LEN_0   9
#define WAIT_FOR_TI_LEN_1  10
#define WAIT_FOR_TI_LEN_2  11
#define WAIT_FOR_LENGTH1   12
#define WAIT_FOR_LENGTH2   13
#define WAIT_FOR_RAW_TI_S  14
#define WAIT_FOR_RAW_TI_ID 15
#define WAIT_FOR_RAW_TI_E  16

#define WAIT_FOR_MUX_START_STX  1
#define WAIT_FOR_MUX_CHAN_ID    2
#define WAIT_FOR_MUX_END_STX    3

#define STX_LF_MODE       0x0001
#define TI_TRC_MODE       0x0002
#define TI_RAW_TRC_MODE   0x0004

/*==== EXTERNALS ==================================================*/

extern ULONG TR_RcvBufferSize;
extern ULONG TR_MaxInd;
extern USHORT ext_data_pool_handle;
extern T_TST_MUX_CHANNEL tst_mux_chan_struct[];

/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM
#ifndef _TARGET_
long int accessNum = 0;
#endif
T_TR_DATA TR_Data;
static char *TR_RcvBuffer;
static ULONG TR_EndInd;
static ULONG TR_WrInd;
static ULONG TR_RdInd;
static ULONG MessageLength;
static union {
    unsigned short int val;
    char part[2];
}tst_trailer_size;
#else
extern T_TR_DATA TR_Data;
#endif

#ifdef _TOOLS_
static int ti_id_not_found;
static int ti_esc_skipped;
extern int tst_message_received;
#else
extern ULONG MaxPrimPartSize;
#endif

/*==== FUNCTIONS ==================================================*/

#ifndef RUN_INT_RAM
USHORT TR_Write ( void *Buffer, ULONG *BytesToWrite ); 
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TR                  |
| STATE   : code                       ROUTINE : TR_Exit             |
+--------------------------------------------------------------------+

  PURPOSE : exit a driver

*/
LOCAL void TR_Exit ( void )
{
  os_DeallocateMemory ( 0, TR_Data.RcvBuffer );
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TR                  |
| STATE   : code                       ROUTINE : TR_Read             |
+--------------------------------------------------------------------+

  PURPOSE : read data from driver

*/
LOCAL USHORT TR_Read ( void *Buffer, ULONG *BytesToRead )
{
ULONG Space = TR_EndInd - (TR_RdInd & TR_MaxInd);
static ULONG bytes_read;

  if ( *BytesToRead == 0 )
  {
    *BytesToRead = MessageLength;
    bytes_read = 0;
  }
  else
  {
    if ( Space > *BytesToRead ) 
    {
      memcpy ( Buffer, &TR_RcvBuffer[TR_RdInd&TR_MaxInd], *BytesToRead );
      bytes_read += *BytesToRead;
    }
    else
    {
      memcpy ( Buffer, &TR_RcvBuffer[TR_RdInd&TR_MaxInd], (unsigned int)Space );
      *BytesToRead = Space;
      bytes_read += Space;
    }
    TR_RdInd += *BytesToRead;
    if ( TR_Data.config & STX_LF_MODE )
      if ( bytes_read == MessageLength )
        TR_RdInd++;  /* skip th LF, its not read by the TIF driver */
  }
  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TR                  |
| STATE   : code                       ROUTINE : TR_Write            |
+--------------------------------------------------------------------+

  PURPOSE : write data to driver

*/
USHORT TR_Write ( void *Buffer, ULONG *BytesToWrite )
{
char *snd_ptr = (char*)Buffer;
ULONG btw;
char ti_mode_header[2];

ti_mode_header[0] = TI_STX;
ti_mode_header[1] = TI_L23_ID;

#ifdef _TOOLS_
btw=(*BytesToWrite) & ~PRIM_FLAG_MASK;
#else
btw=(*BytesToWrite);
#endif

#ifdef _TOOLS_
  if (TR_Data.config & TI_RAW_TRC_MODE)
  {
    ULONG full_btw, segment_size;
    /*lint -e813, suppress Info 813: auto variable 'osver' has size '148' */ 
    char segment[DATA_SEGMENT_SIZE*2+3];
    /*lint +e813 */
    ULONG q;

    full_btw=btw;
    while (full_btw)
    {
      btw= (full_btw>DATA_SEGMENT_SIZE) ? DATA_SEGMENT_SIZE : full_btw;

      /* fill in leading bytes */
      segment_size=0;
      segment[segment_size++]=TI_STX;
      segment[segment_size++]=TI_L23_ID;

      /* fill in payload bytes */
      for (q=0; q<btw; q++)
      {
        /* evtl insert TI_ESC characters */
        if (snd_ptr[q]==TI_STX || snd_ptr[q]==TI_ESC)
        {
          segment[segment_size++]=TI_ESC;
        }
        segment[segment_size++]=snd_ptr[q];
      }

      /* fill in trailing bytes */
      segment[segment_size++]=TI_STX;

      /* write segment */
      if ( vsi_d_write ( TR_Data.Handle, 0, (void*)segment, segment_size) != VSI_OK )
      {
        return DRV_INTERNAL_ERROR;
      }

      full_btw=full_btw-btw;
      if (full_btw)
      {
        snd_ptr=snd_ptr+btw;

        /* let the ti driver on target handle the byte-block */
        vsi_t_sleep ( 0, 100 );
      }
    }

    return DRV_OK;
  }
#endif /* _TOOLS_ */

  /*
   * To avoid reallocation and memcpy() the STX is written in front of the passed 
   * data. This works as long as the sizeof(T_SYST_HEADER) is bigger than the header
   * sent via the test interface.
   */
  if ( TR_Data.config & STX_LF_MODE )
  {
    btw=(*BytesToWrite) & ~PRIM_FLAG_MASK;          /* not really generic, but we need to mask here */
                                                    /* we assume that the TITRC driver is not below */
    if ( *BytesToWrite & PRIM_DATA_FLAG )           /* primitive data -> LF at the end */
    {
      *(snd_ptr+btw) = '\n';
      btw = btw + 1;
    }
    else if ( *BytesToWrite & PRIM_HEADER_FLAG )    /* primitive header -> add STX in front */
    {
      snd_ptr--;                                    /* it is posible to add STX because the first */
      *snd_ptr = STX;                               /* byte of TST_SMALL_HEADER/TST_BIG_HEADER in unused */
      btw = btw + 1;
    }
    else                                            /* trace -> STX in front, LF at the end */
    {
      *(snd_ptr+btw) = '\n';
      snd_ptr--;
      *snd_ptr = STX;
      btw = btw + 2;
    }
  }

  /* Add mux STX and channel id if not already in */
  if (TR_Data.config & TI_RAW_TRC_MODE && *snd_ptr != TI_STX )
  {
    if ( vsi_d_write ( TR_Data.Handle, 0, (void*)ti_mode_header, 2) != VSI_OK )
    {
      return DRV_INTERNAL_ERROR;
    }
  }

  if ( vsi_d_write ( TR_Data.Handle, 0, (void*)snd_ptr, btw) != VSI_OK )
  {
    return DRV_INTERNAL_ERROR;
  }
 
  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TR                  |
| STATE   : code                       ROUTINE : TR_SetSignal        |
+--------------------------------------------------------------------+

  PURPOSE : enable signal for the driver

*/
LOCAL USHORT TR_SetSignal ( USHORT SignalType )
{
  if ( !(SignalType & ALLOWED_TR_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    TR_Data.EnabledSignalType |= SignalType;
  
  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TR                  |
| STATE   : code                       ROUTINE : TR_ResetSignal      |
+--------------------------------------------------------------------+

  PURPOSE : disable signal for the driver

*/
LOCAL USHORT TR_ResetSignal ( USHORT SignalType )
{
  if ( !(SignalType & ALLOWED_TR_SIGNALS) )
    return DRV_INVALID_PARAMS;
  else
    TR_Data.EnabledSignalType &= ~SignalType;
  
  return DRV_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TR                  |
| STATE   : code                       ROUTINE : TR_SetConfig        |
+--------------------------------------------------------------------+

  PURPOSE : set configuration for the driver

*/
LOCAL USHORT TR_SetConfig ( char *Buffer )
{
  if ( !strcmp ( TR_STX_LF, Buffer ) )
  {
    TR_Data.config = STX_LF_MODE;
    TR_Data.rcv_state = WAIT_FOR_STX;
    TR_Data.idle_state = WAIT_FOR_STX;
#ifdef _TOOLS_
    printf ("TR: STX mode enabled\n");
#endif /* _TOOLS_ */
    return DRV_OK;
  }
#ifdef _TOOLS_
  else if ( !strcmp ( DRV_TI_MODE, Buffer ) )
  {
    TR_Data.config = TI_TRC_MODE;
    TR_Data.rcv_state = WAIT_FOR_TI_ID;
    TR_Data.idle_state = WAIT_FOR_TI_ID;
    printf ("TR: TI mode enabled\n");
    return DRV_OK;
  }
#endif /* _TOOLS_ */
  else if ( !strcmp ( DRV_RAW_TI_MODE, Buffer ) )
  {
    TR_Data.config = TI_RAW_TRC_MODE;
    TR_Data.rcv_state = WAIT_FOR_RAW_TI_S;
    TR_Data.idle_state = WAIT_FOR_RAW_TI_S;
#ifdef _TOOLS_
    printf ("TR: raw TI mode enabled\n");
#endif
    return DRV_OK;
  }
  else if ( !strcmp ( DRV_DEFAULT, Buffer ) )
  {
    TR_Data.config = 0;
    TR_Data.rcv_state = WAIT_FOR_IDENT;
    TR_Data.idle_state = WAIT_FOR_IDENT;
#ifdef _TOOLS_
    accessNum++;
    PrintToFile("TR: default mode selected __________________________ (%d)\n", accessNum);
#endif /* _TOOLS_ */
    return DRV_OK;
  }
  else
    return DRV_INVALID_PARAMS;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : TR                  |
| STATE   : code                       ROUTINE : Callback            |
+--------------------------------------------------------------------+

  PURPOSE : callback function of the driver

*/
/*lint -esym(644,infoByteAddr) suppress warning -- Symbol 'infoByteAddr' may not have been initialized */
LOCAL void TR_Callback ( T_DRV_SIGNAL *Signal )
{
static T_DRV_SIGNAL TR_Signal;
static USHORT  counter   = 0;
static char* infoByteAddr;
int tr_abort_discard;
int tr_abort;
unsigned char           c;
ULONG          i;
ULONG BytesToRead;
ULONG BytesRead = 0;
ULONG TR_WorkInd;
ULONG Bytes = 0;
USHORT continue_read;
//#ifdef _TOOLS_
static unsigned int ti_len = 0;
static char ti_id = 0;
static int sigtype = 0;
static int lf_found;
static int crlf_found;
static U8 mux_chan_id;
static U8 mux_chan = 0;
static U8 mux_status = WAIT_FOR_MUX_CHAN_ID;
static int stuffed_byte;
//#endif
  
  tr_abort_discard = 0;
  tr_abort = 0;
  TR_Signal.SignalType = 0;
  switch ( Signal->SignalType )
  {
    case DRV_SIGTYPE_READ:
      BytesToRead = TR_EndInd - TR_WrInd;
      TR_WorkInd = TR_WrInd;

      do
      {
        Bytes = 0;
        do 
        {
          continue_read = 0;
          /*
           * write counter must not overtake the read counter. If more bytes to be read
           * than space available, then process the bytes in the buffer first and then
           * continue to store new data in the TR_RcvBuffer.
           */
          if ( TR_WrInd >= (TR_RdInd&TR_MaxInd) )
            BytesToRead = TR_EndInd - TR_WrInd;
          else
            BytesToRead = (TR_RdInd&TR_MaxInd) - TR_WrInd; 
          BytesRead = BytesToRead;
          vsi_d_read ( TR_Data.Handle, 0, (void*)&TR_RcvBuffer[TR_WrInd], &BytesRead ); 
          Bytes += BytesRead;
          if ( BytesRead < BytesToRead )
          {
            TR_WrInd += BytesRead;
          }
          else 
          {
            if ( TR_WrInd >= (TR_RdInd&TR_MaxInd) )
              TR_WrInd = 0;
            else
            {
              TR_WrInd += BytesRead;
              continue_read = 1;
              break;
            }
          }
        } while ( BytesRead != TR_RcvBufferSize && BytesRead == BytesToRead );

        if ( Bytes )
        {
          UBYTE cMasked;

          i=0;

          while (i++ < Bytes)
          {
            c = TR_RcvBuffer[(TR_WorkInd++) & TR_MaxInd];

#ifdef _TOOLS_
            if (TR_Data.config & TI_RAW_TRC_MODE) /* we are receiving rawly in TI mode */
            {
              if (!ti_esc_skipped && c==TI_ESC)
              {
                /* the TI driver has inserted an TI_ESC -> skip it */
                ULONG q;
                for (q=TR_WorkInd-1; q>TR_RdInd; q--)
                {
                  TR_RcvBuffer[q & TR_MaxInd]=TR_RcvBuffer[(q-1) & TR_MaxInd];
                }
                TR_RdInd++;
                ti_esc_skipped=1;
                continue;
              }
              ti_esc_skipped=0;
            }
#endif /* _TOOLS_ */

            if ( tr_abort_discard == 1 )
            {
              TR_RdInd++;
              continue;
            }
            switch (TR_Data.rcv_state)
            {
              case WAIT_FOR_STX:
                if (c == STX)
                {
#ifdef _TOOLS_ 
                  ti_len=0;
#endif 
                  TR_Data.rcv_state = WAIT_FOR_IDENT;
                  TR_RdInd = TR_WorkInd;                   /* do not pass the STX to TIF */
                }
                else
                  TR_RdInd++;
                break;
//#ifdef _TOOLS_
              case WAIT_FOR_RAW_TI_S:
                if (c==TI_STX)
                {
                  TR_Data.rcv_state=WAIT_FOR_RAW_TI_ID;
                  TR_RdInd = TR_WorkInd;  /* do not read TI_STX */
                }
                break;
              case WAIT_FOR_RAW_TI_ID:
                if (c==TI_STX)
                {
                  // last TI_STX was end ID -> ignore this start ID
                }
                else if (c==TI_L23_ID)
                {
                  ti_len = 0;
                  ti_id = c;
                  TR_Data.rcv_state=WAIT_FOR_IDENT;
                }
                else if (c == TI_L1_ID || c == TI_RIV_ID)
                {
                  ti_len = 0;
                  ti_id = c;

                  if (ti_id == TI_L1_ID)
                    sigtype = DRV_SIGTYPE_READ_L1;
                  if (ti_id == TI_RIV_ID)
                    sigtype = DRV_SIGTYPE_READ_RIV;

                  counter = 0;
                  TR_RdInd = TR_WorkInd;  /* do not read ti_id */
                  crlf_found = 0;
                  lf_found = 0;
                  TR_Data.rcv_state=WAIT_FOR_RAW_TI_E;
                }
                else
                {
                  mux_chan_id = c;
                  if ( mux_status == WAIT_FOR_MUX_CHAN_ID )
                  {
                    for ( mux_chan = 0; mux_chan < MAX_TST_CHANNEL; mux_chan++ )
                    {
                      if ( tst_mux_chan_struct[mux_chan].channel_id == c )
                      {
                        mux_status = WAIT_FOR_MUX_END_STX;
                        tst_mux_chan_struct[mux_chan].rcv_data_ptr  = &TR_RcvBuffer[(TR_WorkInd) & TR_MaxInd];
                        tst_mux_chan_struct[mux_chan].rcv_data_size = 0;
                        stuffed_byte = 0;
                        TR_RdInd = TR_WorkInd;  /* do not read id */
                        TR_Data.rcv_state=WAIT_FOR_RAW_TI_E;
                        break;
                      }
                    }
                  }
                }
                break;
#ifdef _TOOLS_
              case WAIT_FOR_TI_ID:
                                      /* skip TI ID byte */
                if (c == TI_L23_ID || c == TI_L1_ID || c == TI_RIV_ID)
                {
                  ti_len = 0;
                  ti_id = c;
                  TR_Data.rcv_state = WAIT_FOR_TI_LEN_0;
                }
                else
                {
                  ti_id_not_found = 1;
                }
                break;
              case WAIT_FOR_TI_LEN_0:
                                      /* 
                 * skip length
                 * if length byte == 0 then the next 2 bytes are the length
                 * (patch to handle messages longer than 255 bytes)
                 */
                if ( c != 0 )
                {
                  if (ti_id == TI_L23_ID)
                    TR_Data.rcv_state = WAIT_FOR_IDENT;

                  if (ti_id == TI_L1_ID || ti_id == TI_RIV_ID)
                  {
                    if (ti_id == TI_L1_ID)
                      sigtype = DRV_SIGTYPE_READ_L1;
                    if (ti_id == TI_RIV_ID)
                      sigtype = DRV_SIGTYPE_READ_RIV;

                    TR_Data.rcv_state = WAIT_FOR_DATA;
                    counter = 0;
                    TR_RdInd += 2;  /* do not read ti_id and length */
                    crlf_found = 0;
                    lf_found = 0;
                  }
                  ti_len = c & 0xff;
                }
                else
                  TR_Data.rcv_state = WAIT_FOR_TI_LEN_1;

                break;
              case WAIT_FOR_TI_LEN_1:
                ti_len = c ;
                TR_Data.rcv_state = WAIT_FOR_TI_LEN_2;
                break;
              case WAIT_FOR_TI_LEN_2:
                ti_len |= c << 8;
                TR_Data.rcv_state = WAIT_FOR_IDENT;
                break;
#endif /* ifdef _TOOLS_ */

              case WAIT_FOR_IDENT:
                infoByteAddr = &TR_RcvBuffer[(TR_WorkInd - 1) & TR_MaxInd];
                /* analyze INFO  Byte */
                cMasked = (c &  HDR_VERSION_MASK);
#ifdef _TOOLS_
                if (cMasked == HDR_VALID_VERSION_0) 
                {
                  i--; TR_WorkInd--;
                  printf ("TR: changed to OLD header format automatically\n");
//                    tst_drv_write ( NO_TASK, SYS_MASK, FRM_TST_NAME, (char*)SYSPRIM_GET_STACK_TIME );
                  break;
                }
                /* check for lost bytes on interface */
                if (TR_Data.config & TI_TRC_MODE ||
                    TR_Data.config & TI_RAW_TRC_MODE) /* we are receiving from a arm 7 TARGET */
                { /* TR_WorkInd refs position of first size byte now */
                  if (ti_id_not_found) 
                  {
                    *infoByteAddr = (( *infoByteAddr & ~HDR_IDENT_MASK) | IDENT_ABORT); /* mark as bad */
                  }
                }
#endif /* ifdef _TOOLS_ */
                cMasked = (c &  HDR_IDENT_MASK);
                if (((cMasked == IDENT_PS_PRIM) || (cMasked == IDENT_SYS_PRIM) ||
                    (cMasked == IDENT_ABORT)   || (cMasked == IDENT_TRACE))
                    &&
                    ((c != PROT_PRIM_ID) && (c != PROT_PRIM_ID_32BIT) &&
                     (c != SYS_PRIM_ID) && (c != TRACE_ID))
                    )
                {
                  /* Hey fine, everything went OK! */
                  TR_Data.rcv_state = WAIT_FOR_LENGTH1;
                }
                else
                { /* we have to fake a length for abort trace */
                  tst_trailer_size.part[0] = TST_HEADER_LEADING_FIELDS;
                  tst_trailer_size.part[1] = 0; 
                  MessageLength = tst_trailer_size.val + TST_HEADER_TRAILING_FIELDS;

                  *infoByteAddr = (( *infoByteAddr & ~HDR_IDENT_MASK) | IDENT_ABORT); /* mark as bad */
                  counter = tst_trailer_size.val; /* don't let us read all the following bytes before let TIF tracing ABORT */
                  TR_Data.rcv_state = WAIT_FOR_DATA;
                  tr_abort = 1;
                }
                TR_RdInd = TR_WorkInd - 1; /* skip all preceeding interface sync bytes before triggering TIF */
                break;
              case WAIT_FOR_LENGTH1:       
                  /* the use of the union does rely on identical byte sex! */
#ifdef _SOLARIS_
                  /* temporary hack */
                  tst_trailer_size.part[1] = c;
#else
                  tst_trailer_size.part[0] = c;
#endif
                  TR_Data.rcv_state = WAIT_FOR_LENGTH2;
                  break;
              case WAIT_FOR_LENGTH2:       
#ifdef _SOLARIS_
                  /* temporary hack */
                  tst_trailer_size.part[0] = c;
#else
                  tst_trailer_size.part[1] = c;
#endif
                  /* Bytes after size + bytes till size (inclusive) */
                  MessageLength = tst_trailer_size.val + TST_HEADER_LEADING_FIELDS;
#ifdef _TOOLS_
                  /* In case of a lost character the two length information elements mismatch. */
                  if ( (ti_len != 0) && (tst_trailer_size.val != (ti_len - 3)) )
                  {
                    tst_trailer_size.val = ti_len - 3;
                  }
                  ti_len = 0;
                  if ((tst_trailer_size.val - TST_HEADER_TRAILING_FIELDS) > (unsigned short int)(MAX_PRIM_PARTITION_SIZE - sizeof(T_PRIM_HEADER) - sizeof(T_S_HEADER)))
                  {
                    *infoByteAddr = (( *infoByteAddr & ~HDR_IDENT_MASK) | IDENT_ABORT); /* mark as bad */
                    tst_trailer_size.val = (MAX_PRIM_PARTITION_SIZE - sizeof(T_PRIM_HEADER) - sizeof(T_S_HEADER) -TST_HEADER_TRAILING_FIELDS);
                    counter = tst_trailer_size.val; /* don't let us read all the following bytes before let TIF tracing ABORT */
                    tr_abort = 1;
                  }
#else
                  if ((tst_trailer_size.val - TST_HEADER_TRAILING_FIELDS) > (unsigned short int)(MaxPrimPartSize - sizeof(T_PRIM_HEADER) - sizeof(T_S_HEADER)))
                  {
                    *infoByteAddr = (( *infoByteAddr & ~HDR_IDENT_MASK) | IDENT_ABORT); /* mark as bad */
                    tst_trailer_size.val = (USHORT)(MaxPrimPartSize - sizeof(T_PRIM_HEADER) - sizeof(T_S_HEADER) -TST_HEADER_TRAILING_FIELDS);
                    counter = tst_trailer_size.val; /* don't let us read all the following bytes before let TIF tracing ABORT */
                    tr_abort = 1;
                  }
#endif
                  TR_Data.rcv_state = WAIT_FOR_DATA;
                  counter = 0;
                  break;
              case WAIT_FOR_RAW_TI_E:
                if ( mux_status == WAIT_FOR_MUX_END_STX )
                {
                  if ( stuffed_byte )
                  {
                    stuffed_byte = 0;
                    tst_mux_chan_struct[mux_chan].rcv_data_size++;
                  }
                  else
                  {
                    if ( c != TI_STX )
                    {
                      if ( c == 0x10 )
                      {
                        stuffed_byte = 1;
                      }
                      tst_mux_chan_struct[mux_chan].rcv_data_size++;
                    }
                    else
                    {
                      tst_mux_callback (mux_chan,tst_mux_chan_struct[mux_chan].rcv_data_ptr, tst_mux_chan_struct[mux_chan].rcv_data_size);
                      mux_status = WAIT_FOR_MUX_CHAN_ID;
                      MessageLength = tst_mux_chan_struct[mux_chan].rcv_data_size;
                      TR_Data.rcv_state = WAIT_FOR_RAW_TI_S;
                    }
                  }
                }
#ifdef _TOOLS_
                if (c!=TI_STX)
                {
                  if ( counter == 0 && c == 0x0a )
                  {
                    lf_found = 1;
                  }
                  if ( lf_found && counter == 1 && c == 0x0d)
                  {
                    crlf_found = 1;
                  }
                  counter++;
                }
                else
                {
                  if ( TR_Data.EnabledSignalType & DRV_SIGTYPE_READ )
                  {
                    ti_len=counter;
                    if ( crlf_found == 1 )
                    {
                      ti_len -= 2;
                      TR_RdInd += 2;  /* do not read CR and LF at the beginning */
                    }
                    MessageLength = ti_len;
                    TR_Signal.SignalType = sigtype;
                    TR_Signal.DrvHandle = TR_Data.Handle;
                    (TR_Data.Callback)( &TR_Signal );
                  }
                  TR_Data.rcv_state = TR_Data.idle_state;
                }
#endif
                break;
              case WAIT_FOR_DATA:       
#ifdef _TOOLS_
                if ( ti_len )
                { 
                  if ( counter == 0 && c == 0x0a )
                  {
                    lf_found = 1;
                  }
                  if ( lf_found && counter == 1 && c == 0x0d)
                  {
                    crlf_found = 1;
                  }
                  if ( ++counter >= ti_len )
                  {
                    if ( TR_Data.EnabledSignalType & DRV_SIGTYPE_READ )
                    {
                      if ( crlf_found == 1 )
                      {
                        ti_len -= 2;
                        TR_RdInd += 2;  /* do not read CR and LF at the beginning */
                      }
                      MessageLength = ti_len;
                      TR_Signal.SignalType = sigtype;
                      TR_Signal.DrvHandle = TR_Data.Handle;
                      (TR_Data.Callback)( &TR_Signal );
                    }
                    TR_Data.rcv_state = TR_Data.idle_state;
                  }
                  break;
                }
#endif                  
                if (++counter >= tst_trailer_size.val) /* If all went OK up to now we have to read all remaining bytes from the buffer each for each */
                {
                  if (TR_Data.config & STX_LF_MODE )
                  {
                    TR_Data.rcv_state = WAIT_FOR_CR;
                    break;
                  }
                  else
                  {
                    if ( TR_Data.EnabledSignalType & DRV_SIGTYPE_READ )
                    {
#ifdef _TOOLS_
                      tst_message_received = 1;
#endif
                      TR_Signal.SignalType = DRV_SIGTYPE_READ;
                      TR_Signal.DrvHandle = TR_Data.Handle;
                      (TR_Data.Callback)( &TR_Signal );
                    }
                    TR_Data.rcv_state = TR_Data.idle_state;
                    tst_trailer_size.val = 0;
                    if ( tr_abort == 1 ) /* marked as bad */
                      tr_abort_discard = 1;
#ifdef _TOOLS_
                    ti_id_not_found = 0;
#endif
                  }
                }
                break;
              case WAIT_FOR_CR:
#ifdef _TOOLS_     /* check for lost bytes on interface */
                if (TR_Data.config & STX_LF_MODE )  /* we are receiving from a arm 9 TARGET or Windows stack configured stx */
                {
                  if (c != '\n') 
                  {
                    *infoByteAddr = (( *infoByteAddr & ~HDR_IDENT_MASK) | IDENT_ABORT); /* mark as bad */
                  }
                }
#endif /* _TOOLS_ check for lost bytes on interface */
                if ( TR_Data.EnabledSignalType & DRV_SIGTYPE_READ )
                {
#ifdef _TOOLS_
                  tst_message_received = 1;
#endif
                  TR_Signal.SignalType = DRV_SIGTYPE_READ;
                  TR_Signal.DrvHandle = TR_Data.Handle;
                  (TR_Data.Callback)( &TR_Signal );
                }
                TR_Data.rcv_state = TR_Data.idle_state;
                tst_trailer_size.val = 0;
                if ( tr_abort == 1 ) /* marked as bad */
                  tr_abort_discard = 1;
                break;
              default:
                break; 
            }
          } /*  while (i++ < Bytes) */
        } /* if Bytes loop */
      } while ( continue_read == 1 );
    break;
    case DRV_SIGTYPE_CONNECT:
    case DRV_SIGTYPE_DISCONNECT:
      if ( TR_Data.EnabledSignalType & Signal->SignalType )
      {
        TR_Signal.SignalType = Signal->SignalType;
        TR_Signal.DrvHandle = TR_Data.Handle;
        (TR_Data.Callback)( &TR_Signal );
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
| PROJECT : GSM-Frame (8415)           MODULE  : TR                  |
| STATE   : code                       ROUTINE : TR_Init             |
+--------------------------------------------------------------------+

  PURPOSE : initialize driver

*/
USHORT TR_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo )
{                
USHORT j;
ULONG size = 0;
static const T_DRV_EXPORT TR_Info =
{
  "TR",
  0,
  {
#ifdef _TOOLS_
    TR_Init,
#endif
    TR_Exit,
    TR_Read,
    TR_Write,
    NULL,
    NULL,
    NULL,
    TR_SetSignal,
    TR_ResetSignal,
    TR_SetConfig,
    NULL,
    TR_Callback,
  }
};

  TR_Data.Handle = DrvHandle;

  TR_Data.EnabledSignalType = 0;

  TR_Data.config = 0;

  TR_Data.rcv_state = WAIT_FOR_IDENT;

  TR_Data.idle_state = WAIT_FOR_IDENT;

  TR_Data.Callback = CallbackFunc;

  *DrvInfo = &TR_Info;

  /* 
   * TR_RcvBufferSize must be a power of 2 for proper wrap-around
   */
#ifndef _TOOLS_
  TR_RcvBufferSize = MaxPrimPartSize;
#endif
  j = 0;
  do
  {
    if ( (ULONG)(1 << j) >= TR_RcvBufferSize )  /* Size must be a power of 2 */
    {
      size = 1 << j;
      break;
    }
    else
      j++;
  } while ( size < 0xffff );

  TR_RcvBufferSize = size;
  TR_MaxInd = TR_RcvBufferSize - 1;

  if ( os_AllocateMemory ( NO_TASK, &TR_Data.RcvBuffer, (ULONG)TR_RcvBufferSize,
                           OS_NO_SUSPEND, ext_data_pool_handle ) != OS_OK )
  {
      vsi_o_assert ( NO_TASK, OS_SYST_ERR_NO_MEMORY, __FILE__, __LINE__,
                     "No memory available in TR driver, TR_RcvBufferSize = %d",
                     TR_RcvBufferSize );
    return DRV_INITFAILURE;
  }

  TR_RcvBuffer = (char*)TR_Data.RcvBuffer;
  TR_EndInd = TR_RcvBufferSize;
  TR_WrInd = 0;
  TR_RdInd = 0;
  tst_trailer_size.val = 0;
#ifdef _TOOLS_
  ti_id_not_found = 0;
  ti_esc_skipped = 0;
#endif

  return DRV_OK;           
}
#endif
