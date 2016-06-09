/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_MMI
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
|             protocol stack adapter for the man machine interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_MMIS_C
#define PSA_MMIS_C
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

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_sms.h"
#include "psa_mmi.h"
#include "cmh.h"
#include "cmh_sms.h"
#include "cmh_mmi.h"

/*==== CONSTANTS ==================================================*/
#ifdef FF_HOMEZONE
#define CBHZ_ON_MSG_ID  0xDD
#define CBHZ_OFF_MSG_ID 0xFFFF
#endif /* HOMEZONE */

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIS                |
|                                 ROUTINE : psaMMI_Cbch             |
+-------------------------------------------------------------------+

  PURPOSE : cell broadcast channel request

*/

GLOBAL SHORT psaMMI_Cbch ( void )
{
  TRACE_FUNCTION ("psaMMI_Cbch()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */  
  if(!psa_IsVldOwnId(smsShrdPrm.cbmPrm.cbchOwner)) 
    
    return( -1 );

/*
 *-------------------------------------------------------------------
 * create and send primitive for CBCH request
 *-------------------------------------------------------------------
 */
  if (smsShrdPrm.cbmPrm.cbmHndl NEQ BM0)   /* only if NMI for CBM is set*/
  {
    PALLOC (mmi_cbch_req, MMI_CBCH_REQ);

    /* fill in primitive parameter: CBCH request */
    memcpy ( ( CHAR * ) mmi_cbch_req -> msg_id,
             ( CHAR * ) smsShrdPrm.cbmPrm.msgId,
             sizeof ( mmi_cbch_req -> msg_id ) );

    memcpy ( ( CHAR * ) mmi_cbch_req -> dcs_id,
             ( CHAR * ) smsShrdPrm.cbmPrm.dcsId,
             sizeof ( mmi_cbch_req -> dcs_id ) );

    mmi_cbch_req -> modus = smsShrdPrm.cbmPrm.cbmMode;

    PSENDX (PL, mmi_cbch_req);
  }
  else
  {
    PALLOC (mmi_cbch_req, MMI_CBCH_REQ);

    memset (mmi_cbch_req, NOT_PRESENT_8BIT, sizeof(T_MMI_CBCH_REQ));
    mmi_cbch_req -> modus = MMI_CBCH_STOP;

    PSENDX (PL, mmi_cbch_req);
  }
  return 0;
}

#ifdef FF_HOMEZONE
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIS                |
|                                 ROUTINE : psaMMI_homezone_req     |
+-------------------------------------------------------------------+

  PURPOSE : homezone request

*/

GLOBAL SHORT psaMMI_homezone_req( void )
{
  TRACE_FUNCTION ("psaMMI_homezone_req()");

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */  
  if(!psa_IsVldOwnId(smsShrdPrm.cbmPrm.cbchOwner)) 
  {    
    return( -1 );
  }

/*
 *-------------------------------------------------------------------
 * create and send primitive for CBCH request
 *-------------------------------------------------------------------
 */

  {
    PALLOC (mmi_cbch_req, MMI_CBCH_REQ);

    memset (mmi_cbch_req, NOT_PRESENT_8BIT, sizeof(T_MMI_CBCH_REQ));
  
    /* fill in primitive parameter: CBCH request */
    mmi_cbch_req->modus     = CBCH_HOMEZONE;

    if ( smsShrdPrm.cbmPrm.hzMode EQ CBHZ_MOD_Active )
    {
      mmi_cbch_req->msg_id[0] = CBHZ_ON_MSG_ID;
    }
    else
    {
      mmi_cbch_req->msg_id[0] = CBHZ_OFF_MSG_ID;
    }

    mmi_cbch_req->dcs_id[0] = smsShrdPrm.cbmPrm.hzDcs;
    mmi_cbch_req->dcs_id[1] = smsShrdPrm.cbmPrm.hzTimeout;

    TRACE_EVENT_P2("MMI_CBCH_REQ: modus %d, msg_id %d", 
                    mmi_cbch_req->modus, mmi_cbch_req->msg_id[0]);

    PSENDX (PL, mmi_cbch_req);
  }
  return 0;
}
#endif /* FF_HOMEZONE */


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_MMIS                |
|                                 ROUTINE : psaMMI_Display          |
+-------------------------------------------------------------------+

  PURPOSE : set the parameters for display request

*/

GLOBAL SHORT psaMMI_Display ( void )
{

  TRACE_FUNCTION ("psaMMI_Display()");

/*
 *-------------------------------------------------------------------
 * create and send primitive for display request 
 *-------------------------------------------------------------------
 */
  {
    PSENDX (PL, mmiShrdPrm.dspRq);
  }

  return 0;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS                MODULE  : PSA_MMIS                |
|                                 ROUTINE : psaMMI_ConfigPrim       |
+-------------------------------------------------------------------+

  PURPOSE : send a config message to a entity

*/
#define CONFIG_STR 0x0

typedef struct
{
  char config_str[80];
} T_CONFIG_STR;


GLOBAL void psaMMI_ConfigPrim (T_HANDLE receiver_handle, 
                               char *config_msg)
{
  PALLOC (cfg_str, CONFIG_STR);

  D2P(cfg_str)->opc |= SYS_MASK;
  strcpy (cfg_str->config_str, config_msg);

  PSEND (receiver_handle, cfg_str);
}


/*==== EOF ========================================================*/

