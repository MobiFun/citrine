/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : bitfun.c
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
|  Purpose :  Definition of constants, types and global variables and bit 
|             stream manipulation functions for CCD
+----------------------------------------------------------------------------- 
*/ 

#define __BITFUN_C__



#ifdef DEBUG_CCD
#include <stdio.h>
#endif

#include "typedefs.h"
#include "header.h"

#include "string.h"
#include "ccd_globs.h"
#include "bitfun.h"
#include "ccd.h"


EXTERN int 	abs(int);

#ifndef RUN_FLASH
/*
 * For each bitlength between 0 and 32 this table contains the
 * valid masks for right-justificated values
 */

const ULONG ccd_bitfun_mask[] =
{
  0x00000000,
  0x00000001, 0x00000003, 0x00000007, 0x0000000f,
  0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
  0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
  0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
  0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
  0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
  0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
  0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};

/*
 * Table of one-bit values (2^X)
 */

const UBYTE ccd_bitfun_shift[] =
{
  128, 64, 32, 16, 8, 4, 2, 1
};
#else
extern const ULONG ccd_bitfun_mask[];
extern const UBYTE ccd_bitfun_shift[];
#endif

/*
 * maschine-dependent implementation
 * define M_INTEL or M_MOTOROLA dependend on the target system
 */

#ifdef M_INTEL
#define MSB_POS 1
#define LSB_POS 0
#define MSW_POS 2
#define LSW_POS 0
#else
#ifdef M_MOTOROLA
#define MSB_POS 0
#define LSB_POS 1
#define MSW_POS 0
#define LSW_POS 2
#endif
#endif


typedef union
{				                                  /* Conversion structure      */
  UBYTE c[2];                          /* 2 bytes <-> USHORT        */
  USHORT s;
}
t_conv16;

typedef union
{                           				       /* Conversion structure      */
  UBYTE c[4];                          /* 4 Byte <-> ULONG          */
  ULONG l;
}
t_conv32;

#ifndef RUN_FLASH
#ifdef DEBUG_CCD
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : ccd_BITIMAGE        |
+--------------------------------------------------------------------+

  PURPOSE : convert a ULONG value to a ascii-bitstring with the
            length len.
*/


char *ccd_BITIMAGE (ULONG val, ULONG len, T_CCD_Globs *globs)
{

  int i;
  for (i=31; i >= 0; i--)
  {
    globs->buf [i] = (char)(val & 1)+'0';
    val >>= 1;
  }
  globs->buf [32] = '\0';

  return globs->buf+(32-len);
}

#endif
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : fmemcpy             |
+--------------------------------------------------------------------+

  PURPOSE : copies len bytes from source to dest
            for FAR addresses.
*/

void fmemcpy (UBYTE * d, UBYTE *s, USHORT len)
{
  while (len--)
   *d++ = *s++;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_writeBitChunk    |
+--------------------------------------------------------------------+

  PURPOSE : The global pointer globs->pstruct+globs->pstructOffs references
            a buffer struct containing the len, offset and content
            of a bitbuffer. This funtions copies the bits to the
            bitstream buffer at the actual position.
*/

void bf_writeBitChunk (ULONG len, T_CCD_Globs *globs)
{
  ULONG offs;
  ULONG bytesToCopy=0;
  signed char theShift;
  U8 *MsgStruct = (U8*)(globs->pstruct + globs->pstructOffs);
  U8  ByteOffs  = globs->byteoffs;
  U8 *MsgBuf    = (U8*)(globs->bitbuf + globs->bytepos);

  /*
   * Read the length of the buffer (the l_buf component) and 
   * compare it with the parameter given.
   */
  len = MINIMUM (len, *((USHORT *)MsgStruct));
  if (!len)
    return;
  bytesToCopy = (ULONG) ((len + ByteOffs + 7)>> 3);
  offs = (ULONG)(*(USHORT *)(MsgStruct + 2));
  theShift = (signed char) (offs - ByteOffs);

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "reading 2+2+%ld bytes from struct %08X, writing %d bits at byte %d.%d",
              bytesToCopy, MsgStruct + 4, len, globs->bytepos, globs->byteoffs);
#endif

  if ((abs(theShift) & 7) EQ 0)
  {
    MsgStruct += (4+(offs>>3));
    /*
     * the bits in the Buffer are in the right position
     * -> quick memcopy.
     */
    /*
     * write the first byte, clean the leading bits
     */
    *MsgBuf |= (*MsgStruct & (UBYTE) ccd_bitfun_mask[8 - (offs&7)]);
    /*
     * copy the remaining bytes
     */
    if (bytesToCopy)
    {
      memcpy ((UBYTE *) &MsgBuf[1],
               MsgStruct + 1,
               (size_t)bytesToCopy-1);
    }
    if ((ByteOffs+len)&7)
    {
      MsgBuf[bytesToCopy-1] &= ~ccd_bitfun_mask[8-((ByteOffs+len)&7)];
    }
  }
  else
  {
    t_conv16 conv;
    /*
     * shift every single byte
     */
    MsgStruct += (4+(offs>>3));
    if (bytesToCopy > 1)
    {
      --bytesToCopy;
    }
    if (theShift > 0)
    {
      /*
       * shift all bytes leftwise
       */
      /*
       * the first byte must be masked
       */
      theShift &= 7;
      conv.c[MSB_POS] = *MsgStruct++ & (UBYTE) ccd_bitfun_mask[8-(offs&7)];
      conv.c[LSB_POS] = *MsgStruct;
      conv.s <<= theShift;
      *MsgBuf++ |= conv.c[MSB_POS];
      *MsgBuf      = conv.c[LSB_POS];
      --bytesToCopy;

      while (bytesToCopy)
      {
        conv.c[MSB_POS] = *MsgStruct++;
        conv.c[LSB_POS] = *MsgStruct;
        conv.s <<= theShift;
        *MsgBuf++ |= conv.c[MSB_POS];
        *MsgBuf    = conv.c[LSB_POS];
        --bytesToCopy;
      }

      conv.c[MSB_POS] = *MsgStruct++;
      conv.c[LSB_POS] = *MsgStruct;
      conv.s <<= theShift;
      *MsgBuf |= conv.c[MSB_POS];

      if ((ByteOffs+len)&7)
      {
        *MsgBuf &= ~ccd_bitfun_mask[8-((ByteOffs+len)&7)];
      }
    }
    else
    {
      /*
       * shift all bytes rightwise
       */
      /*
       * the first byte must be masked, we dont want to store the
       * leading garbage before the valid bits
       */
      theShift = (-theShift & 7);
      conv.c[MSB_POS] = *MsgStruct++ & (UBYTE) ccd_bitfun_mask[8-(offs&7)];
      conv.c[LSB_POS] = *MsgStruct;
      conv.s >>= theShift;
      *MsgBuf++ |= conv.c[MSB_POS];
      *MsgBuf      = conv.c[LSB_POS];
      --bytesToCopy;

      while (bytesToCopy)
      {
        conv.c[MSB_POS] = *MsgStruct++;
        conv.c[LSB_POS] = *MsgStruct;
        conv.s >>= theShift;
        *MsgBuf++ |= conv.c[MSB_POS];
        *MsgBuf    = conv.c[LSB_POS];
        --bytesToCopy;
      }

      conv.c[MSB_POS] = *MsgStruct++;
      conv.c[LSB_POS] = *MsgStruct;
      conv.s >>= theShift;
      *MsgBuf |= conv.c[MSB_POS];

      if ((ByteOffs+len)&7)
      {
        *MsgBuf &= ~ccd_bitfun_mask[8-((ByteOffs+len)&7)];
      }
    }
  }
  bf_incBitpos (len, globs);
  globs->pstructOffs = MsgStruct - globs->pstruct - 1;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_writeBits        |
+--------------------------------------------------------------------+

  PURPOSE : The pointer globs->pstruct + globs->pstructOffs refers to 
            a var in the message C-structure. This function writes the 
            numeric content of the var into the bit stream referenced 
            by globs->bitbuf. According to the required lenght
            (len) the type of the var is interpreted as BYTE/SHORT
            or LONG.
*/

void bf_writeBits (ULONG len, T_CCD_Globs *globs)
{
  /*
   * Bit field is given by its lenght, offset bit and buffer.
   */
  if (len > 32)
  {
    bf_writeBitChunk (len, globs);
    return;
  }
  else
  {
    UBYTE *MsgStruct = (UBYTE*)(globs->pstruct + globs->pstructOffs);
    U8     ByteOffs   = globs->byteoffs;
    UBYTE *MsgBuf     = (UBYTE*)(globs->bitbuf + globs->bytepos);
    U32 wBits;

    wBits = len + ByteOffs;

    if (len > 24) 
    {
      t_conv32 conv32;
      UBYTE FirstByte;

      conv32.l = *((ULONG *) MsgStruct);
      conv32.l &= (ULONG) ccd_bitfun_mask[len];
      conv32.l <<= (32-len);

      FirstByte = conv32.c[MSW_POS + MSB_POS];    
      FirstByte >>= ByteOffs;
      FirstByte &= (UBYTE) ccd_bitfun_mask[8-ByteOffs];
      *MsgBuf++ |= FirstByte;
      
      conv32.l <<= (8-ByteOffs);
      
      *MsgBuf++  = conv32.c[MSW_POS + MSB_POS] ;
      *MsgBuf++  = conv32.c[MSW_POS + LSB_POS] ;
      *MsgBuf++  = conv32.c[LSW_POS + MSB_POS] ;

      if (wBits > 32)
      *MsgBuf    = conv32.c[LSW_POS + LSB_POS] ;
    }
  /* 
   * Bit field given in a C variable is shorter than a USHORT. 
   */
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "write %d bits at byte %d.%d", len, globs->bytepos, ByteOffs);
#endif
    if (len > 8)
    {
      t_conv32 conv;

      conv.l = (len > 16) ? *(ULONG *) MsgStruct
        : (ULONG) * (USHORT *) MsgStruct;
      conv.l &= ccd_bitfun_mask[len];
#ifdef DEBUG_CCD
      TRACE_CCD (globs, " (.%s)", ccd_BITIMAGE (conv.l, len, globs));
#endif
      conv.l <<= (32 - wBits);
      *MsgBuf |= conv.c[MSW_POS + MSB_POS];
      MsgBuf[1] |= conv.c[MSW_POS + LSB_POS];
      if (wBits > 16)
      {
        MsgBuf[2] |= conv.c[LSW_POS + MSB_POS];
        if (wBits > 24)
        {
      	  MsgBuf[3] |= conv.c[LSW_POS + LSB_POS];
        }
      }
    }
    else
    {
      t_conv16 conv;

      conv.s = (USHORT) (*MsgStruct);
      conv.s &= (USHORT) ccd_bitfun_mask[len];
#ifdef DEBUG_CCD
      TRACE_CCD (globs, " (.%s)", ccd_BITIMAGE ((ULONG) conv.s, len, globs));
#endif
      conv.s <<= (16 - wBits);
      *MsgBuf |= conv.c[MSB_POS];
      
      if (wBits > 8)
      {
        MsgBuf[1] |= conv.c[LSB_POS];
      }
    }
    bf_incBitpos (len, globs);
  }
}
#endif /* !RUN_FLASH */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_writeBitStr_PER  |
+--------------------------------------------------------------------+

  PURPOSE : The pointer globs->pstruct + globs->pstructOffs refers to 
            a var in the message C-structure. This function writes the 
            numeric content of the var into the bit stream referenced 
            by globs->bitbuf.         
*/           
             
void bf_writeBitStr_PER (USHORT len, T_CCD_Globs *globs)
{
  USHORT bytesToCopy;
  t_conv16 Last2Bytes;
  U8 *MsgStruct = (U8*)(globs->pstruct + globs->pstructOffs);
  U8  ByteOffs  = globs->byteoffs;
  U8 *MsgBuf;
  MsgBuf = (U8*)(globs->bitbuf + globs->bytepos + ((ByteOffs + len) >> 3));
  /*
   * Store the data for a later compensation of buffer overwritting.
   * bytepos is always greater than 1, since the first byte with bytepos=0
   * is dedicated to message identifier.
   */
  Last2Bytes.c[MSB_POS] = MsgBuf[0];
  Last2Bytes.c[LSB_POS] = MsgBuf[1];

  MsgBuf -= (ByteOffs + len) >> 3;
  bytesToCopy = (len+7) >> 3;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "copying %d byte(s) from struct 0x%08X, writing %d bits at byte %d.%d",
                bytesToCopy, MsgStruct, len, globs->bytepos, globs->byteoffs);
  {
    int  i=0;
    int  j=0;
    char s[64], c[4];
    int  leftBytes = bytesToCopy;

    TRACE_CCD (globs, " Bit string to be encoded:");

    s[0] = '\0';
    for (i = 0; leftBytes > 0 ; i++)
    {
      for (j = 0; (j < 16 && (leftBytes-- > 0)) ; j++)
      {
        sprintf(c, " %02x", *(MsgStruct+(16*i)+j));
        strcat (s, c);
      }
      TRACE_CCD (globs, "%s", s);
      s[0] = '\0';
    }
  }
#endif

  /*
   * The bits will be appended to the bit buffer at byte boundary.
   */
  if (ByteOffs EQ 0)
  { 
    /*
     * CCD assumes that the bits to be copied are left adjusted 
     * int their place in the C-structure.
     */
    memcpy ((UBYTE *) MsgBuf,
               MsgStruct,
               (size_t)bytesToCopy);
    MsgBuf += bytesToCopy;
  }
  else
  {
    t_conv16 conv;

    /*
     * Write byte for byte while compensating non-zero value of globs->byteoffs.
     */
    while (bytesToCopy--)
    {
      conv.c[MSB_POS] = *MsgStruct++;
      conv.c[LSB_POS] = *MsgStruct;
      conv.s >>= ByteOffs;
      conv.s &= (USHORT) ccd_bitfun_mask[16-ByteOffs];
      *MsgBuf++ |= conv.c[MSB_POS];
      *MsgBuf     |= conv.c[LSB_POS];
    }
  }

  bf_incBitpos (len, globs);

  /*
   * Undo overwriting of bits which do not belong to the bit string.
   */
  MsgBuf = (U8*)(globs->bitbuf + globs->bytepos);
  MsgBuf[0] &= ~ccd_bitfun_mask[8-globs->byteoffs];
  Last2Bytes.c[MSB_POS] &= (UBYTE) ccd_bitfun_mask[8-globs->byteoffs];
  MsgBuf[0] |= Last2Bytes.c[MSB_POS];
  MsgBuf[1]  = Last2Bytes.c[LSB_POS];

}
#endif /* !RUN_INT_RAM */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_writePadBits     |
+--------------------------------------------------------------------+

  PURPOSE : The global pointer globs->bitbuf points to the bit stream 
            buffer. This function adds 0-7 zero bits to the bit stream 
            from the point globs->byteoffs refer to.
*/

void bf_writePadBits (T_CCD_Globs *globs)
{
  if (globs->byteoffs NEQ 0)
  {
    globs->bitbuf[globs->bytepos] &=
      (UBYTE) ~ccd_bitfun_mask[8-globs->byteoffs];
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "writing %d pad bit(s) at byte %d.%d",
             8-globs->byteoffs, globs->bytepos, globs->byteoffs);
#endif

    bf_incBitpos (8-globs->byteoffs, globs);
  }
}
#endif /* !RUN_FLASH */
      
#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_readBit          |
+--------------------------------------------------------------------+

  PURPOSE : The global pointer globs->bitbuf points to the bit stream 
            buffer. This function reads the bit to which the global 
            positioning pointers are pointing to.
*/

BOOL bf_readBit (T_CCD_Globs *globs)
{
  UBYTE ret;

  ret = globs->bitbuf[globs->bytepos] & ccd_bitfun_shift[globs->byteoffs];
  bf_incBitpos (1, globs);
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "reading 1 bit (%d) at byte %d.%d", (ret>0)?1:0, globs->bytepos, globs->byteoffs);
#endif

  return (ret > 0);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_writeBit         |
+--------------------------------------------------------------------+

  PURPOSE : The global pointer globs->bitbuf points to the bit stream 
            buffer. This function writes a given value into the bit 
            to which the global positioning pointers are pointing to.
*/

void bf_writeBit (BOOL Bit, T_CCD_Globs *globs)
{
  globs->bitbuf[globs->bytepos] = Bit ? (globs->bitbuf[globs->bytepos] |
                                        ccd_bitfun_shift[globs->byteoffs])
    : (globs->bitbuf[globs->bytepos] & ~ccd_bitfun_shift[globs->byteoffs]);

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "writing 1 bit (%d) at byte %d.%d", Bit, globs->bytepos, globs->byteoffs);
#endif

  bf_incBitpos (1, globs);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_rShift8Bit       |
+--------------------------------------------------------------------+

  PURPOSE : shifts a bitfield in the bitstream by 8 bits rightwise.
            The function is used only for octet aligned types.
            Hence offset is not involved in calculation.
*/
void bf_rShift8Bit
            (
              USHORT srcBitPos,
              USHORT bitLen,
              T_CCD_Globs *globs
            )
{
  register UBYTE bytesToCopy;
  USHORT bytepos;

  /*
   * destination > source -> start with last byte
   */
  bytepos     = srcBitPos >> 3;
  bytesToCopy = bitLen >> 3;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "shifting %d bits rightwise for 8 bits", bitLen);
#endif

  while (bytesToCopy)
  {
    globs->bitbuf[bytepos+bytesToCopy] = globs->bitbuf[bytepos+bytesToCopy-1];
    bytesToCopy--;
  }
  globs->bitbuf[bytepos] = 0;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_readBitChunk     |
+--------------------------------------------------------------------+

  PURPOSE : reads (len) Bits from the Bitsream (globs->bitbuf) and
            stores them to the T_MSGBUF struct, containig the len
            the offset and a bitbuffer.
*/

void bf_readBitChunk (ULONG len, T_CCD_Globs *globs)
{
  ULONG   bytesToCopy;
  U8 *MsgStruct = (U8*)(globs->pstruct + globs->pstructOffs);
  U8  ByteOffs  = globs->byteoffs;
  U8 *MsgBuf    = (U8*)(globs->bitbuf + globs->bytepos);

  *(U16*) (MsgStruct) = (U16) len;
  MsgStruct += 2;
  *(U16*) (MsgStruct) = (U16)ByteOffs;
  MsgStruct += 2;
  bytesToCopy = (ULONG) ((len >> 3)+(len&7?1:0)+(ByteOffs?1:0));

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "reading %ld bits from byte %d.%d, writing 2+2+%ld bytes to struct %08X",
             len, globs->bytepos, globs->byteoffs, bytesToCopy, MsgStruct);
#endif
  memcpy ((UBYTE *) MsgStruct,
           MsgBuf,
           (size_t)bytesToCopy);

#if 0
#ifdef DEBUG_CCD
  {
    int i;
	for (i=0; i<bytesToCopy; i++)
    {
	  TRACE_CCD (globs, "buf[%d] = 0x%02x", i, MsgStruct[i]);
	}
  }
#endif
#endif
  /*
   * cutoff the leading and trailing bits wich are obsolete
   */
  *MsgStruct &= ccd_bitfun_mask [8-ByteOffs];
  if ((len+ByteOffs)&7)
  {
    MsgStruct += (bytesToCopy-1);
    *MsgStruct &= ~ccd_bitfun_mask [8-((len+ByteOffs)&7)];
  }

#if 0
#ifdef DEBUG_CCD
  {
    int i;
    for (i=0; i<bytesToCopy; i++)
    {
      TRACE_CCD (globs, "buf[%d] = 0x%02x", i, MsgStruct[i]);
    }
  }
#endif
#endif
  bf_incBitpos (len, globs);
}
#endif /* !RUN_FLASH */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_readBitStr_PER   |
+--------------------------------------------------------------------+

  PURPOSE : The pointer globs->pstruct + globs->pstructOffs refers to 
            a var in the message C-structure. This function reads len
            bits from the bit stream (globs->bitbuf) and stores them 
            left adjusted to the bytes in the message C-structure.            
*/

void bf_readBitStr_PER (USHORT len, T_CCD_Globs *globs)
{
  USHORT bytesToCopy;
  t_conv16 Last2Bytes;
  U8 *MsgStruct = (U8*)(globs->pstruct + globs->pstructOffs);
  U8 *MsgStructEnd = (U8*)(globs->pstruct + globs->pstructOffs + (len >> 3));
  U8  ByteOffs  = globs->byteoffs;
  U8 *MsgBuf    = (U8*)(globs->bitbuf + globs->bytepos);

  bytesToCopy = (len+7) >> 3;

  /*
   * Store the data for a later compensation of buffer overwritting.
   */  
  Last2Bytes.c[MSB_POS] = MsgStructEnd[0];
  Last2Bytes.c[LSB_POS] = MsgStructEnd[1];

  /*
   * The bits will be read from the bit buffer at byte boundary.
   */
  if (ByteOffs EQ 0)
  {

#ifdef DEBUG_CCD
    TRACE_CCD (globs, "reading %d bits from byte %d.%d, copying %d bytes to struct at 0x%08X",
             len, globs->bytepos, globs->byteoffs, bytesToCopy, MsgStruct);
#endif

    /*
     * CCD assumes that the caller function needs the bit string to be
     * left adjusted in the C-structure.
     */
    memcpy ((UBYTE *) MsgStruct,//(globs->pstruct + globs->pstructOffs),
           MsgBuf,
           (size_t)bytesToCopy);
  }
  else
  {
    t_conv16 conv;

    /*
     * Read byte for byte while compensating the non-zero globs->byteoffs.
     */
    while (bytesToCopy--)
    {
      conv.c[MSB_POS] = *MsgBuf++;
      conv.c[LSB_POS] = *MsgBuf;
      conv.s <<= ByteOffs;
      conv.s &= (USHORT) ~ccd_bitfun_mask[8-(len&7)];
      *MsgStruct++ = conv.c[MSB_POS];
      *MsgStruct   = conv.c[LSB_POS];
    }
  }

  /*
   * Undo overwriting in C-Structure. This is specially necessary
   * for later reading of valid flags for optional elments.
   */
  MsgStructEnd[0] &= (UBYTE) ~ccd_bitfun_mask[8-(len&7)];
  Last2Bytes.c[MSB_POS]  &= (UBYTE) ccd_bitfun_mask[8-(len&7)];
  MsgStructEnd[0] |= Last2Bytes.c[MSB_POS];
  MsgStructEnd[1]  = Last2Bytes.c[LSB_POS];

#ifdef DEBUG_CCD
  {
    int  i=0;
    int  j=0;
    char s[64], c[4];
    int  leftBytes =  (len+7) >> 3;;

    MsgStruct = (U8*)(globs->pstruct + globs->pstructOffs);
    TRACE_CCD (globs, " Decoded bit string:");

    s[0] = '\0';
    for (i = 0; leftBytes > 0 ; i++)
    {
      for (j = 0; (j < 16 && (leftBytes-- > 0)) ; j++)
      {
        sprintf(c, " %02x", *(MsgStruct+(16*i)+j));
        strcat (s, c);
      }
      TRACE_CCD (globs, "%s", s);
      s[0] = '\0';
    }
  }
#endif

  globs->pstructOffs += ((len+7) >> 3);
  bf_incBitpos (len, globs);
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_readBits         |
+--------------------------------------------------------------------+

  PURPOSE : reads (len) Bits from the Bitsream (globs->bitbuf) and
            stores them typed (dependent on the length) to vars in
            the C-struct referenced by the global var
            globs->pstruct+globs->pstructOff.
*/

void bf_readBits (ULONG len, T_CCD_Globs *globs)
{
  /*
   * Bit field is given by its lenght, offset bit and buffer.
   */
  if (len > 32)
  {
    bf_readBitChunk (len, globs);
    return;
  }
  else
  {
    t_conv16 conv16; 
    t_conv32 conv32;
    U8 *MsgStruct = (U8*)(globs->pstruct + globs->pstructOffs);
    U8  ByteOffs  = globs->byteoffs;
    U8 *MsgBuf    = (U8*)(globs->bitbuf + globs->bytepos);

#ifdef DEBUG_CCD
    TRACE_CCD (globs, "reading %d bits form byte %d.%d,", len, globs->bytepos, globs->byteoffs);
#endif

    /* 
     * Bit field is given in a C variable shorter than a USHORT. 
     */
    if (len <= 8)
    {
      if (globs->lastbytepos16 != globs->bytepos)
      {
        conv16.c[MSB_POS] = *MsgBuf++;
        conv16.c[LSB_POS] = *MsgBuf;
        globs->lastbytepos16 = globs->bytepos;
        globs->last16Bit = conv16.s;
      }
      else
        conv16.s = globs->last16Bit;
      conv16.s >>= (16 - (ByteOffs + len));
      conv16.s &= (USHORT) ccd_bitfun_mask[len];
      *MsgStruct = (UBYTE) conv16.s;
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "writing 1 bytes (%0X) to struct %08X", *MsgStruct, MsgStruct);
#endif
    }
    else if (len + ByteOffs <= 32)
    {
      if (globs->lastbytepos32 != globs->bytepos)
      {
        conv32.c[MSW_POS + MSB_POS] = *MsgBuf++;
        conv32.c[MSW_POS + LSB_POS] = *MsgBuf++;
        conv32.c[LSW_POS + MSB_POS] = *MsgBuf++;
        conv32.c[LSW_POS + LSB_POS] = *MsgBuf;
        globs->lastbytepos32 = globs->bytepos;
        globs->last32Bit = conv32.l;
      }
      else
        conv32.l = globs->last32Bit;
      conv32.l >>= (32 - (ByteOffs + len));
      conv32.l &= ccd_bitfun_mask[len];
      if (len > 16)
      {
        *((ULONG *) MsgStruct) = conv32.l;
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "writing 4 bytes (%08X) to struct %08X",
                 conv32.l,
                 MsgStruct);
#endif
      }
      else
      {
        *((USHORT *) MsgStruct) = (USHORT) conv32.l;
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "writing 2 bytes (%04X) to struct %08X",
                 (USHORT) conv32.l,
                 MsgStruct);
#endif
      }
    }
    else 
    {
      UBYTE FirstByte;
      
      FirstByte = *MsgBuf++;
      FirstByte <<= ByteOffs;
      FirstByte &= (UBYTE) ~ccd_bitfun_mask[ByteOffs];
  
      if (globs->lastbytepos32 != globs->bytepos)
      {
        conv32.c[MSW_POS + MSB_POS] = *MsgBuf++;
        conv32.c[MSW_POS + LSB_POS] = *MsgBuf++;
        conv32.c[LSW_POS + MSB_POS] = *MsgBuf++;
        conv32.c[LSW_POS + LSB_POS] = *MsgBuf;
        globs->lastbytepos32 = globs->bytepos;
        globs->last32Bit = conv32.l;
      }
      else
      {
        conv32.l = globs->last32Bit;
      }
      if (!ByteOffs)
      {
        conv32.l >>= 8;
      }
      else
      {
        conv32.l >>= (8-ByteOffs);
      }
      conv32.c[MSW_POS + MSB_POS] &= (UBYTE) ccd_bitfun_mask[ByteOffs];
      conv32.c[MSW_POS + MSB_POS] |= FirstByte;    
      conv32.l >>= (32-len);
      conv32.l &= (ULONG) ccd_bitfun_mask[len];
  
      *((ULONG *) MsgStruct) = conv32.l;
#ifdef DEBUG_CCD
      TRACE_CCD ( globs, "writing 4 bytes (%08X) to struct %08X",
                  conv32.l,
                  MsgStruct);
#endif
    }
    bf_incBitpos (len, globs); return;
  }
}
#endif /* !RUN_FLASH */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_writeVal         |
+--------------------------------------------------------------------+

  PURPOSE : writes value into the next (bSize) bits of the air message
            buffer(globs->bitbuf). This function does not use the data
            in the C-structure. This is done by the caller function
            while calculating value.
*/
void bf_writeVal (ULONG value, ULONG bSize, T_CCD_Globs *globs)
{
  ULONG BitSum;
  U8 *MsgBuf    = (U8*)(globs->bitbuf + globs->bytepos);
  U8  ByteOffs  = globs->byteoffs;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "writeVal %d bits at byte %d.%d", bSize, globs->bytepos, ByteOffs);
#endif
  /*
   * value will be written into a temporary buffer. This buffer will
   * then be prepared for an ORing with the bit buffer of air message.
   * BitSum is helpful to find out the right size for temporary buffer.
   */ 
  BitSum = bSize + (ULONG)ByteOffs;

  /* 
   * Write in 1 to 8 bits (bSize is 1-8). 
   */
  if (BitSum <= 8)
  {
    UBYTE TmpBuf;

#ifdef DEBUG_CCD
    TRACE_CCD (globs, " (.%s)", ccd_BITIMAGE ((ULONG)(value & ccd_bitfun_mask[bSize]), bSize, globs));
#endif
    TmpBuf = (UBYTE) value;
    TmpBuf &= (UBYTE) ccd_bitfun_mask[bSize];    
    TmpBuf <<= (8 - BitSum);
    *MsgBuf |= TmpBuf;
  }
  else
  {
    /* 
     * Write in 9 to 16 bits (bSize is 9-16). 
     */
    if (BitSum <= 16)
    {
      t_conv16 conv;

      conv.s = (USHORT) value;
      conv.s &= (USHORT) ccd_bitfun_mask[bSize];
#ifdef DEBUG_CCD
      TRACE_CCD (globs, " (.%s)", ccd_BITIMAGE ((ULONG) conv.s, bSize, globs));
#endif
      conv.s <<= (16 - BitSum);
      MsgBuf[0] |= conv.c[MSB_POS];
      MsgBuf[1] |= conv.c[LSB_POS];
    }

    /* 
     * Write in 17 to 25 bits (bSize is 17-25). 
     */
    else if (BitSum <= 32)
    {
      t_conv32 conv;

      conv.l = value;
      conv.l &= ccd_bitfun_mask[bSize];
#ifdef DEBUG_CCD
      TRACE_CCD (globs, " (.%s)", ccd_BITIMAGE ((ULONG)conv.l, bSize, globs));
#endif
      conv.l <<= (32 - BitSum);
      MsgBuf[0] |= conv.c[MSW_POS + MSB_POS];
      MsgBuf[1] |= conv.c[MSW_POS + LSB_POS];
      MsgBuf[2] |= conv.c[LSW_POS + MSB_POS];
      if (BitSum > 24)
      {
        MsgBuf[3] |= conv.c[LSW_POS + LSB_POS];
      }
    }
    /* 
     * Write in 25 to 32 bits (bSize is 25-32). 
     */
    else if ( BitSum < 40)
    {
      UBYTE FirstByte;
      t_conv32 conv;

      conv.l = value;
      conv.l <<= (32 - bSize);
      FirstByte = conv.c[MSW_POS + MSB_POS];
      FirstByte >>= ByteOffs;
      FirstByte &= (UBYTE) ccd_bitfun_mask[8-ByteOffs];
      MsgBuf[0] |= FirstByte;

      conv.l <<= (8 - ByteOffs);
      MsgBuf[1] |= conv.c[MSW_POS + MSB_POS];
      MsgBuf[2] |= conv.c[MSW_POS + LSB_POS];
      MsgBuf[3] |= conv.c[LSW_POS + MSB_POS];
      MsgBuf[4] |= conv.c[LSW_POS + LSB_POS];
#ifdef DEBUG_CCD
      conv.l &= (ULONG) ~ccd_bitfun_mask[24-ByteOffs];
      TRACE_CCD (globs, " (.%s)", ccd_BITIMAGE ((ULONG) FirstByte, (ULONG)(8-ByteOffs), globs));
      TRACE_CCD (globs, " (.%s)", ccd_BITIMAGE (conv.l, (ULONG)(24-ByteOffs), globs));
#endif
    }
    /* 
     * This case is currently not supported.
     * Integer values are written to and read from up to 32 bits.
     */ 
    else
    {
      return;
    }
  }

  bf_incBitpos (bSize, globs);
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_getBits          |
+--------------------------------------------------------------------+

  PURPOSE : reads len Bits from the Bitstream (globs->bitbuf) and
            stores them in a variable. The caller function can now
            interpret or process the content of the returned variable. 
*/

ULONG  bf_getBits (ULONG len, T_CCD_Globs *globs)
{
  U8  ByteOffs  = globs->byteoffs;
  U8 *MsgBuf    = (U8*)(globs->bitbuf + globs->bytepos);

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "reading %ld bits form byte %d.%d,", len, globs->bytepos, globs->byteoffs);
#endif

  /*
   * Read up to 8 bits from the air message buffer. 
   */
  if (len <= 8)
  {
    t_conv16 conv16;

    if (globs->lastbytepos16 != globs->bytepos)
    {
      conv16.c[MSB_POS] = *MsgBuf++;
      conv16.c[LSB_POS] = *MsgBuf;
      globs->lastbytepos16 = globs->bytepos;
      globs->last16Bit = conv16.s;
    }
    else
    {
      conv16.s = globs->last16Bit;
    }
    conv16.s >>= (16 - (ByteOffs + len));
    conv16.s &= (USHORT) ccd_bitfun_mask[len];
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "read value: %d", conv16.s);
#endif
    bf_incBitpos (len, globs);
    return (ULONG) conv16.s;  
  }

  /*
   * Read between 8 and 24 bits from the air message buffer. 
   */
  else if (len <= 24)
  {
    t_conv32 conv32;
    
    if (globs->lastbytepos32 != globs->bytepos)
    {
      conv32.c[MSW_POS + MSB_POS] = *MsgBuf++;
      conv32.c[MSW_POS + LSB_POS] = *MsgBuf++;
      conv32.c[LSW_POS + MSB_POS] = *MsgBuf++;
      conv32.c[LSW_POS + LSB_POS] = *MsgBuf;
      globs->lastbytepos32 = globs->bytepos;
      globs->last32Bit = conv32.l;
    }
    else
    {
      conv32.l = globs->last32Bit;
    }
    conv32.l >>= (32 - (ByteOffs + len));
    conv32.l &= ccd_bitfun_mask[len];
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "read value: %ld", conv32.l);
#endif
    bf_incBitpos (len, globs);
    return conv32.l; 
  }

  /*
   * Read between 24 and 32 bits from the air message buffer. 
   */
  else if ( len <= 32)
  {
    UBYTE FirstByte;
    t_conv32 conv;
    
    FirstByte = *MsgBuf++;
    FirstByte <<= ByteOffs;
    FirstByte &= (UBYTE) ~ccd_bitfun_mask[ByteOffs];

    if (globs->lastbytepos32 != globs->bytepos)
    {
      conv.c[MSW_POS + MSB_POS] = *MsgBuf++;
      conv.c[MSW_POS + LSB_POS] = *MsgBuf++;
      conv.c[LSW_POS + MSB_POS] = *MsgBuf++;
      conv.c[LSW_POS + LSB_POS] = *MsgBuf;
      globs->lastbytepos32 = globs->bytepos;
      globs->last32Bit = conv.l;
    }
    else
    {
      conv.l = globs->last32Bit;
    }
    if (!ByteOffs)
    {
      conv.l >>= 8;
    }
    else
    {
      conv.l >>= (8-ByteOffs);
    }
    conv.c[MSW_POS + MSB_POS] &= (UBYTE) ccd_bitfun_mask[ByteOffs];
    conv.c[MSW_POS + MSB_POS] |= FirstByte;    
    conv.l >>= (32-len);
    conv.l &= (ULONG) ccd_bitfun_mask[len];

#ifdef DEBUG_CCD
    TRACE_CCD (globs, "read value: %ld", conv.l);
#endif
    bf_incBitpos (len, globs);
    return conv.l;
  }

  /* 
   * This case is currently not supported.
   * Integer values are written to and read from up to 32 bits.
   */ 
  else
  {
    return 0;
  }
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_decodeLongNumber |
+--------------------------------------------------------------------+

  PURPOSE : reads (len) Bits from the Bitsream (globs->bitbuf) and
            return this number as a 32 Bit number. The Position
            of the readpointer of the bitstream is incremented by
            the len.
*/

ULONG bf_decodeLongNumber (UBYTE len, T_CCD_Globs *globs)
{
  U32 number;
  t_conv16 conv16; 
  t_conv32 conv32;
  U8  ByteOffs  = globs->byteoffs;
  U8 *MsgBuf    = (U8*)(globs->bitbuf + globs->bytepos);

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "reading %d bits form byte %d.%d,", len, globs->bytepos, globs->byteoffs);
#endif

  if (len <= 8)
  {
    if (globs->lastbytepos16 != globs->bytepos)
    {
      conv16.c[MSB_POS] = *MsgBuf++;
      conv16.c[LSB_POS] = *MsgBuf;
      globs->lastbytepos16 = globs->bytepos;
      globs->last16Bit = conv16.s;
    }
    else
      conv16.s = globs->last16Bit;
    conv16.s >>= (16 - (ByteOffs + len));
    conv16.s &= (USHORT) ccd_bitfun_mask[len];
    number = (ULONG) conv16.s;
  }
  else
  {
    if (globs->lastbytepos32 != globs->bytepos)
    {
      conv32.c[MSW_POS + MSB_POS] = *MsgBuf++;
      conv32.c[MSW_POS + LSB_POS] = *MsgBuf++;
      conv32.c[LSW_POS + MSB_POS] = *MsgBuf++;
      conv32.c[LSW_POS + LSB_POS] = *MsgBuf;
      globs->lastbytepos32 = globs->bytepos;
      globs->last32Bit = conv32.l;
    }
    else
      conv32.l = globs->last32Bit;
    conv32.l >>= (32 - (ByteOffs + len));
    conv32.l &= ccd_bitfun_mask[len];
    number = conv32.l;
  }

  bf_incBitpos (len, globs);

#ifdef DEBUG_CCD
  TRACE_CCD (globs, " (%08X)", number);
#endif

  return number;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_decodeShortNumber|
+--------------------------------------------------------------------+

  PURPOSE : reads (len) Bits from the Bitsream (globs->bitbuf) and
            returns the resulting value as an ULONG.
*/

ULONG bf_decodeShortNumber (const ULONG len, T_CCD_Globs *globs)
{
  UBYTE *p;
  t_conv32 conv32;

  p = globs->bitbuf + globs->bytepos;

  conv32.c[MSW_POS + MSB_POS] = *p++;
  conv32.c[MSW_POS + LSB_POS] = *p++;
  conv32.c[LSW_POS + MSB_POS] = *p++;
  conv32.c[LSW_POS + LSB_POS] = *p;
  conv32.l >>= (32 - (globs->byteoffs + len));
  conv32.l &= ccd_bitfun_mask[len];
  bf_incBitpos (len, globs);
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "reading %d bits as LONG (%d) at byte %d.%d", len, (ULONG) conv32.l, globs->bytepos, globs->byteoffs);
#endif
  return (ULONG) conv32.l;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_decodeByteNumber |
+--------------------------------------------------------------------+

  PURPOSE : reads (len) Bits from the Bitsream (globs->bitbuf) and
            returns the resulting value as an UBYTE.
*/

UBYTE bf_decodeByteNumber (const ULONG len, T_CCD_Globs *globs)
{
  UBYTE *p;
  t_conv16 conv16;

  p = globs->bitbuf + globs->bytepos;

  conv16.c[MSB_POS] = *p++;
  conv16.c[LSB_POS] = *p;
  conv16.s >>= (16 - (globs->byteoffs + len));
  conv16.s &= (USHORT) ccd_bitfun_mask[len];
  bf_incBitpos (len, globs);
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "reading %d bits as BYTE (%d) at byte %d.%d", len, (UBYTE) conv16.s, globs->bytepos, globs->byteoffs);
#endif
  return (UBYTE) conv16.s;
}
#endif /* !RUN_FLASH */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_codeShortNumber  |
+--------------------------------------------------------------------+

  PURPOSE : Converts the value (val) into a MSB/LSB-Bitstring and
            writes it to the aktual position into the bitstream
            globs->bitbuf. The maximum value of (len) is 16.
            If the value of (val) is greater then (2^len)-1 it
            will be truncated.
*/

void bf_codeShortNumber (UBYTE len, USHORT val, T_CCD_Globs *globs)
{
  UBYTE *p;
  t_conv32 conv32;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "codeShortNumber: writing %d bits (.%s) at byte %d.%d",
             len, ccd_BITIMAGE (val, (ULONG) len, globs), globs->bytepos, globs->byteoffs);
#endif
  p = globs->bitbuf + globs->bytepos;
  conv32.l = (ULONG) val;
  conv32.l <<= (32 - len - globs->byteoffs);
  *p++ |= conv32.c[MSW_POS + MSB_POS];
  *p = conv32.c[MSW_POS + LSB_POS];
  if ((globs->byteoffs + len) > 16)
    *++p = conv32.c[LSW_POS + MSB_POS];
  bf_incBitpos (len, globs);
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_recodeShortNumber|
+--------------------------------------------------------------------+

  PURPOSE:  Converts the value (val) into a MSB/LSB-Bitstring and
            writes it at the position (pos) into the bitstream
            globs->bitbuf. The rest of the bitstream and the actual
            position will not changed. The maximum value of (len)
            is 16.
            If the value of (val) is greater then (2^len)-1
            it will be truncated.
*/

void bf_recodeShortNumber (USHORT pos, UBYTE len, USHORT val, T_CCD_Globs *globs)
{
  UBYTE *p;
  USHORT oldbitpos;
  t_conv32 conv32;
  USHORT wBits;

  oldbitpos = globs->bitpos;
  bf_setBitpos (pos, globs);
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "bf_recodeShortNumber:rewriting %d bits (.%s) at byte %d.%d",
             len, ccd_BITIMAGE ((ULONG) val, len, globs), globs->bytepos, globs->byteoffs);
#endif
  wBits = len + globs->byteoffs;
  p = globs->bitbuf + globs->bytepos;
  conv32.l = (ULONG) val;
  conv32.l <<= (32 - wBits);

  /*
   * Since the bits to write are cleared (memclr) in the bitstream,
   * it is ok to perform an OR operation on them.
   */
  *p++ |= conv32.c[MSW_POS + MSB_POS];
  if (wBits > 8)
  {
    *p++ |= conv32.c[MSW_POS + LSB_POS];
    if (wBits > 16)
    {
      *p++ |= conv32.c[LSW_POS + MSB_POS];
      if (wBits > 24)
        *p |= conv32.c[LSW_POS + LSB_POS];
    }
  }

  bf_setBitpos (oldbitpos, globs);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_recodeByteNumber |
+--------------------------------------------------------------------+

  PURPOSE:  Converts the value (val) into a MSB/LSB-Bitstring and
            writes it at the position (pos) into the bitstream
            globs->bitbuf. The rest of the bitstream and the actual
            position will not changed. The maximum value of (len)
            is 8.
            If the value of (val) is greater then (2^len)-1
            it will be truncated.
*/

void bf_recodeByteNumber (USHORT pos, UBYTE len, UBYTE val, T_CCD_Globs *globs)
{
  UBYTE *p;
  USHORT oldbitpos;
  t_conv16 conv16;

  oldbitpos = globs->bitpos;
  bf_setBitpos (pos, globs);
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "bf_recodeByteNumber:rewriting %d bits (.%s) at byte %d.%d",
             len, ccd_BITIMAGE ((ULONG) val, len, globs), globs->bytepos, globs->byteoffs);
#endif
  p = globs->bitbuf + globs->bytepos;
  conv16.s = (USHORT) val;
  conv16.s <<= (16 - len - globs->byteoffs);
  /*
   * if the bits to write are cleared (memclr) in the bitstream
   * we can perform an OR operation on it
   */
  *p++ |= conv16.c[MSB_POS];
  if ((len + globs->byteoffs) > 8)
    *p |= conv16.c[LSB_POS];

  bf_setBitpos (oldbitpos, globs);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_recodeBit        |
+--------------------------------------------------------------------+

  PURPOSE:  Writes the value of (Bit) at the position (pos)
            into the bitstream globs->bitbuf. The rest of the bitstream
            and the actual position will not changed.
*/

void bf_recodeBit (USHORT pos, UBYTE Bit, T_CCD_Globs *globs)
{
  U16 oldbitpos = globs->bitpos;;

  bf_setBitpos (pos, globs);
  globs->bitbuf[globs->bytepos] = Bit ? (globs->bitbuf[globs->bytepos] |
                                        ccd_bitfun_shift[globs->byteoffs])
                          : (globs->bitbuf[globs->bytepos] &
                             ~ccd_bitfun_shift[globs->byteoffs]);
  bf_setBitpos (oldbitpos, globs);
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "recode 1 bit (.%d) at bitpos %d", Bit, pos);
#endif
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_codeByteNumber   |
+--------------------------------------------------------------------+

  PURPOSE:  Converts the value (val) into a Bitstring with the
            length (len) and writes it at the actual position
            into the bitstream globs->bitbuf. The maximum value of
            (len) is 8.
            If the value is greater then (2^len)-1 it will be
            truncated.

*/

void bf_codeByteNumber (UBYTE len, UBYTE val, T_CCD_Globs *globs)
{
  UBYTE *p;
  t_conv16 conv16;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "codeByteNumber:writing %d bits (.%s) at byte %d.%d",
             len, ccd_BITIMAGE (val, (ULONG) len, globs), globs->bytepos, globs->byteoffs);
#endif
  p = globs->bitbuf + globs->bytepos;

  conv16.s = (USHORT) val;
  conv16.s <<= (16 - len - globs->byteoffs);
  *p++ |= conv16.c[MSB_POS];
  if ((globs->byteoffs + len) > 8)
    *p |= conv16.c[LSB_POS];
  bf_incBitpos (len, globs);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_codeLongNumber   |
+--------------------------------------------------------------------+

  PURPOSE : This funtion writes the numeric content of
            the var to the aktual position into the bitstream
            referenced by globs->bitbuf.
*/

void bf_codeLongNumber (UBYTE len, ULONG val, T_CCD_Globs *globs)
{
  UBYTE wBits = len + globs->byteoffs;
  U8 *MsgBuf  = (U8*)(globs->bitbuf + globs->bytepos);

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "codeLongNumber: writing %d bits (.%s) at byte %d.%d",
             len, ccd_BITIMAGE (val, (ULONG) len, globs), globs->bytepos, globs->byteoffs);
#endif

  if (len > 8)
  {
    t_conv32 conv;

    conv.l = val;
    conv.l &= ccd_bitfun_mask[len];
    conv.l <<= (32 - wBits);
    MsgBuf[0] |= conv.c[MSW_POS + MSB_POS];
    MsgBuf[1] = conv.c[MSW_POS + LSB_POS];
    if (wBits > 16)
    {
      MsgBuf[2] = conv.c[LSW_POS + MSB_POS];
      if (wBits > 24)
      {
      	MsgBuf[3] = conv.c[LSW_POS + LSB_POS];
      }
    }
  }
  else
  {
    t_conv16 conv;

    conv.s = (USHORT) val;
    conv.s &= (USHORT) ccd_bitfun_mask[len];
    conv.s <<= (16 - wBits);
    MsgBuf[0] |= conv.c[MSB_POS];
    if (wBits > 8)
    {
      MsgBuf[1] = conv.c[LSB_POS];
    }
  }

  bf_incBitpos (len, globs);
}
#endif /* !RUN_FLASH */

#if 0 /* not used - maybe for the future */
#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : BITFUN              |
| STATE   : code                       ROUTINE : bf_swapBits         |
+--------------------------------------------------------------------+

  PURPOSE : reads (len) Bits from the Bitsream (1-8) and swap
            this part with the next (len) bits in the bitstream.
            The read/write pointer of the bitstream left unchanged.
            This function is used for swapping the nibbles in some
            ugly coded GSM messages.
*/

void bf_swapBits (ULONG len, T_CCD_Globs *globs)
{
  UBYTE         s1, s2;
  USHORT        s21;
  USHORT        startpos = globs->bitpos;
  UBYTE        *p;
  t_conv32      conv32;

  if (len > 0 AND len <= 8)
  {
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "swapping %d bits", len);
#endif

    /*
     * read bitstring#1
     */
    s1 = bf_decodeByteNumber (len, globs);
    /*
     * read bitstring#2
     */
    s2 = bf_decodeByteNumber (len, globs);
    /*
     * concat bitstring#2 and bitstring#1
     */
    s21 = (USHORT) s2 <<len;
    s21 |= s1;

    /*
     * recode it into the bitstream
     */
    bf_setBitpos (startpos, globs);
    p = globs->bitbuf + globs->bytepos;
    conv32.l = (ULONG) s21;
    conv32.l <<= (32 - len) - globs->byteoffs;
    *p++ |= conv32.c[MSW_POS + MSB_POS];
    *p++ |= conv32.c[MSW_POS + LSB_POS];
    *p++ |= conv32.c[LSW_POS + MSB_POS];
    *p   |= conv32.c[LSW_POS + LSB_POS];

    bf_setBitpos (startpos, globs);
    bf_recodeShortNumber (startpos, (UBYTE) (len<<1), s21, globs);
    bf_setBitpos (startpos, globs);
  }
}

#endif /* !RUN_FLASH */
#endif
