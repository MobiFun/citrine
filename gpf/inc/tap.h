/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   : tap.h
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
|  Purpose : Headerfile containing exit codes of Tap.
+-----------------------------------------------------------------------------
*/

#ifndef TAP_H
#define TAP_H

/*==== INCLUDES =============================================================*/
/*==== CONSTS ===============================================================*/

#define TAP_PASSED        0
#define TAP_CMD_LINE      1
#define TAP_TC_NOT_FOUND  2
#define TAP_TC_NO_INIT    3
#define TAP_TC_VERSION    4
#define TAP_SUI_CHECK     5
#define TAP_FAILED        6
#define TAP_EXCLUDED      7
#define TAP_STACKSYNC     8
#define TAP_TDC_SYNTAX    9
#define TAP_TDC_INTERNAL 10
#define TAP_CCDDATA_DLL  11
#define TAP_STACK_ERR    12
#define TAP_STACK_WARN   13
#define TAP_TC_CRASH     14
#define TAP_UNKNOWN       99
/*==== TYPES ================================================================*/
/*==== EXPORTS ==============================================================*/
#endif /* !TAP_H */
