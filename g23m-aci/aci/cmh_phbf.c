/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_PHBF
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
|  Purpose :  This module defines the functions used by the command
|             handler for the phonebook management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_PHBF_C
#define CMH_PHBF_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "phb.h"
#include "cmh.h"
#include "cmh_phb.h"

/*==== CONSTANTS ==================================================*/

#define PHB_CMH_INVALID_TAG_ENTRY 0xFF /* fill up a tag        */
#define PHB_CMH_INVALID_BCD_ENTRY 0xF  /* fill up a BCD number */

#define PHB_CMH_YEAR_INVLD     (255)   /* invalid year         */
#define PHB_CMH_MONTH_INVLD    (255)   /* invalid month        */
#define PHB_CMH_DAY_INVLD      (255)   /* invalid day          */
#define PHB_CMH_HOUR_INVLD     (255)   /* invalid hour         */
#define PHB_CMH_MINUTE_INVLD   (255)   /* invalid minute       */
#define PHB_CMH_SECOND_INVLD   (255)   /* invalid second       */

/*==== TYPES ======================================================*/
/* Implements Measure 108, 109 */
/*
 * Here we are having two SIM Phonebook as in our code we have two, 
 * one is FFS based that has been defined in phb_sim.h 
 * And the other one is RAM based has been defined in phb.h
 */

/* 
 * Conversion Array that convert the type of phonebook used by
 * phonebook management to the type of phonebook used by ACI.
 */
#ifdef TI_PS_FFS_PHB 
/*
 * FFS based SIM phonebook
 */
const T_ACI_PB_STOR cmhPHB_cvtPhbPsaType[]=
{
  PB_STOR_Ed,          /* ECC */
  PB_STOR_Ad,          /* ADN */
  PB_STOR_Fd,          /* FDN */
  PB_STOR_Bd,          /* BDN */
  PB_STOR_Ld,          /* LDN */
  PB_STOR_Lr,          /* LRN */
  PB_STOR_Sd,          /* SDN */
  PB_STOR_Lm,          /* LMN */  
  PB_STOR_Ud,          /* UPN */
  PB_STOR_NotPresent,  /* ME */
  PB_STOR_NotPresent,  /* ADN_ME */
  PB_STOR_NotPresent,  /* MAX_PHONEBOOK */
};
#else
/*
 * Old RAM based SIM phonebook.
*/
const T_ACI_PB_STOR cmhPHB_cvtPhbPsaType[]=
{
  PB_STOR_Ed,          /* ECC */
  PB_STOR_Ad,          /* ADN */
  PB_STOR_Fd,          /* FDN */
  PB_STOR_Bd,          /* BDN */
  PB_STOR_Ld,          /* LDN */
  PB_STOR_Lr,          /* LRN */
  PB_STOR_Sd,          /* SDN */
  PB_STOR_Lm,          /* LMN */
  PB_STOR_NotPresent,  /* ADN_FDN */
  PB_STOR_Ud,          /* UPN */
  PB_STOR_NotPresent   /* MAX_PHONEBOOK */
};
#endif /* TI_PS_FFS_PHB */

/* 
 * Conversion Array that convert the type of phonebook used by
 * used by ACI to the type of phonebook used by phonebook management.
 */
const T_PHB_TYPE cmhPHB_cvtPBType[]=
{
  FDN,                 /* PB_STOR_Fd */
  LDN,                 /* PB_STOR_Ld */
  ECC,                 /* PB_STOR_Ed */
  ADN,                 /* PB_STOR_Ad */
  BDN,                 /* PB_STOR_Bd */
  LRN,                 /* PB_STOR_Lr */
  SDN,                 /* PB_STOR_Sd */
  LMN,                 /* PB_STOR_Lm */
#ifdef TI_PS_FFS_PHB
  INVALID_PHB,        /* PB_STOR_Af , Not supported anymore */
#else
  UNUSED_INDEX,
#endif
  UPN                  /* PB_STOR_Ud */
};

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBF                     |
| STATE   : code             ROUTINE : cmhPHB_rplcIntnl             |
+-------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL BOOL cmhPHB_rplcIntnl ( CHAR*  inNumber, 
                               CHAR*  outNumber,
                               UBYTE  inCnt )
{
  USHORT inIdx         = 0;
  USHORT outIdx        = 0;
  UBYTE  actCnt        = 0;
  BOOL   international = FALSE;

  while ( inNumber[inIdx] NEQ '\0' )
  {
    /*
     *---------------------------------------------------------------
     * international indicator "+"
     *---------------------------------------------------------------
     */
    if ( inNumber[inIdx]   EQ '+' AND
         inNumber[inIdx-1] EQ '*'     )
    {
      if ( actCnt EQ inCnt OR inCnt EQ 0xFF )
      {
        international = TRUE;
      }
      else
      {
        outNumber[outIdx++] = inNumber[inIdx];
      }
    } 
    /* PATCH LE 12.05.00
     * 00 is used in china as national number
     * checked against competitor phone
     * now ACI has the same behaviour
     */
    /*
     *---------------------------------------------------------------
     * international indicator "00"
     *---------------------------------------------------------------
     */
    /* else if ( inNumber[inIdx+1] EQ '0' AND 
               inNumber[inIdx]   EQ '0' AND
               inNumber[inIdx-1] EQ '*'     )
     {
       if ( actCnt EQ inCnt OR inCnt EQ 0xFF )
       {
         international = TRUE;
         inIdx++;
       }
       else
       {
         outNumber[outIdx++] = inNumber[inIdx++];
         outNumber[outIdx++] = inNumber[inIdx];
       }
     } */
    /* END PATCH LE 12.05.00  */
    /*
     *---------------------------------------------------------------
     * national indicator
     *---------------------------------------------------------------
     */
    else
    {
      if ( inNumber[inIdx] EQ '*' )
      {
        actCnt++;
      }

      outNumber[outIdx++] = inNumber[inIdx];
    }

    inIdx++;
  }
                     
  outNumber[outIdx] = '\0';

  return international;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                |
| STATE   : code                  ROUTINE : cmhPHB_cvtPhbType       |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to convert the type of phonebook
            used by ACI to the type of phonebook used by phonebook
            management.
*/
GLOBAL BOOL cmhPHB_cvtPhbType ( T_ACI_PB_STOR inMem, T_PHB_TYPE* outMem )
{
  /* Implements Measure 108, 109 */
  if ( (inMem <= PB_STOR_Ud) AND (inMem NEQ PB_STOR_Af) 
                             AND (inMem > PB_STOR_NotPresent) )
  {
    *outMem = ( T_PHB_TYPE ) cmhPHB_cvtPBType[inMem];
    return ( TRUE );
  }  
  else
  {
    return ( FALSE );
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PHBF           |
| STATE   : code                        ROUTINE : cmhPHB_toaMrg      |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to build the type of address octet
            from T_ACI_TOA structure.
*/
GLOBAL void cmhPHB_toaMrg ( T_ACI_TOA* type, UBYTE* octet )
{
  *octet = ( ( ( type -> ton << 4 ) & 0xF0 ) + ( type -> npi & 0x0F ) ) | 0x80;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CMH_PHBF           |
| STATE   : code                        ROUTINE : cmhPHB_toaDmrg     |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to build the T_ACI_TOA structure
            from the type of address octet.
*/
GLOBAL void cmhPHB_toaDmrg ( UBYTE octet, T_ACI_TOA* type )
{
  if ( octet EQ 0xFF )
  {
    type -> ton = TON_NotPresent;
    type -> npi = NPI_NotPresent;
  }
  else
  {
    type -> ton = (T_ACI_TOA_TON)(( octet & 0x70 ) >> 4);
    type -> npi = (T_ACI_TOA_NPI)(octet & 0x0F);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                 |
| STATE   : code                  ROUTINE : cmhPHB_getAdrStr         |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to convert the phone number
            in BCD to the phone number as a string.

            BCD Note: Two digits are stored in one byte.

                      +-------+-------+-------+-------+
            BCD:      | 1 - 0 | 3 - 2 | F - 4 | F - F | 
                      +-------+-------+-------+-------+

            STRING:   "01234"

*/
GLOBAL CHAR cmhPHB_convertBCD2char( UBYTE bcd )
{
  switch ( bcd )
  {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      return (bcd + '0');

    case 0x0a:
      return ('*');

    case 0x0b:
      return ('#');

    case 0x0c:
      return (PHB_DIAL_PAUSE);  /* 'P' */

    case 0x0d:
      return (PHB_WILD_CRD);  /* '?' */

    case 0x0e:
      return (PHB_EXP_DIGIT); /* 'E' */

    default:
      return ('\0');
  }
}

GLOBAL UBYTE cmhPHB_getAdrStr ( CHAR*  pStr,
                                UBYTE  maxIdx,
                          const UBYTE* pBcd,
                                UBYTE  numDigits )
{
  BOOL  highNibble = FALSE;
  
  UBYTE bcdIdx     = 0;
  UBYTE strIdx     = 0;

  UBYTE bcd;
  char digit;
  BOOL ignore_next = FALSE;

  while ( bcdIdx < numDigits AND strIdx < maxIdx )
  {
    if ( highNibble )
    {
      bcd = ( pBcd[bcdIdx] & 0xF0 ) >> 4;
      bcdIdx++;
    }
    else
    {
      bcd = pBcd[bcdIdx] & 0x0F;
    }

    highNibble = ! highNibble;

    if(bcd EQ PHB_CMH_INVALID_BCD_ENTRY)
    {
      break;
    }

    digit = cmhPHB_convertBCD2char( bcd );
    if (ignore_next EQ TRUE)
    {
      ;                     /* skip the digit following on an extension flag since we do not support it */
      ignore_next = FALSE;  /* reset flag */
    }
    else if (digit EQ PHB_EXP_DIGIT)
    {
      ignore_next = TRUE;   /* we currently do not support Expansion Digit so skip next digit */
    }
    else
    {
      pStr[strIdx++] = cmhPHB_convertBCD2char( bcd );  /* normal and supported digit */
    }

  }

  pStr[strIdx] = '\0';
  return ( strIdx );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                 |
| STATE   : code                  ROUTINE : cmhPHB_getAdrBcd         |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to convert the phone number
            as a string to the phone number in BCD.

            STRING: "01234"

            BCD Note: Two digits are stored in one byte.

                    +-------+-------+-------+-------+
            BCD:    | 1 - 0 | 3 - 2 | F - 4 | F - F | 
                    +-------+-------+-------+-------+
*/
GLOBAL void cmhPHB_getAdrBcd ( UBYTE* pBcd,
                               UBYTE* pNumDigits,
                               UBYTE  maxDigits,
                               CHAR*  pStr )
{
  BOOL  highNibble = FALSE;

  UBYTE bcdIdx     = 0;
  UBYTE strIdx;
  SHORT restLen;

  UBYTE bcd;
  
  for(strIdx = 0; bcdIdx < maxDigits AND pStr[strIdx] NEQ '\0'; strIdx++)
  {
    switch ( pStr[strIdx] )
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
        bcd = pStr[strIdx] - '0';
        break;

      case '*':
        bcd = 0x0a;
        break;

      case '#':
        bcd = 0x0b;
        break;

      case 'w':
      case 'p':
      case 'W':
      case 'P':
        bcd = 0x0c;
        break;

      case '?':
        bcd = 0x0d;
        break;

      case 'E':
        bcd = 0x0e;
        break;

      default:
        continue;
    }

    if ( highNibble )
    {
      pBcd[bcdIdx] += ( bcd << 4 ); 
      bcdIdx++;
    }
    else

      pBcd[bcdIdx] = bcd;

    highNibble = ! highNibble;
  }
  
  /* 
   *-----------------------------------------------------------------
   * The last byte of the phone number is filled up with 'F' if
   * necessary
   *-----------------------------------------------------------------
   */
  if ( highNibble )
  {
    pBcd[bcdIdx] += ( PHB_CMH_INVALID_BCD_ENTRY << 4 );
    bcdIdx++;
  }

  *pNumDigits = bcdIdx;

  /* 
   *-----------------------------------------------------------------
   * The rest of the phone number is filled up with 'FF'
   *-----------------------------------------------------------------
   */
  restLen = maxDigits - bcdIdx;
  if ( restLen > 0 )
    memset ( &pBcd[bcdIdx], 
             (PHB_CMH_INVALID_BCD_ENTRY << 4) + PHB_CMH_INVALID_BCD_ENTRY,
             restLen );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                 |
| STATE   : code                  ROUTINE : cmhPHB_cpyRecEntr        |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to copy the relevant data from the
            T_PHB_RECORD structure to the T_ACI_PB_ENTR structure.
*/
GLOBAL void cmhPHB_cpyRecEntr ( T_ACI_PB_ENTR* entry,
#ifdef TI_PS_FFS_PHB
                               T_PHB_TYPE     book,
#endif
                          const T_PHB_RECORD*  record )
{
#ifdef TI_PS_FFS_PHB
  entry -> index = record->phy_recno;
#else
  entry -> index = ( SHORT ) record -> index;
#endif

  cmhPHB_getAdrStr ( entry -> number,
                     MAX_PHB_NUM_LEN - 1,
                     record -> number,
                     record -> len );

  cmhPHB_toaDmrg ( record -> ton_npi, &entry -> type );

  cmhPHB_getMfwTagNt ( record -> tag,  record -> tag_len, 
                       entry  -> text.data, &entry -> text.len );

  entry -> text.cs = CS_Sim;

/* Implements Measure 108, 109 */
#ifdef TI_PS_FFS_PHB
  if ( (book >= INVALID_PHB) AND (book <= MAX_PHONEBOOK) )
  {
    entry -> book = ( T_ACI_PB_STOR ) cmhPHB_cvtPhbPsaType[book];
  }
#else
  if ( record -> book <= MAX_PHONEBOOK )
  {
    entry -> book = ( T_ACI_PB_STOR ) cmhPHB_cvtPhbPsaType[record -> book];
  }
#endif /* TI_PS_FFS_PHB */

  cmhPHB_cpyPhbDateTime ( record,  &entry -> dateTime );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                 |
| STATE   : code                  ROUTINE : cmhPHB_invldEntr         |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to invalidate the elements of 
            a T_ACI_PB_ENTR structure.
*/
GLOBAL void cmhPHB_invldEntr ( T_ACI_PB_ENTR* entry )
{
  entry -> index     = ACI_NumParmNotPresent;
  entry -> number[0] = '\0';
  entry -> type.ton  = TON_Unknown;
  entry -> type.npi  = NPI_IsdnTelephony;
  entry -> text.len  = 0;  
  entry -> book      = PB_STOR_NotPresent;

  cmhPHB_invldCmhDateTime ( &entry -> dateTime);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                 |
| STATE   : code                  ROUTINE : cmhPHB_getTagNt          |
+--------------------------------------------------------------------+

  PURPOSE : This function converts the actual tag in the format used
            when storing it on the SIM into a null terminated string.
*/
GLOBAL void cmhPHB_getTagNt ( UBYTE* inTag,
                              UBYTE  maxInLen,
                              CHAR*  outTag, 
                              UBYTE  maxOutLen )
{
  UBYTE i = 0;

  while ( i < maxOutLen - 1 AND i < maxInLen AND 
          inTag[i] NEQ PHB_CMH_INVALID_TAG_ENTRY )
  {
    /*
     *-----------------------------------------------------------------
     * Convert from default GSM to internal GSM alphabet
     *-----------------------------------------------------------------
     */
    outTag[i] = ( CHAR ) inTag[i] | 0x80;
    i++;
  }

  outTag[i] = '\0';
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                 |
| STATE   : code                  ROUTINE : cmhPHB_getMfwTagNt       |
+--------------------------------------------------------------------+

  PURPOSE : This function copy the actual tag .
*/
GLOBAL void cmhPHB_getMfwTagNt ( const UBYTE* inTag,
                                 UBYTE  maxInLen,
                                 UBYTE* outTag, 
                                 UBYTE* maxOutLen )
{
  UBYTE len;

 if(maxInLen EQ 0)
 {
   memset(outTag, 0, MAX_ALPHA_LEN); /* we need to set out tag to 0 to prevent to be recognized by UCS2 decoder */
   *maxOutLen = 0;
   return;
 }

 len = pb_get_entry_len(inTag, maxInLen);

 if (len >= MAX_ALPHA_LEN)
 {
   len = MAX_ALPHA_LEN;
 }

 memcpy(outTag, inTag, len);
 *maxOutLen = len;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                 |
| STATE   : code                  ROUTINE : cmhPHB_getTagSim         |
+--------------------------------------------------------------------+

  PURPOSE : This function converts the actual tag as a null
            terminated string in the format used when storing it on 
            the SIM.
*/
GLOBAL void cmhPHB_getTagSim ( T_ACI_PB_TEXT  *inTag,
                               UBYTE          *outTag, 
                               UBYTE          maxOutLen )
{
  UBYTE inLen   = inTag->len;
  UBYTE i       = MINIMUM ( maxOutLen, inLen);
  SHORT restLen = maxOutLen - inLen;
  UBYTE j;

  /*
   *-----------------------------------------------------------------
   * Convert from internal GSM to default GSM alphabet
   *-----------------------------------------------------------------
   */
  for ( j = 0; j < i; j++ )
    outTag[j] = inTag->data[j] & 0x7F;
  
  if ( restLen > 0 )
    memset ( &outTag[i], PHB_CMH_INVALID_TAG_ENTRY, restLen);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                 |
| STATE   : code                  ROUTINE : cmhPHB_getMfwTagSim      |
+--------------------------------------------------------------------+

  PURPOSE : This function converts the actual tag string in the 
            format used when storing it on the SIM.
*/
GLOBAL void cmhPHB_getMfwTagSim ( T_ACI_PB_TEXT*  inTag,
                                  UBYTE*          outTag,
                                  UBYTE*          outTagLen,
                                  UBYTE           maxOutLen )
{
  UBYTE i       = MINIMUM ( maxOutLen, inTag->len);
  SHORT restLen = maxOutLen - inTag->len;
  UBYTE j;

  /*
   *-----------------------------------------------------------------
   * Convert from internal GSM to default GSM alphabet
   *-----------------------------------------------------------------
   */
  for ( j = 0; j < i; j++ )
    outTag[j] = inTag->data[j];
  
  *outTagLen = i;

  if ( restLen > 0 )
    memset ( &outTag[i], PHB_CMH_INVALID_TAG_ENTRY, restLen);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                |
| STATE   : code                  ROUTINE : cmhPHB_invldPhbDateTime |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to invalidate the date and time
            of a phonebook entry.
*/
GLOBAL void cmhPHB_invldPhbDateTime ( T_PHB_RECORD* record )
{
#ifdef TI_PS_FFS_PHB
  record->v_time      = FALSE;

  /* Backward compatibility. Setting v_time to FALSE would be sufficient */
  record->time.year   = PHB_CMH_YEAR_INVLD;
  record->time.month  = PHB_CMH_MONTH_INVLD;
  record->time.day    = PHB_CMH_DAY_INVLD;
  record->time.hour   = PHB_CMH_HOUR_INVLD;
  record->time.minute = PHB_CMH_MINUTE_INVLD;
  record->time.second = PHB_CMH_SECOND_INVLD;
#else
  record -> year   = PHB_CMH_YEAR_INVLD;
  record -> month  = PHB_CMH_MONTH_INVLD;
  record -> day    = PHB_CMH_DAY_INVLD;
  record -> hour   = PHB_CMH_HOUR_INVLD;
  record -> minute = PHB_CMH_MINUTE_INVLD;
  record -> second = PHB_CMH_SECOND_INVLD;
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                |
| STATE   : code                  ROUTINE : cmhPHB_invldCmhDateTime |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to invalidate the date and time
            of a phonebook entry.
*/
GLOBAL void cmhPHB_invldCmhDateTime ( T_ACI_VP_ABS* dateTime )
{
  dateTime -> year  [0] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> year  [1] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> month [0] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> month [1] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> day   [0] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> day   [1] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> hour  [0] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> hour  [1] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> minute[0] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> minute[1] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> second[0] = PHB_CMH_INVALID_BCD_ENTRY;
  dateTime -> second[1] = PHB_CMH_INVALID_BCD_ENTRY;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                |
| STATE   : code                  ROUTINE : cmhPHB_cpyCmhDateTime   |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to convert the date and time used
            within the command handler to the format used in the 
            phonebook management.
*/
GLOBAL void cmhPHB_cpyCmhDateTime ( const T_ACI_VP_ABS* dateTime,
                                    T_PHB_RECORD* record )
{
#ifdef TI_PS_FFS_PHB
  record->time.year   = (UBYTE)(dateTime -> year  [0] << 4) +
                                dateTime -> year  [1];
  record->time.month  = (UBYTE)(dateTime -> month [0] << 4) +
                                dateTime -> month [1];
  record->time.day    = (UBYTE)(dateTime -> day   [0] << 4) +
                                dateTime -> day   [1];
  record->time.hour   = (UBYTE)(dateTime -> hour  [0] << 4) +
                                dateTime -> hour  [1];
  record->time.minute = (UBYTE)(dateTime -> minute[0] << 4) +
                                dateTime -> minute[1];
  record->time.second = (UBYTE)(dateTime -> second[0] << 4) +
                                dateTime -> second[1];
  record->v_time = (record->time.year NEQ 0xff);
#else
  record -> year   = (UBYTE)(dateTime -> year  [0] << 4) +
                             dateTime -> year  [1];
  record -> month  = (UBYTE)(dateTime -> month [0] << 4) +
                             dateTime -> month [1];
  record -> day    = (UBYTE)(dateTime -> day   [0] << 4) +
                             dateTime -> day   [1];
  record -> hour   = (UBYTE)(dateTime -> hour  [0] << 4) +
                             dateTime -> hour  [1];
  record -> minute = (UBYTE)(dateTime -> minute[0] << 4) +
                             dateTime -> minute[1];
  record -> second = (UBYTE)(dateTime -> second[0] << 4) +
                             dateTime -> second[1];
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBF                |
| STATE   : code                  ROUTINE : cmhPHB_cpyPhbDateTime   |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to convert the date and time used
            within the phonebook management to the format used in
            the command handler.
*/
GLOBAL void cmhPHB_cpyPhbDateTime ( const T_PHB_RECORD*  record,
                                    T_ACI_VP_ABS*  dateTime )
{
#ifdef TI_PS_FFS_PHB
  dateTime -> year  [0] = ( record->time.year )   >> 4;
  dateTime -> year  [1] = record->time.year   & 0x0F;
  dateTime -> month [0] = ( record->time.month )  >> 4;
  dateTime -> month [1] = record->time.month  & 0x0F;
  dateTime -> day   [0] = ( record->time.day )    >> 4;
  dateTime -> day   [1] = record->time.day    & 0x0F;
  dateTime -> hour  [0] = ( record->time.hour )   >> 4;
  dateTime -> hour  [1] = record->time.hour   & 0x0F;
  dateTime -> minute[0] = ( record->time.minute ) >> 4;
  dateTime -> minute[1] = record->time.minute & 0x0F;
  dateTime -> second[0] = ( record->time.second ) >> 4;
  dateTime -> second[1] = record->time.second & 0x0F;
#else
  dateTime -> year  [0] = ( record -> year )   >> 4;
  dateTime -> year  [1] = record -> year   & 0x0F;
  dateTime -> month [0] = ( record -> month )  >> 4;
  dateTime -> month [1] = record -> month  & 0x0F;
  dateTime -> day   [0] = ( record -> day )    >> 4;
  dateTime -> day   [1] = record -> day    & 0x0F;
  dateTime -> hour  [0] = ( record -> hour )   >> 4;
  dateTime -> hour  [1] = record -> hour   & 0x0F;
  dateTime -> minute[0] = ( record -> minute ) >> 4;
  dateTime -> minute[1] = record -> minute & 0x0F;
  dateTime -> second[0] = ( record -> second ) >> 4;
  dateTime -> second[1] = record -> second & 0x0F;
#endif
}

/*==== EOF ========================================================*/
 
