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

#ifndef LINE_SPLIT_INTERN_H
#define LINE_SPLIT_INTERN_H

#define LEDIT_MAX_CMD_NAME_LEN (17)
#define LEDIT_MAX_S_NAME_LEN   (2)

#define SEARCH_BAT_LEN  (0)
#define SEARCH_DIAL_LEN (1)
#define SEARCH_EXT_LEN  (2)

#define BATCMD_NO_SUPP  (0)
#define BATCMD_SUPPORT  (1)
#define BATCMD_DIAL     (2) /* D <---- SPECIAL */
#define BATCMD_S_PARAM  (3) /* S <---- SPECIAL */

#define ANDCMD_NO_SUPP  (0)
#define ANDCMD_SUPPORT  (1)


typedef enum
{
  LEDIT_CHAR_TERMINATED = 0x00,
  LEDIT_CHAR_PERCENT    = 0x25, /* '%' */
  LEDIT_CHAR_AND        = 0x26, /* '&' */
  LEDIT_CHAR_PLUS       = 0x2B, /* '+' */
  LEDIT_CHAR_COMMA      = 0x2C, /* ',' */
  LEDIT_CHAR_NUMBER     = 0x30, /* '0' symbolic for 0..9 */
  LEDIT_CHAR_SEPARATOR  = 0x3B, /* ';' */
  LEDIT_CHAR_SET        = 0x3D, /* '=' */
  LEDIT_CHAR_READ       = 0x3F, /* '?' */
  LEDIT_CHAR_CHAR       = 0x41, /* 'A' symbolic for A..Z*/
  LEDIT_CHAR_EXT         = 0x5B, /*'[' symbolic for the rest of characters*/
  LEDIT_CHAR_INVALID    = 0xFF
} T_LEDIT_CHAR_TYPE;

typedef enum
{
  LEDIT_BASIC_NAME, /* consists of one char only */
  LEDIT_S_NAME,     /* first char must be 'S' and then only digits */
  LEDIT_EXT_NAME    /* multi char name           */
} T_LEDIT_NAME_TYPE;

typedef enum
{
  LEDIT_BASIC_PARAM, /* consists of digits  only */
  LEDIT_DIAL_PARAM,  /* the ending ';' is important to inform about voice or data call */
  LEDIT_EXT_PARAM    /* digits ,, "strings" ;    */
} T_LEDIT_PARAM_TYPE;

EXTERN T_LEDIT_RSLT ledit_split      (T_LEDIT_INTERN *leditInt);
EXTERN void         ledit_free_cmd   (T_LEDIT_INTERN *leditInt);

#endif
