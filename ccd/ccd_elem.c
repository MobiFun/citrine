/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccd_elem.c
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
|  Purpose :  Condat Conder Decoder - 
|             Definition of encoding and decoding functions of 
|             information elements of air interface messages
+----------------------------------------------------------------------------- 
*/ 

#define CCD_ELEM_C

#include <stdio.h>
#include <string.h>

#include "typedefs.h"
#include "ccd_globs.h"
#include "ccd.h"
#include "ccdtable.h"
#include "ccddata.h"
#include "ccdapi.h"
#include "bitfun.h"

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : ccd_elem              |
| STATE   : code                     ROUTINE : ccd_decodeElem        |
+--------------------------------------------------------------------+

 PARAMETERS:   ULONG  ccdid
                - Enumeration of the information element to be 
                  decoded in the file ccdid.h. This number is also
                  the reference number of the IE in the melem table.

               USHORT l_buf
                - Number of bits in the encoded IE.

               USHORT o_buf
                - Offset of the bitstream buffer in bits.

               U8 *buf
                - Bitstream buffer of the encoded IE.

               U8 *eStruct
                - reference to the C-Structure containing the
                  C-Representation of the decoded IE.

 PURPOSE:      decodes a bitstream containing an encoded information
               element. The results are written to a corresponding 
               C-Structure, the C-Representation of the IE.
*/
int CCDDATA_PREF(ccd_decodeElem) (ULONG  ccdid,
                                   USHORT  l_buf,
                                   USHORT  o_buf,
                                   UCHAR*  buf,
                                   UCHAR*  eStruct)
{
  int    jmp_ret;
  USHORT mcompRef;
  T_CCD_Globs *globs;
  T_CCD_ERR_LIST_HEAD* eentry;
  T_CCD_STORE_LIST* stoentry;

  globs = ccd_GetGlobVars (&eentry, &stoentry);

#ifdef DEBUG_CCD
  ccd_dump_msg(l_buf, o_buf, buf, globs);
#endif

  /*
   * setup the structure-buffer. */
  globs->pstruct = eStruct;
  globs->pstructOffs = 0;

  ccd_common_decode_init(l_buf, o_buf, buf, globs);
  ccd_err_reset (eentry);
  globs->ccd_recurs_level =1;

  if ((mcompRef = melem[ccdid].elemRef) EQ NO_REF)
  {
    ccd_recordFault (globs, ERR_INVALID_CCDID, BREAK, ccdid, NULL);
    ccd_FreeGlobVars (globs);
    ccd_err_free (eentry);
    return (BYTE)globs->CCD_Error;
  }


#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
  TRACE_CCD (globs, "CCD decode: Element = %s",
  mcomp[mcompRef].name);
#else
  TRACE_CCD (globs, "CCD decode: CCD_Id = %x", ccdid);
#endif
#endif

#ifdef ERR_TRC_STK_CCD
    /* save the value for tracing in error case */
    globs->error_stack[0] = mcompRef;
#endif /* ERR_TRC_STK_CCD */

/* 
 * Clean up the entite C-structure before decoding.
 * Do not overwrite the MsgId (1. Byte)
 */
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "CCD Cleaning struct %ld bytes",
             mcomp[mcompRef].cSize);
#endif
  memset ((UBYTE *) globs->pstruct, 0,
          (size_t)(mcomp[mcompRef].cSize));

  /*
   * clear the UPN stack
   */
  globs->SP=0;
  globs->StackOvfl=FALSE;
  globs->KeepReg[0] = 0;

  /*
   * inform the GSM-CODEC about the begin of a new message
   */
  cdc_GSM_start (globs);

  jmp_ret = setjmp (globs->jmp_mark);

  if (jmp_ret EQ 0)
  {
    globs->jmp_mark_set = TRUE;
    ccd_decodeComposition ((ULONG) mcompRef, globs);
  }

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "CCD-ERROR = %d", globs->CCD_Error);
  TRACE_CCD (globs, "-------------------------------------------------");
#endif /* DEBUG_CCD */

  ccd_FreeGlobVars (globs);
  ccd_err_free (eentry);

  return (BYTE) globs->CCD_Error;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : ccd_elem              |
| STATE   : code                     ROUTINE : ccd_encodeElem        |
+--------------------------------------------------------------------+

 PARAMETERS:   ULONG  ccdid
                - Enumeration of the information element to be 
                  encoded in the file ccdid.h. This number is also 
                  the reference number of the IE in the melem table

               USHORT l_buf
                - Number of bits in the encoded IE.

               USHORT o_buf
                - Offset of the bitstream buffer in bits.

               U8 *buf
                - Bitstream buffer of the encoded IE.

               UBYTE    * eStruct
                - reference to the C-Structure containing the
                  C-Representation of the decoded IE.

 PURPOSE:      encodes a C-Structure containing the C-Representation of
               an information element to a bitstream.
*/
int CCDDATA_PREF(ccd_encodeElem) (ULONG  ccdid,
                                    USHORT* l_buf,
                                    USHORT  o_buf,
                                    UCHAR*  buf,
                                    UCHAR*  eStruct)
{
  int    jmp_ret;
  USHORT maxBytes, mcompRef;
  T_CCD_Globs *globs;
  T_CCD_ERR_LIST_HEAD* eentry;
  T_CCD_STORE_LIST* stoentry;

  globs = ccd_GetGlobVars (&eentry, &stoentry);
  ccd_err_reset (eentry);

#ifdef DEBUG_CCD
  {
    /* to avoid the vsprintf if the traces won't appear anyhow */
    ULONG mask;
    if (vsi_gettracemask (globs->me, globs->me, &mask) != VSI_ERROR)
    {
      globs->TraceIt = mask & TC_CCD;
    }
  }
#endif
  
  /*
   * Set a sign that no call to setjmp() is done. So ccd_setError 
   * performs no longjmp in case of an error.
   */
  globs->jmp_mark_set = FALSE;

  /* Setup the bitbuffer. */
  globs->bitbuf = buf;
  globs->bitpos = 0;

  /* Setup the structure-buffer. */
  globs->pstruct = eStruct;
  globs->pstructOffs = 0;

  /* Cleanup the read-caches. */
  globs->lastbytepos16 = globs->lastbytepos32 = 0xffff;

  /* Setup the bitoffset. */
  globs->bitoffs = o_buf;
  bf_incBitpos (o_buf, globs);
  globs->bitbuf[globs->bytepos] = 0;

  globs->CCD_Error = ccdOK;
  globs->ccd_recurs_level =1;

  if ((mcompRef = melem[ccdid].elemRef) EQ NO_REF)
  {
    ccd_recordFault (globs, ERR_INVALID_CCDID, BREAK, ccdid, NULL);
    ccd_FreeGlobVars (globs);
    ccd_err_free (eentry);
    return (BYTE)globs->CCD_Error;
  }

#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
  TRACE_CCD (globs, "CCD encode: Element = %s",
  mcomp[mcompRef].name);
#else
  TRACE_CCD (globs, "CCD encode: CCD_Id = %x", ccdid);
#endif
#endif

#ifdef ERR_TRC_STK_CCD
  /* Save the value for tracing in error case. */
  globs->error_stack[0] = mcompRef;
#endif
 
  maxBytes = (USHORT) (mcomp[mcompRef].bSize+7)>>3;
  #ifdef DEBUG_CCD
  TRACE_CCD (globs, "-------------------------------------------------");
  TRACE_CCD (globs, "CCD: Code Elem");
  TRACE_CCD (globs, "Cleaning %d bits (%d bytes) of the bitstream",
    mcomp[mcompRef].bSize, maxBytes);
#endif

  /* 
   * Clean up the bit buffer for the encoded message before encoding.
   */
  memset ((U8 *) &buf[o_buf>>3], 0, (size_t) maxBytes);

  /* Store the length of ereased buffer to support error handling. */
  globs->buflen = (USHORT) mcomp[mcompRef].bSize;

  /*
   * Clear the UPN stack.
   */
  globs->SP=0;
  globs->StackOvfl=FALSE;
  globs->KeepReg[0] = 0;

  /*
   * Inform the GSM-CODEC about the begin of a new message.
   */
  cdc_GSM_start (globs);

  jmp_ret = setjmp (globs->jmp_mark);

  if (jmp_ret EQ 0)
  {
    globs->jmp_mark_set = TRUE;
    ccd_encodeComposition ((ULONG) mcompRef, globs);
  }

  *l_buf = (USHORT)(globs->bitpos - globs->bitoffs);

#ifdef DEBUG_CCD
{
  int i, j, buflen;
  char s[64], c[4];  

  buflen = (*l_buf + o_buf + 7) >> 3;

  TRACE_CCD (globs, "-------------------------------------------------");
  TRACE_CCD (globs, " After ENCODING: lbuf= %d, obuf= %d", *l_buf, o_buf);
  TRACE_CCD (globs, " Hex dump of encoded message:"); 

  s[0] = '\0';
  for (i = o_buf >> 3; i < buflen; i+=16)
  {
    for (j = 0; j < 16; j++)
    {
      if ((i+j) < buflen)
      {
        sprintf(c, " %02x", buf[i+j]);
        strcat (s, c);
      }
    }
    TRACE_CCD (globs, "%s", s);
    s[0] = '\0';
  }
}
#endif

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "CCD-ERROR = %d", globs->CCD_Error);
  TRACE_CCD (globs, "-------------------------------------------------");
#endif /* DEBUG_CCD */

  ccd_FreeGlobVars (globs);
  ccd_err_free (eentry);

  return (BYTE) globs->CCD_Error;
}
#endif /* !RUN_INT_RAM */
