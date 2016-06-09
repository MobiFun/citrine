/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_PPPR
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
#if defined (FF_WAP) || defined (FF_PPP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
/* Only for APP's over UDP/IP or TCP/IP or RNET */


#ifndef CMH_PPPR_C
#define CMH_PPPR_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
  #include "aci_fd.h"
#endif

#include "aci_io.h"

#include "psa.h"
#include "psa_ra.h"
#include "psa_cc.h"

#include "cmh.h"

#include "cmh_ra.h"

#include "wap_aci.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/
EXTERN SHORT cmhCC_PPP_Established (ULONG ip_address, USHORT max_receive_unit,
                                    ULONG dns1, ULONG dns2) ;
EXTERN SHORT cmhCC_PPP_Terminated ( void );

EXTERN T_ACI_WAP_STATES wap_state;
EXTERN SHORT		        wapId;

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_IPAS                |
|                                 ROUTINE : cmhPPP_Established      |
+-------------------------------------------------------------------+

  PURPOSE : PPP Established 
*/

GLOBAL T_ACI_RETURN cmhPPP_Established (ULONG ip, USHORT mru,
                                        ULONG dns1, ULONG dns2)
{
  TRACE_FUNCTION ("cmhPPP_established()");
  
  cmhCC_PPP_Established(ip, mru, dns1, dns2) ;
  return (AT_CMPL);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_IPAS                |
|                                 ROUTINE : cmhPPP_Terminated       |
+-------------------------------------------------------------------+

  PURPOSE : PPP Terminated 
*/

GLOBAL T_ACI_RETURN cmhPPP_Terminated ( void )
{


  TRACE_FUNCTION ("cmhPPP_Terminated()");

  
  cmhCC_PPP_Terminated( );
  
  return (AT_CMPL);
}

#endif /* WAP or TCPIP OR FF_GPF_TCPIP or SAT E */
