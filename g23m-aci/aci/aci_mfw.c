/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_MFW
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
|  Purpose :  Only for testing of Conc SMS via the ATI.  
+----------------------------------------------------------------------------- 
*/ 


#ifndef ACI_MFW_C
#define ACI_MFW_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"

#include "aci_fd.h"
#include "aci_mem.h"

#include "psa.h"
#include "psa_sms.h"

#include "cmh.h"
#include "cmh_sms.h"

#include "psa_cc.h"

#include "aci_lst.h"


#if defined _CONC_TESTING_ AND defined TI_PS_FF_CONC_SMS

#include "conc_sms.h"


#include "aci_mfw.h"




//#define TRACE_CCD /* for CCD Debugging */


/**********************************************************************
 *                    TEMPORARY ADAPTION FUNCTIONS                    *
 *********************************************************************/
//ALL THE FUNCTIONS BELOW ARE COMMENTED OUT AS THEY WERE FOR TESITNG ONLY


GLOBAL void rAT_PlusCMTI_Ext( T_ACI_SMS_STOR  mem,
                              UBYTE           index,
                              T_ACI_CMGL_SM*  sm)
{
  
  T_SMS_TYPE smsType;
  T_CONC_ASSEMBLY_RETURN ret;
  T_SM_DATA_EXT data_conc;
  
  TRACE_FUNCTION ("rAT_PlusCMTI_Ext");

  smsType = SMS_getSMSType ( &sm->udh, sm->adress, MODE2);

  if (smsType EQ CONCATE)
  {
    ret = concSMS_Collect ( &data_conc, sm, 1, mem);

    if (ret EQ CONC_COMPLETED)
    {
      UBYTE i;
      for (i=0; i<CMD_SRC_MAX; i++)
        io_sendMessage(i, (CHAR*)data_conc.data, ATI_NORMAL_OUTPUT);
      ACI_MFREE(data_conc.data);
    }

  }
  TRACE_EVENT_P1("smsType: %d", smsType);
  rAT_PlusCMTI( mem, index, sm );
}



GLOBAL void rAT_PlusCMT_Ext (T_ACI_CMGL_SM* sm)
{  
  
  T_SMS_TYPE smsType;
  T_CONC_ASSEMBLY_RETURN ret;
  T_SM_DATA_EXT data_conc;

  TRACE_FUNCTION ("rAT_PlusCMT_Ext");


  smsType = SMS_getSMSType ( &sm->udh, sm->adress, MODE1);

  if (smsType EQ CONCATE)
  {
    ret = concSMS_Collect ( &data_conc, sm, 0, SMS_STOR_NotPresent);

    if (ret EQ CONC_COMPLETED)
    {
      UBYTE i;
      for (i=0; i<CMD_SRC_MAX; i++)
        io_sendMessage(i, (CHAR*)data_conc.data, ATI_NORMAL_OUTPUT);
      ACI_MFREE(data_conc.data);
    }
  }
  rAT_PlusCMT(sm);
}


GLOBAL void rAT_PlusCMGR_Ext (T_ACI_CMGL_SM*  sm,
                              T_ACI_CMGR_CBM* cbm)
{  
  
  T_SMS_TYPE smsType;
  T_CONC_ASSEMBLY_RETURN ret;
  T_SM_DATA_EXT data_conc;
  
  TRACE_FUNCTION ("rAT_PlusCMGR_Ext");

  smsType = SMS_getSMSType ( &sm->udh, sm->adress, MODE3);

  if (smsType EQ CONCATE)
  {
    ret = concSMS_Collect ( &data_conc, sm, 0, SMS_STOR_NotPresent);

    if (ret EQ CONC_COMPLETED)
    {
      UBYTE i;
      for (i=0; i<CMD_SRC_MAX; i++)
        io_sendMessage(i, (CHAR*)data_conc.data, ATI_NORMAL_OUTPUT);
      ACI_MFREE(data_conc.data);
    }
  }
  rAT_PlusCMGR ( sm, NULL );
}

GLOBAL void rAT_PercentCMGR_Ext (T_ACI_CMGL_SM*  sm,
                                 T_ACI_CMGR_CBM* cbm)
{  
  
  T_SMS_TYPE smsType;
  T_CONC_ASSEMBLY_RETURN ret;
  T_SM_DATA_EXT data_conc;
  
  TRACE_FUNCTION ("rAT_PercentCMGR_Ext");

  smsType = SMS_getSMSType ( &sm->udh, sm->adress, MODE3);

  if (smsType EQ CONCATE)
  {
    ret = concSMS_Collect ( &data_conc, sm, 0, SMS_STOR_NotPresent);

    if (ret EQ CONC_COMPLETED)
    {
      UBYTE i;
      for (i=0; i<CMD_SRC_MAX; i++)
        io_sendMessage(i, (CHAR*)data_conc.data, ATI_NORMAL_OUTPUT);
      ACI_MFREE(data_conc.data);
    }
  }
  rAT_PercentCMGR ( sm, NULL );
}

#if !defined _SIMULATION_ AND !defined SMI AND !defined MFW AND !defined FF_MMI_RIV

/* This is for conc. testing when compiling for ACI only */

GLOBAL void rAT_PlusCMGS  ( UBYTE            mr,
                            UBYTE            numSeg) {}

GLOBAL void rAT_PlusCMS   ( T_ACI_AT_CMD     cmdId,
                            T_ACI_CMS_ERR    err,
                            T_EXT_CMS_ERROR* conc_error) {}

GLOBAL void rAT_PlusCMSS  ( UBYTE            mr,
                            UBYTE            numSeg) {}

GLOBAL void rAT_PlusCMGD  ( ) {}

GLOBAL void rAT_PlusCMGW  ( UBYTE            index,
                            UBYTE            numSeg,
                            UBYTE            mem) {}

GLOBAL void rAT_PlusCMGC  ( UBYTE            mr) {}

GLOBAL void rAT_PlusCMGR  ( T_ACI_CMGL_SM*   sm,
                            T_ACI_CMGR_CBM*  cbm ) {}

GLOBAL void rAT_PlusCMTI  ( T_ACI_SMS_STOR  mem,
                            UBYTE           index,
                            T_ACI_CMGL_SM*  sm) {}

GLOBAL void rAT_PlusCMT   ( T_ACI_CMGL_SM*  sm) {}


#endif /* #if !defined _SIMULATION_ AND !defined SMI AND !defined MFW AND !defined FF_MMI_RIV */

#endif /* #ifdef _CONC_TESTING_ */
