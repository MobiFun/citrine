/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : cdc_std.c
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
|  Purpose :  Condat Conder Decoder 
|             Definitions of non protocol specific encoding and decoding 
|             functions
+----------------------------------------------------------------------------- 
*/ 

#define CCD_STD_C

#ifdef _MSDOS
#include <dos.h>
#include <conio.h>
#endif

/*
 * standard definitions like UCHAR, ERROR etc.
 */
#include "typedefs.h"
#include "header.h"
#include <string.h>

/*
 * Prototypes of ccd (USE_DRIVER EQ undef) for prototypes only
 * look at ccdapi.h.
 */
#undef USE_DRIVER
#include "ccdapi.h"

/*
 * Types and functions for bit access and manipulation
 */
#include "ccd_globs.h"
#include "bitfun.h"

/*
 * Declaration of coder/decoder tables and/or functions to access them
 */
#include "ccdtable.h"
#include "ccddata.h"

/*
 * Prototypes of ccd internal functions
 */
#include "ccd.h"

/*
 * Need memory allocation functions for dynamic arrays (pointers)
 */
#ifdef DYNAMIC_ARRAYS
#include "vsi.h"
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : cdc_decodeElemvalue   |
+--------------------------------------------------------------------+

  PURPOSE : Performs a standard decoding for a given elem table entry.
            This means for non structured elements that 1-n bits are
            read from the bitstream and write to a C-Variable
            in a machine dependent format. For structured elements
            an (indirect) recursive call to cc_decodeComposition()
            is performed. If the element is a bitbuffer with variable
            size, the repeat value gives the number of bits to read
            from the bitstream into the buffer. The maxBitpos
            indicates the maximum valid position for the
            readpointer of the bitstream while decoding this element.
            If the readpointer break this boundary, this element will
            not be decoded.
*/

void cdc_decodeElemvalue (ULONG e_ref, ULONG *repeat, T_CCD_Globs *globs)
{
  UBYTE     *ActStructpos;
  ULONG      i;
  UBYTE      spareLen;

  /*
   * element is a bit field of variable length
   */
  if ((melem[e_ref].repType == 'b') || (melem[e_ref].repType == 's'))
  {
    if (*repeat > (ULONG) (globs->maxBitpos-globs->bitpos))
    {
      ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                       (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
      *repeat = MINIMUM (*repeat, (ULONG) (globs->maxBitpos-globs->bitpos));
    }
    if (melem[e_ref].repType == 'b')
    {
      if (mvar[melem[e_ref].elemRef].cType EQ 'X')
        bf_readBitChunk (*repeat, globs);
      else
        bf_readBits (*repeat, globs); 
    }
    else 
    {
      U16 finalBP = globs->bitpos + (USHORT) *repeat;
      /* Store the limit. This comp may contain other comps as bitstring. */
      globs->maxBitpos = finalBP;
      ActStructpos = globs->pstruct;
      globs->pstruct += globs->pstructOffs;
#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
      TRACE_CCD (globs, "decoding composition %s as a bit array",
                       mcomp[melem[e_ref].elemRef].name);
#else
      TRACE_CCD (globs, "decoding composition %d", melem[e_ref].elemRef);
#endif
#endif

      ccd_decodeComposition ((ULONG) (melem[e_ref].elemRef), globs);
      if (finalBP < globs->bitpos)
      {
        ccd_recordFault (globs, ERR_BITSTR_COMP, CONTINUE, (USHORT) e_ref,
                         globs->pstruct + globs->pstructOffs);
      }
      bf_setBitpos (finalBP, globs);
      /* Update maxBitpos to avoid an early end of decoding. */
      globs->maxBitpos = globs->buflen;
      globs->pstruct = ActStructpos;
    }
  }
  else
  {
    /*
     * For pointer types, globs->pstruct is already set to point to
     * the new memory area, and these types are not treated differently
     * from non-pointer types.
     */
    i=0;
    switch (melem[e_ref].elemType)
    {
      case 'R': /* Pointer to (possible array of) basetype */
      case 'F': /* Code-transparent pointer to (possible array of) basetype */
      case 'V':
        while (i < *repeat)
        {
          if (globs->bitpos < globs->maxBitpos)
          {
#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
            TRACE_CCD (globs, "decoding var %s",
                       ccddata_get_alias((USHORT) e_ref, 1));
#else
            TRACE_CCD (globs, "decoding var %d", melem[e_ref].elemRef);
#endif
#endif
            if (mvar[melem[e_ref].elemRef].cType EQ 'X')
              bf_readBitChunk (mvar[melem[e_ref].elemRef].bSize, globs);
            else
              bf_readBits (mvar[melem[e_ref].elemRef].bSize, globs);

            globs->pstructOffs += mvar[melem[e_ref].elemRef].cSize;

            i++;
          }
          else
          {
            if (melem[e_ref].repType != 'i')
            {
              ccd_recordFault (globs, ERR_ELEM_LEN, CONTINUE, (USHORT) e_ref,
                               globs->pstruct + globs->pstructOffs);
            }
            break;
          }
        }
        break;

      case 'D': /* Pointer to a composition */
      case 'P': /* Code transparent pointer to a comp */
      case 'C': /* Element is a composition. */
      case 'U': /* Element is a union. */
        /*
         * Store the actual structure position.
         */
        ActStructpos = globs->pstruct;

        globs->pstruct += globs->pstructOffs;

        while (i < *repeat)
        {
          if (globs->bitpos < globs->maxBitpos)
          {
#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
            TRACE_CCD (globs, "decoding composition %s",
                       mcomp[melem[e_ref].elemRef].name);
#else
            TRACE_CCD (globs, "decoding composition %d", melem[e_ref].elemRef);
#endif
#endif
		        /*
		         * recursiv call
		         */
            ccd_decodeComposition ((ULONG) (melem[e_ref].elemRef), globs);
            globs->pstruct += mcomp[melem[e_ref].elemRef].cSize;

            i++;
          }
          else
          {
            if (melem[e_ref].repType != 'i')
            {
              ccd_recordFault (globs, ERR_ELEM_LEN, CONTINUE, (USHORT) e_ref,
                               globs->pstruct + globs->pstructOffs);
            }
            break;
          }
        }
        /*
         * restore the write pointer
         */
        globs->pstruct = ActStructpos;
        break;

      case 'S': /* Element is a spare. */
      {
        spareLen = spare[melem[e_ref].elemRef].bSize;
        /*
         * Do not decode padding bits. They are not relevant.
         * Just adjust the position pointer in the bit stream buffer.
         */
        while (i < *repeat)
        {
          if (globs->bitpos < globs->maxBitpos)
          {
#ifdef DEBUG_CCD
            TRACE_CCD (globs, "decoding spare");
#endif
            bf_incBitpos (spareLen, globs);
            i++;
          }
          else
            break;
        }
        break;
      }
    }
    *repeat = i;
  }
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : cdc_encodeElemvalue   |
+--------------------------------------------------------------------+

  PURPOSE : Performs a standard encoding for a given elem table entry.
            This means for non structured elements that 1-n bits are
            read from the bitstream and write to a C-Variable
            in a machine dependent format. For structured elements
            an (indirect) recursive call to cc_decodeComposition()
            is performed. If the element is a bitbuffer with variable
            size, the repeat value gives the number of bits to write
            from the buffer into the bitstream.
*/

void cdc_encodeElemvalue (ULONG e_ref, ULONG repeat, T_CCD_Globs *globs)
{
  UBYTE     *ActStructpos = NULL;
  ULONG      i;
  UBYTE      spareLen;

  /*
   * Element is a bit field of variable length.
   */
  if (melem[e_ref].repType == 'b')
  {
    if (mvar[melem[e_ref].elemRef].cType EQ 'X')
      bf_writeBitChunk (repeat, globs);
    else
      bf_writeBits (repeat, globs);
  }
  /*
   * Element is a structured IE defined as bit string.
   */
  else if (melem[e_ref].repType == 's')
  {
    U16 finalBP = (USHORT) (globs->bitpos + repeat);
    ActStructpos = globs->pstruct;
    globs->pstruct += globs->pstructOffs;
#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
    TRACE_CCD (globs, "encoding composition %s as a bit array",
                       mcomp[melem[e_ref].elemRef].name);
#else
    TRACE_CCD (globs, "encoding composition %d", melem[e_ref].elemRef);
#endif
#endif

    ccd_encodeComposition ((ULONG) melem[e_ref].elemRef, globs);
    if (finalBP < globs->bitpos)
    {
      ccd_recordFault (globs, ERR_BITSTR_COMP, CONTINUE, (USHORT) e_ref, 
                       globs->pstruct + globs->pstructOffs);
    }
    bf_setBitpos (finalBP, globs);
    globs->pstruct = ActStructpos;
  }
  else
  {
    /*
     * For pointer types, globs->pstruct is already set to point to
     * the new memory area, and these types are not treated differently
     * from non-pointer types.
     */
    switch(melem[e_ref].elemType)
    {
      case 'R': /* Pointer to (possible array of) basetype */
      case 'F': /* Code-transparent pointer to (possible array of) basetype */
      case 'V':
        for (i=0; i<repeat; i++)
        {
  #ifdef DEBUG_CCD
  #ifdef CCD_SYMBOLS
          TRACE_CCD (globs, "encoding var %s",
                     ccddata_get_alias((USHORT) e_ref, 1));
  #else
          TRACE_CCD (globs, "encoding var %s", melem[e_ref].elemRef);
  #endif
  #endif
          if (mvar[melem[e_ref].elemRef].cType EQ 'X')
            bf_writeBitChunk (mvar[melem[e_ref].elemRef].bSize, globs);
          else
            bf_writeBits (mvar[melem[e_ref].elemRef].bSize, globs);

          globs->pstructOffs += mvar[melem[e_ref].elemRef].cSize;
        }
        break;

      case 'D': /* Pointer to a composition (already dereferenced) */
      case 'P': /* Code transparent pointer to a comp (already dereferenced) */
      case 'C': /* Element is a composition. */
      case 'U': /* Element is a union. */
        /*
         * store the actual structure position
         */
        ActStructpos = globs->pstruct;

        globs->pstruct += globs->pstructOffs;

        for (i=0; i<repeat; i++)
        {
  #ifdef DEBUG_CCD
  #ifdef CCD_SYMBOLS
          TRACE_CCD (globs, "encoding composition %s",
                     mcomp[melem[e_ref].elemRef].name);
  #else
          TRACE_CCD (globs, "encoding composition %d", melem[e_ref].elemRef);
  #endif
  #endif
          ccd_encodeComposition ((ULONG) melem[e_ref].elemRef, globs);	/* recursiv call */
          globs->pstruct += mcomp[melem[e_ref].elemRef].cSize;
        }
        /*
         * restore the write pointer
         */
        globs->pstruct = ActStructpos;
        break;

      case 'S': /* element is a spare          */
      {
        spareLen = spare[melem[e_ref].elemRef].bSize;

        /*
         * encode the spare
         */
        for (i=0; i < repeat; i++)
        {
  #ifdef DEBUG_CCD
          TRACE_CCD (globs, "encoding spare");
  #endif
          bf_codeLongNumber (spareLen, spare[melem[e_ref].elemRef].value, globs);
        }
        break;
      }
    }
  }
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : cdc_STD_decode        |
+--------------------------------------------------------------------+

  PURPOSE : Performs a standard decoding for a given elem table entry.
            This means for non structured elements that 1-n bits are
            read from the bitstream and write to a C-Variable
            in a machine dependent format. For structured elements
            an (indirect) recursive call to cc_decodeComposition()
            is performed.
            The element may be conditional and/or repeatable.
*/

SHORT cdc_std_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG               repeat, amount, act_offset;
  BOOL                is_variable;
  U8                 *old_pstruct = NULL;
  ULONG               cix_ref, num_prolog_steps, prolog_step_ref;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_std_decode()");
	#else
	TRACE_CCD (globs, "cdc_std_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;

  /*
   * if this element is conditional, check the condition
   */
  if (calcidx[cix_ref].numCondCalcs NEQ 0
  AND ! ccd_conditionOK (e_ref, globs))
    return 1;

  /*
   * if this element has a defined prologue
   * we have to process it before decoding the bitstream
   * If there are some epilogue expressions to be processed for this element
   * (rare cases) the result here will be a reading of 0 to an internal
   * register. The valid processing of expression takes place after the 
   * decoding of the element. 
   */
  if (num_prolog_steps)
  {
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }
  
  /*
   * if this element is repeatable, and the number of
   * repeats depends on another element, calculate the repeater
   */
  if (melem[e_ref].repType NEQ ' ')
  {
    is_variable = ccd_calculateRep (e_ref, &repeat, &act_offset, globs);
  }
  else
  {
    repeat = 1;
    is_variable = FALSE;
  }

  if (melem[e_ref].elemType NEQ 'S')
  {
    /*
     * Element is not a SPARE.
     * Setup the offset into the C-structure for this element
     */
    globs->pstructOffs = melem[e_ref].structOffs;

    if (melem[e_ref].optional)
    {
      /* 20010621 MVJ: Dynamic array addition.
       * Postpone optional flag setting for non-code transparent
       * pointer types ('P', 'Q', 'R').
       * For these types, the optional flag is the pointer itself.
       * These types cannot be set yet, as the pointer may be
       * preceeded by a counter octet, a union tag id octet etc.
       */
      if (melem[e_ref].elemType < 'P' OR melem[e_ref].elemType > 'R')
	      globs->pstruct[globs->pstructOffs++] = (UBYTE) TRUE;
    }

    if (is_variable)
    {
      /*
       * for variable sized elements store the min-value
       * as counter into the C-Structure (c_xxx).
       */
      if (act_offset > 65535)
        *(ULONG *) (globs->pstruct + globs->pstructOffs++) = repeat;
      else if (act_offset > 255)
        *(USHORT *) (globs->pstruct + globs->pstructOffs++) = (USHORT) repeat;
      else
        globs->pstruct[globs->pstructOffs] = (UBYTE) repeat;

      globs->pstructOffs++;
    }
  }
  #ifdef DYNAMIC_ARRAYS
  /*
   * MVJ: Dynamic array addition.
   * Check for pointer types; allocate memory if necessary.
   */
  if ( is_pointer_type(e_ref) ) {
    U32     cSize;
    U8      *addr;

    /*
     * Find size to allocate;
     * - Read from mcomp or mvar according to type
     */
    cSize = (ULONG)((melem[e_ref].elemType EQ 'V' OR
		      melem[e_ref].elemType EQ 'R')
                     ? mvar[melem[e_ref].elemRef].cSize
                     : mcomp[melem[e_ref].elemRef].cSize
		     ) * repeat;

    /*
     * Allocate additional memory
     */
    addr = (U8 *)DP_ALLOC( cSize, globs->alloc_head, DP_NO_FRAME_GUESS);

    /* If no memory, log error and return immediately */
    if (addr EQ NULL) {
      ccd_setError (globs, ERR_NO_MEM,
                    BREAK,
                    (USHORT) -1);
      return 1;
    }
    else
      memset (addr, 0, (size_t)cSize);

    /*
     * Memory allocated;
     * 1. Save old "globs->pstruct" variables
     * 2. Store pointer to freshly allocated memory area in structure
     * 3. Initialize pstruct to point to the freshly allocated memory area.
     * 4. Initialize pstructOffs to 0 to start decoding at offset 0
     *    in the new memory area.
     */
    old_pstruct        = globs->pstruct;
    *(U8 **)(globs->pstruct + globs->pstructOffs) = addr;
    globs->pstruct     = addr;
    globs->pstructOffs = 0;
  }
#endif

  amount = repeat;

  cdc_decodeElemvalue (e_ref, &amount, globs);

  /*
   * process the epilogue expression for this element if there is any 
   */
  if (num_prolog_steps)
  {
    if (  (calc[prolog_step_ref+1].operation EQ 'K')
       || (calc[prolog_step_ref+1].operation EQ 'C')
       || (calc[prolog_step_ref+1].operation EQ 's'))
    {
      ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
    }
  }

#ifdef DYNAMIC_ARRAYS
  /*
   * Restore globs->pstruct for possible use below
   */
  if (old_pstruct NEQ NULL)
  {
    globs->pstruct     = old_pstruct;
  }
#endif
  if (amount NEQ repeat AND is_variable)
  {
    /*
     * if the decoded elements are not equal the specified
     * repeat value, because the bitstream or the IE ended,
     * store the new c_xxx value.
     */
    globs->pstructOffs = melem[e_ref].structOffs;

    if (melem[e_ref].optional)
      globs->pstructOffs++;

      if (act_offset > 65535)
        *(ULONG *) (globs->pstruct + globs->pstructOffs) = amount;
      else if (act_offset > 255)
        *(USHORT *) (globs->pstruct + globs->pstructOffs) = (USHORT) amount;
      else
        globs->pstruct[globs->pstructOffs] = (UBYTE) amount;

    if (melem[e_ref].repType NEQ 'i')
    {
      /*
       * if this element is not of the repeat style 'interval'
       * ([X..Y] where X and Y are constants) we have to generate
       * an ccd error because some outstanding repeats are missing.
       */
      ccd_setError (globs, ERR_MAND_ELEM_MISS, CONTINUE, (USHORT) -1);
    }
  }

  return 1;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : cdc_std_encode        |
+--------------------------------------------------------------------+

  PURPOSE : Performs a standard encoding for a given elem table entry.
            This means for non structured elements that m bytes read
            from the C-Variable, converted to MSB-first format and
            write 1-n bits at the end of the bitstream.
            For structured elements an (indirect) recursive call
            to ccd_encodeComposition() is performed.
            The element may be conditional and/or repeatable.
*/

SHORT cdc_std_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG               repeat, max_rep;
  BOOL                is_variable;
  ULONG               cix_ref, num_prolog_steps, prolog_step_ref;
#ifdef DYNAMIC_ARRAYS
  U8                  *old_pstruct = NULL;
#endif

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_std_encode()");
	#else
	TRACE_CCD (globs, "cdc_std_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;
  /*
   * if this element is conditional, check the condition
   */
  if (calcidx[cix_ref].numCondCalcs NEQ 0
  AND ! ccd_conditionOK (e_ref, globs))
    return 1;

  /*
   * if this element have a defined Prolog
   * we have to process it before decoding the bitstream
   */
  if (num_prolog_steps)
  {
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }
  
  if (melem[e_ref].elemType NEQ 'S')
  {
    /*
     * Element is not a SPARE.
     * Setup the readpointer into the C-structure for this element
     * MVJ: In case of pointer types, the pstructOffs must be
     * the offset into the memory area pointed to. CCDGEN must
     * ensure this holds true.
     */
    globs->pstructOffs = melem[e_ref].structOffs;

    if (melem[e_ref].optional)
    {
      /*
       * for optional elements check the valid-flag in the C-struct.
       * Spare elements does not have a corresponding valid flag. For
       * the spare elements we have to calculate and check the
       * condition to decide if this elements have to be coded.
       */
      /* 20010621 MVJ: Dynamic array addition.
       * Postpone optional check for non-code transparent pointer
       * types ('P', 'Q', 'R').
       * For these types, the optional flag is the pointer itself.
       * These types cannot be checked yet, as the pointer may be
       * preceeded by a counter octet, a union tag id octet etc.
       */
      if (melem[e_ref].elemType < 'P' OR melem[e_ref].elemType > 'R')
      { 
        if (globs->pstruct[globs->pstructOffs++] == FALSE)
        {
          return 1;
        }
#ifdef DEBUG_CCD
        else if (globs->pstruct [melem[e_ref].structOffs] != TRUE)
        {
          TRACE_CCD (globs, "Ambiguous value for valid flag!\n...assumed 1 for ccdID=%d",
                     e_ref);
        }
#endif
      }
    }

    if ((melem[e_ref].repType EQ 'v' OR melem[e_ref].repType EQ 'i'))
    {
      /*
       * for variable sized elements read the amount
       * of repeats out of the C-Structure (c_xxx).
       * If the number of repeats given by the C-Structure 
       * exceeds the allowed value (maxRepeat) CCD gives a warning!
       */
      if (melem[e_ref].maxRepeat > 255)
      {
        ULONG count = (ULONG) (* (USHORT *)(globs->pstruct + globs->pstructOffs++));
        repeat = MINIMUM (count, (ULONG) melem[e_ref].maxRepeat);
        if (repeat < count) 
          ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                           (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
      }
      else
      {
        repeat = (ULONG)MINIMUM (globs->pstruct[globs->pstructOffs], 
                                  melem[e_ref].maxRepeat);
        if ( repeat < (ULONG) (globs->pstruct[globs->pstructOffs]) ) 
          ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                           (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
      }
      globs->pstructOffs++;
    }
    else
      if (melem[e_ref].repType EQ 'c')
      {
        repeat = (ULONG) melem[e_ref].maxRepeat;
      }
      else
        if (melem[e_ref].repType == 's' || melem[e_ref].repType == 'b')
        {
          switch (melem[e_ref].elemType)
          {
            case 'R': /* Pointer to (possible array of) basetype */
            case 'F': /* Code-transparent pointer to (possible array of) basetype */
            case 'V':
              globs->maxBitpos = globs->bitpos + mvar[melem[e_ref].elemRef].bSize;
              break;
            case 'D': /* Pointer to a composition */
            case 'P': /* Code transparent pointer to a comp */
            case 'C': /* Element is a composition. */
            case 'E': /* Pointer to a union */
            case 'Q': /* Code transparent pointer to a union */
            case 'U': /* Element is a union. */
              globs->maxBitpos = globs->bitpos + mcomp[melem[e_ref].elemRef].bSize;
              break;
          }
          
          is_variable = ccd_calculateRep (e_ref, &repeat, &max_rep, globs);
        }
        else
          repeat = 1;

    /* 20010621 MVJ: Dynamic array addition.
     * Check for non-code transparent pointer types ('P', 'Q', 'R').
     * For these types, the optional flag is the pointer itself.
     * ASSUMPTION: The pointer may be preceeded by a counter octet,
     * a union tag id octet etc., but it is up to CCDGEN to ensure
     * word alignment (by inserting alignment bytes). Therefore
     * we just read from globs->pstruct[globs->pstructOffs].
    */
#ifdef DEBUG_CCD
    /* Check pointer alignment and re-align if necessary (should never happen) */
    if ( is_pointer_type(e_ref) AND ((globs->pstructOffs & 3) NEQ 0)) {
      TRACE_CCD (globs, "cdc_STD_encode(): Pointer misaligned! pstruct=0x08x,"
		 " pstructOffs=0x%08x", globs->pstruct, globs->pstructOffs);
      globs->pstructOffs = (globs->pstructOffs + 3) & 3;
    }
#endif
#ifdef DYNAMIC_ARRAYS
    /*
     * MVJ: Perform pointer dereference for pointer types.
     * Also, check optionality for these types.
     */
    if ( is_pointer_type(e_ref) ) {
      U8 *deref_pstruct;

      /* Get pointer value */
      deref_pstruct = *(U8 **)&globs->pstruct[globs->pstructOffs];

      /*
       * Strictly speaking the 'D' to 'F' types should not need this
       * check (should have returned after the optionality check above),
       * but it will catch stray NULL pointers (or uninitialized
       * valid flags)
      */
      if (ccd_check_pointer(deref_pstruct) != ccdOK )
      {
        ccd_recordFault (globs, ERR_INVALID_PTR, BREAK, (USHORT) e_ref, 
                         &globs->pstruct[globs->pstructOffs]);
        return 1;
      }
      /*
       * Pointer not NULL;
       * 1. Save old globs->pstruct and assign pointer to globs->pstruct
       *    as new base.
       * 2. Set pstructOffs to 0 (zero) as the next offset will start
       *    in the new memory area.
       */
      old_pstruct        = globs->pstruct;
      globs->pstruct     = deref_pstruct;
      globs->pstructOffs = 0;
    }
#endif /* DYNAMIC_ARRAYS */
  }
  else
  {
    /*
     * for spare elements we have to calculate the conditions
     * and the repetitions because there are no valid-flags and
     * c_xxx variables in the C-structure to read.
     */
    if (melem[e_ref].optional)
    {
      /*
       * Spare elements does not have a corresponding valid flag.
       * For the spare elements we have to calculate and check the
       * condition to decide if this elements have to be coded.
       */
      if (calcidx[cix_ref].numCondCalcs NEQ 0
      AND ! ccd_conditionOK (e_ref, globs))
        return 1;
    }
    /*
     * if this spare is repeatable, calculate the amount of
     * repeats because there is no corresponding c_xxx element
     * in the C-Structure.
     */
    if (melem[e_ref].repType NEQ ' ')
    {
      globs->maxBitpos = globs->bitpos + spare[melem[e_ref].elemRef].bSize;
      is_variable = ccd_calculateRep (e_ref, &repeat, &max_rep, globs);
    }
    else
      repeat = 1;
  }

  cdc_encodeElemvalue (e_ref, repeat, globs);

#ifdef DYNAMIC_ARRAYS
  if ( old_pstruct NEQ NULL )
    globs->pstruct = old_pstruct;
#endif

  return 1;
}
#endif /* !RUN_FLASH */

/*
 * some elementary bitfunctions
 */

/* LSB,MSB Definitions for the CPUs */

#ifdef M_INTEL
#define MSB_POS 1
#define LSB_POS 0
#define MSW_POS 2
#define LSW_POS 0
#else /* M_INTEL */
#ifdef M_MOTOROLA
#define MSB_POS 0
#define LSB_POS 1
#define MSW_POS 0
#define LSW_POS 2
#endif /* M_MOTOROLA */
#endif /* M_INTEL */

extern const ULONG ccd_bitfun_mask[];

/*
 * Table of one-bit values (2^X)
 * This unused variable is commented now in order to avoid some compiler 
 * warnings like:  variable "shift" (line xxx) is not used

LOCAL const UBYTE shift[] =
{
  128, 64, 32, 16, 8, 4, 2, 1
};
 */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_codeByte          |
+--------------------------------------------------------------------+

 PURPOSE:   encodes the value of (value) into a CPU-dependent
            bitstring. The length of the bitstring is specified
            throu (bitlen). The function copies the bitstring into
            the buffer (bitstream) at bit position (startbit).
            The bitlen may be between 1 and 8.

*/

BYTE CCDDATA_PREF(ccd_codeByte) (UBYTE * bitstream,
                                 USHORT startbit,
              		               USHORT bitlen,
                                 UBYTE value)
{
  union
  {	/* Conversion structure 2 Byte <-> unsigned short */
    UBYTE c[2];
    USHORT s;
  }
  conv;

  UBYTE *p, lshift;

  USHORT m;

  p = bitstream + (startbit >> 3);

  lshift = (16 - ((startbit & 7) + bitlen));

  conv.c[MSB_POS] = p[0];

  conv.c[LSB_POS] = p[1];

  m = ((USHORT) ccd_bitfun_mask[bitlen]) << lshift;

  conv.s &= ~m;

  conv.s |= ((value << lshift) & m);

  p[0] = conv.c[MSB_POS];

  p[1] = conv.c[LSB_POS];

  return ccdOK;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_decodeByte        |
+--------------------------------------------------------------------+

 PURPOSE:   decodes (bitlen) bits from the (bitstream) at position
            (startbit) and converts them to a numeric value (value)
            The bitlen may be between 1 and 8.

*/

BYTE CCDDATA_PREF(ccd_decodeByte) (UBYTE *bitstream,
                                   USHORT startbit,
                                   USHORT bitlen,
                                   UBYTE *value)
{
  union
  {	/* Conversion structure 2 Byte <-> unsigned short */
    UBYTE c[2];
    USHORT s;
  }
  conv;

  UBYTE *p;

  p = bitstream + (startbit >> 3);

  conv.c[MSB_POS] = *p++;

  conv.c[LSB_POS] = *p;

  conv.s >>= (16 - ((startbit & 7) + bitlen));

  conv.s &= (USHORT) ccd_bitfun_mask[bitlen];

  *value = (UBYTE) conv.s;

  return ccdOK;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_codeLong          |
+--------------------------------------------------------------------+

 PURPOSE:   encodes the value of (value) into a CPU-dependent
            bitstring. The length of the bitstring is specified
            throu (bitlen). The function copies the bitstring into
            the buffer (bitstream) at bit position (startbit).
            The bitlen may be between 1 and 32.

*/

BYTE CCDDATA_PREF(ccd_codeLong) (UBYTE *bitstream,
                                 USHORT startbit,
	                               USHORT bitlen,
                                 ULONG value)
{
  UBYTE *p;

  union
  {	/* Conversion structure 4 Byte <-> unsigned long */
    UBYTE c[4];
    ULONG l;
  }
  conv;

  p = bitstream + (startbit >> 3);
  startbit &= 7;

  conv.l = value & ccd_bitfun_mask[bitlen];
  conv.l <<= (32-bitlen-startbit);

  p[0] &= (UCHAR)~ccd_bitfun_mask[8-startbit];

  switch ((USHORT)(startbit+bitlen-1) >> 3)
  {
    case 0:
      p[0] |= conv.c[MSW_POS+MSB_POS];
      break;
    case 1:
      p[0] |= conv.c[MSW_POS+MSB_POS];
      p[1]  = conv.c[MSW_POS+LSB_POS];
      break;
    case 2:
      p[0] |= conv.c[MSW_POS+MSB_POS];
      p[1]  = conv.c[MSW_POS+LSB_POS];
      p[2]  = conv.c[LSW_POS+MSB_POS];
      break;
    case 3:
      p[0] |= conv.c[MSW_POS+MSB_POS];
      p[1]  = conv.c[MSW_POS+LSB_POS];
      p[2]  = conv.c[LSW_POS+MSB_POS];
      p[3]  = conv.c[LSW_POS+LSB_POS];
      break;
    default:
      p[0] |= conv.c[MSW_POS+MSB_POS];
      p[1]  = conv.c[MSW_POS+LSB_POS];
      p[2]  = conv.c[LSW_POS+MSB_POS];
      p[3]  = conv.c[LSW_POS+LSB_POS];
      p[4]  = (UBYTE) ((value & 0xff) << (8-startbit));
      break;
  }
  return ccdOK;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_decodeLong        |
+--------------------------------------------------------------------+

 PURPOSE:   decodes (bitlen) bits from the (bitstream) at position
            (startbit) and converts them to a numeric value (value)
            The bitlen may be between 1 and 32.

*/

BYTE CCDDATA_PREF(ccd_decodeLong) (UBYTE *bitstream,
                                   USHORT startbit,
	                                 USHORT bitlen,
                                   ULONG *value)
{
  UBYTE *p;

  union
  {	/* Conversion structure 4 Byte <-> unsigned long */
    UBYTE c[4];
    ULONG l;
  }
  conv;

  p = bitstream + (startbit >> 3);
  startbit &= 7;

  conv.l = 0L;
  switch ((USHORT)(startbit+bitlen-1) >> 3)
  {
    case 0:
      conv.c[MSW_POS+MSB_POS] = p[0];
      conv.l <<= startbit;
      conv.l >>= (32-bitlen);
      break;
    case 1:
      conv.c[MSW_POS+MSB_POS] = p[0];
      conv.c[MSW_POS+LSB_POS] = p[1];
      conv.l <<= startbit;
      conv.l >>= (32-bitlen);
      break;
    case 2:
      conv.c[MSW_POS+MSB_POS] = p[0];
      conv.c[MSW_POS+LSB_POS] = p[1];
      conv.c[LSW_POS+MSB_POS] = p[2];
      conv.l <<= startbit;
      conv.l >>= (32-bitlen);
      break;
    case 3:
      conv.c[MSW_POS+MSB_POS] = p[0];
      conv.c[MSW_POS+LSB_POS] = p[1];
      conv.c[LSW_POS+MSB_POS] = p[2];
      conv.c[LSW_POS+LSB_POS] = p[3];
      conv.l <<= startbit;
      conv.l >>= (32-bitlen);
      break;
    default:
      conv.c[MSW_POS+MSB_POS] = p[0];
      conv.c[MSW_POS+LSB_POS] = p[1];
      conv.c[LSW_POS+MSB_POS] = p[2];
      conv.c[LSW_POS+LSB_POS] = p[3];
      conv.l <<= startbit;
      conv.l >>= (32-bitlen);
      conv.c[LSW_POS+LSB_POS] |= (p[4] >> (8-startbit));
      break;
  }
  *value = conv.l & ccd_bitfun_mask[bitlen];
  return ccdOK;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_bitcopy           |
+--------------------------------------------------------------------+

 PURPOSE: copys bitlen bits from the source buffer to the destination
          buffer. offset contains the position of the first bit in
          the source bitfield. This function may perform a leftshift
          to adjust the most significant bit on byte boundarys of the
          first byte in dest.

*/

void CCDDATA_PREF(ccd_bitcopy) (UBYTE     *dest,
                                UBYTE     *source,
                                USHORT     bitlen,
                                USHORT     offset)
{
  union
  {	/* Conversion structure 2 Byte <-> unsigned short */
    UBYTE  c[2];
    USHORT s;
  }
  conv;

  USHORT l_shift       = offset & 7;
  USHORT bytes_to_copy = (bitlen >> 3);

  /*
   * go to the byte that contains the first valid bit.
   */
  source += (offset >> 3);


  if (l_shift)
  {
    /*
     * shift and copy each byte
     */
    while (bytes_to_copy--)
    {
      conv.c[MSB_POS] = *source;
      source++;
      conv.c[LSB_POS] = *source;
      conv.s <<= l_shift;
      *dest = conv.c[MSB_POS];
      dest++;
    }
  }
  else
  {
    /*
     * no shift operation, do a memcopy;
     */
    while (bytes_to_copy--)
    {
      *dest = *source;
      dest++;
      source++;
    }
  }
  /*
   * cutoff the garbage at the end of the bitstream
   */
 
  *(dest-1) &= (UCHAR)(~ccd_bitfun_mask[(8-(bitlen&7))&7]);
}
#endif /* !RUN_FLASH */
