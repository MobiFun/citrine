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
|             SDL-documentation (TX-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_TXF_C
#define UART_TXF_C
#endif /* !UART_TXF_C */

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
| Function    : tx_proc_output
+------------------------------------------------------------------------------
| Description : The function tx_proc_output() is the actual callback function
|               to write data into the send buffer.
|
| Parameters  : uart_device - database for the affected UART device
|
+------------------------------------------------------------------------------
*/
LOCAL void tx_proc_output(T_UART_DATA* uart_device)
{
  USHORT  i, len, pos;
  T_DLC   *dlc;           /* used Data Link Connection */
  UBYTE   transmit_state; /* state of transmission */
  T_desc2* cur_desc;       /* currently used descriptor */
  UBYTE   temp_field;     /* multi purpose value */
  UBYTE   frame_size;     /* numbr of octets in Information field */
  UBYTE   fcs;            /* Frame Check Sequence */
  SHORT   error_code;     /* Error code returned from a function */

  TRACE_FUNCTION( "tx_proc_output" );

  if(uart_device->tx.dlc_instance EQ UART_EMPTY_INSTANCE)
  {
    /*
     * Raw Data
     */
    /*
     * use entry 0 for raw data
     */
    dlc      = &uart_device->dlc_table[UART_CONTROL_INSTANCE];
    cur_desc = dlc->transmit_data;
    /*
     * search next descriptor that includes data
     */
    while((cur_desc) &&
          (dlc->transmit_pos >= cur_desc->len))
    {
      cur_desc          = (T_desc2*)cur_desc->next;
      dlc->transmit_pos = 0;
    }
    /*
     * for each ring buffer segment
     */
    for (i=0; i < uart_device->tx.ndest; i++)
    {
      pos = 0;
      /*
       * while ring buffer segment is not yet full and
       * there are still data to send
       */
      while((uart_device->tx.size[i] > 0) && (cur_desc))
      {
        /*
         * determine length to copy
         */
        len = cur_desc->len - dlc->transmit_pos;
        if(len > uart_device->tx.size[i])
          len = uart_device->tx.size[i];
        /*
         * copy data
         */
        memcpy((char*) &uart_device->tx.dest[i][pos],
               (char*) &cur_desc->buffer[dlc->transmit_pos],
               len);
        /*
         * updata values
         */
        uart_device->tx.size[i]-= len;
        dlc->transmit_pos      += len;
        pos                    += len;
        /*
         * if current descriptor completly send
         * then move to next descriptor
         */
        while((cur_desc) &&
              (dlc->transmit_pos >= cur_desc->len))
        {
          cur_desc          = (T_desc2*)cur_desc->next;
          dlc->transmit_pos = 0;
        }
      }
    }
  }
  else
  {
    /*
     * Multiplexer Data
     */
    dlc        = &uart_device->dlc_table[uart_device->tx.dlc_instance];
    cur_desc   = dlc->transmit_data;
    temp_field = 0;
    /*
     * search next descriptor that includes data
     */
    while((cur_desc) &&
          (dlc->transmit_pos >= cur_desc->len))
    {
      cur_desc          = (T_desc2*)cur_desc->next;
      dlc->transmit_pos = 0;
    }
    if(cur_desc)
    {
      /*
       * initiailze destination values
       */
      i = 0;
      while((i < uart_device->tx.ndest) && (uart_device->tx.size[i] EQ 0))
      {
        i++;
      }
      pos = 0;
      /*
       * send start HDLC Flag
       */
      uart_device->tx.dest[i][pos] = UART_HDLC_FLAG;
      fcs                          = UART_INITFCS;
      transmit_state               = UART_TX_ADDRESS;
      frame_size                   = 0;
      /*
       * increase destination position
       */
      pos++;
      uart_device->tx.size[i]--;
      while((i < uart_device->tx.ndest) &&
            (uart_device->tx.size[i] EQ 0))
      {
        pos = 0;
        i++;
      }
      while(transmit_state NEQ UART_TX_END)
      {
        switch(transmit_state)
        {
          case UART_TX_ADDRESS:
            /*
             * send Address field
             */
            if(uart_device->tx.dlc_instance EQ UART_CONTROL_INSTANCE)
            {
              /*
               * at Control Channel the address field
               * is included in source data
               */
              temp_field = cur_desc->buffer[dlc->transmit_pos];
              /*
               * if current descriptor completly send
               * then move to next descriptor
               */
              dlc->transmit_pos++;
              while((cur_desc) &&
                    (dlc->transmit_pos >= cur_desc->len))
              {
                cur_desc          = (T_desc2*)cur_desc->next;
                dlc->transmit_pos = 0;
              }
            }
            else
            {
              /*
               * at Data Channel the address field
               * is calculated with the DLCI
               */
              temp_field = (dlc->dlci << UART_DLCI_POS) | UART_EA;
            }
            /*
             * calculate FCS
             */
            fcs = uart_device->fcstab[fcs ^ temp_field];
            /*
             * next field is Control field
             */
            transmit_state = UART_TX_CONTROL;
            break;

          case UART_TX_CONTROL:
            /*
             * send Control field
             */
            if(uart_device->tx.dlc_instance EQ UART_CONTROL_INSTANCE)
            {
              /*
               * at Control Channel the control field
               * is included in source data
               */
              temp_field = cur_desc->buffer[dlc->transmit_pos];
              /*
               * if current descriptor completly send
               * then move to next descriptor
               */
              dlc->transmit_pos++;
              while((cur_desc) &&
                    (dlc->transmit_pos >= cur_desc->len))
              {
                cur_desc          = (T_desc2*)cur_desc->next;
                dlc->transmit_pos = 0;
              }
            }
            else
            {
              /*
               * at Data Channel the control field
               * is always an UIH frame with P/F bit set to 0
               */
              temp_field = UART_UIH_DATA_FRAME;
            }
            /*
             * calculate FCS
             */
            fcs = uart_device->fcstab[fcs ^ temp_field];
            /*
             * if there are still data to send the
             * next field is Information field
             * otherwise next field is FCS field
             */
            if(cur_desc)
              transmit_state = UART_TX_INFORMATION;
            else
              transmit_state = UART_TX_FCS;
            break;

          case UART_TX_INFORMATION:
            /*
             * send Information field
             */
            temp_field = cur_desc->buffer[dlc->transmit_pos];
            /*
             * check if there is still data in the current descriptor and
             * the maximum frame size is not yet reached
             */
            dlc->transmit_pos++;
            frame_size++;
            if((frame_size >= uart_device->n1) ||
               (dlc->transmit_pos >= cur_desc->len))
            {
              /*
               * if current descriptor completly send
               * then move to next descriptor
               */
              while((cur_desc) &&
                    (dlc->transmit_pos >= cur_desc->len))
              {
                cur_desc          = (T_desc2*)cur_desc->next;
                dlc->transmit_pos = 0;
              }
              /*
               * if no more data to send available or
               * maximum frame size is reached then
               * the next field is FCS field
               */
              if((frame_size >= uart_device->n1) ||
                 (cur_desc EQ NULL))
                transmit_state = UART_TX_FCS;
            }
            break;

          case UART_TX_FCS:
            /*
             * send FCS field
             */
#ifdef _SIMULATION_
            /*
             * clear FCS field in simulation mode
             */
            temp_field = UART_GOODFCS;
#else /* _SIMULATION_ */
            temp_field = (0xff - fcs);
#endif /* _SIMULATION_ */
            /*
             * frame complete
             */
            transmit_state = UART_TX_END;
            break;
          default:
            TRACE_EVENT_P1("Warning: Unexpected TX ISR state %d", transmit_state);
            break;
        }
        if((temp_field EQ UART_HDLC_FLAG)   ||
           (temp_field EQ UART_HDLC_ESCAPE) ||
           (temp_field EQ uart_device->xon) ||
           (temp_field EQ uart_device->xoff))
        {
          /*
           * send Control Escape and map character
           */
          /*lint -e661 (Warning -- access of out-of-bounds pointer) */
          uart_device->tx.dest[i][pos] = UART_HDLC_ESCAPE;
          /*lint +e661 (Warning -- access of out-of-bounds pointer) */
          temp_field                  ^= 0x20;
          /*
           * increase destination position
           */
          pos++;
          uart_device->tx.size[i]--;
          while((i < uart_device->tx.ndest) &&
                (uart_device->tx.size[i] EQ 0))
          {
            pos = 0;
            i++;
          }
        }
        /*
         * send character
         */
        /*lint -e661 -e662 (Warning -- access/creation of out-of-bounds pointer) */
        uart_device->tx.dest[i][pos] = temp_field;
        /*lint +e661 +e662 (Warning -- access/creation of out-of-bounds pointer) */
        /*
         * increase destination position
         */
        pos++;
        uart_device->tx.size[i]--;
        while((i < uart_device->tx.ndest) &&
              (uart_device->tx.size[i] EQ 0))
        {
          pos = 0;
          i++;
        }
      }
      /*
       * send stop HDLC Flag
       */
      /*lint -e661 -e662 (Warning -- access/creation of out-of-bounds pointer) */
      uart_device->tx.dest[i][pos] = UART_HDLC_FLAG;
      /*lint +e661 +e662 (Warning -- access/creation of out-of-bounds pointer) */
      /*
       * update size value
       */
      uart_device->tx.size[i]--;
    }
  }
  /*
   * write current descriptor back to table
   */
  dlc->transmit_data = cur_desc;

#ifndef _SIMULATION_
  PSIGNAL(hCommUART, UART_DRIVER_SENT_IND, uart_device);
#endif /* !_SIMULATION_ */
  *uart_device->tx.reInstall = rm_noInstall;

  /*
   * update pointer in UART driver
   */
  if((error_code = UF_OutpAvail (uart_device->device)) < 0)
  {
    TRACE_ERROR_P2("UF Driver: data pointer update failed, [%d], uart_txf.c(%d)",
                                               error_code, __LINE__);
  }

} /* tx_proc_output() */



/*
+------------------------------------------------------------------------------
| Function    : tx_init
+------------------------------------------------------------------------------
| Description : The function tx_init() initializes the TX service.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void tx_init ()
{
#ifndef _SIMULATION_
#ifdef WIN32
#ifndef _TARGET_
  char    buf[80];
#endif /* !_TARGET_ */
  STATUS  sts;
#endif /* WIN32 */
#endif /* !_SIMULATION_ */

  TRACE_FUNCTION( "tx_init" );

#ifndef _SIMULATION_
#ifdef WIN32
  sts = NU_Create_HISR (&uart_data->tx.tx_HISR,
                        "TX_HISR",
                        tx_proc_output,
                        2,
                        uart_data->HISR_stack,
                        HISR_STACK_SIZE);
#ifndef _TARGET_
  sprintf (buf, "NU_Create_HISR(TX) = %d", sts);
  TRACE_EVENT (buf);
#endif /* !_TARGET_ */
#endif /* WIN32 */
#endif /* !_SIMULATION_ */

  uart_data->tx.lines         = 0x80000000; /* invalid */
  uart_data->tx.dlc_instance  = UART_EMPTY_INSTANCE;
  uart_data->tx.p_zero        = 0;
  uart_data->tx.send_state    = UART_TX_NOT_SENDING;

  INIT_STATE( UART_SERVICE_TX , TX_DEAD );
} /* tx_init() */



/*
+------------------------------------------------------------------------------
| Function    : tx_flushUart
+------------------------------------------------------------------------------
| Description : The function tx_flushUart() flush the output buffer of the
|               UART driver.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void tx_flushUart ()
{
#ifndef _TARGET_
  USHORT  oa;             /* output available */
#endif /* !_TARGET_ */
#ifndef ALR
  T_UFRET mt;
#endif /* ALR */
  USHORT  counter;

  TRACE_FUNCTION( "tx_flushUart" );

  counter = 0;
  while(
#ifndef ALR
        ((mt = UF_CheckXEmpty(uart_data->device)) == UF_NOT_READY) ||
#endif /* !ALR */
        (UF_OutpAvail (uart_data->device) < UF_MAX_BUFFER_SIZE))
  {
#ifndef _TARGET_
    oa = UF_OutpAvail (uart_data->device);
    TRACE_EVENT_P1("waiting - output not flushed oa:%d",oa);
#endif /* !_TARGET_ */
    /*
     * poll permanent in the first 500ms
     * after that poll 1 minute only every second
     * after that give up
     */
    if(counter < 50)
    {
      if(vsi_t_sleep (VSI_CALLER ONE_FRAME) NEQ VSI_OK)
      {
        TRACE_ERROR_P1("VSI entity: Can't suspend thread, uart_txf.c(%d)", 
                                                                __LINE__);
      }
    }
    else if(counter < 110)
    {
      if(vsi_t_sleep (VSI_CALLER 1000) NEQ VSI_OK)
      {
        TRACE_ERROR_P1("VSI entity: Can't suspend thread, uart_txf.c(%d)", 
                                                                __LINE__);
      }
    }
    else
    {
      break;
    }
    counter++;
  }
} /* tx_flushUart() */



/*
+------------------------------------------------------------------------------
| Function    : tx_next_send_allowed
+------------------------------------------------------------------------------
| Description : The function tx_next_send_allowed() determines which dlc is the
|               next dlc allow to send. The result of the calculation is stored
|               in dlc_instance.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void tx_next_send_allowed ()
{
  UBYTE   diff;
  UBYTE   inst;
  UBYTE   next_inst;
  T_DLC*  dlc;

  TRACE_FUNCTION( "tx_next_send_allowed" );

  diff = 255;
  next_inst = UART_EMPTY_INSTANCE;
  for(inst = 0; inst <= UART_MAX_NUMBER_OF_CHANNELS; inst++)
  {
    dlc = &uart_data->dlc_table[inst];
    if(dlc->transmit_data)
    {
      if(dlc->p_counter EQ uart_data->tx.p_zero)
      {
        uart_data->tx.dlc_instance = inst;
        return;
      }
      if(diff > (dlc->p_counter - uart_data->tx.p_zero))
      {
        diff = dlc->p_counter - uart_data->tx.p_zero;
        next_inst = inst;
      }
    }
  }
  uart_data->tx.p_zero+= diff;
  uart_data->tx.dlc_instance = next_inst;
} /* tx_next_send_allowed() */



/*
+------------------------------------------------------------------------------
| Function    : tx_writeInFunc_0
+------------------------------------------------------------------------------
| Description : The function tx_writeInFunc_0() is the official callback
|               function to write data into the send buffer of UART device 0.
|               It just copies the parameters and calls then the actual
|               function.
|
| Parameters  : cldFromIrq - called from interrupt
|               reInstall  - reinstallation mode
|               ndest      - number of destination pointers
|               dest       - array of destination pointers
|               size       - array of sizes for every destinition pointer
|
+------------------------------------------------------------------------------
*/
GLOBAL void tx_writeInFunc_0 (BOOL           cldFromIrq,
                              T_reInstMode  *reInstall,
                              UBYTE          ndest,
                              UBYTE         *dest[],
                              USHORT        *size)
{
#ifndef _SIMULATION_
#ifndef _TARGET_
  char buf[40];
#endif /* !_TARGET_ */
#endif /* !_SIMULATION_ */
  T_UART_DATA* uart_device;

  TRACE_FUNCTION( "tx_writeInFunc_0" );

  /*
   * select UART device 0
   */
  uart_device = &(uart_data_base[0]);

  /*
   * store parameters
   */
  uart_device->tx.cldFromIrq = cldFromIrq;
  uart_device->tx.ndest      = ndest;
  uart_device->tx.dest[0]    = dest[0];
  uart_device->tx.dest[1]    = dest[1];
  uart_device->tx.size       = size;
  uart_device->tx.reInstall  = reInstall;

#ifndef _SIMULATION_
#ifdef WIN32
  if (cldFromIrq)
  {
    STATUS sts;
    /*
     * interrupt context of the UART driver -> activate the HISR
     */
    sts = NU_Activate_HISR (&uart_device->tx.tx_HISR);
#ifndef _TARGET_
    sprintf (buf, "NU_Activate_HISR(TX) = %d", sts);
    TRACE_EVENT (buf);
#endif /* !_TARGET_ */
  }
  else
#endif /* WIN32 */
#endif /* !_SIMULATION_ */
  {
    /*
     * normal callback from UF_WriteData
     */
    tx_proc_output(uart_device);

#ifdef _SIMULATION_
    {
      /*
       * trace output
       */
      UBYTE*  trace_dest[2];
      USHORT  trace_size[2];
      USHORT  i;
      USHORT  pos;
      char    buf[90];


      trace_dest[0] = dest[0];
      trace_dest[1] = dest[1];

      trace_size[0] = size[0];
      trace_size[1] = size[1];

      trace_size[0]-= uart_device->tx.size[0];
      trace_size[1]-= uart_device->tx.size[1];

      if((trace_size[0]) ||
         (trace_size[1]))
      {

        TRACE_EVENT("=== OUTRAW");
        i   = 0;
        pos = 0;
        while(pos < trace_size[0])
        {
          i+= sprintf(&buf[i], "0x%02x, ", trace_dest[0][pos]);
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
        i   = 0;
        pos = 0;
        while(pos < trace_size[1])
        {
          i+= sprintf(&buf[i], "0x%02x, ", trace_dest[1][pos]);
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
  }
} /* tx_writeInFunc_0() */



#ifdef FF_TWO_UART_PORTS
/*
+------------------------------------------------------------------------------
| Function    : tx_writeInFunc_1
+------------------------------------------------------------------------------
| Description : The function tx_writeInFunc_1() is the official callback
|               function to write data into the send buffer of UART device 0.
|               It just copies the parameters and calls then the actual
|               function.
|
| Parameters  : cldFromIrq - called from interrupt
|               reInstall  - reinstallation mode
|               ndest      - number of destination pointers
|               dest       - array of destination pointers
|               size       - array of sizes for every destinition pointer
|
+------------------------------------------------------------------------------
*/
GLOBAL void tx_writeInFunc_1 (BOOL           cldFromIrq,
                              T_reInstMode  *reInstall,
                              UBYTE          ndest,
                              UBYTE         *dest[],
                              USHORT        *size)
{
#ifndef _SIMULATION_
#ifndef _TARGET_
  char buf[40];
#endif /* !_TARGET_ */
#endif /* !_SIMULATION_ */
  T_UART_DATA* uart_device;

  TRACE_FUNCTION( "tx_writeInFunc_1" );

  /*
   * select UART device 1
   */
  uart_device = &(uart_data_base[1]);

  /*
   * store parameters
   */
  uart_device->tx.cldFromIrq = cldFromIrq;
  uart_device->tx.ndest      = ndest;
  uart_device->tx.dest[0]    = dest[0];
  uart_device->tx.dest[1]    = dest[1];
  uart_device->tx.size       = size;
  uart_device->tx.reInstall  = reInstall;

#ifndef _SIMULATION_
#ifdef WIN32
  if (cldFromIrq)
  {
    STATUS sts;
    /*
     * interrupt context of the UART driver -> activate the HISR
     */
    sts = NU_Activate_HISR (&uart_device->tx.tx_HISR);
#ifndef _TARGET_
    sprintf (buf, "NU_Activate_HISR(TX) = %d", sts);
    TRACE_EVENT (buf);
#endif /* !_TARGET_ */
  }
  else
#endif /* WIN32 */
#endif /* !_SIMULATION_ */
  {
    /*
     * normal callback from UF_WriteData
     */
    tx_proc_output(uart_device);

#ifdef _SIMULATION_
    {
      /*
       * trace output
       */
      UBYTE*  trace_dest[2];
      USHORT  trace_size[2];
      USHORT  i;
      USHORT  pos;
      char    buf[90];


      trace_dest[0] = dest[0];
      trace_dest[1] = dest[1];

      trace_size[0] = size[0];
      trace_size[1] = size[1];

      trace_size[0]-= uart_device->tx.size[0];
      trace_size[1]-= uart_device->tx.size[1];

      if((trace_size[0]) ||
         (trace_size[1]))
      {

        TRACE_EVENT("=== OUTRAW");
        i   = 0;
        pos = 0;
        while(pos < trace_size[0])
        {
          i+= sprintf(&buf[i], "0x%02x, ", trace_dest[0][pos]);
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
        i   = 0;
        pos = 0;
        while(pos < trace_size[1])
        {
          i+= sprintf(&buf[i], "0x%02x, ", trace_dest[1][pos]);
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
  }
} /* tx_writeInFunc_1() */
#endif /* FF_TWO_UART_PORTS */
#endif /* !FF_MULTI_PORT */
