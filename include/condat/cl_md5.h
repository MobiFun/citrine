/*
+-----------------------------------------------------------------------------
|  Project :  COMLIB
|  Modul   :  cl_md5
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

#ifndef CL_MD5_H
#define CL_MD5_H

/**********************************************************************************/

/* MD5 context. */
typedef struct {
  UINT state[4];                                   /* state (ABCD) */
  UINT count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

/**********************************************************************************/

EXTERN void cl_md5 (UBYTE *input, UINT len, UBYTE *digest);
#ifdef _SIMULATION_
EXTERN void cl_md5TestSuite (void);
#endif

#endif /* CL_RIBU_H */
