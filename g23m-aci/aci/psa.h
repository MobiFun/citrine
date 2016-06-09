/*  
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA
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
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_H
#define PSA_H

/*==== CONSTANTS ==================================================*/

#define OWN_ATI   1
typedef enum                      /* owner id's */
{
  OWN_NONE = -1,                      /* make gcc treat this enum as signed */
  OWN_SRC_INV = CMD_SRC_MAX,          /* not a valid owner */
  OWN_SRC_SAT,                        /* SIM application toolkit */
  OWN_SRC_MAX                         /* maximum owners */
} T_OWN;

/*==== TYPES ======================================================*/
typedef struct
{
  CHAR      num[MAX_DIAL_LEN];      /* party number/dial string */
  UBYTE     ton;                    /* type of number */
  UBYTE     npi;                    /* numbering plan */
  CHAR      sub[MAX_SUBADDR_LEN];   /* party subaddress */
  UBYTE     tos;                    /* type of subaddress */
  UBYTE     oe;                     /* odd/even indicator */
} T_CLPTY_PRM;

#ifdef GPRS
/*
 *    !!!  ATTENTION  !!!
 *    This struct will be writed in one block to the FFS.
 *    So the position of the variables can not be changed!
 */
typedef struct 
{                                 /* default values */
  UBYTE   max_cid;                /* 2 */
  UBYTE   auto_attach;            /* manual mode */
  UBYTE   auto_detach;            /* mode off    */
  UBYTE   default_mobile_class;   /* BG */
  UBYTE   accm         ;          /*  0 (async control character map) */
  UBYTE   restart_timer;          /*  3 */
  UBYTE   max_configure;          /* 10 */
  UBYTE   max_terminate;          /*  2 */
  UBYTE   max_failure;            /*  5 */

} T_FFS_GPRS_ACI;
#endif /* GPRS */

/*==== PROTOTYPES =================================================*/
EXTERN BOOL  psa_IsVldOwnId ( T_OWN ownId );
EXTERN UBYTE psa_timeout    ( USHORT handle );

#ifdef GPRS
EXTERN void psa_GPRSInit ( void );
#endif /* GPRS */

/*==== EXPORT =====================================================*/

#endif /* PSA_H */

/*==== EOF ========================================================*/

