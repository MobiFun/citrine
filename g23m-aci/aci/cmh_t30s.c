/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_T30S
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
|             protocol stack adapter for T30.
+-----------------------------------------------------------------------------
*/

#if defined (DTI) || defined (FF_FAX)
 
#ifndef CMH_T30S_C
#define CMH_T30S_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci_fd.h"
#include "aci_io.h"
#include "psa.h"
#include "psa_t30.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_t30.h"
#include "cmh_cc.h"


#include "cmh_ra.h"

#include "psa_uart.h"
#include "cmh_uart.h"
#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#include "ati_src_psi.h"
#endif /*FF_PSI*/

/*==== CONSTANTS ==================================================*/

#define T30_DEF_THLD    (90)    /* good page threshold in percent */

#ifdef _SIMULATION_
#define T30_DEF_FRMPRIM (3)     /* no. frames per primitive */
#else
#define T30_DEF_FRMPRIM (42)    /* no. frames per primitive */
#endif

/*==== EXPORT =====================================================*/

/* remove after testing ??? */
/* EXTERN void TST_getSimData(UBYTE *data, USHORT nomLen,
                           USHORT *actLen, UBYTE *final); */

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFCLASS           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCLASS AT
            command which is responsible to set the class mode.

            <class_type>:   class mode.
*/

GLOBAL T_ACI_RETURN sAT_PlusFCLASS  ( T_ACI_CMD_SRC srcId,
                                   T_ACI_FCLASS_CLASS class_type )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFCLASS()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the class_type parameter
 *-------------------------------------------------------------------
 */
  switch( class_type )
  {
    case( FCLASS_CLASS_NotPresent ):
      break;

    case( FCLASS_CLASS_Voice ):
    case( FCLASS_CLASS_Data  ):
    case( FCLASS_CLASS_Fax20 ):
      pT30CmdPrm -> FCLASSclass = class_type;
      break;

    case( FCLASS_CLASS_Fax2  ):         /* not implemented */
    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFCR              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCR AT
            command which is responsible to set the receive
            capability.

            <value>:   receive capability.
*/

GLOBAL T_ACI_RETURN sAT_PlusFCR  ( T_ACI_CMD_SRC srcId,
                                   T_ACI_FCR_VAL value )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFCR()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the value parameter
 *-------------------------------------------------------------------
 */
  switch( value )
  {
    case( FCR_VAL_NotPresent ):
      break;

    case( FCR_VAL_NoRcvCap ):
    case( FCR_VAL_RcvCap  ):
      pT30CmdPrm -> FCRval = value;
      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFLI              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FLI AT
            command which is responsible to set the local id string
            (CSI/TSI).

            <idStr>:   local id string(0 terminated).
*/

GLOBAL T_ACI_RETURN sAT_PlusFLI  ( T_ACI_CMD_SRC srcId,
                                   CHAR * idStr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFLI()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the idStr parameter
 *-------------------------------------------------------------------
 */
  if( ! idStr )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  strncpy( pT30CmdPrm -> FLIstr, idStr, MAX_ID_CHAR-1 );
  pT30CmdPrm -> FLIstr[MAX_ID_CHAR-1] = 0;

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFPI              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FPI AT
            command which is responsible to set the local id string
            (CGI).

            <idStr>:   local id string(0 terminated).
*/

GLOBAL T_ACI_RETURN sAT_PlusFPI  ( T_ACI_CMD_SRC srcId,
                                   CHAR * idStr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFPI()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the idStr parameter
 *-------------------------------------------------------------------
 */
  if( ! idStr )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  strncpy( pT30CmdPrm -> FPIstr, idStr, MAX_ID_CHAR-1 );
  pT30CmdPrm -> FPIstr[MAX_ID_CHAR-1] = 0;

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFSA              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FSA AT
            command which is responsible to set the destination
            subaddress string

            <subStr>:   destination subaddress string(0 terminated).
*/

GLOBAL T_ACI_RETURN sAT_PlusFSA  ( T_ACI_CMD_SRC srcId,
                                   CHAR * subStr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFSA()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the idStr parameter
 *-------------------------------------------------------------------
 */
  if( ! subStr )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  strncpy( pT30CmdPrm -> FSAsub, subStr, MAX_ID_CHAR-1 );
  pT30CmdPrm -> FSAsub[MAX_ID_CHAR-1] = 0;

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFPA              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FPA AT
            command which is responsible to set the selective polling
            address string.

            <sepStr>:   selective polling address string(0 terminated).
*/

GLOBAL T_ACI_RETURN sAT_PlusFPA  ( T_ACI_CMD_SRC srcId,
                                   CHAR * sepStr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFPA()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the idStr parameter
 *-------------------------------------------------------------------
 */
  if( ! sepStr )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  strncpy( pT30CmdPrm -> FPAsep, sepStr, MAX_ID_CHAR-1 );
  pT30CmdPrm -> FPAsep[MAX_ID_CHAR-1] = 0;

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFPW              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FPW AT
            command which is responsible to set the password string.

            <pwdStr>:   password string(0 terminated).
*/

GLOBAL T_ACI_RETURN sAT_PlusFPW  ( T_ACI_CMD_SRC srcId,
                                   CHAR * pwdStr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFPW()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the idStr parameter
 *-------------------------------------------------------------------
 */
  if( ! pwdStr )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  strncpy( pT30CmdPrm -> FPWpwd, pwdStr, MAX_ID_CHAR-1 );
  pT30CmdPrm -> FPWpwd[MAX_ID_CHAR-1] = 0;

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFCC              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCC AT
            command which is responsible to set the DCE capabilities.

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

GLOBAL T_ACI_RETURN sAT_PlusFCC  ( T_ACI_CMD_SRC srcId, T_ACI_F_VR vr,
                                   T_ACI_F_BR br, T_ACI_F_WD wd,
                                   T_ACI_F_LN ln, T_ACI_F_DF df,
                                   T_ACI_F_EC ec, T_ACI_F_BF bf,
                                   T_ACI_F_ST st, T_ACI_F_JP jp )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFCC()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the vr parameter
 *-------------------------------------------------------------------
 */
  switch (vr)
  {
  case F_VR_NotPresent:
    vr = pT30CmdPrm -> FCCvr;
    break;

  case F_VR_R8X3_85:
  case F_VR_R8X7_7:
  case F_VR_R8X15_4:
  case F_VR_R16X15_4:
  case F_VR_200X100:
  case F_VR_200X200:
  case F_VR_200X400:
  case F_VR_300X300:
    break;

  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

/*
 *-------------------------------------------------------------------
 * process the br parameter
 *-------------------------------------------------------------------
 */
  switch( br )
  {
    case( F_BR_NotPresent ):

      br = pT30CmdPrm -> FCCbr;
      break;

    case( F_BR_2400  ):
    case( F_BR_4800  ):
    case( F_BR_7200  ):
    case( F_BR_9600  ):
    case( F_BR_12000 ):
    case( F_BR_14400 ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the wd parameter
 *-------------------------------------------------------------------
 */
  switch( wd )
  {
    case( F_WD_NotPresent ):

      wd = pT30CmdPrm -> FCCwd;
      break;

    case( F_WD_1728  ):
    case( F_WD_2048  ):
    case( F_WD_2432  ):
    case( F_WD_1216  ):
    case( F_WD_864 ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the ln parameter
 *-------------------------------------------------------------------
 */
  switch( ln )
  {
    case( F_LN_NotPresent ):

      ln = pT30CmdPrm -> FCCln;
      break;

    case( F_LN_A4  ):
    case( F_LN_B4  ):
    case( F_LN_Unlimited  ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the df parameter
 *-------------------------------------------------------------------
 */
  switch( df )
  {
    case( F_DF_NotPresent ):

      df = pT30CmdPrm -> FCCdf;
      break;

    case( F_DF_1D_MdfHuff  ):

      break;

    case( F_DF_2D_MdfRd_T4 ):       /* not implemented */
    case( F_DF_2D_Uncomp   ):       /* not implemented */
    case( F_DF_2D_MdfRd_T6 ):       /* not implemented */
    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the ec parameter
 *-------------------------------------------------------------------
 */
  switch( ec )
  {
    case( F_EC_NotPresent ):

      ec = pT30CmdPrm -> FCCec;
      break;

    case( F_EC_DisableECM  ):

      break;

    case( F_EC_EnableECM     ):       /* not implemented */
    case( F_EC_EnableHalfDup ):       /* not implemented */
    case( F_EC_EnableFullDup ):       /* not implemented */
    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the bf parameter
 *-------------------------------------------------------------------
 */
  if( bf NEQ F_BF_NotPresent )
  {
    if( bf > F_BF_DisableFileTrnsf )  /* others are not implemented */
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }
  else

    bf = pT30CmdPrm -> FCCbf;

/*
 *-------------------------------------------------------------------
 * process the st parameter
 *-------------------------------------------------------------------
 */
  switch( st )
  {
    case( F_ST_NotPresent ):

      st = pT30CmdPrm -> FCCst;
      break;

    case( F_ST_0_0   ):
    case( F_ST_5_5   ):
    case( F_ST_10_5  ):
    case( F_ST_10_10 ):
    case( F_ST_20_10 ):
    case( F_ST_20_20 ):
    case( F_ST_40_20 ):
    case( F_ST_40_40 ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the jp parameter
 *-------------------------------------------------------------------
 */
  if( jp NEQ F_JP_NotPresent )
  {
    if( jp > F_JP_DisableJPEG )       /* others are not implemented */
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }
  else

    jp = pT30CmdPrm -> FCCjp;


/*
 *-------------------------------------------------------------------
 * assign the parameters
 *-------------------------------------------------------------------
 */
  pT30CmdPrm -> FCCvr = pT30CmdPrm -> FISvr = vr;
  pT30CmdPrm -> FCCbr = pT30CmdPrm -> FISbr = br;
  pT30CmdPrm -> FCCwd = pT30CmdPrm -> FISwd = wd;
  pT30CmdPrm -> FCCln = pT30CmdPrm -> FISln = ln;
  pT30CmdPrm -> FCCdf = pT30CmdPrm -> FISdf = df;
  pT30CmdPrm -> FCCec = pT30CmdPrm -> FISec = ec;
  pT30CmdPrm -> FCCbf = pT30CmdPrm -> FISbf = bf;
  pT30CmdPrm -> FCCst = pT30CmdPrm -> FISst = st;
  pT30CmdPrm -> FCCjp = pT30CmdPrm -> FISjp = jp;


/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFIS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FIS AT
            command which is responsible to set the current session
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

GLOBAL T_ACI_RETURN sAT_PlusFIS  ( T_ACI_CMD_SRC srcId, T_ACI_F_VR vr,
                                   T_ACI_F_BR br, T_ACI_F_WD wd,
                                   T_ACI_F_LN ln, T_ACI_F_DF df,
                                   T_ACI_F_EC ec, T_ACI_F_BF bf,
                                   T_ACI_F_ST st, T_ACI_F_JP jp )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFIS()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the vr parameter
 *-------------------------------------------------------------------
 */
  switch (vr)
  {
  case F_VR_NotPresent:
    vr = pT30CmdPrm -> FISvr;
    break;

  case F_VR_R8X3_85:
  case F_VR_R8X7_7:
  case F_VR_R8X15_4:
  case F_VR_R16X15_4:
  case F_VR_200X100:
  case F_VR_200X200:
  case F_VR_200X400:
  case F_VR_300X300:
    break;

  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

/*
 *-------------------------------------------------------------------
 * process the br parameter
 *-------------------------------------------------------------------
 */
  switch( br )
  {
    case( F_BR_NotPresent ):

      br = pT30CmdPrm -> FISbr;
      break;

    case( F_BR_2400  ):
    case( F_BR_4800  ):
    case( F_BR_7200  ):
    case( F_BR_9600  ):
    case( F_BR_12000 ):
    case( F_BR_14400 ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the wd parameter
 *-------------------------------------------------------------------
 */
  switch( wd )
  {
    case( F_WD_NotPresent ):

      wd = pT30CmdPrm -> FISwd;
      break;

    case( F_WD_1728  ):
    case( F_WD_2048  ):
    case( F_WD_2432  ):
    case( F_WD_1216  ):
    case( F_WD_864 ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the ln parameter
 *-------------------------------------------------------------------
 */
  switch( ln )
  {
    case( F_LN_NotPresent ):

      ln = pT30CmdPrm -> FISln;
      break;

    case( F_LN_A4  ):
    case( F_LN_B4  ):
    case( F_LN_Unlimited  ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the df parameter
 *-------------------------------------------------------------------
 */
  switch( df )
  {
    case( F_DF_NotPresent ):

      df = pT30CmdPrm -> FISdf;
      break;

    case( F_DF_1D_MdfHuff  ):

      break;

    case( F_DF_2D_MdfRd_T4 ):       /* not implemented */
    case( F_DF_2D_Uncomp   ):       /* not implemented */
    case( F_DF_2D_MdfRd_T6 ):       /* not implemented */
    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the ec parameter
 *-------------------------------------------------------------------
 */
  switch( ec )
  {
    case( F_EC_NotPresent ):

      ec = pT30CmdPrm -> FISec;
      break;

    case( F_EC_DisableECM  ):

      break;

    case( F_EC_EnableECM     ):       /* not implemented */
    case( F_EC_EnableHalfDup ):       /* not implemented */
    case( F_EC_EnableFullDup ):       /* not implemented */
    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the bf parameter
 *-------------------------------------------------------------------
 */
  if( bf NEQ F_BF_NotPresent )
  {
    if( bf > F_BF_DisableFileTrnsf )  /* others are not implemented */
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }
  else

    bf = pT30CmdPrm -> FISbf;

/*
 *-------------------------------------------------------------------
 * process the st parameter
 *-------------------------------------------------------------------
 */
  switch( st )
  {
    case( F_ST_NotPresent ):

      st = pT30CmdPrm -> FISst;
      break;

    case( F_ST_0_0   ):
    case( F_ST_5_5   ):
    case( F_ST_10_5  ):
    case( F_ST_10_10 ):
    case( F_ST_20_10 ):
    case( F_ST_20_20 ):
    case( F_ST_40_20 ):
    case( F_ST_40_40 ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the jp parameter
 *-------------------------------------------------------------------
 */
  if( jp NEQ F_JP_NotPresent )
  {
    if( jp > F_JP_DisableJPEG )       /* others are not implemented */
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }
  else

    jp = pT30CmdPrm -> FISjp;

/*
 *-------------------------------------------------------------------
 * assign the parameters
 *-------------------------------------------------------------------
 */
  pT30CmdPrm -> FISvr = vr;
  pT30CmdPrm -> FISbr = br;
  pT30CmdPrm -> FISwd = wd;
  pT30CmdPrm -> FISln = ln;
  pT30CmdPrm -> FISdf = df;
  pT30CmdPrm -> FISec = ec;
  pT30CmdPrm -> FISbf = bf;
  pT30CmdPrm -> FISst = st;
  pT30CmdPrm -> FISjp = jp;

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFNS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FNS AT
            command which is responsible to set the string for
            non-standard facilities.

            <len>:   length of nsf string.
            <nsf>:   nsf octed string.
*/

GLOBAL T_ACI_RETURN sAT_PlusFNS  ( T_ACI_CMD_SRC srcId,
                                   UBYTE len,
                                   UBYTE * nsf )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFNS()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the len and nsf parameter
 *-------------------------------------------------------------------
 */
  if((len > 0 AND ! nsf) OR len > MAX_NSF_BYTE )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  if( len EQ 0 )
  {
    pT30CmdPrm -> FNSlen = 0;
  }
  else
  {
    if (MINIMUM(MAX_NSF_BYTE - pT30CmdPrm -> FNSlen, len))
    {
      memcpy( &pT30CmdPrm -> FNSoct[pT30CmdPrm -> FNSlen], nsf,
              MINIMUM(MAX_NSF_BYTE - pT30CmdPrm -> FNSlen, len) );
    }

    pT30CmdPrm -> FNSlen +=
                  MINIMUM(MAX_NSF_BYTE - pT30CmdPrm -> FNSlen, len);
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFLP              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FLP AT
            command which is responsible to indicate a document for
            polling.

            <value>: polling indication.
*/

GLOBAL T_ACI_RETURN sAT_PlusFLP   (T_ACI_CMD_SRC srcId,
                                   T_ACI_FLP_VAL value )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFLP()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the value parameter
 *-------------------------------------------------------------------
 */
  switch( value )
  {
    case( FLP_VAL_NotPresent ):

      break;

    case( FLP_VAL_NoPollDoc  ):
    case( FLP_VAL_PollDoc    ):

      pT30CmdPrm -> FLPval = value;
      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFIE              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FIE AT
            command which is responsible to enable procedure interrupts.

            <value>: PI mode.
*/

GLOBAL T_ACI_RETURN sAT_PlusFIE   (T_ACI_CMD_SRC srcId,
                                   T_ACI_FIE_VAL value )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFIE()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the value parameter
 *-------------------------------------------------------------------
 */
  switch( value )
  {
    case( FIE_VAL_NotPresent ):

      break;

    case( FIE_VAL_IgnorePRI ):
    case( FIE_VAL_AcceptPRI ):

      pT30CmdPrm -> FIEval = value;
      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFCQ              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCQ AT
            command which is responsible to set the copy quality
            checking.

            <rq>: receive quality.
            <tq>: transmit quality.
*/

GLOBAL T_ACI_RETURN sAT_PlusFCQ   (T_ACI_CMD_SRC srcId,
                                   T_ACI_FCQ_RQ  rq,
                                                 T_ACI_FCQ_TQ  tq )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFCQ()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the rq parameter
 *-------------------------------------------------------------------
 */
  switch( rq )
  {
    case( FCQ_RQ_NotPresent ):

      rq = pT30CmdPrm -> FCQrq;
      break;

    case( FCQ_RQ_CQCDisabled ):
    case( FCQ_RQ_CQCEnabled  ):

      break;

    case( FCQ_RQ_CQCandCorrection ):
    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the tq parameter
 *-------------------------------------------------------------------
 */
  switch( tq )
  {
    case( FCQ_TQ_NotPresent ):
    case( FCQ_TQ_CQCDisabled ):

      break;

    case( FCQ_TQ_CQCEnabled  ):
    case( FCQ_TQ_CQCandCorrection ):
    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * assign the parameters
 *-------------------------------------------------------------------
 */
  pT30CmdPrm -> FCQrq = rq;

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFIT              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FIT AT
            command which is responsible to set the inactivity timeout.

            <time>: inactivity timeout.
            <act> : action.
*/

GLOBAL T_ACI_RETURN sAT_PlusFIT   (T_ACI_CMD_SRC        srcId,
                                   SHORT                time,
                                   T_ACI_FIT_ACT        act )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFIT()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the act parameter
 *-------------------------------------------------------------------
 */
  switch( act )
  {
    case( FIT_ACT_NotPresent ):

      act = pT30CmdPrm -> FITact;
      break;

    case( FIT_ACT_OnHookRst ):
    case( FIT_ACT_OnHook    ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the time parameter
 *-------------------------------------------------------------------
 */
  if( time > 255 )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * assign the parameters
 *-------------------------------------------------------------------
 */
  pT30CmdPrm -> FITact = act;

  if( time NEQ ACI_NumParmNotPresent )
  {
    pT30CmdPrm -> FITtime =  time * 1000;
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFBO              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FBO AT
            command which is responsible to set data bit order.

            <value>: data bit order.
*/

GLOBAL T_ACI_RETURN sAT_PlusFBO   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_FBO_VAL        value )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFBO()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the value parameter
 *-------------------------------------------------------------------
 */
  switch( value )
  {
    case( FBO_VAL_NotPresent ):

      value = pT30CmdPrm -> FBOval;
      break;

    case( FBO_VAL_DirCDirBD ):
    case( FBO_VAL_RvrCDirBD ):
    case( FBO_VAL_DirCRvrBD ):
    case( FBO_VAL_RvrCRvrBD ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * assign the parameters
 *-------------------------------------------------------------------
 */
  pT30CmdPrm -> FBOval = value;

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFRQ              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FRQ AT
            command which is responsible to set the receive quality
            thresholds.

            <pgl>: percentage of good lines.
            <cbl>: consecutive bad lines.
*/

GLOBAL T_ACI_RETURN sAT_PlusFRQ   (T_ACI_CMD_SRC srcId,
                                   SHORT pgl,
                                   SHORT cbl )
{
  TRACE_FUNCTION ("sAT_PlusFRQ()");

  /* process the pgl parameter */
  if( pgl NEQ 0 )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  /* process the cbl parameter */
  if( cbl NEQ 0 )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  /* start FIT timer */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFEA              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FEA AT
            command which is responsible to set phase C EOL alignment.

            <value>: alignment.
*/

GLOBAL T_ACI_RETURN sAT_PlusFEA   (T_ACI_CMD_SRC srcId,
                                   SHORT value )
{

  TRACE_FUNCTION ("sAT_PlusFEA()");

/*
 *-------------------------------------------------------------------
 * process the value parameter
 *-------------------------------------------------------------------
 */
  if( value NEQ ACI_NumParmNotPresent )
  {
    if( value NEQ 0 )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFCT              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FCT AT
            command which is responsible to set phase C response
            timeout.

            <value>: timeout.
*/

GLOBAL T_ACI_RETURN sAT_PlusFCT   (T_ACI_CMD_SRC srcId,
                                   SHORT value )
{

  TRACE_FUNCTION ("sAT_PlusFCT()");

/*
 *-------------------------------------------------------------------
 * process the value parameter
 *-------------------------------------------------------------------
 */
  if( value NEQ ACI_NumParmNotPresent )
  {
    if( value NEQ 30 )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFMS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FMS AT
            command which is responsible to set the minimum phase C
            speed.

            <br>: minimum phase C baud rate.
*/

GLOBAL T_ACI_RETURN sAT_PlusFMS (T_ACI_CMD_SRC srcId,
                                 T_ACI_F_BR br)
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFMS()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the br parameter
 *-------------------------------------------------------------------
 */
  switch( br )
  {
    case( F_BR_NotPresent ):

      break;

    case( F_BR_2400  ):
    case( F_BR_4800  ):
    case( F_BR_7200  ):
    case( F_BR_9600  ):
    case( F_BR_12000 ):
    case( F_BR_14400 ):

      pT30CmdPrm -> FMSbr = br;
      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFFC              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FFC AT
            command which is responsible to set the format conversion
            parameters.

            <vrc>: vertical resolution format.
            <dfc>: data format.
            <lnc>: page length format.
            <wdc>: page width format.
*/

GLOBAL T_ACI_RETURN sAT_PlusFFC (T_ACI_CMD_SRC srcId,
                                 T_ACI_FFC_VRC vrc,
                                 T_ACI_FFC_DFC dfc,
                                 T_ACI_FFC_LNC lnc,
                                 T_ACI_FFC_WDC wdc)
{

  TRACE_FUNCTION ("sAT_PlusFFC()");

/*
 *-------------------------------------------------------------------
 * process the vrc parameter
 *-------------------------------------------------------------------
 */
  if( vrc NEQ FFC_VRC_NotPresent )
  {
    if( vrc NEQ FFC_VRC_Ignored )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * process the dfc parameter
 *-------------------------------------------------------------------
 */
  if( dfc NEQ FFC_DFC_NotPresent )
  {
    if( dfc NEQ FFC_DFC_Ignored )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * process the lnc parameter
 *-------------------------------------------------------------------
 */
  if( lnc NEQ FFC_LNC_NotPresent )
  {
    if( lnc NEQ FFC_LNC_Ignored )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * process the wdc parameter
 *-------------------------------------------------------------------
 */
  if( wdc NEQ FFC_WDC_NotPresent )
  {
    if( wdc NEQ FFC_WDC_Ignored )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFIP              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FIP AT
            command which is responsible to reset the FAX parameter to
            their initial value.

*/

GLOBAL T_ACI_RETURN sAT_PlusFIP (T_ACI_CMD_SRC srcId)
{

  TRACE_FUNCTION ("sAT_PlusFIP()");

/*
 *-------------------------------------------------------------------
 * check command source and reset parameters
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  cmhT30_StopFIT();

  cmhT30_InitFAXPrms( srcId );

  cmhT30_RstNgtPrms();

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFPS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FPS AT command
            which is responsible to set the post page response.

            <ppr>:   post page response.
*/

GLOBAL T_ACI_RETURN sAT_PlusFPS (T_ACI_CMD_SRC srcId,
                                 T_ACI_FPS_PPR ppr )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFPS()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the ppr parameter
 *-------------------------------------------------------------------
 */
  switch( ppr )
  {
    case( FPS_PPR_NotPresent ):
      break;

    case( FPS_PPR_Mcf ):
    case( FPS_PPR_Rtn ):
    case( FPS_PPR_Rtp ):
    case( FPS_PPR_Pip ):
    case( FPS_PPR_Pin ):

      pT30CmdPrm -> FPSppr = ppr;
      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFSP              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FSP AT command
            which is responsible to set the possibility of polling.

            <value>:   polling mode.
*/

GLOBAL T_ACI_RETURN sAT_PlusFSP   (T_ACI_CMD_SRC srcId,
                                   T_ACI_FSP_VAL value )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFSP()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the value parameter
 *-------------------------------------------------------------------
 */
  switch( value )
  {
    case( FSP_VAL_NotPresent ):
      break;

    case( FSP_VAL_PollDisabled ):
    case( FSP_VAL_PollEnable ):

      pT30CmdPrm -> FSPval = value;
      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFAP              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FAP AT
            command which is responsible to set the address and
            polling capabilities.

            <sub>: destination subaddressing.
            <sep>: selective polling.
            <pwd>: password.
*/

GLOBAL T_ACI_RETURN sAT_PlusFAP   (T_ACI_CMD_SRC srcId,
                                   T_ACI_FAP_VAL sub,
                                   T_ACI_FAP_VAL sep,
                                   T_ACI_FAP_VAL pwd )
{
  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  TRACE_FUNCTION ("sAT_PlusFAP()");

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
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * process the sub parameter
 *-------------------------------------------------------------------
 */
  switch( sub )
  {
    case( FAP_VAL_NotPresent ):

      sub = pT30CmdPrm -> FAPsub;
      break;

    case( FAP_VAL_Disabled ):
    case( FAP_VAL_Enabled  ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the sub parameter
 *-------------------------------------------------------------------
 */
  switch( sep )
  {
    case( FAP_VAL_NotPresent ):

      sep = pT30CmdPrm -> FAPsep;
      break;

    case( FAP_VAL_Disabled ):
    case( FAP_VAL_Enabled  ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the sub parameter
 *-------------------------------------------------------------------
 */
  switch( pwd )
  {
    case( FAP_VAL_NotPresent ):

      pwd = pT30CmdPrm -> FAPpwd;
      break;

    case( FAP_VAL_Disabled ):
    case( FAP_VAL_Enabled  ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * assign the parameters
 *-------------------------------------------------------------------
 */
  pT30CmdPrm -> FAPsub = sub;
  pT30CmdPrm -> FAPsep = sep;
  pT30CmdPrm -> FAPpwd = pwd;

/*
 *-------------------------------------------------------------------
 * start FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StartFIT();

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFDT              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FDT AT
            command which is responsible to transmit FAX data.

*/

GLOBAL T_ACI_RETURN sAT_PlusFDT (T_ACI_CMD_SRC srcId)
{

  TRACE_FUNCTION ("sAT_PlusFDT()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
   if(!cmh_IsVldCmdSrc (srcId) OR
      !psaCC_ctbIsValid (t30ShrdPrm.cId) OR
     (t30ShrdPrm.faxStat NEQ FS_IDL AND t30ShrdPrm.faxStat NEQ FS_SND_DOC))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * stop FIT timer
 *-------------------------------------------------------------------
 */
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( t30EntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

  t30EntStat.curCmd = AT_CMD_FDT;
  t30EntStat.entOwn = srcId;

/*
 *-------------------------------------------------------------------
 * negotiate transmission parameters and send DCS
 *-------------------------------------------------------------------
 */
  if( t30ShrdPrm.faxStat EQ FS_IDL OR
      t30ShrdPrm.faxStat EQ FS_DOC_TRF )
  {
    cmhT30_NgtDCEPrms( srcId );

    /* check for minimum speed condition */
    if( t30NgtPrms.FCSbr < fnd_cmhPrm[srcId].t30CmdPrm.FMSbr )
    {
      TRACE_EVENT("DISCONNECT CALL DUE TO FALL BELOW MINIMUM SPEED CONDITION");
      t30ShrdPrm.faxStat = FS_IDL;
      t30EntStat.curCmd  = AT_CMD_NONE;
      ccShrdPrm.datStat  = DS_DSC_REQ;
      cmhT30_Deactivate ();
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_FaxMinSpeedCond );
      return( AT_FAIL );
    }
    cmhT30_SendCaps ( srcId, FRT_DCS );
  }

/*
 *-------------------------------------------------------------------
 * check for connect message
 *-------------------------------------------------------------------
 */
    if( t30ShrdPrm.faxStat EQ FS_IDL OR t30ShrdPrm.faxStat EQ FS_DOC_TRF )
    {
      R_AT( RAT_FCS, t30EntStat.entOwn )
        ( t30NgtPrms.FCSvr,
          t30NgtPrms.FCSbr,
          t30NgtPrms.FCSwd,
          t30NgtPrms.FCSln,
          t30NgtPrms.FCSdf,
          t30NgtPrms.FCSec,
          t30NgtPrms.FCSbf,
          t30NgtPrms.FCSst,
          t30NgtPrms.FCSjp);
    }
    t30ShrdPrm.faxStat = FS_SND_DOC;
    pageSentFlg = FALSE;
    ppmPendFlg  = FALSE;
    cmhT30_StartFIT();
    TRACE_EVENT ("START DTE->DCE FAX DATA PHASE");
/*
    R_AT( RAT_CONNECT, t30EntStat.entOwn )
      ( t30EntStat.curCmd, cmhT30_GetDataRate(), t30ShrdPrm.cId+1, FALSE );
*/
  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFDR              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FDR AT
            command which is responsible to receive FAX data.

*/

GLOBAL T_ACI_RETURN sAT_PlusFDR (T_ACI_CMD_SRC srcId)
{
  TRACE_FUNCTION ("sAT_PlusFDR()");
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId) OR
     !psaCC_ctbIsValid (t30ShrdPrm.cId) OR
     fnd_cmhPrm[srcId].t30CmdPrm.FCRval EQ FCR_VAL_NoRcvCap OR
     (t30ShrdPrm.faxStat NEQ FS_IDL AND t30ShrdPrm.faxStat NEQ FS_RCV_DOC))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
  cmhT30_StopFIT();

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( t30EntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

  t30EntStat.curCmd = AT_CMD_FDR;
  t30EntStat.entOwn = srcId;

/*
 *-------------------------------------------------------------------
 * check to send a DTC frame
 *-------------------------------------------------------------------
 */
  if( DTCSentFlg EQ FALSE                                    AND
      (t30ShrdPrm.faxStat EQ FS_IDL OR
       t30ShrdPrm.faxStat EQ FS_DOC_TRF)                     AND
      psaCC_ctb(t30ShrdPrm.cId)->calType EQ CT_MOC           AND
      fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FSPval EQ FSP_VAL_PollEnable )
  {
    DTCSentFlg = TRUE;
    cmhT30_SendCaps ( srcId, FRT_DTC );
  }

/*
 *-------------------------------------------------------------------
 * check to send a post page response
 *-------------------------------------------------------------------
 */
  else if (t30ShrdPrm.faxStat EQ FS_RCV_DOC OR t30ShrdPrm.faxStat EQ FS_DOC_TRF)
  {
    t30ShrdPrm.sgn_snd = cmhT30_GetPpr(fnd_cmhPrm[srcId].t30CmdPrm.FPSppr);
    if( psaT30_Ppm( ) < 0 )  /* send ppm */
    {
      TRACE_EVENT( "FATAL RETURN psaT30 in sAT_PlusFDR" );
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * check for connect message
 *-------------------------------------------------------------------
 */
  t30ShrdPrm.faxStat = FS_RCV_DOC;
  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30S                 |
| STATE   : code                  ROUTINE : sAT_PlusFKS              |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +FKS AT
            command which is responsible to terminate a FAX session.

*/

GLOBAL T_ACI_RETURN sAT_PlusFKS (T_ACI_CMD_SRC srcId)
{
  TRACE_FUNCTION ("sAT_PlusFKS()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
  cmhT30_StopFIT();

  /* check entity status */
  if( t30EntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

  /* check for active call */
  if (!psaCC_ctbIsValid (t30ShrdPrm.cId))
    return( AT_CMPL );

  t30EntStat.curCmd = AT_CMD_FKS;
  t30EntStat.entOwn = srcId;

  /* disconnect T30 */
  psaT30_Disconnect( );

  return( AT_EXCT );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_Activate         |
+-------------------------------------------------------------------+

  PURPOSE : set all nescessary parameters and activate T30

*/

GLOBAL T_ACI_RETURN cmhT30_Activate ( T_ACI_CMD_SRC srcId,
                                      T_ACI_AT_CMD cmdId,
                                      SHORT cId )
{
  TRACE_FUNCTION ("cmhT30_Activate()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
    return( AT_FAIL );

  /* check entity status */
  if( t30EntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

  /* fill in activate parameters */
  t30ShrdPrm.trans_rate = cmhT30_SelChnRate();
  if( t30ShrdPrm.trans_rate EQ NOT_PRESENT_16BIT )
    return( AT_FAIL );

  t30ShrdPrm.bitord = cmhT30_SelBitOrder( srcId );
  if( t30ShrdPrm.bitord EQ NOT_PRESENT_8BIT )
    return( AT_FAIL );

  t30ShrdPrm.half_rate        = cmhT30_SelHlfRate();
  t30ShrdPrm.threshold        = T30_DEF_THLD;
  t30ShrdPrm.frames_per_prim  = T30_DEF_FRMPRIM;
  t30ShrdPrm.hdlc_report      = TRUE;
  t30ShrdPrm.test_mode        = 0;    /* needed for simulation */

  t30ShrdPrm.cId     = cId;
  t30EntStat.curCmd  = cmdId;
  t30EntStat.entOwn  = srcId;

  FHSstat = FHS_STAT_NormEnd;         /* reset call termination status */

  psaT30_Activate( );
  return( AT_EXCT );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_Deactivate       |
+-------------------------------------------------------------------+

  PURPOSE : set all nescessary parameters and deactivate T30

*/

GLOBAL T_ACI_RETURN cmhT30_Deactivate ( void )
{
  UBYTE src_id = t30EntStat.entOwn;
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI  *src_infos = find_element (psi_src_params, 
                                 src_id, cmhPSItest_srcId);
#endif /*FF_PSI*/
  T_DTI_CNTRL    info;

  TRACE_FUNCTION ("cmht30_Deactivate()");


  if (dti_cntrl_get_info_from_src_id( src_id, &info) EQ FALSE)
  {
    TRACE_EVENT_P1("cannot find info for src_id=%d", src_id);
    return( AT_FAIL );
  }

  if (dti_cntrl_is_dti_channel_connected(DTI_ENTITY_T30, info.dti_id))
  {
    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};
#ifdef FF_PSI
    if (src_infos NEQ NULL)
      dti_cntrl_est_dpath_indirect ( src_id,
                                   entity_list,
                                   1,
                                   SPLIT,
                                   atiPSI_dti_cb,
                                   DTI_CPBLTY_CMD,
                                   DTI_CID_NOTPRESENT);
    else
#endif
      dti_cntrl_est_dpath_indirect ( src_id,
                                   entity_list,
                                   1,
                                   SPLIT,
                                   atiUART_dti_cb,
                                   DTI_CPBLTY_CMD,
                                   DTI_CID_NOTPRESENT);

    return( AT_EXCT );
  }

  /* reset parameters to initial value */
  cmhT30_RstNgtPrms();              /* reset negotiation parameter */

  /* deactivate */
  psaT30_Deactivate( );
  return( AT_EXCT );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_Modify           |
+-------------------------------------------------------------------+

  PURPOSE : set all nescessary parameters and modify T30

*/

GLOBAL T_ACI_RETURN cmhT30_Modify (void)
{
  TRACE_FUNCTION ("cmhT30_Modify()");

  /* fill in modify parameters */
  t30ShrdPrm.trans_rate = cmhT30_SelUsrRate();
  if (t30ShrdPrm.trans_rate EQ NOT_PRESENT_16BIT)
  {
    t30ShrdPrm.trans_rate = cmhT30_SelChnRate();
  }
  if (t30ShrdPrm.trans_rate EQ NOT_PRESENT_16BIT)
    return (AT_FAIL);

  t30ShrdPrm.half_rate = cmhT30_SelHlfRate();

  psaT30_Modify( );
  return( AT_EXCT );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_SendCaps         |
+-------------------------------------------------------------------+

  PURPOSE : Send local DCE capability parameters

*/

GLOBAL T_ACI_RETURN cmhT30_SendCaps ( T_ACI_CMD_SRC srcId,
                                      T_T30_FRTP frmTyp )
{
  T_T30_CMD_PRM * pT30CmdPrm;  /* points to T30 command parameters */

  TRACE_FUNCTION ("cmhT30_SendCaps()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))

    return( AT_FAIL );

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *-------------------------------------------------------------------
 * build capability parameter
 *-------------------------------------------------------------------
 */
  t30ShrdPrm.hdlc_snd.crp = FALSE;
  cmhT30_BuildSndFrm( srcId, frmTyp );

  switch( frmTyp )
  {
    /*
     *---------------------------------------------------------------
     * for a DIS frame
     *---------------------------------------------------------------
     */
    case( FRT_DIS ):

      t30ShrdPrm.hdlc_snd.v_dcs = t30ShrdPrm.hdlc_snd.v_dtc = FALSE;
      t30ShrdPrm.hdlc_snd.c_nsc = t30ShrdPrm.hdlc_snd.c_nss = 0;
      t30ShrdPrm.hdlc_snd.c_tsi = t30ShrdPrm.hdlc_snd.c_cig = 0;

                                          /* non-standard facility */
      t30ShrdPrm.hdlc_snd.c_nsf = pT30CmdPrm -> FNSlen;

      if (pT30CmdPrm -> FNSlen)
      {
        memcpy( t30ShrdPrm.hdlc_snd.nsf, pT30CmdPrm -> FNSoct,
                pT30CmdPrm -> FNSlen);
      }

      if( pT30CmdPrm -> FLIstr[0] NEQ 0 ) /* calling subscr. id */
      {
        t30ShrdPrm.hdlc_snd.c_csi = strlen( pT30CmdPrm -> FLIstr );
        strncpy( (char *) t30ShrdPrm.hdlc_snd.csi,
                 (char *) pT30CmdPrm -> FLIstr,
                 t30ShrdPrm.hdlc_snd.c_csi );
      }
      else
      {
        t30ShrdPrm.hdlc_snd.c_csi = 0;
      }
      break;

    /*
     *---------------------------------------------------------------
     * for a DTC frame
     *---------------------------------------------------------------
     */
    case( FRT_DTC ):

      t30ShrdPrm.hdlc_snd.v_dcs = t30ShrdPrm.hdlc_snd.v_dis = FALSE;
      t30ShrdPrm.hdlc_snd.c_nsf = t30ShrdPrm.hdlc_snd.c_nss = 0;
      t30ShrdPrm.hdlc_snd.c_tsi = t30ShrdPrm.hdlc_snd.c_csi = 0;

                                          /* non-standard facility */
      t30ShrdPrm.hdlc_snd.c_nsc = pT30CmdPrm -> FNSlen;

      if (pT30CmdPrm -> FNSlen)
      {
        memcpy( t30ShrdPrm.hdlc_snd.nsc, pT30CmdPrm -> FNSoct,
                pT30CmdPrm -> FNSlen);
      }

      if( pT30CmdPrm -> FPWpwd[0] NEQ 0 )     /* password string */
      {
        t30ShrdPrm.hdlc_snd.c_pwd = strlen( pT30CmdPrm -> FPWpwd );
        strncpy( (char *) t30ShrdPrm.hdlc_snd.pwd,
                 (char *) pT30CmdPrm -> FPWpwd,
                 t30ShrdPrm.hdlc_snd.c_pwd );
      }
      else
      {
        t30ShrdPrm.hdlc_snd.c_pwd = 0;
      }

      if( pT30CmdPrm -> FPAsep[0] NEQ 0 )     /* selective polling string */
      {
        t30ShrdPrm.hdlc_snd.c_sep = strlen( pT30CmdPrm -> FPAsep );
        strncpy( (char *) t30ShrdPrm.hdlc_snd.sep,
                 (char *) pT30CmdPrm -> FPAsep,
                 t30ShrdPrm.hdlc_snd.c_sep );
      }
      else
      {
        t30ShrdPrm.hdlc_snd.c_sep = 0;
      }

      if( pT30CmdPrm -> FPIstr[0] NEQ 0 ) /* calling subscr. id */
      {
        t30ShrdPrm.hdlc_snd.c_cig = strlen( pT30CmdPrm -> FPIstr );
        strncpy( (char *) t30ShrdPrm.hdlc_snd.cig,
                 (char *) pT30CmdPrm -> FPIstr,
                 t30ShrdPrm.hdlc_snd.c_cig );
      }
      else
      {
        t30ShrdPrm.hdlc_snd.c_cig = 0;
      }
      break;

    /*
     *---------------------------------------------------------------
     * for a DCS frame
     *---------------------------------------------------------------
     */
    case( FRT_DCS ):

      t30ShrdPrm.hdlc_snd.v_dis = t30ShrdPrm.hdlc_snd.v_dtc = FALSE;
      t30ShrdPrm.hdlc_snd.c_nsc = t30ShrdPrm.hdlc_snd.c_nsf = 0;
      t30ShrdPrm.hdlc_snd.c_csi = t30ShrdPrm.hdlc_snd.c_cig = 0;

                                          /* non-standard facility */
      t30ShrdPrm.hdlc_snd.c_nss = pT30CmdPrm -> FNSlen;

      if (pT30CmdPrm -> FNSlen)
      {
        memcpy( t30ShrdPrm.hdlc_snd.nss, pT30CmdPrm -> FNSoct,
              pT30CmdPrm -> FNSlen);
      }

      if( pT30CmdPrm -> FPWpwd[0] NEQ 0 )     /* password string */
      {
        t30ShrdPrm.hdlc_snd.c_pwd = strlen( pT30CmdPrm -> FPWpwd );
        strncpy( (char *) t30ShrdPrm.hdlc_snd.pwd,
                 (char *) pT30CmdPrm -> FPWpwd,
                 t30ShrdPrm.hdlc_snd.c_pwd );
      }
      else
      {
        t30ShrdPrm.hdlc_snd.c_pwd = 0;
      }

      if( pT30CmdPrm -> FSAsub[0] NEQ 0 )     /* subaddress string */
      {
        t30ShrdPrm.hdlc_snd.c_sub = strlen( pT30CmdPrm -> FSAsub );
        strncpy( (char *) t30ShrdPrm.hdlc_snd.sub,
                 (char *) pT30CmdPrm -> FSAsub,
                 t30ShrdPrm.hdlc_snd.c_sub );
      }
      else
      {
        t30ShrdPrm.hdlc_snd.c_sub = 0;
      }

      if( pT30CmdPrm -> FLIstr[0] NEQ 0 ) /* transmitting subscr. id */
      {
        t30ShrdPrm.hdlc_snd.c_tsi = strlen( pT30CmdPrm -> FLIstr );
        strncpy( (char *) t30ShrdPrm.hdlc_snd.tsi,
                 (char *) pT30CmdPrm -> FLIstr,
                 t30ShrdPrm.hdlc_snd.c_tsi );
      }
      else
      {
        t30ShrdPrm.hdlc_snd.c_tsi = 0;
      }
      break;
  }

/*
 *-------------------------------------------------------------------
 * send capability parameter
 *-------------------------------------------------------------------
 */
  psaT30_Capabilities( );

/*
 *-------------------------------------------------------------------
 * check for T30 modification
 *-------------------------------------------------------------------
 */
  if( frmTyp EQ FRT_DCS )
  {
    if( cmhT30_Chk4TCHAdpt() )
    {
      ccShrdPrm.datStat = DS_TCH_MDF; /* wait for TCH change */
    }
    else
    {
      if( cmhT30_Modify() NEQ AT_EXCT )
      {
        ccShrdPrm.datStat = DS_DSC_REQ;
        cmhT30_Deactivate();
      }
    }
  }
  return( AT_EXCT );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30S                     |
|                            ROUTINE : cmhT30_PPMRcvd               |
+-------------------------------------------------------------------+

  PURPOSE : post page message was received

*/
/* does not seem to be called */
#if 0
GLOBAL void cmhT30_PPMRcvd ( UBYTE ppm )
{
  TRACE_FUNCTION ("cmhT30_PPMRcvd()");

  /* determine type of post page message */
  switch( ppm )
  {
    case( SGN_MPS ):                    /* multi-page signal */
    case( SGN_EOP ):                    /* end of procedure */

      t30ShrdPrm.faxStat = FS_SND_DOC;  /* stay in send document */
      break;

    case( SGN_EOM ):

      /*--- no more doc available for polling if polled ---*/
      if( ccShrdPrm.ctb[t30ShrdPrm.cId].calType EQ CT_MTC )
      {
        fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FLPval = FLP_VAL_NoPollDoc;
      }

      t30ShrdPrm.faxStat = FS_DOC_TRF;  /* switch to document transfered */
      break;

    default:

      return;
  }

  /* send PPM to T.30 */
  t30ShrdPrm.sgn_snd = ppm;

  if( PRIRcvdFlg AND
      fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FIEval EQ FIE_VAL_AcceptPRI)

      t30ShrdPrm.sgn_snd |= PRI_MRK;

  if( pageSentFlg EQ TRUE )
  {
    if( psaT30_Ppm( ) < 0 )  /* T30 PPM request */
    {
      TRACE_EVENT( "FATAL RETURN psaT30 in cmhT30_PPMRcvd" );
    }
  }
  else

    ppmPendFlg = TRUE;
}
#endif /* 0 */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30S                     |
|                            ROUTINE : cmhT30_PRIRcvd               |
+-------------------------------------------------------------------+

  PURPOSE : procedure interrupt message was received

*/

GLOBAL void cmhT30_PRIRcvd ( void )
{
  TRACE_FUNCTION ("cmhT30_PRIRcvd()");

  PRIRcvdFlg = TRUE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_FITTimeout       |
+-------------------------------------------------------------------+

  PURPOSE : handle FAX inactivity timeout

*/

GLOBAL void cmhT30_FITTimeout ( void )
{
  TRACE_FUNCTION( "cmhT30_FITTimeout()" );

  if (!psaCC_ctbIsValid (t30ShrdPrm.cId))
  {
    TRACE_ERROR ("t30ShrdPrm.cId invalid");
    return;
  }

/*
 *-------------------------------------------------------------------
 * notify CC  to release the call
 *-------------------------------------------------------------------
 */
  t30ShrdPrm.faxStat = FS_IDL;
  t30EntStat.curCmd  = AT_CMD_NONE;

  cmhCC_T30_Failed();

  if( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FITact EQ FIT_ACT_OnHookRst )

    fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCLASSclass = FCLASS_CLASS_Data;
}
#endif /* DTI OR FF_FAX*/
/*==== EOF ========================================================*/
