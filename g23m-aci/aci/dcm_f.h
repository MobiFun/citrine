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
                  in parallel
+-----------------------------------------------------------------------------
*/
#ifndef __DCM_F_H__
#define __DCM_F_H__


#include "typedefs.h"


#ifdef FF_GPF_TCPIP
#define GPF_TCPIP_STATEMENT(s) s
#else
#define GPF_TCPIP_STATEMENT(s)
#endif

#ifdef CO_UDP_IP
#define UDPIP_STATEMENT(s) s
#else
#define UDPIP_STATEMENT(s)
#endif


EXTERN BOOL is_gpf_tcpip_call();
EXTERN void set_gpf_tcpip_call();
EXTERN void reset_gpf_tcpip_call();

#endif /* __DCM_F_H__ */
