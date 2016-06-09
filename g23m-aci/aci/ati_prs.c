/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ATI
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
|  Purpose :  AT Command parameter parser
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_PRS_C
#define ATI_PRS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_prs.h"

#ifdef GPRS
  #include "gaci_cmh.h"
#endif /* GPRS */

#include "aci_lst.h"
#include "ati_int.h"

#ifdef OPTION_RELATIVE
LOCAL ULONG offset;
#endif

static UBYTE parse_index = 0;
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseBin           |
+--------------------------------------------------------------------+

  PURPOSE : parse binary value

*/

GLOBAL char *parseBin (int digits, char *b, SHORT *i)
{
    int found = 0;

    *i = 0;
    while (digits-- AND *b >= '0' AND *b <= '1')
    {
        found++;
        *i *= 2;
        *i += *b - '0';
        b++;
    }

    return ((found) ? b : 0);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseHex           |
+--------------------------------------------------------------------+

  PURPOSE : parse hexadecimal value

*/

GLOBAL char *parseHex (int digits, char *b, UBYTE *i)
{
    int found = 0;

    *i = 0;
    while (digits-- AND ((*b >= '0' AND *b <= '9') OR
                        (*b >= 'a'  AND *b <= 'f') OR
                        (*b >= 'A'  AND *b <= 'F')))
    {
        found++;
        *i *= 16;
        if (*b >= '0' AND *b <= '9')
            *i += *b - '0';
        if (*b >= 'a' AND *b <= 'f')
            *i += *b - 'a' + 10;
        if (*b >= 'A' AND *b <= 'F')
            *i += *b - 'A' + 10;
        b++;
    }

    return ((found AND found <= 2) ? b : 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseHexToLong     |
+--------------------------------------------------------------------+

  PURPOSE : parse hexadecimal value

*/

GLOBAL char *parseHexToLong (int digits, char *b, LONG *i)
{
    *i = 0;
    while (digits-- AND ((*b >= '0' AND *b <= '9') OR
                        (*b >= 'a'  AND *b <= 'f') OR
                        (*b >= 'A'  AND *b <= 'F')))
    {
        *i *= 16;
        if (*b >= '0' AND *b <= '9')
            *i += *b - '0';
        if (*b >= 'a' AND *b <= 'f')
            *i += *b - 'a' + 10;
        if (*b >= 'A' AND *b <= 'F')
            *i += *b - 'A' + 10;
        b++;
    }

    return ( digits EQ 0 ? 0 : b );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseEnum          |
+--------------------------------------------------------------------+

  PURPOSE : parse enum value

*/

GLOBAL char *parseEnum (int digits, char *b, int *i)
{
    int found = 0;

    *i = 0;
    while (digits-- AND *b >= '0' AND *b <= '9')
    {
        found++;
        *i *= 10;
        *i += *b - '0';
        b++;
    }

    return ((found) ? b : 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseShort         |
+--------------------------------------------------------------------+

  PURPOSE : parse Short value

*/

GLOBAL char *parseShort (int digits, char *b, SHORT *i)
{
    int found = 0;

    *i = 0;
    while (digits-- AND *b >= '0' AND *b <= '9')
    {
        found++;
        *i *= 10;
        *i += *b - '0';
        b++;
    }

    return ((found) ? b : 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseStr           |
+--------------------------------------------------------------------+

  PURPOSE : parse string value

*/

GLOBAL char *parseStr (int digits, char *b, char *s)
{
    int found = 0;

    *s = 0;
    if (*b NEQ '"')
        return 0;
    b++;
    while (digits-- AND *b AND *b >= 0x20)
    {
        found++;
        if (*b EQ '"')
        {
            *s = 0;
            b++;
            break;
        }
        if (*b EQ '\\')
        {
            b = parseHex(2,b+1,(UBYTE *) s); /* ES!! ZERO ?           */
            if (!b)
                return 0;
            s++;
        }
        else
            *s++ = *b++;
    }
    if(*(b-1) NEQ '"')
      return 0;
    *s = 0;                             /* zero terminated          */

    return ((found) ? b : 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseZStr          |
+--------------------------------------------------------------------+

  PURPOSE : parse string value

*/

GLOBAL char *parseZStr (int digits, char *b, char *s)
{
    int found = 0;

    *s = 0;
    if (*b NEQ '"')
        return 0;
    b++;
    while (digits--)
    {
        found++;
        if (*b EQ '"')
        {
            *s = 0;
            b++;
            break;
        }
        if (*b EQ '\\')
        {
            b = parseHex(2,b+1,(UBYTE *) s); /* ES!! ZERO ?           */
            if (!b)
                return 0;
            s++;
        }
        else
            *s++ = *b++;
    }
    if(*(b-1) NEQ '"')
      return 0;
    *s = 0;                             /* zero terminated          */

    return ((found) ? b : 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseNStr          |
+--------------------------------------------------------------------+

  PURPOSE : parse string value and if not numeric returns error

*/

GLOBAL char *parseNStr (int digits, char *b, char *s)
{
    int found = 0;

    *s = 0;
    if (*b NEQ '"')
        return 0;
    b++;
    while (digits-- AND *b AND *b >= 0x20)
    {
        found++;
        if (*b EQ '"')
        {
            *s = 0;
            b++;
            break;
        }
        if (*b EQ '\\')
        {
            b = parseHex(2,b+1,(UBYTE *) s); /* ES!! ZERO ?           */
            if (!b)
                return 0;
            s++;
        }
        if ( *b > '9' OR *b < '0')
	    return 0; /*check if numeric value*/
	else
            *s++ = *b++;
    }
    if(*(b-1) NEQ '"')
      return 0;
    *s = 0;                             /* zero terminated          */

    return ((found) ? b : 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseASCII         |
+--------------------------------------------------------------------+

  PURPOSE : parse ASCII value

*/

GLOBAL char *parseASCII (int digits, char *b, char *i)
{
    int found = 0;

    *i = 0;
    while (digits-- AND *b NEQ ' ' AND *b NEQ ',' AND *b NEQ ';' )
    {
        found++;
        *i = *b;
        i++;
        b++;
    }

    return ((found) ? b : 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseQASCII        |
+--------------------------------------------------------------------+

  PURPOSE : parse ASCII value and terminate with 0

*/

GLOBAL char *parseQASCII (int digits, char *b, char *i)
{
    int found = 0;

    *i = 0;
    while (digits-- AND *b NEQ ',' AND *b NEQ ';' AND *b NEQ '\0')
    {
        found++;
        *i = *b;
        i++;
        b++;
    }

    if (found)
    {
      *i = 0;
      return (b);
    }

    return (NULL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseStrLen        |
+--------------------------------------------------------------------+

  PURPOSE : parse sting, return length and startpoint

*/

GLOBAL char *parseStrLen (USHORT *len, char *b, char **p)
{
  *len = 0;
  if( *b EQ '"' )
    b++;

  /* Check if the string is a null string like "" */
  if( *b EQ '"' )
  {
    b++;

   /* return new buffer position without setting p, thereby omitting the parameter */
    return b; 
  }

  *p = b;

  while ( *b NEQ '\0' AND *b NEQ ',' AND *b NEQ ';' AND *b NEQ '"' )
  {
    (*len)++;
    b++;
  }
 
  if(*b EQ '"')
  {
    b++;
  }

  return ((*len) ? b : 0);
}


#ifdef GPRS

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ACI_PRS            |
| STATE   : devellopment                ROUTINE : parseExpo          |
+--------------------------------------------------------------------+

  PURPOSE : parse exponential expresion (ex. 2E3)

*/

GLOBAL char *parseExpo( int digits, char *b, T_PS_ber *e )
{
  BOOL valid = FALSE;

  e->ratio_mant = 0;
  e->ratio_exp  = 0;
  if( *b EQ '"' ) /* start of string */
  {
    b++;
    while( digits-- AND *b >= '0' AND *b <= '9' )
    {
      e->ratio_mant *= 10;
      e->ratio_mant += *b - '0';
      b++;
    }
    if( *b EQ 'E' )
    {
      b++;
      while( digits-- AND *b >= '0' AND *b <= '9' )
      {
        e->ratio_exp *= 10;
        e->ratio_exp += *b - '0';
        b++;
        valid = TRUE;
      }
      
      if( *b EQ '"' ) /* end of string */
      {
        b++;
      }
      else
      {
        valid = FALSE;
      }
      	
    }
    else
    {
      valid = FALSE;
    }
  }
  else
  {
    valid = FALSE;
  }

  return ((valid) ? b : 0);

} /* END parseExpo(...) */

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ACI_PRS            |
| STATE   : Finished                    ROUTINE : parseCid_list      |
+--------------------------------------------------------------------+

  PURPOSE : Parse a list of Cids. If the parsing is sucessfull the 
            number of parsed cids is returned otherwise -1 is returned.

*/
GLOBAL char *parseCid_list(char *b, U8 *cid_list_length, U8 *cid_list)
{
  U8    no_of_cids;
  char *end;

  /* Parse all cids into the cid_list */
  no_of_cids = 0;
  do
  {

    /* If it's not the first cid remove the "," */
    if( no_of_cids )
      b++;

    /* Set the entry to omitted. */
    cid_list[no_of_cids] = PDP_CONTEXT_CID_OMITTED;

    /* Parse the value */
    cid_list[no_of_cids] = (U8) strtol( b, &end, 10 );
    b = end;

    /* If a cid is omitted no cids must be specified at all. */
    if( cid_list[no_of_cids] EQ PDP_CONTEXT_CID_OMITTED )
    {
      if( no_of_cids NEQ 0 )
      {
        /* Don't accept omitted cids inside if it's not the first (ex. 1,,2,3 is not allowed). */
        cmdCmeError( CME_ERR_OpNotSupp );
        return( NULL );
      }
      /* Break the while */
      break;
    }
    if( (!b) OR 
        (cid_list[no_of_cids] <  PDP_CONTEXT_CID_MIN) OR 
        (cid_list[no_of_cids] >  PDP_CONTEXT_CID_MAX) OR 
        (no_of_cids           >= PDP_CONTEXT_CID_MAX) )
    {
      cmdCmeError( CME_ERR_OpNotSupp );
      return( NULL );
    }
    no_of_cids++;
  }
  
  while( *b EQ ','  AND
    no_of_cids < *cid_list_length );
  
  cid_list[no_of_cids+1] = PDP_CONTEXT_CID_INVALID;

  *cid_list_length = no_of_cids;
  return( b );

}

#endif /* GPRS */

/*
+----------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_PRS              |
| STATE   : -                           ROUTINE : parsePointerToString |
-----------------------------------------------------------------------+

  PURPOSE : Parse pointer to input string. returns pointer to end of string.

*/
GLOBAL char *parsePointerToString(char chars, char *b, char **p_begin)
{
  if( b AND *b EQ '"' )
  {
    b++;
    *p_begin = b;
    
    while (chars-- AND *b AND *b NEQ '"')
    {
      b++;
    }
    if( *b EQ '"' )
    {
      b++;
    }
    else
    {
      if( !chars )
      {
        b = NULL;
      }
    }
  }

  return b;
}

/*
+----------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_PRS              |
| STATE   : -                           ROUTINE : StrToU16             |
-----------------------------------------------------------------------+

  PURPOSE : Parse string value to U16.

*/

char *StrToU16(char *p_str, U16 *p_result)
{
  U32 val = 0;
  
  if( p_str )
  {
    if( *p_str >= '0' AND *p_str <= '9' )
    {
      while( *p_str >= '0' AND *p_str <= '9' )
      {
        val *= 10;
        val += (int)(*p_str - '0');
        p_str++;
      }
    }
  }

  if( val <= 0xffff ) /* 0xffff = max value of U16 */
  	*p_result = (U16)(val);
  else
  	p_str = NULL;
  
  return p_str;
  
}

/*
+---------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_PRS             |
| STATE   : -                           ROUTINE : parseDSNP_U16       |
----------------------------------------------------------------------+

  PURPOSE : parse dot-separated numeric parameters from the 
            input string to an array of U8.
            
            ex. input p_in = "192.124.13.3"
                output p_out = {192,124,13,3} 

*/
GLOBAL char *parseDSNP_U16( char blocks, char *p_in, U16 *p_out )
{
  if( p_in AND p_out )
  {
    if( *p_in EQ '.' ) p_in++;
    while( blocks-- AND *p_in NEQ 0x00 )
    {
      p_in = StrToU16(p_in, p_out);
      
      if( p_in AND blocks AND *p_in EQ '.' )
      {
        p_in++;
        p_out++;
      }
      else
        break;
    }
  }
  return p_in;
}

/*
+---------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_PRS             |
| STATE   : -                           ROUTINE : parseIPv4Address    |
----------------------------------------------------------------------+
*/
GLOBAL char *parseIPv4Address(char *p_in, U8 * p_ip_addr, U8 * is_ip_valid)
{
  U16  ip_addr[4];
  char i;
  char *p_out = NULL;

  if( p_in AND p_ip_addr )
  {
    p_out = parseDSNP_U16(4, p_in, ip_addr);
    for(i=0; i<4; i++)
    {
      if(ip_addr[i] <= 255)
      {
        p_ip_addr[i] = (U8) ip_addr[i];
      }
      else
      {
        p_out = NULL;
      }

    }
  }
  * is_ip_valid = (p_out NEQ NULL);

  return p_out;
}

/*
+---------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_PRS             |
| STATE   : -                           ROUTINE : parseIPv6Address    |
----------------------------------------------------------------------+
*/
GLOBAL char *parseIPv6Address(char *p_in, U8 *p_ip_addr, U8 * is_ip_valid)
{
  U16  ip_addr[16];
  char i;
  char *p_out = NULL;

  if( p_in AND p_ip_addr )
  {
    p_out = parseDSNP_U16(16, p_in, ip_addr);
    for(i=0; i<16; i++)
    {
      if(ip_addr[i] > 255)
        p_out = NULL;
      else
        p_ip_addr[i] = (U8) ip_addr[i];
    }
  }

  * is_ip_valid = (p_out NEQ NULL);

  return p_out;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parse              |
+--------------------------------------------------------------------+

  PURPOSE : parse AT command parameter

*/

GLOBAL char *parse (char *b, char *f, ...)
{
  char fc;
  USHORT *i;

#ifdef GPRS
  U8      *j;
#endif /* GPRS */
  LONG    digits = -1;
  BOOL value_is_valid;
 /*lint -e516 -e10*/
  va_list varpars;

  va_start (varpars, f);        /* Initialize variable arguments. */

  parse_index=0;
  while ((fc = *f++) NEQ 0)
  {
    parse_index++;              /* Save the position of current parsing item */
    value_is_valid = TRUE;
    if (islower(fc))
    {
      if (*b EQ 0 OR *b EQ ';')
      {
        va_end (varpars);        /* Reset variable arguments.      */
        return b;
      }
      if (*b EQ ',')
      {
        b++;
        value_is_valid = FALSE;
      }
    }

    switch (toupper(fc))
    {
      case 'B':
        if (value_is_valid)
          b = parseBin(-1,b,va_arg (varpars, short*));
        else
          (void)va_arg (varpars, short*);
        break;
      
      case 'X':
        if (value_is_valid)
          b = parseHex(-1,b,va_arg (varpars, unsigned char*));
        else
          (void)va_arg (varpars, short*);
        break;
      
      case 'Y':
        if (value_is_valid)
          b = parseHexToLong(-1,b,va_arg (varpars, long*));
        else
          (void)va_arg (varpars, short*);
        break;
      
      case 'D':
        if (value_is_valid)
          b = parseEnum(-1,b,va_arg (varpars, int*));
        else
          (void)va_arg (varpars, int*);
        break;
      
      case 'R':
        if (value_is_valid)
          b = parseShort(-1,b,va_arg (varpars, short*));
        else
          (void)va_arg (varpars, short*);
        break;
      
      case 'S':
        if (value_is_valid)
        {
          digits=va_arg (varpars, long);
          b = parseStr(digits,b,va_arg (varpars, char*));
        }
        else
        {
          (void)va_arg (varpars, long);
          (void)va_arg (varpars, char*);
        }
				break;
      
			case 'N':
        if (value_is_valid)
        {
          digits=va_arg (varpars, long);
   				b = parseNStr(digits,b,va_arg (varpars, char*));
        }
        else
        {
          (void)va_arg (varpars, long);
          (void)va_arg (varpars, char*);
        }
        break;
      
      case 'Z':
        if (value_is_valid)
        {
          char*   c    = b;
          USHORT* plen = NULL;

          char*   cmdPtr;
          USHORT  cmdLen;

          digits = va_arg ( varpars, long );
          cmdLen = va_arg ( varpars, unsigned );
          cmdPtr = va_arg ( varpars, char* );
          plen   = va_arg ( varpars, unsigned short*);
          b      = parseZStr
                     ( MINIMUM (digits, cmdLen - ( b - cmdPtr )),
                       b, va_arg ( varpars, char* ) );

          if ( plen NEQ NULL )
          {
            if ( b NEQ NULL )
              *plen = b - c - 2; /* a string always includes two characters '"' */
            else
              *plen = 0;
          }
        }
        else
        {
          (void)va_arg (varpars, long);
          (void)va_arg (varpars, unsigned short*);
          (void)va_arg (varpars, char*);
        }
        break;
      
      case 'A':
        if (value_is_valid)
        {
          digits=va_arg (varpars, long);
          b = parseASCII(digits,b,va_arg (varpars, char*));
        }
        else
        {
          (void)va_arg (varpars, long);
          (void)va_arg (varpars, char*);
        }
        break;
      
      case 'Q':
        if (value_is_valid)
        {
          digits=va_arg (varpars, long);
          b = parseQASCII(digits,b,va_arg (varpars, char*));
        }
        else
        {
          (void)va_arg (varpars, long);
          (void)va_arg (varpars, char*);
        }
        break;

      case 'L':
        if (value_is_valid)
        {
          i = va_arg(varpars, unsigned short*);
          b = parseStrLen (i,b,va_arg(varpars, char**));
        }
        else
        {
          (void)va_arg (varpars, int*);
          (void)va_arg (varpars, char**);
        }
        break;

#ifdef GPRS

      case 'E':
        if (value_is_valid)
        {/*lint -e78 -e26 -e530*/
          b = parseExpo( -1, b, va_arg(varpars, T_PS_ber*));
        }
        else
        {
          (void)va_arg (varpars, T_PS_ber*);
        }
        break;

#endif

      case 'C':
        if (value_is_valid)
        {
          b = parseByte( -1, b, va_arg(varpars, unsigned char*));
        }
        else
        {
          (void)va_arg (varpars, unsigned char*);
        }
        break;

#ifdef GPRS

      case 'I': /* Parse cid_list */
        if( value_is_valid )
        {
          j = va_arg(varpars, unsigned char*);

          b = parseCid_list( b, j, va_arg(varpars, unsigned char*) );
        }
        break;
#endif /* GPRS */

      case 'P': /* Parse pointer to string */
        if( value_is_valid )
        {
          b = parsePointerToString( (char)-1, b, va_arg(varpars, char**) );
        }
        break;

      case '*': /* Parse '*' */
        if( value_is_valid )
        {
          if( *b NEQ '*' )
          {
            return 0;
          }
          b++;
        }
        break;

      case '#': /* Parse '#' */
        if( value_is_valid )
        {
          if( *b NEQ '#' )
          {
            return 0;
          }
          b++;
        }
        break;
     }
    if (!b)
    {
      va_end (varpars);              /* Reset variable arguments.      */
      return 0;
    }

    if (*f AND *b EQ ',' AND value_is_valid)
    {
      b++;
    }
    else if (*b AND *b NEQ ';' AND *b NEQ ',' AND value_is_valid)  /* if comma or semicolon is missing */
    {
      TRACE_EVENT ("missing separator detected, aborting parse!");
      va_end (varpars);
      return 0;
    }
  }
  if (*b AND *b NEQ ';')
  {
    va_end (varpars);              /* Reset variable arguments.      */
    return 0;
  }
  va_end (varpars);              /* Reset variable arguments.      */
 /*lint +e516 +e10*/
  return b;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parse              |
+--------------------------------------------------------------------+

  PURPOSE : get last parsed index (to determinate which component was malicious)

*/

GLOBAL UBYTE get_parse_index ()
{
  return parse_index;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseByte(U8)      |
+--------------------------------------------------------------------+

  PURPOSE : parse U8 value

*/

GLOBAL char *parseByte (int digits, char *b, U8 *i)
{
    int found = 0;

    *i = 0;
    while (digits-- AND *b >= '0' AND *b <= '9')
    {
        found++;
        *i *= 10;
        *i += *b - '0';
        b++;
    }

    return ((found) ? b : 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PRS            |
| STATE   : code                        ROUTINE : parseTimeStamp              |
+--------------------------------------------------------------------+

  PURPOSE : Parse time stamp format in "yy/MM/dd,hh:mm:ss±zz"

*/

GLOBAL UBYTE parseTimeStamp( char *p_in,U8 *years,U8 *months,U8 *days,
                             U8 *hrs, U8 *mins,U8 *secs, char *sign, U8 *time_zone )
{
  if( p_in[2] NEQ '/' OR  p_in[5] NEQ '/' OR p_in[8] NEQ ',' OR p_in[11] NEQ ':' OR p_in[14] NEQ ':' )
  {
    return NO_TIME_STAMP_FIELD;
  }
  if(!parseByte (2, &p_in[0], years))
  {
    return NO_TIME_STAMP_FIELD;
  }
  if(!parseByte (2, &p_in[3], months))
  {
    return NO_TIME_STAMP_FIELD;
  }
  if(!parseByte (2, &p_in[6], days))
  {
    return NO_TIME_STAMP_FIELD;
  }
  if(!parseByte (2, &p_in[9], hrs))
  {
    return NO_TIME_STAMP_FIELD;
  }
  if(!parseByte (2, &p_in[12], mins))
  {
    return NO_TIME_STAMP_FIELD;
  }
  if(!parseByte (2, &p_in[15], secs))
  {
    return NO_TIME_STAMP_FIELD;
  }
  *sign = p_in[17];

  if(*sign NEQ '+' AND  *sign NEQ '-' )
  {
    return NO_TIME_STAMP_FIELD;
  }
  if(!parseByte (2, &p_in[18], time_zone))
  {
    return NO_TIME_STAMP_FIELD;
  }

 return MAX_TIME_STAMP_FIELDS; 
}

#endif /* ATI_PRS_C */
