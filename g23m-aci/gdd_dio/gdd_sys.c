/* 
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
+-----------------------------------------------------------------------------
|  Copyright 2004 Texas Instruments Berlin, AG
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
|  Purpose :  Implementation of the GDD system interface
+-----------------------------------------------------------------------------
*/

#define GDD_SYS_C

/*==== INCLUDES =============================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"

#include "gdd_dio.h"

#include "gdd_sys.h"

/*==== DEFINITIONS===========================================================*/

#ifdef GDD_DIO_USE_EXTERNAL_MEM_POOL
#define  EXT_ALLOC(S)    (void*)vsi_m_new(S,ExtGroupHandle FILE_LINE_MACRO)
#define  EXT_FREE(P)     vsi_m_free((T_VOID_STRUCT **)&P FILE_LINE_MACRO)

/* Handle to external memory pool dediated to GDD_SYS */
T_HANDLE ExtGroupHandle = 3;

#endif /* GDD_DIO_USE_EXTERNAL_MEM_POOL */


/*==== TYPES ================================================================*/

/*==== PROTOTYPES ===========================================================*/

/*==== GLOBAL VARS ==========================================================*/

/*==== LOCAL FUNCTIONS=======================================================*/

/*==== EXPORTED FUNCTIONS====================================================*/

void * gdd_sys_mem_malloc(int size)
{
  void * p;
  
  TRACE_USER_CLASS_P1(TC_SYS_INTERFACE, "[GDD] gdd_sys_mem_malloc(size=%d)", size);

#ifdef GDD_DIO_USE_EXTERNAL_MEM_POOL
  p = EXT_ALLOC(size);
#else /* GDD_DIO_USE_EXTERNAL_MEM_POOL */
  p = M_ALLOC(size);
#endif /* GDD_DIO_USE_EXTERNAL_MEM_POOL */
  TRACE_USER_CLASS_P1(TC_SYS_INTERFACE, "[GDD] allocated buffer: 0x%4x", p);

  return p;
}

void gdd_sys_mem_free(void * p)
{
  TRACE_USER_CLASS_P1(TC_SYS_INTERFACE, "[GDD] gdd_sys_mem_free(p=0x%4x)", p);

#ifdef GDD_DIO_USE_EXTERNAL_MEM_POOL
  EXT_FREE(p);
#else /* GDD_DIO_USE_EXTERNAL_MEM_POOL */
  M_FREE(p);
#endif /* GDD_DIO_USE_EXTERNAL_MEM_POOL */
}

T_GDD_SEM gdd_sys_sem_open(char * name, unsigned short count)
{
  TRACE_USER_CLASS_P2(TC_SYS_INTERFACE, "[GDD] gdd_sys_sem_open(name=%s, count=%d)", name, count);

  return vsi_s_open(VSI_CALLER name, count);
}

int gdd_sys_sem_close(T_GDD_SEM sem)
{
  TRACE_USER_CLASS_P1(TC_SYS_INTERFACE, "[GDD] gdd_sys_sem_close(sem=%d)", sem);

  return vsi_s_close(VSI_CALLER sem);
}

int gdd_sys_sem_down(T_GDD_SEM sem)
{
  TRACE_USER_CLASS_P1(TC_SYS_INTERFACE, "[GDD] gdd_sys_sem_down(sem=%d)", sem);

  return vsi_s_get(VSI_CALLER sem);
}

int gdd_sys_sem_up(T_GDD_SEM sem)
{
  TRACE_USER_CLASS_P1(TC_SYS_INTERFACE, "[GDD] gdd_sys_sem_up(sem=%d)", sem);

  return vsi_s_release(VSI_CALLER sem);
}

int gdd_sys_sem_status(T_GDD_SEM sem, /*out*/ unsigned short * count)
{
  TRACE_USER_CLASS_P1(TC_SYS_INTERFACE, "[GDD] gdd_sys_sem_status(sem=%d)", sem);

  return vsi_s_status(VSI_CALLER sem, count); 
}
