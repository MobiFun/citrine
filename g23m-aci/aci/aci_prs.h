/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_PRS
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
|  Purpose :  Definitions for the ACI parameter parser
+----------------------------------------------------------------------------- 
*/ 

#ifndef ACI_PRS_H
#define ACI_PRS_H

#define  NO_TIME_STAMP_FIELD   0
#define  MAX_TIME_STAMP_FIELDS 8


EXTERN char *parseByte (int digits, char *b, U8 *i);

EXTERN UBYTE parseTimeStamp( char *p_in,U8 *years,U8 *months,U8 *days,
              U8 *hrs, U8 *mins, U8 *secs, char *sign, U8 *time_zone);

/*EXTERN char *parse (); */
EXTERN char   *parse(char *b,char *f, ...);
EXTERN UBYTE get_parse_index ();



#endif
