/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata_ccdptab.c
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
|  Purpose :  Ccddata abstraction for use in lib/dll. The four tables
|             pvar, pcomp, pelem, and pmtx, genereraed by ccdgen
|             (in ccdptab.cdg) are supplied. In addition, corresponding
|             functions deliver specified elements of the tables. For
|             pmtx the function must be used, because this three-dimesional
|             array cannot be declared as extern without knowing the constants
|             at compile time.
+----------------------------------------------------------------------------- 
*/ 

#include "typedefs.h"
#include "pconst.cdg"
#include "ccdtable.h"

static const T_CCD_VarTabEntry   ccddata_pvar  [] =
{
#include "pvar.cdg"
};

static const T_CCD_CompTabEntry  ccddata_pcomp [] =
{
#include "pcomp.cdg"
};

static const T_CCD_ElemTabEntry  ccddata_pelem [] =
{
#include "pelem.cdg"
};

static const USHORT ccddata_pmtx [MAX_PMTX_SIZE] =
{
#include "pmtxval.cdg"
};

const T_CCD_MTXIDX  ccddata_pmtx_idx [] =
{
#include "pmtxidx.cdg"
};

const T_CCD_VarTabEntry* ccddata_get_pvar (USHORT idx)
{
  return &ccddata_pvar[idx];
}

const T_CCD_CompTabEntry* ccddata_get_pcomp (USHORT idx)
{
  return &ccddata_pcomp[idx];
}

const T_CCD_ElemTabEntry* ccddata_get_pelem (USHORT idx)
{
  return &ccddata_pelem[idx];
}

USHORT ccddata_get_pmtx (USHORT sap, USHORT primid, USHORT index)
{
  if ((sap > MAX_SAP_NUM) || (primid >= ccddata_pmtx_idx[sap].numitems))
    return NO_REF;
  return ccddata_pmtx[2*(ccddata_pmtx_idx[sap].idx + primid) + index];
}
