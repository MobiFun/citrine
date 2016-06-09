/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DYN_DWL_ERROR.H
 *
 *        Filename l1_dyn_dwl_error.h
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/
#if (L1_DYN_DSP_DWNLD == 1)

#ifndef _L1_DYN_DWL_ERROR_H_
#define _L1_DYN_DWL_ERROR_H_

#define C_DYN_DWL_ERR_NONE           (0x0000)   /* no error */
#define C_DYN_DWL_ERR_WR_PTR         (0x0001)   /* write pointer is not correct */
#define C_DYN_DWL_ERR_SIZE           (0x0002)   /* size is not correct          */
#define C_DYN_DWL_ERR_END_SECTION    (0x0004)   /* end word is not detected     */
#define C_DYN_DWL_ERR_UN_INSTALL     (0x0008)   /* installation or un-installation fail */
#define C_DYN_DWL_ERR_NOT_ENDED      (0x0010)   /* new request received but download not finished */ 

#endif    // _L1_DYN_DWL_ERROR_H_
#endif    // L1_DYN_DSP_DWNLD

