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
|  Purpose :  Constants to determine the dimensions of the frame
+-----------------------------------------------------------------------------
*/

#ifndef GSMCONST_H
#define GSMCONST_H

/*==== CONSTANTS ============================================================*/

#ifndef _TARGET_
#define BASE_ENTITIES              17
#define BASE_OS_TASKS              15
#else
#define BASE_ENTITIES              14
#define BASE_OS_TASKS              12
#endif /* #ifndef _TARGET_ */

#ifdef FAX_AND_DATA
#ifdef _TARGET_
#define FAX_AND_DATA_ADD_ENTITIES   4
#define FAX_AND_DATA_ADD_OS_TASKS   2
#else
#define FAX_AND_DATA_ADD_ENTITIES   5
#define FAX_AND_DATA_ADD_OS_TASKS   3
#endif /* #ifndef _TARGET_ */
#else
#define FAX_AND_DATA_ADD_ENTITIES   0
#define FAX_AND_DATA_ADD_OS_TASKS   0
#endif /* else, #ifdef FAX_AND_DATA */

#if defined(FF_TCP_IP) && !defined(_TARGET_)
#define FF_TCP_IP_ADD_ENTITIES      1
#define FF_TCP_IP_ADD_OS_TASKS      1
#else
#define FF_TCP_IP_ADD_ENTITIES      0
#define FF_TCP_IP_ADD_OS_TASKS      0
#endif /* else, #if defined(FF_TCP_IP) && !defined(_TARGET_) */

#ifdef FF_EOTD
#define FF_EOTD_ADD_ENTITIES        2
#define FF_EOTD_ADD_OS_TASKS        1
#else
#define FF_EOTD_ADD_ENTITIES        0
#define FF_EOTD_ADD_OS_TASKS        0
#endif /* else, #ifdef FF_EOTD */

#ifdef FF_WAP
#ifndef UDP_NO_WAP
#define FF_WAP_ADD_ENTITIES         4
#define FF_WAP_ADD_OS_TASKS         4
#else
#define FF_WAP_ADD_ENTITIES         3
#define FF_WAP_ADD_OS_TASKS         3
#endif /* else, #ifndef UDP_NO_WAP */
#else
#define FF_WAP_ADD_ENTITIES         0
#define FF_WAP_ADD_OS_TASKS         0
#endif /* FF_WAP */

#ifdef BT_ADAPTER
#define BT_ADAPTER_ADD_ENTITIES     1
#define BT_ADAPTER_ADD_OS_TASKS     1
#else
#define BT_ADAPTER_ADD_ENTITIES     0
#define BT_ADAPTER_ADD_OS_TASKS     0
#endif /* else, #ifdef BT_ADAPTER */

#define MAX_ENTITIES (BASE_ENTITIES+\
                      FAX_AND_DATA_ADD_ENTITIES+\
                      FF_EOTD_ADD_ENTITIES+\
                      FF_WAP_ADD_ENTITIES+\
                      BT_ADAPTER_ADD_ENTITIES)

#define MAX_OS_TASKS (BASE_OS_TASKS+\
                      FAX_AND_DATA_ADD_OS_TASKS+\
                      FF_EOTD_ADD_OS_TASKS+\
                      FF_WAP_ADD_OS_TASKS+\
                      BT_ADAPTER_ADD_OS_TASKS)

/* Detecting GSM only lite to compile using minimal settings */
#if !defined(FAX_AND_DATA) && !defined(FF_TCP_IP) && !defined(FF_EOTD) &&\
    !defined(FF_WAP) && !defined(BT_ADAPTER)
#define GO_LITE 1
#endif

#define MAX_TIMER                  128
#ifdef GO_LITE
#define MAX_SIMULTANEOUS_TIMER      20
#else
#define MAX_SIMULTANEOUS_TIMER      40
#endif /* else, #ifdef GO_LITE */

#define MAX_OSISRS                  0

#define MAX_SEMAPHORES              13
#define MAX_COMMUNICATIONS          MAX_OS_TASKS

#define MAX_POOL_GROUPS              6
#define MAX_MEMORY_POOLS             6

#undef GO_LITE /* We are not going to use this definition globally */

#endif /* #ifndef GSMCONST_H */
