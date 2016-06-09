/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_BAT
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
|  Purpose :  This Modul is the main entry point of BAT at ACI.
|
+-----------------------------------------------------------------------------
*/

#ifndef ACI_BAT_C
#define ACI_BAT_C

/*==== INCLUDES ===================================================*/
#include "aci_all.h"
#include "aci_mem.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci.h"
#include "aci_lst.h"

#include "psa_psi.h"
#include "cmh_psi.h"
#include "ati_src_psi.h"
#include "sap_dti.h"
#include "psa.h"
#include "cmh.h"

#include "aci_bat_cmh.h"
#include "aci_bat.h"
#include "aci_bat_err.h"
#include "aci_bat_cb.h"  /* for rBAT_OK */
#include "aci_bat_ext.h"

#include "l2p_types.h"
#include "l2p.h"
#include "bat_ctrl.h"
#include "aci_cmd.h"

#include "ati_ext_mech.h"

#ifdef _SIMULATION_
#include "gdd_aci.h"
#endif

/*==== CONSTANTS ==================================================*/

#define BAT_BIT_12 0x1000
#define BAT_BIT_15 0x8000
#define BAT_CTRL_CHANNEL 0xFE

#define MAX_NORMAL_BUFFER_LEN (100-4) /* Since Malloc uses 4 additional internal bytes of the Blocks */
#ifdef GPRS
#define MAX_LARGE_BUFFER_LEN (600-4)
#else
#define MAX_LARGE_BUFFER_LEN (400-4)
#endif

/* This value must always match with >>#define BAT_RCV_BUF_SIZE 800<< of bat_types.h !!! */
#define BAT_MTU_SIZE  800 /* %SIMEF response comes with 784 bytes.  */

/*==== TYPES ======================================================*/
typedef struct
{
  T_ACI_BAT_RSLT (*fptr)(T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd); /* BAT wrapper func */
} BATCommand;

typedef struct
{
  T_ACI_DTI_PRC_PSI *psi_src;      /* remember which PSI source currently active          */
  BOOL               l2p_configured;
  U32                l2p_mem[(L2P_CONTROL_BLOCK_HDR_SIZE+3)/4];
  U8                 l2p_id;       /* L2P calls this bat_id, for us it is a unique l2p_id */
} T_ACI_BAT_GLOBALS;

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

LOCAL T_ACI_BAT_GLOBALS aci_bat_globs[CMD_SRC_MAX];
LOCAL BOOL aci_bat_first_time = TRUE;


/* basic BAT commands, which are always supported */
#define BAT_BAS(batFoo)   {BAT_##batFoo}

/* default extended BAT commands, which are always supported
 * c = command type --> s=set, t=test, q=query
 */
#define BAT_DEF(c,batFoo) {c##BAT_##batFoo}

#ifdef FAX_AND_DATA
  #define BAT_FAD(c,batFoo) {c##BAT_##batFoo}
#else
  #define BAT_FAD(c,batFoo) {0}
#endif

#ifdef SIM_TOOLKIT
  #define BAT_STK(c,batFoo) {c##BAT_##batFoo}
#else
  #define BAT_STK(c,batFoo) {0}
#endif

#ifdef GPRS
  #define BAT_GPR(c,batFoo) {c##BAT_##batFoo}
#else
  #define BAT_GPR(c,batFoo) {0}
#endif

#if defined(FF_WAP) || defined(FF_TCP_IP) || defined(FF_SAT_E)
  #define BAT_PPP(c,batFoo) {c##BAT_##batFoo}
#else
  #define BAT_PPP(c,batFoo) {0}
#endif

#ifdef TI_PS_FF_AT_CMD_P_ECC
  #define BAT_ECC(c,batFoo) {c##BAT_##batFoo}
#else
  #define BAT_ECC(c,batFoo) {0}
#endif /* TI_PS_FF_AT_CMD_P_ECC */

#ifdef SIM_PERS
  #define BAT_SLK(c,batFoo) {c##BAT_##batFoo}
#else
  #define BAT_SLK(c,batFoo) {0}
#endif /* SIM_PERS */

/*
 * all the set commands are in range of 0x0000..0x0nnn
 * T_BAT_ctrl_params types is used as index to the table
 */
#ifdef _SIMULATION_
#pragma message( __TODO__"Currently No Support for Wide Chars !" )
#endif
/* expand the macro by hand: BAT_DEF(s,PercentALS) --> sBAT_PercentALS */
LOCAL const BATCommand bat_cmds [] =
{
  BAT_DEF(s,PercentALS),      /* BAT_CMD_SET_PERCENT_ALS        = 0x0   */
  BAT_DEF(s,PercentBAND),     /* BAT_CMD_SET_PERCENT_BAND       = 0x1   */
  BAT_BAS(ANDC),              /* BAT_CMD_ANDC                   = 0x2   */
  BAT_DEF(s,PlusCACM),        /* BAT_CMD_SET_PLUS_CACM          = 0x3   */
  BAT_DEF(s,PlusCAMM),        /* BAT_CMD_SET_PLUS_CAMM          = 0x4   */
  BAT_DEF(s,PlusCAOC),        /* BAT_CMD_SET_PLUS_CAOC          = 0x5   */
  BAT_DEF(s,PercentCBHZ),     /* BAT_CMD_SET_PERCENT_CBHZ       = 0x6   */
#ifdef FAX_AND_DATA
  BAT_DEF(s,PlusCBST),        /* BAT_CMD_SET_PLUS_CBST          = 0x7   */
#else
  {NULL},
#endif /* FAX_AND_DATA */
  BAT_DEF(s,PercentCCBS),     /* BAT_CMD_SET_PERCENT_CCBS       = 0x8   */
  BAT_DEF(s,PlusCCFC),        /* BAT_CMD_SET_PLUS_CCFC          = 0x9   */
  BAT_DEF(s,PlusCCLK),        /* BAT_CMD_SET_PLUS_CCLK          = 0xa   */
  BAT_DEF(s,PlusCCUG),        /* BAT_CMD_SET_PLUS_CCUG          = 0xb   */
  BAT_DEF(s,PlusCCWA),        /* BAT_CMD_SET_PLUS_CCWA          = 0xc   */
  BAT_DEF(s,PlusCCWA),        /* BAT_CMD_SET_PLUS_CCWA_W        = 0xd   */
  BAT_DEF(s,PlusCFUN),        /* BAT_CMD_SET_PLUS_CFUN          = 0xe   */
  BAT_GPR(s,PercentCGAATT),   /* BAT_CMD_SET_PERCENT_CGAATT     = 0xf   */
  BAT_GPR(s,PlusCGACT),       /* BAT_CMD_SET_PLUS_CGACT         = 0x10  */
  BAT_GPR(s,PlusCGANS),       /* BAT_CMD_SET_PLUS_CGANS         = 0x11  */
  BAT_GPR(s,PlusCGATT),       /* BAT_CMD_SET_PLUS_CGATT         = 0x12  */
  BAT_GPR(s,PercentCGCLASS),  /* BAT_CMD_SET_PERCENT_CGCLASS    = 0x13  */
  BAT_GPR(s,PlusCGCLASS),     /* BAT_CMD_SET_PLUS_CGCLASS       = 0x14  */
  BAT_GPR(s,PlusCGDATA),      /* BAT_CMD_SET_PLUS_CGDATA        = 0x15  */
  BAT_GPR(s,PlusCGDCONT),     /* BAT_CMD_SET_PLUS_CGDCONT       = 0x16  */
  BAT_GPR(s,PercentCGMM),     /* BAT_CMD_SET_PERCENT_CGMM       = 0x17  */
  BAT_GPR(s,PlusCGPADDR),     /* BAT_CMD_SET_PLUS_CGPADDR       = 0x18  */
  BAT_GPR(s,PercentCGPCO),    /* BAT_CMD_SET_PERCENT_CGPCO      = 0x19  */
  BAT_GPR(s,PercentCGPPP),    /* BAT_CMD_SET_PERCENT_CGPPP      = 0x1a  */
  BAT_GPR(s,PlusCGQMIN),      /* BAT_CMD_SET_PLUS_CGQMIN        = 0x1b  */
  BAT_GPR(s,PlusCGQREQ),      /* BAT_CMD_SET_PLUS_CGQREQ        = 0x1c  */
  BAT_GPR(s,PlusCGSMS),       /* BAT_CMD_SET_PLUS_CGSMS         = 0x1d  */
  BAT_DEF(s,PercentCHLD),     /* BAT_CMD_SET_PERCENT_CHLD       = 0x1e  */
  BAT_DEF(s,PercentCHPL),     /* BAT_CMD_SET_PERCENT_CHPL       = 0x1f  */
  BAT_DEF(s,PercentCHPL),     /* BAT_CMD_SET_PERCENT_CHPL_W     = 0x20  */
  BAT_DEF(s,PlusCIND),        /* BAT_CMD_SET_PLUS_CIND          = 0x21  */
  BAT_DEF(s,PlusCLAN),        /* BAT_CMD_SET_PLUS_CLAN          = 0x22  */
  BAT_DEF(s,PlusCLCK),        /* BAT_CMD_SET_PLUS_CLCK          = 0x23  */
  BAT_DEF(s,PlusCLIR),        /* BAT_CMD_SET_PLUS_CLIR          = 0x24  */
  BAT_DEF(s,PlusCLVL),        /* BAT_CMD_SET_PLUS_CLVL          = 0x25  */
  BAT_DEF(s,PlusCMER),        /* BAT_CMD_SET_PLUS_CMER          = 0x26  */
  BAT_DEF(s,PlusCMGC),        /* BAT_CMD_SET_PLUS_CMGC          = 0x27  */
  BAT_DEF(s,PlusCMGD),        /* BAT_CMD_SET_PLUS_CMGD          = 0x28  */
  BAT_DEF(s,PlusCMGL),        /* BAT_CMD_SET_PLUS_CMGL          = 0x29  */
  BAT_DEF(s,PlusCMGL),        /* BAT_CMD_SET_PLUS_CMGL_W        = 0x2a  */
  BAT_DEF(s,PlusCMGR),        /* BAT_CMD_SET_PLUS_CMGR          = 0x2b  */
  BAT_DEF(s,PlusCMGR),        /* BAT_CMD_SET_PLUS_CMGR_W        = 0x2c  */
  BAT_DEF(s,PlusCMGS),        /* BAT_CMD_SET_PLUS_CMGS          = 0x2d  */
  BAT_DEF(s,PlusCMGW),        /* BAT_CMD_SET_PLUS_CMGW          = 0x2e  */
  BAT_DEF(s,PlusCMOD),        /* BAT_CMD_SET_PLUS_CMOD          = 0x2f  */
  BAT_DEF(s,PlusCMSS),        /* BAT_CMD_SET_PLUS_CMSS          = 0x30  */
  BAT_DEF(s,PlusCMUT),        /* BAT_CMD_SET_PLUS_CMUT          = 0x31  */
  BAT_DEF(s,PlusCMUX),        /* BAT_CMD_SET_PLUS_CMUX          = 0x32  */
  BAT_DEF(s,PlusCNMA),        /* BAT_CMD_SET_PLUS_CNMA          = 0x33  */
  BAT_DEF(s,PlusCNMI),        /* BAT_CMD_SET_PLUS_CNMI          = 0x34  */
  BAT_DEF(s,PercentCOPS),     /* BAT_CMD_SET_PERCENT_COPS       = 0x35  */
  BAT_DEF(s,PlusCOPS),        /* BAT_CMD_SET_PLUS_COPS          = 0x36  */
  BAT_DEF(s,PercentCOPS),     /* BAT_CMD_SET_PERCENT_COPS_W     = 0x37  */
#ifdef FF_CPHS    
  BAT_DEF(s,PercentCPALS),    /* BAT_CMD_SET_PERCENT_CPALS      = 0x38  */
  BAT_DEF(s,PercentCPALS),    /* BAT_CMD_SET_PERCENT_CPALS_W    = 0x39  */
#endif
  BAT_DEF(s,PlusCPBF),        /* BAT_CMD_SET_PLUS_CPBF          = 0x3a  */
  BAT_DEF(s,PlusCPBF),        /* BAT_CMD_SET_PLUS_CPBF_W        = 0x3b  */
  BAT_DEF(s,PlusCPBR),        /* BAT_CMD_SET_PLUS_CPBR          = 0x3c  */
  BAT_DEF(s,PlusCPBR),        /* BAT_CMD_SET_PLUS_CPBR_W        = 0x3d  */
  BAT_DEF(s,PlusCPBS),        /* BAT_CMD_SET_PLUS_CPBS          = 0x3e  */
  BAT_DEF(s,PlusCPBW),        /* BAT_CMD_SET_PLUS_CPBW          = 0x3f  */
  BAT_DEF(s,PlusCPBW),        /* BAT_CMD_SET_PLUS_CPBW_W        = 0x40  */
#ifdef FF_CPHS 
  BAT_DEF(s,PercentCPCFU),    /* BAT_CMD_SET_PERCENT_CPCFU      = 0x41  */
  BAT_DEF(s,PercentCPHS),     /* BAT_CMD_SET_PERCENT_CPHS       = 0x42  */
#endif
  BAT_DEF(s,PlusCPIN),        /* BAT_CMD_SET_PLUS_CPIN          = 0x43  */
#ifdef FF_CPHS
  BAT_DEF(s,PercentCPMB),     /* BAT_CMD_SET_PERCENT_CPMB       = 0x44  */
  BAT_DEF(s,PercentCPMBW),    /* BAT_CMD_SET_PERCENT_CPMBW      = 0x45  */
  BAT_DEF(s,PercentCPMBW),    /* BAT_CMD_SET_PERCENT_CPMBW_W    = 0x46  */
  BAT_DEF(s,PercentCPMB),     /* BAT_CMD_SET_PERCENT_CPMB_W     = 0x47  */
#endif
  BAT_DEF(s,PlusCPMS),        /* BAT_CMD_SET_PLUS_CPMS          = 0x48  */
#ifdef FF_CPHS 
  BAT_DEF(s,PercentCPNUMS),   /* BAT_CMD_SET_PERCENT_CPNUMS     = 0x49  */
  BAT_DEF(s,PercentCPNUMS),   /* BAT_CMD_SET_PERCENT_CPNUMS_W   = 0x4a  */
#endif
  BAT_DEF(s,PlusCPOL),        /* BAT_CMD_SET_PLUS_CPOL          = 0x4b  */
  BAT_DEF(s,PlusCPOL),        /* BAT_CMD_SET_PLUS_CPOL_W        = 0x4c  */
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
  BAT_DEF(s,PercentCPRSM),    /* BAT_CMD_SET_PERCENT_CPRSM      = 0x4d  */
#else
  {NULL},
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
  BAT_DEF(s,PlusCPUC),        /* BAT_CMD_SET_PLUS_CPUC          = 0x4e  */
  BAT_DEF(s,PlusCPUC),        /* BAT_CMD_SET_PLUS_CPUC_W        = 0x4f  */
#ifdef FF_CPHS
  BAT_DEF(s,PercentCPVWI),    /* BAT_CMD_SET_PERCENT_CPVWI      = 0x50  */
#endif
  BAT_DEF(s,PlusCPWD),        /* BAT_CMD_SET_PLUS_CPWD          = 0x51  */
  BAT_DEF(s,PlusCRES),        /* BAT_CMD_SET_PLUS_CRES          = 0x52  */
  BAT_DEF(s,PlusCRLP),        /* BAT_CMD_SET_PLUS_CRLP          = 0x53  */
  BAT_DEF(s,PlusCRSM),        /* BAT_CMD_SET_PLUS_CRSM          = 0x54  */
  BAT_DEF(s,PlusCSAS),        /* BAT_CMD_SET_PLUS_CSAS          = 0x55  */
  BAT_DEF(s,PlusCSCA),        /* BAT_CMD_SET_PLUS_CSCA          = 0x56  */
  BAT_DEF(s,PlusCSCB),        /* BAT_CMD_SET_PLUS_CSCB          = 0x57  */
  BAT_DEF(s,PlusCSCS),        /* BAT_CMD_SET_PLUS_CSCS          = 0x58  */
  BAT_DEF(s,PlusCSIM),        /* BAT_CMD_SET_PLUS_CSIM          = 0x59  */
  BAT_DEF(s,PlusCSMS),        /* BAT_CMD_SET_PLUS_CSMS          = 0x5a  */
  BAT_DEF(s,PlusCSNS),        /* BAT_CMD_SET_PLUS_CSNS          = 0x5b  */
  BAT_DEF(s,PlusCSTA),        /* BAT_CMD_SET_PLUS_CSTA          = 0x5c  */
  BAT_DEF(s,PlusCSVM),        /* BAT_CMD_SET_PLUS_CSVM          = 0x5d  */
  BAT_DEF(s,PlusCTFR),        /* BAT_CMD_SET_PLUS_CTFR          = 0x5e  */
  BAT_DEF(s,PercentCTTY),     /* BAT_CMD_SET_PERCENT_CTTY       = 0x5f  */
  BAT_DEF(s,PlusCTZU),        /* BAT_CMD_SET_PLUS_CTZU          = 0x60  */
  BAT_DEF(s,PlusCUSD),        /* BAT_CMD_SET_PLUS_CUSD          = 0x61  */
  BAT_DEF(s,PlusCUSD),        /* BAT_CMD_SET_PLUS_CUSD_W        = 0x62  */
#ifdef TI_PS_FF_AT_P_CMD_CUST
  BAT_DEF(s,PercentCUST),     /* BAT_CMD_SET_PERCENT_CUST       = 0x63  */
#else
  {NULL},
#endif /* TI_PS_FF_AT_P_CMD_CUST */
  BAT_DEF(s,PercentCWUP),     /* BAT_CMD_SET_PERCENT_CWUP       = 0x64  */
  BAT_BAS(D),                 /* BAT_CMD_AT_D                   = 0x65  */
  BAT_DEF(s,PercentDATA),     /* BAT_CMD_SET_PERCENT_DATA       = 0x66  */
  BAT_DEF(s,PercentDINF),     /* BAT_CMD_SET_PERCENT_DINF       = 0x67  */
  BAT_DEF(s,PlusDS),          /* BAT_CMD_SET_PLUS_DS            = 0x68  */
  BAT_BAS(D),                 /* BAT_CMD_AT_D_W                 = 0x69  */
  BAT_DEF(s,PercentEFRSLT),   /* BAT_CMD_SET_PERCENT_EFRSLT     = 0x6a  */
  BAT_DEF(s,PercentEM),       /* BAT_CMD_SET_PERCENT_EM         = 0x6b  */
  BAT_FAD(s,PlusFAP),         /* BAT_CMD_SET_PLUS_FAP           = 0x6c  */
  BAT_FAD(s,PlusFBO),         /* BAT_CMD_SET_PLUS_FBO           = 0x6d  */
  BAT_FAD(s,PlusFBU),         /* BAT_CMD_SET_PLUS_FBU           = 0x6e  */
  BAT_FAD(s,PlusFCC),         /* BAT_CMD_SET_PLUS_FCC           = 0x6f  */
  BAT_FAD(s,PlusFCLASS),      /* BAT_CMD_SET_PLUS_FCLASS        = 0x70  */
  BAT_FAD(s,PlusFCQ),         /* BAT_CMD_SET_PLUS_FCQ           = 0x71  */
  BAT_FAD(s,PlusFCR),         /* BAT_CMD_SET_PLUS_FCR           = 0x72  */
  BAT_FAD(s,PlusFCT),         /* BAT_CMD_SET_PLUS_FCT           = 0x73  */
  BAT_FAD(s,PlusFEA),         /* BAT_CMD_SET_PLUS_FEA           = 0x74  */
  BAT_FAD(s,PlusFFC),         /* BAT_CMD_SET_PLUS_FFC           = 0x75  */
  BAT_FAD(s,PlusFIE),         /* BAT_CMD_SET_PLUS_FIE           = 0x76  */
  BAT_FAD(s,PlusFIS),         /* BAT_CMD_SET_PLUS_FIS           = 0x77  */
  BAT_FAD(s,PlusFIT),         /* BAT_CMD_SET_PLUS_FIT           = 0x78  */
  BAT_FAD(s,PlusFLI),         /* BAT_CMD_SET_PLUS_FLI           = 0x79  */
  BAT_FAD(s,PlusFLO),         /* BAT_CMD_SET_PLUS_FLO           = 0x7a  */
  BAT_FAD(s,PlusFLP),         /* BAT_CMD_SET_PLUS_FLP           = 0x7b  */
  BAT_FAD(s,PlusFMS),         /* BAT_CMD_SET_PLUS_FMS           = 0x7c  */
  BAT_FAD(s,PlusFNS),         /* BAT_CMD_SET_PLUS_FNS           = 0x7d  */
  BAT_FAD(s,PlusFPA),         /* BAT_CMD_SET_PLUS_FPA           = 0x7e  */
  BAT_FAD(s,PlusFPI),         /* BAT_CMD_SET_PLUS_FPI           = 0x7f  */
  BAT_FAD(s,PlusFPS),         /* BAT_CMD_SET_PLUS_FPS           = 0x80  */
  BAT_FAD(s,PlusFPW),         /* BAT_CMD_SET_PLUS_FPW           = 0x81  */
  BAT_FAD(s,PlusFRQ),         /* BAT_CMD_SET_PLUS_FRQ           = 0x82  */
  BAT_FAD(s,PlusFSA),         /* BAT_CMD_SET_PLUS_FSA           = 0x83  */
  BAT_FAD(s,PlusFSP),         /* BAT_CMD_SET_PLUS_FSP           = 0x84  */
  BAT_DEF(s,PlusICF),         /* BAT_CMD_SET_PLUS_ICF           = 0x85  */
  BAT_DEF(s,PlusIFC),         /* BAT_CMD_SET_PLUS_IFC           = 0x86  */
  BAT_DEF(s,PlusIPR),         /* BAT_CMD_SET_PLUS_IPR           = 0x87  */
  BAT_DEF(s,PercentPBCF),     /* BAT_CMD_SET_PERCENT_PBCF       = 0x88  */
  BAT_PPP(s,PercentPPP),      /* BAT_CMD_SET_PERCENT_PPP        = 0x89  */
  BAT_DEF(s,PercentPVRF),     /* BAT_CMD_SET_PERCENT_PVRF       = 0x8a  */
  BAT_DEF(s,PercentRDL),      /* BAT_CMD_SET_PERCENT_RDL        = 0x8b  */
#ifdef TI_PS_FF_AT_P_CMD_RDLB
  BAT_DEF(s,PercentRDLB),     /* BAT_CMD_SET_PERCENT_RDLB       = 0x8c  */
#else
  {NULL},
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
  BAT_STK(s,PercentSATC),     /* BAT_CMD_SET_PERCENT_SATC       = 0x8d  */
  BAT_STK(s,PercentSATCC),    /* BAT_CMD_SET_PERCENT_SATCC      = 0x8e  */
  BAT_STK(s,PercentSATE),     /* BAT_CMD_SET_PERCENT_SATE       = 0x8f  */
  BAT_STK(s,PercentSATR),     /* BAT_CMD_SET_PERCENT_SATR       = 0x80  */
  BAT_STK(s,PercentSATT),     /* BAT_CMD_SET_PERCENT_SATT       = 0x91  */
  BAT_GPR(s,PercentSNCNT),    /* BAT_CMD_SET_PERCENT_SNCNT      = 0x92  */
  BAT_DEF(s,PercentVTS),      /* BAT_CMD_SET_PERCENT_VTS        = 0x93  */
  BAT_DEF(s,PlusVTS),         /* BAT_CMD_SET_PLUS_VTS           = 0x94  */
#ifdef TI_PS_FF_AT_CMD_WS46
  BAT_DEF(s,PlusWS46),        /* BAT_CMD_SET_PLUS_WS46          = 0x95  */
#else
  {NULL},
#endif /* TI_PS_FF_AT_CMD_WS46 */
#ifdef FF_CPHS
  BAT_DEF(s,PercentCPINF),    /* BAT_CMD_SET_PERCENT_CPINF      = 0x96  */
#endif
  BAT_DEF(s,PercentCMGR),     /* BAT_CMD_SET_PERCENT_CMGR       = 0x97  */
  BAT_DEF(s,PercentCMGL),      /* BAT_CMD_SET_PERCENT_CMGL       = 0x98  */
  BAT_DEF(s,PercentCTZV),       /* BAT_CMD_SET_PERCENT_CTZV       = 0x99  */
  BAT_DEF(s,PercentECC),       /* BAT_CMD_SET_PERCENT_ECC        = 0x9a  */
  BAT_SLK(s,PercentMEPD),      /* BAT_CMD_SET_PERCENT_MEPD       = 0x9b  */
  BAT_DEF(s,PercentCSQ),        /* BAT_CMD_SET_PERCENT_CSQ        = 0x9c  */
  BAT_DEF(s,PercentCNIV),       /* BAT_CMD_SET_PERCENT_CNIV       = 0x9d  */
#ifdef REL99
  BAT_DEF(s,PercentCMGRS),     /* BAT_CMD_SET_PERCENT_CMGRS       = 0x9e  */
  BAT_GPR(s,PlusCGCMOD),        /* BAT_CMD_SET_PLUS_CGDSCONT      = 0x9f  */
  BAT_GPR(s,PlusCGDSCONT),      /* BAT_CMD_SET_PLUS_CGDSCONT      = 0xA0  */
  BAT_GPR(s,PlusCGEQREQ),      /* BAT_CMD_SET_PLUS_CGEQREQ        = 0xA1 */
  BAT_GPR(s,PlusCGEQMIN),       /* BAT_CMD_SET_PLUS_CGEQMIN       = 0xA2  */
  BAT_GPR(s,PlusCGEQNEG),       /* BAT_CMD_SET_PLUS_CGEQNEG       = 0xA3  */
  BAT_GPR(s,PlusCGTFT),        /* BAT_CMD_SET_PLUS_CGTFT       = 0xA4  */
#endif
  BAT_DEF(s,PercentSIMEF)       /* BAT_CMD_SET_PERCENT_SIMEF       = 0xA7  */
};

LOCAL const BATCommand bat_cmds_without_params [] =
{
  BAT_BAS(A),                 /* BAT_CMD_AT_A                   = 0x1000 */
  BAT_DEF(s,PlusCBC),         /* BAT_CMD_SET_PLUS_CBC           = 0x1001 */
  BAT_DEF(s,PlusCEER),        /* BAT_CMD_SET_PLUS_CEER          = 0x1002 */
  BAT_DEF(s,PlusCIMI),        /* BAT_CMD_SET_PLUS_CIMI          = 0x1003 */
  BAT_DEF(s,PlusCNUM),        /* BAT_CMD_SET_PLUS_CNUM          = 0x1004 */
  BAT_DEF(s,PlusCNUM),        /* BAT_CMD_SET_PLUS_CNUM_W        = 0x1005 */
  BAT_DEF(s,PlusCPAS),        /* BAT_CMD_SET_PLUS_CPAS          = 0x1006 */
  BAT_DEF(s,PlusCSQ),         /* BAT_CMD_SET_PLUS_CSQ           = 0x1007 */
  BAT_BAS(ANDF),              /* BAT_CMD_ANDF                   = 0x1008 */
  BAT_BAS(H),                 /* BAT_CMD_AT_H                   = 0x1009 */
  BAT_BAS(O),                 /* BAT_CMD_AT_O                   = 0x100a */
  BAT_BAS(Z),                 /* BAT_CMD_AT_Z                   = 0x100b */
  BAT_DEF(q,PercentALS),      /* BAT_CMD_QUE_PERCENT_ALS        = 0x100c */
  BAT_DEF(t,PercentALS),      /* BAT_CMD_TST_PERCENT_ALS        = 0x100d */
#ifdef TI_PS_FF_AT_P_CMD_ATR
  BAT_DEF(q,PercentATR),      /* BAT_CMD_QUE_PERCENT_ATR        = 0x100e */
#else
  {NULL},
#endif /* TI_PS_FF_AT_P_CMD_ATR */
  BAT_DEF(q,PercentBAND),     /* BAT_CMD_QUE_PERCENT_BAND       = 0x100f */
  BAT_DEF(t,PercentBAND),     /* BAT_CMD_TST_PERCENT_BAND       = 0x1010 */
  BAT_DEF(q,PlusCACM),        /* BAT_CMD_QUE_PLUS_CACM          = 0x1011 */
  BAT_DEF(q,PlusCAMM),        /* BAT_CMD_QUE_PLUS_CAMM          = 0x1012 */
  BAT_DEF(q,PercentCBHZ),     /* BAT_CMD_QUE_PERCENT_CBHZ       = 0x1013 */
  BAT_DEF(t,PercentCBHZ),     /* BAT_CMD_TST_PERCENT_CBHZ       = 0x1014 */
#ifdef FAX_AND_DATA
  BAT_DEF(q,PlusCBST),        /* BAT_CMD_QUE_PLUS_CBST          = 0x1015 */
#else
  {NULL},
#endif /* FAX_AND_DATA */
  BAT_DEF(q,PercentCCBS),     /* BAT_CMD_QUE_PERCENT_CCBS       = 0x1016 */
  BAT_DEF(q,PlusCCLK),        /* BAT_CMD_QUE_PLUS_CCLK          = 0x1017 */
  BAT_DEF(q,PlusCCUG),        /* BAT_CMD_QUE_PLUS_CCUG          = 0x1018 */
  BAT_DEF(q,PlusCFUN),        /* BAT_CMD_QUE_PLUS_CFUN          = 0x1019 */
  BAT_GPR(q,PercentCGAATT),   /* BAT_CMD_QUE_PERCENT_CGAATT     = 0x101a */
  BAT_GPR(q,PlusCGACT),       /* BAT_CMD_QUE_PLUS_CGACT         = 0x101b */
  BAT_GPR(q,PlusCGATT),       /* BAT_CMD_QUE_PLUS_CGATT         = 0x101c */
  BAT_GPR(q,PercentCGCLASS),  /* BAT_CMD_QUE_PERCENT_CGCLASS    = 0x101d */
  BAT_GPR(q,PlusCGCLASS),     /* BAT_CMD_QUE_PLUS_CGCLASS       = 0x101e */
  BAT_GPR(q,PlusCGDCONT),     /* BAT_CMD_QUE_PLUS_CGDCONT       = 0x101f */
  BAT_GPR(t,PlusCGPADDR),     /* BAT_CMD_TST_PLUS_CGPADDR       = 0x1020 */
  BAT_GPR(q,PercentCGPPP),    /* BAT_CMD_QUE_PERCENT_CGPPP      = 0x1021 */
  BAT_GPR(q,PlusCGQMIN),      /* BAT_CMD_QUE_PLUS_CGQMIN        = 0x1022 */
  BAT_GPR(q,PlusCGQREQ),      /* BAT_CMD_QUE_PLUS_CGQREQ        = 0x1023 */
  BAT_GPR(q,PercentCGREG),    /* BAT_CMD_QUE_PERCENT_CGREG      = 0x1024 */
  BAT_GPR(q,PlusCGREG),       /* BAT_CMD_QUE_PLUS_CGREG         = 0x1025 */
  BAT_GPR(q,PlusCGSMS),       /* BAT_CMD_QUE_PLUS_CGSMS         = 0x1026 */
  BAT_DEF(q,PlusCIND),        /* BAT_CMD_QUE_PLUS_CIND          = 0x1027 */
  BAT_DEF(q,PlusCLAN),        /* BAT_CMD_QUE_PLUS_CLAN          = 0x1028 */
  BAT_DEF(t,PlusCLAN),        /* BAT_CMD_TST_PLUS_CLAN          = 0x1029 */
  BAT_DEF(q,PlusCLCC),        /* BAT_CMD_QUE_PLUS_CLCC          = 0x102a */
  BAT_DEF(q,PlusCLIP),        /* BAT_CMD_QUE_PLUS_CLIP          = 0x102b */
  BAT_DEF(q,PlusCLIP),        /* BAT_CMD_QUE_PLUS_CLIP_W        = 0x102c */
  BAT_DEF(q,PlusCLIR),        /* BAT_CMD_QUE_PLUS_CLIR          = 0x102d */
  BAT_DEF(q,PlusCLVL),        /* BAT_CMD_QUE_PLUS_CLVL          = 0x102e */
  BAT_DEF(q,PlusCMER),        /* BAT_CMD_QUE_PLUS_CMER          = 0x102f */
  BAT_DEF(q,PlusCMOD),        /* BAT_CMD_QUE_PLUS_CMOD          = 0x1030 */
  BAT_DEF(q,PlusCMUT),        /* BAT_CMD_QUE_PLUS_CMUT          = 0x1031 */
  BAT_DEF(q,PlusCMUX),        /* BAT_CMD_QUE_PLUS_CMUX          = 0x1032 */
  BAT_DEF(q,PercentCNAP),     /* BAT_CMD_QUE_PERCENT_CNAP       = 0x1033 */
  BAT_DEF(q,PlusCNMI),        /* BAT_CMD_QUE_PLUS_CNMI          = 0x1034 */
  BAT_DEF(q,PlusCOLP),        /* BAT_CMD_QUE_PLUS_COLP          = 0x1035 */
  BAT_DEF(q,PlusCOLP),        /* BAT_CMD_QUE_PLUS_COLP_W        = 0x1036 */
  BAT_DEF(q,PercentCOPS),     /* BAT_CMD_QUE_PERCENT_COPS       = 0x1037 */
  BAT_DEF(t,PercentCOPS),     /* BAT_CMD_TST_PERCENT_COPS       = 0x1038 */
  BAT_DEF(q,PlusCOPS),        /* BAT_CMD_QUE_PLUS_COPS          = 0x1039 */
  BAT_DEF(t,PlusCOPS),        /* BAT_CMD_TST_PLUS_COPS          = 0x103a */
  BAT_DEF(q,PercentCOPS),     /* BAT_CMD_QUE_PERCENT_COPS_W     = 0x103b */
  BAT_DEF(t,PercentCOPS),     /* BAT_CMD_TST_PERCENT_COPS_W     = 0x103c */
#ifdef FF_CPHS
  BAT_DEF(q,PercentCPALS),    /* BAT_CMD_QUE_PERCENT_CPALS      = 0x103d */
  BAT_DEF(q,PercentCPALS),    /* BAT_CMD_QUE_PERCENT_CPALS_W    = 0x103e */
#endif  	
  BAT_DEF(t,PlusCPBF),        /* BAT_CMD_TST_PLUS_CPBF          = 0x103f */
  BAT_DEF(t,PlusCPBF),        /* BAT_CMD_TST_PLUS_CPBF_W        = 0x1040 */
  BAT_DEF(t,PlusCPBR),        /* BAT_CMD_TST_PLUS_CPBR          = 0x1041 */
  BAT_DEF(t,PlusCPBR),        /* BAT_CMD_TST_PLUS_CPBR_W        = 0x1042 */
  BAT_DEF(q,PercentCPBS),     /* BAT_CMD_QUE_PERCENT_CPBS       = 0x1043 */  
  BAT_DEF(q,PlusCPBS),        /* BAT_CMD_QUE_PLUS_CPBS          = 0x1044 */
  BAT_DEF(t,PlusCPBW),        /* BAT_CMD_TST_PLUS_CPBW          = 0x1045 */
  BAT_DEF(t,PlusCPBW),        /* BAT_CMD_TST_PLUS_CPBW_W        = 0x1046 */
  #ifdef FF_CPHS			  //fix x0083023
  BAT_DEF(q,PercentCPHS),     /* BAT_CMD_QUE_PERCENT_CPHS       = 0x1047 */
  #endif
  BAT_DEF(q,PlusCPIN),        /* BAT_CMD_QUE_PLUS_CPIN          = 0x1048 */
#ifdef FF_CPHS
  BAT_DEF(q,PercentCPINF),    /* BAT_CMD_QUE_PERCENT_CPINF      = 0x1049 */
  BAT_DEF(q,PercentCPMB),     /* BAT_CMD_QUE_PERCENT_CPMB       = 0x104a */
  BAT_DEF(t,PercentCPMBW),    /* BAT_CMD_TST_PERCENT_CPMBW      = 0x104b */
  BAT_DEF(t,PercentCPMBW),    /* BAT_CMD_TST_PERCENT_CPMBW_W    = 0x104c */
#endif
  BAT_DEF(q,PlusCPMS),        /* BAT_CMD_QUE_PLUS_CPMS          = 0x104d */
#ifdef FF_CPHS
  BAT_DEF(t,PercentCPNUMS),   /* BAT_CMD_TST_PERCENT_CPNUMS     = 0x104e */
  BAT_DEF(t,PercentCPNUMS),   /* BAT_CMD_TST_PERCENT_CPNUMS_W   = 0x104f */
#endif
  BAT_DEF(q,PlusCPOL),        /* BAT_CMD_QUE_PLUS_CPOL          = 0x1050 */
  BAT_DEF(t,PlusCPOL),        /* BAT_CMD_TST_PLUS_CPOL          = 0x1051 */
  BAT_DEF(q,PlusCPOL),        /* BAT_CMD_QUE_PLUS_CPOL_W        = 0x1052 */
  BAT_DEF(t,PlusCPOL),        /* BAT_CMD_TST_PLUS_CPOL_W        = 0x1053 */
#ifdef FF_CPHS 
  BAT_DEF(q,PercentCPOPN),    /* BAT_CMD_QUE_PERCENT_CPOPN      = 0x1054 */
#endif
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
  BAT_DEF(q,PercentCPRSM),    /* BAT_CMD_QUE_PERCENT_CPRSM      = 0x1055 */
#else
  {NULL},
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
  BAT_DEF(q,PlusCPUC),        /* BAT_CMD_QUE_PLUS_CPUC          = 0x1056 */
  BAT_DEF(q,PlusCPUC),        /* BAT_CMD_QUE_PLUS_CPUC_W        = 0x1057 */
  BAT_DEF(q,PercentCREG),     /* BAT_CMD_QUE_PERCENT_CREG       = 0x1058 */
  BAT_DEF(t,PlusCRES),        /* BAT_CMD_TST_PLUS_CRES          = 0x1059 */
  BAT_DEF(q,PlusCRLP),        /* BAT_CMD_QUE_PLUS_CRLP          = 0x105a */
  BAT_DEF(t,PlusCSAS),        /* BAT_CMD_TST_PLUS_CSAS          = 0x105b */
  BAT_DEF(q,PlusCSCA),        /* BAT_CMD_QUE_PLUS_CSCA          = 0x105c */
  BAT_DEF(q,PlusCSCB),        /* BAT_CMD_QUE_PLUS_CSCB          = 0x105d */
  BAT_DEF(q,PlusCSCS),        /* BAT_CMD_QUE_PLUS_CSCS          = 0x105e */
  BAT_DEF(q,PlusCSMS),        /* BAT_CMD_QUE_PLUS_CSMS          = 0x105f */
  BAT_DEF(q,PlusCSNS),        /* BAT_CMD_QUE_PLUS_CSNS          = 0x1060 */
  BAT_DEF(q,PercentCSQ),      /* BAT_CMD_QUE_PERCENT_CSQ        = 0x1061 */
  BAT_DEF(q,PlusCSTA),        /* BAT_CMD_QUE_PLUS_CSTA          = 0x1062 */
  BAT_DEF(q,PlusCSVM),        /* BAT_CMD_QUE_PLUS_CSVM          = 0x1063 */
  BAT_DEF(q,PercentCTTY),     /* BAT_CMD_QUE_PERCENT_CTTY       = 0x1064 */
  BAT_DEF(q,PlusCTZU),        /* BAT_CMD_QUE_PLUS_CTZU          = 0x1065 */
#ifdef TI_PS_FF_AT_P_CMD_CUST
  BAT_DEF(q,PercentCUST),     /* BAT_CMD_QUE_PERCENT_CUST       = 0x1066 */
#else
  {NULL},
#endif /* TI_PS_FF_AT_P_CMD_CUST */
  BAT_DEF(q,PercentDATA),     /* BAT_CMD_QUE_PERCENT_DATA       = 0x1067 */
  BAT_DEF(q,PlusDS),          /* BAT_CMD_QUE_PLUS_DS            = 0x1068 */
  BAT_FAD(q,PlusFAP),         /* BAT_CMD_QUE_PLUS_FAP           = 0x1069 */
  BAT_FAD(q,PlusFBO),         /* BAT_CMD_QUE_PLUS_FBO           = 0x106a */
  BAT_FAD(q,PlusFBS),         /* BAT_CMD_QUE_PLUS_FBS           = 0x106b */
  BAT_FAD(q,PlusFBU),         /* BAT_CMD_QUE_PLUS_FBU           = 0x106c */
  BAT_FAD(q,PlusFCC),         /* BAT_CMD_QUE_PLUS_FCC           = 0x106d */
  BAT_FAD(q,PlusFCLASS),      /* BAT_CMD_QUE_PLUS_FCLASS        = 0x106e */
  BAT_FAD(q,PlusFCQ),         /* BAT_CMD_QUE_PLUS_FCQ           = 0x106f */
  BAT_FAD(q,PlusFCR),         /* BAT_CMD_QUE_PLUS_FCR           = 0x1070 */
  BAT_FAD(q,PlusFCS),         /* BAT_CMD_QUE_PLUS_FCS           = 0x1071 */
  BAT_FAD(q,PlusFCT),         /* BAT_CMD_QUE_PLUS_FCT           = 0x1072 */
  {NULL},                               /* BAT_CMD_QUE_PLUS_FEA           = 0x1073 */
  BAT_FAD(q,PlusFFC),         /* BAT_CMD_QUE_PLUS_FFC           = 0x1074 */
  BAT_FAD(q,PlusFHS),         /* BAT_CMD_QUE_PLUS_FHS           = 0x1075 */
  BAT_FAD(q,PlusFIE),         /* BAT_CMD_QUE_PLUS_FIE           = 0x1076 */
  BAT_FAD(q,PlusFIS),         /* BAT_CMD_QUE_PLUS_FIS           = 0x1077 */
  BAT_FAD(q,PlusFIT),         /* BAT_CMD_QUE_PLUS_FIT           = 0x1078 */
  BAT_FAD(q,PlusFLI),         /* BAT_CMD_QUE_PLUS_FLI           = 0x1079 */
  BAT_FAD(q,PlusFLO),         /* BAT_CMD_QUE_PLUS_FLO           = 0x107a */
  BAT_FAD(q,PlusFLP),         /* BAT_CMD_QUE_PLUS_FLP           = 0x107b */
  BAT_FAD(q,PlusFMS),         /* BAT_CMD_QUE_PLUS_FMS           = 0x107c */
  BAT_FAD(q,PlusFNS),         /* BAT_CMD_QUE_PLUS_FNS           = 0x107d */
  BAT_FAD(q,PlusFPA),         /* BAT_CMD_QUE_PLUS_FPA           = 0x107e */
  BAT_FAD(q,PlusFPI),         /* BAT_CMD_QUE_PLUS_FPI           = 0x107f */
  BAT_FAD(q,PlusFPS),         /* BAT_CMD_QUE_PLUS_FPS           = 0x1080 */
  BAT_FAD(q,PlusFPW),         /* BAT_CMD_QUE_PLUS_FPW           = 0x1081 */
  BAT_FAD(q,PlusFRQ),         /* BAT_CMD_QUE_PLUS_FRQ           = 0x1082 */
  BAT_FAD(q,PlusFSA),         /* BAT_CMD_QUE_PLUS_FSA           = 0x1083 */
  BAT_FAD(q,PlusFSP),         /* BAT_CMD_QUE_PLUS_FSP           = 0x1084 */
  BAT_DEF(q,PlusICF),         /* BAT_CMD_QUE_PLUS_ICF           = 0x1085 */
  BAT_DEF(q,PlusIFC),         /* BAT_CMD_QUE_PLUS_IFC           = 0x1086 */
  {NULL},                              /* BAT_CMD_QUE_PERCENT_IMEI       = 0x1087 */
  BAT_DEF(q,PlusIPR),         /* BAT_CMD_QUE_PLUS_IPR           = 0x1088 */
  BAT_DEF(q,PercentPBCF),     /* BAT_CMD_QUE_PERCENT_PBCF       = 0x1089 */  
  BAT_PPP(q,PercentPPP),      /* BAT_CMD_QUE_PERCENT_PPP        = 0x108a */
  BAT_DEF(q,PercentPVRF),     /* BAT_CMD_QUE_PERCENT_PVRF       = 0x108b */
  BAT_DEF(q,PercentRDL),      /* BAT_CMD_QUE_PERCENT_RDL        = 0x108c */
#ifdef TI_PS_FF_AT_P_CMD_RDLB
  BAT_DEF(q,PercentRDLB),     /* BAT_CMD_QUE_PERCENT_RDLB       = 0x108d */
#else
  {NULL},
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
  BAT_STK(q,PercentSATC),     /* BAT_CMD_QUE_PERCENT_SATC       = 0x108e */
  BAT_STK(q,PercentSATCC),    /* BAT_CMD_QUE_PERCENT_SATCC      = 0x108f */
  BAT_GPR(q,PercentSNCNT),    /* BAT_CMD_QUE_PERCENT_SNCNT      = 0x1090 */
#ifdef TI_PS_FF_AT_CMD_WS46
  BAT_DEF(q,PlusWS46),        /* BAT_CMD_QUE_PLUS_WS46          = 0x1091 */
#else
  {NULL},
#endif /* TI_PS_FF_AT_CMD_WS46 */
  BAT_DEF(q,PlusCDIP),         /* BAT_CMD_QUE_PLUS_CDIP          = 0x1092 */
  BAT_DEF(s,PlusCGMI),        /* BAT_CMD_SET_PLUS_CGMI         = 0x1093 */
  BAT_DEF(s,PlusCGMM),        /* BAT_CMD_SET_PLUS_CGMM          = 0x1094 */
  BAT_DEF(s,PlusCGMR),        /* BAT_CMD_SET_PLUS_CGMR          = 0x1095 */
  BAT_DEF(s,PlusCGSN),        /* BAT_CMD_SET_PLUS_CGSN          = 0x1096 */
  BAT_DEF(q,PercentCNIV),      /* BAT_CMD_QUE_PERCENT_CNIV       = 0x1097 */
  {NULL},                     /* BAT_CMD_QUE_PERCENT_CMGRS       = 0x1098 */
  {NULL},                     /*BAT_DEF(q,PercentPBCI),       BAT_CMD_QUE_PERCENT_PBCI        = 0x1099 */
  BAT_DEF(q,PercentSIMEF),    /*BAT_CMD_QUE_PERCENT_SIMEF        = 0x109A */
  BAT_DEF(s,PlusCHUP)             /*BAT_CMD_SET_PLUS_CHUP            = 0x109B */
};


typedef struct
{
  size_t size;
} T_map_response_2_size;

static T_map_response_2_size response_2_size[] =
{
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_AT_OK                  = 0x0    */       
  {sizeof(T_BAT_res_at_connect)},          /* BAT_RES_AT_CONNECT             = 0x1    */       
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_AT_NO_CARRIER_FINAL    = 0x2    */       
  {sizeof(T_BAT_res_plus_cme_error)},      /* BAT_RES_PLUS_CME_ERROR         = 0x3    */       
  {sizeof(T_BAT_res_plus_cms_error)},      /* BAT_RES_PLUS_CMS_ERROR         = 0x4    */       
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_AT_BUSY                = 0x5    */       
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_AT_CHANNEL_BUSY        = 0x6    */       
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_AT_NO_ANSWER           = 0x7    */       
  {sizeof(T_BAT_res_plus_ext_error)},      /* BAT_RES_PLUS_EXT_ERROR         = 0x8    */       
  {sizeof(T_BAT_res_que_percent_als)},     /* BAT_RES_QUE_PERCENT_ALS        = 0x9    */       
  {sizeof(T_BAT_res_tst_percent_als)},     /* BAT_RES_TST_PERCENT_ALS        = 0xa    */       
  {sizeof(T_BAT_res_que_percent_atr)},     /* BAT_RES_QUE_PERCENT_ATR        = 0xb    */       
  {sizeof(T_BAT_res_que_percent_band)},    /* BAT_RES_QUE_PERCENT_BAND       = 0xc    */       
  {sizeof(T_BAT_res_tst_percent_band)},    /* BAT_RES_TST_PERCENT_BAND       = 0xd    */       
  {sizeof(T_BAT_res_que_plus_cacm)},       /* BAT_RES_QUE_PLUS_CACM          = 0xe    */       
  {sizeof(T_BAT_res_que_plus_camm)},       /* BAT_RES_QUE_PLUS_CAMM          = 0xf    */       
  {sizeof(T_BAT_res_set_plus_caoc)},       /* BAT_RES_SET_PLUS_CAOC          = 0x10   */       
  {sizeof(T_BAT_res_set_plus_cbc)},        /* BAT_RES_SET_PLUS_CBC           = 0x11   */       
  {sizeof(T_BAT_res_que_percent_cbhz)},    /* BAT_RES_QUE_PERCENT_CBHZ       = 0x12   */       
  {sizeof(T_BAT_res_tst_percent_cbhz)},    /* BAT_RES_TST_PERCENT_CBHZ       = 0x13   */       
  {sizeof(T_BAT_res_que_plus_cbst)},       /* BAT_RES_QUE_PLUS_CBST          = 0x14   */       
  {sizeof(T_BAT_res_que_percent_ccbs)},    /* BAT_RES_QUE_PERCENT_CCBS       = 0x15   */       
  {sizeof(T_BAT_res_set_plus_ccfc)},       /* BAT_RES_SET_PLUS_CCFC          = 0x16   */       
  {sizeof(T_BAT_res_que_plus_cclk)},       /* BAT_RES_QUE_PLUS_CCLK          = 0x17   */       
  {sizeof(T_BAT_res_que_plus_ccug)},       /* BAT_RES_QUE_PLUS_CCUG          = 0x18   */       
  {sizeof(T_BAT_res_set_plus_ccwa)},       /* BAT_RES_SET_PLUS_CCWA          = 0x19   */       
  {sizeof(T_BAT_res_set_plus_ccwa_w)},     /* BAT_RES_SET_PLUS_CCWA_W        = 0x1a   */       
  {sizeof(T_BAT_res_set_plus_ceer)},       /* BAT_RES_SET_PLUS_CEER          = 0x1b   */       
  {sizeof(T_BAT_res_que_plus_cfun)},       /* BAT_RES_QUE_PLUS_CFUN          = 0x1c   */       
  {sizeof(T_BAT_res_que_percent_cgaatt)},  /* BAT_RES_QUE_PERCENT_CGAATT     = 0x1d   */       
  {sizeof(T_BAT_res_que_plus_cgact)},      /* BAT_RES_QUE_PLUS_CGACT         = 0x1e   */       
  {sizeof(T_BAT_res_que_plus_cgatt)},      /* BAT_RES_QUE_PLUS_CGATT         = 0x1f   */       
  {sizeof(T_BAT_res_que_percent_cgclass)}, /* BAT_RES_QUE_PERCENT_CGCLASS    = 0x20   */       
  {sizeof(T_BAT_res_que_plus_cgclass)},    /* BAT_RES_QUE_PLUS_CGCLASS       = 0x21   */       
  {sizeof(T_BAT_res_que_plus_cgdcont)},    /* BAT_RES_QUE_PLUS_CGDCONT       = 0x22   */       
  {sizeof(T_BAT_res_set_plus_cgpaddr)},    /* BAT_RES_SET_PLUS_CGPADDR       = 0x23   */       
  {sizeof(T_BAT_res_tst_plus_cgpaddr)},    /* BAT_RES_TST_PLUS_CGPADDR       = 0x24   */       
  {sizeof(T_BAT_res_set_percent_cgpco)},   /* BAT_RES_SET_PERCENT_CGPCO      = 0x25   */       
  {sizeof(T_BAT_res_que_percent_cgppp)},   /* BAT_RES_QUE_PERCENT_CGPPP      = 0x26   */       
  {sizeof(T_BAT_res_que_plus_cgqmin)},     /* BAT_RES_QUE_PLUS_CGQMIN        = 0x27   */       
  {sizeof(T_BAT_res_que_plus_cgqreq)},     /* BAT_RES_QUE_PLUS_CGQREQ        = 0x28   */       
  {sizeof(T_BAT_res_que_percent_cgreg)},   /* BAT_RES_QUE_PERCENT_CGREG      = 0x29   */       
  {sizeof(T_BAT_res_que_plus_cgreg)},      /* BAT_RES_QUE_PLUS_CGREG         = 0x2a   */       
  {sizeof(T_BAT_res_que_plus_cgsms)},      /* BAT_RES_QUE_PLUS_CGSMS         = 0x2b   */       
  {sizeof(T_BAT_res_set_percent_chpl)},    /* BAT_RES_SET_PERCENT_CHPL       = 0x2c   */       
  {sizeof(T_BAT_res_set_percent_chpl_w)},  /* BAT_RES_SET_PERCENT_CHPL_W     = 0x2d   */       
  {sizeof(T_BAT_res_set_plus_cimi)},       /* BAT_RES_SET_PLUS_CIMI          = 0x2e   */       
  {sizeof(T_BAT_res_que_plus_cind)},       /* BAT_RES_QUE_PLUS_CIND          = 0x2f   */       
  {sizeof(T_BAT_res_que_plus_clan)},       /* BAT_RES_QUE_PLUS_CLAN          = 0x30   */       
  {sizeof(T_BAT_res_tst_plus_clan)},       /* BAT_RES_TST_PLUS_CLAN          = 0x31   */       
  {sizeof(T_BAT_res_que_plus_clcc)},       /* BAT_RES_QUE_PLUS_CLCC          = 0x32   */       
  {sizeof(T_BAT_res_set_plus_clck)},       /* BAT_RES_SET_PLUS_CLCK          = 0x33   */       
  {sizeof(T_BAT_res_que_plus_clip)},       /* BAT_RES_QUE_PLUS_CLIP          = 0x34   */       
  {sizeof(T_BAT_res_que_plus_clip_w)},     /* BAT_RES_QUE_PLUS_CLIP_W        = 0x35   */       
  {sizeof(T_BAT_res_que_plus_clir)},       /* BAT_RES_QUE_PLUS_CLIR          = 0x36   */       
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_SET_PLUS_CLVL          = 0x37   */       
  {sizeof(T_BAT_res_que_plus_clvl)},       /* BAT_RES_QUE_PLUS_CLVL          = 0x38   */       
  {sizeof(T_BAT_res_que_plus_cmer)},       /* BAT_RES_QUE_PLUS_CMER          = 0x39   */       
  {sizeof(T_BAT_res_set_plus_cmgc)},       /* BAT_RES_SET_PLUS_CMGC          = 0x3a   */       
  {sizeof(T_BAT_res_set_plus_cmgl)},       /* BAT_RES_SET_PLUS_CMGL          = 0x3b   */       
  {sizeof(T_BAT_res_set_plus_cmgl_w)},     /* BAT_RES_SET_PLUS_CMGL_W        = 0x3c   */       
  {sizeof(T_BAT_res_set_plus_cmgr)},       /* BAT_RES_SET_PLUS_CMGR          = 0x3d   */       
  {sizeof(T_BAT_res_set_plus_cmgr_w)},     /* BAT_RES_SET_PLUS_CMGR_W        = 0x3e   */       
  {sizeof(T_BAT_res_set_plus_cmgs)},       /* BAT_RES_SET_PLUS_CMGS          = 0x3f   */       
  {sizeof(T_BAT_res_set_plus_cmgw)},       /* BAT_RES_SET_PLUS_CMGW          = 0x40   */       
  {sizeof(T_BAT_res_que_plus_cmod)},       /* BAT_RES_QUE_PLUS_CMOD          = 0x41   */       
  {sizeof(T_BAT_res_que_plus_cmut)},       /* BAT_RES_QUE_PLUS_CMUT          = 0x42   */       
  {sizeof(T_BAT_res_que_plus_cmux)},       /* BAT_RES_QUE_PLUS_CMUX          = 0x43   */       
  {sizeof(T_BAT_res_que_percent_cnap)},    /* BAT_RES_QUE_PERCENT_CNAP       = 0x44   */       
  {sizeof(T_BAT_res_que_plus_cnmi)},       /* BAT_RES_QUE_PLUS_CNMI          = 0x45   */       
  {sizeof(T_BAT_res_set_plus_cnum)},       /* BAT_RES_SET_PLUS_CNUM          = 0x46   */       
  {sizeof(T_BAT_res_set_plus_cnum_w)},     /* BAT_RES_SET_PLUS_CNUM_W        = 0x47   */       
  {sizeof(T_BAT_res_que_plus_colp)},       /* BAT_RES_QUE_PLUS_COLP          = 0x48   */       
  {sizeof(T_BAT_res_que_plus_colp_w)},     /* BAT_RES_QUE_PLUS_COLP_W        = 0x49   */       
  {sizeof(T_BAT_res_que_percent_cops)},    /* BAT_RES_QUE_PERCENT_COPS       = 0x4a   */       
  {sizeof(T_BAT_res_tst_percent_cops)},    /* BAT_RES_TST_PERCENT_COPS       = 0x4b   */       
  {sizeof(T_BAT_res_que_plus_cops)},       /* BAT_RES_QUE_PLUS_COPS          = 0x4c   */       
  {sizeof(T_BAT_res_tst_plus_cops)},       /* BAT_RES_TST_PLUS_COPS          = 0x4d   */       
  {sizeof(T_BAT_res_que_percent_cops_w)},  /* BAT_RES_QUE_PERCENT_COPS_W     = 0x4e   */       
  {sizeof(T_BAT_res_tst_percent_cops_w)},  /* BAT_RES_TST_PERCENT_COPS_W     = 0x4f   */       
  {sizeof(T_BAT_res_set_percent_cpals)},   /* BAT_RES_SET_PERCENT_CPALS      = 0x50   */       
  {sizeof(T_BAT_res_que_percent_cpals)},   /* BAT_RES_QUE_PERCENT_CPALS      = 0x51   */       
  {sizeof(T_BAT_res_set_percent_cpals_w)}, /* BAT_RES_SET_PERCENT_CPALS_W    = 0x52   */       
  {sizeof(T_BAT_res_que_percent_cpals_w)}, /* BAT_RES_QUE_PERCENT_CPALS_W    = 0x53   */       
  {sizeof(T_BAT_res_set_plus_cpas)},       /* BAT_RES_SET_PLUS_CPAS          = 0x54   */       
  {sizeof(T_BAT_res_set_plus_cpbf)},       /* BAT_RES_SET_PLUS_CPBF          = 0x55   */       
  {sizeof(T_BAT_res_tst_plus_cpbf)},       /* BAT_RES_TST_PLUS_CPBF          = 0x56   */       
  {sizeof(T_BAT_res_set_plus_cpbf_w)},     /* BAT_RES_SET_PLUS_CPBF_W        = 0x57   */       
  {sizeof(T_BAT_res_tst_plus_cpbf_w)},     /* BAT_RES_TST_PLUS_CPBF_W        = 0x58   */       
  {sizeof(T_BAT_res_set_plus_cpbr)},       /* BAT_RES_SET_PLUS_CPBR          = 0x59   */       
  {sizeof(T_BAT_res_tst_plus_cpbr)},       /* BAT_RES_TST_PLUS_CPBR          = 0x5a   */       
  {sizeof(T_BAT_res_set_plus_cpbr_w)},     /* BAT_RES_SET_PLUS_CPBR_W        = 0x5b   */       
  {sizeof(T_BAT_res_tst_plus_cpbr_w)},     /* BAT_RES_TST_PLUS_CPBR_W        = 0x5c   */       
  {sizeof(T_BAT_res_que_percent_cpbs)},    /* BAT_RES_QUE_PERCENT_CPBS       = 0x5d   */         
  {sizeof(T_BAT_res_que_plus_cpbs)},       /* BAT_RES_QUE_PLUS_CPBS          = 0x5e   */       
  {sizeof(T_BAT_res_tst_plus_cpbw)},       /* BAT_RES_TST_PLUS_CPBW          = 0x5f   */       
  {sizeof(T_BAT_res_tst_plus_cpbw_w)},     /* BAT_RES_TST_PLUS_CPBW_W        = 0x60   */       
  {sizeof(T_BAT_res_set_percent_cpcfu)},   /* BAT_RES_SET_PERCENT_CPCFU      = 0x61   */       
  {sizeof(T_BAT_res_que_percent_cphs)},    /* BAT_RES_QUE_PERCENT_CPHS       = 0x62   */       
  {sizeof(T_BAT_res_que_plus_cpin)},       /* BAT_RES_QUE_PLUS_CPIN          = 0x63   */       
  {sizeof(T_BAT_res_que_percent_cpinf)},   /* BAT_RES_QUE_PERCENT_CPINF      = 0x64   */       
  {sizeof(T_BAT_res_set_percent_cpmb)},    /* BAT_RES_SET_PERCENT_CPMB       = 0x65   */        
  {sizeof(T_BAT_res_que_percent_cpmb)},    /* BAT_RES_QUE_PERCENT_CPMB       = 0x66   */        
  {sizeof(T_BAT_res_tst_percent_cpmbw)},   /* BAT_RES_TST_PERCENT_CPMBW      = 0x67   */       
  {sizeof(T_BAT_res_tst_percent_cpmbw_w)}, /* BAT_RES_TST_PERCENT_CPMBW_W    = 0x68   */       
  {sizeof(T_BAT_res_set_percent_cpmb_w)},  /* BAT_RES_SET_PERCENT_CPMB_W     = 0x69   */       
  {sizeof(T_BAT_res_set_plus_cpms)},       /* BAT_RES_SET_PLUS_CPMS          = 0x6a   */       
  {sizeof(T_BAT_res_que_plus_cpms)},       /* BAT_RES_QUE_PLUS_CPMS          = 0x6b   */       
  {sizeof(T_BAT_res_set_percent_cpnums)},  /* BAT_RES_SET_PERCENT_CPNUMS     = 0x6c   */       
  {sizeof(T_BAT_res_tst_percent_cpnums)},  /* BAT_RES_TST_PERCENT_CPNUMS     = 0x6d   */       
  {sizeof(T_BAT_res_set_percent_cpnums_w)},/* BAT_RES_SET_PERCENT_CPNUMS_W   = 0x6e   */       
  {sizeof(T_BAT_res_tst_percent_cpnums_w)},/* BAT_RES_TST_PERCENT_CPNUMS_W   = 0x6f   */       
  {sizeof(T_BAT_res_que_plus_cpol)},       /* BAT_RES_QUE_PLUS_CPOL          = 0x70   */       
  {sizeof(T_BAT_res_tst_plus_cpol)},       /* BAT_RES_TST_PLUS_CPOL          = 0x71   */       
  {sizeof(T_BAT_res_que_plus_cpol_w)},     /* BAT_RES_QUE_PLUS_CPOL_W        = 0x72   */       
  {sizeof(T_BAT_res_tst_plus_cpol_w)},     /* BAT_RES_TST_PLUS_CPOL_W        = 0x73   */       
  {sizeof(T_BAT_res_que_percent_cpopn)},   /* BAT_RES_QUE_PERCENT_CPOPN      = 0x74   */       
  {sizeof(T_BAT_res_que_percent_cprsm)},   /* BAT_RES_QUE_PERCENT_CPRSM      = 0x75   */       
  {sizeof(T_BAT_res_que_plus_cpuc)},       /* BAT_RES_QUE_PLUS_CPUC          = 0x76   */       
  {sizeof(T_BAT_res_que_plus_cpuc_w)},     /* BAT_RES_QUE_PLUS_CPUC_W        = 0x77   */       
  {sizeof(T_BAT_res_set_percent_cpvwi)},   /* BAT_RES_SET_PERCENT_CPVWI      = 0x78   */       
  {sizeof(T_BAT_res_que_percent_creg)},    /* BAT_RES_QUE_PERCENT_CREG       = 0x79   */       
  {sizeof(T_BAT_res_tst_plus_cres)},       /* BAT_RES_TST_PLUS_CRES          = 0x7a   */       
  {sizeof(T_BAT_res_que_plus_crlp)},       /* BAT_RES_QUE_PLUS_CRLP          = 0x7b   */       
  {sizeof(T_BAT_res_set_plus_crsm)},       /* BAT_RES_SET_PLUS_CRSM          = 0x7c   */       
  {sizeof(T_BAT_res_tst_plus_csas)},       /* BAT_RES_TST_PLUS_CSAS          = 0x7d   */       
  {sizeof(T_BAT_res_que_plus_csca)},       /* BAT_RES_QUE_PLUS_CSCA          = 0x7e   */       
  {sizeof(T_BAT_res_que_plus_cscb)},       /* BAT_RES_QUE_PLUS_CSCB          = 0x7f   */       
  {sizeof(T_BAT_res_que_plus_cscs)},       /* BAT_RES_QUE_PLUS_CSCS          = 0x80   */       
  {sizeof(T_BAT_res_set_plus_csim)},       /* BAT_RES_SET_PLUS_CSIM          = 0x81   */       
  {sizeof(T_BAT_res_que_plus_csms)},       /* BAT_RES_QUE_PLUS_CSMS          = 0x82   */       
  {sizeof(T_BAT_res_que_plus_csns)},       /* BAT_RES_QUE_PLUS_CSNS          = 0x83   */       
  {sizeof(T_BAT_res_que_percent_csq)},     /* BAT_RES_QUE_PERCENT_CSQ        = 0x84   */       
  {sizeof(T_BAT_res_set_plus_csq)},        /* BAT_RES_SET_PLUS_CSQ           = 0x85   */       
  {sizeof(T_BAT_res_que_plus_csta)},       /* BAT_RES_QUE_PLUS_CSTA          = 0x86   */       
  {sizeof(T_BAT_res_que_plus_csvm)},       /* BAT_RES_QUE_PLUS_CSVM          = 0x87   */       
  {sizeof(T_BAT_res_que_percent_ctty)},    /* BAT_RES_QUE_PERCENT_CTTY       = 0x88   */       
  {sizeof(T_BAT_res_que_plus_ctzu)},       /* BAT_RES_QUE_PLUS_CTZU          = 0x89   */       
  {sizeof(T_BAT_res_que_percent_cust)},    /* BAT_RES_QUE_PERCENT_CUST       = 0x8a   */       
  {sizeof(T_BAT_res_que_percent_data)},    /* BAT_RES_QUE_PERCENT_DATA       = 0x8b   */       
  {sizeof(T_BAT_res_set_percent_dinf)},    /* BAT_RES_SET_PERCENT_DINF       = 0x8c   */       
  {sizeof(T_BAT_res_que_plus_ds)},         /* BAT_RES_QUE_PLUS_DS            = 0x8d   */       
  {sizeof(T_BAT_res_que_plus_fap)},        /* BAT_RES_QUE_PLUS_FAP           = 0x8e   */       
  {sizeof(T_BAT_res_que_plus_fbo)},        /* BAT_RES_QUE_PLUS_FBO           = 0x8f   */       
  {sizeof(T_BAT_res_que_plus_fbs)},        /* BAT_RES_QUE_PLUS_FBS           = 0x90   */       
  {sizeof(T_BAT_res_que_plus_fbu)},        /* BAT_RES_QUE_PLUS_FBU           = 0x91   */       
  {sizeof(T_BAT_res_que_plus_fcc)},        /* BAT_RES_QUE_PLUS_FCC           = 0x92   */       
  {sizeof(T_BAT_res_que_plus_fclass)},     /* BAT_RES_QUE_PLUS_FCLASS        = 0x93   */       
  {sizeof(T_BAT_res_que_plus_fcq)},        /* BAT_RES_QUE_PLUS_FCQ           = 0x94   */       
  {sizeof(T_BAT_res_que_plus_fcr)},        /* BAT_RES_QUE_PLUS_FCR           = 0x95   */       
  {sizeof(T_BAT_res_que_plus_fcs)},        /* BAT_RES_QUE_PLUS_FCS           = 0x96   */       
  {sizeof(T_BAT_res_que_plus_fct)},        /* BAT_RES_QUE_PLUS_FCT           = 0x97   */       
  {sizeof(T_BAT_res_que_plus_fea)},        /* BAT_RES_QUE_PLUS_FEA           = 0x98   */       
  {sizeof(T_BAT_res_que_plus_ffc)},        /* BAT_RES_QUE_PLUS_FFC           = 0x99   */       
  {sizeof(T_BAT_res_que_plus_fhs)},        /* BAT_RES_QUE_PLUS_FHS           = 0x9a   */       
  {sizeof(T_BAT_res_que_plus_fie)},        /* BAT_RES_QUE_PLUS_FIE           = 0x9b   */       
  {sizeof(T_BAT_res_que_plus_fis)},        /* BAT_RES_QUE_PLUS_FIS           = 0x9c   */       
  {sizeof(T_BAT_res_que_plus_fit)},        /* BAT_RES_QUE_PLUS_FIT           = 0x9d   */       
  {sizeof(T_BAT_res_que_plus_fli)},        /* BAT_RES_QUE_PLUS_FLI           = 0x9e   */       
  {sizeof(T_BAT_res_que_plus_flo)},        /* BAT_RES_QUE_PLUS_FLO           = 0x9f   */       
  {sizeof(T_BAT_res_que_plus_flp)},        /* BAT_RES_QUE_PLUS_FLP           = 0xa0   */       
  {sizeof(T_BAT_res_que_plus_fms)},        /* BAT_RES_QUE_PLUS_FMS           = 0xa1   */       
  {sizeof(T_BAT_res_que_plus_fns)},        /* BAT_RES_QUE_PLUS_FNS           = 0xa2   */       
  {sizeof(T_BAT_res_que_plus_fpa)},        /* BAT_RES_QUE_PLUS_FPA           = 0xa3   */       
  {sizeof(T_BAT_res_que_plus_fpi)},        /* BAT_RES_QUE_PLUS_FPI           = 0xa4   */       
  {sizeof(T_BAT_res_que_plus_fps)},        /* BAT_RES_QUE_PLUS_FPS           = 0xa5   */       
  {sizeof(T_BAT_res_que_plus_fpw)},        /* BAT_RES_QUE_PLUS_FPW           = 0xa6   */       
  {sizeof(T_BAT_res_que_plus_frq)},        /* BAT_RES_QUE_PLUS_FRQ           = 0xa7   */       
  {sizeof(T_BAT_res_que_plus_fsa)},        /* BAT_RES_QUE_PLUS_FSA           = 0xa8   */       
  {sizeof(T_BAT_res_que_plus_fsp)},        /* BAT_RES_QUE_PLUS_FSP           = 0xa9   */       
  {sizeof(T_BAT_res_que_plus_icf)},        /* BAT_RES_QUE_PLUS_ICF           = 0xaa   */       
  {sizeof(T_BAT_res_que_plus_ifc)},        /* BAT_RES_QUE_PLUS_IFC           = 0xab   */       
  {sizeof(T_BAT_res_que_percent_imei)},    /* BAT_RES_QUE_PERCENT_IMEI       = 0xac   */       
  {sizeof(T_BAT_res_que_plus_ipr)},        /* BAT_RES_QUE_PLUS_IPR           = 0xad   */       
  {sizeof(T_BAT_res_que_percent_pbcf)},    /* BAT_RES_QUE_PERCENT_PBCF       = 0xae   */       
  {sizeof(T_BAT_res_que_percent_ppp)},     /* BAT_RES_QUE_PERCENT_PPP        = 0xaf   */       
  {sizeof(T_BAT_res_que_percent_pvrf)},    /* BAT_RES_QUE_PERCENT_PVRF       = 0xb0   */       
  {sizeof(T_BAT_res_que_percent_rdl)},     /* BAT_RES_QUE_PERCENT_RDL        = 0xb1   */       
  {sizeof(T_BAT_res_que_percent_rdlb)},    /* BAT_RES_QUE_PERCENT_RDLB       = 0xb2   */       
  {sizeof(T_BAT_res_que_percent_satc)},    /* BAT_RES_QUE_PERCENT_SATC       = 0xb3   */       
  {sizeof(T_BAT_res_que_percent_satcc)},   /* BAT_RES_QUE_PERCENT_SATCC      = 0xb4   */       
  {sizeof(T_BAT_res_set_percent_sate)},    /* BAT_RES_SET_PERCENT_SATE       = 0xb5   */       
  {sizeof(T_BAT_res_que_plus_ws46)},       /* BAT_RES_QUE_PLUS_WS46          = 0xb6   */
  {sizeof(T_BAT_res_que_plus_cdip)},       /* BAT_RES_QUE_PLUS_CDIP          = 0xb7   */
  {sizeof(T_BAT_res_set_percent_cmgr)},    /* BAT_RES_SET_PERCENT_CMGR       = 0xb8   */
  {sizeof(T_BAT_res_set_percent_cmgl)},    /* BAT_RES_SET_PERCENT_CMGL       = 0xb9   */
  {sizeof(T_BAT_res_set_plus_cmss)},       /* BAT_RES_SET_PLUS_CMSS          = 0xba   */
  {sizeof(T_BAT_res_set_plus_cgmi)},       /* BAT_RES_SET_PLUS_CGMI          = 0xbb  */ 
  {sizeof(T_BAT_res_set_plus_cgmm)},       /* BAT_RES_SET_PLUS_CGMM          = 0xbc  */ 
  {sizeof(T_BAT_res_set_plus_cgmr)},       /* BAT_RES_SET_PLUS_CGMR          = 0xbd   */ 
  {sizeof(T_BAT_res_set_plus_cgsn)},       /* BAT_RES_SET_PLUS_CGSN          = 0xbe */ 
#ifdef SIM_PERS
  {sizeof(T_BAT_res_set_percent_mepd)},    /* BAT_RES_SET_PERCENT_MEPD       = 0xbf  */
#else
  {0},       /* If the flag is not enabled to get the proper size of structure of response according to the command id  */
#endif /* SIM_PERS */
  {sizeof(T_BAT_res_set_percent_csq)},      /* BAT_RES_SET_PERCENT_CSQ        = 0xc0  */
  {sizeof(T_BAT_res_que_percent_cniv)},     /* BAT_RES_QUE_PERCENT_CNIV       = 0xc1  */
  {sizeof(T_BAT_res_que_percent_simef)}     /* BAT_RES_QUE_PERCENT_SIMEF       = 0xcb  */
};

static T_map_response_2_size unsolicited_2_size[] =
{
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_UNS_AT_BUSY            = 0x1000 */
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_UNS_AT_NO_ANSWER       = 0x1001 */
  {sizeof(T_BAT_res_uns_plus_cbm)},        /* BAT_RES_UNS_PLUS_CBM           = 0x1002 */
  {sizeof(T_BAT_res_uns_plus_cbmi)},       /* BAT_RES_UNS_PLUS_CBMI          = 0x1003 */
  {sizeof(T_BAT_res_uns_percent_ccbs)},    /* BAT_RES_UNS_PERCENT_CCBS       = 0x1004 */
  {sizeof(T_BAT_res_uns_plus_cccm)},       /* BAT_RES_UNS_PLUS_CCCM          = 0x1005 */
  {sizeof(T_BAT_res_uns_percent_cccn)},    /* BAT_RES_UNS_PERCENT_CCCN       = 0x1006 */
  {sizeof(T_BAT_res_uns_plus_ccwa)},       /* BAT_RES_UNS_PLUS_CCWA          = 0x1007 */
  {sizeof(T_BAT_res_uns_plus_ccwa_w)},     /* BAT_RES_UNS_PLUS_CCWA_W        = 0x1008 */
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_UNS_PLUS_CCWE          = 0x1009 */
  {sizeof(T_BAT_res_uns_plus_cdip)},       /* BAT_RES_UNS_PLUS_CDIP          = 0x100a */
  {sizeof(T_BAT_res_uns_plus_cds)},        /* BAT_RES_UNS_PLUS_CDS           = 0x100b */
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_UNS_PLUS_CCWV          = 0x100c */
  {sizeof(T_BAT_res_uns_percent_cgev)},    /* BAT_RES_UNS_PERCENT_CGEV       = 0x100d */
  {sizeof(T_BAT_res_uns_plus_cgev)},       /* BAT_RES_UNS_PLUS_CGEV          = 0x100e */
  {sizeof(T_BAT_res_uns_percent_cgreg)},   /* BAT_RES_UNS_PERCENT_CGREG      = 0x100f */
  {sizeof(T_BAT_res_uns_plus_cgreg)},      /* BAT_RES_UNS_PLUS_CGREG         = 0x1010 */
  {sizeof(T_BAT_res_uns_plus_clae)},       /* BAT_RES_UNS_PLUS_CLAE          = 0x1011 */
  {sizeof(T_BAT_res_uns_plus_clav)},       /* BAT_RES_UNS_PLUS_CLAV          = 0x1012 */
  {sizeof(T_BAT_res_uns_plus_clip)},       /* BAT_RES_UNS_PLUS_CLIP          = 0x1013 */
  {sizeof(T_BAT_res_uns_plus_clip_w)},     /* BAT_RES_UNS_PLUS_CLIP_W        = 0x1014 */
  {sizeof(T_BAT_res_uns_plus_cmt)},        /* BAT_RES_UNS_PLUS_CMT           = 0x1015 */
  {sizeof(T_BAT_res_uns_plus_cmti)},       /* BAT_RES_UNS_PLUS_CMTI          = 0x1016 */
  {sizeof(T_BAT_res_uns_plus_cmt_w)},      /* BAT_RES_UNS_PLUS_CMT_W         = 0x1017 */
  {sizeof(T_BAT_res_uns_percent_cnap)},    /* BAT_RES_UNS_PERCENT_CNAP       = 0x1018 */
  {sizeof(T_BAT_res_uns_plus_colp)},       /* BAT_RES_UNS_PLUS_COLP          = 0x1019 */
  {sizeof(T_BAT_res_uns_plus_colp_w)},     /* BAT_RES_UNS_PLUS_COLP_W        = 0x101a */
  {sizeof(T_BAT_res_uns_percent_cpi)},     /* BAT_RES_UNS_PERCENT_CPI        = 0x101b */
  {sizeof(T_BAT_res_uns_percent_cpi_w)},   /* BAT_RES_UNS_PERCENT_CPI_W      = 0x101c */
  {sizeof(T_BAT_res_uns_percent_cpri)},    /* BAT_RES_UNS_PERCENT_CPRI       = 0x101d */
  {sizeof(T_BAT_res_uns_percent_cproam)},  /* BAT_RES_UNS_PERCENT_CPROAM     = 0x101e */
  {sizeof(T_BAT_res_uns_percent_cpvwi)},   /* BAT_RES_UNS_PERCENT_CPVWI      = 0x101f */
  {sizeof(T_BAT_res_uns_plus_cr)},         /* BAT_RES_UNS_PLUS_CR            = 0x1020 */
  {sizeof(T_BAT_res_uns_percent_creg)},    /* BAT_RES_UNS_PERCENT_CREG       = 0x1021 */
  {sizeof(T_BAT_res_uns_plus_cring)},      /* BAT_RES_UNS_PLUS_CRING         = 0x1022 */
  {sizeof(T_BAT_res_uns_percent_csq)},     /* BAT_RES_UNS_PERCENT_CSQ        = 0x1023 */
  {sizeof(T_BAT_res_uns_plus_cssi)},       /* BAT_RES_UNS_PLUS_CSSI          = 0x1024 */
  {sizeof(T_BAT_res_uns_percent_cssn)},    /* BAT_RES_UNS_PERCENT_CSSN       = 0x1025 */
  {sizeof(T_BAT_res_uns_plus_cssu)},       /* BAT_RES_UNS_PLUS_CSSU          = 0x1026 */
  {sizeof(T_BAT_res_uns_percent_cstat)},   /* BAT_RES_UNS_PERCENT_CSTAT      = 0x1027 */
  {sizeof(T_BAT_res_uns_percent_ctyi)},    /* BAT_RES_UNS_PERCENT_CTYI       = 0x1028 */
  {sizeof(T_BAT_res_uns_plus_ctzr)},       /* BAT_RES_UNS_PLUS_CTZR          = 0x1029 */
  {sizeof(T_BAT_res_uns_percent_ctzv)},    /* BAT_RES_UNS_PERCENT_CTZV       = 0x102a */
  {sizeof(T_BAT_res_uns_plus_ctzv)},       /* BAT_RES_UNS_PLUS_CTZV          = 0x102b */
  {sizeof(T_BAT_res_uns_percent_ctzv_w)},  /* BAT_RES_UNS_PERCENT_CTZV_W     = 0x102c */
  {sizeof(T_BAT_res_uns_plus_cusd)},       /* BAT_RES_UNS_PLUS_CUSD          = 0x102d */
  {sizeof(T_BAT_res_uns_plus_cusd_w)},     /* BAT_RES_UNS_PLUS_CUSD_W        = 0x102e */
  {sizeof(T_BAT_no_parameter)},            /* BAT_RES_UNS_AT_NO_CARRIER      = 0x102f */
  {sizeof(T_BAT_res_uns_plus_fhs)},        /* BAT_RES_UNS_PLUS_FHS           = 0x1030 */
  {sizeof(T_BAT_res_uns_plus_ilrr)},       /* BAT_RES_UNS_PLUS_ILRR          = 0x1031 */
  {sizeof(T_BAT_res_uns_percent_rdl)},     /* BAT_RES_UNS_PERCENT_RDL        = 0x1032 */
  {sizeof(T_BAT_res_uns_percent_rdlb)},    /* BAT_RES_UNS_PERCENT_RDLB       = 0x1033 */
  {sizeof(T_BAT_res_uns_percent_sata)},    /* BAT_RES_UNS_PERCENT_SATA       = 0x1034 */
  {sizeof(T_BAT_res_uns_percent_sati)},    /* BAT_RES_UNS_PERCENT_SATI       = 0x1035 */
  {sizeof(T_BAT_res_uns_percent_satn)},    /* BAT_RES_UNS_PERCENT_SATN       = 0x1036 */
  {sizeof(T_BAT_res_uns_percent_simef)},   /* BAT_RES_UNS_PERCENT_SIMEF      = 0x1037 */
  {sizeof(T_BAT_res_uns_percent_simins)},  /* BAT_RES_UNS_PERCENT_SIMINS     = 0x1038 */
  {sizeof(T_BAT_res_uns_percent_simrem)},  /* BAT_RES_UNS_PERCENT_SIMREM     = 0x1039 */
  {sizeof(T_BAT_res_uns_percent_sncnt)},   /* BAT_RES_UNS_PERCENT_SNCNT      = 0x103a */
  {sizeof(T_BAT_res_uns_percent_cniv)}       /* BAT_RES_UNS_PERCENT_CNIV     = 0x103b */
};

typedef struct
{
  T_BAT_ctrl_params bat;
  T_ACI_AT_CMD      aci;
} T_map_batcmd_to_acicmd;
/*
 * only asynchronous commands are listed (interaction with PS)
 */
 #define CMD_MAP_NOT_PRESENT -1
T_map_batcmd_to_acicmd  map_batcmd_to_acicmd [] = 
{                 
  {BAT_CMD_SET_PLUS_CFUN,         AT_CMD_CFUN},
  {BAT_CMD_SET_PERCENT_BAND,      AT_CMD_BAND},
  {BAT_CMD_SET_PLUS_CPIN,         AT_CMD_CPIN},
  {BAT_CMD_QUE_PLUS_CPIN,         AT_CMD_CPIN},
  {BAT_CMD_QUE_PERCENT_CPINF,     AT_CMD_CPIN},
  {BAT_CMD_SET_PLUS_CPBW,         AT_CMD_CPBW},
  {BAT_CMD_TST_PERCENT_COPS,      AT_CMD_P_COPS},
  {BAT_CMD_TST_PLUS_COPS,         AT_CMD_COPS},
  {BAT_CMD_SET_PERCENT_COPS,      AT_CMD_P_COPS},
  {BAT_CMD_SET_PLUS_COPS,         AT_CMD_COPS},
  {BAT_CMD_AT_D,                  AT_CMD_D},
  {BAT_CMD_AT_A,                  AT_CMD_A},
  {BAT_CMD_AT_Z,                  AT_CMD_Z},
  {BAT_CMD_SET_PERCENT_CHLD,      AT_CMD_CHLD},
  {BAT_CMD_SET_PLUS_CTFR,         AT_CMD_CTFR},
  {BAT_CMD_SET_PERCENT_VTS,       AT_CMD_VTS},
  {BAT_CMD_SET_PLUS_VTS,          AT_CMD_VTS},
  {BAT_CMD_QUE_PLUS_CLIR,         AT_CMD_CLIR},
  {BAT_CMD_SET_PLUS_CLIR,         AT_CMD_CLIR},
  {BAT_CMD_QUE_PLUS_CLIP,         AT_CMD_CLIP},
  {BAT_CMD_QUE_PLUS_COLP,         AT_CMD_COLP},
  {BAT_CMD_SET_PLUS_CCFC,         AT_CMD_CCFC},
  {BAT_CMD_SET_PLUS_CLCK,         AT_CMD_CLCK},
  {BAT_CMD_SET_PLUS_CCWA,         AT_CMD_CCWA},
  {BAT_CMD_SET_PLUS_CPWD,         AT_CMD_CPWD},
  {BAT_CMD_SET_PLUS_CUSD,         AT_CMD_CUSD},
  {BAT_CMD_SET_PLUS_CMGD,         AT_CMD_CMGD},
  {BAT_CMD_SET_PLUS_CMGW,         AT_CMD_CMGW},
  {BAT_CMD_SET_PLUS_CMGS,         AT_CMD_CMGS},
  {BAT_CMD_SET_PLUS_CMSS,         AT_CMD_CMSS},
  {BAT_CMD_SET_PLUS_CSCA,         AT_CMD_CSCA},
  {BAT_CMD_QUE_PLUS_CSCA,         AT_CMD_CSCA},
  {BAT_CMD_SET_PLUS_CGATT,        AT_CMD_CGATT}, 
  {BAT_CMD_QUE_PLUS_CGATT,        AT_CMD_CGATT}, 
  {BAT_CMD_QUE_PERCENT_CGCLASS,   AT_CMD_P_CGCLASS},
  {BAT_CMD_QUE_PLUS_CGCLASS,      AT_CMD_CGCLASS},
  {BAT_CMD_SET_PERCENT_CGCLASS,   AT_CMD_P_CGCLASS},
  {BAT_CMD_SET_PLUS_CGCLASS,      AT_CMD_CGCLASS},
  {BAT_CMD_SET_PLUS_CGACT,        AT_CMD_CGACT},
  {BAT_CMD_QUE_PLUS_CGACT,        AT_CMD_CGACT},
  {BAT_CMD_SET_PLUS_CGDATA,       AT_CMD_CGDATA},
  {BAT_CMD_SET_PLUS_CGANS,        AT_CMD_CGANS},
  {BAT_CMD_SET_PLUS_CGSMS,        AT_CMD_CGSMS},
  {BAT_CMD_QUE_PLUS_CGSMS,        AT_CMD_CGSMS},
  {(T_BAT_ctrl_params)CMD_MAP_NOT_PRESENT,  AT_CMD_NONE}
};



/*==== LOCAL FUNCTONS =============================================*/
LOCAL void aci_bat_init_globs(void);
LOCAL void aci_bat_queue_put (T_aci_bat_queue *que, T_BAT_cmd_send *cmd, U8 client_id, U16 dataSize);
LOCAL BOOL aci_bat_queue_get (T_aci_bat_queue *que, T_BAT_cmd_send *cmd, U8 *client_id);
LOCAL T_aci_bat_queue *aci_bat_queue_create (void);
LOCAL void             aci_bat_queue_remove (T_aci_bat_queue *que);


LOCAL void *aci_bat_give_l2p_rx_buf_cb (U8 batId);
LOCAL void *aci_bat_give_l2p_tx_buf_cb (U8 batId, U16 dataSize, void **seg_hdr_ptr,  U16 *totalSize, U16 *segSize);
LOCAL void  aci_bat_free_data_buffer   (T_ACI_DTI_DATA_BUF *data_buffer);
LOCAL BOOL  aci_bat_buffer_dti_data    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_desc2 *data);
LOCAL void  aci_bat_send_dti_data      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_desc2 *data);
LOCAL void *aci_bat_l2p_next_seg_cb    (U8 batId , void *last_seg_hdr, void **seg_hdr_ptr,  U16 *segSize);
LOCAL int   aci_bat_l2p_send_frame_cb  (U8 batId);
LOCAL void  aci_bat_l2p_msg_rxd_cb     (U8 batId, U8 muxId, U32 dataTag, void *dataPtr, U16 dataSize);

LOCAL int  aci_bat_get_size (T_BAT_ctrl_response ctrl_response);
LOCAL void aci_bat_init_max_client_maintenance (T_ACI_DTI_PRC_PSI *src_infos_psi, U8 num_clients);
LOCAL void aci_bat_clean_client_maintenance (T_ACI_DTI_PRC_PSI *src_infos_psi, U8 client);
LOCAL BOOL aci_bat_bootstrap_l2p            (T_ACI_DTI_PRC_PSI *src_infos_psi, T_DTI2_DATA_IND *dti_data_ind);
LOCAL BOOL aci_bat_abort (T_ACI_DTI_PRC_PSI *src_infos_psi, U8 client_id);
LOCAL T_ACI_AT_CMD  map_cmd_id (T_BAT_ctrl_params cmd);

/*------------------------------------------------------------------------------
 * prepare unique identifiers for L2P_Configure, BATLib starts with 0, we start with 0x80
 */
LOCAL void aci_bat_init_globs(void)
{
  int i;
  for (i=0; i<CMD_SRC_MAX; i++)
  {
    aci_bat_globs[i].l2p_configured = FALSE;
    aci_bat_globs[i].l2p_id = 0x80+i;
    aci_bat_globs[i].psi_src = NULL;
    memset(aci_bat_globs[i].l2p_mem, 0x00, L2P_CONTROL_BLOCK_HDR_SIZE);
  }
}


/*------------------------------------------------------------------------------
 * received BAT commands cannot be processed, make it pending
 */
LOCAL void aci_bat_queue_put (T_aci_bat_queue *que, T_BAT_cmd_send *cmd, U8 client_id, U16 dataSize)
{
  T_aci_bat_queue_entry *qe;

  ACI_MALLOC(qe, sizeof (T_aci_bat_queue_entry));

  qe->next = 0;
  qe->cmd.ctrl_params = cmd->ctrl_params;
  qe->client_id       = client_id;

  /* due to union params, we can use any pointer. will be freed in aci_bat_run_cmd() */
  ACI_MALLOC(qe->cmd.params.ptr_andc, dataSize);
  memcpy (qe->cmd.params.ptr_andc, cmd->params.ptr_andc, dataSize);
  
  if (que->first)
  {
    que->last->next = qe;
  }
  else
  {
    que->first = qe;
  }
  que->last = qe;
}

/*------------------------------------------------------------------------------
 * get pending BAT command of a client queue
 */
LOCAL BOOL aci_bat_queue_get (T_aci_bat_queue *que, T_BAT_cmd_send *cmd, U8 *client_id)
{
  T_aci_bat_queue_entry *qe;

  if (!que)
  {
    return (FALSE);
  }

  qe = que->first;
  if (!qe)
  {
     return (FALSE);
  }
  cmd->ctrl_params = qe->cmd.ctrl_params;
  cmd->params      = qe->cmd.params;
  *client_id       = qe->client_id;
  
  que->first       = qe->next;
  
  if (que->first EQ NULL)
  {
    que->last = NULL;
  }
    
  ACI_MFREE(qe);
    
  return (TRUE);
}

/*------------------------------------------------------------------------------
 * aci_bat_queue_create
 */
LOCAL T_aci_bat_queue *aci_bat_queue_create (void)
{
  T_aci_bat_queue *que;

  ACI_MALLOC(que, sizeof (T_aci_bat_queue));

  if (que EQ NULL)
  {
    return 0;
  }
  que->first = 0;
  return que;
}

/*------------------------------------------------------------------------------
 * aci_bat_queue_remove
 */
LOCAL void aci_bat_queue_remove (T_aci_bat_queue *que)
{
  T_BAT_cmd_send dummy1;
  U8             dummy2 =  0;

  while (aci_bat_queue_get (que, &dummy1, &dummy2))
  {
    ;
  }
  if (que)
  {
    ACI_MFREE (que);
  }
}


/*------------------------------------------------------------------------------
 * get the response size needed for L2P to know how many bytes to send
 */
LOCAL int aci_bat_get_size (T_BAT_ctrl_response ctrl_response)
{
  if (BITFIELD_CHECK(ctrl_response, BAT_BIT_12)) /* 0x1000 */
  {
    return (unsolicited_2_size[ctrl_response - BAT_BIT_12].size);
  }
  else
  {
    return (response_2_size[ctrl_response].size);
  }
}

/*------------------------------------------------------------------------------
 * init max BAT clients maintance for a PSI source
 */
LOCAL void aci_bat_init_max_client_maintenance(T_ACI_DTI_PRC_PSI *src_infos_psi, U8 num_clients)
{
  int i;
  ACI_MALLOC(src_infos_psi->bat_client, num_clients * sizeof(T_bat_aci_client_maintain));
  src_infos_psi->queue       = aci_bat_queue_create();
  src_infos_psi->max_clients = num_clients;
  for (i=0; i< num_clients; i++)
  {
    src_infos_psi->bat_client[i].opened    = FALSE;
    src_infos_psi->bat_client[i].curCmd    = (T_BAT_ctrl_params)-1;
    src_infos_psi->bat_client[i].send_data = (T_desc2 *)NULL;
  }
}

/*------------------------------------------------------------------------------
 * clean up the whole clients maintenance for a PSI source
 */
LOCAL void aci_bat_clean_client_maintenance(T_ACI_DTI_PRC_PSI *src_infos_psi, U8 client)
{
  BOOL all_closed = TRUE;
  int i = 0;

  src_infos_psi->bat_client[client].curCmd = (T_BAT_ctrl_params)-1;
  src_infos_psi->bat_client[client].opened = FALSE;

  for (i=0; i< src_infos_psi->max_clients; i++)
  {
    if (src_infos_psi->bat_client[i].opened EQ TRUE)
    {
      all_closed = FALSE;
      break;
    }
  }
  if (all_closed)
  {
    aci_bat_queue_remove(src_infos_psi->queue);
    L2P_Remove(aci_bat_globs[src_infos_psi->srcId].l2p_id);
    ACI_MFREE(src_infos_psi->bat_client);
    aci_bat_globs[src_infos_psi->srcId].l2p_configured = FALSE;
  }

  all_closed = TRUE;
  for (i=0; i< CMD_SRC_MAX; i++)
  {
    if (aci_bat_globs[i].l2p_configured NEQ FALSE)
    {
      all_closed = FALSE;
      break;
    }
  }
  if (all_closed)
  {
    aci_bat_first_time = TRUE;
  }
}


/*------------------------------------------------------------------------------
 * pick out from the very first received DTI_DATA_IND the L2P protocol id.
 * our client (BAT lib at Application or ATI) defined the L2P protocol
 */

LOCAL BOOL aci_bat_bootstrap_l2p (T_ACI_DTI_PRC_PSI *src_infos_psi, T_DTI2_DATA_IND *dti_data_ind)
{
  U16  tot_len;
  U8   protocol_id = 0;
  T_L2P_STATUS l2p_status = L2P_STAT_UNKNOWN_ERROR;
  T_L2P_FRAME_CONTROL_BYTES *l2p_header = (T_L2P_FRAME_CONTROL_BYTES *)NULL;
  T_desc2 *p_desc;

  tot_len = dti_data_ind->desc_list2.list_len;
  p_desc  = (T_desc2 *)dti_data_ind->desc_list2.first;

  if (p_desc NEQ NULL)
  {
    if( p_desc->len > tot_len)
    {
      TRACE_EVENT("aci_bat_bootstrap_l2p(): invalid dti_data_ind");
      return (FALSE);
    }

    l2p_header  = (T_L2P_FRAME_CONTROL_BYTES *)p_desc->buffer;
    protocol_id = l2p_header->protocol_id;


    TRACE_EVENT_P3("aci_bat_bootstrap_l2p(): protocol_id = %d, src_id = %d, l2p_id = 0x%02X",
                    protocol_id, src_infos_psi->srcId, aci_bat_globs[src_infos_psi->srcId].l2p_id);


    l2p_status = L2P_Configure(aci_bat_globs[src_infos_psi->srcId].l2p_id, /* what L2P maintains as bat_id */
                               (void *)(aci_bat_globs[src_infos_psi->srcId].l2p_mem), /* memory for the internal L2P maintenance   */
                               (T_L2P_SUPPORTED_PROTOCOLS)protocol_id,
                               BAT_MTU_SIZE,            /* replace replace  GDD_DIO_MTU_SIZE                     */
                               aci_bat_give_l2p_tx_buf_cb,  /* L2P needs callbacks                */
                               aci_bat_give_l2p_rx_buf_cb,
                               aci_bat_l2p_next_seg_cb,
                               aci_bat_l2p_send_frame_cb,
                               aci_bat_l2p_msg_rxd_cb);

    switch (l2p_status)
    {
      case (L2P_STAT_SUCCESS):
        return (TRUE);
      default: /* any error */
        TRACE_EVENT_P1("aci_bat_bootstrap_l2p(): L2P error = %d", l2p_status);
        return (FALSE);
    }
  }
  return (FALSE);
}

/*------------------------------------------------------------------------------
 * L2P wants always to have the BAT_MTU_SIZE memory for the received data
 * it rebuilds from L2P fragments and T_desc2 segements the complete BAT command
 * We will free the memory in aci_bat_l2p_msg_rxd_cb()
 * From the batId we derive the src_id by substract 0x80, see aci_bat_init_globs()
 */
LOCAL void *aci_bat_give_l2p_rx_buf_cb (U8 batId)
{
  U8 src_id = batId-0x80;
  TRACE_FUNCTION("aci_bat_give_l2p_rx_buf_cb()");

  if (batId EQ aci_bat_globs[src_id].l2p_id)
  {
    T_ACI_DTI_PRC_PSI *src_infos_psi = aci_bat_globs[src_id].psi_src;

    ACI_MALLOC(src_infos_psi->rcv_data, BAT_MTU_SIZE);

    return (src_infos_psi->rcv_data);
  }
  else
  {
    TRACE_ERROR("aci_bat_give_l2p_rx_buf_cb(): L2P used wrong identifier");
    return (NULL);
  }
}

/*------------------------------------------------------------------------------
 * L2P wants to have some memory to transmit data. It is called only for the first segment.
 * dataSize is what L2P requests from us
 * with totalSize we tell L2P what we got from the memory pool
 * segSize ist the size of the first segment
 * seg_hdr_ptr is the pointer to T_desc2, but casted to void
 * returned is T_desc2.buffer
 * PSI will free the memory
 * From the batId we derive the src_id by substract 0x80, see aci_bat_init_globs()
 */
LOCAL void *aci_bat_give_l2p_tx_buf_cb (U8 batId, U16 dataSize, void **seg_hdr_ptr, U16 *totalSize, U16 *segSize)
{
  U8 src_id = batId-0x80;
  TRACE_FUNCTION("aci_bat_give_l2p_tx_buf_cb()");

  if (batId EQ aci_bat_globs[src_id].l2p_id)
  {
    T_desc2  *send_data;
    U16       send_len = (U16)(sizeof(T_desc2) - 1 + dataSize);

    T_ACI_DTI_PRC_PSI *src_infos_psi = aci_bat_globs[src_id].psi_src;

    ACI_MALLOC(send_data, send_len);

    send_data->next   = (U32)NULL;
    send_data->len    = dataSize;
    send_data->size   = dataSize;
    send_data->offset = 0;

    *seg_hdr_ptr  = (void *)send_data;
    *totalSize    = send_len;
    *segSize      = send_len;
    TRACE_EVENT_P2("aci_bat_give_l2p_tx_buf_cb(): L2P requested %i bytes and got %i bytes", dataSize, send_len);

    /* for control and broadcast channel we must not maintain the first T_desc2
     * in the client array, because it were just too large,  BAT_CTRL_CHANNEL = 0xFE)
     */
    if (src_infos_psi->active_client >= BAT_CTRL_CHANNEL)
    {
      src_infos_psi->send_data = send_data;
    }
    else
    {
      src_infos_psi->bat_client[src_infos_psi->active_client].send_data = send_data;
    }

    return ((void *)send_data->buffer);
  }
  else
  {
    TRACE_ERROR("aci_bat_give_l2p_tx_buf_cb(): L2P used wrong identifier");
    return (NULL);
  }
}

/*------------------------------------------------------------------------------
 *
 */
LOCAL void *aci_bat_l2p_next_seg_cb (U8 batId, void *last_seg_hdr, void **seg_hdr_ptr, U16 *segSize)
{
  T_desc2 *segment;

  TRACE_FUNCTION("aci_bat_l2p_next_seg_cb()");

  segment      = (T_desc2 *)last_seg_hdr;
  segment      = (T_desc2 *)segment->next;
  *seg_hdr_ptr = segment;
  *segSize     = segment->size;

  return ((void *)segment->buffer);
}

LOCAL void aci_bat_free_data_buffer (T_ACI_DTI_DATA_BUF *data_buffer)
{
  T_desc2 *p_desc, *p_desc_next=NULL;

  if (data_buffer->data_list EQ NULL)
  {
    return;
  }

  p_desc = data_buffer->data_list;
  if (p_desc NEQ NULL)
  {
    p_desc_next = (T_desc2 *)p_desc->next;
  }

  while (p_desc NEQ NULL)
  {
    ACI_MFREE (p_desc);

    p_desc = p_desc_next;
    if (p_desc NEQ NULL)
    {
      p_desc_next = (T_desc2 *)p_desc->next;
    }
  }

  data_buffer->data_list       = NULL;
  data_buffer->max_buffer_size = 0;
  data_buffer->create_time     = 0;
}


/*------------------------------------------------------------------------------
 *
 */
 LOCAL BOOL aci_bat_buffer_dti_data (T_ACI_DTI_PRC_PSI *src_infos_psi, T_desc2 *data)
{
  T_desc2 *tmp;
  T_TIME current_time;

  static int i = 0;

  TRACE_FUNCTION("aci_bat_buffer_dti_data()");

  if (src_infos_psi->data_buffer.data_list EQ NULL)
  {
    src_infos_psi->data_buffer.data_list = data; /* the first buffered resp/ind */
    /* store the creation time for the data buffer */
    vsi_t_time (VSI_CALLER &src_infos_psi->data_buffer.create_time);
  }
  else
  {
    /*
     * check whether the buffered resp/ind are already too long buffered
     * this avoids eating up all the memory
     */
    vsi_t_time (VSI_CALLER &current_time);
    if ((current_time - src_infos_psi->data_buffer.create_time) > MAX_DATA_BUFFER_LIFE_TIME)
    {
      TRACE_EVENT_P1 ("[WRN] aci_bat_buffer_dti_data(): life time expired, buffer deleted (%d)",
                      current_time - src_infos_psi->data_buffer.create_time);
      aci_bat_free_data_buffer (&src_infos_psi->data_buffer);
      /* send an error to BAT Lib */
      return (FALSE);
  }

    /*
     * we still can buffer further resp/ind
     */
    tmp = src_infos_psi->data_buffer.data_list;
    while ((T_desc2*)tmp->next NEQ NULL)
    {
      tmp = (T_desc2*)tmp->next;
    }
    tmp->next = (U32)data; /* append the current resp/ind */
    i++;
  }
  return (TRUE);
}


/*------------------------------------------------------------------------------
 *
 */
 LOCAL void aci_bat_send_dti_data (T_ACI_DTI_PRC_PSI *src_infos_psi, T_desc2 *data)
{
  U16 data_len = 0;
  T_desc2 *next_data;
  TRACE_FUNCTION("aci_bat_send_dti_data");

  {
    PALLOC_DESC2(dti_data_ind, DTI2_DATA_IND);

    next_data = data;

    while (next_data NEQ NULL)
    {
      data_len += next_data->len;
      next_data = (T_desc2*)next_data->next;
    }

    dti_data_ind->desc_list2.list_len = data_len;
    dti_data_ind->desc_list2.first    = (U32)data;
    dti_data_ind->parameters.p_id     = 0; /*dummy_ubyte; */

    dti_data_ind->parameters.st_lines.st_flow      = DTI_FLOW_OFF;
    dti_data_ind->parameters.st_lines.st_line_sb   = DTI_SB_ON;
    dti_data_ind->parameters.st_lines.st_line_sa   = DTI_SA_ON;
    dti_data_ind->parameters.st_lines.st_break_len = DTI_BREAK_OFF;
#ifdef _SIMULATION_
    if (src_infos_psi->devId NEQ DIO_DATA_PKT)
    {
      gdd_aci_write_bat(src_infos_psi->srcId, &(dti_data_ind->desc_list2)); /* special GDD adapter between BATLib and BATModule */
      PFREE(dti_data_ind);
    }
    else
    {    /* this is for the acibat_test suite, which does not use the ATI as application */
      dti_send_data (aci_hDTI,
                     src_infos_psi->srcId,
                     (U8)DTI_ENTITY_PSI,
                     ACI_DTI_DN_CHANNEL,
                     dti_data_ind);
    }
#else
    dti_send_data (aci_hDTI,
                   src_infos_psi->srcId,
                   (U8)DTI_ENTITY_PSI,
                   ACI_DTI_DN_CHANNEL,
                   dti_data_ind);
#endif
  }
}


/*------------------------------------------------------------------------------
 * L2P has a frame ready for sending.
 * We have to send a DTI2_DATA_IND primitive to PSI
 * the pointer of the first T_desc2 is maintained in src_infos_psi.
 * From the batId we derive the src_id by substract 0x80, see aci_bat_init_globs()
 */
LOCAL int aci_bat_l2p_send_frame_cb (U8 batId)
{
  U8 src_id = batId-0x80;
  T_desc2 *data;
  T_ACI_DTI_PRC_PSI *src_infos_psi = aci_bat_globs[src_id].psi_src;

  TRACE_FUNCTION("aci_bat_l2p_send_frame_cb()");

  if (src_infos_psi->active_client >= BAT_CTRL_CHANNEL)
  {
    data = src_infos_psi->send_data;
  }
  else
  {
    data = src_infos_psi->bat_client[src_infos_psi->active_client].send_data;
  }

  if ((aci_src_dti_params[src_infos_psi->srcId].isDtiConnected) AND
      (aci_src_dti_params[src_infos_psi->srcId].dtxState EQ READY))
  {
    aci_bat_send_dti_data (src_infos_psi, data);
  }
  else
  {
    /* buffer data, will be send in sig_dti_tx_buffer_ready_ind() */
    if (aci_bat_buffer_dti_data (src_infos_psi, data) EQ FALSE)
    {
      return (1); /* any value larger than 0 tells L2P that there is a problem */
    }
  }

  return (0); /* what L2P expects */
}

/*------------------------------------------------------------------------------
 * L2P signals that our payload has been completely received (BAT command request or control signal)
 * control signal is immediately processed , commands are queued first
 * From the batId we derive the src_id by substract 0x80, see aci_bat_init_globs()
 */
LOCAL void aci_bat_l2p_msg_rxd_cb (U8 batId , U8 muxId, U32 dataTag, void *dataPtr, U16 dataSize)
{
  U8 src_id = batId-0x80;
  TRACE_FUNCTION("aci_bat_l2p_msg_rxd_cb()");

  if (batId EQ aci_bat_globs[src_id].l2p_id)
  {
    T_ACI_DTI_PRC_PSI *src_infos_psi = aci_bat_globs[src_id].psi_src;

    dataSize = dataSize; /* has no meaning for aci_bat */

    if (BITFIELD_CHECK(dataTag, BAT_BIT_13)) /* 0x2000 - 0x3FFF = customer commands */
    {
      aci_bat_rcv_custom (src_id, muxId, dataTag, dataPtr, dataSize);
      /* free the requested L2P RX buffer, see aci_bat_give_l2p_rx_buf_cb */  
      if (src_infos_psi->rcv_data)
      {
        ACI_MFREE(src_infos_psi->rcv_data); 
      }
    }
    else if (BITFIELD_CHECK(dataTag, BAT_BIT_15)) /* 0x8000 = BAT Lib control cmd*/
    {
      T_BATC_signal signal;
      signal.ctrl_params  = (T_BATC_ctrl_params)dataTag; /* this is the important thing */
      /* due to union of pointers, we can use any pointer, just to make the compiler happy */
      signal.params.ptr_max_clients = (T_BATC_max_clients *)dataPtr;
      aci_bat_ctrl_rcv(src_infos_psi, &signal);
      /* free the requested L2P RX buffer, see aci_bat_give_l2p_rx_buf_cb */  
      if (src_infos_psi->rcv_data)
      {
        ACI_MFREE(src_infos_psi->rcv_data); 
      }    
    }
    else /* binary AT command */
    {
      T_BAT_cmd_send cmd;  
      cmd.ctrl_params     = (T_BAT_ctrl_params)dataTag;
      cmd.params.ptr_at_a = (T_BAT_no_parameter *)dataPtr; /* we can use any pointer, don't worry */
      TRACE_EVENT_P1("command identifier: = 0x%04X", dataTag);
      TRACE_BINDUMP(aci_handle, TC_USER4, "BAT COMMAND PARAMETER 2", (U8 *)dataPtr, 4);

      /* in case of no memory, ACI goes sleeping until there is enough */
      aci_bat_queue_put(src_infos_psi->queue, &cmd, muxId, dataSize);     
      
      /* free the requested L2P RX buffer, see aci_bat_give_l2p_rx_buf_cb */  
      if (src_infos_psi->rcv_data)
      {
        ACI_MFREE(src_infos_psi->rcv_data); 
      } 
      if (src_infos_psi->run_cmd EQ FALSE)
      {
        aci_bat_run_cmd (src_infos_psi);
      }
      else
      {
        TRACE_EVENT_P1("aci_bat_l2p_msg_rxd_cb(): there is a cmd running, put 0x%04X in queue", cmd.ctrl_params);
      }
    }
  }
  else
  {
    TRACE_ERROR("aci_bat_l2p_msg_rxd_cb(): L2P used wrong identifier");
  }
}


/*------------------------------------------------------------------------------
 * AT command identifier is different in BAT and ACI. Need to map it.
 */
LOCAL T_ACI_AT_CMD  map_cmd_id (T_BAT_ctrl_params cmd)
{
  int i = 0;

  while (map_batcmd_to_acicmd[i].aci NEQ AT_CMD_NONE)
  {
      if (cmd EQ map_batcmd_to_acicmd[i].bat)
      {
        return (map_batcmd_to_acicmd[i].aci);
      }
      i++;
  }
  return (AT_CMD_NONE);
}

/*------------------------------------------------------------------------------
 * BAT Lib sent command abort request for a particular client
 */
LOCAL BOOL aci_bat_abort (T_ACI_DTI_PRC_PSI *src_infos_psi, U8 client_id)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_AT_CMD AciCurCmd = map_cmd_id ( src_infos_psi->bat_client[client_id].curCmd);

  ret =  sAT_Abort ((T_ACI_CMD_SRC)src_infos_psi->srcId, AciCurCmd);
  switch (ret)
  {
    case (AT_EXCT):
    {
      /*
       * in aci_bat_ctrl_rcv() we check for BATC_ABORT_CMD to suppress any response.
       * only call control needs some time to abort (disconnet the call).
       * command handler of call control emits the OK via R_AT (rBAT_OK).
       * in rBAT_OK we have to check again for BATC_ABORT_COMMAND_CNF and send then
       * a
       */
      src_infos_psi->bat_client[client_id].curCmd = (T_BAT_ctrl_params)BATC_ABORT_CMD;
      return(TRUE);
    }
    case ( AT_CMPL ):
    {
      src_infos_psi->bat_client[client_id].curCmd = (T_BAT_ctrl_params)-1;
      return(TRUE);
    }
    default:
    {
      switch (src_infos_psi->bat_client[client_id].curCmd)
      {
        case ( BAT_CMD_SET_PLUS_CMGC ):
        case ( BAT_CMD_SET_PLUS_CNMI ):
        case ( BAT_CMD_SET_PLUS_CSMS ):
        case ( BAT_CMD_SET_PLUS_CMGD ):
        case ( BAT_CMD_SET_PLUS_CMGR ):
        case ( BAT_CMD_SET_PLUS_CMGR_W ):
        case ( BAT_CMD_SET_PLUS_CMGW ):
        case ( BAT_CMD_SET_PLUS_CMGS ):
        case ( BAT_CMD_SET_PLUS_CMSS ):
        case ( BAT_CMD_SET_PLUS_CPMS ):
 /*       case ( AT_CMD_CSMP ):  not supported by BAT ? */
        case ( BAT_CMD_SET_PLUS_CSCA ):
        case ( BAT_CMD_SET_PLUS_CRES ):
        case ( BAT_CMD_SET_PLUS_CSAS ):
        case ( BAT_CMD_SET_PLUS_CMGL ):
        case ( BAT_CMD_SET_PLUS_CMGL_W ):
          /* ACI_ERR_DESC( ACI_ERR_CLASS_Cms, CMS_ERR_FailedToAbort); */
          break;
        default:
          /* ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_FailedToAbort); */
          break;
      }
      /* aci_bat_err(src_infos_psi); */
      break;
    }
  }
  return(FALSE);
}


/*==== GLOBAL FUNCTONS ============================================*/


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)   MODULE  : ACI_BAT                       |
| STATE   : code             ROUTINE : aci_bat_send_buffer_dti_data  |
+--------------------------------------------------------------------+

  PURPOSE : Send buffered DTI data, because dtxState is ready now.
            This function is called by sig_dti_tx_buffer_ready_ind()
*/

GLOBAL U8 aci_bat_get_l2p_id(U8 src_id)
{
  return (aci_bat_globs[src_id].l2p_id);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)   MODULE  : ACI_BAT                       |
| STATE   : code             ROUTINE : aci_bat_send_buffer_dti_data  |
+--------------------------------------------------------------------+

  PURPOSE : Send buffered DTI data, because dtxState is ready now.
            This function is called by sig_dti_tx_buffer_ready_ind()
*/
GLOBAL void aci_bat_send_buffer_dti_data (UBYTE src_id)
{
  T_ACI_DTI_PRC_PSI   *src_infos_psi = NULL;
  int i = 0;

  TRACE_FUNCTION("aci_bat_send_buffer_dti_data()");

  /* dti_id should be supported by DTI_ENTITY_ID or something, here.. */
  src_infos_psi = find_element (psi_src_params, src_id, cmhPSItest_srcId);

  if (aci_src_dti_params[src_id].isDtiConnected EQ FALSE)
  {
    TRACE_EVENT ("DTI_READY_IND: line not connected");
    return;
  }
  /*
   * send data which may have been stored ..
   */
  if (src_infos_psi->data_buffer.data_list NEQ NULL)
  {
    T_desc2 *data_list = src_infos_psi->data_buffer.data_list;  

    while (data_list NEQ NULL)
    {
      /* TRACE_BINDUMP(aci_handle, TC_USER4, "BUFFERED BAT DATA", (U8 *)data_list, data_list->len); */
      TRACE_EVENT_P1 ("aci_bat_send_buffer_dti_data(): buffered resp/ind has %i bytes", data_list->len);
      i++;
      data_list = (T_desc2*)data_list->next;
    }

    TRACE_EVENT_P1 ("aci_bat_send_buffer_dti_data(): there are %i buffered resp/ind", i);
    aci_bat_send_dti_data (src_infos_psi, src_infos_psi->data_buffer.data_list);
    src_infos_psi->data_buffer.data_list       = NULL;
    src_infos_psi->data_buffer.max_buffer_size = 0;
    src_infos_psi->data_buffer.create_time     = 0;
  }
}

/*
+-------------------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)   MODULE  : ACI_BAT                                        |
| STATE   : code             ROUTINE : aci_bat_check_for_multi_line_response_command  |
+-------------------------------------------------------------------------------------+

  PURPOSE : Some BAT commands send several responses to the application (CPBR,COPN,...).
            To avoid eating up too much memory with aci_bat_buffer_dti_data(),
            these multi line response commands send only a limited number of responses
            at once. After that the command stores the current context (how many
            responses it already sent and how many it still has to sent) and gives up
            the CPU context. With it we return to pei_primitive and the frame is able
            to pass the next primtive from ACI's input queue to the next call of pei_primitive.
            In case of a DTI_REASON_TX_BUFFER_READY we will check in this function
            whether there is still a multi line BAT command active. If so, we call this
            BAT function once again with updated parameters to send the next responses.
*/

GLOBAL void aci_bat_check_for_multi_line_response_command(UBYTE src_id)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_DTI_PRC_PSI *src_infos_psi = NULL;

  TRACE_FUNCTION("aci_bat_check_for_multi_line_response_command()");

  src_infos_psi = find_element (psi_src_params, src_id, cmhPSItest_srcId);

  if(src_infos_psi->large_type) /* large type means multi line responses */
  {
    switch (src_infos_psi->bat_client[src_infos_psi->active_client].curCmd)
    {
      case (BAT_CMD_SET_PLUS_CPBR):
      {
        ret = sBAT_PlusCPBR_CB(src_infos_psi);
        /* vsi_t_sleep (aci_handle, 3000); for testing only */
        break;
      }
      case (BAT_CMD_SET_PLUS_CPBF):
      {
        ret = sBAT_PlusCPBF_CB(src_infos_psi);
        /* vsi_t_sleep (aci_handle, 3000); for testing only */
        break;
      }
      default:
      {
        TRACE_EVENT_P1("aci_bat_check_for_multi_line_response_command(): cmd %04X has no multi line responses", 
                        src_infos_psi->bat_client[src_infos_psi->active_client].curCmd);
        return;
      }
    }

    switch (ret)
    {
      case (ACI_BAT_CMPL):
      {
        srcId_cb = src_infos_psi->srcId;
        rBAT_OK((T_ACI_AT_CMD)src_infos_psi->bat_client[src_infos_psi->active_client].curCmd);
        break;
      }
      case (ACI_BAT_EXCT):
      {
        break;
      }
      default:
      {
        aci_bat_err(src_infos_psi);
      }
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_cur_cmd_get  |
+--------------------------------------------------------------------+

  PURPOSE : Get the current command of the specified source.
*/
GLOBAL T_BAT_ctrl_params aci_bat_cur_cmd_get (T_ACI_DTI_PRC_PSI *src_infos_psi)
{
  if (src_infos_psi EQ NULL)
    return((T_BAT_ctrl_params)-1);

  if (src_infos_psi->bat_client EQ NULL)
    return((T_BAT_ctrl_params)-1);

  return(src_infos_psi->bat_client[src_infos_psi->active_client].curCmd);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_cs_get       |
+--------------------------------------------------------------------+

  PURPOSE : Get the character set, potentially for BAT or even the
            individual client, though for now it just returns the
            value of the ATI global variable 'cscsChset'.
*/
GLOBAL T_BAT_plus_cscs_cs aci_bat_cs_get (T_ACI_DTI_PRC_PSI *src_infos_psi)
{
  /*
  *   Necessary for target build?
  */
  src_infos_psi=src_infos_psi;

  /*
  *   This relies on T_BAT_plus_cscs_cs being equivalent to
  *   T_ACI_CSCS_CHSET. It is, except that the ACI version has
  *   a 'NotPresent' value, which we must deal with.
  */
  if (ati_user_output_cfg[src_infos_psi->srcId].cscsChset EQ CSCS_CHSET_NotPresent)
    return(BAT_CSCS_CS_IRA);
  else
    return((T_BAT_plus_cscs_cs)ati_user_output_cfg[src_infos_psi->srcId].cscsChset);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_cs_set       |
+--------------------------------------------------------------------+

  PURPOSE : Set the character set, potentially for BAT or even the
            individual client, though for now it just acts on the
            ATI global variable 'cscsChset'.
*/
GLOBAL void aci_bat_cs_set (T_ACI_DTI_PRC_PSI  *src_infos_psi,
                            T_BAT_plus_cscs_cs  cs)
{
  /*
  *   Necessary for target build?
  */
  src_infos_psi=src_infos_psi;

  /*
  *   This relies on T_BAT_plus_cscs_cs being equivalent to
  *   T_ACI_CSCS_CHSET. It is, except that the ACI version has
  *   a 'NotPresent' value. As the input to this function is in
  *   BAT format, this will not be a problem.
  */
  if ((cs>=BAT_CSCS_CS_IRA) AND (cs<=BAT_CSCS_CS_UCS2))
  {
    ati_user_output_cfg[src_infos_psi->srcId].cscsChset=(T_ACI_CSCS_CHSET)cs;
    TRACE_EVENT_P1 ("aci_bat_cs_set(): new charset is %i", cs);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_rcv          |
+--------------------------------------------------------------------+

  PURPOSE : called by sig_dti_data_received_ind(), see sap_dti.c
*/
GLOBAL void aci_bat_rcv (T_ACI_DTI_PRC_PSI *src_infos_psi, T_DTI2_DATA_IND *dti_data_ind)
{
  U8 src_id = src_infos_psi->srcId;
  T_desc2 *p_desc = (T_desc2 *)NULL;
  T_L2P_STATUS l2p_status = L2P_STAT_UNKNOWN_ERROR;

  TRACE_FUNCTION("aci_bat_rcv()");

  if (aci_bat_first_time)
  {
    aci_bat_first_time = FALSE;
    aci_bat_init_globs();
  }

  if (aci_bat_globs[src_id].l2p_configured EQ FALSE)
  {
    if (aci_bat_bootstrap_l2p(src_infos_psi, dti_data_ind) EQ TRUE)
    {
      aci_bat_globs[src_id].l2p_configured = TRUE;
      aci_bat_globs[src_id].psi_src = src_infos_psi;
    }
    else
    {
      return; /* trace message already in aci_bat_bootstrap_l2p */
    }
  }

  p_desc  = (T_desc2 *)dti_data_ind->desc_list2.first;
  
  if (aci_bat_globs[src_id].psi_src NEQ src_infos_psi)
  {
    /* maintenance problem, just a reminder that there is still something to do */
    TRACE_ERROR("aci_bat_rcv(): race condition between two PSI sources");
  }
  aci_bat_globs[src_id].psi_src  = src_infos_psi;

  /*
   * L2P_Receive() calls aci_bat_l2p_msg_rxd_cb() and returns with
   * a senseful value, which could used by ourself to decide what to do.
   * callback functions make it hard to follow the function flow.
   */
  l2p_status = L2P_Receive(aci_bat_globs[src_id].l2p_id,      /* unique identifier        */
                           (void *)p_desc->buffer,            /* first segment buffer     */
                           (void *)p_desc,                    /* first segment descriptor */
                           dti_data_ind->desc_list2.list_len, /* total size               */
                           p_desc->size);                     /* first segment size       */

  switch (l2p_status)
  {
    case (L2P_STAT_SUCCESS): /* BAT command has been processed */
      break;
    default: /* any error */
      TRACE_EVENT_P1("aci_bat_rcv(): L2P status = %d", l2p_status);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_ctrl_rcv     |
+--------------------------------------------------------------------+

  PURPOSE : BAT Library sent a control signal to BAT Module
*/
GLOBAL void aci_bat_ctrl_rcv (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BATC_signal *signal)
{
  T_BATC_confirm confirm;

  TRACE_FUNCTION("aci_bat_ctrl_rcv()");

  switch (signal->ctrl_params)
  {
    case (BATC_MAX_CLIENTS):
    {
      T_BATC_max_clients max_clients;
      aci_bat_init_max_client_maintenance(src_infos_psi, (U8)signal->params.ptr_max_clients->num_clients);
      confirm.rsp_params = BATC_MAX_CLIENTS_CNF;
      /* acknowledge with the same data, because L2P does not accept NULL pointer */
      confirm.rsp.ptr_bat_open_client_cnf = (T_BATC_open_client *)&max_clients;
      max_clients.num_clients = signal->params.ptr_max_clients->num_clients;
      TRACE_EVENT_P2("aci_bat_ctrl_rcv(): BAT Lib instance with %d clients for src_id %d",
                      (U8)signal->params.ptr_max_clients->num_clients, src_infos_psi->srcId);
      break;
    }
    case (BATC_OPEN_CLIENT):
    {
      T_BATC_open_client  open_client;
      src_infos_psi->bat_client[signal->params.ptr_open_client->client_id].opened = TRUE;
      confirm.rsp_params = BATC_OPEN_CLIENT_CNF;
      confirm.rsp.ptr_bat_open_client_cnf = &open_client;
      confirm.rsp.ptr_bat_open_client_cnf->client_id = signal->params.ptr_open_client->client_id;
      TRACE_EVENT_P2("aci_bat_ctrl_rcv(): BAT Lib client %d for src_id %d opened",
                      (U8)signal->params.ptr_open_client->client_id, src_infos_psi->srcId);
      break;
    }
    case (BATC_CLOSE_CLIENT):
    {
      aci_bat_clean_client_maintenance(src_infos_psi, (U8)signal->params.ptr_close_client->client_id);
      TRACE_EVENT_P2("aci_bat_ctrl_rcv(): BAT Lib client %d for src_id %d closed",
                      (U8)signal->params.ptr_close_client->client_id, src_infos_psi->srcId);
#ifndef _SIMULATION_  
      /* make DTI happy with its flow control stuff */
      psa_psi_DTI_getdata ((UBYTE)(src_infos_psi->srcId), DTI_ENTITY_PSI);
#endif
      return; /* return, because it is not confirmed to BAT library */
    }
    case (BATC_ABORT_CMD):
    {
      T_BATC_abort_cmd abort_cmd;
      U8 client_id = (U8)(signal->params.ptr_abort_cmd->client_id);
      T_BAT_ctrl_params curCmd = src_infos_psi->bat_client[client_id].curCmd;

      if (src_infos_psi->run_cmd EQ FALSE)
      {
        /*
         * there is no command running at all, but return with confirmation
         * to make the state machine of BAT Lib happy
         */
        confirm.rsp_params = BATC_ABORT_COMMAND_CNF;
        TRACE_EVENT("aci_bat_ctrl_rcv(): no command to abort");
      }     
      else if (aci_bat_abort(src_infos_psi, client_id))
      {
        confirm.rsp_params = BATC_ABORT_COMMAND_CNF;
        TRACE_EVENT_P1("aci_bat_ctrl_rcv(): aborted the command %d ", curCmd);
        src_infos_psi->run_cmd = FALSE;
      }
      else
      {
        TRACE_EVENT_P1("aci_bat_ctrl_rcv(): failed aborting command %d ", curCmd);
        confirm.rsp_params = BATC_ABORT_COMMAND_REJ;
      }
      if (src_infos_psi->bat_client[client_id].curCmd EQ (T_BAT_ctrl_params)BATC_ABORT_CMD)/*lint !e650 (Warning - constant out of range for operator) */
      {
        TRACE_EVENT_P1("aci_bat_ctrl_rcv(): executing to abort command %d ", curCmd);
        return; /* do nothing, (call must be disconnected), takes some time, then send OK via rBAT_OK*/
      }
      abort_cmd.client_id = client_id;
      confirm.rsp.ptr_bat_abort_command_cnf = &abort_cmd;
      break;
    }
    default:
      TRACE_EVENT("aci_bat_ctrl_rcv(): unknown ctrl_params");
      return; /* control signal to BAT library */
  }
  src_infos_psi->active_client = BAT_CTRL_CHANNEL; /* control channel */
  /* command responses and signal confirmations are organized in exactly the same way,
     but signal confirmation must be casted to a command response */
  aci_bat_send(src_infos_psi, (T_BAT_cmd_response *)&confirm);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_send         |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL void aci_bat_send (T_ACI_DTI_PRC_PSI  *src_infos_psi,
                          T_BAT_cmd_response *resp)
{
  T_L2P_STATUS l2p_status = L2P_STAT_UNKNOWN_ERROR;
  int errVal = -1;
  U8  client_id = 0xFF; /*broadcast */
  U16 resp_len = 0;

  TRACE_FUNCTION("aci_bat_send()");

  if (BITFIELD_CHECK(resp->ctrl_response, BAT_BIT_12) || /* 0x1xxx = unsolicited --> broadcast  */
      resp->ctrl_response EQ BAT_RES_UNS_PLUS_CRING   ||
      resp->ctrl_response EQ BAT_RES_UNS_PLUS_CCWV)
  {
    client_id = 0xFF; /* broadcast */
    TRACE_EVENT_P1("aci_bat_send(): unsolicited response 0x%04X over broadcast channel", resp->ctrl_response);
  }
  else
  {
    client_id = src_infos_psi->active_client; /* not a broadcast message */
    TRACE_EVENT_P2("aci_bat_send(): response 0x%04X on client channel %d", resp->ctrl_response, client_id);
  }

  if (BITFIELD_CHECK(resp->ctrl_response, BAT_BIT_15)) /* 0x8xxx = control response  */
  {
    resp_len = 4; /* the resp->ctrl_response (for L2P = Cmd Tag) does not count to resp length */
  }
  else
  {
    resp_len = aci_bat_get_size(resp->ctrl_response);
  }

  TRACE_EVENT_P2("aci_bat_send(): response = 0x%04X, size = %i", resp->ctrl_response, resp_len);
  TRACE_BINDUMP(aci_handle, TC_USER4, "BAT RESPONSE PARAMETER", (U8 *)resp->response.ptr_at_ok, resp_len);


   /*
    * The format of an L2 frame for BAT will be as follows :
    *
    *     For the first frame
    *     ----------------------------------------------------------------
    *     |  L2P Header       | BAT Cmd TAG |     BAT Command Data       |
    *     |      (n Bytes)    |    4 Bytes  |     (Remaining Bytes)      |
    *     ----------------------------------------------------------------
    *
    *     For any subsequent frames
    *     ----------------------------------------------------------------
    *     |  L2 Header        |     BAT Command Data                     |
    *     |      (n Bytes)    |     (Remaining Bytes)                    |
    *     ----------------------------------------------------------------
    */
  srcId_cb = src_infos_psi->srcId;

  l2p_status = L2P_Send(aci_bat_globs[src_infos_psi->srcId].l2p_id, /* unique identifier */
                        client_id,
                        resp->ctrl_response,          /* this is the important thing */
                        resp->response.ptr_at_ok,     /* don't worry, it always points to the right thing */
                        resp_len,
                        &errVal);
  switch (l2p_status)
  {
    case (L2P_STAT_SUCCESS):
      if (resp->ctrl_response <= BAT_RES_PLUS_EXT_ERROR)
      {
        if (src_infos_psi->large_type EQ FALSE) /* not CPBR, COPN, ... */
        {
          src_infos_psi->run_cmd = FALSE;
          src_infos_psi->bat_client[src_infos_psi->active_client].curCmd = (T_BAT_ctrl_params)-1;
        }
      }
      break;
    default:
      TRACE_EVENT_P1("aci_bat_send(): l2p_status = %d", l2p_status);
      TRACE_EVENT_P1("aci_bat_send(): errVal     = %d", errVal); /* what is so special ? */
      break;
  }
#ifndef _SIMULATION_
  /* make DTI happy with its flow control stuff */
  psa_psi_DTI_getdata ((UBYTE)(src_infos_psi->srcId), DTI_ENTITY_PSI);
#endif
}


GLOBAL void aci_bat_run_cmd (T_ACI_DTI_PRC_PSI *src_infos_psi)
{
  U8 client_id;
  T_BAT_cmd_send cmd;  

  TRACE_FUNCTION("aci_bat_run_cmd()");

  /* run all pending commands from the queue */
  while (aci_bat_queue_get(src_infos_psi->queue, &cmd, &client_id) EQ TRUE)
  {
    src_infos_psi->bat_client[client_id].curCmd = cmd.ctrl_params;
    src_infos_psi->active_client = client_id;
    src_infos_psi->run_cmd = TRUE;

    TRACE_EVENT_P1("client_id = %d", client_id);
    TRACE_EVENT_P1("command identifier = 0x%04X", cmd.ctrl_params);
    TRACE_BINDUMP(aci_handle, TC_USER4, "BAT COMMAND PARAMETER 3", (U8 *)cmd.params.ptr_at_a, 4);    
    
    /* run the command, result code is send back also from wrapper */
    switch (aci_bat_wrapper(src_infos_psi, &cmd)) /* run the command */
    {
      case (ACI_BAT_EXCT):
      {
        ACI_MFREE(cmd.params.ptr_andc); /* was allocated in aci_bat_queue_put() */
        return;  /* asynchronous command terminates with rBAT_OK ... rBAT_NO_ANSWER */
      }
      default:
      {
        src_infos_psi->bat_client[client_id].curCmd = (T_BAT_ctrl_params)-1;
        src_infos_psi->active_client = 0xFF;
        src_infos_psi->run_cmd = FALSE;    
        ACI_MFREE(cmd.params.ptr_andc); /* was allocated in aci_bat_queue_put() */
        break;   /* terminated, so start with the next command */
      }
    }   
  }
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_wrapper      |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT aci_bat_wrapper (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd)
{
  U16 index = 0;
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;

  if (BITFIELD_CHECK(cmd->ctrl_params, BAT_BIT_12)) /* 0x1000 */
  {
    index = (U16)(cmd->ctrl_params - BAT_BIT_12);
    if (bat_cmds_without_params[index].fptr)
    {
      ret = bat_cmds_without_params[index].fptr(src_infos_psi, cmd);
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
    }
  }
  else
  {
    index = (U16)cmd->ctrl_params;
    if (bat_cmds[index].fptr)
    {
      ret = bat_cmds[index].fptr(src_infos_psi, cmd);
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
    }
  }

  switch (ret)
  {
    case ACI_BAT_FAIL:
      /*
      *   The reason for the failure will have been stored in aciErrDesc,
      *   we first call aci_bat_err() to translate this into a BAT
      *   response message (CME ERROR / CMS ERROR / CEER) and send it.
      */
      aci_bat_err(src_infos_psi);
      break;

    case ACI_BAT_CMPL:
      /*
      *   Send OK.
      */
      {
        T_BAT_cmd_response resp;
        T_BAT_no_parameter dummy;

        resp.ctrl_response=BAT_RES_AT_OK;
        resp.response.ptr_at_ok=(T_BAT_no_parameter *)&dummy;
        dummy.bat_dummy = 0xFF;
        TRACE_EVENT("aci_bat_wrapper(): OK");
        aci_bat_send(src_infos_psi,&resp);
      }
      break;

    case ACI_BAT_BUSY:
      /*
      *   Send BUSY.
      */
      {
        T_BAT_cmd_response resp;
        T_BAT_no_parameter dummy;

        resp.ctrl_response=BAT_RES_AT_BUSY;
        resp.response.ptr_at_busy=(T_BAT_no_parameter *)&dummy;
        dummy.bat_dummy = 0xFF;
        TRACE_EVENT("aci_bat_wrapper(): BUSY");
        aci_bat_send(src_infos_psi,&resp);
      }
      break;

    case ACI_BAT_EXCT:
      /*
      *   This return value implies interaction with the protocol stack,
      *   and the response will be delivered via an rBAT_xxx(). No further
      *   action needs to be taken at this point.
      */
      TRACE_EVENT("aci_bat_wrapper(): EXCT");
#ifdef _SIMULATION_
      src_infos_psi->res.response = NULL;
#endif
      break;

    default:
      /*
      *   Reaching this point, which should not happen, may indicate that
      *   T_ACI_BAT_RSLT is no longer identical to T_ACI_RETURN - and it
      *   needs to be.
      */
      break;
  }
  return (ret);
}

#endif /* ACI_BAT_C */
