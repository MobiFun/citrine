/*
+------------------------------------------------------------------------------
|  File:       cmh_dti.c
+------------------------------------------------------------------------------
|                 Copyright Condat AG 1999-2000, Berlin
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Condat AG.
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Condat AG.
+------------------------------------------------------------------------------
| Purpose:     This module provides the set functions related to the
|                   DTI managment.
| $Identity:$
+------------------------------------------------------------------------------
*/

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef DTI

#ifndef CMH_DTI_C
#define CMH_DTI_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#ifdef GPRS
#include "gaci_cmh.h"
#endif
#include "ati_cmd.h"
#include "psa.h"
#include "cmh.h"
#include "cmh_dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
 * for the device names see "AT commands for Packet IO.doc"
 * a device name has (almost) nothing to do with entity names defined in custom.h !
 */
LOCAL T_DTI_ENTITY_ID map_name_2_id (char *dev_name)
{
  if ((strcmp (dev_name,"UART")) EQ 0)
  {
    return (DTI_ENTITY_UART); 
  }
  else if ((strcmp (dev_name,"RIV")) EQ 0)
  {
    return (DTI_ENTITY_AAA); 
  }
  else if ((strcmp (dev_name,"PSI")) EQ 0)
  {
    return (DTI_ENTITY_PSI); 
  }
#ifdef GPRS
  else if ((strcmp (dev_name,"PKTIO")) EQ 0)
  {
    return (DTI_ENTITY_PKTIO); 
  }
#endif
  else
  {
    TRACE_EVENT_P1 ("sAT_PercentDATA(): non valid device name %s", dev_name);
    return (DTI_ENTITY_INVALID);
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)            MODULE  : CMH_DTI                |
| STATE   : finished               ROUTINE : sAT_PercentDATA        |
+-------------------------------------------------------------------+

  PURPOSE : set data flow, means to split the data channel (destination)
            from the AT cmd channel (source)
*/
GLOBAL T_ACI_RETURN sAT_PercentDATA (T_ACI_CMD_SRC  srcId,
                                     UBYTE          redir_mode,  /* delete,once,permanent */
                                     CHAR          *des_devname,
                                     UBYTE          des_devno,
                                     UBYTE          des_subno,
                                     CHAR          *dev_cap,
                                     CHAR          *src_devname,
                                     UBYTE          src_devno,
                                     UBYTE          src_subno,
                                     UBYTE          pdp_cid)
{
  const CHAR         *del        = ",";
        CHAR         *s          = NULL;
        UBYTE         capability = 0x00; /* DTI_CPBLTY_NO */
        BOOL          rv         = FALSE;
  T_ACI_REDIR_MODE    mode       = (T_ACI_REDIR_MODE)redir_mode;
  T_DTI_ENTITY_ID     des_dev_id = DTI_ENTITY_INVALID;
  T_DTI_ENTITY_ID     src_dev_id = DTI_ENTITY_INVALID;

/* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
  extern CHAR gob_tst_buf[];
  sprintf(gob_tst_buf+strlen(gob_tst_buf), "spd ");
#endif

  TRACE_FUNCTION ("sAT_PercentDATA ()");

  for (s = strtok(dev_cap, del); s; s = strtok(NULL, del))
  {
    if ((strcmp (s,"CMD")) EQ 0)
    {
      capability = capability | 0x01; /* DTI_CPBLTY_CMD */
    }
    else if ((strcmp (s,"PKT")) EQ 0)
    {
      capability = capability | 0x02; /* DTI_CPBLTY_PKT */
    }
    else if ((strcmp (s,"SER")) EQ 0)
    {
      capability = capability | 0x04; /* DTI_CPBLTY_SER */
    }
    else
    {
      TRACE_EVENT_P1 ("sAT_PercentDATA(): non valid capability %s",s);
      return (AT_FAIL);
    }
  }

#ifdef GPRS
  if (pdp_cid > PDP_CONTEXT_CID_MAX )
  {
     TRACE_EVENT_P1 ("sAT_PercentDATA(): non valid cid %d", pdp_cid);
     return (AT_FAIL);
  }
#else
  if (pdp_cid > 0)
  {
     TRACE_EVENT_P1 ("sAT_PercentDATA(): for GSM cid must be 0, but is %d", pdp_cid);
     return (AT_FAIL);
  }
#endif

  if (mode > REDIR_ALWAYS)
  {
     TRACE_EVENT_P1 ("sAT_PercentDATA(): non valid redirection mode %d", mode);
     return (AT_FAIL);
  }

  des_dev_id = map_name_2_id (des_devname);

  if ((des_dev_id NEQ DTI_ENTITY_UART)
  AND (des_dev_id NEQ DTI_ENTITY_PSI)
#ifdef GPRS
  AND (des_dev_id NEQ DTI_ENTITY_PKTIO)
#endif
  AND (des_dev_id NEQ DTI_ENTITY_AAA)) 
  {
    TRACE_EVENT_P1 ("sAT_PercentDATA(): non valid destination device name %s",des_devname);
    return (AT_FAIL);
  }

  if (strlen (src_devname))
  {
    /*
     * special case, where three different devices are involved
     * a device with the srcId (has an AT cmd channel) 
     * wants to redirect a second device (src_dev) to a third device (des_dev) at data transmission
     */
    src_dev_id = map_name_2_id (src_devname);

    if ((src_dev_id NEQ DTI_ENTITY_INVALID) 
    AND (src_dev_id EQ des_dev_id)
    AND (src_devno  EQ des_devno)
    AND (src_subno  EQ des_subno)) 
    {
      TRACE_EVENT_P1 ("sAT_PercentDATA(): non valid source device name %s",src_devname);
      return (AT_FAIL);
    }

    rv = dti_cntrl_set_redirect_from_device ((UBYTE)mode,
                                             des_dev_id,
                                             des_devno,
                                             des_subno,
                                             src_dev_id,
                                             src_devno,
                                             src_subno,
                                             capability,
                                             pdp_cid);
  }
  else
  {
    /*
     * device with the srcId (AT cmd channel) itself wants to be redirected for data transmission
     */
    rv = dti_cntrl_set_redirect_from_src ((UBYTE)srcId, 
                                          (UBYTE)mode,
                                          des_dev_id,
                                          des_devno,
                                          des_subno,
                                          capability,
                                          pdp_cid);
  }


  if (rv EQ TRUE)
  {
    return (AT_CMPL);
  }
  else
  {
    return (AT_FAIL);
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_DTI                 |
| STATE   : finished              ROUTINE : dinf_assign             |
+-------------------------------------------------------------------+

  PURPOSE : help function for qAT_PercentDATA

*/
LOCAL void percentDATA_assign (UBYTE* mode, 
                               UBYTE  *cid,
                               T_DINF_PARAM  *des_param, 
                               T_DTI_CNTRL   *tmp_param)

{
  des_param->dev_id     = tmp_param->dev_id;
  des_param->dev_no     = tmp_param->dev_no;
  des_param->sub_no     = tmp_param->sub_no;
  des_param->capability = tmp_param->redirect_info.info.capability;
  des_param->driver_id  = tmp_param->driver_id;
  des_param->dio_ctrl_id = tmp_param->dio_ctrl_id;
  
  *cid                  = tmp_param->redirect_info.info.cid;       
  *mode                 = tmp_param->redirect_info.info.mode;     
}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)            MODULE  : CMH_DTI                |
| STATE   : finished               ROUTINE : qAT_PercentDATA        |
+-------------------------------------------------------------------+

  PURPOSE : query data flow

*/
GLOBAL T_ACI_RETURN qAT_PercentDATA (T_ACI_CMD_SRC  srcId, 
                                     UBYTE         *mode, 
                                     UBYTE         *cid,
                                     T_DINF_PARAM  *des_param, 
                                     T_DINF_PARAM  *src_param)
{
  static BOOL first_query = TRUE;

  T_DTI_CNTRL tmp_param; /* the DTI Controller will put all neccessarly pieces of information in this structure */

  TRACE_FUNCTION ("qAT_PercentDATA ()");

  if (dti_cntrl_get_info_from_src_id ((UBYTE)srcId, &tmp_param))
  {
    /*
     * at first save the parameter of the source, because we use tmp_param for the destination as well
     */
    src_param->dev_id = tmp_param.dev_id;
    src_param->dev_no = tmp_param.dev_no;
    src_param->sub_no = tmp_param.sub_no;

    src_param-> driver_id = tmp_param. driver_id;
    src_param-> dio_ctrl_id = tmp_param. dio_ctrl_id;

    if (first_query)
    {
      if (tmp_param.redirect_info.info.direction  EQ DTI_DIRECTION_NOTPRESENT)
      {
        first_query = TRUE;                   /* reset for later new query                    */
        des_param->dev_id = NOT_PRESENT_8BIT; /* to indicate no redirection at all            */
        return (AT_FAIL);                     /* no error, but there is no redirection at all */
      }
      first_query = FALSE;
      /*
       * the very first query, start with redirection we got from dti_cntrl_get_info_from_src_id()
       */
      if (dti_cntrl_get_first_redirection ((UBYTE)srcId, tmp_param.redirect_info.info.capability, &tmp_param))
      {
        percentDATA_assign (mode, cid, des_param, &tmp_param);
        return (AT_CMPL);
      }
    }
    else
    {
      if (dti_cntrl_get_next_redirection  ((UBYTE)srcId, *cid, src_param->capability, &tmp_param))
      {
        percentDATA_assign (mode, cid, des_param, &tmp_param);
        return (AT_CMPL);
      }
      else
      {
        first_query = TRUE;                      /* reset for later new query */
        des_param->capability = DEV_CPBLTY_NONE; /* sentinel                  */
        return (AT_FAIL);    /* no error, but there is no further redirection */
      }
    }
  }
  else  
  {
    TRACE_EVENT_P1("[ERR] sAT_PercentDINF(): no info to srcId %d !", srcId);
    ACI_ERR_DESC (ACI_ERR_CLASS_Ext, EXT_ERR_Parameter);
    return (AT_FAIL);
  }
  TRACE_EVENT_P1("[ERR] sAT_PercentDINF(): Unknown", srcId);
  ACI_ERR_DESC (ACI_ERR_CLASS_Ext, EXT_ERR_Unknown);
  return (AT_FAIL);
  
}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_DTI                 |
| STATE   : finished              ROUTINE : dinf_assign             |
+-------------------------------------------------------------------+

  PURPOSE : help function for sAT_PercentDINF

*/
LOCAL void dinf_assign (T_DINF_PARAM  *device_param, T_DTI_CNTRL *tmp_param)
{
  device_param->dev_id     = tmp_param->dev_id;
  device_param->dev_no     = tmp_param->dev_no;
  device_param->sub_no     = tmp_param->sub_no;
  device_param->capability = tmp_param->capability;
  device_param->cur_cap    = tmp_param->cur_cap;
  device_param->src_id     = tmp_param->src_id;
  device_param->driver_id  = tmp_param->driver_id;
  device_param->dio_ctrl_id = tmp_param->dio_ctrl_id;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_DTI                 |
| STATE   : finished              ROUTINE : sAT_PercentDINF         |
+-------------------------------------------------------------------+

  PURPOSE : list all available data and ATI channels and their capabilities

*/

GLOBAL T_ACI_RETURN sAT_PercentDINF( T_ACI_CMD_SRC  srcId, /* UBYTE src_id passed by setatPercentDINF ! */
                                     UBYTE           mode,
                                     T_DINF_PARAM  *device_param)
{
  static BOOL first_device = TRUE;
  T_DTI_CNTRL tmp_param;

  TRACE_FUNCTION ("sAT_PercentDINF()");
  
  switch (mode)
  {
    case (SHOW_CURR_CHAN_INF):
    {     
      if (dti_cntrl_get_info_from_src_id ((UBYTE)srcId, &tmp_param))
      {
        dinf_assign (device_param, &tmp_param);
        return (AT_CMPL);
      }
      else  
      {
        TRACE_EVENT_P1("[ERR] sAT_PercentDINF(): no info to srcId %d !", srcId);
        break;
      }
    } 
    case (SHOW_ALL_CHAN_INF):
    {
      if (first_device)
      {
        first_device = FALSE;

        if (dti_cntrl_get_first_device (&tmp_param))
        {
          dinf_assign (device_param, &tmp_param);
          return (AT_CMPL);
        }
        else
        {
          TRACE_EVENT("[ERR] sAT_PercentDINF(): there is no device listed at all, so watch dti_cntrl_list !");
          break; 
        }
      }
      else
      {
        if (dti_cntrl_get_next_device (&tmp_param))
        {
          dinf_assign (device_param, &tmp_param);
          return (AT_CMPL); 
        }
        else
        {
          first_device = TRUE; /* important to reset for a later new AT%DINF */

          device_param->capability = DEV_CPBLTY_NONE; /* this is the sentinel, that there are no further devices */
          return (AT_FAIL); /* it is not an error, but the end of AT%DINF */
        }
      }
    }
    default:
    {
      TRACE_EVENT("[ERR] sAT_PercentDINF(): unknown mode");
      break;
    }
  } 

  ACI_ERR_DESC (ACI_ERR_CLASS_Ext, EXT_ERR_Parameter);
  return (AT_FAIL);  
}



#endif /* DTI */

/*==== EOF ========================================================*/
