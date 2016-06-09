#ifndef GSI_H
#define GSI_H
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
|  Purpose :  These definitions provide a driver interface to multiple serial 
|             ports.
+----------------------------------------------------------------------------- 
*/ 


/*==== INCLUDES =============================================================*/

#include "gpfconf.h"
#include "gdi.h"

/*==== DEFINITIONS ==========================================================*/

/*
 * Baud rates
 */
#define GSI_BAUD_AUTO                                      ( 1)
#define GSI_BAUD_75                                        ( 2)
#define GSI_BAUD_150                                       ( 3)
#define GSI_BAUD_300                                       ( 4)
#define GSI_BAUD_600                                       ( 5)
#define GSI_BAUD_1200                                      ( 6)
#define GSI_BAUD_2400                                      ( 7)
#define GSI_BAUD_4800                                      ( 8)
#define GSI_BAUD_7200                                      ( 9)
#define GSI_BAUD_9600                                      (10)
#define GSI_BAUD_14400                                     (11)
#define GSI_BAUD_19200                                     (12)
#define GSI_BAUD_28800                                     (13)
#define GSI_BAUD_33900                                     (14)
#define GSI_BAUD_38400                                     (15)
#define GSI_BAUD_57600                                     (16)
#define GSI_BAUD_115200                                    (17)
#define GSI_BAUD_203125                                    (18)
#define GSI_BAUD_406250                                    (19)
#define GSI_BAUD_812500                                    (20)

/*
 * Bits per character
 */
#define GSI_CHAR5                                          ( 5)
#define GSI_CHAR6                                          ( 6)
#define GSI_CHAR7                                          ( 7)
#define GSI_CHAR8                                          ( 8)

/*
 * Stop bits
 */
#define GSI_STOP1                                          ( 1)
#define GSI_STOP15                                         (15)
#define GSI_STOP2                                          ( 2)

/*
 * Parity bit
 */
#define GSI_PARITYNO                                       ( 1)
#define GSI_PARITYODD                                      ( 2)
#define GSI_PARITYEVEN                                     ( 3)
#define GSI_PARITYSPACE                                    ( 4)

/*
 * Flow control
 */
#define GSI_FLOWNO                                         ( 1)
#define GSI_FLOWHW                                         ( 2)
#define GSI_FLOWSW                                         ( 3)

/*
 * Bit positions
 */
#define GSI_BRKLEN_POS                                     ( 0)
#define GSI_BRK_POS                                        (25)
#define GSI_DISC_POS                                       (26)
#define GSI_ESC_POS                                        (27)
#define GSI_RING_POS                                       (28)
#define GSI_X_POS                                          (29)
#define GSI_SB_POS                                         (30)
#define GSI_SA_POS                                         (31)

/*
 * Bit masks
 */
#define GSI_BRKLEN_MASK               (255UL << GSI_BRKLEN_POS)
#define GSI_BRK_MASK                       (1UL << GSI_BRK_POS)
#define GSI_DISC_MASK                     (1UL << GSI_DISC_POS)
#define GSI_ESC_MASK                       (1UL << GSI_ESC_POS)
#define GSI_RING_MASK                     (1UL << GSI_RING_POS)
#define GSI_X_MASK                           (1UL << GSI_X_POS)
#define GSI_SB_MASK                         (1UL << GSI_SB_POS)
#define GSI_SA_MASK                         (1UL << GSI_SA_POS)

/*
 * Size of the circular buffers used in the driver.
 */
#define GSI_MAX_BUFFER_SIZE                               (512)

/*==== TYPES ================================================================*/

/*
 * Device Control Block
 */
typedef struct {
  USHORT  Baud;
  UBYTE   DataBits;
  UBYTE   StopBits;
  UBYTE   Parity;
  UBYTE   RxFlowControl;
  UBYTE   TxFlowControl;
  USHORT  RxBufferSize;
  USHORT  TxBufferSize;
  USHORT  RxThreshold;
  USHORT  TxThreshold;
  UBYTE   XON;
  UBYTE   XOFF;
  UBYTE   EscChar;
  USHORT  GuardPeriod;
} T_GSI_DCB;

/*==== FUNCTION PROTOTYPES ==================================================*/

#ifdef _TARGET_
/*
+------------------------------------------------------------------------------
| Function    : GSI_Init
+------------------------------------------------------------------------------
| Description : The function initializes the module and the connected serial 
|               device.
|
| Parameters  : DeviceNo     - serial device number
|               DrvHandle    - unique handle for this device
|               CallbackFunc - callback function for this device
|               DrvInfo      - pointer to the driver parameters
|
| Return      : DRV_OK             - Initialization successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_INITIALIZED    - Driver already initialized
|
+------------------------------------------------------------------------------
*/
extern USHORT GSI_Init (UBYTE         DeviceNo,
                        USHORT        DrvHANDLE,
                        T_DRV_CB_FUNC CallbackFunc,
                        T_DRV_EXPORT  **DrvInfo);

/*
+------------------------------------------------------------------------------
| Function    : GSI_Exit
+------------------------------------------------------------------------------
| Description : The function is called when the driver functionality is no 
|               longer required. The driver terminates regardless of any 
|               outstanding data to be sent.
|
| Parameters  : DeviceNo - serial device number
|
| Return      : no return value
|
+------------------------------------------------------------------------------
*/
extern void GSI_Exit (UBYTE DeviceNo);

/*
+------------------------------------------------------------------------------
| Function    : GSI_Read
+------------------------------------------------------------------------------
| Description : This function copies received data into a caller provided 
|               buffer and returns the line status. It should always return 
|               immediately after copying the data, without waiting for any 
|               more data.
|
| Parameters  : DeviceNo - serial device number
|               Buffer   - buffer to copy the data
|               Length   - On call: size of Buffer; On return: copied bytes
|               State    - line states of the serial connection
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The driver is not yet configured
|
+------------------------------------------------------------------------------
*/
extern USHORT GSI_Read (UBYTE   DeviceNo,
                        void    *Buffer,
                        USHORT  *Length,
                        ULONG   *State);

/*
+------------------------------------------------------------------------------
| Function    : GSI_Write
+------------------------------------------------------------------------------
| Description : This functions copies the provided data into the buffer of the 
|               driver and sets the line states. This function must return 
|               immediately after copying, even if there is not enough space in 
|               the driver buffer to copy all provided data.
|
| Parameters  : DeviceNo - serial device number
|               Buffer   - buffer containing the data
|               Length   - On call: size of data; On return: copied bytes
|               State    - line states of the serial connection
|               Mask     - manipulated State bits
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The driver is not yet configured
|
+------------------------------------------------------------------------------
*/
extern USHORT GSI_Write (UBYTE  DeviceNo,
                         void   *Buffer,
                         USHORT *Length,
                         ULONG  State,
                         ULONG  Mask);

/*
+------------------------------------------------------------------------------
| Function    : GSI_Look
+------------------------------------------------------------------------------
| Description : This function copies received data into a caller provided 
|               buffer without delete the data in the driver buffer. It should 
|               always return immediately after copying the data, without 
|               waiting for any more data.
|
| Parameters  : DeviceNo - serial device number
|               Buffer   - buffer to copy the data
|               Length   - On call: size of Buffer; On return: copied bytes
|               State    - line states of the serial connection
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The driver is not yet configured
|
+------------------------------------------------------------------------------
*/
extern USHORT GSI_Look (UBYTE   DeviceNo,
                        void    *Buffer,
                        USHORT  *Length,
                        ULONG   *State);

/*
+------------------------------------------------------------------------------
| Function    : GSI_Clear
+------------------------------------------------------------------------------
| Description : This function is used to clear the device internal buffers.
|
| Parameters  : DeviceNo   - serial device number
|               BufferType - buffer to be cleared
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_INPROCESS      - The driver is busy clearing the buffers
|
+------------------------------------------------------------------------------
*/
extern USHORT GSI_Clear (UBYTE  DeviceNo,
                         USHORT BufferType);

/*
+------------------------------------------------------------------------------
| Function    : GSI_Flush
+------------------------------------------------------------------------------
| Description : This function is used to flush the device internal transmit 
|               buffer.
|
| Parameters  : DeviceNo - serial device number
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_INPROCESS      - The driver is busy clearing the buffers
|               DRV_NOTCONFIGURED  - The driver is not yet configured
|
+------------------------------------------------------------------------------
*/
extern USHORT GSI_Flush (UBYTE  DeviceNo);

/*
+------------------------------------------------------------------------------
| Function    : GSI_SetSignal
+------------------------------------------------------------------------------
| Description : This function enables indication signals from the driver. These 
|               signals are sent via the device callback function.
|
| Parameters  : DeviceNo   - serial device number
|               SignalType - Signal type to be set
|
| Return      : DRV_OK                  - Function successful
|               DRV_INVALID_PARAMS      - parameters out of range
|               DRV_INTERNAL_ERROR      - Internal driver error
|               DRV_SIGFCT_NOTAVAILABLE - no event signaling functionality
|
+------------------------------------------------------------------------------
*/
extern USHORT GSI_SetSignal (UBYTE  DeviceNo,
                             USHORT SignalType);

/*
+------------------------------------------------------------------------------
| Function    : GSI_ResetSignal
+------------------------------------------------------------------------------
| Description : This function disables indication signals from the driver. 
|
| Parameters  : DeviceNo   - serial device number
|               SignalType - Signal type to be reset
|
| Return      : DRV_OK                  - Function successful
|               DRV_INVALID_PARAMS      - parameters out of range
|               DRV_INTERNAL_ERROR      - Internal driver error
|               DRV_SIGFCT_NOTAVAILABLE - no event signaling functionality
|
+------------------------------------------------------------------------------
*/
extern USHORT GSI_ResetSignal (UBYTE  DeviceNo,
                               USHORT SignalType);

/*
+------------------------------------------------------------------------------
| Function    : GSI_SetConfig
+------------------------------------------------------------------------------
| Description : This function configures the serial device.
|
| Parameters  : DeviceNo - serial device number
|               DCBPtr   - pointer to the driver control block
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - parameters out of range
|               DRV_INTERNAL_ERROR - Internal driver error
|
+------------------------------------------------------------------------------
*/
extern USHORT GSI_SetConfig (UBYTE      DeviceNo,
                             T_GSI_DCB  *DCBPtr);

/*
+------------------------------------------------------------------------------
| Function    : GSI_GetConfig
+------------------------------------------------------------------------------
| Description : This function reads the serial device configuration.
|
| Parameters  : DeviceNo - serial device number
|               DCBPtr   - pointer to the driver control block
|
| Return      : DRV_OK             - Function successful
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The driver is not yet configured
|
+------------------------------------------------------------------------------
*/
extern USHORT GSI_GetConfig (UBYTE      DeviceNo,
                             T_GSI_DCB  *DCBPtr);

/*
+------------------------------------------------------------------------------
| Function    : GSI_Callback
+------------------------------------------------------------------------------
| Description : This function is needed for cascaded drivers. This function 
|               must not be confused with the parameter CallbackFunc passed to 
|               GSI_Init().
|
| Parameters  : Signal - pointer to the signal information data
|
| Return      : no return value
|
+------------------------------------------------------------------------------
*/
extern void GSI_Callback (T_DRV_SIGNAL  *Signal);

#endif /* _TARGET_ */
#endif /* !GSI_H */
