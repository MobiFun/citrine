/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_MMI
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
|             man machine interface ( MMI )
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_MMI_H
#define PSA_MMI_H


/*==== CONSTANTS ==================================================*/

#define TEST_STR_LEN (80) /* max. length of test parameter string */

#define NO_ENTRY     (-1) /* not a valid entry */

/*==== TYPES ======================================================*/

typedef struct MMIShrdParm
{
  UBYTE  keyCd;                    /* key code */
  UBYTE  keySt;                    /* key status */
  UBYTE  rxLev;                    /* rx level */
  UBYTE  btLev;                    /* battery level */
  void*  dspRq;                    /* display request */
  UBYTE cmdSrc;                    /* source of current command */
} T_MMI_SHRD_PRM;

/*==== PROTOTYPES =================================================*/

SHORT psaMMI_Display     ( void );
SHORT psaMMI_Cbch        ( void );

void  psaMMI_Init        ( void );

EXTERN void psaMMI_ConfigPrim (T_HANDLE receiver_handle, 
                               CHAR     *config_msg);


/*==== EXPORT =====================================================*/

#ifdef PSA_MMIF_C

GLOBAL T_MMI_SHRD_PRM mmiShrdPrm;

#else

EXTERN T_MMI_SHRD_PRM mmiShrdPrm;

#endif /* PSA_MMIF_C */

 

#endif /* PSA_MMI_H */

/*==== EOF =======================================================*/
