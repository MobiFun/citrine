/*******************************************************************************
 *
 * SERIALSWITCH.C
 *
 * This module allows managing the use of the serial ports of TI GSM Evaluation
 * Boards.
 * An application may have to send several serial data flows. The board on which
 * the application is running may have one or several devices. The purpose of
 * this module is to establish connections between the serial data flows and the
 * serial devices at runtime, when the application is started.
 *
 * (C) Texas Instruments 1999 - 2003
 *
 ******************************************************************************/

#define __SERIALSWITCH_C__

#define __STANDARD_H__ /* Avoid to define UBYTE, SYS_UWORD16 and UINT32. */

#include "../include/config.h"
#include "../include/sys_types.h"
#include "../riviera/rv/rv_general.h"
#include "../riviera/rvf/rvf_api.h"
#include "../nucleus/nucleus.h"

#include "serialswitch.h" 

#include "uart.h"
#include "uartfax.h"

#include "../bsp/mem.h" 

#include <string.h> /* needed for memcmp & memset */

#if SERIAL_DYNAMIC_SWITCH
  #include "ffs/ffs.h" 
  #include "rvf/rvf_api.h"
  #include "inth/iq.h"
  #include "rvt/rvt_def_i.h" /* needed for Riviera/Layer1 Trace's callback function */
#endif

#if defined(BTEMOBILE)
  #include "hci_ser.h"
#endif

#define DUMMY_DEVICE (0)

#define IIR (0x02) /* UART interrupt ident. register - Read only  */
#define SCR (0x10) /* UART suppl. control register   - Read/Write */
#define SSR (0x11) /* UART suppl. status register    - Read only  */

/*
 * Interrupt identification register.
 * Bit 0 is set to 0 if an IT is pending.
 * Bits 1 and 2 are used to identify the IT.
 */

#define IIR_BITS_USED  (0x07)
#define IT_NOT_PENDING (0x01)

/*
 * Supplementary Control Register
 */

#define RX_CTS_WAKE_UP_ENABLE_BIT (4)

/*
 * Supplementary Status Register
 */

#define RX_CTS_WAKE_UP_STS (0x02) /* Wake-up interrupt occurred  */

/*
 * This macro allows to read an UART register.
 */

#define READ_UART_REGISTER(UART,REG) \
            *((volatile SYS_UWORD8 *) ((UART)->base_address + (REG)))


/*
 * This macro allows to disable the UART's wake-up interrupt.
 */

#define DISABLE_WAKE_UP_INTERRUPT(UART) \
            *((volatile SYS_UWORD8 *) ((UART)->base_address + SCR)) &= \
                ~(1 << (RX_CTS_WAKE_UP_ENABLE_BIT)); 

/*
 * Wake-up time duration in seconds and in number of TDMAs.
 * 1 TDMA = (6 / 1300) s = 0.004615 s (= 4.615 ms).
 */

#define WAKE_UP_TIME_DURATION (10)  /* 10 seconds */
#define WAKE_UP_TIME_IN_TDMA  (WAKE_UP_TIME_DURATION * 1300 / 6)


/*
 * Global uartswitch variable as read from FFS.
 * It is supposed that NUMBER_OF_TR_UART, NUMBER_OF_FD_UART
 * and NUMBER_OF_BT_UART have the same values.
 */

#define DUMMY             ('0')
#define G23_PANEL         ('G')
#define RIVIERA_TRACE_MUX ('R')
#define FD_AT_COMMAND     ('D')
#define BLUETOOTH_HCI     ('B')

#if (CHIPSET == 12)
  char ser_cfg_info[NUMBER_OF_TR_UART] = {DUMMY, DUMMY, DUMMY};
#else
  char ser_cfg_info[NUMBER_OF_TR_UART] = {DUMMY, DUMMY};
#endif
static SYS_UWORD16 serial_cfg = 0x0048; /* All dummies */

#if SERIAL_DYNAMIC_SWITCH
  /*
   * Global variables used for Dynamic Switch.
   */

  static char ser_new_cfg[NUMBER_OF_TR_UART]  = {DUMMY, DUMMY};
  const static char uart_config_file[] = "/sys/uartswitch";
  static SYS_BOOL dynamic_switch = 0;

  /* Import Serial Info structure. */
  extern T_AppliSerialInfo appli_ser_cfg_info;
#endif

/*
 * Types of flows supported.
 */

typedef enum {
    TRACE_FLOW,
    FAX_DATA_FLOW,
    BLUETOOTH_HCI_FLOW
} t_flow_type;

/*
 * For each serial data flow, a set of function pointers allows calling the
 * functions associated to a serial device.
 */

typedef struct s_tr_functions {

    T_tr_UartId   device;
    
    void (*tr_Init) (T_tr_UartId device,
                     T_tr_Baudrate baudrate,
                     void (callback_function (void)));
                         
    SYS_UWORD32 (*tr_ReadNChars) (T_tr_UartId device,
                                  char *buffer,
                                  SYS_UWORD32 chars_to_read);

    SYS_UWORD32 (*tr_ReadNBytes) (T_tr_UartId device,
                                  char *buffer,
                                  SYS_UWORD32 chars_to_read,
                                  SYS_BOOL *eof_detected);

    SYS_UWORD32 (*tr_WriteNChars) (T_tr_UartId device,
                                   char *buffer,
                                   SYS_UWORD32 chars_to_write);

    SYS_UWORD32 (*tr_EncapsulateNChars) (T_tr_UartId device,
                                         char *buffer,
                                         SYS_UWORD32 chars_to_write);

    SYS_UWORD32 (*tr_WriteNBytes) (T_tr_UartId device,
                                   SYS_UWORD8 *buffer,
                                   SYS_UWORD32 chars_to_write);

    void  (*tr_WriteChar) (T_tr_UartId device,
                           char character);

    void  (*tr_WriteString) (T_tr_UartId device,
                             char *buffer);

    SYS_BOOL (*tr_EnterSleep) (T_tr_UartId device);

    void (*tr_WakeUp) (T_tr_UartId device);

} t_tr_functions;

/*
 * Set of function pointers for fax & data functions.
 */
 
typedef struct s_fd_functions {

    T_fd_UartId   device;
    
    T_FDRET (*fd_Initialize) (T_fd_UartId device);

    T_FDRET (*fd_Enable) (T_fd_UartId device,
                          SYS_BOOL enable);

    T_FDRET (*fd_SetComPar) (T_fd_UartId device,
                             T_baudrate baudrate,
                             T_bitsPerCharacter bpc,
                             T_stopBits sb,
                             T_parity parity);

    T_FDRET (*fd_SetBuffer) (T_fd_UartId device,
                             SYS_UWORD16 bufSize,
                             SYS_UWORD16 rxThreshold,
                             SYS_UWORD16 txThreshold);

    T_FDRET (*fd_SetFlowCtrl) (T_fd_UartId device,
                               T_flowCtrlMode fcMode,
                               SYS_UWORD8 XON,
                               SYS_UWORD8 XOFF);

    T_FDRET (*fd_SetEscape) (T_fd_UartId device,
                             SYS_UWORD8 escChar,
                             SYS_UWORD16 guardPeriod);

    T_FDRET (*fd_InpAvail) (T_fd_UartId device);

    T_FDRET (*fd_OutpAvail) (T_fd_UartId device);

    T_FDRET (*fd_EnterSleep) (T_fd_UartId device);

    T_FDRET (*fd_WakeUp) (T_fd_UartId device);

    T_FDRET (*fd_ReadData) (T_fd_UartId device,
                            T_suspendMode suspend,
                            void (readOutFunc (SYS_BOOL cldFromIrq,
                                               T_reInstMode *reInstall,
                                               SYS_UWORD8 nsource,
                                               SYS_UWORD8 *source[],
                                               SYS_UWORD16 size[],
                                               SYS_UWORD32 state)));

    T_FDRET (*fd_WriteData) (T_fd_UartId device,
                             T_suspendMode suspend,
                             void (writeInFunc (SYS_BOOL cldFromIrq,
                                                T_reInstMode *reInstall,
                                                SYS_UWORD8 ndest,
                                                SYS_UWORD8 *dest[],
                                                SYS_UWORD16 size[])));

    T_FDRET (*fd_StopRec) (T_fd_UartId device);

    T_FDRET (*fd_StartRec) (T_fd_UartId device);

    T_FDRET (*fd_GetLineState) (T_fd_UartId device,
                                SYS_UWORD32 *state);

    T_FDRET (*fd_SetLineState) (T_fd_UartId device,
                                SYS_UWORD32 state,
                                SYS_UWORD32 mask);

    T_FDRET (*fd_CheckXEmpty) (T_fd_UartId device);

} t_fd_functions;

#ifdef BTEMOBILE
  /*
   * Set of function pointers for Bluetooth HCI functions.
   */
   
  typedef struct s_bt_functions {

      T_bt_UartId   device;

      T_HCI_RET (*bt_Init) (T_bt_UartId uart_device);

      T_HCI_RET (*bt_Start) (void);

      T_HCI_RET (*bt_Stop) (void);

      T_HCI_RET (*bt_Kill) (void);

      T_HCI_RET (*bt_SetBaudrate) (UINT8 baudrate);

      T_HCI_RET (*bt_TransmitPacket) (void *uart_tx_buffer);

      SYS_BOOL  (*bt_EnterSleep) (void);

      void      (*bt_WakeUp) (void);

  } t_bt_functions;
#endif

/*
 * Prototypes of dummy functions.
 * Dummy functions for Trace.
 */

static void dummy_tr_Init (T_tr_UartId device,
                           T_tr_Baudrate baudrate,
                           void (callback_function (void)));
 
static SYS_UWORD32 dummy_tr_ReadNChars (T_tr_UartId device,
                                        char *buffer,
                                        SYS_UWORD32 chars_to_read);

static SYS_UWORD32 dummy_tr_ReadNBytes (T_tr_UartId device,
                                        char *buffer,
                                        SYS_UWORD32 chars_to_read,
                                        SYS_BOOL *eof_detected);
 
static SYS_UWORD32 dummy_tr_WriteNChars (T_tr_UartId device,
                                         char *buffer,
                                         SYS_UWORD32 chars_to_write);
 
static SYS_UWORD32 dummy_tr_EncapsulateNChars (T_tr_UartId device,
                                               char *buffer,
                                               SYS_UWORD32 chars_to_write);
 
static SYS_UWORD32 dummy_tr_WriteNBytes (T_tr_UartId device,
                                         SYS_UWORD8 *buffer,
                                         SYS_UWORD32 chars_to_write);
 
static void dummy_tr_WriteChar (T_tr_UartId device,
                                char character);
 
static void dummy_tr_WriteString (T_tr_UartId device,
                                  char *buffer);
 
static SYS_BOOL dummy_tr_EnterSleep (T_tr_UartId device);
 
static void dummy_tr_WakeUp (T_tr_UartId device);
 
/*
 * Dummy functions for Fax & Data.
 */

static T_FDRET dummy_fd_Init (T_fd_UartId device);
 
static T_FDRET dummy_fd_Enable (T_fd_UartId device,
                                SYS_BOOL enable);
 
static T_FDRET dummy_fd_SetComPar (T_fd_UartId device,
                                   T_baudrate baudrate,
                                   T_bitsPerCharacter bpc,
                                   T_stopBits sb,
                                   T_parity parity);
 
static T_FDRET dummy_fd_SetBuffer (T_fd_UartId device,
                                   SYS_UWORD16 bufSize,
                                   SYS_UWORD16 rxThreshold,
                                   SYS_UWORD16 txThreshold);
 
static T_FDRET dummy_fd_SetFlowCtrl (T_fd_UartId device,
                                     T_flowCtrlMode fcMode,
                                     SYS_UWORD8 XON,
                                     SYS_UWORD8 XOFF);
 
static T_FDRET dummy_fd_SetEscape (T_fd_UartId device,
                                   SYS_UWORD8 escChar,
                                   SYS_UWORD16 guardPeriod);
 
static T_FDRET dummy_fd_InpAvail (T_fd_UartId device);
 
static T_FDRET dummy_fd_OutpAvail (T_fd_UartId device);
 
static T_FDRET dummy_fd_EnterSleep (T_fd_UartId device);
 
static T_FDRET dummy_fd_WakeUp (T_fd_UartId device);
 
static T_FDRET dummy_fd_ReadData (T_fd_UartId device,
                                  T_suspendMode suspend,
                                  void (readOutFunc (SYS_BOOL cldFromIrq,
                                                     T_reInstMode *reInstall,
                                                     SYS_UWORD8 nsource,
                                                     SYS_UWORD8 *source[],
                                                     SYS_UWORD16 size[],
                                                     SYS_UWORD32 state)));
 
static T_FDRET dummy_fd_WriteData (T_fd_UartId device,
                                   T_suspendMode suspend,
                                   void (writeInFunc (SYS_BOOL cldFromIrq,
                                                      T_reInstMode *reInstall,
                                                      SYS_UWORD8 ndest,
                                                      SYS_UWORD8 *dest[],
                                                      SYS_UWORD16 size[])));
 
static T_FDRET dummy_fd_StopRec (T_fd_UartId device);
 
static T_FDRET dummy_fd_StartRec (T_fd_UartId device);
 
static T_FDRET dummy_fd_GetLineState (T_fd_UartId device,
                                      SYS_UWORD32 *state);
 
static T_FDRET dummy_fd_SetLineState (T_fd_UartId device,
                                      SYS_UWORD32 state,
                                      SYS_UWORD32 mask);
 
static T_FDRET dummy_fd_CheckXEmpty (T_fd_UartId device);
 
#ifdef BTEMOBILE
  /*
   * Dummy functions for Bluetooth HCI.
   */

  static T_HCI_RET dummy_bt_Init (T_bt_UartId uart_device);
   
  static T_HCI_RET dummy_bt_Start (void);
   
  static T_HCI_RET dummy_bt_Stop (void);
   
  static T_HCI_RET dummy_bt_Kill (void);
   
  static T_HCI_RET dummy_bt_SetBaudrate (UINT8 baudrate);
   
  static T_HCI_RET dummy_bt_TransmitPacket (void *uart_tx_buffer);

  static SYS_BOOL  dummy_bt_EnterSleep (void);
  
  static void      dummy_bt_WakeUp (void);

#endif

/*
 * Constants tables representing the various possible configurations
 * for Trace, Fax & Data and Bluetooth HCI according to the different devices.
 * Constant table for Trace using no device.
 */

static const t_tr_functions dummy_trace = {

    DUMMY_DEVICE,
    dummy_tr_Init,
    dummy_tr_ReadNChars,
    dummy_tr_ReadNBytes,
    dummy_tr_WriteNChars,
    dummy_tr_EncapsulateNChars,
    dummy_tr_WriteNBytes,
    dummy_tr_WriteChar,
    dummy_tr_WriteString,
    dummy_tr_EnterSleep,
    dummy_tr_WakeUp
};

/*
 * Constant table for Trace using UART IrDA.
 */

static const t_tr_functions uart_irda_trace = {

    UA_UART_0,
    UA_Init,
    UA_ReadNChars,
    UA_ReadNBytes,
    UA_WriteNChars,
    UA_EncapsulateNChars,
    UA_WriteNBytes,
    UA_WriteChar,
    UA_WriteString,
    UA_EnterSleep,
    UA_WakeUp
};

/*
 * Constant table for Trace using UART Modem.
 */

static const t_tr_functions uart_modem_trace = {

    UA_UART_1,
    UA_Init,
    UA_ReadNChars,
    UA_ReadNBytes,
    UA_WriteNChars,
    UA_EncapsulateNChars,
    UA_WriteNBytes,
    UA_WriteChar,
    UA_WriteString,
    UA_EnterSleep,
    UA_WakeUp
};

#if (CHIPSET == 12)
  /*
   * Constant table for Trace using UART Modem2.
   */

  static const t_tr_functions uart_modem2_trace = {

      UA_UART_2,
      UA_Init,
      UA_ReadNChars,
      UA_ReadNBytes,
      UA_WriteNChars,
      UA_EncapsulateNChars,
      UA_WriteNBytes,
      UA_WriteChar,
      UA_WriteString,
      UA_EnterSleep,
      UA_WakeUp
  };
#endif

/*
 * Constant table for Fax & Data using no device.
 */

static const t_fd_functions dummy_fax_data = {

    DUMMY_DEVICE,
    dummy_fd_Init,
    dummy_fd_Enable,
    dummy_fd_SetComPar,
    dummy_fd_SetBuffer,
    dummy_fd_SetFlowCtrl,
    dummy_fd_SetEscape,
    dummy_fd_InpAvail,
    dummy_fd_OutpAvail,
    dummy_fd_EnterSleep,
    dummy_fd_WakeUp,
    dummy_fd_ReadData,
    dummy_fd_WriteData,
    dummy_fd_StopRec,
    dummy_fd_StartRec,
    dummy_fd_GetLineState,
    dummy_fd_SetLineState,
    dummy_fd_CheckXEmpty
};

/*
 * Constant table for Fax & Data using UART Modem.
 */

#if CONFIG_FDMODEM
static const t_fd_functions uart_modem_fax_data = {

    UAF_UART_1,
    UAF_Init,
    UAF_Enable,
    UAF_SetComPar,
    UAF_SetBuffer,
    UAF_SetFlowCtrl,
    UAF_SetEscape,
    UAF_InpAvail,
    UAF_OutpAvail,
    UAF_EnterSleep,
    UAF_WakeUp,
    UAF_ReadData,
    UAF_WriteData,
    UAF_StopRec,
    UAF_StartRec,
    UAF_GetLineState,
    UAF_SetLineState,
    UAF_CheckXEmpty
};
#endif

#ifdef BTEMOBILE
  /*
   * Constant table for BT HCI using no device.
   */

  static const t_bt_functions dummy_bt_hci = {

      DUMMY_DEVICE,
      dummy_bt_Init,
      dummy_bt_Start,
      dummy_bt_Stop,
      dummy_bt_Kill,
      dummy_bt_SetBaudrate,
      dummy_bt_TransmitPacket,
      dummy_bt_EnterSleep,
      dummy_bt_WakeUp
  };

  /*
   * Constant table for BT HCI using UART IrDA.
   */

  static const t_bt_functions uart_irda_bt_hci = {

      UABT_UART_0,
      hciu_init,
      hciu_start,
      hciu_stop,
      hciu_kill,
      hciu_set_baudrate,
      hciu_transmit_packet,
      hciu_enter_sleep,
      hciu_wakeup
  };

  /*
   * Constant table for BT HCI using UART Modem.
   */

  static const t_bt_functions uart_modem_bt_hci = {

      UABT_UART_1,
      hciu_init,
      hciu_start,
      hciu_stop,
      hciu_kill,
      hciu_set_baudrate,
      hciu_transmit_packet,
      hciu_enter_sleep,
      hciu_wakeup
  };

  #if (CHIPSET == 12)
    /*
     * Constant table for BT HCI using UART Modem2.
     */

    static const t_bt_functions uart_modem2_bt_hci = {

        UABT_UART_2,
        hciu_init,
        hciu_start,
        hciu_stop,
        hciu_kill,
        hciu_set_baudrate,
        hciu_transmit_packet,
        hciu_go_to_sleep,
        hciu_wakeup
    };
  #endif
#endif

#if SERIAL_DYNAMIC_SWITCH
  /*
   * Structure used to store initialization parameters related to the AT-Cmd/F&D flow.
   * Numbers of paramaters (in case of multiple calls) have been figured out from
   * Condat AT-Command/F&D flow initialization.
   */
   
  typedef struct s_data_flow {
    
      /*
       * Parameters related to SER_fd_SetComPar (2 calls)
       */
      T_baudrate         baudrate[2];
      T_bitsPerCharacter bpc[2];
      T_stopBits         sb[2];
      T_parity           parity[2];

      /*
       * Parameters related to SER_fd_SetBuffer
       */
      SYS_WORD16         bufSize;
      SYS_WORD16         rxThreshold;
      SYS_WORD16         txThreshold;

      /*
       * Parameters related to SER_fd_SetFlowCtrl (2 calls)
       */
      T_flowCtrlMode     fcMode[2];
      SYS_UWORD8         XON[2];
      SYS_UWORD8         XOFF[2];

      /*
       * Parameters related to SER_fd_SetEscape (2 calls)
       */
      SYS_UWORD8         escChar[2];
      SYS_UWORD16        guardPeriod[2];

      /*
       * Parameters related to SER_fd_SetLineState (4 calls)
       */
      SYS_UWORD32        state[4];
      SYS_UWORD32        mask[4];

      /*
       * Parameters related to SER_fd_ReadData
       */
      T_suspendMode      suspend_rd;
      void               (*readOutFunc) (SYS_BOOL cldFromIrq,
                                         T_reInstMode *reInstall,
                                         SYS_UWORD8 nsource,
                                         SYS_UWORD8 *source[],
                                         SYS_UWORD16 size[],
                                         SYS_UWORD32 state);
      /*
       * Parameters related to SER_fd_WriteData
       */
      T_suspendMode      suspend_wr;
      void               (*writeInFunc) (SYS_BOOL cldFromIrq,
                                         T_reInstMode *reInstall,
                                         SYS_UWORD8 ndest,
                                         SYS_UWORD8 *dest[],
                                         SYS_UWORD16 size[]);

  } t_data_flow;
#endif /* (defined BTEMOBILE && (CHIPSET != 12)) */

/*
 * UART structure used for UARTs.
 */
 
typedef struct s_uart {
    
    SYS_UWORD32 base_address;
    SYS_BOOL    device_used;
    SYS_BOOL    deep_sleep_set_up;
    t_flow_type flow_type;
    SYS_WORD16  flow_id;
    void (*interrupt_handler) (int uart_id,
                               SYS_UWORD8 interrupt_status);
    
} t_uart;

static const t_tr_functions *tr_functions[SER_MAX_NUMBER_OF_FLOWS];
static const t_fd_functions *fd_functions;

#ifdef BTEMOBILE
  static const t_bt_functions *bt_functions;
#endif

#if SERIAL_DYNAMIC_SWITCH
  static SYS_BOOL uart_fd_initialized = 0;
#endif

static SYS_UWORD8  fd_buffer[FD_MAX_BUFFER_SIZE];
static SYS_BOOL    fd_driver_enabled;

#if SERIAL_DYNAMIC_SWITCH
  static t_data_flow data_flow_parameters;
#else
  static SYS_WORD16 bufSize;
#endif

#if SERIAL_DYNAMIC_SWITCH
  /*
   * Variables used to count calls to SER_fd_XXX functions.
   */

  static SYS_UWORD8 fd_UAF_SetBuffer = 0;
  static SYS_UWORD8 fd_UAF_SetEscape = 0;
  static SYS_UWORD8 fd_UAF_SetComPar = 0;
  static SYS_UWORD8 fd_UAF_SetFlowCtrl = 0;
  static SYS_UWORD8 fd_UAF_ReadData = 0;
  static SYS_UWORD8 fd_UAF_SetLineState = 0;
  static SYS_UWORD8 fd_UAF_WriteData = 0;
#endif

/*
 * Timer used for duration control when UARTs are waked up by an interrupt or
 * each time any new incoming characters are received; This timer prevents the
 * system to enter deep sleep mode.
 */

static NU_TIMER uart_sleep_timer;
       SYS_BOOL uart_sleep_timer_enabled;

/*
 * HISR used to reset and restart the sleep timer from an UART use by a Trace
 * flow in case of incoming characters.
 */

#define TIMER_HISR_PRIORITY      (2)
#define TIMER_HISR_STACK_SIZE  (512) /* Bytes. */ 

static NU_HISR timer_hisr_ctrl_block;
static char    timer_hisr_stack[TIMER_HISR_STACK_SIZE];

/*
 * For next arrays, it is supposed that NUMBER_OF_TR_UART, NUMBER_OF_FD_UART
 * and NUMBER_OF_BT_UART have the same values.
 * An index on an internal uart for trace, fax & data or bluetooth hci reffers
 * to the same uart device.
 */
 
static t_uart int_uart[NUMBER_OF_TR_UART];

#if ((CHIPSET == 2) || (CHIPSET == 3))
  static SYS_UWORD32 uart_spurious_interrupts;
#elif ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  static SYS_UWORD32 uart_modem_spurious_interrupts;
  static SYS_UWORD32 uart_irda_spurious_interrupts;
#endif
#if (CHIPSET == 12)
  static SYS_UWORD32 uart_modem2_spurious_interrupts;
#endif

static const SYS_UWORD32 uart_base_address[NUMBER_OF_TR_UART] =
{
    MEM_UART_IRDA,
    MEM_UART_MODEM
    #if (CHIPSET == 12)
      , MEM_UART_MODEM2
    #endif
};

 
/*******************************************************************************
 *
 *                              dummy_tr_Init
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_Init.
 *
 * Return: none
 *
 ******************************************************************************/
 
static void
dummy_tr_Init (T_tr_UartId device,
               T_tr_Baudrate baudrate,
               void (callback_function (void)))
{
    /*
     * No action.
     */
}

/*******************************************************************************
 *
 *                          dummy_tr_ReadNChars
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_ReadNChars.
 *
 * Return: 0
 *
 ******************************************************************************/
 
static SYS_UWORD32
dummy_tr_ReadNChars (T_tr_UartId device,
                     char *buffer,
                     SYS_UWORD32 chars_to_read)
{
    return (0);
}

/*******************************************************************************
 *
 *                          dummy_tr_ReadNBytes
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_ReadNBytes.
 *
 * Return: 0
 *
 ******************************************************************************/
 
static SYS_UWORD32
dummy_tr_ReadNBytes (T_tr_UartId device,
                     char *buffer,
                     SYS_UWORD32 chars_to_read,
                     SYS_BOOL *eof_detected)
{
    return (0);
}

/*******************************************************************************
 *
 *                          dummy_tr_WriteNChars
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_WriteNChars.
 *
 * Return: The number of character to write.
 *
 ******************************************************************************/
 
static SYS_UWORD32
dummy_tr_WriteNChars (T_tr_UartId device,
                      char *buffer,
                      SYS_UWORD32 chars_to_write)
{
    return (chars_to_write);
}

/*******************************************************************************
 *
 *                          dummy_tr_EncapsulateNChars
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_EncapsulateNChars.
 *
 * Return: The number of character to write.
 *
 ******************************************************************************/
 
static SYS_UWORD32
dummy_tr_EncapsulateNChars (T_tr_UartId device,
                      char *buffer,
                      SYS_UWORD32 chars_to_write)
{
    return (chars_to_write);
}

/*******************************************************************************
 *
 *                          dummy_tr_WriteNBytes
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_WriteNBytes.
 *
 * Return: The number of byte to write.
 *
 ******************************************************************************/
 
static SYS_UWORD32
dummy_tr_WriteNBytes (T_tr_UartId device,
                      SYS_UWORD8 *buffer,
                      SYS_UWORD32 chars_to_write)
{
    return (chars_to_write);
}

/*******************************************************************************
 *
 *                              dummy_tr_WriteChar
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_WriteChar.
 *
 * Return: none
 *
 ******************************************************************************/
 
static void
dummy_tr_WriteChar (T_tr_UartId device,
                    char character)
{
    /*
     * No action.
     */
}

/*******************************************************************************
 *
 *                          dummy_tr_WriteString
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_WriteString.
 *
 * Return: none
 *
 ******************************************************************************/
 
static void
dummy_tr_WriteString (T_tr_UartId device,
                      char *buffer)
{
    /*
     * No action.
     */
}

/*******************************************************************************
 *
 *                          dummy_tr_EnterSleep
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_EnterSleep.
 *
 * Return: 1
 *
 ******************************************************************************/
 
static SYS_BOOL
dummy_tr_EnterSleep (T_tr_UartId device)
{
    return (1);
}

/*******************************************************************************
 *
 *                           dummy_tr_WakeUp
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_WakeUp.
 *
 * Return: none
 *
 ******************************************************************************/
 
static void
dummy_tr_WakeUp (T_tr_UartId device)
{
    /*
     * No action.
     */
}

/*******************************************************************************
 *
 *                              dummy_fd_Init
 *
 * Purpose: Sets the size of the circular buffer to the maximum value and the
 *          state of the driver to 'disabled'.
 *
 * Parameters: See SER_fd_Init.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_Init (T_fd_UartId device)
{
    #if SERIAL_DYNAMIC_SWITCH
      data_flow_parameters.bufSize = FD_MAX_BUFFER_SIZE;
    #else
      bufSize = FD_MAX_BUFFER_SIZE;
    #endif
    fd_driver_enabled = 0;

    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_Enable
 *
 * Purpose: Stores the state of the driver.
 *
 * Parameters: See SER_fd_Enable.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_Enable (T_fd_UartId device,
                 SYS_BOOL enable)
{
    fd_driver_enabled = enable;
    
    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_SetComPar
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_SetComPar.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_SetComPar (T_fd_UartId device,
                    T_baudrate baudrate,
                    T_bitsPerCharacter bpc,
                    T_stopBits sb,
                    T_parity parity)
{
    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_SetBuffer
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_SetBuffer.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_SetBuffer (T_fd_UartId device,
                    SYS_UWORD16 bufSize,
                    SYS_UWORD16 rxThreshold,
                    SYS_UWORD16 txThreshold)
{
    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_SetFlowCtrl
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_SetFlowCtrl.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_SetFlowCtrl (T_fd_UartId device,
                      T_flowCtrlMode fcMode,
                      SYS_UWORD8 XON,
                      SYS_UWORD8 XOFF)
{
    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_SetEscape
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_SetEscape.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_SetEscape (T_fd_UartId device,
                    SYS_UWORD8 escChar,
                    SYS_UWORD16 guardPeriod)
{
    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_InpAvail
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_InpAvail.
 *
 * Return: The size of the circular buffer.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_InpAvail (T_fd_UartId device)
{
    #if SERIAL_DYNAMIC_SWITCH
      return (data_flow_parameters.bufSize);
    #else
      return (bufSize);
    #endif
}

/*******************************************************************************
 *
 *                              dummy_fd_OutpAvail
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_OutpAvail.
 *
 * Return: The size of the circular buffer.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_OutpAvail (T_fd_UartId device)
{
    #if SERIAL_DYNAMIC_SWITCH
      return (data_flow_parameters.bufSize);
    #else
      return (bufSize);
    #endif
}

/*******************************************************************************
 *
 *                          dummy_fd_EnterSleep
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_EnterSleep.
 *
 * Return: 1
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_EnterSleep (T_fd_UartId device)
{
    return (1);
}

/*******************************************************************************
 *
 *                           dummy_fd_WakeUp
 *
 * Purpose: No action.
 *
 * Parameters: See SER_tr_WakeUp.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_WakeUp (T_fd_UartId device)
{
    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_ReadData
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_ReadData.
 *
 * Return: 0 if the suspend parameter is set to 'sm_noSuspend'.
 *         FD_SUSPENDED if the suspend parameter is set to 'sm_suspend'.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_ReadData (T_fd_UartId device,
                   T_suspendMode suspend,
                   void (readOutFunc (SYS_BOOL cldFromIrq,
                                      T_reInstMode *reInstall,
                                      SYS_UWORD8 nsource,
                                      SYS_UWORD8 *source[],
                                      SYS_UWORD16 size[],
                                      SYS_UWORD32 state)))
{
    T_FDRET result;

    if (suspend == sm_noSuspend)
        result = 0;
    else
        result = FD_SUSPENDED;
        
    return (result);
}

/*******************************************************************************
 *
 *                              dummy_fd_WriteData
 *
 * Purpose: The user's function is called with:
 *            - cldFromIrq = 0
 *            - ndest = 1
 *            - dest[0] is a SYS_UWORD8 pointer on the beginning address of a local
 *              buffer
 *            - size[0] is set to data_flow_parameters.bufSize.
 *
 * Parameters: See SER_fd_WriteData.
 *
 * Return: The number of bytes written in the local buffer.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_WriteData (T_fd_UartId device,
                    T_suspendMode suspend,
                    void (writeInFunc (SYS_BOOL cldFromIrq,
                                       T_reInstMode *reInstall,
                                       SYS_UWORD8 ndest,
                                       SYS_UWORD8 *dest[],
                                       SYS_UWORD16 size[])))
{
    T_reInstMode dummyInstall;
    SYS_UWORD8   *destination[2];
    SYS_UWORD16  buffer_size[2];

    destination[0] = &(fd_buffer[0]);
    #if SERIAL_DYNAMIC_SWITCH
      buffer_size[0] = data_flow_parameters.bufSize;
    #else
      buffer_size[0] = bufSize;
    #endif

    (*writeInFunc) (0, &dummyInstall, 1, &(destination[0]), &(buffer_size[0]));

    #if SERIAL_DYNAMIC_SWITCH
      return ((T_FDRET) (data_flow_parameters.bufSize - buffer_size[0]));
    #else
      return ((T_FDRET) (bufSize - buffer_size[0]));
    #endif
}

/*******************************************************************************
 *
 *                              dummy_fd_StopRec
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_StopRec.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_StopRec (T_fd_UartId device)
{
    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_StartRec
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_StartRec.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_StartRec (T_fd_UartId device)
{
    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_GetLineState
 *
 * Purpose: Sets the RXBLEV field to the bufSize value.
 *
 * Parameters: See SER_fd_GetLineState.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_GetLineState (T_fd_UartId device,
                       SYS_UWORD32 *state)
{
    #if SERIAL_DYNAMIC_SWITCH
      *state = data_flow_parameters.bufSize << RXBLEV;
    #else
      *state = bufSize << RXBLEV;
    #endif
    
    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_SetLineState
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_SetLineState.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_SetLineState (T_fd_UartId device,
                       SYS_UWORD32 state,
                       SYS_UWORD32 mask)
{
    return (FD_OK);
}

/*******************************************************************************
 *
 *                              dummy_fd_CheckXEmpty
 *
 * Purpose: No action.
 *
 * Parameters: See SER_fd_CheckXEmpty.
 *
 * Return: FD_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_FDRET
dummy_fd_CheckXEmpty (T_fd_UartId device)
{
    return (FD_OK);
}

#ifdef BTEMOBILE
/*******************************************************************************
 *
 *                              dummy_bt_Init
 *
 * Purpose: No action.
 *
 * Parameters: See SER_bt_Init.
 *
 * Return: HCI_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_HCI_RET
dummy_bt_Init (T_bt_UartId uart_device)
{
    return (HCI_OK);
}

/*******************************************************************************
 *
 *                              dummy_bt_Start
 *
 * Purpose: No action.
 *
 * Parameters: See SER_bt_Start.
 *
 * Return: HCI_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_HCI_RET
dummy_bt_Start (void)
{
    return (HCI_OK);
}

/*******************************************************************************
 *
 *                              dummy_bt_Stop
 *
 * Purpose: No action.
 *
 * Parameters: See SER_bt_Stop.
 *
 * Return: HCI_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_HCI_RET
dummy_bt_Stop (void)
{
    return (HCI_OK);
}

/*******************************************************************************
 *
 *                              dummy_bt_Kill
 *
 * Purpose: No action.
 *
 * Parameters: See SER_bt_Kill.
 *
 * Return: HCI_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_HCI_RET
dummy_bt_Kill (void)
{
    return (HCI_OK);
}

/*******************************************************************************
 *
 *                           dummy_bt_SetBaudrate
 *
 * Purpose: No action.
 *
 * Parameters: See SER_bt_SetBaudrate.
 *
 * Return: HCI_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_HCI_RET
dummy_bt_SetBaudrate (UINT8 baudrate)
{
    return (HCI_OK);
}

/*******************************************************************************
 *
 *                         dummy_bt_TransmitPacket
 *
 * Purpose: No action.
 *
 * Parameters: See SER_bt_TransmitPacket.
 *
 * Return: HCI_OK: Successful operation.
 *
 ******************************************************************************/
 
static T_HCI_RET dummy_bt_TransmitPacket (void *uart_tx_buffer)

{
    return (HCI_OK);
}

/*******************************************************************************
 *
 *                              dummy_bt_EnterSleep
 *
 * Purpose: No action.
 *
 * Parameters: See SER_bt_EnterSleep.
 *
 * Return: TRUE.
 *
 ******************************************************************************/ 

static SYS_BOOL
dummy_bt_EnterSleep (void)
{
    return (TRUE);
}

/*******************************************************************************
 *
 *                              dummy_bt_WakeUp
 *
 * Purpose: No action.
 *
 * Parameters: See SER_bt_WakeUp
 *
 * Return: HCI_OK: none
 *
 ******************************************************************************/ 

static void
dummy_bt_WakeUp (void)
{
    /*
     * No action.
     */
}

#endif /* BTEMOBILE */

/*******************************************************************************
 *
 *                     analyze_uart_sleep_timer_expiration
 * 
 * Purpose  : The timer has just expired. If requested, UARTs can again be set
 *            up to enter Deep Sleep.
 *
 * Arguments: In : id: parameter not used.
 *            Out: none
 *
 * Returns  : none 
 *
 ******************************************************************************/

static VOID
analyze_uart_sleep_timer_expiration (UNSIGNED id)
{
    /*
     * Timer has expired.
     * UARTs can again be set up for Deep Sleep.
     */

    (void) NU_Control_Timer (&uart_sleep_timer,
                             NU_DISABLE_TIMER);
      
    uart_sleep_timer_enabled = 0;
}

/*******************************************************************************
 *
 *                          start_uart_sleep_timer
 * 
 * Purpose  : Starts the sleep timer once UARTs have been waked-up by an
 *            interrupt or if new incoming characters have been received.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none 
 *
 ******************************************************************************/

static void
start_uart_sleep_timer (void)
{
    /*
     * UART sleep timer is started.
     * UARTs can't no more be set up for Deep Sleep until the timer expires.
     */

    (void) NU_Reset_Timer (&uart_sleep_timer,
                           &analyze_uart_sleep_timer_expiration,
                           WAKE_UP_TIME_IN_TDMA,
                           0, /* The timer expires once. */
                           NU_DISABLE_TIMER);

    (void) NU_Control_Timer (&uart_sleep_timer,
                             NU_ENABLE_TIMER);
}

/*******************************************************************************
 *
 *                              set_flow_functions
 *
 * Purpose: Initializes a serial data flow functions set with the set of
 *          functions of the selected device.
 *
 * Parameters: In : flow         : index of the serial data flow
 *                  serial_driver: allows knowing which set of functions must
 *                                 be selected
 *             Out: none
 *
 * Return: none
 *
 ******************************************************************************/

static void
set_flow_functions (int flow,
                    T_SerialDriver serial_driver)
{

    switch (serial_driver) {

    case UART_MODEM_FAX_DATA:

#if CONFIG_FDMODEM
        fd_functions = &uart_modem_fax_data;
        int_uart[fd_functions->device].device_used = 1;
        int_uart[fd_functions->device].flow_type   = FAX_DATA_FLOW;
        int_uart[fd_functions->device].flow_id     = flow;
        int_uart[fd_functions->device].interrupt_handler =
                                           UAF_InterruptHandler;
        break;
#endif

    case DUMMY_FAX_DATA:
        
        fd_functions = &dummy_fax_data;                               
        break;


    case UART_IRDA_TRACE:
    case UART_MODEM_TRACE:
    #if (CHIPSET == 12)
      case UART_MODEM2_TRACE:
    #endif

        if (serial_driver == UART_IRDA_TRACE)
            tr_functions[flow] = &uart_irda_trace;
        else {
          #if (CHIPSET == 12)
            if (serial_driver == UART_MODEM2_TRACE)
                tr_functions[flow] = &uart_modem2_trace;
            else
          #endif
                tr_functions[flow] = &uart_modem_trace;
        }

        int_uart[tr_functions[flow]->device].device_used = 1;
        int_uart[tr_functions[flow]->device].flow_type   = TRACE_FLOW;
        int_uart[tr_functions[flow]->device].flow_id     = flow;
        int_uart[tr_functions[flow]->device].interrupt_handler =
                                      UA_InterruptHandler;
        break;

    case DUMMY_TRACE:

        tr_functions[flow] = &dummy_trace;
        break;

    case DUMMY_BT_HCI:

        /*
         * if serial_driver = DUMMY_BT_HCI & if BTEMOBILE is not defined
         * no action is performed.
         */

#ifdef BTEMOBILE
        bt_functions = &dummy_bt_hci;                               
        break;

    case UART_IRDA_BT_HCI:
    case UART_MODEM_BT_HCI:
    #if (CHIPSET == 12)
      case UART_MODEM2_BT_HCI:
    #endif

        if (serial_driver == UART_IRDA_BT_HCI)
            bt_functions = &uart_irda_bt_hci;
        else {
          #if (CHIPSET == 12)
            if (serial_driver == UART_MODEM2_BT_HCI)
                bt_functions = &uart_modem2_bt_hci;
            else
          #endif
                bt_functions = &uart_modem_bt_hci;
        }

        int_uart[bt_functions->device].device_used = 1;
        int_uart[bt_functions->device].flow_type   = BLUETOOTH_HCI_FLOW;
        int_uart[bt_functions->device].flow_id     = flow;
        int_uart[bt_functions->device].interrupt_handler =
                                             hciu_interrupt_handler;
#endif /* BTEMOBILE */
          break;
    }
}

/*******************************************************************************
 *
 *                          SER_InitSerialConfig
 *
 * Purpose: The parameter serial_info allows knowing all serial information
 *          necessary to set up the serial configuration of an application.
 *          From this information, the function is able to determine if the
 *          current serial configuration read out from the flash memory is
 *          valid. If it does not correspond to an allowed configuration, the
 *          default configuration is selected. This function must be called at
 *          the application's initialization, but never after.
 *
 * Parameters: In : serial_info: application serial information like the default
 *                               configuration and all allowed configurations.
 *             Out: none
 *
 * Return: none
 *
 ******************************************************************************/

void
SER_InitSerialConfig (const T_AppliSerialInfo *serial_info)
{
    int         uart_id;
    int         flow;
    SYS_UWORD16 serial_driver;
    SYS_UWORD16 *allowed_config;
    SYS_UWORD8  nb_allowed_config;
    SYS_BOOL    valid_config_selected;
    SYS_BOOL    uart_used;
    SYS_BOOL    uart_used_for_trace;
    SYS_UWORD16 current_config;
    SYS_UWORD16 *pt_current_config = &(current_config);

    /*
	 * Basic UARTs initializations.
	 */

    for (uart_id = 0; uart_id < NUMBER_OF_TR_UART; uart_id++) {

        int_uart[uart_id].base_address = uart_base_address[uart_id];
        int_uart[uart_id].device_used = 0;
        int_uart[uart_id].deep_sleep_set_up = 0;
    }
						  
#if ((CHIPSET == 2) || (CHIPSET == 3))
    uart_spurious_interrupts = 0;
#elif ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
    uart_modem_spurious_interrupts = 0;
    uart_irda_spurious_interrupts = 0;
#endif
#if (CHIPSET == 12)
    uart_modem2_spurious_interrupts = 0;
#endif
    uart_sleep_timer_enabled = 0;

    /*
     * Compute the current serial configuration.
     */

    for (uart_id = 0; uart_id < NUMBER_OF_TR_UART; uart_id++) {

        switch (ser_cfg_info[uart_id]) { 

            case G23_PANEL:
                serial_cfg = serial_cfg +
                    ((uart_id + 1) << (12 - (4 * SER_PROTOCOL_STACK)));
                break;

            case RIVIERA_TRACE_MUX:
                serial_cfg = serial_cfg +
                    ((uart_id + 1) << (12 - (4 * SER_LAYER_1)));
                break;

            case FD_AT_COMMAND:
                serial_cfg = serial_cfg +
                    ((uart_id + 1) << (12 - (4 * SER_FAX_DATA)));
                break;

            case BLUETOOTH_HCI:
                serial_cfg = serial_cfg +
                    ((uart_id + 1) << (12 - (4 * SER_BLUETOOTH_HCI)));
                break;

            case DUMMY:
                break;
        }
    }

    current_config = serial_cfg;
    valid_config_selected = 0;
    nb_allowed_config = serial_info->num_config;

    /*
     * Checks if the current serial config is one of the allowed.
     */
     
    while ((nb_allowed_config > 0) && !valid_config_selected) {
        
        nb_allowed_config--;
        allowed_config = (SYS_UWORD16 *)
                          &(serial_info->allowed_config[nb_allowed_config]);
        
        if (*pt_current_config == *allowed_config)
            valid_config_selected = 1;
    }

    /*
     * If not, the default configuration is selected.
     */

    if (!valid_config_selected) {

        pt_current_config = (SYS_UWORD16 *)&(serial_info->default_config);

        #if SERIAL_DYNAMIC_SWITCH
          /*
           * Setup the global variable accordingly.
           * The following default value are identical to the ones defined at
           * the application initialization in init.c.
           */

          #ifdef BT_UART_USED_MODEM
            memcpy (ser_cfg_info, "RB", NUMBER_OF_TR_UART);
          #else
            memcpy (ser_cfg_info, "BR", NUMBER_OF_TR_UART);
          #endif
        #endif
    }

    /*
     * The serial data flow functions set is initialized.
     */

    flow = 0;
    while (flow < SER_MAX_NUMBER_OF_FLOWS) {

        serial_driver = (T_SerialDriver)
                            (((*pt_current_config) >> (12 - flow * 4)) & 0x000F);

        set_flow_functions (flow, serial_driver);
        flow++;
    }
    
    /*
     * Checks if both UARTs are used.
     * If not, performs minimum initialization including Sleep Mode.
     * Checks also if at least one UART is used by a Trace flow.
     * If so, create a HISR in order to reset and restart the sleep timer
     * in case of incoming characters.
     */

    uart_used = 0;
    uart_used_for_trace = 0;
    for (uart_id = 0; uart_id < NUMBER_OF_TR_UART; uart_id++) {

        if (!(int_uart[uart_id].device_used))
		    initialize_uart_sleep (uart_id);

        else { /* if (int_uart[uart_id].device_used) */

            uart_used = 1;  /* At least one UART is used */

            if (int_uart[uart_id].flow_type == TRACE_FLOW) {

                /* At least one UART used by a Trace flow */
                uart_used_for_trace = 1;
            }
        }
    }

    /*
     * If at least one uart is used, create a timer to figure out if the system
     * can enter deep sleep mode regarding the UARTs.
     */

    if (uart_used) {

        (void) NU_Create_Timer (
                   &uart_sleep_timer,
                   "Sleep",
                   &analyze_uart_sleep_timer_expiration,
                   0, /* Parameter supplied to the routine: not used. */
                   WAKE_UP_TIME_IN_TDMA,
                   0, /* The timer expires once. */
                   NU_DISABLE_TIMER);

        /*
         * If at least one uart is used by a Trace flow, create a HISR to reset
         * and restart the sleep timer.
         */

        if (uart_used_for_trace) {

            /*
             * The stack is entirely filled with the pattern 0xFE.
             */

            memset (&(timer_hisr_stack[0]), 0xFE, TIMER_HISR_STACK_SIZE);

            /*
             * The HISR entry function is the same function than the one called
             * by the Rx HISR of the UARTFAX, since the only aim is to reset
             * and restart the sleep timer in case of incoming characters on
             * the Trace UART.
             */

            (void) NU_Create_HISR (
                       &timer_hisr_ctrl_block,
                       "Tim_HISR",
                       SER_restart_uart_sleep_timer,
                       TIMER_HISR_PRIORITY,
                       &(timer_hisr_stack[0]),
                       TIMER_HISR_STACK_SIZE);
        }
    }
}


/*******************************************************************************
 *
 *                          SER_WriteConfig
 *
 * Purpose: TBD
 *
 * Parameters: In : new_config: TBD
 *                  write_to_flash: TBD
 *             Out: none
 *
 * Return: 0 (FALSE)   : In case of error while trying to write file in FFS
 *         >= 1 (TRUE) : Successful operation.
 *
 ******************************************************************************/

SYS_BOOL
SER_WriteConfig (char *new_config,
                 SYS_BOOL write_to_flash)
{
#if SERIAL_DYNAMIC_SWITCH
    int      uart_id;
    SYS_BOOL status = 1;

    for (uart_id = 0; uart_id < NUMBER_OF_TR_UART; uart_id++)
        ser_new_cfg[uart_id] = *new_config++;

    /*
     * Write in flash the new serial configuration if requested.
     */

    if (write_to_flash) {
        if (ffs_fwrite (uart_config_file,
                        ser_new_cfg,
                        NUMBER_OF_TR_UART) < EFFS_OK) {
            status = 0;
        }
    }

    return (status);
#else
    /*
     * Real Dynamic Switch is only available with Bluetooth AND all chips but
     * Calypso+.
     */

    return (1);
#endif
}

/*******************************************************************************
 *
 *                          SER_ImmediateSwitch
 *
 * Purpose: TBD
 *
 * Parameters: In : none
 *             Out: none
 *
 * Return: 0 (FALSE)   : In case of error.
 *         >= 1 (TRUE) : Successful operation.
 *
 ******************************************************************************/

SYS_BOOL
SER_ImmediateSwitch (void)
{
#if SERIAL_DYNAMIC_SWITCH
    int                uart_id;
    SYS_BOOL           valid_config = 0;
    T_AppliSerialInfo *serial_info = &appli_ser_cfg_info;
    SYS_UWORD8         nb_allowed_config = serial_info->num_config;
    SYS_UWORD16       *allowed_config;
    int                flow;
    T_SerialDriver     serial_flows[SER_MAX_NUMBER_OF_FLOWS];
    T_tr_UartId        uart_nb;

    /*
     * First check if the new serial configuration is actually different from
     * the previous one. A return is used to simplify the code.
     */

    if (!memcmp (ser_new_cfg,
                 ser_cfg_info,
                 NUMBER_OF_TR_UART))
        return (1); /* new config and old config are identical => nothing to do */

    /*
     * Then check if the new serial config is valid or not.
     * At that point, we assume that a serial config is valid if and only if the
     * Bluetooth HCI flow is still enabled and still uses the same UART.
     * Reset the current serial config, and compute the new one.
     */

    serial_cfg = 0x0048; /* All dummies */
    for (uart_id = 0; uart_id < NUMBER_OF_TR_UART; uart_id++) {

        switch (ser_new_cfg[uart_id]) { 

            case G23_PANEL:
                serial_cfg = serial_cfg +
                    ((uart_id + 1) << (12 - (4 * SER_PROTOCOL_STACK)));
                break;

            case RIVIERA_TRACE_MUX:
                serial_cfg = serial_cfg +
                    ((uart_id + 1) << (12 - (4 * SER_LAYER_1)));
                break;

            case FD_AT_COMMAND:
                serial_cfg = serial_cfg +
                    ((uart_id + 1) << (12 - (4 * SER_FAX_DATA)));
                break;

            case BLUETOOTH_HCI:
                serial_cfg = serial_cfg +
                    ((uart_id + 1) << (12 - (4 * SER_BLUETOOTH_HCI)));

                /*
                 * Check if the Bluetooth HCI flow is enabled on the same UART.
                 */

                if (ser_cfg_info[uart_id] == BLUETOOTH_HCI)
                    valid_config = 1;

                break;

            case DUMMY:
                break;
        }
    }

    if (!valid_config)
        return (0); /* Bluetooth HCI flow not enabled in the new serial config,
                       or enabled but using a different UART. */

    /*
     * Finally check if the new serial config is allowed by the application.
     */
     
    valid_config = 0;
    while ((nb_allowed_config > 0) && !valid_config) {
        
        nb_allowed_config--;
        allowed_config = (SYS_UWORD16 *)
                          &(serial_info->allowed_config[nb_allowed_config]);
        
        if (serial_cfg == *allowed_config)
            valid_config = 1;
    }

    if (!valid_config) /* the new config is not allowed by the application */
        return (0);

    /*
     * From now on, Dynamic Switch is being processed.
     */

    dynamic_switch = 1;

    /*
     * Disable UART interrupts until new serial config setup is complete.
     */

    #if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
      IQ_Mask (IQ_UART_IRDA_IT);
    #endif
    IQ_Mask (IQ_UART_IT);

    /*
     * Reset UARTs set-up.
     */

    for (uart_id = 0; uart_id < NUMBER_OF_TR_UART; uart_id++) {

        int_uart[uart_id].device_used = 0;
        int_uart[uart_id].deep_sleep_set_up = 0;
        int_uart[uart_id].interrupt_handler = NULL;
    }

    /*
     * All function pointers are set to dummy functions.
     */

    rvf_disable (21); /* beginning of the critical section */

    for (flow = 0; flow < SER_MAX_NUMBER_OF_FLOWS; flow++)
        tr_functions[flow] = &dummy_trace;

    fd_functions = &dummy_fax_data;
    bt_functions = &dummy_bt_hci;

    rvf_enable (); /* end of the critical section */

    /*
     * Calls the Exit function of the UARTFAX driver if it was previously used.
     */

    if (uart_fd_initialized) {

        /*
         * UART IrDA can't be used for F&D/AT-Cmd flow => UART Modem was used
         * by the F&D/AT-Cmd flow.
         */

        if (UAF_Exit (UAF_UART_1) == FD_OK) {
            uart_fd_initialized = 0;
        }
    }
    else {

        /*
         * AT that point, since the Bluetooth HCI flow already uses one UART,
         * and since the second UART was not used by the F&D/AT-Cmd flow, we
         * assume it was used by a Trace flow. Therefore, the HISR used to 
         * reset and restart the sleep timer is deleted.
         */

        (void) NU_Delete_HISR (&timer_hisr_ctrl_block);
    }

    /*
     * Initialization of the new flows (Only AT-Cmd/F&D or Riviera/Layer1 Trace)
     * and their associated UARTs HW (Irda or Modem) & SW (Trace or Fax&Data).
     */

    for (flow = 0; flow < SER_MAX_NUMBER_OF_FLOWS; flow++) {

        serial_flows[flow] = (T_SerialDriver)
                             ((serial_cfg >> (12 - flow * 4)) & 0x000F);

        switch (serial_flows[flow]) {

            /*
             * For Riviera/Layer1 Trace flow, default baudrate is 115200 bps
             * and callback function is defined in rvt_def_i.h.
             */

            case UART_IRDA_TRACE:
            case UART_MODEM_TRACE:

                if (serial_flows[flow] == UART_IRDA_TRACE)
                    uart_nb = UA_UART_0;
                else /* if (serial_flows[flow] == UART_MODEM_TRACE) */
                    uart_nb = UA_UART_1;

                if (flow == SER_LAYER_1) {

                    UA_Init (uart_nb,
                             TR_BAUD_CONFIG,
                             rvt_activate_RX_HISR);

                    /*
                     * Create the HISR used to reset and restart the sleep
                     * timer in case of incoming characters on the Trace flow.
                     * The stack is entirely filled with the pattern 0xFE.
                     */

                    memset (&(timer_hisr_stack[0]),
                            0xFE,
                            TIMER_HISR_STACK_SIZE);

                    (void) NU_Create_HISR (
                               &timer_hisr_ctrl_block,
                               "Tim_HISR",
                               SER_restart_uart_sleep_timer,
                               TIMER_HISR_PRIORITY,
                               &(timer_hisr_stack[0]),
                               TIMER_HISR_STACK_SIZE);
                }
                else /* Other Trace flows are disabled */
                    initialize_uart_sleep (uart_nb);
                break;

            /*
             * For At-Cmd/F&D flow, functions are called in the appropriate
             * order with the saved parameters.
             * This has been figured out from the G23 initialization.
             */

            case UART_MODEM_FAX_DATA:

                /* Global Initialization */
                if (UAF_Init (UAF_UART_1) == FD_OK) {
                    uart_fd_initialized = 1;
                }

                /* Disable the driver */
                UAF_Enable (UAF_UART_1,
                            0);

                /* Set the SW Buffers parameters */
                UAF_SetBuffer (UAF_UART_1,
                               data_flow_parameters.bufSize,
                               data_flow_parameters.rxThreshold,
                               data_flow_parameters.txThreshold);

                /* Set the Escape Sequence parameters (1st call) */
                UAF_SetEscape (UAF_UART_1,
                               data_flow_parameters.escChar[0],
                               data_flow_parameters.guardPeriod[0]);

                /* Set the Communication parameters (1st call) */
                UAF_SetComPar (UAF_UART_1,
                               data_flow_parameters.baudrate[0],
                               data_flow_parameters.bpc[0],
                               data_flow_parameters.sb[0],
                               data_flow_parameters.parity[0]);

                /* Set the Flow Control parameters (1st call) */
                UAF_SetFlowCtrl (UAF_UART_1,
                                 data_flow_parameters.fcMode[0],
                                 data_flow_parameters.XON[0],
                                 data_flow_parameters.XOFF[0]);

                /* Set the Communication parameters (2nd call) */
                UAF_SetComPar (UAF_UART_1,
                               data_flow_parameters.baudrate[1],
                               data_flow_parameters.bpc[1],
                               data_flow_parameters.sb[1],
                               data_flow_parameters.parity[1]);

                /* Set the Flow Control parameters (2nd call) */
                UAF_SetFlowCtrl (UAF_UART_1,
                                 data_flow_parameters.fcMode[1],
                                 data_flow_parameters.XON[1],
                                 data_flow_parameters.XOFF[1]);

                /* Set the Escape Sequence parameters (2nd call) */
                UAF_SetEscape (UAF_UART_1,
                               data_flow_parameters.escChar[1],
                               data_flow_parameters.guardPeriod[1]);

                /* Enable the driver */
                UAF_Enable (UAF_UART_1,
                            1);

                /* Get the number of input bytes available */
                UAF_InpAvail (UAF_UART_1);

                /* Set the readOutFunc and the suspend mode */
                UAF_ReadData (UAF_UART_1,
                              data_flow_parameters.suspend_rd,
                              data_flow_parameters.readOutFunc);

                /* Get the number of output bytes available (1st call) */
                UAF_OutpAvail (UAF_UART_1);

                /* Set the states of the V.24 status lines (1st call) */
                UAF_SetLineState (UAF_UART_1,
                                  data_flow_parameters.state[0],
                                  data_flow_parameters.mask[0]);

                /* Set the states of the V.24 status lines (2nd call) */
                UAF_SetLineState (UAF_UART_1,
                                  data_flow_parameters.state[1],
                                  data_flow_parameters.mask[1]);

                /* Set the states of the V.24 status lines (3rd call) */
                UAF_SetLineState (UAF_UART_1,
                                  data_flow_parameters.state[2],
                                  data_flow_parameters.mask[2]);

                /* Set the states of the V.24 status lines (4th call) */
                UAF_SetLineState (UAF_UART_1,
                                  data_flow_parameters.state[3],
                                  data_flow_parameters.mask[3]);

                /* Set the writeInFunc and the suspend mode */
                UAF_WriteData (UAF_UART_1,
                               data_flow_parameters.suspend_wr,
                               data_flow_parameters.writeInFunc);

                /* Get the number of output bytes available (2nd call) */
                UAF_OutpAvail (UAF_UART_1);

                break;

            case UART_IRDA_BT_HCI:
            case UART_MODEM_BT_HCI:
                /*
                 * Nothing to initialize for Bluetooth HCI flow since it does
                 * use the same UART.
                 */

            case DUMMY_TRACE:
            case DUMMY_FAX_DATA:
            case DUMMY_BT_HCI:
                /*
                 * Of course nothing to perform for Dummy flows.
                 */

                break;
        }
    }

    /*
     * All function pointers are set to the appropriate functions set.
     */

    for (flow = 0; flow < SER_MAX_NUMBER_OF_FLOWS; flow++){

        /*
         * For Dummy flows, pointers to dummy functions are already set.
         */

        if ((serial_flows[flow] != DUMMY_TRACE) &&
            (serial_flows[flow] != DUMMY_FAX_DATA) &&
            (serial_flows[flow] != DUMMY_BT_HCI)) {

            rvf_disable (21); /* beginning of the critical section */
            set_flow_functions (flow, serial_flows[flow]);
            rvf_enable (); /* end of the critical section */
        }
    }

    /*
     * Dynamic Switch has been processed.
     * The new serial config is actually stored.
     */

    dynamic_switch = 0;
    for (uart_id = 0; uart_id < NUMBER_OF_TR_UART; uart_id++)
        ser_cfg_info[uart_id] = ser_new_cfg[uart_id];

    /*
     * Re-enable UART interrupts.
     */

    #if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
      IQ_Unmask (IQ_UART_IRDA_IT);
    #endif
    IQ_Unmask (IQ_UART_IT);

#endif
    /*
     * Real Dynamic Switch is only available with Bluetooth AND all chips but
     * Calypso+.
     */

    return (1);
}

/*******************************************************************************
 *
 * All functions SER_tr_xxx and SER_fd_xxx call a function of the UART trace
 * driver or the UART fax & data driver.
 * All functions SER_bt_xxx call a function of the UART Bluetooth HCI driver.
 * See the function call for parameters and return values.
 *
 ******************************************************************************/
 
void
SER_tr_Init (int serial_data_flow,
             T_tr_Baudrate baudrate,
             void (callback_function (void)))
{
    tr_functions[serial_data_flow]->tr_Init (
                tr_functions[serial_data_flow]->device, baudrate, callback_function);
}

SYS_UWORD32
SER_tr_ReadNChars (int serial_data_flow,
                   char *buffer,
                   SYS_UWORD32 chars_to_read)
{
    return (tr_functions[serial_data_flow]->tr_ReadNChars (
                tr_functions[serial_data_flow]->device, buffer, chars_to_read));
}

SYS_UWORD32
SER_tr_ReadNBytes (int serial_data_flow,
                   char *buffer,
                   SYS_UWORD32 chars_to_read,
                   SYS_BOOL *eof_detected)
{
    return (tr_functions[serial_data_flow]->tr_ReadNBytes (
                tr_functions[serial_data_flow]->device, buffer, chars_to_read, eof_detected));
}

SYS_UWORD32
SER_tr_WriteNChars (int serial_data_flow,
                    char *buffer,
                    SYS_UWORD32 chars_to_write)
{
    return (tr_functions[serial_data_flow]->tr_WriteNChars (
                tr_functions[serial_data_flow]->device, buffer, chars_to_write));
}

SYS_UWORD32
SER_tr_EncapsulateNChars (int serial_data_flow,
                          char *buffer,
                          SYS_UWORD32 chars_to_write)
{
    return (tr_functions[serial_data_flow]->tr_EncapsulateNChars (
                tr_functions[serial_data_flow]->device, buffer, chars_to_write));
}

SYS_UWORD32
SER_tr_WriteNBytes (int serial_data_flow,
                    SYS_UWORD8 *buffer,
                    SYS_UWORD32 chars_to_write)
{
    return (tr_functions[serial_data_flow]->tr_WriteNBytes (
                tr_functions[serial_data_flow]->device, buffer, chars_to_write));
}

void
SER_tr_WriteChar (int serial_data_flow,
                  char character)
{
    tr_functions[serial_data_flow]->tr_WriteChar (
                tr_functions[serial_data_flow]->device, character);
}

void
SER_tr_WriteString (int serial_data_flow,
                    char *buffer)
{
    tr_functions[serial_data_flow]->tr_WriteString (
                tr_functions[serial_data_flow]->device, buffer);
}

SYS_BOOL
SER_tr_EnterSleep (int serial_data_flow)
{
    return (tr_functions[serial_data_flow]->tr_EnterSleep (
                tr_functions[serial_data_flow]->device));
}

void
SER_tr_WakeUp (int serial_data_flow)
{
    tr_functions[serial_data_flow]->tr_WakeUp (
                tr_functions[serial_data_flow]->device);
}

/* Dummy function for backward compatibility. */
T_FDRET
SER_fd_Init (void)
{
    return (FD_OK);
}

T_FDRET
SER_fd_Initialize (void)
{
    T_FDRET status;

    #if SERIAL_DYNAMIC_SWITCH
      data_flow_parameters.bufSize = FD_MAX_BUFFER_SIZE;
    #else
      bufSize = FD_MAX_BUFFER_SIZE;
    #endif
    status = fd_functions->fd_Initialize (fd_functions->device);

    #if SERIAL_DYNAMIC_SWITCH
      /*
       *  Check if the UARTFAX driver has actually been initialized.
       */

      if ((fd_functions->fd_Initialize == UAF_Init) &&
          (status = FD_OK)) {

          uart_fd_initialized = 1;
      }
    #endif

    return (status);
}

static void
fd_dbgtrace(char *msg, UINT32 val)
{
	rvf_send_trace(msg, strlen(msg), val, RV_TRACE_LEVEL_DEBUG_HIGH, 0);
}

T_FDRET
SER_fd_Enable (SYS_BOOL enable)
{
	T_FDRET rc;

	fd_dbgtrace("SER_fd_Enable called", enable);
	rc = fd_functions->fd_Enable (fd_functions->device, enable);
	fd_dbgtrace("fd_Enable returned", rc);
	return rc;
}

T_FDRET
SER_fd_SetComPar (T_baudrate baudrate,
                  T_bitsPerCharacter bpc,
                  T_stopBits sb,
                  T_parity parity)
{
	T_FDRET rc;
	char trbuf[80];

	sprintf(trbuf, "SER_fd_SetComPar(%d, %d, %d, %d)",
		baudrate, bpc, sb, parity);
	fd_dbgtrace(trbuf, NULL_PARAM);
	rc = (fd_functions->fd_SetComPar (
                fd_functions->device, baudrate, bpc, sb, parity));
	fd_dbgtrace("fd_SetComPar returned", rc);
	return rc;
}

T_FDRET
SER_fd_SetBuffer (SYS_UWORD16 bufSize,
                  SYS_UWORD16 rxThreshold,
                  SYS_UWORD16 txThreshold)
{
	T_FDRET rc;
	char trbuf[80];

	sprintf(trbuf, "SER_fd_SetBuffer(%u, %u, %u)",
		bufSize, rxThreshold, txThreshold);
	fd_dbgtrace(trbuf, NULL_PARAM);
	rc = (fd_functions->fd_SetBuffer (
                fd_functions->device, bufSize, rxThreshold, txThreshold));
	fd_dbgtrace("fd_SetBuffer returned", rc);
	return rc;
}

T_FDRET
SER_fd_SetFlowCtrl (T_flowCtrlMode fcMode,
                    SYS_UWORD8 XON,
                    SYS_UWORD8 XOFF)
{
	T_FDRET rc;
	char trbuf[80];

	sprintf(trbuf, "SER_fd_SetFlowCtrl(%d, %02x, %02x)", fcMode, XON, XOFF);
	fd_dbgtrace(trbuf, NULL_PARAM);
	rc = (fd_functions->fd_SetFlowCtrl (
                fd_functions->device, fcMode, XON, XOFF));
	fd_dbgtrace("fd_SetFlowCtrl returned", rc);
	return rc;
}

T_FDRET
SER_fd_SetEscape (char escChar,
                  SYS_UWORD16 guardPeriod)
{
	T_FDRET rc;
	char trbuf[80];

	sprintf(trbuf, "SER_fd_SetEscape(%02x, %u)", escChar, guardPeriod);
	fd_dbgtrace(trbuf, NULL_PARAM);
	rc = (fd_functions->fd_SetEscape (
                fd_functions->device, escChar, guardPeriod));
	fd_dbgtrace("fd_SetEscape returned", rc);
	return rc;
}

T_FDRET
SER_fd_InpAvail (void)
{
	T_FDRET rc;

	fd_dbgtrace("SER_fd_InpAvail called", NULL_PARAM);
	rc = (fd_functions->fd_InpAvail (fd_functions->device));
	fd_dbgtrace("fd_InpAvail returned", rc);
	return rc;
}

T_FDRET
SER_fd_OutpAvail (void)
{
	T_FDRET rc;

	fd_dbgtrace("SER_fd_OutpAvail called", NULL_PARAM);
	rc = (fd_functions->fd_OutpAvail (fd_functions->device));
	fd_dbgtrace("fd_OutpAvail returned", rc);
	return rc;
}

T_FDRET
SER_fd_EnterSleep (void)
{
    return (fd_functions->fd_EnterSleep (fd_functions->device));
}

T_FDRET
SER_fd_WakeUp (void)
{
    return (fd_functions->fd_WakeUp (fd_functions->device));
}

T_FDRET
SER_fd_ReadData (T_suspendMode suspend,
                 void (readOutFunc (SYS_BOOL cldFromIrq,
                                    T_reInstMode *reInstall,
                                    SYS_UWORD8 nsource,
                                    SYS_UWORD8 *source[],
                                    SYS_UWORD16 size[],
                                    SYS_UWORD32 state)))
{
	T_FDRET rc;

	fd_dbgtrace("SER_fd_ReadData called", suspend);
	rc = (fd_functions->fd_ReadData (
                fd_functions->device, suspend, readOutFunc));
	fd_dbgtrace("fd_ReadData returned", rc);
	return rc;
}

T_FDRET
SER_fd_WriteData (T_suspendMode suspend,
                  void (writeInFunc (SYS_BOOL cldFromIrq,
                                     T_reInstMode *reInstall,
                                     SYS_UWORD8 ndest,
                                     SYS_UWORD8 *dest[],
                                     SYS_UWORD16 size[])))
{
	T_FDRET rc;

	fd_dbgtrace("SER_fd_WriteData called", suspend);
	rc = (fd_functions->fd_WriteData (
                fd_functions->device, suspend, writeInFunc));
	fd_dbgtrace("fd_WriteData returned", rc);
	return rc;
}

T_FDRET
SER_fd_StopRec (void)
{
	T_FDRET rc;

	fd_dbgtrace("SER_fd_StopRec called", NULL_PARAM);
	rc = (fd_functions->fd_StopRec (fd_functions->device));
	fd_dbgtrace("fd_StopRec returned", rc);
	return rc;
}

T_FDRET
SER_fd_StartRec (void)
{
	T_FDRET rc;

	fd_dbgtrace("SER_fd_StartRec called", NULL_PARAM);
	rc = (fd_functions->fd_StartRec (fd_functions->device));
	fd_dbgtrace("fd_StartRec returned", rc);
	return rc;
}

T_FDRET
SER_fd_GetLineState (SYS_UWORD32 *state)
{
	T_FDRET rc;
	char trbuf[80];

	fd_dbgtrace("SER_fd_GetLineState called", NULL_PARAM);
	rc = (fd_functions->fd_GetLineState (fd_functions->device, state));
	sprintf(trbuf, "fd_GetLineState returned %d, %x", rc, *state);
	fd_dbgtrace(trbuf, NULL_PARAM);
	return rc;
}

T_FDRET
SER_fd_SetLineState (SYS_UWORD32 state,
                     SYS_UWORD32 mask)
{
	T_FDRET rc;
	char trbuf[80];

	sprintf(trbuf, "SER_fd_SetLineState(%x, %x)", state, mask);
	fd_dbgtrace(trbuf, NULL_PARAM);
	rc = (fd_functions->fd_SetLineState (fd_functions->device, state, mask));
	fd_dbgtrace("fd_SetLineState returned", rc);
	return rc;
}

T_FDRET
SER_fd_CheckXEmpty (void)
{
	T_FDRET rc;

	fd_dbgtrace("SER_fd_CheckXEmpty called", NULL_PARAM);
	rc = (fd_functions->fd_CheckXEmpty (fd_functions->device));
	fd_dbgtrace("fd_CheckXEmpty returned", rc);
	return rc;
}

#ifdef BTEMOBILE
T_HCI_RET
SER_bt_Init (void)
{
    return (bt_functions->bt_Init (bt_functions->device));
}

T_HCI_RET
SER_bt_Start (void)
{
    return (bt_functions->bt_Start ());
}

T_HCI_RET
SER_bt_Stop (void)
{
    return (bt_functions->bt_Stop ());
}

T_HCI_RET
SER_bt_Kill (void)
{
    return (bt_functions->bt_Kill ());
}

T_HCI_RET
SER_bt_SetBaudrate (UINT8 baudrate)
{
    return (bt_functions->bt_SetBaudrate (baudrate));
}

T_HCI_RET SER_bt_TransmitPacket (void *uart_tx_buffer)
{
    return (bt_functions->bt_TransmitPacket (uart_tx_buffer));
}

SYS_BOOL SER_bt_EnterSleep (void)
{
    return (bt_functions->bt_EnterSleep());
}

void SER_bt_WakeUp (void)
{
    bt_functions->bt_WakeUp();
}
#endif /* BTEMOBILE */

/*******************************************************************************
 *
 *                          SER_UartSleepStatus
 *
 * Purpose: This function checks if both UARTs are ready to enter Deep Sleep. 
 *
 * Parameters: In : none
 *             Out: none 
 *
 * Return: 0	 : Deep Sleep is not possible.
 *         >= 1  : Deep Sleep is possible.
 *
 ******************************************************************************/

SYS_BOOL
SER_UartSleepStatus (void)
{
    t_uart   *uart;
    int      uart_id;
    SYS_BOOL status;

    /*
     * Check first if the sleep timer is active or if a Dynamic Switch is
     * being processed. A return is used to simplify the code.
     */

#if SERIAL_DYNAMIC_SWITCH
    if (uart_sleep_timer_enabled || dynamic_switch)
#else
    if (uart_sleep_timer_enabled)
#endif
	    return (0);

    /*
     * Check if both UARTs are ready to enter Deep Sleep.
     */

    status = 1;
    uart_id = 0;
    while ((uart_id < NUMBER_OF_TR_UART) &&
           (status)) {

           uart = &(int_uart[uart_id]);

           /*
            * Check if the specified UART is actually used.
            */

           if (uart->device_used) {

               /*
                * Check if the specified UART is used by a Trace or
                * by a Fax & Data flow.
                */

               if (uart->flow_type == TRACE_FLOW)
                   status = SER_tr_EnterSleep (uart->flow_id);

               else
                   if (uart->flow_type == FAX_DATA_FLOW)
                       status = (SYS_BOOL) SER_fd_EnterSleep ();
#ifdef BTEMOBILE
                   else
                       if (uart->flow_type == BLUETOOTH_HCI_FLOW) 
                           status = SER_bt_EnterSleep();
#endif
                       else
                           status = 0;

               if (status) {

                   /*
    	            * The specified UART is now set up for Deep Sleep.
    	            */

                   uart->deep_sleep_set_up = 1;

               }
           }

           uart_id++;
    }

    /*
     * Check if Deep Sleep is finally possible.
     * If not revert eventual Deep Sleep settings.
     */

    if (!status) {

        for (uart_id = 0; uart_id < NUMBER_OF_TR_UART; uart_id++) {

            uart = &(int_uart[uart_id]);

            /*
             * If the specified used UART has already been set up for
             * Deep Sleep, revert these settings.
             */

            if ((uart->device_used) &&
                (uart->deep_sleep_set_up)) {
				
                /*
                 * Check if the specified UART is used by a Trace or
                 * by a Fax & Data flow.
                 * Bluetooth HCI can not yet handled Deep Sleep Mode.
                 */

                if (uart->flow_type == TRACE_FLOW)
                    SER_tr_WakeUp (uart->flow_id);

                else  
                    if (uart->flow_type == FAX_DATA_FLOW) 
                    	SER_fd_WakeUp ();
#ifdef BTEMOBILE
                    else 
                        if (uart->flow_type == BLUETOOTH_HCI_FLOW)
                            SER_bt_WakeUp ();
#endif
                uart->deep_sleep_set_up = 0;

            }
        }
    }

    return (status);
}


/*******************************************************************************
 *
 *                            SER_WakeUpUarts
 *
 * Purpose: This function wakes up used UARTs after Deep Sleep.
 *
 * Parameters: In : none
 *             Out: none 
 *
 * Return: none
 *
 ******************************************************************************/

void
SER_WakeUpUarts (void)
{
    t_uart   *uart;
    int      uart_id;

    if (uart_sleep_timer_enabled)
        start_uart_sleep_timer ();

    for (uart_id = 0; uart_id < NUMBER_OF_TR_UART; uart_id++) {

        uart = &(int_uart[uart_id]);

        /*
         * Check if the specified UART is actually used, and has not yet
		 * been waked up.
         */

        if ((uart->device_used) &&
            (uart->deep_sleep_set_up)) {

            /*
             * Check if the specified UART is used by a Trace or
             * by a Fax & Data flow.
             * Bluetooth HCI can not yet handled Deep Sleep Mode.
             */

            if (uart->flow_type == TRACE_FLOW)
                SER_tr_WakeUp (uart->flow_id);

            else
                if (uart->flow_type == FAX_DATA_FLOW) 
                	SER_fd_WakeUp ();
#ifdef BTEMOBILE
                else
                    if (uart->flow_type == BLUETOOTH_HCI_FLOW)
                        SER_bt_WakeUp ();
#endif
            /*
             * The specified UART is no more set up for Deep Sleep.
             */

            uart->deep_sleep_set_up = 0;

        }
    }
}


/*******************************************************************************
 *
 *                         SER_restart_uart_sleep_timer
 * 
 * Purpose  : Resets and restarts the sleep timer each time some characters are
 *            received.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

void
SER_restart_uart_sleep_timer (void)
{
    /*
     * First disable the timer.
     */

    (void) NU_Control_Timer (&uart_sleep_timer,
                             NU_DISABLE_TIMER);

    /*
     * Then start again this timer for a new period.
     */

    start_uart_sleep_timer ();
}


/*******************************************************************************
 *
 *                            SER_activate_timer_hisr
 * 
 * Purpose  : Activates the timer HISR to reset and restart the sleep timer
 *            each time some characters are received.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

void
SER_activate_timer_hisr (void)
{
    (void) NU_Activate_HISR (&timer_hisr_ctrl_block);
}


#if ((CHIPSET == 2) || (CHIPSET == 3))

/*******************************************************************************
 *
 *                                SER_uart_handler
 * 
 * Purpose  : UART interrupt handler.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

void
SER_uart_handler (void)
{
    SYS_UWORD8 interrupt_status;
    t_uart     *uart;
    int        uart_id;
    SYS_BOOL   it_identified;

    it_identified = 0;

    /*
     * Check first for a wake-up interrupt.
     */

    uart_id = 0;
    while ((uart_id < NUMBER_OF_TR_UART) &&
           (!it_identified)) {

           uart = &(int_uart[uart_id]);
           interrupt_status = READ_UART_REGISTER (uart, SSR);

           if (interrupt_status & RX_CTS_WAKE_UP_STS) { /* Wake-up IT has occurred */

               it_identified = 1;
               uart_sleep_timer_enabled = 1;
			   DISABLE_WAKE_UP_INTERRUPT (uart);
           }

           uart_id++;
    }

    /*
     * If no wake-up interrupt has been detected, check then systematically
     * both UARTs for other interrupt causes.
     */

    if (!it_identified) {

        for (uart_id = 0; uart_id < NUMBER_OF_TR_UART; uart_id++) {

            uart = &(int_uart[uart_id]);
            interrupt_status = READ_UART_REGISTER (uart, IIR) & IIR_BITS_USED;

            if (!(interrupt_status & IT_NOT_PENDING)) {

                it_identified = 1;
                (*(uart->interrupt_handler)) (uart_id, interrupt_status);

            } else {

                if ((uart_id == UA_UART_1) && (!it_identified))
                uart_spurious_interrupts++;
            }
        }
    }
}

#elif ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))

/*******************************************************************************
 *
 *                                SER_uart_modem_handler
 * 
 * Purpose  : UART MODEM interrupt handler.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

void
SER_uart_modem_handler (void)
{
    SYS_UWORD8 interrupt_status;
    t_uart     *uart;
    SYS_BOOL   it_wakeup_identified;

    it_wakeup_identified = 0;
    uart = &(int_uart[UA_UART_1]);

    /*
     * Check first for a wake-up interrupt.
     */

    interrupt_status = READ_UART_REGISTER (uart, SSR);

    if (interrupt_status & RX_CTS_WAKE_UP_STS) { /* Wake-up IT has occurred */

        it_wakeup_identified = 1;
        uart_sleep_timer_enabled = 1;
#ifdef BTEMOBILE
        if (uart->flow_type == BLUETOOTH_HCI_FLOW)
        {
            interrupt_status = READ_UART_REGISTER (uart, IIR) & IIR_BITS_USED;
            (*(uart->interrupt_handler)) (UA_UART_1, interrupt_status);
        }
#endif /* BTEMOBILE */
        DISABLE_WAKE_UP_INTERRUPT (uart);
    }

    /*
     * If no wake-up interrupt has been detected, check UART for other
     * interrupt causes.
     */

    if (!it_wakeup_identified) {

        interrupt_status = READ_UART_REGISTER (uart, IIR) & IIR_BITS_USED;

        if (!(interrupt_status & IT_NOT_PENDING))
            (*(uart->interrupt_handler)) (UA_UART_1, interrupt_status);

        else
            uart_modem_spurious_interrupts++;
    }
}


/*******************************************************************************
 *
 *                                SER_uart_irda_handler
 * 
 * Purpose  : UART IrDA interrupt handler.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

void
SER_uart_irda_handler (void)
{
    SYS_UWORD8 interrupt_status;
    t_uart     *uart;
    SYS_BOOL   it_wakeup_identified;

    it_wakeup_identified = 0;
    uart = &(int_uart[UA_UART_0]);

    /*
     * Check first for a wake-up interrupt.
     */

    interrupt_status = READ_UART_REGISTER (uart, SSR);

    if (interrupt_status & RX_CTS_WAKE_UP_STS) { /* Wake-up IT has occurred */

        it_wakeup_identified = 1;
        uart_sleep_timer_enabled = 1;
#ifdef BTEMOBILE
        if (uart->flow_type == BLUETOOTH_HCI_FLOW)
        {
            interrupt_status = READ_UART_REGISTER (uart, IIR) & IIR_BITS_USED;
            (*(uart->interrupt_handler)) (UA_UART_0, interrupt_status);
        }
#endif /* BTEMOBILE */
        DISABLE_WAKE_UP_INTERRUPT (uart);
    }

    /*
     * If no wake-up interrupt has been detected, check UART for other
     * interrupt causes.
     */

    if (!it_wakeup_identified) {

        interrupt_status = READ_UART_REGISTER (uart, IIR) & IIR_BITS_USED;

        if (!(interrupt_status & IT_NOT_PENDING))
            (*(uart->interrupt_handler)) (UA_UART_0, interrupt_status);

        else
            uart_irda_spurious_interrupts++;
    }
}

#endif

#if (CHIPSET == 12)
  /*******************************************************************************
   *
   *                                SER_uart_modem2_handler
   * 
   * Purpose  : UART IrDA interrupt handler.
   *
   * Arguments: In : none
   *            Out: none
   *
   * Returns  : none
   *
   ******************************************************************************/

  void
  SER_uart_modem2_handler (void)
  {
      SYS_UWORD8 interrupt_status;
      t_uart     *uart;
      SYS_BOOL   it_wakeup_identified;

      it_wakeup_identified = 0;
      uart = &(int_uart[UA_UART_2]);

      /*
       * Check first for a wake-up interrupt.
       */

      interrupt_status = READ_UART_REGISTER (uart, SSR);

      if (interrupt_status & RX_CTS_WAKE_UP_STS) { /* Wake-up IT has occurred */

          it_wakeup_identified = 1;
          uart_sleep_timer_enabled = 1;
#ifdef BTEMOBILE
          if (uart->flow_type == BLUETOOTH_HCI_FLOW)
          {
              interrupt_status = READ_UART_REGISTER (uart, IIR) & IIR_BITS_USED;
              (*(uart->interrupt_handler)) (UA_UART_2, interrupt_status);
          }
#endif /* BTEMOBILE */
          DISABLE_WAKE_UP_INTERRUPT (uart);
      }

      /*
       * If no wake-up interrupt has been detected, check UART for other
       * interrupt causes.
       */

      if (!it_wakeup_identified) {

          interrupt_status = READ_UART_REGISTER (uart, IIR) & IIR_BITS_USED;

          if (!(interrupt_status & IT_NOT_PENDING))
              (*(uart->interrupt_handler)) (UA_UART_2, interrupt_status);
        
          else
              uart_modem2_spurious_interrupts++;
      }
  }

#endif

/*
 * Temporary functions. 
 *
 * FreeCalypso note: I'll put them back in if they are actually needed.
 */

#if 0

void
UT_Init (int device_id,
         int baudrate,
         void (callback_function (void)))
{
    SER_tr_Init (SER_PROTOCOL_STACK, baudrate, callback_function);
}

SYS_UWORD32
UT_ReadNChars (int device_id,
               char *buffer,
               SYS_UWORD32 chars_to_read)
{
    return (SER_tr_ReadNChars (SER_PROTOCOL_STACK, buffer, chars_to_read));
}

SYS_UWORD32
UT_WriteNChars (int device_id,
                char *buffer,
                SYS_UWORD32 chars_to_write)
{
    return (SER_tr_WriteNChars (SER_PROTOCOL_STACK, buffer, chars_to_write));
}

void
UT_WriteChar (int device_id,
              char character)
{
    SER_tr_WriteChar (SER_PROTOCOL_STACK, character);
}

void
UT_WriteString (int device_id,
                char *buffer)
{
    SER_tr_WriteString (SER_PROTOCOL_STACK, buffer);
}

short
UF_Init (SYS_UWORD8 deviceNo)
{
    return (SER_fd_Init ());
}

short
UF_Enable (SYS_UWORD8 deviceNo,
           SYS_BOOL enable)
{
    return (SER_fd_Enable (enable));
}

short
UF_SetComPar (SYS_UWORD8 deviceNo,
              T_baudrate baudrate,
              T_bitsPerCharacter bpc,
              T_stopBits sb,
              T_parity parity)
{
    return (SER_fd_SetComPar (baudrate,
                              bpc,
                              sb,
                              parity));
}

short
UF_SetBuffer (SYS_UWORD8 deviceNo,
              SYS_UWORD16 bufSize,
              SYS_UWORD16 rxThreshold,
              SYS_UWORD16 txThreshold)
{
    return (SER_fd_SetBuffer (bufSize, rxThreshold, txThreshold));
}

short
UF_SetFlowCtrl (SYS_UWORD8 deviceNo,
                T_flowCtrlMode fcMode,
                SYS_UWORD8 XON,
                SYS_UWORD8 XOFF)
{
    return (SER_fd_SetFlowCtrl (fcMode, XON, XOFF));
}

short
UF_SetEscape (SYS_UWORD8 deviceNo,
              SYS_UWORD8 escChar,
              SYS_UWORD16 guardPeriod)
{
    return (SER_fd_SetEscape (escChar, guardPeriod));
}

short
UF_InpAvail (SYS_UWORD8 deviceNo)
{
    return (SER_fd_InpAvail ());
}

short
UF_OutpAvail (SYS_UWORD8 deviceNo)
{
    return (SER_fd_OutpAvail ());
}

short
UF_ReadData (SYS_UWORD8 deviceNo,
             T_suspendMode suspend,
             void (readOutFunc (SYS_BOOL cldFromIrq,
                                T_reInstMode *reInstall,
                                SYS_UWORD8 nsource,
                                SYS_UWORD8 *source[],
                                SYS_UWORD16 size[],
                                SYS_UWORD32 state)))
{
    return (SER_fd_ReadData (suspend, readOutFunc));
}

short
UF_WriteData (SYS_UWORD8 deviceNo,
              T_suspendMode suspend,
              void (writeInFunc (SYS_BOOL cldFromIrq,
                                 T_reInstMode *reInstall,
                                 SYS_UWORD8 ndest,
                                 SYS_UWORD8 *dest[],
                                 SYS_UWORD16 size[])))
{
    return (SER_fd_WriteData (suspend, writeInFunc));
}

short
UF_StopRec (SYS_UWORD8 deviceNo)
{
    return (SER_fd_StopRec ());
}

short
UF_StartRec (SYS_UWORD8 deviceNo)
{
    return (SER_fd_StartRec ());
}

short
UF_GetLineState (SYS_UWORD8 deviceNo,
                 SYS_UWORD32 *state)
{
    return (SER_fd_GetLineState (state));
}

short
UF_SetLineState (SYS_UWORD8 deviceNo,
                 SYS_UWORD32 state,
                 SYS_UWORD32 mask)
{
    return (SER_fd_SetLineState (state, mask));
}

short
UF_CheckXEmpty (SYS_UWORD8 deviceNo)
{
    return (SER_fd_CheckXEmpty ());
}
#endif
