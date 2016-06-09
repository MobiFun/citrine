/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata_ccdent.c
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
|  Purpose :  Ccddata abstraction for use in lib/dll. The table ccdent.cdg
|             genereraed by ccdgen is included. 
|             The exported function ccddata_get_ccdent delivers a mapping
|             from an entitiy name (like e.g. "RR") to the generated
|             entity number (CCDENT_RR).
+----------------------------------------------------------------------------- 
*/ 

#define CCDDATA_CCDENT_C
#include <string.h>
#include "typedefs.h"
#include "mconst.cdg"
#include "ccdtable.h"
#include "ccddata.h"

static struct
{
  char* entname;
  int    ccdent;
} ccdenttbl [] =
{
#ifndef CCDDATA_NO_CCDENT
#include "ccdent.cdg"
#endif /* CCDDATA_NO_CCDENT */
{ (char *) NULL , 65535 }
};

short ccddata_get_ccdent (char* entname)
{
  int i = 0;

  for (;;)
  {
    if (!ccdenttbl[i].entname)
      return CCDDATA_CCDENT_INVALID;
    if (!strcmp (ccdenttbl[i].entname, entname))
      return ccdenttbl[i].ccdent;
    ++i;
  }
}

const char * ccddata_get_entname (short ccdent)
{
  int i = 0;

  for (;;)
  {
    if (!ccdenttbl[i].entname)
    {
      return NULL;
    }
    if (ccdenttbl[i].ccdent==ccdent)
    {
      return ccdenttbl[i].entname;
    }
    ++i;
  }
}
