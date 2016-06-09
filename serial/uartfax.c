/*******************************************************************************
 *
 * UARTFAX.C
 *
 * This driver allows to control the UARTs of chipset 1.5 for fax and data
 * services. It performs flow control: RTS/CTS, XON/XOFF.
 *
 * On C & D-Sample, DCD and DTR signals are supported on UART modem only with 2
 * I/Os.
 *
 * On E-Sample, DCD and DTR signals are directly handled by Calypso+.
 *
 * On Calypso, RTS and CTS are supported on both UARTs.
 *
 * On Calypso+, RTS and CTS are supported on UART Modem1 & IrDA. UART Modem2 is
 * not available through DB9 connector on E-Sample.
 *
 * (C) Texas Instruments 1999 - 2003
 *
 ******************************************************************************/

/*
 * E-Sample
 *
 * UART Modem1                  UART Irda
 *
 *     DB9   Calypso+               DB9   Calypso+
 *
 * 1   DCD   DCD      output    1   1, 6 and 4 are connected together on DB9
 * 2   RX    TX       output    2   RX    TX2       output
 * 3   TX    RX       input     3   TX    RX2       input
 * 4   DTR   DSR      input     4   
 * 5   GND                      5   GND
 * 6   NC                       6   
 * 7   RTS   CTS      input     7   RTS   CTS2      input
 * 8   CTS   RTS      output    8   CTS   RTS2      output
 * 9   NC                       9   NC
 *
 */

/*
 * C & D-Sample
 *
 * UART Modem                   UART Irda
 *
 *     DB9   Calypso                DB9   Calypso
 *
 * 1   DCD   I/O 2    output    1   1, 6 and 4 are connected together on DB9
 * 2   RX    TX       output    2   RX    TX2       output
 * 3   TX    RX       input     3   TX    RX2       input
 * 4   DTR   I/O 3    input     4   
 * 5   GND                      5   GND
 * 6   NC                       6   
 * 7   RTS   CTS      input     7   RTS   CTS2      input
 * 8   CTS   RTS      output    8   CTS   RTS2      output
 * 9   NC                       9   NC
 *
 */

/*
 * B-Sample
 *
 * UART Modem                   UART Irda
 *
 *     DB9   Ulysse                 DB9   Ulysse
 *
 * 1   1, 6 and 4 are connected together on DB9 (Modem and Irda)
 * 2   RX    TX                 2   RX    TX
 * 3   TX    RX                 3   TX    RX
 * 4                            4
 * 5   GND                      5   GND
 * 6                            6
 * 7   RTS   CTS                7   7 and 8 are connected together on DB9
 * 8   CTS   RTS                8
 * 9   NC                       9   NC
 *
 */
 
#include "../include/config.h"

#include <string.h>

/*
 * rv_general.h is needed for macros Min & Min3.
 */
#include "../riviera/rv/rv_general.h"
#include "../nucleus/nucleus.h"

#include "../include/sys_types.h"
#include "faxdata.h"
#include "uartfax.h"

/*
 * Needed to reset and restart the sleep timer in case of incoming characters.
 */

#include "serialswitch.h"
extern SYS_BOOL uart_sleep_timer_enabled;

#include "../bsp/mem.h"
#include "../bsp/armio.h"

#if CONFIG_TARGET_GTAMODEM
  #define EXTHOST_WAKEUP_GPIO	1
#else
  #undef  EXTHOST_WAKEUP_GPIO
#endif

/* 
 * Maximal value for an unsigned 32 bits.
 */

#define MAX_UNSIGNED_32 (0xFFFFFFFFU)

#define FIFO_SIZE (64) /* In bytes. */


/*
 * TLR is used to program the RX FIFO trigger levels. FCR[7:4] are  not used.
 * No trigger level used for TX FIFO. THR_IT generated on TX FIFO empty.
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

#define IIR (0x02)  /* Interrupt ident. register - Read only */
#define FCR (0x02)  /* FIFO control register - Write only    */
#define LCR (0x03)  /* Line control register                 */
#define MCR (0x04)  /* Modem control register                */
#define LSR (0x05)  /* Line status register                  */
#define MSR (0x06)  /* Modem status register                 */
#define TCR  (0x06) /* Transmission control register         */
#define TLR  (0x07) /* Trigger level register                */
#define MDR1 (0x08) /* Mode definition register 1            */
#define SCR  (0x10) /* Supplementary Control register        */
#define SSR  (0x11) /* Supplementary Status register         */
#define UASR (0x0E) /* Autobauding Status register           */

/*
 * Supplementary control register.
 */

#define TX_EMPTY_CTL_IT (0x08)
#define RX_CTS_WAKE_UP_ENABLE_BIT (4) /* Use RESET_BIT and SET_BIT macros. */
#define DSR_IT_BIT                (5) /* Use RESET_BIT and SET_BIT macros. */

/*
 * Enhanced feature register.
 */
 
#define ENHANCED_FEATURE_BIT (4) /* Use RESET_BIT and SET_BIT macros. */
#define AUTO_CTS_BIT         (7) /* Transmission is halted when the CTS pin is high (inactive). */

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

#define MDCD (0x01) /* Data Carrier Detect. */
#define MRTS (0x02) /* Request To Send.     */
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

#define BYTE_ERROR (OE | PE | FE | BI)

/*
 * Interrupt identification register.
 * Bit 0 is set to 0 if an IT is pending.
 * Bits 1 and 2 are used to identify the IT.
 */

#define IIR_BITS_USED    (0x07)
#define IT_PENDING       (0x01)
#define RX_DATA          (0x04)
#define TX_EMPTY         (0x02)
#define MODEM_STATUS     (0x00)

/*
 * Modem status register.
 */

#define DELTA_CTS (0x01)
#define DELTA_DSR (0x02)
#define MCTS      (0x10) /* Clear to send       */
#define MDSR      (0x20) /* Data set ready      */

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

/*
 * FIFO control register.
 */

#define FIFO_ENABLE   (0x01)
#define RX_FIFO_RESET (0x02)
#define TX_FIFO_RESET (0x04)

/*
 * These constants define the states of the escape sequence detection.
 */

#define INITIALIZATION       (0)
#define NO_ESCAPE_SEQUENCE   (1)
#define ONE_CHAR_DETECTED    (2)
#define TWO_CHARS_DETECTED   (3)
#define THREE_CHARS_DETECTED (4)

#define CHARACTERS_IN_ESC_SEQ        (3)
#define DEFAULT_ESC_SEQ_CHARACTER    '+'
#define DEFAULT_GUARD_PERIOD      (1000) /* 1 second. */

/*
 * 3 HISR are used to avoid to execute operations from the LISR.
 */
 
#define RX_HISR_PRIORITY      (2)

// NGENGE increase hisr stack otherwise overflows with multiple callbacks
//#define RX_HISR_STACK_SIZE  (512) /* Bytes. */ 
#define RX_HISR_STACK_SIZE  (768) /* Bytes. */ 

#define TX_HISR_PRIORITY      (2)

// NGENGE increase hisr stack otherwise overflows with multiple callbacks
//#define TX_HISR_STACK_SIZE  (512) /* Bytes. */ 
#define TX_HISR_STACK_SIZE  (768) /* Bytes. */ 

#define V24_HISR_PRIORITY     (2)
#define V24_HISR_STACK_SIZE (512) /* Bytes. */ 

/*
 * When the break interrupt indicator (BI) is set in the line status register
 * (LSR), it indicates that the received data input was held in the low state
 * for longer than a full-word transmission time. In the FIFO mode, when a break
 * occurs, only one 0 character is loaded into the FIFO. The next character
 * transfer is enabled after SIN goes to the marking state for at least two RCLK
 * samples and then receives the next valid start bit.
 * This constant defined a defined break length returned by the US_GetLineState
 * function.
 */

#define MINIMAL_BREAK_LENGTH (2)

#define BREAK_HISR_PRIORITY     (2)
#define BREAK_HISR_STACK_SIZE (512) /* Bytes. */

/*
 * These macros allow to read and write a UART register.
 */

#define READ_UART_REGISTER(UART,REG)                                  \
            *((volatile SYS_UWORD8 *) ((UART)->base_address + (REG)))

#define WRITE_UART_REGISTER(UART,REG,VALUE)                           \
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
 * These macros allow to enable or disable the DSR interrupt.
 */

#define ENABLE_DSR_INTERRUPT(UART)   \
	SET_BIT(UART, SCR, DSR_IT_BIT);

#define DISABLE_DSR_INTERRUPT(UART)   \
	RESET_BIT(UART, SCR, DSR_IT_BIT);


/*
 * The transmitter is disabled only when the application disables the driver.
 * To disable the driver, the receiver and the transmitter are disabled by the
 * application. The transmitter is disabled first to test if the driver is
 * disabled.
 */

#define DRIVER_DISABLED(UART) ((UART)->tx_stopped_by_application)

#define DISABLE_DRIVER(UART)                       \
        {                                          \
            (UART)->tx_stopped_by_application = 1; \
            (UART)->rx_stopped_by_application = 1; \
        }

#define ENABLE_DRIVER(UART)                        \
        {                                          \
            (UART)->rx_stopped_by_application = 0; \
            (UART)->tx_stopped_by_application = 0; \
        }

/*
 * Low and high watermarks for the RX buffer. If it is enabled, the flow
 * control is activated or deactivated according to these values.
 * The high watermark value allows to copy an array filled with the RX FIFO
 * into the RX buffer. 
 */

#define RX_LOW_WATERMARK(RX_BUFFER_SIZE)  (FIFO_SIZE)
#define RX_HIGH_WATERMARK(RX_BUFFER_SIZE) ((RX_BUFFER_SIZE) - 2 * FIFO_SIZE)

/* 
 * This macro allows to know if the RX buffer is full. It must be called only
 * from the RX HISR. If it is called from the application, the rx_in and 
 * rx_fifo_in pointers may be updated if a RX interrupt occurs or if the 
 * RX HISR is activated.
 */

#define RX_BUFFER_FULL(UART)                                          \
            (((UART)->rx_in == (UART)->rx_out - 1) ||                 \
             ((UART)->rx_in == (UART)->rx_out + (UART)->buffer_size))

/*
 * This macro allows to know if the TX buffer is empty.
 */

#define TX_BUFFER_EMPTY(UART)                                         \
            ((UART)->tx_in == (UART)->tx_out)

/*
 * This macro is used to convert a time (unit: ms) into a number of TDMA.
 * 1 TDMA = 4.6 ms (23/5).
 */

#define CONVERT_TIME_IN_TDMA(TIME) (((TIME) * 5) / 23)

/*
 * This structure describes an UART compatible with the UART 16750 and
 * contains some fields to manage this UART.
 */

typedef struct s_uart {

    SYS_UWORD32 base_address;

    /*
     * HISR executed from the RX/TX interrupt handler.
     */
     
    NU_HISR rx_hisr_ctrl_block;
    NU_HISR tx_hisr_ctrl_block;
    NU_HISR v24_hisr_ctrl_block;

    char    rx_hisr_stack[RX_HISR_STACK_SIZE];
    char    tx_hisr_stack[TX_HISR_STACK_SIZE];
    char    v24_hisr_stack[V24_HISR_STACK_SIZE];
    
    /*
     * 2 arrays are used to store bytes read in RX FIFO. A UART RX interrupt
     * may occur while executing RX operations in RX HISR. To avoid overwriting
     * the array in which received bytes are stored, a second array is used.
     */
     
    SYS_UWORD8  *rx_buffer_used_by_rx_lisr;
    SYS_UWORD8  *rx_buffer_used_by_rx_hisr;
    SYS_UWORD8  rx_fifo_byte_1[FIFO_SIZE];
    SYS_UWORD8  rx_fifo_byte_2[FIFO_SIZE];
    SYS_UWORD16 bytes_in_rx_buffer_1;
    SYS_UWORD16 bytes_in_rx_buffer_2;
    
    /*
     * RX and TX buffers.
     * One character is not used in each buffer to allow to know if the buffer
     * is empty or not (See macro RX_BUFFER_FULL). If buffers are empty,
     * rx_in = rx_out and tx_in = tx_out. It is impossible to use fields to
     * count the number of bytes in each buffer because these fields may be
     * updated from the application and from the interrupt handlers. That avoids
     * to have conflicts.
     */

    SYS_UWORD16 buffer_size;
    SYS_UWORD16 rx_threshold_level;
    SYS_UWORD16 tx_threshold_level;
    SYS_UWORD8  rx_buffer[FD_MAX_BUFFER_SIZE + 1];
    SYS_UWORD8  tx_buffer[FD_MAX_BUFFER_SIZE + 1];
    SYS_UWORD8  *rx_in;
    SYS_UWORD8  *rx_out;
    SYS_UWORD8  *tx_in;
    SYS_UWORD8  *tx_out;

    /*
     * Escape sequence.
     * the field esc_seq_modified may have 2 values:
     *      - 0: No modification.
     *      - 1: Parameters are in the process of modification: The detection
     *           is stopped.
     */

    NU_TIMER      guard_period_timer_ctrl_block;
    SYS_UWORD8    esc_seq_modified;
    SYS_UWORD8    esc_seq_detection_state;
    SYS_UWORD8    esc_seq_character;
    UNSIGNED      guard_period;
    UNSIGNED      current_time;
    UNSIGNED      previous_time;

    /*
     * Flow control.
     */

    T_flowCtrlMode     flow_control_mode;
    SYS_BOOL           send_xon_xoff;
    SYS_UWORD8         xon_xoff_to_send;
    SYS_UWORD8         xon_character;
    SYS_UWORD8         xoff_character;
    SYS_BOOL           rx_stopped_by_application;
    SYS_BOOL           rx_stopped_by_driver;
    SYS_BOOL           rx_stopped_by_lisr;
    SYS_BOOL           tx_stopped_by_application;
    SYS_BOOL           tx_stopped_by_driver;
    /* SYS_BOOL           tx_stopped_by_lisr;*/

    /*
     * Break.
     */

    SYS_BOOL     break_received;
    SYS_BOOL     break_to_send;
    SYS_BOOL     break_in_progress;
    NU_HISR      break_hisr_ctrl_block;
    char         break_hisr_stack[BREAK_HISR_STACK_SIZE];
    NU_TIMER     break_timer_ctrl_block;
    UNSIGNED     baudrate;
    UNSIGNED     autobauding;	
    UNSIGNED     bits_per_char; /* Including start, stop and parity bits. */
    UNSIGNED     break_length;  /* In bytes. */
    UNSIGNED     time_without_character;

    /*
     * Callback (UAF_ReadData and UAF_WriteData).
     * rd: read, wr: write.
     */

    SYS_BOOL   esc_seq_received;
    SYS_UWORD8 rts_level; /* RTS on RS232 side, CTS on chipset side.
                             1: The RS232 line is deactivated (low). */

#if TARGET_HAS_DTR_INPUT
    SYS_UWORD8 dtr_level; /* Controlled with an I/O on C & D-Sample and
                             handled by Calypso+ on E-Sample.
                             1: The RS232 line is deactivated (low). */
    /*
     * When the DTR interrupt is detected the user's Rx callback function must
     * be called but if the Rx FIFO is not empty the Rx HISR must be activated
     * to read the bytes received in the Rx FIFO and to put them into the Rx
     * buffer before to call the user's Rx callback function.
     * If the Rx HISR is activated due to a Rx interrupt the user's Rx callback
     * function will be called if conditions to call it are fulfilled. If it is
     * activated due to the DTR interrupt the user's Rx callback function must
     * be called without any conditions.
     * Because the Rx HISR may have been activated but not executed before the
     * DTR interrupt we must be sure that the user's Rx callback function will
     * be called for each Rx HISR activation. Call is done for Rx HISR activated
     * on Rx interrupt if conditions are fulfilled.
     * A circular buffer of 2 elements is used to memorize the source of
     * interrupt. Before the activation of the Rx HISR, the source of interrupt
     * is memorized into this array. When the code of the Rx HISR is executed
     * the user's Rx callback function is called if the source of interrupt was
     * the DTR interrupt regardless of the other conditions.
     * The level of DTR is saved to provide the level detected on Rx interrupt
     * or DTR interrupt in the 'state' parameter of the user's Rx callback
     * function.
     */                             

    SYS_BOOL   dtr_change_detected[2];
    SYS_UWORD8 dtr_level_saved[2];
    SYS_UWORD8 index_it;
    SYS_UWORD8 index_hisr;
#endif /* BOARD 8 or 9 or 40 or 41 or CHIPSET 12 */                            
    
    SYS_BOOL         reading_suspended;
    SYS_BOOL         writing_suspended;
    SYS_BOOL         rd_call_from_hisr_in_progress;
    SYS_BOOL         wr_call_from_hisr_in_progress;
    T_reInstMode     rd_call_setup;
    T_reInstMode     wr_call_setup;
    SYS_UWORD8       *rd_address[2];
    SYS_UWORD8       *wr_address[2];
    SYS_UWORD16      rd_size_before_call[2];
    SYS_UWORD16      rd_size_after_call[2];
    SYS_UWORD16      wr_size_before_call[2];
    SYS_UWORD16      wr_size_after_call[2];

    void (*readOutFunc) (SYS_BOOL cldFromIrq,
                         T_reInstMode *reInstall,
                         SYS_UWORD8 nsource,
                         SYS_UWORD8 *source[],
                         SYS_UWORD16 size[],
                         SYS_UWORD32 state);

    void (*writeInFunc) (SYS_BOOL cldFromIrq,
                         T_reInstMode *reInstall,
                         SYS_UWORD8 ndest,
                         SYS_UWORD8 *dest[],
                         SYS_UWORD16 size[]);

    /*
     * These fields are used to store the state defined in UAF_GetLineState.The
     * first field is used when UAF_GetLineState and UAF_ReadData are not called.
     * When one of these functions is called the second field is used. That
     * avoids to lose events when UAF_GetLineState or UAF_ReadData resets the
     * first field.
     */

    SYS_UWORD32 state_1;
    SYS_UWORD32 state_2;
    SYS_UWORD32 *state;

    /*
     * Errors counters.
     */

    SYS_UWORD32 framing_error;
    SYS_UWORD32 parity_error;
    SYS_UWORD32 overrun_error;
    SYS_UWORD32 spurious_interrupts;

    SYS_UWORD16 max_rx_fifo_level;

} t_uart;


static t_uart uart_parameters;


static const SYS_UWORD32 base_address[NUMBER_OF_FD_UART] =
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
      0, /*   Auto baud:                */
     81, /*     75 baud.                */
     40, /*    150 baud.                */
    148, /*    300 baud.                */
     74, /*    600 baud.                */
    165, /*   1200 baud.                */
     83, /*   2400 baud.                */
    169, /*   4800 baud.                */
    113, /*   7200 baud.                */
     84, /*   9600 baud.                */
     56, /*  14400 baud.                */
     42, /*  19200 baud.                */
     28, /*  28800 baud.                */
     24, /*  33900 baud: not supported. */
     21, /*  38400 baud.                */
     14, /*  57600 baud.                */
      7, /* 115200 baud.                */
      0, /* 203125 baud: not supported. */
      0, /* 406250 baud: not supported. */
      0  /* 812500 baud: not supported. */
};

static const SYS_UWORD8 dlh[] = 
{
    0, /*   Auto baud:               */
   42, /*     75 baud.                */
   21, /*    150 baud.                */
   10, /*    300 baud.                */
    5, /*    600 baud.                */
    2, /*   1200 baud.                */
    1, /*   2400 baud.                */
    0, /*   4800 baud.                */
    0, /*   7200 baud.                */
    0, /*   9600 baud.                */
    0, /*  14400 baud.                */
    0, /*  19200 baud.                */
    0, /*  28800 baud.                */
    0, /*  33900 baud: not supported. */
    0, /*  38400 baud.                */
    0, /*  57600 baud.                */
    0, /* 115200 baud.                */
    0, /* 203125 baud: not supported. */
    0, /* 406250 baud: not supported. */
    0  /* 812500 baud: not supported. */
};

static const UNSIGNED baudrate_value[] =
{
         1,
        75,
       150,
       300,
       600,
      1200,
      2400,
      4800,
      7200,
      9600,
     14400,
     19200,
     28800,
         0, /* Not supported. */
     38400,
     57600,
    115200,
         0, /* Not supported. */
         0, /* Not supported. */
         0  /* Not supported. */
};


 

/*******************************************************************************
 *
 *                          get_bytes_in_rx_buffer
 * 
 * Purpose  : Gets the number of bytes in the RX buffer.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : The number of bytes in the RX buffer.
 *
 ******************************************************************************/

static SYS_UWORD16
get_bytes_in_rx_buffer (t_uart *uart)
{
    SYS_UWORD16 bytes_in_rx_buffer;
    volatile SYS_UWORD8 *rx_in;

    rx_in = uart->rx_in;

    if (uart->rx_out <= rx_in)
        bytes_in_rx_buffer = (SYS_UWORD16) (rx_in - uart->rx_out);
    else
        bytes_in_rx_buffer =
            (SYS_UWORD16) (rx_in - uart->rx_out + uart->buffer_size + 1);

    return (bytes_in_rx_buffer);
}

/*******************************************************************************
 *
 *                          get_bytes_in_tx_buffer
 * 
 * Purpose  : Gets the number of bytes in the TX buffer.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : The number of bytes in the TX buffer.
 *
 ******************************************************************************/

static SYS_UWORD16
get_bytes_in_tx_buffer (t_uart *uart)
{
    SYS_UWORD16 bytes_in_tx_buffer;
    volatile SYS_UWORD8 *tx_out;

    tx_out = uart->tx_out;

    if (tx_out <= uart->tx_in)
        bytes_in_tx_buffer = (SYS_UWORD16) (uart->tx_in - tx_out);
    else
        bytes_in_tx_buffer =
            (SYS_UWORD16) (uart->tx_in - tx_out + uart->buffer_size + 1);

    return (bytes_in_tx_buffer);
}

/*******************************************************************************
 *
 *                              compute_break_time
 * 
 * Purpose  : Computes a number of TDMA from 3 parameters:
 *              - baudrate,
 *              - bits per character including start bit, stop bits and parity,
 *              - number of characters.
 *            Due to the TDMA value (4.6 ms), a minimal value is sent: 2 TDMA.
 *
 * Arguments: In : baudrate
 *                 bits_per_char
 *                 number_of_chars
 *            Out: none
 *
 * Returns  : The number of TDMA.
 *
 ******************************************************************************/

static UNSIGNED
compute_break_time (UNSIGNED baudrate,
                    UNSIGNED bits_per_char,
                    UNSIGNED number_of_chars)
{
    UNSIGNED number_of_tdma;

    number_of_tdma = CONVERT_TIME_IN_TDMA (
                         1000 * bits_per_char * number_of_chars / baudrate);

    if (number_of_tdma == 0)
        number_of_tdma = 1;

    number_of_tdma++;

    return (number_of_tdma);
}

/*******************************************************************************
 *
 *                          update_reading_callback
 * 
 * Purpose  : Updates the sizes array and the addresses array and get and builds
 *            the state parameter defined in UAF_GetLineState to call the
 *            readOutFunc function.
 *
 * Arguments: In : uart       : Pointer on the UART structure.
 *                 call_source: 0: application, 1: HISR (Rx or V24), 3: Rx HISR
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static void
update_reading_callback (t_uart *uart,
                         SYS_BOOL call_source)
{
    SYS_UWORD32  state;
    SYS_UWORD8  dtr_level;
    SYS_UWORD8  fragments_number;
    SYS_UWORD16 bytes_in_rx_buffer;
    volatile SYS_UWORD8 *rx_in;

    /*
     * Update the sizes array and the addresses array.
     * A copy of rx_in is used because it may be updated by the interrupt
     * handler if this function is called from the application.
     */

    rx_in = uart->rx_in;
    
    if (uart->rx_out < rx_in) {

        fragments_number = 1;

        uart->rd_address[0] = uart->rx_out;
        uart->rd_size_before_call[0] = (SYS_UWORD16) (rx_in - uart->rx_out);
        uart->rd_size_after_call[0] = uart->rd_size_before_call[0];

        uart->rd_size_before_call[1] = 0;
        uart->rd_size_after_call[1] = 0;

        bytes_in_rx_buffer = uart->rd_size_before_call[0];

    } else if (rx_in == uart->rx_out) { /* RX buffer empty. */
        
        fragments_number = 1;

        uart->rd_address[0] = uart->rx_out;
        uart->rd_size_before_call[0] = 0;
        uart->rd_size_after_call[0] = 0;

        uart->rd_size_before_call[1] = 0;
        uart->rd_size_after_call[1] = 0;

        bytes_in_rx_buffer = 0;
                    
    } else {
    
        fragments_number = 2;

        uart->rd_address[0] = uart->rx_out;
        uart->rd_size_before_call[0] =
            uart->buffer_size + 1 - (SYS_UWORD16) (uart->rx_out -
                                               &(uart->rx_buffer[0]));
        uart->rd_size_after_call[0] = uart->rd_size_before_call[0];

        uart->rd_address[1] = &(uart->rx_buffer[0]);
        uart->rd_size_before_call[1] = (SYS_UWORD16) (rx_in -
                                                  &(uart->rx_buffer[0]));
        uart->rd_size_after_call[1] = uart->rd_size_before_call[1];

        bytes_in_rx_buffer =
            uart->rd_size_before_call[0] + uart->rd_size_before_call[1];

        if (!uart->rd_size_before_call[1])
            fragments_number = 1;
    }

    /*
     * Build the state parameter defined in UAF_GetLineState.
     * The field state_2 is used when state_1 is set to 0 to avoid to
     * lose events detected in the RX interrupt handler.
     */

#if TARGET_HAS_DTR_INPUT
    if (call_source == 3) /* Call from Rx HISR */
        dtr_level = uart->dtr_level_saved[uart->index_hisr];
    else
        dtr_level = uart->dtr_level;
#endif

    state = uart->state_2;
    uart->state_2 = 0;
    uart->state = &(uart->state_2);

    state |= uart->state_1;
    uart->state_1 = 0;
    uart->state = &(uart->state_1);

    state |= ((((SYS_UWORD32) uart->rts_level) << RTS) |

#if TARGET_HAS_DTR_INPUT
              (((SYS_UWORD32) dtr_level) << DTR) |
#endif

              (((SYS_UWORD32) (uart->tx_stopped_by_application  |
                         uart->tx_stopped_by_driver)) << TXSTP) |

              (((SYS_UWORD32) (uart->rx_stopped_by_application  |
                         uart->rx_stopped_by_driver)) << RXSTP) |

              (((SYS_UWORD32) (uart->buffer_size - bytes_in_rx_buffer)) << RXBLEV));

    /*
     * Fields SA, SB and X are set according to the flow control:
     *
     *       None    RTS/CTS    XON/XOFF
     * SA    DTR     DTR        DTR
     * SB    RTS     0          RTS
     * X     0       RTS        XON:0 XOFF:1 (transmitter)
     *
     * DTR is supported on C, D & E-Sample.
     */

#if TARGET_HAS_DTR_INPUT
    state |= (((SYS_UWORD32) uart->dtr_level) << SA);
#endif

    if (uart->flow_control_mode != fc_rts)
        state |= (((SYS_UWORD32) uart->rts_level) << SB);

    if (uart->flow_control_mode == fc_rts)
        state |= (((SYS_UWORD32) uart->rts_level) << X);
            
    else if ((uart->flow_control_mode == fc_xoff) &&
             (uart->tx_stopped_by_application ||
              uart->tx_stopped_by_driver))
        state |= (1 << X);

    /*
     * Call the readOutFunc function with these parameters.
     */

    uart->rd_call_setup = rm_notDefined;

    (*(uart->readOutFunc)) (call_source & 0x01, /* From HISR or application */
                            &(uart->rd_call_setup),
                            fragments_number,
                            &(uart->rd_address[0]),
                            &(uart->rd_size_after_call[0]),
                            state);
}

/*******************************************************************************
 *
 *                          update_writing_callback
 * 
 * Purpose  : Updates the sizes array and the addresses array to call the
 *            writeInFunc function.
 *
 * Arguments: In : uart      : Pointer on the UART structure.
 *                 call_source: 0: application, 1: HISR
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static void
update_writing_callback (t_uart *uart,
                         SYS_BOOL call_source)
{
    SYS_UWORD8 fragments_number;
    volatile SYS_UWORD8 *tx_out;

    /*
     * Update the array of sizes and the array of addresses.
     * A copy of tx_out is used because it may be updated by the interrupt
     * handler if this function is called from the application.
     */

    tx_out = uart->tx_out;
    
    if (uart->tx_in < tx_out) {

        fragments_number = 1;

        uart->wr_address[0] = uart->tx_in;
        uart->wr_size_before_call[0] =
            (SYS_UWORD16) (tx_out - uart->tx_in - 1);
        uart->wr_size_after_call[0] = uart->wr_size_before_call[0];

        uart->wr_size_before_call[1] = 0;
        uart->wr_size_after_call[1] = 0;
                                    
    } else if (tx_out == &(uart->tx_buffer[0])) {

        fragments_number = 1;
        
        uart->wr_address[0] = uart->tx_in;
        uart->wr_size_before_call[0] =
            uart->buffer_size - 
            (SYS_UWORD16) (uart->tx_in - &(uart->tx_buffer[0]));
        uart->wr_size_after_call[0] = uart->wr_size_before_call[0];

        uart->wr_size_before_call[1] = 0;
        uart->wr_size_after_call[1] = 0;

    } else {
    
        fragments_number = 2;

        uart->wr_address[0] = uart->tx_in;
        uart->wr_size_before_call[0] =
            uart->buffer_size + 1 - 
            (SYS_UWORD16) (uart->tx_in - &(uart->tx_buffer[0]));
        uart->wr_size_after_call[0] = uart->wr_size_before_call[0];

        uart->wr_address[1] = &(uart->tx_buffer[0]);
        uart->wr_size_before_call[1] =
            (SYS_UWORD16) (tx_out - &(uart->tx_buffer[0]) - 1);
        uart->wr_size_after_call[1] = uart->wr_size_before_call[1];

        if (!uart->wr_size_before_call[1])
            fragments_number = 1;
    }

    /*
     * Call the writeInFunc function with these parameters;
     */

    uart->wr_call_setup = rm_notDefined;

    (*(uart->writeInFunc)) (call_source,
                            &(uart->wr_call_setup),
                            fragments_number,
                            &(uart->wr_address[0]),
                            &(uart->wr_size_after_call[0]));
}

/*******************************************************************************
 *
 *                                  stop_break
 * 
 * Purpose  : The timer is activated to expire when a time corresponding to the
 *            sending time of 2 characters at least has elapsed. After a break,
 *            no character may be sent during this period.
 *
 * Arguments: In : id: parameter not used.
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static VOID
stop_break (UNSIGNED id)
{
    t_uart *uart;

    uart = &uart_parameters;

    uart->break_to_send = 0;
    uart->break_in_progress = 0;

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Disable sleep mode.
     */
          
    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

    /*
     * Unmask Tx interrupt.
     */

    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) | ETBEI);
}

/*******************************************************************************
 *
 *                          hisr_start_break
 * 
 * Purpose  : Enables the timer used to control the time without character.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static VOID
hisr_start_break (VOID)
{
    t_uart *uart;

    uart = &uart_parameters;

    (void) NU_Control_Timer (&(uart->break_timer_ctrl_block),
                             NU_DISABLE_TIMER);

    (void) NU_Reset_Timer (&(uart->break_timer_ctrl_block),
                           stop_break,
                           uart->time_without_character,
                           0, /* The timer expires once. */
                           NU_DISABLE_TIMER);

    (void) NU_Control_Timer (&(uart->break_timer_ctrl_block),
                             NU_ENABLE_TIMER);
}

/*******************************************************************************
 *
 *                              stop_receiver
 * 
 * Purpose  : Activates DTR or RTS or sends XOFF. 
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static void
stop_receiver (t_uart *uart)
{
#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Disable sleep mode.
     */
          
    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

    switch (uart->flow_control_mode) {

    case fc_rts:

        /*
         * CTS (RTS on UART side) is deactivated (high).
         */

        WRITE_UART_REGISTER (
            uart, MCR, READ_UART_REGISTER (uart, MCR) & ~MRTS);
        break;

    case fc_xoff:

        uart->xon_xoff_to_send = uart->xoff_character;
        uart->send_xon_xoff = 1;

        /*
         * Unmask Tx interrupt.
         */
          
        WRITE_UART_REGISTER (
            uart, IER, READ_UART_REGISTER (uart, IER) | ETBEI);
        break;
    }
}

/*******************************************************************************
 *
 *                              start_receiver
 * 
 * Purpose  : Deactivates DTR or RTS or sends XON.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static void
start_receiver (t_uart *uart)
{
#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Disable sleep mode.
     */
          
    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

    switch (uart->flow_control_mode) {

    case fc_rts:

        /*
         * CTS (RTS on UART side) is activated (low).
         */

        WRITE_UART_REGISTER (
            uart, MCR, READ_UART_REGISTER (uart, MCR) | MRTS);
        break;

    case fc_xoff:

        uart->xon_xoff_to_send = uart->xon_character;
        uart->send_xon_xoff = 1;

        /*
         * Unmask Tx interrupt.
         */
          
        WRITE_UART_REGISTER (
            uart, IER, READ_UART_REGISTER (uart, IER) | ETBEI);
        break;
    }
}

/*******************************************************************************
 *
 *                          add_esc_seq_char_in_rx_buffer
 * 
 * Purpose  : Writes one escape sequence character in the RX buffer.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static void
add_esc_seq_char_in_rx_buffer (t_uart *uart)
{
    /*
     * IF the RX buffer is not full, write an escape sequence character in the
     * RX buffer and check wrap-around.
     */

    if (!RX_BUFFER_FULL (uart)) {

        *(uart->rx_in++) = uart->esc_seq_character;

        if (uart->rx_in == &(uart->rx_buffer[0]) + uart->buffer_size + 1)
            uart->rx_in = &(uart->rx_buffer[0]);
    }
}

/*******************************************************************************
 *
 *                      analyze_guard_period_timer_expiration
 * 
 * Purpose  : According to the state of the escape sequence detection, 1 or 2
 *            escape sequence characters may be written into the TX buffer or
 *            the escape sequence is declared as detected.
 *            If 1 or 2 escape sequence characters have been detected the
 *            guard period must not expire.
 *            If 3 characters have been detected the escape sequence must
 *            expire.
 *
 * Arguments: In : id: parameter not used.
 *            Out: none
 *
 * Returns  : none 
 *
 ******************************************************************************/

static VOID
analyze_guard_period_timer_expiration (UNSIGNED id)
{
    t_uart *uart;
    SYS_UWORD16 bytes_in_rx_buffer;

    uart = &uart_parameters;

    switch (uart->esc_seq_detection_state) {
    
    case ONE_CHAR_DETECTED:
    
        /*
         * 1 escape sequence character has been detected. The guard period has
         * ellapsed. This character is written into the TX buffer.
         */
        
        add_esc_seq_char_in_rx_buffer (uart);
        break;
        
    case TWO_CHARS_DETECTED:
    
        /*
         * 2 escape sequence characters have been detected. The guard period has
         * ellapsed. These characters are written into the TX buffer.
         */
        
        add_esc_seq_char_in_rx_buffer (uart);
        add_esc_seq_char_in_rx_buffer (uart);

        break;
        
    case THREE_CHARS_DETECTED:

        /*
         * 3 escape sequence characters have been detected and the guard period
         * has ellapsed. The escape sequence is detected.
         */    
         
        uart->esc_seq_received = 1;
        *(uart->state) |= (1 << ESC);
        
        break;
    }
    
    uart->esc_seq_detection_state = NO_ESCAPE_SEQUENCE;
    
    /*
     * If the high watermark is reached, RTS is activated or XOFF is sent
     * according to the flow control mode.
     */

    bytes_in_rx_buffer = get_bytes_in_rx_buffer (uart);

    if ((uart->flow_control_mode != fc_none) &&
        (bytes_in_rx_buffer >= RX_HIGH_WATERMARK (uart->buffer_size))) {

        /*
         * Check if receipt must be stopped.
         */

        if (!uart->rx_stopped_by_driver) {

            uart->rx_stopped_by_driver = 1;
            if (!uart->rx_stopped_by_application)
                stop_receiver (uart);
        }
    }

    /*
     * If a reading was suspended or if the callback function is installed,
     * it is called if one of these conditions is fulfiled:
     *      - the RX threshold level is reached,
     *      - a break has been detected,
     *      - an escape sequence has been detected,
     */

    if ((!uart->rd_call_from_hisr_in_progress) &&
        (uart->reading_suspended ||
         (uart->rd_call_setup == rm_reInstall))) {

        if ((bytes_in_rx_buffer >= uart->rx_threshold_level) ||
            uart->break_received ||
            uart->esc_seq_received) {

            uart->rd_call_from_hisr_in_progress = 1;
            update_reading_callback (uart, 1); /* 1: call from HISR. */

            uart->reading_suspended = 0;
            uart->break_received = 0;
            uart->esc_seq_received = 0;
        }
    }
}

/*******************************************************************************
 *
 *                          stop_guard_period_timer
 * 
 * Purpose  : Stops the timer used to detect the guard period expiration.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : none 
 *
 ******************************************************************************/

static void
stop_guard_period_timer (t_uart *uart)
{
    (void) NU_Control_Timer (&(uart->guard_period_timer_ctrl_block),
                             NU_DISABLE_TIMER);
}

/*******************************************************************************
 *
 *                          start_guard_period_timer
 * 
 * Purpose  : Starts a timer which expires if the guard period has ellapsed.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : none 
 *
 ******************************************************************************/

static void
start_guard_period_timer (t_uart *uart)
{
    (void) NU_Control_Timer (&(uart->guard_period_timer_ctrl_block),
                             NU_DISABLE_TIMER);

    (void) NU_Reset_Timer (&(uart->guard_period_timer_ctrl_block),
                           analyze_guard_period_timer_expiration,
                           uart->guard_period,
                           0, /* The timer expires once. */
                           NU_DISABLE_TIMER);

    (void) NU_Control_Timer (&(uart->guard_period_timer_ctrl_block),
                             NU_ENABLE_TIMER);
}

/*******************************************************************************
 *
 *                          detect_escape_sequence
 * 
 * Purpose  : The state machine used to detect an escape sequence is updated
 *            according to the array of bytes to analyse. If the state machine
 *            goes to the initial state due to a break in the sequence
 *            detection, the previous characters are put into the RX buffer.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : 0: Break in detection or a sequence has been detected. 
 *            1: A sequence may be detected.
 *
 ******************************************************************************/

static int
detect_escape_sequence (t_uart *uart)
{
    int      detection_result;
    SYS_UWORD8    *rx_fifo_byte;
    SYS_UWORD16   bytes_in_rx_buffer;
    UNSIGNED elapsed_time;

    detection_result = 0;

    rx_fifo_byte = uart->rx_buffer_used_by_rx_hisr;
    if (rx_fifo_byte == &(uart->rx_fifo_byte_1[0]))
        bytes_in_rx_buffer = uart->bytes_in_rx_buffer_1;
    else
        bytes_in_rx_buffer = uart->bytes_in_rx_buffer_2;

    if (uart->current_time > uart->previous_time)
        elapsed_time = uart->current_time - uart->previous_time;
    else
        elapsed_time =
            MAX_UNSIGNED_32 - uart->previous_time + uart->current_time;

    switch (uart->esc_seq_detection_state) {

    case INITIALIZATION:

        /*
         * It is the first character received. It may be the first character
         * of an escape sequence. The elapsed_time variable is set to the
         * guard period value to consider this character as the first character
         * of an escape sequence.
         */

        if (!uart->esc_seq_modified) {

            elapsed_time = uart->guard_period;
            uart->esc_seq_detection_state = NO_ESCAPE_SEQUENCE;
        }

        /* No break! */

    case NO_ESCAPE_SEQUENCE:

        /*
         * To go to the next state (one, two or three characters detected):
         *      - a guard period must have elapsed since the last receipt,
         *      - the characters must belong to the escape sequence.
         */

        if ((elapsed_time >= uart->guard_period) &&
            (!uart->esc_seq_modified)) {

            switch (bytes_in_rx_buffer) {

            case 1:

                if (*rx_fifo_byte++ == uart->esc_seq_character) {
                
                    uart->esc_seq_detection_state = ONE_CHAR_DETECTED;
                    start_guard_period_timer (uart);
                    detection_result = 1;
                }

                break;

            case 2:

                if ((*rx_fifo_byte++ == uart->esc_seq_character) &&
                    (*rx_fifo_byte++ == uart->esc_seq_character)) {
                    
                    uart->esc_seq_detection_state = TWO_CHARS_DETECTED;
                    start_guard_period_timer (uart);
                    detection_result = 1;
                }
                                    
                break;

            case 3:

                if ((*rx_fifo_byte++ == uart->esc_seq_character) &&
                    (*rx_fifo_byte++ == uart->esc_seq_character) &&
                    (*rx_fifo_byte++ == uart->esc_seq_character)) {
                        
                    uart->esc_seq_detection_state = THREE_CHARS_DETECTED;
                    start_guard_period_timer (uart);
                    detection_result = 1;
                }
                    
                break;

            default:
            
                /*
                 * No action.
                 */

                break;
            }
        }

        uart->previous_time = uart->current_time;

        break;

    case ONE_CHAR_DETECTED:

        /*
         * To go to the next state (two or three characters detected):
         *      - the difference between the current time and the previous time
         *        must be less than the guard period,
         *      - the characters must belong to the escape sequence.
         * Otherwise, an escape sequence character is written in the RX buffer.
         */

        if (!uart->esc_seq_modified) {

            switch (bytes_in_rx_buffer) {

            case 1:

                if (*rx_fifo_byte++ == uart->esc_seq_character) {
                
                    uart->esc_seq_detection_state = TWO_CHARS_DETECTED;
                    detection_result = 1;                
                }

                break;

            case 2:

                if ((*rx_fifo_byte++ == uart->esc_seq_character) &&
                    (*rx_fifo_byte++ == uart->esc_seq_character)) {
                
                    start_guard_period_timer (uart); /* Reset the timer. */
                       
                    uart->esc_seq_detection_state = THREE_CHARS_DETECTED;
                    detection_result = 1;                    
                }
                    
                break;

            default:

                /*
                 * No action.
                 */

                break;
            }
        }

        if (!detection_result) {

            add_esc_seq_char_in_rx_buffer (uart);

            uart->previous_time = uart->current_time;
            uart->esc_seq_detection_state = NO_ESCAPE_SEQUENCE;
        }

        break;

    case TWO_CHARS_DETECTED:

        /*
         * To go to the next state (three chars detected):
         *      - the difference between the current time and the previous time
         *        must be less than the guard period,
         *      - the character must belong to the escape sequence.
         * Otherwise, 2 escape sequence characters are written in the RX buffer.
         */

        if (!uart->esc_seq_modified) {

            switch (bytes_in_rx_buffer) {

            case 1:

                if (*rx_fifo_byte++ == uart->esc_seq_character) {

                    start_guard_period_timer (uart); /* Reset the timer. */
                    
                    uart->esc_seq_detection_state = THREE_CHARS_DETECTED;
                    detection_result = 1;
                }
                
                break;

            default:

                /*
                 * No action.
                 */

                break;
            }
        }

        if (!detection_result) {

            add_esc_seq_char_in_rx_buffer (uart);
            add_esc_seq_char_in_rx_buffer (uart);

            uart->previous_time = uart->current_time;
            uart->esc_seq_detection_state = NO_ESCAPE_SEQUENCE;
        }

        break;

    case THREE_CHARS_DETECTED:

        /*
         * An escape sequence is detected if a guard period has elapsed since
         * the last receipt. Otherwise, 3 escape sequence characters are
         * written in the RX buffer.
         */

        stop_guard_period_timer (uart);

        add_esc_seq_char_in_rx_buffer (uart);
        add_esc_seq_char_in_rx_buffer (uart);
        add_esc_seq_char_in_rx_buffer (uart);

        uart->previous_time = uart->current_time;
        uart->esc_seq_detection_state = NO_ESCAPE_SEQUENCE;

        break;
    }

    return (detection_result);
}

/*******************************************************************************
 *
 *                              send_break
 * 
 * Purpose  : This function may only called if the TX FIFO is empty.
 *            Null characters are written in the TX FIFO. The number of bytes to
 *            write has been defined with UAF_SetLineState. Enables the break
 *            condition.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : Number of bytes sent.
 *
 ******************************************************************************/

static SYS_UWORD16
send_break (t_uart *uart)
{
    SYS_UWORD16 bytes_in_tx_fifo;

    bytes_in_tx_fifo = 0;
    uart->break_in_progress = 1;

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Disable sleep mode.
     */

    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

    WRITE_UART_REGISTER (
        uart, LCR, READ_UART_REGISTER (uart, LCR) | BREAK_CONTROL);
      
#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Re-enable sleep mode.
     */

/* BELOW LINES WERE COMMENTED TO DISABLE SLEEP MODE IN DRIVER */
/*
    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) | IER_SLEEP);

        */
#endif

    while (uart->break_length) {

        WRITE_UART_REGISTER (uart, THR, 0x00);
        uart->break_length--;
        bytes_in_tx_fifo++;
    }

    return (bytes_in_tx_fifo);
}

/*******************************************************************************
 *
 *                              build_rx_fifo_array
 * 
 * Purpose  : Reads the RX FIFO to build an array with bytes read.
 *            A byte is written in this array if no error is detected. 
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : The number of bytes in RX FIFO.
 *
 ******************************************************************************/

static SYS_UWORD16
build_rx_fifo_array (t_uart *uart)
{
    SYS_UWORD8  status;
    SYS_UWORD8  *first_byte;
    SYS_UWORD8  *current_byte;
    SYS_UWORD16 *bytes_in_rx_buffer;
    SYS_UWORD16 bytes_received;
    SYS_UWORD8 cbyte;

    volatile int x;

    x = 1;

    bytes_received = 0;
    
    
    /*
     * Switch to the other buffer.
     */
     
    first_byte = uart->rx_buffer_used_by_rx_lisr;
    if (first_byte == &(uart->rx_fifo_byte_1[0])) {
    
        first_byte = &(uart->rx_fifo_byte_2[0]);
        bytes_in_rx_buffer = &(uart->bytes_in_rx_buffer_2);
    
    } else {
    
        first_byte = &(uart->rx_fifo_byte_1[0]);
        bytes_in_rx_buffer = &(uart->bytes_in_rx_buffer_1);
    }

    current_byte = first_byte;

    if (*bytes_in_rx_buffer) {

	/* The Rx buffer is not empty and is being used by HISR ! */
	/* Hence stop the flow control */
	stop_receiver (uart); 

        /*
         * Reset LCR[7] (DLAB) to have access to the RBR, THR and IER registers.
         */
        WRITE_UART_REGISTER (uart, LCR, READ_UART_REGISTER (uart, LCR) & ~DLAB);

        /* Mask The Rx  and interrupt */
       
        WRITE_UART_REGISTER (
            uart, IER, READ_UART_REGISTER (uart, IER) &
                       ~(ERBI | EDSSI));
       
        uart->rx_stopped_by_lisr = 1;
      	return (bytes_received);

    }

    uart->rx_buffer_used_by_rx_lisr = first_byte;
    
    status = READ_UART_REGISTER (uart, LSR);

    /*
     * Build an array with the bytes contained in the RX FIFO.
     */

    while (status & DR) { /* While RX FIFO is not empty... */

        *current_byte = READ_UART_REGISTER (uart, RHR);

        /*
         * Check if a parity error or a framing error is associated with the
         * received data. If there is an error the byte is not copied into the
         * bytes array.
         */

        if (status & BYTE_ERROR) {

            if (status & OE)
                uart->overrun_error++;

            if (status & PE)
                uart->parity_error++;

            if (status & FE)
                uart->framing_error++;

            /*
             * Check break detection.
             */

            if (status & BI) {
            
                uart->break_received = 1;
                *(uart->state) |=
                    ((1 << BRK) | (MINIMAL_BREAK_LENGTH << BRKLEN));
            }

        } else /* No error */ 
            current_byte++;

        status = READ_UART_REGISTER (uart, LSR);
    }

    bytes_received = (SYS_UWORD16) (current_byte - first_byte);
    *bytes_in_rx_buffer = bytes_received;

    /*
     * Re-switch to the other buffer if no valid character has been received.
     */

    if (!bytes_received) {
         
        if (uart->rx_buffer_used_by_rx_lisr == &(uart->rx_fifo_byte_1[0]))
            uart->rx_buffer_used_by_rx_lisr = &(uart->rx_fifo_byte_2[0]);
        
        else
            uart->rx_buffer_used_by_rx_lisr = &(uart->rx_fifo_byte_1[0]);
    }

    if (bytes_received > uart->max_rx_fifo_level)
        uart->max_rx_fifo_level = bytes_received;
        
    return (bytes_received);
}

/*******************************************************************************
 *
 *                              empty_rx_fifo
 * 
 * Purpose  : Read the RX FIFO.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static void
empty_rx_fifo (t_uart *uart)
{
    SYS_UWORD16 bytes_in_rx_fifo;
    volatile SYS_UWORD8 dummy_byte;

    bytes_in_rx_fifo = 0;

    while (READ_UART_REGISTER (uart, LSR) & DR) {

        dummy_byte = READ_UART_REGISTER (uart, RHR);
        bytes_in_rx_fifo++;
    }

    if (bytes_in_rx_fifo > uart->max_rx_fifo_level)
        uart->max_rx_fifo_level = bytes_in_rx_fifo;
}

/*******************************************************************************
 *
 *                          hisr_execute_rx_operations
 * 
 * Purpose  : If an escape sequence is detected or if a break in the detection
 *            has occured RX FIFO bytes are written in the RX buffer. 
 *            If the software flow control is used bytes are analyzed to know
 *            if a XON or a XOFF character is received to stop or start the
 *            transmitter. 
 *            If a flow control is used and if the high watermark of the RX
 *            buffer is reached the receiver is stopped.
 *            If the RX threshold level is reached the callback mechanism is
 *            activated.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static VOID
hisr_execute_rx_operations (VOID)
{
    SYS_UWORD16 bytes_free_in_rx_buffer;
    SYS_UWORD16 wrap_around_counter;
    SYS_UWORD16 bytes_in_rx_buffer;
    SYS_UWORD16 bytes_read;
    SYS_UWORD16 bytes_to_copy;
    SYS_UWORD8  *current_byte;
    SYS_UWORD8  xon_xoff_detected;
    t_uart *uart;

    uart = &uart_parameters;
    
    /*
     * Since new characters have been received, the sleep timer is reset then
     * restarted preventing the system to enter deep-sleep for a new period of
     * time.
     */

    SER_restart_uart_sleep_timer ();
    uart_sleep_timer_enabled = 1;

#if TARGET_HAS_DTR_INPUT
    uart->index_hisr = (uart->index_hisr + 1) & 0x01; /* 0 or 1 */
#endif

    xon_xoff_detected = 0;
    
    /*
     * Switch to the other buffer.
     */
     
    current_byte = uart->rx_buffer_used_by_rx_hisr;
    if (current_byte == &(uart->rx_fifo_byte_1[0])) {
    
        current_byte = &(uart->rx_fifo_byte_2[0]);
        bytes_read = uart->bytes_in_rx_buffer_2;
    
    } else {
    
        current_byte = &(uart->rx_fifo_byte_1[0]);
        bytes_read = uart->bytes_in_rx_buffer_1;
    }

    uart->rx_buffer_used_by_rx_hisr = current_byte;

    /*
     * All bytes are copied into the RX buffer only if an escape sequence has
     * been detected or a break in the detection has occured.
     */

    if (!detect_escape_sequence (uart)) {

        if (uart->rx_out > uart->rx_in)
            bytes_free_in_rx_buffer = (SYS_UWORD16) (uart->rx_out - uart->rx_in - 1);
        else
            bytes_free_in_rx_buffer =
                (SYS_UWORD16) (uart->buffer_size + uart->rx_out - uart->rx_in);

        wrap_around_counter = uart->buffer_size + 1 -
                              (SYS_UWORD16) (uart->rx_in - &(uart->rx_buffer[0]));
        
        if (uart->flow_control_mode == fc_xoff) {
        
            /*
             * For SW Flow Control, need to further investigate the processing
             * in order to improve the performance of the driver, and in order
             * to avoid managing the wrap around of the circular buffer each
             * time a character is copied.
             */
        
            while (bytes_read && bytes_free_in_rx_buffer) {

                /*
                 * If the data received is XON or XOFF, the transmitter is
                 * enabled (XON) or disabled (XOFF).
                 */

                if (*current_byte == uart->xoff_character) {

                    uart->tx_stopped_by_driver = 1;
                    xon_xoff_detected = 1;

                } else if (*current_byte == uart->xon_character) {

                    uart->tx_stopped_by_driver = 0;
                    xon_xoff_detected = 1;

#if ((CHIPSET != 5) && (CHIPSET != 6))
                    /*
                     * Disable sleep mode.
                     */
          
                    WRITE_UART_REGISTER (
                        uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

                    /*
                     * Unmask Tx interrupt.
                     */
          
                    WRITE_UART_REGISTER (
                        uart, IER, READ_UART_REGISTER (uart, IER) | ETBEI);

                } else {
    
                    *(uart->rx_in++) = *current_byte;

                    wrap_around_counter--;
                    if (!wrap_around_counter) {
                    
                        uart->rx_in = &(uart->rx_buffer[0]);
                        wrap_around_counter = uart->buffer_size + 1;
                    }
                    
                    bytes_free_in_rx_buffer--;
                }

                current_byte++;
                bytes_read--;
            }
        } else { /* No Flow Control or HW Flow Control */
                
            /*
             * Figure out the most restricting condition.
             */

            bytes_to_copy =
                Min3 (bytes_free_in_rx_buffer, wrap_around_counter, bytes_read);
                          
            /*
             * Copy characters into the circular Rx buffer.
             */
        
            memcpy (uart->rx_in, current_byte, bytes_to_copy);

            /*
             * Update first the variables associated to blocking conditions:
             * if (bytes_read = 0) OR
             *    (bytes_free_in_rx_buffer = 0) => No more characters to copy.
             */
                
            bytes_free_in_rx_buffer	-= bytes_to_copy;
            bytes_read -= bytes_to_copy;
                
            wrap_around_counter -= bytes_to_copy;
            if (!wrap_around_counter)
                uart->rx_in = &(uart->rx_buffer[0]);
            else
                uart->rx_in += bytes_to_copy;

            /*
             * Check if there are still some characters to copy.
             */

            if (bytes_read && bytes_free_in_rx_buffer) {

                /*
                 * Update the remaining variables and figure out again the
                 * most restricting condition. Since (bytes_read = 0) and
                 * (bytes_free_in_rx_buffer = 0) are blocking conditions, if
                 * we reach that point it means that the wrap around condition
                 * has just occurred and it is not needed to manage it again.
                 */

                current_byte += bytes_to_copy;
                bytes_to_copy = Min (bytes_read, bytes_free_in_rx_buffer);

                /*
                 * Copy characters into the circular Rx buffer and update
                 * current pointer.
                 */

                memcpy (uart->rx_in, current_byte, bytes_to_copy);

                uart->rx_in += bytes_to_copy;
                
                /*
                 * bytes_free_in_rx_buffer not updated since not used anymore.
                 */
                bytes_read -= bytes_to_copy;

            }
            else {
            	     bytes_read = 0;
            }
        } /* end if (uart->flow_control_mode == fc_xoff) */


        /*
         * If the high watermark is reached, RTS is activated or XOFF is
         * sent according to the flow control mode.
         */

        bytes_in_rx_buffer = get_bytes_in_rx_buffer (uart);

        if ((uart->flow_control_mode != fc_none) &&
            (bytes_in_rx_buffer >= RX_HIGH_WATERMARK (uart->buffer_size))) {

            /*
             * Check if receipt must be stopped.
             */

            if (!uart->rx_stopped_by_driver) {

                uart->rx_stopped_by_driver = 1;
                if (!uart->rx_stopped_by_application)
                    stop_receiver (uart);
            }
        }

        /*
         * If a reading was suspended or if the callback function is installed,
         * it is called if one of these conditions is fulfiled:
         *      - the RX threshold level is reached,
         *      - a break has been detected,
         */

        if ((!uart->rd_call_from_hisr_in_progress) &&
            (uart->reading_suspended ||
             (uart->rd_call_setup == rm_reInstall))) {

            if ((bytes_in_rx_buffer >= uart->rx_threshold_level) ||
#if TARGET_HAS_DTR_INPUT
                uart->dtr_change_detected[uart->index_hisr] ||
#endif
                uart->break_received ||
                xon_xoff_detected) {


                uart->rd_call_from_hisr_in_progress = 1;
                update_reading_callback (uart, 3); /* 3: call from Rx HISR. */

                uart->reading_suspended = 0;
                uart->break_received = 0;
                uart->esc_seq_received = 0;
#if TARGET_HAS_DTR_INPUT
                uart->dtr_change_detected[uart->index_hisr] = 0;
#endif
            }
        }

    }


    WRITE_UART_REGISTER (uart, LCR, READ_UART_REGISTER (uart, LCR) & ~DLAB);
              
    /* Mask The Rx and Modem status interrupt */
    WRITE_UART_REGISTER (
               uart, IER, READ_UART_REGISTER (uart, IER) &
                       ~(ERBI | EDSSI));
              
    if ((uart->rx_buffer_used_by_rx_hisr) == &(uart->rx_fifo_byte_1[0]))
	uart->bytes_in_rx_buffer_1 = 0;
    else
	uart->bytes_in_rx_buffer_2 = 0;

       
    WRITE_UART_REGISTER (uart, LCR, READ_UART_REGISTER (uart, LCR) & ~DLAB);
              
    /* Unmask The Rx and Modem status interrupt*/
    WRITE_UART_REGISTER (
               uart, IER, READ_UART_REGISTER (uart, IER) |
                       (ERBI | EDSSI));

    if(uart->rx_stopped_by_lisr  ) {
       	if (!uart->rx_stopped_by_driver) {

              uart->rx_stopped_by_lisr = 0;
        
              /*
                * Reset LCR[7] (DLAB) to have access to the RBR, THR and IER registers.
                */
              WRITE_UART_REGISTER (uart, LCR, READ_UART_REGISTER (uart, LCR) & ~DLAB);
         

              /* UnMask The Rx interrupt */
              WRITE_UART_REGISTER (
               uart, IER, READ_UART_REGISTER (uart, IER) |
                       (ERBI | EDSSI));
             
              start_receiver (uart);
             
       	}
    }

}

/*******************************************************************************
 *
 *                          hisr_execute_v24_operations
 * 
 * Purpose  : The user's function is called if all conditions to call it are
 *            fulfiled.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static VOID
hisr_execute_v24_operations (VOID)
{
    t_uart *uart;

    uart = &uart_parameters;

    /*
     * If a reading was suspended or if the callback function is installed,
     * it is called.
     */

    if ((!DRIVER_DISABLED (uart)) &&
        (!uart->rd_call_from_hisr_in_progress) &&
        (uart->reading_suspended || (uart->rd_call_setup == rm_reInstall))) {

        uart->rd_call_from_hisr_in_progress = 1;
        update_reading_callback (uart, 1); /* 1: call from HISR. */
        uart->reading_suspended = 0;
        uart->break_received = 0;
        uart->esc_seq_received = 0;
    }

}

/*******************************************************************************
 *
 *                          hisr_execute_tx_operations
 * 
 * Purpose  : Writes bytes from the TX buffer to the TX FIFO.
 *            The user's function is called if all conditions to call it are
 *            fulfiled.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static VOID
hisr_execute_tx_operations (VOID)
{
    SYS_UWORD16 bytes_in_tx_buffer;
    SYS_UWORD16 bytes_in_tx_fifo;
    SYS_UWORD16 wrap_around_counter;
    SYS_UWORD16 bytes_to_write;
    t_uart *uart;
    int counter;


    uart = &uart_parameters;

    /*
     * A TX interrupt may have occured during the previous TX HISR. This case
     * may appear when a HISR having a higher priority has been activated when
     * the TX HISR was activated. When the next TX HISR is activated, the TX
     * FIFO may not be empty. Nothing is done until a TX interrupt will occur.
     * The RX HISR will be activated again and the TX FIFO will be empty.
     */

    if (READ_UART_REGISTER (uart, LSR) & THRE) {

        bytes_in_tx_fifo = 0;
    
        /*
         * A request to send a XON/XOFF character may have been done by the 
         * RX interrupt handler. The byte can be written because we are sure
         * that the TX FIFO is not full.
         */

        if (uart->send_xon_xoff) {

            WRITE_UART_REGISTER (uart, THR, uart->xon_xoff_to_send);
            uart->send_xon_xoff = 0;
            bytes_in_tx_fifo++;
        }

        if ((!uart->tx_stopped_by_application) &&
            (!uart->tx_stopped_by_driver)) {

            bytes_in_tx_buffer = get_bytes_in_tx_buffer (uart);
            wrap_around_counter =
                uart->buffer_size + 1 - (SYS_UWORD16) (uart->tx_out -
                                                  &(uart->tx_buffer[0]));

            /*
             * Figure out the most restricting condition.
             */

#if ((CHIPSET == 3) || (CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6))
            /*
             * Loading of only (FIFO_SIZE - 1) characters in the Tx FIFO to
             * avoid the generation of a spurious Tx FIFO almost empty
             * interrupt (Ulysse bug report #35).
             */

            bytes_to_write =
                Min3 (bytes_in_tx_buffer, wrap_around_counter,
                      (FIFO_SIZE - 1 - bytes_in_tx_fifo));
#elif ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
            /*
             * Bug corrected on Calypso rev. A, rev. B, C035, Ulysse C035,
             * Calypso Lite & Calypso+.
             */

            bytes_to_write =
                Min3 (bytes_in_tx_buffer, wrap_around_counter,
                (FIFO_SIZE - bytes_in_tx_fifo));
#endif

            /*
             * Write characters into the Tx FIFO.
             */

            for (counter = 0; counter < bytes_to_write; counter++)
                WRITE_UART_REGISTER (uart, THR, *(uart->tx_out++));

            /*
             * Update the variables associated to blocking conditions:
             * if (bytes_in_tx_buffer = 0) OR
             *    (bytes_in_tx_fifo = FIFO_SIZE) => No more characters to copy.
             */

            bytes_in_tx_buffer -= bytes_to_write;
            bytes_in_tx_fifo += bytes_to_write;

            wrap_around_counter -= bytes_to_write;
            if (!wrap_around_counter)
                uart->tx_out = &(uart->tx_buffer[0]);

            /*
             * Check if there are still some characters to write.
             */

            if (bytes_in_tx_buffer &&
#if ((CHIPSET == 3) || (CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6))
                (bytes_in_tx_fifo < (FIFO_SIZE - 1))) {
#elif ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
                (bytes_in_tx_fifo < FIFO_SIZE)) {
#endif

                /*
                 * Figure out again the most restricting condition. Since
                 * (bytes_in_tx_buffer = 0) and	(bytes_in_tx_fifo = FIFO_SIZE)
                 * are blocking conditions, if we reach that point it means
                 * that the wrap around condition has just occurred and it is
                 * not needed to manage it again.
                 */

#if ((CHIPSET == 3) || (CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6))
                bytes_to_write =
                    Min (bytes_in_tx_buffer,
                         (FIFO_SIZE - 1 - bytes_in_tx_fifo));
#elif ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
                bytes_to_write =
                    Min (bytes_in_tx_buffer,
                         (FIFO_SIZE - bytes_in_tx_fifo));
#endif

                /*
                 * Write characters into the Tx FIFO and update associated
                 * variables.
                 */

                for (counter = 0; counter < bytes_to_write; counter++)
                    WRITE_UART_REGISTER (uart, THR, *(uart->tx_out++));

                bytes_in_tx_buffer += bytes_to_write;
                bytes_in_tx_fifo += bytes_to_write;
            }

            /*
             * If a writing was suspended or if the callback function is
             * installed, it is called if the TX threshold level is reached.
             */

            if ((!DRIVER_DISABLED (uart)) &&
                (!uart->wr_call_from_hisr_in_progress) &&
                (bytes_in_tx_buffer <= uart->tx_threshold_level) &&
                ((uart->wr_call_setup == rm_reInstall) ||
                 uart->writing_suspended)) {

                uart->writing_suspended = 0;

                uart->wr_call_from_hisr_in_progress = 1;
                update_writing_callback (uart, 1); /* 1: call from HISR. */
            }
        } /* end if ((!uart->tx_stopped_by_application) && */
          /*         (!uart->tx_stopped_by_driver))        */

        if (bytes_in_tx_fifo)

            /*
             * Unmask Tx interrupt.
             */
          
            WRITE_UART_REGISTER (
                uart, IER, READ_UART_REGISTER (uart, IER) | ETBEI);

        else {

            if ((!bytes_in_tx_fifo) && (uart->break_to_send))
                bytes_in_tx_fifo = send_break (uart);
        }

#if ((CHIPSET != 5) && (CHIPSET != 6))
        /*
         * Re-enable the sleep mode.
         */

/* BELOW LINES WERE COMMENTED TO DISABLE SLEEP MODE IN DRIVER */
/*
        WRITE_UART_REGISTER (
            uart, IER, READ_UART_REGISTER (uart, IER) | IER_SLEEP);

            */
#endif
    }
}

/*******************************************************************************
 *
 *                              read_rx_fifo
 * 
 * Purpose  : Reads the RX FIFO. If the driver is enabled bytes are written in
 *            an array to be analyzed by the RX HISR.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static void
read_rx_fifo (t_uart *uart)
{

    /*
     * If the driver is disabled the RX FIFO is read to acknoledge the
     * interrupt else bytes received are written into an array which will be
     * analyzed from the RX HISR.
     */
     
    if (DRIVER_DISABLED (uart))
        empty_rx_fifo (uart);

    else if (build_rx_fifo_array (uart)) {
        (void) NU_Activate_HISR (&(uart->rx_hisr_ctrl_block));
    }

}

/*******************************************************************************
 *
 *                          check_v24_input_lines
 * 
 * Purpose  : Check the V.24 input lines. According to the states of the input
 *            lines and to the flow control mode selected, the transmitter is
 *            enabled or disabled. The reading callback function is called if
 *            it is installed and if all conditions are fulfiled.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static void
check_v24_input_lines (t_uart *uart)
{
    SYS_BOOL v24_input_line_changed;
    volatile SYS_UWORD8 modem_status;

    modem_status = READ_UART_REGISTER (uart, MSR);
    v24_input_line_changed = 0;

    if (modem_status & DELTA_CTS) {

        v24_input_line_changed = 1;

        if (modem_status & MCTS)
            uart->rts_level = 0;
        else
            uart->rts_level = 1;
    }

#if (CHIPSET == 12)
    else if (modem_status & DELTA_DSR) {
	 v24_input_line_changed = 1;

        if (modem_status & MDSR)
        {
            uart->dtr_level = 0;
	     if (uart->flow_control_mode != fc_dtr  && uart->baudrate == baudrate_value[FD_BAUD_AUTO])
            UAF_SetComPar (UAF_UART_1, FD_BAUD_AUTO, bpc_8, sb_1, pa_none); /* switch back to autobaud */
        }
        else
            uart->dtr_level = 1;

        /*
         * The reading callback function has to be called. But bytes received before
         * the change of state of DTR must be copied into the RX buffer before to
         * call it.
         */
         
        if (READ_UART_REGISTER (uart, LSR) & DR) { /* If Rx FIFO is not empty */
    
            /*
             * The Rx FIFO will be read to fill one of the two buffers and the Rx
             * HISR will be activated.
             */
           
            uart->index_it = (uart->index_it + 1) & 0x01; /* 0 or 1 */
            uart->dtr_change_detected[uart->index_it] = 1;
            uart->dtr_level_saved[uart->index_it] = uart->dtr_level;
            read_rx_fifo (uart);
            
        } else
            v24_input_line_changed = 1;
    }
#endif

    /*
     * When the hardware flow control is selected, if the RS 232 input signal is
     * deactivated (low), the transmitter is stopped.
     */

    if (uart->flow_control_mode == fc_rts) {
        
        if (uart->rts_level) {
            uart->tx_stopped_by_driver = 1;
        }    

        else {
            
            uart->tx_stopped_by_driver = 0;
#ifdef EXTHOST_WAKEUP_GPIO
            AI_ResetBit(EXTHOST_WAKEUP_GPIO);
#endif

#if ((CHIPSET != 5) && (CHIPSET != 6))
            /*
             * Disable sleep mode.
             */
          
            WRITE_UART_REGISTER (
                uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

            /*
             * Unmask Tx interrupt.
             */

            WRITE_UART_REGISTER (
                uart, IER, READ_UART_REGISTER (uart, IER) | ETBEI);

        }
    }

    if (v24_input_line_changed)
        (void) NU_Activate_HISR (&(uart->v24_hisr_ctrl_block));
}

/*******************************************************************************
 *
 *                              fill_tx_fifo
 * 
 * Purpose  : If the TX buffer is not empty, and if there is no break in
 *            progress, bytes are written into the TX FIFO until the TX FIFO is
 *            full or the TX buffer is empty. Else, if there is a break to send
 *            an all 0s character is written into the TX FIFO and a break is
 *            declared in progress to avoid to fill the TX FIFO on the next
 *            interrupt.
 *            When the TX FIFO is empty and if a break is in progress, the break
 *            length is programmed: all 0s characters are written into the TX
 *            FIFO. The number of bytes has been defined previously with the
 *            UAF_SetLineState function. The break condition is enabled.
 *            When the TX FIFO and the transmitter shift register (TSR) are both
 *            empty and if a break is in progress, the break condition is 
 *            disabled.
 *            When bytes are written from the TX buffer to the TX FIFO, the
 *            writing callback function is called if it is installed and if all
 *            conditions are fulfiled.
 *
 * Arguments: In : uart: Pointer on the UART structure.
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

static void
fill_tx_fifo (t_uart *uart)
{
#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Disable sleep mode.
     */

    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

    /*
     * Mask Tx interrupt.
     */

    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) & ~ETBEI);

    /*
     * If a break is in progress, bytes of the TX buffer are not written into
     * the TX FIFO.
     */

    if (!uart->break_in_progress)
        (void) NU_Activate_HISR (&(uart->tx_hisr_ctrl_block));    

    else {

        /*
         * The break HISR is activated and the break condition is cleared.
         */

        WRITE_UART_REGISTER (
            uart, LCR, READ_UART_REGISTER (uart, LCR) & ~BREAK_CONTROL);

        (void) NU_Activate_HISR (&(uart->break_hisr_ctrl_block));
    }
}

/*******************************************************************************
 *
 *                                UAF_Init
 * 
 * Purpose  : Initializes the UART hardware and installs interrupt handlers.
 *            The parameters are set to the default values:
 *               - 19200 baud,
 *               - 8 bits / character,
 *               - no parity,
 *               - 1 stop bit,
 *               - no flow control.
 *            All functionalities of the UART driver are disabled.
 *
 * Arguments: In : uartNo: Used UART.
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_INTERNAL_ERR : Internal problem.
 *
 ******************************************************************************/

T_FDRET
UAF_Init (T_fd_UartId uartNo)
{
    t_uart *uart;
    volatile SYS_UWORD8 status;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    uart = &uart_parameters;

    /*
     * Create the 3 HISR actived in the RX/TX and V24 interrupt handlers.
     * A return is used to simplify the code if an error occurs.
     * All stacks are entirely filled with the pattern 0xFE.
     */
     
    memset (&(uart->rx_hisr_stack[0]), 0xFE, RX_HISR_STACK_SIZE);

    if (NU_Create_HISR (&(uart->rx_hisr_ctrl_block),
                        "UAF_Rx",
                        hisr_execute_rx_operations,
                        RX_HISR_PRIORITY,
                        &(uart->rx_hisr_stack[0]),
                        RX_HISR_STACK_SIZE) != NU_SUCCESS)

        return (FD_INTERNAL_ERR);

    memset (&(uart->tx_hisr_stack[0]), 0xFE, TX_HISR_STACK_SIZE);

    if (NU_Create_HISR (&(uart->tx_hisr_ctrl_block),
                        "UAF_Tx",
                        hisr_execute_tx_operations,
                        TX_HISR_PRIORITY,
                        &(uart->tx_hisr_stack[0]),
                        TX_HISR_STACK_SIZE) != NU_SUCCESS)

        return (FD_INTERNAL_ERR);

    memset (&(uart->v24_hisr_stack[0]), 0xFE, V24_HISR_STACK_SIZE);

    if (NU_Create_HISR (&(uart->v24_hisr_ctrl_block),
                        "UAF_V24",
                        hisr_execute_v24_operations,
                        V24_HISR_PRIORITY,
                        &(uart->v24_hisr_stack[0]),
                        V24_HISR_STACK_SIZE) != NU_SUCCESS)

        return (FD_INTERNAL_ERR);

    /*
     * Create the HISR used to send a break.
     * A return is used to simplify the code if an error occurs.
     * The stack is entirely filled with the pattern 0xFE.
     */

    memset (&(uart->break_hisr_stack[0]), 0xFE, BREAK_HISR_STACK_SIZE);

    if (NU_Create_HISR (&(uart->break_hisr_ctrl_block),
                        "UAF_Brk",
                        hisr_start_break,
                        BREAK_HISR_PRIORITY,
                        &(uart->break_hisr_stack[0]),
                        BREAK_HISR_STACK_SIZE) != NU_SUCCESS)

        return (FD_INTERNAL_ERR);

    /*
     * Create a timer used in the break HISR.
     * A return is used to simplify the code if an error occurs.
     */

    if (NU_Create_Timer (&(uart->break_timer_ctrl_block),
                         "Break",
                         stop_break,
                         0, /* Parameter supplied to the routine: not used. */
                         1, /* This parameter is set when the timer is reset. */
                         0, /* The timer expires once. */
                         NU_DISABLE_TIMER) != NU_SUCCESS)

        return (FD_INTERNAL_ERR);

    /*
     * Create a timer used in the detection of the escape sequence.
     * A return is used to simplify the code if an error occurs.
     */

    if (NU_Create_Timer (&(uart->guard_period_timer_ctrl_block),
                         "Esc seq",
                         analyze_guard_period_timer_expiration,
                         0, /* Parameter supplied to the routine: not used. */
                         1, /* This parameter is set when the timer is reset. */
                         0, /* The timer expires once. */
                         NU_DISABLE_TIMER) != NU_SUCCESS)

        return (FD_INTERNAL_ERR);

    /*
     * These data are used to send a break.
     * A character has: 8 data bits + 1 start bit + 1 stop bit = 10 bits.
     */

    uart->baudrate = baudrate_value[FD_BAUD_19200];
    uart->autobauding = 0;	
    uart->bits_per_char = 10;

    /*
     * UART base address.
     */

    uart->base_address = base_address[uartNo];

    /*
     * Select the current array used to store received bytes.
     */
     
    uart->rx_buffer_used_by_rx_lisr = &(uart->rx_fifo_byte_2[0]);
    uart->rx_buffer_used_by_rx_hisr = &(uart->rx_fifo_byte_2[0]);
    
    /*
     * RX and TX buffers.
     */

    uart->buffer_size        = FD_MAX_BUFFER_SIZE;
    uart->rx_threshold_level = 1;
    uart->tx_threshold_level = 0;
    uart->rx_in              = &(uart->rx_buffer[0]);
    uart->rx_out             = &(uart->rx_buffer[0]);
    uart->tx_in              = &(uart->tx_buffer[0]);
    uart->tx_out             = &(uart->tx_buffer[0]);

    /*
     * Escape sequence.
     */

    uart->esc_seq_modified        = 0;
    uart->esc_seq_detection_state = INITIALIZATION;
    uart->esc_seq_character       = DEFAULT_ESC_SEQ_CHARACTER;
    uart->guard_period            = CONVERT_TIME_IN_TDMA (
                                         DEFAULT_GUARD_PERIOD);

    /*
     * Flow control.
     */

    uart->flow_control_mode         = fc_none;
    uart->send_xon_xoff             = 0;
    uart->rx_stopped_by_application = 1;
    uart->rx_stopped_by_driver      = 0;
    uart->rx_stopped_by_lisr      = 0;
    uart->tx_stopped_by_application = 1;
    uart->tx_stopped_by_driver      = 0;

    /*
     * Break.
     */

    uart->break_received    = 0;
    uart->break_to_send     = 0;
    uart->break_in_progress = 0;

    /*
     * Callback (UAF_ReadData and UAF_WriteData).
     */

    uart->esc_seq_received  = 0;

    uart->reading_suspended             = 0;
    uart->writing_suspended             = 0;
    uart->rd_call_from_hisr_in_progress = 0;
    uart->wr_call_from_hisr_in_progress = 0;
    uart->rd_call_setup                 = rm_noInstall;
    uart->wr_call_setup                 = rm_noInstall;

    /*
     * State defined in UAF_GetLineState.
     */

    uart->state_1 = 0;
    uart->state_2 = 0;
    uart->state   = &(uart->state_1);

    /*
     * Errors counters.
     */

    uart->framing_error       = 0;
    uart->parity_error        = 0;
    uart->overrun_error       = 0;
    uart->spurious_interrupts = 0;

    uart->max_rx_fifo_level   = 0;

    /*
     * Mask all interrupts causes and disable sleep mode and low power mode.
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
    WRITE_UART_REGISTER (uart, TLR, RX_FIFO_TRIGGER_LEVEL);
    RESET_BIT (uart, MCR, TCR_TLR_BIT);

    /*
     * Force the generation of THR_IT on TX FIFO empty: SCR[3] = 1.
     */

    WRITE_UART_REGISTER (
        uart, SCR, READ_UART_REGISTER (uart, SCR) | TX_EMPTY_CTL_IT);
    
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

    WRITE_UART_REGISTER (uart, DLL, dll[FD_BAUD_19200]);
    WRITE_UART_REGISTER (uart, DLM, dlh[FD_BAUD_19200]);

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

    /*
     * Read the state of RTS (RTS on RS232, CTS on chipset).
     */

    status = READ_UART_REGISTER (uart, MSR);

    if (status & MCTS)
        uart->rts_level = 0;
    else
        uart->rts_level = 1;

#if TARGET_HAS_DTR_INPUT
    /*
     * On C & D-Sample, 2 I/O are used to control DCD and DTR on UART Modem.
     * DCD: I/O 2 (output)
     * DTR: I/O 3 (input)
     */
     
    #define EXTENDED_MCU_REG (0xFFFEF006) /* Extended MCU register       */
    #define ASIC_CONFIG_REG  (0xFFFEF008) /* Asic Configuration register */

    #define IO_DTR (10) /* I/O 3; bit 10 of Asic Configuration register */
    #define IO_DCD ( 6) /* I/O 2; bit 6 of Extended MCU register        */

    /*
     * Select I/O for DCD and configure it as output.
     * DCD should start HIGH (not asserted).
     */

    *((volatile SYS_UWORD16 *) EXTENDED_MCU_REG) &= ~(1 << IO_DCD);
    AI_ConfigBitAsOutput (ARMIO_DCD);
    AI_SetBit (ARMIO_DCD);

    /*
     * Select I/O for DTR and configure it as input.
     * An interrupt is used to detect a change of state of DTR. Falling edge
     * or rising edge is selected according to the state of DTR.
     */
    
    *((volatile SYS_UWORD16 *) ASIC_CONFIG_REG) &= ~(1 << IO_DTR);
    AI_ConfigBitAsInput (ARMIO_DTR);
    uart->dtr_level = AI_ReadBit (ARMIO_DTR);

    if (uart->dtr_level)
        AI_SelectIOForIT (ARMIO_DTR, ARMIO_FALLING_EDGE);
    else
        AI_SelectIOForIT (ARMIO_DTR, ARMIO_RISING_EDGE);
        
    AI_UnmaskIT (ARMIO_MASKIT_GPIO);

    /*
     * Reset the 2 indexes of the circular buffer of 2 elements.
     * The circular buffer does not need to be initialized.
     */
     
    uart->index_it = 0;
    uart->index_hisr = 0;
#elif (CHIPSET == 12)
    /*
     * DCD and DTR are directly handled by Calypso+.
     * Force DCD pin to HIGH
     */

    WRITE_UART_REGISTER (uart, MCR, READ_UART_REGISTER(uart, MCR) & ~MDCD);

    /*
     * Read the state of DTR (DTR on RS232, DSR on chipset).
     */

    status = READ_UART_REGISTER (uart, MSR);

    if (status & MDSR)
        uart->dtr_level = 0;
    else
        uart->dtr_level = 1;

    /*
     * Reset the 2 indexes of the circular buffer of 2 elements.
     * The circular buffer does not need to be initialized.
     */
     
    uart->index_it = 0;
    uart->index_hisr = 0;
#endif /* BOARD == 8, 9, 40 or 41, CHIPSET == 12 */
    
    /*
     * Unmask RX interrupt and the modem status interrupt.
     */

    WRITE_UART_REGISTER (uart, IER, ERBI | EDSSI);

#if (CHIPSET == 12)
    /*
     * Unmask DSR interrupt in order to detect a change of state of DTR.
     */

	ENABLE_DSR_INTERRUPT (uart);
#endif

    return (FD_OK);
}

/*******************************************************************************
 *
 *                               UAF_Enable
 * 
 * Purpose  : The functionalities of the UART driver are disabled or enabled.
 *            In the deactivated state, all information about the communication
 *            parameters should be stored and recalled if the driver is again
 *            enabled. When the driver is enabled the RX and TX buffers are
 *            cleared.
 *
 * Arguments: In : uartNo: Used UART.
 *               : enable: 1: enable the driver
 *                         0: disable the driver
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_Enable (T_fd_UartId uartNo,
            SYS_BOOL enable)
{
    t_uart  *uart;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     */
     
    uart = &uart_parameters;

    if (enable) {

        uart->rx_stopped_by_driver = 0;
            
        ENABLE_DRIVER (uart);
        start_receiver (uart);

    } else {

        DISABLE_DRIVER (uart);
        stop_receiver (uart);

        uart->tx_in = &(uart->tx_buffer[0]);
        uart->rx_in = &(uart->rx_buffer[0]);
        uart->tx_out = uart->tx_in;
        uart->rx_out = uart->rx_in;
    }

    return (FD_OK);
}

/*******************************************************************************
 *
 *                            UAF_SetComPar
 * 
 * Purpose  : Sets up the communication parameters: baud rate, bits per
 *            character, number of stop bits, parity.
 *
 * Arguments: In : uartNo  : Used UART.
 *                 baudrate: Used baud rate.
 *                 bpc     : Used bits per character.
 *                 sb      : Used stop bits.
 *                 parity  : Used parity.
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: The specified parameters don't fit to the
 *                              capabilities of the UART or wrong UART number.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_SetComPar (T_fd_UartId uartNo,
               T_baudrate baudrate,
               T_bitsPerCharacter bpc,
               T_stopBits sb,
               T_parity parity)
{
    t_uart *uart;
    volatile SYS_UWORD8 mcr_value;
    volatile SYS_UWORD8 status;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     * pa_space is not supported. Some baudrates are not supported too.
     * A return is used to simplify the code.
     */

    if ((!baudrate_value[baudrate]) ||
        (parity == pa_space))

        return (FD_NOT_SUPPORTED);

    uart = &uart_parameters;

    /*
     * Mask all interrupts causes and disable sleep mode and low power mode.
     */

    WRITE_UART_REGISTER (uart, IER, 0x00);

#if TARGET_HAS_DTR_INPUT
    AI_MaskIT (ARMIO_MASKIT_GPIO);
#elif (CHIPSET == 12)
    DISABLE_DSR_INTERRUPT (uart);
#endif

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

    uart->baudrate = baudrate_value[baudrate];
    uart->autobauding = (baudrate == FD_BAUD_AUTO);  /* if autobauding enable trigger */
    uart->bits_per_char = 1; /* Start bit. */
    mcr_value = DLAB;

    if (bpc == bpc_7) {
    
        mcr_value |= WLS_7;
        uart->bits_per_char += 7;

    } else {

        mcr_value |= WLS_8;
        uart->bits_per_char += 8;
    }

    if (sb == sb_2) {

        mcr_value |= STB;
        uart->bits_per_char += 2;

    } else
        uart->bits_per_char += 1;

    switch (parity) {

    case pa_even:

        mcr_value |= (PEN | EPS);
        uart->bits_per_char += 1;

        break;

    case pa_odd:

        mcr_value |= PEN;
        uart->bits_per_char += 1;

        break;

    default:

        /*
         * There is nothing to do.
         */

        break;
    }

    WRITE_UART_REGISTER (uart, LCR, mcr_value);

    /*
     * Program the trigger levels.
     * MCR[6] must be set to 1.
     */
     
    SET_BIT (uart, MCR, TCR_TLR_BIT);
    WRITE_UART_REGISTER (uart, TCR, 0x0F);
    WRITE_UART_REGISTER (uart, TLR, RX_FIFO_TRIGGER_LEVEL);
    RESET_BIT (uart, MCR, TCR_TLR_BIT);

    /*
     * Force the generation of THR_IT on TX FIFO empty: SCR[3] = 1.
     */

    WRITE_UART_REGISTER (
        uart, SCR, READ_UART_REGISTER (uart, SCR) | TX_EMPTY_CTL_IT);
    
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
     
    WRITE_UART_REGISTER (uart, MDR1,  ((baudrate==FD_BAUD_AUTO)?
                                         UART_MODE_AUTOBAUDING:
                                         UART_MODE)             |
                                     IR_SLEEP_DISABLED     |
                                     SIR_TX_WITHOUT_ACREG2 |
                                     FRAME_LENGTH_METHOD);

    /*
     * Read the state of RTS (RTS on RS232, CTS on chipset).
     */

    status = READ_UART_REGISTER (uart, MSR);

    if (status & MCTS)
        uart->rts_level = 0;
    else
        uart->rts_level = 1;

#if TARGET_HAS_DTR_INPUT
    /*
     * Read the state of DTR and select the edge.
     */
         
    uart->dtr_level = AI_ReadBit (ARMIO_DTR);
    
    if (uart->dtr_level)
        AI_SelectIOForIT (ARMIO_DTR, ARMIO_FALLING_EDGE);
    else
        AI_SelectIOForIT (ARMIO_DTR, ARMIO_RISING_EDGE);
        
    AI_UnmaskIT (ARMIO_MASKIT_GPIO);
#elif (CHIPSET == 12)
    /*
     * Read the state of DTR - No need to reload MSR register since its value
     * is still stored in the "status" local variable.
     */

    if (status & MDSR)
        uart->dtr_level = 0;
    else
        uart->dtr_level = 1;
#endif

#if ((CHIPSET == 5) || (CHIPSET == 6))
    /*
     * Unmask RX and TX interrupts and the modem status interrupt. 
     */

    WRITE_UART_REGISTER (uart, IER, ERBI | ETBEI | EDSSI);
#elif (CHIPSET == 12)
    /*
     * Unmask RX and TX interrupts and the modem status interrupt... 
     */

    WRITE_UART_REGISTER (uart, IER, ERBI | ETBEI | EDSSI);

    /*
     * ... Then, unmask DSR interrupt...
     */

	ENABLE_DSR_INTERRUPT (uart);  

    /*
     * ... And finally allow sleep mode.
     */

   /* BELOW LINES WERE COMMENTED TO DISABLE SLEEP MODE IN DRIVER */
    /*
    WRITE_UART_REGISTER (uart, IER, READ_UART_REGISTER (uart, IER) | IER_SLEEP);
    */
#else
    /*
     * Unmask RX and TX interrupts and the modem status interrupt 
     * and allow sleep mode.
     */
    /* BELOW LINES WERE COMMENTED TO DISABLE SLEEP MODE IN DRIVER */

    /* WRITE_UART_REGISTER (uart, IER, ERBI | ETBEI | EDSSI | IER_SLEEP);*/

    WRITE_UART_REGISTER (uart, IER, ERBI | ETBEI | EDSSI);
#endif


    return (FD_OK);
}

/*******************************************************************************
 *
 *                            UAF_SetBuffer
 * 
 * Purpose  : Sets up the size of the circular buffers to be used in the UART
 *            driver. This function may be called only if the UART is disabled
 *            with UAF_Enable.
 *
 * Arguments: In : uartNo     : Used UART.
 *                 bufSize    : Specifies the size of the circular buffer.
 *                 rxThreshold: Amount of received bytes that leads to a call
 *                              to suspended read-out function which is passed
 *                              to the function UAF_ReadData.
 *                 txThreshold: Amount of bytes in the TX buffer to call the
 *                              suspended write-in function which is passed to
 *                              the function UAF_WriteData
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: bufSize exceeds the maximal possible
 *                              capabilities of the driver or the threshold
 *                              values don't correspond to the bufSize or
 *                              wrong UART number.
 *            FD_INTERNAL_ERR : Internal problem with the hardware or the
 *                              function has been called while the UART is
 *                              enabled.
 *
 ******************************************************************************/

T_FDRET
UAF_SetBuffer (T_fd_UartId uartNo,
               SYS_UWORD16 bufSize,
               SYS_UWORD16 rxThreshold,
               SYS_UWORD16 txThreshold)
{
    T_FDRET result;
    t_uart *uart;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    if ((bufSize > FD_MAX_BUFFER_SIZE) ||
        (rxThreshold > FD_MAX_BUFFER_SIZE) ||
        (txThreshold > FD_MAX_BUFFER_SIZE))

        result = FD_NOT_SUPPORTED;

    else {
    
        uart = &uart_parameters;
        
        if (!DRIVER_DISABLED (uart))
            result = FD_INTERNAL_ERR;

        else if (RX_HIGH_WATERMARK (bufSize) < RX_LOW_WATERMARK (bufSize))
            result = FD_NOT_SUPPORTED;

        else {
        
            uart->buffer_size = bufSize;
            uart->rx_threshold_level = rxThreshold;
            uart->tx_threshold_level = txThreshold;

            result = FD_OK;
        }
    }

    return (result);
}

/*******************************************************************************
 *
 *                             UAF_SetFlowCtrl
 * 
 * Purpose  : Changes the flow control mode of the UART driver.
 *            If a flow control is activated, DTR is activated or XOFF is sent
 *            if the RX buffer is not able to store the received characters else
 *            DTR is deactivated or XON is sent.
 *
 * Arguments: In : uartNo: Used UART.
 *                 fcMode: flow control mode (none, DTR/DSR, RTS/CTS, XON/XOFF).
 *                 XON   : ASCII code of the XON character.
 *                 XOFF  : ASCII code of the XOFF character.
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: The flow control mode is not supported or wrong
 *                              UART number.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_SetFlowCtrl (T_fd_UartId uartNo,
                 T_flowCtrlMode fcMode,
                 SYS_UWORD8 XON,
                 SYS_UWORD8 XOFF)
{
    T_FDRET result;
    t_uart  *uart;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     * The DTR/DSR protocol is not supported.
     */

    if (fcMode == fc_dtr)
        result = FD_NOT_SUPPORTED;

    else {
    
        uart = &uart_parameters;

        uart->tx_stopped_by_driver = 0;

        
        uart->xon_character = XON;
        uart->xoff_character = XOFF;
        uart->flow_control_mode = fcMode;

#if ((CHIPSET != 5) && (CHIPSET != 6))
        /*
         * Disable sleep mode.
         */

        WRITE_UART_REGISTER (
            uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

        WRITE_UART_REGISTER (
            uart, MCR, READ_UART_REGISTER (uart, MCR) | MRTS);
          
#if ((CHIPSET != 5) && (CHIPSET != 6))
        /*
         * Re-enable sleep mode.
         */
         /* BELOW LINES WERE COMMENTED TO DISABLE SLEEP MODE IN DRIVER */
         /*
        WRITE_UART_REGISTER (
            uart, IER, READ_UART_REGISTER (uart, IER) | IER_SLEEP);
            */
#endif
        
        if (fcMode == fc_rts) {
#if 1 // Dmitriy: enable hardware assisted CTS 
            volatile SYS_UWORD8 oldValue;

            oldValue = READ_UART_REGISTER (uart, LCR);
            
            // LCR value to allow acces to EFR
     
            WRITE_UART_REGISTER (uart, LCR, 0xBF);
            
            // enable hardware assisted CTS
            
            SET_BIT (uart, EFR, AUTO_CTS_BIT);
            
            WRITE_UART_REGISTER (uart, LCR, oldValue);    
#endif
            if (uart->rts_level)
                uart->tx_stopped_by_driver = 1;
        }

        /*
         * If the high watermark is reached, RTS is activated or XOFF is sent
         * according to the flow control mode. Else, RTS is deactivated or XON
         * is sent.
         */

        if (fcMode != fc_none) {

            if (get_bytes_in_rx_buffer (uart) >= RX_HIGH_WATERMARK (
                                                     uart->buffer_size)) {

                uart->rx_stopped_by_driver = 1;
                stop_receiver (uart);

            } else if (!DRIVER_DISABLED (uart)) {

                uart->rx_stopped_by_driver = 0;
                start_receiver (uart);
            }
            
        } else {
        
            uart->rx_stopped_by_driver = 0;
            uart->tx_stopped_by_driver = 0;
        }

        result = FD_OK;
    }

    return (result);
}

/*******************************************************************************
 *
 *                               UAF_SetEscape
 * 
 * Purpose  : To return to the command mode at the ACI while a data connection
 *            is established, an escape sequence has to be detected.
 *            To distinguish between user data and the escape sequence a
 *            defined guard period is necessary before and after this sequence.
 *
 * Arguments: In:  uartNo     : Used UART.
 *                 escChar    : ASCII character which could appear three times
 *                              as an escape sequence.
 *                 guardPeriod: Denotes the minimal duration of the rest before
 *                              the first and after the last character of the
 *                              escape sequence, and the maximal receiving
 *                              duration of the whole escape string. This value
 *                              is expressed in ms.
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_SetEscape (T_fd_UartId uartNo,
               SYS_UWORD8 escChar,
               SYS_UWORD16 guardPeriod)
{
    t_uart  *uart;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     */

    uart = &uart_parameters;

    uart->esc_seq_modified = 1;
    uart->esc_seq_character = escChar;
    uart->guard_period = CONVERT_TIME_IN_TDMA ((UNSIGNED) guardPeriod);
    uart->esc_seq_modified = 0; /* Set to 0 by the RX interrupt handler. */

    return (FD_OK);
}

/*******************************************************************************
 *
 *                              UAF_InpAvail
 * 
 * Purpose  : Returns the number of characters available in the RX buffer of the
 *            driver. If the driver is disabled the function returns 0.
 *
 * Arguments: In : uartNo: Used UART.
 *            Out: none
 *
 * Returns  : >= 0            : The returned value is the amount of data in the
 *                              RX buffer.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_NOT_READY    : The function is called while the callback of the
 *                              readOutFunc function is activated and still not
 *                              terminated.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_InpAvail (T_fd_UartId uartNo)
{
    T_FDRET result;
    t_uart  *uart;
    SYS_UWORD16  bytes_read;
    SYS_UWORD16  bytes_in_rx_buffer;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     */

    uart = &uart_parameters;

    if (uart->rd_call_setup == rm_notDefined)
        result = FD_NOT_READY;

    else if (DRIVER_DISABLED (uart))
        result = 0;

    else {

        bytes_in_rx_buffer = get_bytes_in_rx_buffer (uart);

        /*
         * Update reading pointer of the RX buffer if a callback from LISR
         * has been done.
         */

        if (uart->rd_call_from_hisr_in_progress) {

            bytes_read = uart->rd_size_before_call[0] -
                         uart->rd_size_after_call[0] +
                         uart->rd_size_before_call[1] -
                         uart->rd_size_after_call[1];

            uart->rx_out += bytes_read;

            if (uart->rx_out >= &(uart->rx_buffer[0]) + uart->buffer_size + 1)
                uart->rx_out = uart->rx_out - uart->buffer_size - 1;

            /*
             * Check if the low watermark is reached to enable the receiver.
             */

            bytes_in_rx_buffer = get_bytes_in_rx_buffer (uart);

            if ((uart->flow_control_mode != fc_none) &&
                (bytes_in_rx_buffer <= RX_LOW_WATERMARK (uart->buffer_size))) {

                if ((!uart->rx_stopped_by_application) &&
                    uart->rx_stopped_by_driver)
                    start_receiver (uart);

                uart->rx_stopped_by_driver = 0;
            }

            uart->rd_call_from_hisr_in_progress = 0;
        }

        result = (T_FDRET) bytes_in_rx_buffer;
    }

    return (result);
}

/*******************************************************************************
 *
 *                             UAF_OutpAvail
 * 
 * Purpose  : Returns the number of free characters in TX buffer of the driver.
 *            If the driver is disabled the function returns 0.
 *
 * Arguments: In : uartNo: Used UART.
 *            Out: none
 *
 * Returns  : >= 0            : The returned value is the amount of data in the
 *                              TX buffer.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_NOT_READY    : The function is called while the callback of the
 *                              writeInFunc function is activated and still not
 *                              terminated.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_OutpAvail (T_fd_UartId uartNo)
{
    T_FDRET result;
    t_uart  *uart;
    SYS_UWORD16  bytes_written;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     */

    uart = &uart_parameters;

    if (uart->wr_call_setup == rm_notDefined)
        result = FD_NOT_READY;

    else if (DRIVER_DISABLED (uart))
        result = 0;

    else {

        /*
         * Update reading pointer of the TX buffer if a callback from LISR
         * has been done.
         */

        if (uart->wr_call_from_hisr_in_progress) {

            bytes_written = uart->wr_size_before_call[0] -
                            uart->wr_size_after_call[0] +
                            uart->wr_size_before_call[1] -
                            uart->wr_size_after_call[1];

            uart->tx_in += bytes_written;

            if (uart->tx_in >= &(uart->tx_buffer[0]) + uart->buffer_size + 1)
                uart->tx_in = uart->tx_in - uart->buffer_size - 1;

            uart->wr_call_from_hisr_in_progress = 0;

            /*
             * if the TX FIFO is empty, unmask TX empty interrupt.
             */

            if (!uart->tx_stopped_by_driver &&
                (READ_UART_REGISTER (uart, LSR) & THRE))
            {
#if ((CHIPSET != 5) && (CHIPSET != 6))
                /*
                 * Disable sleep mode.
                 */
          
                WRITE_UART_REGISTER (
                   uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

                /*
                 * Unmask Tx interrupt.
                 */
          
                WRITE_UART_REGISTER (
                    uart, IER, READ_UART_REGISTER (uart, IER) | ETBEI);
            }
        }

        result = (T_FDRET) (uart->buffer_size - get_bytes_in_tx_buffer (uart));
    }

    return (result);
}

/*******************************************************************************
 *
 *                             UAF_EnterSleep
 * 
 * Purpose  : Checks if UART is ready to enter Deep Sleep. If ready, enables
 *            wake-up interrupt.
 *
 * Arguments: In : uartNo: Used UART.
 *            Out: none
 *
 * Returns  : 0	              : Deep Sleep is not possible.
 *            >= 1            : Deep Sleep is possible.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

T_FDRET
UAF_EnterSleep (T_fd_UartId uartNo)
{
    t_uart              *uart;
    SYS_BOOL            deep_sleep;
    volatile SYS_UWORD8 status;
    
    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    uart = &uart_parameters;
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

#if (CHIPSET == 12)
        /*
         * Mask DSR interrupt.
         */

        DISABLE_DSR_INTERRUPT (uart);
#endif

        /*
         * Mask RX, TX and the modem status interrupts.
         */

        WRITE_UART_REGISTER (
            uart, IER, READ_UART_REGISTER (uart, IER) &
                       ~(ERBI | ETBEI | EDSSI));

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
 *                              UAF_WakeUp
 * 
 * Purpose  : Wakes up UART after Deep Sleep.
 *
 * Arguments: In : uartNo: Used UART.
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *
 * Warning: Parameters are not verified.
 *
 ******************************************************************************/

T_FDRET
UAF_WakeUp (T_fd_UartId uartNo)
{
    t_uart *uart;
   
    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    uart = &uart_parameters;

    /*
     * Disable the wake-up interrupt.
     */

    DISABLE_WAKEUP_INTERRUPT (uart);

    /*
     * Unmask RX and modem status interrupts.
     */

    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) | (ERBI | EDSSI));

#if (CHIPSET == 12)
    /*
     * Unmask DSR interrupt.
     */

    ENABLE_DSR_INTERRUPT (uart);
#endif

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Allow sleep mode.
     */
  /* BELOW LINES WERE COMMENTED TO DISABLE SLEEP MODE IN DRIVER */  
  /*
    WRITE_UART_REGISTER ( 
        uart, IER, READ_UART_REGISTER (uart, IER) | IER_SLEEP);
        */
#endif

    return (FD_OK);
}

/*******************************************************************************
 *
 *                              UAF_ReadData
 * 
 * Purpose  : To read the received characters out of the RX buffer the address
 *            of a function is passed. If characters are available, the driver
 *            calls this function and pass the address and the amount of
 *            readable characters. Because the RX buffer is circular, the
 *            callback function may be called with more than one address of
 *            buffer fragment.
 *            The readOutFunc function modifies the contents  of the size array
 *            to return the driver the number of processed characters. Each
 *            array entry is decremented by the number of bytes read in the
 *            fragment.
 *            If the UAF_ReadData is called while the RX buffer is empty, it
 *            depends on the suspend parameter to suspend the call-back or to
 *            leave without any operation. In the case of suspension, the
 *            return value of UAF_ReadData is UAF_SUSPENDED. A delayed call-back
 *            will be performed if:
 *              - the RX buffer reachs the adjusted threshold (rxThreshold of
 *                UAF_SetBuffer),
 *              - the state of a V.24 input line has changed,
 *              - a break is detected,
 *              - an escape sequence is detected.
 *            If no suspension is necessary the function returns the number of
 *            processed bytes.
 *
 * Arguments: In : uartNo     : Used UART.
 *                 suspend    : mode of suspension in case of RX buffer empty.
 *                 readOutFunc: Callback function.
 *                              cldFromIrq: The driver sets this parameter to 1
 *                                          if the callback function is called
 *                                          from an interrupt service routine.
 *                              reInstall : The call-back function sets this
 *                                          parameter to rm_reInstall if the
 *                                          driver must call again the callback
 *                                          function when the RX threshold level
 *                                          is reached. Else it will be set to
 *                                          rm_noInstall. Before to call the
 *                                          readOutFunc function this parameter
 *                                          is set to rm_notDefined.
 *                              nsource   : Informed the callback function about
 *                                          the number of fragments which are
 *                                          ready to copy from the circular RX
 *                                          buffer.
 *                              source    : Array which contains the addresses
 *                                          of the fragments.
 *                              size      : Array which contains the sizes of
 *                                          each fragments.
 *                              state     : The state parameter is the status
 *                                          of the V.24 lines and the break / 
 *                                          escape detection. The state
 *                                          parameter is described in the
 *                                          specification of UAF_GetLineState.
 *            Out: none
 *
 * Returns  : >= 0            : Succesful operation. Amount of processed bytes.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_SUSPENDED    : The callback is suspended until the buffer or
 *                              state condition changed.
 *            FD_NOT_READY    : The function is called while the callback is
 *                              activated and still not terminated.
 *            FD_INTERNAL_ERR : Internal problems with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_ReadData (T_fd_UartId uartNo,
              T_suspendMode suspend,
              void (readOutFunc (SYS_BOOL cldFromIrq,
                                 T_reInstMode *reInstall,
                                 SYS_UWORD8 nsource,
                                 SYS_UWORD8 *source[],
                                 SYS_UWORD16 size[],
                                 SYS_UWORD32 state)))
{
    T_FDRET result;
    t_uart *uart;
    SYS_UWORD16  bytes_read;
    SYS_UWORD16  bytes_in_rx_buffer;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     */

    uart = &uart_parameters;

    if (uart->rd_call_setup == rm_notDefined)
        result = FD_NOT_READY;

    else if (get_bytes_in_rx_buffer (uart) || uart->esc_seq_received) {

        uart->readOutFunc = readOutFunc;
        update_reading_callback (uart, 0); /* 0: call from application. */

        bytes_read = uart->rd_size_before_call[0] -
                     uart->rd_size_after_call[0] +
                     uart->rd_size_before_call[1] -
                     uart->rd_size_after_call[1];

        uart->rx_out += bytes_read;

        if (uart->rx_out >= &(uart->rx_buffer[0]) + uart->buffer_size + 1)
            uart->rx_out = uart->rx_out - uart->buffer_size - 1;

        /*
         * Check if the low watermark is reached to enable the receiver.
         */

        if ((uart->flow_control_mode != fc_none) &&
            (get_bytes_in_rx_buffer (uart) <= RX_LOW_WATERMARK (
                                                  uart->buffer_size))) {

            if ((!uart->rx_stopped_by_application) &&
                uart->rx_stopped_by_driver && (!uart->rx_stopped_by_lisr))
                start_receiver (uart);

            uart->rx_stopped_by_driver = 0;
        }

        uart->esc_seq_received = 0;
        result = (T_FDRET) bytes_read;

    } else  if (suspend == sm_suspend) {

        uart->readOutFunc = readOutFunc;
        uart->reading_suspended = 1;
        result = FD_SUSPENDED;

    } else {
        
        /*
         * The previous callback function is deinstalled.
         */
             
        uart->rd_call_setup = rm_noInstall;
        uart->reading_suspended = 0;
        result = 0; /* 0 byte read. */
    }

    return (result);
}

/*******************************************************************************
 *
 *                              UAF_WriteData
 * 
 * Purpose  : To write characters into the TX buffer the address of a function
 *            is passed. If free space is available in the buffer, the driver
 *            calls this function and passes the destination address and the
 *            amount of space. Because the TX buffer is circular, the callback
 *            function may be called with more than one address of buffer
 *            fragment.
 *            The writeInFunc function modifies the contents of the size array
 *            to return the driver the number of processed bytes. Each array
 *            entry is decremented  by the number of bytes written in this
 *            fragment.
 *            If the UAF_WriteData function is called while the TX buffer is
 *            full, it depends on the suspend parameter to suspend the
 *            call-back or to leave this function without any operation. In the
 *            case of suspension the returned value of the UAF_WriteData is
 *            UAF_SUSPENDED. A delayed call-back will be performed if the TX
 *            buffer reaches the adjusted threshold (txThreshold of
 *            UAF_SetBuffer). If no suspension is necessary the function returns
 *            the number of processed bytes.
 *
 * Arguments: In : uartNo     : Used UART.
 *                 suspend    : mode of suspension in case of TX buffer empty.
 *                 writeInFunc: Callback function. 
 *                              cldFromIrq: The driver sets this parameter to 1
 *                                          if the call-back function is called
 *                                          from an interrupt service routine.
 *                              reInstall : The callback function sets this
 *                                          parameter to rm_reInstall if the
 *                                          driver must call again the callback
 *                                          function when the TX threshold level
 *                                          is reached. Else it will be set to
 *                                          rm_noInstall. Before to call the
 *                                          writeInFunc function this parameter
 *                                          is set to rm_notDefined.
 *                              ndest     : Informed the callback function about
 *                                          the number of fragments which are
 *                                          available in the TX buffer.
 *                              dest      : Array which contains the addresses
 *                                          of the fragments.
 *                              size      : Array which contains the sizes of
 *                                          each fragments.
 *            Out: none
 *
 * Returns  : >= 0            : Succesful operation. Amount of processed bytes.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_SUSPENDED    : The callback is suspended until the buffer
 *                              condition changed.
 *            FD_NOT_READY    : The function is called while the callback is
 *                              activated and still not terminated.
 *            FD_INTERNAL_ERR : Internal problems with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_WriteData (T_fd_UartId uartNo,
               T_suspendMode suspend,
               void (writeInFunc (SYS_BOOL cldFromIrq,
                                  T_reInstMode *reInstall,
                                  SYS_UWORD8 ndest,
                                  SYS_UWORD8 *dest[],
                                  SYS_UWORD16 size[])))
{
    T_FDRET result;
    t_uart *uart;
    SYS_UWORD16  bytes_written;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     */

    uart = &uart_parameters;

    if (uart->wr_call_setup == rm_notDefined)
        result = FD_NOT_READY;

    else if ((!DRIVER_DISABLED (uart)) &&
             (get_bytes_in_tx_buffer (uart) < uart->buffer_size)) {

        uart->writeInFunc = writeInFunc;
        update_writing_callback (uart, 0); /* 0: call from application. */

        bytes_written = uart->wr_size_before_call[0] -
                        uart->wr_size_after_call[0] +
                        uart->wr_size_before_call[1] -
                        uart->wr_size_after_call[1];

        uart->tx_in += bytes_written;

        if (uart->tx_in >= &(uart->tx_buffer[0]) + uart->buffer_size + 1)
            uart->tx_in = uart->tx_in - uart->buffer_size - 1;

	/* If we have been stopped due to high RTS, we have to
	 * wake up application processor by IRQ via IO1 -HW */
#ifdef EXTHOST_WAKEUP_GPIO
	if (uart->tx_stopped_by_driver)
            AI_SetBit(EXTHOST_WAKEUP_GPIO);
#endif

        /*
         * If:
         *      - there is no break to send,
         *      - the flow control is not activated,
         * unmask the TX empty interrupt to be able to send characters.
         */
        if (!uart->break_to_send &&
            !uart->tx_stopped_by_driver)
        {    
#if ((CHIPSET != 5) && (CHIPSET != 6))
            /*
             * Disable sleep mode.
             */
          
            WRITE_UART_REGISTER (
                uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

            /*
             * Unmask Tx interrupt.
             */
          
            WRITE_UART_REGISTER (
                uart, IER, READ_UART_REGISTER (uart, IER) | ETBEI);
        }

        result = (T_FDRET) bytes_written;

    } else  if (suspend == sm_suspend) {

        uart->writeInFunc = writeInFunc;
        uart->writing_suspended = 1;
        result = FD_SUSPENDED;

    } else {
        
        /*
         * The previous callback function is deinstalled.
         */
             
        uart->wr_call_setup = rm_noInstall;
        uart->writing_suspended = 0;
        result = 0;
    }

    return (result);
}

/*******************************************************************************
 *
 *                              UAF_StopRec
 * 
 * Purpose  : If a flow control mode is set, this function tells the terminal
 *            equipment that no more data can be received.
 *            XON/XOFF: XOFF is sent.
 *            DTR/DSR : DTR is desactivated.
 *            RTS/CTS : RTS is deactivated.
 *
 * Arguments: In : uartNo: Used UART.
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_StopRec (T_fd_UartId uartNo)
{
    t_uart *uart;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     */

    uart = &uart_parameters;

    if (uart->flow_control_mode != fc_none)
        stop_receiver (uart);

    uart->rx_stopped_by_application = 1;

    return (FD_OK);
}

/*******************************************************************************
 *
 *                              UAF_StartRec
 * 
 * Purpose  : If a flow control mode is set, this function tells the terminal
 *            equipment that the receiver is again able to receive more data.
 *            If the buffer has already reached the high water mark the driver
 *            sends the signal only if the buffer drains to a low water mark.
 *            XON/XOFF: XON is sent.
 *            DTR/DSR : DTR is activated.
 *            RTS/CTS : RTS is activated.
 *
 * Arguments: In : uartNo: Used UART.
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET 
UAF_StartRec (T_fd_UartId uartNo)
{
    t_uart *uart;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     */

    uart = &uart_parameters;

    if ((uart->flow_control_mode != fc_none) && (!uart->rx_stopped_by_driver))
        start_receiver (uart);

    uart->rx_stopped_by_application = 0;

    return (FD_OK);
}

/*******************************************************************************
 *
 *                            UAF_GetLineState
 * 
 * Purpose  : Returns the state of the V.24 lines, the flow control state and
 *            the result of the break/escape detection process as a bit field.
 *
 * Arguments: In : uartNo: Used UART.
 *            Out: state : State of the V.24 lines, the flow control state and
 *                         the result of the break/escape sequence detection
 *                         process as a bit field.
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_NOT_READY    : The function is called while the callback of 
 *                              the readOutFunc function is activated and still
 *                              not terminated.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_GetLineState (T_fd_UartId uartNo,
                  SYS_UWORD32 *state)
{
    T_FDRET result;
    t_uart *uart;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     * Signals not supported are reported as 0.
     */

    uart = &uart_parameters;

    if (uart->rd_call_setup == rm_notDefined)
        result = FD_NOT_READY;

    else {

        /*
         * The field state_2 is used when state_1 is set to 0 to avoid to
         * lose events detected in the RX interrupt handler.
         * Fields BRK and BRKLEN are set when a break is detected.
         * The field ESC is set when an escape sequence is detected. 
         */

        *state = uart->state_2;
        uart->state_2 = 0;
        uart->state = &(uart->state_2);

        *state |= uart->state_1;
        uart->state_1 = 0;
        uart->state = &(uart->state_1);

        *state |= ((((SYS_UWORD32) uart->rts_level) << RTS) |
        
#if TARGET_HAS_DTR_INPUT
                   (((SYS_UWORD32) uart->dtr_level) << DTR) |
#endif

                   (((SYS_UWORD32) (uart->tx_stopped_by_application  |
                              uart->tx_stopped_by_driver)) << TXSTP) |

                   (((SYS_UWORD32) (uart->rx_stopped_by_application  |
                              uart->rx_stopped_by_driver)) << RXSTP) |

                   (((SYS_UWORD32) (uart->buffer_size - 
                              get_bytes_in_rx_buffer (uart))) << RXBLEV));
                                 
        /*
         * Fields SA, SB and X are set according to the flow control:
         *
         *       None    RTS/CTS    XON/XOFF
         * SA    DTR     DTR        DTR
         * SB    RTS     0          RTS
         * X     0       RTS        XON:0 XOFF:1 (transmitter)
         *
         * DTR is supported on C, D & E-Sample.
         */

#if TARGET_HAS_DTR_INPUT
        *state |= (((SYS_UWORD32) uart->dtr_level) << SA);
#endif

        if (uart->flow_control_mode != fc_rts)
            *state |= (((SYS_UWORD32) uart->rts_level) << SB);

        if (uart->flow_control_mode == fc_rts)
            *state |= (((SYS_UWORD32) uart->rts_level) << X);
            
        else if ((uart->flow_control_mode == fc_xoff) &&
                 (uart->tx_stopped_by_application ||
                  uart->tx_stopped_by_driver))
            *state |= (1 << X);

        result = FD_OK;
    }

    return (result);
}

/*******************************************************************************
 *
 *                            UAF_SetLineState
 * 
 * Purpose  : Sets the states of the V.24 status lines according to the bit
 *            field of the parameter state.
 *
 * Arguments: In : uartNo: Used UART.
 *                 state : Bit field. Only the signals which are marked with
 *                         the 'set' access can be used to change the state of
 *                         the signal.
 *                 mask  : Bit field with the same structure as state. Each bit
 *                         in state corresponds to a bit in mask. Settabled
 *                         bits marked by a 1 are manipulated by the driver.
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_SetLineState (T_fd_UartId uartNo,
                  SYS_UWORD32 state,
                  SYS_UWORD32 mask)
{
    t_uart  *uart;
    UNSIGNED break_length;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    uart = &uart_parameters;

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     * DCD is supported on C, D & E-Sample. The SA field is not supported because
     * DSR is not supported on all platforms.
     */

#if ((BOARD == 8) || (BOARD == 9) || (BOARD == 40) || (BOARD == 41) || (CHIPSET == 12))
    if (mask & (1 << SA))
#else
    if ((mask & (1 << SA)) || (mask & (1 << DCD)))
#endif
        return (FD_NOT_SUPPORTED); /* Return used to simplify the code */
        
    /*
     * Check if a break has to be sent.
     */

    uart->break_length = (UNSIGNED) ((state >> BRKLEN) & 0xFF);

    if (state & (1 << BRK) && (mask & (1 << BRK))) {

        if (uart->break_length > FIFO_SIZE)
            return (FD_NOT_SUPPORTED); /* Return used to simplify the code */

        else {

            uart->time_without_character =
                compute_break_time (uart->baudrate, uart->bits_per_char, 3);

            uart->break_to_send = 1;

            /*
             * If the TX FIFO is empty the break is send from this function
             * else the interrupt handler will send the break.
             */

            if (READ_UART_REGISTER (uart, LSR) & TEMT)
                send_break(uart);
        }
    }

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Disable sleep mode.
     */

    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) & ~IER_SLEEP);
#endif

    /*
     * The CTS field is ignored if the X bit in the mask is set. In this case
     * the X bit controls CTS.
     */

    if (mask & (1 << CTS)) {
    
        if (uart->flow_control_mode != fc_rts) {
        
            /*
             * As the RTS/CTS flow control is not selected, the X bit does not
             * control CTS. CTS needs only to be activated or deactivated
             * according to the value of the CTS field.
             */

            if (state & (1 << CTS))
                WRITE_UART_REGISTER (
                    uart, MCR, READ_UART_REGISTER (uart, MCR) | MRTS);

            else
                WRITE_UART_REGISTER (
                    uart, MCR, READ_UART_REGISTER (uart, MCR) & ~MRTS);

        } else if (!(mask & (1 << X))) {

            /*
             * The RTS/CTS flow control is selected but the X bit in the mask
             * is null. Then the CTS bit controls CTS and the receiver must be
             * stopped or started according to the state of the CTS bit.
             * The receiver is started only if it was not stopped by the driver
             * and if it was stopped by the application.
             */
             
            if (state & (1 << CTS)) {

                if (!uart->rx_stopped_by_application) {
                
                    if (!uart->rx_stopped_by_driver)
                        stop_receiver (uart);

                    uart->rx_stopped_by_application = 1;
                }
                
            } else {
                
                if ((!uart->rx_stopped_by_driver) &&
                    uart->rx_stopped_by_application)
                    start_receiver (uart);

                uart->rx_stopped_by_application = 0;
            }
        }
    }

    /*
     * The DCD field is ignored if the SB bit of the mask is set.
     */

#if ((BOARD == 8) || (BOARD == 9) || (BOARD == 40) || (BOARD == 41) || (CHIPSET == 12))

    if (!(mask & (1 << SB)) && (mask & (1 << DCD))) {

        if (state & (1 << DCD)) {
            /* Turn on DCD */
            #if (CHIPSET == 12)
                WRITE_UART_REGISTER (uart, MCR, READ_UART_REGISTER(uart, MCR) | MDCD);
            #else
                AI_ResetBit (ARMIO_DCD);
            #endif
        } else {
            /* Turn off DCD */
            #if (CHIPSET == 12)
                WRITE_UART_REGISTER (uart, MCR, READ_UART_REGISTER(uart, MCR) & ~MDCD);
            #else
                AI_SetBit (ARMIO_DCD);
            #endif
        }
    }

#endif /* BOARD 8 or 9 or 40 or 41 or CHIPSET 12 */

    /*
     * Signals are set according to fields SA, SB and X states and flow
     * control:
     *
     *       None    RTS/CTS    XON/XOFF
     * SA    0 (ns)  0 (ns)     0 (ns)
     * SB    DCD     DCD        DCD
     * X     ignore  CTS        XON:0 XOFF:1 (receiver)
     *
     * ns: signal not supported.
     * DCD is supported on C, D & E-Sample.
     */

#if ((BOARD == 8) || (BOARD == 9) || (BOARD == 40) || (BOARD == 41) || (CHIPSET == 12))

    if (mask & (1 << SB)) {

        if (state & (1 << SB)) {
            /* Turn on DCD */
            #if (CHIPSET == 12)
                WRITE_UART_REGISTER (uart, MCR, READ_UART_REGISTER(uart, MCR) | MDCD);
            #else
                AI_ResetBit (ARMIO_DCD);
            #endif
        } else {
            /* Turn off DCD */
            #if (CHIPSET == 12)
                WRITE_UART_REGISTER (uart, MCR, READ_UART_REGISTER(uart, MCR) & ~MDCD);
            #else
                AI_SetBit (ARMIO_DCD);
            #endif
        }
    }

#endif /* BOARD 8 or 9 or 40 or 41 or CHIPSET 12 */
        
    if ((mask & (1 << X)) &&
        (uart->flow_control_mode != fc_none)) {

        if (state & (1 << X)) {
                
            if (!uart->rx_stopped_by_application) {
                
                if (!uart->rx_stopped_by_driver)
                    stop_receiver (uart);

                uart->rx_stopped_by_application = 1;
            }
                
        } else {
                
            /*
             * The receiver is started only if it is not stopped by the driver
             * and if it is stopped by the application.
             */

            if ((!uart->rx_stopped_by_driver) &&
                uart->rx_stopped_by_application)
                start_receiver (uart);

            uart->rx_stopped_by_application = 0;
        }
    }

#if ((CHIPSET != 5) && (CHIPSET != 6))
    /*
     * Re-enable sleep mode.
     */
    /* BELOW LINES WERE COMMENTED TO DISABLE SLEEP MODE IN DRIVER */ 
    /*
    WRITE_UART_REGISTER (
        uart, IER, READ_UART_REGISTER (uart, IER) | IER_SLEEP);
        */
#endif

    return (FD_OK);
}

/*******************************************************************************
 *
 *                           UAF_InterruptHandler
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
UAF_InterruptHandler (T_fd_UartId uart_id,
                      SYS_UWORD8 interrupt_status)
{

    t_uart *uart;

    /*
     * uart_id is not used.
     */

    uart = &uart_parameters;

    uart->current_time = NU_Retrieve_Clock ();

    /*
     * Causes of interrupt:
     *      - trigger level reached,
     *      - character time-out indication,
     *      - transmitter holding register empty,
     *      - modem status.
     */

    switch (interrupt_status) {

    case RX_DATA:

#if TARGET_HAS_DTR_INPUT
        uart->index_it = (uart->index_it + 1) & 0x01; /* 0 or 1 */
        uart->dtr_change_detected[uart->index_it] = 0;
        uart->dtr_level_saved[uart->index_it] = uart->dtr_level;
#endif
        read_rx_fifo (uart);

        // Disable Autobaud and lock baudrate upon first received character
        if (uart->autobauding != 0)
        {
          /* MSMSMSMS */
          SYS_UWORD8 uasr;
          T_baudrate baudrate;
          T_bitsPerCharacter bpc;
          T_parity parity;

          WRITE_UART_REGISTER (uart, LCR, READ_UART_REGISTER (uart, LCR) | DLAB);


          uasr = READ_UART_REGISTER (uart, UASR);

          switch (uasr & 0x1F)
          {
            case 0x01: baudrate = FD_BAUD_115200; break;
            case 0x02: baudrate = FD_BAUD_57600; break;
            case 0x03: baudrate = FD_BAUD_38400; break;
            case 0x04: baudrate = FD_BAUD_28800; break;
            case 0x05: baudrate = FD_BAUD_19200; break;
            case 0x06: baudrate = FD_BAUD_14400; break;
            case 0x07: baudrate = FD_BAUD_9600; break;
            case 0x08: baudrate = FD_BAUD_4800; break;
            case 0x09: baudrate = FD_BAUD_2400; break;
            case 0x0A: baudrate = FD_BAUD_1200; break;
            default: /* no baudrate detected, abort for now */
              return;
          }

          switch (uasr>>5 & 0x01)
          {
            case 0x00: bpc = bpc_7; break;
            case 0x01: bpc = bpc_8; break;
          }

          switch (uasr>>6 & 0x03)
          {
            case 0x00: parity = pa_none; break;
            case 0x01: parity = pa_space; break;
            case 0x02: parity = pa_even; break;
            case 0x03: parity = pa_odd; break;
          }

          UAF_SetComPar (UAF_UART_1,
                         baudrate,
                         bpc,
                         sb_1,
                         parity);

          uart->baudrate = baudrate_value[FD_BAUD_AUTO]; /* remember autobauding */
        }

        break;

    case TX_EMPTY:

        fill_tx_fifo (uart);
        break;

    case MODEM_STATUS:

        check_v24_input_lines (uart);
        break;
    }
}

/*******************************************************************************
 *
 *                              UAF_CheckXEmpty
 * 
 * Purpose  : Checks the empty condition of the Transmitter.
 *
 * Arguments: In : uartNo: Used UART.
 *            Out: none
 *
 * Returns  : FD_OK           : Empty condition OK.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_NOT_READY    : Empty condition not OK.
 *            FD_INTERNAL_ERR : Internal problem with the hardware.
 *
 ******************************************************************************/

T_FDRET
UAF_CheckXEmpty (T_fd_UartId uartNo)
{
    T_FDRET result;
    t_uart *uart;
    SYS_UWORD8 status;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    /*
     * There is no case where FD_INTERNAL_ERR may be returned.
     */

    result = FD_OK;
    
    uart = &uart_parameters;
    status = READ_UART_REGISTER (uart, LSR);

    /*
     * Checks if:
     *     - the TX SW Buffer is empty,
     *     - the TX HW FIFO is empty (THRE),
     *     - the Transmitter Shift Register is empty (TEMT).
     */

    if (!(TX_BUFFER_EMPTY (uart)) ||
        !(status & THRE) ||
        !(status & TEMT))

        result = FD_NOT_READY;

    return (result);
}

#if TARGET_HAS_DTR_INPUT
/*******************************************************************************
 *
 *                              UAF_DTRInterruptHandler
 *
 * Purpose  : This function is only used on C & D-Sample. On this platform, the
 *            DTR signal is controlled with an I/O. A change of state of this
 *            signal is detected with an interrupt. This function is called when
 *            this interrupt occurs.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

void
UAF_DTRInterruptHandler (void)
{
    t_uart *uart;
    
    uart = &uart_parameters;
    
    /*
     * Read the state of DTR and change the edge to detect the next change
     * of DTR.
     */
     
    uart->dtr_level = AI_ReadBit (ARMIO_DTR);
    
    if (uart->dtr_level)
    	{
        AI_SelectIOForIT (ARMIO_DTR, ARMIO_FALLING_EDGE);
	 if (uart->flow_control_mode != fc_dtr  && uart->baudrate == baudrate_value[FD_BAUD_AUTO])
        UAF_SetComPar (UAF_UART_1, FD_BAUD_AUTO, bpc_8, sb_1, pa_none);
       }
    else
        AI_SelectIOForIT (ARMIO_DTR, ARMIO_RISING_EDGE);
        
    /*
     * The reading callback function has to be called. But bytes received before
     * the change of state of DTR must be copied into the RX buffer before to
     * call it.
     */
     
    if (READ_UART_REGISTER (uart, LSR) & DR) { /* If Rx FIFO is not empty */
    
        /*
         * The Rx FIFO will be read to fill one of the two buffers and the Rx
         * HISR will be activated.
         */
       
        uart->index_it = (uart->index_it + 1) & 0x01; /* 0 or 1 */
        uart->dtr_change_detected[uart->index_it] = 1;
        uart->dtr_level_saved[uart->index_it] = uart->dtr_level;
        read_rx_fifo (uart);
        
    } else
        (void) NU_Activate_HISR (&(uart->v24_hisr_ctrl_block));
    
}
#endif /* BOARD 8 or 9 or 40 or 41 */

#if (defined BTEMOBILE && (CHIPSET != 12))
/*******************************************************************************
 *
 *                                UAF_Exit
 * 
 * Purpose  : 
 *
 * Arguments: In : uartNo: Used UART.
 *            Out: none
 *
 * Returns  : FD_OK           : Successful operation.
 *            FD_NOT_SUPPORTED: Wrong UART number.
 *            FD_INTERNAL_ERR : Internal problem.
 *
 ******************************************************************************/

T_FDRET
UAF_Exit (T_fd_UartId uartNo)
{
    t_uart *uart;

    /*
     * Check UART number.
     * A return is used to simplify the code.
     * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
     * flow control is not supported.
     * DCD and DTR are not supported on UART Irda on C & D-Sample.
     * DCD and DTR are not supported on UART Irda & Modem2 on E-Sample.
     */

    if (uartNo != UAF_UART_1)
        return (FD_NOT_SUPPORTED);

    uart = &uart_parameters;

    /*
     * Delete the 3 HISR actived in the RX/TX and V24 interrupt handlers.
     * A return is used to simplify the code if an error occurs.
     */

    if (NU_Delete_HISR (&(uart->rx_hisr_ctrl_block)) != NU_SUCCESS)
        return (FD_INTERNAL_ERR);

    if (NU_Delete_HISR (&(uart->tx_hisr_ctrl_block)) != NU_SUCCESS)
        return (FD_INTERNAL_ERR);

    if (NU_Delete_HISR (&(uart->v24_hisr_ctrl_block)) != NU_SUCCESS)
        return (FD_INTERNAL_ERR);

    /*
     * Delete the HISR used to send a break.
     * A return is used to simplify the code if an error occurs.
     */

    if (NU_Delete_HISR (&(uart->break_hisr_ctrl_block)) != NU_SUCCESS)
        return (FD_INTERNAL_ERR);

    /*
     * Disable and then delete the timer used in the break HISR
     * A return is used to simplify the code if an error occurs.
     */

    (void) NU_Control_Timer (&(uart->break_timer_ctrl_block),
                             NU_DISABLE_TIMER);

    if (NU_Delete_Timer (&(uart->break_timer_ctrl_block)) != NU_SUCCESS)
        return (FD_INTERNAL_ERR);

    /*
     * Disable and then delete the timer used in the detection of the escape
     * sequence. A return is used to simplify the code if an error occurs.
     */

    (void) NU_Control_Timer (&(uart->guard_period_timer_ctrl_block),
                             NU_DISABLE_TIMER);

    if (NU_Delete_Timer (&(uart->guard_period_timer_ctrl_block)) != NU_SUCCESS)
        return (FD_INTERNAL_ERR);

    /*
     * At that point, all HISRs and Timers have been successfully deleted.
     */

    return (FD_OK);
}
#endif /* (defined BTEMOBILE && (CHIPSET != 12)) */
