/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  ccddata_version.c
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
|  Purpose :  This files supplies function delivering version information
|             about ccddata and the tables.
+----------------------------------------------------------------------------- 
*/ 

/*==== INCLUDES =============================================================*/
static char*
#include "ccddata_version.h"
;

#include "typedefs.h"
#include "ccdtable.h"

/*==== CONSTS ================================================================*//*==== TYPES =================================================================*/
/*==== LOCALS ================================================================*/
/*==== PRIVATE FUNCTIONS =====================================================*/
/*==== PUBLIC FUNCTIONS ======================================================*/

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_get_version
+------------------------------------------------------------------------------
|  Description  :  Deliver the version of ccddata. In ccddata_version.h the
|                  variable CCDDATA_VERSION defines a string containing the
|                  version information. This file may only contain one line
|                  CCDDATA_VERSION="X.Y.Z"
|                  because it is also used by ccddata.mk.
|
|  Parameters   :  none
|
|  Return       :  The string containing the version information.
+------------------------------------------------------------------------------
*/

char* ccddata_get_version ()
{
  return CCDDATA_VERSION;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_get_table_version
+------------------------------------------------------------------------------
|  Description  :  Deliver the version of ccddata tables.
|                  The version is a constant from ccdtable.h and is increased
|                  if the tables format changes.
|
|  Parameters   :  none
|
|  Return       :  The version number.
+------------------------------------------------------------------------------
*/

int ccddata_get_table_version ()
{
  return CCDDATA_TABLE_VERSION;
}
