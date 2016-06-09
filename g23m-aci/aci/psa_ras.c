/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_RAS
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
|  Purpose :  This module defines the signalling functions of the
|             protocol stack adapter for rate adaptation.
+----------------------------------------------------------------------------- 
*/ 

#ifdef DTI

#ifndef PSA_RAS_C
#define PSA_RAS_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "psa.h"
#include "aci_io.h"
#include "psa_ra.h"
#include "ra_l1int.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_RA                       |
|                            ROUTINE : psaRA_Activate               |
+-------------------------------------------------------------------+

  PURPOSE : Activation RA.

*/

GLOBAL void psaRA_Activate(void)
{
#define X raShrdPrm.set_prm[raShrdPrm.owner]

  TRACE_FUNCTION ("psaRA_Activate()");

#ifdef USE_L1FD_FUNC_INTERFACE
  {
    T_RA_ACTIVATE_REQ ra_activate_req;

    ra_activate_req.model     = X.model;
    ra_activate_req.tra_rate  = X.tra_rate;
    ra_activate_req.user_rate = X.user_rate;
    ra_activate_req.ndb       = X.ndb;
    ra_activate_req.nsb       = X.nsb;

    l1i_ra_activate_req (&ra_activate_req);
  }
#else
  {
    PALLOC (ra_activate_req, RA_ACTIVATE_REQ);

    ra_activate_req->model     = X.model;
    ra_activate_req->tra_rate  = X.tra_rate;
    ra_activate_req->user_rate = X.user_rate;
    ra_activate_req->ndb       = X.ndb;
    ra_activate_req->nsb       = X.nsb;

    PSENDX (RA, ra_activate_req);
  }
#endif

#undef X
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_RA                       |
|                            ROUTINE : psaRA_Deactivate             |
+-------------------------------------------------------------------+

  PURPOSE : Deactivation of RA.

*/

GLOBAL void psaRA_Deactivate (void)
{
  TRACE_FUNCTION ("psaRA_Deactivate()");

#ifdef USE_L1FD_FUNC_INTERFACE
  {
    T_RA_DEACTIVATE_REQ ra_deactivate_req;

    l1i_ra_deactivate_req (&ra_deactivate_req);
  }
#else
  {
    PALLOC (ra_deactivate_req, RA_DEACTIVATE_REQ);

    PSENDX (RA, ra_deactivate_req);
  }
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_RA                       |
|                            ROUTINE : psaRA_Modify                 |
+-------------------------------------------------------------------+

  PURPOSE : Modification of RA in case of CMM.

*/

GLOBAL void psaRA_Modify (void)
{
#define X raShrdPrm.set_prm[raShrdPrm.owner]

  TRACE_FUNCTION ("psaRA_Modify()");

#ifdef USE_L1FD_FUNC_INTERFACE
  {
    T_RA_MODIFY_REQ ra_modify_req;

    ra_modify_req.tra_rate  = X.tra_rate;
    ra_modify_req.user_rate = X.user_rate;

    l1i_ra_modify_req (&ra_modify_req);
  }
#else
  {
    PALLOC (ra_modify_req, RA_MODIFY_REQ);

    ra_modify_req->tra_rate  = X.tra_rate;
    ra_modify_req->user_rate = X.user_rate;

    PSENDX (RA, ra_modify_req);
  }
#endif
}

#endif /* DTI */
/*==== EOF ========================================================*/
