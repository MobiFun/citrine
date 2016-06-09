/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * FFS Types and globals
 *
 * $Id: ffs.h 1.19.1.45.1.26 Mon, 28 Apr 2003 11:27:14 +0200 cm $
 *
 ******************************************************************************/

#ifndef _FFS_H_
#define _FFS_H_

#ifndef TARGET
#define	TARGET	1
#define	_RVF	1
#endif

#ifdef _RVF
#include "../../riviera/rvf/rvf_api.h"
#include "../../riviera/rvm/rvm_use_id_list.h"
#endif

/******************************************************************************
 * Types
 ******************************************************************************/

#ifndef BASIC_TYPES
#define BASIC_TYPES
typedef signed   char  int8;
typedef unsigned char  uint8;
typedef signed   short int16;
typedef unsigned short uint16;
typedef signed   int   int32;
typedef unsigned int   uint32;
#endif

#if (TARGET == 1)
// Unique message offset returned in the header of each mail (msg_id).
#define FFS_MESSAGE_OFFSET	 BUILD_MESSAGE_OFFSET(FFS_USE_ID)
#endif

typedef int8   effs_t;      // error type
//typedef int    effs_t;
typedef int32  req_id_t;    // request id
typedef int32  offset_t;    // offset from first address of ffs.
typedef uint32 location_t;  // object location offset
typedef int32  blocksize_t; // can hold size of a block
typedef uint8  objflags_t;  // object flags
typedef uint8  objtype_t;   // object type

typedef int16  iref_t;      // inode reference
typedef int8   bref_t;      // block reference

typedef int32  fd_t;        // file descriptor
typedef uint16 ffs_options_t;  // option flags to open() and file_write()

// For directory operations
struct dir_s {
    iref_t this;   // iref of dir that was opened
    iref_t index;  // last inode returned by ffs_readdir()
};

// File stat structure
struct stat_s {
    objtype_t  type;
    objflags_t flags;
    iref_t     inode;
    int        size;     // size of data space occupied by object
};

// File xstat structure
struct xstat_s {
    objtype_t  type;
    objflags_t flags;
    iref_t     inode;
    int        size;     // size of data space occupied by object
    int        space;    // size of physical data space occupied by object
    location_t location;
    uint8      reserved; // only for debug
    bref_t     block;    // only for debug
    uint16     sequence; // only for debug
    uint16     updates;  // only for debug
};

#if (TARGET == 0)
// Only use to run on PC and not in target. Must be syncron with the typedef
// from rv_general.h
typedef void (*CALLBACK_FUNC)(void *);

typedef uint16 T_RVF_ADDR_ID;

/* define return_path */              
typedef struct
{
	T_RVF_ADDR_ID	addr_id;
	void			(*callback_func)(void *);
} T_RV_RETURN;

/* Define the header of each message used in Riviera. */
typedef struct {
    uint32			msg_id;
	void			(*callback_func)(void *);
    T_RVF_ADDR_ID	src_addr_id;
	T_RVF_ADDR_ID	dest_addr_id;
} T_RV_HDR;

// Used riviera types
typedef uint16          UINT16;
typedef int8            INT8;

#endif 

// Confirm mail sent from FFS task to caller (application)
struct ffs_file_cnf_s {
    T_RV_HDR header;
    int       error;      // error code of FFS operation
    req_id_t  request_id; // Unique id number
    char      *path;      // path name of object operation was performed on
};

struct ffs_stream_cnf_s {
    T_RV_HDR header;
    int       error;      // error code of FFS operation
    req_id_t  request_id; // Unique id number
    fd_t      fdi;        // file descriptor
};

/******************************************************************************
 * RVF Types
 ******************************************************************************/

typedef ffs_options_t   T_FFS_OPEN_FLAGS;
typedef int             T_FFS_SIZE;
typedef offset_t        T_FFS_OFFSET;
typedef effs_t          T_FFS_RET;
typedef req_id_t        T_FFS_REQ_ID;
typedef int             T_FFS_WHENCE;
typedef fd_t            T_FFS_FD;
typedef objtype_t       T_FFS_OBJECT_TYPE;
typedef objflags_t      T_FFS_FLAGS;
typedef struct stat_s   T_FFS_STAT;
typedef struct xstat_s  T_FFS_XSTAT;
typedef struct dir_s    T_FFS_DIR;
typedef struct ffs_file_cnf_s     T_FFS_FILE_CNF;
typedef struct ffs_stream_cnf_s   T_FFS_STREAM_CNF;

/******************************************************************************
 * Errors
 ******************************************************************************/

enum FFS_ERRORS {
    EFFS_OK          =  0,  /* ok */
    EFFS_NODEVICE    = -1,  /* flash device unknown */
    EFFS_CORRUPTED   = -2,  /* filesystem corrupted!? */
    EFFS_NOPREFORMAT = -3,  /* ffs not preformatted */
    EFFS_NOFORMAT    = -4,  /* ffs not formatted */
    EFFS_BADFORMAT   = -5,  /* incompatible ffs version, re-format needed */
    EFFS_MAGIC       = -6,  /* bad magic */
    EFFS_AGAIN       = -7,  /* not ready, try again later */
    EFFS_NOSYS       = -8,  /* function not implemented */
    EFFS_DRIVER      = -9,  /* ffs device driver error */

    EFFS_NOSPACE     = -10, /* out of data space */
    EFFS_FSFULL      = -11, /* file system full, no free inodes */
    EFFS_BADNAME     = -12, /* bad filename */
    EFFS_NOTFOUND    = -13, /* object not found */
    EFFS_EXISTS      = -14, /* object exists */
    EFFS_ACCESS      = -15, /* access permission violation */
    EFFS_NAMETOOLONG = -16, /* filename too long */
    EFFS_INVALID     = -17, /* invalid argument */
    EFFS_DIRNOTEMPTY = -18, /* directory not empty */
    EFFS_NOTADIR     = -19, /* object is not a directory */
    EFFS_SPARE       = -20, /* SPARE */
    EFFS_FILETOOBIG  = -21, /* file too big */
    EFFS_NOTAFILE    = -22, /* object is not a file */
    EFFS_PATHTOODEEP = -23, /* path too deep */

    EFFS_NUMFD       = -24, /* Max number of open files reached */
    EFFS_BADFD       = -25, /* Bad file descriptor */
    EFFS_BADOP       = -26, /* Bad operation */
    EFFS_LOCKED      = -27, /* The file is locked */

    EFFS_TOOBIG      = -30, /* too big (tmffs buffer overflow) */
    EFFS_MEMORY      = -31, /* out of memory */
    EFFS_MSGSEND     = -32, /* message send failed */

    /* debug errors */

    EFFS_SIBLINGLOOP = -40, /* directory sibling loop */
    EFFS_NOBLOCKS    = -41, /* No more blocks!? */
    EFFS_DBR         = -42, /* Data reclaim did not finish!? */
    EFFS_RECLAIMLOOP = -43  /* Data reclaim loop */
};


/******************************************************************************
 * Enumerations
 ******************************************************************************/

enum FFS_OBJECT_CONTROL_ACTION {
    OC_FLAGS = 1
};

enum FFS_OBJECT_TYPE {
    OT_FILE    = 1,
    OT_DIR     = 2,
    OT_LINK    = 3,
    OT_SEGMENT = 4
};

enum FFS_OBJECT_FLAGS {
    OF_READONLY = 1<<4  // object cannot be modified
};

enum FFS_OPEN {
    FFS_O_EMPTY  = 0x00,  // Okay?
    FFS_O_CREATE = 0x01,
    FFS_O_APPEND = 0x02,
    FFS_O_EXCL   = 0x04,
    FFS_O_TRUNC  = 0x08,
    FFS_O_RDONLY = 0x10,
    FFS_O_WRONLY = 0x20,
    FFS_O_RDWR   = FFS_O_RDONLY | FFS_O_WRONLY
};

enum FFS_SEEK {
    FFS_SEEK_SET = 0,
    FFS_SEEK_CUR = 1,
    FFS_SEEK_END = 2
};

// FIXME: debug indices to go into core.h
enum FFS_QUERY {              // data size, description
    Q_BYTES_FREE        =  1, // 4, number of free bytes in FFS
    Q_BYTES_USED        =  2, // 4, number of used bytes in FFS
    Q_BYTES_LOST        =  3, // 4, number of lost bytes in FFS
    Q_BYTES_MAX         =  4, // 4, number of max available bytes in FFS
    Q_BYTES_FREE_RAW    =  5, // 4, number of free raw bytes in FFS (used internal)

    Q_FD_BUF_SIZE       = 10, // 4, size of buffer used by stream functions

    Q_TM_BUFADDR        = 11, // 4, testmode buffer addr
    Q_TM_BUFSIZE        = 12, // 4, testmode ffs buffer size
    Q_DEV_BASE          = 13, // 4, FFS device base address
    Q_CHUNK_SIZE_MAX    = 14, // 4, max size of chunks made by non stream fkt.

    // FFS versions
    Q_FFS_API_VERSION   = 16, // 2, FFS API Version
    Q_FFS_DRV_VERSION   = 17, // 2, FFS Driver Version
    Q_FFS_REVISION      = 18, // 2, FFS Revision (from PRCS)
    Q_FFS_FORMAT_READ   = 19, // 2, FFS version as read from ffs
    Q_FFS_LASTERROR     = 20, // 2, FFS last error (from init)
    Q_FFS_FORMAT_WRITE  = 21, // 2, FFS version as written to ffs on format
	Q_FFS_TM_VERSION    = 22, // 2, FFS Testmode version

    // File system queries
    Q_FILENAME_MAX      = 24, // 2, max filename length
    Q_PATH_DEPTH_MAX    = 25, // 2, max path/directory nesting depth
    Q_FD_MAX            = 26, // 2, max numbers of simultaneous open files

    Q_OBJECTS_FREE      = 32, // 2, number of objects that can be created
    Q_INODES_USED       = 33, // 2, number of inodes used
    Q_INODES_LOST       = 34, // 2, number of inodes lost
    Q_OBJECTS_USED      = 33, // 2, DEPRECATED: old name for Q_INODES_USED
    Q_OBJECTS_LOST      = 34, // 2, DEPRECATED: old name for Q_INODES_LOST
    Q_OBJECTS_MAX       = 35, // 2, max number of valid objects allowed
    Q_INODES_MAX        = 36, // 2, physical total max number of inodes
    Q_INODES_HIGH       = 37, // 2, watermark for when inodes will be reclaimed
    Q_LOST_HIGH         = 38, // 2, watermark for when data block will be reclaimed

    // Device queries
    Q_DEV_MANUFACTURER  = 48, // 2, flash manufacturer ID
    Q_DEV_DEVICE        = 49, // 2, flash device ID
    Q_DEV_BLOCKS        = 50, // 2, number of FFS blocks in device
    Q_DEV_ATOMSIZE      = 51, // 2, atomsize used by FFS for this device
    Q_DEV_DRIVER        = 52, // 2, flash device driver

    // All queries below here are for debug purpose only, are unsupported
    // and can change at any time without notice!

    // Miscellaneous/Internal
    Q_BLOCKS_FREE_MIN   = 64, // 2, Number of spare blocks (0 or 1)

    Q_BLOCKS_FREE       = 70, // 2, number of free blocks

    // Debug queries
    Q_FS_FLAGS          = 80,
    Q_FS_INODES         = 81,
    Q_FS_ROOT           = 82,

    Q_OBJECTS_TOTAL     = 90, // 2, Accumulated number of valid objects
    Q_TOTAL_OBJECTS     = 90, // 2, DEPRECATED: old name for Q_OBJECTS_TOTAL

    Q_STATS_FIRST             = 100,
    Q_STATS_DRECLAIMS         = 100,
    Q_STATS_IRECLAIMS         = 101,
    Q_STATS_BRECLAIMS         = 102,
    Q_STATS_DATA_RECLAIMED    = 103,
    Q_STATS_INODES_RECLAIMED  = 104,
    Q_STATS_DATA_ALLOCATED    = 105,

    Q_REQUEST_ID_LAST         = 110,

    Q_DEBUG_FIRST             = 120,
    Q_DEBUG_0                 = 120,
    Q_DEBUG_1                 = 121,
    Q_DEBUG_2                 = 122,
    Q_DEBUG_3                 = 123,
    Q_DEBUG_LAST              = 127,

    // individual lines of the bstat array can be returned by the following
    // id plus the bstat index of the line wanted.
    Q_BSTAT                   = -128
};


/******************************************************************************
 * Function prototypes
 ******************************************************************************/

// Call-back function prototypes
T_FFS_REQ_ID ffs_fcreate_nb(const char *name, void *addr, T_FFS_SIZE size,
                         T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_fupdate_nb(const char *name, void *addr, T_FFS_SIZE size,
                         T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_fwrite_nb(const char *name, void *addr, T_FFS_SIZE size,
                        T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_file_write_nb(const char *name, void *addr, T_FFS_SIZE size,
                            T_FFS_OPEN_FLAGS flags, T_RV_RETURN *cp);

T_FFS_REQ_ID ffs_mkdir_nb(const char *name, T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_symlink_nb(const char *name, const char *actualpath, 
                            T_RV_RETURN *cp);

T_FFS_REQ_ID ffs_remove_nb(const char *namestruct, T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_fcontrol_nb(const char *pathname, INT8 action, int param, 
                             T_RV_RETURN *cp);

T_FFS_REQ_ID ffs_rename_nb(const char *oldname, const char *newname,
                           T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_preformat_nb(UINT16 magic, T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_format_nb(const char *name, UINT16 magic, T_RV_RETURN *cp);

T_FFS_REQ_ID  ffs_open_nb(const char *name, T_FFS_OPEN_FLAGS option, 
                          T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_close_nb(T_FFS_FD fdi, T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_write_nb(T_FFS_FD fdi, void *src, T_FFS_SIZE size, 
                          T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_seek_nb(T_FFS_FD fdi, T_FFS_SIZE offset, T_FFS_WHENCE whence
                         , T_RV_RETURN *cp); 
T_FFS_REQ_ID ffs_truncate_nb(const char *path, T_FFS_OFFSET length, 
                             T_RV_RETURN *cp); 
T_FFS_REQ_ID ffs_ftruncate_nb(T_FFS_FD fdi, T_FFS_OFFSET length, 
                              T_RV_RETURN *cp);
T_FFS_REQ_ID ffs_fdatasync_nb(T_FFS_FD fdi, T_RV_RETURN *cp); 

// No-call-back function prototypes
T_FFS_RET ffs_fcreate(const char *name, void *addr, T_FFS_SIZE size);
T_FFS_RET ffs_fupdate(const char *name, void *addr, T_FFS_SIZE size);
T_FFS_RET ffs_fwrite(const char *name, void *addr, T_FFS_SIZE size);
T_FFS_RET ffs_file_write(const char *name, void *addr, T_FFS_SIZE size, 
                         T_FFS_OPEN_FLAGS flags);
T_FFS_SIZE ffs_fread(const char *name, void *addr, T_FFS_SIZE size);
T_FFS_SIZE ffs_file_read(const char *name, void *addr, T_FFS_SIZE size);

T_FFS_RET ffs_mkdir(const char *name);
T_FFS_SIZE ffs_opendir(const char *name, T_FFS_DIR *dir);
T_FFS_SIZE ffs_readdir (T_FFS_DIR *dir, char *name, T_FFS_SIZE size);

T_FFS_RET ffs_symlink(const char *name, const char *actualpath);
T_FFS_SIZE ffs_readlink(const char *name, char *addr, T_FFS_SIZE size);

T_FFS_RET ffs_stat(const char *name, T_FFS_STAT *stat);
T_FFS_RET ffs_linkstat(const char *name, T_FFS_STAT *stat);
T_FFS_RET ffs_lstat(const char *name, T_FFS_STAT *stat);
T_FFS_RET ffs_xlstat(const char *name, T_FFS_XSTAT *stat);
T_FFS_RET ffs_fstat(T_FFS_FD fdi, T_FFS_STAT *stat);

T_FFS_RET ffs_remove(const char *name);
T_FFS_RET ffs_fcontrol(const char *pathname, INT8 action, int param);


T_FFS_RET ffs_rename(const char *oldname, const char *newname);

T_FFS_RET ffs_query(INT8 query, void *p);

T_FFS_RET ffs_preformat(UINT16 magic);
T_FFS_RET ffs_format(const char *name, UINT16 magic);

T_FFS_FD  ffs_open(const char *name, T_FFS_OPEN_FLAGS option);
T_FFS_RET ffs_close(T_FFS_FD fdi);
T_FFS_SIZE ffs_write(T_FFS_FD fdi, void *src, T_FFS_SIZE amount);
T_FFS_SIZE ffs_seek(T_FFS_FD fdi, T_FFS_SIZE offset, T_FFS_WHENCE whence);
T_FFS_SIZE ffs_read(T_FFS_FD fdi, void *src, T_FFS_SIZE size);

T_FFS_RET ffs_truncate(const char *path, T_FFS_OFFSET length); 

T_FFS_RET ffs_ftruncate(T_FFS_FD fdi, T_FFS_OFFSET length); 

T_FFS_RET ffs_fdatasync(T_FFS_FD fdi); 

// This function is to be implemented by user. It is defined in cfgffs.c.
extern T_FFS_RET ffs_is_modifiable(const char *name);

#endif //_FFS_H_
