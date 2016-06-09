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
|  Purpose :  Interface betweeen ACI_SIM and DBM module
+----------------------------------------------------------------------------- 
*/ 
#ifndef DB_H
#define DB_H

#include "typedefs.h"
#define DB_MAX_AFFECTED 5           /* As per architecture document */

#define MAX_DBs                 2   /* This needs to increased if DBM is used 
                                       for storing new databases (say SMSs)   */

#define MAX_NUM_FILES           255 /* As per architecture document           */

#define MAX_NUM_OF_SORT_INDEXS  4   /* As per architecture document           */

#define MAX_NUM_RECORDS         254 /* As per architecture document           */

#define INVALID_FIELD_ID        0xFFFF  

#define INVALID_SORT_INDEX      0xFF  

/* 
    Database type 
 */
typedef enum
{

  DB_UNMANAGED, /* Not managed (1:1 slave of another file)  */
  DB_FREELIST   /* Fast free list maintained                */

} T_DB_TYPE;

/* 
    Database affected records (History log) 
 */
typedef struct
{

  USHORT entries;                     /* Number of affected entries                 */
  USHORT field_id[DB_MAX_AFFECTED];   /* Elementary file, e.g. EF_ADN               */
  USHORT record[DB_MAX_AFFECTED];     /* Corresponding record within elementary file*/

} T_DB_CHANGED;

/* 
    Database Information 
 */
typedef struct
{
  BOOL clean;   /* Database consistent                  */
  BOOL tracked; /* Tracked for external storage device  */
} T_DB_INFO;

/* 
    File Information 
 */
typedef struct
{

  BOOL        clean;            /* Database consistent    */
  T_DB_TYPE   entry_type;       /* Type of database field */
  USHORT      record_size;      /* Record size            */
  USHORT      num_records;      /* Number of records      */
  USHORT      used_records;     /* Number of used records */

} T_DB_INFO_FIELD;

/* 
    Search node
    Note: This node is internal to DBM.
 */
typedef struct
{

  UBYTE top;
  UBYTE bottom;

} T_DB_SEARCH_NODE;

/* 
    Database return codes 
 */ 
typedef enum
{

  DB_OK                 =  0, /* Execution of command completed         */
  DB_FAIL               = -1, /* Execution of command failed within db  */
  DB_FAIL_FS            = -2, /* Unexpected failure of FFS              */
  DB_FULL               = -3, /* Database is full                       */
  DB_INVALID_DB         = -4, /* Database handle not known              */
  DB_INVALID_FIELD      = -5, /* Invalid index requested                */
  DB_INVALID_RECORD     = -6, /* Invalid record requested               */
  DB_INVALID_INDEX      = -7, /* Invalid index requested                */
  DB_INVALID_SIZE       = -8, /* Invalid size given                     */
  DB_EMPTY_RECORD       = -9, /* Attempted to read a free record        */

/* Architecture document need to be updated for following constants     */

  DB_EXISTS             = -10,/* Database already exists                */
  DB_FIELD_EXISTS       = -11,/* Elementary file already exists         */
  DB_HISTORY_FULL       = -12,/* Change log full                        */
  DB_NOT_INITIALISED    = -13,/* Database not initialised               */
  DB_IN_USE             = -14,/* Database in use                        */
  DB_RECORD_NOT_FOUND   = -15 /* Record not found                       */

} T_DB_CODE;


/* 
   This function is used in db_create_index
 */

typedef int (*T_COMP_FUNC)( int db_handle, 
                            USHORT field_id, 
                            USHORT record_1, 
                            USHORT record_2,
                            ULONG  flags );

/* 
   This function is used for db_search
 */

typedef int (*T_SEARCH_FUNC)( ULONG Flags, 
                              const UBYTE* Search_tag, 
                              int db_handle, 
                              USHORT field_id,
                              USHORT record_num );

/* 
    Interface Functions 
 */

EXTERN void db_init ( void );

EXTERN int db_create ( const char* directory,
                       UBYTE       num_of_fields,
                       BOOL        tracked );

EXTERN int db_open ( const char* directory );

EXTERN T_DB_CODE db_create_field ( int        db_handle, 
                                   T_DB_TYPE  db_type, 
                                   USHORT     field_id,
                                   USHORT     record_size, 
                                   USHORT     num_of_records );

EXTERN int db_write_record ( int          db_handle, 
                             USHORT       field_id, 
                             USHORT       record_num, 
                             USHORT       offset, 
                             USHORT       length, 
                             const UBYTE* buffer );

EXTERN int db_update_ext_bitmap(int          db_handle, 
                               USHORT       field_id, 
                               USHORT       record_num,
                               BOOL           flag); 


/* 
1) If ACI would like to use incremental sort, it needs to call
   db_update_index at end of writing/deleting the record(s).
   If the index does not exist, it would be created.
 
   Note that ACI can call this function after every record or
   after 'n' records (n>1) and DB would process it properly.
 
2) If ACI would like to use quick sort, it can go for db_create_index
   at any moment. 
 
   db_create_index is compatible with db_update_index and vice versa.

*/
EXTERN T_DB_CODE db_create_index ( int          db_handle, 
                                   USHORT       field_id, 
                                   UBYTE        sort_index, 
                                   T_COMP_FUNC  compare_function, 
                                   ULONG        flags );

EXTERN T_DB_CODE db_update_index ( int          db_handle, 
                                   USHORT       field_id, 
                                   UBYTE        sort_index, 
                                   T_COMP_FUNC  compare_function, 
                                   ULONG        flags );

EXTERN T_DB_CODE db_get_phy_from_idx ( int    db_handle, 
                                       USHORT field_id, 
                                       UBYTE  sort_index, 
                                       USHORT order_num );

EXTERN T_DB_CODE db_flush (int db_handle);

EXTERN int db_read_record ( int     db_handle, 
                            USHORT  field_id, 
                            USHORT  record_num, 
                            USHORT  offset, 
                            USHORT  length, 
                            UBYTE*  buffer );

EXTERN T_DB_CODE db_read_change_log ( int           db_handle, 
                                      T_DB_CHANGED* changed);

EXTERN T_DB_CODE db_delete_record ( int    db_handle,
                                    USHORT field_id, 
                                    USHORT record_num );

EXTERN T_DB_CODE db_info ( int        db_handle, 
                           T_DB_INFO* db_info );

EXTERN T_DB_CODE db_info_field ( int              db_handle,  
                                 USHORT           field_id, 
                                 T_DB_INFO_FIELD* info_field );

EXTERN int db_find_free_record  ( int    db_handle,
                                  USHORT field_id );

EXTERN T_DB_CODE db_field_changed ( int    db_handle, 
                                    USHORT field_id,
                                    BOOL*  changed );

EXTERN T_DB_CODE db_record_empty ( int    db_handle,
                                   USHORT field_id,
                                   USHORT record_num,
                                   BOOL*  empty );

EXTERN int db_search ( int               db_handle, 
                       USHORT            field_id, 
                       UBYTE             sort_index, 
                       SHORT*            order_num,
                       T_SEARCH_FUNC     search_function, 
                       ULONG             flags, 
                       const UBYTE*      search_tag );

EXTERN int db_get_last_fs_error ( void );

EXTERN T_DB_CODE db_close ( int db_handle );

EXTERN void db_exit ( void );

EXTERN T_DB_CODE db_remove_index  ( int    db_handle,
                                    USHORT field_id,
                                    UBYTE  sort_index );

EXTERN T_DB_CODE db_remove_field ( int    db_handle, 
                                   USHORT field_id );

EXTERN T_DB_CODE db_remove ( const char* directory );

#endif
