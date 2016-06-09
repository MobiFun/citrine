/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * FFS Testmode protocol definitions
 *
 * $Id: tmffs.h 1.16.1.3 Fri, 30 Aug 2002 15:11:37 +0200 tsj $
 *
 ******************************************************************************/


/******************************************************************************
 * Prototypes
 ******************************************************************************/

int tm_ffs(unsigned char *outp, int outsize, unsigned char *inp, int insize);
int tmffs_bufsize(void);
unsigned char *tmffs_bufaddr(void);

// Version of the TMFFS protocol
#define TMFFS1_VERSION ((uint16) 0x0188)

// Version of the TMFFS2 protocol
#define TMFFS2_VERSION ((uint16) 0x0300)

#ifdef TMFFS1
#define FFS_TM_VERSION  TMFFS1_VERSION
#endif

#ifdef TMFFS2
#define FFS_TM_VERSION  TMFFS2_VERSION
#endif

/******************************************************************************
 * FFS1 Protocol Indentifiers
 ******************************************************************************/

typedef enum FFS_PROTOCOL_IDENTIFIERS {
    FPI_END = 0,         /* end */
    FPI_BEGIN,           /* begin */
    FPI_TMFFS_VERSION,   /* tmffs_version */

    FPI_PREFORMAT,       /* preformat */
    FPI_FORMAT,          /* format */

    FPI_FCREATE,         /* fcreate */
    FPI_FUPDATE,         /* fupdate */
    FPI_FWRITE,          /* fwrite */
    FPI_FREAD,           /* fread */
    FPI_REMOVE,          /* remove */

    FPI_MKDIR,           /* mkdir */
    FPI_OPENDIR,         /* opendir */
    FPI_READDIR,         /* readdir */

    FPI_STAT,            /* stat */
    FPI_LINKSTAT,        /* linkstat */

    FPI_SYMLINK,         /* symlink */
    FPI_READLINK,        /* readlink */

    FPI_QUERY,           /* query */
    FPI_FCONTROL,        /* fcontrol */

    FPI_INIT,            /* init */
    FPI_EXIT,            /* exit */

    FPI_PCM_GETFILEINFO, /* getfileinfo */
    FPI_PCM_READFILE,    /* readfile */
    FPI_PCM_WRITEFILE,   /* writefile */
    FPI_PCM_READRECORD,  /* readrecord */
    FPI_PCM_WRITERECORD, /* writerecord */

    FPI_BUFREAD,         /* buf_read */
    FPI_BUFWRITE,        /* buf_write */
    FPI_BUFSET,          /* buf_set */

    FPI_UINT8,           /* UINT8 */
    FPI_UINT16,          /* UINT16 */
    FPI_UINT32,          /* UINT32 */
    FPI_INT8,            /* INT8 */
    FPI_INT16,           /* INT16 */
    FPI_INT32,           /* INT32 */
    FPI_BUFFER,          /* BUFFER */
    FPI_DATA,            /* DATA */
    FPI_STRBUF,          /* STRBUF */
    FPI_STRING,          /* STRING */

    FPI_TFFS             /* TFFS */

} tmffs_cid_t;

typedef tmffs_cid_t tmffs_proto_type_t;


/******************************************************************************
 * FFS2 Protocol Indentifiers
 ******************************************************************************/

typedef enum FFS2_PROTOCOL_IDENTIFIERS {
	TMFFS_FORMAT     = 'f',
	TMFFS_PREFORMAT  = 'p',

	TMFFS_MKDIR      = 'm',
	TMFFS_OPENDIR    = 'o',
	TMFFS_READDIR    = 'D',
	TMFFS_REMOVE     = 'd',
	TMFFS_RENAME     = 'n',
	TMFFS_XLSTAT     = 'x',

	TMFFS_SYMLINK    = 'y',
	TMFFS_READLINK   = 'Y',

	TMFFS_OPEN       = 'O',
	TMFFS_CLOSE      = 'C',
	TMFFS_READ       = 'R',
	TMFFS_WRITE      = 'W',
	TMFFS_SEEK       = 'S',

	TMFFS_FTRUNCATE  = 'T',
	TMFFS_TRUNCATE   = 't',

	TMFFS_FILE_READ  = 'r',
	TMFFS_FILE_WRITE = 'w',

	TMFFS_FSTAT      = 'F',
	TMFFS_LSTAT      = 'l',
	TMFFS_STAT       = 's',

	TMFFS_FCONTROL   = 'c',
	TMFFS_QUERY      = 'q',

	TMFFS_INIT       = 'i',
	TMFFS_EXIT       = 'e', 

	// Special
	TMFFS_DIRXLSTAT  = 'X',

	TMFFS_VERSION    = 'v',
	TMFFS_TFFS       = 'z'
} tmffs2_cid_t; 


