/*******************************************************************************
 *
 * FAXDATA.H
 *
 * This module defines constants used by UART and USART fax/data drivers.
 *
 * (C) Texas Instruments 1999
 *
 ******************************************************************************/

#ifndef __FAXDATA_H__
#define __FAXDATA_H__

/*
 * Returned values.
 */

#define FD_OK            ( 0)
#define FD_SUSPENDED     (-1)
#define FD_NOT_SUPPORTED (-2)
#define FD_NOT_READY     (-3)
#define FD_INTERNAL_ERR  (-9)

/*
 * Line status.
 */

#define FD_LINE_ON  (1)
#define FD_LINE_OFF (0)

/*
 * Signals used in UAF_xxx_GetLineState and UAF_xxx_SetLineState.
 */

#define CTS    ( 0) /* set              */
#define RTS    ( 1) /* get              */
#define DSR    ( 2) /* set              */
#define DTR    ( 3) /* get              */
#define DCD    ( 4) /* set              */
#define BRK    ( 5) /* set/get          */
#define ESC    ( 6) /* get              */
#define TXSTP  ( 7) /* get              */
#define RXSTP  ( 8) /* get              */
#define BRKLEN ( 9) /* set/get (8 bits) */
#define RXBLEV (17) /* get (12 bits)    */
#define SA     (29) /* set/get          */
#define SB     (30) /* set/get          */
#define X      (31) /* set/get          */

#define RI (1)

/*
 * Size of the circular buffers used in the driver.
 */

#define FD_MAX_BUFFER_SIZE (512) /* In bytes. */

/*
 * Type of the returned value for each function.
 */

typedef short T_FDRET;

/*
 * Baud rates.
 */

typedef enum {
    FD_BAUD_AUTO,
    FD_BAUD_75,
    FD_BAUD_150,
    FD_BAUD_300,
    FD_BAUD_600,
    FD_BAUD_1200,
    FD_BAUD_2400,
    FD_BAUD_4800,
    FD_BAUD_7200,
    FD_BAUD_9600,
    FD_BAUD_14400,
    FD_BAUD_19200,
    FD_BAUD_28800,
    FD_BAUD_33900,
    FD_BAUD_38400,
    FD_BAUD_57600,
    FD_BAUD_115200,
    FD_BAUD_203125,
    FD_BAUD_406250,
    FD_BAUD_812500
} T_baudrate;

/*
 * Bits per character.
 */

typedef enum {
    bpc_7,
    bpc_8
} T_bitsPerCharacter;

/*
 * Stop bits.
 */

typedef enum {
    sb_1,
    sb_2
} T_stopBits;

/*
 * Parity.
 */

typedef enum {
    pa_none,
    pa_even,
    pa_odd,
    pa_space
} T_parity;

/*
 * Flow control mode.
 */

typedef enum {
    fc_none,
    fc_rts,
    fc_dtr,
    fc_xoff
} T_flowCtrlMode;

/*
 * Suspend parameter used in UAF_xxx_ReadData and UAF_xxx_WriteData.
 */

typedef enum {
    sm_noSuspend,
    sm_suspend
} T_suspendMode;

/*
 * Install mode parameter used in UAF_xxx_ReadData and UAF_xxx_WriteData.
 */

typedef enum {
    rm_notDefined,
    rm_reInstall,
    rm_noInstall
} T_reInstMode;

#endif /* __FAXDATA_H__ */
