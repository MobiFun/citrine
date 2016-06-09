/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_DYN_DWL_MSGTY.H
 *
 *        Filename l1_dyn_dwl_msgty.h
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/
#if (L1_DYN_DSP_DWNLD == 1)

typedef struct 
{
  UWORD16 patch_id;
}T_API_L1_CRC_NOT_OK;
typedef struct 
{
  UWORD16 error;
}T_API_L1_DYN_DWNLD_STOP;

#endif // L1_DYN_DSP_DWNLD == 1
