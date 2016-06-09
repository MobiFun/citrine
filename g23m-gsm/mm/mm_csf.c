/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (8410)
|  Modul   :  MM_CSF
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
|  Purpose :  This Modul defines the functions for the csf
|             capability of the module Mobility Management.
+-----------------------------------------------------------------------------
*/

#ifndef MM_CSF_C
#define MM_CSF_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_MM

/*==== INCLUDES ===================================================*/
#if defined (NEW_FRAME)

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
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
#include "cnf_mm.h"
#include "mon_mm.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"
#include "cl_imei.h"  /* IMEI common library */

#else

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "stddefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_mm.h"
#include "mon_mm.h"
#include "vsi.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"
#include "cl_imei.h"  /* IMEI common library */

#endif

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * Procedures
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_CSF                     |
| STATE   : code                ROUTINE : csf_read_imei              |
+--------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void csf_read_imei (T_imsi_struct *imei_struct)
{
  UBYTE buf[CL_IMEI_SIZE];

  TRACE_FUNCTION ("csf_read_imei()");

  imei_struct->v_mid        = V_MID_PRES;
  imei_struct->id_type      = 2; /*TYPE_IMEI*/
  imei_struct->tmsi_dig     = 0L;

  /*
   * Get IMEISV from IMEI common library
   */
  cl_get_imeisv(CL_IMEI_SIZE, buf, CL_IMEI_GET_SECURE_IMEI);
  imei_struct->id[0]  = (buf [0] >> 4) & 0x0F; /* TAC 8 byte */
  imei_struct->id[1]  =  buf [0] & 0x0F;
  imei_struct->id[2]  = (buf [1] >> 4) & 0x0F;
  imei_struct->id[3]  =  buf [1] & 0x0F;
  imei_struct->id[4]  = (buf [2] >> 4) & 0x0F;
  imei_struct->id[5]  =  buf [2] & 0x0F;
  imei_struct->id[6]  = (buf [3] >> 4) & 0x0F;
  imei_struct->id[7]  =  buf [3] & 0x0F;
  imei_struct->id[8]  = (buf [4] >> 4) & 0x0F; /* SNR 6 byte */
  imei_struct->id[9]  =  buf [4] & 0x0F;
  imei_struct->id[10] = (buf [5] >> 4) & 0x0F;
  imei_struct->id[11] =  buf [5] & 0x0F;
  imei_struct->id[12] = (buf [6] >> 4) & 0x0F;
  imei_struct->id[13] =  buf [6] & 0x0F;
  imei_struct->id[14] = (buf [7] >> 4) & 0x0F; /* SV 2 byte */
  imei_struct->id[15] =  buf [7] & 0x0F;
  TRACE_EVENT_P8("MM INFO IMEI: TAC %1x%1x%1x%1x%1x%1x%1x%1x",
                  imei_struct->id[0], imei_struct->id[1],
                  imei_struct->id[2], imei_struct->id[3],
                  imei_struct->id[4], imei_struct->id[5],
                  imei_struct->id[6], imei_struct->id[7]);
  TRACE_EVENT_P6("MM INFO IMEI: SNR %1x%1x%1x%1x%1x%1x",
                  imei_struct->id[8],  imei_struct->id[9],
                  imei_struct->id[10], imei_struct->id[11],
                  imei_struct->id[12], imei_struct->id[13]);
  TRACE_EVENT_P2("MM INFO IMEI: SV  %1x%1x", imei_struct->id[14],
                                             imei_struct->id[15]);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_CSF                     |
| STATE   : code                ROUTINE : csf_read_mobile_class_1    |
+--------------------------------------------------------------------+

  PURPOSE : Read classmark 1.

*/

GLOBAL void csf_read_mobile_class_1 (T_mob_class_1 *mob_class_1)
{
  /*
   * The classmarks got from RR are valid after RR becomes active.
   * That applies to the case of simulation too, because RR checks the existence
   * and validity of the RF capabilities before filling the structure and read
   * it as necessary.
   */
#if 1
  EXTERN UBYTE rr_csf_get_classmark1 (T_mob_class_1 *mob_class_1);
  rr_csf_get_classmark1 (mob_class_1);
#else
  UBYTE version;
  UBYTE buf[SIZE_EF_CLASS2];
  int   result;

  TRACE_FUNCTION ("csf_read_mobile_class_1()");

  memset (mob_class_1, 0, sizeof (T_mob_class_1));

  result = pcm_ReadFile ((UBYTE *)EF_CLASS2_ID, SIZE_EF_CLASS2, buf, &version);

  TRACE_EVENT_P1 ("Mobile Class 1: %02X", buf[0]);
  TRACE_EVENT_P1 ("Result = %d", result);

  /* mob_class_1->spare_1 = GET_BITS (buf[0], 7, 1); */
  mob_class_1->rev_lev    = GET_BITS (buf[0], 5, 2);
  mob_class_1->es_ind     = GET_BITS (buf[0], 4, 1);
  mob_class_1->a5_1       = GET_BITS (buf[0], 3, 1);
  mob_class_1->rf_pow_cap = GET_BITS (buf[0], 0, 3);
#endif  /* else, #ifndef WIN32 */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_CSF                     |
| STATE   : code                ROUTINE : csf_read_mobile_class_2    |
+--------------------------------------------------------------------+

  PURPOSE : Read classmark 2. Power may not be valid as this is
            delivered by RR after cell selection appropriately for
            the selected band.

*/

GLOBAL void csf_read_mobile_class_2 (T_mob_class_2 *mob_class_2)
{
  /*
   * The classmarks got from RR are valid after RR becomes active.
   * That applies to the case of simulation too, because RR checks the existence
   * and validity of the RF capabilities before filling the structure and read
   * it as necessary.
   */
#if 1
  EXTERN UBYTE rr_csf_get_classmark2 (T_mob_class_2 *mob_class_2);
  rr_csf_get_classmark2 (mob_class_2);
#else
  UBYTE version;
  UBYTE buf[SIZE_EF_CLASS2];
  int   result;

  TRACE_FUNCTION ("csf_read_mobile_class_2()");

  memset (mob_class_2, 0, sizeof (T_mob_class_2));

  result = pcm_ReadFile ((UBYTE *)EF_CLASS2_ID, SIZE_EF_CLASS2, buf, &version);

  TRACE_EVENT_P3 ("Mobile Class 2: %02X %02X %02X", buf[0], buf[1], buf[2]);
  TRACE_EVENT_P1 ("Result = %d", result);

  /* mob_class_2->spare_1 = GET_BITS (buf[0], 7, 1); */
  mob_class_2->rev_lev    = GET_BITS (buf[0], 5, 2);
  mob_class_2->es_ind     = GET_BITS (buf[0], 4, 1);
  mob_class_2->a5_1       = GET_BITS (buf[0], 3, 1);
  mob_class_2->rf_pow_cap = GET_BITS (buf[0], 0, 3);

  /* mob_class_2->spare_2 = GET_BITS (buf[1], 7, 1); */
  mob_class_2->ps         = GET_BITS (buf[1], 6, 1);
  mob_class_2->ss_screen  = GET_BITS (buf[1], 4, 2);
  mob_class_2->mt_pp_sms  = GET_BITS (buf[1], 3, 1);
  mob_class_2->vbs        = GET_BITS (buf[1], 2, 1);
  mob_class_2->vgcs       = GET_BITS (buf[1], 1, 1);
  mob_class_2->egsm       = GET_BITS (buf[1], 0, 1);

  /* mob_class_2->class3  = GET_BITS (buf[2], 7, 1); */
  mob_class_2->class3     = SUPPORTED;
  /* mob_class_2->spare_3 = GET_BITS (buf[2], 6, 1); */
  mob_class_2->lcsva      = GET_BITS (buf[2], 5, 1);
  mob_class_2->ucs2_treat = GET_BITS (buf[2], 4, 1);
  mob_class_2->solsa      = GET_BITS (buf[2], 3, 1);
  mob_class_2->cmsp       = GET_BITS (buf[2], 2, 1);
  mob_class_2->a5_3       = GET_BITS (buf[2], 1, 1);
  mob_class_2->a5_2       = GET_BITS (buf[2], 0, 1);
#endif  /* else, #ifndef WIN32 */
}

/* N950 Memory Optimization - Implements Measure #39*/
/* Removed unused Function*/
#endif
