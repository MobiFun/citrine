/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
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
|  Purpose :  This module defines the custom specific functions
|             for the component RR of the mobile station
+-----------------------------------------------------------------------------
*/

#ifndef RR_CSF_C
#define RR_CSF_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR
#define SAP_ACI

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>    /* offsetof */
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_rr.h"
#include "tok.h"
#include "rr.h"
#include "cl_imei.h"   /* IMEI common library */
#if defined(_SIMULATION_FFS_)
#include "../../services/ffs/ffs.h"
#include "ffs_coat.h"
#endif /* _SIMULATION_FFS_ */

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/
#if defined(_SIMULATION_FFS_)
LOCAL BOOL rr_csf_handle_ffs_read_result(T_FFS_SIZE status_read);
LOCAL BOOL rr_csf_handle_ffs_write_result(T_FFS_RET status_write);
LOCAL void rr_csf_check_ffs_dirs( void );
LOCAL void rr_csf_create_ffs_dirs(const char *dir_name);
#endif /* _SIMULATION_FFS_ */



/*==== VARIABLES ==================================================*/
/* Cell selection improvements LLD section 4.1.1.1 */
#if defined(_SIMULATION_)
T_LIST  win_black_list[MAX_REGIONS];  /* Simulates FFS for "Black List" */
T_CS_WHITE_LIST win_white_list;       /* Simulates FFS for "White List  */

/*Simulates FFS for lower rxlevel thresholds*/
UBYTE win_lower_rxlev_thr[MAX_NUM_BANDS] = { LOWER_RXLEV_THRESHOLD_850, LOWER_RXLEV_THRESHOLD, \
    LOWER_RXLEV_THRESHOLD, LOWER_RXLEV_THRESHOLD, LOWER_RXLEV_THRESHOLD };
 /*Simulates FFS for medium rxlevel thresholds*/
UBYTE win_medium_rxlev_thr[MAX_NUM_BANDS] = { MEDIUM_RXLEV_THRESHOLD, MEDIUM_RXLEV_THRESHOLD, \
    MEDIUM_RXLEV_THRESHOLD, MEDIUM_RXLEV_THRESHOLD, MEDIUM_RXLEV_THRESHOLD };
/*Simulates FFS for upper rxlevel thresholds*/
UBYTE win_upper_rxlev_thr[MAX_NUM_BANDS] = { UPPER_RXLEV_THRESHOLD, UPPER_RXLEV_THRESHOLD, \
    UPPER_RXLEV_THRESHOLD, UPPER_RXLEV_THRESHOLD, UPPER_RXLEV_THRESHOLD }; 
  /* Initialize the simulated rxlevel thresholds here itself */
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_read_imei           |
+--------------------------------------------------------------------+

  PURPOSE : The function reads the International mobile identity number
            from the non-volantile memory into an internal data structure.

*/

GLOBAL void rr_csf_read_imei (T_mob_ident *imei)
{
  UBYTE buf[CL_IMEI_SIZE];

  TRACE_FUNCTION ("rr_csf_read_imei()");

  imei->v_ident_dig  = TRUE;
  imei->v_tmsi_1     = FALSE;
  imei->ident_type   = 3;     /* set to TYPE_IMEISV */
  imei->c_ident_dig  = 16;
  imei->odd_even     = 0;

  /* Get IMEISV from IMEI common library */
  cl_get_imeisv(CL_IMEI_SIZE, buf, CL_IMEI_GET_SECURE_IMEI);

  /* copy digits */
  imei->ident_dig[0] = (buf [0] >> 4) & 0x0F; /* TAC 8 byte */
  imei->ident_dig[1] =  buf [0] & 0x0F;
  imei->ident_dig[2] = (buf [1] >> 4) & 0x0F;
  imei->ident_dig[3] =  buf [1] & 0x0F;
  imei->ident_dig[4] = (buf [2] >> 4) & 0x0F;
  imei->ident_dig[5] =  buf [2] & 0x0F;
  imei->ident_dig[6] = (buf [3] >> 4) & 0x0F;
  imei->ident_dig[7] =  buf [3] & 0x0F;
  imei->ident_dig[8] = (buf [4] >> 4) & 0x0F; /* SNR 6 byte */
  imei->ident_dig[9] =  buf [4] & 0x0F;
  imei->ident_dig[10]= (buf [5] >> 4) & 0x0F;
  imei->ident_dig[11]=  buf [5] & 0x0F;
  imei->ident_dig[12]= (buf [6] >> 4) & 0x0F;
  imei->ident_dig[13]=  buf [6] & 0x0F;
  imei->ident_dig[14]= (buf [7] >> 4) & 0x0F; /* SV 2 byte */
  imei->ident_dig[15]=  buf [7] & 0x0F;
  
  TRACE_EVENT_P8("RR INFO IMEI: TAC %1x%1x%1x%1x%1x%1x%1x%1x",
                  imei->ident_dig[0], imei->ident_dig[1], imei->ident_dig[2],
                  imei->ident_dig[3], imei->ident_dig[4], imei->ident_dig[5],
                  imei->ident_dig[6], imei->ident_dig[7]);
  TRACE_EVENT_P6("RR INFO IMEI: SNR %1x%1x%1x%1x%1x%1x",
                  imei->ident_dig[8],  imei->ident_dig[9], imei->ident_dig[10],
                  imei->ident_dig[11], imei->ident_dig[12], imei->ident_dig[13]);
  TRACE_EVENT_P2("RR INFO IMEI: SV  %1x%1x", imei->ident_dig[14],
                                             imei->ident_dig[15]);
}

#if !defined(NTRACE)
GLOBAL void rr_csf_trace_power (void)
{
  GET_INSTANCE_DATA;
  UBYTE setbands = rr_data->ms_data.rf_cap.setbands;
  UBYTE bands = rr_data->ms_data.rf_cap.bands;
  UBYTE stdbands;

  if (setbands EQ 0)
  {
    setbands = bands; /* auto detection, use all supported bands */
  }
  else
    setbands &= bands;/* bands set, use only supported bands */

  /* bands which might be supported with current std */
  stdbands = std_bands[std-1];

  TRACE_EVENT_P5 ("std=%d(%02x), (set)bands=%02x(%02x)->%02x",
    std, stdbands, rr_data->ms_data.rf_cap.setbands,
    rr_data->ms_data.rf_cap.bands, setbands);
  TRACE_EVENT_P6 ("%sGSM 900: power class=%u %s,%s%s%s",
    (bands&0x08)?"E":" ",
    rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_900].pow_class,
    (bands&0x09)?"rfcap":"     ",
    (stdbands&0x09)?"std":"   ",
    (setbands&0x09)?",supported":"",
    ((setbands&0x09) AND (setbands&0x09) EQ 0x01)?" (only GSM)":"");
  TRACE_EVENT_P4 ("DCS 1800: power class=%u %s,%s%s",
    rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_1800].pow_class,
    (bands&0x02)?"rfcap":"     ",
    (stdbands&0x02)?"std":"   ",
    (setbands&0x02)?",supported":"");
  TRACE_EVENT_P4 ("PCS 1900: power class=%u %s,%s%s",
    rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_1900].pow_class,
    (bands&0x04)?"rfcap":"     ",
    (stdbands&0x04)?"std":"   ",
    (setbands&0x04)?",supported":"");
  TRACE_EVENT_P4 ("GSM  850: power class=%u %s,%s%s",
    rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_850].pow_class,
    (bands&0x10)?"rfcap":"     ",
    (stdbands&0x10)?"std":"   ",
    (setbands&0x10)?",supported":"");
  TRACE_EVENT_P4 ("GSM  400: power class=%u %s,%s%s",
    rr_data->ms_data.rf_cap.rf_power.pow_class4[IDX_PWRCLASS_400].pow_class,
    (bands&0x60)?"rfcap":"     ",
    (stdbands&0x60)?"std":"   ",
    (setbands&0x60)?",supported":"");
  TRACE_EVENT_P1 ("GPRS multislot class =%u",
    rr_data->ms_data.rf_cap.rf_ms.gprs_ms_class);
}
#endif  /* !NTRACE */

#if defined(_SIMULATION_) && !defined(NTRACE)
#define TRACE_STRUCT(struct_member) TRACE_EVENT_P1("0x%02x " #struct_member , *((unsigned int*)&struct_member))

LOCAL void rr_csf_trace_class1 (T_mob_class_1* cm1)
{
  TRACE_STRUCT (cm1->rev_lev);
  TRACE_STRUCT (cm1->es_ind);
  TRACE_STRUCT (cm1->a5_1);
  TRACE_STRUCT (cm1->rf_pow_cap);
}

LOCAL void rr_csf_trace_class2 (T_mob_class_2* cm2)
{
  TRACE_STRUCT (cm2->rev_lev);
  TRACE_STRUCT (cm2->es_ind);
  TRACE_STRUCT (cm2->a5_1);
  TRACE_STRUCT (cm2->rf_pow_cap);
  TRACE_STRUCT (cm2->ps);
  TRACE_STRUCT (cm2->ss_screen);
  TRACE_STRUCT (cm2->mt_pp_sms);
  TRACE_STRUCT (cm2->vbs);
  TRACE_STRUCT (cm2->vgcs);
  TRACE_STRUCT (cm2->egsm);
  TRACE_STRUCT (cm2->class3);
  TRACE_STRUCT (cm2->lcsva);
  TRACE_STRUCT (cm2->ucs2_treat);
  TRACE_STRUCT (cm2->solsa);
  TRACE_STRUCT (cm2->cmsp);
  TRACE_STRUCT (cm2->a5_3);
  TRACE_STRUCT (cm2->a5_2);
}

LOCAL void rr_csf_trace_class3 (T_mob_class_3* cm3)
{
  TRACE_STRUCT (cm3->mb_value);
  TRACE_STRUCT (cm3->a5_7);
  TRACE_STRUCT (cm3->a5_6);
  TRACE_STRUCT (cm3->a5_5);
  TRACE_STRUCT (cm3->a5_4);
  if (cm3->v_radio_cap_2)
  {
    TRACE_STRUCT (cm3->radio_cap_2);
  }
  if (cm3->v_radio_cap_1)
  {
    TRACE_STRUCT (cm3->radio_cap_1);
  }
  if (cm3->v_rgsm_class)
  {
    TRACE_STRUCT (cm3->rgsm_class);
  }
  if (cm3->v_ms_class)
  {
    TRACE_STRUCT (cm3->ms_class);
  }

  TRACE_STRUCT (cm3->ucs2_treat);
  TRACE_STRUCT (cm3->ext_meas);
  if (cm3->v_measurement)
  {
    TRACE_STRUCT (cm3->measurement);
  }
  if (cm3->v_pos_method)
  {
    TRACE_STRUCT (cm3->pos_method);
  }
  if (cm3->v_edge_ms_class)
  {
    TRACE_STRUCT (cm3->edge_ms_class);
  }
  if (cm3->v_egde_struct)
  {
    TRACE_STRUCT (cm3->egde_struct.mod);
    if (cm3->egde_struct.v_egde_pow1)
    {
      TRACE_STRUCT (cm3->egde_struct.egde_pow1);
    }
    if (cm3->egde_struct.v_egde_pow2)
    {
      TRACE_STRUCT (cm3->egde_struct.egde_pow2);
    }
  }
#ifdef REL99
  if (cm3->v_gsm400_struct)
  {
    TRACE_STRUCT (cm3->gsm400_struct.gsm400_supp);
    TRACE_STRUCT (cm3->gsm400_struct.gsm400_cap);
  }
#endif
  if (cm3->v_gsm850_cap)
  {
    TRACE_STRUCT (cm3->gsm850_cap);
  }
  if (cm3->v_pcs1900_cap)
  {
    TRACE_STRUCT (cm3->pcs1900_cap);
  }
#ifdef REL99
  TRACE_STRUCT (cm3->umts_fdd);
  TRACE_STRUCT (cm3->umts_tdd);
  TRACE_STRUCT (cm3->cdma2000);
#endif
  if (cm3->v_dtm_ms)
  {
    TRACE_STRUCT (cm3->dtm_ms.dtm_g_ms_class);
    TRACE_STRUCT (cm3->dtm_ms.mac_support);
    if (cm3->dtm_ms.v_dtm_e_ms_class)
    {
      TRACE_STRUCT (cm3->dtm_ms.dtm_e_ms_class);
    }
  }
  if (cm3->v_single_band)
  {
    TRACE_STRUCT (cm3->single_band);
  }
}
#endif  /* _SIMULATION_ && !NTRACE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_ms_cap              |
+--------------------------------------------------------------------+

  PURPOSE : Read the mobile station capabilities from a non-volantile
            memory into an internal data structure.

*/
#if !defined(NTRACE)
#define  RFCAP_TEST
#endif /* !NTRACE */

GLOBAL void rr_csf_ms_cap (void)
{
  GET_INSTANCE_DATA;
  drv_Return_Type   pcm_ret;
  UBYTE             version;

  TRACE_FUNCTION ("rr_csf_ms_cap()");

  pcm_ret = pcm_ReadFile ((UBYTE *)EF_MSCAP_ID, SIZE_EF_MSCAP,
                          (UBYTE *)&rr_data->mscap, &version);
  if(pcm_ret NEQ PCM_OK )
  {
    TRACE_ERROR( "MS RF capability (EF_MSCAP_ID) -> PCM read error" );
  }
#if defined(RFCAP_TEST)
  else
  {
    TRACE_ARRAY ((UBYTE*)&rr_data->mscap, SIZE_EF_MSCAP, "EF_MSCAP_ID");
  }
#endif  /* RFCAP_TEST */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_read_rf_cap              |
+--------------------------------------------------------------------+

  PURPOSE : Read the mobile station capabilities from a non-volantile
            memory into an internal data structure.

*/
GLOBAL void rr_csf_read_rfcap (void)
{
  GET_INSTANCE_DATA;
#define FldSetEx(f,n,v) (f|=((((ULONG)f)&(~((ULONG)n##m)))|(((ULONG)v)<<n##s)))

  EF_RFCAP          rfcap_stream;
  BYTE              ccd_ret;
  UBYTE             version;
#if !defined(_SIMULATION_)
  unsigned int      ffs_ret;
#else
  drv_Return_Type   pcm_ret;
#endif /* _TMS470 */
  UBYTE  gprs_ms, cmsp;

  TRACE_FUNCTION ("rr_csf_read_rfcap()");

  /* save configured GPRS multislot class */
  gprs_ms = rr_data->ms_data.rf_cap.rf_ms.gprs_ms_class;
  /* save configured CMSP bit */
  cmsp = rr_data->ms_data.rf_cap.cmsp;

#if !defined(_SIMULATION_)
  ffs_ret = FFS_fread("/gsm/com/rfcap", (UBYTE *)&rfcap_stream, sizeof(EF_RFCAP));
  if (ffs_ret EQ sizeof(EF_RFCAP))
    version = TRUE;
  else
    version = FALSE;
#else
  pcm_ret = pcm_ReadFile ((UBYTE *)EF_RFCAP_ID, SIZE_EF_RFCAP,
                          (UBYTE *)&rfcap_stream, &version);
  if(pcm_ret EQ PCM_OK )
    version = TRUE;
  else
    version = FALSE;
#endif
#define RFCAP_DEFAULT /* G23M/RR-ENH-9401 */
#if !defined(RFCAP_DEFAULT)
  if (!version)
  {
    TRACE_ERROR ("MS RF capability (rfcap) -> FFS read error. Reset will follow ...");
    /*
     * the following delay is made on the target for reasons highlighted with
     * issue 7510: some customers do during their production the download of the
     * configuration to the flash whilst the target ist running; another reason is
     * that the target is not accessable (e.g. via PTCM) during resets; thus
     * the delay is present to allow that writing of config data is not disturbed
     * by the reset
     */
#if !defined(_SIMULATION_)
    TRACE_ERROR ("... in some seconds; download config data to flash please!");
    vsi_t_sleep(VSI_CALLER DELAY_RESET_NO_CONFIG_DATA);
#endif
    assert (FALSE);
  }
#else /* !RFCAP_DEFAULT */
  if (!version)
  {
/* Implements Measure#32: Row 150 */

    /* error output via trace interface */
    TRACE_ERROR( "MS RF capability (RFCAP) -> FFS/PCM read error: use default values instead");

#ifdef TI_PS_CUSTOM_RFCAP_DEFAULT
    /* Update with the Custom defined default RF capabilities */
    memcpy(&rfcap_stream, custom_rfcap_default, SIZE_EF_RFCAP);
    TRACE_EVENT_P2("Using Custom RFCAP values: Set_bands=%d, Freq_bands=%d", rfcap_stream.setbands, rfcap_stream.bands);
    #ifndef FF_L23_A5_3
    FldSetEx (rfcap_stream.encryption, rfCap_A5_3, NO);
    #endif
#else

    memset (&rfcap_stream, 0, SIZE_EF_RFCAP);

    /* default frequency bands */
    rfcap_stream.setbands = BAND_AUTO;
    rfcap_stream.bands = BAND_GSM_900|BAND_E_GSM|BAND_DCS_1800|BAND_PCS_1900|BAND_GSM_850;

    /* power classes */
    FldSetEx (rfcap_stream.power1, rfCap_900,  POWER_CLASS_4);
    FldSetEx (rfcap_stream.power1, rfCap_1800, POWER_CLASS_1);
    FldSetEx (rfcap_stream.power2, rfCap_1900, POWER_CLASS_1);
    FldSetEx (rfcap_stream.power2, rfCap_850,  POWER_CLASS_4);

#if CONFIG_TARGET_PIRELLI || CONFIG_TARGET_FCDEV3B
    /*
     * The Pirelli is known to always be 900/1800/1900 MHz triband,
     * so let's operate it as such by default.
     *
     * In the case of FCDEV3B there is at least a hypothetical possibility
     * that we may need to build a tri850 version some day, but we can
     * still default to tri900 - overriding defaults is what the FFS is for.
     */
    rfcap_stream.bands &= ~BAND_GSM_850;
    FldSetEx (rfcap_stream.power2, rfCap_850, 0);
#endif

    /* multi slot classes */
    FldSetEx (rfcap_stream.msGPRS, rfCap_MSC, MSLOT_CLASS_10);

    /* classmark 2 capabilities */
    FldSetEx (rfcap_stream.capability1, rfCap_esind, YES);
    FldSetEx (rfcap_stream.capability2, rfCap_vgcs, NOT_SUPPORTED);
    FldSetEx (rfcap_stream.capability2, rfCap_vbs, NOT_SUPPORTED);
    FldSetEx (rfcap_stream.capability1, rfCap_ppsms, SUPPORTED);
    FldSetEx (rfcap_stream.capability2, rfCap_ssc, SS_SCREEN_PHASE_2);
    FldSetEx (rfcap_stream.capability1, rfCap_ps, NOT_SUPPORTED);
    FldSetEx (rfcap_stream.capability1, rfCap_cmsp, SUPPORTED);
    FldSetEx (rfcap_stream.capability1, rfCap_solsa, NOT_SUPPORTED);
    FldSetEx (rfcap_stream.capability2, rfCap_usc2, SUPPORTED);
    FldSetEx (rfcap_stream.capability1, rfCap_lcsva, NOT_SUPPORTED);

    /* classmark 3 capabilities */
    FldSetEx (rfcap_stream.capability2, rfCap_extmeas, NOT_SUPPORTED);
    FldSetEx (rfcap_stream.capability2, rfCap_meas, NO);
    FldSetEx (rfcap_stream.switchmeasure, rfCap_smst, SM_1);
    FldSetEx (rfcap_stream.switchmeasure, rfCap_smt, SM_1);

    /* encryption algorithm */
    /*
     * note inverse logic for the A5/1 encryption algorithm between
     * classmark 1 and 2 on the one hand and radio access capability
     * on the other hand
     */
    FldSetEx (rfcap_stream.encryption, rfCap_A5_1, YES);
    FldSetEx (rfcap_stream.encryption, rfCap_A5_2, NO);
    #ifdef FF_L23_A5_3
    FldSetEx (rfcap_stream.encryption, rfCap_A5_3, YES);
    #else
    FldSetEx (rfcap_stream.encryption, rfCap_A5_3, NO);
    #endif
    FldSetEx (rfcap_stream.encryption, rfCap_A5_4, NO);
    FldSetEx (rfcap_stream.encryption, rfCap_A5_5, NO);
    FldSetEx (rfcap_stream.encryption, rfCap_A5_6, NO);
    FldSetEx (rfcap_stream.encryption, rfCap_A5_7, NO);
#endif /* TI_PS_CUSTOM_RFCAP_DEFAULT */
  }
#endif /* !RFCAP_DEFAULT */

  /* E-GSM includes P-GSM */
  if (FldGet (rfcap_stream.bands, rf_EGSM))
  {
    FldSetEx (rfcap_stream.bands, rf_900, SUPPORTED);
  }

#if defined(RFCAP_TEST)
  TRACE_ARRAY ((UBYTE*)&rfcap_stream, SIZE_EF_RFCAP, "rfcap_stream");
#endif  /* RFCAP_TEST */

  {/* decode stream into c struct */
    #define DECODE_TMP  MS_RF_CAPABILITY
    typedef struct
    {
      U32                       dummy; /*<  0:  4>                   */
      T_sdu                     sdu;   /*<  4: ? > Service Data Unit */
    } T_DECODE_TMP;
    T_MS_RF_CAPABILITY *ms_rf_cap;
    USHORT  off;
    PALLOC_SDU (encode, DECODE_TMP, (SIZE_EF_RFCAP + 1 ) * BITS_PER_BYTE);
    MALLOC (ms_rf_cap, sizeof (T_MS_RF_CAPABILITY));

    /* The dummy field is needed to make the structure compatible for *
     * the CCD. We need to initialise it to keep LINT happy */
    encode->dummy = 0x00;

    off = (USHORT)(encode->sdu.o_buf / BITS_PER_BYTE);
    memcpy ( &encode->sdu.buf[off+1], &rfcap_stream, SIZE_EF_RFCAP);
    encode->sdu.buf[off] = MS_RF_CAPABILITY;

    memset (ms_rf_cap, 0, sizeof (T_MS_RF_CAPABILITY));
    ccd_ret = ccd_decodeMsg(CCDENT_RR_COM, DOWNLINK,
                            (T_MSGBUF *)&encode->sdu, (UBYTE *)ms_rf_cap,
                            0xFF);
    rr_data->ms_data.rf_cap = ms_rf_cap->rf_cap; /* Struct copy */
    MFREE (ms_rf_cap);
    PFREE (encode);
  }

  /* restore configured GPRS multislot class */
  if (rr_data->ms_data.multislot_class_configured)
  {
    rr_data->ms_data.rf_cap.rf_ms.gprs_ms_class = gprs_ms;
    TRACE_EVENT_P1 ("'gprs_ms_class' configured to a value of %d (overwrite of rfcap)", gprs_ms);
  }

  /* restore configured CMSP bit */
  if (rr_data->ms_data.cmsp_configured)
  {
    rr_data->ms_data.rf_cap.cmsp = cmsp;
    TRACE_EVENT_P1 ("'cmsp' configured to a value of %d (overwrite of rfcap)", cmsp);
  }

#if defined(RFCAP_TEST)
  if (ccd_ret NEQ ccdOK)
  {
    /*
     * CCD has detected an error
     */
    UBYTE first_err;
    USHORT parlist [6];
/* Implements Measure#32: Row 152...165 */

    TRACE_EVENT_P1 ("ccd_decodeMsg(): %02x", ccd_ret);
    /*
     * get the first error
     */
    first_err = ccd_getFirstError (CCDENT_RR, parlist);

    /*
     * Error Handling
     */
    do
    {
/* Implements Measure#32: Row 152...165 */
      switch (first_err)
      {
        case ERR_NO_MORE_ERROR:
          TRACE_EVENT_P1("%u the end of the error list is reached", first_err);
          break;
        case ERR_INVALID_CALC:
          TRACE_EVENT_P1("%u calculation of the element repeat value failed", first_err);
          break;
        case ERR_PATTERN_MISMATCH:
          TRACE_EVENT_P1("%u a bit pattern was not expected", first_err);
          break;
        case ERR_COMPREH_REQUIRED:
          TRACE_EVENT_P1("%u check for comprehension required failed", first_err);
          break;
        case ERR_IE_NOT_EXPECTED:
          TRACE_EVENT_P1("%u an information element was not expected", first_err);
          break;
        case ERR_IE_SEQUENCE:
          TRACE_EVENT_P1("%u wrong sequence of information elements", first_err);
          break;
        case ERR_MAX_IE_EXCEED:
          TRACE_EVENT_P1("%u maximum amount of repeatable information elements has exceeded", first_err);
          break;
        case ERR_MAX_REPEAT:
          TRACE_EVENT_P1("%u a repeatable element occurs too often in the message", first_err);
          break;
        case ERR_MAND_ELEM_MISS:
          TRACE_EVENT_P1("%u a mandatory information element is missing", first_err);
          break;
        case ERR_INVALID_MID:
          TRACE_EVENT_P1("%u the message ID is not correct", first_err);
          break;
        case ERR_INVALID_TYPE:
          TRACE_EVENT_P1("%u the information element is not a spare padding", first_err);
          break;
        case ERR_EOC_TAG_MISSING:
          TRACE_EVENT_P1("%u indefinite length is specified for the ASN.1-BER but the end tag is missing", first_err);
          break;
        case ERR_INTERNAL_ERROR:
          TRACE_EVENT_P1("%u an internal CCD error occured ", first_err);
          break;
        default:
          TRACE_EVENT_P1("%u unknown error", first_err);
          break;
      }
      first_err = ccd_getNextError (CCDENT_RR, parlist);
    }while (first_err);

  }

#if 0
  if (ccd_ret < ccdError)
  {
    rr_csf_trace_power ();
  }
#endif  /* 0|1 */
#endif /* RFCAP_TEST */
}

GLOBAL void rr_csf_get_freq_bands (UBYTE *pfreq_bands)
{
  GET_INSTANCE_DATA;
  T_rf_cap *rfc = &rr_data->ms_data.rf_cap;

#if defined(_SIMULATION_)
  rr_csf_check_rfcap (FALSE);
#endif  /* _SIMULATION_ */

  if (rfc->setbands EQ BAND_AUTO)
    *pfreq_bands = rfc->bands;
  else
    *pfreq_bands = rfc->setbands &  rfc->bands;
#if !defined(NTRACE)
  rr_csf_trace_power ();
#endif  /* !NTRACE */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_get_classmark1      |
+--------------------------------------------------------------------+

  PURPOSE : fills c structure of the mobile station classmark 1
            information element; returns 0 if OK.

*/
/*lint -esym(765,rr_csf_get_classmark1) | used by MM*/
/*lint -esym(714,rr_csf_get_classmark1) | used by MM */
GLOBAL UBYTE rr_csf_get_classmark1 (T_mob_class_1 *mob_class_1)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rr_csf_get_classmark1()");

#if defined(_SIMULATION_)
  rr_csf_check_rfcap (FALSE);
#endif  /* _SIMULATION_ */

  *mob_class_1 = rr_data->ms_data.classmark1;

#if defined(_SIMULATION_) && !defined(NTRACE)
  rr_csf_trace_class1 (mob_class_1);
#endif  /* _SIMULATION_ && !NTRACE */
  return 0;/* no error */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_get_classmark2      |
+--------------------------------------------------------------------+

  PURPOSE : fills c structure of the mobile station classmark 2
            information element; returns 0 if OK.

*/
/*lint -esym(765,rr_csf_get_classmark2) | used by CC,MM */
/*lint -esym(714,rr_csf_get_classmark2) | used by CC,MM */
GLOBAL UBYTE rr_csf_get_classmark2 (T_mob_class_2 *mob_class_2)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rr_csf_get_classmark2()");

#if defined(_SIMULATION_)
  rr_csf_check_rfcap (FALSE);
#endif  /* _SIMULATION_ */

  *mob_class_2 = rr_data->ms_data.classmark2;

#if defined(_SIMULATION_) && !defined(NTRACE)
  rr_csf_trace_class2 (mob_class_2);
#endif  /* _SIMULATION_ && !NTRACE */
  return 0;/* no error */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_get_classmark3      |
+--------------------------------------------------------------------+

  PURPOSE : fills c structure of the mobile station classmark 3
            information element; returns 0 if OK.

*/
#if 0
GLOBAL UBYTE rr_csf_get_classmark3 (T_mob_class_3 *mob_class_3)
{
  TRACE_FUNCTION ("rr_csf_get_classmark3()");

#if defined(_SIMULATION_)
  rr_csf_check_rfcap (FALSE);
#endif  /* _SIMULATION_ */

  *mob_class_3 = rr_data->ms_data.classmark3;

#if defined(_SIMULATION_) && !defined(NTRACE)
  rr_csf_trace_class3 (mob_class_3);
#endif  /* _SIMULATION_ && !NTRACE */
  return 0;/* no error */
}
#endif

#ifdef GPRS
/* fills c structure of the mobile station radio access capability information element; returns 0 if OK */
/*lint -esym(765,rr_csf_get_radio_access_capability) | used by GMM,GRR */
/*lint -esym(714,rr_csf_get_radio_access_capability) | used by GMM,GRR */
GLOBAL UBYTE rr_csf_get_radio_access_capability (T_ra_cap *ra_cap)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rr_csf_get_radio_access_capability()");

#if defined(_SIMULATION_)
  rr_csf_check_rfcap (FALSE);
#endif  /* _SIMULATION_ */

  *ra_cap = rr_data->ms_data.ra_cap;
  return 0;
}
#endif

/* get pointer to mobile station RF capability c structure; returns NULL on error */
/*lint -esym(765,rr_csf_get_rf_capability) | used by GRR */
/*lint -esym(714,rr_csf_get_rf_capability) | used by GRR */
GLOBAL T_rf_cap*  rr_csf_get_rf_capability (void)
{
  GET_INSTANCE_DATA;
#if defined(_SIMULATION_)
  rr_csf_check_rfcap (FALSE);
#endif  /* _SIMULATION_ */

  return &rr_data->ms_data.rf_cap;
}

GLOBAL void rr_csf_check_rfcap (UBYTE init)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rr_csf_check_rfcap()");

  if (init OR (rr_data->ms_data.rf_cap.bands EQ 0))
  {
    rr_csf_read_rfcap ();
    rr_csf_fit_capability ();
  }
}


#ifndef REL99

#define RAC_RELEASE97
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_fit_capability      |
+--------------------------------------------------------------------+

  PURPOSE : fit classmark 2 and 3 and radio access capabilities
            depend on the value of 'std' and the readed values of
            /gsm/com/rfcap file according to R99 specifications
*/

GLOBAL UBYTE rr_csf_fit_capability (void)
{
  GET_INSTANCE_DATA;
  USHORT serving_cell;
  T_rf_cap      *rfc = &rr_data->ms_data.rf_cap;
#ifdef GPRS
  T_ra_cap      *rac = &rr_data->ms_data.ra_cap;
#endif
  T_mob_class_1 *cm1 = &rr_data->ms_data.classmark1;
  T_mob_class_2 *cm2 = &rr_data->ms_data.classmark2;
  T_mob_class_3 *cm3 = &rr_data->ms_data.classmark3;

  TRACE_FUNCTION ("rr_csf_fit_capability()");

  if (rr_data->nc_data[SC_INDEX].bcch_status NEQ EMPTY)
    serving_cell = rr_data->nc_data[SC_INDEX].arfcn;
  else
    serving_cell = (USHORT)-1;

  memset (cm2, 0, sizeof(T_mob_class_2));
  memset (cm3, 0, sizeof(T_mob_class_3));
#ifdef GPRS
  memset (rac, 0, sizeof(T_ra_cap));
#endif

  cm1->rev_lev = cm2->rev_lev = REV_LEV_PHASE_2;  /* PHASE_1 | PHASE_2 | R99_SUPPORT */
#ifdef GPRS
#if !defined(RAC_RELEASE97)
  rac->acc_cap.rev99 = REV_LEV_PHASE_1;           /* PHASE_1 | RELEASE99 */
#endif  /* !RAC_RELEASE97 */
#endif
  cm1->es_ind     = cm2->es_ind = rfc->es_ind;
  cm2->ps         = rfc->ps;
  cm2->ss_screen  = rfc->ss_screen;
  cm2->mt_pp_sms  = rfc->mt_pp_sms;
  cm2->vbs        = rfc->vbs;
  cm2->vgcs       = rfc->vgcs;
  cm2->class3     = SUPPORTED;             /* options indicated in classmark 3 */
  cm2->lcsva      = rfc->lcsva;
  cm2->ucs2_treat = cm3->ucs2_treat = rfc->ucs2_treat;
  cm2->solsa      = rfc->solsa;
  cm2->cmsp       = rfc->cmsp;

  /*
   * note inverse logic for the A5/1 encryption algorithm between
   * classmark 1 and 2 on the one hand and radio access capability
   * on the other hand
   */
  cm1->a5_1 = cm2->a5_1 = !rfc->a5_bits.a5_1;
  cm2->a5_2 = rfc->a5_bits.a5_2;
  cm2->a5_3 = rfc->a5_bits.a5_3;

  cm3->a5_4 = rfc->a5_bits.a5_4;
  cm3->a5_5 = rfc->a5_bits.a5_5;
  cm3->a5_6 = rfc->a5_bits.a5_6;
  cm3->a5_7 = rfc->a5_bits.a5_7;

#ifdef GPRS
  rac->acc_cap.es_ind = rfc->es_ind;
  rac->acc_cap.ps = rfc->ps;
  rac->acc_cap.vbs = rfc->vbs;
  rac->acc_cap.vgcs = rfc->vgcs;
  rac->acc_cap.v_a5_bits = SUPPORTED;
  rac->acc_cap.a5_bits = rfc->a5_bits;
#if !defined(RAC_RELEASE97)
  rac->acc_cap.compact = rfc->compact;
#endif  /* !RAC_RELEASE97 */

  /* single- or multiband */
  rac->flag_ra_cap2 = rac->v_ra_cap2 = NO;

#endif

  switch (std)
  {
    case STD_900:
      cm3->mb_value = MB_GSM900;
#ifdef GPRS
      rac->acc_tech_typ = ACC_GSM_P;
#endif
      break;
    case STD_EGSM:
      cm2->egsm = SUPPORTED;
      cm3->mb_value = MB_EGSM;
#ifdef GPRS
      rac->acc_tech_typ = ACC_GSM_E;
#endif
      break;
    case STD_1800:
      cm3->mb_value = MB_DCS1800;
#ifdef GPRS
      rac->acc_tech_typ = ACC_GSM_1800;
#endif
      break;
    case STD_DUAL:
      cm3->mb_value = MB_DUAL;
#ifdef GPRS
      rac->acc_tech_typ = ACC_GSM_P;
      rac->flag_ra_cap2 = rac->v_ra_cap2 = YES;
      rac->ra_cap2.acc_tech_typ = ACC_GSM_1800;
      rac->ra_cap2.acc_cap = rac->acc_cap;
      rac->ra_cap2.acc_cap.v_a5_bits = NOT_SUPPORTED;
      rac->ra_cap2.acc_cap.v_ms_struct = NOT_SUPPORTED;
#if !defined(RAC_RELEASE97)
      rac->ra_cap2.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif  /* !RAC_RELEASE97 */
#endif
      break;
    case STD_DUAL_EGSM:
      /* note: incase of 1800 band, the FC bit shall be set to 0 */ 
      if ((rr_data->ms_data.rr_service EQ FULL_SERVICE) AND 
          (!INRANGE(LOW_CHANNEL_1800, rr_data->nc_data[SC_INDEX].arfcn,
		    HIGH_CHANNEL_1800)))
        cm2->egsm = SUPPORTED;
      cm3->mb_value = MB_DUAL_EXT;
#ifdef GPRS
      rac->acc_tech_typ = ACC_GSM_E;
      rac->flag_ra_cap2 = rac->v_ra_cap2 = YES;
      rac->ra_cap2.acc_tech_typ = ACC_GSM_1800;
      rac->ra_cap2.acc_cap = rac->acc_cap;
      rac->ra_cap2.acc_cap.v_a5_bits = NOT_SUPPORTED;
      rac->ra_cap2.acc_cap.v_ms_struct = NOT_SUPPORTED;
#if !defined(RAC_RELEASE97)
      rac->ra_cap2.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif  /* !RAC_RELEASE97 */
#endif
     break;

    case STD_1900:
    case STD_850:
    case STD_DUAL_US:
      /* no break; go through */
    default:
      cm3->mb_value = MB_NO_EUROPEAN_BAND;
      break;
  }

  /* power classes */
  if (rfc->bands & 0x80)
  {
    TRACE_ERROR("R-GSM not supported by PS software");
    /* note: cm3->v_rgsm_class (and cm3->rgsm_class) are hard coded to 0 */
  }
  if (rfc->bands & 0x60)
  {
    TRACE_ERROR("GSM 400 not supported by PS software");
    /* note: cm3->v_gsm400_struct (and cm3->gsm400_struct) are hard coded to 0 */
  }
  switch (std)
  {
    case STD_900:
    case STD_EGSM:
      cm3->v_radio_cap_1 = TRUE;
      cm3->radio_cap_1 = rfc->rf_power.pow_class4[IDX_PWRCLASS_900].pow_class;
      cm2->rf_pow_cap = cm3->radio_cap_1 - 1;
#ifdef GPRS
      rac->acc_cap.pow_class = cm3->radio_cap_1;
#endif
      break;

    case STD_1800:
      /* single band => only radio_cap_1 */
      cm3->v_radio_cap_1 = TRUE;
      cm3->radio_cap_1 = rfc->rf_power.pow_class4[IDX_PWRCLASS_1800].pow_class;
      cm2->rf_pow_cap = cm3->radio_cap_1 - 1;
#ifdef GPRS
      rac->acc_cap.pow_class = cm3->radio_cap_1;
#endif
      break;

    case STD_DUAL:
    case STD_DUAL_EGSM:
      cm3->v_radio_cap_1 = TRUE;
      cm3->radio_cap_1 = rfc->rf_power.pow_class4[IDX_PWRCLASS_900].pow_class;
      cm3->v_radio_cap_2 = TRUE;
      cm3->radio_cap_2 = rfc->rf_power.pow_class4[IDX_PWRCLASS_1800].pow_class;
      /* note: in case the sc is´nt set, it is out of range */
      if (INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
        cm2->rf_pow_cap = cm3->radio_cap_2 - 1;
      else
        cm2->rf_pow_cap = cm3->radio_cap_1 - 1;
#ifdef GPRS
      rac->acc_cap.pow_class = cm3->radio_cap_1;
      rac->ra_cap2.acc_cap.pow_class = cm3->radio_cap_2;
#endif
      break;

    case STD_1900:
      cm3->v_pcs1900_cap = SUPPORTED;
      cm3->pcs1900_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_1900].pow_class;
      cm2->rf_pow_cap = cm3->pcs1900_cap - 1;
#ifdef GPRS
      rac->acc_tech_typ = ACC_GSM_1900;
      rac->acc_cap.pow_class = cm3->pcs1900_cap;
#endif
      break;

    case STD_850:
      cm3->v_gsm850_cap = SUPPORTED;
      cm3->gsm850_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_850].pow_class;
      cm2->rf_pow_cap = cm3->gsm850_cap - 1;
#ifdef GPRS
      rac->acc_tech_typ = ACC_GSM_850;
      rac->acc_cap.pow_class = cm3->gsm850_cap;
#endif
      break;

    case STD_DUAL_US:
      cm3->v_gsm850_cap = SUPPORTED;
      cm3->gsm850_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_850].pow_class;
      cm3->v_pcs1900_cap = SUPPORTED;
      cm3->pcs1900_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_1900].pow_class;
      /* note: in case the sc isn't set, it is out of range */
      if (INRANGE(LOW_CHANNEL_1900,serving_cell,HIGH_CHANNEL_1900))
        cm2->rf_pow_cap = cm3->pcs1900_cap - 1;
      else
        cm2->rf_pow_cap = cm3->gsm850_cap - 1;
#ifdef GPRS
      rac->acc_tech_typ = ACC_GSM_850;
      rac->acc_cap.pow_class = cm3->gsm850_cap;
      rac->flag_ra_cap2 = rac->v_ra_cap2 = YES;
      rac->ra_cap2.acc_tech_typ = ACC_GSM_1900;
      rac->ra_cap2.acc_cap = rac->acc_cap;
      rac->ra_cap2.acc_cap.v_a5_bits = NOT_SUPPORTED;
      rac->ra_cap2.acc_cap.v_ms_struct = NOT_SUPPORTED;
#if !defined(RAC_RELEASE97)
      rac->ra_cap2.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif  /* !RAC_RELEASE97 */
      rac->ra_cap2.acc_cap.pow_class = cm3->pcs1900_cap;
#endif
      break;

    default:
      break;
  }
  cm1->rf_pow_cap = cm2->rf_pow_cap;

#if defined(_SIMULATION_) && defined(RFCAP_TEST)
  rr_csf_trace_power ();
#endif  /* RFCAP_TEST*/

#if defined(FF_EGDE)
  if (rfc->mod OR rfc->rf_power.egde_pow1 OR rfc->rf_power.egde_pow2)
  {
    cm3->v_egde_struct = SUPPORTED;
    cm3->egde_struct.mod = rfc->mod;
  }
  switch (std)
  {
    case STD_900:
    case STD_EGSM:
    case STD_850:
      if (rfc->rf_power.egde_pow1)
      {
        if (rfc->mod)
        {
          cm3->egde_struct.v_egde_pow1 = SUPPORTED;
          cm3->egde_struct.egde_pow1 = rfc->rf_power.egde_pow1;
        }
#ifdef GPRS
        rac->acc_cap.v_pow_8psk_cap = SUPPORTED;
        rac->acc_cap.pow_8psk_cap = rfc->rf_power.egde_pow1;
#endif
      }
      break;
    case STD_1800:
    case STD_1900:
      if (rfc->rf_power.egde_pow2)
      {
        if (rfc->mod)
        {
          cm3->egde_struct.v_egde_pow2 = SUPPORTED;
          cm3->egde_struct.egde_pow2 = rfc->rf_power.egde_pow2;
        }
#ifdef GPRS
        rac->acc_cap.v_pow_8psk_cap = SUPPORTED;
        rac->acc_cap.pow_8psk_cap = rfc->rf_power.egde_pow2;
#endif
      }
      break;
    case STD_DUAL:
    case STD_DUAL_EGSM:
    case STD_DUAL_US:
      if (rfc->rf_power.egde_pow1)
      {
        if (rfc->mod)
        {
          cm3->egde_struct.v_egde_pow1 = SUPPORTED;
          cm3->egde_struct.egde_pow1 = rfc->rf_power.egde_pow1;
        }
#ifdef GPRS
        rac->acc_cap.v_pow_8psk_cap = SUPPORTED;
        rac->acc_cap.pow_8psk_cap = rfc->rf_power.egde_pow1;
#endif
      }
      if (rfc->rf_power.egde_pow2)
      {
        if (rfc->mod)
        {
          cm3->egde_struct.v_egde_pow2 = SUPPORTED;
          cm3->egde_struct.egde_pow2 = rfc->rf_power.egde_pow2;
        }
#ifdef GPRS
        rac->ra_cap2.acc_cap.v_pow_8psk_cap = SUPPORTED;
        rac->ra_cap2.acc_cap.pow_8psk_cap = rfc->rf_power.egde_pow2;
#endif
      }
      break;
    default:
      break;
  }
#endif  /* FF_EGDE */

  /* multi slot capabilities */
  if (rfc->rf_ms.gsm_ms_class)
  {
    cm3->v_ms_class = SUPPORTED;
    cm3->ms_class = rfc->rf_ms.gsm_ms_class;
  }
#if defined(FF_EGDE)
  if (rfc->rf_ms.edge_ms_class)
  {
    cm3->v_edge_ms_class = SUPPORTED;
    cm3->edge_ms_class = rfc->rf_ms.edge_ms_class;
  }
#endif  /* FF_EGDE */
  if (rfc->rf_ms.dtm_g)
  {
#ifdef GPRS
    rac->acc_cap.v_ms_struct = SUPPORTED;
#endif
    cm3->v_dtm_ms = SUPPORTED;
    cm3->dtm_ms.mac_support =  rfc->mac_support;
    cm3->dtm_ms.dtm_g_ms_class = rfc->rf_ms.dtm_g_ms_class;
#ifdef GPRS
#if !defined(RAC_RELEASE97)
    rac->acc_cap.ms_struct.v_dtm_struct = SUPPORTED;
    rac->acc_cap.ms_struct.dtm_struct.dtm_g_ms_class = rfc->rf_ms.dtm_g_ms_class;
    rac->acc_cap.ms_struct.dtm_struct.mac_support = rfc->mac_support;
#endif  /* !RAC_RELEASE97 */
#endif

#if defined(FF_EGDE)
    if (rfc->rf_ms.dtm_e)
    {
      cm3->dtm_ms.v_dtm_e_ms_class = SUPPORTED;
      cm3->dtm_ms.dtm_e_ms_class = rfc->rf_ms.dtm_e_ms_class;
#ifdef GPRS
#if !defined(RAC_RELEASE97)
      rac->acc_cap.ms_struct.v_dtm_struct = SUPPORTED;
      rac->acc_cap.ms_struct.dtm_struct.v_dtm_e_ms_class = SUPPORTED;
      rac->acc_cap.ms_struct.dtm_struct.dtm_e_ms_class =  rfc->rf_ms.dtm_e_ms_class;
#endif  /* !RAC_RELEASE97 */
#endif
    }
#endif  /* FF_EGDE */
  }

#ifdef GPRS
  if (rfc->rf_ms.hscsd_ms_class)
  {
    rac->acc_cap.v_ms_struct = SUPPORTED;
    rac->acc_cap.ms_struct.v_hscsd_ms_class = SUPPORTED;
    rac->acc_cap.ms_struct.hscsd_ms_class = rfc->rf_ms.hscsd_ms_class;
  }

  if (rfc->rf_ms.gprs_ms_class)
  {
    rac->acc_cap.v_ms_struct = SUPPORTED;
    rac->acc_cap.ms_struct.v_gprs_struct = SUPPORTED;
    rac->acc_cap.ms_struct.gprs_struct.gprs_ms_class = rfc->rf_ms.gprs_ms_class;
    rac->acc_cap.ms_struct.gprs_struct.gprs_eda = rfc->gprs_eda;
  }
#endif
  
  cm3->ext_meas = rfc->ext_meas;

  if (rfc->meas)
  {
#ifdef GPRS
    rac->acc_cap.v_ms_struct = SUPPORTED;
    rac->acc_cap.ms_struct.v_sms_sm_value = SUPPORTED;
    rac->acc_cap.ms_struct.sms_sm_value.sms_val = rfc->sms_val;
    rac->acc_cap.ms_struct.sms_sm_value.sm_val = rfc->sm_val;
#endif

    cm3->v_measurement = SUPPORTED;
    cm3->measurement.sms_val = rfc->sms_val;
    cm3->measurement.sm_val  = rfc->sm_val;
  }

#ifdef GPRS
  if (rfc->rf_ms.ecsd_ms_class)
  {
    rac->acc_cap.v_ms_struct = SUPPORTED;
#if !defined(RAC_RELEASE97)
    rac->acc_cap.ms_struct.v_ecsd_ms_class = SUPPORTED;
    rac->acc_cap.ms_struct.ecsd_ms_class = rfc->rf_ms.ecsd_ms_class;
#endif  /* !RAC_RELEASE97 */
  }
#endif

#ifdef GPRS
  if (rfc->rf_ms.egprs_ms_class)
  {
    rac->acc_cap.v_ms_struct = SUPPORTED;
#if !defined(RAC_RELEASE97)
    rac->acc_cap.ms_struct.v_egprs_struct = SUPPORTED;
    rac->acc_cap.ms_struct.egprs_struct.egprs_ms_class = rfc->rf_ms.egprs_ms_class;
    rac->acc_cap.ms_struct.egprs_struct.egprs_eda = rfc->egprs_eda;
#endif  /* !RAC_RELEASE97 */
  }

  if (rac->v_ra_cap2 AND rac->acc_cap.v_ms_struct)
  {
#if !defined(RAC_RELEASE97)
    rac->ra_cap2.acc_cap.rev99 = PHASE_1;           /* PHASE_1 | RELEASE99 */
    rac->ra_cap2.acc_cap.compact = rfc->compact;
#endif  /* !RAC_RELEASE97 */
    rac->ra_cap2.acc_cap.es_ind = rfc->es_ind;
    rac->ra_cap2.acc_cap.ps = rfc->ps;
    rac->ra_cap2.acc_cap.vbs = rfc->vbs;
    rac->ra_cap2.acc_cap.vgcs = rfc->vgcs;
    /*
     * zero means that the same value for multislot parameters as given
     * in an earlier access capabilities field within this IE apply
     * also here.
     */
    rac->ra_cap2.acc_cap.v_ms_struct = FALSE;
  }
#endif

#if defined FF_EOTD
  if (rfc->assist_eotd OR rfc->based_eotd OR rfc->assist_gps OR rfc->based_gps OR rfc->conv_gps)
#else
  if (rfc->assist_gps OR rfc->based_gps OR rfc->conv_gps)
#endif /* FF_EOTD */
  {
    cm3->v_pos_method = SUPPORTED;
#if defined FF_EOTD
    cm3->pos_method.assist_eotd = rfc->assist_eotd;
    cm3->pos_method.based_eotd  = rfc->based_eotd;
#endif /* FF_EOTD */
    cm3->pos_method.assist_gps  = rfc->assist_gps;
    cm3->pos_method.based_gps   = rfc->based_gps;
    cm3->pos_method.conv_gps    = rfc->conv_gps;
  }

#if defined TI_PS_FF_REL99_AND_ABOVE
  cm3->v_gsm700_cap = NOT_SUPPORTED;
  cm3->umts_tdd_128 = NOT_SUPPORTED;
  cm3->geran_feat_pack_1 = NOT_SUPPORTED;
  cm3->v_ext_dtm_ms = NOT_SUPPORTED;
  cm3->v_high_ms_cap = NOT_SUPPORTED;
  cm3->geran_iu_mod_cap = NOT_SUPPORTED;
  cm3->geran_feat_pack_2 = NOT_SUPPORTED;
  cm3->gmsk_ms_pwr_prof = MS_PWR_PROF0;
  cm3->psk8_ms_pwr_prof = MS_PWR_PROF0;
  cm3->v_t_gsm400_struct = NOT_SUPPORTED;
  cm3->v_t_gsm900_cap = NOT_SUPPORTED;
#ifdef L1_SAIC
  /* To support SAIC release 6 feature */
  cm3->dl_adv_rx_per  = DL_ADVANC_RX_PERF_PHASE1_SUPPORT;
#else
  cm3->dl_adv_rx_per  = DL_ADVANC_RX_PERF_NOT_SUPPORT;
#endif /* L1_SAIC */
  cm3->dtm_enhance_cap = NOT_SUPPORTED;
  cm3->v_dtm_high_ms = NOT_SUPPORTED;
#ifdef FF_REPEATED_SACCH
  cm3->rep_acch_cap = REP_SACCH_DL_FACCH;
#else
  cm3->rep_acch_cap = REP_DL_FACCH;
#endif /* FF_REPEATED_SACCH */
#endif /* TI_PS_FF_REL99_AND_ABOVE */

#if defined(_SIMULATION_) && !defined(NTRACE)
  rr_csf_trace_class1 (cm1);
  rr_csf_trace_class2 (cm2);
  rr_csf_trace_class3 (cm3);
#endif  /* _SIMULATION_ && !NTRACE */
  /*
   * note: cm3->umts_fdd, cm3->umts_tdd, cm3->cdma2000, cm3->v_single_band and
   * cm3->single_band are hard coded to 0 due to the lack of information about
   * these features inside the T_rf_cap structure
   */
  return 0;
}

#else  /* Release 99 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_fit_capability      |
+--------------------------------------------------------------------+

  PURPOSE : fit classmark 2 and 3 and radio access capabilities
            depend on the value of 'std' and the readed values of
            /gsm/com/rfcap file according to R99 specifications

*/

GLOBAL UBYTE rr_csf_fit_capability (void)
{
  GET_INSTANCE_DATA;
  USHORT serving_cell;
  T_rf_cap      *rfc = &rr_data->ms_data.rf_cap;
#ifdef GPRS
  T_ra_cap      *rac = &rr_data->ms_data.ra_cap;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  int i;
#endif
#endif
  T_mob_class_1 *cm1 = &rr_data->ms_data.classmark1;
  T_mob_class_2 *cm2 = &rr_data->ms_data.classmark2;
  T_mob_class_3 *cm3 = &rr_data->ms_data.classmark3;

  TRACE_FUNCTION ("rr_csf_fit_capability()");

  if (rr_data->nc_data[SC_INDEX].bcch_status NEQ EMPTY)
    serving_cell = rr_data->nc_data[SC_INDEX].arfcn;
  else
    serving_cell = (USHORT)-1;

  memset (cm2, 0, sizeof(T_mob_class_2));
  memset (cm3, 0, sizeof(T_mob_class_3));
#ifdef GPRS
  memset (rac, 0, sizeof(T_ra_cap));
#endif


  /* Set clasmark 1 fields */
  cm1->rev_lev = REV_LEV_R99_SUPPORT; /* R99 mobile */
  cm1->es_ind  = rfc->es_ind;
  cm1->a5_1    = !rfc->a5_bits.a5_1;


  /* Set classmark 2 fields */
  cm2->rev_lev    = REV_LEV_R99_SUPPORT;
  cm2->es_ind     = rfc->es_ind;
  cm2->ps         = rfc->ps;
  cm2->ss_screen  = rfc->ss_screen;
  cm2->mt_pp_sms  = rfc->mt_pp_sms;
  cm2->vbs        = rfc->vbs;
  cm2->vgcs       = rfc->vgcs;
  cm2->class3     = SUPPORTED;
  cm2->lcsva      = rfc->lcsva;
  cm2->ucs2_treat = rfc->ucs2_treat;
  cm2->solsa      = rfc->solsa;
  cm2->cmsp       = rfc->cmsp;
  cm2->a5_1       = !rfc->a5_bits.a5_1;
  cm2->a5_2       = rfc->a5_bits.a5_2;
  cm2->a5_3       = rfc->a5_bits.a5_3;


  /* Set classmark 3 fields */
  cm3->a5_4               = rfc->a5_bits.a5_4;
  cm3->a5_5               = rfc->a5_bits.a5_5;
  cm3->a5_6               = rfc->a5_bits.a5_6;
  cm3->a5_7               = rfc->a5_bits.a5_7;
  cm3->ucs2_treat         = rfc->ucs2_treat;
  cm3->ext_meas           = rfc->ext_meas;

  /* Set multi slot capabilities */
  if (rfc->rf_ms.gsm_ms_class)
  {
    cm3->v_ms_class = SUPPORTED;
    cm3->ms_class   = rfc->rf_ms.gsm_ms_class;
  }

  /* Set MS measurement capability */
  if (rfc->meas)
  {
    cm3->v_measurement       = SUPPORTED;
    cm3->measurement.sms_val = rfc->sms_val;
    cm3->measurement.sm_val  = rfc->sm_val;
  }

  /* Set MS positioning method */
#if defined FF_EOTD
  if (rfc->assist_eotd OR rfc->based_eotd OR rfc->assist_gps OR rfc->based_gps OR rfc->conv_gps)
#else
  if (rfc->assist_gps OR rfc->based_gps OR rfc->conv_gps)
#endif /* FF_EOTD */
  {
    cm3->v_pos_method = SUPPORTED;
#if defined FF_EOTD
    cm3->pos_method.assist_eotd = rfc->assist_eotd;
    cm3->pos_method.based_eotd  = rfc->based_eotd;
#endif /* FF_EOTD */
    cm3->pos_method.assist_gps  = rfc->assist_gps;
    cm3->pos_method.based_gps   = rfc->based_gps;
    cm3->pos_method.conv_gps    = rfc->conv_gps;
  }


#ifdef GPRS
  /* Set Radio Access Capability info */
  rac->ra_cap_values.v_acc_cap             = YES;
  rac->ra_cap_values.acc_cap.v_a5_bits = SUPPORTED;

  rac->ra_cap_values.acc_cap.rev99     = RELEASE99;
  rac->ra_cap_values.acc_cap.es_ind    = rfc->es_ind;
  rac->ra_cap_values.acc_cap.ps        = rfc->ps;
  rac->ra_cap_values.acc_cap.vbs       = rfc->vbs;
  rac->ra_cap_values.acc_cap.vgcs      = rfc->vgcs;
  rac->ra_cap_values.acc_cap.a5_bits   = rfc->a5_bits;
  rac->ra_cap_values.acc_cap.compact   = rfc->compact;

  rac->ra_cap_values.acc_cap.v_ms_struct                = SUPPORTED;
  rac->ra_cap_values.acc_cap.ms_struct.v_ecsd_ms_class  = NOT_SUPPORTED;/*CR 637 */
  rac->ra_cap_values.acc_cap.ms_struct.v_hscsd_ms_class = NOT_SUPPORTED;/*CR 637 */
  rac->ra_cap_values.acc_cap.ms_struct.v_sms_sm_value   = NOT_SUPPORTED;/*CR 637 */

  rac->c_ra_cap_r = NO;
  rac->v_ra_cap_r = NO;

#endif

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  /* Set the MS multiband capabilities */
  switch (std)
  {
    case STD_900:
      cm3->mb_value = MB_GSM900;
#ifdef GPRS
      rac->ra_cap_values.acc_tech_typ = ACC_GSM_P;
#endif
      break;
    case STD_EGSM:
      cm2->egsm = SUPPORTED;
#ifdef GPRS
      rac->ra_cap_values.acc_tech_typ = ACC_GSM_E;
#endif
      /* No break */
    case STD_900_1900:
    case STD_850_900_1900:
      cm3->mb_value = MB_EGSM;
      if ((!INRANGE(LOW_CHANNEL_1900,serving_cell,HIGH_CHANNEL_1900))
        AND (!INRANGE(LOW_CHANNEL_850,serving_cell,HIGH_CHANNEL_850)))
      {
        cm2->egsm = SUPPORTED;
      }
      break;
    case STD_1800:
#ifdef GPRS
      rac->ra_cap_values.acc_tech_typ = ACC_GSM_1800;
#endif
      /* NO break */
    case STD_850_1800:
      cm3->mb_value = MB_DCS1800;
      break;
    case STD_DUAL:
      cm3->mb_value = MB_DUAL;
#ifdef GPRS
      rac->v_ra_cap_r = YES;
      rac->c_ra_cap_r++;
      rac->ra_cap_r[0].ra_cap_values.v_acc_cap = YES;

      if (INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_1800;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_P;
      }
      else
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_P;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1800;
      }

      rac->ra_cap_r[0].ra_cap_values.acc_cap = rac->ra_cap_values.acc_cap;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_a5_bits      = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_ms_struct    = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif
      break;
    case STD_DUAL_EGSM:
    case STD_850_900_1800:
      /* note: incase of 1800 band, the FC bit shall be set to 0 */
      if((!INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
        AND (!INRANGE(LOW_CHANNEL_850,serving_cell,HIGH_CHANNEL_850)))
        cm2->egsm = SUPPORTED;
      cm3->mb_value = MB_DUAL_EXT;

#ifdef GPRS
      rac->v_ra_cap_r = YES;
      rac->c_ra_cap_r++;
      rac->ra_cap_r[0].ra_cap_values.v_acc_cap = YES;
      if (INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_1800;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_E;
      }
      else
      {
        if(INRANGE(LOW_CHANNEL_850,serving_cell,HIGH_CHANNEL_850))
        {
          rac->ra_cap_values.acc_tech_typ = ACC_GSM_850;
          rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1800;
        }
        else
        {
          rac->ra_cap_values.acc_tech_typ = ACC_GSM_E;
          rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1800;
        }
      }

      rac->ra_cap_r[0].ra_cap_values.acc_cap = rac->ra_cap_values.acc_cap;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_a5_bits      = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_ms_struct    = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif
     break;

    case STD_1900:
    case STD_850:
    case STD_DUAL_US:
      /* no break; go through */
    default:
      cm3->mb_value = MB_NO_EUROPEAN_BAND;
      break;
  }
#else
  /* Set the MS multiband capabilities */
  switch (std)
  {
    case STD_900:
      cm3->mb_value = MB_GSM900;
#ifdef GPRS
      rac->ra_cap_values.acc_tech_typ = ACC_GSM_P;
#endif
      break;
    case STD_EGSM:
      cm2->egsm = SUPPORTED;
      cm3->mb_value = MB_EGSM;
#ifdef GPRS
      rac->ra_cap_values.acc_tech_typ = ACC_GSM_E;
#endif
      break;
    case STD_1800:
      cm3->mb_value = MB_DCS1800;
#ifdef GPRS
      rac->ra_cap_values.acc_tech_typ = ACC_GSM_1800;
#endif
      break;
    case STD_DUAL:
      cm3->mb_value = MB_DUAL;
#ifdef GPRS
      rac->v_ra_cap_r = YES;
      rac->c_ra_cap_r++;
      rac->ra_cap_r[0].ra_cap_values.v_acc_cap = YES;
      
      if (INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
      {
          rac->ra_cap_values.acc_tech_typ = ACC_GSM_1800;
          rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_P;
      }
      else
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_P;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1800;
      }

      rac->ra_cap_r[0].ra_cap_values.acc_cap = rac->ra_cap_values.acc_cap;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_a5_bits      = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_ms_struct    = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif
      break;
    case STD_DUAL_EGSM:
      /* note: incase of 1800 band, the FC bit shall be set to 0 */ 
      if(!INRANGE(LOW_CHANNEL_1800,rr_data->nc_data[SC_INDEX].arfcn,HIGH_CHANNEL_1800))
      cm2->egsm = SUPPORTED;
      cm3->mb_value = MB_DUAL_EXT;

#ifdef GPRS
      rac->v_ra_cap_r = YES;
      rac->c_ra_cap_r++;
      rac->ra_cap_r[0].ra_cap_values.v_acc_cap = YES;

      if (INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
      {
          rac->ra_cap_values.acc_tech_typ = ACC_GSM_1800;
          rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_E;
      }
      else
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_E;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1800;
      }

      rac->ra_cap_r[0].ra_cap_values.acc_cap = rac->ra_cap_values.acc_cap;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_a5_bits      = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_ms_struct    = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif
     break;

    case STD_1900:
    case STD_850:
    case STD_DUAL_US:
      /* no break; go through */
    default:
      cm3->mb_value = MB_NO_EUROPEAN_BAND;
      break;
  }
#endif

  /* Set MS power classes info */
  if (rfc->bands & 0x80)
  {
    TRACE_ERROR("R-GSM not supported by PS software");
    /* note: cm3->v_rgsm_class (and cm3->rgsm_class) are hard coded to 0 */
  }
  if (rfc->bands & 0x60)
  {
    TRACE_ERROR("GSM 400 not supported by PS software");
    /* note: cm3->v_gsm400_struct (and cm3->gsm400_struct) are hard coded to 0 */
  }

  switch (std)
  {
    case STD_900:
    case STD_EGSM:
      cm3->v_radio_cap_1 = TRUE;
      cm3->radio_cap_1 = rfc->rf_power.pow_class4[IDX_PWRCLASS_900].pow_class;
      cm2->rf_pow_cap = cm3->radio_cap_1 - 1;
#ifdef GPRS
      rac->ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
#endif
      break;

    case STD_1800:
      /* single band => only radio_cap_1 */
      cm3->v_radio_cap_1 = TRUE;
      cm3->radio_cap_1 = rfc->rf_power.pow_class4[IDX_PWRCLASS_1800].pow_class;
      cm2->rf_pow_cap = cm3->radio_cap_1 - 1;
#ifdef GPRS
      rac->ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
#endif
      break;

    case STD_DUAL:
    case STD_DUAL_EGSM:
      cm3->v_radio_cap_1 = TRUE;
      cm3->radio_cap_1 = rfc->rf_power.pow_class4[IDX_PWRCLASS_900].pow_class;
      cm3->v_radio_cap_2 = TRUE;
      cm3->radio_cap_2 = rfc->rf_power.pow_class4[IDX_PWRCLASS_1800].pow_class;
      /* note: in case the sc is´nt set, it is out of range */
      if (INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
        cm2->rf_pow_cap = cm3->radio_cap_2 - 1;
      else
        cm2->rf_pow_cap = cm3->radio_cap_1 - 1;
#ifdef GPRS
      if (rac->ra_cap_values.acc_tech_typ EQ ACC_GSM_1800)
      {
        rac->ra_cap_values.acc_cap.pow_class = cm3->radio_cap_2;
        rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
      }
      else
      {
        rac->ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
        rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->radio_cap_2;
      }
#endif
      break;

    case STD_1900:
      cm3->v_pcs1900_cap = SUPPORTED;
      cm3->pcs1900_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_1900].pow_class;
      cm2->rf_pow_cap = cm3->pcs1900_cap - 1;
#ifdef GPRS
      rac->ra_cap_values.acc_tech_typ = ACC_GSM_1900;
      rac->ra_cap_values.acc_cap.pow_class = cm3->pcs1900_cap;
#endif
      break;

    case STD_850:
      cm3->v_gsm850_cap = SUPPORTED;
      cm3->gsm850_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_850].pow_class;
      cm2->rf_pow_cap = cm3->gsm850_cap - 1;
#ifdef GPRS
      rac->ra_cap_values.acc_tech_typ = ACC_GSM_850;
      rac->ra_cap_values.acc_cap.pow_class = cm3->gsm850_cap;
#endif
      break;

    case STD_DUAL_US:
      cm3->v_gsm850_cap = SUPPORTED;
      cm3->gsm850_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_850].pow_class;
      cm3->v_pcs1900_cap = SUPPORTED;
      cm3->pcs1900_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_1900].pow_class;
      
      /* note: in case the sc isn't set, it is out of range */
      if (INRANGE(LOW_CHANNEL_1900,serving_cell,HIGH_CHANNEL_1900))
        cm2->rf_pow_cap = cm3->pcs1900_cap - 1;
      else
        cm2->rf_pow_cap = cm3->gsm850_cap - 1;

#ifdef GPRS
      if (INRANGE(LOW_CHANNEL_1900,serving_cell,HIGH_CHANNEL_1900))
      {
        cm2->rf_pow_cap = cm3->pcs1900_cap - 1;
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_1900;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_850;
        rac->ra_cap_values.acc_cap.pow_class = cm3->pcs1900_cap;
        rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->gsm850_cap;
      }
      else
      {
        cm2->rf_pow_cap = cm3->gsm850_cap - 1;
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_850;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1900;
        rac->ra_cap_values.acc_cap.pow_class = cm3->gsm850_cap;
        rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->pcs1900_cap;
      }
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_a5_bits      = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_ms_struct    = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif
      break;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    case STD_900_1900:
      cm3->v_radio_cap_1 = TRUE;
      cm3->radio_cap_1 = rfc->rf_power.pow_class4[IDX_PWRCLASS_900].pow_class;
      cm3->v_pcs1900_cap = SUPPORTED;
      cm3->pcs1900_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_1900].pow_class;
      if (INRANGE(LOW_CHANNEL_1900,serving_cell,HIGH_CHANNEL_1900))
        cm2->rf_pow_cap = cm3->pcs1900_cap - 1;
      else
        cm2->rf_pow_cap = cm3->radio_cap_1 - 1;
#ifdef GPRS
      if (INRANGE(LOW_CHANNEL_1900,serving_cell,HIGH_CHANNEL_1900))
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_1900;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_E;
        rac->ra_cap_values.acc_cap.pow_class = cm3->pcs1900_cap;
        rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
      }
      else
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_E;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1900;
        rac->ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
        rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->pcs1900_cap;
      }
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_a5_bits      = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_ms_struct    = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif
      break;

    case STD_850_1800:
      cm3->v_radio_cap_1 = TRUE;
      cm3->radio_cap_1 = rfc->rf_power.pow_class4[IDX_PWRCLASS_1800].pow_class;
      cm3->v_gsm850_cap = SUPPORTED;
      cm3->gsm850_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_850].pow_class;
      if (INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
        cm2->rf_pow_cap = cm3->radio_cap_1 - 1;
      else
        cm2->rf_pow_cap = cm3->gsm850_cap - 1;
#ifdef GPRS
      if (INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_1800;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_850;
        rac->ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
        rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->gsm850_cap;
      }
      else
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_850;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1800;
        rac->ra_cap_values.acc_cap.pow_class = cm3->gsm850_cap;
        rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
      }
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_a5_bits      = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_ms_struct    = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif
      break;

    case STD_850_900_1900:
      cm3->v_gsm850_cap = SUPPORTED;
      cm3->gsm850_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_850].pow_class;
      cm3->v_pcs1900_cap = SUPPORTED;
      cm3->pcs1900_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_1900].pow_class;
      cm3->v_radio_cap_1 = TRUE;
      cm3->radio_cap_1 = rfc->rf_power.pow_class4[IDX_PWRCLASS_900].pow_class;
     /* note: in case the sc is´nt set, it is out of range */
      if (INRANGE(LOW_CHANNEL_1900,serving_cell,HIGH_CHANNEL_1900))
        cm2->rf_pow_cap = cm3->pcs1900_cap - 1;
      else if (INRANGE(LOW_CHANNEL_850,serving_cell,HIGH_CHANNEL_850))
        cm2->rf_pow_cap = cm3->gsm850_cap - 1;
      else
        cm2->rf_pow_cap = cm3->radio_cap_1 - 1;

#ifdef GPRS
      if (INRANGE(LOW_CHANNEL_1900,serving_cell,HIGH_CHANNEL_1900))
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_1900;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_850;
        rac->ra_cap_values.acc_cap.pow_class = cm3->pcs1900_cap;
        rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->gsm850_cap;
      }
      else
      {
        if (INRANGE(LOW_CHANNEL_850,serving_cell,HIGH_CHANNEL_850))
        {
          rac->ra_cap_values.acc_tech_typ = ACC_GSM_850;
          rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1900;
          rac->ra_cap_values.acc_cap.pow_class = cm3->gsm850_cap;
          rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->pcs1900_cap;
        }
        else
        {
          rac->ra_cap_values.acc_tech_typ = ACC_GSM_E;
          rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1900;
          rac->ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
          rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->pcs1900_cap;
        }
      }
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_a5_bits      = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_ms_struct    = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif
      break;
    case STD_850_900_1800:
      cm3->v_radio_cap_1 = TRUE;
      cm3->radio_cap_1 = rfc->rf_power.pow_class4[IDX_PWRCLASS_900].pow_class;
      cm3->v_radio_cap_2 = TRUE;
      cm3->radio_cap_2 = rfc->rf_power.pow_class4[IDX_PWRCLASS_1800].pow_class;
      cm3->v_gsm850_cap = SUPPORTED;
      cm3->gsm850_cap = rfc->rf_power.pow_class4[IDX_PWRCLASS_850].pow_class;
      /* note: in case the sc is´nt set, it is out of range */
      if (INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
        cm2->rf_pow_cap = cm3->radio_cap_2 - 1;
      else if (INRANGE(LOW_CHANNEL_850,serving_cell,HIGH_CHANNEL_850))
        cm2->rf_pow_cap = cm3->gsm850_cap - 1;
      else
        cm2->rf_pow_cap = cm3->radio_cap_1 - 1;
#ifdef GPRS
      if (INRANGE(LOW_CHANNEL_1800,serving_cell,HIGH_CHANNEL_1800))
      {
        rac->ra_cap_values.acc_tech_typ = ACC_GSM_1800;
        rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_E;
        rac->ra_cap_values.acc_cap.pow_class = cm3->radio_cap_2;
        rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
      }
      else
      {
        if (INRANGE(LOW_CHANNEL_850,serving_cell,HIGH_CHANNEL_850))
        {
          rac->ra_cap_values.acc_tech_typ = ACC_GSM_850;
          rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1800;
          rac->ra_cap_values.acc_cap.pow_class = cm3->gsm850_cap;
          rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->radio_cap_2;
        }
        else
        {
          rac->ra_cap_values.acc_tech_typ = ACC_GSM_E;
          rac->ra_cap_r[0].ra_cap_values.acc_tech_typ = ACC_GSM_1800;
          rac->ra_cap_values.acc_cap.pow_class = cm3->radio_cap_1;
          rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_class = cm3->radio_cap_2;
        }
      }
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_a5_bits      = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_ms_struct    = NOT_SUPPORTED;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
#endif
      break;
#endif

    default:
      break;
  }

  cm1->rf_pow_cap = cm2->rf_pow_cap;

#if defined(_SIMULATION_) && defined(RFCAP_TEST)
  rr_csf_trace_power ();
#endif  /* RFCAP_TEST*/

/*Set 8psk capability in uplink for EGPRS*/
#if defined(FF_EGPRS)

  /*Initialize with 8psk is not supported in uplink*/
  cm3->v_egde_struct = NOT_SUPPORTED;
  rac->ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;
  rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = NOT_SUPPORTED;

  switch (std)
  {
    case STD_900:
    case STD_EGSM:
    case STD_850:
      if (rfc->mod)
      {
        if (rfc->rf_power.egde_pow1)
        {
          cm3->v_egde_struct = TRUE;
          cm3->egde_struct.mod = SUPPORTED;
          cm3->egde_struct.v_egde_pow1 = TRUE;
          cm3->egde_struct.egde_pow1 = rfc->rf_power.egde_pow1;
          rac->ra_cap_values.acc_cap.v_pow_8psk_cap = TRUE;
          rac->ra_cap_values.acc_cap.pow_8psk_cap = rfc->rf_power.egde_pow1;
          TRACE_EVENT_P1 ("1: 8PSK SUPPORTED (EGPRS) POWER CLASS E1 = %d",rac->ra_cap_values.acc_cap.pow_8psk_cap);
        }
      }
      break;
    case STD_1800:
    case STD_1900:
      if (rfc->mod)
      {
        if (rfc->rf_power.egde_pow2)
        {
          cm3->v_egde_struct = TRUE;
          cm3->egde_struct.mod = SUPPORTED;
          cm3->egde_struct.v_egde_pow2 = TRUE;
          cm3->egde_struct.egde_pow2 = rfc->rf_power.egde_pow2;
          rac->ra_cap_values.acc_cap.v_pow_8psk_cap = TRUE;
          rac->ra_cap_values.acc_cap.pow_8psk_cap = rfc->rf_power.egde_pow2;
          TRACE_EVENT_P1 ("2: 8PSK SUPPORTED (EGPRS) POWER CLASS E2 = %d",rac->ra_cap_values.acc_cap.pow_8psk_cap);
        }
      }
      break;
    case STD_DUAL:
    case STD_DUAL_EGSM:
    case STD_DUAL_US:
      if ((rac->ra_cap_values.acc_tech_typ EQ ACC_GSM_1800) OR (rac->ra_cap_values.acc_tech_typ EQ ACC_GSM_1900))
      {
        if (rfc->mod)
        {
          if (rfc->rf_power.egde_pow2)
          {
            /*8psk is supported in uplink in 1800/1900 BAND*/
            cm3->v_egde_struct = TRUE;
            cm3->egde_struct.mod = SUPPORTED;
            cm3->egde_struct.v_egde_pow2 = TRUE;
            cm3->egde_struct.egde_pow2 = rfc->rf_power.egde_pow2;
            rac->ra_cap_values.acc_cap.v_pow_8psk_cap = TRUE;
            rac->ra_cap_values.acc_cap.pow_8psk_cap = rfc->rf_power.egde_pow2;
            TRACE_EVENT_P1 ("3: 8PSK SUPPORTED (EGPRS) POWER CLASS E2 = %d",rac->ra_cap_values.acc_cap.pow_8psk_cap);
          }
          if (rfc->rf_power.egde_pow1)
          {
            /*Set 8psk capability for 850/900 BAND*/
            cm3->v_egde_struct = TRUE;
            cm3->egde_struct.mod = SUPPORTED;
            cm3->egde_struct.v_egde_pow1 = TRUE;
            cm3->egde_struct.egde_pow1 = rfc->rf_power.egde_pow1;
            rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = TRUE;
            rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_8psk_cap = rfc->rf_power.egde_pow1;
            TRACE_EVENT_P1 ("4: 8PSK SUPPORTED (EGPRS) POWER CLASS E1 = %d",rac->ra_cap_values.acc_cap.pow_8psk_cap);
          }
        }
      }
      else
      {
        if (rfc->mod)
        {
          if (rfc->rf_power.egde_pow1)
          {
            /*8psk is supported in uplink in 1800/1900*/
            cm3->v_egde_struct = TRUE;
            cm3->egde_struct.mod = SUPPORTED;
            cm3->egde_struct.v_egde_pow1 = TRUE;
            cm3->egde_struct.egde_pow1 = rfc->rf_power.egde_pow1;
            rac->ra_cap_values.acc_cap.v_pow_8psk_cap = TRUE;
            rac->ra_cap_values.acc_cap.pow_8psk_cap = rfc->rf_power.egde_pow1;
            TRACE_EVENT_P1 ("5: 8PSK SUPPORTED (EGPRS) POWER CLASS E1 = %d",rac->ra_cap_values.acc_cap.pow_8psk_cap);
          }
          if (rfc->rf_power.egde_pow2)
          {
            /*Set 8psk capability for 850/900*/
            cm3->v_egde_struct = TRUE;
            cm3->egde_struct.mod = SUPPORTED;
            cm3->egde_struct.v_egde_pow2 = TRUE;
            cm3->egde_struct.egde_pow2 = rfc->rf_power.egde_pow2;
            rac->ra_cap_r[0].ra_cap_values.acc_cap.v_pow_8psk_cap = TRUE;
            rac->ra_cap_r[0].ra_cap_values.acc_cap.pow_8psk_cap = rfc->rf_power.egde_pow2;
            TRACE_EVENT_P1 ("6: 8PSK SUPPORTED (EGPRS) POWER CLASS E2 = %d",rac->ra_cap_values.acc_cap.pow_8psk_cap);
          }
        }
      }
      break;
    default:
      break;
  }
#endif  /* FF_EGPRS */

  if (rfc->rf_ms.dtm_g)
  {
    cm3->v_dtm_ms = SUPPORTED;
    cm3->dtm_ms.mac_support =  rfc->mac_support;
    cm3->dtm_ms.dtm_g_ms_class = rfc->rf_ms.dtm_g_ms_class;
#ifdef GPRS
    rac->ra_cap_values.acc_cap.ms_struct.v_dtm_struct = SUPPORTED;
    rac->ra_cap_values.acc_cap.ms_struct.dtm_struct.dtm_g_ms_class = rfc->rf_ms.dtm_g_ms_class;
    rac->ra_cap_values.acc_cap.ms_struct.dtm_struct.mac_support = rfc->mac_support;
#endif

#if defined(FF_EGPRS)
    if (rfc->rf_ms.dtm_e)
    {
      cm3->dtm_ms.v_dtm_e_ms_class = SUPPORTED;
      cm3->dtm_ms.dtm_e_ms_class = rfc->rf_ms.dtm_e_ms_class;
#ifdef GPRS
      rac->ra_cap_values.acc_cap.ms_struct.v_dtm_struct = SUPPORTED;
      rac->ra_cap_values.acc_cap.ms_struct.dtm_struct.v_dtm_e_ms_class = SUPPORTED;
      rac->ra_cap_values.acc_cap.ms_struct.dtm_struct.dtm_e_ms_class =  rfc->rf_ms.dtm_e_ms_class;
#endif
    }
#endif  /* FF_EGDE */
  }

#ifdef GPRS
  
#if 0  /* CR 637 */
  if (rfc->rf_ms.hscsd_ms_class)
  {
    rac->ra_cap_values.acc_cap.v_ms_struct = SUPPORTED;
    rac->ra_cap_values.acc_cap.ms_struct.v_hscsd_ms_class = SUPPORTED;
    rac->ra_cap_values.acc_cap.ms_struct.hscsd_ms_class = rfc->rf_ms.hscsd_ms_class;
  }
#endif

  if (rfc->rf_ms.gprs_ms_class)
  {
    rac->ra_cap_values.acc_cap.ms_struct.v_gprs_struct = SUPPORTED;
    rac->ra_cap_values.acc_cap.ms_struct.gprs_struct.gprs_ms_class = rfc->rf_ms.gprs_ms_class;
    rac->ra_cap_values.acc_cap.ms_struct.gprs_struct.gprs_eda = rfc->gprs_eda;
  }

  if (rfc->meas)
  {
    rac->ra_cap_values.acc_cap.ms_struct.v_sms_sm_value = SUPPORTED;
    rac->ra_cap_values.acc_cap.ms_struct.sms_sm_value.sms_val = rfc->sms_val;
    rac->ra_cap_values.acc_cap.ms_struct.sms_sm_value.sm_val = rfc->sm_val;
  }

#if 0 /* CR 637 */
  if (rfc->rf_ms.ecsd_ms_class)
  {
    rac->ra_cap_values.acc_cap.v_ms_struct = SUPPORTED;
    rac->ra_cap_values.acc_cap.ms_struct.v_ecsd_ms_class = SUPPORTED;
    rac->ra_cap_values.acc_cap.ms_struct.ecsd_ms_class = rfc->rf_ms.ecsd_ms_class;
  }
#endif

#if defined(FF_EGPRS)
  if (rfc->rf_ms.egprs_ms_class)
  {
    cm3->v_edge_ms_class = TRUE;
    cm3->edge_ms_class = rfc->rf_ms.egprs_ms_class;
    rac->ra_cap_values.acc_cap.ms_struct.v_egprs_struct = TRUE;
    rac->ra_cap_values.acc_cap.ms_struct.egprs_struct.egprs_ms_class = rfc->rf_ms.egprs_ms_class;
    rac->ra_cap_values.acc_cap.ms_struct.egprs_struct.egprs_eda = rfc->egprs_eda;
  }
#endif

  if ((rac->c_ra_cap_r > 0) AND rac->ra_cap_values.acc_cap.v_ms_struct)
  {
    rac->v_ra_cap_r = TRUE;
    rac->ra_cap_r[0].ra_cap_values.acc_cap.rev99 = RELEASE99; 

    rac->ra_cap_r[0].ra_cap_values.acc_cap.compact = rfc->compact;
    rac->ra_cap_r[0].ra_cap_values.acc_cap.es_ind = rfc->es_ind;
    rac->ra_cap_r[0].ra_cap_values.acc_cap.ps = rfc->ps;
    rac->ra_cap_r[0].ra_cap_values.acc_cap.vbs = rfc->vbs;
    rac->ra_cap_r[0].ra_cap_values.acc_cap.vgcs = rfc->vgcs;

#if defined(FF_EGPRS)
    if (rfc->rf_ms.egprs_ms_class)
    {
      rac->ra_cap_r[0].ra_cap_values.acc_cap.ms_struct.v_egprs_struct = TRUE;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.ms_struct.egprs_struct.egprs_ms_class = rfc->rf_ms.egprs_ms_class;
      rac->ra_cap_r[0].ra_cap_values.acc_cap.ms_struct.egprs_struct.egprs_eda = rfc->egprs_eda;
    }
#endif

  }
#endif

#if defined TI_PS_FF_REL99_AND_ABOVE
  /* To support SAIC release 6 feature */
  cm3->v_gsm700_cap = NOT_SUPPORTED;
  cm3->umts_tdd_128 = NOT_SUPPORTED;
  cm3->geran_feat_pack_1 = NOT_SUPPORTED;
  cm3->v_ext_dtm_ms = NOT_SUPPORTED;
  cm3->v_high_ms_cap = NOT_SUPPORTED;
  cm3->geran_iu_mod_cap = NOT_SUPPORTED;
  cm3->geran_feat_pack_2 = NOT_SUPPORTED;
  cm3->gmsk_ms_pwr_prof = MS_PWR_PROF0;
  cm3->psk8_ms_pwr_prof = MS_PWR_PROF0;
  cm3->v_t_gsm400_struct = NOT_SUPPORTED;
  cm3->v_t_gsm900_cap = NOT_SUPPORTED;
  cm3->dtm_enhance_cap = NOT_SUPPORTED;
  cm3->v_dtm_high_ms = NOT_SUPPORTED;
#ifdef L1_SAIC
  /* To support SAIC release 6 feature */
  cm3->dl_adv_rx_per  = DL_ADVANC_RX_PERF_PHASE1_SUPPORT;
#else
  cm3->dl_adv_rx_per  = DL_ADVANC_RX_PERF_NOT_SUPPORT;
#endif /* L1_SAIC */
#ifdef FF_REPEATED_SACCH
  cm3->rep_acch_cap = REP_SACCH_DL_FACCH;
#else
  cm3->rep_acch_cap = REP_DL_FACCH;
#endif /* FF_REPEATED_SACCH */
#endif /* TI_PS_FF_REL99_AND_ABOVE */

#if defined(_SIMULATION_) && !defined(NTRACE)
  rr_csf_trace_class1 (cm1);
  rr_csf_trace_class2 (cm2);
  rr_csf_trace_class3 (cm3);
#endif  /* _SIMULATION_ && !NTRACE */
  /*
   * note: cm3->umts_fdd, cm3->umts_tdd, cm3->cdma2000, cm3->v_single_band and
   * cm3->single_band are hard coded to 0 due to the lack of information about
   * these features inside the T_rf_cap structure
   */
  return 0;
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : RX_Enable                  |
+--------------------------------------------------------------------+

  PURPOSE : This function enables reporting of RX level.

*/
static T_VOID_FUNC rx_driver_entry = NULL;
static UBYTE       rx_act_value    = 0;
static UBYTE       rx_act_qual     = RX_QUAL_UNAVAILABLE;
#ifdef FF_PS_RSSI
static UBYTE       rx_act_rxlev_access_min = RX_ACCE_UNAVAILABLE;
#endif

/*lint -esym(765,RX_Enable) | used by RX driver */
/*lint -esym(714,RX_Enable) | used by RX driver */
GLOBAL void RX_Enable (T_VOID_FUNC rx_indication)
{
  /*
   *   store callback function
   */
  rx_driver_entry = rx_indication;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : RX_SetValue                |
+--------------------------------------------------------------------+

  PURPOSE : This function stores a new fieldstrength value and
            informs the RX driver if possible about a change.

*/

/*
 * for reduce number of indications during dedicated mode
 */
static UBYTE rssi_dedi_counter = 0;

/*
 * Minimum RSSI difference for indication to MMI
 */
#define RSSI_MIN_DIFFERENCE  5
#define RSSI_MIN_LEVEL       10


#ifdef FF_PS_RSSI
GLOBAL void RX_SetValue (UBYTE new_value, UBYTE new_qual, UBYTE new_access)
#else
GLOBAL void RX_SetValue (UBYTE new_value)
#endif
{
  GET_INSTANCE_DATA;
  UBYTE rssi_delta;

#ifdef FF_PS_RSSI
  rx_act_rxlev_access_min = new_access;
#endif

  if (rx_driver_entry EQ NULL)
  {
    rx_act_value  = new_value;
#ifdef FF_PS_RSSI
    rx_act_qual   = new_qual;
#endif
  }
  else /*if a callback is installed*/
  {

    /*
     * Compute delta compared to last MMI value
     */
    rssi_delta = ( new_value >= rx_act_value ?
                   ( new_value - rx_act_value ) :
                   ( rx_act_value - new_value )
                 );
      /*
       * if difference greather than defined threshold
       * rx_qual is available in dedicated state only
       * Change in RX_QUAL
       */
    if ((rssi_delta >= RSSI_MIN_DIFFERENCE) OR (new_value < RSSI_MIN_LEVEL) 
#ifdef FF_PS_RSSI
        OR (new_qual NEQ  rx_act_qual)
#endif
      )
    {
      if ( GET_STATE (STATE_ATT) NEQ ATT_DEDICATED )
      {
        rx_act_value = new_value;
#ifdef FF_PS_RSSI
        rx_act_qual  = new_qual;
#endif
        (*rx_driver_entry)();
      }
      else
      {
        /*
         * in dedicated mode take only each 16 the attempt
         * equal 8 seconds
         */
        if (!((rssi_dedi_counter ++ ) & 0x0F ))
        {
          /*
           * New RSSI to be forwarded
           */
          rx_act_value = new_value;
#ifdef FF_PS_RSSI
          rx_act_qual  = new_qual;
#endif
          (*rx_driver_entry)();
        }
      } 
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : RX_GetValue                |
+--------------------------------------------------------------------+

  PURPOSE : This function simulates a low level driver call to get
            the actual fieldstrength.

*/
/*lint -esym(765,RX_GetValue) | used by RX driver */
/*lint -esym(714,RX_GetValue) | used by RX driver */
GLOBAL USHORT RX_GetValue (void)
{
  USHORT rx_level;

  rx_level = (USHORT)rx_act_value;

  return rx_level;
}

#ifdef FF_PS_RSSI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : RX_GetRxLevAccessMin       |
+--------------------------------------------------------------------+

  PURPOSE : This function simulates a low level driver call to get
            the minimum access level.

*/
GLOBAL UBYTE RX_GetRxLevAccessMin (void)
{
  return rx_act_rxlev_access_min;
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : RX_SetRxQual               |
+--------------------------------------------------------------------+

  PURPOSE : This function simulates a low level driver call to set
            the actual RXQUAL value.

*/
/*lint -esym(765,RX_SetRxQual) | used by GRR */
/*lint -esym(714,RX_SetRxQual) | used by GRR */
GLOBAL void RX_SetRxQual (UBYTE new_rx_qual)
{
  rx_act_qual = new_rx_qual;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : RX_GetRxQual               |
+--------------------------------------------------------------------+

  PURPOSE : This function simulates a low level driver call to get
            the actual RXQUAL value.

*/
/*lint -esym(765,RX_GetRxQual) | used by RX driver */
/*lint -esym(714,RX_GetRxQual) | used by RX driver */
GLOBAL UBYTE RX_GetRxQual (void)
{
  return rx_act_qual;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_get_found_channels      |
+--------------------------------------------------------------------+

  PURPOSE : This function is a little bit dirty solution to get
            the BCCH channel numbers of a PLMN available list.

*/
#if 0
GLOBAL USHORT * rr_get_found_channels (void)
{
  static USHORT found_channels[MAX_PLMN];
  int i;
  for (i=0; i<MAX_PLMN; i++)
    found_channels[i] = rr_data->sc_data.found[i].arfcn;
  return found_channels;
}
#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_get_bcch_channel        |
+--------------------------------------------------------------------+

  PURPOSE : This function is a little bit dirty solution to get
            the BCCH channel numbers of the current PLMN.

*/
#if 0
GLOBAL USHORT rr_get_bcch_channel (void)
{
  return rr_data->nc_data[SC_INDEX].arfcn;
}
#endif

/*
 *  These functions (get_network_meas() & get_bcch_chan_list()) should only
 *  temporary defined here.
 */
/*lint -esym(765,get_network_meas) | used by SIM  */
/*lint -esym(714,get_network_meas) | used by SIM  */
GLOBAL UBYTE get_network_meas (UBYTE * chan_list)
{
  GET_INSTANCE_DATA;
#if defined (_SIMULATION_)
  /*
   * fixed values for the windows simulation
   */
  memset (chan_list, 0, 16);
  chan_list [0] = 0x32;
  chan_list [1] = 0x16;
  chan_list [2] = 0x7E;
  chan_list [3] = 0x52;
  chan_list [4] = 0x37;
  return 1;
#else /* _SIMULATION_ */
  switch (GET_STATE (STATE_ATT))
  {
    case ATT_IDLE:
      memset (chan_list, 0, 16);
      chan_list [0] =  rr_data->nc_data[SC_INDEX].rxlev;
      return 1;
    case ATT_DEDICATED:
      memcpy (chan_list, dl_get_sacch_buffer(), 16);
      return 1;
    default:
      return 0;
  }
#endif /* _SIMULATION_ */
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : GPRS EDGE          MODULE  : RR_CSF                      |
| STATE   : code               ROUTINE : get_msc_release_version           |
+--------------------------------------------------------------------+

  PURPOSE : use to get the MSC release version of the serving cell.
*/

GLOBAL void get_msc_release_version(UBYTE* mscr)
{
  GET_INSTANCE_DATA;
  T_NC_DATA * rrd;
  rrd = &rr_data->nc_data[SC_INDEX];
  *mscr = rrd->mscr_flag;
}
#endif


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : get_bcch_chan_list      |
+--------------------------------------------------------------------+

  PURPOSE : This function returns the network measurement results
            in the SIM TOOLKIT Format.

*/
#if defined (_SIMULATION_)
/*
 * predefined values for windows simulation
 */
static USHORT win_list[MAX_NEIGHBOURCELLS] = {  124, 512, 17, 0xFFFF,
                                                0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                                0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                                0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                                0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                                0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                                0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                                0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                                0xFFFF } ;
#endif  /* _SIMULATION_ */
/*lint -esym(765,get_bcch_chan_list) | used by SIM  */
/*lint -esym(714,get_bcch_chan_list) | used by SIM  */
GLOBAL UBYTE get_bcch_chan_list (stk_data_type * out_stk_data)
{
  GET_INSTANCE_DATA;
  UBYTE i;
  UBYTE value;
#if defined (_SIMULATION_)
  USHORT * ncell_list = win_list;
#else  /* _SIMULATION_ */
  USHORT * ncell_list = rr_data->act_ncell_list;
#endif  /* _SIMULATION_ */

#if !defined (_SIMULATION_)
  switch (GET_STATE (STATE_ATT))
  {
    case ATT_IDLE:
    case ATT_DEDICATED:
#endif  /* !_SIMULATION_ */

      /*
       * clear output parameter
       */
      memset (out_stk_data->stk_parameter, 0, sizeof (out_stk_data->stk_parameter));
      for (i=0;i<MAX_NEIGHBOURCELLS;i++)
      {
        if (ncell_list[i] EQ NOT_PRESENT_16BIT)
        {
          /*
           * if no further channel is available
           * calculate length in bytes
           * number of channels a 10 bit
           */
          out_stk_data->stk_length = (i*10+7)/8;
          return 1;
        }
        /*
         * channel is available
         * then use basic CCD function to add to bitstream
         */
        value = ncell_list[i] >> 8;
        ccd_codeByte (out_stk_data->stk_parameter, (USHORT)(i*10),  2, value);
        value = ncell_list[i] & 0xFF;
        ccd_codeByte (out_stk_data->stk_parameter, (USHORT)(i*10+2), 8, value);
      }
      /*
       * if no end indicator has been found
       * and all parameters are channels
       * calculate length in bytes
       * number of channels a 10 bit.
       */
      out_stk_data->stk_length = (i*10+7)/8;
      return 1;
#if !defined (_SIMULATION_)
    default:
      return 0;
  }
#endif  /* _SIMULATION_ */
}


#if defined(_SIMULATION_FFS_)

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_ffs_init            |
+--------------------------------------------------------------------+

  PURPOSE :

*/

LOCAL const char rr_white_list_name[] = "/gsm/l3/rr_white_list";
LOCAL const char rr_black_list_name[] = "/gsm/l3/rr_black_list";
LOCAL const char rr_lower_rxlev_thr_name[] = "/gsm/l3/rr_lower_rxlev_thr";
LOCAL const char rr_medium_rxlev_thr_name[] = "/gsm/l3/rr_medium_rxlev_thr";
LOCAL const char rr_upper_rxlev_thr_name[] = "/gsm/l3/rr_upper_rxlev_thr";
LOCAL const char rr_shield_mcc_name[] = "/gsm/l3/shield";

GLOBAL void rr_csf_ffs_init(void)
{
  TRACE_FUNCTION ("rr_csf_ffs_init()");

  /* If MM has to create the directories for FFS, there is no point
   * in carry on with the initialisation
   */
  rr_csf_check_ffs_dirs();

  /* Read white list from FFS */
  rr_csf_read_white_list();

  /* Read Black List from FFS */
  rr_csf_read_black_list();

  /* Read RXlevel thresholds from FFS */
  rr_csf_read_rxlev_thr();
}


/*
+------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                   |
| STATE   : code                ROUTINE : rr_csf_write_white_list  |
+------------------------------------------------------------------+

  PURPOSE : This function writes the White list information to FFS.
            CSI-LLD - 4.1.2.2.4
*/

GLOBAL void rr_csf_write_white_list(T_CS_WHITE_LIST *white_list)
{
  TRACE_FUNCTION ("rr_csf_write_white_list()");
  
#if defined(_SIMULATION_)
  memcpy(&win_white_list,white_list,sizeof(T_CS_WHITE_LIST));
#else
  rr_csf_check_ffs_dirs();
  rr_csf_handle_ffs_write_result(ffs_file_write(rr_white_list_name,
                                                white_list,
                                                sizeof(T_CS_WHITE_LIST),
                                                FFS_O_CREATE | FFS_O_RDWR));
#endif
 
  TRACE_EVENT("White List written to FFS");
}

/*
+-----------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                  |
| STATE   : code                ROUTINE : rr_csf_read_white_list  |
+-----------------------------------------------------------------+

  PURPOSE : This function read the White list information from FFS.
            CSI-LLD - 4.1.2.2.5
*/

GLOBAL void rr_csf_read_white_list(void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rr_csf_read_white_list()");

  TRACE_EVENT("Read White List from FFS");

#if defined(_SIMULATION_)
  memcpy(&rr_data->cs_data.white_list,&win_white_list,sizeof(T_CS_WHITE_LIST));
#else
  rr_csf_check_ffs_dirs();
  if(!rr_csf_handle_ffs_read_result(ffs_file_read(rr_white_list_name,
                                             &rr_data->cs_data.white_list,
                                             sizeof(T_CS_WHITE_LIST))))
  {
    /* In case of read error, reset the White List */
    cs_clear_white_list(CLR_WHITE_LIST_RAM);
  }
#endif
}

/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                        |
| STATE   : code                ROUTINE : rr_csf_write_black_list       |
+-----------------------------------------------------------------------+

  PURPOSE : This function wirtes "Black List" information to FFS. 
            In case of windows simulation environment, "Black List" is 
            stored to simulated FFS area. 
            This function is called during switch off.
            CSI-LLD section:4.1.1.5.3
*/

GLOBAL void rr_csf_write_black_list(T_LIST *black_list)
{
  TRACE_FUNCTION ("rr_csf_write_black_list()");

#if defined(_SIMULATION_)
  memcpy(&win_black_list[0],black_list,MAX_REGIONS*sizeof(T_LIST));
#else
  rr_csf_check_ffs_dirs();
  rr_csf_handle_ffs_write_result(ffs_file_write (rr_black_list_name,
                                                 black_list,
                                                 MAX_REGIONS*sizeof(T_LIST),
                                                 FFS_O_CREATE | FFS_O_RDWR));
#endif

  TRACE_EVENT("Black List written to FFS");
}


/*
+------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                         |
| STATE   : code                ROUTINE : rr_csf_read_black_list         |
+------------------------------------------------------------------------+

  PURPOSE : This function copies  "Black List" information from FFS to RR
            internal "Black List" data structures. In case of windows 
            simulation environment, "Black List" is read from simulated FFS 
            area. This function is called after power on
            Cell Selection Improvements-LLD section:4.1.1.5.4
*/

GLOBAL void rr_csf_read_black_list(void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rr_csf_read_black_list()");

  TRACE_EVENT("Read Black List from FFS");

#if defined(_SIMULATION_)
  memcpy(&rr_data->cs_data.black_list.list[0],&win_black_list[0],
         MAX_REGIONS*sizeof(T_LIST));
#else
  rr_csf_check_ffs_dirs();
  if(!rr_csf_handle_ffs_read_result(ffs_file_read (rr_black_list_name,
                                               &rr_data->cs_data.black_list.list[0],
                                               MAX_REGIONS*sizeof(T_LIST))))
  {
    /* In case of read error, reset the Black List */
    cs_clear_black_list(CLR_BLACK_LIST_RAM);
  }
#endif
} 

/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                        |
| STATE   : code                ROUTINE : rr_csf_write_rxlev_thr       |
+-----------------------------------------------------------------------+

  PURPOSE : This function wirtes "RXLevel Threshold" information to FFS. 
            In case of windows simulation environment, "RXLevel Threshold" is 
            stored to simulated FFS area. 
            This function is called during dynamic configuration.
*/

GLOBAL void rr_csf_write_rxlev_thr(UBYTE rxt, SHORT valno, char* val[MAX_NUM_BANDS])
{
  GET_INSTANCE_DATA;
  UBYTE i, rxlev_thr_val; 
  UBYTE *rxlev_thr = NULL;
  #if defined(_SIMULATION_)
  UBYTE *win_rxlev_thr = NULL;
  #endif
  const char *rxlev_thr_name, *rxlev_thr_string;
  rxlev_thr_name = NULL;
  rxlev_thr_string = NULL;
  TRACE_FUNCTION ("rr_csf_write_rxlev_thr()");

  switch(rxt)
  {
    case U_RXT: rxlev_thr_string = "Upper";
                rxlev_thr = &rr_data->dyn_config.upper_rxlev_thr[0];
                rxlev_thr_name = &rr_upper_rxlev_thr_name[0];
#if defined(_SIMULATION_)
                win_rxlev_thr = &win_upper_rxlev_thr[0];
#endif
                break;
    case M_RXT: rxlev_thr_string = "Medium";
                rxlev_thr = &rr_data->dyn_config.medium_rxlev_thr[0];
                rxlev_thr_name = &rr_medium_rxlev_thr_name[0];
#if defined(_SIMULATION_)
                win_rxlev_thr = &win_medium_rxlev_thr[0];
#endif
                break;
    case L_RXT: rxlev_thr_string = "Lower";
                rxlev_thr = &rr_data->dyn_config.lower_rxlev_thr[0];
                rxlev_thr_name = &rr_lower_rxlev_thr_name[0];
#if defined(_SIMULATION_)
                win_rxlev_thr = &win_lower_rxlev_thr[0];
#endif
                break;
    default:
      return;
  }
  
  for( i=0 ; i<valno ; i++ )
  {
    rxlev_thr_val = atoi(val[i]);
    if( rxlev_thr_val > 0 AND rxlev_thr_val <= 63 )
    {
        rxlev_thr[i] = rxlev_thr_val;
    }
    else
    {
        TRACE_ERROR ("[PEI_CONFIG]: RxLev Threshold(0-63)-Incorrect Range");
    }
  }
#if defined(_SIMULATION_)
    memcpy(win_rxlev_thr,rxlev_thr,MAX_NUM_BANDS*sizeof(UBYTE));
#else
    rr_csf_check_ffs_dirs();
    rr_csf_handle_ffs_write_result(ffs_file_write(rxlev_thr_name,
                                    rxlev_thr,
                                    MAX_NUM_BANDS*sizeof(UBYTE),
                                    FFS_O_CREATE | FFS_O_RDWR));
#endif
TRACE_EVENT_P6("%s RxLev Threshold written to FFS: GSM 850=%d, PGSM 900=%d, DCS 1800=%d, PCS 1900=%d, EGSM=%d",
    rxlev_thr_string, rxlev_thr[B_GSM_850], rxlev_thr[B_GSM_900],
    rxlev_thr[B_DCS_1800], rxlev_thr[B_PCS_1900], rxlev_thr[B_E_GSM] );

}

/*
+------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                         |
| STATE   : code                ROUTINE : rr_csf_read_rxlev_thr          |
+------------------------------------------------------------------------+

  PURPOSE : This function copies  "RXLevel Threshold" information from FFS to RR
            internal "RXLevel Threshold" arrays. In case of windows 
            simulation environment, "RXLevel Threshold" is read from simulated FFS 
            area. This function is called after power on
*/

GLOBAL void rr_csf_read_rxlev_thr(void)
{

  UBYTE x;
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rr_csf_read_rxlev_thr()");

  TRACE_EVENT("Read RXLevel thresholds from FFS");

#if defined(_SIMULATION_)
  memcpy(rr_data->dyn_config.lower_rxlev_thr,win_lower_rxlev_thr,
         MAX_NUM_BANDS*sizeof(UBYTE));
  memcpy(rr_data->dyn_config.medium_rxlev_thr,win_medium_rxlev_thr,
         MAX_NUM_BANDS*sizeof(UBYTE));
  memcpy(rr_data->dyn_config.upper_rxlev_thr,win_upper_rxlev_thr,
         MAX_NUM_BANDS*sizeof(UBYTE));
#else
  rr_csf_check_ffs_dirs();
  if(!rr_csf_handle_ffs_read_result(ffs_file_read (rr_lower_rxlev_thr_name,
                                               rr_data->dyn_config.lower_rxlev_thr,
                                               MAX_NUM_BANDS*sizeof(UBYTE))))
  {
    for( x=0 ; x<MAX_NUM_BANDS ; x++ )
    {
       rr_data->dyn_config.lower_rxlev_thr[x]  = LOWER_RXLEV_THRESHOLD;
    }
    rr_data->dyn_config.lower_rxlev_thr[B_GSM_850]  = LOWER_RXLEV_THRESHOLD_850;
  }
  if(!rr_csf_handle_ffs_read_result(ffs_file_read (rr_medium_rxlev_thr_name,
                                               rr_data->dyn_config.medium_rxlev_thr,
                                               MAX_NUM_BANDS*sizeof(UBYTE))))
  {
    for( x=0 ; x<MAX_NUM_BANDS ; x++ )
    {
            rr_data->dyn_config.medium_rxlev_thr[x]  = MEDIUM_RXLEV_THRESHOLD;
    }
  }
  if(!rr_csf_handle_ffs_read_result(ffs_file_read (rr_upper_rxlev_thr_name,
                                               rr_data->dyn_config.upper_rxlev_thr,
                                               MAX_NUM_BANDS*sizeof(UBYTE))))
  {
    for( x=0 ; x<MAX_NUM_BANDS ; x++ )
    {
            rr_data->dyn_config.upper_rxlev_thr[x]  = UPPER_RXLEV_THRESHOLD;
    }
  }
#endif
} 

/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                        |
| STATE   : code                ROUTINE : rr_csf_write_mcc_shield_to_ffs|
+-----------------------------------------------------------------------+

  PURPOSE : This function writes the mcc shield values to FFS
*/

GLOBAL void rr_csf_write_mcc_shield_to_ffs(void)
{
  GET_INSTANCE_DATA;
  UBYTE count;
  TRACE_FUNCTION ("rr_csf_write_mcc_shield_to_ffs()");

  rr_csf_check_ffs_dirs();
  rr_csf_handle_ffs_write_result(ffs_file_write (rr_shield_mcc_name,
                                                 &rr_data->dyn_config.mcc_shield,
                                                 sizeof(T_shield_mcc),
                                                 FFS_O_CREATE | FFS_O_RDWR));
  for(count=0;count<MAX_MCC_SHIELD;count++)
  {
    TRACE_EVENT_P5 ( "Shield MCC=%d%d%d enabled=[%d]index=[%d]",
                      rr_data->dyn_config.mcc_shield.mcc[count][0],
                      rr_data->dyn_config.mcc_shield.mcc[count][1],
                      rr_data->dyn_config.mcc_shield.mcc[count][2],
                      rr_data->dyn_config.mcc_shield.enabled,
                      rr_data->dyn_config.mcc_shield.index);
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_CSF                      |
| STATE   : code               ROUTINE : rr_csf_mcc_present                  |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to find mcc already present or not
*/

GLOBAL UBYTE rr_csf_mcc_present(UBYTE* mcc_value)
{
  GET_INSTANCE_DATA;
   U8 count = 0;
   for(count = 0;count<MAX_MCC_SHIELD;count++)
   {
     if((memcmp(rr_data->dyn_config.mcc_shield.mcc[count],mcc_value,SIZE_MCC)EQ 0))
     {
       return TRUE;
     }
   }
   return FALSE;
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                        |
| STATE   : code                ROUTINE : rr_csf_handle_ffs_read_result |
+-----------------------------------------------------------------------+

  PURPOSE : This function checks the status from FFS and returns
            TRUE if there is no error and FALSE if there is
*/
LOCAL BOOL rr_csf_handle_ffs_read_result(T_FFS_SIZE status_read)
{
  if(status_read>0)
  {
    TRACE_EVENT ("FFS Read OK");
    return TRUE;
  }
  else
  {
    TRACE_EVENT_P1("FFS Read ERROR - %x", status_read);
    return FALSE;
  }
}

/*
+------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                         |
| STATE   : code                ROUTINE : rr_csf_handle_ffs_write_result |
+------------------------------------------------------------------------+

  PURPOSE : This function checks the status from FFS and returns
            TRUE if there is no error and FALSE if there is
*/
LOCAL BOOL rr_csf_handle_ffs_write_result(T_FFS_RET status_write)
{
  if(status_write >= EFFS_OK)
  {
    TRACE_EVENT("FFS Write OK");
    if(status_write)
      TRACE_EVENT_P1 ("Bytes written: %d",status_write);

    return(TRUE);
  }
  else
  {
    TRACE_EVENT_P1 ("FFS Write Error - Status: %x",status_write);
    return(FALSE);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_check_ffs_dirs      |
+--------------------------------------------------------------------+

  PURPOSE : This routine creates the /gsm/l3 directory for the last used
            sc arfcn in the FFS. The return result indicates whether either
            directories existed or not. There is no error handling.
*/

LOCAL void rr_csf_check_ffs_dirs( void )
{
  const char gsm_name[] = "/gsm";
  const char gsm_l3_name[] = "/gsm/l3";

  rr_csf_create_ffs_dirs(gsm_name);
  rr_csf_create_ffs_dirs(gsm_l3_name);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_CSF                     |
| STATE   : code                ROUTINE : rr_csf_create_ffs_dirs     |
+--------------------------------------------------------------------+

  PURPOSE : This routine calls the ffs_mkdir() routine to create a
            specified directory. It returns whether the directory
            existed or not prior to creation.
*/
LOCAL void rr_csf_create_ffs_dirs(const char *dir_name)
{
  T_FFS_RET status;

  status=ffs_mkdir(dir_name);
  switch(status)
  {
    case EFFS_EXISTS:
    case EFFS_OK:
      break;

    default: /*Error*/
      TRACE_EVENT_P1("Create Dir error - %x",status);
      break;
  }
}
#endif /* _SIMULATION_FFS_ */

#endif
