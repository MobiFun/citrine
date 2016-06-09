/*
+-----------------------------------------------------------------------------
|  Project :  
|  Modul   :  ACI - DCM
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
|  Description :  Contains functionality to maintain UDP/IP and TCP/IP stacks
|                 in parallel
+-----------------------------------------------------------------------------
*/

#include "dcm_f.h"
#include "aci_all.h"


/* This flag is set if an application uses the GPF TCP/IP functionality,
   in case using the old UDP/IP stack this flag is set to false (default) */
static BOOL gpfTcpIpCall = FALSE;

/******************************************************************************/
BOOL is_gpf_tcpip_call()
{
  /* check to ensure that everything is configured well*/
#if !defined(FF_GPF_TCPIP)
  ACI_ASSERT(gpfTcpIpCall EQ FALSE);
#endif
  
  return gpfTcpIpCall;
}


/******************************************************************************/
void set_gpf_tcpip_call()
{
#if !defined(FF_GPF_TCPIP)
    ACI_ASSERT(FALSE);
#endif
  ACI_ASSERT(gpfTcpIpCall NEQ TRUE);
  gpfTcpIpCall = TRUE;
}


/******************************************************************************/
void reset_gpf_tcpip_call()
{
#if !defined(FF_GPF_TCPIP)
    ACI_ASSERT(FALSE);
#endif
  /*ACI_ASSERT(gpfTcpIpCall EQ TRUE);*/
  gpfTcpIpCall = FALSE;
}

