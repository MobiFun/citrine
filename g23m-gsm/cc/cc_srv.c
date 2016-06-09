/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CC_SRV
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
|  Purpose :  This Modul defines the functions for the common
|             services of the component CC of the mobile station.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CC_SRV_C
#define CC_SRV_C

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
#include "cus_cc.h"
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
| PROJECT : GSM-PS (6147)       MODULE  : CC_SRV                     |
| STATE   : code                ROUTINE : srv_convert_ti             |
+--------------------------------------------------------------------+

  PURPOSE : Converts the transaction identifier to an index in the
            call data.

*/
GLOBAL UBYTE srv_convert_ti (UBYTE ti)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("srv_convert_ti()");

  for (i=0;i<MAX_CC_CALLS;i++)
    if (cc_data->stored_ti_values[i] EQ ti)
    {
      cc_data->ti = ti;
      return i;
    }
  return NOT_PRESENT_8BIT;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_SRV                     |
| STATE   : code                ROUTINE : srv_define_ti              |
+--------------------------------------------------------------------+

  PURPOSE : Allocates a free entry in the call data.

*/

GLOBAL UBYTE srv_define_ti (void)
{
  GET_INSTANCE_DATA;
  UBYTE i;

  TRACE_FUNCTION ("srv_define_ti()");

  for (i=0;i<MAX_CC_CALLS;i++)
  {
    if (cc_data->stored_ti_values[i] == NOT_PRESENT_8BIT)
    {
      cc_data->stored_ti_values[i] = cc_data->ti;
      return i;
    }
  }
  return NOT_PRESENT_8BIT;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_SRV                     |
| STATE   : code                ROUTINE : srv_free_ti                |
+--------------------------------------------------------------------+

  PURPOSE : Frees an allocated entry in the call data.

*/

GLOBAL void srv_free_ti (void)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  UBYTE connection_available = FALSE;

  TRACE_FUNCTION ("srv_free_ti()");

  for (i=0;i<MAX_CC_CALLS;i++)
  {
    if (cc_data->stored_ti_values[i] EQ cc_data->ti)
    {
      cc_data->stored_ti_values[i] = NOT_PRESENT_8BIT;
    }
    if (cc_data->stored_ti_values[i] NEQ NOT_PRESENT_8BIT)
      connection_available = TRUE;
  }
  if (connection_available EQ FALSE)
    cc_data->channel_mode = NAS_CHM_SIG_ONLY;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_REL                     |
| STATE   : code                ROUTINE : srv_free_stored_setup      |
+--------------------------------------------------------------------+

  PURPOSE : Free a stored SETUP or EMERGENCY SETUP message. 

*/

GLOBAL void srv_free_stored_setup (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("srv_free_stored_setup()");

  cc_data->setup_reattempt_ti = NOT_PRESENT_8BIT;
  if (cc_data->stored_setup NEQ NULL)
  {
    PFREE (cc_data->stored_setup);
    cc_data->stored_setup = NULL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_SRV                     |
| STATE   : code                ROUTINE : srv_store_prim             |
+--------------------------------------------------------------------+

  PURPOSE : stores a primitive.

*/

GLOBAL void srv_store_prim (T_PRIM * prim)
{
  GET_INSTANCE_DATA;
#ifdef OPTION_REF
  cc_data->stored_prim [cc_data->stored_prim_in++] = prim;
#else
  memcpy (&cc_data->stored_prim [cc_data->stored_prim_in++],
          prim, sizeof (T_PRIM));
#endif
  if (cc_data->stored_prim_in EQ MAX_STORED_PRIM)
    cc_data->stored_prim_in = 0;

  cc_data->stored_prim_write++;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_SRV                     |
| STATE   : code                ROUTINE : srv_use_stored_prim        |
+--------------------------------------------------------------------+

  PURPOSE : Uses all stored primitives.

*/

GLOBAL void srv_use_stored_prim (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("srv_use_stored_prim()");

  if (cc_data->stored_prim_write > 0)
  {
    cc_data->stored_prim_read +=
      cc_data->stored_prim_write;
    cc_data->stored_prim_write = 0;
    while (cc_data->stored_prim_read NEQ 0)
    {
      T_PRIM * prim;

      cc_data->stored_prim_read--;
#ifdef OPTION_REF
      prim = cc_data->stored_prim[cc_data->stored_prim_out++];
#else
      prim = &cc_data->stored_prim[cc_data->stored_prim_out++];
#endif
      if (cc_data->stored_prim_out EQ MAX_STORED_PRIM)
        cc_data->stored_prim_out = 0;
      cc_pei_primitive (prim);
    }
  }
}

#endif
