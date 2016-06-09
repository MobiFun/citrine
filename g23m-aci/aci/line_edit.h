/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  line
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

#ifndef LINE_EDIT_H
#define LINE_EDIT_H

/*
 ****************************************************************************************************
 * public stuff                                                                                     *
 ****************************************************************************************************
 */
typedef enum
{
  LEDIT_FAIL = -1,
  LEDIT_EXCT =  0,
  LEDIT_CMPL,
  LEDIT_COLLECT,
  LEDIT_ESC,     /* in text mode do nothing when received ESC */
  LEDIT_OK,      /* for single <CR> and simple "AT<CR>"       */
  LEDIT_IGNORE   /* ignore chars before A and between A..T    */
} T_LEDIT_RSLT;

typedef enum 
{
         LEDIT_ERR_NONE   = -1,
/*000*/  LEDIT_ERR_NoMemory ,
/*001*/  LEDIT_ERR_NoMaintenance,
/*002*/  LEDIT_ERR_NoValidCommand,
/*003*/  LEDIT_ERR_LastCmdPending,
/*004*/  LEDIT_ERR_Unknown
} T_LEDIT_ERR_CODE;

typedef struct  /* error code - error message mapping */
{
  CHAR*            msg;  /* error message */
  T_LEDIT_ERR_CODE code; /* error code    */
} T_LEDIT_ERR;


typedef enum
{
  LEDIT_CTRL_ERROR,
  LEDIT_CTRL_CMPL,
  LEDIT_CTRL_REMOVE_SRC,
  LEDIT_CTRL_PROMPT,
  LEDIT_CTRL_CRLF_PROMPT,
  LEDIT_CTRL_MORE_CMDS
} T_LEDIT_CTRL;


typedef struct
{
  UBYTE  S3;    /* CR  */
  UBYTE  S4;    /* LF  */
  UBYTE  S5;    /* BS  */
  UBYTE  atE;   /* Command Echo: ON                   */
  UBYTE  smsEnd;/* want to be non V25.ter conform     */
} T_LEDIT;      /* initial  = {13, 10,  8, 1, 0x1a};  */
                /*             S3  S4  S5  E, smsEnd  */
                /* done by liblineedit.a              */
                /* can be overwritten by ledit_ctrl() */

typedef enum
{
  LEDIT_ATCMD_NIL        = 0x00,
  LEDIT_ATCMD_AND        = 0x26,  /* '&' */
  LEDIT_ATCMD_BASIC      = 0x42,  /* 'B' */
  LEDIT_ATCMD_DIAL       = 0x44,  /* 'D' */
  LEDIT_ATCMD_EXTENDED   = 0x45,  /* 'E' */
  LEDIT_ATCMD_S          = 0x53   /* 'S' */
} T_LEDIT_ATCMD_MAINTYPE;

typedef enum
{
  LEDIT_ATCMD_QUERY      = 0x51,
  LEDIT_ATCMD_SET        = 0x53,  /* 'S' */
  LEDIT_ATCMD_TEST       = 0x54   /* 'T' */
} T_LEDIT_ATCMD_SUBTYPE;

typedef struct
{
  T_LEDIT_ATCMD_MAINTYPE mType;
  T_LEDIT_ATCMD_SUBTYPE  sType;  
  UBYTE append_Flag;
} T_LEDIT_ATCMD_TYPE;


typedef struct
{
  T_LEDIT_ATCMD_TYPE  type;   
  char  *name;   
  char  *params;  
} T_LEDIT_ATCMD;

/*
 *  taken from ACI.doc, must be removed when including "P_ACI.val" !
 *  #define MAX_CMD_LEN 350
 */

/*
 * global echo buffer
 */
EXTERN char g_ledit_echoBuf[MAX_CMD_LEN+1];
/* 
 *  public functions
 */
EXTERN T_LEDIT_RSLT ledit_cmd        (UBYTE src_id, const UBYTE *chars, USHORT len);
EXTERN T_LEDIT_RSLT ledit_text       (UBYTE src_id, const UBYTE *chars, USHORT len);
EXTERN T_LEDIT_RSLT ledit_get_first  (UBYTE src_id, T_LEDIT_ATCMD **cmd);
EXTERN T_LEDIT_RSLT ledit_get_next   (UBYTE src_id, T_LEDIT_ATCMD **cmd);
EXTERN T_LEDIT_RSLT ledit_get_current(UBYTE src_id, T_LEDIT_ATCMD **cmd);
EXTERN T_LEDIT_RSLT ledit_get_text   (UBYTE src_id, char **txt);
EXTERN T_LEDIT_RSLT ledit_ctrl       (UBYTE src_id, T_LEDIT_CTRL ctrl, T_LEDIT_ERR **err);
EXTERN T_LEDIT_RSLT ledit_set_config (UBYTE src_id, T_LEDIT line);
EXTERN T_LEDIT_RSLT ledit_trace_line (UBYTE src_id, char *txt);

#endif
