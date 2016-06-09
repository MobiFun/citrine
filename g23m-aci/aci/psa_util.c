/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_UTIL
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
|  Purpose :  This module provides utility functions 
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_UTIL_C
#define PSA_UTIL_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"

/*==== CONSTANTS ==================================================*/
/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D ()                  MODULE  : ATI_RET            |
| STATE   : code                        ROUTINE : psa_search_SATSrcId|
+--------------------------------------------------------------------+

  PURPOSE : search SAT Source Id
*/

/* CLB: can't be returning a UBYTE... Otherwise search_SATSrcId is
ALWAYS positive: -1 is 0xFF */
#ifdef FF_SAT_E
GLOBAL T_ACI_CMD_SRC psa_search_SATSrcId (void)
{
  T_ACI_CMD_SRC src_id; /* Current command source id. */

  TRACE_FUNCTION("psa_search_SATSrcId ()");

#ifdef SIM_TOOLKIT
  for (src_id = CMD_SRC_LCL; src_id < CMD_SRC_MAX; src_id++)
  {
    if (ati_is_src_type((UBYTE) src_id, ATI_SRC_TYPE_SAT))
    {
      return src_id;
    }
  }
#endif /* SIM_TOOLKIT */
  
  return(CMD_SRC_NONE);
}
#endif /* FF_SAT_E */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_UTIL                 |
| STATE   : code                  ROUTINE : utl_BCD2String           |
+--------------------------------------------------------------------+

  PURPOSE : Function for converting a buffer that contains unpacked
            bcd digits of either MCC or MNC into a printable, 
            null terminated C-string. Note: The output buffer "bcd_string"
            has to have at least the size len + 1, as the "len" is the
            length of the input "bcd" digits and the terminating '\0' 
            also has to be stored.

*/

GLOBAL void utl_BCD2String (char *bcd_string, const UBYTE *bcd, USHORT len)
{
  SHORT idx;

  for (idx = 0; idx < len; idx++)
  {
    if (bcd[idx] <= 9)
    {
      bcd_string[idx] = bcd[idx] + '0';
    }
    else
    {
      bcd_string[idx] = '\0';
      return;
    }
  }
  bcd_string[len] = '\0';
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_UTIL                 |
| STATE   : code                  ROUTINE : utl_dialStr2BCD          |
+--------------------------------------------------------------------+

  PURPOSE : Function that cleans all characters in a dialnumber
            that are not in 0-9,a,b,c,#,*. and converts the valid
            characters into a BCD number. the function returns the
            number of BCD digits that are converted. If pBCDBuf is 
            NULL, only the length of the resulting BCD number is evaluated.

*/

GLOBAL USHORT utl_dialStr2BCD (const char *pDialStr, UBYTE *pBCDBuf,
                               UBYTE  maxDigits)
{
  USHORT numBcdDigits = 0, strIdx;
  UBYTE c;

  for(strIdx = 0; numBcdDigits < maxDigits AND pDialStr[strIdx] NEQ '\0'; strIdx++)
  {
    c = 0xFF;
    switch (pDialStr[strIdx])
    {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        c = pDialStr[strIdx]-'0';
        break;

      case '*':
        c = 0x0a;
        break;

      case '#':
        c = 0x0b;
        break;

      case 'A':
        c = 0x0c;
        break;

      case 'B':
        c = 0x0d;
        break;

      case 'C':
        c = 0x0e;
        break;

      default: /* Ignore non-matching characters */
        break;
    }
    if (c NEQ 0xFF)
    {
      if (pBCDBuf NEQ NULL)
        pBCDBuf[numBcdDigits] = c;
      numBcdDigits++;
    }
  }

  return numBcdDigits;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_UTIL                 |
| STATE   : code                  ROUTINE : utl_BCD2DialStr          |
+--------------------------------------------------------------------+

  PURPOSE : converts a BCD number into a string of ascii digits

*/

GLOBAL void utl_BCD2DialStr (const UBYTE *pBCD, char *pDSBuf, UBYTE numDigits)
{

  for( ;numDigits > 0; numDigits--, pBCD++ )
  {
    switch( *pBCD )
    {
      case( 0x00 ):
      case( 0x01 ):
      case( 0x02 ):
      case( 0x03 ):
      case( 0x04 ):
      case( 0x05 ):
      case( 0x06 ):
      case( 0x07 ):
      case( 0x08 ):
      case( 0x09 ):

        *pDSBuf++ = *pBCD + '0';
        break;

      case( 0x0a ):
        *pDSBuf++ = '*';
        break;

      case( 0x0b ):
        *pDSBuf++ = '#';
        break;

      case( 0x0c ):
        *pDSBuf++ = 'A';
        break;

      case( 0x0d ):
        *pDSBuf++ = 'B';
        break;

      case( 0x0e ):
        *pDSBuf++ = 'C';
        break;
    }
  }

  *pDSBuf = 0x0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_UTIL                |
| STATE   : code                  ROUTINE : utl_cvt7To8             |
+-------------------------------------------------------------------+

  PURPOSE : This function converts from the 7 bit default alphabet
            for SMS point-to-point messages to 8 bit alphabet.
            The function returns the length in bytes of the converted
            destination string.

            Note: This function is copied from entity SMS, file
                  sms_tlf.c
*/

/* PATCH add a parameter to tell the function to begin to expand
   to 8 bits from a specific bit offset inside the first byte 
   (needed to handle correctly TP-UD headers in SMS) */
GLOBAL UBYTE utl_cvt7To8 (const UBYTE* source,
                          UBYTE  source_len,
                          UBYTE* dest,
                          UBYTE bit_offset) /* indicates which bit of the first byte is the boundary of the 7bits source
                                               ex: for bit_offset = 3, bits 0-2 (low weight bits) will not be decoded*/
{
  UBYTE d_mask = 0x01;
  UBYTE s_mask;
  UBYTE icnt;
  UBYTE ocnt;
  
  *dest = 0x00;
  icnt  = source_len;   

  if( !icnt ) return( 0 );
  ocnt  = 0;
  
  while (icnt)               
  {
    s_mask = 0x01;      

    /* PATCH */
    if (icnt == source_len)
      s_mask <<= bit_offset;
    /* PATCH end */

    while (s_mask > 0x00)   
    {
      if (s_mask & *source)  
        *dest |= d_mask;  
          
      s_mask <<= 1;     
      d_mask <<= 1;
              
      if (d_mask > 0x40)     
      {
        dest++;              
        ocnt++;
        d_mask = 0x01;
        *dest  = 0x00;
      }
    }

    source++;                
    icnt--;
  }

  return( ocnt );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_UTIL                |
| STATE   : code                  ROUTINE : utl_cvtPnn7To8          |
+-------------------------------------------------------------------+

  PURPOSE : This function converts from the 7 bit default alphabet
            for PNN Operator names to 8 bit alphabet.
            The function returns the length in bytes of the converted
            destination string.
*/

GLOBAL UBYTE utl_cvtPnn7To8 (const UBYTE* source, UBYTE source_len, UBYTE dcs, UBYTE* dest )
{
  return utl_cvt7To8 (source,
                      (UBYTE)(MINIMUM (MAX_ALPHA_OPER_LEN,(source_len*8-(dcs&0x07)+6)/7) ),
                      dest,
                      0);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_UTIL                |
| STATE   : code                  ROUTINE : utl_cvt8To7             |
+-------------------------------------------------------------------+

  PURPOSE : This function converts from the 8 bit default alphabet
            for SMS point-to-point messages to 7 bit alphabet.
            The function returns the length in bytes of the converted
            destination string.
            
            Note: This function is copied from entity SMS, file
                  sms_tlf.c
*/
GLOBAL UBYTE utl_cvt8To7 ( UBYTE* source,
                          UBYTE  source_len,
                           UBYTE* dest,
                           UBYTE  bit_offset)
{
  UBYTE  d_mask = 0x01;
  UBYTE  s_mask;
  UBYTE  icnt;
  UBYTE ocnt;

  *dest = 0x00;
  icnt  = source_len;

  if( !icnt ) return( 0 );
  ocnt  = 1;

  /* PATCH */
  if (icnt == source_len)
    d_mask <<= bit_offset;
  /* PATCH end */

  while (icnt)
  {
    s_mask = 0x01;

    while (s_mask <= 0x40)
    {
      if (s_mask & *source)
        *dest |= d_mask;

      s_mask <<= 1;
      d_mask <<= 1;

      if (d_mask EQ 0x00)
      {
        dest++;

        /* don't increment ocnt if the new octet will not be used */
        if ((s_mask < 0x80) OR (icnt NEQ 1))
          ocnt++;

        d_mask = 0x01;
        *dest  = 0x00;
      }
    }

    source++;
    icnt--;
  }

  return( ocnt );
}

/* Implements Measure 25 */

/*==== EOF ========================================================*/
