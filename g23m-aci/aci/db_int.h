/* 
+----------------------------------------------------------------------------- 
|  Project :  PHB
|  Modul   :  DBM
+----------------------------------------------------------------------------- 
|  Copyright 2005 Texas Instruments Berlin, AG 
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
|  Purpose :  Internal declarations for DBM module
+----------------------------------------------------------------------------- 
*/ 
#ifndef DB_INT_H
#define DB_INT_H

#include "db.h"

/* This macro allows us to close the file before
   returning from function */
#define FFS_OPEN_PROBLEM_PATCH

/* If this macro is activated, we close earlier activated file
   before opening next file */
#define FFS_CLOSE_BEFORE_OPEN

/* Mutually exclusive macros */
#ifdef FFS_CLOSE_BEFORE_OPEN
#undef FFS_OPEN_PROBLEM_PATCH
#endif

#ifdef FFS_CLOSE_BEFORE_OPEN

  #define MAX_OPEN_READ_PER_DB 2
  #define MAX_OPEN_WRITE_PER_DB 1

#endif

#ifdef _SIMULATION_
  #include "ffs_pc_api.h"
#else
  #include "ffs/ffs.h"
#endif

#include "vsi.h"
#include "aci_mem.h"

#define RECORD_BITMAP_SIZE      32  /* Based on MAX_NUM_RECORDS 
                                        RECORD_BITMAP_SIZE = MAX_NUM_RECORDS/8*/
#define MAX_LEN_DIRECTORY       16  /* Assumed to be max 16 characters        */


/* 
    DBM state 
 */
typedef enum
{
  
  DBM_NOT_INITIALISED = 0, 
  DBM_INITIALISED 

} T_DBM_STATE;

/* 
    Database state 
*/
typedef enum
{ 

  CLOSED = 0, 
  OPEN,
  IN_USE,
  UNUSED_ENTRY = 0xFF

} T_DB_STATE;

#ifdef FFS_CLOSE_BEFORE_OPEN

typedef struct
{

  UBYTE    fld_ctr;
  T_FFS_FD filehandle;

} FileHandleRecord;

#endif

/* 
    Field record
 */
typedef struct
{

  USHORT    FieldID;                                /* Elementary file identifier                           */
  T_DB_TYPE DBType;                                 /* database type (UNMANAGED, FREELIST)                  */
  USHORT    RecordSize;                             /* Size of data in data base record                     */
  UBYTE     NumOfRecords;                           /* Number of records                                    */
  UBYTE     UsedRecords;                            /* Number of used records                               */
  UBYTE     RecordBitMap [RECORD_BITMAP_SIZE];      /* Bitmap representing whether record is present or not */
  BOOL      Clean;                                  /* File consistency                                     */
  UBYTE     SortIndexList [MAX_NUM_OF_SORT_INDEXS]; /* Number of associated index files                     */
  UBYTE*    SortedLists [MAX_NUM_OF_SORT_INDEXS];   /* Sorted lists                                         */

#ifndef FFS_CLOSE_BEFORE_OPEN
  T_FFS_FD  FFSFileHandle;                          /* FFS File Handle for User file                        */
#endif

  UBYTE     NextRecordNum;                          /* To take care of non-seqential write                  */

} T_DBM_FIELDRECORD;


typedef struct 
{

  UBYTE               DBDirectory[MAX_LEN_DIRECTORY]; /* Directory path                                       */
  UBYTE               NumOfFiles;                     /* Maximum files in that database                       */
  UBYTE               UsedFiles;                      /* Number of used files                                 */
  BOOL                Clean;                          /* Clean: Database is consistent                        */
  BOOL                Tracked;                        /* Tracked: History log to be maintained                */
  T_DB_STATE          DBState;                        /* Database state                                       */

  T_FFS_FD            FFSFileHandle;                  /* FFS File Handle for DD_<db_handle>                   */

#ifdef FFS_CLOSE_BEFORE_OPEN
  FileHandleRecord READ_OPEN_FIELDS [MAX_OPEN_READ_PER_DB];   /* fields opened for read only                  */
  FileHandleRecord WRITE_OPEN_FIELDS [MAX_OPEN_WRITE_PER_DB]; /* fields opened for read/write both            */
  UBYTE  old_read;                                            /* remeber oldest open                          */
  UBYTE  old_write;                                           /* remeber oldest read                          */
#endif

  T_DBM_FIELDRECORD*  ptrFieldRecord;                 /* pointer to fields                                    */

} T_DBM_MASTERRECORD;

#endif
