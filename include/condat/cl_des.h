/*
+-----------------------------------------------------------------------------
|  Project :  COMLIB
|  Modul   :  cl_des.h
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
|  Purpose :  Definitions of global types used by common library functions
|             and the prototypes of those functions.
+-----------------------------------------------------------------------------
*/
/*
 *  Version 1.0
 */

/**********************************************************************************/

#ifndef CL_DES_H
#define CL_DES_H

/**********************************************************************************/

#define CL_DES_BUFFER_SIZE   8
#define CL_DES_KEY_SIZE      8
#define CL_DES_ENCRYPTION    1
#define CL_DES_DECRYPTION    2

/*
+------------------------------------------------------------------------------
| Function    : cl_des
+------------------------------------------------------------------------------
| Description : The function performs DES encrypting or decrypting
|
| Parameters  : inMsgPtr   : pointer to input message M. The length of message
|                            has to be min. 8 bytes e.g. M = 0123456789abcdef
|               desKeyPtr  : pointer to DES key. Length has to be 8 bytes
|                outMsgPtr : output encrypted/decrypted message. The length is 8 b.
|                     code : CL_DES_ENCRYPTION, CL_DES_DECRYPTION
+------------------------------------------------------------------------------
*/
EXTERN void cl_des(UBYTE *inMsgPtr, UBYTE *desKeyPtr, UBYTE *outMsgPtr, UBYTE code);


/**********************************************************************************/

#endif /* CL_DES_H */
