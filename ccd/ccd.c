/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccd.c
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
|  Purpose :  Condat Conder Decoder - 
|             Definition of encoding and decoding functions of 
|             air interface messages
+----------------------------------------------------------------------------- 
*/ 

#define CCD_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>

#ifdef _MSDOS
  #include <dos.h>
  #include <conio.h>
#endif

/*
 * Standard definitions like UCHAR, ERROR etc.
 */

#include "typedefs.h"
#include "header.h"

/*
 * Types and constants used by CCD
 */
#include "ccd_globs.h"

/*
 * Type definitions for CCD data tables
 */
#include "ccdtable.h"

/*
 * Function prototypes of CCD-CCDDATA interface 
 */
#include "ccddata.h"

/*
 * Error codes and prototypes of exported functions by CCD 
 * (USE_DRIVER EQ undef)
 * For prototypes only look at ccdapi.h.
 */
#undef USE_DRIVER
#include "ccdapi.h"

/*
 * Types and functions for bit access and manipulation
 */
#include "bitfun.h"


/*
 * Prototypes of ccd internal functions
 */
#include "ccd.h"

#if !(defined (CCD_TEST))
#include "vsi.h"
#include "os.h"
#endif

#ifdef SHARED_VSI
  #define VSI_CALLER 0,
#else
  #define VSI_CALLER
#endif

#ifndef RUN_FLASH
/* task_null is used in ccd_signup. It must have the same RUN_... setting. */
static T_CCD_TASK_TABLE task_null;
#endif /* !RUN_FLASH */

//TISH modified for MSIM
#if defined (CCD_TEST) || defined (_TOOLS_) || defined (WIN32)
/* For the DLL and for all simple environment define the task list locally */
/* 10 entries should be more than enough */
#define MAX_ENTITIES 10
T_CCD_TASK_TABLE* ccd_task_list[MAX_ENTITIES];
#else
extern T_CCD_TASK_TABLE* ccd_task_list[];
#endif

#ifndef RUN_FLASH
const T_CCD_VarTabEntry*   mvar;
const T_CCD_SpareTabEntry* spare;
const T_CCD_CalcTabEntry*  calc;
const T_CCD_CompTabEntry*  mcomp;
const T_CCD_ElemTabEntry*  melem;
const T_CCD_CalcIndex*     calcidx;
const T_CCD_ValTabEntry*   mval;
#else
extern const T_CCD_VarTabEntry*   mvar;
extern const T_CCD_SpareTabEntry* spare;
extern const T_CCD_CalcTabEntry*  calc;
extern const T_CCD_CompTabEntry*  mcomp;
extern const T_CCD_ElemTabEntry*  melem;
extern const T_CCD_CalcIndex*     calcidx;
extern const T_CCD_ValTabEntry*   mval;
#endif

#ifndef RUN_FLASH
/*
 * Attention: if one of the following static data shall be used in
 * both, internal as well as external RAM, they must be made global.
 */
static USHORT max_message_id;

/*
 * CCD internal buffer for decoded message
 */
static UBYTE *ccd_decMsgBuffer; 
/*
 * Layer-specific cache for the bitlength of the message-identifier;
 * A value of 0 indicates an empty (undetermined) entry
 */
static UBYTE *mi_length; 

/*
 * CCD internal variables used in each call to code or decode message
 */

static U8 aim_rrc_rcm;
static U8 aim_rrlp;
static U8 aim_sat;

static T_CCD_Globs globs_all;
#endif /* !RUN_FLASH */

/*
 * CCD will call its encoding/decoding functions through a jump table. 
 */
#include "ccd_codingtypes.h"
#ifndef RUN_FLASH
T_FUNC_POINTER  codec[MAX_CODEC_ID+1][2];
#else
extern T_FUNC_POINTER  codec[MAX_CODEC_ID+1][2];
#endif

#ifndef RUN_FLASH
/* initialized is used in ccd_signup. It must have the same RUN_... setting. */
/*
 * Initialising flag
 */
BOOL initialized = FALSE;
#endif /* !RUN_FLASH */

#ifdef SHARED_CCD
  /*
   * If CCD is used in a premptive multithreaded system we need
   * a semaphore to protect the coding and decoding sections.
   */
  #ifndef RUN_FLASH
    T_HANDLE semCCD_Codec, semCCD_Buffer;
  #else
    extern T_HANDLE semCCD_Codec, semCCD_Buffer;
  #endif /* RUN_FLASH */
#endif /* SHARED_CCD */

static U8* mempat;
#define LOWSEGMASK 0xFFFFF000

#ifndef RUN_FLASH
#ifdef DEBUG_CCD
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : TRACE_CCD           |
+--------------------------------------------------------------------+

  PURPOSE : Error processing of the CCD.

*/

void TRACE_CCD (T_CCD_Globs *globs, char *format, ...)
{
  va_list varpars;
  char trace_buf[256];
  int i=0;

  if (!globs->TraceIt)
    return;

  va_start (varpars, format); /* Initialize variable arguments. */
#if defined CCD_TEST
  /*
   * use vsi_o_trace - prefix the tracestring with [CCD]
   * so the PCO can display it in a CCD window
   */
  strcpy (trace_buf, "~CCD~");
  i = 5; 
#endif /* CCD_TEST */
  vsprintf (trace_buf+i, format, varpars);
  va_end (varpars);              /* Reset variable arguments.   */

#ifdef CCD_TEST
  printf ("\n%s\n", trace_buf);
#else
  vsi_o_ttrace (globs->me, TC_CCD, trace_buf);
#endif /* CCD_TEST */

}
#endif /* !RUN_FLASH */

#endif

/*
 * Stack operations
 */
#define ST_CLEAR(globs) globs->SP=0;globs->StackOvfl=FALSE;

#define ST_OK(globs) (globs->StackOvfl EQ FALSE)

#define ST_CHECK(A, globs) {if (!(globs->StackOvfl) AND globs->SP < MAX_UPN_STACK_SIZE)\
                    {(A);}\
                    else\
                    {globs->StackOvfl = TRUE;}}

#define ST_PUSH(A, globs) ST_CHECK ((globs->Stack[globs->SP++] = (A)), globs)

#define ST_POP(A, globs)  ST_CHECK ((A = globs->Stack[--(globs->SP)]), globs)

#define ST_TOP(A, globs)  ST_CHECK ((A = globs->Stack[globs->SP-1]), globs)

#ifndef RUN_FLASH
/*
 * Attention: if the static function calcUPN shall be used in
 * both, internal as well as external RAM, it must be made global.
 */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : calcUPN             |
+--------------------------------------------------------------------+

  PURPOSE : calculates the UPN-term for an element.

*/
LOCAL BOOL calcUPN  (ULONG        op,
                     ULONG        num_ops,
                     ULONG       *result,
                     T_CCD_Globs *globs)
{
  BOOL   opError;
  ULONG  op1=0,op2=0;

  opError = FALSE;
  
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "Calculation of UPN-term ");
#endif

  while (num_ops-- AND !opError AND ST_OK(globs))
  {
    switch (calc[op].operation)
    {
      case '+':
        /*
         * get the upper two elements from the stack, add them
         * and push the result on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 + op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '+' causes invalid calculation");
#endif
        break;
      case '-':
        /*
         * get the upper two elements from the stack, subtract them
         * and push the result on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 - op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '-' causes invalid calculation");
#endif
        break;
      case '*':
        /*
         * get the upper two elements from the stack, multiply them
         * and push the result on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 * op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '*' causes invalid calculation");
#endif
        break;
      case '/':
        /*
         * get the upper two elements from the stack, divide them
         * and push the result on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          if (!op2)
          {
            ccd_setError (globs, ERR_DEFECT_CCDDATA, BREAK, 
                        (USHORT) (globs->bitpos), (USHORT) -1);
          }
          else
          {
            ST_POP(op1, globs);
            ST_PUSH ((op1 / op2), globs);
          }
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '/' causes invalid calculation");
#endif
        break;
      case '&':
        /*
         * get the upper two elements from the stack, perform a
         * binary AND
         * and push the result on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 & op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '&' causes invalid calculation");
#endif
        break;
      case '|':
        /*
         * get the upper two elements from the stack, perform a
         * binary OR
         * and push the result on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 | op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '|' causes invalid calculation");
#endif
        break;
      case 'A':
        /*
         * get the upper two elements from the stack, perform a
         * logical AND
         * and push a TRUE or FALSE on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 AND op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter 'AND' causes invalid calculation");
#endif
        break;
      case 'O':
        /*
         * get the upper two elements from the stack, perform a
         * logical OR
         * and push a TRUE or FALSE on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 OR op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter 'OR' causes invalid calculation");
#endif
        break;
      case 'X':
        /*
         * get the upper two elements from the stack, perform a
         * logical XOR
         * and push a TRUE or FALSE on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ( ((op1 AND !op2) OR (!op1 AND op2)) , globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter 'XOR' causes invalid calculation");
#endif
        break;
      case '=':
        /*
         * get the upper two elements from the stack, look if they
         * are equal
         * and push a TRUE or FALSE on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 EQ op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '=' causes invalid calculation");
#endif
        break;
      case '#':
        /*
         * get the upper two elements from the stack, look if they
         * are different
         * and push a TRUE or FALSE on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 NEQ op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '#' causes invalid calculation");
#endif
        break;
      case '>':
        /*
         * get the upper two elements from the stack, look if
         * op1 > op2
         * and push a TRUE or FALSE on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 > op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '>' causes invalid calculation");
#endif
        break;
      case '<':
        /*
         * get the upper two elements from the stack, look if
         * op1 < op2
         * and push a TRUE or FALSE on the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op2, globs);
          ST_POP(op1, globs);
          ST_PUSH ((op1 < op2), globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '<' causes invalid calculation");
#endif
        break;
      case 'P':
        /*
         * push a constant on the stack
         */
        ST_PUSH (calc[op].operand, globs);
#ifdef DEBUG_CCD
        if (globs->StackOvfl == TRUE)
          TRACE_CCD (globs, "Constant can't be pused on UPN stack");
#endif
        break;
      case ':':
        /*
         * duplicate the upper element on the stack
         */
        if (globs->SP >= 1)
        {
          ST_TOP (op1, globs);
          ST_PUSH (op1, globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "No UPN stack element to duplicate");
#endif
        break;
      case 'R':
        /*
         * push the content of a C-structure variable on the stack
         */
      {
        /* if no register available some compilers may react with 
           a warning like this: identifier p not bound to register
        */
        UBYTE *p;
        ULONG           value;

        /*
         * normaly we have to check if the element is a VAR
         * and not an array/substructure.
         * - but this must be done by ccdgen
         */

        /*
         * setup the read pointer to the element in the C-structure
         */
        p = globs->pstruct + melem[(USHORT) calc[op].operand].structOffs;

        /*
         * Get the element table entry from the element to read.
         * if the element of the condition is conditional too
         * then look for the valid flag of this element.
         * if the element is not valid,
         * we dont need to check the contents of it
         */
        if (melem[(USHORT) calc[op].operand].optional)
        {
          if (*(UBYTE *) p EQ FALSE)
          {
            ST_PUSH (0L, globs);
            break;
          }
          else
            p++;
        }

        /*
         * read the contents of the element
         */
        switch (mvar[melem[(USHORT) calc[op].operand].elemRef].cType)
        {
          case 'B':
            value = (ULONG) * p;
            break;
          case 'S':
            value = (ULONG) * (USHORT *) p;
            break;
          case 'L':
            value = *(ULONG *) p;
            break;
          default:
            value = 0L;
        }
        ST_PUSH (value, globs);
        break;
      }
      case 'S':
        /*
         * get the upper element from the stack an
         * set the position of the bitstream pointer to this
         * value
         */
        if (globs->SP >= 1)
        {
          ST_POP(op1, globs);
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "SETBITPOS %d (byte %d.%d)",
                     (USHORT) op1, (USHORT) (op1 / 8), (USHORT) (op1 % 8));
#endif
          bf_setBitpos ((USHORT) op1, globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter 'SETBITPOS' causes invalid calculation");
#endif
        break;
      case 'G':
        /*
         * push the position of the bitstream pointer on the
         * stack
         */
        ST_PUSH ((ULONG) globs->bitpos, globs);
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "GETBITPOS %d (byte %d.%d)", 
                   (USHORT) globs->bitpos, globs->bytepos, globs->byteoffs);
#endif
        break;
      case '^':
        /*
         * swap the upper two elements of the stack
         */
        if (globs->SP >= 2)
        {
          ST_POP(op1, globs);
          ST_POP(op2, globs);
          ST_PUSH(op1, globs);
          ST_PUSH(op2, globs);
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter '^' causes invalid calculation");
#endif
        break;
      case 'K':
        /*
         * Keep a value in the KEEP register.
         */
        if (globs->SP >= 1)
        {
          ST_POP(op1, globs);
          globs->KeepReg[calc[op].operand] = op1;
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter 'KEEP' causes invalid calculation");
#endif
        break;
      case 'L':
        /*
         * Copy the L part of a TLV element from the KEEP register to the UPN stack.
         */          
        ST_PUSH(globs->KeepReg[0]*8, globs);
#ifdef DEBUG_CCD
        if (globs->StackOvfl == TRUE)
          TRACE_CCD (globs, "Control parameter 'LTAKE' causes invalid calculation");
#endif
        break;
      case 'T':
        /*
         * Take a value from the KEEP register and push it on the UPN stack.
         */          
        ST_PUSH(globs->KeepReg[calc[op].operand], globs);
#ifdef DEBUG_CCD
        if (globs->StackOvfl == TRUE)
          TRACE_CCD (globs, "Control parameter 'TAKE' causes invalid calculation");
#endif
        break;
      case 'C':
        /*
         * Compare the value on the UPN stack with the one stored in the KEEP register.
         * Push the higher value in the KEEP register.
         */
        if (globs->SP >= 1)
        {          
          ST_POP(op1, globs);
          if ((globs->KeepReg[calc[op].operand]) < op1)
          {
            globs->KeepReg[calc[op].operand] = op1;
          }
        }
#ifdef DEBUG_CCD
        else
          TRACE_CCD (globs, "Control parameter 'MAX' causes invalid calculation");
#endif
        break;
      case 'Z':
        /*
         *  Used to mark presence of an address information part error label
         */          
        globs->errLabel = ERR_ADDR_INFO_PART;
          break;
      case 'D':
        /*
         *  Used to mark presence of a distribution part error label
         */          
        globs->errLabel = ERR_DISTRIB_PART;
          break;
      case 'N':
        /*
         *  Used to mark presence of a non distribution part error label
         */          
        globs->errLabel = ERR_NON_DISTRIB_PART;
          break;
      case 'M':
        /*
         *  Used to mark presence of a message escape error label
         */          
        globs->errLabel = ERR_MESSAGE_ESCAPE;
          break;
      case 'I':
        /*
         *  Used to mark presence of an ignore error label
         */          
        globs->errLabel = ERR_IGNORE;
          break;
      case 'l':
        /*
         * Take a value from the CCD STO register and push it on the UPN stack.
         */
        opError = ccd_getStore (globs, calc[op].operand, &op1);
        if (!opError)
        {
          ST_PUSH(op1, globs);
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "Push CCD STORE register [%d] value to UPN stack",
                     calc[op].operand);
        }
        else
        {
          TRACE_CCD (globs, "Reading from CCD STORE register [%d] impossible",
                     calc[op].operand);
#endif
        }
        break;
      case 's':
        /*
         * Store a value in the CCD STO register.
         */
        if (globs->SP >= 1)
        {
          ST_POP(op1, globs);
          opError =  ccd_writeStore (globs, calc[op].operand, op1);
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "Store value in CCD STO register [%d]",
                     calc[op].operand);
        }
        else
        {
          TRACE_CCD (globs, "Control parameter 'STORE' causes invalid calculation");
#endif
        }
        break;
 
      default:
        opError = TRUE;
        break;
    }
    op++;
  }

  if (!opError AND ST_OK(globs))
  {
    if (result NEQ (ULONG *) NULL)
      ST_POP (*result, globs);
    return (ST_OK(globs));
  }
  else
  {
#ifdef DEBUG_CCD
    if(opError)
      TRACE_CCD (globs, "Calculation of UPN-term failed ");
    else
      TRACE_CCD (globs, "Calculation of UPN-term failed due to stack overflow");
#endif
    return FALSE;
  }
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : ccd_conditionOK     |
+--------------------------------------------------------------------+

  PURPOSE : Check if the conditions for an element are true.
            The elemRef references an element entry from the elem tab.
            The function returns TRUE if the defined
            conditions are TRUE.

*/

BOOL ccd_conditionOK (const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG result, cond_calc_ref, num_cond_calcs, cix_ref;

  cix_ref = melem[e_ref].calcIdxRef;
  num_cond_calcs = calcidx[cix_ref].numCondCalcs;
  cond_calc_ref  = calcidx[cix_ref].condCalcRef;


  if (! calcUPN (cond_calc_ref,
                 num_cond_calcs,
                 &result,
                 globs))
    return FALSE;

  return (result EQ TRUE);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : ccd_calculateRep    |
+--------------------------------------------------------------------+

  PURPOSE : For a given element (referenced by elemRef) calculate the
            repeat value.

*/

BOOL ccd_calculateRep  (const ULONG  e_ref,
                              ULONG *repeat,
                              ULONG *max_repeat,
                              T_CCD_Globs *globs)
{
  ULONG cix_ref, result;
  ULONG remaining_repeats=0;
  ULONG rep_calc_ref, num_rep_calcs;

  BOOL   is_variable;

  cix_ref = melem[e_ref].calcIdxRef;
  num_rep_calcs = calcidx[cix_ref].numRepCalcs;
  rep_calc_ref  = calcidx[cix_ref].repCalcRef;

  *max_repeat  = (ULONG) melem[e_ref].maxRepeat;

  if (num_rep_calcs EQ 0)
  {
    if (melem[e_ref].repType EQ 'i')
    {
      switch (melem[e_ref].elemType)
      {
        case 'S':
          remaining_repeats = (ULONG)((globs->maxBitpos-globs->bitpos)
                           / spare[melem[e_ref].elemRef].bSize);
          break;
        case 'F': /* Code transparent pointer to base type */
        case 'R': /* Pointer to base type */
        case 'V':
          remaining_repeats = (ULONG)((globs->maxBitpos-globs->bitpos)
                           / mvar[melem[e_ref].elemRef].bSize);
          break;
        case 'D': /* Code transparent pointer to composition */
        case 'P': /* Pointer to composition */
        case 'C':
          /* for repeated compositions the remaining repeats
             are set to the max repeats because of the unknown
             size of one composition */
          remaining_repeats = (ULONG) melem[e_ref].maxRepeat;
          break;
        default:
          ccd_setError (globs, ERR_INVALID_CALC, BREAK, (USHORT) -1);
          break;
      }

      *repeat   = MINIMUM (*max_repeat, remaining_repeats);
      is_variable = TRUE;
    }
    else
    {
      if (melem[e_ref].repType EQ 'b')
      {
        remaining_repeats = (ULONG)(globs->maxBitpos-globs->bitpos);
        *repeat   = MINIMUM (*max_repeat, remaining_repeats);
      }
      else
      {
        *repeat     =  MINIMUM (*max_repeat,
                                (ULONG) calcidx[cix_ref].repCalcRef);
        if (*repeat < (ULONG) calcidx[cix_ref].repCalcRef) 
          ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                           (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
      }
      is_variable = FALSE;
    }
  }
  else
  {
    is_variable = FALSE;
    if (! calcUPN (rep_calc_ref,
                   num_rep_calcs,
                   &result,
                   globs))
    {
      *repeat = *max_repeat = 0;

      ccd_setError (globs, ERR_INVALID_CALC, BREAK, (USHORT) -1);
    }
    else
    {
      if ((melem[e_ref].repType != 'b') && (melem[e_ref].repType != 's'))
      {
        is_variable = TRUE;
      }
      if (melem[e_ref].repType EQ 'i')
      {
        switch (melem[e_ref].elemType)
        {
          case 'S':
            remaining_repeats = (ULONG)((globs->maxBitpos-globs->bitpos)
                             / spare[melem[e_ref].elemRef].bSize);
            break;
          case 'F': /* Code transparent pointer to base type */
          case 'R': /* Pointer to base type */
          case 'V':
            remaining_repeats = (ULONG)((globs->maxBitpos-globs->bitpos)
                             / mvar[melem[e_ref].elemRef].bSize);
            break;
          default:
            ccd_setError (globs, ERR_INVALID_CALC, BREAK, (USHORT) -1);
            break;
        }
        *repeat = MINIMUM (result, remaining_repeats);
      }
      else
      {
        *repeat = MINIMUM (result, *max_repeat);
        if (*repeat < result) 
          ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                           (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
      }
    }
  }
  return (is_variable);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_performOperations |
+--------------------------------------------------------------------+

  PURPOSE : Perform the operation for an element. This operations
            are executed before an element is encoded or decoded.
            The Operations work with the UPN-Stack.

*/

void ccd_performOperations (ULONG        num_of_ops,
                            ULONG        op_def_ref,
                            T_CCD_Globs *globs)
{
  if (! calcUPN (op_def_ref,
                 num_of_ops,
                 NULL,
                 globs))
    ccd_setError (globs, ERR_INVALID_CALC, BREAK, (USHORT) -1);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
 * Attention: if the static function ccd_isOptional shall be used in
 * both, internal as well as external RAM, it must be made global.
 */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_isOptional        |
+--------------------------------------------------------------------+

  PURPOSE : Checks if a given element is optional. If the element is
            not optional and is a composition,
            a recursive call is performed in order to check
            if the composition contains only optional elements.
            In this case the whole composition is optional.

            In case of components concatenated with a CSN1 coding 
            rules the meaning of the word <optional> differs from the 
            traditional TI tool chain conventions. So far some coding 
            types (like tagged types, e.g. GSM3_TV) characterise 
            optional elements inherently. The value of their valid 
            flag indicates the presence or absence of such an element.
            Components concatenated with a CSN1 coding type cause 
            these valid flags in the C header structure too. If you 
            find a bit in the received message stream indicating 
            optional values not included in the message (e. g. a 
            CSN1_S1 element is represented by ??, CCD will set the 
            valid flag to zero. But the whole element represented by 
            the flag is present; only the value is absent! Therefor 
            the valid flag in the C structure is not an indication of 
            an element’s absence.
*/

LOCAL BOOL ccd_isOptional (ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG  cix_ref;
  
  switch (melem[e_ref].codingType)
  {
  case CCDTYPE_CSN1_S1: /* Fall through. */
  case CCDTYPE_CSN1_S0:
  case CCDTYPE_CSN1_SHL:
  
    cix_ref = melem[e_ref].calcIdxRef;
    /*
     * If this element is conditional, check the condition.
     */
    if (calcidx[cix_ref].numCondCalcs NEQ 0
    AND ! ccd_conditionOK (e_ref, globs))
      return TRUE;
      
    if (melem[e_ref].repType == 'i' AND 
        calcidx[melem[e_ref].calcIdxRef].repCalcRef == 0)
      return TRUE;
    else 
      return FALSE;
//    break;
//    PATCH FROM M18
   case CCDTYPE_CSN1_CONCAT:
      return TRUE;

  default:
    break;
  }
  
  if (! melem[e_ref].optional)
  {
    /*
     * if the element is an array with an interval [0..x]
     * it can be handled like an optional element
     */
    if ( ! (melem[e_ref].repType EQ 'i'
        AND calcidx[melem[e_ref].calcIdxRef].repCalcRef EQ 0))
    {
      /*
       * if the element is not optional but it is a composition
       * we check recursive if the composition consists only of
       * optional elements
       */
      if (melem[e_ref].elemType EQ 'C' OR
	      melem[e_ref].elemType EQ 'D' OR
	      melem[e_ref].elemType EQ 'P')
      {
        ULONG el, lel, c_ref;
        
        c_ref = melem[e_ref].elemRef;

        el = (ULONG) mcomp[c_ref].componentRef;
        lel = el + mcomp[c_ref].numOfComponents;
   
        while (el < lel)
        {
          if (! ccd_isOptional (el, globs))
            return FALSE;
          el++;
        }
      }
      else
        return FALSE;
    }
  }
  return TRUE; 
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_decodeComposition |
+--------------------------------------------------------------------+

  PURPOSE : decodes the bitstream to a C-Structure.The decoding
            rules contains the element definitions for the
            elements of this message.
            This function may called recursivly because of a
            substructured element definition.
*/

void ccd_decodeComposition (const ULONG c_ref, T_CCD_Globs *globs)
{
  /*
   * index in table melem
   */
  ULONG  e_ref;       /* element reference */
  ULONG  l_ref;       /* reference to the last element of a component */
  SHORT  codecRet;
  BOOL   ExtendedGroupActive = FALSE;
  BOOL   GroupExtended       = FALSE;
  BOOL   MoreData;
  BOOL   SetPosExpected = FALSE;
  int    i;
  ULONG  act_err_label;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "ccd_decodeComposition()");
  #else
  TRACE_CCD (globs, "ccd_decodeComposition(): Composition = %s",
               mcomp[c_ref].name);
  #endif
#endif

  act_err_label = (ULONG) globs->errLabel;
  globs->ccd_recurs_level++;

  /*
   * setup the index in the melem table for this composition.
   * If this function is called for the first time
   * (ccd_recurs_level == 1) the elem-table entry for the msg_type
   * was skipped.
   */

  l_ref = mcomp[c_ref].componentRef
           + mcomp[c_ref].numOfComponents;

  e_ref  = mcomp[c_ref].componentRef
           + ((globs->ccd_recurs_level EQ 1) ? 1 : 0);

  /*
   * decode all elements
   */
  while (e_ref < l_ref)
  {
#ifdef ERR_TRC_STK_CCD
    /* save the value for tracing in error case */
    globs->error_stack[globs->ccd_recurs_level] = (USHORT) e_ref;
#endif /* ERR_TRC_STK_CCD */

    if (melem[e_ref].extGroup != ' ' && GroupExtended && !ExtendedGroupActive)
    {
      /*
       * the last read extension bit signals an
       * extension of the group but there are no
       * more elements defined for this group.
       * This indicates a protocol extension, that must be
       * skipped by ccd.
       */

      do
      {
        /*
         * read the ext-bit to determine the extension
         * of this group
         */
        GroupExtended = (bf_readBit (globs) EQ 0);
        /*
         * skip to next octett
         */
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "skipping 7 bits");
#endif
        bf_incBitpos (7, globs);
      } while (GroupExtended);
    }

    /*
     * check if the bitstream has ended
     */
    if (bf_endOfBitstream(globs) AND !globs->TagPending)
    {
        ULONG  cix_ref, num_prolog_steps;
        
        cix_ref = melem[e_ref].calcIdxRef;
        num_prolog_steps = calcidx[cix_ref].numPrologSteps;
        
      /* End of the bit stream is not reached if a call to bf_setBitpos()
       * is expected for the next element of the current substructure. 
       * An instructive example is an empty "mob_id"
       */
      if (num_prolog_steps)
      {
        ULONG prolog_step_ref = calcidx[cix_ref].prologStepRef;
        
        i = (int) (prolog_step_ref + num_prolog_steps);
        
        while (i >= (int) prolog_step_ref)
        {
          if (calc[i].operation == 'S')
          {
            SetPosExpected = TRUE;
            break;
          }
          i--;
        }
      }

     if (SetPosExpected EQ FALSE)
     {
       /*
        * no more bits to decode.
        * If at least one mandatory element is to decode
        * generate an error.
        */

        while (e_ref < l_ref)
        {
          if (! ccd_isOptional (e_ref, globs))
            ccd_setError (globs, ERR_MAND_ELEM_MISS, BREAK, (USHORT) -1);

          e_ref++;
        }
        /* after the while loop the recursion level will be decremented. */
        break;
      }
    }

    /*
     * look for extension group processing
     */
    if (melem[e_ref].extGroup NEQ ' ')
    {
      /*
       * extended group symbol found
       */
      switch (melem[e_ref].extGroup)
      {
        case '+':
          /*
           * start of an extended group
           */
          ExtendedGroupActive = TRUE;
          /*
           * read the extension bit to determine the extension
           * of this group
           */
          GroupExtended = (bf_readBit (globs) EQ 0);
          /*
           * use the jump-table for selecting the decode function
           */
          codecRet =
            codec[melem[e_ref].codingType][DECODE_FUN](c_ref,
                                                         (USHORT)e_ref, globs);
          if (codecRet NEQ 0x7f)
          {
            /*
             * set the e_ref to the next or the same element
             */
            e_ref += codecRet;
          }
          break;

        case '-':
          /*
           * end of one extension,
           * decode first and the read the extension bit.
           */
          /*
           * use the jump-table for selecting the decode function
           */
          codecRet =
            codec[melem[e_ref].codingType][DECODE_FUN](c_ref,
                                                         (USHORT)e_ref, globs);
          if (codecRet NEQ 0x7f)
          {
            /*
             * set the e_ref to the next or the same element
             */
            e_ref += codecRet;
          }

          /*
           * look if the previously readed extension bit
           * alows an extension of this group
           */
          if (!GroupExtended)
          {

            ExtendedGroupActive = FALSE;
            /*
             * overread the following elements in the group
             */
            /*
             * search the last element of the extended group
             */
            while (e_ref < l_ref
            AND melem[e_ref].extGroup NEQ '*')
            {
#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
              if (melem[e_ref].elemType EQ 'V')
                TRACE_CCD (globs, "Skipping ext-group element %s",
                           ccddata_get_alias((USHORT) e_ref, 1));
              else if (melem[e_ref].elemType EQ 'C')
                TRACE_CCD (globs, "Skipping ext-group element %s",
                           mcomp[melem[e_ref].elemRef].name);
              else
                TRACE_CCD (globs, "Skipping ext-group spare");
#else
              TRACE_CCD (globs, "Skipping ext-group element %c-%d",
                         melem[e_ref].elemType,
                         e_ref);

#endif
#endif
              e_ref++;
            }
            /*
             * skip the last element
             */
            e_ref++;
          }
          else
          {
            /*
             * read the extension bit to determine if the group
             * extension holds on
             */
            GroupExtended = (bf_readBit (globs) EQ 0);
          }
          break;

        case '*':
          if (!ExtendedGroupActive)
          {
            /*
             * this is a single element extended group
             * often used for later extension of the protocol
             */
            /*
             * read the extension bit to determine if a group
             * extension is present
             */
            GroupExtended = (bf_readBit (globs) EQ 0);
          }
          ExtendedGroupActive = FALSE;

          /*
           * use the jump-table for selecting the decode function
           */
          codecRet =
            codec[melem[e_ref].codingType][DECODE_FUN](c_ref, e_ref, globs);
          if (codecRet NEQ 0x7f)
          {
            /*
             * set the e_ref to the next or the same element
             */
            e_ref += codecRet;
          }
          break;

        case '!': /* Moredata bit */
        case '#':
          if ((MoreData = bf_readBit (globs)) EQ 1)
          {
            /*
             * moredata-bit is set to 1
             * process this element.
             */

            /*
             * use the jump-table for selecting the decode function
             */
            codecRet =
              codec[melem[e_ref].codingType][DECODE_FUN](c_ref, e_ref, globs);

            if (melem[e_ref].extGroup EQ '#')
            {
              /*
               * if more data are signaled with an additional
               * bit but there are no
               * more elements defined for this group.
               * This indicates a protocol extension, that must be
               * skipped by ccd.
               */

              do
              {
                /*
                 * read the ext-bit to determine the extension
                 * of this group
                 */
                GroupExtended = (bf_readBit (globs) EQ 1);
                /*
                 * skip to next octett
                 */
          #ifdef DEBUG_CCD
                TRACE_CCD (globs, "skipping 7 bits");
          #endif
                bf_incBitpos (7, globs);
              } while (!bf_endOfBitstream(globs) AND GroupExtended);

            }

            if (codecRet NEQ 0x7f)
            {
              /*
               * set the elemRef to the next or the same element(USHORT)
               */
              e_ref += codecRet;
            }
          }

          if (!MoreData)
          {
            /*
             * more data bit is cleared,
             * overread the following elements in the group
             * search the last element of the extended group
             */
            while (e_ref < l_ref
            AND melem[e_ref].extGroup NEQ '#')
            {
#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
              if (melem[e_ref].elemType EQ 'V')
                TRACE_CCD (globs, "Skipping ext-group element %s",
                           ccddata_get_alias((USHORT) e_ref, 1));
              else if (melem[e_ref].elemType EQ 'C')
                TRACE_CCD (globs, "Skipping ext-group element %s",
                           mcomp[melem[e_ref].elemRef].name);
              else
                TRACE_CCD (globs, "Skipping ext-group spare");
#else
              TRACE_CCD (globs, "Skipping ext-group element %c-%d",
                         melem[e_ref].elemType,
                         e_ref);

#endif
#endif
              e_ref++;
            }

            /*
             * skip the last element of the moredata group
             */
            e_ref++;
          }
          break;
        default:
          ccd_setError (globs, ERR_DEFECT_CCDDATA, BREAK, 
                        (USHORT) (globs->bitpos), (USHORT) -1);
          break;
      }
    }
    else
    {
      /*
       * no processing action for an extended group
       */

      /*
       * use the jump-table for selecting the decode function
       */
      codecRet =
        codec[melem[e_ref].codingType][DECODE_FUN](c_ref, e_ref, globs);
      if (codecRet NEQ 0x7f)
      {
        /*
         * set the e_ref to the next or the same element
         */
        e_ref += codecRet;
      }
    }
  }

  /* Reset indicator of exhaustion in the IEI table*/
  for (i = 0; globs->iei_ctx[globs->ccd_recurs_level].iei_table[i].valid== TRUE; i++)
  {
    globs->iei_ctx[globs->ccd_recurs_level].iei_table[i].exhausted = FALSE;
    globs->iei_ctx[globs->ccd_recurs_level].iei_table[i].act_amount = 0;

  }

  globs->iei_ctx[globs->ccd_recurs_level].countSkipped = 0;
  globs->ccd_recurs_level--;
  globs->errLabel  = (U8) act_err_label;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
 * Attention: if this static function shall be used in
 * both, internal as well as external RAM, it must be made global.
 */
static void SkipExtensionOctets (ULONG *e_ref, ULONG l_ref)
{
  /* The extended group is terminated. Skip over its IEs. */
  while (*e_ref < l_ref && melem[*e_ref].extGroup != '*')
    (*e_ref)++;
  if (*e_ref < l_ref)
    (*e_ref)++;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
 * Attention: if this static function shall be used in
 * both, internal as well as external RAM, it must be made global.
 */
LOCAL BOOL CheckBP_RecodeXB (ULONG ExtBitPos1, ULONG ExtBitPos2, 
                             ULONG ExtBitPos3, T_CCD_Globs *globs)
{
  /* Absence due to validity flag, condition or a ccdWarning. */
  if (ExtBitPos3 == globs->bitpos )
  {
    globs->bitpos --;    
    if (ExtBitPos1 NEQ ExtBitPos2)
      bf_recodeBit ((USHORT) ExtBitPos1, 1, globs);
    return FALSE;
  }
  return TRUE;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_encodeComposition |
+--------------------------------------------------------------------+

  PURPOSE : codes the content of a C-Structure into a bitstream.
            This function may be called recursivly if an IE in the
            structure is itself a structured IE.

            For extended octet groups the following rules apply.
            
            Any IE within an extension octet is characterised by its 
            extGroup which is one of the following:
            
            
            '+'    octet begins with this IE.
            ' '    octet continues with this IE or a new octet begins.
            '-'    octet ends with this IE.
            '*'    octet group ends with this IE.
            
            An octet may begin with '+', '-', ' ' or '*'.
            An octet beginning with '-' or '*' is made of only one IE (7 bits).
            A group made of one octet has only one IE with '*'.
            If '+' is present, next IEs ending with '*' or '-' are present too.
            If the beginning IE  with ' ' is not IE

            Examples of extended group configurations.
             _ _      _ _ _ 
            | | |    | | | |
            |+|*|    |+| |*|  one octet
            |_|_|    |_|_|_|

             _ _ _ _      _ _ _      _ _ _ _ 
            | | | | |    | | | |    | | | | |
            |+|-| |*|    |+|-|*|    |+| |-|*|  two octets
            |_|_|_|_|    |_|_|_|    |_|_|_|_|


             _ _ _ _ _ _ _ _ _ _ 
            | | | | | | | | | | |
            |+| | |-| |-| |-| |*|   four octets
            |_|_|_|_|_|_|_|_|_|_|
            
            Also groups after groups are possible.
             _ _ _ _ _ _ _ _ _
            | | | | | | | | | |
            |+| |-| |*|+|*|*|*|
            |_|_|_|_|_|_|_|_|_|
            
            The status of encoding is given as follows:

            ExtendedGroupActive is set to 1 if '+' is present in the encoding.
            ExtendedGroupActive is changed from 1 to 0 when encoding '*'.
            ExtendedGroupActive is set to 0 if an octet beginning with ' ' is
            not present.
            OpenNewOctet is set to 1 after encoding an IE with '-'.
            ExtBitPos1 is set to the globs->bitpos when writing extension bit:
            1) for '+' or 
            2) for ' ' after '-'

            Extension bit is set to 0 if a further octet is encoded for
            the group. If an octet is the last one within a group, the 
            extension bit is set to 1.

            While processing the first IE of an octet (e.g. for '+'),
            if the IE is absent, CCD will skip over all IEs up to the
            ending '*'.
            When processing IEs with ' ' and for ExtendedGroupActive=1 the valid
            flag is set to 1 in order to compensate probable mistakes in PS.

*/

void ccd_encodeComposition (const ULONG c_ref, T_CCD_Globs *globs)
{
  ULONG  e_ref;
  ULONG  l_ref;
  BOOL   ExtendedGroupActive = FALSE, OpenNewOctet = FALSE;
  ULONG  ext_bit_pos1=0, ext_bit_pos2=0, ext_bit_pos3=0, more_data_bit_pos=0;
  int    ext_bit_pending=0;
  UBYTE  codecRet;


#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
    TRACE_CCD (globs, "ccd_encodeComposition()");
  #else
    TRACE_CCD (globs, "ccd_encodeComposition(): Composition = %s",
               mcomp[c_ref].name);
    #endif
#endif

  globs->ccd_recurs_level++;

  /*
   * setup the index in the melem table for this composition.
   * If this function is called for the first time
   * (ccd_recurs_level == 1) the elem-table entry for the msg_type
   * was skipped.
   */

  l_ref = (USHORT)(mcomp[c_ref].componentRef
           + mcomp[c_ref].numOfComponents);

  e_ref  = (USHORT)(mcomp[c_ref].componentRef
           + ((globs->ccd_recurs_level EQ 1) ? 1 : 0));

  /*
   * code all elements
   */
  while (e_ref < l_ref)
  {
#ifdef ERR_TRC_STK_CCD
    /* 
     * Save the value for tracing in error case.
     */
    globs->error_stack[globs->ccd_recurs_level] = (USHORT) e_ref;
#endif /* ERR_TRC_STK_CCD */

    /*
     * look for extension group processing
     */
    if (melem[e_ref].extGroup NEQ ' ')
    {
      /*
       * extended group symbol found
       */
      switch (melem[e_ref].extGroup)
      {
        case '+':

          /*
           * remember the position of the extension bit
           * because we maybe have to recode it.
           */
          ext_bit_pos1 = ext_bit_pos2 = (ULONG) globs->bitpos;


          /*
           * write the extension bit. It may overwritten
           * later. (ext=0 -> group extended)
           */
          bf_writeBit (0, globs);
          ext_bit_pos3 = (USHORT) globs->bitpos;

#if defined(_TOOLS_)
      if (ccd_patch (globs, 0))
        codecRet = 1;
      else
#endif /* _TOOLS_ */
          /* Use the jump-table for selecting encode function. */
          codecRet = (UBYTE)
            codec[melem[e_ref].codingType][ENCODE_FUN](c_ref, e_ref, globs);

          /* Absence due to validity flag, conditions or a ccdWarning. */
          if (!CheckBP_RecodeXB (ext_bit_pos1, ext_bit_pos2, ext_bit_pos3, globs))
          {
            ExtendedGroupActive = FALSE;
            SkipExtensionOctets (&e_ref, l_ref);
            continue;
          }
          /* Start of an extended group */
          else
          {
            OpenNewOctet = FALSE;
          }

          ExtendedGroupActive = TRUE;
          ext_bit_pending = 1;

          if (codecRet NEQ 0x7f)
          {
            /* Set the elemRef to the next or the same element. */
            e_ref += codecRet;
          }
          break;

        /* End of one extension octet. */
        case '-':
          /* IE must be present if the previous one was not with '-'. */
          if (OpenNewOctet == FALSE)
          {
#ifdef DEBUG_CCD
            if (globs->pstruct [melem[e_ref].structOffs] != TRUE)
            {
              TRACE_CCD (globs, "Wrong value for valid flag!\n...changed to 1 for ccdID=%d",
                         e_ref);
            }
#endif
            /* IE has to be present. Don't trust PS code. */
            globs->pstruct [melem[e_ref].structOffs] = TRUE;
          }
          ext_bit_pos3 = (USHORT) globs->bitpos;

#if defined(_TOOLS_)
      if (ccd_patch (globs, 0))
        codecRet = 1;
      else
#endif /* _TOOLS_ */
          /* Use the jump-table for selecting encode function. */
          codecRet = (UBYTE)
            codec[melem[e_ref].codingType][ENCODE_FUN](c_ref, e_ref, globs);

          if (OpenNewOctet == TRUE)
          {
            /* Absence due to validity flag, conditions or a ccdWarning. */
            if (!CheckBP_RecodeXB (ext_bit_pos1, ext_bit_pos2, ext_bit_pos3, globs))
            {
              ExtendedGroupActive = FALSE;
              SkipExtensionOctets (&e_ref, l_ref);
              continue;
            }
          }
          
          if (codecRet NEQ 0x7f)
          {
          /* Set the elemRef to the next or the same element. */
            e_ref += codecRet;
          }

          /* IE with '-' belongs to the first octet. */
          if (ext_bit_pending EQ 1)
          {
            ext_bit_pending++;
          }
          /*
           * At least one octet has been encoded for the current group.
           * Swap the stored positions of the extension bits.
           */
          else
          {
            ext_bit_pos1 = ext_bit_pos2;
          }

          /*
           * Store the position of the extension bit
           * because we maybe have to recode it.
           */
          ext_bit_pos2 = (ULONG) globs->bitpos;
          /*
           * write the extension bit. It may overwritten
           * later. (ext=0 -> group extended)
           */
          bf_writeBit (0, globs);
          OpenNewOctet = TRUE;

          break;

        case '*':
          if (!ExtendedGroupActive)
          {
            /*
             * This is a single element extended group, often
             * used for later extension of the protocol
             * Write '1' in extension bit since group is not extended yet.
             */
            bf_writeBit (1, globs);
            ext_bit_pos3 = (USHORT) globs->bitpos;

#if defined(_TOOLS_)
      if (ccd_patch (globs, 0))
        codecRet = 1;
      else
#endif /* _TOOLS_ */
            /* Use the jump-table for selecting encode function. */
            codecRet = (UBYTE)
              codec[melem[e_ref].codingType][ENCODE_FUN](c_ref, e_ref, globs);
            /* Absence due to the valid flag or a ccdWarning. */
            if (ext_bit_pos3 == globs->bitpos)
            {
              globs->bitpos --;
            }

            if (codecRet NEQ 0x7f)
            {
              /* Set the elemRef to the next or the same element. */
              e_ref += codecRet;
            }
          }
          else
          {
            ExtendedGroupActive = FALSE;
            /* IE must be present if the previous one was not with '-'. */
            if (OpenNewOctet == FALSE)
            {
#ifdef DEBUG_CCD
              if (globs->pstruct [melem[e_ref].structOffs] != TRUE)
              {
                TRACE_CCD (globs, "Wrong value for valid flag!\n...changed to 1 for ccdID=%d",
                           e_ref);
              }
#endif
              /* IE has to be present. Don't trust PS code. */
              globs->pstruct [melem[e_ref].structOffs] = TRUE;
            }

#if defined(_TOOLS_)
      if (ccd_patch (globs, 0))
        codecRet = 1;
      else
#endif /* _TOOLS_ */
             /* Use the jump-table for selecting the encode function. */
            codecRet = (UBYTE)
              codec[melem[e_ref].codingType][ENCODE_FUN](c_ref, e_ref, globs);
            ext_bit_pos3 = (USHORT) globs->bitpos;

            if (codecRet NEQ 0x7f)
            {
              /* Set the elemRef to the next or the same element. */
              e_ref += codecRet;
            }

            if ((ext_bit_pos3-1) NEQ ext_bit_pos2)
            {
              /*
               * if the writepointer (ext_bit_pos3) have incremented
               * since the first extension bit (NEQ ext_bit_pos1)
               * at least one element of this group is valid
               * and written into the bitstream.
               */
              /*
               * the extended group is terminated,
               * so we have to switch the previously written
               * extBit from 0 to 1
               */
              bf_recodeBit ((USHORT) ext_bit_pos2, 1, globs);
            }
            else
            {
              /*
               * break of an extended group, no element are coded
               * since the last extension bit.
               */
              /*
               * the extended group is terminated,
               * so we have to switch the previously written
               * extBit from 0 to 1
               */
              if (ext_bit_pos1 NEQ ext_bit_pos2)
              {
                bf_recodeBit ((USHORT) ext_bit_pos1, 1, globs);
              }
              /*
               * delete the written ext-bit because the extended
               * group ended.
               */
              bf_setBitpos ((USHORT) ext_bit_pos2, globs);
            }
          }
          break;
        case '!':
        case '#':
          more_data_bit_pos = (ULONG) globs->bitpos;
          bf_writeBit (1, globs);

#if defined(_TOOLS_)
      if (ccd_patch (globs, 0))
        codecRet = 1;
      else
#endif /* _TOOLS_ */
          /*
           * use the jump-table for selecting the encode function
           */
          codecRet = (UBYTE)
            codec[melem[e_ref].codingType][ENCODE_FUN](c_ref, e_ref, globs);
          if (codecRet NEQ 0x7f)
          {
            if (more_data_bit_pos+1 EQ globs->bitpos)
            {
              /*
               * rewrite the written moredata bit to 0
               */
              bf_setBitpos (more_data_bit_pos, globs);
              bf_writeBit (0, globs);
              /*
               * no coding performed -> extension group ended
               * We skip all elements in this group
               */
              while (melem[e_ref].extGroup EQ '!')
                e_ref++;

              if (melem[e_ref].extGroup EQ '#')
                e_ref++;
            }
            else
            {
              if (melem[e_ref].extGroup EQ '#')
              {
                /*
                 * code a zero bit for the last element
                 */
                bf_writeBit (0, globs);
              }
              e_ref++;
            }
          }
          break;
        default:
          ccd_setError (globs, ERR_DEFECT_CCDDATA, BREAK, 
                      (USHORT) (globs->bitpos), (USHORT) -1);
        break;
      }
    }
    else
    {
      if (ExtendedGroupActive)
      {
        ext_bit_pos3 = (USHORT) globs->bitpos;

        /* IE in the middle part of an ext-octet. */
        if (OpenNewOctet == FALSE)
        {
#ifdef DEBUG_CCD
          if (globs->pstruct [melem[e_ref].structOffs] != TRUE)
          {
            TRACE_CCD (globs, "Wrong value for valid flag!\n...changed to 1 for ccdID=%d",
                       e_ref);
          }
#endif
          globs->pstruct [melem[e_ref].structOffs] = TRUE;
        }
      }

#if defined(_TOOLS_)
      if (ccd_patch (globs, 0))
        codecRet = 1;
      else
#endif /* _TOOLS_ */
      /* Use the jump-table for selecting encode function. */
      codecRet = (UBYTE)
        codec[melem[e_ref].codingType][ENCODE_FUN](c_ref, e_ref, globs);

      /* The following is only meant for IEs after an IE with '-'. */
      if (ExtendedGroupActive)
      {
        if (OpenNewOctet == TRUE)
        {
          /* Absence due to validity flag, conditions or a ccdWarning. */
          if (!CheckBP_RecodeXB (ext_bit_pos1, ext_bit_pos2, ext_bit_pos3, globs))
          {
            ExtendedGroupActive = FALSE;
            SkipExtensionOctets (&e_ref, l_ref);
            continue;
          }
          /* Start of an extension octet */
          else
          {
            OpenNewOctet = FALSE;
          }
        }
      }

      if (codecRet NEQ 0x7f)
      {
        /* Set the elemRef to the next or the same element. */
        e_ref += codecRet;
      }
    }
  }

  globs->ccd_recurs_level--;
}
#endif /* !RUN_FLASH */

/* ---------------------------------------------------------------- */
/* GLOBAL FUNCTIONS                                                 */
/* ---------------------------------------------------------------- */

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_check_pointer
+------------------------------------------------------------------------------
|  Description  :  This function checks the validity of a pointer
|
|  Parameters   :  ptr - the pointer
|
|  Return       :  ccdOK if pointer valid, otherwise ccdError
+------------------------------------------------------------------------------
*/
int ccd_check_pointer (U8* ptr)
{
  if (!ptr || ptr == mempat
#ifdef WIN32
      /* For windows: check also if ptr is too small */
      || !(ptr && LOWSEGMASK)
#endif
     )
  {
    return ccdError;
  }
  return ccdOK;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_begin             |
+--------------------------------------------------------------------+


  PARAMETERS:  none

  PURPOSE:     Returns the address of the CCD buffer for decoded
               messages and locks it until ccd_end() is called

*/

UBYTE* CCDDATA_PREF(ccd_begin) (void)
{
#ifdef SHARED_CCD
  /*
   * if the CCD is used in a premptive multithreaded system
   * we must lock this critical section
   */
  vsi_s_get (VSI_CALLER semCCD_Buffer);
#endif

#ifdef DEBUG_CCD
{
  #ifndef _TMS470
  T_CCD_Globs globs;
  globs.me = 0;
  globs.TraceIt = 1;
  TRACE_CCD (&globs, "CCD Begin");
  #endif
}
#endif
  return ccd_decMsgBuffer;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_end               |
+--------------------------------------------------------------------+


  PARAMETERS:  none

  PURPOSE:     Unlocks the CCD buffer for decoded
               messages

*/

void CCDDATA_PREF(ccd_end) (void)
{
#ifdef SHARED_CCD
  /*
   * if the CCD is used in a premptive multithreaded system
   * we must unlock this critical section of accessing the
   * decoded message buffer
   */
  vsi_s_release (VSI_CALLER semCCD_Buffer);
#endif
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_GetGlobVars       |
+--------------------------------------------------------------------+


  PARAMETERS:  

  PURPOSE:     Determines which set of global variables to use
               and locks the CCD buffer (semaphore) if necessary.

*/
T_CCD_Globs* ccd_GetGlobVars (T_CCD_ERR_LIST_HEAD** eentry, T_CCD_STORE_LIST** stoentry)
{
  T_CCD_TASK_TABLE* tentry;
#ifndef SHARED_CCD
  int me = 0;
#else
  T_HANDLE me;
  me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
#endif /* !SHARED_CCD */

  tentry = ccd_task_list[me];
  tentry->ccd_globs->me = me;
  *eentry = tentry->ccd_err_list;
  *stoentry = tentry->ccd_store;
#ifdef SHARED_CCD
  if (tentry->ccd_globs == &globs_all)
  {
    /*
     * if the CCD is used in a premptive multithreaded system
     * we must lock this critical section
     */
    vsi_s_get (VSI_CALLER semCCD_Codec);
  }
#endif /* SHARED_CCD */
  return tentry->ccd_globs;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_FreeGlobVars      |
+--------------------------------------------------------------------+


  PARAMETERS:  none

  PURPOSE:     Unlocks the CCD buffer for decoded messages 
               if the entity is not GRR and if 
               the CCD is used in a premptive multithreaded system.

*/
void ccd_FreeGlobVars (T_CCD_Globs* globs)
{
#ifdef SHARED_CCD
  if (globs == &globs_all)
  {   
    vsi_s_release (VSI_CALLER semCCD_Codec);
  }
#endif /* SHARED_CCD */
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
#ifdef DEBUG_CCD
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_dump_msg          |
+--------------------------------------------------------------------+

  PARAMETERS:  U16 l_buf
                - Number of bits in the encoded message or IE.

               U16 o_buf
                - Offset of the bitstream buffer in bits.

               U8 *buf
                - Bitstream buffer of the encoded message or IE.

               T_CCD_Globs globs
                - Pointer to global variables

  PURPOSE:     Dump contents of air interface message - for debugging

*/
void ccd_dump_msg (U16 l_buf, U16 o_buf, U8 *buf, T_CCD_Globs *globs)
{

  if (!globs->TraceIt)
    return;
  else
  {
    int  i, j, buflen;
    char s[64], c[4];

    buflen = (l_buf + o_buf + 7) >> 3;
    TRACE_CCD (globs, "-------------------------------------------------");
    TRACE_CCD (globs, " CCD: Decode Message");
    TRACE_CCD (globs, " Before DECODING: lbuf= %d, obuf= %d", l_buf, o_buf);
    TRACE_CCD (globs, " Hex dump of message to be decoded:");

    s[0] = '\0';
    for (i = o_buf>>3; i < buflen; i+=16)
    {
      for (j = 0; j < 16; j++)
      {
	if ((i+j) < buflen)
	{
	  sprintf(c, " %02x", buf[i+j]);
	  strcat (s, c);
	}
      }
      TRACE_CCD (globs, "%s", s);
      s[0] = '\0';
    }
  }
}
#endif /* DEBUG_CCD */
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_common_decode_init|
+--------------------------------------------------------------------+

  PARAMETERS:  

  PURPOSE:     Perform common CCD initialization before message decode.

*/
void ccd_common_decode_init(U16 l_buf, U16 o_buf, U8 *buf, T_CCD_Globs *globs)
{
#if defined (DEBUG_CCD)
  /* to avoid the vsprintf if the traces won't appear anyhow */
  ULONG mask;
  if (vsi_gettracemask (globs->me, globs->me, &mask) != VSI_ERROR)
  {
    globs->TraceIt = mask & TC_CCD;
  }
#endif

  /*
   * no call to setjmp() done. So ccd_Error performs no longjmp in
   * case of an error
   */
  globs->jmp_mark_set = FALSE;

  /* setup the bitbuffer */
  globs->bitbuf = buf;
  globs->bitpos = 0;

  /* cleanup the read-caches */
  globs->lastbytepos16 = globs->lastbytepos32 = 0xffff;

  /* setup the bitoffset */
  globs->bitoffs = o_buf;

  
  bf_incBitpos (o_buf, globs);

  /*
   * calclate the max bitpos to read
   */
  globs->buflen = l_buf + o_buf;
  globs->maxBitpos = globs->buflen;
  globs->msgLen = globs->buflen;
  globs->errLabel = 0;
  globs->continue_array = TRUE;
  globs->SeekTLVExt = TRUE;

  globs->ccd_recurs_level = 0;
  globs->CCD_Error = ccdOK;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
 * Attention: if this static function shall be used in
 * both, internal as well as external RAM, it must be made global.
 */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : SeekForGSMExtensions  |
+--------------------------------------------------------------------+

  PURPOSE:     Check if there are elements left in the air message
               without being decoded. Assume that the next IE has the
               type T, TV or a TLV. Seek for Comprehension Required
               extensions.
               1) Rules to distinguish between the types:
               handling of unknown IEs (GSM 04.07, section 11.2.4)
               "Bit 8 of the IEI octet is set to "1" indicates a TV 
               formatted type 1 standard IE or a T formatted type 2 
               IEs, and to "0" indicates a TLV formatted type 4 IE. 
               Hence, a 1 valued bit 8 indicates that the whole IE is 
               one octet long, and a 0 valued bit 8 indicates that the
               following octet is a length octet.
               1) Rules to read the length:
               For TLV4 encode L in one byte.
               For TLV5 and ASN.1 encode in one byte or more. Use
               0x81, 0x82 etc in the first byte to say L is encoded
               in one, two etc bytes.
               For ASN.1 BER type use 0x80 in the first byte and
               two otets of 0 bits after the encoded IE.
               For all the three types it is possible to encode L
               in only one byte.
               TLV6 is not supported. TLV6 is not a GSM standard 
               type. It has been defined in g23net software.
*/
static void SeekForGSMExtensions (U8 entity, T_CCD_Globs *globs)
{
  U8 *MsgBuf = (U8*)globs->bitbuf;
  U16 BP = globs->bytepos;
  U16 OctNr = (globs->buflen+7) >> 3;
  U16 errPos = 0;
  U8 errTag = 0xFF;
  U8 errCode;

#ifdef DEBUG_CCD
  TRACE_CCD( globs, "SeekForGSMExtensions...");
#endif
  while (BP < OctNr)
  {
    U8 T = MsgBuf[BP];
    BOOL tlv5 = FALSE;

    /* Expecting a CCDTYPE_GSM5_TLV type we get an unknown tag with MSB set.
     * Expecting other types than CCDTYPE_GSM5_TLV we get an unknown tag with
     * comprehension required bits (5, 6, 7 and 8 of IEI according to GSM0407)
     * are set to zero.
     */
    tlv5 = (aim_sat == entity);
    if ( (tlv5 && ((T & 0x80) == 0x80))
        ||
         (!tlv5 && ((T & 0xf0) == 0)) )
    {
      errTag = T;
      errPos = BP*8;
      errCode = ERR_COMPREH_REQUIRED;
      /* Report error. Do not analyse the rest. */
      break;
    }
    else
    {
      errTag = T;
      errPos = BP*8;
      errCode = ERR_IE_NOT_EXPECTED;
    }

    /* The type is TLV or ASN.1. */
    if ((T & 0x80) NEQ 0x80)
    { 
      /* BP points to the first L byte */
      U16 L = MsgBuf[++BP]; 
      switch (L)
      {
        /* End Of Content (ASN.1 BER) */
        case 0x80:
          do
          {
            BP++;
          } while ((*(U16*)&MsgBuf[BP]) != 0 && BP < OctNr);
          BP++;
          L = 0;
          break;

        /* Length encoding in two bytes */
        case 0x81:
          L = MsgBuf[++BP];
          break;

        /* Length encoding in three bytes */
        case 0x82:
          L = (MsgBuf[++BP]) << 8;
          L |= MsgBuf[++BP];
          break;

        /* Length encoding in one byte. */
        default:
          break;
      }

      /* Skip over the bytes. */
      while (L > 0 && BP < OctNr)
      {
        --L;
        ++BP;
      } 

    } /* if TLV */
    /* else: The type is T or TV. Skip one byte (see next step). */

    /* 
     * BP points to the last byte of the skipped IE, 
     * move to the next IE.
     */
    BP++;
  }/* while bytes left unread */

  /* Report the summary of the found erros. */
  if (errTag != 0xFF)
  {
    ccd_setError (globs, errCode,
      (U8)((errCode == ERR_COMPREH_REQUIRED) ? BREAK : CONTINUE),
                  errTag,
                  errPos,
                  (USHORT) -1);
  }
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_decodeMsg         |
+--------------------------------------------------------------------+


  PARAMETERS:  UBYTE      entity
                - specifies the calling entity of CCD. The Constants
                  for each valid entity is defined in MCONST.CDG

               UBYTE      direction
                - specifies wether the message goes UPLINK or DOWNLINK.
                  This is nessesary because there are same PDU-Type
                  codes for different messages.

	              T_MSGBUF * mBuf
                - specifies the bitstream buffer of the message. The
                  struct contains the l_buf and the o_buf elements.
                  These elements specifies the length and offset in bits
                  of the bitstream in the T_MSGBUF component buf.

               UBYTE    * mStruct
                - reference to the C-Structure of the decoded message.
                  The type may differ so the Pointer is always typed as
                  UBYTE* and must be casted after decoding. If this parameter
                  is NULL CCD uses his internal buffer wich must be
                  protected via ccd_begin() in a multithread environment.

               UBYTE      mId
                - specifies the PDU-Type of the bitstream. If this
                  parameter is not equal 0xff the CCD does not decode
                  the pdu-type from the bitstream to decide wich decoding
                  rules to select. Normaly this param is set to 0xff.

  PURPOSE:     decodes a bitstream, containing a valid TETRA-Message
               of the Air-Interface to a corresponding C-Structure.

*/

BYTE CCDDATA_PREF(ccd_decodeMsg) (UBYTE         entity,
                                  UBYTE         direction,
                                  T_MSGBUF     *mBuf,
                                  UBYTE        *mStruct,
                                  UBYTE         mId)
{
  UBYTE theMsgId;
  USHORT mcompRef;
  int    jmp_ret;
  T_CCD_Globs *globs;
  T_CCD_ERR_LIST_HEAD* eentry;
  T_CCD_STORE_LIST* stoentry;

    globs = ccd_GetGlobVars (&eentry, &stoentry);

    /*
     * setup the structure-buffer
     *
     * if the parameter of the decoded message buffer address is NULL
     * we use the internal one
     */
    globs->pstruct = (mStruct EQ NULL) ? ccd_decMsgBuffer : mStruct;
    globs->pstructOffs = 0;

    ccd_common_decode_init(mBuf->l_buf, mBuf->o_buf, mBuf->buf, globs);
    ccd_err_reset (eentry);
    globs->errLabel = 0;
    globs->continue_array = TRUE;

  #ifdef DEBUG_CCD
    ccd_dump_msg(mBuf->l_buf, mBuf->o_buf, mBuf->buf, globs);
  #endif

    if (mId NEQ 0xff AND mId NEQ 0xfe)
      globs->pstruct[0] = theMsgId = mId;
    else
    {
      /* Read the message identifier */
      globs->pstruct[0] = bf_decodeByteNumber ((ULONG) mi_length[entity], globs);
      theMsgId = globs->pstruct[0];
    }

    /* Get entry in mmtx table for this message */
    mcompRef = ccddata_get_mmtx(entity,theMsgId,direction);

    /* Check the message identifier */
    if (theMsgId > max_message_id OR mcompRef EQ NO_REF)
    {
  #ifdef ERR_TRC_STK_CCD
      globs->ccd_recurs_level = 255;
  #endif /* ERR_TRC_STK_CCD */
      ccd_setError (globs, ERR_INVALID_MID, BREAK,
		    (USHORT) theMsgId, (USHORT) -1);
      ccd_FreeGlobVars (globs);
      ccd_err_free (eentry);
      return (BYTE)globs->CCD_Error;
    }
  #ifdef ERR_TRC_STK_CCD
    /* save the value for tracing in error case */
    globs->error_stack[0] = mcompRef;
  #endif /* ERR_TRC_STK_CCD */

  #ifdef DEBUG_CCD
  #ifdef CCD_SYMBOLS
    TRACE_CCD (globs, "CCD decode: Message = %s",
    mcomp[mcompRef].name);
  #else
    TRACE_CCD (globs, "CCD decode: MessageId = %x", theMsgId);
  #endif
  #endif

  /* 
   * Clean up the entite C-structure before decoding.
   * Do not overwrite the MsgId (1. Byte)
   */
  #ifdef DEBUG_CCD
    TRACE_CCD (globs, "CCD Cleaning struct %ld bytes",
	       mcomp[mcompRef].cSize-1);
  #endif
  memset ((UBYTE *) &globs->pstruct[1], 0,
    (size_t)(mcomp[mcompRef].cSize - 1));

    /*
     * clear the UPN stack
     */
    ST_CLEAR(globs);
    memset ((ULONG *) &(globs->KeepReg[0]), 0, MAX_KEEP_REG_SIZE);

    /*
     * inform the GSM-CODEC about the begin of a new message
     */
    cdc_GSM_start (globs);

    jmp_ret = setjmp (globs->jmp_mark);

    if (jmp_ret EQ 0)
    {
      globs->jmp_mark_set = TRUE;
      ccd_decodeComposition ((ULONG) mcompRef, globs);

      /*
       * The coding rules and the data tables must be able to lead CCD to
       * understand the bit buffer, element by element. If for some reason
       * the bit pointer points to a place after the message limit, the
       * encoding action is not trustworthy. Nevertheless CCD reports a
       * WARNING and not error.
       * In development phase it is helpful to detect such cases.
       * Otherwise the caller can supposedly still use the message.
       */
      if (globs->bitpos > globs->buflen)
      {
        ccd_recordFault (globs, ERR_MSG_LEN, CONTINUE, mcompRef, NULL);
      }
      /*
       * Seek for GSM extensions with types T, TV and TLV.
       * (GRR, RCM/RRC and RRLP assume other extension mechanisms.)
       * Check only at octet border.
       */
      else if ((globs->SeekTLVExt) && !globs->byteoffs)
      {
        SeekForGSMExtensions (entity, globs);
      }
  }


  #ifdef DEBUG_CCD
    TRACE_CCD (globs, "CCD-ERROR = %d", globs->CCD_Error);
    TRACE_CCD (globs, "-------------------------------------------------");
  #endif /* DEBUG_CCD */

    ccd_FreeGlobVars (globs);
    ccd_err_free (eentry);

    return (BYTE) globs->CCD_Error;
  }  /* end ccd_decodeMsg () */
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
  /*
  +--------------------------------------------------------------------+
  | PROJECT : CCD (6144)               MODULE  : CCD                   |
  | STATE   : code                     ROUTINE : ccd_decodeMsgPtr      |
  +--------------------------------------------------------------------+


    PARAMETERS:  U8      entity
		  - specifies the calling entity of CCD. The Constants
		    for each valid entity is defined in MCONST.CDG

		 U8      direction
		  - specifies wether the message goes UPLINK or DOWNLINK.
		    This is nessesary because there are same PDU-Type
		    codes for different messages.

		       U16 l_buf
		  - Lenght of the bitstrem to be decoded.

							   U16 o_buf
		  - specifies the bitstream buffer of the message. The
		    struct contains the l_buf and the o_buf elements.
		    These elements specifies the length and offset in bits
		    of the bitstream in the T_MSGBUF component buf.

							   U8 *buf
		  - specifies the bitstream buffer of the message. 

		 U8 ** mStructPtr
		  -	points to the pointer on the C-structure of the 
		    decoded message. The buffer containing this message 
		    structure will be allocated by CCD. After decoding 
		    the first element in the message C-structure contains 
		    the message (PDU) type as a UBYTE.

		 U8  mId
		  - specifies the PDU-Type of the bitstream. If this
		    parameter is not equal 0xff the CCD does not decode
		    the pdu-type from the bitstream to decide wich decoding
		    rules to select. Normaly this param is set to 0xff.

    PURPOSE:     Like ccd_decodeMsg, this function decodes a bitstream
		 containing a valid TETRA-Message from the Air-Interface
		 to a corresponding C-Structure, only this function
		 allows the use of pointer types in the C-structure.

  */
  #if defined DYNAMIC_ARRAYS || defined _TOOLS_
S8 CCDDATA_PREF(ccd_decodeMsgPtr) (U8   entity,
		       U8   direction,
		       U16  l_buf,
		       U16  o_buf,
		       U8*  buf,
		       U8** mStructPtr,
		       U8   mId)
{
  /*
   * Make a dummy for the DLLs, even if DYNAMIC_ARRAYS is not defined to
   * keep the ccd.def unique
   */
  #ifndef DYNAMIC_ARRAYS
    return -1;
}
  #else /* DYNAMIC_ARRAYS */
    UBYTE  theMsgId;
    int    jmp_ret;
    U32    msgsize;
    USHORT mcompRef;
    T_CCD_Globs *globs;
    T_CCD_ERR_LIST_HEAD* eentry;
    T_CCD_STORE_LIST* stoentry;

    globs = ccd_GetGlobVars (&eentry, &stoentry);
    ccd_common_decode_init(l_buf, o_buf, buf, globs);
    ccd_err_reset (eentry);
    globs->errLabel = 0;
    globs->continue_array = TRUE;
    globs->alloc_head = *mStructPtr = NULL;

  #ifdef DEBUG_CCD
    TRACE_CCD( globs, "======================================================");
    TRACE_CCD( globs, "ccd_decodeMsgPtr: Decoding message with pointer types.");
    ccd_dump_msg(l_buf, o_buf, buf, globs);
  #endif

	    /* Read the message identifier. */
    if (mId NEQ 0xff AND mId NEQ 0xfe)
      theMsgId = mId;
    else
      theMsgId = bf_decodeByteNumber ((ULONG) mi_length[entity], globs);

    /* Get the entry in mmtx table for this message */
    mcompRef = ccddata_get_mmtx (entity,theMsgId,direction);
     
    /* Check the validity of the message identifier */
    if (theMsgId > max_message_id OR mcompRef EQ NO_REF)
    {
  #ifdef ERR_TRC_STK_CCD
      globs->ccd_recurs_level = 255;
  #endif /* ERR_TRC_STK_CCD */
      ccd_setError (globs, ERR_INVALID_MID, BREAK, (USHORT) theMsgId, (USHORT) -1);
      ccd_FreeGlobVars (globs);
      ccd_err_free (eentry);
      return (BYTE) globs->CCD_Error;
    }

  #ifdef ERR_TRC_STK_CCD
    /* save the value for tracing in error case */
    globs->error_stack[0] = mcompRef;
  #endif /* ERR_TRC_STK_CCD */

  #ifdef DEBUG_CCD
  #ifdef CCD_SYMBOLS
    TRACE_CCD (globs, "CCD decode: Message = %s",
    mcomp[mcompRef].name);
  #else
    TRACE_CCD (globs, "CCD decode: MessageId = %x", theMsgId);
  #endif
  #endif

    /*
     * Setup the structure-buffer.
     * Make a first simple estimation of much memory to allocate.
     * It is twice as much as the air interface message, rounded up 
     * to the nearest kilobyte boundary.
     */
    msgsize = mcomp[mcompRef].cSize;
    
  #ifdef DEBUG_CCD
    TRACE_CCD( globs, "Allocating %ld bytes for msg.", msgsize);
  #endif
    
    globs->alloc_head = (U8 *) DRP_ALLOC(msgsize, DP_NO_FRAME_GUESS);

    if (globs->alloc_head == NULL)
    {
      ccd_setError (globs, ERR_NO_MEM, BREAK, (USHORT) theMsgId, (USHORT) -1);
      ccd_FreeGlobVars (globs);
      ccd_err_free (eentry);
      return (BYTE) globs->CCD_Error;
    }
    *mStructPtr = globs->alloc_head;
    globs->pstruct = globs->alloc_head;
    globs->pstructOffs = 0;

    /*Write the MSG ID in the buffer*/
    globs->pstruct[0] = theMsgId;

    /* 
     * Clean up the entite C-structure before decoding.
     * Do not overwrite the MsgId (1. Byte)
     */
  #ifdef DEBUG_CCD
    TRACE_CCD (globs, "CCD Cleaning struct %ld bytes", mcomp[mcompRef].cSize);
  #endif
  memset ((U8 *)globs->pstruct, 0, (size_t)(msgsize));

    /* 
     * Write the message identifier into the C-structure.
     */
    globs->pstruct[0] = theMsgId;

    /*
     * clear the UPN stack
     */
    ST_CLEAR(globs);
    memset ((ULONG *) &(globs->KeepReg[0]), 0, MAX_KEEP_REG_SIZE);

    /*
     * inform the GSM-CODEC about the begin of a new message
     */
    cdc_GSM_start (globs);

    jmp_ret = setjmp (globs->jmp_mark);

    if (jmp_ret EQ 0)
    {
      globs->jmp_mark_set = TRUE;
      ccd_decodeComposition ((ULONG) mcompRef, globs);
      if (globs->byteoffs NEQ 0)
      {
       bf_incBitpos (8-globs->byteoffs, globs);
  
        /* There are more bits to be decoded than ccddata expects.
         * The additional bits may belong to unknown non-critical extensions.
         */
        if (globs->bitpos > globs->buflen)
        {
         ccd_recordFault (globs, ERR_UNEXPECT_PAD, CONTINUE, mcompRef, NULL);
        }
        /*
         * Seek for GSM extensions with types T, TV and TLV.
         * (GRR, RCM/RRC and RRLP assume other extension mechanisms.)
         * Check only at octet border.
         */
        else if ((entity != aim_rrc_rcm) 
                && (entity != aim_rrlp)
                && (globs->SeekTLVExt)
                && !globs->byteoffs)
        {
          SeekForGSMExtensions (entity, globs);
        }
      }
      else
      {
        if (globs->bitpos > globs->buflen)
        {
         ccd_recordFault (globs, ERR_MSG_LEN, CONTINUE, mcompRef, NULL);
        }
        /*
         * Seek for GSM extensions with types T, TV and TLV.
         * (GRR, RCM/RRC and RRLP assume other extension mechanisms.)
         * Check only at octet border.
         */
        else if ((entity != aim_rrc_rcm) 
                && (entity != aim_rrlp)
                && (globs->SeekTLVExt)
                && !globs->byteoffs)
        {
          SeekForGSMExtensions (entity, globs);
        }
      }
    }

  #ifdef DEBUG_CCD
    TRACE_CCD (globs, "CCD-ERROR = %d", globs->CCD_Error);
    TRACE_CCD (globs, "-----------------------------------------------------");
  #endif /* DEBUG_CCD */

    ccd_FreeGlobVars (globs);
    ccd_err_free (eentry);

    return (BYTE) globs->CCD_Error;
}  /* end ccd_decodeMsgPtr () */
  #endif /* !DYNAMIC_ARRAYS */
  #endif /* DYNAMIC_ARRAYS || _TOOLS_ */
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
  /*
  +--------------------------------------------------------------------+
  | PROJECT : CCD (6144)               MODULE  : CCD                   |
  | STATE   : code                     ROUTINE : ccd_codeMsgPtr        |
  +--------------------------------------------------------------------+

   PARAMETERS:   UBYTE      entity
		  - specifies the calling entity of CCD. The Constants
		    for each valid entity is defined in MCONST.CDG

		 UBYTE      direction
		  - specifies wether the message goes UPLINK or DOWNLINK.
		    This is nessesary because there are same PDU-Type
		    codes for different messages.

		 T_MSGBUF * mBuf
		  - specifies the bitstream buffer of the message. The
		    struct contains the l_buf and the o_buf elements.
		    These elements specifies the length and offset in bits
		    of the bitstream in the T_MSGBUF component buf.
		    The o_buf component must be specified by the caller,
		    the l_buf component is calculated by CCD.

		 UBYTE    * mStruct
		  - reference to the C-Structure containing the
		    C-Representation of the decoded message.
		    The type should be casted to UBYTE*. If this parameter
		    is NULL CCD uses his internal buffer wich must be
		    protected via ccd_begin() in a multithread environment.

		 UBYTE      mId
		  - specifies the PDU-Type of the bitstream. If this
		    parameter is not equal 0xff the CCD does not read
		    the pdu-type from the structure component pt
		    to decide wich decoding rules to select.
		    Normaly this param is set to 0xff.

   PURPOSE:      encodes a C-Structure containing the C-Representation of
		 a valid Air-interface message to a bitstream.

  */

S8 CCDDATA_PREF(ccd_codeMsgPtr)(U8   entity,
		    U8   direction,
		    U16* l_buf,
		    U16  o_buf,
		    U8   *buf,
		    U8*  mStruct,
		    U8   mId)
{
    UBYTE  theMsgId;
    int    jmp_ret;
    USHORT maxBytes, mcompRef;
    T_CCD_Globs *globs;
    T_CCD_ERR_LIST_HEAD* eentry;
    T_CCD_STORE_LIST* stoentry;
    
    globs = ccd_GetGlobVars (&eentry, &stoentry);
    ccd_err_reset (eentry);

  #if defined (DEBUG_CCD)
    {
      /* to avoid the vsprintf if the traces won't appear anyhow */
      ULONG mask;
      if (vsi_gettracemask (globs->me, globs->me, &mask) != VSI_ERROR)
      {
	globs->TraceIt = mask & TC_CCD;
      }
    }
  #endif

    /*
     * Set a sign that no call to setjmp() is done. So ccd_setError 
     * performs no longjmp in case of an error.
     */
    globs->jmp_mark_set = FALSE;

    /* Setup the bitbuffer. */
    globs->bitbuf = buf;
    globs->bitpos = 0;

    /* and the structure-buffer */
    globs->pstruct = (mStruct EQ NULL) ? ccd_decMsgBuffer : mStruct;
    globs->pstructOffs = 0;

    /* Cleanup the read-caches. */
    globs->lastbytepos16 = globs->lastbytepos32 = 0xffff;

    /* Setup the bitoffset. */
    globs->bitoffs = o_buf;

    
    bf_incBitpos (o_buf, globs);

    globs->bitbuf[globs->bytepos] = 0;

    globs->ccd_recurs_level = 0;

    globs->CCD_Error = ccdOK;

    globs->continue_array = TRUE;

    if (mId NEQ 0xff AND mId NEQ 0xfe)
      theMsgId = mId;
    else
    {
      theMsgId = globs->pstruct[0];
    }

    mcompRef = ccddata_get_mmtx(entity,theMsgId,direction);
    /* Check the validity of the given message identifier. */
    if (theMsgId > max_message_id OR mcompRef EQ NO_REF)
    {
  #ifdef ERR_TRC_STK_CCD
      globs->ccd_recurs_level = 255;
  #endif /* ERR_TRC_STK_CCD */
      ccd_setError (globs, ERR_INVALID_MID, BREAK, (USHORT) theMsgId, (USHORT) -1);
      ccd_FreeGlobVars (globs);
      ccd_err_free (eentry);
      return (BYTE) globs->CCD_Error;
    }

  #ifdef ERR_TRC_STK_CCD
    /* save the value for tracing in error case */
    globs->error_stack[0] = mcompRef;
  #endif /* ERR_TRC_STK_CCD */

  maxBytes = (*l_buf + 7)>>3;
  globs->msgLen = *l_buf;

  #ifdef DEBUG_CCD
    TRACE_CCD (globs, "-------------------------------------------------");
    TRACE_CCD (globs, "CCD: Code Message");
    TRACE_CCD (globs, "Cleaning %d bits (%d bytes) of the bitstream",
      mcomp[mcompRef].bSize, maxBytes);
  #endif

    /* 
     * Clean up the bit buffer for the encoded message before encoding.
     */
    memset ((U8 *) &buf[(o_buf>>3)], 0, (size_t) maxBytes);
    /* Store the length of ereased buffer to support error handling. */
    globs->buflen = *l_buf;

    if (mId EQ 0xff)
    {
      /* Write the message identifier. */
      bf_writeBits ((U32)mi_length[entity], globs);
    }

  #ifdef DEBUG_CCD
  #ifdef CCD_SYMBOLS
    TRACE_CCD (globs, "CCD encode: Message = %s", mcomp[mcompRef].name);
  #else
    TRACE_CCD (globs, "CCD encode: MessageId = %x", theMsgId);
  #endif
  #endif

    /*
     * Clear the UPN stack.
     */
    ST_CLEAR(globs);
    memset ((ULONG *) &(globs->KeepReg[0]), 0, MAX_KEEP_REG_SIZE);

    /*
     * Inform the GSM-CODEC about the begin of a new message.
     */
    cdc_GSM_start (globs);

    jmp_ret = setjmp (globs->jmp_mark);

    if (jmp_ret EQ 0)
    {
      /*
       * no call to setjmp() done. So ccd_Error performs no longjmp in
       * case of an error
       */
      globs->jmp_mark_set = TRUE;
      ccd_encodeComposition ((ULONG) mcompRef, globs);
      if (globs->bitpos > o_buf + *l_buf)
      {
	ccd_setError (globs, ERR_BUFFER_OF, CONTINUE, (USHORT) -1);
      }
      bf_writePadBits (globs);
    }

    *l_buf = (USHORT) globs->bitpos - (USHORT) o_buf;

  #ifdef DEBUG_CCD
  {
    int i, j, buflen;
    char s[64], c[4];  

    buflen = (*l_buf + o_buf + 7) >> 3;

    TRACE_CCD (globs, "-------------------------------------------------");
    TRACE_CCD (globs, " After ENCODING: lbuf= %d, obuf= %d", *l_buf, o_buf);
    TRACE_CCD (globs, " Hex dump of encoded message:"); 

    s[0] = '\0';
    for (i = o_buf >> 3; i < buflen; i+=16)
    {
      for (j = 0; j < 16; j++)
      {
	if ((i+j) < buflen)
	{
	  sprintf(c, " %02x", buf[i+j]);
	  strcat (s, c);
	}
      }
      TRACE_CCD (globs, "%s", s);
      s[0] = '\0';
    }
  }
  #endif

  #ifdef DEBUG_CCD
    TRACE_CCD (globs, "CCD-ERROR = %d", globs->CCD_Error);
    TRACE_CCD (globs, "-------------------------------------------------");
  #endif

    ccd_FreeGlobVars (globs);
    ccd_err_free (eentry);

    return (BYTE)globs->CCD_Error;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
  /*
  +--------------------------------------------------------------------+
  | PROJECT : CCD (6144)               MODULE  : CCD                   |
  | STATE   : code                     ROUTINE : ccd_codeMsg           |
  +--------------------------------------------------------------------+

   PARAMETERS:   UBYTE      entity
		  - specifies the calling entity of CCD. The Constants
		    for each valid entity is defined in MCONST.CDG

		 UBYTE      direction
		  - specifies wether the message goes UPLINK or DOWNLINK.
		    This is nessesary because there are same PDU-Type
		    codes for different messages.

		 T_MSGBUF * mBuf
		  - specifies the bitstream buffer of the message. The
		    struct contains the l_buf and the o_buf elements.
		    These elements specifies the length and offset in bits
		    of the bitstream in the T_MSGBUF component buf.
		    The o_buf component must be specified by the caller,
		    the l_buf component is calculated by CCD.

		 UBYTE    * mStruct
		  - reference to the C-Structure containing the
		    C-Representation of the decoded message.
		    The type should be casted to UBYTE*. If this parameter
		    is NULL CCD uses his internal buffer wich must be
		    protected via ccd_begin() in a multithread environment.

		 UBYTE      mId
		  - specifies the PDU-Type of the bitstream. If this
		    parameter is not equal 0xff the CCD does not read
		    the pdu-type from the structure component pt
		    to decide wich decoding rules to select.
		    Normaly this param is set to 0xff.

   PURPOSE:      encodes a C-Structure containing the C-Representation of
		 a valid Air-interface message to a bitstream.

  */

BYTE CCDDATA_PREF(ccd_codeMsg) (UBYTE  entity,
		    UBYTE         direction,
		    T_MSGBUF     *mBuf,
		    UBYTE        *mStruct,
		    UBYTE         mId)
{
  return CCDDATA_PREF(ccd_codeMsgPtr) (entity, direction, &mBuf->l_buf,
      mBuf->o_buf, mBuf->buf, mStruct, mId);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function    : ccd_init_ccddata
+------------------------------------------------------------------------------
|  Description : Initialize local tables.
|
|  Parameters  : -
|
|  Return      : ccdOK, ccdWarning, or ccdError depending on the success.
+------------------------------------------------------------------------------
*/
ULONG CCDDATA_PREF(ccd_init_ccddata) (void)
{
  const T_CCD_CompTabEntry *msg;
  USHORT                    mcompRef;  
  UBYTE                     ret, entity, num_of_entities;
  USHORT                    messageId;
#ifdef DEBUG_CCD
  T_CCD_Globs *globs = &globs_all;
#endif

  aim_rrc_rcm = (U8)ccddata_get_ccdent ("UMTS_AS_ASN1_MSG");
  aim_rrlp    = (U8)ccddata_get_ccdent ("RRLP_ASN1_MSG");
  aim_sat     = (U8)ccddata_get_ccdent ("SAT");

  mcomp = ccddata_get_mcomp (0);
  mvar = ccddata_get_mvar (0);
  mval = ccddata_get_mval (0);
  melem = ccddata_get_melem (0);
  spare = ccddata_get_spare (0);
  calc = ccddata_get_calc (0);
  calcidx = ccddata_get_calcidx (0);

  mi_length = ccddata_get_mi_length();
  ccd_decMsgBuffer = ALIGN_BUF(ccddata_get_decmsgbuffer());
  max_message_id = (USHORT) ccddata_get_max_message_id();

#ifdef CCD_SYMBOLS
  if (!ccddata_mccd_symbols())
  {
#ifdef CCD_TEST
    printf ("CCD_SYMBOLS is not set in ccddata\n");
#else     /* CCD_TEST */
    vsi_o_assert( VSI_CALLER OS_SYST_ERR, __FILE__, __LINE__,
                  "CCD_SYMBOLS is not set in ccddata" );
#endif    /* CCD_TEST */
  }

#else     /* CCD_SYMBOLS */
  if (ccddata_mccd_symbols())
  {
#ifdef CCD_TEST
    printf ("Undefine CCD_SYMBOLS in ccddata\n");
#else     /* CCD_TEST */
    vsi_o_assert( VSI_CALLER OS_SYST_ERR, __FILE__, __LINE__,
                  "Undefine CCD_SYMBOLS in ccddata" );
#endif    /* CCD_TEST */
  }
#endif    /* CCD_SYMBOLS */

#ifdef DEBUG_CCD
  /* Only for TRACE_CCD call in ccd_init. */
  globs->me = 0;
  globs->TraceIt = 1;
#endif /* DEBUG_CCD */

  num_of_entities = (UBYTE) ccddata_get_num_of_entities();
  for (entity = 0; entity < num_of_entities; entity++)
  {
    /*
     * Point to the first defined Message, to get the length
     * of the message identifier
     */
    msg = NULL;
    messageId = 0;
    while (msg EQ NULL AND messageId <= max_message_id)
    {
      /* 
       * If there is no UPLINK-decoding, try the DOWNLINK.
       */
      if ((mcompRef = ccddata_get_mmtx(entity, messageId, UPLINK)) NEQ NO_REF)
        msg = (T_CCD_CompTabEntry *) &mcomp[mcompRef];
      else
        if ((mcompRef = ccddata_get_mmtx(entity, messageId, DOWNLINK)) NEQ NO_REF)
          msg = (T_CCD_CompTabEntry *) &mcomp[mcompRef];
        else
          messageId++;
    }
    if (msg NEQ NULL
    AND melem[msg->componentRef].elemType EQ 'V'
    AND melem[msg->componentRef].elemRef NEQ NO_REF)
    {
      /*
       * if there is a message for this layer - get the length
       * of the first element (msg_type or pdu_type)
       */
      mi_length[entity] =(UBYTE) (mvar[melem[msg->componentRef].elemRef].bSize); 
    }
    else
      mi_length[entity] = 0;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "MI-LEN [ENTITY %d] = %d", entity, mi_length[entity]);
#endif
  }

  /*
   * Register all needed coding/decoding functions in the jump table.
   */
  ret = cdc_init (codec);
    
  if (ret EQ ccdError)
    return ccdError;

#ifdef DEBUG_CCD
  if (ret EQ ccdWarning)
  {
    TRACE_CCD (globs, "CCD: Mismatch between CCD and CCDDATA. Check the codec list.");//return ccdWarning;
  }
#endif

  return ccdOK;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_signup
+------------------------------------------------------------------------------
|  Description  :  This function sets up the local CCD data for the calling
|                  entity.
|
|  Parameters   :  ccd_reg - set if called by ccd_register, not set if called
|                            by ccd_init
|                  decmsgbuf_size - further enhancement: size of the entity's
|                                   decoded msg buffer size
|
|  Return       :  ccdErr or ccdOK depending on the success
+------------------------------------------------------------------------------
*/

static int ccd_signup (int ccd_reg, int decmsgbuf_size)
{
#ifndef _TOOLS_
    UBYTE                     ret;
#endif
  #ifdef SHARED_CCD
    T_HANDLE me;
  #else
    int me = 0;
  #endif
  T_CCD_TASK_TABLE* tentry;

#ifdef SHARED_CCD
  me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
  {
    me = 0;
    tentry = ccd_task_list[0] = &task_null;
    task_null.ccd_globs = &globs_all;
#ifdef DEBUG_CCD
    TRACE_CCD (&globs_all, "Ccd initialization: task could not be identified. Try to continue with a non reentrant init");
#endif
  }
  else
  {
    if (!ccd_task_list[me])
    {
      ccd_task_list[me] = D_ALLOC (sizeof (T_CCD_TASK_TABLE));
      if (!ccd_task_list[me])
      {
        vsi_o_assert( VSI_CALLER OS_SYST_ERR, __FILE__, __LINE__,
                  "Could not allocate memory for ccd task table entry" );
      }
      else
      {
        ccd_task_list[me]->ccd_globs = NULL;
        ccd_task_list[me]->ccd_err_list = NULL;
        ccd_task_list[me]->ccd_store = NULL;
      }
    }
    tentry = ccd_task_list[me];
    tentry->decmsgbuf = NULL;
    if (ccd_reg)
    {
      if (!tentry->ccd_globs)
      {
        if (decmsgbuf_size != CCD_REENTRANT)
        {
#ifdef DEBUG_CCD
          TRACE_CCD (tentry->ccd_globs, "Ccd_register (task %d): ignore %d for parameter decmsgbuf_size. Make non reentrant ccd_init instead.", me, decmsgbuf_size);
#endif
          tentry->ccd_globs = &globs_all;
        }
        else
        {
          tentry->ccd_globs = D_ALLOC (sizeof(T_CCD_Globs));
          if (!tentry->ccd_globs)
          {
            vsi_o_assert( VSI_CALLER OS_SYST_ERR, __FILE__, __LINE__,
                      "Could not allocate memory for ccd_globs" );
          }
        }
      }
    }
    else
    {
      tentry->ccd_globs = &globs_all;
    }
  }
#else   /* SHARED_CCD */
  tentry = ccd_task_list[0] = &task_null;
  task_null.ccd_globs = &globs_all;
#endif

  tentry->ccd_globs->me = me;
  if (ccd_err_init (&tentry->ccd_err_list))
  {
  #ifdef CCD_TEST
    printf ("Cannot initialize error list: out of memory\n");
  #else   /* CCD_TEST */
    vsi_o_assert( VSI_CALLER OS_SYST_ERR, __FILE__, __LINE__,
	  "Cannot initialize error list: out of memory" );
  #endif  /* CCD_TEST */
    return ccdError;
  }

  if (ccd_store_init (&tentry->ccd_store))
  {
#ifdef CCD_TEST
    printf ("Cannot initialize store register: out of memory\n");
#else   /* CCD_TEST */
    vsi_o_assert( VSI_CALLER OS_SYST_ERR, __FILE__, __LINE__,
	  "Cannot initialize store register: out of memory" );
#endif  /* CCD_TEST */
    return ccdError;
  }

  if (!initialized)
  {
#ifdef SHARED_CCD
   /*
   * if the CCD is used in a premptive multithreaded system
   * we must create a semaphore for the coding and decoding
   */
    semCCD_Codec  = vsi_s_open (VSI_CALLER "CCD_COD",1);
    semCCD_Buffer = vsi_s_open (VSI_CALLER "CCD_BUF",1);

#endif     /* SHARED_CCD */
  
#ifndef _TOOLS_
    ret = (UBYTE)ccd_init_ccddata ();
    if (ret != ccdOK)
    return  ret;

#endif /* !_TOOLS_ */
    initialized = TRUE;
    /* save memory init pattern */
    mempat = (U8*) CCDDATA_PREF(ccd_get_numFaults ());
  }
  return ccdOK;
}

BYTE CCDDATA_PREF(ccd_init) (void)
{
  return (BYTE) ccd_signup (0, 0);
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccd_register
+------------------------------------------------------------------------------
|  Description  :  This function sets up the local CCD data for the calling
|                  entity.
|                  Entities calling this function with a parameter 0 will
|                  get an own set of CCD local data, i.e., they don't have to
|                  synchronize with other entities to use CCD.     
|
|  Parameters   :  decmsgbuf_size - further enhancement: size of the entity's
|                                   decoded msg buffer size
|
|  Return       :  ccdErr or ccdOK depending on the success
+------------------------------------------------------------------------------
*/

int ccd_register (int decmsgbuf_size)
{
  return ccd_signup (1, decmsgbuf_size);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : ccd_exit              |
+--------------------------------------------------------------------+

 PURPOSE:  performs a shutdown of CCD.

*/

int CCDDATA_PREF(ccd_exit) (void)
{
#ifdef SHARED_CCD
  T_CCD_TASK_TABLE* tentry;
  T_HANDLE me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    return ccdError;
  tentry = ccd_task_list[me]; 
  if (tentry->ccd_globs && (tentry->ccd_globs != &globs_all))
  {
    D_FREE (tentry->ccd_globs);
    tentry->ccd_globs = 0;
  }
  ccd_store_exit();
  ccd_err_exit (); 
  D_FREE (ccd_task_list[me]);
  ccd_task_list[me] = NULL;
#else
  ccd_store_exit();
  ccd_err_exit (); 
#endif /* SHARED_CCD */
  return ccdOK;
}
#endif /* !RUN_FLASH */
