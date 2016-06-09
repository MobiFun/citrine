/*
+-----------------------------------------------------------------------------
|  Copyright 2004 Texas Instruments Berlin, AG
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
|  Purpose : General definitions of DIO driver interface
+-----------------------------------------------------------------------------
*/

#ifndef __DIO_H__
#define __DIO_H__

/*==== INCLUDES =============================================================*/

#include "gdi.h"

/*==== DEFINITIONS ==========================================================*/

/*
 *  Device Type
 */ 
#define DIO_DATA_SER     0x00800000 /* device can transfer serial data       */
#define DIO_DATA_PKT     0x00400000 /* device can transfer packet data       */
#define DIO_DATA_MUX     0x00200000 /* device can start a multiplexer        */
#define DIO_TYPE_ID      0x0000FF00 /* id to separate different capabilities */

#define DIO_TYPE_SER     (DIO_DATA_SER | 0x00000100)                /* serial device */
#define DIO_TYPE_SER_MUX (DIO_DATA_SER | DIO_DATA_MUX | 0x00000100) /* mux device    */
#define DIO_TYPE_PKT     (DIO_DATA_PKT | 0x00000100)                /* packet device */

/*
 *  Device Flags
 */
#define DIO_FLAG_SLEEP   0x80000000 /* sleep state allowed                   */

/*
 * Baud rates 
 */
#define DIO_BAUD_921600  0x00800000
#define DIO_BAUD_812500  0x00400000
#define DIO_BAUD_460800  0x00200000
#define DIO_BAUD_406250  0x00100000
#define DIO_BAUD_230400  0x00080000
#define DIO_BAUD_203125  0x00040000
#define DIO_BAUD_115200  0x00020000
#define DIO_BAUD_57600   0x00010000
#define DIO_BAUD_38400   0x00008000
#define DIO_BAUD_33900   0x00004000
#define DIO_BAUD_28800   0x00002000
#define DIO_BAUD_19200   0x00001000
#define DIO_BAUD_14400   0x00000800
#define DIO_BAUD_9600    0x00000400
#define DIO_BAUD_7200    0x00000200
#define DIO_BAUD_4800    0x00000100
#define DIO_BAUD_2400    0x00000080
#define DIO_BAUD_1200    0x00000040
#define DIO_BAUD_600     0x00000020
#define DIO_BAUD_300     0x00000010
#define DIO_BAUD_150     0x00000008
#define DIO_BAUD_110     0x00000004
#define DIO_BAUD_75      0x00000002
#define DIO_BAUD_AUTO    0x00000001 /* automatic baud rate detection */

/*
 * Character framing
 */
#define DIO_CF_7N1    0x00000800
#define DIO_CF_7S1    0x00000400
#define DIO_CF_7M1    0x00000200
#define DIO_CF_7E1    0x00000100
#define DIO_CF_7O1    0x00000080
#define DIO_CF_7N2    0x00000040
#define DIO_CF_8N1    0x00000020
#define DIO_CF_8S1    0x00000010
#define DIO_CF_8M1    0x00000008
#define DIO_CF_8E1    0x00000004
#define DIO_CF_8O1    0x00000002
#define DIO_CF_8N2    0x00000001

/*
 * Types of flow control 
 */
#define DIO_FLOW_XTR_CTS   0x00000800  /* TX= transp. XON/XOFF   RX=CTS      */
#define DIO_FLOW_XTR_XOFF  0x00000400  /* TX= transp. XON/XOFF   RX=XON/XOFF */
#define DIO_FLOW_XTR_NO    0x00000200  /* TX= transp. XON/XOFF   RX=OFF      */
#define DIO_FLOW_RTS_CTS   0x00000100  /* TX=RTS                 RX=CTS      */
#define DIO_FLOW_RTS_XOFF  0x00000080  /* TX=RTS                 RX=XON/XOFF */
#define DIO_FLOW_RTS_NO    0x00000040  /* TX=RTS                 RX=OFF      */
#define DIO_FLOW_XON_CTS   0x00000020  /* TX=XON/XOFF            RX=CTS      */
#define DIO_FLOW_XON_XOFF  0x00000010  /* TX=XON/XOFF            RX=XON/XOFF */
#define DIO_FLOW_XON_NO    0x00000008  /* TX=XON/XOFF            RX=OFF      */
#define DIO_FLOW_NO_CTS    0x00000004  /* TX=OFF                 RX=CTS      */
#define DIO_FLOW_NO_XOFF   0x00000002  /* TX=OFF                 RX=XON/XOFF */
#define DIO_FLOW_NONE      0x00000001  /* TX=OFF                 RX=OFF      */

/*
 * Serial device flags
 */
#define DIO_FLAG_SER_ESC   0x80000000  /* device supports esc seq detection  */

/*
 *  MUX mode
 */
#define DIO_MUX_I      0x00000008  /* advanced option with I frames   */
#define DIO_MUX_UI     0x00000004  /* advanced option with UI frames  */
#define DIO_MUX_UIH    0x00000002  /* advanced option with UIH frames */
#define DIO_MUX_BASIC  0x00000001  /* basic option                    */

/*
 *  Sleep Mode
 */
#define DIO_SLEEP_ENABLE   0x01   /* enter sleep mode if possible */
#define DIO_SLEEP_DISABLE  0x02   /* do not enter sleep mode      */

/*
 *  Special Guard Period
 */
#define DIO_ESC_OFF 0x0000   /* turn escape sequence detection off */

/*
 *  Device Mode
 */
#define DIO_MODE_MUX  0x00000002  /* acts as 27.010 multiplexer device */
#define DIO_MODE_SER  0x00000001  /* acts as serial device             */

/*
 * Control Type
 */
#define DIO_CTRL_LINES 0x0001    /* associated structure is T_DIO_CTRL_LINES */
#define DIO_CTRL_MUX   0x0002    /* associated structure is T_DIO_CTRL_MUX   */
 
/* 
 * Line State definition & control structure.
 */
#define DIO_SA          0x80000000  /* state: read=DTR  write-DSR            */
#define DIO_SB          0x40000000  /* state: read=RTS  write=DCD            */
#define DIO_X           0x20000000  /* state: read=RTS  write=CTS            */
#define DIO_RING        0x10000000  /* state: ring indicator                 */
#define DIO_ESC         0x08000000  /* state: escape sequence detested.      */
#define DIO_MUX_STOPPED 0x04000000  /* multiplexer stopped                   */
#define DIO_BRK         0x02000000  /* state: break received/to be sent      */
#define DIO_BRKLEN      0x000000FF  /* state: break signal # of characters   */

/*==== TYPES ================================================================*/

/*
 *  Device Capabilities
 */ 
typedef struct 
{
  U32         device_type; 
  U32         device_flags; 
  U16         mtu_control; 
  U16         mtu_data; 
  char const  *driver_name; 
} T_DIO_CAP;

typedef struct 
{
  U32         device_type; 
  U32         device_flags; 
  U16         mtu_control; 
  U16         mtu_data; 
  char const  *driver_name; 
  U32         baudrate_auto; 
  U32         baudrate_fixed; 
  U32         char_frame; 
  U32         flow_control; 
  U32         ser_flags; 
} T_DIO_CAP_SER;

typedef struct 
{
  U32         device_type; 
  U32         device_flags; 
  U16         mtu_control; 
  U16         mtu_data; 
  char const  *driver_name; 
  U32         baudrate_auto; 
  U32         baudrate_fixed; 
  U32         char_frame; 
  U32         flow_control; 
  U32         ser_flags; 
  U32         mux_mode;
} T_DIO_CAP_SER_MUX;

typedef struct 
{
  U32         device_type; 
  U32         device_flags; 
  U16         mtu_control; 
  U16         mtu_data; 
  char const  *driver_name; 
} T_DIO_CAP_PKT;

/*
 *  Device Control Block
 */
typedef struct
{
  U32                       device_type;              /*<  0:  4> device identifier with general capability information */
  U8                        sleep_mode;               /*<  4:  1> enter sleep mode is possible or not                */
  U8                        _align0;                  /*<  5:  1> alignment                                          */
  U8                        _align1;                  /*<  6:  1> alignment                                          */
  U8                        _align2;                  /*<  7:  1> alignment                                          */
} T_DIO_DCB;

typedef struct
{
  U32                       device_type;              /*<  0:  4> device identifier with general capability information */
  U8                        sleep_mode;               /*<  4:  1> enter sleep mode is possible or not                */
  U8                        _align0;                  /*<  5:  1> alignment                                          */
  U8                        _align1;                  /*<  6:  1> alignment                                          */
  U8                        _align2;                  /*<  7:  1> alignment                                          */
  U32                       baudrate;                 /*<  8:  4> user set baudrate                                  */
  U32                       char_frame;               /*< 12:  4> supported character framing                        */
  U32                       flow_control;             /*< 16:  4> supported modes of flow control                    */
  U8                        xon;                      /*< 20:  1> set/reset XON for flow control                     */
  U8                        xoff;                     /*< 21:  1> set/reset XOFF for flow control                    */
  U8                        esc_char;                 /*< 22:  1> ASCII character which is used in an escape sequence */
  U8                        _align3;                  /*< 23:  1> alignment                                          */
  U16                       guard_period;             /*< 24:  2> duration value for escape sequence                 */
  U8                        _align4;                  /*< 26:  1> alignment                                          */
  U8                        _align5;                  /*< 27:  1> alignment                                          */
} T_DIO_DCB_SER;

typedef struct
{
  U32                       device_type;              /*<  0:  4> device identifier with general capability information */
  U8                        sleep_mode;               /*<  4:  1> enter sleep mode is possible or not                */
  U8                        _align0;                  /*<  5:  1> alignment                                          */
  U8                        _align1;                  /*<  6:  1> alignment                                          */
  U8                        _align2;                  /*<  7:  1> alignment                                          */
  U32                       baudrate;                 /*<  8:  4> user set baudrate                                  */
  U32                       char_frame;               /*< 12:  4> supported character framing                        */
  U32                       flow_control;             /*< 16:  4> supported modes of flow control                    */
  U8                        xon;                      /*< 20:  1> set/reset XON for flow control                     */
  U8                        xoff;                     /*< 21:  1> set/reset XOFF for flow control                    */
  U8                        esc_char;                 /*< 22:  1> ASCII character which is used in an escape sequence */
  U8                        _align3;                  /*< 23:  1> alignment                                          */
  U16                       guard_period;             /*< 24:  2> duration value for escape sequence                 */
  U8                        _align4;                  /*< 26:  1> alignment                                          */
  U8                        _align5;                  /*< 27:  1> alignment                                          */
  U32                       device_mode;              /*< 28:  4> work mode of device                                */
  U32                       mux_mode;                 /*< 32:  4> supported multiplexer modes                        */
  U16                       n1;                       /*< 36:  2> max frame size of mux frame                        */
  U8                        n2;                       /*< 38:  1> max number of retransmissions                      */
  U8                        t1;                       /*< 39:  1> acknowledgement timer                              */
  U8                        t2;                       /*< 40:  1> response timer                                     */
  U8                        t3;                       /*< 41:  1> wake up response timer                             */
  U8                        k;                        /*< 42:  1> windows size                                       */
  U8                        _align6;                  /*< 43:  1> alignment                                          */
} T_DIO_DCB_SER_MUX;

typedef struct
{
  U32                       device_type;              /*<  0:  4> device identifier with general capability information */
  U8                        sleep_mode;               /*<  4:  1> enter sleep mode is possible or not                */
  U8                        _align0;                  /*<  5:  1> alignment                                          */
  U8                        _align1;                  /*<  6:  1> alignment                                          */
  U8                        _align2;                  /*<  7:  1> alignment                                          */
} T_DIO_DCB_PKT;

/*
 *  Data Control
 */
typedef struct 
{
  U16 control_type ; 
  U16 length ; 
}T_DIO_CTRL ; 

typedef struct 
{
  U16 control_type; 
  U16 length;
  U32 state;
} T_DIO_CTRL_LINES; 

typedef struct 
{
  U16 control_type ; 
  U16 length ; 
  U32 state;
}T_DIO_CTRL_MUX ; 

/*
 *  DIO Data format.
 */
typedef struct
{
  U8                        _align0;                  /*<  0:  1> alignment                                          */
  U8                        _align1;                  /*<  1:  1> alignment                                          */
  U16                       c_data;                   /*<  2:  2> counter                                            */
  U8                        *ptr_data;                /*<  4:  4> pointer to  pointer to the first byte of the data buffer segment */
} T_dio_segment;

typedef struct
{
  U16                       length;                   /*<  0:  2> len of dio_ctrl                                    */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        c_dio_segment;            /*<  3:  1> counter                                            */
  T_dio_segment             *ptr_dio_segment;         /*<  4:  4> pointer to Structured Element                      */
} T_dio_buffer;

/*==== PROTOTYPES ===========================================================*/

/*
+------------------------------------------------------------------------------
| Function    : dio_init
+------------------------------------------------------------------------------
| Description : The function initializes the interface and the drivers.
|
| Parameters  : none
|
| Return      : DRV_OK          - Initialization successful
|               DRV_INITIALIZED - Interface already initialized
|               DRV_INITFAILURE - Initialization failed
|
+------------------------------------------------------------------------------
*/
extern U16 dio_init(void);

/*
+------------------------------------------------------------------------------
| Function    : dio_user_init
+------------------------------------------------------------------------------
| Description : The function sets the signal callback of a DIO user.
|
| Parameters  : user_name       - DIO user name
|               drv_handle      - unique handle for this user
|               signal_callback - callback function for this user
|
| Return      : DRV_OK             - Callback successfully set
|               DRV_INVALID_PARAMS - The specified user does not exist
|               DRV_INTERNAL_ERROR - Internal error
|               DRV_NOTCONFIGURED  - DIO interface is not yet initialized.
|
+------------------------------------------------------------------------------
*/
extern U16 dio_user_init(char const *      user_name,
						 U16               drv_handle,
						 T_DRV_CB_FUNC signal_callback);

/*
+------------------------------------------------------------------------------
| Function    : dio_user_exit
+------------------------------------------------------------------------------
| Description : Termination of User Operation.
|
| Parameters  : user_name - DIO user name
|
| Return      : DRV_OK             - User operation successfully terminated.
|               DRV_INVALID_PARAMS - User operation can not be terminated yet.
|               DRV_INTERNAL_ERROR - Internal error
|
+------------------------------------------------------------------------------
*/
extern U16 dio_user_exit(char const * user_name);

/*
+------------------------------------------------------------------------------
| Function    : dio_exit
+------------------------------------------------------------------------------
| Description : Termination of the Interface.
|
| Parameters  : none
|
| Return      : none
|
+------------------------------------------------------------------------------
*/
extern void dio_exit(void);

/*
+------------------------------------------------------------------------------
| Function    : dio_set_rx_buffer
+------------------------------------------------------------------------------
| Description : This function provides a receive buffer to the driver.
|
| Parameters  : device - device number
|               buffer - buffer to copy received data in it
|
| Return      : DRV_OK             - Function successful
|               DRV_BUFFER_FULL    - Buffer queue full.
|               DRV_INVALID_PARAMS - The specified device does not exist or
|                                    the data buffer is not big enough.
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The device is not yet configured
|
+------------------------------------------------------------------------------
*/
extern U16 dio_set_rx_buffer(U32            device,
							 T_dio_buffer * buffer);

/*
+------------------------------------------------------------------------------
| Function    : dio_read
+------------------------------------------------------------------------------
| Description : This function returns a receive buffer and control information.
|
| Parameters  : device       - device number
|               control_info - control information from the driver
|               buffer       - buffer with received data in it
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The device is not yet configured
|
+------------------------------------------------------------------------------
*/
extern U16 dio_read(U32             device,
					T_DIO_CTRL *    control_info,
					T_dio_buffer ** buffer);

/*
+------------------------------------------------------------------------------
| Function    : dio_write
+------------------------------------------------------------------------------
| Description : This function provides a send buffer to the driver which
|               contains data to send.
|
| Parameters  : device       - device number
|               control_info - control information for the driver
|               buffer       - buffer with data to send
|
| Return      : DRV_OK             - Function successful
|               DRV_BUFFER_FULL    - Buffer queue full.
|               DRV_INVALID_PARAMS - The specified device does not exist or
|                                    the data buffer to big.
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The device is not yet configured
|
+------------------------------------------------------------------------------
*/
extern U16 dio_write(U32            device,
					 T_DIO_CTRL *   control_info,
					 T_dio_buffer * buffer);

/*
+------------------------------------------------------------------------------
| Function    : dio_get_tx_buffer
+------------------------------------------------------------------------------
| Description : This function returns a send buffer provided via dio_write().
|
| Parameters  : device - device number
|               buffer - return sent data buffer
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The device is not yet configured
|
+------------------------------------------------------------------------------
*/
extern U16 dio_get_tx_buffer(U32             device,
							 T_dio_buffer ** buffer);

/*
+------------------------------------------------------------------------------
| Function    : dio_clear
+------------------------------------------------------------------------------
| Description : This function is used to clear the hardware send buffer.
|
| Parameters  : device - device number
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The device is not yet configured
|               DRV_INPROCESS      - The driver is busy clearing the buffer
|
+------------------------------------------------------------------------------
*/
extern U16 dio_clear(U32 device);

/*
+------------------------------------------------------------------------------
| Function    : dio_flush
+------------------------------------------------------------------------------
| Description : With this function the driver is requested to inform the
|               user, when data of the hardware send buffer have been written
|               successfully.
|
| Parameters  : device - device number
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The device is not yet configured
|               DRV_INPROCESS      - The driver is busy flushing the buffer
|
+------------------------------------------------------------------------------
*/
extern U16 dio_flush(U32 device);

/*
+------------------------------------------------------------------------------
| Function    : dio_get_capabilities
+------------------------------------------------------------------------------
| Description : This function is used to retrieve the capabilities of a device.
|
| Parameters  : device       - device number
|               capabilities - Return: Pointer to the device capabilities
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist
|               DRV_INTERNAL_ERROR - Internal driver error
|
+------------------------------------------------------------------------------
*/
extern U16 dio_get_capabilities(U32                device,
								T_DIO_CAP  ** capabilities);

/*
+------------------------------------------------------------------------------
| Function    : dio_set_config
+------------------------------------------------------------------------------
| Description : This function is used to configure a device.
|
| Parameters  : device - device number
|               dcb    - pointer to a device control block
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - parameter invalid or out of range
|               DRV_INTERNAL_ERROR - Internal driver error
|
+------------------------------------------------------------------------------
*/
extern U16 dio_set_config(U32         device,
						  T_DIO_DCB * dcb);

/*
+------------------------------------------------------------------------------
| Function    : dio_get_config
+------------------------------------------------------------------------------
| Description : This function reads the device configuration.
|
| Parameters  : device - device number
|               dcb    - pointer to a device control block
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device does not exist or
|                                    wrong Device Control Block provided.
|               DRV_INTERNAL_ERROR - Internal driver error
|               DRV_NOTCONFIGURED  - The device is not yet configured
|
+------------------------------------------------------------------------------
*/
extern U16 dio_get_config(U32         device,
						  T_DIO_DCB * dcb);

/*
+------------------------------------------------------------------------------
| Function    : dio_close_device
+------------------------------------------------------------------------------
| Description : This function is used to close a device.
|
| Parameters  : device - device number
|
| Return      : DRV_OK             - Function successful
|               DRV_INVALID_PARAMS - The specified device can not be closed yet
|               DRV_INTERNAL_ERROR - Internal driver error
|
+------------------------------------------------------------------------------
*/
extern U16 dio_close_device(U32 device); 
 
#endif /* __DIO_H__ */
