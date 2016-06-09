/* 
+----------------------------------------------------------------------------- 
|  Project : 
|  Modul   : Ccdedit.c
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
|  Purpose :  Coder Decoder editfunctions for reading/writing the
|             	           C-Structures of primitives and messages.
+----------------------------------------------------------------------------- 
*/ 

#define CCDEDIT_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * standard definitions like UCHAR, ERROR etc.
 */
#include "typedefs.h"


/*
 * Declaration of coder/decoder-tables
 */
#include "ccdtable.h"
#include "ccddata.h"

#include "ccdedit.h"

typedef union
{
  UBYTE  buffer[4];
  UBYTE  b[4];
  USHORT s[2];
  ULONG  l;
} T_CONV;

typedef enum
{
  isvar,
  isstruct,
  isunion,
  issdu,
  isductrl
} T_ELEMTYPE;

/*
 *  strncpy() does not append a null chararcter to the copied string;
 *  This macro adds a terminating null following a call to strncpy()
 *  In Ccdedit.h the buffers are all len+1 bytes long.
 */
#define STRNCPY(dest,source,len)  {\
                                    strncpy (dest, source, len);\
                                    dest [len] = 0;\
                                  }


static const T_CCD_CompTabEntry* mcomp;
static const T_CCD_CompTabEntry* pcomp;
static const T_CCD_VarTabEntry*  pvar;
static const T_CCD_ElemTabEntry* pelem;
static const T_CCD_StrTabEntry*  pstr;
static const T_CCD_VarTabEntry*  mvar;
static const T_CCD_ElemTabEntry* melem;
static const T_CCD_StrTabEntry*  mstr;
static const T_CCD_ValTabEntry*  mval;
static const T_CCD_ValTabEntry*  pval;
static int ccddata_num_of_entities;
static int ccddata_max_message_id;
static int ccddata_max_primitive_id;
static int ccddata_max_sap_num;


void CCDDATA_PREF(cde_init) ()
{
  mcomp = ccddata_get_mcomp (0);
  pcomp = ccddata_get_pcomp (0);
  pvar = ccddata_get_pvar (0);
  pelem = ccddata_get_pelem (0);
  pstr = ccddata_get_pstr (0);
  mvar = ccddata_get_mvar (0);
  melem = ccddata_get_melem (0);
  mstr = ccddata_get_mstr (0);
  mval = ccddata_get_mval (0);
  pval = ccddata_get_pval (0);
  ccddata_num_of_entities = ccddata_get_num_of_entities ();
  ccddata_max_message_id = ccddata_get_max_message_id ();
  ccddata_max_primitive_id = ccddata_get_max_primitive_id ();
  ccddata_max_sap_num = ccddata_get_max_sap_num ();
}

/*
+------------------------------------------------------------------------------
|  Function     :  cde_val_iterate
+------------------------------------------------------------------------------
|  Description  :  This function searches the values in [pm]val.cdg for
|                  a given value and depending on the value of the parameter
|                  'copy' adds the "Comment" of the SAP-/MSG-catalogues
|                  to the member symbolicValue of a given T_CCDE_ELEM_DESCR.
|
|  Parameters   :  elem_value - the actual value searched for
|                  edescr - the element descriptor
|                  copy - s.a.
|
|  Return       :  -1 if no values are defined or if the value is not found;
|                  else: the ordinal of the value in relation to first valid
|                    value of the var
+------------------------------------------------------------------------------
*/

static void cde_val_iterate (int elem_value,
                              T_CCDE_ELEM_DESCR* edescr)
{
  
  S32  StartVal, EndVal;
  BOOL   IsDefault;
  char  *ValStr;
  SHORT  NumDefs;
  USHORT ValueDef;
  USHORT valdefstart;
  const T_CCD_ValTabEntry* val;
  const T_CCD_StrTabEntry* str;

  if (edescr->ccdIndex == NO_REF)
    return;

  if (edescr->esource EQ FromMsg)
  {
    NumDefs  = mvar[edescr->ccdIndex].numValueDefs;
    ValueDef = mvar[edescr->ccdIndex].valueDefs;
    val = mval;
    str = mstr;
  }
  else
  {
    NumDefs  = pvar[edescr->ccdIndex].numValueDefs;
    ValueDef = pvar[edescr->ccdIndex].valueDefs;
    val = pval;
    str = pstr;
  }

  valdefstart = ValueDef;

  edescr->valcheck = NumDefs ? -1 : 1;
  while (NumDefs-- > 0)
  {
    IsDefault = val[ValueDef].isDefault;
    ValStr    = str[val[ValueDef].valStringRef];
    StartVal  = val[ValueDef].startValue;
    EndVal    = val[ValueDef].endValue;

    if (IsDefault)
    {
      /* default definition */
      
      STRNCPY (edescr->symbolicValue, ValStr, SYMBOLIC_VAL_LENGTH);
      /* 
       * If IsDefault is 2 it is an ASN1 default value; StartVal and EndVal
       * are set to the value. If IsDefault is 1, it means only a default
       * symbolic value, but StartVal and EndVal are not set. In this case
       * valcheck get the value 0.
       */
      if (IsDefault == 2)
      {
        if (elem_value == StartVal)
        {
          edescr->valcheck = 1;
          return;
        }
      }
      else
      {
        edescr->valcheck = 0;
      }
    }
    else
    {
      if (elem_value == StartVal && elem_value == EndVal)
      {
        STRNCPY (edescr->symbolicValue, ValStr, SYMBOLIC_VAL_LENGTH);
        edescr->valcheck = 1;
        return;
      }

      if (elem_value >= StartVal AND elem_value <= EndVal)
      {
        /* found in range, but continue to search an exact match */
        STRNCPY (edescr->symbolicValue, ValStr, SYMBOLIC_VAL_LENGTH);
        edescr->valcheck = 1;
      }
    }
    ValueDef++;
  }
}

static void eval_elemtype (T_CCDE_ELEM_DESCR* edescr,
                           const T_CCD_ElemTabEntry* elem,
                           T_ELEMTYPE* elemtype,
                           BOOL* linked)
{
  *linked = FALSE;

  switch (elem->elemType)
  {
    case 'W':
    case 'M':
    case 'I':
      *linked = TRUE; 
      /* fallthrough */
    case 'V':
    case 'R':
    case 'F':
     *elemtype = isvar;
     break;

    case 'Z':
    case 'K':
    case 'G':
      *linked = TRUE;
      /* fallthrough */
    case 'C':
    case 'P':
    case 'D':
     *elemtype = isstruct;
     break;

    case 'c':
    case 'p':
    case 'd':
     *elemtype = issdu;
     break;

    case 'Y':
    case 'L':
    case 'H':
      *linked = TRUE;
      /* fallthrough */
    case 'U':
    case 'Q':
    case 'E':
     *elemtype = isunion;
     break;
    case '!':
     *elemtype = isductrl;
     break;
  }
  if ((elem->elemType >= 'P' && elem->elemType <= 'R') ||
      (elem->elemType >= 'K' && elem->elemType <= 'M') ||
       elem->elemType == 'p')
    edescr->ptrtype = usptr;
  else if((elem->elemType >= 'D' && elem->elemType <= 'F') ||
          (elem->elemType >= 'G' && elem->elemType <= 'I') ||
           elem->elemType == 'd')
    edescr->ptrtype = ctptr;
  else
    edescr->ptrtype = noptr;
}


/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_get_next_elem   |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

static USHORT cde_get_next_elem (T_CCDE_HANDLE       *handle,
                                 UBYTE                descent,
                                 T_CCDE_ELEM_DESCR   *edescr)
{
  T_CCDE_CONTEXT * ctx;
  BOOL isMsg;
  BOOL linked;
  BOOL             validElemFound = FALSE;
  const T_CCD_VarTabEntry   *var;
  const T_CCD_CompTabEntry  *comp;
  const T_CCD_ElemTabEntry  *elem;
  const T_CCD_StrTabEntry   *str;
  T_ELEMTYPE elemtype;

  isMsg = handle->source == FromMsg;

  if (isMsg)
  /* var, str, and comp may become reset later */
  {
    comp = mcomp;
    elem = melem;
    str = mstr;
    var = mvar;
  }
  else
  {
    comp = pcomp;
    elem = pelem;
    str = pstr;
    var = pvar;
  }
  
  if (descent > handle->level AND handle->canDescent)
    handle->level++;

  handle->canDescent = FALSE;
  
  ctx = &handle->context[handle->level];

  do
  {
    if (ctx->numElems EQ 0)
    {
      /*
       * end of composition or maybe of the entire message/primitive.
       */
      if (handle->level > 0)
      {
        /*
         * end of substructured element.
         * switch to the context of the previous level.
         */
        handle->level--;
        ctx = &handle->context[handle->level];
      }
      else
      {
        edescr->offset = comp[ctx->structIdx].cSize
                         + handle->lenVarPart;
        return isMsg ? CCDEDIT_END_OF_MSG : CCDEDIT_END_OF_PRIM;
      }
    }
    /*
     * skip the spare elements (do it only for messages)
     */
    if (ctx->numElems)
    {
      /* 
       * remember: primitives does not contain spare definitions
       */
      if (elem[ctx->elemIdx].elemType == 'S')
      {
        ctx->elemIdx++;
        ctx->numElems--;
      }
      else
        validElemFound = TRUE;
    }
  } while (!validElemFound);


  eval_elemtype (edescr, &elem[ctx->elemIdx], &elemtype, &linked);

  if (elemtype == isductrl)
  {
    edescr->btype = T_ductrl;
    ctx->elemIdx++;
    ctx->numElems--;
    return CCDEDIT_OK;
  }

  if (linked)
  {
    /* element linked from pelem to mvar/mcomp */
    edescr->esource = FromMsg;
    comp = mcomp;
    var = mvar;
    str = mstr;
  }
  else
    edescr->esource = handle->source;

  if (ctx->state EQ TRAVERSE_ARRAY)
  {
    /*
     * for every array element calculate the offset for the
     * C-structure access.
     * offset = leveloffset + (arrayIndex * csize) + 1
     */
	  edescr->offset = elem[ctx->elemIdx].structOffs + ctx->levelOffset
                   + (ctx->arrayIndex
                      * ((elem[ctx->elemIdx].elemType NEQ 'C') 
                         ? var[elem[ctx->elemIdx].elemRef].cSize
                         : comp[elem[ctx->elemIdx].elemRef].cSize
                        )
                     );
/*
                   + 1;
  */
  }
  else
  {
	  edescr->offset = elem[ctx->elemIdx].structOffs
                   + ctx->levelOffset;
  }

  edescr->level        = handle->level;
  edescr->maxRepeat    = elem[ctx->elemIdx].maxRepeat;
  edescr->index        = NO_REF;
  edescr->ccdIndex     = NO_REF;
  edescr->validRepeats = NO_REF;
  edescr->isOptional   = elem[ctx->elemIdx].optional;
  edescr->arrayType    = NoArray;
  edescr->elemref      = NO_REF;
  edescr->u_member     = FALSE;
  edescr->u_ctrl       = 0xffffffff;
  edescr->bitstring    = 0;
  edescr->c_implicit   = 1;
  edescr->issigned     = 0;
  edescr->valcheck     = 0;

  if ( edescr->maxRepeat > 0
   && elem[ctx->elemIdx].repType != 'b'
   && elem[ctx->elemIdx].repType != 's'
   && elem[ctx->elemIdx].elemType != 'E'
   && ctx->state == TRAVERSE_STRUCTURE)
  {
    edescr->arrayType = (   elem[ctx->elemIdx].repType == 'v'
                         || elem[ctx->elemIdx].repType == 'i'
                         || elem[ctx->elemIdx].repType == 'J'
                         || elem[ctx->elemIdx].repType == 'j') 
                        ? VarArray
                        : FixArray;

    if (elem[ctx->elemIdx].repType == 'C'
        || elem[ctx->elemIdx].repType == 'J') 
      edescr->bitstring = 1;

    if (elem[ctx->elemIdx].repType == 'j'
        || elem[ctx->elemIdx].repType == 'J') 
      edescr->c_implicit = 0;

    if (handle->level < MAX_LEVELS)
    {
      T_CCDE_CONTEXT * new_ctx = ctx+1;

      ctx->repeats        = edescr->maxRepeat;

      handle->canDescent = TRUE;

	    new_ctx->structIdx      = ctx->structIdx;

	    new_ctx->elemIdx        = ctx->elemIdx;
	    new_ctx->elemType       = 0;
      new_ctx->arrayIndex     = 0;
	    new_ctx->numElems       = edescr->maxRepeat;
      new_ctx->levelOffset    = ctx->levelOffset;
      new_ctx->arrayType      = edescr->arrayType;
      new_ctx->state          = TRAVERSE_ARRAY;
      /*
       * if the composition is optional, increment the offset
       * because of the valid flag (v_xxx).
       */
      if (edescr->isOptional)
        new_ctx->levelOffset++;
      /*
       * if the composition is a array with variable size,
       * increment the offset because of the counter (c_xxx).
       */
      if (edescr->arrayType EQ VarArray)
        new_ctx->levelOffset += edescr->maxRepeat >> 8 ? 2 : 1;
    }
  }

  if (ctx->state EQ TRAVERSE_ARRAY)
  {
    /*
     * if the size of the array is variable, mark the
     * components of this array as optional. So we can later
     * determine if the array component is valid
     */
    if (ctx->arrayType EQ VarArray)
      edescr->isOptional = TRUE;
    /*
     * increment the array index
     */
    edescr->index = ctx->arrayIndex++;
  }

  if (elemtype == isvar)
  {
    /*
     * basic element (var)
     */
    switch (var[elem[ctx->elemIdx].elemRef].cType)
    {
      case 'C':
        edescr->issigned = 1;
        /* fallthrough */
      case 'B':
        edescr->btype = T_byte;
        break;
      case 'T':
        edescr->issigned = 1;
        /* fallthrough */
      case 'S':
        edescr->btype = T_short;
        break;
      case 'M':
        edescr->issigned = 1;
        /* fallthrough */
      case 'L':
        edescr->btype = T_long;
        break;
      case 'X':
        edescr->btype = T_buffer;
        break;
    }
    edescr->bytelen = var[elem[ctx->elemIdx].elemRef].cSize;

#ifdef CCD_SYMBOLS
    strcpy (edescr->aname, var[elem[ctx->elemIdx].elemRef].name);
    strcpy (edescr->sname, ccddata_get_alias (ctx->elemIdx, (int) isMsg));
    if (edescr->sname[0] == '\0')
      strcpy (edescr->sname, var[elem[ctx->elemIdx].elemRef].name);
    STRNCPY (edescr->lname, str[var[elem[ctx->elemIdx].elemRef].longNameRef],
             LONG_NAME_LENGTH);
#else
    strcpy (edescr->sname, "No name info avail.");
    strcpy (edescr->aname, "No name info avail.");
    strcpy (edescr->lname, "No name info avail.");
#endif
  }
  else if (elemtype == isunion)
  {
    /* union */
    edescr->btype = T_union;
    edescr->bytelen = comp[elem[ctx->elemIdx].elemRef].cSize;
    edescr->elemref = elem[ctx->elemIdx].elemRef;

#ifdef CCD_SYMBOLS
    strcpy (edescr->aname, comp[elem[ctx->elemIdx].elemRef].name);
    strcpy (edescr->sname, ccddata_get_alias (ctx->elemIdx, (int) isMsg));
    if (edescr->sname[0] == '\0')
      strcpy (edescr->sname, comp[elem[ctx->elemIdx].elemRef].name);
    STRNCPY (edescr->lname, str[comp[elem[ctx->elemIdx].elemRef].longNameRef],
            LONG_NAME_LENGTH);
#else
    strcpy (edescr->sname, "No name info avail.");
    strcpy (edescr->aname, "No name info avail.");
    strcpy (edescr->lname, "No name info avail.");
#endif
  }
  else
  {
    /*
     * substructured info element
     */
    if (elemtype == issdu)
      edescr->btype = T_issdu;
    else
      edescr->btype = T_struct;
    edescr->bytelen = comp[elem[ctx->elemIdx].elemRef].cSize;
    edescr->elemref = elem[ctx->elemIdx].elemRef;

#ifdef CCD_SYMBOLS
    strcpy (edescr->aname, comp[elem[ctx->elemIdx].elemRef].name);
    strcpy (edescr->sname, ccddata_get_alias (ctx->elemIdx, (int) isMsg));
    if (edescr->sname[0] == '\0')
      strcpy (edescr->sname, comp[elem[ctx->elemIdx].elemRef].name);
    STRNCPY (edescr->lname, str[comp[elem[ctx->elemIdx].elemRef].longNameRef],
            LONG_NAME_LENGTH);
#else
    strcpy (edescr->sname, "No name info avail.");
    strcpy (edescr->aname, "No name info avail.");
    strcpy (edescr->lname, "No name info avail.");
#endif
    if (edescr->arrayType EQ NoArray
      AND handle->level < MAX_LEVELS)
    {
      T_CCDE_CONTEXT * new_ctx = ctx+1;
      
      handle->canDescent     = TRUE;

	    new_ctx->structIdx      = elem[ctx->elemIdx].elemRef;

	    new_ctx->elemIdx        = comp[new_ctx->structIdx].componentRef;
	    new_ctx->elemType       = 0;
      new_ctx->arrayIndex     = 0;
	    new_ctx->numElems       = comp[new_ctx->structIdx].numOfComponents;
      new_ctx->levelOffset    = edescr->offset;
      /*
       * if the composition is optional, increment the offset
       * because of the valid flag (v_xxx).
       */
      if (edescr->isOptional)
        new_ctx->levelOffset++;
      /*
       * if the composition is a array with variable size,
       * increment the offset because of the counter (c_xxx).
       */
      if (edescr->arrayType EQ VarArray)
        new_ctx->levelOffset++;

      new_ctx->state          = TRAVERSE_STRUCTURE;
    }
  }
  

  if (edescr->arrayType EQ NoArray && elem[ctx->elemIdx].elemType == 'V'
  AND  var[elem[ctx->elemIdx].elemRef].numValueDefs > 0)
  {
    /*
     * value definitions available
     * store the index of this information element in the
     * element descriptor for later value requests.
     */
    edescr->ccdIndex = elem[ctx->elemIdx].elemRef;
  }
    
  ctx->numElems--;
  
  if (ctx->state EQ TRAVERSE_STRUCTURE)
    ctx->elemIdx++;

  return CCDEDIT_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_prim_first      |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

USHORT CCDDATA_PREF(cde_prim_first) (T_CCDE_HANDLE    * phandle,
                       ULONG              primcode,
                       char             * name)
{
  USHORT SAP, Opcode, Direction, ThePrimitive;

  if (primcode & 0x80000000)
  {
    SAP       = (USHORT) (primcode & 0x3fff);
    Opcode    = (USHORT) ((primcode >> 16) & 0xff);
  }
  else
  {
    SAP       = (USHORT) (((primcode & 0x3f00)>>8) & 0xff);
    Opcode    = (USHORT) (primcode & 0xff);
  }
  Direction = (USHORT) (((primcode & 0x4000)>>14) & 0x01);

  if (SAP > ccddata_max_sap_num OR Opcode > ccddata_max_primitive_id)
    return CCDEDIT_PRIM_NOT_FOUND;

  if ((ThePrimitive = ccddata_get_pmtx(SAP,Opcode,Direction)) EQ NO_REF)
    return CCDEDIT_PRIM_NOT_FOUND;

  phandle->context[0].structIdx = ThePrimitive;

#ifdef CCD_SYMBOLS
  strcpy (name, pcomp[phandle->context[0].structIdx].name);
#else
  strcpy (name, "No name info avail.");
#endif

  phandle->level                  = 0;
	phandle->context[0].elemIdx     = pcomp[phandle->context[0].structIdx].componentRef;
	phandle->context[0].elemType    = 0;
	phandle->context[0].numElems    = pcomp[phandle->context[0].structIdx].numOfComponents;
  phandle->context[0].levelOffset = 0;
  phandle->context[0].arrayIndex  = 0;
  phandle->context[0].state       = TRAVERSE_STRUCTURE;
  phandle->canDescent             = FALSE;
  phandle->source                 = FromPrim;
	phandle->maxCSize               = pcomp[phandle->context[0].structIdx].cSize;
  phandle->lenVarPart             = 0;

  return CCDEDIT_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_prim_next       |
+--------------------------------------------------------------------+

  PURPOSE : 

*/


USHORT CCDDATA_PREF(cde_prim_next) (T_CCDE_HANDLE      *phandle,
                      UBYTE               descent,
                      T_CCDE_ELEM_DESCR  *pdescr)
{
  return cde_get_next_elem (phandle,
                            descent,
                            pdescr);
}

/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_msg_first       |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

USHORT CCDDATA_PREF(cde_msg_first) (T_CCDE_HANDLE  * mhandle,
                      UBYTE            type,
                      UBYTE            direction,
                      UBYTE            entity,
                      char           * name)

{
  USHORT TheMessage;

  if (entity > ccddata_num_of_entities OR type > ccddata_max_message_id)
    return CCDEDIT_MESSAGE_NOT_FOUND;

  if ((TheMessage = ccddata_get_mmtx((USHORT) entity,
                        (USHORT) type,
                        (USHORT) direction)) EQ NO_REF)
    return CCDEDIT_MESSAGE_NOT_FOUND;

  mhandle->context[0].structIdx = TheMessage;
  
#ifdef CCD_SYMBOLS
  strcpy (name, mcomp[mhandle->context[0].structIdx].name);
#else
  strcpy (name, "No name info avail.");
#endif

  mhandle->level                  = 0;
	mhandle->context[0].elemIdx     = mcomp[mhandle->context[0].structIdx].componentRef;
	mhandle->context[0].elemType    = 0;
	mhandle->context[0].numElems    = mcomp[mhandle->context[0].structIdx].numOfComponents;
  mhandle->context[0].levelOffset = 0;
  mhandle->context[0].arrayIndex  = 0;
  mhandle->context[0].state       = TRAVERSE_STRUCTURE;
  mhandle->canDescent             = FALSE;
  mhandle->source                 = FromMsg;
	mhandle->maxCSize               = mcomp[mhandle->context[0].structIdx].cSize;
  mhandle->lenVarPart             = 0;

  return CCDEDIT_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_msg_next        |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

USHORT CCDDATA_PREF(cde_msg_next) (T_CCDE_HANDLE     *mhandle,
                     UBYTE              descent,
                     T_CCDE_ELEM_DESCR *iedescr)
{
  return cde_get_next_elem (mhandle,
                            descent,
                            iedescr);
}


/*
+------------------------------------------------------------------------------
|  Function     :  cde_get_comp
+------------------------------------------------------------------------------
|  Description  :  This function works with similar results like cde_comp_first,
|                  but not the whole comp table is searched for the name of the
|                  component. Instead the previous set elemref in the
|                  parameter edescr is taken to directly jump to the component.
|                  The component found is compared with the given name in
|                  edescr. If equal chandle is defined. Otherwise there is an
|                  error.
|
|  Parameters   :  chandle - the handle for the component (returned)
|                  edescr - the element descriptor
|
|  Return       :  CCDEDIT_OK on success, CCDEDIT_COMP_NOT_FOUND otherwise
+------------------------------------------------------------------------------
*/

USHORT CCDDATA_PREF(cde_get_comp) (T_CCDE_HANDLE*     chandle,
                     T_CCDE_ELEM_DESCR* edescr)
{
  const T_CCD_CompTabEntry* comp;
  USHORT index = edescr->elemref;

  if (index == NO_REF)
    return CCDEDIT_COMP_NOT_FOUND;

  comp = edescr->esource == FromMsg ? &mcomp[index] : &pcomp[index];

#ifdef CCD_SYMBOLS
  if (strcmp (comp->name, edescr->aname))
    return CCDEDIT_COMP_NOT_FOUND;
#endif /* CCD_SYMBOLS */

    chandle->context[0].structIdx = index;
    chandle->level                  = 0;
    chandle->context[0].elemIdx     = comp->componentRef;
    chandle->context[0].elemType    = 0;
    chandle->context[0].numElems    = comp->numOfComponents;
    chandle->context[0].levelOffset = 0;
    chandle->context[0].arrayIndex  = 0;
    chandle->context[0].state       = TRAVERSE_STRUCTURE;
    chandle->canDescent             = FALSE;
    chandle->source                 = edescr->esource;
    chandle->maxCSize               = comp->cSize;
    chandle->lenVarPart             = 0;

    return CCDEDIT_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  cde_comp_alias
+------------------------------------------------------------------------------
|  Description  :  This function works with similar results like cde_comp_first,
|                  but not thewhole comp table is searched for the name of the
|                  component. Instead the alias name (as_name) from ?elem.cdg
|                  is taken for name comparison.
|
|  Parameters   :  chandle - the handle for the component (returned)
|                  source - if message or primitve
|                  name - the name of the searched component
|
|  Return       :  CCDEDIT_OK on success, CCDEDIT_COMP_NOT_FOUND otherwise
+------------------------------------------------------------------------------
*/
USHORT cde_comp_alias (T_CCDE_HANDLE      * chandle,
                              T_ELM_SRC            source,
                              char               * name)

{
  const T_CCD_CompTabEntry* comp;
  const T_CCD_ElemTabEntry* elem;
  int   found = 0;
  USHORT index, cindex;

  elem = source == FromMsg ? melem : pelem;

  index = 0;
  while (!found AND (ccddata_get_alias (index, source == FromMsg) != NULL))
  {
    /* name found */
    if (elem[index].elemType == 'C' &&
        !strcmp (ccddata_get_alias (index, source == FromMsg), name))
      found = 1;
    else
      index++;
  }
  if (!found)
    return CCDEDIT_COMP_NOT_FOUND;

  cindex = elem[index].elemRef;
  comp = source == FromMsg ? &mcomp[cindex] : &pcomp[cindex];
  chandle->context[0].structIdx = cindex;
  chandle->level                  = 0;
  chandle->context[0].elemIdx     = comp->componentRef;
  chandle->context[0].elemType    = 0;
  chandle->context[0].numElems    = comp->numOfComponents;
  chandle->context[0].levelOffset = 0;
  chandle->context[0].arrayIndex  = 0;
  chandle->context[0].state       = TRAVERSE_STRUCTURE;
  chandle->canDescent             = FALSE;
  chandle->source                 = source;
  chandle->maxCSize               = comp->cSize;
  chandle->lenVarPart             = 0;

  return CCDEDIT_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_comp_first      |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

USHORT CCDDATA_PREF(cde_comp_first) (T_CCDE_HANDLE      * chandle,
                       T_ELM_SRC            source,
                       char               * compname)

{
  USHORT index;
  BOOL   found = FALSE;

  if (source EQ FromMsg)
  {
    /*
     * search the mcomp-table for the given name
     */
    index = 0;
    while (!found AND mcomp[index].name NEQ NULL)
    {
      /*
       * composition name found
       */
      if (strcmp (mcomp[index].name, compname) EQ 0)
        found = TRUE;
      else
        index++;
    }
    if (found)
      chandle->context[0].structIdx = index;
    else
      return CCDEDIT_COMP_NOT_FOUND;

    chandle->level                  = 0;
	  chandle->context[0].elemIdx     = mcomp[index].componentRef;
	  chandle->context[0].elemType    = 0;
	  chandle->context[0].numElems    = mcomp[index].numOfComponents;
    chandle->context[0].levelOffset = 0;
    chandle->context[0].arrayIndex  = 0;
    chandle->context[0].state       = TRAVERSE_STRUCTURE;
    chandle->canDescent             = FALSE;
    chandle->source                 = FromMsg;
    chandle->maxCSize               = mcomp[index].cSize;
    chandle->lenVarPart             = 0;
  }
  else
  {
    /*
     * search the pcomp-table for the given name
     */
    index = 0;
    while (!found AND pcomp[index].name NEQ NULL)
    {
      /*
       * composition name found
       */
      if (strcmp (pcomp[index].name, compname) EQ 0)
        found = TRUE;
      else
        index++;
    }
    if (found)
      chandle->context[0].structIdx = index;
    else
      return CCDEDIT_COMP_NOT_FOUND;

    chandle->level                  = 0;
	  chandle->context[0].elemIdx     = pcomp[index].componentRef;
	  chandle->context[0].elemType    = 0;
	  chandle->context[0].numElems    = pcomp[index].numOfComponents;
    chandle->context[0].levelOffset = 0;
    chandle->context[0].arrayIndex  = 0;
    chandle->context[0].state       = TRAVERSE_STRUCTURE;
    chandle->canDescent             = FALSE;
    chandle->source                 = FromPrim;
    chandle->maxCSize               = pcomp[index].cSize;
    chandle->lenVarPart             = 0;
  }

  return CCDEDIT_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_comp_next       |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

USHORT CCDDATA_PREF(cde_comp_next) (T_CCDE_HANDLE     *chandle,
                      UBYTE              descent,
                      T_CCDE_ELEM_DESCR *descr)
{
  return cde_get_next_elem      (chandle,
                                 descent,
                                 descr);
}

/*
+------------------------------------------------------------------------------
|  Function     :  cde_get_symval
+------------------------------------------------------------------------------
|  Description  :  This function adds the "Comment" of the SAP-/MSG-catalogues
|                  to the member symbolicValue of a given T_CCDE_ELEM_DESCR.
|
|  Parameters   :  elem_value - the actual value for that the comment
|                               is searched for
|                  edescr - the element descriptor
|
|  Return       :  The string itself is returned which is a pointer to
|                  '\0' if no comment was defined for that value.
+------------------------------------------------------------------------------
*/

char* CCDDATA_PREF(cde_get_symval) (int elem_value, T_CCDE_ELEM_DESCR* edescr)
{
  edescr->symbolicValue[0] = '\0';
  
  cde_val_iterate (elem_value, edescr);

  return edescr->symbolicValue;
}

/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_read_elem       |
+--------------------------------------------------------------------+

  PURPOSE : Reads the value of the element, referenced by the element
            descriptor edescr, out of the C-Structure cstruct. The 
            value is stored in the memory area, addressed by the
            parameter value.

*/

USHORT CCDDATA_PREF(cde_read_elem) (T_CCDE_HANDLE     * handle,
                      void              * cstruct,
                      T_CCDE_ELEM_DESCR * edescr,
                      UBYTE             * value)
{
  T_CONV         * cvp;
  U32              voffset;
  ULONG            elem_value;
  UBYTE          * cs = (UBYTE *) cstruct;

  /* 
   * if this element is optional and it is no array component
   * read the valid flag out of the C-structure.
   */
  if (edescr->isOptional && edescr->index == NO_REF && edescr->ptrtype != usptr)
  {
    voffset = edescr->offset++;
    edescr->isValid = (cs[voffset] EQ TRUE);
  }
  else
  {
    if (edescr->index NEQ NO_REF)
    {
      T_CCDE_CONTEXT *last_ctx;

      last_ctx = &handle->context[handle->level-1];
      edescr->isValid = (edescr->index < last_ctx->repeats);
    }
    else
      edescr->isValid = TRUE;
  }

  if (!edescr->isValid)
    return CCDEDIT_OK;
   
  if (edescr->u_member)
  {
    edescr->u_ctrl = * (U32 *) &cs[edescr->offset];
    edescr->offset += sizeof (U32); 
  }

  if (edescr->arrayType NEQ NoArray)
  {
    T_CCDE_CONTEXT *ctx;

    ctx = &handle->context[handle->level];

    /*
     * array of message elements (info elements)
     */

    if (edescr->arrayType EQ VarArray)
    {
      USHORT sz_of_len;
      sz_of_len = edescr->maxRepeat >> 8 ? 2 : 1; /* 1 or 2 bytes for len */
      if (sz_of_len == 1)
      {
        ctx->repeats = (USHORT) cs[edescr->offset];
      }
      else
      {
        ctx->repeats = * (USHORT *) &cs[edescr->offset];
      }
      edescr->offset += sz_of_len;

      if (ctx->repeats > edescr->maxRepeat)
        ctx->repeats = edescr->maxRepeat;
    }
    else
      ctx->repeats = edescr->maxRepeat;

    edescr->bytelen = edescr->bytelen * ctx->repeats;
    edescr->validRepeats = ctx->repeats;
    *value++ = (UBYTE) edescr->validRepeats;
  }

  if (edescr->ptrtype != noptr)
  {
    cs = * (UBYTE **) &cs[edescr->offset];
    if (!cs)
    {
      edescr->isValid = FALSE;
      return CCDEDIT_OK;
    }
  }
  else
    cs += edescr->offset;

  /*
   * read the current value from the C-structure
   */
  if ((edescr->btype == T_issdu) || (edescr->btype == T_buffer))
  {
    USHORT l_buf, o_buf, len;
 
    /*
     * For the structure SDU perform a special handling.
     * The SDU contains l_buf and o_buf and the element
     * buf. This element is only defined as buf[1] because
     * the real length results of the encoded message and
     * must be calculated form l_buf and o_buf
     */

    /*
     * read l_buf and o_buf (length and offset) out of the struct
     */
    memcpy ((UBYTE *)&l_buf, cs, sizeof (USHORT));

    memcpy ((UBYTE *)&o_buf, cs+sizeof (USHORT), sizeof (USHORT));

    len = ((l_buf+o_buf+7)/8);
    handle->lenVarPart += len;
    handle->canDescent = FALSE;
    if ((edescr->btype == T_issdu) &&
        ((len > (U32)(ccddata_get_max_bitstream_len()/8)) ||
        (len > 0x1FFF))) /* max bytes: 0xFFFF/8 = 0x1FFF */
    {
      return CCDEDIT_MESSAGE_ERROR;
    }
    edescr->bytelen    = (2 * sizeof (USHORT)) + len;
  }
  
  memcpy (value, cs, edescr->bytelen);

  cvp = (T_CONV *) cs;
  
  switch (edescr->btype)
  {
    case T_byte:
      elem_value = (ULONG) cvp->b[0];
      break;
    case T_short:
      elem_value = (ULONG) cvp->s[0];
      break;
    case T_long:
      elem_value = (ULONG) cvp->l;
      break;
    default:
      return CCDEDIT_OK;
  }

  (void) CCDDATA_PREF(cde_get_symval) (elem_value, edescr);

  return CCDEDIT_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  cde_write_prepare
+------------------------------------------------------------------------------
|  Description  :  This function prepares the writing of elements, by setting
|                  valid flag, union controller and length of vaiable arrays
|                  if necessary. Current version: only valid flag and union
|                  controller.
|
|  Parameters   :  same as cde_write_elem except value
|
|  Return       :  -
+------------------------------------------------------------------------------
*/

void CCDDATA_PREF(cde_write_prepare) (T_CCDE_HANDLE     * handle,
                         void              * cstruct,
                         T_CCDE_ELEM_DESCR * edescr)
{
  UBYTE          * cs = (UBYTE *) cstruct;

  /* 
   * if this element is optional and it is no array component
   * set the corresponding valid flag in the C-structure.
   */
  if (edescr->isOptional && edescr->ptrtype != usptr)
  {
    cs[edescr->offset++] = TRUE;
  }

  if (edescr->u_member)
  {
    * (U32 *) &cs[edescr->offset] = (UBYTE) edescr->u_ctrl;
    edescr->offset += sizeof (U32); 
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_write_elem      |
+--------------------------------------------------------------------+

  PURPOSE : Write the value wich is stored in the memory area,
            addressed by the parameter value, into the C-Structure
            element, referenced by the element descriptor edescr.

*/

USHORT CCDDATA_PREF(cde_write_elem) (T_CCDE_HANDLE     * handle,
                       void              * cstruct,
                       T_CCDE_ELEM_DESCR * edescr,
                       UBYTE             * value)
{
  char  *cs = (UBYTE *) cstruct;
  char  *vb;
  U32   len;
   
  CCDDATA_PREF(cde_write_prepare) (handle, cs, edescr);

  if ((edescr->arrayType != NoArray) && (edescr->btype != T_struct)) 
  {
    T_CCDE_CONTEXT *ctx;
    T_CCDE_CONTEXT _ctx;

    ctx = handle ? &handle->context[handle->level] : &_ctx;

    /*
     * Array of message elements (info elements) or 
     * parameter.
     * In case of variable sized arrays, store the 
     * amount of elements into the corresponding c_xxx variable 
     */
    if (edescr->arrayType EQ VarArray)
    {
      /*
       * array with a variable number of elements
       * set the c_xxx variable in the C-Structure
       */
      USHORT sz_of_len;
      sz_of_len = edescr->maxRepeat >> 8 ? 2 : 1; /* 1 or 2 bytes for len */
      if (sz_of_len == 1)
      {
        cs[edescr->offset] = (UBYTE) edescr->validRepeats;
      }
      else
      {
        * (USHORT *) &cs[edescr->offset] = edescr->validRepeats;
      }
      edescr->offset += sz_of_len;
    }
    ctx->repeats = edescr->validRepeats; 
    if (edescr->bitstring)
    {
      ctx->repeats = (ctx->repeats+7)/8; 
    }

    edescr->bytelen = edescr->bytelen * ctx->repeats;
  }

  if (edescr->ptrtype != noptr)
  {
    char* pointer = value;
    vb = (char*) &pointer;
    len = sizeof (char*);
  }
  else
  {
    vb = (char*) value;
    len = edescr->bytelen;

    if ((edescr->btype == T_issdu) || (edescr->btype == T_buffer))
    {
      USHORT l_buf, o_buf;
   
      /*
       * For the structure SDU perform a special handling.
       * The SDU contains l_buf and o_buf and the element
       * buf. This element is only defined as buf[1] because
       * the real length results of the encoded message and
       * must be calculated form l_buf and o_buf
       */

      /*
       * read l_buf and o_buf (length and offset) out of the value
       */
      memcpy ((UBYTE *)&l_buf, vb, sizeof (USHORT));

      memcpy ((UBYTE *)&o_buf, vb+sizeof (USHORT), sizeof (USHORT));

      len = (2 * sizeof (USHORT)) + ((l_buf+o_buf+7)/8);
      if (handle)
      {
        if (edescr->ptrtype == noptr)
          handle->lenVarPart += (USHORT) len;
        else
          handle->lenVarPart += sizeof (void*);
      }
      edescr->bytelen = len;
    }
  }

  /*
   * write the value into the C-structure
   */
  memcpy (cs+edescr->offset, vb, len);

  return CCDEDIT_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_get_type        |
+--------------------------------------------------------------------+

  PURPOSE : Requests the type (primitive or message) for a given
            name. The type is stored in the return parameter type.

*/

USHORT CCDDATA_PREF(cde_get_type) (char      *name,
                     T_ELM_SRC *type)
{
#ifdef CCD_SYMBOLS
  USHORT SAP, Opcode, Direction, Entity;

  /*
   * check the primitive table first. Look in all SAPs ands for
   * all direction alls opcodes to find the name as a primitve 
   * name.
   */

  for (SAP = 0; SAP <= ccddata_max_sap_num; SAP++)
    for (Direction = 0; Direction <= 1; Direction++)
      for (Opcode = 0; Opcode <= ccddata_max_primitive_id; Opcode++)
        if (ccddata_get_pmtx(SAP, Opcode, Direction) NEQ NO_REF)
        {
          if (!strcmp (name,
                       pcomp[ccddata_get_pmtx(SAP, Opcode, Direction)].name)) 
          {
            *type = FromPrim;
            return CCDEDIT_OK;
          }
        }

  /*
   * check the message table second. Look in all entities ands for
   * all direction alls opcodes to find the name as a message
   * name.
   */

  for (Entity = 0; Entity < ccddata_num_of_entities; Entity++)
    for (Direction = 0; Direction <= 1; Direction++)
      for (Opcode = 0; Opcode <= ccddata_max_message_id; Opcode++)
        if (ccddata_get_mmtx(Entity, Opcode, Direction) NEQ NO_REF)
        {
          if (!strcmp (name,
                       mcomp[ccddata_get_mmtx(Entity, Opcode, Direction)].name)) 
          {
            *type = FromPrim;
            return CCDEDIT_OK;
          }
        }

#endif

  return CCDEDIT_PRIM_NOT_FOUND;
}

/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_get_primcode    |
+--------------------------------------------------------------------+

  PURPOSE : Requests the opcode of the primitive for a given
            name. The opcode is stored in the return parameter primcode.

*/

USHORT CCDDATA_PREF(cde_get_primcode) (char      *name,
                         ULONG    *primcode)
{
#ifdef CCD_SYMBOLS
  USHORT SAP, Opcode, Direction;

  /*
   * check the primitive table. Look in all SAPs ands for
   * all direction alls opcodes to find the name as a primitve 
   * name.
   */

  for (SAP = 0; SAP <= ccddata_max_sap_num; SAP++)
    for (Direction = 0; Direction <= 1; Direction++)
      for (Opcode = 0; Opcode <= ccddata_max_primitive_id; Opcode++)
        if (ccddata_get_pmtx(SAP, Opcode, Direction) NEQ NO_REF)
        {
          if (!strcmp (name, pcomp[ccddata_get_pmtx(SAP, Opcode, Direction)].name)) 
          {
            *primcode = ((Direction & 0x01) << 14);
            *primcode |= (SAP & 0x3fff);
            *primcode |= ((Opcode & 0xff) << 16);
            *primcode |= 0x80000000;
            
            return CCDEDIT_OK;
          }
        }
#endif

  return CCDEDIT_PRIM_NOT_FOUND;
}


/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCDEDIT             |
| STATE   : code                       ROUTINE : cde_get_msgcode     |
+--------------------------------------------------------------------+

  PURPOSE : Requests the opcode, the direction and the entity-number
            of the message for a given name.
            The opcode is stored in the return parameters .

*/

USHORT CCDDATA_PREF(cde_get_msgcode) (char      *name,
                        UBYTE     *type,
                        UBYTE     *direction,
                        UBYTE     *entity)
{
#ifdef CCD_SYMBOLS
  USHORT Opcode, Direction, Entity;

  /*
   * check the message table. Look in all entities ands for
   * all direction alls opcodes to find the name as a message
   * name.
   */

  for (Entity = 0; Entity < ccddata_num_of_entities; Entity++)
    for (Direction = 0; Direction <= 1; Direction++)
      for (Opcode = 0; Opcode <= ccddata_max_message_id; Opcode++)
        if (ccddata_get_mmtx(Entity, Opcode, Direction) NEQ NO_REF)
        {
          if (!strcmp (name,
              mcomp[ccddata_get_mmtx(Entity, Opcode, Direction)].name)) 
          {
            *type      = (UBYTE) Opcode;
            *direction = (UBYTE) Direction;
            *entity    = (UBYTE) Entity;
            return CCDEDIT_OK;
          }
        }

#endif

  return CCDEDIT_MESSAGE_NOT_FOUND;
}

/*
+------------------------------------------------------------------------------
|  Function     :  cde_get_is_downlink
+------------------------------------------------------------------------------
|  Description  :  This function finds out if an AIM is a downlink or uplink.
|
|  Parameters   :  comp_index.
|
|  Return       :  False if uplink otherwise true (downlink or both).
+------------------------------------------------------------------------------
*/
int CCDDATA_PREF(cde_get_is_downlink) (ULONG comp_index)
{
  UBYTE ent;
  UBYTE msg_id;

  for(ent = 0; ent < ccddata_num_of_entities ; ent++)
  {
    for(msg_id = 0; msg_id <= ccddata_max_message_id ; msg_id++)
    {
      if(ccddata_get_mmtx (ent, msg_id, 1) == (USHORT)comp_index)
        return 1;
    }
  }
  return 0;
}

/*
 * The following functions are copied from ..\TAP\tdc_interface.c and
 * renamed to get the cde_ prefix instead of tdc_.
 * It should be checked if instead of these functions the usual approach
 * to ccdedit by the functions pairs cde_comp_first/cde_comp_next
 * (respectively their prim/msg pendants) can be used (maybe in combination
 * with cde_get_comp). If the check confirms to use the usual approach,
 * those 3 functions here should be deleted again
 */
/*
+------------------------------------------------------------------------------
|  Function     :  cde_get_comp_index
+------------------------------------------------------------------------------
|  Description  :  This function searches the comp index in either pcomp or mcomp
|
|  Parameters   :  name and table type.
|
|  Return       :  The table entry, if non found it returns 0xffffffff;
+------------------------------------------------------------------------------
*/

ULONG CCDDATA_PREF(cde_get_comp_index) (CHAR* comp_name, T_ELM_SRC table)
{
  ULONG comp_index;
  BOOL  found = FALSE;

  if (table == FromMsg)
  {
    /*
     * search the mcomp-table for the given name
    */
    comp_index = 0;
    while (!found AND mcomp[comp_index].name NEQ NULL)
    {
      /*
       * composition name found
       */
      if (strcmp (mcomp[comp_index].name, comp_name) EQ 0)
        found = TRUE;
      else
        comp_index++;
    }
    if(found)
      return comp_index;
    else
      return NO_ENTRY_FOUND;
  }
  else
  {
    /*
     * search the pcomp-table for the given name
     */
    comp_index = 0;
    while (!found AND pcomp[comp_index].name NEQ NULL)
    {
      /*
       * composition name found
       */
      if (strcmp (pcomp[comp_index].name, comp_name) EQ 0)
        found = TRUE;
      else
        comp_index++;
    }
    if(found)
      return comp_index;
    else
      return NO_ENTRY_FOUND;
  }
}

/*
+------------------------------------------------------------------------------
|  Function     :  cde_get_element_name
+------------------------------------------------------------------------------
|  Description  :  This function gets the element name for a given index + offset.
|
|  Parameters   :  comp_index, offset and table type.
|
|  Return       :  The element name.
+------------------------------------------------------------------------------
*/

CHAR* CCDDATA_PREF(cde_get_element_name) (ULONG comp_index, USHORT elem_off , T_ELM_SRC table)
{
  if (table == FromMsg)
  {
    if (mcomp[comp_index].componentRef == -1 || elem_off >= mcomp[comp_index].numOfComponents)
      return NULL;
    return ccddata_get_alias ((USHORT) (mcomp[comp_index].componentRef + elem_off), table == FromMsg);
  }
  else
    if (pcomp[comp_index].componentRef == -1 || elem_off >= pcomp[comp_index].numOfComponents)
      return NULL;
    return ccddata_get_alias ((USHORT) (pcomp[comp_index].componentRef + elem_off), table == FromMsg);
}

/*
+------------------------------------------------------------------------------
|  Function     :  cde_get_array_kind
+------------------------------------------------------------------------------
|  Description  :  This function gets the array kind - e.g. the cSize of the 
|                  arrays (byte, short og long).
|
|  Parameters   :  Name of the base type (var_name) and table type.
|
|  Return       :  The cSize of the var_name. If not found it returns 0xffffffff
+------------------------------------------------------------------------------
*/

ULONG CCDDATA_PREF(cde_get_array_kind) (CHAR* var_name, T_ELM_SRC table)
{
  ULONG var_index;
  BOOL found = FALSE;

  if (table == FromMsg)
  {
    /*
     * search the mvar-table for the given name
    */
    var_index = 0;
    while (!found AND mvar[var_index].name NEQ NULL)
    {
      /*
       * name found
       */
      if (strcmp (mvar[var_index].name, var_name) EQ 0)
        found = TRUE;
      else
        var_index++;
    }
    if(found)
      return (ULONG) mvar[var_index].cSize;
    else
      return NO_ENTRY_FOUND;
  }
  else
  {
    /*
     * search the pvar-table for the given name
     */
    var_index = 0;
    while (!found AND pvar[var_index].name NEQ NULL)
    {
      /*
       * name found
       */
      if (strcmp (pvar[var_index].name, var_name) EQ 0)
        found = TRUE;
      else
        var_index++;
    }
    if(found)
      return (ULONG) pvar[var_index].cSize;
    else
      return NO_ENTRY_FOUND;
  }
}
