/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_FORP
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

#ifndef MM_FORP_C
#define MM_FORP_C

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

/*==== TEST =====================================================*/

/*
 * -------------------------------------------------------------------
 * PRIMITIVE Processing functions
 * -------------------------------------------------------------------
 */


#if 0 
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_init_mm_data           |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the MM data for the module formatter.
            This function is not referenced.

*/

GLOBAL void for_init_mm_data (void)
{
  TRACE_FUNCTION ("for_init_mm_data()");
}
#endif /* #if 0 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_FOR                     |
| STATE   : code                ROUTINE : for_rr_data_ind            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_DATA_IND.

*/

GLOBAL void for_rr_data_ind (T_RR_DATA_IND *rr_data_ind_org)
{
  GET_INSTANCE_DATA;
  UBYTE pd;
  UBYTE ti;
  U8   *payload;
  U16   length;
/* Implements Measure#32: Row 14,...,24 */
  PPASS (rr_data_ind_org, rr_data_ind, RR_DATA_IND);

  TRACE_FUNCTION ("for_rr_data_ind()");
  mm_data->error = 0;

  GET_PD (rr_data_ind->sdu, pd);
  GET_TI (rr_data_ind->sdu, ti);

  if (pd EQ PD_MM)
  {
    if (! ti)
    {
      T_mob_class_2 mob_class_2;

      csf_read_mobile_class_2 (&mob_class_2);

      rr_data_ind->sdu.l_buf -= 8;
      rr_data_ind->sdu.o_buf += 8;

      if (ccd_decodeMsg (CCDENT_MM,
                         DOWNLINK,
                         (T_MSGBUF *) &rr_data_ind->sdu,
                         (UBYTE *) _decodedMsg,
                         NOT_PRESENT_8BIT) NEQ ccdOK)
      {
        U16 parlist [6];
        U8  ccd_err;
        
        memset (parlist,0, sizeof (parlist));
        ccd_err = ccd_getFirstError (CCDENT_MM, parlist);

        /*
         * Error Handling
         */

        do      /* Read errors */
        {
          switch (ccd_err)
          {
            case ERR_INVALID_MID:             /* ignore message & send STATUS #97 */
              mm_for_set_error (RC_MESSAGE_TYPE_NOT_IMPLEM);
              break;                              
            case ERR_MSG_LEN:                 /* will be activated soon by CCD; agreemen in RR
                                                 if decoding was continued outside MSG-buffer it 
                                                 should be handled as mand. IE missing */
            case ERR_MAND_ELEM_MISS:          /* ignore message & send STATUS #96 */
            case ERR_COMPREH_REQUIRED:        /* ignore message & send STATUS #96 */
              mm_for_set_error (RC_INVALID_MAND_MESSAGE);
              break;
#if defined (WIN32)                           /* This part is interesting only in implementation 
                                                 and testing; may be included if specific handling
                                                 for specific errors is required in future releases */
            case ERR_IE_NOT_EXPECTED:         /* ignore IE */
            case ERR_IE_SEQUENCE:             /* ignore IE */
            case ERR_MAX_IE_EXCEED:           /* ignore IE */
              TRACE_EVENT_P1 ("IE ignored, CCD error code = %u", ccd_err);
              break;
            case ERR_PATTERN_MISMATCH:
            case ERR_INVALID_TYPE:
              TRACE_EVENT_P1 ("MNC decoding erroneous = %u", ccd_err);
              break;
#endif /* #if defined (WIN32) */
            default:
#if defined (WIN32)
              TRACE_EVENT_P1 ("Unexpected warnings/errors = %u", ccd_err);
#endif /* #if defined (WIN32) */
              break; /* No or non-interesting error */
          }
          ccd_err = ccd_getNextError (CCDENT_MM, parlist);
        } while ( ccd_err NEQ ERR_NO_MORE_ERROR                       
                  AND !mm_check_critical_error()  );                  
      } /* if (ccd_ ... */

      if (!mm_check_critical_error())               
      {
/* Implements Measure#32: Row 14,...,24 */
        payload  = &(rr_data_ind->sdu.buf[0]);     /* beginning of buffer      */
        payload += ((rr_data_ind->sdu.o_buf) >> 3) - 1;  /*  plus offset (bytes)     */
        length   = ((rr_data_ind->sdu.l_buf) >> 3) + 1;  /* length (bytes, bits / 8) */


/* Implements Measure#32: Row 14,...,24 */
        switch (_decodedMsg[0])
        {
          case D_AUTH_REJ:  
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: AUTHENTICATION REJECT",
                         payload,
                         length);
            break;

          case D_AUTH_REQ:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: AUTHENTICATION REQUEST",
                         payload,
                         length);
            break;

          case D_CM_SERV_ACCEPT:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: CM SERV ACCEPT",
                         payload,
                         length);
            break;

          case D_ABORT:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: ABORT",
                         payload,
                         length);
            break;

          case D_CM_SERV_REJ:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: CM SERVICE REJECT",
                         payload,
                         length);
            break;

          case D_IDENT_REQ:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: IDENTITY REQUEST",
                         payload,
                         length);
            break;

          case D_LOC_UPD_ACCEPT:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: LOC UPDATE ACCEPT",
                         payload,
                         length);
            break;

          case D_LOC_UPD_REJ:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: LOC UPDATE REJECT",
                         payload,
                         length);
            break;

          case B_MM_STATUS:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: MM STATUS",
                         payload,
                         length);
            break;

          case D_TMSI_REALLOC_CMD:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: TMSI REALLOC COMMAND",
                         payload,
                         length);
            break;

          case D_CM_SERVICE_PROMPT:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: CM SERVICE PROMPT",
                         payload,
                         length);
            break;

          case D_MM_INFORMATION:
            TRACE_BINDUMP( mm_handle,
                         TC_USER4,
                         "Down Link: MM INFORMATION",
                         payload,
                         length);
            break;

          default:
            /*
             * message is unknown; if this happens, AIM does not match 
             * implementation
             */
            mm_for_set_error(RC_MESSAGE_TYPE_NOT_IMPLEM);
            break;
        } /* switch */
        switch (_decodedMsg[0])
        {
          case D_AUTH_REJ:  
            mm_auth_rej ();
            break;

          case D_AUTH_REQ:
            mm_auth_req ((T_D_AUTH_REQ *)_decodedMsg);
            break;

          case D_CM_SERV_ACCEPT:
            mm_cm_serv_accept ();
            break;

          case D_ABORT:
            mm_abort ((T_D_ABORT *)_decodedMsg);
            break;

          case D_CM_SERV_REJ:
            mm_cm_serv_rej (MSG(D_CM_SERV_REJ));
            break;

          case D_IDENT_REQ:
            mm_ident_req ((T_D_IDENT_REQ *)_decodedMsg);
            break;

          case D_LOC_UPD_ACCEPT:
            mm_loc_upd_acc (MSG(D_LOC_UPD_ACCEPT));
            break;

          case D_LOC_UPD_REJ:
            mm_lup_rej (MSG(D_LOC_UPD_REJ));
            break;

          case B_MM_STATUS:
            mm_mm_status ();
            break;

          case D_TMSI_REALLOC_CMD:
              mm_tmsi_realloc_cmd (MSG(D_TMSI_REALLOC_CMD));
            break;

          case D_CM_SERVICE_PROMPT:
            mm_cm_service_prompt (MSG(D_CM_SERVICE_PROMPT));
            break;

          case D_MM_INFORMATION:
            mm_mm_information (MSG(D_MM_INFORMATION));
            break;

          default:
            /*
             * message is unknown; if this happens, AIM does not match 
             * implementation
             */
            mm_for_set_error(RC_MESSAGE_TYPE_NOT_IMPLEM);
            break;
        } /* switch */
      } /* if (!mm_check_critical_error() */
    } /* if !ti */
  } /* if pd EQ PD_MM */
  else 
  {
    if (ti >= 8)
      ti -= 8;
    else
      ti += 8;
    mm_cm_message (pd, ti, rr_data_ind);
    return;
  } /* else if pd EQ PD_MM */
  
  mm_send_status_on_error();  /* handles critical errors preventing AIM from processing */
  PFREE (rr_data_ind);        /* the message pointer should be freed here */
}

#endif
