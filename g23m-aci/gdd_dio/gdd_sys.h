/*  
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  gdd_sys.h
+-----------------------------------------------------------------------------
|  Copyright 2005 Texas Instruments Berlin, AG
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
|  Purpose :  API for limited system functionality offered to application
|             domain libraries.
+-----------------------------------------------------------------------------
*/

#ifndef GDD_SYS_H 
#define GDD_SYS_H

/*==== DEFINITIONS ==========================================================*/

typedef int T_GDD_SEM;


/*==== DYNAMIC MEMORY FUNCTIONS =============================================*/


/*
+------------------------------------------------------------------------------
| Function    : gdd_sys_mem_malloc
+------------------------------------------------------------------------------
| Description : Allocate memory.
|
| Parameters  : size           - Size of memory required.
|
| Return      : Pointer to first byte of allocated memory.
|               0 if allocation failed.
+------------------------------------------------------------------------------
*/
void * gdd_sys_mem_malloc(int size);


/*
+------------------------------------------------------------------------------
| Function    : gdd_sys_mem_free
+------------------------------------------------------------------------------
| Description : Release memory which was previously allocated.
|
| Parameters  : p              - Pointer to memory to be deallocated
+------------------------------------------------------------------------------
*/
void gdd_sys_mem_free(void * p);


/*==== SEMAPHORE FUNCTIONS ==================================================*/


/*
+------------------------------------------------------------------------------
| Function    : gdd_sys_sem_open
+------------------------------------------------------------------------------
| Description : Opens a (counting) semaphore specified by its name. If the
|               semaphore does not exist, it will be created with the initial
|               count given. If the semaphore already exists the parameter count
|               will be ignored.
|
| Parameters  : name              - Some name to identify the semaphore
|               count             - initial count (e.g. 1 for a binary sem.)
|
| Return      : Returns handle to created semaphore, or -1 if an error occured.
+------------------------------------------------------------------------------
*/
T_GDD_SEM gdd_sys_sem_open(char * name, unsigned short count);


/*
+------------------------------------------------------------------------------
| Function    : gdd_sys_sem_close
+------------------------------------------------------------------------------
| Description : Closes a semaphore
|
| Parameters  : sem               - handle of semaphore
|
| Return      : 0 = succees
|               1 = error
+------------------------------------------------------------------------------
*/
int gdd_sys_sem_close(T_GDD_SEM sem);


/*
+------------------------------------------------------------------------------
| Function    : gdd_sys_sem_down
+------------------------------------------------------------------------------
| Description : This functions obtains the specified semaphore, i.e.
|               the counter is decremented, if it is greater than zero.
|               If the counter is equal to zero, than the calling task is
|               suspended until the counter is incremented by another task.
|               If the caller is a non-task thread the function returns
|               immediately regardless if the request can be satisfied or not.
|               In this case, -1 is returned if the counter was already zero. 
|
| Parameters  : sem               - handle of semaphore
|
| Return      : 0 = succees
|               1 = error
+------------------------------------------------------------------------------
*/
int gdd_sys_sem_down(T_GDD_SEM sem);


/*
+------------------------------------------------------------------------------
| Function    : gdd_sys_sem_down
+------------------------------------------------------------------------------
| Description : Releases a semaphore, i.e. counter is incremented.
|
| Parameters  : sem               - handle of semaphore
|
| Return      : 0 = succees
|               1 = error
+------------------------------------------------------------------------------
*/
int gdd_sys_sem_up(T_GDD_SEM sem);


/*
+------------------------------------------------------------------------------
| Function    : gdd_sys_sem_status
+------------------------------------------------------------------------------
| Description : Query (obtain) the counter of a semaphore.
|
| Parameters  : sem               - handle of semaphore
|               count             - output variable to pass back the counter
|
| Return      : 0 = succees
|               1 = error
+------------------------------------------------------------------------------
*/
int gdd_sys_sem_status(T_GDD_SEM sem, /*out*/ unsigned short * count);


#endif /* GDD_SYS_H */
