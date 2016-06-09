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
|  Purpose :  GPRS Command handler interface definitions.
+----------------------------------------------------------------------------- 
*/ 

/* needs: aci_cmd.h */

#ifndef GACI_CMD_H
#define GACI_CMD_H

/* sub structure concerning AT+CGREG and
AT%CGREG command in ATI */

typedef struct
{
  T_ATI_REG_MOD_LAC_CID mod_lac_cid;
  T_CGREG_STAT          last_presented_state;
} T_ATI_CGREG;

typedef struct
{
  T_ATI_REG_MOD_LAC_CID mod_lac_cid;
  T_P_CGREG_STAT        last_presented_state;
} T_ATI_P_CGREG;

typedef struct
{
  T_ATI_CGREG    plus_cgreg;
  T_ATI_P_CGREG  percent_cgreg;

} T_ATI_GPRS_USER_OUTPUT_CFG;


EXTERN void gaci_ati_cmd_init_each_source( UBYTE srcId );


#ifdef ACI_CMD_C
/* next variable is aimed at containing the user defined configuration
for the output format through AT interpreter for GPRS */
GLOBAL T_ATI_GPRS_USER_OUTPUT_CFG ati_gprs_user_output_cfg[CMD_SRC_MAX];

#else /* ACI_CMD_C */

EXTERN T_ATI_GPRS_USER_OUTPUT_CFG ati_gprs_user_output_cfg[CMD_SRC_MAX];
#endif /* ACI_CMD_C */


#endif /* GACI_CMD_H */
/*==== EOF ========================================================*/

