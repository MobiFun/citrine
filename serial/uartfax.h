/*******************************************************************************
 *
 * UARTFAX.H
 *
 * This driver allows to control the UARTs of chipset 1.5 for fax and data
 * It performs flow control: RTS/CTS, XON/XOFF.
 *
 * (C) Texas Instruments 1999 - 2003
 *
 ******************************************************************************/

#ifndef __UARTFAX_H__
#define __UARTFAX_H__

#ifndef C_EXTERN
  #if 1 //(OP_L1_STANDALONE == 1)
    #define C_EXTERN  extern
  #else
    #define C_EXTERN
  #endif
#endif

/*
 * In UAF_Init, a constant is used to identify the UART.
 */

typedef enum {
    UAF_UART_0, /* IrDA */
    UAF_UART_1  /* Modem */
    #if (CHIPSET == 12)
      , UAF_UART_2  /* Modem 2 */
    #endif
} T_fd_UartId;

#if (CHIPSET == 12)
  #define NUMBER_OF_FD_UART (3)
#else
  #define NUMBER_OF_FD_UART (2)
#endif

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

C_EXTERN T_FDRET UAF_Init (T_fd_UartId uartNo);

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

C_EXTERN T_FDRET UAF_Enable (T_fd_UartId uartNo,
                             SYS_BOOL enable);

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

C_EXTERN T_FDRET UAF_SetComPar (T_fd_UartId uartNo,
		                        T_baudrate baudrate,
         		                T_bitsPerCharacter bpc,
                  				T_stopBits sb,
		                        T_parity parity);

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

C_EXTERN T_FDRET UAF_SetBuffer (T_fd_UartId uartNo,
                    		    SYS_UWORD16 bufSize,
               			        SYS_UWORD16 rxThreshold,
                  			    SYS_UWORD16 txThreshold);

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

C_EXTERN T_FDRET UAF_SetFlowCtrl (T_fd_UartId uartNo,
                       			  T_flowCtrlMode fcMode,
                        		  SYS_UWORD8 XON,
                       			  SYS_UWORD8 XOFF);

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

C_EXTERN T_FDRET UAF_SetEscape (T_fd_UartId uartNo,
                    		    SYS_UWORD8 escChar,
                    		    SYS_UWORD16 guardPeriod);

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

C_EXTERN T_FDRET UAF_InpAvail (T_fd_UartId uartNo);

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

C_EXTERN T_FDRET UAF_OutpAvail (T_fd_UartId uartNo);

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

C_EXTERN T_FDRET UAF_EnterSleep (T_fd_UartId uartNo);

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

C_EXTERN T_FDRET UAF_WakeUp (T_fd_UartId uartNo);

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

C_EXTERN T_FDRET UAF_ReadData (T_fd_UartId uartNo,
                   			   T_suspendMode suspend,
                  			   void (readOutFunc (SYS_BOOL cldFromIrq,
                      		                      T_reInstMode *reInstall,
                                     			  SYS_UWORD8 nsource,
                                        		  SYS_UWORD8 *source[],
                                         		  SYS_UWORD16 size[],
                                         		  SYS_UWORD32 state)));

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

C_EXTERN T_FDRET UAF_WriteData (T_fd_UartId uartNo,
                       		    T_suspendMode suspend,
                                void (writeInFunc (SYS_BOOL cldFromIrq,
                                                   T_reInstMode *reInstall,
                                                   SYS_UWORD8 ndest,
                                                   SYS_UWORD8 *dest[],
                                                   SYS_UWORD16 size[])));

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

C_EXTERN T_FDRET UAF_StopRec (T_fd_UartId uartNo);

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

C_EXTERN T_FDRET UAF_StartRec (T_fd_UartId uartNo);

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

C_EXTERN T_FDRET UAF_GetLineState (T_fd_UartId uartNo,
                                   SYS_UWORD32 *state);

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

C_EXTERN T_FDRET UAF_SetLineState (T_fd_UartId uartNo,
                                   SYS_UWORD32 state,
                                   SYS_UWORD32 mask);

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

C_EXTERN void UAF_InterruptHandler (T_fd_UartId uart_id,
                                    SYS_UWORD8 interrupt_status);

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

C_EXTERN T_FDRET UAF_CheckXEmpty (T_fd_UartId uartNo);

/*******************************************************************************
 *
 *                              UAF_DTRInterruptHandler
 *
 * Purpose  : This function is only used on C-Sample. On this platform, the DTR
 *            signal is controlled with an I/O. A change of state of this signal
 *            is detected with an interrupt. This function is called when this
 *            interrupt occurs.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns  : none
 *
 ******************************************************************************/

void UAF_DTRInterruptHandler (void);

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

T_FDRET UAF_Exit (T_fd_UartId uartNo);

#undef C_EXTERN

#endif /* __UARTFAX_H__ */
