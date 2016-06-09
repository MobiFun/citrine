/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  SMS_CSF
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
|  Purpose :  This Modul defines the custom specific functions
|             for the component SMS of the mobile station
|             
|             Use this module to integrate the entity
|             in your target system !
+----------------------------------------------------------------------------- 
*/ 

#ifndef SMS_CSF_C
#define SMS_CSF_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SMS

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "message.h"
#include "ccdapi.h"
#include "prim.h"
#include "cus_sms.h"
#include "cnf_sms.h"
#include "mon_sms.h"
#include "pei.h"
#include "tok.h"
#include "sms.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CSF                    |
| STATE   : code                ROUTINE : csf_get_sms_instance       |
+--------------------------------------------------------------------+

  PURPOSE : Checks the transaction identitfier and sets the instance.

*/

GLOBAL T_SMS_DATA* csf_get_sms_instance (UBYTE ti)
{
  GET_INSTANCE_DATA;
  
  TRACE_FUNCTION ("csf_get_sms_instance()");

  if (ti < 8)
  {
    /*
     * mobile originated transaction 
     */
    sms_data->inst = INST_MO;

    return sms_data;
  }

  if (ti >= 8)
  {
    /*
     * mobile terminated transaction 
     */
    if (sms_data->data[INST_MT].ti EQ ti)
    {
      sms_data->inst = INST_MT;

      return sms_data;
    }
    else
      return NULL;
  }

  return NULL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CSF                    |
| STATE   : code                ROUTINE : csf_get_new_sms_instance   |
+--------------------------------------------------------------------+

  PURPOSE : Checks the transaction identitfier and sets the instance.

*/
GLOBAL UBYTE csf_get_new_mo_ti()
{
  T_SMS_DATA *sms_data = GET_INSTANCE(0);
  UBYTE ti = sms_data->data[INST_MO].ti+1;
 /*
  * mobile originated transaction, valid ti has to be in set [0..6]
  */
  if (ti >= 7)
  {
     ti=0;
  }
  return ti;
}

GLOBAL T_SMS_DATA * csf_get_new_sms_instance (UBYTE ti)
{
  T_SMS_DATA *sms_data = GET_INSTANCE(0);
  
  TRACE_FUNCTION ("csf_get_new_sms_instance()");

  if (ti < 8)
  {
    /*
     * mobile originated transaction, valid ti has to be in set [0..6]
     */
    sms_data->inst = INST_MO;
   // sms_data->data[INST_MO].ti++;
   // if (sms_data->data[INST_MO].ti >= 7)
   //    sms_data->data[INST_MO].ti = 0;

    return sms_data;
  }

  if (ti >= 8)
  {
    /*
     * mobile terminated transaction
     */
    if (sms_data->data[INST_MT].ti EQ 0)
    {
      /* 
       * The mobile terminated part of SMS is IDLE
       */
      sms_data->inst       = INST_MT;
      sms_data->data[INST_MT].ti = ti;

      return sms_data;
    }
  }

  return NULL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CSF                    |
| STATE   : code                ROUTINE : csf_free_sms_instance      |
+--------------------------------------------------------------------+

  PURPOSE : Checks the transaction identifier and frees the instance.

*/

GLOBAL void csf_free_sms_instance (UBYTE ti)
{
  T_SMS_DATA *sms_data = GET_INSTANCE(0);

  TRACE_FUNCTION_P1 ("csf_free_sms_instance(TI=%u)", ti);

//#if defined (GPRS)
//  SMS_INST.downlink = SMS_DOWNLINK_NONE;
//#endif 

  if (ti >= 8)
  {
    /*
     * mobile terminated transaction 
     */
    if (sms_data->data[INST_MT].ti EQ ti)
      sms_data->data[INST_MT].ti = 0;

    if (SMS_RP_RCVD(sms_data) NEQ NULL)
    {
      MFREE (SMS_RP_RCVD(sms_data));
      SMS_RP_RCVD(sms_data) = NULL;
    }
  }
  /* Commented out as this is used for 
     retransmission of the failed message 
  if (SMS_DATA_REQ(sms_data) NEQ NULL)
  {
    PFREE (SMS_DATA_REQ(sms_data));
    SMS_DATA_REQ(sms_data) = NULL;
  }
  */
  if (SMS_SDU(sms_data) NEQ NULL)
  {
    MFREE (SMS_SDU(sms_data));
    SMS_SDU(sms_data) = NULL;
  }
  if (SMS_SIM_READ(sms_data) NEQ NULL)
  {
    PFREE (SMS_SIM_READ(sms_data));
    SMS_SIM_READ(sms_data) = NULL;
  }
}

#endif /* #ifndef SMS_CSF_C */
