/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_PPPS
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
|  Purpose :  This module provides the set functions related to the 
|             protocol stack adapter for the PPP module.
+----------------------------------------------------------------------------- 
*/ 

#ifdef DTI

#if defined(FF_WAP) || defined(FF_PPP) || defined(FF_GPF_TCPIP) || defined(FF_SAT_E)

#ifndef CMH_PPPS_C
#define CMH_PPPS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
  #include "aci_fd.h"
#endif

#include "ksd.h"

#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"

#include "psa.h"
#include "psa_sim.h"
#include "psa_sms.h"
#include "wap_aci.h"
#include "psa_ppp_w.h"

#include "phb.h"
#include "cmh.h"
#include "cmh_phb.h"
#include "cmh_sms.h"


/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PPPS                     |
| STATE   : code             ROUTINE : sAT_PercentPPP               |
+-------------------------------------------------------------------+

  PURPOSE : This command will be called from MMI to set the 
            authentification protocol, the login name, and the password 
		    for the PPP entity.

            
*/
GLOBAL T_ACI_RETURN sAT_PercentPPP ( T_ACI_CMD_SRC srcId, 
                                     T_ACI_PPP_PROT protocol, 
									                   CHAR *login, 
									                   CHAR *pwd,
                                     T_ACI_PPP_CON  con_type )
{
	
  switch (protocol)
  {
	case(A_NO_AUTH):
			pppShrdPrm.auth_prot = (T_ACI_PPP_PROT)PPP_AP_NO;
		  break;

		case(A_PAP):
			pppShrdPrm.auth_prot = (T_ACI_PPP_PROT)PPP_AP_PAP;
			break;

		case(A_CHAP):
			pppShrdPrm.auth_prot = (T_ACI_PPP_PROT)PPP_AP_CHAP;
			break;

		case(A_AUTO_AUTH):
			pppShrdPrm.auth_prot = (T_ACI_PPP_PROT)PPP_AP_AUTO;
			break;

		default:
			TRACE_EVENT("Wrong value for authentification protocol");
			return AT_FAIL;
  }
#ifdef FF_PPP
  switch (con_type)
  {
   
   case(USE_NO_PPP_FOR_AAA):
      pppShrdPrm.is_PPP_CALL = FALSE;
      break;

   case(USE_PPP_FOR_AAA):
      pppShrdPrm.is_PPP_CALL = TRUE;
      break;

   default:
      TRACE_EVENT("Wrong value for connection type");
      return AT_FAIL;
  }
#endif /* FF_PPP */


	if((login NEQ NULL) AND (pwd NEQ NULL))
	{
		memcpy(pppShrdPrm.ppp_login,login, sizeof(pppShrdPrm.ppp_login)); 
		memcpy(pppShrdPrm.ppp_password,pwd, sizeof(pppShrdPrm.ppp_password));
	}
	else
		return AT_FAIL;

	return AT_CMPL;
}


#ifdef DTI
/** Functional interface to the AT%PPP? command. The IP addresses are in host
 * byte order.
 * 
 * @param srcId       ID of command source (unused).
 * @param ipaddr_p    Pointer to IP address variable of caller.
 * @param dns1_p      Pointer to dns1 variable of caller.
 * @param dns2_p      Pointer to dns2 variable of caller.
 * @return AT_CMPL
 */
GLOBAL T_ACI_RETURN qAT_PercentPPP(UBYTE srcId, ULONG *ipaddr_p, 
                                   ULONG *dns1_p, ULONG *dns2_p)
{
  if (ipaddr_p NEQ NULL)
  {
    *ipaddr_p = pppShrdPrm.ipaddr ;
  }
  if (dns1_p NEQ NULL)
  {
    *dns1_p = pppShrdPrm.dns1 ;
  }
  if (dns2_p NEQ NULL)
  {
    *dns2_p = pppShrdPrm.dns2 ;
  }

  return AT_CMPL ;
}
#endif /* DTI */



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PPPS                     |
| STATE   : code             ROUTINE : cmhPPP_Terminate             |
+-------------------------------------------------------------------+

  PURPOSE : 

            
*/
GLOBAL T_ACI_RETURN cmhPPP_Terminate ( T_ACI_PPP_LOWER_LAYER ppp_lower_layer )
{
  TRACE_FUNCTION("cmhPPP_Terminate()");
  
  psaPPP_Terminate(ppp_lower_layer);

  return (AT_CMPL);
}

#endif /* defined(WAP) || defined(FF_PPP) || defined(FF_GPF_TCPIP) || defined (FF_SAT_E) */
#endif /* DTI */
