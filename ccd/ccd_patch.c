/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccd_patch.c
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
|  Purpose :  CCD -  routines for patching msg elements with given values
+----------------------------------------------------------------------------- 
*/ 

#define CCD_PATCH_C

/*==== INCLUDES ==============================================================*/
#include <stdio.h>
#include <string.h>

#include "typedefs.h"
#include "ccd_globs.h"
#include "Bitfun.h"
#include "ccddata.h"
#include "ccd.h"

/*==== CONSTS ================================================================*//*==== TYPES =================================================================*/
/*==== LOCALS ================================================================*/
static T_patch_info* pi;

/*==== PRIVATE FUNCTIONS =====================================================*/
/*==== PUBLIC FUNCTIONS ======================================================*/

/*
+------------------------------------------------------------------------------
|  Function     :  ccd_set_patch_infos
+------------------------------------------------------------------------------
|  Description  :  This function submits a list of patch records to CCD
|
|  Parameters   :  pinfo - the list
|
|  Return       :  ccdOK on success, otherwise ccdError
+------------------------------------------------------------------------------
*/
int CCDDATA_PREF(ccd_set_patch_infos) (T_patch_info* pinfo)
{
  pi = pinfo;
  return ccdOK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccd_patch
+------------------------------------------------------------------------------
|  Description  :  This function checks if the element is to be patched
|                  and patches if yes.
|
|  Parameters   :  globs - entity/code information (containing nesting stack)
|                  validflag - set if called from valid flag coding
|
|  Return       :  FALSE if real coding function is to be called, TRUE otherwise
|                  if validflag is set, TRUE means, the element is identified
|                  in the patch list
+------------------------------------------------------------------------------
*/

int ccd_patch (T_CCD_Globs* globs, int validflag)
{
  int i = 0;
  USHORT elem;
  if (pi)
  {
    while (pi[i].numelems)
    {
      if (!memcmp (pi[i].elemid, globs->error_stack,
                   (pi[i].numelems+1) * sizeof (U16)))
      {
        elem = pi[i].elemid[pi[i].numelems];
        if (validflag)
        {
          if (pi[i].errorcode != CCDP_NOT_FOUND)
          {
#ifdef DEBUG_CCD
            TRACE_CCD (globs, "ccd_patch(): invalid error code (%d) for %s",
                        pi[i].errorcode, ccddata_get_alias (elem, 1));
#endif
          }
          else
          {
#ifdef DEBUG_CCD
            TRACE_CCD (globs, "ccd_patch(): checked valid flag for %s",
                        ccddata_get_alias (elem, 1));
#endif
            pi[i].errorcode = CCDP_VALIDFLAG_SEEN;
          }
          return TRUE;
        }
        else
        {
          if ((pi[i].errorcode != CCDP_NOT_FOUND) &&
              (pi[i].errorcode != CCDP_VALIDFLAG_SEEN))
          {
#ifdef DEBUG_CCD
            TRACE_CCD (globs, "ccd_patch(): invalid error code (%d) for %s",
                        pi[i].errorcode, ccddata_get_alias (elem, 1));
#endif
          }
          else
          {
            UBYTE* pstruct = globs->pstruct;
            ULONG offset = globs->pstructOffs;
#ifdef DEBUG_CCD
            int j, len;
            char out[32];
            len = (pi[i].bitwidth+7)/8;
            TRACE_CCD (globs, "ccd_patch(): patching %s with 0x%x bits",
                        ccddata_get_alias (elem, 1), pi[i].bitwidth);
            for (j=0; j<len; j++)
            {
              sprintf (&out[(3*j)%24], "%02x \0", pi[i].bits[j]);
              if (!((j+1)%8))
                TRACE_CCD (globs, out);
            }
            if (((j+1)%8))
              TRACE_CCD (globs, out);
#endif
            globs->pstruct = pi[i].bits;
            globs->pstructOffs = 0;
            bf_writeBitChunk (pi[i].bitwidth, globs);
            globs->pstruct = pstruct;
            globs->pstructOffs = offset;
            pi[i].errorcode = CCDP_NO_ERROR;
          }
          return TRUE;
        }
      }
      i++;
    }
  }
  return FALSE;
}
