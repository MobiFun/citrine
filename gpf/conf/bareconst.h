/*
 * If we are building a configuration with just "bare" GPF, plus maybe L1,
 * but no GSM or GPRS L23 stack, then this bare{comp.c,const.h} config
 * takes the place of gsm* or gprs*.
 */

#ifndef __BARECONST_H
#define __BARECONST_H

/*==== CONSTANTS ============================================================*/

#define MAX_TIMER                   64
#define MAX_SIMULTANEOUS_TIMER      40

#define MAX_ENTITIES                10
#define MAX_OS_TASKS                10
#define MAX_SEMAPHORES               8

#define MAX_OSISRS                   0

#define MAX_COMMUNICATIONS           MAX_OS_TASKS
#define MAX_POOL_GROUPS              6
#define MAX_MEMORY_POOLS             6

#endif	/* include guard */
