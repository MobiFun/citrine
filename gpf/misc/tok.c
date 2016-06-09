/* 
+------------------------------------------------------------------------------
|  File:       tok.c
+------------------------------------------------------------------------------
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
|  Purpose :  This Modul performs Configuration string functions
+----------------------------------------------------------------------------- 
*/ 

#ifndef __TOK_C__
#define __TOK_C__

#include "typedefs.h"
#include <string.h>
#include <ctype.h>

#include "tok.h"

#ifdef _VXWORKS_
#undef ERROR     -1                   /* Token types               */
#undef EOS        1
#endif
#define ERROR     -1                   /* Token types               */
#define EOS        1
#define COMMA      2
#define EQUAL      3
#define IDENT      4
#define STRING     5
#define LBRACK     6
#define RBRACK     7

static SHORT tok_gettok (char **);
static SHORT tok_value  (char **);
static int tok_issep  (char);

/*
Format:
   List   ::= Token
          ::= Token , Token, ...

   Token  ::= Ident
          ::= Ident = Values

   Values ::= Value
          ::= (Value)
          ::= (Value , Value , ... )
          ::= ()

   Value  ::= Ident
          ::= "String"

*/

#ifndef RUN_INT_RAM
static TOK_DCB  tok_dcb;
static char     tok_buf[80];
#endif

#ifndef RUN_INT_RAM
/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
| PROJEKT : TETRA-PS (6143)            MODUL   : TOK.C              |
| STATUS  : code                       ROUTINE : tok_key            |
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

   PURPOSE : Return code value of a keyword from table.

*/

SHORT tok_key (KW_DATA * keytab, char * keyword)
{
  /*
   * Empty string terminates
   */
  while (keytab->keyword[0])
  {
    if (strcmp (keytab->keyword, keyword ) == 0)
      return (keytab->code);
    keytab++;
  }

  return (TOK_NOT_FOUND);
}
#endif


#ifndef RUN_INT_RAM
/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
| PROJEKT : TETRA-PS (6143)            MODUL   : TOK.C              |
| STATUS  : code                       ROUTINE : tok_init           |
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

   PURPOSE : Initialize token scanner.

*/

void tok_init (char * s)
{
   tok_dcb.tokbuf      = tok_buf;
   strncpy (tok_dcb.tokbuf, s, 79);
   tok_dcb.tokbuf[79]  = '\0';
   tok_dcb.nexttok     = tok_dcb.tokbuf;
   tok_dcb.lastchar    = 0;
}
#endif

#ifndef RUN_INT_RAM
/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
| PROJEKT : TETRA-PS (6143)            MODUL   : TOK.C              |
| STATUS  : code                       ROUTINE : tok_next           |
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

   PURPOSE : Get next token (and its values) from token string.
             Return number of value strings.

*/

SHORT tok_next (char ** keyw, char * value [])
{
  char * val;

  if (tok_dcb.nexttok == NULL)
    return (TOK_EOCS);

  if (tok_gettok (keyw) != IDENT)
  {
    tok_dcb.nexttok = NULL;
    return (TOK_EOCS);
  }

  /*
   * Check next token
   */
  switch (tok_gettok (&val))
  {
    /*
     * No value present
     */
    case COMMA:
      return (0);
    /*
     * Value(s) follows
     */
    case EQUAL:
      return (tok_value (value));
    /*
     * No value present and EOS
     */
    case EOS:
      tok_dcb.nexttok = NULL;
      return (0);
    /*
     * Syntax error
     */
    default:
      tok_dcb.nexttok = NULL;
      return (TOK_EOCS);
  }
}
#endif

#ifndef RUN_INT_RAM
/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
| PROJEKT : TETRA-PS (6143)            MODUL   : TOK.C              |
| STATUS  : code                       ROUTINE : tok_value          |
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

   PURPOSE : Get list of values for token.
             Return number of values found.

             Formats:  Value
                       (Value)
                       (Value, Value,...)
                       ()
*/

static SHORT tok_value (char * value [])
{
  SHORT   count;
  SHORT   tok;
  SHORT   inbrack;
  char  * val;
  char  * val2;

  inbrack = 0;
  /*
   * Get next token
   */
  tok = tok_gettok (&val);

  switch (tok)
  {
    case LBRACK:
      inbrack++;
      break;

    case IDENT  :
    case STRING :
      tok = tok_gettok (&val2);
      if ((tok != COMMA) && (tok != EOS))
      {
        tok_dcb.nexttok = NULL;
        return (0);
      }

      value[0] = val;
      return (1);

    case EOS :
    default  :
      tok_dcb.nexttok = NULL;
      return (0);
  }

  /*
   * Get first token of list
   */

   tok = tok_gettok (&val);
   count = 0;
   for(;;)
   {
     if ((tok == IDENT) || (tok == STRING))
       value[count++] = val;
     else
     {
       tok_dcb.nexttok = NULL;
       return (0);
     }

     tok = tok_gettok (&val);
     switch (tok)
     {
       case COMMA:
         break;

       case RBRACK :
         if (inbrack)
         {
           if (((tok = tok_gettok (&val)) == COMMA) ||
                (tok == EOS))
             return (count);
         }
         /*
          * Fall through
          */
       /*lint -fallthrough */
       default:
         tok_dcb.nexttok = NULL;
         return (0);
     }
     tok = tok_gettok (&val);
   }
   /*lint -e527 suppress Warning -- Unreachable */
   return (0);
   /*lint +e527 */

}
#endif

#ifndef RUN_INT_RAM
/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
| PROJEKT : TETRA-PS (6143)            MODUL   : TOK.C              |
| STATUS  : code                       ROUTINE : tok_gettok         |
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

   PURPOSE : Return next token in input string.

*/

static SHORT tok_gettok (char ** token)
{
  SHORT   tok;
  char   hadnull;
  char  * cp;

  cp      = tok_dcb.nexttok;
  hadnull = tok_dcb.lastchar != 0;

  if (tok_dcb.lastchar)
  {
    * tok_dcb.nexttok = tok_dcb.lastchar;
    tok_dcb.lastchar = 0;
  }
  /*
   * Skip leading white space
   */
  while (isspace (* tok_dcb.nexttok))
    tok_dcb.nexttok++;

  * token = tok_dcb.nexttok++;

  switch (** token)
  {
    case '\0':                         /* End of string             */
    case '\n':
      tok = EOS;
      break;

    case ',':
      ** token = '\0';
      tok = COMMA;
      break;

    case '=':
      ** token = '\0';
         tok = EQUAL;
         break;

      case '(':
      case '<':
      case '[':
        ** token = '\0';
        tok = LBRACK;
        break;

      case ')':
      case '>':
      case ']':
        ** token = '\0';
        tok = RBRACK;
        break;

      case '"':
        /*
         * Get first char of string
         */
        * token = tok_dcb.nexttok;
        while ((* tok_dcb.nexttok != '\0') && (* tok_dcb.nexttok != '"'))
          tok_dcb.nexttok++;

        if (* tok_dcb.nexttok != '\0')
            * tok_dcb.nexttok++ = '\0';

        tok = STRING;
        break;

      default:
        /*
         * Read an identifier
         */
        if (!tok_issep (** token))
        {
          while (!tok_issep (* tok_dcb.nexttok))
            tok_dcb.nexttok++;

          tok_dcb.lastchar = * tok_dcb.nexttok;
          * tok_dcb.nexttok = '\0';

          tok = IDENT;
        }
        else
          tok = ERROR;
        break;
   }
   /*
    * '\0 was replaced with char
    * Replace original '\0'
    */

   if (hadnull)
     * cp = '\0';

   return (tok);
}
#endif

#ifndef RUN_INT_RAM
/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
| PROJEKT : TETRA-PS (6143)            MODUL   : TOK.C              |
| STATUS  : code                       ROUTINE : tok_issep          |
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

   PURPOSE : Return 1 if special character.

*/

static int tok_issep (char c)
{
   switch (c)
   {
      case '\0' :
      case '\n' :
      case ','  :
      case '='  :
      case '('  :
      case '<'  :
      case '['  :
      case ')'  :
      case '>'  :
      case ']'  :
      case '"'  : return (1);

      default   : return (isspace (c));
   }
}
#endif
#endif
