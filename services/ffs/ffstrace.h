/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * ffs tracing
 *
 * $Id: ffstrace.h 1.38.1.4 Wed, 14 Aug 2002 13:02:34 +0200 tsj $
 *
 ******************************************************************************/


/******************************************************************************
 ** Common Tracing and logging
 *****************************************************************************/

int tr_query(int mask);


/******************************************************************************
 ** Target Tracing
 ******************************************************************************/

#if (TARGET == 1)

#define tw(contents)
#define ttw(contents) contents
#define tlw(contents)
#define NL
void ttr_init(unsigned int mask);
void ttr(unsigned trmask, char *format, ...);
void str(unsigned mask, char *string);

void led_config(unsigned char n);
void led_set(unsigned char n);
void led_counter(unsigned char n);
void led_on(unsigned char n);
void led_off(unsigned char n);
void led_toggle(unsigned char n);

enum TargetTraceMask {
    TTrTest       = 0x00000001,
    TTrTestInfo   = 0x00000002,

    TTrApi        = 0x00000010,
    TTrDrvWrite   = 0x00000020,
    TTrDrvErase   = 0x00000040,
    TTrDrvOther   = 0x00000080,

    TTrInit       = 0x00000100,
    TTrInitLow    = 0x00000200,
    TTrFormat     = 0x00000400,

    TTrObj        = 0x00001000,
    TTrInode      = 0x00002000,
    TTrData       = 0x00004000,
    TTrMisc       = 0x00008000,

    TTrRec        = 0x00020000,
    TTrPcmRead    = 0x00040000,
    TTrPcmWrite   = 0x00080000,

    TTrTask       = 0x00100000,
    TTrTaskLow    = 0x00200000,
    TTrBstat      = 0x00400000,

    TTrTmffs      = 0x08000000,

    TTrTaskDelays = 0x10000000,
    TTrFatal      = 0x80000000,

    TTrAll        = 0xFFFFFFFF
};

enum {
    LED_INIT          = 0,
    LED_DRV_INIT      = 1,
    LED_BLOCKS_FSCK   = 2,
    LED_INODES_FSCK   = 3,
    LED_WRITE_SUSPEND = 4,
    LED_WRITE         = 5,
    LED_ERASE_SUSPEND = 6,
    LED_ERASE         = 7
};


/******************************************************************************
 ** PC Tracing
 ******************************************************************************/

#else // (TARGET == 1)

void tr_init(unsigned int mask, int spaces, char *filename);
void tr(int type, unsigned int mask, char *format, ...);


//void tr_init(int mask, int level,int spaces, char *filename);
//void tr(int level, int type, char *format, ...);
#define tw(contents) contents
#define ttw(contents)
#define tlw(contents)

#define led_config(n)
#define led_set(n)
#define led_counter(n)
#define led_on(n)
#define led_off(n)

enum SimulationTraceMask {
    TrTest       = 0x00000001,
    TrTestHigh   = 0x00000002,
    TrTestLow    = 0x00000004,
    TrApi        = 0x00000008,

    TrBstat      = 0x00000010,
    TrFormat     = 0x00000020,
    TrFsck       = 0x00000040,
    TrFsckLow    = 0x00000080,

    TrObject     = 0x00000100, // object_create/remove/read/stat, etc.
    TrLookup     = 0x00000200,
    TrDirLow     = 0x00000400,
    TrDirHigh    = 0x00000800,

    TrBlock      = 0x00001000, // block_alloc/free, etc.
    TrInode      = 0x00002000, // inode_alloc, etc.
    TrData       = 0x00004000, // data_alloc

    TrIReclaim   = 0x00010000,
    TrDReclaim   = 0x00020000,
    TrReclaimLow = 0x00040000,
    TrJournal    = 0x00080000,

    TrDrvInit    = 0x00100000,
    TrDrvWrite   = 0x00200000,
    TrDrvErase   = 0x00400000,

    TrOther      = 0x01000000, // object_control, query, etc.
    TrUtil       = 0x02000000,
    TrTmffs      = 0x03000000,
    TrServer     = 0x08000000,

    TrTrace      = 0x80000000,
    TrAll        = 0xFFFFFFFF
};

enum TRACE_TYPES {
    TR_BEGIN = 0x100,
    TR_END =   0x200,
    TR_FUNC =  0x400,
    TR_NULL =  0x800
};


#endif // (TARGET == 1)
