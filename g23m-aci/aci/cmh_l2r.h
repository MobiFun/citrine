/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_L2R
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
|  Purpose :  Definitions for the command handler of Layer 2 Relay
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_L2R_H
#define CMH_L2R_H

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/

typedef struct entL2rStatus          /* entity status */
{
  T_ACI_CMD_SRC  entOwn;          /* entity owner */
  T_ACI_AT_CMD   curCmd;          /* current command processing */
  BOOL           isTempDisconnected;            
} T_L2R_ENT_STAT;

/*==== PROTOTYPES =================================================*/

GLOBAL void cmhL2R_TRA_Enabled ( void );
GLOBAL void cmhL2R_Deactivated ( void );
GLOBAL void cmhTRA_Deactivated ( void );

#ifdef DTI
GLOBAL void cmhL2R_TRA_Disabled ( T_DTI_ENTITY_ID entityId);
#endif

GLOBAL void cmhL2R_Failure ( void );

GLOBAL SHORT        cmhL2R_GetCompDir ( void );
GLOBAL UBYTE        cmhL2R_SelCompDir ( T_L2R_CMD_PRM * pCmdPrm );
GLOBAL UBYTE        cmhL2R_SelChnRate ( void );
GLOBAL T_ACI_RETURN cmhL2R_Activate   ( T_ACI_CMD_SRC srcId,
                                        T_ACI_AT_CMD cmdId,
                                        SHORT cId );
GLOBAL T_ACI_RETURN cmhL2R_Deactivate ( void );
GLOBAL T_ACI_RETURN cmhTRA_Deactivate ( void );

#ifdef DTI
GLOBAL void cmhCC_L2R_or_TRA_Activated ( T_DTI_ENTITY_ID activated_module, SHORT cId );
#endif /* DTI */
GLOBAL void cmhCC_L2R_or_TRA_Deactivated ( SHORT cId );

#ifdef DTI
GLOBAL BOOL TRA_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type); 
GLOBAL BOOL L2R_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type); 
#endif /* DTI */


GLOBAL T_ACI_RETURN cmhL2R_Enable     ( void );


/*==== EXPORT =====================================================*/
#ifdef CMH_L2RF_C

GLOBAL T_L2R_ENT_STAT l2rEntStat;

#else

EXTERN T_L2R_ENT_STAT l2rEntStat;

#endif /* CMH_L2RF_C */


#endif /* CMH_L2R_H */

/*==== EOF =======================================================*/

