/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_L2RS
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
|             protocol stack adapter for L2R.
+----------------------------------------------------------------------------- 
*/ 
#ifdef DTI

#ifndef CMH_L2RS_C
#define CMH_L2RS_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "psa.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "psa_l2r.h"
#include "cmh_l2r.h"
#include "psa_ra.h"

#include "psa_tra.h"

#ifdef UART
#include "psa_uart.h"
#endif
#ifdef FF_PSI
#include "psa_psi.h"
#endif /*FF_PSI*/
/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_L2RS                 |
| STATE   : code                  ROUTINE : sAT_PlusCRLP             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CRLP AT command
            which is responsible to set the radio link protocol
            parameters for following asynchronous data transfer calls.

            <iws>:   IWF to MS window size.
            <mws>:   MS to IWF window size
            <t1> :   acknowledge timer
            <n2> :   retransmission attemps
*/

GLOBAL T_ACI_RETURN sAT_PlusCRLP  ( T_ACI_CMD_SRC srcId,
                                    SHORT iws, SHORT mws,
                                    SHORT t1, SHORT n2)
{
  T_L2R_CMD_PRM * pL2RCmdPrm; /* points to L2R command parameters */

  TRACE_FUNCTION ("sAT_PlusCRLP()");

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

  pL2RCmdPrm = &fnd_cmhPrm[srcId].l2rCmdPrm;

/*
 *-------------------------------------------------------------------
 * process the iws parameter ( IWF to MS window size )
 *-------------------------------------------------------------------
 */
  if( iws NEQ ACI_NumParmNotPresent )
  {
    if( iws > L2R_K_IWF_MS_MAX OR iws < L2R_K_IWF_MS_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }
  else

    iws = pL2RCmdPrm -> CRLPiws;
/*
 *-------------------------------------------------------------------
 * process the mws parameter ( MS to IWF window size )
 *-------------------------------------------------------------------
 */
  if( mws NEQ ACI_NumParmNotPresent )
  {
    if( mws > L2R_K_MS_IWF_MAX OR mws < L2R_K_MS_IWF_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }
  else

    mws = pL2RCmdPrm -> CRLPmws;

/*
 *-------------------------------------------------------------------
 * process the t1 parameter ( acknowledge timer )
 *-------------------------------------------------------------------
 */
  if( t1 NEQ ACI_NumParmNotPresent )
  {
    if( t1 > L2R_T1_MAX OR t1 < L2R_T1_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }
  else

    t1 = pL2RCmdPrm -> CRLPt1;

/*
 *-------------------------------------------------------------------
 * process the n2 parameter ( retransmission attemps )
 *-------------------------------------------------------------------
 */
  if( n2 NEQ ACI_NumParmNotPresent )
  {
    if( n2 > L2R_N2_MAX OR n2 < L2R_N2_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }
  else

    n2 = pL2RCmdPrm -> CRLPn2;

/*
 *-------------------------------------------------------------------
 * assign the parameters
 *-------------------------------------------------------------------
 */
  pL2RCmdPrm -> CRLPiws = iws;
  pL2RCmdPrm -> CRLPmws = mws;
  pL2RCmdPrm -> CRLPt1  = t1;
  pL2RCmdPrm -> CRLPn2  = n2;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_L2RS                 |
| STATE   : code                  ROUTINE : sAT_PlusDS               |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +DS AT command
            which is responsible to set the data compression control
            parameters for following asynchronous data transfer calls.

            <dir>     :   desired direction of data compression.
            <comp>    :   mode of continue
            <maxDict> :   maximum number of dictionary entries
            <maxStr>  :   maximum string length
*/

GLOBAL T_ACI_RETURN sAT_PlusDS    ( T_ACI_CMD_SRC srcId,
                                    T_ACI_DS_DIR dir,
                                    T_ACI_DS_COMP comp,
                                    LONG maxDict,
                                    SHORT maxStr )
{
  T_L2R_CMD_PRM * pL2RCmdPrm; /* points to L2R command parameters */

  TRACE_FUNCTION ("sAT_PlusDS()");

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

  pL2RCmdPrm = &fnd_cmhPrm[srcId].l2rCmdPrm;

/*
 *-------------------------------------------------------------------
 * process the dir parameter
 *-------------------------------------------------------------------
 */
  switch( dir )
  {
    case( DS_DIR_NotPresent ):

      dir = pL2RCmdPrm -> DSdir;
      break;

    case( DS_DIR_Negotiated ):
    case( DS_DIR_TxOnly ):
    case( DS_DIR_RxOnly ):
    case( DS_DIR_Both ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the comp parameter
 *-------------------------------------------------------------------
 */
  switch( comp )
  {
    case( DS_COMP_NotPresent ):

      comp = pL2RCmdPrm -> DScomp;
      break;

    case( DS_COMP_DoNotDisc ):
    case( DS_COMP_Disc ):

      break;

    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process the maxDict parameter
 *-------------------------------------------------------------------
 */
  if( maxDict NEQ ACI_NumParmNotPresent )
  {
    if( maxDict > L2R_P1_MAX OR maxDict < L2R_P1_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }
  else

    maxDict = pL2RCmdPrm -> DSmaxDict;

/*
 *-------------------------------------------------------------------
 * process the maxStr parameter
 *-------------------------------------------------------------------
 */
  if( maxStr NEQ ACI_NumParmNotPresent )
  {
    if( maxStr > L2R_P2_MAX OR maxStr < L2R_P2_MIN )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }
  else

    maxStr = pL2RCmdPrm -> DSmaxStr;

/*
 *-------------------------------------------------------------------
 * assign the  parameters
 *-------------------------------------------------------------------
 */
  pL2RCmdPrm -> DSdir     = dir;
  pL2RCmdPrm -> DScomp    = comp;
  pL2RCmdPrm -> DSmaxDict = maxDict;
  pL2RCmdPrm -> DSmaxStr  = maxStr;

  return( AT_CMPL );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_L2R                 |
|                                 ROUTINE : cmhL2R_Activate         |
+-------------------------------------------------------------------+

  PURPOSE : set all nescessary parameters and activate L2R

*/

GLOBAL T_ACI_RETURN cmhL2R_Activate ( T_ACI_CMD_SRC srcId,
                                      T_ACI_AT_CMD cmdId,
                                      SHORT cId )
{
  T_L2R_CMD_PRM * pL2RCmdPrm;  /* points to L2R command parameters */
  T_L2R_SET_PRM * pL2RSetPrm;  /* points to L2R parameter set */

  TRACE_FUNCTION ("cmhL2R_Activate()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))

    return( AT_FAIL );

  pL2RCmdPrm = &fnd_cmhPrm[srcId].l2rCmdPrm;
  pL2RSetPrm = &l2rShrdPrm.set_prm[srcId];

/*
 *-------------------------------------------------------------------
 * fill in activate parameters
 *-------------------------------------------------------------------
 */
  pL2RSetPrm -> k_ms_iwf = pL2RCmdPrm -> CRLPmws;
  pL2RSetPrm -> k_iwf_ms = pL2RCmdPrm -> CRLPiws;
  pL2RSetPrm -> t1       = (UBYTE)pL2RCmdPrm -> CRLPt1;
  pL2RSetPrm -> n2       = (UBYTE)pL2RCmdPrm -> CRLPn2;

  pL2RSetPrm -> p0       = cmhL2R_SelCompDir( pL2RCmdPrm );
  pL2RSetPrm -> p1       = (USHORT)pL2RCmdPrm -> DSmaxDict;
  pL2RSetPrm -> p2       = (UBYTE)pL2RCmdPrm -> DSmaxStr;

  pL2RSetPrm -> uil2p    = L2R_ISO6429;
  pL2RSetPrm -> rate     = cmhL2R_SelChnRate();


  l2rEntStat.curCmd  = cmdId;
  l2rEntStat.entOwn  = srcId;

  if( psaL2R_Activate((UBYTE)srcId ) < 0 )  /* activate L2R */
  {
    TRACE_EVENT( "FATAL RETURN psaL2R in cmhL2R_Activate" );
    return( AT_FAIL );
  }

  return( AT_EXCT );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_L2R                 |
|                                 ROUTINE : cmhL2R_Deactivate       |
+-------------------------------------------------------------------+

  PURPOSE : set all nescessary parameters and deactivate L2R

*/

GLOBAL T_ACI_RETURN cmhL2R_Deactivate ( void )
{
  UBYTE src_id = raShrdPrm.owner;
  UBYTE dti_id;
  T_DTI_CNTRL    info;

  TRACE_FUNCTION ("cmhL2R_Deactivate()");


  if (dti_cntrl_get_info_from_src_id( src_id, &info) EQ FALSE)
  {
    TRACE_EVENT_P1("cannot find info for src_id=%d", src_id);
   
    if( psaL2R_Deactivate() < 0 )  /* deactivate L2R */
    {
      TRACE_EVENT( "FATAL RETURN psaL2R in cmhL2R_Deactivate" );
      return( AT_FAIL );
    }
    return( AT_EXCT );
  }

#if defined (FF_TCP_IP)
  if( dti_cntrl_is_dti_channel_connected(DTI_ENTITY_AAA, info.dti_id) EQ TRUE ) 
  {
    if ( dti_cntrl_close_dpath_from_src_id(src_id) EQ TRUE )
    {
      return( AT_EXCT );
    }
    else
    {
      TRACE_EVENT("FATAL RETURN in dti_cntrl_close_dpath_from_src_id");
      return( AT_FAIL );
    }
  }
#endif /* FF_TCP_IP */

  dti_id = info.dti_id;

  /* get the redirection of this src_id */
  if (info.redirect_info.info.direction EQ DTI_DEV_IS_REDIRECTED)
  {
    T_DTI_CNTRL tmp_param;
    if (BITFIELD_CHECK(info.redirect_info.info.capability, DTI_CPBLTY_SER))
    {
      dti_cntrl_get_first_redirection (info.src_id, DTI_CPBLTY_SER, &tmp_param);
      src_id = tmp_param.src_id;
      dti_id = tmp_param.dti_id;
    }
    else if (BITFIELD_CHECK(info.redirect_info.info.capability, DTI_CPBLTY_PKT))
    {
      dti_cntrl_get_first_redirection (info.src_id, DTI_CPBLTY_PKT, &tmp_param);
      src_id = tmp_param.src_id;
      dti_id = tmp_param.dti_id;
    }
    else
    {
      TRACE_EVENT("capability is not SER or PKT");
    }
  }
  if( dti_cntrl_is_dti_channel_connected(DTI_ENTITY_L2R, dti_id) EQ TRUE AND 
     dti_cntrl_is_dti_channel_connected(DTI_ENTITY_UART, dti_id) EQ TRUE)
  { /*no internal dti_id */
    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};
    if (dti_cntrl_est_dpath_indirect ( src_id,
                                       entity_list,
                                       1,
                                       SPLIT,
                                       atiUART_dti_cb,
                                       DTI_CPBLTY_CMD,
                                       DTI_CID_NOTPRESENT) EQ TRUE)
    {
      return( AT_EXCT );
    }
    else
    {
      /* deactivate */
      if( psaL2R_Deactivate() < 0 )  /* deactivate L2R */
      {
        TRACE_EVENT( "FATAL RETURN psaL2R in cmhL2R_Deactivate" );
        return( AT_FAIL );
      }
    }
  }
#ifdef FF_PSI
 if (dti_cntrl_is_dti_channel_connected(DTI_ENTITY_L2R, dti_id) AND 
     dti_cntrl_is_dti_channel_connected(DTI_ENTITY_PSI, dti_id))
  {
     T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};
    if (dti_cntrl_est_dpath_indirect ( src_id,
                                       entity_list,
                                       1,
                                       SPLIT,
                                       atiPSI_dti_cb,
                                       DTI_CPBLTY_CMD,
                                       DTI_CID_NOTPRESENT) EQ TRUE)

    {
      return( AT_EXCT );
    }
    else
   {
  /* deactivate */
  if( psaL2R_Deactivate() < 0 )  /* deactivate L2R */
  {
     TRACE_EVENT( "FATAL RETURN psaL2R in cmhL2R_Deactivate" );
     return( AT_FAIL );
  }
    }
  }
#endif /*FF_PSI*/

  else
  {
    if( psaL2R_Deactivate() < 0 )  /* deactivate L2R */
    {
      TRACE_EVENT( "FATAL RETURN psaL2R in cmhL2R_Deactivate" );
      return( AT_FAIL );
    }
  }

  return( AT_EXCT );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_L2R                 |
|                                 ROUTINE : cmhTRA_Deactivate       |
+-------------------------------------------------------------------+

  PURPOSE : set all nescessary parameters and deactivate TRA

*/

GLOBAL T_ACI_RETURN cmhTRA_Deactivate ( void )
{
  UBYTE src_id = raShrdPrm.owner;
  UBYTE dti_id;
  T_DTI_CNTRL    info;

  TRACE_FUNCTION ("cmhTRA_Deactivate()");


  if (dti_cntrl_get_info_from_src_id( src_id, &info) EQ FALSE)
  {
    TRACE_EVENT_P1("cannot find info for src_id=%d", src_id);

    psaTRA_Deactivate();  /* deactivate TRA */
    return( AT_EXCT );
  }

  dti_id = info.dti_id;

  /* get the redirection of this src_id */
  if (info.redirect_info.info.direction EQ DTI_DEV_IS_REDIRECTED)
  {
    T_DTI_CNTRL tmp_param;
    if (BITFIELD_CHECK(info.redirect_info.info.capability, DTI_CPBLTY_SER))
    {
      dti_cntrl_get_first_redirection (info.src_id, DTI_CPBLTY_SER, &tmp_param);
      src_id = tmp_param.src_id;
      dti_id = tmp_param.dti_id;
    }
    else if (BITFIELD_CHECK(info.redirect_info.info.capability, DTI_CPBLTY_PKT))
    {
      dti_cntrl_get_first_redirection (info.src_id, DTI_CPBLTY_PKT, &tmp_param);
      src_id = tmp_param.src_id;
      dti_id = tmp_param.dti_id;
    }
    else
    {
      TRACE_EVENT("capability is not SER or PKT");
    }
  }
  if (dti_cntrl_is_dti_channel_connected(DTI_ENTITY_TRA, dti_id) AND 
     dti_cntrl_is_dti_channel_connected(DTI_ENTITY_UART, dti_id))
  {
    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};
    if (dti_cntrl_est_dpath_indirect ( src_id,
                                       entity_list,
                                       1,
                                       SPLIT,
                                       atiUART_dti_cb,
                                       DTI_CPBLTY_CMD,
                                       DTI_CID_NOTPRESENT) EQ TRUE)
    {
      return( AT_EXCT );
    }
    else
    {
      /* deactivate */
      psaTRA_Deactivate();  /* deactivate TRA */
    }
  }

#ifdef FF_PSI

  if (dti_cntrl_is_dti_channel_connected(DTI_ENTITY_TRA, dti_id) AND 
     dti_cntrl_is_dti_channel_connected(DTI_ENTITY_PSI, dti_id))
  {
     T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};
    if (dti_cntrl_est_dpath_indirect ( src_id,
                                       entity_list,
                                       1,
                                       SPLIT,
                                       atiPSI_dti_cb,
                                       DTI_CPBLTY_CMD,
                                       DTI_CID_NOTPRESENT) EQ TRUE)

    {
      return( AT_EXCT );
    }
    else
    {
      /* deactivate */
      psaTRA_Deactivate();  /* deactivate TRA */
    }
  }
#endif /*FF_PSI*/

  else
  {
    psaTRA_Deactivate();  /* deactivate TRA */
  }

  return( AT_EXCT );
}
#endif /* DTI */
/*==== EOF ========================================================*/
