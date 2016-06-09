/* 
+------------------------------------------------------------------------------
|  File:       tools.h
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
|  Purpose :  Definitions and prototypes.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __TOOLS_H__
#define __TOOLS_H__

/*==== INCLUDES =============================================================*/

/*==== CONSTS ===============================================================*/

#define CHARS_FOR_8BIT       2
#define CHARS_FOR_16BIT      4
#define CHARS_FOR_32BIT      8

/*==== TYPES ================================================================*/


/*==== PROTOTYPES ===========================================================*/

unsigned int GetNextToken (char *source, char *token, char const *seperators);
char *HexToASCII (ULONG value, char *ptr, int num);
unsigned int ASCIIToHex (char *p, int num);
char* InsertString (char *string, char *p, int num);
char *rm_path ( const char *file );

#endif                

