/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SMSF
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
|             handler for the short message service.
+-----------------------------------------------------------------------------
*/

#ifndef CMH_SMSF_C 
#define CMH_SMSF_C
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

#include "aci.h"
#include "aci_lst.h"
#include "aci_mem.h"
#include "psa.h"
#include "psa_cc.h"
#ifdef SIM_TOOLKIT
#include "psa_sat.h"
#endif
#include "psa_sms.h"
#include "psa_sim.h"
#include "psa_mmi.h"
#include "psa_util.h"
#include "phb.h"
#include "cmh.h"
#include "cmh_sms.h"
#include "pcm.h"

#include "aci_lst.h"

#if (defined (MFW) AND !defined (FF_MMI_RIV)) OR defined (_CONC_TESTING_)
#include "conc_sms.h"
#endif /* ##if (defined (MFW) AND !defined (FF_MMI_RIV)) OR defined (_CONC_TESTING_)*/


#ifdef UART
#include "dti_conn_mng.h"
#endif

#include "cmh_sim.h"
#ifndef _SIMULATION_

/* temporary solution to get ffs.h included without GPRS to be set ! */
#ifdef GPRS
#include "../../services/ffs/ffs.h"
#else
#include "../../services/ffs/ffs.h"
#undef GPRS
#endif /* GPRS */

#include "ffs_coat.h"

#endif /* !_SIMULATION_ */

#ifdef FF_CPHS
#include "cphs.h"
#endif /* FF_CPHS */

/*==== CONSTANTS ==================================================*/
#define CMMS_MODE_TIMER_VAL (5000)
#define SMS_CMH_YEAR_MAX   (99) /* maximum value for year in   */
                                /* absolute validity period    */
#define SMS_CMH_YEAR_MIN   (0)  /* minimum value for year in   */
                                /* absolute  validity period   */
#define SMS_CMH_MONTH_MAX  (12) /* maximum value for month in  */
                                /* absolute validity period    */
#define SMS_CMH_MONTH_MIN  (1)  /* minimum value for month in  */
                                /* absolute  validity period   */
#define SMS_CMH_DAY_MAX    (31) /* maximum value for day in    */
                                /* absolute validity period    */
#define SMS_CMH_DAY_MIN    (1)  /* minimum value for day in    */
                                /* absolute  validity period   */
#define SMS_CMH_HOUR_MAX   (23) /* maximum value for hour in   */
                                /* absolute validity period    */
#define SMS_CMH_HOUR_MIN   (0)  /* minimum value for hour in   */
                                /* absolute  validity period   */
#define SMS_CMH_MINUTE_MAX (59) /* maximum value for minute in */
                                /* absolute validity period    */
#define SMS_CMH_MINUTE_MIN (0)  /* minimum value for minute in */
                                /* absolute  validity period   */
#define SMS_CMH_SECOND_MAX (59) /* maximum value for second in */
                                /* absolute validity period    */
#define SMS_CMH_SECOND_MIN (0)  /* minimum value for second in */
                                /* absolute  validity period   */

#define SMS_CMH_TZ_MAX    (47)  /* maximum value for a time  */
                                /* zone in absolute validity */
                                /* period                    */
#define SMS_CMH_TZ_MIN    (-47) /* minimum value for a time  */
                                /* zone in absolute validity */
                                /* period                    */


/* macro for converting a two digit BCD into an UBYTE */
#define BCD2UBYTE(bcd) (UBYTE)(10 * bcd[0] + bcd[1])

/* macro for testing whether any digit of a two digit */
/* BCD lies outside of a predefined value range       */
#define NOTBCD(bcd) (bcd[0] > 9 OR\
                     bcd[1] > 9 )


#define L_MAX_UD      140
#define L_MAX_UD_CONC 134

#ifdef FF_CPHS_REL4
typedef enum 
{
  MSG_WAITING_TYPE_INVALID = -1,
  MSG_WAITING_TYPE_VOICE,
  MSG_WAITING_TYPE_FAX,
  MSG_WAITING_TYPE_EMAIL,
  MSG_WAITING_TYPE_OTHER
} T_MSG_WAITING_TYPE;
#endif

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/
EXTERN T_ACI_LIST *set_prm_list;

/*==== PROTOTYPES==================================================*/
/* Implements Measure # 73 */
LOCAL BOOL cmhSMS_findMessageIds (USHORT lower_mid, USHORT upper_mid);
/* Implements Measure # 126 */
LOCAL void cmhSMS_processOrigDestAddr (T_ACI_CMGL_SM  *sm, 
                                       T_rp_addr      *rp_addr,
                                       T_tp_da        *tp_addr );
/* Implements Measure # 9 */
LOCAL void cmhSMS_clearCbmPrm (void);


/*==== VARIABLES ==================================================*/

const char * const ffs_smsprfl_fname[] = { FFS_SMSPRFL_FNAME01,
                                    FFS_SMSPRFL_FNAME02,
                                    FFS_SMSPRFL_FNAME03,
                                    FFS_SMSPRFL_FNAME04 };

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : aci_encodeVpenh    |
+--------------------------------------------------------------------+

  PURPOSE : This function encodes a given T_ACI_VP_ENH type to a
            string.
*/
GLOBAL void aci_encodeVpenh ( CHAR*         vpenh_str,
                              T_ACI_VP_ENH* vpenh )
{
  UBYTE pos;

  /* functionality indicator */
  utl_binToHex (&vpenh->func_ind, 1, vpenh_str);
  pos = 2;

  /* extension octet */
  if (vpenh->func_ind & TP_VPF_ENH_EXT_BIT_MASK)
  {
    utl_binToHex (&vpenh->ext_oct, 1, vpenh_str+pos);
    pos += 2;
  }

  /* process validity period values */
  if ((vpenh->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_REL)
  {
    utl_binToHex (&vpenh->val.vpenh_relative, 1, vpenh_str+pos );
    pos += 2;
  }
  else if ((vpenh->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_SEC)
  {
    utl_binToHex (&vpenh->val.vpenh_seconds, 1, vpenh_str+pos );
    pos += 2;
  }
  else if ((vpenh->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_HRS)
  {
    vpenh_str[pos++] = vpenh->val.vpenh_hours.hour[1] + '0';
    vpenh_str[pos++] = vpenh->val.vpenh_hours.hour[0] + '0';

    vpenh_str[pos++] = vpenh->val.vpenh_hours.minute[1] + '0';
    vpenh_str[pos++] = vpenh->val.vpenh_hours.minute[0] + '0';

    vpenh_str[pos++] = vpenh->val.vpenh_hours.second[1] + '0';
    vpenh_str[pos++] = vpenh->val.vpenh_hours.second[0] + '0';
  }

  /* fill the rest with zeros */
  while (pos < 14)
  {
    vpenh_str[pos++] = '0';
  }

  /* string terminator */
  vpenh_str[pos] = '\0';
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_getNType         |
+-------------------------------------------------------------------+

  PURPOSE : This function calculates the 'numbering type' out of the
            current 'type of numbering'.
*/
GLOBAL UBYTE cmhSMS_getNType ( T_ACI_TOA_TON ton )
{
  switch( ton )
  {
    case( TON_International ):
    case( TON_National      ):
    case( TON_NetSpecific   ):
    case( TON_DedAccess     ):
    case( TON_Alphanumeric  ):
    case( TON_Abbreviated   ): return (UBYTE)ton;
    default:                   return SMS_TON_UNKNOWN;
  }
}


GLOBAL BOOL cmhSMS_findPrflId ( UBYTE critrerium, void* elem )
{
  T_SMS_SET_PRM *compared = (T_SMS_SET_PRM*)elem;
  if ( compared->prflId == critrerium )
    return TRUE;
  else
    return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_getTon           |
+-------------------------------------------------------------------+

  PURPOSE : This function calculates the 'type of numbering' out of
            the current 'numbering type'.
*/
GLOBAL T_ACI_TOA_TON cmhSMS_getTon ( UBYTE ntype )
{
  switch( ntype )
  {
    case( SMS_TON_UNKNOWN       ):
    case( SMS_TON_INTERNATIONAL ):
    case( SMS_TON_NATIONAL      ):
    case( SMS_TON_NETWORK_SPEC  ):
    case( SMS_TON_SUBSCRIBER    ):
    case( SMS_TON_ALPHANUMERIC  ):
    case( SMS_TON_ABBREVIATED   ): return (T_ACI_TOA_TON)ntype;
    default:                       return TON_NotPresent;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_getNPlan         |
+-------------------------------------------------------------------+

  PURPOSE : This function calculates the 'numbering plan' out of the
            current 'numbering plan identification'.
*/
GLOBAL UBYTE cmhSMS_getNPlan ( T_ACI_TOA_NPI npi )
{
  switch( npi )
  {
    case( NPI_IsdnTelephony ):
    case( NPI_Data          ):
    case( NPI_Telex         ):
    case( NPI_National      ):
    case( NPI_Private       ): return (UBYTE)npi;
    default:                   return NPI_Unknown;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_getNpi           |
+-------------------------------------------------------------------+

  PURPOSE : This function calculates the 'numbering plan
            identification' out of the current 'numbering plan'.
*/
GLOBAL T_ACI_TOA_NPI cmhSMS_getNpi ( UBYTE nplan )
{
  switch( nplan )
  {
    case( SMS_TON_UNKNOWN  ):
    case( SMS_NPI_ISDN     ):
    case( SMS_NPI_X121     ):
    case( SMS_NPI_F69      ):
    case( SMS_NPI_NATIONAL ):
    case( SMS_NPI_PRIVATE  ): return (T_ACI_TOA_NPI)nplan;
    default:                  return NPI_NotPresent;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_getStatCmh       |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the message status from
            PSA type to CMH type.
*/
GLOBAL void cmhSMS_getStatCmh ( UBYTE           inStat,
                                T_ACI_SMS_STAT* outStat )
{
  switch( inStat & STAT_MASK )
  {
    case( REC_UNREAD ): *outStat = SMS_STAT_RecUnread; break;
    case( REC_READ   ): *outStat = SMS_STAT_RecRead;   break;
    case( STO_UNSENT ): *outStat = SMS_STAT_StoUnsent; break;
    case( STO_SENT   ): *outStat = SMS_STAT_StoSent;   break;
    default:            *outStat = SMS_STAT_NotPresent;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_getStatPsa       |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the message status from
            CMH type to PSA type.
*/
GLOBAL BOOL cmhSMS_getStatPsa ( T_ACI_SMS_STAT inStat,
                                UBYTE*         outStat )
{
  switch( inStat )
  {
    case( SMS_STAT_RecUnread ): *outStat = REC_UNREAD; break;
    case( SMS_STAT_RecRead   ): *outStat = REC_READ;   break;
    case( SMS_STAT_StoUnsent ): *outStat = STO_UNSENT; break;
    case( SMS_STAT_StoSent   ): *outStat = STO_SENT;   break;
    default                   : return (FALSE);
  }

  return ( TRUE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_isVpabsVld       |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to process whether the given
            absolute validity period is valid.

            Note: This function checks whether every individual
                  time value is within a valid range. It does not
                  check whether the whole expression is valid.
*/
GLOBAL BOOL cmhSMS_isVpabsVld ( T_ACI_VP_ABS* vpabs )
{
  if ( BCD2UBYTE ( vpabs -> year   ) > SMS_CMH_YEAR_MAX   OR
       BCD2UBYTE ( vpabs -> month  ) < SMS_CMH_MONTH_MIN  OR
       BCD2UBYTE ( vpabs -> month  ) > SMS_CMH_MONTH_MAX  OR
       BCD2UBYTE ( vpabs -> day    ) < SMS_CMH_DAY_MIN    OR
       BCD2UBYTE ( vpabs -> day    ) > SMS_CMH_DAY_MAX    OR
       BCD2UBYTE ( vpabs -> hour   ) > SMS_CMH_HOUR_MAX   OR
       BCD2UBYTE ( vpabs -> minute ) > SMS_CMH_MINUTE_MAX OR
       BCD2UBYTE ( vpabs -> second ) > SMS_CMH_SECOND_MAX OR
       vpabs -> timezone             < SMS_CMH_TZ_MIN     OR
       vpabs -> timezone             > SMS_CMH_TZ_MAX     OR
       NOTBCD ( vpabs -> year   )                         OR
       NOTBCD ( vpabs -> month  )                         OR
       NOTBCD ( vpabs -> day    )                         OR
       NOTBCD ( vpabs -> hour   )                         OR
       NOTBCD ( vpabs -> minute )                         OR
       NOTBCD ( vpabs -> second )                            )

        return ( FALSE );

  return ( TRUE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_isVpenhVld       |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to process whether the given
            enhanced validity period is valid.

            Note: This function checks whether every individual
                  time value is within a valid range. It does not
                  check whether the whole expression is valid.
*/
GLOBAL BOOL cmhSMS_isVpenhVld ( T_ACI_VP_ENH* vpenh )
{
  if ((vpenh->func_ind & TP_VPF_ENH_FORMAT_MASK) > TP_VPF_ENH_HRS)
  {
    return FALSE;
  }

  if ((vpenh->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_HRS)
  {
    if ( BCD2UBYTE ( vpenh->val.vpenh_hours.minute ) > SMS_CMH_MINUTE_MAX OR
         BCD2UBYTE ( vpenh->val.vpenh_hours.second ) > SMS_CMH_SECOND_MAX OR
         NOTBCD ( vpenh->val.vpenh_hours.hour   )                         OR
         NOTBCD ( vpenh->val.vpenh_hours.minute )                         OR
         NOTBCD ( vpenh->val.vpenh_hours.second )                            )
    {
      return ( FALSE );
    }
  }

  return ( TRUE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_setVpabsPsa      |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to copy the elements of the
            absolute validity period structure of the ACI to the
            corresponding structure of the PSA.

            Note: Copying of the structure elements might be
                  dangerous when array size of time elements differ
                  from MAX_VP_ABS_DIGIT due to changes in PSA
                  declaration.
*/
GLOBAL void cmhSMS_setVpabsPsa ( T_tp_vp_abs*     psaVp,
                                 T_ACI_VP_ABS* cmhVp )
{
  USHORT i; /* used for counting */

  BOOL  isNegative = ( cmhVp->timezone & 0x8000 );
  SHORT tz   = ( UBYTE ) cmhVp->timezone;

  for (i = 0; i < MAX_VP_ABS_DIGITS; i++)
  {
    psaVp -> year  [i] = cmhVp -> year  [i];
    psaVp -> month [i] = cmhVp -> month [i];
    psaVp -> day   [i] = cmhVp -> day   [i];
    psaVp -> hour  [i] = cmhVp -> hour  [i];
    psaVp -> minute[i] = cmhVp -> minute[i];
    psaVp -> second[i] = cmhVp -> second[i];
  }

  if ( isNegative )
  {
    tz = -tz;
    psaVp -> tz_sign = 1;
  }
  else
  {
    psaVp -> tz_sign = 0;
  }

  psaVp -> tz_lsb = tz & 0x000F;
  psaVp -> tz_msb = tz & 0x00F0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_setVpenhPsa      |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to copy the elements of the
            enhanced validity period structure of the ACI to the
            corresponding structure of the PSA.
*/
GLOBAL void cmhSMS_setVpenhPsa ( T_tp_vp_enh*  psaVp,
                                 T_ACI_VP_ENH* cmhVp )
{
  memset(psaVp, 0, sizeof(T_tp_vp_enh));

  if (cmhVp->func_ind & TP_VPF_ENH_EXT_BIT_MASK)
  {
    psaVp->tp_ext = SMS_EXT_INCLUDED;
    psaVp->v_tp_rsrvd = 1;
    psaVp->tp_rsrvd = cmhVp->ext_oct;
  }

  if (cmhVp->func_ind & TP_VPF_ENH_SINGLE_SHOT_MASK)
  {
    psaVp->tp_ss = SMS_SS_SET;
  }

  if ((cmhVp->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_NOT_PRESENT)
  {
    psaVp->tvpf = SMS_TVPF_NOT_PRESENT;
  }
  else if ((cmhVp->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_REL)
  {
    psaVp->tvpf = SMS_TVPF_RELATIVE;
    psaVp->v_tp_vp_rel = 1;
    psaVp->tp_vp_rel = cmhVp->val.vpenh_relative;
  }
  else if ((cmhVp->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_SEC)
  {
    psaVp->tvpf = SMS_TVPF_SECONDS;
    psaVp->v_tp_vp_sec = 1;
    psaVp->tp_vp_sec = cmhVp->val.vpenh_seconds;
  }
  else if ((cmhVp->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_HRS)
  {
    psaVp->tvpf = SMS_TVPF_HOURS;
    psaVp->v_hour = 1;
    psaVp->hour[0] = cmhVp->val.vpenh_hours.hour[0];
    psaVp->hour[1] = cmhVp->val.vpenh_hours.hour[1];
    psaVp->v_minute = 1;
    psaVp->minute[0] = cmhVp->val.vpenh_hours.minute[0];
    psaVp->minute[1] = cmhVp->val.vpenh_hours.minute[1];
    psaVp->v_second = 1;
    psaVp->second[0] = cmhVp->val.vpenh_hours.second[0];
    psaVp->second[1] = cmhVp->val.vpenh_hours.second[1];
  }
  else
  {
    TRACE_EVENT("[ERR] cmhSMS_setVpenhPsa: wrong type of validity period format");
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_setVpabsCmh      |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to copy the elements of the
            absolute validity period structure of the PSA to the
            corresponding structure of the ACI.

            Note: Copying of the structure elements might be
                  dangerous when array size of time elements differ
                  from MAX_VP_ABS_DIGIT due to changes in PSA
                  declaration.
*/
GLOBAL void cmhSMS_setVpabsCmh ( T_ACI_VP_ABS* cmhVp,
                                 T_tp_vp_abs*  psaVp )
{
  USHORT i; /* used for counting */
  SHORT tz;

  for (i = 0; i < MAX_VP_ABS_DIGITS; i++)
  {
    cmhVp -> year  [i] = psaVp -> year  [i];
    cmhVp -> month [i] = psaVp -> month [i];
    cmhVp -> day   [i] = psaVp -> day   [i];
    cmhVp -> hour  [i] = psaVp -> hour  [i];
    cmhVp -> minute[i] = psaVp -> minute[i];
    cmhVp -> second[i] = psaVp -> second[i];
  }

  tz = ((psaVp->tz_msb & 0x07) * 10) +  psaVp->tz_lsb;  /* BCD */

  if (psaVp->tz_sign)
    cmhVp -> timezone = -tz;
  else
    cmhVp -> timezone = tz;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_setVpenhCmh      |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to copy the elements of the
            enhanced validity period structure of the PSA to the
            corresponding structure of the ACI.

*/
GLOBAL void cmhSMS_setVpenhCmh ( T_ACI_VP_ENH* cmhVp,
                                 T_tp_vp_enh*  psaVp )
{
  memset(cmhVp, 0, sizeof(T_ACI_VP_ENH));

  cmhVp->func_ind = psaVp->tvpf;

  if ((psaVp->tp_ext EQ SMS_EXT_INCLUDED) AND (psaVp->v_tp_rsrvd))
  {
    cmhVp->func_ind |= TP_VPF_ENH_EXT_BIT_MASK;
    cmhVp->ext_oct = psaVp->tp_rsrvd;
  }

  if (psaVp->tp_ss EQ SMS_SS_SET)
  {
    cmhVp->func_ind |= TP_VPF_ENH_SINGLE_SHOT_MASK;
  }

  if (psaVp->tvpf EQ SMS_TVPF_NOT_PRESENT)
  {
    /* do nothing */
  }
  else if ((psaVp->tvpf EQ SMS_TVPF_RELATIVE) AND (psaVp->v_tp_vp_rel))
  {
    cmhVp->val.vpenh_relative = psaVp->tp_vp_rel;
  }
  else if ((psaVp->tvpf EQ SMS_TVPF_SECONDS) AND (psaVp->v_tp_vp_sec))
  {
    cmhVp->val.vpenh_seconds = psaVp->tp_vp_sec;
  }
  else if (psaVp->tvpf EQ SMS_TVPF_HOURS)
  {
    if (psaVp->v_hour)
    {
      cmhVp->val.vpenh_hours.hour[0] = psaVp->hour[0];
      cmhVp->val.vpenh_hours.hour[1] = psaVp->hour[1];
    }
    if (psaVp->v_minute)
    {
      cmhVp->val.vpenh_hours.minute[0] = psaVp->minute[0];
      cmhVp->val.vpenh_hours.minute[1] = psaVp->minute[1];
    }
    if (psaVp->v_second)
    {
      cmhVp->val.vpenh_hours.second[0] = psaVp->second[0];
      cmhVp->val.vpenh_hours.second[1] = psaVp->second[1];
    }
  }
  else
  {
    TRACE_EVENT("[ERR] cmhSMS_setVpenhCmh: wrong type of validity period format");
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                 |
| STATE   : code                  ROUTINE : cmhSMS_getAdrStr         |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to convert the service center
            address in BCD to the service center address as a string.
*/
GLOBAL UBYTE cmhSMS_getAdrStr ( CHAR*  pStr,
                                UBYTE  maxIdx,
                                UBYTE* pBcd,
                                UBYTE  numDigits )
{
  UBYTE bcdIdx;
  UBYTE strIdx = 0;

  memset(pStr, 0x00, maxIdx);

  for(bcdIdx = 0; bcdIdx < numDigits AND strIdx < maxIdx; bcdIdx++)
  {
    switch (pBcd[bcdIdx])
    {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        pStr[strIdx++] = pBcd[bcdIdx] + '0';
        break;

      case BCD_ASTSK:
        pStr[strIdx++] = '*';
        break;

      case BCD_PND:
        pStr[strIdx++] = '#';
        break;

      case BCD_A:
        pStr[strIdx++] = 'A';
        break;

      case BCD_B:
        pStr[strIdx++] = 'B';
        break;

      case BCD_C:
        pStr[strIdx++] = 'C';
        break;
    }
  }

  pStr[strIdx] = '\0';

  return ( strIdx );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                 |
| STATE   : code                  ROUTINE : cmhSMS_getAdrBcd         |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to convert the service center
            address as a string to the service center address in BCD.
*/
GLOBAL void cmhSMS_getAdrBcd ( UBYTE* pBcd,
                               UBYTE* pNumDigits,
                               UBYTE  maxDigits,
                               CHAR*  pStr )
{
  UBYTE bcdIdx = 0;
  UBYTE strIdx;

  for(strIdx = 0; bcdIdx < maxDigits AND pStr[strIdx] NEQ '\0'; strIdx++)
  {
    switch (pStr[strIdx])
    {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        pBcd[bcdIdx++] = pStr[strIdx] - '0';
        break;

      case '*':
        pBcd[bcdIdx++] = BCD_ASTSK;
        break;

      case '#':
        pBcd[bcdIdx++] = BCD_PND;
        break;

      case 'A':
        pBcd[bcdIdx++] = BCD_A;
        break;

      case 'B':
        pBcd[bcdIdx++] = BCD_B;
        break;

      case 'C':
        pBcd[bcdIdx++] = BCD_C;
        break;
    }
  }

  *pNumDigits = bcdIdx;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                 |
| STATE   : code                  ROUTINE : cmhSMS_getAdrAlphaNum    |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to pack the Alphanumeric 
  Destination Adress according to 23.038
*/
GLOBAL T_ACI_RETURN cmhSMS_packAlphaNumAddr( CHAR*          da, 
                                            T_tp_da*       da_addr)
{
  UBYTE dest[((MAX_SMS_ADDR_DIG/2) * 8) / 7];
  UBYTE dest_len;
  USHORT dalen;
  
  if( (da NEQ NULL) AND (da_addr NEQ NULL) )
  {
    dalen = strlen(da);
    
    dest_len = utl_cvt8To7( (UBYTE*)da, (UBYTE)dalen, dest, 0);
    
    if( dest_len > (MAX_SMS_ADDR_DIG/2) )
    {
      TRACE_EVENT("DA length is greater then the supported length");
      ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_UnknownErr );
      return(AT_FAIL);
    }
    if( ((dalen + 3) %7) <= 3 )
    {
      da_addr->c_num = (dest_len*2) - 1;  /* odd number of usefull nibbles */
    }
    else
    {
      da_addr->c_num = dest_len*2; /* even number of usefull nibbles */
    }
    
    cmh_unpackBCD(da_addr->num, dest, dest_len );
    da_addr->digits = da_addr->c_num;
  }
  
  return(AT_CMPL);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_getMemCmh        |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the preferred memory from
            PSA type to CMH type.
*/
GLOBAL void cmhSMS_getMemCmh ( UBYTE inMem, T_ACI_SMS_STOR* outMem )
{
  switch( inMem )
  {
    case( MEM_ME ): *outMem = SMS_STOR_Me; break;
    case( MEM_SM ): *outMem = SMS_STOR_Sm; break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_getMemPsa        |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the preferred memory from
            CMH type to PSA type.

            returns: TRUE if conversion was successfull,
                     otherwise FALSE
*/
GLOBAL BOOL cmhSMS_getMemPsa ( T_ACI_SMS_STOR inMem, UBYTE* outMem )
{
  switch( inMem )
  {
    case( SMS_STOR_Me ): *outMem = MEM_ME; break;
    case( SMS_STOR_Sm ): *outMem = MEM_SM; break;
    default            : return ( FALSE );
  }

  return ( TRUE );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_getAlphabetPp       |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to extract the used alphabet out
            of the data coding scheme for point-to-point SMS.
*/
GLOBAL UBYTE cmhSMS_getAlphabetPp ( UBYTE dcs )
{
  UBYTE alphabet = 0;  /* means 7 bit default alphabet */

  switch (dcs & 0xF0)
  {
    case( 0x30 ):
    case( 0x20 ):
      alphabet = 0x01; /* compressed, counts as 8 bit data */
      break;
    case( 0x10 ):
    case( 0x00 ):
      alphabet = (dcs & 0x0C) >> 2;
      if (alphabet EQ 3)
        alphabet = 0;  /* reserved coding */
      break;
    case( 0xE0 ):
      alphabet = 0x02; /* UCS2 */
      break;
    case( 0xF0 ):
      alphabet = (dcs & 0x04) >> 2;
      break;
  }

  return alphabet;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_expdSmsPp        |
+-------------------------------------------------------------------+

  PURPOSE : This function expands a point-to-point SMS from
            7 to 8 bit.
*/
GLOBAL void cmhSMS_expdSmsPp ( UBYTE  byte_offset,
                               UBYTE  dcs,
                               UBYTE* source,
                               UBYTE  source_len,
                               UBYTE* dest,
                               UBYTE* dest_len )
{
  UBYTE  alphabet;
  UBYTE  bit_offset = 0;

  TRACE_FUNCTION ("cmhSMS_expdSmsPp ()");


  alphabet = cmhSMS_getAlphabetPp ( dcs );

  switch (alphabet)
  {
    case( 0 ): /* 7 bit alphabet */

      if ( byte_offset % 7 NEQ 0 )
      {
         bit_offset = 7 - ((byte_offset*8) % 7);
      }

      *dest_len = source_len - ((byte_offset*8+6)/7); /* adjust byte_offset to septets */

      /* In 7-Bit mode we get number of septets but we need octets */
      source_len = (source_len*7+7)/8; /* round up to next octet*/
      source_len -= byte_offset;

      utl_cvt7To8 ( source, source_len, dest, bit_offset);
      

      break;

    default:   /* 8 bit alphabet, UCS2, reserved */

      *dest_len = source_len-byte_offset;
      memcpy ( (CHAR*) dest, (CHAR*)source, *dest_len );
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
| STATE   : code                  ROUTINE : cmhSMS_rdcSmsPp         |
+-------------------------------------------------------------------+

  PURPOSE : This function reduces a point-to-point SMS from
            8 to 7 bit.
*/
GLOBAL void cmhSMS_rdcSmsPp ( UBYTE  byte_offset,
                              UBYTE  dcs,
                              UBYTE* source,
                              UBYTE  source_len,
                              UBYTE* dest,
                              UBYTE* dest_len )
{
  UBYTE  data_len;
  UBYTE  alphabet;
  UBYTE  bit_offset = 0;

  TRACE_FUNCTION ("cmhSMS_rdcSmsPp ()");

  if (source_len EQ 0)
  {
    *dest_len = source_len;
    return;
  }

  alphabet = cmhSMS_getAlphabetPp ( dcs );

  switch (alphabet)
  {
    case( 0 ): /* 7 bit alphabet */

      if ( byte_offset % 7 NEQ 0 )
      {
         bit_offset = 7 - ((byte_offset*8) % 7);
      }

      data_len = MINIMUM (source_len, (SMS_MSG_LEN * 8) / 7);

      data_len = utl_cvt8To7 ( source, data_len, dest, bit_offset );
      break;

    default:   /* 8 bit alphabet, UCS2, reserved */
      data_len = MINIMUM ( source_len, SMS_MSG_LEN );

      memcpy ( ( CHAR * ) dest, ( CHAR * ) source, data_len );
      break;
  }

  *dest_len = data_len;
}
/* Implements Measure 25. This function is replaced by cmh_getAlphabetCb*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_expdSmsCb           |
+--------------------------------------------------------------------+

  PURPOSE : This function expands a cell broadcast SMS from
            7 to 8 bit.
*/
GLOBAL void cmhSMS_expdSmsCb ( UBYTE      dcs,
                               UBYTE     *source,
                               UBYTE      source_len,
                               UBYTE     *dest,
                               UBYTE     *dest_len )
{
  UBYTE  alphabet;

  TRACE_FUNCTION ("cmhSMS_expdSmsCb()");
  alphabet = cmh_getAlphabetCb ( dcs );

  switch (alphabet)
  {
    case( 0 ):  /* 7 bit alphabet */
/* PATCH Add bit_offset parameter to function cvt7To8 */
/*      utl_cvt7To8 ( source, source_len, dest ); */
      utl_cvt7To8 ( source, source_len, dest, 0);
/* PATCH END */

      *dest_len = ( source_len * 8 ) / 7;
      break;

    default: /* 8 bit alphabet, UCS2, reserved */
      memcpy ( ( CHAR * ) dest, ( CHAR * ) source, source_len );

      *dest_len = source_len;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_setToaDef           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the type of address to default values
            (when the first character of address is '+' or the first
            two characters are '0' default is 145 otherwise default
            is 129)
*/
GLOBAL CHAR* cmhSMS_setToaDef ( CHAR*  number,
                                UBYTE* ntype,
                                UBYTE* nplan )
{
  *nplan = SMS_NPI_ISDN;

  if ( *number EQ '+' )
  {
    *ntype = SMS_TON_INTERNATIONAL;
    return number+1;
  }
  else
  {
    *ntype = SMS_TON_UNKNOWN;
    return number;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_setTimezone         |
+--------------------------------------------------------------------+

  PURPOSE : Codes the timezone format used in entity SMS.

*/
GLOBAL UBYTE cmhSMS_setTimezone (SHORT timezone)
{
  UBYTE local_tz   = ( UBYTE ) timezone;
  BOOL  isNegative = ( local_tz & 0x80 );

  if ( isNegative )
    local_tz = ~local_tz + 1;

  local_tz = ( local_tz / 10 ) + ( ( local_tz % 10 ) << 4 );

  return ( ( isNegative ) ? local_tz | 0x08 : local_tz );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_getTimezone         |
+--------------------------------------------------------------------+

  PURPOSE : Decodes the timezone format used in entity SMS.

*/
GLOBAL SHORT cmhSMS_getTimezone (UBYTE timezone)
{
  signed char local_tz;

  local_tz = ( timezone & 0x07 ) * 10 + ( ( timezone & 0xF0 ) >> 4 );

  return ( SHORT ) (( timezone & 0x08 ) ? -local_tz : local_tz );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_setStorOcc          |
+--------------------------------------------------------------------+

  PURPOSE : Fills the T_ACI_SMS_STOR_OCC structure with data from the
            shared parameter buffer.

*/
GLOBAL void cmhSMS_setStorOcc  ( T_ACI_SMS_STOR_OCC* outMem,
                                 UBYTE               inMem   )
{
  cmhSMS_getMemCmh ( inMem, &outMem -> mem );

  if ( outMem -> mem EQ SMS_STOR_Sm )
  {
    outMem -> used  = smsShrdPrm.aci_sms_parameter.simUsed;
    outMem -> total = smsShrdPrm.aci_sms_parameter.simTotal;
  }
  else
  {
    outMem -> used  = smsShrdPrm.aci_sms_parameter.meUsed;
    outMem -> total = smsShrdPrm.aci_sms_parameter.meTotal;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
|                                 ROUTINE : cmhSMS_GetCmsFromSim    |
+-------------------------------------------------------------------+

  PURPOSE : Mapping of SIM error code to ACI error code.

*/
GLOBAL T_ACI_CMS_ERR cmhSMS_GetCmsFromSim ( USHORT errCode )
{
  switch ( errCode )
  {
    case SIM_NO_ERROR:
      return CMS_ERR_NotPresent;

    case SIM_CAUSE_PIN1_EXPECT:
      return CMS_ERR_SimPinReq;

    case SIM_CAUSE_PIN2_EXPECT:
      return CMS_ERR_SimPin2Req;

    case SIM_CAUSE_PUK1_EXPECT:
    case SIM_CAUSE_PIN1_BLOCKED:
      return CMS_ERR_SimPukReq;

    case SIM_CAUSE_PUK2_EXPECT:
    case SIM_CAUSE_PIN2_BLOCKED:
      return CMS_ERR_SimPuk2Req;

    case SIM_CAUSE_PUK1_BLOCKED:
    case SIM_CAUSE_PUK2_BLOCKED:
      return CMS_ERR_SimWrong;

    case SIM_CAUSE_UNKN_FILE_ID:
    case SIM_CAUSE_DNL_ERROR:
      return CMS_ERR_UnknownErr;

    case SIM_CAUSE_EF_INVALID:
      return CMS_ERR_OpNotSup;

    case SIM_CAUSE_ADDR_WRONG:
      return CMS_ERR_InValMemIdx;

    case SIM_CAUSE_CMD_INCONSIST:
    case SIM_CAUSE_MAX_INCREASE:
    case SIM_CAUSE_CHV_NOTSET:
    case SIM_CAUSE_CHV_VALIDATED:
    case SIM_CAUSE_ACCESS_PROHIBIT:
      return CMS_ERR_OpNotAllowed;

    case SIM_CAUSE_CARD_REMOVED:
    case SIM_CAUSE_DRV_NOCARD:
      return CMS_ERR_SimNotIns;

    case SIM_CAUSE_NO_SELECT:
    case SIM_CAUSE_CLA_WRONG:
    case SIM_CAUSE_INS_WRONG:
    case SIM_CAUSE_P1P2_WRONG:
    case SIM_CAUSE_P3_WRONG:
    case SIM_CAUSE_PARAM_WRONG:
      return CMS_ERR_MeFail;

    case SIM_CAUSE_SAT_BUSY:
      return CMS_ERR_SimBsy;

    case SIM_CAUSE_DRV_TEMPFAIL:
      return CMS_ERR_SimFail;

    default:
      if (GET_CAUSE_DEFBY(errCode) EQ DEFBY_CONDAT AND
          GET_CAUSE_ORIGSIDE(errCode) EQ ORIGSIDE_MS)
      {
        return CMS_ERR_UnknownErr;
      }
      return CMS_ERR_UnknownErr;
  }
}



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                |
|                                 ROUTINE : cmhSMS_GetCmsFromSms    |
+-------------------------------------------------------------------+

  PURPOSE : Mapping of SMS causes (MNSMS.doc) to ACI error code.

*/
GLOBAL T_ACI_CMS_ERR cmhSMS_GetCmsFromSms ( USHORT errCode )
{
  switch ( errCode )
  {
    case SMS_NO_ERROR:              /* No error                           */
      return CMS_ERR_NotPresent;

    case SMS_CAUSE_PARAM_WRONG:     /* Wrong parameter in primitive       */

    case SMS_CAUSE_ENTITY_BUSY:     /* Entity is busy                     */
      return CMS_ERR_SimBsy;

    case SMS_CAUSE_OPER_NOT_ALLW:   /* Operation not allowed              */
      return CMS_ERR_OpNotAllowed;

    case SMS_CAUSE_OPER_NOT_SUPP:   /* Operation not supported            */
      return CMS_ERR_OpNotSup;

    case SMS_CAUSE_SIM_BUSY:        /* SIM busy                           */
      return CMS_ERR_SimBsy;
    default:
      break;
  }
      
      /*
       *    The switch must be splitted because of a compiler bug
       *    - asm files can not compiled
       *
       *    TMS470 ANSI C Compiler       Version 1.22e
       *
       *    brz, 2004.02.14
       */
      
  switch(errCode) {
    case SMS_CAUSE_MEM_FAIL:        /* Memory failure                     */
      return CMS_ERR_MemFail;

    case SMS_CAUSE_INV_INDEX:       /* Invalid memory index               */
      return CMS_ERR_InValMemIdx;

    case SMS_CAUSE_MEM_FULL:        /* Memory full                        */
      return CMS_ERR_MemFull;

    case SMS_CAUSE_NO_SERVICE:      /* No network service                 */
      return CMS_ERR_NoNetServ;

    case SMS_CAUSE_NET_TIMEOUT:     /* Network timeout                    */
      return CMS_ERR_NetTimeOut;

    case SMS_CAUSE_UNEXP_CNMA:      /* No +CNMA acknowledgement expected  */
      return CMS_ERR_NoCnmaAckExpect;

    case SMS_CAUSE_OTHER_ERROR:     /* Any other error                    */
      return CMS_ERR_UnknownErr;

    default:
      return ((T_ACI_CMS_ERR) errCode); /* cmdCmsError(errCode) will figure out other error values from the first byte of the error code */
  }
}






/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_ready                 |
+-------------------------------------------------------------------+

  PURPOSE : This function notifies SMS_READY to all sources.
*/
GLOBAL void cmhSMS_ready ( void )
{
  int idx;
  T_opl_field * ptr_opl;

  TRACE_FUNCTION ("cmhSMS_ready()");

  {
    PALLOC (sim_sync_req, SIM_SYNC_REQ);
    sim_sync_req -> synccs = SYNC_MMI_FINISHED_READING;
    simShrdPrm.synCs = SYNC_MMI_FINISHED_READING;
    PSENDX (SIM, sim_sync_req);
  }

  smsShrdPrm.accessEnabled = TRUE;
  ptr_opl = cmhSIM_GetOPL();
  ptr_opl->opl_status = FALSE;

  cmhSIM_UpdateOperatorName(NOT_PRESENT_16BIT);  /* start EF_PNN and EF_OPL reading */

  pb_start_build(FALSE);     /* start phonebook reading, no guarantees */

  percentCSTAT_indication(STATE_MSG_SMS, ENTITY_STATUS_Ready);

  for( idx = 0; idx < CMD_SRC_MAX; idx++ )
  {
    R_AT( RAT_SMS_READY, (T_ACI_CMD_SRC)idx )();
  }

}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_disableAccess         |
+-------------------------------------------------------------------+

  PURPOSE : This function disables access to all SMS functions.
*/
GLOBAL void cmhSMS_disableAccess (void)
{
  smsShrdPrm.accessEnabled = FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_checkSIM              |
+-------------------------------------------------------------------+

  PURPOSE : This function checks if SMS is initialized. If not, the
            SIM state is checked. AN error code dependent of the
            SIM state is stored.
*/
GLOBAL BOOL cmhSMS_checkSIM (void)
{
  if (smsShrdPrm.accessEnabled)
    return TRUE;                /* SMS is accessible*/
  /*
   *-----------------------------------------------------------------
   * check SIM status
   *-----------------------------------------------------------------
   */

  switch (simShrdPrm.SIMStat)
  {
  case NO_VLD_SS:
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_SimNotIns );
    break;
  case SS_INV:
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_SimFail );
    break;
/*  case SS_BLKD: 
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_SimPukReq );
    break; */
  case SS_URCHB:
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_SimWrong );
    break;
  case SS_OK:
    switch (simShrdPrm.PINStat)
    {
      case PS_PIN1:
        ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_SimPinReq);
        break;

      case PS_PIN2:
        ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_SimPin2Req);
        break;

      case PS_PUK1:
        ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_SimPukReq);
        break;

      case PS_PUK2:
        ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_SimPuk2Req);
        break;

      default:
        ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_SimBsy );
        break;
    }
    break;
  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_SimBsy );
    break;
  }
  return FALSE;
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_checkAccess           |
+-------------------------------------------------------------------+

  PURPOSE : This function checks if SMS is accessible. If an error
            condition is found, then *ret is set to either AT_FAIL or
            AT_BUSY, otherwise it remains unaffected. The error code
            is stored.
*/
GLOBAL BOOL cmhSMS_checkAccess (T_ACI_CMD_SRC srcId,
                                T_ACI_RETURN *ret)
{
  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */
  if(srcId NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT)
  {
    if(!cmh_IsVldCmdSrc (srcId))
    {
    TRACE_ERROR ("[cmhSMS_checkAccess]: Cmd Src not valid");
    *ret = AT_FAIL;
    return FALSE;
  }
  }
  /*
   *-----------------------------------------------------------------
   * check entity status
   *-----------------------------------------------------------------
   */
  if( smsShrdPrm.smsEntStat.curCmd NEQ AT_CMD_NONE )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
    TRACE_ERROR ("[cmhSMS_checkAccess]: Entity is busy");
    *ret = AT_BUSY;
    return FALSE;
  }

  /*
   *-----------------------------------------------------------------
   * check SMS access status
   *-----------------------------------------------------------------
   */
  if (cmhSMS_checkSIM ())
    return TRUE;

  TRACE_ERROR ("[cmhSMS_checkAccess]: Wrong SIM status");

  *ret = AT_FAIL;
  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_getPrfRge             |
+-------------------------------------------------------------------+

  PURPOSE : This function gets the number of available profiles for
            SMS related parameters. First, the SIM is checked. If
            there is no space, then PCM ist checked.
*/
GLOBAL SHORT cmhSMS_getPrfRge ( void )
{
  /* TEMPORARY, ONLY FOR TEST PURPOSES: Pretend that the SIM has no SMSP !!! */
  /* smsShrdPrm.aci_sms_parameter.smsParamMaxRec = 0;*/

  if (smsShrdPrm.aci_sms_parameter.smsParamMaxRec > 0)
  {
    return (SHORT)smsShrdPrm.aci_sms_parameter.smsParamMaxRec;
  }
  else
  {
    return (MAX_FFS_SMSPRFLS);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_ReadPrmFFS            |
+-------------------------------------------------------------------+

  PURPOSE : This function reads one record of EF_SMSPRFL_ID from FFS.
            The processed parameters are controlled by bits in
            'access'.
*/
static T_ACI_RETURN cmhSMS_ReadPrmFFS (T_ACI_CMD_SRC  srcId,
                                       SHORT          recNr,
                                       int            access)
{
  T_SMS_SET_PRM     *pSMSSetPrm;  /* points to SMS parameter set */
  int               i;
  T_SMS_SET_PRM     *elem;
  T_ACI_FFS_SMSPRFL smsPrfl;
#ifndef _SIMULATION_
  T_FFS_SIZE        ffs_size;
#endif

  TRACE_FUNCTION ("cmhSMS_ReadPrmFFS()");

#ifdef _SIMULATION_
  smsPrfl.vldFlag = FFS_SMSPRFL_INVLD;          /* trigger a failure since nothing can be read in simulation */
#else
  ffs_size = FFS_fread(ffs_smsprfl_fname[recNr-1], (void*)&smsPrfl, SIZE_FSS_SMSPRFL);
  /*
    Don't check FFS return for only specif ERRORS, as ACI will not know what 
    ERROR FFS may send. So, it's walyws better to check for ERROR OK
    i.e EFFS_OK  Considering this in mind the below condition is changes to 
    do the same.
  */
  if ( ffs_size < EFFS_OK )
  {
    TRACE_EVENT_P1("error when reading FFS object \"%s\"", ffs_smsprfl_fname[recNr-1]);
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_MemFail );
    return AT_FAIL;
  }
#endif

  if ( smsPrfl.vldFlag EQ FFS_SMSPRFL_INVLD )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_OpNotAllowed );
    return AT_FAIL;
  }

  if (access & ACI_PCM_ACCESS_SMSP)
  {
    if (srcId >= CMD_SRC_MAX)
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_UnknownErr );
      return AT_FAIL;
    }
    else
    {
      if (set_prm_list EQ NULL)
      {
        set_prm_list = new_list();
      }

      elem = find_element(set_prm_list, (UBYTE)recNr, cmhSMS_findPrflId);
      if (elem EQ NULL)
      {
        ACI_MALLOC(elem, sizeof(T_SMS_SET_PRM));
        memset(elem, 0, sizeof(T_SMS_SET_PRM));
        insert_list(set_prm_list, elem);
      }
      if (srcId <= CMD_SRC_NONE)
      {
        elem->numOfRefs = OWN_SRC_MAX;
        for (i=0; i < OWN_SRC_MAX; i++)
        {
          smsShrdPrm.pSetPrm[i] = (T_SMS_SET_PRM*) elem;
        }
      }
      else
      {
        smsShrdPrm.pSetPrm[srcId] = (T_SMS_SET_PRM*) elem;
        (smsShrdPrm.pSetPrm[srcId])->numOfRefs++;
      }
    }

    pSMSSetPrm = (T_SMS_SET_PRM*) elem;
    pSMSSetPrm->prflId = (UBYTE)recNr;

#ifndef _SIMULATION_
    /*
     *-------------------------------------------------------------
     * restore the service center address
     *-------------------------------------------------------------
     */
    pSMSSetPrm->sca.c_num = MINIMUM (smsPrfl.CSCAlenSca, MAX_SMS_ADDR_DIG);
    memcpy ( pSMSSetPrm->sca.num, smsPrfl.CSCAsca, pSMSSetPrm->sca.c_num);
    pSMSSetPrm->sca.ton = smsPrfl.CSCAton;
    pSMSSetPrm->sca.npi = smsPrfl.CSCAnpi;
    pSMSSetPrm->sca.v_ton = TRUE;
    pSMSSetPrm->sca.v_npi = TRUE;

    /*
     *-------------------------------------------------------------
     * restore the text mode parameters
     *-------------------------------------------------------------
     */
    pSMSSetPrm->vpRel = smsPrfl.CSMPvprel;
    memcpy ( &pSMSSetPrm->vpAbs, smsPrfl.CSMPvpabs, SIZE_FFS_SMSPRFL_VPABS );
    memcpy ( &pSMSSetPrm->vpEnh, smsPrfl.CSMPvpenh, SIZE_FFS_SMSPRFL_VPENH );

    pSMSSetPrm->msgType = smsPrfl.CSMPfo;
    pSMSSetPrm->pid = smsPrfl.CSMPpid;
    pSMSSetPrm->dcs = smsPrfl.CSMPdcs;
#endif
  }

#ifndef _SIMULATION_
  if (access & ACI_PCM_ACCESS_CBMP)
  {
      /*
       *-------------------------------------------------------------
       * restore the cell broadcast message types and data coding
       * schemes
       *-------------------------------------------------------------
       */
      smsShrdPrm.cbmPrm.cbmMode = smsPrfl.CSCBmode;

      {                         /* default setting */
        memset (smsShrdPrm.cbmPrm.msgId, DEF_MID_RANGE, sizeof (smsShrdPrm.cbmPrm.msgId));
        memset (smsShrdPrm.cbmPrm.dcsId, DEF_DCS_RANGE, sizeof(smsShrdPrm.cbmPrm.dcsId));
      }

      for ( i = 0; i < MINIMUM(MAX_IDENTS,SIZE_FFS_SMSPRFL_MIDS/2); i++ )
      {
        smsShrdPrm.cbmPrm.msgId[i]  = ( USHORT )smsPrfl.CSCBmids[2*i] << 8;
        smsShrdPrm.cbmPrm.msgId[i] |= ( USHORT )smsPrfl.CSCBmids[2*i+1];
      }

      memcpy ( smsShrdPrm.cbmPrm.dcsId, smsPrfl.CSCBdcss,
               MINIMUM(MAX_IDENTS,SIZE_FFS_SMSPRFL_DCSS) );

      memcpy (smsShrdPrm.cbmPrm.IMSI, smsPrfl.IMSI, MAX_IMSI);
  }
#endif
  return AT_CMPL;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_WritePrmFFS           |
+-------------------------------------------------------------------+

  PURPOSE : This function writes one record of EF_SMSPRFL_ID to FFS.
            The processed parameters are controlled by bits in
            'access'.
*/
static T_ACI_RETURN cmhSMS_WritePrmFFS (T_ACI_CMD_SRC  srcId,
                                        SHORT          recNr,
                                        int            access)
{
  T_SMS_SET_PRM     *pSMSSetPrm;  /* points to SMS parameter set */
  T_ACI_FFS_SMSPRFL smsPrfl;
  int               i;
#ifndef _SIMULATION_
  T_FFS_RET         ffs_ret;
#endif

  TRACE_FUNCTION ("cmhSMS_WritePrmFFS()");

  if (access & ACI_PCM_ACCESS_SMSP)
  {
    if (srcId > CMD_SRC_NONE AND srcId < CMD_SRC_MAX)
    {
      pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

      /*
       *-------------------------------------------------------------
       * save the service center address
       *-------------------------------------------------------------
       */
      smsPrfl.CSCAlenSca = MINIMUM ( pSMSSetPrm->sca.c_num, SIZE_FFS_SMSPRFL_SCA );
      memcpy ( smsPrfl.CSCAsca, pSMSSetPrm->sca.num, smsPrfl.CSCAlenSca );

      i = (int)smsPrfl.CSCAlenSca;
      while ( i < SIZE_FFS_SMSPRFL_SCA )
        smsPrfl.CSCAsca[i++] = 0xFF;

      smsPrfl.CSCAton = pSMSSetPrm->sca.ton;
      smsPrfl.CSCAnpi = pSMSSetPrm->sca.npi;

      /*
       *-------------------------------------------------------------
       * store the text mode parameters
       *-------------------------------------------------------------
       */
      smsPrfl.CSMPfo    = pSMSSetPrm->msgType;

      smsPrfl.CSMPvprel = pSMSSetPrm->vpRel;
      memcpy ( smsPrfl.CSMPvpabs, &pSMSSetPrm->vpAbs, SIZE_FFS_SMSPRFL_VPABS );
      memcpy ( smsPrfl.CSMPvpenh, &pSMSSetPrm->vpEnh, SIZE_FFS_SMSPRFL_VPENH );

      smsPrfl.CSMPpid   = pSMSSetPrm->pid;
      smsPrfl.CSMPdcs   = pSMSSetPrm->dcs;

      /*
       *-------------------------------------------------------------
       * set the valid flag
       *-------------------------------------------------------------
       */
      smsPrfl.vldFlag = FFS_SMSPRFL_VLD;
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_UnknownErr );
      return AT_FAIL;
    }
  }
  if (access & ACI_PCM_ACCESS_CBMP)
  {
      /*
       *-------------------------------------------------------------
       * save the cell broadcast message types
       *-------------------------------------------------------------
       */
      smsPrfl.CSCBmode = smsShrdPrm.cbmPrm.cbmMode;

      {                         /* default setting */
        memset (smsPrfl.CSCBmids, 0xFF, sizeof(smsPrfl.CSCBmids));
        memset (smsPrfl.CSCBdcss, 0xFF, sizeof(smsPrfl.CSCBdcss));
      }

      for ( i = 0; i < MINIMUM(MAX_IDENTS*2,SIZE_FFS_SMSPRFL_MIDS)-1; i += 2 )
      {
        smsPrfl.CSCBmids[i]   = (UBYTE)(smsShrdPrm.cbmPrm.msgId[i/2] >> 8);
        smsPrfl.CSCBmids[i+1] = (UBYTE)smsShrdPrm.cbmPrm.msgId[i/2];
      }

      memcpy ( smsPrfl.CSCBdcss, smsShrdPrm.cbmPrm.dcsId,
               MINIMUM(MAX_IDENTS,SIZE_FFS_SMSPRFL_DCSS) );

      /*
       * Save IMSI also in FFS 
       */
      memcpy (smsPrfl.IMSI, simShrdPrm.imsi.field, simShrdPrm.imsi.c_field);
      smsPrfl.vldFlag = FFS_SMSPRFL_VLD;
  }

#ifndef _SIMULATION_
  ffs_ret = FFS_mkdir(FFS_SMSPRFL_PATH);
  if (ffs_ret EQ EFFS_OK)
  {
    TRACE_EVENT_P1("FFS directory \"%s\" successfully created", FFS_SMSPRFL_PATH);
  }
  else if (ffs_ret EQ EFFS_EXISTS)
  {
    TRACE_EVENT_P1("FFS directory \"%s\" already exists", FFS_SMSPRFL_PATH);
  }
  else
  {
    TRACE_EVENT_P1("error when creating FFS directory \"%s\"", FFS_SMSPRFL_PATH);
  }

  ffs_ret = FFS_fwrite(ffs_smsprfl_fname[recNr-1], (void*)&smsPrfl, SIZE_FSS_SMSPRFL);
  if (ffs_ret NEQ EFFS_OK)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_MemFail );
    TRACE_EVENT_P1("error when writing FFS object \"%s\"", ffs_smsprfl_fname[recNr-1]);
    return AT_FAIL;
  }
#endif

  return AT_CMPL;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_ReadParams            |
+-------------------------------------------------------------------+

  PURPOSE : This function reads parameters from SIM and/or PCM.
            In case of AT command +CFUN the first record is read to
            be informed about storage capability of SIM.
*/
GLOBAL T_ACI_RETURN cmhSMS_ReadParams (T_ACI_CMD_SRC  srcId,
                                       T_ACI_AT_CMD   cmd,
                                       SHORT          recNr)
{
  T_ACI_RETURN ret = AT_CMPL;
  int access = 0;
  
  TRACE_FUNCTION ("cmhSMS_ReadParams()");

  smsShrdPrm.smsEntStat.curCmd = cmd;
  smsShrdPrm.owner = (T_OWN)srcId; 
  smsShrdPrm.smsEntStat.entOwn = srcId;


  if (cmd EQ AT_CMD_CFUN)
  {  
    /* initialization */
  
   /*
    * Always read CSCB params from FFS so that previously stored mode and dcss
    *  can also be restored from FFS
    */
    access |= ACI_PCM_ACCESS_CBMP;
    /*if (!psaSIM_ChkSIMSrvSup( SRV_SMS_Parms ))*/
    access |= ACI_PCM_ACCESS_SMSP;

      ret = cmhSMS_ReadPrmFFS (CMD_SRC_NONE, 1, access);

    if (psaSIM_ChkSIMSrvSup( SRV_SMS_Parms ))
    {
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_SMSP;
      ret = cmhSIM_ReadRecordEF (CMD_SRC_NONE, cmd, FALSE, NULL,
                                  SIM_SMSP, 1, 255,
                                  NULL, cmhSMS_InitSMSP);
    }
    else if (psaSIM_ChkSIMSrvSup( SRV_CBMIdRnge ))
    {
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CBMIR;
      ret = cmhSIM_ReadTranspEF (CMD_SRC_NONE, cmd, FALSE, NULL,
                                  SIM_CBMIR, 0, 255,
                                  NULL, cmhSMS_InitSMSP);
    }
    else if (psaSIM_ChkSIMSrvSup( SRV_CBM_Ident ))
    {
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CBMI;
      ret = cmhSIM_ReadTranspEF (CMD_SRC_NONE, cmd, FALSE, NULL,
                                  SIM_CBMI, 0, 255,
                                  NULL, cmhSMS_InitSMSP);
    }
#ifdef SIM_TOOLKIT
    else if ((psaSIM_ChkSIMSrvSup( SRV_DtaDownlCB )) AND 
             smsShrdPrm.owner EQ OWN_SRC_SAT )
    {
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CBMID;
      ret = cmhSIM_ReadTranspEF (CMD_SRC_NONE, cmd, FALSE, NULL,
                                  SIM_CBMID, 0, 255,
                                  NULL, cmhSMS_InitSMSP);
    }
#endif /* of SIM_TOOLKIT */
  }
  else
  {

    /* TEMPORARY, ONLY FOR TEST PURPOSES: Pretend that the SIM has no SMSP !!! */
    /* smsShrdPrm.aci_sms_parameter.smsParamMaxRec = 0; */
    
    /*
     * Always read CSCB params from FFS so that previously stored mode and dcss
     *  can also be restored from FFS
     */
    /* normal operation */
    access |= ACI_PCM_ACCESS_CBMP;
   /* if (smsShrdPrm.aci_sms_parameter.smsParamMaxRec EQ 0)*/
      access |= ACI_PCM_ACCESS_SMSP;
      ret = cmhSMS_ReadPrmFFS (srcId, recNr, access);

    if (smsShrdPrm.aci_sms_parameter.smsParamMaxRec > 0)
    {
      if (recNr <= 0
          OR recNr > (SHORT)smsShrdPrm.aci_sms_parameter.smsParamMaxRec)
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_InValMemIdx );
        ret= AT_FAIL;
      }
      else
      {
        smsShrdPrm.prmRdSeq = SMS_READ_SIM_SMSP;
        return cmhSIM_ReadRecordEF (CMD_SRC_NONE, cmd, FALSE, NULL, SIM_SMSP,
                                    (UBYTE)(recNr),
                                    smsShrdPrm.aci_sms_parameter.smsParamRecLen,
                                    NULL, cmhSMS_RdCnfCRES);
      }
    }
    if (smsShrdPrm.cbmPrm.cbmSIMmaxIdRge > 0)
    {
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CBMIR;
      return cmhSIM_ReadTranspEF (CMD_SRC_NONE, cmd, FALSE, NULL, SIM_CBMIR,
                                  0, smsShrdPrm.cbmPrm.cbmSIMmaxIdRge,
                                  NULL, cmhSMS_RdCnfCRES);
    }
    if (smsShrdPrm.cbmPrm.cbmSIMmaxId > 0)
    { 
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CBMI;
      return cmhSIM_ReadTranspEF (CMD_SRC_NONE, cmd, FALSE, NULL, SIM_CBMI,
                                  0, smsShrdPrm.cbmPrm.cbmSIMmaxId,
                                  NULL, cmhSMS_RdCnfCRES);
    }
  }
  if (ret NEQ AT_EXCT)
  {
    smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
    smsShrdPrm.owner             = (T_OWN)CMD_SRC_NONE;
    smsShrdPrm.smsEntStat.entOwn =CMD_SRC_NONE;

    smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)srcId;
    psaMMI_Cbch();
    smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)CMD_SRC_NONE;
  }
  return ret;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_WriteParams           |
+-------------------------------------------------------------------+

  PURPOSE : This function reads parameters from SIM and/or PCM.
            In case of AT command +CFUN the first record is read to
            be informed about storage capability of SIM.
*/
GLOBAL T_ACI_RETURN cmhSMS_WriteParams (T_ACI_CMD_SRC  srcId,
                                        T_ACI_AT_CMD   cmd,
                                        SHORT          recNr)
{
  T_ACI_RETURN ret = AT_CMPL;
  int access = 0;
  UBYTE data[MAX_SIM_CMD];

  TRACE_FUNCTION ("cmhSMS_WriteParams()");

  smsShrdPrm.smsEntStat.curCmd = cmd;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn = srcId;

  /* TEMPORARY, ONLY FOR TEST PURPOSES: Pretend that the SIM has no SMSP !!! */
  /* smsShrdPrm.aci_sms_parameter.smsParamMaxRec = 0; */

  /*
   * Always write CSCB params to FFS so that mode and dcss
   *  can later be restored from FFS
   */
  access |= ACI_PCM_ACCESS_CBMP;
  /*if (smsShrdPrm.aci_sms_parameter.smsParamMaxRec EQ 0)*/
    access |= ACI_PCM_ACCESS_SMSP;

    ret = cmhSMS_WritePrmFFS (srcId, recNr, access);

  if (smsShrdPrm.aci_sms_parameter.smsParamMaxRec > 0)
  {
    if (recNr <= 0 OR recNr > (SHORT)smsShrdPrm.aci_sms_parameter.smsParamMaxRec)
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_InValMemIdx );
      ret= AT_FAIL;
    }
    else
    {
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_SMSP;
      cmhSMS_PutPrmSIM (srcId, data,
                        (int)smsShrdPrm.aci_sms_parameter.smsParamRecLen);
      return cmhSIM_WriteRecordEF (CMD_SRC_NONE, cmd, FALSE, NULL, SIM_SMSP,
                                   (UBYTE)(recNr),
                                   smsShrdPrm.aci_sms_parameter.smsParamRecLen,
                                   data, cmhSMS_WrCnfCSAS);
    }
  }
  if (smsShrdPrm.cbmPrm.cbmSIMmaxIdRge > 0)
  {
    smsShrdPrm.prmRdSeq = SMS_READ_SIM_CBMIR;
    cmhSMS_PutCbmirSIM (srcId, data,
                        (int)smsShrdPrm.cbmPrm.cbmSIMmaxIdRge * 4);

    return cmhSIM_WriteTranspEF (CMD_SRC_NONE, cmd, FALSE, NULL, SIM_CBMIR,
                                 0, (UBYTE)(smsShrdPrm.cbmPrm.cbmSIMmaxIdRge * 4),
                                 data, cmhSMS_WrCnfCSAS);
  }
  if (smsShrdPrm.cbmPrm.cbmSIMmaxId > 0)
  {
    smsShrdPrm.prmRdSeq = SMS_READ_SIM_CBMI;
    cmhSMS_PutCbmiSIM (srcId, data,
                       (int)smsShrdPrm.cbmPrm.cbmSIMmaxId * 2);
    
    return cmhSIM_WriteTranspEF (CMD_SRC_NONE, cmd, FALSE, NULL, SIM_CBMI,
                                 0, (UBYTE)(smsShrdPrm.cbmPrm.cbmSIMmaxId * 2),
                                 data, cmhSMS_WrCnfCSAS);
  }
  if (ret NEQ AT_EXCT)
  {
    smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
    smsShrdPrm.owner             = (T_OWN)CMD_SRC_NONE;
    smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
  }
  return ret;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_GetPrmSIM             |
+-------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of one record of
            EF(SMSP) read from SIM.
*/
GLOBAL BOOL cmhSMS_GetPrmSIM (T_ACI_CMD_SRC srcId,
                              UBYTE         *data,
                              int           dataLen)
{
  T_SMS_SET_PRM *pSMSSetPrm;    /* points to SMS parameter set */
  T_ACI_SMS_SIM_PARAMS *smsprm;

  TRACE_FUNCTION ("cmhSMS_GetPrmSIM ()");

  if (data NEQ NULL)
  {    
    pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];
    
    smsprm = (T_ACI_SMS_SIM_PARAMS *)&data[(dataLen <= MIN_SMS_PRM_LEN)?
                                            0: dataLen - MIN_SMS_PRM_LEN];

    if (smsprm->par_ind EQ NOT_PRESENT_8BIT)
    {
    
      /* ACI-SPR-16431: reset sca number */
      pSMSSetPrm->sca.c_num = 0;
      memset(pSMSSetPrm->sca.num, 0xFF, sizeof(pSMSSetPrm->sca.num)); 
      /* end of ACI-SPR-16431: reset sca number */
      
      return TRUE;              /* record is empty */
    }

    if ((smsprm->par_ind & SIM_SMSP_V_SCA) EQ 0)
    {
       cmh_demergeTOA (smsprm->sca_ton_npi, &pSMSSetPrm->sca.ton,
                      &pSMSSetPrm->sca.npi);
      pSMSSetPrm->sca.c_num = (UBYTE)cmh_unpackBCD (pSMSSetPrm->sca.num,
                                                     smsprm->sca_addr,
                                                     (USHORT)(smsprm->sca_length - 1));
      pSMSSetPrm->sca.v_ton = TRUE;
      pSMSSetPrm->sca.v_npi = TRUE;

    }
    if ((smsprm->par_ind & SIM_SMSP_V_PID) EQ 0)
      pSMSSetPrm->pid = smsprm->pid;

    if ((smsprm->par_ind & SIM_SMSP_V_DCS) EQ 0)
      pSMSSetPrm->dcs = smsprm->dcs;

    if ((smsprm->par_ind & SIM_SMSP_V_VPREL) EQ 0)
    {
      pSMSSetPrm->vpRel = smsprm->vp_rel;
      pSMSSetPrm->msgType &= ~TP_VPF_MASK;
      pSMSSetPrm->msgType |= TP_VPF_RELATIVE;
    }
    else if ((pSMSSetPrm->msgType & TP_VPF_MASK) EQ TP_VPF_RELATIVE)
      pSMSSetPrm->msgType &= ~TP_VPF_MASK;

    return TRUE;
  }
  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_PutPrmSIM             |
+-------------------------------------------------------------------+

  PURPOSE : This function conbiness the parameters for one record of
            EF(SMSP) to be written to SIM.
*/
GLOBAL BOOL cmhSMS_PutPrmSIM (T_ACI_CMD_SRC srcId,
                              UBYTE         *data,
                              int           maxDataLen)
{
  T_SMS_SET_PRM *pSMSSetPrm;    /* points to SMS parameter set */
  T_ACI_SMS_SIM_PARAMS *smsprm;
  size_t datalen;

  TRACE_FUNCTION ("cmhSMS_PutPrmSIM ()");

  if (data NEQ NULL)
  {
    pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

    if (maxDataLen < MIN_SMS_PRM_LEN)
    {
      datalen = MIN_SMS_PRM_LEN;
      smsprm = (T_ACI_SMS_SIM_PARAMS *)data;
    }
    else
    {
      datalen = (size_t)maxDataLen;
      smsprm = (T_ACI_SMS_SIM_PARAMS *)&data[datalen - MIN_SMS_PRM_LEN];
    }
    memset (data, NOT_PRESENT_8BIT, datalen);

  /*
   *-------------------------------------------------------------
   * set the service center address
   *-------------------------------------------------------------
   */
    if (pSMSSetPrm->sca.c_num > 0)
    {
      smsprm->sca_ton_npi = cmh_mergeTOA (pSMSSetPrm->sca.ton,
                                          pSMSSetPrm->sca.npi);
      smsprm->sca_length = (UBYTE)cmh_packBCD (smsprm->sca_addr,
                                               pSMSSetPrm->sca.num,
                                               (USHORT)MINIMUM(pSMSSetPrm->sca.c_num,
                                                MAX_SMS_ADDR_DIG)) + 1;
      smsprm->par_ind &= ~SIM_SMSP_V_SCA;
    }

  /*
   *-------------------------------------------------------------
   * set PID, DCS and VP-REL
   *-------------------------------------------------------------
   */

    smsprm->pid = pSMSSetPrm->pid;
    smsprm->par_ind &= ~SIM_SMSP_V_PID;

    smsprm->dcs = pSMSSetPrm->dcs;
    smsprm->par_ind &= ~SIM_SMSP_V_DCS;

    if ((pSMSSetPrm->msgType & TP_VPF_MASK) EQ TP_VPF_RELATIVE)
    {
      smsprm->vp_rel = pSMSSetPrm->vpRel;
      smsprm->par_ind &= ~SIM_SMSP_V_VPREL;
    }
    return TRUE;
  }
  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_GetCbmirSIM           |
+-------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of EF(CBMIR)
            read from SIM.
*/
GLOBAL BOOL cmhSMS_GetCbmirSIM (T_ACI_CMD_SRC srcId,
                                UBYTE         *data,
                                int           dataLen)
{
  T_ACI_CBM_SIM_MID_RANGE *mid_range;
  USHORT lower_mid, upper_mid;
  int mid_entry;

  TRACE_FUNCTION ("cmhSMS_GetCbmirSIM ()");

  if (data NEQ NULL)
  {
/* Please be aware that to keep simulation test cases as less changed as 
   possible lets assume that IMSI and CBMs stored in the FFS and from SIM 
   card are the same, since we cannot access  FFS under SIMULATION*/
#ifndef _SIMULATION_
    /*
     * The CSCB parameters read from FFS should be restored only if the same SIM
     * is inserted ( check IMSI to ensure the same SIM), otherwise reset the CSCB
     * parameters to default values
     */
    if (memcmp (smsShrdPrm.cbmPrm.IMSI, simShrdPrm.imsi.field, simShrdPrm.imsi.c_field) NEQ 0)
    {
    /* Implements Measure # 9 */
      cmhSMS_clearCbmPrm();     
      return FALSE;
    }
#endif
    mid_range = (T_ACI_CBM_SIM_MID_RANGE *)data;
    mid_entry = 0;

    while (smsShrdPrm.cbmPrm.cbmFoundIds < MAX_IDENTS/2 AND
           mid_entry < dataLen)
    {
      lower_mid = (USHORT)mid_range->lowerLSB |
                  ((USHORT)mid_range->lowerMSB << 8);
      upper_mid = (USHORT)mid_range->upperLSB |
                  ((USHORT)mid_range->upperMSB << 8);
      mid_range++;
      mid_entry += 4;

      if (lower_mid NEQ NOT_PRESENT_16BIT OR
          upper_mid NEQ NOT_PRESENT_16BIT)
      {
/* Please be aware that to keep simulation test cases as less changed as 
   possible lets assume that  CBMs stored in the FFS and from SIM 
   card are the same, since we cannot access  FFS under SIMULATION*/
#ifndef _SIMULATION_
   
        /* Implements Measure # 73 */ 
        if (!cmhSMS_findMessageIds (lower_mid, upper_mid))
        {
          return FALSE;
        }

#else /* Just for simulation - copy data, received from SIM */
      smsShrdPrm.cbmPrm.msgId[smsShrdPrm.cbmPrm.cbmFoundIds * 2] = lower_mid;
      smsShrdPrm.cbmPrm.msgId[smsShrdPrm.cbmPrm.cbmFoundIds * 2 + 1] = upper_mid;
      smsShrdPrm.cbmPrm.cbmFoundIds++;
#endif
      }
    }
    return TRUE;
  }
  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_PutCbmirSIM           |
+-------------------------------------------------------------------+

  PURPOSE : This function conbines the parameters of EF(CBMIR)
            to be written to SIM.
*/
GLOBAL BOOL cmhSMS_PutCbmirSIM (T_ACI_CMD_SRC srcId,
                                UBYTE         *data,
                                int           maxDataLen)
{
  T_ACI_CBM_SIM_MID_RANGE *mid_range;
  USHORT lower_mid, upper_mid;
  int mid_entry;

  TRACE_FUNCTION ("cmhSMS_PutCbmirSIM ()");

  if (data NEQ NULL)
  {
    memset (data, NOT_PRESENT_8BIT, maxDataLen);

    mid_range = (T_ACI_CBM_SIM_MID_RANGE *)data;
    mid_entry = 0;

    while (smsShrdPrm.cbmPrm.cbmFoundIds < MAX_IDENTS/2 AND
           mid_entry < maxDataLen)
    {
      lower_mid = smsShrdPrm.cbmPrm.msgId[smsShrdPrm.cbmPrm.cbmFoundIds * 2];
      upper_mid = smsShrdPrm.cbmPrm.msgId[smsShrdPrm.cbmPrm.cbmFoundIds * 2 + 1];
      smsShrdPrm.cbmPrm.cbmFoundIds++;

      if (lower_mid NEQ NOT_PRESENT_16BIT OR
          upper_mid NEQ NOT_PRESENT_16BIT)
      {
        mid_range->lowerLSB = (UBYTE)lower_mid;
        mid_range->lowerMSB = (UBYTE)(lower_mid >> 8);
        mid_range->upperLSB = (UBYTE)upper_mid;
        mid_range->upperMSB = (UBYTE)(upper_mid >> 8);
        mid_range++;
        mid_entry += 4;
      }
    }
    return TRUE;
  }
  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_GetCbmiSIM            |
+-------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of one record of
            EF(SMSP) read from SIM.
*/
GLOBAL BOOL cmhSMS_GetCbmiSIM  (T_ACI_CMD_SRC srcId,
                                UBYTE         *data,
                                int           dataLen)
{
  T_ACI_CBM_SIM_MID_LIST **mid_list;
  USHORT mid;
  int mid_entry;

  TRACE_FUNCTION ("cmhSMS_GetCbmiSIM ()");

  if (data NEQ NULL)
  {

  /* Please be aware that to keep simulation test cases as less changed as 
   possible lets assume that IMSI and CBMs stored in the FFS and from SIM 
   card are the same, since we cannot access  FFS under SIMULATION*/
#ifndef _SIMULATION_
    /*
     * The CSCB parameters read from FFS should be restored only if the same SIM
     * is inserted ( check IMSI to ensure the same SIM), otherwise reset the CSCB
     * parameters to default values
     */
    if (memcmp (smsShrdPrm.cbmPrm.IMSI, simShrdPrm.imsi.field, simShrdPrm.imsi.c_field) NEQ 0)
    {
    /* Implements Measure # 9 */
      cmhSMS_clearCbmPrm();     
      return FALSE;
    }
#endif
    mid_list = (T_ACI_CBM_SIM_MID_LIST **)&data;
    mid_entry = 0;

    while (smsShrdPrm.cbmPrm.cbmFoundIds < MAX_IDENTS/2 AND
           mid_entry < dataLen)
    {
      mid = (USHORT)(*mid_list)->LSB |
            ((USHORT)(*mid_list)->MSB << 8);
      data += 2;           /* overcome TI alignment problem */
      mid_entry += 2;

      if (mid NEQ NOT_PRESENT_16BIT)
      {
/* Please be aware that to keep simulation test cases as less changed as 
   possible lets assume that  CBMs stored in the FFS and from SIM 
   card are the same, since we cannot access  FFS under SIMULATION*/
#ifndef _SIMULATION_

        /* Implements Measure # 73 */
        if (!cmhSMS_findMessageIds (mid, mid))
        {
          return FALSE;
        }

#else /* Just for simulation - copy data, received from SIM */
      smsShrdPrm.cbmPrm.msgId[smsShrdPrm.cbmPrm.cbmFoundIds * 2] = 
      smsShrdPrm.cbmPrm.msgId[smsShrdPrm.cbmPrm.cbmFoundIds * 2 + 1] = mid;
      smsShrdPrm.cbmPrm.cbmFoundIds++;
#endif
      }
    }
    return TRUE;
  }
  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_PutCbmiSIM            |
+-------------------------------------------------------------------+

  PURPOSE : This function conbines the parameters of EF(CBMI)
            to be written to SIM.
*/
GLOBAL BOOL cmhSMS_PutCbmiSIM (T_ACI_CMD_SRC srcId,
                               UBYTE         *data,
                               int           maxDataLen)
{
  T_ACI_CBM_SIM_MID_LIST **mid_list;
  USHORT lower_mid, upper_mid;
  UBYTE  mid_entry;

  TRACE_FUNCTION ("cmhSMS_PutCbmiSIM ()");

  if (data NEQ NULL)
  {
    memset (data, NOT_PRESENT_8BIT, maxDataLen);

    mid_list = (T_ACI_CBM_SIM_MID_LIST **)&data;
    mid_entry = 0;

    while (smsShrdPrm.cbmPrm.cbmFoundIds < MAX_IDENTS/2 AND
           mid_entry < maxDataLen)
    {
      lower_mid = smsShrdPrm.cbmPrm.msgId[smsShrdPrm.cbmPrm.cbmFoundIds * 2];
      upper_mid = smsShrdPrm.cbmPrm.msgId[smsShrdPrm.cbmPrm.cbmFoundIds * 2 + 1];
      smsShrdPrm.cbmPrm.cbmFoundIds++;

      if (lower_mid NEQ NOT_PRESENT_16BIT OR
          upper_mid NEQ NOT_PRESENT_16BIT)
      {
        while (lower_mid <= upper_mid AND mid_entry < maxDataLen)
        {
          (*mid_list)->LSB = (UBYTE)lower_mid;
          (*mid_list)->MSB = (UBYTE)(lower_mid >> 8);
          lower_mid++;
          data += 2;           /* overcome TI alignment problem */
          mid_entry += 2;
        }
      }
    }
    return TRUE;
  }
  return FALSE;
}

#ifdef SIM_TOOLKIT

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_FileUpdate            |
+-------------------------------------------------------------------+

  PURPOSE : This function processes the primitive SIM_FILE_UPDATE_IND
            to update the SMS parameters stored on the SIM.
*/
GLOBAL BOOL cmhSMS_FileUpdate (int ref, T_SIM_FILE_UPDATE_IND *fu)
{
  BOOL found = FALSE;
  int i;

  TRACE_FUNCTION ("cmhSMS_FileUpdate ()");

  for (i = 0; i < (int)fu->val_nr; i++)
  {
    if (!found AND
        (fu->file_info[i].v_path_info EQ TRUE AND
         fu->file_info[i].path_info.df_level1 EQ SIM_DF_GSM AND
         fu->file_info[i].path_info.v_df_level2 EQ FALSE AND
         (fu->file_info[i].datafield EQ SIM_CBMI OR
          fu->file_info[i].datafield EQ SIM_CBMIR OR
          fu->file_info[i].datafield EQ SIM_CBMID)) OR

         (fu->file_info[i].v_path_info EQ TRUE AND
          fu->file_info[i].path_info.df_level1 EQ SIM_DF_TELECOM AND
          fu->file_info[i].path_info.v_df_level2 EQ FALSE AND
          fu->file_info[i].datafield EQ SIM_SMSP))
    {
      found = TRUE;
    }
    if (fu->file_info[i].v_path_info EQ TRUE AND
        fu->file_info[i].path_info.df_level1   EQ SIM_DF_TELECOM AND
        fu->file_info[i].path_info.v_df_level2 EQ FALSE AND
        fu->file_info[i].datafield   EQ SIM_SMS)
    {
      smsShrdPrm.aci_sms_parameter.simTotal = 0;
      smsShrdPrm.aci_sms_parameter.simUsed  = 0;
    }
  }
  if (found)
  {
    smsShrdPrm.cbmPrm.cbmFoundIds = 0; /* new CBMI(R) */

    if (cmhSMS_ReadParams ((T_ACI_CMD_SRC)OWN_SRC_SAT, AT_CMD_CFUN, 1) EQ AT_EXCT)
    {
      smsShrdPrm.accessEnabled = FALSE;

      simShrdPrm.fuRef = ref;
      return FALSE;           /* reading files */
    }
    else
      return TRUE;            /* nothing to do */
  }
  else
  {
    simShrdPrm.fuRef = -1;    /* nothing to do */
    return TRUE;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhSMS_GetCbDtaDwnlSIM       |
+-------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of CBMID record of
            EF(CBMID) read from SIM.
*/
GLOBAL BOOL cmhSMS_GetCbDtaDwnlSIM  (T_ACI_CMD_SRC srcId,
                                     UBYTE         *data,
                                     int           dataLen)
{
  T_ACI_CBM_SIM_MID_LIST **mid_list;
  USHORT mid;
  int mid_entry;

  TRACE_FUNCTION ("cmhSMS_GetCbDtaDwnlSIM ()");

  if (data NEQ NULL)
  {
    smsShrdPrm.cbmPrm.CBDtaDwnlFoundIds = 0;
    memset (smsShrdPrm.cbmPrm.CBDtaDwnlIdent, NOT_PRESENT_8BIT,
            sizeof (smsShrdPrm.cbmPrm.CBDtaDwnlIdent));

    mid_list = (T_ACI_CBM_SIM_MID_LIST **)&data;
    mid_entry = 0;

    while (smsShrdPrm.cbmPrm.CBDtaDwnlFoundIds < MAX_IDENTS_SAT AND
           mid_entry < dataLen)
    {
      mid = (USHORT)(*mid_list)->LSB |
            ((USHORT)(*mid_list)->MSB << 8);
      data += 2;           /* overcome TI alignment problem */
      mid_entry += 2;

      if (mid NEQ NOT_PRESENT_16BIT)
      {
        smsShrdPrm.cbmPrm.CBDtaDwnlIdent[smsShrdPrm.cbmPrm.CBDtaDwnlFoundIds] = mid;
        smsShrdPrm.cbmPrm.CBDtaDwnlFoundIds++;
      }
    }
    return TRUE;
  }
  return FALSE;
}

#endif  /*of SIM_TOOLKIT */




/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : cmhSMS_decodeMsg   |
+--------------------------------------------------------------------+

  PURPOSE : decodes a SM in two steps

*/
GLOBAL UBYTE* cmhSMS_decodeMsg (T_sms_sdu *sms_sdu, T_rp_addr* rp_addr,
                                UBYTE vt_mti)
{
  UBYTE ccdRet;
  UBYTE direction;
  UBYTE recover_from_error;

  BUF_tpdu   sim_buf;   /* source for the second decoding */

  TRACE_FUNCTION("cmhSMS_decodeMsg ()");

  if( sms_sdu->l_buf EQ 0)
  {
    TRACE_EVENT("empty SDU: no decoding");
    return(NULL);
  }

  CCD_START;
  {

    MCAST( sim_pdu, SIM_PDU ); /* sim_pdu points to _decodedMsg */
    /*memset( sim_pdu, 0, sizeof (T_SIM_PDU) ); */

    /* decoding outer layer */
    ccdRet = ccd_decodeMsg ( CCDENT_SMS,
                             BOTH /* doesn't work with DOWNLINK!!! */,
                             (T_MSGBUF *) sms_sdu,
                             (UBYTE    *) _decodedMsg, /* target */
                             SMS_VT_SIM_PDU);

    if ( (ccdRet NEQ ccdOK) OR (!sim_pdu->v_tpdu)
         OR (_decodedMsg[0] NEQ SMS_VT_SIM_PDU) )
    {
      TRACE_EVENT_P1("CCD Decoding Error: %d", ccdRet);
      CCD_END;
      return NULL;
    }

    memcpy(rp_addr, &sim_pdu->rp_addr, sizeof(T_rp_addr) );

    memcpy(&sim_buf, &sim_pdu->tpdu, sizeof(BUF_tpdu) );

    /*memset( _decodedMsg, 0, sizeof (T_TP_SUBMIT) ); */

    if (vt_mti EQ SMS_VT_SUBMIT)
    {
      /* for decoding of SMS-SUBMIT (in response to +CMGR, +CMGL) */
      direction = UPLINK;
    }
    else
    {
      direction = DOWNLINK;
    }

    /* decoding inner layer */
    ccdRet = ccd_decodeMsg ( CCDENT_SMS,
                             direction,
                             (T_MSGBUF *) &sim_buf,
                             (UBYTE    *) _decodedMsg,  /* target */
                             vt_mti );

    if (ccdRet EQ ccdWarning)
      recover_from_error = TRUE;  /* Try to recover if a ccdWarning occoured */
    else
      recover_from_error = FALSE;

    if ((ccdRet EQ ccdError) OR (ccdRet EQ ccdWarning))
    {
      UBYTE ccd_err;
      USHORT parlist [6];

      TRACE_EVENT_P1 ("ccd_decodeMsg(): %02x", ccdRet);
      /*
       * get the first error
       */
      ccd_err = ccd_getFirstError (CCDENT_SMS, parlist);

      /*
       * Error Handling
       */
      do
      {
#ifndef NTRACE /* save some ROM */
/* Implements Measure#32: Row 1080,...,1093 */
        if (ccdRet EQ ccdError)
        {
          TRACE_EVENT_P1 ("ERR: %u ", ccd_err);
        }
        else if (ccdRet EQ ccdWarning)
        {
          TRACE_EVENT_P1 ("WRN: %u ", ccd_err);
        }
        switch (ccd_err)
        {
          case ERR_NO_MORE_ERROR:   
            TRACE_EVENT("the end of the error list is reached");
            break;
          case ERR_INVALID_CALC:    
            TRACE_EVENT("calculation of the element repeat value failed");
            break;
          case ERR_PATTERN_MISMATCH:
            TRACE_EVENT("a bit pattern was not expected");
            break;
          case ERR_COMPREH_REQUIRED:
            TRACE_EVENT("check for comprehension required failed");
            break;
          case ERR_IE_NOT_EXPECTED: 
            TRACE_EVENT("an information element was not expected");
            break;
          case ERR_IE_SEQUENCE:     
            TRACE_EVENT("wrong sequence of information elements");
            break;
          case ERR_MAX_IE_EXCEED:   
            TRACE_EVENT("maximum amount of repeatable information elements has exceeded");
            break;
          case ERR_MAX_REPEAT:      
            TRACE_EVENT("a repeatable element occurs too often in the message");
            break;
          case ERR_MAND_ELEM_MISS:  
            TRACE_EVENT("a mandatory information element is missing");
            break;
          case ERR_INVALID_MID:     
            TRACE_EVENT("the message ID is not correct");
            break;
          case ERR_INVALID_TYPE:    
            TRACE_EVENT("the information element is not a spare padding");
            break;
          case ERR_EOC_TAG_MISSING: 
            TRACE_EVENT("indefinite length is specified for the ASN.1-BER but the end tag is missing");
            break;
          case ERR_INTERNAL_ERROR:  
            TRACE_EVENT("an internal CCD error occured ");
            break;
          default:                  
            TRACE_EVENT("unknown error");
            break;
        }
#endif /* NTRACE */

        if (ccdRet EQ ccdWarning)
        {
          switch (ccd_err)
          {
            case ERR_PATTERN_MISMATCH:     /* recoverable warnings */
            case ERR_COMPREH_REQUIRED:
            case ERR_INTERNAL_ERROR:
              break;                       
            default:
              recover_from_error = FALSE;  /* in all other cases reset the recover flag */
          }
        }

        ccd_err = ccd_getNextError (CCDENT_SMS, parlist);
      }while (ccd_err NEQ ERR_NO_MORE_ERROR);
    }

    if ( (ccdRet EQ ccdError) OR
        ((ccdRet EQ ccdWarning) AND (recover_from_error EQ FALSE)) OR  /* not possible to recover */
         (_decodedMsg[0] NEQ vt_mti) )
    {
      TRACE_EVENT_P1("CCD Decoding Error Stage 2: %d", ccdRet);
      CCD_END;
      return NULL;
    }
    else if ((ccdRet EQ ccdWarning) AND (recover_from_error EQ TRUE))
    {
      TRACE_EVENT ("recovered from warning");
    }
  }
  CCD_END;

  return _decodedMsg;
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : cmhSMS_codeMsg     |
+--------------------------------------------------------------------+

  PURPOSE : encodes a SM in two steps

*/
GLOBAL void cmhSMS_codeMsg (T_sms_sdu *sms_sdu, UBYTE tp_vt_mti,
                            T_rp_addr* sc_addr, UBYTE tp_mti,
                            UBYTE* decoded_pdu )
{
  BUF_tpdu  sim_buf;      /* target for first coding  */
  UBYTE direction;

  TRACE_FUNCTION ("cmhSMS_codeMsg()");


  CCD_START;
  {
    UBYTE ccdRet;

    MCAST( sim_pdu, SIM_PDU );

    /* source of outer encoding */
    memset( sim_pdu, 0, sizeof (T_SIM_PDU) );

    /* target of outer encoding */
    /* memset( sms_sdu, 0, sizeof (T_sms_sdu) ); */
    sms_sdu->o_buf = 0;
    sms_sdu->l_buf = SIM_PDU_LEN << 3;

    sim_pdu->tp_vt_mti = SMS_VT_SIM_PDU;
    sim_pdu->tp_mti    = tp_mti;

    /* service center address exists */
    if (sc_addr)
    {
      memcpy(&sim_pdu->rp_addr, sc_addr, sizeof(T_rp_addr));
      sim_pdu->rp_addr.v_ton = 1;
      sim_pdu->rp_addr.v_npi = 1;
    }

    /* pdu data exists */
    if (decoded_pdu)
    {
      /* target of inner encoding */
      /* memset(sim_buf, 0, sizeof(BUF_tpdu)); */
      sim_buf.o_tpdu = 0;
//TISH modified for MSIM
//TISH: l_tpdu should be lenth of sim_buf.b_tpdu not BUF_tpdu.
//      sim_buf.l_tpdu = (sizeof(BUF_tpdu)) << 3;
      sim_buf.l_tpdu = (sizeof(sim_buf.b_tpdu)) << 3;

      if (tp_vt_mti EQ SMS_VT_DELIVER)
      {
        /* for encoding of SMS-DELIVER (only in +CMGW) */
        direction = DOWNLINK;
      }
      else
      {
        direction = UPLINK;
      }

      /* encoding inner layer */
      ccdRet = ccd_codeMsg (CCDENT_SMS,
                            direction,
                            (T_MSGBUF *) &sim_buf,  /* target */
                            (UBYTE *) decoded_pdu,
                            tp_vt_mti);

      if ( ccdRet NEQ ccdOK )
      {
        TRACE_EVENT_P1("CCD Coding Error: %d", ccdRet);
        CCD_END;
        return;
      }
      if (sim_buf.l_tpdu EQ 0)
      {
         TRACE_EVENT("Encoded length is zero");
      }

      memcpy(&sim_pdu->tpdu, &sim_buf, sizeof(BUF_tpdu));
      sim_pdu->v_tpdu = 1; /* set validy flag */      
    }

    /* encoding outer layer */
    ccdRet = ccd_codeMsg (CCDENT_SMS,
                          UPLINK,
                          (T_MSGBUF *) sms_sdu,  /* target */
                          (UBYTE *) sim_pdu,
                          SMS_VT_SIM_PDU);

    if ( ccdRet NEQ ccdOK )
    {
      TRACE_EVENT_P1("CCD Coding Error: %d", ccdRet);
      CCD_END;
      return;
    }
    if (sms_sdu->l_buf EQ 0)
    {
       TRACE_EVENT("Encoded length is zero");
    }
  }
  CCD_END;
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : cmhSMS_cpyDeliver  |
+--------------------------------------------------------------------+

  PURPOSE : converts a SMS-DELIVER message to the T_ACI_CMGL_SM 
            structure

*/
GLOBAL BOOL cmhSMS_cpyDeliver ( T_ACI_CMGL_SM * sm, T_sms_sdu * sms_sdu )
{
  T_rp_addr rp_addr;
  UBYTE* message;
  T_TP_DELIVER *sms_deliver;
  UBYTE alphabet;

  TRACE_FUNCTION("cmhSMS_cpyDeliver()");

  message = cmhSMS_decodeMsg(sms_sdu, &rp_addr, SMS_VT_DELIVER);

  if (message EQ NULL)
  {
    sm->stat = SMS_STAT_Invalid;
    return (FALSE);
  }

  if ( message[0] NEQ SMS_VT_DELIVER)
  {
    TRACE_EVENT_P1("wrong VTI = %x", message[0]);
  }

  sms_deliver = (T_TP_DELIVER*)message;

  /*
   *-----------------------------------------------------------------
   * process originator address
   *-----------------------------------------------------------------
   */
  /* Implements Measure # 126 */
  /* Since T_tp_oa and T_tp_da are of same type, there is no problem in 
     passing sms_deliver->tp_oa */
  cmhSMS_processOrigDestAddr (sm,
                              &rp_addr,
                              &sms_deliver->tp_oa);

  /*
   *-----------------------------------------------------------------
   * process first octet
   *-----------------------------------------------------------------
   */
  sm -> fo = sms_sdu->buf[sms_sdu->buf[0] + 1];

  /*
   *-----------------------------------------------------------------
   * process protocol identifier
   *-----------------------------------------------------------------
   */
  sm -> pid = sms_deliver->tp_pid;

  /*
   *-----------------------------------------------------------------
   * process data coding scheme
   *-----------------------------------------------------------------
   */
  sm -> dcs = sms_deliver->tp_dcs;


  /*
   *-----------------------------------------------------------------
   * process short message data, expanding from 7 to 8 bit
   *-----------------------------------------------------------------
   */

  if (sms_deliver->v_tp_ud)
  {
    /* user data */
    cmhSMS_expdSmsPp ( 0,
                       sms_deliver->tp_dcs,
                       sms_deliver->tp_ud.data,
                       sms_deliver->tp_ud.length,
                       sm -> data.data,
                       &sm->data.len );

    sm->udh.len = 0;
  }
  else
  {
    alphabet = cmhSMS_getAlphabetPp ( sms_deliver->tp_dcs );

    /* user data header */
    memcpy (sm->udh.data, sms_deliver->tp_udh_inc.tp_udh.data,
            sms_deliver->tp_udh_inc.tp_udh.c_data);
    sm->udh.len = sms_deliver->tp_udh_inc.tp_udh.c_data;

    /* user data (only user data can be 7bit data!!!) */
    cmhSMS_expdSmsPp ( (UBYTE)(sm->udh.len+1),
                       sms_deliver->tp_dcs,
                       sms_deliver->tp_udh_inc.data,
                       sms_deliver->tp_udh_inc.length,    /* ACI-SPR-9440 */
                       sm->data.data,
                       &sm->data.len );

    /* 7-bit data */
    if (alphabet EQ 0x00)
    {
      sm->data.len = sms_deliver->tp_udh_inc.length - ((sm->udh.len+1)*8)/7;

      /* minus space for the fill bits */
      if (((sm->udh.len+1)*8)%7 NEQ 0) sm->data.len--;
    }
    /* 8-bit data */
    else
    {
      sm->data.len = sms_deliver->tp_udh_inc.length-(sm->udh.len+1);
    }
  }

  /*
   *-----------------------------------------------------------------
   * process service center time stamp
   *-----------------------------------------------------------------
   */
  sm -> vp_rel = 0;
  memset ( &sm->vp_enh, 0, sizeof(T_ACI_VP_ENH) );

  cmhSMS_setVpabsCmh ( &sm -> scts,
                       (T_tp_vp_abs*) &sms_deliver->tp_scts );

  return (TRUE);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : cmhSMS_cpySubmit   |
+--------------------------------------------------------------------+

  PURPOSE : converts a SMS-SUBMIT message to the T_ACI_CMGL_SM
            structure

*/
GLOBAL BOOL cmhSMS_cpySubmit ( T_ACI_CMGL_SM * sm, T_sms_sdu * sms_sdu )
{
  T_rp_addr rp_addr;
  UBYTE* message;
  T_TP_SUBMIT *sms_submit;
  UBYTE alphabet;

  TRACE_FUNCTION("cmhSMS_cpySubmit()");

  message = cmhSMS_decodeMsg(sms_sdu, &rp_addr, SMS_VT_SUBMIT);

  if (message EQ NULL)
  {
    sm->stat = SMS_STAT_Invalid;  
    return (FALSE);
  }

  if ( message[0] NEQ SMS_VT_SUBMIT)
  {
    TRACE_EVENT_P1("wrong VTI = %x", message[0]);
  }

  sms_submit = (T_TP_SUBMIT*)message;


  /*
   *-----------------------------------------------------------------
   * process recepient address
   *-----------------------------------------------------------------
   */
  /* Implements Measure # 126 */
  cmhSMS_processOrigDestAddr (sm,
                              &rp_addr,
                              &sms_submit->tp_da);

  /*
   *-----------------------------------------------------------------
   * process first octet
   *-----------------------------------------------------------------
   */
  sm -> fo = sms_sdu->buf[sms_sdu->buf[0] + 1];

  /*
   *-----------------------------------------------------------------
   * process protocol identifier
   *-----------------------------------------------------------------
   */
  sm -> pid = sms_submit->tp_pid;

  /*
   *-----------------------------------------------------------------
   * process data coding scheme
   *-----------------------------------------------------------------
   */
  sm -> dcs = sms_submit->tp_dcs;

  /*
   *-----------------------------------------------------------------
   * process short message data, expanding from 7 to 8 bit
   *-----------------------------------------------------------------
   */

  if (sms_submit->v_tp_ud)
  {
    /* user data */
    cmhSMS_expdSmsPp ( 0,
                       sms_submit->tp_dcs,
                       sms_submit->tp_ud.data,
                       sms_submit->tp_ud.length,   /* ACI-SPR-9440 */
                       sm->data.data,
                       &sm->data.len );

    sm->udh.len = 0;
  }
  else
  {
    alphabet = cmhSMS_getAlphabetPp ( sms_submit->tp_dcs );

    /* user data header */
    memcpy (sm->udh.data, sms_submit->tp_udh_inc.tp_udh.data,
            sms_submit->tp_udh_inc.tp_udh.c_data);
    sm->udh.len = sms_submit->tp_udh_inc.tp_udh.c_data;

    /* user data (only user data can be 7bit data!!!) */
    cmhSMS_expdSmsPp ( (UBYTE)(sm->udh.len+1),
                       sms_submit->tp_dcs,
                       sms_submit->tp_udh_inc.data,
                       sms_submit->tp_udh_inc.length,  /* ACI-SPR-9440 */
                       sm->data.data,
                       &sm->data.len );

    /* 7-bit data */
    if (alphabet EQ 0x00)
    {
      sm->data.len = sms_submit->tp_udh_inc.length - ((sm->udh.len+1)*8)/7;

      /* minus space for the fill bits */
      if (((sm->udh.len+1)*8)%7 NEQ 0) sm->data.len--;
    }
    /* 8-bit data */
    else
    {
      sm->data.len = sms_submit->tp_udh_inc.length-(sm->udh.len+1);
    }
  }

  /*
   *-----------------------------------------------------------------
   * process validity period
   *-----------------------------------------------------------------
   */
  if (sms_submit->v_tp_vp_abs)
  {
    cmhSMS_setVpabsCmh ( &sm->scts, &sms_submit->tp_vp_abs );
    sm -> vp_rel = 0;
    memset ( &sm->vp_enh, 0, sizeof(T_ACI_VP_ENH) );
  }
  else if (sms_submit->v_tp_vp_enh)
  {
    cmhSMS_setVpenhCmh ( &sm->vp_enh, &sms_submit->tp_vp_enh );
    sm -> vp_rel = 0;
    memset ( &sm->scts, 0, sizeof(T_ACI_VP_ABS) );
  }
  else
  {
    sm -> vp_rel = sms_submit->tp_vp_rel;
    memset ( &sm -> scts, 0, sizeof(T_ACI_VP_ABS) );
    memset ( &sm -> vp_enh, 0, sizeof(T_ACI_VP_ENH) );
  }
  return (TRUE);
}

/* Implements Measure # 110 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : cmhSMS_cpyStatInd  |
+--------------------------------------------------------------------+

  PURPOSE : handles a status indication

*/
GLOBAL BOOL cmhSMS_cpyStatInd ( T_ACI_CDS_SM  * sm, 
                                T_MNSMS_STATUS_IND *mnsms_status_ind )
{
  T_rp_addr rp_addr;
  UBYTE* message;
  T_TP_STATUS *sms_status;
  
  TRACE_FUNCTION("cmhSMS_cpyStatInd()");

  message = cmhSMS_decodeMsg(&mnsms_status_ind->sms_sdu, &rp_addr, SMS_VT_STATUS);

  if (message EQ NULL)
  {
    return (FALSE);
  }

  if ( message[0] NEQ SMS_VT_STATUS)
  {
    TRACE_EVENT_P1("wrong VTI = %x", message[0]);
  }

  sms_status = (T_TP_STATUS*)message;


  /*
   *-----------------------------------------------------------------
   * process message type
   *-----------------------------------------------------------------
   */
  sm -> fo = mnsms_status_ind->sms_sdu.buf[mnsms_status_ind->sms_sdu.buf[0]+1];

  /*
   *-----------------------------------------------------------------
   * process message reference
   *-----------------------------------------------------------------
   */
  sm -> msg_ref = sms_status->tp_mr;

  /*
   *-----------------------------------------------------------------
   * process recipient address
   *-----------------------------------------------------------------
   */
  cmhSMS_getAdrStr ( sm->addr,
                     MAX_SMS_ADDR_DIG - 1,
                     sms_status->tp_ra.num,
                     sms_status->tp_ra.digits);

  sm->toa.ton = cmhSMS_getTon ( sms_status->tp_ra.ton );
  sm->toa.npi = cmhSMS_getNpi ( sms_status->tp_ra.npi );

  /*
   *-----------------------------------------------------------------
   * process service center time stamp
   *-----------------------------------------------------------------
   */
  cmhSMS_setVpabsCmh ( &sm->vpabs_scts, (T_tp_vp_abs*) &sms_status->tp_scts );

  /*
   *-----------------------------------------------------------------
   * process discharge time
   *-----------------------------------------------------------------
   */
  cmhSMS_setVpabsCmh ( &sm->vpabs_dt, (T_tp_vp_abs*) &sms_status->tp_dt );

  /*
   *-----------------------------------------------------------------
   * process status
   *-----------------------------------------------------------------
   */
  sm -> tp_status = sms_status->tp_st;

  return (TRUE);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : cmhSMS_getPhbEntry |
+--------------------------------------------------------------------+

  PURPOSE : gets phone book entry from coded SM

*/
GLOBAL void cmhSMS_getPhbEntry( UBYTE *buf,
                                T_ACI_PB_TEXT *alpha,
                                T_ACI_SMS_STAT status )
{
  UBYTE *p_data;
  UBYTE sca_len;
  UBYTE nplan;
  UBYTE no_bcd;
  UBYTE bcd[MAX_SMS_ADDR_DIG];
  CHAR  addr[MAX_SMS_ADDR_DIG+1];   /* for '\0' */
  USHORT octets;

  alpha->len = 0;

  p_data =  &buf[0];
  sca_len = buf[0];

  switch (status)
  {
    case SMS_STAT_RecUnread:
    case SMS_STAT_RecRead:
      if ((*(p_data+sca_len+1) & TP_MTI_MASK) EQ TP_MTI_SMS_STATUS_REP)
      {
        p_data++;
      }
      p_data += (sca_len+1) + 1;  /* sms-deliver */
      break;
    case SMS_STAT_StoUnsent:
    case SMS_STAT_StoSent:
      p_data += (sca_len+1) + 2;  /* sms-submit */
      break;
    default:
      return;
  }

  /* process originator/destination address */
  no_bcd = *p_data++;
  no_bcd = MINIMUM(no_bcd, MAX_SMS_ADDR_DIG);

  nplan  = *p_data++ & 0x0F; /* npi */

  octets = (no_bcd+1)/2;

  no_bcd = (UBYTE)cmh_unpackBCD (bcd, p_data, octets);

  if (nplan EQ 0x05) /* VO patch - if (patch) else ... */
  {
    UBYTE i,j = 0;
    for (i = 0 ; i < no_bcd; i=i+2)
    {
      addr[j] = (UBYTE)(bcd[i+1] << 4) + (UBYTE)(bcd[i]);
      j++;
    }
    addr[j] = '\0';
  }
  else
  {
    cmhSMS_getAdrStr ( addr,
                       MAX_SMS_ADDR_DIG,
                       bcd,
                       no_bcd );
  }

  /*
   *-----------------------------------------------------------------
   * process alphanumerical phonebook entry
   *-----------------------------------------------------------------
   */
  psaCC_phbMfwSrchNumPlnTxt ( addr, alpha );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : cmhSMS_codeDelRep  |
+--------------------------------------------------------------------+

  PURPOSE : codes RP-ACK for SMS-DELIVER-REPORT (without PID, DCS, UD)

*/
GLOBAL void cmhSMS_codeDelRep(T_sms_sdu *sms_sdu, T_rp_addr *sc_addr)
{
  UBYTE sca_len;  /* SCA len incl. length byte and toa */
  UBYTE sca_buf[MAX_SMS_ADDR_DIG/2 + 2];

  sca_len = CodeRPAddress( sca_buf,
                           sc_addr->c_num,
                           sc_addr->ton,
                           sc_addr->npi,
                           sc_addr->num );

  sms_sdu->l_buf = (sca_len+2)<<3;
  sms_sdu->o_buf = 0;

  memcpy ( sms_sdu->buf, sca_buf, sca_len);
  memset ( sms_sdu->buf+sca_len, 0, 2);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : CodeRPAddress      |
+--------------------------------------------------------------------+

  PURPOSE : converts address (in BCD format) to semi-octet representation
            (including address-length, ton and npi),
            for RP addresses (e.g SCA)

            returns number of octets including address-length,
            ton and npi

*/
GLOBAL UBYTE CodeRPAddress( UBYTE *buf, UBYTE  numDigits, UBYTE ton,
                      UBYTE npi,  UBYTE *bcd )
{
  UBYTE length;

  if (numDigits EQ 0)
  {
    length = 1;
    *buf = 0x00;
  }
  else
  {
    *buf++ = (numDigits+1)/2 + 1;
    *buf++ = (ton << 4) + npi + 0x80;

    length = (UBYTE)cmh_packBCD ( buf, bcd, numDigits );

    /* add length of length field, ton and npi */
    length += 2;
  }

  return length;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : CodeTPAddress      |
+--------------------------------------------------------------------+

  PURPOSE : converts address (in BCD format) to semi-octet representation
            (including address-length, ton and npi),
            for TP addresses (e.g OA or DA)

            returns number of octets including including address-length,
            ton and npi

*/
GLOBAL UBYTE CodeTPAddress( UBYTE *buf, UBYTE  numDigits, UBYTE ton,
                      UBYTE npi,  UBYTE *bcd )
{
  UBYTE length;

  *buf++ = numDigits;
  *buf++ = (ton << 4) + npi + 0x80;

  length = (UBYTE)cmh_packBCD ( buf, bcd, numDigits );

  /* add length of length field, to and npi */
  length += 2;

  return length;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : DecodeRPAddress    |
+--------------------------------------------------------------------+

  PURPOSE : converts RP address (SC address) from semi-octet
            representation to address in BCD format (including
            address-length, ton and npi)

            returns number of processed octets in buf

*/
GLOBAL UBYTE DecodeRPAddress(UBYTE *c_num, UBYTE *ton,
                       UBYTE *npi,  UBYTE *bcd, UBYTE *buf)
{
  UBYTE sca_length;  /* sca length is lenth in bytes plus TOA-octet */
  UBYTE processed_octets;

  sca_length = *buf++;
  *ton = (*buf & 0x70)>>4;
  *npi = *buf++ & 0x0F;

  *c_num = (UBYTE)cmh_unpackBCD (bcd, buf, (USHORT)(sca_length-1));

  processed_octets = (*c_num+1)/2 + 2;

  return processed_octets;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF           |
| STATE   : code                        ROUTINE : DecodeTPAddress    |
+--------------------------------------------------------------------+

  PURPOSE : converts TP address (DA, OA)from semi-octet
            representation to address in BCD format (including
            address-length, ton and npi)

            returns number of processed octets in buf

*/
GLOBAL UBYTE DecodeTPAddress(UBYTE *c_num, UBYTE *ton,
                       UBYTE *npi,  UBYTE *bcd, UBYTE *buf)
{
  UBYTE digits;
  UBYTE processed_octets;

  digits = *buf++;
  *ton = (*buf & 0x70)>>4;
  *npi = *buf++ & 0x0F;

  *c_num = (UBYTE)cmh_unpackBCD (bcd, buf, (USHORT)((digits+1)/2));

  processed_octets = (*c_num+1)/2 + 2;

  return processed_octets;
}




/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CMH_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_fillTpSubmit        |
+--------------------------------------------------------------------+

  PURPOSE : fills a T_TP_SUBMIT

*/
GLOBAL void cmhSMS_fillTpSubmit  (T_TP_SUBMIT*    tp_submit,
                                  T_ACI_CMD_SRC   srcId,
                                  UBYTE           msgType,
                                  UBYTE           mr,
                                  T_tp_da*        da_addr,
                                  T_ACI_SM_DATA*  data,
                                  UBYTE           septets,
                                  T_ACI_UDH_DATA* udh )
{
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set     */
  UBYTE  alphabet;

  TRACE_FUNCTION ("cmhSMS_fillTpSubmit()");

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  memset(tp_submit, 0, sizeof(T_TP_SUBMIT));

  tp_submit->tp_vt_mti = SMS_VT_SUBMIT;
  tp_submit->tp_rp = (msgType & TP_RP_MASK) ? 1 : 0;
  tp_submit->tp_mti = SMS_SUBMIT;
  tp_submit->tp_mr = mr;
  tp_submit->tp_srr  = (msgType & TP_SRR_MASK) ? 1 : 0;

  if (da_addr->digits NEQ 0x00)
    memcpy(&tp_submit->tp_da, da_addr, sizeof(T_tp_da));

  tp_submit->tp_pid = pSMSSetPrm -> pid;
  tp_submit->tp_dcs = pSMSSetPrm -> dcs;

  switch (msgType & VPF_MASK)
  {

    case VPF_RELATIVE:  /* validity period relative */
      tp_submit->v_tp_vp_rel = 1;
      tp_submit->tp_vpf = SMS_VPF_RELATIVE;
      tp_submit->tp_vp_rel = pSMSSetPrm -> vpRel;
      break;

    case VPF_ABSOLUTE:  /* validity period absolute */
      tp_submit->v_tp_vp_abs = 1;
      tp_submit->tp_vpf = SMS_VPF_ABSOLUTE;
      memcpy(&tp_submit->tp_vp_abs, &pSMSSetPrm -> vpAbs, sizeof(T_tp_vp_abs));
      break;

    case VPF_ENHANCED:  /* validity period enhanced */
      tp_submit->v_tp_vp_enh = 1;
      tp_submit->tp_vpf = SMS_VPF_ENHANCED;
      memcpy(&tp_submit->tp_vp_enh, &pSMSSetPrm -> vpEnh, sizeof(T_tp_vp_enh));
      break;

    default:  /* validity period not present */
      break;
  }

  
  alphabet = cmhSMS_getAlphabetPp ( pSMSSetPrm -> dcs );

  if ((udh) AND (udh->len))
  {
    tp_submit->tp_udhi = 1;
    tp_submit->v_tp_udh_inc = 1;
    tp_submit->tp_udh_inc.tp_udh.c_data = udh->len;
    memcpy(tp_submit->tp_udh_inc.tp_udh.data, udh->data, udh->len);

    /* copy user data */
    if ((data) AND (data->len))
    {
      tp_submit->tp_udh_inc.c_data = data->len;
      memcpy(tp_submit->tp_udh_inc.data, data->data, data->len);
    }

    /* 7-bit data */
    if (alphabet EQ 0x00)
    {
      tp_submit->tp_udh_inc.length = septets + (((udh->len+1)*8)/7);

      /* space for the fill bits */
      if (((udh->len+1)*8)%7 NEQ 0) 
      {
        tp_submit->tp_udh_inc.length++;
        if ((data EQ NULL) OR (data->len EQ 0))
        {
          tp_submit->tp_udh_inc.c_data = 1;
          tp_submit->tp_udh_inc.data[0] = 0; /* redundant  */
        }
      }
    }
    /* 8-bit data */
    else
    {
      tp_submit->tp_udh_inc.length = tp_submit->tp_udh_inc.c_data+udh->len+1; /* UDH length should be also added */
    }
  }
  else
  {
    /* validity flag set for both normal SMS and empty SMS */
      tp_submit->v_tp_ud = 1;

    if ((data) AND (data->len))
    {
      tp_submit->tp_ud.length = septets;
      tp_submit->tp_ud.c_data = data->len;
      memcpy(tp_submit->tp_ud.data, data->data, data->len); 
    }
    else 
    {
      /* enters when zero character SMS to be sent */
    }
  }
}



/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CMH_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_fillTpDeliver       |
+--------------------------------------------------------------------+

  PURPOSE : fills a T_TP_DELIVER (only used by +CMGW)

*/
GLOBAL void cmhSMS_fillTpDeliver (T_TP_DELIVER*   tp_deliver,
                                  T_ACI_CMD_SRC   srcId,
                                  UBYTE           msgType,
                                  T_tp_oa*        oa_addr,
                                  T_ACI_SM_DATA*  data,
                                  UBYTE           septets,
                                  T_ACI_UDH_DATA* udh )
{
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set     */
  UBYTE  alphabet;

  TRACE_FUNCTION ("cmhSMS_fillTpDeliver()");

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  memset(tp_deliver, 0, sizeof(T_TP_DELIVER));

  tp_deliver->tp_vt_mti = SMS_VT_DELIVER;
  tp_deliver->tp_rp = (msgType & TP_RP_MASK) ? 1 : 0;
  tp_deliver->tp_mti = SMS_DELIVER;

  if (oa_addr->digits NEQ 0x00)
    memcpy(&tp_deliver->tp_oa, oa_addr, sizeof(T_tp_oa));

  tp_deliver->tp_pid = pSMSSetPrm -> pid;
  tp_deliver->tp_dcs = pSMSSetPrm -> dcs;

  memcpy(&tp_deliver->tp_scts, &pSMSSetPrm -> vpAbs, sizeof(T_tp_scts));

  if ((data) AND (data->len))
  {
    alphabet = cmhSMS_getAlphabetPp ( pSMSSetPrm -> dcs );

    if ((udh) AND (udh->len))
    {
      tp_deliver->tp_udhi = 1;
      tp_deliver->v_tp_udh_inc = 1;
      tp_deliver->tp_udh_inc.tp_udh.c_data = udh->len;
      memcpy(tp_deliver->tp_udh_inc.tp_udh.data, udh->data, udh->len);

      /* copy user data */
      tp_deliver->tp_udh_inc.c_data = data->len;
      memcpy(tp_deliver->tp_udh_inc.data, data->data, data->len);

      /* 7-bit data */
      if (alphabet EQ 0x00)
      {
        tp_deliver->tp_udh_inc.length = septets + (((udh->len+1)*8)/7);

        /* space for the fill bits */
        if (((udh->len+1)*8)%7 NEQ 0) tp_deliver->tp_udh_inc.length++;
      }
      /* 8-bit data */
      else
      {
        tp_deliver->tp_udh_inc.length = tp_deliver->tp_udh_inc.c_data;
      }
    }
    else
    {
      tp_deliver->v_tp_ud = 1;
      tp_deliver->tp_ud.length = septets;
      tp_deliver->tp_ud.c_data = data->len;
      memcpy(tp_deliver->tp_ud.data, data->data, data->len);
    }
  }
  else /* if ((data) AND (data->len)) */
  {
    tp_deliver->v_tp_ud = 1;
    tp_deliver->tp_ud.length = 0;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CMH_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_fillTpCommand       |
+--------------------------------------------------------------------+

  PURPOSE : fills a T_TP_COMMAND

*/
GLOBAL void cmhSMS_fillTpCommand ( T_TP_COMMAND*   tp_command,
                                   UBYTE           fo,
                                   UBYTE           ct,
                                   UBYTE           mr,
                                   UBYTE           pid,
                                   UBYTE           mn,
                                   T_tp_da*        da_addr,
                                   T_ACI_CMD_DATA* data,
                                   T_ACI_UDH_DATA* udh)

{
  TRACE_FUNCTION ("cmhSMS_fillTpCommand()");

  memset(tp_command, 0, sizeof(T_TP_COMMAND));

  tp_command->tp_vt_mti = SMS_VT_COMMAND;
  tp_command->tp_srr  = (fo & TP_SRR_MASK) ? 1 : 0;
  tp_command->tp_mti = SMS_COMMAND;
  tp_command->tp_mr = mr;
  tp_command->tp_pid = pid;
  tp_command->tp_ct = ct;
  tp_command->tp_mn = mn;

  if (da_addr->digits NEQ 0x00)
    memcpy(&tp_command->tp_da, da_addr, sizeof(T_tp_da));

  if ((data) AND (data->len))
  {
    if ((udh) AND (udh->len))
    {
      tp_command->tp_udhi = 1;
      tp_command->v_tp_cdh_inc = 1;
      tp_command->tp_cdh_inc.tp_udh.c_data = udh->len;
      memcpy(tp_command->tp_cdh_inc.tp_udh.data, udh->data, udh->len);
      tp_command->tp_cdh_inc.c_data = data->len;
      memcpy(tp_command->tp_cdh_inc.data, data->data, data->len);
    }
    else
    {
      tp_command->v_tp_cd = 1;
      tp_command->tp_cd.c_data = data->len;
      memcpy(tp_command->tp_cd.data, data->data, data->len);
    }
  }
  else
  {
    /* command length field must be present */
    tp_command->v_tp_cd = 1;
    tp_command->tp_cd.c_data = 0;
  }

}
#ifdef FF_CPHS

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                   |
|                                 ROUTINE : cmhSMS_check_voice_mail_ind|
+----------------------------------------------------------------------+

  PURPOSE : 
*/

LOCAL BOOL cmhSMS_check_voice_mail_ind( T_TP_DELIVER *sms_deliver )
{
  TRACE_FUNCTION("cmhSMS_check_voice_mail_ind()");

  if ( sms_deliver->tp_oa.digits NEQ 4 OR
       sms_deliver->tp_oa.ton NEQ SMS_TON_ALPHANUMERIC )
  {
    TRACE_EVENT_P2("no vmi -> oa... digits:%d ton:%d", 
                    sms_deliver->tp_oa.digits, sms_deliver->tp_oa.ton);
    return FALSE;
  }

  switch (sms_deliver->tp_dcs & 0xF0)
  {
    case SMS_DCS_GRP_DEF:
    case SMS_DCS_GRP_CLASS:
      if ( (sms_deliver->tp_dcs & 0xF) EQ 0 OR
           (sms_deliver->tp_dcs & 0xF) EQ 0xC )
      {
        break;            /* GSM Default Alphabet */
      }
      /* no break, if FALSE */
      /*lint -fallthrough*/
    case SMS_DCS_GRP_COMPR:
    case SMS_DCS_GRP_CLASS_COMPR:
    case SMS_DCS_GRP_MW_STORE_UCS2:
      TRACE_EVENT_P1("no vmi -> dcs: %d", sms_deliver->tp_dcs);
      return FALSE;       /* no GSM Default Alphabet */
    case SMS_DCS_DATA_CLASS:
      if (sms_deliver->tp_dcs & 0x4)
      {
        TRACE_EVENT_P1("no vmi -> dcs: %d", sms_deliver->tp_dcs);
        return FALSE;     /* no GSM Default Alphabet */
      }
  }

  if (!sms_deliver->v_tp_ud)
  {
    TRACE_EVENT("no vmi -> no ud");
    return FALSE;       /* not only text present */
  }
  if ( ( sms_deliver->tp_ud.length NEQ 1 ) OR
       ( (sms_deliver->tp_ud.data[0] & 0x7F) NEQ ' ') )
  {
    TRACE_EVENT_P2("no vmi -> ud... length:%d data[0]:%d", 
                    sms_deliver->tp_ud.length, sms_deliver->tp_ud.data[0]);
    return FALSE;       /* no single space */
  }
  
  TRACE_FUNCTION ("CPHS VWI: send to cphs module");

  return TRUE;
}


/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                      |
|                                 ROUTINE : cmhSMS_voice_mail_ind_get_line|
+-------------------------------------------------------------------------+

  PURPOSE : 
*/
LOCAL void cmhSMS_voice_mail_ind_get_data( T_TP_DELIVER *sms_deliver, 
                                           UBYTE        *flag_set,
                                           T_CPHS_LINES *line )
{
  TRACE_FUNCTION("cmhSMS_voice_mail_ind_get_line()");

  /* get flag set, which is send in the first bit of the first
     byte of the originating address number. Due to the decoding
     each byte of the originating address number is now splitted
     into 2 bytes, so that the first 4 bits of the former first 
     byte can be found in byte 1 */
  *flag_set = sms_deliver->tp_oa.num[0] & 0x1;

  /* line is coded in character 1 bit 1 of second byte of GSM originating 
     address based on 7bit default alphabet. In the message indication we 
     get the data in the last bit of the first byte of the orginating address.
     Due to the decoding each byte of the originating address number is now 
     splitted into 2 bytes, so that the last 4 bits of the former first 
     byte can be found in byte 2 */
     
  if (sms_deliver->tp_oa.num[1] & 0x8)

  {
    *line = CPHS_LINE2;
  }
  else
  {
    *line = CPHS_LINE1;
  }
}


/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                   |
|                                 ROUTINE : cmhSMS_voice_mail_ind      |
+----------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL BOOL cmhSMS_voice_mail_ind( T_sms_sdu *sms_sdu)
{
  T_TP_DELIVER *sms_deliver;
  T_rp_addr rp_addr;
  T_CPHS_LINES line;
  UBYTE flag_set;
  UBYTE *message;
  
  TRACE_FUNCTION("cmhSMS_voice_mail_ind()");

  /* check if cphs is active */
  

  /* check if indication is voice mail indication */
  if ( sms_sdu )
  {   
    message = cmhSMS_decodeMsg(sms_sdu, &rp_addr, SMS_VT_DELIVER);
                 
    if (message EQ NULL) 
      return FALSE;

    if ( message[0] NEQ SMS_VT_DELIVER)
    {
      TRACE_EVENT_P1("wrong VTI = %x", message[0]);
    }

    sms_deliver = (T_TP_DELIVER*)message;

    if ( cphs_check_status() EQ CPHS_OK )
    {
      if ( cmhSMS_check_voice_mail_ind(sms_deliver) )
      {
        cmhSMS_voice_mail_ind_get_data(sms_deliver, &flag_set, &line);

        cphs_voice_mail_ind(flag_set, line);

        return TRUE;
      }
    }
  }
  return FALSE;
}

#endif /* FF_CPHS*/


/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_SMSF                   |
|                                 ROUTINE : cmhSMS_resetMtDsCnmiParam  |
+----------------------------------------------------------------------+
  PURPOSE : this function resets the <mt> and <ds> parameter for the
            SMS acknowledge in Phase2+ mode (+CNMA).
*/
GLOBAL void cmhSMS_resetMtDsCnmiParam(void)
{
    smsShrdPrm.mtHndl = 0; /* reset <mt> and <ds> param in error case */
    smsShrdPrm.srHndl = 0;

    smsShrdPrm.CNMImt = CNMI_MT_NoSmsDeliverInd;
    smsShrdPrm.CNMIds = CNMI_DS_NoSmsStatRpt;
    /* Implements measure 149 */
    cmhSMS_sendConfigureReq (FALSE);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSS                     |
| STATE   : code             ROUTINE : sAT_PlusCMGWPdu              |
+-------------------------------------------------------------------+

  PURPOSE : This is the internal function which is responsible for 
            storing messages into the mobile station or SIM card.
            AT command which is responsible for writing a short
            message to memory in PDU mode.
*/
GLOBAL BOOL cmhSMS_storePduToSim( T_ACI_CMD_SRC  srcId,
                                      UBYTE          stat,
                                      T_ACI_SM_DATA  *pdu )
{
  T_ACI_RETURN  ret;

  TRACE_FUNCTION ("cmhSMS_storePduToSim()");

  /* check if command executable */
  if(!cmhSMS_checkAccess (srcId, &ret))
  {
    return FALSE;
  }

  {
    PALLOC (mnsms_store_req, MNSMS_STORE_REQ);

    mnsms_store_req -> mem_type  = smsShrdPrm.mem2;
    mnsms_store_req -> rec_num   = CMGW_IDX_FREE_ENTRY;
    mnsms_store_req -> condx     = SMS_CONDX_OVR_NON;
    mnsms_store_req -> status    = stat;

    if ( (pdu->len > 0) AND (pdu->len <= SIM_PDU_LEN) )
    {
      mnsms_store_req->sms_sdu.l_buf = pdu->len * 8;
      mnsms_store_req->sms_sdu.o_buf = 0;
      memcpy (mnsms_store_req->sms_sdu.buf, pdu->data, pdu->len);
    }
    else
    {
      TRACE_FUNCTION ("cmhSMS_storePduToSim() : wrong PDU len");
      PFREE (mnsms_store_req);
      return FALSE;
    }

    PSENDX (SMS, mnsms_store_req);
  }

  smsShrdPrm.smsEntStat.curCmd    = AT_CMD_CMGW;
  smsShrdPrm.owner = (T_OWN)srcId;
  smsShrdPrm.smsEntStat.entOwn    = srcId;
  smsShrdPrm.uiInternalSmsStorage = srcId;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_removeStatusRFUBits |
+--------------------------------------------------------------------+

  PURPOSE :  ACI-SPR-17004: Method for removing of SMS RFU status 
  bits as defined by GSM 11.11 section 9.3
  
*/
GLOBAL void cmhSMS_removeStatusRFUBits  ( UBYTE* status )
{
  TRACE_FUNCTION("cmhSMS_removeStatusRFUBits()");

  /* See GSM 11.11 section 10.5.3 for position of RFU bits in status */

  /* test the pattern of the three lowest bits, match for "101" */ 
  if ((*status & 0x07) EQ 0x05)
  {
    /* set the highest three bits to 0 */
    *status &= 0x1F; 
  } 
  else
  {
    /* set the highest five bits to 0 */
    *status &= 0x07; 
  }

}

/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_SMSF                        |
| STATE   : code                ROUTINE : cmhSMS_convertDeliverStatusToACI|
+-------------------------------------------------------------------------+

  PURPOSE :  converts the SMS delivery status to ACI CPRSM status.
  
*/
GLOBAL  T_ACI_CPRSM_MOD cmhSMS_convertDeliverStatusToACI ( UBYTE status )
{
  T_ACI_CPRSM_MOD mode;
  
  TRACE_FUNCTION("cmhSMS_convertDeliverStatusToACI()");

  switch (status)
  {
    case SMS_DELIVER_STATUS_PAUSE:
      mode = CPRSM_MOD_Pause;
      break;
    case SMS_DELIVER_STATUS_RESUME:      
      mode = CPRSM_MOD_Resume;
      break;
    default:
      mode = CPRSM_MOD_NotPresent;
  }  
  return mode;
}

#ifdef FF_CPHS_REL4
/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_SMSF                        |
| STATE   : code                ROUTINE : cmhSMS_chk_WtngInd              |
+-------------------------------------------------------------------------+

  PURPOSE : This function expect an already decoded SMS in the T_TP_DELIVER 
            format and checks if it is a "special SMS" (refer to TS 23.040, 
            chpt. 9.2.3.24.2).

*/
LOCAL BOOL cmhSMS_chk_WtngInd( T_TP_DELIVER *pSms )
{

 TRACE_FUNCTION("cmhSMS_chk_WtngInd");
  return ( (pSms->tp_udhi)                        AND /* it is an UDH message */
           (pSms->v_tp_udh_inc)                   AND /* UDH successfuly decoded */
           (pSms->tp_udh_inc.tp_udh.c_data >= 4)  AND /* min len of special SMS */
           (pSms->tp_udh_inc.tp_udh.data[0] EQ 0x01)
         ); 
}


/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_SMSF                        |
| STATE   : code                ROUTINE : cmhSMS_chk_SpclMsg              |
+-------------------------------------------------------------------------+

  PURPOSE : This function checks if the received SMS is a special sms which
  indicates the message waiting status.If so,the message will be  processed
  and TRUE will be returned,indicating that the message was already handled.
  If it is no special message , the function will returns FALSE causing the
  SMS to be treated in the common way.

*/

GLOBAL BOOL cmhSMS_chk_SpclMsg( T_MNSMS_MESSAGE_IND *mnsms_message_ind )
{
  BOOL res    = FALSE ;
  T_MSG_WAITING_TYPE msgType = MSG_WAITING_TYPE_INVALID;
  U8 *ptr;
  T_TP_DELIVER *smDlvr;

  TRACE_FUNCTION( "cmhSMS_chk_SpclMsg()" );

  if(!psaSIM_ChkSIMSrvSup( SRV_No_54 ))
  {
    return res;
  }

  if (&mnsms_message_ind->sms_sdu)
  {
    T_rp_addr rpAddr;
    UBYTE     *pMsg;

    pMsg = cmhSMS_decodeMsg( &mnsms_message_ind->sms_sdu, &rpAddr, SMS_VT_DELIVER );
    smDlvr = (T_TP_DELIVER *)pMsg;

    if (pMsg)
    {
      if (pMsg[0] NEQ SMS_VT_DELIVER)
      {
        TRACE_EVENT_P1( "wrong VTI = %x", (int)pMsg[0] );
      }
      
      /* Check the reqired fields in the SMS to qaulify for Special Message */

      if (cmhSMS_chk_WtngInd( smDlvr ))
      {

        /* For Loop: Decode the SMS properly as ,more than one type of message
           can   be   indicated    within   one   SMS  message. A  varible for 
           tp_udh_inc.tp_udh.data  has  been  avoided ,   therefore  For  loop 
           seems to be lengthy,
        */
        for( ptr = smDlvr->tp_udh_inc.tp_udh.data ;
             ptr < &smDlvr->tp_udh_inc.tp_udh.data[smDlvr->tp_udh_inc.tp_udh.c_data ];
             ptr +=(ptr[1] + 2 ))
             {
               /* Filling the Message Type */

               msgType  = (ptr[2] & 0x03);             /* see TS 23.040 */

               /* Update Status field in EF-MWIS according to the values already
                  present in the SIM and the current values in the recieved  SMS.
                  Also  if  the count for any type of Special SMS is  ZERO  then
                  also update the status.
               */
               if( (!(smsShrdPrm.MWISdata.mwiStat >> msgType & 1U)) OR (ptr[3] EQ 0) )
                 {
                   smsShrdPrm.MWISdata.mwiStat ^= 1U << msgType;
                 }

               /* Filling the count for specfic type of SMS  */

               switch ( msgType )
               {
                 case MSG_WAITING_TYPE_VOICE :
                   smsShrdPrm.MWISdata.mwis_count_voice = ptr[3];
                   break;
                 case MSG_WAITING_TYPE_FAX :
                   smsShrdPrm.MWISdata.mwis_count_fax = ptr[3];
                   break;
                 case MSG_WAITING_TYPE_EMAIL :
                   smsShrdPrm.MWISdata.mwis_count_email = ptr[3];
                   break;
                 case MSG_WAITING_TYPE_OTHER :
                   smsShrdPrm.MWISdata.mwis_count_other = ptr[3];
                   break;
                 default:
                   TRACE_EVENT("INVALID MESSAGE TYPE");
                   return FALSE;
               }
             }

             /* Writing decoded data to EF-MWIS  */

             if(cmhSIM_WrMwis(CMD_SRC_NONE,MWIS_MOD_Write,1,
                &smsShrdPrm.MWISdata) NEQ AT_EXCT )
             {
               TRACE_EVENT(" Error in Sending the Sim Update Request ");
             }

        /* If message was stored we need to indicate this, too */

        if (mnsms_message_ind->rec_num NEQ SMS_RECORD_NOT_EXIST) 
        {
          cmhSMS_SMSMemory( mnsms_message_ind );
        }  
        res = TRUE; /* message processed, nothing left to do */
      }
    }
  }
  return res;
}
#endif
/* Implements Measure # 73 */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
|                            ROUTINE : cmhSMS_findMessageIds        |
+-------------------------------------------------------------------+
  PARAMETERS  : lower_mid - lower message id
                upper_mid - upper message id
  RETURN      : TRUE if message has been found, else FALSE

  PURPOSE : Find message ids based on the mids given as parameters and 
            update smsShrdPrm.cbmPrm.cbmFoundIds based on the same.

*/
LOCAL BOOL cmhSMS_findMessageIds (USHORT lower_mid, USHORT upper_mid)
{
  TRACE_FUNCTION("cmhSMS_findMessageIds()");
  /*
   * If the msg ids are same that in FFS and SIM, then restore else reset
   * to default values
   */
  if (smsShrdPrm.cbmPrm.msgId[smsShrdPrm.cbmPrm.cbmFoundIds * 2] EQ lower_mid
    AND smsShrdPrm.cbmPrm.msgId[smsShrdPrm.cbmPrm.cbmFoundIds * 2 + 1] EQ upper_mid)
  {
    smsShrdPrm.cbmPrm.cbmFoundIds++;
  }
  else
  {
    cmhSMS_clearCbmPrm();     
    return FALSE;
  }
  return TRUE;
}

/* Implements Measure # 126 */
/*
+---------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF                  |
| STATE   : code                        ROUTINE : cmhSMS_processOrigDestAddr|
+---------------------------------------------------------------------------+
  PARAMETERS  : sm - Structure into which the data is filled. 
                rp_addr - RP Address used for filling sm
                tp_addr - destination or origination address that has to be 
                          filled in sm  
  RETURN      : None

  PURPOSE : Processes originator address and converts it into 8 bit or string
            format as requried

*/
LOCAL void cmhSMS_processOrigDestAddr (T_ACI_CMGL_SM  *sm, 
                                       T_rp_addr      *rp_addr,
                                       T_tp_da        *tp_addr)
{
  TRACE_FUNCTION("cmhSMS_processOrigDestAddr()");
  /*
   *-----------------------------------------------------------------
   * process address
   *-----------------------------------------------------------------
   */
  if (tp_addr->ton EQ SMS_TON_ALPHANUMERIC)
  {
    UBYTE i,j = 0;
    UBYTE dest_len;
    CHAR address_buf[MAX_SMS_ADDR_DIG/2];

    for (i = 0 ; i < tp_addr->digits ; i=i+2)
    {
      address_buf[j] = (UBYTE)(tp_addr->num[i+1] << 4) + (UBYTE)(tp_addr->num[i]);
      j++;
    }

    if( tp_addr->digits % 2 )
    {
      address_buf[j-1] = address_buf[j-1] & 0x0f;
    }

    dest_len = utl_cvt7To8 ( (UBYTE*)address_buf, j, (UBYTE*)sm->adress, 0);
    sm -> adress[dest_len] = '\0';

    if( (tp_addr->digits % 2) AND (dest_len + 3) % 8 >= 4 )
      sm -> adress[dest_len--] = '\0';
    else
      sm -> adress[dest_len] = '\0';

  }
  else
  {
    cmhSMS_getAdrStr ( sm -> adress,
                       MAX_SMS_NUM_LEN - 1,
                       tp_addr->num,
                       tp_addr->digits );
  }
  sm -> toa.ton = cmhSMS_getTon ( tp_addr->ton );
  sm -> toa.npi = cmhSMS_getNpi ( tp_addr->npi );
  /*
   *-----------------------------------------------------------------
   * process service center address
   *-----------------------------------------------------------------
   */
  cmhSMS_getAdrStr ( sm -> sca,
                     MAX_SMS_NUM_LEN - 1,
                     rp_addr->num,
                     rp_addr->c_num);

  sm -> tosca.ton = cmhSMS_getTon ( rp_addr->ton );
  sm -> tosca.npi = cmhSMS_getNpi ( rp_addr->npi );

  /*
   *-----------------------------------------------------------------
   * process alphanumerical phonebook entry
   *-----------------------------------------------------------------
   */
  psaCC_phbMfwSrchNumPlnTxt ( sm -> adress, &sm -> alpha );
}

/* Implements Measure # 149 */
/*
+---------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF                  |
| STATE   : code                        ROUTINE : cmhSMS_sendConfigureReq   |
+---------------------------------------------------------------------------+
  PARAMETERS  : v_cmms_val - Set to TRUE if cmms_mode and tmms_val has to be 
                             filled.
  RETURN      : None

  PURPOSE : Sends mnsms_configure_req to inform the SMS entity 
            about the parameter changes. v_cmms_val is TRUE if cmms_mode and 
            tmms_val has to be filled.

*/
GLOBAL void cmhSMS_sendConfigureReq (BOOL v_cmms_val)
{
  TRACE_FUNCTION("cmhSMS_sendConfigureReq()");
  { /* inform the SMS entity about the parameter changes */
    PALLOC (mnsms_configure_req, MNSMS_CONFIGURE_REQ);

    /* fill in primitive parameter: command request */
    mnsms_configure_req -> pref_mem_3 = smsShrdPrm.mem3;
    mnsms_configure_req -> mt         = smsShrdPrm.mtHndl;
    mnsms_configure_req -> ds         = smsShrdPrm.srHndl;
    mnsms_configure_req -> mhc        = (smsShrdPrm.CSMSservice NEQ CSMS_SERV_GsmPh2Plus)
                                        ? SMS_MHC_PH2 : SMS_MHC_PH2PLUS;
    mnsms_configure_req->v_cmms_mode = mnsms_configure_req->v_tmms_val = 
                                                              v_cmms_val;
    if(v_cmms_val)
    {
      mnsms_configure_req -> cmms_mode = smsShrdPrm.CMMSmode;
      mnsms_configure_req->tmms_val = CMMS_MODE_TIMER_VAL;
    }

    PSENDX (SMS, mnsms_configure_req);
  }
}

/* Implements Measure # 110 */
/*
+---------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF                  |
| STATE   : code                        ROUTINE : cmhSMS_cpyMsgIndReadCnf   |
+---------------------------------------------------------------------------+
  PARAMETERS  : sm - Structure into which the data is filled.
                status - SMS record status
                sms_sdu - SMS SDU from which data has to be filled in sm
                rec_num - Record number of the SMS
  RETURN      : BOOL value indicating the status returned by cmhSMS_cpyDeliver 
                or cmhSMS_cpySubmit

  PURPOSE : handles a message indication or a read cnf

*/
GLOBAL BOOL cmhSMS_cpyMsgIndReadCnf (T_ACI_CMGL_SM  * sm, 
                                     UBYTE          *status, 
                                     T_sms_sdu      *sms_sdu,
                                     UBYTE          rec_num)
{
  BOOL ret = FALSE;
  TRACE_FUNCTION("cmhSMS_cpyMsgIndReadCnf()");

  /*
   *-----------------------------------------------------------------
   * decode SMS-DELIVER or SMS-SUBMIT
   *-----------------------------------------------------------------
   */

  /* 
   * ACI-SPR-17004: set RFU bits of status to 0 if present since these 
   * should be ignored in a GSM session (GSM 11.11 section 9.3)
   */
  cmhSMS_removeStatusRFUBits( status );  
   
  switch (*status)
  {
    case SMS_RECORD_REC_UNREAD:
    case SMS_RECORD_REC_READ:
      ret = cmhSMS_cpyDeliver ( sm, sms_sdu );
      break;
    case SMS_RECORD_STO_UNSENT:
    case SMS_RECORD_STO_SENT:
    /* ACI-SPR-17003: Handle unsupported statuses */  
    case SMS_RECORD_STAT_UNRCVD: 
    case SMS_RECORD_STAT_UNSTRD:
    case SMS_RECORD_STAT_STRD:            
      ret = cmhSMS_cpySubmit ( sm, sms_sdu );
      break;
    /* 21.Mar.2003 currently not used by SMS entity */
    /* 
     * case SMS_RECORD_INVALID: 
     *  ret = FALSE;
     *  break;
     */
    default:
      TRACE_EVENT("incorrect status");
  }

  /*
   *-----------------------------------------------------------------
   * process status (convert from PSA type to CMH type)
   *-----------------------------------------------------------------
   */
  if (sm->stat NEQ SMS_STAT_Invalid)
  {
     cmhSMS_getStatCmh ( *status, &sm -> stat );
  }

  /*
   *-----------------------------------------------------------------
   * process message reference
   *-----------------------------------------------------------------
   */
  sm -> msg_ref = rec_num;

  /*
   *-----------------------------------------------------------------
   * process tp_status
   *-----------------------------------------------------------------
   */
  sm ->tp_status = (UBYTE) -1;
  
  return (ret);
}

/* Implements Measure # 9 */
/*
+---------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SMSF                  |
| STATE   : code                        ROUTINE : cmhSMS_clearCbmPrm        |
+---------------------------------------------------------------------------+
  PARAMETERS  : None
  RETURN      : None

  PURPOSE : Resets smsShrdPrm.cbmPrm to default values

*/
LOCAL void cmhSMS_clearCbmPrm (void)
{
  TRACE_FUNCTION("cmhSMS_clearCbmPrm()");

  smsShrdPrm.cbmPrm.cbmMode = CBCH_ACCEPT;
  memset (smsShrdPrm.cbmPrm.msgId, DEF_MID_RANGE,
          sizeof (smsShrdPrm.cbmPrm.msgId));
  memset (smsShrdPrm.cbmPrm.dcsId, DEF_DCS_RANGE, 
          sizeof(smsShrdPrm.cbmPrm.dcsId));
}
/*==== EOF ========================================================*/
