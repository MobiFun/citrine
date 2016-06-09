/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :  J:\g23m-aci\aci\ati_fd.c
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
|  Purpose :
+-----------------------------------------------------------------------------
*/

#ifndef ACI_FD_CMD_C
#define ACI_FD_CMD_C
#endif

#include "aci_all.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci_lst.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"

#include "aci_fd.h"

#include "aci_mem.h"
#include "aci_prs.h"

#include "ati_int.h"

#ifdef  FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /* FF_ATI_BAT */

#define RPT_LEN   (20)        /* hex report length per line */
#define SIZE_TMP_BUF (5)

LOCAL  void  OutHexReport( UBYTE srcId, USHORT len, UBYTE * buf );
EXTERN T_ATI_RSLT setflowCntr(CHAR* cl, UBYTE srcId);
EXTERN T_ATI_RSLT queflowCntr(CHAR* cl, UBYTE srcId);

#ifdef FF_FAX
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusFLO          |
+--------------------------------------------------------------------+

  PURPOSE : +FLO command (DTE DCE / DCE DTE flow control)
*/

GLOBAL T_ATI_RSLT setatPlusFLO(char *cl, UBYTE srcId)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_flo my_bat_set_plus_flo;

  T_ACI_RX_FLOW_CTRL DCE_by_DTE = RX_FLOW_NotPresent;  /* by TE: Rx flow control */
  T_ACI_RX_FLOW_CTRL DTE_by_DCE = TX_FLOW_NotPresent;  /* by TA: Tx flow control */

  TRACE_FUNCTION("setatPlusFLO() calls bat_send() <=== as APPLICATION");

  cl = parse (cl, "dd", &DCE_by_DTE, &DTE_by_DCE);

  if(!cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

  memset(&my_bat_set_plus_flo, 0, sizeof(my_bat_set_plus_flo));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FLO;
  cmd.params.ptr_set_plus_flo = &my_bat_set_plus_flo;

  my_bat_set_plus_flo.flo_c = DCE_by_DTE;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFLO()");

  src_params->curAtCmd = AT_CMD_FLO;
  return setflowCntr(cl, srcId);

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusFLO          |
+--------------------------------------------------------------------+

  PURPOSE : +FLO command (DTE DCE / DCE DTE flow control)
*/

GLOBAL T_ATI_RSLT queatPlusFLO(char *cl, UBYTE srcId)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  TRACE_FUNCTION("queatPlusFLO()");

  src_params->curAtCmd = AT_CMD_FLO;
  return queflowCntr(cl, srcId);
}
#endif

/************************************************************************
************ Fax and Data only commands...
************************************************************************/

#if defined (FAX_AND_DATA) AND defined (DTI)

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atO                    |
+--------------------------------------------------------------------+

  PURPOSE : O command (return to online state)
*/

GLOBAL T_ATI_RSLT atO(char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("atO()");

  return (map_aci_2_ati_rslt(sAT_O((T_ACI_CMD_SRC)srcId)));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusDR           |
+--------------------------------------------------------------------+

  PURPOSE : +DR command (Select DATA Compression reporting)
*/
GLOBAL T_ATI_RSLT setatPlusDR (char *cl, UBYTE srcId)
{
  SHORT val;
  TRACE_FUNCTION("setatPLusDR()");

  switch(*cl)
  {
    case('0'):
    case('1'):
    {
      val=*cl - '0';
      break;
    }
    default:
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
  }
  cl++;
  if (!(*cl EQ 0 OR *cl EQ ';'))
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
  ati_user_output_cfg[srcId].DR_stat=(UBYTE)val;
  return ATI_CMPL;
}

GLOBAL T_ATI_RSLT queatPlusDR (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPLusDR()");

  resp_disp(srcId, cl,"b",&ati_user_output_cfg[srcId].DR_stat);
  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusDS           |
+--------------------------------------------------------------------+

  PURPOSE : +DS command (Select DATA Compression)
*/

GLOBAL T_ATI_RSLT setatPlusDS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret    = AT_FAIL;
  T_ACI_DS_DIR dir    = DS_DIR_NotPresent;
  T_ACI_DS_COMP comp  = DS_COMP_NotPresent;
  LONG  maxDict       = ACI_NumParmNotPresent;
  SHORT maxStr        = ACI_NumParmNotPresent ;

  cl = parse(cl, "dddr", &dir, &comp, &maxDict, &maxStr);
  if (!cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_ds my_bat_set_plus_ds;

  TRACE_FUNCTION("atPLusDS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_ds, 0, sizeof(my_bat_set_plus_ds));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_DS;
  cmd.params.ptr_set_plus_ds = &my_bat_set_plus_ds;

  my_bat_set_plus_ds.dir = dir;
  my_bat_set_plus_ds.neg ;
  my_bat_set_plus_ds.p1; 
  my_bat_set_plus_ds.p2; 

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("atPLusDS()");

  ret = sAT_PlusDS((T_ACI_CMD_SRC)srcId,dir,comp,maxDict,maxStr);

  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusDS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_DS_DIR dir=DS_DIR_NotPresent;
  T_ACI_DS_COMP comp=DS_COMP_NotPresent;
  LONG  maxDict=ACI_NumParmNotPresent;
  SHORT maxStr=ACI_NumParmNotPresent ;

  TRACE_FUNCTION("atPLusDS()");

  ret = qAT_PlusDS((T_ACI_CMD_SRC)srcId,&dir,&comp,&maxDict,&maxStr);
  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl,"eels",&dir,&comp,&maxDict,&maxStr);
  }
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusCRLP         |
+--------------------------------------------------------------------+

  PURPOSE : +CRLP command (Select RADIO LINK Protokoll)
*/

GLOBAL T_ATI_RSLT setatPlusCRLP(char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT iws =- 1, mws = -1, t1 = -1, n2 = -1;  /* !!! */

  cl = parse(cl, "rrrr", &iws, &mws, &t1, &n2);
  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_crlp my_bat_set_plus_crlp;

  TRACE_FUNCTION("setatPLusCRLP() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_crlp, 0, sizeof(my_bat_set_plus_crlp));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CRLP;
  cmd.params.ptr_set_plus_crlp = &my_bat_set_plus_crlp;

  my_bat_set_plus_crlp.iws = iws;
  my_bat_set_plus_crlp.mws = mws;
  my_bat_set_plus_crlp.t1  = t1;
  my_bat_set_plus_crlp.n2  = n2;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCRLP()");

  ret = sAT_PlusCRLP((T_ACI_CMD_SRC)srcId,iws,mws,t1,n2);
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusCRLP(char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT iws=-1,mws=-1,t1=-1,n2=-1;  /* !!! */

  TRACE_FUNCTION("queatPLusCRLP()");

  ret = qAT_PlusCRLP((T_ACI_CMD_SRC)srcId,&iws,&mws,&t1,&n2);
  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl,"ssss",&iws,&mws,&t1,&n2);
  }
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));
}

#ifdef FF_FAX

/*
 *
 *
 *---------------------------------------- FAX Commands ------------------------------------------
 *
 *
 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFAP          |
+--------------------------------------------------------------------+

  PURPOSE : +FAP command (address and polling capabilities )
*/

GLOBAL T_ATI_RSLT setatPlusFAP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FAP_VAL sub=FAP_VAL_NotPresent,
                sep=FAP_VAL_NotPresent,
                pwd=FAP_VAL_NotPresent;

  cl = parse(cl,"ddd",&sub,&sep,&pwd);
  if (!cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fap my_bat_set_plus_fap;

  TRACE_FUNCTION("setatPlusFAP() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fap, 0, sizeof(my_bat_set_plus_fap));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FAP;
  cmd.params.ptr_set_plus_fap = &my_bat_set_plus_fap;

  my_bat_set_plus_fap.sub = sub;
  my_bat_set_plus_fap.sep = sep;
  my_bat_set_plus_fap.pwd = pwd;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFAP()");

  ret = sAT_PlusFAP((T_ACI_CMD_SRC)srcId,sub,sep,pwd);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFAP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FAP_VAL sub=FAP_VAL_NotPresent,
                sep=FAP_VAL_NotPresent,
                pwd=FAP_VAL_NotPresent;

  TRACE_FUNCTION("queatPlusFAP()");

  ret = qAT_PlusFAP((T_ACI_CMD_SRC)srcId,&sub,&sep,&pwd);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FAP:%d,%d,%d",sub,sep,pwd);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFBU          |
+--------------------------------------------------------------------+

  PURPOSE : +FBU
*/

GLOBAL T_ATI_RSLT setatPlusFBU(char *cl, UBYTE srcId)
{
  SHORT fbu = -1;

  cl = parse(cl,"r",&fbu);
  if (!cl OR fbu > 1)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fbu my_bat_set_plus_fbu;

  TRACE_FUNCTION("setatPlusFBU() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fbu, 0, sizeof(my_bat_set_plus_fbu));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FBU;
  cmd.params.ptr_set_plus_fbu = &my_bat_set_plus_fbu;

  my_bat_set_plus_fbu.value = fbu;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFBU()");

  if (fbu NEQ -1)
  {
    fd.FBU_stat = (char) fbu;
  }
  return ATI_CMPL;

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFBU(char *cl, UBYTE srcId)
{

  TRACE_FUNCTION("queatPlusFBU()");

  sprintf(g_sa,"+FBU:%d",fd.FBU_stat);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFCC          |
+--------------------------------------------------------------------+

  PURPOSE :

  ITU-T.32/8.5.1.1 DCE capabilities parameters, +FCC

  – Write syntax: +FCC=VR,BR,WD,LN,DF,EC,BF,ST,JP
  – Valid values: see Table 21
  – Default values: set by manufacturer
  – Mandatory values: as required by Recommendation T.30

  +FCC allows the DTE to sense and constrain the capabilities of the facsimile DCE,
  from the choices defined in Table 2/T.30.
  When +FCC is modified by the DTE, the DCE shall copy +FCC into +FIS.

*/

GLOBAL T_ATI_RSLT setatPlusFCC (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;

  T_ACI_F_VR  vr = F_VR_NotPresent;
  T_ACI_F_BR  br = F_BR_NotPresent;
  T_ACI_F_WD  wd = F_WD_NotPresent;
  T_ACI_F_LN  ln = F_LN_NotPresent;
  T_ACI_F_DF  df = F_DF_NotPresent;
  T_ACI_F_EC  ec = F_EC_NotPresent;
  T_ACI_F_BF  bf = F_BF_NotPresent;
  T_ACI_F_ST  st = F_ST_NotPresent;
  T_ACI_F_JP  jp = F_JP_NotPresent;

  UBYTE      bvr = NOT_PRESENT_8BIT,
             bbf = NOT_PRESENT_8BIT,
             bjp = NOT_PRESENT_8BIT;

  cl = parse(cl,"xdddddxdx",&bvr,&br,&wd,&ln,&df,&ec,&bbf,&st,&bjp);

  if ( !cl )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

  if (bvr NEQ NOT_PRESENT_8BIT)
  {
    vr = (T_ACI_F_VR)bvr;
  }
  if (bbf NEQ NOT_PRESENT_8BIT)
  {
    bf = (T_ACI_F_BF)bbf;
  }
  if (bjp NEQ NOT_PRESENT_8BIT)
  {
    jp = (T_ACI_F_JP)bjp;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fcc my_bat_set_plus_fcc;

  TRACE_FUNCTION("setatPlusFCC() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fcc, 0, sizeof(my_bat_set_plus_fcc));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FCC;
  cmd.params.ptr_set_plus_fcc = &my_bat_set_plus_fcc;
     
  my_bat_set_plus_fcc.vr = vr;
  my_bat_set_plus_fcc.br = br;
  my_bat_set_plus_fcc.wd = wd;
  my_bat_set_plus_fcc.ln = ln;
  my_bat_set_plus_fcc.df = df;
  my_bat_set_plus_fcc.ec = ec;
  my_bat_set_plus_fcc.bf = bf;
  my_bat_set_plus_fcc.st = st;
  my_bat_set_plus_fcc.jp = jp;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFCC()");

  ret = sAT_PlusFCC((T_ACI_CMD_SRC)srcId,vr,br,wd,ln,df,ec,bf,st,jp);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFCC (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_F_VR  vr = F_VR_NotPresent;
  T_ACI_F_BR  br = F_BR_NotPresent;
  T_ACI_F_WD  wd = F_WD_NotPresent;
  T_ACI_F_LN  ln = F_LN_NotPresent;
  T_ACI_F_DF  df = F_DF_NotPresent;
  T_ACI_F_EC  ec = F_EC_NotPresent;
  T_ACI_F_BF  bf = F_BF_NotPresent;
  T_ACI_F_ST  st = F_ST_NotPresent;
  T_ACI_F_JP  jp = F_JP_NotPresent;
 
  TRACE_FUNCTION("queatPlusFCC()");

  ret = qAT_PlusFCC((T_ACI_CMD_SRC)srcId, &vr,&br,&wd,&ln,&df,&ec,&bf,&st,&jp);
  if (ret EQ AT_CMPL)
  {
     sprintf(g_sa,"+FCC:%X,%d,%d,%d,%d,%d,%X,%d,%X",vr,br,wd,ln,df,ec,bf,st,jp);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFCLASS       |
+--------------------------------------------------------------------+

  PURPOSE : +FCLASS command (Service Class Indication)
*/

GLOBAL T_ATI_RSLT setatPlusFCLASS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FCLASS_CLASS class_type = FCLASS_CLASS_NotPresent;

  switch (*cl)
  {
  case('0'):
    {
      cl++;
      class_type = FCLASS_CLASS_Data;
      break;
    }
  case('2'):
    {
      cl++;
      if (*cl EQ '.')
      {
        cl++;
        if (*cl EQ '0')
        {
          cl++;
          class_type = FCLASS_CLASS_Fax20;
          break;
        }
      }
      cmdAtError(atError);
      return ATI_FAIL;
    }
  case('8'):
    {
      cl++;
      class_type = FCLASS_CLASS_Voice;
      break;
    }
  default:
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
  }
  if (*cl NEQ '\0' AND *cl NEQ ';')
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fclass my_bat_set_plus_fclass;

  TRACE_FUNCTION("setatPlusFCLASS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fclass, 0, sizeof(my_bat_set_plus_fclass));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FCLASS;
  cmd.params.ptr_set_plus_fclass = &my_bat_set_plus_fclass;

  my_bat_set_plus_fclass.n = class_type;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFCLASS()");

  ret = sAT_PlusFCLASS((T_ACI_CMD_SRC)srcId, class_type);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFCLASS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FCLASS_CLASS class_type = FCLASS_CLASS_NotPresent;

  TRACE_FUNCTION("atPlusFCLASS()");

  ret = qAT_PlusFCLASS((T_ACI_CMD_SRC)srcId, &class_type);
  if (ret EQ AT_CMPL)
  {
    if (class_type EQ 20)
    {
      sprintf(g_sa,"%s","2.0");
    }
    else
    {
      sprintf(g_sa,"%d",class_type);
    }
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFCQ          |
+--------------------------------------------------------------------+

  PURPOSE : +FCQ command (quality control)
*/

GLOBAL T_ATI_RSLT setatPlusFCQ (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FCQ_RQ rq=FCQ_RQ_NotPresent;
  T_ACI_FCQ_TQ tq=FCQ_TQ_NotPresent;

  cl = parse(cl,"dd",&rq,&tq);
  if ( !cl )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fcq my_bat_set_plus_fcq;

  TRACE_FUNCTION("setatPlusFCQ() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fcq, 0, sizeof(my_bat_set_plus_fcq));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FCQ;
  cmd.params.ptr_set_plus_fcq = &my_bat_set_plus_fcq;

  my_bat_set_plus_fcq.rq = rq;
  my_bat_set_plus_fcq.tq = tq;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFCQ()");

  ret = sAT_PlusFCQ((T_ACI_CMD_SRC)srcId,rq,tq);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFCQ (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FCQ_RQ  rq=FCQ_RQ_NotPresent;
  T_ACI_FCQ_TQ  tq=FCQ_TQ_NotPresent;

  TRACE_FUNCTION("queatPlusFCQ()");

  ret = qAT_PlusFCQ((T_ACI_CMD_SRC)srcId, &rq,&tq);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s%d,%d","+FCQ:",rq,tq);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFCR          |
+--------------------------------------------------------------------+

  PURPOSE : +FCR command (Receive capability)
*/

GLOBAL T_ATI_RSLT setatPlusFCR (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FCR_VAL val = FCR_VAL_NotPresent;

  cl = parse(cl,"d",&val);
  if ( !cl )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fcr my_bat_set_plus_fcr;

  TRACE_FUNCTION("setatPlusFCR() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fcr, 0, sizeof(my_bat_set_plus_fcr));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FCR;
  cmd.params.ptr_set_plus_fcr = &my_bat_set_plus_fcr;

  my_bat_set_plus_fcr.value = val;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFCR()");

  ret = sAT_PlusFCR((T_ACI_CMD_SRC)srcId,val);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFCR (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FCR_VAL val = FCR_VAL_NotPresent;

  TRACE_FUNCTION("queatPlusFCR()");

  ret = qAT_PlusFCR((T_ACI_CMD_SRC)srcId, &val);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FCR:%d",val);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFCS          |
+--------------------------------------------------------------------+

  PURPOSE:

  ITU-T.32/8.5.1.3 Current session results, +FCS

  – Read syntax: +FCS?
  – DCE response: VR,BR,WD,LN,DF,EC,BF,ST,JP
  – Valid values: see Table 21
  – Default values: 0,0,0,0,0,0,0,0,0
  – Mandatory values: as required by Recommendation T.30

  The +FCS parameter is loaded with the negotiated T.30 parameters for the current session.
  A transmitting DCE generates DCS;
  a receiving DCE gets DCS from the remote station.

  The DTE may only read this parameter.
  The DCE shall set the +FCS parameter to the default values on DCE initialization,
  on +FIP command execution, and at the end of a session.

  The contents of +FCS are spontaneously reported
  during execution of +FDR (8.3.4) or +FDT (8.3.3) commands,
  by the +FCS:VR,BR,WD,LN,DF,EC,BF,ST,JP response,
  using the same compound parameter format. See 8.4.2.1.

  NOTE – The use of additional subparameters for the +FCS,
  in order to support future T.30 features, is for further study.
*/

GLOBAL T_ATI_RSLT queatPlusFCS(char *cl, UBYTE srcId)
{
  T_ACI_RETURN       ret = AT_FAIL;
  T_ACI_F_VR         vr;
  T_ACI_F_BR         br;
  T_ACI_F_WD         wd;
  T_ACI_F_LN         ln;
  T_ACI_F_DF         df;
  T_ACI_F_EC         ec;
  T_ACI_F_BF         bf;
  T_ACI_F_ST         st;
  T_ACI_F_JP         jp;

  TRACE_FUNCTION("queatPlusFCS()");

  ret = qAT_PlusFCS((T_ACI_CMD_SRC)srcId,&vr,&br,&wd,&ln,&df,&ec,&bf,&st,&jp);

  if(ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FCS:%X,%d,%d,%d,%d,%d,%X,%d,%X",vr,br,wd,ln,df,ec,bf,st,jp);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFEA          |
+--------------------------------------------------------------------+

  PURPOSE : +FEA
*/

GLOBAL T_ATI_RSLT setatPlusFEA(char *cl, UBYTE srcId)
{
  SHORT fea = -1;

  cl = parse(cl, "r", &fea);
  if (!cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fea my_bat_set_plus_fea;

  TRACE_FUNCTION("setatPlusFEA() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fea, 0, sizeof(my_bat_set_plus_fea));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FEA;
  cmd.params.ptr_set_plus_fea = &my_bat_set_plus_fea;

  my_bat_set_plus_fea.value = fea;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFEA()");

  if (fea NEQ -1)
  {
    fd.FEA_stat = (char) fea;
  }
  return ATI_CMPL;

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFEA(char *cl, UBYTE srcId)
{

  TRACE_FUNCTION("queatPlusFEA()");

  sprintf(g_sa,"+FEA:%d", fd.FEA_stat);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFFC          |
+--------------------------------------------------------------------+

  PURPOSE : +FFC command (format conversion)
*/

GLOBAL T_ATI_RSLT setatPlusFFC (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FFC_VRC vrc=FFC_VRC_NotPresent;
  T_ACI_FFC_DFC dfc=FFC_DFC_NotPresent;
  T_ACI_FFC_LNC lnc=FFC_LNC_NotPresent;
  T_ACI_FFC_WDC wdc=FFC_WDC_NotPresent;

  cl = parse(cl,"dddd",&vrc,&dfc,&lnc,&wdc);
  if ( !cl )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_ffc my_bat_set_plus_ffc;

  TRACE_FUNCTION("setatPlusFFC() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_ffc, 0, sizeof(my_bat_set_plus_ffc));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FFC;
  cmd.params.ptr_set_plus_ffc = &my_bat_set_plus_ffc;

  my_bat_set_plus_ffc.vrc = vrc;
  my_bat_set_plus_ffc.dfc = dfc;
  my_bat_set_plus_ffc.lnc = lnc;
  my_bat_set_plus_ffc.wdc = wdc;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFFC()");

  ret = sAT_PlusFFC((T_ACI_CMD_SRC)srcId,vrc,dfc,lnc,wdc);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFFC (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FFC_VRC vrc=FFC_VRC_NotPresent;
  T_ACI_FFC_DFC dfc=FFC_DFC_NotPresent;
  T_ACI_FFC_LNC lnc=FFC_LNC_NotPresent;
  T_ACI_FFC_WDC wdc=FFC_WDC_NotPresent;

  TRACE_FUNCTION("atPlusFFC()");

  ret = qAT_PlusFFC((T_ACI_CMD_SRC)srcId,&vrc,&dfc,&lnc,&wdc);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FFC:%d,%d,%d,%d",vrc,dfc,lnc,wdc);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFHS          |
+--------------------------------------------------------------------+

  PURPOSE : +FHS command (query termination status)
*/

GLOBAL T_ATI_RSLT queatPlusFHS(char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_FHS_STAT stat;

  TRACE_FUNCTION("queatPlusFHS()");

  ret = qAT_PlusFHS((T_ACI_CMD_SRC)srcId,&stat);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FHS:%02X",stat);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFIE          |
+--------------------------------------------------------------------+

  PURPOSE : +FIE command (procedure interrupt enable)
*/

GLOBAL T_ATI_RSLT setatPlusFIE (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FIE_VAL val=FIE_VAL_NotPresent;

  cl = parse(cl,"d",&val);
  if ( !cl )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fie my_bat_set_plus_fie;

  TRACE_FUNCTION("setatPlusFIE() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fie, 0, sizeof(my_bat_set_plus_fie));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FIE;
  cmd.params.ptr_set_plus_fie = &my_bat_set_plus_fie;

  my_bat_set_plus_fie.value = val;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("atPlusFIE()");

  ret = sAT_PlusFIE((T_ACI_CMD_SRC)srcId,val);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFIE (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FIE_VAL val=FIE_VAL_NotPresent;

  TRACE_FUNCTION("queatPlusFIE()");

  ret = qAT_PlusFIE((T_ACI_CMD_SRC)srcId, &val);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FIE:%d",val);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFIS          |
+--------------------------------------------------------------------+

  PURPOSE :

  ITU-T.32/8.5.1.2 Current session parameters, +FIS

  – Write syntax: +FIS=VR,BR,WD,LN,DF,EC,BF,ST,JP
  – Valid values: see Table 21
  – Default values: set by manufacturer
  – Mandatory values: as required by Recommendation T.30.

  The +FIS parameter allows the DTE to sense and constrain
  the capabilities used for the current session.
  The DCE uses +FIS to generate DIS or DTC messages directly,
  and uses +FIS and received DIS messages to generate DCS messages.

  The DCE shall set the +FIS parameter from the +FCC parameter
  on DCE initialization,
  upon +FIP command execution, when +FCC is written,
  and at the end of a session.
*/

GLOBAL T_ATI_RSLT setatPlusFIS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_F_VR  vr = F_VR_NotPresent;
  T_ACI_F_BR  br = F_BR_NotPresent;
  T_ACI_F_WD  wd = F_WD_NotPresent;
  T_ACI_F_LN  ln = F_LN_NotPresent;
  T_ACI_F_DF  df = F_DF_NotPresent;
  T_ACI_F_EC  ec = F_EC_NotPresent;
  T_ACI_F_BF  bf = F_BF_NotPresent;
  T_ACI_F_ST  st = F_ST_NotPresent;
  T_ACI_F_JP  jp = F_JP_NotPresent;

  UBYTE bvr = NOT_PRESENT_8BIT,
        bbf = NOT_PRESENT_8BIT,
        bjp = NOT_PRESENT_8BIT;


  cl = parse(cl,"xdddddxdx",&bvr,&br,&wd,&ln,&df,&ec,&bbf,&st,&bjp);

  if (bvr NEQ NOT_PRESENT_8BIT)
    vr = (T_ACI_F_VR)bvr;

  if (bbf NEQ NOT_PRESENT_8BIT)
    bf=(T_ACI_F_BF)bbf;

  if (bjp NEQ NOT_PRESENT_8BIT)
    jp=(T_ACI_F_JP)bjp;

  if ( !cl )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fis my_bat_set_plus_fis;

  TRACE_FUNCTION("setatPlusFIS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fis, 0, sizeof(my_bat_set_plus_fis));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FIS;
  cmd.params.ptr_set_plus_fis = &my_bat_set_plus_fis;

  my_bat_set_plus_fis.vr = vr;
  my_bat_set_plus_fis.br = br;
  my_bat_set_plus_fis.wd = wd;
  my_bat_set_plus_fis.ln = ln;
  my_bat_set_plus_fis.df = df;
  my_bat_set_plus_fis.ec = ec;
  my_bat_set_plus_fis.bf = bf;
  my_bat_set_plus_fis.st = st;
  my_bat_set_plus_fis.jp = jp;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFIS()");

  ret = sAT_PlusFIS((T_ACI_CMD_SRC)srcId,vr,br,wd,ln,df,ec,bf,st,jp);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFIS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_F_VR  vr = F_VR_NotPresent;
  T_ACI_F_BR  br = F_BR_NotPresent;
  T_ACI_F_WD  wd = F_WD_NotPresent;
  T_ACI_F_LN  ln = F_LN_NotPresent;
  T_ACI_F_DF  df = F_DF_NotPresent;
  T_ACI_F_EC  ec = F_EC_NotPresent;
  T_ACI_F_BF  bf = F_BF_NotPresent;
  T_ACI_F_ST  st = F_ST_NotPresent;
  T_ACI_F_JP  jp = F_JP_NotPresent;
  
  TRACE_FUNCTION("queatPlusFIS()");

  ret = qAT_PlusFIS((T_ACI_CMD_SRC)srcId, &vr,&br,&wd,&ln,&df,&ec,&bf,&st,&jp);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FIS:%X,%d,%d,%d,%d,%d,%X,%d,%X",vr,br,wd,ln,df,ec,bf,st,jp);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFLI          |
+--------------------------------------------------------------------+

  PURPOSE : +FLI command (Local Station ID)
*/

GLOBAL T_ATI_RSLT setatPlusFLI (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  char idstr[MAX_ID_CHAR]={0};

  cl = parse(cl, "s", (LONG)MAX_ID_CHAR, idstr);
  if ( !cl )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fli my_bat_set_plus_fli;

  TRACE_FUNCTION("setatPlusFLI() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fli, 0, sizeof(my_bat_set_plus_fli));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FLI;
  cmd.params.ptr_set_plus_fli = &my_bat_set_plus_fli;

  my_bat_set_plus_fli.c_id_str = strlen(idstr);
  memcpy(my_bat_set_plus_fli.id_str, idstr, my_bat_set_plus_fli.c_id_str);

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFLI()");

  ret = sAT_PlusFLI((T_ACI_CMD_SRC)srcId,idstr);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFLI (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  char idstr[MAX_ID_CHAR]={0};

  TRACE_FUNCTION("queatPlusFLI()");

  ret = qAT_PlusFLI((T_ACI_CMD_SRC)srcId, idstr);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FLI:\"%s\"",idstr);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFLP          |
+--------------------------------------------------------------------+

  PURPOSE : +FLP command (document to poll indication)
*/

GLOBAL T_ATI_RSLT setatPlusFLP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FLP_VAL val=FLP_VAL_NotPresent;

  cl = parse(cl,"d",&val);
  if ( !cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_flp my_bat_set_plus_flp;

  TRACE_FUNCTION("setatPlusFLP() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_flp, 0, sizeof(my_bat_set_plus_flp));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FLP;
  cmd.params.ptr_set_plus_flp = &my_bat_set_plus_flp;

  my_bat_set_plus_flp.value = val;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFLP()");

  ret = sAT_PlusFLP((T_ACI_CMD_SRC)srcId,val);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFLP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FLP_VAL val=FLP_VAL_NotPresent;

  TRACE_FUNCTION("queatPlusFLP()");

  ret = qAT_PlusFLP((T_ACI_CMD_SRC)srcId, &val);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FLP:%d",val);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFMS          |
+--------------------------------------------------------------------+

  PURPOSE : +FMS command (minimum phase C speed)
*/

GLOBAL T_ATI_RSLT setatPlusFMS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_F_BR val=F_BR_NotPresent;

  cl = parse(cl,"d",&val);
  if ( !cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fms my_bat_set_plus_fms;

  TRACE_FUNCTION("setatPlusFMS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fms, 0, sizeof(my_bat_set_plus_fms));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FMS;
  cmd.params.ptr_set_plus_fms = &my_bat_set_plus_fms;

  my_bat_set_plus_fms.value = val;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFMS()");

  ret = sAT_PlusFMS((T_ACI_CMD_SRC)srcId, val);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFMS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_F_BR val=F_BR_NotPresent;

  TRACE_FUNCTION("queatPlusFMS()");

  ret=qAT_PlusFMS((T_ACI_CMD_SRC)srcId, &val);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FMS:%d",val);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFNR          |
+--------------------------------------------------------------------+

  PURPOSE : +FNR
*/

GLOBAL T_ATI_RSLT setatPlusFNR(char *cl, UBYTE srcId)
{
  SHORT r=-1,t=-1,i=-1,n=-1;

  cl = parse(cl,"rrrr",&r,&t,&i,&n);
  if (!cl OR r > 1 OR t > 1 OR i > 1 OR  n > 1
  OR (r EQ -1 AND t EQ -1 AND i EQ -1 AND n EQ -1))
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
/*
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fnr my_bat_set_plus_fnr; //???

  TRACE_FUNCTION("setatPlusFNR() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fnr, 0, sizeof(my_bat_set_plus_fnr));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FNR;
  cmd.params.ptr_set_plus_fnr = &my_bat_set_plus_fnr;

  my_bat_set_plus_fnr.value = val;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; 
*/
  TRACE_EVENT("setatPlusFNR(): is obsolete ? <=== !!! consult KJF");
  return ATI_FAIL;
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFNR()");
  if(r NEQ -1)
    fd.FNR.rpr_stat = (char) r;
  if(t NEQ -1)
    fd.FNR.tpr_stat = (char) t;
  if(i NEQ -1)
    fd.FNR.idr_stat = (char) i;
  if(n NEQ -1)
    fd.FNR.nsr_stat = (char) n;
  return ATI_CMPL;

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFNR(char *cl, UBYTE srcId)
{
  
  TRACE_FUNCTION("queatPlusFNR()");
  sprintf(g_sa,"+FNR:%d,%d,%d,%d",fd.FNR.rpr_stat,fd.FNR.tpr_stat,fd.FNR.idr_stat,fd.FNR.nsr_stat);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFND          |
+--------------------------------------------------------------------+

  PURPOSE : +FND command (NSF message data indication)
*/

GLOBAL T_ATI_RSLT setatPlusFND (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FND_VAL val=FND_VAL_NotPresent;

  cl = parse(cl,"d",&val);
  if ( !cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
/*
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fnd my_bat_set_plus_fnd; //???

  TRACE_FUNCTION("setatPlusFND() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fnd, 0, sizeof(my_bat_set_plus_fnd));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FND;
  cmd.params.ptr_set_plus_fnd = &my_bat_set_plus_fnd;

  my_bat_set_plus_fnd.value = val;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; 
*/
  TRACE_EVENT("setatPlusFND(): is obsolete ? <=== !!! consult KJF");
  return ATI_FAIL;
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFND()");

  ret = sAT_PlusFND((T_ACI_CMD_SRC)srcId,val);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFND (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FND_VAL val=FND_VAL_NotPresent;

  TRACE_FUNCTION("queatPlusFND()");

  ret = qAT_PlusFND((T_ACI_CMD_SRC)srcId, &val);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FND:%d",val);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFNS          |
+--------------------------------------------------------------------+

  PURPOSE : +FNS command (non standard facilities)
*/

GLOBAL T_ATI_RSLT setatPlusFNS (char *cl, UBYTE srcId)
{
        char        *me="+FNS:";   /* weird...!! */
        T_ACI_RETURN ret = AT_FAIL;
        UBYTE        len=0;
        UBYTE        nsf[90];
  LOCAL char         nsf_str[269]={0};
        char         buf [3];
        SHORT        i=0;

  cl = parse(cl, "s", (LONG)269, nsf_str);
  if ( !cl )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

  compact(nsf_str,(USHORT)strlen(nsf_str));
  while (nsf_str[i] NEQ '\0')
  {
    buf[0]=nsf_str[i++];
    buf[1]=nsf_str[i++];
    buf[2]='\0';
    me = parse (buf, "x", &nsf[len++]);
    if (!me)
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fns my_bat_set_plus_fns;

  TRACE_FUNCTION("setatPlusFNS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fns, 0, sizeof(my_bat_set_plus_fns));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FNS;
  cmd.params.ptr_set_plus_fns = &my_bat_set_plus_fns;

  my_bat_set_plus_fns.v_nsf = TRUE;
  my_bat_set_plus_fns.c_nsf = len;
  memcpy(my_bat_set_plus_fns.nsf, nsf, len);

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFNS()");

  ret = sAT_PlusFNS((T_ACI_CMD_SRC)srcId, len, nsf);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFNS (char *cl, UBYTE srcId)
{
        T_ACI_RETURN ret = AT_FAIL;
        UBYTE        len=0;
        UBYTE        nsf[90];

  TRACE_FUNCTION("queatPlusFNS()");

  ret=qAT_PlusFNS((T_ACI_CMD_SRC)srcId, &len, nsf);
  if (ret EQ AT_CMPL)
  {
    strcpy(g_sa,"+FNS:");
    OutHexReport(srcId, (USHORT)len, nsf);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFPA          |
+--------------------------------------------------------------------+

  PURPOSE : +FPA command (selective polling address string)
*/

GLOBAL T_ATI_RSLT setatPlusFPA (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  char sepstr[MAX_ID_CHAR]={0};

  cl  =parse(cl,"s",(LONG)MAX_ID_CHAR,sepstr);
  if (!cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fpa my_bat_set_plus_fpa;

  TRACE_FUNCTION("setatPlusFPA() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fpa, 0, sizeof(my_bat_set_plus_fpa));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FPA;
  cmd.params.ptr_set_plus_fpa = &my_bat_set_plus_fpa;

  my_bat_set_plus_fpa.c_spa_str = strlen(sepstr);
  memcpy(my_bat_set_plus_fpa.spa_str, sepstr, my_bat_set_plus_fpa.c_spa_str);

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFPA()");

  ret = sAT_PlusFPA((T_ACI_CMD_SRC)srcId, sepstr);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFPA (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  char sepstr[MAX_ID_CHAR]={0};

  TRACE_FUNCTION("atPlusFPA()");

  ret = qAT_PlusFPA((T_ACI_CMD_SRC)srcId, sepstr);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FPA:\"%s\"",sepstr);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFPI          |
+--------------------------------------------------------------------+

  PURPOSE : +FPI command (local fax station id string)
*/

GLOBAL T_ATI_RSLT setatPlusFPI (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  char idstr[MAX_ID_CHAR] = {0};

  cl = parse(cl, "s", (LONG)MAX_ID_CHAR, idstr);
  if (!cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fpi my_bat_set_plus_fpi;

  TRACE_FUNCTION("setatPlusFPI() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fpi, 0, sizeof(my_bat_set_plus_fpi));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FPI;
  cmd.params.ptr_set_plus_fpi = &my_bat_set_plus_fpi;

  my_bat_set_plus_fpi.c_id_str = strlen(idstr);
  memcpy(my_bat_set_plus_fpi.id_str, idstr, my_bat_set_plus_fpi.c_id_str);

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFPI()");

  ret = sAT_PlusFPI((T_ACI_CMD_SRC)srcId, idstr);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFPI (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  char idstr[MAX_ID_CHAR]={0};

  TRACE_FUNCTION("queatPlusFPI()");

  ret = qAT_PlusFPI((T_ACI_CMD_SRC)srcId, idstr);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FPI:\"%s\"",idstr);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFPS          |
+--------------------------------------------------------------------+

  PURPOSE : +FPS command (page status)
*/

GLOBAL T_ATI_RSLT setatPlusFPS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FPS_PPR ppr=FPS_PPR_NotPresent;

  cl = parse(cl,"d",&ppr);
  if ( !cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fps my_bat_set_plus_fps;

  TRACE_FUNCTION("setatPlusFPS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fps, 0, sizeof(my_bat_set_plus_fps));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FPS;
  cmd.params.ptr_set_plus_fps = &my_bat_set_plus_fps;

  my_bat_set_plus_fps.ppr = ppr;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFPS()");

  ret = sAT_PlusFPS((T_ACI_CMD_SRC)srcId, ppr);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFPS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FPS_PPR ppr=FPS_PPR_NotPresent;

  TRACE_FUNCTION("queatPlusFPS()");

  ret = qAT_PlusFPS((T_ACI_CMD_SRC)srcId, &ppr);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FPS:%d",ppr);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFPW          |
+--------------------------------------------------------------------+

  PURPOSE : +FPW command (password string)
*/

GLOBAL T_ATI_RSLT setatPlusFPW (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  char pwdstr[MAX_ID_CHAR]={0};

  cl = parse(cl,"s",(LONG)MAX_ID_CHAR,pwdstr);
  if (!cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fpw my_bat_set_plus_fpw;

  TRACE_FUNCTION("setatPlusFPW() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fpw, 0, sizeof(my_bat_set_plus_fpw));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FPW;
  cmd.params.ptr_set_plus_fpw = &my_bat_set_plus_fpw;

  my_bat_set_plus_fpw.c_pw_str = strlen(pwdstr);
  memcpy(my_bat_set_plus_fpw.pw_str, pwdstr, my_bat_set_plus_fpw.c_pw_str);

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFPW()");

  ret = sAT_PlusFPW((T_ACI_CMD_SRC)srcId, pwdstr);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFPW (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  char pwdstr[MAX_ID_CHAR]={0};

  TRACE_FUNCTION("queatPlusFPW()");

  ret = qAT_PlusFPW((T_ACI_CMD_SRC)srcId, pwdstr);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FPW:\"%s\"",pwdstr);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFRQ          |
+--------------------------------------------------------------------+

  PURPOSE : +FRQ command (quality control)
*/

GLOBAL T_ATI_RSLT setatPlusFRQ (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  UBYTE        pgl = NOT_PRESENT_8BIT,
               cbl = NOT_PRESENT_8BIT;

  cl = parse(cl,"xx",&pgl,&cbl);
  if ( !cl )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_frq my_bat_set_plus_frq;

  TRACE_FUNCTION("setatPlusFRQ() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_frq, 0, sizeof(my_bat_set_plus_frq));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FRQ;
  cmd.params.ptr_set_plus_frq = &my_bat_set_plus_frq;

  my_bat_set_plus_frq.pgl = pgl;
  my_bat_set_plus_frq.cbl = cbl;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFRQ()");

  ret = sAT_PlusFRQ((T_ACI_CMD_SRC)srcId, pgl, cbl);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFRQ (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT        pgl = ACI_NumParmNotPresent,
               cbl = ACI_NumParmNotPresent;

  TRACE_FUNCTION("queatPlusFRQ()");

  ret = qAT_PlusFRQ((T_ACI_CMD_SRC)srcId, &pgl,&cbl);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FRQ:%X,%X",pgl,cbl);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFCT          |
+--------------------------------------------------------------------+

  PURPOSE : +FCT command (phase C response timeout)
*/

GLOBAL T_ATI_RSLT setatPlusFCT (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  UBYTE        val = NOT_PRESENT_8BIT;

  cl = parse(cl,"x",&val);
  if ( !cl )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fct my_bat_set_plus_fct;

  TRACE_FUNCTION("setatPlusFCT() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fct, 0, sizeof(my_bat_set_plus_fct));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FCT;
  cmd.params.ptr_set_plus_fct = &my_bat_set_plus_fct;

  my_bat_set_plus_fct.value = val;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFCT()");

  ret = sAT_PlusFCT((T_ACI_CMD_SRC)srcId,val);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFCT (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT val=-1;

  TRACE_FUNCTION("queatPlusFCT()");

  ret = qAT_PlusFCT((T_ACI_CMD_SRC)srcId,(SHORT*)&val);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FCT:%X",val);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFSA          |
+--------------------------------------------------------------------+

  PURPOSE : +FSA command (destination subaddress string)
*/

GLOBAL T_ATI_RSLT setatPlusFSA (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  char substr[MAX_ID_CHAR]={0};

  cl = parse(cl,"s",(LONG)MAX_ID_CHAR,substr);
  if (!cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fsa my_bat_set_plus_fsa;

  TRACE_FUNCTION("setatPlusFSA() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fsa, 0, sizeof(my_bat_set_plus_fsa));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FSA;
  cmd.params.ptr_set_plus_fsa = &my_bat_set_plus_fsa;

  my_bat_set_plus_fsa.c_sub_str = strlen(substr);
  memcpy(my_bat_set_plus_fsa.sub_str, substr, my_bat_set_plus_fsa.c_sub_str);

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFSA()");

  ret = sAT_PlusFSA((T_ACI_CMD_SRC)srcId, substr);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFSA (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  char substr[MAX_ID_CHAR]={0};

  TRACE_FUNCTION("queatPlusFSA()");

  ret = qAT_PlusFSA((T_ACI_CMD_SRC)srcId, substr);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FSA:\"%s\"",substr);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFSP          |
+--------------------------------------------------------------------+

  PURPOSE : +FSP command (document to poll indication)
*/

GLOBAL T_ATI_RSLT setatPlusFSP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FSP_VAL val=FSP_VAL_NotPresent;

   cl = parse(cl, "d", &val);
   if ( !cl )
   {
     cmdAtError(atError);
     return ATI_FAIL;
   }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fsp my_bat_set_plus_fsp;

  TRACE_FUNCTION("setatPlusFSP() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fsp, 0, sizeof(my_bat_set_plus_fsp));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FSP;
  cmd.params.ptr_set_plus_fsp = &my_bat_set_plus_fsp;

  my_bat_set_plus_fsp.poll = val;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFSP()");

   ret = sAT_PlusFSP((T_ACI_CMD_SRC)srcId, val);
   if (ret EQ AT_FAIL)
   {
     cmdAtError(atError);
   }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFSP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_FSP_VAL val=FSP_VAL_NotPresent;

  TRACE_FUNCTION("queatPlusFSP()");

  ret = qAT_PlusFSP((T_ACI_CMD_SRC)srcId, &val);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FSP:%d",val);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*--------- Action Commands -------------------------------------*/



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFDT          |
+--------------------------------------------------------------------+

  PURPOSE : +FDT command (data transmission)
*/

GLOBAL T_ATI_RSLT setatPlusFDT(char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;

  TRACE_FUNCTION("setatPlusFDT()");
  ret = sAT_PlusFDT((T_ACI_CMD_SRC)srcId);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFDR          |
+--------------------------------------------------------------------+

  PURPOSE : +FDR command (data reception)
*/

GLOBAL T_ATI_RSLT setatPlusFDR(char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;

  TRACE_FUNCTION("setatPlusFDR()");

  ret = sAT_PlusFDR((T_ACI_CMD_SRC)srcId);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFIP          |
+--------------------------------------------------------------------+

  PURPOSE :

  ITU-T.32/8.3.6 Initialize facsimile parameters, +FIP

  – Syntax: +FIP[=<value>]

  The +FIP command causes the DCE
  to initialize all Service Class 2 Facsimile Parameters
  to the manufacturer determined default settings.

  This command does not change the setting of +FCLASS.
  This command has the same effect as if the DTE had issued
  individual parameter setting commands.

  Manufacturers may also provide a selection of default profiles,
  chosen by the optional <value>.

  If <value> is unspecified or 0, the parameters shall be set to those
  specified in this Recommendation (e.g. Appendix I).

  For other <value>s, the manufacturer determines the profile settings.

  This command may be issued during a session.

  The DCE shall use the new settings at the next time they are sampled;
  for example, a new +FIS setting would be used the next time
  the DCE enters Phase B.
*/

GLOBAL T_ATI_RSLT setatPlusFIP(char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;

#ifdef FF_ATI_BAT
  {
/*
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fip my_bat_set_plus_fip; //???

  TRACE_FUNCTION("setatPlusFIP() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fip, 0, sizeof(my_bat_set_plus_fip));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FIP;
  cmd.params.ptr_set_plus_fip = &my_bat_set_plus_fip;

  my_bat_set_plus_fip.value = val;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; 
*/
  TRACE_EVENT("setatPlusFIP(): is obsolete ? <=== !!! consult KJF");
  return ATI_FAIL;
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFIP()");

  ret = sAT_PlusFIP((T_ACI_CMD_SRC)srcId);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
  /*
   * sAT_PlusFIP always returns AT_CMPL in the present implementation
   * and so setatPlusFIP always results in an error ..
   * this happens for example in testcase ACIFD052
   */

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFKS          |
+--------------------------------------------------------------------+

  PURPOSE : +FKS command (terminate)
*/

GLOBAL T_ATI_RSLT setatPlusFKS(char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
/*
  T_BAT_cmd_set_plus_fks my_bat_set_plus_fks; //???

  TRACE_FUNCTION("setatPlusFKS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fks, 0, sizeof(my_bat_set_plus_fks));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FKS;
  cmd.params.ptr_set_plus_fks = &my_bat_set_plus_fks;

  my_bat_set_plus_fks.value = val;
*/
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFKS()");

  ret = sAT_PlusFKS((T_ACI_CMD_SRC)srcId);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFIT          |
+--------------------------------------------------------------------+

  PURPOSE : +FIT command (inactivity timeout)
*/

GLOBAL T_ATI_RSLT setatPlusFIT (char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_FIT_ACT act  = FIT_ACT_NotPresent;
  SHORT         time = ACI_NumParmNotPresent;

  cl = parse(cl,"dd",&time,&act);
  if ( !cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fit my_bat_set_plus_fit;

  TRACE_FUNCTION("setatPlusFIT() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fit, 0, sizeof(my_bat_set_plus_fit));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FIT;
  cmd.params.ptr_set_plus_fit = &my_bat_set_plus_fit;

  my_bat_set_plus_fit.time = time;
  my_bat_set_plus_fit.action = act;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFIT()");

  ret = sAT_PlusFIT((T_ACI_CMD_SRC)srcId, time, act);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFIT (char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_FIT_ACT act  = FIT_ACT_NotPresent;
  SHORT         time = ACI_NumParmNotPresent;

  TRACE_FUNCTION("queatPlusFIT()");

  ret = qAT_PlusFIT((T_ACI_CMD_SRC)srcId, &time, &act);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FIT:%d,%d",time,act);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFBO          |
+--------------------------------------------------------------------+

  PURPOSE : +FBO command (data bit order)
*/

GLOBAL T_ATI_RSLT setatPlusFBO (char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_FBO_VAL val = FBO_VAL_NotPresent;

  cl = parse(cl,"d",&val);
  if ( !cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_fbo my_bat_set_plus_fbo;

  TRACE_FUNCTION("setatPlusFBO() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_fbo, 0, sizeof(my_bat_set_plus_fbo));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_FBO;
  cmd.params.ptr_set_plus_fbo = &my_bat_set_plus_fbo;

  my_bat_set_plus_fbo.value = val;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusFBO()");

  ret = sAT_PlusFBO((T_ACI_CMD_SRC)srcId,val);
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusFBO (char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_FBO_VAL val = FBO_VAL_NotPresent;

  TRACE_FUNCTION("queatPlusFBO()");

  ret = qAT_PlusFBO((T_ACI_CMD_SRC)srcId, &val);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FBO:%d",val);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFBS          |
+--------------------------------------------------------------------+

  PURPOSE : +FBS command (buffer size)
*/

GLOBAL T_ATI_RSLT queatPlusFBS (char *cl, UBYTE srcId)
{
  SHORT         tbs;
  SHORT         rbs;
  T_ACI_RETURN  ret = AT_FAIL;

  TRACE_FUNCTION("queatPlusFBS()");

  ret = qAT_PlusFBS((T_ACI_CMD_SRC)srcId, &tbs,&rbs);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+FBS:%X,%X",tbs,rbs);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*---------------------- Call Backs -----------------------------------------*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFCI        |
+--------------------------------------------------------------------+

  PURPOSE : +FCI answer (remote ID CSI report)
*/

GLOBAL void rCI_PlusFCI( /*UBYTE srcId,*/ CHAR * rmtId)
{
  UBYTE srcId = srcId_cb;

  if (fd.FNR.idr_stat)
  {
    if(rmtId)
      sprintf(g_sa,"%s\"%s\"","+FCI:",rmtId);
    else
      strcpy(g_sa,"+FCI:");
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFCO        |
+--------------------------------------------------------------------+

  PURPOSE : +FCO answer (connect information)
*/

GLOBAL void rCI_PlusFCO(void)
{
  UBYTE srcId = srcId_cb;

  io_sendMessage(srcId, "+FCO", ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFCS        |
+--------------------------------------------------------------------+

  PURPOSE : +FCS answer (DCS frame information)
*/

GLOBAL void rCI_PlusFCS  (  /*UBYTE srcId,*/
                            T_ACI_F_VR       vr,
                            T_ACI_F_BR       br,
                            T_ACI_F_WD       wd,
                            T_ACI_F_LN       ln,
                            T_ACI_F_DF       df,
                            T_ACI_F_EC       ec,
                            T_ACI_F_BF       bf,
                            T_ACI_F_ST       st,
                            T_ACI_F_JP       jp )
{
  UBYTE srcId = srcId_cb;

  if (fd.FNR.tpr_stat)
  {
    sprintf(g_sa,"+FCS:%X,%d,%d,%d,%d,%d,%X,%d,%X",vr,br,wd,ln,df,ec,bf,st,jp);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFDM        |
+--------------------------------------------------------------------+

  PURPOSE : +FDM answer (transition to data modem)
*/
GLOBAL void rCI_PlusFDM (UBYTE srcId)
{
  io_sendMessage(srcId, "+FDM", ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : atPlusFET          |
+--------------------------------------------------------------------+

  PURPOSE : +FET command (procedure interrupt enable)
*/
GLOBAL void rCI_PlusFET( /*UBYTE srcId,*/ T_ACI_FET_PPM ppm)
{
  UBYTE srcId = srcId_cb;

  sprintf(g_sa,"%s%d","+FET:",ppm);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFFD        |
+--------------------------------------------------------------------+

  PURPOSE : +FFD answer (file diagnostics)
*/
GLOBAL void rCI_PlusFFD (UBYTE srcId)
{
  io_sendMessage(srcId, "+FFD", ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFHS        |
+--------------------------------------------------------------------+

  PURPOSE : +FHS answer (hang up information)
*/

GLOBAL void rCI_PlusFHS( /*UBYTE srcId,*/ T_ACI_FHS_STAT stat)
{
  UBYTE srcId = srcId_cb;

  sprintf(g_sa,"+FHS:%02X", stat);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFHR        |
+--------------------------------------------------------------------+

  PURPOSE : +FHR answer (received HDLC frame reporting)
*/


GLOBAL void rCI_PlusFHR( /*UBYTE srcId,*/ USHORT len, UBYTE * hdlc)
{
  UBYTE srcId = srcId_cb;

  if (fd.FBU_stat)
  {
    strcpy(g_sa,"+FHR:");
    OutHexReport(srcId, len, hdlc);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFHT        |
+--------------------------------------------------------------------+

  PURPOSE : +FHT answer (transmitted HDLC Frame reporting)
*/


GLOBAL void rCI_PlusFHT( /*UBYTE srcId,*/ USHORT len, UBYTE * hdlc)
{
  UBYTE srcId = srcId_cb;

  if (fd.FBU_stat)
  {
    strcpy(g_sa,"+FHT:");
    OutHexReport(srcId, len, hdlc);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFIS        |
+--------------------------------------------------------------------+

  PURPOSE : +FIS answer (DIS frame information)
*/

GLOBAL void rCI_PlusFIS  (  /*UBYTE srcId,*/
                            T_ACI_F_VR       vr,
                            T_ACI_F_BR       br,
                            T_ACI_F_WD       wd,
                            T_ACI_F_LN       ln,
                            T_ACI_F_DF       df,
                            T_ACI_F_EC       ec,
                            T_ACI_F_BF       bf,
                            T_ACI_F_ST       st,
                            T_ACI_F_JP       jp )
{
  UBYTE srcId = srcId_cb;

  if (fd.FNR.rpr_stat)
  {
    sprintf(g_sa,"+FIS:%X,%d,%d,%d,%d,%d,%X,%d,%X",vr,br,wd,ln,df,ec,bf,st,jp);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFNC        |
+--------------------------------------------------------------------+

  PURPOSE : +FNC answer (non standard command reporting)
*/


GLOBAL void rCI_PlusFNC( /*UBYTE srcId,*/ USHORT len, UBYTE * nsc)
{
  UBYTE srcId = srcId_cb;

  if (fd.FNR.nsr_stat)
  {
    strcpy(g_sa,"+FNC:");

    OutHexReport(srcId, len, nsc);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFNF        |
+--------------------------------------------------------------------+

  PURPOSE : +FNF answer (non standard facility reporting)
*/


GLOBAL void rCI_PlusFNF( /*UBYTE srcId,*/ USHORT len, UBYTE * nsf)
{
  UBYTE srcId = srcId_cb;

  if (fd.FNR.nsr_stat)
  {
    strcpy(g_sa,"+FNF:");

    OutHexReport(srcId, len, nsf);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFNS        |
+--------------------------------------------------------------------+

  PURPOSE : +FNS answer (non standard setup reporting)
*/


GLOBAL void rCI_PlusFNS( /*UBYTE srcId,*/ USHORT len, UBYTE * nss)
{
  UBYTE srcId = srcId_cb;

  if (fd.FNR.nsr_stat)
  {
    strcpy(g_sa,"+FNS:");

    OutHexReport(srcId, len, nss);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFPA        |
+--------------------------------------------------------------------+

  PURPOSE : +FPA answer (report polling address)
*/
GLOBAL void rCI_PlusFPA (/*UBYTE srcId,*/ char *sep)
{
  UBYTE srcId = srcId_cb;

  if(sep)
    sprintf(g_sa,"+FPA:\"%s\"",sep);
  else
    strcpy(g_sa,"+FPA:");
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFPI        |
+--------------------------------------------------------------------+

  PURPOSE : +FPI answer (remote ID CIG report)
*/

GLOBAL void rCI_PlusFPI( /*UBYTE srcId,*/ CHAR *cig)
{
  UBYTE srcId = srcId_cb;

  if (fd.FNR.idr_stat)
  {
    if(cig)
      sprintf(g_sa,"%s\"%s\"","+FPI:",cig);
    else
      strcpy(g_sa,"+FPI:");
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFPO        |
+--------------------------------------------------------------------+

  PURPOSE : +FPO answer (polling request)
*/

GLOBAL void rCI_PlusFPO(void)
{
  UBYTE srcId = srcId_cb;

  io_sendMessage(srcId, "+FPO", ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFPS        |
+--------------------------------------------------------------------+

  PURPOSE : +FPS answer (DIS frame information)
*/

GLOBAL void rCI_PlusFPS  (   /*UBYTE srcId,*/
                             T_ACI_FPS_PPR    ppr,
                             SHORT            lc,
                             SHORT            blc,
                             SHORT            cblc,
                             SHORT            lbc )
{
  UBYTE srcId = srcId_cb;

  sprintf(g_sa,"%s%d,%d,%d,%d,%d","+FPS:",ppr,lc,blc,cblc,lbc);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFPW        |
+--------------------------------------------------------------------+

  PURPOSE : +FPW answer (report password)
*/
GLOBAL void rCI_PlusFPW (/*UBYTE srcId,*/ char *pwd)
{
  UBYTE srcId = srcId_cb;

  if(pwd)
    sprintf(g_sa,"+FPW:\"%s\"",pwd);
  else
    strcpy(g_sa,"+FPW:");
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFSA        |
+--------------------------------------------------------------------+

  PURPOSE : +FSA answer (report subaddress)
*/
GLOBAL void rCI_PlusFSA (/*UBYTE srcId,*/ char *sub)
{
  UBYTE srcId = srcId_cb;

  if(sub)
    sprintf(g_sa,"+FSA:\"%s\"",sub);
  else
    sprintf(g_sa,"+FSA:");
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFTC        |
+--------------------------------------------------------------------+

  PURPOSE : +FTC answer (DTC frame information)
*/

GLOBAL void rCI_PlusFTC  ( /*UBYTE srcId,*/
                            T_ACI_F_VR       vr,
                            T_ACI_F_BR       br,
                            T_ACI_F_WD       wd,
                            T_ACI_F_LN       ln,
                            T_ACI_F_DF       df,
                            T_ACI_F_EC       ec,
                            T_ACI_F_BF       bf,
                            T_ACI_F_ST       st,
                            T_ACI_F_JP       jp )
{
  UBYTE srcId = srcId_cb;

  if (fd.FNR.rpr_stat)
  {

    sprintf(g_sa,"+FTC:%X,%d,%d,%d,%d,%d,%X,%d,%X",vr,br,wd,ln,df,ec,bf,st,jp);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFTI        |
+--------------------------------------------------------------------+

  PURPOSE : +FTI answer (remote ID TSI report)
*/

GLOBAL void rCI_PlusFTI( CHAR * tsi)
{
  UBYTE srcId = srcId_cb;

  if (fd.FNR.idr_stat)
  {
    if(tsi)
      sprintf(g_sa,"%s\"%s\"","+FTI:",tsi);
    else
      strcpy(g_sa,"+FTI:");
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : rCI_PlusFVO        |
+--------------------------------------------------------------------+

  PURPOSE : +FVO answer (transition to voice)
*/

GLOBAL void rCI_PlusFVO(void /*UBYTE srcId*/)
{
  UBYTE srcId = srcId_cb;

  io_sendMessage(srcId, "+FVO", ATI_NORMAL_OUTPUT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_FD_CMD         |
| STATE   : code                        ROUTINE : OutHexReport       |
+--------------------------------------------------------------------+

  PURPOSE : formatted hex report output
*/

GLOBAL void OutHexReport( UBYTE srcId, USHORT len, UBYTE * buf )
{
  USHORT i;
  UBYTE  tmpBuf[SIZE_TMP_BUF];
  /* memory should be allocated before the function is called for sa !!!

  sa needs at most (len * SIZE_TMP_BUF + 1) for what will be used here */
  for( i=0; i < len; i++ )
  {
    sprintf((char*)tmpBuf, "%02X ", buf[i]);
    strcat( g_sa, (char*)tmpBuf );

  }
  g_sa[strlen(g_sa)-1] = '\0';     /* cut off last space char */
  io_sendMessage(srcId, g_sa, ATI_FORCED_OUTPUT);
}

#endif /* FF_FAX */

#endif /* FAX_AND_DATA */



