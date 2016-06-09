/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata_cdepval.c
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
|             pval and pstr, genereraed by ccdgen (in cdepval.cdg)
|             are supplied. In addition, corresponding functions deliver
|             specified elements of the tables.
+----------------------------------------------------------------------------- 
*/ 

#include "typedefs.h"
#include "ccdtable.h"

static const T_CCD_ValTabEntry ccddata_pval [] =
{
#include "pval.cdg"
};

static const T_CCD_StrTabEntry ccddata_pstr [] =
{
#include "pstr.cdg"
};

const T_CCD_ValTabEntry* ccddata_get_pval (USHORT idx)
{
  return &ccddata_pval[idx];
}

const T_CCD_StrTabEntry* ccddata_get_pstr (USHORT idx)
{
  return &ccddata_pstr[idx];
}
