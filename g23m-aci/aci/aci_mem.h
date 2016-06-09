/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci\aci_mem.h
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
#ifndef ACI_MEM_H
#define ACI_MEM_H

/* #define ACI_MEMORY_TRACE */

#ifdef ACI_MEMORY_TRACE

EXTERN void *aci_malloc (USHORT size, char* file, int line);
#define ACI_MALLOC(P,S)  { P = (void*)aci_malloc(S, __FILE__, __LINE__);}

#if (CHIPSET==15)
EXTERN void *aci_malloc_nb (USHORT size, char* file, int line);
#define ACI_MALLOC_NB(P,S)  { P = (void*)aci_malloc_nb(S, __FILE__, __LINE__);}
#endif /* (CHIPSET==15) */

EXTERN void  aci_mfree  (void *ptr, char* file, int line);
#define ACI_MFREE(P) aci_mfree(P, __FILE__, __LINE__)

#else  /* ifdef ACI_MEMORY_TRACE */

#define ACI_MALLOC  MALLOC

#if (CHIPSET==15)
#define ACI_MALLOC_NB MALLOC_NB
#endif /* (CHIPSET==15) */

#define ACI_MFREE   MFREE

#endif /* ifdef ACI_MEMORY_TRACE */

#endif
