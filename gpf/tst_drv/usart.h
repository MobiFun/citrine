/* 
+------------------------------------------------------------------------------
|  File:       usart.h
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
|  Purpose :  This Modul defines functions for actual or simulated 
|             USART comunication between two PS-Frames.
|             Use US_set_mode() to select actual USART under windows95 or 
|             under windowsNT, or to select simulated USART under win95/NT
+----------------------------------------------------------------------------- 
*/ 

#ifndef USART_H
#define USART_H

/*
 * Device addresses 
 */ 
#define USART_DC	        (USART_BASE + 0x00)     /* Data Channel */   
#define USART_CC            (USART_BASE + 0x04)     /* Control Channel */
#define USART_CLK           (USART_BASE + 0x08)     /* Clock */         

#define USART_CTL           (USART_BASE + 0x0C)     /* FIFO control */
#define USART_FSTAT         (USART_BASE + 0x10)     /* FIFO status */
#define USART_FTHR          (USART_BASE + 0x14)     /* FIFO threshold */ 

/*
 * USART_CLK bit definitions
 */ 
#define US_CLK_CLK_SEL  0x0001
#define US_CLK_TXC_SEL  0x0002
#define US_CLK_RXC_SEL  0x0004
#define US_CLK_TXC_CTL  0x0100
#define US_CLK_RXC_CTL  0x0200
#define US_CLK_RESET    0x0400

/*
 * USART_MODE bit definitions
 */ 
/* Sync/Async and Baud rate */ 
#define US_MODE_SYNC    0x00
#define US_MODE_ASYNC   0x01
#define US_MODE_ASYNC16 0x02
#define US_MODE_SYNC64  0x03

/* Character length */
#define US_MODE_CHAR5   (0x00 << 2)
#define US_MODE_CHAR6   (0x01 << 2)
#define US_MODE_CHAR7   (0x02 << 2)
#define US_MODE_CHAR8   (0x03 << 2)

/* Parity */
#define US_MODE_PEN     0x10
#define US_MODE_EP      0x20

/* Stop bits */
#define US_MODE_STOP1   (0x01 << 6)
#define US_MODE_STOP15  (0x02 << 6)
#define US_MODE_STOP2   (0x03 << 6)

/* Baud rates */
#define US_BAUD_115200  0
#define US_BAUD_57600   1
#define US_BAUD_38400   2
#define US_BAUD_33900   3
#define US_BAUD_28800   4
#define US_BAUD_19200   5
#define US_BAUD_14400   6
#define US_BAUD_9600    7
#define US_BAUD_4800    8
#define US_BAUD_2400    9
#define US_BAUD_1200   10
#define US_BAUD_600    11
#define US_BAUD_300    12
#define US_BAUD_150    13
#define US_BAUD_75     14
#define US_BAUD_128000 15
#define US_BAUD_256000 16


/*
 * USART COMMAND bit definitions
 */ 
#define US_CMD_TXEN     0x01
#define US_CMD_DTR      0x02
#define US_CMD_RXE      0x04
#define US_CMD_SBRK     0x08
#define US_CMD_ER       0x10
#define US_CMD_RTS      0x20
#define US_CMD_IR       0x40
#define US_CMD_EH       0x80


/*
 * USART STATUS bit definitions
 */ 
#define US_STAT_TXRDY    0x01
#define US_STAT_RXRDY    0x02
#define US_STAT_TXEMPTY  0x04
#define US_STAT_PE       0x08
#define US_STAT_OE       0x10
#define US_STAT_FE       0x20
#define US_STAT_SBDET    0x40
#define US_STAT_DSR      0x80

/* 
 * FIFO control
 */
#define US_CTL_RST_FIFO    0x0001
#define US_CTL_FIFOBYPASS  0x0002
#define US_CTL_AUTOFC      0x0004
#define US_CTL_MS_MSK      0x0008
#define US_CTL_RDRY_MSK    0x0100
#define US_CTL_TO_MSK      0x0200
#define US_CTL_TDRY_MSK    0x1000
#define US_CTL_TXE_MSK     0x2000

#define US_FSTAT_GFE       0x0001
#define US_FSTAT_GPE       0x0002
#define US_FSTAT_MODSTAT   0x0008
#define US_FSTAT_RXOE      0x0020
#define US_FSTAT_RXFULL    0x0040
#define US_FSTAT_RXEMPTY   0x0080 
#define US_FSTAT_RDRY      0x0100
#define US_FSTAT_TIMEOUT   0x0200
#define US_FSTAT_TXFULL    0x0800
#define US_FSTAT_TDRY      0x1000
#define US_FSTAT_TXE       0x2000
#define US_FSTAT_TXEMPTY   0x4000

/*
 * mode constants
 */
#define US_MODE_95         1
#define US_MODE_NT         2
#define US_MODE_SIM        3
#define US_MODE_FILE       4

/*
 * Prototypes
 */ 
void US_set_mode(int mode);
int  US_get_mode(void);

int  US_Init(unsigned int baudRate, int fifoSize, char flow_ctrl, void (func(void)), const char* fname );
BOOL US_Close(void);
BYTE US_ReadChar(void);
void US_Handler(void);
void US_WriteChar(int usart_id, char ch);
void US_WriteNChars(int usart_id, BYTE *s, unsigned int n);
int  US_IsChar(void);
void US_WriteString(int usart_id, char *s);
void US_Handler(void);
void US_SetFlowCtrl (char flowCtrl);
/*
int  US_InitBlk(int baudRate, int fifoSize, NU_HISR *hP );
*/
ULONG US_ReadNChars(int usart_id, BYTE *s, ULONG n);

/*
 defines for name_change US -> UT
*/
#ifndef _TARGET_
#define UT_DEVICE_0 (0)

#define UT_BAUD_256000 US_BAUD_256000 
#define UT_BAUD_128000 US_BAUD_128000
#define UT_BAUD_115200 US_BAUD_115200
#define UT_BAUD_57600  US_BAUD_57600
#define UT_BAUD_38400  US_BAUD_38400
#define UT_BAUD_33900  US_BAUD_33900
#define UT_BAUD_28800  US_BAUD_28800
#define UT_BAUD_19200  US_BAUD_19200
#define UT_BAUD_14400  US_BAUD_14400
#define UT_BAUD_9600   US_BAUD_9600
#define UT_BAUD_4800   US_BAUD_4800
#define UT_BAUD_2400   US_BAUD_2400
#define UT_BAUD_1200   US_BAUD_1200
#define UT_BAUD_600    US_BAUD_600
#define UT_BAUD_300    US_BAUD_300
#define UT_BAUD_150    US_BAUD_150
#define UT_BAUD_75     US_BAUD_75

#define UT_MODE_95     US_MODE_95
#define UT_MODE_NT     US_MODE_NT
#define UT_MODE_SIM    US_MODE_SIM
#define UT_MODE_FILE   US_MODE_FILE

#define UT_set_mode    US_set_mode
#define UT_get_mode    US_get_mode
#define UT_Init        US_Init
#define UT_Close       US_Close
#define UT_ReadChar    US_ReadChar
#define UT_ReadNChars  US_ReadNChars
#define UT_Handler     US_Handler
#define UT_WriteChar   US_WriteChar
#define UT_WriteNChars US_WriteNChars
#define UT_IsChar      US_IsChar
#define UT_WriteString US_WriteString
#define UT_SetFlowCtrl US_SetFlowCtrl 

#endif


#endif
