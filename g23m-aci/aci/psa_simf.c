/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SIMF
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
|  Purpose :  This module defines the functions for the protocol
|             stack adapter for the subscriber identity module.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_SIMF_C
#define PSA_SIMF_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#undef TRACING

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "psa.h"
#include "psa_sim.h"
#include "psa_util.h"

#ifdef UART
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#ifdef SIM_TOOLKIT
#include "psa_cc.h"
#include "psa_sat.h"
#include "aci_fd.h" /* necessary for cmh.h */
#include "cmh.h"    /* necessary for cmh_sat.h */
#include "cmh_sat.h"
#include "cmh_sim.h"
#endif /* SIM_TOOLKIT */

/*==== CONSTANTS ==================================================*/

#ifdef TRACING
#define ITM_WDT         (14)    /* item width in chars */
#define HDR_WDT         (10)    /* header width in chars */
#endif /* TRACING */

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : psaSIM_ChkSIMSrvSup     |
+-------------------------------------------------------------------+

  PURPOSE : Check if the SIM service is supported or not.

*/

GLOBAL BOOL psaSIM_ChkSIMSrvSup( UBYTE srvNr )
{
  srvNr--;

  if( srvNr/4 >= SRV_TAB_LEN ) return ( FALSE );

  if( ((simShrdPrm.srvTab[srvNr/4] >> ((srvNr%4)*2))&0x03) EQ
         SRV_ALLOC_ACTIV )

    return( TRUE );

  return( FALSE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMF                |
|                                 ROUTINE : psaSIM_atbNewEntry      |
+-------------------------------------------------------------------+

  PURPOSE : returns the access table index for an entry that is
            free to use. -1 indicates that the access table is full.
*/

GLOBAL SHORT psaSIM_atbNewEntry ( void )
{
  SHORT atbIdx;             /* holds access table index */

  for( atbIdx = 0; atbIdx < ACC_MAX; atbIdx++ )
  {
    if( simShrdPrm.atb[atbIdx].ntryUsdFlg EQ FALSE )
    {
      psaSIM_InitAtbNtry( atbIdx );

#if defined _SIMULATION_
      TRACE_EVENT_P1("SIM table ID = %d", atbIdx);
#endif
      return( atbIdx );
    }
  }

  TRACE_ERROR("[psaSIM_atbNewEntry]: did not find a new SIM table entry");
  return( -1 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMF                |
|                                 ROUTINE : psaSIM_CloseAtb         |
+-------------------------------------------------------------------+

  PURPOSE : close all ATB entries with the provided SIM error code.

*/

GLOBAL void psaSIM_CloseAtb ( USHORT error )
{
  int atbIdx;             /* holds access table index */

  for( atbIdx = 0; atbIdx < ACC_MAX; atbIdx++ )
  {
    if( simShrdPrm.atb[atbIdx].ntryUsdFlg )
    {
      simShrdPrm.atb[atbIdx].errCode = error;

      if( simShrdPrm.atb[atbIdx].rplyCB )
        simShrdPrm.atb[atbIdx].rplyCB( (SHORT)atbIdx );

      simShrdPrm.atb[atbIdx].ntryUsdFlg = FALSE;
    }
  }
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMF                |
|                                 ROUTINE : psaSIM_InitAtbNtry      |
+-------------------------------------------------------------------+

  PURPOSE : initialize the indexed access table entry.

*/

GLOBAL void psaSIM_InitAtbNtry ( SHORT idx )
{

/*
 *-------------------------------------------------------------------
 * initialize access table entry
 *-------------------------------------------------------------------
 */
  simShrdPrm.atb[idx].ntryUsdFlg  = FALSE;
  simShrdPrm.atb[idx].accType     = NO_VLD_ACT;
  simShrdPrm.atb[idx].v_path_info = FALSE;
  simShrdPrm.atb[idx].reqDataFld  = 0;
  simShrdPrm.atb[idx].dataOff     = 0;
  simShrdPrm.atb[idx].recNr       = 0;
  simShrdPrm.atb[idx].check_dataLen = FALSE;  /* in case of read record operation,
     datalen passed to SIM in the rquest will always be 0xFF. However when a buffer
     has been passed, the size of data received from SIM to be copied in it has to
     be checked...*/
  simShrdPrm.atb[idx].dataLen     = 0;
  simShrdPrm.atb[idx].exchData    = NULL;
  simShrdPrm.atb[idx].recMax      = 0;
  simShrdPrm.atb[idx].errCode     = SIM_NO_ERROR;
  simShrdPrm.atb[idx].rplyCB      = NULL;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMF                |
|                                 ROUTINE : psaSIM_cnvrtIMSI2ASCII  |
+-------------------------------------------------------------------+

  PURPOSE : converts the IMSI into ASCII representation and returns
            pointer to IMSI ASCII string.
*/

GLOBAL CHAR* psaSIM_cnvrtIMSI2ASCII ( CHAR * imsiBuf )
{
  psaSIM_decodeIMSI (  simShrdPrm.imsi.field, 
                       simShrdPrm.imsi.c_field, 
                       imsiBuf);
  return( imsiBuf );
}


/*
  PURPOSE : convert imsi (packed bcd to ASCIIZ; ->11.11)
*/
GLOBAL void psaSIM_decodeIMSI (UBYTE* imsi_field, 
                               UBYTE  imsi_c_field, 
                               CHAR* imsi_asciiz)
{
  UBYTE imsi_len;
  UBYTE i;
  UBYTE digit;

  TRACE_FUNCTION ("aci_slock_sim_decodeIMSI()");

  /*
                     | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
                     +---+---+---+---+---+---+---+---+
     imsi_c_field  = |         Length indicator      |
     imsi_field[0] = | IMSI digit 1  | p | 0 | 0 | 1 |
     imsi_field[1] = | IMSI digit 3  | IMSI digit 2  |
                    

     imsi_c_field = length indicator:
                    The length indicator refers to the number of significant bytes,
                    not including this length byte, required for the IMSI.
     p = parity
         0: Even number of IMSI digits
         1: Odd number of IMSI digits

     If the number of IMSI digits is even then bits 5 to 8 of the last octet
     shall be filled with an end mark coded as 1111b
   */

  /*
   * Check length
   */
  if ((imsi_c_field EQ 0) OR (imsi_c_field > (MAX_IMSI-1)))  /* maybe 0xFF on some testcards */
  {
    TRACE_EVENT_P1("[WRN] imsi_c_field = %d is not valid", imsi_c_field);
    imsi_asciiz[0] = '\0';    /* return empty string in case of error */
    return;
  }

  /*
   * calculate number of digits
   */
  imsi_len = (imsi_c_field)*2-1;   /* -1 goes off for parity nibble */

  /*
   * if even number of digits then last upper nibble is an end mark '1111'
   */
  if ((imsi_field[0] & 0x08) EQ 0)
  {
    imsi_len--;
  }

  /*
   * extract all digits
   */
  for (i=0; i<imsi_len; i++)
  {
    if ((i & 1) EQ 0)
    {
      /* process IMSI digit 1,3,5,... at i=0,2,4,...*/
      digit = (imsi_field[(i+1)/2] & 0xf0) >> 4;   /* +1 is to skip parity nibble */
    }
    else
    {
      /* process IMSI digit 2,4,6,... at i=1,3,5,...*/
      digit = (imsi_field[(i+1)/2] & 0x0f); 
    }

    if (digit > 9)  /* 3.03/2.3 IMSI shall consist of numerical characters (0 through 9) only.*/
    {
      TRACE_EVENT("[WRN] invalid BCD digit found in IMSI, aborting");
      imsi_asciiz[0] = '\0';    /* return empty string in case of error */
      return;
    }
    else
    {
      imsi_asciiz[i] = '0' + digit;
    }
  }
  imsi_asciiz[i] = '\0';
  return;
}

/*
  PURPOSE : convert imsi (ASCIIZ to packed bcd; ->11.11)
*/
GLOBAL void psaSIM_encodeIMSI (CHAR* imsi_asciiz, 
                               UBYTE* imsi_c_field, 
                               UBYTE* imsi_field)
{
  UBYTE i;
  UBYTE digit;
  UBYTE imsi_len;

  TRACE_FUNCTION ("aci_slock_sim_encodeIMSI()");
  imsi_len = strlen(imsi_asciiz);
  if (imsi_len > 15) /* 3.03/2.3 The overall number of digits in IMSI shall not exceed 15 digits. */
  {
    TRACE_EVENT_P1("[WRN] imsi_len shall not exceed 15 digits, but is %d, truncating!", imsi_len);
    imsi_len = 15;
  }

  /* Calculate the Length indicator */
  *imsi_c_field = (imsi_len+1+1)/2;          /* +1 for parity nibble and +1 to round up */
  imsi_field[0] = (imsi_len%2 ? 0x09:0x01);  /* fill parity and some default bits */
  for (i=0; i<MAX_IMSI_LEN; i++)             /* fill the whole EF field, pad remaining with 0xf */
  {
    if (i<imsi_len)
    {
      digit = imsi_asciiz[i];
      if (digit >= '0' OR digit <= '9')
      {
        digit-= '0';
      }
      else
      {
        TRACE_EVENT_P1("[WRN] invalid digit in IMSI \"%d\", skipping!", digit);
        digit = 0x0f;
      }
    }
    else
    {
      digit = 0x0f;   /* 11.11/10.3.2 If a network operator chooses an IMSI of less than 15 digits,
                          unused nibbles shall be set to 'F'. */
    }

    if ((i & 1) EQ 0)
    {
      /* process IMSI digit 1,3,5,... at i=0,2,4,...*/
      imsi_field[(i+1)/2] |= digit << 4;           /* first +1 is to skop parity nibble */
    }
    else
    {
      /* process IMSI digit 2,4,6,... at i=1,3,5,...*/
      imsi_field[(i+1)/2]  = digit;
    }
  }
}




/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIM                 |
|                                 ROUTINE : psaSIM_Init             |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for SIM.

*/

/* MACRO: initializer for set parameter */
#define INIT_SIM_SET_PARM( dest, def )\
  for( LpCnt = 0; LpCnt < OWN_SRC_MAX; LpCnt++ )\
    simShrdPrm.setPrm[LpCnt].dest = def

GLOBAL void psaSIM_Init (T_ACI_INIT_TYPE init_type)
{
  UBYTE LpCnt;          /* holds loop counter for macro */
  UBYTE atbIdx;         /* holds index to access table */

#ifdef SIM_TOOLKIT
  T_ACI_CMD_SRC cmd_src;  /* for setting up SIMEF mode */
  UBYTE idx;            /* holds profile index */
#endif

/*
 *-------------------------------------------------------------------
 * initialize access table
 *-------------------------------------------------------------------
 */
  for( atbIdx = 0; atbIdx < ACC_MAX; atbIdx++ )
  {
    psaSIM_InitAtbNtry( atbIdx );
  }

/*
 *-------------------------------------------------------------------
 * set default parms
 *-------------------------------------------------------------------
 */
  memset( simShrdPrm.setPrm, 0, sizeof(T_SIM_SET_PRM));
  memset( simShrdPrm.imsi.field, 0, MAX_IMSI);
  simShrdPrm.rslt          = SIM_NO_ERROR;
  simShrdPrm.pn1Cnt        = 0;
  simShrdPrm.pn2Cnt        = 0;
  simShrdPrm.pk1Cnt        = 0;
  simShrdPrm.pk2Cnt        = 0;
  simShrdPrm.PINStat       = NO_VLD_PS;
  simShrdPrm.pn1Stat       = NO_VLD_PS;
  simShrdPrm.pn2Stat       = NO_VLD_PS;
  simShrdPrm.PINQuery      = 0;
  simShrdPrm.crdPhs        = 0xFF;
  simShrdPrm.SIMStat       = NO_VLD_SS;
  simShrdPrm.PEDStat       = NO_VLD_PEDS;
  simShrdPrm.imsi.c_field  = 0;
  simShrdPrm.PLMN_Mode_Bit = NOT_PRESENT_8BIT;
  simShrdPrm.crdFun        = SIM_NO_OPERATION;
  simShrdPrm.ciSIMEnabled  = TRUE;
  simShrdPrm.imei_blocked  = FALSE;
  simShrdPrm.opl_list.opl_status = FALSE;
  simShrdPrm.pnn_list.pnn_status = FALSE;
  memset( simShrdPrm.srvTab, 0, SRV_TAB_LEN );
#ifdef FF_SAT_E
  simShrdPrm.sat_class_e_dti_id = DTI_DTI_ID_NOTPRESENT;
#endif /*FF_SAT_E*/
#ifdef TI_PS_FF_AT_P_CMD_ATR
  simShrdPrm.atr.len       = 0;
#endif /* TI_PS_FF_AT_P_CMD_ATR */
#ifdef FF_DUAL_SIM
  simShrdPrm.SIM_Powered_on = 0;
  simShrdPrm.SIM_Selection = FALSE;
#endif /*FF_DUAL_SIM*/
  INIT_SIM_SET_PARM( actProc,    SIM_INITIALISATION );
  INIT_SIM_SET_PARM( PINType,    0 );
  
  for( LpCnt = 0; LpCnt < OWN_SRC_MAX; LpCnt++ )
  {
    memset( simShrdPrm.setPrm[LpCnt].curPIN, NOT_PRESENT_CHAR, PIN_LEN );
    memset( simShrdPrm.setPrm[LpCnt].newPIN, NOT_PRESENT_CHAR, PIN_LEN );
    memset( simShrdPrm.setPrm[LpCnt].unblkKey, NOT_PRESENT_CHAR, PUK_LEN );

#ifdef TI_PS_FF_AT_P_CMD_CUST
    simShrdPrm.setPrm[LpCnt].cust_mode = (UBYTE)CUST_NORMAL_BEHAVIOUR;
#endif /* TI_PS_FF_AT_P_CMD_CUST */
    simShrdPrm.setPrm[LpCnt].sat_cc_mode = SATCC_CONTROL_BY_SIM_ACTIVE;
  }

#ifdef TI_PS_FF_AT_P_CMD_CUST
  simShrdPrm.overall_cust_mode = (UBYTE)CUST_NORMAL_BEHAVIOUR;
#endif /* TI_PS_FF_AT_P_CMD_CUST */
  simShrdPrm.pb_stat = PB_STAT_Blocked;

#ifdef SIM_TOOLKIT

  for (cmd_src=CMD_SRC_LCL;cmd_src<CMD_SRC_MAX;cmd_src++)
  {
    simShrdPrm.SIMEFMode[cmd_src]=SIMEF_MODE_OFF;
  }
  for( idx = 0; idx < MAX_STK_PRF; idx++ )
  {
    INIT_SIM_SET_PARM( STKprof[idx], satDefPrfl[idx] );
  }

  if (init_type EQ ACI_INIT_TYPE_ALL)
  {
    if (!psaSAT_FURegister (cmhSIM_AD_Update))
    {
      TRACE_EVENT ("FAILED to register the handler cmhSIM_AD_Update() for FU");
    }
    if (!psaSAT_FURegister (cmhSIM_OpUpdate))
    {
      TRACE_EVENT ("FAILED to register the handler cmhSIM_OpUpdate() for FU");
    }
    if (!psaSAT_FURegister (cmhSIM_CSP_Update))
    {
      TRACE_EVENT ("FAILED to register the handler cmhSIM_CSP_Update() for FU");
    }
    if (!psaSAT_FURegister (cmhSIM_ONS_Update))
    {
      TRACE_EVENT ("FAILED to register the handler cmhSIM_ONS_Update() for FU");
    }
  }
  else
  {
    cmhSAT_CBMDestroyList();
  }
#endif

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMF                |
|                                 ROUTINE : psaSIM_shrPrmDump       |
+-------------------------------------------------------------------+

  PURPOSE : this function dumps the shared parameter to the debug
            output.
*/

#ifdef TRACING
GLOBAL void psaSIM_shrPrmDump ( void )
{
  char  lnBuf [80];             /* holds buffer for output line */
  char  pinBuf [PIN_LEN+1];     /* holds buffer for pin */
  SHORT chrNr;                  /* holds number of processed chars */

  /* --- PIN type -------------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "  PIN type" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         simShrdPrm.setPrm[0].PINType );
  TRACE_EVENT( lnBuf );

  /* --- current PIN ----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " curr. PIN" );
  sprintf( pinBuf, "%*.*s", PIN_LEN, PIN_LEN, simShrdPrm.setPrm[0].curPIN );
  chrNr += sprintf( lnBuf+chrNr, "%*.*s", ITM_WDT, ITM_WDT, pinBuf );
  TRACE_EVENT( lnBuf );

  /* --- new PIN --------------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "   new PIN" );
  sprintf( pinBuf, "%*.*s", PIN_LEN, PIN_LEN, simShrdPrm.setPrm[0].newPIN );
  chrNr += sprintf( lnBuf+chrNr, "%*.*s", ITM_WDT, ITM_WDT, pinBuf );
  TRACE_EVENT( lnBuf );

  /* --- unblock key ----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "unblck.key" );
  sprintf( pinBuf, "%*.*s", PIN_LEN, PIN_LEN, simShrdPrm.setPrm[0].unblkKey );
  chrNr += sprintf( lnBuf+chrNr, "%*.*s", ITM_WDT, ITM_WDT, pinBuf );
  TRACE_EVENT( lnBuf );

  /* --- PIN status -----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "PIN status" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         simShrdPrm.PINStat );
  TRACE_EVENT( lnBuf );

  /* --- result ---------------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "    result" );
  chrNr += sprintf( lnBuf+chrNr, "%*X", ITM_WDT,
                                        simShrdPrm.rslt );
  TRACE_EVENT( lnBuf );

  /* --- PIN 1 count ----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " PIN 1 cnt" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         simShrdPrm.pn1Cnt );
  TRACE_EVENT( lnBuf );

  /* --- PIN 2 count ----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " PIN 2 cnt" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         simShrdPrm.pn2Cnt );
  TRACE_EVENT( lnBuf );

  /* --- PUK 1 count ----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " PUK 1 cnt" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         simShrdPrm.pk1Cnt );
  TRACE_EVENT( lnBuf );

  /* --- PUK 2 count ----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " PUK 2 cnt" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         simShrdPrm.pk2Cnt );
  TRACE_EVENT( lnBuf );

  /* --- card phase -----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "card phase" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         simShrdPrm.crdPhs );
  TRACE_EVENT( lnBuf );

  /* --- SIM status -----------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "SIM status" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         simShrdPrm.SIMStat );
  TRACE_EVENT( lnBuf );

  /* --- card functionality ---------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "card funct" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         simShrdPrm.crdFun );
  TRACE_EVENT( lnBuf );
}
#endif  /* of #ifdef TRACING */

#ifdef SIM_PERS_OTA
/*
+----------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SIMF       |
|                                 ROUTINE : aci_slock_ota_init       |
+----------------------------------------------------------+

  PURPOSE : this function registers the ccmhSIM_Register_Read_DCKduring init
      and also reads the contents of EF_DCK at the init
*/

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_ota_init
+------------------------------------------------------------------------------
|  Description : this function registers the ccmhSIM_Register_Read_DCKduring init
|      and also reads the contents of EF_DCK at the init
|
|  Parameters  : None
|
|  Return      : None
|
+------------------------------------------------------------------------------
*/

GLOBAL void aci_slock_ota_init()
{
  TRACE_FUNCTION("aci_slock_ota_init()");
  psaSAT_FURegister(cmhSIM_Register_Read_DCK);
}
#endif
/*==== EOF ========================================================*/
