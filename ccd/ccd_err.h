/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccd_err.h
+----------------------------------------------------------------------------- 
|  Copyright 2004 Texas Instruments Berlin, AG 
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
|  Purpose :  Condat Coder Decoder
|             Definition of CCD Error table
+----------------------------------------------------------------------------- 
*/ 

#ifndef __CCD_ERR_H
#define __CCD_ERR_H

#include "ccdapi.h"

#define ERR_TXT_LEN 30
#if defined (DEBUG_CCD)

/*
 * CCD Error table
 */

char* ccdErrCodeTable[] = 
{
  "ERR_NO_MORE_ERROR",
  "ERR_INVALID_CALC",
  "ERR_PATTERN_MISMATCH",
  "ERR_COMPREH_REQUIRED",
  "ERR_IE_NOT_EXPECTED",
  "ERR_IE_SEQUENCE",
  "ERR_MAX_IE_EXCEED",
  "ERR_MAND_ELEM_MISS",
  "ERR_EOC_TAG_MISSING",
  "ERR_INVALID_MID",
  "ERR_INVALID_TYPE",
  "ERR_MAX_REPEAT",
  "ERR_NO_MEM",
  "ERR_ADDRESS_INFOMATION_PART",
  "ERR_DISTRIBUTION_PART",
  "ERR_NON_DISTRIBUTION_PART",
  "ERR_MESSAGE_ESCAPE",
  "ERR_IGNORE",
  "ERR_DUMMY",
  "ERR_DUMMY",
  "ERR_INTERNAL_ERROR",
  "ERR_DEFECT_CCDDATA",
  "ERR_END_OF_BUF",
  "ERR_INT_VALUE",
  "ERR_LONG_MSG",
  "ERR_ASN1_ENCODING",
  "ERR_ASN1_MAND_IE",
  "ERR_ASN1_OPT_IE",
  "ERR_ASN1_COND_IE",
  "ERR_COND_ELEM_MISS",
  "ERR_BUFFER_OF",
  "ERR_NONCRITICAL_EXT",
  "ERR_CRITICAL_EXT",
  "ERR_INVALID_CCDID",
  "ERR_MSG_LEN",
  "ERR_INVALID_PTR",
  "ERR_PROTOCOL_EXTENSION",
  "ERR_BITSTR_COMP",
  "ERR_ELEM_LEN",
  "ERR_LEN_MISMATCH",
  "ERR_CONCAT_LEN",
  "ERR_UNEXPECT_PAD",
  "ERR_CSN1_CHOICE",
  "MAX_CCD_ERROR"
};
#endif /* DEBUG_CCD */
 
#endif
