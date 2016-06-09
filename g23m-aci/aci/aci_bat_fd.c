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
#include "aci_cmh.h"     /* prototypes of sAT_, qAT_, tAT_    */
#include "aci_bat_cmh.h" /* prototypes of sBAT_, qBAT_, tBAT_ */
#include "aci_bat.h"
#include "aci_fd.h"

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : BAT_O                |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT BAT_O(T_ACI_DTI_PRC_PSI *src_infos_psi,
                            T_BAT_cmd_send    *cmd)
{
  TRACE_FUNCTION ("BAT_O()");
  return(ACI_BAT_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusDS          |
+--------------------------------------------------------------------+

  PURPOSE : V.42bis data compression functions
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusDS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                  T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusDS()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusDS((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_DS_DIR)cmd->params.ptr_set_plus_ds->dir,
                    (T_ACI_DS_COMP)cmd->params.ptr_set_plus_ds->neg,
                    cmd->params.ptr_set_plus_ds->p1,
                    cmd->params.ptr_set_plus_ds->p2);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusDS          |
+--------------------------------------------------------------------+

  PURPOSE : V.42bis data compression functions
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusDS          (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_ds ds;

  TRACE_FUNCTION ("qBAT_PlusDS()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_DS;
  resp.response.ptr_que_plus_ds = &ds;

  ret =(T_ACI_BAT_RSLT)qAT_PlusDS((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	             (T_ACI_DS_DIR*)&ds.dir,(T_ACI_DS_COMP*)&ds.neg,
  	             (LONG*)&ds.p1,(SHORT*)&ds.p2);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCRLP        |
+--------------------------------------------------------------------+

  PURPOSE : Radio link protocol
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCRLP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusCRLP()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusCRLP((T_ACI_CMD_SRC)src_infos_psi->srcId,
                     (S16)cmd->params.ptr_set_plus_crlp->iws,
                     (S16)cmd->params.ptr_set_plus_crlp->mws,
                     (S16)cmd->params.ptr_set_plus_crlp->t1,
                     (S16)cmd->params.ptr_set_plus_crlp->n2);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCRLP        |
+--------------------------------------------------------------------+

  PURPOSE : Radio link protocol
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCRLP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_crlp crlp;

  TRACE_FUNCTION ("qBAT_PlusCRLP()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CRLP;
  resp.response.ptr_que_plus_crlp = &crlp;

  ret = (T_ACI_BAT_RSLT)qAT_PlusCRLP( (T_ACI_CMD_SRC)src_infos_psi->srcId,
                      (SHORT*)&crlp.iws, (SHORT*)&crlp.mws, (SHORT*)&crlp.t1, (SHORT*)&crlp.n2);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

#ifdef FF_FAX

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFAP         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFAP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFAP()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFAP((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                    (T_ACI_FAP_VAL)cmd->params.ptr_set_plus_fap->sub,
                    (T_ACI_FAP_VAL)cmd->params.ptr_set_plus_fap->sep,
                    (T_ACI_FAP_VAL)cmd->params.ptr_set_plus_fap->pwd);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFAP         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFAP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fap fap;
  
  TRACE_FUNCTION ("qBAT_PlusFAP()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FAP;
  resp.response.ptr_que_plus_fap = &fap;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFAP((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	             (T_ACI_FAP_VAL*)&fap.sub,(T_ACI_FAP_VAL*)&fap.sep,(T_ACI_FAP_VAL*)&fap.pwd);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFBO         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFBO(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFBO()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFBO((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                    (T_ACI_FBO_VAL)cmd->params.ptr_set_plus_fbo->value);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFBO         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFBO(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fbo fbo;

  TRACE_FUNCTION ("qBAT_PlusFBO()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FBO;
  resp.response.ptr_que_plus_fbo  = &fbo;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFBO((T_ACI_CMD_SRC)src_infos_psi->srcId, 
  	              (T_ACI_FBO_VAL*)&fbo.value);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFBS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFBS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fbs fbs;

  TRACE_FUNCTION ("qBAT_PlusFBS()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FBS; 
  resp.response.ptr_que_plus_fbs = &fbs;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFBS((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	              (S16*)&fbs.tbs, (S16*)&fbs.rbs);
  
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFBU         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFBU(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;

  TRACE_FUNCTION ("sBAT_PlusFBU(): is obsolete <=== !!! consult KJF");

  /*
  ret = sAT_PlusFBU(src_infos_psi->srcId, 
                    cmd->params.ptr_set_plus_fbu->value);
  */

  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp);
  
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFBU         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFBU(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fbu fbu;

  TRACE_FUNCTION ("qBAT_PlusFBU()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FBU;
  resp.response.ptr_que_plus_fbu = &fbu;
  
  //???ret = qAT_PlusFBU(src_infos_psi->srcId, &fbu.val);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFCC         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCC(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  
  T_BAT_cmd_set_plus_fcc *fcc = cmd->params.ptr_set_plus_fcc;

  TRACE_FUNCTION ("sBAT_PlusFCC()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFCC((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_F_VR)fcc->vr,
                    (T_ACI_F_BR)fcc->br,
                    (T_ACI_F_WD)fcc->wd,
                    (T_ACI_F_LN)fcc->ln,
                    (T_ACI_F_DF)fcc->df,
                    (T_ACI_F_EC)fcc->ec,
                    (T_ACI_F_BF)fcc->bf,
                    (T_ACI_F_ST)fcc->st,
                    (T_ACI_F_JP)fcc->jp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFCC         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCC(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fcc fcc;

  TRACE_FUNCTION ("qBAT_PlusFCC()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FCC;
  resp.response.ptr_que_plus_fcc = &fcc;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFCC((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_F_VR*)&fcc.vr,
                    (T_ACI_F_BR*)&fcc.br,
                    (T_ACI_F_WD*)&fcc.wd,
                    (T_ACI_F_LN*)&fcc.ln,
                    (T_ACI_F_DF*)&fcc.df,
                    (T_ACI_F_EC*)&fcc.ec,
                    (T_ACI_F_BF*)&fcc.bf,
                    (T_ACI_F_ST*)&fcc.st,
                    (T_ACI_F_JP*)&fcc.jp);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFCLASS      |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCLASS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                      T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFCLASS()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFCLASS((T_ACI_CMD_SRC)src_infos_psi->srcId,
                       (T_ACI_FCLASS_CLASS)cmd->params.ptr_set_plus_fclass->n); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFCLASS      |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCLASS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                      T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fclass fclass;

  TRACE_FUNCTION ("qBAT_PlusFCLASS()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FCLASS;
  resp.response.ptr_que_plus_fclass = &fclass;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFCLASS((T_ACI_CMD_SRC)src_infos_psi->srcId, 
  	               (T_ACI_FCLASS_CLASS*)&fclass.n); 

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFCQ         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCQ(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFCQ()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFCQ((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_FCQ_RQ)cmd->params.ptr_set_plus_fcq->rq,
                    (T_ACI_FCQ_TQ)cmd->params.ptr_set_plus_fcq->tq);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFCQ         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCQ(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fcq fcq;

  TRACE_FUNCTION ("qBAT_PlusFCQ()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FCQ;
  resp.response.ptr_que_plus_fcq = &fcq;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFCQ((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	              (T_ACI_FCQ_RQ*)&fcq.rq,(T_ACI_FCQ_TQ*)&fcq.tq); 

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFCR         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCR(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFCR()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFCR((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_FCR_VAL)cmd->params.ptr_set_plus_fcr->value);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFCR         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCR(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fcr fcr;

  TRACE_FUNCTION ("qBAT_PlusFCR()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FCR;
  resp.response.ptr_que_plus_fcr = &fcr;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFCR((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	             (T_ACI_FCR_VAL*)&fcr.value); 

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFCS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fcs fcs;

  TRACE_FUNCTION ("qBAT_PlusFCS()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FCS;
  resp.response.ptr_que_plus_fcs = &fcs;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFCS((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_F_VR*)&fcs.vr,
                    (T_ACI_F_BR*)&fcs.br,
                    (T_ACI_F_WD*)&fcs.wd,
                    (T_ACI_F_LN*)&fcs.ln,
                    (T_ACI_F_DF*)&fcs.df,
                    (T_ACI_F_EC*)&fcs.ec,
                    (T_ACI_F_BF*)&fcs.bf,
                    (T_ACI_F_ST*)&fcs.st,
                    (T_ACI_F_JP*)&fcs.jp); 

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFEA         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFEA(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;

  TRACE_FUNCTION ("sBAT_PlusFEA(): is obsolete <=== !!! consult KJF");
  /* adjust the left hand side
    xxx = cmd->params.ptr_set_plus_fea->value;
  */
  /* hmm, do not know which ACI function to call ! */
  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFEA         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFEA(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;

  TRACE_FUNCTION ("qBAT_PlusFEA(): is obsolete <=== !!! consult KJF");
  /* hmm, do not know which ACI function to call ! */
  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFFC         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFFC(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  T_BAT_cmd_set_plus_ffc *ffc = cmd->params.ptr_set_plus_ffc;

  TRACE_FUNCTION ("sBAT_PlusFFC()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFFC((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_FFC_VRC)ffc->vrc,
                    (T_ACI_FFC_DFC)ffc->dfc,
                    (T_ACI_FFC_LNC)ffc->lnc,
                    (T_ACI_FFC_WDC)ffc->wdc);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFFC         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFFC(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_ffc ffc;

  TRACE_FUNCTION ("qBAT_PlusFFC()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FFC;
  resp.response.ptr_que_plus_ffc = &ffc;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFFC((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_FFC_VRC*)&ffc.vrc,(T_ACI_FFC_DFC*)&ffc.dfc,
                    (T_ACI_FFC_LNC*)&ffc.lnc,(T_ACI_FFC_WDC*)&ffc.wdc);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFHS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFHS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fhs fhs;

  TRACE_FUNCTION ("qBAT_PlusFHS()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FHS;
  resp.response.ptr_que_plus_fhs = &fhs;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFHS((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	              (T_ACI_FHS_STAT*)&fhs.status); 

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFIE         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFIE(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFIE()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFIE((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_FIE_VAL)cmd->params.ptr_set_plus_fie->value);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFIE         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFIE(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;

  T_BAT_res_que_plus_fie fie;

  TRACE_FUNCTION ("qBAT_PlusFIE()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FIE;
  resp.response.ptr_que_plus_fie = &fie;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFIE((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	              (T_ACI_FIE_VAL*)&fie.value); 
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFIS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFIS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  T_BAT_cmd_set_plus_fis *fis = cmd->params.ptr_set_plus_fis;

  TRACE_FUNCTION ("sBAT_PlusFIS()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFIS((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_F_VR)fis->vr,
                    (T_ACI_F_BR)fis->br,
                    (T_ACI_F_WD)fis->wd,
                    (T_ACI_F_LN)fis->ln,
                    (T_ACI_F_DF)fis->df,
                    (T_ACI_F_EC)fis->ec,
                    (T_ACI_F_BF)fis->bf,
                    (T_ACI_F_ST)fis->st,
                    (T_ACI_F_JP)fis->jp); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFIS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFIS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;

  T_BAT_res_que_plus_fis fis;

  TRACE_FUNCTION ("qBAT_PlusFIS()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FIS;
  resp.response.ptr_que_plus_fis = &fis;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFIS((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_F_VR*)&fis.vr,
                    (T_ACI_F_BR*)&fis.br,
                    (T_ACI_F_WD*)&fis.wd,
                    (T_ACI_F_LN*)&fis.ln,
                    (T_ACI_F_DF*)&fis.df,
                    (T_ACI_F_EC*)&fis.ec,
                    (T_ACI_F_BF*)&fis.bf,
                    (T_ACI_F_ST*)&fis.st,
                    (T_ACI_F_JP*)&fis.jp); 

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFLI         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFLI(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFLI()");
 
  ret = (T_ACI_BAT_RSLT)sAT_PlusFLI((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (CHAR*)cmd->params.ptr_set_plus_fli->id_str);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFLI         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFLI(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fli fli;

  TRACE_FUNCTION ("qBAT_PlusFLI()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FLI;
  resp.response.ptr_que_plus_fli = &fli;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusFLI((T_ACI_CMD_SRC)src_infos_psi->srcId, 
  	              (CHAR*)fli.id_str);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFLO         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFLO(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFLO()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusIFC((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_RX_FLOW_CTRL)cmd->params.ptr_set_plus_flo->flo_c,
                    (T_ACI_RX_FLOW_CTRL)cmd->params.ptr_set_plus_flo->flo_c); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFLO         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFLO(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_flo flo;
  T_ACI_RX_FLOW_CTRL DCE_by_DTE;
  T_ACI_RX_FLOW_CTRL DTE_by_DCE;
  
  TRACE_FUNCTION ("qBAT_PlusFLO()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FLO;
  resp.response.ptr_que_plus_flo = &flo;

  ret = (T_ACI_BAT_RSLT)qAT_PlusIFC((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	              &DCE_by_DTE, &DTE_by_DCE);

  flo.flo_q = (T_BAT_plus_flo_flo_q)DCE_by_DTE;

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFLP         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFLP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFLP()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFLP((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_FLP_VAL)cmd->params.ptr_set_plus_flp->value);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFLP         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFLP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_flp flp;

  TRACE_FUNCTION ("qBAT_PlusFLP()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FLP;
  resp.response.ptr_que_plus_flp = &flp;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFLP((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	             (T_ACI_FLP_VAL*)&flp.value);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFMS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFMS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFMS()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFMS((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_F_BR)cmd->params.ptr_set_plus_fms->value); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFMS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFMS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fms fms;

  TRACE_FUNCTION ("qBAT_PlusFMS()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FMS;
  resp.response.ptr_que_plus_fms = &fms;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFMS((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	             (T_ACI_F_BR*)&fms.value);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFNS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFNS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFNS()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFNS((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    cmd->params.ptr_set_plus_fns->c_nsf,
                    cmd->params.ptr_set_plus_fns->nsf); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFNS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFNS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fns fns;

  TRACE_FUNCTION ("qBAT_PlusFNS()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FNS;
  resp.response.ptr_que_plus_fns = &fns;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusFNS((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	              &fns.c_nsf, fns.nsf);
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFPA         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPA(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFPA()");
  ret = (T_ACI_BAT_RSLT)sAT_PlusFPA((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (CHAR*)cmd->params.ptr_set_plus_fpa->spa_str); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFPA         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFPA(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fpa fpa;

  TRACE_FUNCTION ("qBAT_PlusFPA()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FPA;
  resp.response.ptr_que_plus_fpa = &fpa;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFPA((T_ACI_CMD_SRC)src_infos_psi->srcId, 
  	             (CHAR*)fpa.spa_str);
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFPI         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPI(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFPI()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFPI((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (CHAR*)cmd->params.ptr_set_plus_fpi->id_str);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFPI         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFPI(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fpi fpi;

  TRACE_FUNCTION ("qBAT_PlusFPI()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FPI;
  resp.response.ptr_que_plus_fpi = &fpi;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFPI((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	             (CHAR*)fpi.id_str); 
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFPS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFPS()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFPS((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_FPS_PPR)cmd->params.ptr_set_plus_fps->ppr);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFPS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFPS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fps fps;

  TRACE_FUNCTION ("qBAT_PlusFPS()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FPS;
  resp.response.ptr_que_plus_fps = &fps;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFPS((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	               (T_ACI_FPS_PPR*)&fps.ppr); 
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFPW         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPW(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFPW()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFPW((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (CHAR*)cmd->params.ptr_set_plus_fpw->pw_str);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFPW         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFPW(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fpw fpw;

  TRACE_FUNCTION ("qBAT_PlusFPW()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FPW;
  resp.response.ptr_que_plus_fpw = &fpw;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusFPW((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	               (CHAR*)fpw.pw_str);
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFRQ         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFRQ(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFRQ()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFRQ((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    cmd->params.ptr_set_plus_frq->pgl,
                    cmd->params.ptr_set_plus_frq->cbl);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFRQ         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFRQ(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_frq frq;

  TRACE_FUNCTION ("qBAT_PlusFRQ()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FRQ;
  resp.response.ptr_que_plus_frq = &frq;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFRQ((T_ACI_CMD_SRC)src_infos_psi->srcId, 
  	             (SHORT*)&frq.pgl, (SHORT*)&frq.cbl);
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFCT         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCT(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFCT()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFCT((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    cmd->params.ptr_set_plus_fct->value);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFCT         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCT(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fct fct;

  TRACE_FUNCTION ("qBAT_PlusFCT()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FCT;
  resp.response.ptr_que_plus_fct = &fct;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFCT((T_ACI_CMD_SRC)src_infos_psi->srcId, 
  	               (SHORT*)&fct.value); 
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFSA         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFSA(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFSA()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFSA((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                    (CHAR*)cmd->params.ptr_set_plus_fsa->sub_str);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFSA         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFSA(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fsa fsa;

  TRACE_FUNCTION ("qBAT_PlusFSA()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FSA;
  resp.response.ptr_que_plus_fsa = &fsa;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFSA((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	                                                    (CHAR*)fsa.sub_str);
  
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFSP         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFSP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFSP()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFSP((T_ACI_CMD_SRC)src_infos_psi->srcId, 
          (T_ACI_FSP_VAL)cmd->params.ptr_set_plus_fsp->poll);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFSP         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFSP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fsp fsp;

  TRACE_FUNCTION ("qBAT_PlusFSP()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FSP;
  resp.response.ptr_que_plus_fsp = &fsp;

  ret = (T_ACI_BAT_RSLT)qAT_PlusFSP((T_ACI_CMD_SRC)src_infos_psi->srcId, 
  	    (T_ACI_FSP_VAL*)&fsp.poll);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusFIT         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFIT(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusFIT()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusFIT((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    cmd->params.ptr_set_plus_fit->time,
                    (T_ACI_FIT_ACT)cmd->params.ptr_set_plus_fit->action);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusFIT         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFIT(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                   T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fit fit;

  TRACE_FUNCTION ("qBAT_PlusFIT()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_FIT;
  resp.response.ptr_que_plus_fit = &fit;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusFIT((T_ACI_CMD_SRC)src_infos_psi->srcId, 
  	   (SHORT*)&fit.time, (T_ACI_FIT_ACT*)&fit.action);
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

#endif /* FF_FAX */
