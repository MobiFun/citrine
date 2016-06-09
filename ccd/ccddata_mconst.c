/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata_mconst.c
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
|  Purpose :  Ccddata abstraction for use in lib/dll. The four defines (MAX...
|             from mconst.cdg are supplied as constants. In addition,
|             corresponding functions deliver these constants.
+----------------------------------------------------------------------------- 
*/ 

#include "mconst.cdg"

#ifdef CCD_SYMBOLS
static int ccddata_ccdsymbols = 1;
#else /* CCD_SYMBOLS */
static int ccddata_ccdsymbols = 0;
#endif /* CCD_SYMBOLS */

const int ccddata_num_of_entities = NUM_OF_ENTITIES;
const int ccddata_max_message_id = MAX_MESSAGE_ID;
const int ccddata_max_bitstream_len = MAX_BITSTREAM_LEN;
const int ccddata_max_mstruct_len = MAX_MSTRUCT_LEN;

int ccddata_get_num_of_entities ()
{
  return ccddata_num_of_entities;
}

int ccddata_get_max_message_id ()
{
  return ccddata_max_message_id;
}

int ccddata_get_max_bitstream_len ()
{
   return ccddata_max_bitstream_len;
}

int ccddata_get_max_mstruct_len ()
{
  return ccddata_max_mstruct_len;
}

int ccddata_mccd_symbols ()
{
  return ccddata_ccdsymbols;
}
