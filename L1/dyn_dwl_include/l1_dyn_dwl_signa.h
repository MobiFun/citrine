/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DYN_DWL_SIGNA.H
 *
 *        Filename l1_dyn_dwl_signa.h
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/
#if (L1_DYN_DSP_DWNLD == 1)

#ifndef _L1_DYN_DWL_SIGNA_H_
#define _L1_DYN_DWL_SIGNA_H_

#define P_DYN_DWNLD 0x41

// Messages L1S -> L1A
#define L1_DYN_DWNLD_STOP_CON              ( ( P_DYN_DWNLD << 8 ) | 0x02 )

// Messages API HISR -> L1A  //
#define API_L1_DYN_DWNLD_START_CON         ( ( P_DYN_DWNLD << 8 ) | 0x03 )
#define API_L1_DYN_DWNLD_FINISHED          ( ( P_DYN_DWNLD << 8 ) | 0x04 )
#define API_L1_DYN_DWNLD_STOP              ( ( P_DYN_DWNLD << 8 ) | 0x05 )
#define API_L1_CRC_NOT_OK                  ( ( P_DYN_DWNLD << 8 ) | 0x07 )
#define API_L1_CRC_OK                      ( ( P_DYN_DWNLD << 8 ) | 0x08 )
#define API_L1_DYN_DWNLD_UNINST_OK         ( ( P_DYN_DWNLD << 8 ) | 0x09 )

#endif  //_L1_DYN_DWL_SIGNA_H_

#endif  // L1_DYN_DSP_DWNLD

