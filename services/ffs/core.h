/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * ffs core functions
 *
 * $Id: core.h 1.80.1.15.1.36 Thu, 08 Jan 2004 15:05:23 +0100 tsj $
 *
 ******************************************************************************/

#if (TARGET == 1)
#include "../../riviera/rv/rv_defined_swe.h"
#endif

/******************************************************************************
 * Compile option switches
 ******************************************************************************/

// FFS compiled with extra test functionality
#define FFS_TEST 1

// Default max number of simultaneous open files
#ifdef RVM_MSFE_SWE
#define FFS_FD_MAX 20
#else
#define FFS_FD_MAX 4
#endif

#define FFS_RECLAIM_NEW 1

/******************************************************************************
 * Compile constants
 ******************************************************************************/

// FFS API version (in four-digit BCD format)
#define FFS_API_VERSION ((uint16) 0x0642)

// FFS_DRV_VERSION is in drv.h

// TMFFS protocol version is in tmffs.h

// Magic for determining (formatted) file system version. First two digits
// represent major version, bottom two digits represent minor version. An
// ffs code compiled for one major version X is compatible with any other
// format version with major = X. Minor version is incremented when adding
// new features that does not break compatibility.
#define FFS_FORMAT_VERSION (0x0210)
#define BLOCK_MAGIC_LOW    ('f'<<8|'F') // "Ffs#"
#define BLOCK_MAGIC_HIGH   ('#'<<8|'s')
#define BLOCK_MAGIC        ((BLOCK_MAGIC_HIGH << 8)|(BLOCK_MAGIC_LOW))

// Absolute maximum number of inodes allowed
#define FFS_INODES_MAX 2048

// Default maximum number of inodes allowed
#define FFS_INODES_MAX_DEFAULT 1024

// Default number of path components (limit due to recursiveness of
// inodes_reclaim())
#define FFS_PATH_DEPTH_MAX 6

// Maximum number of blocks (flash sectors) in a ffs system. FFS_BLOCKS_MAX
// must be >= the number of blocks in the largest flash device memory
// map. It is used to allocate the number of entries in the static bstat
// array.
#define FFS_BLOCKS_MAX 128
// Default size of journal file (represented as 256'ths of the blocksize)
#define FFS_JOURNAL_SIZE_IN256THS 16  // one 16'ths of the block size. 

// Without the min size will the maximum of files (fs.blocks_files_max) in
// one block be 32 files if the blocksize is 8kB!
#define FFS_JOURNAL_SIZE_MIN 1024

#define FFS_JOURNAL_NAME ".journal"

// Default max size of file name (excluding null terminator)
#define FFS_FILENAME_MAX 20

// Maximum distance in age between youngest and oldest blocks
#define FFS_DAGE_MAX 256
#define FFS_DAGE_GAIN_MIN (FFS_DAGE_MAX / 4)
#define FFS_DAGE_EARLY_WIDTH 64

// Offset on file descriptors 
#define FFS_FD_OFFSET '1'

// Macros to set flags and test bits in flash memory words (negative logic)
#define BIT_SET(value, bits) ((value) & (~bits))
#define IS_BIT_SET(value, bits) (~(value) & (bits))

// Number of free inodes and journal entries to keep for "emergencies"
#define FFS_INODES_MARGIN  4
#define FFS_JOURNAL_MARGIN 4


/******************************************************************************
 * Macros used in both drv.c and core.c
 ******************************************************************************/

// Convert a offset_t value to a block index
#define offset2block(offset) (((uint32) offset) >> dev.binfo[0].size_ld)
 
// Convert between offset and address
#define offset2addr(offset) (dev.base + (offset))

// Size of a block
#define blocksize(block) (1 << dev.binfo[block].size_ld)

// Test if flag is set
#define is_open_option(options, flags) ((options & flags) == flags)

// Amount of reserved space. 
#define	RESERVED_LOW  2 * fs.journal_size 
#define RESERVED_NONE 0

// We have to saturate because a recently reclaimed inodes block could
// theoretically possess a high age
#define saturate_dage(dage) (dage > (2*FFS_DAGE_MAX) ? (2*FFS_DAGE_MAX) : dage) 


/******************************************************************************
 * Block Types
 ******************************************************************************/

// Block age, ie. number of times block has been erased
typedef uint16 age_t;

// Maximum age a block can have
#define BLOCK_AGE_MAX 0xFFFF

// ffs block status flags. These are stored in the first 2 bytes of
// the ffs block in the flash sector.
enum BLOCK_FLAGS {
    BF_LOST        =  0x80,  // block is lost and will soon be erased
    BF_FREE        =  0x40,  // free (preformatted and with block magic)
    BF_DATA        =  0x02,  // data
    BF_CLEANING    =  0x01,  // block is being cleaned
    BF_INODES      =  0x10,  // block contains inodes
    BF_COPYING     =  0x04   // block is a coming inodes block
};
enum BLOCK_STATES {
    BF_IS_EMPTY       = ~(0),
    BF_IS_FREE        = ~(BF_FREE),
    BF_IS_DATA        = ~(BF_FREE | BF_DATA),
    BF_IS_CLEANING    = ~(BF_FREE | BF_DATA | BF_CLEANING),
    BF_IS_COPYING     = ~(BF_FREE | BF_COPYING),
    BF_IS_INODES      = ~(BF_FREE | BF_COPYING | BF_INODES),
    BF_IS_INODES_LOST = ~(BF_FREE | BF_COPYING | BF_INODES | BF_LOST)
};

// Header of each FFS block
struct block_header_s {
    uint16 magic_low;  // 32-bit magic number
    uint16 magic_high;
    uint16 version;    // FFS_FORMAT_VERSION used for formatting
    age_t  age;        // number of times this block has been erased
    uint16 flags;      // status flags of this block (BLOCK_FLAGS)
    uint16 reserved0;
    uint16 reserved1;
    uint16 reserved2;
};

// Important the below define MUST fit to the size of the header that is written
#define BHEADER_SIZE sizeof(struct block_header_s)

#define OLD_BLOCK_MAGIC_LOW    ('S'<<8|'F') // "FS"
#define OLD_FFS_FORMAT_VERSION (0x0100)     // 1.00 (in four-digit BCD format)

// Old header of each FFS block. From old/previous FFS format version
struct block_header_old_s {
    uint8  flags;
    uint8  copied;
    uint8  magicflags;
    uint8  reserved0;
    uint16 magic_low;
    uint16 magic_high;
    uint16 reserved1;
    uint16 reserved2;
};

// Block status. This struct holds the status of one ffs block This relation
// is always valid: <block size> = <used> + <lost> + <free>. The block size
// is obtained from the corresponding block_info structure. <used> and
// <lost> variables always holds a value which is a multiple of
// FFS_GRANULARITY.  For inodes, <used> is number of inodes in active use,
// <lost> is number of deleted/lost inodes, <numfiles> is the index of the
// first free inode.
struct block_stat_s {
    blocksize_t used;    // number of used bytes
    blocksize_t lost;    // number of lost bytes
    uint16      flags;   // flash block flags (first 16 bits of each block)
    uint16      objects; // number of valid objects
};

/******************************************************************************
 * External declarations
 ******************************************************************************/

extern struct fs_s fs;
extern struct block_stat_s bstat[FFS_BLOCKS_MAX];

extern struct ffs_stats_s stats;

extern const struct block_info_s *binfo;


/******************************************************************************
 * Object Types
 ******************************************************************************/

// This enum MUST be in sync with the one in ffs.h.
enum OBJECT_TYPE_E {
    // remaining filetypes are in ffs.h
    OT_ERASED  = 0,
    OT_NULL    = 7,
    OT_MASK    = 7,
    OT_MAX     = 4
};

// This enum MUST be in sync with the one in ffs.h.
enum OBJECT_FLAGS_E {
    // remaining object flags are in ffs.h
    OF_UNDEF0   = 1<<5,
    OF_UNDEF1   = 1<<6,
    OF_EXACT    = 1<<7, // used by control()/update_commit() interaction. This
                        // is *not* an object flag!
    OF_ALL      = OF_READONLY, // all flags allowed to be changed by user
    OF_MASK     = 0xF0
};

struct inode_s {
    uint16      size;
    uint8       reserved;  // size extension?
    objflags_t  flags;
    iref_t      child;     // link to first inode in dir (this inode is a dir)
    iref_t      sibling;   // link to next inode in same directory
    location_t  location;  // location of object
    uint16      sequence;  // 
    uint16      updates;   // times this object has been updated
};

struct file_descriptor_s {      
    char    *buf;          // Write buffer 
    iref_t  seghead;       // First chunk. Contain file name and optional data 
    iref_t  wch;           // Inode of work chunk (if chunk is read to buf)
    int     fp;            // File pointer
    int     wfp;           // Work file pointer always points to start of wch
    int     size;          // Size of object (all chunks and data from buf)
    int8    options;       // Open options
    int     dirty;         // Indicate if buf contain valid data or not
};


/******************************************************************************
 * Journal types and global fs structure
 ******************************************************************************/

enum JOURNAL_FLAGS {
    JOURNAL_WRITING = 0x02,  // journal is being written to journal file
    JOURNAL_READY   = 0x04,  // journal has been written to journal file
    JOURNAL_DONE    = 0x08   // journal has been written to ffs
};

enum JOURNAL_STATES {
    JOURNAL_IS_EMPTY   = ~(0),
    JOURNAL_IS_WRITING = ~(JOURNAL_WRITING),
    JOURNAL_IS_READY   = ~(JOURNAL_WRITING | JOURNAL_READY),
    JOURNAL_IS_DONE    = ~(JOURNAL_WRITING | JOURNAL_READY | JOURNAL_DONE)
};

// Journal entry structure. Note that the state byte *MUST* be the first
// byte of the structure!
struct journal_s {
    uint8      state;    // state of journal entry.
    objflags_t flags;    // type of object
    iref_t     i;        // iref of object
    iref_t     diri;     // iref of object that is this object's parent/sibling
    iref_t     oldi;     // iref of object being replaced (only for updates)
    location_t location; // object's location
    uint16     size;     // object's size
    iref_t     repli;    // inode which is replaced
};

// Main ffs info struct (initialised by ffs_initialize())
struct fs_s {
    struct inode_s *inodes_addr; // base address of inodes
    iref_t    root;            // iref of root directory
    bref_t    inodes;          // index into bstat containing inode block
    bref_t    newinodes;       // index into bstat containing new inode block
    bref_t    blocks_free_min; // Number of spare blocks (0 or 1)
    int       filesize_max;    // Max size of object data
    int       reserved_space;  // Byte size of space reserved for journal relocation
    iref_t    inodes_max;      // Max number of inodes possible
    iref_t    inodes_high;     // number of inodes triggering an inodes_reclaim()
    iref_t    objects_max;     // Max number of objects (valid inodes) allowed
    age_t     age_max;         // Max block age found by blocks_fsck()
    iref_t    block_files_max; // max number of files in a block
    iref_t    block_files_reserved;    // Reserved for journals
    uint16    format;          // FFS version as formatted in flash blocks
    uint16    sequence;        // Object sequence number (for debug only)
    effs_t    initerror;       // ffs_initialize() return code
    uint8     lost_threshold;  // Threshold percentage for data block reclaim
    uint8     flags;           // Global FFS options/flags
    uint8     filename_max;    // Max length of a filename
    uint8     path_depth_max;  // Max path componenents allowed
    uint8     numfds;          // Mumber of available file descriptors
    uint8     testflags;
    int8      journal_depth;   // Current journal nesting depth (0 or 1)
    iref_t    ijournal;        // iref of journal file
    uint32    journal_size;    // Byte size of journal file
    uint32    journal_pos;     // Byte offset to first free entry in journal file
    struct journal_s journal;
    uint8     fd_max;          // number of max available file descriptors
    int       fd_buf_size;     // size of stream buffer
    struct file_descriptor_s fd[FFS_FD_MAX];
    struct journal_s ojournal; // "Old" journal
    int       link_child;      // Link child in journal or not
    iref_t    i_backup;        // Used by ffs_file_write()
    int       chunk_size_max;  // Max size of one chunk
    int       chunk_size_min;  // Min size of one chunk 
    uint32    debug[4];
};

// This is the layout of the FFS performance statistics file. The file is
// created with the name ".statistics" in the root directory at format. It
// is updated after each data and inodes reclaim (after writing the file
// that provoked the reclaim). The file is only updated if it exists, so if
// the user does not want the file, she can erase it after the initial
// format. FIXME: The use of the .statistics file is not yet implemented
struct ffs_stats_s {
    uint32 data_allocated;   // implemented

    struct {                 // Not yet implemented
        uint32 created;
        uint32 updated;
        uint32 read;
    } files;
    struct {                 // Not yet implemented
        uint32 written[2];
        uint32 read[2];
    } bytes;
    struct {
        uint32 most_lost;    // Block candidate
        uint32 most_unused;  // Block candidate
        uint32 youngest;     // Block candidate
        uint32 valid[2];     // Amount of valid reclaimed data 
        uint32 lost[2];      // Amount of lost reclaimed data 
    } drec;
    struct { 
        uint32 num;          // Number of inode reclaims
        uint32 valid;        // Number of valid reclaimed inodes
        uint32 lost;         // Number of lost reclaimed inodes
    } irec;
};
extern struct ffs_stats_s stats;


/******************************************************************************
 * Miscellaneous types
 ******************************************************************************/

// only used with (FFS_TEST == 1)
enum TEST_RECOVERY {
    JOURNAL_TEST_BASE       = 0x10,
    JOURNAL_TEST_EMPTY,
    JOURNAL_TEST_WRITING,
    JOURNAL_TEST_READY,
    JOURNAL_TEST_COMMITTING,
    JOURNAL_TEST_COMMITTED,
    JOURNAL_TEST_DONE,
    BLOCK_COMMIT_BASE       = 0x20,
    BLOCK_COMMIT_BEFORE,
    BLOCK_COMMIT_NO_VALID,
    BLOCK_COMMIT_OLD_FREE,
    BLOCK_COMMIT_AFTER,
    BLOCK_RECLAIM_BASE      = 0x40,
    BLOCK_RECLAIM_ALLOC,
    BLOCK_RECLAIM_CLEANING,
    BLOCK_RECLAIM_NO_CLEAN,
    BLOCK_RECOVER_OBJECTS
};

enum FLASH_DATA {
    FLASH_NULL8  = 0xFF,
    FLASH_NULL16 = 0xFFFF,
    FLASH_NULL32 = 0xFFFFFFFFL,
    IREF_NULL    = FLASH_NULL16
};


// This enum MUST be in sync with the one in ffs.h.
enum OBJECT_CONTROL {
    // remaining object control codes are in ffs.h
    OC_FS_FLAGS       =  80,
    OC_TRACE_INIT     =  82,
    OC_DEV_MANUFACT   =  88,
    OC_DEV_DEVICE     =  89,

    OC_DEBUG_FIRST    = 120,
    OC_DEBUG_0        = 120,
    OC_DEBUG_1        = 121,
    OC_DEBUG_2        = 122,
    OC_DEBUG_3        = 123,
    OC_DEBUG_LAST     = 123,

    OC_FS_TESTFLAGS   = 127
};

enum FS_FLAGS {
    FS_DIR_DATA   = 0x01   // allow directory objects to contain data.
};

enum RECLAIM_CANDIDATE {
    MOST_LOST,
    MOST_UNUSED,
    YOUNGEST
};

/******************************************************************************
 * Macros
 ******************************************************************************/

// Convert between location and offset
#define location2offset(location) ((location) << dev.atomlog2)
#define offset2location(offset) (((uint32) offset) >> dev.atomlog2)

// test if object is of a specific type
#define is_object(objp, type) (((objp)->flags & OT_MASK) == (type))

// test if object is valid (directory, file or symlink)
#define is_object_valid(ip) ((ip->flags & OT_MASK) <= OT_MAX && (ip->flags & OT_MASK) != OT_ERASED)

// test if block is in a specific state
#define is_block(block, state) (bstat[block].flags == (uint16) (state))

// test if block has certain flags set
#define is_block_flag(block, bits) (IS_BIT_SET(bstat[block].flags, (bits)))

// convert an object's data address to the address of the object's name
#define addr2name(addr) (addr)

// Convert a size to an aligned size
#define atomalign(size) (((size) + dev.atomsize-1) & ~dev.atomnotmask)
#define wordalign(size) (((size) + 3) & ~3)
#define halfwordalign(size) (((size) + 1) & ~1)

#define inode_addr(i)   (fs.inodes_addr + i)

#define JOURNAL_POS_INITIAL (wordalign(2 + sizeof(FFS_JOURNAL_NAME) + 1))


/******************************************************************************
 * Function prototypes
 ******************************************************************************/

// Helper functions

effs_t is_filename(const char *s);
int ffs_strlen(const char *s);
int ffs_strcmp(const char *s, const char *p);
char *addr2data(const char *addr, const struct inode_s *ip);

int object_datasize(iref_t i);
iref_t is_readonly(iref_t i, const char *name);
iref_t dir_traverse(iref_t i, iref_t *entries);

bref_t block_alloc(bref_t n, uint16 flags);
bref_t block_alloc_try(bref_t *n);
void block_flags_write(uint8 block, uint8 flags);

offset_t data_alloc(int size);
offset_t data_alloc_try(int size);
offset_t data_reserved_alloc(int size);

iref_t inode_alloc(void);

effs_t is_fd_valid(fd_t fdi);
effs_t is_offset_in_buf(int offset, fd_t fdi);

iref_t chunk_alloc(int realsize, int is_journal, offset_t *offset);  

iref_t inode_alloc_try(void);
fd_t get_fdi(iref_t i);

offset_t data_prealloc(int realsize);

// Functions used by API

effs_t object_update(iref_t oldi);
iref_t object_create(const char *name, const char *buf, int size,
                         iref_t dir);
int file_read(const char *name, void *addr, int size);
int stream_read(fd_t fdi, void *src, int size);
int object_read(const char *name, char *buf, int size, int linkflag);

iref_t object_stat(const char *name, struct xstat_s *stat,
                       int linkflag, int fdi, int extended);
effs_t object_remove(iref_t i);
iref_t object_rename(iref_t oldi, const char *newname, iref_t newdir);
effs_t object_control(iref_t i, int8 action, int value);
int object_truncate(const char *pathname, fd_t fdi, offset_t length);
iref_t object_lookup(const char *path, char **leaf, iref_t *dir);
iref_t object_lookup_once(const char *path, char **leaf, iref_t *dir);
iref_t dir_open(const char *name);
iref_t dir_next (iref_t dir, iref_t i, char *name, int8 size);


// Journalling

void journal_begin(iref_t oldi);
void journal_end(uint8 type);
void journal_commit(uint8 type);
int journal_push(void);
int journal_pop(void);
iref_t journal_create(iref_t oldi);
effs_t journal_init(iref_t i);


// Format, Init and Reclaim

void block_preformat(bref_t b, age_t age);
effs_t fs_preformat(void);
effs_t is_formattable(int8 flag);
effs_t fs_format(const char *fsname_and_options);

effs_t ffs_initialize(void);
void fs_params_init(const char *p);
blocksize_t block_used(bref_t b);

effs_t ffs_begin(void);
int ffs_end(int error);

int block_reclaim(bref_t b);
int blocks_reclaim(void);
void block_commit(void);

iref_t data_reclaim(int space);
int data_reclaim_try(int space);
iref_t data_block_reclaim(bref_t b, int reclaim_candidate);
iref_t object_relocate(iref_t oldi);
iref_t block_clean(bref_t b);

void block_free(bref_t block);

void inodes_set(iref_t i);
effs_t inodes_reclaim(void);

int reclaim(void);

// Internally used functions

effs_t file_read_int(const char *path, void *src, int size);
effs_t file_update(const char *path, void *src, int size);

int statistics_file_create(void);
int statistics_write(void);
void statistics_init(void);
void statistics_update_drec(int valid, int lost, int candidate);
void statistics_update_irec(int valid, int lost);

// Chunk Operations
iref_t segment_create(const char *buf, int size, iref_t dir);
int segment_datasize(const struct inode_s *ip);
int segment_read(iref_t i, char *buf, int size, int offset);
iref_t segment_next(iref_t i);
iref_t segment_traverse(iref_t i, iref_t *entries);
int segfile_seek(iref_t in_i, int in_pos, 
                     iref_t *out_i, int *out_pos_i);
iref_t chunk_traverse(iref_t i);
effs_t datasync(fd_t fdi);
// debug/test functions

void tr_bstat(void);
void tr_fd(fd_t fdi);

// These prototypes really belong in ffs.h but as they have not been
// implemented, we will not show these prototypes to application
// programmers.
effs_t fcntl(fd_t fd, int8 action, uint32 *param);
