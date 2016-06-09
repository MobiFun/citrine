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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "typedefs.h"
#include "vsi.h"
#include "pei.h"
#include "aci_mem.h"
#include "p_aci.val"

#include "line_edit.h"
#include "line_edit_intern.h"
#include "line_split_intern.h"

static T_LEDIT_CHAR_TYPE     ledit_lookAhead             (char c);
static UBYTE                 ledit_check_valid_basic_cmd (char c);
static UBYTE                 ledit_check_valid_and_cmd   (char c);
static USHORT                ledit_get_para_len      (T_LEDIT_INTERN   *leditInt, USHORT searchLen);
static USHORT                ledit_get_name_len      (T_LEDIT_INTERN   *leditInt, T_LEDIT_NAME_TYPE nType);
static T_LEDIT_RSLT          ledit_get_memory        (T_LEDIT_ATCMD_M **cmdm, int size, int name);
static T_LEDIT_RSLT          ledit_get_cmd_mem       (T_LEDIT_INTERN   *leditInt);
static T_LEDIT_RSLT          ledit_assign_name       (T_LEDIT_INTERN   *leditInt, int len);
static T_LEDIT_RSLT          ledit_assign_param      (T_LEDIT_INTERN   *leditInt, T_LEDIT_PARAM_TYPE pType);
static T_LEDIT_ATCMD_SUBTYPE ledit_extended_type     (T_LEDIT_INTERN   *leditInt);
static T_LEDIT_RSLT          ledit_extended_cmd      (T_LEDIT_INTERN   *leditInt);
static T_LEDIT_RSLT          ledit_s_cmd             (T_LEDIT_INTERN   *leditInt);
static T_LEDIT_RSLT          ledit_dial              (T_LEDIT_INTERN   *leditInt);
static T_LEDIT_RSLT          ledit_and_basic_cmd     (T_LEDIT_INTERN   *leditInt);
static T_LEDIT_RSLT          ledit_basic_cmd         (T_LEDIT_INTERN   *leditInt);

static T_LEDIT_RSLT ledit_restore_dial_string (char *procDialString,
                                             int procDialStringLen,
                                             char *origCommandLine,
                                             int origCommandLineLen);


static const UBYTE basic_ATcmd [26][2] =
{
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* A */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* B */
  {BATCMD_NO_SUPP, ANDCMD_SUPPORT}, /* C */
  {BATCMD_DIAL   , ANDCMD_SUPPORT}, /* D <---- SPECIAL */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* E */
  {BATCMD_SUPPORT, ANDCMD_SUPPORT}, /* F */
  {BATCMD_NO_SUPP, ANDCMD_NO_SUPP}, /* G */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* H */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* I */
  {BATCMD_NO_SUPP, ANDCMD_NO_SUPP}, /* J */
  {BATCMD_NO_SUPP, ANDCMD_SUPPORT}, /* K */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* L */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* M */
  {BATCMD_NO_SUPP, ANDCMD_NO_SUPP}, /* N */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* O */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* P */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* Q */
  {BATCMD_NO_SUPP, ANDCMD_NO_SUPP}, /* R */
  {BATCMD_S_PARAM, ANDCMD_NO_SUPP}, /* S <---- SPECIAL */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* T */
  {BATCMD_NO_SUPP, ANDCMD_NO_SUPP}, /* U */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* V */
  {BATCMD_SUPPORT, ANDCMD_SUPPORT}, /* W */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}, /* X */
  {BATCMD_NO_SUPP, ANDCMD_NO_SUPP}, /* Y */
  {BATCMD_SUPPORT, ANDCMD_NO_SUPP}  /* Z */
};


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)     MODULE  : LINE_EDIT_LIB               |
| STATE   : code               ROUTINE : ledit_check_valid_basic_cmd |
+--------------------------------------------------------------------+

  PURPOSE : - 
*/
static UBYTE  ledit_check_valid_basic_cmd (char c)
{
  return  basic_ATcmd [(int)(c-'A')][0];
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : LINE_EDIT_LIB              |
| STATE   : code                ROUTINE : ledit_check_valid_and_cmd  |
+--------------------------------------------------------------------+

  PURPOSE : - 
*/
static UBYTE  ledit_check_valid_and_cmd (char c)
{
  return  basic_ATcmd [(int)(c-'A')][1];
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : LINE_EDIT_LIB         |
| STATE   : code                     ROUTINE : ledit_lookAhead       |
+--------------------------------------------------------------------+

  PURPOSE : - qualify the character at the look ahead position
*/

static T_LEDIT_CHAR_TYPE ledit_lookAhead (char c)
{
  if (c EQ '\0')
    return LEDIT_CHAR_TERMINATED;

  switch (c)
  {
    case '%':
      return LEDIT_CHAR_PERCENT;
    case '&':
      return LEDIT_CHAR_AND;
    case '+':
      return LEDIT_CHAR_PLUS;
    case ',':
      return LEDIT_CHAR_COMMA;
    case '=':
      return LEDIT_CHAR_SET;
    case ';':
      return LEDIT_CHAR_SEPARATOR;
    case '?':
      return LEDIT_CHAR_READ;
    default:
    {
      if ((c-'0') < 10 AND (c - '0') >=0)
        return LEDIT_CHAR_NUMBER;
      if ((c>='A') && (c<='Z'))
        return LEDIT_CHAR_CHAR;
      if (((c >= '!') AND (c <= '_')) OR ((c >= 123) AND ((unsigned char)c <= 127)))
      /*the rest of characters*/
        return LEDIT_CHAR_EXT;
      return LEDIT_CHAR_INVALID;
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : LINE_EDIT_LIB         |
| STATE   : code                     ROUTINE : ledit_get_para_len    |
+--------------------------------------------------------------------+

  PURPOSE : - calculate length of an AT command parameter
*/
static USHORT ledit_get_para_len (T_LEDIT_INTERN *leditInt, USHORT searchLen)
{
  USHORT i = 0;
#ifdef _SIMULATION_  
  TRACE_FUNCTION("ledit_get_para_len()");
#endif
  if (searchLen EQ SEARCH_BAT_LEN)
  {
    /* basic AT command can have numbers only e.g.: ATF123 */
    i = leditInt->execBufIter; /* start with first number   */
    while (isdigit(leditInt->execBuffer[i]))
    {
      ++i;
    }
    return (i - leditInt->execBufIter);
  }
  else
  {
    /* 
     * dial string finish with ';'
     * extended AT command is terminated by ';' or by '\0' if last one in cmd line
     */
    i = leditInt->execBufIter;
   
    while (';' != leditInt->execBuffer[i])
    {
       ++i;
       if (leditInt->execBuffer[i] EQ '\0')
       {
         break;
       }
    }
    if ((searchLen EQ SEARCH_DIAL_LEN) AND (';' EQ leditInt->execBuffer[i]))
    {
       ++i; /* for dial the ';' is important */
    }
    return (i - leditInt->execBufIter);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : LINE_EDIT_LIB         |
| STATE   : code                     ROUTINE : ledit_get_name_len    |
+--------------------------------------------------------------------+

  PURPOSE : - get the length of an extended AT command or ATSnn where nn := 0..99
              extended name starts with '+', '%', '$' ... or every other char which is not [a-zA-Z][0-9]
              after prefix must start with [A-Z] and then up to further 15 characters
              of [A-Z][0-9]!%-./:_  according to V.25 ter  chapter 5.4.1
*/
static USHORT ledit_get_name_len (T_LEDIT_INTERN *leditInt, T_LEDIT_NAME_TYPE nType)
{
  USHORT i = leditInt->execBufIter;

  if (nType EQ LEDIT_EXT_NAME)
  {
    ++i; /* skip prefix of extended name */
  }
  /*
   * first char (after extended prefix) must be [A-Z] (to capital letter done by ledit_collect())
   */
  if (isupper (leditInt->execBuffer[i]))
  {
    while (leditInt->execBuffer[i])
    {
      if ((nType EQ LEDIT_S_NAME) AND (i > leditInt->execBufIter))
      {
        if (isdigit(leditInt->execBuffer[i]) EQ 0)
        {
          return 0; /* invalid name for ATSnn, where nn must be [0-9]+ */
        }
        if ((i - leditInt->execBufIter) > LEDIT_MAX_S_NAME_LEN)
        {
          return 0; /* invalid name for ATSnn, where nn is too large */
        }
      }
      ++i;
      switch (leditInt->execBuffer[i])
      {
        case '\0':
        {
          return (i - leditInt->execBufIter);
        }
        case '=':
        {
          if (leditInt->execBuffer[i+1] NEQ '?')
          {
            return (i - leditInt->execBufIter);
          }
          else /* AT+XXX=? */
          {
            if ((leditInt->execBuffer[i+2] NEQ '\0') AND (leditInt->execBuffer[i+2] NEQ ';'))
            {
              return 0; /* after =? must be ';' or terminated */
            }
            return (i - leditInt->execBufIter);
          }
        }
        case '?':
        {
          if ((leditInt->execBuffer[i+1] NEQ '\0') AND (leditInt->execBuffer[i+1] NEQ ';'))
          {
            return 0; /* after ? must be ';' or terminated */
          }
          return (i - leditInt->execBufIter);
        }
        case ';':
        {
          /*
           * there are several extended AT cmds without "=parameter" or "=?" or "?" after the name, e.g.: +CGMM;+CGMI;+CGMR;+CGSN;...
           * the ";" after the name will be parsed in ledit_get_para_len()
           */
          return (i - leditInt->execBufIter);
        }
      }
    } /* end of while (leditInt->execBuffer[i]) */
  }
  return 0; /* invalid name */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_free_cmd     |
+--------------------------------------------------------------------+

  PURPOSE :  - 
*/
void ledit_free_cmd (T_LEDIT_INTERN *leditInt)
{
 
  T_LEDIT_ATCMD_M *tmp = NULL;
#ifdef _SIMULATION_  
  TRACE_FUNCTION("ledit_free_cmd()");
#endif
  while (leditInt->cmdm)
  {
    tmp = leditInt->cmdm->next;
    if (leditInt->cmdm->cmd)
    {
      if (leditInt->cmdm->cmd->name)
      {
        ACI_MFREE(leditInt->cmdm->cmd->name);
      }
      if (leditInt->cmdm->cmd->params)
      {
        ACI_MFREE (leditInt->cmdm->cmd->params);
      }
      ACI_MFREE (leditInt->cmdm->cmd);
    }
/*    if (leditInt->cmdm)  ... always true */
    {
      ACI_MFREE (leditInt->cmdm);
    }
    leditInt->cmdm = tmp;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_get_cmd_mem  |
+--------------------------------------------------------------------+

  PURPOSE :  - allocate some memory for T_LEDIT_ATCMD
*/

static T_LEDIT_RSLT ledit_get_cmd_mem (T_LEDIT_INTERN *leditInt)
{
  ACI_MALLOC(leditInt->cmdm,sizeof(T_LEDIT_ATCMD_M));
  if (leditInt->cmdm EQ NULL)
  {
    return LEDIT_FAIL;
  }
  memset(leditInt->cmdm, '\0', sizeof(T_LEDIT_ATCMD_M));
  leditInt->cmdm->next = NULL;

  ACI_MALLOC(leditInt->cmdm->cmd,sizeof(T_LEDIT_ATCMD));
  if (leditInt->cmdm->cmd EQ NULL)
  {
    return LEDIT_FAIL;
  }
  memset(leditInt->cmdm->cmd, '\0', sizeof(T_LEDIT_ATCMD));

  leditInt->cmdm->cmd->type.mType = LEDIT_ATCMD_NIL;
  leditInt->cmdm->cmd->type.sType = (T_LEDIT_ATCMD_SUBTYPE) LEDIT_ATCMD_NIL;
  /* Initializing flag here BUG 11920 */
  leditInt->cmdm->cmd->type.append_Flag = FALSE;
  leditInt->cmdm->cmd->name       = NULL;
  leditInt->cmdm->cmd->params     = NULL;
  return LEDIT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_get_memory   |
+--------------------------------------------------------------------+

  PURPOSE :  - allocate some memory (for char *)
*/

static T_LEDIT_RSLT ledit_get_memory (T_LEDIT_ATCMD_M **cmdm, int size, int name)
{

  T_LEDIT_ATCMD_M **tmp = cmdm;
  if (name)
  {
    ACI_MALLOC((*tmp)->cmd->name,size * sizeof(char));
    if ((*tmp)->cmd->name EQ NULL)
    {
      return LEDIT_FAIL;
    }
    (*tmp)->cmd->name[0]='\0';
  }
  else
  {
    ACI_MALLOC((*tmp)->cmd->params,size * sizeof(char));
    if ((*tmp)->cmd->params EQ NULL)
    {
      TRACE_FUNCTION("ledit_get_memory(): returning fail");
      return LEDIT_FAIL;
    }
    (*tmp)->cmd->params[0]='\0';
  }
  return LEDIT_CMPL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_assign_name  |
+--------------------------------------------------------------------+

  PURPOSE :  - extract the name of an AT command from command line
               and put it into T_LEDIT_ATCMD member name         
*/

static T_LEDIT_RSLT ledit_assign_name (T_LEDIT_INTERN *leditInt, int len)
{
  if(ledit_get_memory (&(leditInt->cmdm), len+1, 1) EQ LEDIT_FAIL)
  {
    leditInt->err.code = LEDIT_ERR_NoMemory;
    leditInt->err.msg  = ledit_err[LEDIT_ERR_NoMemory].msg;
    return LEDIT_FAIL;
  }
  memcpy (leditInt->cmdm->cmd->name, leditInt->execBuffer+leditInt->execBufIter, len);
  *(leditInt->cmdm->cmd->name+len) = '\0'; /* terminate name */
  leditInt->execBufIter += len; /* update iterator in execBuffer */
  return LEDIT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_assign_param |
+--------------------------------------------------------------------+

  PURPOSE :  - extract the param(s) of an AT command from command line
               and put it into T_LEDIT_ATCMD member param
             - can be empty         
*/

static T_LEDIT_RSLT ledit_assign_param (T_LEDIT_INTERN *leditInt, T_LEDIT_PARAM_TYPE pType)
{
  int len = 0;
#ifdef _SIMULATION_  
  TRACE_FUNCTION("ledit_assign_param()");
#endif
  if (pType EQ LEDIT_BASIC_PARAM)
  {
    len = ledit_get_para_len (leditInt, SEARCH_BAT_LEN);
  }
  else 
  if (pType EQ LEDIT_DIAL_PARAM) /* dial parameter, we need the possible ';' */
  {
    len = ledit_get_para_len (leditInt, SEARCH_DIAL_LEN);
    /*
      * If there were any 'g' or 'i' characters in a dial string that have
      * been turned into 'G' or 'I' characters on conversion to upper case
      * by the ledit_collect function...
      * Turn them back into the lower case characters so that they affect
      * CUG and CLIR in the way they were intended to.
      */
    ledit_restore_dial_string ( &leditInt->execBuffer[leditInt->execBufIter],
                                len,
                                leditInt->origBuffer,
                                strlen(leditInt->origBuffer));
  }
  else /* extended AT cmd params */
  {
    len = ledit_get_para_len (leditInt, SEARCH_EXT_LEN);
  }
  if(ledit_get_memory (&(leditInt->cmdm), len+1, 0) EQ LEDIT_FAIL)
  {
    leditInt->err.code = LEDIT_ERR_NoMemory;
    leditInt->err.msg  = ledit_err[LEDIT_ERR_NoMemory].msg;
    return LEDIT_FAIL;
  }
  memcpy (leditInt->cmdm->cmd->params, leditInt->execBuffer+leditInt->execBufIter, len);
  *(leditInt->cmdm->cmd->params+len) = '\0';
  
  /* 
   * update iterator in execBuffer 
   * for extended cmds skip the ';'
   */
  if (pType EQ LEDIT_EXT_PARAM)
  {
    if (leditInt->execBuffer[leditInt->execBufIter+len] EQ '\0')
    {
      leditInt->execBufIter += len;
    }
    else
    {
      leditInt->execBufIter += len+1; /* skip the ';' */
    }
  }
  else
  {
    leditInt->execBufIter += len;
  }

  return LEDIT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  : LINE_EDIT_LIB       |
| STATE   : code                       ROUTINE : ledit_extended_type |
+--------------------------------------------------------------------+

  PURPOSE :  -         
*/
static T_LEDIT_ATCMD_SUBTYPE ledit_extended_type (T_LEDIT_INTERN *leditInt)
{
#ifdef _SIMULATION_
 TRACE_FUNCTION("ledit_extended_type()");
#endif
  switch (ledit_lookAhead(leditInt->execBuffer[leditInt->execBufIter]))
  {
    case LEDIT_CHAR_SET:
    {
      ++(leditInt->execBufIter);
      if (ledit_lookAhead(leditInt->execBuffer[leditInt->execBufIter]) EQ LEDIT_CHAR_READ)
      {
        ++(leditInt->execBufIter);
        /*Setting flag as 1*/
        leditInt->cmdm->cmd->type.append_Flag = TRUE;
        /* bug 11920*/
        return LEDIT_ATCMD_TEST;
      }
      /*Setting flag as 1*/
      leditInt->cmdm->cmd->type.append_Flag = TRUE;
      /* bug 11920*/
      return LEDIT_ATCMD_SET;
    }
    case LEDIT_CHAR_READ:
    {
      ++(leditInt->execBufIter);
       /*Setting flag as 1*/
    leditInt->cmdm->cmd->type.append_Flag = TRUE;
      /* bug 11920*/
      return LEDIT_ATCMD_QUERY;
    }
    default:
    {
      /* return LEDIT_ATCMD_NIL; */
      /*Setting flag as 0. 
      These types need not be appended with the characters later on while reconstructing
      command in leedit_fwrd_extension*/
      leditInt->cmdm->cmd->type.append_Flag = FALSE;
      /* bug 11920*/
         
      return LEDIT_ATCMD_SET;  /* if after name no parameters e.g.: AT+CMGL; */
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_extended_cmd |
+--------------------------------------------------------------------+

  PURPOSE :  - split cmd line into single canonical commands          
*/
static T_LEDIT_RSLT ledit_extended_cmd  (T_LEDIT_INTERN *leditInt)
{
  int len = 0;
#ifdef _SIMULATION_   
  TRACE_FUNCTION ("ledit_extended_cmd()");
#endif
  /*
   * get name lenght
   */
  len = ledit_get_name_len (leditInt, LEDIT_EXT_NAME);
  if(len)
  {
    if (ledit_assign_name(leditInt, len) NEQ LEDIT_CMPL)
    {
      return LEDIT_FAIL;
    }
    leditInt->cmdm->cmd->type.mType = LEDIT_ATCMD_EXTENDED;
    leditInt->cmdm->cmd->type.sType = ledit_extended_type(leditInt);
    if (ledit_assign_param (leditInt, LEDIT_EXT_PARAM) NEQ LEDIT_CMPL)
    {
      return LEDIT_FAIL;
    }
  }
  else
  {
    leditInt->err.code = LEDIT_ERR_NoValidCommand;
    leditInt->err.msg  = ledit_err[LEDIT_ERR_NoValidCommand].msg;
    return LEDIT_FAIL;
  }
  return LEDIT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : LINE_EDIT_LIB        |
| STATE   : code                      ROUTINE : ledit_s_cmd          |
+--------------------------------------------------------------------+

  PURPOSE :  -  special basic AT command ATS 
                with own syntax similiar to extended AT commands
                e.g.:  ATS0?    <-- query register S0
                       ATS0=123 <-- set new value for register S0
                but the parameter is basic cmd like  
*/
static T_LEDIT_RSLT ledit_s_cmd (T_LEDIT_INTERN *leditInt)
{
  int len = 0;
#ifdef _SIMULATION_
  TRACE_FUNCTION ("ledit_s_cmd()");
#endif
  /*
   * get name length, support for up to ATS99, but not ATS100 !
   */
  len = ledit_get_name_len (leditInt, LEDIT_S_NAME);
  if(len)
  {
    if (ledit_assign_name(leditInt, len) NEQ LEDIT_CMPL)
    {
      return LEDIT_FAIL;
    }
    leditInt->cmdm->cmd->type.mType = LEDIT_ATCMD_S;
    leditInt->cmdm->cmd->type.sType = ledit_extended_type(leditInt); /* after Sx must be [?|=] */
    if (ledit_assign_param (leditInt, LEDIT_BASIC_PARAM) NEQ LEDIT_CMPL)
    {
      return LEDIT_FAIL;
    }
  }
  else
  {
    leditInt->err.code = LEDIT_ERR_NoValidCommand;
    leditInt->err.msg  = ledit_err[LEDIT_ERR_NoValidCommand].msg;
    return LEDIT_FAIL;
  }
  return LEDIT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : LINE_EDIT_LIB        |
| STATE   : code                      ROUTINE : ledit_dial           |
+--------------------------------------------------------------------+

  PURPOSE :  - ATD12345; , but also ATD(030)12345; or ATD!; ATD@;  ...
             - semantic of ATD paramter has to be checked by ATI    
*/
static T_LEDIT_RSLT ledit_dial (T_LEDIT_INTERN *leditInt)
{
#ifdef _SIMULATION_
  TRACE_FUNCTION ("ledit_dial()");
#endif
  leditInt->cmdm->cmd->type.mType = LEDIT_ATCMD_DIAL;
  leditInt->cmdm->cmd->type.sType = LEDIT_ATCMD_SET;
    /*Setting flag as 0*/
 leditInt->cmdm->cmd->type.append_Flag = FALSE;
    /*Issue 11920*/
  if (ledit_assign_name(leditInt, 1) NEQ LEDIT_CMPL)
  {
    return LEDIT_FAIL;
  }
  /*
   * dial paramter ends with ';' -> LEDIT_EXT_PARAM
   */
  if (ledit_assign_param (leditInt, LEDIT_DIAL_PARAM) NEQ LEDIT_CMPL)
  {
    return LEDIT_FAIL;
  }
  if (leditInt->execBuffer[leditInt->execBufIter])
  {
    return LEDIT_FAIL; /* dial command is not the last on cmd line */
  }
  return LEDIT_CMPL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : LINE_EDIT_LIB        |
| STATE   : code                      ROUTINE : ledit_and_basic_cmd  |
+--------------------------------------------------------------------+

  PURPOSE :  -         
*/
static T_LEDIT_RSLT ledit_and_basic_cmd (T_LEDIT_INTERN *leditInt)
{
  USHORT rv = ANDCMD_NO_SUPP;
#ifdef _SIMULATION_
  TRACE_FUNCTION ("ledit_and_basic_cmd()");
#endif
  ++(leditInt->execBufIter); /* skip the & */

  rv = ledit_check_valid_and_cmd (leditInt->execBuffer[leditInt->execBufIter]);
  if(rv EQ ANDCMD_SUPPORT)
  {
    leditInt->cmdm->cmd->type.mType = LEDIT_ATCMD_AND;
    leditInt->cmdm->cmd->type.sType = LEDIT_ATCMD_QUERY;
    
    /*Setting flag as 0*/
    leditInt->cmdm->cmd->type.append_Flag = FALSE;
    /* bug 11920*/
    
    if (ledit_assign_name(leditInt, 1) NEQ LEDIT_CMPL)
    {
      return LEDIT_FAIL;
    }
    if (ledit_assign_param (leditInt, LEDIT_BASIC_PARAM) NEQ LEDIT_CMPL)
    {
      return LEDIT_FAIL;
    }
  }
  else
  {
    leditInt->err.code = LEDIT_ERR_NoValidCommand;
    leditInt->err.msg  = ledit_err[LEDIT_ERR_NoValidCommand].msg;
    return LEDIT_FAIL;
  }
  return LEDIT_CMPL;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_basic_cmd    |
+--------------------------------------------------------------------+

  PURPOSE :  - check for basic command
             - check for dial command        
*/
static T_LEDIT_RSLT ledit_basic_cmd (T_LEDIT_INTERN *leditInt)
{
  USHORT rv = BATCMD_NO_SUPP;
#ifdef _SIMULATION_
  TRACE_FUNCTION ("ledit_basic_cmd()");
#endif
  rv = ledit_check_valid_basic_cmd (leditInt->execBuffer[leditInt->execBufIter]);
  if (rv EQ BATCMD_SUPPORT)
  {
    leditInt->cmdm->cmd->type.mType = LEDIT_ATCMD_BASIC;
    leditInt->cmdm->cmd->type.sType = LEDIT_ATCMD_SET;
    
    /* Setting flag as 0*/
     leditInt->cmdm->cmd->type.append_Flag = FALSE;
    /* Issue 11920*/
    
    if (ledit_assign_name(leditInt, 1) NEQ LEDIT_CMPL)
    {
      return LEDIT_FAIL;
    }
    if (ledit_assign_param (leditInt, LEDIT_BASIC_PARAM) NEQ LEDIT_CMPL)
    {
      return LEDIT_FAIL;
    }
  }
  else if (rv EQ BATCMD_DIAL)
  {
    if (ledit_dial(leditInt) NEQ LEDIT_CMPL)
    {
      return LEDIT_FAIL;
    }
  }
  else if (rv EQ BATCMD_S_PARAM)
  {
    if (ledit_s_cmd(leditInt) NEQ LEDIT_CMPL)
    {
      return LEDIT_FAIL;
    }
  }
  else
  {
    leditInt->err.code = LEDIT_ERR_NoValidCommand;
    leditInt->err.msg  = ledit_err[LEDIT_ERR_NoValidCommand].msg;
    return LEDIT_FAIL;
  }
  return LEDIT_CMPL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_split        |
+--------------------------------------------------------------------+

  PURPOSE :  - split cmd line into single canonical commands          
*/
T_LEDIT_RSLT ledit_split (T_LEDIT_INTERN *leditInt)
{
 
  char c = leditInt->execBuffer[leditInt->execBufIter];
  TRACE_FUNCTION("ledit_split");

  /*
   * check first character of a command: has to be valid basic command letter, "&" or 
   * any extension sign "+", "%" , "$" or what ever else
   */
  if(c)
  {
    if (ledit_get_cmd_mem (leditInt) EQ LEDIT_FAIL)
    { 
      return LEDIT_FAIL;
    }
    if (isalpha(c))
    {
      if (ledit_basic_cmd (leditInt) EQ  LEDIT_FAIL)
      {
        return  LEDIT_FAIL;
      }
    }
    else if (c EQ '&')
    {
      if (ledit_and_basic_cmd (leditInt) EQ  LEDIT_FAIL)
      {
        return  LEDIT_FAIL;
      }
    }
    else
    {
      if (ledit_extended_cmd (leditInt) EQ  LEDIT_FAIL)
      {
        return  LEDIT_FAIL;
      }
    }

    /* here we should peek into the next character to see if command is valid */
    switch (ledit_lookAhead(leditInt->execBuffer[leditInt->execBufIter]))
    {
      /* legal characters */
      case LEDIT_CHAR_PERCENT:    /* '%' */
      case LEDIT_CHAR_AND:        /* '&' */
      case LEDIT_CHAR_PLUS:       /* '+' */
      case LEDIT_CHAR_CHAR:       /* 'A' symbolic for A..Z*/
      case LEDIT_CHAR_EXT:         /*The rest of the characters*/
        leditInt->cmdIndex++;     /* More commands will follow */
      /*lint -fallthrough */
      case LEDIT_CHAR_TERMINATED: /* '\0' */
        return LEDIT_CMPL;

      /* illegal characters */
#if 0
      case LEDIT_CHAR_COMMA:      /* ',' */
      case LEDIT_CHAR_NUMBER:     /* '0' symbolic for 0..9 */
      case LEDIT_CHAR_SEPARATOR:  /* ';' */
      case LEDIT_CHAR_SET:        /* '=' */
      case LEDIT_CHAR_READ:       /* '?' */
      case LEDIT_CHAR_INVALID:
#endif
      default:
        {
          return LEDIT_FAIL;
       }
    }
  }
  else
  {
    TRACE_ERROR ("ledit_split: unexp. EOF execBuffer");
  }
  return LEDIT_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_restore_dial_string        |
+--------------------------------------------------------------------+

  PURPOSE :  -          
      If there were any 'g' or 'i' characters in a dial string that have
      been turned into 'G' or 'I' characters on conversion to upper case
      by the ledit_collect function...
      Turn them back into the lower case characters so that they affect
      CUG and CLIR in the way they were intended to.
*/
static T_LEDIT_RSLT ledit_restore_dial_string (char *procDialString,
                                               int   procDialStringLen,
                                               char *origCommandLine,
                                               int   origCommandLineLen)
{
    int procDialStringIndex = 0;        /* Loop index to procDialString array */
    int origCommandLineIndex = 0;   /* Loop index to origCommandLine array */
    char *origDialString;               /* Pointer to original dial string array in origCommandLine */
    int origDialStringIndex;            /* Loop index to origDialString array */

    TRACE_FUNCTION("ledit_restore_dial_string");

    if ((procDialString == NULL) OR
        (procDialStringLen == 0) OR
        (origCommandLine == NULL) OR
        (origCommandLineLen == 0))
    {
        TRACE_ERROR("FAILURE: Invalid parameter");
        if (procDialString == NULL)
            TRACE_EVENT_P1("procDialString = %p", procDialString);
        if (procDialStringLen == 0)
            TRACE_EVENT_P1("procDialStringLen = %d", procDialStringLen);
        if (origCommandLine == NULL)
            TRACE_EVENT_P1("origCommandLine = %p", origCommandLine);
        if (origCommandLineLen == 0)
            TRACE_EVENT_P1("origCommandLineLen = %d", origCommandLineLen);
        return LEDIT_FAIL;
    }
    
    /* Look for a 'G' or 'I' character in the processed dial string */
    for (procDialStringIndex = 0;
        procDialStringIndex < procDialStringLen;
        procDialStringIndex++)
    {
        if ((procDialString[procDialStringIndex] EQ 'G') OR
         (procDialString[procDialStringIndex] EQ 'I'))
        {
            /* 'G' or 'I' found, so we have something to do */
            break;
        }
    }

    if (procDialStringIndex EQ procDialStringLen)
    {
        /* No 'G' or 'I' found, so we have nothing to do */
        TRACE_EVENT("SUCCESS: Nothing to do");
        return LEDIT_CMPL;
    }

    /*
     * Look for the start of the original dial string in the original command
     * line by looking for 'd' or 'D' characters
     */
    for (origCommandLineIndex = 0;
        origCommandLineIndex < origCommandLineLen;
        origCommandLineIndex++)
    {
        if ((origCommandLine[origCommandLineIndex] EQ 'd') OR
          (origCommandLine[origCommandLineIndex] EQ 'D'))
        {
            /* Found the possible start of the original dial string */
            origDialString = &origCommandLine[origCommandLineIndex+1];
            origDialStringIndex = 0;

            for (procDialStringIndex = 0;
                procDialStringIndex < procDialStringLen;
                procDialStringIndex++)
            {
                /* Ignore any spaces in the original dial string */
                while (isspace(origDialString[origDialStringIndex]))
                {
                    origDialStringIndex++;
                }
                
                /* Do we have a matching character? */
                if (origDialString[origDialStringIndex] NEQ procDialString[procDialStringIndex])
                {
                    /* No: Is the mismatch just on the alphabetic case? */
                    if (isalpha(origDialString[origDialStringIndex]) AND
                        isalpha(procDialString[procDialStringIndex]) AND
                        (toupper(origDialString[origDialStringIndex]) EQ
                        toupper(procDialString[procDialStringIndex])))
                    {  
                        /* Yes, so ignore mismatch */;
                    }
                    else
                    {  /* No, so not the original dial string */
                        break;
                    }
                }
                origDialStringIndex++;
            }
                
            /* Have we found the original dial string? */
            if (procDialStringIndex EQ procDialStringLen)
            {
                /* Yes, so copy 'G'/'g' and 'I'/'i' characters from the original dial string */
                origDialStringIndex = 0;
                for (procDialStringIndex = 0;
                    procDialStringIndex < procDialStringLen;
                    procDialStringIndex++)
                {
                    /* Ignore any spaces in the original dial string */
                    while (isspace(origDialString[origDialStringIndex]))
                    {
                        origDialStringIndex++;
                    }
                    
                    if ((procDialString[procDialStringIndex] EQ 'G') OR
                     (procDialString[procDialStringIndex] EQ 'I'))
                    {   /* Copy the 'G'/'g' or 'I'/'i' character from the original dial string */
                        procDialString[procDialStringIndex] = origDialString[origDialStringIndex];
                    }
                    origDialStringIndex++;
                }

                TRACE_EVENT("SUCCESS");
                return LEDIT_CMPL;
            }
        }
    }
    /* If we get this far then we haven't found the original dial string */
    TRACE_ERROR("FAILURE: Could not find original dial string");
    return LEDIT_FAIL;
}


