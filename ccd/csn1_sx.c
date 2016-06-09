/* 
+----------------------------------------------------------------------------- 
|  Project : CCD
|  Modul   : csn1_sx.c
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
|  Purpose :  Definition of encoding and decoding functions for CSN1_S0 elements
+----------------------------------------------------------------------------- 
*/
/*
 * standard definitions like GLOBAL, UCHAR, ERROR etc.
 */
#include "typedefs.h"
#include "header.h"


/*
 * Prototypes of ccd (USE_DRIVER EQ undef) for prototypes only
 * look at ccdapi.h
 */
#undef USE_DRIVER
#include "ccdapi.h"

/*
 * Types and functions for bit access and manipulation
 */
#include "ccd_globs.h"
#include "bitfun.h"

/*
 * Prototypes of ccd internal functions
 */
#include "ccd.h"

/*
 * Declaration of coder/decoder tables
 */
#include "ccdtable.h"
#include "ccddata.h"

/*
 * Need memory allocation functions for dynamic arrays (pointers)
 */
#ifdef DYNAMIC_ARRAYS
#include "vsi.h"
#include <string.h>
#endif


#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : cdc_csn1_sx_decode    |
+--------------------------------------------------------------------+

  PURPOSE : The encoded IE consists of one bit for presence flag and 
            a value part. 
            In case of CSN1_S1 only if the flag bit is equal 1 the  
            value part will follow it.
            In case of CSN1_S0 only if the flag bit is equal 0 the 
            value part will follow it.
*/

SHORT cdc_csn1_sx_decode (int flag, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG  repeat, max_rep, act_offset;
  ULONG  amount = 1;
  BOOL   is_variable;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;
#ifdef DYNAMIC_ARRAYS
  U8     *old_pstruct = NULL;
  U8     *addr = NULL;
#endif

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_csn1_sx_decode()");
	#else
	TRACE_CCD (globs, "cdc_csn1_sx_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  globs->SeekTLVExt = FALSE;
  
  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;

  /*
   * If this element is conditional, check the condition.
   */
  if (calcidx[cix_ref].numCondCalcs NEQ 0
  AND ! ccd_conditionOK (e_ref, globs))
    return 1;

  /*
   * If this element has a defined prologue
   * we have to process it before decoding the bitstream.
   */
  if (num_prolog_steps)
  {
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }
  
  if (melem[e_ref].repType NEQ ' ')
  {
    is_variable = ccd_calculateRep (e_ref, &repeat, &max_rep, globs);
    /* Structured IE is to be handeled as bitstring.*/
    if (melem[e_ref].repType == 's')
    {
      repeat--;
    }
  }
  else
  {
    is_variable = FALSE;
    repeat     = 1;
  }

#ifdef DYNAMIC_ARRAYS
  /*
   * Check for pointer types; allocate memory if necessary.
   * May overwrite globs->pstruct (and initialize globs->pstructOffs to 0).
   */
  if ( is_pointer_type(e_ref) ) 
  {
    U32     cSize;

    /*
     * Find size to allocate;
     * - Read from mcomp or mvar according to type
     */
    cSize = (ULONG)((melem[e_ref].elemType EQ 'F' OR
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
     */
  }
#endif
    
  if (melem[e_ref].elemType NEQ 'S')
  {
    /*
     * Element is not a SPARE. Setup the struct pointer.
     */
    globs->pstructOffs = melem[e_ref].structOffs;


    if (is_variable)
    {
      UBYTE *addr_v_xxx = NULL;
      UBYTE *addr_c_xxx;
      UBYTE  act_continue_array;
  

      if (melem[e_ref].optional)
      {
        /*
         * For optional elements we must set the valid-flag,
         * if there is at least one element in the message.
         * Therefore we store the address of the valid flag.
         */
        /* Dynamic array addition.
         * Postpone optional flag setting for non-code transparent
         * pointer types ('P', 'Q', 'R').
         * For these types, the optional flag is the pointer itself.
         * These types cannot be set yet, as the pointer may be
         * preceeded by a counter octet, a union tag id octet etc.
         */
        if (melem[e_ref].elemType < 'P' OR melem[e_ref].elemType > 'R')
        addr_v_xxx = (UBYTE *) (globs->pstruct + globs->pstructOffs++);
      }
      /*
       * For variable sized elements store the min-value
       * as counter into the C-Structure (c_xxx).
       */
      addr_c_xxx = (UBYTE *) (globs->pstruct + globs->pstructOffs++);
      if (max_rep > 255)
        globs->pstructOffs++;      
      
      /*
       * Store the initial offset
       */

      act_offset = (ULONG) globs->pstructOffs;

#ifdef DYNAMIC_ARRAYS
    if ( is_pointer_type(e_ref) ) 
    {
      /*
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

      /*
       * repType ='i':
       * Repeat this element (if it is an array) until we detect a 
       * flag indicating absence.
       *
       * repType ='v' and 'c':
       * Repeat the IE and leave the loop.
       *
       * In both cases we expect a 0 at first.
       */
      if ((melem[e_ref].repType == 'v') || (melem[e_ref].repType == 'c'))
      {
        amount = repeat;
      }
      repeat = 0;
      act_continue_array = globs->continue_array;
      globs->continue_array = TRUE;

      while (globs->continue_array)
      {
        if (flag == 0xFF) /* CSN1_SH type*/
        {
          if (bf_readBit(globs) != GET_HL_PREV(1))
            break;
        }
        else
        {
          if (bf_readBit(globs) != flag)
            break;
        }
          /*
           * Flag is set, we must decode the element
           */
  
          cdc_decodeElemvalue (e_ref, &amount, globs);

          if (++repeat > max_rep)
          {
            /*
             * Too many repetitions means error in encoded message.
             */
            ccd_setError (globs, ERR_MAX_REPEAT,
                          BREAK,
                          (USHORT) (globs->bitpos),
                          (USHORT) -1);
            return 1;
          }
          else if (melem[e_ref].repType EQ 'v')
          {
            repeat = amount;
            break;
          }
          /*
           * Recalculate the struct offset for repeatable IEs.
           */
          if (is_variable_type(e_ref))
          {
            globs->pstructOffs = (USHORT)(act_offset + 
                                 (repeat * mvar[melem[e_ref].elemRef].cSize));
          }
          else
          {
            globs->pstructOffs = (USHORT)(act_offset +
                                 (repeat * mcomp[melem[e_ref].elemRef].cSize));
          }
        }
      globs->continue_array = act_continue_array;
              
#ifdef DYNAMIC_ARRAYS
      /*
       * Restore globs->pstruct 
       */
      if (old_pstruct NEQ NULL)
      {
        globs->pstruct     = old_pstruct;
      }
#endif

      if (addr_v_xxx NEQ NULL)
      {
        /*
         * For optional elements set the valid-flag
         * In this case the pointer addr_c_xxx does not mark
         * the counter. It points to the valid flag.
         */
        if (repeat > 0)
          *addr_v_xxx++ = (UBYTE) TRUE;
      }
      /*
       * Store the number of digits into the c_xxx variable if there is one.
       */
      if (max_rep > 65535)
      {
       ULONG *addr_c_xxx_u32;
       addr_c_xxx_u32 = (ULONG *)addr_c_xxx;
       *addr_c_xxx_u32 = repeat;
      }
      else if (max_rep > 255)
      {
        USHORT *addr_c_xxx_u16;
        addr_c_xxx_u16 = (USHORT *)addr_c_xxx;
        *addr_c_xxx_u16 = (USHORT) repeat;
      }
      else
        *addr_c_xxx = (UBYTE) repeat;
    }
    /*
     * IE is not defined as an array.
     */
    else
    {
      BOOL  elemPresent;

      if (flag == 0xFF)     
        elemPresent = (bf_readBit(globs) EQ GET_HL_PREV(1));
      else 
        elemPresent = (bf_readBit(globs) == flag);
        
      if (elemPresent)
      {
        if (melem[e_ref].optional)
        {
          /*
           * For optional elements set the valid-flag.
           */
            globs->pstruct[globs->pstructOffs++] = (UBYTE) TRUE;
        }
        
        /*
         * Flag is set, we must decode the element.
         */
        cdc_decodeElemvalue (e_ref, &repeat, globs);
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
      }
    }
  }

  return 1;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_GSM                |
| STATE   : code                    ROUTINE : cdc_csn1_sx_encode     |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of the CSN1 element.
            1) GSM Type CSN1 S1 element
               This element consists of a 1 bit valid flag and a 
               value part. If the element is valid (the v_xxx 
               components is TRUE in the decoded message) a 1 bit will
               be coded followed by the coding of the value part.
               Otherwise a 0 bit will be coded.

            2) GSM Type CSN1 S0 element
               This element consists of a single bit valid flag and a 
               value part, too. But in this case the presence flag is
               set to 0. If the element is present (the v_xxx component
               is TRUE in the decoded message) a 0 bit will be coded 
               followed by the encoded value part. Otherwise a 1 bit
               will be encoded.
*/

SHORT cdc_csn1_sx_encode (int flag, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG  i, repeat=1, amount=1;
  USHORT cSize = 0, startOffset;
  int    elemPresent;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;
#ifdef DYNAMIC_ARRAYS
  U8     *old_pstruct = NULL;
#endif

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_csn1_sx_encode()");
	#else
	TRACE_CCD (globs, "cdc_csn1_sx_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;
  /*
   * If this element is conditional, check the condition.
   */
  if (calcidx[cix_ref].numCondCalcs NEQ 0
  AND ! ccd_conditionOK (e_ref, globs))
    return 1;

  /*
   * If this element has a defined prologue
   * we have to process it before decoding the bitstream.
   */
  if (num_prolog_steps)
  {
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }
  
  if (melem[e_ref].elemType NEQ 'S')
  {
    UBYTE act_continue_array;

    act_continue_array = globs->continue_array;
    globs->continue_array = TRUE;
    /*
     * Element is not a SPARE.
     * Setup the offset into the C-structure for this element
     */
    globs->pstructOffs = melem[e_ref].structOffs;

    if (melem[e_ref].optional)
    {  
      /*
       * For optional elements check the valid-flag in the C-struct.
       * Spare elements does not have a corresponding valid flag.
       */
      /* Dynamic array addition.
       * Postpone optional flag setting for non-code transparent
       * pointer types ('P', 'Q', 'R').
       * For these types, the optional flag is the pointer itself.
       * These types cannot be set yet, as the pointer may be
       * preceeded by a counter octet, a union tag id octet etc.
       */
      if (melem[e_ref].elemType < 'P' OR melem[e_ref].elemType > 'R')
      {
        if (globs->pstruct[globs->pstructOffs++] == FALSE)
        {
          /*
           * The IE should not be present in the message so we code
           * a single bit
           * for CSN1_S1 as 0,
           * for CSN1_S0 as 1 and
           * for CSN1_SH as GET_HL(0)
           */
          if (flag == 0xFF)
            elemPresent = GET_HL(0);
          else
            elemPresent = flag ^ 0x00000001;
          bf_writeBit (elemPresent, globs);
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

    /* As a default amount =1 due to initialization. */
    if (melem[e_ref].repType EQ 'i')
    {
      /* The actual number of elements belonging to the array is unknown.
       * The user should have written the desired number to the C-Structure
       * (c_xxx). CCD reads the number of these variable repeatable elements
       * out of this C-Structure (c_xxx) and encodes each element with a
       * preceeding bit set to '0'. The last element is followed by a bit
       * set to '1' to indicate the end of this array. 
       * If the number of repeats given by the C-Structure exceeds the 
       * allowed value (maxRepeat) CCD gives a warning!
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
        repeat = (ULONG) MINIMUM (globs->pstruct[globs->pstructOffs], 
                                  melem[e_ref].maxRepeat);
        if ( repeat < (ULONG) (globs->pstruct[globs->pstructOffs]) )
          ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                           (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
      }
      amount = 1;
      globs->pstructOffs++;
    }
    else
    if (melem[e_ref].repType EQ 'v')
    {
      /* The number of elements belonging to the array depends on the value 
       * of another element. The user should have written this number to the 
       * C-Structure (c_xxx).
       * CCD reads the number of these variable repeatable elements out of 
       * this C-Structure (c_xxx).
       * If the number of repetitions given by the C-Structure exceeds the
       * allowed value (maxRepeat) CCD gives a warning!
       */
      if (melem[e_ref].maxRepeat > 255)
      {
        ULONG count = (ULONG) (* (USHORT *)(globs->pstruct + globs->pstructOffs++));
        amount = MINIMUM (count, (ULONG) melem[e_ref].maxRepeat);
        if (amount < count) 
          ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                           (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
      }
      else
      {
        amount = (ULONG)MINIMUM (globs->pstruct[globs->pstructOffs], 
                                  melem[e_ref].maxRepeat);
        if ( amount < (ULONG) (globs->pstruct[globs->pstructOffs]) )
          ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                           (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
      }
      repeat = 1;
      globs->pstructOffs++;
    }
    else
    if (melem[e_ref].repType EQ 'c')
    {
      amount = (ULONG) melem[e_ref].maxRepeat;
      repeat = 1;
    }
    else
    if (melem[e_ref].repType == 's')
    {
      BOOL  is_variable;
      ULONG max_rep;
      
      is_variable = ccd_calculateRep (e_ref, &repeat, &max_rep, globs);
      /* Substract one bit which will be spent on the (CSN.1) flag. */
      amount = repeat - 1;
      repeat = 1;
    }
    if (melem[e_ref].repType EQ 'v' OR melem[e_ref].repType EQ 'i')
    {
      cSize = (USHORT)((melem[e_ref].elemType EQ 'V' OR
		      melem[e_ref].elemType EQ 'R' OR melem[e_ref].elemType EQ 'F')
                 ? mvar[melem[e_ref].elemRef].cSize
                 : mcomp[melem[e_ref].elemRef].cSize
              );
      
      startOffset = (USHORT) globs->pstructOffs;
    }
    
    /* Dynamic array addition.
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
      TRACE_CCD (globs, "cdc_csn1_sx_encode(): Pointer misaligned! pstruct=0x08x,"
		 " pstructOffs=0x%08x", globs->pstruct, globs->pstructOffs);
      globs->pstructOffs = (globs->pstructOffs + 3) & 3;
    }
#endif
#ifdef DYNAMIC_ARRAYS
    /*
     * Perform pointer dereference for pointer types.
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
      startOffset        = 0;
    }
#endif /* DYNAMIC_ARRAYS */
  
    for (i=0; i < repeat; i++)
    {
      /*
       * The IE should be present in the message so we code 0 bit.
       */
      if (flag == 0xFF)
        elemPresent = GET_HL(1);
      else
        elemPresent = flag;
        
      bf_writeBit (elemPresent, globs);
      if (cSize)
      {
        /*
         * Calculate the offset if it is an array.
         */
        globs->pstructOffs = (USHORT)(startOffset + (i * cSize));
      }
      /*
       * Encode the value.
       */
      cdc_encodeElemvalue (e_ref, amount, globs);
    }

#ifdef DYNAMIC_ARRAYS
  if ( old_pstruct NEQ NULL )
    globs->pstruct = old_pstruct;
#endif

    if ((melem[e_ref].repType == 'i') && (globs->continue_array == TRUE))
    {
      /*
       * For fields of variable length we code a 1 flag
       * to mark the end of the array.
       */
      if (flag == 0xFF)
        elemPresent = GET_HL(0);
      else
        elemPresent = flag ^ 0x00000001;
      bf_writeBit (elemPresent, globs);
    }
    globs->continue_array = act_continue_array;
  }

  return 1;
}
#endif /* !RUN_FLASH */
