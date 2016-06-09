/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  ACI
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
|  Purpose :  This header shall be included in all ACI files.
+----------------------------------------------------------------------------- 
*/ 

#ifdef MFW
#define ENTITY_MFW
#else
#ifdef SMI
#define ENTITY_SMI
#else
#define ENTITY_ACI
#endif
#endif

//TISH modified for MSIM
//#if defined(WIN32) || defined(_SIMULATION_)
//#define FF_TTY
//#endif

#define ACI_MEMBER

/*==== INCLUDES ===================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "typedefs.h"
#include <string.h>
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#ifdef GPRS
#include "gprs.h"  /* to see SNDCP_NAME */
#endif /* GPRS */
#include "prim.h"
#include "message.h"
#include "tok.h"
#include "ccdapi.h"



EXTERN char * getFileName(char * file);

#ifdef _SIMULATION_
#define ACI_ASSERT(exp)   \
  do {                    \
    if(!(exp)) {          \
      int * p = NULL;     \
      TRACE_EVENT_P2("[ERROR] Assertion Failed in %s:%i: "#exp,getFileName(__FILE__),__LINE__); \
      vsi_t_sleep(0,2000);\
      /*lint -e413 */ *p=1; /*lint +e413 */ \
    }                     \
  }while(0)
#else
#define ACI_ASSERT(exp) \
  if(!(exp)) {            \
    TRACE_EVENT_P2("[ERROR] Assertion Failed in %s:%i: "#exp,getFileName(__FILE__),__LINE__); \
  }
#endif
