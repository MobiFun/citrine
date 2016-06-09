/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_UDPAR
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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_UDPAR_C
#define CMH_UDPAR_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"

#ifdef FAX_AND_DATA
  #include "aci_fd.h"
#endif

#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_io.h"

#include "psa.h"
#include "psa_ra.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_ra.h"

#include "wap_aci.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/
EXTERN SHORT cmhCC_UDPA_Configurated (void);
EXTERN SHORT cmhCC_UDPA_Deconfigurated (void);

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UDPAR               |
|                                 ROUTINE : cmhUDPA_Configurated    |
+-------------------------------------------------------------------+

  PURPOSE : UDPA configurated 
*/

GLOBAL T_ACI_RETURN cmhUDPA_Configurated ( void )
{
  TRACE_FUNCTION ("cmhUDPA_Configurated()");
 
  cmhCC_UDPA_Configurated();
  
  return (AT_CMPL);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UDPAR               |
|                                 ROUTINE : cmhUDPA_Configurated    |
+-------------------------------------------------------------------+

  PURPOSE : UDPA deconfigurated 
*/

GLOBAL T_ACI_RETURN cmhUDPA_Deconfigurated ( void )
{
  TRACE_FUNCTION ("cmhUDPA_Deconfigurated()");
 
  cmhCC_UDPA_Deconfigurated();
  
  return (AT_CMPL);
}
