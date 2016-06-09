/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  SS_FOR
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
|  Purpose :  This modul defines the functions for the formatter
|             capability of the module supplementary services.
+----------------------------------------------------------------------------- 
*/ 

#ifndef SS_FOR_C
#define SS_FOR_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SS

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
#include "cnf_ss.h"
#include "mon_ss.h"
#include "pei.h"
#include "tok.h"
#include "ss.h"

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
#include "cnf_ss.h"
#include "mon_ss.h"
#include "vsi.h"
#include "pei.h"
#include "tok.h"
#include "ss.h"

#endif

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/
#ifdef TI_PS_HCOMM_CHANGE
#if defined (NEW_FRAME)
EXTERN T_HANDLE ss_handle;
#endif
#else
#if defined (NEW_FRAME)
EXTERN T_HANDLE hCommMM;                 /* MM   Communication       */
EXTERN T_HANDLE ss_handle;
#else
EXTERN T_VSI_CHANDLE  hCommMM;         /* MM   Communication       */
#endif
#endif /* TI_PS_HCOMM_CHANGE */

#if !defined (SHARED_CCD_BUF)
GLOBAL UBYTE               _decodedMsg[MAX_MSTRUCT_LEN_SS];
#else
GLOBAL UBYTE *             _decodedMsg;
#endif
#ifdef SS_TEST
EXTERN UBYTE  trc[];
#endif

/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * Procedures
 * -------------------------------------------------------------------
 */

/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_FOR                        |
| STATE   : code                ROUTINE : for_mmss_establish_n_data_ind |
+-----------------------------------------------------------------------+

  PURPOSE : This common routine process the primitives MMSS_ESTATBLISH_IND 
            and MMSS_DATA_IND

*/


LOCAL void for_mmss_establish_n_data_ind(T_PRIM *prim)
{
  GET_INSTANCE_DATA;

  CHAR   *msg_type;
  BOOL   invalid_msg = FALSE;
  T_sdu  *sdu;
  
  TRACE_FUNCTION ("for_mmss_establish_n_data_ind()");
  
  if(prim->custom.opc EQ MMSS_ESTABLISH_IND)
  {
    sdu = &((T_MMSS_ESTABLISH_IND  *)P2D(prim))->sdu;
    ss_data->est_flag              = TRUE;
  }
  else
  {
    sdu = &((T_MMSS_DATA_IND  *)P2D(prim))->sdu;
    ss_data->est_flag  =  FALSE;
  }
  
  if ((ss_data->ti & 7) NEQ 7)
  {
    sdu->o_buf += 8;
    sdu->l_buf -= 8;
  
    CCD_START;
  
    if(ccd_decodeMsg (CCDENT_SS,
                      DOWNLINK,
                      (T_MSGBUF *) sdu, 
                      (UBYTE    *) _decodedMsg,
                      NOT_PRESENT_8BIT) NEQ ccdOK )
    {
      USHORT parlist[6];      
      UBYTE  prev_err;
      UBYTE  ccd_err;
      memset (parlist,0, sizeof (parlist));
      ccd_err = ccd_getFirstError (CCDENT_SS, parlist);      
     /*
     * Error Handling
     */
        ss_data->error = CAUSE_UNSPECIFIED; /*Normal*/
        do      /* Read errors */
        {
          switch (ccd_err)
          {
            case ERR_INVALID_MID:     /* Send RELEASE REQUEST message with cause #97 */
             /*TS TS 24.010 Section 3.7.4 */
               send_rel_comp(CAUSE_MESSAGE_TYPE_NOT_IMPLEM);
               PFREE(P2D(prim)); 
              CCD_END;
              return;
      
            case ERR_COMPREH_REQUIRED:    /* comprehension required     */
            case ERR_MAND_ELEM_MISS:      /* Mandatory elements missing */
            case ERR_MSG_LEN:             /* Message Length exceeds its limit */
              /*TS TS 24.010 Section 3.7.5 */
              ss_data->error = CAUSE_INVALID_MAND_INFO;
              break;

#if defined (WIN32)
              /* This part is interesting only in implementation and testing; may be 
               * included if specific handling for specific errors is required in 
               * future releases 
               */
          case ERR_IE_NOT_EXPECTED:
               TRACE_ERROR("CCD ERROR: IE not expected. properly an R99 optional element like cell notification, eplmn, or t3302");
               break;
      
            case ERR_IE_SEQUENCE:
              TRACE_EVENT_P1("Wrong sequence of IE,  CCD error code = %u", ccd_err);
              break;
      
            case ERR_MAX_IE_EXCEED:
              TRACE_EVENT_P1("Maximum amount of repeatable IE has exceeded,  CCD error code = %u", ccd_err);
              break;
      
            case ERR_MAX_REPEAT:
              TRACE_EVENT_P1("A repeatable IE occurs too often in the message,  CCD error code = %u", ccd_err);
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
              prev_err = ccd_err;
              ccd_err = ccd_getNextError (CCDENT_SS, parlist);             
      }while ( prev_err NEQ ERR_NO_MORE_ERROR                       
                  AND !ss_check_critical_error(ss_data->error) );  
    }

    switch (_decodedMsg[0])
    {
      case D_SS_FACILITY:
        ss_d_ss_facility ();
        msg_type = "D_SS_FACILITY";
        break;
      case D_SS_REGISTER:
        ss_d_ss_register ();
        msg_type = "D_SS_REGISTER";
        break;
      case B_SS_REL_COMP:
        ss_b_ss_rel_comp ();
        msg_type = "B_SS_REL_COMP";
        break;
      default:
        /* This should NOT be reached */
        TRACE_FUNCTION("**UNKNOWN MESSAGE ID**");
        msg_type = "UNKNOWN MESSAGE";
        invalid_msg = TRUE;
        break;
    }
    if(!invalid_msg)
    {
      TRACE_BINDUMP( ss_handle,
                     TC_USER4,
                     msg_type,
                     (&(sdu->buf[0]))+(((sdu->o_buf)>>3) -1),
                     (((sdu->l_buf)>>3) +1));
    }

    CCD_END;
  }
  else  if(prim->custom.opc EQ MMSS_ESTABLISH_IND)
  {
    /* Establishment with a reserved TI. Release the MM connection */
    PALLOC (mmss_release_req, MMSS_RELEASE_REQ); /* T_MMSS_RELEASE_REQ */
    mmss_release_req->ti = ss_data->ti;
    PSENDX (MM, mmss_release_req);
  }

  PFREE(P2D(prim));
}


/*
 * -------------------------------------------------------------------
 * PRIMITIVE Processing functions
 * -------------------------------------------------------------------
 */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_FOR                     |
| STATE   : code                ROUTINE : for_mmss_establish_ind     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSS_ESTATBLISH_IND.

*/

GLOBAL void for_mmss_establish_ind (T_MMSS_ESTABLISH_IND *mmss_establish_ind_org)
{
  GET_INSTANCE_DATA;	

  /* Enable the PCO to correctly decode the message */
  PPASS (mmss_establish_ind_org, mmss_establish_ind, MMSS_ESTABLISH_IND);

  TRACE_FUNCTION ("for_mmss_establish_ind()");

  GET_PD (mmss_establish_ind->sdu, ss_data->pd);
  GET_TI (mmss_establish_ind->sdu, ss_data->ti);

  ss_data->ti += ((ss_data->ti > 7) ? -8 : 8);

  for_mmss_establish_n_data_ind((T_PRIM *)D2P(mmss_establish_ind));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_FOR                     |
| STATE   : code                ROUTINE : for_mmss_data_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSS_DATA_IND.

*/

GLOBAL void for_mmss_data_ind (T_MMSS_DATA_IND *mmss_data_ind_org)
{
  GET_INSTANCE_DATA;	

  /* Enable the PCO to correctly decode the message */
  PPASS (mmss_data_ind_org, mmss_data_ind, MMSS_DATA_IND);

  TRACE_FUNCTION ("for_mmss_data_ind()");

  GET_PD (mmss_data_ind->sdu, ss_data->pd);
  GET_TI (mmss_data_ind->sdu, ss_data->ti);

  ss_data->ti += ((ss_data->ti > 7) ? -8 : 8);

  for_mmss_establish_n_data_ind((T_PRIM *)D2P(mmss_data_ind));
}

/*
 * -------------------------------------------------------------------
 * SIGNAL Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_FOR                     |
| STATE   : code                ROUTINE : ss_for_data_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal DATA_REQ.

*/

GLOBAL void ss_for_data_req (USHORT bit_size_message)
{
  GET_INSTANCE_DATA;
  CHAR *msg_type;
  BOOL invalid_msg = FALSE;
  PALLOC_SDU (mmss_data_req, MMSS_DATA_REQ, bit_size_message);

  TRACE_FUNCTION ("ss_for_data_req()");

  mmss_data_req->sdu.l_buf = bit_size_message;
  mmss_data_req->sdu.o_buf = ENCODE_OFFSET;

  if (ccd_codeMsg (CCDENT_SS,
                   UPLINK,
                   (T_MSGBUF *) &mmss_data_req->sdu,
                   (UBYTE    *) _decodedMsg,
                   NOT_PRESENT_8BIT)
      EQ ccdOK)
  {
    SET_PD (mmss_data_req->sdu, PD_SS);
    SET_TI (mmss_data_req->sdu, ss_data->ti);

    mmss_data_req->sdu.o_buf -= 8;
    mmss_data_req->sdu.l_buf += 8;

    /* Trace Bindump() for uplink messages.
    * called from mnss_facility_req / mnss_end_req / mnss_esatblish_req etc
    */
    switch (_decodedMsg[0])
    {
      case U_SS_FACILITY:
      msg_type = "U_SS_FACILITY";
      break;
      case U_SS_REGISTER:
      msg_type = "U_SS_REGISTER";
      break;
      case B_SS_REL_COMP:
      msg_type = "B_SS_REL_COMP";
      break;
      default:
      TRACE_FUNCTION("**UNKNOWN MESSAGE ID**");
      msg_type = "UNKNOWN MESSAGE";
      invalid_msg = TRUE;
      break;
    }
    if(!invalid_msg)
    {
      TRACE_BINDUMP( ss_handle,
                     TC_USER4,
                     msg_type,
                     (&(mmss_data_req->sdu.buf[0]))+(((mmss_data_req->sdu.o_buf)>>3) -1),
                     (((mmss_data_req->sdu.l_buf)>>3) +1));
    }

    PSENDX (MM, mmss_data_req);
  }
  else
  {
    PFREE (mmss_data_req);
  }
}

#endif /* #ifndef SS_FOR_C */
