/*
+-----------------------------------------------------------------------------
|  Project :  COMLIB
|  Modul   :  cl_md5.c
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
|  Purpose :  Definitions of common library functions: MD5 algorithm
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

#ifndef CL_MD5_C
#define CL_MD5_C

#include "typedefs.h"
#include <string.h>
#include "vsi.h"        /* to get a lot of macros */
#include "cl_md5.h"
#include "stdio.h"


/*==== FUNCTIONS ==================================================*/


/*
 * Constants for MD5 routine.
 */

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/*
 * F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/*
 * ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/*
 * FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
 * Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
/* 
 * 1. Append the length in bits
 * 2. Process message
 * 3. Store state in digest 
 */
#define PROCESS_MSG() {\
  memcpy(&context.buffer[56], bits, 8);\
  cl_md5_transform(context.state, context.buffer);\
  cl_md5_enc (digest, context.state, 16);\
  }

#ifdef _SIMULATION_
/*
+------------------------------------------------------------------------------
| Function    : MDPrint
+------------------------------------------------------------------------------
| Description : Prints a message digest in hexadecimal.
|
| Parameters  : UBYTE digest[16]
+------------------------------------------------------------------------------
*/
GLOBAL void MDPrint (UBYTE *digest, UINT len)
{
  UINT i;
  for (i = 0; i < len; i+=8)
    TRACE_EVENT_P8 ("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, ",
                     digest[i], digest[i+1], digest[i+2], digest[i+3],
                     digest[i+4], digest[i+5], digest[i+6], digest[i+7]);
}
#endif

/*
+------------------------------------------------------------------------------
| Function    : cl_md5_enc
+------------------------------------------------------------------------------
| Description : Encodes input (UINT) into output (UBYTE). Assumes len
|               is a multiple of 4.
|
| Parameters  : UBYTE *output
|               UINT *input
|               UINT len
+------------------------------------------------------------------------------
*/
void cl_md5_enc (UBYTE *output, UINT *input, UINT len)
{
  UINT i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
    output[j] = (UBYTE)(input[i] & 0xff);
    output[j+1] = (UBYTE)((input[i] >> 8) & 0xff);
    output[j+2] = (UBYTE)((input[i] >> 16) & 0xff);
    output[j+3] = (UBYTE)((input[i] >> 24) & 0xff);
  }
}

/*
+------------------------------------------------------------------------------
| Function    : cl_md5_dec
+------------------------------------------------------------------------------
| Description : Decodes input (UBYTE) into output (UINT). Assumes len
|               is a multiple of 4.
|
| Parameters  : UINT *output
|               UBYTE *input
|               UINT len
+------------------------------------------------------------------------------
*/
void cl_md5_dec (UINT *output, UBYTE *input, UINT len)
{
  UINT i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
    output[i] = ((UINT)input[j]) | (((UINT)input[j+1]) << 8) |
                (((UINT)input[j+2]) << 16) | (((UINT)input[j+3]) << 24);
}


/*
+------------------------------------------------------------------------------
| Function    : cl_md5_transform
+------------------------------------------------------------------------------
| Description : MD5 basic transformation. Transforms state based on block. 
|               For more information see RFC 1321 "MD5 Message-Digest Algorithm".
|               This routine is derived from the RSA Data Security, Inc. MD5
|               Message-Digest Algorithm. 
|
| Parameters  : UINT state[4]
|               UBYTE block[64]
|
+------------------------------------------------------------------------------
*/
void cl_md5_transform (UINT state[4], UBYTE block[64])
{
  UINT a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  cl_md5_dec (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */

  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.*/
  memset((UBYTE *)x, 0, sizeof (x));
}

/*
+------------------------------------------------------------------------------
| Function    : cl_md5
+------------------------------------------------------------------------------
| Description : Digests a string and prints the result. For more information see
|               RFC 1321 "MD5 Message-Digest Algorithm".
|
| Parameters  : UBYTE *input  - input challenge string
|               UINT   len    - length of input string
|               UBYTE *digest - output digest message
+------------------------------------------------------------------------------
*/
GLOBAL void cl_md5 (UBYTE *input, UINT len, UBYTE *digest)
{
  MD5_CTX context;
  UBYTE bits[8];
  UINT  ind;
  int chk_len;
  TRACE_EVENT("cl_md5");

  if(len==0)
    len = strlen ((char *)input);

#ifdef _SIMULATION_
  TRACE_EVENT("Input Message");
  MDPrint (input, len);
#endif

  /*
   * MD5 initialization. Begins an MD5 operation, writing a new context.
   */
  context.count[0] = context.count[1] = 0;
  /* 
   * Load magic initialization constants according to RFC 1321
   */
  context.state[0] = 0x67452301; /* word A */
  context.state[1] = 0xefcdab89; /* word B */
  context.state[2] = 0x98badcfe; /* word C */
  context.state[3] = 0x10325476; /* word D */
  /* 
   * Update number of bits (64-bit representation of message length) 
   */
  if ((context.count[0] += ((UINT)len << 3)) < ((UINT)len << 3))
    context.count[1]++;
  context.count[1] += ((UINT)len >> 29);
  /* 
   * Save number of bits 
   */
  cl_md5_enc (bits, context.count, 8);

  /*
   * Step 1. Append Padding Bits
   * The message is extended so that its length is congruent to 56 bytes  
   * (448 bits), modulo 64 (512). Extending is always performed, even if the
   * length is already congruent to 56 bytes, modulo 64.
   *
   * Extending is performed as follows, a single "1" bit is append to the 
   * message, and then "0" bits are appended so that the legth in bits of the
   * message becomes congruent to 56, modulo 64. In all, at least one byte and 
   * at most 64 bytes are appended.
   *
   * Step 2. Append Length.
   * A 64-bit representation of the message length before the padding is 
   * appended to the result of previous step. 
   * 
   * Step 3. Process Message in 16-word blocks
   */
  if(len < 56)
  {
    /* 
     * copy message 
     */
    memcpy(&context.buffer[0], &input[0], len);
    /* 
     * Append length to 56 bytes 
     */
    context.buffer[len] = 0x80; /* append a single "1" bit */
    memset(&context.buffer[len+1], 0, 55-len); /* append "0" bits */
    /* 
     * Append length in bits and process message
     */
    PROCESS_MSG();
  }
  else if(len >= 56 && len < 64)
  {
    /* 
     * copy message 
     */
    memcpy(&context.buffer[0], &input[0], len);
    /* 
     * Append length to 64 bytes 
     */
    context.buffer[len] = 0x80;
    memset(&context.buffer[len+1], 0, 63-len);
    /*
     * Process message
     */
    cl_md5_transform (context.state, context.buffer);
    /* 
     * Append length to 56 bytes 
     */
    memset(&context.buffer[0], 0, 56);
    /* 
     * Append the length in bits and process message
     */
    PROCESS_MSG();
  }
  else if(len >= 64)
  {
    /*
     * Copy first 64 bytes
     */
    memcpy(&context.buffer[0], &input[0], 64);
    /*
     * Process message
     */
    cl_md5_transform (context.state, context.buffer);
    if(len >= 120)
    {
      /*
       * Process message in 64-byte blocks 
       */
      for (ind = 64; ind + 63 < len; ind += 64)
        cl_md5_transform (context.state, &input[ind]);
    }
    else
      ind = 64;
    /*
     * Copy the rest
     */
    memcpy(&context.buffer[0], &input[ind], len-ind);
    /* 
     * Append length to 56 bytes 
     */
     /*lint -e661 -e662 -e669 possible access or creation of bount ptr or data overrun*/
    context.buffer[len-ind] = 0x80;
    chk_len=55-(len-ind);
    if(chk_len >=0)
    memset(&context.buffer[len-ind+1], 0, chk_len);
    /*lint +e661 +e662 +e669 possible access or creation of bount ptr or data overun*/
     /* 
     * Append the length in bits and process message
     */
    PROCESS_MSG();
  }

#ifdef _SIMULATION_
  TRACE_EVENT("Digest Message");
  MDPrint (digest, 16);
#endif
}


#ifdef _SIMULATION_
/*
+------------------------------------------------------------------------------
| Function    : MDTestSuite
+------------------------------------------------------------------------------
| Description : Digests a reference suite of strings and prints the results.
|
| Parameters  : void
+------------------------------------------------------------------------------
*/
GLOBAL void cl_md5TestSuite ()
{
  UBYTE digest[16];
  UBYTE test_digest0[16]   = {0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04,
                              0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e};
  UBYTE test_digest1[16]   = {0x0c, 0xc1, 0x75, 0xb9, 0xc0, 0xf1, 0xb6, 0xa8,
                              0x31, 0xc3, 0x99, 0xe2, 0x69, 0x77, 0x26, 0x61};
  UBYTE test_digest3[16]   = {0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0,
                              0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72};
  UBYTE test_digest14[16]  = {0xf9, 0x6b, 0x69, 0x7d, 0x7c, 0xb7, 0x93, 0x8d,
                              0x52, 0x5a, 0x2f, 0x31, 0xaa, 0xf1, 0x61, 0xd0};
  UBYTE test_digest26[16]  = {0xc3, 0xfc, 0xd3, 0xd7, 0x61, 0x92, 0xe4, 0x00,
                              0x7d, 0xfb, 0x49, 0x6c, 0xca, 0x67, 0xe1, 0x3b};
  UBYTE test_digest62[16]  = {0xd1, 0x74, 0xab, 0x98, 0xd2, 0x77, 0xd9, 0xf5,
                              0xa5, 0x61, 0x1c, 0x2c, 0x9f, 0x41, 0x9d, 0x9f};
  UBYTE test_digest80[16]  = {0x57, 0xed, 0xf4, 0xa2, 0x2b, 0xe3, 0xc9, 0x55,
                              0xac, 0x49, 0xda, 0x2e, 0x21, 0x07, 0xb6, 0x7a};
  UBYTE test_digest160[16] = {0x26, 0x8c, 0x79, 0x19, 0x18, 0x9d, 0x85, 0xe2,
                              0x76, 0xd7, 0x4b, 0x8c, 0x60, 0xb2, 0xf8, 0x4f};

  TRACE_EVENT("MD5 test suite:");

  /* 
   * Test 1. Lenght := 0 
   */
  cl_md5("", 0, digest);
  if(memcmp(digest, test_digest0, 16))
  {
    TRACE_EVENT("CHAP MD5: Test 1 failed!!");
  }
  else
  {
    TRACE_EVENT("CHAP MD5: Test 1 passed.");
  }

  /* 
   * Test 2. Lenght := 1 
   */
  cl_md5("a", 0, digest);
  if(memcmp(digest, test_digest1, 16))
  {
    TRACE_EVENT("CHAP MD5: Test 2 failed!!");
  }
  else
  {
    TRACE_EVENT("CHAP MD5: Test 2 passed.");
  }

  /* 
   * Test 3. Lenght := 3 
   */
  cl_md5("abc", 0, digest);
  if(memcmp(digest, test_digest3, 16))
  {
    TRACE_EVENT("CHAP MD5: Test 3 failed!!");
  }
  else
  {
    TRACE_EVENT("CHAP MD5: Test 3 passed.");
  }

  /* 
   * Test 4. Lenght := 14 
   */
  cl_md5("message digest", 0, digest);
  if(memcmp(digest, test_digest14, 16))
  {
    TRACE_EVENT("CHAP MD5: Test 4 failed!!");
  }
  else
  {
    TRACE_EVENT("CHAP MD5: Test 4 passed.");
  }

  /* 
   * Test 5. Lenght := 26 
   */
  cl_md5("abcdefghijklmnopqrstuvwxyz", 0, digest);
  if(memcmp(digest, test_digest26, 16))
  {
    TRACE_EVENT("CHAP MD5: Test5 failed!!");
  }
  else
  {
    TRACE_EVENT("CHAP MD5: Test 5 passed.");
  }

  /* 
   * Test 6. Lenght := 62 
   */
  cl_md5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", 0, digest);/*62*/
  if(memcmp(digest, test_digest62, 16))
  {
    TRACE_EVENT("CHAP MD5: Test 6 failed!!");
  }
  else
  {
    TRACE_EVENT("CHAP MD5: Test 6 passed.");
  }

  /* 
   * Test 7. Lenght := 80 
   */
  cl_md5("12345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, digest);/*80*/
  if(memcmp(digest, test_digest80, 16))
  {
    TRACE_EVENT("CHAP MD5: Test 7 failed!!");
  }
  else
  {
    TRACE_EVENT("CHAP MD5: Test 7 passed.");
  }

  /* 
   * Test 8. Lenght := 160 
   */
  cl_md5("12345678901234567890123456789012345678901234567890123456789012345678901234567890\
12345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, digest);
  if(memcmp(digest, test_digest160, 16))
  {
    TRACE_EVENT("CHAP MD5: Test 8 failed!!");
  }
  else
  {
    TRACE_EVENT("CHAP MD5: Test 8 passed.");
  }

}
#endif/*_SIMULATION_*/


#endif /* CL_MD5_C */
