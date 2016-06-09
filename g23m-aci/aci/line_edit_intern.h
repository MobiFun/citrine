/*
+-----------------------------------------------------------------------------
|  Project :  
|  Modul   :  line edit
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
|  Purpose :  This modul ...
+-----------------------------------------------------------------------------
*/

#ifndef LINE_EDIT_INTERN_H
#define LINE_EDIT_INTERN_H

#define WS 0x20
#define SLASH '/'

typedef struct S_LEDIT_ATCMD_M
{
         T_LEDIT_ATCMD   *cmd;
  struct S_LEDIT_ATCMD_M *next;
} T_LEDIT_ATCMD_M;



typedef struct S_LEDIT_INTERN
{
  UBYTE   src_id;
  USHORT  len;
  BOOL    isStr;                                         /* flag for start and end of "....."       */
  USHORT  copyIter;                                      /* copy iterator from src to cmd buffer    */
  USHORT  srcBufIter;                                    /* iterator over the srcBuffer             */
  USHORT  srcBufIterE;                                   /* from here all chars to echo             */
  const
  UBYTE  *srcBuffer;
                                                         
  char   *lineBuffer;                                    /* cmds without "AT" and removed WS,BS,... */
  USHORT lineBufIter;

  char   *origBuffer;                                    /* cmds without "AT", removed WS,BS,...
                                                          - NOT converted to upper case */
  USHORT origBufIter;
                                                         
  char   *execBuffer;                                    /* the current command(s) as well as       */
  USHORT execBufIter;                                    /* for AT/ the copy of the last cmd line   */

  char   *smsBuffer;                                     /* text of a SMS                           */
  BOOL    txtChunk;                                      /* received a chunk of text only           */
  T_LEDIT_ATCMD_M *cmdm;                                 /*                                         */
  UBYTE   cmdIndex;                                      /*                                         */
  UBYTE   cmdGetIter;                                    /* needed in ledit_get_next()              */
  T_LEDIT_RSLT (*state) (struct S_LEDIT_INTERN *atiInt); /* state machine function pointer          */
  T_LEDIT lineHabit;                                     /* the behaviour of the line               */
  T_LEDIT_ERR  err;
} T_LEDIT_INTERN;


typedef struct S_LEDIT_SRC_MAINTAIN
{
         T_LEDIT_INTERN       *leditInt;
  struct S_LEDIT_SRC_MAINTAIN *next;
} T_LEDIT_SRC_MAINTAIN;


/* moved from line_edit.h to avoid multiple memory allocation in different modules */
static const T_LEDIT_ERR ledit_err[] =
{
  {"out of memory",             LEDIT_ERR_NoMemory},
  {"no maintenance for src_id", LEDIT_ERR_NoMaintenance}, 
  {"no valid command",          LEDIT_ERR_NoValidCommand},
  {"last cmd line is pending",  LEDIT_ERR_LastCmdPending},
  {"unkown error",              LEDIT_ERR_Unknown} 
};


#endif
