/*******************************************************************************
 *
 * UART.C
 *
 * This module allows to use the UARTs of chipset 1.5 in interrupt mode for
 * the Receive side and in polling mode for the Transmit side.
 * The driver calls a user's function when characters are received.
 *
 * (C) Texas Instruments 1999
 *
 ******************************************************************************/

#include "../include/config.h"
#include "../include/sys_types.h"

#include "serialswitch.h"
#include "uart.h"

#include <string.h>

#include "../bsp/mem.h"

#if (BOARD != 34)
/*
 * Needed to reset and restart the sleep timer in case of incoming characters.
 */

extern SYS_BOOL uart_sleep_timer_enabled;
#endif

#define BUFFER_SIZE (512) /* In bytes. */
#define FIFO_SIZE    (64) /* In bytes. */

#define STX                  0x02
#define DLE                  0x10

/*
 * TLR is used to program the RX FIFO trigger levels. FCR[7:4] are  not used.
 */
 
#define RX_FIFO_TRIGGER_LEVEL (12 << 4)


/*
 * 16750 addresses. Registers accessed when LCR[7] = 0.
 */

#define RHR (0x00) /* Rx buffer register - Read access   */
#define THR (0x00) /* Tx holding register - Write access */
#define IER (0x01) /* Interrupt enable register          */

/*
 * 16750 addresses. Registers accessed when LCR[7] = 1.
 */

#define DLL (0x00) /* Divisor latch (LSB) */
#define DLM (0x01) /* Divisor latch (MSB) */


/*
 * EFR is accessed when LCR[7:0] = 0xBF.
 */

#define EFR (0x02) /* Enhanced feature register */


/*
 * 16750 addresses. Bit 5 of the FCR register is accessed when LCR[7] = 1.
 */

#define IIR  (0x02) /* Interrupt ident. register - Read only */
#define FCR  (0x02) /* FIFO control register - Write only    */
#define LCR  (0x03) /* Line control register                 */
#define MCR  (0x04) /* Modem control register                */
#define LSR  (0x05) /* Line status register                  */
#define MSR  (0x06) /* Modem status register                 */
#define TCR  (0x06) /* Transmission control register         */
#define TLR  (0x07) /* Trigger level register                */
#define MDR1 (0x08) /* Mode definition register 1            */
#define SCR  (0x10) /* Supplementary Control register        */
#define SSR  (0x11) /* Supplementary Status register         */


/*
 * Supplementary control register.
 */

#define TX_EMPTY_CTL_IT (0x08)
#define RX_CTS_WAKE_UP_ENABLE_BIT (4) /* Use RESET_BIT and SET_BIT macros. */

/*
 * Enhanced feature register.
 */
 
#define ENHANCED_FEATURE_BIT (4) /* Use RESET_BIT and SET_BIT macros. */

/*
 * Mode definition register 1.
 */

#define UART_MODE             (0x00)
#define SIR_MODE              (0x01)
#define UART_MODE_AUTOBAUDING (0x02) /* Reserved in UART/IrDA. */
#define RESET_DEFAULT_STATE   (0x07)
#define IR_SLEEP_DISABLED     (0x00)
#define IR_SLEEP_ENABLED      (0x08)
#define SIR_TX_WITHOUT_ACREG2 (0x00) /* Reserved in UART/modem. */
#define SIR_TX_WITH_ACREG2    (0x20) /* Reserved in UART/modem. */
#define FRAME_LENGTH_METHOD   (0x00) /* Reserved in UART/modem. */
#define EOT_BIT_METHOD        (0x80) /* Reserved in UART/modem. */

/*
 * Supplementary Status Register
 */

#define TX_FIFO_FULL (0x01)


/*
 * Interrupt enable register.
 */

#define ERBI  (0x01) /* Enable received data available interrupt            */
#define ETBEI (0x02) /* Enable transmitter holding register empty interrupt */
#define ELSI  (0x04) /* Enable receiver line status interrupt               */
#define EDSSI (0x08) /* Enable modem status interrupt                       */
#define IER_SLEEP (0x10)  /* Enable sleep mode                              */

/*
 * Modem control register.
 */

#define MDTR (0x01) /* Data terminal ready. */
#define MRTS (0x02) /* Request to send.     */
#define TCR_TLR_BIT (6)

/*
 * Line status register.
 */

#define DR   (0x01) /* Data ready                                  */
#define OE   (0x02) /* Overrun error                               */
#define PE   (0x04) /* Parity error                                */
#define FE   (0x08) /* Framing error                               */
#define BI   (0x10) /* Break interrupt                             */
#define THRE (0x20) /* Transmitter holding register (FIFO empty)   */
#define TEMT (0x40) /* Transmitter empty (FIFO and TSR both empty) */

/*
 * Interrupt identification register.
 * Bit 0 is set to 0 if an IT is pending.
 * Bits 1 and 2 are used to identify the IT.
 */

#define IIR_BITS_USED  (0x07)
#define IT_NOT_PENDING (0x01)
#define RX_DATA        (0x04)
#define TX_EMPTY       (0x02)
#define MODEM_STATUS   (0x00)

/*
 * Line control register.
 */

#define WLS_5         (0x00) /* Word length: 5 bits                    */
#define WLS_6         (0x01) /* Word length: 6 bits                    */
#define WLS_7         (0x02) /* Word length: 7 bits                    */
#define WLS_8         (0x03) /* Word length: 8 bits                    */
#define STB           (0x04) /* Number of stop bits: 0: 1, 1: 1,5 or 2 */
#define PEN           (0x08) /* Parity enable                          */
#define EPS           (0x10) /* Even parity select                     */
#define BREAK_CONTROL (0x40) /* Enable a break condition               */
#define DLAB          (0x80) /* Divisor latch access bit               */
#define DIV_EN_BIT    (7)

/*
 * FIFO control register.
 */

#define FIFO_ENABLE   (0x01)
#define RX_FIFO_RESET (0x02)
#define TX_FIFO_RESET (0x04)

/*
 * These macros allow to read and write a UART register.
 */

#define READ_UART_REGISTER(UART,REG) \
            *((volatile SYS_UWORD8 *) ((UART)->base_address + (REG)))

#define WRITE_UART_REGISTER(UART,REG,VALUE) \
            *((volatile SYS_UWORD8 *) ((UART)->base_address + (REG))) = (VALUE)

#define RESET_BIT(UART,REG,BIT)    \
			(WRITE_UART_REGISTER ( \
			     UART, REG, READ_UART_REGISTER (UART, REG) & ~(1 << (BIT))))

#define SET_BIT(UART,REG,BIT)      \
			(WRITE_UART_REGISTER ( \
			     UART, REG, READ_UART_REGISTER (UART, REG) | (1 << (BIT))))
            
/* 
 * These macros allow to enable or disable the wake-up interrupt.
 */

#define ENABLE_WAKEUP_INTERRUPT(UART)   \
	SET_BIT(UART, SCR, RX_CTS_WAKE_UP_ENABLE_BIT);

#define DISABLE_WAKEUP_INTERRUPT(UART)   \
	RESET_BIT(UART, SCR, RX_CTS_WAKE_UP_ENABLE_BIT);


/* 
 * This macro allows to know if the RX buffer is full. It must be called only
 * from the RX interrupt handler. If it is called from the application, the
 * rx_in pointer may be updated if a RX interrupt occurs.
 */

#define RX_BUFFER_FULL(UART)                                      \
            (((UART)->rx_in == (UART)->rx_out - 1) ||             \
             ((UART)->rx_in == (UART)->rx_out + BUFFER_SIZE - 1))


/* 
 * This allows monitor the last 32 inbound buffers gotten from the RX FIFO.
 */

//#define UART_RX_BUFFER_DUMP

#ifdef UART_RX_BUFFER_DUMP
struct {
    char rx_buffer[(BUFFER_SIZE + 1) << 5];
    char *rx_in;
    int  errors_count;
    int  wrong_interrupt_status;
} uart_rx_buffer_dump = {0};
#endif


typedef struct s_uart {

    SYS_UWORD32 base_address;

    /*
     * Buffers management.
     */
    
    char rx_buffer[BUFFER_SIZE + 1];
    char *rx_in;
    char *rx_out;
    void (*callback_function) (void);

    /*
     * Errors counters.
     */

    SYS_UWORD32 framing_error;
    SYS_UWORD32 parity_error;
    SYS_UWORD32 overrun_error;

    /*
     * Framing flags.
     */

    SYS_BOOL dle_detected;
    SYS_BOOL inframe;
    SYS_BOOL encapsulation_flag;
    unsigned char frame_length;

} t_uart;

static t_uart uart_parameter[NUMBER_OF_TR_UART];

static const SYS_UWORD32 base_address[NUMBER_OF_TR_UART] =
{
    MEM_UART_IRDA,
    MEM_UART_MODEM
    #if (CHIPSET == 12)
      , MEM_UART_MODEM2
    #endif
};


/*
 * DLL (LSB) and DLH (MSB) registers values using the 13 MHz clock.
 */

static const SYS_UWORD8 dll[] =
{
      2, /* 406250 baud. */
      7, /* 115200 baud. */
     14, /*  57600 baud. */
     21, /*  38400 baud. */
     24, /*  33900 baud. */
     28, /*  28800 baud. */
     42, /*  19200 baud. */
     56, /*  14400 baud. */
     84, /*   9600 baud. */
    169, /*   4800 baud. */
     83, /*   2400 baud. */
    165, /*   1200 baud. */
     74, /*    600 baud. */
    148, /*    300 baud. */
     40, /*    150 baud. */
     81  /*     75 baud. */
};

static const SYS_UWORD8 dlh[] =
{
     0, /* 406250 baud. */
     0, /* 115200 baud. */
     0, /*  57600 baud. */
     0, /*  38400 baud. */
     0, /*  33900 baud. */
     0, /*  28800 baud. */
     0, /*  19200 baud. */
     0, /*  14400 baud. */
     0, /*   9600 baud. */
     0, /*   4800 baud. */
     1, /*   2400 baud. */
     2, /*   1200 baud. */
     5, /*    600 baud. */
    10, /*    300 baud. */
    21, /*    150 baud. */
    42  /*     75 baud. */
};


/*******************************************************************************
 *
 *                              read_rx_fifo
 * 
 * Purpose  : Check the bytes written into the RX FIFO. Characters are not
 *            written in the RX buffer if it is full. The HISR is called if
 *            enough characters are received.
 *
 * Arguments: In : uart: pointer on UART structure.
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static void
read_rx_fifo (t_uart *uart)
{
    volatile SYS_UWORD8 status;
    int                 error_detected;
    SYS_UWORD8          char_received;

#if (BOARD != 34)
    /*
     * Since new characters have been received, the sleep timer is reset then
     * restarted preventing the system to enter deep-sleep for a new period of
     * time.
     */

    SER_activate_timer_hisr ();
    uart_sleep_timer_enabled = 1;
#endif
    
    status = READ_UART_REGISTER (uart, LSR);

    while (status & DR) { /* While RX FIFO is not empty... */

        error_detected = 0;

        char_received = READ_UART_REGISTER (uart, RHR);

        /*
         * Check if an error (overrun, parity, framing or break) is associated with the
         * received data. If there is an error the byte is not copied into the
         * RX buffer.
         */

        if (status & (OE | PE | FE | BI)) {

            if (status & PE)
                uart->parity_error++;

            if (status & FE)
                uart->framing_error++;

            if (status & OE)
                uart->overrun_error++;

            error_detected = 1;
        }

        /*
         * If there is no error the byte is copied into the RX
         * buffer if it is not full.
         */

        if (!error_detected && !RX_BUFFER_FULL (uart)) {

            *(uart->rx_in++) = char_received;

            if (uart->rx_in == &(uart->rx_buffer[0]) + BUFFER_SIZE + 1)
                uart->rx_in = &(uart->rx_buffer[0]);

#ifdef UART_RX_BUFFER_DUMP
            *(uart_rx_buffer_dump.rx_in)++ = char_received;

            if (uart_rx_buffer_dump.rx_in == uart_rx_buffer_dump.rx_buffer + sizeof (uart_rx_buffer_dump.rx_buffer))
                uart_rx_buffer_dump.rx_in = uart_rx_buffer_dump.rx_buffer;
        }
        else {
            uart_rx_buffer_dump.errors_count++;
#endif
        }

        status = READ_UART_REGISTER (uart, LSR);
    }

    /*
     * Call the user's function.
     */

    if (uart->callback_function != NULL)
        (*(uart->callback_function)) ();
}

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

void
initialize_uart_sleep (T_tr_UartId uart_id)
{
    t_uart     *uart;
    int        index;
    SYS_UWORD8 dummy;

    for (index = 0; index < NUMBER_OF_TR_UART; index++)
        uart_parameter[index].base_address = base_address[index];
        
    uart = &(uart_parameter[uart_id]);

    /*
     * Mask all interrupts causes and disable sleep mode.
     */

    WRITE_UART_REGISTER (uart, IER, 0x00);

    /*
     * Reset UART mode configuration.
     */
     
    WRITE_UART_REGISTER (uart, MDR1, RESET_DEFAULT_STATE);

    /*
     * LCR[7:0] = 0xBF to allow to access EFR 
     * EFR[4] = 1 to allow to program IER[4].
     */
     
    WRITE_UART_REGISTER (uart, LCR, 0xBF);
    SET_BIT (uart, EFR, ENHANCED_FEATURE_BIT);
    WRITE_UART_REGISTER (uart, LCR, 0x83);

    /*
     * Enable FIFO and reset them.
     */

    WRITE_UART_REGISTER (uart, FCR, FIFO_ENABLE   |
                                    RX_FIFO_RESET |
                                    TX_FIFO_RESET);

    /*
     * Program the baud generator (dummy 115200).
     */

    WRITE_UART_REGISTER (uart, DLL, 0x07);
    WRITE_UART_REGISTER (uart, DLM, 0x00);

    /*
     * LCR[7] = 0 to allow to access IER and RHR - normal mode.
     */
     
    RESET_BIT (uart, LCR, DIV_EN_BIT);

    /*
     * Select UART mode.
     */
     
    WRITE_UART_REGISTER (uart, MDR1, UART_MODE);

    /*
     * Clear Interrupt and check that Rx FIFO is empty.
     */

    dummy = READ_UART_REGISTER (uart, IIR);

    while (READ_UART_REGISTER (uart, LSR) & DR)
	    dummy = READ_UART_REGISTER (uart, RHR);

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Enable sleep mode.
     */

    WRITE_UART_REGISTER (uart, IER, IER_SLEEP);
#endif
}


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

void
UA_Init (T_tr_UartId uart_id,
         T_tr_Baudrate baudrate,
         void (callback_function (void)))
{
    t_uart *uart;
    int    index;

#ifdef UART_RX_BUFFER_DUMP
    uart_rx_buffer_dump.rx_in = uart_rx_buffer_dump.rx_buffer;
#endif

    for (index = 0; index < NUMBER_OF_TR_UART; index++)
        uart_parameter[index].base_address = base_address[index];
        
    uart = &(uart_parameter[uart_id]);

    uart->rx_in  = &(uart->rx_buffer[0]);
    uart->rx_out = &(uart->rx_buffer[0]);

    uart->callback_function = callback_function;

    uart->framing_error = 0;
    uart->parity_error  = 0;
    uart->overrun_error = 0;

    uart->dle_detected = 0;
    uart->inframe = 0;
    uart->encapsulation_flag = 0;
    uart->frame_length = 0;

    /*
     * Mask all interrupts causes and disable sleep mode.
     */

    WRITE_UART_REGISTER (uart, IER, 0x00);

    /*
     * Reset UART mode configuration.
     */
     
    WRITE_UART_REGISTER (uart, MDR1, RESET_DEFAULT_STATE   |
                                     IR_SLEEP_DISABLED     |
                                     SIR_TX_WITHOUT_ACREG2 |
                                     FRAME_LENGTH_METHOD);

    /*
     * FIFO configuration.
     * EFR[4] = 1 to allow to program FCR[5:4] and MCR[7:5].
     */
     
    WRITE_UART_REGISTER (uart, LCR, 0xBF);
    SET_BIT (uart, EFR, ENHANCED_FEATURE_BIT);

    /*
     * Select the word length, the number of stop bits , the parity and set
     * LCR[7] (DLAB) to allow to program FCR, DLL and DLM.
     */

    WRITE_UART_REGISTER (uart, LCR, WLS_8 | DLAB);

    /*
     * Program the trigger levels.
     * MCR[6] must be set to 1.
     */
     
    SET_BIT (uart, MCR, TCR_TLR_BIT);
    WRITE_UART_REGISTER (uart, TCR, 0x0F);
    WRITE_UART_REGISTER (
        uart, TLR, RX_FIFO_TRIGGER_LEVEL);
    
    /*
     * Program the FIFO control register. Bit 0 must be set when other FCR bits
     * are written to or they are not programmed.
     * FCR is a write-only register. It will not be modified.
     */

    WRITE_UART_REGISTER (uart, FCR, FIFO_ENABLE   |
                                    RX_FIFO_RESET | /* self cleared */
                                    TX_FIFO_RESET); /* self cleared */

    /*
     * Program the baud generator.
     */

    WRITE_UART_REGISTER (uart, DLL, dll[baudrate]);
    WRITE_UART_REGISTER (uart, DLM, dlh[baudrate]);

    
    /*
     * Reset LCR[7] (DLAB) to have access to the RBR, THR and IER registers.
     */

    WRITE_UART_REGISTER (uart, LCR, READ_UART_REGISTER (uart, LCR) & ~DLAB);


    /*
     * Select UART mode.
     */
     
    WRITE_UART_REGISTER (uart, MDR1, UART_MODE             |
                                     IR_SLEEP_DISABLED     |
                                     SIR_TX_WITHOUT_ACREG2 |
                                     FRAME_LENGTH_METHOD);

#if ((CHIPSET == 5) || (CHIPSET == 6))
    /*
     * Unmask RX interrupt
     */

    WRITE_UART_REGISTER (uart, IER, ERBI);
#else
    /*
     * Unmask RX interrupt and allow sleep mode.
     */

    WRITE_UART_REGISTER (uart, IER, ERBI | IER_SLEEP);
#endif
}

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

SYS_UWORD32
UA_ReadNChars (T_tr_UartId uart_id,
               char *buffer,
               SYS_UWORD32 chars_to_read)
{
    SYS_UWORD32 chars_in_rx_buffer;
    SYS_UWORD32 chars_to_copy;
    SYS_UWORD32 chars_written;
    char        *rx_in;
    t_uart      *uart;
    
    uart = &(uart_parameter[uart_id]);

    /*
     * A copy of the rx_in pointer is used because it may be updated by
     * the interrupt handler.
     * Get the number of bytes available in the RX buffer.
     */
         
    rx_in = uart->rx_in;

    if (uart->rx_out <= rx_in)
        chars_in_rx_buffer = (SYS_UWORD32) (rx_in - uart->rx_out);
    else
        chars_in_rx_buffer = (SYS_UWORD32) (rx_in - uart->rx_out + BUFFER_SIZE + 1);

    /*
     * No more bytes than those received may be written in the output buffer.
     */

    if (chars_in_rx_buffer >= chars_to_read)
        chars_to_copy = chars_to_read;
    else
        chars_to_copy = chars_in_rx_buffer;

    chars_written = chars_to_copy;

    /*
     * Write the received bytes in the output buffer.
     */

    while (chars_to_copy) {

        *(buffer++) = *(uart->rx_out++);
        chars_to_copy--;

        if (uart->rx_out == &(uart->rx_buffer[0]) + BUFFER_SIZE + 1)
            uart->rx_out = &(uart->rx_buffer[0]);
    }

    return (chars_written);
}

/*******************************************************************************
 *
 *                           UA_ReadNBytes
 * 
 * Purpose  : Reads and destuff N bytes from the RX buffer.
 *
 * Arguments: In : uart_id      : UART id.
 *                 buffer       : buffer address where the bytes are copied.
 *                 chars_to_read: number of bytes to read.
 *            Out: eof_detected : indicates if an EOF has been detected. Possible
 *                                values are:
 *                                 - 0: EOF not detected,
 *                                 - 1: EOF detected and no more bytes left,
 *                                 - 2: EOF not detected and more bytes left.
 *                                      Users must invoke this function one more
 *                                      time in order to get those remaining
 *                                      bytes,
 *                                 - 3: EOF detected and more bytes left. Users
 *                                      must invoke this function one more time
 *                                      in order to get those remaining bytes.
 *
 * Returns  : The number of bytes read.
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

SYS_UWORD32
UA_ReadNBytes (T_tr_UartId uart_id,
               char *buffer_p,
               SYS_UWORD32 bytes_to_read,
               SYS_BOOL *eof_detected_p)
{
    SYS_UWORD32 bytes_written;
    SYS_UWORD32 bytes_in_rx_buffer;
    SYS_UWORD32 bytes_to_process;
    t_uart      *uart_p;
    char        *rx_in_p;

    bytes_written = 0;
    uart_p = &(uart_parameter[uart_id]);

    /*
     * A copy of the rx_in pointer is used because it may be updated by
     * the interrupt handler.
     * Get the number of bytes available in the RX buffer.
    */

    rx_in_p = uart_p->rx_in;

    if (uart_p->rx_out <= rx_in_p)
        bytes_in_rx_buffer = (SYS_UWORD32) (rx_in_p - uart_p->rx_out);
    else
        bytes_in_rx_buffer = (SYS_UWORD32) (rx_in_p - uart_p->rx_out + BUFFER_SIZE + 1);

    /*
     * No more bytes than those received may be processed and then written
     * in the output buffer.
     */

    if (bytes_in_rx_buffer > bytes_to_read) {
        bytes_to_process = bytes_to_read;

        /*
         * More bytes left. Users must invoke this function one more time
		 * in order to get those remaining bytes.
		 */

        *eof_detected_p  = 2;
    }
    else {
        bytes_to_process = bytes_in_rx_buffer;

        /*
         * No more bytes left.
		 */

        *eof_detected_p  = 0;
    }

    /*
     * Perform the byte destuffing and then write the "valid" received bytes in
     * the output buffer.
     */

	while ((bytes_to_process) && !(*eof_detected_p & 0x01)) {

        switch (*(uart_p->rx_out)) {

            /*
             * Current byte is DLE.
             */

            case DLE:
                                                        
                if (!uart_p->dle_detected) {

                    /*
                     * No DLE previously detected => 
                     * Skip the current byte and set the flag.
                     */

                    uart_p->dle_detected = 1;
                    uart_p->rx_out++;
                }

                else { /* if (uart_p->dle_detected) */

                    if (uart_p->inframe) {

                        /*
                         * DLE previously detected AND currently inside of a frame =>
                         * Copy the current byte in the output buffer, reset the flag
                         * and increase the frame length.
                         */

                        uart_p->dle_detected = 0;
                        uart_p->frame_length++;
                        *(buffer_p++) = *(uart_p->rx_out++);
                        bytes_written++;
                    }

                    else { /* if (!uart_p->inframe) */

                        /*
                         * DLE previously detected AND currently outside of a frame =>
                         * Skip the current byte.
                         */

                        uart_p->rx_out++;
                    }
                }

            break; /* case DLE */

            /*
             * Current byte is STX.
             */

            case STX:
                                                      
                if ((!uart_p->dle_detected) && (uart_p->inframe)) {

                    /*
                     * No DLE previously detected AND currently inside of a frame.
                     */

                    if (uart_p->frame_length) {

                        /*
                         * Frame length is not zero (End of Frame) => 
                         * Skip the current byte and set the flags (EOF).
                         */

                        uart_p->inframe = 0;
                        uart_p->frame_length = 0;
                        uart_p->rx_out++;

                        /*
                         * More bytes left.
                         */

                        if ((*eof_detected_p == 0) && (bytes_to_process))
                            *eof_detected_p = 2;

                        /*
                         * EOF detected.
                         */

                        (*eof_detected_p)++;
                    }

                    else { /* if (!uart_p->frame_length) */

                        /*
                         * Frame length is zero (STX followed by another STX =
                         * Synchro lost but start of a new frame) =>
                         * Skip the current byte and keep the flag set.
                         */

                        uart_p->rx_out++;
                    }
                }

                else if ((!uart_p->dle_detected) && (!uart_p->inframe)) {

                    /*
                     * No DLE previously detected AND currently outside of a
                     * frame (Start of Frame) =>
                     * Skip the current byte and set the flag.
                     */

                    uart_p->inframe = 1;
                    uart_p->rx_out++;
                }

                else if ((uart_p->dle_detected) && (uart_p->inframe)) {

                    /*
                     * DLE previously detected AND currently inside of a frame =>
                     * Copy the current byte in the output buffer, reset the flag
                     * and increase the frame length.
                     */

                    uart_p->dle_detected = 0;
                    uart_p->frame_length++;
                    *(buffer_p++) = *(uart_p->rx_out++);
                    bytes_written++;
                }

                else if ((uart_p->dle_detected) && (!uart_p->inframe)) {

                    /*
                     * DLE previously detected AND currently outside of a frame =>
                     * Skip the current byte and reset the flag.
                     */

                    uart_p->dle_detected = 0;
                    uart_p->rx_out++;
                }

            break; /* case STX */

            /*
             * Current byte is neither DLE nor STX.
             */

            default:

                if (uart_p->inframe) {

                    /*
                     * Currently inside of a frame =>
                     * Copy the current byte in the output buffer and increase
                     * the frame length.
                     */

                    uart_p->frame_length++;
                    *(buffer_p++) = *(uart_p->rx_out++);
                    bytes_written++;
                }

                else { /* if (!uart_p->inframe) */

                    /*
                     * Currently outside of a frame =>
                     * Skip the current byte.
                     */

                    uart_p->rx_out++;
                }

            break; /* default */
        }

        if (uart_p->rx_out == &(uart_p->rx_buffer[0]) + BUFFER_SIZE + 1)
            uart_p->rx_out = &(uart_p->rx_buffer[0]);

        bytes_to_process--;
    }

    return (bytes_written);
}


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

SYS_UWORD32
UA_WriteNChars (T_tr_UartId uart_id,
                char *buffer,
                SYS_UWORD32 chars_to_write)
{
    SYS_UWORD32 chars_in_tx_fifo;
    SYS_UWORD32 chars_written;
    t_uart      *uart;
    
    chars_written = 0;
    uart = &(uart_parameter[uart_id]);

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Disable sleep mode.
     */
              
    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

    /*
     * Copy the input buffer to the TX FIFO.
     * Ulyssse Bug #44: TX FIFO full status bit (SSR[1]) is corrupted during
     * one period of Bclock => Workaround S/W.
     * Write in TX FIFO only if FIFO is empty instead of writing in TX FIFO
     * while FIFO is not full.
     */

    if (READ_UART_REGISTER (uart, LSR) & THRE) {

        chars_in_tx_fifo = 0;

        while ((chars_written < chars_to_write) &&
		       (chars_in_tx_fifo < FIFO_SIZE)) {

            WRITE_UART_REGISTER (uart, THR, *(buffer++));
            chars_written++;
            chars_in_tx_fifo++;
        }
    }

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Re-enable sleep mode.
     */

    WRITE_UART_REGISTER ( 
        uart, IER, READ_UART_REGISTER (uart, IER) | IER_SLEEP);
#endif

    return (chars_written);
}


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

SYS_UWORD32
UA_EncapsulateNChars (T_tr_UartId uart_id,
                      char *buffer,
                      SYS_UWORD32 chars_to_write)
{
    SYS_UWORD32 chars_written;
    SYS_UWORD32 chars_in_tx_fifo;
    t_uart      *uart;
    
    chars_written = 0;
    uart = &(uart_parameter[uart_id]);

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Disable sleep mode.
     */
              
    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

    /*
     * Copy the input buffer to the TX FIFO.
     * Ulyssse Bug #44: TX FIFO full status bit (SSR[1]) is corrupted during
     * one period of Bclock => Workaround S/W.
     * Write in TX FIFO only if FIFO is empty instead of writing in TX FIFO
     * while FIFO is not full.
     */

    if (READ_UART_REGISTER (uart, LSR) & THRE) {

        chars_in_tx_fifo = 0;

        /*
         * Check if the message has been already encapsulated.
         */

        if (!uart->encapsulation_flag) {
            /*
             * Write STX in the TX FIFO and set the flag.
             */

            WRITE_UART_REGISTER (uart, THR, STX);
            chars_in_tx_fifo++;
            uart->encapsulation_flag = 1;
        }

        /*
         * Keep one char margin in the TX FIFO for the last STX.
         */

        while ((chars_written < chars_to_write) &&
               (chars_in_tx_fifo < (FIFO_SIZE-1))) {

            WRITE_UART_REGISTER (uart, THR, *(buffer++));
            chars_written++;
            chars_in_tx_fifo++;
        }

        /*
         * Append STX byte at the end if the frame is complete.
         */

        if (chars_written == chars_to_write) {

            /*
             * Write STX in the TX FIFO and reset the flag.
             */

            WRITE_UART_REGISTER (uart, THR, STX);
            uart->encapsulation_flag = 0;
        }
    }

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Re-enable sleep mode.
     */

    WRITE_UART_REGISTER ( 
        uart, IER, READ_UART_REGISTER (uart, IER) | IER_SLEEP);
#endif

    return (chars_written);
}


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

SYS_UWORD32
UA_WriteNBytes (T_tr_UartId uart_id,
                SYS_UWORD8 *buffer,
                SYS_UWORD32 bytes_to_write)
{
    SYS_UWORD32 bytes_written;
    SYS_UWORD32 bytes_in_tx_fifo;
    t_uart      *uart;
        
    bytes_written = 0;
    uart = &(uart_parameter[uart_id]);

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Disable sleep mode.
     */
              
    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

    /*
     * Copy the input buffer to the TX FIFO.
     * Ulyssse Bug #44: TX FIFO full status bit (SSR[1]) is corrupted during
     * one period of Bclock => Workaround S/W.
     * Write in TX FIFO only if FIFO is empty instead of writing in TX FIFO
     * while FIFO is not full.
     */

    if (READ_UART_REGISTER (uart, LSR) & THRE) {

        bytes_in_tx_fifo = 0;

        /*
         * Check if the message has been already encapsulated.
         */

        if (!uart->encapsulation_flag) {

            /*
             * Write STX in the TX FIFO and set the flag.
             */

            WRITE_UART_REGISTER (uart, THR, STX);
            bytes_in_tx_fifo++;
            uart->encapsulation_flag = 1;
        }

        /*
         * Keep 2 chars margin in the FIFO, one for the stuffing (if necessary)
         * and one for the last STX.
         */

        while ((bytes_written < bytes_to_write) && 
               (bytes_in_tx_fifo < (FIFO_SIZE-2))) {

            /*
             * Check for STX or DLE in order to perform the stuffing.
             */

            if ((*(buffer) == STX) || (*(buffer) == DLE)) {

                /*
                 * Write DLE in the TX FIFO.
                 */

                WRITE_UART_REGISTER (uart, THR, DLE);
                bytes_in_tx_fifo++;
            }

            WRITE_UART_REGISTER (uart, THR, *(buffer++));
            bytes_written++;
            bytes_in_tx_fifo++;
        }

        /*
         * Append STX byte at the end if the frame is complete.
         */

        if (bytes_written == bytes_to_write) {

            /*
             * Write STX in the TX FIFO and reset the flag.
             */

            WRITE_UART_REGISTER (uart, THR, STX);
            uart->encapsulation_flag = 0;
        }
    }

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Re-enable sleep mode.
     */

    WRITE_UART_REGISTER ( 
        uart, IER, READ_UART_REGISTER (uart, IER) | IER_SLEEP);
#endif

    return (bytes_written);
}


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

void
UA_WriteChar (T_tr_UartId uart_id,
              char character)
{
    (void) UA_WriteNChars (uart_id, &character, 1);
}

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

void
UA_WriteString (T_tr_UartId uart_id,
                char *buffer)
{
    (void) UA_WriteNChars (uart_id, buffer, strlen (buffer));
}

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

SYS_BOOL
UA_EnterSleep (T_tr_UartId uart_id)
{
    t_uart              *uart;
    SYS_BOOL            deep_sleep;
    volatile SYS_UWORD8 status;
        
    uart = &(uart_parameter[uart_id]);
    deep_sleep = 0;

    /*
     * Check if RX & TX FIFOs are both empty
     */

    status = READ_UART_REGISTER (uart, LSR);

    if (!(status & DR) &&
        (status & TEMT)) {

#if ((CHIPSET != 5) && (CHIPSET != 6))
        /*
         * Disable sleep mode.
         */
              
        WRITE_UART_REGISTER (
            uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

        /*
         * Mask RX interrupt.
         */

        WRITE_UART_REGISTER (
            uart, IER, READ_UART_REGISTER (uart, IER) & ~ERBI);

        /*
         * Enable the wake-up interrupt.
         */

        ENABLE_WAKEUP_INTERRUPT (uart);

        deep_sleep = 1;
    }

    return (deep_sleep);
}

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

void
UA_WakeUp (T_tr_UartId uart_id)
{
    t_uart *uart;
       
    uart = &(uart_parameter[uart_id]);

    /*
     * Disable the wake-up interrupt.
     */

    DISABLE_WAKEUP_INTERRUPT (uart);

    /*
     * Unmask RX interrupts.
     */

    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) | ERBI);

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Allow sleep mode.
     */

    WRITE_UART_REGISTER ( 
        uart, IER, READ_UART_REGISTER (uart, IER) | IER_SLEEP);
#endif
}

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

void
UA_InterruptHandler (T_tr_UartId uart_id,
                     SYS_UWORD8 interrupt_status)
{
    t_uart *uart;
   
    uart = &(uart_parameter[uart_id]);

    switch (interrupt_status) {

        case RX_DATA:

            read_rx_fifo (uart);

        break;

        default:

#ifdef UART_RX_BUFFER_DUMP
            uart_rx_buffer_dump.wrong_interrupt_status++;
#endif

            /* No Processing */

        break;
    }
}
