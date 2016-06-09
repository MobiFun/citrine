/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  ccddata_ccd.c
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
|  Purpose :  Definition of variables exported from CCDDATA to CCD 
+----------------------------------------------------------------------------- 
*/ 

/*
 * standard definitions like UCHAR, ERROR etc.
 */
#include "typedefs.h"

/* BUFFER_ALIGNMENT is defined in ccd.h, requiring ccd_globs.h */
#include "ccd_globs.h"
#include "ccd.h"

#include "mconst.cdg"

/*
 * export information about the value of constants to ccd.lib
 * NUM_OF_ENTITIES and MAX_MSTRUCT_LEN are defined in mconst.cdg 
 */

UBYTE decMsgBuffer[MAX_MSTRUCT_LEN + BUFFER_ALIGNMENT];

UBYTE* ccddata_get_decmsgbuffer (void)
{
  return decMsgBuffer;
}

UBYTE mi_length[NUM_OF_ENTITIES];

UBYTE* ccddata_get_mi_length (void)
{
  return mi_length;
}
