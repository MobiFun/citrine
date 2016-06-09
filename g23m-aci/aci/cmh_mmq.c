/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MMQ
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
|             protocol stack adapter for mobility management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_MMQ_C
#define CMH_MMQ_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "pcm.h"
#include "psa.h"
#include "psa_mm.h"
#include "psa_util.h"
#include "cmh.h"
#include "cmh_mm.h"
#include "rx.h"
#ifdef TI_PS_OP_OPN_TAB_ROMBASED
#include "plmn_decoder.h"
#include "rom_tables.h"
#endif

#include "rtc.h"

#ifdef FF_TIMEZONE
#include "rv/rv_general.h"
#include "rtc/rtc_tz.h"
#endif

#ifdef GPRS
#include "gaci_cmh.h"
#include "psa_gmm.h"
#endif

#ifndef _SIMULATION_
/******************************/
/* just to get FFS !!!!       */
/* TEMPORARY until use of NVM */
#ifndef GPRS
#define DONT_LET_FFSH2_DEF_GPRS
#endif

#include "../../services/ffs/ffs.h"

/* check whether latter has defined GPRS !!! */
#ifdef DONT_LET_FFSH2_DEF_GPRS
#undef GPRS
#endif

#undef DONT_LET_FFSH2_DEF_GPRS
/***************************/
#endif /* _SIMULATION_ */


/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
#if defined (GPRS) && defined (DTI)
EXTERN T_GMM_SHRD_PRM gmmShrdPrm;
#endif
EXTERN T_ONS_READ_STATE ONSReadStatus;
/*==== FUNCTIONS ==================================================*/

EXTERN BOOL cmhSIM_plmn_is_hplmn();

LOCAL T_ACI_RETURN qat_plus_percent_COPS( T_ACI_CMD_SRC srcId,
                                          T_ACI_COPS_MOD * mode,
                                          T_ACI_COPS_FRMT * format,
                                          CHAR * oper,
                                          T_ACI_COPS_SVST * svrStatus);
#ifdef TI_PS_FF_AT_P_CMD_CTREG
EXTERN BOOL cl_shrd_get_treg_val(T_ACI_TREG *treg);
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMQ                  |
| STATE   : code                  ROUTINE : qAT_PercentBAND             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %BAND? AT command
            which returns the current multiband configuration.

            <bandMode>: band switch mode.
            <bandType>: band selection.
*/

GLOBAL T_ACI_RETURN qAT_PercentBAND(T_ACI_CMD_SRC   srcId,
                                    T_ACI_BAND_MODE *bandMode,
                                    UBYTE           *bandType)
{
  UBYTE dummy;

  TRACE_FUNCTION ("qAT_PercentBAND()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  if( bandMode EQ NULL OR
      bandType EQ NULL )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  if(!cmhMM_getBandSettings( bandType, &dummy ))
  {
    *bandMode = BAND_MODE_Auto;
  }
  else
  {
    *bandMode = BAND_MODE_Manual;
  }

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCOPS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +COPS? AT command
            which returns the current setting of mode, format and
            operator.

            <mode>:   registration mode.
            <format>: format of operator
            <oper>:   operator string
*/

GLOBAL T_ACI_RETURN qAT_PlusCOPS ( T_ACI_CMD_SRC srcId,
                                   T_ACI_COPS_MOD * mode,
                                   T_ACI_COPS_FRMT * format,
                                   CHAR * oper )
{
  /* Implements Measure 127 */
  return (qat_plus_percent_COPS (srcId, mode, format, oper, NULL ) );

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCOPS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +COPS? AT command
            which returns the current setting of mode, format and
            operator.

            <mode>:   registration mode.
            <format>: format of operator
            <oper>:   operator string
*/

GLOBAL T_ACI_RETURN qAT_PercentCOPS ( T_ACI_CMD_SRC srcId,
                                   T_ACI_COPS_MOD * mode,
                                   T_ACI_COPS_FRMT * format,
                                   T_ACI_COPS_SVST * svrStatus,
                                   CHAR * oper )
{
  /* Implements Measure 127 */
  return (qat_plus_percent_COPS (srcId, mode, format, oper, svrStatus ) );
}




/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCREG             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CREG? AT command
            which returns the current setting of registration status.

            <stat>:   registration status.
            <lac>:    current lac.
            <cid>:    current cell id.
*/

GLOBAL T_ACI_RETURN qAT_PlusCREG ( T_ACI_CMD_SRC    srcId,
                                   T_ACI_CREG_STAT *stat,
                                   USHORT          *lac,
                                   USHORT          *cid )
{

  TRACE_FUNCTION ("qAT_PlusCREG()");

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  *stat = mmShrdPrm.creg_status;
  *lac  = mmShrdPrm.lac;
  *cid  = mmShrdPrm.cid;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMQ                  |
| STATE   : code                  ROUTINE : qAT_PercentCREG          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CREG? AT command
            which returns the current setting of registration status.

            <stat>:     registration status.
            <lac>:      current lac.
            <cid>:      current cell id.
            <gprs_ind>: if GPRS is available or not
*/

GLOBAL T_ACI_RETURN qAT_PercentCREG ( T_ACI_CMD_SRC srcId, 
                                      T_ACI_CREG_STAT *stat, 
                                      USHORT *lac, 
                                      USHORT *cid, 
                                      T_ACI_P_CREG_GPRS_IND *gprs_ind,
                                      U8*           rt)
{

  TRACE_FUNCTION ("qAT_PercentCREG()");

  qAT_PlusCREG (srcId, stat, lac, cid);

#if defined (GPRS) AND defined (DTI)
  *gprs_ind = gmmShrdPrm.gprs_indicator;
  *rt       = gmmShrdPrm.rt;
#else
  *gprs_ind = P_CREG_GPRS_Not_Supported; /* ACI-SPR-17218: use ACI type */
  *rt       = 0;
#endif /* GPRS */

  return( AT_CMPL );
}

#ifdef TI_PS_FF_AT_CMD_WS46
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMQ                  |
| STATE   : code                  ROUTINE : qAT_PlusWS46             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +WS46? AT
            command which returns the current setting wireless network
            selection.

            <mode>:   network mode.
*/

GLOBAL T_ACI_RETURN qAT_PlusWS46 (T_ACI_CMD_SRC  srcId,
                                  T_ACI_WS46_MOD * mode )
{

  TRACE_FUNCTION ("qAT_PlusWS46()");

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  *mode = WS46_MOD_Gsm;

  return( AT_CMPL );
}
#endif /* TI_PS_FF_AT_CMD_WS46 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCOPN             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +COPN AT
            command which returns the current operator names stored in
            ME.

            <lstId>:      list identifier.
            <startIdx>:   start index to read from.
            <lastIdx>:    buffer for index of last copied name.
            <oprLstBuf>:  buffer for operator names. length of MAX_OPER.
*/

GLOBAL T_ACI_RETURN qAT_PlusCOPN  ( T_ACI_CMD_SRC    srcId,
                                    T_ACI_COPN_LID   lstId,
                                    SHORT            startIdx,
                                    SHORT           *lastIdx,
                                    T_ACI_COPN_OPDESC *oprLstBuf)
{
  USHORT  idx;                        /* holds list idx */
  EF_PLMN plmnBuf;                    /* buffer PCM entry */
  USHORT  maxRec;                     /* holds maximum records */
  USHORT  recNr;                      /* holds record number */
  USHORT  oprLstLen;                  /* holds fixed oper list length */
  UBYTE   retVal;                     /* holds return value */
  UBYTE   ver;                        /* holds version */
  UBYTE   len;                        /* holds operator name length */

#ifdef TI_PS_OP_OPN_TAB_ROMBASED
/* Changes for ROM data */ 
  const UBYTE *plmn_comp_entry; /* get a local pointer holder */
  T_OPER_ENTRY oper;
  int  i;
#endif

  TRACE_FUNCTION ("qAT_PlusCOPN()"); 

/*
 *-------------------------------------------------------------------
 * determine list identifier
 *-------------------------------------------------------------------
 */
  switch( lstId )
  {
  /*
   *-----------------------------------------------------------------
   * for the list in permanent configuration memory
   *-----------------------------------------------------------------
   */
    case( COPN_LID_Pcm ):

      recNr = startIdx+1;
      idx   = 0;

      do
      {
        /* Implements Measure#32: Row 981  */
        retVal= pcm_ReadRecord( (UBYTE *) ef_plmn_id, recNr, SIZE_EF_PLMN,
                                (UBYTE *) &plmnBuf, &ver, &maxRec );

        if( retVal EQ PCM_INVALID_SIZE OR retVal EQ PCM_INVALID_RECORD )
        {
          if( idx EQ 0 )
          {
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
            return( AT_FAIL );
          }
          break;
        }

        if( plmnBuf.mcc[0] NEQ 0xFF AND plmnBuf.mnc[0] NEQ 0xFF AND
            plmnBuf.lngNam[0] NEQ 0xFF )
        {
          /* Implements Measure#32: Row 980 */
          sprintf(oprLstBuf[idx].numOper, format_03x02x_str,
                  ((plmnBuf.mcc[0]<<8) + plmnBuf.mcc[1]),
                  ((plmnBuf.mnc[0]<<8) + plmnBuf.mnc[1]));

          for( len = 0;
               len < SIZE_EF_PLMN_LONG AND plmnBuf.lngNam[len] NEQ 0xFF;
               len++ )
            ;

          utl_cvtGsmIra( plmnBuf.lngNam, len,
                         (UBYTE*)oprLstBuf[idx].alphaOper,
                         MAX_ALPHA_OPER_LEN,
                         CSCS_DIR_GsmToIra );

          oprLstBuf[idx].alphaOper[MINIMUM(len,MAX_ALPHA_OPER_LEN-1)] = 0x0;
          idx++;
        }

        recNr++;
      }
      while( idx < MAX_OPER );

      break;
  /*
   *-----------------------------------------------------------------
   * for the list in constant memory
   *-----------------------------------------------------------------
   */
    case( COPN_LID_Cnst ):

      oprLstLen = cmhMM_GetOperLstLen();

      if( startIdx >= oprLstLen )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
        return( AT_FAIL );
      }
#ifdef TI_PS_OP_OPN_TAB_ROMBASED
   /* Changes for ROM data */ 

    plmn_comp_entry = ptr_plmn_compressed;

    /* skip the first (start-1) number of plmn entries. */
    i=0;
    while (i < startIdx) {
        plmn_comp_entry += cmhMM_PlmnEntryLength (plmn_comp_entry);
        i+=1;
    }

      for( idx = 0, recNr = startIdx;
           idx < MAX_OPER AND recNr < oprLstLen AND 
           !(cmhMM_decodePlmn (&oper, plmn_comp_entry));
           idx++, recNr++ )
      {
        /* Implements Measure#32: Row 980 */
        cmhMM_mcc_mnc_print(&(oprLstBuf[idx].numOper[0]),
                            oper.mcc,
                            oper.mnc);

        strncpy( oprLstBuf[idx].alphaOper, oper.longName,
                 MAX_ALPHA_OPER_LEN-1 );

        oprLstBuf[idx].alphaOper[MAX_ALPHA_OPER_LEN-1] = 0x0;

        /* Next compressed PLMN entry */
        plmn_comp_entry += cmhMM_PlmnEntryLength (plmn_comp_entry);        
      }
#else
      for( idx = 0, recNr = startIdx;
           idx < MAX_OPER AND recNr < oprLstLen;
           idx++, recNr++ )
      {
        /* Implements Measure#32: Row 980 */
        cmhMM_mcc_mnc_print(&(oprLstBuf[idx].numOper[0]),
                            operListFixed[recNr].mcc,
                            operListFixed[recNr].mnc);

        strncpy( oprLstBuf[idx].alphaOper, operListFixed[recNr].longName,
                 MAX_ALPHA_OPER_LEN-1 );

        oprLstBuf[idx].alphaOper[MAX_ALPHA_OPER_LEN-1] = 0x0;
      }
#endif
      break;
  /*
   *-----------------------------------------------------------------
   * unexpected list type
   *-----------------------------------------------------------------
   */
    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * terminate list and set last index
 *-------------------------------------------------------------------
 */
  if( idx < MAX_OPER )
  {
    oprLstBuf[idx].numOper[0]   = 0x0;
    oprLstBuf[idx].alphaOper[0] = 0x0;
  }


  *lastIdx = ( recNr NEQ 0 )?recNr-1:0;

  return( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
|  NEW AT Command qAT_PercentCOPN                                   |
|                                                                   |
+-------------------------------------------------------------------+
*/
 /*The same function in EDGE should be remved from ACIA when EDGE is 
     merged to the UMTS*/ 

GLOBAL T_ACI_RETURN qAT_PercentCOPN( T_ACI_CMD_SRC  srcId,
                                     T_ACI_COPS_FRMT format,
                                     CHAR *opr,
                                     T_ACI_OPER_NTRY *oper_ntry)
{
  T_OPER_ENTRY plmnDesc;
  BOOL         found;

  if(format EQ COPS_FRMT_Numeric)
  {
    found = cmhMM_FindNumeric(&plmnDesc, opr);
  }
  else
  {
    found = cmhMM_FindName(&plmnDesc, opr,(T_ACI_CPOL_FRMT)format);
  }
  if (found)
  {
    oper_ntry->mcc = plmnDesc.mcc;
    oper_ntry->mnc = plmnDesc.mnc;
    strncpy (oper_ntry->longName, plmnDesc.longName, sizeof(oper_ntry->longName));
    oper_ntry->longName[sizeof(oper_ntry->longName) - 1] = '\0';
    strncpy ((CHAR *)oper_ntry->shrtName, plmnDesc.shrtName, sizeof(oper_ntry->shrtName));
    oper_ntry->shrtName[sizeof(oper_ntry->shrtName) - 1] = '\0';
    oper_ntry->long_len = plmnDesc.long_len;
    oper_ntry->shrt_len = plmnDesc.shrt_len;
    switch (plmnDesc.pnn)
    {
      case Read_EONS:
        oper_ntry->source = Read_EONS;
        break;
      case Read_CPHS:
        oper_ntry->source = Read_CPHS;
        break;
      default:
        oper_ntry->source = Read_ROM_TABLE;
        break;
    }
  }
  else
  {
    oper_ntry->longName[0] = '\0';
    oper_ntry->shrtName[0] = '\0';
    oper_ntry->mcc = 0;
    oper_ntry->mnc = 0;
    oper_ntry->source = Read_INVALID;
  }	
  return AT_CMPL;
}




/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMQ                  |
| STATE   : code                  ROUTINE : qAT_PercentNRG           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %NRG? AT command
            which returns the current setting of registration mode and
            service mode and the current status of service.

            <regMode>:   registration mode.
            <srvMode>:   service mode.
            <oprFrmt>:   operator format.
            <srvStat>:   service status.
            <oper>   :   operator
*/

GLOBAL T_ACI_RETURN qAT_PercentNRG( T_ACI_CMD_SRC srcId,
                                    T_ACI_NRG_RGMD *regMode,
                                    T_ACI_NRG_SVMD *srvMode,
                                    T_ACI_NRG_FRMT *oprFrmt,
                                    T_ACI_NRG_SVMD *srvStat,
                                    CHAR           *oper)
{

  T_ACI_COPS_FRMT copsFormat;
  

  TRACE_FUNCTION ("qAT_PercentNRG()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  switch( mmShrdPrm.regStat )
  {
    case( NO_VLD_RS   ):
    case( RS_NO_SRV   ):  *srvStat = NRG_SVMD_NoSrv;      break;
    case( RS_LMTD_SRV ):  *srvStat = NRG_SVMD_Limited;    break;
    case( RS_FULL_SRV ):  *srvStat = NRG_SVMD_Full;       break;
    default:              *srvStat = NRG_SVMD_NotPresent; break;
  }

  *srvMode = cmhPrm[srcId].mmCmdPrm.NRGsrvMode;
  *oprFrmt = cmhPrm[srcId].mmCmdPrm.NRGoprFrmt;
  *oper    = 0x0;

  *regMode = cmhPrm[srcId].mmCmdPrm.NRGregMode;

  switch( *oprFrmt )
  {
    case(  NRG_FRMT_NotPresent  ): 
      copsFormat = COPS_FRMT_NotPresent; 
      break;
    case( NRG_FRMT_Numeric   ): 
      copsFormat = COPS_FRMT_Numeric  ; 
      break;
    case( NRG_FRMT_Short):
      copsFormat = COPS_FRMT_Short ; 
      break;
    case( NRG_FRMT_Long):
      copsFormat = COPS_FRMT_Long; 
      break;
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  cmhMM_OperatorQuery(srcId,copsFormat,oper);

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI	         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : qAT_PercentCSQ             |
+--------------------------------------------------------------------+

  PURPOSE : This is the function for Signal Quality query  			

  Shen,Chao
  Juni.13th, 2003
*/
#ifdef FF_PS_RSSI
GLOBAL T_ACI_RETURN qAT_PercentCSQ( T_ACI_CMD_SRC  srcId,
                                    UBYTE *rssi,
                                    UBYTE *ber,
                                    UBYTE *actlevel,
                                    UBYTE *min_access_level)
#else
GLOBAL T_ACI_RETURN qAT_PercentCSQ( T_ACI_CMD_SRC  srcId,
                                    UBYTE *rssi,
                                    UBYTE *ber,
                                    UBYTE *actlevel)
#endif
{
  rx_Status_Type    rxStat;  

  TRACE_FUNCTION("qAT_PercentCSQ()");

  if ( rx_GetStatus ( &rxStat ) NEQ DRV_OK )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }
  else
  {
    if ( rxStat.gsmLevel EQ 0xFF OR rxStat.gsmLevel EQ 0 )
    {
      *rssi = ACI_RSSI_FAULT;
    }
    else if ( rxStat.gsmLevel > 59 )
    {
      *rssi = 31;
    }
    else
    {
      *rssi = ( rxStat.gsmLevel / 2 ) + 2;
    }

    if ( rxStat.rxQuality EQ RX_QUAL_UNAVAILABLE )
    {
      *ber = ACI_BER_FAULT;
    }
    else
    {
      *ber = rxStat.rxQuality;
    }

    *actlevel = rxStat.actLevel;


#ifdef FF_PS_RSSI
    if ( rxStat.min_access_level EQ RX_ACCE_UNAVAILABLE )
    {
      *min_access_level = ACI_MIN_RXLEV_FAULT;
    }
    else
    {
      *min_access_level = rxStat.min_access_level;
    }
#endif

    return AT_CMPL;
  }
}

#ifdef TI_PS_FF_AT_P_CMD_DBGINFO
/*
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI	              MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : qAT_PercentDBGINFO       |
+--------------------------------------------------------------------+

  PURPOSE : query free mem pool blocks.		
*/
GLOBAL T_ACI_RETURN qAT_PercentDBGINFO(T_ACI_CMD_SRC srcId, 
                                       ULONG param,
                                       USHORT stor,
                                       USHORT *free,
                                       USHORT *alloc)
{
  int ret=0;

  TRACE_FUNCTION ("qAT_PercentDBGINFO()");
  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if( ! cmh_IsVldCmdSrc (srcId) ) 
  { 
    return (AT_FAIL);
  }

  ret = vsi_m_status (hCommACI,
                       param,
                       stor,
                       free,
                       alloc );
  if (ret EQ VSI_ERROR OR *free EQ 0)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_MemFull );
    return (AT_FAIL);
  }

  return (AT_CMPL);
}
#endif /* TI_PS_FF_AT_P_CMD_DBGINFO */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : qAT_PlusCTZR             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CTZR AT command
            which will query the status of CTZRmode, indicating whether time
            zone change rep[orting is enabled or disabled.

            <on/off>:   Indicates whether time zone reporting is enabled or disabled.
*/

GLOBAL T_ACI_RETURN qAT_PlusCTZR ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CTZR_MODE *mode)
{
  TRACE_FUNCTION ("qAT_PlusCTZR()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("qAT_PlusCTZR(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

  *mode = cmhPrm[srcId].mmCmdPrm.CTZRMode;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : qAT_PlusCTZU             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CTZU AT command
            which will query the status of CTZUmode, indicating whether time
            zone automatic update is enabled or disabled.

            <on/off>:   Indicates whether time zone automatic update is enabled or disabled.
*/

GLOBAL T_ACI_RETURN qAT_PlusCTZU ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CTZU_MODE *mode)
{
  TRACE_FUNCTION ("qAT_PlusCTZU()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("qAT_PlusCTZU(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

  *mode = cmhPrm[srcId].mmCmdPrm.CTZUMode;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : qAT_PlusCCLK          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the sAT_PlusCCLK command
            which will query the status of the RTC in the MT, indicating what the time
            is in the MT.
*/
GLOBAL T_ACI_RETURN qAT_PlusCCLK ( T_ACI_CMD_SRC srcId,
                                   T_ACI_RTC_DATE *date_s,
                                   T_ACI_RTC_TIME *time_s,
                                   int * timeZone
                                 )
{
#ifndef _SIMULATION_
  UBYTE  ret;
#endif /* _SIMULATION_ */

  TRACE_FUNCTION ("qAT_PlusCCLK()");
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("qAT_PlusCCLK(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

  TRACE_EVENT("qAT_PlusCCLK(): get time and date " );
#ifndef _SIMULATION_
  ret = rtc_get_time_date((T_RTC_DATE *) date_s, (T_RTC_TIME *) time_s);
  switch (ret)
  {
    case 0:                     /* RVF_OK */
#ifdef FF_TIMEZONE
      *timeZone = RTC_GetCurrentTZ(); /* Get current timezone now time and date are obtained.*/
      TRACE_EVENT_P1("qAT_PlusCCLK(): timezone obatained: %d", *timeZone);
#endif /* FF_TIMEZONE */
      return( AT_CMPL );
    default:
      TRACE_EVENT_P1("qAT_PlusCCLK(): ERROR: %d", ret);  /* RVF_NOT_READY or RVF_INTERNAL ERROR */
      return( AT_FAIL );
  }
#else  /* _SIMULATION_ */
/* Set return info for date time to: "04/08/17,13:31:04-10" */
  date_s->year    = 2004;
  date_s->month   = 8;
  date_s->day     = 17;

  time_s->hour    = 13;
  time_s->minute  = 31;
  time_s->second  = 4;

  time_s->format  = 0;  /* RTC_TIME_FORMAT_24HOUR = 0 */
  time_s->PM_flag = 0;
  *timeZone = -10;  /* Set timezone to -10 */
  return( AT_CMPL );
#endif /* _SIMULATION_ */

}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : qAT_PercentCTZV             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CTZV AT command
            which will query the status of PCTZVmode, indicating whether time
            and date report is enabled or disabled.

            <on/off>:   Indicates whether time and date report is enabled or disabled.
*/

GLOBAL T_ACI_RETURN qAT_PercentCTZV ( T_ACI_CMD_SRC srcId,
                                   T_ACI_PCTZV_MODE *mode)
{
  TRACE_FUNCTION ("qAT_PercentCTZV()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("qAT_PercentCTZV(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

  *mode = cmhPrm[srcId].mmCmdPrm.PCTZVMode;

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : qAT_PercentCNIV             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CNIV AT command
            which will query the status of CNIVmode, indicating whether time
            and date report is enabled or disabled.

            <on/off>:   Indicates whether time and date report is enabled or disabled.
*/

GLOBAL T_ACI_RETURN qAT_PercentCNIV ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CNIV_MODE *mode)
{
  TRACE_FUNCTION ("qAT_PercentCNIV()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    TRACE_EVENT_P1("qAT_PercentCNIV(): Invalid source: %d", srcId);
    return( AT_FAIL );
  }

  *mode = cmhPrm[srcId].mmCmdPrm.CNIVMode;

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI               MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : qAT_PlusCIND             |
+--------------------------------------------------------------------+

  PURPOSE : This is the function for Signal Quality query
*/

GLOBAL T_ACI_RETURN qAT_PlusCIND ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CIND_SIGNAL_TYPE  *sCindSgnalSettings,
                                   T_ACI_CIND_SMSFULL_TYPE *sCindSmsFullSettings )
{
  T_ACI_MM_CIND_VAL_TYPE  *pMMCmdPrm;

  TRACE_FUNCTION ("qAT_PlusCIND()");

  if( !cmh_IsVldCmdSrc( srcId ) OR (sCindSgnalSettings EQ NULL) OR (sCindSmsFullSettings EQ NULL) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pMMCmdPrm = &(cmhPrm[srcId].mmCmdPrm.sIndicationParam.sMmCINDSettings);

  *sCindSgnalSettings   = pMMCmdPrm->sCindSignalParam;
  *sCindSmsFullSettings = pMMCmdPrm->sCindSmsFullParam;

  return(AT_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI               MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : qAT_PlusCMER             |
+--------------------------------------------------------------------+

  PURPOSE : This is the function for Signal Quality query
*/

GLOBAL T_ACI_RETURN qAT_PlusCMER ( T_ACI_CMD_SRC srcId,
                                   T_ACI_CMER_MODE_TYPE *sCmerModeSettings,
                                   T_ACI_CMER_IND_TYPE  *sCmerIndicationSettings,
                                   T_ACI_CMER_BFR_TYPE  *sCmerBfrSettings )

{
  T_ACI_MM_CMER_VAL_TYPE  *pMMCmdPrm;

  TRACE_FUNCTION ("qAT_PlusCMER()");

  if( !cmh_IsVldCmdSrc( srcId ) OR (sCmerModeSettings EQ NULL) OR
      (sCmerIndicationSettings EQ NULL) OR (sCmerBfrSettings EQ NULL))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  pMMCmdPrm = &(cmhPrm[srcId].mmCmdPrm.sIndicationParam.sMmCMERSettings);

  *sCmerModeSettings       = pMMCmdPrm->sCmerModeParam;
  *sCmerIndicationSettings = pMMCmdPrm->sCmerIndParam;
  *sCmerBfrSettings        = pMMCmdPrm->sCmerBfrParam;

  return(AT_CMPL);
}

/* Implements Measure 127 */
/*
+------------------------------------------------------------------------------
|  Function    : qat_plus_percent_COPS 
+------------------------------------------------------------------------------
|  Purpose     : This is the functional counterpart to Both +COPS? and %COPS 
|                AT command which returns the current setting of mode, 
|                format and operator.
|
|  Parameters  : srcId      - AT command source identifier
|                mode       - +COPS parameter <mode> 
|                format     - +COPS parameter <format>
|                oper       - Operator
|                svrStatus  - %COPS parameter <srvStatus>
|                at_cmd_id  - AT Command Identifier
|
|  Return      : ACI functional return codes
+------------------------------------------------------------------------------
*/

LOCAL T_ACI_RETURN qat_plus_percent_COPS( T_ACI_CMD_SRC srcId, T_ACI_COPS_MOD * mode,
                                          T_ACI_COPS_FRMT * format, CHAR * oper,
                                          T_ACI_COPS_SVST * svrStatus)
{

  T_MM_CMD_PRM * pMMCmdPrm;   /* points to MM command parameters */

  TRACE_FUNCTION ("qat_plus_percent_COPS()");

  /* check command source */  
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pMMCmdPrm = &cmhPrm[srcId].mmCmdPrm;

  /* fill in parameters */  
  switch(mmShrdPrm.COPSmode)
  {
    case(COPS_MOD_Auto):
    case(COPS_MOD_Man):
    case(COPS_MOD_Dereg):
      *mode = mmShrdPrm.COPSmode;
      break;

    /*
     * case(COPS_MOD_SetOnly):
     * mmShrdPrm.COPSmode can't have been set with this value
     */

    case(COPS_MOD_Both):
      if(mmShrdPrm.regModeAutoBack)
      {
        *mode = COPS_MOD_Man;
      }
      else
      {
        *mode = COPS_MOD_Auto;
      }
      break;

    default:
      *mode = (T_ACI_COPS_MOD)mmShrdPrm.regMode;
      break;
  }

  *format = pMMCmdPrm -> COPSfrmt;

  if ( svrStatus )
  {
    switch( mmShrdPrm.regStat )
    {
      case( NO_VLD_RS   ):
      case( RS_NO_SRV   ):  *svrStatus = COPS_SVST_NoSrv;      break;
      case( RS_LMTD_SRV ):  *svrStatus =COPS_SVST_Limited;     break;
      case( RS_FULL_SRV ):  *svrStatus = COPS_SVST_Full;       break;
      default:              *svrStatus = COPS_SVST_NotPresent; break;
    }
  }

  *oper   = 0x0;
  cmhMM_OperatorQuery(srcId,pMMCmdPrm -> COPSfrmt,oper);
  TRACE_EVENT(oper);
  return( AT_CMPL );
}
#ifdef TI_PS_FF_AT_P_CMD_CTREG
/*
+--------------------------------------------------------------------+
| PROJECT : ACI/MMI               MODULE  : CMH_MMS                  |
| STATE   : code                  ROUTINE : qAT_PlusCTREG            |
+--------------------------------------------------------------------+

  PURPOSE : This is the function for Signal Quality query
*/

GLOBAL T_ACI_RETURN qAT_PercentCTREG ( T_ACI_CMD_SRC srcId, T_ACI_TREG *treg )
{
  BOOL ret = FALSE;

  TRACE_FUNCTION ("qAT_PlusCTREG()");

  if( !cmh_IsVldCmdSrc( srcId ))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  ret = cl_shrd_get_treg_val(treg);

  if(ret EQ TRUE)
  {
    return (AT_CMPL);
  }

  return (AT_FAIL);
}
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

/*==== EOF ========================================================*/
