/*  
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SIMF
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
|  Purpose :  This module defines the functions used by the commad
|             handler for the subscriber identity module.
+-----------------------------------------------------------------------------
*/

#ifndef CMH_SIMF_C
#define CMH_SIMF_C
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
#include "pcm.h"

#ifdef DTI
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_sim.h"
#include "psa_cc.h"
#include "psa_sat.h"
#include "psa_mm.h"
#include "psa_util.h"
#include "cmh.h"
#include "cmh_sim.h"
#include "cmh_mm.h"
#include "phb.h"
#include "cmh_phb.h"
#ifdef TI_PS_OP_OPN_TAB_ROMBASED
#include "plmn_decoder.h"
#include "rom_tables.h"
#endif

#ifdef SIM_PERS
#include "aci_ext_pers.h"
#include "aci_slock.h"
#include "general.h"  /* inluded for UINT8 compilation error in sec_drv.h */
#include "sec_drv.h"

 EXTERN  T_ACI_SIM_CONFIG aci_slock_sim_config;
 EXTERN T_SEC_DRV_CONFIGURATION *cfg_data;
#endif

/* #include "m_cc.h" */

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
const UBYTE PLMNselEmpty[] = { 0xFF, 0xFF, 0xFF };
#ifdef SIM_PERS_OTA
UBYTE nw_ctrl_key[9], nw_subset_ctrl_key[9], sp_ctrl_key[9], corp_ctrl_key[9];
#define MAX_DCK_LEN 16 
#endif

/* Implements Measure#32: Row 60 & 1039  */
const char * const ef_mssup_id = EF_MSSUP_ID;
/*==== FUNCTIONS ==================================================*/
void cmhSIM_Read_AD_cb(SHORT table_id);
void cmhSIM_Get_CSP_cb(SHORT table_id);
void cmhSIM_Read_CSP_cb(SHORT table_id);
#ifdef SIM_PERS_OTA
void cmhSIM_Read_DCK_cb(SHORT table_id);
void cmhSIM_Read_DCK_init_cb(SHORT table_id);
void cmhSIM_WriteDefaultValue_DCK_cb(SHORT table_id);
#endif
LOCAL void cmhSIM_OpRead_simDataFld_RcdCb( SHORT table_id,
                                           USHORT reqDataFld );
LOCAL BOOL cmhSIM_AD_CSP_Update ( int ref,
                                  T_SIM_FILE_UPDATE_IND *fu,
                                  USHORT sim_datafld );
LOCAL BOOL cmhSIM_OpRead_Opl_or_Pnn_Rcd( UBYTE rcd,
                                         USHORT reqDataFld,
                                         void  (*rplyCB)(SHORT) );
LOCAL T_ACI_RETURN cmhSIM_Write_or_Read_TranspEF ( T_ACI_CMD_SRC srcId,
                                                   T_ACI_AT_CMD cmd,
                                                   USHORT datafield,
                                                   USHORT offset,
                                                   UBYTE datalen,
                                                   UBYTE * exchData,
                                                   void (*rplyCB)(SHORT),
                                                   T_SIM_ACTP accType,
                                                   BOOL v_path_info,
                                                  T_path_info   *path_info_ptr);
LOCAL T_ACI_RETURN cmhSIM_Write_or_Read_RecordEF ( T_ACI_CMD_SRC srcId,
                                                   T_ACI_AT_CMD cmd,
                                                   USHORT datafield,
                                                   UBYTE record,
                                                   UBYTE datalen,
                                                   UBYTE * exchData,
                                                   void (*rplyCB)(SHORT),
                                                   T_SIM_ACTP accType,
                                                   BOOL v_path_info,
                                                   T_path_info   *path_info_ptr);

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIM                 |
|                                 ROUTINE : cmhSIM_FillInPIN        |
+-------------------------------------------------------------------+

  PURPOSE : fill in the PIN into the PIN field of size length and
            stuff unused chars with 0xFF.

*/

GLOBAL void cmhSIM_FillInPIN ( CHAR * PINStr, CHAR * PINFld, UBYTE len )
{
  UBYTE idx;

  strncpy( PINFld, PINStr, len );

  for( idx = strlen( PINStr ); idx < len; idx++ )
  {
    PINFld[idx] = NOT_PRESENT_CHAR;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIM                 |
|                                 ROUTINE : cmhSIM_GetHomePLMN      |
+-------------------------------------------------------------------+

  PURPOSE : Extract home PLMN out of the IMSI.

*/

GLOBAL void cmhSIM_GetHomePLMN ( SHORT * mccBuf, SHORT * mncBuf )
{
  UBYTE digit;        /* holds 4bit digit */
  UBYTE i;            /* holds counter */

  if( simShrdPrm.imsi.c_field EQ 0 )
  {
    /*
     * No SIM present
     */
    *mccBuf = *mncBuf = -1;
  }
  else
  {
    /*
     * SIM present
     */
    *mccBuf = *mncBuf = 0;

    /* Convert MCC and MNC. */
    for (i = 0; i < SIZE_MCC + SIZE_MNC; i++)
    {
      digit = (i & 1) ?
        (simShrdPrm.imsi.field[(i + 1)/2] & 0x0f) :
        (simShrdPrm.imsi.field[(i + 1)/2] & 0xf0) >> 4;
      if (i < SIZE_MCC)
        *mccBuf = (*mccBuf << 4) | digit;
      else
        *mncBuf = (*mncBuf << 4) | digit;
    }
    /* The only 3 digit mnc codes that are valid are the values between 310 and 316 */
    if ((*mccBuf >= 0x310) AND (*mccBuf <= 0x316)
        OR  simShrdPrm.mnc_len EQ 3)
    {
      /* used in the US - mcc = 0x310 */
    }
    /* Set the third digit of the MNC to 'F' if the SIM indicates a 2-digit MNC country */
    else /* if (simShrdPrm.mnc_len EQ 2) */
    {
      /* The MS digit of the mnc is not valid, so replace the LSB it with 0xF. */
      *mncBuf |= 0x00F;
    }
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhSIM_plmn_equal_sim   |
+-------------------------------------------------------------------+

  PURPOSE : Return TRUE if the PLMN received equals the PLMN 
            stored on the SIM.
            This is not exactly the algorithm as shown for HPLMN 
            matching as shown in 03.22 Normative Annex A, this version
            here is more universal.            

*/

GLOBAL BOOL cmhSIM_plmn_equal_sim (SHORT bcch_mcc, SHORT bcch_mnc, 
                                   SHORT  sim_mcc, SHORT  sim_mnc)
{
  /*TRACE_FUNCTION ("cmhSIM_plmn_equal_sim()");*/

  /* Check MCC */
  if (sim_mcc NEQ bcch_mcc)
    return FALSE;

  /* Check first 2 MNC digits */
  if ((sim_mnc & 0xff0) NEQ (bcch_mnc & 0xff0))
    return FALSE;

  /* Check for full match */
  if ((sim_mnc & 0xf) EQ (bcch_mnc & 0xf))
    return TRUE;

  /* The 3rd digit of the MNC differs */
  if ((bcch_mcc >= 0x310) AND (bcch_mcc <= 0x316)
      OR simShrdPrm.mnc_len EQ 3)
  {
    /* 
     * The MCC is in the range 310..316, this means North America.
     * The zero suffix rule applies.
     */
    return ((((sim_mnc & 0xf) EQ 0xf) AND ((bcch_mnc & 0xf) EQ 0x0)) OR
            (((sim_mnc & 0xf) EQ 0x0) AND ((bcch_mnc & 0xf) EQ 0xf)));
  }
  return ((bcch_mnc & 0xf) EQ 0xf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhSIM_plmn_is_hplmn    |
+-------------------------------------------------------------------+

  PURPOSE : Return TRUE if the PLMN received is the HPLMN, otherwise
            return FALSE.

*/

GLOBAL BOOL cmhSIM_plmn_is_hplmn (SHORT bcch_mcc, SHORT bcch_mnc)
{
  SHORT sim_mcc;              /* Holds the MCC of the HPLMN from the SIM */
  SHORT sim_mnc;              /* Holds the MNC of the HPLMN from the SIM */

  TRACE_FUNCTION ("cmhSIM_plmn_is_hplmn()");

  if(!cmhMM_GetActingHPLMN(&sim_mcc, &sim_mnc))/*Enhancement Acting HPLMN*/
  {
    /* Extract the HPLMN identification out of the IMSI digits. */
    cmhSIM_GetHomePLMN (&sim_mcc, &sim_mnc);
  }

  return cmhSIM_plmn_equal_sim (bcch_mcc, bcch_mnc, sim_mcc, sim_mnc);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SIM                 |
|                                 ROUTINE : cmhSIM_GetCmeFromSim    |
+-------------------------------------------------------------------+

  PURPOSE : Mapping of SIM error code to ACI error code.

*/
GLOBAL T_ACI_CME_ERR cmhSIM_GetCmeFromSim ( USHORT errCode )
{
  switch ( errCode )
  {
    case SIM_NO_ERROR:
      return CME_ERR_NotPresent;

    case SIM_CAUSE_PIN1_EXPECT:
      return CME_ERR_SimPinReq;

    case SIM_CAUSE_PIN2_EXPECT:
      return CME_ERR_SimPin2Req;

    case SIM_CAUSE_PUK1_EXPECT:
      return CME_ERR_WrongPasswd;
    case SIM_CAUSE_PIN1_BLOCKED:
      return CME_ERR_SimPukReq;

    case SIM_CAUSE_PUK2_EXPECT:
      return CME_ERR_WrongPasswd;
    case SIM_CAUSE_PIN2_BLOCKED:
      return CME_ERR_SimPuk2Req;

    case SIM_CAUSE_PUK1_BLOCKED:
    case SIM_CAUSE_PUK2_BLOCKED:
      return CME_ERR_SimWrong;

    case SIM_CAUSE_NO_SELECT:
    case SIM_CAUSE_UNKN_FILE_ID:
      return CME_ERR_NotFound;

    case SIM_CAUSE_EF_INVALID:
      return CME_ERR_OpNotSupp;

    case SIM_CAUSE_ADDR_WRONG:
      return CME_ERR_InvIdx;

    case SIM_CAUSE_CMD_INCONSIST:
    case SIM_CAUSE_MAX_INCREASE:
    case SIM_CAUSE_CHV_NOTSET:
    case SIM_CAUSE_CHV_VALIDATED:
      return CME_ERR_OpNotAllow;

    case SIM_CAUSE_ACCESS_PROHIBIT:
      return CME_ERR_WrongPasswd;

    case SIM_CAUSE_CARD_REMOVED:
    case SIM_CAUSE_DRV_NOCARD:
      return CME_ERR_SimNotIns;

    case SIM_CAUSE_CLA_WRONG:
    case SIM_CAUSE_INS_WRONG:
    case SIM_CAUSE_P1P2_WRONG:
    case SIM_CAUSE_P3_WRONG:
    case SIM_CAUSE_PARAM_WRONG:
      return CME_ERR_PhoneFail;

    case SIM_CAUSE_SAT_BUSY:
      return CME_ERR_SimBusy;

    case SIM_CAUSE_DNL_ERROR:
      return CME_ERR_Unknown;

    case SIM_CAUSE_DRV_TEMPFAIL:
      return CME_ERR_SimFail;

    default:
      if (GET_CAUSE_DEFBY(errCode) EQ DEFBY_CONDAT AND
          GET_CAUSE_ORIGSIDE(errCode) EQ ORIGSIDE_MS)
      {
        return CME_ERR_Unknown;
      }
      return CME_ERR_Unknown;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMF           |
| STATE   : code                        ROUTINE : cmhSIM_getUserRate |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BS_SPEED cmhSIM_GetUserRate ( UBYTE userRate )
{
  switch( userRate )
  {
    case ( MNCC_UR_0_3_KBIT        ): return BS_SPEED_300_V110;
    case ( MNCC_UR_1_2_KBIT        ): return BS_SPEED_1200_V110;
    case ( MNCC_UR_2_4_KBIT        ): return BS_SPEED_2400_V110;
    case ( MNCC_UR_4_8_KBIT        ): return BS_SPEED_4800_V110;
    case ( MNCC_UR_9_6_KBIT        ): return BS_SPEED_9600_V110;
    case ( MNCC_UR_1_2_KBIT_V23    ): return BS_SPEED_1200_75_V23;
    case ( MNCC_UR_12_0_KBIT_TRANS ):
    default:                     return BS_SPEED_NotPresent;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : CMH_SIMF              |
| STATE   : code                     ROUTINE : cmhSIM_getSrvFromSync |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_CNUM_SERV cmhSIM_GetSrvFromSync ( UBYTE sync )
{
  switch( sync )
  {
    case ( M_CC_ASYNCHRONOUS ): return CNUM_SERV_Asynch;
    case ( M_CC_SYNCHRONOUS  ): return CNUM_SERV_Synch;
    default:               return CNUM_SERV_NotPresent;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_getSrvFromItc |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_CNUM_SERV cmhSIM_GetSrvFromItc ( UBYTE itc )
{
  switch( itc )
  {
    case ( M_CC_ITC_SPEECH      ): return CNUM_SERV_Voice;
    case ( M_CC_ITC_FAX_GROUP_3 ): return CNUM_SERV_Fax;
    default:                  return CNUM_SERV_NotPresent;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMF           |
| STATE   : code                        ROUTINE : cmhSIM_getUserRate |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_CNUM_ITC cmhSIM_GetItc ( UBYTE itc )
{
  switch( itc )
  {
    case ( M_CC_ITC_DIGITAL_UNRESTRICTED ): return CNUM_ITC_Udi;
    case ( M_CC_ITC_AUDIO                ): return CNUM_ITC_3_1_kHz;
    default:                           return CNUM_ITC_NotPresent;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)          MODULE  : CMH_SIMF               |
| STATE   : code                    ROUTINE : cmhSIM_getMncMccFrmPLMNsel |
+--------------------------------------------------------------------+

  PURPOSE : get MNC and MCC out of EF PLMNsel entry
*/

GLOBAL void cmhSIM_getMncMccFrmPLMNsel(const UBYTE *ntry,
                                             SHORT *mcc,
                                             SHORT *mnc )
{
  if (memcmp (ntry, PLMNselEmpty, sizeof(PLMNselEmpty)) EQ 0)
  {
    *mcc = *mnc = -1;
  }
  else
  {
    *mcc  = ( ntry[0]       & 0x0f) << 8;
    *mcc |= ((ntry[0] >> 4) & 0x0f) << 4;
    *mcc |= ( ntry[1]       & 0x0f);
    *mnc  = ( ntry[2]       & 0x0f) << 8;
    *mnc |= ((ntry[2] >> 4) & 0x0f) << 4;
    *mnc |= ((ntry[1] >> 4) & 0x0f);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)          MODULE  : CMH_SIMF               |
| STATE   : code                    ROUTINE : cmhSIM_GetCodedPLMN    |
+--------------------------------------------------------------------+

  PURPOSE : Code MNC and MCC according a EF PLMNsel entry
*/

GLOBAL BOOL cmhSIM_GetCodedPLMN( const CHAR *oper, T_ACI_CPOL_FRMT format,
                                 UBYTE *sim_plmn )
{
  T_OPER_ENTRY operDesc;     /* operator description */
  BOOL found;

  /* get MNC and MCC */
  switch( format )
  {
    case( CPOL_FRMT_Long ):
      found = cmhMM_FindName( &operDesc, oper, CPOL_FRMT_Long );
      break;
    case( CPOL_FRMT_Short ):
      found = cmhMM_FindName( &operDesc, oper, CPOL_FRMT_Short );
      break;
    case( CPOL_FRMT_Numeric ):
      found = cmhMM_FindNumeric( &operDesc, oper );
      break;
    default:
      return( FALSE );
  }

  if( !found )
    return( FALSE );

  /* code MCC and MNC */
  sim_plmn[0]  = (operDesc.mcc & 0xf00) >> 8;
  sim_plmn[0] |= (operDesc.mcc & 0x0f0)     ;
  sim_plmn[1]  = (operDesc.mcc & 0x00f)     ;
  sim_plmn[1] |= (operDesc.mnc & 0x00f) << 4;
  sim_plmn[2]  = (operDesc.mnc & 0xf00) >> 8;
  sim_plmn[2] |= (operDesc.mnc & 0x0f0)     ;

  return( TRUE );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)          MODULE  : CMH_SIMF               |
| STATE   : code                    ROUTINE : cmhSIM_FillPlmnSelList |
+--------------------------------------------------------------------+

  PURPOSE : Fills in the CPOL list out of EF PLMNsel and returns the
            last written index.
*/
GLOBAL SHORT cmhSIM_FillPlmnSelList ( UBYTE              index,
                                      T_ACI_CPOL_FRMT    frmt,
                                      T_ACI_CPOL_OPDESC* operLst,
                                      UBYTE              length,
                                      UBYTE*             pData   )
{
  UBYTE idx;              /* holds list index */
  SHORT off;              /* holds PLMNsel offset */
  SHORT lastIdx;          /* holds last index written to list */
  SHORT mcc;              /* holds mcc value */
  SHORT mnc;              /* holds mnc value */
  T_OPER_ENTRY OpDsc;     /* operator description */

/*
 *-----------------------------------------------------------------
 * calculate PLMNsel offset
 *-----------------------------------------------------------------
 */
  off = (index-1) * ACI_LEN_PLMN_SEL_NTRY;

/*
 *-----------------------------------------------------------------
 * fill the preferred operator list
 *-----------------------------------------------------------------
 */
  idx = 0;
  lastIdx = ACI_NumParmNotPresent;

  do
  {
    /* get mnc and mcc out of PLMNsel field */
    cmhSIM_getMncMccFrmPLMNsel( (pData+off), &mcc, & mnc );

    /* end of PLMNsel field */
    if( off >= length )
    {
      operLst[idx].index   = ACI_NumParmNotPresent;
      operLst[idx].format  = CPOL_FRMT_NotPresent;
      operLst[idx].oper[0] = 0x0;
      break;
    }

    /* valid entry */
    if( !(mcc < 0 AND mnc < 0) )
    {
      operLst[idx].index  = index;
      operLst[idx].format = frmt;

      cmhMM_FindPLMN( &OpDsc, mcc, mnc, NOT_PRESENT_16BIT, FALSE);

      switch( frmt )
      {
        case( CPOL_FRMT_Long ):
          if (OpDsc.pnn EQ Read_EONS)
          {
            if (OpDsc.long_len)
            {
              switch (OpDsc.long_ext_dcs>>4 & 0x07)
              {
                case 0x00:
                  utl_cvtPnn7To8((UBYTE *)OpDsc.longName,
                                          OpDsc.long_len,
                                          OpDsc.long_ext_dcs,
                                 (UBYTE *)operLst[idx].oper);
                  break;
                case 0x01:
                  TRACE_ERROR ("ERROR: Unhandled UCS2");
                  break;
                default:
                  TRACE_ERROR ("ERROR: Unknown DCS");
                  break;
              }
            }
            else
            {
              operLst[idx].oper[0] = '\0';
            }
          }
          else
          {
            /* MAX_LONG_OPER_LEN <= MAX_ALPHA_OPER_LEN, no length check needed */
            strcpy( operLst[idx].oper, OpDsc.longName );
          } 
          break;

        case( CPOL_FRMT_Short ):
          if (OpDsc.pnn EQ Read_EONS)
          {
            if (OpDsc.shrt_len)
            {
              switch (OpDsc.shrt_ext_dcs>>4 & 0x07)
              {
                case 0x00:
                  utl_cvtPnn7To8((UBYTE *)OpDsc.shrtName,
                                          OpDsc.shrt_len,
                                          OpDsc.shrt_ext_dcs,
                                 (UBYTE *)operLst[idx].oper);
                  break;
                case 0x01:
                  TRACE_ERROR ("ERROR: Unhandled UCS2");
                  break;
                default:
                  TRACE_ERROR ("ERROR: Unknown DCS");
                  break;
              }
            }
            else
            {
              operLst[idx].oper[0] = '\0';
            }
          }
          else
          {
            /* MAX_SHRT_OPER_LEN <= MAX_ALPHA_OPER_LEN, no length check needed */
            strcpy( operLst[idx].oper, OpDsc.shrtName );
          }
          break;

        case( CPOL_FRMT_Numeric ):
          /* Implements Measure#32: Row 1037 & 1036 */
          cmhMM_mcc_mnc_print(&(operLst[idx].oper[0]), mcc, mnc);
          break;
      }
      idx++;
      lastIdx = index;
    }

    off += ACI_LEN_PLMN_SEL_NTRY;
    index++;

  } while( idx < MAX_OPER );

  return( lastIdx );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)          MODULE  : CMH_SIMF               |
| STATE   : code                    ROUTINE : cmhSIM_UsdPlmnSelNtry  |
+--------------------------------------------------------------------+

  PURPOSE : Counts the used entries of the preferred PLMN list and
            returns the number of used entries.
*/
GLOBAL SHORT cmhSIM_UsdPlmnSelNtry ( UBYTE              length,
                                     UBYTE*             pData   )
{
  UBYTE idx;              /* holds list index */
  SHORT off;              /* holds PLMNsel offset */
  UBYTE maxNtry;          /* holds the maximum number of entries */
  SHORT used;             /* holds number of used entries */
  SHORT mcc;              /* holds mcc value */
  SHORT mnc;              /* holds mnc value */

/*
 *-----------------------------------------------------------------
 * count the used entries
 *-----------------------------------------------------------------
 */
  maxNtry = length / ACI_LEN_PLMN_SEL_NTRY;

  for( idx = 0, used = 0, off = 0;
       idx < maxNtry;
       idx++, off += ACI_LEN_PLMN_SEL_NTRY )
  {
    /* get mnc and mcc out of PLMNsel field */
    cmhSIM_getMncMccFrmPLMNsel( (pData+off), &mcc, & mnc );

    /* valid entry */
    if( !(mcc < 0 AND mnc < 0) )
    {
      used++;
    }
  }

  return( used );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)          MODULE  : CMH_SIMF               |
| STATE   : code                    ROUTINE : cmhSIM_CmpctPlmnSel    |
+--------------------------------------------------------------------+

  PURPOSE : Shoves entries of preferred PLMN list to remove empty
            entries of the list.
*/
GLOBAL void cmhSIM_CmpctPlmnSel ( UBYTE length, UBYTE* pData )
{
  UBYTE  maxNtry;          /* holds the maximum number of entries */
  UBYTE  lstIdx;           /* holds list index */
  UBYTE* dstNtry;          /* points to destination entry index */

/*
 *-----------------------------------------------------------------
 * compact the list
 *-----------------------------------------------------------------
 */
  lstIdx  = 0;
  dstNtry = pData;

  maxNtry = length / ACI_LEN_PLMN_SEL_NTRY;

  while( lstIdx < maxNtry )
  {
    if(memcmp( pData, PLMNselEmpty, sizeof(PLMNselEmpty)))
    {
      if( pData NEQ dstNtry )
      {
        memcpy( dstNtry, pData, ACI_LEN_PLMN_SEL_NTRY );
        memcpy( pData, PLMNselEmpty, ACI_LEN_PLMN_SEL_NTRY);
      }
      dstNtry += ACI_LEN_PLMN_SEL_NTRY;
    }
    lstIdx++;
    pData += ACI_LEN_PLMN_SEL_NTRY;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMS           |
| STATE   : code                        ROUTINE : cmhSIM_UpdPlmnSel  |
+--------------------------------------------------------------------+

  PURPOSE : This function updates the indexed EF PLMN SEL entry and
            writes the field to the SIM.
*/
GLOBAL T_ACI_RETURN cmhSIM_UpdPlmnSel ( T_ACI_CMD_SRC srcId,
                                        SHORT index,
                                        UBYTE *plmn,
                                        T_ACI_CPOL_MOD mode )
{
  SHORT off;      /* holds EF offset */
  SHORT cpyOff;   /* holds offset for copy operation */
  UBYTE maxIdx;   /* holds maximum number of index */

  TRACE_FUNCTION ("cmhSIM_UpdPlmnSel()");

/*
 *-----------------------------------------------------------------
 * update EF PLMNsel RAM copy
 *-----------------------------------------------------------------
 */
  maxIdx = CPOLSimEfDataLen / ACI_LEN_PLMN_SEL_NTRY;

  off = (index-1) * ACI_LEN_PLMN_SEL_NTRY;

  if( mode EQ CPOL_MOD_Insert AND index < maxIdx )
  {
    cmhSIM_CmpctPlmnSel ( CPOLSimEfDataLen, CPOLSimEfData );

    cpyOff = (maxIdx-1) * ACI_LEN_PLMN_SEL_NTRY;

    cpyOff -= ACI_LEN_PLMN_SEL_NTRY;  /* need not copy since last index will fall out of list! */

    while( cpyOff >= off AND cpyOff >= 0 )
    {
      memcpy( CPOLSimEfData+cpyOff+ACI_LEN_PLMN_SEL_NTRY,
              CPOLSimEfData+cpyOff, ACI_LEN_PLMN_SEL_NTRY );

      cpyOff -= ACI_LEN_PLMN_SEL_NTRY;
    }
  }

  memcpy( CPOLSimEfData+off, plmn, ACI_LEN_PLMN_SEL_NTRY );
  /* Implements Measure 150 and 159 */
  return ( cmhSIM_Req_or_Write_PlmnSel( srcId, ACT_WR_DAT ) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMS           |
| STATE   : code                        ROUTINE : cmhSIM_FndEmptyPlmnSel  |
+--------------------------------------------------------------------+

  PURPOSE : This function searches for an empty entry in EF PLMN SEL,
            fills it and writes the field to the SIM.
*/
GLOBAL T_ACI_RETURN cmhSIM_FndEmptyPlmnSel ( T_ACI_CMD_SRC srcId,
                                             UBYTE *plmn )
{
  UBYTE maxNtry;  /* holds maximum number of entries */
  SHORT off;      /* holds EF offset */

  TRACE_FUNCTION ("cmhSIM_FndEmptyPlmnSel()");

/*
 *-----------------------------------------------------------------
 * search for an empty entry, and update
 *-----------------------------------------------------------------
 */
  maxNtry = CPOLSimEfDataLen / ACI_LEN_PLMN_SEL_NTRY;

  for( off = 0; maxNtry > 0; off += ACI_LEN_PLMN_SEL_NTRY, maxNtry-- )
  {
    if( !memcmp( CPOLSimEfData+off, PLMNselEmpty,
                 ACI_LEN_PLMN_SEL_NTRY ))
    {
      memcpy( CPOLSimEfData+off, plmn, ACI_LEN_PLMN_SEL_NTRY );
      /* Implements Measure 150 and 159 */
      return ( cmhSIM_Req_or_Write_PlmnSel( srcId, ACT_WR_DAT ) );
    }
  }

  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_MemFull );
  return( AT_FAIL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMS           |
| STATE   : code                        ROUTINE : cmhSIM_DelPlmnSel  |
+--------------------------------------------------------------------+

  PURPOSE : This function updates the indexed EF PLMN SEL entry and
            writes the field to the SIM.
*/
GLOBAL T_ACI_RETURN cmhSIM_DelPlmnSel ( T_ACI_CMD_SRC srcId,
                                        SHORT index,
                                        T_ACI_CPOL_MOD mode )
{
  SHORT off;      /* holds EF offset */

  TRACE_FUNCTION ("cmhSIM_DelPlmnSel()");

/*
 *-----------------------------------------------------------------
 * delete entry in EF PLMNsel RAM copy
 *-----------------------------------------------------------------
 */

  off = (index-1) * ACI_LEN_PLMN_SEL_NTRY;

  memcpy( CPOLSimEfData+off, PLMNselEmpty, ACI_LEN_PLMN_SEL_NTRY );

  if( mode EQ CPOL_MOD_CompactList )

    cmhSIM_CmpctPlmnSel ( CPOLSimEfDataLen, CPOLSimEfData );
  /* Implements Measure 150 and 159 */
  return ( cmhSIM_Req_or_Write_PlmnSel( srcId, ACT_WR_DAT ) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SIMS           |
| STATE   : code                        ROUTINE : cmhSIM_ChgPlmnSel  |
+--------------------------------------------------------------------+

  PURPOSE : This function exchanges the indexed EF PLMN SEL entries and
            writes the field to the SIM.
*/
GLOBAL T_ACI_RETURN cmhSIM_ChgPlmnSel ( T_ACI_CMD_SRC srcId,
                                        SHORT index,
                                        SHORT index2 )
{
  SHORT off1, off2;                   /* holds EF offset */
  UBYTE plmn1[ACI_LEN_PLMN_SEL_NTRY]; /* buffers PLMN 1 */
  UBYTE plmn2[ACI_LEN_PLMN_SEL_NTRY]; /* buffers PLMN 2 */

  TRACE_FUNCTION ("cmhSIM_ChgPlmnSel()");

/*
 *-----------------------------------------------------------------
 * change entries in EF PLMNsel RAM copy
 *-----------------------------------------------------------------
 */

  off1 = (index-1)  * ACI_LEN_PLMN_SEL_NTRY;
  off2 = (index2-1) * ACI_LEN_PLMN_SEL_NTRY;

  memcpy( plmn1, CPOLSimEfData+off1, ACI_LEN_PLMN_SEL_NTRY );
  memcpy( plmn2, CPOLSimEfData+off2, ACI_LEN_PLMN_SEL_NTRY );

  memcpy( CPOLSimEfData+off1, plmn2, ACI_LEN_PLMN_SEL_NTRY );
  memcpy( CPOLSimEfData+off2, plmn1, ACI_LEN_PLMN_SEL_NTRY );
  /* Implements Measure 150 and 159 */
  return ( cmhSIM_Req_or_Write_PlmnSel( srcId, ACT_WR_DAT ) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_WriteTranspEF |
+--------------------------------------------------------------------+

  PURPOSE : This function starts writing of a transparent EF to SIM.
            (SIM only busy with valid 'srcId')
*/
GLOBAL T_ACI_RETURN cmhSIM_WriteTranspEF (T_ACI_CMD_SRC srcId,
                                          T_ACI_AT_CMD  cmd,
                                          BOOL          v_path_info,
                                          T_path_info   *path_info_ptr,
                                          USHORT        datafield,
                                          USHORT        offset,
                                          UBYTE         datalen,
                                          UBYTE       * exchData,
                                          void      (*rplyCB)(SHORT))
{
  /* Implements Measure 158 */
  return ( cmhSIM_Write_or_Read_TranspEF( srcId, cmd, datafield, offset,
                                          datalen,exchData, rplyCB,
                                          ACT_WR_DAT ,v_path_info,path_info_ptr) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  : CMH_SIMF            |
| STATE   : code                       ROUTINE : cmhSIM_ReadTranspEF |
+--------------------------------------------------------------------+

  PURPOSE : This function starts reading of EF PLMN SEL from SIM.
            (SIM only busy with valid 'srcId')
*/
GLOBAL T_ACI_RETURN cmhSIM_ReadTranspEF ( T_ACI_CMD_SRC srcId,
                                          T_ACI_AT_CMD  cmd,
                                          BOOL          v_path_info,
                                          T_path_info   *path_info_ptr,
                                          USHORT        datafield,
                                          USHORT        offset,
                                          UBYTE         explen,
                                          UBYTE       * exchData,
                                          void      (*rplyCB)(SHORT))
{
  /* Implements Measure 158 */
  return ( cmhSIM_Write_or_Read_TranspEF( srcId, cmd, datafield, offset,
                                          explen, exchData, rplyCB,
                                          ACT_RD_DAT,v_path_info,path_info_ptr ) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_WriteRecordEF |
+--------------------------------------------------------------------+

  PURPOSE : This function starts writing of a transparent EF to SIM.
            (SIM only busy with valid 'srcId')
*/
GLOBAL T_ACI_RETURN cmhSIM_WriteRecordEF (T_ACI_CMD_SRC srcId,
                                          T_ACI_AT_CMD  cmd,
                                          BOOL          v_path_info,
                                          T_path_info   *path_info_ptr,
                                          USHORT        datafield,
                                          UBYTE         record,
                                          UBYTE         datalen,
                                          UBYTE       * exchData,
                                          void      (*rplyCB)(SHORT))
{
  /* Implements Measure 168 */
  return ( cmhSIM_Write_or_Read_RecordEF ( srcId, cmd, datafield, record,
                                  datalen, exchData, rplyCB, ACT_WR_REC ,v_path_info,path_info_ptr) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  : CMH_SIMF            |
| STATE   : code                       ROUTINE : cmhSIM_ReadRecordEF |
+--------------------------------------------------------------------+

  PURPOSE : This function starts reading of EF PLMN SEL from SIM.
            (SIM only busy with valid 'srcId')
*/
GLOBAL T_ACI_RETURN cmhSIM_ReadRecordEF ( T_ACI_CMD_SRC srcId,
                                          T_ACI_AT_CMD  cmd,
                                          BOOL          v_path_info,
                                          T_path_info   *path_info_ptr,
                                          USHORT        datafield,
                                          UBYTE         record,
                                          UBYTE         explen,
                                          UBYTE       * exchData,
                                          void      (*rplyCB)(SHORT))
{
  /* Implements Measure 168 */
  return ( cmhSIM_Write_or_Read_RecordEF ( srcId, cmd, datafield, record, explen,
                                  exchData, rplyCB, ACT_RD_REC ,v_path_info,path_info_ptr) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : SimStatusError       |
+--------------------------------------------------------------------+

  PURPOSE : sync_notification: TRUE if notification through asynchrone callback
                               FALSE if notif. whithin synchrone context.

*/
#define GetSIMError (1)
#define CheckSimStatus (0)

LOCAL T_ACI_RETURN SimStatusError ( T_ACI_CMD_SRC srcBuf,
                                    T_ACI_AT_CMD  cmdBuf,
                                    UBYTE         sync_notification)
{
  T_ACI_CPIN_RSLT code = CPIN_RSLT_NotPresent;
  T_ACI_CME_ERR   err  = CME_ERR_NotPresent;

  /* trace if needed... TRACE_EVENT_P1("simShrdPrm.SIMStat: %d", simShrdPrm.SIMStat); */

  switch( simShrdPrm.SIMStat )
  {
    case( SS_OK ):
      if ( qAT_PlusCPIN(CMD_SRC_LCL, &code) EQ AT_CMPL )
      {
        switch ( code )
        {
          case CPIN_RSLT_PhSimPinReq:
            err = CME_ERR_PhSimPinReq;
            break;
          case CPIN_RSLT_SimPinReq:
            err = CME_ERR_SimPinReq;
            break;
          case CPIN_RSLT_SimPin2Req:
            err = CME_ERR_SimPin2Req;
            break;
          case(CPIN_RSLT_PhFSimPinReq): 
            err = CME_ERR_PhFSimPinReq;
            break;
          case(CPIN_RSLT_PhFSimPukReq): 
            err = CME_ERR_PhFSimPukReq;
            break;
          case(CPIN_RSLT_PhNetPinReq): 
            err = CME_ERR_NetworkPersPinReq;
            break;
          case(CPIN_RSLT_PhNetPukReq): 
            err = CME_ERR_NetworkPersPukReq;
            break;
          case(CPIN_RSLT_PhNetSubPinReq): 
            err = CME_ERR_NetworkSubsetPersPinReq;
            break;
          case(CPIN_RSLT_PhNetSubPukReq):
            err = CME_ERR_NetworkSubsetPersPukReq;
            break;
          case(CPIN_RSLT_PhSPPinReq): 
            err = CME_ERR_ProviderPersPinReq;
            break;
          case(CPIN_RSLT_PhSPPukReq): 
            err = CME_ERR_ProviderPersPukReq;
            break;
          case(CPIN_RSLT_PhCorpPinReq):
            err = CME_ERR_CorporatePersPinReq;
            break;
          case(CPIN_RSLT_PhCorpPukReq):
            err = CME_ERR_CorporatePersPukReq;
            break;

        }
      }
      break;

    case( SS_BLKD ):
      if ( qAT_PlusCPIN(CMD_SRC_LCL, &code) EQ AT_CMPL )
      {
        switch ( code )
        {
          case CPIN_RSLT_SimPukReq:
            err = CME_ERR_SimPukReq;
            break;
          case CPIN_RSLT_SimPuk2Req:
            err = CME_ERR_SimPuk2Req;
            break;
        }
      }
      break;

    case( SS_INV ):
      err = CME_ERR_SimWrong;
      break;

    case( SS_URCHB ):
      err = CME_ERR_SimFail;
      break;

    default:            /* unexpected result */
      break;
  }

  if( err EQ CME_ERR_NotPresent )
  {
    return ( AT_FAIL );
  }
  else
  {
    TRACE_EVENT_P1("err: %d", err);
  }

  switch( sync_notification )
  {
  case( GetSIMError ):
    R_AT( RAT_CME, srcBuf ) ( cmdBuf, err );
    break;

  case( CheckSimStatus ):
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, err );
    break;
  }
  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_GetSIMError   |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to request the SIM error status.

*/
GLOBAL T_ACI_RETURN cmhSIM_GetSIMError ( T_ACI_CMD_SRC srcBuf,
                                         T_ACI_AT_CMD cmdBuf )
{
  TRACE_FUNCTION("cmhSIM_GetSIMError");

  return(SimStatusError( srcBuf, cmdBuf,  GetSIMError ));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_GetSIMError   |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to check the SIM pin status.

*/
GLOBAL T_ACI_RETURN cmhSIM_CheckSimPinStatus ( T_ACI_CMD_SRC srcBuf,
                                               T_ACI_AT_CMD cmdBuf )
{
  TRACE_FUNCTION("cmhSIM_CheckSimPinStatus");

  return(SimStatusError( srcBuf, cmdBuf,  CheckSimStatus ));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_CheckSimStatus|
+--------------------------------------------------------------------+

  PURPOSE : This function is used to check the SIM pin status using 
            global parameters.
*/
GLOBAL BOOL cmhSIM_CheckSimStatus ( )
{
  TRACE_FUNCTION("cmhSIM_CheckSimStatus()");

  /*
   *-----------------------------------------------------------------
   * check SIM status
   *-----------------------------------------------------------------
   */

  switch (simShrdPrm.SIMStat)
  {
    case NO_VLD_SS:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimFail );
      break;
    case SS_INV:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimWrong );
      break;
    case SS_URCHB:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimNotIns );
      break;
    case SS_BLKD:
      switch (simShrdPrm.PINStat)
      {
        case PS_PUK1:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPukReq);
          break;

        case PS_PUK2:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPuk2Req);
          break;
      }
      break;
    
    case SS_OK:
      switch (simShrdPrm.PINStat)
      {
        case PS_RDY:
          return (TRUE);

        case PS_PIN1:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPinReq);
          break;

        case PS_PIN2:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPin2Req );
          break;

        case PS_PUK1:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPukReq);
          break;

        case PS_PUK2:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPuk2Req);
          break;
        default:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimFail );
        break;
      }
      break;
    }
  return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)          MODULE  : CMH_SIMF               |
| STATE   : code                    ROUTINE : getSupLangFromPCM      |
+--------------------------------------------------------------------+

  PURPOSE : get Supported Language from PCM and compare it
            with Language table.
*/
GLOBAL T_ACI_RETURN getSupLangFromPCM ( T_ACI_LAN_SUP *lanlst, SHORT *lastIdx)

{
  pcm_FileInfo_Type fileInfo;
  EF_MSSUP          mssup;
  LONG              value;
  SHORT             i, idx=0;
  LONG              bitmask = 0x01;

  TRACE_FUNCTION ("getSupLangFromPCM()");

/*
 *-------------------------------------------------------------------
 *   read supported language from ME
 *-------------------------------------------------------------------
 */
   /* Implements Measure#32: Row 60 & 1039  */
   if (pcm_GetFileInfo ( (UBYTE*)ef_mssup_id, &fileInfo) NEQ PCM_OK)
   {
     TRACE_EVENT("Error getting datas from PCM");
     ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_MemFail );
     return( AT_FAIL );
   }
   else
   {
     /* Implements Measure#32: Row 60 & 1039  */
     if ( pcm_ReadFile ( ( UBYTE* )ef_mssup_id,fileInfo.FileSize,
                         ( UBYTE*) &mssup,
                         &fileInfo.Version) EQ PCM_OK )
     {
       value =mssup.lng1;
       value |=mssup.lng2 <<8;
       value |=mssup.lng3 <<16;

       for(i=0;i<MAX_LAN;i++)
       {
         if (bitmask & value)
         {
           lanlst[idx].lng =(T_ACI_CLAN_LNG)i;
           idx++;
         }

         bitmask= bitmask<< 1;
       }

     }
     else
     {
       ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_MemFail );
       return( AT_FAIL );
     }
   }

/*
 *-------------------------------------------------------------------
 * terminate list and set last index
 *-------------------------------------------------------------------
 */
  if( idx < MAX_LAN )
  {
    lanlst[idx].str = 0x0;
    lanlst[idx].lng = CLAN_LNG_ENG;

  }
  *lastIdx = idx;

/*
 *-------------------------------------------------------------------
 *   compare the code of supported language in PCM with
 *   Language table to get the char code
 *-------------------------------------------------------------------
 */
  for(i=0;i < *lastIdx;i++)
  {
    idx=0;
    while (lngs[idx].str NEQ NULL AND
           lngs[idx].lng NEQ lanlst[i].lng)
      idx++;

    if (lngs[idx].lng EQ lanlst[i].lng)
      lanlst[i].str=lngs[idx].str;
  }

  return( AT_CMPL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)          MODULE  : CMH_SIMF               |
| STATE   : code                    ROUTINE : checkSuppLangInELP     |
+--------------------------------------------------------------------+

  PURPOSE : check if the language to be read from the EF ELP
            is supported in PCM

            SupLng: is true if the language is supprted else False
*/

GLOBAL BOOL checkSuppLang     (T_ACI_LAN_SUP   *lanlst,
                               SHORT           lastIdx,
                               T_ACI_LAN_SUP   *clng)
{
  USHORT i;
  BOOL   SupLng=FALSE;

/*
 *-----------------------------------------------------------------
 *  check if the Language from EF ELP is supported in PCM
 *-----------------------------------------------------------------
 */
  for(i=0;i < lastIdx;i++)
  {
    if (!strcmp(lanlst[i].str,clng->str) )
    {
      clng->lng=lanlst[i].lng;
      SupLng= TRUE;
      break;
    }
  }

  return( SupLng );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)          MODULE  : CMH_SIMF               |
| STATE   : code                    ROUTINE : checkSuppLangInLP      |
+--------------------------------------------------------------------+

  PURPOSE : check if the language to be read from the EF LP
            is supported in PCM

            SupLng: is true if the language is supprted else False
*/

GLOBAL BOOL checkSuppLangInLP(T_ACI_LAN_SUP *lanlst,SHORT lastIdx,
                              T_ACI_LAN_SUP *clng)
{
  USHORT i;
  BOOL   SupLng=FALSE;

/*
 *-----------------------------------------------------------------
 *  check if the Language from EF LP is supported in PCM
 *-----------------------------------------------------------------
 */
  for(i=0;i < lastIdx;i++)
  {
    if (lanlst[i].lng EQ clng->lng )
    {
      clng->str=lanlst[i].str;
      SupLng= TRUE;
      break;
    }
  }

  return( SupLng );
}

#if 0
/*
+------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  : CMH_SIMS                |
| STATE   : code                       ROUTINE : cmhSIM_LanguageLP_Update|
+------------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL BOOL cmhSIM_LanguageLP_Update  ( int ref, T_SIM_FILE_UPDATE_IND *fu)
{
  T_ACI_CMD_SRC ownBuf;     /* buffers current owner */
  UBYTE i;
  BOOL found = FALSE;

  char *auptr="au";
  CHAR *ef = EF_CLNG_ID;
  pcm_FileInfo_Type fileInfo;
  EF_CLNG lng;


  TRACE_FUNCTION ("cmhSIM_LanguageLP_Update()");

  ownBuf     = simEntStat.entOwn;

  for (i = 0; i < (int)fu->val_nr; i++)
  {
    if (!found AND
        (fu->file_info[i].datafiled EQ SIM_LP))
    {
      found = TRUE;
    }
  }

  if (found)
  {
    /*
     *-------------------------------------------------------------------
     *   read supported language from ME
     *-------------------------------------------------------------------
     */
     if (pcm_GetFileInfo ( ( UBYTE* ) ef, &fileInfo) NEQ PCM_OK)
     {
       TRACE_EVENT("cmhSIM_LanguageLP_Update: error returned by pcm_GetFileInfo()" );
       return TRUE;
     }
     else
     {

       if ( pcm_ReadFile ( (UBYTE*)ef,
                           fileInfo.FileSize,
                           (UBYTE*) &lng,
                           &fileInfo.Version) EQ PCM_OK )
       {
       }
       else
       {
         TRACE_EVENT("cmhSIM_LanguageLP_Update: error returned by pcm_ReadFile()" );
         return TRUE;
       }
     }

    /*
     *-------------------------------------------------------------------
     *  Read EF LP from the sim if Automatic language is selected
     *-------------------------------------------------------------------
     */
    if (!strncmp((char*)&lng.data[0], auptr, 2))
    {
      cmhSIM_ReqLanguageLP(ownBuf);  /* reading files */
      simShrdPrm.fuRef = (UBYTE)ref;
      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }
  else
  {
    return TRUE;  /* nothing to do */
  }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_AD_Update     |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to request the SIM Administrative
            Data (EF_AD).

*/
GLOBAL BOOL cmhSIM_AD_Update (int ref, T_SIM_FILE_UPDATE_IND *fu)
{
  /* Implements Measure 42 */
  return ( cmhSIM_AD_CSP_Update( ref, fu, SIM_AD ) );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_Read_AD       |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to request the SIM Administrative
            Data (EF_AD).

*/
GLOBAL T_ACI_RETURN cmhSIM_Read_AD()
{
  SHORT table_id;

  TRACE_FUNCTION ("cmhSIM_Read_AD()");

  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
    simShrdPrm.atb[table_id].accType    = ACT_RD_DAT;
    simShrdPrm.atb[table_id].v_path_info = FALSE;
    simShrdPrm.atb[table_id].reqDataFld = SIM_AD;
    simShrdPrm.atb[table_id].dataOff    = 0;
    simShrdPrm.atb[table_id].dataLen    = NOT_PRESENT_8BIT;
    simShrdPrm.atb[table_id].recMax     = 0;
    simShrdPrm.atb[table_id].exchData   = NULL;
    simShrdPrm.atb[table_id].rplyCB     = cmhSIM_Read_AD_cb;

    simShrdPrm.aId = table_id;

    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR");
      return ( AT_FAIL );
    }

    return ( AT_CMPL );
  }
  return ( AT_FAIL );
}

/*
+-------------------------------------------------------------------+
| PROJECT :                  MODULE  : CMH_SIMF                     |
| STATE   : code             ROUTINE : cmhSIM_EvalMNCLength         |
+-------------------------------------------------------------------+

  PURPOSE : This function evaluates the MNC length by extracting MNC 
            from IMSI and comparing it with the MNC in operListFixed.
*/

#ifdef TI_PS_OP_OPN_TAB_ROMBASED
GLOBAL  UBYTE cmhSIM_EvalMNCLength(void)
{
  UBYTE digit;        /* holds 4bit digit */
  USHORT i;           /* holds counter */
  SHORT sim_mcc;      /* holds mcc from operListFixed */
  SHORT sim_mnc;      /* holds mnc from operListFixed */
  SHORT mcc;          /* holds mcc extracted from IMSI */
  SHORT mnc;          /* holds mnc extracted from IMSI */

  /* Changes for ROM data */ 
  const UBYTE *plmn_comp_entry; /* get a local pointer holder */
  T_OPER_ENTRY oper;

  mcc = mnc = 0;      /* Initialize mcc, mnc */
  
  for (i = 0; i < SIZE_MCC + SIZE_MNC; i++)   /* Extract MCC and MNC. */
  {
    digit = (i & 1) ?
            (simShrdPrm.imsi.field[(i + 1)/2] & 0x0f) :
            (simShrdPrm.imsi.field[(i + 1)/2] & 0xf0) >> 4;
    if (i < SIZE_MCC)
    {
      mcc = (mcc << 4) | digit;
    }
    else
    {
      mnc = (mnc << 4) | digit;
    }
  }

  /* Changes for ROM data */ 
  plmn_comp_entry = ptr_plmn_compressed;

  /* Get first compressed PLMN entry */
  while (cmhMM_decodePlmn (&oper, plmn_comp_entry) EQ 0)
  {
    sim_mcc = oper.mcc;
    sim_mnc = oper.mnc;
    if ( sim_mcc EQ mcc )
    {
      if ( (sim_mnc & 0xff0) EQ (mnc & 0xff0) )
      {
        if ( (sim_mnc & 0x0f) EQ 0x0f )
        {
          return 2;
        }
        else 
        {
          return 3;
        }
      }
    }
      /* Next compressed PLMN entry */
      plmn_comp_entry += cmhMM_PlmnEntryLength (plmn_comp_entry);   
  }
  return NOT_PRESENT_8BIT;
}

#else

GLOBAL  UBYTE cmhSIM_EvalMNCLength(void)
{
  UBYTE digit;        /* holds 4bit digit */
  USHORT i;           /* holds counter */
  SHORT sim_mcc;      /* holds mcc from operListFixed */
  SHORT sim_mnc;      /* holds mnc from operListFixed */
  SHORT mcc;          /* holds mcc extracted from IMSI */
  SHORT mnc;          /* holds mnc extracted from IMSI */

  mcc = mnc = 0;      /* Initialize mcc, mnc */
  
  for (i = 0; i < SIZE_MCC + SIZE_MNC; i++)   /* Extract MCC and MNC. */
  {
    digit = (i & 1) ?
      (simShrdPrm.imsi.field[(i + 1)/2] & 0x0f) :
      (simShrdPrm.imsi.field[(i + 1)/2] & 0xf0) >> 4;
    if (i < SIZE_MCC)
       mcc = (mcc << 4) | digit;
    else
       mnc = (mnc << 4) | digit;
  }
  
  for( i = 0; operListFixed[i].mcc NEQ -1 AND operListFixed[i].mnc NEQ -1; i++ ) /* Evaluate mnc length */
  {
    sim_mcc = operListFixed[i].mcc;
    sim_mnc = operListFixed[i].mnc;

    if ( sim_mcc EQ mcc )
    {
      if ( (sim_mnc & 0xff0) EQ (mnc & 0xff0) )
      {
        if ( (sim_mnc & 0x0f) EQ 0x0f )
        {
          return 2;
        }
        else 
        {
          return 3;
        }
      }
    }
  }
  return NOT_PRESENT_8BIT;
}

#endif


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_Read_AD_cb    |
+--------------------------------------------------------------------+

  PURPOSE :   Call back for SIM read (EF_AD).

*/
void cmhSIM_Read_AD_cb(SHORT table_id)
{
  TRACE_FUNCTION ("cmhSIM_Read_AD_cb()");

  if ( simShrdPrm.atb[table_id].reqDataFld EQ SIM_AD )
  {
    if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR )
    {
      cmhSIM_AD_Updated(simShrdPrm.atb[table_id].dataLen, simShrdPrm.atb[table_id].exchData);
    }
#ifdef SIM_TOOLKIT
    if (simShrdPrm.fuRef >= 0)
    {
      psaSAT_FUConfirm (simShrdPrm.fuRef,
                        (USHORT)((simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR)?
                        SIM_FU_SUCCESS: SIM_FU_ERROR));
    }
#endif
  }
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_Access_AD     |
+--------------------------------------------------------------------+

  PURPOSE :   Access the AD data

*/
GLOBAL void cmhSIM_AD_Updated(UBYTE ad_len, UBYTE* ad_data)
{
   TRACE_FUNCTION("cmhSIM_AD_Updated()");
   /* 
    * byte 2 and byte 3 of EF AD have only a meaning 
    * if the bit1 of byte 1 is set 
    */
    if (ad_data[0] & 0x01)
    {
      if (ad_data[2] & 0x01)
      {
        /* ci enabled on SIM */
         simShrdPrm.ciSIMEnabled = TRUE;
      }
      else
      {
        /* ci disabled on SIM, don't show indications */
        simShrdPrm.ciSIMEnabled = FALSE;
      }
    }
    /* byte 4 is optional */
    if (ad_len >= 4)
    {
      switch (ad_data[3] & 0x0F)
      {
        case 2:  simShrdPrm.mnc_len = 2; break;
        case 3:  simShrdPrm.mnc_len = 3; break;
        default: simShrdPrm.mnc_len =NOT_PRESENT_8BIT;
      }
    }
    else
    {
      simShrdPrm.mnc_len = NOT_PRESENT_8BIT;
    }
    if (simShrdPrm.mnc_len NEQ 3)        /* Update MNC length */
    {
      simShrdPrm.mnc_len = cmhSIM_EvalMNCLength();
    }
} 

/*
+--------------------------------------------------------------------+
| PROJECT : EONS        MODULE  : CMH_SIMF                           |
| STATE   : code        ROUTINE : cmhSIM_OpUpdate                    |
+--------------------------------------------------------------------+

  PURPOSE : This function will be used to process the update of EFopl and EFpnn.
         
*/
GLOBAL BOOL cmhSIM_OpUpdate (int ref, T_SIM_FILE_UPDATE_IND *fu)
{
  UBYTE i;
  BOOL  ps_is_not_reading_files_1 = FALSE, 
        ps_is_not_reading_files_2 = FALSE;
  
  TRACE_FUNCTION ("cmhSIM_OpUpdate()");

  for (i = 0; i < (int)fu->val_nr; i++)
  {
    if(fu->file_info[i].v_path_info EQ TRUE   AND
       fu->file_info[i].path_info.df_level1   EQ SIM_DF_GSM   AND
       fu->file_info[i].path_info.v_df_level2 EQ FALSE)
    {
      switch(fu->file_info[i].datafield)
      {
        case SIM_OPL:
          TRACE_EVENT("EF_OPL has been updated ");
          ps_is_not_reading_files_1 = cmhSIM_UpdateOperatorName(SIM_OPL);
          /*lint -fallthrough */

        case SIM_PNN:
          if(fu->file_info[i].datafield NEQ SIM_OPL)
          {
            TRACE_EVENT("EF_PNN has been updated ");
          }
          ps_is_not_reading_files_2 = !cmhMM_OpUpdateName();
          cmhMM_Registered();
          simShrdPrm.fuRef = (UBYTE)ref;
          if(ps_is_not_reading_files_1 OR
            ps_is_not_reading_files_2)
          {
            return(TRUE);
          }
        
          return(FALSE); /* reading files ? */

        default:
          break;
      }
    }
  }
  return(TRUE);  /* nothing to do */
}

/*
+-------------------------------------------------------------------+
| PROJECT : EONS             MODULE  : CMH_SIMF                     |
| STATE   : code             ROUTINE : cmhSIM_BuildOPLList          |
+-------------------------------------------------------------------+

  PURPOSE : 
*/
LOCAL void cmhSIM_BuildOPLList(SHORT table_id)
{
  UBYTE *data = simShrdPrm.atb[table_id].exchData;
  T_opl *opl_entry = &simShrdPrm.opl_list.opl_rcd[simShrdPrm.opl_list.num_rcd];

  /* Test if record is valid */
/*  opl_entry->plmn.v_plmn = (data[0] EQ 0xFF) ? INVLD_PLMN : VLD_PLMN; */

  /* Copy MCC and MNC from SIM data to OPL list */
  memcpy (opl_entry->plmn, data, UBYTES_PER_PLMN);

  /* Extract LAC from SIM data and copy to OPL list*/
  opl_entry->lac1        = data[3] << 8 | data[4];
  opl_entry->lac2        = data[5] << 8 | data[6];

  /* Extract PNN record number from SIM data and copy to OPL list*/
  opl_entry->pnn_rec_num = data[7];
}


/*
+-------------------------------------------------------------------+
| PROJECT : EONS             MODULE  : CMH_SIMF                     |
| STATE   : code             ROUTINE : cmhSIM_OpReadOplRcdCb        |
+-------------------------------------------------------------------+

  PURPOSE : Call back for SIM retrieval of EF_OPL.
*/
GLOBAL void cmhSIM_OpReadOplRcdCb(SHORT table_id)
{
  TRACE_FUNCTION("cmhSIM_OpReadOplRcdCb");

  /* Implements Measure 120 */
  cmhSIM_OpRead_simDataFld_RcdCb( table_id, SIM_OPL );
}



/*
+-------------------------------------------------------------------+
| PROJECT : EONS             MODULE  : CMH_SIMF                     |
| STATE   : code             ROUTINE : cmhSIM_BuildPnnList          |
+-------------------------------------------------------------------+

  PURPOSE : decodes EF_PNN record read from SIM
*/
LOCAL void cmhSIM_BuildPnnList(SHORT table_id)
{
  UBYTE *data = simShrdPrm.atb[table_id].exchData;
  T_pnn *pnn_entry = &simShrdPrm.pnn_list.pnn_rcd[simShrdPrm.pnn_list.num_rcd];

  if (*data++ EQ PNN_LONG_NAME_IEI)
  {
    pnn_entry->v_plmn = TRUE;

    pnn_entry->long_len = (*data++)-1;  /* adjust dcs */
    pnn_entry->long_ext_dcs = *data++;
    memcpy(pnn_entry->long_name,
           data,
           MINIMUM(pnn_entry->long_len, sizeof(pnn_entry->long_name)));
    data += pnn_entry->long_len;
  
     /*----- IEI PNN short name ------*/
    if (*data++ EQ PNN_SHORT_NAME_IEI)
    {
      pnn_entry->shrt_len = (*data++)-1; /* adjust dcs */
      pnn_entry->shrt_ext_dcs = *data++;
      memcpy(pnn_entry->shrt_name,
             data, 
             MINIMUM(pnn_entry->shrt_len, sizeof(pnn_entry->shrt_name)));
    } 
    else
    {
      pnn_entry->shrt_len = 0;
    }
  }
  else
  {
    /* marc record as unused */
    pnn_entry->v_plmn = FALSE;
    pnn_entry->long_len = pnn_entry->shrt_len = 0;
  }

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}


/*
+-------------------------------------------------------------------+
| PROJECT : EONS             MODULE  : CMH_SIMF                     |
| STATE   : code             ROUTINE : cmhSIM_OpReadPnnRcdCb        |
+-------------------------------------------------------------------+

  PURPOSE : Call back for SIM retrieval of EF_PNN.
*/
GLOBAL void cmhSIM_OpReadPnnRcdCb(SHORT table_id)
{

  TRACE_FUNCTION("cmhSIM_OpReadPnnRcdCb");
  /* Implements Measure 120 */
  cmhSIM_OpRead_simDataFld_RcdCb( table_id, SIM_PNN );
}


/*
+-------------------------------------------------------------------+
| PROJECT : EONS             MODULE  : CMH_SIMF                     |
| STATE   : code             ROUTINE : cmhSIM_StartOperatorName     |
+-------------------------------------------------------------------+

  PURPOSE : Start retireval of EF_OPL from SIM
*/
GLOBAL BOOL cmhSIM_StartOperatorName()
{
  TRACE_FUNCTION ("cmhSIM_StartOperatorName()");

  /* If EF_PNN and/or EF_OPL are allocated then start retrieval of from EF_OPL */
  if (simShrdPrm.opl_list.num_rcd EQ 0 AND psaSIM_ChkSIMSrvSup(SRV_PNN) AND psaSIM_ChkSIMSrvSup(SRV_OPL))
  {
    TRACE_EVENT (" start reading SIM_OPL");
    /* Implements Measure 135 */
    return ( !cmhSIM_OpRead_Opl_or_Pnn_Rcd(1, SIM_OPL, cmhSIM_OpReadOplRcdCb));
  }

  if (simShrdPrm.pnn_list.num_rcd EQ 0 AND psaSIM_ChkSIMSrvSup(SRV_PNN))
  {
    TRACE_EVENT (" start reading SIM_PNN");
    /* Implements Measure 135 */
    return ( !cmhSIM_OpRead_Opl_or_Pnn_Rcd(1, SIM_PNN, cmhSIM_OpReadPnnRcdCb));
  }

  TRACE_EVENT (" reading finished!");

  if(psaSIM_ChkSIMSrvSup(SRV_PNN))
  {
    if (simShrdPrm.pnn_list.pnn_status EQ TRUE)
    {
      percentCSTAT_indication(STATE_MSG_EONS, ENTITY_STATUS_Ready);
    }
    else if (psaSIM_ChkSIMSrvSup(SRV_OPL) AND simShrdPrm.opl_list.opl_status EQ TRUE)
    {
      percentCSTAT_indication(STATE_MSG_EONS, ENTITY_STATUS_Ready);
    }
    else
    {
      percentCSTAT_indication(STATE_MSG_EONS, ENTITY_STATUS_NotReady);
    }
  }
  else
  {
    percentCSTAT_indication(STATE_MSG_EONS, ENTITY_STATUS_NotReady);
  }
  if (mmShrdPrm.regStat EQ RS_FULL_SRV)
  {
    cmhMM_Registered();
  }

  return(TRUE);
}


/*
+-------------------------------------------------------------------+
| PROJECT : EONS             MODULE  : CMH_SIMF                     |
| STATE   : code             ROUTINE : cmhSIM_UpdateOperatorName    |
+-------------------------------------------------------------------+

  PURPOSE : Start File Update of EF_OPL from SIM
*/
GLOBAL BOOL cmhSIM_UpdateOperatorName(USHORT reqDataFld)
{
  int i;
  TRACE_FUNCTION ("cmhSIM_UpdateOperatorName()");

  if (reqDataFld EQ SIM_OPL OR reqDataFld EQ NOT_PRESENT_16BIT)
  {
    simShrdPrm.opl_list.num_rcd = 0;
    for (i=0; i<OPL_MAX_RECORDS; i++)
    {
      memcpy (simShrdPrm.opl_list.opl_rcd[i].plmn, 
              PLMNselEmpty,
              UBYTES_PER_PLMN);
    }
  }

  if (reqDataFld EQ SIM_PNN OR reqDataFld EQ NOT_PRESENT_16BIT)
  {
    simShrdPrm.pnn_list.num_rcd = 0;
    for (i=0; i<PNN_MAX_RECORDS; i++)
    {
      simShrdPrm.pnn_list.pnn_rcd[i].v_plmn=FALSE;
    }
  }

  return (!cmhSIM_StartOperatorName());
}


GLOBAL T_opl_field* cmhSIM_GetOPL()
{
  return &simShrdPrm.opl_list;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_Read_AD       |
+--------------------------------------------------------------------+

  PURPOSE : This function returns the current PLMN mode bit value

*/
GLOBAL UBYTE cmhSIM_isplmnmodebit_set()
{
  return simShrdPrm.PLMN_Mode_Bit;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_ONS_Update    |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to Read the CPHS ONS file EF_ONS.

*/

GLOBAL BOOL cmhSIM_ONS_Update (int ref, T_SIM_FILE_UPDATE_IND *fu)
{
  BOOL found = FALSE;
  UBYTE i;
  TRACE_FUNCTION ("cmhSIM_ONS_Update()");

  for (i = 0; i < (int)fu->val_nr; i++)
  {
    if (fu->file_info[i].v_path_info EQ TRUE       AND
            fu->file_info[i].path_info.df_level1   EQ SIM_DF_GSM AND
            fu->file_info[i].path_info.v_df_level2 EQ FALSE AND 
            fu->file_info[i].datafield EQ SIM_CPHS_ONSTR)
    {
      found = TRUE;
      break;
    }
  }
  if (found)
  {
    cmhMM_ONSReadName();
    simShrdPrm.fuRef = (UBYTE)ref;
    return FALSE; /* reading files */
  }
  else
  {
    return TRUE;  /* nothing to do */
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_Get_CSP       |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to request the SIM CPHS file

*/
GLOBAL void cmhSIM_Get_CSP()
{
  
  TRACE_FUNCTION ("cmhSIM_Get_CSP()");

  cmhSIM_ReadTranspEF( CMD_SRC_NONE,
                       AT_CMD_NONE,
                       FALSE,
                       NULL,
                       SIM_CPHS_CINF,
                       0,
                       ACI_CPHS_INFO_SIZE,
                       NULL,
                       cmhSIM_Get_CSP_cb );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_Get_CSP_cb    |
+--------------------------------------------------------------------+

  PURPOSE :   Call back for SIM read for CPHS.

*/
void cmhSIM_Get_CSP_cb(SHORT table_id)
{
  
  TRACE_FUNCTION ("cmhSIM_Get_CSP_cb()");

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  /* Step #1: Reading of CPHS Info */
  if (simShrdPrm.atb[table_id].errCode NEQ SIM_NO_ERROR OR
      simShrdPrm.atb[table_id].exchData EQ NULL         OR
      simShrdPrm.atb[table_id].dataLen < ACI_CPHS_INFO_SIZE )
  {
    ;  /* CPHS info not supported or invalid */
  }
  else if ((simShrdPrm.atb[table_id].exchData[1] & 0x03) EQ ALLOCATED_AND_ACTIVATED)
  {
    /* continue with reading of CSP file */
    cmhSIM_Read_CSP();
    return;
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_AD_Update     |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to request the SIM Customer Service Profile (EF_CPHS_CSP).

*/
GLOBAL BOOL cmhSIM_CSP_Update (int ref, T_SIM_FILE_UPDATE_IND *fu)
{
  /* Implements Measure 42 */
  return ( cmhSIM_AD_CSP_Update( ref, fu, SIM_CPHS_CSP ) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_Read_CSP       |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to request the SIM Customer Service Profile
            (EF_CSP).

*/
GLOBAL void cmhSIM_Read_CSP()
{
  
  TRACE_FUNCTION ("cmhSIM_Read_CSP()");

  cmhSIM_ReadTranspEF( CMD_SRC_NONE,
                       AT_CMD_NONE,
                       FALSE,
                       NULL,
                       SIM_CPHS_CSP,
                       0,
                       0,
                       NULL,
                       cmhSIM_Read_CSP_cb );

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_Read_CSP_cb    |
+--------------------------------------------------------------------+

  PURPOSE :   Call back for SIM read (EF_CSP).

*/
void cmhSIM_Read_CSP_cb(SHORT table_id)
{
  
  TRACE_FUNCTION ("cmhSIM_Read_CSP_cb()");

  if ( simShrdPrm.atb[table_id].reqDataFld EQ SIM_CPHS_CSP )
  {
    if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR )
    {
      if(simShrdPrm.atb[table_id].dataLen >= ACI_CPHS_CSP_SIZE)
      {
        USHORT idx;
        for(idx=0; idx < simShrdPrm.atb[table_id].dataLen; idx = idx+2)
        {
         if(simShrdPrm.atb[table_id].exchData[idx] EQ VAS_SERVICE_GROUP_CODE)
         {
           if(simShrdPrm.atb[table_id].exchData[idx+1] & PLMN_MODE_BIT_ON)
           {
             simShrdPrm.PLMN_Mode_Bit = CPHS_CSP_PLMN_MODE_BIT_ON;
           }
           else
           {
             simShrdPrm.PLMN_Mode_Bit = CPHS_CSP_PLMN_MODE_BIT_OFF;
           }
           break;
         }
        }
      }
    }
    
#ifdef SIM_TOOLKIT
    if (simShrdPrm.fuRef >= 0)
    {
      psaSAT_FUConfirm (simShrdPrm.fuRef,
                        (USHORT)((simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR)?
                        SIM_FU_SUCCESS: SIM_FU_ERROR));
    }
#endif
  }
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}
#ifdef SIM_PERS_OTA

/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_Register_Read_DCK
+------------------------------------------------------------------------------
|  Description : This function is used to request the SIM De-personalization Control Keys
|            (EF_DCK)
|  Parameters  : ref      - Reference for file update
|                       *fu     - Pointer to T_SIM_FILE_UPDATE_IND
|                
|  Return      : TRUE if no files left to read, 
|                    FALSE if some files are there to read
|
+------------------------------------------------------------------------------
*/

GLOBAL BOOL cmhSIM_Register_Read_DCK (int ref, T_SIM_FILE_UPDATE_IND *fu)
{
  UBYTE i;

  TRACE_FUNCTION ("cmhSIM_Register_Read_DCK()");

  for (i = 0; i < (int)fu->val_nr AND (fu->file_info[i].datafield NEQ SIM_DCK); i++)
  {
  }

  if (i NEQ (int)fu->val_nr )
  {
  
     if (psaSIM_ChkSIMSrvSup(SRV_DePersCK))
    {
      TRACE_FUNCTION("SRV_DePersCK supported.");
      cmhSIM_ReadTranspEF( CMD_SRC_NONE,
               AT_CMD_NONE,
               FALSE,
               NULL,
               SIM_DCK,
               0,
               16,
               NULL,
               cmhSIM_Read_DCK_cb );
    }
    simShrdPrm.fuRef = (UBYTE)ref;
    return FALSE; /* reading files */
  }
  else
  {
    return TRUE;  /* nothing to do */
  }
}




/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_Read_DCK_cb
+------------------------------------------------------------------------------
|  Description : Call back for SIM read (EF_DCK).
|  Parameters  : SHORT table_id
|                
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void cmhSIM_Read_DCK_cb(SHORT table_id)
{
  TRACE_FUNCTION ("cmhSIM_Read_DCK_cb()");

  if ( simShrdPrm.atb[table_id].reqDataFld EQ SIM_DCK )
  {
    if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR )
    {
      aci_slock_psaSIM_decodeIMSI_without_parity (&simShrdPrm.atb[table_id].exchData[0], 8, (char *) nw_ctrl_key);
      aci_slock_psaSIM_decodeIMSI_without_parity (&simShrdPrm.atb[table_id].exchData[4], 8, (char *) nw_subset_ctrl_key);
      aci_slock_psaSIM_decodeIMSI_without_parity (&simShrdPrm.atb[table_id].exchData[8], 8, (char *) sp_ctrl_key);
      aci_slock_psaSIM_decodeIMSI_without_parity (&simShrdPrm.atb[table_id].exchData[12], 8, (char *) corp_ctrl_key);

      aci_slock_unlock(SIMLOCK_NETWORK, (char *) nw_ctrl_key);
      aci_slock_unlock(SIMLOCK_NETWORK_SUBSET, (char *) nw_subset_ctrl_key);
      aci_slock_unlock(SIMLOCK_SERVICE_PROVIDER, (char *) sp_ctrl_key);
      aci_slock_unlock(SIMLOCK_CORPORATE, (char *) corp_ctrl_key);
      cmhSIM_WriteDefaultValue_DCK();
    }
  }
}


/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_WriteDefaultValue_DCK
+------------------------------------------------------------------------------
|  Description : This function is used to write back the SIM De-personalization Control Keys
|            (EF_DCK) read during init after depersonalizing the ME
|
|  Parameters  : none
|                
|  Return      : void
|
+------------------------------------------------------------------------------
*/
GLOBAL void cmhSIM_WriteDefaultValue_DCK()
{
  UBYTE all_keys[MAX_DCK_LEN];

  TRACE_FUNCTION ("cmhSIM_WriteDefaultValue_DCK()");
  memset(all_keys,NOT_PRESENT_8BIT,MAX_DCK_LEN);
  cmhSIM_WriteTranspEF( CMD_SRC_NONE,
                        AT_CMD_NONE,
                        FALSE,
                        NULL,
                        SIM_DCK,
                        0,
                        MAX_DCK_LEN,
                        all_keys  ,
                        NULL );
}


#endif /* SIM_PERS_OTA */

#ifdef SIM_PERS
/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_UnlockTimeout
+------------------------------------------------------------------------------
|  Description : This function handles the Penalty timout.
|
|  Parameters  : none
|                
|  Return      : void
|
+------------------------------------------------------------------------------
*/
GLOBAL void cmhSIM_UnlockTimeout(void)
{
  aci_slock_unlock_timer_stopped();   
}
#endif

#ifdef FF_CPHS_REL4
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_WrCfis        |
+--------------------------------------------------------------------+

  PURPOSE :   Writing data to EF-CFIS in SIM

*/

GLOBAL T_ACI_RETURN cmhSIM_WrCfis (T_ACI_CMD_SRC srcId,
                                   T_ACI_CFIS_MOD mode, UBYTE index,
                                   UBYTE mspId,
                                   UBYTE cfuStat,CHAR* number, 
                                   T_ACI_TOA* type,
                                   UBYTE cc2_id)
{
  T_ACI_RETURN ret = AT_FAIL;
  /* data send to SIM */
  UBYTE *exchData ;
  T_ACI_TOA        toa_type;

  TRACE_FUNCTION ("cmhSIM_WrCfis()");
 
  MALLOC (exchData,ACI_SIZE_EF_CFIS);

  /* reset exchData */
  memset(exchData, 0xFF,ACI_SIZE_EF_CFIS );

  if( mode EQ CFIS_MOD_Write )
  {
    *exchData= mspId;
    *(exchData+1) = cfuStat;
    cmhPHB_getAdrBcd ( exchData+4, exchData+2,
                       MAX_PHB_NUM_LEN, number );
    if(type NEQ NULL)
    {
      memcpy(&toa_type,type,sizeof(toa_type));
    }
    else
    {
      toa_type.ton = TON_Unknown;
      toa_type.npi = NPI_IsdnTelephony;
       if ( number[0] EQ '+' )
       {
         toa_type.ton = TON_International;
       }
    }
    cmhPHB_toaMrg(&toa_type, exchData+3 );

    /* capability/configuration identifier data and EXT identifier data */
    *(exchData+14)      = cc2_id;
    *(exchData + 15) = 0xFF;
  }
  else
  {
    *(exchData+1) = 0x00;
  }
 /* write to SIM */    
  ret = cmhSIM_WriteRecordEF( srcId,AT_CMD_P_CFIS,SIM_CFIS,index,
                              ACI_SIZE_EF_CFIS,exchData,
                              cmhSIM_WrCnfCfis);
  MFREE(exchData);
  return ( ret );

}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_WrMwis        |
+--------------------------------------------------------------------+

  PURPOSE :   Writing data to EF-MWIS in SIM

*/

GLOBAL T_ACI_RETURN cmhSIM_WrMwis (T_ACI_CMD_SRC srcId,
                                   T_ACI_MWIS_MOD mode,
                                   UBYTE mspId,
                                   T_ACI_MWIS_MWI *mwis)
{
  T_ACI_RETURN ret = AT_FAIL;
  /* data send to SIM */
  UBYTE *exchData ;

  TRACE_FUNCTION ("cmhSIM_WrMwis()");

  MALLOC (exchData,ACI_SIZE_EF_MWIS);

  /* reset exchData */
  memset(exchData, 0xFF,ACI_SIZE_EF_MWIS );

  if( mode EQ MWIS_MOD_Write )
  {
    *exchData= mwis->mwiStat;
    *(exchData+1) = mwis->mwis_count_voice;
    *(exchData+2) = mwis->mwis_count_fax;
    *(exchData+3) = mwis->mwis_count_email;
    *(exchData+4) = mwis->mwis_count_other;
  }
  else
  {
    *exchData = 0x00;
  }
 /* write to SIM */
  ret = cmhSIM_WriteRecordEF( srcId,AT_CMD_P_MWIS,SIM_MWIS,mspId,
                              ACI_SIZE_EF_MWIS,exchData,
                              cmhSIM_WrCnfMwis);
  MFREE(exchData);
  return ( ret );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : CMH_SIMF             |
| STATE   : code                      ROUTINE : cmhSIM_WrMbdn        |
+--------------------------------------------------------------------+

  PURPOSE :   Writing data to EF-MBDN in SIM

*/

GLOBAL T_ACI_RETURN cmhSIM_WrMbdn (T_ACI_CMD_SRC srcId,
                                   T_ACI_MBN_MODE mode, UBYTE index,
                                   CHAR* number, T_ACI_TOA* type,
                                   UBYTE cc2_id, T_ACI_PB_TEXT *text)
{
  T_ACI_RETURN ret = AT_FAIL;
  /* data send to SIM */
  UBYTE *exchData;
  T_ACI_TOA        toa_type;
  UBYTE   tag_len = 0;                /* Length of Alpha identifier          */
  UBYTE   tag[PHB_MAX_TAG_LEN];       /* Alpha identifier                    */


  TRACE_FUNCTION ("cmhSIM_WrMbdn()");
 
  MALLOC (exchData,ACI_SIZE_EF_MBDN);

  /* reset exchData */
  memset(exchData, 0xFF,ACI_SIZE_EF_MBDN );

  if( mode EQ MBN_Mode_Write )
  {
    if (text NEQ NULL)
    {
      cmhPHB_getMfwTagSim ( text, (UBYTE*)tag, &tag_len, 
                            PHB_MAX_TAG_LEN );
      memcpy(exchData, tag, PHB_MAX_TAG_LEN);
    }

    /* The number and length has to be stored after the alpha 
       identifier(Max 20 Bytes).Hence start updating the number from the 22nd
       address location of exchData and similarly for other parameters */
    cmhPHB_getAdrBcd ( exchData+22, exchData+20,
                       MAX_MB_NUM_LEN, number );

    if(type NEQ NULL)
    {
      memcpy(&toa_type,type,sizeof(toa_type));
    }
    else
    {
      toa_type.ton = TON_Unknown;
      toa_type.npi = NPI_IsdnTelephony;
      if ( number[0] EQ '+' )
      {
        toa_type.ton = TON_International;
      }
    }
    cmhPHB_toaMrg(&toa_type, exchData+21 );

   /*  capability/configuration identifier data and EXT identifier data */
    *(exchData+32)    = cc2_id;
    *(exchData+33)    = 0xFF;      /* EXT7 identifier is not supported */
  }
  /* write to SIM */    
  ret = cmhSIM_WriteRecordEF( srcId,AT_CMD_P_MBDN,SIM_MBDN,index,
                               ACI_SIZE_EF_MBDN,exchData,
                               cmhSIM_WrCnfMbdn);
  MFREE(exchData);
  return ( ret );
}
#endif /* FF_CPHS_REL4 */

/* Implements Measure 120 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_OpRead_simDataFld_RcdCb
+------------------------------------------------------------------------------
|  Description : This is CallBack function for SIM retrieval of EF_PNN
|                or EF_OPL, depending upon which one has been passed as an
|                argument reqDataFld.
|
|  Parameters  : table_id            - 
|                reqDataFld          - requested datafield identifier 
|                                      (SIM_PNN or SIM_OPL)
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void cmhSIM_OpRead_simDataFld_RcdCb( SHORT table_id, USHORT reqDataFld )

{
  void (*BuildsimDataFldList)(SHORT);
  void (*rplyCB)(SHORT);
  UBYTE *status,*num_rcd, max_records;

  TRACE_FUNCTION( "cmhSIM_OpRead_simDataFld_RcdCb()" );

  if( reqDataFld EQ SIM_PNN )
  {
    BuildsimDataFldList = cmhSIM_BuildPnnList;
    status  = &simShrdPrm.pnn_list.pnn_status;
    num_rcd = &simShrdPrm.pnn_list.num_rcd;
    rplyCB  = cmhSIM_OpReadPnnRcdCb;
    max_records = PNN_MAX_RECORDS;
  }
  else /* SIM_OPL */
  {
    BuildsimDataFldList = cmhSIM_BuildOPLList;
    status  = &simShrdPrm.opl_list.opl_status;
    num_rcd = &simShrdPrm.opl_list.num_rcd;
    rplyCB  = cmhSIM_OpReadOplRcdCb;
    max_records = OPL_MAX_RECORDS;
  }

  /* Decode and copy OPL record data to OPL list*/
  /*--------------------------------------------*/
  if( simShrdPrm.atb[table_id].dataLen AND 
      simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR )
  {
    /*
     * Here we are calling either cmhSIM_BuildPnnList()
     * or cmhSIM_BuildOPLList()
     */
    (*BuildsimDataFldList)( table_id );
    *status = TRUE;
  }
  else
  {
    TRACE_EVENT( "Empty record" );
  }

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  /* If not last EF record retrieve next record*/
  /*-------------------------------------------*/
  simShrdPrm.atb[table_id].recNr++;
  (*num_rcd)++;

  if ( (*num_rcd) > max_records )
  {
    TRACE_EVENT( "Max records reached" );
  }
  else if(simShrdPrm.atb[table_id].recNr <= simShrdPrm.atb[table_id].recMax )
  {
    TRACE_EVENT_P1("Read next record: %d",simShrdPrm.atb[table_id].recNr);
    cmhSIM_OpRead_Opl_or_Pnn_Rcd( simShrdPrm.atb[table_id].recNr, reqDataFld, rplyCB);
    return;
  }

  TRACE_EVENT("Retrieval of records finished");
  /* continue with next one */
  cmhSIM_StartOperatorName();
}

/* Implements Measure 42 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_AD_CSP_Update
+------------------------------------------------------------------------------
|  Description : This function can be used to request the SIM Customer Service 
|                Profile (EF_CPHS_CSP) or SIM Administrative Data (EF_AD).
|                by passing appropriate EF as an argument sim_datafld.
|
|
|
|  Parameters  : ref         - Reference for File Update
|                fu          - Pointer to SIM_FILE_UPDATE_IND
|                sim_datafld - EF - (SIM_AD or SIM_CPHS_CSP)
|
|  Return      : BOOL - TRUE or FALSE
+------------------------------------------------------------------------------
*/

LOCAL BOOL cmhSIM_AD_CSP_Update ( int ref, T_SIM_FILE_UPDATE_IND *fu ,
                                  USHORT sim_datafld )
{
  BOOL found = FALSE;
  UBYTE i;
  TRACE_FUNCTION ( "cmhSIM_AD_CSP_Update()" );

  for (i = 0; i < (int)fu->val_nr; i++)
  {
    if (!found AND fu->file_info[i].v_path_info EQ TRUE       AND
                   fu->file_info[i].path_info.df_level1   EQ SIM_DF_GSM AND
                   fu->file_info[i].path_info.v_df_level2 EQ FALSE      AND 
                   fu->file_info[i].datafield   EQ sim_datafld)
     {

       found = TRUE;
     }
  }
  if (found)
  {
    if ( sim_datafld EQ SIM_AD )
    {
      cmhSIM_Read_AD();
    }
    else
    {
      cmhSIM_Read_CSP();
    }
    simShrdPrm.fuRef = (UBYTE)ref;
    return FALSE; /* reading files */
   }
  else
  {
    return TRUE;  /* nothing to do */
  }
}

/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_Req_or_Write_PlmnSel
+------------------------------------------------------------------------------
|  Description : This function can be used for Either Reading or Writing
|                the EF PLMN SEL from SIM
|
|  Parameters  : sercId         - AT command source identifier
|                accType        - Access Type 
|                                 (ACT_WR_DAT or ACT_RD_DAT )
|
|  Return      : ACI functional return codes
+------------------------------------------------------------------------------
*/

GLOBAL T_ACI_RETURN cmhSIM_Req_or_Write_PlmnSel ( T_ACI_CMD_SRC srcId,
                                                  T_SIM_ACTP accType )
{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT        table_id;

  TRACE_FUNCTION ("cmhSIM_Req_or_Write_PlmnSel()");

/*
 *-----------------------------------------------------------------
 * request table id for SIM SAP access
 *-----------------------------------------------------------------
 */
  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    if( accType EQ ACT_WR_DAT )
    {
      simShrdPrm.atb[table_id].dataLen      = CPOLSimEfDataLen;
      simShrdPrm.atb[table_id].recMax       = NOT_PRESENT_8BIT;
      simShrdPrm.atb[table_id].rplyCB       = cmhSIM_WrCnfPlmnSel;
    }
    else /* ACT_RD_DAT */
    {
      simShrdPrm.atb[table_id].dataLen      = NOT_PRESENT_8BIT;
      simShrdPrm.atb[table_id].recMax       = ACI_LEN_PLMN_SEL_FLD;
      simShrdPrm.atb[table_id].rplyCB       = cmhSIM_RdCnfPlmnSel;
    }

    simShrdPrm.atb[table_id].accType      = accType;
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].reqDataFld   = SIM_PLMNSEL;
    simShrdPrm.atb[table_id].dataOff      = 0;
    simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;
    simShrdPrm.atb[table_id].exchData     = CPOLSimEfData;

    simShrdPrm.aId = table_id;

    simEntStat.curCmd = AT_CMD_CPOL;
    simShrdPrm.owner = (T_OWN)srcId;
    simEntStat.entOwn =  srcId;

    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR psaSIM in +CPOL");
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    }
    else
    {
      ret = AT_EXCT;
    }
  }

  return ( ret );
}

/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_ReqLanguage_LP_or_ELP
+------------------------------------------------------------------------------
|  Description : This function can be used for Reading the ELP or LP field
|                of the SIM
|
|  Parameters  : sercId         - AT command source identifier
|                reqDataFld     - SIM_ELP or SIM_LP
|
|  Return      : ACI functional return codes
+------------------------------------------------------------------------------
*/

GLOBAL T_ACI_RETURN cmhSIM_ReqLanguage_LP_or_ELP  ( T_ACI_CMD_SRC srcId,
                                                    USHORT reqDataFld )

{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT        table_id;

  TRACE_FUNCTION ("cmhSIM_ReqLanguage_LP_or_ELP()");

/*
 *-----------------------------------------------------------------
 * request table id for SIM SAP access
 *-----------------------------------------------------------------
 */
  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].reqDataFld   = reqDataFld;

    if(reqDataFld EQ SIM_ELP )
    {
      simShrdPrm.atb[table_id].recMax       = ACI_LEN_LAN_FLD;
      simShrdPrm.atb[table_id].exchData     = CLANSimEfData;
      simShrdPrm.atb[table_id].rplyCB       = cmhSIM_RdCnfLangELP;
    }
    else /* SIM_LP */
    {
      simShrdPrm.atb[table_id].recMax       = ACI_MAX_LAN_LP_NTRY;
      simShrdPrm.atb[table_id].exchData     = CLANSimEfDataLP;
      simShrdPrm.atb[table_id].rplyCB       = cmhSIM_RdCnfLangLP;
    }
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].accType      = ACT_RD_DAT;
    simShrdPrm.atb[table_id].dataOff      = 0;
    /* length is variable */
    simShrdPrm.atb[table_id].dataLen      = NOT_PRESENT_8BIT;
    simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;

    simShrdPrm.aId = table_id;

    simEntStat.curCmd = AT_CMD_CLAN;
    simShrdPrm.owner = (T_OWN)srcId;
    simEntStat.entOwn =  srcId;

    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR psaSIM in +CLAN");
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    }
    else
    {
      ret = AT_EXCT;
    }
  }

  return ( ret );
}

/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_OpRead_Opl_or_Pnn_Rcd
+------------------------------------------------------------------------------
|  Description : This function can be used for Reading the SIM_OPL or SIM_PNN
|                field of the SIM
|
|  Parameters  : rcd         - Record Number
|                reqDataFld  - SIM_OPL or SIM_PNN
|                rplyCB      - Call Back Function
|
|  Return      : BOOL - TRUE or FALSE
+------------------------------------------------------------------------------
*/

LOCAL BOOL cmhSIM_OpRead_Opl_or_Pnn_Rcd( UBYTE rcd, USHORT reqDataFld,
                                         void  (*rplyCB)(SHORT) )
{
  SHORT table_id;

  TRACE_FUNCTION ("cmhSIM_OpRead_Opl_or_Pnn_Rcd()");

  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
    simShrdPrm.atb[table_id].accType    = ACT_RD_REC;
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].reqDataFld = reqDataFld;
    simShrdPrm.atb[table_id].dataOff    = 0;
    simShrdPrm.atb[table_id].recNr      = rcd;
    simShrdPrm.atb[table_id].recMax     = 0;
    simShrdPrm.atb[table_id].exchData   = NULL;
    simShrdPrm.atb[table_id].dataLen    = NOT_PRESENT_8BIT;
    simShrdPrm.atb[table_id].rplyCB     = rplyCB;

    simShrdPrm.aId = table_id;

    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR");
      return FALSE;
    }
    
    return TRUE;
  }
  return TRUE;
}

/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_ReqLanguage_LP_or_ELP
+------------------------------------------------------------------------------
|  Description : This function can be used for Reading or Writing the 
|                Transparent EF of the SIM
|
|  Parameters  : srcId        - AT command source identifier
|                cmd          - AT command identifier 
|                datafield    - Requested datafield identifier
|                offset       - DataField Offset
|                dataLen      - Data Length
|                exchData     - points to exchange data buffer
|                rplyCB       - points to reply call-back 
|                accType      - Type of access (Read or Write)
|
|  Return      : BOOL - TRUE or FALSE
+------------------------------------------------------------------------------
*/

LOCAL T_ACI_RETURN cmhSIM_Write_or_Read_TranspEF (T_ACI_CMD_SRC srcId,
                                                  T_ACI_AT_CMD  cmd,
                                                  USHORT        datafield,
                                                  USHORT        offset,
                                                  UBYTE         datalen,
                                                  UBYTE       * exchData,
                                                  void      (*rplyCB)(SHORT),
                                                  T_SIM_ACTP accType,
                                                  BOOL v_path_info,
                                                  T_path_info   *path_info_ptr)
{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT        table_id;

  TRACE_FUNCTION ("cmhSIM_Write_or_Read_TranspEF()");

/*
 *-----------------------------------------------------------------
 * request table id for SIM SAP access
 *-----------------------------------------------------------------
 */
  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].accType      = accType;
    simShrdPrm.atb[table_id].v_path_info  = v_path_info;
    if(v_path_info EQ TRUE)
    {
      simShrdPrm.atb[table_id].path_info.df_level1    = path_info_ptr->df_level1;
      simShrdPrm.atb[table_id].path_info.v_df_level2  = path_info_ptr->v_df_level2;
      if(path_info_ptr->v_df_level2 EQ TRUE)
      {
        simShrdPrm.atb[table_id].path_info.df_level2  = path_info_ptr->df_level2;
      }
    }
    simShrdPrm.atb[table_id].reqDataFld   = datafield;
    simShrdPrm.atb[table_id].dataOff      = offset;
    simShrdPrm.atb[table_id].recNr        = 0;
    if(accType EQ ACT_WR_DAT)
    {
      simShrdPrm.atb[table_id].dataLen      = datalen;
      simShrdPrm.atb[table_id].recMax       = NOT_PRESENT_8BIT;
    }
    else
    {
      simShrdPrm.atb[table_id].dataLen      = NOT_PRESENT_8BIT;
      simShrdPrm.atb[table_id].recMax       = datalen;      
    }
    simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;
    simShrdPrm.atb[table_id].exchData     = exchData;
    simShrdPrm.atb[table_id].rplyCB       = rplyCB;


    simShrdPrm.aId = table_id;

    if (srcId NEQ CMD_SRC_NONE)
    {
      simEntStat.curCmd = cmd;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn =  srcId;
    }
    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR psaSIM");
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    }
    else
    {
      ret = AT_EXCT;
    }
  }

  return ( ret );
}

/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_Write_or_Read_RecordEF
+------------------------------------------------------------------------------
|  Description : This function can be used for Reading or Writing the 
|                EF of the SIM
|
|  Parameters  : srcId        - AT command source identifier
|                cmd          - AT command identifier 
|                datafield    - Requested datafield identifier
|                offset       - DataField Offset
|                dataLen      - Data Length
|                exchData     - points to exchange data buffer
|                rplyCB       - points to reply call-back 
|                accType      - Type of access (Read or Write)
|
|  Return      : BOOL - TRUE or FALSE
+------------------------------------------------------------------------------
*/

LOCAL T_ACI_RETURN cmhSIM_Write_or_Read_RecordEF ( T_ACI_CMD_SRC srcId,
                                                   T_ACI_AT_CMD  cmd,
                                                   USHORT        datafield,
                                                   UBYTE         record,
                                                   UBYTE         datalen,
                                                   UBYTE       * exchData,
                                                   void      (*rplyCB)(SHORT),
                                                   T_SIM_ACTP accType,
                                                   BOOL v_path_info,
                                                   T_path_info   *path_info_ptr)
{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT        table_id;

  TRACE_FUNCTION ("cmhSIM_Write_or_Read_RecordEF()");

/*
 *-----------------------------------------------------------------
 * request table id for SIM SAP access
 *-----------------------------------------------------------------
 */
  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].accType      = accType;
    simShrdPrm.atb[table_id].v_path_info  = v_path_info;
    if(v_path_info EQ TRUE)
    {
      simShrdPrm.atb[table_id].path_info.df_level1    = path_info_ptr->df_level1;
      simShrdPrm.atb[table_id].path_info.v_df_level2  = path_info_ptr->v_df_level2;
      if(path_info_ptr->v_df_level2 EQ TRUE)
      {
        simShrdPrm.atb[table_id].path_info.df_level2  = path_info_ptr->df_level2;
      }
    }

    simShrdPrm.atb[table_id].reqDataFld   = datafield;
    simShrdPrm.atb[table_id].dataOff      = 0;
    /* 
     * for CPHS (and this shall probably be extended to other SIM
     * access operations) dataLen passed in SIM_READ_RECORD_REQ should
     * be NOT_PRESENT_8BIT (to let the SIM entity handling this length
     * ACI does not know it anyway...). Yet size of received data should
     * be checked when conf is received (to avoid crashes with exotic SIM cards
     */
    if(accType EQ ACT_RD_REC)
    {
#ifdef FF_CPHS
      if(cmd EQ AT_CMD_CPHS)
      {
        simShrdPrm.atb[table_id].check_dataLen = TRUE;
      }
#endif /* FF_CPHS */
    }
    simShrdPrm.atb[table_id].dataLen      = datalen;
    simShrdPrm.atb[table_id].recNr        = record;
    simShrdPrm.atb[table_id].recMax       = NOT_PRESENT_8BIT;
    simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;
    simShrdPrm.atb[table_id].exchData     = exchData;
    simShrdPrm.atb[table_id].rplyCB       = rplyCB;

    simShrdPrm.aId = table_id;

    if (srcId NEQ CMD_SRC_NONE)
    {
      simEntStat.curCmd = cmd;
      simShrdPrm.owner = (T_OWN)srcId;
      simEntStat.entOwn =  srcId;
    }
    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR psaSIM");
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    }
    else
    {
      ret = AT_EXCT;
    }
  }

  return ( ret );
}

/*
+------------------------------------------------------------------------------
|  Function    : cmhSIM_ReqLanguage_LP_or_ELP
+------------------------------------------------------------------------------
|  Description : This function starts reading of LP or ELP field from SIM 
|                for SAT feature LS.
|
|  Parameters  : reqDataFld  - SIM_OPL or SIM_PNN
|                recMax      - Maximum Records.
|                exchData    - points to exchange data buffer
|                rplyCB      - Call Back Function
|
|  Return      : BOOL - TRUE or FALSE
+------------------------------------------------------------------------------
*/

GLOBAL BOOL cmhSIM_ReqLanguagePrf_LP_or_ELP( USHORT reqDataFld,
                                             UBYTE  recMax,
                                             UBYTE  *exchData,
                                             void   (*rplyCB)(SHORT) )
{
  BOOL     ret  = FALSE;
  SHORT        table_id;

  TRACE_FUNCTION ("cmhSIM_ReqLanguagePrf_LP_or_ELP()");

/*
 *-----------------------------------------------------------------
 * request table id for SIM SAP access
 *-----------------------------------------------------------------
 */
  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].accType      = ACT_RD_DAT;
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].reqDataFld   = reqDataFld;
    simShrdPrm.atb[table_id].dataOff      = 0;
    // length is variable
    simShrdPrm.atb[table_id].dataLen      = NOT_PRESENT_8BIT;
    simShrdPrm.atb[table_id].recMax       = recMax;
    simShrdPrm.atb[table_id].ntryUsdFlg   = TRUE;
    simShrdPrm.atb[table_id].exchData     = exchData;
    simShrdPrm.atb[table_id].rplyCB       = rplyCB;

    simShrdPrm.aId = table_id;

       
    if(psaSIM_AccessSIMData() < 0)
    {
      return ( ret );      
    }
    else
    {
      ret = TRUE;
    }
  }

  return ( ret );
}


/*==== EOF ========================================================*/

