/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata_pconst.c
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
|  Purpose :  Ccddata abstraction for use in lib/dll. The three defines (MAX...
|             from pconst.cdg are supplied as constants. In addition,
|             corresponding functions deliver these constants.
+----------------------------------------------------------------------------- 
*/ 

#include "pconst.cdg"

const int ccddata_max_sap_num = MAX_SAP_NUM;
const int ccddata_max_primitive_id = MAX_PRIMITIVE_ID;
const int ccddata_max_pstruct_len = MAX_PSTRUCT_LEN;

int ccddata_get_max_sap_num ()
{
  return ccddata_max_sap_num;
}

int ccddata_get_max_primitive_id ()
{
  return ccddata_max_primitive_id;
}

int ccddata_get_max_pstruct_len ()
{
  return ccddata_max_pstruct_len;
}
