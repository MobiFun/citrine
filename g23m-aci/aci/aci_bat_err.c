/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_BAT
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
|  Purpose :  Handle cme and cms errors for BAT clients
+-----------------------------------------------------------------------------
*/
#include "aci_all.h"

#include "aci_cmh.h"  /* so see T_ACI_ERR_CLASS and other */
#include "psa.h"
#include "cmh.h"      /* extern definition of aciErrDesc */

#include "aci_bat_cmh.h"
#include "aci_bat.h"
#include "aci_bat_err.h"

#include "psa_psi.h"


/*
 * the [stq]AT_xxx function will set the global variable aciErrDesc with ACI_ERR_DESC.
 * here we read it and cast it to the BAT value defined in
 * T_BAT_plus_cme_error_error and T_BAT_plus_cms_error_err.
 * Currently (03.March 2005) the error values of ACI and BAT are identical,
 * but can cause a lot of problems when there is a mismatch!
 * There should be used only the values of p_bat.h defined in bat.sap !
 *
 * on ACI level there are CME, CMS, CEER and EXT error values.
 * CEER errors are/is only one in
 *      cmh_ccf.c:2159:    ACI_ERR_DESC( ACI_ERR_CLASS_Ceer, CAUSE_MAKE(DEFBY_STD,
 * and must be requested with the
 * T_BAT_cmd_send send.ctrl_params = BAT_CMD_SET_PLUS_CEER by the apllication.
 * BAT returns with a T_BAT_res_set_plus_ceer structure.
 *
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_err          |
+--------------------------------------------------------------------+

  PURPOSE : central BAT error function called by aci_bat_wrapper only.
            it evaluates the global varaible aciErrDesc, which has been set
            with the appropriate error value of the just ran ACI function.
*/
GLOBAL void aci_bat_err (T_ACI_DTI_PRC_PSI *src_infos_psi)
{
  T_ACI_ERR_CLASS err_class;
  T_BAT_res_plus_cme_error cme_data;
  T_BAT_res_plus_cms_error cms_data;
  T_BAT_res_plus_ext_error ext_data;
  T_BAT_cmd_response resp;

  if (ACI_BAT_VALID_ERR_DESC())
  {
    err_class = (T_ACI_ERR_CLASS)ACI_BAT_GET_ERR_CLASS();
    switch (err_class)
    {
      case (ACI_ERR_CLASS_Cme):
      {
        T_BAT_plus_cme_error_error bat_err;
        T_ACI_CME_ERR err = (T_ACI_CME_ERR)ACI_BAT_GET_ERR_VALUE();
        ACI_BAT_RESET_ERR_DESC();
        bat_err = (T_BAT_plus_cme_error_error)err;
        resp.ctrl_response = BAT_RES_PLUS_CME_ERROR;
        resp.response.ptr_plus_cme_error=&cme_data;
        resp.response.ptr_plus_cme_error->error = bat_err;
        TRACE_EVENT_P1("aci_bat_err(): CME ERROR: %d", bat_err);
        break;
      }
      case (ACI_ERR_CLASS_Cms):
      {
        T_BAT_plus_cms_error_err bat_err;
        T_ACI_CMS_ERR err = (T_ACI_CMS_ERR)ACI_BAT_GET_ERR_VALUE();
        ACI_BAT_RESET_ERR_DESC();
        bat_err = (T_BAT_plus_cms_error_err)err;
        resp.ctrl_response = BAT_RES_PLUS_CMS_ERROR;
        resp.response.ptr_plus_cms_error=&cms_data;
        resp.response.ptr_plus_cms_error->err = bat_err;
        TRACE_EVENT_P1("aci_bat_err(): CMS ERROR: %d", bat_err);
        break;
      }
      case (ACI_ERR_CLASS_Ext):
      {
        T_BAT_plus_ext_error_err bat_err;
        T_ACI_EXT_ERR err = (T_ACI_EXT_ERR)ACI_BAT_GET_ERR_VALUE();
        ACI_BAT_RESET_ERR_DESC();
        bat_err = (T_BAT_plus_ext_error_err)err;
        resp.ctrl_response = BAT_RES_PLUS_EXT_ERROR;
        resp.response.ptr_plus_ext_error=&ext_data;
        resp.response.ptr_plus_ext_error->err = bat_err;
        TRACE_EVENT_P1("aci_bat_err(): EXT ERROR: %d", bat_err);
        break;
      }
      case (ACI_ERR_CLASS_NotPresent):
      default:
        /* this should trigger to request the CEER errors by the application */
        resp.ctrl_response = BAT_RES_PLUS_CME_ERROR;
        resp.response.ptr_plus_cms_error=&cms_data;
        resp.response.ptr_plus_cms_error->err=(T_BAT_plus_cms_error_err)BAT_CME_ERROR_UNKNOWN;
        TRACE_EVENT("aci_bat_err(): UNKNOWN ERROR");
        break;
    }
  }
  else
  {
    TRACE_EVENT("aci_bat_err(): WARNING, no error value set!");

    resp.ctrl_response = BAT_RES_PLUS_CME_ERROR;
    resp.response.ptr_plus_cme_error=&cme_data;
    resp.response.ptr_plus_cme_error->error=BAT_CME_ERROR_UNKNOWN;
  }

  aci_bat_send(src_infos_psi,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCEER        |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCEER        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  USHORT cause;

  TRACE_FUNCTION ("sBAT_PlusCEER()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PlusCEER((T_ACI_CMD_SRC)src_infos_psi->srcId,&cause);

  /*
  *   If the query completes - and it should - send the response now.
  */
  if (ret==ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_set_plus_ceer ceer_data;

    resp.ctrl_response=BAT_RES_SET_PLUS_CEER;
    resp.response.ptr_set_plus_ceer=&ceer_data;

    ceer_data.definedby=(T_BAT_plus_ceer_definedby)GET_CAUSE_DEFBY(cause);
    ceer_data.originatingside=(T_BAT_plus_ceer_originatingside)GET_CAUSE_ORIGSIDE(cause);
    ceer_data.originatingentity=(T_BAT_plus_ceer_originatingentity)GET_CAUSE_ORIGIN_ENTITY(cause);
    ceer_data.causevalue=(U8)GET_CAUSE_VALUE(cause);

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}
