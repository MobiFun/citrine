/*
+-----------------------------------------------------------------------------
|  Project :  COMLIB
|  Modul   :  cl_des.c
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
|  Purpose :  Definitions of common library functions: Implementation of
              DES algorithm
+-----------------------------------------------------------------------------
*/
/*
 *  Version 1.0
 */

/**********************************************************************************/

/*
NOTE:
*/

/**********************************************************************************/

#ifndef CL_DES_C
#define CL_DES_C

#include <string.h>
#include "typedefs.h"
#include "cl_des.h"


/* 64+64+17*56+16*48+64+17*32+17*32 = 3000 bytes */
static UBYTE binmsg[64] , binkey[64], cd[17][56] , deskey[16][48] , ip[64];
static UBYTE l[17][32] , r[17][32];
/* 64+64+32+32+64+64+17*3+2 = 373 bytes */
static UBYTE rnew[64] , xorres[64] , scale[32] , perm[32] , rl[64] , encpt[64];

/* 64+16+48+64+48+32+64+8*66 = 864 bytes */
static
const UBYTE shtamt[16]  = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
static
const UBYTE iporder[64] = {58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,62,54,
                           46,38,30,22,14,6,64,56,48,40,32,24,16,8,57,49,41,33,
                           25,17,9,1,59,51,43,35,27,19,11,3,61,53,45,37,29,21,
                           13,5,63,55,47,39,31,23,15,7};
static
const UBYTE pc1[64]   = {57,49,41,33,25,17,9,1,58,50,42,34,26,18,10,2,59,51,43,
                         35,27,19,11,3,60,52,44,36,63,55,47,39,31,23,15,7,62,54,
                         46,38,30,22,14,6,61,53,45,37,29,21,13,5,28,20,12,4};
static
const UBYTE pc2[48]   = {14,17,11,24,1,5,3,28,15,6,21,10,23,19,12,4,26,8,
                         16,7,27,20,13,2,41,52,31,37,47,55,30,40,51,45,33,
                         48,44,49,39,56,34,53,46,42,50,36,29,32};
static
const UBYTE e[48]     = {32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,12,13,14,15,
                         16,17,16,17,18,19,20,21,20,21,22,23,24,25,24,25,26,27,
                         28,29,28, 29,30,31,32,1};
static
const UBYTE sp[32]    = {16,7,20,21,29,12,28,17,1,15,23,26,5,18,31,10,
                         2,8,24,14,32,27,3,9,19,13,30,6,22,11,4,25};
static
const UBYTE ipinv[64] = {40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,38,6,46,
                         14,54,22,62,30,37,5,45,13,53,21,61,29,36,4,44,12,52,
                         20,60,28,35,3,43,11,51,19,59,27,34,2,42,10,50,18,58,
                         26,33,1,41,9,49,17,57,25};
static
const UBYTE s[8][66]  = {{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,0,15,7,4,14,2,13,
                          1,10,6,12,11,9,5,3,8,4,1,14,8,13,6,2,11,15,12,9,7,3,10,
                          5,0,15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13},
                         {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,3,13,4,7,15,2,8,
                          14,12,0,1,10,6,9,11,5,0,14,7,11,10,4,13,1,5,8,12,6,9,3,
                          2,15,13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9},
                         {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,13,7,0,9,3,4,6,
                          10,2,8,5,14,12,11,15,1,13,6,4,9,8,15,3,0,11,1,2,12,5,
                          10,14,7,1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12 },
                         {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,13,8,11,5,6,15,
                          0,3,4,7,2,12,1,10,14,9,10,6,9,0,12,11,7,13,15,1,3,14,
                          5,2,8,4,3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14 },
                         {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,14,11,2,12,4,7,
                          13,1,5,0,15,10,3,9,8,6,4,2,1,11,10,13,7,8,15,9,12,5,6,
                          3,0,14,11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3 },
                         {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,10,15,4,2,7,12,
                          9,5,6,1,13,14,0,11,3,8,9,14,15,5,2,8,12,3,7,0,4,10,1,
                          13,11,6,4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13 },
                         {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,13,0,11,7,4,9,1,
                          10,14,3,5,12,2,15,8,6,1,4,11,13,12,3,7,14,10,15,6,8,0,
                          5,9,2,6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12 },
                         {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,1,15,13,8,10,3,7,
                          4,12,5,6,11,0,14,9,2,7,11,4,1,9,12,14,2,0,6,10,13,15,3,
                          5,8,2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11 }};


/*==== FUNCTIONS ==================================================*/

/*
+------------------------------------------------------------------------------
| Function    : des_hex2bin4
+------------------------------------------------------------------------------
| Description : The function converts a 4 bit hex value to 4 binary values
|
| Parameters  : hex : value in hex
|                 m : pointer to buffer of 4 elements to store binary values
+------------------------------------------------------------------------------
*/
LOCAL void des_hex2bin4(UBYTE hex, UBYTE *m)
{
  m[0] = (hex & 0x08) >> 3;
  m[1] = (hex & 0x04) >> 2;
  m[2] = (hex & 0x02) >> 1;
  m[3] =  hex & 0x01;
}

/*
+------------------------------------------------------------------------------
| Function    : des_hex2bin8
+------------------------------------------------------------------------------
| Description : The function converts a 8 bit hex value to 8 binary values
|
| Parameters  : hex : value in hex
|                 m : pointer to buffer of 8 elements to store binary values
+------------------------------------------------------------------------------
*/
LOCAL void des_hex2bin8(UBYTE hex, UBYTE *m)
{
  m[0] = (hex & 0x80) >> 7;
  m[1] = (hex & 0x40) >> 6;
  m[2] = (hex & 0x20) >> 5;
  m[3] = (hex & 0x10) >> 4;
  m[4] = (hex & 0x08) >> 3;
  m[5] = (hex & 0x04) >> 2;
  m[6] = (hex & 0x02) >> 1;
  m[7] =  hex & 0x01;
}

/*
+------------------------------------------------------------------------------
| Function    : des_bin2hex
+------------------------------------------------------------------------------
| Description : The function converts 8 bin values to an 8 bit hex value
|
| Parameters  : m[8] : input bin values
| Return      : converted hex value
+------------------------------------------------------------------------------
*/

LOCAL UBYTE des_bin2hex(UBYTE *m)
{
  UBYTE hex;
  return hex = (m[0]<<7) | (m[1]<<6) | (m[2]<<5) | (m[3]<<4) |
               (m[4]<<3) | (m[5]<<2) | (m[6]<<1) | m[7];
}



/*
+------------------------------------------------------------------------------
| Function    : des_shift
+------------------------------------------------------------------------------
| Description : The function performs shifting
|
| Parameters  : dst : pointer to destination buffer
|               src : pointer to source buffer
|               sht : shift value
+------------------------------------------------------------------------------
*/

LOCAL void des_shift(UBYTE *dst, UBYTE *src, UBYTE sht)
{
  UBYTE c1 , c2 , d1 , d2;
  int i;

  c1 = src[0];
  c2 = src[1];
  d1 = src[28];
  d2 = src[29];

  for ( i = 0 ; i < 28 - sht ; i++) {
    dst[i] =  src[i + sht];            /* copying c[i] */
    dst[28 + i] =  src[28 + i + sht];  /* copying d[i] */
  }

  if (sht == 1){
    dst[27] = c1;
    dst[55] = d1;
  } else {
     dst[26] = c1;
     dst[27] = c2;
     dst[54] = d1;
     dst[55] = d2;
  }
}

/*
+------------------------------------------------------------------------------
| Function    : des_indx
+------------------------------------------------------------------------------
| Description : The function generates index for S table
|
| Parameters  : m[6] :
| Return      : index value
+------------------------------------------------------------------------------
*/
LOCAL UBYTE  des_indx(UBYTE *m)
{
   return( (((m[0]<<1) + m[5])<<4) + ((m[1]<<3) + (m[2]<<2) + (m[3]<<1) + m[4]));
}

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
EXTERN void cl_des(UBYTE *inMsgPtr, UBYTE *desKeyPtr, UBYTE *outMsgPtr, UBYTE code)
{

  int y , z , g;
  UBYTE temp, more;

  /*
   * convert message from hex to bin format
   */
  for(y = 0; y < 8; y++){
    des_hex2bin8(inMsgPtr[y], &binmsg[8 * y]);
  }

  /*
   * Convert DES key value from hex to bin format
   */
  for( y = 0; y < 8; y++){
    des_hex2bin8(desKeyPtr[y], &binkey[8 * y]);
  }

  /*
   *  Step 1: Create 16 subkeys, each of which is 48-bits long.
   *
   * The 64-bit key is permuted according to the table pc1,
   * to get the 56 bit subkey K+. The subkey K+ consists of left
   * and right halves C0 and D0, where each half has 28 bits.
   */
  for(y = 0 ; y < 56 ; y++)
    cd[0][y] = binkey[pc1[y] - 1];
  /*
   * Create futher 15 subkeys C1-C16 and D1-D16 by left shifts of
   * each previous key, i.e. C2 and D2 are obtained from C1 and D1 and so on.
   */
  for(y = 0 ; y < 16 ; y++)
    des_shift(cd[y + 1] , cd[y] , shtamt[y]);

  /*
   * Form the keys K1-K16 by applying the pc2 permutation
   * table to each of the concatenated pairs CnDn.
   */
  for(y = 0; y < 16; y++){
    for(z = 0 ; z < 48 ; z++){
      deskey[y][z] = cd[y + 1][pc2[z] - 1];
    }
  }

  /*
   * Step 2: Encode each 64-bit block of data
   *
   * Perform initial permutation IP of th e64 bits the message data M.
   * This rearranges the bits according to the iporder table.
   */
  for(y = 0; y < 64; y++)
    ip[y] = binmsg[iporder[y]  - 1];

  /*
   * Divide the permuted block IP into left half L0
   * and a right half R0 each of 32 bits.
   */
  for(y = 0; y < 32; y++){
    l[0][y] = ip[y];
    r[0][y] = ip[y + 32];
  }

  /*
   * Proceed through 16 iterations, operation on two blocks:
   * a data block of 32 bits and a key Kn of 48 bits to produce a block of 32
   * bits. This results in a final block L16R16. In each iteration, we take
   * the right 32 bits of the previous result and make them the left 32 bits
   * of the current step. For the right 32 bits in the current step, we XOR
   * the left 32 bits of the previous step.
   */
  for (y = 0; y < 16; y++){
    if (code == CL_DES_ENCRYPTION)/* encryption */
      g = y;
    else          /* decryption */
      g = 15 - y;

    /*
     * Copie the right bits Rn of the current step
     * to the left bits Ln+1 of the next step
     */
    for(z = 0; z < 32; z++)
      l[y + 1][z] = r[y][z];

    /*
     * Expand the block Rn from 32 to 48 bits by using the selection table E.
     * Then XOR the result with the key Kn+1.
     */
    for(z = 0; z < 48; z++){
      rnew[z] = r[y][e[z] - 1];
      xorres[z] = (rnew[z] ^ deskey[g][z]);
    }

    /*
     * We now have 48 bits, or eight groups of six bits. We use them as
     * addresses in tables calle "S boxes". Each group of six bits will
     * give us an address in a different S box.
     */
    for(z = 0; z < 8; z++){
      temp = s[z][des_indx(&xorres[z * 6])];
      des_hex2bin4(temp, &scale[z * 4]);
    }

    /*
     * Perform a permutation P of the S box output.
     */
    for(z = 0; z < 32; z++)
      perm[z] = scale[sp[z] - 1];

    /*
     * XOR the result with the left half of current step
     * and copie it to the right half of the next step
     */
    for(z = 0; z < 32; z++)
      r[y+1][z] = (l[y][z] ^ perm[z]);
  }

  /*
   * Reserve the order of the final block L16R16 to R16L16
   */
  for( z = 0; z < 32; z++){
    rl[z] = r[16][z];
    rl[z + 32] = l[16][z];
  }

  /*
   * Apply the final inverse permutation IP
   */
  for( z = 0; z < 64; z++){
    encpt[z] = rl[ipinv[z] - 1];
  }

  /*
   * Convert from bin to hex format
   */
  for(z = 0; z < 8; z++){
     outMsgPtr[z] = des_bin2hex(&encpt[8 * z]);
  }
}
#endif /* CL_DES_C */
