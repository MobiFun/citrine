/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  
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
|  Purpose :  This modul defines the functions for the supplementary
|             services.
+----------------------------------------------------------------------------- 
*/ 

#ifndef SS_SS_C
#define SS_SS_C

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
#include "ss_em.h"

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
#include "ss_em.h"

#endif

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/
#ifdef TI_PS_HCOMM_CHANGE
#else
#if defined (NEW_FRAME)
EXTERN T_HANDLE         hCommMM;         /* MM   Communication       */
EXTERN T_HANDLE         hCommMMI;        /* MMI  Communication       */
#else
EXTERN T_VSI_CHANDLE  hCommMM;         /* MM   Communication       */
EXTERN T_VSI_CHANDLE  hCommMMI;        /* MMI  Communication       */
#endif
#endif

#ifdef SS_TEST
UBYTE  trc[100];
#endif

/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * PRIMITIVE Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_init_ss_data            |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the SS data for the module supplem. services.

*/

GLOBAL void ss_init_ss_data (void)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  TRACE_FUNCTION ("ss_init_ss_data()");

  ss_init ();
  for (i = 0; i < MAX_INST; i++)
    SET_SS_STATE(ss_data->ss_state,i,SS_IDLE); /*lint !e502 (Warning -- Warning 502: Expected unsigned type ) */

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_mmss_error_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSS_ERROR_IND.

*/

GLOBAL void ss_mmss_error_ind (T_MMSS_ERROR_IND  *mmss_error_ind)
{
  GET_INSTANCE_DATA;	
  TRACE_FUNCTION ("ss_mmss_error_ind()");

  ss_data->ti = mmss_error_ind->ti;
  switch (GET_SS_STATE(ss_data->ss_state,ss_data->ti))
  {
    case SS_CONNECTION_PENDING:
      PFREE (P2D(ss_data->prim[ss_data->ti]));
      ss_data->prim[ss_data->ti] = NULL;
      /*FALLTHROUGH*/
    case SS_CONNECTED:
    {
      PALLOC (mnss_end_ind, MNSS_END_IND);

      ss_data->ti = mmss_error_ind->ti;

      mnss_end_ind->cause = mmss_error_ind->cause;
      mnss_end_ind->ti    = ss_data->ti;
      memset (&mnss_end_ind->fac_inf, 0, sizeof (T_fac_inf)) ;

      SET_SS_STATE(ss_data->ss_state,ss_data->ti,SS_IDLE); /*lint !e502 (Warning -- Warning 502: Expected unsigned type ) */

      PSENDX (MMI, mnss_end_ind);
      {
        PALLOC (mmss_release_req, MMSS_RELEASE_REQ);
        mmss_release_req->ti = ss_data->ti;
        PSENDX (MM, mmss_release_req);
      }

      MM_EM_MM_CONNECTION_ABORTED;

      break;
    }

    default:
      break;
  }
  PFREE (mmss_error_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_mmss_establish_cnf      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSS_ESTABLISH_CNF.

*/
GLOBAL void ss_mmss_establish_cnf (T_MMSS_ESTABLISH_CNF *mmss_establish_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("ss_mmss_establish_cnf()");

  ss_data->ti = mmss_establish_cnf->ti;

  switch (GET_SS_STATE(ss_data->ss_state,ss_data->ti))
  {
    case SS_CONNECTION_PENDING:
      CCD_START;
      {
        T_MNSS_BEGIN_REQ * mnss_begin_req;
        MCAST (ss_register, U_SS_REGISTER);

        ss_register->msg_type    = U_SS_REGISTER;
        mnss_begin_req = (T_MNSS_BEGIN_REQ *)P2D(ss_data->prim[ss_data->ti]);

        
        ss_register->ss_version.c_ver = mnss_begin_req->ss_ver.len;
        if(ss_register->ss_version.c_ver)
        {
          ss_register->v_ss_version = TRUE; 
          memcpy (ss_register->ss_version.ver, 
                  mnss_begin_req->ss_ver.ver,
                  ss_register->ss_version.c_ver);
        }
        else
        {
          ss_register->v_ss_version = FALSE; 
        }
       
        ss_register->ss_facility.c_fac_info = mnss_begin_req->fac_inf.l_fac >> 3;
        if(ss_register->ss_facility.c_fac_info)
        {
          ss_register->v_ss_facility = TRUE;
          memcpy (ss_register->ss_facility.fac_info,
                  &mnss_begin_req->fac_inf.fac[mnss_begin_req->fac_inf.o_fac >>3],
                  ss_register->ss_facility.c_fac_info);
        }
        else
        {
          ss_register->v_ss_facility = FALSE;
        }

        SET_SS_STATE(ss_data->ss_state,ss_data->ti,SS_CONNECTED); /*lint !e502 (Warning -- Warning 502: Expected unsigned type ) */

        PFREE (P2D(ss_data->prim[ss_data->ti]));
        ss_data->prim[ss_data->ti] = NULL;
        ss_for_data_req ((USHORT)(LEN_U_SS_REGISTER + ss_register->ss_facility.c_fac_info * 8));

        MM_EM_MM_CONNECTION_ESTABLISHED;  
      
      }
      CCD_END;
      break;

    default:
      break;
  }
  PFREE (mmss_establish_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_mmss_release_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSS_RELEASE_IND.

*/
GLOBAL void ss_mmss_release_ind (T_MMSS_RELEASE_IND *mmss_release_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("ss_mmss_release_ind()");

  ss_data->ti = mmss_release_ind->ti;

  switch (GET_SS_STATE(ss_data->ss_state,ss_data->ti))
  {
    case SS_CONNECTION_PENDING:
      PFREE (P2D(ss_data->prim[ss_data->ti]));
      ss_data->prim[ss_data->ti] = NULL;

      MM_EM_MM_CONNECTION_FAILED;
      
      /*FALLTHROUGH*/
    case SS_CONNECTED:
    {
      PALLOC (mnss_end_ind, MNSS_END_IND);

      mnss_end_ind->ti = ss_data->ti;
      mnss_end_ind->cause = mmss_release_ind->cause;
      memset (&mnss_end_ind->fac_inf, 0, sizeof (T_fac_inf));
      SET_SS_STATE(ss_data->ss_state,ss_data->ti,SS_IDLE); /*lint !e502 (Warning -- Warning 502: Expected unsigned type ) */

      MM_EM_MM_CONNECTION_RELEASED;

      PSENDX (MMI, mnss_end_ind);
      break;
    }
    default:
      break;
  }
  PFREE (mmss_release_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_mnss_begin_req          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSS_BEGIN_REQ.

*/
GLOBAL void ss_mnss_begin_req (T_MNSS_BEGIN_REQ *mnss_begin_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("ss_mnss_begin_req()");

  ss_data->ti = mnss_begin_req->ti;

  switch (GET_SS_STATE(ss_data->ss_state,ss_data->ti))
  {
    case SS_IDLE:
    {
      if (ss_data->ti < 7)
      {
#if defined (NEW_FRAME)
        ss_data->prim[ss_data->ti] = (T_PRIM *)D2P(mnss_begin_req);
#else
        ss_data->prim[ss_data->ti] = D2P(mnss_begin_req);
#endif
        {
          PALLOC (mmss_establish_req, MMSS_ESTABLISH_REQ);

          mmss_establish_req->ti = ss_data->ti;
          SET_SS_STATE(ss_data->ss_state,ss_data->ti,SS_CONNECTION_PENDING); /*lint !e502 (Warning -- Warning 502: Expected unsigned type ) */

          PSENDX (MM, mmss_establish_req);

          MM_EM_MM_CONNECTION_STARTED;
         
        }
        return; /* Don't free primitive */
      }
      break;
    }
    default:
      break;
  }
  PFREE (mnss_begin_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_mnss_end_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSS_END_REQ.

*/
GLOBAL void ss_mnss_end_req (T_MNSS_END_REQ *mnss_end_req)
{
  GET_INSTANCE_DATA;	
  TRACE_FUNCTION ("ss_mnss_end_req()");

  ss_data->ti                = mnss_end_req->ti;

  switch (GET_SS_STATE(ss_data->ss_state,ss_data->ti))
  {
    case SS_CONNECTION_PENDING:
      if (ss_data->prim[ss_data->ti] NEQ NULL)
      {
        /* Free stored primitive */
        PFREE (P2D(ss_data->prim[ss_data->ti]));
        ss_data->prim[ss_data->ti] = NULL;
      }

      /* Send MMSS_RELEASE_REQ */
      {
        PALLOC (mmss_release_req, MMSS_RELEASE_REQ); /* T_MMSS_RELEASE_REQ */
        mmss_release_req->ti = ss_data->ti;
        PSENDX (MM, mmss_release_req);
      }

      /* Next state is SS_IDLE */
      SET_SS_STATE(ss_data->ss_state,ss_data->ti,SS_IDLE); /*lint !e502 (Warning -- Warning 502: Expected unsigned type ) */
      break;

    case SS_CONNECTED:
      CCD_START;
      {
        MCAST (ss_rel_comp, B_SS_REL_COMP);

        ss_rel_comp->msg_type      = B_SS_REL_COMP;
        ss_rel_comp->v_ss_cause    = FALSE;
        ss_rel_comp->v_ss_facility = (mnss_end_req->fac_inf.l_fac) ?
                                                       TRUE : FALSE;
        ss_rel_comp->ss_facility.c_fac_info = mnss_end_req->fac_inf.l_fac >> 3;
        memcpy (ss_rel_comp->ss_facility.fac_info,
                &mnss_end_req->fac_inf.fac[mnss_end_req->fac_inf.o_fac >>3],
                ss_rel_comp->ss_facility.c_fac_info);
        if (ss_rel_comp->v_ss_facility)
        {
          ss_for_data_req ((USHORT)(LEN_U_SS_RELEASE_COMPLETE + 
                                    ss_rel_comp->ss_facility.c_fac_info * 8));
        }
        else
        {
          ss_for_data_req (LEN_U_SS_RELEASE_COMPLETE);
        }

        {
          PALLOC (mmss_release_req, MMSS_RELEASE_REQ);

          mmss_release_req->ti = ss_data->ti;
          ss_data->ss_state = SET_SS_STATE(ss_data->ss_state,ss_data->ti,SS_IDLE); /*lint !e502 (Warning -- Warning 502: Expected unsigned type ) */

          PSENDX (MM, mmss_release_req);
        }
      }
      CCD_END;

      MM_EM_MM_RELEASE_COMPLETE_SENT;  

      break;
  }
  PFREE (mnss_end_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_mnss_facility_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MNSS_FACILITY_REQ.

*/

GLOBAL void ss_mnss_facility_req (T_MNSS_FACILITY_REQ *mnss_facility_req)
{
  GET_INSTANCE_DATA;
  CCD_START;
  {
    MCAST (ss_facility, U_SS_FACILITY);

    TRACE_FUNCTION ("ss_mnss_facility_req()");

    switch (GET_SS_STATE(ss_data->ss_state,ss_data->ti))
    {
      case SS_CONNECTED:
        ss_data->ti = mnss_facility_req->ti;
        ss_facility->msg_type = U_SS_FACILITY;
        ss_facility->ss_facility.c_fac_info = mnss_facility_req->fac_inf.l_fac >> 3;
        memcpy (ss_facility->ss_facility.fac_info,
                &mnss_facility_req->fac_inf.fac[mnss_facility_req->fac_inf.o_fac >>3],
                ss_facility->ss_facility.c_fac_info);
        ss_for_data_req ((USHORT)(LEN_U_SS_FACILITY + 
                                  ss_facility->ss_facility.c_fac_info * 8));

        MM_EM_FACILITY_MESSAGE_SENT;

        break;
      default:
        break;
    }
  }
  CCD_END;
  PFREE (mnss_facility_req);
}

/*
 * -------------------------------------------------------------------
 * SIGNAL Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_b_ss_rel_comp           |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal B_SS_REL_COMP.

*/

GLOBAL void ss_b_ss_rel_comp (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("ss_b_ss_rel_comp()");

  switch (GET_SS_STATE(ss_data->ss_state,ss_data->ti))
  {
    case SS_IDLE:
      /* Unrecognised TI : Ignore Message TS 24.010 Section 3.7.3 a)*/
      {
        PALLOC (mmss_release_req, MMSS_RELEASE_REQ); /* T_MMSS_RELEASE_REQ */
        mmss_release_req->ti = ss_data->ti;
        PSENDX (MM, mmss_release_req);
      }
      break;

    case SS_CONNECTED:
      if (! ss_data->est_flag)
      {
        MCAST (ss_rel_comp, B_SS_REL_COMP);
        PALLOC (mnss_end_ind, MNSS_END_IND);

        mnss_end_ind->ti = ss_data->ti;
        if (ss_rel_comp->v_ss_cause)
          mnss_end_ind->cause = CAUSE_MAKE(DEFBY_STD, 
                                           ORIGSIDE_NET, 
                                           SS_ORIGINATING_ENTITY, 
                                           ss_rel_comp->ss_cause.cs);
        else
          mnss_end_ind->cause = CAUSE_MAKE(DEFBY_CONDAT, 
                                           ORIGSIDE_NET, 
                                           SS_ORIGINATING_ENTITY, 
                                           NOT_PRESENT_8BIT);
        memcpy (mnss_end_ind->fac_inf.fac, ss_rel_comp->ss_facility.fac_info,
                ss_rel_comp->ss_facility.c_fac_info);
        mnss_end_ind->fac_inf.l_fac = ss_rel_comp->ss_facility.c_fac_info << 3;
        mnss_end_ind->fac_inf.o_fac = 0;

        SET_SS_STATE(ss_data->ss_state,ss_data->ti,SS_IDLE); /*lint !e502 (Warning -- Warning 502: Expected unsigned type ) */

        PSENDX (MMI, mnss_end_ind);
        {
          PALLOC (mmss_release_req, MMSS_RELEASE_REQ);
          mmss_release_req->ti = ss_data->ti;
          PSENDX (MM, mmss_release_req);
        }

        MM_EM_MM_RELEASE_COMPLETE_RECEIVED;
      }
      break;

    default:
      /* Should never reach here */
      TRACE_FUNCTION("**BAD SS State**");
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_d_ss_facility           |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal D_SS_FACILITY.

*/

GLOBAL void ss_d_ss_facility (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("ss_d_ss_facility()");

  switch( ss_data->error )
  {
    case CAUSE_INVALID_MAND_INFO:
      /* Send RELEASE COMPLETE with this cause TS 24.010 Section 3.7.4*/
      send_rel_comp(CAUSE_INVALID_MAND_INFO);
      return;

    default:
      break;
  }

  switch (GET_SS_STATE(ss_data->ss_state,ss_data->ti))
  {
    case SS_IDLE:
      /* Unrecognised TI : TS 24.010 Section 3.7.3 b)*/
      send_rel_comp(CAUSE_INVALID_TI);
      break;

    case SS_CONNECTED:
    {
      MCAST (ss_facility, U_SS_FACILITY);

      if (! ss_data->est_flag)
      {
        PALLOC (mnss_facility_ind, MNSS_FACILITY_IND);

        mnss_facility_ind->ti = ss_data->ti;

        memcpy (mnss_facility_ind->fac_inf.fac, ss_facility->ss_facility.fac_info,
                ss_facility->ss_facility.c_fac_info);
        mnss_facility_ind->fac_inf.l_fac = ss_facility->ss_facility.c_fac_info << 3;
        mnss_facility_ind->fac_inf.o_fac = 0;

        MM_EM_FACILITY_MESSAGE_RECEIVED;
       
        PSENDX (MMI, mnss_facility_ind);
      }
      break;
    }
    default:
      /* Should never reach here */
      TRACE_FUNCTION("**BAD SS State**");
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_d_ss_register           |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal D_SS_REGISTER.

*/

GLOBAL void ss_d_ss_register (void)
{
  GET_INSTANCE_DATA;
  MCAST (ss_register, U_SS_REGISTER);

  TRACE_FUNCTION ("ss_d_ss_register()");

  switch( ss_data->error )
  {
    case CAUSE_INVALID_MAND_INFO:
      /* Send RELEASE COMPLETE with this cause TS 24.010 Section 3.7.4*/
      send_rel_comp(CAUSE_INVALID_MAND_INFO);
      return;

    default:
      break;
  }

  switch (GET_SS_STATE(ss_data->ss_state,ss_data->ti))
  {
    case SS_CONNECTED:
      /* This TI is already being used therefore ignore Message */
      /* TS 24.010 Section 3.7.3 c) */
      break;

    case SS_IDLE:
      if (ss_data->est_flag AND 
          (ss_data->ti >= 8 AND ss_data->ti < 15))
      {
        PALLOC (mnss_begin_ind, MNSS_BEGIN_IND);

        mnss_begin_ind->ti = ss_data->ti;
        memcpy (mnss_begin_ind->fac_inf.fac, ss_register->ss_facility.fac_info,
                ss_register->ss_facility.c_fac_info);
        mnss_begin_ind->fac_inf.l_fac = ss_register->ss_facility.c_fac_info << 3;
        mnss_begin_ind->fac_inf.o_fac = 0;
        
        SET_SS_STATE(ss_data->ss_state,ss_data->ti,SS_CONNECTED); /*lint !e502 (Warning -- Warning 502: Expected unsigned type ) */

        MM_EM_REGISTER_MESSAGE_RECEIVED;

        PSENDX (MMI, mnss_begin_ind);
      }
      else
      {
        /* REGISTER with set TI flag received. Release the connection */ 
        PALLOC (mmss_release_req, MMSS_RELEASE_REQ); /* T_MMSS_RELEASE_REQ */
        mmss_release_req->ti = ss_data->ti;
        PSENDX (MM, mmss_release_req);
      }
      break;

    default:
      /* Should never reach here */
      TRACE_FUNCTION("**BAD SS State**");
      break;
  }
}

/*
 * -------------------------------------------------------------------
 * Procedures
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : ss_init                    |
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void ss_init (void)
{
  GET_INSTANCE_DATA;
  memset (ss_data, 0, sizeof (T_SS_DATA));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : SS_SS                      |
| STATE   : code                ROUTINE : send_rel_comp              |
+--------------------------------------------------------------------+

  PURPOSE : This functions sends a RELEASE COMPLETE message with a 
            specified 'cause' value

*/

GLOBAL void send_rel_comp(U8 cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("send_rel_comp()");

  {
    MCAST (ss_rel_comp, B_SS_REL_COMP);

    ss_rel_comp->msg_type        = B_SS_REL_COMP;
    ss_rel_comp->v_ss_facility   = FALSE;
    ss_rel_comp->v_ss_cause      = TRUE;

    ss_rel_comp->ss_cause.v_cs2  = TRUE;
    ss_rel_comp->ss_cause.cs2    = CS_GSM_PLMN;
 
    ss_rel_comp->ss_cause.v_loc  = TRUE;
    ss_rel_comp->ss_cause.loc    = LOC_PUB_NET_REMOTE_USER;

    ss_rel_comp->ss_cause.v_rec  = TRUE;
    ss_rel_comp->ss_cause.rec    = 0x00;

    ss_rel_comp->ss_cause.v_cs   = TRUE;
    ss_rel_comp->ss_cause.cs     = cause;

    ss_rel_comp->ss_cause.c_diag = 0x00;

    ss_for_data_req(LEN_U_SS_RELEASE_COMPLETE);
  }

  {
    PALLOC (mmss_release_req, MMSS_RELEASE_REQ);
    mmss_release_req->ti = ss_data->ti;
    PSENDX (MM, mmss_release_req);
  }

  SET_SS_STATE(ss_data->ss_state,ss_data->ti,SS_IDLE); /*lint !e502 (Warning -- Warning 502: Expected unsigned type ) */
}

/*
+--------------------------------------------------------------------+
| | STATE   : code                ROUTINE : ss_check_critical_error    |
+--------------------------------------------------------------------+

  PURPOSE : This function checks wheter a critical error has been 
            detected in the air message. Critical errors which prevent 
            the treatment of an air message are 
            - invalid Message ID
            - mandatory IE missing
            - IE coded as comprehension required missing
*/

GLOBAL BOOL ss_check_critical_error (UINT ss_err)
{
  TRACE_FUNCTION ("cc_check_critical_error ()");
  if (ss_err  EQ CAUSE_INVALID_MAND_INFO       OR 
      ss_err  EQ CAUSE_MESSAGE_TYPE_NOT_IMPLEM OR
      ss_err  EQ ERR_INVALID_MID               OR
      ss_err  EQ CAUSE_SERVICE_NOT_IMPLEM        )
    return (TRUE);
  else
    return (FALSE);
}
#endif /* #ifndef SS_SS_C */
