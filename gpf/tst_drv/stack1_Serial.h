/*=============================================================================
 *    Copyright 1996-2001 Texas Instruments Inc. All rights reserved.
 */
#ifndef STACK1_SERIAL_HEADER
#define STACK1_SERIAL_HEADER

//#include "puf_PrimHdr.h"

/*=============================================================================
 * Description:
 */
//typedef struct{
//    Puf_PrimHdr primHdr;
//}Stack1_Serial_RxInd;

typedef void (*SERIAL_CALLBACK_FUNC)(void);

/*===========================================================================
 * Description:
 *   Install the Uart interrupt handlers and initialize the Uart.
 */
int
stack1_Serial_PowerUp( SERIAL_CALLBACK_FUNC callback );

/*===========================================================================
 * Description:
 *   Copy the data from the Rx buffer into the passed in buffer. If there
 *   aren't dataLen bytes in the Rx buffer all the bytes currently in the rx
 *   buffer are returned.
 */
unsigned long
stack1_Serial_receiveData( unsigned char  *dataPtr,
                           unsigned long dataLen );

/*===========================================================================
 * Description:
 *   Copy the data to be transmitted into the Tx Buffer and enable the Uart Tx
 *   interrupt. If the Tx buffer is full, discard the data and return a buffer
 *   full indication.
 */
int
stack1_Serial_sendData( unsigned char  *dataPtr,
                        unsigned long dataLen );

#endif
