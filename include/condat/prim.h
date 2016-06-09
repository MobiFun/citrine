/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PRIM
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
|  Purpose :  Primitive Definitions, depending on entity
+-----------------------------------------------------------------------------
*/

#ifndef PRIM_H
#define PRIM_H

/*
 *  Definitions for Primitive Opcodes
 */

#ifndef PCONST_CDG
#include "pconst.cdg"
#endif /* PCONST_CDG */

/*
 *  Define the service access points depending on the entity
 */

#ifdef ENTITY_PL

#define SAP_PH
#define SAP_DL
#define SAP_MPH
#ifdef ALR
  #define SAP_MPHC
#elif defined(FF_GTI)
  /* GTI */
  /* do nothing */
#else
  #define SAP_MPH5
#endif  /* ALR, FF_GTI */
#if defined(GPRS) && defined(ALR)
  #define SAP_TB
#endif  /* GPRS */
#define SAP_MMI
#define SAP_MON
#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */


/* added by ppuel on 06/10 */
#define SAP_CST

#endif  /* ENTITY_PL */

#ifdef ENTITY_L1
#ifdef ALR
  #define SAP_MPHC
#elif defined(FF_GTI)
  /* GTI */
  /* do nothing */
#else
  #define SAP_MPH5
#endif  /* ALR, FF_GTI */
#endif  /* ENTITY_L1 */

#ifdef ENTITY_DL

#define SAP_MPHC	/* for PH_DATA_IND */
#define SAP_PH
#define SAP_DL
#define SAP_MDL

#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

#endif  /*  ENTITY_DL*/

#ifdef ENTITY_RR

#ifdef GPRS
#define SAP_INCLUDES /*Needed for CL*/
#endif

#define SAP_DL
#define SAP_RR
#define SAP_MPH
#define SAP_MON
#define SAP_RRRRLP
#define SAP_RRLC
#ifdef GPRS
#define SAP_CL_INLINE
#ifdef _SIMULATION_
#define SAP_CL
#endif /* _SIMULATION_ */
#endif

#ifdef GPRS
  #define SAP_RRGRR
#endif  /* GPRS */

#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

#endif  /* ENTITIY_RR  */

#ifdef ENTITY_LC

#define SAP_RRLC
#define SAP_RRLP
#define SAP_CSRLC
#define SAP_MNLC

#endif /* ENTITY_LC */

#ifdef ENTITY_RRLP

#define SAP_RRRRLP
#define SAP_RRLP

#endif /* ENTITY_RRLP */

#ifdef ENTITY_CSR

#define SAP_CSRLC

#endif /* ENTITY_CSR */

#ifdef ENTITY_MM

#ifdef  GPRS
#define SAP_MMGMM
#else
#define SAP_MMREG
#endif /* GPRS */
#define SAP_MMCM
#define SAP_MMSS
#define SAP_MMSMS
#define SAP_MDL
#define SAP_RR
#define SAP_SIM

#ifdef FF_EM_MODE
#define SAP_EM
#endif /* FF_EM_MODE */

#endif /* ENTITY_MM */

#ifdef ENTITY_CC

#define SAP_MMCM
#define SAP_MNCC

#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

#endif  /*  ENTITY_CC*/

#ifdef ENTITY_ESIM
#define SAP_AAA
#define SAP_MMI
#endif /* ENTITY_ESIM */

#ifdef ENTITY_SS

#define SAP_MMSS
#define SAP_MNSS

#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

#endif   /* ENTITY_SS  */

#ifdef ENTITY_SMS

#define SAP_MMSMS
#define SAP_MNSMS
#define SAP_SIM

#ifdef GPRS
  #define SAP_LL
  #define SAP_GMMSMS
#endif  /* GPRS */

#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

#endif /* ENTITY_SMS */


#ifdef ENTITY_CST

/* added by ppuel on 06/10 */
#define SAP_CST
#ifdef ALR
  #define SAP_MPHC
#elif defined(FF_GTI)
  /* GTI */
  /* do nothing */
#else
  #define SAP_MPH5
#endif  /* ALR, FF_GTI */
#define SAP_MNSMS
#define SAP_MMREG
#ifdef GPRS
 #define SAP_GMMREG
#endif /*GPRS*/
#endif   /* ENTITY_CST  */

#ifdef ENTITY_MMI

#define SAP_MNSS
#define SAP_MNSMS
#define SAP_MNCC
#define SAP_MMREG
#define SAP_MMI
#define SAP_SIM
#ifdef FF_ESIM
#define SAP_AAA /* needed for esim */
#endif
#ifdef FF_WAP
#define SAP_WAP
#endif /* FF_WAP */

#endif /* ENTITY_MMI */

#ifdef ENTITY_SMI

#define SAP_ACI
#define SAP_MNSS
#define SAP_MNSMS
#define SAP_MNCC
#define SAP_MMI
#define SAP_SIM
#define SAP_CST

#ifdef UART
#define SAP_DTI2
#define SAP_UART
#endif /* UART */

#ifdef FF_PSI
#define SAP_PSI
#endif /* FF_PSI */

#ifdef FF_BAT
#define SAP_APP
#endif /* FF_BAT */

#if defined FF_EOTD
#define SAP_MNLC
#endif  /* FF_EOTD */

#ifdef BT_ADAPTER
#define SAP_BTP
#endif /* BT_ADAPTER */

#ifdef FAX_AND_DATA
#define SAP_L2R
#define SAP_TRA
#define SAP_RA

#ifdef FF_FAX
#define SAP_T30
#endif

#endif  /* FAX_AND_DATA */

#ifdef GPRS
  #define SAP_INCLUDES /*For new include SAPs from TI DK*/
  #define SAP_GMMREG
  #define SAP_SN
  #define SAP_SMREG
  #define SAP_PPP
  #define SAP_DTI
  #define SAP_UART
  #define SAP_PKT
  #define SAP_UPM

#endif /* GPRS */

#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

#define SAP_MMREG

#if defined (FF_WAP) || defined (FF_SAT_E)
#define SAP_PPP
#endif

#ifdef CO_UDP_IP
#define SAP_IPA
#define SAP_UDPA
#endif /* CO_UDP_IP */

#ifdef FF_WAP 
#define SAP_WAP
#endif  /* FF_WAP */

#ifdef FF_TCP_IP
#define SAP_PPP
#define SAP_AAA
#endif  /* FF_TCP_IP */

#ifdef FF_GPF_TCPIP
#define SAP_TCPIP
#define SAP_DCM
#endif

#ifdef FF_WAP
#define SAP_WAP
#endif

#endif /* ENTITY_SMI */

#ifdef ENTITY_MFW

#define SAP_ACI
#define SAP_MNSS
#define SAP_MNSMS
#define SAP_MNCC
#define SAP_MMI
#define SAP_SIM
#define SAP_CST

#ifdef UART
#define SAP_DTI2
#define SAP_UART
#endif /* UART */

#ifdef FF_PSI
#define SAP_PSI
#endif  /* FF_PSI */

#ifdef FF_BAT
#define SAP_APP
#endif /* FF_BAT */

#ifdef FAX_AND_DATA
#define SAP_L2R

#ifdef FF_FAX
#define SAP_T30
#endif

#define SAP_RA
#define SAP_TRA
#endif  /* FAX_AND_DATA */

#ifdef GPRS
  #define SAP_INCLUDES /*For new include SAPs from TI DK*/
  #define SAP_GMMREG
  #define SAP_SN
  #define SAP_SMREG
  #define SAP_PPP
  #define SAP_DTI
  #define SAP_UART
  #define SAP_PKT
  #define SAP_UPM
#endif /* GPRS */

#define SAP_MMREG

#if defined (FF_WAP) || defined (FF_SAT_E)
#define SAP_PPP
#endif

#ifdef CO_UDP_IP
#define SAP_IPA
#define SAP_UDPA
#endif /* CO_UDP_IP */

#ifdef FF_WAP 
#define SAP_WAP
#endif /* FF_WAP */

#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

#ifdef FF_TCP_IP
#define SAP_PPP
#define SAP_AAA
#endif  /* FF_TCP_IP */

#ifdef FF_GPF_TCPIP
#define SAP_TCPIP
#define SAP_DCM
#endif

#ifdef FF_WAP
#define SAP_WAP
#endif

#if defined FF_EOTD
#define SAP_MNLC
#endif  /* FF_EOTD */

#ifdef BT_ADAPTER
#define SAP_BTP
#endif /* BT_ADAPTER */

#endif /* ENTITY_MFW */


#ifdef ENTITY_SIM

#define SAP_SIM
#ifdef CO_UDP_IP 
#define SAP_UDP
#endif  /* FF_WAP || FF_SAT_E */

#ifdef UART
#define SAP_DTI2
#endif  /*  UART*/
#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

#endif   /* ENTITY_SIM  */

#ifdef ENTITY_ACI

#define SAP_ACI
#define SAP_MNSS
#define SAP_MNSMS
#define SAP_SIM
#define SAP_MNCC
#define SAP_MMI
#define SAP_CST
#ifdef FF_ESIM
#define SAP_AAA /* needed for esim */
#endif
#ifdef UART
#define SAP_DTI2
#define SAP_UART
#endif /* UART */

#ifdef FF_PSI
#define SAP_PSI
#endif  /* FF_PSI */

#ifdef FF_BAT
#define SAP_APP
#endif /* FF_BAT */

#ifdef FF_EOTD
#define SAP_MNLC
#endif  /*  FF_EOTD*/

#ifdef FAX_AND_DATA
#define SAP_L2R
#define SAP_RA

#ifdef FF_FAX
#define SAP_T30
#endif

#define SAP_TRA
#endif  /* FAX_AND_DATA */

#ifdef GPRS
  #define SAP_INCLUDES /*For new include SAPs from TI DK*/
  #define SAP_GMMREG
  #define SAP_SN
  #define SAP_SMREG
  #define SAP_PPP
  #define SAP_DTI
  #define SAP_UART
  #define SAP_PKT
  #define SAP_UPM
#endif  /* GPRS */

#define SAP_MMREG

#if defined (FF_WAP) || defined (FF_SAT_E)
#define SAP_PPP
#endif

#ifdef CO_UDP_IP
#define SAP_IPA
#define SAP_UDPA
#endif  /* CO_UDP_IP */

#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

#ifdef FF_TCP_IP
#define SAP_PPP
#define SAP_AAA
#endif  /* FF_TCP_IP */

#ifdef FF_GPF_TCPIP
#define SAP_TCPIP
#define SAP_DCM
#endif

#ifdef FF_WAP
#define SAP_WAP
#endif

#endif  /* ENTITY_ACI */

#ifdef ENTITY_L2R

#define SAP_L2R
#define SAP_RLP
#define SAP_DTI2
#define SAP_TRA
#define SAP_RA

#endif   /* ENTITY_L2R  */

#ifdef ENTITY_RLP

#define SAP_RLP
#define SAP_RA

#endif   /* ENTITY_RLP  */

#ifdef ENTITY_T30

#define SAP_T30
#define SAP_DTI2
#define SAP_FAD

#endif  /* ENTITY_T30  */

#ifdef ENTITY_FAD

#define SAP_FAD
#define SAP_RA

#endif  /* ENTITY_FAD  */

#ifdef ENTITY_RA

#define SAP_RA

#endif  /* ENTITY_RA  */

#ifdef ENTITY_WAP

#define SAP_WAP

#ifdef FF_GPF_TCPIP
#define SAP_TCPIP
#define SAP_DCM
#endif

#ifdef CO_UDP_IP 
#define SAP_UDP
#define SAP_UDPA
#endif  /* CO_UDP_IP */

#define SAP_DTI2

#endif  /* ENTITY_WAP  */

#ifdef ENTITY_UDP

#define SAP_UDP
#define SAP_UDPA
#define SAP_IP
#define SAP_DTI2

#endif  /* ENTITY_UDP  */

#ifdef ENTITY_IP

#define SAP_IP
#define SAP_IPA
#define SAP_DTI2

#endif  /* ENTITY_IP  */

#ifdef ENTITY_BTI

/* removed check if compiling for dialup as it is part of the minimum choice
 * kk 010525
 */
#define SAP_ACI
#define SAP_BTP
#define SAP_DTI2

#endif /* ENTITY_BTI */


#ifdef ENTITY_GRR
#define SAP_INCLUDES /*Needed for CL*/
#define SAP_GMMRR
#define SAP_RRGRR
#define SAP_MPHP
#define SAP_TB
#define SAP_CGRLC
#define SAP_CL_INLINE

#ifdef _SIMULATION_
#define SAP_CL
#endif /* _SIMULATION_ */


#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

/*
 * The following SAP are not used in GRR,
 * but some of the structures defined
 * in theses interfaces are used.
 */
#define SAP_MPH

#endif  /* ENTITY_GRR  */


#ifdef ENTITY_GRLC

#define SAP_CGRLC
#define SAP_GRLC
#define SAP_MAC
#define SAP_L1TEST

#ifdef FF_EM_MODE
 #define SAP_EM
#endif  /* FF_EM_MODE */

#endif  /* ENTITY_GRLC */

#ifdef ENTITY_LLC

#define SAP_LLGMM
#define SAP_LL
#define SAP_CCI
#define SAP_GRLC
#define SAP_DTI2
#endif  /* ENTITY_LLC  */

#ifdef ENTITY_SM

#define SAP_INCLUDES /*For new include SAPs from TI DK*/
#define SAP_SMREG
/* #define SAP_SNSM
   #define SAP_GMMSM
   #define SAP_GMMAA
   #define SAP_LL   */
#define SAP_SM
#define SAP_MMPM

#define SAP_CL_INLINE

#ifdef _SIMULATION_
#define SAP_CL
#endif /* _SIMULATION_ */

#endif  /* ENTITY_SM  */

/*Newly added entity from TI DK*/
#ifdef ENTITY_UPM

#define SAP_INCLUDES /*For new include SAPs from TI DK*/
#define SAP_SM
#define SAP_SN
#define SAP_MMPM /*For MMPM_SEQUENCE_IND/RES*/
#define SAP_UPM

#define SAP_CL_INLINE

#ifdef _SIMULATION_
#define SAP_CL
#endif /* _SIMULATION_ */

#endif /* ENTITY_UPM */


#ifdef ENTITY_SNDCP

#define SAP_INCLUDES /*For new include SAPs from TI DK*/
#define SAP_SN /*SNDCP<->UPM and SNDCP<->ACI*/
#define SAP_LL
#define SAP_CCI
#define SAP_DTI2

#endif  /* ENTITY_SNDCP  */

#ifdef ENTITY_GMM

#define SAP_INCLUDES /*For new include SAPs from TI DK*/
#define SAP_GMMAA
#define SAP_GMMREG
#define SAP_GMMRR
#define SAP_MMPM /*Replaced GMMSM with MMPM */
#define SAP_GMMSMS
#define SAP_SIM
#define SAP_LL
#define SAP_LLGMM
#define SAP_MMGMM
#define SAP_CGRLC

#define SAP_CL_INLINE

#ifdef _SIMULATION_
#define SAP_CL
#endif /* _SIMULATION_ */

#ifdef FF_EM_MODE
#define SAP_EM
#endif  /* FF_EM_MODE */

#endif  /* ENTITY_GMM  */

#ifdef ENTITY_PPP

#define SAP_PPP
#define SAP_DTI
#ifdef GPRS
#define SAP_UART
#endif
#endif /* ENTITY_PPP */

#ifdef ENTITY_UART

#define SAP_UART
#define SAP_DTI2

#endif /* ENTITY_UART */

#ifdef ENTITY_PKTIO

#define SAP_PKT
#define SAP_DTI2

#endif /* ENTITY_PKTIO */

#ifdef ENTITY_PSI

#define SAP_PSI
#define SAP_DTI2
#define SAP_DIO

#endif /* ENTITY_PSI */

#ifdef ENTITY_AAA

#define SAP_AAA

#endif /* ENTITY_AAA */


#ifdef ENTITY_TCPIP

#define SAP_TCPIP
#define SAP_DTI2

#endif /* ENTITY_TCPIP */


#ifdef ENTITY_APP

#ifdef FF_BAT
#define SAP_APP
#endif /* FF_BAT */

#ifdef FF_GPF_TCPIP
#define SAP_TCPIP
#define SAP_DCM
#endif

#endif /* ENTITY_APP */

#ifdef _SIMULATION_

#ifdef ENTITY_CLT

#define SAP_INCLUDES /*For new include SAPs from TI DK*/
#define SAP_CL
#define SAP_CL_INLINE

#endif /* ENTITY_CLT */

#endif /* _SIMULATION_ */


/*The following is needed for target build*/
#ifdef ENTITY_CLT
#define SAP_INCLUDES
#endif /* ENTITY_CLT */

/*
 *  Define constants and primitive definitions depending on the
 *  service access points
 */
#ifdef SAP_INCLUDES

#include "p_8010_137_nas_include.h"
#include "p_8010_153_cause_include.h"
#include "p_8010_152_ps_include.h"

#endif /* SAP_INCLUDES */


#ifdef SAP_PH

/*
 * Mask for PH Opcodes
 */

#define PH_DL  0x4100
#define PH_UL  0x0100

#include "p_ph.h"

#endif  /* SAP_PH */

#ifdef SAP_MPH

/*
 * Mask for MPH Opcodes
 * old values:
#define MPH_DL  0x4200
#define MPH_UL  0x0200
 * changed to avoid double opcodes in TAP
 */
#define MPH_DL  0x5700
#define MPH_UL  0x1700

#include "p_mph.h"

#endif  /* SAP_MPH */

#ifdef SAP_MPH5

#include "p_mph5.h"

#endif  /* SAP_MPH5 */

#ifdef SAP_MPHC

/*
 * Mask for DL Opcodes
 */

#define MPHC_DL  0x0000 /* same as MPHC_UL */
#define MPHC_UL  0x0000 /* same as MPHC_DL */

#include "p_mphc.h"

#endif  /* SAP_MPHC */

#ifdef SAP_DL

/*
 * Mask for DL Opcodes
 */

#define DL_DL  0x80004003
#define DL_UL  0x80000003

#include "p_dl.h"

#endif  /* SAP_DL */

#ifdef SAP_MDL

/*
 * Mask for MDL Opcodes
 */

#define MDL_DL  0x80004004
#define MDL_UL  0x80000004

#include "p_mdl.h"

#endif  /* SAP_MDL */

#ifdef SAP_SIM

/*
 * Mask for SIM Opcodes
 */

#define SIM_DL  0x80004005
#define SIM_UL  0x80000005

#include "p_sim.h"

#ifdef TI_PS_UICC_CHIPSET_15
#include "p_8010_136_simdrv_sap.h"
#endif

#endif  /* SAP_SIM */

#ifdef SAP_RR

/*
 * Mask for RR Opcodes
 */

#define RR_DL  0x80004006
#define RR_UL  0x80000006

#include "p_rr.h"

#endif  /* SAP_RR */

#ifdef SAP_MMCM

/*
 * Mask for MMCM Opcodes
 */

#define MMCM_DL  0x80004007
#define MMCM_UL  0x80000007

#include "p_mmcm.h"

#endif  /* SAP_MMCM */

#ifdef SAP_MMSS

/*
 * Mask for MMSS Opcodes
 */

#define MMSS_DL  0x80004008
#define MMSS_UL  0x80000008

#include "p_mmss.h"

#endif  /* SAP_MMSS */

#ifdef SAP_MMSMS

/*
 * Mask for MMSMS Opcodes
 */

#define MMSMS_DL  0x80004009
#define MMSMS_UL  0x80000009

#include "p_mmsms.h"

#endif  /* SAP_MMSMS */

#ifdef SAP_MMREG

/*
 * Mask for MMREG Opcodes
 */

#define MMREG_DL  0x8000400A
#define MMREG_UL  0x8000000A

#include "p_mmreg.h"

#endif  /* SAP_MMREG */

#ifdef SAP_MNCC

/*
 * Mask for MNCC Opcodes
 */

#define MNCC_DL  0x8000400B
#define MNCC_UL  0x8000000B

#include "p_mncc.h"

#endif  /* SAP_MNCC */

#ifdef SAP_MNSS

/*
 * Mask for MNSS Opcodes
 */

#define MNSS_DL  0x8000400C
#define MNSS_UL  0x8000000C

#include "p_mnss.h"

#endif  /* SAP_MNSS */

#ifdef SAP_MNSMS

/*
 * Mask for MNSMS Opcodes
 */

#define MNSMS_DL  0x8000400D
#define MNSMS_UL  0x8000000D

#include "p_mnsms.h"

#endif  /* SAP_MNSMS */

#ifdef SAP_MMI

/*
 * Mask for MMI Opcodes
 */

#define MMI_DL  0x4E00
#define MMI_UL  0x0E00

#include "p_mmi.h"

#endif  /* SAP_MMI */

#ifdef SAP_MON

#include "p_mon.h"

#endif  /* SAP_MON */

#ifdef SAP_ACI

/*
 * Mask for ACI Opcodes
 */

#define ACI_DL  0x5500
#define ACI_UL  0x1500

#include "p_aci.h"

#endif  /* SAP_ACI */

#ifdef SAP_L2R

/*
 * Mask for L2R Opcodes
 */

#define L2R_DL  0x80004012
#define L2R_UL  0x80000012

#include "p_l2r.h"

#endif  /* SAP_L2R */


#ifdef SAP_TRA

/*
 * Mask for TRA Opcodes
 */

#define TRA_DL  0x80004019
#define TRA_UL  0x80000019

#include "p_tra.h"

#endif  /* SAP_TRA */


#ifdef SAP_RLP

/*
 * Mask for RLP Opcodes
 */

#define RLP_DL  0x80004011
#define RLP_UL  0x80000011

#include "p_rlp.h"

#endif  /* SAP_RLP */

#ifdef SAP_T30

/*
 * Mask for T30 Opcodes
 */

#define T30_DL  0x80004014
#define T30_UL  0x80000014

#ifdef FF_FAX
#include "p_t30.h"
#endif

#endif  /* SAP_T30 */

#ifdef SAP_FAD

/*
 * Mask for FAD Opcodes
 */

#define FAD_DL  0x80004013
#define FAD_UL  0x80000013

#ifdef FF_FAX
#include "p_fad.h"
#endif

#endif  /* SAP_FAD */

#ifdef SAP_RA

/*
 * Mask for RA Opcodes
 */

#define RA_DL  0x80004010
#define RA_UL  0x80000010

#include "p_ra.h"

#endif  /* SAP_RA */



#ifdef SAP_DTI

/*
 * Mask for DTI Opcodes
 */

#define DTI_DL  0x7700
#define DTI_UL  0x3700

#include "p_dti.h"

#endif  /* SAP_DTI */


#ifdef SAP_WAP

/*
 * Mask for WAPcodes
 */

#define WAP_DL  0x7D00
#define WAP_UL  0x3D00

#include "p_wap.h"

#endif  /* SAP_WAP */



#ifdef SAP_UDP

/*
 * Mask for UDP Opcodes
 */

#define UDP_DL  0x80004046
#define UDP_UL  0x80000046

#include "p_udp.h"

#endif  /* SAP_UDP */



#ifdef SAP_TCPIP

/*
 * Mask for TCPIP Opcodes
 */

#define TCPIP_DL  0x80004048
#define TCPIP_UL  0x80000048

#include "p_tcpip.h"

#endif



#ifdef SAP_UDPA

/*
 * Mask for UDPA Opcodes
 */

#define UDPA_DL  0x8000403B
#define UDPA_UL  0x8000003B

#include "p_udpa.h"

#endif  /* SAP_UDPA */



#ifdef SAP_IP

/*
 * Mask for IP Opcodes
 */

#define IP_DL  0x7900
#define IP_UL  0x3900

#include "p_ip.h"

#endif  /* SAP_IP */



#ifdef SAP_IPA

/*
 * Mask for IPA Opcodes
 */
#define IPA_DL  0x8000403C
#define IPA_UL  0x8000003C

#include "p_ipa.h"

#endif  /* SAP_IPA */



#ifdef SAP_PPP

/*
 * Mask for PPP Opcodes
 */

#define PPP_DL  0x7500
#define PPP_UL  0x3500

#include "p_ppp.h"

#endif  /* SAP_PPP */



#ifdef SAP_CST

/*
 * Mask for CST Opcodes
 */
#define CST_DL  0x5600
#define CST_UL  0x1600

#include "p_cst.h"

#endif  /* SAP_CST */

#ifdef SAP_TB

/*
 * Mask for TB Opcodes
 */

#define TB_DL  0x5800
#define TB_UL  0x1800

#include "p_tb.h"

#endif  /* SAP_TB */

#ifdef SAP_MPHP

/*
 * Mask for MPHP Opcodes
 */

#define MPHP_DL 0x0200
#define MPHP_UL 0x0200

#include "p_mphp.h"

#endif  /* SAP_MPHP */

#ifdef SAP_CGRLC

/*
 * Mask for CGRLC Opcodes
 */

#define CGRLC_DL 0x80014098
#define CGRLC_UL 0x80000098

#include "p_cgrlc.h"

#endif  /* SAP_CGRLC */

#ifdef SAP_CL_INLINE
#include "p_cl.val"
#include "cl_inline.h"

#endif /* SAP_CL_INLINE */


#ifdef SAP_MAC

/*
 * Mask for MAC Opcodes
 */

#define MAC_DL 0x7200
#define MAC_UL 0x3200

#include "p_mac.h"

#endif  /* SAP_MAC */

#ifdef SAP_L1TEST

/*
 * Mask for L1TEST Opcodes
 */

#define L1TEST_DL 0x8000409B
#define L1TEST_UL 0x8000009B

#include "p_l1test.h"

#endif  /* SAP_L1TEST */

#ifdef SAP_RRGRR

/*
 * Mask for RRGRR Opcodes
 */

#define RRGRR_DL 0x6d00
#define RRGRR_UL 0x2d00

#include "p_rrgrr.h"

#endif  /* SAP_RRGRR */

#ifdef SAP_LLGMM

/*
 * Mask for LLGMM Opcodes
 */

#define LLGMM_DL 0x6100
#define LLGMM_UL 0x2100

#include "p_llgmm.h"

#endif  /* SAP_LLGMM */

#ifdef SAP_LL

/*
 * Mask for LL Opcodes
 */

#define LL_DL    0x6200
#define LL_UL    0x2200

#include "p_ll.h"

#endif  /* SAP_LL */


#ifdef SAP_SNSM
/*
 * Mask for SNSM Opcodes
 */
#define SNSM_DL  0x6700
#define SNSM_UL  0x2700
#include "p_snsm.h"

#endif  /* SAP_SNSM */

#ifdef SAP_SN
/*
 * Mask for SN Opcodes
 */
#define SN_DL  0x8000409E
#define SN_UL  0x8000009E

#include "p_8010_135_sn_sap.h"
#endif /*#ifdef SAP_SN*/


#ifdef SAP_SMREG

/*
 * Mask for SMREG Opcodes
 */

#define SMREG_DL  0x6600
#define SMREG_UL  0x2600

#include "p_8010_142_smreg_sap.h"

#endif  /* SAP_SMREG */

#ifdef SAP_SM
/*
 * Mask for SM Opcodes
 */
#define SM_DL  0x80004090
#define SM_UL  0x80000090

#include "p_8010_128_sm_sap.h"

#endif /*#ifdef SAP_SN*/


#ifdef SAP_UPM
/*
 * Mask for UPM Opcodes
 */
#define UPM_DL 0x8000409D
#define UPM_UL 0x8000009D

#include "p_8010_157_upm_sap.h"

#endif /*#ifdef SAP_UPM*/



#ifdef SAP_GRLC

/*
 * Mask for GRLC Opcodes
 */

#define GRLC_DL   0x80004097
#define GRLC_UL   0x80000097

#include "p_grlc.h"

#endif  /* SAP_GRLC */

#ifdef SAP_GMMAA

/*
 * Mask for GMMAA Opcodes
 */

#define GMMAA_DL   0x6500
#define GMMAA_UL   0x2500

#include "p_gmmaa.h"

#endif  /* SAP_GMMAA */

#ifdef SAP_GMMREG

/*
 * Mask for GMMREG Opcodes
 */

#define GMMREG_DL   0x7300
#define GMMREG_UL   0x3300

#include "p_gmmreg.h"

#endif  /* SAP_GMMREG */

#ifdef SAP_GMMRR

/*
 * Mask for GMMRR Opcodes
 */

#define GMMRR_DL   0x5f00
#define GMMRR_UL   0x1f00

#include "p_gmmrr.h"

#endif  /* SAP_GMMRR */

#ifdef SAP_GMMSM

/*
 * Mask for GMMSM Opcodes
 */

#define GMMSM_DL   0x6400
#define GMMSM_UL   0x2400

#include "p_gmmsm.h"

#endif  /* SAP_GMMSM */

#ifdef SAP_MMPM
/*
 * Mask for MMPM Opcodes
 * GMMRABM, GMMSM, GMMSMS, PMMSMS SAPs replaced by MMPM SAP
 */
#define MMPM_DL  0x80004096
#define MMPM_UL  0x80000096

#include "p_8010_134_mmpm_sap.h"

#endif /*#ifdef SAP_MMPM*/

#ifdef SAP_GMMSMS

/*
 * Mask for GMMSMS Opcodes
 */

#define GMMSMS_DL   0x6300
#define GMMSMS_UL   0x2300

#include "p_gmmsms.h"

#endif  /* SAP_GMMSMS */


#ifdef SAP_GSIM

/*
 * Mask for GMMSIM Opcodes
 */

#define GSIM_DL   0x6900
#define GSIM_UL   0x2900

#include "p_gsim.h"

#endif  /* SAP_GSIM */

#ifdef SAP_MMGMM

/*
 * Mask for MMGMM Opcodes
 */

#define MMGMM_DL   0x6e00
#define MMGMM_UL   0x2e00

#include "p_mmgmm.h"

#endif  /* SAP_MMGMM */

#ifdef SAP_BTP

/*
 * Mask for BTP Opcodes
 */

#define BTP_G   0x7a00
#define BTP_B   0x3a00

#include "p_btp.h"

#endif  /* SAP_BTP */

#ifdef SAP_PPP

/*
 * Mask for PPP Opcodes
 */

#define PPP_DL   0x7500
#define PPP_UL   0x3500

#include "p_ppp.h"

#endif /* SAP_PPP */

#ifdef SAP_UART

/*
 * Mask for UART Opcodes
 */

#define UART_DL   0x7400
#define UART_UL   0x3400

#include "p_uart.h"

#endif /* SAP_UART */

#ifdef SAP_CCI

/*
 * Mask for CCI Opcodes
 */

#define CCI_DL    0x7600
#define CCI_UL    0x3600

#include "p_cci.h"

#endif /* SAP_CCI */

#ifdef SAP_DTI2

/*
 * Mask for DTI Opcodes
 */

#define DTI2_DL   0x7700
#define DTI2_UL   0x3700

#include "p_dti2.h"

#endif /* SAP_DTI2 */

#ifdef SAP_PKT

/*
 * Mask for PKT opcodes
 */

#define PKT_DL 0x80004045
#define PKT_UL 0x80000045

#include "p_pkt.h"

#endif /* SAP_PKT */
#ifdef SAP_EM

/*
 * Mask for EM Opcodes
 */

#define EM_Dl  0x7E00
#define EM_Ul  0x3E00

#include "p_em.h"

#endif  /* SAP_EM */



#ifdef SAP_RRLC

/*
 * Mask for RREOTD Opcodes
 */

#define RRLC_DL  0x80004040
#define RRLC_UL  0x80000040

#include "p_rrlc.h"

#endif /* SAP_RRLC */

#ifdef SAP_RRRRLP

/*
 * Mask for RRRRLP Opcodes
 */

#define RRRRLP_DL  0x80004041
#define RRRRLP_UL  0x80000041

#include "p_rrrrlp.h"

#endif /* SAP_RRRRLP */

#ifdef SAP_RRLP

/*
 * Mask for RRLP Opcodes
 */

#define RRLP_DL  0x80004042
#define RRLP_UL  0x80000042

#include "p_rrlp.h"

#endif /* SAP_RRLP */

#ifdef SAP_CSRLC

/*
 * Mask for CSRLC Opcodes
 */

#define CSRLC_DL  0x80004043
#define CSRLC_UL  0x80000043

//#include "p_csrlc.h"

#endif /* SAP_CSRLC */

#ifdef SAP_MNLC

/*
 * Mask for MNLC Opcodes
 */

#define MNLC_DL  0x80004044
#define MNLC_UL  0x80000044

#include "p_mnlc.h"

#endif /* SAP_MNLC */


#ifdef SAP_AAA

/*
 * Mask for AAA Opcodes
 */
#define AAA_DL  0x80004047
#define AAA_UL  0x80000047

#include "p_aaa.h"

#endif /* SAP_AAA */

/*
* Mask for DCM Opcodes
*/
#ifdef SAP_DCM

#define DCM_DL 0x8000401C
#define DCM_UL 0x8000001C

#include "p_dcm.h"

#endif /* SAP_DCM */
#ifdef SAP_PSI

/*
 * Mask for PSI Opcodes
 */
#define PSI_DL  0x8000401D
#define PSI_UL  0x8000001D

#include "p_psi.h"
#include "DIO_inline.h"
#endif /* SAP_PSI */

#ifdef _SIMULATION_

#ifdef SAP_CL

/*
 * Mask for CL Opcodes
 */

#define CL_DL   0x8000409C
#define CL_UL   0x8000009C

#include "p_cl.h"

#endif /* SAP_CL */

#endif /* _SIMULATION_ */

#ifdef SAP_APP

/*
 * Mask for APP Opcodes
 */
#define APP_DL  0x8000401E
#define APP_UL  0x8000001E

#include "p_app.h"
#endif /* SAP_APP */

#ifdef FF_BAT
#include "p_bat.h"
#endif /* FF_BAT */

#endif  /* PRIM_H */
