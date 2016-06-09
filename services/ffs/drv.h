/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * ffs low level flash driver
 *
 * $Id: drv.h 1.15.1.25.1.1.1.20 Mon, 17 Nov 2003 08:51:37 +0100 tsj $
 *
 ******************************************************************************/

#ifndef TARGET
#define	TARGET	1
#define	_RVF	1
#endif

/******************************************************************************
 * Compile constants
 ******************************************************************************/

// FFS driver version (in four-digit BCD format). Format is MMID, where MM
// is major revision number, incremented e.g. for major revision or support
// for new flash family/driver. I is minor revision that is incremented for
// minor changes or when a bug is corrected. D is incremented when support
// of another device is added.
#define FFS_DRV_VERSION ((uint16) 0x1011)

// Default Allocation granularity of ffs data sectors (as power of two)
#define FFS_ATOM_LOG2 4


/******************************************************************************
 * Macros
 ******************************************************************************/

// Convert between address and block index. Only works if all blocks are the
// same size!
#define block2addr(block) (dev.base + dev.binfo[block].offset)

// Note that it is *VERY* important that pointers to hardware and flash are
// declared volatile, otherwise the compiler optimizes some reads and writes
// out and this results in non-working code!
#define FLASH_WRITE_HALFWORD(addr, data) *(volatile uint16 *) (addr) = (data)
#define FLASH_READ_HALFWORD(addr) *((volatile uint16 *) (addr))

#if (TARGET == 1)

#include "../../include/config.h"
#include "../../bsp/mem.h"
#if (CHIPSET == 12)
  #include "sys_inth.h"
#else
#include "../../bsp/inth.h" 
#endif

#if (CHIPSET == 3)
#define INT_REQUESTED (*(volatile uint16 *) INTH_IT_REG) & \
                        ~(*(volatile uint16 *) INTH_MASK_REG)
#elif (CHIPSET == 4 || CHIPSET == 5 || CHIPSET == 6 || CHIPSET == 7 || CHIPSET == 8 || CHIPSET == 9 || CHIPSET == 10 || CHIPSET == 11)
#define INT_REQUESTED ((*(volatile uint16 *) INTH_IT_REG1) & \
                         ~(*(volatile uint16 *) INTH_MASK_REG1)) || \
                        ((*(volatile uint16 *) INTH_IT_REG2) & \
                         ~(*(volatile uint16 *) INTH_MASK_REG2))
#elif (CHIPSET == 12)
#define INT_REQUESTED ((*(volatile uint16 *) C_INTH_IT_REG1) & \
                         ~(*(volatile uint16 *) C_INTH_MASK_REG1)) || \
                        ((*(volatile uint16 *) C_INTH_IT_REG2) & \
                         ~(*(volatile uint16 *) C_INTH_MASK_REG2))
#endif
#endif // (TARGET == 1)


/******************************************************************************
 * Types
 ******************************************************************************/

// Flash driver identifiers.
enum FFS_DRIVER {
    FFS_DRIVER_NULL          =  0, // Null driver

    FFS_DRIVER_AMD           =  2, // AMD dual/multi-bank driver
    FFS_DRIVER_AMD_SB        =  3, // AMD single-bank driver

    FFS_DRIVER_SST           =  8, // SST dual/multi-bank driver
    FFS_DRIVER_SST_SB        =  9, // SST single-bank driver

    FFS_DRIVER_INTEL         = 16, // Intel dual/multi-bank driver
    FFS_DRIVER_INTEL_SB      = 17, // Intel single-bank driver

    FFS_DRIVER_AMD_PSEUDO_SB = 32, // Test driver
    FFS_DRIVER_TEST          = 34, // Test driver

    FFS_DRIVER_RAM           = 64  // Ram driver
};


// Manufacturer identifiers. These should never have to be changed. They are
// ordered in alphabetically ascending order.
enum FFS_MANUFACTURER {
    MANUFACT_AMD     = 0x01,
    MANUFACT_ATMEL   = 0x1F,
    MANUFACT_FUJITSU = 0x04,
    MANUFACT_INTEL   = 0x89,
    MANUFACT_MXIC    = 0xC2,
    MANUFACT_SAMSUNG = 0xEC,
    MANUFACT_SHARP   = 0xB0,
    MANUFACT_SST     = 0xBF,
    MANUFACT_TOSHIBA = 0x98,
    MANUFACT_RAM     = 0xFE, // Ram 
    MANUFACT_TEST    = 0x54  // 'T'est manufacturer
};


// Flash block information for one ffs block (flash sector). Note that the
// ffs block definition might be of a smaller size then the physical flash
// sector. The ffs blocks must be defined in ascending order of addresses.
struct block_info_s {
    uint32 offset;
    uint8  size_ld;   // log2 of block size
    uint8  unused1;
    uint8  unused2;
    uint8  unused3;
};


// General flash information for one flash device
struct flash_info_s {
    const struct block_info_s *binfo; // block info array for this device
    char   *base;      // base flash address of ffs blocks
    uint16 manufact;   // read with flash A0 = 0
    uint16 device;     // read with flash A0 = 1
    uint8  driver;     // flash driver type
    uint8  numblocks;  // number of blocks defined for use by ffs
};
extern const struct flash_info_s flash_info[];

enum DEVICE_STATE {
    DEV_READ,
    DEV_ERASE,
    DEV_ERASE_SUSPEND,
    DEV_WRITE
};


// Note that it is *VERY* important that pointers to hardware and flash are
// declared volatile, otherwise the compiler optimizes some reads and writes
// out and this results in non-working code!
struct dev_s {
    char   *base;       // base flash address of ffs blocks
    struct block_info_s *binfo;
    uint16 manufact;
    uint16 device;
    volatile uint16 *addr; // address being written or erased
    uint16 data;           // data currently being written (dev.state = WRITE)
    uint32 blocksize;
    uint8  blocksize_ld;
    uint8  atomlog2;
    uint8  driver;
    uint8  state;       // state of device (DEVICE_STATE)
    uint8  numblocks;
    uint8  atomsize;
    uint8  atomnotmask;
};
extern struct dev_s dev;

    
// Flash low level driver function pointers
struct ffsdrv_s {
    int  (* init)(void);
    void (* erase)(uint8 block);
    void (* write_halfword)(volatile uint16 *dst, uint16 value);
    void (* write)(void *dst, const void *src, uint16 size);
    void (* write_end)(void);
    void (* erase_suspend)(void);
    void (* erase_resume)(void);
};
extern const struct ffsdrv_s ffsdrv;	/* const added for FreeCalypso */


/******************************************************************************
 * Function Prototypes
 ******************************************************************************/

void ffsdrv_write_byte(void *dst, uint8 value);
effs_t ffsdrv_init(void);

//extern int ffs_ram_image_address;
// We do it in a different way in FreeCalypso
