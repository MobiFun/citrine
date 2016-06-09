/* 
+----------------------------------------------------------------------------- 
|  Project : CCD
|  Modul   : csn1_choice_x.c
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
|  Purpose :  Definition of encoding and decoding functions for CSN1_CHOICE2
|             elements
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
#include "ccd_codingtypes.h"
/*
 * Declaration of coder/decoder tables
 */
#include "ccdtable.h"
#include "ccddata.h"

EXTERN T_FUNC_POINTER codec[MAX_CODEC_ID+1][2];

#ifndef RUN_INT_RAM
/* Attention for RUN_...: static function */

static void decode_csn1_choice_alternative(const ULONG e_ref, T_ENUM union_tag, 
                                           T_CCD_Globs *globs);
static void encode_csn1_choice_alternative(const ULONG e_ref, T_ENUM union_tag, 
                                           T_CCD_Globs *globs);
#endif /* !RUN_INT_RAM */


#ifndef RUN_INT_RAM
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)  MODULE  : csn1_choice1                       |
| STATE   : code        ROUTINE : decode_csn1_choice_alternative     |
+--------------------------------------------------------------------+

  PURPOSE : Decode a chosen alternative of a CSN.1 CHOICE type
            Use the parameter union_tag to read the CHOICE index. 
            Because of union_tag it is easy to calculate the elemRef 
            for the chosen element which is to be decoded. Then the 
            appropriate decoding function for the chosen element is 
            called.
*/
static void decode_csn1_choice_alternative(const ULONG e_ref, T_ENUM union_tag, 
                                           T_CCD_Globs *globs)
{
  ULONG  elem_ref, mcomp_ref;
  UBYTE  *act_structpos;

  mcomp_ref= melem[e_ref].elemRef;
  /*
   * Write the CHOICE tag value in the C-structure.
   * Calculate the elem_ref for the chosen element.
   */
  *(T_ENUM *) (globs->pstruct+globs->pstructOffs) = union_tag;
  elem_ref =  mcomp[mcomp_ref].componentRef + (ULONG) union_tag;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "decode_csn1_choice_alternative()");
  #else
  TRACE_CCD (globs, "decode_csn1_choice_alternative() %s", ccddata_get_alias((USHORT)e_ref, 1));
  #endif
#endif

  /*
   * Store the actual structure position.
   */
  act_structpos = globs->pstruct;
  globs->pstruct += (globs->pstructOffs + sizeof(T_ENUM));
  /* 
   * Use the jump-table for selecting the decode function
   * Call the decode function for the chosen element.
   */
  (void) codec[melem[elem_ref].codingType][DECODE_FUN]
                                        (mcomp_ref, elem_ref, globs);
  /*
   * Restore the write pointer to prepare decoding of the next element
   */
  globs->pstruct = act_structpos;
}
#endif /* !RUN_INT_RAM */


#ifndef RUN_INT_RAM
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)  MODULE  : csn1_choice1                       |
| STATE   : code        ROUTINE : encode_csn1_choice_alternative     |
+--------------------------------------------------------------------+

  PURPOSE : Encode a chosen alternative of a CSN.1 CHOICE type.

            Because of union_tag it is easy to calculate the e_ref for 
            the chosen element to be encoded. Then the appropriate 
            encoding function for the chosen element is called.
*/
static void encode_csn1_choice_alternative (const ULONG e_ref, T_ENUM union_tag, 
                                            T_CCD_Globs *globs)
{
  ULONG  elem_ref, mcomp_ref;
  UBYTE  *act_structpos;

  mcomp_ref= melem[e_ref].elemRef;

  /*
   * Calculate the elem_ref for the chosen element.
   */
  elem_ref =  mcomp[mcomp_ref].componentRef + (ULONG) union_tag;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "encode_csn1_choice_alternative()");
  #else
  TRACE_CCD (globs, "encode_csn1_choice_alternative() %s", ccddata_get_alias((USHORT)e_ref, 1));
  #endif
#endif

  /*
   * Store the actual structure position.
   */
  act_structpos = globs->pstruct;
  globs->pstruct += (globs->pstructOffs + sizeof(T_ENUM));
  /* 
   * Use the jump-table for selecting the encode function
   * Call the encode function for the chosen element.
   */
  (void) codec[melem[elem_ref].codingType][ENCODE_FUN]
                                        (mcomp_ref, elem_ref, globs);
  /*
   * Restore the write pointer to prepare decoding of the next element
   */
  globs->pstruct = act_structpos;
}
#endif /* !RUN_INT_RAM */


#ifndef RUN_INT_RAM/*
+-----------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                      |
| STATE   : code                     ROUTINE : cdc_csn1_choice_x_decode |
+-----------------------------------------------------------------------+

  PURPOSE : Decode CSN.1 CHOICE type
            In the header file the structure containing an element of 
            coding type CSN1_CHOICE is represented by a structure
            type declaration.
            This structure type is composed at least of a control item 
            preceding an item of an union type. The control item 
            indicates the CHOICE index. The item of the union type
            represents the CHOICE alternatives.
            The CHOICE index is "num" bits long. It is read from the 
            message bit string and the result is written to the control
            item. CCD determes "elemRef" depending on this control item
            and processes the union element according to table entry 
            of "elemRef" .
*/

SHORT cdc_csn1_choice_x_decode (const ULONG c_ref, const ULONG e_ref,
                                ULONG num, T_CCD_Globs *globs)
{
  ULONG union_tag, num_of_alt, num_of_comps;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;
  
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_csn1_choice1_decode()");
  #else
  TRACE_CCD (globs, "cdc_csn1_choice1_decode() %s", ccddata_get_alias((USHORT)e_ref, 1));
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

  globs->pstructOffs = melem[e_ref].structOffs;

  if (melem[e_ref].optional)
  {
    /* Postpone optional flag setting for non-code transparent
     * pointer types ('P', 'Q', 'R').
     * For these types, the optional flag is the pointer itself.
     * These types cannot be set yet, as the pointer may be
     * preceeded by a counter octet, a union tag id octet etc.
     */
    if (melem[e_ref].elemType < 'P' OR melem[e_ref].elemType > 'R')
      globs->pstruct[globs->pstructOffs++] = (UBYTE) TRUE;
  }
  
  /*
   * Get the number of alternatives from the C-structure.
   */
  num_of_comps = mcomp[melem[e_ref].elemRef].numOfComponents;
  /* Determine number of possible alternatives */
  num_of_alt = num <<1;
  
  /* read the bit representing the CHOICE index*/
  union_tag = bf_getBits(num, globs);

  /* Check number of alternatives */ 
  if (!num_of_comps)
  {
      /* Don't do anything for empty choices */
      return 1;
  }
  else if (num_of_comps != num_of_alt)
  { 
    /* if the number of components doesn't match to number of possible 
     * examine whether the CHOICE index demands an impossible alternative
     */
    if (union_tag > num_of_comps)
    {
    /* if CHOICE index demands an impossible alternative
     * return an error and break decoding
     */
      ccd_recordFault (globs, ERR_CSN1_CHOICE, BREAK, (USHORT) e_ref, 
                       globs->pstruct+globs->pstructOffs);
    }
    else 
    {
      ccd_recordFault (globs, ERR_CSN1_CHOICE, CONTINUE, (USHORT) e_ref, 
                       globs->pstruct+globs->pstructOffs);
    }
  }
  
  /*
   * Decode a chosen alternative of an CSN.1 CHOICE type
   */
  decode_csn1_choice_alternative(e_ref, (T_ENUM) union_tag, globs);
  return 1;
}
#endif /* !RUN_INT_RAM */



#ifndef RUN_INT_RAM

/*
+----------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : csn1_choice1             |
| STATE   : code                    ROUTINE : cdc_csn1_choice_x_encode |
+----------------------------------------------------------------------+

  PURPOSE : Encoding of CHOICE1 type for UMTS
            In the header file the structure containing an element of 
            coding type CSN1_CHOICE is represented by a structure
            type declaration.
            This structure type is composed at least of a control item 
            preceding an item of an union type. The control item 
            indicates the CHOICE index. The item of the union type
            represents the CHOICE alternatives.
            The CHOICE index is "num" bit long. Tts value is read  
            from the message structureand the result is written to the 
            bit string. CCD determines "elemRef" depending on this index
            and processes the union element according to table entry 
            of "elemRef" .
*/
SHORT cdc_csn1_choice_x_encode (const ULONG c_ref, const ULONG e_ref, 
                                ULONG num, T_CCD_Globs *globs)
{
  ULONG  union_tag, num_of_alt, num_of_comps; 
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;
  
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_csn1_choice_encode()");
  #else
  TRACE_CCD (globs, "cdc_csn1_choice_encode() %s", 
             mcomp[melem[e_ref].elemRef].name);
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
  
  /*
   * Setup the offset into the C-structure for this element
   */
  globs->pstructOffs = melem[e_ref].structOffs;

  if (melem[e_ref].optional)
  {  
    /*
     * For optional elements check the valid-flag in the C-struct.
     * Postpone optional flag setting for non-code transparent
     * pointer types ('P', 'Q', 'R').
     * For these types, the optional flag is the pointer itself.
     * These types cannot be set yet, as the pointer may be
     * preceeded by a counter octet, a union tag id octet etc.
     */
    if (melem[e_ref].elemType < 'P' OR melem[e_ref].elemType > 'R')
    {
      if (globs->pstruct[globs->pstructOffs++] EQ FALSE)
        return 1;
    }
  }
  /*
   * Get the number of alternatives from the C-structure.
   */
  num_of_comps = mcomp[melem[e_ref].elemRef].numOfComponents;
  /* Determine number of possible alternatives */
  num_of_alt = num <<1;
  /*
   * Get the value of CHOICE index (= union controller) from the C-structure.
   * Check its correctness. Write it in the air message.
   * Afterwards encode the chosen CHOICE altervative.
   */
  union_tag = (ULONG) globs->pstruct[globs->pstructOffs];
  /*
   * Check its correctness.  
   */
  if (union_tag >= num_of_alt)
  {
    /* 
     * CHOICE index goes beyond the number of alternatives determined by 
     * selected coding type => return Error and break encoding process
     */
    ccd_recordFault (globs, ERR_CSN1_CHOICE, BREAK, 
                     (USHORT) e_ref, globs->pstruct+globs->pstructOffs);
  }

  /* 
   * Write the CHOICE tag value in the C-structure.
   */
  bf_writeVal (union_tag, num, globs);
  
  /* Check number of alternatives */ 
  if (!num_of_comps)
  {
    /* Don't do anything for empty choices */
    return 1;
  }
  else if (num_of_comps != num_of_alt)
  { 
    /* if the number of components doesn't match to number of possible
       alternatives return a warning */
    ccd_recordFault (globs, ERR_CSN1_CHOICE, CONTINUE, 
                     (USHORT) e_ref, globs->pstruct+globs->pstructOffs);
    if (union_tag > num_of_comps)
    {
    /* if the CHOICE index demands an impossible one handle this choice
       as no_code element */
    #ifdef DEBUG_CCD
      #ifndef CCD_SYMBOLS
      TRACE_CCD (globs, "cdc_No_encode()");
      #else
      TRACE_CCD (globs, "cdc_No_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
      #endif
    #endif
      return 1;
    }
  }

  /*
   * Encode a chosen alternative of an CSN.1 CHOICE type
   */
  encode_csn1_choice_alternative (e_ref, (T_ENUM) union_tag, globs);

  return 1;
}
#endif /* !RUN_INT_RAM */


