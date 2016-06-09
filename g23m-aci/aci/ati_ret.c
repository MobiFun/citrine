/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ATI
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
|  Purpose :  AT Command Interpreter Call-Back Functions
+-----------------------------------------------------------------------------
*/

#ifndef ATI_RET_C
#define ATI_RET_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"  /* defines FF_TIMEZONE */

#include "line_edit.h"
#include "aci_cmh.h"  /* uses FF_TIMEZONE    */
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_io.h"
#include "p_aci.h"
#include "aci.h"
#include "l4_tim.h"
#include "ksd.h"

#include "psa.h"
#include "psa_sms.h"
#include "psa_sim.h"
#include "cmh.h"
#include "cmh_sms.h"
#include "psa_cc.h"
#include "psa_ss.h"
#include "cmh_ss.h"
#include "cmh_cc.h"
#include "cmh_mm.h"

#include "psa_util.h"
#include "cmh_mm.h"

#ifdef DTI
#include "dti.h"      /* functionality of the dti library */
#endif
#include "dti_conn_mng.h"

#ifdef GPRS
  #include "gaci.h"
  #include "gaci_cmh.h"
  #include "psa_sm.h"
  #include "cmh_sm.h"
#endif  /* GPRS */

#include "aci_mem.h"

#include "aci_lst.h"
#ifdef UART
#include "psa_uart.h"
#include "cmh_uart.h"
#endif
#include "conc_sms.h"

#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#endif /*FF_PSI*/

#ifdef GPRS
#include "gaci_cmd.h"
#endif
#include "aci_cmd.h"

#if defined FF_EOTD
#include "cmh_lc.h"
#endif

#include "ati_int.h"

#ifdef ACI
#include "cmh_mmi.h"
#endif /* ACI */

#ifdef FF_ATI

GLOBAL  T_CIEV_SIGNAL_BUFFER  asCievSignalBuf;
GLOBAL  T_CIEV_SMSFULL_BUFFER asCievSmsFullBuf;

EXTERN CHAR *CPIN_RESULT(T_ACI_CPIN_RSLT code);
EXTERN UBYTE src_id_ext; /* this source runs currently an extension command */

EXTERN T_ATI_IO_MODE ati_get_mode  (UBYTE srcId );
EXTERN T_ATI_RSLT ati_execute_cmd_line (T_ATI_SRC_PARAMS *src_params);

/* 
 *  needed for rCI_Z 
 */
EXTERN void ati_cmd_reset          (UBYTE srcId);
EXTERN void ati_zn_retrieve_params ( UBYTE srcId );


LOCAL UBYTE aci_writeCnumMsisdn (UBYTE srcId, T_ACI_CNUM_MSISDN* msisdn );
LOCAL void aci_frmtOutput  ( UBYTE fo,
                             UBYTE dcs,
                             T_ACI_SM_DATA *data );

LOCAL  void  r_Plus_Percent_COPS( T_ACI_AT_CMD cmd,
                            SHORT lastIdx,
                            T_ACI_COPS_OPDESC * operLst);
LOCAL void aci_format_plmn_name(T_full_name * plmn, UBYTE *out);							
/*
typedef struct
{
  char *name;
  T_ACI_FAC fac;
} net_fac; */

LOCAL BOOL  allowCCCM = TRUE;
LOCAL UBYTE callTime  = 0;

LOCAL const SMS_Memory sms_mem [] =

{
  {"ME",  SMS_STOR_Me},
  {"me",  SMS_STOR_Me},
  {"SM",  SMS_STOR_Sm},
  {"sm",  SMS_STOR_Sm},
  {0,SMS_STOR_Me}
};

#if defined (SMS_PDU_SUPPORT)

LOCAL void rCI_PlusCMTText  ( T_MNSMS_MESSAGE_IND * mnsms_message_ind);
LOCAL void rCI_PlusCDSText ( T_MNSMS_STATUS_IND * mnsms_status_ind );
LOCAL void rCI_PlusCBMText ( T_MMI_CBCH_IND     * mmi_cbch_ind );
LOCAL void rCI_PlusCMGSText( T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf);
LOCAL void rCI_PlusCMSSText( T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf);
LOCAL void rCI_PlusCMGCText( T_MNSMS_COMMAND_CNF * mnsms_command_cnf);
#ifdef REL99
LOCAL void rCI_PercentCMGRSText ( UBYTE mode,
                                  T_MNSMS_RETRANS_CNF * mnsms_retrans_cnf,
                                  T_MNSMS_SEND_PROG_IND * mnsms_send_prog_ind );
#endif /* REL99 */
#endif

LOCAL void rCI_Plus_Percent_CMGLText ( T_MNSMS_READ_CNF *mnsms_read_cnf,
                                       T_ACI_AT_CMD cmd);
LOCAL void rCI_Plus_Percent_CMGRText ( T_MNSMS_READ_CNF  * mnsms_read_cnf,
                                       T_ACI_CMGR_CBM * cbm,
                                       T_ACI_AT_CMD cmd);
LOCAL void rCI_Plus_Percent_CMGLTextSP ( T_MNSMS_READ_CNF* mnsms_read_cnf,
                                        T_ACI_AT_CMD cmdId);
LOCAL void rCI_Plus_Percent_CMGRTextSP ( T_MNSMS_READ_CNF* mnsms_read_cnf,
                                        T_ACI_AT_CMD cmdId);

GLOBAL CHAR *sms_status (T_ACI_SMS_STAT stat) /* used in text mode by CMGL and CMGR */
{
  switch(stat)
  {
  case(SMS_STAT_Invalid):   return "INVALID MESSAGE";
  case(SMS_STAT_RecUnread): return "REC UNREAD";
  case(SMS_STAT_RecRead):   return "REC READ";
  case(SMS_STAT_StoUnsent): return "STO UNSENT";
  case(SMS_STAT_StoSent):   return "STO SENT";
  case(SMS_STAT_All):       return "ALL";
  default:                  return 0;
  }
}

GLOBAL char *ring_stat(T_ACI_CRING_SERV_TYP type)
{
  switch(type)
  {
  case(CRING_SERV_TYP_NotPresent): return  "Not Present";
  case(CRING_SERV_TYP_Async):      return  "ASYNC";
  case(CRING_SERV_TYP_Sync):       return  "SYNC";
  case(CRING_SERV_TYP_RelAsync):   return  "REL ASYNC";
  case(CRING_SERV_TYP_RelSync):    return  "REL SYNC";
  case(CRING_SERV_TYP_Fax):        return  "FAX";
  case(CRING_SERV_TYP_Voice):      return  "VOICE";
  case(CRING_SERV_TYP_AuxVoice):   return  "AUX VOICE";
  default:                    return  "Error";
  }
}
#define RING_STAT_LTH 15
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_OK             |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_OK call back

*/

GLOBAL void rCI_OK ( /*UBYTE srcId,*/ T_ACI_AT_CMD cmdId )
{
  UBYTE          srcId  = srcId_cb;

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_OK()");

  src_params->curAtCmd = AT_CMD_NONE;

  switch (cmdId)
  {
  case (AT_CMD_D):
    TRACE_EVENT("AT command line successfully processed") ;
    /*lint -fallthrough*/
  case (AT_CMD_NONE):
  case (AT_CMD_EXT):
#if FF_FAX
  case (AT_CMD_FDR):
  case (AT_CMD_FDT):
  case (AT_CMD_FKS):
#endif
    /*
     * tell line edit that the cmd line is finished/aborted
     * and to be able to receive a new one
     */
    ledit_ctrl (srcId,LEDIT_CTRL_CMPL, NULL);
    io_sendConfirm (srcId, cmdAtError(atOk), ATI_NORMAL_OUTPUT);
    curAbrtCmd = AT_CMD_NONE;
    cmdErrStr  = NULL;
    break;

  default:
    /*
     * there are possibly further AT cmds after re-entering the ACI context
     */
    /* this is only a bypass for ATI sources which uses the functional interface */
    if (src_params->cmd_state EQ CMD_IDLE)
    {
      io_sendConfirm (src_params->src_id, cmdAtError(atOk), ATI_NORMAL_OUTPUT);
    }
    else
    {
      /*
       * there are possibly further AT cmds after re-entering the ACI context
       */
      ati_execute_cmd_line (src_params);
     }
    break;
  }
#ifdef ACI /* for ATI only version */
  cmhMMI_handleAudioTone ( cmdId, RAT_OK, CPI_MSG_NotPresent );
#endif
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_NO_CARRIER     |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_NO_CARRIER call back

*/

GLOBAL void rCI_NO_CARRIER ( /*UBYTE srcId,*/ T_ACI_AT_CMD cmdId, SHORT cId )
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_NO_CARRIER()");

  if (   src_params->curAtCmd EQ AT_CMD_D
      OR src_params->curAtCmd EQ AT_CMD_A
      OR src_params->curAtCmd EQ AT_CMD_H
      OR src_params->curAtCmd EQ AT_CMD_CGDATA
      OR src_params->curAtCmd EQ AT_CMD_CGANS
#ifdef FF_FAX
      OR src_params->curAtCmd EQ AT_CMD_FDR
      OR src_params->curAtCmd EQ AT_CMD_FDT
#endif
      )
  {
    ledit_ctrl (srcId,LEDIT_CTRL_CMPL, NULL);
    src_params->curAtCmd = AT_CMD_NONE;
    io_sendConfirm(srcId, cmdAtError(atNoCarrier), (T_ATI_OUTPUT_TYPE)(ATI_ERROR_OUTPUT | ATI_RESULT_CODE_OUTPUT));
  }
  else
  {
    /*
     *  According to the v25ter NO CARRIER is stated as a final result code.
     *
     *  This implementation differ in this case from the standard.
     *  There are two differnt reasons:
     *     - Data transfer on a different source than the transfer was started
     *     - NO CARRIER if the other peer of a CS call hangs up
     *
     *  For the first reason the CONNECT will be the final result code (standard: intermediate)
     *  For the second reason the ATD resuts an OK.
     */
    io_sendIndication(srcId, cmdAtError(atNoCarrier), (T_ATI_OUTPUT_TYPE)(ATI_FORCED_OUTPUT | ATI_RESULT_CODE_OUTPUT));
  }

  cmdErrStr   = NULL;

#ifdef ACI /* for ATI only version */
  cmhMMI_handleAudioTone ( cmdId, RAT_NO_CARRIER, CPI_MSG_NotPresent );
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_BUSY           |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_BUSY call back

*/

GLOBAL void rCI_BUSY ( /*UBYTE srcId,*/ T_ACI_AT_CMD cmdId, SHORT cId )
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_BUSY()");

  src_params->curAtCmd    = AT_CMD_NONE;

  ledit_ctrl (srcId,LEDIT_CTRL_CMPL, NULL);

  io_sendConfirm(srcId, cmdAtError(atBusy), ATI_ERROR_OUTPUT);

#ifdef ACI /* for ATI only version */
  cmhMMI_handleAudioTone ( cmdId, RAT_BUSY, CPI_MSG_NotPresent );
#endif

  cmdErrStr   = NULL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_NO_ANSWER      |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_NO_ANSWER call back

*/

GLOBAL void rCI_NO_ANSWER ( /*UBYTE srcId,*/ T_ACI_AT_CMD cmdId, SHORT cId )
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_NO_ANSWER()");

  src_params->curAtCmd    = AT_CMD_NONE;

  ledit_ctrl (srcId,LEDIT_CTRL_CMPL, NULL);

  io_sendConfirm(srcId, cmdAtError(atNoAnswer), ATI_ERROR_OUTPUT);

  cmdErrStr   = NULL;

#ifdef ACI /* for ATI only version */
  cmhMMI_handleAudioTone ( cmdId, RAT_NO_ANSWER, CPI_MSG_NotPresent );
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_CONNECT        |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_CONNECT call back.
            flow_cntr = TRUE flow control active.
            flow_cntr = FALSE  "       "  inactive.

*/

GLOBAL void rCI_CONNECT   ( /*UBYTE srcId,*/
                            T_ACI_AT_CMD   cmdId,
                            T_ACI_BS_SPEED speed,
                            SHORT          cId,
                            BOOL           flow_cntr )
{
  LONG val;
  LONG val_nv=-1;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  CHAR  *cmd_key = "";

  TRACE_FUNCTION("rCI_CONNECT()");

  src_params->curAtCmd = AT_CMD_NONE;

  switch(speed)
  {
  case(BS_SPEED_300_V110): val=300;     val_nv = 10; break;
  case(BS_SPEED_1200_V110): val=1200;   val_nv = 11; break;
  case(BS_SPEED_2400_V110): val=2400;   val_nv = 13; break;
  case(BS_SPEED_4800_V110): val=4800;   val_nv = 14; break;
  case(BS_SPEED_9600_V110): val=9600;   val_nv = 15; break;
  case(BS_SPEED_14400_V110): val=14400; val_nv = 16; break;
  case(BS_SPEED_19200_V110): val=19200; break;
  case(BS_SPEED_38400_V110): val=38400; break;
  default:  val=-1;
  }

  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] rCI_CONNECT(): srcId=%d not found", srcId);
    return;
  }
#ifdef UART  
  if (src_params->src_type EQ ATI_SRC_TYPE_UART)
  {
    T_ACI_DTI_PRC *uart_infos;

    uart_infos = find_element (uart_src_params,
                               (UBYTE)srcId,
                               cmhUARTtest_srcId);
    if( flow_cntr )
    {
      /* this is in case of fax sending */
      BITFIELD_SET (uart_infos->data_cntr, UART_DTI_FLOW_OFF);
    }
    else
    {
      BITFIELD_CLEAR (uart_infos->data_cntr, UART_DTI_FLOW_OFF);
    }
  }
#endif
#ifdef FF_PSI
  if (src_params->src_type EQ ATI_SRC_TYPE_PSI)
  {
    T_ACI_DTI_PRC_PSI *psi_infos;

    psi_infos = find_element (psi_src_params,
                               (UBYTE)srcId,
                               cmhPSItest_srcId);
    if( flow_cntr )
    {
      /* this is in case of fax sending */
      BITFIELD_SET (psi_infos->data_cntr, PSI_DTI_FLOW_OFF);
    }
    else
    {
      BITFIELD_CLEAR (psi_infos->data_cntr, PSI_DTI_FLOW_OFF);
    }
  }
#endif /*FF_PSI*/

  if(val > 0 AND ati_user_output_cfg[srcId].atX > 0)
  {
    if(at.s1415.atV NEQ 0)          /*Verbose case*/
    {
      sprintf(g_sa,"CONNECT %d",val);
      io_sendConfirm(srcId, g_sa, (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT | ATI_CONNECT_OUTPUT));
    }
    else if(val_nv > 0)             /*Nonverbose case*/
    {
      sprintf(g_sa,"%d",val_nv);
      io_sendConfirm(srcId, g_sa, (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT | ATI_CONNECT_OUTPUT) );
    }
    else
      io_sendConfirm(srcId, cmdAtError(atConnect), (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT | ATI_CONNECT_OUTPUT));
  }
  else
  {
    io_sendConfirm(srcId, cmdAtError(atConnect), (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT | ATI_CONNECT_OUTPUT));
  }

  /*
   * tell line edit that the cmd line is finished/aborted
   * and to be able to receive a new one
   */
  ati_get_cmds_key(cmdId, &cmd_key, NULL);
  TRACE_EVENT_P1("%s completed", cmd_key);
  ledit_ctrl (srcId,LEDIT_CTRL_CMPL, NULL);

  cmdErrStr   = NULL;

#ifdef ACI /* for ATI only version */
  cmhMMI_handleAudioTone ( cmdId, RAT_CONNECT, CPI_MSG_NotPresent );
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCME        |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCME call back

*/

GLOBAL void rCI_PlusCME ( /*UBYTE srcId,*/ T_ACI_AT_CMD cmdId, T_ACI_CME_ERR err )
{
  UBYTE srcId = srcId_cb;

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PLusCME()");
  /*
   * tell line edit that the cmd line is finished/aborted
   * and to be able to receive a new one
   */
  ledit_ctrl (srcId,LEDIT_CTRL_CMPL, NULL);
  /*
   * if (asynchroniously) extension command, reset the global src_id_ext
   */
  if (src_id_ext EQ src_params->src_id)
  {
    src_id_ext = 0xFF;
  }
  if ( curAbrtCmd EQ AT_CMD_NONE )
  {
    src_params->curAtCmd    = AT_CMD_NONE;

    io_sendConfirm(srcId, cmdCmeError(err), ATI_ERROR_OUTPUT);

    cmdErrStr   = NULL;

#ifdef ACI /* for ATI only version */
    cmhMMI_handleAudioTone ( cmdId, RAT_CME, CPI_MSG_NotPresent );
#endif
  }
  else
  {
    curAbrtCmd = AT_CMD_NONE;

    io_sendMessage ( srcId, cmdCmeError ( CME_ERR_FailedToAbort ),
                          (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT | ATI_RESULT_CODE_OUTPUT));
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCPIN       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCPIN call back

*/

GLOBAL void rCI_PlusCPIN (/*UBYTE srcId,*/ T_ACI_CPIN_RSLT rslt )
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PLusCPIN()");

  src_params->curAtCmd = AT_CMD_NONE;

  sprintf(g_sa,"%s%s","+CPIN: ",CPIN_RESULT(rslt));
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

typedef enum
{
  STAT_NotPresent = -1,
  STAT_Other,           /* other than registered and Roaming */
  STAT_Reg,
  STAT_Roam
} T_ACI_NTW_STATUS;

LOCAL T_ACI_NTW_STATUS get_raw_stat_among_CREG( T_ACI_CREG_STAT status )
{
  switch(status)
  {
  case(CREG_STAT_NotPresent):
    return(STAT_NotPresent);

  case(CREG_STAT_Reg):
    return(STAT_Reg);

  case(CREG_STAT_Roam):
    return(STAT_Roam);

  default:
    return(STAT_Other);
   }
}

#ifdef GPRS
LOCAL T_ACI_NTW_STATUS get_raw_stat_among_CGREG( T_CGREG_STAT status )
{
  switch(status)
  {
  case(CGREG_STAT_REG_HOME):
    return(STAT_Reg);

  case(CGREG_STAT_REG_ROAM):
    return(STAT_Roam);

  default:
    return(STAT_Other);
  }
}

LOCAL T_ACI_NTW_STATUS get_raw_stat_among_P_CGREG( T_P_CGREG_STAT status )
{
  switch(status)
  {
  case(P_CGREG_STAT_REG_HOME):
    return(STAT_Reg);

  case(P_CGREG_STAT_REG_ROAM):
    return(STAT_Roam);

  default:
    return(STAT_Other);
  }
}
#endif /* GPRS */

/* function fills up registration status parameters according to type of
user notification (+CREG/+CGREG/%CGREG)
returns whether network status has changed or not: BOOL */
LOCAL BOOL get_registration_data_among_cmd( UBYTE                  srcId,
                                            int                    status,
                                            T_ATI_REG_MOD_LAC_CID *mod_lac_cid,
                                            T_ACI_NTW_STATUS      *raw_status,
                                            CHAR                  *cmd_string,
                                            T_ACI_CREG_CMD         cmd)
{
  USHORT  previous_status,
          current_status;

  current_status = previous_status = status;

  switch(cmd)
  {
  case(CREG_CMD):
    *mod_lac_cid    = ati_user_output_cfg[srcId].creg.mod_lac_cid;
    previous_status = ati_user_output_cfg[srcId].creg.last_presented_state;

    *raw_status = get_raw_stat_among_CREG((T_ACI_CREG_STAT) current_status);
    strcpy(cmd_string, "+CREG: ");
    break;

  case(PercentCREG_CMD):
    *mod_lac_cid    = ati_user_output_cfg[srcId].percent_creg.mod_lac_cid;
    previous_status = ati_user_output_cfg[srcId].percent_creg.last_presented_state;

    *raw_status = get_raw_stat_among_CREG((T_ACI_CREG_STAT) current_status);
    strcpy(cmd_string, "%CREG: ");
    break;


#if defined (GPRS) AND defined (DTI)
  case(PlusCGREG_CMD):
    *mod_lac_cid    = ati_gprs_user_output_cfg[srcId].plus_cgreg.mod_lac_cid;
    previous_status = ati_gprs_user_output_cfg[srcId].plus_cgreg.last_presented_state;

    *raw_status = get_raw_stat_among_CGREG((T_CGREG_STAT) current_status);
    strcpy(cmd_string, "+CGREG: ");
    break;

  case(PercentCGREG_CMD):
    *mod_lac_cid    = ati_gprs_user_output_cfg[srcId].percent_cgreg.mod_lac_cid;
    previous_status = ati_gprs_user_output_cfg[srcId].percent_cgreg.last_presented_state;

    *raw_status = get_raw_stat_among_P_CGREG((T_P_CGREG_STAT) current_status);
    strcpy(cmd_string, "%CGREG: ");
    break;
#endif
  }

  if( current_status NEQ previous_status)
  {
    return TRUE;
  }

  return FALSE;
}

LOCAL void update_last_presented_datas_among_cmd( UBYTE           srcId,
                                                  int             status,
                                                  USHORT          lac,
                                                  USHORT          cid,
                                                  T_ACI_CREG_CMD  cmd)
{
  T_ATI_REG_MOD_LAC_CID *mod_lac_cid;

  switch(cmd)
  {
  case(CREG_CMD):
    ati_user_output_cfg[srcId].creg.last_presented_state = (T_ACI_CREG_STAT)status;
    mod_lac_cid = &ati_user_output_cfg[srcId].creg.mod_lac_cid;
    break;

  case(PercentCREG_CMD):
    ati_user_output_cfg[srcId].percent_creg.last_presented_state = (T_ACI_CREG_STAT)status;
    mod_lac_cid = &ati_user_output_cfg[srcId].percent_creg.mod_lac_cid;
    break;

#if defined (GPRS) AND defined (DTI)
  case(PlusCGREG_CMD):
    ati_gprs_user_output_cfg[srcId].plus_cgreg.last_presented_state = (T_CGREG_STAT)status;
    mod_lac_cid = &ati_gprs_user_output_cfg[srcId].plus_cgreg.mod_lac_cid;
    break;

  case(PercentCGREG_CMD):
    ati_gprs_user_output_cfg[srcId].percent_cgreg.last_presented_state = (T_P_CGREG_STAT)status;
    mod_lac_cid = &ati_gprs_user_output_cfg[srcId].percent_cgreg.mod_lac_cid;
    break;
#endif /* GPRS */

  default:
    return;
  }

  if( lac NEQ NOT_PRESENT_16BIT )
  {
    mod_lac_cid->last_presented_lac = lac;
  }
  if( cid NEQ NOT_PRESENT_16BIT )
  {
    mod_lac_cid->last_presented_cid = cid;
  }

}

/* function sends notification to user about network registration status.
Functionnality centralised for either +CREG/+CGREG/%CGREG... */
GLOBAL void r_plus_percent_CREG  ( UBYTE                 srcId,
                                   int                   status,
                                   USHORT                lac,
                                   USHORT                cid,
                                   T_ACI_CREG_CMD        cmd, 
                                   T_ACI_P_CREG_GPRS_IND gprs_ind,
                                   U8              rt,
                                   BOOL                  bActiveContext )
{
  T_ATI_CREG_MOD        mode;
  T_ATI_REG_MOD_LAC_CID mod_lac_cid;
  USHORT                previous_lac,
                        previous_cid;
  BOOL                  network_status_has_changed = FALSE;
  BOOL                  cell_has_changed           = FALSE;
  CHAR                  loc_info[LOC_INFO_STRLTH] = ""; /* enough to get e.g. ",A1F4, F4D0" */
  CHAR                  coverage_info[COVERAGE_INFO_STRLTH] = "";
  CHAR                  *ctxtInfo = "";
  CHAR                  gprs_info[GPRS_INFO_STRLTH] = ""; /* enough to get e.g. ", 2" */
  T_ACI_NTW_STATUS      raw_status;
  CHAR                  cmd_key[KEY];
  USHORT                presented_lac = NOT_PRESENT_16BIT,
                        presented_cid = NOT_PRESENT_16BIT;

  TRACE_FUNCTION("r_plus_percent_CREG()");

  /* check whether cell has changed */
  network_status_has_changed = get_registration_data_among_cmd( srcId,
                                                                status,
                                                                &mod_lac_cid,
                                                                &raw_status,
                                                                cmd_key,
                                                                cmd );
  if( network_status_has_changed )
  {
    TRACE_EVENT_P3("cmd: %d, source: %d, new network status: %d !", cmd, srcId, status);
  }

  mode            = mod_lac_cid.pres_mode;
  previous_lac    = mod_lac_cid.last_presented_lac;
  previous_cid    = mod_lac_cid.last_presented_cid;

  /* check whether cell has changed */
  if( ((lac NEQ NOT_PRESENT_16BIT) AND (lac NEQ previous_lac))
      OR
      ((cid NEQ NOT_PRESENT_16BIT) AND (cid NEQ previous_cid)) )
  {
    if(raw_status EQ STAT_Reg  OR  raw_status EQ STAT_Roam)
    {
      /* cell info is only relevant when registered or in roaming */
      TRACE_EVENT_P2("cell has changed !, lac: %04X, cId: %04X", lac, cid);
      cell_has_changed = TRUE;
    }
  }

  /* comma always needed */
  if (cmd EQ PercentCREG_CMD OR
      mode EQ CREG_MOD_LOC_INF_ON_CTXACT)
  {
    strcat(loc_info,",,");
  }

  /* check if presentation should occur */
  switch( mode )
  {
    case(CREG_MOD_NotPresent):
    case(CREG_MOD_OFF):
      /* no presentation to user */
      return;

    case(CREG_MOD_ON):
      if( cmd EQ PercentCREG_CMD )
      {
        sprintf(loc_info, ", , , %d, %d", gprs_ind, rt);
      }
      else if( !network_status_has_changed )
      {
        return;
      }
      /* network status is available and has changed */
      break;

    case(CREG_MOD_LOC_INF_ON_CTXACT):
    case(CREG_MOD_LOC_INF_ON):
      if( (raw_status EQ STAT_Reg OR
           raw_status EQ STAT_Roam)   AND

           cell_has_changed )
      {
        presented_lac = lac;
        presented_cid = cid;
      }
      else if( (raw_status EQ STAT_Reg OR
                raw_status EQ STAT_Roam)   AND

                network_status_has_changed )
      {
        /* show last presented lac/cid */
        presented_lac = mod_lac_cid.last_presented_lac;
        presented_cid = mod_lac_cid.last_presented_cid;
      }
      else if( network_status_has_changed )
      {
        /* mobile is not registered: do not show location information
        parameters have already been initialized, so no need to set them */
      }
      else
      {
        if( cmd NEQ PercentCREG_CMD )
        {
          TRACE_EVENT_P3("%s presentation denied. mode: %d, network status: %d", cmd_key, mode, status);
          return;
        }  
        
      }

      if( presented_lac NEQ NOT_PRESENT_16BIT AND
          presented_cid NEQ NOT_PRESENT_16BIT )
      {
        sprintf(loc_info, ",\"%04X\",\"%04X\"", presented_lac, presented_cid);
      }
      else if ( mode EQ CREG_MOD_LOC_INF_ON_CTXACT )
      {
        sprintf(loc_info, ",,");
      }
      else
      {
        /* else no location info to be shown */
        if( cmd EQ PercentCREG_CMD )
        {
          sprintf(loc_info, ",,");          
        }
      }

      if ( cmd EQ PercentCREG_CMD )
      {
        sprintf(coverage_info, ",%d,%d",gprs_ind,rt);
        strcat(loc_info, coverage_info);
      }
      break;
  }

#ifdef GPRS
  if ( (cmd  EQ PercentCGREG_CMD) AND 
       (mode EQ CREG_MOD_LOC_INF_ON_CTXACT) )
  {
    if( bActiveContext EQ TRUE )
      ctxtInfo = ",1";
    else
      ctxtInfo = ",0";
  }
  sprintf(g_sa,"%s%d%s%s%s", cmd_key, status, loc_info, gprs_info, ctxtInfo );
#endif /* GPRS */

  if( (cmd EQ PercentCREG_CMD) OR (cmd EQ CREG_CMD) )
  {
    sprintf(g_sa,"%s%d%s", cmd_key, status, loc_info);
  }


  io_sendIndication(srcId, g_sa, ATI_FORCED_OUTPUT);

  update_last_presented_datas_among_cmd( srcId,
                                         status,
                                         presented_lac,
                                         presented_cid,
                                         cmd );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCREG       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCREG call backs(change in network registration status)

*/

GLOBAL void rCI_PlusCREG  ( T_ACI_CREG_STAT status,
                            USHORT          lac,
                            USHORT          cid )
{
  UBYTE srcId = srcId_cb;
  U8    rt      = 0;


  TRACE_FUNCTION("rCI_PlusCREG()");

  r_plus_percent_CREG  ( srcId,
                         status,
                         lac,
                         cid,
                         CREG_CMD,
                         P_CREG_GPRS_Support_Unknown, /*ACI-SPR-17218: ignored*/ 
                         rt,
                         FALSE );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCREG    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCREG call backs(change in network registration status)

*/

GLOBAL void rCI_PercentCREG  ( T_ACI_CREG_STAT       status,
                               USHORT                lac,
                               USHORT                cid,
                               T_ACI_P_CREG_GPRS_IND gprs_ind,
                               U8              rt)
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentCREG()");

  r_plus_percent_CREG  ( srcId,
                         status,
                         lac,
                         cid,
                         PercentCREG_CMD, /*ACI-SPR-17218: introduced for %CREG*/  
                         gprs_ind,
                         rt,
                         TRUE );
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCRING      |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCRING call back

*/
/* should be called when ringing indications should stop */
GLOBAL void ati_stop_ring(void)
{
  TRACE_FUNCTION("ati_stop_ring()");

  at.rngPrms.rngCnt = 1;
  at.rngPrms.isRng  = FALSE;
  at.S[1] = 0; /* S1 register will hold the ring counts */
  
  /* Reset the CLIP parameters to default values */
  at.clipPrms.stat = CLIP_STAT_NotPresent;
  memset( at.clipPrms.number, 0, sizeof(at.clipPrms.number));
  memset(&at.clipPrms.type, 0, sizeof(T_ACI_TOA));
  at.clipPrms.validity = MNCC_PRES_NOT_PRES;
  memset( at.clipPrms.subaddr, 0, sizeof(at.clipPrms.subaddr));
  memset(&at.clipPrms.satype, 0, sizeof(T_ACI_TOS));
#ifdef NO_ASCIIZ
  memset(&at.clipPrms.alpha, 0, sizeof(T_ACI_PB_TEXT));
#else
  memset(at.clipPrms.alpha, 0, sizeof(at.clipPrms.alpha));
#endif

  TIMERSTOP( ACI_TRING );
}

GLOBAL void rCI_PlusCRING ( /*UBYTE srcId,*/
                            T_ACI_CRING_MOD mode,
                            T_ACI_CRING_SERV_TYP type1,
                            T_ACI_CRING_SERV_TYP type2 )
{
  static T_ACI_CMD_SRC first_ringing_srcId = CMD_SRC_NONE;  /* Source which will handle the ringing-timer */
  UBYTE            srcId           = srcId_cb,
                       autoAnswerSrcId = at.rngPrms.srcID_S0;

  T_ACI_AT_CMD     autoanswer_command;
  T_ACI_RETURN     ret;
  int              DTR=0;
  UBYTE            DTR_behaviour = DTR_BEHAVIOUR_Ignore;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCRING()");

  if (first_ringing_srcId EQ CMD_SRC_NONE)
    first_ringing_srcId = (T_ACI_CMD_SRC)srcId;    /* catch the first source id for setting up the ring timer */

  /* save ringing parameter at the first call to rCI_PlusCRING */
  if( !at.rngPrms.isRng       AND
       at.rngPrms.rngCnt EQ 1 AND     /* this is only done on the first ring, not on any subsequent*/
       srcId EQ first_ringing_srcId)  /* only first source will reach the setting of "at.rngPrms.isRng = TRUE" */
  {
    at.rngPrms.mode  = mode;
    at.rngPrms.type1 = type1;
    at.rngPrms.type2 = type2;
    at.rngPrms.isRng = TRUE;
  }

  if (!at.rngPrms.isRng)
  {
    return; /* dont emmit any "RING" but escape here if ringing has been stopped
               in a multi-source configuration on any source in rhe RAT_CRING loop */
  }

  at.S[1] = at.rngPrms.rngCnt; /* S1 register will return the ring counts */

  /* check for automatic reject of an network initiated context activation */
#if defined (GPRS) AND defined (DTI)
  if( at.rngPrms.mode EQ CRING_MOD_Gprs   AND
      first_ringing_srcId EQ srcId            AND       /* it is sufficent to check this only for one ringing source */
      cmhSM_call_reject(at.rngPrms.rngCnt, mode) )
  {
    TRACE_EVENT_P2("PDP context is to be automatically rejected: RING count: %d, srcId %d", at.rngPrms.rngCnt, srcId_cb);

    ati_stop_ring();

    ret=sAT_PlusCGANS((T_ACI_CMD_SRC)srcId, 0, NULL, GPRS_CID_OMITTED);
    if( ret EQ AT_CMPL )
      return;

    TRACE_EVENT_P1("auto reject return error: %d", ret);
    cmdAtError(atError);
    return;
  }
#endif

#ifdef UART  
  if (src_params->src_type EQ ATI_SRC_TYPE_UART)
  {
    T_ACI_DTI_PRC *uart_infos;

    uart_infos = find_element (uart_src_params,
                               (UBYTE)srcId,
                               cmhUARTtest_srcId);

    if (BITFIELD_CHECK (uart_infos->data_cntr, UART_DTI_SA_BIT))
      DTR=1;
  }
#endif
#if defined (FF_PSI) AND defined (DTI)
  if (src_params->src_type EQ ATI_SRC_TYPE_PSI)
  {
    T_ACI_DTI_PRC_PSI *psi_infos;

    psi_infos = find_element (psi_src_params,
                               (UBYTE)srcId,
                               cmhPSItest_srcId);

    if (BITFIELD_CHECK (psi_infos->data_cntr, PSI_DTI_SA_BIT))
      DTR=1;
  }
#endif

  qAT_AndD ( (T_ACI_CMD_SRC)srcId, &DTR_behaviour);

  if(DTR EQ 0 AND
     DTR_behaviour EQ DTR_BEHAVIOUR_ClearCall)
  {
     /* V.250 6.2.9 Circuit 108 (Data terminal ready) behaviour:
        Automatic answer is disabled while circuit 108/2 remains off. */
    ;
    /*  Also don't emit any RING while 108/2 remains off */
  }
  else
  {
    if (at.rngPrms.rngCnt EQ 1 OR
        !check_should_buffer_ind(src_params))      /* Ring indications are not buffered, except of the first one */
    {
      if(ati_user_output_cfg[srcId].CRC_stat)
      {
        switch(mode)
        {
          case CRING_MOD_Direct:
            sprintf(g_sa,"+CRING: %s",ring_stat(at.rngPrms.type1));
            break;
          case CRING_MOD_Alternate:
            sprintf(g_sa,"+CRING: %s/%s",ring_stat(at.rngPrms.type1),ring_stat(at.rngPrms.type2));
            break;
#if defined (GPRS) AND defined (DTI)
          case CRING_MOD_Gprs:
            sprintf(g_sa,"+CRING: %s", cmhSM_ring_gprs_par());
            break;
#endif  /* GPRS AND DTI */
        }
      }
      else
      {
        strcpy(g_sa, cmdAtError(atRing));
        cmdErrStr = NULL;
      }

      io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }

    /* check for automatic answer */
#ifdef GPRS
    if( at.rngPrms.mode EQ CRING_MOD_Gprs )
    {
      autoAnswerSrcId = (UBYTE)(at.rngPrms.srcID_CGAUTO NEQ (char)NOT_PRESENT_8BIT ?
                        at.rngPrms.srcID_CGAUTO : at.rngPrms.srcID_S0);
    }
#endif

    if( srcId EQ autoAnswerSrcId                AND  /* Caution !!!                                  */
        at.rngPrms.type1 NEQ CRING_SERV_TYP_Fax      AND  /* Never auto answer FAX calls !!!              */
        at.rngPrms.type1 NEQ CRING_SERV_TYP_AuxVoice AND  /* Instead continue ringing!                    */
#if defined (GPRS) AND defined (DTI)
        cmhSM_call_answer(at.rngPrms.rngCnt, at.rngPrms.mode) EQ TRUE
#else /* GPRS */
        (at.S[0] AND at.rngPrms.rngCnt >= at.S[0])
#endif /* GPRS */
      )
    {
      TRACE_EVENT_P2("Call is to be automatically answered: RING count: %d, srcId %d", at.rngPrms.rngCnt, srcId_cb);

      ati_stop_ring();

#if defined (GPRS) AND defined (DTI)
      if ( at.rngPrms.mode EQ CRING_MOD_Gprs )
      {
        ret = sAT_PlusCGANS((T_ACI_CMD_SRC)autoAnswerSrcId, 1, "PPP", INVALID_CID);
        autoanswer_command = AT_CMD_CGANS;
      }
      else
#endif
      {
        ret = sAT_A((T_ACI_CMD_SRC)autoAnswerSrcId);
        autoanswer_command = AT_CMD_A;
      }

      if( ret EQ AT_EXCT )
      {
        src_params->curAtCmd = autoanswer_command;
        return;
      }

      TRACE_EVENT_P1("auto answer return error: %d", ret);
      cmdAtError(atError);
      return;
    }
  }

  if( first_ringing_srcId EQ srcId AND    /* only the first source will start the ringing timer */
      at.rngPrms.isRng             AND    /* ringing is still valid? */
      at.rngPrms.rngCnt EQ 1       )      /* since Timer is self-retriggering */
  {
    TRACE_EVENT_P1("start RING timer on Source: %d", srcId);
    PTIMERSTART( TRING_VALUE, TRING_VALUE, ACI_TRING );
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCRING_OFF  |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCRING off call back

*/

GLOBAL void rCI_PlusCRING_OFF ( /*UBYTE srcId,*/ SHORT cId )
{

  TRACE_FUNCTION("rCI_PlusCRING_OFF()");

  if( at.rngPrms.isRng )
  {
    ati_stop_ring();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentSIMREM  |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentSIMREM call back

*/

GLOBAL void rCI_PercentSIMREM ( /*UBYTE srcId,*/ T_ACI_SIMREM_TYPE srType )
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentSIMREM()");

  if (ati_user_output_cfg[srcId].SIMIND_stat)
  {
    sprintf (g_sa, "%s%d", "%SIMREM: ", srType);
    io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentSIMINS  |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentSIMINS call back

*/

GLOBAL void rCI_PercentSIMINS ( /*UBYTE srcId,*/ T_ACI_CME_ERR err )
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentSIMINS()");

  if (ati_user_output_cfg[srcId].SIMIND_stat)
  {
    sprintf (g_sa, "%s%d", "%SIMINS: ", err);
    io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentKSIR    |
+--------------------------------------------------------------------+

  PURPOSE : handles PercentKSIR call back.
            Code is in aci_util.c to share the code with BAT
*/
GLOBAL void rCI_PercentKSIR ( /*UBYTE srcId,*/ T_ACI_KSIR * ksStat )
{
  TRACE_FUNCTION("rCI_PercentKSIR()");
  
  utl_cb_percentKSIR (srcId_cb, ksStat);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCLIR       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCLIR call back

*/

GLOBAL void rCI_PlusCLIR  ( /*UBYTE srcId,*/
                            T_ACI_CLIR_MOD  mode,
                            T_ACI_CLIR_STAT stat)
{
  UBYTE srcId = srcId_cb;

  sprintf(g_sa,"+CLIR: %d,%d",mode,stat);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}



/*
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI             MODULE  : ACI_RET                    |
| STATE   : code                ROUTINE : rCI_PercentCSQ             |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCSQ call back

  Shen,Chao

*/
#ifdef FF_PS_RSSI
GLOBAL void rCI_PercentCSQ(UBYTE rssi, UBYTE ber, UBYTE actlevel, UBYTE min_access_level)
#else
GLOBAL void rCI_PercentCSQ(UBYTE rssi, UBYTE ber, UBYTE actlevel)
#endif
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentCSQ()");

#ifdef FF_PS_RSSI
  sprintf (g_sa, "%s %d, %d, %d, %d", "%CSQ: ",rssi, ber, actlevel, min_access_level);
#else
  sprintf (g_sa, "%s %d, %d, %d", "%CSQ: ",rssi, ber, actlevel);
#endif
  io_sendIndication(srcId, g_sa, ATI_FORCED_OUTPUT);

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusClip       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCLIP call back

*/
#ifdef NO_ASCIIZ
GLOBAL void rCI_PlusCLIP  ( /*UBYTE srcId,*/
                            T_ACI_CLIP_STAT stat,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            U8              validity,
                            CHAR          * subaddr,
                            T_ACI_TOS     * satype,
                            T_ACI_PB_TEXT * alpha)
#else  /* ifdef NO_ASCIIZ */
GLOBAL void rCI_PlusCLIP  ( /*UBYTE srcId,    */
                            T_ACI_CLIP_STAT stat,
                            CHAR*           number,
                            T_ACI_TOA*      type,
                            U8              validity,
                            CHAR*           subaddr,
                            T_ACI_TOS*      satype,
                            CHAR*           alpha)
#endif /* ifdef NO_ASCIIZ */
{
  char*  me                          = "+CLIP: ";
  SHORT  pos                         = 0;
  CHAR   cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT lenCvtdAlpha;
  UBYTE  *uniAlpha = NULL;
  UBYTE  uniAlphaLen = 0;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PLusCLIP()");
  
  /* save CLIP parameter */

  at.clipPrms.stat = stat;
  if ( number )
    memcpy(at.clipPrms.number, number, strlen(number));
  if ( type )
    memcpy(&at.clipPrms.type, type, sizeof(T_ACI_TOA));
  at.clipPrms.validity = validity;
  if ( subaddr )
    memcpy(at.clipPrms.subaddr, subaddr, strlen(subaddr));
  if ( satype )
    memcpy(&at.clipPrms.satype, satype, sizeof(T_ACI_TOS));
#ifdef NO_ASCIIZ
  if ( alpha )
    memcpy(&at.clipPrms.alpha, alpha, sizeof(T_ACI_PB_TEXT));
#else
  if ( alpha )
    memcpy(at.clipPrms.alpha, alpha, strlen(alpha));
#endif

#ifdef NO_ASCIIZ
  if ( alpha NEQ NULL )
  {
    uniAlpha = alpha->data;
    uniAlphaLen = alpha->len;
  }
#else  /* ifdef NO_ASCIIZ */
  if ( alpha NEQ NULL )
  {
    uniAlpha = (UBYTE *)alpha;
    uniAlphaLen = strlen(alpha);
  }
#endif /* ifdef NO_ASCIIZ */

  pos = sprintf(g_sa,"%s",me);

  if ( stat EQ CLIP_STAT_NotPresent )       /*Function call when connection is made*/
  {
    if ( ati_user_output_cfg[srcId].CLIP_stat
/*         AND
         (
           io_getIoMode ( ) EQ IO_MODE_CMD OR
           io_getIoMode ( ) EQ IO_MODE_RUN
         )*/
       )
    {
      if (number)
      {
        pos += sprintf(g_sa+pos,"\"%s\",",number);
        if (type)
          pos += sprintf(g_sa+pos,"%d,",toa_merge(*type));
        else
          pos += sprintf(g_sa+pos,",");
      }
      else
        pos += sprintf(g_sa+pos,"\"\",128,");
      if (subaddr)
      {
        pos += sprintf(g_sa+pos,"\"%s\",",subaddr);
        if (satype)
          pos += sprintf(g_sa+pos,"%d,",tos_merge(*satype));
        else
          pos += sprintf(g_sa+pos,",");
      }
      else if (alpha)
        pos += sprintf(g_sa+pos,"\"\",,");
      else if (validity NEQ MNCC_PRES_NOT_PRES)
        pos += sprintf(g_sa+pos,",");

      if (alpha)
      {
        utl_chsetFromSim ( uniAlpha,
                           uniAlphaLen,
                           (UBYTE*)cvtdAlpha,
                           sizeof(cvtdAlpha),
                           &lenCvtdAlpha,
                           GSM_ALPHA_Def );
        pos+=sprints(g_sa+pos,cvtdAlpha,lenCvtdAlpha);
      }
      else if (validity NEQ MNCC_PRES_NOT_PRES)
        pos += sprintf(g_sa+pos,",");

      if (validity NEQ MNCC_PRES_NOT_PRES)
        pos += sprintf(g_sa+pos,",%d",validity);

      ci_remTrailCom(g_sa, pos);
      io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }
  }
  else                                  /* Answer for qA_PlusCLIP*/
  {
    src_params->curAtCmd = AT_CMD_NONE;

    sprintf(g_sa+pos,"%d,%d",ati_user_output_cfg[srcId].CLIP_stat,stat);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCdip       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCDIP call back

*/
GLOBAL void rCI_PlusCDIP  (CHAR          * number,
                            T_ACI_TOA     * type,
                            CHAR          * subaddr,
                            T_ACI_TOS     * satype)
{
  char*  me                          = "+CDIP: ";
  SHORT  pos                         = 0;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PlusCDIP()");

  pos = sprintf(g_sa,"%s",me);

  if ( ati_user_output_cfg[srcId].CDIP_stat)
  {
    if (number)
    {
      pos += sprintf(g_sa+pos,"\"%s\",",number);
      if (type)
      {
        pos += sprintf(g_sa+pos,"%d,",toa_merge(*type));

        if (subaddr)   /* Only check to display subaddr if Number and Type are present */
        {
          pos += sprintf(g_sa+pos,"\"%s\",",subaddr);
          if (satype)
              pos += sprintf(g_sa+pos,"%d",tos_merge(*satype));
        }
      }
      else  /* No Number Type present */
      {
        pos += sprintf(g_sa+pos,",");
      }
    }
    else    /* No Number present */
    {
      pos += sprintf(g_sa+pos,"\"\",128,");
    }

    ci_remTrailCom(g_sa, pos); 
    io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCOLP       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCOLP call back

*/
#ifdef NO_ASCIIZ
GLOBAL void rCI_PlusCOLP  ( /*UBYTE srcId,*/
                            T_ACI_COLP_STAT stat,
                            CHAR            *number,
                            T_ACI_TOA       *type,
                            CHAR            *subaddr,
                            T_ACI_TOS       *satype,
                            T_ACI_PB_TEXT   *alpha)
#else  /* ifdef NO_ASCIIZ */
GLOBAL void rCI_PlusCOLP  ( /*UBYTE srcId,    */
                            T_ACI_COLP_STAT stat,
                            CHAR            *number,
                            T_ACI_TOA       *type,
                            CHAR            *subaddr,
                            T_ACI_TOS       *satype,
                            CHAR            *alpha)
#endif /* ifdef NO_ASCIIZ */
{
  char*  me                          = "+COLP: ";
  int    pos                         = 0;
  CHAR   cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT lenCvtdAlpha;
  UBYTE  *uniAlpha = NULL;
  UBYTE  uniAlphaLen = 0;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PLusCOLP()");

#ifdef NO_ASCIIZ
  if ( alpha NEQ NULL )
  {
    uniAlpha = alpha->data;
    uniAlphaLen = alpha->len;
  }
#else  /* ifdef NO_ASCIIZ */
  if ( alpha NEQ NULL )
  {
    uniAlpha = (UBYTE *)alpha;
    uniAlphaLen = strlen(alpha);
  }
#endif /* ifdef NO_ASCIIZ */

  pos = sprintf(g_sa,"%s",me);
  if (stat EQ COLP_STAT_NotPresent) /* Function call if connection is made*/
  {
    if ( at.flags.COLP_stat
/*         AND
         (
           io_getIoMode ( ) EQ IO_MODE_CMD OR
           io_getIoMode ( ) EQ IO_MODE_RUN
         ) */
       )
    {
      if (number)
      {
        pos += sprintf(g_sa+pos,"\"%s\",",number);
        if (type)
          pos += sprintf(g_sa+pos,"%d,",toa_merge(*type));
        else
          pos += sprintf(g_sa+pos,"128,");
      }
      else
        pos += sprintf(g_sa+pos,"\"\",128,");

      if (subaddr)
      {
        pos += sprintf(g_sa+pos,"\"%s\",",subaddr);
        if (satype)
          pos += sprintf(g_sa+pos,"%d,",tos_merge(*satype));
        else  pos += sprintf(g_sa+pos,",");
      }
      else
        pos += sprintf(g_sa+pos,",,");

      if (alpha)
      {
        utl_chsetFromSim ( uniAlpha,
                           uniAlphaLen,
                           (UBYTE*)cvtdAlpha,
                           sizeof(cvtdAlpha),
                           &lenCvtdAlpha,
                           GSM_ALPHA_Def );
        pos+=sprints(g_sa+pos,cvtdAlpha,lenCvtdAlpha);
      }

      ci_remTrailCom(g_sa,(USHORT)pos);
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }
  }
  else /* Answer for qAT_PlusCOLP*/
  {
   src_params->curAtCmd = AT_CMD_NONE;

   if (at.flags.COLP_stat)
     sprintf(g_sa+pos,"1,%d",stat);
    else
     sprintf(g_sa+pos,"0,%d",stat);

    ci_remTrailCom(g_sa,(USHORT)strlen(g_sa));
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCOLR    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCOLR call back

*/

GLOBAL void rCI_PercentCOLR  ( /*UBYTE srcId,*/ T_ACI_COLR_STAT stat)
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentCOLR()");

  sprintf(g_sa, "%s: %d", "%COLR", stat);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_DR             |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_DR call back

*/

GLOBAL void rCI_PlusDR    ( /*UBYTE srcId,*/ T_ACI_DR_TYP type )
{
  UBYTE srcId = srcId_cb;


  TRACE_FUNCTION("rCI_PLusDR()");

  if ( ati_user_output_cfg[srcId].DR_stat
/*       AND
       (
         io_getIoMode ( ) EQ IO_MODE_CMD OR
         io_getIoMode ( ) EQ IO_MODE_RUN
       )*/
     )
  {
    sprintf(g_sa,"+DR: %d",type);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_CR             |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_CR call back

*/

GLOBAL void rCI_PlusCR    ( /*UBYTE srcId,*/ T_ACI_CRING_SERV_TYP service )
{
  UBYTE srcId = srcId_cb;


  TRACE_FUNCTION("rCI_PLusCR()");


  if ( ati_user_output_cfg[srcId].CR_stat
/*       AND
       (
         io_getIoMode ( ) EQ IO_MODE_CMD OR
         io_getIoMode ( ) EQ IO_MODE_RUN
       ) */
     )
  {
    sprintf(g_sa,"+CR: %s",ring_stat(service));
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCOPS       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCOPS call back

*/
GLOBAL  void  rCI_PlusCOPS(/*UBYTE srcId, */
                            SHORT lastIdx,
                            T_ACI_COPS_OPDESC * operLst)
{
  TRACE_FUNCTION("rCI_PLusCOPS()");

  r_Plus_Percent_COPS(AT_CMD_COPS, lastIdx, operLst);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCOPS       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PecentCOPS call back

*/
GLOBAL  void  rCI_PercentCOPS(/*UBYTE srcId, */
                            SHORT lastIdx,
                            T_ACI_COPS_OPDESC * operLst)
{
  TRACE_FUNCTION("rCI_PercentCOPS()");

  r_Plus_Percent_COPS(AT_CMD_P_COPS, lastIdx, operLst);

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_Plus_Percent_COPS       |
+--------------------------------------------------------------------+

  PURPOSE : common functioality for rCI_PlusCOPS and rCI_PercentCOPS

*/
LOCAL  void  r_Plus_Percent_COPS  ( T_ACI_AT_CMD cmd,
                                     SHORT lastIdx,
                                     T_ACI_COPS_OPDESC * operLst)
{
  char          *format;
  USHORT i;
  BOOL          END_OF_LIST = FALSE;
  T_ACI_RETURN  ret;
  SHORT         startIdx=0;
  int           pos=0;
  UBYTE srcId = srcId_cb;

  char longName[MAX_ALPHA_OPER_LEN] = { '\0' };
  char shrtName[MAX_ALPHA_OPER_LEN] = { '\0' };

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("r_Plus_Percent_COPS()");



  if (lastIdx < 0)   /* no operLst available */
  {
    TRACE_EVENT("No operator list available");
    return;
  }

  if (lastIdx < MAX_OPER)
  {
    if(cmd EQ AT_CMD_P_COPS)
    {
       pos=sprintf(g_sa,"%s","%COPS: ");
  }
    else
    {
      pos=sprintf(g_sa,"%s","+COPS: ");
    }
     
  }

  src_params->curAtCmd = AT_CMD_NONE;

  for(i=0; i < MAX_OPER; i++)
  {
    if (operLst[i].status < 0)
    {
      END_OF_LIST = TRUE;
      break;
    }
    
    format = ",(%d,\"%s\",\"%s\",\"%s\")";
    if (i EQ 0)
      format++;   /* the first recond begins without a comma */

    if (operLst[i].pnn EQ Read_EONS)
    {
      if (operLst[i].long_len)
      {
        switch (operLst[i].long_ext_dcs>>4 & 0x07)
        {
          case 0x00:  /* GSM default alphabet */
            utl_cvtPnn7To8((UBYTE *)operLst[i].longOper,
                                    operLst[i].long_len,
                                    operLst[i].long_ext_dcs,
                           (UBYTE *)longName);
            break;
          case 0x01:  /* UCS2 */
            TRACE_ERROR ("Unhandled UCS2 in PNN entry");
            break;
          default:
            TRACE_ERROR ("Unknown DCS in PNN entry");
            break;
        }
      }
      if (operLst[i].shrt_len)
      {
        switch (operLst[i].shrt_ext_dcs>>4 & 0x07)
        {
          case 0x00:  /* GSM default alphabet */
            utl_cvtPnn7To8((UBYTE *)operLst[i].shortOper,
                                    operLst[i].shrt_len,
                                    operLst[i].shrt_ext_dcs,
                           (UBYTE *)shrtName);
            break;
          case 0x01:  /* UCS2 */
            TRACE_ERROR ("ERROR: Unhandled UCS2");
            break;
          default:
            TRACE_ERROR ("ERROR: Unknown DCS");
            break;
        }
      }
      pos+=sprintf(g_sa+pos, format, operLst[i].status, longName, shrtName, operLst[i].numOper);
    }
    else
    {
      pos+=sprintf(g_sa+pos, format, operLst[i].status, operLst[i].longOper, operLst[i].shortOper, operLst[i].numOper);
    }

    /* Beware of overwriting of g_sa */
#ifdef _SIMULATION_   /* Option for line by line output in simulation */
    if (pos > 1)                  /* line-by-line in simulation */
    {
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      g_sa[0]='\0';
      pos=0;
    }
#else
    if ((unsigned)pos > sizeof(g_sa)-80)    /* a single plmn should not exceed 80 bytes */
    {
      io_sendMessageEx(srcId, g_sa, ATI_ECHO_OUTPUT);
      g_sa[0]='\0';
      pos=0;
    }
#endif /* _SIMULATION_ */

  }

  if (i EQ MAX_PLMN_ID)
    END_OF_LIST = TRUE;

  if (!END_OF_LIST)
  {
    startIdx=lastIdx+1;
    do
    {
      if(cmd EQ AT_CMD_P_COPS)
      {
        ret=tAT_PercentCOPS((T_ACI_CMD_SRC)srcId,startIdx,&lastIdx,&operLst[0]);
      }
      else
      {
        ret=tAT_PlusCOPS((T_ACI_CMD_SRC)srcId,startIdx,&lastIdx,&operLst[0]);
      }
      
      if (ret EQ AT_CMPL)
      {
        END_OF_LIST=FALSE;
        for(i=0;i<MAX_OPER;i++)
        {
          if (operLst[i].status < 0)
          {
            END_OF_LIST = TRUE;
            break;
          }
          pos+=sprintf(g_sa+pos,",(%d,\"%s\",\"%s\",\"%s\")",
                       operLst[i].status,operLst[i].longOper,
                       operLst[i].shortOper,operLst[i].numOper);

          /* Beware of overwriting of g_sa */
#ifdef _SIMULATION_
          if (pos > 1)                  /* line-by-line in simulation */
          {
            io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
            g_sa[0]='\0';
            pos=0;
          }
#else
          if ((unsigned)pos > sizeof(g_sa)-80)    /* a single plmn should not exceed 80 bytes */
          {
            io_sendMessageEx(srcId, g_sa, ATI_ECHO_OUTPUT);
            g_sa[0]='\0';
            pos=0;
          }
#endif /* _SIMULATION_ */

        }
      }
      if (ret EQ AT_EXCT)
      {
        src_params->curAtCmd = cmd;
        break;
      }
      if (ret EQ AT_FAIL)
      {
        cmdCmeError(CME_ERR_Unknown);
        break;
      }
    }
    while(!END_OF_LIST);
  }
  if (END_OF_LIST)
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_CCWA           |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_CCWA call back

*/
#ifdef NO_ASCIIZ
GLOBAL void rCI_PlusCCWA  ( /*UBYTE srcId,*/
                            T_ACI_CLSSTAT * clsStat,
                            CHAR          * number,
                            T_ACI_TOA     * type,
                            U8              validity,
                            T_ACI_CLASS     class_type,
                            T_ACI_PB_TEXT * alpha )
#else  /* ifdef NO_ASCIIZ */
GLOBAL void rCI_PlusCCWA  (/*UBYTE srcId,     */
                           T_ACI_CLSSTAT  * clsStat,
                           CHAR           * number,
                           T_ACI_TOA      * type,
                           U8               validity,
                           T_ACI_CLASS      class_type,
                           CHAR           * alpha )
#endif /* ifdef NO_ASCIIZ */
{
  char*  me                        = "+CCWA: ";
  int    i                         = 0;
  CHAR   cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT lenCvtdAlpha;
  UBYTE  *uniAlpha = NULL;
  UBYTE  uniAlphaLen = 0;
  UBYTE srcId = srcId_cb;
  T_ACI_CLASS test_class;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PlusCCWA()");

#ifdef NO_ASCIIZ
  if ( alpha NEQ NULL )
  {
    uniAlpha = alpha->data;
    uniAlphaLen = alpha->len;
  }
#else  /* ifdef NO_ASCIIZ */
  if ( alpha NEQ NULL )
  {
    uniAlpha = (UBYTE *)alpha;
    uniAlphaLen = strlen(alpha);
  }
#endif /* ifdef NO_ASCIIZ */

  if(clsStat)          /*Callback for q_AT_PlusCCWA*/
  {

    if (clsStat->status EQ STATUS_Active)
    {
      src_params->curAtCmd = AT_CMD_NONE;
      test_class = CLASS_Vce;
      while( test_class < 2*CLASS_Fax )
      {
        if( clsStat->class_type & test_class )
        {
          TRACE_EVENT_P1("test_class: %d", test_class);

          sprintf(g_sa,"%s%d,%d", me, clsStat->status, test_class);
          io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
        }
        test_class *= CLASS_Dat; /* test next class */
      }
    }
    else
    {
      sprintf ( g_sa, "%s0,%d",me, clsStat->class_type );
      io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
    }
  }
  else                    /*call waiting indication*/
  {
    if ( at.flags.CCWA_stat
/*         AND
         (
           io_getIoMode ( ) EQ IO_MODE_CMD OR
           io_getIoMode ( ) EQ IO_MODE_RUN
         )*/
       )
    {
      i=sprintf(g_sa,"%s",me);
      if (number)
      {
        i+=sprintf(g_sa+i,"\"%s\",",number);
        if (type)
          i+=sprintf(g_sa+i,"%d,",toa_merge(*type));
        else
          i+=sprintf(g_sa+i,"128,");
      }
      else
        i+=sprintf(g_sa+i,"\"\",128,");

      if (class_type >=0)
        i+=sprintf(g_sa+i,"%d,",class_type);
      else
        i+=sprintf(g_sa+i,",");

      if (alpha)
      {
        utl_chsetFromSim ( uniAlpha,
                           uniAlphaLen,
                           (UBYTE*)cvtdAlpha,
                           sizeof(cvtdAlpha),
                           &lenCvtdAlpha,
                           GSM_ALPHA_Def );
        i+=sprints(g_sa+i,cvtdAlpha,lenCvtdAlpha);
      }

      if (validity NEQ MNCC_PRES_NOT_PRES)
      {
        i += sprintf(g_sa+i,",%d",validity);
      }

      ci_remTrailCom(g_sa, (USHORT)i);
      io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);

#ifdef ACI /* for ATI only version */
      cmhMMI_handleAudioTone ( AT_CMD_NONE, RAT_CCWA, CPI_MSG_NotPresent );
#endif
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_CCFC           |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_CCFC call back

*/

GLOBAL void rCI_PlusCCFC  (/*UBYTE srcId,*/ T_ACI_CCFC_SET* setting)
{
  char *me="+CCFC: ";
  int i=0;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCCFC()");

  src_params->curAtCmd = AT_CMD_NONE;

  if (setting)          /*Callback for qAT_PlusCCFC*/
  {
    if (setting->clsstat.status EQ STATUS_Active)
    {
      i=sprintf(g_sa,"%s",me);

      if (setting->clsstat.status NEQ STATUS_NotPresent)
        i+=sprintf(g_sa+i,"%d,",setting->clsstat.status);
      else
        i+=sprintf(g_sa+i,",");
      if (setting->clsstat.class_type NEQ CLASS_NotPresent)
        i+=sprintf(g_sa+i,"%d,",setting->clsstat.class_type);
      else
        i+=sprintf(g_sa+i,",");
      if (setting->number[0] NEQ 0x0 )
        i+=sprintf(g_sa+i,"\"%s\",",setting->number);
      else
        i+=sprintf(g_sa+i,",");
      if (setting->type.npi NEQ NPI_NotPresent )
        i+=sprintf(g_sa+i,"%d,",toa_merge(setting->type));
      else
        i+=sprintf(g_sa+i,",");
      if (setting->subaddr[0] NEQ 0x0 )
        i+=sprintf(g_sa+i,"\"%s\",",setting->subaddr);
      else
        i+=sprintf(g_sa+i,",");
      if (setting->satype.tos NEQ TOS_NotPresent )
        i+=sprintf(g_sa+i,"%d,",tos_merge(setting->satype));
      else
        i+=sprintf(g_sa+i,",");
      if (setting->time NEQ ACI_NumParmNotPresent)
        i+=sprintf(g_sa+i,"%d",setting->time);

      ci_remTrailCom(g_sa,(USHORT)strlen(g_sa));
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }
    else
    {
      sprintf ( g_sa, "%s0,%d",me, setting->clsstat.class_type );
      io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_CLCK           |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_CLCK call back

*/

GLOBAL void rCI_PlusCLCK  (/*UBYTE srcId,*/ T_ACI_CLSSTAT  * clsStat)

{
  char *me="+CLCK: ";
  UBYTE srcId = srcId_cb;
  T_ACI_CLASS test_class;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);



  TRACE_FUNCTION("rCI_PlusCLCK()");

  src_params->curAtCmd = AT_CMD_NONE;

  if(clsStat)          /*Callback for qAT_PlusCLCK*/
  {
    if (clsStat->status EQ STATUS_Active)
    {
      test_class = CLASS_Vce;

      while( test_class < 2*CLASS_Sms )
      {
        if( clsStat->class_type & test_class )
        {
          sprintf(g_sa,"%s%d,%d",me,clsStat->status,test_class);
          io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
        }
        test_class *= CLASS_Dat; /* test next class */
      }
    }
    else
    {
      sprintf ( g_sa, "%s0,%d", me, clsStat->class_type );
      io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_CRSM           |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_CRSM call back

*/

GLOBAL void rCI_PlusCRSM  ( /*UBYTE srcId,*/
                            SHORT           sw1,
                            SHORT           sw2,
                            SHORT           rspLen,
                            UBYTE          *rsp    )
{
  int    i=0;
  UBYTE srcId = srcId_cb;


  TRACE_FUNCTION("rCI_PlusCRSM()");

  i=sprintf(g_sa,"%s","+CRSM: ");

  if (sw1 NEQ ACI_NumParmNotPresent)
    i+=sprintf(g_sa+i,"%d,",sw1);
  else
    i+=sprintf(g_sa+i,",");
  if (sw2 NEQ ACI_NumParmNotPresent)
    i+=sprintf(g_sa+i,"%d,",sw2);
  else
    i+=sprintf(g_sa+i,",");
  if (rspLen AND rsp)
  {
    utl_binToHex( rsp, rspLen, g_sa+i );
  }

  ci_remTrailCom(g_sa,(USHORT)strlen(g_sa));
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_CSIM           |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_CSIM call back

*/

GLOBAL void rCI_PlusCSIM  ( /*UBYTE srcId,*/
                            SHORT           len,
                            UBYTE          *rsp    )
{
  UBYTE i;
  UBYTE srcId = srcId_cb;
  SHORT rstLen = 0;
  SHORT tmpLen;

  TRACE_FUNCTION("rCI_PlusCSIM()");

  i=sprintf(g_sa,"%s%d,","+CSIM: ",len*2);

  if (len AND rsp)
  {
    if( len*2+i > (MAX_CMD_LEN-1) ) rstLen = (len*2+i) - (MAX_CMD_LEN-1);

    tmpLen = (rstLen)?len-rstLen/2:len;

    utl_binToHex( rsp, tmpLen, g_sa+i );
    io_sendMessageEx(srcId, g_sa, (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT|ATI_BEGIN_CRLF_OUTPUT ));
    while( rstLen )
    {
      rsp    += tmpLen;
      tmpLen  = rstLen/2;

      rstLen =( tmpLen*2 > (MAX_CMD_LEN-1))?tmpLen*2 - (MAX_CMD_LEN-1):0;

      if(rstLen) tmpLen = tmpLen-rstLen/2;

      utl_binToHex( rsp, tmpLen, g_sa );
      io_sendMessageEx(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }
    g_sa[0] = '\0';
    io_sendMessageEx(srcId, g_sa, (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT|ATI_END_CRLF_OUTPUT));
  }
  else
  {
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_CSMS           |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_CSMS call back

*/

GLOBAL void rCI_PlusCSMS  ( /*UBYTE srcId,*/
                            T_ACI_CSMS_SERV service,
                            T_ACI_CSMS_SUPP mt,
                            T_ACI_CSMS_SUPP mo,
                            T_ACI_CSMS_SUPP bm)

{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  char *me="+CSMS: ";

  TRACE_FUNCTION("rCI_PlusCSMS()");

  src_params->curAtCmd = AT_CMD_NONE;
  if (service < 0)
    sprintf(g_sa,"%s%d,%d,%d",me,mt,mo,bm);
  else
    sprintf(g_sa,"%s%d,%d,%d,%d",me,service,mt,mo,bm);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_CPMS           |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_CPMS call back

*/

GLOBAL void rCI_PlusCPMS  ( /*UBYTE srcId,*/
                            T_ACI_SMS_STOR_OCC * mem1,
                            T_ACI_SMS_STOR_OCC * mem2,
                            T_ACI_SMS_STOR_OCC * mem3 )

{
  CHAR* me         = "+CPMS: ";
  CHAR  memstr1[3] = {0};
  CHAR  memstr2[3] = {0};
  CHAR  memstr3[3] = {0};
  BOOL  f1         = TRUE;
  BOOL  f2         = TRUE;
  BOOL  f3         = TRUE;
  int   i          = 0;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PlusCPMS()");

  src_params->curAtCmd = AT_CMD_NONE;

  if ( cpmsCallType EQ QAT_CALL )
  {
    for ( i=0; sms_mem[i].name NEQ NULL; i++ )
    {
      if ( ( sms_mem[i].stor EQ mem1->mem ) AND f1 )
      {
        strcpy ( memstr1, sms_mem[i].name );
        f1 = FALSE;
      }

      if ( ( sms_mem[i].stor EQ mem2->mem ) AND f2 )
      {
        strcpy ( memstr2, sms_mem[i].name );
        f2 = FALSE;
      }

      if ( ( sms_mem[i].stor EQ mem3->mem ) AND f3 )
      {
        strcpy ( memstr3, sms_mem[i].name );
        f3 = FALSE;
      }

    }
    sprintf ( g_sa, "%s\"%s\",%d,%d,\"%s\",%d,%d,\"%s\",%d,%d",
              me, memstr1, mem1->used, mem1->total,
                  memstr2, mem2->used, mem2->total,
                  memstr3, mem3->used, mem3->total );
  }
  else
  {
    sprintf ( g_sa, "%s%d,%d,%d,%d,%d,%d",
              me, mem1->used, mem1->total,
                  mem2->used, mem2->total,
                  mem3->used, mem3->total );
  }

  io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );

  cpmsCallType = NONE_CALL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCMS        |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMS call back
*/

GLOBAL void rCI_PlusCMS ( /*UBYTE srcId,*/ T_ACI_AT_CMD cmdId, T_ACI_CMS_ERR err,
                         T_EXT_CMS_ERROR *conc_error)
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PLusCMS()");

  ledit_ctrl (srcId,LEDIT_CTRL_CMPL, NULL);

  if ( curAbrtCmd EQ AT_CMD_NONE )
  {
    src_params->curAtCmd    = AT_CMD_NONE;
    io_sendConfirm(srcId, cmdCmsError(err), ATI_NORMAL_OUTPUT);
    cmdErrStr   = NULL;
  }
  else
  {
    curAbrtCmd = AT_CMD_NONE;
    io_sendMessage ( srcId, cmdCmsError ( CMS_ERR_FailedToAbort ),
                           (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT | ATI_RESULT_CODE_OUTPUT));
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCBM        |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCBM call back
*/

GLOBAL void rCI_PlusCBM   ( T_MMI_CBCH_IND * mmi_cbch_ind )
{

  UBYTE srcId = srcId_cb;
  T_ATI_IO_MODE  ioMode;
#if defined (SMS_PDU_SUPPORT)
  T_ACI_CMGF_MOD mode;
#endif

  TRACE_FUNCTION("rCI_PlusCBM()");

  /* Check if the source invoking this function is the same as the one 
   * interested in SMS indications */ /* Issue 25033 */
  if (srcId NEQ smsShrdPrm.smsSrcId)
    return;

  ioMode = ati_get_mode( srcId_cb );

  if ( !cnmiFlushInProgress
       AND (ioMode NEQ ATI_UNKN_MODE)
       AND ( (at.CNMI_mode EQ CNMI_MOD_Buffer)
             OR
             ((ioMode EQ ATI_DATA_MODE) AND (at.CNMI_mode EQ CNMI_MOD_BufferAndFlush)) ) )
  {
    T_CNMI_IND ind;

    memcpy (&ind.cbm, mmi_cbch_ind, sizeof(T_MMI_CBCH_IND));
    cmd_addCnmiNtry ( CNMI_CBM, &ind );
  }
  else if ( cnmiFlushInProgress             OR
            ( ioMode EQ ATI_CMD_MODE        AND
              at.CNMI_mode NEQ CNMI_MOD_Buffer ) )
#if defined (SMS_PDU_SUPPORT)
  {
    /*
     * request current mode
     */
    qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
    if (mode EQ 0)
    {
      /*
       * handle PDU mode
       */
      rCI_PlusCBMPdu (mmi_cbch_ind);
    }
    else
    {
      /*
       * handle Text mode
       */
      rCI_PlusCBMText (mmi_cbch_ind);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCBM        |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCBM call back
*/

LOCAL void rCI_PlusCBMText   ( T_MMI_CBCH_IND * mmi_cbch_ind )
#endif  /* SMS_PDU_SUPPORT */
{
  USHORT pos;
  USHORT lenCvtdData             = 0;
  CHAR   cvtdData[2*MAX_CBM_LEN] = {0x00};
  UBYTE  srcId = srcId_cb;
  USHORT         sn;       /* serial number               */
  USHORT         mid;      /* message identifier          */
  UBYTE          dcs;      /* data coding scheme          */
  UBYTE          page;     /* actual page number          */
  UBYTE          pages;    /* total number of pages       */
  T_ACI_CBM_DATA msg;      /* cell broadcast message data */

  TRACE_FUNCTION("rCI_PLusCBMText()");


  /*
   *-----------------------------------------------------------------
   * process parameters for new message indication
   *-----------------------------------------------------------------
   */
  sn    = ( ( SHORT )mmi_cbch_ind->cbch_msg[0] << 8 ) +
                     mmi_cbch_ind->cbch_msg[1];
  mid   = ( ( SHORT )mmi_cbch_ind->cbch_msg[2] << 8 ) +
                     mmi_cbch_ind->cbch_msg[3];
  dcs   = mmi_cbch_ind->cbch_msg[4];
  page  = ( mmi_cbch_ind->cbch_msg[5] & 0xF0 ) >> 4;
  pages = ( mmi_cbch_ind->cbch_msg[5] & 0x0F );

  /*
   *-----------------------------------------------------------------
   * process message data, expanding from 7 to 8 bit
   *-----------------------------------------------------------------
   */
  cmhSMS_expdSmsCb ( dcs,
                     &mmi_cbch_ind->cbch_msg[CBCH_HEAD_LEN],
                     (UBYTE)(mmi_cbch_ind->cbch_len - CBCH_HEAD_LEN),
                     msg.data, &msg.len );

  pos=sprintf(g_sa,"+CBM: %d,%d,%d,%d,%d",sn,mid,dcs,page,pages);
  io_sendIndication(srcId, g_sa, ATI_FORCED_OUTPUT);
  if (msg.len > 0)
  {
    pos = 0;

    utl_cbmDtaToTe((UBYTE*)msg.data,
                   msg.len,
                   (UBYTE*)cvtdData,
                   sizeof(cvtdData),
                   &lenCvtdData,
                   0,
                   (UBYTE)dcs);

    pos+=sprintq(g_sa+pos,cvtdData,lenCvtdData);

    io_sendMessageEx(srcId, g_sa, (T_ATI_OUTPUT_TYPE)(ATI_INDICATION_OUTPUT +
                                ATI_FORCED_OUTPUT +
                                ATI_END_CRLF_OUTPUT));
  }
}
#if !defined (SMS_PDU_SUPPORT)
} /* rCI_PlusCBM */
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCDS        |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCDS call back
*/

GLOBAL void rCI_PlusCDS   ( T_MNSMS_STATUS_IND * mnsms_status_ind )
{

  UBYTE srcId = srcId_cb;
  T_ATI_IO_MODE  ioMode;
#if defined (SMS_PDU_SUPPORT)
  T_ACI_CMGF_MOD mode;
#endif

  TRACE_FUNCTION("rCI_PLusCDS()");

  /* Check if the source invoking this function is the same as the one 
   * interested in SMS indications */ /* Issue 25033 */
  if (srcId NEQ smsShrdPrm.smsSrcId)
    return;

  ioMode = ati_get_mode( srcId_cb );

  if ( !cnmiFlushInProgress
       AND (ioMode NEQ ATI_UNKN_MODE)
       AND ( (at.CNMI_mode EQ CNMI_MOD_Buffer)
             OR
             ((ioMode EQ ATI_DATA_MODE) AND (at.CNMI_mode EQ CNMI_MOD_BufferAndFlush)) ) )
  {
    T_CNMI_IND ind;

    memcpy ( &ind.cds, mnsms_status_ind, sizeof ( T_MNSMS_STATUS_IND) );
    cmd_addCnmiNtry ( CNMI_CDS, &ind );
  }
  else if ( cnmiFlushInProgress             OR
            ( ioMode EQ ATI_CMD_MODE        AND
              at.CNMI_mode NEQ CNMI_MOD_Buffer ) )
#if defined (SMS_PDU_SUPPORT)
  {
    /*
     * request current mode
     */
    qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
    if (mode EQ 0)
    {
      /*
       * handle PDU mode
       */
      rCI_PlusCDSPdu (mnsms_status_ind);
    }
    else
    {
      /*
       * handle Text mode
       */
      rCI_PlusCDSText (mnsms_status_ind);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCDSText    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCDS call back for text mode
*/

LOCAL void rCI_PlusCDSText   (T_MNSMS_STATUS_IND * mnsms_status_ind)
#endif /* SMS_PDU_SUPPORT */
{

  UBYTE srcId = srcId_cb;
  T_ACI_CDS_SM* p_st;
  SHORT pos   = 0;

  TRACE_FUNCTION("rCI_PLusCDSText()");



  /* is the SM already decoded ? */
  if (smsShrdPrm.pDecMsg)
    p_st = (T_ACI_CDS_SM*)smsShrdPrm.pDecMsg;
  else
  {
    ACI_MALLOC(smsShrdPrm.pDecMsg, sizeof(T_ACI_CDS_SM) );
    p_st = (T_ACI_CDS_SM*)smsShrdPrm.pDecMsg;
    cmhSMS_cpyStatInd ( p_st, mnsms_status_ind);
  }

  pos=sprintf(g_sa,"+CDS: %d,%d,",p_st->fo, p_st->msg_ref);
  if (strlen(p_st->addr))
    pos+=sprintf(g_sa+pos,"\"%s\"",p_st->addr);
  else
    pos+=sprintf(g_sa+pos,",");
  if( (p_st->toa.ton NEQ TON_NotPresent) AND (p_st->toa.npi NEQ NPI_NotPresent) )
    pos+=sprintf(g_sa+pos,",%d",toa_merge(p_st->toa));
  else
    pos+=sprintf(g_sa+pos,",");

  pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",
    p_st->vpabs_scts.year  [0], p_st->vpabs_scts.year  [1],
    p_st->vpabs_scts.month [0], p_st->vpabs_scts.month [1],
    p_st->vpabs_scts.day   [0], p_st->vpabs_scts.day   [1],
    p_st->vpabs_scts.hour  [0], p_st->vpabs_scts.hour  [1],
    p_st->vpabs_scts.minute[0], p_st->vpabs_scts.minute[1],
    p_st->vpabs_scts.second[0], p_st->vpabs_scts.second[1],
    p_st->vpabs_scts.timezone);

  pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\",%d",
    p_st->vpabs_dt.year  [0], p_st->vpabs_dt.year  [1],
    p_st->vpabs_dt.month [0], p_st->vpabs_dt.month [1],
    p_st->vpabs_dt.day   [0], p_st->vpabs_dt.day   [1],
    p_st->vpabs_dt.hour  [0], p_st->vpabs_dt.hour  [1],
    p_st->vpabs_dt.minute[0], p_st->vpabs_dt.minute[1],
    p_st->vpabs_dt.second[0], p_st->vpabs_dt.second[1],
    p_st->vpabs_dt.timezone , p_st->tp_status);

  io_sendIndication(srcId, g_sa, ATI_FORCED_OUTPUT);
}
#if !defined (SMS_PDU_SUPPORT)
} /* rCI_PlusCDS */
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCMT        |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMT call back
*/

GLOBAL void rCI_PlusCMT (T_MNSMS_MESSAGE_IND* mnsms_message_ind)
{
  UBYTE srcId = srcId_cb;
  T_ATI_IO_MODE  ioMode;
#if defined (SMS_PDU_SUPPORT)
  T_ACI_CMGF_MOD mode;
#endif

  TRACE_EVENT_P1("rCI_PlusCMT(): source Id: %d", srcId);

  /* Check if the source invoking this function is the same as the one 
   * interested in SMS indications */ /* Issue 25033 */
  if (srcId NEQ smsShrdPrm.smsSrcId)
    return;

  ioMode = ati_get_mode( srcId );
  if( ioMode EQ ATI_UNKN_MODE )
  {
    TRACE_EVENT("rCI_PlusCMT(): unused source (unknown ioMode) -> return");
    return;
  }

  if ( !cnmiFlushInProgress       AND
       ( (at.CNMI_mode EQ CNMI_MOD_Buffer)
          OR
         ( (ioMode       EQ ATI_DATA_MODE) AND 
           (at.CNMI_mode EQ CNMI_MOD_BufferAndFlush)) ) )
  { /* terminal is busy --> store data into CNMI buffer */
    T_CNMI_IND ind;

    TRACE_EVENT("rCI_PlusCMT(): terminal busy or buffer mode --> store data into CNMI buffer");
    memcpy ( &ind.cmt, mnsms_message_ind, sizeof ( T_MNSMS_MESSAGE_IND) );
    cmd_addCnmiNtry ( CNMI_CMT, &ind );
    if ( smsShrdPrm.cnma_ack_expected EQ TRUE )
    { /* the acknowledge must be send automatically, because terminal is busy */
      PALLOC (mnsms_ack_res, MNSMS_ACK_RES);

      TRACE_EVENT("rCI_PlusCMT(): send 'automatic' acknowledge (Phase2+ mode and buffer)");

      mnsms_ack_res->resp          = SMS_RP_ACK;
      mnsms_ack_res->sms_sdu.o_buf = 0;
      mnsms_ack_res->sms_sdu.l_buf = 0;

      PSENDX (SMS, mnsms_ack_res);

      smsShrdPrm.cnma_ack_expected = FALSE;
    }
  }
  else
  { /* flush in progress or terminal is not busy */
    TRACE_EVENT("rCI_PlusCMT(): flush in progress or command mode --> show +CMT on terminal");
    if ( cnmiFlushInProgress               OR
         ( (ioMode        EQ ATI_CMD_MODE) AND
           (at.CNMI_mode NEQ CNMI_MOD_Buffer) ) )
#if defined (SMS_PDU_SUPPORT)
    { /* request current mode (text or PDU) */
      if( smsShrdPrm.CNMImt EQ 0 )
      {
        TRACE_EVENT("rCI_PlusCMT(): +CNMI <mt>=0 --> no routing to terminal (error in SMS entity)");
        return;
      }
      qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
      if (mode EQ 0)
      {
        /*
         * handle PDU mode
         */
        rCI_PlusCMTPdu (mnsms_message_ind);
      }
      else
      {
        /*
         * handle Text mode
         */
        rCI_PlusCMTText (mnsms_message_ind);
      }
    }
    else
    {
      TRACE_EVENT_P3("rCI_PlusCMT(): cannot display the +CMT --> flushInProgess?: %d, ioMode: %d, CNMI Mode: % d",
                     cnmiFlushInProgress, ioMode, at.CNMI_mode);
    }
  }
}


LOCAL void rCI_PlusCMTText (T_MNSMS_MESSAGE_IND* mnsms_message_ind)
#endif  /* SMS_PDU_SUPPORT */
{
  SHORT     pos                         = 0;
  CHAR      cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT    lenCvtdAlpha;
  CHAR      cvtdData[2*MAX_SM_LEN];
  USHORT    lenCvtdData;
  UBYTE srcId = srcId_cb;
  T_ACI_CMGL_SM* p_sm;
#if defined FF_EOTD
  T_LOC_MLC_ADDRESS* lc_orig_addr_ptr;
  int i = 0;
#endif
  
  CHAR              cvtdAddr[4*MAX_SMS_ADDR_DIG];
  USHORT            lenCvtdAddr;

  TRACE_FUNCTION("rCI_PlusCMTText()");

  pos=sprintf(g_sa,"+CMT: ");

  /* is the SM already decoded ? */
  if (smsShrdPrm.pDecMsg)
    p_sm = smsShrdPrm.pDecMsg;
  else
  {
    ACI_MALLOC(smsShrdPrm.pDecMsg, sizeof(T_ACI_CMGL_SM) );
    p_sm = smsShrdPrm.pDecMsg;

    /* Implements Measure # 110 */
    cmhSMS_cpyMsgIndReadCnf (p_sm, 
                             &mnsms_message_ind->status, 
                             &mnsms_message_ind->sms_sdu,
                             mnsms_message_ind->rec_num);
  }

#if defined FF_EOTD
    memset(&lc_orig_addr,0,sizeof(T_LOC_MLC_ADDRESS));
    lc_orig_addr_ptr = &lc_orig_addr;
    lc_orig_addr_ptr->toa.npi = p_sm->toa.npi;
    lc_orig_addr_ptr->toa.ton = p_sm->toa.ton;
    if(lc_orig_addr_ptr->toa.ton EQ TON_International)
    {
          lc_orig_addr_ptr->address[i] = '+';
          i++;
    }
    strncpy(lc_orig_addr_ptr->address+i,p_sm->adress,sizeof(p_sm->adress)-(size_t)i);
#endif /* FF_EOTD */

  /* If toa is Alphanumeric then convert the GSM chars to the one used by TE chars */
    if(p_sm->toa.ton EQ TON_Alphanumeric) /* Alphanumeric Destination Address */
    {
      utl_chsetFromGsm ( (UBYTE*)p_sm->adress, (USHORT)strlen(p_sm->adress), (UBYTE*)cvtdAddr, sizeof(cvtdAddr), &lenCvtdAddr, GSM_ALPHA_Def );
      pos+=sprintf(g_sa+pos,"\"%s\",", cvtdAddr);
    }
    else
    {
      pos+=sprintf(g_sa+pos,"\"%s\",",p_sm->adress);
    }

#ifdef FF_ATI_BAT

  /*
  *   Extracting the alphanumeric data from the phonebook at this point
  *   would defeat the object (testing the BAT message). Having this
  *   data stored globally represents an easier solution to the
  *   problem of conveying it here.
  */
  if (smsShrdPrm.alpha_len)
  {
    pos+=sprints(g_sa+pos,smsShrdPrm.alpha,smsShrdPrm.alpha_len);
  }

#else

  if( p_sm->alpha.len > 0 )
  {
    utl_chsetFromGsm((UBYTE*)p_sm->alpha.data,
                     p_sm->alpha.len,
                     (UBYTE*)cvtdAlpha,
                     sizeof(cvtdAlpha),
                     &lenCvtdAlpha,
                     GSM_ALPHA_Def);
    pos+=sprints(g_sa+pos,cvtdAlpha,lenCvtdAlpha);
  }

#endif

  pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",  /* SCTS */
    p_sm->scts.year  [0], p_sm->scts.year  [1],
    p_sm->scts.month [0], p_sm->scts.month [1],
    p_sm->scts.day   [0], p_sm->scts.day   [1],
    p_sm->scts.hour  [0], p_sm->scts.hour  [1],
    p_sm->scts.minute[0], p_sm->scts.minute[1],
    p_sm->scts.second[0], p_sm->scts.second[1],
    p_sm->scts.timezone);

  if (ati_user_output_cfg[srcId].CSDH_stat)
  {
    if(strlen(p_sm->adress))
      pos+=sprintf(g_sa+pos,",%d",toa_merge(p_sm->toa));
    else
      pos+=sprintf(g_sa+pos,",");
    pos+=sprintf(g_sa+pos,",%d,%d,%d",p_sm->fo,p_sm->pid,p_sm->dcs);  /*FO, PID, DCS*/

    if(strlen(p_sm->sca))
    {
      pos+=sprintf(g_sa+pos,",\"%s\"",p_sm->sca);
      pos+=sprintf(g_sa+pos,",%d",toa_merge(p_sm->tosca));
    }
    else
      pos+=sprintf(g_sa+pos,",,");
  }

  if (((UBYTE)p_sm->fo & TP_UDHI_MASK) EQ TP_UDHI_WITH_HEADER)
  {
    T_ACI_SM_DATA data;

    aci_frmtOutput(p_sm->fo, p_sm->dcs, &data);

    utl_smDtaToTe((UBYTE*)data.data,
                  data.len,
                  (UBYTE*)cvtdData,
                  sizeof(cvtdData),
                  &lenCvtdData,
                  (UBYTE)p_sm->fo,
                  (UBYTE)p_sm->dcs);

    if (ati_user_output_cfg[srcId].CSDH_stat)
      pos+=sprintf(g_sa+pos,",%d",data.len);

    io_sendIndication(srcId, g_sa, ATI_FORCED_OUTPUT);

    pos=sprintq(g_sa,cvtdData,lenCvtdData);
    io_sendMessageEx(srcId, g_sa, (T_ATI_OUTPUT_TYPE)(ATI_INDICATION_OUTPUT +
                                ATI_FORCED_OUTPUT +
                                ATI_END_CRLF_OUTPUT));
  }
  else
  {
    if (ati_user_output_cfg[srcId].CSDH_stat)
      pos+=sprintf(g_sa+pos,",%d",p_sm->data.len);

    io_sendIndication(srcId, g_sa, ATI_FORCED_OUTPUT);

    if (p_sm->data.len > 0)
    {
      utl_smDtaToTe((UBYTE*)p_sm->data.data,
                    p_sm->data.len,
                    (UBYTE*)cvtdData,
                    sizeof(cvtdData),
                    &lenCvtdData,
                    (UBYTE)p_sm->fo,
                    (UBYTE)p_sm->dcs);
      pos=sprintq(g_sa,cvtdData,lenCvtdData);
    }
    else
    {
      g_sa[0]='\0';
    }
    io_sendMessageEx(srcId, g_sa, (T_ATI_OUTPUT_TYPE)(ATI_INDICATION_OUTPUT +
                                  ATI_FORCED_OUTPUT +
                                  ATI_END_CRLF_OUTPUT));
  }
}
#if !defined (SMS_PDU_SUPPORT)
} /* else in previous function */
} /* rCI_PlusCMT */
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCMGR       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMGR call back (Read Message)
*/

GLOBAL void rCI_PlusCMGR  ( T_MNSMS_READ_CNF* mnsms_read_cnf,
                            T_ACI_CMGR_CBM * cbm)
{
  UBYTE msg_type;
#if defined (SMS_PDU_SUPPORT)
  UBYTE srcId = srcId_cb;
  T_ACI_CMGF_MOD mode;

  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
    /*
     * handle PDU mode
     */
    rCI_Plus_Percent_CMGRPdu (mnsms_read_cnf, AT_CMD_CMGR);
  else
#endif
    /*
     * handle Text mode
     */
  {
   /* querying the type of the SM */
   cmhSMS_SMSQueryType (&mnsms_read_cnf->sms_sdu, &msg_type);

   if (msg_type EQ TP_MTI_SMS_STATUS_REP)
     rCI_Plus_Percent_CMGRTextSP (mnsms_read_cnf, AT_CMD_CMGR);
   else
     rCI_Plus_Percent_CMGRText (mnsms_read_cnf, cbm, AT_CMD_CMGR);
  }
}

LOCAL void rCI_Plus_Percent_CMGRText (T_MNSMS_READ_CNF* mnsms_read_cnf,
                                      T_ACI_CMGR_CBM * cbm,
                                      T_ACI_AT_CMD cmd )
{
  CHAR*      buf;
  SHORT      pos      = 0;

  CHAR       cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT     lenCvtdAlpha;
  CHAR       cvtdData[2*MAX_SM_LEN];
  USHORT     lenCvtdData;
  UBYTE srcId = srcId_cb;
  T_ACI_CMGL_SM* p_sm;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  char       vpenh_str[15];
  CHAR       cvtdAddr[4*MAX_SMS_ADDR_DIG];
  USHORT     lenCvtdAddr;

  TRACE_FUNCTION("rCI_Plus_Percent_CMGRText()");

  src_params->curAtCmd = AT_CMD_NONE;

  /* is the SM already decoded ? */
  if (smsShrdPrm.pDecMsg)
    p_sm = smsShrdPrm.pDecMsg;
  else
  {
    ACI_MALLOC(smsShrdPrm.pDecMsg, sizeof(T_ACI_CMGL_SM) );
    p_sm = smsShrdPrm.pDecMsg;
    /* Implements Measure # 110 */
    cmhSMS_cpyMsgIndReadCnf (p_sm, 
                             &mnsms_read_cnf->status, 
                             &mnsms_read_cnf->sms_sdu,
                             mnsms_read_cnf->rec_num);
  }

  buf=sms_status(p_sm->stat);

  if (buf) /* STATUS */
  {
    if (cmd EQ AT_CMD_CMGR)
    {
      pos=sprintf(g_sa,"+CMGR: \"%s\"",buf);
    }
    else
    {
      pos=sprintf (g_sa, " %s:  \"%s\"", "%CMGR", buf);
    }
  }
  else
  {
    if (cmd EQ AT_CMD_CMGR)
    {
      pos=sprintf(g_sa,"+CMGR: ,");
    }
    else
    {
      pos=sprintf (g_sa, " %s", "%CMGR: ,");
    }
  }

  /* If toa is Alphanumeric then convert the GSM chars to the one used by TE chars */
  if(p_sm->toa.ton EQ TON_Alphanumeric) /* Alphanumeric Destination Address */
  {
    utl_chsetFromGsm ( (UBYTE*)p_sm->adress, (USHORT)strlen(p_sm->adress), (UBYTE*)cvtdAddr, sizeof(cvtdAddr), &lenCvtdAddr, GSM_ALPHA_Def );
    pos+=sprintf(g_sa+pos,",\"%s\"", cvtdAddr);
  }
  else
  {
    pos+=sprintf(g_sa+pos,",\"%s\"",p_sm->adress); /*Destination Address*/
  }

#ifdef FF_ATI_BAT

  /*
  *   Extracting the alphanumeric data from the phonebook at this point
  *   would defeat the object (testing the BAT message). Having this
  *   data stored globally represents a quick and dirty solution to the
  *   problem of conveying it here.
  */
  if (smsShrdPrm.alpha_len)
  {
    pos+=sprints(g_sa+pos,smsShrdPrm.alpha,smsShrdPrm.alpha_len);
  }
  else
  {
    pos+=sprintf(g_sa+pos,",");
  }

#else

  if (p_sm->alpha.len > 0) /*ALPHA*/
  {
    pos+=sprintf(g_sa+pos,",");
    utl_chsetFromGsm((UBYTE*)p_sm->alpha.data,
                     p_sm->alpha.len,
                     (UBYTE*)cvtdAlpha,
                     sizeof(cvtdAlpha),
                     &lenCvtdAlpha,
                     GSM_ALPHA_Def);
    pos+=sprints(g_sa+pos,cvtdAlpha,lenCvtdAlpha);
  }
  else
    pos+=sprintf(g_sa+pos,",");

#endif /*FF_ATI_BAT*/

  if ((p_sm->fo & TP_MTI_MASK) EQ TP_MTI_SMS_DELIVER)
  {
    pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",
        p_sm->scts.year  [0], p_sm->scts.year  [1],
        p_sm->scts.month [0], p_sm->scts.month [1],
        p_sm->scts.day   [0], p_sm->scts.day   [1],
        p_sm->scts.hour  [0], p_sm->scts.hour  [1],
        p_sm->scts.minute[0], p_sm->scts.minute[1],
        p_sm->scts.second[0], p_sm->scts.second[1],
        p_sm->scts.timezone);
  }

  if(ati_user_output_cfg[srcId].CSDH_stat)
  {
    if(strlen(p_sm->adress)) /*TOA Destination*/
      pos+=sprintf(g_sa+pos,",%d",toa_merge(p_sm->toa));
    else
      pos+=sprintf(g_sa+pos,",");
    pos+=sprintf(g_sa+pos,",%d,%d,%d",p_sm->fo,p_sm->pid,p_sm->dcs); /*FO, PID, DCS*/

    if ((p_sm->fo & TP_MTI_MASK) EQ TP_MTI_SMS_SUBMIT)
    {
      if ((p_sm->fo & TP_VPF_MASK) EQ TP_VPF_ABSOLUTE)
      {
        pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",
          p_sm->scts.year  [0], p_sm->scts.year  [1],
          p_sm->scts.month [0], p_sm->scts.month [1],
          p_sm->scts.day   [0], p_sm->scts.day   [1],
          p_sm->scts.hour  [0], p_sm->scts.hour  [1],
          p_sm->scts.minute[0], p_sm->scts.minute[1],
          p_sm->scts.second[0], p_sm->scts.second[1],
          p_sm->scts.timezone);
      }
      else if ((p_sm->fo & TP_VPF_MASK) EQ TP_VPF_ENHANCED)
      {
        aci_encodeVpenh ( vpenh_str, &p_sm->vp_enh );
        pos+=sprintf(g_sa+pos,",\"%s\"",vpenh_str);
      }
      else if ((p_sm->fo & TP_VPF_MASK) EQ TP_VPF_RELATIVE)
        pos+=sprintf(g_sa+pos,",%d",p_sm->vp_rel);
      else
        pos+=sprintf(g_sa+pos,",");
    }
    if(strlen(p_sm->sca)) /*Destination Address*/
    {
      pos+=sprintf(g_sa+pos,",\"%s\"",p_sm->sca);
      pos+=sprintf(g_sa+pos,",%d",toa_merge(p_sm->tosca));
    }
    else
      pos+=sprintf(g_sa+pos,",,");
  }

  if (((UBYTE)p_sm->fo & TP_UDHI_MASK) EQ TP_UDHI_WITH_HEADER)
  {
    T_ACI_SM_DATA data;

    aci_frmtOutput (p_sm->fo, p_sm->dcs, &data);

    utl_smDtaToTe((UBYTE*)data.data,
                  data.len,
                  (UBYTE*)cvtdData,
                  sizeof(cvtdData),
                  &lenCvtdData,
                  (UBYTE)p_sm->fo,
                  (UBYTE)p_sm->dcs);

    if(ati_user_output_cfg[srcId].CSDH_stat)
      pos+=sprintf(g_sa+pos,",%d",data.len);

    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

    pos=sprintq(g_sa,cvtdData,lenCvtdData);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    if(ati_user_output_cfg[srcId].CSDH_stat)
      pos+=sprintf(g_sa+pos,",%d",p_sm->data.len);

    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

    if (p_sm->data.len > 0)
    {
      utl_smDtaToTe( (UBYTE*)p_sm->data.data,
                     p_sm->data.len,
                     (UBYTE*)cvtdData,
                     sizeof(cvtdData),
                     &lenCvtdData,
                     (UBYTE)p_sm->fo,
                     (UBYTE)p_sm->dcs);
      pos=sprintq(g_sa,cvtdData,lenCvtdData);
    }
    else /* empty SMS passed */
    {
      g_sa[0]='\0';
    }
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCMTI       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMTI call back
*/

GLOBAL void rCI_PlusCMTI   ( /*UBYTE srcId,*/ T_ACI_SMS_STOR mem, UBYTE index)
{
  UBYTE srcId = srcId_cb;
  char      mem_str[3];
  SHORT     i;
  T_ATI_IO_MODE ioMode;

  /* Check if the source invoking this function is the same as the one 
   * interested in SMS indications */ /* Issue 25033 */
  if (srcId NEQ smsShrdPrm.smsSrcId)
    return;

  TRACE_EVENT_P1("rCI_PlusCMTI(): source Id: %d", srcId);
  ioMode = ati_get_mode( srcId_cb );

  if ( !cnmiFlushInProgress
       AND (ioMode NEQ ATI_UNKN_MODE)
       AND ( (at.CNMI_mode EQ CNMI_MOD_Buffer)
             OR
             ((ioMode EQ ATI_DATA_MODE) AND (at.CNMI_mode EQ CNMI_MOD_BufferAndFlush)) ) )
  {
    T_CNMI_IND ind;

    ind.cmti.mem   = mem;
    ind.cmti.index = index;

    cmd_addCnmiNtry ( CNMI_CMTI, &ind );
  }
  else if ( cnmiFlushInProgress             OR
            ( ioMode EQ ATI_CMD_MODE        AND
              at.CNMI_mode NEQ CNMI_MOD_Buffer ) )
  {
    for (i=0;sms_mem[i].name NEQ 0;i++)
    {
      if (sms_mem[i].stor EQ mem)
      {
        strcpy(mem_str,sms_mem[i].name);
        break;
      }
    }
    sprintf(g_sa,"+CMTI: \"%s\",%d",mem_str,index);
    io_sendIndication(srcId, g_sa, ATI_FORCED_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCMGS       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMGS call back
*/

GLOBAL void rCI_PlusCMGS (T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf)
#if defined (SMS_PDU_SUPPORT)
{
  UBYTE srcId = srcId_cb;

  T_ACI_CMGF_MOD mode;

  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
    /*
     * handle PDU mode
     */
    rCI_PlusCMGSPdu (mnsms_submit_cnf);
  else
    /*
     * handle Text mode
     */
    rCI_PlusCMGSText (mnsms_submit_cnf);
}

LOCAL void rCI_PlusCMGSText ( T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf )
#endif
{
  SHORT pos = 0;
  T_ACI_VP_ABS scts;
  UBYTE srcId = srcId_cb;
  UBYTE *buf;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCMGSText()");

  src_params->curAtCmd = AT_CMD_NONE;

  pos=sprintf(g_sa,"+CMGS: %d", mnsms_submit_cnf->tp_mr);

  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
  {
    if (mnsms_submit_cnf->sms_sdu.l_buf)
    {
      /* skip SCA in SIM-PDU, buf points to SMS-SUBMIT-REPORT */
      buf = mnsms_submit_cnf->sms_sdu.buf + mnsms_submit_cnf->sms_sdu.buf[0] + 1;

      /* advance to TP-SCTS */
      if (mnsms_submit_cnf->cause EQ SMS_NO_ERROR)
        buf += 2;  /* offset in SMS-SUBMIT-REPORT for RP-ACK */
      else
        buf += 3;  /* offset in SMS-SUBMIT-REPORT for RP-ERROR */

      cmh_unpackSCTS (&scts, buf);

      pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",  /* SCTS */
      scts.year  [0], scts.year  [1],
      scts.month [0], scts.month [1],
      scts.day   [0], scts.day   [1],
      scts.hour  [0], scts.hour  [1],
      scts.minute[0], scts.minute[1],
      scts.second[0], scts.second[1],
      scts.timezone);
    }
  }

  io_sendMessageEx( srcId, g_sa,(T_ATI_OUTPUT_TYPE)
                    (ATI_NORMAL_OUTPUT | ATI_BEGIN_CRLF_OUTPUT | ATI_END_CRLF_OUTPUT));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCMSS       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMSS call back
*/

GLOBAL void rCI_PlusCMSS (T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf)
#if defined (SMS_PDU_SUPPORT)
{
  UBYTE srcId = srcId_cb;

  T_ACI_CMGF_MOD mode;

  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
    /*
     * handle PDU mode
     */
    rCI_PlusCMSSPdu (mnsms_submit_cnf);
  else
    /*
     * handle Text mode
     */
    rCI_PlusCMSSText (mnsms_submit_cnf);
}

LOCAL void rCI_PlusCMSSText ( T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf )
#endif
{
  SHORT pos = 0;
  T_ACI_VP_ABS scts;
  UBYTE srcId = srcId_cb;
  UBYTE *buf;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCMSSText()");

  src_params->curAtCmd = AT_CMD_NONE;

  pos=sprintf(g_sa,"+CMSS: %d", mnsms_submit_cnf->tp_mr);

  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
  {
    if (mnsms_submit_cnf->sms_sdu.l_buf)
    {
      /* skip SCA in SIM-PDU, buf points to SMS-SUBMIT-REPORT */
      buf = mnsms_submit_cnf->sms_sdu.buf + mnsms_submit_cnf->sms_sdu.buf[0] + 1;

      /* advance to TP-SCTS */
      if (mnsms_submit_cnf->cause EQ SMS_NO_ERROR)
        buf += 2;  /* offset in SMS-SUBMIT-REPORT for RP-ACK */
      else
        buf += 3;  /* offset in SMS-SUBMIT-REPORT for RP-ERROR */

      cmh_unpackSCTS (&scts, buf);

      pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",  /* SCTS */
      scts.year  [0], scts.year  [1],
      scts.month [0], scts.month [1],
      scts.day   [0], scts.day   [1],
      scts.hour  [0], scts.hour  [1],
      scts.minute[0], scts.minute[1],
      scts.second[0], scts.second[1],
      scts.timezone);
    }
  }

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCMGW       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMGW call back
*/

GLOBAL void rCI_PlusCMGW ( /*UBYTE srcId,*/ UBYTE index)
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PLusCMGW()");

  src_params->curAtCmd = AT_CMD_NONE;

  sprintf(g_sa,"+CMGW: %d",index);
  io_sendMessageEx( srcId, g_sa,(T_ATI_OUTPUT_TYPE)
                    (ATI_NORMAL_OUTPUT | ATI_BEGIN_CRLF_OUTPUT | ATI_END_CRLF_OUTPUT));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCMGC       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMGC call back
*/

GLOBAL void rCI_PlusCMGC (T_MNSMS_COMMAND_CNF * mnsms_command_cnf)
#if defined (SMS_PDU_SUPPORT)
{
  UBYTE srcId = srcId_cb;

  T_ACI_CMGF_MOD mode;

  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
    /*
     * handle PDU mode
     */
    rCI_PlusCMGCPdu (mnsms_command_cnf);
  else
    /*
     * handle Text mode
     */
    rCI_PlusCMGCText (mnsms_command_cnf);
}

LOCAL void rCI_PlusCMGCText ( T_MNSMS_COMMAND_CNF * mnsms_command_cnf )
#endif
{
  SHORT pos = 0;
  T_ACI_VP_ABS scts;
  UBYTE srcId = srcId_cb;
  UBYTE *buf;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCMGCText()");

  src_params->curAtCmd = AT_CMD_NONE;

  pos=sprintf(g_sa,"+CMGC: %d", mnsms_command_cnf->tp_mr);

  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
  {
    if (mnsms_command_cnf->sms_sdu.l_buf)
    {
      /* skip SCA in SIM-PDU, buf points to SMS-STATUS-REPORT */
      buf = mnsms_command_cnf->sms_sdu.buf + mnsms_command_cnf->sms_sdu.buf[0] + 1;

      /* advance to to TP-RA */
      buf += 2;

      /* skip variable TP-RA and advance to TP-SCTS */
      buf += (*buf+1)/2 + 2;  /* length is in BCD digits! TON/NPI is also skipped */

      cmh_unpackSCTS (&scts, buf);

      pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",  /* SCTS */
      scts.year  [0], scts.year  [1],
      scts.month [0], scts.month [1],
      scts.day   [0], scts.day   [1],
      scts.hour  [0], scts.hour  [1],
      scts.minute[0], scts.minute[1],
      scts.second[0], scts.second[1],
      scts.timezone);
    }
  }

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCMGRS      |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCMGRS call back
*/

GLOBAL void rCI_PercentCMGRS (UBYTE mode,
                              T_MNSMS_RETRANS_CNF * mnsms_retrans_cnf,
                              T_MNSMS_SEND_PROG_IND * mnsms_send_prog_ind)
#if defined (SMS_PDU_SUPPORT)
{
  UBYTE srcId = srcId_cb;

  T_ACI_CMGF_MOD mgf_mode;

  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mgf_mode);
  if (mgf_mode EQ CMGF_MOD_Pdu)
  {    
     rCI_PercentCMGRSPdu (mode, mnsms_retrans_cnf, mnsms_send_prog_ind);   
  }
  else
  {   
     rCI_PercentCMGRSText (mode, mnsms_retrans_cnf, mnsms_send_prog_ind);   
  }
  return;
}

LOCAL void rCI_PercentCMGRSText ( UBYTE mode,
                                  T_MNSMS_RETRANS_CNF * mnsms_retrans_cnf,
                                  T_MNSMS_SEND_PROG_IND * mnsms_send_prog_ind )
#endif
{
  SHORT pos = 0;
  T_ACI_VP_ABS scts;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PercentCMGRSText()");

  if (mode EQ CMGRS_MODE_MANUAL_RETRANS AND mnsms_retrans_cnf NEQ NULL)
  {
    src_params->curAtCmd = AT_CMD_NONE;
    pos=sprintf(g_sa,"%s: 2,%d", "%CMGRS", mnsms_retrans_cnf->tp_mr);  

    if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
    {
      cmh_unpackBCD ((UBYTE*)&scts, &mnsms_retrans_cnf->sms_sdu.buf[2], 7);

      pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",  /* SCTS */
      scts.year  [0], scts.year  [1],
      scts.month [0], scts.month [1],
      scts.day   [0], scts.day   [1],
      scts.hour  [0], scts.hour  [1],
      scts.minute[0], scts.minute[1],
      scts.second[0], scts.second[1],
      scts.timezone);
    }
  }
  else if (mode EQ CMGRS_MODE_ENABLE_AUTO_RETRANS AND mnsms_send_prog_ind NEQ NULL)
  {
    pos=sprintf(g_sa,"%s: 1,%d,%d", "%CMGRS", 
                                    mnsms_send_prog_ind->resend_count,
                                    mnsms_send_prog_ind->max_retrans);
  }
  else
  {
    TRACE_ERROR("%CMGRS: wrong combination of parameters");
    return;
  }

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return;
}
#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusILRR       |
+--------------------------------------------------------------------+

  PURPOSE : handles CI_PlusILRR call back
*/

GLOBAL void rCI_PlusILRR( /*UBYTE srcId, */
                          T_ACI_BS_SPEED  speed,
                          T_ACI_BS_FRM    format,
                          T_ACI_BS_PAR    parity)

{
  LONG val;
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PlusILRR()");

  if ( ati_user_output_cfg[srcId].ILRR_stat )
  {
    switch(speed)
    {
      case(BS_SPEED_300_V110):   val=  300; break;
      case(BS_SPEED_1200_V110):  val= 1200; break;
      case(BS_SPEED_2400_V110):  val= 2400; break;
      case(BS_SPEED_4800_V110):  val= 4800; break;
      case(BS_SPEED_9600_V110):  val= 9600; break;
      case(BS_SPEED_14400_V110): val=14400; break;
      case(BS_SPEED_19200_V110): val=19200; break;
      case(BS_SPEED_38400_V110): val=38400; break;
      default:                   val=    1; break;
    }
    sprintf(g_sa,"+ILRR: %d,%d,%d",val,format,parity);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentBC      |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_OK call back

*/

GLOBAL void rCI_PercentBC (BYTE segm);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentDRV     |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_OK call back

*/

GLOBAL void rCI_PercentDRV( T_ACI_DRV_DEV device,
                            T_ACI_DRV_FCT function,
                            UBYTE         val1,
                            UBYTE         val2);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCMGL       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMGL call back

*/

GLOBAL void rCI_PlusCMGL  (T_MNSMS_READ_CNF *mnsms_read_cnf)
{
  UBYTE msg_type;
#if defined (SMS_PDU_SUPPORT)
  UBYTE srcId = srcId_cb;
  T_ACI_CMGF_MOD mode;

  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
    /*
     * handle PDU mode
     */
    rCI_Plus_Percent_CMGLPdu (mnsms_read_cnf, AT_CMD_CMGL);
  else
#endif
    /*
     * handle Text mode
     */
  {
   /* querying the type of the SM */
   cmhSMS_SMSQueryType (&mnsms_read_cnf->sms_sdu, &msg_type);

   if (msg_type EQ TP_MTI_SMS_STATUS_REP)
     rCI_Plus_Percent_CMGLTextSP (mnsms_read_cnf, AT_CMD_CMGL);
   else
     rCI_Plus_Percent_CMGLText (mnsms_read_cnf, AT_CMD_CMGL);
  }
}


LOCAL void rCI_Plus_Percent_CMGLText (T_MNSMS_READ_CNF *mnsms_read_cnf,
                                      T_ACI_AT_CMD cmd)
{
  SHORT  pos = 0;
  CHAR*  buf;

  CHAR   cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT lenCvtdAlpha;
  CHAR   cvtdData[2*MAX_SM_LEN];
  USHORT lenCvtdData;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  T_ACI_CMGL_SM* p_sm;
  CHAR              cvtdAddr[4*MAX_SMS_ADDR_DIG];
  USHORT            lenCvtdAddr;

  TRACE_FUNCTION("rCI_Plus_Percent_CMGLText()");

  src_params->curAtCmd = AT_CMD_NONE;


  /* is the SM already decoded ? */
  if (smsShrdPrm.pDecMsg)
    p_sm = smsShrdPrm.pDecMsg;
  else
  {
    ACI_MALLOC(smsShrdPrm.pDecMsg, sizeof(T_ACI_CMGL_SM) );
    p_sm = smsShrdPrm.pDecMsg;
    /* Implements Measure # 110 */
    cmhSMS_cpyMsgIndReadCnf (p_sm, 
                             &mnsms_read_cnf->status, 
                             &mnsms_read_cnf->sms_sdu,
                             mnsms_read_cnf->rec_num);
  }

  buf = sms_status ( p_sm->stat );

  if (cmd EQ AT_CMD_CMGL )
  {
    pos = sprintf ( g_sa, "+CMGL: %d", p_sm->msg_ref);
  }
  else
  {
    pos=sprintf (g_sa, " %s:  %d", "%CMGL", p_sm->msg_ref);
  }
  if ( buf )
  {
    pos += sprintf ( g_sa + pos, ",\"%s\"", buf );   /* STATUS */
  }
  else
  {
    pos += sprintf ( g_sa + pos, "," );
  }

  if (p_sm->stat EQ SMS_STAT_Invalid)
  {
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT); /* emit "INVALID MESSAGE" only */
    return;
  }

  if(p_sm->toa.ton EQ TON_Alphanumeric) /* Alphanumeric Destination Address */
  {
    utl_chsetFromGsm ( (UBYTE*)p_sm->adress, (USHORT)strlen(p_sm->adress), (UBYTE*)cvtdAddr, sizeof(cvtdAddr), &lenCvtdAddr, GSM_ALPHA_Def );
    pos += sprintf ( g_sa + pos, ",\"%s\",", cvtdAddr);
  }
  else
  {
    pos += sprintf ( g_sa + pos, ",\"%s\",", p_sm->adress);
  }

#ifdef FF_ATI_BAT

  /*
  *   Extracting the alphanumeric data from the phonebook at this point
  *   would defeat the object (testing the BAT message). Having this
  *   data stored globally represents a quick and dirty solution to the
  *   problem of conveying it here.
  */
  if (smsShrdPrm.alpha_len)
  {
    pos+=sprints(g_sa+pos,smsShrdPrm.alpha,smsShrdPrm.alpha_len);
  }

#else

  if ( p_sm->alpha.len NEQ 0 )
  {
    utl_chsetFromGsm ( (UBYTE*)p_sm->alpha.data,
                       p_sm->alpha.len,
                       (UBYTE*)cvtdAlpha,
                       sizeof(cvtdAlpha),
                       &lenCvtdAlpha,
                       GSM_ALPHA_Def );
    pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
  }

#endif /*FF_ATI_BAT*/

  if ((p_sm->fo & TP_MTI_MASK) NEQ TP_MTI_SMS_DELIVER)
  {
    pos += sprintf ( g_sa + pos, "," );
  }
  else
  {
    pos += sprintf ( g_sa + pos, ",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",
      p_sm->scts.year  [0], p_sm->scts.year  [1],
      p_sm->scts.month [0], p_sm->scts.month [1],
      p_sm->scts.day   [0], p_sm->scts.day   [1],
      p_sm->scts.hour  [0], p_sm->scts.hour  [1],
      p_sm->scts.minute[0], p_sm->scts.minute[1],
      p_sm->scts.second[0], p_sm->scts.second[1],
      p_sm->scts.timezone);
  }

  if (((UBYTE)p_sm->fo & TP_UDHI_MASK) EQ TP_UDHI_WITH_HEADER)
  {
    T_ACI_SM_DATA data;

    aci_frmtOutput(p_sm->fo, p_sm->dcs, &data);

    utl_smDtaToTe((UBYTE*)data.data,
                  data.len,
                  (UBYTE*)cvtdData,
                  sizeof(cvtdData),
                  &lenCvtdData,
                  (UBYTE)p_sm->fo,
                  (UBYTE)p_sm->dcs);

    if(ati_user_output_cfg[srcId].CSDH_stat)
      pos += sprintf ( g_sa + pos, ",%d,%d",toa_merge ( p_sm->toa ), data.len );

    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

    pos=sprintq(g_sa,cvtdData,lenCvtdData);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    if(ati_user_output_cfg[srcId].CSDH_stat)
      pos += sprintf ( g_sa + pos, ",%d,%d",
                       toa_merge ( p_sm->toa ), p_sm->data.len );

    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

    if (p_sm->data.len > 0)
    {
      utl_smDtaToTe( (UBYTE*)p_sm->data.data,
                     p_sm->data.len,
                     (UBYTE*)cvtdData,
                     sizeof(cvtdData),
                     &lenCvtdData,
                     (UBYTE)p_sm->fo,
                     (UBYTE)p_sm->dcs);
      pos=sprintq(g_sa,cvtdData,lenCvtdData);
    }
    else
    {
      g_sa[0]='\0';
    }
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCSSI       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCSSI call back

*/
GLOBAL void rCI_PlusCSSI  ( T_ACI_CSSI_CODE code,
                            SHORT           index)
{
  SHORT pos=0;
  char *me="+CSSI: ";
  UBYTE srcId = srcId_cb;


  TRACE_FUNCTION("rCI_PLusCSSI()");

  if ( ati_user_output_cfg[srcId].CSSI_stat
/*       AND
       (
         io_getIoMode ( ) EQ IO_MODE_CMD OR
         io_getIoMode ( ) EQ IO_MODE_RUN
       )   */
     )
  {

    pos = sprintf(g_sa,"%s",me);

    if (code NEQ CSSI_CODE_NotPresent)   pos += sprintf(g_sa+pos,"%d,",code);
    else  pos += sprintf(g_sa+pos,",");
    if (index NEQ ACI_NumParmNotPresent) sprintf(g_sa+pos,"%d",index);

    ci_remTrailCom(g_sa,(USHORT)strlen(g_sa));
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCSSU       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCSSU call back

*/

GLOBAL void rCI_PlusCSSU  ( /*UBYTE srcId,*/
                            T_ACI_CSSU_CODE code,
                            SHORT           index,
                            CHAR           *number,
                            T_ACI_TOA      *type,
                            CHAR           *subaddr,
                            T_ACI_TOS      *satype)
{
  char *me="+CSSU: ";
  SHORT pos=0;
  UBYTE srcId = srcId_cb;


  TRACE_FUNCTION("rCI_PLusCSSU()");

  if ( ati_user_output_cfg[srcId].CSSU_stat
/*       AND
       (
         io_getIoMode ( ) EQ IO_MODE_CMD OR
         io_getIoMode ( ) EQ IO_MODE_RUN
       )  */
     )
  {

    pos = sprintf(g_sa,"%s",me);
    if (code NEQ CSSU_CODE_NotPresent  )   pos += sprintf(g_sa+pos,"%d,",code);
    else  pos += sprintf(g_sa+pos,",");
    if (index NEQ ACI_NumParmNotPresent) pos += sprintf(g_sa+pos,"%d,",index);
    else  pos += sprintf(g_sa+pos,",");
    if (number)
    {
      pos += sprintf(g_sa+pos,"\"%s\",",number);
      if (type)
        pos += sprintf(g_sa+pos,"%d,",toa_merge(*type));
      else
        pos += sprintf(g_sa+pos,"128,");
    }
    else
      pos += sprintf(g_sa+pos,"\"\",128,");

    if (subaddr)
    {
      pos += sprintf(g_sa+pos,"\"%s\",",subaddr);
      if (satype)
        pos += sprintf(g_sa+pos,"%d",tos_merge(*satype));
    }

    ci_remTrailCom(g_sa,(USHORT)strlen(g_sa));
    io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCUSD       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCUSD call back

*/

GLOBAL void rCI_PlusCUSD  ( /*UBYTE srcId,*/
                            T_ACI_CUSD_MOD   m,
                            T_ACI_USSD_DATA  *ussd,
                            SHORT            dcs)
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCUSD()");

  src_params->curAtCmd = AT_CMD_NONE;

  if ( ati_user_output_cfg[srcId].CUSD_stat )
  {
    if (ussd NEQ NULL)
    {
#ifdef FF_ATI_BAT
      rci_display_USSD( srcId, m, ussd->data, ussd->len, FALSE, dcs );
#else
      rci_display_USSD( srcId, m, ussd->data, ussd->len, CONVERT_STRING, dcs );
#endif
    }
    else
    {
#ifdef FF_ATI_BAT
      rci_display_USSD( srcId, m, NULL, 0, FALSE, dcs );
#else
      rci_display_USSD( srcId, m, NULL, 0, CONVERT_STRING, dcs );
#endif
    }

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCIMI       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCIMI call back

*/

GLOBAL void rCI_PlusCIMI  ( /*UBYTE srcId,*/ CHAR *imsi)
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PlusCIMI()");

  src_params->curAtCmd = AT_CMD_NONE;
  if (imsi)
  {
    sprintf(g_sa,"%s",imsi);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCNUM       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCNUM call back

*/

GLOBAL void rCI_PlusCNUM ( /*UBYTE srcId,*/ T_ACI_CNUM_MSISDN *msisdn,
                           UBYTE             num )
{
  UBYTE        count = 0;
  UBYTE        i;
  T_ACI_RETURN ret;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PlusCNUM()");

  for ( i = 0; i < MAX_MSISDN; i++ )
  {
    count += aci_writeCnumMsisdn ( srcId, &msisdn[i] );
  }

  if ( count EQ MAX_MSISDN )
  {
    ret = qAT_PlusCNUM ( (T_ACI_CMD_SRC)srcId, CNUM_MOD_NextRead ) ;

    if ( ret EQ AT_EXCT )
    {
      src_params->curAtCmd    = AT_CMD_CNUM;
    }
    else if ( ret EQ AT_CMPL )
    {
      src_params->curAtCmd    = AT_CMD_NONE;
      cmdErrStr   = NULL;
    }
    else
    {
      src_params->curAtCmd    = AT_CMD_NONE;
      cmdErrStr   = NULL;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCPOL       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCPOL call back

*/
GLOBAL void rCI_PlusCPOL  ( /*UBYTE srcId,*/ SHORT              startIdx,
                            SHORT              lastIdx,
                            T_ACI_CPOL_OPDESC *operLst,
                            SHORT              usdNtry )
{
  UBYTE idx;
  BOOL  loop;
  UBYTE srcId = srcId_cb;


  TRACE_FUNCTION("rCI_PlusCPOL()");

  loop     = TRUE;

  /* if oper list is present, reply to read command */
  if( operLst )
  {
    do
    {
      for( idx=0; idx < MAX_OPER; idx++ )
      {
        if( operLst[idx].index EQ ACI_NumParmNotPresent )
        {
          loop = FALSE;
          break;
        }
        sprintf(g_sa,"+CPOL: %d,%d,\"%s\"", operLst[idx].index,
                                          operLst[idx].format,
                                          operLst[idx].oper );
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      }

      if( lastIdx EQ ACI_NumParmNotPresent ) break;

      startIdx = lastIdx+1;

#ifdef WIN32
      if( qAT_PlusCPOL(srcId, startIdx, &lastIdx, operLst,
                       cpolMode )
#else
      if( qAT_PlusCPOL((T_ACI_CMD_SRC)srcId, startIdx, &lastIdx, operLst,
                       CPOL_MOD_NotPresent )
#endif
          EQ AT_FAIL OR !loop)
        break;

    }
    while( loop );
  }
  /* else, reply to test command */
  else
  {
    sprintf(g_sa,"+CPOL: (1-%d),(0-2)", lastIdx);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCLAN       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCLAN call back

*/
GLOBAL void rCI_PlusCLAN  ( T_ACI_LAN_SUP  *CLang)

{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PlusCLAN()");

  sprintf(g_sa,"+CLAN: %s",CLang->str );
  io_sendMessage(srcId,g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCLAE       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCLAE call back

*/
GLOBAL void rCI_PlusCLAE  (T_ACI_LAN_SUP  *CLang)

{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PlusCLAE()");

  sprintf(g_sa,"+CLAV: %s",CLang->str );
  io_sendMessage(srcId,g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCCCM       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCCCM call back

*/

GLOBAL void rCI_PlusCCCM  ( /*UBYTE srcId,*/ LONG *ccm )
{
  static LONG oldCCM = 0;
  UBYTE srcId = srcId_cb;


  TRACE_FUNCTION("rCI_PlusCCCM()");

  if( *ccm < oldCCM )
  {
    allowCCCM = TRUE;
    callTime  = 0;
  }

  if (ccm AND allowCCCM AND ati_user_output_cfg[srcId].CAOC_stat)
  {
    allowCCCM = FALSE;
    sprintf(g_sa,"+CCCM: \"%6X\"",*ccm);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCCWV       |
+--------------------------------------------------------------------+

  PURPOSE : handles unsolicited result code +CCWV

*/

GLOBAL void rCI_PlusCCWV ( /*UBYTE srcId,*/ T_ACI_CCWV_CHRG charging )
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PlusCCWV()");

  if ( charging NEQ CCWV_CHRG_NotPresent )
  {
    ati_user_output_cfg[srcId].CCWV_charging = charging;
  }
  else
  {
    charging = ati_user_output_cfg[srcId].CCWV_charging;
  }

  if ( charging           EQ CCWV_CHRG_Shortage AND
       ati_user_output_cfg[srcId].CCWE_stat EQ 1               /*   AND
       (
         io_getIoMode ( ) EQ IO_MODE_CMD OR
         io_getIoMode ( ) EQ IO_MODE_RUN
       ) */
     )
  {
    io_sendIndication(srcId, "+CCWV", ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCPI     |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCPI call back

*/

GLOBAL void rCI_PercentCPI   ( /*UBYTE srcId,*/ SHORT           cId,
                              T_ACI_CPI_MSG   msgType,
                              T_ACI_CPI_IBT   ibt,
                              T_ACI_CPI_TCH   tch,
                              USHORT          cause)
{
  SHORT               pos  = 0;
  UBYTE               srcId = srcId_cb;
  SHORT               idx;
  T_ACI_CLCC_CALDESC *calLst;
  CHAR                cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT              lenCvtdAlpha;
  UBYTE               type;

  TRACE_FUNCTION("rCI_PercentCPI()");
  TRACE_EVENT_P1("msgType: %d", msgType);

#ifdef ACI
  if (ibt NEQ CPI_IBT_NotPresent)
    ibt_params.ati_currIbt = ibt;

  if (tch NEQ CPI_TCH_NotPresent)
    ibt_params.ati_currTch = tch;
#endif

  if ( (ati_user_output_cfg[srcId].CPI_stat > 0) )
  {
    pos = sprintf(g_sa,"%s: ","%CPI");

    if (cId NEQ ACI_NumParmNotPresent)
      pos += sprintf(g_sa+pos,"%d,",cId);
    else
      pos += sprintf(g_sa+pos,",");
    if (msgType NEQ CPI_MSG_NotPresent)
      pos += sprintf(g_sa+pos,"%d,",msgType);
    else
      pos += sprintf(g_sa+pos,",");
    if (ibt NEQ CPI_IBT_NotPresent)
      pos += sprintf(g_sa+pos,"%d,",ibt);
    else
      pos += sprintf(g_sa+pos,",");
    if (tch NEQ CPI_TCH_NotPresent)
      pos += sprintf(g_sa+pos,"%d",tch);
    else
      pos += sprintf(g_sa+pos,",");

    ACI_MALLOC (calLst, MAX_CALL_NR * sizeof (T_ACI_CLCC_CALDESC));
    if ((ati_user_output_cfg[srcId].CPI_stat > 1) AND
      (qAT_PlusCLCC((T_ACI_CMD_SRC)srcId, calLst) EQ AT_CMPL))
    {


      for (idx = 0; idx < MAX_CALL_NR; idx++)
      {
        if( calLst[idx].idx EQ ACI_NumParmNotPresent )
        {
          if (ati_user_output_cfg[srcId].CPI_stat > 2)
          {
            pos += sprintf(g_sa + pos,",,,,,");
          }
          break;
        }
        else
        {
          if (calLst[idx].idx NEQ cId)
          {
            /* continue; */
          }
          else /* if (calLst[idx].idx NEQ cId) */
          {
            if (calLst[idx].dir NEQ CLCC_DIR_NotPresent)
              pos += sprintf(g_sa+pos,",%d",calLst[idx].dir);
            else
              pos += sprintf(g_sa+pos,",");
            if (calLst[idx].mode NEQ CLCC_MODE_NotPresent)
              pos += sprintf(g_sa+pos,",%d",calLst[idx].mode);
            else
              pos += sprintf(g_sa+pos,",");
            if (calLst[idx].number[0] NEQ 0x0)
            {
              pos += sprintf(g_sa+pos,",\"%s\"",calLst[idx].number);
              if (calLst[idx].type.ton NEQ TON_NotPresent)
              {
                type = toa_merge(calLst[idx].type);
                pos += sprintf(g_sa+pos,",%d",(int)type);
              }
              else
                pos += sprintf(g_sa+pos,",");
#ifdef NO_ASCIIZ
              if (calLst[idx].alpha.len NEQ 0x0)
              {
                pos += sprintf(g_sa+pos,",");
                utl_chsetFromGsm ( calLst[idx].alpha.data,
                  calLst[idx].alpha.len,
                  (UBYTE*)cvtdAlpha,
                  sizeof(cvtdAlpha),
                  &lenCvtdAlpha,
                  GSM_ALPHA_Def );
                pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
              }
#else  /* #ifdef NO_ASCIIZ */
              if (calLst[idx].alpha[0] NEQ 0x0)
              {
                pos += sprintf(g_sa+pos,",");
                utl_chsetFromGsm ( (UBYTE*)calLst[idx].alpha,
                  0,
                  (UBYTE*)cvtdAlpha,
                  sizeof(cvtdAlpha),
                  &lenCvtdAlpha,
                  GSM_ALPHA_Int );
                pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
              }
#endif /* #ifdef NO_ASCIIZ */
              else if(ati_user_output_cfg[srcId].CPI_stat > 2)
              {
                pos += sprintf(g_sa + pos,",");
              }
            } /* if (calLst[idx].number[0] NEQ 0x0) */
            else
            {
              pos += sprintf(g_sa + pos,",,,");
            }
            break;
          } /* else of - if (calLst[idx].idx NEQ cId) */

        } /* else of - if( calLst[idx].idx EQ ACI_NumParmNotPresent ) */

      } /*for (idx = 0; idx < MAX_CALL_NR; idx++) */

      
      if (ati_user_output_cfg[srcId].CPI_stat > 2)
      {
        if (ati_user_output_cfg[srcId].CPI_stat EQ 4)
        {
          pos += sprintf(g_sa + pos,",0x%4x", cause);
        }
        else
        {
          /*
           * Issue : OMAPS00061262, Removed the two TI internal Causes
           * MNCC_CAUSE_REEST_STARTED & MNCC_CAUSE_REEST_FINISHED 
           * from the %CPI.
           */
          if( GET_CAUSE_ORIGIN_ENTITY(cause) EQ MNCC_CC_ORIGINATING_ENTITY  AND 
              GET_CAUSE_DEFBY(cause)         EQ DEFBY_STD )
          {
            TRACE_EVENT_P2("cause: %d pos: %d",cId, cause);
            pos += sprintf ( g_sa + pos,",%d", GET_CAUSE_VALUE(cause) );
          }
          else
          {
            pos += sprintf(g_sa + pos,",");
          }
        }

        /* Report the the ALS bearer */
        if(calLst[idx].class_type EQ CLCC_CLASS_Line2)
        {
          pos += sprintf ( g_sa + pos,",1");
        }
        else
        {
          pos += sprintf ( g_sa + pos,",0");
        }

      }


    }/*if ( (ati_user_output_cfg[srcId].CPI_stat > 0) ) */

    ACI_MFREE (calLst);
    ci_remTrailCom(g_sa, pos);
    io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
}
#ifdef ACI /* for ATI only version */
  cmhMMI_handleAudioTone ( AT_CMD_NONE, RAT_CPI, msgType );
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCTYI    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCTYI call back

*/

GLOBAL void rCI_PercentCTYI (T_ACI_CTTY_NEG neg,
                             T_ACI_CTTY_TRX trx)
{
  int pos = 0;

  TRACE_FUNCTION ("rCI_PercentCTYI()");

  if (cmhPrm[srcId_cb].ccCmdPrm.CTTYmode EQ CTTY_MOD_Enable)
  {
    pos = sprintf (g_sa, "%s: %d", "%CTYI", (int)neg);

    if (trx NEQ CTTY_TRX_Unknown)
      pos += sprintf (g_sa + pos, ",%d", trx);

    io_sendMessage (srcId_cb, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCTV     |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCTV call back

*/

GLOBAL void rCI_PercentCTV  ( /*UBYTE srcId*/ void )
{
  TRACE_FUNCTION("rCI_PercentCTV()");

  callTime++;

  if( callTime EQ 10 )
  {
    callTime = 0;
    allowCCCM = TRUE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : sAT_URC            |
+--------------------------------------------------------------------+

  PURPOSE : this function handles unsolicited result codes generated
            using the extension mechanism for unknown AT commands.

*/

GLOBAL T_ACI_RETURN sAT_URC ( CHAR * out )
{

  TRACE_FUNCTION("sAT_URC ()");

  if (ati_get_mode( src_id_ext ) NEQ ATI_UNKN_MODE)        /* Send only to "serial" sources, no SAT etc. */
  {
    aci_sendPString( src_id_ext, out );
  }

  return AT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : sAT_FRI            |
+--------------------------------------------------------------------+

  PURPOSE : this function handles final result codes generated
            using the extension mechanism for unknown AT commands.

*/

GLOBAL T_ACI_RETURN sAT_FRI ( USHORT cmdLen )
{
  T_ACI_RETURN rslt = AT_FAIL;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  if ( src_params->curAtCmd EQ AT_CMD_EXT )
  {
    rslt = AT_CMPL;
  }

  return ( rslt );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCCBS    |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PercentCCBS call back

*/

GLOBAL void rCI_PercentCCBS  ( /*UBYTE srcId,*/ T_ACI_CCBS_IND  ind,
                               T_ACI_CCBS_STAT status,
                               T_ACI_CCBS_SET *setting,
                               BOOL           intermediate_result )
{
  SHORT pos   = 0;
  UBYTE srcId = srcId_cb;


  TRACE_FUNCTION("rCI_PercentCCBS()");

  if ( at.flags.CCBS_stat         OR
       ind EQ CCBS_IND_IrgtResult )
  {
    pos=sprintf(g_sa,"%s: ","%CCBS");

    /* 1st param */
    switch(ind)
    {
    case(CCBS_IND_NotPresent):
      pos+=sprintf(g_sa+pos,",");
      break;

    case(CCBS_IND_IrgtResult):
      if(status NEQ CCBS_STAT_NotPresent)
      {
        pos+=sprintf(g_sa+pos,"%d,",ind);
      }
      break;

    default:
      pos+=sprintf(g_sa+pos,"%d,",ind);
      break;
    }

    if(setting EQ NULL)
    {
      /* do nothing: if later a parameter is added, extra commas may be needed...*/
    }
    else
    {
      /* 2nd param: CCBS index */
      if(setting->idx NEQ ACI_NumParmNotPresent)
        pos+=sprintf(g_sa+pos,"%d,",setting->idx);
      else
        pos+=sprintf(g_sa+pos,",");

      /* 3rd param: Number */
      if (setting->number[0] NEQ 0x0 )
        pos+=sprintf(g_sa+pos,"\"%s\",",setting->number);
      else
        pos+=sprintf(g_sa+pos,",");

      /* 4th param: Type of number */
      if (setting->type.npi NEQ NPI_NotPresent )
        pos+=sprintf(g_sa+pos,"%d,",toa_merge(setting->type));
      else
        pos+=sprintf(g_sa+pos,",");

      /* 5th param: sub address */
      if (setting->subaddr[0] NEQ 0x0 )
        pos+=sprintf(g_sa+pos,"\"%s\",",setting->subaddr);
      else
        pos+=sprintf(g_sa+pos,",");

      /* 6th param: type of subaddress */
      if (setting->satype.tos NEQ TOS_NotPresent )
        pos+=sprintf(g_sa+pos,"%d,",tos_merge(setting->satype));
      else
        pos+=sprintf(g_sa+pos,",");

      /* 7th param:  CCBS class */
      if (setting->class_type NEQ CLASS_NotPresent)
        pos+=sprintf(g_sa+pos,"%d,",setting->class_type);
      else
        pos+=sprintf(g_sa+pos,",");

      /* 8th param: ALPT */
      if (setting->alrtPtn NEQ ALPT_NotPresent)
        pos+=sprintf(g_sa+pos,"%d",setting->alrtPtn);
    }

    ci_remTrailCom(g_sa, pos);

    if(intermediate_result)
    {
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }
    else
      io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_sms_ready      |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_sms_ready call back

*/

GLOBAL void rCI_sms_ready  ( /*UBYTE srcId*/ void )
{

  TRACE_FUNCTION("rCI_sms_ready()");
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_phb_status     |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_phb_status call back

*/

GLOBAL void rCI_phb_status ( /*UBYTE srcId,*/ T_ACI_PB_STAT status )
{
 
  TRACE_FUNCTION("rCI_phb_status()");

}


GLOBAL void rCI_PlusCMGD ()
{
  /* dummy function */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : aci_writeCnumMsisdn|
+--------------------------------------------------------------------+

  PURPOSE : This function writes the formatted string to AT command
            interface.
*/
LOCAL UBYTE aci_writeCnumMsisdn ( UBYTE srcId, T_ACI_CNUM_MSISDN* msisdn )
{
  USHORT pos = 0;

#ifndef FF_ATI_BAT
  CHAR   cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT lenCvtdAlpha;
#endif

  TRACE_FUNCTION ("aci_writeCnumMsisdn()");

  if ( msisdn -> vldFlag EQ FALSE )
  {
    return 0;
  }

  pos = sprintf ( g_sa, "+CNUM: " );

  /*
   *-----------------------------------------------------------------
   * process parameter <alpha>
   *-----------------------------------------------------------------
   */

  if ( msisdn -> alpha[0] NEQ '\0' )
  {
#ifdef FF_ATI_BAT
    pos += sprints ( g_sa + pos, (char *)msisdn->alpha, (USHORT)strlen(msisdn->alpha) );
#else
    utl_chsetFromGsm((UBYTE*)msisdn->alpha,
                     (USHORT)strlen(msisdn->alpha),
                     (UBYTE*)cvtdAlpha,
                     sizeof(cvtdAlpha),
                     &lenCvtdAlpha,
                     GSM_ALPHA_Int);
    pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
#endif
  }

  /*
   *-----------------------------------------------------------------
   * process parameter <number> and <type>
   *-----------------------------------------------------------------
   */
  pos += sprintf ( g_sa + pos, ",\"%s\",%d",
                   msisdn -> number,
                   toa_merge ( msisdn -> type ) );

  /*
   *-----------------------------------------------------------------
   * process parameter <speed>
   *-----------------------------------------------------------------
   */
  if ( msisdn -> speed NEQ BS_SPEED_NotPresent )
  {
    pos += sprintf ( g_sa + pos, ",%d", msisdn -> speed );
  }
  else
  {
    pos += sprintf ( g_sa + pos, "," );
  }

  /*
   *-----------------------------------------------------------------
   * process parameter <service>
   *-----------------------------------------------------------------
   */
  if ( msisdn -> service NEQ CNUM_SERV_NotPresent )
  {
    pos += sprintf ( g_sa + pos, ",%d", msisdn -> service );
  }
  else
  {
    pos += sprintf ( g_sa + pos, "," );
  }

  /*
   *-----------------------------------------------------------------
   * process parameter <itc>
   *-----------------------------------------------------------------
   */
  if ( msisdn -> itc NEQ CNUM_ITC_NotPresent )
  {
    pos += sprintf ( g_sa + pos, ",%d", msisdn -> itc );
  }

  ci_remTrailCom ( g_sa, ( USHORT ) strlen ( g_sa ) );

  io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );

  return 1;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCNAP    |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PercentCNAP call back

*/
typedef enum
{
  CNAP_PRES_ALLOWED = 0,
  CNAP_PRES_RESTRICTED,
  CNAP_NAME_UNAVAILABLE,
  CNAP_NAME_PRES_RESTRICTED
} T_ATI_CNAP_PRESENTATION;

LOCAL void send_cnap_name_information(UBYTE srcId,
                                      T_ATI_CNAP_PRESENTATION pres_mode,
                                      T_callingName *NameId)
{
  T_namePresentationAllowed *cnap_name_info = NULL;
  UBYTE number_of_chars = 0;
  CHAR  *cnap_name = NULL;
  int pos = 0;

  TRACE_FUNCTION("send_cnap_name_information()");

  switch(pres_mode)
  {
    case(CNAP_PRES_ALLOWED):
      TRACE_EVENT("CNAP: name presentation is allowed");
      cnap_name_info = &(NameId->namePresentationAllowed);
      break;

    case(CNAP_NAME_PRES_RESTRICTED):
      TRACE_EVENT("CNAP: name presentation is restricted");
      cnap_name_info = &(NameId->namePresentationRestricted);
      break;

    default:
      TRACE_ERROR("CNAP: wrong presentation mode");
      return;
  }
  if(cnap_name_info EQ NULL)
  {
    TRACE_ERROR("CNAP cnap_name_info is NULL");
    return;
  }

  pos += sprintf(g_sa+pos,"%s%d,", "%CNAP: ", pres_mode);

  if(cnap_name_info->v_dataCodingScheme)
  {
    pos += sprintf(g_sa+pos,"%d,", cnap_name_info->dataCodingScheme);
  }
  if(cnap_name_info->v_lengthInCharacters)
  {
    number_of_chars = cnap_name_info->lengthInCharacters;
    pos += sprintf(g_sa+pos,"%d,", number_of_chars);
    number_of_chars = MINIMUM(number_of_chars, sizeof(g_sa)-(size_t)pos);
  }
  if(cnap_name_info->v_nameString AND
     number_of_chars NEQ 0)
  {
     ACI_MALLOC(cnap_name, number_of_chars+3); /* 1 char for \0 and 2 chars for " */
     memcpy(cnap_name, &(cnap_name_info->nameString.b_nameString), number_of_chars);
     cnap_name[number_of_chars] = 0;  /* Null terminated */
     pos += sprintf(g_sa+pos,"\"%s\"", cnap_name);
  }
  ci_remTrailCom(g_sa,(USHORT)strlen(g_sa));

  io_sendIndication( srcId, g_sa, ATI_NORMAL_OUTPUT );

  if(cnap_name NEQ NULL)
  {
    ACI_MFREE(cnap_name);
  }
}

LOCAL void unsolicited_rci_percent_cnap(UBYTE srcId, T_callingName *NameId)
{
  TRACE_FUNCTION("unsolicited_rci_percent_cnap()");

  if(ati_user_output_cfg[srcId].cnap_mode EQ CNAP_DISABLED)
  {
    TRACE_EVENT_P1("unsolicited message CNAP ignored: CNAP not enable for source: %d", srcId);
    return;
  }

  if(NameId->v_namePresentationAllowed)
  {
    send_cnap_name_information(srcId, CNAP_PRES_ALLOWED, NameId);
    return;
  }

  if(NameId->v_presentationRestricted)
  {
    sprintf(g_sa, "%s%d", "%CNAP: ", CNAP_PRES_RESTRICTED);
    io_sendIndication( srcId, g_sa, ATI_NORMAL_OUTPUT );
    return;
  }

  if(NameId->v_nameUnavailable)
  {
    sprintf(g_sa, "%s%d", "%CNAP: ", CNAP_NAME_UNAVAILABLE);
    io_sendIndication( srcId, g_sa, ATI_NORMAL_OUTPUT );
    return;
  }

  if(NameId->v_namePresentationRestricted)
  {
    send_cnap_name_information(srcId, CNAP_NAME_PRES_RESTRICTED, NameId);
    return;
  }
}

LOCAL void intermediate_result_rci_percent_cnap(UBYTE srcId, T_ACI_CNAP_STATUS status)
{
  TRACE_FUNCTION("intermediate_result_rci_percent_cnap()");

  sprintf(g_sa, "%s%d,%d", "%CNAP: ", ati_user_output_cfg[srcId].cnap_mode, status);
  io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT );
}

GLOBAL void rCI_PercentCNAP  ( T_callingName *NameId, T_ACI_CNAP_STATUS status )
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentCNAP()");

  if(NameId NEQ NULL)
  {
    unsolicited_rci_percent_cnap(srcId, NameId);
    return;
  }

  if(status NEQ CNAP_SERVICE_STATUS_NOT_PRESENT)
  {
    intermediate_result_rci_percent_cnap(srcId, status);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCMS_Conc   |
+--------------------------------------------------------------------+

  PURPOSE : handles new rCI_PlusCMS call back for concatenated SMS

*/
GLOBAL void rCI_PlusCMS_Conc ( T_ACI_AT_CMD     cmdId,
                               T_ACI_CMS_ERR    err,
                               T_EXT_CMS_ERROR *conc_error )
{
  TRACE_FUNCTION("rCI_PlusCMS_Conc()");
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_SignalSMS      |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_SignalSMS call back for concatenated SMS

*/
GLOBAL void rCI_SignalSMS ( UBYTE state )
{
  TRACE_FUNCTION("rCI_SignalSMS()");
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCIEV       |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_SignalCIEV call back for signal strength
            and SIM-full indications

*/
GLOBAL void rCI_PlusCIEV ( T_ACI_MM_CIND_VAL_TYPE sCindValues,
                          T_ACI_MM_CMER_VAL_TYPE sCmerSettings )
{
  CHAR*  me                       = "+CIEV: ";
  SHORT  pos                      = 0;

  UBYTE          srcId = srcId_cb;
  T_ATI_IO_MODE  sIoMode;
  char          *acTempString = "";
  UINT           uiCount;

  TRACE_FUNCTION("rCI_PlusCIEV()");

  pos = sprintf(g_sa,"%s",me);

  /* check settings in 'cmhCmdPrm' and depends on, send the indication or buffer it */
  sIoMode = ati_get_mode(srcId_cb);
  if( sIoMode EQ ATI_UNKN_MODE )
  {
    acTempString = "internal ACI error at rCI_PlusCIEV() ->> ATI unknown mode !!";
    sprintf( g_sa+pos, "%s, 0", acTempString );
    io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return;
  }
  if( (sIoMode EQ ATI_CMD_MODE) AND
      (sCmerSettings.sCmerModeParam NEQ CMER_MODE_INVALID) AND /* 'invalid' EQ 'mode_0' */
      (sCmerSettings.sCmerModeParam NEQ CMER_MODE_0) )
    /* ---------------------------------------------------------------------------------- */
  { /* ----------- UART is in command mode --> send the +CIEV to terminal --------------- */
    /* ---------------------------------------------------------------------------------- */
    if( sCindValues.sCindSignalParam NEQ CIND_SIGNAL_INDICATOR_INVALID )
    {
      acTempString = "1"; /* the 'signal' indicator has the order number '1' in +CIND */
      sprintf( g_sa+pos, "%s, %d", acTempString, sCindValues.sCindSignalParam );
    }
    else
    {
      if( sCindValues.sCindSmsFullParam NEQ CIND_SMSFULL_INDICATOR_INVALID )
      {
        acTempString = "2"; /* the 'smsfull' indicator has the order number '2' in +CIND */
        sprintf( g_sa+pos, "%s, %d", acTempString, sCindValues.sCindSmsFullParam );
      }
      else
      { /* error!! --> no signal has to be send */
        acTempString = "internal ACI error at rCI_PlusCIEV() !!";
        sprintf( g_sa+pos, "%s, 0", acTempString);
      }
    }
    io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
       /* -------------------------------------------------------------------------------- */
  else /* -- UART is in data mode or CMER-mode EQ '0' --> buffer the +CIEV if necessary -- */
  {    /* -------------------------------------------------------------------------------- */
    if( sCmerSettings.sCmerModeParam NEQ CMER_MODE_1 )
    { /* discard message in case of MODEEQ'1' */
      if( sCindValues.sCindSignalParam NEQ CIND_SIGNAL_INDICATOR_INVALID )
      { /* storing of SIGNAL Value */
        if( asCievSignalBuf.uiLastIndex < CIEV_BUF_SIZE )
        { /* free space in buffer -> store +CIEV indication for further purpose */
          asCievSignalBuf.asBufferValues[asCievSignalBuf.uiLastIndex] = sCindValues.sCindSignalParam;
          asCievSignalBuf.uiLastIndex++;
        }
        else /* = buffer is full (uiLastIndex EQ CIEV_BUF_SIZE) = */
        {    /* -> drop the oldest one and try to store it again  */
          uiCount = 0;
          while( uiCount < (CIEV_BUF_SIZE-1) )
          { /* drop the oldes one */
            asCievSignalBuf.asBufferValues[uiCount] = asCievSignalBuf.asBufferValues[uiCount+1];
            uiCount++;
          }
          asCievSignalBuf.asBufferValues[asCievSignalBuf.uiLastIndex-1] = sCindValues.sCindSignalParam;
        }
      }
      else
      { /* storing of SMS full Value */
        if( asCievSmsFullBuf.uiLastIndex < CIEV_BUF_SIZE )
        { /* free space in buffer -> store +CIEV indication for further purpose */
          asCievSmsFullBuf.asBufferValues[asCievSmsFullBuf.uiLastIndex] = sCindValues.sCindSmsFullParam;
          asCievSmsFullBuf.uiLastIndex++;
        }
        else /* = buffer is full (uiLastIndex EQ CIEV_BUF_SIZE) = */
        {    /* -> drop the oldest one and try to store it again  */
          uiCount = 0;
          while( uiCount < (CIEV_BUF_SIZE-1) )
          { /* drop the oldes one */
            asCievSmsFullBuf.asBufferValues[uiCount] = asCievSmsFullBuf.asBufferValues[uiCount+1];
            uiCount++;
          }
          asCievSmsFullBuf.asBufferValues[asCievSmsFullBuf.uiLastIndex-1] = sCindValues.sCindSmsFullParam;
        }
      }
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ACI_RET                  |
| STATE   : code                  ROUTINE : aci_frmtOutput           |
+--------------------------------------------------------------------+

  PURPOSE : fills T_ACI_SM_DATA data structure for UD with UDHI
*/
LOCAL void aci_frmtOutput ( UBYTE fo,
                            UBYTE dcs,
                            T_ACI_SM_DATA *data )
{
  switch (fo & TP_MTI_MASK)
  {
  case TP_MTI_SMS_DELIVER:
    {
      T_TP_DELIVER *sms_deliver = (T_TP_DELIVER*)_decodedMsg;
      data->data[0] = sms_deliver->tp_udh_inc.tp_udh.c_data;
      memcpy(data->data+1,
             sms_deliver->tp_udh_inc.tp_udh.data,
             sms_deliver->tp_udh_inc.tp_udh.c_data);

      memcpy(data->data+sms_deliver->tp_udh_inc.tp_udh.c_data+1,
             sms_deliver->tp_udh_inc.data,
             sms_deliver->tp_udh_inc.c_data);

      /*
      data->len = sms_deliver->tp_udh_inc.tp_udh.c_data +
                 sms_deliver->tp_udh_inc.c_data + 1;
      */

      if (cmhSMS_getAlphabetPp (dcs) EQ 0)
        data->len = (sms_deliver->tp_udh_inc.length * 7 + 7)/8;
      else
        data->len = sms_deliver->tp_udh_inc.length;
    }
    break;

  case TP_MTI_SMS_SUBMIT:
    {
      T_TP_SUBMIT *sms_submit = (T_TP_SUBMIT*)_decodedMsg;
      data->data[0] = sms_submit->tp_udh_inc.tp_udh.c_data;
      memcpy(data->data+1,
             sms_submit->tp_udh_inc.tp_udh.data,
             sms_submit->tp_udh_inc.tp_udh.c_data);

      memcpy(data->data+sms_submit->tp_udh_inc.tp_udh.c_data+1,
             sms_submit->tp_udh_inc.data,
             sms_submit->tp_udh_inc.c_data);

      /*
      data->len = sms_submit->tp_udh_inc.tp_udh.c_data +
                  sms_submit->tp_udh_inc.c_data + 1;
      */

      if (cmhSMS_getAlphabetPp (dcs) EQ 0)
        data->len = (sms_submit->tp_udh_inc.length * 7 + 7)/8;
      else
        data->len = sms_submit->tp_udh_inc.length;
    }
    break;

  case TP_MTI_SMS_COMMAND:
    {
      T_TP_COMMAND *sms_command = (T_TP_COMMAND*)_decodedMsg;
      data->data[0] = sms_command->tp_cdh_inc.tp_udh.c_data;
      memcpy(data->data+1,
             sms_command->tp_cdh_inc.tp_udh.data,
             sms_command->tp_cdh_inc.tp_udh.c_data);

      memcpy(data->data+sms_command->tp_cdh_inc.tp_udh.c_data+1,
             sms_command->tp_cdh_inc.data,
             sms_command->tp_cdh_inc.c_data);

      /*
      data->len = sms_command->tp_cdh_inc.tp_udh.c_data +
                  sms_command->tp_cdh_inc.c_data + 1;
      */

      data->len = sms_command->tp_cdh_inc.c_data;
    }
    break;
/*
  case TP_MTI_SMS_STATUS_REP:
    {
      T_TP_STATUS *sms_status = (T_TP_STATUS*)_decodedMsg;
      data->data[0] = sms_status->tp_udh_inc.tp_udh.c_data;
      memcpy(data->data+1,
             sms_status->tp_udh_inc.tp_udh.data,
             sms_status->tp_udh_inc.tp_udh.c_data);

      memcpy(data->data+sms_status->tp_udh_inc.tp_udh.c_data+1,
             sms_status->tp_udh_inc.data,
             sms_status->tp_udh_inc.c_data);

      data->len = sms_status->tp_udh_inc.tp_udh.c_data +
                  sms_status->tp_udh_inc.c_data + 1;
    }
    break;
*/
  default:
    data->data[0] = '\0';
    data->len = 0;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCPRI    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCPRI call back
*/

GLOBAL void rCI_PercentCPRI   ( UBYTE gsm_ciph, UBYTE gprs_ciph )
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PLusCPRI()");

  /* %CPRI notification is sent only to the source through which the status is enabled */
  if (ati_user_output_cfg[srcId].CPRI_stat EQ CI_SHOW)
  {
    sprintf(g_sa, "%s: %d,%d", "%CPRI", gsm_ciph, gprs_ciph);
    io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCTZR       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCTZV call back.
            timezone is a range from -48..0..+48 and expresses the deviation
            from GMT in steps of 15 minutes.
*/
#ifdef FF_TIMEZONE
GLOBAL void rCI_PlusCTZV ( S32 timezone )
{
  UBYTE srcId = srcId_cb;
  TRACE_FUNCTION("rCI_PlusCTZV()");
  
  sprintf (g_sa, "%s\"%+02d\"", "+CTZV: ", timezone);

  /* Send response to AT interface */
  io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
}
#else
GLOBAL void rCI_PlusCTZV ( UBYTE* timezone )
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PlusCTZV()");

  sprintf (g_sa, "%s%d", "+CTZV: ", *timezone);

  /* Send response to AT interface */
  io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
}
#endif


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCTZV    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCTZV call back, the parameter mmr_info_ind
            includes time and time zone information. 

*/

GLOBAL void rCI_PercentCTZV (T_MMR_INFO_IND *mmr_info_ind, S32 timezone)
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentCTZV()");

 /*Report the network time in time zone 0*/
  sprintf (g_sa, "%s\"%02d/%02d/%02d,%02d:%02d:%02d%+02d\"", "%CTZV: ", 
                   mmr_info_ind->time.year, 
                   mmr_info_ind->time.month,
                   mmr_info_ind->time.day, 
                   mmr_info_ind->time.hour, 
                   mmr_info_ind->time.minute,
                   mmr_info_ind->time.second,
                   timezone);

  /* Send response to AT interface */
  io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET |
| STATE   : code          ROUTINE : aci_format_plmn_name    |
+--------------------------------------------------------------------+

  PURPOSE : Decodes PLMN name to remote source format 

*/

void aci_format_plmn_name(T_full_name * plmn, UBYTE *out)
{ 
  USHORT size_of_plmn_name;
  UBYTE dest_len=0;
  UBYTE buf[MMR_MAX_TEXT_LEN];

  if(plmn->dcs ==0) /*GSM default */
  {
    dest_len = utl_cvt7To8(plmn->text,plmn->c_text,buf,0);
    utl_chsetFromGsm(buf,dest_len,out,MMR_MAX_TEXT_LEN,&size_of_plmn_name,GSM_ALPHA_Def);
  }
  else if(plmn->dcs ==1) /*UCS2 */
  {
    utl_hexFromGsm(plmn->text,plmn->c_text,out,MMR_MAX_TEXT_LEN,&size_of_plmn_name,GSM_ALPHA_Def,CSCS_ALPHA_7_Bit);
  }
}	

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCNIV    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCNIV call back

*/

GLOBAL void rCI_PercentCNIV (T_MMR_INFO_IND *mmr_info_ind)
{
  UBYTE srcId = srcId_cb;
  char  buf1[MMR_MAX_TEXT_LEN] ; /*we limit the size of printed symbols to avoid stack size increase*/
  char  buf2[MMR_MAX_TEXT_LEN] ;

  char  plmn[7] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  SHORT mcc = 0; 
  SHORT mnc = 0;

  TRACE_FUNCTION("rCI_PercentCNIV()");

  memset(buf1,0,MMR_MAX_TEXT_LEN);
  memset(buf2,0,MMR_MAX_TEXT_LEN);

  if (mmr_info_ind->plmn.v_plmn)
  {
    cmhMM_CnvrtPLMN2INT( mmr_info_ind->plmn.mcc, mmr_info_ind->plmn.mnc, &mcc, &mnc );
  }

  /* build numeric plmn representation */
  if ((mnc & 0x000F) EQ 0x000F)
  {
    sprintf (plmn, "\"%03X%02X\"", mcc, (mnc & 0x0FF0) >> 4);
  }
  else
  {
    sprintf (plmn, "\"%03X%03X\"", mcc, mnc);
  }

  if (mmr_info_ind->short_name.v_name)  /* short name  */
  { 
    aci_format_plmn_name(&mmr_info_ind->short_name, (UBYTE *)buf1);
  }

  if (mmr_info_ind->full_name.v_name) /* full name  */
  {
    aci_format_plmn_name(&mmr_info_ind->full_name, (UBYTE *)buf2);
  }
  
  sprintf (g_sa, "%s \"%s\",\"%s\",%s", "%CNIV:", buf2, buf1, plmn);  
  
  /* Send response to AT interface */
  io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ACI_RET                  |
| STATE   : code                  ROUTINE : rCI_Z                    |
+--------------------------------------------------------------------+

  PURPOSE : RAT callback for ATZ, for abstraction of ATI and CMH.
*/
GLOBAL void rCI_Z ( void )
{
  UBYTE srcId = srcId_cb;

  ati_cmd_reset( srcId );
  ati_zn_retrieve_params( srcId );
}

#endif /* FF_ATI */


#if defined FF_EOTD
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PlusCLPS       |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCLPS call back
*/

GLOBAL void rCI_PlusCLPS   ( UBYTE srcId, T_LOC_POS_DATA * p_aci_lc_data )
{
  TRACE_FUNCTION("rCI_PlusCLPS()");

  io_sendIndication(srcId, "%CLPS:", ATI_NORMAL_OUTPUT);
}
#endif /* FF_EOTD */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentALS     |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusALS call back
*/

GLOBAL void rCI_PercentALS ( T_ACI_ALS_MOD ALSmode )
{
  UBYTE srcId = srcId_cb;
  char *me = "%ALS";

  TRACE_FUNCTION("rCI_PlusALS()");

  if (ALSmode EQ ALS_MOD_SPEECH)
    sprintf(g_sa,"%s: (0)", me);
  if (ALSmode EQ ALS_MOD_AUX_SPEECH)
    sprintf(g_sa,"%s: (1)", me);
  if (ALSmode EQ (ALS_MOD_SPEECH | ALS_MOD_AUX_SPEECH))
    sprintf(g_sa,"%s: (0,1)", me);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentRDL     |
+--------------------------------------------------------------------+

  PURPOSE : handles AT%RDL call back
*/
GLOBAL void rCI_PercentRDL ( T_ACI_CC_REDIAL_STATE state )
{
  UBYTE srcId = srcId_cb;
  char *me = "%RDL:";

  TRACE_FUNCTION("rCI_PercentRDL()");

  if(rdlPrm.rdlModN EQ NOTIF_USER)
  {
    sprintf(g_sa,"%s %d",me,state);
    io_sendMessage(srcId, g_sa, ATI_INDICATION_OUTPUT);
  }
}

#ifdef TI_PS_FF_AT_P_CMD_RDLB
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentRDLB    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT%RDLB call back
*/
GLOBAL void rCI_PercentRDLB ( T_ACI_CC_RDL_BLACKL_STATE state )
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentRDLB()");

  if(rdlPrm.rdlBlN EQ NOTIF_USER)
  {
    sprintf(g_sa,"%s: %d","%RDLB",state);
    io_sendMessage(srcId, g_sa, ATI_INDICATION_OUTPUT);
  }
}
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCSTAT   |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PercentCSTAT call back

*/
GLOBAL void rCI_PercentCSTAT ( T_ACI_STATE_MSG msgType )
{
  SHORT     pos   = 0;
  UBYTE   srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentCSTAT()");

  if (ati_user_output_cfg[srcId].CSTAT_stat EQ 1)
  {
    pos = sprintf(g_sa,"%s: ","%CSTAT");
    
    switch(msgType.entityId)
    {
    case STATE_MSG_PBOOK:
      pos += sprintf(g_sa+pos,"PHB, %d",msgType.entityState);
      break;
    case STATE_MSG_SMS:
      pos += sprintf(g_sa+pos,"SMS, %d",msgType.entityState);
      break;
    case STATE_MSG_EONS:
      pos += sprintf(g_sa+pos,"EONS, %d",msgType.entityState);
      break;
    case STATE_MSG_RDY:
      pos += sprintf(g_sa+pos,"RDY, %d",msgType.entityState);
      break;
    default:
      return;
    }
    
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    
  }

} 

/*
+------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET                |
| STATE   : code                        ROUTINE : ati_printRemainFieElem |
+------------------------------------------------------------------------+

  PURPOSE : this function prints all remained FIE-elemnts onto terminal
*/
LOCAL void ati_printRemainFieElem( T_MNCC_fac_inf *fie,
                                   UBYTE srcId )
{
  UBYTE       uTotalCharCounter = 0;
  USHORT      uLineCounter   = 0;
  UBYTE const cuNbrOfMsgElem = fie->l_fac >> 3;

  TRACE_FUNCTION("ati_printRemainFieElem()");

  /* print FIE elements */
  while(uTotalCharCounter < cuNbrOfMsgElem)
  {
    uLineCounter += sprintf(g_sa+uLineCounter, "%02X", fie->fac[uTotalCharCounter++]);
    if ( uLineCounter > (MAX_CMD_LEN-((MAX_CMD_LEN*95)/100)) )  /* only 5% of g_sa buffer used */
    {                                                           /* allows better sim test */
      g_sa[++uLineCounter] = 0x00; /* terminate g_sa after last char */
#ifdef _SIMULATION_
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);  /* and send the data in chunks with CRLF */
#else /* _SIMULATION_ */
      io_sendMessage(srcId, g_sa, ATI_ECHO_OUTPUT);
#endif
      uLineCounter=0; /* go to beginning of g_sa for next chunk */
    }
  }
  g_sa[++uLineCounter] = 0x00; /* terminate g_sa after last char in last chunk */
#ifdef _SIMULATION_
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);  /* and send the data in chunks with CRLF */
#else /* _SIMULATION_ */
  io_sendMessage(srcId, g_sa, ATI_ECHO_OUTPUT);
  g_sa[0] = 0x0; /* print CR/LF after last chunk only on target */
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCCCN    |
+--------------------------------------------------------------------+

  PURPOSE : handles unsolicited CC-network changes callback
*/

GLOBAL void rCI_PercentCCCN ( T_ACI_FAC_DIR tDirection,
                              SHORT         cId,
                              T_MNCC_fac_inf    *fie )
{
  UBYTE srcId           = srcId_cb;
  char *me              = "%CCCN";

  TRACE_FUNCTION("rCI_PercentCCCN()");

  /* no output in case of empty FIE-string */
  if( (fie->l_fac >> 3) EQ 0)
  {
    TRACE_FUNCTION("rCI_PercentCCCN() : Empty FIE-string");
    return;
  }

  sprintf(g_sa, "%s: %d,%d,", me, tDirection, cId); /* Preamble */

#ifdef _SIMULATION_ /* 'ATI_ECHO_OUTPUT' doesn't work while Windows simulation */
  io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT ); /* print preamble while SIM-test */
#else /* _SIMULATION_ */
  io_sendMessage( srcId, g_sa, ATI_ECHO_OUTPUT ); /* print preamble to target */
#endif

  ati_printRemainFieElem( fie, srcId );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCSSN    |
+--------------------------------------------------------------------+

  PURPOSE : handles unsolicited CC-network changes callback
*/

GLOBAL void rCI_PercentCSSN ( T_ACI_FAC_DIR        tDirection,
                              T_ACI_FAC_TRANS_TYPE tType,
                              T_MNCC_fac_inf    *fie )
{
  UBYTE srcId = srcId_cb;
  char *me = "%CSSN";

  TRACE_FUNCTION("rCI_PercentCSSN()");

  /* no output in case of empty FIE-string */
  if( (fie->l_fac >> 3) EQ 0)
  {
    TRACE_FUNCTION("rCI_PercentCSSN() : Empty FIE-string");
    return;
  }

  sprintf(g_sa, "%s: %d,%d,", me, tDirection, tType); /* Preamble */

#ifdef _SIMULATION_ /* 'ATI_ECHO_OUTPUT' doesn't work while Windows simulation */
  io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT); /* print preamble while SIM-test */
#else /* _SIMULATION_ */
  io_sendMessage( srcId, g_sa, ATI_ECHO_OUTPUT ); /* print preamble to target */
#endif

  ati_printRemainFieElem( fie, srcId );
}

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCPRSM   |
+--------------------------------------------------------------------+

  PURPOSE : handles callback for %CPRSM? querying the SMS delivery status
            which can be either PAUSE or RESUME 
*/

GLOBAL void rCI_PercentCPRSM ( T_ACI_CPRSM_MOD mode )
{
  UBYTE srcId = srcId_cb;
  char *me = "%CPRSM";
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PercentCPRSM()");

  sprintf(g_sa,"%s: %d", me, mode);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentSIMEF   |
+--------------------------------------------------------------------+

  PURPOSE : unsolicited result code %SIMEF: <NUM>, <Id1>, <Id2>… 

  For Id, which can be EF only or concatenation of DFEF or DF1DF2EF,
  watch 11.11 for DF and EF
*/
GLOBAL void rCI_PercentSIMEF(T_SIM_FILE_UPDATE_IND *sim_file_update_ind)
{
  UBYTE srcId=srcId_cb;
  SHORT pos;
  UBYTE n;

  TRACE_FUNCTION("rCI_PercentSIMEF()");

  pos = sprintf(g_sa,"%s: %d,","%SIMEF", sim_file_update_ind->val_nr);

  for (n=0; n<sim_file_update_ind->val_nr; n++)
  {
    if (sim_file_update_ind->file_info[n].v_path_info )
    {
      pos+=sprintf(g_sa+pos,"%04X,",sim_file_update_ind->file_info[n].path_info.df_level1); /* the DF */

      if (sim_file_update_ind->file_info[n].path_info.v_df_level2)
      {
        pos+=sprintf(g_sa+pos,"%04X,",sim_file_update_ind->file_info[n].path_info.df_level2); /* the DF 2.level */     
      }
    }
    pos+=sprintf(g_sa+pos,"%04X",sim_file_update_ind->file_info[n].datafield);

    if (n NEQ sim_file_update_ind->val_nr-1)
    {
      *(g_sa+pos++)=',';
    }

    if(pos >= (MAX_CMD_LEN-20))
    {
#ifdef _SIMULATION_
      io_sendMessage(srcId,g_sa,ATI_NORMAL_OUTPUT); /* append CR+LF only for testcase */
#else
      io_sendMessage(srcId,g_sa,ATI_ECHO_OUTPUT); /* Output without CR,LF for AT based MMI */
#endif /* _SIMULATION */
      pos=0;
    }
  }
  io_sendMessage(srcId,g_sa,ATI_NORMAL_OUTPUT); /* Second line output */
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ACI_RET                  |
| STATE   : code                  ROUTINE : mfwExtOut                |
+--------------------------------------------------------------------+

  PURPOSE : This function is used as a dummy to fullfill display
            driver calls.
*/
#ifndef MFW

int mfwExtOut (char *cmd)
{
  TRACE_FUNCTION ("mfwExtOut");

  return 1;
}

#endif /* no MFW */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCMGR    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCMGR call back (Read Message)
*/

GLOBAL void rCI_PercentCMGR  ( T_MNSMS_READ_CNF* mnsms_read_cnf,
                               T_ACI_CMGR_CBM * cbm)

{
  UBYTE msg_type;
#if defined (SMS_PDU_SUPPORT)
  UBYTE srcId = srcId_cb;
  T_ACI_CMGF_MOD mode;
  

  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
    /*
     * handle PDU mode
     */
    rCI_Plus_Percent_CMGRPdu (mnsms_read_cnf, AT_CMD_P_CMGR);
  else
#endif
    /*
     * handle Text mode
     */
  { 
     /* querying the type of the SM */
     cmhSMS_SMSQueryType (&mnsms_read_cnf->sms_sdu, &msg_type);

     if (msg_type EQ TP_MTI_SMS_STATUS_REP)
      rCI_Plus_Percent_CMGRTextSP (mnsms_read_cnf, AT_CMD_P_CMGR);
     else
      rCI_Plus_Percent_CMGRText (mnsms_read_cnf, cbm, AT_CMD_P_CMGR);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCMGL    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCMGL call back

*/

GLOBAL void rCI_PercentCMGL  (T_MNSMS_READ_CNF *mnsms_read_cnf)
{
  UBYTE msg_type;
#if defined (SMS_PDU_SUPPORT)
  UBYTE srcId = srcId_cb;
  T_ACI_CMGF_MOD mode;
  
  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
    /*
     * handle PDU mode
     */
    rCI_Plus_Percent_CMGLPdu (mnsms_read_cnf, AT_CMD_P_CMGL);
  else
#endif
    /*
     * handle Text mode
     */
   
  {
    /* querying the type of the SM */
    cmhSMS_SMSQueryType (&mnsms_read_cnf->sms_sdu, &msg_type);

    if (msg_type EQ TP_MTI_SMS_STATUS_REP)
      rCI_Plus_Percent_CMGLTextSP (mnsms_read_cnf, AT_CMD_P_CMGL);
    else
     rCI_Plus_Percent_CMGLText (mnsms_read_cnf, AT_CMD_P_CMGL);
  }
}

LOCAL void rCI_Plus_Percent_CMGLTextSP (T_MNSMS_READ_CNF* mnsms_read_cnf,
                                        T_ACI_AT_CMD cmdId)
{
  UBYTE srcId = srcId_cb;
  T_ACI_CDS_SM* p_st;
  SHORT pos   = 0;
  CHAR*      buf;
  T_MNSMS_STATUS_IND mnsms_status_ind;
  CHAR              cvtdAddr[4*MAX_SMS_ADDR_DIG];
  USHORT            lenCvtdAddr;
  
  TRACE_FUNCTION("rCI_Plus_Percent_CMGLTextSP()");

  /* is the SM already decoded ? */
  if (smsShrdPrm.pDecMsg)
     p_st = (T_ACI_CDS_SM*)smsShrdPrm.pDecMsg;
  else
  {
    ACI_MALLOC(smsShrdPrm.pDecMsg, sizeof(T_ACI_CDS_SM) );
    p_st = (T_ACI_CDS_SM*)smsShrdPrm.pDecMsg;
    memcpy (&mnsms_status_ind.sms_sdu, &mnsms_read_cnf->sms_sdu, 
            sizeof (T_sms_sdu));
    cmhSMS_cpyStatInd ( p_st, &mnsms_status_ind);
  }

  buf=sms_status((T_ACI_SMS_STAT)mnsms_read_cnf->status );
  if (cmdId EQ AT_CMD_CMGL )
  {
    pos = sprintf ( g_sa, "+CMGL: %d", mnsms_read_cnf->rec_num);
  }
  else
  {
    pos=sprintf (g_sa, " %s:  %d", "%CMGL", mnsms_read_cnf->rec_num);
  }
  if ( buf )
  {
    pos += sprintf ( g_sa + pos, ",\"%s\"", buf );   /* STATUS */
  }
  else
  {
    pos += sprintf ( g_sa + pos, "," );
  }
  pos+=sprintf(g_sa+pos,",%d,%d,",p_st->fo, p_st->msg_ref);
  if(strlen(p_st->addr))
  {
    if(p_st->toa.ton EQ TON_Alphanumeric) /* Alphanumeric Destination Address */
    {
      utl_chsetFromGsm ( (UBYTE*)p_st->addr, (USHORT)strlen(p_st->addr), (UBYTE*)cvtdAddr, sizeof(cvtdAddr), &lenCvtdAddr, GSM_ALPHA_Def );
      pos+=sprintf(g_sa+pos,"\"%s\"", cvtdAddr);
    }
    else
    {
      pos+=sprintf(g_sa+pos,"\"%s\"",p_st->addr);
    }
  }
  else
  {
    pos+=sprintf(g_sa+pos,",");
  }
  if( (p_st->toa.ton NEQ TON_NotPresent) AND (p_st->toa.npi NEQ NPI_NotPresent) )
    pos+=sprintf(g_sa+pos,",%d",toa_merge(p_st->toa));
  else
   pos+=sprintf(g_sa+pos,",");

  pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",
   p_st->vpabs_scts.year  [0], p_st->vpabs_scts.year  [1],
   p_st->vpabs_scts.month [0], p_st->vpabs_scts.month [1],
   p_st->vpabs_scts.day   [0], p_st->vpabs_scts.day   [1],
   p_st->vpabs_scts.hour  [0], p_st->vpabs_scts.hour  [1],
   p_st->vpabs_scts.minute[0], p_st->vpabs_scts.minute[1],
   p_st->vpabs_scts.second[0], p_st->vpabs_scts.second[1],
   p_st->vpabs_scts.timezone);

  pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\",%d",
   p_st->vpabs_dt.year  [0], p_st->vpabs_dt.year  [1],
   p_st->vpabs_dt.month [0], p_st->vpabs_dt.month [1],
   p_st->vpabs_dt.day   [0], p_st->vpabs_dt.day   [1],
   p_st->vpabs_dt.hour  [0], p_st->vpabs_dt.hour  [1],
   p_st->vpabs_dt.minute[0], p_st->vpabs_dt.minute[1],
   p_st->vpabs_dt.second[0], p_st->vpabs_dt.second[1],
   p_st->vpabs_dt.timezone , p_st->tp_status);

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

LOCAL void rCI_Plus_Percent_CMGRTextSP (T_MNSMS_READ_CNF* mnsms_read_cnf,
                                        T_ACI_AT_CMD cmdId)
{
  UBYTE srcId = srcId_cb;
  T_ACI_CDS_SM* p_st;
  SHORT pos   = 0;
  CHAR*      buf;
  T_MNSMS_STATUS_IND mnsms_status_ind;
  CHAR              cvtdAddr[4*MAX_SMS_ADDR_DIG];
  USHORT            lenCvtdAddr;
  
  TRACE_FUNCTION("rCI_Plus_Percent_CMGRTextSP()");

  /* is the SM already decoded ? */
  if (smsShrdPrm.pDecMsg)
     p_st = (T_ACI_CDS_SM*)smsShrdPrm.pDecMsg;
  else
  {
    ACI_MALLOC(smsShrdPrm.pDecMsg, sizeof(T_ACI_CDS_SM) );
    p_st = (T_ACI_CDS_SM*)smsShrdPrm.pDecMsg;
    memcpy (&mnsms_status_ind.sms_sdu, &mnsms_read_cnf->sms_sdu, 
            sizeof (T_sms_sdu));
    cmhSMS_cpyStatInd ( p_st, &mnsms_status_ind);
  }

  buf=sms_status((T_ACI_SMS_STAT)mnsms_read_cnf->status );
  if (buf) /* STATUS */
  {
    if (cmdId EQ AT_CMD_CMGR)
    {
      pos=sprintf(g_sa,"+CMGR: \"%s\"",buf);
    }
    else
    {
      pos=sprintf (g_sa, " %s:  \"%s\"", "%CMGR", buf);
    }
  }
  else
  {
    if (cmdId EQ AT_CMD_CMGR)
    {
      pos=sprintf(g_sa,"+CMGR: ,");
    }
    else
    {
      pos=sprintf (g_sa, " %s", "%CMGR: ,");
    }
  }
  pos+=sprintf(g_sa+pos,",%d,%d,",p_st->fo, p_st->msg_ref);
  if(strlen(p_st->addr))
  {
    if(p_st->toa.ton EQ TON_Alphanumeric) /* Alphanumeric Destination Address */
    {
      utl_chsetFromGsm ( (UBYTE*)p_st->addr, (USHORT)strlen(p_st->addr), (UBYTE*)cvtdAddr, sizeof(cvtdAddr), &lenCvtdAddr, GSM_ALPHA_Def );
      pos+=sprintf(g_sa+pos,"\"%s\"", cvtdAddr);
    }
    else
    {
      pos+=sprintf(g_sa+pos,"\"%s\"",p_st->addr);
    }
  }
  else
  {
    pos+=sprintf(g_sa+pos,",");
  }
  if( (p_st->toa.ton NEQ TON_NotPresent) AND (p_st->toa.npi NEQ NPI_NotPresent) )
    pos+=sprintf(g_sa+pos,",%d",toa_merge(p_st->toa));
  else
   pos+=sprintf(g_sa+pos,",");

  pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",
   p_st->vpabs_scts.year  [0], p_st->vpabs_scts.year  [1],
   p_st->vpabs_scts.month [0], p_st->vpabs_scts.month [1],
   p_st->vpabs_scts.day   [0], p_st->vpabs_scts.day   [1],
   p_st->vpabs_scts.hour  [0], p_st->vpabs_scts.hour  [1],
   p_st->vpabs_scts.minute[0], p_st->vpabs_scts.minute[1],
   p_st->vpabs_scts.second[0], p_st->vpabs_scts.second[1],
   p_st->vpabs_scts.timezone);

  pos+=sprintf(g_sa+pos,",\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\",%d",
   p_st->vpabs_dt.year  [0], p_st->vpabs_dt.year  [1],
   p_st->vpabs_dt.month [0], p_st->vpabs_dt.month [1],
   p_st->vpabs_dt.day   [0], p_st->vpabs_dt.day   [1],
   p_st->vpabs_dt.hour  [0], p_st->vpabs_dt.hour  [1],
   p_st->vpabs_dt.minute[0], p_st->vpabs_dt.minute[1],
   p_st->vpabs_dt.second[0], p_st->vpabs_dt.second[1],
   p_st->vpabs_dt.timezone , p_st->tp_status);

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

#ifdef FF_CPHS_REL4
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCFIS    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCFIS call back

*/

GLOBAL void rCI_PercentCFIS  ( T_ACI_CFIS_CFU *cfis)
{
  SHORT     pos   = 0;
  UBYTE   srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentCFIS()");

  pos = sprintf(g_sa,"%s ","%CFIS:");

  pos += sprintf ( g_sa + pos, "%d,%d", cfis->mspId,cfis->cfuStat);

 /*
  *-----------------------------------------------------------------
  * process parameter <number> and <type>
  *-----------------------------------------------------------------
  */
  pos += sprintf ( g_sa + pos, ",\"%s\",%d",
                  cfis->number ,
                  toa_merge ( cfis->type ) );
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentMBI     |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentMBI call back

*/
GLOBAL void rCI_PercentMBI  ( T_ACI_MBI *mbi)
{
  SHORT     pos   = 0;
  UBYTE   srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentMBDN()");

  pos = sprintf(g_sa,"%s ","%MBI:");

 /*
  *-----------------------------------------------------------------
  * process all the identifiers
  *-----------------------------------------------------------------
  */
  pos += sprintf ( g_sa + pos, "%d, %d, %d, %d",
                   mbi->mbdn_id_voice, mbi->mbdn_id_fax, 
                   mbi->mbdn_id_email, mbi->mbdn_id_other);

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentMBDN    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentMBDN call back

*/
GLOBAL void rCI_PercentMBDN  ( T_ACI_MBDN *mbdn)
{
  SHORT     pos   = 0;
  UBYTE   srcId = srcId_cb;
  CHAR   cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT lenCvtdAlpha;

  TRACE_FUNCTION("rCI_PercentMBDN()");

  pos = sprintf(g_sa,"%s ","%MBDN:");

 /*
  *-----------------------------------------------------------------
  * process parameter <number> and <type>
  *-----------------------------------------------------------------
  */
  pos += sprintf ( g_sa + pos, "\"%s\",%d",
                  mbdn->number ,
                  toa_merge ( mbdn->type ) );

  /* As of now Capability Configuration Parameter is not supported 
     we just skip the variable. It has put because in future if we
     support we can just add the variable here */
  pos += sprintf ( g_sa + pos, ",," );

 /*
  *-----------------------------------------------------------------
  * process parameter <alpha>
  *-----------------------------------------------------------------
  */
  if ( mbdn -> alpha[0] NEQ '\0' )
  {
    utl_chsetFromGsm((UBYTE*)mbdn->alpha,
                      (USHORT)strlen(mbdn->alpha),
                      (UBYTE*)cvtdAlpha,
                      sizeof(cvtdAlpha),
                      &lenCvtdAlpha,
                      GSM_ALPHA_Int);
    pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
  }
  else
  {
    pos += sprintf ( g_sa + pos, "," );
  }
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentMWIS    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentMWIS call back

*/

GLOBAL void rCI_PercentMWIS  ( T_ACI_MWIS_MWI *mwis)
{
  SHORT     pos   = 0;
  UBYTE   srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentMWIS()");

  pos = sprintf(g_sa,"%s ","%MWIS:");

  pos += sprintf ( g_sa + pos, "%d,%d,%d,%d,%d", mwis->mwiStat,
                  mwis->mwis_count_voice,mwis->mwis_count_fax,mwis->mwis_count_email,
                  mwis->mwis_count_other);

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentMWI     |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentMWI call back

*/

GLOBAL void rCI_PercentMWI  ( UBYTE mspId , T_ACI_MWIS_MWI *mwis)
{
  SHORT     pos   = 0;
  UBYTE   srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentMWI()");

  if (ati_user_output_cfg[srcId].MWI_stat EQ 1)
  {
    pos = sprintf(g_sa,"%s ","%MWI:");

    pos += sprintf ( g_sa + pos, "%d,%d,%d,%d,%d,%d", mspId,
                   mwis->mwiStat, mwis->mwis_count_voice,mwis->mwis_count_fax,
                   mwis->mwis_count_email,mwis->mwis_count_other);

    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}
#endif /* FF_CPHS_REL4 */

#endif /* ATI_RET_C */
