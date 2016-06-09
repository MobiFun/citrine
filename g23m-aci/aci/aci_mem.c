/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci\aci_mem.c
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
#ifndef ACI_MEM_C
#define ACI_MEM_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_mem.h"

#ifdef ACI_MEMORY_TRACE

GLOBAL void aci_mfree (void *ptr, char *file, int line)
{
  TRACE_EVENT_P3 ("MFREE : 0x%X  (%d : %s)", ptr, line, file);
  MFREE (ptr);
}

GLOBAL void *aci_malloc (USHORT size, char *file, int line)
{
  void *newP;

  MALLOC (newP, size);
  TRACE_EVENT_P4 ("MALLOC: 0x%X  %d (%d : %s)", 
                  newP, size, line, file);
  return(newP);
}

#if (CHIPSET==15)
GLOBAL void *aci_malloc_nb (USHORT size, char *file, int line)
{
  void *newP;

  MALLOC_NB (newP, size);
  TRACE_EVENT_P4 ("MALLOC_NB: 0x%X  %d (%d : %s)", 
                  newP, size, line, file);
  return(newP);
}
#endif /* (CHIPSET==15) */

#endif /* ACI_MEMORY_TRACE */
