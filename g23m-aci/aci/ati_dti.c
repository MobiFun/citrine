/*
+--------------------------------------------------------------------+
| PROJECT: GSM-F&D (8411)           $Workfile:: ati_dti.c           $|
| $Author:: Rm                      $Revision:: 187                 $|
| CREATED: 13.11.02                $Modtime::                       $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+

   MODULE  : ATI 

   PURPOSE : AT Command to control data flow and devices in addition with DTI
             managment
*/

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef DTI

#ifndef ATI_DTI_C
#define ATI_DTI_C

#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */
#include "aci_lst.h"
#include "ati_int.h"
#include "aci_mem.h"
#include "cmh_dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#ifdef GPRS
#include "gaci_cmh.h"
#endif

#ifdef  FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /* FF_ATI_BAT */
/*==== EXPORT ==================================================*/
EXTERN CHAR  *cmdCmeError ( T_ACI_CME_ERR e );
EXTERN CHAR  *parse(CHAR *b,CHAR *f, ...);

/*==== VARIABLE====================================================*/

const T_CAP_NAME ati_cap_name[] =
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

/*==== CONSTANT====================================================*/

#define DIO_DEVNO_MASK 0xFF

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
GLOBAL T_CAP_ID map_dti_cpblty (UBYTE dti_cpblty)
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
      TRACE_EVENT_P1("map_dti_cpblty():[ERR] unknown capability value = %d",dti_cpblty);
      return (DEV_CPBLTY_NONE);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  :                    |
| STATE   : code                        ROUTINE :  setatPercentDATA  |
+--------------------------------------------------------------------+

  PURPOSE : set %DATA command ( control data flow )
            format: 
            %DATA=<mode>,<des_dev_name>,<des_dev_no>,[<sub_no>],<cap>[,<src_dev_name>,<src_dev_no>,[<sub_no>],[,<cid>]]
            example:
            %DATA=2,"RIV",1,,"PKT","SNDCP",1,,,
            means, set a permanent(2) redirection for "RIV" with packet capability to "SNDCP"
*/

GLOBAL T_ATI_RSLT setatPercentDATA ( char *cl, UBYTE srcId )

{
  CHAR          des_dev_name[MAX_SMS_ADDR_DIG];  /* destination device name       */
  CHAR          src_dev_name[MAX_SMS_ADDR_DIG];  /* source device name            */
  CHAR          dev_cap[MAX_SMS_ADDR_DIG];       /* device capability             */
  UBYTE         red_mode   = NOT_PRESENT_8BIT;   /* redirection mode              */
  UBYTE         pdp_cid    = NOT_PRESENT_8BIT;   /* context cid                   */
  U32           des_devId  = NOT_PRESENT_32BIT; /* destination device identifier  */
  SHORT         des_sub_no = ACI_NumParmNotPresent; /* destination sub number     */
  U32           src_devId  = NOT_PRESENT_32BIT; /* source device identifier       */
  SHORT         src_sub_no = ACI_NumParmNotPresent; /* source sub number          */
  T_ACI_RETURN  ret        = AT_FAIL;

  src_dev_name[0]='\0';
  des_dev_name[0]='\0';

  /* parse mode,des_dev_name, des_dev_no, cap, src_dev_name, src_dev_no, cid */
  cl = parse (cl, "xsyrssyrx", &red_mode,
                               (LONG)MAX_SMS_ADDR_DIG,
                               des_dev_name, &des_devId, &des_sub_no,
                               (LONG)MAX_SMS_ADDR_DIG,
                               dev_cap,
                               (LONG)MAX_SMS_ADDR_DIG,
                               src_dev_name, &src_devId, &src_sub_no,
                               &pdp_cid );
  if (!cl)
  {
      cmdCmeError (CME_ERR_Unknown);
      return (ATI_FAIL);
  }

  /*

   * if optional parameters have not been given, set to defaults
   */
  if (des_sub_no EQ ACI_NumParmNotPresent)
  {
    des_sub_no = 0;
  }
  if (src_sub_no EQ ACI_NumParmNotPresent)
  {
    src_sub_no = 0;
  }
  if (pdp_cid EQ NOT_PRESENT_8BIT)
  {
    pdp_cid = 0;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_data my_bat_set_percent_data;

  TRACE_FUNCTION("setatPercentDATA() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_data, 0, sizeof(my_bat_set_percent_data));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_DATA;
  cmd.params.ptr_set_percent_data = &my_bat_set_percent_data;

  my_bat_set_percent_data.mode = red_mode;   

  if (!strcmp(des_dev_name, "UART"))
    my_bat_set_percent_data.des_dev_name = BAT_DEV_NAME_UART;  
  else if (!strcmp(des_dev_name, "RIV"))
    my_bat_set_percent_data.des_dev_name = BAT_DEV_NAME_RIV;  
  else if (!strcmp(des_dev_name, "PKTIO"))
    my_bat_set_percent_data.des_dev_name = BAT_DEV_NAME_PKTIO;  
  else if (!strcmp(des_dev_name, "PSI"))
    my_bat_set_percent_data.des_dev_name = BAT_DEV_NAME_PSI;  
  
  my_bat_set_percent_data.des_devId = des_devId;    
  my_bat_set_percent_data.des_sub_no = des_sub_no;   
  
  if (!strcmp(dev_cap, "CMD"))
    my_bat_set_percent_data.capability = BAT_CAP_CMD;
  else if (!strcmp(dev_cap, "SER"))
    my_bat_set_percent_data.capability = BAT_CAP_SER;
  else if(!strcmp(dev_cap, "PKT"))
    my_bat_set_percent_data.capability = BAT_CAP_PKT;
  
  if (!strcmp(des_dev_name, "UART"))
    my_bat_set_percent_data.src_dev_name = BAT_DEV_NAME_UART;  
  else if (!strcmp(src_dev_name, "RIV"))
    my_bat_set_percent_data.src_dev_name = BAT_DEV_NAME_RIV;  
  else if (!strcmp(src_dev_name, "PKTIO"))
    my_bat_set_percent_data.src_dev_name = BAT_DEV_NAME_PKTIO;  
  else if (!strcmp(src_dev_name, "PSI"))
    my_bat_set_percent_data.src_dev_name = BAT_DEV_NAME_PSI;  

  my_bat_set_percent_data.src_devId = des_devId;   
  my_bat_set_percent_data.src_sub_no = src_sub_no;   
  my_bat_set_percent_data.cid = pdp_cid;           

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION( "setatPercentDATA()" );

  ret = sAT_PercentDATA ((T_ACI_CMD_SRC)srcId, red_mode,
                          des_dev_name,(UBYTE)(des_devId&DIO_DEVNO_MASK),(UBYTE)des_sub_no,
                          dev_cap,src_dev_name,(UBYTE)(src_devId&DIO_DEVNO_MASK),
                          (UBYTE)src_sub_no,pdp_cid);

  if (ret EQ AT_FAIL)
  {
    cmdCmeError (CME_ERR_Unknown);
    return (ATI_FAIL);
  }
  else
  {
    return (map_aci_2_ati_rslt(ret));
  }

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)                MODULE  :                  |
| STATE   : code                          ROUTINE :  queatPercentDATA|
+--------------------------------------------------------------------+

  PURPOSE : query %DATA command ( control data flow )
*/

GLOBAL T_ATI_RSLT queatPercentDATA ( char *cl, UBYTE srcId )
{
  UBYTE         mode = (UBYTE)-1; /* mode is UBYTE type so typecasting -1 with UBYTE  */
  UBYTE         cid  = (UBYTE)-1;    /* cid is UBYTE type so typecasting -1 with UBYTE  */
  T_DINF_PARAM  des_param;      /* des_dev_name, des_dev_no, des_sub_no, des_cap */
  T_DINF_PARAM  src_param;      /* src_dev_name, src_dev_no, src_sub_no, pdp_cid */
  T_CAP_ID      capability = DEV_CPBLTY_NONE;

#ifdef GPRS
  char          *pktioName = "PKTIO";
#endif /* GPRS */
     
  TRACE_FUNCTION( "queatPercentDATA()" );

  des_param.dev_id     = NOT_PRESENT_8BIT; /* in case of there is no redirection at all */
  des_param.capability = 0xAA;             /* init with a value which makes never sense */
  /*
   * we want to get the pieces of information to all redirected devices
   */
  while ((qAT_PercentDATA ((T_ACI_CMD_SRC)srcId, &mode, &cid, &des_param, &src_param)) EQ AT_CMPL)
  {
    U32 des_devId = NOT_PRESENT_32BIT;
    U32 src_devId = NOT_PRESENT_32BIT;
    char *tmp_des_name = dti_entity_name[des_param.dev_id].name;
    char *tmp_src_name = dti_entity_name[src_param.dev_id].name;
    capability         = map_dti_cpblty (des_param.capability);
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
      tmp_des_name = pktioName;
    }

    if (src_param.dev_id EQ DTI_ENTITY_PKTIO)
    {
      tmp_src_name = pktioName;
    }
#endif
    sprintf (g_sa,"%s: %d,\"%s\",%x,%d,\"%s\",\"%s\",%x,%d,%d", "%DATA",
                                                                mode,
                                                                tmp_des_name, 
                                                                des_devId, 
                                                                des_param.sub_no, 
                                                                ati_cap_name[capability].name,
                                                                tmp_src_name, 
                                                                src_devId, 
                                                                src_param.sub_no, 
                                                                cid);

    io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  /*
   * when qAT_PercentDATA reached the end it returns with AT_FAIL, but check capability as sentinel
   */
  if (des_param.capability EQ DEV_CPBLTY_NONE)
  {
    /* 
     * DEV_CPBLTY_NONE is the sentinel, that there are no further devices 
     */
    return (ATI_CMPL);
  }
  else if (des_param.dev_id EQ NOT_PRESENT_8BIT)
  {
    /* 
     * NOT_PRESENT_8BIT, there is no redirection at all, but print default
     */
    sprintf (g_sa,"%s: 0,,0,0,,,0,0,0", "%DATA"); 
    io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }

  cmdCmeError (CME_ERR_Unknown);
  return (ATI_FAIL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  :                     |
| STATE   : code                       ROUTINE :  setatPercentDINF   |
+--------------------------------------------------------------------+

  PURPOSE : show list of available data and ATI channels and capabilities
            %DINF=<mode>   mode: 0=current, 1=other
*/

GLOBAL T_ATI_RSLT setatPercentDINF( char *cl, UBYTE srcId )
{
  UBYTE               mode      = (UBYTE)-1;    /* mode is UBYTE type so typecasting -1 with UBYTE  */
  T_ACI_RETURN  ret          = AT_FAIL;
  T_CAP_ID      capability   = DEV_CPBLTY_NONE;
  T_CAP_ID      cur_cap      = DEV_CPBLTY_NONE;

#ifdef GPRS
  char          *pktioName   = "PKTIO";
#endif /* GPRS */
  U32 des_devId = NOT_PRESENT_32BIT;
  
  cl = parse (cl, "x", &mode);
  if (!cl)
  {
    cmdCmeError (CME_ERR_Unknown);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_dinf my_bat_set_percent_dinf;

  TRACE_FUNCTION("setatPercentDINF() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_dinf, 0, sizeof(my_bat_set_percent_dinf));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_DINF;
  cmd.params.ptr_set_percent_dinf = &my_bat_set_percent_dinf;

  my_bat_set_percent_dinf.mode = mode;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */
  {
  T_DINF_PARAM  device_para;

  TRACE_FUNCTION( "setatPercentDINF()" );

  switch (mode)
  {
    case (SHOW_CURR_CHAN_INF):
    {
      /*
       * we want to get the piece of information, which belongs to the srcId
       */
      ret = sAT_PercentDINF ((T_ACI_CMD_SRC)srcId, mode, &device_para);

      if ((ret EQ AT_CMPL) AND (device_para.src_id EQ srcId))
      {
        char *tmp_des_name = dti_entity_name[device_para.dev_id].name;
        capability = map_dti_cpblty (device_para.capability);
        cur_cap    = map_dti_cpblty (device_para.cur_cap);
        /* With the device number, driver number and the dio id, form
           the device identifier of the destination and  
           present to the terminal */

        des_devId = (((U32)device_para.dev_no)
                     |((U32) (device_para.driver_id <<24))
                     |((U32) (device_para.dio_ctrl_id <<8)));


        /*
         * unfortunately in custom.h for PKTIO is defined "PKT" (Frame handles up to 4 chars only)
         * but for the device name we want to emit "PKTIO" to distinguish from capability "PKT"
         */
#ifdef GPRS
    if (device_para.dev_id EQ DTI_ENTITY_PKTIO)
    {
      tmp_des_name = pktioName;
    }
#endif

        sprintf (g_sa,"%s:\"%s\",%x,%d,\"%s\",\"%s\",%d", "%DINF",
                                                          tmp_des_name,
                                                          des_devId,
                                                          device_para.sub_no,
                                                          ati_cap_name[capability].name,
                                                          ati_cap_name[cur_cap].name,
                                                          mode); /* current channel */



        io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
        return (ATI_CMPL);
      }
      else
      {
        break; /* an error */
      }
    }
    case (SHOW_ALL_CHAN_INF):
    {
      device_para.capability = 0xAA; /* init with a value which makes never sense, but is valuable for debug */
      /*
       * we want to get the pieces of information to all devices
       */
      while ((ret = sAT_PercentDINF ((T_ACI_CMD_SRC)srcId, mode, &device_para)) EQ AT_CMPL)
      {
        char *tmp_des_name = dti_entity_name[device_para.dev_id].name;
        if (device_para.src_id EQ srcId)
        {
          mode = SHOW_CURR_CHAN_INF; /* this is the current channel, so temporarily set mode */
        }
        else
        {
          mode = SHOW_ALL_CHAN_INF; /* other channel */
        }

        capability = map_dti_cpblty (device_para.capability);
        cur_cap    = map_dti_cpblty (device_para.cur_cap);
        /* With the device number, driver number and the dio id, form
           the device identifier of the destination and  
           present to the terminal */

        des_devId = (((U32)device_para.dev_no)
                     |((U32) (device_para.driver_id <<24))
                     |((U32) (device_para.dio_ctrl_id <<8)));



        /*
         * unfortunately in custom.h for PKTIO is defined "PKT" (Frame handles up to 4 chars only)
         * but for the device name we want to emit "PKTIO" to distinguish from capability "PKT"
         */
#ifdef GPRS
    if (device_para.dev_id EQ DTI_ENTITY_PKTIO)
    {
      tmp_des_name = pktioName;
    }
#endif
        sprintf (g_sa,"%s:\"%s\",%x,%d,\"%s\",\"%s\",%d", "%DINF",
                                                          tmp_des_name,
                                                          des_devId, 
                                                          device_para.sub_no,
                                                          ati_cap_name[capability].name,
                                                          ati_cap_name[cur_cap].name,
                                                          mode);
        io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
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
        return (ATI_CMPL);
      }

      break; /* an error */
    }
    default:
    {
      break; /* an error */
    }
  }

  cmdCmeError (CME_ERR_Unknown);
  return (ATI_FAIL);
  }
#endif /* no FF_ATI_BAT*/
}

#endif /* ATI_DTI_C */
#endif /* DTI */

