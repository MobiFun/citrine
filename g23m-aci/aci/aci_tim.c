/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_TIM
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
|  Purpose :  This Modul defines the timer handling functions
|             for the AT Command Interpreter
+----------------------------------------------------------------------------- 
*/ 

#ifndef ACI_TIM_C
#define ACI_TIM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif

#include "aci.h"
#include "aoc.h"
#include "psa.h"
#include "cmh.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_TIM            |
| STATE   : code                        ROUTINE : tim_exec_timeout   |
+--------------------------------------------------------------------+

  PURPOSE : execute timeout function depending on the timer

*/
GLOBAL void tim_exec_timeout (USHORT index)
{
#if defined SMI
    /*
     * Check SMI Timer
     */
    if (smi_timeout (index))
      return;
#endif

#if defined MFW
    /*
     * Check MFW Timer
     */
    if (mfw_timeout (index))
      return;
#endif

    /*
     * check Advice of Charge Timer
     */
   if (aoc_timeout (index))
     return;

#ifdef FF_ATI
    /*
     * check Command Interpreter Timer
     */
    if (aci_timeout (index))
      return;
#endif

    /*
     * check Command Handler Timer
     */
    if (cmh_timeout (index))
      return;

    /*
     * check Command Handler Timer
     */
    if (psa_timeout (index))
      return;
}

#endif

