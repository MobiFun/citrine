/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (8410)
|  Modul   :  MM_FORF
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
|  Purpose :  This Modul defines the functions for the formatter
|             capability of the module Mobility Management.
+-----------------------------------------------------------------------------
*/

#ifndef MM_FORF_C
#define MM_FORF_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_MM

/*==== INCLUDES ===================================================*/

#if defined (NEW_FRAME)

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_mm.h"
#include "mon_mm.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"

#else

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "stddefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_mm.h"
#include "mon_mm.h"
#include "vsi.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"

#endif

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * Procedures
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_check_identity_type    |
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL BOOL for_check_identity_type (UBYTE id)
{
  TRACE_FUNCTION ("for_check_identity_type()");

  switch (id)
  {
    case ID_TYPE_IMSI:
    case ID_TYPE_TMSI:
    case ID_TYPE_IMEI:
    case ID_TYPE_IMEISV:
      return TRUE;
    default:
      return FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_check_mobile_identy    |
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL BOOL for_check_mobile_identity (T_mob_id *mob_ident)
{
  TRACE_FUNCTION ("for_check_mobile_identity()");

  return (for_check_identity_type (mob_ident->ident_type));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_check_reject_cause     |
+--------------------------------------------------------------------+

  PURPOSE : In this function it is checked whether a reject cause
            received from the network is a valid MM cause. If this
            is the case, this cause is returned as result
            of the function. In case the reject cause is in the
            range 0x30..0x3F, it is mapped to 0x30. If the reject
            cause neither is a member of the table of valid causes
            nor is in the range 0x30..0x3F, it is mapped to the
            cause RC_SERVICE_ORDER.

*/
LOCAL const UBYTE cause_table[MAX_DEFINED_CAUSES] =
{
  RC_IMSI_IN_HLR,             /* 0x02 */
  RC_ILLEGAL_MS,              /* 0x03 */
  RC_IMSI_IN_VLR,             /* 0x04 */
  RC_IMEI_NOT_ACCEPTED,       /* 0x05 */
  RC_ILLEGAL_ME,              /* 0x06 */
  RC_PLMN_NOT_ALLOWED,        /* 0x0B */
  RC_LA_NOT_ALLOWED,          /* 0x0C */
  RC_ROAMING_NOT_ALLOWED,     /* 0x0D */
#ifdef REL99
  RC_NO_SUITABLE_CELL_IN_LA,  /* 0x0f */
#endif
  RC_NETWORK_FAILURE,         /* 0x11 */
  RC_CONGETION,               /* 0x16 */
  RC_SERVICE_NOT_SUPPORTED,   /* 0x20 */
  RC_SERVICE_NOT_SUBSCRIBED,  /* 0x21 */
  RC_SERVICE_ORDER,           /* 0x22 */
  RC_IDENTIFIY,               /* 0x26 */
  RC_INCORRECT_MESSAGE,       /* 0x5F */
  RC_INVALID_MAND_MESSAGE,    /* 0x60 */
  RC_MESSAGE_TYPE_NOT_IMPLEM, /* 0x61 */
  RC_MESSAGE_TYPE_INCOMPAT,   /* 0x62 */
  RC_IE_NOT_IMPLEM,           /* 0x63 */
  RC_CONDITIONAL_IE,          /* 0x64 */
  RC_MESSAGE_INCOMPAT,        /* 0x65 */
  RC_UNSPECIFIED              /* 0x6f */
};

GLOBAL UBYTE for_check_reject_cause (UBYTE cause)
{
  UBYTE i;

  TRACE_FUNCTION ("for_check_reject_cause()");

  for (i = 0; i < MAX_DEFINED_CAUSES; i++)
  {
    if (cause EQ cause_table[i])
    {
      return cause;
    }
  }

  /*
   * cause value retry upon entry into a new cell
   */
  if (cause >= 0x30 AND
      cause <= 0x3F)
    return 0x30;

  /*
   * any other value shall be treated as service option
   * temporarily out of order
   */
  return RC_SERVICE_ORDER;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : mm_for_set_error           |
+--------------------------------------------------------------------+

  PURPOSE : set an specified cause value

*/

GLOBAL void mm_for_set_error (U8 cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_for_set_error()");
  mm_data->error = cause;
}

#if 0 /* body bag; closed at 02.10.2003; clean up not before 04/2004 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_set_mandatory_error    |
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void for_set_mandatory_error (void)
{
  TRACE_FUNCTION ("for_set_mandatory_error()");
  mm_data->error = RC_INVALID_MAND_MESSAGE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_set_content_error      |
+--------------------------------------------------------------------+

  PURPOSE : ?
            This function is not referenced

*/

GLOBAL void for_set_content_error (void)
{
  TRACE_FUNCTION ("for_set_content_error()");

  if (mm_data->error NEQ RC_INVALID_MAND_MESSAGE)
    mm_data->error = RC_INCORRECT_MESSAGE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_set_optional_error     |
+--------------------------------------------------------------------+

  PURPOSE : ?
            This function is not referenced

*/

GLOBAL void for_set_optional_error (UBYTE     iei)
{
  TRACE_FUNCTION ("for_set_optional_error()");

  if (mm_data->error NEQ RC_INVALID_MAND_MESSAGE)
  {
    if (! (iei & 0xf0))
      mm_for_set_error (RC_INVALID_MAND_MESSAGE);
    else
      mm_for_set_error (OPTIONAL_INFO_ERROR);
  }
}
#endif /* #if 0 */

#endif
