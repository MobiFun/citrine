/****************************************************************************/
/*                                                                          */
/*  Name        rvf_pool_size.h                                             */
/*                                                                          */
/*  Function    this file contains definitions for buffer pool sizes        */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Version     0.3                                                         */
/*                                                                          */
/*  Date        Modification                                                */
/*  ------------------------------------                                    */
/*  4/19/1999   Create                                                      */
/*  10/27/1999  Remove declaration of ntohs, htons, ntohl, htonl            */
/*              in order to avoid conflict with winsock.h                   */
/*  12/23/1999  Add buffer and memory bank related definition               */
/*  07/12/2000  Implement dynamic memory allocation.                        */
/*  01/13/2002  Extracted buffer pool size info from rvf_target.h           */
/*              to prevent useless full recompilation when those datas      */
/*              are changed                                                 */
/*  10/01/2002  Completely reworked in order to be modular                  */
/*  07/08/2003  Rework                                                      */
/*                                                                          */
/*  Author      David Lamy-Charrier (dlamy@tif.ti.com)                      */
/*                                                                          */
/* (C) Copyright 1999 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#ifndef _RVF_POOL_SIZE_H
#define _RVF_POOL_SIZE_H

#if 0 //#ifndef _WINDOWS
  #include "config/r2d.cfg"
  #include "config/rv.cfg"
  #include "config/debug.cfg"
  #include "config/board.cfg"
  #include "config/l1sw.cfg"
  #include "config/swconfig.cfg"
#endif

#include "../rv/rv_defined_swe.h"
//#include "config/mdc.cfg"


/* 
 * RPC SWE 
 */
#ifdef RVM_RPC_SWE
  #include "widcomm/rpc/rpc_pool_size.h"
  #define RVF_RPC_POOL_SIZE  (RPC_POOL_SIZE)
#else
  #define RVF_RPC_POOL_SIZE  (0)
#endif

/* 
 * BTA SWE 
 */
#ifdef RVM_BTA_SWE
  #include "widcomm/bta/bta_pool_size.h"
  #define RVF_BTA_POOL_SIZE  (BTA_POOL_SIZE)
#else
  #define RVF_BTA_POOL_SIZE  (0)
#endif

/* 
 * BTU SWE 
 */
#ifdef RVM_BTU_SWE
  #include "widcomm/btu/btu_pool_size.h"
  #define RVF_BTU_POOL_SIZE  (BTU_POOL_SIZE)
#else
  #define RVF_BTU_POOL_SIZE  (0)
#endif

/* 
 * BTUI SWE
 */
#ifdef RVM_BTUI_SWE
  #include "widcomm/btui/btui_pool_size.h"
  #define RVF_BTUI_POOL_SIZE  (BTUI_POOL_SIZE)
#else
  #define RVF_BTUI_POOL_SIZE  (0)
#endif

/* 
 * BTH SWE  
 */
#ifdef RVM_BTH_SWE
  #include "widcomm/hci/pc/bth/bth_pool_size.h"
  #define RVF_BTH_POOL_SIZE  (BTU_POOL_SIZE)
#else
  #define RVF_BTH_POOL_SIZE  (0)
#endif

/* 
 * GKI SWE 
 */
#ifdef RVM_GKI_SWE
  #include "widcomm/gki/gki_pool_size.h"
  #define RVF_GKI_POOL_SIZE  (GKI_POOL_SIZE)
#else
  #define RVF_GKI_POOL_SIZE  (0)
#endif

/*
 * HCI SWE
 */
#ifdef RVM_HCI_SWE
  #include "widcomm/hci/hci_pool_size.h"
  #define RVF_HCI_POOL_SIZE  (HCI_POOL_SIZE)
#else
  #define RVF_HCI_POOL_SIZE  (0)
#endif



/*
 * ATP SWE
 */
#ifdef RVM_ATP_SWE
  #include "atp/atp_pool_size.h"
  #define RVF_ATP_POOL_SIZE (ATP_POOL_SIZE)
#else
  #define RVF_ATP_POOL_SIZE (0)
#endif

/*
 * ATP_UART SWE
 */
#ifdef RVM_ATP_UART_SWE
  #include "atp/atp_pool_size.h"
  #define RVF_ATP_UART_POOL_SIZE  (ATP_UART_POOL_SIZE)
#else
  #define RVF_ATP_UART_POOL_SIZE  (0)
#endif

/*
 * AUDIO SWE and background
 */
#ifdef RVM_AUDIO_MAIN_SWE
  #include "audio/audio_pool_size.h"
  #define RVF_AUDIO_POOL_SIZE  (AUDIO_POOL_SIZE)
#else
  #define RVF_AUDIO_POOL_SIZE  (0)
#endif

#ifdef RVM_AUDIO_BGD_SWE
  #include "audio/audio_pool_size.h"
  #define RVF_AUDIO_BGD_POOL_SIZE  (AUDIO_BGD_POOL_SIZE)
#else
  #define RVF_AUDIO_BGD_POOL_SIZE  (0)
#endif

/*
 * DAR SWE
 */
#ifdef RVM_DAR_SWE
  #include "dar/dar_pool_size.h"
  #define RVF_DAR_POOL_SIZE  (DAR_POOL_SIZE)
#else
  #define RVF_DAR_POOL_SIZE  (0)
#endif

/*
 * ETM SWE
 */
#ifdef RVM_ETM_SWE
  #include "../../services/etm/etm_pool_size.h"
  #define RVF_ETM_POOL_SIZE  (ETM_POOL_SIZE)
#else
  #define RVF_ETM_POOL_SIZE  (0)
#endif

/*
 * FFS SWE
 */
#ifdef RVM_FFS_SWE
  #include "../../services/ffs/ffs_pool_size.h"
  #define RVF_FFS_POOL_SIZE  (FFS_POOL_SIZE)
#else
  #define RVF_FFS_POOL_SIZE  (0)
#endif


/*
 * KCL SWE
 */
#ifdef RVM_KCL_SWE
  #include "j2me_k/kcl/kcl_pool_size.h"
  #define RVF_KCL_POOL_SIZE (KCL_POOL_SIZE)
#else
  #define RVF_KCL_POOL_SIZE (0)
#endif

/*
 * KGC SWE
 */
#ifdef RVM_KGC_SWE
  #include "j2me_k/kgc/kgc_pool_size.h"
  #define RVF_KGC_POOL_SIZE (KGC_POOL_SIZE)
#else
  #define RVF_KGC_POOL_SIZE (0)
#endif

/*
 * KIL SWE
 */
#ifdef RVM_KIL_SWE
  #include "j2me_k/kil/kil_pool_size.h"
  #define RVF_KIL_POOL_SIZE (KIL_POOL_SIZE)
#else
  #define RVF_KIL_POOL_SIZE (0)
#endif

/*
 * KMM SWE
 */
#ifdef RVM_KMM_SWE
  #include "j2me_k/kmm/kmm_pool_size.h"
  #define RVF_KMM_POOL_SIZE (KMM_POOL_SIZE)
#else
  #define RVF_KMM_POOL_SIZE (0)
#endif

/*
 * KNM SWE
 */
#ifdef RVM_KNM_SWE
  #include "j2me_k/knm/knm_pool_size.h"
  #define RVF_KNM_POOL_SIZE (KNM_POOL_SIZE)
#else
  #define RVF_KNM_POOL_SIZE (0)
#endif

/*
 * KPG SWE
 */
#ifdef RVM_KPG_SWE
  #include "j2me_k/kpg/kpg_pool_size.h"
  #define RVF_KPG_POOL_SIZE (KPG_POOL_SIZE)
#else
  #define RVF_KPG_POOL_SIZE (0)
#endif

/*
 * KZP SWE
 */
#ifdef RVM_KZP_SWE
  #include "j2me_k/kzp/kzp_pool_size.h"
  #define RVF_KZP_POOL_SIZE (KZP_POOL_SIZE)
#else
  #define RVF_KZP_POOL_SIZE (0)
#endif


/*
 * KPD SWE
 */
#ifdef RVM_KPD_SWE
  #include "kpd/kpd_pool_size.h"
  #define RVF_KPD_POOL_SIZE  (KPD_POOL_SIZE)
#else
  #define RVF_KPD_POOL_SIZE  (0)
#endif

   
/*
 * LLS SWE
 */
#ifdef RVM_LLS_SWE
  #include "lls/lls_pool_size.h"
  #define RVF_LLS_POOL_SIZE (LLS_POOL_SIZE)
#else
  #define RVF_LLS_POOL_SIZE (0)
#endif

/*
 * MKS SWE
 */
#ifdef RVM_MKS_SWE
  #include "mks/mks_pool_size.h"
  #define RVF_MKS_POOL_SIZE  (MKS_POOL_SIZE)
#else
  #define RVF_MKS_POOL_SIZE  (0)
#endif

/*
 * MPM SWE
 */
#ifdef RVM_MPM_SWE
  #include "mpm/mpm_pool_size.h"
  #define RVF_MPM_POOL_SIZE  (MPM_POOL_SIZE)
#else
  #define RVF_MPM_POOL_SIZE  (0)
#endif

/*
 * PWR SWE
 */
#ifdef RVM_PWR_SWE
  #include "pwr/pwr_pool_size.h"
  #define RVF_PWR_POOL_SIZE (PWR_POOL_SIZE)
#else
  #define RVF_PWR_POOL_SIZE (0)
#endif

/*
 * LCC (PWR) SWE
 */

#ifdef RVM_LCC_SWE
  #include "lcc/lcc_pool_size.h"
  #define RVF_LCC_POOL_SIZE (LCC_POOL_SIZE)
#else
  #define RVF_LCC_POOL_SIZE (0)
#endif

/*
 * R2D SWE
 */
#ifdef RVM_R2D_SWE
  #include "r2d/r2d_pool_size.h"
  #define RVF_R2D_POOL_SIZE    (R2D_POOL_SIZE)
#else
  #define RVF_R2D_POOL_SIZE    (0)
#endif

/*
 * RGUI SWE
 */
#ifdef RVM_RGUI_SWE
  #include "rgui/rgui_pool_size.h"
  #define RVF_RGUI_POOL_SIZE  (RGUI_POOL_SIZE)
#else
  #define RVF_RGUI_POOL_SIZE  (0)
#endif


/*
 * RNET SWE
 */
#ifdef RVM_RNET_SWE
  #include "rnet/rnet_pool_size.h"
  #define RVF_RNET_POOL_SIZE (RNET_POOL_SIZE)
#else
  #define RVF_RNET_POOL_SIZE (0)
#endif

/*
 * RNET_BR SWE
 */
#ifdef RVM_RNET_BR_SWE
  #include "rnet/rnet_br/rnet_br_pool_size.h"
  #define RVF_RNET_BR_POOL_SIZE (RNET_BR_POOL_SIZE)
#else
  #define RVF_RNET_BR_POOL_SIZE (0)
#endif

/*
 * RNET_RT SWE
 */
#ifdef RVM_RNET_RT_SWE
  #include "rnet/rnet_rt/rnet_rt_pool_size.h"
  #define RVF_RNET_RT_POOL_SIZE  (RNET_RT_POOL_SIZE)
#else
  #define RVF_RNET_RT_POOL_SIZE  (0)
#endif

/*
 * RNET_WS SWE
 */
#ifdef RVM_RNET_WS_SWE
  #include "rnet/rnet_ws/rnet_ws_pool_size.h"
  #define RVF_RNET_WS_POOL_SIZE  (RNET_WS_POOL_SIZE)
#else
  #define RVF_RNET_WS_POOL_SIZE  (0)
#endif


/*
 * DCM SWE
 */
#ifdef RVM_DCM_SWE
  #include "dcm/dcm_pool_size.h"
  #define RVF_DCM_POOL_SIZE  (DCM_POOL_SIZE)
#else
  #define RVF_DCM_POOL_SIZE  (0)
#endif

/*
 * DCFG SWE
 */

#ifdef RVM_DCFG_SWE
  #include "dcfg/dcfg_pool_size.h"
  #define RVF_DCFG_POOL_SIZE  (DCFG_POOL_SIZE)
#else
  #define RVF_DCFG_POOL_SIZE  (0)
#endif

/*
 * RTC SWE
 */
#ifdef RVM_RTC_SWE
  #include "../../bsp/rtc/rtc_pool_size.h"
  #define RVF_RTC_POOL_SIZE  (RTC_POOL_SIZE)
#else
  #define RVF_RTC_POOL_SIZE  (0)
#endif

/*
 * RTEST SWE
 */
#ifdef RVM_RTEST_SWE
  #include "tests/rtest/rtest_pool_size.h"
  #define RVF_RTEST_POOL_SIZE  (RTEST_POOL_SIZE)
#else
  #define RVF_RTEST_POOL_SIZE  (0)
#endif

/*
 * RVM SWE; This SWE is of course always enabled.
 * TO CLEAN??
 */
#include "../rvm/rvm_pool_size.h"


/*
 * SPI SWE
 */
#ifdef RVM_SPI_SWE
  #include "../../bsp/abb+spi/spi_pool_size.h"
  #define RVF_SPI_POOL_SIZE  (SPI_POOL_SIZE)
#else
  #define RVF_SPI_POOL_SIZE  (0)
#endif



/*
 * TI_PRF SWE
 */
#ifdef RVM_TI_PRF_SWE
  #include "rvt/ti_profiler/ti_prf_pool_size.h"
  #define RVF_TI_PRF_POOL_SIZE  (TI_PRF_POOL_SIZE)
#else
  #define RVF_TI_PRF_POOL_SIZE  (0)
#endif

/*
 * TRACE SWE
 */
#ifdef RVM_RVT_SWE
  #include "../rvt/rvt_pool_size.h"
  #define RVF_TRACE_POOL_SIZE  (TRACE_POOL_SIZE)
#else
  #define RVF_TRACE_POOL_SIZE  (0)
#endif

/*
 * TUT SWE
 */
#ifdef RVM_TUT_SWE
  #include "tut/tut_pool_size.h"
  #define RVF_TUT_POOL_SIZE  (TUT_POOL_SIZE)
#else
  #define RVF_TUT_POOL_SIZE  (0)
#endif

/*
 * UVM SWE
 */
#ifdef RVM_UVM_SWE
  #include "j2me_k/uvm/uvm_pool_size.h"
  #define RVF_UVM_POOL_SIZE  (UVM_POOL_SIZE)
#else
  #define RVF_UVM_POOL_SIZE  (0)
#endif

/*
 * JTM SWE
 */
#ifdef RVM_JTM_SWE
  #include "j2me_k/jtm/jtm_pool_size.h"
  #define RVF_JTM_POOL_SIZE  (JTM_POOL_SIZE)
#else
  #define RVF_JTM_POOL_SIZE  (0)
#endif

/*
 * MDC SWE
 */
#ifdef RVM_MDC_SWE
  #include "mdc/mdc_pool_size.h"
  #define RVF_MDC_POOL_SIZE  (MDC_POOL_SIZE)
#else
  #define RVF_MDC_POOL_SIZE  (0)
#endif

/*
 * TTY SWE
 */
#ifdef RVM_TTY_SWE
  #include "tty/tty_pool_size.h"
  #define RVF_TTY_POOL_SIZE  (TTY_POOL_SIZE)
#else
  #define RVF_TTY_POOL_SIZE  (0)
#endif


/*
 * Obigo SWEs
 */
/* BRAE SWE */
#ifdef RVM_BRAE_SWE
  #include "obigo/brae/brae_pool_size.h"
  #define RVF_BRAE_POOL_SIZE  BRAE_POOL_SIZE
#else
  #define RVF_BRAE_POOL_SIZE  (0)
#endif
/* BRSE SWE */
#ifdef RVM_BRSE_SWE
  #include "obigo/brse/brse_pool_size.h"
  #define RVF_BRSE_POOL_SIZE  BRSE_POOL_SIZE
#else
  #define RVF_BRSE_POOL_SIZE  (0)
#endif

/* UISE SWE */
#ifdef RVM_UISE_SWE
  #include "obigo/uise/uise_pool_size.h"
  #define RVF_UISE_POOL_SIZE  UISE_POOL_SIZE
#else
  #define RVF_UISE_POOL_SIZE  (0)
#endif

/* UIAE SWE */
#ifdef RVM_UIAE_SWE
  #include "obigo/uiae/uiae_pool_size.h"
  #define RVF_UIAE_POOL_SIZE  UIAE_POOL_SIZE
#else
  #define RVF_UIAE_POOL_SIZE  (0)
#endif

/* MEAE SWE */
#ifdef RVM_MEAE_SWE
  #include "obigo/meae/meae_pool_size.h"
  #define RVF_MEAE_POOL_SIZE  MEAE_POOL_SIZE
#else
  #define RVF_MEAE_POOL_SIZE  (0)
#endif
/* MMSE SWE */
#ifdef RVM_MMSE_SWE
  #include "obigo/mmse/mmse_pool_size.h"
  #define RVF_MMSE_POOL_SIZE  MMSE_POOL_SIZE
#else
  #define RVF_MMSE_POOL_SIZE  (0)
#endif
/* MSFE SWE */
#ifdef RVM_MSFE_SWE
  #include "obigo/msfe/msfe_pool_size.h"
  #define RVF_MSFE_POOL_SIZE  MSFE_POOL_SIZE
#else
  #define RVF_MSFE_POOL_SIZE  (0)
#endif
/* MSME SWE */
#ifdef RVM_MSME_SWE
  #include "obigo/msme/msme_pool_size.h"
  #define RVF_MSME_POOL_SIZE  MSME_POOL_SIZE
#else
  #define RVF_MSME_POOL_SIZE  (0)
#endif
/* PHSE SWE */
#ifdef RVM_PHSE_SWE
  #include "obigo/phse/phse_pool_size.h"
  #define RVF_PHSE_POOL_SIZE  PHSE_POOL_SIZE
#else
  #define RVF_PHSE_POOL_SIZE  (0)
#endif
/* PRSE SWE */
#ifdef RVM_PRSE_SWE
  #include "obigo/prse/prse_pool_size.h"
  #define RVF_PRSE_POOL_SIZE  PRSE_POOL_SIZE
#else
  #define RVF_PRSE_POOL_SIZE  (0)
#endif
/* SECE SWE */
#ifdef RVM_SECE_SWE
  #include "obigo/sece/sece_pool_size.h"
  #define RVF_SECE_POOL_SIZE  SECE_POOL_SIZE
#else
  #define RVF_SECE_POOL_SIZE  (0)
#endif
/* SELE SWE */
#ifdef RVM_SELE_SWE
  #include "obigo/sele/sele_pool_size.h"
  #define RVF_SELE_POOL_SIZE  SELE_POOL_SIZE
#else
  #define RVF_SELE_POOL_SIZE  (0)
#endif
/* SLSE SWE */
#ifdef RVM_SLSE_SWE
  #include "obigo/slse/slse_pool_size.h"
  #define RVF_SLSE_POOL_SIZE  SLSE_POOL_SIZE
#else
  #define RVF_SLSE_POOL_SIZE  (0)
#endif
/* SMAE SWE */
#ifdef RVM_SMAE_SWE
  #include "obigo/smae/smae_pool_size.h"
  #define RVF_SMAE_POOL_SIZE  SMAE_POOL_SIZE
#else
  #define RVF_SMAE_POOL_SIZE  (0)
#endif
/* STKE SWE */
#ifdef RVM_STKE_SWE
  #include "obigo/stke/stke_pool_size.h"
  #define RVF_STKE_POOL_SIZE  STKE_POOL_SIZE
#else
  #define RVF_STKE_POOL_SIZE  (0)
#endif
/* EMAE SWE */
#ifdef RVM_EMAE_SWE
  #include "obigo/emae/emae_pool_size.h"
  #define RVF_EMAE_POOL_SIZE  EMAE_POOL_SIZE
#else
  #define RVF_EMAE_POOL_SIZE  (0)
#endif
/* EMSE SWE */
#ifdef RVM_EMSE_SWE
  #include "obigo/emse/emse_pool_size.h"
  #define RVF_EMSE_POOL_SIZE  EMSE_POOL_SIZE
#else
  #define RVF_EMSE_POOL_SIZE  (0)
#endif

/* IT0E SWE */
#ifdef RVM_IT0E_SWE
  #include "obigo/tests/it0e/it0e_pool_size.h"
  #define RVF_IT0E_POOL_SIZE  IT0E_POOL_SIZE
#else
  #define RVF_IT0E_POOL_SIZE  (0)
#endif
/* IT1E SWE */
#ifdef RVM_IT1E_SWE
  #include "obigo/tests/it1e/it1e_pool_size.h"
  #define RVF_IT1E_POOL_SIZE  IT1E_POOL_SIZE
#else
  #define RVF_IT1E_POOL_SIZE  (0)
#endif
/* IT2E SWE */
#ifdef RVM_IT2E_SWE
  #include "obigo/tests/it2e/it2e_pool_size.h"
  #define RVF_IT2E_POOL_SIZE  IT2E_POOL_SIZE
#else
  #define RVF_IT2E_POOL_SIZE  (0)
#endif


/* 
 * MFW SWE
 */
#ifdef RVM_MFW_SWE
  #include "mfw/mfw_pool_size.h"
  #define RVF_MFW_POOL_SIZE  (MFW_POOL_SIZE)
#else
  #define RVF_MFW_POOL_SIZE  (0)
#endif

/*
 * IMG SWE
 */
#ifdef RVM_IMG_SWE
  #include "img/img_pool_size.h"
  #define RVF_IMG_POOL_SIZE  IMG_POOL_SIZE
#else
  #define RVF_IMG_POOL_SIZE  (0)
#endif

/*
MMS
//TISHMMS Project add 10K for buffer mms message 
*/
#ifdef RVM_MMS_SWE
  /*
   * Values used in mms_env.h
   */
   #include "mms/mms_pool_size.h"

  #define RVF_MMS_POOL_SIZE  MMS_POOL_SIZE
#else
  #define RVF_MMS_POOL_SIZE  (0)
#endif

/* 
 * SMBS SWE
 */
#ifdef RVM_SMBS_SWE
  #include "smbs/smbs_pool_size.h"
  #define RVF_SMBS_POOL_SIZE  (SMBS_POOL_SIZE)
#else
  #define RVF_SMBS_POOL_SIZE  (0)
#endif

/* 
 * MDL SWE
 */
#ifdef RVM_MDL_SWE
  #include "mdl/mdl_pool_size.h"
  #define RVF_MDL_POOL_SIZE  (MDL_POOL_SIZE)
#else
  #define RVF_MDL_POOL_SIZE  (0)
#endif



/* define the memory pools used by the RVF */

/* define RVF_NB_POOLS to 1 or 2 if you want to use 2 buffers in RAM,
for example 1 in internal RAM, 1 in external RAM */
#define RVF_NB_POOLS            1
/* 500 corresponds to a "hard-coded" value for RVM_STACKS Memory Bank defined in rvm_i.h */
#define RVF_POOL_0_SIZE (RVF_ATP_POOL_SIZE     + RVF_ATP_UART_POOL_SIZE + RVF_AUDIO_POOL_SIZE     + \
                         RVF_DAR_POOL_SIZE     + RVF_HCI_POOL_SIZE      + RVF_ETM_POOL_SIZE       + \
                         RVF_FFS_POOL_SIZE     + RVF_KCL_POOL_SIZE      + RVF_KGC_POOL_SIZE       + \
                         RVF_KIL_POOL_SIZE     + RVF_KMM_POOL_SIZE      + RVF_KNM_POOL_SIZE       + \
                         RVF_KPD_POOL_SIZE     + RVF_KPG_POOL_SIZE      + RVF_KZP_POOL_SIZE       + \
                         RVF_LLS_POOL_SIZE     + RVF_MKS_POOL_SIZE      + RVF_TTY_POOL_SIZE       + \
                         RVF_MPM_POOL_SIZE     + RVF_PWR_POOL_SIZE      + RVF_LCC_POOL_SIZE       + \
                         RVF_R2D_POOL_SIZE + RVF_MMS_POOL_SIZE +\
                         RVF_RGUI_POOL_SIZE    + RVF_RNET_POOL_SIZE     + RVF_RNET_BR_POOL_SIZE   + \
                         RVF_MDC_POOL_SIZE     + RVF_RNET_RT_POOL_SIZE  + RVF_RNET_WS_POOL_SIZE   + \
                         RVF_RTC_POOL_SIZE     + RVF_RTEST_POOL_SIZE    + RVF_RVM_POOL_SIZE       + \
                         RVF_SPI_POOL_SIZE     + RVF_TI_PRF_POOL_SIZE   + RVF_TRACE_POOL_SIZE     + \
                         RVF_TUT_POOL_SIZE     + RVF_UVM_POOL_SIZE      + RVF_JTM_POOL_SIZE       + \
                         RVF_DCM_POOL_SIZE     + RVF_BRAE_POOL_SIZE     + RVF_BRSE_POOL_SIZE      + \
                         RVF_UISE_POOL_SIZE    + RVF_UIAE_POOL_SIZE     + RVF_MEAE_POOL_SIZE      + \
                         RVF_MMSE_POOL_SIZE                                                       + \
                         RVF_MSFE_POOL_SIZE    + RVF_MSME_POOL_SIZE     + RVF_PHSE_POOL_SIZE      + \
                         RVF_PRSE_POOL_SIZE    + RVF_SECE_POOL_SIZE     + RVF_SELE_POOL_SIZE      + \
                         RVF_SLSE_POOL_SIZE    + RVF_SMAE_POOL_SIZE     + RVF_STKE_POOL_SIZE      + \
                         RVF_IT0E_POOL_SIZE    + RVF_EMAE_POOL_SIZE     + RVF_EMSE_POOL_SIZE      + \
                         RVF_IT1E_POOL_SIZE    + RVF_IT2E_POOL_SIZE     + RVF_IMG_POOL_SIZE       + \
                         RVF_SMBS_POOL_SIZE    + RVF_MFW_POOL_SIZE      + RVF_AUDIO_BGD_POOL_SIZE + \
                         RVF_BTA_POOL_SIZE     + RVF_BTU_POOL_SIZE      + RVF_GKI_POOL_SIZE       + \
                         RVF_BTUI_POOL_SIZE    + RVF_BTH_POOL_SIZE      + RVF_RPC_POOL_SIZE       + \
                         RVF_DCFG_POOL_SIZE    + RVF_MDL_POOL_SIZE)

#if (RVF_NB_POOLS > 1 )
    #define RVF_POOL_1_SIZE     0
#endif

/* define the memory usage ratio */
/* e.g. the ratio between the sum of the memory banks sizes and the memory available in the system */
/* for example: 90 for 90 % to be sure to always satisfy memory requests*/
/*              120 for 120 % to optimize memory usage */
#define RVF_MEM_USAGE_RATIO     100

#endif /* _RVF_POOL_SIZE_H */

