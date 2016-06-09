/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_F
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
|  Purpose :  This module defines global functions of the command
|             handler.
+-----------------------------------------------------------------------------
*/

#ifndef CMH_F_C
#define CMH_F_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/

#include "aci_cmh.h"
#include "cphs.h"
#include "aci_cphs.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "ksd.h"
#include "l4_tim.h"

#include "aci_io.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif

#ifdef DTI
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#endif
#include "psa.h"
#include "psa_mm.h"
#include "psa_sim.h"
#include "psa_cc.h"
#include "psa_sms.h"
#include "psa_ss.h"
#include "cmh.h"
#include "cmh_mm.h"
#include "cmh_sim.h"
#include "cmh_cc.h"
#include "cmh_sms.h"
#include "cmh_ss.h"

#include "aci.h"
#include "phb.h"
#include "aci_lst.h"
#include "conc_sms.h"


#ifdef FAX_AND_DATA
#include "psa_ra.h"
#include "cmh_ra.h"
#include "psa_l2r.h"
#include "cmh_l2r.h"
#include "psa_tra.h"

#ifdef FF_FAX
#include "psa_t30.h"
#include "cmh_t30.h"
#endif /* FF_FAX */

#endif /* FAX_AND_DATA */

#ifdef GPRS
#include "gaci.h"
#include "gaci_cmh.h"
#include "pcm.h"
#include "psa_gmm.h"
#include "cmh_gmm.h"
#include "psa_sm.h"
#include "cmh_sm.h"
#include "psa_gppp.h"
#include "cmh_gppp.h"

#endif /* GPRS */

#ifdef _CONC_TESTING_
#include "aci_mfw.h"
#endif

#ifdef UART
#include "psa_uart.h"
#include "cmh_uart.h"
#endif

#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#endif /*FF_PSI*/


#include "rx.h"

#ifndef _SIMULATION_
#include "../../services/ffs/ffs.h"
#include "ffs_coat.h"
#endif

/*==== CONSTANTS ==================================================*/

/*==== GLOBALS ====================================================*/
#ifdef FF_ATI
EXTERN T_CIEV_SIGNAL_BUFFER  asCievSignalBuf;
EXTERN T_CIEV_SMSFULL_BUFFER asCievSmsFullBuf;
#endif /* FF_ATI */

GLOBAL T_VOID_FUNC rat_fptr; 
/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/
/* MACRO CB_VC() for call-backs used by the voice stack */
#ifdef FF_BAT

#ifdef FF_ATI
  #if defined SMI OR defined MFW OR defined FF_MMI_RIV
    #define CB_VC( cbNam ){rAT_##cbNam,rCI_##cbNam,rBAT_##cbNam}
    #define CB_VC_EXT( cbNam ){rAT_##cbNam##_Ext,rCI_##cbNam,rBAT_##cbNam}
  #else
    #define CB_VC( cbNam ){0,rCI_##cbNam,rBAT_##cbNam} /* ACI only */
    #ifdef _CONC_TESTING_   /* for _CONC_TESTING for ACI-only */
       #define CB_VC_EXT( cbNam ){rAT_##cbNam##_Ext,rCI_##cbNam,rBAT_##cbNam}
    #else
       #define CB_VC_EXT( cbNam ){0,rCI_##cbNam,rBAT_##cbNam}
    #endif
  #endif
#else
  #if defined SMI OR defined MFW OR defined FF_MMI_RIV
    #define CB_VC( cbNam ){rAT_##cbNam,0,rBAT_##cbNam}
    #define CB_VC_EXT( cbNam ){rAT_##cbNam##_Ext,0,rBAT_##cbNam}
  #else
    #define CB_VC( cbNam ){0,0,0}
    #define CB_EXT( cbNam ){0,0,0}
  #endif
#endif

/* MACRO CB_FD() for call-backs used by the F&D stack */
#ifdef FAX_AND_DATA
  #define CB_FD( cbNam ){0,rCI_##cbNam,rBAT_##cbNam}
#else
  #define CB_FD( cbNam ){0,0,0}
#endif

/* MACRO CB_ST() for call-backs used by SIM toolkit */
#ifdef SIM_TOOLKIT
  #ifdef FF_ATI
    #if defined SMI OR defined MFW OR defined FF_MMI_RIV
      #define CB_ST( cbNam ){rAT_##cbNam,rCI_##cbNam,rBAT_##cbNam}
    #else
      #define CB_ST( cbNam ){0,rCI_##cbNam,rBAT_##cbNam}
    #endif
  #else
    #if defined SMI OR defined MFW OR defined FF_MMI_RIV
      #define CB_ST( cbNam ){rAT_##cbNam,0,rBAT_##cbNam}
    #else
      #define CB_ST( cbNam ){0,0,0}
    #endif
  #endif
#else
  #define CB_ST( cbNam ){0,0,0}
#endif

/* MACRO CB_GP() for call-backs used by GPRS */
#ifdef GPRS
  #ifdef FF_ATI
    #if defined SMI OR defined MFW OR defined FF_MMI_RIV
      #define CB_GP( cbNam ){rAT_##cbNam,rCI_##cbNam,rBAT_##cbNam}
    #else
      #define CB_GP( cbNam ){0,rCI_##cbNam,rBAT_##cbNam}
    #endif
  #else
    #if defined SMI OR defined MFW OR defined FF_MMI_RIV
      #define CB_GP( cbNam ){rAT_##cbNam,0,rBAT_##cbNam}
    #else
      #define CB_GP( cbNam ){0,0,0}
    #endif
  #endif
#else
  #define CB_GP( cbNam ){0,0,0}
#endif  /* GPRS */

#else

#ifdef FF_ATI
  #if defined SMI OR defined MFW OR defined FF_MMI_RIV
    #define CB_VC( cbNam ){rAT_##cbNam,rCI_##cbNam}
    #define CB_VC_EXT( cbNam ){rAT_##cbNam##_Ext,rCI_##cbNam}
  #else
    #define CB_VC( cbNam ){0,rCI_##cbNam}
    #ifdef _CONC_TESTING_   /* for _CONC_TESTING for ACI-only */
       #define CB_VC_EXT( cbNam ){rAT_##cbNam##_Ext,rCI_##cbNam}
    #else
       #define CB_VC_EXT( cbNam ){0,rCI_##cbNam}
    #endif
  #endif
#else
  #if defined SMI OR defined MFW OR defined FF_MMI_RIV
    #define CB_VC( cbNam ){rAT_##cbNam,0}
    #define CB_VC_EXT( cbNam ){rAT_##cbNam##_Ext,0}
  #else
    #define CB_VC( cbNam ){0,0}
    #define CB_EXT( cbNam ){0,0}
  #endif
#endif

/* MACRO CB_FD() for call-backs used by the F&D stack */
#ifdef FAX_AND_DATA
  #define CB_FD( cbNam ){0,rCI_##cbNam}
#else
  #define CB_FD( cbNam ){0,0}
#endif

/* MACRO CB_ST() for call-backs used by SIM toolkit */
#ifdef SIM_TOOLKIT
  #ifdef FF_ATI
    #if defined SMI OR defined MFW OR defined FF_MMI_RIV
      #define CB_ST( cbNam ){rAT_##cbNam,rCI_##cbNam}
    #else
      #define CB_ST( cbNam ){0,rCI_##cbNam}
    #endif
  #else
    #if defined SMI OR defined MFW OR defined FF_MMI_RIV
      #define CB_ST( cbNam ){rAT_##cbNam,0}
    #else
      #define CB_ST( cbNam ){0,0}
    #endif
  #endif
#else
  #define CB_ST( cbNam ){0,0}
#endif

/* MACRO CB_GP() for call-backs used by GPRS */
#ifdef GPRS
  #ifdef FF_ATI
    #if defined SMI OR defined MFW OR defined FF_MMI_RIV
      #define CB_GP( cbNam ){rAT_##cbNam,rCI_##cbNam}
    #else
      #define CB_GP( cbNam ){0,rCI_##cbNam}
    #endif
  #else
    #if defined SMI OR defined MFW OR defined FF_MMI_RIV
      #define CB_GP( cbNam ){rAT_##cbNam,0}
    #else
      #define CB_GP( cbNam ){0,0}
    #endif
  #endif
#else
  #define CB_GP( cbNam )
#endif  /* GPRS */

#endif /* FF_BAT */

/* call back jump table */
GLOBAL const T_VOID_FUNC RATJmpTbl[RAT_MAX][CMD_MODE_MAX] =
{
  CB_VC( OK ),            /* RAT_OK */
  CB_VC( CONNECT ),       /* RAT_CONNECT */
  CB_VC( PlusCME ),       /* RAT_CME */
  CB_VC( NO_CARRIER ),    /* RAT_NO_CARRIER */
  CB_VC( PlusCRING ),     /* RAT_CRING */
  CB_FD( PlusDR ),        /* RAT_DR */
  CB_FD( PlusCR ),        /* RAT_CR */
  CB_VC( PlusCPIN ),      /* RAT_CPIN */
  CB_VC( PlusCOPS ),      /* RAT_COPS */
  CB_VC( PlusCREG ),      /* RAT_CREG */
  CB_VC( PlusCCWA ),      /* RAT_CCWA */
  CB_VC( PlusCLIP ),      /* RAT_CLIP */
  CB_VC( PlusCDIP ),      /* RAT_CDIP */  
  CB_VC( PlusCOLP ),      /* RAT_COLP */
  CB_VC( PlusCRING_OFF ), /* RAT_CRING_OFF */

#if defined (FF_FAX) AND defined (DTI)
  CB_FD( PlusFCO ),       /* RAT_FCO */
  CB_FD( PlusFIS ),       /* RAT_FIS */
  CB_FD( PlusFTI ),       /* RAT_FTI */
  CB_FD( PlusFCS ),       /* RAT_FCS */
  CB_FD( PlusFCI ),       /* RAT_FCI */
#endif /* FF_FAX */

  CB_VC( PlusCMS  ),      /* RAT_CMS */
  CB_VC( PlusCSMS ),      /* RAT_CSMS */
  CB_VC( PlusCMGS ),      /* RAT_CMGS */
  CB_VC( PlusCMSS ),      /* RAT_CMSS */
  CB_VC( PlusCMGW ),      /* RAT_CMGW */
  CB_VC( PlusCDS  ),      /* RAT_CDS */
  CB_VC( PlusCMGC ),      /* RAT_CMGC */
  CB_VC( PlusCMGD ),      /* RAT_CMGD */
  CB_VC( PlusCMGR ),      /* RAT_CMGR */
  CB_VC( PlusCMGL ),      /* RAT_CMGL */
#if defined _CONC_TESTING_ AND defined TI_PS_FF_CONC_SMS
  CB_VC_EXT( PlusCMTI ),  /* RAT_CMTI */
  CB_VC_EXT( PlusCMT  ),  /* RAT_CMT */
#else
  CB_VC( PlusCMTI ),      /* RAT_CMTI */
  CB_VC( PlusCMT  ),      /* RAT_CMT */
#endif
  CB_VC( PlusCBM  ),      /* RAT_CBM */
  CB_VC( PlusCPMS ),      /* RAT_CPMS */

#if defined (FF_FAX) AND defined (DTI)
  
  CB_FD( PlusFHT ),       /* RAT_FHT */
  CB_FD( PlusFHR ),       /* RAT_FHR */
  CB_FD( PlusFSA ),       /* RAT_FSA */
  CB_FD( PlusFPA ),       /* RAT_FPA */
  CB_FD( PlusFPW ),       /* RAT_FPW */
  CB_FD( PlusFET ),       /* RAT_FET */
  CB_FD( PlusFVO ),       /* RAT_FVO */
  CB_FD( PlusFPO ),       /* RAT_FPO */
  CB_FD( PlusFPI ),       /* RAT_FPI */
  CB_FD( PlusFNF ),       /* RAT_FNF */
  CB_FD( PlusFNS ),       /* RAT_FNS */
  CB_FD( PlusFNC ),       /* RAT_FNC */
  CB_FD( PlusFHS ),       /* RAT_FHS */
  CB_FD( PlusFPS ),       /* RAT_FPS */
  CB_FD( PlusFTC ),       /* RAT_FTC */
#endif /* FF_FAX */
  
  CB_FD( PlusILRR ),      /* RAT_ILRR */
  CB_VC( BUSY ),          /* RAT_BUSY */
  CB_VC( NO_ANSWER ),     /* RAT_NO_ANSWER */
  CB_VC( PercentSIMREM ), /* RAT_SIMREM */
  CB_VC( PlusCLIR ),      /* RAT_CLIR */
  CB_VC( PercentCOLR ),   /* RAT_COLR */
  CB_VC( PlusCSSI ),      /* RAT_CSSI */
  CB_VC( PlusCSSU ),      /* RAT_CSSU */
  CB_VC( PlusCUSD ),      /* RAT_CUSD */
  CB_VC( PlusCCFC ),      /* RAT_CCFC */
  CB_VC( PlusCLCK ),      /* RAT_CLCK */
  CB_VC( PlusCIMI ),      /* RAT_CIMI */
  CB_ST( PercentSATI ),   /* RAT_SATI */
  CB_ST( PercentSATE ),   /* RAT_SATE */
  CB_VC( PercentKSIR ),   /* RAT_KSIR */
  CB_VC( PercentCPI  ),   /* RAT_CPI  */
  CB_VC( PercentCTYI ),   /* RAT_CTYI */
  CB_VC( PlusCNUM ),      /* RAT_CNUM */
  CB_VC( PlusCPOL ),      /* RAT_CPOL */
  CB_VC( PlusCCCM ),      /* RAT_CCCM */
  CB_VC( PercentCTV ),    /* RAT_CTV  */
  CB_ST( PercentSATN ),   /* RAT_SATN */
  CB_ST( PercentSATA ),   /* RAT_SATA */
  CB_VC( sms_ready),      /* RAT_SMS_READY */
  CB_VC( phb_status),     /* RAT_PHB_STATUS */
  CB_VC( PercentSIMINS ), /* RAT_SIMINS */
  CB_VC( PlusCRSM ),      /* RAT_CRSM */
  CB_VC( PlusCSIM ),      /* RAT_CSIM */
  CB_VC( PercentCCBS ),   /* RAT_CCBS */
  CB_VC( PlusCCWV ),      /* RAT_CCWV */
  CB_VC( PercentCNAP ),   /* RAT_CNAP */
  CB_VC( SignalSMS ),     /* RAT_SIG_SMS */
  CB_VC( PlusCLAN ),      /* RAT_CLAN */
  CB_VC( PlusCLAE ),      /* RAT_CLAE */
  CB_VC( PercentCSQ ),    /* RAT_CSQ */
  CB_VC( PercentALS ),    /* RAT_ALS */

  CB_VC( PlusCTZV ),      /* RAT_CTZR */
  CB_VC( PercentCREG ),   /* RAT_P_CREG */
#ifdef REL99
  CB_VC( PercentCMGRS ),  /* RAT_P_CMGRS */
#endif

#ifdef GPRS
  
  CB_GP( PlusCGACT ),     /* RAT_CGACT */
  CB_GP( PlusCGDATA ),    /* RAT_CGDATA */
  CB_GP( PlusCGANS ),     /* RAT_CGANS */
  CB_GP( PlusCGEREP ),    /* RAT_CGEREP */
  CB_GP( PlusCGREG ),     /* RAT_CGREG */
  CB_GP( changedQOS ),    /* RAT_QOS_MOD */
  CB_GP( PercentSNCNT ),  /* RAT_SNCNT */
  CB_GP( PercentCGREG ),  /* RAT_P_CGREG */
#ifdef REL99
  CB_GP( PlusCGCMOD ),    /* RAT_CGCMOD */
#endif /* REL99 */
  CB_GP( PercentCGEV ),    /* RAT_P_CGEV */
#endif  /* GPRS */

#ifdef FF_EM_MODE
  CB_VC( PercentEM ),     /* RAT_EM */
  CB_VC( PercentEMET ),   /* RAT_EMET */
  CB_VC( PercentEMETS ),  /* RAT_EMETS */
#endif /* FF_EM_MODE */

#ifdef FF_CPHS
  CB_VC( PercentCPNUMS ),  /* RAT_CPNUMS */
  CB_VC( PercentCPVWI ),   /* RAT_CPVWI */
  CB_VC( PercentCPROAM ),  /* RAT_CPROAM */
#endif /* FF_CPHS */
  CB_VC( PlusCIEV ),      /* RAT_CIEV */
  CB_VC( PercentRDL ),    /* RAT_RDL */
#ifdef TI_PS_FF_AT_P_CMD_RDLB
  CB_VC( PercentRDLB ),   /* RAT_RDLB */
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
  CB_VC( PercentCCCN ),   /* RAT_CCCN */
  CB_VC( PercentCSSN ),   /* RAT_CSSN */


  CB_VC( PercentCSTAT ),  /* RAT_CSTAT */



  CB_VC( Z ),             /* RAT_Z */
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
  CB_VC( PercentCPRSM ),  /* RAT_CPRSM */
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
  CB_VC( PercentCTZV ),   /*RAT_P_CTZV*/
  CB_VC( PercentCPRI ),    /*RAT_P_CPRI*/
  CB_VC( PercentSIMEF ),   /*RAT_P_SIMEF*/

  CB_VC( PercentCNIV ),    /*RAT_P_CNIV*/
  CB_VC( PercentCOPS ),   /* RAT_P_COPS */
  CB_VC( PercentCMGR ),   /* RAT_P_CMGR */
  CB_VC( PercentCMGL )    /* RAT_P_CMGL */
#ifdef FF_CPHS_REL4
  ,CB_VC( PercentCFIS ),    /* RAT_P_CFIS */
  CB_VC( PercentMWIS ),   /* RAT_P_MWIS */
  CB_VC( PercentMWI  ),   /* RAT_P_MWI  */
  CB_VC( PercentMBI  ),   /* RAT_P_MBI  */
  CB_VC( PercentMBDN )    /* RAT_P_MBDN */
#endif /* FF_CPHS_REL4 */
};

/*==== VARIABLES ==================================================*/
/* needed for function percentCSTAT_indication() */
static UBYTE percentCSTAT_stateField;

LOCAL T_ACI_CMD_MODE aci_cmd_src_mode[CMD_SRC_MAX];

/* Implements Measure#32: Row 105, 1115 & 1151 */
const char * const ffff_str = "FFFF";
/*==== FUNCTIONS ==================================================*/
LOCAL UBYTE       cmh_ClearCall      ( T_ACI_CMD_SRC srcId,
                                       T_ACI_AT_CMD cmd,
                                       UBYTE idx );
LOCAL BOOL cmh_tstAndUnflagCall( T_ACI_AT_CMD cmd , UBYTE *srcBuf );


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : cmh_Init                |
+-------------------------------------------------------------------+

  PURPOSE : Handles all deallocation of dynamic allocated memory.

*/


GLOBAL void cmh_Exit ( void )
{
  TRACE_FUNCTION ("cmh_Exit()");
#ifdef GPRS
  cmhSM_free_pdpcontext_list();
#endif
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : cmh_Init                |
+-------------------------------------------------------------------+

  PURPOSE : initialize the command handler.

*/


GLOBAL void cmh_Init ( void )
{
  T_ACI_ENTITY_ID_MSG index_entiy_id_msg;

  #ifdef FF_ATI
  UBYTE index_cievbuf_size;
  #endif

  #ifdef UART
  T_ACI_CMD_SRC index;
  #endif


  TRACE_FUNCTION ("cmh_Init()");

  mmEntStat.curCmd  = AT_CMD_NONE;
  mmEntStat.entOwn  = CMD_SRC_NONE;
  simEntStat.curCmd = AT_CMD_NONE;
  simEntStat.entOwn = CMD_SRC_NONE;
  smsShrdPrm.smsEntStat.curCmd  = AT_CMD_NONE;
  smsShrdPrm.smsEntStat.entOwn  = CMD_SRC_NONE;

#ifdef UART
  for (index=CMD_SRC_LCL; index<CMD_SRC_MAX; index++)
  {
    uartEntcurCmd[index]        = AT_CMD_NONE;
  }

  uartShrdPrm.dtr_behaviour     = DTR_BEHAVIOUR_Ignore;
  uartShrdPrm.dtr_clearcall     = FALSE;
  cmhUART_init_ipr_params();
#endif

#ifdef FF_PSI
    psiShrdPrm.dtr_behaviour     = DTR_BEHAVIOUR_Ignore;
    psiShrdPrm.dtr_clearcall     = FALSE;
#endif /*FF_PSI*/


#ifdef FAX_AND_DATA
#ifdef FF_FAX
  t30EntStat.curCmd = AT_CMD_NONE;
  t30EntStat.entOwn = CMD_SRC_NONE;
#endif

  l2rEntStat.curCmd = AT_CMD_NONE;
  l2rEntStat.entOwn = CMD_SRC_NONE;
  raEntStat.curCmd  = AT_CMD_NONE;
  raEntStat.entOwn  = CMD_SRC_NONE;
#endif

#ifndef _SIMULATION_
  /* Read status of ALSlock */
/* Implements Measure#32: Row 103 */
  if (FFS_fread (gsm_com_alslock_path,&ALSlock,sizeof(ALSlock)) NEQ sizeof(ALSlock))
    /* if failed, reset to default */
    ALSlock = ALS_MOD_NOTPRESENT;
  else if (ALSlock NEQ ALS_MOD_SPEECH AND ALSlock NEQ ALS_MOD_AUX_SPEECH)
    /* if unknown value was stored */
    ALSlock = ALS_MOD_NOTPRESENT;
#else
  ALSlock = ALS_MOD_NOTPRESENT;
#endif

#ifdef GPRS
  gaci_init();
#endif

#ifdef FF_ATI
  for( index_cievbuf_size=CMD_SRC_LCL; index_cievbuf_size < CIEV_BUF_SIZE; index_cievbuf_size ++ )
  {

    asCievSignalBuf.asBufferValues[index_cievbuf_size ]  = CIND_SIGNAL_INDICATOR_INVALID;
    asCievSmsFullBuf.asBufferValues[index_cievbuf_size ] = CIND_SMSFULL_INDICATOR_INVALID;

  }

  asCievSignalBuf.uiLastIndex  = 0;
  asCievSmsFullBuf.uiLastIndex = 0;
#endif /* FF_ATI */

  rx_Init(NULL); /* reset the signal strength callback */

  for(percentCSTAT_stateField = 0,index_entiy_id_msg = STATE_MSG_PBOOK;
      index_entiy_id_msg < STATE_MSG_MAX_ENTITIES; index_entiy_id_msg++)
  {
    percentCSTAT_stateField |= 1U << index_entiy_id_msg;
  }

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : cmh_Reset               |
+-------------------------------------------------------------------+

  PURPOSE : Reset the command handler to defaults.
*/

/* MACRO: initializer for command parameter */
#define INIT_PARM( entity, dest, def )\
    {cmhPrm[srcId].entity##CmdPrm.dest = def;}

#ifdef FAX_AND_DATA
#define INIT_PARM_FND( entity, dest, def )\
    {fnd_cmhPrm[srcId].entity##CmdPrm.dest = def;}
#endif /* FAX_AND_DATA */

GLOBAL void cmh_Reset ( T_ACI_CMD_SRC srcId, BOOL atz )
{
#ifdef FF_ATI
  UINT uiCount;
#endif

  TRACE_EVENT_P1("Parameters reset for source: %d", srcId);

  /* set default parms for command parameters */

  INIT_PARM( cc,  CLIRmode,     CLIR_MOD_Subscript      );
  INIT_PARM( cc,  DCDmode,      DCD_DISABLE_AFTER_CALL  );
  INIT_PARM( cc,  CSTAtoa.ton,  TON_Unknown             );
  INIT_PARM( cc,  CSTAtoa.npi,  NPI_IsdnTelephony       ); /* according to GSM 07.07 description of AT+CSTA */
  INIT_PARM( cc,  CSTAdef,      TRUE                    );
  INIT_PARM( cc,  CHLDmode,     CHLD_MOD_NotPresent     );
  INIT_PARM( cc,  CCUGidx,      CCUG_IDX_0              );
  INIT_PARM( cc,  CCUGinfo,     CCUG_INFO_No            );
  INIT_PARM( cc,  CCUGmode,     CCUG_MOD_DisableTmp     );
  INIT_PARM( cc,  mltyCncFlg,   0                       );
  INIT_PARM( cc,  mltyDscFlg,   0                       );

  /* Check if ALS is locked to a line */
  if (ALSlock EQ ALS_MOD_NOTPRESENT)
  {
    INIT_PARM( cc,  ALSmode,      ALS_MOD_SPEECH          ); /* Use Default line */
  }
  else
  {
    INIT_PARM( cc,  ALSmode,      ALSlock                 ); /* Lock to specific line */
  }

  INIT_PARM( cc,  CTTYmode,     CTTY_MOD_Disable        );

  INIT_PARM( cc,  CSCNcc_mode.CcCSCNModeState,     CC_CSCN_MOD_STATE_INVALID ); /* initialize the shared params for %CSCN */
  INIT_PARM( cc,  CSCNcc_mode.CcCSCNModeDirection, CC_CSCN_MOD_DIR_INVALID   );


  INIT_PARM( mm,  COPSfrmt,     COPS_FRMT_Long          );
  INIT_PARM( mm,  NRGsrvMode,   NRG_SVMD_Full           );
  INIT_PARM( mm,  NRGregMode,   NRG_RGMD_Auto           );
  INIT_PARM( mm,  NRGoprFrmt,   NRG_FRMT_Long           );
  INIT_PARM( mm,  CTZRMode,     CTZR_MODE_OFF           );
  INIT_PARM( mm,  CSQworkStat,  CSQ_Disable             );

  INIT_PARM( mm,  sIndicationParam.sMmCINDSettings.sCindSignalParam,  CIND_SIGNAL_INDICATOR_INVALID  ); /* initialize the shared params for %CIND */
  INIT_PARM( mm,  sIndicationParam.sMmCINDSettings.sCindSmsFullParam, CIND_SMSFULL_INDICATOR_INVALID );
  INIT_PARM( mm,  sIndicationParam.sMmCMERSettings.sCmerModeParam,    CMER_MODE_INVALID              );
  INIT_PARM( mm,  sIndicationParam.sMmCMERSettings.sCmerIndParam,     CMER_INDICATOR_INVALID         );
  INIT_PARM( mm,  sIndicationParam.sMmCMERSettings.sCmerBfrParam,     CMER_BFR_INVALID               );



  /* initalize SS password */
/* INIT_PARM( ss,  CXXXpwd[0],   0x0                     );  */
/* Implements Measure#32: Row 105 */
  strcpy((CHAR *)cmhPrm[srcId].ssCmdPrm.CXXXpwd, ffff_str);
  INIT_PARM( ss,  CXXXnewPwd[0],0x0                     );
  INIT_PARM( ss,  mltyTrnFlg,   0                       );

#ifdef TI_PS_FF_AT_P_CMD_CSCN
  INIT_PARM( ss,  CSCNss_mode.SsCSCNModeState,     SS_CSCN_MOD_STATE_INVALID );   /* initialize the shared params for %CSCN */
  INIT_PARM( ss,  CSCNss_mode.SsCSCNModeDirection, SS_CSCN_MOD_DIR_INVALID   );
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

  if (!atz)
    INIT_PARM( sim, CNUMActRec,     1                     );

  INIT_PARM( sim, CPOLfrmt,       CPOL_FRMT_Numeric     );
  INIT_PARM( sim, CPOLact,        CPOL_ACT_None         );
  INIT_PARM( sim, CPOLidx,        NOT_PRESENT_8BIT      );
  INIT_PARM( sim, CPOLidx2,       NOT_PRESENT_8BIT      );
  INIT_PARM( sim, CPOLmode,       CPOL_MOD_NotPresent   );

  INIT_PARM( phb, cmhStor,      PB_STOR_Ad              );
  INIT_PARM( phb, phbStor,      ADN                     );
  INIT_PARM( phb, fndRec,       0                       );
  INIT_PARM( phb, wrtRec,       0                       );
  INIT_PARM( phb, curCmd,       AT_CMD_NONE             );

#ifdef FAX_AND_DATA
  INIT_PARM_FND( l2r, CRLPiws,      L2R_K_IWF_MS_DEF        );
  INIT_PARM_FND( l2r, CRLPmws,      L2R_K_MS_IWF_DEF        );
  INIT_PARM_FND( l2r, CRLPt1,       L2R_T1_DEF              );
  INIT_PARM_FND( l2r, CRLPn2,       L2R_N2_DEF              );
  INIT_PARM_FND( l2r, DSdir,        DS_DIR_Negotiated       );
  INIT_PARM_FND( l2r, DScomp,       DS_COMP_DoNotDisc       );
  INIT_PARM_FND( l2r, DSmaxDict,    L2R_P1_DEF              );
  INIT_PARM_FND( l2r, DSmaxStr,     L2R_P2_DEF              );

#ifdef FF_FAX
  cmhT30_InitFAXPrms( srcId );
  cmhT30_RstNgtPrms();
#endif /* FF_FAX */
#endif /* FAX_AND_DATA */

  /* only if ATZ */
  if(atz)
  {
    psaCC_init_mtcbearer();
  }

#ifdef FF_ATI
  for( uiCount=0; uiCount < CIEV_BUF_SIZE; uiCount++ )
  {
    asCievSignalBuf.asBufferValues[uiCount]  = CIND_SIGNAL_INDICATOR_INVALID;
    asCievSmsFullBuf.asBufferValues[uiCount] = CIND_SMSFULL_INDICATOR_INVALID;
  }
  asCievSignalBuf.uiLastIndex  = 0;
  asCievSmsFullBuf.uiLastIndex = 0;
#endif /* FF_ATI */
  rx_Init(NULL); /* reset the signal strength callback */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : cmh_IsVldCmdSrc         |
+-------------------------------------------------------------------+

  PURPOSE : check for a valid command source.

*/

GLOBAL BOOL cmh_IsVldCmdSrc ( T_ACI_CMD_SRC srcId )
{
  if( srcId > CMD_SRC_NONE AND srcId < CMD_SRC_MAX )

    return TRUE;

  ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
  return( FALSE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : qAT_ErrDesc             |
+-------------------------------------------------------------------+

  PURPOSE : query error description of last error.

*/

GLOBAL T_ACI_ERR_DESC qAT_ErrDesc ( void )
{
  return( aciErrDesc );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : sAT_end_ussd            |
+-------------------------------------------------------------------+

  PURPOSE : Give the user the opportunity to end at any time a USSD transaction.
*/

GLOBAL T_ACI_RETURN sAT_end_ussd ( T_ACI_CMD_SRC srcId )
{
  UBYTE srcBuf;             /* buffers current source of command */
  SHORT sId;

  TRACE_FUNCTION("sAT_end_ussd( )");

  sId = psaSS_stbFindActSrv( NO_ENTRY );

  if( sId NEQ NO_ENTRY )
  {
    if(ssShrdPrm.stb[sId].ussd_operation)
    {
      psaSS_EndTrns(sId);
      ssShrdPrm.stb[sId].ntryUsdFlg = FALSE;
      ssShrdPrm.stb[sId].ussd_operation = FALSE; /* probably not really necessary  */
      return(AT_CMPL);
    }
    if(ssShrdPrm.stb[sId].curCmd EQ ((T_ACI_AT_CMD)KSD_CMD_USSD) OR
       ssShrdPrm.stb[sId].curCmd EQ AT_CMD_CUSD)
    {
      srcBuf = ssShrdPrm.stb[sId].srvOwn;
      psaSS_EndTrns(sId);
      ssShrdPrm.stb[sId].ntryUsdFlg = FALSE;
      ssShrdPrm.stb[sId].ussd_operation = FALSE; /*  probably not really necessary  */
      ssShrdPrm.stb[sId].curCmd     = AT_CMD_NONE;
      if( srcBuf NEQ srcId )
      {
        R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf )
           ( ssShrdPrm.stb[sId].curCmd, CME_ERR_Unknown );
      }
      return( AT_CMPL );
    }
  }
  return( AT_FAIL );

}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : sAT_Abort               |
+-------------------------------------------------------------------+

  PURPOSE : abort command execution for passed command.

*/

GLOBAL T_ACI_RETURN sAT_Abort ( T_ACI_CMD_SRC srcId,
                                T_ACI_AT_CMD cmd )
{
  UBYTE idx;                /* holds call table index */
  UBYTE srcBuf = 0;             /* buffers current source of command */
  BOOL  doneFlg = FALSE;    /* flags that an action was done */
  SHORT ret_val = -1;       /* Return value used when AT+COPS is aborted */  
  TRACE_FUNCTION ("sAT_Abort()");
/*
 *-------------------------------------------------------------------
 * determine if command is abortable
 *-------------------------------------------------------------------
 */
  switch( cmd )
  {
    /*
     *---------------------------------------------------------------
     * for SIM related commands
     *---------------------------------------------------------------
     */
    case( AT_CMD_CFUN ):          /* +CFUN command id */
    case( AT_CMD_BAND ):          /* %BAND command id */
    case( AT_CMD_CPIN ):          /* +CPIN command id */
      if( simEntStat.curCmd EQ cmd )
      {
        TRACE_EVENT("Command abortion for CFUN/CPIN");
        if( simEntStat.entOwn NEQ srcId )
        {
          R_AT( RAT_CME, simEntStat.entOwn )
            ( cmd, CME_ERR_Unknown );
        }
        simEntStat.entOwn = CMD_SRC_NONE;
        simEntStat.curCmd = AT_CMD_NONE;
        return( AT_CMPL );
      }
      else if( mmEntStat.curCmd EQ cmd ) /* can happen with AT%band */
      {
        TRACE_EVENT("Command abortion for %%BAND");
        if( mmEntStat.entOwn NEQ srcId )
        {
          R_AT( RAT_CME, simEntStat.entOwn )
            ( cmd, CME_ERR_Unknown );
        }
        mmEntStat.entOwn = CMD_SRC_NONE;
        mmEntStat.curCmd = AT_CMD_NONE;
        return( AT_CMPL );
      }
      break;

    /*
     *---------------------------------------------------------------
     * for phone book related commands
     *---------------------------------------------------------------
     */
    case( AT_CMD_CPBW ):
      if (cmhPrm[srcId].phbCmdPrm.curCmd EQ AT_CMD_CPBW)
      {
        cmhPrm[srcId].phbCmdPrm.curCmd = AT_CMD_NONE;
        return (AT_CMPL);
      }
      break;

    /*
     *---------------------------------------------------------------
     * for MM related commands
     *---------------------------------------------------------------
     */
    case( AT_CMD_COPS ):          /* +COPS command id */
    case( AT_CMD_P_COPS ):          /* %COPS command id */
    case( AT_CMD_NRG  ):          /* %NRG  command id */
      if( mmEntStat.curCmd EQ cmd )
      {
        TRACE_EVENT("Command abortion for COPS/NRG");
        if( mmEntStat.entOwn NEQ srcId )
        {
          R_AT( RAT_CME, mmEntStat.entOwn )
            ( cmd, CME_ERR_Unknown );
        }
        if( (cmd EQ AT_CMD_COPS) OR (cmd EQ AT_CMD_P_COPS) )
        {
          mmShrdPrm.regMode = mmShrdPrm.regModeBeforeAbort; /* AT_CMD_COPS aborted, restore regMode and COPSmode */
          mmShrdPrm.COPSmode = mmShrdPrm.COPSmodeBeforeAbort;
        
          switch (mmShrdPrm.COPSmode)
          {
            case COPS_MOD_Dereg:
#if defined (GPRS) AND defined (DTI)
              mmShrdPrm.nrgCs   = GMMREG_DT_COMB;
              ret_val = psaG_MM_CMD_DEREG ( mmShrdPrm.nrgCs );      
#else
              mmShrdPrm.nrgCs   = CS_SIM_REM;
              ret_val = psaMM_DeRegistrate();
#endif
            break;

            case COPS_MOD_Auto:
#if defined (GPRS) AND defined (DTI)
              ret_val = psaG_MM_CMD_REG ();  /* register to network */
#else
              ret_val = psaMM_Registrate();   /* register to network */
#endif
            break; 

            case COPS_MOD_Man:
              if( mmShrdPrm.usedPLMN.v_plmn EQ VLD_PLMN )
              {
                mmShrdPrm.slctPLMN = mmShrdPrm.usedPLMN;
              }
              else
              {
                mmShrdPrm.slctPLMN.v_plmn = VLD_PLMN;
                cmhMM_CnvrtINT2PLMN( 0xFFF,
                                     0xFFF,
                                     mmShrdPrm.slctPLMN.mcc,
                                     mmShrdPrm.slctPLMN.mnc );
              }

#if defined (GPRS) AND defined (DTI)
                ret_val = psaG_MM_CMD_NET_SEL();   /* register to network */
#else
                ret_val = psaMM_NetSel();          /* register to network */
#endif
            break;

            case COPS_MOD_Both:
              if (mmShrdPrm.regMode EQ MODE_AUTO)
              {
                mmShrdPrm.regModeAutoBack = FALSE;
#if defined (GPRS) AND defined (DTI)
                ret_val = psaG_MM_CMD_REG ();  /* register to network */
#else
                ret_val = psaMM_Registrate();  /* register to network */
#endif
              }
              else
              {
                mmShrdPrm.regModeAutoBack = TRUE;
                if( mmShrdPrm.usedPLMN.v_plmn EQ VLD_PLMN )
                {
                  mmShrdPrm.slctPLMN = mmShrdPrm.usedPLMN;
                }
                else
                {
                  mmShrdPrm.slctPLMN.v_plmn = VLD_PLMN;
                  cmhMM_CnvrtINT2PLMN( 0xFFF,
                                       0xFFF,
                                       mmShrdPrm.slctPLMN.mcc,
                                       mmShrdPrm.slctPLMN.mnc );
                }
              }
#if defined (GPRS) AND defined (DTI)
                  ret_val = psaG_MM_CMD_NET_SEL();   /* register to network */
#else
                  ret_val = psaMM_NetSel();          /* register to network */
#endif
            break;
          } /* end of switch case */

          if (ret_val < 0)
          {
            TRACE_EVENT( "FATAL RETURN in sAT_Abort()" );
            ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
          }
        } /* end of AT_CMD_COPS */

        mmEntStat.entOwn = CMD_SRC_NONE;
        mmEntStat.curCmd = AT_CMD_NONE;
        return( AT_CMPL );
      }
    break;

    /*
     *---------------------------------------------------------------
     * for CC related commands
     *---------------------------------------------------------------
     */
    case( AT_CMD_D    ):          /* D     command id */

      for (idx = 0; idx < MAX_CALL_NR; idx++)
      {
        if (ccShrdPrm.ctb[idx] NEQ NULL)
        {
          if ((psaCC_ctb(idx)->curCmd EQ cmd AND
              (psaCC_ctb(idx)->calStat EQ CS_ACT_REQ OR
               psaCC_ctb(idx)->calStat EQ CS_DSC_REQ))
           OR
              (psaCC_ctb(idx)->calStat EQ CS_ACT AND
              (cmhCC_getcalltype(idx) NEQ VOICE_CALL)))
          {
            doneFlg = TRUE;
            /* Implements Measure 178 */
            srcBuf = cmh_ClearCall( srcId, AT_CMD_D, idx );
          }
        }
      }
      if( doneFlg EQ TRUE)
        return( AT_EXCT );

      /* Implements Measure 34 */
      doneFlg = cmh_tstAndUnflagCall( cmd, &srcBuf );

      if( doneFlg )
      {
        if( /*lint -e(644)*/srcBuf NEQ srcId )
        {
          R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf )
            ( AT_CMD_D, CME_ERR_Unknown );
        }
        return( AT_CMPL );
      }

      for( idx = 0; idx < MAX_SS_NR; idx++ )
      {
        if( (ssShrdPrm.stb[idx].curCmd EQ ((T_ACI_AT_CMD)KSD_CMD_CB)  OR
             ssShrdPrm.stb[idx].curCmd EQ ((T_ACI_AT_CMD)KSD_CMD_CF)  OR
             ssShrdPrm.stb[idx].curCmd EQ ((T_ACI_AT_CMD)KSD_CMD_CW)  OR
             ssShrdPrm.stb[idx].curCmd EQ ((T_ACI_AT_CMD)KSD_CMD_CL)  OR
             ssShrdPrm.stb[idx].curCmd EQ ((T_ACI_AT_CMD)KSD_CMD_PWD) OR
             ssShrdPrm.stb[idx].curCmd EQ ((T_ACI_AT_CMD)KSD_CMD_USSD)    ))
        {
          srcBuf = ssShrdPrm.stb[idx].srvOwn;
          /* PATCH LE 26.06.00  */
          /* End transaction in SS */
          psaSS_EndTrns(idx);
          /* END PATCH LE 26.06.00 */
          ssShrdPrm.stb[idx].ntryUsdFlg = FALSE;
          ssShrdPrm.stb[idx].curCmd     = AT_CMD_NONE;
          doneFlg = TRUE;
        }
      }
      if( doneFlg )
      {
        if( srcBuf NEQ srcId )
        {
          R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf )
            ( AT_CMD_D, CME_ERR_Unknown );
        }
        return( AT_CMPL );
      }
      break;

    case( AT_CMD_A    ):          /* A     command id */

      for( idx = 0; idx < MAX_CALL_NR; idx++ )
      {
        if (ccShrdPrm.ctb[idx] NEQ NULL)
        {
          if (psaCC_ctb(idx)->curCmd EQ cmd AND
              (psaCC_ctb(idx)->calStat EQ CS_ACT_REQ OR
               psaCC_ctb(idx)->calStat EQ CS_CPL_REQ OR
               psaCC_ctb(idx)->calStat EQ CS_DSC_REQ))
          {
            doneFlg = TRUE;
            /* Implements Measure 178 */
            srcBuf = cmh_ClearCall( srcId, AT_CMD_A, idx );
          }
        }
      }
      if( doneFlg )
        return( AT_EXCT );

      /* Implements Measure 34 */
      doneFlg = cmh_tstAndUnflagCall( cmd, &srcBuf);

      if( doneFlg )
      {
        if( srcBuf NEQ srcId )
        {
          R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf )
            ( AT_CMD_A, CME_ERR_Unknown );
        }
        return( AT_CMPL );
      }
//patch for OMAPS00173631: if there is no call need to answer. just return AT_CMPL
      return( AT_CMPL );
      break;

    case( AT_CMD_Z    ):          /* Z     command id */
    case( AT_CMD_CHUP ):          /* +CHUP command id */
    case( AT_CMD_CHLD ):          /* +CHLD command id */
    case( AT_CMD_CTFR ):          /* +CTFR command id */

      for( idx = 0; idx < MAX_CALL_NR; idx++ )
      {
        if (ccShrdPrm.ctb[idx] NEQ NULL AND
            psaCC_ctb(idx)->curCmd EQ cmd)
        {
          srcBuf = psaCC_ctb(idx)->curSrc;

          cmhCC_tstAndUnflagCall( idx, &cmhPrm[srcBuf].ccCmdPrm.mltyCncFlg );
          cmhCC_tstAndUnflagCall( idx, &cmhPrm[srcBuf].ccCmdPrm.mltyDscFlg );

          psaCC_ctb(idx)->curCmd = AT_CMD_NONE;
          psaCC_ctb(idx)->curSrc = CMD_SRC_NONE;
          doneFlg = TRUE;
        }
      }
      if( doneFlg )
      {
        if( /*lint -e(644)*/srcBuf NEQ srcId )
        {
          R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf )
            ( cmd, CME_ERR_Unknown );
        }
        return( AT_CMPL );
      }
      break;

    case( AT_CMD_VTS ):          /* +VTS command id */

      for( idx = 0; idx < MAX_CALL_NR; idx++ )
      {
        if (ccShrdPrm.ctb[idx] NEQ NULL AND
            psaCC_ctb(idx)->dtmfCmd EQ cmd)
        {
          if (psaCC_ctb(idx)->dtmfMode NEQ MNCC_DTMF_MOD_AUTO) /* DTMF in Auto Mode is not abortable
                                                                 since the tone is always immediately stopped
                                                                 after the network confirmed the tone to CC. So
                                                                 it makes absolut no sense to abort the VTS nor
                                                                 does this have any effect on the length of the tone. */
          {
            srcBuf = psaCC_ctb(idx)->dtmfSrc;
            psaCC_ctb(idx)->dtmfCmd = AT_CMD_NONE;
            psaCC_ctb(idx)->dtmfSrc = (T_OWN)CMD_SRC_NONE;
            sAT_PlusVTS( srcId, ccShrdPrm.dtmf.dig[0], VTS_MOD_ManStop );
            doneFlg = TRUE;
          }
        }
      }
      if( doneFlg )
      {
        if( srcBuf NEQ srcId )
        {
          R_AT( RAT_CME,(T_ACI_CMD_SRC)srcBuf )
            ( AT_CMD_VTS, CME_ERR_Unknown );
        }
        return( AT_CMPL );
      }

      break;

    /*
     *---------------------------------------------------------------
     * for SS related commands
     *---------------------------------------------------------------
     */
    case( AT_CMD_CLIR ):          /* +CLIR command id */
    case( AT_CMD_CLIP ):          /* +CLIP command id */
    case( AT_CMD_COLP ):          /* +COLP command id */
    case( AT_CMD_CCFC ):          /* +CCFC command id */
    case( AT_CMD_CLCK ):          /* +CLCK command id */
    case( AT_CMD_CCWA ):          /* +CCWA command id */
    case( AT_CMD_CPWD ):          /* +CPWD command id */
    case( AT_CMD_CUSD ):          /* +CUSD command id */

      for( idx = 0; idx < MAX_SS_NR; idx++ )
      {
        if( ssShrdPrm.stb[idx].curCmd EQ cmd )
        {
          srcBuf = ssShrdPrm.stb[idx].srvOwn;
          psaSS_EndTrns(idx);
          cmhPrm[srcId].ssCmdPrm.mltyTrnFlg = 0;
          ssShrdPrm.stb[idx].ntryUsdFlg = FALSE;
          ssShrdPrm.stb[idx].curCmd     = AT_CMD_NONE;
          doneFlg = TRUE;
        }
      }
      if( simEntStat.curCmd EQ AT_CMD_CLCK )
      {
        simEntStat.curCmd = AT_CMD_NONE;
        doneFlg = TRUE;
      }
      if( doneFlg )
      {
        if( srcBuf NEQ srcId )
        {
          R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf )
            ( cmd, CME_ERR_Unknown );
        }
        return( AT_CMPL );
      }
      break;                      /* SS not implemented */

    /*
     *---------------------------------------------------------------
     * for SMS related commands
     *---------------------------------------------------------------
     */
    case( AT_CMD_CMGD ):          /* +CMGD command id */
    case( AT_CMD_CMGW ):          /* +CMGW command id */
    case( AT_CMD_CMGS ):          /* +CMGS command id */
    case( AT_CMD_CMSS ):          /* +CMSS command id */
    case( AT_CMD_CSMP ):          /* +CSMP command id */
    case( AT_CMD_CSCA ):          /* +CSCA command id */

      if( smsShrdPrm.smsEntStat.curCmd EQ cmd )
      {
        if( smsShrdPrm.smsEntStat.entOwn NEQ srcId )
        {
          R_AT( RAT_CMS, smsShrdPrm.smsEntStat.entOwn )
            ( cmd, CMS_ERR_UnknownErr, NULL );
        }

        smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
        smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
        return( AT_CMPL );
      }

      break;

#ifdef GPRS
    /* brz:to check
     *---------------------------------------------------------------
     * for GMM related commands
     *---------------------------------------------------------------
     */
    case( AT_CMD_CGATT ):         /* +CGATT command id */
    case( AT_CMD_CGCLASS ):       /* +CGCLASS command id */
      if( gmmEntStat.curCmd EQ cmd )
      {
        TRACE_EVENT("Command abortion for CGATT/CGCLASS/CGAUTO");
        if( gmmEntStat.entOwn NEQ srcId )
        {
          R_AT( RAT_CME, gmmEntStat.entOwn )
            ( cmd, CME_ERR_Unknown );
        }
        gmmEntStat.entOwn = CMD_SRC_NONE;
        gmmEntStat.curCmd = AT_CMD_NONE;
        return( AT_CMPL );
      }

      break;
    /*
     *---------------------------------------------------------------
     * for SM related commands
     *---------------------------------------------------------------
     */
    case( AT_CMD_CGACT ):         /* +CGACT command id */
    case( AT_CMD_CGDATA ):        /* +CGDATA command id */
      if( gpppEntStat.curCmd EQ cmd OR
            smEntStat.curCmd EQ cmd    )
      {
        TRACE_EVENT("Command abortion for CGDATA or CGACT");
        if( gpppEntStat.entOwn NEQ srcId  AND
              smEntStat.entOwn NEQ srcId     )
        {
          R_AT( RAT_CME, gpppEntStat.entOwn )
            ( cmd, CME_ERR_Unknown );
        }

        /*
        * Deactivate all the activated contexts in the work cid list. Starting from the one
        * that is currently being activated. The work_cids array is reversed.
        */
        {
          U8 i, temp_work_cids[PDP_CONTEXT_CID_MAX + 1];
          memset(temp_work_cids, INVALID_CID, sizeof(temp_work_cids));
          for (i=0; i<=cid_pointer; i++)
              temp_work_cids[cid_pointer - i] = work_cids[i];
          memcpy(work_cids, temp_work_cids, sizeof(work_cids));
          cid_pointer = 0;
        }
        smEntStat.curCmd = AT_CMD_NONE;
        gpppEntStat.curCmd = AT_CMD_NONE;
       /*
        * Fake that the current command is +CGACT=0
        */
        return cmhSM_deactivateAContext(srcId, work_cids[cid_pointer] );
      }
      break;

    case( AT_CMD_CGANS ):         /* +CGANS command id */

      if( smEntStat.curCmd EQ cmd )
      {
        TRACE_EVENT("Command abortion for CGANS");
        if( smEntStat.entOwn NEQ srcId )
        {
          R_AT( RAT_CME, smEntStat.entOwn )
            ( cmd, CME_ERR_Unknown );
        }
        smEntStat.entOwn = CMD_SRC_NONE;
        smEntStat.curCmd = AT_CMD_NONE;
        doneFlg = TRUE;
      }

      if( gpppEntStat.curCmd EQ cmd )
      {
        if( gpppEntStat.entOwn NEQ srcId )
        {
          R_AT( RAT_CME, gpppEntStat.entOwn )
            ( cmd, CME_ERR_Unknown );
        }
        gpppEntStat.entOwn = CMD_SRC_NONE;
        gpppEntStat.curCmd = AT_CMD_NONE;
        return( AT_EXCT );
      }

      if( doneFlg EQ TRUE )
        return( AT_EXCT );
      break;

    case( AT_CMD_CGSMS ):         /* +CGSMS command id */
      if( smEntStat.curCmd EQ cmd )
      {
        TRACE_EVENT("Command abortion for CGSMS");
        if( smEntStat.entOwn NEQ srcId )
        {
          R_AT( RAT_CME, smEntStat.entOwn )
            ( cmd, CME_ERR_Unknown );
        }
        smEntStat.entOwn = CMD_SRC_NONE;
        smEntStat.curCmd = AT_CMD_NONE;
      }
      return AT_CMPL;
    /*
     *---------------------------------------------------------------
     * for SM related UMTS commands
     *---------------------------------------------------------------
     */
#ifdef REL99
    case( AT_CMD_CGCMOD ):         /* +CGCMOD command id */
      if( smEntStat.curCmd EQ cmd )
      {
        TRACE_EVENT("Command abortion for CGCMOD");
        if( smEntStat.entOwn NEQ srcId )
        {
          R_AT( RAT_CME, smEntStat.entOwn )
            ( cmd, CME_ERR_Unknown );
        }
        switch(get_state_working_cid())
        {
          case PDP_CONTEXT_STATE_ACTIVATED_MODIFYING:
            /* Reset the state and remove the command. CmhSM will ignore any SMREG_PDP_MODIFY_CNF */
            set_state_working_cid( PDP_CONTEXT_STATE_ACTIVATED );
            smEntStat.entOwn = CMD_SRC_NONE;
            smEntStat.curCmd = AT_CMD_NONE;
            work_cids[0]     = PDP_CONTEXT_CID_INVALID;
            cid_pointer      = PDP_CONTEXT_CID_OMITTED;
            break;

          case PDP_CONTEXT_STATE_DATA_LINK_MODIFYING:
            /* Reset the state and remove the command. CmhSM will ignore any SMREG_PDP_MODIFY_CNF */
            set_state_working_cid( PDP_CONTEXT_STATE_DATA_LINK );
            smEntStat.entOwn = CMD_SRC_NONE;
            smEntStat.curCmd = AT_CMD_NONE;
            work_cids[0]     = PDP_CONTEXT_CID_INVALID;
            cid_pointer      = PDP_CONTEXT_CID_OMITTED;
            break;

          default:
            return( AT_FAIL );
        }
        return( AT_CMPL );
      }
#endif /* REL99 */

#endif /*GPRS*/

  }

  return( AT_FAIL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_mergeTOA            |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the type of number (TON) and the
            numbering plan identification into the octet representation
            according to GSM 04.08 ( 10.5.4.7 ).
*/

GLOBAL UBYTE cmh_mergeTOA ( UBYTE ton, UBYTE npi )
{

/*
 *-------------------------------------------------------------------
 * convert TON and NPI
 *-------------------------------------------------------------------
 */
  return( ((ton & 0x07)<<4)+
          ((npi & 0x0F)<<0)+0x80 );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_mergeTOS            |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the type of subaddress (TOS) and the
            odd/even indicator into the octet representation
            according to GSM 04.08 ( 10.5.4.8 ).

*/

GLOBAL UBYTE cmh_mergeTOS ( UBYTE tos, UBYTE oe )
{

/*
 *-------------------------------------------------------------------
 * convert TOS and odd/even
 *-------------------------------------------------------------------
 */
  return( ((tos & 0x07)<<4)+
          ((oe & 0x01)<<3)+0x80 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_demergeTOA          |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the the octet representation
            according to GSM 04.08 ( 10.5.4.7 ) into type of number
            (TON) and the numbering plan identification.
*/

GLOBAL void cmh_demergeTOA ( UBYTE toa, UBYTE* ton, UBYTE* npi )
{

/*
 *-------------------------------------------------------------------
 * convert TON and NPI
 *-------------------------------------------------------------------
 */
  *ton = (toa>>4) & 0x07;
  *npi = (toa>>0) & 0x0F;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_demergeTOS          |
+-------------------------------------------------------------------+

  PURPOSE : this function converts the octet representation
            according to GSM 04.08 ( 10.5.4.8 ) into the type of
            subaddress (TOS) and the odd/even indicator.

*/

GLOBAL void cmh_demergeTOS ( UBYTE tosa, UBYTE* tos, UBYTE* oe )
{

/*
 *-------------------------------------------------------------------
 * convert TOS and odd/even
 *-------------------------------------------------------------------
 */
  *tos = (tosa>>4) & 0x07;
  *oe  = (tosa>>3) & 0x01;
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMH_F                      |
| STATE   : code                ROUTINE : cmh_setToaDef              |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the type of address to default values
            (when the first character of address is '+' or the first
            two characters are '0' default is 145 otherwise default
            is 129)
*/
GLOBAL CHAR* cmh_setToaDef ( CHAR* number, T_ACI_TOA *toa )
{
  toa->npi = NPI_IsdnTelephony;

  if ( *number EQ '+' )
  {
    toa->ton = TON_International;
    return number+1;
  }
  else
  {
    toa->ton = TON_Unknown;
    return number;
  }
}




/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_packBCD             |
+-------------------------------------------------------------------+

  PURPOSE : packing two BCD values in one octet
            <bcd_out>   resulting packed BCD numbers
            <char_in>   BCD values input per character
            <len_in>    number of BCD values to be packed
            RETURN :    number of packed octets
*/

GLOBAL USHORT cmh_packBCD (UBYTE *bcd_out, const UBYTE *char_in,
                           USHORT len_in)
{
  USHORT no_oct = 0, i;

  if (len_in > MAX_SMS_ADDR_DIG)            /* check length */
    len_in = MAX_SMS_ADDR_DIG;

  for (i = 0; i < len_in; i++)
  {
    if ((i & 1) EQ 0)                     /* pack high nibble */
    {
      no_oct++;
      *bcd_out = *char_in++ | 0xF0;       /* in case of odd length */
    }
    else                                  /* pack low nibble */
      *bcd_out++ &= (*char_in++ << 4) | 0xF;
  }
  return no_oct;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_unpackBCD           |
+-------------------------------------------------------------------+

  PURPOSE : unpacking one octet in two BCD values
            <char_out>  resulting unpacked BCD values
            <bcd_in>    BCD numbers input
            <len_in>    number of octets to be unpacked
            RETURN :    number of unpacked BCD values
*/

GLOBAL USHORT cmh_unpackBCD (UBYTE *char_out, const UBYTE *bcd_in,
                             USHORT len_in)
{
  USHORT no_bcd = 0, i;

  if (len_in > MAX_SMS_ADDR_DIG/2)          /* check length */
    len_in = MAX_SMS_ADDR_DIG/2;

  for (i = 0; i < len_in; i++)
  {
    if ((*bcd_in & 0xF) NEQ 0xF)          /* in case of wrong length */
    {
      *char_out++ = *bcd_in & 0xF;        /* unpack low nibble */
      no_bcd++;
    }
    else
      break;
    if ((*bcd_in & 0xF0) NEQ 0xF0)        /* in case of odd length */
    {
      *char_out++ = *bcd_in++ >> 4;       /* unpack high nibble */
      no_bcd++;
    }
    else
      break;
  }
  return no_bcd;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_unpackSCTS          |
+-------------------------------------------------------------------+

  PURPOSE : unpacking SCTS (or validity period absolute) to the
            T_ACI_VP_ABS structure
*/
GLOBAL void cmh_unpackSCTS (T_ACI_VP_ABS *scts, const UBYTE *buf_in)
{

  UBYTE lsb, msb, sign;

  cmh_unpackBCD ((UBYTE*)scts, buf_in, 6);

  /* get the time zone octet */
  buf_in += 6;

  lsb = *buf_in >> 4;
  msb = *buf_in & 0x07;
  scts->timezone = (msb *10) + lsb;  /* BCD */
  sign = *buf_in & 0x08;
  if (sign)
    scts->timezone = -scts->timezone; /* ~scts->timezone + 1; */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_logRslt             |
+-------------------------------------------------------------------+

  PURPOSE : log result code.

*/

GLOBAL void cmh_logRslt ( T_ACI_CMD_SRC dest, RAT_ID rat,
                          T_ACI_AT_CMD cmd,   SHORT cId,
                          T_ACI_BS_SPEED spd, T_ACI_CME_ERR err )
{
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  T_ACI_RLOG rslt;          /* holds result pararmeter */

  rslt.dest = dest;

  switch( rat )
  {
    case( RAT_OK ):

      rslt.atRslt = RLOG_RSLT_OK;
      rslt.rsltPrm.rOK.cmdId = cmd;
      rslt.rsltPrm.rOK.cId   = cId;
      break;

    case( RAT_NO_CARRIER ):

      rslt.atRslt = RLOG_RSLT_NoCarrier;
      rslt.rsltPrm.rNO_CARRIER.cmdId = cmd;
      rslt.rsltPrm.rNO_CARRIER.cId   = cId;
      break;

    case( RAT_CONNECT ):

      rslt.atRslt = RLOG_RSLT_Connect;
      rslt.rsltPrm.rCONNECT.cmdId = cmd;
      rslt.rsltPrm.rCONNECT.cId   = cId;
      rslt.rsltPrm.rCONNECT.speed = spd;
      break;

    case( RAT_BUSY ):

      rslt.atRslt = RLOG_RSLT_Busy;
      rslt.rsltPrm.rBUSY.cmdId = cmd;
      rslt.rsltPrm.rBUSY.cId   = cId;
      break;

    case( RAT_NO_ANSWER ):

      rslt.atRslt = RLOG_RSLT_NoAnswer;
      rslt.rsltPrm.rNO_ANSWER.cmdId = cmd;
      rslt.rsltPrm.rNO_ANSWER.cId   = cId;
      break;

    case( RAT_CME ):

      rslt.atRslt = RLOG_RSLT_CME;
      rslt.rsltPrm.rCME.cmdId = cmd;
      rslt.rsltPrm.rCME.err   = err;
      rslt.rsltPrm.rCME.cId   = cId;
      break;

    default:

      return;
  }

  rAT_PercentRLOG( &rslt );
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_cvtToDefGsm         |
+-------------------------------------------------------------------+

  PURPOSE : converts from internal GSM to default GSM alphabet.

*/
GLOBAL void cmh_cvtToDefGsm ( CHAR* in, CHAR* out, USHORT* len )
{
  USHORT i; /* used for counting */

  *len = ( USHORT ) strlen ( in );

  for ( i = 0; i < *len; i++ )
    out[i] = in[i] & 0x7F;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_cvtFromDefGsm       |
+-------------------------------------------------------------------+

  PURPOSE : converts from default GSM to internal GSM alphabet.

*/
GLOBAL void cmh_cvtFromDefGsm ( CHAR* in, USHORT len, CHAR* out )
{
  USHORT i; /* used for counting */

  for ( i = 0; i < len; i++ )
    out[i] = in[i] | 0x80;

  out[len] = '\0';
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  ACI_CMD             |
| STATE  : code                         ROUTINE: cmh_set_delayed_call|
+--------------------------------------------------------------------+

  PURPOSE : Set function pointer for general-purpose timer routine.
            TRUE, if delayed call timer slot is available
            FALSE otherwise
            setting call=NULL always succeeds.
*/
static UCHAR (*cmh_timeout_delayed_call) (void*) = NULL;
static void* cmh_timeout_delayed_arg = NULL;

GLOBAL UBYTE cmh_set_delayed_call (UCHAR (*call) (void*), void* arg)
{
  UCHAR ret;
  if (cmh_timeout_delayed_call EQ NULL
   OR cmh_timeout_delayed_call EQ call
   OR call EQ NULL)
  {
    cmh_timeout_delayed_call = call;
    cmh_timeout_delayed_arg = arg;
    ret = TRUE;
  }
  else ret = FALSE;
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  ACI_CMD             |
| STATE  : code                         ROUTINE: cmh_set_delayed_call|
+--------------------------------------------------------------------+

  PURPOSE : start timer for general-purpose timer routine.
            TRUE, if delayed call timer slot has been set before
*/

GLOBAL UBYTE cmh_start_delayed_call (ULONG ms)
{
  UBYTE ret;
  if (cmh_timeout_delayed_call NEQ NULL) {
    TRACE_FUNCTION ("cmh_start_delayed_call()");
    vsi_t_start (hCommACI, ACI_TDELAY, ms);
    ret = TRUE;
  } else ret = FALSE;
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  ACI_CMD             |
| STATE  : code                         ROUTINE: cmh_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : A timeout has occured for a timer. The function returns
            TRUE, if it is a timer which is handled by CMH, else FALSE
            is returned to indicate that the timer has not been
            processed.

*/

GLOBAL UBYTE cmh_timeout (USHORT index)
{
  TRACE_FUNCTION ("cmh_timeout()");

  if (index EQ ACI_TMPTY)
  {
    /*
     * timeout multiparty timer
     */
    cmhCC_MPTYTimeout();
    return TRUE;
  }

  if (index EQ ACI_TECT)
  {
    /*
     * timeout multiparty timer
     */
    cmhCC_ECTTimeout();
    return TRUE;
  }

#ifdef FF_FAX
  if (index EQ ACI_TFIT)
  {
    /*
     * timeout FAX inactivity timer
     */
    cmhT30_FITTimeout();
    return TRUE;
  }
#endif /* FF_FAX */

  if (index EQ ACI_TDELAY)
  {
    /*
     * timeout delayed call timer
     */
    if (cmh_timeout_delayed_call EQ NULL)
    {
      vsi_t_stop (hCommACI, ACI_TDELAY);
    }
    else if (!cmh_timeout_delayed_call (cmh_timeout_delayed_arg))
      {
        cmh_timeout_delayed_call = NULL;
        vsi_t_stop (hCommACI, ACI_TDELAY);
      }
    return TRUE;
  }

  if (index EQ ACI_REPEAT_HND)
  {
    /* redial timer is elapsed */
    cmhCC_redialTimeout();
  }

#ifdef SIM_TOOLKIT
  if(index EQ ACI_SAT_MAX_DUR_HND)
  {
    /* SAT max redialling duration elapsed */
    cmhCC_satTimeout();
  }
#endif /* SIM_TOOLKIT */

 #ifdef SIM_PERS
  if(index EQ ACI_UNLOCK_TIMER_HANDLE)
  {
    cmhSIM_UnlockTimeout();
    return TRUE;
  }
 #endif
  
  /*
   * the timeout is not for CMH
   */
  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : cmh_bldCalPrms          |
+-------------------------------------------------------------------+

  PURPOSE : this function converts a dial string into the settings
            for the called address parameters.
*/

/*******************************************
This function is intended only to be called ONCE
for a given number...
Otherwise, TOA info might be corrupted
********************************************/
GLOBAL SHORT cmh_bldCalPrms ( char * pDialStr, T_CLPTY_PRM * calPrm )
{
  char * pSubAdr;           /* points to subaddress */
  int   i;
  int   len = strlen(pDialStr);

  TRACE_FUNCTION("cmh_bldCalPrms()");

/* seach for subaddress */
  pSubAdr = strchr( pDialStr, '-' );
  if ( pSubAdr NEQ NULL )    /* subaddress found */
  {
    *pSubAdr = 0x0; /* cut off subaddress */
    pSubAdr++;

    /*
     * fill in subaddress information
     */
    calPrm -> tos = MNCC_TOS_NSAP;
    strncpy( calPrm->sub, pSubAdr, MAX_SUBADDR_LEN-1 );
    calPrm->sub[MAX_SUBADDR_LEN-1] = 0x0;
    calPrm -> oe  = (strlen(calPrm->sub) & 1)? MNCC_OE_ODD : MNCC_OE_EVEN;
  }
  else            /* subaddress not found */
  {
    calPrm -> tos = MNCC_TOS_NOT_PRES;
    calPrm -> oe  = MNCC_OE_EVEN;
    calPrm->sub[0]    = 0x0;
  }

/* fill in address information */

  /* Sign "+" could be found anywhere in the string:
  for instance in a Key Stroke Sequence */

  /* Copy character until '+' digit */
  for (i = 0; (i < len) AND (pDialStr[i] NEQ '+'); i++)
    calPrm->num[i] = pDialStr[i];

  if (i NEQ len)
  {
    calPrm->ton = MNCC_TON_INT_NUMB;

    /* Skip '+' digit and copy next characters */
    for (i++; i < len; i++)
      calPrm->num[i-1] = pDialStr[i];
    i--;
  }
  else
    calPrm->ton = MNCC_TON_UNKNOWN;

  /* Copy EOS */
  calPrm->num[i] = '\0';

  calPrm -> npi = MNCC_NPI_ISDN_TEL_NUMB_PLAN;

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_F                   |
|                                 ROUTINE : percentCSTAT_stateField |
+-------------------------------------------------------------------+

  PURPOSE : This function calls the %CSTAT callback and
            handels the occurance of %CSTAT and
            %CSTAT: RDY indication.
*/
GLOBAL void percentCSTAT_indication(T_ACI_ENTITY_ID_MSG     entityId,
                                    T_ACI_ENTITY_STATE_MSG  entityState)
{
  T_ACI_STATE_MSG msgType;
  int i = 0;
  msgType.entityId    = entityId;
  msgType.entityState = entityState;

  TRACE_FUNCTION("percentCSTAT_indication");
 
  /* ! EONS ready does not impact %CSTAT: RDY ! */
  if (entityId EQ STATE_MSG_EONS)
  {
    entityState = ENTITY_STATUS_Ready;
  }

  /* Ready indication should be sent only once for all the entities */

  if( (percentCSTAT_stateField >> entityId & 1U) EQ (unsigned) entityState )
  {
    for(; i < CMD_SRC_MAX; i++)
    {
      R_AT( RAT_CSTAT, (T_ACI_CMD_SRC)i)(msgType);
    }
    percentCSTAT_stateField ^= 1U << entityId;
  
  if(!percentCSTAT_stateField)
  {
    msgType.entityId = STATE_MSG_RDY;
    for(i = 0; i < CMD_SRC_MAX; i++)
    {
        R_AT( RAT_CSTAT, (T_ACI_CMD_SRC)i)(msgType);
    }
  }
  }
}



/* New Function Replacing Macro-code */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : aci_get_cb_cmd                    |
+-------------------------------------------------------------------+

  PURPOSE : check for valid srcId and return rAT function pointer.

*/

GLOBAL BOOL aci_get_cb_cmd (RAT_ID cmd, T_ACI_CMD_SRC src)
{
  T_ACI_CMD_MODE mode;

  TRACE_FUNCTION ("aci_get_cb_cmd()");
  
  if (src > CMD_SRC_LCL)
  {
    srcId_cb = (U8)src;
  }

  if (src NEQ CMD_SRC_NONE AND src<CMD_SRC_MAX AND IS_SRC_USED(src))
  {   
    //   Get the command mode for the specified source.
        mode = aci_cmd_src_mode_get(src);

        /*
        *   If RATJmpTbl is populated for the specified command mode
        *   and command, indicate this by returning the command mode.
        *   The nature of the R_AT macro is such that it is not possible
        *   to simply call the function in the table from here.
        */
      
    if (mode NEQ CMD_MODE_NONE AND RATJmpTbl[cmd][mode]) 
    {
      rat_fptr = RATJmpTbl[cmd][mode];
      return (TRUE);
    }
    TRACE_EVENT_P2("RATJmpTbl[cmd=%d][mode=%d]", cmd, mode);
  }

  /*
   *   A return value of FALSE is meant to indicate that the
  *   response should not be sent at all.
  */
  
  TRACE_EVENT_P1("aci_get_cb_cmd(): CMD_MODE_NONE for src %d", src);
  return(FALSE);
}


/*
+-------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_F                   |
|                                 ROUTINE : aci_cmd_src_mode_get    |
+-------------------------------------------------------------------+

  PURPOSE : Access function for reading aci_cmd_src_mode[] which
            stores the command mode of each command source.

*/
GLOBAL T_ACI_CMD_MODE aci_cmd_src_mode_get(
  T_ACI_CMD_SRC src_id)
{
  T_ACI_CMD_MODE mode;


  if (src_id<CMD_SRC_MAX)
  {
    mode = aci_cmd_src_mode[src_id];
    TRACE_EVENT_P2("aci_cmd_src_mode_get(): aci_cmd_src_mode[%d] = %d", src_id, mode);
    return(mode);
  }
  else
  {
    TRACE_EVENT("aci_cmd_src_mode_get(): CMD_MODE_NONE");
    return(CMD_MODE_NONE);
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_F                   |
|                                 ROUTINE : aci_cmd_src_mode_set    |
+-------------------------------------------------------------------+

  PURPOSE : Access function for writing to aci_cmd_src_mode[] which
            stores the command mode of each command source.

*/
GLOBAL void aci_cmd_src_mode_set(
  UBYTE src_id,
  T_ACI_CMD_MODE mode)
{
  if (src_id<CMD_SRC_MAX)
  {
    aci_cmd_src_mode[src_id]=mode;
    TRACE_EVENT_P2("aci_cmd_src_mode_set(): aci_cmd_src_mode[%d] = %d", src_id, mode);
  }
}

/* Implements Measure 178 */
/*
+------------------------------------------------------------------------------
|  Function    : cmh_ClearCall
+------------------------------------------------------------------------------
|  Purpose     : Clear a Call.
|
|  Parameters  : srcId - AT command source identifier.
|                cmd   - At Command Identifier 
|                idx   - Holds call table index.
|
|  Return      : UBYTE
+------------------------------------------------------------------------------
*/

LOCAL UBYTE cmh_ClearCall( T_ACI_CMD_SRC srcId, T_ACI_AT_CMD cmd, UBYTE idx )
{
  UBYTE srcBuf;
  
  TRACE_FUNCTION ("cmh_ClearCall()");

  srcBuf = psaCC_ctb(idx)->curSrc;

  cmhCC_tstAndUnflagCall( idx, &cmhPrm[srcBuf].ccCmdPrm.mltyCncFlg );
  cmhCC_flagCall        ( idx, &cmhPrm[srcId ].ccCmdPrm.mltyDscFlg );

  psaCC_ctb(idx)->curCmd = AT_CMD_ABRT;
  psaCC_ctb(idx)->curSrc = srcId;
  psaCC_ctb(idx)->nrmCs  = MNCC_CAUSE_CALL_CLEAR;

  psaCC_ClearCall (idx);

  if( srcBuf NEQ srcId )
  {
    R_AT( RAT_NO_CARRIER, (T_ACI_CMD_SRC)srcBuf ) ( cmd, idx+1 );
  }
  return srcBuf;
}

/* Implements Measure 34 */
/*
+------------------------------------------------------------------------------
|  Function    : cmh_tstAndUnflagCall
+------------------------------------------------------------------------------
|  Purpose     : Test and unflag a call for multy-call operation.
|
|  Parameters  : cmd    -  At Command Identifier 
|                srcBuf -  buffers current source of command
|
|  Return      : Return the Test Result.
+------------------------------------------------------------------------------
*/

LOCAL BOOL cmh_tstAndUnflagCall( T_ACI_AT_CMD cmd , UBYTE *srcBuf )
{
  UBYTE idx;                /* holds call table index */
  BOOL  doneFlg = FALSE;    /* flags that an action was done */
  UBYTE curSrc;

  TRACE_FUNCTION ("cmh_tstAndUnflagCall()");

  for( idx = 0; idx < MAX_CALL_NR; idx++ )
  {
    if (ccShrdPrm.ctb[idx] NEQ NULL AND 
        psaCC_ctb(idx)->curCmd EQ cmd AND
        psaCC_ctb(idx)->calStat EQ CS_MDF_REQ)
    {
      curSrc = psaCC_ctb(idx)->curSrc;

      cmhCC_tstAndUnflagCall( idx, &cmhPrm[curSrc].ccCmdPrm.mltyCncFlg );

      psaCC_ctb(idx)->curCmd = AT_CMD_NONE;
      psaCC_ctb(idx)->curSrc = CMD_SRC_NONE;
      doneFlg = TRUE;
      *srcBuf = curSrc;
    }
  }
  return doneFlg;
}


/* Implements Measure 25 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CMF_SMSF                   |
| STATE   : code                ROUTINE : cmhSMS_getAlphabetCb       |
+--------------------------------------------------------------------+
  PARAMETERS  : dcs - Data coding scheme
  RETURN      : Alphabet type

  PURPOSE : This function is used to extract the used alphabet out
            of the data coding scheme for cell broadcast SMS.
*/
GLOBAL UBYTE cmh_getAlphabetCb ( UBYTE dcs )
{
  UBYTE alphabet = 0;  /* means 7 bit default alphabet */

  switch (dcs & 0xF0)
  {
    case( 0x10 ):
      if (dcs EQ 0x11)
      {
        alphabet = 2;
      }
      break;

    case( 0x70 ):
    case( 0x60 ):
    case( 0x50 ):
    case( 0x40 ):
      alphabet = (dcs & 0x0C) >> 2;
      /* According to 03.38, "Any reserved codings shall be assumed 
       * to be the GSM 7 bit default alphabet by a receiving entity".
       */
      if (alphabet EQ 3)
      {
        alphabet = 0;
      }
      break;

    case( 0xF0 ):
      alphabet = (dcs & 0x04) >> 2;
      break;
  }

  return alphabet;
}
/*==== EOF ========================================================*/
