/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
+-----------------------------------------------------------------------------
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
|  Purpose :  This modul is part of the entity UART and implements all
|             procedures and functions as described in the
|             SDL-documentation (RX-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_RXF_C
#define UART_RXF_C
#endif /* !UART_RXF_C */

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_UART

#ifndef FF_MULTI_PORT
/*==== INCLUDES =============================================================*/

#ifdef WIN32
#include "nucleus.h"
#endif /* WIN32 */
#include "typedefs.h"   /* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */
#include "macdef.h"     /* to get a lot of macros */
#include "custom.h"
#include "gsm.h"        /* to get a lot of macros */
#include "cnf_uart.h"   /* to get cnf-definitions */
#include "mon_uart.h"   /* to get mon-definitions */
#include "prim.h"       /* to get the definitions of used SAP and directions */
#ifdef DTILIB
#include "dti.h"        /* to get dti lib */
#endif /* DTILIB */
#include "pei.h"        /* to get PEI interface */
#ifdef _TARGET_
#include "../../serial/serialswitch.h"
#include "../../serial/traceswitch.h"
#else /* _TARGET_ */
#include "serial_dat.h" /* to get definitions of serial driver */
#endif /* _TARGET_ */
#include "uart.h"       /* to get the global entity definitions */

#ifdef _SIMULATION_
#include <stdio.h>      /* to get sprintf */
#endif /* _SIMULATION_ */
#include <string.h>    /* JK, delete warnings: to get memcpy */

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : rx_proc_input
+------------------------------------------------------------------------------
| Description : The function rx_proc_input() is the actual callback function
|               to read data from the receive buffer.
|
| Parameters  : uart_device - database for the affected UART device
|
+------------------------------------------------------------------------------
*/
LOCAL void rx_proc_input (T_UART_DATA* uart_device)
{
  USHORT  i, len;
  T_DLC   *dlc;               /* used Data Link Connection */
  UBYTE   *source;            /* Data source pointer */
  UBYTE   *destination;       /* Data destination pointer */
  UBYTE   temp_field;         /* multi purpose value */
  BOOL    channels_complete;  /* indicator of complete reception */
  SHORT   error_code;         /* error code returned from a function */

  TRACE_FUNCTION( "rx_proc_input" );

  /*
   * read data only if we have read permission
   */
  if(uart_device->rx.read_permission)
  {
    if(uart_device->rx.dlc_instance EQ UART_EMPTY_INSTANCE)
      dlc = &uart_device->dlc_table[UART_CONTROL_INSTANCE];
    else
      dlc = &uart_device->dlc_table[uart_device->rx.dlc_instance];
    destination = &dlc->receive_data->buffer[dlc->receive_pos];
#ifdef _SIMULATION_
{
  char buf[80];
  sprintf(buf,"uart_device->rx.dlc_instance: %d", uart_device->rx.dlc_instance);
  TRACE_EVENT(buf);
}
#endif /* _SIMULATION_ */
    if(uart_device->rx.dlc_instance EQ UART_EMPTY_INSTANCE)
    {
      /*
       * Raw data
       */
      /*
       * Is a data descriptor allocated and
       * is the channel ready to receive
       */
      if((dlc->receive_process EQ UART_RX_PROCESS_READY) &&
         (dlc->receive_data))
      {
        for (i=0; i < uart_device->rx.nsource; i++)
        {
          /*
           * are there still data in the ring buffer segment and
           * are there still space in the data descriptor
           */
          if ((uart_device->rx.size[i] > 0) &&
              (dlc->receive_size > dlc->receive_pos))
          {
            len = dlc->receive_size - dlc->receive_pos;
            if (len > uart_device->rx.size[i])
              len = uart_device->rx.size[i];

            memcpy(&dlc->receive_data->buffer[dlc->receive_pos],
                   uart_device->rx.source[i],
                   len);

            uart_device->rx.size[i] -= len;
            dlc->receive_pos        += len;
            dlc->receive_data->len   = dlc->receive_pos;
          }
        }
        dlc->receive_process = UART_RX_PROCESS_COMPLETE;
      }
    }
    else
    {
      channels_complete = FALSE;
      /*
       * for each fragment
       */
      for (i=0; i < uart_device->rx.nsource; i++)
      {
        /*
         * while there is still data in buffer and
         * not yet all channels are processed
         */
        source = uart_device->rx.source[i];
        while((uart_device->rx.size[i] > 0) && (channels_complete NEQ TRUE))
        {
          /*
           * detect HDLC flag
           */
          if(*source EQ UART_HDLC_FLAG)
          {
            switch(uart_device->rx.analyze_state)
            {
              case UART_RX_ERROR:
                /*
                 * begin of frame detected
                 */
                uart_device->rx.analyze_state = UART_RX_BEGIN;
                /* fall through */
              case UART_RX_BEGIN:
                /*
                 * initialize new packet
                 */
                uart_device->rx.stored_len    = 0;
                uart_device->rx.address_field = 0;
                uart_device->rx.fcs           = UART_INITFCS;
                uart_device->rx.escape        = FALSE;
                uart_device->rx.analyze_state = UART_RX_ADDRESS;
                break;

              default:
                /*
                 * detect HDLC flag
                 */
                if(uart_device->rx.stored_len > 0)
                {
                  /*
                   * determine whether FCS already calculated
                   */
                  if(uart_device->rx.analyze_state NEQ UART_RX_FCS)
                  {
                    /*
                     * UART_RX_INFORMATION_...
                     */
                    destination--;
#ifdef _SIMULATION_
                    uart_device->rx.fcs = *destination;
#else /* _SIMULATION_ */
                    uart_device->rx.fcs = uart_device->
                                        fcstab[uart_device->rx.fcs ^ *destination];
#endif /* _SIMULATION_ */
                    /*
                     * remove FCS from data stream
                     */
                    dlc->receive_pos--;
                    uart_data->rx.stored_len--;
                  }
                  if(uart_device->rx.fcs EQ UART_GOODFCS)
                  {
                    /*
                     * no error occured, frame complete
                     */
                    dlc->receive_data->len        = dlc->receive_pos;
                    dlc->receive_process          = UART_RX_PROCESS_COMPLETE;
                    uart_device->rx.analyze_state = UART_RX_END;
                    break;
                  }
                }
                /*
                 * remove receiced frame because of an error
                 */
                switch(uart_device->rx.analyze_state)
                {
                  case UART_RX_INFORMATION:
                  case UART_RX_FCS:
                    if(uart_data->rx.dlc_instance EQ UART_CONTROL_INSTANCE)
                      dlc->receive_pos-= 2;
                    dlc->receive_pos    -= uart_data->rx.stored_len;
                    dlc->receive_process = UART_RX_PROCESS_READY;
                    break;
                  default:
                    /*
                     * Other states are not handeled here
                     */
                    break;
                }
                uart_device->rx.analyze_state = UART_RX_END;
                break;
            }
          }
          else if((*source EQ UART_HDLC_ESCAPE) &&
                  (uart_device->rx.escape NEQ TRUE))
          {
            /*
             * detect Control Escape octet
             */
            uart_device->rx.escape = TRUE;
          }
          else
          {
            /*
             * bit 5 complement for the octet followed by Control Escape
             */
            if(uart_device->rx.escape EQ TRUE)
            {
              *source             ^= 0x20;
              uart_device->rx.escape = FALSE;
            }
            /*
             * store the packet and determine the protocol
             */
            switch(uart_device->rx.analyze_state)
            {
              case UART_RX_ERROR:
                /*
                 * wait for next HDLC flag
                 */
                break;

              case UART_RX_ADDRESS:
                if((*source & UART_EA) EQ UART_EA)
                {
                  /*
                   * FCS calculation
                   */
#ifdef _SIMULATION_
                  uart_device->rx.fcs = *source;
#else /* _SIMULATION_ */
                  uart_device->rx.fcs = uart_device->
                                      fcstab[uart_device->rx.fcs ^ *source];
#endif /* _SIMULATION_ */
                  /*
                   * store Address field
                   */
                  uart_device->rx.address_field = *source;
                  uart_device->rx.analyze_state = UART_RX_CONTROL;
                }
                else
                {
                  /*
                   * invalid frame detected
                   */
                  uart_device->rx.analyze_state = UART_RX_ERROR;
                }
                break;

              case UART_RX_CONTROL:
                switch(*source)
                {
                  case UART_UIH_DATA_FRAME:
                    /*
                     * Data frame detected
                     */
                    temp_field = uart_device->rx.address_field >> UART_DLCI_POS;
                    /*
                     * if it is an existing channel, but not control channel
                     */
                    if((temp_field NEQ UART_DLCI_CONTROL) &&
                       (uart_device->dlc_instance[temp_field] NEQ
                          UART_EMPTY_INSTANCE))
                    {
                      uart_device->rx.dlc_instance =
                        uart_device->dlc_instance[temp_field];
                      dlc = &uart_device->dlc_table[uart_device->rx.dlc_instance];
#ifdef _SIMULATION_
                      TRACE_EVENT_P2("Addressfield found DLCI: 0x%02X \
                                     (dlc_instance 0x%02X)",
                                     temp_field, uart_device->rx.dlc_instance);
#endif /* _SIMULATION_ */
                      if(dlc->receive_process EQ UART_RX_PROCESS_READY)
                      {
                        /*
                         * reception Data channel found
                         * FCS calculation
                         */
#ifdef _SIMULATION_
                        uart_device->rx.fcs = *source;
#else /* _SIMULATION_ */
                        uart_device->rx.fcs = uart_device->
                                            fcstab[uart_device->
                                            rx.fcs ^ *source];
#endif /* _SIMULATION_ */
                        destination = &dlc->receive_data->
                                       buffer[dlc->receive_pos];
                        uart_device->rx.analyze_state = UART_RX_INFORMATION;
                        break;
                      }
                      else if(dlc->receive_process EQ UART_RX_PROCESS_COMPLETE)
                      {
                        channels_complete = TRUE;
                        break;
                      }
                    }
                    /* fall through */
                  case UART_SABM_FRAME:
                  case UART_UA_FRAME:
                  case UART_DM_DATA_FRAME:
                  case UART_DM_CONTROL_FRAME:
                  case UART_DISC_FRAME:
                  case UART_UIH_CONTROL_FRAME:
                    /*
                     * Control frame detected
                     */
                    dlc = &uart_device->dlc_table[UART_CONTROL_INSTANCE];
                    uart_device->rx.dlc_instance = UART_CONTROL_INSTANCE;
                    if(dlc->receive_process EQ UART_RX_PROCESS_READY)
                    {
                      /*
                       * reception Control channel found
                       * FCS calculation
                       */
#ifdef _SIMULATION_
                      uart_device->rx.fcs = *source;
#else /* _SIMULATION_ */
                      uart_device->rx.fcs = uart_device->
                                          fcstab[uart_device->
                                          rx.fcs ^ *source];
#endif /* _SIMULATION_ */
                      destination = &dlc->receive_data->
                                     buffer[dlc->receive_pos];
                      uart_device->rx.analyze_state = UART_RX_INFORMATION;
                      /*
                       * store Address and Control field
                       */
                      *destination = uart_device->rx.address_field;
                      destination++;
                      dlc->receive_pos++;
                      *destination = *source;
                      destination++;
                      dlc->receive_pos++;
                    }
                    else if(dlc->receive_process EQ UART_RX_PROCESS_COMPLETE)
                    {
                      channels_complete = TRUE;
                    }
                    else
                      /*
                       * discard frame, because it is unexpected
                       */
                      uart_device->rx.analyze_state = UART_RX_ERROR;
                    break;
                  default:
                    /*
                     * invalid frame detected
                     */
                    uart_device->rx.analyze_state = UART_RX_ERROR;
                    break;
                }
                break;

              case UART_RX_INFORMATION:
                if(uart_device->rx.stored_len < uart_device->n1)
                {
                  *destination = *source;
                  /*
                   * increase destination pointer
                   */
                  destination++;
                  uart_device->rx.stored_len++;
                  dlc->receive_pos++;
                }
                else
                {
                  /*
                   * FCS calculation
                   */
#ifdef _SIMULATION_
                  uart_device->rx.fcs = *source;
#else /* _SIMULATION_ */
                  uart_device->rx.fcs = uart_device->
                                      fcstab[uart_device->rx.fcs ^ *source];
#endif /* _SIMULATION_ */
                  uart_device->rx.analyze_state = UART_RX_FCS;
                }
                break;

              case UART_RX_FCS:
                /*
                 * remove receiced packet because its to long
                 */
                if(uart_data->rx.dlc_instance EQ UART_CONTROL_INSTANCE)
                  dlc->receive_pos-= 2;
                dlc->receive_pos             -= uart_data->rx.stored_len;
                dlc->receive_process          = UART_RX_PROCESS_READY;
                uart_device->rx.analyze_state = UART_RX_ERROR;
                break;

              default:
                /*
                 * wrong analyze state
                 */
                uart_device->rx.analyze_state = UART_RX_ERROR;
                break;
            }
          }
          if(uart_device->rx.analyze_state EQ UART_RX_END)
            uart_device->rx.analyze_state = UART_RX_BEGIN;
          /*
           * don't move source pointer
           * if each possible channel was processed
           * In this case analyze_state should be UART_RX_CONTROL.
           * The Control field must be analyzed again in next call of
           * this function.
           */
          else if(channels_complete NEQ TRUE)
          {
            /*
             * increase source pointer
             */
            source++;
            uart_device->rx.size[i]--;
          }
        }
      }
    }
  }

  PSIGNAL(hCommUART, UART_DRIVER_RECEIVED_IND, uart_device);

  *uart_device->rx.reInstall = rm_noInstall;

  /*
   * update pointer in UART driver
   */
  if((error_code = UF_InpAvail (uart_device->device)) < 0 )
  {
    TRACE_ERROR_P2("UF Driver: DataPointerUpdate failed, [%d], uart_rxf.c(%d)",
                                                         error_code, __LINE__);
  }
} /* rx_proc_input() */



/*
+------------------------------------------------------------------------------
| Function    : rx_init
+------------------------------------------------------------------------------
| Description : The function rx_init() initializes the RX service
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void rx_init ()
{
#ifndef _SIMULATION_
#ifdef WIN32
#ifndef _TARGET_
  char    buf[80];
#endif /* !_TARGET_ */
  STATUS  sts;
#endif /* WIN32 */
#endif /* !_SIMULATION_ */

  TRACE_FUNCTION( "rx_init" );

#ifndef _SIMULATION_
#ifdef WIN32
  sts = NU_Create_HISR (&uart_data->rx.rx_HISR,
                        "RX_HISR",
                        rx_proc_input,
                        2,
                        uart_data->HISR_stack,
                        HISR_STACK_SIZE);
#ifndef _TARGET_
  sprintf (buf, "NU_Create_HISR(RX) = %d", sts);
  TRACE_EVENT (buf);
#endif /* _TARGET_ */
#endif /* WIN32 */
#endif /* !_SIMULATION_ */

  uart_data->rx.read_permission = FALSE;
  uart_data->rx.prev_lines      = 0;
  uart_data->rx.dlc_instance    = UART_EMPTY_INSTANCE;
  uart_data->rx.escape          = FALSE;
  uart_data->rx.analyze_state   = UART_RX_ERROR;
  uart_data->rx.receive_state   = UART_RX_NOT_RECEIVING;
  uart_data->rx.fcs             = UART_INITFCS;
  uart_data->rx.address_field   = 0;
  uart_data->rx.stored_len      = 0;

  INIT_STATE( UART_SERVICE_RX , RX_DEAD );
} /* rx_init() */



/*
+------------------------------------------------------------------------------
| Function    : rx_readOutFunc_0
+------------------------------------------------------------------------------
| Description : The function rx_readOutFunc_0() is the official callback
|               function to read data from the receive buffer of UART device 0.
|               It just copies the parameters and calls then the actual
|               function.
|
| Parameters  : cldFromIrq - called from interrupt
|               reInstall  - reinstallation mode
|               nsource    - number of source pointers
|               source     - array of source pointers
|               size       - array of sizes for every source pointer
|               state      - state of V.24 lines
|
+------------------------------------------------------------------------------
*/
GLOBAL void rx_readOutFunc_0 (BOOL           cldFromIrq,
                              T_reInstMode  *reInstall,
                              UBYTE          nsource,
                              UBYTE         *source[],
                              USHORT        *size,
                              ULONG          state)
{
#ifndef _SIMULATION_
#ifndef _TARGET_
  char buf[40];
#endif /* !_TARGET_ */
#endif /* !_SIMULATION_ */
  T_UART_DATA* uart_device;

  TRACE_FUNCTION( "rx_readOutFunc_0" );

  /*
   * select UART device 0
   */
  uart_device = &(uart_data_base[0]);

  /*
   * store parameters
   */
  uart_device->rx.cldFromIrq = cldFromIrq;
  uart_device->rx.nsource    = nsource;
  uart_device->rx.source[0]  = source[0];
  uart_device->rx.source[1]  = source[1];
  uart_device->rx.size       = size;
  uart_device->rx.lines      = state;
  uart_device->rx.reInstall  = reInstall;

#ifndef _SIMULATION_
#ifdef WIN32
  if (cldFromIrq)
  {
    STATUS sts;
    /*
     * interrupt context of the UART driver -> activate the HISR
     */
    sts = NU_Activate_HISR (&uart_device->rx.rx_HISR);
#ifndef _TARGET_
    sprintf (buf, "NU_Activate_HISR(RX) = %d", sts);
    TRACE_EVENT (buf);
#endif /* !_TARGET_ */
  }
  else
#endif /* WIN32 */
#endif /* !_SIMULATION_ */
  {
#ifdef _SIMULATION_
    UBYTE*  trace_source[2];
    USHORT  trace_size[2];
    USHORT  i;
    USHORT  pos;
    char    buf[90];

    trace_source[0] = source[0];
    trace_source[1] = source[1];

    trace_size[0] = size[0];
    trace_size[1] = size[1];

    /*
     * trace input
     */
    if((nsource) &&
       (uart_device->rx.read_permission))
    {

      TRACE_EVENT("==== INRAW");
      i   = 0;
      pos = 0;
      while(pos < trace_size[0])
      {
        i+= sprintf(&buf[i], "0x%02x, ", trace_source[0][pos]);
        pos++;
        if(i > 80)
        {
          TRACE_EVENT( buf );
          i = 0;
        }
        else if(pos >= trace_size[0])
        {
          TRACE_EVENT( buf );
        }
      }
      if(nsource > 1)
      {
        i   = 0;
        pos = 0;
        while(pos < trace_size[1])
        {
          i+= sprintf(&buf[i], "0x%02x, ", trace_source[1][pos]);
          pos++;
          if(i > 80)
          {
            TRACE_EVENT( buf );
            i = 0;
          }
          else if(pos >= trace_size[1])
          {
            TRACE_EVENT( buf );
          }
        }
      }
    }
#endif /* _SIMULATION_ */

    /*
     * normal callback from UF_ReadData
     */
    rx_proc_input(uart_device);
  }

} /* rx_readOutFunc_0() */



#ifdef FF_TWO_UART_PORTS
/*
+------------------------------------------------------------------------------
| Function    : rx_readOutFunc_1
+------------------------------------------------------------------------------
| Description : The function rx_readOutFunc_1() is the official callback
|               function to read data from the receive buffer of UART device 1.
|               It just copies the parameters and calls then the actual
|               function.
|
| Parameters  : cldFromIrq - called from interrupt
|               reInstall  - reinstallation mode
|               nsource    - number of source pointers
|               source     - array of source pointers
|               size       - array of sizes for every source pointer
|               state      - state of V.24 lines
|
+------------------------------------------------------------------------------
*/
GLOBAL void rx_readOutFunc_1 (BOOL           cldFromIrq,
                              T_reInstMode  *reInstall,
                              UBYTE          nsource,
                              UBYTE         *source[],
                              USHORT        *size,
                              ULONG          state)
{
#ifndef _SIMULATION_
#ifndef _TARGET_
  char buf[40];
#endif /* !_TARGET_ */
#endif /* !_SIMULATION_ */
  T_UART_DATA* uart_device;

  TRACE_FUNCTION( "rx_readOutFunc_1" );

  /*
   * select UART device 1
   */
  uart_device = &(uart_data_base[1]);

  /*
   * store parameters
   */
  uart_device->rx.cldFromIrq = cldFromIrq;
  uart_device->rx.nsource    = nsource;
  uart_device->rx.source[0]  = source[0];
  uart_device->rx.source[1]  = source[1];
  uart_device->rx.size       = size;
  uart_device->rx.lines      = state;
  uart_device->rx.reInstall  = reInstall;

#ifndef _SIMULATION_
#ifdef WIN32
  if (cldFromIrq)
  {
    STATUS sts;
    /*
     * interrupt context of the UART driver -> activate the HISR
     */
    sts = NU_Activate_HISR (&uart_device->rx.rx_HISR);
#ifndef _TARGET_
    sprintf (buf, "NU_Activate_HISR(RX) = %d", sts);
    TRACE_EVENT (buf);
#endif /* !_TARGET_ */
  }
  else
#endif /* WIN32 */
#endif /* !_SIMULATION_ */
  {
#ifdef _SIMULATION_
    UBYTE*  trace_source[2];
    USHORT  trace_size[2];
    USHORT  i;
    USHORT  pos;
    char    buf[90];

    trace_source[0] = source[0];
    trace_source[1] = source[1];

    trace_size[0] = size[0];
    trace_size[1] = size[1];

    /*
     * trace input
     */
    if((nsource) &&
       (uart_device->rx.read_permission))
    {

      TRACE_EVENT("==== INRAW");
      i   = 0;
      pos = 0;
      while(pos < trace_size[0])
      {
        i+= sprintf(&buf[i], "0x%02x, ", trace_source[0][pos]);
        pos++;
        if(i > 80)
        {
          TRACE_EVENT( buf );
          i = 0;
        }
        else if(pos >= trace_size[0])
        {
          TRACE_EVENT( buf );
        }
      }
      if(nsource > 1)
      {
        i   = 0;
        pos = 0;
        while(pos < trace_size[1])
        {
          i+= sprintf(&buf[i], "0x%02x, ", trace_source[1][pos]);
          pos++;
          if(i > 80)
          {
            TRACE_EVENT( buf );
            i = 0;
          }
          else if(pos >= trace_size[1])
          {
            TRACE_EVENT( buf );
          }
        }
      }
    }
#endif /* _SIMULATION_ */

    /*
     * normal callback from UF_ReadData
     */
    rx_proc_input(uart_device);
  }

} /* rx_readOutFunc_1() */
#endif /* FF_TWO_UART_PORTS */
#endif /* !FF_MULTI_PORT */
