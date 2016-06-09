/* 
+----------------------------------------------------------------------------- 
|  Project :  PCO2
|  Modul   :  PCO_PDI
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
|  Purpose :  This header provides functions to read and store data for
|             usage with the pdi-module (Primitive Decoding Information).
|             Furthermore filter functions are included. (filtering by opc,
|             up/downlink and air message id)
+----------------------------------------------------------------------------- 
*/ 


#ifndef _PCO_PDI_H_
#define _PCO_PDI_H_

/*==== INCLUDES ==================================================*/
#include "ccdapi.h"
extern "C" {
#include "pdi.h"
}

/*==== GLOABAL FUNCTION DECLARATIONS =============================*/

/*
+---------------------------------------------------------------------+
| PROJECT : PCO2                       MODULE  : PCO_PDI              |
| STATE   : code                       ROUTINE : pco_pdi_init         |
+---------------------------------------------------------------------+

  PURPOSE : initalized pdi data for pco

  PARAMS:   fname .. name of file with pdi data

  RETURNS:   0 .. sucess
            -1 .. otherwise

*/
int pco_pdi_init(const char* fname);

int pco_pdi_exit();


T_PDI_CONTEXT* pco_pdi_context();


#endif /*_PCO_PDI_H_*/