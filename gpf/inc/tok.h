/*
+------------------------------------------------------------------------------
|  File:       tok.h
+------------------------------------------------------------------------------
|                 Copyright 2002 Texas Instruments Berlin, AG
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Texas
|                 Instruments Berlin, AG.
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Texas Instruments Berlin, AG.
+------------------------------------------------------------------------------
| Purpose:    Definitions for the configuration string functions.
+------------------------------------------------------------------------------
*/


#ifndef __TOK_H__
#define __TOK_H__

/*==== CONSTANTS ==================================================*/

#define TOK_ERRBASE    (-300)
#define TOK_OK         (0)
#define TOK_EOCS       (TOK_ERRBASE - 1)
#define TOK_NOT_FOUND  (TOK_ERRBASE - 2)

/*==== TYPES ======================================================*/

typedef struct KW_DATA
{
   const char * const keyword;
   const SHORT code;   
} KW_DATA;

typedef struct TOK_DCB
{
  char   * tokbuf;
  char   * nexttok;
  char   lastchar;
} TOK_DCB;

/*==== EXPORT =====================================================*/

void  tok_init (char *);
SHORT tok_next (char **, char **);
SHORT tok_key  (KW_DATA *, char *);

#endif

