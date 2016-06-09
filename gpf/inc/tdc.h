/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : TDC
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
#ifdef TDC_H
#error "TDC.H already included, TDC.H must come after all primitives and messages"
#endif

#include <setjmp.h>

//namespace tdc {
#include "tdc_prim.h"

#ifndef TDC_H
#define TDC_H

#ifndef TDC_DESCRIPTOR
static int tdc_initialized_primitive = T_PRIMITIVE_UNION::call_tdc_initialize_primitive();
static int tdc_initialized_message = T_MESSAGE_UNION::call_tdc_initialize_message();
#endif //TDC_DESCRIPTOR

#endif //TDC_H
//}//namespace tdc
//using namespace tdc;
