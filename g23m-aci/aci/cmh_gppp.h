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
|  Purpose :  Definitions for the command handler of the
|             Point-to-Point Protocol ( PPP ).
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef CMH_GPPP_H
#define CMH_GPPP_H

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/

/*==== PROTOTYPES =================================================*/
EXTERN SHORT cmhGPPP_Established   ( void );
EXTERN SHORT cmhGPPP_Terminated    ( void );
EXTERN SHORT cmhGPPP_Activated     ( void );
EXTERN SHORT cmhGPPP_Modified      ( void );


EXTERN void cmhGPPPS_DTIconnected( UBYTE dti_direction );

EXTERN void cmhPPPS_Disable( UBYTE dti_id );
EXTERN void cmhPPPS_DTIconnected( UBYTE dti_direction );
/*==== EXPORT =====================================================*/
#ifdef CMH_GPPPR_C

GLOBAL T_ENT_STAT gpppEntStat;

#else

EXTERN T_ENT_STAT gpppEntStat;

#endif /* CMH_GPPPR_C */

#endif /* CMH_GPPP_H */


#endif  /* GPRS */
/*==== EOF =======================================================*/
