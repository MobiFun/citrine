/*
+------------------------------------------------------------------------------
|  File:       printtofile.h
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
+------------------------------------------------------------------------------
| Purpose:    Definitions for the TST internal header 
+------------------------------------------------------------------------------
*/

#ifndef PRINTTOFILE_H
#define PRINTTOFILE_H

/*==== INCLUDES =============================================================*/


/*==== CONSTS ===============================================================*/


/*==== TYPES =================================================================*/



/*==== EXPORTS ===============================================================*/

void initPrintToFile();
int PrintToFile( const char *format, ... );



#endif /* !PRINTTOFILE_H */
