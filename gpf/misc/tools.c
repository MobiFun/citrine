/* 
+------------------------------------------------------------------------------
|  File:       tools.c
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
|  Purpose :  This module implements some format conversion functions.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __TOOLS_C__
#define __TOOLS_C__
#endif
 
/*==== INCLUDES ===================================================*/

#include "typedefs.h"
#include <string.h>
#include <ctype.h>
#include "vsi.h"
#include "tools.h"

/*==== TYPES ======================================================*/


/*==== CONSTANTS ==================================================*/


/*==== EXTERNALS ==================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/



#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  GetNextToken
+------------------------------------------------------------------------------
|  Description  :  Exract text until next separator.
|
|  Parameters   :  source     - where to search
|                  token      - destination for text
|                  seperator  - sperator
|
|  Return       :  string length of token
+------------------------------------------------------------------------------
*/
unsigned int GetNextToken (char *source, char *token, char const *seperators)
{
  unsigned int  i, j, k, sep_len, src_len;
  BOOL sepFound = FALSE;

  sep_len = strlen (seperators);
  src_len = strlen (source);

  i = 0;
  j = 0;
  k = 0;

  do
  {
    j = 0;
    sepFound = FALSE;

    while (j < sep_len)
    {
      if (source[i] EQ seperators[j])
        sepFound = TRUE;
      j++;
    }
    if (sepFound)
      i++;
  }
  while (i < src_len AND sepFound);

  sepFound = FALSE;

  while (!sepFound AND i < src_len)
  {
    j = 0;
    while (!sepFound AND j < sep_len)
    {
      if (source[i] EQ seperators[j])
        sepFound = TRUE;
      else
        j++;
    }

    if (!sepFound)
      token[k++] = source[i++];
  }
  token[k] = '\0';

  return strlen (token);
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  HexToASCII
+------------------------------------------------------------------------------
|  Description  :  Convert hexadecimal value to ASCII string.
|
|  Parameters   :  value     - value to be converted
|                  *ptr      - destination for string
|                  num       - number of characters
|
|  Return       :  pointer behind the end of string 
+------------------------------------------------------------------------------
*/
char *HexToASCII (ULONG value, char *ptr, int num)
{
  UBYTE i;
  char v;

  ptr += (num-1);

  for (i=0; i<num ;i++)
  {
    v = (char)(value & 0x000000f);

    value >>= 4;

    if (v > 9)
      v += ('A'-10);
    else
      v += '0';

    *ptr-- = v;
  }

  return(ptr+num+1);
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ASCIIToHex
+------------------------------------------------------------------------------
|  Description  :  Convert ASCII string to hexadecimal value.
|
|  Parameters   :  *ptr      - string to be converted
|                  num       - number of characters
|
|  Return       :  value 
+------------------------------------------------------------------------------
*/
unsigned int ASCIIToHex (char *ptr, int num)
{
  unsigned int  i;
  unsigned int result = 0;
  unsigned int len;
  char  c;

  len = strlen(ptr);
  if ( len < (unsigned int)num )
    num = (int)len;

  ptr += (num-1);

  for (i=0; i < (unsigned int)num; i++)
  {
    if (isxdigit (*ptr) )
    {
      if (*ptr > '9')
        c = *ptr-- - 'A' + 10;
      else
        c = *ptr-- - '0';

      /*lint -e701 suppress Info -- shift left of signed quantity */
      result += (c << (i*4));
      /*lint +e701 */
    }
  }
  return result;
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  InsertString
+------------------------------------------------------------------------------
|  Description  :  write a string into the buffer. If the string has less than 
|                  'num' characters, spaces are added.
|
|  Parameters   :  *string   - string to be written
|                  *ptr      - pointer to destination
|                  num       - number of characters to be filled
|
|  Return       :  pointer to next character
+------------------------------------------------------------------------------
*/
char *InsertString (char *string, char *ptr, int num)
{
  UBYTE  i;
  BYTE  end_detected = FALSE;

  for (i=0;i<num;i++)
  {
    if (end_detected)
      *ptr++ = ' ';
    else
    {
      if (string [i] == 0)
      {
        end_detected = TRUE;
        *ptr++ = ' ';
      }
      else
        *ptr++ = string [i];
    }
  }
  return(ptr);
}
#endif


#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  rm_path
+------------------------------------------------------------------------------
|  Description  :  remove path name from file name
|
|  Parameters   :  *file        - file name
|
|  Return       :  file name without path information
|                  
+------------------------------------------------------------------------------
*/
char *rm_path ( const char *file )
{
char *end;

  /*
   * to avoid crashes in case the passed parameter file is a pointer to
   * anywhere it is checked if file begins with a character that is allowed
   * at the beginning of file/path names (any alpanumeric character, '.', '/',
   * and '\')
   */
  if ( isalpha(*file) == 0 && *file != '.' && *file != '/' && *file != '\\' )
    return ((char*)"NO VALID FILENAME");

  end = (char*)file + strlen(file);

  while ((*end != 0x2f) && (*end != 0x5c) && (end >= (char*)file))
    end--;

  return end + 1;
}
#endif
