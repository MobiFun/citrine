/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  alr_CSF
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
|  Purpose :  This Modul defines the customer specific functions.
+-----------------------------------------------------------------------------
*/

#ifndef ALR_CSF_C
#define ALR_CSF_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_PL

/*==== INCLUDES ===================================================*/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_alr.h"
#include "mon_alr.h"
#include "pei.h"
#include "tok.h"

#include "pcm.h"
#ifdef GPRS
#include "alr_gprs.h"
#endif

/*
 * FreeCalypso note: I left the crud below untouched because it produces
 * no effect other than some trace output containing no useful info,
 * and leaving the code alone results in this "functionality" being
 * disabled because we don't define _TMS470.
 */

#ifdef _TMS470
#include "inth/iq.h"
#include "armio/armio.h"
#endif  /* _TMS470 */

#include "alr.h"


/*==== IMPORT =====================================================*/

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : alr_CSF                    |
| STATE   : code                ROUTINE : csf_show_version           |
+--------------------------------------------------------------------+

  PURPOSE : Trace Layer 1 DSP version numbers

*/

GLOBAL void csf_show_version (T_TST_TEST_HW_CON  *ver)
{
#if defined (_TMS470)
  USHORT build, hw, jtag, rev;

  /*
   * Retrieve hardware info and build from library
   */
  hw = 0;

  build = IQ_GetBuild();
  jtag  = IQ_GetJtagId();
  rev   = IQ_GetRevision();

  TRACE_EVENT_P4 ("Build=%04d HW ver=%04X, jtag=%04X, rev=%04X", build, hw, jtag, rev);

  if (ver)
  {
    /*
     * Retrieve Layer 1 info from primitive
     */
    TRACE_EVENT_P3 ("DSP version/patch=%04X, %04x, chksum=%04X",
                    ver->dsp_code_version,
                    ver->dsp_patch_version,
                    ver->dsp_checksum);
    TRACE_EVENT_P3 ("MCU version ALR=%04X, GPRS=%04X, TM=%04X",
                    ver->mcu_alr_version,
                    ver->mcu_gprs_version,
                    ver->mcu_tm_version);
    PFREE (ver);
  }
#else  /* _TMS470  */
  if (ver)
  {
    PFREE (ver);
  }
#endif  /* _TMS470 */
}

#if !defined NTRACE

/*
 * The following monitor capabilities are used by the Condat RT system.
 * They are only included in the trace version of protocol stack.

+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : alr_CSF                    |
| STATE   : code                ROUTINE : trc_mon_counter_idle       |
+--------------------------------------------------------------------+

  PURPOSE : traces the downlink counter values in idle mode if a change
            occur.

*/

UBYTE v_mon_counter_idle = 0;

GLOBAL void trc_mon_counter_idle (UBYTE act_dlt, UBYTE max_dlt)
{
  if (v_mon_counter_idle)
  {
    /*
     * only if monitoring is enabled
     */
    PALLOC (mon_counter_idle, MON_COUNTER_IDLE_IND);

    mon_counter_idle->act_dlt = act_dlt;
    mon_counter_idle->max_dlt = max_dlt;

    PSENDX(RR, mon_counter_idle);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : alr_CSF                    |
| STATE   : code                ROUTINE : trc_mon_counter_dedi       |
+--------------------------------------------------------------------+

  PURPOSE : traces the radiolink counter values in dedicated mode
            if a change occur.

*/

UBYTE v_mon_counter_dedi = 0;

GLOBAL void trc_mon_counter_dedi (UBYTE act_rlt, UBYTE max_rlt)
{
  if (v_mon_counter_dedi)
  {
    /*
     * only if monitoring is enabled
     */
    PALLOC (mon_counter_dedi, MON_COUNTER_DEDI_IND);

    mon_counter_dedi->act_rlt = act_rlt;
    mon_counter_dedi->max_rlt = max_rlt;

    PSENDX(RR, mon_counter_dedi);
  }
}

#endif /* !NTRACE */

#endif
