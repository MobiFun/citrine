/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SMSS
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
|  Purpose :  This module provides the set functions related to the
|             protocol stack adapter for the short message service.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_SMSS_C
#define CMH_SMSS_C
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

#include "gdi.h"
#include "aci.h"
#include "aci_lst.h"
#include "aci_mem.h"

#if defined (MFW) OR defined (SMI) OR defined (_CONC_TESTING_)
#include "conc_sms.h"
#endif

#include "psa.h"
#include "psa_sms.h"
#include "psa_mmi.h"
#include "cmh.h"
#include "cmh_sms.h"
#include "pcm.h"
#include "phb.h"
#include "cmh_phb.h"

#ifdef SIM_TOOLKIT
#include "psa_sim.h"
#include "psa_cc.h"
#include "psa_sat.h"
#include "cmh_sat.h"
#endif /*of SIM_TOOLKIT */


#if 1
#include <stdio.h>
#endif

/*==== CONSTANTS ==================================================*/


#define SMS_CMH_IDX_MAX   (255) /* memory location index        */
                                /* maximum value                */
#define SMS_CMH_IDX_MIN   (1)   /* memory location index        */
                                /* minimum value                */
/*--------------------------------------------------------------*/
#define SMS_CMH_MN_MAX    (255) /* message number maximum value */
#define SMS_CMH_MN_MIN    (0)   /* message number minimum value */
/*--------------------------------------------------------------*/
#define SMS_CMH_CT_MAX    (255) /* command type maximum value   */
#define SMS_CMH_CT_MIN    (0)   /* command type minimum value   */
/*--------------------------------------------------------------*/
#define SMS_CMH_FO_MAX    (255) /* first octet maximum value    */
#define SMS_CMH_FO_MIN    (0)   /* first octet minimum value    */
/*--------------------------------------------------------------*/
#define SMS_CMH_VPREL_MAX (255) /* relative validity period     */
                                /* maximum value                */
#define SMS_CMH_VPREL_MIN (0)   /* relative validity period     */
                                /* minimum value                */
/*--------------------------------------------------------------*/
#define SMS_CMH_DCS_MAX   (255) /* data coding scheme           */
                                /* maximum value                */
#define SMS_CMH_DCS_MIN   (0)   /* data coding scheme           */
                                /* minimum value                */
/*--------------------------------------------------------------*/
#define SMS_CMH_PID_MAX   (255) /* TP-Protocol-Identifier       */
                                /* maximum value                */
#define SMS_CMH_PID_MIN   (0)   /* TP-Protocol-Identifier       */
                                /* minimum value                */

/*==== EXPORT =====================================================*/
/*==== VARIABLES ==================================================*/
EXTERN T_ACI_LIST *set_prm_list;
/*==== FUNCTIONS ==================================================*/
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCSMP                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CSMP
            AT command which is responsible for setting the text
            mode parameter.

            <fo>:    first octet of SMS-DELIVER, SMS-SUBMIT, etc.
            <vprel>: relative validity period
            <vpabs>: absolute validity period
            <vpenh>: enhanced validity period
            <pid>:   TP-Protocol-Identifier
            <dcs>:   SMS data coding scheme
*/
GLOBAL T_ACI_RETURN sAT_PlusCSMP ( T_ACI_CMD_SRC srcId,
                                   SHORT         fo,
                                   SHORT         vprel,
                                   T_ACI_VP_ABS* vpabs,
                                   T_ACI_VP_ENH* vpenh,
                                   SHORT         pid,
                                   SHORT         dcs )
{
  T_SMS_SET_PRM * pSMSSetPrm;    /* points to SMS parameter set     */
  T_SMS_SET_PRM * pSMSSetPrmCopy;/* points to SMS parameter set     */
  UBYTE    chkFo;                /* checked parameter <fo>          */
  UBYTE    chkVprel = '\0';      /* checked parameter <vprel>       */
  T_tp_vp_abs chkVpabs;          /* checked parameter <vpabs>       */
  T_tp_vp_enh chkVpenh;          /* checked parameter <vpenh>       */
  UBYTE    chkPid;               /* checked parameter <pid>         */
  UBYTE    chkDcs;               /* checked parameter <dcs>         */

  TRACE_FUNCTION ("sAT_PlusCSMP ()");

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
   * create a temporary copy
   *-----------------------------------------------------------------
   */
  if ( pSMSSetPrm->isCopy EQ FALSE )
  {
    ACI_MALLOC(pSMSSetPrmCopy, sizeof(T_SMS_SET_PRM));
    memcpy(pSMSSetPrmCopy, pSMSSetPrm, sizeof(T_SMS_SET_PRM));

    pSMSSetPrm->numOfRefs--;
    pSMSSetPrmCopy->isCopy = TRUE;
    smsShrdPrm.pSetPrm[srcId] = pSMSSetPrmCopy;
    pSMSSetPrm = pSMSSetPrmCopy;
  }

  /*
   *-----------------------------------------------------------------
   * process the <fo> parameter
   *-----------------------------------------------------------------
   */
  if( fo NEQ ACI_NumParmNotPresent )
  {
    if( fo > SMS_CMH_FO_MAX OR fo < SMS_CMH_FO_MIN OR
        ( fo & TP_MTI_MASK ) EQ TP_MTI_SMS_RESERVED )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    else

      chkFo = (UBYTE) fo;
  }
  else

    chkFo = pSMSSetPrm -> msgType;

  /*
   *-----------------------------------------------------------------
   * process the <vprel> parameter
   *-----------------------------------------------------------------
   */
  if( ( chkFo & VPF_MASK ) EQ VPF_RELATIVE )
  {
    if ( vprel NEQ ACI_NumParmNotPresent )
    {
      if( vprel > SMS_CMH_VPREL_MAX OR vprel < SMS_CMH_VPREL_MIN )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return ( AT_FAIL );
      }
      else

        chkVprel = (UBYTE) vprel;
    }
    else

      chkVprel = pSMSSetPrm -> vpRel;

  }

  /*
   *-----------------------------------------------------------------
   * process the <vpabs> parameter
   *-----------------------------------------------------------------
   */
  memset(&chkVpabs, 0, sizeof(T_tp_vp_abs));
  if( ( chkFo & VPF_MASK ) EQ VPF_ABSOLUTE )
  {
    if (vpabs NEQ NULL )
    {
      if ( ! cmhSMS_isVpabsVld ( vpabs ) )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return ( AT_FAIL );
      }
      else

        cmhSMS_setVpabsPsa ( &chkVpabs, vpabs );
    }
    else

      memcpy ( ( CHAR * ) &chkVpabs, ( CHAR * ) &pSMSSetPrm -> vpAbs,
               sizeof ( T_tp_vp_abs ) );

  }

  /*
   *-----------------------------------------------------------------
   * process the <vpenh> parameter
   *-----------------------------------------------------------------
   */

  memset(&chkVpenh, 0, sizeof(T_tp_vp_enh));
  if( ( chkFo & VPF_MASK ) EQ VPF_ENHANCED )
  {
    if (vpenh NEQ NULL )
    {
      if ( ! cmhSMS_isVpenhVld ( vpenh ) )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return ( AT_FAIL );
      }
      else

        cmhSMS_setVpenhPsa ( &chkVpenh, vpenh );
    }
    else

      memcpy ( ( CHAR * ) &chkVpenh, ( CHAR * ) &pSMSSetPrm -> vpEnh,
               sizeof ( T_tp_vp_enh ) );

  }

  /*
   *-----------------------------------------------------------------
   * process the <pid> parameter
   *-----------------------------------------------------------------
   */
  if( pid NEQ ACI_NumParmNotPresent )
  {
    if( pid > SMS_CMH_PID_MAX OR pid < SMS_CMH_PID_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    else

      chkPid = (UBYTE) pid;
  }
  else

    chkPid = pSMSSetPrm -> pid;

  /*
   *-----------------------------------------------------------------
   * process the <dcs> parameter
   *-----------------------------------------------------------------
   */
  if( dcs NEQ ACI_NumParmNotPresent )
  {
    if( dcs > SMS_CMH_DCS_MAX OR dcs < SMS_CMH_DCS_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    else

      chkDcs = (UBYTE) dcs;
  }
  else

    chkDcs = pSMSSetPrm -> dcs;

  /*
   *-----------------------------------------------------------------
   * copy parameter
   *-----------------------------------------------------------------
   */
  pSMSSetPrm -> msgType = chkFo;


  if( ( chkFo & VPF_MASK ) EQ VPF_RELATIVE )
  {
    pSMSSetPrm -> vpRel   = chkVprel;
  }
  else if ( ( chkFo & VPF_MASK ) EQ VPF_ABSOLUTE )
  {
    memcpy ( ( CHAR * ) &pSMSSetPrm -> vpAbs, ( CHAR * ) &chkVpabs,
             sizeof ( T_tp_vp_abs ) );
  }
  else if ( ( chkFo & VPF_MASK ) EQ VPF_ENHANCED )
  {
    memcpy ( ( CHAR * ) &pSMSSetPrm -> vpEnh, ( CHAR * ) &chkVpenh,
             sizeof ( T_tp_vp_enh ) );
  }

  pSMSSetPrm -> pid     = chkPid;
  pSMSSetPrm -> dcs     = chkDcs;
  return( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCSCA                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CSCA
            AT command which is responsible for setting the
            service center address.

            <sca>:   service center address
            <tosca>: type of service center address
*/
GLOBAL T_ACI_RETURN sAT_PlusCSCA ( T_ACI_CMD_SRC srcId,
                                   CHAR*         sca,
                                   T_ACI_TOA*    tosca )
{
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set    */
  T_SMS_SET_PRM * pSMSSetPrmCopy; /* copy of SMS parameter set  */
  CHAR* restSca = sca;        /* service center address without */
                              /* international access function  */
                              /* characters                     */

  TRACE_FUNCTION ("sAT_PlusCSCA ()");

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
   * create a temporary copy
   *-----------------------------------------------------------------
   */
  if ( pSMSSetPrm->isCopy EQ FALSE )
  {
    ACI_MALLOC(pSMSSetPrmCopy, sizeof(T_SMS_SET_PRM));
    memcpy(pSMSSetPrmCopy, pSMSSetPrm, sizeof(T_SMS_SET_PRM));

    pSMSSetPrm->numOfRefs--;
    pSMSSetPrmCopy->isCopy = TRUE;
    smsShrdPrm.pSetPrm[srcId] = pSMSSetPrmCopy;
    pSMSSetPrm = pSMSSetPrmCopy;
  }

  /*
   *-----------------------------------------------------------------
   * process the <sca> parameter
   *-----------------------------------------------------------------
   */
  if ( sca NEQ NULL )
  {
    /*
     *---------------------------------------------------------------
     * process the <tosca> parameter
     *---------------------------------------------------------------
     */
    if ( tosca NEQ NULL )
    {
      pSMSSetPrm -> sca.ton = cmhSMS_getNType ( tosca -> ton );
      pSMSSetPrm -> sca.npi = cmhSMS_getNPlan ( tosca -> npi );
    }
    else
    {
      restSca = cmhSMS_setToaDef ( sca,
                                   &pSMSSetPrm -> sca.ton,
                                   &pSMSSetPrm -> sca.npi );
    }

    cmhSMS_getAdrBcd ( pSMSSetPrm -> sca.num,
                       &pSMSSetPrm -> sca.c_num,
                       MAX_SMS_ADDR_DIG, restSca);

    pSMSSetPrm -> sca.v_npi = TRUE;
    pSMSSetPrm -> sca.v_ton = TRUE;
    return ( AT_CMPL );
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return ( AT_FAIL );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCSMS                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CSMS
            AT command which is responsible for setting the
            message service.

            <service>: message service
*/
GLOBAL T_ACI_RETURN sAT_PlusCSMS ( T_ACI_CMD_SRC    srcId,
                                   T_ACI_CSMS_SERV  service )
{
  T_ACI_RETURN  ret;          /* AT response code                 */

  TRACE_FUNCTION ("sAT_PlusCSMS ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  /*
   *-----------------------------------------------------------------
   * process the <service> parameter
   *-----------------------------------------------------------------
   */
  switch( service )
  {
    case( CSMS_SERV_NotPresent ):
      break;

    case( CSMS_SERV_GsmPh2 ):
    case( CSMS_SERV_GsmPh2Plus ):
      smsShrdPrm.CSMSservice = service;

      smsShrdPrm.smsEntStat.curCmd = AT_CMD_CSMS;
      smsShrdPrm.owner = (T_OWN)srcId;
      smsShrdPrm.smsEntStat.entOwn = srcId;
    /* Implements measure 147,148, 149 */
      cmhSMS_sendConfigureReq(FALSE);
      {
        R_AT ( RAT_CSMS, smsShrdPrm.smsEntStat.entOwn )
          ( CSMS_SERV_NotPresent,
            smsShrdPrm.CSMSmt,
            smsShrdPrm.CSMSmo,
            smsShrdPrm.CSMSbm );
      }
      smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
      smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
      smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  return( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGF                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGF
            AT command which is responsible for setting the
            message format.

            <mode>: indicates the message format
*/
GLOBAL T_ACI_RETURN sAT_PlusCMGF ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_CMGF_MOD mode )
{
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set */

  TRACE_FUNCTION ("sAT_PlusCMGF ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  /*
   *-----------------------------------------------------------------
   * process the <mode> parameter
   *-----------------------------------------------------------------
   */
  switch( mode )
  {
    case( CMGF_MOD_NotPresent ):
      pSMSSetPrm -> CMGFmode = CMGF_MOD_Pdu;
      break;

    case( CMGF_MOD_Txt ):
    case (CMGF_MOD_Pdu ):
      pSMSSetPrm -> CMGFmode = mode;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  return( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code                   ROUTINE : sAT_PercentSMBS        |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the %SMBS
            AT command which is responsible for setting the SMS message format.

            <mode>: indicates the message format
*/
#ifdef FF_MMI_RIV
GLOBAL T_ACI_RETURN sAT_PercentSMBS ( T_ACI_CMD_SRC       srcId,
                                      T_ACI_PERC_SMBS_MOD mode )
{
  TRACE_FUNCTION ("sAT_PercentSMBS ()");

   /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* process the <mode> parameter */
  switch( mode )
  {
    case( PERC_SMBS_MOD_NotPresent ):
      smsShrdPrm.perccmgf_smbs_mode = PERC_SMBS_MOD_DISABLE; /* default */
      break;

    case( PERC_SMBS_MOD_DISABLE ):
    case (PERC_SMBS_MOD_ENABLE ):
      smsShrdPrm.perccmgf_smbs_mode = mode;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  return( AT_CMPL );
}
#endif /* #ifdef FF_MMI_RIV */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCNMA                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CNMA
            AT command which is responsible for sending the
            new message acknowledgement.

*/
GLOBAL T_ACI_RETURN sAT_PlusCNMA ( T_ACI_CMD_SRC srcId)
{
  T_ACI_RETURN  ret;         /* AT response code */

  TRACE_FUNCTION ("sAT_PlusCNMA ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  /*
   *-----------------------------------------------------------------
   * check current service and whether an acknowledgement is expected.
   *-----------------------------------------------------------------
   */

  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus AND
      smsShrdPrm.cnma_ack_expected)
  {
    PALLOC (mnsms_ack_res, MNSMS_ACK_RES);

    smsShrdPrm.cnma_ack_expected = FALSE;

    mnsms_ack_res->resp    = SMS_RP_ACK;
    mnsms_ack_res->sms_sdu.o_buf = 0;
    mnsms_ack_res->sms_sdu.l_buf = 0;

    PSENDX (SMS, mnsms_ack_res);
    return ( AT_CMPL );
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_NoCnmaAckExpect );
    return ( AT_FAIL );
  }
}

#if defined (SMS_PDU_SUPPORT)
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCNMAPdu              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CNMA
            AT command which is responsible for sending the
            new message acknowledgement in PDU mode

*/
GLOBAL T_ACI_RETURN sAT_PlusCNMAPdu ( T_ACI_CMD_SRC srcId,
                                      SHORT         n,
                                      T_ACI_SM_DATA *pdu)
{
  T_ACI_RETURN  ret;         /* AT response code */

  TRACE_FUNCTION ("sAT_PlusCNMAPdu ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  /*
   *-----------------------------------------------------------------
   * check current service and whether an acknowledgement is expected.
   *-----------------------------------------------------------------
   */
  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus AND
      smsShrdPrm.cnma_ack_expected)
  {
    PALLOC (mnsms_ack_res, MNSMS_ACK_RES);

    smsShrdPrm.cnma_ack_expected = FALSE;

    if (n EQ 2)
      mnsms_ack_res->resp = SMS_RP_ERROR;
    else
      mnsms_ack_res->resp = SMS_RP_ACK;

    /* one byte for SCA length */
    if( pdu->len NEQ 0 )
    {
      mnsms_ack_res->sms_sdu.l_buf = (pdu->len+1) * 8;
    }
    else
    {
      mnsms_ack_res->sms_sdu.l_buf = 0;
    }
    mnsms_ack_res->sms_sdu.o_buf = 0;

    /* length of SCA length (is always empty!) */
    mnsms_ack_res->sms_sdu.buf[0] = 0x00;
    memcpy (&mnsms_ack_res->sms_sdu.buf[1], pdu->data, pdu->len);

    PSENDX (SMS, mnsms_ack_res);
    return ( AT_CMPL );
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_NoCnmaAckExpect );
    return ( AT_FAIL );
  }
}
#endif	/* SMS_PDU_SUPPORT */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCNMI                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CNMI
            AT command which is responsible for setting the
            new message indication parameter.

            <mt>: indication routing for SMS-DELIVER
            <bm>: indication routing for CBM
            <ds>: indication routing for SMS-STATUS-REPORT
*/
GLOBAL T_ACI_RETURN sAT_PlusCNMI ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CNMI_MT mt,
                                   T_ACI_CNMI_BM bm,
                                   T_ACI_CNMI_DS ds )
{
  T_ACI_RETURN  ret;          /* AT response code                 */
  UBYTE         chkMtHndl;    /* checked parameter <mt>, prm set  */
  UBYTE         chkBmHndl;    /* checked parameter <bm>, prm set  */
  UBYTE         chkDsHndl;    /* checked parameter <ds>, prm set  */

  T_ACI_CNMI_MT chkMt;        /* checked parameter <mt>, cmd set  */
  T_ACI_CNMI_BM chkBm;        /* checked parameter <bm>, cmd set  */
  T_ACI_CNMI_DS chkDs;        /* checked parameter <ds>, cmd set  */

  TRACE_FUNCTION ("sAT_PlusCNMI ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  /*
   *-----------------------------------------------------------------
   * process the <mt> parameter, prm set
   *-----------------------------------------------------------------
   */
  switch( mt )
  {
    case(CNMI_MT_NotPresent     ): chkMtHndl = smsShrdPrm.mtHndl; break;
    case(CNMI_MT_NoSmsDeliverInd): chkMtHndl = MT0;               break;
    case(CNMI_MT_SmsDeliverInd  ): chkMtHndl = MT1;               break;
    case(CNMI_MT_SmsDeliver     ): chkMtHndl = MT2;               break;
    case(CNMI_MT_SmsDeliverCls3 ): chkMtHndl = MT3;               break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * process the <mt> parameter, cmd set
   *-----------------------------------------------------------------
   */
  if ( mt EQ CNMI_MT_NotPresent )
    chkMt = smsShrdPrm.CNMImt;
  else
    chkMt = mt;

  /*
   *-----------------------------------------------------------------
   * process the <bm> parameter, prm set
   *-----------------------------------------------------------------
   */
  switch( bm )
  {
    case( CNMI_BM_NotPresent ): chkBmHndl = smsShrdPrm.cbmPrm.cbmHndl;
                                break;
    case( CNMI_BM_NoCbmInd   ): chkBmHndl = BM0;                break;
    case( CNMI_BM_Cbm        ): chkBmHndl = BM2;                break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * process the <bm> parameter, cmd set
   *-----------------------------------------------------------------
   */
  if ( bm EQ CNMI_BM_NotPresent )
    chkBm = smsShrdPrm.CNMIbm;
  else
    chkBm = bm;

  /*
   *-----------------------------------------------------------------
   * process the <ds> parameter, prm set
   *-----------------------------------------------------------------
   */
  switch( ds )
  {
    case( CNMI_DS_NotPresent   ): chkDsHndl = smsShrdPrm.srHndl; break;
    case( CNMI_DS_NoSmsStatRpt ): chkDsHndl = DS0;               break;
    case( CNMI_DS_SmsStatRpt   ): chkDsHndl = DS1;               break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * process the <ds> parameter, cmd set
   *-----------------------------------------------------------------
   */
  if ( ds EQ CNMI_DS_NotPresent )
    chkDs = smsShrdPrm.CNMIds;
  else
    chkDs = ds;

  /*
   *-----------------------------------------------------------------
   * copy parameter
   *-----------------------------------------------------------------
   */
  smsShrdPrm.mtHndl         = chkMtHndl;
  smsShrdPrm.cbmPrm.cbmHndl = chkBmHndl;
  smsShrdPrm.srHndl         = chkDsHndl;

  smsShrdPrm.CNMImt = chkMt;
  smsShrdPrm.CNMIbm = chkBm;
  smsShrdPrm.CNMIds = chkDs;

  /*
   *-----------------------------------------------------------------
   * configure SMS
   *-----------------------------------------------------------------
   */
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_CNMI;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;
    /* Implements measure 147,148, 149 */
  cmhSMS_sendConfigureReq (FALSE);
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
  smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
  smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

  /*
   *-----------------------------------------------------------------
   * set the new CBCH configuration
   *-----------------------------------------------------------------
   */
  smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)srcId;

  if( psaMMI_Cbch() < 0 )
  {
    TRACE_EVENT( "FATAL RETURN psaSMS in +CNMI" );
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    return( AT_FAIL );
  }

  smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)CMD_SRC_NONE;

  return ( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCPMS                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CPMS
            AT command which is responsible for setting the
            preferred message storages.

            <mem1>: preferred memory 1
            <mem2>: preferred memory 2
            <mem3>: preferred memory 3
*/
GLOBAL T_ACI_RETURN sAT_PlusCPMS ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_SMS_STOR mem1,
                                   T_ACI_SMS_STOR mem2,
                                   T_ACI_SMS_STOR mem3 )
{
  T_ACI_RETURN  ret;          /* AT response code            */

  UBYTE chkMem1;              /* checked parameter <mem1>    */
  UBYTE chkMem2;              /* checked parameter <mem2>    */
  UBYTE chkMem3;              /* checked parameter <mem3>    */

  T_ACI_SMS_STOR_OCC m1;
  T_ACI_SMS_STOR_OCC m2;
  T_ACI_SMS_STOR_OCC m3;

  UBYTE isModified = FALSE;

  TRACE_FUNCTION ("sAT_PlusCPMS ()");


  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  /*
   *-----------------------------------------------------------------
   * process the <mem1> parameter
   *-----------------------------------------------------------------
   */
  if( mem1 NEQ SMS_STOR_NotPresent )
  {
    if( ! cmhSMS_getMemPsa ( mem1, &chkMem1 ) )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    smsShrdPrm.mem1 = chkMem1;
  }

  /*
   *-----------------------------------------------------------------
   * process the <mem2> parameter
   *-----------------------------------------------------------------
   */
  if( mem2 NEQ SMS_STOR_NotPresent )
  {
    if( ! cmhSMS_getMemPsa ( mem2, &chkMem2 ) )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
     smsShrdPrm.mem2 = chkMem2;
  }

  /*
   *-----------------------------------------------------------------
   * process the <mem3> parameter
   *-----------------------------------------------------------------
   */
  if( mem3 NEQ SMS_STOR_NotPresent )
  {
    if( ! cmhSMS_getMemPsa ( mem3, &chkMem3 ) )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    if (smsShrdPrm.mem3 NEQ chkMem3)
    {
      smsShrdPrm.mem3 = chkMem3;
      isModified = TRUE;
    }
  }


  /*-------------------------------------------------------------------
   * create and send primitive for configure request
   *-------------------------------------------------------------------
   */
  if (isModified)
    /* Implements measure 147,148, 149 */
    cmhSMS_sendConfigureReq (FALSE);


  /*
   * -----------------------------------------------------------------
   * Fills the T_ACI_SMS_STOR_OCC structure with data from the
   * shared parameter buffer (used mem, total mem).
   * -----------------------------------------------------------------
   */
  cmhSMS_setStorOcc ( &m1, smsShrdPrm.mem1 );
  cmhSMS_setStorOcc ( &m2, smsShrdPrm.mem2 );
  cmhSMS_setStorOcc ( &m3, smsShrdPrm.mem3 );

#ifdef FF_ATI
  cpmsCallType = SAT_CALL;
#endif /* FF_ATI */

  R_AT ( RAT_CPMS, srcId ) ( &m1, &m2, &m3 );

  return( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGS_Old             |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMSS
            AT command which is responsible for sending a short
            message from memory.

            This function exists for compatibility reasons.

*/
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
GLOBAL T_ACI_RETURN sAT_PlusCMGS_Old ( T_ACI_CMD_SRC  srcId,
                                       CHAR*          da,
                                       T_ACI_TOA*     toda,
                                       T_ACI_SM_DATA* data,
                                       CHAR*          sca,
                                       T_ACI_TOA*     tosca,
                                       SHORT          isReply )
{
  TRACE_FUNCTION ("sAT_PlusCMGS_Old ()");

  return sAT_PlusCMGS_Gl
         (srcId, da, toda, data, NULL, sca, tosca, isReply, rAT_PlusCMGS,
         rAT_PlusCMS);
}
#endif /*#if defined SMI OR defined MFW OR defined FF_MMI_RIV*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGS                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGS
            AT command which is responsible for sending a short
            message.

            Features Concatenated SMS.
            For GPF-MMI.

            <da>:      destination address
            <toda>:    type of destination address
            <data>:    message data
            <sca>:     service center address
            <tosca>:   type of service center address
            <isReply>:  > 0: set TP-Reply-Path explicitly
                       EQ 0: clear TP-Reply-Path explicitly
*/
#if (defined (MFW) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS
GLOBAL T_ACI_RETURN sAT_PlusCMGS ( T_ACI_CMD_SRC  srcId,
                                   CHAR*          da,
                                   T_ACI_TOA*     toda,
                                   T_SM_DATA_EXT* src_data,
                                   CHAR*          sca,
                                   T_ACI_TOA*     tosca,
                                   SHORT          isReply )
{
  T_CONC_INIT_RETURN ret;
  T_ACI_UDH_DATA udh;

  T_ACI_SM_DATA tar_data;
  UBYTE alphabet;

  TRACE_FUNCTION ("sAT_PlusCMGS ()");


  alphabet = cmhSMS_getAlphabetPp ( smsShrdPrm.pSetPrm[srcId]->dcs );

  /* cut off more than CONC_MAX_SEGS segments */
  if (alphabet EQ 0)
  {
    src_data->len = MINIMUM (src_data->len, concShrdPrm.l_uncomp7bit_data_conc*CONC_MAX_SEGS);
  }
  else
  {
    src_data->len = MINIMUM (src_data->len, concShrdPrm.l_uncomp8bit_data_conc*CONC_MAX_SEGS);
  }

  ret=concSMS_initSend(&tar_data, &udh, srcId, da, toda, src_data, sca,
                       tosca, isReply, alphabet);

  if (ret EQ CONC_NEEDED)
  {
    SET_CONC;
    return sAT_PlusCMGS_Gl(srcId, da, toda, &tar_data, &udh, sca, tosca,
                           isReply, rConcSMS_PlusCMGS, rConcSMS_PlusCMS_CMGS);
  }
  else
  {
    return sAT_PlusCMGS_Gl(srcId, da, toda, &tar_data, NULL, sca, tosca,
                           isReply, rAT_PlusCMGS, rAT_PlusCMS);
  }
}
#endif /* #if (defined (MFW) OR defined (_CONC_TESTING_) */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGS                 |
+-------------------------------------------------------------------+

  PURPOSE : For Riv-MMI if conc. is not needed.
*/
#if defined (FF_MMI_RIV) /* OR !defined TI_PS_FF_CONC_SMS */
GLOBAL T_ACI_RETURN sAT_PlusCMGS ( T_ACI_CMD_SRC  srcId,
                                   CHAR*          da,
                                   T_ACI_TOA*     toda,
                                   T_ACI_SM_DATA* src_data,
                                   CHAR*          sca,
                                   T_ACI_TOA*     tosca,
                                   SHORT          isReply )
{
  TRACE_FUNCTION ("sAT_PlusCMGS ()");

  return sAT_PlusCMGS_Gl(srcId, da, toda, src_data, NULL, sca, tosca,
                           isReply, rAT_PlusCMGS, rAT_PlusCMS);  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PercentCMGS              |
+-------------------------------------------------------------------+

  PURPOSE : Handling Percent CMGS command from Riv-MMI side which 
            contains the UDH also for each segment.
*/
GLOBAL T_ACI_RETURN sAT_PercentCMGS ( T_ACI_CMD_SRC  srcId,
                                   CHAR*          da,
                                   T_ACI_TOA*     toda,
                                   T_ACI_SM_DATA* src_data,
                                   T_ACI_UDH_DATA* udh_data,
                                   CHAR*          sca,
                                   T_ACI_TOA*     tosca,
                                   SHORT          isReply )
{
  TRACE_FUNCTION ("sAT_PercentCMGS () with udh");

    return sAT_PlusCMGS_Gl(srcId, da, toda, src_data, udh_data, sca, tosca,
                           isReply, rAT_PlusCMGS, rAT_PlusCMS);  
}
#endif /*#if defined FF_MMI_RIV*/

/*
 * The following function has been conditioned out for the FreeCalypso build
 * as it was failing compilation; it does not appear in the TCS211 version;
 * to be investigated later.
 */

#if 0
GLOBAL T_ACI_RETURN sAT_PlusCMGS_byPort( T_ACI_CMD_SRC srcId,
                  CHAR*     da,
                  T_ACI_TOA*   toda,
                  T_SM_DATA_EXT* src_data,
                  CHAR*     sca,
                  T_ACI_TOA*   tosca,
                  SHORT     isReply,
                  SHORT     isSpPORT,
                  SHORT     destPORT,
                  SHORT     origPORT)
{
 T_CONC_INIT_RETURN ret;
 T_ACI_UDH_DATA udh;

 

 T_ACI_SM_DATA tar_data;
 UBYTE alphabet;

 

 TRACE_FUNCTION ("sAT_PlusCMGS_byPort ()");

 

 memset(&udh, 0, sizeof(T_ACI_UDH_DATA));
 alphabet = cmhSMS_getAlphabetPp ( smsShrdPrm.pSetPrm[srcId]->dcs );

 

 ret=concSMS_initSend(&tar_data, &udh, srcId, da, toda, src_data, sca,
            tosca, isReply, alphabet);
 if(isSpPORT)//add port data to udh
 {
  udh.data[udh.len]  = 0x05;//Application Port Addressing 16 bit address
  udh.data[udh.len+1] = 0x04;//ud length
  udh.data[udh.len+2] = destPORT>>8;//destination port
  udh.data[udh.len+3] = destPORT&0xff;
  udh.data[udh.len+4] = origPORT>>8;//orignal port
  udh.data[udh.len+5] = origPORT&0xff;
  udh.len+=6;
 }
 if (ret EQ CONC_NEEDED)
 {
  SET_CONC;
  return sAT_PlusCMGS_Gl(srcId, da, toda, &tar_data, &udh, sca, tosca,
              isReply, rConcSMS_PlusCMGS, rConcSMS_PlusCMS_CMGS);
 }
 else if(isSpPORT)
 {
  return sAT_PlusCMGS_Gl(srcId, da, toda, &tar_data, &udh, sca, tosca,
              isReply, rAT_PlusCMGS, rAT_PlusCMS);
 }
 else
 {
  return sAT_PlusCMGS_Gl(srcId, da, toda, &tar_data, NULL, sca, tosca,
              isReply, rAT_PlusCMGS, rAT_PlusCMS);
 }
}
#endif

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGS_Gl              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGS
            AT command which is responsible for sending a short
            message.

            <da>:      destination address
            <toda>:    type of destination address
            <data>:    message data
            <udh>:     user data header
            <sca>:     service center address
            <tosca>:   type of service center address
            <isReply>:  > 0: set TP-Reply-Path explicitly
                       EQ 0: clear TP-Reply-Path explicitly
            <rplyCB>:  reply call-back
            <errorCB>: error call-back
*/
GLOBAL T_ACI_RETURN sAT_PlusCMGS_Gl ( T_ACI_CMD_SRC   srcId,
                                      CHAR*           da,
                                      T_ACI_TOA*      toda,
                                      T_ACI_SM_DATA*  data,
                                      T_ACI_UDH_DATA* udh,
                                      CHAR*           sca,
                                      T_ACI_TOA*      tosca,
                                      SHORT           isReply,
                                      T_CMSS_FCT      rplyCB,
                                      T_ERROR_FCT     errorCB)
{
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set       */
  T_ACI_RETURN  ret=AT_CMPL;  /* AT response code                  */
  CHAR*       restDa = da;    /* destination address without       */
                              /* international access function     */
                              /* characters                        */
  CHAR* restSca = sca;        /* service center address without    */
                              /* international access function     */
                              /* characters                        */

  T_ACI_SM_DATA packedData;
  T_tp_da da_addr;
  UBYTE msgType;
  UBYTE byte_offset = 0;
  BOOL mtchFlag;

#ifdef _CONC_TESTING_
/* Implements Measure#32 */
#endif

  TRACE_FUNCTION ("sAT_PlusCMGS_Gl ()");


  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  smsShrdPrm.rplyCB.cmgs  = rplyCB;
  smsShrdPrm.errorCB      = errorCB;

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  /*
   *-----------------------------------------------------------------
   * process the <da> parameter
   *-----------------------------------------------------------------
   */
  if ( da NEQ NULL )
  {
    /*
     *---------------------------------------------------------------
     * process the <toda> parameter
     *---------------------------------------------------------------
     */
    if ( toda NEQ NULL )
    {
       da_addr.ton = toda -> ton;
       da_addr.npi = toda -> npi;
    }
    else
    {
      restDa = cmhSMS_setToaDef ( da, &da_addr.ton, &da_addr.npi );
    }

    /* start: check if fixed dialing phonebook is enabled */
    if (pb_get_fdn_mode () EQ FDN_ENABLE AND             
         pb_get_fdn_classtype() EQ CLASS_VceDatFaxSms  )
    {
      TRACE_FUNCTION("sAT_PlusCMGS_Gl: FDN");
      if ( da_addr.ton NEQ TON_Alphanumeric ) /* not supported by Phonebook  */
      {
        TRACE_EVENT_P1("sAT_PlusCMGS_Gl: FDN checking: %s",da);

        if ( da[0] EQ '+' ) 
        {
          /* check  if '+' is within the string and remove it if it is */
          mtchFlag = (pb_check_fdn (0, (const UBYTE *)(da + 1)) EQ PHB_OK);
        }
        else
        {
          mtchFlag = (pb_check_fdn (0, (const UBYTE *)(da    )) EQ PHB_OK);
        }      
        if ( mtchFlag )
        {
          TRACE_EVENT("sAT_PlusCMGS_Gl: Found match in FDN!");
        }
        else 
        {
          TRACE_EVENT("sAT_PlusCMGS_Gl: No match in FDN found, SMS rejected!");
          ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_OpNotAllowed );
          return ( AT_FAIL );
        }
      }
      else /* TON signals ALPHANUMERIC */
      {
        TRACE_EVENT("FDN doesn't support alphanumeric addr., SMS rejected");
        ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_OpNotAllowed );
        return ( AT_FAIL );      
      }
    } 
    /* end: check fixed dialing phonebook if enabled */

    if ( da_addr.ton EQ TON_Alphanumeric )
    {
      if( cmhSMS_packAlphaNumAddr(restDa, &da_addr) EQ AT_FAIL )
      {
        return (AT_FAIL);
      }
    }
    else
    {
      cmhSMS_getAdrBcd ( da_addr.num, &da_addr.c_num, MAX_SMS_ADDR_DIG, restDa);
      da_addr.digits = da_addr.c_num;
    }
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return ( AT_FAIL );
  }

  /*
   *-------------------------------------------------------------------
   * create and send primitive for SMS submit
   *-------------------------------------------------------------------
   */
  {
    PALLOC (mnsms_submit_req, MNSMS_SUBMIT_REQ);

    /*
     *-----------------------------------------------------------------
     * process the <sca> parameter
     *-----------------------------------------------------------------
     */
    if ( sca NEQ NULL AND *sca NEQ '\0' )
    {
      /*
       *---------------------------------------------------------------
       * process the <tosca> parameter
       *---------------------------------------------------------------
       */
      if ( tosca NEQ NULL )
      {
        smsShrdPrm.tpdu.sc_addr.ton = tosca -> ton;
        smsShrdPrm.tpdu.sc_addr.npi = tosca -> npi;

      }
      else
      {
        restSca = cmhSMS_setToaDef( sca, &smsShrdPrm.tpdu.sc_addr.ton,
                                         &smsShrdPrm.tpdu.sc_addr.npi);
      }
      cmhSMS_getAdrBcd( smsShrdPrm.tpdu.sc_addr.num,
                        &smsShrdPrm.tpdu.sc_addr.c_num,
                        MAX_SMS_ADDR_DIG, restSca);

      smsShrdPrm.tpdu.sc_addr.v_ton = TRUE;
      smsShrdPrm.tpdu.sc_addr.v_npi = TRUE;

    }
    else
    {
      memcpy (&smsShrdPrm.tpdu.sc_addr, &pSMSSetPrm->sca, sizeof(T_rp_addr));
    }

    /*
     *-----------------------------------------------------------------
     * process the <isReply> parameter
     *-----------------------------------------------------------------
     */
    msgType  = (pSMSSetPrm->msgType & ~TP_MTI_MASK)
                                    | TP_MTI_SMS_SUBMIT;
    if (isReply EQ 0)
      msgType &= ~TP_RP_MASK;  /* set TP-Reply-Path bit to 0 */
    else if (isReply > 0)
      msgType |= TP_RP_MASK;   /* set TP-Reply-Path bit to 1 */

    /*
     *-----------------------------------------------------------------
     * process the <data> parameter and
     * copy already to shared parameter
     *-----------------------------------------------------------------
     * reducing from 8 to 7 bit (if DCS is set)
     *-----------------------------------------------------------------
     */

    if (udh)
      byte_offset = udh->len+1;

    if (data EQ NULL)
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      PFREE (mnsms_submit_req);

      return ( AT_FAIL );
    }

    cmhSMS_rdcSmsPp ( byte_offset,
                      pSMSSetPrm -> dcs,
                      ( UBYTE * ) data->data, ( UBYTE ) data->len,
                      packedData.data,
                      ( UBYTE * ) &packedData.len);

    ACI_MALLOC(smsShrdPrm.tpdu.tp_submit, sizeof(T_TP_SUBMIT));
    cmhSMS_fillTpSubmit (smsShrdPrm.tpdu.tp_submit,
                         srcId,
                         msgType,
                         0,
                         &da_addr,
                         &packedData,
                         data->len,
                         udh );

    mnsms_submit_req -> mem_type = smsShrdPrm.mem2;
    mnsms_submit_req -> rec_num  = SMS_RECORD_NOT_EXIST;
    mnsms_submit_req -> condx    = SMS_CONDX_OVR_NON;
    mnsms_submit_req -> modify   = SMS_MODIFY_NON;
#ifdef REL99
    if(srcId NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT)
    {
      mnsms_submit_req -> auto_rep_flag = smsShrdPrm.auto_repeat_flag;
    }
    else
    {
      mnsms_submit_req -> auto_rep_flag = FALSE;
    }
#endif

  /*
   *-----------------------------------------------------------------
   * check if message should be transfered directly to SIM for SAT
   *-----------------------------------------------------------------
   */

#ifdef SIM_TOOLKIT

  if (psaSIM_ChkSIMSrvSup( SRV_MOSMCtrlSIM ))
  {
        if (simShrdPrm.setPrm[srcId].sat_cc_mode EQ SATCC_CONTROL_BY_SIM_ACTIVE)
        {
            ret = cmhSAT_MoSmCntr( smsShrdPrm.tpdu.sc_addr,
                                   da_addr,
                                   (UBYTE)srcId);
        }
        else
        {
            simShrdPrm.setPrm[srcId].sat_cc_mode = SATCC_CONTROL_BY_SIM_ACTIVE;
        }
  }

  /* SIM_TOOLKIT_REQ was sent */
  if (ret NEQ AT_CMPL)
  {
    /* save primitive address for SIM_TOOLKIT response */
    sat_mnsms_submit_req = mnsms_submit_req;

    smsShrdPrm.smsEntStat.curCmd = AT_CMD_CMGS;
    smsShrdPrm.owner = (T_OWN)srcId;
    smsShrdPrm.smsEntStat.entOwn = srcId;

    /* mnsms_submit_req is send and coded in cmhSAT_ResSMCntrlBySIM() */

    return ret;
  }
#endif /* SIM_TOOLKIT */

    /* code SMS-SUBMIT here */
    cmhSMS_codeMsg (&mnsms_submit_req->sms_sdu, SMS_VT_SUBMIT,
                    &smsShrdPrm.tpdu.sc_addr, SMS_SUBMIT,
                    (UBYTE*)smsShrdPrm.tpdu.tp_submit);

    PSENDX (SMS, mnsms_submit_req);

#ifdef _CONC_TESTING_
/* Implements Measure#32 */
    cmhSMS_sdu_buf_print(&(mnsms_submit_req->sms_sdu.buf[0]), 0);
#endif

    if (smsShrdPrm.tpdu.tp_submit NEQ NULL)
    {
      ACI_MFREE(smsShrdPrm.tpdu.tp_submit);
      smsShrdPrm.tpdu.tp_submit = NULL;
    }
  }

  smsShrdPrm.smsEntStat.curCmd = AT_CMD_CMGS;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;

  return( AT_EXCT );
}

#if defined (SMS_PDU_SUPPORT) || defined (SIM_TOOLKIT)

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGSPdu              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGS
            AT command which is responsible for sending a short
            message in pdu mode.

*/
GLOBAL T_ACI_RETURN sAT_PlusCMGSPdu ( T_ACI_CMD_SRC  srcId,
                                      T_ACI_SM_DATA  *pdu )
{
  T_ACI_RETURN  ret=AT_CMPL;      /* AT response code */
  T_rp_addr sc_addr;
  T_tp_da   dest_addr;
  UBYTE len;

  CHAR  daString[MAX_DIAL_LEN];

  TRACE_FUNCTION ("sAT_PlusCMGSPdu()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;


#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  smsShrdPrm.rplyCB.cmgs  = rAT_PlusCMGS;
  smsShrdPrm.errorCB      = rAT_PlusCMS;
#endif /* defined SMI OR defined MFW OR defined FF_MMI_RIV */

  /*
   *-------------------------------------------------------------------
   * create and send primitive for SMS submit
   *-------------------------------------------------------------------
   */
  {
    PALLOC (mnsms_submit_req, MNSMS_SUBMIT_REQ);

    mnsms_submit_req -> mem_type = smsShrdPrm.mem2;
    mnsms_submit_req -> rec_num  = SMS_RECORD_NOT_EXIST;
    mnsms_submit_req -> condx    = SMS_CONDX_OVR_NON;
    mnsms_submit_req -> modify   = SMS_MODIFY_NON;

#ifdef REL99
    if(srcId NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT)
    {
      mnsms_submit_req -> auto_rep_flag = smsShrdPrm.auto_repeat_flag;
    }
    else
    {
      mnsms_submit_req -> auto_rep_flag = FALSE;
    }
#endif /* REL99 */

    if ( pdu->len > 0 )
    {
      mnsms_submit_req->sms_sdu.l_buf = pdu->len * 8;
      mnsms_submit_req->sms_sdu.o_buf = 0;
      memcpy (mnsms_submit_req->sms_sdu.buf, pdu->data, pdu->len);
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      PFREE (mnsms_submit_req);
      return ( AT_FAIL );
    }


  len = DecodeRPAddress( &sc_addr.c_num, &sc_addr.ton,
                          &sc_addr.npi, (UBYTE*)&sc_addr.num, pdu->data );

  DecodeTPAddress( &dest_addr.c_num, &dest_addr.ton,
                    &dest_addr.npi, (UBYTE*)&dest_addr.num, pdu->data+len+2);

  /* start: check number if fdn is activated and search number */      
/*  if( ( simShrdPrm.crdFun EQ SIM_FDN_ENABLED OR
        simShrdPrm.crdFun EQ SIM_FDN_BDN_ENABLED ) AND 
        pb_get_fdn_classtype() EQ CLASS_VceDatFaxSms  )*/
//TISH, patch for OMAPS00129150
//start
#if 0
  if (pb_get_fdn_mode () EQ FDN_ENABLE AND             
      pb_get_fdn_classtype() EQ CLASS_VceDatFaxSms  )
#else
  if (pb_get_fdn_mode () EQ FDN_ENABLE AND             
      pb_get_fdn_classtype() EQ CLASS_VceDatFaxSms  AND srcId NEQ OWN_SRC_SAT)
#endif
//end
  {
     cmhPHB_getAdrStr(daString, MAX_DIAL_LEN-1, 
                     pdu->data+len+4, dest_addr.c_num);
    
     TRACE_EVENT_P1("sAT_PlusCMGSPdu: FDN checking %s:", daString);
    
     if ( dest_addr.ton NEQ TON_Alphanumeric ) /* not supported by Phonebook  */
    {

      if (pb_check_fdn (0, (const UBYTE*)daString) EQ PHB_OK)
      {
        TRACE_EVENT("sAT_PlusCMGSPdu: Found match in FDN");
      }
      else 
      {
        TRACE_EVENT("sAT_PlusCMGSPdu: No match in FDN found, SMS rejected");
        ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_OpNotAllowed );
        return ( AT_FAIL );
      }
    }
    else /* da alphanumeric encoded */
    {
      TRACE_EVENT("FDN doesn't support alphanumeric addr., SMS rejected");
      ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_OpNotAllowed );
      return ( AT_FAIL );      
    }
  }
  /* end: check number if fdn is activated and search number */   

  /*
   *-----------------------------------------------------------------
   * check if message should be transfered directly to SIM for SAT
   *-----------------------------------------------------------------
   */

#ifdef SIM_TOOLKIT


  if (psaSIM_ChkSIMSrvSup( SRV_MOSMCtrlSIM ))
  {
        if (simShrdPrm.setPrm[srcId].sat_cc_mode EQ SATCC_CONTROL_BY_SIM_ACTIVE)
        {
            ret = cmhSAT_MoSmCntr( sc_addr,
                                   dest_addr,
                                   (UBYTE)srcId );
        }
        else
        {
            simShrdPrm.setPrm[srcId].sat_cc_mode = SATCC_CONTROL_BY_SIM_ACTIVE;
        }
  }

  if (ret NEQ AT_CMPL)
  {
    /* save primitive address for SIM_TOOLKIT response */
    sat_mnsms_submit_req = mnsms_submit_req;

    smsShrdPrm.smsEntStat.curCmd  = AT_CMD_CMGS;
    smsShrdPrm.owner = (T_OWN)srcId;
    smsShrdPrm.smsEntStat.entOwn  = srcId;

    return ret;
   /* PFREE(mnsms_submit_req);  
    * NO PRIMITVE FREE !!! 
    */
  }
#endif /* SIM_TOOLKIT */

    
#ifdef TI_PS_FF_AT_P_CMD_CUST
    if ((srcId EQ ((T_ACI_CMD_SRC)OWN_SRC_SAT)) AND
        (simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1))
    {
           /*
           ** MMI is a Cust1 Application
           ** Forward the original PDU to the MMI, and free the Primitive
           ** (it will be recreated when the MMI sends its Request)
           */
           cmhSAT_Cust1StkCmdInd();
           PFREE(mnsms_submit_req);

           /*
           ** Ensure that the SMS parameters are reset, so that the SMS Entity is freed to
           ** process the command later.
           */
           smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
           smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
           smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

           return( AT_EXCT );
    }
#endif /* TI_PS_FF_AT_P_CMD_CUST */
    PSENDX (SMS, mnsms_submit_req);
  }

  smsShrdPrm.smsEntStat.curCmd  = AT_CMD_CMGS;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn  = srcId;

  return( AT_EXCT );
}

#endif


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMSS                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMSS
            AT command which is responsible for sending a short
            message from memory.

            Features Concatenated SMS.
            For GPF-MMI.

            <index>: storage area index
            <da>:    destination address
            <toda>:  type of destination address
*/
#if (defined (MFW) OR defined (SMI) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS
GLOBAL T_ACI_RETURN sAT_PlusCMSS ( T_ACI_CMD_SRC srcId,
                                   UBYTE         index,
                                   CHAR*         da,
                                   T_ACI_TOA*    toda )
{
  T_CONC_INIT_RETURN ret;
  TRACE_FUNCTION ("sAT_PlusCMSS ()");

  ret=concSMS_initSendFromMem(srcId, &index, da, toda);

  if (ret EQ CONC_NEEDED)
  {
    SET_CONC;

    /* memorize mem2, this is for rConcSMS_PlusCMGS */
    concShrdPrm.mem_store = smsShrdPrm.mem2;

    return sAT_PlusCMSS_Gl(srcId, index, da, toda, rConcSMS_PlusCMSS,
                           rConcSMS_PlusCMS_CMSS);
  }
  else if (ret EQ CONC_NOT_NEEDED)
  {
    return sAT_PlusCMSS_Gl(srcId, index, da, toda, rAT_PlusCMSS,
                           rAT_PlusCMS);
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_InValMemIdx );
    return ( AT_FAIL );
  }
}
#endif /*#if defined (MFW) OR defined (SMI) OR defined (_CONC_TESTING_)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMSS                 |
+-------------------------------------------------------------------+

  PURPOSE : For Riv-MMI.
*/
#if defined (FF_MMI_RIV) /* OR !defined TI_PS_FF_CONC_SMS */
GLOBAL T_ACI_RETURN sAT_PlusCMSS ( T_ACI_CMD_SRC srcId,
                                   UBYTE         index,
                                   CHAR*         da,
                                   T_ACI_TOA*    toda )
{
 
   TRACE_FUNCTION ("sAT_PlusCMSS ()");
   TRACE_EVENT_P1("sAT_PlusCMGS () index: %d", index);
   return sAT_PlusCMSS_Gl(srcId, index, da, toda, rAT_PlusCMSS,
                           rAT_PlusCMS);
    
}
#endif /*#if defined (FF_MMI_RIV)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMSS_Gl              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMSS
            AT command which is responsible for sending a short
            message from memory.

            <index>:   storage area index
            <da>:      destination address
            <toda>:    type of destination address
            <rplyCB>:  reply call-back
            <errorCB>: error call-back
*/
GLOBAL T_ACI_RETURN sAT_PlusCMSS_Gl ( T_ACI_CMD_SRC srcId,
                                      UBYTE         index,
                                      CHAR*         da,
                                      T_ACI_TOA*    toda,
                                      T_CMSS_FCT    rplyCB,
                                      T_ERROR_FCT   errorCB )
{
  T_SMS_SET_PRM * pSMSSetPrm;
  T_ACI_RETURN  ret;          /* AT response code                  */
  CHAR       *restDa = da;    /* destination address without       */
                              /* international access function     */
                              /* characters                        */
  T_tp_da da_addr;

#ifdef _CONC_TESTING_
/* Implements Measure#32 */
#endif

  TRACE_FUNCTION ("sAT_PlusCMSS_Gl ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;


  smsShrdPrm.rplyCB.cmss = rplyCB;
  smsShrdPrm.errorCB     = errorCB;

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];


  /*
   *-----------------------------------------------------------------
   * process the <da> parameter
   *-----------------------------------------------------------------
   */
  if ( da NEQ NULL )
  {
    /*
     *---------------------------------------------------------------
     * process the <toda> parameter
     *---------------------------------------------------------------
     */
    if ( toda NEQ NULL )
    {
       da_addr.ton = toda -> ton;
       da_addr.npi = toda -> npi;
    }
    else
    {
      restDa = cmhSMS_setToaDef ( da, &da_addr.ton,
                                      &da_addr.npi );
    }

    if ( da_addr.ton EQ TON_Alphanumeric )
    {
      if( cmhSMS_packAlphaNumAddr(restDa, &da_addr) EQ AT_FAIL )
      {
        return (AT_FAIL);
      }
    }
    else
    {
      cmhSMS_getAdrBcd ( da_addr.num, &da_addr.c_num,
                       MAX_SMS_ADDR_DIG, restDa);
      da_addr.digits = da_addr.c_num;
    }

    /* check da_addr if FDN enabled */
   /* if( ( simShrdPrm.crdFun EQ SIM_FDN_ENABLED OR
          simShrdPrm.crdFun EQ SIM_FDN_BDN_ENABLED ) AND 
          pb_get_fdn_classtype() EQ CLASS_VceDatFaxSms  )*/
    if (pb_get_fdn_mode () EQ FDN_ENABLE AND             
         pb_get_fdn_classtype() EQ CLASS_VceDatFaxSms  )
    {

      TRACE_EVENT_P1("sAT_PlusCMSS_Gl: FDN check of %s", restDa );
      
      if (pb_check_fdn (0, (const UBYTE*) restDa) EQ PHB_OK)
      {
        TRACE_EVENT("sAT_PlusCMSS_Gl: Found match in FDN!");
      }
      else 
      {
        TRACE_EVENT("sAT_PlusCMSS_Gl: No match in FDN found, SMS rejected!");
        ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_OpNotAllowed );
        return ( AT_FAIL );
      }
    } 
    /* end: check FDN if enabled */
  }
  else
  {
    #ifdef SIM_TOOLKIT
    /* FDN entry is already controlled by SAT */
    if (psaSIM_ChkSIMSrvSup( SRV_MOSMCtrlSIM ))
    {
      memset (&da_addr, 0, sizeof(T_tp_da));
    }
    else
    {
    #endif
      /* if FDN enabled read SMS from storage and verify address 
         in callback function cmhSMS_SMReadCMSS() */
   /*   if( ( simShrdPrm.crdFun EQ SIM_FDN_ENABLED OR
           simShrdPrm.crdFun EQ SIM_FDN_BDN_ENABLED ) AND 
           pb_get_fdn_classtype() EQ CLASS_VceDatFaxSms  )*/
       if (pb_get_fdn_mode () EQ FDN_ENABLE AND             
         pb_get_fdn_classtype() EQ CLASS_VceDatFaxSms  )
      {
        smsShrdPrm.smsEntStat.curCmd = AT_CMD_CMSS;
        smsShrdPrm.owner = (T_OWN)srcId;
        smsShrdPrm.smsEntStat.entOwn = srcId;
        psaSMS_ReadReq(smsShrdPrm.mem2, index, READ_PREVIEW, SMS_STAT_NotPresent);
        return ( AT_EXCT );        
      }
      else /* proceed with empty address */        
      {
        memset (&da_addr, 0, sizeof(T_tp_da));
      }
#ifdef SIM_TOOLKIT
    } /* end else psaSIM_ChkSIMSrvSup( SRV_MOSMCtrlSIM )*/
#endif

  }
  

  ACI_MALLOC(smsShrdPrm.tpdu.tp_submit, sizeof(T_TP_SUBMIT));
  cmhSMS_fillTpSubmit (smsShrdPrm.tpdu.tp_submit,
                       srcId,
                       0, /* fo */
                       0,
                       &da_addr,
                       NULL,  /* data */
                       0,
                       NULL   /* udh  */   );


  {
    PALLOC (mnsms_submit_req, MNSMS_SUBMIT_REQ);
    memset (mnsms_submit_req, 0, sizeof(T_MNSMS_SUBMIT_REQ));

    memcpy (&smsShrdPrm.tpdu.sc_addr, &pSMSSetPrm->sca, sizeof(T_rp_addr));
    mnsms_submit_req -> modify = SMS_MODIFY_SCA;

    if (da_addr.digits NEQ 0)
    {
      /* modify destination address */
      mnsms_submit_req -> modify |= SMS_MODIFY_TPOA;
    }
    mnsms_submit_req -> mem_type = smsShrdPrm.mem2;
    mnsms_submit_req -> rec_num  = index;
    mnsms_submit_req -> condx    = SMS_CONDX_OVR_ANY;

#ifdef REL99
    if(srcId NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT)
    {
      mnsms_submit_req -> auto_rep_flag = smsShrdPrm.auto_repeat_flag;
    }
    else
    {
      mnsms_submit_req -> auto_rep_flag = FALSE;
    }
#endif /* REL99 */


    /*
     *-----------------------------------------------------------------
     * read the short message from memory
     *-----------------------------------------------------------------
     */
    smsShrdPrm.smsEntStat.curCmd = AT_CMD_CMSS;
    smsShrdPrm.owner = (T_OWN)srcId;
    smsShrdPrm.smsEntStat.entOwn = srcId;

    /*
     *-----------------------------------------------------------------
     * check if message should be transfered directly to SIM for SAT
     *-----------------------------------------------------------------
     */
#ifdef SIM_TOOLKIT

    if (psaSIM_ChkSIMSrvSup( SRV_MOSMCtrlSIM ))
    {
        if (simShrdPrm.setPrm[srcId].sat_cc_mode EQ SATCC_CONTROL_BY_SIM_ACTIVE)
        {
            if (da_addr.digits)
            {
                ret = cmhSAT_MoSmCntr( smsShrdPrm.tpdu.sc_addr,
                                       da_addr,
                                       (UBYTE)srcId );
                if (ret NEQ AT_CMPL)
                {
                    /* save primitive address for SIM_TOOLKIT response */
                    sat_mnsms_submit_req = mnsms_submit_req;
                    return ret;
                }
            }
            else
            {
                /* send MNSMS_READ_REQ to SMS entity */
                psaSMS_ReadReq ( smsShrdPrm.mem2, index, READ_PREVIEW, SMS_STAT_NotPresent);
                if (smsShrdPrm.tpdu.tp_submit NEQ NULL)
                {
                  ACI_MFREE(smsShrdPrm.tpdu.tp_submit);
                  smsShrdPrm.tpdu.tp_submit = NULL;
                }
                PFREE(mnsms_submit_req);
                return ( AT_EXCT );
            }
        }
        else
        {
            simShrdPrm.setPrm[srcId].sat_cc_mode = SATCC_CONTROL_BY_SIM_ACTIVE;
        }
    }

#endif /* SIM_TOOLKIT */

    /* code SMS-SUBMIT here */
    cmhSMS_codeMsg (&mnsms_submit_req->sms_sdu, SMS_VT_SUBMIT,
                    &smsShrdPrm.tpdu.sc_addr, SMS_SUBMIT,
                   (UBYTE*)smsShrdPrm.tpdu.tp_submit);

    /* send message */
    PSENDX (SMS, mnsms_submit_req);
#ifdef _CONC_TESTING_
/* Implements Measure#32 */
    cmhSMS_sdu_buf_print(&(mnsms_submit_req->sms_sdu.buf[0]), 0);
#endif
    if (smsShrdPrm.tpdu.tp_submit NEQ NULL)
    {
      ACI_MFREE(smsShrdPrm.tpdu.tp_submit);
      smsShrdPrm.tpdu.tp_submit = NULL;
    }
  }

  smsShrdPrm.smsEntStat.curCmd = AT_CMD_CMSS;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;

  return( AT_EXCT );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGD                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGD
            AT command which is responsible for deleting a short
            message from memory.

            Features Concatenated SMS.
            For GPF-MMI.
            
            <index>: storage area index
*/
#if (defined (MFW) OR defined (SMI) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS
GLOBAL T_ACI_RETURN sAT_PlusCMGD ( T_ACI_CMD_SRC  srcId,
                                   UBYTE          index,
                                   UBYTE          status )
{
  T_CONC_INIT_RETURN ret;

  TRACE_FUNCTION ("sAT_PlusCMGD ()");

  ret=concSMS_initDeleteFromMem (srcId, index);

  if (ret EQ CONC_NEEDED)
  {
    SET_CONC;

    /* memorize mem1, this is for rConcSMS_PlusCMGD */
    concShrdPrm.mem_store = smsShrdPrm.mem1;

    return sAT_PlusCMGD_Gl(srcId, index, status, rConcSMS_PlusCMGD,
                           rConcSMS_PlusCMS_CMGD);
  }
  else if (ret EQ CONC_NOT_NEEDED)
  {
    return sAT_PlusCMGD_Gl(srcId, index, status, rAT_PlusCMGD, rAT_PlusCMS);
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_InValMemIdx );
    return ( AT_FAIL );
  }
}
#endif /*#if defined (MFW) OR defined (SMI) OR defined (_CONC_TESTING_)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGD                 |
+-------------------------------------------------------------------+

  PURPOSE : For Riv-MMI.
*/
#if defined (FF_MMI_RIV) /* OR !defined TI_PS_FF_CONC_SMS */
GLOBAL T_ACI_RETURN sAT_PlusCMGD ( T_ACI_CMD_SRC  srcId,
                                   UBYTE          index,
                                   UBYTE          status )
{
  TRACE_FUNCTION ("sAT_PlusCMGD ()");
  TRACE_EVENT_P1("sAT_PlusCMGD () index: %d", index);
 
  return sAT_PlusCMGD_Gl(srcId, index, status, rAT_PlusCMGD, rAT_PlusCMS); 
}
#endif /*#if defined (FF_MMI_RIV)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGD_Gl              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGD
            AT command which is responsible for deleting a short
            message from memory.

            <index>:   storage area index
            <rplyCB>:  reply call-back
            <errorCB>: error call-back
*/
GLOBAL T_ACI_RETURN sAT_PlusCMGD_Gl ( T_ACI_CMD_SRC srcId,
                                      UBYTE         index,
                                      UBYTE         status,
                                      T_CMGD_FCT    rplyCB,
                                      T_ERROR_FCT   errorCB )
{
  T_ACI_RETURN  ret;          /* AT response code                */

  TRACE_FUNCTION ("sAT_PlusCMGD_Gl ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  smsShrdPrm.errorCB     = errorCB;
  smsShrdPrm.rplyCB.cmgd = rplyCB;

  cmhSMS_SendDelete_Req (index, status);

  smsShrdPrm.smsEntStat.curCmd  = AT_CMD_CMGD;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn  = srcId;

  return( AT_EXCT );
}




/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGW_Old             |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGW
            AT command which is responsible for writing a short
            message to memory.

            <index>:   index of location area to be written
            <address>: originating/destination address
            <toa>:     type of address
            <stat>:    message status
            <data>:    message data
            <sca>:     service center address
            <tosca>:   type of service center address
            <isReply>:  > 0: set TP-Reply-Path explicitly
                       EQ 0: clear TP-Reply-Path explicitly
*/
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
GLOBAL T_ACI_RETURN sAT_PlusCMGW_Old ( T_ACI_CMD_SRC  srcId,
                                   SHORT          index,
                                   CHAR*          address,
                                   T_ACI_TOA*     toa,
                                   T_ACI_SMS_STAT stat,
                                   UBYTE          msg_ref,
                                   T_ACI_SM_DATA* data,
                                   CHAR*          sca,
                                   T_ACI_TOA*     tosca,
                                   SHORT          isReply )
{
  TRACE_FUNCTION ("sAT_PlusCMGW_Old ()");

  return sAT_PlusCMGW_Gl
         (srcId, index, address, toa, stat, msg_ref, data,
          NULL, sca, tosca, isReply, rAT_PlusCMGW, rAT_PlusCMS);
}
#endif /*#if defined SMI OR defined MFW */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGW                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGW
            AT command which is responsible for writing a short
            message to memory.

            Features Concatenated SMS.
            For GPF-MMI.

            <index>:   index of location area to be written
            <address>: originating/destination address
            <toa>:     type of address
            <stat>:    message status
            <data>:    message data
            <sca>:     service center address
            <tosca>:   type of service center address
            <isReply>:  > 0: set TP-Reply-Path explicitly
                       EQ 0: clear TP-Reply-Path explicitly
*/
#if (defined (MFW) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS
GLOBAL T_ACI_RETURN sAT_PlusCMGW ( T_ACI_CMD_SRC  srcId,
                                   SHORT          index,
                                   CHAR*          address,
                                   T_ACI_TOA*     toa,
                                   T_ACI_SMS_STAT stat,
                                   UBYTE          msg_ref,
                                   T_SM_DATA_EXT* src_data,
                                   CHAR*          sca,
                                   T_ACI_TOA*     tosca,
                                   SHORT          isReply )
{
  T_CONC_INIT_RETURN ret;
  T_ACI_UDH_DATA udh;
  T_ACI_SM_DATA tar_data;
  UBYTE alphabet;

  TRACE_FUNCTION ("sAT_PlusCMGW ()");


  alphabet = cmhSMS_getAlphabetPp ( smsShrdPrm.pSetPrm[srcId]->dcs );

  /* cut off more than CONC_MAX_SEGS segments */
  if (alphabet EQ 0)
  {
    src_data->len = MINIMUM (src_data->len, concShrdPrm.l_uncomp7bit_data_conc*CONC_MAX_SEGS);
  }
  else
  {
    src_data->len = MINIMUM (src_data->len, concShrdPrm.l_uncomp8bit_data_conc*CONC_MAX_SEGS);
  }

  ret=concSMS_initStoreInMem(&tar_data, &udh, srcId, index, address, toa, stat,
                             msg_ref, src_data, sca, tosca, isReply, alphabet);

  if (ret EQ CONC_NEEDED)
  {
    /* check if concBuffer is already full */
    if (concSMS_concBufferAvail() EQ FALSE)
    {
      TRACE_ERROR ("no concBuffer available, so CSMS can not be written");
      ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_MemFull );
      return AT_FAIL;
    }

    /* limit the maximum number of CSMS segments to MAX_SEG_TOTAL */
    if (concShrdPrm.elem_count+concShrdPrm.udh.max_num > MAX_SEG_TOTAL)
    {
      TRACE_ERROR ("total number of segments will exceed the maximum, so CSMS can not be written");
      ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_UnknownErr );
      return AT_FAIL;
    }

    SET_CONC;
    return sAT_PlusCMGW_Gl(srcId, index, address, toa, stat,
                           msg_ref, &tar_data, &udh, sca, tosca, isReply,
                           rConcSMS_PlusCMGW, rConcSMS_PlusCMS_CMGW);
  }
  else
  {
    return sAT_PlusCMGW_Gl(srcId, index, address, toa, stat,
                           msg_ref, &tar_data, (T_ACI_UDH_DATA*)NULL,
                           sca, tosca, isReply,
                           rAT_PlusCMGW, rAT_PlusCMS);
  }
}
#endif /*#if defined (MFW) OR defined (_CONC_TESTING_)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGW                 |
+-------------------------------------------------------------------+

  PURPOSE : For Riv-MMI if conc. is not needed.
*/
#if defined (FF_MMI_RIV) /* OR !defined TI_PS_FF_CONC_SMS */
GLOBAL T_ACI_RETURN sAT_PlusCMGW ( T_ACI_CMD_SRC  srcId,
                                   SHORT          index,
                                   CHAR*          address,
                                   T_ACI_TOA*     toa,
                                   T_ACI_SMS_STAT stat,
                                   UBYTE          msg_ref,
                                   T_ACI_SM_DATA* src_data,
                                   CHAR*          sca,
                                   T_ACI_TOA*     tosca,
                                   SHORT          isReply )
{
  TRACE_FUNCTION ("sAT_PlusCMGW ()");

    return sAT_PlusCMGW_Gl(srcId, index, address, toa, stat,
                           msg_ref, src_data, (T_ACI_UDH_DATA*)NULL,
                           sca, tosca, isReply,
                           rAT_PlusCMGW, rAT_PlusCMS);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PercentCMGW              |
+-------------------------------------------------------------------+

  PURPOSE : Adding Percent CMGW command to handle udh and src 
            data for each segment for conc module.
            
            For Riv-MMI if conc. is needed.
*/
GLOBAL T_ACI_RETURN sAT_PercentCMGW ( T_ACI_CMD_SRC  srcId,
                                   SHORT          index,
                                   CHAR*          address,
                                   T_ACI_TOA*     toa,
                                   T_ACI_SMS_STAT stat,
                                   UBYTE          msg_ref,
                                   T_ACI_SM_DATA* src_data,
                                   T_ACI_UDH_DATA* udh_data,
                                   CHAR*          sca,
                                   T_ACI_TOA*     tosca,
                                   SHORT          isReply )
{
  TRACE_FUNCTION ("sAT_PercentCMGW with udh()");

    return sAT_PlusCMGW_Gl(srcId, index, address, toa, stat,
                           msg_ref, src_data, udh_data,
                           sca, tosca, isReply,
                           rAT_PlusCMGW, rAT_PlusCMS);
}

#endif /*#if defined (FF_MMI_RIV)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGW_Gl              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGW
            AT command which is responsible for writing a short
            message to memory.

            <index>:   index of location area to be written
            <address>: originating/destination address
            <toa>:     type of address
            <stat>:    message status
            <data>:    message data
            <udh>:     user data header
            <sca>:     service center address
            <tosca>:   type of service center address
            <isReply>:  > 0: set TP-Reply-Path explicitly
                       EQ 0: clear TP-Reply-Path explicitly
            <rplyCB>:  reply call-back
            <errorCB>: error call-back
*/
GLOBAL T_ACI_RETURN sAT_PlusCMGW_Gl ( T_ACI_CMD_SRC   srcId,
                                      SHORT           index,
                                      CHAR*           address,
                                      T_ACI_TOA*      toa,
                                      T_ACI_SMS_STAT  stat,
                                      UBYTE           msg_ref,
                                      T_ACI_SM_DATA*  data,
                                      T_ACI_UDH_DATA* udh,
                                      CHAR*           sca,
                                      T_ACI_TOA*      tosca,
                                      SHORT           isReply,
                                      T_CMGW_FCT      rplyCB,
                                      T_ERROR_FCT     errorCB)
{
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set           */
  T_ACI_RETURN  ret;          /* AT response code                      */
  CHAR          * restAddress = address;
                              /* address without international access  */
                              /* function characters                   */
  UBYTE       chkIdx;         /* checked parameter <index>             */
  UBYTE       chkStat;        /* checked parameter <stat>              */
  CHAR* restSca = sca;        /* service center address without        */
                              /* international access function         */
                              /* characters                            */
  UBYTE msgType;

  T_ACI_SM_DATA packedData;
  T_tp_da da_addr;
  T_rp_addr sc_addr;
  UBYTE mr;
  UBYTE  byte_offset = 0;

#ifdef _CONC_TESTING_
/* Implements Measure#32 */
#endif

  TRACE_FUNCTION ("sAT_PlusCMGW_Gl ()");


  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  smsShrdPrm.rplyCB.cmgw = rplyCB;
  smsShrdPrm.errorCB     = errorCB;

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  /*
   *-----------------------------------------------------------------
   * process the <index> parameter
   *-----------------------------------------------------------------
   */
  if( index NEQ ACI_NumParmNotPresent )
  {
    if( ( index > SMS_CMH_IDX_MAX OR index < SMS_CMH_IDX_MIN ) AND
        index NEQ CMGW_IDX_FREE_ENTRY )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    else

      chkIdx = (UBYTE) index;
  }
  else

    chkIdx = CMGW_IDX_FREE_ENTRY;

  /*
   *-----------------------------------------------------------------
   * process the <address> parameter
   *-----------------------------------------------------------------
   */

  memset (&da_addr, 0, sizeof(T_tp_da));
  if ( address NEQ NULL )
  {
    /*
     *---------------------------------------------------------------
     * process the <toa> parameter
     *---------------------------------------------------------------
     */
    if ( toa NEQ NULL )
    {
       da_addr.ton = toa -> ton;
       da_addr.npi = toa -> npi;
    }
    else
    {
      restAddress = cmhSMS_setToaDef ( address,
                                       &da_addr.ton,
                                       &da_addr.npi );
    }

    if ( da_addr.ton EQ TON_Alphanumeric )
    {
      if( cmhSMS_packAlphaNumAddr(restAddress, &da_addr) EQ AT_FAIL )
      {
        return (AT_FAIL);
      }
    } 
    else
    {
      cmhSMS_getAdrBcd ( da_addr.num,
                       &da_addr.c_num,
                       MAX_SMS_ADDR_DIG,
                       restAddress );
      da_addr.digits = da_addr.c_num;
    }

  }
  else  /* if ( address NEQ NULL ) */
  {
    /*
     *---------------------------------------------------------------
     * process the <toa> parameter
     *---------------------------------------------------------------
     */
    if ( toa NEQ NULL )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    else
    {
      da_addr.digits = 0;
    }
  }

  /*
   *-----------------------------------------------------------------
   * process the <stat> parameter
   *-----------------------------------------------------------------
   */
  if( stat NEQ SMS_STAT_NotPresent )
  {
    if( !cmhSMS_getStatPsa ( stat, &chkStat ) )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
  }
  else
  {
    chkStat = STO_UNSENT;
  }

  /*
   *-------------------------------------------------------------------
   * create and send primitive for SMS store
   *-------------------------------------------------------------------
   */
  {
    PALLOC (mnsms_store_req, MNSMS_STORE_REQ);

    mnsms_store_req -> mem_type  = smsShrdPrm.mem2;
    if (chkIdx EQ CMGW_IDX_FREE_ENTRY)
      mnsms_store_req -> condx     = SMS_CONDX_OVR_NON;
    else
      mnsms_store_req -> condx     = SMS_CONDX_OVR_ANY;

    /*
     *-----------------------------------------------------------------
     * process the <sca> parameter
     *-----------------------------------------------------------------
     */
    if ( sca NEQ NULL AND *sca NEQ '\0' )
    {
      /*
       *---------------------------------------------------------------
       * process the <tosca> parameter
       *---------------------------------------------------------------
       */
      if ( tosca NEQ NULL )
      {
        sc_addr.ton = tosca -> ton;
        sc_addr.npi = tosca -> npi;
      }
      else
      {
        restSca = cmhSMS_setToaDef( sca, &sc_addr.ton,
                                         &sc_addr.npi);
      }
      cmhSMS_getAdrBcd( sc_addr.num,
                        &sc_addr.c_num,
                        MAX_SMS_ADDR_DIG,
                        restSca);
    }
    else
    {
      memcpy (&sc_addr, &pSMSSetPrm->sca, sizeof(T_rp_addr));
    }

    /*
     *-----------------------------------------------------------------
     * process the <isReply> parameter
     *-----------------------------------------------------------------
     */
    msgType  = (pSMSSetPrm->msgType & ~TP_MTI_MASK)
                                    | TP_MTI_SMS_SUBMIT;
    if (isReply EQ 0)
      msgType &= ~TP_RP_MASK;  /* set TP-Reply-Path bit to 0 */
    else if (isReply > 0)
      msgType |= TP_RP_MASK;   /* set TP-Reply-Path bit to 1 */

    /*
     *-----------------------------------------------------------------
     * copy parameter
     *-----------------------------------------------------------------
     */
    mnsms_store_req->rec_num = chkIdx;
    mnsms_store_req->status  = chkStat;

    /*
     *-----------------------------------------------------------------
     * process the <msg_ref> parameter
     *-----------------------------------------------------------------
     */
    if (msg_ref EQ NOT_PRESENT_8BIT)
    {
      mr = 0;
    }
    else
    {
      mr = msg_ref;
    }

    /*
     *-----------------------------------------------------------------
     * process the <data> parameter and
     * copy already to shared parameter
     *-----------------------------------------------------------------
     * reducing from 8 to 7 bit
     *-----------------------------------------------------------------
     */
    if (udh)
      byte_offset = udh->len+1;

    if (data EQ NULL)
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      PFREE (mnsms_store_req);

      return ( AT_FAIL );
    }

    cmhSMS_rdcSmsPp ( byte_offset,
                      pSMSSetPrm -> dcs,
                      ( UBYTE * ) data->data, ( UBYTE ) data->len,
                      packedData.data, &packedData.len);

    if ( (chkStat EQ REC_UNREAD) OR (chkStat EQ REC_READ) )
    {
      ACI_MALLOC(smsShrdPrm.tpdu.tp_deliver, sizeof(T_TP_DELIVER));
      cmhSMS_fillTpDeliver (smsShrdPrm.tpdu.tp_deliver,
                           srcId,
                           msgType,
                           (T_tp_oa *)&da_addr,
                           &packedData,
                           (UBYTE)data->len,
                           udh );

      /* code encode SMS-DELIVER here */
      cmhSMS_codeMsg (&mnsms_store_req->sms_sdu,
                      SMS_VT_DELIVER,
                      &sc_addr,
                      SMS_DELIVER,
                      (UBYTE*)smsShrdPrm.tpdu.tp_deliver);

      PSENDX (SMS, mnsms_store_req);
      smsShrdPrm.uiInternalSmsStorage = CMD_SRC_NONE;
      if (smsShrdPrm.tpdu.tp_deliver NEQ NULL)
      {
        ACI_MFREE(smsShrdPrm.tpdu.tp_deliver);
        smsShrdPrm.tpdu.tp_deliver = NULL;
      }
    }
    else
    {
      ACI_MALLOC(smsShrdPrm.tpdu.tp_submit, sizeof(T_TP_SUBMIT));
      cmhSMS_fillTpSubmit (smsShrdPrm.tpdu.tp_submit,
                           srcId,
                           msgType,
                           mr,
                           &da_addr,
                           &packedData,
                           data->len,
                           udh );

      /* encode SMS-SUBMIT here */
      cmhSMS_codeMsg (&mnsms_store_req->sms_sdu,
                      SMS_VT_SUBMIT,
                      &sc_addr,
                      SMS_SUBMIT,
                      (UBYTE*)smsShrdPrm.tpdu.tp_submit);

      PSENDX (SMS, mnsms_store_req);
      smsShrdPrm.uiInternalSmsStorage = CMD_SRC_NONE;
#ifdef _CONC_TESTING_
      TRACE_EVENT_P1("cmgw:rec_num: %d", mnsms_store_req->rec_num);
      TRACE_EVENT_P1("cmgw:status:  %d", mnsms_store_req->status);
      TRACE_EVENT_P1("cmgw:mem_type:%d", mnsms_store_req -> mem_type);
      TRACE_EVENT_P1("cmgw:condx:   %d", mnsms_store_req -> condx);
/* Implements Measure#32 */
      cmhSMS_sdu_buf_print(&(mnsms_store_req->sms_sdu.buf[0]), 0);
#endif
      if (smsShrdPrm.tpdu.tp_submit NEQ NULL)
      {
        ACI_MFREE(smsShrdPrm.tpdu.tp_submit);
        smsShrdPrm.tpdu.tp_submit = NULL;
      }
    }
  }

  smsShrdPrm.smsEntStat.curCmd  = AT_CMD_CMGW;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;

  return( AT_EXCT );
}

#if defined (SMS_PDU_SUPPORT)

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGWPdu              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGW
            AT command which is responsible for writing a short
            message to memory in PDU mode.

*/

GLOBAL T_ACI_RETURN sAT_PlusCMGWPdu ( T_ACI_CMD_SRC  srcId,
                                      UBYTE          stat,
                                      T_ACI_SM_DATA  *pdu)
{
  T_ACI_RETURN  ret;          /* AT response code                    */
  UBYTE         chkStat = '\0'; /* checked parameter <stat>              */

  TRACE_FUNCTION ("sAT_PlusCMGWPdu ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

 #if defined SMI OR defined MFW OR defined FF_MMI_RIV
   smsShrdPrm.errorCB      = rAT_PlusCMS;
 #endif /* defined SMI OR defined MFW OR defined FF_MMI_RIV */

  /*
   *-----------------------------------------------------------------
   * process the <stat> parameter
   *-----------------------------------------------------------------
   */
  switch (stat)
  {
    case SMS_STAT_RecUnread:
      chkStat = REC_UNREAD;
      break;
    case SMS_STAT_RecRead:
      chkStat = REC_READ;
      break;
    case SMS_STAT_StoUnsent:
      chkStat = STO_UNSENT;
      break;
    case SMS_STAT_StoSent:
      chkStat = STO_SENT;
      break;
  }

  /*
   *-------------------------------------------------------------------
   * create and send primitive for SMS store
   *-------------------------------------------------------------------
   */
  {
    PALLOC (mnsms_store_req, MNSMS_STORE_REQ);

    mnsms_store_req -> mem_type  = smsShrdPrm.mem2;
    mnsms_store_req -> rec_num   = CMGW_IDX_FREE_ENTRY;
    mnsms_store_req -> condx     = SMS_CONDX_OVR_NON;
    mnsms_store_req -> status    = chkStat;

    if ( pdu->len > 0 )
    {
      mnsms_store_req->sms_sdu.l_buf = pdu->len * 8;
      mnsms_store_req->sms_sdu.o_buf = 0;
      memcpy (mnsms_store_req->sms_sdu.buf, pdu->data, pdu->len);
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      PFREE (mnsms_store_req);
      return ( AT_FAIL );
    }

    PSENDX (SMS, mnsms_store_req);
  }

  smsShrdPrm.smsEntStat.curCmd  = AT_CMD_CMGW;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn  = srcId;
  smsShrdPrm.uiInternalSmsStorage = CMD_SRC_NONE;

  return( AT_EXCT );
}

#endif


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGC                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGC
            AT command which is responsible for sending a command.

            Features Concatenated SMS.
            For GPF-MMI.

            <fo>:   first octet of SMS-COMMAND
            <ct>:   command type
            <pid>:  protocol identifier
            <mn>:   message number
            <da>:   destination address
            <toda>: type of destination address
            <data>: command data
*/
#if (defined (MFW) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS
GLOBAL T_ACI_RETURN sAT_PlusCMGC ( T_ACI_CMD_SRC   srcId,
                                   SHORT           fo,
                                   SHORT           ct,
                                   SHORT           pid,
                                   SHORT           mn,
                                   CHAR*           da,
                                   T_ACI_TOA*      toda,
                                   T_ACI_CMD_DATA* data )
{
  T_CONC_INIT_RETURN ret;

  TRACE_FUNCTION ("sAT_PlusCMGC ()");


  ret=concSMS_initCommand (srcId, fo, ct, pid, mn, da, toda,
                           data);

  if (ret EQ CONC_NEEDED)
  {
    SET_CONC;
    return sAT_PlusCMGC_Gl(srcId, fo, ct, pid, mn, da, toda,
                           data, rConcSMS_PlusCMGC);
  }
  else if (ret EQ CONC_NOT_NEEDED)
  {
    return sAT_PlusCMGC_Gl(srcId, fo, ct, pid, mn, da, toda,
                           data, rAT_PlusCMGC);
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_UnknownErr );
    return ( AT_FAIL );
  }
}
#endif /*#if defined (MFW) OR defined (_CONC_TESTING_)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGC                 |
+-------------------------------------------------------------------+

  PURPOSE : New Plus CMGC command for handling the conc module on 
            the Riv-MMI side.
*/
#if defined (FF_MMI_RIV) /* OR !defined TI_PS_FF_CONC_SMS */
GLOBAL T_ACI_RETURN sAT_PlusCMGC ( T_ACI_CMD_SRC   srcId,
                                   SHORT           fo,
                                   SHORT           ct,
                                   SHORT           pid,
                                   SHORT           mn,
                                   CHAR*           da,
                                   T_ACI_TOA*      toda,
                                   T_ACI_CMD_DATA* data )
{
   TRACE_FUNCTION ("sAT_PlusCMGC ()");
   TRACE_EVENT_P1("sAT_PlusCMGC () mn: %d", mn);

   return sAT_PlusCMGC_Gl(srcId, fo, ct, pid, mn, da, toda,
                           data, rAT_PlusCMGC); 
}
#endif /*#if defined (FF_MMI_RIV)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGC_Gl              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGC
            AT command which is responsible for sending a command.

            <fo>:     first octet of SMS-COMMAND
            <ct>:     command type
            <pid>:    protocol identifier
            <mn>:     message number
            <da>:     destination address
            <toda>:   type of destination address
            <data>:   command data
            <rplyCB>: reply call-back
*/
GLOBAL T_ACI_RETURN sAT_PlusCMGC_Gl ( T_ACI_CMD_SRC   srcId,
                                      SHORT           fo,
                                      SHORT           ct,
                                      SHORT           pid,
                                      SHORT           mn,
                                      CHAR*           da,
                                      T_ACI_TOA*      toda,
                                      T_ACI_CMD_DATA* data,
                                      T_CMGC_FCT      rplyCB )
{
  T_ACI_RETURN  ret;          /* AT response code                  */
  UBYTE       chkFo;          /* checked parameter <fo>            */
  UBYTE       chkPid;         /* checked parameter <pid>           */
  UBYTE       chkCt;          /* checked parameter <ct>            */
  UBYTE       chkMn;          /* checked parameter <mn>            */
  CHAR*       restDa = da;    /* destination address without       */
                              /* international access function     */
                              /* characters                        */
  T_tp_da da_addr;            /* destination address               */

  TRACE_FUNCTION ("sAT_PlusCMGC_Gl ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  smsShrdPrm.rplyCB.cmgc = rplyCB;
  /*
   *-----------------------------------------------------------------
   * process the <fo> parameter
   *-----------------------------------------------------------------
   */
  if( fo NEQ ACI_NumParmNotPresent )
  {
    if( fo > SMS_CMH_FO_MAX OR fo < SMS_CMH_FO_MIN OR
        ( fo & TP_MTI_MASK ) NEQ TP_MTI_SMS_COMMAND )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    else

      chkFo = (UBYTE) fo;

  }
  else

    chkFo = TP_MTI_SMS_COMMAND;

  /*
   *-----------------------------------------------------------------
   * process the <ct> parameter
   *-----------------------------------------------------------------
   */
  if( ct NEQ ACI_NumParmNotPresent )
  {
    if( ct > SMS_CMH_CT_MAX OR ct < SMS_CMH_CT_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    else

      chkCt = (UBYTE) ct;

  }
  else
    /*
     * set to default value
     */
    chkCt = SMS_CT_ENQUIRY;

  /*
   *-----------------------------------------------------------------
   * process the <pid> parameter
   *-----------------------------------------------------------------
   */
  if( pid NEQ ACI_NumParmNotPresent )
  {
    if( pid > SMS_CMH_PID_MAX OR pid < SMS_CMH_PID_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    else

      chkPid = (UBYTE) pid;
  }
  else

    /*
     * set to default value
     */
    chkPid = SMS_PID_DEFAULT;

  /*
   *-----------------------------------------------------------------
   * process the <mn> parameter
   *-----------------------------------------------------------------
   */
  if( mn NEQ ACI_NumParmNotPresent )
  {
    if( mn > SMS_CMH_MN_MAX OR mn < SMS_CMH_MN_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
    else

      chkMn = (UBYTE) mn;
  }
  else
  {
    chkMn = smsShrdPrm.aci_sms_parameter.snd_msg_ref;
  }

  /*
   *---------------------------------------------------------------
   * process the <da> and <toda> parameter
   *---------------------------------------------------------------
   */
  if ( toda NEQ NULL )
  {
    if ( da NEQ NULL )
    {
       da_addr.ton = toda -> ton;
       da_addr.npi = toda -> npi;
       cmhSMS_getAdrBcd ( da_addr.num, &da_addr.c_num, MAX_SMS_ADDR_DIG, restDa);
       da_addr.digits = da_addr.c_num;
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }
  }
  else
  {
    if ( da NEQ NULL )
    {
      restDa = cmhSMS_setToaDef ( da, &da_addr.ton, &da_addr.npi );
      cmhSMS_getAdrBcd ( da_addr.num, &da_addr.c_num, MAX_SMS_ADDR_DIG, restDa);
      da_addr.digits = da_addr.c_num;
    }
    else
    {
      da_addr.digits = 0;
    }
  }

  /*
   *-------------------------------------------------------------------
   * create and send primitive for command request
   *-------------------------------------------------------------------
   */
  {
    PALLOC (mnsms_command_req, MNSMS_COMMAND_REQ);

    ACI_MALLOC(smsShrdPrm.tpdu.tp_command, sizeof(T_TP_COMMAND));
    cmhSMS_fillTpCommand( smsShrdPrm.tpdu.tp_command,
                          chkFo,
                          chkCt,
                          0,
                          chkPid,
                          chkMn,
                          &da_addr,
                          data,
                          NULL);

#ifdef REL99
    if(srcId NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT)
    {
      mnsms_command_req -> auto_rep_flag = smsShrdPrm.auto_repeat_flag;
    }
    else
    {
      mnsms_command_req -> auto_rep_flag = FALSE;
    }
#endif

    /* code command here */
    cmhSMS_codeMsg (&mnsms_command_req->sms_sdu,
                    SMS_VT_COMMAND,
                    &smsShrdPrm.pSetPrm[srcId]->sca,
                    SMS_COMMAND,
                    (UBYTE*)smsShrdPrm.tpdu.tp_command);

    PSENDX (SMS, mnsms_command_req);

    if (smsShrdPrm.tpdu.tp_command NEQ NULL)
    {
      ACI_MFREE(smsShrdPrm.tpdu.tp_command);
      smsShrdPrm.tpdu.tp_command = NULL;
    }
  }

  smsShrdPrm.smsEntStat.curCmd = AT_CMD_CMGC;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;

  return( AT_EXCT );
}


#if defined (SMS_PDU_SUPPORT) || defined (SIM_TOOLKIT)

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGCPdu              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGC
            AT command which is responsible for sending a command
            message in pdu mode.

*/
GLOBAL T_ACI_RETURN sAT_PlusCMGCPdu ( T_ACI_CMD_SRC   srcId,
                                      T_ACI_SM_DATA   *pdu )
{
  T_ACI_RETURN  ret;          /* AT response code                */

  TRACE_FUNCTION ("sAT_PlusCMGCPdu ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

 #if defined SMI OR defined MFW OR defined FF_MMI_RIV
    smsShrdPrm.rplyCB.cmgc  = rAT_PlusCMGC;
    smsShrdPrm.errorCB      = rAT_PlusCMS;
 #endif /* defined SMI OR defined MFW OR defined FF_MMI_RIV */

  /*
   *-------------------------------------------------------------------
   * create and send primitive for SMS command
   *-------------------------------------------------------------------
   */
  {
    PALLOC (mnsms_command_req, MNSMS_COMMAND_REQ);
#ifdef REL99
    if(srcId NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT)
    {
      mnsms_command_req -> auto_rep_flag = smsShrdPrm.auto_repeat_flag;
    }
    else
    {
      mnsms_command_req -> auto_rep_flag = FALSE;
    }
#endif /* REL99 */

    if ( pdu->len > 0 )
    {
      mnsms_command_req->sms_sdu.l_buf = pdu->len * 8;
      mnsms_command_req->sms_sdu.o_buf = 0;
      memcpy (mnsms_command_req->sms_sdu.buf, pdu->data, pdu->len);
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      PFREE (mnsms_command_req);
      return ( AT_FAIL );
    }

    PSENDX (SMS, mnsms_command_req);
  }

  smsShrdPrm.smsEntStat.curCmd  = AT_CMD_CMGC;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn  = srcId;

  return( AT_EXCT );
}
#endif


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGR                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGR
            AT command which is responsible for reading a short
            message from memory.

            Features Concatenated SMS.
            For GPF-MMI.

            <index>:  storage area index
            <rdMode>: read mode
*/
#if (defined (MFW) OR defined (SMI) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS
GLOBAL T_ACI_RETURN sAT_PlusCMGR ( T_ACI_CMD_SRC  srcId,
                                   UBYTE          index,
                                   T_ACI_SMS_READ rdMode )
{
  T_CONC_INIT_RETURN ret;

  TRACE_FUNCTION ("sAT_PlusCMGR ()");

  TRACE_EVENT_P1("sAT_PlusCMGR () index: %d", index);

  ret=concSMS_initReadFromMem(srcId, index, rdMode);

  if (ret EQ CONC_NEEDED)
  {
    SET_CONC;
    TRACE_EVENT("sAT_PlusCMGR: CONC_NEEDED");

    /* memorize mem1, this is for rConcSMS_PlusCMGR */
    concShrdPrm.mem_store = smsShrdPrm.mem1;

    return sAT_PlusCMGR_Gl(srcId, index, rdMode, rConcSMS_PlusCMGR);
  }
  else if (ret EQ CONC_NOT_NEEDED)
  {
    TRACE_EVENT("sAT_PlusCMGR: CONC_NOT_NEEDED");
    return sAT_PlusCMGR_Gl(srcId, index, rdMode, rAT_PlusCMGR);
  }
  else
  {
    TRACE_EVENT("ERROR: sAT_PlusCMGR: CMS_ERR_InValMemIdx");
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_InValMemIdx );
    return ( AT_FAIL );
  }
}
#endif /*#if defined (MFW) OR defined (SMI) OR defined (_CONC_TESTING_)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGR                 |
+-------------------------------------------------------------------+

  PURPOSE : New Plus CMGR command for handling the conc module on 
            the Riv-MMI side.
*/
#if defined (FF_MMI_RIV) /* OR !defined TI_PS_FF_CONC_SMS */
GLOBAL T_ACI_RETURN sAT_PlusCMGR ( T_ACI_CMD_SRC  srcId,
                                   UBYTE          index,
                                   T_ACI_SMS_READ rdMode )
{
  TRACE_FUNCTION ("sAT_PlusCMGR ()");

  TRACE_EVENT_P1("sAT_PlusCMGR () index: %d", index);

   return sAT_PlusCMGR_Gl(srcId, index, rdMode, rAT_PlusCMGR);
}
#endif /*#if defined (FF_MMI_RIV)*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGR_Gl              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGR
            AT command which is responsible for reading a short
            message from memory.

            <index>:   storage area index
            <rdMode>:  read mode
            <rplyCB>:  reply call-back
            <errorCB>: error call-back
*/
GLOBAL T_ACI_RETURN sAT_PlusCMGR_Gl ( T_ACI_CMD_SRC  srcId,
                                      UBYTE          index,
                                      T_ACI_SMS_READ rdMode,
                                      T_CMGR_FCT     rplyCB )
{
  T_ACI_RETURN ret = AT_EXCT;

  TRACE_FUNCTION ("sAT_PlusCMGR_Gl ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
  {
    return ret;
  }

  smsShrdPrm.rplyCB.cmgr = rplyCB;


  /*
   *-----------------------------------------------------------------
   * process the <index> parameter
   *-----------------------------------------------------------------
   */
  switch (rdMode)
  {
  case SMS_READ_StatusChange:
/*    ret = AT_CMPL;    Change to AT_EXCT, call back expected */
    smsShrdPrm.rdMode = READ_STATUS_CHANGE;
    break;
  case SMS_READ_Preview:
    smsShrdPrm.rdMode = READ_PREVIEW;
    break;
  case SMS_READ_Normal:
  case SMS_READ_NotPresent:
    smsShrdPrm.rdMode = READ_NORMAL;
    break;
  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return ( AT_FAIL );
  }
  /*
   *-----------------------------------------------------------------
   * read a short message from memory
   *-----------------------------------------------------------------
   */
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_CMGR;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;

  psaSMS_ReadReq ( smsShrdPrm.mem1, index,
                   smsShrdPrm.rdMode, SMS_STAT_NotPresent);

  return( ret );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCSCB                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CSCB
            AT command which is responsible for selecting the
            cell broadcast message types.

            <mode>: acception mode
            <mids>: message identifiers
            <dcss>: data coding schemes
*/
GLOBAL T_ACI_RETURN sAT_PlusCSCB ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_CSCB_MOD mode,
                                   USHORT*        mids,
                                   UBYTE*         dcss)
{
  UBYTE  chkMode;             /* checked parameter <mode>         */
  USHORT chkMids[MAX_IDENTS]; /* checked parameter <mids>         */
  UBYTE  chkDcss[MAX_IDENTS]; /* checked parameter <dcss>         */
  int  i;                     /* used for counting                */

  TRACE_FUNCTION ("sAT_PlusCSCB ()");

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
   * check access status
   *-----------------------------------------------------------------
   */
  if (!cmhSMS_checkSIM ())      /* sets error code */
  {
    return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check entity status
   *-----------------------------------------------------------------
   */
  if( smsShrdPrm.cbmPrm.cbchOwner NEQ ((T_OWN)CMD_SRC_NONE) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    return( AT_BUSY );
  }

  /*
   *-----------------------------------------------------------------
   * process the <mode> parameter
   *-----------------------------------------------------------------
   */
  switch ( mode )
  {
    case (CSCB_MOD_NotPresent): chkMode = smsShrdPrm.cbmPrm.cbmMode; break;
    case (CSCB_MOD_Accept    ): chkMode = CBCH_ACCEPT;        break;
    case (CSCB_MOD_NotAccept ): chkMode = CBCH_IGNORE;        break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * process the <mids> parameter
   *-----------------------------------------------------------------
   */
  if( mids NEQ NULL )

    memcpy ( ( CHAR * ) chkMids, ( CHAR * ) mids, sizeof (chkMids) );

  else

    memcpy ( ( CHAR * ) chkMids, ( CHAR * ) smsShrdPrm.cbmPrm.msgId,
             sizeof ( chkMids ) );

  /*
   *-----------------------------------------------------------------
   * process the <dcss> parameter
   *-----------------------------------------------------------------
   */
  if( dcss NEQ NULL )
  {
    i = 0;

    while ( i < MAX_IDENTS AND dcss[i] NEQ DEF_DCS_RANGE )
    {
/* Lint: this cannot happen
      if( dcss[i] > SMS_CMH_DCS_MAX OR dcss[i] < SMS_CMH_DCS_MIN )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return ( AT_FAIL );
      }
      else
*/
        i++;
    }

    if ( i % 2 NEQ 0 )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
    }

    memcpy ( ( CHAR * ) chkDcss, ( CHAR * ) dcss, sizeof (chkDcss) );
  }
  else

    memcpy ( ( CHAR * ) chkDcss, ( CHAR * ) smsShrdPrm.cbmPrm.dcsId,
             sizeof ( chkDcss ) );

  /*
   *-----------------------------------------------------------------
   * copy parameter
   *-----------------------------------------------------------------
   */
  memcpy ( ( CHAR * ) smsShrdPrm.cbmPrm.msgId,
           ( CHAR * ) chkMids,
           sizeof ( smsShrdPrm.cbmPrm.msgId ) );

  memcpy ( ( CHAR * ) smsShrdPrm.cbmPrm.dcsId,
           ( CHAR * ) chkDcss,
           sizeof ( smsShrdPrm.cbmPrm.dcsId ) );

  smsShrdPrm.cbmPrm.cbmMode = chkMode;

  /* CSCBmode = mode; */

  /*
   *-----------------------------------------------------------------
   * set the new CBCH configuration
   *-----------------------------------------------------------------
   */
  smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)srcId;

  if( psaMMI_Cbch() < 0 )
  {
    TRACE_EVENT( "FATAL RETURN psaSMS in +CSCB" );
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    return( AT_FAIL );
  }

  smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)CMD_SRC_NONE;

  return ( AT_CMPL );
}


#ifdef FF_HOMEZONE
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PercentCBHZ              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the %CBHZ
            AT command which is responsible for activation and
            deactivation of the homezone feature.

            <mode>   : activation mode
            <dcs>    : datacoding scheme
            <timeout>: timeout period for homezone CBM
*/
GLOBAL T_ACI_RETURN sAT_PercentCBHZ ( T_ACI_CMD_SRC  srcId,
                                      T_ACI_CBHZ_MOD mode,
                                      T_ACI_CS       dcs,
                                      UBYTE          timeout)
{
  UBYTE chkMode;             /* checked parameter <mode>        */
  UBYTE chkDcs;              /* checked parameter <dcs>         */
  UBYTE chkTimeout;          /* checked parameter <timeout>     */

  TRACE_FUNCTION ("sAT_PercentCBHZ()");

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
   * check access status
   *-----------------------------------------------------------------
   */
  if (!cmhSMS_checkSIM ())      /* sets error code */
  {
    return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check entity status
   *-----------------------------------------------------------------
   */
  if( smsShrdPrm.cbmPrm.cbchOwner NEQ ((T_OWN)CMD_SRC_NONE) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    return( AT_BUSY );
  }

  /*
   *-----------------------------------------------------------------
   * process the <mode> parameter
   *-----------------------------------------------------------------
   */
  switch ( mode )
  {
    case (CBHZ_MOD_Active)   :
    case (CBHZ_MOD_NotActive):
      chkMode = mode;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * process the <dcs> parameter
   *-----------------------------------------------------------------
   */
  switch ( dcs )
  {
    case (CS_NotPresent):
      chkDcs = CS_GsmDef;
      break;

    case (CS_GsmDef)    :
    case (CS_GsmInt)    :
    case (CS_Hex)       :
    case (CS_Ucs2)      :
    case (CS_Sim)       :
    case (CS_Ascii)     :
      chkDcs = dcs;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * process the <timeout> parameter
   *-----------------------------------------------------------------
   */
  if ( (timeout >= CBHZ_MIN_TIMEOUT) AND (timeout <= CBHZ_MAX_TIMEOUT) )
  {
    chkTimeout = timeout;
  }
  else if ( timeout EQ NOT_PRESENT_8BIT )
  {
    chkTimeout = CBHZ_DEF_TIMEOUT;
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return ( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * all parameters valid. now copy parameter
   *-----------------------------------------------------------------
   */
  smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)srcId;
  smsShrdPrm.cbmPrm.hzMode    = chkMode;
  smsShrdPrm.cbmPrm.hzDcs     = chkDcs;
  smsShrdPrm.cbmPrm.hzTimeout = chkTimeout;

  /* send homezone request */
  if( psaMMI_homezone_req() < 0 )
  {
    TRACE_EVENT( "FATAL RETURN psaSMS in %%CBHZ" );
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    return( AT_FAIL );
  }

  smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)CMD_SRC_NONE;

  return ( AT_CMPL );
}
#endif /* FF_HOMEZONE */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGL                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMGL
            AT command which is responsible for creating a list of
            all stored short messages.

            <state>: state of stored short message
*/
GLOBAL T_ACI_RETURN sAT_PlusCMGL ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_SMS_STAT state,
                                   SHORT          startIdx,
                                   T_ACI_SMS_READ rdMode )
{
  T_ACI_RETURN   ret;          /* AT response code            */
  T_ACI_SMS_STAT chk_state;

  TRACE_FUNCTION ("sAT_PlusCMGL ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
 {
    return ret;
  }

  /*
   *-----------------------------------------------------------------
   * process the <state> parameter
   *-----------------------------------------------------------------
   */
  switch ( state )
  {
    case( SMS_STAT_NotPresent ):
      chk_state = SMS_STAT_RecUnread;
      break;

    case( SMS_STAT_Invalid    ):
    case( SMS_STAT_All        ):
    case( SMS_STAT_RecUnread  ):
    case( SMS_STAT_RecRead    ):
    case( SMS_STAT_StoUnsent  ):
    case( SMS_STAT_StoSent    ):
      chk_state = state;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check if startIdx is in range
   *-----------------------------------------------------------------
   */
  if ( startIdx >= MAX_SIM_ME_ENTRIES /*256*/   OR
       startIdx >  SMS_CMH_MAX_START_IDX /*255*/    )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return ( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * process the <rdMode> parameter
   *-----------------------------------------------------------------
   */
  switch (rdMode)
  {
  case SMS_READ_StatusChange:
  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return ( AT_FAIL );
  case SMS_READ_Preview:
    smsShrdPrm.rdMode = READ_PREVIEW;
    break;
  case SMS_READ_Normal:
  case SMS_READ_NotPresent:
    smsShrdPrm.rdMode = READ_NORMAL;
    break;
  }

  /*
   *---------------------------------------------------------------
   * PSA notification
   *---------------------------------------------------------------
   */
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_CMGL;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;

  /*
   *---------------------------------------------------------------
   * request the list element
   *---------------------------------------------------------------
   */
  psaSMS_ReadReq ( smsShrdPrm.mem1, startIdx,
                   smsShrdPrm.rdMode, chk_state );

  return( AT_EXCT );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCSAS                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CSAS
            AT command which is responsible for saving of certain
            SMS parameters to nonvolatile memory.

            <profile>: SMS profile number (1..max_record)
*/
GLOBAL T_ACI_RETURN sAT_PlusCSAS ( T_ACI_CMD_SRC  srcId,
                                   SHORT       profile)
{
  T_ACI_RETURN  ret;             /* AT response code               */
  UBYTE tmpNumOfRefs;
  UBYTE max_record;

  TRACE_FUNCTION ("sAT_PlusCSAS ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  max_record = (UBYTE)cmhSMS_getPrfRge ();
  if ((profile <= 0) OR (profile > max_record))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_InValMemIdx );
    return AT_FAIL;
  }

  smsShrdPrm.cbmPrm.cbmFoundIds = 0;

  if ( (smsShrdPrm.pSetPrm[srcId]->prflId NEQ profile) OR
       (smsShrdPrm.pSetPrm[srcId]->isCopy EQ TRUE) )
  {
    T_SMS_SET_PRM * elem;
    T_SMS_SET_PRM * pSMSSetPrmOld = smsShrdPrm.pSetPrm[srcId];

    /*
     *-----------------------------------------------------------------
     * find or create new profile
     *-----------------------------------------------------------------
     */
    elem = find_element(set_prm_list, (UBYTE)profile, cmhSMS_findPrflId);
    if (elem EQ NULL)
    {
      ACI_MALLOC(elem, sizeof(T_SMS_SET_PRM));
      memset(elem, 0, sizeof(T_SMS_SET_PRM));
      insert_list(set_prm_list, elem);
    }

    /* save the number of references of the list element */
    tmpNumOfRefs = elem->numOfRefs;

    /* copy the old profile */
    memcpy(elem, pSMSSetPrmOld, sizeof(T_SMS_SET_PRM));

    /* decrease the numOfRefs of the old profile */
    pSMSSetPrmOld->numOfRefs--;

    /* set the Prm Pointer to the (new or found) list element */
    smsShrdPrm.pSetPrm[srcId] = elem;

    smsShrdPrm.pSetPrm[srcId]->numOfRefs = ++tmpNumOfRefs;
    smsShrdPrm.pSetPrm[srcId]->isCopy = FALSE;
    smsShrdPrm.pSetPrm[srcId]->prflId = (UBYTE)profile;

    /*
     *-----------------------------------------------------------------
     * destroy temporary copy
     *-----------------------------------------------------------------
     */
    if ( pSMSSetPrmOld->isCopy EQ TRUE )
    {
      ACI_MFREE(pSMSSetPrmOld);
    }
  }

  /*
   *-------------------------------------------------------------
   * store parameters to non-volatile memory
   *-------------------------------------------------------------
   */
  ret = cmhSMS_WriteParams (srcId, AT_CMD_CSAS, profile);
  return ret;

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCRES                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CRES
            AT command which is responsible for retrieving of
            certain SMS parameters from nonvolatile memory.

            <profile>: SMS profile number (1..max_record)
*/
GLOBAL T_ACI_RETURN sAT_PlusCRES ( T_ACI_CMD_SRC  srcId,
                                   SHORT       profile)
{
  T_ACI_RETURN  ret=AT_CMPL;     /* AT response code                */
  UBYTE max_record;

  TRACE_FUNCTION ("sAT_PlusCRES ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  max_record = (UBYTE)cmhSMS_getPrfRge ();
  if ((profile <= 0) OR (profile > max_record))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_InValMemIdx );
    return AT_FAIL;
  }

  smsShrdPrm.cbmPrm.cbmFoundIds = 0;

  if ( (smsShrdPrm.pSetPrm[srcId]->prflId NEQ profile) OR
       (smsShrdPrm.pSetPrm[srcId]->isCopy EQ TRUE) )
  {
    T_SMS_SET_PRM * pSMSSetPrmOld; /* points to SMS parameter set    */
    T_SMS_SET_PRM * pSMSSetPrmNew; /* points to SMS parameter set    */

    pSMSSetPrmOld = smsShrdPrm.pSetPrm[srcId];
    pSMSSetPrmOld->numOfRefs--;

    /*
     *-----------------------------------------------------------------
     * destroy temporary copy
     *-----------------------------------------------------------------
     */
    if ( pSMSSetPrmOld->isCopy EQ TRUE)
    {
      ACI_MFREE(pSMSSetPrmOld);
    }

    /*
     *-------------------------------------------------------------
     * find the new profile in the SET PRM list
     *-------------------------------------------------------------
     */
    pSMSSetPrmNew = find_element(set_prm_list, (UBYTE)profile, cmhSMS_findPrflId);
    if (pSMSSetPrmNew NEQ NULL)  /* element was in SET PRM List */
    {
      smsShrdPrm.pSetPrm[srcId] = pSMSSetPrmNew;
      smsShrdPrm.pSetPrm[srcId]->numOfRefs++;
    }
    else                         /* element was not found in SET PRM List */
    {
      ACI_MALLOC(pSMSSetPrmNew, sizeof(T_SMS_SET_PRM));
      memset(pSMSSetPrmNew, 0, sizeof(T_SMS_SET_PRM));
      insert_list(set_prm_list, pSMSSetPrmNew);
      pSMSSetPrmNew->prflId = (UBYTE)profile;

      smsShrdPrm.pSetPrm[srcId] = pSMSSetPrmNew;
    }
  }

  /*
   *-------------------------------------------------------------
   * read parameters from non-volatile memory
   *-------------------------------------------------------------
       */
      ret = cmhSMS_ReadParams (srcId, AT_CMD_CRES, profile);
  return ret;

}



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSS                     |
| STATE   : code                  ROUTINE : tAT_PlusCRES                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CRES
            AT command which is responsible for retrieving of
            certain SMS parameters from nonvolatile memory.

            Shen,Chao July 1st, 2003
            
*/
GLOBAL T_ACI_RETURN tAT_PlusCRES ( T_ACI_CMD_SRC  srcId, T_ACI_CRES *values)
{
SHORT   profile  = -1;
  if ((profile = cmhSMS_getPrfRge ()) < 1)
  {
    return (AT_FAIL);
  }

  values->min = 0;
  values->max = profile - 1;

  return AT_CMPL;
}

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSS                |
| STATE   : code                  ROUTINE : sAT_PercentCPRSM        |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the %CPRSM 
            AT command which is responsible for setting the receiving 
            of SMS to pause or resume.
            [MDF: 07.04.2004]             
*/
GLOBAL T_ACI_RETURN sAT_PercentCPRSM ( T_ACI_CMD_SRC  srcId, 
                                       T_ACI_CPRSM_MOD mode)
{
  T_ACI_RETURN ret;

  TRACE_FUNCTION("sAT_PercentCPRSM()");

  /* check whether SMS is currently accessible */ 
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  /* check mode and inform SMS entity */
  switch ( mode )
  {
    case CPRSM_MOD_Pause:
      /* send pause request to SMS */
      psaSMS_PauseReq();
      /* command complete */
      ret = AT_CMPL;
      break;
    case CPRSM_MOD_Resume:
      /* store paramters for current command */
      smsShrdPrm.smsEntStat.curCmd = AT_CMD_CPRSM;
      smsShrdPrm.owner = (T_OWN)srcId; 
      smsShrdPrm.smsEntStat.entOwn = srcId;      
      /* send resume request to SMS */
      psaSMS_ResumeReq();
      ret = AT_EXCT;
      break;
    default:
      TRACE_EVENT_P1("sAT_PercentCPRSM ERROR: mode %d not supported", mode);
      ret = AT_FAIL;
  }
  return ret;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSS                |
| STATE   : code                  ROUTINE : qAT_PercentCPRSM        |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the %CPRSM 
            AT command which is responsible for quering the status 
            of receiving of SMS. It might be resume or pause. 
            [MDF: 15.04.2004]             
*/
GLOBAL T_ACI_RETURN qAT_PercentCPRSM ( T_ACI_CMD_SRC  srcId )
{
  T_ACI_RETURN ret = AT_EXCT;

  TRACE_FUNCTION("qAT_PercentCPRSM()");
  
  /* check whether SMS is currently accessible */ 
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  /* store paramters for current command */
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_CPRSM;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;      

  /* add parameter for signaling which parameter has been requested */
  psaSMS_QueryReq (SMS_QUERY_DELIVER_STATUS);
  
  return ret;
}
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMMS                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CMMS
            AT command which is responsible for setting the
            mode.

            <mode>:   command mode
*/
GLOBAL T_ACI_RETURN sAT_PlusCMMS ( T_ACI_CMD_SRC srcId,
                                   UBYTE         mode )
{
  T_ACI_RETURN ret = AT_FAIL;

  TRACE_FUNCTION ("sAT_PlusCMMS ()");

  /*
   *-----------------------------------------------------------------
   * check source id and if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
    return ret;

  smsShrdPrm.CMMSmode = mode;
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_CMMS;

  /* Implements measure 147,148, 149 */  
  cmhSMS_sendConfigureReq(TRUE);
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;

  return( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PercentCMGMDU            |
+-------------------------------------------------------------------+

  PURPOSE : Function is used to reset the reply path

            <index>:   Index of the record
*/

#if (defined (MFW) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS
GLOBAL T_ACI_RETURN sAT_PercentCMGMDU ( T_ACI_CMD_SRC   srcId,
                                        UBYTE           index )
{
  T_CONC_INIT_RETURN ret;

  TRACE_FUNCTION ("sAT_PercentCMGMDU ()");

  TRACE_EVENT_P1("sAT_PercentCMGMDU () index: %d", index);

  ret=concSMS_initReadFromMem(srcId, index, SMS_READ_Normal);

  if (ret EQ CONC_NEEDED)
  {
    SET_CONC;
    TRACE_EVENT("sAT_PercentCMGMDU: CONC_NEEDED");
    return sAT_PercentCMGMDU_Gl(srcId, index, rConcSMS_PercentCMGMDU);
  }
  else if (ret EQ CONC_NOT_NEEDED)
  {
    TRACE_EVENT("sAT_PercentCMGMDU: CONC_NOT_NEEDED");
    return sAT_PercentCMGMDU_Gl(srcId, index, NULL);
  }
  else
  {
    TRACE_EVENT("ERROR: sAT_PercentCMGMDU: CMS_ERR_InValMemIdx");
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_InValMemIdx );
    return ( AT_FAIL );
  }
}
#endif /* #if (defined (MFW) OR defined (_CONC_TESTING_) */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PercentCMGMDU            |
+-------------------------------------------------------------------+

  PURPOSE : For Riv-MMI.
*/
#if defined (FF_MMI_RIV) OR !defined TI_PS_FF_CONC_SMS
GLOBAL T_ACI_RETURN sAT_PercentCMGMDU ( T_ACI_CMD_SRC   srcId,
                                        UBYTE           index )
{
  TRACE_FUNCTION ("sAT_PercentCMGMDU ()");
  TRACE_EVENT_P1("sAT_PercentCMGMDU () index: %d", index);
  
  return sAT_PercentCMGMDU_Gl(srcId, index, NULL);
}
#endif /*#if defined (FF_MMI_RIV)*/

GLOBAL T_ACI_RETURN sAT_PercentCMGMDU_Gl      ( T_ACI_CMD_SRC   srcId,
                                                UBYTE           index,
                                                T_CMGMDU_FCT    rplyCB )
{
  T_ACI_RETURN ret = AT_EXCT;

  TRACE_FUNCTION ("sAT_PercentCMGMDU_Gl ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
  {
    return ret;
  }

  smsShrdPrm.rplyCB.cmgmdu = rplyCB;
  smsShrdPrm.rdMode = READ_NORMAL;

  /*
   *-----------------------------------------------------------------
   * read a short message from memory
   *-----------------------------------------------------------------
   */
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_P_CMGMDU;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;

  psaSMS_ReadReq ( smsShrdPrm.mem1, index,
                   smsShrdPrm.rdMode, SMS_STAT_NotPresent);

  return( ret );
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PercentCMGR_Gl           |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the %CMGR
            AT command which is responsible for reading a short
            message from memory.

            <index>:   storage area index
            <rdMode>:  read mode
            <rplyCB>:  reply call-back
            <errorCB>: error call-back
*/
GLOBAL T_ACI_RETURN sAT_PercentCMGR_Gl ( T_ACI_CMD_SRC  srcId,
                                         UBYTE          index,
                                         T_ACI_SMS_READ rdMode,
                                         T_CMGR_FCT     rplyCB )
{
  T_ACI_RETURN ret = AT_EXCT;

  TRACE_FUNCTION ("sAT_PercentCMGR_Gl ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
  {
    return ret;
  }

  smsShrdPrm.rplyCB.cmgr = rplyCB;


  /*
   *-----------------------------------------------------------------
   * process the <index> parameter
   *-----------------------------------------------------------------
   */
  switch (rdMode)
  {
  case SMS_READ_StatusChange:
    smsShrdPrm.rdMode = READ_STATUS_CHANGE;
    break;
  case SMS_READ_Preview:
    smsShrdPrm.rdMode = READ_PREVIEW;
    break;
  case SMS_READ_Normal:
  case SMS_READ_NotPresent:
    smsShrdPrm.rdMode = READ_NORMAL;
    break;
  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return ( AT_FAIL );
  }
  /*
   *-----------------------------------------------------------------
   * read a short message from memory
   *-----------------------------------------------------------------
   */
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_P_CMGR;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;

  psaSMS_ReadReq ( smsShrdPrm.mem1, index,
                   smsShrdPrm.rdMode, SMS_STAT_NotPresent);

  return( ret );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PercentCMGR              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the %CMGR
            AT command which is responsible for reading a short
            message from memory.

            Features Concatenated SMS.
            For GPF-MMI.

            <index>:  storage area index
            <rdMode>: read mode
*/
#if (defined (MFW) OR defined (SMI) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS

GLOBAL T_ACI_RETURN sAT_PercentCMGR ( T_ACI_CMD_SRC  srcId,
                                      UBYTE          index,
                                      T_ACI_SMS_READ rdMode )
{
  T_CONC_INIT_RETURN ret;

  TRACE_FUNCTION ("sAT_PercentCMGR ()");

  ret=concSMS_initReadFromMem(srcId, index, rdMode);

  if (ret EQ CONC_NEEDED)
  {
    SET_CONC;
    TRACE_EVENT("sAT_PlusCMGR: CONC_NEEDED");
    return sAT_PercentCMGR_Gl(srcId, index, rdMode, rConcSMS_PercentCMGR);
  }
  else if (ret EQ CONC_NOT_NEEDED)
  {
    TRACE_EVENT("sAT_PlusCMGR: CONC_NOT_NEEDED");
    return sAT_PercentCMGR_Gl(srcId, index, rdMode, rAT_PercentCMGR);
  }
  else
  {
    TRACE_EVENT("ERROR: sAT_PercentCMGR: CMS_ERR_InValMemIdx");
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_InValMemIdx );
    return ( AT_FAIL );
  }
}
#endif /*#if defined (MFW) OR defined (SMI) OR defined (_CONC_TESTING_)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PercentCMGR                 |
+-------------------------------------------------------------------+

  PURPOSE :  %CMGR command for handling the conc module on 
            the Riv-MMI side.
*/
#if defined (FF_MMI_RIV) /* OR !defined TI_PS_FF_CONC_SMS */
GLOBAL T_ACI_RETURN sAT_PercentCMGR ( T_ACI_CMD_SRC  srcId,
                                      UBYTE          index,
                                      T_ACI_SMS_READ rdMode )
{
  TRACE_FUNCTION ("sAT_PercentCMGR ()");

  TRACE_EVENT_P1("sAT_PercentCMGR () index: %d", index);

   return sAT_PercentCMGR_Gl(srcId, index, rdMode, rAT_PercentCMGR);
}
#endif /*#if defined (FF_MMI_RIV)*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PercentCMGL              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the %CMGL
            AT command which is responsible for creating a list of
            all stored short messages.

            <state>: state of stored short message
            <rdmode>: Read mode
*/
GLOBAL T_ACI_RETURN sAT_PercentCMGL ( T_ACI_CMD_SRC  srcId,
                                      T_ACI_SMS_STAT state,
                                      T_ACI_SMS_READ rdMode )
{
  T_ACI_RETURN   ret;          /* AT response code            */
  T_ACI_SMS_STAT chk_state;

  TRACE_FUNCTION ("sAT_PercentCMGL ()");

  /*
   *-----------------------------------------------------------------
   * check if command executable
   *-----------------------------------------------------------------
   */
  if(!cmhSMS_checkAccess (srcId, &ret))
  {
    return ret;
  }

  /*
   *-----------------------------------------------------------------
   * process the <state> parameter
   *-----------------------------------------------------------------
   */
  switch ( state )
  {
    case( SMS_STAT_NotPresent ):
      chk_state = SMS_STAT_RecUnread;
      break;

    case( SMS_STAT_Invalid    ):
    case( SMS_STAT_All        ):
    case( SMS_STAT_RecUnread  ):
    case( SMS_STAT_RecRead    ):
    case( SMS_STAT_StoUnsent  ):
    case( SMS_STAT_StoSent    ):
      chk_state = state;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return ( AT_FAIL );
  }


  /*
   *-----------------------------------------------------------------
   * process the <rdMode> parameter
   *-----------------------------------------------------------------
   */
  switch (rdMode)
  {
  case SMS_READ_StatusChange:
    smsShrdPrm.rdMode = READ_STATUS_CHANGE;
    break;
  case SMS_READ_Preview:
    smsShrdPrm.rdMode = READ_PREVIEW;
    break;
  case SMS_READ_Normal:
  case SMS_READ_NotPresent:
    smsShrdPrm.rdMode = READ_NORMAL;
    break;
  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return ( AT_FAIL );
  }

  /*
   *---------------------------------------------------------------
   * PSA notification
   *---------------------------------------------------------------
   */
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_P_CMGL;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;

  /*
   *---------------------------------------------------------------
   * request the list element
   *---------------------------------------------------------------
   */
  psaSMS_ReadReq ( smsShrdPrm.mem1, 0,
                   smsShrdPrm.rdMode, chk_state );

  return( AT_EXCT );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSS                |
| STATE   : code                  ROUTINE : cmhSMS_SendDelete_Req   |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional used to send a delete request
            to SMS module.

*/

GLOBAL void cmhSMS_SendDelete_Req ( UBYTE         index,
                                    UBYTE         status )
{
  TRACE_FUNCTION ("cmhSMS_SendDelete_Req ()");
 
  smsShrdPrm.index  = index;
  smsShrdPrm.status = status;

  /*-------------------------------------------------------------------*
   *             Create and Send primitive for SMS delete              *
   *-------------------------------------------------------------------*/
  {
    PALLOC (mnsms_delete_req, MNSMS_DELETE_REQ);

    /* fill in primitive parameter: delete message */
    mnsms_delete_req -> mem_type = smsShrdPrm.mem1;
    mnsms_delete_req -> rec_num  = index;
    mnsms_delete_req -> delete_status   = status;

    PSENDX (SMS, mnsms_delete_req);

    TRACE_EVENT_P1("cmgd: delete rec_num: %d", mnsms_delete_req->rec_num);
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PercentCMGRS                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the %CMGRS
            AT command which is responsible for selecting auto 
            retransmission mode/request manual retransmission of last 
            failed SMS

            <mode>: requested mode
*/
#ifdef REL99

GLOBAL T_ACI_RETURN sAT_PercentCMGRS ( T_ACI_CMD_SRC     srcId,
                                       T_ACI_CMGRS_MODE  mode)
{  
  T_ACI_RETURN  aci_ret_val = AT_CMPL;

  TRACE_FUNCTION ("sAT_PercentCMGRS ()");
  
  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */
  if(!cmh_IsVldCmdSrc (srcId) AND (srcId NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT))
  {
    TRACE_ERROR ("[cmhSMS_checkAccess]: Cmd Src not valid");    
    return AT_FAIL;
  }
  /*
   *-----------------------------------------------------------------
   * check SMS access status
   *-----------------------------------------------------------------
   */
  if (!cmhSMS_checkSIM ())
  {   
    return AT_FAIL;
  }
    
  switch( mode )
  {
    case( CMGRS_MODE_DISABLE_AUTO_RETRANS ):
    case( CMGRS_MODE_ENABLE_AUTO_RETRANS ):
      smsShrdPrm.auto_repeat_flag = mode;
      aci_ret_val = AT_CMPL;
      break;

    case( CMGRS_MODE_MANUAL_RETRANS ):  
      /*
       *-----------------------------------------------------------------
       * check entity status
       *-----------------------------------------------------------------
       */
      if( smsShrdPrm.smsEntStat.curCmd NEQ AT_CMD_NONE )
      {

        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        TRACE_ERROR ("[cmhSMS_checkAccess]: Entity is busy");
        aci_ret_val = AT_BUSY;
        break ;
      }     
      
      if(smsShrdPrm.is_msg_present_for_retrans EQ FALSE)
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_FailedMsgNotPresent );
        aci_ret_val = AT_FAIL;       
      }
      else
      {
        /*-------------------------------------------------------------------
         * create and send primitive for retrasnmission request
         *-------------------------------------------------------------------
         */
        PALLOC (mnsms_retrans_req, MNSMS_RETRANS_REQ);
        mnsms_retrans_req -> auto_rep_flag = smsShrdPrm.auto_repeat_flag;      
        PSENDX (SMS, mnsms_retrans_req);

        smsShrdPrm.smsEntStat.curCmd = AT_CMD_P_CMGRS;
        smsShrdPrm.smsEntStat.entOwn = srcId;            
        smsShrdPrm.owner = (T_OWN)srcId;
#if defined (MFW) OR defined (FF_MMI_RIV)
        smsShrdPrm.rplyCB.cmgrs = rAT_PercentCMGRS;
        smsShrdPrm.errorCB      = rAT_PlusCMS;
#else
        smsShrdPrm.rplyCB.cmgrs = NULL;
        smsShrdPrm.errorCB      = NULL;
#endif
        aci_ret_val = AT_EXCT;      
      }
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      aci_ret_val = AT_FAIL;
      break;      
  }

  return aci_ret_val;
}
#endif

/*==== EOF ========================================================*/
