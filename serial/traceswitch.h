/*******************************************************************************
 *
 * TRACESWITCH.H
 *
 * This module defines constants used by UART and USART trace drivers.
 *
 * (C) Texas Instruments 1999
 *
 ******************************************************************************/

#ifndef __TRACESWITCH_H__
#define __TRACESWITCH_H__

typedef enum {
    TR_BAUD_406250,
    TR_BAUD_115200,
    TR_BAUD_57600,
    TR_BAUD_38400,
    TR_BAUD_33900,
    TR_BAUD_28800,
    TR_BAUD_19200,
    TR_BAUD_14400,
    TR_BAUD_9600,
    TR_BAUD_4800,
    TR_BAUD_2400,
    TR_BAUD_1200,
    TR_BAUD_600,
    TR_BAUD_300,
    TR_BAUD_150,
    TR_BAUD_75
} T_tr_Baudrate;

#endif /* __TRACESWITCH_H__ */
