/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_T30R
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
|  Purpose :  This module defines the functions which are responsible
|             for the responses of the protocol stack adapter for
|             T 30.
+----------------------------------------------------------------------------- 
*/ 

#if defined (DTI) || defined (FF_FAX)

#ifndef CMH_T30R_C
#define CMH_T30R_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_io.h"

#include "aci_lst.h"

#ifdef FAX_AND_DATA
  #include "aci_fd.h"
#endif

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci.h"
#include "psa.h"
#include "psa_t30.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_t30.h"
#include "cmh_cc.h"

#include "p_gsmcom.val" /* error cause originating entities */
#include "p_fad.val"    /* causes values */

#include "cmh_ra.h"

#ifdef UART
#include "psa_uart.h"
#include "cmh_uart.h"
#endif
#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#endif /*FF_PSI*/
/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

#ifdef _SIMULATE_FAX_DATA_
EXTERN void TST_rstSimData( void );
#endif /*_SIMULATE_FAX_DATA_*/

/*==== VARIABLES ==================================================*/

LOCAL BOOL HDLCfst = TRUE;       /* flags first occurence of HDLC frame */

/*==== FUNCTIONS ==================================================*/
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_Activated             |
+-------------------------------------------------------------------+

  PURPOSE : T30 entity activated, connection established

*/

GLOBAL void cmhT30_Activated ( void )
{
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI  *src_infos = find_element (psi_src_params, 
                                 (UBYTE)t30EntStat.entOwn, cmhPSItest_srcId);
#endif /*FF_PSI*/
  TRACE_FUNCTION ("cmhT30_Activated()");

  io_setDCD (t30EntStat.entOwn, IO_DCD_ON);
#ifdef FF_PSI
  if (src_infos NEQ NULL)
    psaPSI_ESCAPEreq( (UBYTE)t30EntStat.entOwn, ESC_DETECTION_OFF);
  else 
#endif /*FF_PSI*/

#ifdef UART    
    psaUART_ESCAPEreq( (UBYTE)t30EntStat.entOwn, UART_ESC_DETECTION_OFF );
#endif
  /* inform call control about successful operation */

#ifdef _SIMULATE_FAX_DATA_
  TST_rstSimData();
#endif /*_SIMULATE_FAX_DATA_*/

  psaT30_Config( );
  
  cmhCC_T30_Activated();

  t30ShrdPrm.faxStat = FS_IDL;
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_CapRmtSite            |
+-------------------------------------------------------------------+

  PURPOSE : Capabilities recieved from remote site.

*/

GLOBAL void cmhT30_CapRmtSite ( void )
{
  UBYTE strBuf[CSI_LEN];       /* string buffer */
  UBYTE cmdBuf;                /* buffers current command */

  TRACE_FUNCTION ("cmhT30_CapRmtSite()");
/*
 *-------------------------------------------------------------------
 * determine call type
 *-------------------------------------------------------------------
 */
  switch( psaCC_ctb(t30ShrdPrm.cId)->calType )
  {
    case( CT_MOC ):
    /*
     *---------------------------------------------------------------
     * check for DCS frame
     *---------------------------------------------------------------
     */
      if( t30ShrdPrm.hdlc_rcv.v_dcs EQ TRUE )
      {
        /*--- send non-standard set-up information ---*/
        if( t30ShrdPrm.hdlc_rcv.c_nss NEQ 0 )
        {
          R_AT( RAT_FNS, t30EntStat.entOwn )
            ( t30ShrdPrm.hdlc_rcv.c_nss, t30ShrdPrm.hdlc_rcv.nss );
        }
        /*--- send password information ---*/
        if( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FAPpwd EQ
            FAP_VAL_Enabled AND
            t30ShrdPrm.hdlc_rcv.c_pwd NEQ 0 )
        {
          memcpy
          (
            (char *) strBuf,
            (char *) t30ShrdPrm.hdlc_rcv.pwd,
            t30ShrdPrm.hdlc_rcv.c_pwd
          );

          strBuf[t30ShrdPrm.hdlc_rcv.c_pwd] = '\0';

          R_AT( RAT_FPW, t30EntStat.entOwn )
            ( strBuf );
        }

        /*--- send subaddress information ---*/
        if( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FAPsub EQ
            FAP_VAL_Enabled AND
            t30ShrdPrm.hdlc_rcv.c_sub NEQ 0 )
        {
          memcpy
          (
            (char *) strBuf,
            (char *) t30ShrdPrm.hdlc_rcv.sub,
            t30ShrdPrm.hdlc_rcv.c_sub
          );

          strBuf[t30ShrdPrm.hdlc_rcv.c_sub] = '\0';

          R_AT( RAT_FSA, t30EntStat.entOwn )
            ( strBuf );
        }

        /*--- send transmitting subscriber information ---*/
        if( t30ShrdPrm.hdlc_rcv.c_tsi NEQ 0 )
        {
          memcpy
          (
            (char *) strBuf,
            (char *) t30ShrdPrm.hdlc_rcv.tsi,
            t30ShrdPrm.hdlc_rcv.c_tsi
          );

          strBuf[t30ShrdPrm.hdlc_rcv.c_tsi] = '\0';

          R_AT( RAT_FTI, t30EntStat.entOwn )
            ( strBuf );
        }

        /*--- send DCS frame information ---*/
        R_AT( RAT_FCS, t30EntStat.entOwn )
          ( t30NgtPrms.FCSvr = cmhT30_GetResolution
                                      ( &t30ShrdPrm.hdlc_rcv.dcs, FRT_DCS),
            t30NgtPrms.FCSbr = cmhT30_GetBitRate
                                      ( &t30ShrdPrm.hdlc_rcv.dcs, FRT_DCS),
            t30NgtPrms.FCSwd = cmhT30_GetPageWidth
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSln = cmhT30_GetPageLength
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSdf = cmhT30_GetDataComp
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSec = cmhT30_GetErrCorr
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSbf = cmhT30_GetFileTrnsfr
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSst = cmhT30_GetScanTime
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSjp = cmhT30_GetJPEG
                                      ( &t30ShrdPrm.hdlc_rcv.dcs) );

        /*--- check for T30 modification ---*/
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

    /*
     *---------------------------------------------------------------
     * check for DIS frame
     *---------------------------------------------------------------
     */
      if( t30ShrdPrm.hdlc_rcv.v_dis EQ TRUE )
      {
        /*--- a DIS frame which is in context with the
              FDT command, forces a DCS retransmission and quit ---*/
        if( t30EntStat.curCmd EQ AT_CMD_FDT )
        {
          cmhT30_SendCaps ( t30EntStat.entOwn, FRT_DCS );
          break;
        }

        /*--- send non-standard facility information ---*/
        if( t30ShrdPrm.hdlc_rcv.c_nsf NEQ 0 )
        {
          R_AT( RAT_FNF, t30EntStat.entOwn )
            ( t30ShrdPrm.hdlc_rcv.c_nsf, t30ShrdPrm.hdlc_rcv.nsf );
        }

        /*--- send calling subscriber information ---*/
        if( t30ShrdPrm.hdlc_rcv.c_csi NEQ 0 )
        {
          memcpy
          (
            (char *) strBuf,
            (char *) t30ShrdPrm.hdlc_rcv.csi,
            t30ShrdPrm.hdlc_rcv.c_csi
          );

          strBuf[t30ShrdPrm.hdlc_rcv.c_csi] = '\0';

          R_AT( RAT_FCI, t30EntStat.entOwn )
            ( strBuf );
        }

        /*--- send DIS frame information ---*/
        R_AT( RAT_FIS, t30EntStat.entOwn )
          ( cmhT30_GetResolution( &t30ShrdPrm.hdlc_rcv.dis, FRT_DIS ),
            cmhT30_GetBitRate   ( &t30ShrdPrm.hdlc_rcv.dis, FRT_DIS ),
            cmhT30_GetPageWidth ( &t30ShrdPrm.hdlc_rcv.dis),
            cmhT30_GetPageLength( &t30ShrdPrm.hdlc_rcv.dis),
            cmhT30_GetDataComp  ( &t30ShrdPrm.hdlc_rcv.dis),
            cmhT30_GetErrCorr   ( &t30ShrdPrm.hdlc_rcv.dis),
            cmhT30_GetFileTrnsfr( &t30ShrdPrm.hdlc_rcv.dis),
            cmhT30_GetScanTime  ( &t30ShrdPrm.hdlc_rcv.dis),
            cmhT30_GetJPEG      ( &t30ShrdPrm.hdlc_rcv.dis)  );

        /*--- send polling capability information ---*/
        if( t30ShrdPrm.hdlc_rcv.dis.ready_tx_fax       EQ 1      AND
            fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FSPval EQ
                                              FSP_VAL_PollEnable AND
            fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCRval EQ
                                              FCR_VAL_RcvCap )
        {
          R_AT( RAT_FPO, t30EntStat.entOwn )();
        }

        /*--- release command state ---*/
        switch( t30EntStat.curCmd )
        {
          case( AT_CMD_D ):

            t30EntStat.curCmd = AT_CMD_NONE;
            break;
        }
      }
      break;

    case( CT_MTC ):

    /*
     *---------------------------------------------------------------
     * check for DCS frame
     *---------------------------------------------------------------
     */
      if( t30ShrdPrm.hdlc_rcv.v_dcs EQ TRUE )
      {
        /*--- send non-standard set-up information ---*/
        if( t30ShrdPrm.hdlc_rcv.c_nss NEQ 0 )
        {
          R_AT( RAT_FNS, t30EntStat.entOwn )
            ( t30ShrdPrm.hdlc_rcv.c_nss, t30ShrdPrm.hdlc_rcv.nss );
        }

        /*--- send password information ---*/
        if( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FAPpwd EQ
            FAP_VAL_Enabled AND
            t30ShrdPrm.hdlc_rcv.c_pwd NEQ 0 )
        {
          memcpy
          (
            (char *) strBuf,
            (char *) t30ShrdPrm.hdlc_rcv.pwd,
            t30ShrdPrm.hdlc_rcv.c_pwd
          );

          strBuf[t30ShrdPrm.hdlc_rcv.c_pwd] = '\0';

          R_AT( RAT_FPW, t30EntStat.entOwn )
            ( strBuf );
        }

        /*--- send subaddress information ---*/
        if( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FAPsub EQ
            FAP_VAL_Enabled AND
            t30ShrdPrm.hdlc_rcv.c_sub NEQ 0 )
        {
          memcpy
          (
            (char *) strBuf,
            (char *) t30ShrdPrm.hdlc_rcv.sub,
            t30ShrdPrm.hdlc_rcv.c_sub
          );

          strBuf[t30ShrdPrm.hdlc_rcv.c_sub] = '\0';

          R_AT( RAT_FSA, t30EntStat.entOwn )
            ( strBuf );
        }

        /*--- send transmitting subscriber information ---*/
        if( t30ShrdPrm.hdlc_rcv.c_tsi NEQ 0 )
        {
          memcpy
          (
            (char *) strBuf,
            (char *) t30ShrdPrm.hdlc_rcv.tsi,
            t30ShrdPrm.hdlc_rcv.c_tsi
          );

          strBuf[t30ShrdPrm.hdlc_rcv.c_tsi] = '\0';

          R_AT( RAT_FTI, t30EntStat.entOwn )
            ( strBuf );
        }

        /*--- send DCS frame information ---*/
        R_AT( RAT_FCS, t30EntStat.entOwn )
          ( t30NgtPrms.FCSvr = cmhT30_GetResolution
                                      ( &t30ShrdPrm.hdlc_rcv.dcs, FRT_DCS),
            t30NgtPrms.FCSbr = cmhT30_GetBitRate
                                      ( &t30ShrdPrm.hdlc_rcv.dcs, FRT_DCS),
            t30NgtPrms.FCSwd = cmhT30_GetPageWidth
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSln = cmhT30_GetPageLength
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSdf = cmhT30_GetDataComp
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSec = cmhT30_GetErrCorr
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSbf = cmhT30_GetFileTrnsfr
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSst = cmhT30_GetScanTime
                                      ( &t30ShrdPrm.hdlc_rcv.dcs),
            t30NgtPrms.FCSjp = cmhT30_GetJPEG
                                      ( &t30ShrdPrm.hdlc_rcv.dcs) );

        /*--- check for T30 modification ---*/
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

    /*
     *---------------------------------------------------------------
     * check for DTC frame
     *---------------------------------------------------------------
     */
      else if( t30ShrdPrm.hdlc_rcv.v_dtc EQ TRUE )
      {
        /*--- DTC received but no polling document available ---*/
        if( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FLPval EQ
            FLP_VAL_NoPollDoc )
        {
          R_AT( RAT_FHS, t30EntStat.entOwn )
            ( FHSstat = FHS_STAT_InvComrecCmd );

          cmdBuf = t30EntStat.curCmd;
          t30EntStat.curCmd = AT_CMD_NONE;

          R_AT( RAT_OK, t30EntStat.entOwn )( cmdBuf );

          cmhCC_T30_Failed();
          return;
        }

        /*--- send non-standard facilities command ---*/
        if( t30ShrdPrm.hdlc_rcv.c_nsc NEQ 0 )
        {
          R_AT( RAT_FNC, t30EntStat.entOwn )
            ( t30ShrdPrm.hdlc_rcv.c_nsc, t30ShrdPrm.hdlc_rcv.nsc );
        }

        /*--- send password information ---*/
        if( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FAPpwd EQ
            FAP_VAL_Enabled AND
            t30ShrdPrm.hdlc_rcv.c_pwd NEQ 0)
        {
          memcpy
          (
            (char *) strBuf,
            (char *) t30ShrdPrm.hdlc_rcv.pwd,
            t30ShrdPrm.hdlc_rcv.c_pwd
          );

          strBuf[t30ShrdPrm.hdlc_rcv.c_pwd] = '\0';

          R_AT( RAT_FPW, t30EntStat.entOwn )
            ( strBuf );
        }

        /*--- send selective polling information ---*/
        if( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FAPsep EQ
            FAP_VAL_Enabled AND
            t30ShrdPrm.hdlc_rcv.c_sep NEQ 0)
        {
          memcpy
          (
            (char *) strBuf,
            (char *) t30ShrdPrm.hdlc_rcv.sep,
            t30ShrdPrm.hdlc_rcv.c_sep
          );

          strBuf[t30ShrdPrm.hdlc_rcv.c_sep] = '\0';

          R_AT( RAT_FPA, t30EntStat.entOwn )
            ( strBuf );
        }

        /*--- send calling subscriber information ---*/
        if( t30ShrdPrm.hdlc_rcv.c_cig NEQ 0 )
        {
          memcpy
          (
            (char *) strBuf,
            (char *) t30ShrdPrm.hdlc_rcv.cig,
            t30ShrdPrm.hdlc_rcv.c_cig
          );

          strBuf[t30ShrdPrm.hdlc_rcv.c_cig] = '\0';

          R_AT( RAT_FPI, t30EntStat.entOwn )
            ( strBuf );
        }

        /*--- send DTC frame information ---*/
        R_AT( RAT_FTC, t30EntStat.entOwn )
          ( cmhT30_GetResolution( &t30ShrdPrm.hdlc_rcv.dtc, FRT_DTC),
            cmhT30_GetBitRate   ( &t30ShrdPrm.hdlc_rcv.dtc, FRT_DTC),
            cmhT30_GetPageWidth ( &t30ShrdPrm.hdlc_rcv.dtc),
            cmhT30_GetPageLength( &t30ShrdPrm.hdlc_rcv.dtc),
            cmhT30_GetDataComp  ( &t30ShrdPrm.hdlc_rcv.dtc),
            cmhT30_GetErrCorr   ( &t30ShrdPrm.hdlc_rcv.dtc),
            cmhT30_GetFileTrnsfr( &t30ShrdPrm.hdlc_rcv.dtc),
            cmhT30_GetScanTime  ( &t30ShrdPrm.hdlc_rcv.dtc),
            cmhT30_GetJPEG      ( &t30ShrdPrm.hdlc_rcv.dtc) );
      }

      /*--- release command state ---*/
      switch( t30EntStat.curCmd )
      {
        case( AT_CMD_A ):

          t30EntStat.curCmd = AT_CMD_NONE;
          break;

        case( AT_CMD_FDR ):

          cmhT30_StartFIT();
          t30EntStat.curCmd = AT_CMD_NONE;
          R_AT( RAT_OK, t30EntStat.entOwn )( AT_CMD_FDR );
          return;
      }
      break;
  }

/*
 *-------------------------------------------------------------------
 * inform call control about received capability parameter
 *-------------------------------------------------------------------
 */
  if( t30ShrdPrm.faxStat EQ FS_IDL )
  {
    cmhCC_T30_RmtCaps();
  }
  return;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhL2R_Deactivated           |
+-------------------------------------------------------------------+

  PURPOSE : T30 entity deactivated, connection disconnected

*/

GLOBAL void cmhT30_Deactivated ( void )
{
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI  *src_infos = find_element (psi_src_params, 
                                 (UBYTE)t30EntStat.entOwn, cmhPSItest_srcId);
#endif /*FF_PSI*/
  TRACE_FUNCTION ("cmhT30_Deactivated()");
  io_setDCD (t30EntStat.entOwn, IO_DCD_OFF);
#ifdef FF_PSI
  if (src_infos NEQ NULL)
    psaPSI_ESCAPEreq( (UBYTE)t30EntStat.entOwn, ESC_DETECTION_ON);
  else 
#endif /*FF_PSI*/
#ifdef UART
    psaUART_ESCAPEreq( (UBYTE)t30EntStat.entOwn, UART_ESC_DETECTION_ON );
#endif
  /* inform call control about disconnection */
  HDLCfst = TRUE;

  cmhCC_T30_Deactivated();

  t30ShrdPrm.faxStat  = NO_VLD_FS;
  t30ShrdPrm.cId      = NO_ENTRY;
  t30EntStat.curCmd   = AT_CMD_NONE;

  pageSentFlg       = TRUE;
  DTCSentFlg        = FALSE;
  PRIRcvdFlg        = FALSE;

  cmhT30_StopFIT();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_Disconnect            |
+-------------------------------------------------------------------+

  PURPOSE : request to disconnect

*/

GLOBAL void cmhT30_Disconnect ( void )
{

  TRACE_FUNCTION ("cmhT30_Disconnect()");

  /* clear FAX call */
  t30EntStat.curCmd = AT_CMD_NONE;

  R_AT( RAT_FHS, t30EntStat.entOwn )( FHSstat = FHS_STAT_NormEnd );

  /* pretends an orderly FAX abort */
  psaCC_ctb(t30ShrdPrm.cId)->curCmd = AT_CMD_FKS;
  psaCC_ctb(t30ShrdPrm.cId)->curSrc = t30EntStat.entOwn;

  cmhT30_Deactivate();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_Disconnected          |
+-------------------------------------------------------------------+

  PURPOSE : T30 entity deactivated

*/

GLOBAL void cmhT30_Disconnected ( void )
{
  TRACE_FUNCTION ("cmhT30_Disconnected()");

  /* clear FAX call */
  switch( t30EntStat.curCmd )
  {
    case( AT_CMD_FKS ):

      t30EntStat.curCmd = AT_CMD_NONE;
      break;
  }

  R_AT( RAT_FHS, t30EntStat.entOwn )( FHSstat = FHS_STAT_NormEnd );

  psaCC_ctb(t30ShrdPrm.cId)->curCmd = AT_CMD_FKS;
  psaCC_ctb(t30ShrdPrm.cId)->curSrc = t30EntStat.entOwn;

  cmhT30_Deactivate();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_Failure               |
+-------------------------------------------------------------------+

  PURPOSE : T30 entity failure, connection will be disconnected

*/

GLOBAL void cmhT30_Failure ( void )
{
  TRACE_FUNCTION ("cmhT30_Failure()");

  /*
   *-------------------------------------------------------------------
   * general failure indication
   *-------------------------------------------------------------------
   */
  TRACE_EVENT_P1("T30/FAD ERROR: 0x%04x", t30ShrdPrm.err_cause);

  switch (GET_CAUSE_ORIGIN_ENTITY(t30ShrdPrm.err_cause))
  {
    case (FAD_ORIGINATING_ENTITY):   /* FAD error cause */

      if ( ERR_REM_FAD < t30ShrdPrm.err_cause AND t30ShrdPrm.err_cause < ERR_LOCAL_FAD)
      {
        FHSstat = FHS_STAT_FADRmtStnErr;
      }
      else if (ERR_LOCAL_FAD < t30ShrdPrm.err_cause AND t30ShrdPrm.err_cause < ERR_OWN_FAD)
      {
        switch (t30ShrdPrm.err_cause)
        {
        case ERR_LOCAL_DATA_MSG_UFL:
          FHSstat = FHS_STAT_DteDceDataUndrflw;
          break;
        default:
          FHSstat = FHS_STAT_FADLclStnErr;
          break;
        }
      }
      else if (ERR_OWN_FAD < t30ShrdPrm.err_cause AND t30ShrdPrm.err_cause < ERR_GENERAL_FAD)
      {
        FHSstat = FHS_STAT_FADOwnErr;
      }
      else
        FHSstat = FHS_STAT_FADGnrlErr;
      break;

    case (T30_ORIGINATING_ENTITY):   /* T30 error cause */

      FHSstat = (T_ACI_FHS_STAT)GET_CAUSE_VALUE(t30ShrdPrm.err_cause);
      break;
  }

/*
 *-------------------------------------------------------------------
 * indicate a hang-up status code if a FAX transaction is active and
 * it is in context with the +FDR or +FDT command
 *-------------------------------------------------------------------
 */
  if( ccShrdPrm.datStat EQ DS_ACT)
/*
    AND
     (t30EntStat.curCmd EQ AT_CMD_FDR OR
      t30EntStat.curCmd EQ AT_CMD_FDT   ))
*/  {
    TRACE_EVENT ("STOP FAX DATA PHASE");

    R_AT( RAT_FHS, t30EntStat.entOwn )( FHSstat );

    cmhCC_flagCall( t30ShrdPrm.cId, &((&cmhPrm[t30EntStat.entOwn].ccCmdPrm) -> mltyDscFlg));

    /* pretend an orderly abort */
    psaCC_ctb(t30ShrdPrm.cId)->curCmd = t30EntStat.curCmd;
    psaCC_ctb(t30ShrdPrm.cId)->curSrc = t30EntStat.entOwn;
  }
/*
 *-------------------------------------------------------------------
 * notify CC about the failure to release the call
 *-------------------------------------------------------------------
 */
  t30ShrdPrm.faxStat = FS_IDL;
  t30EntStat.curCmd  = AT_CMD_NONE;

  cmhCC_T30_Failed();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_HDLCRpt               |
+-------------------------------------------------------------------+

  PURPOSE : HDLC reporting

*/

GLOBAL void cmhT30_HDLCRpt ( void )
{
  TRACE_FUNCTION ("cmhT30_HDLCRpt()");

  /* HDLC report indication */
  if( t30ShrdPrm.report.l_buf NEQ 0 )
  {
    switch( t30ShrdPrm.report.dir )
    {
      case( DIR_SND ):

        R_AT( RAT_FHT, t30EntStat.entOwn )
          ((t30ShrdPrm.report.l_buf>>3), &t30ShrdPrm.report.buf );
        break;

      case( DIR_RCV ):

        R_AT( RAT_FHR, t30EntStat.entOwn )
          ((t30ShrdPrm.report.l_buf>>3), &t30ShrdPrm.report.buf );
        break;
    }
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_PreambleRcvd          |
+-------------------------------------------------------------------+

  PURPOSE : Preamble received

*/

GLOBAL void cmhT30_PreambleRcvd ( void )
{
  TRACE_FUNCTION ("cmhT30_PreambleRcvd()");

  /* indicate first received preamble */
  if( HDLCfst EQ TRUE )
  {
    R_AT( RAT_FCO, t30EntStat.entOwn )();
    HDLCfst = FALSE;
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_TransCmpl             |
+-------------------------------------------------------------------+

  PURPOSE : transmission completed

*/

GLOBAL void cmhT30_TransCmpl ( void )
{
  TRACE_FUNCTION ("cmhT30_TransCmpl()");

  /* terminate FAX call */
  t30ShrdPrm.faxStat = FS_IDL;
  DTCSentFlg = TRUE;

  psaCC_ctb(t30ShrdPrm.cId)->curCmd = t30EntStat.curCmd;
  psaCC_ctb(t30ShrdPrm.cId)->curSrc = t30EntStat.entOwn;
  t30EntStat.curCmd = AT_CMD_NONE;

  TRACE_EVENT ("STOP FAX DATA PHASE");

  R_AT( RAT_FHS, t30EntStat.entOwn )( FHSstat = FHS_STAT_NormEnd );

  cmhCC_flagCall( t30ShrdPrm.cId, &((&cmhPrm[t30EntStat.entOwn].ccCmdPrm) -> mltyDscFlg));

  ccShrdPrm.datStat = DS_DSC_REQ;
  cmhT30_Deactivate  ();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_NextPage              |
+-------------------------------------------------------------------+

  PURPOSE : next page requested

*/

GLOBAL void cmhT30_NextPage ( void )
{
  UBYTE cmdBuf;                /* buffers current command */

  TRACE_FUNCTION ("cmhT30_NextPage()");

  /* terminate command and store result to FPS( FAX send ) */
  if (ccShrdPrm.ctb[t30ShrdPrm.cId] NEQ NULL AND
      psaCC_ctb(t30ShrdPrm.cId)->calType EQ CT_MOC )
  {
    fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FPSppr = FPS_PPR_Mcf;

    cmdBuf = t30EntStat.curCmd;
    t30EntStat.curCmd = AT_CMD_NONE;

    TRACE_EVENT ("STOP FAX DATA PHASE");

    cmhT30_StartFIT();

    R_AT( RAT_OK, t30EntStat.entOwn )( cmdBuf );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_NextDoc               |
+-------------------------------------------------------------------+

  PURPOSE : next document requested

*/

GLOBAL void cmhT30_NextDoc ( void )
{
  UBYTE cmdBuf;                /* buffers current command */

  TRACE_FUNCTION ("cmhT30_NextDoc()");

  /* send capabilities for next document ( FAX receive ) */
  if (psaCC_ctb(t30ShrdPrm.cId)->calType EQ CT_MTC)
  {
    cmhT30_SendCaps ( t30EntStat.entOwn, FRT_DIS );
  }

  /* terminate command and store result to FPS( FAX send ) */
  else if (psaCC_ctb(t30ShrdPrm.cId)->calType EQ CT_MOC )
  {
    fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FPSppr = FPS_PPR_Mcf;

    cmdBuf = t30EntStat.curCmd;
    t30EntStat.curCmd = AT_CMD_NONE;

    TRACE_EVENT ("STOP FAX DATA PHASE");
    cmhT30_StartFIT();

    R_AT( RAT_OK, t30EntStat.entOwn )( cmdBuf );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_RTN                   |
+-------------------------------------------------------------------+

  PURPOSE: Retrain Negative (RTN)
  To indicate that the previous message has not been satisfactorily received.
  However, further receptions may be possible, 
  provided training is retransmitted.
*/

GLOBAL void cmhT30_RTN ( void )
{
  TRACE_FUNCTION ("cmhT30_RTN()");

  psaT30_Capabilities();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_RTP                   |
+-------------------------------------------------------------------+

  PURPOSE: Retrain Positive (RTP)
  To indicate that a complete message has been received 
  and that additional messages may follow
  after retransmission of training and CFR.
*/

GLOBAL void cmhT30_RTP ( void )
{
  TRACE_FUNCTION ("cmhT30_RTP()");

  psaT30_Capabilities();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_FTT                   |
+-------------------------------------------------------------------+

  PURPOSE : failure to train

*/

GLOBAL void cmhT30_FTT ( void )
{
  TRACE_FUNCTION ("cmhT30_FTT()");

  /* re-negotiate bit rate and send new DCS */
  
  /* check for minimum speed condition */
  if( t30NgtPrms.FCSbr    EQ F_BR_2400 OR
      t30NgtPrms.FCSbr -1 <  fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FMSbr )
  {
    t30ShrdPrm.faxStat = FS_IDL;
    t30EntStat.curCmd  = AT_CMD_NONE;
    /* pretend an orderly abort */
    psaCC_ctb(t30ShrdPrm.cId)->curCmd = AT_CMD_FKS;
    psaCC_ctb(t30ShrdPrm.cId)->curSrc = t30EntStat.entOwn;
    ccShrdPrm.datStat  = DS_DSC_REQ;
    cmhT30_Deactivate ();

    R_AT( RAT_FHS, t30EntStat.entOwn )( FHSstat = FHS_STAT_TrainFail );
  }
  else
  {
    t30NgtPrms.FCSbr--;
    cmhT30_SendCaps ( t30EntStat.entOwn, FRT_DCS );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_PageReceived          |
+-------------------------------------------------------------------+

  PURPOSE : FAX page was received

*/

GLOBAL void cmhT30_PageReceived ( void )
{
  TRACE_FUNCTION ("cmhT30_PageReceived()");

  /* indicate page status */
  t30ShrdPrm.faxStat = FS_RCV_DOC;
  t30EntStat.curCmd = AT_CMD_NONE;

  TRACE_EVENT ("STOP FAX DATA PHASE");
  cmhT30_StartFIT();

 /* FCQ controls output of FPS */

  if(fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCQrq EQ FCQ_RQ_CQCDisabled)
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( FPS_PPR_Mcf,t30ShrdPrm.eol,0,0,0 );
  }
  else
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FPSppr,t30ShrdPrm.eol,0,0,0 );
  }

  R_AT( RAT_FET, t30EntStat.entOwn )( FET_PPM_Mps );
  R_AT( RAT_OK, t30EntStat.entOwn ) ( AT_CMD_FDR );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_DocReceived           |
+-------------------------------------------------------------------+

  PURPOSE : FAX document was received

*/

GLOBAL void cmhT30_DocReceived ( void )
{
  TRACE_FUNCTION ("cmhT30_DocReceived()");

  /* reset polling mode if received doc was polled */
  if (psaCC_ctb(t30ShrdPrm.cId)->calType EQ CT_MOC)
  {
    fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FSPval = FSP_VAL_PollDisabled;
  }

  /* indicate page status */
  t30ShrdPrm.faxStat = FS_DOC_TRF;
  t30EntStat.curCmd = AT_CMD_NONE;

  TRACE_EVENT ("STOP FAX DATA PHASE");
  cmhT30_StartFIT();

 /* FCQ controls output of FPS */ 
  if(fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCQrq EQ FCQ_RQ_CQCDisabled)
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( FPS_PPR_Mcf,t30ShrdPrm.eol,0,0,0 );
  }
  else
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FPSppr,t30ShrdPrm.eol,0,0,0 );
  }

  R_AT( RAT_FET, t30EntStat.entOwn )( FET_PPM_Eom );
  R_AT( RAT_OK, t30EntStat.entOwn ) ( AT_CMD_FDR );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_ProcEnd               |
+-------------------------------------------------------------------+

  PURPOSE : FAX document was received

*/

GLOBAL void cmhT30_ProcEnd ( void )
{
  TRACE_FUNCTION ("cmhT30_ProcEnd()");

  /* indicate page status */
  t30ShrdPrm.faxStat = FS_RCV_DOC;
  t30EntStat.curCmd = AT_CMD_NONE;

  TRACE_EVENT ("STOP FAX DATA PHASE");
  cmhT30_StartFIT();

 /* FCQ controls output of FPS */
  if(fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCQrq EQ FCQ_RQ_CQCDisabled)
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( FPS_PPR_Mcf,t30ShrdPrm.eol,0,0,0 );
  }
  else
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FPSppr,t30ShrdPrm.eol,0,0,0 );
  }

  R_AT( RAT_FET, t30EntStat.entOwn )( FET_PPM_Eop );
  R_AT( RAT_OK, t30EntStat.entOwn )( AT_CMD_FDR );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_ProcInt               |
+-------------------------------------------------------------------+

  PURPOSE : Procedural Interrupt
*/

GLOBAL void cmhT30_ProcInt ( void )
{
  TRACE_FUNCTION ("cmhT30_ProcInt()");

  /* resend PPM to T.30 */
  if(fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FIEval EQ FIE_VAL_AcceptPRI)
  {
    t30ShrdPrm.sgn_snd |= PRI_MRK;
  }

  if( pageSentFlg EQ TRUE )
  {
    if( psaT30_Ppm( ) < 0 )  /* T30 PPM request */
    {
      TRACE_EVENT( "FATAL RETURN psaT30 in cmhT30_ProcInt" );
    }
  }
  else
  {
    ppmPendFlg = TRUE;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_ProcIntInst           |
+-------------------------------------------------------------------+

  PURPOSE : Procedural Interrupt Installed

*/

GLOBAL void cmhT30_ProcIntInst ( void )
{

  TRACE_FUNCTION ("cmhT30_ProcIntInst()");

  /* send notification and wait for call modification */
  t30EntStat.curCmd = AT_CMD_NONE;

  TRACE_EVENT ("STOP FAX DATA PHASE");

  R_AT( RAT_FVO, t30EntStat.entOwn )();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_PageReceivedPRI       |
+-------------------------------------------------------------------+

  PURPOSE : FAX page was received, procedural interupt request

*/

GLOBAL void cmhT30_PageReceivedPRI ( void )
{
  TRACE_FUNCTION ("cmhT30_PageReceivedPRI()");

  /* indicate page status */
  t30ShrdPrm.faxStat = FS_RCV_DOC;
  t30EntStat.curCmd = AT_CMD_NONE;

  TRACE_EVENT ("STOP FAX DATA PHASE");
  cmhT30_StartFIT();

 /* FCQ controls output of FPS */
  if(fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCQrq EQ FCQ_RQ_CQCEnabled)
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( FPS_PPR_Pip,t30ShrdPrm.eol,0,0,0 );
  }
  else if(fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCQrq EQ FCQ_RQ_CQCDisabled)
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( FPS_PPR_Mcf,t30ShrdPrm.eol,0,0,0 );
  }

  R_AT( RAT_FET, t30EntStat.entOwn )( FET_PPM_PriMps );
  R_AT( RAT_OK, t30EntStat.entOwn ) ( AT_CMD_FDR );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_DocReceivedPRI        |
+-------------------------------------------------------------------+

  PURPOSE : FAX document was received, procedural interrupt request

*/

GLOBAL void cmhT30_DocReceivedPRI ( void )
{

  TRACE_FUNCTION ("cmhT30_DocReceivedPRI()");

  /* reset polling mode if received doc was polled */
  if(psaCC_ctb(t30ShrdPrm.cId)->calType EQ CT_MOC)
  {
    fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FSPval = FSP_VAL_PollDisabled;
  }

  /* indicate page status */
  t30ShrdPrm.faxStat = FS_DOC_TRF;
  t30EntStat.curCmd = AT_CMD_NONE;


  TRACE_EVENT ("STOP FAX DATA PHASE");
  cmhT30_StartFIT();

 /* FCQ controls output of FPS */
  if(fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCQrq EQ FCQ_RQ_CQCEnabled)
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( FPS_PPR_Pin,t30ShrdPrm.eol,0,0,0 );
  }
  else if(fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCQrq EQ FCQ_RQ_CQCDisabled)
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( FPS_PPR_Mcf,t30ShrdPrm.eol,0,0,0 );
  }

  R_AT( RAT_FET, t30EntStat.entOwn )( FET_PPM_PriEom );
  R_AT( RAT_OK, t30EntStat.entOwn ) ( AT_CMD_FDR );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_T30R                     |
|                            ROUTINE : cmhT30_ProcEndPRI            |
+-------------------------------------------------------------------+

  PURPOSE : FAX document was received, procedural interrupt request

*/

GLOBAL void cmhT30_ProcEndPRI ( void )
{

  TRACE_FUNCTION ("cmhT30_ProcEndPRI()");

  /* indicate page status */
  t30ShrdPrm.faxStat = FS_RCV_DOC;
  t30EntStat.curCmd = AT_CMD_NONE;

  TRACE_EVENT ("STOP FAX DATA PHASE");
  cmhT30_StartFIT();

 /* FCQ controls output of FPS */
  if(fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCQrq EQ FCQ_RQ_CQCEnabled)
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( FPS_PPR_Pin,t30ShrdPrm.eol,0,0,0 );
  }
  else if(fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FCQrq EQ FCQ_RQ_CQCDisabled)
  {
    R_AT( RAT_FPS, t30EntStat.entOwn )( FPS_PPR_Mcf,t30ShrdPrm.eol,0,0,0 );
  }

  R_AT( RAT_FET, t30EntStat.entOwn )( FET_PPM_PriEop );
  R_AT( RAT_OK, t30EntStat.entOwn ) ( AT_CMD_FDR );
}

//#ifdef DTI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_T30R           |
| STATE   : code                        ROUTINE : T30_connect_dti_cb |
+--------------------------------------------------------------------+

  PURPOSE : Callback for connection between T30 and UART.

*/

GLOBAL BOOL T30_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  T_ACI_DTI_PRC *srcInfos;
  T_DTI_CNTRL    info;

  TRACE_FUNCTION("T30_connect_dti_cb");


  if (dti_cntrl_get_info_from_dti_id( dti_id, &info) EQ FALSE)
  {
    TRACE_EVENT_P1("cannot find info for dti_id=%d", dti_id);
    return FALSE;
  }

  srcInfos = find_element (uart_src_params, info.src_id, cmhUARTtest_srcId);
  if (srcInfos EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] T30_connect_dti_cb: srcId=%d not found", info.src_id);
    return FALSE;
  }

  switch( result_type)
  {
    case(DTI_CONN_STATE_DISCONNECTED):

      dti_cntrl_clear_conn_parms( dti_id );

      if (t30EntStat.isTempDisconnected)
      {
        BITFIELD_SET (srcInfos->data_cntr, UART_DTI_SB_BIT);
        t30EntStat.isTempDisconnected = FALSE;
      }
      else
      {
        BITFIELD_CLEAR (srcInfos->data_cntr, UART_DTI_SB_BIT);
        cmhT30_Deactivate();
      }
      break;

    case(DTI_CONN_STATE_CONNECTED):
    case(DTI_CONN_STATE_CONNECTING):
    case(DTI_CONN_STATE_DISCONNECTING):
      break;
    case(DTI_CONN_STATE_ERROR):
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;

    default:
      TRACE_EVENT("T30_connect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}
#endif /* DTI OR FF_FAX*/
/*==== EOF ========================================================*/
