/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_DYN_DWL_CONST.H
 *
 *        Filename l1_dyn_dwl_const.h.h
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/
#if (L1_DYN_DSP_DWNLD == 1)

#ifndef _L1_DYN_DWL_CONST_H_
#define _L1_DYN_DWL_CONST_H_

/* DSP dynamic download background task id */
#define C_BGD_DSP_DYN_DWNLD   9

#define RED                                1
#define GREEN                              0
#define MAX_NUM_OF_PATCH_IDS               5
#define MAX_NUM_OF_SEMAPHORES              6

#define NUM_OF_DYN_DWNLD_PRIMITIVES 6

#define NUM_WORDS_COPY_API                 256     // even value mandatory
#define START_API_DWNLD_AREA               0x1808  // 0x1808
#define SIZE_API_DWNLD_AREA                0x7F8   // 0x800
#define START_API_DWNLD_AREA_DURING_E2     0x10C1  // 0x10BE
#define SIZE_API_DWNLD_AREA_DURING_E2      0x15B   // 0x410
#define MCU_API_BASE_ADDRESS               0xFFD00000L
#define DSP_API_BASE_ADDRESS               0x800


/* Dynamic Download API base address */
#define C_DYN_DWNLD_API_BASE_ADDRESS       0x17F6
#define HEADER_PATCH_SIZE                  4

#if(CODE_VERSION == SIMULATION)
  #define CRC_SIMU_OK           0xCAFE
  #define SIZE_DWNLD_AREA_SIMU  2048
#endif

#define TRUE  1
#define FALSE 0


/* Define commands MCU/DSP*/
#define C_DWL_DOWNLOAD_CTRL_DSP_ACK   0
#define C_DWL_DOWNLOAD_CTRL_DOWNLOAD  1
#define C_DWL_DOWNLOAD_CTRL_INSTALL   2
#define C_DWL_DOWNLOAD_CTRL_UNINSTALL 3
#define C_DWL_DOWNLOAD_CTRL_ABORT     4
#define C_DWL_DOWNLOAD_CTRL_INIT      5
#define C_DWL_ERR_RESET               0

#endif  // _L1_DYN_DWL_CONST_H_
#endif // L1_DYN_DSP_DWNLD
