/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SSF
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
|  Purpose :  This module defines the functions used by the command
|             handler for the supplementary service module.
+-----------------------------------------------------------------------------
*/
#ifndef CMH_SSF_C
#define CMH_SSF_C
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

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifdef UART
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#include "pcm.h"
#include "ksd.h"
#include "aci.h"
#include "psa.h"
#include "psa_ss.h"
#include "psa_sim.h"
#include "psa_util.h"
#include "cmh.h"
#include "cmh_ss.h"
#include "cmh_sim.h"
#include "psa_cc.h"
#include "psa_sat.h"

#include "cl_imei.h"

/* For SAT and FDN check */
#include "cmh_cc.h"
#include "cmh_sat.h"
#include "phb.h"
#include "aci_ext_pers.h"

/*
 * FreeCalypso hack: gcc is complaining about integer/pointer mix violations
 * with regard to NULL being used in integer contexts, so we are going to
 * silence it as follows:
 */
#undef NULL
#define	NULL	0

/*==== CONSTANTS ==================================================*/
#ifdef SIM_TOOLKIT
 /* cModMapTable maps Mode into SS Mode Code. */
const T_IDX_TO_STR_MAP cModMapTable[]   = 
                                  {{CCFC_MOD_Disable, "#"}, {CCFC_MOD_Enable, "*"}, {CCFC_MOD_Register, "**"}, 
                                    {CCFC_MOD_Erasure, "##"}, {CCFC_MOD_Query, "*#"}, {NULL, NULL}}; 

/* Call Forwarding */
 /* cRsnMapTable maps Reason into SS Service Code. Must correspond to T_ACI_CCFC_RSN*/
const T_IDX_TO_STR_MAP cCFRsnMapTable[] = 
                                  {{CCFC_RSN_Uncond, "21"}, {CCFC_RSN_Busy, "67"}, {CCFC_RSN_NoReply, "61"},
                                    {CCFC_RSN_NotReach, "62"}, {CCFC_RSN_Forward, "002"}, {CCFC_RSN_CondForward, "004"},
                                    {NULL, NULL}};

/* Call Barring */
 /* cRsnMapTable maps Reason into SS Service Code. Must correspond to T_ACI_CLCK_FAC*/
const T_IDX_TO_STR_MAP cCBRsnMapTable[] = 
                                  {{FAC_Ao, "33"}, {FAC_Oi, "331"}, {FAC_Ox, "332"},
                                    {FAC_Ai, "35"}, {FAC_Ir, "351"}, {FAC_Ab, "330"},
                                    {FAC_Ag, "333"}, {FAC_Ac, "353"},
                                    {NULL, NULL}};
                                    

const T_IDX_TO_DIG_MAP cCFBSMapTable[] = 
                               {{CLASS_Vce, KSD_BS_Telephony}, {CLASS_Dat, KSD_BS_AllBearer}, {CLASS_VceDat, KSD_BS_Telephony},
                                 {CLASS_Fax, KSD_BS_AllFax}, {CLASS_VceFax, KSD_BS_AllTeleXcptSMS}, {CLASS_DatFax, KSD_BS_AllFax}, 
                                 {CLASS_VceDatFax, KSD_BS_AllTeleXcptSMS}, {CLASS_Sms, KSD_BS_SMS}, {CLASS_DatSms, KSD_BS_SMS},
                                 {CLASS_VceDatFaxSms, KSD_BS_AllTele}, {CLASS_DatCirSync, KSD_BS_AllDataCircSync},
                                 {CLASS_DatCirAsync, KSD_BS_AllDataCircAsync}, {CLASS_AllSync, KSD_BS_AllSync}, {CLASS_AllAsync, KSD_BS_AllAsync},
                                 {CLASS_DedPacAccess, KSD_BS_AllDedPackAcc}, {CLASS_DedPADAcess, KSD_BS_AllDedPADAcc},
                                 {CLASS_AuxVce, KSD_BS_PLMN1}, {NULL, NULL}};

const T_IDX_TO_DIG_MAP cCFBSAllBearMapTable[] = 
                               {{CLASS_VceDat, KSD_BS_AllBearer}, {CLASS_DatFax, KSD_BS_AllBearer}, {CLASS_VceDatFax, KSD_BS_AllBearer},
                                 {CLASS_Sms, KSD_BS_AllBearer}, {CLASS_VceDatFaxSms, KSD_BS_AllBearer}, {NULL, NULL}};
#endif /* SIM_TOOLKIT */

/*==== TYPES ======================================================*/

LOCAL UBYTE cmhSS_getSumOf2Digits ( UBYTE       digits );

#if defined (SIM_PERS) || defined (FF_PHONE_LOCK)
 T_ACI_RETURN cmhSS_check_oper_result(T_OPER_RET_STATUS result); 
#endif

/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

EXTERN T_PCEER causeMod;
EXTERN SHORT causeCeer;

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_GetClass               |
+-------------------------------------------------------------------+

  PURPOSE : Get class value out of a basic service type.

*/

GLOBAL T_ACI_CLASS cmhSS_GetClass( T_basicService * bs )
{
  TRACE_FUNCTION ("cmhSS_GetClass()");

  if( bs -> v_bearerService )
  {
    return( CLASS_Dat );
  }

  else if( bs -> v_teleservice )
  {
    switch( bs -> teleservice )
    {
      case( TS_CD_ALL_SPCH ):     return( CLASS_Vce );
      case( TS_CD_ALL_FAX ):      return( CLASS_Fax );
      case( TS_CD_ALL_XCPT_SMS ): return( CLASS_VceFax );
      case( TS_CD_PLMN1 ):        return( CLASS_AuxVce );
      default:                    return( CLASS_None );
    }
  }

  else

    return( CLASS_None );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_GetClassLst            |
+-------------------------------------------------------------------+

  PURPOSE : Get combined class value out of a basic service list.

*/

GLOBAL T_ACI_CLASS cmhSS_GetClassLst( T_basicServiceGroupList * bs )
{
  T_ACI_CLASS class_type = CLASS_None;
  UBYTE       idx;

  TRACE_FUNCTION ("cmhSS_GetClassLst()");

  for (idx = 0; idx < bs -> c_basicServiceGroupList_value; idx++)
  {
    if( bs -> basicServiceGroupList_value[idx].v_bearerService )
    {
      switch(bs-> basicServiceGroupList_value[idx].bearerService)
      {
        case( BS_CD_ALL_DC_SYN ):     class_type |= CLASS_DatCirSync; break;
        case( BS_CD_ALL_DC_ASYN ):    class_type |= CLASS_DatCirAsync; break;
        case( BS_CD_ALL_DATA_PDS ):   class_type |= CLASS_DedPacAccess; break;
        case( BS_CD_ALL_DATA_PAD ):   class_type |= CLASS_DedPADAcess; break;
        default:                      class_type |= CLASS_Dat; break;
      }
    }
  }

  for (idx = 0; idx < bs -> c_basicServiceGroupList_value; idx++)
  {
    if( bs -> basicServiceGroupList_value[idx].v_teleservice )
    {
      switch( bs -> basicServiceGroupList_value[idx].teleservice )
      {

        case( TS_CD_ALL_SPCH ):     class_type |= CLASS_Vce; break;
        case( TS_CD_ALL_FAX ):      class_type |= CLASS_Fax; break;
        case( TS_CD_ALL_XCPT_SMS ): class_type |= CLASS_VceFax; break;
        case( TS_CD_ALL_SMS ):
        case( TS_CD_SMS_MT ):
        case( TS_CD_SMS_MO ):       class_type |= CLASS_Sms; break;
        case( TS_CD_PLMN1 ):        class_type |= CLASS_AuxVce; break;
      }
    }
  }

  return( class_type );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_CheckClass             |
+-------------------------------------------------------------------+

  PURPOSE : Check class value.

*/

GLOBAL BOOL cmhSS_CheckClassInterr( T_ACI_CLASS class_type)
{
  TRACE_FUNCTION ("cmhSS_CheckClassInterr()");

  switch( class_type )
  {
  case( CLASS_None ):
  case( CLASS_Vce ):
  case( CLASS_Dat ):
  case( CLASS_VceDat ):
  case( CLASS_Fax ):
  case( CLASS_VceFax ):
  case( CLASS_DatFax ):
  case( CLASS_NotPresent ):
  case( CLASS_VceDatFax  ):
  case( CLASS_DatCirSync ):
  case( CLASS_DatCirAsync ):
  case( CLASS_DedPacAccess ):
  case( CLASS_AllSync ):
  case( CLASS_AllAsync ):
  case( CLASS_DedPADAcess ):
  case( CLASS_AuxVce ):
    return( TRUE );

  default:
    return( FALSE );
  }
}

GLOBAL BOOL cmhSS_CheckCbClassInterr( T_ACI_CLASS class_type)
{
  TRACE_FUNCTION ("cmhSS_CheckClassInterr()");

  switch( class_type )
  {
  case( CLASS_None ):
  case( CLASS_Vce ):
  case( CLASS_Dat ):
  case( CLASS_VceDat ):
  case( CLASS_Fax ):
  case( CLASS_VceFax ):
  case( CLASS_DatFax ):
  case( CLASS_NotPresent ):
  case( CLASS_VceDatFax  ):
  case( CLASS_Sms ):
  case( CLASS_VceSms ):
  case( CLASS_DatSms ):
  case( CLASS_VceDatSms ):
  case( CLASS_FaxSms ):
  case( CLASS_VceFaxSms ):
  case( CLASS_DatFaxSms ):
  case( CLASS_VceDatFaxSms ):
  case( CLASS_DatCirSync ):
  case( CLASS_DatCirAsync ):
  case( CLASS_DedPacAccess ):
  case( CLASS_AllSync ):
  case( CLASS_AllAsync ):
  case( CLASS_DedPADAcess ):
  case( CLASS_AuxVce) :
    return( TRUE );

  default:
    return( FALSE );
  }
}

GLOBAL BOOL cmhSS_CheckClass( T_ACI_CLASS class_type,
                              UBYTE *bs1, UBYTE *bst1,
                              UBYTE *bs2, UBYTE *bst2,
                              BOOL  *mltyTrnFlg )
{
  TRACE_FUNCTION ("cmhSS_CheckClass()");

  *bs1 = *bs2 = *bst1 = *bst2 = SS_NO_PRM;
  *mltyTrnFlg = FALSE;

  switch( class_type )
  {
    case( CLASS_NotPresent ):
    case( CLASS_None ):          *bs1 =                     *bst1 = SS_NO_PRM;
      break;
    case( CLASS_Vce ):           *bs1 = TS_CD_ALL_SPCH;     *bst1 = BS_TELE_SRV;
      break;
    case( CLASS_AuxVce ):        *bs1 = TS_CD_PLMN1;        *bst1 = BS_TELE_SRV;
      break;
    case( CLASS_Dat ):           *bs1 = BS_CD_ALL_BS;       *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_VceDat ):        *bs1 = TS_CD_ALL_SPCH;     *bst1 = BS_TELE_SRV;
                                 *bs2 = BS_CD_ALL_BS;       *bst2 = BS_BEAR_SRV;
                                 *mltyTrnFlg = TRUE;
      break;
    case( CLASS_Fax ):           *bs1 = TS_CD_ALL_FAX;      *bst1 = BS_TELE_SRV;
      break;
    case( CLASS_VceFax ):        *bs1 = TS_CD_ALL_XCPT_SMS; *bst1 = BS_TELE_SRV;
      break;
    case( CLASS_DatFax ):        *bs1 = TS_CD_ALL_FAX;      *bst1 = BS_TELE_SRV;
                                 *bs2 = BS_CD_ALL_BS;       *bst2 = BS_BEAR_SRV;
                                 *mltyTrnFlg = TRUE;
      break;

    case( CLASS_VceDatFax  ):    *bs1 = TS_CD_ALL_XCPT_SMS; *bst1 = BS_TELE_SRV;
                                 *bs2 = BS_CD_ALL_BS;       *bst2 = BS_BEAR_SRV;
                                 *mltyTrnFlg = TRUE;
      break;
    case( CLASS_DatCirSync ):
                                 *bs1 = BS_CD_ALL_DC_SYN;   *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_DatCirAsync ):
                                 *bs1 = BS_CD_ALL_DC_ASYN;  *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_DedPacAccess ):
                                 *bs1 = BS_CD_ALL_DATA_PDS; *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_AllSync ):
                                 *bs1 = BS_CD_ALL_SYN;      *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_AllAsync ):
                                 *bs1 = BS_CD_ALL_ASYN;     *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_DedPADAcess ):
                                 *bs1 = BS_CD_ALL_DATA_PAD; *bst1 = BS_BEAR_SRV;
      break;
  default:
    return( FALSE );
  }

  return( TRUE );
}

GLOBAL BOOL cmhSS_CheckCbClass( T_ACI_CLASS class_type,
                                UBYTE *bs1, UBYTE *bst1,
                                UBYTE *bs2, UBYTE *bst2,
                                BOOL  *mltyTrnFlg )
{
  TRACE_FUNCTION ("cmhSS_CheckCBClass()");

  *bs1 = *bs2 = *bst1 = *bst2 = SS_NO_PRM;
  *mltyTrnFlg = FALSE;

  switch( class_type )
  {
    case CLASS_NotPresent:
    case CLASS_None:             *bs1 =                     *bst1 = SS_NO_PRM;
      break;
    case CLASS_Vce:              *bs1 = TS_CD_ALL_SPCH;     *bst1 = BS_TELE_SRV;
      break;
    case CLASS_AuxVce:           *bs1 = TS_CD_PLMN1;        *bst1 = BS_TELE_SRV;
      break;
    case CLASS_Dat :             *bs1 = BS_CD_ALL_BS;       *bst1 = BS_BEAR_SRV;
      break;
    case CLASS_VceDat:           *bs1 = TS_CD_ALL_SPCH;     *bst1 = BS_TELE_SRV;
                                 *bs2 = BS_CD_ALL_BS;       *bst2 = BS_BEAR_SRV;
                                 *mltyTrnFlg = TRUE;
      break;
    case CLASS_Fax:              *bs1 = TS_CD_ALL_FAX;      *bst1 = BS_TELE_SRV;
      break;
    case CLASS_VceFax:           *bs1 = TS_CD_ALL_XCPT_SMS; *bst1 = BS_TELE_SRV;
      break;
    case CLASS_DatFax:           *bs1 = TS_CD_ALL_FAX;      *bst1 = BS_TELE_SRV;
                                 *bs2 = BS_CD_ALL_BS;       *bst2 = BS_BEAR_SRV;
                                 *mltyTrnFlg = TRUE;
      break;
    case CLASS_VceDatFax:        *bs1 = TS_CD_ALL_XCPT_SMS; *bst1 = BS_TELE_SRV;
                                 *bs2 = BS_CD_ALL_BS;       *bst2 = BS_BEAR_SRV;
                                 *mltyTrnFlg = TRUE;
      break;
    case CLASS_Sms:              *bs1 = TS_CD_ALL_SMS;      *bst1 = BS_TELE_SRV;
      break;
    case CLASS_DatSms:           *bs1 = TS_CD_ALL_SMS;     *bst1 = BS_TELE_SRV;
                                 *bs2 = BS_CD_ALL_BS;       *bst2 = BS_BEAR_SRV;
                                 *mltyTrnFlg = TRUE;
      break;
    case CLASS_VceDatFaxSms:     *bs1 = TS_CD_ALL_TS;       *bst1 = BS_TELE_SRV;
                                 *bs2 = BS_CD_ALL_BS;       *bst2 = BS_BEAR_SRV;
                                 *mltyTrnFlg = TRUE;
      break;
    case( CLASS_DatCirSync ):
                                 *bs1 = BS_CD_ALL_DC_SYN;   *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_DatCirAsync ):
                                 *bs1 = BS_CD_ALL_DC_ASYN;  *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_DedPacAccess ):
                                 *bs1 = BS_CD_ALL_DATA_PDS; *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_AllSync ):
                                 *bs1 = BS_CD_ALL_SYN;      *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_AllAsync ):
                                 *bs1 = BS_CD_ALL_ASYN;     *bst1 = BS_BEAR_SRV;
      break;
    case( CLASS_DedPADAcess ):
                                 *bs1 = BS_CD_ALL_DATA_PAD; *bst1 = BS_BEAR_SRV;
      break;
    
    case CLASS_VceSms:
    case CLASS_VceDatSms:
    case CLASS_FaxSms:
    case CLASS_VceFaxSms:
    case CLASS_DatFaxSms:
      return( FALSE );

    default:
      return( FALSE );
  }


  return( TRUE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_CheckBscSrv            |
+-------------------------------------------------------------------+

  PURPOSE : Check basic service value.

*/

GLOBAL BOOL cmhSS_CheckBscSrv ( UBYTE bsCd,
                                UBYTE *bs, UBYTE *bst, T_ACI_CLASS *class_type )
{
  T_ACI_CLASS dummy_class;

  TRACE_FUNCTION ("cmhSS_CheckBscSrv()");

  if(class_type EQ NULL)
  {
    class_type = &dummy_class;
  }

  *bs = *bst = SS_NO_PRM;
  *class_type = CLASS_None;

  switch( bsCd )
  {
  case( KSD_BS_None ):
  case( KSD_BS_AllTeleAllBearer ): /* no code required */
    *class_type = (T_ACI_CLASS)(CLASS_VceDatFaxSms + CLASS_AuxVce);
    break;
  case( KSD_BS_AllTele ):          *bs = TS_CD_ALL_TS;       *bst = BS_TELE_SRV;  
    *class_type =(T_ACI_CLASS)( CLASS_VceFaxSms + CLASS_AuxVce);
    break;
  /* For MMI service Code Telephony, use the corresponding Service Group
     "Speech"... The MS shall always send the Service Group... See GSM: 03.11 */
  case( KSD_BS_Telephony ):        *bs = TS_CD_ALL_SPCH;     *bst = BS_TELE_SRV;  
    *class_type =(T_ACI_CLASS) (CLASS_Vce + CLASS_AuxVce);
    break;
  case( KSD_BS_AllData ):          *bs = TS_CD_ALL_DATA;     *bst = BS_TELE_SRV;  *class_type = CLASS_Dat;
    break;
  case( KSD_BS_AllFax ):           *bs = TS_CD_ALL_FAX;      *bst = BS_TELE_SRV;  *class_type = CLASS_Fax;
    break;
  case( KSD_BS_SMS ):              *bs = TS_CD_ALL_SMS;      *bst = BS_TELE_SRV;  *class_type = CLASS_Sms;
    break;
  case( KSD_BS_VoiceGroup ):       *bs = TS_CD_ALL_SPCH;     *bst = BS_TELE_SRV;  
    *class_type = (T_ACI_CLASS)(CLASS_Vce + CLASS_AuxVce);
    break;
  case( KSD_BS_AllTeleXcptSMS ):   *bs = TS_CD_ALL_XCPT_SMS; *bst = BS_TELE_SRV; 
    *class_type = (T_ACI_CLASS)(CLASS_VceFax  + CLASS_AuxVce);
    break;
  case( KSD_BS_AllBearer ):        *bs = BS_CD_ALL_BS;       *bst = BS_BEAR_SRV;  *class_type = CLASS_Dat;
    break;
  case( KSD_BS_AllAsync ):         *bs = BS_CD_ALL_ASYN;     *bst = BS_BEAR_SRV;  *class_type = CLASS_Dat;
    break;
  case( KSD_BS_AllSync ):          *bs = BS_CD_ALL_SYN;      *bst = BS_BEAR_SRV;  *class_type = CLASS_Dat;
    break;
  case( KSD_BS_AllDataCircSync ):  *bs = BS_CD_ALL_DC_SYN;   *bst = BS_BEAR_SRV;  *class_type = CLASS_Dat;
    break;
  case( KSD_BS_AllDataCircAsync ): *bs = BS_CD_ALL_DC_ASYN;  *bst = BS_BEAR_SRV;  *class_type = CLASS_Dat;
    break;
  case( KSD_BS_AllDedPackAcc ):    *bs = BS_CD_ALL_DATA_PDS; *bst = BS_BEAR_SRV;  *class_type = CLASS_Dat;
    break;
  case( KSD_BS_AllDedPADAcc ):     *bs = BS_CD_ALL_DATA_PAD; *bst = BS_BEAR_SRV;  *class_type = CLASS_Dat;
    break;
  case( KSD_BS_AllPLMNSpecTele ):  *bs = TS_CD_ALL_PSSS; *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele1 ):    *bs = TS_CD_PLMN1;    *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele2 ):    *bs = TS_CD_PLMN2;    *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele3 ):    *bs = TS_CD_PLMN3;    *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele4 ):    *bs = TS_CD_PLMN4;    *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele5 ):    *bs = TS_CD_PLMN5;    *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele6 ):    *bs = TS_CD_PLMN6;    *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele7 ):    *bs = TS_CD_PLMN7;    *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele8 ):    *bs = TS_CD_PLMN8;    *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele9 ):    *bs = TS_CD_PLMN9;    *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele10 ):   *bs = TS_CD_PLMNA;   *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele11 ):   *bs = TS_CD_PLMNB;   *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele12 ):   *bs = TS_CD_PLMNC;   *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele13 ):   *bs = TS_CD_PLMND;   *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele14 ):   *bs = TS_CD_PLMNE;   *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_PLMNSpecTele15 ):   *bs = TS_CD_PLMNF;   *bst = BS_TELE_SRV;
    break;
  case( KSD_BS_AllPLMNSpecBearer ):*bs = BS_CD_ALL_PSSS; *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer1 ):  *bs = BS_CD_PLMN1;    *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer2 ):  *bs = BS_CD_PLMN2;    *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer3 ):  *bs = BS_CD_PLMN3;    *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer4 ):  *bs = BS_CD_PLMN4;    *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer5 ):  *bs = BS_CD_PLMN5;    *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer6 ):  *bs = BS_CD_PLMN6;    *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer7 ):  *bs = BS_CD_PLMN7;    *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer8 ):  *bs = BS_CD_PLMN8;    *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer9 ):  *bs = BS_CD_PLMN9;    *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer10 ): *bs = BS_CD_PLMNA;   *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer11 ): *bs = BS_CD_PLMNB;   *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer12 ): *bs = BS_CD_PLMNC;   *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer13 ): *bs = BS_CD_PLMND;   *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer14 ): *bs = BS_CD_PLMNE;   *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_PLMNSpecBearer15 ): *bs = BS_CD_PLMNF;   *bst = BS_BEAR_SRV;
    break;
  case( KSD_BS_AuxTelephony     ): *bs = TS_CD_PLMN1;   *bst = BS_TELE_SRV;  *class_type = CLASS_AuxVce;
    break;

  default:

    return( FALSE );
  }


  return( TRUE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_GetBscSrv              |
+-------------------------------------------------------------------+

  PURPOSE : Get basic service code out of a basic service type.

*/

GLOBAL void cmhSS_GetBscSrv( T_basicService * p, UBYTE * bs,
                             UBYTE * bst )
{
  TRACE_FUNCTION ("cmhSS_GetBscSrv()");

  if( p -> v_bearerService )
  {
    *bst = KSD_BS_TP_Bearer;
    *bs  = p -> bearerService;
  }

  else if( p -> v_teleservice )
  {
    *bst = KSD_BS_TP_Tele;
    *bs  = p -> teleservice;
  }

  else
  {
    *bst = KSD_BS_None;
    *bs  = KSD_BS_TeleBearerUnknown;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmhSS_flagTrn           |
+-------------------------------------------------------------------+

  PURPOSE : flag a call for multy-transaction operation

*/

GLOBAL void cmhSS_flagTrn ( SHORT sId, USHORT * flags )
{
  *flags |= (0x1 << sId);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmhSS_tstAndUnflagTrn   |
+-------------------------------------------------------------------+

  PURPOSE : test and unflag a call for multy-transaction operation.
            Return the test result.

*/

GLOBAL BOOL cmhSS_tstAndUnflagTrn ( SHORT sId, USHORT * flags )
{
  if( *flags & (1u << sId))
  {
    *flags &= ~(1u << sId);
    return( TRUE );
  }
  return( FALSE );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdCF              |
+--------------------------------------------------------------------+

  PURPOSE : This function invokes the operation for key sequence
            command for call forwarding supplementary services.
*/

GLOBAL T_ACI_RETURN cmhSS_ksdCF  ( T_ACI_CMD_SRC srcId,
                                   T_ACI_KSD_CF_PRM * cfPrm )
{
  SHORT sId;                /* holds service id */
  UBYTE bst;                /* holds basic service type */
  UBYTE bs;                 /* holds basic service */
  T_ACI_CLASS user_class;

  TRACE_FUNCTION ("cmhSS_ksdCF");

/*
 *-------------------------------------------------------------------
 * check parameter ss code
 *-------------------------------------------------------------------
 */
  switch( cfPrm->ssCd )
  {
    case( KSD_SS_CFU       ):
    case( KSD_SS_CFB       ):
    case( KSD_SS_CFNRY     ):
    case( KSD_SS_CFNRC     ):
    case( KSD_SS_ALL_FWSS  ):
    case( KSD_SS_ALL_CFWSS ):

      break;

    default:

      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter basic service code
 *-------------------------------------------------------------------
 */
  if( !cmhSS_CheckBscSrv( cfPrm->bsCd, &bs, &bst, &user_class ))

    return( AT_FAIL );

/*
 *-------------------------------------------------------------------
 * check parameter <class> against possible ALS-Lock
 *-------------------------------------------------------------------
 */
  if ((ALSlock EQ ALS_MOD_SPEECH     AND user_class EQ  CLASS_AuxVce) OR
      (ALSlock EQ ALS_MOD_AUX_SPEECH AND user_class NEQ CLASS_AuxVce))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_AlsLock );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * get a new service table entry
 *-------------------------------------------------------------------
 */
  sId = psaSS_stbNewEntry();

  if( sId EQ NO_ENTRY )
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter operation code
 *-------------------------------------------------------------------
 */
  CCD_START;

  switch( cfPrm->opCd )
  {
    /* interrogate call forwarding service */
    case( KSD_OP_IRGT  ):
      psaSS_asmInterrogateSS( cfPrm->ssCd, bst, bs );
      break;

    /* register call forwarding service */
    case( KSD_OP_REG   ):

      psaSS_asmRegisterSS( cfPrm->ssCd, bst, bs,
                           cfPrm->ton, cfPrm->npi, cfPrm->num,
                           cfPrm->tos, cfPrm->oe,  cfPrm->sub,
                           cfPrm->time );
      break;

    /* erase call forwarding service */
    case( KSD_OP_ERS   ):
      psaSS_asmEraseSS( cfPrm->ssCd, bst, bs );
      break;

    /* activate call forwarding service */
    case( KSD_OP_ACT   ):
      psaSS_asmActivateSS( cfPrm->ssCd, bst, bs );
      break;

    /* deactivate call forwarding service */
    case( KSD_OP_DEACT ):
      psaSS_asmDeactivateSS( cfPrm->ssCd, bst, bs );
      break;

    /* unexpected operation code */
    default:

      CCD_END;
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * start transaction
 *-------------------------------------------------------------------
 */
  ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
  ssShrdPrm.stb[sId].ssCode     = cfPrm->ssCd;
  ssShrdPrm.stb[sId].orgOPC     = cfPrm->opCd;
  ssShrdPrm.stb[sId].opCode     = cfPrm->opCd;
  ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;
  ssShrdPrm.stb[sId].ClassType  = user_class;

  TRACE_EVENT_P1("USER_CLASS: %d", user_class);

  ssShrdPrm.stb[sId].curCmd     = (T_ACI_AT_CMD)KSD_CMD_CF;

  psaSS_NewTrns(sId);

  CCD_END;
/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  if( srcId NEQ ((T_ACI_CMD_SRC)OWN_SRC_SAT) )
  {
  T_ACI_CLOG cmdLog;        /* holds logging info */

  cmdLog.atCmd                = (T_ACI_AT_CMD) KSD_CMD_CF;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = sId+1;
  cmdLog.cmdPrm.sKSCF.srcId   = srcId;
  cmdLog.cmdPrm.sKSCF.opCd    = cfPrm->opCd;
  cmdLog.cmdPrm.sKSCF.ssCd    = cfPrm->ssCd;
  cmdLog.cmdPrm.sKSCF.bsTp    = bst;
  cmdLog.cmdPrm.sKSCF.bsCd    = bs;
  cmdLog.cmdPrm.sKSCF.num     = cfPrm->num;
  cmdLog.cmdPrm.sKSCF.ton     = cfPrm->ton;
  cmdLog.cmdPrm.sKSCF.npi     = cfPrm->npi;
  cmdLog.cmdPrm.sKSCF.sub     = cfPrm->sub;
  cmdLog.cmdPrm.sKSCF.tos     = cfPrm->tos;
  cmdLog.cmdPrm.sKSCF.oe      = cfPrm->oe;
  cmdLog.cmdPrm.sKSCF.time    = cfPrm->time;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdPW              |
+--------------------------------------------------------------------+

  PURPOSE : This function invokes the operation for key sequence
            command for password registration supplementary services.
*/

GLOBAL T_ACI_RETURN cmhSS_ksdPW  ( T_ACI_CMD_SRC srcId,
                                   T_ACI_KSD_PWD_PRM * pwPrm )
{
  SHORT sId=NO_ENTRY;           /* holds service id */

  TRACE_FUNCTION ("cmhSS_ksdPW");

/*
 *-------------------------------------------------------------------
 * check password parameter
 *-------------------------------------------------------------------
 */
  if( !pwPrm->oldPwd OR !pwPrm->newPwd )
  {
    TRACE_EVENT("parameters are missing");
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter ss code
 *-------------------------------------------------------------------
 */
  switch( pwPrm->ssCd )
  {
    /*
     *-------------------------------------------------------------------
     * if password relates to SS
     *-------------------------------------------------------------------
     */
    case( KSD_SS_BOC      ):
    case( KSD_SS_BOICXH   ):
    case( KSD_SS_BOIC     ):
    case( KSD_SS_ALL_CBSS ):
    case( KSD_SS_BAOC     ):
    case( KSD_SS_BIC      ):
    case( KSD_SS_BAIC     ):
    case( KSD_SS_BICRM    ):
    case( KSD_SS_NONE     ):
    case( KSD_SS_ALL_SERV ):


    /*
     *-------------------------------------------------------------------
     * check password length
     *-------------------------------------------------------------------
     */
      if( strlen( (char *) pwPrm->oldPwd ) > MAX_PWD_NUM  OR
          strlen( (char *) pwPrm->newPwd ) > MAX_PWD_NUM  OR
          strlen( (char *) pwPrm->newPwd2) > MAX_PWD_NUM     )
      {
        TRACE_EVENT("password too long");
        return( AT_FAIL );
      }

    /*
     *-------------------------------------------------------------------
     * get a new service table entry
     *-------------------------------------------------------------------
     */
      sId = psaSS_stbNewEntry();

      if( sId EQ NO_ENTRY )
      {
        return( AT_FAIL );
      }

    /*
     *-------------------------------------------------------------------
     * check parameter operation code
     *-------------------------------------------------------------------
     */
      CCD_START;

      psaSS_asmRegisterPWD( pwPrm->ssCd );

    /*
     *-------------------------------------------------------------------
     * start transaction
     *-------------------------------------------------------------------
     */
      ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
      ssShrdPrm.stb[sId].ssCode     = pwPrm->ssCd;
      ssShrdPrm.stb[sId].orgOPC     = KSD_OP_REG;
      ssShrdPrm.stb[sId].opCode     = KSD_OP_REG;
      ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;

      ssShrdPrm.stb[sId].curCmd     = (T_ACI_AT_CMD)KSD_CMD_PWD;

      psaSS_NewTrns(sId);

      CCD_END;

    /*
     *-------------------------------------------------------------------
     * store passwords for later use
     *-------------------------------------------------------------------
     */
      strncpy
      (
        (char *) cmhPrm[srcId].ssCmdPrm.CXXXpwd,
        (char *) pwPrm->oldPwd,
        MAX_PWD_NUM
      );
      cmhPrm[srcId].ssCmdPrm.CXXXpwd[MAX_PWD_NUM] = '\0';

      strncpy
      (
        (char *) cmhPrm[srcId].ssCmdPrm.CXXXnewPwd,
        (char *) pwPrm->newPwd,
        MAX_PWD_NUM
      );
      cmhPrm[srcId].ssCmdPrm.CXXXnewPwd[MAX_PWD_NUM] = '\0';

      strncpy
      (
        (char *) cmhPrm[srcId].ssCmdPrm.CXXXnewPwd2,
        (char *) pwPrm->newPwd2,
        MAX_PWD_NUM
      );
      cmhPrm[srcId].ssCmdPrm.CXXXnewPwd2[MAX_PWD_NUM] = '\0';
      break;

    /*
     *-------------------------------------------------------------------
     * if password relates to SIM
     *-------------------------------------------------------------------
     */
    case( KSD_SS_PIN1     ):
    case( KSD_SS_PIN2     ):

      if( simEntStat.curCmd NEQ AT_CMD_NONE )

        return( AT_BUSY );

      if( strlen( (char *) pwPrm->oldPwd ) > PIN_LEN  OR
          strlen( (char *) pwPrm->newPwd ) > PIN_LEN     )

        return( AT_FAIL );

      /* Marcus: Issue 1589: 28/01/2003: Start */
      if( strlen( (char *) pwPrm->oldPwd ) < MIN_PIN_LEN  OR
          strlen( (char *) pwPrm->newPwd ) < MIN_PIN_LEN     )

        return( AT_FAIL );
      /* Marcus: Issue 1589: 28/01/2003: End */

      /* if newPwd2 (verify new PIN ) is given, then check if
         newPwd2 is identical with newPwd */
      if( strlen( (char *) pwPrm->newPwd2 ) > 0 )
      {
        if( strlen( (char *) pwPrm->newPwd ) > PIN_LEN )
        {
          return( AT_FAIL );
        }
        if( strcmp((char *)pwPrm->newPwd, (char *)pwPrm->newPwd2) )
        {
          TRACE_EVENT_P2("new PIN (%s) differs from verify new PIN (%s)",
            (char *)pwPrm->newPwd, (char *)pwPrm->newPwd2 );
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_WrongPasswd);
          return( AT_FAIL );
        }
      }

      cmhSIM_FillInPIN ( (char *) pwPrm->oldPwd, simShrdPrm.setPrm[srcId].curPIN, PIN_LEN );
      cmhSIM_FillInPIN ( (char *) pwPrm->newPwd, simShrdPrm.setPrm[srcId].newPIN, PIN_LEN );

      simShrdPrm.setPrm[srcId].PINType = (pwPrm->ssCd EQ KSD_SS_PIN1)?
                                 PHASE_2_PIN_1:PHASE_2_PIN_2;

      if ( simShrdPrm.setPrm[srcId].PINType EQ PHASE_2_PIN_1 AND
           simShrdPrm.PEDStat               EQ PEDS_DIS )
      {
        TRACE_EVENT( "FATAL RETURN PIN not enabled" );
        return( AT_FAIL );
      }

      simEntStat.curCmd     = (T_ACI_AT_CMD)KSD_CMD_PWD;
      simShrdPrm.owner     = (T_OWN)srcId;
      simEntStat.entOwn     =  srcId;

      if( psaSIM_ChangePIN() < 0 )  /* change PIN */
      {
        TRACE_EVENT( "FATAL RETURN psaSIM in key sequence" );
        return( AT_FAIL );
      }
      break;

    default:
      TRACE_EVENT("unexpected SS code");
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  if( srcId NEQ ((T_ACI_CMD_SRC)OWN_SRC_SAT) )
  {
  T_ACI_CLOG cmdLog;        /* holds logging info */

  cmdLog.atCmd                = (T_ACI_AT_CMD)KSD_CMD_PWD;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = sId+1;
  cmdLog.cmdPrm.sKSPW.srcId   = srcId;
  cmdLog.cmdPrm.sKSPW.ssCd    = pwPrm->ssCd;
  cmdLog.cmdPrm.sKSPW.oldPwd  = pwPrm->oldPwd;
  cmdLog.cmdPrm.sKSPW.newPwd  = pwPrm->newPwd;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdUSSD            |
+--------------------------------------------------------------------+

  PURPOSE : This function invokes the operation for key sequence
            command for unstructured supplementary services.
*/

GLOBAL T_ACI_RETURN cmhSS_ksdUSSD  ( T_ACI_CMD_SRC srcId,
                                     T_ACI_KSD_USSD_PRM * ussdPrm )
{
  SHORT sId;                /* holds service id */
  T_ACI_RETURN retCd;       /* holds return code */
  UBYTE ussdLen;
  UBYTE *ussdString;
  UBYTE src_len;

  TRACE_FUNCTION ("cmhSS_ksdUSSD");

/*
 *-------------------------------------------------------------------
 * check for ussd string
 *-------------------------------------------------------------------
 */
  if( !ussdPrm->ussd )

    return( AT_FAIL );

/*
 *-------------------------------------------------------------------
 * get a new service table entry
 *-------------------------------------------------------------------
 */
  sId = psaSS_stbNewEntry();

  if( sId EQ NO_ENTRY )
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter operation code
 *-------------------------------------------------------------------
 */
  sId = psaSS_stbFindUssdReq();

  if( sId EQ NO_ENTRY )
  {
    /* check if there is another service in progress */
    if( psaSS_stbFindActSrv( sId ) NEQ NO_ENTRY )
      return( AT_FAIL );

    /* get new service table entry */
    sId = psaSS_stbNewEntry();
    if( sId EQ NO_ENTRY ) return( AT_FAIL );

    ssShrdPrm.ussdLen = strlen((char *)ussdPrm->ussd);

    if( !utl_cvtGsmIra ( ussdPrm->ussd,
                         ssShrdPrm.ussdLen,
                         ssShrdPrm.ussdBuf,
                         MAX_USSD_DATA,
                         CSCS_DIR_IraToGsm ) ) return( AT_FAIL );
    CCD_START;

    MALLOC(ussdString, MAX_USSD_STRING);

    src_len = (UBYTE)MINIMUM( MAX_USSD_STRING, ssShrdPrm.ussdLen); 
    /* to be sent as dcs = 0x0F: so always pack */
    ussdLen = utl_cvt8To7( ssShrdPrm.ussdBuf,
                           src_len,
                           ussdString, 0 );

    /* According to spec 23.038 section 6.1.2.3 for USSD packing, for bytes end with
     * (8*n)-1 i.e where n is 1,2,3....i.e byte 7, 15, 23 ... to be padded 
     * with carriage return <CR>(0xD) 
     */
    if ((src_len+1)%8 EQ 0)
    {
      ussdString[ussdLen-1] |= (0xD << 1);
    }

    psaSS_asmProcUSSDReq( 0x0F, ussdString, ussdLen );

    MFREE(ussdString);

    /* start new transaction */
    ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
    ssShrdPrm.stb[sId].curCmd     = (T_ACI_AT_CMD)KSD_CMD_USSD;
    ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;
    psaSS_NewTrns(sId);

    CCD_END;

    if(satShrdPrm.SIMCCParm.owner EQ OWN_SRC_SAT)
    {
      TRACE_EVENT("OWN_SRC_SAT in ksdUSSD");
      /* some more info needed if sat initiated USSD*/
      satShrdPrm.SentUSSDid = sId;
    }
    retCd = AT_CMPL;
  }
  else
  {
    ssShrdPrm.ussdLen = strlen((char *)ussdPrm->ussd);

    if( !utl_cvtGsmIra ( ussdPrm->ussd,
                         ssShrdPrm.ussdLen,
                         ssShrdPrm.ussdBuf,
                         MAX_USSD_DATA,
                         CSCS_DIR_IraToGsm ) ) return( AT_FAIL );

    CCD_START;

    psaSS_asmCnfUSSDReq( 0x0F, ssShrdPrm.ussdBuf, ssShrdPrm.ussdLen );

    ssShrdPrm.stb[sId].ussdReqFlg = FALSE;

    /* continue existing transaction */
    psaSS_CntTrns(sId);

    CCD_END;

    retCd = AT_CMPL;
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  if( srcId NEQ ((T_ACI_CMD_SRC)OWN_SRC_SAT) )
  {
  T_ACI_CLOG cmdLog;        /* holds logging info */

  cmdLog.atCmd                = (T_ACI_AT_CMD)KSD_CMD_USSD;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = retCd;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = sId+1;
  cmdLog.cmdPrm.sKSUS.srcId   = srcId;
  cmdLog.cmdPrm.sKSUS.ussd    = ussdPrm->ussd;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( retCd );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdUBLK            |
+--------------------------------------------------------------------+

  PURPOSE : This function invokes the operation for key sequence
            command for unblocking PIN.
*/

GLOBAL T_ACI_RETURN cmhSS_ksdUBLK  ( T_ACI_CMD_SRC srcId,
                                     T_ACI_KSD_UBLK_PRM * ublkPrm )
{
  TRACE_FUNCTION ("cmhSS_ksdUBLK");

/*
 *-------------------------------------------------------------------
 * check parameter puk
 *-------------------------------------------------------------------
 */
  if( !ublkPrm->puk OR strlen( (char *) ublkPrm->puk ) NEQ PUK_LEN )

    return( AT_FAIL );

/*
 *-------------------------------------------------------------------
 * check parameter pin
 *-------------------------------------------------------------------
 */
  if( !ublkPrm->pin OR strlen( (char *) ublkPrm->pin ) > PIN_LEN
                    OR strlen( (char *) ublkPrm->pin ) < MIN_PIN_LEN)

    return( AT_FAIL );

/*
 *-------------------------------------------------------------------
 * check parameter ss code
 *-------------------------------------------------------------------
 */
  switch( ublkPrm->ssCd )
  {
    case( KSD_SS_PIN1     ):
    case( KSD_SS_PIN2     ):

      break;

    default:

      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * unblock PIN
 *-------------------------------------------------------------------
 */
  if( simEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

  cmhSIM_FillInPIN ( (char *) ublkPrm->puk, simShrdPrm.setPrm[srcId].unblkKey, PUK_LEN );
  cmhSIM_FillInPIN ( (char *) ublkPrm->pin, simShrdPrm.setPrm[srcId].curPIN,   PIN_LEN );

  simShrdPrm.setPrm[srcId].PINType = (ublkPrm->ssCd EQ KSD_SS_PIN1)?
                                        PHASE_2_PUK_1:PHASE_2_PUK_2;

  simEntStat.curCmd     = (T_ACI_AT_CMD)KSD_CMD_UBLK;
  simShrdPrm.owner = (T_OWN)srcId;
  simEntStat.entOwn     = srcId;

  if( psaSIM_UnblockCard( ) < 0 )  /* verify PIN */
  {
    TRACE_EVENT( "FATAL RETURN psaSIM in KSD" );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  if( srcId NEQ ((T_ACI_CMD_SRC)OWN_SRC_SAT) )
  {
  T_ACI_CLOG cmdLog;        /* holds logging info */

  cmdLog.atCmd                = (T_ACI_AT_CMD)KSD_CMD_UBLK;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sKSUB.srcId   = srcId;
  cmdLog.cmdPrm.sKSUB.ssCd    = ublkPrm->ssCd;
  cmdLog.cmdPrm.sKSUB.puk     = ublkPrm->puk;
  cmdLog.cmdPrm.sKSUB.pin     = ublkPrm->pin;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdCL              |
+--------------------------------------------------------------------+

  PURPOSE : This function invokes the operation for key sequence
            command for calling line supplementary services.
*/

GLOBAL T_ACI_RETURN cmhSS_ksdCL  ( T_ACI_CMD_SRC srcId,
                                   T_ACI_KSD_CL_PRM * clPrm )
{
  SHORT sId;                /* holds service id */

  TRACE_FUNCTION ("cmhSS_ksdCL");

/*
 *-------------------------------------------------------------------
 * check parameter ss code
 *-------------------------------------------------------------------
 */
  switch( clPrm->ssCd )
  {
    case( KSD_SS_CLIP ):
    case( KSD_SS_CLIR ):
    case( KSD_SS_COLP ):
    case( KSD_SS_COLR ):
    case( KSD_SS_CNAP ):		

      break;

    default:

      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * get a new service table entry
 *-------------------------------------------------------------------
 */
  sId = psaSS_stbNewEntry();

  if( sId EQ NO_ENTRY )
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter operation code
 *-------------------------------------------------------------------
 */
  CCD_START;

  switch( clPrm->opCd )
  {
    /* interrogate call forwarding service */
    case( KSD_OP_IRGT  ):
      psaSS_asmInterrogateSS( clPrm->ssCd, SS_NO_PRM, SS_NO_PRM );
      break;

    /* activate call forwarding service */
    case( KSD_OP_ACT   ):
    /* deactivate call forwarding service */
    case( KSD_OP_DEACT ):
    /* register call forwarding service */
    case( KSD_OP_REG   ):
    /* erase call forwarding service */
    case( KSD_OP_ERS   ):
    /* unexpected operation code */
    default:

      CCD_END;
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * start transaction
 *-------------------------------------------------------------------
 */
  ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
  ssShrdPrm.stb[sId].ssCode     = clPrm->ssCd;
  ssShrdPrm.stb[sId].orgOPC     = clPrm->opCd;
  ssShrdPrm.stb[sId].opCode     = clPrm->opCd;
  ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;

  if (clPrm->ssCd EQ KSD_SS_CNAP)
  {
    ssShrdPrm.stb[sId].curCmd     = AT_CMD_CNAP;
  }
  else
  {
    ssShrdPrm.stb[sId].curCmd     = (T_ACI_AT_CMD)KSD_CMD_CL;
  }

  psaSS_NewTrns(sId);

  CCD_END;

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  if( srcId NEQ ((T_ACI_CMD_SRC)OWN_SRC_SAT) )
  {
  T_ACI_CLOG cmdLog;        /* holds logging info */

  cmdLog.atCmd                = (T_ACI_AT_CMD)KSD_CMD_CW;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = sId+1;
  cmdLog.cmdPrm.sKSCL.srcId   = srcId;
  cmdLog.cmdPrm.sKSCL.opCd    = clPrm->opCd;
  cmdLog.cmdPrm.sKSCL.ssCd    = clPrm->ssCd;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdCCBS            |
+--------------------------------------------------------------------+

  PURPOSE : This function invokes the operation for key sequence
            command for call completion to busy subscriber
            supplementary services.
*/

GLOBAL T_ACI_RETURN cmhSS_ksdCCBS  ( T_ACI_CMD_SRC srcId,
                                     T_ACI_KSD_CCBS_PRM * ccbsPrm )
{
  SHORT sId;                /* holds service id */

  TRACE_FUNCTION ("cmhSS_ksdCCBS");

/*
 *-------------------------------------------------------------------
 * check parameter ccbs index
 *-------------------------------------------------------------------
 */
  if( ccbsPrm->idx NEQ KSD_IDX_NONE AND
      (ccbsPrm->idx < 1 OR ccbsPrm->idx > 5 ))

    return( AT_FAIL );

/*
 *-------------------------------------------------------------------
 * get a new service table entry
 *-------------------------------------------------------------------
 */
  sId = psaSS_stbNewEntry();

  if( sId EQ NO_ENTRY )
  {
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter operation code
 *-------------------------------------------------------------------
 */
  CCD_START;

  switch( ccbsPrm->opCd )
  {
#if 0  /* For further study, so not yet used */
    case( KSD_OP_ACT ):
      psaSS_asmActivateCCBS( );
      break;
#endif

    /* interrogate CCBS */
    case( KSD_OP_IRGT ):
      psaSS_asmInterrogateSS( SS_CD_CCBS, SS_NO_PRM, SS_NO_PRM );
      ssShrdPrm.stb[sId].SSver = MNCC_SS_VERSION_2;
      break;

    /* deactivate CCBS */
    case( KSD_OP_DEACT ):
      psaSS_asmDeactivateCCBS( ccbsPrm -> idx );
      ssShrdPrm.stb[sId].SSver = MNCC_SS_VERSION_3;
      break;

    default:

      CCD_END;
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * start transaction
 *-------------------------------------------------------------------
 */
  ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
  ssShrdPrm.stb[sId].ssCode     = KSD_SS_CCBS;
  ssShrdPrm.stb[sId].orgOPC     = ccbsPrm->opCd;
  ssShrdPrm.stb[sId].opCode     = ccbsPrm->opCd;
  ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;

  ssShrdPrm.stb[sId].curCmd     = (T_ACI_AT_CMD)KSD_CMD_CCBS;

  psaSS_NewTrns(sId);

  CCD_END;

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  if( srcId NEQ ((T_ACI_CMD_SRC)OWN_SRC_SAT) )
  {
  T_ACI_CLOG cmdLog;        /* holds logging info */

  cmdLog.atCmd                = (T_ACI_AT_CMD)KSD_CMD_CCBS;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = sId+1;
  cmdLog.cmdPrm.sKSCC.srcId   = srcId;
  cmdLog.cmdPrm.sKSCC.opCd    = ccbsPrm->opCd;
  cmdLog.cmdPrm.sKSCC.ssCd    = KSD_SS_CCBS;
  cmdLog.cmdPrm.sKSCC.idx     = ccbsPrm->idx;

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( AT_EXCT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_getSumOf2Digits    |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to compute the sum of an even and
            an odd labeled digit acc. to GSM 02.16. Only used for
            computing the CD for an IMEI.
*/

LOCAL UBYTE cmhSS_getSumOf2Digits ( UBYTE digits )
{
  UBYTE dval;

  /* compute odd labeled digit */
  dval = 2 * ( digits & 0x0F );
  dval = dval - ( dval < 10 ? 0 : 9 );

  /*
     return sum of even labeled digit and
     already processed odd labeled digit
  */
  return dval + ( ( digits >> 4 ) & 0x0F );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_getCdFromImei      |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to compute the CD for an IMEI
            according to GSM 02.16.
*/

GLOBAL UBYTE cmhSS_getCdFromImei ( T_ACI_IMEI* imei )
{
  UBYTE sum = 0; /* sum of step 2    */
  UBYTE cd;      /* Luhn Check Digit */

  /* Step 1 and 2 of CD computation acc. GSM 02.16 */
  sum += cmhSS_getSumOf2Digits ( imei -> tac1 );
  sum += cmhSS_getSumOf2Digits ( imei -> tac2 );
  sum += cmhSS_getSumOf2Digits ( imei -> tac3 );
  sum += cmhSS_getSumOf2Digits ( imei -> fac  );
  sum += cmhSS_getSumOf2Digits ( imei -> snr1 );
  sum += cmhSS_getSumOf2Digits ( imei -> snr2 );
  sum += cmhSS_getSumOf2Digits ( imei -> snr3 );

  /* Step 3 of CD computation acc. GSM 02.16 */
  cd = 10 - ( sum % 10 );
  cd = ( cd EQ 10 ? 0 : cd );

  return ( cd );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdIMEI            |
+--------------------------------------------------------------------+

  PURPOSE : This function invokes the operation for key sequence
            command for getting the IMEI.
*/

GLOBAL T_ACI_RETURN cmhSS_ksdIMEI( T_ACI_CMD_SRC srcId )
{
  UBYTE        IMEIBuf[CL_IMEI_SIZE];      /* IMEI buffer */
  EF_IMEI     *pIMEI;                      /* points to IMEI */
  BYTE        retVal;                      /* holds return value */
  T_ACI_KSIR   ksStat;                     /* ksd status */

  pIMEI = (EF_IMEI *)IMEIBuf;
  /*
   * ACI-SPR-19706: Get IMEISV from IMEI common library
   */
  retVal = cl_get_imeisv(CL_IMEI_SIZE, IMEIBuf, CL_IMEI_GET_STORED_IMEI);
  if(retVal != CL_IMEI_OK)
  {
    TRACE_ERROR("ACI ERROR: no IMEISV available!");
    return( AT_FAIL );
  }  
  TRACE_EVENT("ACI INFO IMEI: got IMEI");
  TRACE_EVENT_P8("%02x%02x%02x%02x %02x%02x%02x %02x",
                 IMEIBuf[0], IMEIBuf[1], IMEIBuf[2], IMEIBuf[3],
                 IMEIBuf[4], IMEIBuf[5], IMEIBuf[6], IMEIBuf[7]);

  ksStat.ksdCmd          = KSD_CMD_IMEI;
  ksStat.ir.rKSIMEI.tac1 = pIMEI->tac1;
  ksStat.ir.rKSIMEI.tac2 = pIMEI->tac2;
  ksStat.ir.rKSIMEI.tac3 = pIMEI->tac3;
  ksStat.ir.rKSIMEI.fac  = pIMEI->fac;
  ksStat.ir.rKSIMEI.snr1 = pIMEI->snr1;
  ksStat.ir.rKSIMEI.snr2 = pIMEI->snr2;
  ksStat.ir.rKSIMEI.snr3 = pIMEI->snr3;
  ksStat.ir.rKSIMEI.svn  = pIMEI->svn;
  ksStat.ir.rKSIMEI.cd   = cmhSS_getCdFromImei ( &ksStat.ir.rKSIMEI );

  /*
  ** CQ12314 : NDH : 23/9/2003
  ** Added srcID field to ksStat to enable called entity to determine the originator of the command
  ** and take appropriate action.  (eg to Activate Call Forwarding Icon)
  */
  ksStat.srcId = srcId;

#if defined (MFW)
  if (srcId NEQ CMD_SRC_LCL)
  {
    R_AT( RAT_KSIR, CMD_SRC_LCL)( &ksStat );
  }
#endif

  R_AT( RAT_KSIR, srcId )( &ksStat );

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdGetOpCd         |
+--------------------------------------------------------------------+

  PURPOSE : Transforms SS operation code into KSD operation code.
            Returns KSD operation code.
*/
#if(0)
GLOBAL UBYTE cmhSS_ksdGetOpCd  ( UBYTE opcSS )
{
  switch( opcSS )
  {
    case( OPC_REGISTER_SS ):        return( KSD_OP_REG );
    case( OPC_ERASE_SS ):           return( KSD_OP_ERS );
    case( OPC_ACTIVATE_SS ):        return( KSD_OP_ACT );
    case( OPC_DEACTIVATE_SS ):      return( KSD_OP_DEACT );
    case( OPC_INTERROGATE_SS ):     return( KSD_OP_IRGT );
    case( OPC_REGISTER_PASSWORD ):  return( KSD_OP_REG );
  }

  return( KSD_OP_NONE );
}
#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdBuildErrRslt    |
+--------------------------------------------------------------------+

  PURPOSE : This function build an error result for a failed KSD
            action.
*/

GLOBAL void cmhSS_ksdBuildErrRslt  ( SHORT sId, T_ACI_KSIR * ksStat, UBYTE err )
{

/*
 *-------------------------------------------------------------------
 * find error value
 *-------------------------------------------------------------------
 */
  if( err EQ KSD_NO_ERROR )
  {
    switch( ssShrdPrm.stb[sId].failType )
    {
      case( SSF_SS_ERR ):

        err = ssShrdPrm.stb[sId].errCd;
        break;

      case( SSF_GEN_PRB ):

        err = ssShrdPrm.stb[sId].rejPrb + 0xA0;
        break;

      case( SSF_INV_PRB ):

        err = ssShrdPrm.stb[sId].rejPrb + 0xB0;
        break;

      case( SSF_RSL_PRB ):

        err = ssShrdPrm.stb[sId].rejPrb + 0xC0;
        break;

      case( SSF_ERR_PRB ):

        err = ssShrdPrm.stb[sId].rejPrb + 0xD0;
        break;

      case( SSF_CCD_DEC ):

        err = KSD_ERR_FATAL_CCD_DEC;
        break;

      case( SSF_SS_ENT ):

        err = KSD_ERR_FATAL_SS_ENT;
        break;
    }
  }

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch((T_ACI_KSD_CMD) ssShrdPrm.stb[sId].curCmd )
  {
   /*
    *----------------------------------------------------------------
    * process result for KSD CF command
    *----------------------------------------------------------------
    */
    case( KSD_CMD_CF  ):

      ksStat->ksdCmd = KSD_CMD_CF;
      ksStat->ir.rKSCF.opCd  = ssShrdPrm.stb[sId].orgOPC;
      ksStat->ir.rKSCF.ssCd  = ssShrdPrm.stb[sId].ssCode;
      ksStat->ir.rKSCF.ssErr = err;
      break;
   /*
    *----------------------------------------------------------------
    * process result for KSD CB command
    *----------------------------------------------------------------
    */
    case( KSD_CMD_CB  ):

      ksStat->ksdCmd = KSD_CMD_CB;
      ksStat->ir.rKSCB.opCd  = ssShrdPrm.stb[sId].orgOPC;
      ksStat->ir.rKSCB.ssCd  = ssShrdPrm.stb[sId].ssCode;
      ksStat->ir.rKSCB.ssErr = err;
      break;
   /*
    *----------------------------------------------------------------
    * process result for KSD CL command
    *----------------------------------------------------------------
    */
    case( KSD_CMD_CL  ):

      ksStat->ksdCmd = KSD_CMD_CL;
      ksStat->ir.rKSCL.opCd  = ssShrdPrm.stb[sId].orgOPC;
      ksStat->ir.rKSCL.ssCd     = ssShrdPrm.stb[sId].ssCode;
      ksStat->ir.rKSCL.ssErr    = err;
      ksStat->ir.rKSCL.ssSt     = KSD_ST_NOT_VALID;
      ksStat->ir.rKSCL.clirOpt  = KSD_CO_NOT_VALID;
      ksStat->ir.rKSCL.ovrdCtg  = KSD_OVR_CAT_NOT_VALID;
      break;
   /*
    *----------------------------------------------------------------
    * process result for KSD CW command
    *----------------------------------------------------------------
    */
    case( KSD_CMD_CW  ):

      ksStat->ksdCmd = KSD_CMD_CW;
      ksStat->ir.rKSCW.opCd  = ssShrdPrm.stb[sId].orgOPC;
      ksStat->ir.rKSCW.ssCd  = ssShrdPrm.stb[sId].ssCode;
      ksStat->ir.rKSCW.ssErr = err;
      ksStat->ir.rKSCW.ssSt  = KSD_ST_NOT_VALID;
      break;
   /*
    *----------------------------------------------------------------
    * process result for KSD PW command
    *----------------------------------------------------------------
    */
    case( KSD_CMD_PWD  ):

      ksStat->ksdCmd = KSD_CMD_PWD;
      ksStat->ir.rKSPW.opCd  = ssShrdPrm.stb[sId].orgOPC;
      ksStat->ir.rKSPW.ssCd  = ssShrdPrm.stb[sId].ssCode;
      ksStat->ir.rKSPW.ssErr = err;
      if(ssShrdPrm.stb[sId].errPrms NEQ NOT_PRESENT_8BIT)
        ksStat->ir.rKSPW.errPrms = ssShrdPrm.stb[sId].errPrms;

      break;
   /*
    *----------------------------------------------------------------
    * process result for KSD USSD command
    *----------------------------------------------------------------
    */
    case( KSD_CMD_USSD  ):

      ksStat->ksdCmd = KSD_CMD_USSD;
      ksStat->ir.rKSUS.ssErr = err;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdFillFwdFeatList |
+--------------------------------------------------------------------+

  PURPOSE : This function fills in the values of a forwarding
            feature list and returns the number of filled entries.
*/

GLOBAL UBYTE cmhSS_ksdFillFwdFeatList( T_forwardingFeatureList * ffSS,
                                        T_CF_FEAT * cfFeat )
{
  UBYTE idx;          /* holds list index */

  for( idx=0;
       idx < MINIMUM( ffSS->c_ff, MAX_CF_FEAT_NR );
       idx++, cfFeat++ )
  {
    cmhSS_GetBscSrv( &ffSS->ff[idx].basicService,
                     &cfFeat->bsCd,
                     &cfFeat->bsTp  );

    if( !ffSS->ff[idx].v_ssStatus )
      cfFeat->ssSt  = KSD_ST_NOT_VALID;
    else
      cfFeat->ssSt  = ffSS->ff[idx].ssStatus;

    if( ffSS->ff[idx].v_forwardedToNumber AND
        ffSS->ff[idx].forwardedToNumber.c_bcdDigit )
    {
      utl_BCD2DialStr
        (ffSS->ff[idx].forwardedToNumber.bcdDigit,
         (char*)cfFeat->num,
         ffSS->ff[idx].forwardedToNumber.c_bcdDigit);

      cfFeat->npi = ffSS->ff[idx].forwardedToNumber.npi;
      cfFeat->ton = ffSS->ff[idx].forwardedToNumber.noa;
    }
    else
    {
      cfFeat->npi = 0xFF;
      cfFeat->ton = 0xFF;
    }

    if( ffSS->ff[idx].v_forwardedToSubaddress AND
        ffSS->ff[idx].forwardedToSubaddress.c_subadr_str )
    {
      utl_BCD2DialStr
        (ffSS->ff[idx].forwardedToSubaddress.subadr_str,
         (char*)cfFeat->sub,
         ffSS->ff[idx].forwardedToSubaddress.c_subadr_str);

      cfFeat->tos = ffSS->ff[idx].forwardedToSubaddress.tos;
      cfFeat->oe  = ffSS->ff[idx].forwardedToSubaddress.oei;
    }
    else
    {
      cfFeat->tos = 0xFF;
      cfFeat->oe  = 0xFF;
    }

    if( ffSS->ff[idx].v_noReplyConditionTime )
      cfFeat->time = ffSS->ff[idx].noReplyConditionTime;
    else
      cfFeat->time = 0xFF;
  }

  return( idx );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdFillCbFeatList  |
+--------------------------------------------------------------------+

  PURPOSE : This function fills in the values of a call barring
            feature list and returns the number of filled entries.
*/

GLOBAL UBYTE cmhSS_ksdFillCbFeatList( T_callBarringFeatureList * bfSS,
                                      T_CB_INFO * cbInfo )
{
  UBYTE idx;          /* holds list index */

  for( idx=0;
       idx < MINIMUM( bfSS->c_cbf, MAX_CB_INFO_NR);
       idx++, cbInfo++ )
  {
    cbInfo->ssSt = (bfSS->cbf[idx].v_ssStatus)?
                    bfSS->cbf[idx].ssStatus :
                    (UBYTE)KSD_ST_NOT_VALID;

    cmhSS_GetBscSrv(&bfSS->cbf[idx].basicService,
                    &cbInfo->bsCd,
                    &cbInfo->bsTp  );
  }

  return( idx );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdFillBSGList     |
+--------------------------------------------------------------------+

  PURPOSE : This function fills in the values of a basic service
            group list and returns the number of filled entries.
*/

GLOBAL UBYTE cmhSS_ksdFillBSGList( T_basicServiceGroupList * bsgSS,
                                   T_Cx_BSG * cxBSG )
{
  UBYTE idx;          /* holds list index */
  UBYTE lstSpce;      /* holds list space */

  lstSpce = MAX_CW_BSG_NR;

  for( idx=0;
       idx < bsgSS->c_basicServiceGroupList_value AND lstSpce;
       idx++ )
  {
    if (bsgSS->basicServiceGroupList_value[idx].v_teleservice)
    {
      cxBSG->bsCd = bsgSS->basicServiceGroupList_value[idx].teleservice;
      cxBSG->bsTp = KSD_BS_TP_Tele;
      cxBSG++;
      lstSpce--;
    }
    if (bsgSS->basicServiceGroupList_value[idx].v_bearerService)
    {
      cxBSG->bsCd = bsgSS->basicServiceGroupList_value[idx].bearerService;
      cxBSG->bsTp = KSD_BS_TP_Bearer;
      cxBSG++;
      lstSpce--;
    }
  }

  return( MAX_CW_BSG_NR - lstSpce );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdFillCCBSFeatList|
+--------------------------------------------------------------------+

  PURPOSE : This function fills in the values of a forwarding
            feature list and returns the number of filled entries.
*/

GLOBAL UBYTE cmhSS_ksdFillCCBSFeatList( T_ccbsFeatureList * ccbsfSS,
                                        T_CC_FEAT * ccFeat )
{
  UBYTE idx;          /* holds list index */

  for( idx=0;
       idx < MINIMUM( ccbsfSS->c_ccbsf, MAX_CC_FEAT_NR );
       idx++, ccFeat++ )
  {
    cmhSS_GetBscSrv( (T_basicService*)&ccbsfSS->ccbsf[idx].basicServiceGroup,
                     &ccFeat->bsCd,
                     &ccFeat->bsTp  );

    if( ccbsfSS->ccbsf[idx].v_b_subscriberNumber AND
        ccbsfSS->ccbsf[idx].b_subscriberNumber.c_bcdDigit )
    {
      utl_BCD2DialStr
        (ccbsfSS->ccbsf[idx].b_subscriberNumber.bcdDigit,
         (char*)ccFeat->num,
         ccbsfSS->ccbsf[idx].b_subscriberNumber.c_bcdDigit);

      ccFeat->npi = ccbsfSS->ccbsf[idx].b_subscriberNumber.npi;
      ccFeat->ton = ccbsfSS->ccbsf[idx].b_subscriberNumber.noa;
    }
    else
    {
      ccFeat->npi = 0xFF;
      ccFeat->ton = 0xFF;
    }

    if( ccbsfSS->ccbsf[idx].v_b_subscriberSubaddress AND
        ccbsfSS->ccbsf[idx].b_subscriberSubaddress.c_subadr_str )
    {
      utl_BCD2DialStr
        (ccbsfSS->ccbsf[idx].b_subscriberSubaddress.subadr_str,
         (char*)ccFeat->sub,
         ccbsfSS->ccbsf[idx].b_subscriberSubaddress.c_subadr_str);

      ccFeat->tos = ccbsfSS->ccbsf[idx].b_subscriberSubaddress.tos;
      ccFeat->oe  = ccbsfSS->ccbsf[idx].b_subscriberSubaddress.oei;
    }
    else
    {
      ccFeat->tos = 0xFF;
      ccFeat->oe  = 0xFF;
    }

    if( ccbsfSS->ccbsf[idx].v_ccbsIndex )
      ccFeat->idx = ccbsfSS->ccbsf[idx].ccbsIndex;
    else
      ccFeat->idx = 0xFF;
  }

  return( idx );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdFillFwdRes      |
+--------------------------------------------------------------------+

  PURPOSE : This function fills in a forwarding info
            into a forwarding result structure.
*/

GLOBAL void cmhSS_ksdFillFwdRes( T_forwardingInfo * fiSS,
                                 T_ACI_KSIR *ksStat,
                                 T_CF_FEAT * cfFeat )
{
  /* if SS code is present */
  ksStat->ir.rKSCF.ssCd= (fiSS->v_ssCode)?
                          fiSS->ssCode:
                          (UBYTE)KSD_SS_NONE;

  /* if forwarding feature list is present, decode the forwarding
     information */
  if( fiSS->v_forwardingFeatureList )
  {
    /* no forwarding features present */
    if( fiSS->forwardingFeatureList.c_ff EQ 0 )
    {
      TRACE_EVENT("UNEXP: NO FORWARDING FEATURE IN FEATURE LIST");
    }

    /* fowarding feature list present */
    else
    {
      ksStat->ir.rKSCF.cfFeatLst   = cfFeat;

      ksStat->ir.rKSCF.c_cfFeatLst =
        cmhSS_ksdFillFwdFeatList( &fiSS->forwardingFeatureList, cfFeat );
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdFillCbRes       |
+--------------------------------------------------------------------+

  PURPOSE : This function fills in a barring info
            into a barring result structure.
*/

GLOBAL void cmhSS_ksdFillCbRes( T_callBarringInfo * biSS,
                                T_ACI_KSIR *ksStat,
                                T_CB_INFO * cbInfo )
{
  /* if SS code is present */
  ksStat->ir.rKSCB.ssCd= (biSS->v_ssCode)?
                          biSS->ssCode:
                          (UBYTE)KSD_SS_NONE;

  /* if barring feature list is present, decode the barring
     information */
  if( biSS->v_callBarringFeatureList )
  {
    /* no forwarding features present */
    if( biSS->callBarringFeatureList.c_cbf EQ 0 )
    {
      TRACE_EVENT("UNEXP: NO BARRING FEATURE IN FEATURE LIST");
    }

    /* fowarding feature list present */
    else
    {
      ksStat->ir.rKSCB.cbInfoLst = cbInfo;

      ksStat->ir.rKSCB.c_cbInfoLst =
        cmhSS_ksdFillCbFeatList( &biSS->callBarringFeatureList, cbInfo );
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SSS                  |
| STATE   : code                  ROUTINE : cmhSS_ksdFillCwRes       |
+--------------------------------------------------------------------+

  PURPOSE : This function fills in a SS data
            into a waiting result structure.
*/

GLOBAL void cmhSS_ksdFillCwRes( T_ssData   *datSS,
                                T_ACI_KSIR *ksStat,
                                T_Cx_BSG   *cwBSG )
{
  /* if SS code is present */
  ksStat->ir.rKSCW.ssCd= (datSS->v_ssCode)?
                          datSS->ssCode:
                          (UBYTE)KSD_SS_NONE;

  /* if SS status is present */
  ksStat->ir.rKSCW.ssSt= (datSS->v_ssStatus)?
                          datSS->ssStatus:
                          (UBYTE)KSD_ST_NONE;

  /* if basic service list is present, decode the basic service
     information */
  if( datSS->v_basicServiceGroupList )
  {
    /* no forwarding features present */
    if( datSS->basicServiceGroupList.c_basicServiceGroupList_value EQ 0 )
    {
      TRACE_EVENT("UNEXP: NO BASIC SERVICE IN SERVICE LIST");
    }

    /* fowarding feature list present */
    else
    {
      ksStat->ir.rKSCW.cwBSGLst = cwBSG;

      ksStat->ir.rKSCW.c_cwBSGLst =
        cmhSS_ksdFillBSGList( &datSS->basicServiceGroupList, cwBSG );
    }
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_GetClassType           |
+-------------------------------------------------------------------+

  PURPOSE : Convert class type value.

*/

GLOBAL T_ACI_CLASS cmhSS_GetClassType( UBYTE bsTp, UBYTE bsCd )
{
  TRACE_FUNCTION ("cmhSS_GetClassType()");

  if ( bsTp EQ KSD_BS_TP_None           AND
       bsCd EQ KSD_BS_TeleBearerUnknown     )
    return CLASS_VceDatFax;

  if (bsTp EQ KSD_BS_TP_Bearer)
  {
    return( CLASS_Dat );
  }
  if (bsTp EQ KSD_BS_TP_Tele)
  {
    switch( bsCd )
    {
      case( TS_CD_TLPHNY ):        /* no break */
      case( TS_CD_ALL_SPCH ):
        return( CLASS_Vce );
      case( TS_CD_PLMN1 ):
        return( CLASS_AuxVce);

      case( TS_CD_ALL_FAX ):
        return( CLASS_Fax );

      case( TS_CD_ALL_TS ):        /* no break */
      case( TS_CD_ALL_XCPT_SMS ):
        return( CLASS_VceFax );

      default:
        return( CLASS_None );
    }
  }
  return( CLASS_None );
}

GLOBAL T_ACI_CLASS cmhSS_GetCbClassType( UBYTE bsTp, UBYTE bsCd )
{
  TRACE_FUNCTION ("cmhSS_GetCbClassType()");

  if ( bsTp EQ KSD_BS_TP_None           AND
       bsCd EQ KSD_BS_TeleBearerUnknown     )
    return CLASS_VceDatFaxSms;

  if (bsTp EQ KSD_BS_TP_Bearer)
  {
    return( CLASS_Dat );
  }
  if (bsTp EQ KSD_BS_TP_Tele)
  {
    switch( bsCd )
    {
      case( TS_CD_TLPHNY ):        /* no break */
      case( TS_CD_ALL_SPCH ):     return( CLASS_Vce );
      case( TS_CD_PLMN1 ):        return( CLASS_AuxVce);

      case( TS_CD_ALL_FAX ):      return( CLASS_Fax );

      case( TS_CD_ALL_XCPT_SMS ): return( CLASS_VceFax );

      case( TS_CD_ALL_SMS ):
      case( TS_CD_SMS_MT ):
      case( TS_CD_SMS_MO ):       return (CLASS_Sms );

      case( TS_CD_ALL_TS ):       return (CLASS_VceFaxSms );

      default:                    return( CLASS_None );
    }
  }
  return( CLASS_None );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : checkSSforFDN           |
+-------------------------------------------------------------------+

  PURPOSE : Check, if SS string exists in FDN phone book
                   Returns FALSE if not found

*/
BOOL checkSSforFDN(char *cSSString)
{

  T_CLPTY_PRM ssCldPty;

    if(cSSString NEQ NULL)
    {
      cmhCC_init_cldPty( &ssCldPty );
      cmh_bldCalPrms ( cSSString, &ssCldPty );
      /*
        *-------------------------------------------------------------------
        * check fixed dialing phonebook if enabled
        *-------------------------------------------------------------------
        */
      if((simShrdPrm.crdFun EQ SIM_FDN_ENABLED OR
          simShrdPrm.crdFun EQ SIM_FDN_BDN_ENABLED) )
      {
        if( ksd_isFDNCheckSeq ( ssCldPty.num ) )
         {
            if(!psaCC_phbNtryFnd( FDN, &ssCldPty ))
            {
              TRACE_EVENT ( "SS FDN check failed" );
              return FALSE;
            }
        }
      }
    }
    return TRUE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : checkFDNSendSS           |
+-------------------------------------------------------------------+

  PURPOSE : Check, if SS string exists in FDN phone book and send to STK if allowed
                   Returns AT_FAIL if FDN is not allowed or send to STK fails

*/
T_ACI_RETURN checkFDNSendSS(T_ACI_CMD_SRC srcId, CHAR *pSSString)
{
    /* If SS string should be checked, but not found in FDN - return error */
    if (!checkSSforFDN(pSSString)) 
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_FdnCheck );
      return AT_FAIL; 
    }
#ifdef SIM_TOOLKIT
    return cmhSS_SendToSAT(srcId, pSSString);
#else /* SIM_TOOLKIT */
    return AT_CMPL;
#endif /* SIM_TOOLKIT */

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : mapIdxToString           |
+-------------------------------------------------------------------+

  PURPOSE : Find the occurence of the record of type T_IDX_TO_STR_MAP with "idx" field 
                    equal index and returns field "cod"
                    If NULL is reurned, no record with correct "idx" has been found

*/
char *mapIdxToString(const T_IDX_TO_STR_MAP *table, BYTE index)
{
  int i=0;
  while (table[i].cod NEQ NULL)
  {
    if (table[i].idx EQ index)
      return table[i].cod;
    i++;
  }
  return NULL;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : mapIdxToDigit           |
+-------------------------------------------------------------------+

  PURPOSE : Find the occurence of the record of type T_IDX_TO_STR_MAP with "idx" field 
                    equal index and returns field "cod"

*/

BYTE mapIdxToDigit(const T_IDX_TO_DIG_MAP *table, BYTE index, BYTE * result)
{
  int i=0;
  while (table[i].cod NEQ NULL || table[i].idx NEQ NULL)
  {
    if (table[i].idx EQ index)
    {
      *result = table[i].cod;
      return 0;
    }
    i++;
  }    
  return(BYTE)-1;
}

#ifdef SIM_TOOLKIT
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_SendToSAT           |
+-------------------------------------------------------------------+

  PURPOSE : Send SS string to SIM

*/
T_ACI_RETURN cmhSS_SendToSAT(T_ACI_CMD_SRC srcId, CHAR *pSSString)
{
   T_CLPTY_PRM *cldPty_sim;
   T_ACI_RETURN   retVal;

    TRACE_EVENT_P1("cmhSS_SendToSAT SS String = %s", pSSString);
    
    retVal = AT_CMPL;
    
    MALLOC(cldPty_sim, sizeof(T_CLPTY_PRM));
    
    cmhCC_init_cldPty( cldPty_sim );
    cmh_bldCalPrms ( pSSString, cldPty_sim );

    if( ksd_isSATSscs (cldPty_sim->num) )
    {   
      retVal = cmhSAT_SSCntrlBySIM( cldPty_sim, (UBYTE)srcId );
    }

    MFREE(cldPty_sim);
    
    switch (retVal)
    {
      case AT_FAIL: 
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
        break;

      case AT_BUSY:
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimBusy);
        break;

      default:
        break;
    }
    return( retVal );

}
#endif /* SIM_TOOLKIT */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_CF_SAT_Handle           |
+-------------------------------------------------------------------+

  PURPOSE : Builds SS string from parameters of AT+CCFC command
                  and sends to SIM

*/
T_ACI_RETURN cmhSS_CF_SAT_Handle(T_ACI_CMD_SRC srcId,
                                    T_ACI_CCFC_RSN reason,
                                    T_ACI_CCFC_MOD mode,
                                    CHAR*          number,
                                    T_ACI_TOA*     type,
                                    T_ACI_CLASS    class_type,
                                    CHAR*          subaddr,
                                    T_ACI_TOS*     satype,
                                    SHORT          time)
{

  int ii;
  BYTE bMapResult;
 
  CHAR cSIString[MAX_DIAL_LEN];
  CHAR cSIbuf[MAX_DIAL_LEN];
  CHAR cSSString[MAX_DIAL_LEN];
  

  char *stReason;
  char *stMode;
  BYTE bBaseService;

 
  cSIString[0]= '\0';

  TRACE_EVENT("cmhSS_CF_SAT_Handle()");
 

  stReason = mapIdxToString(cCFRsnMapTable/*lint -e40 */, reason); 
  stMode = mapIdxToString(cModMapTable, mode);
  bMapResult = mapIdxToDigit(cCFBSMapTable, class_type, &bBaseService);


  if (stMode NEQ NULL AND stReason NEQ NULL) 
  {
    /* Build SI string */
    for (ii=0; ii<3; ii++)
    {
      cSIbuf[0] = '\0';
      if (ii EQ 0) /* Step 1 - check Time */ 
      {
        if (time NEQ 0 AND reason NEQ CCFC_RSN_Uncond 
            AND reason NEQ CCFC_RSN_Busy AND reason NEQ CCFC_RSN_NotReach) /*No time for these Service code*/ 
        {
          sprintf(cSIbuf, "%d", time);
        }
      }
      else if(ii EQ 1) /* Step 2 - check BS */
      {
        if (bMapResult EQ NULL)
          sprintf(cSIbuf, "%d", bBaseService);
      }
      else if(ii EQ 2) /* Step 3 - check DN */
      {
        if (number NEQ NULL)
          sprintf(cSIbuf, "%s", number);
      }
      strcat(cSIbuf, cSIString);
      if (strlen(cSIbuf) NEQ 0)  /* If some parameters are already filled...*/
      {
        sprintf(cSIString, "*%s", cSIbuf);
      }
    }

    sprintf(cSSString,"%s%s%s#",stMode, stReason, cSIString); /* Build SS string */
    
    /* check is FDN is on and string is in FDN phone book. If OK - send to STK */
    return checkFDNSendSS(srcId, cSSString);
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return AT_FAIL;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_CW_SAT_Handle           |
+-------------------------------------------------------------------+

  PURPOSE : Builds SS string from parameters of AT+CCWA command
                  and sends to SIM

*/
T_ACI_RETURN cmhSS_CW_SAT_Handle(T_ACI_CMD_SRC srcId,
                                    T_ACI_CCFC_MOD mode,
                                    T_ACI_CLASS    class_type)
{

  BYTE bMapResult;
  CHAR cSSString[MAX_DIAL_LEN];
  
  char *stMode;
  BYTE bBaseService;

  TRACE_EVENT("cmhSS_CW_SAT_Handle()");
  stMode = mapIdxToString(cModMapTable, mode); 
  bMapResult = mapIdxToDigit(cCFBSMapTable, class_type, &bBaseService); 


  if (stMode NEQ NULL) 
  {
    /* Build SS string */
    if (bMapResult NEQ 0)
      sprintf(cSSString,"%s43#",stMode); 
    else
      sprintf(cSSString,"%s43*%d#",stMode, bBaseService);

    /* check is FDN is on and string is in FDN phone book. If OK - send to STK */
    return checkFDNSendSS(srcId, cSSString);
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return AT_FAIL;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_CLIP_SAT_Handle           |
+-------------------------------------------------------------------+

  PURPOSE : Builds SS string from parameters of AT+CLIP command
                  and sends to SIM

*/
T_ACI_RETURN cmhSS_CLIP_SAT_Handle(T_ACI_CMD_SRC srcId)
{
    CHAR cSSString[] = "*#30#"; /* Only Query */ 
    
    TRACE_EVENT("cmhSS_CLIP_SAT_Handle()");
    
    /* check is FDN is on and string is in FDN phone book. If OK - send to STK */
    return checkFDNSendSS(srcId, cSSString);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_CLIR_SAT_Handle           |
+-------------------------------------------------------------------+

  PURPOSE : Builds SS string from parameters of AT+CLIR command
                  and sends to SIM

*/
T_ACI_RETURN cmhSS_CLIR_SAT_Handle(T_ACI_CMD_SRC srcId)
{
    CHAR cSSString[] = "*#31#"; /* Only Query */
     
    TRACE_EVENT("cmhSS_CLIR_SAT_Handle()");

    /* check is FDN is on and string is in FDN phone book. If OK - send to STK */
    return checkFDNSendSS(srcId, cSSString);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_COLP_SAT_Handle           |
+-------------------------------------------------------------------+

  PURPOSE : Builds SS string from parameters of AT+COLP command
                  and sends to SIM

*/
T_ACI_RETURN cmhSS_COLP_SAT_Handle(T_ACI_CMD_SRC srcId)
{
    CHAR cSSString[] = "*#76#"; /* Only Query */
    
    TRACE_EVENT("cmhSS_COLP_SAT_Handle()");

    /* check is FDN is on and string is in FDN phone book. If OK - send to STK */
    return checkFDNSendSS(srcId, cSSString);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_COLR_SAT_Handle           |
+-------------------------------------------------------------------+

  PURPOSE : Builds SS string from parameters of AT+COLR command
                  and sends to SIM

*/
T_ACI_RETURN cmhSS_COLR_SAT_Handle(T_ACI_CMD_SRC srcId)
{
    CHAR cSSString[] = "*#77#"; /* Only Query */
    
    TRACE_EVENT("cmhSS_COLR_SAT_Handle()");

    /* check is FDN is on and string is in FDN phone book. If OK - send to STK */
    return checkFDNSendSS(srcId, cSSString);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_CCBS_SAT_Handle           |
+-------------------------------------------------------------------+

  PURPOSE : Builds SS string from parameters of AT+CCBS command
                  and sends to SIM

*/
T_ACI_RETURN cmhSS_CCBS_SAT_Handle(T_ACI_CMD_SRC srcId, T_ACI_CCBS_MOD mode, SHORT idx)
{
    char *stMode;
    CHAR cSSString[MAX_DIAL_LEN];
    
    TRACE_EVENT("cmhSS_CCBS_SAT_Handle()");
    
    stMode = mapIdxToString(cModMapTable, mode);

    if (stMode NEQ NULL) 
    {
      /* Build SS string */
      if (idx EQ -1) /* According to GSM 02.30 Index can vary 1..5*/
        sprintf(cSSString,"%s37#",stMode); 
      else
        sprintf(cSSString,"%s37*%d#",stMode, idx);

      /* check is FDN is on and string is in FDN phone book. If OK - send to STK */
      return checkFDNSendSS(srcId, cSSString);
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return AT_FAIL;
    }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_CNAP_SAT_Handle           |
+-------------------------------------------------------------------+

  PURPOSE : Builds SS string from parameters of AT+CNAP command
                  and sends to SIM

*/
T_ACI_RETURN cmhSS_CNAP_SAT_Handle(T_ACI_CMD_SRC srcId)
{
    CHAR cSSString[] = "*#300#"; /* Only Query */
    
    TRACE_EVENT("cmhSS_CNAP_SAT_Handle()");

    /* check is FDN is on and string is in FDN phone book. If OK - send to STK */
    return checkFDNSendSS(srcId, cSSString);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SSF                      |
|                            ROUTINE : cmhSS_Call_Barr_SAT_Handle           |
+-------------------------------------------------------------------+

  PURPOSE : Builds SS string from parameters of AT+CLCK command
                  and sends to SIM

*/
T_ACI_RETURN cmhSS_Call_Barr_SAT_Handle(T_ACI_CMD_SRC srcId, 
                        T_ACI_CLCK_MOD mode, 
                        T_ACI_FAC fac, 
                        CHAR  *passwd, 
                        T_ACI_CLASS    class_type)
{
    int ii;
    BYTE bBaseService;
    BYTE bMapResult;

    char *stMode;
    char *stFac;
  
    CHAR cSIString[MAX_DIAL_LEN];
    CHAR cSIbuf[MAX_DIAL_LEN];
    CHAR cSSString[MAX_DIAL_LEN];

    
    TRACE_EVENT("cmhSS_Call_Barr_SAT_Handle()");
    
    cSIString[0]= '\0';
    
    stMode = mapIdxToString(cModMapTable, mode);
    stFac = mapIdxToString(cCBRsnMapTable, fac); 
    bMapResult = mapIdxToDigit(cCFBSMapTable, class_type, &bBaseService);

    if (stFac EQ NULL ) return AT_CMPL; /* If no call barring facility - continue executing sAT_PlusCLCK*/

    if (stMode NEQ NULL) 
    {
      for (ii=0; ii<2; ii++)
      {
        cSIbuf[0] = '\0';
        if(ii EQ 0) /* Step 2 - check BS */
        {
          if (bMapResult EQ NULL)
            sprintf(cSIbuf, "%d", bBaseService);
        }
        else if(ii EQ 1) /* Step 2 - check PW */
        {
          if (passwd NEQ NULL)
            sprintf(cSIbuf, "%s", passwd);
        }
        strcat(cSIbuf, cSIString);
        if (strlen(cSIbuf) NEQ 0)  /* If some parameters are already filled...*/
        {
          sprintf(cSIString, "*%s", cSIbuf);
        }
      }
      
      /* Build SS string */
      sprintf(cSSString,"%s%s%s#",stMode, stFac, cSIString); /* Build SS string */

    /* check is FDN is on and string is in FDN phone book. If OK - send to STK */
    return checkFDNSendSS(srcId, cSSString);
    
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return AT_FAIL;
    }
}

/* Implements Measure # 166 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SS                       |
| STATE   : code             ROUTINE : cmhSMS_getSSCd               |
+-------------------------------------------------------------------+
  PARAMETERS  : fac - CLCK facility for which SS code has to be 
                      returned.
                ssCd - SS Code is filled in this variable. 
  RETURN      : FALSE if unknown CLCK facility is passed 

  PURPOSE     : This function returns the SS code for CLCK functions
  
*/

GLOBAL BOOL cmhSS_getSSCd(T_ACI_FAC fac, UBYTE *ssCd)
{
/*
 *-------------------------------------------------------------------
 * check parameter <fac>
 *-------------------------------------------------------------------
 */  
  switch( fac )
  {
    case( FAC_Ao ): *ssCd = SS_CD_BAOC;     break;
    case( FAC_Oi ): *ssCd = SS_CD_BOIC;     break;
    case( FAC_Ox ): *ssCd = SS_CD_BOICXH;   break;
    case( FAC_Ai ): *ssCd = SS_CD_BAIC;     break;
    case( FAC_Ir ): *ssCd = SS_CD_BICRM;    break;
    case( FAC_Ab ): *ssCd = SS_CD_ALL_CBSS; break;
    case( FAC_Ag ): *ssCd = SS_CD_BOC;      break;
    case( FAC_Ac ): *ssCd = SS_CD_BIC;      break;

    case( FAC_Sc ):
    case( FAC_Fd ):
    case( FAC_Al ):
#ifdef SIM_PERS
    case( FAC_Pn ):
    case( FAC_Pu ):
    case( FAC_Pc ):
    case( FAC_Pp ):
    case( FAC_Ps ):
    case( FAC_Pf ):
    case( FAC_Bl ):
    case( FAC_Mu ):
    case( FAC_Mum ):
#endif
#ifdef FF_PHONE_LOCK
    case( FAC_Pl ):
    case( FAC_Apl):
#endif
      *ssCd = NOT_PRESENT_8BIT; break;

    case( FAC_NotPresent  ):
    default:

      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( FALSE );
  }
  return TRUE;
}

#if defined (SIM_PERS) || defined (FF_PHONE_LOCK)

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SS                       |
| STATE   : code             ROUTINE : cmhSS_check_oper_result               |
+-------------------------------------------------------------------+
  PARAMETERS  : result - CLCK operation result for SIMP and Phone Lock .

   RETURN      :  AT_CMPL for success and AT_FAIL for Failure case

  PURPOSE     : This function returns AT_CMPL for success case and 
                        returns  AT_FAIL for Failure cases  with proper  Error code
  
*/

 T_ACI_RETURN cmhSS_check_oper_result(T_OPER_RET_STATUS result)
  {

    switch( result )
      { 
       case( OPER_SUCCESS ): 
        return ( AT_CMPL );
			
       case( OPER_WRONG_PASSWORD ): 
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme,CME_ERR_WrongPasswd );
        return( AT_FAIL );  
      }
       case( OPER_BUSY ): 
     {
            ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Busy );
            return( AT_FAIL );
     }
  	 
       default:
       ACI_ERR_DESC( ACI_ERR_CLASS_Cme,CME_ERR_Unknown );
       return( AT_FAIL );
     
     }
     
  }

#endif

/* Implements Measure # 38 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SS                       |
| STATE   : code             ROUTINE : cmhSS_ksdCWCBStartTrans      |
+-------------------------------------------------------------------+
  PARAMETERS  : srcId - source of AT Command
                ksdCmd- Contains KSD command identifier. Used to 
                        identify whether to operate on key sequence 
                        command for CB or for CW supplementary services

                seqPrm - T_KSD_SEQPARAM structure containing the CB 
                         and CW parameter information 
  RETURN      : Status of operation
  PURPOSE     : This function invokes the operation for key sequence
                command for CB and CW supplementary services.
*/

GLOBAL T_ACI_RETURN cmhSS_ksdCWCBStartTrans(T_ACI_CMD_SRC  srcId,  
                                           T_ACI_KSD_CMD  ksdCmd,
                                           T_KSD_SEQPARAM *seqPrm)
{
  SHORT sId;                /* holds service id */
  UBYTE bst,bs, opCd, ssCd, bsCd;

  if(ksdCmd EQ KSD_CMD_CB)
  {
   /*
    *-------------------------------------------------------------------
    * check parameter ss code
    *-------------------------------------------------------------------
    */
    switch( seqPrm->cb.ssCd )
    {
      case( KSD_SS_BOC      ):
      case( KSD_SS_BOICXH   ):
      case( KSD_SS_BOIC     ):
      case( KSD_SS_ALL_CBSS ):
      case( KSD_SS_BAOC     ):
      case( KSD_SS_BIC      ):
      case( KSD_SS_BAIC     ):
      case( KSD_SS_BICRM    ):

        break;

      default:

        return( AT_FAIL );
    }
    opCd = seqPrm->cb.opCd;
    ssCd = seqPrm->cb.ssCd;  
    bsCd = seqPrm->cb.bsCd;  
  }
  else if(ksdCmd EQ KSD_CMD_CW)
  {
    opCd = seqPrm->cw.opCd;
    ssCd = KSD_SS_CW;  
    bsCd = seqPrm->cw.bsCd; 
  }
  else
  {
    return( AT_FAIL );
  }
  /*
   *-------------------------------------------------------------------
   * check parameter basic service code
   *-------------------------------------------------------------------
   */
  if( !cmhSS_CheckBscSrv( bsCd, &bs, &bst, NULL ))
  {
    return( AT_FAIL );
  }
  /*
   *-------------------------------------------------------------------
   * get a new service table entry
   *-------------------------------------------------------------------
   */
  sId = psaSS_stbNewEntry();

  if( sId EQ NO_ENTRY )
  {
    return( AT_FAIL );
  }

  /*
   *-------------------------------------------------------------------
   * check parameter operation code
   *-------------------------------------------------------------------
   */
  CCD_START;

  switch( opCd )
  {
    /* interrogate call forwarding service */
    case( KSD_OP_IRGT  ):
      psaSS_asmInterrogateSS( ssCd, bst, bs );
      break;

    /* activate call forwarding service */
    case( KSD_OP_ACT   ):
      psaSS_asmActivateSS( ssCd, bst, bs );
      break;

    /* deactivate call forwarding service */
    case( KSD_OP_DEACT ):
      psaSS_asmDeactivateSS( ssCd, bst, bs );
      break;

    /* register call forwarding service */
    case( KSD_OP_REG   ):
    /* erase call forwarding service */
    case( KSD_OP_ERS   ):
    /* unexpected operation code */
    default:

      CCD_END;
      return( AT_FAIL );
  }

  /*
   *-------------------------------------------------------------------
   * start transaction
   *-------------------------------------------------------------------
   */
  ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
  ssShrdPrm.stb[sId].ssCode     = ssCd;
  ssShrdPrm.stb[sId].orgOPC     = opCd;
  ssShrdPrm.stb[sId].opCode     = opCd;
  ssShrdPrm.stb[sId].srvOwn     = (T_OWN)srcId;
  ssShrdPrm.stb[sId].curCmd     = (T_ACI_AT_CMD)ksdCmd;

  psaSS_NewTrns(sId);

  CCD_END;
  /*
   *-------------------------------------------------------------------
   * store password for later use
   *-------------------------------------------------------------------
   */
  if(ksdCmd EQ KSD_CMD_CB)
  {
    if( seqPrm->cb.pwd )
    {
      strncpy
      (
        (char *) cmhPrm[srcId].ssCmdPrm.CXXXpwd,
        (char *) seqPrm->cb.pwd,
        MAX_PWD_NUM
      );
      cmhPrm[srcId].ssCmdPrm.CXXXpwd[MAX_PWD_NUM] = '\0';
    }
    else
    {
      /* No password was given reset to default */
/* Implements Measure#32: Row 1115 */
      strncpy
      (
        (char *) cmhPrm[srcId].ssCmdPrm.CXXXpwd,
        ffff_str,
        MAX_PWD_NUM
      );
      cmhPrm[srcId].ssCmdPrm.CXXXpwd[MAX_PWD_NUM] = '\0';
    }
  }
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  if( srcId NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT )
  {
    T_ACI_CLOG cmdLog;        /* holds logging info */
    cmdLog.cmdType              = CLOG_TYPE_Set;
    cmdLog.retCode              = AT_EXCT;
    cmdLog.cId                  = ACI_NumParmNotPresent;
    cmdLog.sId                  = sId+1;  
    cmdLog.atCmd                = (T_ACI_AT_CMD)ksdCmd;

    if(ksdCmd EQ KSD_CMD_CB)
    {
      cmdLog.cmdPrm.sKSCB.srcId   = srcId;
      cmdLog.cmdPrm.sKSCB.opCd    = seqPrm->cb.opCd;
      cmdLog.cmdPrm.sKSCB.ssCd    = seqPrm->cb.ssCd;
      cmdLog.cmdPrm.sKSCB.bsTp    = bst;
      cmdLog.cmdPrm.sKSCB.bsCd    = bs;
      cmdLog.cmdPrm.sKSCB.pwd     = seqPrm->cb.pwd;
    }
    else
    {
      cmdLog.cmdPrm.sKSCW.srcId   = srcId;
      cmdLog.cmdPrm.sKSCW.opCd    = seqPrm->cw.opCd;
      cmdLog.cmdPrm.sKSCW.bsTp    = bst;
      cmdLog.cmdPrm.sKSCW.bsCd    = bs;
    }
    rAT_PercentCLOG( &cmdLog );
  }
#endif

  return AT_EXCT;
}
/* Implements Measure # 85 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SS                       |
| STATE   : code             ROUTINE : cmhSS_checkCCFC_RSN          |
+-------------------------------------------------------------------+
  PARAMETERS  : reason - T_ACI_CCFC_RSN that has to be converted to 
                appropriate SS code
                ssCd - Parameter in which SS Code is filled
  RETURN      : Boolean value indicates FALSE if unknown T_ACI_CCFC_RSN 
                is sent
  PURPOSE     : This function converts the T_ACI_CCFC_RSN to the 
                appropriate ss Code.  
*/

GLOBAL BOOL cmhSS_checkCCFC_RSN(T_ACI_CCFC_RSN reason, UBYTE *ssCd)
{
/*
 *-------------------------------------------------------------------
 * check parameter <reason>
 *-------------------------------------------------------------------
 */
  switch( reason )
  {
    case( CCFC_RSN_Uncond      ): *ssCd = SS_CD_CFU;       break;
    case( CCFC_RSN_Busy        ): *ssCd = SS_CD_CFB;       break;
    case( CCFC_RSN_NoReply     ): *ssCd = SS_CD_CFNRY;     break;
    case( CCFC_RSN_NotReach    ): *ssCd = SS_CD_CFNRC;     break;
    case( CCFC_RSN_Forward     ): *ssCd = SS_CD_ALL_FWSS;  break;
    case( CCFC_RSN_CondForward ): *ssCd = SS_CD_ALL_CFWSS; break;

    case( CCFC_RSN_NotPresent  ):
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( FALSE );
  }
  return TRUE;
}

/*==== EOF ========================================================*/
