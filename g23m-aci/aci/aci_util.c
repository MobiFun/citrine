/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  ACI_UTIL
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
|  Purpose :  This module defines the utility functions for the AT
|             command interpreter.
+----------------------------------------------------------------------------- 
*/ 



#ifndef ACI_UTIL_C
#define ACI_UTIL_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_cmh.h" 
#include "ati_cmd.h"  

#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_sms.h"
#include "phb.h"
#include "cmh.h"
#include "cmh_sms.h"

#if defined (FF_ATI) || defined (FF_BAT)
#include "aci_mem.h"
#include "aci_lst.h"
#include "aci_io.h"
#include "ksd.h"
#include "cmh_ss.h"
#include "psa_ss.h"
#endif
#ifdef FF_ATI
#include "ati_int.h"
#endif
#ifdef FF_BAT
#include "aci_bat_cb.h"
#endif

#ifdef GPRS
#include "gaci_cmh.h"
#endif /* GPRS */

#ifdef TI_PS_OP_CNV_TAB_ROMBASED
  #include "rom_tables.h"
#endif /* TI_PS_OP_CNV_TAB_ROMBASED */


#ifdef TI_PS_OP_CNV_TAB_ROMBASED
    /* If this flag is enabled CSCS_CHSET_Chars is defined in rom_tables.h file */
#else
  #define CSCS_CHSET_Chars  256
#endif

/*==== CONSTANTS ==================================================*/
#ifdef REL99
#define NUM_EXT_TABLE_CHARS 9
#define NUM_IRA_8859_TABLE_CHARS 1
#endif


/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/
/* This fucntion cuts the pathname from the file
   used by ACI_ASSERT makro*/
GLOBAL char * getFileName(char * file)
{
  char *cursor = file;
  do {
    #ifdef _SIMULATION_
    if(*cursor EQ '\\') { file = cursor+1;}
    #else
    if(*cursor EQ '/') { file = cursor+1;}
    #endif
    cursor++;
  } while(*cursor NEQ '\0');
  return file;
}


LOCAL USHORT utl_ucs2FromGsm ( UBYTE*           in,
                              USHORT           inLen,
                              UBYTE*           out,
                              USHORT           maxOutLen,
                              USHORT*          outLen,
                              T_ACI_GSM_ALPHA  gsm,
                              T_ACI_CSCS_ALPHA alphabet );

/*==== VARIABLES ==================================================*/
#ifdef TI_PS_OP_CNV_TAB_ROMBASED
/* Changes for ROM data */
/* NHK:
 * Once the tables chset and gsmToAsciiTable  go to ROM, their names below should match corresponding ROM addresses
 * KSR: Moved to g23m\condat\com\src\drivers\rom_tables
 */
#else
GLOBAL const UBYTE chset [CSCS_CHSET_Tables][CSCS_CHSET_Chars] =
{
/*
 *-------------------------------------------------------------------
 * Conversion table: IRA -> internal GSM
 *-------------------------------------------------------------------
 */
/* 0x00 */  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x08 */    0x00, 0x00, 0x8A, 0x00, 0x00, 0x8D, 0x00, 0x00,
/* 0x10 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x18 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x20 */    0xA0, 0xA1, 0xA2, 0xA3, 0x82, 0xA5, 0xA6, 0xA7,
/* 0x28 */    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
/* 0x30 */    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
/* 0x38 */    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
/* 0x40 */    0x80, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
/* 0x48 */    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
/* 0x50 */    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
/* 0x58 */    0xD8, 0xD9, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x91/*00*/,
/* 0x60 */    0x00, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
/* 0x68 */    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
/* 0x70 */    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
/* 0x78 */    0xF8, 0xF9, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x80 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x88 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x90 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x98 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xA0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xA8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xB0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xB8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xC0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xC8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xD0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xD8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xE0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xE8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xF0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xF8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },

/*
 *-------------------------------------------------------------------
 * Conversion table: PC Danish/Norwegian -> internal GSM
 *-------------------------------------------------------------------
 */
/* 0x00 */  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x08 */    0x00, 0x00, 0x8A, 0x00, 0x00, 0x8D, 0x00, 0x00,
/* 0x10 */    0x00, 0x00, 0x00, 0x00, 0x00, 0xDF, 0x00, 0x00,
/* 0x18 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x20 */    0xA0, 0xA1, 0xA2, 0xA3, 0x82, 0xA5, 0xA6, 0xA7,
/* 0x28 */    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
/* 0x30 */    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
/* 0x38 */    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
/* 0x40 */    0x80, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
/* 0x48 */    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
/* 0x50 */    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
/* 0x58 */    0xD8, 0xD9, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x91/*00*/,
/* 0x60 */    0x00, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
/* 0x68 */    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
/* 0x70 */    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
/* 0x78 */    0xF8, 0xF9, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x80 */    0x89, 0xFE, 0x85, 0xE1, 0xFB, 0xFF, 0x8F, 0x89,
/* 0x88 */    0xE5, 0xE5, 0x84, 0xE9, 0xE9, 0x87, 0xDB, 0x8E,
/* 0x90 */    0x9F/*C5*/, 0x9D, 0x9C, 0xEF, 0xFC, 0x88, 0xF5/*E5*/, 0x86,
/* 0x98 */    0xF9/*E9*/, 0xDC, 0xDE, 0x8C, 0x81, 0x8B, 0x00, 0x00,
/* 0xA0 */    0xE1, 0xE9, 0xEF, 0xF5, 0xFD, 0xDD, 0x00, 0x00,
/* 0xA8 */    0xE0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00,
/* 0xB0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xB8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xC0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xC8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xD0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xD8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xE0 */    0x00, 0x9E, 0x93, 0x00, 0x98, 0x00, 0x00, 0x00,
/* 0xE8 */    0x92, 0x99, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xF0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xF8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },

/*
 *-------------------------------------------------------------------
 * Conversion table: ISO 8859 Latin 1 -> internal GSM
 *-------------------------------------------------------------------
 */
/* 0x00 */  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x08 */    0x00, 0x00, 0x8A, 0x00, 0x00, 0x8D, 0x00, 0x00,
/* 0x10 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x18 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x20 */    0xA0, 0xA1, 0xA2, 0xA3, 0x82, 0xA5, 0xA6, 0xA7,
/* 0x28 */    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
/* 0x30 */    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
/* 0x38 */    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
/* 0x40 */    0x80, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
/* 0x48 */    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
/* 0x50 */    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
/* 0x58 */    0xD8, 0xD9, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x91/*00*/,
/* 0x60 */    0x00, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
/* 0x68 */    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
/* 0x70 */    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
/* 0x78 */    0xF8, 0xF9, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x80 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x88 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x90 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x98 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xA0 */    0x00, 0xC0, 0x00, 0x81, 0xA4, 0x83, 0x00, 0xDF,
/* 0xA8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xB0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xB8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0,
/* 0xC0 */    0xC1, 0xC1, 0xC1, 0xC1, 0xDB, 0x8E, 0x9C, 0x89,
/* 0xC8 */    0xC5, 0x9F/*C5*/, 0xC5, 0xC5, 0xC9, 0xC9, 0xC9, 0xC9,
/* 0xD0 */    0x00, 0xDD, 0xCF, 0xCF, 0xCF, 0xCF, 0xDC, 0x00,
/* 0xD8 */    0x8B, 0xD5, 0xD5, 0xD5, 0xDE, 0xD9, 0x00, 0x9E,
/* 0xE0 */    0xFF, 0xE1, 0xE1, 0xE1, 0xFB, 0x8F, 0x9D, 0x89,
/* 0xE8 */    0x84, 0x85, 0xE5, 0xE5, 0x87, 0xE9, 0xE9, 0xE9,
/* 0xF0 */    0x00, 0xFD, 0x88, 0xEF, 0xEF, 0xEF, 0xFC, 0x00,
/* 0xF8 */    0x8C, 0x86, 0xF5, 0xF5, 0xFE, 0xF9, 0x00, 0xF9 },

/*
 *-------------------------------------------------------------------
 * Conversion table: PC Code Page 437 -> internal GSM
 *-------------------------------------------------------------------
 */
/* 0x00 */  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x08 */    0x00, 0x00, 0x8A, 0x00, 0x00, 0x8D, 0x00, 0x00,
/* 0x10 */    0x00, 0x00, 0x00, 0x00, 0x00, 0xDF, 0x00, 0x00,
/* 0x18 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x20 */    0xA0, 0xA1, 0xA2, 0xA3, 0x82, 0xA5, 0xA6, 0xA7,
/* 0x28 */    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
/* 0x30 */    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
/* 0x38 */    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
/* 0x40 */    0x80, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
/* 0x48 */    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
/* 0x50 */    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
/* 0x58 */    0xD8, 0xD9, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x91/*00*/,
/* 0x60 */    0x00, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
/* 0x68 */    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
/* 0x70 */    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
/* 0x78 */    0xF8, 0xF9, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x80 */    0x89, 0xFE, 0x85, 0xE1, 0xFB, 0xFF, 0x8F, 0x89,
/* 0x88 */    0xE5, 0xE5, 0x84, 0xE9, 0xE9, 0x87, 0xDB, 0x8E,
/* 0x90 */    0x9F/*C5*/, 0x9D, 0x9C, 0xEF, 0xFC, 0x88, 0xF5/*E5*/, 0x86,
/* 0x98 */    0xF9/*E9*/, 0xDC, 0xDE, 0x00, 0x81, 0x83, 0x00, 0x00,
/* 0xA0 */    0xE1, 0xE9, 0xEF, 0xF5, 0xFD, 0xDD, 0x00, 0x00,
/* 0xA8 */    0xE0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00,
/* 0xB0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xB8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xC0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xC8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xD0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xD8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xE0 */    0x00, 0x9E, 0x93, 0x00, 0x98, 0x00, 0x00, 0x00,
/* 0xE8 */    0x92, 0x99, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xF0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xF8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },

/*
 *-------------------------------------------------------------------
 * Conversion table: GSM -> internal GSM
 *-------------------------------------------------------------------
 */
/* 0x00 */  { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
/* 0x08 */    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
/* 0x10 */    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
/* 0x18 */    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
/* 0x20 */    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
/* 0x28 */    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
/* 0x30 */    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
/* 0x38 */    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
/* 0x40 */    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
/* 0x48 */    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
/* 0x50 */    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
/* 0x58 */    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
/* 0x60 */    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
/* 0x68 */    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
/* 0x70 */    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
/* 0x78 */    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
/* 0x80 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x88 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x90 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x98 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xA0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xA8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xB0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xB8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xC0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xC8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xD0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xD8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xE0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xE8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xF0 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xF8 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};


/*
 *-------------------------------------------------------------------
 * Conversion table: ASCII <-> internal GSM
 * ( needed for the conversion of UCS2 <-> internal GSM )
 *-------------------------------------------------------------------
 */
/* GSM alphabet characters unknown in the ASCII table have been 
   replaced by <SP> characters */
LOCAL const UBYTE gsmToAsciiTable[128] =
{   
    /*n =     0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
/* 0x0n */   64,163, 36,165,232,233,249,236,242,199, 10,216,248, 13,197,229,
/* 0x1n */  128, 95,129,130,131,132,133,134,135,136,137, 27,198,230,223,201,
/* 0x2n */   32, 33, 34, 35,164, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
/* 0x3n */   48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
/* 0x4n */  161, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
/* 0x5n */   80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,196,214,209,220,167,
/* 0x6n */  191, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
/* 0x7n */  112,113,114,115,116,117,118,119,120,121,122,228,246,241,252,224
};

LOCAL const UBYTE  hexVal[] = {"0123456789ABCDEF"};

#endif

/*
 * These tables are moved out of TI_PS_OP_CNV_TAB_ROMBASED
 */

#ifdef REL99
/*
 *-------------------------------------------------------------------
 * Conversion table: Extension character table that can be used for 
                     mapping IRA or PC D/N or ISO 8859 Latin 1 or PCCP 437
                     characters to GSM 7 extension table characters
 * The first column contains the GSM 7 extension table character value and 
 * the second column contains the corresponding character in the other 
 * character sets 
 * Ref: 23.038 Sec 6.2.1.1
 *-------------------------------------------------------------------
 */
GLOBAL const UBYTE ext_common_chset_map [NUM_EXT_TABLE_CHARS][2] =
{
  { 0x14, 0x5E }, /* ^  */
  { 0x1B, 0x20 }, /* Reserved for another extension table. To be replaced by space */
  { 0x28, 0x7B }, /* { */
  { 0x29, 0x7D }, /* } */
  { 0x2F, 0x5C }, /* \ */
  { 0x3C, 0x5B }, /* [ */
  { 0x3D, 0x7E }, /* ~ */
  { 0x3E, 0x5D }, /* ] */
  { 0x40, 0x7C }  /* | */  
};

/*
 *-------------------------------------------------------------------
 * Conversion table: Extension table character that is used only for 
   IRA OR ISO 8859 Latin 1, in addition to the character table given 
   above. 
 * Ref: 23.038 Sec 6.2.1.1
 * The Euro character is treated separately as it there is no mapping 
 * character in PCCP 437 and PCDN.  
 *-------------------------------------------------------------------
 */  
GLOBAL const UBYTE ext_ira_8859_chset_map [NUM_IRA_8859_TABLE_CHARS][2] =
{
  { 0x65, 0x80 }  /* Euro symbol */
};
#endif


/*
static const unsigned char
gsm_2_ascii_table[128] = 
{
  0x40, 0x9C, 0x24, 0x9D, 0x8A, 0x82, 0x97, 0x8D, 0x95, 0x80, 0x0A, 0x02, 0x07, 0x0D, 0x8F, 0x86,
  0x04, 0x5F, 0xE8, 0xE2, 0xEF, 0xEA, 0xE3, 0x05, 0xE6, 0xE9, 0xF0, 0x20, 0x92, 0x91, 0xE1, 0x90, 
  0x20, 0x21, 0x22, 0x23, 0x01, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0xAD, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x8E, 0x99, 0xA5, 0x9A, 0x06,
  0xA8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x84, 0x94, 0xA4, 0x81, 0x85
};
*/

/*==== FUNCTIONS ==================================================*/
#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : strupper                     |
+-------------------------------------------------------------------+

  PURPOSE : Converts all characters from 'a' to 'z' to capital
            characters from 'A' to 'Z'.
*/
GLOBAL char* strupper ( char* s )
{
  USHORT i = 0;

  while ( s NEQ 0 AND s[i] NEQ '\0')
  {
    if ( s[i] >= 0x61 AND s[i] <= 0x7a )

      s[i] = s[i] - 0x20;

    i++;
  }

  return s;
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_chsetToSim               |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of a string from
            the character set used within the AT command interpreter
            to the SIM GSM character set.
*/
GLOBAL void utl_chsetToSim ( UBYTE*          in,
                             USHORT          inLen,
                             UBYTE*          out,
                             USHORT*         outLen,
                             T_ACI_GSM_ALPHA gsm )
{
  UBYTE  cvtdVal;
  USHORT outIdx   = 0;
  USHORT inIdx;
  UBYTE  corr     = ( gsm EQ GSM_ALPHA_Int ? 0xFF : 0x7F );
  T_ACI_CSCS_CHSET cscsChset;
  UBYTE  srcId = srcId_cb;

  cscsChset = ati_user_output_cfg[srcId].cscsChset;
  
  if ( cscsChset EQ CSCS_CHSET_Hex )
  {
    utl_hexToGsm ( in, inLen, out, outLen, gsm, CSCS_ALPHA_8_Bit );
  }
  else if ( cscsChset EQ CSCS_CHSET_Ucs2 )
  {
    /* If the user chooses UCS2, then the best mode of coding should be 
     * used to store the characters in the SIM */
    utl_ucs2ToSim(in, inLen, out, outLen, gsm, CSCS_ALPHA_8_Bit);
  }
  else
  {
    for ( inIdx = 0; inIdx < inLen; inIdx++ )
    {
      cvtdVal = chset[cscsChset][( UBYTE ) in[inIdx]]; 
        
      if ( cvtdVal NEQ 0x00 )
      {
        out[outIdx] = ( CHAR ) cvtdVal & corr;
        outIdx++;
      }
    }

    if ( gsm EQ GSM_ALPHA_Int )
      out[outIdx] = '\0';

    *outLen = outIdx;
  }
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_codeUcs2                 |
+-------------------------------------------------------------------+

  PURPOSE : This function finds optimal coding format to store the 
            UCS2 character string in the phone book.
*/
GLOBAL void utl_ucs2ToSim( UBYTE*          in,
                           USHORT          inLen,
                           UBYTE*          out,
                           USHORT*         outLen,
                           T_ACI_GSM_ALPHA gsm,
                           T_ACI_CSCS_ALPHA alphabet)
{
  int    i;
  BOOL   flag = TRUE;
  UBYTE  hexOut[MAX_ALPHA_LEN * 4];
  USHORT hexOutLen = 0;

  /* Convert the hex character string to actual hex values */
  hexOutLen = utl_HexStrToBin(in, inLen, hexOut, (MAX_ALPHA_LEN * 4));

  /* Initial check is done for GSM or ASCII only characters */
  for(i = 0; i < (hexOutLen/2); i++)
  {
    if(( hexOut[i*2] NEQ 0x00 ))
    {
      flag = FALSE;
      break;
    }
  }
  if (flag EQ TRUE)
  {
    utl_ConvUcs2ToGSM(hexOut, hexOutLen, out, outLen, gsm, alphabet);
    if (*outLen)
      return;
  }

  /* If possible UCS2 character string is coded in 0x81 format which
     uses a one byte base pointer */
  utl_Ucs2InFormat1 (hexOut, hexOutLen, out, outLen);
  if (*outLen)
    return;

  /* If possible UCS2 character string is coded in 0x82 format which
     uses two byte base pointer */
  utl_Ucs2InFormat2 (hexOut, hexOutLen, out, outLen);
  if (*outLen)
    return;

  /* If none of the above work, UCS2 character string is coded in 0x80 */
  *out = 0x80;
  utl_hexToGsm ( in, inLen, out + 1, outLen, gsm, CSCS_ALPHA_8_Bit );
  (*outLen)++;
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_ConvUcs2ToGSM            |
+-------------------------------------------------------------------+

  PURPOSE : This function converts UCS2 charecter string consisting of 
            only ASCII charecters to default GSM values.
*/
GLOBAL void utl_ConvUcs2ToGSM ( UBYTE*           in,
                                USHORT           inLen,
                                UBYTE*           out,
                                USHORT*          outLen,
                                T_ACI_GSM_ALPHA  gsm,
                                T_ACI_CSCS_ALPHA alphabet )
{
  int i, j;
  BOOL   flag;
  UBYTE  val = 0x00;
  USHORT len;
  UBYTE  tmpOut;
  USHORT outIdx = 0;
  UBYTE  corr   = ( gsm EQ GSM_ALPHA_Int ? 0xFF : 0x7F );

  len = inLen/2;
  for (i = 0;i < len; i++)
  {
    tmpOut = in[(i*2) + 1];    
    /* convert the hexadecimal ASCII value to GSM, if the value is
     * not convertible then we exit */
    if ( tmpOut EQ ( gsmToAsciiTable[tmpOut])) /* ASCII and GSM are identical */
    {
      val = tmpOut;
    }
    else /* find match in the table and copy index of match */
    {
      flag = FALSE;
      for (j=0; j<128; j++)
      {
        if (tmpOut EQ gsmToAsciiTable[j])
        {
          val = (UBYTE)j;
          flag = TRUE;
          break;
        }
#ifdef REL99
        else
        {
          /* There is a chance that this is in the mapping table */
          UBYTE ext_tbl_idx = 0;        
          while( ext_tbl_idx < NUM_EXT_TABLE_CHARS              AND 
                 ext_common_chset_map[ext_tbl_idx][1] NEQ tmpOut )
          { 
            ext_tbl_idx++;
          }
          if(ext_tbl_idx < NUM_EXT_TABLE_CHARS)
          {
            /*Insert the escape character, 0x1B, to output stream*/
            out[outIdx++] = 0x1B |(corr &0x80);
            /* Insert the mapping character into the output stream */
            val = ( UBYTE )ext_common_chset_map[ext_tbl_idx][0] |(corr &0x80);
            flag = TRUE;
            break;
          }
        }
#endif /* REL99 */
      }
      if (!flag)
      {
        *outLen = 0;
        return;
      }
    }      
    
    /* if necessary, cut the GSM value to 7bit */
    if ( alphabet EQ CSCS_ALPHA_7_Bit AND !( val & 0x80 ) )
    {
      if ( gsm EQ GSM_ALPHA_Int )
      {
        out[outIdx++] = val | 0x80; 
      }
      else
      {
        out[outIdx++] = val;
      }
    }
    else
    {
      out[outIdx++] = val;
    }
  }
      
  if ( gsm EQ GSM_ALPHA_Int )
  {
    out[outIdx] = '\0';
  }

  /* set the outlength */
  *outLen = outIdx;
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_Ucs2InFormat1            |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the UCS2 character string in the 
            0x81 coding scheme for UCS2.
            Octet 1: 0x81
            Octet 2: Num of Characters
            Octet 3: Bits 15 to 8 of Base pointer
            Octet 4 onwards: Characters
*/
GLOBAL void utl_Ucs2InFormat1( UBYTE*           in,
                               USHORT           inLen,
                               UBYTE*           out,
                               USHORT*          outLen)
{
  int i, j;
  UBYTE  base_ptr = 0x00;
  UBYTE  temp_ptr;
  USHORT len = inLen/2;
  USHORT tmp_base;
  USHORT tmp;
  USHORT outIdx = 0;
  UBYTE corr = 0x7F;
  

  /* We first check if the UCS2 string can be coded 
   * using a single base pointer */
  for (i = 0; i < len; i++)
  {
    if (in[i*2])
    {
      if ((in[i*2] & 0x80))
      {
        *outLen = 0;
        return;
      }
      temp_ptr = in[i*2] & 0x7f;
      temp_ptr <<= 1;
      temp_ptr |= ((in[(i*2)+1] & 0x80) >> 7);
      
      if (base_ptr)
      {
        if (temp_ptr NEQ base_ptr)
        {
          *outLen = 0;
          return;           
        }
      } else
      {
        base_ptr = temp_ptr;
      }
    }
  }


  /* Characters are coded using the base pointer below */
  /* For details, see GSM 11.11 Annex B (normative) */
  out[0] = 0x81;
  if (len < PHB_MAX_TAG_LEN - 3)
  {
    out[1] = (UBYTE)len;
  } else
  {
    out[1] = PHB_MAX_TAG_LEN - 3;
  }
  out[2]   = base_ptr;
  tmp_base = 0;
  tmp_base = base_ptr << 7;
  outIdx   = 3;

  len = out[1];
  for (i = 0; i < len; i++)
  {
    if (in[i*2])
    {
      tmp  = 0;
      tmp  = in[i*2] << 8;
      tmp |= in[(i*2)+1];
      out[outIdx++] = (tmp - tmp_base) | 0x80;
    } else 
    {
      for (j=0; j<128; j++)
      {
        if (in[(i*2)+1] EQ gsmToAsciiTable[j])
        {
          out[outIdx++] = (UBYTE)j;
          break;
        }
#ifdef REL99
        else
        {
          /* There is a chance that this is in the mapping table */
          UBYTE ext_tbl_idx = 0;        
          while( ext_tbl_idx < NUM_EXT_TABLE_CHARS              AND 
                 ext_common_chset_map[ext_tbl_idx][1] NEQ in[(i*2)+1] )
          { 
            ext_tbl_idx++;
          }
          if(ext_tbl_idx < NUM_EXT_TABLE_CHARS)
          {
            /*Insert the escape character, 0x1B, to output stream*/
            out[outIdx++] = 0x1B |(corr &0x80);
            /* Insert the mapping character into the output stream */
            out[outIdx++] = ( UBYTE )ext_common_chset_map[ext_tbl_idx][0] |(corr &0x80);
            break;
          }
        }
#endif /* REL99 */
      }
    }
   /* If the charecter cannot be found in ASCII table exit */
   if (j EQ 128)
   {
     *outLen = 0;
     return ;
   }
 }

#ifdef REL99
 if (outIdx -3 > len)  
 { 
   if (outIdx >=  PHB_MAX_TAG_LEN)
   {
     if (out[PHB_MAX_TAG_LEN - 1] EQ 0x1B) 
     {
       outIdx = PHB_MAX_TAG_LEN - 1;
       out[1] = PHB_MAX_TAG_LEN - 4;
     }
     else 
     {
       outIdx = PHB_MAX_TAG_LEN;
       out[1] = PHB_MAX_TAG_LEN - 3;
     }
   }
   else 
   {
     out[1] = outIdx -3;
   }
 }
#endif /* REL99 */

  *outLen = outIdx;
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_Ucs2InFormat2            |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the UCS2 character string in the 
            0x82 coding scheme for UCS2.
            Octet 1: 0x82
            Octet 2: Number of characters
            Octet 3 and Octet 4: 2 byte base pointer
            Octet 5 onwards: Characters 
            
*/
GLOBAL void utl_Ucs2InFormat2( UBYTE*           in,
                               USHORT           inLen,
                               UBYTE*           out,
                               USHORT*          outLen)
{
  int i, j;
  USHORT len = inLen/2;
  USHORT lowest_ch  = 0;
  USHORT highest_ch = 0;
  USHORT tmp;
  USHORT outIdx = 0;
  UBYTE corr = 0x7F;

  /* We first check if the UCS2 string can be coded 
   * using a smallest char as the base pointer */
  for (i = 0; i < len; i++)
  {
    if (in[i*2])
    {
      tmp  = 0;
      tmp  = in[i*2] << 8;
      tmp |= in[(i*2)+1];

      if (lowest_ch EQ 0 OR tmp < lowest_ch)
        lowest_ch = tmp;

      if (tmp > highest_ch)
        highest_ch = tmp;
    }
  }

  /* To use lowest char can be used as the base pointer, the distance
   * between the lowest and highest char must not be more then 128 */
  /* correct here: must not equal or more then 128. the max offset will be less then 0x80 = 128 */
//TI-SH-TEST-PATCH for CSR OMAPS00168884 
  if ((highest_ch - lowest_ch) >= 0x80)
  {
    *outLen = 0;
    return;
  }

  /* Characters are coded using the base pointer below */
  /* For details, see GSM 11.11 Annex B (normative) */
  out[0] = 0x82;
  if (len < PHB_MAX_TAG_LEN - 4)
  {
    out[1] = (UBYTE)len;
  } else
  {
    out[1] = PHB_MAX_TAG_LEN - 4;
  }
  out[2] = (lowest_ch & 0xff00) >> 8;
  out[3] = (lowest_ch & 0x00ff);  
  outIdx   = 4;

  len = out[1];
  for (i = 0; i < len; i++)
  {
    if (in[i*2])
    {
      tmp  = 0;
      tmp  = in[i*2] << 8;
      tmp |= in[(i*2)+1];
      out[outIdx++] = (tmp - lowest_ch) | 0x80;
    } else 
    {
      for (j=0; j<128; j++)
      {
        if (in[(i*2)+1] EQ gsmToAsciiTable[j])
        {
          out[outIdx++] = (UBYTE)j;
          break;
        }
#ifdef REL99
        else
        {
          /* There is a chance that this is in the mapping table */
          UBYTE ext_tbl_idx = 0;        
          while( ext_tbl_idx < NUM_EXT_TABLE_CHARS              AND 
                 ext_common_chset_map[ext_tbl_idx][1] NEQ in[(i*2)+1] )
          { 
            ext_tbl_idx++;
          }
          if(ext_tbl_idx < NUM_EXT_TABLE_CHARS)
          {
            /*Insert the escape character, 0x1B, to output stream*/
            out[outIdx++] = 0x1B |(corr &0x80);
            /* Insert the mapping character into the output stream */
            out[outIdx++] = ( UBYTE )ext_common_chset_map[ext_tbl_idx][0] |(corr &0x80);
            break;
          }
        }
#endif /* REL99 */
      }

    /* If the charecter cannot be found in ASCII table exit */
     if (j EQ 128)
     {
      *outLen = 0;
      return ;
     }
    }
  }

#ifdef REL99
  if (outIdx -4 > len)  
  { 
    if (outIdx >=  PHB_MAX_TAG_LEN)
    {
      if (out[PHB_MAX_TAG_LEN - 1] EQ 0x1B) 
      {
        outIdx = PHB_MAX_TAG_LEN - 1;
        out[1] = PHB_MAX_TAG_LEN - 5;
      }
      else 
      {
        outIdx = PHB_MAX_TAG_LEN;
        out[1] = PHB_MAX_TAG_LEN - 4;
      }
    }
    else 
    {
      out[1] = outIdx -4;
    }
  }
#endif /* REL99 */

  *outLen = outIdx;
}
#endif

#ifdef FF_ATI

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_chsetToGsm               |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of a string from
            the character set used within the AT command interpreter
            to the ACI internal GSM character set. The outBufLen was added 
            so that buffer overflow is kept in check when replacing one  
            non-GSM character with two characters - an escape character 0x1B 
            and a character from the extension table.
*/
GLOBAL void utl_chsetToGsm ( UBYTE*          in,
                             USHORT          inLen,
                             UBYTE*          out,
                             USHORT*         outLen,
#ifdef REL99
                             USHORT          outBufLen,
#endif
                             T_ACI_GSM_ALPHA gsm )
{
  UBYTE  cvtdVal;
  USHORT outIdx   = 0;
  USHORT inIdx;
  UBYTE  corr     = ( gsm EQ GSM_ALPHA_Int ? 0xFF : 0x7F );
  T_ACI_CSCS_CHSET cscsChset;
  UBYTE  srcId = srcId_cb;

  cscsChset = ati_user_output_cfg[srcId].cscsChset;

  if ( ati_user_output_cfg[srcId].cscsChset EQ CSCS_CHSET_Hex )
  {
    utl_hexToGsm ( in, inLen, out, outLen, gsm, CSCS_ALPHA_8_Bit );
  }
  else if ( cscsChset EQ CSCS_CHSET_Ucs2 )
  {
    utl_ucs2ToGsm ( in, inLen, out, outLen, gsm, CSCS_ALPHA_8_Bit );
  }
  else
  {
#ifndef REL99
    for ( inIdx = 0; inIdx < inLen; inIdx++ )
    {
      cvtdVal = chset[cscsChset][( UBYTE ) in[inIdx]];
      if ( cvtdVal NEQ 0x00 )
      {
        out[outIdx] = ( CHAR ) cvtdVal & corr;
        outIdx++;
      }
    }
#else
    for ( inIdx = 0; inIdx < inLen AND outIdx < outBufLen; inIdx++ )
    {
      cvtdVal = chset[cscsChset][( UBYTE ) in[inIdx]];

      if ( cvtdVal NEQ 0x00 )
      {
        out[outIdx] = ( CHAR ) cvtdVal & corr;
        outIdx++;
      }
      else
      {
        /* There is a chance that this is in the mapping table */
        UBYTE ext_tbl_idx = 0;        
        while( ext_tbl_idx < NUM_EXT_TABLE_CHARS              AND 
               ext_common_chset_map[ext_tbl_idx][1] NEQ in[inIdx] )
        {
          ext_tbl_idx++;
        }
        if(ext_tbl_idx < NUM_EXT_TABLE_CHARS)
        {
          /*Insert the escape character, 0x1B, to output stream*/
          out[outIdx++] = 0x1B |(corr &0x80);
          /* Check that output buffer limit has not been reached */
          if(outIdx < outBufLen)
          {
            /* Insert the mapping character into the output stream */
            out[outIdx++] = ( UBYTE )ext_common_chset_map[ext_tbl_idx][0] |(corr &0x80);
          }
        }
        else if ( ( cscsChset EQ CSCS_CHSET_Ira ) OR ( cscsChset EQ CSCS_CHSET_8859_1 ) )
        {
          /* Check if the character is present in ext_ira_8859_chset_map */
          ext_tbl_idx = 0;

          while( ext_tbl_idx < NUM_IRA_8859_TABLE_CHARS              AND 
                 ext_ira_8859_chset_map[ext_tbl_idx][1] NEQ in[inIdx] )
          {
            ext_tbl_idx++;
          }
             
          if( ext_tbl_idx < NUM_IRA_8859_TABLE_CHARS )
          {
            /*Insert the escape character, 0x1B, to output stream*/
            out[outIdx++] = 0x1B |(corr &0x80);
            /* Check that output buffer limit has not been reached */
            if(outIdx < outBufLen)
            {
              /* Insert the mapping character into the output stream */
              out[outIdx++] = ( UBYTE )ext_ira_8859_chset_map[ext_tbl_idx][0] |(corr &0x80);
            }
          }
        }
      }
    }
#endif /* ifndef REL99 */

    if ( gsm EQ GSM_ALPHA_Int )
      out[outIdx] = '\0';

    *outLen = outIdx;
  }
}
#endif


#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_hexFromAlpha             |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of a string from
            the SIM with UCS2 form 0x81 or 0x82 to 16 Bit hex values.
            see GSM 11.11 Annex B Coding of Alpha fields in the SIM for UCS2
            UCS2 form 0x81:
             - octet 1 = 0x81
             - octet 2 = length
             - octet 3 = Bits 15 to 8 of the base pointer to a half page in the UCS2 code space
                         0hhh hhhh h000 0000 , with h = bits of the half page base pointer
                        16... .... .... ...1

            UCS2 form 0x82:
             - octet 1 = 0x81
             - octet 2 = length
             - octet 3 = upper 8 Bit of 16 Bit base pointer to a half page in the UCS2 code space
             - octet 4 = lower 8 Bit of 16 Bit base pointer to a half page in the UCS2 code space           
*/
GLOBAL void utl_hexFromUCS2  ( UBYTE  *in, 
                              UBYTE  *out, 
                              USHORT  maxOutlen, 
                              USHORT *outlen,
                              T_ACI_GSM_ALPHA gsm)
{
  USHORT outIdx = 0;
  USHORT base_pointer;
  USHORT length = in[1];
  USHORT tmp;
  UBYTE  corr     = ( gsm EQ GSM_ALPHA_Int ? 0x00 : 0x80 );

  if (*in EQ 0x81)
  {
    base_pointer = in[2] << 7;
    in += 3;
  }
  else /* if (*in EQ 0x82) */
  {
    base_pointer = in[2]<<8 | in[3];
    in += 4;
  }
  
  while ( length-- AND outIdx < maxOutlen-2)
  {  
    if (*in & 0x80)
    {
      tmp  = base_pointer + (*in & 0x7F); 
    }
    else if((*in | corr) NEQ 0x9B)
    {
      tmp  = 0x0000 + gsmToAsciiTable[*in];
    }
#ifdef REL99
    else
    {
      UBYTE ext_tbl_idx = 0;
      BOOL  mapping_char_found = FALSE;  
      in++;

      while( ext_tbl_idx < NUM_EXT_TABLE_CHARS              AND 
             ext_common_chset_map[ext_tbl_idx][0] NEQ (*in & 0x7F))
      {
        ext_tbl_idx++;
      }
      if( ext_tbl_idx < NUM_EXT_TABLE_CHARS )
      {
        /* Insert the mapping character into the output stream */
        tmp = 0x0000 + ( UBYTE )ext_common_chset_map[ext_tbl_idx][1];
      }
    }
#endif
    out[outIdx++] = hexVal[(tmp >> 12) & 0x0F ];
    out[outIdx++] = hexVal[(tmp >>  8) & 0x0F ];
    out[outIdx++] = hexVal[(tmp >>  4) & 0x0F ];
    out[outIdx++] = hexVal[(tmp      ) & 0x0F ];

    in++;
  }

  out[outIdx] = '\0';
  *outlen = outIdx;
}
#endif


#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_chsetFromSim             |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of a string from
            the SIM format to the character set
            used within the AT command interpreter.
*/
GLOBAL void utl_chsetFromSim ( UBYTE*          in,
                               USHORT          inLen,
                               UBYTE*          out,
                               USHORT          maxOutLen,
                               USHORT*         outLen,
                               T_ACI_GSM_ALPHA gsm )
{
  T_ACI_CSCS_CHSET cscsChset;
  UBYTE srcId = srcId_cb;

  cscsChset = ati_user_output_cfg[srcId].cscsChset;

  if ( cscsChset EQ CSCS_CHSET_Ucs2 AND
        (( *in EQ 0x80 ) OR ( *in EQ 0x81 ) OR ( *in EQ 0x82 )) )
  {
    /* UCS2 with form 0x81 or 0x82 as HEX string */
    if (( *in EQ 0x81 ) OR ( *in EQ 0x82 ))
    {
      utl_hexFromUCS2 ( in, out, maxOutLen, outLen, gsm);
      return;
    }
    
    /* UCS2 form 0x80 as HEX string */
    utl_hexFromGsm ( in + 1, (USHORT)( inLen - 1 ), out, maxOutLen, outLen,
                     gsm, CSCS_ALPHA_8_Bit );
  } 
  else 
  {
    utl_chsetFromGsm ( in, inLen, out, maxOutLen, outLen, gsm );
  }
}
#endif /* #ifdef FF_ATI */

#if defined(FF_ATI) || defined(FF_BAT)
#ifdef TI_PS_OP_CNV_TAB_ROMBASED
#endif

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_chsetFromGsm             |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of a string from
            the ACI internal GSM character set to the character set
            used within the AT command interpreter.
*/

GLOBAL USHORT utl_chsetFromGsm(
  UBYTE*            in,
  USHORT            inLen,
  UBYTE*            out,
  USHORT            maxOutLen,
  USHORT*           outLen,
  T_ACI_GSM_ALPHA   gsm)
{
  USHORT outIdx = 0;
  USHORT inIdx  = 0;
  USHORT actLen = ( gsm EQ GSM_ALPHA_Int ?
                        ( USHORT ) strlen ( ( CHAR* ) in ) : inLen );
  UBYTE  corr   = ( gsm EQ GSM_ALPHA_Int ? 0x00 : 0x80 );
  T_ACI_CSCS_CHSET cscsChset;
  UBYTE  srcId = srcId_cb;
  USHORT tblIdx;

  cscsChset = ati_user_output_cfg[srcId].cscsChset;

  if ( cscsChset EQ CSCS_CHSET_Hex )
  {
    return utl_hexFromGsm ( in, inLen, out, maxOutLen, outLen, gsm, CSCS_ALPHA_8_Bit );
  }
  else if ( cscsChset EQ CSCS_CHSET_Ucs2 )
  {
    return utl_ucs2FromGsm ( in, inLen, out, maxOutLen, outLen, gsm, CSCS_ALPHA_8_Bit );
  }
  else
  {
#ifndef REL99

    while ( inIdx < actLen AND outIdx < maxOutLen - 1 )
    {
      tblIdx = 0;

      
      while ( tblIdx <= 0xFF                                      AND
               chset[cscsChset][tblIdx] NEQ (( UBYTE )in[inIdx] | corr ))
        tblIdx++;
      if ( tblIdx <= 0xFF )
      {
        out[outIdx] = ( UBYTE )tblIdx;
        outIdx++;
      }

      inIdx++;
    }

#else
    /* TODO: What if the CSCS is GSM7 itself. Do we have to do this? The following check
       if( cscsChset NEQ CSCS_CHSET_Gsm AND gsm EQ GSM_ALPHA_Def */
    while ( inIdx < actLen AND outIdx < maxOutLen - 1 )
    {
      /* Check if the character is a escape to an extension of the GSM 7 bit 
       * default alphabet table. */
      if( cscsChset NEQ CSCS_CHSET_Gsm AND ((in[inIdx] | corr) EQ 0x9B) )
      {
        UBYTE ext_tbl_idx = 0;
        BOOL  mapping_char_found = FALSE;  
        inIdx++;

        /* Check if the end of the array is reached */
        if(inIdx EQ actLen)
        {
          break;
        }

        while( ext_tbl_idx < NUM_EXT_TABLE_CHARS              AND 
               ext_common_chset_map[ext_tbl_idx][0] NEQ (in[inIdx] & 0x7F))
        {
          ext_tbl_idx++;
        }
        if( ext_tbl_idx < NUM_EXT_TABLE_CHARS )
        {
          /* Insert the mapping character into the output stream */
          out[outIdx++] = ( UBYTE )ext_common_chset_map[ext_tbl_idx][1];
          mapping_char_found = TRUE;
        }
        else if( cscsChset EQ CSCS_CHSET_Ira OR cscsChset EQ CSCS_CHSET_8859_1 )
        { 
          ext_tbl_idx = 0;

          while( ext_tbl_idx < NUM_IRA_8859_TABLE_CHARS              AND 
                 ext_ira_8859_chset_map[ext_tbl_idx][0] NEQ (in[inIdx]&0x7F))
          {
            ext_tbl_idx++;
          }

          if(ext_tbl_idx < NUM_IRA_8859_TABLE_CHARS)
          {
            /* Insert the mapping character into the output stream */
            out[outIdx++] = ( UBYTE )ext_ira_8859_chset_map[ext_tbl_idx][1];
            mapping_char_found = TRUE;
          }
        }

        if(mapping_char_found EQ FALSE)
        {
          /* From 23.038: In the event that an MS receives a code where a symbol 
             is not represented  in the extension table then the MS shall display 
             the character shown in the main GSM 7 bit default alphabet table */
            tblIdx = 0;
            while ( tblIdx <= 0xFF     AND
                    chset[cscsChset][tblIdx] NEQ (( UBYTE )in[inIdx] | corr ))
              tblIdx++;

            if ( tblIdx <= 0xFF )
            {
              out[outIdx] = ( UBYTE )tblIdx;
              outIdx++;
            }

        }
      }
      else
      {
        tblIdx = 0;

        while ( tblIdx <= 0xFF                                      AND
                chset[cscsChset][tblIdx] NEQ (( UBYTE )in[inIdx] | corr ))
          tblIdx++;

        if ( tblIdx <= 0xFF )
        {
          out[outIdx] = ( UBYTE )tblIdx;
          outIdx++;
        }

      }

      inIdx++;
    }
#endif

    out[outIdx] = '\0';
    *outLen     = outIdx;

    return inIdx;
  }
}
#endif


#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_sprints                  |
+-------------------------------------------------------------------+

  PURPOSE : This function writes a not null terminated string to a
            buffer.
*/
GLOBAL USHORT sprints ( CHAR* buf, CHAR* arg, USHORT len )
{
  buf[0] = '\"';

  memcpy ( &buf[1], arg, len );

  buf[len + 1] = '\"';
  buf[len + 2] = '\0';

  return (len + 2);
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_sprintq                  |
+-------------------------------------------------------------------+

  PURPOSE : This function writes a not null terminated string to a
            buffer.
*/
GLOBAL USHORT sprintq ( CHAR* buf, CHAR* arg, USHORT len )
{
  memcpy ( &buf[0], arg, len );

  buf[len] = '\0';

  return (len);
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_hexToIntGsm              |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of a string from
            the "HEX" character set to the ACI internal GSM
            character set.
*/
GLOBAL void utl_hexToGsm ( UBYTE*           in,
                           USHORT           inLen,
                           UBYTE*           out,
                           USHORT*          outLen,
                           T_ACI_GSM_ALPHA  gsm,
                           T_ACI_CSCS_ALPHA alphabet )
{
  SHORT  val    = 0;
  UBYTE  digit;
  USHORT inIdx  = 0;
  USHORT outIdx = 0;
  SHORT  base   = 0x10;

  while ( inIdx < inLen )
  {
    if ( in[inIdx] >= 0x30 AND in[inIdx] <= 0x39 )        /* '0' ... '9' */
    {
      digit = in[inIdx] - 0x30;                           /* ->0 ... 9  */
    }
    else if ( in[inIdx] >= 0x61 AND in[inIdx] <= 0x66 )   /* 'a' ... 'f' */
    {
      digit = in[inIdx] - 0x61 + 0x0A;                    /* ->0x0a...0x0f */
    }
    else if ( in[inIdx] >= 0x41 AND in[inIdx] <= 0x46 )   /* 'A' ... 'F' */
    {
      digit = in[inIdx] - 0x41 + 0x0A;                    /* ->0x0a...0x0f */
    }
    else
    {
      digit = 0xFF;
    }

    if ( digit NEQ 0xFF )  /* skip invalid digit */
    {
      if ( base EQ 0x10 )
      {
        val = digit << 4;
      }
      else
      {
        val |= digit;

        if ( alphabet EQ CSCS_ALPHA_7_Bit AND !( val & 0x80 ) )
        {
          if ( gsm EQ GSM_ALPHA_Int )
          {
            out[outIdx++] = ( CHAR ) val | 0x80;
          }
          else
          {
            out[outIdx++] = ( CHAR ) val;
          }
        }
        else if ( alphabet NEQ CSCS_ALPHA_7_Bit )
        {
          out[outIdx++] = ( CHAR ) val;
        }
      }

      base ^= 0x10;
    }

    inIdx++;
  }

  if ( gsm EQ GSM_ALPHA_Int )
    out[outIdx] = '\0';

  *outLen = outIdx;
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_ucs2ToGsm              |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of a string from
            the "UCS2" character set to the ACI internal GSM
            character set.
*/
GLOBAL void utl_ucs2ToGsm ( UBYTE*           in,
                           USHORT           inLen,
                           UBYTE*           out,
                           USHORT*          outLen,
                           T_ACI_GSM_ALPHA  gsm,
                           T_ACI_CSCS_ALPHA alphabet )
{
  int i, j;
  UBYTE val;
  USHORT len;
  UBYTE tmpIn[2];  /* store temporary two chars (HEX conversion) */
  UBYTE tmpInIdx;  /* holds a temporary index for two chars (HEX conversion)*/
  UBYTE digit;     /* holds a digit (HEX conversion) */
  SHORT tmpVal;    /* hold a temporary value (HEX conversion) */
  UBYTE tmpOut; /* the HEX output (HEX conversion) */ 
  SHORT  base   = 0x10;
  USHORT outIdx = 0;
  UBYTE  corr   = ( gsm EQ GSM_ALPHA_Int ? 0xFF : 0x7F );
  
  /* TRACE_FUNCTION(" utl_ucs2ToGsm() "); */
  
  len = inLen/4;
  
  for (i=0;i<len;i++)
  {
    /* check if this is a UCS2 character in 00xy format */
    if ( ( in[i*4] EQ '0' ) AND ( in[i*4+1] EQ '0' )  ) 
    {
      /* convert the next two character to HEX */
      tmpIn[0] = in[i*4+2];
      tmpIn[1] = in[i*4+3];

      tmpVal = tmpOut = val = 0;
      
      /* convert the two characters into the real hexadecimal ASCII value */
      for( tmpInIdx=0; tmpInIdx<2; tmpInIdx++ )
      {
        if ( tmpIn[tmpInIdx] >= '0' AND tmpIn[tmpInIdx] <= '9' )
        {
          digit = tmpIn[tmpInIdx] - '0';
        }
        else if ( tmpIn[tmpInIdx] >= 'a' AND tmpIn[tmpInIdx] <= 'f' )
        {
          digit = tmpIn[tmpInIdx] - 'a' + 0x0A;
        }
        else if ( tmpIn[tmpInIdx] >= 'A' AND tmpIn[tmpInIdx] <= 'F' )
        {
          digit = tmpIn[tmpInIdx] - 'A' + 0x0A;
        }
        else
        {
          digit = 0xFF;
        }
        
        if ( digit NEQ 0xFF )
        {
          if ( base EQ 0x10 )
          {
            tmpVal = digit * 0x10;
          }
          else
          {
            tmpVal += digit;
            tmpOut = (UBYTE) tmpVal;
          }
          
          base ^= 0x10;
        }
      }      
      
      /* convert the hexadecimal ASCII value to GSM */
      if (!(tmpOut & 0x80))  
      {
        if ( tmpOut EQ ( gsmToAsciiTable[tmpOut])) /* ASCII and GSM are identical */
        {
          val = tmpOut;
        }
        else /* find match in the table and copy index of match */
        {
          for (j=0; j<128; j++)
          {
            if (tmpOut EQ gsmToAsciiTable[j])
            {
              val = (UBYTE)j;
              break;
            }
#ifdef REL99
           else
           {
             /* There is a chance that this is in the mapping table */
             UBYTE ext_tbl_idx = 0;        
             while( ext_tbl_idx < NUM_EXT_TABLE_CHARS              AND 
                    ext_common_chset_map[ext_tbl_idx][1] NEQ tmpOut )
             { 
               ext_tbl_idx++;
             }
             if(ext_tbl_idx < NUM_EXT_TABLE_CHARS)
             {
               /*Insert the escape character, 0x1B, to output stream*/
               out[outIdx++] = 0x1B |(corr &0x80);
               /* Insert the mapping character into the output stream */
               val = ( UBYTE )ext_common_chset_map[ext_tbl_idx][0] |(corr &0x80);
               break;
             }
           }
#endif /* REL99 */
          }
        }      
      }
      else /* find match in the table and copy index of match */
      {
        for (j=0; j<128; j++)
        {
          if (tmpOut EQ gsmToAsciiTable[j])
          {
            val = (UBYTE)j;
            break;
          }
#ifdef REL99
          else
          {
            /* There is a chance that this is in the mapping table */
            UBYTE ext_tbl_idx = 0;        
            while( ext_tbl_idx < NUM_EXT_TABLE_CHARS              AND 
                   ext_common_chset_map[ext_tbl_idx][1] NEQ tmpOut )
            { 
              ext_tbl_idx++;
            }
            if(ext_tbl_idx < NUM_EXT_TABLE_CHARS)
            {
              /*Insert the escape character, 0x1B, to output stream*/
              out[outIdx++] = 0x1B |(corr &0x80);
              /* Insert the mapping character into the output stream */
              val = ( UBYTE )ext_common_chset_map[ext_tbl_idx][0] |(corr &0x80);
              break;
            }
          }
#endif /* REL99 */
        }
      }
      
      /* if necessary, cut the GSM value to 7bit */
      if ( alphabet EQ CSCS_ALPHA_7_Bit AND !( val & 0x80 ) )
      {
        if ( gsm EQ GSM_ALPHA_Int )
        {
          out[outIdx++] = val | 0x80; 
        }
        else
        {
          out[outIdx++] = val;
        }
      }
      else
      {
        out[outIdx++] = val;
      }
    }
    
    else  /* unknown char, skip it */
    {
      /* TRACE_EVENT("UCS2 MISMATCH: Unknown UCS2 entry, character skipped!"); */
      /* out[outIdx++] = '?'; */
    }
  }
  
  if ( gsm EQ GSM_ALPHA_Int )
  {
    out[outIdx] = '\0';
  }
  
  /* set the outlength */
  *outLen = outIdx;
  
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_hexFromGsm               |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of a string from
            the ACI internal GSM character set to the "HEX"
            character set.
*/
GLOBAL USHORT utl_hexFromGsm ( UBYTE*           in,
                             USHORT           inLen,
                             UBYTE*           out,
                             USHORT           maxOutLen,
                             USHORT*          outLen,
                             T_ACI_GSM_ALPHA  gsm,
                             T_ACI_CSCS_ALPHA alphabet )
{
  USHORT inIdx  = 0;
  USHORT outIdx = 0;

  while ( inIdx < inLen AND outIdx < maxOutLen - 2 )
  {
      if ( alphabet NEQ CSCS_ALPHA_7_Bit OR
         ( gsm EQ GSM_ALPHA_Def AND !( in[inIdx] & 0x80 ) )  OR
         ( gsm EQ GSM_ALPHA_Int AND  ( in[inIdx] & 0x80 ) ) )
    {
      /* Convert to 7bit default alphabet only if the data is 
         coded in internal GSM character set */
      if ( gsm EQ GSM_ALPHA_Int AND  ( in[inIdx] & 0x80 ) )
      {
        out[outIdx++] = hexVal[ (in[inIdx] & 0x7F) >> 4    ];
      }
      else
      {
        out[outIdx++] = hexVal[ in[inIdx] >> 4    ];
      }
      out[outIdx++] = hexVal[ in[inIdx]  & 0x0F ];
    }

    inIdx++;
  }

  out[outIdx] = '\0';
  *outLen     = outIdx;
  
  return inIdx;
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_ucs2FromGsm              |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of a string from
            the ACI internal GSM character set to the "UCS2"
            character set.
*/
LOCAL USHORT utl_ucs2FromGsm (UBYTE*           in,
                              USHORT           inLen,
                              UBYTE*           out,
                              USHORT           maxOutLen,
                              USHORT*          outLen,
                              T_ACI_GSM_ALPHA  gsm,
                              T_ACI_CSCS_ALPHA alphabet )
{
  USHORT inIdx  = 0;
  USHORT outIdx = 0;
  UBYTE  corr   = ( gsm EQ GSM_ALPHA_Int ? 0x00 : 0x80 );

  while ( inIdx < inLen AND outIdx < maxOutLen - 4 )
  {
    if ( alphabet NEQ CSCS_ALPHA_7_Bit OR
       ( gsm EQ GSM_ALPHA_Def AND !( in[inIdx] & 0x80 ) )     OR
       ( gsm EQ GSM_ALPHA_Int AND  ( in[inIdx] & 0x80 ) ) )
    {
      /* Insert two leading Os, convert GSM to ASCII and print out as HEX  */
      out[outIdx++] = '0';
      out[outIdx++] = '0';
    #ifdef REL99
      if( (in[inIdx] | corr) EQ 0x9B)
      {
        UBYTE ext_tbl_idx = 0;
        inIdx++;
        while( ext_tbl_idx < NUM_EXT_TABLE_CHARS              AND 
               ext_common_chset_map[ext_tbl_idx][0] NEQ (in[inIdx] & 0x7F))
        {
          ext_tbl_idx++;
        }
        if( ext_tbl_idx < NUM_EXT_TABLE_CHARS )
        {
          /* Insert the mapping character into the output stream */
          out[outIdx++] = hexVal[(ext_common_chset_map[ext_tbl_idx][1] & 0x7F) >> 4   ];
          out[outIdx++] = hexVal[(ext_common_chset_map[ext_tbl_idx][1] & 0x7F) & 0x0F ];
        }
      }
      else
      {
    #endif /* REL99 */
      out[outIdx++] = hexVal[ gsmToAsciiTable[in[inIdx] & 0x7F] >> 4    ];
      out[outIdx++] = hexVal[ gsmToAsciiTable[in[inIdx] & 0x7F] &  0x0F ];
    #ifdef REL99
      }
    #endif /* REL99 */

    }

    inIdx++;
  }

  out[outIdx] = '\0';
  *outLen     = outIdx;
  
  return inIdx;

}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_ucs2FromGsm_ussd         |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of USSD string from
            the ACI internal GSM character set to the "UCS2"
            character set with 2 bytes only.
*/
GLOBAL USHORT utl_ucs2FromGsm_ussd(UBYTE*           in,
                              USHORT           inLen,
                              UBYTE*           out,
                              USHORT           maxOutLen,
                              USHORT*          outLen,
                              T_ACI_GSM_ALPHA  gsm,
                              T_ACI_CSCS_ALPHA alphabet )
{
  USHORT inIdx  = 0;
  USHORT outIdx = 0;

  while ( inIdx < inLen AND outIdx < maxOutLen - 4 )
  {
    if ( alphabet NEQ CSCS_ALPHA_7_Bit OR
       ( gsm EQ GSM_ALPHA_Def AND !( in[inIdx] & 0x80 ) )     OR
       ( gsm EQ GSM_ALPHA_Int AND  ( in[inIdx] & 0x80 ) ) )
    {
      /* 2 bytes for convertion from GSM to UCS2 */
      out[outIdx++] = 0x00;
      out[outIdx++] = gsmToAsciiTable[in[inIdx] & 0x7F];      
    }

    inIdx++;
  }

  out[outIdx] = '\0';
  *outLen     = outIdx;
  
  return inIdx;

}
#endif
#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_smDtaToTe                |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the SM data in the format
            specified in Rec. GSM 07.05, message data parameter
            <data>, to the format used by the AT Command Interface.
*/
GLOBAL void utl_smDtaToTe ( UBYTE*  in,
                            USHORT  inLen,
                            UBYTE*  out,
                            USHORT  maxOutLen,
                            USHORT* outLen,
                            UBYTE   fo,
                            UBYTE   dcs )
{
  UBYTE alphabet = cmhSMS_getAlphabetPp ( dcs );

  if ( alphabet EQ CSCS_ALPHA_7_Bit AND
       (fo & TP_UDHI_MASK) EQ TP_UDHI_WITHOUT_HEADER )
  {
    utl_chsetFromGsm ( in, inLen, out, maxOutLen, outLen, GSM_ALPHA_Def );
  }
  else if ( alphabet NEQ CSCS_ALPHA_7_Bit OR
            (fo & TP_UDHI_MASK) EQ TP_UDHI_WITH_HEADER )
  {
    utl_hexFromGsm ( in, inLen, out, maxOutLen, outLen, GSM_ALPHA_Def, 
                     CSCS_ALPHA_8_Bit );
  }
  else
  {
    *outLen = MINIMUM ( inLen, maxOutLen - 1 );
    memcpy ( out, in,  *outLen );
    out[*outLen] = '\0';
  }
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_smDtaFromTe              |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the SM data in the format used
            by the AT Command Interface to the format specified in
            Rec. GSM 07.05, message data parameter <data>.
*/
GLOBAL void utl_smDtaFromTe ( UBYTE*  in,
                              USHORT  inLen,
                              UBYTE*  out,
                              USHORT* outLen,
#ifdef REL99
                              USHORT  outBufLen, 
#endif
                              UBYTE   fo,
                              UBYTE   dcs )
{
  UBYTE alphabet = cmhSMS_getAlphabetPp ( dcs );

  if ( alphabet EQ CSCS_ALPHA_7_Bit                AND
       ( fo & TP_UDHI_MASK ) EQ TP_UDHI_WITHOUT_HEADER )
  {
#ifdef REL99
    utl_chsetToGsm ( in, inLen, out, outLen, outBufLen, GSM_ALPHA_Def );
#else
    utl_chsetToGsm ( in, inLen, out, outLen, GSM_ALPHA_Def );
#endif

  }
  else if ( alphabet NEQ CSCS_ALPHA_7_Bit             OR
            ( fo & TP_UDHI_MASK ) EQ TP_UDHI_WITH_HEADER )
  {
    utl_hexToGsm ( in, inLen, out, outLen, GSM_ALPHA_Def, (T_ACI_CSCS_ALPHA)alphabet );
  }
  else
  {
    memcpy ( out, in, inLen );
    *outLen = inLen;
  }
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_ussdDtaFromTe            |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the USSD data in the format used
            by the AT Command Interface to the format specified in
            Rec. GSM 07.07.
*/
GLOBAL void utl_ussdDtaFromTe ( UBYTE*  in,
                                USHORT  inLen,
                                UBYTE*  out,
                                USHORT* outLen,
#ifdef REL99
                                USHORT  outBufLen,
#endif
                                UBYTE   dcs )
{
  /* Implements Measure 25 */
  UBYTE alphabet = cmh_getAlphabetCb ( dcs );

  if ( alphabet EQ CSCS_ALPHA_7_Bit )
  {
#ifdef REL99
    utl_chsetToGsm ( in, inLen, out, outLen, outBufLen, GSM_ALPHA_Def );
#else
    utl_chsetToGsm ( in, inLen, out, outLen, GSM_ALPHA_Def );
#endif

  }
  else
  {
    utl_hexToGsm ( in, inLen, out, outLen, GSM_ALPHA_Def, (T_ACI_CSCS_ALPHA)alphabet );
  }
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_ussdDtaToTe              |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the USSD data in the format
            specified in Rec. GSM 07.07 to the format used
            by the AT Command Interface.
*/

GLOBAL USHORT utl_ussdDtaToTe ( UBYTE*  in,
                              USHORT  inLen,
                              UBYTE*  out,
                              USHORT  maxOutLen,
                              USHORT* outLen,
                              UBYTE   dcs )
{
  /* Implements Measure 25 */
  UBYTE alphabet = cmh_getAlphabetCb ( dcs );
  if ( alphabet EQ CSCS_ALPHA_7_Bit )
  {
    return utl_chsetFromGsm ( in, inLen, out, maxOutLen, outLen, GSM_ALPHA_Def );
  }
  else
  {
    return utl_hexFromGsm ( in, inLen, out, maxOutLen, outLen, GSM_ALPHA_Def, (T_ACI_CSCS_ALPHA)alphabet );
  }
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_cbmDtaToTe               |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the CBM data in the format
            specified in Rec. GSM 07.05, message data parameter
            <data>, to the format used by the AT Command Interface.
*/
GLOBAL void utl_cbmDtaToTe ( UBYTE*  in,
                             USHORT  inLen,
                             UBYTE*  out,
                             USHORT  maxOutLen,
                             USHORT* outLen,
                             UBYTE   fo,
                             UBYTE   dcs )
{
  /* Implements Measure 25 */
  UBYTE alphabet = cmh_getAlphabetCb ( dcs );
  if ( alphabet EQ CSCS_ALPHA_7_Bit                AND
       ( fo & TP_UDHI_MASK ) EQ TP_UDHI_WITHOUT_HEADER )
  {
    utl_chsetFromGsm ( in, inLen, out, maxOutLen, outLen, GSM_ALPHA_Def );
  }
  else if ( alphabet NEQ CSCS_ALPHA_7_Bit             OR
            ( fo & TP_UDHI_MASK ) EQ TP_UDHI_WITH_HEADER )
  {
    utl_hexFromGsm ( in, inLen, out, maxOutLen, outLen, GSM_ALPHA_Def, (T_ACI_CSCS_ALPHA)alphabet );
  }
  else
  {
    *outLen = MINIMUM ( inLen, maxOutLen );
    memcpy ( out, in, *outLen );
  }
}
#endif

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_cvtGsmIra                |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the characters of a string from
            the ACI internal GSM character set to the IRA character
            set or vice versa.
*/
GLOBAL BOOL utl_cvtGsmIra ( UBYTE*         in,
                            USHORT         inLen,
                            UBYTE*         out,
                            USHORT         outLen,
                            T_ACI_CSCS_DIR dir )
{
  USHORT inIdx  = 0;
  USHORT outIdx = 0;
  USHORT tblIdx;
  UBYTE  cvtdVal;

  TRACE_FUNCTION("utl_cvtGsmIra()");


  if ( inLen > outLen )
  {
    return ( FALSE );
  }

  if ( dir EQ CSCS_DIR_GsmToIra )
  {
    while ( inIdx < inLen )
    {
      tblIdx = 0;

      while ( tblIdx <= 0xFF                                             AND
              chset[CSCS_CHSET_Ira][tblIdx] NEQ (( UBYTE )in[inIdx] | 0x80 ))
        tblIdx++;

      if ( tblIdx <= 0xFF )
      {
        out[outIdx] = ( UBYTE )tblIdx;
        outIdx++;
      }
      else
      {
        return ( FALSE );
      }

      inIdx++;
    }
  }
  else if ( dir EQ CSCS_DIR_IraToGsm )
  {
    for ( inIdx = 0; inIdx < inLen; inIdx++ )
    {
      cvtdVal = chset[CSCS_CHSET_Ira][( UBYTE ) in[inIdx]];
        
      if ( cvtdVal NEQ 0x00 )
      {
        out[outIdx] = ( CHAR ) cvtdVal & 0x7F;
        outIdx++;
      }
      else
      {
        return ( FALSE );
      }
    }

  }
  else
  {
    return ( FALSE );
  }

  return ( TRUE );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_binToHex                 |
+-------------------------------------------------------------------+

  PURPOSE : This function converts a binary string of bytes with a
            given length into an null terminated ASCII string
            representation.
*/
GLOBAL void utl_binToHex (UBYTE*         in,
                          SHORT          inLen,
                          CHAR*          out )
{
  SHORT idx = 0;
  UBYTE hNib;
  UBYTE lNib;

  while( idx < inLen )
  {
    hNib = (in[idx]&0xF0)>>4;

    if( hNib > 9 ) *out = (hNib-10)+0x41;
    else           *out = hNib + 0x30;

    out++;

    lNib = in[idx]&0x0F;

    if( lNib > 9 ) *out = (lNib-10)+0x41;
    else           *out = lNib + 0x30;

    out++;
    idx++;
  }
  *out = 0x0;
}


#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM              MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : utl_HexStrToBin              |
+-------------------------------------------------------------------+

  PURPOSE : This function converts a null terminated string of HEX
            values in a binary buffer.
            RETURN: byte count.
*/
GLOBAL USHORT utl_HexStrToBin (UBYTE         *in,
                               USHORT         inLen,
                               UBYTE         *out,
                               USHORT         outLen)
{
  USHORT inIdx = 0;
  USHORT outIdx = 0;
  UBYTE value;
  BOOL  hNib = TRUE;

  while ((inIdx < inLen) AND (outIdx < outLen))
  {
    if ((in[inIdx] >= 0x30) AND (in[inIdx] <= 0x39))
    {
      value = in[inIdx] - 0x30;
    }
    else if ((in[inIdx] >= 0x61) AND (in[inIdx] <= 0x66))
    {
      value = in[inIdx] - 0x61 + 0x0A;
    }
    else if ((in[inIdx] >= 0x41) AND (in[inIdx] <= 0x46))
    {
      value = in[inIdx] - 0x41 + 0x0A;
    }
    else
    {
      return (0);
    }

    if (hNib)
    {
      out[outIdx] = (value << 0x04) & 0xF0;
      hNib = FALSE;
    }
    else
    {
      out[outIdx] |= value & 0x0F;
      hNib = TRUE;
      outIdx++;
    }
    inIdx++;
  }

  return (outIdx);
}
#endif

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : trace_cmd_line               |
+-------------------------------------------------------------------+

  PURPOSE : This function trace the command line and check
            this string for % character
*/

GLOBAL void trace_cmd_line (char *prefix, char *output, UBYTE srcId, USHORT output_len)
{
  char trcBuf[80];
  int dst_i;

  dst_i = sprintf (trcBuf, "%s(Src %d)[lth %d]", (prefix) ? prefix : "", srcId, output_len);

  strncpy(&trcBuf[dst_i], output, 79-dst_i);
  trcBuf[79] = '\0';

  if (trcBuf[76])
  {
    trcBuf[76] = trcBuf[77] = trcBuf[78] = '.';  /* add trailing "..." if string is >=76 */
  }

  /* Send the trace if either EVENT or CLASS8 is turned on */
  TRACE_USER_CLASS_P1(TC_EVENT|TC_USER8, "%s",trcBuf);
}
#endif


#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : trace_cmd_state              |
+-------------------------------------------------------------------+

  PURPOSE : This function trace the command line state
*/

GLOBAL void trace_cmd_state (UBYTE            srcId, 
                             T_ATI_CMD_STATE  old_state, 
                             T_ATI_CMD_STATE  new_state)
{
  char trcBuf[50];

  if (old_state EQ new_state)
  {
    return;
  }

  sprintf (trcBuf, "(Src %d) cmd_state: ", srcId );

  switch (old_state)
  {
    case (CMD_IDLE):
      strcat (trcBuf, "CMD_IDLE -> ");
      break;
    case (CMD_TYPING):
      strcat (trcBuf, "CMD_TYPING -> ");
      break;
    case (CMD_RUNNING):
      strcat (trcBuf, "CMD_RUNNING -> ");
      break;
    default:
      strcat (trcBuf, "CMD_UNKNOWN ! ");
  }
  
  switch (new_state)
  {
    case (CMD_IDLE):
      strcat (trcBuf, "CMD_IDLE");
      break;
    case (CMD_TYPING):
      strcat (trcBuf, "CMD_TYPING");
      break;
    case (CMD_RUNNING):
      strcat (trcBuf, "CMD_RUNNING");
      break;
    default:
      strcat (trcBuf, "CMD_UNKNOWN !");
  }

  TRACE_EVENT_P1("%s",trcBuf);
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : toa_merge          |
+--------------------------------------------------------------------+

  PURPOSE : builds type of address octet from TOA structure

*/

GLOBAL UBYTE toa_merge (T_ACI_TOA type)
{

return   ((type.ton << 4) & 0xF0) 
       | ( type.npi       & 0x0F)
       | 0x80;
}



#ifdef GPRS
/*
  +-----------------------------------------------------------------------------
  | Function    : utl_create_pco
  +-----------------------------------------------------------------------------
  | Description : The function create_pco() creates protocol configuration
  |               options needed to activate a PDPcontext.
  |
  | Parameters  : buffer      - buffer to write the PCOs
  |               length      - on call: buffer size; on return: written bytes
  |               content     - mask to specify the PCO content
  |               config_prot - used configuration protocol (currently PPP)
  |               auth_prot   - authentication protocol (currently PAP)
  |               user_name   - string with the user name
  |               password    - string with the password
  |               dns1        - requested primary DNS address
  |               dns2        - requested secondary DNS address
  |
  | Return      :  0 - operation successful
  |               -1 - operation fails
  |
  +-----------------------------------------------------------------------------
*/
GLOBAL int utl_create_pco (UBYTE*  buffer,
                           USHORT* length,
                           ULONG   content,
                           UBYTE   config_prot,
                           USHORT  auth_prot,
                           UBYTE*  user_name,
                           UBYTE*  password,
                           ULONG   dns1,
                           ULONG   dns2)
{
  USHORT  size;
  USHORT  temp_len;
  USHORT  len_user;
  USHORT  len_pwd;
  USHORT  written;
  UBYTE*  pos;
  UBYTE*  len_pos1;
  UBYTE*  len_pos2;

  TRACE_FUNCTION( "create_pco" );

  /*
   * store buffer size
   */
  size = *length;
  /*
   * initialize values
   */
  *length = 0;
  written = 0;
  pos     = buffer;
  /*
   * calculate if requested content is writeable
   */
  temp_len = 1;
  if(content & ACI_PCO_CONTENTMASK_AUTH)
  {
    len_user = strlen((char*)user_name);
    len_pwd  = strlen((char*)password);
    temp_len+= ACI_PCO_PAP_OVERHEAD + len_user + len_pwd;
  }
  if((content & ACI_PCO_CONTENTMASK_DNS1) ||
     (content & ACI_PCO_CONTENTMASK_DNS2))
  {
    temp_len+= ACI_PCO_IPCP_OVERHEAD;
    if(content & ACI_PCO_CONTENTMASK_DNS1)
      temp_len+= ACI_PCO_IPCP_LENGTH_DNS1;
    if(content & ACI_PCO_CONTENTMASK_DNS2)
      temp_len+= ACI_PCO_IPCP_LENGTH_DNS2;
  }
  TRACE_EVENT_P2("temp_len=%d size=%d", temp_len, size);
  if(temp_len > size)
  {
    /*
     * content is to long
     */
    return -1;
  }
  /*
   * set configuration protocol identifier
   */
  *pos = 0x80 | config_prot;
  pos++;
  written++;

  /*
   * authentication
   */
  if(content & ACI_PCO_CONTENTMASK_AUTH)
  {
    /*
     * set authentication protocol
     */
    *pos = (UBYTE)((auth_prot >> 8) & 0x00ff);
    pos++;
    written++;
    *pos = (UBYTE)(auth_prot        & 0x00ff);
    pos++;
    written++;
    /*
     * store first length position
     */
    len_pos1 = pos;
    pos++;
    written++;
    /*
     * Code field
     */
    *pos = ACI_PCO_PAP_AUTH_REQ;
    pos++;
    written++;
    /*
     * Identifier field (just some value)
     */
    *pos = 0x01;
    pos++;
    written++;
    /*
     * Length field (store length position)
     */
    *pos = 0x00;
    pos++;
    written++;
    len_pos2 = pos;
    pos++;
    written++;
    /*
     * User Name Length field
     */
    *pos = (UBYTE)(/*lint -e(644) */len_user & 0x00ff);
    pos++;
    written++;
    /*
     * User Name field
     */
    memcpy(pos, user_name, len_user);
    pos    += len_user;
    written+= len_user;
    /*
     * Password Length field
     */
    *pos = (UBYTE)(/*lint -e(644) */len_pwd & 0x00ff);  
    pos++;
    written++;
    /*
     * Password field
     */
    memcpy(pos, password, len_pwd);
    pos    += len_pwd;
    written+= len_pwd;
    /*
     * fill length fields
     */
    *len_pos1 = (UBYTE)(pos - len_pos1 - 1);
    *len_pos2 = *len_pos1;
  }
  /*
   * DNS addresses
   */
  if((content & ACI_PCO_CONTENTMASK_DNS1) ||
     (content & ACI_PCO_CONTENTMASK_DNS2))
  {
    /*
     * set IPCP protocol
     */
    *pos = ACI_PCO_IPCP_PROT_MSB;
    pos++;
    written++;
    *pos = ACI_PCO_IPCP_PROT_LSB;
    pos++;
    written++;
    /*
     * store first length position
     */
    len_pos1 = pos;
    pos++;
    written++;
    /*
     * Code field
     */
    *pos = ACI_PCO_IPCP_CONF_REQ;
    pos++;
    written++;
    /*
     * Identifier field (just some value)
     */
    *pos = 0x01;
    pos++;
    written++;
    /*
     * Length field (store length position)
     */
    *pos = 0x00;
    pos++;
    written++;
    len_pos2 = pos;
    pos++;
    written++;
    /*
     * primary DNS address
     */
    if(content & ACI_PCO_CONTENTMASK_DNS1)
    {
      /*
       * Type field
       */
      *pos = ACI_PCO_IPCP_TYPE_DNS1;
      pos++;
      written++;
      /*
       * Length field
       */
      *pos = ACI_PCO_IPCP_LENGTH_DNS1;
      pos++;
      written++;
      /*
       * primary DNS address
       */
      *pos = (UBYTE)((dns1 >> 24) & 0x000000ff);
      pos++;
      written++;
      *pos = (UBYTE)((dns1 >> 16) & 0x000000ff);
      pos++;
      written++;
      *pos = (UBYTE)((dns1 >>  8) & 0x000000ff);
      pos++;
      written++;
      *pos = (UBYTE)(dns1         & 0x000000ff);
      pos++;
      written++;
    }
    /*
     * secondary DNS address
     */
    if(content & ACI_PCO_CONTENTMASK_DNS2)
    {
      /*
       * Type field
       */
      *pos = ACI_PCO_IPCP_TYPE_DNS2;
      pos++;
      written++;
      /*
       * Length field
       */
      *pos = ACI_PCO_IPCP_LENGTH_DNS2;
      pos++;
      written++;
      /*
       * primary DNS address
       */
      *pos = (UBYTE)((dns2 >> 24) & 0x000000ff);
      pos++;
      written++;
      *pos = (UBYTE)((dns2 >> 16) & 0x000000ff);
      pos++;
      written++;
      *pos = (UBYTE)((dns2 >>  8) & 0x000000ff);
      pos++;
      written++;
      *pos = (UBYTE)(dns2         & 0x000000ff);
      pos++;
      written++;
    }
    /*
     * fill length fields
     */
    *len_pos1 = (UBYTE)(pos - len_pos1 - 1);
    *len_pos2 = *len_pos1;
  }
  /*
   * pass written bytes to caller
   */
  *length = written;
  /*
   * return result
   */
  return 0;
} /* utl_create_pco() */



/*
  +-----------------------------------------------------------------------------
  | Function    : utl_analyze_pco
  +-----------------------------------------------------------------------------
  | Description : The function analyze_pco() analyzes the response protocol
  |               configuration from the network
  |
  | Parameters  : buffer  - buffer with the response PCOs
  |               length  - length of PCOs
  |               dns1    - returns primary DNS address
  |               dns2    - returns secondary DNS address
  |               gateway - returns Gateway address
  |
  | Return      :  0 - operation successful
  |               -1 - operation fails
  |
  +-----------------------------------------------------------------------------
*/
GLOBAL int utl_analyze_pco (UBYTE* buffer,
                            USHORT length,
                            ULONG* dns1,
                            ULONG* dns2,
                            ULONG* gateway)
{
  UBYTE*  pos;
  USHORT  pos_len;
  USHORT  packet_len;
  USHORT  start_pos;
  USHORT  dist_to_next;
  UBYTE   type_len;

  TRACE_FUNCTION( "utl_analyze_pco" );

  /*
   * initialize values
   */
  pos      = buffer;
  pos_len  = 0;
  *dns1    = 0;
  *dns2    = 0;
  *gateway = 0;
  /*
   * check configuration protocol
   */
  if( (length <= pos_len) ||
      !(*pos & 0x80) )
  {
    return -1;
  }

#if 0
  /* Table 10.5.154/3GPP TS 24.008 states:
       "All other values are interpreded as PPP in this version of the protocol.
     so the next check should not be performed
   */
  if((*pos & 0x07) NEQ ACI_PCO_CONFIG_PROT_PPP)
  {
    /*
     * configuration protocol is not PPP
     */
    return 0;
  }
#endif

  pos++;
  pos_len++;
  
  /*
   * search for IPCP packet
   */
  while(length > (pos_len + ACI_PCO_IPCP_OVERHEAD))
  {
    /*
     * initialize distance to next packet
     */
    dist_to_next = *(pos + 2) + 3;
    /*
     * check for IPCP packet
     */
    if((*pos       EQ ACI_PCO_IPCP_PROT_MSB) AND
       (*(pos + 1) EQ ACI_PCO_IPCP_PROT_LSB))
    {
      /*
       * check for correct length field
       */
      pos         += 2;
      pos_len     += 2;
      dist_to_next-= 2;
      packet_len   = *(pos + 3);
      packet_len   = packet_len << 8;
      packet_len  += *(pos + 4);
      if((packet_len EQ *pos) AND
         (length > (pos_len + packet_len)))
      {
        pos++;
        pos_len++;
        dist_to_next--;
        /*
         * check of code field
         */
        start_pos = pos_len;
        switch(*pos)
        {
          case ACI_PCO_IPCP_CONF_REQ:
            /*
             * search for Gateway address
             */
            pos    += 4;
            pos_len+= 4;
            while((pos_len + 1 - start_pos) < packet_len)
            {
              type_len = *(pos + 1);
              if((*pos     EQ ACI_PCO_IPCP_TYPE_IP) AND
                 (type_len EQ ACI_PCO_IPCP_LENGTH_IP) AND
                 ((pos_len +
                   ACI_PCO_IPCP_LENGTH_IP -
                   start_pos) <= packet_len))
              {
                *gateway = *(pos + 2);
                *gateway = ((*gateway) << 8);
                *gateway+= *(pos + 3);
                *gateway = ((*gateway) << 8);
                *gateway+= *(pos + 4);
                *gateway = ((*gateway) << 8);
                *gateway+= *(pos + 5);
              }
              pos    += type_len;
              pos_len+= type_len;
            }
            if((pos_len - start_pos) <= packet_len)
            {
              dist_to_next = packet_len + start_pos - pos_len;
            }
            else
            {
              dist_to_next = 0;
              pos         -= (pos_len - start_pos - packet_len);
              pos_len     -= (pos_len - start_pos - packet_len);
            }
            break;

          case ACI_PCO_IPCP_CONF_ACK:
          case ACI_PCO_IPCP_CONF_NAK:
            /*
             * search for DNS addresses
             */
            pos    += 4;
            pos_len+= 4;
            while((pos_len + 1 - start_pos) < packet_len)
            {
              type_len = *(pos + 1);
              if((*pos     EQ ACI_PCO_IPCP_TYPE_DNS1) AND
                 (type_len EQ ACI_PCO_IPCP_LENGTH_DNS1) AND
                 ((pos_len +
                   ACI_PCO_IPCP_LENGTH_DNS1 -
                   start_pos) <= packet_len))
              {
                *dns1 = *(pos + 2);
                *dns1 = ((*dns1) << 8);
                *dns1+= *(pos + 3);
                *dns1 = ((*dns1) << 8);
                *dns1+= *(pos + 4);
                *dns1 = ((*dns1) << 8);
                *dns1+= *(pos + 5);
              }
              else if((*pos     EQ ACI_PCO_IPCP_TYPE_DNS2) AND
                      (type_len EQ ACI_PCO_IPCP_LENGTH_DNS2) AND
                      ((pos_len +
                        ACI_PCO_IPCP_LENGTH_DNS2 -
                        start_pos) <= packet_len))
              {
                *dns2 = *(pos + 2);
                *dns2 = ((*dns2) << 8);
                *dns2+= *(pos + 3);
                *dns2 = ((*dns2) << 8);
                *dns2+= *(pos + 4);
                *dns2 = ((*dns2) << 8);
                *dns2+= *(pos + 5);
              }
              pos    += type_len;
              pos_len+= type_len;
            }
            if((pos_len - start_pos) <= packet_len)
            {
              dist_to_next = packet_len + start_pos - pos_len;
            }
            else
            {
              dist_to_next = 0;
              pos         -= (pos_len - start_pos - packet_len);
              pos_len     -= (pos_len - start_pos - packet_len);
            }
            break;
        }
      }
    }
    /*
     * go to next packet
     */
    pos_len+= dist_to_next;
    pos    += dist_to_next;
  }
  /*
   * return
   */
  return 0;
} /* utl_analyze_pco() */




GLOBAL int utl_strcasecmp (const char *s1, const char *s2)
{
  char c1, c2;
  int i;

  int len1 = (s1 EQ NULL) ? 0 : strlen(s1);
  int len2 = (s2 EQ NULL) ? 0 : strlen(s2);

  if (len1 > len2) 
  {
    return (1);
  }
  if(len1 < len2) 
  {
    return (-1);
  }
  for (i = 0; i < len1; i++)
  {
    c1 = s1[i]; /*lint !e613 (Warning -- 613: Possible use of null pointer 's1' in left argument to operator '[' [Reference: file condat\ms\src\aci\aci_util.c: line 2349] ) */
    c2 = s2[i]; /*lint !e613 (Warning -- 613: Possible use of null pointer 's2' in left argument to operator '[' [Reference: file condat\ms\src\aci\aci_util.c: line 2349] ) */
    if (c1 EQ c2)
    {
      continue ;
    }
    if (c1 >= 'a' AND c1 <= 'z')
    {
      c1 = c1 - ('a'-'A');
    }
    if (c2 >= 'a' AND c2 <= 'z')
    {
      c2 = c2 - ('a'-'A');
    }
    if (c1 NEQ c2)
    {
      return (1);
    }
  }
  return (0);
}


#endif /* GPRS */



#if defined (FF_ATI) || defined (FF_BAT)

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ACI_UTIL                 |
| STATE   : code                  ROUTINE : rci_display_USSD         |
+--------------------------------------------------------------------+

  PURPOSE : used by rCI_PlusCUSD and utl_cb_percentKSIR
*/
GLOBAL void rci_display_USSD (UBYTE            srcId,
                              T_ACI_CUSD_MOD   mode,
                              UBYTE            *ussd_str,
                              UBYTE            ussd_len,
                              BOOL             cvtStr,
                              SHORT            dcs )
{
  SHORT  pos = 0;
  USHORT chunksize;
  USHORT lenCvtdStr;
  CHAR sa[80];


  TRACE_FUNCTION("rci_display_USSD()");

  pos = sprintf(sa,"+CUSD: ");

  if (mode NEQ CUSD_MOD_NotPresent)
  {
    pos += sprintf(sa+pos,"%d,",mode);
  }
  else
    pos += sprintf(sa+pos,",");

  if(ussd_str NEQ NULL)
  {
    pos += sprintf(sa+pos,"\"");    /* beginning double quote */
    while (ussd_len)
    {
      if( cvtStr EQ CONVERT_STRING )
      {
        chunksize = utl_ussdDtaToTe( ussd_str,
                         (USHORT)ussd_len,
                         (UBYTE*)(sa + pos),
                         sizeof(sa)-pos,
                         &lenCvtdStr,
                         (UBYTE)dcs );

        pos += lenCvtdStr;
      }
      else
      {
        /*
        *   How much space do we have left in the output array?
        */
        chunksize=(sizeof(sa)-pos)-1;

        if (ussd_len<chunksize)
        {
          /*
          *   Remaining data fits within the output array.
          */
          chunksize=ussd_len;
        }
        else if (chunksize>3)
        {
          /*
          *   If the remaining data will not fit in the output
          *   array, and we have more than 3 bytes available (which
          *   we will), make sure that the amount of data output
          *   divides by 4. This is so we don't split UCS2 characters
          *   between two lines of output.
          */
          chunksize &= ~0x3;
        }

        memcpy(sa+pos,ussd_str,chunksize);
        
        pos += chunksize;

        sa[pos]=0;
      }
      ussd_len-=chunksize;
      ussd_str+=chunksize;

      if (ussd_len OR (sizeof(sa)-pos) < 10)
      {
#ifdef _SIMULATION_
        io_sendMessage(srcId, sa, ATI_NORMAL_OUTPUT);   /* append CR+LF only for testcase */
#else
        io_sendMessage(srcId, sa, ATI_ECHO_OUTPUT);     /* normal output a chunk */
#endif
        pos=0;
        memset(sa, 0, 80);
      }
    }

    pos+=sprintf(sa+pos,"\",");   /* ending double quote */

    if (dcs EQ ACI_NumParmNotPresent)
    {
      /* see 07.07: default is 0 */
      dcs = 0;
    }

    pos+=sprintf(sa+pos,"%d",dcs);
  }
  
  ci_remTrailCom(sa, pos);
  io_sendMessage(srcId, sa, ATI_NORMAL_OUTPUT);
}

/*
 ****************************************************************
 */
LOCAL void utl_cvtCLIRStat ( UBYTE clirOpt,
                             UBYTE ssSt,
                             T_ACI_CLIR_STAT *clirStat )
{
  if ( clirOpt EQ KSD_CO_NOT_VALID )
  {
    /* SS Status of class - T_ACI_KSD_ST */
    if ( ssSt EQ KSD_ST_NOT_VALID )
      *clirStat = CLIR_STAT_Unknown;
    else if ( ssSt & SSS_P )
      *clirStat = CLIR_STAT_Permanent;
    else
      *clirStat = CLIR_STAT_NotProv;
  }
  else
  {
    switch ( clirOpt )
    {
      case( CLIR_OPT_PERMANENT ):
        *clirStat = CLIR_STAT_Permanent;
        break;

      case( CLIR_OPT_TEMPORARY ):
        *clirStat = CLIR_STAT_RestrictTemp;
        break;

      case( CLIR_OPT_ALLOWED ):
        *clirStat = CLIR_STAT_AllowTemp;
        break;

      default:
        *clirStat = CLIR_STAT_Unknown;
        break;
    }
  }
}

/*
 ****************************************************************
 */
LOCAL  void utl_cb_ccbs( UBYTE srcId, T_ACI_KSIR * ksStat )
{
  T_ACI_CCBS_SET *CCBS_Setting = NULL;

  T_ACI_CCBS_STAT status;
  T_basicService basicServ;
  int i;
  T_CC_FEAT *ccbs_features;

  TRACE_FUNCTION("utl_cb_ccbs()");

  /* SS Status of CCBS */
  if (ksStat->ir.rKSCC.ssSt EQ KSD_ST_NOT_VALID)
    status = CCBS_STAT_NotPresent;
  else if (!(ksStat->ir.rKSCC.ssSt & SSS_P))
    status = CCBS_STAT_NotProvisioned;
  else if (ksStat->ir.rKSCC.ssSt & SSS_A)
    status = CCBS_STAT_Active;
  else
    status = CCBS_STAT_Provisioned;

  ACI_MALLOC(CCBS_Setting, sizeof(T_ACI_CCBS_SET));

  /* CCBS_Setting infos */
  if (!ksStat->ir.rKSCC.c_ccFeatLst)
  {
    TRACE_EVENT("KSD_CMD_CCBS: no feature list");

    if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
    {
      rCI_PercentCCBS(CCBS_IND_IrgtResult,
                      status,
                      CCBS_Setting,
                      TRUE);
    }
#ifdef FF_BAT
    else /* CMD_MODE_BAT */
    {
      rBAT_PercentCCBS(CCBS_IND_IrgtResult,
                       status,
                       CCBS_Setting,
                       TRUE);
   }
#endif
    return;
  }

  for (i=0; i<ksStat->ir.rKSCC.c_ccFeatLst; i++)
  {
    ccbs_features = ksStat->ir.rKSCC.ccFeatLst + i;

    memcpy( CCBS_Setting->number, ccbs_features->num, MAX_B_SUBSCR_NUM_LEN);
    memcpy( CCBS_Setting->subaddr, ccbs_features->sub, MAX_SUBADDR_LEN);

    CCBS_Setting->type.npi   = ccbs_features->npi NEQ 0xFF 
                               ? (T_ACI_TOA_NPI)ccbs_features->npi 
                               : NPI_NotPresent;
    CCBS_Setting->type.ton   = ccbs_features->ton NEQ 0xFF 
                               ? (T_ACI_TOA_TON)ccbs_features->ton 
                               : TON_NotPresent;
    CCBS_Setting->satype.tos = ccbs_features->tos NEQ 0xFF 
                               ? (T_ACI_TOS_TOS)ccbs_features->tos 
                               : TOS_NotPresent;
    CCBS_Setting->satype.oe  = ccbs_features->oe NEQ 0xFF 
                               ? (T_ACI_TOS_OE)ccbs_features->oe 
                               : OE_NotPresent;

    if( ccbs_features->bsTp EQ BS_TELE_SRV )
    {
      basicServ.v_teleservice = TRUE;
      basicServ.v_bearerService = FALSE;
      basicServ.teleservice = ccbs_features->bsCd;
    }
    else /* if( p_servType EQ BS_BEAR_SRV ) */
    {
      basicServ.v_bearerService = TRUE;
      basicServ.v_teleservice = FALSE;
      basicServ.bearerService = ccbs_features->bsCd;
    }
    
    CCBS_Setting->class_type = cmhSS_GetClass( &basicServ );

    CCBS_Setting->idx = ccbs_features->idx NEQ 0xFF 
                        ? ccbs_features->idx 
                        : ACI_NumParmNotPresent;
    
    CCBS_Setting->alrtPtn = ALPT_NotPresent;
    
    if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
    {
      rCI_PercentCCBS(CCBS_IND_IrgtResult,
                      status,
                      CCBS_Setting,
                      TRUE);
    }
#ifdef FF_BAT
    else /* CMD_MODE_BAT */
    {
      rBAT_PercentCCBS(CCBS_IND_IrgtResult,
                      status,
                      CCBS_Setting,
                      TRUE);
    }
#endif
  }

  ACI_MFREE( CCBS_Setting );
}

/*
 ****************************************************************
 */
LOCAL void utl_cb_callwaiting( UBYTE srcId, T_ACI_KSIR *ksStat )
{
  T_ACI_CLSSTAT   classStat;
  UBYTE           idx;
  T_ACI_TOA       type;

  TRACE_FUNCTION("utl_cb_callwaiting()");

  if ( ksStat->ir.rKSCW.opCd NEQ KSD_OP_IRGT )
    return;

  /* SS Status of class - T_ACI_KSD_ST */
  if ( ksStat->ir.rKSCW.ssSt EQ KSD_ST_NOT_VALID )
  {
    classStat.status = STATUS_NotPresent;
  }
  else if ( ksStat->ir.rKSCW.ssSt & KSD_ST_A )
  {
    classStat.status = STATUS_Active;
  }
  else
  {
    classStat.status = STATUS_NotActive;
  }

  /* init parameter type: */
  type.npi = NPI_NotPresent;
  type.ton = TON_NotPresent;

  for( idx = 0; idx < ksStat->ir.rKSCW.c_cwBSGLst; idx++ )
  {
    /* type of class */
    classStat.class_type = cmhSS_GetClassType(ksStat->ir.rKSCW.cwBSGLst[idx].bsTp,
                                              ksStat->ir.rKSCW.cwBSGLst[idx].bsCd);
    if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
    {
    }
    else /* CMD_MODE_BAT */
    {
    }
    
    rCI_PlusCCWA(&classStat,"\0", &type, MNCC_PRES_NOT_PRES, CLASS_NotPresent, NULL);
  }
}

/*
 ****************************************************************
 */
LOCAL  void utl_cb_lineidentification( UBYTE srcId, T_ACI_KSIR * ksStat )
{
  T_ACI_CLIP_STAT clipStat;
  T_ACI_CLIR_MOD  clirMode;
  T_ACI_CLIR_STAT clirStat;
  T_ACI_COLP_STAT colpStat;
  T_ACI_COLR_STAT colrStat;

  TRACE_FUNCTION("utl_cb_lineidentification()");

  if ( ksStat->ir.rKSCL.opCd NEQ KSD_OP_IRGT )
    return;

  switch ( ksStat->ir.rKSCL.ssCd )
  {
    case KSD_SS_CLIP:
     {
      /* SS Status of class - T_ACI_KSD_ST */
      if ( ksStat->ir.rKSCL.ssSt EQ KSD_ST_NOT_VALID )
        clipStat = CLIP_STAT_Unknown;
      else if ( ksStat->ir.rKSCL.ssSt & SSS_P )
        clipStat = CLIP_STAT_Prov;
      else
        clipStat = CLIP_STAT_NotProv;

      if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
      {
        rCI_PlusCLIP ( clipStat, NULL, NULL, MNCC_PRES_NOT_PRES, NULL, NULL, NULL );
      }
#ifdef FF_BAT
      else /* CMD_MODE_BAT */
      {
        rBAT_PlusCLIP ( clipStat, NULL, NULL, MNCC_PRES_NOT_PRES, NULL, NULL, NULL );
      }
#endif
      break;
    }
    case KSD_SS_CLIR:
    {
      clirMode = ksStat->ir.rKSCL.mode;
      utl_cvtCLIRStat(ksStat->ir.rKSCL.clirOpt,
                      ksStat->ir.rKSCL.ssSt,
                      &clirStat);
      if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
      {
        rCI_PlusCLIR (clirMode, clirStat);
      }
#ifdef FF_BAT
      else /* CMD_MODE_BAT */
      {
        rBAT_PlusCLIR (clirMode, clirStat);
      }
#endif
      break;
    } 
    case KSD_SS_COLP:
    {
      /* SS Status of class - T_ACI_KSD_ST */
      if ( ksStat->ir.rKSCL.ssSt EQ KSD_ST_NOT_VALID )
        colpStat = COLP_STAT_Unknown;
      else if ( ksStat->ir.rKSCL.ssSt & SSS_P )
        colpStat = COLP_STAT_Prov;
      else
        colpStat = COLP_STAT_NotProv;

      if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
      {
        rCI_PlusCOLP ( colpStat, NULL, NULL, NULL, NULL, NULL );
      }
#ifdef FF_BAT
      else /* CMD_MODE_BAT */
      {
        rBAT_PlusCOLP ( colpStat, NULL, NULL, NULL, NULL, NULL );
      }
#endif
      break;
    } 
    case KSD_SS_COLR:
    {
      /* SS Status of class - T_ACI_KSD_ST */
      if ( ksStat->ir.rKSCL.ssSt EQ KSD_ST_NOT_VALID )
        colrStat = COLR_STAT_Unknown;
      else if ( ksStat->ir.rKSCL.ssSt & SSS_P )
        colrStat = COLR_STAT_Prov;
      else
        colrStat = COLR_STAT_NotProv;
      
      if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
      {
        rCI_PercentCOLR(colrStat);
      }
#ifdef FF_BAT
      else /* CMD_MODE_BAT */
      {
        rBAT_PercentCOLR(colrStat);
      } 
#endif
      break;
    }
  }
}

/*
 ****************************************************************
 */
LOCAL  void utl_cb_callforwarding( UBYTE srcId, T_ACI_KSIR * ksStat )
{
  T_ACI_CCFC_SET  ccfcSetting;
  UBYTE           idx;

  TRACE_FUNCTION("utl_cb_callforwarding()");

  if ( ksStat->ir.rKSCF.opCd NEQ KSD_OP_IRGT )
    return;

  for( idx = 0; idx < ksStat->ir.rKSCF.c_cfFeatLst; idx++ )
  {
    /* SS Status of class - T_ACI_KSD_ST */
    if ( ksStat->ir.rKSCF.cfFeatLst[idx].ssSt EQ KSD_ST_NOT_VALID )
      ccfcSetting.clsstat.status = STATUS_NotPresent;
    else if ( ksStat->ir.rKSCF.cfFeatLst[idx].ssSt & SSS_A )
      ccfcSetting.clsstat.status = STATUS_Active;
    else
      ccfcSetting.clsstat.status = STATUS_NotActive;
    /* type of class */
    if ( ksStat->ir.rKSCF.cfFeatLst[idx].bsTp EQ KSD_BS_TP_None
     AND ksStat->ir.rKSCF.cfFeatLst[idx].bsCd EQ KSD_BS_TeleBearerUnknown)
    {
      if (ccfcSetting.clsstat.status EQ STATUS_NotPresent)
        ccfcSetting.clsstat.class_type = CLASS_None;
      else
        ccfcSetting.clsstat.class_type = CLASS_VceDatFax;
    }
    else
      ccfcSetting.clsstat.class_type = cmhSS_GetClassType(ksStat->ir.rKSCF.cfFeatLst[idx].bsTp,
                                                           ksStat->ir.rKSCF.cfFeatLst[idx].bsCd);
    /* number */
    strcpy(ccfcSetting.number, (char *)ksStat->ir.rKSCF.cfFeatLst[idx].num);
    if ( !strlen ( ccfcSetting.number ))
    {
      ccfcSetting.type.npi   = NPI_NotPresent;
      ccfcSetting.type.ton   = TON_NotPresent;
      ccfcSetting.subaddr[0] = 0x0;
      ccfcSetting.satype.tos = TOS_NotPresent;
      ccfcSetting.satype.oe  = OE_NotPresent;
      ccfcSetting.time       = ACI_NumParmNotPresent;
    }
    else
    {
      T_CF_FEAT *cfFeat = &ksStat->ir.rKSCF.cfFeatLst[idx];
      /* toa */
      ccfcSetting.type.ton = cfFeat->ton NEQ 0xFF 
                             ? (T_ACI_TOA_TON)cfFeat->ton 
                             : TON_NotPresent;
      ccfcSetting.type.npi = cfFeat->npi NEQ 0xFF 
                             ? (T_ACI_TOA_NPI)cfFeat->npi 
                             : NPI_NotPresent;
      /* subaddr */
      strcpy(ccfcSetting.subaddr, (char *)ksStat->ir.rKSCF.cfFeatLst[idx].sub);
      /* tos */
      ccfcSetting.satype.tos = cfFeat->tos NEQ 0xFF
                               ? (T_ACI_TOS_TOS)cfFeat->tos
                               : TOS_NotPresent;
      ccfcSetting.satype.oe  = cfFeat->oe NEQ 0xFF 
                               ? (T_ACI_TOS_OE)cfFeat->oe 
                               : OE_NotPresent;
      /* time */
      ccfcSetting.time       = cfFeat->time NEQ 0xFF 
                               ? cfFeat->time 
                               : ACI_NumParmNotPresent;
    }
    if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
    {
      rCI_PlusCCFC (&ccfcSetting);
    }
#ifdef FF_BAT
    else
    {
      rBAT_PlusCCFC (&ccfcSetting);
    }
#endif
  }
}

/*
 ****************************************************************
 */
LOCAL  void utl_cb_callbarring( UBYTE srcId, T_ACI_KSIR * ksStat )
{
  T_ACI_CLSSTAT   classStat;
  UBYTE           idx;

  TRACE_FUNCTION("utl_cb_callbarring()");

  if (ksStat->ir.rKSCB.opCd NEQ KSD_OP_IRGT)
  {
    return;
  }
  for (idx = 0; idx < ksStat->ir.rKSCB.c_cbInfoLst; idx++)
  {
    /* type of class */
    classStat.class_type = cmhSS_GetCbClassType(ksStat->ir.rKSCB.cbInfoLst[idx].bsTp,
                                                 ksStat->ir.rKSCB.cbInfoLst[idx].bsCd);

    /* SS Status of class */
    if (classStat.class_type EQ CLASS_VceDatFaxSms 
     OR classStat.class_type EQ CLASS_None)
    {
      classStat.status = STATUS_NotActive;
    }
    else
    {
      classStat.status = STATUS_Active;
    }
    if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
    {
      rCI_PlusCLCK(&classStat);
    }
#ifdef FF_BAT
    else /* CMD_MODE_BAT */
    {
      rBAT_PlusCLCK(&classStat);
    }
#endif
  }
}

/*
 ****************************************************************
 */
LOCAL  void utl_cb_imei( UBYTE srcId, T_ACI_KSIR * ksStat )
{
  TRACE_FUNCTION("utl_cb_imei()");

  sprintf ( g_sa,
            "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
            (ksStat->ir.rKSIMEI.tac1 >> 4) & 0x0F,
             ksStat->ir.rKSIMEI.tac1 & 0x0F,

            (ksStat->ir.rKSIMEI.tac2 >> 4) & 0x0F,
             ksStat->ir.rKSIMEI.tac2 & 0x0F,

            (ksStat->ir.rKSIMEI.tac3 >> 4) & 0x0F,
             ksStat->ir.rKSIMEI.tac3 & 0x0F,

            (ksStat->ir.rKSIMEI.fac  >> 4) & 0x0F,
             ksStat->ir.rKSIMEI.fac  & 0x0F,

            (ksStat->ir.rKSIMEI.snr1 >> 4) & 0x0F,
             ksStat->ir.rKSIMEI.snr1 & 0x0F,

            (ksStat->ir.rKSIMEI.snr2 >> 4) & 0x0F,
             ksStat->ir.rKSIMEI.snr2 & 0x0F,

            (ksStat->ir.rKSIMEI.snr3 >> 4) & 0x0F,
             ksStat->ir.rKSIMEI.snr3 & 0x0F,

             ksStat->ir.rKSIMEI.cd,
            (ksStat->ir.rKSIMEI.svn  >> 4) & 0x0F,
             ksStat->ir.rKSIMEI.svn  & 0x0F );

  io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_UTIL           |
| STATE   : code                        ROUTINE :                    |
+--------------------------------------------------------------------+

  PURPOSE : handles PercentKSIR call back

*/
GLOBAL void utl_cb_percentKSIR (U8 srcId, T_ACI_KSIR *ksStat)
{
#ifdef FF_BAT
  T_ACI_USSD_DATA  ussd;
#endif /* FF_BAT */

  TRACE_FUNCTION("utl_cb_percentKSIR()");
  
  switch (ksStat->ksdCmd)
  {
    case (KSD_CMD_IMEI):
      if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
      {
        utl_cb_imei(srcId, ksStat);
      }
#ifdef FF_BAT
      else /* CMD_MODE_BAT */
      {
        rBAT_PercentIMEI(&(ksStat->ir.rKSIMEI));
      }
#endif
      break;

    case (KSD_CMD_CB):
      utl_cb_callbarring(srcId, ksStat);
      break;

    case (KSD_CMD_CF):
      utl_cb_callforwarding(srcId, ksStat);
      break;

    case (KSD_CMD_CL):
      utl_cb_lineidentification(srcId, ksStat);
      break;

    case (KSD_CMD_CW):
      utl_cb_callwaiting(srcId, ksStat);
      break;

    case (KSD_CMD_PWD):
      /* no specific answer to +CPWD which is the corresponding AT cmd */
      break;

    case (KSD_CMD_UBLK):
      /* no specific answer to +CPIN which is the corresponding AT cmd */
      break;

    case (KSD_CMD_USSD):
      if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)srcId) EQ CMD_MODE_ATI)
      {
        rci_display_USSD(srcId,
                         ksStat->ir.rKSUS.mode,
                         (UBYTE *)ksStat->ir.rKSUS.ussd,
                         ksStat->ir.rKSUS.len,
                         FALSE, /* means DO NOT CONVERT STRING */
                         ksStat->ir.rKSUS.dcs);
      }
#ifdef FF_BAT
      else /* CMD_MODE_BAT */
      {
        ussd.len = ksStat->ir.rKSUS.len;
        memcpy (ussd.data, (UBYTE *)ksStat->ir.rKSUS.ussd, ussd.len);
        rBAT_PlusCUSD(ksStat->ir.rKSUS.mode, &ussd, ksStat->ir.rKSUS.dcs);
      }
#endif
      break;

    case (KSD_CMD_CCBS):
      utl_cb_ccbs(srcId, ksStat);
      break;
  }

}
#endif /* defined (FF_ATI) || defined (FF_BAT) */
