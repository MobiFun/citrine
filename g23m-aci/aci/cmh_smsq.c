/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SMSQ
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
|  Purpose :  This module provides the query functions related to the 
|             protocol stack adapter for short message service.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_SMSQ_C
#define CMH_SMSQ_C
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
#include "cmh.h"
#include "cmh_sms.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/* parameter block for delayed rAT_PlusCPMS call */
  LOCAL struct qAT_PlusCPMS_buffer { 
    T_ACI_CMD_SRC srcId;
    T_ACI_SMS_STOR_OCC m1;
    T_ACI_SMS_STOR_OCC m2;
    T_ACI_SMS_STOR_OCC m3;
  } qAT_PlusCPMS_buffer;


/*==== PROTOTYPES =================================================*/
LOCAL UCHAR qAT_PlusCPMS_delayed (void* arg);

/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCSMP             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CSMP? 
            AT command which returns the current settings of the
            text mode parameter.

            <fo>:    first octet of SMS-SUBMIT
            <vprel>: relative validity period
            <vpabs>: absolute validity period
            <vpenh>: enhanced validity period
            <pid>:   TP-Protocol-Identifier
            <dcs>:   SMS data coding scheme 
*/
GLOBAL T_ACI_RETURN qAT_PlusCSMP  ( T_ACI_CMD_SRC srcId,
                                    SHORT*        fo,
                                    SHORT*        vprel,
                                    T_ACI_VP_ABS* vpabs,
                                    T_ACI_VP_ENH* vpenh,
                                    SHORT*        pid,
                                    SHORT*        dcs )
{
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set */

  TRACE_FUNCTION ("qAT_PlusCSMP ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */
  if(!cmh_IsVldCmdSrc (srcId)) 
  {
    return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */  
  if(!cmhSMS_checkSIM ()) 
    return AT_FAIL;

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  /*
   *-----------------------------------------------------------------
   * fill in parameters
   *-----------------------------------------------------------------
   */  
  *fo    = pSMSSetPrm -> msgType;

  *vprel = pSMSSetPrm -> vpRel;

  if (vpabs)
    cmhSMS_setVpabsCmh ( vpabs, &pSMSSetPrm -> vpAbs );

  if (vpenh)
    cmhSMS_setVpenhCmh ( vpenh, &pSMSSetPrm -> vpEnh );
  
  *pid   = pSMSSetPrm -> pid;
  *dcs   = pSMSSetPrm -> dcs;

  return ( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCPMS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPMS? 
            AT command which returns the current settings and usage
            of the memory for writing, reading, ... of SMS.

            <mem1>: memory information, reading, deleting
            <mem2>: memory information, writing, sending
            <mem3>: memory information, storing
*/
GLOBAL T_ACI_RETURN qAT_PlusCPMS  ( T_ACI_CMD_SRC srcId )
{
  T_ACI_RETURN  ret;          /* AT response code            */


  TRACE_FUNCTION ("qAT_PlusCPMS ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret)) 
    return ret;

  if (!cmh_set_delayed_call (qAT_PlusCPMS_delayed, &qAT_PlusCPMS_buffer))
    return AT_BUSY;
  
  /*
   *-----------------------------------------------------------------
   * Fills the T_ACI_SMS_STOR_OCC structure with data from the
   * shared parameter buffer (used mem, total mem).
   *-----------------------------------------------------------------
   */
  qAT_PlusCPMS_buffer.srcId = srcId;
  cmhSMS_setStorOcc ( &qAT_PlusCPMS_buffer.m1, smsShrdPrm.mem1 );
  cmhSMS_setStorOcc ( &qAT_PlusCPMS_buffer.m2, smsShrdPrm.mem2 );
  cmhSMS_setStorOcc ( &qAT_PlusCPMS_buffer.m3, smsShrdPrm.mem3 );

#ifdef FF_ATI
  cpmsCallType = QAT_CALL;
#endif /* FF_ATI */
  TRACE_EVENT("delayed return requested: 100 ms");
  cmh_start_delayed_call (100);
  
  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCPMS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPMS? 
            AT command which returns the current settings and usage
            of the memory for writing, reading, ... of SMS.

            <sim_total>: memory information, total available in SIM
            <sim_used>: memory information, Used in SIM
            <me_total>: memory information, total available in ME
            <me_used>   memory information, Used in ME
*/
/* provide immediate return in case of reading */
GLOBAL T_ACI_RETURN qAT_PlusCPMS_ext  ( T_ACI_CMD_SRC srcId, UBYTE *sim_total, UBYTE *sim_used, UBYTE *me_total, UBYTE *me_used )
{
  TRACE_FUNCTION ("qAT_PlusCPMS_ext()");

  *sim_total = smsShrdPrm.aci_sms_parameter.simTotal;
  *sim_used = smsShrdPrm.aci_sms_parameter.simUsed;
  *me_total = smsShrdPrm.aci_sms_parameter.meTotal;
  *me_used = smsShrdPrm.aci_sms_parameter.meUsed;

  TRACE_EVENT_P4("sim_total = %d, sim_used = %d, me_total = %d, me_used = %d", *sim_total , *sim_used, *me_total, *me_used);
  return AT_CMPL;
}

/* function for delivering delayed rAT_PlusCPMS callback */
LOCAL UCHAR qAT_PlusCPMS_delayed (void* arg)
{
  struct qAT_PlusCPMS_buffer* p = (struct qAT_PlusCPMS_buffer*) arg;
  TRACE_EVENT("delayed delivery of RAT_CPMS after qAT_PlusCPMS");
  R_AT ( RAT_CPMS, p->srcId ) ( &p->m1, &p->m2, &p->m3 );
  R_AT ( RAT_OK, p->srcId ) ( AT_CMD_CPMS );
  return FALSE; /* single-shot */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCSMS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CSMS?
            AT command which returns the current settings of the
            message service.

            <service>: message service
            <mt>:      mobile terminated messages
            <mo>:      mobile originated messages
            <bm>:      broadcast type messages
*/
GLOBAL T_ACI_RETURN qAT_PlusCSMS  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_CSMS_SERV *service,
                                    T_ACI_CSMS_SUPP *mt,
                                    T_ACI_CSMS_SUPP *mo,
                                    T_ACI_CSMS_SUPP *bm )
{
  TRACE_FUNCTION ("qAT_PlusCSMS ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if( ! cmh_IsVldCmdSrc (srcId) ) 
  {    
    return ( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */  
  if(!cmhSMS_checkSIM ()) 
    return AT_FAIL;

  /*
   *-----------------------------------------------------------------
   * fill in parameters
   *-----------------------------------------------------------------
   */  
  *service = smsShrdPrm.CSMSservice;
  *mt      = smsShrdPrm.CSMSmt;
  *mo      = smsShrdPrm.CSMSmo;
  *bm      = smsShrdPrm.CSMSbm;

  return ( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCMGF             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CMGF?
            AT command which returns the current settings of the
            message format.

            <mode>: indicates the message format
*/
GLOBAL T_ACI_RETURN qAT_PlusCMGF ( T_ACI_CMD_SRC   srcId,
                                   T_ACI_CMGF_MOD* mode )
{
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set */

  TRACE_FUNCTION ("qAT_PlusCMGF ()");


  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if( ! cmh_IsVldCmdSrc (srcId) ) 
  {    
    return ( AT_FAIL );
  }

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  /*
   *-----------------------------------------------------------------
   * fill in parameters
   *-----------------------------------------------------------------
   */  
  *mode = pSMSSetPrm -> CMGFmode;

  return ( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PercentSMBS          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %SMBS?
            <mode>: indicates whether presenting of both text and pdu modes is enabled
*/
#ifdef FF_MMI_RIV
GLOBAL T_ACI_RETURN qAT_PercentSMBS ( T_ACI_CMD_SRC       srcId,
                                      T_ACI_PERC_SMBS_MOD *mode )
{
  TRACE_FUNCTION ("qAT_PercentSMBS ()");

  /* check command source */  
  if( ! cmh_IsVldCmdSrc (srcId) ) 
  {    
    return ( AT_FAIL );
  }

  /* fill in parameters */  
  *mode = smsShrdPrm.perccmgf_smbs_mode;

  return ( AT_CMPL );
}
#endif /* #ifdef FF_MMI_RIV */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCSCA             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CSCA?
            AT command which returns the current settings of the 
            service center address.

            <sca>:   service center address
            <tosca>: type of service center address
*/
GLOBAL T_ACI_RETURN qAT_PlusCSCA ( T_ACI_CMD_SRC srcId,
                                   CHAR*         sca,
                                   T_ACI_TOA*    tosca )
{
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set */

  TRACE_FUNCTION ("qAT_PlusCSCA ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */
  if(srcId NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT)
  {    
    if(!cmh_IsVldCmdSrc (srcId))
    {
      return( AT_FAIL );
    }
  }
  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */  
  if(!cmhSMS_checkSIM ()) 
    return AT_FAIL;

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  /*
   *-----------------------------------------------------------------
   * fill in parameters
   *-----------------------------------------------------------------
   */  
  tosca -> ton = cmhSMS_getTon ( pSMSSetPrm -> sca.ton );
  tosca -> npi = cmhSMS_getNpi ( pSMSSetPrm -> sca.npi );

  cmhSMS_getAdrStr ( sca, 
                     MAX_SMS_NUM_LEN - 1,
                     pSMSSetPrm -> sca.num,
                     pSMSSetPrm -> sca.c_num );

  return ( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCNMI             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CNMI?
            AT command which returns the current settings of the 
            new message indication parameter.

            <mt>: indication routing for SMS-DELIVER
            <bm>: indication routing for CBM
            <ds>: indication routing for SMS-STATUS-REPORT
*/
GLOBAL T_ACI_RETURN qAT_PlusCNMI ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_CNMI_MT* mt,
                                   T_ACI_CNMI_BM* bm,
                                   T_ACI_CNMI_DS* ds)
{
  TRACE_FUNCTION ("qAT_PlusCNMI ()");


  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */
  if(!cmh_IsVldCmdSrc (srcId)) 
  {    
    return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */  
  if(!cmhSMS_checkSIM ()) 
    return AT_FAIL;

  /*
   *-----------------------------------------------------------------
   * fill in parameters
   *-----------------------------------------------------------------
   */  
  *mt = smsShrdPrm.CNMImt;
  *bm = smsShrdPrm.CNMIbm;
  *ds = smsShrdPrm.CNMIds;

  return ( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCSCB             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CSCB?
            AT command which returns the current selected cell
            broadcast message types.

            <mode>: acception mode 
            <mids>: message identifiers
            <dcss>: data coding schemes
*/
GLOBAL T_ACI_RETURN qAT_PlusCSCB ( T_ACI_CMD_SRC   srcId,
                                   T_ACI_CSCB_MOD* mode,
                                   USHORT*         mids,
                                   UBYTE*          dcss)
{

  TRACE_FUNCTION ("qAT_PlusCSCB ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */
  if(!cmh_IsVldCmdSrc (srcId)) 
  {    
    return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */  
  if(!cmhSMS_checkSIM ()) 
    return AT_FAIL;

  /*
   *-----------------------------------------------------------------
   * fill in parameters
   *-----------------------------------------------------------------
   */
  memcpy ( ( CHAR * ) mids, ( CHAR * ) smsShrdPrm.cbmPrm.msgId,
           sizeof ( smsShrdPrm.cbmPrm.msgId ) );

  memcpy ( ( CHAR * ) dcss, ( CHAR * ) smsShrdPrm.cbmPrm.dcsId,
           sizeof ( smsShrdPrm.cbmPrm.dcsId ) );

  if (smsShrdPrm.cbmPrm.cbmMode NEQ CBCH_IGNORE)
    *mode = CSCB_MOD_Accept;
  else
    *mode = CSCB_MOD_NotAccept;

  return ( AT_CMPL );
}


#ifdef FF_HOMEZONE
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PercentCBHZ          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CBHZ?
            AT command which returns the current selected homezone
            parameters.

            <mode>   : acctivation mode 
            <dcs>    : data coding scheme for homezone period
            <timeout>: timeout period for homezone CBM
*/
GLOBAL T_ACI_RETURN qAT_PercentCBHZ ( T_ACI_CMD_SRC   srcId,
                                      T_ACI_CBHZ_MOD* mode,
                                      T_ACI_CS*       dcs,
                                      UBYTE*          timeout)
{

  TRACE_FUNCTION ("qAT_PercentCBHZ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */
  if(!cmh_IsVldCmdSrc (srcId)) 
  {    
    return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */  
  if(!cmhSMS_checkSIM ()) 
    return AT_FAIL;

  /*
   *-----------------------------------------------------------------
   * fill in parameters
   *-----------------------------------------------------------------
   */
  *mode    = (T_ACI_CBHZ_MOD)smsShrdPrm.cbmPrm.hzMode;
  *dcs     = (T_ACI_CS)smsShrdPrm.cbmPrm.hzDcs;
  *timeout = smsShrdPrm.cbmPrm.hzTimeout;  

  return ( AT_CMPL );
}

#endif /* FF_HOMEZONE */

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PercentCMGRS         |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CMGRS?
            AT command which returns the current settings of the
            auto retransmission flag.

*/
GLOBAL T_ACI_RETURN qAT_PercentCMGRS  ( T_ACI_CMD_SRC    srcId,
                                        UBYTE *auto_rep_flag )
{
  TRACE_FUNCTION ("qAT_PercentCMGRS ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if( ! cmh_IsVldCmdSrc (srcId) ) 
  {    
    return ( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */  
  if(!cmhSMS_checkSIM ()) 
    return AT_FAIL;

  /*
   *-----------------------------------------------------------------
   * fill in parameters
   *-----------------------------------------------------------------
   */  
  *auto_rep_flag = smsShrdPrm.auto_repeat_flag;  

  return ( AT_CMPL );
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCMMS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CMMS? 
            AT command which returns the current mode setting.

            <mode>:    mode for +CMMS
            
*/
GLOBAL T_ACI_RETURN qAT_PlusCMMS  ( T_ACI_CMD_SRC srcId,
                                    UBYTE*        mode)
{
  

  TRACE_FUNCTION ("qAT_PlusCMMS ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */
  if(!cmh_IsVldCmdSrc (srcId)) 
  {
    return( AT_FAIL );
  }
  
  *mode   = smsShrdPrm.CMMSmode;

  return ( AT_CMPL );
}





/*==== EOF ========================================================*/
