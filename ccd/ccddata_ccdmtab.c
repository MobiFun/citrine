/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata_ccdmtab.c
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
|  Purpose :  Ccddata abstraction for use in lib/dll. The six tables
|             mvar, spare, calc, mcomp, melem, and mmtx, genererated by ccdgen
|             (in ccdmtab.cdg) are supplied as well as the table calcidx
|             (generated in an extra file). In addition, corresponding
|             functions deliver specified elements of the tables. For
|             mmtx the function must be used, because this three-dimesional
|             array cannot be declared as extern without knowing the constants
|             at compile time.
+----------------------------------------------------------------------------- 
*/ 

#include "typedefs.h"
#include "mconst.cdg"
#include "ccdtable.h"

static const T_CCD_VarTabEntry   ccddata_mvar  [] =
{
#include "mvar.cdg"
};

static const T_CCD_SpareTabEntry ccddata_spare [] =
{
#include "spare.cdg"
};

static const T_CCD_CalcTabEntry  ccddata_calc  [] =
{
#include "calc.cdg"
};

static const T_CCD_CompTabEntry  ccddata_mcomp [] =
{
#include "mcomp.cdg"
};

static const T_CCD_ElemTabEntry  ccddata_melem [] =
{
#include "melem.cdg"
};

static const USHORT ccddata_mmtx [MAX_MMTX_SIZE] =
{
#include "mmtxval.cdg"
};

static const T_CCD_CalcIndex ccddata_calcidx [] =
{
#include "calcidx.cdg"
};

const T_CCD_MTXIDX  ccddata_mmtx_idx [] =
{
#include "mmtxidx.cdg"
};

const T_CCD_CalcIndex* ccddata_get_calcidx (USHORT idx)
{
  return &ccddata_calcidx[idx];
}

const T_CCD_VarTabEntry* ccddata_get_mvar (USHORT idx)
{
  return &ccddata_mvar[idx];
}

const T_CCD_SpareTabEntry* ccddata_get_spare (USHORT idx)
{
  return &ccddata_spare[idx];
}

const T_CCD_CalcTabEntry* ccddata_get_calc (USHORT idx)
{
  return &ccddata_calc[idx];
}

const T_CCD_CompTabEntry* ccddata_get_mcomp (USHORT idx)
{
  return &ccddata_mcomp[idx];
}

const T_CCD_ElemTabEntry* ccddata_get_melem (USHORT idx)
{
  return &ccddata_melem[idx];
}

USHORT ccddata_get_mmtx (USHORT entity, USHORT msgid, USHORT index)
{
  if ((entity >= NUM_OF_ENTITIES) || (msgid >= ccddata_mmtx_idx[entity].numitems))
     return NO_REF;
  return ccddata_mmtx[2*(ccddata_mmtx_idx[entity].idx + msgid) + index];
}
