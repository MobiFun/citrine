/* plmn_decoder.cpp : Defines the entry point for the console application. */

#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef TI_PS_OP_OPN_TAB_ROMBASED
#ifndef CMH_PLMN_DECODER_C
#define CMH_PLMN_DECODER_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_mem.h"
#include "pcm.h"

#ifdef TI_PS_OP_OPN_TAB_ROMBASED
  #include "rom_tables.h"
#endif /* TI_PS_OP_OPN_TAB_ROMBASED */


#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_mm.h"
#include "psa_sim.h"
#include "psa_util.h"
#include "cmh.h"



#define PLMN_LONGNAME_LOOKUP 0xff

EXTERN void  cmhSIM_getMncMccFrmPLMNsel( const UBYTE* ntry, 
                                                         SHORT* mcc, 
                                                         SHORT* mnc );

/* NHK:
 * Once the tables plmn_compressed, plmn_dict_offset and plmn_dict go to ROM, their names below should match corresponding ROM addresses
 * KSR: Moved to g23m\condat\com\src\drivers\rom_tables.c
 */

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : cmhMM_CompLongLength               |
+----------------------------------------------------------------------------+

  PURPOSE : This function gets the compressed length of the longName
            from a compressed PLMN entry.
  
*/
LOCAL USHORT cmhMM_CompLongLength (const UBYTE *plmn_comp_entry)
{
  UBYTE length_byte;

  length_byte = *(plmn_comp_entry + 3);

  return ((length_byte >> 3) & 0x1f) + 1;
}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : cmhMM_CompShrtLength               |
+----------------------------------------------------------------------------+

  PURPOSE : This function gets the compressed length of the shrtName
            from a compressed PLMN entry.
  
*/
LOCAL USHORT cmhMM_CompShrtLength (const UBYTE *plmn_comp_entry)
{
  UBYTE length_byte;

  length_byte = *(plmn_comp_entry + 3);

  return ((length_byte     ) & 0x07) + 1;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : cmhMM_PlmnEntryLength              |
+----------------------------------------------------------------------------+

  PURPOSE : This function gets the length of a compressed PLMN entry.
  
*/
GLOBAL USHORT cmhMM_PlmnEntryLength (const UBYTE *plmn_comp_entry)
{
  return (4 + cmhMM_CompLongLength (plmn_comp_entry) 
            + cmhMM_CompShrtLength (plmn_comp_entry));
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : cmhMM_Decompress                   |
+----------------------------------------------------------------------------+

  PURPOSE : This function decompresses a compressed string.
            If the decompressed string is the long PLMN name the longName
            parameter is NULL, when the decompressed string is the short
            PLMN name the longName parameter is the previously decompressed
            PLMN long name, as in some cases the short PLMN name consists of
            parts of the long PLMN name lookups within the decompressed long
            name will be done when advised to do so by the compressed byte 
            string.
            On success the function returns with zero, on detection of an
            internal error a negative value is returned.
 
*/
LOCAL SHORT cmhMM_Decompress (char *decomp_string, 
                              const char *longName,
                              const UBYTE *comp_string,
                              USHORT comp_length)
{
  USHORT offset;
  USHORT length;

  while (comp_length > 0)
  {
    if (*comp_string < 128)
    {
      /* GSM default alphabet. Take character 1:1 */
      *decomp_string = *comp_string;
      decomp_string++;
    }
    else if (*comp_string EQ PLMN_LONGNAME_LOOKUP)
    {
      if ((longName EQ NULL) OR (comp_length <= 1))
      {
        TRACE_ERROR ("Cannot process longName lookup");
        return -1;
      }

      /* Decoding short name. Lookup into long name */

      /* Skip PLMN_LONGNAME_LOOKUP */
      comp_string++;
      comp_length--;

      /* Get offset and length of substring in long name */
      offset = (*comp_string >> 3) & 0x1f;
      length = (*comp_string & 0x07) + 1;
      memcpy (decomp_string, longName + offset, length);
      decomp_string += length;
    }
    else
    {
      /* Dictionary lookup */

      /* Get offset of dictionary entry */
      offset = *(ptr_plmn_dict_offset + (*comp_string - 128)); 

      /* Get length of dictionary entry */
      length = *(ptr_plmn_dict_offset + (*comp_string - 128 + 1)) - offset;
      memcpy (decomp_string, ptr_plmn_dict + offset, length);
      decomp_string += length;
    }
    comp_string++;
    comp_length--;
  }
  decomp_string = '\0';
  return 0;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : cmhMM_decodePlmn                   |
+----------------------------------------------------------------------------+

  PURPOSE : This function decodes a compressed PLMN entry into a decompressed
            T_OPER_ENTRY structure. On success this function returns with 
            zero, if during decompression an internal error is detected the
            function returns with a negative value.
 
*/
GLOBAL SHORT cmhMM_decodePlmn (T_OPER_ENTRY *oper, const UBYTE *plmn_comp_entry)
{
  SHORT mcc;              /* Mobile Country Code */
  SHORT mnc;              /* Mobile Network Code */
  USHORT comp_Length;     /* Compressed string length */
  SHORT result;           /* Decompression result */
  const UBYTE *comp_ptr;  /* Compressed PLMNs pointer */

  memset (oper, 0, sizeof (T_OPER_ENTRY));

  cmhSIM_getMncMccFrmPLMNsel (plmn_comp_entry, &mcc, &mnc);
  oper->mcc = mcc;
  oper->mnc = mnc;

  if (mcc EQ -1)
    return -1; /* Got end of list, not processing further */

  /* Decompress longName */
  comp_ptr = plmn_comp_entry + 4;
  comp_Length = cmhMM_CompLongLength (plmn_comp_entry);
  result = cmhMM_Decompress (oper->longName, NULL,
                             comp_ptr, comp_Length);
  if (result < 0)
    return result;

  /* Decompress shrtName */
  comp_ptr += comp_Length;
  comp_Length = cmhMM_CompShrtLength (plmn_comp_entry);
  result = cmhMM_Decompress (oper->shrtName, oper->longName, 
                             comp_ptr, comp_Length);
  return result;
}

#endif
