/*******************************************************************************
 *
 * SERIALSWITCH.H
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

#ifndef __SERIALSWITCH_H__
#define __SERIALSWITCH_H__

#include "../include/config.h"
#include "../include/sys_types.h"

#include "traceswitch.h"
#include "faxdata.h"

#ifndef C_EXTERN
  #if 1 //(OP_L1_STANDALONE)
    #define C_EXTERN  extern
  #else
    #define C_EXTERN
  #endif
#endif

/*
 * Constants used to identify the serial data flows.
 */

#define SER_FLOW_1  (0)
#define SER_FLOW_2  (1)
#define SER_FLOW_3  (2)
#define SER_FLOW_4  (3)

#define SER_PROTOCOL_STACK (SER_FLOW_1)
#define SER_LAYER_1        (SER_FLOW_2) 
#define SER_FAX_DATA       (SER_FLOW_3)
#define SER_BLUETOOTH_HCI  (SER_FLOW_4)

#define SER_HWTEST         (SER_FLOW_1)
#define SER_SERIAL_TEST_1  (SER_FLOW_3)

 
#define SER_MAX_NUMBER_OF_FLOWS  (4)

#define SER_MAX_NUMBER_OF_CFG  (16)

/*
 * Type used to define the various drivers configuration
 * available, according to the UART devices.
 */

typedef enum {
    /* Trace Flow */
    DUMMY_TRACE,             /* = 0 */
    UART_IRDA_TRACE,         /* = 1 */
    UART_MODEM_TRACE,        /* = 2 */
    #if (CHIPSET == 12)
      UART_MODEM2_TRACE,     /* = 3 */
    #endif
    /* AT-Commands/Fax & Data Flow */
    DUMMY_FAX_DATA = 4,      /* = 4 */
    /* UART IrDA F&D Driver, not supported - should be = 5 */
    UART_MODEM_FAX_DATA = 6, /* = 6 */
    #if (CHIPSET == 12)
      /* UART Modem2 F&D Driver, not supported - should be = 7 */
    #endif
    /* Bluetooth HCI Flow */
    DUMMY_BT_HCI = 8,        /* = 8 */
    UART_IRDA_BT_HCI,        /* = 9 */
    UART_MODEM_BT_HCI        /* = A */
    #if (CHIPSET == 12)
      , UART_MODEM2_BT_HCI   /* = B */
    #endif
} T_SerialDriver;

/*
 * Type used to describe a defined serial configuration;
 * Each field is a 4 bits field representing one serial flow.
 *
 * T_DefinedSerialConfig : [ flow_1 | flow_2 | flow_3 | flow_4 ]
 *                         15    12 11     8  7     4  3     0
 */

typedef struct {

    unsigned int flow_4 :4;
    unsigned int flow_3 :4;
    unsigned int flow_2 :4;
    unsigned int flow_1 :4;
    
} T_DefinedSerialConfig;


/*
 * Type used to describe all serial configuration informations
 * of a defined application:
 *     - the default configuration to set up, if the current one is
 *       not valid,
 *     - the number of allowed serial configurations,
 *     - the entire allowed serial configurations.
 */

typedef struct {

    T_DefinedSerialConfig default_config;
	SYS_UWORD8         num_config;
	T_DefinedSerialConfig allowed_config[SER_MAX_NUMBER_OF_CFG];

} T_AppliSerialInfo;


/*
 * Functions prototypes.
 */

#if (DP==1)
  void SER_InitSerialConfig (int application_id);
#else
  C_EXTERN  void SER_InitSerialConfig (const T_AppliSerialInfo *serial_info);
#endif //DP

C_EXTERN SYS_BOOL SER_UartSleepStatus (void);

C_EXTERN void SER_WakeUpUarts (void);

void SER_restart_uart_sleep_timer (void);

void SER_activate_timer_hisr (void);

#if (DP==1)
  void SER_tr_Init (int serial_data_flow,
                    int baudrate,
                    void (callback_function (void)));
#else
  C_EXTERN  void SER_tr_Init (int serial_data_flow,
                              T_tr_Baudrate baudrate,
                              void (callback_function (void)));
#endif //DP

C_EXTERN SYS_UWORD32 SER_tr_ReadNChars (int serial_data_flow,
                                        char *buffer,
                                        SYS_UWORD32 chars_to_read);

C_EXTERN SYS_UWORD32 SER_tr_ReadNBytes (int serial_data_flow,
                                        char *buffer,
                                        SYS_UWORD32 chars_to_read,
                                        SYS_BOOL *eof_detected);

C_EXTERN SYS_UWORD32 SER_tr_WriteNChars (int serial_data_flow,
                                         char *buffer,
                                         SYS_UWORD32 chars_to_write);

C_EXTERN SYS_UWORD32 SER_tr_EncapsulateNChars (int serial_data_flow,
                                               char *buffer,
                                               SYS_UWORD32 chars_to_write);

C_EXTERN SYS_UWORD32 SER_tr_WriteNBytes (int serial_data_flow,
                                         SYS_UWORD8 *buffer,
                                         SYS_UWORD32 chars_to_write);

C_EXTERN void SER_tr_WriteChar (int serial_data_flow,
                                char character);

C_EXTERN SYS_BOOL SER_tr_EnterSleep (int serial_data_flow);

C_EXTERN void SER_tr_WakeUp (int serial_data_flow);

C_EXTERN void SER_tr_WriteString (int serial_data_flow,
                                  char *buffer);

#define T_UFRET T_FDRET

#define UF_DEVICE_0 (0)

#define UF_OK             FD_OK
#define UF_SUSPENDED      FD_SUSPENDED
#define UF_NOT_SUPPORTED  FD_NOT_SUPPORTED
#define UF_NOT_READY      FD_NOT_READY
#define UF_INTERNAL_ERROR FD_INTERNAL_ERR

#define UF_LINE_ON  FD_LINE_ON
#define UF_LINE_OFF FD_LINE_OFF

#define UF_MAX_BUFFER_SIZE FD_MAX_BUFFER_SIZE

#define UF_BAUD_AUTO   FD_BAUD_AUTO
#define UF_BAUD_75     FD_BAUD_75
#define UF_BAUD_150    FD_BAUD_150
#define UF_BAUD_300    FD_BAUD_300
#define UF_BAUD_600    FD_BAUD_600
#define UF_BAUD_1200   FD_BAUD_1200
#define UF_BAUD_2400   FD_BAUD_2400
#define UF_BAUD_4800   FD_BAUD_4800
#define UF_BAUD_7200   FD_BAUD_7200
#define UF_BAUD_9600   FD_BAUD_9600
#define UF_BAUD_14400  FD_BAUD_14400
#define UF_BAUD_19200  FD_BAUD_19200
#define UF_BAUD_28800  FD_BAUD_28800
#define UF_BAUD_33900  FD_BAUD_33900
#define UF_BAUD_38400  FD_BAUD_38400
#define UF_BAUD_57600  FD_BAUD_57600
#define UF_BAUD_115200 FD_BAUD_115200
#define UF_BAUD_203125 FD_BAUD_203125
#define UF_BAUD_406250 FD_BAUD_406250
#define UF_BAUD_812500 FD_BAUD_812500

C_EXTERN T_FDRET SER_fd_Init (void);

#if (DP==0)
  C_EXTERN  T_FDRET SER_fd_Initialize (void);
#endif

C_EXTERN T_FDRET SER_fd_Enable (SYS_BOOL enable);

C_EXTERN T_FDRET SER_fd_SetComPar (T_baudrate baudrate,
                                   T_bitsPerCharacter bpc,
                                   T_stopBits sb,
                                   T_parity parity);

C_EXTERN T_FDRET SER_fd_SetBuffer (SYS_UWORD16 bufSize,
                                   SYS_UWORD16 rxThreshold,
                                   SYS_UWORD16 txThreshold);

C_EXTERN T_FDRET SER_fd_SetFlowCtrl (T_flowCtrlMode fcMode,
                                     SYS_UWORD8 XON,
                                     SYS_UWORD8 XOFF);

C_EXTERN T_FDRET SER_fd_SetEscape (char escChar,
                                   SYS_UWORD16 guardPeriod);

C_EXTERN T_FDRET SER_fd_InpAvail (void);

C_EXTERN T_FDRET SER_fd_OutpAvail (void);

C_EXTERN T_FDRET SER_fd_EnterSleep (void);

C_EXTERN T_FDRET SER_fd_WakeUp (void);

C_EXTERN T_FDRET SER_fd_ReadData (T_suspendMode suspend,
                                  void (readOutFunc (SYS_BOOL cldFromIrq,
                                                     T_reInstMode *reInstall,
                                                     SYS_UWORD8 nsource,
                                                     SYS_UWORD8 *source[],
                                                     SYS_UWORD16 size[],
                                                     SYS_UWORD32 state)));

C_EXTERN T_FDRET SER_fd_WriteData (T_suspendMode suspend,
                                   void (writeInFunc (SYS_BOOL cldFromIrq,
                                                      T_reInstMode *reInstall,
                                                      SYS_UWORD8 ndest,
                                                      SYS_UWORD8 *dest[],
                                                      SYS_UWORD16 size[])));

C_EXTERN T_FDRET SER_fd_StopRec (void);

C_EXTERN T_FDRET SER_fd_StartRec (void);

C_EXTERN T_FDRET SER_fd_GetLineState (SYS_UWORD32 *state);

C_EXTERN T_FDRET SER_fd_SetLineState (SYS_UWORD32 state,
                                      SYS_UWORD32 mask);

#if (DP==0)
  C_EXTERN T_FDRET SER_fd_CheckXEmpty (void);
#endif

#ifdef BTEMOBILE
  C_EXTERN T_HCI_RET SER_bt_Init (void);

  C_EXTERN T_HCI_RET SER_bt_Start (void);

  C_EXTERN T_HCI_RET SER_bt_Stop (void);

  C_EXTERN T_HCI_RET SER_bt_Kill (void);

  C_EXTERN T_HCI_RET SER_bt_SetBaudrate (UINT8 baudrate);

  C_EXTERN T_HCI_RET SER_bt_TransmitPacket (void *uart_sco_tx_buffer);

  C_EXTERN SYS_BOOL  SER_bt_EnterSleep (void);

  C_EXTERN void SER_bt_WakeUp (void);
#endif

#if ((CHIPSET == 2) || (CHIPSET == 3))
   C_EXTERN void SER_uart_handler (void);
#elif ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
   C_EXTERN void SER_uart_modem_handler (void);
   C_EXTERN void SER_uart_irda_handler (void);
#endif
#if (CHIPSET == 12)
   C_EXTERN void SER_uart_modem2_handler (void);
#endif

#if (DP==1)
  T_FDRET UF_Init (int serial_data_flow);
  T_FDRET UF_Enable (int serial_data_flow,
                     SYS_BOOL enable);
  T_FDRET UF_SetComPar (int serial_data_flow,
                        T_baudrate baudrate,
                        T_bitsPerCharacter bpc,
                        T_stopBits sb,
                        T_parity parity);
  T_FDRET UF_SetBuffer (int serial_data_flow,
                        SYS_UWORD16 bufSize,
                        SYS_UWORD16 rxThreshold,
                        SYS_UWORD16 txThreshold);
  T_FDRET UF_SetFlowCtrl (int serial_data_flow,
                          T_flowCtrlMode fcMode,
                          SYS_UWORD8 XON,
                          SYS_UWORD8 XOFF);
  T_FDRET UF_SetEscape (int serial_data_flow,
                        char escChar,
                        SYS_UWORD16 guardPeriod);
  T_FDRET UF_InpAvail (int serial_data_flow);
  T_FDRET UF_OutpAvail (int serial_data_flow);
  T_FDRET UF_ReadData (int serial_data_flow,
                       T_suspendMode suspend,
                       void (readOutFunc (SYS_BOOL cldFromIrq,
                                          T_reInstMode *reInstall,
                                          SYS_UWORD8 nsource,
                                          SYS_UWORD8 *source[],
                                          SYS_UWORD16 size[],
                                          SYS_UWORD32 state)));
  T_FDRET UF_WriteData (int uartNo,
                        T_suspendMode suspend,
                        void (writeInFunc (SYS_BOOL cldFromIrq,
                                           T_reInstMode *reInstall,
                                           SYS_UWORD8 ndest,
                                           SYS_UWORD8 *dest[],
                                           SYS_UWORD16 size[])));
  T_FDRET UF_StopRec (int serial_data_flow);
  T_FDRET UF_StartRec (int serial_data_flow);
  T_FDRET UF_GetLineState (int serial_data_flow,
                           SYS_UWORD32 *state);
  T_FDRET UF_SetLineState (int serial_data_flow,
                           SYS_UWORD32 state,
                           SYS_UWORD32 mask);
  T_FDRET UF_CheckXEmpty (int serial_data_flow);
  T_FDRET UF_EnterSleep (int serial_data_flow);
  T_FDRET UF_WakeUp (int serial_data_flow);
#endif //DP

/*
 * Functions used for Dynamic Switch.
 */

SYS_BOOL SER_WriteConfig (char *new_config,
                            SYS_BOOL write_to_flash);

SYS_BOOL SER_ImmediateSwitch (void);

/*
 * Constants and macros used by Condat.
 * Condat uses a serial device for the protocol stack trace.
 */

#ifndef __SERIALSWITCH_C__

#define UT_DEVICE_0 (0)

#define UT_BAUD_406250 TR_BAUD_406250
#define UT_BAUD_115200 TR_BAUD_115200
#define UT_BAUD_57600  TR_BAUD_57600
#define UT_BAUD_38400  TR_BAUD_38400
#define UT_BAUD_33900  TR_BAUD_33900
#define UT_BAUD_28800  TR_BAUD_28800
#define UT_BAUD_19200  TR_BAUD_19200
#define UT_BAUD_14400  TR_BAUD_14400
#define UT_BAUD_9600   TR_BAUD_9600
#define UT_BAUD_4800   TR_BAUD_4800
#define UT_BAUD_2400   TR_BAUD_2400
#define UT_BAUD_1200   TR_BAUD_1200
#define UT_BAUD_600    TR_BAUD_600
#define UT_BAUD_300    TR_BAUD_300
#define UT_BAUD_150    TR_BAUD_150
#define UT_BAUD_75     TR_BAUD_75

#define UT_Init(A,B,C) SER_tr_Init (SER_PROTOCOL_STACK, B, C)

#define UT_ReadNChars(A,B,C) SER_tr_ReadNChars (SER_PROTOCOL_STACK, B, C)

#define UT_WriteNChars(A,B,C) SER_tr_WriteNChars (SER_PROTOCOL_STACK, B, C)

#define UT_WriteChar(A,B) SER_tr_WriteChar (SER_PROTOCOL_STACK, B)

#define UT_WriteString(A,B) SER_tr_WriteString (SER_PROTOCOL_STACK, B)

#if (DP==0)
  #define UF_Init(A) SER_fd_Init ()

  #define UF_Enable(A,B) SER_fd_Enable (B)

  #define UF_SetComPar(A,B,C,D,E) SER_fd_SetComPar (B, C, D, E)

  #define UF_SetBuffer(A,B,C,D) SER_fd_SetBuffer (B, C, D)

  #define UF_SetFlowCtrl(A,B,C,D) SER_fd_SetFlowCtrl (B, C, D)

  #define UF_SetEscape(A,B,C) SER_fd_SetEscape (B, C)

  #define UF_InpAvail(A) SER_fd_InpAvail ()

  #define UF_OutpAvail(A) SER_fd_OutpAvail ()

  #define UF_ReadData(A,B,C) SER_fd_ReadData (B, C)

  #define UF_WriteData(A,B,C) SER_fd_WriteData (B, C)

  #define UF_StopRec(A) SER_fd_StopRec()

  #define UF_StartRec(A) SER_fd_StartRec ()

  #define UF_GetLineState(A,B) SER_fd_GetLineState (B) 

  #define UF_SetLineState(A,B,C) SER_fd_SetLineState (B, C)

  #define UF_CheckXEmpty(A) SER_fd_CheckXEmpty ()
#endif //DP

#endif /* __SERIALSWITCH_C__ */

#undef C_EXTERN

#endif /* __SERIALSWITCH_H__ */
