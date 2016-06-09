/* 
+----------------------------------------------------------------------------- 
|  Project :  WAP
|  Modul   :  CMH_PPP
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
|  Purpose :  Definitions for command handler PPP.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_PPP_H
#define CMH_PPP_H

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== PROTOTYPES =================================================*/

EXTERN T_ACI_RETURN cmhPPP_Established(ULONG ip, USHORT mru,
                                       ULONG dns1, ULONG dns2) ;
EXTERN T_ACI_RETURN cmhPPP_Terminated ( void );
EXTERN T_ACI_RETURN cmhPPP_Terminate (  T_ACI_PPP_LOWER_LAYER ppp_lower_layer );

/*==== EXPORT =====================================================*/

#ifdef CMH_PPPF_C

GLOBAL T_ENT_STAT pppEntStat;

#else

EXTERN T_ENT_STAT pppEntStat;

#endif /* CMH_PPPF_C */


#endif /* CMH_PPP_H */

/*==== EOF =======================================================*/

