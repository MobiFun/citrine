/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata_cdemval.c
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
|  Purpose :  Ccddata abstraction for use in lib/dll. The two tables
|             mval and mstr, genereraed by ccdgen (in cdemval.cdg)
|             are supplied. In addition, corresponding functions deliver
|             specified elements of the tables.
+----------------------------------------------------------------------------- 
*/ 

#include "typedefs.h"
#include "ccdtable.h"

static const T_CCD_ValTabEntry ccddata_mval [] =
{
#include "mval.cdg"
};

const T_CCD_ValTabEntry* ccddata_get_mval (USHORT idx)
{
  return &ccddata_mval[idx];
}

//TISH modified for MSIM
//Begin
#ifndef WIN32
#ifdef CCDDATA_NO_CDEMSTR
const T_CCD_StrTabEntry* ccddata_get_mstr (USHORT idx)
{
  return &ccddata_mstr[idx];
}
#endif /* CCDDATA_NO_CDEMSTR */
#endif
//End
