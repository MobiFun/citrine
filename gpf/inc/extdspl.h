#ifndef EXTDSPL_H
#define EXTDSPL_H
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
|  Purpose :  Definitions of the EXTDSPL service access point.
+----------------------------------------------------------------------------- 
*/ 


/*
 * Definition of the SAP
 */
#define SAP_EXTDSPL


/*
 * Mask for Opcodes
 */
#define EXTDSPL_DL  0x7F00
#define EXTDSPL_UL  0x3F00


/*
 * The primitves and their OPC's
 */
typedef struct 
{
  U32 dummy;
} T_EXTDSPL_CAP_REQ;

#define EXTDSPL_CAP_REQ 0x3F00


typedef struct 
{
  U32 dummy;
} T_EXTDSPL_DATA_REQ;

#define EXTDSPL_DATA_REQ 0x3F01


typedef struct 
{
  U16 width;
  U16 height;
  U16 bpp;
  U8  _dallign1;
  U8  _dallign2;
} T_EXTDSPL_CAP_IND;

#define EXTDSPL_CAP_IND 0x7F00


typedef struct 
{
  U16 col;
  U16 row;
  U16 width;
  U16 height;
  T_sdu sdu;
} T_EXTDSPL_DATA_IND;

#define EXTDSPL_DATA_IND 0x7F01


#endif /* EXTDSPL_H */
