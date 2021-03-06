/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  ccddata_priv.h 
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
|  Purpose :  Function that belongs to tap_tdl.c, but is are based
|             on the constants (CCDENT_...) generated by ccdgen
|             (mconst.cdg). Therefore it must be placed in the ccddata_dll.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CCDDATA_TAP_PRIV_H
#define CCDDATA_TAP_PRIV_H

/*==== INCLUDES =============================================================*/

/*==== CONSTS ===============================================================*/

#define TAP_PD_CC       3
#define TAP_PD_MM       5
#define TAP_PD_GMM      8
#define TAP_PD_SM       10
#define TAP_PD_RR       6
#define TAP_PD_SS       11
#define TAP_PD_SMS      9
#define TAP_PD_TST      15
#define TAP_PD_XX       1

#define TAP_PD_ABIS             0
#define TAP_PD_OK               128
#define TAP_PD_INVALID          -1
#define TAP_NOPD_NOMT           70
#define TAP_NOPD_MT             80
#define TAP_RR_SHORT            90

/*==== TYPES =================================================================*/

/*==== EXPORTS ===============================================================*/

#ifndef CCDDATA_TAP_PRIV_C

CCDDATA_IMPORT_FUNC int ccddata_tap_get_pd   (UCHAR comp);
CCDDATA_IMPORT_FUNC int ccddata_tap_check_pd (UCHAR comp, UCHAR pd);

#endif /* !CCDDATA_TAP_PRIV_C*/

#endif /* !CCDDATA_TAP_PRIV_H */
