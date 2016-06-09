/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  ALR_MMI
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
|  Purpose :  This Modul defines the functions for the SDL process
|             MMI_Control of the TI++ functionality.
+-----------------------------------------------------------------------------
*/

#ifndef ALR_MMI_C
#define ALR_MMI_C

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

#include "alr.h"

#if defined TM_SPECIAL

#include "gdi.h"
#include "audio.h"

#endif

/*==== EXPORT =====================================================*/
#if defined (WIN32)
#define TRACING
#endif

#if defined (TRACING)
#define ALR_TRACE_MMI(a)  ALR_TRACE(a)
#else
#define ALR_TRACE_MMI(a)
#endif

/*==== PRIVAT =====================================================*/

/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * PRIMITIVE Processing functions
 * -------------------------------------------------------------------
 */

#endif
