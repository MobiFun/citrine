/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  
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
|  Purpose :  source control management for GACI.
+----------------------------------------------------------------------------- 
*/ 

#ifndef GACI_SRCC_H
#define GACI_SRCC_H


typedef enum {
  SRRC_SNDCP_ENTITY = 0,
  SRRC_PPPS_ENTITY,
  SRRC_SIM_ENTITY,
#ifdef CO_UDP_IP
  SRRC_IP_ENTITY,
#endif
#ifdef FF_GPF_TCPIP
  SRRC_TCPIP_ENTITY,
#endif /* FF_GPF_TCPIP */
  SRRC_MAX_ENTITY

} SRCC_ENTITY_ID;


typedef enum {
  SRCC_INVALID_LINK = -1,
  SRCC_PPPS_SNDCP_LINK,
  //SRCC_NULL_SNDCP_LINK,
  SRCC_SIM_SNDCP_LINK,
#if defined (FF_PKTIO) OR defined (FF_TCP_IP) OR defined(FF_GPF_TCPIP) OR defined(FF_PSI)
  SRCC_PKTIO_SNDCP_LINK,
#endif /* FF_PKTIO OR FF_TCP_IP OR FF_GPF_TCPIP OR FF_PSI */
#if defined(FF_GPF_TCPIP)
  SRCC_TCPIP_SNDCP_LINK,
#endif /* FF_GPF_TCPIP */
  SRCC_IP_SNDCP_LINK,
  SRCC_MAX_LINK

} SRCC_LINK_NO;


/*********************************************************************/
/** START ** This will be defined in the SAP of the entity ** START **/

#ifndef MAX_SNDCP_INSTANCES
  #define MAX_SNDCP_INSTANCES  11
#endif

#ifndef MAX_PPPS_INSTANCES
  #define MAX_PPPS_INSTANCES  1
#endif

#ifndef MAX_SIM_INSTANCES
  #define MAX_SIM_INSTANCES  1
#endif

/************ WAP ************/
#ifdef CO_UDP_IP 
#ifndef MAX_IP_INSTANCES
  #define MAX_IP_INSTANCES  1
#endif
#endif
#ifdef FF_GPF_TCPIP
#ifndef MAX_TCPIP_INSTANCES
  #define MAX_TCPIP_INSTANCES 1
#endif
#endif
/************ WAP ************/
/*** END *** This will be defined in the SAP of the entity *** END ***/
/*********************************************************************/





/*
 *    function declarations
 */
EXTERN void   srcc_init             ( void );
EXTERN void   srcc_new_count        ( SRCC_LINK_NO link_no );
EXTERN void   srcc_delete_count     ( SRCC_LINK_NO link_no );
EXTERN BOOL   srcc_reserve_sources  ( SRCC_LINK_NO link_no, SHORT no );

#endif

