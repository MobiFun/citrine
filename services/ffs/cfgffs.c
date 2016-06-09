/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * FFS configuration
 *
 * $Id: cfgffs.c 1.27 Fri, 19 Dec 2003 12:00:13 +0100 tsj $
 *
 ******************************************************************************/

#include "../../include/config.h"
#include "ffs.h"
#include "drv.h"
#include "ramffs.h"
#include <string.h>

#if FFS_IN_RAM

struct dev_s dev = {
    .base	   = _RAMFFS_area,
    .manufact	   = MANUFACT_RAM,
    .blocksize	   = RAMFFS_BLKSIZE_BYTES,
    .blocksize_ld  = RAMFFS_BLKSIZE_LOG2,
    .driver	   = FFS_DRIVER_RAM,
    .numblocks	   = RAMFFS_NBLOCKS,
};

#elif CONFIG_MOKOFFS

struct dev_s dev = {
    .base	   = (char *) 0x380000,
    .manufact	   = MANUFACT_SAMSUNG,
    .device	   = 0x22A0,
    .blocksize	   = 0x10000,
    .blocksize_ld  = 16,
    .driver	   = FFS_DRIVER_AMD,
    .numblocks	   = 7,
};

#elif CONFIG_AFTERMARKET_FFS

/* see ../../cfgmagic/feature.aftermarket-ffs */

#if CONFIG_TARGET_COMPAL

struct dev_s dev = {
    .base	   = (char *) (CONFIG_AFTERMARKET_FFS_START * 0x10000),
    .manufact	   = MANUFACT_INTEL,
    .blocksize	   = 0x10000,
    .blocksize_ld  = 16,
    .driver	   = FFS_DRIVER_INTEL_SB,
    .numblocks	   = CONFIG_AFTERMARKET_FFS_NBLOCKS,
};

#elif CONFIG_TARGET_PIRELLI || CONFIG_TARGET_FCFAM

struct dev_s dev = {
    .base	   = (char *) FLASH2_BASE_ADDR +
				CONFIG_AFTERMARKET_FFS_START * 0x40000,
    .manufact	   = MANUFACT_AMD,
    .blocksize	   = 0x40000,
    .blocksize_ld  = 18,
    .driver	   = FFS_DRIVER_AMD,
    .numblocks	   = CONFIG_AFTERMARKET_FFS_NBLOCKS,
};

#else

#error "Unknown target for CONFIG_AFTERMARKET_FFS"

#endif

#else

#error "No valid FFS configuration defined"

#endif

/******************************************************************************
 * ffs_is_modify_valid()
 ******************************************************************************/

// This is function to be implemented by the application programmer. It is
// called by ffs when a read-only object is about to be modified or
// removed. It should return zero if the operation should be
// disallowed. Returning non-zero means go ahead.
effs_t ffs_is_modifiable(const char *name)
{
    // default is to allow any modification of read-only objects.

/*
 * TI's original code (conditioned out below) disallowed changing the
 * IMEI of a device.  FreeCalypso follows a different philosophy:
 * 100% user empowerment, hence no artificial obstacles to hacking.
 */
#if 0
    // example of how to disallow modifying a specific object...
    if (strcmp("IMEI", &name[strlen(name) - 4]) == 0)
        return 0;
#endif

    return 1;
}
