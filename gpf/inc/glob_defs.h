/* 
+------------------------------------------------------------------------------
|  File:       glob_defs.h
+------------------------------------------------------------------------------
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
|  Purpose :  Definitions for the fixed dimensions of frame.
+----------------------------------------------------------------------------- 
*/ 

#ifndef GLOB_DEFS_H
#define GLOB_DEFS_H

#include "gpfconf.h"

#define GUARD_PATTERN        0xAFFEDEAD

#ifdef _ESF_SUPPORT_
 #define RESOURCE_NAMELEN           16
#else
 #define RESOURCE_NAMELEN            8
#endif /* _ESF_SUPPORT_ */

#endif
