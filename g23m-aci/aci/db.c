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
|  Purpose :  Implementation of DBM functions
+----------------------------------------------------------------------------- 
*/ 

#ifdef TI_PS_FFS_PHB

#include "db_int.h"
#include <string.h>

#ifdef _SIMULATION_
int sprintf (char *, const char *, ...); /* Use appropriate include ... */
#endif

/* For DB Testing */

/* Check if we are closing FFS files properly */
/* Implements measure 54 */

/* A simple check to make sure that all allocated 
   memory is freed to avoid memory leak */
// #define DB_MEMORY_CHECK

/**************************
  D E F I N I T I O N S & 
  D E C L A R A T I O N S
 **************************/

#define INVALID_FD      (-1)
#define IS_FD_VALID(fd) ((fd) >= 0)

GLOBAL UBYTE              UsedDBs;								          /* Number of existing databases  */
GLOBAL int                LastFFS_ReturnCode;			          /* Last FFS return code          */
GLOBAL T_DBM_MASTERRECORD DbmMaster [MAX_DBs];              /* Database Master               */
GLOBAL T_DBM_STATE        DBM_State = DBM_NOT_INITIALISED;  /* DBM State                     */
GLOBAL T_DB_CHANGED       DB_History_log;                   /* History log                   */
/* Implements Measure#32: Row 1167, 1171, 1189 & 1200 */
LOCAL const char * const format_sUDd_str="%s/UD_%04X";
/* Implements Measure#32: Row 1170, 1177 & 1193 */
LOCAL const char * const format_sUDd_sortd_str="%s/UD_%04X_sort_%d";
/* Implements Measure#32: Row 1178, 1182, 1186, 1190, 1195, 1198 & 1202 */
LOCAL const char * const format_sDDd_str="%s/DD_%d";

#ifdef FFS_CLOSE_BEFORE_OPEN

  T_FFS_FD Dummy_FFSFileHandle = INVALID_FD;
  #define INVALID_FLD_CTR  0xFF
  #define NOT_OPENED       0x00
  #define OPENED_FOR_READ  0x01
  #define OPENED_FOR_WRITE 0x02

#endif

#ifdef DB_TEST

  S8 db_test_ffs_open_ctr = 0; 
  T_FFS_SIZE db_test_ffs_ret_code = EFFS_OK;

  #define DB_FFS_OPEN( ret, file, flag ) \
  { \
    ret = ffs_open( file, flag ); \
    \
    if( ret >= EFFS_OK ) \
    { \
      ++db_test_ffs_open_ctr; \
      TRACE_EVENT_P1( "DB_FFS_OPEN:db_test_ffs_open_ctr:%d", db_test_ffs_open_ctr ); \
    } else { \
      TRACE_EVENT_P1( "DB_FFS_OPEN:FFS ERROR:%d", ret ); \
    } \
  }


  #define DB_FFS_CLOSE( fd ) \
  { \
    T_FFS_SIZE db_test_ffs_ret_code = ffs_close( fd ); \
    \
    if( db_test_ffs_ret_code >= EFFS_OK ) \
    { \
      --db_test_ffs_open_ctr; \
      TRACE_EVENT_P1( "DB_FFS_CLOSE:db_test_ffs_open_ctr:%d", db_test_ffs_open_ctr ); \
    } else { \
      TRACE_EVENT_P1( "DB_FFS_CLOSE:FFS ERROR:%d", db_test_ffs_ret_code ); \
    } \
    \
    fd = INVALID_FD; \
  }

  #define DB_RETURN( ret ) \
  { \
    TRACE_EVENT_P1( "DB_RETURN:ret:%d", ret ); \
    return ret; \
  }
 
  #define DB_VALUE_RETURN( ret ) \
  { \
    TRACE_EVENT_P1( "DB_VALUE_RETURN:ret:%d", ret ); \
    return ret; \
  }
 
#else

  #define DB_FFS_OPEN( ret, file, flag ) { ret = ffs_open( file, flag ); }
  #define DB_FFS_CLOSE( fd ) { ffs_close( fd ); fd = INVALID_FD; }
  #define DB_RETURN( ret ) return ret;
  #define DB_VALUE_RETURN( ret ) return ret;

#endif

#ifdef DB_MEMORY_CHECK

  LOCAL UBYTE alloc_ctr = 0;

  LOCAL void db_mfree (void *ptr)
  {
    if (ptr NEQ NULL) 
    {
      --alloc_ctr;
      TRACE_EVENT_P1( "DB Memory Free:%d", alloc_ctr );
      ACI_MFREE( ptr );
    }
    else
    {
      TRACE_ERROR ("Attempted to free NULL");
    }
  }

  LOCAL void *db_malloc (ULONG num_of_bytes)
  {
    void *ptr;
    
    ++alloc_ctr;
    TRACE_EVENT_P1( "DB Memory Alloc:%d", alloc_ctr );
    ACI_MALLOC( ptr, num_of_bytes );
    return ptr;
  }

  #define DB_MALLOC( ptr, size )  { ptr = db_malloc ( size ); }
  #define DB_MFREE( ptr )         { db_mfree ( ptr ); }


#else

  #define DB_MALLOC( ptr, num_of_bytes ) ACI_MALLOC( ptr, num_of_bytes )
  #define DB_MFREE( ptr ) ACI_MFREE( ptr ) 

#endif


#ifdef _SIMULATION_
  #define DB_DIR "/dbm"
  #define DB_MASTER_FILE "/dbm/DD_master"
#else
  #define DB_DIR "/gsm/dbm" 
  #define DB_MASTER_FILE "/gsm/dbm/DD_master"
#endif

/* Implements Measure#32: Row 1179, 1184, 1191, 1196, 1199 & 1203 */
LOCAL const char * const db_dir=DB_DIR;
/* Implements Measure#32: Row 1181, 1188 & 1201 */
LOCAL  char * const db_master_file=DB_MASTER_FILE;

#define FILENAME_LEN 40

#define T_DB_MASTER_RECORD_SIZE 18        
/* DBM master record
   File: DB_DIR/DD_master
   Data: 
      1)	DBDirectory (16 bytes, based on MAX_LEN_DIRECTORY=16)
      2)	NumOfFiles (1 byte, based on MAX_NUM_FILES=255)
      3) Clean (1 byte)
 */


#define T_DB_FIELD_RECORD_SIZE 43         
/* Based on the following data written into FFS 
   File: DB_DIR/DD_<DBDirectory>
   Data: 
    1)	FieldID (2 bytes, given in Interface)
    2)	DBType (1 byte)
    3)	RecordSize (2 bytes, based on MAX_RECORD_SIZE=65kb)
    4)	NumOfRecords (1 byte, based on MAX_NUM_RECORDS=255)
    5) SortIndexes (MAX_NUM_OF_SORT_INDEXS bytes, 1 byte for each index)
    6)	RecordBitMap (32 bytes, as of now based on MAX_NUM_RECORDS=255)
    7) Clean (1 byte)
 */

#define RecordBitMap_OFFSET 10  /* depending on structure in DB_DIR/DD_<DBDirectory> */

#define SortIndexList_OFFSET 6  /* depending on structure in DB_DIR/DD_<DBDirectory> */

#define SEARCH_FAILED -1

/* For FFS strctures */
#define FFS_CLEAN       0x01
#define FFS_DIRTY       0x00

#define FFS_TRACKED 0x01
#define FFS_NOT_TRACKED 0x00

#define NOT !

#define INVALID_RECORD_NUM 0xFF

/*
    Internal function to initialize RAM structures with FFS
 */
LOCAL T_DB_CODE init_RAM_with_FFS ( void );

#define	DBM_State_check \
  if ( DBM_State EQ DBM_NOT_INITIALISED ) DB_RETURN( DB_NOT_INITIALISED );

LOCAL T_DB_CODE db_handle_check ( int db_handle );

LOCAL UBYTE field_id_search ( int db_handle, 
                              USHORT field_id );

LOCAL T_DB_CODE update_history_log ( int    db_handle,
                                     USHORT field_id,
                                     USHORT record_num );

LOCAL void db_set_bit_in_bitmap (UBYTE *bitmap, 
                                 USHORT position,
                                 BOOL value);

LOCAL UBYTE db_search_bit_in_bitmap (UBYTE* bitmap,
                                     USHORT max_record,
                                     BOOL value);

LOCAL T_DB_CODE populate_sorted_list_from_FFS ( 

#ifdef FFS_CLOSE_BEFORE_OPEN
                                                UBYTE db_ctr,
#endif
                                                char*   sort_file,
                                                UBYTE   num_of_elements,
                                                UBYTE** sort_list_ptr );

LOCAL T_DB_CODE write_sorted_list_to_FFS ( 
                                     
#ifdef FFS_CLOSE_BEFORE_OPEN
                                          UBYTE db_ctr,
#endif
                                          char*   sort_file,
                                          UBYTE   num_of_elements,
                                          UBYTE*  sort_list );

LOCAL int db_binary_search ( UBYTE*            sort_list,
                             UBYTE             num_of_elements,
                             SHORT*            order_num,
                             T_SEARCH_FUNC     search_function, 
                             ULONG             flags, 
                             const UBYTE*      search_tag,
                             int               db_handle,
                             USHORT            field_id );

LOCAL T_DB_CODE update_field_data_in_FFS ( T_FFS_FD*   filehandle,
                                           UBYTE       db_handle,
                                           UBYTE       fld_ctr,
                                           UBYTE*      field_data,
                                           USHORT      offset,
                                           USHORT      length );

LOCAL UBYTE cal_NextRecordNum ( const char* DBDirectory,
                                USHORT      FieldID,
                                USHORT      RecordSize );


LOCAL T_DB_CODE read_write_user_record_from_FFS ( const char      *user_field_file,

#ifdef FFS_CLOSE_BEFORE_OPEN
                                            UBYTE       db_ctr,
                                            UBYTE       fld_ctr,
#endif
                                                  T_FFS_FD        *filehandle,
                                            UBYTE       record_num,
                                            USHORT      record_size,
                                            USHORT      offset,
                                            USHORT      length,
                                                  UBYTE           *record_buffer,
                                                  T_FFS_OPEN_FLAGS open_option);

LOCAL T_DB_CODE check_ffs_ret_code (T_FFS_SIZE ffs_ret_code,
                                    T_FFS_FD  *filehandle,
                                    T_FFS_FD   ffs_fd,
                                    BOOL       check_invalid);


LOCAL T_DB_CODE delete_file_dir_from_FFS ( const char* filename );

LOCAL T_DB_CODE update_dbm_data_in_FFS ( const char* filename,
                                         UBYTE       db_ctr,
                                         UBYTE       db_data_max_size,
                                         UBYTE*      db_data,
                                         USHORT      offset,
                                         USHORT      length );

LOCAL T_DB_CODE remove_field_from_FFS ( UBYTE db_ctr,
                                        UBYTE fld_ctr );


#ifdef FFS_CLOSE_BEFORE_OPEN

LOCAL T_FFS_FD db_open_user_field_file ( UBYTE            db_ctr, 
                                         UBYTE            fld_ctr, 
                                         const char*      user_field_file,
                                         T_FFS_OPEN_FLAGS open_option );


LOCAL void db_close_user_field_files ( UBYTE db_ctr );

LOCAL BOOL db_open_full_for_read ( UBYTE db_ctr );

LOCAL BOOL db_open_full_for_write ( UBYTE db_ctr );

LOCAL void db_close_for_write ( UBYTE db_ctr );

LOCAL void db_close_for_read ( UBYTE db_ctr );

LOCAL UBYTE db_status_user_field_file ( UBYTE db_ctr,
                                        UBYTE fld_ctr );


#endif /* FFS_CLOSE_BEFORE_OPEN */

LOCAL T_DB_CODE delete_dir_forced ( const char* filename );

LOCAL void get_sort_lists_from_FFS ( UBYTE db_ctr,
                                     UBYTE fld_ctr );

LOCAL void new_in_sort_lists ( UBYTE db_ctr,
                               UBYTE fld_ctr,
                               UBYTE record_num );

LOCAL void update_in_sort_lists ( UBYTE db_ctr,
                                  UBYTE fld_ctr,
                                  UBYTE record_num );

LOCAL void delete_in_sort_lists ( UBYTE db_ctr,
                                  UBYTE fld_ctr,
                                  UBYTE record_num );

LOCAL void insertion_sort ( UBYTE*        sort_list,
                            UBYTE         num_of_elements,
                            T_COMP_FUNC   compare_function, 
                            ULONG         flags,
                            int           db_handle,
                            USHORT        field_id ); 

LOCAL void insert_element ( UBYTE*        sort_list,
                            UBYTE         num_of_elements,
                            UBYTE         bottom,
                            UBYTE         top,
                            UBYTE         record_num,
                            T_COMP_FUNC   compare_function, 
                            ULONG         flags,
                            int           db_handle,
                            USHORT        field_id );

LOCAL T_DB_CODE internal_db_sort ( int          db_handle, 
                                   USHORT       field_id, 
                                   UBYTE        sort_index, 
                                   T_COMP_FUNC  compare_function, 
                                   ULONG        flags );

LOCAL void db_close_for_read_write ( FileHandleRecord  *tmp_open_fields,
                                     UBYTE             *tmp_old,
                                     UBYTE              max_per_db);




/******************************
  I M P L E M E N T A T I O N 
 ******************************/

/**********************/
/* INTERNAL FUNCTIONS */
/**********************/

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_handle_check      |
+--------------------------------------------------------------------+

    PURPOSE : DB Handle checks
*/

LOCAL T_DB_CODE db_handle_check ( int db_handle )
{
  /* Check for db_handle range */
  if( db_handle >= MAX_DBs )
    return DB_INVALID_DB;
  
  /* Check for DBState of db_handle is CLOSED or UNUSED_ENTRY
     If yes, return DB_INVALID_DB  */

  if( ( DbmMaster[db_handle].DBState EQ CLOSED       ) OR
      ( DbmMaster[db_handle].DBState EQ UNUSED_ENTRY ) )
    return DB_INVALID_DB;
  
  return DB_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: field_id_search      |
+--------------------------------------------------------------------+

    PURPOSE : checks for the existence of field id in RAM
*/
LOCAL UBYTE field_id_search ( int     db_handle, 
                              USHORT  field_id )
{
  UBYTE  fld_ctr;
  
  for( fld_ctr = 0; fld_ctr < DbmMaster[db_handle].NumOfFiles; ++fld_ctr )
  {
    if( DbmMaster[db_handle].ptrFieldRecord[fld_ctr].FieldID EQ field_id )
    {
      return fld_ctr;
    }
  }
  return fld_ctr;
}


/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: update_history_log   |
+--------------------------------------------------------------------+

    PURPOSE : Updating history log
*/
LOCAL T_DB_CODE update_history_log ( int    db_handle,
                                     USHORT field_id,
                                     USHORT record_num )
{
  UBYTE i;

  /* Check if database is tracked */
  if (!DbmMaster[db_handle].Tracked) /* if db is NOT tracked */
    return DB_OK;

  /* Search history log if the record is present
     If present, return DB_OK */
  for( i = 0; i < DB_History_log.entries; ++i )
  {
    if( DB_History_log.field_id[i] EQ field_id )
      if( DB_History_log.record[i] EQ record_num )
        return DB_OK;
  }

  /* If the number of existing logs in history is reached limit, DB_MAX_AFFECTED, 
     return error "DB_HISTORY_FULL". */
  if( DB_History_log.entries EQ DB_MAX_AFFECTED )
    return DB_HISTORY_FULL;

  /* Add new entry */
  DB_History_log.field_id [DB_History_log.entries] = field_id;
  DB_History_log.record [DB_History_log.entries] = record_num;
  ++DB_History_log.entries;

  return DB_OK;

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_get_bit_in_bitmap |
+--------------------------------------------------------------------+

    PURPOSE : Gets a bit in record bitmap
*/
LOCAL BOOL db_get_bit_in_bitmap (const UBYTE *bitmap, USHORT position)
{
  USHORT offset;
  UBYTE mask;

  position--; /* We count from 1 */
  offset = position >> 3;
  mask = 0x80 >> (position & 0x7);
  return ((bitmap[offset] & mask) NEQ 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: set_bit_in_bitmap    |
+--------------------------------------------------------------------+

    PURPOSE : Sets a bit in record bitmap
*/
LOCAL void db_set_bit_in_bitmap (UBYTE *bitmap, USHORT position, BOOL value)
{
  USHORT offset;
  UBYTE mask;

  //TISH modified for MSIM
  if (position==0) return;
  
  position--; /* We count from 1 */
  offset = position >> 3;
  mask = 0x80 >> (position & 0x7);
  if (value)
    bitmap[offset] |= mask;
  else
    bitmap[offset] &= ~mask;
}

/*
+----------------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                          |
|                                       ROUINE: db_search_bit_in_bitmap      |
+----------------------------------------------------------------------------+

    PURPOSE : Searches for the bit not set in record bitmap which 
              indicates free record 
*/
LOCAL UBYTE db_search_bit_in_bitmap (UBYTE* bitmap, USHORT max_record, BOOL value)
{
  UBYTE i;

  for (i = 1; i <= max_record; i++)
  {
    if (db_get_bit_in_bitmap (bitmap, i) EQ value)
      return i;
  }
  return 0; /* Not found */
}

/*
+----------------------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                                |
|                                       ROUINE: populate_sorted_list_from_FFS      |
+----------------------------------------------------------------------------------+

    PURPOSE : Reads the sorted list from FFS
*/
LOCAL T_DB_CODE populate_sorted_list_from_FFS ( 

#ifdef FFS_CLOSE_BEFORE_OPEN
                                                UBYTE db_ctr,
#endif
                                                char*   sort_file,
                                                UBYTE   num_of_elements,
                                                UBYTE** sort_list_ptr )
{
  T_FFS_FD   ffs_fd_sort_file;
  T_FFS_SIZE ffs_ret_code;
  UBYTE*     sort_list;

#ifdef FFS_CLOSE_BEFORE_OPEN
  /* READ */
  if( db_open_full_for_read( db_ctr ) )
  {
     db_close_for_read( db_ctr );  
  }
#endif

  DB_FFS_OPEN( ffs_fd_sort_file, sort_file, FFS_O_RDONLY );

  if( (ffs_fd_sort_file EQ EFFS_NAMETOOLONG ) OR
      (ffs_fd_sort_file EQ EFFS_BADNAME     ) OR
      (ffs_fd_sort_file EQ EFFS_NOTFOUND    ) OR
      (ffs_fd_sort_file EQ EFFS_INVALID     ) OR
      (ffs_fd_sort_file EQ EFFS_LOCKED      ) )

  {
	  TRACE_EVENT_P1( "populate_sorted_list_from_FFS:DB_FFS_OPEN %d", ffs_fd_sort_file );
    return DB_FAIL;
  }

  if( ffs_fd_sort_file < EFFS_OK )
  {
    LastFFS_ReturnCode = ffs_fd_sort_file;
	  TRACE_EVENT_P1( "populate_sorted_list_from_FFS:DB_FFS_OPEN %d", ffs_fd_sort_file );
    return DB_FAIL_FS;
  }

  /* This seek is not required as file pointer will be at start of file,
     but ffs guide doesn't say so ! This needs to be clarified and 
     accordingly all ffs_seek at start of file after opening the file
     can be removed ! */
  ffs_ret_code = ffs_seek( ffs_fd_sort_file, 0, FFS_SEEK_SET );

  if( ( ffs_ret_code EQ EFFS_BADFD )   OR
      ( ffs_ret_code EQ EFFS_INVALID ) OR
      ( ffs_ret_code EQ EFFS_BADOP )   )
  {
    DB_FFS_CLOSE( ffs_fd_sort_file );
	  TRACE_EVENT_P1( "populate_sorted_list_from_FFS:ffs_seek %d", ffs_ret_code );
    return DB_FAIL;
  }

  DB_MALLOC( sort_list, num_of_elements + 1 );
  memset( sort_list, INVALID_RECORD_NUM, num_of_elements + 1 );

  ffs_ret_code = ffs_read( ffs_fd_sort_file, 
                           sort_list, 
                           num_of_elements );

  if( (ffs_ret_code EQ EFFS_BADFD ) OR
      (ffs_ret_code EQ EFFS_BADOP ) )
  {
    DB_MFREE( sort_list );
    DB_FFS_CLOSE( ffs_fd_sort_file );
	  TRACE_EVENT_P1( "populate_sorted_list_from_FFS:ffs_read %d", ffs_ret_code );
    return DB_FAIL;
  }

  if( ffs_ret_code < EFFS_OK )
  {
    LastFFS_ReturnCode = ffs_fd_sort_file;
    DB_MFREE( sort_list );
	  TRACE_EVENT_P1( "populate_sorted_list_from_FFS:ffs_read %d", ffs_ret_code );
    DB_FFS_CLOSE( ffs_fd_sort_file );
    return DB_FAIL_FS;
  }

  DB_FFS_CLOSE( ffs_fd_sort_file );

  /* to avoid memory leak */
  if( *sort_list_ptr NEQ NULL )
    DB_MFREE( *sort_list_ptr );

  *sort_list_ptr = sort_list;

  return DB_OK;
}

/*
+-----------------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                           |  
|                                       ROUINE: write_sorted_list_to_FFS      |
+-----------------------------------------------------------------------------+

    PURPOSE : Writes the sorted list into FFS
*/
LOCAL T_DB_CODE write_sorted_list_to_FFS ( 
                                     
#ifdef FFS_CLOSE_BEFORE_OPEN
                                           UBYTE db_ctr,
#endif
                                           char*   sort_file,
                                           UBYTE   num_of_elements,
                                           UBYTE*  sort_list )
{
  T_FFS_FD   ffs_fd_sort_file;
  T_FFS_SIZE ffs_ret_code;

#ifdef FFS_CLOSE_BEFORE_OPEN
  /* WRITE */
  if( db_open_full_for_write( db_ctr ) )
  {
     db_close_for_write( db_ctr );  
  }
#endif

  /* FFS_O_TRUNC => If file already exists and it is opened for writing, its 
                    length is truncated to zero. */
                    
  DB_FFS_OPEN( ffs_fd_sort_file, sort_file, FFS_O_CREATE | FFS_O_WRONLY | FFS_O_TRUNC );

  if( (ffs_fd_sort_file EQ EFFS_NAMETOOLONG ) OR
      (ffs_fd_sort_file EQ EFFS_BADNAME     ) )
  {
	  TRACE_EVENT_P1( "write_sorted_list_to_FFS:DB_FFS_OPEN %d", ffs_fd_sort_file );
    return DB_FAIL;
  }

  if( ffs_fd_sort_file < EFFS_OK )
  {
	  TRACE_EVENT_P1( "write_sorted_list_to_FFS:DB_FFS_OPEN %d", ffs_fd_sort_file );
    LastFFS_ReturnCode = ffs_fd_sort_file;
    return DB_FAIL_FS;
  }


  ffs_ret_code = ffs_seek( ffs_fd_sort_file, 0, FFS_SEEK_SET );

  if( ( ffs_ret_code EQ EFFS_BADFD )   OR
      ( ffs_ret_code EQ EFFS_INVALID ) OR
      ( ffs_ret_code EQ EFFS_BADOP )   )
  {
	  TRACE_EVENT_P1( "write_sorted_list_to_FFS:ffs_seek %d", ffs_ret_code );
    DB_FFS_CLOSE( ffs_fd_sort_file );
    return DB_FAIL;
  }

  ffs_ret_code = ffs_write( ffs_fd_sort_file, 
                            sort_list, 
                            num_of_elements );
   
  if( ( ffs_ret_code EQ EFFS_BADFD ) OR
      ( ffs_ret_code EQ EFFS_BADOP ) )
  {
    DB_FFS_CLOSE( ffs_fd_sort_file );
    ffs_ret_code = ffs_remove( sort_file );
	  TRACE_EVENT_P1( "write_sorted_list_to_FFS:ffs_write %d", ffs_ret_code );
    return DB_FAIL;
  }

  if( ffs_ret_code < EFFS_OK )
  {
    LastFFS_ReturnCode = ffs_ret_code;
    DB_FFS_CLOSE( ffs_fd_sort_file );
    ffs_ret_code = ffs_remove( sort_file );
	  TRACE_EVENT_P1( "write_sorted_list_to_FFS:ffs_write %d", ffs_ret_code );
    return DB_FAIL_FS;
  }

  DB_FFS_CLOSE( ffs_fd_sort_file ); /* yeah, we close it here ! */

  return DB_OK;

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_binary_search     |
+--------------------------------------------------------------------+

    PURPOSE : Searches for the record; uses binary search and compare
              function is passed as a parameter.
*/
LOCAL int db_binary_search ( UBYTE*            sort_list,
                             UBYTE             num_of_elements,
                             SHORT*            order_num,
                             T_SEARCH_FUNC     search_function, 
                             ULONG             flags, 
                             const UBYTE*      search_tag,
                             int               db_handle,
                             USHORT            field_id ) 
{
  UINT32 left, right;
  UINT32 mid;
  int cmp_res;
  BOOL matched;

  TRACE_FUNCTION ("db_binary_search()");

  left = *order_num;
  right = num_of_elements;

  if (left >= right)
    return SEARCH_FAILED;

  /* As often the 1st element is the matching one we try a shortcut. */
  cmp_res = search_function (flags, 
                             search_tag, 
                             db_handle, 
                             field_id, 
                             sort_list[left]);
  if (cmp_res EQ 0)
  {
    *order_num = left + 1;
    return *order_num;
  }
  else if (cmp_res > 0)
    return SEARCH_FAILED; /* We learned already no exact match possible */

  /* Get the first (leftmost) matching element in the sorted list. */
  matched = FALSE;
  do
  {
    mid = (left + right) >> 1;
    cmp_res = search_function (flags, 
                               search_tag, 
                               db_handle, 
                               field_id, 
                               sort_list[mid]);

    if (cmp_res EQ 0)
      matched = TRUE;

    if (cmp_res < 0)
      left = mid + 1;
    else
      right = mid;
  }
  while (left < right);

  if (matched)
  {
    *order_num = left + 1;
    return *order_num;
  }
  return SEARCH_FAILED;
}


/*
+-----------------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                           |
|                                       ROUINE: update_field_data_in_FFS      |
+-----------------------------------------------------------------------------+

    PURPOSE : Updates elementary file data in FFS

    PARAMS:   filehandle: Pointer to file handle, maybe INVALID_FD
              db_handle: Handle of the particular DB
              fld_ctr: Number of field (0..n)
              field_data: (const) Pointer to data to be written
              offset: offset within the data
              length: length of data to be written

*/
LOCAL T_DB_CODE update_field_data_in_FFS ( T_FFS_FD*   filehandle,
                                           UBYTE       db_handle,
                                           UBYTE       fld_ctr,
                                           UBYTE*      field_data,
                                           USHORT      offset,
                                           USHORT      length )
{
  T_FFS_FD   ffs_fd = *filehandle;
  T_FFS_SIZE ffs_ret_code;
  CHAR field_file[FILENAME_LEN];

  /* Updating field data in FFS in file "field_file" */
/* Implements Measure#32: */
  sprintf(field_file, format_sDDd_str, db_dir, db_handle);    

  if (!IS_FD_VALID(ffs_fd))
  {
    DB_FFS_OPEN( ffs_fd, field_file, FFS_O_WRONLY );

    if( ( ffs_fd EQ EFFS_NOTFOUND )    OR
        ( ffs_fd EQ EFFS_NAMETOOLONG ) OR
        ( ffs_fd EQ EFFS_BADNAME )     OR
        ( ffs_fd EQ EFFS_INVALID )     )
    {
	    TRACE_EVENT_P1( "update_field_data_in_FFS:DB_FFS_OPEN %d", ffs_fd );
      return DB_FAIL;
    }

    if( ffs_fd < EFFS_OK )
    {
      LastFFS_ReturnCode = ffs_fd;
	    TRACE_EVENT_P1( "update_field_data_in_FFS:DB_FFS_OPEN %d", ffs_fd );
      return DB_FAIL_FS;
    }

    *filehandle = ffs_fd;

  } /* filehandle invalid? */

  /* Seek the file to fld_ctr position to update the entry */
  ffs_ret_code = ffs_seek( ffs_fd, 
                           ( fld_ctr * T_DB_FIELD_RECORD_SIZE ) + offset, 
                           FFS_SEEK_SET );

  if( ( ffs_ret_code EQ EFFS_BADFD )   OR
      ( ffs_ret_code EQ EFFS_INVALID ) OR
      ( ffs_ret_code EQ EFFS_BADOP )   )
  {
    TRACE_EVENT_P1( "update_field_data_in_FFS:ffs_seek %d", ffs_ret_code );
    DB_FFS_CLOSE( ffs_fd );
    *filehandle = INVALID_FD;
    return DB_FAIL;
  }

  if( ffs_ret_code < EFFS_OK )
  {
    TRACE_EVENT_P1( "update_field_data_in_FFS:ffs_seek %d", ffs_ret_code );
    LastFFS_ReturnCode = ffs_ret_code;
    DB_FFS_CLOSE( ffs_fd );
    *filehandle = INVALID_FD;
    return DB_FAIL_FS;
  }

  /* clean is reset in memset, so no processing for it 
     (in case of non-tracked database, anyway we ignore it ! */ 

  ffs_ret_code = ffs_write( ffs_fd, 
                            field_data, 
                            length );

  if( (ffs_ret_code EQ EFFS_BADFD ) OR
      (ffs_ret_code EQ EFFS_BADOP ) )
  {
    TRACE_EVENT_P1( "update_field_data_in_FFS:ffs_write %d", ffs_ret_code );
    DB_FFS_CLOSE( ffs_fd );
    *filehandle = INVALID_FD;
    return DB_FAIL;
  }

  if( ffs_ret_code < EFFS_OK )
  {
    TRACE_EVENT_P1( "update_field_data_in_FFS:ffs_write %d", ffs_ret_code );
    DB_FFS_CLOSE ( ffs_fd );
    *filehandle = INVALID_FD;
    LastFFS_ReturnCode = ffs_ret_code;
    return DB_FAIL_FS;
  }

  /* DB_FFS_CLOSE( ffs_fd ); we will do it in db_flush */
  /* updation is over */

#ifdef FFS_OPEN_PROBLEM_PATCH
  DB_FFS_CLOSE( ffs_fd );
  *filehandle = INVALID_FD;
#endif

#ifndef FFS_CLOSE_BEFORE_OPEN
  /* this is db file, but we do not close it */
#endif

  return DB_OK;

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: cal_NextRecordNum    |
+--------------------------------------------------------------------+

    PURPOSE : Reads the status of the file and returns 
              the next available record (position)
*/
LOCAL UBYTE cal_NextRecordNum ( const char* db_directory,
                                USHORT      field_id,
                                USHORT      record_size )
{
  char user_field_file[FILENAME_LEN];

  T_FFS_SIZE ffs_ret_code;
  T_FFS_STAT ffs_file_stat;

/* Implements Measure#32: Row 1167 */
  sprintf( user_field_file, 
          format_sUDd_str, 
          db_directory, 
          field_id );

  ffs_ret_code = ffs_stat( user_field_file, &ffs_file_stat );

  if( ffs_ret_code EQ EFFS_OK )
  {
    return ( (ffs_file_stat.size / record_size) + 1);
  }

  if( ffs_ret_code EQ EFFS_NOTFOUND )
  {
    TRACE_EVENT( "cal_NextRecordNum:ffs_stat:FILE NOT FOUND" );
    return 1;
  }

  /* this is a problem with user_field_file, raise an error and 
     return 0 */

  TRACE_ERROR( "cal_NextRecordNum: PROBLEM with user_field_file" );
  TRACE_EVENT_P1( "cal_NextRecordNum:ffs_stat %d", ffs_ret_code );
  return 1;

}


/* Implements Measure # 211 */

/*
+-----------------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                           |
|                                       ROUINE: delete_file_dir_from_FFS      |
+-----------------------------------------------------------------------------+

    PURPOSE : Deletes a file from FFS
*/
LOCAL T_DB_CODE delete_file_dir_from_FFS ( const char* filename )
{
  T_FFS_SIZE ffs_ret_code;

  TRACE_EVENT_P1( "delete_file_dir_from_FFS %s", filename );

  ffs_ret_code = ffs_remove( filename );

  if( ffs_ret_code EQ EFFS_DIRNOTEMPTY )
  {
    TRACE_EVENT( "Inconsistency; going for delete_dir_forced" );
    return delete_dir_forced( filename ); 
  }

  if( ffs_ret_code EQ EFFS_NOTFOUND )
  {
    TRACE_EVENT( "delete_file_dir_from_FFS:ffs_remove:FILE NOT FOUND" );
    return DB_OK;
  }

  if( ( ffs_ret_code EQ EFFS_ACCESS   ) OR
      ( ffs_ret_code EQ EFFS_LOCKED   ) )
  {
    TRACE_EVENT_P1( "delete_file_dir_from_FFS:ffs_remove %d", ffs_ret_code );
    return DB_FAIL;
  }

  if( ffs_ret_code < EFFS_OK )
  {
    TRACE_EVENT_P1( "delete_file_dir_from_FFS:ffs_remove %d", ffs_ret_code );
    LastFFS_ReturnCode = ffs_ret_code;
    return DB_FAIL_FS;
  }

  return DB_OK;

}

/*
+---------------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                         |
|                                       ROUINE: update_dbm_data_in_FFS      |
+---------------------------------------------------------------------------+

    PURPOSE : Updates the database manager information of a 
              particular database
*/
LOCAL T_DB_CODE update_dbm_data_in_FFS ( const char* filename,
                                         UBYTE       db_ctr,
                                         UBYTE       db_data_max_size,
                                         UBYTE*      db_data,
                                         USHORT      offset,
                                         USHORT      length )
{
  T_FFS_FD   ffs_fd = INVALID_FD;
  T_FFS_SIZE ffs_ret_code;

  /* See if file already opened, 
     if not open it and update the file handle */

#ifdef FFS_CLOSE_BEFORE_OPEN
  /* WRITE */
  if( db_open_full_for_write( db_ctr ) )
  {
     db_close_for_write( db_ctr );  
  }
#endif

  DB_FFS_OPEN( ffs_fd, filename, FFS_O_WRONLY );

  if( ( ffs_fd EQ EFFS_NAMETOOLONG ) OR
      ( ffs_fd EQ EFFS_BADNAME )     OR
      ( ffs_fd EQ EFFS_NOTFOUND )    OR
      ( ffs_fd EQ EFFS_INVALID )     OR
      ( ffs_fd EQ EFFS_LOCKED )      )
  {
    TRACE_EVENT_P1( "update_dbm_data_in_FFS:DB_FFS_OPEN %d", ffs_fd );
     return DB_FAIL;
  }

  if( ffs_fd < EFFS_OK )
  {
    TRACE_EVENT_P1( "update_dbm_data_in_FFS:DB_FFS_OPEN %d", ffs_fd );
    LastFFS_ReturnCode = ffs_fd;
    return DB_FAIL_FS;
  }

  ffs_ret_code = ffs_seek( ffs_fd, db_ctr * db_data_max_size, FFS_SEEK_SET );

  if( ( ffs_ret_code EQ EFFS_BADFD )   OR
      ( ffs_ret_code EQ EFFS_INVALID ) OR
      ( ffs_ret_code EQ EFFS_BADOP )   )
  {
    TRACE_EVENT_P1( "update_dbm_data_in_FFS:ffs_seek %d", ffs_ret_code );
    DB_FFS_CLOSE( ffs_fd );
    return DB_FAIL;
  }

  if( ffs_ret_code < EFFS_OK )
  {
    TRACE_EVENT_P1( "update_dbm_data_in_FFS:ffs_seek %d", ffs_ret_code );
    LastFFS_ReturnCode = ffs_ret_code;
    DB_FFS_CLOSE( ffs_fd );
    return DB_FAIL_FS;
  }

  ffs_ret_code = ffs_write( ffs_fd, db_data, db_data_max_size );
   
  if( ( ffs_ret_code EQ EFFS_BADFD ) OR
      ( ffs_ret_code EQ EFFS_BADOP ) )
  {
    TRACE_EVENT_P1( "update_dbm_data_in_FFS:ffs_write %d", ffs_ret_code );
    DB_FFS_CLOSE( ffs_fd );
    return DB_FAIL;
  }

  if( ffs_ret_code < EFFS_OK )
  {
    TRACE_EVENT_P1( "update_dbm_data_in_FFS:ffs_write %d", ffs_ret_code );
    DB_FFS_CLOSE( ffs_fd );
    LastFFS_ReturnCode = ffs_ret_code;
    return DB_FAIL_FS;
  }

  DB_FFS_CLOSE( ffs_fd );  /* yeah, DB file, so ok to close it here ! */

  /* updation of dbm data done */

  return DB_OK;
}

/*
+--------------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                        |
|                                       ROUINE: remove_field_from_FFS      |
+--------------------------------------------------------------------------+

    PURPOSE : Removes the elementary file and associated Index 
              file from FFS
*/
LOCAL T_DB_CODE remove_field_from_FFS ( UBYTE db_ctr,
                                        UBYTE fld_ctr )
{
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;
  char               user_field_file[FILENAME_LEN],
                     sort_file[FILENAME_LEN];
  T_DB_CODE          db_ret_code;
  UBYTE              sort_index_ctr;

  /* Assumed that DBState is not IN_USE */

  tmp_ptrFieldRecord = DbmMaster[db_ctr].ptrFieldRecord + fld_ctr;
  
  if( tmp_ptrFieldRecord -> FieldID EQ INVALID_FIELD_ID ) /* if not valid field id */
    return DB_OK;

  /* Close of user record files not required as DBState is not IN_USE */

  /* Delete file "<DBDirectory>/UD_<field_id>" */

/* Implements Measure#32: Row 1171 */
  sprintf( user_field_file, 
           format_sUDd_str, 
           (char *)DbmMaster[db_ctr].DBDirectory, 
           tmp_ptrFieldRecord -> FieldID );

  db_ret_code = delete_file_dir_from_FFS ( user_field_file );

  if( db_ret_code NEQ DB_OK )
    return db_ret_code;

  /* For sorted lists */

  for( sort_index_ctr = 0; sort_index_ctr < MAX_NUM_OF_SORT_INDEXS; ++sort_index_ctr )
  {
   
    /* Delete the SortedLists from memory */
    if (tmp_ptrFieldRecord->SortedLists[sort_index_ctr] NEQ NULL)
    {
      DB_MFREE (tmp_ptrFieldRecord->SortedLists[sort_index_ctr]);
      tmp_ptrFieldRecord->SortedLists[sort_index_ctr] = NULL;
    }

    /* Delete the SortedLists from the FFS */
    if( tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] NEQ INVALID_SORT_INDEX )
    {
      /* Sort file, "<DBDirectory>/UD_<field_id>_sort_<sort_index>" */

/* Implements Measure#32: Row 1170 */
      sprintf( sort_file, 
               format_sUDd_sortd_str, 
               (char *)DbmMaster[db_ctr].DBDirectory, 
               tmp_ptrFieldRecord -> FieldID,
               tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] );

      db_ret_code = delete_file_dir_from_FFS ( sort_file );

      if( db_ret_code NEQ DB_OK )
        return db_ret_code;        
    }
  }

  return DB_OK;

}



#ifdef FFS_CLOSE_BEFORE_OPEN

LOCAL T_FFS_FD db_open_user_field_file ( UBYTE            db_ctr, 
                                         UBYTE            fld_ctr, 
                                         const char*      user_field_file,
                                         T_FFS_OPEN_FLAGS open_option )
{
  UBYTE i;
  FileHandleRecord* tmp_open_fields;
  UBYTE*            tmp_old;
  UBYTE             tmp_max;

  /* sanity checks not done (db_ctr >= MAX_DBs OR fld_ctr EQ INVALID_FLD_CTR) */

  /* note that we store field counters and not field ids */

  /* Search in write/read list */
  for( i = 0; i < MAX_OPEN_WRITE_PER_DB; ++i )
  {
    if( DbmMaster[db_ctr].WRITE_OPEN_FIELDS[i].fld_ctr EQ fld_ctr )
      return DbmMaster[db_ctr].WRITE_OPEN_FIELDS[i].filehandle;
  } /* for */

  /* Search in read only list */
  for( i = 0; i < MAX_OPEN_READ_PER_DB; ++i )
  {
    if( DbmMaster[db_ctr].READ_OPEN_FIELDS[i].fld_ctr EQ fld_ctr )
    {
      if( NOT ( open_option & FFS_O_WRONLY ) ) /* if not opened for writing */
      {
        return DbmMaster[db_ctr].READ_OPEN_FIELDS[i].filehandle;
      }

      /* we need to reopen it in write mode */
      DB_FFS_CLOSE( DbmMaster[db_ctr].READ_OPEN_FIELDS[i].filehandle );
      DbmMaster[db_ctr].READ_OPEN_FIELDS[i].filehandle = INVALID_FD;
      DbmMaster[db_ctr].READ_OPEN_FIELDS[i].fld_ctr = INVALID_FLD_CTR;
      break;
      
    }
  } /* for */

  /* field is not present in both lists */

  if( open_option & FFS_O_WRONLY )
  {

    /* for writing and reading both */
    tmp_open_fields = DbmMaster[db_ctr].WRITE_OPEN_FIELDS;
    tmp_old = &(DbmMaster[db_ctr].old_write);
    tmp_max = MAX_OPEN_WRITE_PER_DB;

  } else {

    /* for reading only */
    tmp_open_fields = DbmMaster[db_ctr].READ_OPEN_FIELDS;
    tmp_old = &(DbmMaster[db_ctr].old_read);
    tmp_max = MAX_OPEN_READ_PER_DB;
  }


  /* search the list */
  for( i = 0; i < tmp_max; ++i )
  {
    if( tmp_open_fields[i].fld_ctr EQ INVALID_FLD_CTR )
    {
      DB_FFS_OPEN( tmp_open_fields[i].filehandle,
                   user_field_file,
                   open_option );

      tmp_open_fields[i].fld_ctr = fld_ctr;

      return tmp_open_fields[i].filehandle;
    }
  } /* for */

  /* we need to close the some file for opening this */

  if( *tmp_old >= tmp_max )
    *tmp_old = 0;

  DB_FFS_CLOSE( tmp_open_fields[*tmp_old].filehandle );

  DB_FFS_OPEN( tmp_open_fields[*tmp_old].filehandle,
               user_field_file,
               open_option );

  if( tmp_open_fields[*tmp_old].filehandle >= EFFS_OK )
  {
    tmp_open_fields[*tmp_old].fld_ctr = fld_ctr;
    ++(*tmp_old);
    return tmp_open_fields[*tmp_old - 1].filehandle;
  } 
  
  tmp_open_fields[*tmp_old].fld_ctr = INVALID_FLD_CTR;

  return tmp_open_fields[*tmp_old].filehandle;

}

LOCAL void db_close_user_field_files ( UBYTE db_ctr )
{
  UBYTE i;
  FileHandleRecord* tmp_open_fields;

  /* sanity check not done */

  /* for write list */

  tmp_open_fields = DbmMaster[db_ctr].WRITE_OPEN_FIELDS;

  for( i = 0; i < MAX_OPEN_WRITE_PER_DB; ++i )
  {
    if( tmp_open_fields[i].fld_ctr NEQ INVALID_FLD_CTR )
    {
      DB_FFS_CLOSE( tmp_open_fields[i].filehandle );
      tmp_open_fields[i].filehandle = INVALID_FD;
      tmp_open_fields[i].fld_ctr = INVALID_FLD_CTR;
    }
  }

  /* for read list */

  tmp_open_fields = DbmMaster[db_ctr].READ_OPEN_FIELDS;

  for( i = 0; i < MAX_OPEN_READ_PER_DB; ++i )
  {
    if( tmp_open_fields[i].fld_ctr NEQ INVALID_FLD_CTR )
    {
      DB_FFS_CLOSE( tmp_open_fields[i].filehandle );
      tmp_open_fields[i].filehandle = INVALID_FD;
      tmp_open_fields[i].fld_ctr = INVALID_FLD_CTR;
    }
  }

}

LOCAL BOOL db_open_full_for_read ( UBYTE db_ctr )
{
  UBYTE i;
  FileHandleRecord* tmp_open_fields;

  /* sanity check not done */

  /* for read list */

  tmp_open_fields = DbmMaster[db_ctr].READ_OPEN_FIELDS;

  for( i = 0; i < MAX_OPEN_READ_PER_DB; ++i )
  {
    if( tmp_open_fields[i].fld_ctr EQ INVALID_FLD_CTR )
      return FALSE;
  }

  return TRUE;
}

LOCAL BOOL db_open_full_for_write ( UBYTE db_ctr )
{
  UBYTE i;
  FileHandleRecord* tmp_open_fields;

  /* sanity check not done */

  /* for write list */

  tmp_open_fields = DbmMaster[db_ctr].WRITE_OPEN_FIELDS;

  for( i = 0; i < MAX_OPEN_WRITE_PER_DB; ++i )
  {
    if( tmp_open_fields[i].fld_ctr EQ INVALID_FLD_CTR )
      return FALSE;
  }

  return TRUE;
}

LOCAL void db_close_for_read ( UBYTE db_ctr )
{
  /* Implements Measure # 83 */
  db_close_for_read_write (DbmMaster[db_ctr].READ_OPEN_FIELDS, 
                         &(DbmMaster[db_ctr].old_read), MAX_OPEN_READ_PER_DB);

  return;

}

LOCAL void db_close_for_write ( UBYTE db_ctr )
{
  /* Implements Measure # 83 */
  db_close_for_read_write (DbmMaster[db_ctr].WRITE_OPEN_FIELDS,
                         &(DbmMaster[db_ctr].old_write), MAX_OPEN_WRITE_PER_DB);

  return;

}


LOCAL UBYTE db_status_user_field_file ( UBYTE db_ctr,
                                        UBYTE fld_ctr )
{
  UBYTE i;
  FileHandleRecord* tmp_open_fields;

  /* sanity check not done */

  /* for write list */

  tmp_open_fields = DbmMaster[db_ctr].WRITE_OPEN_FIELDS;

  for( i = 0; i < MAX_OPEN_WRITE_PER_DB; ++i )
  {
    if( tmp_open_fields[i].fld_ctr EQ fld_ctr )
      return OPENED_FOR_WRITE;
  }

  /* for read list */

  tmp_open_fields = DbmMaster[db_ctr].READ_OPEN_FIELDS;

  for( i = 0; i < MAX_OPEN_READ_PER_DB; ++i )
  {
    if( tmp_open_fields[i].fld_ctr EQ fld_ctr )
      return OPENED_FOR_READ;
  }

  return NOT_OPENED;
}


#endif /* FFS_CLOSE_BEFORE_OPEN */

/*
+-----------------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                           |
|                                       ROUINE: delete_dir_forced             |
+-----------------------------------------------------------------------------+

    PURPOSE : Removes contents of dir and then delete the dir
              (it uses recursion)
*/
LOCAL T_DB_CODE delete_dir_forced ( const char* dir )
{
  T_FFS_SIZE ffs_ret_code;
  T_FFS_DIR  ffs_dir; /*lint !e813 big var on stack, not true for target */
  T_FFS_STAT tmp_ffs_stat;

  char  pathname[FILENAME_LEN];
  UBYTE dir_len;

  /* Open dir, if does not exist, just return DB_OK */

  ffs_ret_code = ffs_opendir (dir, &ffs_dir);

  if( ffs_ret_code EQ EFFS_NOTFOUND )
    DB_RETURN( DB_OK );

  if( ffs_ret_code < EFFS_OK )
  {
    TRACE_EVENT_P2( "delete_dir_forced:ffs_remove %d, %s", ffs_ret_code, dir );
    LastFFS_ReturnCode = ffs_ret_code;
    DB_RETURN( DB_FAIL_FS )
  }

  /* read dir recursively */
  sprintf( pathname, 
           "%s/", 
           dir );

  dir_len = strlen( pathname );

  while( 1 )
  {
    /* filename inside the directory would be copied at pathname + dirlen */

    ffs_ret_code = ffs_readdir( &ffs_dir,
                                pathname + dir_len,
                                FILENAME_LEN - dir_len - 1 );

    if( ffs_ret_code EQ 0 ) /* dir is empty now */
    {
      break;
    }

    if( ffs_ret_code < EFFS_OK )
    {
      TRACE_EVENT_P2( "delete_dir_forced:ffs_readdir:%d, %s", ffs_ret_code, dir );
      LastFFS_ReturnCode = ffs_ret_code;
      DB_RETURN( DB_FAIL_FS )
    }

    ffs_ret_code = ffs_stat( pathname, &tmp_ffs_stat );

    if( ffs_ret_code < EFFS_OK )
    {
      TRACE_EVENT_P2( "delete_dir_forced:ffs_stat:%d, %s", ffs_ret_code, pathname);
      LastFFS_ReturnCode = ffs_ret_code;
      DB_RETURN( DB_FAIL_FS )
    }

    if( tmp_ffs_stat.type EQ OT_DIR )
    {
      /* Directory !! */

      T_DB_CODE db_code;

      TRACE_EVENT_P1( "Warning: directory %s", pathname );

      /* Recursion */
      db_code = delete_dir_forced( pathname ); 

      if( db_code NEQ DB_OK )
        DB_RETURN( db_code );

    } else {

      /* normal file (OT_FILE) or symbolic link (OT_LINK) */

      ffs_ret_code = ffs_remove( pathname );

      if( ffs_ret_code < EFFS_OK )
      {
        TRACE_EVENT_P2( "delete_dir_forced:ffs_remove:%d, %s", ffs_ret_code, pathname );
        LastFFS_ReturnCode = ffs_ret_code;
        DB_RETURN( DB_FAIL_FS )
      }

    } /* if( tmp_ffs_stat.type EQ OT_DIR ) */

  } /* while( 1 ) */

  /* now delete the directory */
  ffs_ret_code = ffs_remove( dir );

  if( ffs_ret_code < EFFS_OK )
  {
    TRACE_EVENT_P2( "delete_dir_forced:ffs_remove:%d, %s", ffs_ret_code, dir );
    LastFFS_ReturnCode = ffs_ret_code;
    DB_RETURN( DB_FAIL_FS )
  }

  DB_RETURN( DB_OK )
}


LOCAL void get_sort_lists_from_FFS ( UBYTE db_ctr,
                                     UBYTE fld_ctr )
{
  UBYTE sort_index_ctr;

  T_DB_CODE db_ret_code;

  char sort_file[FILENAME_LEN];

  T_DBM_FIELDRECORD* tmp_ptrFieldRecord = DbmMaster[db_ctr].ptrFieldRecord + fld_ctr;

  /* for all existing sort indexes */
  for( sort_index_ctr = 0; sort_index_ctr < MAX_NUM_OF_SORT_INDEXS; ++sort_index_ctr )
  {
    /* valid index ? */
    if( tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] NEQ INVALID_SORT_INDEX )
    {
      /* Check if we already have sorted list in RAM structure SortedLists */

      if( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] EQ NULL )
      { 
        /* If no, populate the sorted list from file, 
           "<DBDirectory>/UD_<field_id>_sort_<sort_index>". 
           Sorted lists are freed in db_flush */

/* Implements Measure#32: Row 1177 */
        sprintf( sort_file, 
                 format_sUDd_sortd_str, 
                 (char *)DbmMaster[db_ctr].DBDirectory, 
                 tmp_ptrFieldRecord -> FieldID,
                 tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] );

        db_ret_code = 
          populate_sorted_list_from_FFS (  
#ifdef FFS_CLOSE_BEFORE_OPEN
                                          db_ctr,
#endif
                                          sort_file,
                                          tmp_ptrFieldRecord -> NumOfRecords,
                                          &(tmp_ptrFieldRecord -> SortedLists[sort_index_ctr]) );
        if( db_ret_code NEQ DB_OK )
        {
          /* Failure, still we can continue as later during actual sorting, 
             FFS would be retried */

          TRACE_EVENT( "Warning: populate_sorted_list_from_FFS failed" );
          TRACE_EVENT_P3( "db_ctr %d field_id %04X sort_index %d",
                          db_ctr,
                          tmp_ptrFieldRecord -> FieldID,
                          tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] );

          continue;
        }
    
      } /* if we have sort list */

    } /* if valid index */

  } /* for all sort indexes */

  return;

}

LOCAL void new_in_sort_lists ( UBYTE db_ctr,
                               UBYTE fld_ctr,
                               UBYTE record_num )
{
  UBYTE sort_index_ctr;

  T_DBM_FIELDRECORD* tmp_ptrFieldRecord = DbmMaster[db_ctr].ptrFieldRecord + fld_ctr;

  /* make a note that this record needs to be sorted */

  /* for all existing sort indexes */
  for( sort_index_ctr = 0; sort_index_ctr < MAX_NUM_OF_SORT_INDEXS; ++sort_index_ctr )
  {
    if( tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] NEQ INVALID_SORT_INDEX )
    {
      /* add it at the end like all sorted records + INVALID RECORD + .... */
      tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][tmp_ptrFieldRecord -> UsedRecords] 
        = record_num;
    }
  }

  return;

}

LOCAL void update_in_sort_lists ( UBYTE db_ctr,
                                  UBYTE fld_ctr,
                                  UBYTE record_num )
{
  UBYTE sort_index_ctr,
        rec_ctr;

  T_DBM_FIELDRECORD* tmp_ptrFieldRecord = DbmMaster[db_ctr].ptrFieldRecord + fld_ctr;

  /* make a note that this record needs to be sorted */

  /* for all existing sort indexes */
  for( sort_index_ctr = 0; sort_index_ctr < MAX_NUM_OF_SORT_INDEXS; ++sort_index_ctr )
  {
    if( tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] NEQ INVALID_SORT_INDEX )
    {
      /* search entry in sorted records */
      rec_ctr = 0;
      while( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][rec_ctr] NEQ INVALID_RECORD_NUM )
      {
        if( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][rec_ctr] EQ record_num )
        {
          /* found in sorted records, 
             now delete this entry */

          memmove( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] + rec_ctr,                    /* dest */
                   (const char*)(tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] + rec_ctr + 1), /* source */
                   tmp_ptrFieldRecord -> UsedRecords - rec_ctr );                                  /* size */   

          tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][tmp_ptrFieldRecord -> UsedRecords] 
            = record_num;

          break;
        }

        ++rec_ctr;
      }

    } /* NEQ INVALID_SORT_INDEX */

  } /* for sort_index_ctr */

  return;
}

LOCAL void delete_in_sort_lists ( UBYTE db_ctr,
                                  UBYTE fld_ctr,
                                  UBYTE record_num )
{
  UBYTE sort_index_ctr,
        rec_ctr;

  T_DBM_FIELDRECORD* tmp_ptrFieldRecord = DbmMaster[db_ctr].ptrFieldRecord + fld_ctr;

  /* make a note that this record needs to be sorted */

  /* for all existing sort indexes */
  for( sort_index_ctr = 0; sort_index_ctr < MAX_NUM_OF_SORT_INDEXS; ++sort_index_ctr )
  {
    if( tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] NEQ INVALID_SORT_INDEX )
    {
      /* search entry in sorted records */
      rec_ctr = 0;
      while( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][rec_ctr] NEQ INVALID_RECORD_NUM )
      {
        if( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][rec_ctr] EQ record_num )
          break;

        ++rec_ctr;
      }

      if( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][rec_ctr] NEQ INVALID_RECORD_NUM )
      {
        /* found in sorted records, 
           now delete this entry */

        memmove( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] + rec_ctr,
                 (const char*)(tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] + rec_ctr + 1),
                 tmp_ptrFieldRecord -> UsedRecords - rec_ctr );

        tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][tmp_ptrFieldRecord -> UsedRecords] 
          = INVALID_RECORD_NUM;

      } else {

        /* search in UN-sorted records */

        ++rec_ctr;
        while( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][rec_ctr] NEQ INVALID_RECORD_NUM )
        {
          if( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][rec_ctr] EQ record_num )
          {
            /* delete this entry */
            memmove( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] + rec_ctr,
                     (const char*)(tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] + rec_ctr + 1),
                     tmp_ptrFieldRecord -> UsedRecords - rec_ctr );

            tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][tmp_ptrFieldRecord -> UsedRecords] 
              = INVALID_RECORD_NUM;

            break;
          }
          ++rec_ctr;
        }

      }

    } /* NEQ INVALID_SORT_INDEX */

  } /* for sort_index_ctr */

  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: insertion_sort       |
+--------------------------------------------------------------------+

    PURPOSE : Sorts the user data records in FFS based on insertion sort method
*/
LOCAL void insertion_sort ( UBYTE*        sort_list,
                            UBYTE         num_of_elements,
                            T_COMP_FUNC   compare_function, 
                            ULONG         flags,
                            int           db_handle,
                            USHORT        field_id )
{
  UBYTE rec_ctr;

  /* sanity check */
  if( num_of_elements EQ 0 )
    return;

  if( sort_list[0] EQ INVALID_RECORD_NUM )
  {
    /* This is beginning of new sort list,
       Let's add first element */
    sort_list[0] = sort_list[1];
    sort_list[1] = INVALID_RECORD_NUM;
  }


  /* So now here we have list something like this
     { Sorted records } + INVALID_RECORD_NUM + { UNsorted records }
  */

  /* let's get to Unsorted records */
  rec_ctr = 1;
  while( sort_list[rec_ctr] NEQ INVALID_RECORD_NUM )
  {
    ++rec_ctr;
  }
  ++rec_ctr;

  /* rec_ctr is now pointing at UNsorted records */

  /* now add UNsorted records one by one */

  /* Not required "( sort_list[rec_ctr] NEQ INVALID_RECORD_NUM )", right ? */

  while( rec_ctr <= num_of_elements )
  {
    insert_element( sort_list,
                    rec_ctr,                /* num_of_elements */
                    0,                      /* bottom */
                    rec_ctr - 2,            /* top */
                    sort_list[rec_ctr],     /* record_num to be inserted */
                    compare_function,
                    flags,
                    db_handle,
                    field_id );

    ++rec_ctr;
  }

  /* done ! */
  return;
}

LOCAL void insert_element ( UBYTE*        sort_list,
                            UBYTE         num_of_elements,
                            UBYTE         bottom,
                            UBYTE         top,
                            UBYTE         record_num,
                            T_COMP_FUNC   compare_function, 
                            ULONG         flags,
                            int           db_handle,
                            USHORT        field_id )
{
  UBYTE middle = ( top + bottom ) / 2;

  /* use binary algorithm to find the right place for insertion */
  while( middle NEQ bottom )
  {
    if( compare_function( db_handle, 
                          field_id,
                          sort_list[middle],
                          record_num,
                          flags ) 
        < 0 )
    {
      bottom = middle;
    } else {
      top = middle;
    }
    middle = ( top + bottom ) / 2;

  } /* while( middle NEQ bottom ) */

  if( compare_function( db_handle, 
                        field_id,
                        sort_list[bottom],
                        record_num,
                        flags ) 
      < 0 )
  {

    if( compare_function( db_handle, 
                          field_id,
                          sort_list[top],
                          record_num,
                          flags ) 
        < 0 )
    {
      /* insert at top + 1 */
      memmove( sort_list + top + 2,            /* dest */
               sort_list + top + 1,            /* source */
               num_of_elements - (top + 1) );  /* size */

      sort_list[top + 1] = record_num;

    } else {

      /* insert at bottom + 1 */
      memmove( sort_list + bottom + 2,            /* dest */
               sort_list + bottom + 1,            /* source */
               num_of_elements - (bottom + 1) );  /* size */

      sort_list[bottom + 1] = record_num;

    } /* if top_th < insert_th */

  } else {

    /* insert at bottom */
    memmove( sort_list + bottom + 1,       /* dest */
             sort_list + bottom,           /* source */
             num_of_elements - bottom );   /* size */

    sort_list[bottom] = record_num;

  } /* if bottom_th < insert_th */

  return;
}


/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: internal_db_sort     |
+--------------------------------------------------------------------+

    PURPOSE : (Internal) Creates or updates a sort index for
              a given (database, field)
*/
LOCAL T_DB_CODE internal_db_sort ( int          db_handle, 
                                   USHORT       field_id, 
                                   UBYTE        sort_index, 
                                   T_COMP_FUNC  compare_function, 
                                   ULONG        flags )
{
  UBYTE  fld_ctr,
         sort_index_ctr,
         remb_sort_index_ctr,
         record_num;
  UBYTE* sort_list;
  UBYTE* copy_sort_list;
  T_DB_CODE  db_ret_code;
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;
  UBYTE* field_data;

  TRACE_FUNCTION("internal_db_sort()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  /* Check if this is existing sort index */
  remb_sort_index_ctr = MAX_NUM_OF_SORT_INDEXS;

  for( sort_index_ctr = 0; sort_index_ctr < MAX_NUM_OF_SORT_INDEXS; ++sort_index_ctr )
  {
    if( tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] EQ sort_index )
      break;

    if( tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] EQ INVALID_SORT_INDEX )
      remb_sort_index_ctr = sort_index_ctr;

  }

  if( sort_index_ctr EQ MAX_NUM_OF_SORT_INDEXS )
  { 
    /* ok, this is new index, so
       Check if limit for sort list corresponding to field_id (for a given database) 
       has reached limit, MAX_NUM_OF_SORT_INDEXS. If so, return DB_FULL. */

    if( remb_sort_index_ctr EQ MAX_NUM_OF_SORT_INDEXS )
      DB_RETURN( DB_FULL );
    
    sort_index_ctr = remb_sort_index_ctr;

    /* update file , "~/dbm/DD_<db_handle>" for SortIndexes. */

    /* create the field data that is to be written */

    DB_MALLOC( field_data, MAX_NUM_OF_SORT_INDEXS );
    memcpy( field_data, tmp_ptrFieldRecord -> SortIndexList, MAX_NUM_OF_SORT_INDEXS );

    field_data[sort_index_ctr] = sort_index;

    db_ret_code = 
      update_field_data_in_FFS ( &DbmMaster[db_handle].FFSFileHandle,
                                 db_handle,
                                 fld_ctr,
                                 field_data,
                                 SortIndexList_OFFSET,
                                 MAX_NUM_OF_SORT_INDEXS );
    if( db_ret_code NEQ DB_OK )
    {
      DB_MFREE( field_data );
      DB_RETURN( db_ret_code )
    }

    DB_MFREE( field_data );
    /* updation of DD_<db_handle> is over */
  }

  /* create sort list (if not there) */
  if( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] NEQ NULL )
  {
    sort_list = tmp_ptrFieldRecord -> SortedLists[sort_index_ctr];
  } 
  else
  {
    DB_MALLOC( sort_list, tmp_ptrFieldRecord -> NumOfRecords + 1 );
    memset( sort_list, INVALID_RECORD_NUM, tmp_ptrFieldRecord -> NumOfRecords + 1 );

      /* the first element would be INVALID_RECORD_NUM (0xFF) */
      copy_sort_list = sort_list + 1;

    /* The list of available records is prepared from RecordBitMap */
    for (record_num = 1; record_num <= tmp_ptrFieldRecord->NumOfRecords; record_num++)
    {
      if (db_get_bit_in_bitmap (tmp_ptrFieldRecord->RecordBitMap, record_num))
      {
        *copy_sort_list++ = record_num;
      }
    }

  } /* sort_list is NULL */

  /* Sort the records with given "comparison_function" (it takes record numbers, 
     field_id and db_handle). Note that the sorted list would be kept in RAM 
     (structure SortedLists, part of field record) till db_flush is called and 
     cleared in db_flush. */

    insertion_sort( sort_list,
                    tmp_ptrFieldRecord -> UsedRecords,   /* only used records */
                    compare_function,
                    flags,
                    db_handle,
                    field_id );

  /* writing sorted list to FFS would be done in db_flush */
 
  /* Update corresponding field SortIndexList in RAM structures. */

  tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] = sort_index;
  tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] = sort_list;

  DB_RETURN( DB_OK )
}



/***********************/
/* INTERFACE FUNCTIONS */
/***********************/


/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_init              |
+--------------------------------------------------------------------+

    PURPOSE : Initialises database manager
*/
void db_init ( void )
{
  UBYTE db_ctr;

#ifdef FFS_CLOSE_BEFORE_OPEN
  UBYTE fld_ctr;
#endif

  TRACE_FUNCTION("db_init()");

  /* 1) Check DBM_State and if already initialized, return DB_OK */
  if ( DBM_State EQ DBM_INITIALISED )
    return;

  /* 2)	Initialize DbmMaster structure */
  for ( db_ctr = 0; db_ctr < MAX_DBs; ++db_ctr )
  {
    DbmMaster[db_ctr].DBState = UNUSED_ENTRY;

#ifdef FFS_CLOSE_BEFORE_OPEN 

    for( fld_ctr = 0; fld_ctr < MAX_OPEN_READ_PER_DB; ++fld_ctr )
    {
      DbmMaster[db_ctr].READ_OPEN_FIELDS[fld_ctr].fld_ctr = INVALID_FLD_CTR;
    }
    for( fld_ctr = 0; fld_ctr < MAX_OPEN_WRITE_PER_DB; ++fld_ctr )
    {
      DbmMaster[db_ctr].WRITE_OPEN_FIELDS[fld_ctr].fld_ctr = INVALID_FLD_CTR;
    }
    DbmMaster[db_ctr].old_read = 0;
    DbmMaster[db_ctr].old_write = 0;

#endif

  }

  UsedDBs = 0;

#ifdef _SIMULATION_
  {
    T_FFS_SIZE ffs_ret_code = ffs_init();

    TRACE_EVENT_P1("Initialization of FFS Simulation %d", ffs_ret_code );

    /* Delete existing database */
    init_RAM_with_FFS();

    for( db_ctr = 0; db_ctr < MAX_DBs; ++db_ctr )
    {
      if( DbmMaster[db_ctr].DBState NEQ UNUSED_ENTRY )
        db_remove( DbmMaster[db_ctr].DBDirectory );
    }

    db_exit();
  }
#endif

  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: init_RAM_with_FFS    |
+--------------------------------------------------------------------+

    PURPOSE : Initialise DBM RAM structures with FFS data
*/
LOCAL T_DB_CODE init_RAM_with_FFS ( void )
{
  /* Initializing "DBM related data" in RAM from FFS */

  T_FFS_FD            ffs_dbm_fd;
  T_FFS_SIZE          ffs_ret_code;
  T_FFS_STAT          ffs_status;
  UBYTE*              db_buffer;
  UBYTE*              remb_db_buffer;
  UBYTE*              field_buffer;
  UBYTE*              remb_field_buffer;
  UBYTE               db_ctr,
                      fld_ctr,
                      i;
  char                filename[FILENAME_LEN];
/* Implements Measure#32: Row 1181 */
  char*               dbm_file = db_master_file;
  T_DBM_MASTERRECORD  *dbm;

  TRACE_FUNCTION("init_RAM_with_FFS()");

  /* Check the presence of FFS directory DB_DIR */
/* Implements Measure#32: Row 1184 */
  ffs_ret_code = ffs_stat( db_dir, &ffs_status );

  TRACE_EVENT_P1( "init_RAM_with_FFS:ffs_stat %d", ffs_ret_code );
  /* error values of ffs_ret_code are handled later */

  if( ffs_ret_code >= EFFS_OK )
  {
    /* Directory is present */
     
    /* Synchronize DB related data between FFS and RAM 
       i.e. structures DbmMasterRecord and corresponding DbmFieldRecord. */

    /* For DbmMaster */

    DB_FFS_OPEN( ffs_dbm_fd, dbm_file, FFS_O_RDONLY );

    if( (ffs_dbm_fd EQ EFFS_NAMETOOLONG ) OR
        (ffs_dbm_fd EQ EFFS_BADNAME     ) OR
        (ffs_dbm_fd EQ EFFS_NOTFOUND    ) OR
        (ffs_dbm_fd EQ EFFS_INVALID     ) OR
        (ffs_dbm_fd EQ EFFS_LOCKED      ) )

    {
      TRACE_EVENT_P1( "init_RAM_with_FFS:DB_FFS_OPEN %d", ffs_dbm_fd );
      return DB_FAIL;
    }

    if( ffs_dbm_fd < EFFS_OK )
    {
      TRACE_EVENT_P1( "init_RAM_with_FFS:DB_FFS_OPEN %d", ffs_dbm_fd );
      LastFFS_ReturnCode = ffs_dbm_fd;
      return DB_FAIL_FS;
    }

    /* buffer for master records */
    DB_MALLOC( db_buffer, T_DB_MASTER_RECORD_SIZE ); 
    remb_db_buffer = db_buffer;

    for( db_ctr = 0; db_ctr < MAX_DBs; ++db_ctr )
    {
      dbm = &DbmMaster[db_ctr];

      db_buffer = remb_db_buffer;
      ffs_ret_code = ffs_read( ffs_dbm_fd, db_buffer, T_DB_MASTER_RECORD_SIZE );

      if( ( ffs_ret_code EQ EFFS_BADFD ) OR
          ( ffs_ret_code EQ EFFS_BADOP ) )
      {
        TRACE_EVENT_P1( "init_RAM_with_FFS:ffs_read %d", ffs_ret_code );
        DB_FFS_CLOSE( ffs_dbm_fd );
        DB_MFREE( db_buffer );
        return DB_FAIL;
      }

      if( ffs_ret_code EQ 0 ) /* number of bytes read EQ 0 */
        break;

      if( *db_buffer EQ 0xFF ) /* if invalid database */
        continue;

      memcpy( dbm->DBDirectory, db_buffer, MAX_LEN_DIRECTORY );
      db_buffer += MAX_LEN_DIRECTORY;

      dbm->NumOfFiles = *db_buffer++; /* Maximum number of supported fields */

      dbm->Clean   = TRUE;
      dbm->Tracked = (*db_buffer++ EQ FFS_TRACKED);

      dbm->UsedFiles = 0;

      dbm->DBState = CLOSED;

      ++UsedDBs;

      /* we need not "seek" here file pointer would have moved to next */
    }
    
    DB_FFS_CLOSE( ffs_dbm_fd ); 
    DB_MFREE( remb_db_buffer ); /* freeing db_buffer */

    /* For FieldRecords */

    /* buffer for field records */
    DB_MALLOC( field_buffer, T_DB_FIELD_RECORD_SIZE ); 
    remb_field_buffer = field_buffer;

    for( db_ctr = 0; db_ctr < MAX_DBs; ++db_ctr )
    {
      dbm = &DbmMaster[db_ctr];

      if( dbm->DBState EQ UNUSED_ENTRY )
        continue;

      /* Field file = DD_<pos. in DD_master> */

/* Implements Measure#32: Row 1182, 1184 */
      sprintf( filename, format_sDDd_str, db_dir, db_ctr);  

      DB_FFS_OPEN( ffs_dbm_fd, filename, FFS_O_RDONLY );

      if(  ( ffs_dbm_fd EQ EFFS_NOTFOUND ) OR   /* Field file not found */
           ( ffs_dbm_fd EQ EFFS_NUMFD )    OR   /* FFS specific errors */
           ( ffs_dbm_fd EQ EFFS_LOCKED)    )
      {
        TRACE_EVENT_P1( "init_RAM_with_FFS:DB_FFS_OPEN %d", ffs_dbm_fd );
        DB_MFREE( field_buffer );
        return DB_FAIL;
      }

      /* Allocate memory for field records */
      DB_MALLOC( dbm->ptrFieldRecord, 
                 dbm->NumOfFiles * sizeof(T_DBM_FIELDRECORD) );

      for (fld_ctr = 0; fld_ctr < dbm->NumOfFiles; fld_ctr++)
      {
        field_buffer = remb_field_buffer;

        ffs_ret_code = ffs_read( ffs_dbm_fd, field_buffer, T_DB_FIELD_RECORD_SIZE );

        if( (ffs_ret_code EQ EFFS_BADFD ) OR
            (ffs_ret_code EQ EFFS_BADOP ) )
        {
          TRACE_EVENT_P2( "init_RAM_with_FFS:ffs_read %d, fld_ctr %d", ffs_ret_code, fld_ctr );
          DB_FFS_CLOSE ( ffs_dbm_fd );
          DB_MFREE( field_buffer );
          DB_MFREE( dbm->ptrFieldRecord );
          dbm->ptrFieldRecord = NULL;
          return DB_FAIL; 
        }

        if( ffs_ret_code < T_DB_FIELD_RECORD_SIZE )
        {
          /* we need not go further, just make rest of field ids as invalid (not existing) */
          for( ; fld_ctr < dbm->NumOfFiles; ++fld_ctr )
          {
            dbm->ptrFieldRecord[fld_ctr].FieldID = INVALID_FIELD_ID;
          }
          break;
        }

        /* Syncing of field records (FFS => RAM) */
        dbm->ptrFieldRecord[fld_ctr].FieldID = *field_buffer++;
        dbm->ptrFieldRecord[fld_ctr].FieldID <<= 8;
        dbm->ptrFieldRecord[fld_ctr].FieldID |= *field_buffer++;

        dbm->ptrFieldRecord[fld_ctr].DBType = (T_DB_TYPE)*field_buffer++;

        dbm->ptrFieldRecord[fld_ctr].RecordSize = *field_buffer++;
        dbm->ptrFieldRecord[fld_ctr].RecordSize <<= 8;
        dbm->ptrFieldRecord[fld_ctr].RecordSize |= *field_buffer++;

        dbm->ptrFieldRecord[fld_ctr].NumOfRecords = *field_buffer++;

        for (i = 0; i < MAX_NUM_OF_SORT_INDEXS; i++)
        {
          dbm->ptrFieldRecord[fld_ctr].SortIndexList[i] = *field_buffer++;
          dbm->ptrFieldRecord[fld_ctr].SortedLists[i] = NULL;
        }

        memcpy (dbm->ptrFieldRecord[fld_ctr].RecordBitMap, field_buffer, RECORD_BITMAP_SIZE);
          field_buffer += RECORD_BITMAP_SIZE;

        dbm->ptrFieldRecord[fld_ctr].Clean = *field_buffer++;

        if ( dbm->ptrFieldRecord[fld_ctr].FieldID NEQ INVALID_FIELD_ID )
        {
          /* 
           * Trace out whether we consider the respective field as not clean.
           */
          if (!dbm->ptrFieldRecord[fld_ctr].Clean)
          {
            TRACE_EVENT_P1 ("Field %04X not clean in master file",
                            dbm->ptrFieldRecord[fld_ctr].FieldID);
          }

          /* 
           * Consistency check.
           * Check whether the respective user field exists.
           * If it does not exist set it to invalid.
           * We are not taking care about possibly left over sort indexes.
           * This inconsistency can happen when we got a hard power cycle 
           * during some database updating operation.
           */
          sprintf (filename, "%s/UD_%04X", (char *)dbm->DBDirectory, 
                   dbm->ptrFieldRecord[fld_ctr].FieldID);
          ffs_ret_code = ffs_stat (filename, &ffs_status);
          if (ffs_ret_code NEQ EFFS_OK)
          {
            TRACE_EVENT_P1 ("User data %s not existing in FFS", filename);
            dbm->ptrFieldRecord[fld_ctr].FieldID = INVALID_FIELD_ID;
            dbm->Clean = FALSE; /* Mark DB as not clean */
          }

          /* 
           * Consistency check.
           * Check that the respective user data does not exist twice.
           * Normally it is expected that this inconsistency should not happen,
           * if it happens we ignore all subsequent entries.
           */
          if ( dbm->ptrFieldRecord[fld_ctr].FieldID NEQ INVALID_FIELD_ID )
          {
            for (i = 0; i < fld_ctr; i++)
            {
              if (dbm->ptrFieldRecord[fld_ctr].FieldID EQ 
                  dbm->ptrFieldRecord[i].FieldID)
              {
                TRACE_EVENT_P1 ("User data %04x existed twice", 
                                dbm->ptrFieldRecord[fld_ctr].FieldID);
                dbm->ptrFieldRecord[fld_ctr].FieldID = INVALID_FIELD_ID;
                dbm->ptrFieldRecord[i].Clean = FALSE; /* Don't trust this field */
                dbm->Clean = FALSE; /* Mark DB as not clean */
                break;
              }
            }
          }

          /*
           * Consistency check. 
           * Check whether the sorted index really exists in the FFS.
           * If it does not exist set it to invalid.
           */
          if ( dbm->ptrFieldRecord[fld_ctr].FieldID NEQ INVALID_FIELD_ID )
          {
            for( i = 0; i < MAX_NUM_OF_SORT_INDEXS; i++ )
            {
              if (dbm->ptrFieldRecord[fld_ctr].SortIndexList[i] NEQ INVALID_SORT_INDEX)
              {
                sprintf(filename, "%s/UD_%04X_sort_%d", 
                         (char *)dbm->DBDirectory, 
                         dbm->ptrFieldRecord[fld_ctr].FieldID,
                         dbm->ptrFieldRecord[fld_ctr].SortIndexList[i]);
                ffs_ret_code = ffs_stat (filename, &ffs_status);
                if (ffs_ret_code NEQ EFFS_OK)
                {
                  TRACE_EVENT_P1 ("Sort index %s not existing in FFS", filename);
                  dbm->ptrFieldRecord[fld_ctr].SortIndexList[i] = INVALID_SORT_INDEX;
                  dbm->ptrFieldRecord[fld_ctr].Clean = FALSE; /* Don't trust this field */
                }
              }
            }
          }

#ifndef FFS_CLOSE_BEFORE_OPEN
          tmp_ptrFieldRecord -> FFSFileHandle = INVALID_FD; 
#endif
          /* we need to get size of user file <DBDirectory>/UD_<field_id> 
             NextRecordNum = (size of user file) / record size */
          dbm->ptrFieldRecord[fld_ctr].NextRecordNum = 
                  cal_NextRecordNum ( (const char*)DbmMaster[db_ctr].DBDirectory, 
                                      dbm->ptrFieldRecord[fld_ctr].FieldID,
                                      dbm->ptrFieldRecord[fld_ctr].RecordSize );
                                            
          /* Update database records for UsedFiles, Clean, and UsedRecords */

          dbm->UsedFiles++;

          /* Mark DB as not clean if only one field is not clean */
          if (!dbm->ptrFieldRecord[fld_ctr].Clean)
            dbm->Clean = FALSE;

          /* For calculating UsedRecords */
          dbm->ptrFieldRecord[fld_ctr].UsedRecords = 0;
          for (i = 1; i <= dbm->ptrFieldRecord[fld_ctr].NumOfRecords; i++)
          {
            if (db_get_bit_in_bitmap (dbm->ptrFieldRecord[fld_ctr].RecordBitMap, i))
              dbm->ptrFieldRecord[fld_ctr].UsedRecords++;
          }

        } /* FieldID NEQ INVALID_FIELD_ID */

      } /* for( fld_ctr ) ... all fields */

      /* close the DD_<db_handle> file */
      DB_FFS_CLOSE( ffs_dbm_fd );

    } /* ffs_ret_code >= EFFS_OK */

    DB_MFREE( remb_field_buffer );  /* freeing field buffer */

    DBM_State = DBM_INITIALISED;

    return DB_OK;

  } /* if( ffs_ret_code >= EFFS_OK ) */

  /* Directory not present */
  if( ffs_ret_code EQ EFFS_NOTFOUND )
  {
    /* Create DBM directory */
/* Implements Measure#32: Row 1184 */
    ffs_ret_code = ffs_mkdir( db_dir ); 
    
    if( (ffs_ret_code EQ EFFS_NAMETOOLONG ) OR
        (ffs_ret_code EQ EFFS_BADNAME     ) )
    {
      TRACE_EVENT_P1( "init_RAM_with_FFS:ffs_mkdir %d", ffs_ret_code );
      return DB_FAIL;
    }

    if( ffs_ret_code < EFFS_OK )
    {
      TRACE_EVENT_P1( "init_RAM_with_FFS:ffs_mkdir %d", ffs_ret_code );
      LastFFS_ReturnCode = ffs_ret_code;
      return DB_FAIL_FS;
    }

    /* Create empty file DD_master
       FFS_O_EXCL => Generate error if FFS_O_CREATE is also specified and 
                     the file already exists. */

    DB_FFS_OPEN( ffs_dbm_fd, dbm_file, FFS_O_CREATE | FFS_O_RDWR | FFS_O_EXCL );

    if( (ffs_dbm_fd EQ EFFS_NAMETOOLONG ) OR
        (ffs_dbm_fd EQ EFFS_BADNAME     ) )
    {
      TRACE_EVENT_P1( "init_RAM_with_FFS:DB_FFS_OPEN %d", ffs_dbm_fd );
/* Implements Measure#32: Row 1184 */
      ffs_ret_code = ffs_remove( db_dir ); /* undo creating of DBM directory */
      return DB_FAIL;
    }

    if( ffs_dbm_fd < EFFS_OK )
    {
      TRACE_EVENT_P1( "init_RAM_with_FFS:DB_FFS_OPEN %d", ffs_dbm_fd );
      LastFFS_ReturnCode = ffs_dbm_fd;
/* Implements Measure#32: Row 1184 */
      ffs_ret_code = ffs_remove( db_dir ); /* undo creating of DBM directory */
      return DB_FAIL_FS;
    }

    DB_FFS_CLOSE( ffs_dbm_fd );

    DBM_State = DBM_INITIALISED;

    return DB_OK;
  }

  /* Unexpected error: Not a directory */
  return DB_FAIL;

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_create            |
+--------------------------------------------------------------------+

    PURPOSE : Create a new database in the given directory of the FFS. 
              The directory must not to exist yet, it will be created.
*/
int db_create ( const char* directory,
                UBYTE       num_of_fields,
                BOOL        tracked )
{
  int         db_handle;
  T_DB_CODE   db_ret_code;
  UBYTE       fld_ctr;
  S8          db_ctr;
  T_FFS_SIZE  ffs_ret_code;
  T_FFS_FD    ffs_fd;
  char        field_file[FILENAME_LEN];
  char*       field_file_ptr = field_file;
  UBYTE*      dbm_data;
  UBYTE*      remb_dbm_data;

  TRACE_FUNCTION("db_create()");

  /* If DBM is not in initialized state, call init_RAM_with_FFS (). */
  if( DBM_State NEQ DBM_INITIALISED )
  {
    T_DB_CODE db_ret_code;

    db_ret_code = init_RAM_with_FFS();

    if( db_ret_code NEQ DB_OK )
      DB_RETURN( db_ret_code );
  }

  /* See if you have reached the maximum number of DBs, ? 
     (UsedDBs+1) > MAX_DBs */

  if( UsedDBs EQ MAX_DBs )
    DB_RETURN( DB_FULL );


  /* Check DbmMaster if already database for "directory" exists. 
     If yes, return error "DB_EXISTS" */

  db_handle = -1;
  for( db_ctr = (MAX_DBs - 1); db_ctr >= 0; --db_ctr )
  {
    if ( DbmMaster[db_ctr].DBState NEQ UNUSED_ENTRY )
    {
        if( memcmp( DbmMaster[db_ctr].DBDirectory, 
                    directory, 
                    strlen( directory )          ) EQ 0 )
        DB_RETURN( DB_EXISTS )
    }
    else
    {
      db_handle = db_ctr;
    }
  }

  /* sanity check */
  if( num_of_fields <= 0 ) /*lint !e775 check for <= on non-negative*/
    DB_RETURN( DB_FAIL );

  if (db_handle EQ -1) 
  {
    /* No free slot found */
    DB_RETURN ( DB_FAIL ) /* DB_FULL maybe better choice */
  }

  /* Create a directory, "<DBDirectory>" (DBDirectory = directory) */
  ffs_ret_code = ffs_mkdir( directory ); 

  if( ffs_ret_code < EFFS_OK ) 
  {
    TRACE_EVENT_P1( "db_create:ffs_mkdir %d", ffs_ret_code );
    LastFFS_ReturnCode = ffs_ret_code;
    DB_RETURN( DB_FAIL_FS )
  }


  /* Create a empty file, "~/dbm/DD_<position in DD_master = db_handle>"; 
     if this FFS operation fails, we need to delete the directory 
     before returning error. 
  */
/* Implements Measure#32: Row 1182, 1191 */
  sprintf( field_file, format_sDDd_str, db_dir, db_handle);  

  TRACE_EVENT_P1( "field_file %s", field_file );

  /* FFS_O_EXCL => Generate error if FFS_O_CREATE is also specified and 
                   the file already exists. */
  DB_FFS_OPEN( ffs_fd, field_file_ptr, FFS_O_CREATE | FFS_O_RDWR | FFS_O_EXCL ); 

  if( ( ffs_fd EQ EFFS_EXISTS )      OR
      ( ffs_fd EQ EFFS_NAMETOOLONG ) OR
      ( ffs_fd EQ EFFS_BADNAME )     OR
      ( ffs_fd EQ EFFS_INVALID )     )
  {
    TRACE_EVENT_P1( "db_create:DB_FFS_OPEN %d", ffs_fd );
    ffs_fd = ffs_remove( directory ); 
    DB_RETURN( DB_FAIL )
  }

  if( ffs_fd < EFFS_OK )
  {
    TRACE_EVENT_P1( "db_create:DB_FFS_OPEN %d", ffs_fd );
    LastFFS_ReturnCode = ffs_fd;
    ffs_ret_code = ffs_remove( directory ); 
    DB_RETURN( DB_FAIL_FS )
  }

  /* DB_FFS_CLOSE( ffs_fd ); we will close it in db_flush */

#ifdef FFS_OPEN_PROBLEM_PATCH
  DB_FFS_CLOSE( ffs_fd );
  ffs_fd = INVALID_FD;
#endif

#ifdef FFS_CLOSE_BEFORE_OPEN
  /* no we do not close it */
#endif

  /* Update FFS data i.e. file, "~/dbm/DD_master". */

  /* prepare the dbm data and write */
  DB_MALLOC( dbm_data, T_DB_MASTER_RECORD_SIZE );
  memset( dbm_data, 0, T_DB_MASTER_RECORD_SIZE );
  remb_dbm_data = dbm_data;

  memcpy( dbm_data, directory, MAX_LEN_DIRECTORY );
  dbm_data += MAX_LEN_DIRECTORY;
  
  *dbm_data++ = num_of_fields;

  *dbm_data++ = ( tracked ) ? FFS_TRACKED : FFS_NOT_TRACKED;

  dbm_data = remb_dbm_data;

/* Implements Measure#32: Row 1188 */
  db_ret_code = 
    update_dbm_data_in_FFS ( db_master_file,
                             (UBYTE) db_handle,   /* to supress warning */
                             T_DB_MASTER_RECORD_SIZE,
                             dbm_data,
                             0,
                             T_DB_MASTER_RECORD_SIZE );

  if( db_ret_code NEQ DB_OK )
  {
    DB_MFREE( dbm_data );
    ffs_ret_code = ffs_remove( directory );
    DB_FFS_CLOSE( ffs_fd );
    ffs_ret_code = ffs_remove( field_file );
    DB_RETURN( db_ret_code )
  }

  DB_MFREE( dbm_data );
  /* updation of DD_master done 
     now update RAM structures  */

  memcpy( DbmMaster[db_handle].DBDirectory, 
          directory, 
          strlen(directory) );

  DbmMaster[db_handle].NumOfFiles    = num_of_fields;
  DbmMaster[db_handle].UsedFiles     = 0;  
  DbmMaster[db_handle].Clean = TRUE;
  DbmMaster[db_handle].Tracked = tracked;
  DbmMaster[db_handle].DBState       = OPEN;
  DbmMaster[db_handle].FFSFileHandle = ffs_fd;

#ifdef FFS_CLOSE_BEFORE_OPEN

  /* No files corresponding to fields have been opened 
     This initialization is already done in db_init */

#endif
  
  /* Allocate memory for storing field information 
     and initialize the memory with "FF" */

  DB_MALLOC( DbmMaster[db_handle].ptrFieldRecord, 
              num_of_fields * sizeof(T_DBM_FIELDRECORD) );

  for( fld_ctr = 0; fld_ctr < num_of_fields; ++fld_ctr )
  {
    DbmMaster[db_handle].ptrFieldRecord[fld_ctr].FieldID = INVALID_FIELD_ID; 
  }

  UsedDBs++;

  DB_RETURN( db_handle )

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_open              |
+--------------------------------------------------------------------+

    PURPOSE : Open an existing database
*/
int db_open ( const char *directory )
{
  T_DB_CODE  db_ret_code;
  UBYTE      db_ctr;

  TRACE_FUNCTION("db_open()");

  /* If DBM is not in initialized state, call init_RAM_with_FFS (). */
  if( DBM_State NEQ DBM_INITIALISED )
  {
    db_ret_code = init_RAM_with_FFS();

    if( db_ret_code NEQ DB_OK )
      DB_RETURN( db_ret_code );
  }

  /* Search for "directory" in DbmMaster */
  for( db_ctr = 0; db_ctr < MAX_DBs; ++db_ctr )
  {
    if( DbmMaster[db_ctr].DBState NEQ UNUSED_ENTRY )
    {
        if( memcmp( DbmMaster[db_ctr].DBDirectory, 
                    directory, 
                    strlen(directory)                     ) EQ 0 )
          break;
    } 
  }

  /* If "directory" not found, return DB_FAIL. */
  if( db_ctr EQ MAX_DBs )
    DB_RETURN( DB_INVALID_DB );

  /* Update DBState as "OPEN". */
  DbmMaster[db_ctr].DBState = OPEN;

  /* Return corresponding DBHandle. */
  DB_RETURN( db_ctr )
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_create_field      |
+--------------------------------------------------------------------+

    PURPOSE : Create a new elementary file of a given type with a given 
              numeric identifier and a given record size.
*/
T_DB_CODE db_create_field ( int         db_handle, 
                            T_DB_TYPE   db_type, 
                            USHORT      field_id, 
                            USHORT      record_size, 
                            USHORT      num_of_records )
{
  T_DB_CODE          db_ret_code;
  char               user_field_file[FILENAME_LEN];
  UBYTE              fld_ctr,
                     sort_index_ctr;
  T_FFS_FD           ffs_fd_user_field_file;
  UBYTE*             field_data;
  UBYTE*             remb_field_data;
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;  


  TRACE_FUNCTION("db_create_field()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field id found, return error DB_FIELD_EXISTS */
  if( field_id_search( db_handle, field_id ) NEQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_FIELD_EXISTS );

  /* If limit for number of fields for this DB has been reached, return error DB_FULL. */
  if( DbmMaster[db_handle].UsedFiles  EQ 
      DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_FULL );

  /* Search for free field entry in field records corresponding to the database. */
  for( fld_ctr = 0; fld_ctr < DbmMaster[db_handle].NumOfFiles; ++fld_ctr )
  {
    if( DbmMaster[db_handle].ptrFieldRecord[fld_ctr].FieldID EQ INVALID_FIELD_ID )
      break;
  }

  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
  { /* this is "inconsistency"; should never happen */
    DB_RETURN( DB_FAIL )
  }

  /* sanity check */
  /*lint -e{775} check for <= on non-negative */
  if ((num_of_records <= 0) OR
      (record_size <= 0))
  {
    DB_RETURN( DB_FAIL )
  }

  /* Create file "<DBDirectory>/UD_<field_id>" */
/* Implements Measure#32: Row 1189 */
  sprintf( user_field_file, format_sUDd_str, (char *)DbmMaster[db_handle].DBDirectory, field_id );

  /* FFS_O_EXCL => Generate error if FFS_O_CREATE is also specified and 
                   the file already exists. */
#ifndef FFS_CLOSE_BEFORE_OPEN

  DB_FFS_OPEN( ffs_fd_user_field_file, user_field_file, FFS_O_CREATE | FFS_O_RDWR | FFS_O_EXCL ); 

#else

  ffs_fd_user_field_file = 
    db_open_user_field_file( db_handle, 
                             fld_ctr, 
                             user_field_file, 
                             FFS_O_CREATE | FFS_O_RDWR | FFS_O_EXCL );
#endif

  if( ( ffs_fd_user_field_file EQ EFFS_EXISTS )      OR
      ( ffs_fd_user_field_file EQ EFFS_NAMETOOLONG ) OR
      ( ffs_fd_user_field_file EQ EFFS_BADNAME )     OR
      ( ffs_fd_user_field_file EQ EFFS_INVALID )     )
  {
    TRACE_EVENT_P1( "db_create_field:DB_FFS_OPEN %d", ffs_fd_user_field_file );
    DB_RETURN( DB_FAIL )
  }

  if( ffs_fd_user_field_file < EFFS_OK )
  {
    TRACE_EVENT_P1( "db_create_field:DB_FFS_OPEN %d", ffs_fd_user_field_file );
    LastFFS_ReturnCode = ffs_fd_user_field_file;
    DB_RETURN( DB_FAIL_FS )
  }

  /* We will close this file in db_flush */

#ifdef FFS_OPEN_PROBLEM_PATCH
  DB_FFS_CLOSE( ffs_fd_user_field_file );
  ffs_fd_user_field_file = INVALID_FD;
#endif

  
  /* file "<DBDirectory>/UD_<field_id>" has been created 
     Now, Add new field entry in "~/dbm/DD_<db_handle>" */

  /* create the field data that needs to be written */

  DB_MALLOC( field_data, T_DB_FIELD_RECORD_SIZE );
  memset( field_data, 0, T_DB_FIELD_RECORD_SIZE );
  remb_field_data = field_data;

  *field_data = (UBYTE) ( (field_id & 0xFF00) >> 8 ); 
  ++field_data;
  *field_data = (UBYTE) (field_id & 0x00FF);
  ++field_data;

  *field_data = db_type; 
  ++field_data;

  *field_data = (UBYTE) ( (record_size & 0xFF00) >> 8 );
  ++field_data;
  *field_data = (UBYTE) (record_size & 0x00FF);
  ++field_data;

  *field_data = (UBYTE)num_of_records;
  field_data++;
  
  memset( field_data, 0xFF, MAX_NUM_OF_SORT_INDEXS ); 
  field_data += MAX_NUM_OF_SORT_INDEXS;

  memset( field_data, 0, RECORD_BITMAP_SIZE );
  field_data += RECORD_BITMAP_SIZE;

  *field_data = FFS_CLEAN;
  
  field_data = remb_field_data;

  db_ret_code = 
    update_field_data_in_FFS ( &DbmMaster[db_handle].FFSFileHandle,
                               db_handle,
                               fld_ctr,
                               field_data,
                               0,
                               T_DB_FIELD_RECORD_SIZE );

  if( db_ret_code NEQ DB_OK )
  {
    DB_MFREE( field_data );
    ffs_remove( user_field_file );
    DB_RETURN( db_ret_code )
  }

  DB_MFREE( field_data );
  /* updation of DD_<db_handle> is over */

  /* Add new field entry in field records in RAM. */
  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  tmp_ptrFieldRecord -> FieldID       = field_id;
  tmp_ptrFieldRecord -> DBType        = db_type;
  tmp_ptrFieldRecord -> RecordSize    = record_size;
  tmp_ptrFieldRecord -> NumOfRecords  = (UBYTE)num_of_records;
  tmp_ptrFieldRecord -> UsedRecords   = 0; 
  tmp_ptrFieldRecord -> Clean         = TRUE;

#ifndef FFS_CLOSE_BEFORE_OPEN
  tmp_ptrFieldRecord -> FFSFileHandle = ffs_fd_user_field_file;
#endif

  tmp_ptrFieldRecord -> NextRecordNum = 1;

  memset( tmp_ptrFieldRecord -> RecordBitMap, 0, RECORD_BITMAP_SIZE );

  for( sort_index_ctr = 0; sort_index_ctr < MAX_NUM_OF_SORT_INDEXS; ++sort_index_ctr )
  {
    tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr]  = INVALID_SORT_INDEX;
    tmp_ptrFieldRecord -> SortedLists[sort_index_ctr]    = NULL;
  }

  /* Increment UsedFiles for the particular database in RAM */
  DbmMaster[db_handle].UsedFiles += 1;

  /* everyting ok */
  DB_RETURN( DB_OK )

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_write_record      |
+--------------------------------------------------------------------+

    PURPOSE : Write length bytes at offset into the given record in 
              (database, field)
*/
int db_write_record ( int          db_handle,	 
                      USHORT       field_id, 
                      USHORT       record_num, 
                      USHORT       offset, 
                      USHORT       length, 
                      const UBYTE* buffer )
{
  T_DB_CODE  db_ret_code;
  UBYTE fld_ctr,
        rec_ctr;
  char  user_field_file[FILENAME_LEN];
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;
  UBYTE*  record_buffer;
  UBYTE*  field_data;
  BOOL this_is_new_record = TRUE;

  TRACE_FUNCTION("db_write_record()");

  TRACE_EVENT_P1("field_id %04X", field_id);

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  /* Check if record exceeds the record size (i.e. offset + length > record size); 
     if it exceeds, return error DB_INVALID_SIZE */
  if( (offset + length) > 
      tmp_ptrFieldRecord -> RecordSize )
    DB_RETURN( DB_INVALID_SIZE );

  /* Check if FFS file is already opened using FFSFileHandle, part of field record. 
     If it is not yet opened, open the FFS file and update FFSFileHandle. 
     This is taken care in read_user_record_from_FFS */

  /* User data file name is "<DBDirectory>/UD_<field_id>" 
     This would be used below */

/* Implements Measure#32: Row 1200 */
  sprintf( user_field_file, 
           format_sUDd_str, 
           (char *)DbmMaster[db_handle].DBDirectory, 
           tmp_ptrFieldRecord -> FieldID );

  if( record_num EQ 0 )
  {
    /* If given record is equal to zero, 
        o	Find a free record using RecordBitMap.
        o	If free record not found, return DB_FULL */

    record_num = db_search_bit_in_bitmap (tmp_ptrFieldRecord->RecordBitMap,
                                          tmp_ptrFieldRecord->NumOfRecords,
                                          FALSE);

    if (record_num EQ 0)
      DB_RETURN( DB_FULL );
                                             

  }
  else
  {
    /* If given record is not equal to zero, 
        o	Check if record exists using RecordBitMap; 
          if not found, proceed to writing..
        o	Read the record from "<DBDirectory>/UD_<field_id>" and 
          compare it with given record (partially if offset > 0). Memcmp would be used.
        o	If record match, return DB_OK. */

    if (db_get_bit_in_bitmap (tmp_ptrFieldRecord->RecordBitMap, record_num))
    {
      /* record exists */
      this_is_new_record = FALSE;

      DB_MALLOC( record_buffer, length); 

      /* Implements Measure # 211 */
      db_ret_code = 
        read_write_user_record_from_FFS ( user_field_file,

#ifndef FFS_CLOSE_BEFORE_OPEN
                                    &(tmp_ptrFieldRecord -> FFSFileHandle),
#else
                                    db_handle,
                                    fld_ctr,
                                    &Dummy_FFSFileHandle,
#endif
                                    (UBYTE)record_num,
                                    tmp_ptrFieldRecord -> RecordSize,
                                    offset,
                                    length,
                                          record_buffer,
                                          FFS_O_RDONLY );


      if( db_ret_code NEQ DB_OK )
      {
        DB_MFREE( record_buffer );
        DB_RETURN( db_ret_code )
      }

      /* See if there is same data, if so, we need not overwrite */
      if( memcmp( record_buffer, buffer, length) EQ 0 )
      { /* matching data */
        DB_MFREE( record_buffer );
        DB_RETURN( DB_OK )
      }

      DB_MFREE( record_buffer );

    } /* record exists */

  } /* record_num EQ 0 ? */


  /* As per the request, write record in FFS in "<DBDirectory>/UD_<field_id>" */

  /* in case of new record, we need to create fill unfilled-data with FF */

  if( this_is_new_record )
  { /* new record */

    DB_MALLOC( record_buffer, tmp_ptrFieldRecord -> RecordSize ); 
    memset( record_buffer, 0xFF, tmp_ptrFieldRecord -> RecordSize ) ;

    /* To take care of non-sequential write 
       For example, initially when file is empty, and say user writes
       3rd record, 1st and 2nd record should be addded as dummy and then
       3rd record */

    rec_ctr = tmp_ptrFieldRecord -> NextRecordNum;

    while( rec_ctr < record_num   )
    {
      /* Implements Measure # 211 */
      db_ret_code = 
      read_write_user_record_from_FFS ( user_field_file,

#ifndef FFS_CLOSE_BEFORE_OPEN
                                   &(tmp_ptrFieldRecord -> FFSFileHandle),
#else
                                   db_handle,
                                   fld_ctr,
                                   &Dummy_FFSFileHandle,
#endif
                                   rec_ctr,
                                   tmp_ptrFieldRecord -> RecordSize,
                                   0,
                                   tmp_ptrFieldRecord -> RecordSize,
                                        record_buffer,
                                        FFS_O_RDWR );

      if( db_ret_code NEQ DB_OK )
      {
        DB_MFREE( record_buffer );
        DB_RETURN( db_ret_code )
      }

      ++rec_ctr;
    }

    /* Add the new record */

    memcpy( record_buffer + offset, buffer, length );

      /* Implements Measure # 211 */
      db_ret_code = 
      read_write_user_record_from_FFS ( user_field_file,

#ifndef FFS_CLOSE_BEFORE_OPEN
                                 &(tmp_ptrFieldRecord -> FFSFileHandle),
#else
                                 db_handle,
                                 fld_ctr,
                                 &Dummy_FFSFileHandle,
#endif
                                 (UBYTE)record_num,
                                 tmp_ptrFieldRecord -> RecordSize,
                                 0,
                                 tmp_ptrFieldRecord -> RecordSize,
                                        record_buffer,
                                        FFS_O_RDWR );


    if( db_ret_code NEQ DB_OK )
    {
      DB_MFREE( record_buffer );
      DB_RETURN( db_ret_code )
    }

    DB_MFREE( record_buffer );

    /* writing of record is over, update DBState (as IN_USE). */
    DbmMaster[db_handle].DBState = IN_USE;

  } else {
    /* overwritten record */ 

      /* Implements Measure # 211 */
      db_ret_code = 
      read_write_user_record_from_FFS ( user_field_file,

#ifndef FFS_CLOSE_BEFORE_OPEN
                                 &(tmp_ptrFieldRecord -> FFSFileHandle),
#else
                                 db_handle,
                                 fld_ctr,
                                 &Dummy_FFSFileHandle,
#endif
                                 (UBYTE)record_num,
                                 tmp_ptrFieldRecord -> RecordSize,
                                 offset,
                                 length,
                                        (UBYTE*)buffer,
                                        FFS_O_RDWR );

    if( db_ret_code NEQ DB_OK )
    {
      DB_RETURN( db_ret_code )
    }

    /* writing of record is over, update DBState (as IN_USE). */
    DbmMaster[db_handle].DBState = IN_USE;

    /* get the sort lists in RAM from FFS */
    get_sort_lists_from_FFS( (UBYTE)db_handle, fld_ctr );

    /* this is updation of record, so need to be re-sorted */
    update_in_sort_lists( (UBYTE)db_handle, fld_ctr, (UBYTE)record_num );

    /* if database is not tracked, no need to go further for history log writing */
    if (!DbmMaster[db_handle].Tracked)
      DB_VALUE_RETURN( record_num );

    /* if already Clean has been reset, only history updation */
    if (!DbmMaster[db_handle].Clean)
    {
      db_ret_code = update_history_log( db_handle, field_id, record_num );

      if( db_ret_code NEQ DB_OK )
        DB_RETURN( db_ret_code );

      DB_VALUE_RETURN( record_num )
    }

  } /* this_is_new_record */

  /* Updating RecordBitMap and Clean in FFS, ~/dbm/DD_<db_handle> */

  /* create the field data that is to be written */

  DB_MALLOC( field_data, ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );
  memset( field_data, 0, ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );
  memcpy( field_data, tmp_ptrFieldRecord -> RecordBitMap, RECORD_BITMAP_SIZE );

  if( this_is_new_record )
  { /* RecordBitMap to be updated only for new records */
    db_set_bit_in_bitmap (field_data, record_num, TRUE);
  } 

  /* clean is reset in memset, so no processing for it 
     (in case of non-tracked database, anyway we ignore it ! */ 

  db_ret_code = 
    update_field_data_in_FFS ( &DbmMaster[db_handle].FFSFileHandle,
                               db_handle,
                               fld_ctr,
                               field_data,
                               RecordBitMap_OFFSET,
                               ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );


  if( db_ret_code NEQ DB_OK )
  {
    DB_MFREE( field_data );
    DB_RETURN( db_ret_code )
  }

  DB_MFREE( field_data );
  /* updation of DD_<db_handle> is over */

  /* Updating RAM (both DbmMaster and Field records) strctures */
  tmp_ptrFieldRecord->Clean = FALSE;

  if( this_is_new_record )
  { /* RecordBitMap to be updated only for new records */
    db_set_bit_in_bitmap (tmp_ptrFieldRecord->RecordBitMap,
                          record_num,
                          TRUE);

    ++tmp_ptrFieldRecord -> UsedRecords;

    tmp_ptrFieldRecord -> NextRecordNum =  
        ( (record_num + 1) > tmp_ptrFieldRecord -> NextRecordNum ) ?
          (record_num + 1) : tmp_ptrFieldRecord -> NextRecordNum ;

    /* get the sort lists in RAM from FFS */
    get_sort_lists_from_FFS( (UBYTE)db_handle, fld_ctr );

    /* note the record number for sorting later 
       yeah, we call it *after* updating UsedRecords */
    new_in_sort_lists( (UBYTE)db_handle, fld_ctr, (UBYTE)record_num );

  }

  DbmMaster[db_handle].Clean = FALSE;

  /* history log updation */

  db_ret_code = update_history_log( db_handle, field_id, record_num );

  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  DB_VALUE_RETURN( record_num )

}


int db_update_ext_bitmap(int          db_handle, 
                               USHORT       field_id, 
                               USHORT       record_num,
                               BOOL           flag)
{
T_DB_CODE  db_ret_code;
  UBYTE fld_ctr;
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;
  UBYTE*  record_buffer;
  UBYTE*  field_data;
  BOOL this_is_new_record = TRUE;

  TRACE_FUNCTION("db_update_ext_bimap()");

 
  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  /* Updating RecordBitMap and Clean in FFS, DD_<db_handle> */

  /* create the field data that is to be written */

  DB_MALLOC( field_data, ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );
  memset( field_data, 0, ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );
  memcpy( field_data, tmp_ptrFieldRecord -> RecordBitMap, RECORD_BITMAP_SIZE );

  /* RecordBitMap to be updated only for new records */
   db_set_bit_in_bitmap (field_data, record_num, flag);

  /* clean is reset in memset, so no processing for it 
     (in case of non-tracked database, anyway we ignore it ! */ 

  db_ret_code = 
    update_field_data_in_FFS ( &DbmMaster[db_handle].FFSFileHandle,
                               db_handle,
                               fld_ctr,
                               field_data,
                               RecordBitMap_OFFSET,
                               ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );


  if( db_ret_code NEQ DB_OK )
  {
    DB_MFREE( field_data );
    DB_RETURN( db_ret_code )
  }

  DB_MFREE( field_data );
 /* RecordBitMap to be updated only for new records */
    db_set_bit_in_bitmap (tmp_ptrFieldRecord->RecordBitMap,
                          record_num,
                          flag);

    ++tmp_ptrFieldRecord -> UsedRecords;

    tmp_ptrFieldRecord -> NextRecordNum =  
        ( (record_num + 1) > tmp_ptrFieldRecord -> NextRecordNum ) ?
          (record_num + 1) : tmp_ptrFieldRecord -> NextRecordNum ; 
	
     DB_VALUE_RETURN( record_num )
}



/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
    PURPOSE : Creates or updates a sort index for a given (database, field)
*/
T_DB_CODE db_create_index ( int          db_handle, 
                            USHORT       field_id, 
                            UBYTE        sort_index, 
                            T_COMP_FUNC  compare_function, 
                            ULONG        flags )
{

  return internal_db_sort( db_handle,
                           field_id,
                           sort_index,
                           compare_function,
                           flags );
}


/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_update_index      |
+--------------------------------------------------------------------+

    PURPOSE : Sort the list for newly added/deleted records (uses incremental
              sort i.e. add the element at the right place)
*/
T_DB_CODE db_update_index ( int          db_handle, 
                            USHORT       field_id, 
                            UBYTE        sort_index, 
                            T_COMP_FUNC  compare_function, 
                            ULONG        flags )
{

  return internal_db_sort( db_handle,
                           field_id,
                           sort_index,
                           compare_function,
                           flags );
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_get_phy_from_idx  |
+--------------------------------------------------------------------+

    PURPOSE : Translates a logical entry within a given sort index to the 
              physical record number
*/
T_DB_CODE db_get_phy_from_idx ( int    db_handle, 
                                USHORT field_id, 
                                UBYTE  sort_index, 
                                USHORT order_num )
{
  T_DB_CODE          db_ret_code;
  UBYTE              fld_ctr,
                     sort_index_ctr;
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;
  char              sort_file[FILENAME_LEN];


  TRACE_FUNCTION("db_get_phy_from_idx()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  /* sanity check on order_num */
  if( ( order_num > tmp_ptrFieldRecord -> NumOfRecords ) OR
      ( order_num EQ 0 ) )
    DB_RETURN( DB_FAIL );  /* may be we can return 0xFF => no record for it ! */

  /* Search given sort_index in SortIndexList for above field_id; 
     if not found, return DB_INVALID_INDEX. */
  for( sort_index_ctr = 0; sort_index_ctr < MAX_NUM_OF_SORT_INDEXS; ++sort_index_ctr )
  {
    if( tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] EQ sort_index )
      break;
  }

  if( sort_index_ctr EQ MAX_NUM_OF_SORT_INDEXS )
    DB_RETURN( DB_INVALID_INDEX );

  /* Check if we already have sorted list in RAM structure SortedLists */

  if( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] EQ NULL )
  { 
    /* If no, populate the sorted list from file, 
       "<DBDirectory>/UD_<field_id>_sort_<sort_index>". 
       Sorted lists are freed in db_flush */

/* Implements Measure#32: Row 1177 */
    sprintf( sort_file, 
             format_sUDd_sortd_str, 
             (char *)DbmMaster[db_handle].DBDirectory, 
             tmp_ptrFieldRecord -> FieldID,
             sort_index );

    db_ret_code = 
      populate_sorted_list_from_FFS (  
#ifdef FFS_CLOSE_BEFORE_OPEN
                                      db_handle,
#endif
                                      sort_file,
                                      tmp_ptrFieldRecord -> NumOfRecords,
                                      &(tmp_ptrFieldRecord -> SortedLists[sort_index_ctr]) );
    if( db_ret_code NEQ DB_OK )
      DB_RETURN( db_ret_code );
    
  } /* if we have sort list */

  /* Get (physical) record number for given order_num from sorted list. */
  DB_VALUE_RETURN((T_DB_CODE)tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][order_num - 1] )

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_flush             |
+--------------------------------------------------------------------+

    PURPOSE : Flush all internal data structures of the database described by 
              db_handle
*/
T_DB_CODE db_flush ( int db_handle)
{
  T_DB_CODE  db_ret_code;
  UBYTE      clean_byte, /* Coded as in the FFS */
        fld_ctr,
        i;
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;

  char sort_file[FILENAME_LEN];

  T_FFS_STAT ffs_file_stat;
  T_FFS_SIZE ffs_ret_code;

  TRACE_FUNCTION("db_flush()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /* Clear "clean" in "~/dbm/DD_<db_handle>" for fields. 
     after ffs updation, update RAM data */

  clean_byte = FFS_CLEAN;

  /* Update the affected field records */

#ifdef FFS_CLOSE_BEFORE_OPEN
  db_close_user_field_files( db_handle );
#endif

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord;

  for( fld_ctr = 0; fld_ctr < DbmMaster[db_handle].NumOfFiles; ++fld_ctr )
  {
    if( tmp_ptrFieldRecord -> FieldID EQ INVALID_FIELD_ID ) /* if not valid field id */
    {
      ++tmp_ptrFieldRecord;   /* next field */
      continue;
    }

    /* Clear file handles */
#ifndef FFS_CLOSE_BEFORE_OPEN

    if (IS_FD_VALID(tmp_ptrFieldRecord->FFSFileHandle))
    {
      DB_FFS_CLOSE( tmp_ptrFieldRecord -> FFSFileHandle );
      tmp_ptrFieldRecord -> FFSFileHandle = INVALID_FD;
    }

#else
    /* Done it closing before for loop */
#endif

    /* Write sort lists to FFS and clear (free memory) sort lists */
    for( i = 0; i < MAX_NUM_OF_SORT_INDEXS; ++i )
    {
      if( tmp_ptrFieldRecord -> SortedLists[i] NEQ NULL )
      {
        /* currently I am not removing unused entries in sort list */
        /* Write sorted lists to "<DBDirectory>/UD_<field_id>_sort_<sort_index>". */

/* Implements Measure#32: Row 1193 */
        sprintf( sort_file, 
                 format_sUDd_sortd_str, 
                 (char *)DbmMaster[db_handle].DBDirectory, 
                 tmp_ptrFieldRecord -> FieldID,
                 tmp_ptrFieldRecord -> SortIndexList[i] );

        /* writing sorted lists to FFS s required in two cases
           1) the field is not clean and/or
           2) sort file does not exist, but we have sort list (happen during 
               bootup time with zero records)
         */
        
        ffs_ret_code = ffs_stat( sort_file, &ffs_file_stat );

        if( ( ffs_ret_code NEQ EFFS_OK )      AND
            ( ffs_ret_code NEQ EFFS_NOTFOUND ) )
        {
          LastFFS_ReturnCode = ffs_ret_code;
          DB_MFREE( tmp_ptrFieldRecord -> SortedLists[i] );
          tmp_ptrFieldRecord -> SortedLists[i] = NULL;
          DB_RETURN( DB_FAIL_FS )
        }

        if( ( NOT tmp_ptrFieldRecord -> Clean )  OR /* not clean (consistent) */
            ( ffs_ret_code EQ EFFS_NOTFOUND )    )  /* file does not exist */
        {
          db_ret_code =
            write_sorted_list_to_FFS ( 
#ifdef FFS_CLOSE_BEFORE_OPEN
                                       db_handle,
#endif
                                       sort_file,
                                       tmp_ptrFieldRecord -> NumOfRecords,
                                       tmp_ptrFieldRecord -> SortedLists[i] );

          if( db_ret_code NEQ DB_OK )
          {
            DB_MFREE( tmp_ptrFieldRecord -> SortedLists[i] );
            tmp_ptrFieldRecord -> SortedLists[i] = NULL;
            DB_RETURN( db_ret_code )
          }
        }

        /* writing in sort file is over */
        DB_MFREE( tmp_ptrFieldRecord -> SortedLists[i] );
        tmp_ptrFieldRecord -> SortedLists[i] = NULL;
      }
    }

    if( tmp_ptrFieldRecord -> Clean ) /* is it clean (consistent) ? */
    {
      ++tmp_ptrFieldRecord;   /* next field */
      continue;
    }

    /* Clear "clean" in "~/dbm/DD_<db_handle>" if not clean  */

    db_ret_code = 
      update_field_data_in_FFS ( &DbmMaster[db_handle].FFSFileHandle,
                                 db_handle,
                                 fld_ctr,
                                 &clean_byte,
                                 ( T_DB_FIELD_RECORD_SIZE - 1 ),
                                 1 );       /* "Clean" contained in last one byte */

    if( db_ret_code NEQ DB_OK )
    {
      /* this leaves out FFS and RAM in consistent state, 
         this is ok since FFS is in correct state for some of fields ! */
      DB_RETURN( db_ret_code )
    }

    tmp_ptrFieldRecord->Clean = TRUE;

    ++tmp_ptrFieldRecord;   /* next field */

  } /* for all field records */

  if (IS_FD_VALID(DbmMaster[db_handle].FFSFileHandle))
  {
    DB_FFS_CLOSE( DbmMaster[db_handle].FFSFileHandle );
    DbmMaster[db_handle].FFSFileHandle = INVALID_FD;
  }

  /* updation of FFS is over */

  /* Update corresponding "clean" field for database in RAM data structures, 
     also DBState = OPEN */
  DbmMaster[db_handle].Clean = TRUE;
  DbmMaster[db_handle].DBState = OPEN;

  /* done */
  DB_RETURN( DB_OK )
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_search            |
+--------------------------------------------------------------------+

    PURPOSE : Searches for an entry within a given database and a given file
*/
int db_search ( int               db_handle, 
                USHORT            field_id, 
                UBYTE             sort_index, 
                SHORT*            order_num,
                T_SEARCH_FUNC     search_function, 
                ULONG             flags, 
                const UBYTE*      search_tag )
{
  T_DB_CODE  db_ret_code;
  UBYTE fld_ctr,
        sort_index_ctr;
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;

  char sort_file[FILENAME_LEN];              

  int record_num = 0,
      index_num ;

  TRACE_FUNCTION("db_search()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  if( sort_index EQ 0 )
  {
    /* If sort_index is zero, 
        o	Do linear search using RecordBitMap, search/comparison function is given by caller.
        o	If search is successful, return the record number.
        o	Otherwise return DB_RECORD_NOT_FOUND. */

    /* Sanity check */
    if( (*order_num) > tmp_ptrFieldRecord -> NumOfRecords )
      DB_RETURN( DB_FAIL );

    /* search from next record,
       search_node->top would be zero for fresh search */

    for (record_num = *order_num + 1; 
         record_num <= tmp_ptrFieldRecord->NumOfRecords; 
         record_num++)
    {
      if (db_get_bit_in_bitmap (tmp_ptrFieldRecord->RecordBitMap, record_num))
      {
        /* ok, this is a valid record */

        if( search_function( flags,
                             search_tag, 
                             db_handle, 
                             field_id,
                             (UBYTE)record_num ) 
            EQ 0 )  /* yeah, "EQ 0" is success */
        {
          /* found it ! */
          *order_num = record_num; /* for next search */
          DB_VALUE_RETURN( record_num )
        }
      }
    } /* for bytes in bitmap */

    /* record not found */
    DB_RETURN( DB_RECORD_NOT_FOUND )

  } 

  /* If sort_index is not zero, search using sorted list */
 else 
  {
    /* Search given sort_index in SortIndexList for above field_id; 
     if not found, return DB_INVALID_INDEX. */
   for( sort_index_ctr = 0; sort_index_ctr < MAX_NUM_OF_SORT_INDEXS; ++sort_index_ctr )
   {
     if( tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] EQ sort_index )
       break;
   }

   if( sort_index_ctr EQ MAX_NUM_OF_SORT_INDEXS )
     DB_RETURN( DB_INVALID_INDEX );

   if( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] EQ NULL )
   { 
     /* If no, populate the sorted list from file, 
         "<DBDirectory>/UD_<field_id>_sort_<sort_index>". 
         Sorted lists are freed in db_flush */

/* Implements Measure#32: Row 1193 */
     sprintf( sort_file, 
               format_sUDd_sortd_str, 
               (char *)DbmMaster[db_handle].DBDirectory, 
               tmp_ptrFieldRecord -> FieldID,
               sort_index );

     db_ret_code = 
       populate_sorted_list_from_FFS (  

#ifdef FFS_CLOSE_BEFORE_OPEN
                                        db_handle,
#endif
                                        sort_file,
                                        tmp_ptrFieldRecord -> NumOfRecords,
                                        &(tmp_ptrFieldRecord -> SortedLists[sort_index_ctr]) );

     if( db_ret_code NEQ DB_OK )
        DB_RETURN( db_ret_code );

   }

    /* (Binary) search the sorted list, search/compare function is given by caller. 
       o	If search is successful, return the record number.
       o	Otherwise return DB_RECORD_NOT_FOUND */


   index_num = 
            db_binary_search( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr],
                   tmp_ptrFieldRecord -> UsedRecords,
                   order_num,
                   search_function, 
                   flags, 
                   search_tag,
                   db_handle, 
                   field_id );

   if( index_num EQ SEARCH_FAILED )
      DB_RETURN( DB_RECORD_NOT_FOUND );

    DB_VALUE_RETURN( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr][index_num - 1] )
                   
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_delete_record     |
+--------------------------------------------------------------------+

    PURPOSE : Delete a record in the given field of the given database
*/
T_DB_CODE db_delete_record ( int    db_handle,
                             USHORT field_id, 
                             USHORT record_num )
{
  T_DB_CODE  db_ret_code;
  UBYTE fld_ctr;
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;
  UBYTE*  field_data;

  TRACE_FUNCTION("db_delete_record()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  /* Using RecordBitMap, check whether record_num exists; 
     if it does not exist, return DB_OK. */

  if (!db_get_bit_in_bitmap (tmp_ptrFieldRecord->RecordBitMap, record_num))
  {
    /* if record does *not* exist */
    DB_RETURN( DB_OK )
  }

  /* Update "Clean" and "RecordBitMap" fields in "~/dbm/DD_<db_handle>" */

  /* create the field data that is to be written */

  DB_MALLOC( field_data, ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );
  memset( field_data, 0, ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );
  memcpy( field_data, tmp_ptrFieldRecord -> RecordBitMap, RECORD_BITMAP_SIZE );

  /* updating RecordBitMap */
  db_set_bit_in_bitmap (field_data, record_num, FALSE);

  /* clean is reset in memset, so no processing for it 
     (in case of non-tracked database, anyway we ignore it ! */ 

  db_ret_code = 
    update_field_data_in_FFS ( &DbmMaster[db_handle].FFSFileHandle,
                               db_handle,
                               fld_ctr,
                               field_data,
                               RecordBitMap_OFFSET,
                               ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );

  if( db_ret_code NEQ DB_OK )
  {
    DB_MFREE( field_data );
    DB_RETURN( db_ret_code )
  }

  DB_MFREE( field_data );
  /* updation of DD_<db_handle> is over */

  /* Update corresponding "Clean" and "RecordBitMap" fields in RAM data structures, 
     also update DBState (as IN_USE) and decrement UsedRecords. */

  tmp_ptrFieldRecord->Clean = FALSE;

  /* updating RecordBitMap */
  db_set_bit_in_bitmap (tmp_ptrFieldRecord->RecordBitMap, record_num, FALSE);

  /* get the sort lists in RAM from FFS */
  get_sort_lists_from_FFS( (UBYTE)db_handle, fld_ctr );

  /* delete in available sort lists
     yeah, we call it *before* updating UsedRecords */
  delete_in_sort_lists( (UBYTE)db_handle, fld_ctr, (UBYTE)record_num );

  --tmp_ptrFieldRecord -> UsedRecords;

  DbmMaster[db_handle].Clean = FALSE;
  DbmMaster[db_handle].DBState = IN_USE;

  /* history log updation */

  return update_history_log( db_handle, field_id, record_num );

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_read_record       |
+--------------------------------------------------------------------+

    PURPOSE : Read length bytes at offset from the record in given (database, field)
*/
int db_read_record ( int     db_handle, 
                     USHORT  field_id, 
                     USHORT  record_num, 
                     USHORT  offset, 
                     USHORT  length, 
                     UBYTE*  record_buffer )
{
  T_DB_CODE  db_ret_code;
  UBYTE fld_ctr;
  char  user_field_file[FILENAME_LEN];
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;

  TRACE_FUNCTION("db_read_record()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  /* Using RecordBitMap, check whether record_num exists; 
     if it does not exist, return DB_EMPTY_RECORD. */

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  if (!db_get_bit_in_bitmap (tmp_ptrFieldRecord -> RecordBitMap, record_num))
  {
    /* if record does *not* exist */
    DB_RETURN( DB_EMPTY_RECORD )
  }

  /* Check if record exceeds the record size (i.e. offset + length > record size); 
     if it exceeds, return error DB_INVALID_SIZE */
  if( (offset + length) > 
      tmp_ptrFieldRecord -> RecordSize )
    DB_RETURN( DB_INVALID_SIZE );

  /* Check if FFS file is already opened using FFSFileHandle, part of field record. 
     If it is not yet opened, open the FFS file and update FFSFileHandle. 
     This is taken care in read_user_record_from_FFS below. */

  /* Read record_num from "<DBDirectory>/UD_<field_id>"
     Put "length" number of bytes from "offset" in buffer. (assuming that 
     buffer contain enough memory i.e. caller should take care of allocating 
     enough memory space for buffer) */

  /* User data file name is "<DBDirectory>/UD_<field_id>" */

/* Implements Measure#32: Row 1200 */
  sprintf( user_field_file, 
           format_sUDd_str, 
           (char *)DbmMaster[db_handle].DBDirectory, 
           tmp_ptrFieldRecord -> FieldID );

      /* Implements Measure # 211 */
      db_ret_code = 
        read_write_user_record_from_FFS ( user_field_file,

#ifndef FFS_CLOSE_BEFORE_OPEN
                                &(tmp_ptrFieldRecord -> FFSFileHandle),
#else
                                db_handle,
                                fld_ctr,
                                &Dummy_FFSFileHandle,
#endif
                                (UBYTE)record_num,
                                tmp_ptrFieldRecord -> RecordSize,
                                offset,
                                length,
                                          record_buffer,
                                          FFS_O_RDONLY );


  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  DB_VALUE_RETURN( record_num )
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_remove            |
+--------------------------------------------------------------------+

    PURPOSE : Remove a database. The database must not be in use.
*/
T_DB_CODE db_remove ( const char* directory )
{
  int db_handle;

  UBYTE db_ctr,
        fld_ctr;

  T_DB_CODE          db_ret_code;

  UBYTE* dbm_data;
  UBYTE field_file[FILENAME_LEN];


 
  TRACE_FUNCTION("db_remove()");

  /* DBM_State check */
  DBM_State_check;

  /* Search for "directory" in DbmMaster
     If not found, return DB_INVALID_DB */

  for( db_ctr = 0; db_ctr < MAX_DBs; ++db_ctr )
  {
    if ( DbmMaster[db_ctr].DBState NEQ UNUSED_ENTRY )
    {
        if( memcmp( DbmMaster[db_ctr].DBDirectory, 
                    directory, 
                    strlen(directory) ) EQ 0 )
          break;
    }
  }

  if( db_ctr EQ MAX_DBs )
    DB_RETURN( DB_INVALID_DB );

  db_handle = db_ctr;

  /* If DBState for found database in DbmMaster is OPEN or IN_USE, return error DB_IN_USE */

  if( ( DbmMaster[db_ctr].DBState EQ OPEN ) OR
      ( DbmMaster[db_ctr].DBState EQ IN_USE ) )
     DB_RETURN( DB_IN_USE );


  /* Delete all files under directory "<DBDirectory>" (DBDirectory = directory) */

  for( fld_ctr = 0; fld_ctr < DbmMaster[db_handle].NumOfFiles; ++fld_ctr )
  {
    db_ret_code = remove_field_from_FFS( (UBYTE)db_handle, fld_ctr );

    if( db_ret_code NEQ DB_OK )
      DB_RETURN( db_ret_code );
  }

  /* Delete the DBDirectory */
  db_ret_code = delete_file_dir_from_FFS ( (const char*)DbmMaster[db_handle].DBDirectory );

  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );
  
  /* Delete Field file = DD_<pos. in DD_master> */

/* Implements Measure#32: Row 1195, 1203 */
  sprintf( (char*)field_file, format_sDDd_str, db_dir, db_ctr);    

  db_ret_code = delete_file_dir_from_FFS ( (const char*)field_file );

  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /* Update FFS data i.e. file, "~/dbm/DD_master" */

  /* prepare the dbm data and write */

  DB_MALLOC( dbm_data, T_DB_MASTER_RECORD_SIZE );
  memset( dbm_data, 0xFF, T_DB_MASTER_RECORD_SIZE );

/* Implements Measure#32: Row 1201 */
  db_ret_code = 
    update_dbm_data_in_FFS ( db_master_file,
                             (UBYTE) db_handle,   /* to supress warning */
                             T_DB_MASTER_RECORD_SIZE,
                             dbm_data,
                             0,
                             T_DB_MASTER_RECORD_SIZE );

  if( db_ret_code NEQ DB_OK )
  {
    DB_MFREE( dbm_data );
    DB_RETURN( db_ret_code )
  }

  DB_MFREE( dbm_data );

  /* updation of DD_master done */

  /* Free the memory for field records. 
     Update this entry in DbmMaster as unused. (DBState = UNUSED_ENTRY). 
     Also, decrement UsedDBs. */

  DB_MFREE( DbmMaster[db_handle].ptrFieldRecord );

  DbmMaster[db_handle].DBState = UNUSED_ENTRY;

  --UsedDBs;

  /* everything ok ! */
  DB_RETURN( DB_OK )

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_close             |
+--------------------------------------------------------------------+

    PURPOSE : Close a database; if in use, it flushes it before closing it.
*/
T_DB_CODE db_close ( int db_handle )
{
  TRACE_FUNCTION("db_close()");

  /* DBM_State check */
  DBM_State_check;

  /* Check for db_handle range */
  if( db_handle >= MAX_DBs )
    DB_RETURN( DB_INVALID_DB );

  /* If DBState for db_handleth position in DbmMaster is 
      o	UNUSED_ENTRY, return error DB_INVALID_DB
      o	CLOSED, return DB_OK.
      o	OPEN, update DBState as CLOSED and return DB_OK.
      o	IN_USE, call db_flush, update DBState as CLOSED and return DB_OK */

  switch( DbmMaster[db_handle].DBState )
  {
    case UNUSED_ENTRY: DB_RETURN( DB_INVALID_DB )

    case CLOSED: DB_RETURN( DB_OK )

    case OPEN:  
    case IN_USE:  {
                    db_flush( db_handle );
                    DbmMaster[db_handle].DBState = CLOSED; 
                    DB_RETURN( DB_OK )
                  }

    default: break;
  }
  DB_RETURN( DB_FAIL )
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_exit              |
+--------------------------------------------------------------------+

    PURPOSE : Shutdown the data base
*/
void db_exit ( void )
{
  /* It is assumed as "forced" exit and so no error would be 
     raised even if database is in use. Also, here we will not 
     delete the database, but just sync the database and free 
     the RAM structures.

    1)	Call db_flush for all databases present in DbmMaster.
    2)	Free the memory for field records for all databases.
    3)	Update all entries in DbmMaster as unused (this step is not required)
    4)	Set DBM_State as DBM_NOT_INITIALISED. */

  UBYTE db_ctr;

  for ( db_ctr = 0; db_ctr < MAX_DBs; ++db_ctr )
  {
    if( DbmMaster[db_ctr].DBState EQ UNUSED_ENTRY )
      continue;

    db_close( db_ctr );

    DB_MFREE( DbmMaster[db_ctr].ptrFieldRecord );

    DbmMaster[db_ctr].DBState = UNUSED_ENTRY;

  }

  DBM_State = DBM_NOT_INITIALISED;

  UsedDBs = 0;

  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_remove_field      |
+--------------------------------------------------------------------+

    PURPOSE : Delete field in given database
*/
T_DB_CODE db_remove_field ( int    db_handle, 
                            USHORT field_id )
{
  UBYTE fld_ctr;
  T_DB_CODE db_ret_code;
  UBYTE* field_data;
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;

  TRACE_FUNCTION("db_remove_field()");

  TRACE_EVENT_P1 ("Removing field %04X", field_id);

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field id not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  /* >>>HM 4-Sep-2006
   It would be nice if we would close here only the file
   which is opened and not everything, but such a solution is impossible
   within the work environment at the customer.*/
#ifdef FFS_CLOSE_BEFORE_OPEN
  db_close_user_field_files( db_handle );
#endif
/*   <<< HM 4-Sep-2006 */

#ifndef FFS_CLOSE_BEFORE_OPEN
  /* Check if FFS file is open using FFSFileHandle, part of field record; 
     if so, return error DB_IN_USE. */
  if (IS_FD_VALID(tmp_ptrFieldRecord->FFSFileHandle)
  {
    TRACE_ERROR ("File handle in use");
    DB_RETURN( DB_IN_USE );
  }
#else
  if( db_status_user_field_file( db_handle, fld_ctr ) EQ OPENED_FOR_WRITE )
  {
    TRACE_ERROR ("File handle in use for writing");
    DB_RETURN( DB_IN_USE )
  }
#endif

  /* Remove file "<DBDirectory>/UD_<field_id>" */

  db_ret_code = remove_field_from_FFS( (UBYTE)db_handle, fld_ctr );

  if( db_ret_code NEQ DB_OK )
  {
    TRACE_ERROR ("Could not remove user field file");
    DB_RETURN( db_ret_code )
  }

  /* Update "~/dbm/DD_<db_handle>" for removed field id. */

  /* create the field data that is to be written */

  DB_MALLOC( field_data, ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );
  memset( field_data, 0xFF, ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );

  db_ret_code = 
    update_field_data_in_FFS ( &DbmMaster[db_handle].FFSFileHandle,
                               db_handle,
                               fld_ctr,
                               field_data,
                               RecordBitMap_OFFSET,
                               ( T_DB_FIELD_RECORD_SIZE - RecordBitMap_OFFSET ) );

  if( db_ret_code NEQ DB_OK )
  {
    TRACE_ERROR ("Could not update master file");
    DB_MFREE( field_data );
    DB_RETURN( db_ret_code )
  }

  DB_MFREE( field_data );
  /* updation of DD_<db_handle> is over */

  /* update in RAM */

  tmp_ptrFieldRecord -> FieldID = INVALID_FIELD_ID;

  /* One field less */
  DbmMaster[db_handle].UsedFiles--; // HM 5-Sep-2006 Midnight patch ADN FDN switch

  DB_RETURN( DB_OK )

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_read_change_log   |
+--------------------------------------------------------------------+

    PURPOSE : Get an information about the changed records since the last 
              call of this function
*/
T_DB_CODE db_read_change_log ( int           db_handle, 
                               T_DB_CHANGED* changed)
{
  T_DB_CODE db_ret_code;
  TRACE_FUNCTION("db_read_change_log()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /* Copy contents of internal history log to HistoryRecord */
  *changed = DB_History_log;  /* This would work ?? */
   
  /* Clear internal history log. */
  DB_History_log.entries = 0; /* this is enough for clearing history log ! */

  /* done */
  DB_RETURN( DB_OK )

}

#ifdef _SIMULATION_ /* HM 19-Oct-2006: Currently not used but to be kept */
/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_remove_index      |
+--------------------------------------------------------------------+

    PURPOSE : Removes a sort index
*/
T_DB_CODE db_remove_index ( int    db_handle,
                            USHORT field_id,
                            UBYTE  sort_index )
{
  T_DB_CODE db_ret_code;
  UBYTE     fld_ctr,sort_index_ctr;
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;
  char sort_file[FILENAME_LEN];              
  UBYTE* field_data;
   
  TRACE_FUNCTION("db_remove_index()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  /* Search given sort_index in SortIndexList for above field_id; 
     if not found, return DB_INVALID_INDEX. */
  for( sort_index_ctr = 0; sort_index_ctr < MAX_NUM_OF_SORT_INDEXS; ++sort_index_ctr )
  {
    if( tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] EQ sort_index )
      break;
  }

  if( sort_index_ctr EQ MAX_NUM_OF_SORT_INDEXS )
    DB_RETURN( DB_INVALID_INDEX );

  /* Remove file, "<DBDirectory>/UD_<field_id>_sort_<sort_index>" */

/* Implements Measure#32: Row 1193 */
  sprintf( sort_file, 
           format_sUDd_sortd_str, 
           DbmMaster[db_handle].DBDirectory, 
           tmp_ptrFieldRecord -> FieldID,
           tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] );

  db_ret_code = delete_file_dir_from_FFS ( sort_file );

  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );        

  /* Update file , "~/dbm/DD_<db_handle>" for SortIndexes. */

  /* create the field data that is to be written */

  DB_MALLOC( field_data, MAX_NUM_OF_SORT_INDEXS );
  memcpy( field_data, tmp_ptrFieldRecord -> SortIndexList, MAX_NUM_OF_SORT_INDEXS );

  field_data[sort_index_ctr] = INVALID_SORT_INDEX;

  db_ret_code = 
    update_field_data_in_FFS ( &DbmMaster[db_handle].FFSFileHandle,
                               db_handle,
                               fld_ctr,
                               field_data,
                               SortIndexList_OFFSET,
                               MAX_NUM_OF_SORT_INDEXS );
  if( db_ret_code NEQ DB_OK )
  {
    DB_MFREE( field_data );
    DB_RETURN( db_ret_code )
  }

  DB_MFREE( field_data );
  /* updation of DD_<db_handle> is over */

  /* Update corresponding RAM structure i.e. DbmFieldRecord 
     Check if we have sort list corresponding to this sort index; if so, free it. */

  tmp_ptrFieldRecord -> SortIndexList[sort_index_ctr] = INVALID_SORT_INDEX;
  if( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] NEQ NULL )
  {
    DB_MFREE( tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] );
    tmp_ptrFieldRecord -> SortedLists[sort_index_ctr] = NULL;
  }

  DB_RETURN( DB_OK )
}
#endif /* #ifdef _SIMULATION_ */

#ifdef _SIMULATION_ /* HM 19-Oct-2006: Currently not used but to be kept */
/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_field_changed     |
+--------------------------------------------------------------------+

    PURPOSE : Check whether a database field has been changed since last 
              db_flush().
*/
T_DB_CODE db_field_changed ( int    db_handle, 
                             USHORT field_id,
                             BOOL*  changed )
{
  UBYTE fld_ctr;
  T_DB_CODE db_ret_code;

  TRACE_FUNCTION("db_field_changed()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  /* Check "Clean" field in field record; 
     a) if (Clean = true), changed = false. */

  *changed = NOT DbmMaster[db_handle].ptrFieldRecord[fld_ctr].Clean;

  DB_RETURN( DB_OK )
}
#endif /* #ifdef _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_record_empty      |
+--------------------------------------------------------------------+

    PURPOSE : Check whether a database field is empty. The result is 
              given to the caller in the *empty variable
*/
T_DB_CODE db_record_empty ( int    db_handle,
                            USHORT field_id,
                            USHORT record_num,
                            BOOL*  empty )
{
  UBYTE fld_ctr;
  T_DB_CODE db_ret_code;
  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;

  TRACE_FUNCTION("db_record_empty()");

  *empty = TRUE;

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  /* Using RecordBitMap, check whether record_num is free; 
     if so, empty = true and false otherwise */

  if (db_get_bit_in_bitmap (tmp_ptrFieldRecord -> RecordBitMap, record_num))
  {
    /* if record exist */
    *empty = FALSE;
  }

  DB_RETURN( DB_OK )

}


/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_find_free_record  |
+--------------------------------------------------------------------+

    PURPOSE : Find an free record in given (database, field)
*/
int db_find_free_record	( int    db_handle,
                          USHORT field_id )
{
  UBYTE fld_ctr;
  T_DB_CODE  db_ret_code;

  int record_num;

  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;


  TRACE_FUNCTION("db_find_free_record()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  /* Using RecordBitMap, search for free record.
     If found , return the found record_num
     Return DB_RECORD_NOT_FOUND */

  record_num = db_search_bit_in_bitmap (tmp_ptrFieldRecord->RecordBitMap,
                                        tmp_ptrFieldRecord->NumOfRecords,
                                        FALSE);

  if (record_num EQ 0)
    DB_RETURN( DB_RECORD_NOT_FOUND );

  DB_VALUE_RETURN( record_num )

}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_info              |
+--------------------------------------------------------------------+

    PURPOSE : Get information about an open database
*/
T_DB_CODE db_info ( int        db_handle, 
                    T_DB_INFO* db_info )
{
  T_DB_CODE db_ret_code;

  TRACE_FUNCTION("db_info()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  (*db_info).clean   = DbmMaster[db_handle].Clean;
  (*db_info).tracked = DbmMaster[db_handle].Tracked;

  DB_RETURN( DB_OK )
}

/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_info_field        |
+--------------------------------------------------------------------+

    PURPOSE : Get general information about a field in an open database.
*/
T_DB_CODE db_info_field ( int              db_handle,	
                          USHORT           field_id, 
                          T_DB_INFO_FIELD* info_field )
{
  UBYTE fld_ctr;
  T_DB_CODE db_ret_code;

  T_DBM_FIELDRECORD* tmp_ptrFieldRecord;


  TRACE_FUNCTION("db_info_field()");

  /* DBM_State check */
  DBM_State_check;

  /* db_handle check */
  db_ret_code = db_handle_check( db_handle );
  if( db_ret_code NEQ DB_OK )
    DB_RETURN( db_ret_code );

  /*	field id search; if field not found, return error DB_INVALID_FIELD */
  fld_ctr = field_id_search( db_handle, field_id );
  if( fld_ctr EQ DbmMaster[db_handle].NumOfFiles )
    DB_RETURN( DB_INVALID_FIELD );

  tmp_ptrFieldRecord = DbmMaster[db_handle].ptrFieldRecord + fld_ctr;

  /* Using data available in RAM in field record, fill up info_field 
     (i.e. clean, db_type, num_of_records and used_records). */

  (*info_field).clean        = tmp_ptrFieldRecord -> Clean;
  (*info_field).entry_type   = tmp_ptrFieldRecord -> DBType;
  (*info_field).record_size  = tmp_ptrFieldRecord -> RecordSize;
  (*info_field).num_records  = tmp_ptrFieldRecord -> NumOfRecords;
  (*info_field).used_records = tmp_ptrFieldRecord -> UsedRecords;

  DB_RETURN( DB_OK )

}

#ifdef _SIMULATION_ /* HM 19-Oct-2006: Currently not used but to be kept */
/*
+--------------------------------------------------------------------+
| PROJECT: PHB                          MODULE: DBM                  |
|                                       ROUINE: db_get_last_fs_error |
+--------------------------------------------------------------------+

    PURPOSE : Delivers the last error provided by the (flash) file system
              that resulted in DB_FAIL_FS.
*/
int db_get_last_fs_error ( void )
{
   DB_VALUE_RETURN( LastFFS_ReturnCode )
}
#endif /* #ifdef _SIMULATION_ */

/* Implements Measure # 83 */
/*
+--------------------------------------------------------------------+
| PROJECT: PHB                       MODULE: DBM                     |
|                                    ROUTINE: db_close_for_read_write|
+--------------------------------------------------------------------+

    PURPOSE : Close Read & write FFS Operations
*/

LOCAL void db_close_for_read_write ( FileHandleRecord  *tmp_open_fields,
                                     UBYTE             *tmp_old,
                                     UBYTE              max_per_db)
{

  TRACE_FUNCTION ("db_close_for_read_write ()");

  if( *tmp_old >= max_per_db )
  {
    *tmp_old = 0;
  }

  DB_FFS_CLOSE( tmp_open_fields[*tmp_old].filehandle );
  tmp_open_fields[*tmp_old].filehandle = INVALID_FD;
  tmp_open_fields[*tmp_old].fld_ctr = INVALID_FLD_CTR;

  ++(*tmp_old);
} 

/* Implements Measure # 211 */
/*
+-------------------------------------------------------------------------------+
| PROJECT: PHB                         MODULE:  DBM                             |
|                                      ROUTINE: read_write_user_record_from_FFS |
+-------------------------------------------------------------------------------+

    PURPOSE : Reads OR Writes user record into FFS
*/
LOCAL T_DB_CODE read_write_user_record_from_FFS ( const char      *user_field_file,

#ifdef FFS_CLOSE_BEFORE_OPEN
                                                  UBYTE            db_ctr,
                                                  UBYTE            fld_ctr,
#endif
                                                  T_FFS_FD        *filehandle,
                                                  UBYTE            record_num,
                                                  USHORT           record_size,
                                                  USHORT           offset,
                                                  USHORT           length,
                                                  UBYTE           *record_buffer,
                                                  T_FFS_OPEN_FLAGS open_option)
{
  T_FFS_FD   ffs_fd = INVALID_FD;
  T_FFS_SIZE ffs_ret_code;

  T_DB_CODE db_code;

  /* See if file already opened, 
     if not open it and update the file handle */

  TRACE_FUNCTION ("read_write_user_record_from_FFS ()");

#ifndef FFS_CLOSE_BEFORE_OPEN
  ffs_fd = *filehandle;

  if( !IS_FD_VALID(ffs_fd) )
  {
    DB_FFS_OPEN( ffs_fd, user_field_file, open_option );

    if (open_option EQ FFS_O_RDONLY )
    {
      TRACE_EVENT_P1( "read_user_record_from_FFS:DB_FFS_OPEN %d", ffs_fd );
    }
    else
    {
      TRACE_EVENT_P1( "write_user_record_to_FFS:DB_FFS_OPEN %d", ffs_fd );
    }

    if( (ffs_fd EQ EFFS_NOTFOUND)    OR
        (ffs_fd EQ EFFS_NAMETOOLONG) OR
        (ffs_fd EQ EFFS_BADNAME)     OR
        (ffs_fd EQ EFFS_INVALID)     OR
        (ffs_fd EQ EFFS_LOCKED)      )
    {
      return DB_FAIL;
    }

    if( ffs_fd < EFFS_OK )
    {
      LastFFS_ReturnCode = ffs_fd;
      return DB_FAIL_FS;
    }

    *filehandle = ffs_fd;
  }

#else /* FFS_CLOSE_BEFORE_OPEN */

  ffs_fd = db_open_user_field_file( db_ctr, fld_ctr, user_field_file, open_option );

#endif /* FFS_CLOSE_BEFORE_OPEN */

  ffs_ret_code = ffs_seek( ffs_fd, 
                           ( (record_num - 1) * (record_size) + offset),
                           FFS_SEEK_SET ); 

  if (open_option EQ FFS_O_RDONLY )
  {
    TRACE_EVENT_P1( "read_user_record_from_FFS:ffs_seek %d", ffs_ret_code );
  }
  else
  {
    TRACE_EVENT_P1( "write_user_record_to_FFS:ffs_seek %d", ffs_ret_code );
  }

  db_code = check_ffs_ret_code (ffs_ret_code, filehandle, ffs_fd, TRUE);

  if (db_code NEQ DB_OK)
  {
      return db_code;
  }


  if (open_option EQ FFS_O_RDONLY )
  {
    ffs_ret_code = ffs_read( ffs_fd, 
                             record_buffer, 
                             length ); 
    TRACE_EVENT_P1( "read_user_record_from_FFS:ffs_read %d", ffs_ret_code );
  }
  else
  {
    ffs_ret_code = ffs_write( ffs_fd, 
                              record_buffer, 
                              length ); 
    TRACE_EVENT_P1( "write_user_record_to_FFS:ffs_write %d", ffs_ret_code );
  }

  db_code = check_ffs_ret_code (ffs_ret_code, filehandle, ffs_fd, FALSE);

  if (db_code NEQ DB_OK)
  {
      return db_code;
  }

  /* DB_FFS_CLOSE( ffs_fd ); we will do it in db_flush */

#ifdef FFS_OPEN_PROBLEM_PATCH
  DB_FFS_CLOSE( ffs_fd );
  *filehandle = INVALID_FD;
#endif

  return DB_OK;
}

/* Implements Measure # 211 */
/*
+-------------------------------------------------------------------------------+
| PROJECT: PHB                         MODULE:  DBM                             |
|                                      ROUTINE: check_ffs_ret_code              |
+-------------------------------------------------------------------------------+

    PURPOSE : Checks ffs_ret_code
*/
LOCAL T_DB_CODE check_ffs_ret_code (T_FFS_SIZE ffs_ret_code,
                                    T_FFS_FD  *filehandle,
                                    T_FFS_FD   ffs_fd,
                                    BOOL       check_invalid)
{

  TRACE_FUNCTION ("check_ffs_ret_code ()");

  if( ( ffs_ret_code EQ EFFS_BADFD )   OR
      ( (ffs_ret_code EQ EFFS_INVALID) AND check_invalid ) OR
      ( ffs_ret_code EQ EFFS_BADOP )   )
  {
    DB_FFS_CLOSE( ffs_fd );
    *filehandle = INVALID_FD;
    return DB_FAIL;
  }
 
  if( ffs_ret_code < EFFS_OK )
  {
    LastFFS_ReturnCode = ffs_ret_code;
    DB_FFS_CLOSE( ffs_fd );
    *filehandle = INVALID_FD;
    return DB_FAIL_FS;
  }
  return DB_OK;
}


#endif /* #ifdef TI_PS_FFS_PHB */

