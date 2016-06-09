/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_T30Q
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
|             protocol stack adapter for T30.
+----------------------------------------------------------------------------- 
*/ 
#if defined (DTI) || defined (FF_FAX)

#ifndef CMH_T30Q_C
#define CMH_T30Q_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti.h"
#include "dti_conn_mng.h"

#include "aci_fd.h"

#include "psa.h"
#include "psa_t30.h"
#include "cmh.h"
#include "cmh_t30.h"

#include "cmh_ra.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFCLASS           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCLASS? AT 
            command which returns the current setting of class mode.

            <class_type>:   class mode.
*/

GLOBAL T_ACI_RETURN qAT_PlusFCLASS  ( T_ACI_CMD_SRC srcId,
                                      T_ACI_FCLASS_CLASS* class_type )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFCLASS()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *class_type = pT30CmdPrm -> FCLASSclass;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFCR              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCR? AT 
            command which returns the current setting of the receive 
            capability.

            <value>:   receive capability.
*/

GLOBAL T_ACI_RETURN qAT_PlusFCR  ( T_ACI_CMD_SRC srcId,
                                   T_ACI_FCR_VAL* value )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFCR()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *value = pT30CmdPrm -> FCRval;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFLI              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FLI? AT 
            command which returns the current setting for the local 
            id string (CSI/TSI).

            <idStr>:   local id string(0 terminated).
*/

GLOBAL T_ACI_RETURN qAT_PlusFLI  ( T_ACI_CMD_SRC srcId,
                                   CHAR * idStr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFLI()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  strncpy( idStr, pT30CmdPrm -> FLIstr, MAX_ID_CHAR-1 );
  idStr[MAX_ID_CHAR-1] = 0;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFPI              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FPI? AT 
            command which returns the current setting for the local 
            id string (CGI).

            <idStr>:   local id string(0 terminated).
*/

GLOBAL T_ACI_RETURN qAT_PlusFPI  ( T_ACI_CMD_SRC srcId,
                                   CHAR * idStr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFPI()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  strncpy( idStr, pT30CmdPrm -> FPIstr, MAX_ID_CHAR-1 );
  idStr[MAX_ID_CHAR-1] = 0;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFSA              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FSA? AT 
            command which returns the current setting for the 
            destination subaddress string.

            <subStr>:   destination subaddress string (0 terminated).
*/

GLOBAL T_ACI_RETURN qAT_PlusFSA  ( T_ACI_CMD_SRC srcId,
                                   CHAR * subStr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFSA()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  strncpy( subStr, pT30CmdPrm -> FSAsub, MAX_ID_CHAR-1 );
  subStr[MAX_ID_CHAR-1] = 0;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFPA              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FPA? AT 
            command which returns the current setting for the 
            selective polling address.

            <idStr>:   selective polling address string(0 terminated).
*/

GLOBAL T_ACI_RETURN qAT_PlusFPA  ( T_ACI_CMD_SRC srcId,
                                   CHAR * sepStr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFPA()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  strncpy( sepStr, pT30CmdPrm -> FPAsep, MAX_ID_CHAR-1 );
  sepStr[MAX_ID_CHAR-1] = 0;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFPW              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FPW? AT 
            command which returns the current setting for the 
            password string.

            <idStr>:   password string(0 terminated).
*/

GLOBAL T_ACI_RETURN qAT_PlusFPW  ( T_ACI_CMD_SRC srcId,
                                   CHAR * pwdStr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFPW()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  strncpy( pwdStr, pT30CmdPrm -> FPWpwd, MAX_ID_CHAR-1 );
  pwdStr[MAX_ID_CHAR-1] = 0;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFCC              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCC? AT 
            command which returns the current setting of the DCE 
            capabilities.

            <vr>:   resolution.
            <br>:   bit rate.
            <wd>:   page width.
            <ln>:   page length.
            <df>:   data compression.
            <ec>:   error correction.
            <bt>:   file transer.
            <st>:   scan time.
            <jp>:   JPEG mode.
*/

GLOBAL T_ACI_RETURN qAT_PlusFCC  ( T_ACI_CMD_SRC srcId, T_ACI_F_VR* vr,
                                   T_ACI_F_BR* br, T_ACI_F_WD* wd,
                                   T_ACI_F_LN* ln, T_ACI_F_DF* df,
                                   T_ACI_F_EC* ec, T_ACI_F_BF* bf,
                                   T_ACI_F_ST* st, T_ACI_F_JP* jp )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFCC()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in the parameters
 *-------------------------------------------------------------------
 */  
  *vr = pT30CmdPrm -> FCCvr;  
  *br = pT30CmdPrm -> FCCbr;
  *wd = pT30CmdPrm -> FCCwd;
  *ln = pT30CmdPrm -> FCCln;
  *df = pT30CmdPrm -> FCCdf;
  *ec = pT30CmdPrm -> FCCec;
  *bf = pT30CmdPrm -> FCCbf;  
  *st = pT30CmdPrm -> FCCst;
  *jp = pT30CmdPrm -> FCCjp;  

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFIS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FIS? AT 
            command which returns the settings of the current session
            capabilities.

            <vr>:   resolution.
            <br>:   bit rate.
            <wd>:   page width.
            <ln>:   page length.
            <df>:   data compression.
            <ec>:   error correction.
            <bt>:   file transer.
            <st>:   scan time.
            <jp>:   JPEG mode.
*/

GLOBAL T_ACI_RETURN qAT_PlusFIS  ( T_ACI_CMD_SRC srcId, T_ACI_F_VR* vr,
                                   T_ACI_F_BR* br, T_ACI_F_WD* wd,
                                   T_ACI_F_LN* ln, T_ACI_F_DF* df,
                                   T_ACI_F_EC* ec, T_ACI_F_BF* bf,
                                   T_ACI_F_ST* st, T_ACI_F_JP* jp )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFIS()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in the parameters
 *-------------------------------------------------------------------
 */  
  *vr = pT30CmdPrm -> FISvr;  
  *br = pT30CmdPrm -> FISbr;
  *wd = pT30CmdPrm -> FISwd;
  *ln = pT30CmdPrm -> FISln;
  *df = pT30CmdPrm -> FISdf;
  *ec = pT30CmdPrm -> FISec;
  *bf = pT30CmdPrm -> FISbf;  
  *st = pT30CmdPrm -> FISst;
  *jp = pT30CmdPrm -> FISjp;  

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFNS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FNS? AT 
            command which returns the current length and setting for 
            the non-standard facility string.

            <len>:   length of nsf string in bytes.
            <nsf>:   nsf string.
*/

GLOBAL T_ACI_RETURN qAT_PlusFNS  ( T_ACI_CMD_SRC srcId,
                                   UBYTE * len,
                                   UBYTE * nsf )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFNS()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  if (pT30CmdPrm -> FNSlen)
  {
    memcpy( nsf, pT30CmdPrm -> FNSoct, pT30CmdPrm -> FNSlen );
  }

  *len = pT30CmdPrm -> FNSlen;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFLP              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FLP? AT 
            command which returns the current setting of indication
            for a document to poll.

            <value>:   polling indication.
*/

GLOBAL T_ACI_RETURN qAT_PlusFLP  ( T_ACI_CMD_SRC srcId,
                                   T_ACI_FLP_VAL* value )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFLP()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *value = pT30CmdPrm -> FLPval;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFCQ              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCQ? AT 
            command which returns the current setting for copy quality.

            <rq>:   receive quality.
            <tq>:   transmit quality.
*/

GLOBAL T_ACI_RETURN qAT_PlusFCQ   (T_ACI_CMD_SRC srcId,
                                   T_ACI_FCQ_RQ * rq,
                                   T_ACI_FCQ_TQ * tq )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFCQ()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *rq = pT30CmdPrm -> FCQrq;
  *tq = FCQ_TQ_CQCDisabled;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFRQ              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FRQ? AT 
            command which returns the current setting for receive 
            quality thresholds.

            <pgl>:   percentage of good lines.
            <cbl>:   consecutive bad lines.
*/

GLOBAL T_ACI_RETURN qAT_PlusFRQ   (T_ACI_CMD_SRC srcId, 
                                   SHORT * pgl, 
                                   SHORT * cbl )
{

  TRACE_FUNCTION ("qAT_PlusFRQ()");

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *pgl = 0;
  *cbl = 0;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFHS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FRQ? AT 
            command which returns the current call termination status.

            <status>:   call termination status.
*/

GLOBAL T_ACI_RETURN qAT_PlusFHS   (T_ACI_CMD_SRC  srcId, 
                                   T_ACI_FHS_STAT * status )
{

  TRACE_FUNCTION ("qAT_PlusFHS()");

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *status = FHSstat;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFIT              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FIT? AT 
            command which returns the current setting for inactivity
            timer.

            <time>:   inactivity timeout.
            <act> :   action.
*/

GLOBAL T_ACI_RETURN qAT_PlusFIT   (T_ACI_CMD_SRC        srcId, 
                                   SHORT              * time,
                                   T_ACI_FIT_ACT      * act )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFIT()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *act  = pT30CmdPrm -> FITact;

  *time = pT30CmdPrm -> FITtime / 1000;      
  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFBO              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FBO? AT 
            command which returns the current setting for data bit 
            order.

            <value>:  data bit order.
*/

GLOBAL T_ACI_RETURN qAT_PlusFBO   (T_ACI_CMD_SRC        srcId, 
                                   T_ACI_FBO_VAL      * value )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFBO()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *value  = pT30CmdPrm -> FBOval;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFBS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FBS? AT 
            command which returns the current setting of buffer sizes.

            <tbs>:  transmit buffer size.
            <rbs>:  receive buffer size.
*/

GLOBAL T_ACI_RETURN qAT_PlusFBS   (T_ACI_CMD_SRC        srcId, 
                                   SHORT              * tbs,
                                   SHORT              * rbs )
{

  TRACE_FUNCTION ("qAT_PlusFBS()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *tbs = (SHORT)t30ShrdPrm.tbs;
  *rbs = (SHORT)t30ShrdPrm.rbs;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFEA              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FEA? AT 
            command which returns the current setting for phase C EOL
            alignment.

            <value>:   alignment.
*/

GLOBAL T_ACI_RETURN qAT_PlusFEA (T_ACI_CMD_SRC srcId, 
                                 SHORT * value )
{

  TRACE_FUNCTION ("qAT_PlusFEA()");

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *value = 0;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFCT              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCT? AT 
            command which returns the current setting phase C response
            timeout.

            <value>:   timeout.
*/

GLOBAL T_ACI_RETURN qAT_PlusFCT (T_ACI_CMD_SRC srcId, 
                                 SHORT * value )
{

  TRACE_FUNCTION ("qAT_PlusFCT()");

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *value = 30;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFMS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FMS? AT 
            command which returns the current setting for minimum
            phase C speed.

            <br>:   minimum phase C speed.
*/

GLOBAL T_ACI_RETURN qAT_PlusFMS (T_ACI_CMD_SRC srcId,
                                 T_ACI_F_BR * br)
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFMS()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *br = pT30CmdPrm -> FMSbr;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFIE              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FIE? AT 
            command which returns the current setting for procedure 
            interrupt enable.

            <value>:   PI enable.
*/

GLOBAL T_ACI_RETURN qAT_PlusFIE (T_ACI_CMD_SRC srcId,
                                 T_ACI_FIE_VAL *value)
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFIE()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *value = pT30CmdPrm -> FIEval;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFFC              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FFC? AT 
            command which returns the current setting of format 
            converersion parameters.

            <vrc>: vertical resolution format.
            <dfc>: data format.
            <lnc>: page length format.
            <wdc>: page width format.
*/

GLOBAL T_ACI_RETURN qAT_PlusFFC (T_ACI_CMD_SRC srcId, 
                                 T_ACI_FFC_VRC * vrc,
                                 T_ACI_FFC_DFC * dfc,
                                 T_ACI_FFC_LNC * lnc,
                                 T_ACI_FFC_WDC * wdc)
{

  TRACE_FUNCTION ("qAT_PlusFFC()");

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *vrc = FFC_VRC_Ignored;
  *dfc = FFC_DFC_Ignored;
  *lnc = FFC_LNC_Ignored;
  *wdc = FFC_WDC_Ignored;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFCS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCS? AT 
            command which returns the current session results.

            <vr>:   resolution.
            <br>:   bit rate.
            <wd>:   page width.
            <ln>:   page length.
            <df>:   data compression.
            <ec>:   error correction.
            <bt>:   file transer.
            <st>:   scan time.
            <jp>:   JPEG mode.
*/

GLOBAL T_ACI_RETURN qAT_PlusFCS   (T_ACI_CMD_SRC srcId,
                                   T_ACI_F_VR * vr, T_ACI_F_BR * br, 
                                   T_ACI_F_WD * wd, T_ACI_F_LN * ln, 
                                   T_ACI_F_DF * df, T_ACI_F_EC * ec, 
                                   T_ACI_F_BF * bf, T_ACI_F_ST * st,
                                   T_ACI_F_JP * jp)
{

  TRACE_FUNCTION ("qAT_PlusFCS()");

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
#ifdef _SIMULATION_ /* simulates negotiation */
  
  {
  T_T30_CMD_PRM *pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

  *vr = pT30CmdPrm->FISvr;
  *br = pT30CmdPrm->FISbr;
  *wd = pT30CmdPrm->FISwd;
  *ln = pT30CmdPrm->FISln;
  *df = pT30CmdPrm->FISdf;
  *ec = pT30CmdPrm->FISec;
  *bf = pT30CmdPrm->FISbf;
  *st = pT30CmdPrm->FISst;
  *jp = pT30CmdPrm->FISjp;
  }

#else

  *vr = t30NgtPrms.FCSvr;
  *br = t30NgtPrms.FCSbr;
  *wd = t30NgtPrms.FCSwd;
  *ln = t30NgtPrms.FCSln;
  *df = t30NgtPrms.FCSdf;
  *ec = t30NgtPrms.FCSec;
  *bf = t30NgtPrms.FCSbf;
  *st = t30NgtPrms.FCSst;
  *jp = t30NgtPrms.FCSjp;

#endif

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFPS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FPS? AT command
            which returns the current setting of the post page 
            response.

            <ppr>:   post page response.
*/

GLOBAL T_ACI_RETURN qAT_PlusFPS (T_ACI_CMD_SRC srcId, 
                                 T_ACI_FPS_PPR * ppr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFPS()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *ppr = pT30CmdPrm -> FPSppr;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFSP              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FSP? AT command
            which returns the current polling possibilities.

            <value>:   polling mode.
*/

GLOBAL T_ACI_RETURN qAT_PlusFSP   (T_ACI_CMD_SRC  srcId, 
                                   T_ACI_FSP_VAL * value )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFSP()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *value = pT30CmdPrm -> FSPval;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30Q                 |
| STATE   : code                  ROUTINE : qAT_PlusFAP              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FAP? AT command
            which returns the current address and polling capabilities.

            <sub>: destination subaddressing.
            <sep>: selective polling.
            <pwd>: password. 
*/

GLOBAL T_ACI_RETURN qAT_PlusFAP   (T_ACI_CMD_SRC   srcId, 
                                   T_ACI_FAP_VAL * sub,
                                   T_ACI_FAP_VAL * sep,
                                   T_ACI_FAP_VAL * pwd )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("qAT_PlusFAP()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */  
  *sub = pT30CmdPrm -> FAPsub;
  *sep = pT30CmdPrm -> FAPsep;
  *pwd = pT30CmdPrm -> FAPpwd;

  return( AT_CMPL );
}
#endif /* DTI OR FF_FAX*/

/*==== EOF ========================================================*/
