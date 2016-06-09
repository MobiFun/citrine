/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_FORS
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
|             capability of the module Mobility Managemant.
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_FORS_C
#define MM_FORS_C

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
 * SIGNAL Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_cm_message             |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal cm_message.

*/

GLOBAL void for_cm_message (T_RR_DATA_REQ *rr_data_req)
{
  TRACE_FUNCTION ("for_cm_message()");

  TIMERSTOP (T3240);

  PSENDX (RR, rr_data_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_data_req               |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal data_req.

*/

GLOBAL void for_data_req (USHORT bit_size_message)
{
  U8 *payload;
  U16 length;
/* Implements Measure#32: Row 26,...,33 */
  PALLOC_SDU (data, RR_DATA_REQ, bit_size_message);

  TRACE_FUNCTION ("for_data_req()");

  data->sdu.o_buf = ENCODE_OFFSET ;

  ccd_codeMsg (CCDENT_MM,
               UPLINK,
               (T_MSGBUF *) &data->sdu,
               (UBYTE *) _decodedMsg,
               NOT_PRESENT_8BIT);

  ccd_codeByte (data->sdu.buf, (USHORT)(data->sdu.o_buf - 8), 8,
                PD_MM_TI_0);
  data->sdu.l_buf += 8;
  data->sdu.o_buf -= 8;

  payload = &(data->sdu.buf[0]);     /* beginning of buffer      */
  payload += (data->sdu.o_buf) >> 3; /*  plus offset (bytes)     */
  length  = (data->sdu.l_buf) >> 3;  /* length (bytes, bits / 8) */

/* Implements Measure#32: Row 26,...,33 */
  switch ( _decodedMsg [0] )
  {
    case U_AUTH_RES:
      TRACE_BINDUMP( mm_handle,
                   TC_USER4,
                   "Up Link: AUTHENTICATION RESPONSE",
                   payload,
                   length);
      break;

    case U_IDENT_RES:
      TRACE_BINDUMP( mm_handle,
                   TC_USER4,
                   "Up Link: IDENTITY RESPONSE",
                   payload,
                   length);
      break;

    case U_CM_REESTAB_REQ:
      TRACE_BINDUMP( mm_handle,
                   TC_USER4,
                   "Up Link: CM RE-ESTABLISH REQUEST",
                   payload,
                   length);
      break;

    case U_CM_SERV_ABORT:
      TRACE_BINDUMP( mm_handle,
                   TC_USER4,
                   "Up Link: CM SERVICE ABORT",
                   payload,
                   length);
      break;

    case U_CM_SERV_REQ:
      TRACE_BINDUMP( mm_handle,
                   TC_USER4,
                   "Up Link: CM SERVICE REQUEST",
                   payload,
                   length);
      break;

    case U_IMSI_DETACH_IND:
      TRACE_BINDUMP( mm_handle,
                   TC_USER4,
                   "Up Link: IMSI DETACH INDICATION",
                   payload,
                   length);
      break;

    case U_LOC_UPD_REQ:
      TRACE_BINDUMP( mm_handle,
                   TC_USER4,
                   "Up Link: LOC UPDATE REQUEST",
                   payload,
                   length);
      break;

    case U_TMSI_REALLOC_COMP:
      TRACE_BINDUMP( mm_handle,
                   TC_USER4,
                   "Up Link: TMSI REALLOC COMPLETE",
                   payload,
                   length);
      break;

    default:
      break;
  }
  PSENDX (RR, data);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_est_req                |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal est_req.

*/

GLOBAL void for_est_req (USHORT est_cause, USHORT bit_size_message)
{
  PALLOC_SDU ( establish_req, RR_ESTABLISH_REQ, bit_size_message);

  TRACE_FUNCTION ("for_est_req()");

  TIMERSTOP (T3240);

  establish_req->sdu.o_buf = ENCODE_OFFSET;

  ccd_codeMsg (CCDENT_MM,
               UPLINK,
               (T_MSGBUF *) &establish_req->sdu,
               (UBYTE *) _decodedMsg,
               0xff);

  ccd_codeByte (establish_req->sdu.buf,
                (USHORT)(establish_req->sdu.o_buf - 8),
                8,
                PD_MM_TI_0);
  establish_req->sdu.l_buf += 8;
  establish_req->sdu.o_buf -= 8;
  establish_req->estcs      = est_cause;

  PSENDX (RR, establish_req);
}

#endif
