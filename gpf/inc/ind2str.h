/* 
+----------------------------------------------------------------------------- 
|  Project :  STR2IND
|  Modul   :  ind2str.h
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
|  Purpose :  This Module defines interface of the ind2str library. The
|             main purpose of this library is the translation of an trace
|             index to a complete trace string.
+----------------------------------------------------------------------------- 
*/ 

#ifndef IND2STR_H
#define IND2STR_H


/*==== CONSTANTS ==================================================*/
#define IND2STR_OK                      0
#define IND2STR_FILE_NOT_FOUND         -1
#define IND2STR_WRONGFILEFORMAT        -2
#define IND2STR_INDEXOUTOFBOUNDS       -3
#define IND2STR_PARAMCOUNTOUTOFBOUNDS  -4
#define IND2STR_NOTINITIALISED         -5
#define IND2STR_ACCESSVIOLATION        -6
#define IND2STR_INTERNAL_ERROR         -7

#ifdef __cplusplus
extern "C" {
#endif



/*-----------------------------------------------------------------------------
|  Function    : ind2str_Init()
+------------------------------------------------------------------------------
|  Description : Initialises the library: opens the file containing the trace
|                mapping table, build the internal representation , closes the
|                the file after reading.
|
|  Parameters  : tableFile  - the mapping table file name
|                 
|  Return      : success or failure
+------------------------------------------------------------------------------
*/
signed char ind2str_Init(char * tableFile);


/*-----------------------------------------------------------------------------
|  Function    : ind2str_Exit()
+------------------------------------------------------------------------------
|  Description : De-initialises the library: closes the file containing the
|                trace mapping table, frees allocated memory.
|
|  Parameters  : 
|                 
|  Return      : 
+------------------------------------------------------------------------------
*/
void ind2str_Exit(void);


/*-----------------------------------------------------------------------------
|  Function    : ind2str_Version()
+------------------------------------------------------------------------------
|  Description : Returns the version (build date) of the table. The table must
|                be initialized before.
|
|  Parameters  : 
|                 
|  Return      : The version of the table or 0.
+------------------------------------------------------------------------------
*/
unsigned long ind2str_Version(void);


/*-----------------------------------------------------------------------------
|  Function    : ind2str_VersionNoInit()
+------------------------------------------------------------------------------
|  Description : Returns the version (build date) of the table. The table does
|                not need to be initialized before.
|
|  Parameters  : fileName - the name of the file, that contains a table
|                 
|  Return      : The version of the table or 0.
+------------------------------------------------------------------------------
*/
unsigned long ind2str_VersionNoInit(char * fileName);


/*-----------------------------------------------------------------------------
|  Function    : ind2str
+------------------------------------------------------------------------------
|  Description : Transforms a trace index string into a trace string, according
|                to the trace mapping table.
|
|  Parameters  : dst - destination string
|
|                src - source string
|                            
|
|  Return      : The number of characters printed to the destination string.
|                
+------------------------------------------------------------------------------
*/
int ind2str(char * dst, char * src);


#ifdef __cplusplus
}
#endif


#endif /* IND2STR_H */

