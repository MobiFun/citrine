/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * ffs low level flash driver
 *
 * $Id: drv.c 1.30.1.6.1.51.1.1.1.13.1.11 Tue, 06 Jan 2004 14:36:52 +0100 tsj $
 *
 ******************************************************************************/

#include "../../include/config.h"
#include "ffs.h"
#include "drv.h"
#include "core.h"	/* for FFS_BLOCKS_MAX */
#include "ffstrace.h"
#include "intctl.h"
#include "ramffs.h"
#include <string.h>


/******************************************************************************
 * "Block info" stupidity
 ******************************************************************************/

static struct block_info_s block_info[FFS_BLOCKS_MAX];


/******************************************************************************
 * Macros
 ******************************************************************************/

#define addr2offset(address) ( (int) (address) - (int) dev.base )


/******************************************************************************
 * Generic Driver Functions
 ******************************************************************************/

// Note: This function is designed for little-endian memory addressing!
void ffsdrv_write_byte(void *dst, uint8 value)
{
    uint16 halfword;

    tw(tr(TR_FUNC, TrDrvWrite, "ffsdrv_write_byte(0x%05x, 0x%x)\n",
       (int) (addr2offset(dst)), value));
    ttw(str(TTrDrvWrite, "wb" NL));

    if ((int) dst & 1)
        halfword =                (value << 8) | *((uint8 *) dst - 1);
    else
        halfword = (*((uint8 *) dst + 1) << 8) | (value);

    ffsdrv.write_halfword((uint16 *) ((int) dst & ~1), halfword);
}

void ffsdrv_generic_write(void *dst, const void *src, uint16 size)
{
    uint8 *mydst = dst;
    const uint8 *mysrc = src;

    if (size > 0)
    {
        if ((unsigned int) mydst & 1) {
            ffsdrv_write_byte(mydst++, *mysrc++);
            size--;
        }
        while (size >= 2) {
            ffsdrv.write_halfword((uint16 *) mydst,
                                  mysrc[0] | (mysrc[1] << 8));
            size -= 2;
            mysrc += 2;
            mydst += 2;
        }
        if (size == 1)
            ffsdrv_write_byte(mydst++, *mysrc++);
    }
}

/******************************************************************************
 * Dummy Functions
 ******************************************************************************/

int ffsdrv_null_init(void)
{
    ttw(ttr(TTrDrvOther, "ffsdrv_null_init()" NL));

    return 0;
}

void ffsdrv_null_erase(uint8 block)
{
    ttw(ttr(TTrDrvErase, "ffsdrv_null_erase(%d)" NL, block));
}

void ffsdrv_null_write_halfword(volatile uint16 *addr, uint16 value)
{
    ttw(ttr(TTrDrvWrite, "ffsdrv_null_write_halfword(0x%x, 0x%x)" NL, addr, value));
}

void ffsdrv_null_write(void *dst, const void *src, uint16 size)
{
    ttw(ttr(TTrDrvWrite, "ffsdrv_null_write(0x%x, 0x%x, %d)" NL, dst, src, size));
}

void ffsdrv_null_erase_suspend(void)
{
    ttw(str(TTrDrvErase, "ffsdrv_null_erase_suspend()" NL));
}

void ffsdrv_null_erase_resume(void)
{
    ttw(str(TTrDrvErase, "ffsdrv_null_erase_resume()" NL));
}

void ffsdrv_null_write_end(void)
{
    ttw(str(TTrDrvWrite, "ffsdrv_null_write_end()" NL));
}

void ffsdrv_null_erase_end(void)
{
    ttw(str(TTrDrvErase, "ffsdrv_null_erase_end()" NL));
}

/*
 * FreeCalypso change from TI: we only compile one flash "driver" type
 * based on the build configuration.
 */

#if FFS_IN_RAM

/******************************************************************************
 * RAM Family Functions
 ******************************************************************************/

void ffsdrv_ram_write_halfword(volatile uint16 *dst, uint16 value)
{
  *dst = value;
}

#if 0
/* duplicates ffsdrv_generic_write */
void ffsdrv_ram_write(void *dst, const void *src, uint16 size)
{
    uint8 *mydst = dst;
    const uint8 *mysrc = src;
  
    if (size == 0)
        return;
    else if (size == 1)
        ffsdrv_write_byte(mydst, *mysrc);
    else {
        if ((int) mydst & 1) {
            ffsdrv_write_byte(mydst++, *mysrc++);
            size--;
        }
        while (size >= 2) {
            ffsdrv_ram_write_halfword((uint16 *) mydst, mysrc[0]|(mysrc[1] << 8));
            size -= 2;
            mysrc += 2;
            mydst += 2;
        }
        if (size == 1)
            ffsdrv_write_byte(mydst++, *mysrc++);
    }
}
#endif

void ffsdrv_ram_erase(uint8 block) 
{
    int i;
    char *addr;

    addr = block2addr(block);
    
    for (i = 0; i < (1 << dev.binfo[block].size_ld); i++) {
        *addr++ = 0xFF;
    }
}

const struct ffsdrv_s ffsdrv = {
    ffsdrv_null_init,
    ffsdrv_ram_erase,
    ffsdrv_ram_write_halfword,
    ffsdrv_generic_write,
    ffsdrv_null_write_end,
    ffsdrv_null_erase_suspend,
    ffsdrv_null_erase_resume
};

#elif CONFIG_FLASH_WRITE

#if FLASH_IS_AMD_MULTIBANK

/******************************************************************************
 * AMD Dual/Multi Bank Driver Functions
 ******************************************************************************/

// All erase and write operations are performed atomically (interrupts
// disabled). Otherwise we cannot trust the value of dev.state and we cannot
// determine exactly how many of the command words have already been
// written.

// in ffs_end() when we resume an erasure that was previously suspended, how
// does that affect multiple tasks doing that simultaneously?

void ffsdrv_amd_write_end(void);
void ffsdrv_amd_erase_end(void);

void ffsdrv_amd_write_halfword(volatile uint16 *addr, uint16 value)
{
    volatile uint16 *flash = (volatile uint16 *)dev.base;
    uint32 cpsr;

    tlw(led_on(LED_WRITE));
    ttw(ttr(TTrDrvWrite, "wh(%x,%x)" NL, addr, value));

    dev.addr = addr;
    dev.data = value;

    if (~*addr & value) {
        ttw(ttr(TTrFatal, "wh(%x,%x->%x) fatal" NL, addr, *addr, value));
        return;
    }

    cpsr = int_disable();
    tlw(led_toggle(LED_WRITE_SUSPEND));
    dev.state = DEV_WRITE;
    flash[0x555] = 0xAA; // unlock cycle 1
    flash[0x2AA] = 0x55; // unlock cycle 2
    flash[0x555] = 0xA0;
    *addr         = value;
    int_enable(cpsr);
    tlw(led_toggle(LED_WRITE_SUSPEND));

    ffsdrv_amd_write_end();
}

#if 0
/* duplicates ffsdrv_generic_write */
void ffsdrv_amd_write(void *dst, const void *src, uint16 size)
{
    uint8 *mydst = dst;
    const uint8 *mysrc = src;

    if (size > 0)
    {
        if ((unsigned int) mydst & 1) {
            ffsdrv_write_byte(mydst++, *mysrc++);
            size--;
        }
        while (size >= 2) {
            ffsdrv_amd_write_halfword((uint16 *) mydst,
                                      mysrc[0] | (mysrc[1] << 8));
            size -= 2;
            mysrc += 2;
            mydst += 2;
        }
        if (size == 1)
            ffsdrv_write_byte(mydst++, *mysrc++);
    }
}
#endif

void ffsdrv_amd_write_end(void)
{
    while ((*dev.addr ^ dev.data) & 0x80)
        tlw(led_toggle(LED_WRITE_SUSPEND));

    dev.state = DEV_READ;

    tlw(led_off(LED_WRITE));
}

void ffsdrv_amd_erase(uint8 block)
{
    volatile uint16 *flash = (volatile uint16 *)dev.base;
    uint32 cpsr;

    tlw(led_on(LED_ERASE));
    ttw(ttr(TTrDrvErase, "e(%d)" NL, block));

    dev.addr = (uint16 *) block2addr(block);

    cpsr = int_disable();
    dev.state = DEV_ERASE;
    flash[0x555] = 0xAA; // unlock cycle 1
    flash[0x2AA] = 0x55; // unlock cycle 2
    flash[0x555] = 0x80; 
    flash[0x555] = 0xAA; // unlock cycle 1
    flash[0x2AA] = 0x55; // unlock cycle 2
    *dev.addr = 0x30; // AMD erase sector command
    int_enable(cpsr);

    ffsdrv_amd_erase_end();
}

void ffsdrv_amd_erase_end(void)
{
    while ((*dev.addr & 0x80) == 0)
        ;

    dev.state = DEV_READ;

    tlw(led_off(LED_ERASE));
}

void ffsdrv_amd_erase_suspend(void)
{
    uint32 cpsr;

    tlw(led_on(LED_ERASE_SUSPEND));
    ttw(str(TTrDrvErase, "es" NL));

    // if erase has finished then all is ok
    if (*dev.addr & 0x80) {
        ffsdrv_amd_erase_end();
        tlw(led_off(LED_ERASE_SUSPEND));
        return;
    }

    // NOTEME: As there is no way to be absolutely certain that erase
    // doesn't finish between last poll and the following erase suspend
    // command, we assume that the erase suspend is safe even though the
    // erase IS actually already finished.

    cpsr = int_disable();
    dev.state = DEV_ERASE_SUSPEND;
    *dev.addr = 0xB0;

    // Wait for erase suspend to finish
    while ((*dev.addr & 0x80) == 0)
        ;

    int_enable(cpsr);
}

void ffsdrv_amd_erase_resume(void)
{
    uint32 cpsr;

    ttw(str(TTrDrvErase, "er" NL));

    // NOTEME: See note in erase_suspend()... We assume that the erase
    // resume is safe even though the erase IS actually already finished.
    cpsr = int_disable();
    dev.state = DEV_ERASE;
    *dev.addr = 0x30;
    int_enable(cpsr);

    tlw(led_off(LED_ERASE_SUSPEND));
}

const struct ffsdrv_s ffsdrv = {
    ffsdrv_null_init,
    ffsdrv_amd_erase,
    ffsdrv_amd_write_halfword,
    ffsdrv_generic_write,
    ffsdrv_amd_write_end,
    ffsdrv_amd_erase_suspend,
    ffsdrv_amd_erase_resume
};

#elif FLASH_IS_INTEL_ONEBANK

extern int  ffsdrv_ram_intel_sb_init(void);
extern void ffsdrv_ram_intel_sb_write_halfword(volatile uint16 *addr,
						uint16 value);
extern void ffsdrv_ram_intel_sb_erase(uint8 block);

const struct ffsdrv_s ffsdrv = {
    ffsdrv_ram_intel_sb_init,
    ffsdrv_ram_intel_sb_erase,
    ffsdrv_ram_intel_sb_write_halfword,
    ffsdrv_generic_write,
    ffsdrv_null_write_end,
    ffsdrv_null_erase_suspend,
    ffsdrv_null_erase_resume
};

#else

#error "Flash hardware type unknown"

#endif	/* flash hardware type */

#else

/*
 * This part will get compiled if we have real FFS (FFS_IN_RAM=0),
 * but not allowed to write to flash (CONFIG_FLASH_WRITE=0).
 */

const struct ffsdrv_s ffsdrv = {
    ffsdrv_null_init,
    ffsdrv_null_erase,
    ffsdrv_null_write_halfword,
    ffsdrv_null_write,
    ffsdrv_null_write_end,
    ffsdrv_null_erase_suspend,
    ffsdrv_null_erase_resume
};

#endif


/******************************************************************************
 * Initialization
 *
 * Significantly simplified in FreeCalypso.
 *
 ******************************************************************************/

effs_t ffsdrv_init(void)
{
    int error;
    unsigned i;
    uint32 offset;

    tw(tr(TR_BEGIN, TrDrvInit, "drv_init() {\n"));
    ttw(str(TTrDrvOther, "ffsdrv_init() {" NL));

    dev.state = DEV_READ;
    dev.atomlog2 = FFS_ATOM_LOG2;
    dev.atomsize = 1 << dev.atomlog2;
    dev.atomnotmask = dev.atomsize - 1;

    offset = 0;
    for (i = 0; i < dev.numblocks; i++) {
	block_info[i].offset = offset;
	block_info[i].size_ld = dev.blocksize_ld;
	offset += dev.blocksize;
    }
    dev.binfo = block_info;

    error = ffsdrv.init();

    tw(tr(TR_FUNC, TrDrvInit, "dev.binfo     = 0x%x\n", (unsigned int) dev.binfo));
    tw(tr(TR_FUNC, TrDrvInit, "dev.base      = 0x%x\n", (unsigned int) dev.base));
    tw(tr(TR_FUNC, TrDrvInit, "dev.numblocks = %d\n", dev.numblocks));
    tw(tr(TR_FUNC, TrDrvInit, "dev.blocksize = %d\n", dev.blocksize));
    tw(tr(TR_FUNC, TrDrvInit, "dev.atomlog2/atomsize/atomnotmask = %d/%d/%x\n",
          dev.atomlog2, dev.atomsize, dev.atomnotmask));
    tw(tr(TR_END, TrDrvInit, "} %d\n", error));
    ttw(ttr(TTrDrvOther, "} %d" NL, error));

    return error;
}
