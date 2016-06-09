/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  fflags.h
+----------------------------------------------------------------------------- 
|  Copyright 2003 Texas Instruments Berlin, AG 
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
|  Purpose :  Define valid and invalid versions for a product release
|             include file for CCDGEN, e.g.:
|      ccdgen -h -m512 -a2 -$(ff_path)/fflags.h -o$(cdgincdir) -R$(cmdFile)
+----------------------------------------------------------------------------- 
*/
#define FF_PS_RSSI
#define SIM_PERS
#define FF_PHONE_LOCK
#define TI_PS_FF_AT_CMD_P_ECC
#define TI_PS_FF_REL99_AND_ABOVE
#undef  TI_DUAL_MODE
#define REL99
#define FF_BHO
#define TI_PS_FF_EMR
#define TI_PS_FF_RTD
#undef  FF_EGPRS
#undef  REL4
#define TI_PS_FF_TBF_EST_PACCH
#define TI_PS_FF_QUAD_BAND_SUPPORT

