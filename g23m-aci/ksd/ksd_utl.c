/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  KSD_UTL
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
|  Purpose :  This module defines utility functions for the KSD
|             component of the protocol stack.
+----------------------------------------------------------------------------- 
*/ 

#ifndef KSD_UTL_C
#define KSD_UTL_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/

/*==== CONSTANTS ==================================================*/

#define SUBADDR_DELIMITER '-'  /* used to separate the subaddress  */
                               /* from the main address            */
#define NULL_TERM         '\0' /* string termination               */

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD_UTL                  |
| STATE   : code                  ROUTINE : utl_cutInternational     |
+--------------------------------------------------------------------+

  PURPOSE : This function cuts all characters which signals that the
            dial number is international (leading '+' or leading
            '00').

            <dial>:       the origin dial number
            <cuttedDial>: the dial number without leading '+' or '00'

            returns:      TRUE if the dial number was of the type
                          international, otherwise FALSE.
*/
LOCAL BOOL utl_cutInternational (CHAR* dial, CHAR** cuttedDial)
{
  BOOL international = FALSE; /* holds whether number is */
                              /* international           */

  if (dial[0] EQ '+')
  {
    international = TRUE;
    *cuttedDial   = dial + 1;
  }
  /*
   * 00 is not an indication for an international call
  else if (dial[0] EQ '0' AND dial[1] EQ '0')
  {
    international = TRUE;
    *cuttedDial   = dial + 2;
  }
  */
  else
    *cuttedDial   = dial;

  return international;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD_UTL                  |
| STATE   : code                  ROUTINE : utl_getValidDialchars    |
+--------------------------------------------------------------------+

  PURPOSE : This function returns the number of valid dial characters 
            in the buffer. This function is only used to process 
            the subaddress part.

            <dial>:  the dial number (subaddress)

            returns: the number of valid dial characters.
*/
LOCAL USHORT utl_getValidDialchars (CHAR* dial)
{
  USHORT numChars = 0; /* holds number of valid dial characters */
  USHORT i        = 0; /* used for counting                     */

  while (dial[i] NEQ NULL_TERM)
  {
    if ((dial[i] >= '0' AND dial[i] <= '9') OR
        (dial[i] >= 'A' AND dial[i] <= 'C') OR
        (dial[i] >= 'a' AND dial[i] <= 'c') OR
         dial[i] EQ '*'                     OR
         dial[i] EQ '#'                       )
      numChars++;
         
    i++;
  }

  return numChars;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD_UTL                  |
| STATE   : code                  ROUTINE : utl_splitDialnumber      |
+--------------------------------------------------------------------+

  PURPOSE : This function splits a dial string into address and
            subaddress and delivers additional information about
            the specific types of addresses.

            <dial>:          the origin dial number
            <main>:          the main address of the dial number;
                             if it was international, leading '+'
                             or '00' are cutted
            <international>: TRUE if the dial number is international,
                             otherwise FALSE
            <sub>:           the sub address of the dial number or
                             NULL if there is none
            <numCharsSub>:   the number of valid dial characters of
                             the sub address
*/
GLOBAL void utl_splitDialnumber (CHAR*   dial,
                                 CHAR**  main,
                                 BOOL*   international,
                                 CHAR**  sub,
                                 USHORT* numCharsSub)
{
  CHAR* token; /* points to dial number */
  
  if (dial NEQ NULL)
  {
    token = strchr (dial, SUBADDR_DELIMITER);
  
    if (token NEQ NULL)
    {
      *token       = NULL_TERM;
      *sub         = ++token;
      *numCharsSub = utl_getValidDialchars (token);
    }
    else
    {
      *sub         = NULL;
      *numCharsSub = 0;
    }

    *international = utl_cutInternational (dial, main);
  }
  else
  {
    *main          = NULL;
    *international = FALSE;
    *sub           = NULL;
    *numCharsSub   = 0;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD_UTL                  |
| STATE   : code                  ROUTINE : utl_string2UByte         |
+--------------------------------------------------------------------+

  PURPOSE : This function converts characters from a buffer to an
            unsigned byte value.

            <inUByte>:  buffer containing the characters forming 
                        the unsigned byte value
            <inLen>:    number of characters to be taken into account
            <outValue>: the converted value

            returns:    TRUE if the conversion was successful,
                        otherwise FALSE.
*/
GLOBAL BOOL utl_string2UByte (CHAR*  inUByte,
                              USHORT inLen,
                              UBYTE* outValue)
{
  UBYTE result = 0; /* holds the result value of conversion */

  if (inLen EQ 0)
    return TRUE;

  while (inLen--)
  {
    result *= 10;
    if (*inUByte >= '0' AND *inUByte <= '9')
      result += *inUByte++ - '0';
    else
      return FALSE;
  }

  *outValue = result;
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD_UTL                  |
| STATE   : code                  ROUTINE : utl_string2Byte          |
+--------------------------------------------------------------------+

  PURPOSE : This function converts characters from a buffer to a
            signed byte value.

            <inByte>:   buffer containing the characters forming 
                        the signed byte value
            <inLen>:    number of characters to be taken into account
            <outValue>: the converted value

            returns:    TRUE if the conversion was successful,
                        otherwise FALSE.
*/
GLOBAL BOOL utl_string2Byte (CHAR*  inByte,
                             USHORT inLen,
                             BYTE*  outValue)
{
  BYTE result = 0; /* holds the result value of conversion */

  if (inLen EQ 0)
    return TRUE;

  while (inLen--)
  {
    result *= 10;
    if (*inByte >= '0' AND *inByte <= '9')
      result += *inByte++ - '0';
    else
      return FALSE;
  }

  *outValue = result;
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD_UTL                  |
| STATE   : code                  ROUTINE : utl_string2Short         |
+--------------------------------------------------------------------+

  PURPOSE : This function converts characters from a buffer to an
            short value.

            <inShort>:  buffer containing the characters forming 
                        the short value
            <inLen>:    number of characters to be taken into account
            <outValue>: the converted value

            returns:    TRUE if the conversion was successful,
                        otherwise FALSE.
*/
GLOBAL BOOL utl_string2Short (CHAR*  inShort,
                              USHORT inLen,
                              SHORT* outValue)
{
  SHORT result = 0; /* holds the result value of conversion */

  if (inLen EQ 0)
    return TRUE;

  while (inLen--)
  {
    result *= 10;
    if (*inShort >= '0' AND *inShort <= '9')
      result += *inShort++ - '0';
    else
      return FALSE;
  }

  *outValue = result;
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD_UTL                  |
| STATE   : code                  ROUTINE : utl_string2Long          |
+--------------------------------------------------------------------+

  PURPOSE : This function converts characters from a buffer to an
            long value.

            <inLong>:   buffer containing the characters forming 
                        the short value
            <inLen>:    number of characters to be taken into account
            <outValue>: the converted value

            returns:    TRUE if the conversion was successful,
                        otherwise FALSE.
*/
GLOBAL BOOL utl_string2Long (CHAR*  inLong,
                             USHORT inLen,
                             LONG*  outValue)
{
  LONG result = 0; /* holds the result value of conversion */

  if (inLen EQ 0)
    return TRUE;

  while (inLen--)
  {
    result *= 10;
    if (*inLong >= '0' AND *inLong <= '9')
      result += *inLong++ - '0';
    else
      return FALSE;
  }

  *outValue = result;
  return TRUE;
}
