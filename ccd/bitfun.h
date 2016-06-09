/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : bitfun.h
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
|  Purpose :  Condat Coder Decoder
|             Prototypes of the elementary bit manipulation functions
+----------------------------------------------------------------------------- 
*/ 


#ifndef __BITFUN
#define __BITFUN


#ifndef __BITFUN_C__

EXTERN void   bf_writePadBits      (T_CCD_Globs *globs);
EXTERN void   bf_writeVal          (ULONG value, ULONG bSize, T_CCD_Globs *globs);
EXTERN ULONG  bf_getBits           (ULONG  len, T_CCD_Globs *globs);
EXTERN void   bf_writeBitStr_PER   (USHORT len, T_CCD_Globs *globs);
EXTERN void   bf_readBitStr_PER    (USHORT len, T_CCD_Globs *globs);
EXTERN void   bf_writeBits         (ULONG  len, T_CCD_Globs *globs);
EXTERN void   bf_readBits          (ULONG  len, T_CCD_Globs *globs);
EXTERN void   bf_writeBitChunk     (ULONG  len, T_CCD_Globs *globs);
EXTERN void   bf_readBitChunk      (ULONG  len, T_CCD_Globs *globs);
EXTERN BOOL   bf_readBit           (T_CCD_Globs *globs);
EXTERN void   bf_writeBit          (BOOL Bit, T_CCD_Globs *globs);
EXTERN UBYTE  bf_decodeByteNumber  (const ULONG len, T_CCD_Globs *globs);
EXTERN ULONG  bf_decodeShortNumber (const ULONG len, T_CCD_Globs *globs);
EXTERN ULONG  bf_decodeLongNumber  (UBYTE len, T_CCD_Globs *globs);
EXTERN void   bf_codeShortNumber   (UBYTE len, USHORT val, T_CCD_Globs *globs);
EXTERN void   bf_codeByteNumber    (UBYTE len, UBYTE val, T_CCD_Globs *globs);
EXTERN void   bf_codeLongNumber    (UBYTE len, ULONG val, T_CCD_Globs *globs);
EXTERN void   bf_recodeShortNumber (USHORT pos, UBYTE len, USHORT val, T_CCD_Globs *globs);
EXTERN void   bf_recodeByteNumber  (USHORT pos, UBYTE len, UBYTE  val, T_CCD_Globs *globs);
EXTERN void   bf_recodeBit         (USHORT pos, UBYTE Bit, T_CCD_Globs *globs);
EXTERN void   bf_rShift8Bit        (USHORT srcBitPos, USHORT bitLen, T_CCD_Globs *globs);

#endif /* __BITFUN_C__ */

/* a Macro for incrementing the position in the bitbuffer         */
/* _bitpos, _bytepos and _byteoffs are recalculated               */

#define bf_incBitpos(A, globs) globs->bitpos = (USHORT)(globs->bitpos+(A));\
                        globs->bytepos = (USHORT)(globs->bitpos >> 3);\
                        globs->byteoffs = (UBYTE)(globs->bitpos & 7)
#define bf_setBitpos(A, globs) globs->bitpos = (USHORT)(A);\
                        globs->bytepos = (USHORT)(globs->bitpos >> 3);\
                        globs->byteoffs = (UBYTE)(globs->bitpos & 7)

/*
 * end of bitstream if we can not read almost 4 bits
 */
#define bf_endOfBitstream(globs) (globs->bitpos >= globs->maxBitpos)

#endif
