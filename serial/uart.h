/*******************************************************************************
 *
 * UART.H
 *
 * This module allows to use the UARTs of chipset 1.5 in interrupt mode.
 * The driver calls a user's function when characters are received.
 *
 * (C) Texas Instruments 1999 - 2003
 *
 ******************************************************************************/

#ifndef __UART_H__
#define __UART_H__

typedef enum {
    UA_UART_0,  /* IrDA */
    UA_UART_1   /* Modem */
    #if (CHIPSET == 12)
      , UA_UART_2  /* Modem 2 */
    #endif
} T_tr_UartId;

#if (CHIPSET == 12)
  #define NUMBER_OF_TR_UART (3)
#else
  #define NUMBER_OF_TR_UART (2)
#endif

#ifndef C_EXTERN
  #if 1 //(OP_L1_STANDALONE == 1)
    #define C_EXTERN  extern
  #else
    #define C_EXTERN
  #endif
#endif

/*******************************************************************************
 *
 *                           initialize_uart_sleep
 * 
 * Purpose  : Performs basic UART hardware initialization including sleep mode.
 *
 * Arguments: In : uart_id : UART id.
 *            Out: none
 *
 * Returns: none
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN void initialize_uart_sleep (T_tr_UartId uart_id);

/*******************************************************************************
 *
 *                               UA_Init
 * 
 * Purpose  : Initializes the module and the UART.
 *
 * Arguments: In : uart_id : UART id.
 *                 baudrate: baud rate selected.
 *                 callback: user's function called characters are received.
 *            Out: none
 *
 * Returns: none
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN void UA_Init (T_tr_UartId uart_id,
              T_tr_Baudrate baudrate,
              void (callback_function (void)));

/*******************************************************************************
 *
 *                           UA_ReadNChars
 * 
 * Purpose  : Reads N characters from the RX buffer.
 *
 * Arguments: In : uart_id      : UART id.
 *                 buffer       : buffer address where the characters are
 *                                copied.
 *                 chars_to_read: number of characters to read.
 *            Out: none
 *
 * Returns  : The number of characters read.
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN SYS_UWORD32 UA_ReadNChars (T_tr_UartId uart_id,
                           char *buffer,
                           SYS_UWORD32 chars_to_read);

/*******************************************************************************
 *
 *                           UA_ReadNBytes
 * 
 * Purpose  : Reads and destuff N bytes from the RX buffer.
 *
 * Arguments: In : uart_id      : UART id.
 *                 buffer       : buffer address where the bytes are copied.
 *                 chars_to_read: number of bytes to read.
 *            Out: eof_detected : indicates if an EOF has been detected.
 *
 * Returns  : The number of bytes read.
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN SYS_UWORD32 UA_ReadNBytes (T_tr_UartId uart_id,
                           char *buffer,
                           SYS_UWORD32 bytes_to_read,
                           SYS_BOOL *eof_detected);

/*******************************************************************************
 *
 *                           UA_WriteNChars
 * 
 * Purpose  : Writes N characters in the TX FIFO.
 *
 * Arguments: In : uart_id       : UART id.
 *                 buffer        : buffer address from which characters are
 *                                 written.
 *                 bytes_to_write: number of bytes to write.
 *            Out: none
 *
 * Returns  : Number of bytes written.
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN SYS_UWORD32 UA_WriteNChars (T_tr_UartId uart_id,
                            char *buffer,
                            SYS_UWORD32 chars_to_write);

/*******************************************************************************
 *
 *                           UA_EncapsulateNChars
 * 
 * Purpose  : Writes N characters in the TX FIFO in encapsulating them with 2
 *            STX bytes (one at the beginning and one at the end).
 *
 * Arguments: In : uart_id       : UART id.
 *                 buffer        : buffer address from which characters are
 *                                 written.
 *                 chars_to_write: number of chars to write.
 *            Out: none
 *
 * Returns  : Number of chars written.
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN SYS_UWORD32 UA_EncapsulateNChars (T_tr_UartId uart_id,
                                  char *buffer,
                                  SYS_UWORD32 chars_to_write);

/*******************************************************************************
 *
 *                           UA_WriteNBytes
 * 
 * Purpose  : Writes N bytes in the TX FIFO in encapsulating with 2 STX bytes
 *            at the beginning and the end of the frame, and in making byte
 *            stuffing.
 *
 * Arguments: In : uart_id       : UART id.
 *                 buffer        : buffer address from which bytes are
 *                                 written.
 *                 bytes_to_write: number of bytes to write.
 *            Out: none
 *
 * Returns  : Number of bytes written.
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN SYS_UWORD32 UA_WriteNBytes (T_tr_UartId uart_id, 
                            SYS_UWORD8 *buffer, 
                            SYS_UWORD32 bytes_to_write);
                
/*******************************************************************************
 *
 *                            UA_WriteChar
 * 
 * Purpose  : Writes a character in the TX FIFO.
 *
 * Arguments: In : uart: UART id.
 *                 character
 *            Out: none
 *
 * Returns  : none
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN void UA_WriteChar (T_tr_UartId uart_id,
                   char character);

/*******************************************************************************
 *
 *                          UA_WriteString
 * 
 * Purpose  : Writes a null terminated string in the TX FIFO.
 *
 * Arguments: In : uart_id: UART id.
 *                 buffer : buffer address from which characters are written.
 *            Out: none
 *
 * Returns  : none
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN void UA_WriteString (T_tr_UartId uart_id,
                     char *buffer);

/*******************************************************************************
 *
 *                             UA_EnterSleep
 * 
 * Purpose  : Checks if UART is ready to enter Deep Sleep. If ready, enables
 *            wake-up interrupt.
 *
 * Arguments: In : uart_id : UART id.
 *            Out: none
 *
 * Returns: 0	 : Deep Sleep is not possible.
 *          >= 1 : Deep Sleep is possible.
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN SYS_BOOL UA_EnterSleep (T_tr_UartId uart_id);

/*******************************************************************************
 *
 *                              UA_WakeUp
 * 
 * Purpose  : Wakes up UART after Deep Sleep.
 *
 * Arguments: In : uart_id : UART id.
 *            Out: none
 *
 * Returns: none
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

C_EXTERN void UA_WakeUp (T_tr_UartId uart_id);

/*******************************************************************************
 *
 *                       UA_InterruptHandler
 * 
 * Purpose  : Interrupt handler.
 *
 * Arguments: In : uart_id         : origin of interrupt
 *                 interrupt_status: source of interrupt
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

void UA_InterruptHandler (T_tr_UartId uart_id,
                          SYS_UWORD8 interrupt_status);

#undef C_EXTERN

#endif /* __UART_H__ */
