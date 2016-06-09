/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci_ext\aci_ext_pers_cus.h
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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 

/*
   This is the initial setup for the customisation of MMI security. 
   Additional personalisation will be added at a later stage.
 */
T_ACI_PERS_MMI_DATAS MMI_personalisation_status =
{
   0x02,                // State disabled no pin required
   0x00,                // Count
   0x03,                // Max Count
   0x05,                // Pwd length
   0x21,0x43,0xF5,      // Current code BCD
   0x21,0x43,0xF5       // Original code BCD
};
   
#ifdef SIM_PERS

T_SEC_DRV_CATEGORY *personalisation_nw;

T_SEC_DRV_CATEGORY *personalisation_ns;

T_SEC_DRV_CATEGORY *personalisation_sp;

T_SEC_DRV_CATEGORY *personalisation_cp;

T_SEC_DRV_CATEGORY *personalisation_sim;

T_SEC_DRV_CATEGORY *personalisation_bnw;

T_SEC_DRV_CATEGORY *personalisation_first_sim;
#endif
