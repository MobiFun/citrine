/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata_alias.c
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

#define CCDDATA_ALIAS_C
#include <string.h>
#include "typedefs.h"
#include "ccdtable.h"

static char* nullstr = "";

static T_CCD_ALIASTABLE maliastbl [] =
{
#ifndef CCDDATA_NO_ALIAS
#include "malias.cdg"
#else /* CCDDATA_NO_ALIAS */
{ "" }
#endif /* CCDDATA_NO_ALIAS */
};

static T_CCD_ALIASTABLE paliastbl [] =
{
#ifndef CCDDATA_NO_ALIAS
#include "palias.cdg"
#else /* CCDDATA_NO_ALIAS */
{ "" }
#endif /* CCDDATA_NO_ALIAS */
};

char* ccddata_get_alias (USHORT idx, int from_msg)
{
#ifndef CCDDATA_NO_ALIAS
  USHORT entries = (USHORT) (from_msg ?
                   sizeof (maliastbl) / sizeof (T_CCD_ALIASTABLE) :
                   sizeof (paliastbl) / sizeof (T_CCD_ALIASTABLE));
  if (idx >= entries)
    return nullstr;
  return from_msg ? maliastbl[idx].as_name : paliastbl[idx].as_name;
#else /* CCDDATA_NO_ALIAS */
  return nullstr;;
#endif /* CCDDATA_NO_ALIAS */
}
