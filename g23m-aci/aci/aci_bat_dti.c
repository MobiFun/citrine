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
|  Purpose :  This BAT wrapper modul is ...
| 
+----------------------------------------------------------------------------- 
*/ 
 
#include "aci_all.h"     /* includes prim.h, which includes p_bat.h */ 
#include "aci_cmh.h"     /* prototypes of sAT_,qAT_,tAT_    */ 
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */
#include "aci_bat_cmh.h" /* prototypes of sBAT_,qBAT_,tBAT_ */ 
#include "aci_bat.h"
#include "cmh_dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#include "aci_bat_cmh.h" /* prototypes of sBAT_,qBAT_,tBAT_ */ 
#include "aci_bat_err.h" /*prototypes of err functions for BAT*/

#define DIO_DEVNO_MASK 0xFF


/*==== VARIABLE====================================================*/

static const T_CAP_NAME aci_bat_cap_name[] =
{
  {"CMD",         DEV_CPBLTY_CMD        },
  {"SER",         DEV_CPBLTY_SER        },
  {"PKT",         DEV_CPBLTY_PKT        },
  {"CMD,SER",     DEV_CPBLTY_CMD_SER    },
  {"CMD,PKT",     DEV_CPBLTY_CMD_PKT    },
  {"PKT,SER",     DEV_CPBLTY_PKT_SER    },
  {"CMD,PKT,SER", DEV_CPBLTY_CMD_PKT_SER},
  {"",            DEV_CPBLTY_NONE}
};

/*==============================================================*/


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)                MODULE  :                  |
| STATE   : code                          ROUTINE :  map_dti_cpblty  |
+--------------------------------------------------------------------+

  PURPOSE : help function to map the capability values of 
            DTI_CPBLTY_xxx to enum value of T_CAP_ID.
            With the enum value we get the capability string in ati_cap_name[]
            For DTI_CPBLTY_xxx see SAP document AAA.doc chapter Capability
            Dor T_CAP_ID see cmh_dti.h
*/
LOCAL T_CAP_ID aci_bat_map_dti_cpblty (UBYTE dti_cpblty)
{
  switch (dti_cpblty)
  {
    case (DTI_CPBLTY_NO):
      return (DEV_CPBLTY_NONE);

    case (DTI_CPBLTY_CMD):
      return (DEV_CPBLTY_CMD);

    case (DTI_CPBLTY_SER):
      return (DEV_CPBLTY_SER);

    case (DTI_CPBLTY_PKT):
      return (DEV_CPBLTY_PKT);

    case (DTI_CPBLTY_CMD + DTI_CPBLTY_SER):
      return (DEV_CPBLTY_CMD_SER);

    case (DTI_CPBLTY_CMD + DTI_CPBLTY_PKT):
      return (DEV_CPBLTY_CMD_PKT);

    case (DTI_CPBLTY_PKT + DTI_CPBLTY_SER):
      return (DEV_CPBLTY_PKT_SER);

    case (DTI_CPBLTY_CMD + DTI_CPBLTY_PKT + DTI_CPBLTY_SER):
      return (DEV_CPBLTY_CMD_PKT_SER);

    default:
    {
      TRACE_EVENT_P1("aci_bat_map_dti_cpblty():[ERR] unknown capability value = %d",dti_cpblty);
      return (DEV_CPBLTY_NONE);
    }
  }
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentDATA     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentDATA     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  CHAR          des_dev_name[MAX_SMS_ADDR_DIG];  /* destination device name       */
  CHAR          src_dev_name[MAX_SMS_ADDR_DIG];  /* source device name            */
  CHAR          dev_cap[MAX_SMS_ADDR_DIG];       /* device capability             */
  CHAR*         p_des_dev_name;
  CHAR*         p_src_dev_name;
  CHAR*         p_dev_cap;
  UBYTE         red_mode   ;                     /* redirection mode              */
  UBYTE         cid        = (UBYTE)BAT_PARAMETER_NOT_PRESENT;   /* context cid                   */
  U32           des_devId;                              /* destination device identifier     */
  SHORT         des_sub_no = (SHORT)BAT_PARAMETER_NOT_PRESENT;   /* destination sub number        */
  U32           src_devId ;                              /* source device identifier        */
  SHORT         src_sub_no = (SHORT)BAT_PARAMETER_NOT_PRESENT; /* source sub number             */
  
  TRACE_FUNCTION ("sBAT_PercentDATA()");
  
  red_mode = cmd->params.ptr_set_percent_data->mode;
  des_devId  = cmd->params.ptr_set_percent_data->des_devId;
  des_sub_no = cmd->params.ptr_set_percent_data->des_sub_no;
  src_devId  = cmd->params.ptr_set_percent_data->src_devId;
  src_sub_no = cmd->params.ptr_set_percent_data->src_sub_no;
  cid        = cmd->params.ptr_set_percent_data->cid;
 
  src_dev_name[0]='\0';
  des_dev_name[0]='\0';
  dev_cap[0]='\0';
  p_des_dev_name = des_dev_name;
  p_src_dev_name = src_dev_name;
  p_dev_cap = dev_cap;
  
  switch(cmd->params.ptr_set_percent_data->des_dev_name)
  {
    case (BAT_DEV_NAME_UART):
    {
      p_des_dev_name = "UART";
      break;
    }
    case (BAT_DEV_NAME_RIV):
    {
      p_des_dev_name = "RIV";
      break;
    }
    case (BAT_DEV_NAME_PKTIO):
    {
      p_des_dev_name = "PKTIO";
      break;
    }
    case (BAT_DEV_NAME_PSI):
    {
      p_des_dev_name = "PSI";
      break;
    }  
  }
  
  switch(cmd->params.ptr_set_percent_data->src_dev_name)
  {
    case (BAT_DEV_NAME_UART):
    {
      p_src_dev_name = "UART";
      break;
    }
    case (BAT_DEV_NAME_RIV):
    {
      p_src_dev_name = "RIV";
      break;
    }
    case (BAT_DEV_NAME_PKTIO):
    {
      p_src_dev_name = "PKTIO";
      break;
    }
    case (BAT_DEV_NAME_PSI):
    {
      p_src_dev_name = "PSI";
      break;
    }  
  }

  switch(cmd->params.ptr_set_percent_data->capability)
  {
    case(BAT_CAP_CMD):
    {
      p_dev_cap = "CMD";
      break;
    }
    case(BAT_CAP_SER):
    {
      p_dev_cap = "SER";
      break;
    }
    case(BAT_CAP_CMD + BAT_CAP_SER ):
    {
      p_dev_cap = "CMD,SER";
      break;
    }
    case(BAT_CAP_PKT):
    {
      p_dev_cap = "PKT";
      break;
    }
    case(BAT_CAP_CMD + BAT_CAP_PKT):
    {
      p_dev_cap = "CMD_PKT";
      break;
    }
    case(BAT_CAP_SER + BAT_CAP_PKT):
    {
      p_dev_cap = "SER_PKT";
      break;
    }
    case(BAT_CAP_CMD + BAT_CAP_SER + BAT_CAP_PKT):
    {
      p_dev_cap = "CMD_SER_PKT";
      break;
    }
  }
  ret = (T_ACI_BAT_RSLT)sAT_PercentDATA ((T_ACI_CMD_SRC)src_infos_psi->srcId, red_mode,
                         p_des_dev_name, (UBYTE)(des_devId&DIO_DEVNO_MASK), (UBYTE)des_sub_no, p_dev_cap,
                         p_src_dev_name, (UBYTE)(src_devId&DIO_DEVNO_MASK), (UBYTE)src_sub_no, cid);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentDATA     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentDATA     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  UBYTE         mode ;
  UBYTE         cid  = (UBYTE)BAT_PDP_CID_NOT_PRESENT;
  T_DINF_PARAM  des_param;      /* des_dev_name, des_dev_no, des_sub_no, des_cap */
  T_DINF_PARAM  src_param;      /* src_dev_name, src_dev_no, src_sub_no, pdp_cid */
  T_CAP_ID      capability = DEV_CPBLTY_NONE;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_data que_data_buffer;
  U32 des_devId = BAT_PARAMETER_NOT_PRESENT;
  U32 src_devId = BAT_PARAMETER_NOT_PRESENT;
  
  TRACE_FUNCTION ("qBAT_PercentDATA()");
  resp.ctrl_response = BAT_RES_QUE_PERCENT_DATA; 
  resp.response.ptr_que_percent_data = &que_data_buffer;
  
  des_param.dev_id     = NOT_PRESENT_8BIT; /* in case of there is no redirection at all */
  des_param.capability = 0xAA;             /* init with a value which makes never sense */
  while ((ret = (T_ACI_BAT_RSLT)qAT_PercentDATA ((T_ACI_CMD_SRC)src_infos_psi->srcId, &mode, &cid, &des_param, &src_param)) EQ ACI_BAT_CMPL)
  {
    T_DTI_ENTITY_ID des_id = dti_entity_name[des_param.dev_id].id;
    T_DTI_ENTITY_ID src_id = dti_entity_name[src_param.dev_id].id;
    
    capability         = aci_bat_map_dti_cpblty (des_param.capability);

    /* With the device number, driver number and the dio id, form
       the device identifier of both the source and destination to 
       present to the terminal */

    des_devId = (((U32)des_param.dev_no )
                |((U32) (des_param.driver_id<<24))
                |((U32) (des_param.dio_ctrl_id <<8)));

    src_devId = (((U32)src_param.dev_no )
                 |((U32) (src_param.driver_id <<24))
                 |((U32) (src_param.dio_ctrl_id <<8)));

    /*
     * unfortunately in custom.h for PKTIO is defined "PKT" (Frame handles up to 4 chars only)
     * but for the device name we want to emit "PKTIO" to distinguish from capability "PKT"
     */
        
#ifdef GPRS
    if (des_param.dev_id EQ DTI_ENTITY_PKTIO)
    {
      des_id = DTI_ENTITY_PKTIO;
    }

    if (src_param.dev_id EQ DTI_ENTITY_PKTIO)
    {
      src_id = DTI_ENTITY_PKTIO;
    }
#endif

    switch(des_id)
    {
      case (DTI_ENTITY_UART):
      {
        resp.response.ptr_que_percent_data->des_dev_name = BAT_DEV_NAME_UART;
        break;
      }
#ifdef GPRS
      case (DTI_ENTITY_PKTIO):
      {
        resp.response.ptr_que_percent_data->des_dev_name = BAT_DEV_NAME_PKTIO;
        break;
      }
#endif
      case (DTI_ENTITY_PSI):
      {
        resp.response.ptr_que_percent_data->des_dev_name = BAT_DEV_NAME_PSI;
        break;
      }
      default:
      {
        TRACE_ERROR("ERROR: Undefined dev name type encoutered");
        break;
      }  
    }
    switch(src_id)
    {
      case (DTI_ENTITY_UART):
      {
        resp.response.ptr_que_percent_data->src_dev_name = BAT_DEV_NAME_UART;
        break;
      }
#ifdef GPRS
      case (DTI_ENTITY_PKTIO):
      {
        resp.response.ptr_que_percent_data->src_dev_name = BAT_DEV_NAME_PKTIO;
        break;
      }
#endif
      case (DTI_ENTITY_PSI):
      {
        resp.response.ptr_que_percent_data->src_dev_name = BAT_DEV_NAME_PSI;
        break;
      }
      default:
      {
        TRACE_ERROR("ERROR: Undefined dev name type encoutered");
        break;
      }  
    } 
    switch(aci_bat_cap_name[capability].id)
    {
      case (DEV_CPBLTY_CMD):
      {
        resp.response.ptr_que_percent_data->capability = BAT_CAP_CMD;
        break;
      }
      case (DEV_CPBLTY_SER):
      {
        resp.response.ptr_que_percent_data->capability = BAT_CAP_SER;
        break;
      }
      case (DEV_CPBLTY_CMD_SER):
      {
        resp.response.ptr_que_percent_data->capability = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_SER);
        break;
      }
      case (DEV_CPBLTY_PKT):
      {
        resp.response.ptr_que_percent_data->capability = BAT_CAP_PKT;
        break;
      }
      case (DEV_CPBLTY_CMD_PKT):
      {
        resp.response.ptr_que_percent_data->capability = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_PKT);
        break;
      }
      case (DEV_CPBLTY_PKT_SER):
      {
        resp.response.ptr_que_percent_data->capability = (T_BAT_capability)(BAT_CAP_SER + BAT_CAP_PKT);
        break;
      }
      case (DEV_CPBLTY_CMD_PKT_SER):
      {
        resp.response.ptr_que_percent_data->capability = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_SER + BAT_CAP_PKT);
        break;
      }
    }
    resp.response.ptr_que_percent_data->des_devId = des_devId;
    resp.response.ptr_que_percent_data->des_sub_no = des_param.sub_no;
    resp.response.ptr_que_percent_data->src_devId = src_devId;
    resp.response.ptr_que_percent_data->src_sub_no = src_param.sub_no;
    resp.response.ptr_que_percent_data->cid  = (T_BAT_pdp_cid)cid;
    resp.response.ptr_que_percent_data->mode = (T_BAT_percent_data_mode)mode;

    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentDINF     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentDINF     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  UBYTE         mode         ;    /* DINF  */
  T_DINF_PARAM  device_para;
  T_CAP_ID      capability   = DEV_CPBLTY_NONE;
  T_CAP_ID      cur_cap      = DEV_CPBLTY_NONE;
  T_BAT_cmd_response resp;
  T_BAT_res_set_percent_dinf que_dinf_buffer;
  U32 des_devId = BAT_PARAMETER_NOT_PRESENT;
  
  TRACE_FUNCTION ("sBAT_PercentDINF()");
  
  mode = cmd->params.ptr_set_percent_dinf->mode;
  resp.ctrl_response = BAT_RES_SET_PERCENT_DINF;
  resp.response.ptr_set_percent_dinf = &que_dinf_buffer;
  
  switch(mode)
  {
    case (BAT_P_DINF_MODE_CURRENT):
    {
      /*
       * we want to get the piece of information, which belongs to the srcId
       */
      ret = (T_ACI_BAT_RSLT)sAT_PercentDINF((T_ACI_CMD_SRC)src_infos_psi->srcId, mode, &device_para);
      if ((ret EQ ACI_BAT_CMPL) AND (device_para.src_id EQ src_infos_psi->srcId))
      {
        capability = aci_bat_map_dti_cpblty (device_para.capability);
        cur_cap    = aci_bat_map_dti_cpblty (device_para.cur_cap);

        /* With the device number, driver number and the dio id, form
           the device identifier of the destination to 
           present to the terminal */

        des_devId = (((U32)device_para.dev_no )
                     |((U32) (device_para.driver_id<<24))
                     |((U32) (device_para.dio_ctrl_id <<8)));

        resp.response.ptr_set_percent_dinf->dev_Id = des_devId;
        resp.response.ptr_set_percent_dinf->sub_no = device_para.sub_no;
        resp.response.ptr_set_percent_dinf->src_id = mode;

        switch(dti_entity_name[device_para.dev_id].id)
        {
          case (DTI_ENTITY_UART):
          {
            resp.response.ptr_set_percent_dinf->dev_name = BAT_DEV_NAME_UART;
            break;
          }
#ifdef GPRS
          case (DTI_ENTITY_PKTIO):
          {
            resp.response.ptr_set_percent_dinf->dev_name = BAT_DEV_NAME_PKTIO;
            break;
          }
#endif
          case (DTI_ENTITY_PSI):
          {
            resp.response.ptr_set_percent_dinf->dev_name = BAT_DEV_NAME_PSI;
            break;
          }
          default:
          {
            TRACE_ERROR("ERROR: Undefined dev name type encoutered");
            break;
          }  
        }
        switch(aci_bat_cap_name[cur_cap].id)
        {
          case (DEV_CPBLTY_CMD):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = BAT_CAP_CMD;
            break;
          }
          case (DEV_CPBLTY_SER):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = BAT_CAP_SER;
            break;
          }
          case (DEV_CPBLTY_CMD_SER):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_SER);
            break;
          }
          case (DEV_CPBLTY_PKT):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = BAT_CAP_PKT;
            break;
          }
          case (DEV_CPBLTY_CMD_PKT):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_PKT);
            break;
          }
          case (DEV_CPBLTY_PKT_SER):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = (T_BAT_capability)(BAT_CAP_SER + BAT_CAP_PKT);
            break;
          }
          case (DEV_CPBLTY_CMD_PKT_SER):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_SER + BAT_CAP_PKT);
            break;
          }
        }
        switch(aci_bat_cap_name[capability].id)
        {
          case (DEV_CPBLTY_CMD):
          {
            resp.response.ptr_set_percent_dinf->cap = BAT_CAP_CMD;
            break;
          }
          case (DEV_CPBLTY_SER):
          {
            resp.response.ptr_set_percent_dinf->cap = BAT_CAP_SER;
            break;
          }
          case (DEV_CPBLTY_CMD_SER):
          {
            resp.response.ptr_set_percent_dinf->cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_SER);
            break;
          }
          case (DEV_CPBLTY_PKT):
          {
            resp.response.ptr_set_percent_dinf->cap = BAT_CAP_PKT;
            break;
          }
          case (DEV_CPBLTY_CMD_PKT):
          {
            resp.response.ptr_set_percent_dinf->cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_PKT);
            break;
          }
          case (DEV_CPBLTY_PKT_SER):
          {
            resp.response.ptr_set_percent_dinf->cap = (T_BAT_capability)(BAT_CAP_SER + BAT_CAP_PKT);
            break;
          }
          case (DEV_CPBLTY_CMD_PKT_SER):
          {
            resp.response.ptr_set_percent_dinf->cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_SER + BAT_CAP_PKT);
            break;
          }
        }

      aci_bat_send(src_infos_psi,&resp);
      return (ACI_BAT_CMPL);
      }
      else
      {
        break; /* an error */
      }
    }
    case (BAT_P_DINF_MODE_ALL):
    {
      device_para.capability = 0xAA; /* init with a value which makes never sense, but is valuable for debug */
      /*
       * we want to get the pieces of information to all devices
       */
      while ((ret =(T_ACI_BAT_RSLT)sAT_PercentDINF ((T_ACI_CMD_SRC)src_infos_psi->srcId, mode, &device_para)) EQ ACI_BAT_CMPL)
      {
        if (device_para.src_id EQ src_infos_psi->srcId)
        {
          mode = BAT_P_DINF_MODE_CURRENT ; /* this is the current channel, so temporarily set mode */
        }
        else
        {
          mode = BAT_P_DINF_MODE_ALL; /* other channel */
        }

        capability = aci_bat_map_dti_cpblty (device_para.capability);
        cur_cap    = aci_bat_map_dti_cpblty (device_para.cur_cap);

        /* With the device number, driver number and the dio id, form
           the device identifier of the destination to 
           present to the terminal */

        des_devId = (((U32)device_para.dev_no )
                     |((U32) (device_para.driver_id<<24))
                     |((U32) (device_para.dio_ctrl_id <<8)));

        resp.response.ptr_set_percent_dinf->dev_Id = des_devId;
        resp.response.ptr_set_percent_dinf->sub_no = device_para.sub_no;
        resp.response.ptr_set_percent_dinf->src_id = mode;

        switch(dti_entity_name[device_para.dev_id].id)
        {
          case (DTI_ENTITY_UART):
          {
            resp.response.ptr_set_percent_dinf->dev_name = BAT_DEV_NAME_UART;
            break;
          }
#ifdef GPRS
          case (DTI_ENTITY_PKTIO):
          {
            resp.response.ptr_set_percent_dinf->dev_name = BAT_DEV_NAME_PKTIO;
            break;
          }
#endif
          case (DTI_ENTITY_PSI):
          {
            resp.response.ptr_set_percent_dinf->dev_name = BAT_DEV_NAME_PSI;
            break;
          }
          default:
          {
            TRACE_ERROR("ERROR: Undefined dev name type encoutered");
            break;
          }  
        }
        switch(aci_bat_cap_name[cur_cap].id)
        {
          case (DEV_CPBLTY_CMD):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = BAT_CAP_CMD;
            break;
          }
          case (DEV_CPBLTY_SER):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = BAT_CAP_SER;
            break;
          }
          case (DEV_CPBLTY_CMD_SER):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_SER);
            break;
          }
          case (DEV_CPBLTY_PKT):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = BAT_CAP_PKT;
            break;
          }
          case (DEV_CPBLTY_CMD_PKT):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_PKT);
            break;
          }
          case (DEV_CPBLTY_PKT_SER):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = (T_BAT_capability)(BAT_CAP_SER + BAT_CAP_PKT);
            break;
          }
          case (DEV_CPBLTY_CMD_PKT_SER):
          {
            resp.response.ptr_set_percent_dinf->cur_cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_SER + BAT_CAP_PKT);
            break;
          }
        }
        switch(aci_bat_cap_name[capability].id)
        {
          case (DEV_CPBLTY_CMD):
          {
            resp.response.ptr_set_percent_dinf->cap = BAT_CAP_CMD;
            break;
          }
          case (DEV_CPBLTY_SER):
          {
            resp.response.ptr_set_percent_dinf->cap = BAT_CAP_SER;
            break;
          }
          case (DEV_CPBLTY_CMD_SER):
          {
            resp.response.ptr_set_percent_dinf->cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_SER);
            break;
          }
          case (DEV_CPBLTY_PKT):
          {
            resp.response.ptr_set_percent_dinf->cap = BAT_CAP_PKT;
            break;
          }
          case (DEV_CPBLTY_CMD_PKT):
          {
            resp.response.ptr_set_percent_dinf->cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_PKT);
            break;
          }
          case (DEV_CPBLTY_PKT_SER):
          {
            resp.response.ptr_set_percent_dinf->cap = (T_BAT_capability)(BAT_CAP_SER + BAT_CAP_PKT);
            break;
          }
          case (DEV_CPBLTY_CMD_PKT_SER):
          {
            resp.response.ptr_set_percent_dinf->cap = (T_BAT_capability)(BAT_CAP_CMD + BAT_CAP_SER + BAT_CAP_PKT);
            break;
          }
        }
        aci_bat_send(src_infos_psi,&resp);
        mode = SHOW_ALL_CHAN_INF; /* reset to show all channels */
      }
      /*
       * when sAT_PercentDINF reached the end it returns with AT_FAIL, but check capability as sentinel
       */
      if (device_para.capability EQ DEV_CPBLTY_NONE)
      {
        /* 
         * DEV_CPBLTY_NONE is the sentinel, that there are no further devices 
         */
        return (ACI_BAT_CMPL);
      }
      break; /* an error */
    }
    default:
    {
      break; /* an error */
    }
  } 
  return (ACI_BAT_FAIL);
 }

