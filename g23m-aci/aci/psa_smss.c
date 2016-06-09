/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SMSS
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
|             protocol stack adapter for the short message service SMS.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_SMSS_C
#define PSA_SMSS_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "psa.h"
#include "psa_sms.h"
#include "cmh_sms.h"
/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

GLOBAL SHORT psaSMS_ReadReq ( UBYTE mem_type,  USHORT rec_num, 
                              UBYTE read_mode, T_ACI_SMS_STAT cmh_state )
{
  UBYTE psa_state;

  TRACE_FUNCTION ("psaSMS_ReadReq()");

  /*
   *-------------------------------------------------------------------
   * create and send primitive for SMS read
   *-------------------------------------------------------------------
   */
  {
    PALLOC (mnsms_read_req, MNSMS_READ_REQ);

    mnsms_read_req -> mem_type  = mem_type;
    mnsms_read_req -> rec_num   = (UBYTE)rec_num;
    mnsms_read_req -> read_mode = read_mode;
    if (cmhSMS_getStatPsa(cmh_state, &psa_state))
      mnsms_read_req -> status = psa_state;
    else
      mnsms_read_req -> status = NOT_PRESENT_8BIT;
      
    PSENDX (SMS, mnsms_read_req);
  }

  return 0;
}

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SMSS                     |
|                            ROUTINE : psaSMS_PauseReq              |
+-------------------------------------------------------------------+

  ACI-ENH-19450: Added for %CPRMS command (MDF-07.04.2004)
  
  PURPOSE : build and send MNSMS_PAUSE_REQ to SMS

*/

GLOBAL void psaSMS_PauseReq ( )
{

  TRACE_FUNCTION ("psaSMS_PauseReq()");

  /* build and send MNSMS_PAUSE_REQ */
  {
    PALLOC (mnsms_pause_req, MNSMS_PAUSE_REQ);      
    PSENDX (SMS, mnsms_pause_req);
  }
  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SMSS                     |
|                            ROUTINE : psaSMS_ResumeReq             |
+-------------------------------------------------------------------+

  ACI-ENH-19450: Added for %CPRMS command (MDF-07.04.2004)
  
  PURPOSE : build and send MNSMS_RESUME_REQ to SMS

*/

GLOBAL void psaSMS_ResumeReq ( )
{

  TRACE_FUNCTION ("psaSMS_ResumeReq()");

  /* build and send MNSMS_PAUSE_REQ */
  {
    PALLOC (mnsms_resume_req, MNSMS_RESUME_REQ);      
    PSENDX (SMS, mnsms_resume_req);
  }
  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SMSS                     |
|                            ROUTINE : psaSMS_TestReq             |
+-------------------------------------------------------------------+

  ACI-ENH-19450: Added for %CPRMS command (MDF-07.04.2004)
  
  PURPOSE : build and send MNSMS_RESUME_REQ to SMS

*/

GLOBAL void psaSMS_QueryReq (UBYTE query_type)
{

  TRACE_FUNCTION ("psaSMS_QueryReq()");

  /* build and send MNSMS_QUERY_REQ */
  {
    PALLOC (mnsms_query_req, MNSMS_QUERY_REQ);

    mnsms_query_req->query_type = query_type; 
    
    PSENDX (SMS, mnsms_query_req);
  }
  
}
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */

/*==== EOF ========================================================*/
 
