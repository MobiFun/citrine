/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_RA
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
|  Purpose :  Definitions for the protocol stack adapter
|             Rata Adaptation ( RA )
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_RA_H
#define PSA_RA_H

/*==== CONSTANTS ==================================================*/

#define MAX_TRANSP_DATA_SIZE 36

/*==== TYPES ======================================================*/

typedef struct
{
  UBYTE            model; 
  UBYTE            tra_rate;
  UBYTE            user_rate;
  UBYTE            ndb;
  UBYTE            nsb;
  UBYTE            bitord;
} T_RA_SET_PRM;

typedef struct RAShrdParm
{
  UBYTE     owner;
  SHORT     cId;                 /* related call id */ 
#ifdef DTI
  UBYTE     dti_id;              /* related dti id  */
#endif 
  T_RA_SET_PRM set_prm[OWN_SRC_MAX];
} T_RA_SHRD_PRM;


/*==== PROTOTYPES =================================================*/

EXTERN void psaRA_Init (void);
EXTERN void psaRA_Activate   ( void );
EXTERN void psaRA_Deactivate ( void );
EXTERN void psaRA_Modify     ( void );

#ifdef TRACING
EXTERN void psaRA_shrPrmDump ( void );
#endif

/*==== EXPORT =====================================================*/

#ifdef PSA_RAF_C

GLOBAL T_RA_SHRD_PRM raShrdPrm;

#else

EXTERN T_RA_SHRD_PRM raShrdPrm;

#endif /* PSA_RAF_C */


#endif /* PSA_RA_H */

/*==== EOF =======================================================*/

