/*
+-----------------------------------------------------------------------------
|  Project :  COMLIB
|  Modul   :  cl_ribu.c
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
|  Purpose :  Definitions of common library functions: ring buffer
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

#ifndef CL_RIBU_C
#define CL_RIBU_C

#include <string.h>

#include "typedefs.h"
#include "vsi.h"
#include "cl_ribu.h"

#undef ENA_ASSERT

/*==== FUNCTIONS ==================================================*/

GLOBAL void cl_ribu_init(T_RIBU *ribu, const U8 depth)
{
  TRACE_FUNCTION("cl_ribu_init()");

#ifdef ENA_ASSERT
  assert(ribu NEQ NULL);
#else
  if (ribu EQ NULL)
  {
    TRACE_ERROR("ribu EQ NULL");
    return;
  }
#endif
  
  ribu->ri = 0;
  ribu->wi = 0;
  ribu->depth = depth;
  ribu->filled = 0;
}

GLOBAL U8 cl_ribu_read_index(T_RIBU *ribu)
{
  U8 ri;

  TRACE_FUNCTION("cl_ribu_read_index()");

#ifdef ENA_ASSERT
  assert(ribu NEQ NULL);
#else
  if (ribu EQ NULL)
  {
    TRACE_ERROR("ribu EQ NULL");
    return 0; //255;
  }
#endif
  
  ri = ribu->ri;
  ribu->ri++;
  if (ribu->ri EQ ribu->depth)
  {
    ribu->ri = 0;
  }
  ribu->filled--;
  return ri;
}

GLOBAL U8 cl_ribu_write_index(T_RIBU *ribu)
{
  U8 wi;

  TRACE_FUNCTION("cl_ribu_write_index()");

#ifdef ENA_ASSERT
  assert(ribu NEQ NULL);
#else
  if (ribu EQ NULL)
  {
    TRACE_ERROR("ribu EQ NULL");
    return 0; //255;
  }
#endif
  
  wi = ribu->wi;
  ribu->wi++;
  if (ribu->wi EQ ribu->depth)
  {
    ribu->wi = 0;
  }

#ifdef ENA_ASSERT
  assert(ribu->ri NEQ ribu->wi);
#else
  if (ribu->ri EQ ribu->wi)
  {
    TRACE_ERROR("cl_ribu_write_index(): buffer full!");
    return 0; //255;
  }
#endif

  ribu->filled++;
  return wi;
}

GLOBAL void cl_ribu_create(T_RIBU_FD **ribu, const U8 buflen, const U8 depth)
{
  int i;

  TRACE_FUNCTION("cl_ribu_create()");

  if (*ribu NEQ NULL)
  {
    TRACE_EVENT("cl_ribu_create(): *ribu already created ?");
  }

  MALLOC(*ribu, sizeof(T_RIBU_FD));
  
  cl_ribu_init(&(*ribu)->idx, depth);

  MALLOC((*ribu)->pFDv, depth * sizeof(T_FD*));

  for (i = 0; i < depth; i++)
  {
    T_FD **pFD = &(*ribu)->pFDv[i];
    MALLOC(*pFD, sizeof(T_FD));
    MALLOC((*pFD)->buf, buflen * sizeof(U8));
  }
}

GLOBAL void cl_ribu_release(T_RIBU_FD **ribu)
{
  int i;

  TRACE_FUNCTION("cl_ribu_release()");

  if (*ribu EQ NULL)
  {
    TRACE_EVENT("cl_ribu_release(): *ribu EQ NULL!");
    return;
  }

  for (i = 0; i < (*ribu)->idx.depth; i++)
  {
    T_FD *pFD = (*ribu)->pFDv[i];
    MFREE(pFD->buf);
    MFREE(pFD);
  }

  MFREE((*ribu)->pFDv);
  MFREE(*ribu);
  *ribu = NULL;
}

GLOBAL BOOL cl_ribu_data_avail(const T_RIBU_FD *ribu)
{
  TRACE_FUNCTION("cl_ribu_data_avail()");

#ifdef ENA_ASSERT
  assert(ribu NEQ NULL);
#else
  if (ribu EQ NULL)
  {
    TRACE_ERROR("ribu EQ NULL");
    return 0; //255;
  }
#endif

  return ribu->idx.ri NEQ ribu->idx.wi;
}

GLOBAL T_FD *cl_ribu_get_new_frame_desc(T_RIBU_FD *ribu)
{
  U8 wi;
  T_FD *pFDc;

  TRACE_FUNCTION("cl_ribu_get_new_frame_desc()");

#ifdef ENA_ASSERT
  assert(ribu NEQ NULL);
#else
  if (ribu EQ NULL)
  {
    TRACE_ERROR("ribu EQ NULL");
    return NULL;
  }
#endif

  wi = cl_ribu_write_index(&ribu->idx);
  if (wi >= ribu->idx.depth)
  {
    TRACE_EVENT_P1("invalid write index: %d", (int)wi);
    return NULL;
  }
  pFDc = ribu->pFDv[wi];

  return pFDc;
}

GLOBAL void cl_ribu_put(const T_FD fd, T_RIBU_FD *ribu)
{
  T_FD *pFDc = cl_ribu_get_new_frame_desc(ribu);

  TRACE_FUNCTION("cl_ribu_put()");

  if (pFDc EQ NULL)
  {
    TRACE_ERROR("cl_ribu_put(): no write buffer!");
    return;
  }

  (*pFDc).type = fd.type;
  (*pFDc).status = fd.status;
  (*pFDc).len = fd.len;
  memcpy((*pFDc).buf, fd.buf, fd.len); 
}

GLOBAL T_FD *cl_ribu_get(T_RIBU_FD *ribu)
{
  int ri;
  T_FD *pFDc;

  TRACE_FUNCTION("cl_ribu_get()");

#ifdef ENA_ASSERT
  assert(ribu NEQ NULL);
#else
  if (ribu EQ NULL)
  {
    TRACE_ERROR("ribu EQ NULL");
    return NULL;
  }
#endif

  ri = (int)cl_ribu_read_index(&ribu->idx);
  pFDc = ribu->pFDv[ri];

  return pFDc;
}

GLOBAL void cl_set_frame_desc(T_FRAME_DESC *frame_desc, U8 *A0, U16 L0, U8 *A1, U16 L1)
{
  TRACE_ASSERT(frame_desc NEQ NULL);

  frame_desc->Adr[0] = A0;
  frame_desc->Len[0] = L0;
  frame_desc->Adr[1] = A1;
  frame_desc->Len[1] = L1;
}

GLOBAL void cl_set_frame_desc_0(T_FRAME_DESC *frame_desc, U8 *A0, U16 L0)
{
  TRACE_ASSERT(frame_desc NEQ NULL);

  frame_desc->Adr[0] = A0;
  frame_desc->Len[0] = L0;
  frame_desc->Adr[1] = NULL;
  frame_desc->Len[1] = 0;
}

#endif /* CL_RIBU_C */
