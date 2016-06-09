/*
 * This FFS implementation header file is a FreeCalypso addition.
 * Here we have some preprocessor magic and extern declarations
 * for the FFS-in-RAM configuration which are needed at both
 * "flash driver" and "FFS configuration" abstraction levels.
 */

#include "../../include/config.h"
#if FFS_IN_RAM

#define	RAMFFS_BLKSIZE_BYTES	(1 << RAMFFS_BLKSIZE_LOG2)
#define	RAMFFS_TOTAL_SIZE	(RAMFFS_BLKSIZE_BYTES * RAMFFS_NBLOCKS)

extern char _RAMFFS_area[RAMFFS_TOTAL_SIZE];

#endif	/* FFS_IN_RAM */
