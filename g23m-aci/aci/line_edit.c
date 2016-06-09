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


GLOBAL char g_ledit_echoBuf[MAX_CMD_LEN+1];

static T_LEDIT_SRC_MAINTAIN *rootSrc = NULL;

/* state machine functions */
static T_LEDIT_RSLT ledit_idle       (T_LEDIT_INTERN *leditInt);
static T_LEDIT_RSLT ledit_prefix_a   (T_LEDIT_INTERN *leditInt);
static T_LEDIT_RSLT ledit_prefix_t   (T_LEDIT_INTERN *leditInt);
static T_LEDIT_RSLT ledit_repeat     (T_LEDIT_INTERN *leditInt);
static T_LEDIT_RSLT ledit_collect    (T_LEDIT_INTERN *leditInt);
static T_LEDIT_RSLT ledit_run_cmd    (T_LEDIT_INTERN *leditInt);
/* help functions */
static T_LEDIT_SRC_MAINTAIN *ledit_getNewSrcMaintain (T_LEDIT_SRC_MAINTAIN *lesm);
static T_LEDIT_SRC_MAINTAIN *ledit_getSrcM           (UBYTE src_id, const UBYTE *chars, USHORT len);
static T_LEDIT_SRC_MAINTAIN *ledit_getSrc            (UBYTE src_id);

static T_LEDIT_RSLT ledit_remove_src   (T_LEDIT_SRC_MAINTAIN *lesm);
static T_LEDIT_RSLT ledit_check_param  (const UBYTE *chars, USHORT len);
static T_LEDIT_RSLT ledit_clear_lineBuf(T_LEDIT_INTERN *leditInt);
static T_LEDIT_RSLT ledit_clear_all    (T_LEDIT_INTERN *leditInt);
static T_LEDIT_RSLT ledit_backup       (T_LEDIT_INTERN *leditInt);
static T_LEDIT_RSLT ledit_echo         (T_LEDIT_INTERN *leditInt);
static void         ledit_echo_clear   (void);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : LINE_EDIT_LIB        |
| STATE   : code                      ROUTINE : ledit_set_config     |
+--------------------------------------------------------------------+

  PURPOSE : - configure CR, LF, BS chars  --> similiar to S3, S4, S5
            - configure echo --> similiar to ATE echo cmds in command mode
            - configure echo --> similiar to ATF echo cmds in data mode
*/
T_LEDIT_RSLT ledit_set_config (UBYTE src_id, T_LEDIT line)
{
  T_LEDIT_SRC_MAINTAIN *lineSrcM = NULL;
  UBYTE                *chars    = NULL;
  USHORT                len      = 0;

  /*
   * get the maintenance for this AT source or create a new one or die
   */
  if ((lineSrcM = ledit_getSrcM (src_id, chars, len)) EQ NULL)
  {
    return (LEDIT_FAIL);
  }
  /*
   * some checks of T_LEDIT line
   */  
  if (line.atE NEQ 0xFF)
  {
    if ((line.atE EQ TRUE) OR (line.atE EQ FALSE))
    {
      lineSrcM->leditInt->lineHabit.atE = line.atE;
    }
    else
    {
      return (LEDIT_FAIL);
    }
  }
  
  if (line.S3 NEQ 0xFF)
  {
    if (line.S3 < 128)
    {
      lineSrcM->leditInt->lineHabit.S3 = line.S3;
    }
    else
    {
      return (LEDIT_FAIL);    
    }
  }
  
  if (line.S4 NEQ 0xFF)
  {
    if (line.S4 < 128)
    {
      lineSrcM->leditInt->lineHabit.S4 = line.S4;
    }
    else
    {
      return (LEDIT_FAIL);    
    }
  }
  
  if (line.S5 NEQ 0xFF) 
  {
    if (line.S5 < 128)
    {
      lineSrcM->leditInt->lineHabit.S5 = line.S5;
    }
    else
    {
      return (LEDIT_FAIL);    
    }
  }
  
  if (line.smsEnd NEQ 0xFF)
  {
    if (line.smsEnd < 128)
    {
      lineSrcM->leditInt->lineHabit.smsEnd = line.smsEnd;
    }
    else
    {
      return (LEDIT_FAIL);    
    }
  }
  
  return (LEDIT_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_ctrl         |
+--------------------------------------------------------------------+

  PURPOSE : - inform and control line edit
            - in err will be error code and error message of line edit
*/

T_LEDIT_RSLT ledit_ctrl (UBYTE src_id, T_LEDIT_CTRL ctrl, T_LEDIT_ERR **err)
{
  T_LEDIT_SRC_MAINTAIN *lineSrcM = NULL;

  TRACE_FUNCTION ("ledit_ctrl()");

  /*
   * get the maintenance for this AT source or create a new one or die
   */
  if ((lineSrcM = ledit_getSrcM (src_id, NULL, 0)) EQ NULL)
  {
    return (LEDIT_FAIL);
  }

  switch (ctrl)
  {
    case LEDIT_CTRL_PROMPT:
    {
      ledit_echo_clear();
      strcpy (g_ledit_echoBuf,"> ");
      TRACE_EVENT("ledit_ctrl(): prompt for SMS editing");
      return (LEDIT_CMPL);
    }
    case LEDIT_CTRL_CRLF_PROMPT:
    {
      ledit_echo_clear();
      /* (IRA 13, 10, 62, 32) see 07.05 */
      g_ledit_echoBuf[0] = 0x0D; /* lineSrcM->leditInt->lineHabit.S3; */ /* CR  */
      g_ledit_echoBuf[1] = 0x0A; /* lineSrcM->leditInt->lineHabit.S4; */ /* LF  */
      g_ledit_echoBuf[2] = 0x3E;  /* '>' */
      g_ledit_echoBuf[3] = 0x20;  /* ' ' */
      g_ledit_echoBuf[4] = '\0';
      TRACE_EVENT("ledit_ctrl(): default settings for line behaviour");
      return (LEDIT_CMPL);
    }
    case LEDIT_CTRL_CMPL:
    {
      TRACE_EVENT("ledit_ctrl(): reset ledit internals");
      ledit_clear_all(lineSrcM->leditInt);
      return (LEDIT_CMPL);
    }
    case LEDIT_CTRL_REMOVE_SRC:
    {
      ledit_remove_src(lineSrcM);
      return (LEDIT_CMPL);
    }
    case LEDIT_CTRL_ERROR:
    {
      if (err)
      {
        *err  = &lineSrcM->leditInt->err;
      }
      return (LEDIT_CMPL);
    }
    case LEDIT_CTRL_MORE_CMDS: /* query whether there are more than one cmd */
    {
      if (lineSrcM->leditInt->cmdGetIter < lineSrcM->leditInt->cmdIndex)   /* check if cmdline already fully parsed */
      {
        TRACE_EVENT("ledit_ctrl(): are there more commands ? Yes !");
        return (LEDIT_CMPL);
      }
      else
      {
        TRACE_EVENT("ledit_ctrl(): are there more commands ? No !");
        return (LEDIT_FAIL);
      }
    }
    default:
    {
      if (*err)
      {
        (*err)->code = LEDIT_ERR_Unknown;
        (*err)->msg  = ledit_err[LEDIT_ERR_Unknown].msg;
      }
      return (LEDIT_FAIL);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_get_first    |
+--------------------------------------------------------------------+

  PURPOSE : -
*/

T_LEDIT_RSLT ledit_get_first (UBYTE src_id, T_LEDIT_ATCMD **cmd)
{
  T_LEDIT_SRC_MAINTAIN *lesm = ledit_getSrc(src_id);

  TRACE_FUNCTION ("ledit_get_first()");

  *cmd = NULL; /* init with no command available */

  if (lesm EQ NULL)
  {
    return (LEDIT_FAIL);
  }

  if (lesm->leditInt->cmdm EQ NULL)
  {
    return (LEDIT_FAIL);
  }

  *cmd = lesm->leditInt->cmdm->cmd;
  return (LEDIT_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_get_next     |
+--------------------------------------------------------------------+

  PURPOSE : -
*/

T_LEDIT_RSLT ledit_get_next (UBYTE src_id, T_LEDIT_ATCMD **cmd)
{
  T_LEDIT_ATCMD_M **curCmd = NULL;

  T_LEDIT_SRC_MAINTAIN *lesm = ledit_getSrc(src_id);

  TRACE_FUNCTION ("ledit_get_next()");

  *cmd = NULL; /* init with no command available */

  if (lesm EQ NULL)
  {
    return (LEDIT_FAIL);
  }

  ++(lesm->leditInt->cmdGetIter); /* 1 .. n */

  ledit_free_cmd(lesm->leditInt);       /* clear all (=previous) commands */
  lesm->leditInt->cmdm = NULL;
  if (ledit_split(lesm->leditInt) EQ LEDIT_CMPL)
  {
    curCmd = &(lesm->leditInt->cmdm);
    if (*curCmd)
    {
      *cmd = (*curCmd)->cmd;
      return (LEDIT_CMPL);
    }
  }

  return (LEDIT_FAIL); /* no further command available */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_get_current  |
+--------------------------------------------------------------------+

  PURPOSE : - for text mode relevant functions, which have to be called twice
              1. with the destination adress
              2. with the text
*/

T_LEDIT_RSLT ledit_get_current (UBYTE src_id, T_LEDIT_ATCMD **cmd)
{
  T_LEDIT_SRC_MAINTAIN *lesm = ledit_getSrc(src_id);

  TRACE_FUNCTION ("ledit_get_current()");

  *cmd = NULL; /* init with no command available */

  if (lesm EQ NULL)
  {
    return (LEDIT_FAIL);
  }

  if (lesm->leditInt->cmdm EQ NULL)
  {
    return (LEDIT_FAIL);
  }

  *cmd = lesm->leditInt->cmdm->cmd;
  return (LEDIT_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_text         |
+--------------------------------------------------------------------+

  PURPOSE : -
            - switch from cmd mode to text mode
*/

T_LEDIT_RSLT ledit_text (UBYTE src_id, const UBYTE *chars, USHORT len)
{
  T_LEDIT_SRC_MAINTAIN *lineSrcM = NULL;
  char c = '\0';

  static unsigned int beginline;  /* first character position in current line */

  TRACE_FUNCTION ("ledit_text()");

  /*
   * get the maintenance for this AT source
   */
  if ((lineSrcM = ledit_getSrcM (src_id, chars, len)) EQ NULL)
  {
    return (LEDIT_FAIL);
  }

  ledit_echo_clear ();
  /*
   * we use the cmdBuffer to store the text, so clean it up at first calling of this function
   * in case of text sent in several chunks (this function is called again), concatenate them
   * by leaving the copy iterator and the cmdBuffer untouched
   * txtChunk is a binary flag
   */
  if (lineSrcM->leditInt->txtChunk EQ FALSE)
  {
    ledit_clear_lineBuf (lineSrcM->leditInt);
    lineSrcM->leditInt->txtChunk = TRUE;
    beginline=0;
  }

  while (lineSrcM->leditInt->srcBufIter < len)
  {
    c = lineSrcM->leditInt->srcBuffer[lineSrcM->leditInt->srcBufIter];

    if (lineSrcM->leditInt->copyIter EQ MAX_CMD_LEN)  /* FIXME: Should be depending to csca arround 160 */
    {
      if (c EQ 0x1b)
        ;
      else if (c EQ lineSrcM->leditInt->lineHabit.S5)
        ;
      else if (c EQ lineSrcM->leditInt->lineHabit.smsEnd)
        ;
      else
        return (LEDIT_IGNORE);
    }
    if (c EQ 0x1b)  /* ESC */
    {
      if (lineSrcM->leditInt->smsBuffer)
      {
        ACI_MFREE (lineSrcM->leditInt->smsBuffer); /* at first delete possible last text */
      }
      lineSrcM->leditInt->smsBuffer = NULL;
      lineSrcM->leditInt->txtChunk = FALSE;
      return (LEDIT_ESC); /* don't generate any error message */
    }
    else if (c EQ lineSrcM->leditInt->lineHabit.smsEnd) /* CTRL-Z, for all non V25.ter conform rockers */
    {
      lineSrcM->leditInt->origBuffer[lineSrcM->leditInt->origBufIter++] = '\0';  /* terminate text */
      lineSrcM->leditInt->lineBuffer[lineSrcM->leditInt->copyIter++] = '\0'; /* terminate text */
      
      if (lineSrcM->leditInt->smsBuffer)
      {
        ACI_MFREE (lineSrcM->leditInt->smsBuffer); /* at first delete possible last text */
      }
      ACI_MALLOC(lineSrcM->leditInt->smsBuffer, ((lineSrcM->leditInt->copyIter+1) * sizeof(char)));
      if (lineSrcM->leditInt->smsBuffer EQ NULL)
      {
        return (LEDIT_FAIL);
      }
      strcpy (lineSrcM->leditInt->smsBuffer, lineSrcM->leditInt->lineBuffer);
      lineSrcM->leditInt->txtChunk = FALSE;
      return (LEDIT_CMPL);
    }
    else if (c EQ lineSrcM->leditInt->lineHabit.S3)
    {
      lineSrcM->leditInt->origBuffer[lineSrcM->leditInt->origBufIter++] = 0x0a; /* add newline into SMS */
      lineSrcM->leditInt->lineBuffer[lineSrcM->leditInt->copyIter++] = 0x0a; /* add newline into SMS */
      /*
       * split text, so emit promp "> " again
       */
      g_ledit_echoBuf[lineSrcM->leditInt->srcBufIter]   = 0x0d; /* lineSrcM->leditInt->lineHabit.S3; */
      g_ledit_echoBuf[lineSrcM->leditInt->srcBufIter+1] = 0x0a; /* lineSrcM->leditInt->lineHabit.S4; */
      g_ledit_echoBuf[lineSrcM->leditInt->srcBufIter+2] = 0x3e; /* '>' */
      g_ledit_echoBuf[lineSrcM->leditInt->srcBufIter+3] = 0x20; /* ' ' */
      beginline = lineSrcM->leditInt->copyIter;                 /* reset line */
    }
    else if (c EQ lineSrcM->leditInt->lineHabit.S4)
    {
      ; /* ignore Linefeeds */
    }
    else if (c EQ lineSrcM->leditInt->lineHabit.S5)    /* backspace */
    {
      /*
       * delete last character in cmd buffer by setting iterator one step back
       */
      if (lineSrcM->leditInt->copyIter > beginline)
      {
        --(lineSrcM->leditInt->origBufIter);
        --(lineSrcM->leditInt->copyIter);
        g_ledit_echoBuf[lineSrcM->leditInt->srcBufIter] = c;
        return (LEDIT_COLLECT);
      }
      else
        return (LEDIT_IGNORE);
    }
    else
    {
      lineSrcM->leditInt->origBuffer[lineSrcM->leditInt->origBufIter++] = c; /* copy character to cmd buffer */
      lineSrcM->leditInt->lineBuffer[lineSrcM->leditInt->copyIter++] = c; /* copy character to cmd buffer */
      g_ledit_echoBuf[lineSrcM->leditInt->srcBufIter] = c;
    }
    ++(lineSrcM->leditInt->srcBufIter);
  }
  return (LEDIT_COLLECT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_get_text     |
+--------------------------------------------------------------------+

  PURPOSE : -
*/

T_LEDIT_RSLT ledit_get_text (UBYTE src_id, char **txt)
{
  T_LEDIT_SRC_MAINTAIN *lineSrcM = NULL;
  /*
   * get the maintenance for this AT source
   */
  if ((lineSrcM = ledit_getSrcM (src_id, NULL, 0)) EQ NULL)
  {
    return (LEDIT_FAIL);
  }

  if (*txt)
  {
    ACI_MFREE (*txt); /* free the cmd parameter part */
  }

  ACI_MALLOC (*txt, (strlen(lineSrcM->leditInt->smsBuffer)+1) * sizeof(char));
  if (*txt EQ NULL)
  {
    return (LEDIT_FAIL);
  }
  /*
   * the complementary ACI_MFREE takes place in ledit_ctrl (src_params->src_id,LEDIT_CTRL_CMPL, NULL);
   * which has to called for success or fail of a AT cmd line. grep in ati_cmd.c and ati_ret.c
   */
  if (lineSrcM->leditInt->smsBuffer)
  {
    strcpy (*txt, lineSrcM->leditInt->smsBuffer); /* the parameter of the AT cmd is now the SMS text */
  }
  else
  {
    **txt = '\0'; /* in case of 0 byte text */
  }
  return (LEDIT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : LINE_EDIT_LIB         |
| STATE   : code                     ROUTINE : ledit_backup          |
+--------------------------------------------------------------------+

  PURPOSE : - for possible next A/ back up the current cmd line
  NOW: we shift the lineBuffer to execBuffer for execution!!!
*/

static T_LEDIT_RSLT ledit_backup (T_LEDIT_INTERN *leditInt)
{
  if (leditInt->execBuffer)
  {
    ACI_MFREE (leditInt->execBuffer);
  }

  leditInt->execBuffer = leditInt->lineBuffer;
  leditInt->lineBuffer = NULL;
  return (LEDIT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : LINE_EDIT_LIB         |
| STATE   : code                     ROUTINE : ledit_echo_clear      |
+--------------------------------------------------------------------+

  PURPOSE : -
*/
static void ledit_echo_clear (void)
{
  memset(g_ledit_echoBuf, '\0', sizeof(g_ledit_echoBuf));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : LINE_EDIT_LIB         |
| STATE   : code                     ROUTINE : ledit_echo            |
+--------------------------------------------------------------------+

  PURPOSE : - for echoing put the current chars into the g_ledit_echoBuffer
              but attention:
              all chars before the first [aA] are ignored
              the same between [aA] and [tT]
  Example :  "murx blah fiT + cFun = 1" will be echoed as
                     -    ------------
                        "aT + cFun = 1"
*/
static T_LEDIT_RSLT ledit_echo (T_LEDIT_INTERN *leditInt)
{
  int i = 0;
  while (g_ledit_echoBuf[i]) /* skip possible [aAtT] */
  {
    ++i;
  }

  while (leditInt->srcBufIterE < leditInt->len)
  {
    g_ledit_echoBuf[i] = leditInt->srcBuffer[leditInt->srcBufIterE];
    if (leditInt->srcBuffer[leditInt->srcBufIterE] EQ leditInt->lineHabit.S3)
    {
      ++i;
      break;
    }
    ++(leditInt->srcBufIterE);
    ++i;
  }

  g_ledit_echoBuf[i] = '\0';

  leditInt->srcBufIterE = 0;

  return (LEDIT_CMPL);
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : LINE_EDIT_LIB           |
| STATE   : code                   ROUTINE : ledit_remove_src        |
+--------------------------------------------------------------------+

  PURPOSE :
*/
static T_LEDIT_RSLT ledit_remove_src (T_LEDIT_SRC_MAINTAIN *lesm)
{
  T_LEDIT_SRC_MAINTAIN *ledSrcM = rootSrc;
  T_LEDIT_SRC_MAINTAIN *tmp     = rootSrc;

  if (rootSrc EQ lesm)
  {
    tmp = rootSrc->next;
    ledit_clear_all (rootSrc->leditInt);
    ACI_MFREE (rootSrc->leditInt);
    rootSrc->leditInt = NULL;
    rootSrc = tmp;
    return (LEDIT_CMPL);
  }
  else
  {
    while(ledSrcM)
    {
      if (ledSrcM EQ lesm)
      {
        tmp->next = ledSrcM->next;
        ledit_clear_all (ledSrcM->leditInt);
        ACI_MFREE (ledSrcM->leditInt);
        ACI_MFREE (ledSrcM);
        ledSrcM = NULL;
        return (LEDIT_CMPL);
      }
      tmp     = ledSrcM;
      ledSrcM = ledSrcM->next;
    }
    return (LEDIT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : LINE_EDIT_LIB           |
| STATE   : code                   ROUTINE : ledit_getNewSrcMaintain |
+--------------------------------------------------------------------+

  PURPOSE :
*/
static T_LEDIT_SRC_MAINTAIN *ledit_getNewSrcMaintain (T_LEDIT_SRC_MAINTAIN *lesm)
{
  static
  T_LEDIT ledit_initial  = {13, 10,  8, 1, 0x1a};
  /*                        S3  S4  S5  E, smsEnd */

  ACI_MALLOC(lesm, (sizeof (T_LEDIT_SRC_MAINTAIN)));
  if (lesm EQ NULL)
  {
    return NULL;
  }
  lesm->next = NULL;

  ACI_MALLOC(lesm->leditInt,(sizeof (T_LEDIT_INTERN)));
  if (lesm->leditInt EQ NULL)
  {
    ACI_MFREE (lesm);
    lesm = NULL;
    return NULL;
  }
  memset(lesm->leditInt, '\0', sizeof (T_LEDIT_INTERN));

  lesm->leditInt->copyIter   = 0;

  lesm->leditInt->srcBuffer  = NULL;
  lesm->leditInt->srcBufIter = 0;

  lesm->leditInt->smsBuffer  = NULL;

  lesm->leditInt->lineBuffer = NULL;
  lesm->leditInt->lineBufIter = 0;

  lesm->leditInt->origBuffer = NULL;
  lesm->leditInt->origBufIter = 0;
  
  lesm->leditInt->execBuffer = NULL;
  lesm->leditInt->execBufIter = 0;

  lesm->leditInt->state      = ledit_idle;
  lesm->leditInt->lineHabit  = ledit_initial;
  lesm->leditInt->err.code   = LEDIT_ERR_NONE;
  lesm->leditInt->err.msg    = NULL;
  lesm->leditInt->isStr      = FALSE;
  lesm->leditInt->cmdGetIter = 0;
  lesm->leditInt->cmdm       = NULL;
  ledit_clear_all(lesm->leditInt);

  return lesm;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_getSrcM      |
+--------------------------------------------------------------------+

  PURPOSE :
*/

static T_LEDIT_SRC_MAINTAIN *ledit_getSrcM (UBYTE src_id, const UBYTE *chars, USHORT len)
{
  static BOOL firstCall                = 1;
         T_LEDIT_SRC_MAINTAIN *ledSrcM = NULL;
         T_LEDIT_SRC_MAINTAIN *tmp     = NULL;

  /*
   * get the very first AT source maintenance
   */
  if (firstCall)
  {
    firstCall = 0;
    if ((rootSrc = ledit_getNewSrcMaintain (rootSrc)) EQ NULL)
    {
      return NULL;
    }
    rootSrc->leditInt->src_id      = src_id;
    rootSrc->leditInt->srcBufIterE = 0;
    rootSrc->leditInt->srcBufIter  = 0;
    rootSrc->leditInt->srcBuffer   = chars;
    rootSrc->leditInt->len         = len;
    return rootSrc;
  }
  /*
   * do we have a maintenance for this src_id ?
   */
  ledSrcM = rootSrc;
  tmp     = rootSrc;
  while(ledSrcM AND ledSrcM->leditInt->src_id NEQ src_id)
  {
    tmp     = ledSrcM;
    ledSrcM = ledSrcM->next;
  }
  if (ledSrcM EQ NULL)
  {
    /*
     * No, this AT source called line edit for the very first time
     */
    if ((ledSrcM = ledit_getNewSrcMaintain (ledSrcM)) EQ NULL)
    {
      return NULL;
    }
    tmp->next = ledSrcM;
  }

  ledSrcM->leditInt->src_id      = src_id;
  ledSrcM->leditInt->srcBufIterE = 0;
  ledSrcM->leditInt->srcBufIter  = 0;
  ledSrcM->leditInt->srcBuffer   = chars;
  ledSrcM->leditInt->len         = len;
  ledSrcM->leditInt->err.msg     = NULL;
  ledSrcM->leditInt->err.code    = LEDIT_ERR_NONE;

  return ledSrcM;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_getSrc       |
+--------------------------------------------------------------------+

  PURPOSE : get an existing source maintenance
*/

static T_LEDIT_SRC_MAINTAIN *ledit_getSrc (UBYTE src_id)
{
    T_LEDIT_SRC_MAINTAIN *ledSrcM = NULL;

  ledSrcM = rootSrc;

  while(ledSrcM AND ledSrcM->leditInt->src_id NEQ src_id)
  {
    ledSrcM = ledSrcM->next;
  }
  if (ledSrcM EQ NULL)
  {
    return NULL;
  }
  return ledSrcM;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_check_param  |
+--------------------------------------------------------------------+

  PURPOSE : -
*/

static T_LEDIT_RSLT ledit_check_param (const UBYTE *chars, USHORT len)
{
  if (len   EQ 0
     OR chars EQ NULL)
  {
    return (LEDIT_FAIL);
  }
  if (len > MAX_CMD_LEN)
  {
    return (LEDIT_FAIL);
  }
  return (LEDIT_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_cmd          |
+--------------------------------------------------------------------+

  PURPOSE : - receive char, chars or complete AT-string sent by UART, bluetooth, SAT, ...
            - run the state machine to build up valid AT commands
*/

T_LEDIT_RSLT ledit_cmd (UBYTE src_id, const UBYTE *chars, USHORT len)
{
  T_LEDIT_RSLT rv                = LEDIT_COLLECT;
  T_LEDIT_SRC_MAINTAIN *lineSrcM = NULL;

  TRACE_FUNCTION ("ledit_cmd()");

  /*
   * get the maintenance for this AT source or create a new one or die
   */
  if ((lineSrcM = ledit_getSrcM (src_id, chars, len)) EQ NULL)
  {
    return (LEDIT_FAIL);
  }
  /*
   * at first check, whether the last cmd line is already processed by ATI
   * ATI will call then ledit_ctrl(src_id,LEDIT_CTRL_CMPL,NULL)
   * which reset the state machine
   */
  if (lineSrcM->leditInt->state EQ ledit_run_cmd)
  {
    /*
     * ATI did not confirmed the last cmd line
     */
    lineSrcM->leditInt->err.code = LEDIT_ERR_LastCmdPending;
    lineSrcM->leditInt->err.msg  = ledit_err[LEDIT_ERR_LastCmdPending].msg;
    return (LEDIT_FAIL);
  }

  /*
   * the caller of ledit_cmd is responsible to echoing the passed chars back
   * so clear at first the old chars from the last call of ledit_cmd
   */
  ledit_echo_clear();

  if (ledit_check_param(chars, len) EQ LEDIT_FAIL)
  {
    lineSrcM->leditInt->err.code = LEDIT_ERR_NoValidCommand;
    lineSrcM->leditInt->err.msg  = ledit_err[LEDIT_ERR_NoValidCommand].msg;
    return (LEDIT_FAIL);
  }
  /*
   * the chars of the raw source cmd line can be parsed now
   * call state machine and check the return value
   */
  while ((lineSrcM->leditInt->srcBufIter < len) AND ((rv EQ LEDIT_COLLECT) OR (rv EQ LEDIT_IGNORE)))
  {
    rv = (*lineSrcM->leditInt->state)(lineSrcM->leditInt);
    if (rv EQ LEDIT_FAIL)
    {
      ledit_clear_all(lineSrcM->leditInt);
      if (lineSrcM->leditInt->lineHabit.atE)
      {
        ledit_echo(lineSrcM->leditInt);
      }
      return rv;
    }
  }
  /*
   * the chars [aAtT] have already been put into g_ledit_echoBuf by these states
   */
  if ((lineSrcM->leditInt->state NEQ ledit_prefix_a)
   AND(lineSrcM->leditInt->state NEQ ledit_prefix_t))
  {
    /*
     * but then, put all chars after [tT] into g_ledit_echoBuf
     */
    if (lineSrcM->leditInt->lineHabit.atE)
    {
      ledit_echo(lineSrcM->leditInt);
    }
  }
  /*
   * if ready to execute, state machine will call ledit_run_cmd()
   * or in case of A/ ledit_repeat() first and then ledit_run_cmd()
   */
  if (lineSrcM->leditInt->state EQ ledit_repeat)
  {
    rv = (*lineSrcM->leditInt->state)(lineSrcM->leditInt);
    if (rv EQ LEDIT_FAIL)
    {
      ledit_clear_all(lineSrcM->leditInt);
      TRACE_EVENT ("ledit_cmd(): ledit_repeat FAILED");
      return rv;
    }
  }

  if (lineSrcM->leditInt->state EQ ledit_run_cmd)
  {
    rv = (*lineSrcM->leditInt->state)(lineSrcM->leditInt);
    if (rv EQ LEDIT_FAIL)
    {
      ledit_clear_all(lineSrcM->leditInt);
      if (lineSrcM->leditInt->lineHabit.atE)
      {
        ledit_echo(lineSrcM->leditInt);
      }
      ledit_echo_clear();
      TRACE_EVENT ("ledit_cmd(): ledit_run_cmd FAILED");
      return rv;
    }
  }
  TRACE_EVENT_P1 ("ledit_cmd(): returns with %d", rv);
  return rv;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_idle         |
+--------------------------------------------------------------------+

  PURPOSE : - state 1 of command line parser
            - start parsing of AT command line with received [aA]
            - is final state with received '\0'
*/
static T_LEDIT_RSLT ledit_idle (T_LEDIT_INTERN *leditInt)
{
  char c = leditInt->srcBuffer[leditInt->srcBufIter++];
#ifdef _SIMULATION_
  TRACE_FUNCTION ("ledit_idle()");
#endif
  if(c EQ leditInt->lineHabit.S3)
  {
    return (LEDIT_OK); /* simple <CR> will emit an OK */
  }

  switch (c)
  {
    case 'a':
    case 'A':
    {
      if (leditInt->lineHabit.atE)
      {
        g_ledit_echoBuf[0] = c;
      }
      leditInt->state = ledit_prefix_a;
      return (LEDIT_COLLECT);
    }
    default:
    {
      TRACE_EVENT_P1("ledit_idle(): ignored character is %d", c);
      return (LEDIT_IGNORE);
    }
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_prefix_a     |
+--------------------------------------------------------------------+

  PURPOSE : - state 2 of command line parser

*/

static void ledit_prefix_help (T_LEDIT_INTERN *leditInt, char c)
{
  if (leditInt->lineHabit.atE)
  {
    if ((g_ledit_echoBuf[0] EQ 'a') OR (g_ledit_echoBuf[0] EQ 'A'))
    {
      g_ledit_echoBuf[1] = c;
    }
    else
    {
      g_ledit_echoBuf[0] = c;
    }
  }
}

static T_LEDIT_RSLT ledit_prefix_a (T_LEDIT_INTERN *leditInt)
{
  char c = leditInt->srcBuffer[leditInt->srcBufIter++];
#ifdef _SIMULATION_
  TRACE_FUNCTION ("ledit_prefix_a()");
#endif
  if(c EQ leditInt->lineHabit.S3)
  {
    return (LEDIT_FAIL); /* A<CR> (or what has been set for S3) makes no sense */
  }

  switch (c)
  {
    case SLASH:                          /* '/' will not be echoed */
    {
      leditInt->state = ledit_repeat;
      return (LEDIT_EXCT);
    }
    case 't':
    case 'T':
    {
      ledit_prefix_help (leditInt, c);
      leditInt->srcBufIterE = leditInt->srcBufIter;
      leditInt->state = ledit_prefix_t;
      return (LEDIT_COLLECT);
    }
    default:
    {
      if(c EQ leditInt->lineHabit.S5)   /* backspace */
      {
        ledit_prefix_help (leditInt, c);
        leditInt->state = ledit_idle;
        return (LEDIT_COLLECT); /* A<S5> go back */
      }
      return (LEDIT_IGNORE);
    }
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_prefix_t     |
+--------------------------------------------------------------------+

  PURPOSE :  - state 3 of command line parser

*/
static T_LEDIT_RSLT ledit_prefix_t (T_LEDIT_INTERN *leditInt)
{
  char c = leditInt->srcBuffer[leditInt->srcBufIter++];
#ifdef _SIMULATION_
  TRACE_FUNCTION ("ledit_prefix_t()");
#endif
  if(c EQ leditInt->lineHabit.S3)
  {

    if (leditInt->lineHabit.atE)
    {
      g_ledit_echoBuf[2] = c;
    }
    return (LEDIT_OK); /* AT<CR> will emit an OK */
  }
  if(c EQ leditInt->lineHabit.S5)     /* backspace */
  {
    ledit_prefix_help (leditInt, c);
    leditInt->state = ledit_prefix_a;
    return (LEDIT_COLLECT); /* AT<S5> go back */
  }
  leditInt->srcBufIter--; /* collect needs the first character after "AT", as well */
  leditInt->state = ledit_collect;
  return (LEDIT_COLLECT);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_collect      |
+--------------------------------------------------------------------+

  PURPOSE :  - state 4 of command line parser

*/
static T_LEDIT_RSLT ledit_repeat (T_LEDIT_INTERN *leditInt)
{
  TRACE_FUNCTION ("ledit_repeat()");

  ++(leditInt->srcBufIter); /* to reach the end of cmd line */

  if (leditInt->execBuffer AND leditInt->execBuffer[0])  /* is there anything to repeat? */
  {
    if (leditInt->lineBuffer)                           /* discard current "A/" */
    {
      ACI_MFREE(leditInt->lineBuffer);
      leditInt->lineBuffer = NULL;
    }

    if (leditInt->origBuffer)                           /* discard current "A/" */
    {
      ACI_MFREE(leditInt->origBuffer);
      leditInt->origBuffer = NULL;
    }

    ledit_echo_clear ();
    leditInt->state = ledit_run_cmd;
    return (LEDIT_EXCT);
  }
  return (LEDIT_FAIL);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : LINE_EDIT_LIB      |
| STATE   : code                        ROUTINE : ledit_collect      |
+--------------------------------------------------------------------+

  PURPOSE :  - state 4 of command line parser

*/
static T_LEDIT_RSLT ledit_collect (T_LEDIT_INTERN *leditInt)
{
  int  i = 0;
  char c = leditInt->srcBuffer[leditInt->srcBufIter++];
  USHORT len = 0;
#ifdef _SIMULATION_
  TRACE_FUNCTION ("ledit_collect()");
#endif
  if (leditInt->lineBuffer EQ NULL)
  {
    ledit_clear_lineBuf(leditInt);
  }

  while (i < leditInt->len)
  {
    if (leditInt->copyIter+2 EQ MAX_CMD_LEN) /* some precheck, +2 for "AT" */
    {
      if (c EQ leditInt->lineHabit.S3)
        ;
      else if (c EQ leditInt->lineHabit.S5)
        ;
      else
      {
        ledit_prefix_help(leditInt, '\a');
        return (LEDIT_IGNORE);  /* loop on all other commands if we would exceed the string */
      }
    }

    if(c EQ leditInt->lineHabit.S3)
    {
      leditInt->origBuffer[leditInt->origBufIter] = '\0';    /* terminate cmd string */
      leditInt->lineBuffer[leditInt->copyIter] = '\0';    /* terminate cmd string */
      if (leditInt->isStr NEQ FALSE)
      {
        /*
         * a string parameter did not end with "
         * e.g: at+cpbw=1,"+491721212",145,"D2 Kundenbetreuung <--- missing closing "
         */
        leditInt->err.code = LEDIT_ERR_NoValidCommand;
        leditInt->err.msg  = ledit_err[LEDIT_ERR_NoValidCommand].msg;
        return (LEDIT_FAIL);
      }
      /*
       * for possibly repeating backup the current cmd line
       */
      ledit_backup(leditInt);
      leditInt->state    = ledit_run_cmd;
      leditInt->origBufIter = 0;
      leditInt->copyIter = 0;
      return (LEDIT_EXCT);
    }
    else if(c EQ leditInt->lineHabit.S4)
    {
      ; /* ignore LF */
    }
    else if(c EQ leditInt->lineHabit.S5)
    {
      len = leditInt->origBufIter;                      
      if ( leditInt->origBuffer[--(len)] EQ '"' )    /* Checks if the character to be deleted is '"' */
      {
        leditInt->isStr = !(leditInt->isStr);      /* Toggles the variable "isStr" when '"" got deleted */
      }
      if (leditInt->copyIter > 0)
      {
        --(leditInt->copyIter);     /* delete last character in cmd buffer by setting iterator i one step back */
      }
      if (leditInt->origBufIter > 0)
      {
        --(leditInt->origBufIter);     /* delete last character in cmd buffer by setting iterator i one step back */
      }
      if (leditInt->copyIter EQ 0)  /* reached first character after "AT" */
      {
        leditInt->state = ledit_prefix_t;       /* Fall back into state 't' */
        ledit_prefix_help (leditInt, c);
        leditInt->origBuffer[leditInt->origBufIter] = '\0';  /* Terminate Command !!!  ACI-FIX-12036 AV2 */
        leditInt->lineBuffer[leditInt->copyIter] = '\0';  /* Terminate Command !!!  ACI-FIX-12036 AV2 */
        return (LEDIT_COLLECT); /* AT<S5> go back */
      }
    }
    else
    {
      switch (c)
      {
        case WS:
        {
          if (leditInt->isStr) /* don't modify anything within string e.g.: "BlAh 1234" */
          {
            leditInt->origBuffer[leditInt->origBufIter++] = c;
            leditInt->lineBuffer[leditInt->copyIter++] = c;
            break;
          }
          break; /* just to eat up white space */
        }
        default:
        {
          if ((c > 0x20) AND (!(c & 0x80))) /* only printable chars with 7 bits */
          {
            if (c EQ '"')
            {
              leditInt->isStr = !(leditInt->isStr);
            }
            leditInt->origBuffer[leditInt->origBufIter++] = c; /* copy character to cmd buffer */
            if (!(leditInt->isStr) AND c >= 'a' AND c <= 'z')
            {
               c -= 0x20; /* to upper */
            }
             leditInt->lineBuffer[leditInt->copyIter++] = c; /* copy character to cmd buffer */
          }
        }
      }
    }
    c = leditInt->srcBuffer[leditInt->srcBufIter++];
    ++i;
    if (leditInt->srcBufIter > leditInt->len)
    {
      break;
    }
  }
  leditInt->origBuffer[leditInt->origBufIter] = '\0';
  leditInt->lineBuffer[leditInt->copyIter] = '\0';  /* Terminate Command !!!  ACI-FIX-12036 AV2 */

  return (LEDIT_COLLECT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  : LINE_EDIT_LIB       |
| STATE   : code                       ROUTINE : ledit_run_cmd       |
+--------------------------------------------------------------------+

  PURPOSE :  - state 5 of command line parser

*/
static T_LEDIT_RSLT ledit_run_cmd (T_LEDIT_INTERN *leditInt)
{
  TRACE_FUNCTION("ledit_run_cmd()");
  leditInt->cmdIndex = 1;
  leditInt->cmdGetIter = 1;
  ledit_free_cmd(leditInt);       /* clear all (=previous) commands */
  leditInt->cmdm = NULL;
  return (ledit_split (leditInt));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)       MODULE  : LINE_EDIT_LIB             |
| STATE   : code                 ROUTINE : ledit_clear_cmdBuf        |
+--------------------------------------------------------------------+

  PURPOSE : -
*/
static T_LEDIT_RSLT ledit_clear_lineBuf (T_LEDIT_INTERN *leditInt)
{
  if (leditInt->lineBuffer EQ NULL)
  {
    ACI_MALLOC(leditInt->lineBuffer,((MAX_CMD_LEN+1) * sizeof (char)));
  }
  if (leditInt->lineBuffer EQ NULL)
    return (LEDIT_FAIL);

  memset(leditInt->lineBuffer, '\0',(MAX_CMD_LEN+1) * sizeof (char));
  if (leditInt->origBuffer EQ NULL)
  {
    ACI_MALLOC(leditInt->origBuffer,((MAX_CMD_LEN+1) * sizeof (char)));
  }
  if (leditInt->origBuffer EQ NULL)
    return (LEDIT_FAIL);

  memset(leditInt->origBuffer, '\0',(MAX_CMD_LEN+1) * sizeof (char));
  return (LEDIT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)       MODULE  : LINE_EDIT_LIB             |
| STATE   : code                 ROUTINE : ledit_clear_all           |
+--------------------------------------------------------------------+

  PURPOSE : -
*/
static T_LEDIT_RSLT ledit_clear_all (T_LEDIT_INTERN *leditInt)
{
  ledit_free_cmd(leditInt); /* maintained by ledit_split */
  leditInt->cmdm = NULL;

  if (leditInt->smsBuffer)
  {
    ACI_MFREE (leditInt->smsBuffer);
    leditInt->smsBuffer = NULL;
  }

  if (leditInt->lineBuffer)
  {
    ACI_MFREE (leditInt->lineBuffer);
    leditInt->lineBuffer = NULL;
  }
  if (leditInt->origBuffer)
  {
    ACI_MFREE (leditInt->origBuffer);
    leditInt->origBuffer = NULL;
  }

  leditInt->txtChunk       = 0;
  leditInt->copyIter       = 0;
  leditInt->origBufIter       = 0;
  leditInt->lineBufIter    = 0;
  leditInt->execBufIter    = 0;
  leditInt->srcBufIter     = 0;
  leditInt->cmdGetIter     = 0;
  leditInt->cmdIndex       = 0;
  leditInt->isStr          = FALSE;
  leditInt->state          = ledit_idle; /* reset to default state */
  return (LEDIT_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)       MODULE  : LINE_EDIT_LIB             |
| STATE   : code                 ROUTINE : ledit_trace_line          |
+--------------------------------------------------------------------+

  PURPOSE : prepare a text buffer for tracing of an AT cmd line.
            a cmd line is ready for tracing after receiving of the termination
            character<S3> and removing of all non necessary characters,
            e.g.: white spaces and the the characters in the mAlberTO phenomenon
            "   m   a lb  er t    o<S3>" is interpreted as ATO !
            the characters can come in as single chars, chunk of chars or as complete line.
            the actual trace takes place with trace_cmd_line()
            
*/

T_LEDIT_RSLT ledit_trace_line (UBYTE src_id, char *txt)
{
  T_LEDIT_SRC_MAINTAIN *lineSrcM = NULL;
  /*
   * get the maintenance for this AT source
   */
  if ((lineSrcM = ledit_getSrcM (src_id, NULL, 0)) EQ NULL)
  {
    return (LEDIT_FAIL);
  }
  
  if (lineSrcM->leditInt->execBuffer)
  {
    strncpy (txt, lineSrcM->leditInt->execBuffer, 77); /* limit to 77 chars, 2 are used for "AT" */
    txt[77] = '\0';
    return (LEDIT_CMPL);
  }
  else
  {
    return (LEDIT_FAIL);
  } 
}

