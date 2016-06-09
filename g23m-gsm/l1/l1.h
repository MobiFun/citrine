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
|  Purpose :  Definitions for the Protocol Stack Entity l1.
+----------------------------------------------------------------------------- 
*/ 

#ifndef L1_H
#define L1_H

#ifdef TI_PS_HCOMM_CHANGE
#include "cl_hComm_handle.h"

#ifdef NEW_FRAME


  #define hCommGRR              l1_hCommGRR
  #define hCommLLC              l1_hCommLLC
  #define hCommSNDCP            l1_hCommSNDCP
  #define hCommGMM              l1_hCommGMM
  #define hCommTOM              l1_hCommTOM

  #define L1_Handle             l1_L1_Handle
   
/*jk: delete warnings  #define VSI_CALLER L1_Handle, */

  #ifdef L1_PEI_C

    /* Communication handles */

  #ifdef GPRS
    T_HANDLE                 hCommGRR = VSI_ERROR;
    T_HANDLE                 hCommLLC = VSI_ERROR;
    T_HANDLE                 hCommSNDCP = VSI_ERROR;
    T_HANDLE                 hCommGMM = VSI_ERROR;
    T_HANDLE                 hCommTOM = VSI_ERROR;
  #endif	
    T_HANDLE                 L1_Handle;

  #else /* L1_PEI_C */

    extern T_HANDLE          hCommGRR;
    extern T_HANDLE          hCommLLC;
    extern T_HANDLE          hCommSNDCP;
    extern T_HANDLE          hCommGMM;
    extern T_HANDLE          hCommTOM;
    extern T_HANDLE          L1_Handle;

  #endif /* L1_PEI_C */

#else /* NEW_FRAME */

  #define PL_NAME    "PL"
  #define L1_NAME    "L1"
 
  #define VSI_CALLER L1_NAME,

#endif /* NEW_FRAME */
#else
#ifdef NEW_FRAME

  #define hCommPL               l1_hCommPL
  #define hCommL1               l1_hCommL1
  #define hCommGRR              l1_hCommGRR
  #define hCommLLC              l1_hCommLLC
  #define hCommSNDCP            l1_hCommSNDCP
  #define hCommGMM              l1_hCommGMM
  #define hCommTOM              l1_hCommTOM
  #define hCommACI              l1_hCommACI
  #define L1_Handle             l1_L1_Handle
   
/*jk: delete warnings  #define VSI_CALLER L1_Handle, */

  #ifdef L1_PEI_C

    /* Communication handles */
    T_HANDLE                 hCommPL = VSI_ERROR;
    T_HANDLE                 hCommACI = VSI_ERROR;
    T_HANDLE                 hCommL1 = VSI_ERROR;
  #ifdef GPRS
    T_HANDLE                 hCommGRR = VSI_ERROR;
    T_HANDLE                 hCommLLC = VSI_ERROR;
    T_HANDLE                 hCommSNDCP = VSI_ERROR;
    T_HANDLE                 hCommGMM = VSI_ERROR;
    T_HANDLE                 hCommTOM = VSI_ERROR;
  #endif	
    T_HANDLE                 L1_Handle;

  #else /* L1_PEI_C */

    extern T_HANDLE          hCommPL;
    extern T_HANDLE          hCommACI;
    extern T_HANDLE          hCommL1;
    extern T_HANDLE          hCommGRR;
    extern T_HANDLE          hCommLLC;
    extern T_HANDLE          hCommSNDCP;
    extern T_HANDLE          hCommGMM;
    extern T_HANDLE          hCommTOM;
    extern T_HANDLE          L1_Handle;

  #endif /* L1_PEI_C */

#else /* NEW_FRAME */

  #define PL_NAME    "PL"
  #define L1_NAME    "L1"
 
  #define VSI_CALLER L1_NAME,

  #ifdef L1_PEI_C
  
    T_VSI_CHANDLE            hCommPL = VSI_ERROR;
    T_VSI_CHANDLE            hCommL1 = VSI_ERROR;

  #else /* L1_PEI_C */

    extern T_VSI_CHANDLE     hCommPL;
    extern T_VSI_CHANDLE     hCommL1;

  #endif /* L1_PEI_C */

#endif /* NEW_FRAME */
#endif /* TI_PS_HCOMM_CHANGE */
#endif /* L1_H */
