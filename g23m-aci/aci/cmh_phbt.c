/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_PHBT
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
|  Purpose :  This module provides the test functions related to the 
|             protocol stack adapter for the phonebook management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_PHBT_C
#define CMH_PHBT_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "phb.h"
#include "cmh.h"
#include "cmh_phb.h"

/*==== CONSTANTS ==================================================*/

#define PHB_CMH_FIRST_IDX   (1)  /* first index in phonebook */

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBT                 |
| STATE   : code                  ROUTINE : cmhPHB_get_phonebook_info|
+--------------------------------------------------------------------+

  PURPOSE : This function is used to get the basic adjustments of the
            current selected phonebook storage.

            <firstIdx>: first supported index 
            <lastIdx>:  last supported index
            <nlength>:  maximum length of phone number
            <tlength>:  maximum length of associated text
*/
LOCAL T_ACI_RETURN cmhPHB_get_phonebook_info( T_ACI_CMD_SRC srcId,
                                              SHORT*        firstIdx,
                                              SHORT*        lastIdx,
                                              UBYTE*        nlength,
                                              UBYTE*        tlength )
{
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameter */
  SHORT maxIdx;   /* maximum record index     */
  UBYTE numLen;   /* maximum num length       */
  UBYTE tagLen;   /* maximum tag length       */
  SHORT dmyUsed;  /* dummy variable, not used */
  SHORT dmyMaxExt;
  SHORT dmyUsedExt;
#ifndef TI_PS_FFS_PHB
  UBYTE dmySrvc;  /* dummy variable, not used */
  SHORT dmyAvail; /* dummy variable, not used */
#endif

  TRACE_FUNCTION("cmhPHB_get_phonebook_info( )");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if( ! cmh_IsVldCmdSrc (srcId) ) 
  { 
    return( AT_FAIL );
  }

  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;

  /*
   *-----------------------------------------------------------------
   * fill in parameter <firstIdx>, <lastIdx>, <nlength> and <tlength>
   *-----------------------------------------------------------------
   */  

  if ( pPHBCmdPrm -> cmhStor EQ PB_STOR_NotPresent )
  {
    *firstIdx = ACI_NumParmNotPresent;
    *lastIdx  = ACI_NumParmNotPresent;
    *nlength  = 0;
    *tlength  = 0;
    return(AT_CMPL);
  }

#ifdef TI_PS_FFS_PHB
  if (pb_read_sizes ((T_PHB_TYPE)pPHBCmdPrm -> phbStor,
                     &maxIdx, 
                     &dmyUsed,
                     &numLen,
                     &tagLen,
                     &dmyMaxExt,
                     &dmyUsedExt) NEQ PHB_OK)
#else
  numLen = 2 * PHB_PACKED_NUM_LEN;
  if ( pb_read_status ( pPHBCmdPrm -> phbStor,
                        &dmySrvc,
                        &maxIdx, 
                        &dmyUsed,
                        &tagLen,
                        &dmyAvail,
                        &dmyMaxExt,
                        &dmyUsedExt ) EQ PHB_FAIL )
#endif
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
    return( AT_FAIL );
  }

  *firstIdx = PHB_CMH_FIRST_IDX;
  *lastIdx  = maxIdx;
  *nlength  = numLen;
  *tlength  = tagLen; /* excluding null termination */

  return ( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBT                 |
| STATE   : code                  ROUTINE : tAT_PlusCPBR             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPBR=? 
            AT command which is responsible to test for all supported
            memory indices, the length of phone numbers and
            the length of texts associated with these numbers.

            <firstIdx>: first supported index 
            <lastIdx>:  last supported index
            <nlength>:  maximum length of phone number
            <tlength>:  maximum length of associated text
*/
GLOBAL T_ACI_RETURN tAT_PlusCPBR ( T_ACI_CMD_SRC srcId,
                                   SHORT*        firstIdx,
                                   SHORT*        lastIdx,
                                   UBYTE*        nlength,
                                   UBYTE*        tlength )
{
  TRACE_FUNCTION ("tAT_PlusCPBR ()");

  return(cmhPHB_get_phonebook_info(srcId, firstIdx, lastIdx, nlength, tlength));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBT                 |
| STATE   : code                  ROUTINE : tAT_PlusCPBF             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPBF=? 
            AT command which is responsible to test for the length of
            phone numbers and the length of texts associated with
            these numbers.

            <nlength>:  maximum length of phone number
            <tlength>:  maximum length of associated text
*/
GLOBAL T_ACI_RETURN tAT_PlusCPBF ( T_ACI_CMD_SRC srcId,
                                   UBYTE*        nlength,
                                   UBYTE*        tlength )
{
  SHORT firstIdx; /* dummy variable, not used */
  SHORT lastIdx;  /* dummy variable, not used */
  
  TRACE_FUNCTION ("tAT_PlusCPBF ()");

  return(cmhPHB_get_phonebook_info(srcId, &firstIdx, &lastIdx, nlength, tlength));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBT                 |
| STATE   : code                  ROUTINE : tAT_PlusCPBW             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPBW=? 
            AT command which is responsible to test for all supported
            memory indices, the length of phone numbers and
            the length of texts associated with these numbers.

            <firstIdx>: first supported index 
            <lastIdx>:  last supported index
            <nlength>:  maximum length of phone number
            <tlength>:  maximum length of associated text
*/
GLOBAL T_ACI_RETURN tAT_PlusCPBW ( T_ACI_CMD_SRC srcId,
                                   SHORT*        firstIdx,
                                   SHORT*        lastIdx,
                                   UBYTE*        nlength,
                                   UBYTE*        tlength )
{
  TRACE_FUNCTION ("tAT_PlusCPBW ()");

  return(cmhPHB_get_phonebook_info(srcId, firstIdx, lastIdx, nlength, tlength));
}

/*==== EOF ========================================================*/
