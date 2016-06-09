/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CC_FFK
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
|  Purpose :  This Modul defines the functions called by the primitive
|             processing functions of the SDL process FORMATTER.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CC_FFK_C
#define CC_FFK_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_CC
/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "message.h"
#include "ccdapi.h"
#include "prim.h"
#include "cnf_cc.h"
#include "mon_cc.h"
#include "pei.h"
#include "tok.h"
#include "cc.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FFK                     |
| STATE   : code                ROUTINE : for_check_called_party_bcd |
+--------------------------------------------------------------------+

  PURPOSE : Checks the content of a called party bcd information
            element.

*/

GLOBAL BOOL for_check_called_party_bcd (UBYTE ton, UBYTE npi)
{
  TRACE_FUNCTION ("for_check_called_party_bcd()");

  /*
   * Check numbering type
   */
  if (ton <= M_CC_TON_DEDICATED)
  {
    /*
     * Check numbering plan
     */
    switch (npi)
    {
      case M_CC_NPI_UNKNOWN:
      case M_CC_NPI_ISDN:
      case M_CC_NPI_X121:
      case M_CC_NPI_F69:
      case M_CC_NPI_NATIONAL:
      case M_CC_NPI_PRIVATE:
        return TRUE;
      default:
        break;
    }
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FFK                     |
| STATE   : code                ROUTINE : for_check_called_party_sub |
+--------------------------------------------------------------------+

  PURPOSE : Checks the content of a called party sub information
            element.

*/

GLOBAL BOOL for_check_called_party_sub (T_M_CC_called_subaddr * called_subaddr)
{
  TRACE_FUNCTION ("for_check_called_party_sub()");

  switch (called_subaddr->tos)
  {
    case M_CC_TOS_NSAP:
    case M_CC_TOS_USER:
      return TRUE;
    default:
      return FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FFK                     |
| STATE   : code                ROUTINE : for_check_calling_party_bcd|
+--------------------------------------------------------------------+

  PURPOSE : Checks the content of a calling party bcd information
            element.

*/

GLOBAL BOOL for_check_calling_party_bcd (T_M_CC_calling_num * calling_num)
{
  TRACE_FUNCTION ("for_check_calling_party_bcd()");

  /*
   * Check numbering type
   */
  if (calling_num->ton <= M_CC_TON_DEDICATED)
  {
    /*
     * Check numbering plan
     */
    switch (calling_num->npi)
    {
      case M_CC_NPI_UNKNOWN:
      case M_CC_NPI_ISDN:
      case M_CC_NPI_X121:
      case M_CC_NPI_F69:
      case M_CC_NPI_NATIONAL:
      case M_CC_NPI_PRIVATE:
        break;
      default:
        return FALSE;
    }
    /*
     * Check Present Indicator
     */
    if (calling_num->v_present AND
        (calling_num->present > M_CC_PRES_NOT_AVAIL))
      return FALSE;
    else
      return TRUE;
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : CC_FFK                      |
| STATE   : code               ROUTINE : for_check_calling_party_sub |
+--------------------------------------------------------------------+

  PURPOSE : Checks the content of a calling party sub information
            element.

*/

GLOBAL BOOL for_check_calling_party_sub (T_M_CC_calling_subaddr * calling_subaddr)
{
  TRACE_FUNCTION ("for_check_calling_party_sub()");

  switch (calling_subaddr->tos)
  {
    case M_CC_TOS_NSAP:
    case M_CC_TOS_USER:
      return TRUE;
    default:
      return FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FFK                     |
| STATE   : code                ROUTINE : for_check_callstate        |
+--------------------------------------------------------------------+

  PURPOSE : Checks the content of a callstate information
            element.

*/

GLOBAL void for_check_call_state (T_M_CC_call_state * call_state)
{
  TRACE_FUNCTION ("for_check_call_state()");

  if ( call_state->cs NEQ M_CC_CS_GSM_PLMN )        /* VK 04-jul-97 */
    call_state->state = M_CC_CS_10;                 /* VK 04-jul-97 */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : CC_FFK                      |
| STATE   : code               ROUTINE : for_check_cc_cause          |
+--------------------------------------------------------------------+

  PURPOSE : Checks the content of a cc cause information
            element.

*/

GLOBAL BOOL for_check_cc_cause (T_M_CC_cc_cause* cc_cause)
{
  TRACE_FUNCTION ("for_check_cc_cause()");
//TISH patch for OMAPS00129223
#if 0
  /* 
   * this is a (dirty) "trick" to detect that the cause IE was not present
   * although mandatory (for some messages): value 0 is reserved, i.e. it is not
   * expected that it will be sent by a network; CCD/the frame ensures that the 
   * memory contents is 0 which will be seen as value if the cause IE was not present
   */
  if (cc_cause->cause EQ 0)
    return FALSE;
#endif
  /* 
   * coding standards other than GSM are not supported (which is a valid option);
   * map the cause value to interworking, unspecified then, skip further checks
   */
  if (cc_cause->cs NEQ M_CC_CS_GSM_PLMN )
  {
    cc_cause->cause = M_CC_CAUSE_INTERWORKING;
    return TRUE;
  }

  /* check for reserved Location (octet 3) */
  switch (cc_cause->loc)
  {
    case M_CC_LOC_USER:
    case M_CC_LOC_PRIV_NET_LOCAL_USER:
    case M_CC_LOC_PUB_NET_LOCAL_USER:
    case M_CC_LOC_TRANSIT_NET:
    case M_CC_LOC_PRIV_NET_REMOTE_USER:
    case M_CC_LOC_PUB_NET_REMOTE_USER:
    case M_CC_LOC_INTERNATIONAL_NET:
    case M_CC_LOC_BEYOND_POINT:
      break;
    default:
      return FALSE;
  }

  /* check Recommendation (octet 3a) */
  /* 
   * the condition below also checks the value of octet 3a (if present);
   * this stems from GSM 04.08 of Ph1 were the value 1 was defined as indicating
   * GSM; since Ph2 there is the requirement that octet 3a shall not be included
   * for coding standard being equal to GSM; however, the design decision is
   * to keep the Ph1 behaviour in order to be a "tolerant" MS and to avoid
   * interworking problems with Ph1 infrastructure, especially as this case is
   * not treated in GSM 09.90
   */
  if (cc_cause->v_rec AND
      (cc_cause->rec NEQ 1))
    return FALSE;
  else
    return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : CC_FFK                      |
| STATE   : code               ROUTINE : for_check_progress_indicator|
+--------------------------------------------------------------------+

  PURPOSE : Checks the content of a progress indicator information
            element.

*/

GLOBAL BOOL for_check_progress_indicator (T_M_CC_progress * progress)
{
  TRACE_FUNCTION ("for_check_progress_indicator()");

  /* coding standard and progress description are not in optional octets (3 and 4) */
  if ((progress->v_cs EQ FALSE) OR
      (progress->v_progress_desc EQ FALSE))
    return FALSE;

  /* 
   * map progress description to "Unspecific" if coding standard other than
   * GSM is used; note that it is an option to not support coding standards
   * other than GSM (which Condat takes)
   */
  if (progress->cs NEQ M_CC_CS_GSM_PLMN)
  {
    progress->progress_desc = MNCC_PROG_UNSPECIFIC; 
    return TRUE; /* don't care about reserved values for Location anymore of other standards */
  }

  /* check for reserved Location values */
  switch (progress->loc)
  {
    case M_CC_LOC_USER:
    case M_CC_LOC_PRIV_NET_LOCAL_USER:
    case M_CC_LOC_PUB_NET_LOCAL_USER:
    case M_CC_LOC_PRIV_NET_REMOTE_USER:
    case M_CC_LOC_PUB_NET_REMOTE_USER:
    case M_CC_LOC_BEYOND_POINT:
      break;
    default:
      return FALSE;
  }

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : CC_FFK                      |
| STATE   : code               ROUTINE : for_check_repeat_indicator  |
+--------------------------------------------------------------------+

  PURPOSE : Checks the content of a repeat indicator information
            element.

*/

GLOBAL BOOL for_check_repeat_indicator (UBYTE repeat)
{
  TRACE_FUNCTION ("for_check_repeat_indicator()");

  if ((repeat EQ M_CC_REPEAT_CIRCULAR)
       OR
      (repeat EQ M_CC_REPEAT_SEQUENTIAL)
     )
    return TRUE;
  else
    return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : CC_FFK                      |
| STATE   : code               ROUTINE : for_check_signal            |
+--------------------------------------------------------------------+

  PURPOSE : Checks the content of a signal information
            element.

*/

GLOBAL BOOL for_check_signal (UBYTE signal)
{
  TRACE_FUNCTION ("for_check_signal()");

  if ((signal <= M_CC_SIGNAL_HOOK_OFF_ON)
       OR
      (signal EQ  M_CC_SIGNAL_TONES_OFF)
       OR
      (signal EQ M_CC_SIGNAL_ALERT_OFF))
    return TRUE;
  else
    return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : CC_FFK                      |
| STATE   : code               ROUTINE : for_set_conditional_error   |
+--------------------------------------------------------------------+

  PURPOSE : Stores a detected conditional error.

*/

GLOBAL void for_set_conditional_error (UBYTE       iei)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("for_set_conditional_error()");

  switch (cc_data->error)
  {
    case M_CC_CAUSE_INFO_ELEM_NOT_IMPLEM:
    case M_CC_CAUSE_INVALID_MAND_INFO:
      break;

    default:
      cc_data->error = M_CC_CAUSE_COND_INFO_ELEM;
      if (cc_data->error_count < MAX_ERROR_TAGS)
        cc_data->error_inf [cc_data->error_count++] = iei;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : CC_FFK                      |
| STATE   : code               ROUTINE : for_set_mandatory_error     |
+--------------------------------------------------------------------+

  PURPOSE : Stores a detected mandatory error.

*/

GLOBAL void for_set_mandatory_error ( UBYTE       iei)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("for_set_mandatory_error()");

  switch (cc_data->error)
  {
    case M_CC_CAUSE_INFO_ELEM_NOT_IMPLEM:
    case M_CC_CAUSE_COND_INFO_ELEM:
      cc_data->error_count = 0;
      /*FALLTHROUGH*/ /*lint -fallthrough*/

    default:
      cc_data->error = M_CC_CAUSE_INVALID_MAND_INFO;
      if (cc_data->error_count < MAX_ERROR_TAGS)
        cc_data->error_inf [cc_data->error_count++] = iei;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : CC_FFK                      |
| STATE   : code               ROUTINE : for_set_optional_error      |
+--------------------------------------------------------------------+

  PURPOSE : Stores a detected optional error.

*/

GLOBAL void for_set_optional_error ( UBYTE       iei)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("for_set_optional_error()");

  switch (cc_data->error)
  {
    case M_CC_CAUSE_INVALID_MAND_INFO:
      break;

    case M_CC_CAUSE_COND_INFO_ELEM:
      cc_data->error_count = 0;
      /*FALLTHROUGH*/ /*lint -fallthrough*/

    default:
      /*
       * Check whether the iei is compression required
       */
      if ((iei & 0xF0) EQ 0)
        for_set_mandatory_error (iei);
      else
      {
        cc_data->error = M_CC_CAUSE_INFO_ELEM_NOT_IMPLEM;
        if (cc_data->error_count < MAX_ERROR_TAGS)
          cc_data->error_inf [cc_data->error_count++] = iei;
      }
      break;
  }
}

/*
+--------------------------------------------------------------------+
| | STATE   : code                ROUTINE : cc_check_critical_error    |
+--------------------------------------------------------------------+

  PURPOSE : This function checks wheter a critical error has been 
            detected in the air message. Critical errors which prevent 
            the treatment of an air message are 
            - invalid Message ID
            - mandatory IE missing
            - IE coded as comprehension required missing
*/

GLOBAL BOOL cc_check_critical_error (UBYTE cc_err)
{
  TRACE_FUNCTION ("cc_check_critical_error ()");
  if (cc_err  EQ M_CC_CAUSE_INVALID_MAND_INFO       OR 
      cc_err  EQ M_CC_CAUSE_MESSAGE_TYPE_NOT_IMPLEM OR
      cc_err  EQ M_CC_CAUSE_COND_INFO_ELEM          OR
      cc_err  EQ M_CC_CAUSE_SERVICE_NOT_IMPLEM        )
    return (TRUE);
  else
    return (FALSE);
}

#endif
