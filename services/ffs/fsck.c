/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * FFS file system integrity checking, journalling, init and exit
 *
 * $Id: fsck.c 1.3.1.1.1.33 Thu, 08 Jan 2004 15:05:23 +0100 tsj $
 *
 ******************************************************************************/

#include <string.h>
#include <assert.h>

#include "ffs.h"
#include "core.h"
#include "drv.h"
#include "ffstrace.h"

/******************************************************************************
 * Functions
 ******************************************************************************/

bref_t blocks_fsck(void);
iref_t inodes_fsck(void);

/******************************************************************************
 * Init and Exit
 ******************************************************************************/

effs_t ffs_initialize(void)
{
    bref_t b;
    struct inode_s *ip;
    int i;
    
    tlw(led_set(0));
    tlw(led_on(LED_INIT));
    ttw(str(TTrInit, "initialize {" NL));
    tw(tr(TR_BEGIN, TrFsck, "ffs_initialize() {\n"));

    // default to non-initialized ffs
    fs.root = 0;
    fs.debug[0] = fs.debug[1] = fs.debug[2] = fs.debug[3] = 0;
    fs.testflags = 0;

    tlw(led_on(LED_DRV_INIT));
    fs.initerror = ffsdrv_init();  // read manufacturer and device ID
    tlw(led_off(LED_DRV_INIT));
    if (fs.initerror < 0) {
        tlw(led_off(0));
        tw(tr(TR_END, TrFsck, "} %d\n", fs.initerror));
        ttw(ttr(TTrInit, "} %d" NL, fs.initerror));
        return fs.initerror;
    }

    for (i = 0; i < 2; i++)
    {
        tlw(led_on(LED_BLOCKS_FSCK));
        fs.initerror = EFFS_INVALID;
        fs.initerror = b = blocks_fsck();
        tlw(led_off(LED_BLOCKS_FSCK));
        if (fs.initerror < 0) {
            tlw(led_off(0));
            tw(tr(TR_END, TrFsck, "} %d\n", fs.initerror));
            ttw(ttr(TTrInit, "} %d" NL, fs.initerror));
            return fs.initerror;
        }
        
        tlw(led_on(LED_INODES_FSCK));
        fs.initerror = EFFS_INVALID;
        fs.initerror = inodes_fsck();
        tlw(led_off(LED_INODES_FSCK));
        if (fs.initerror < 0) {
            tlw(led_off(0));
            tw(tr(TR_END, TrFsck, "} %d\n", fs.initerror));
            ttw(ttr(TTrInit, "} %d" NL, fs.initerror));
            return fs.initerror;
        }

        // parse the fs options in the root inode's name
        ip = inode_addr(fs.root);
        fs_params_init(addr2name(offset2addr(location2offset(ip->location))));

        if ((fs.initerror = journal_init(fs.ijournal)) == 0)
            break;    
    }

    // Init all file_descriptors to zero
    memset(fs.fd, 0, sizeof(struct file_descriptor_s) * fs.fd_max);

    // If blocks_fsck() found a block that needs cleaning, we do it, now
    // that all the file system has been initialized.
    if (b > 0) {
        block_clean(b - 1);
	block_free(b - 1);
    }

    statistics_init();

    // In target, we do this before entering the task event loop...
    // Otherwise we would in some cases impose a long reboot delay if we did
    // it here. If we test in target it is nessesary to call
    // blocks_reclaim() anyway because we re-init ffs.

#if (TARGET == 1)         //NOTEME: can this be done in another/better way?
#if (WITH_TFFS == 1)
    blocks_reclaim(); 
#endif
#else
    blocks_reclaim();
#endif
    tlw(led_off(LED_INIT));
    tw(tr(TR_END, TrFsck, "} %d\n", EFFS_OK));
    ttw(str(TTrInit, "} 0" NL));

    return EFFS_OK;
}

void fs_params_init(const char *p)
{
    uint8 opt, digit;
    uint32 n;
    int numdatablocks;

    tw(tr(TR_BEGIN, TrFsck, "fsparams_init('%s') {\n", p));

    // Compiled default values
    fs.filename_max   = FFS_FILENAME_MAX;
    fs.path_depth_max = FFS_PATH_DEPTH_MAX;
    fs.fd_max         = FFS_FD_MAX;
    fs.journal_size   = FFS_JOURNAL_SIZE_IN256THS;
    fs.flags          = 0;
    fs.testflags      = 0;

    // Flag that it not has been changed by an input arg.
    fs.block_files_max = 0;

    // The default lost bytes percentage of a block before it is reclaimed
    // is approx. 90%.
    fs.lost_threshold  = (256 - 256/10);

    // If we only have two blocks, we cannot make any reclaims and thus we
    // have a write-once FFS system.
    fs.blocks_free_min = (dev.numblocks > 2 ? 1 : 0);

    // Don't count free and inodes blocks
    numdatablocks = dev.numblocks - fs.blocks_free_min - 1;

    // Abselute max number of inodes.
    fs.inodes_max = dev.blocksize / sizeof(struct inode_s);
    if (fs.inodes_max > FFS_INODES_MAX)
	fs.inodes_max = FFS_INODES_MAX;

    // MUST be true: objects_max <= inodes_max - block_files_max, this is do
    // to the fact that we always need to have block_files_max number of
    // inodes left when we run a data reclaim.
    fs.objects_max = fs.inodes_max / 2;

    // Find a suitable chunk_size
    if (dev.numblocks*dev.blocksize > 1024*1024)
        fs.chunk_size_max = 8192;
    else
        fs.chunk_size_max = (2048 > (dev.blocksize / 8) 
                             ? (dev.blocksize / 8)
                             : 2048);
    fs.fd_buf_size = fs.chunk_size_max;

    fs.journal_size = fs.journal_size * dev.blocksize / 256;
    if (fs.journal_size < FFS_JOURNAL_SIZE_MIN)
        fs.journal_size = FFS_JOURNAL_SIZE_MIN;

    // Set it just below the same amount as entries in one journal file
    fs.block_files_max = (fs.journal_size / sizeof(struct journal_s) 
                          - FFS_JOURNAL_MARGIN - 2);

    // MUST be true: block_files_max < objects_max / 2. But if we want
    // to reach objects_max must block_files_max >= objects_max / number
    // of datablocks, however a big block_files_max require higher
    // reserved_space.
    if (fs.block_files_max > fs.objects_max / 2)
        fs.block_files_max = fs.objects_max / 2 - 4;

    // Are we able to reach objects_max? If not then lower the number
    if (fs.objects_max > numdatablocks * fs.block_files_max)
        fs.objects_max = numdatablocks * fs.block_files_max + 10;

    // Absolute minimum is RESERVED_LOW the rest is 'workspace' which is
    // needed to have a reasonable performance.
    fs.reserved_space = dev.blocksize / 2 + 
        numdatablocks * dev.blocksize / 16 + RESERVED_LOW; 

    // skip to first char following second slash in name
    n = 0;
    while (*p) {
        if (*p++ == '/') {
            n++;
            if (n == 2)
                break;
        }
    }
    if (n == 2) {
        // while still options to process...
        while (*p) {
            opt = *p++; // save option letter for later
            // collect option value...
            n = 0;
            while ((digit = *p)) {
                if (digit >= '0' && digit <= '9') {
                    n = 10 * n + digit - '0';
                    p++;
                }
                else
                    break;
            }
            switch (opt) {
            case 'b': dev.numblocks = n;       break;
            case 'm': fs.blocks_free_min = n;  break;
            case 'i': fs.inodes_max = n;       break;
            case 'o': fs.objects_max = n;      break;
            case 'n': fs.filename_max = n;     break;
            case 'f': fs.block_files_max = n;  break;
            case 'd': fs.fd_max = n;           break;
            case 's': fs.fd_buf_size = n;      break;
            case 't': fs.lost_threshold = n;   break;
            case 'z': fs.flags = n;            break;
            case 'j': fs.journal_size = n;     break;
            case 'c': fs.chunk_size_max = n;   break;
            case 'r': fs.reserved_space = n;   break;
                // d = &fs.path_depth_max;  // really necessary?
            default:
                break;
            }
        }
    }

    // Now recompute a few parameters based on adjusted values.

    // No journal file thuse no reserved space.
    if (fs.journal_size == 0) {   
	fs.block_files_max = fs.objects_max / 2;
	fs.reserved_space = 0;
	fs.block_files_reserved = 0;
    }

    else {
        // If journal size is less than minimum must it have been changed by an
        // input arg, recalculate.
        if (fs.journal_size < FFS_JOURNAL_SIZE_MIN)
            fs.journal_size = fs.journal_size * dev.blocksize / 256;

        if (fs.reserved_space < RESERVED_LOW)
            fs.reserved_space = fs.reserved_space * dev.blocksize / 256;

	// Only one reserved is needed however we want a margin and set it to 2
	fs.block_files_reserved = 2;
    }

    // Don't count free blocks, inode block, reserved space, block headers
    // and the size of one filename.
    fs.filesize_max = numdatablocks * dev.blocksize - fs.reserved_space - 
	numdatablocks * BHEADER_SIZE - FFS_FILENAME_MAX;

    // Furthermore don't count the overhead from each chunk (alignment) 
    fs.filesize_max -= ((fs.filesize_max / fs.chunk_size_max) * dev.atomsize 
                        + dev.atomsize);

    // NOTEME: chunk_size_min is never used
    fs.chunk_size_min = numdatablocks / fs.objects_max;

    tw(tr(TR_FUNC, TrFsck, "dev.numblocks      = %d\n", dev.numblocks));
    tw(tr(TR_FUNC, TrFsck, "fs.blocks_free_min = %d\n", fs.blocks_free_min));
    tw(tr(TR_FUNC, TrFsck, "fs.inodes_max      = %d\n", fs.inodes_max));
    tw(tr(TR_FUNC, TrFsck, "fs.objects_max     = %d\n", fs.objects_max));
    tw(tr(TR_FUNC, TrFsck, "fs.block_files_max = %d\n", fs.block_files_max));
    tw(tr(TR_FUNC, TrFsck, "fs.block_files_reserved    = %d\n", fs.block_files_reserved));
    tw(tr(TR_FUNC, TrFsck, "fs.chunk_size_max  = %d\n", fs.chunk_size_max));
    tw(tr(TR_FUNC, TrFsck, "fs.filename_max    = %d\n", fs.filename_max));
    tw(tr(TR_FUNC, TrFsck, "fs.lost_threshold  = %d\n", fs.lost_threshold));
    tw(tr(TR_FUNC, TrFsck, "fs.path_depth_max  = %d\n", fs.path_depth_max));
    tw(tr(TR_FUNC, TrFsck, "fs.journal_size    = %d\n", fs.journal_size));
    tw(tr(TR_FUNC, TrFsck, "fs.reserved_space  = %d\n", fs.reserved_space));
    tw(tr(TR_FUNC, TrFsck, "fs.fd_max          = %d\n", fs.fd_max));
    tw(tr(TR_FUNC, TrFsck, "fs.fd_buf_size     = 0x%02x\n", fs.fd_buf_size));
    tw(tr(TR_FUNC, TrFsck, "fs.flags           = 0x%02x\n", fs.flags));
    tw(tr(TR_END,  TrFsck, "}\n"));
}

// TODO: Finish pending commits/writes. 
effs_t ffs_exit(void)
{
    tw(tr(TR_FUNC, TrFsck, "exit() 0\n"));


    return EFFS_OK;
}

#if 0 // Not used in this version
// Purely for core internal use; Read a file.
effs_t file_read_int(const char *path, void *src, int size)
{
    if (fs.initerror != EFFS_OK)
        return fs.initerror;

    return object_read(path, src, size, 0);
}

// Purely for core internal use; Update a file.
effs_t file_update(const char *path, void *src, int size)
{
    char *name;
    iref_t i, dir;
    
    if (fs.initerror != EFFS_OK)
        return fs.initerror;

    if ((i = object_lookup(path, &name, &dir)) < 0)
        return i;
    
    journal_begin(i);

    if ((i = object_create(name, src, size, -dir)) < 0)
        return i;

    journal_end(0);

    return EFFS_OK;
}
#endif

/******************************************************************************
 * blocks_fsck()
 ******************************************************************************/

blocksize_t block_used(bref_t b)
{
    blocksize_t used;
    uint32 *p, *q;

    tlw(led_toggle(LED_BLOCKS_FSCK));

    // We search backwards through block to find the last used byte and
    // thus the total number of used bytes. Note that this code depends
    // on the fact that an erased flash location is 0xFF!
    p = (uint32 *) offset2addr(dev.binfo[b].offset);
    for (q = p + dev.blocksize/4 - 4; q > p; q -= 4) {
        if ( ~(q[0] & q[1] & q[2] & q[3]) )
            break;
    }

    if ( ~(q[0] & q[1] & q[2] & q[3]) )
        q += 4;
    used = atomalign((char *) q - (char *) p);

    tw(tr(TR_FUNC, TrFsckLow, "ffs_block_used(%d) %d\n", b, used));

    return used;
}


age_t age_distance(age_t x, age_t y)
{
    age_t a = x - y;
    
    if (a > 0x8000)
        a = -a;

    tw(tr(TR_FUNC, TrFsckLow, "age_distance(%d, %d) %d\n", x, y, a));
    
    return a;
}

// For each ffs block, we initialise the basic bstat array information,
// namely the number of used bytes. Also, we locate the inodes block and if
// a previous operation was interrupted by a powerfail, we clean it up.
//
// We return EFFS_OK if all is fine. If a positive integer is returned, it
// denotes a block that needs to be cleaned by block_clean() once FFS
// has been properly intialized (we actually return the block number + 1
// because otherwise it would clash with EFFS_OK return code). If no inodes
// block is found or another error occurs, we return the error code.
bref_t blocks_fsck(void)
{
    bref_t b, b_to_clean, b_inode_lost;
    int age_valid;
    age_t age_min, age_max, age_dist, age_dist_min, age_dist_max;
    struct block_header_s *bhp;
    struct block_header_old_s *obhp;

    ttw(str(TTrInitLow, "blocks_fsck {" NL));
    tw(tr(TR_BEGIN, TrFsck, "blocks_fsck() {\n"));

    // initialize ages to the illegal/unset value
    age_min = age_max = age_dist = 0;

    fs.format    =  0;
    fs.inodes    = -1;
    fs.newinodes = -1;
    b_inode_lost = -1;
    b_to_clean   = EFFS_OK;

    for (b = 0; b < dev.numblocks; b++)
    {
        tlw(led_toggle(LED_DRV_INIT));

        // read block flags from flash
        bhp = (struct block_header_s *) offset2addr(dev.binfo[b].offset);
        obhp = (struct block_header_old_s *) bhp;

        bstat[b].used    = dev.blocksize;
        bstat[b].lost    = bstat[b].used;
        bstat[b].flags   = bhp->flags;
        bstat[b].objects = 0;

        age_valid = 0;

        if (bhp->magic_low  != BLOCK_MAGIC_LOW ||
            bhp->magic_high != BLOCK_MAGIC_HIGH) {
            // The block magic as bad! It *could* be because the flash
            // memory map is incorrect or because another application has
            // spuriously written to the flash or ... who knows what. First
            // we check to see if the reason is that we are dealing with a
            // (really) old ffs format version.
            if (obhp->magic_low == OLD_BLOCK_MAGIC_LOW &&
                obhp->magic_high == OLD_FFS_FORMAT_VERSION) {
                tw(tr(TR_FUNC, TrFsck, "OLD     "));
                fs.format = obhp->magic_high;
                // We simulate that all the blocks are data blocks, in order
                // to have some well-defined state that preformat() can work
                // on. Later we will return EFFS_BADFORMAT and otherwise
                // leave everything as it is, *without* modifying anything!
                bstat[b].flags = BF_IS_DATA;
            }
            else {
                // Quickly test if block is in empty state. We do not make a
                // full check with block_used() because that takes too
                // long --- we let preformat() do that.
                if (bhp->magic_low  == FLASH_NULL16 &&
                    bhp->magic_high == FLASH_NULL16 &&
                    bhp->age        == FLASH_NULL16 &&
                    bhp->version    == FLASH_NULL16 &&
                    bhp->flags      == FLASH_NULL16)
                {
                    bstat[b].used  = 0;
                    bstat[b].lost  = 0;
                    bstat[b].flags = BF_IS_EMPTY;
                    tw(tr(TR_FUNC, TrFsck, "EMPTY     "));
                }
                else {
                    // If the block is not free, it is probably corrupted.
                    // Thus we reset its age and free it.
                    tw(tr(TR_FUNC, TrFsck, "magic = 0x%08x\n",
                          bhp->magic_low | (bhp->magic_high << 16)));
                    ffsdrv.write_halfword(&bhp->age, 0);
                    block_free(b);
                    tw(tr(TR_FUNC, TrFsck, "BAD       "));
                }
            }
        }
        else {
            fs.format = bhp->version;
            age_valid = 1;

            if (!is_block(b, BF_IS_FREE)) {
                bstat[b].used = block_used(b);
                bstat[b].lost = bstat[b].used - BHEADER_SIZE; 
            }

            if (is_block(b, BF_IS_FREE)) {
                // The only case where we do not call block_used() is
                // when the block is truly free.
                bstat[b].used = 0;
                bstat[b].lost = 0;
                tw(tr(TR_FUNC, TrFsck, "FREE      "));
                ttw(ttr(TTrInitLow, "FREE" NL));

            }
            else if (is_block(b, BF_IS_DATA)) {
                tw(tr(TR_FUNC, TrFsck, "DATA      "));
                ttw(ttr(TTrInitLow, "DATA" NL)); 
            }
            else if (is_block(b, BF_IS_CLEANING)) {
                // Here we schedule a block_clean(). Note that we can
                // and do not execute the block cleaning now, as the info
                // that block_clean() needs is not at all ready at this
                // point in the initialization. So we set a flag and then
                // clean the block at the end of ffs_initialize()
                tw(tr(TR_FUNC, TrFsck, "CLEANING  "));
                ttw(ttr(TTrInitLow, "CLEANING" NL)); 
                b_to_clean = b + 1;
            }
            else if (is_block(b, BF_IS_COPYING)) {
                tw(tr(TR_FUNC, TrFsck, "COPYING   "));
                ttw(ttr(TTrInitLow, "COPYING" NL)); 
                fs.newinodes = b;
            }
            else if (is_block(b, BF_IS_INODES)) {
                tw(tr(TR_FUNC, TrFsck, "INODES    "));
                ttw(ttr(TTrInitLow, "INODES" NL)); 
                    fs.inodes = b;
            }
            else if (is_block(b, BF_IS_INODES_LOST)) {
                tw(tr(TR_FUNC, TrFsck, "INODESLOST"));
                ttw(ttr(TTrInitLow, "INODESLOST" NL)); 
                b_inode_lost = b;
            }
            else {
                block_free(b);
                tw(tr(TR_FUNC, TrFsck, "INVALID   "));
                ttw(ttr(TTrInitLow, "INVALID" NL)); 
            }
        }

        tw(tr(TR_NULL, TrFsck, " %2d: (0x%05x) %02x, used = %6d\n",
              b, dev.binfo[b].offset, bstat[b].flags & 0xFF, bstat[b].used));

        if (age_valid) {
            if (age_min == 0) {
                // Initialize minimum and maximum block ages
                age_min = age_max = bhp->age;
                tw(tr(TR_FUNC, TrFsckLow, "age_min/max = %d\n", age_min));
            }
            else {
                age_dist_min = age_distance(bhp->age, age_min);
                age_dist_max = age_distance(bhp->age, age_max);
                if (age_dist_min > age_dist ||
                    age_dist_max > age_dist) {
                    if (age_dist_max > age_dist_min) {
                        age_dist = age_dist_max;
                        age_min  = bhp->age;
                        tw(tr(TR_FUNC, TrFsckLow, "age_min = %d (dist = %d)\n",
                              age_min, age_dist));
                    }
                    else {
                        age_dist = age_dist_min;
                        age_max  = bhp->age;
                        tw(tr(TR_FUNC, TrFsckLow, "age_max = %d (dist = %d)\n",
                              age_max, age_dist));
                    }
                }
            }
        }
    }
    tlw(led_off(LED_DRV_INIT));
    tw(tr(TR_FUNC, TrFsck, "age min, max, max-min = %d, %d, %d\n",
          age_min, age_max, (uint16) (age_max-age_min)));
    // If age_max is untouched is is because all blocks were in the 'Empty'
    // state. In this case we let the age be as it is (0xFFFF).
    if (age_max == 0)
        age_max = age_min = BLOCK_AGE_MAX;

    // Handle age wrap around thus ensuring fs.age_max is set correctly. We
    // have to type-cast the whole computation, otherwise it will be
    // incorrect.
    if ((age_t) (age_max - age_min) > 0x8000) {
        age_dist = age_max;
        age_max  = age_min;
        age_min  = age_dist;
    }

    // save maximum age found for the case of a bad block that is going to
    // be reclaimed later on by blocks_reclaim()
    fs.age_max = age_max;

    tw(tr(TR_FUNC, TrFsck, "fs.format = 0x%04x\n", fs.format));
    tw(tr(TR_FUNC, TrFsck, "fs.inodes, newinodes = %d, %d\n",
          fs.inodes, fs.newinodes));
    ttw(ttr(TTrInit, "fs.inodes, newinodes = %d, %d" NL, 
            fs.inodes, fs.newinodes));
    tw(tr(TR_FUNC, TrFsck, "age min, max = %d, %d\n", age_min, age_max));

    // If any blocks were in the EMPTY state, now is the time to bring them
    // into the FREE state. Note that we must only do this *after*
    // fs.age_max has been initialized.
    for (b = 0; b < dev.numblocks; b++) {
        if (is_block(b, BF_IS_EMPTY)) {
            if ((bstat[b].used = block_used(b)) == 0)
                block_preformat(b, 0);
            else
                block_free(b);
        }
    }

    if (fs.inodes >= 0) {  
        // The 'old' inode block is still valid thus we keep it.
	if (fs.newinodes >= 0)
            // The copying of inodes to the new block was not finished thus
            // we free the block
	    block_free(fs.newinodes);   
	inodes_set(fs.inodes);
    }
    else {                                 
	// Copying must have been finished
	if (fs.newinodes >= 0 && b_inode_lost >= 0) {
            // The inode reclaim did finish but currently there is no valid
            // inode block thus the operation must be finished by committing
            // the new block as the valid inode block.
	    fs.inodes = b_inode_lost;
	    block_commit();
	}
	else {
            // No old or new Inode block!
	    tw(tr(TR_END, TrFsck, "} %d\n", EFFS_NOFORMAT));
	    ttw(ttr(TTrInitLow, "} %d" NL, EFFS_NOFORMAT));
	    return EFFS_NOFORMAT;
	}
    }

    if ((fs.format >> 8) != (FFS_FORMAT_VERSION >> 8)) {
        tw(tr(TR_END, TrFsck, "} %d\n", EFFS_BADFORMAT));
        ttw(ttr(TTrInitLow, "} %d" NL, EFFS_BADFORMAT));
        return EFFS_BADFORMAT;
    }

    // FIXME: Insert age sanity check; age distance must not be too big (> 2
    // * FFS_AGE_DISTANCE)?

    tw(tr(TR_END, TrFsck, "} %d\n", b_to_clean));
    ttw(ttr(TTrInitLow, "} %d" NL, b_to_clean));

    return b_to_clean;
}

// Set fs.inodes and fs.inodes_addr
void inodes_set(iref_t i)
{
    fs.inodes = i;
    fs.inodes_addr = (struct inode_s *)
        (offset2addr(dev.binfo[fs.inodes].offset)
         + dev.atomsize - sizeof(struct inode_s));
}


/******************************************************************************
 * inodes_fsck()
 ******************************************************************************/

// Now for each inode in the inodes block, update the bstat array
// information: free, used, objects. Also, locate the root inode. We could
// optimize this a little, because bstat[binodes].used gives an inidication
// of how many inodes are actually present in the system.
iref_t inodes_fsck(void)
{
    iref_t i;
    struct inode_s *ip;
    char *addr;
    bref_t block;

    ttw(str(TTrInitLow, "inodes_fsck {" NL));
    tw(tr(TR_BEGIN, TrFsck, "inodes_fsck() {\n"));
    tw(tr(TR_FUNC, TrFsck, "inodes in block %d:\n", fs.inodes));

    // the fields of the bstat entry for the inodes have the meaning:
    // used = total number of used inodes (valid, erased, invalid)
    // lost = total number of lost inodes (erased, invalid)
    // objects = index of first free inode (used by inode_alloc())

    fs.root = 0;     // default to root inode not found
    fs.ijournal = 0; // default to journal file inode not found
    bstat[fs.inodes].objects = 1;
    bstat[fs.inodes].used = 0;
    bstat[fs.inodes].lost = 0;
    fs.sequence = 0; // just for debug (fun)

    // we must set some default value for this, so we set it to max possible!
    fs.inodes_max = dev.blocksize / sizeof(struct inode_s);

    ip = inode_addr(1);
    tw(tr(TR_FUNC, TrFsck, "  i    addr  cld sib  seq upd  flag size name\n"));
    for (i = 1; i < fs.inodes_max; i++, ip++)
    {
        // just for debug (fun)
        if (ip->sequence > fs.sequence)
            fs.sequence = ip->sequence;
                
        // compute block index and total data space occupied
        block = offset2block(location2offset(ip->location));

        // Only scan used inodes. blocks_fsck() accounted all used space as
        // also being lost space, so now we subtract from the lost space,
        // the space used by valid objects
        if (ip->location != FLASH_NULL32)
        {
            bstat[fs.inodes].used++;

            tw(tr(TR_FUNC, TrFsck, "%3d 0x%05X  %3d %3d %4d %3d  %s%s%s%s%s%s %6d %s\n",
                  i,
                  location2offset(ip->location),
                  ip->child, ip->sibling,
                  ip->sequence, ip->updates,
                  is_object(ip, OT_DIR)     ? "d" : "",
                  is_object(ip, OT_LINK)    ? "l" : "",
                  is_object(ip, OT_FILE)    ? "f" : "",
                  is_object(ip, OT_SEGMENT) ? "s" : "",
                  is_object(ip, OT_ERASED)  ? " " : "",
                  IS_BIT_SET(ip->flags, OF_READONLY) && !is_object(ip, OT_ERASED) ?
                  "r" : " ",
                  ip->size,
                  // Erased chunks do not have any name so we can not trace erased objects!
                  (ip->size && !is_object(ip, OT_SEGMENT) && !is_object(ip, OT_ERASED) ? 
                   addr2name(offset2addr(location2offset(ip->location))) : "")
                  ));
            
            if (is_object_valid(ip)) {
                // This inode is valid, so we account the data space as used
                // and the inode as used too.
                bstat[block].lost -= ip->size;
                bstat[block].objects++;
                // test if this is the root inode. store index if it is.
                if (!is_object(ip, OT_SEGMENT)) {
                    addr = addr2name(offset2addr(location2offset(ip->location)));
                    if (*addr == '/')
                        fs.root = i;
                    else if (*addr == '.' &&
                             ffs_strcmp(addr, FFS_JOURNAL_NAME) == 0) {
                        fs.ijournal = i;
                    }
                }
            }
            else if (is_object(ip, OT_ERASED)) {
                // this inode's data is deleted, so we account the data
                // space as used and lost and the inode as lost too.
                bstat[fs.inodes].lost++;
            }
            else {
                // This is an invalid object, so we account the data space
                // as used and lost and the inode as lost too. NOTEME: error
                // what should we do? Perhaps we should record semi-lost
                // inodes? Can we safely account for it here if this is an
                // object to be recovered because another inode.copied is
                // referring to this?  Will used/lost etc. be updated
                // correctly then?
                bstat[fs.inodes].lost++;
                tw(tr(TR_NULL, TrFsck, "(invalid = %d)\n", ip->flags & OT_MASK));
            }
        }
    }
    ttw(ttr(TTrInit, "fs.root=%d, journal=%d" NL, fs.root, fs.ijournal));
    tw(tr(TR_END, TrFsck, "} used: %d, lost: %d, root: %d, journal: %d\n",
       bstat[fs.inodes].used, bstat[fs.inodes].lost, fs.root, fs.ijournal));

    fs.sequence++;
    
    tw(tr_bstat());

    if (fs.root == 0) {
        ttw(ttr(TTrInitLow, "} %d" NL, EFFS_NOFORMAT));
        return EFFS_NOFORMAT;
    }

    ttw(str(TTrInitLow, "} 0" NL));

    return EFFS_OK;
}


/******************************************************************************
 * Preformat and format
 ******************************************************************************/

// Prepare all blocks for fs_format(). Because ffs_is_formattable() has
// already been called prior to this function, we know that no sector erase
// is in progress! The blocks are prepared by putting them into the 'Free'
// state.
effs_t fs_preformat(void)
{
    bref_t b;

    ttw(str(TTrFormat, "preformat {" NL));
    tw(tr(TR_BEGIN, TrFormat, "fs_preformat() {\n"));

    // Mark ffs as being non-formatted from now on.
    fs.root = 0;

    // We must initialize bstat[fs.inodes].used and inodes_high, such that
    // inodes_reclaim() isn't triggered in reclaim() on the following
    // fs_format().
    inodes_set(0);
    bstat[fs.inodes].used    = 0;
    bstat[fs.inodes].lost    = 0;
    bstat[fs.inodes].objects = 0;

    // While format is in progress, we make FFS inaccessible to other
    // functions...
    fs.initerror = EFFS_NOFORMAT;

    if (dev.manufact == 0) {
        b = EFFS_NODEVICE;
    }
    else {
        for (b = 0; b < dev.numblocks; b++) {
            if (is_block(b, BF_IS_EMPTY)) {
                if ((bstat[b].used = block_used(b)) == 0)
                    block_preformat(b, 0);
                else
                    block_free(b);
            }
            else if (!is_block(b, BF_IS_FREE)) {
                block_free(b);
            }
        }
        b = EFFS_OK;
    }

    tw(tr(TR_END, TrFormat, "} %d\n", b));
    ttw(ttr(TTrFormat, "} %d" NL, b));

    return b;
}

// Preformat a single block thus taking it from the 'Empty' state into
// 'Free' state.
void block_preformat(bref_t b, age_t age)
{
    int set_age_max;
    struct block_header_s *bhp =
        (struct block_header_s *) offset2addr(dev.binfo[b].offset);

    tw(tr(TR_BEGIN, TrFormat, "fs_block_preformat(%d, %d)\n", b, age));

    if (age == 0) {
        age = fs.age_max;
    }
    else {
        // We schedule an update of fs.age_max. Due to proper handling of
        // age wrap-around, we can not actually set it now.
        set_age_max = (age == fs.age_max);
        age++;
        if (age == 0)
            age++;
        if (set_age_max) {
            fs.age_max = age;
            tw(tr(TR_FUNC, TrFormat, "new fs.age_max = %d\n", fs.age_max));
        }
    }

    ffsdrv.write_halfword(&bhp->age,        age);
    ffsdrv.write_halfword(&bhp->version,    FFS_FORMAT_VERSION);
    ffsdrv.write_halfword(&bhp->magic_low,  BLOCK_MAGIC_LOW);
    ffsdrv.write_halfword(&bhp->magic_high, BLOCK_MAGIC_HIGH);

    bstat[b].flags = BF_IS_EMPTY;
    bstat[b].used = 0;
    bstat[b].lost = 0;
    bstat[b].objects = 0;

    block_flags_write(b, BF_FREE);

    tw(tr(TR_END, TrFormat, ""));
}

// After preformat() has erased two blocks, this function can be called to
// initialize ffs by writing fs data and metadata.  Note that ffs_begin() is
// *not* called before this function in ffs.c. Otherwise we would never
// enter this function because fs.root is zero. NOTEME: this is also a bug
// as this means we risk that this operation is started while an erase (or a
// write) is in progress! How the flash device reacts to this is currently
// unknown.
effs_t fs_format(const char *name)
{
    bref_t i, b;

    ttw(str(TTrFormat, "format {" NL));
    tw(tr(TR_BEGIN, TrFormat, "fs_format('%s') {\n", name));

    // Initialize file system parameters. It should be safe to change these
    // now, as the format cannot fail at this point onwards.
    fs_params_init(name);

    // Make the first block be the inodes block
    if ((fs.inodes = block_alloc(1, BF_COPYING)) < 0)
        return EFFS_AGAIN;
    block_flags_write(fs.inodes, BF_INODES);
    inodes_set(fs.inodes);

    // Make all block as data blocks except from the free_min and inode block
    for (i = 0; i < dev.numblocks - fs.blocks_free_min - 1; i++) 
        if ((b = block_alloc(0, BF_DATA)) < 0)
            return EFFS_AGAIN;

    // Restart object sequencing (debug feature only)
    fs.sequence = 0;

    // Create root directory
    journal_begin(0);
    if ((fs.root = object_create(name, 0, 0, 0)) < 0) {
        tw(tr(TR_END, TrFormat, "} %d\n", fs.root));
        return fs.root;
    }
    journal_commit(OT_DIR);

    if ((fs.ijournal = journal_create(0)) < 0) {
        tw(tr(TR_END, TrFormat, "} %d\n", fs.ijournal));
        return fs.ijournal;
    }

    fs.initerror = ffs_initialize();

    ttw(ttr(TTrFormat, "} %d" NL, fs.initerror));
    tw(tr(TR_END, TrFormat, "} %d\n", fs.initerror));

    return fs.initerror;
}

// Check if we are ready to preformat (flag = 0) or format (flag = 1)
//
// For a format, we must first ensure no blocks are valid e.g. a preformat
// has already been run. Next, we must ensure we have preformatted all
// blocks e.g. all blocks are in the 'Free' state. This is actually the same
// thing but it sure helps the user because it yields a more precise error
// code when the format fails. In future we might be able to start a format
// when only two blocks have been preformatted, but this is harder because
// we have to make sure not to read from the physical sector that we are
// erasing, and this is exactly what ffs_ffs_initialize() currently does
// (when it is called at the end of format()).
//
// For a preformat, we must ensure an erase is not in progress (because we
// don't know how the device will react to a new erase when an erase is
// currently suspended).
effs_t is_formattable(int8 flag)
{
    bref_t i, free, valid;
    effs_t error = EFFS_OK;

    tw(tr(TR_FUNC, TrFormat, "is_formattable() "));

    // Count the number of valid and free blocks. These numbers will later
    // be checked to see if we are really ready for a (pre)format(). Note
    // that we *only* read block flags from the bstat[] array. We must not
    // read directly from the flash sectors because an erase might be in
    // progress!
    for (i = 0, free = 0, valid = 0; i < dev.numblocks; i++) {
        if (is_block(i, BF_IS_DATA) || is_block(i, BF_IS_INODES))
            valid++;
        if (is_block(i, BF_IS_FREE))
            free++;
    }
    if (flag == 0) {
        // In the case of a preformat, ensure an erase is not in
        // progress (because we don't know how the device will react to a new
        // erase when an erase is currently suspended).
        if (dev.state == DEV_ERASE || dev.state == DEV_ERASE_SUSPEND) {
            tw(tr(TR_NULL, TrFormat, "(%d)\n", EFFS_AGAIN));
            return EFFS_AGAIN;
        }
    }
    else {
        if (valid > 0)
            // Ensure we have preformatted prior to a format.
            error = EFFS_NOPREFORMAT;
        else if (free < dev.numblocks)
            // Ensure all blocks are free before a format(). If not, a
            // preformat() is currently in progress.
            error = EFFS_AGAIN;
    }

    tw(tr(TR_NULL, TrFormat, "(%d)\n", error));
    return error;
}


/******************************************************************************
 * Journalling
 ******************************************************************************/

// The following matrix illustrates how the members of an inode change for
// the various (journalled) operations:
//
//          | flags | size | loc | child | siblg | dir | oldi | updates
// ---------+-------+------+-----+-------+-------+-----+------+--------
// create   | new   | new  | new | -     | -     | ins | n/a  | 0
// fupdate  | o     | new  | new | o     | -     | ins | del  | old+1
// relocate | o     | o    | new | o     | -     | ins | del  | old+1
// fctrl    | new   | o    | o   | o     | -     | ins | del  | old+1
// remove   | n/a   | n/a  | n/a | n/a   | n/a   | n/a | del  | n/a  
//
//  -  = leave empty (0xFFFF)
// ins = insert/append into directory
// o   = old value
//
// We don't have to store child member in the journal entry because either
// it is EMPTY (fs.journal.oldi = 0) or it is retrieved from oldip->child.

// NOTEME: With journalling implemented, object_relocate might be able just
// to make a simple data copy!

// block_clean() is safe (without journalling), now that only ip->size is
// set to zero.

// Begin a new journal. Either a fresh object create (oldi == 0) or an
// update of an existing object (oldi == iref of old object)
void journal_begin(iref_t oldi)
{
    tw(tr(TR_FUNC, TrJournal, "journal_begin(%d)\n", oldi));

    fs.journal.i = 0;
    fs.journal.state = JOURNAL_IS_EMPTY;
    fs.journal.repli = 0;
    fs.link_child = 1;   //Default link child in journal_commit()

    if (oldi == 0) {
        fs.journal.flags    = 0xFF;
        fs.journal.diri     = 0;
        fs.journal.oldi     = 0;
        fs.journal.location = 0;
        fs.journal.size     = 0;
    }
    else {
        struct inode_s *oldip = inode_addr(oldi);
        fs.journal.flags    = oldip->flags;
        fs.journal.diri     = oldi;
        fs.journal.oldi     = oldi;
        fs.journal.location = oldip->location;
        fs.journal.size     = oldip->size;
    }
}

// NOTEME: We have compressed the macro code because it will NOT compile on
// Unix otherwise. So until we find out why, we use this as a work-around.
#if (FFS_TEST == 1)
#define JOURNAL_TEST(testcase, text) if (fs.testflags == testcase) { tw(tr(TR_END, TrJournal, "} (" text ")\n")); return; }
#else
#define JOURNAL_TEST(testcase, text)
#endif

// NOTEME: Should we empty journal file when we are anyway relocating it in
// data_reclaim()?
void journal_end(uint8 type)
{
    struct inode_s *ip = inode_addr(fs.ijournal);
    struct journal_s *addr = (struct journal_s *)
        offset2addr(location2offset(ip->location) + fs.journal_pos);
    
    tw(tr(TR_BEGIN, TrJournal, "journal_end(0x%x) {\n", type));
    tw(tr(TR_FUNC, TrJournal, "journal_pos = 0x%04x (%d)\n", fs.journal_pos,
          (fs.journal_pos - JOURNAL_POS_INITIAL) / sizeof(struct journal_s)));
    
    // If this is a create, set the object type
    if (type != 0 && fs.journal.oldi == 0)
        fs.journal.flags = (fs.journal.flags & OF_MASK) | type;

    // If there is no journal file, we can do without it, although we
    // certainly don't like it!
    if (fs.ijournal == 0) {
        journal_commit(0); 
        tw(tr(TR_END, TrJournal, "} No jounal file\n"));
        return;
    }
    
    JOURNAL_TEST(JOURNAL_TEST_EMPTY, "Oops in JOURNAL_IS_EMPTY");
        
    // Write RAM journal to journal file.
    if (fs.journal.state == (uint8) JOURNAL_IS_EMPTY) {
        fs.journal.state = JOURNAL_IS_WRITING;
        ffsdrv.write(addr, &fs.journal, sizeof(fs.journal));
    }
    
    JOURNAL_TEST(JOURNAL_TEST_WRITING, "Oops in JOURNAL_IS_WRITING");
        
    // Advance journal file's state
    if (fs.journal.state == (uint8) JOURNAL_IS_WRITING) {
        fs.journal.state = JOURNAL_IS_READY;
        ffsdrv_write_byte(&addr->state, fs.journal.state);
    }
    
    JOURNAL_TEST(JOURNAL_TEST_READY, "Oops in JOURNAL_IS_READY");
    
    journal_commit(0);
    
    JOURNAL_TEST(JOURNAL_TEST_COMMITTING, "Oops in JOURNAL_TEST_COMMITTING");
    JOURNAL_TEST(JOURNAL_TEST_COMMITTED, "Oops in JOURNAL_COMMITTED");

    // Advance journal file's state
    ffsdrv_write_byte(&addr->state, JOURNAL_IS_DONE);

    JOURNAL_TEST(JOURNAL_TEST_DONE, "Oops in JOURNAL_IS_DONE");

    // Advance journal
    fs.journal_pos += sizeof(struct journal_s);

    // Unless we are currently relocating the journal file itself, check if
    // journal file is near full and relocate it if it is.
    if (fs.journal_pos >= fs.journal_size - FFS_JOURNAL_MARGIN * 
	sizeof(struct journal_s) && fs.journal.oldi != fs.ijournal) {
        tw(tr(TR_FUNC, TrJournal, "Journal file (near) full!\n"));
        journal_create(fs.ijournal);
    }

    // Check if we have just committed the journal file itself
    if (fs.journal.oldi == fs.ijournal) {
        fs.journal_pos = JOURNAL_POS_INITIAL;
        fs.ijournal = fs.journal.i;
        tw(tr(TR_FUNC, TrJournal, "Journal file re-created, fs.ijournal = %d\n",
              fs.ijournal));
    }
    tw(tr(TR_END, TrJournal, "}\n"));
}

// Write contents of fs.journal to FFS meta data (inodes). Note that we do
// NOT traverse ip->copied as we used to do in the old
// object_update_commit(). Also, we do not check if object has been
// erased after traversing ip->copied. All this code has been removed
// because we will very soon have full callback functionality and thus the
// code is redundant.
void journal_commit(uint8 type)
{
    struct inode_s *ip    = inode_addr(fs.journal.i);
    struct inode_s *oldip = inode_addr(fs.journal.oldi);
    struct inode_s *dp;
    bref_t b;

    tw(tr(TR_BEGIN, TrJournal, "journal_commit(%d) {\n", type));
    tw(tr(TR_FUNC, TrJournal, "i = %d\n", fs.journal.i));
    ttw(ttr(TTrObj, "jc(){" NL));

    if (fs.journal.i)
    {
        // If this is a create, set the object type
        if (type != 0 && fs.journal.oldi == 0)
            fs.journal.flags = (fs.journal.flags & OF_MASK) | type;
        
        tw(tr(TR_FUNC, TrJournal, "loc   = 0x%04x, size = %d\n",
              fs.journal.location, fs.journal.size));
        ffsdrv.write((uint32 *) &ip->location, (uint32 *) &fs.journal.location, sizeof(location_t));
        ffsdrv.write_halfword((uint16 *) &ip->size,     fs.journal.size);

        if (fs.journal.oldi != 0 && fs.link_child != 0)
            // If this is an update, we copy the child member from old
            // inode. We must do this before we validate the new object,
            // otherwise an intermediate readdir() will detect an empty
            // directory!
            ffsdrv.write_halfword((uint16*) &ip->child, oldip->child);
    
        tw(tr(TR_FUNC, TrJournal, "seq   = %d\n", fs.sequence));
        // We must check if sequence is already written because if this
        // commit was inititiated by journal_init(), we don't know exactly
        // what was written
        if (ip->sequence == FLASH_NULL16)
            ffsdrv.write_halfword(&ip->sequence, fs.sequence++);
        if (fs.journal.oldi == 0)
            ffsdrv.write_halfword(&ip->updates,  0);
        else
            ffsdrv.write_halfword(&ip->updates,  oldip->updates + 1);

        JOURNAL_TEST(JOURNAL_TEST_COMMITTING, "Oops in JOURNAL_TEST_COMMITTING")

        // Insert object into directory structure. We must do this before
        // deleting old object, otherwise an intermediate readdir() will
        // fail with EFFS_NOTFOUND. Note that when the root directory is
        // created, fs.journal.diri is zero --- thus the test!
        if (fs.journal.diri != 0) {
            tw(tr(TR_FUNC,  TrJournal, "diri  = %d ", fs.journal.diri));
            if (fs.journal.diri < 0) {
                tw(tr(TR_NULL,  TrJournal, "child\n"));
                dp = inode_addr(-fs.journal.diri);
                ffsdrv.write_halfword((uint16 *) &dp->child, fs.journal.i);
            }
            else {
                tw(tr(TR_NULL,  TrJournal, "sibling\n"));
                dp = inode_addr(fs.journal.diri);
                ffsdrv.write_halfword((uint16 *) &dp->sibling, fs.journal.i);
            }
        }

        // The new object is validated before the old object is deleted.
        // This is in order to avoid an interrupting stat or read operation
        // to fail with EFFS_NOTFOUND
        tw(tr(TR_FUNC,  TrJournal, "flags = 0x%02x\n", fs.journal.flags));
        ffsdrv_write_byte(&ip->flags, fs.journal.flags);

        // Update bstat[] appropriately
        b = offset2block(location2offset(ip->location));
        bstat[b].objects++;
        tw(tr(TR_FUNC,  TrJournal, "bstat[%d].objects = %d\n", b, bstat[b].objects));
    }

    tw(tr(TR_FUNC,  TrJournal, "oldi  = %d\n", fs.journal.oldi));
    if (fs.journal.oldi != 0)
    {
        // If this is an update or an erase, we erase the old object
        ffsdrv_write_byte(&oldip->flags, OT_ERASED);

        // Update bstat according to deletion of the old object.
        b = offset2block(location2offset(oldip->location));
        bstat[b].objects--;
        tw(tr(TR_FUNC,  TrJournal, "bstat[%d].objects = %d\n", b, bstat[b].objects));

        // If we moved the data (all cases, except fcontrol), update lost
        if (fs.journal.location != oldip->location)
            bstat[b].lost += oldip->size;

        bstat[fs.inodes].lost++;
        
        // If we renamed a file to an existing filename, remove the replaced file. 
        if (fs.journal.repli > 0)
            object_remove(fs.journal.repli); // Ignore error! 
    }

    tw(tr(TR_END, TrJournal, "}\n"));
    ttw(ttr(TTrObj, "}" NL));
}

// Save the current journal into "old" journal. We need this because an
// object_create() can call data_reclaim() which can call object_relocate()
// which uses the journal system.
int journal_push(void)
{
    memcpy(&fs.ojournal, &fs.journal, sizeof(struct journal_s));
    fs.journal_depth++;
    if (fs.journal_depth > 1) {
        tw(tr(TR_FUNC, TrAll, "FATAL: journal_push() to depth %d\n",
              fs.journal_depth));
	return -1;
    }

    tw(tr(TR_FUNC, TrJournal, "journal_push() to depth %d\n",
          fs.journal_depth));

    return EFFS_OK;
}

// Recall "old" journal into current journal
int journal_pop(void)
{
    tw(tr(TR_FUNC, TrJournal, "journal_pop() from depth %d\n",
          fs.journal_depth));

    fs.journal_depth--;
    if (fs.journal_depth < 0) {
        tw(tr(TR_FUNC, TrAll, "FATAL: journal_pop() to depth %d\n",
              fs.journal_depth));
	return -1;
    }
    memcpy(&fs.journal, &fs.ojournal, sizeof(struct journal_s));

    return EFFS_OK;
}

// Initialize the journalling system. Create journal file if it not already
// exist. Commit/write pending journal if such exists --- return 1 in that
// case. Otherwise, if journal file is clean (no journals pending) and all
// is fine, return EFFS_OK.
effs_t journal_init(iref_t i)
{
    int j;
    struct inode_s *ip = inode_addr(i);
    struct journal_s *addr;

    if (i == 0) {
        // Journal file does not exist, so create it
        if ((i = journal_create(0)) <= 0) {
            fs.ijournal = 0;
            return i;
        }
    }

    fs.journal_depth = 0;
    fs.journal_pos = JOURNAL_POS_INITIAL;

    addr = (struct journal_s *)
        offset2addr(location2offset(ip->location) + fs.journal_pos);

    tw(tr(TR_BEGIN, TrJournal, "journal_init(%d) {\n", i));

    fs.ijournal = i;

    // Search for first non-completed journal entry.
    for (j = 0; /* FIXME: limit to end of journal */; j++, addr++) {
        if (addr->state != (uint8) JOURNAL_IS_DONE)
            break;
    }
    tw(tr(TR_FUNC, TrJournal, "entry %d is in state 0x%x\n", j, addr->state));

    fs.journal_pos += j * sizeof(fs.journal);
    i = EFFS_OK;

    if (addr->state == (uint8) JOURNAL_IS_EMPTY) {
        tw(tr(TR_FUNC, TrJournal, "Last journal is in EMPTY state\n"));
        // Journal file is proper, so just record position
    }
    else if (addr->state == (uint8) JOURNAL_IS_READY) {
        // Copy the entry into fs.journal.
        tw(tr(TR_FUNC, TrJournal, "Last journal is in READY state\n"));
        memcpy(&fs.journal, addr, sizeof(fs.journal));
        journal_end(0);
        i = 1;
    }
    else {
        // Journal entry wasn't finished, so just ignore it after updating
        // its state to JOURNAL_IS_DONE.
        tw(tr(TR_FUNC, TrJournal, "Last journal is between EMPTY and READY\n"));
        ffsdrv_write_byte(&addr->state, JOURNAL_IS_DONE);
        fs.journal_pos += sizeof(fs.journal);
    }

    if (ip->size != fs.journal_size + atomalign(sizeof(FFS_JOURNAL_NAME) + 1)) {
        tw(tr(TR_FUNC, TrJournal, "Wrong journal size, create new\n"));
        // Journal size do not match default size, so create new. This
        // should only happen if we use an old FFS image with a newer FFS
        // version.
        if ((i = journal_create(fs.ijournal)) <= 0) {
            fs.ijournal = 0;
            return i;
        }
    }

    tw(tr(TR_FUNC, TrJournal, "journal_pos = 0x%04x\n", fs.journal_pos));
    tw(tr(TR_END, TrJournal, "} %d\n", i));
    
    return i;
}

// Create the journal file from scratch or relocate an existing one. It is
// marked read-only just for clarity --- it cannot be deleted anyway!
// fs_format() calls this function. Note that no data are written in
// object_create() because the journal file is handled specially in that
// function.
iref_t journal_create(iref_t oldi)
{
    iref_t i;

    tw(tr(TR_BEGIN, TrJournal, "journal_create(%d) {\n", oldi));
    tw(tr(TR_FUNC, TrJournal, "journal file size = %d\n", fs.journal_size));

    if (fs.journal_size == 0) {
        tw(tr(TR_FUNC, TrJournal, "Journal file creation aborted because fs.journal_size = 0 (No journal file wanted)\n"));
        tw(tr(TR_END, TrJournal, "} %d\n", 0));
        return 0;
    }

    // If we are working on a write-once file system, we do not need a
    // journal.
    if (fs.blocks_free_min == 0) {
        tw(tr(TR_FUNC, TrJournal, "Journal file creation aborted because fs.blocks_free_min = 0 (write-once system)\n"));
        tw(tr(TR_END, TrJournal, "} %d\n", 0));
        return 0;
    }

    journal_begin(oldi);

    i = object_create(FFS_JOURNAL_NAME, 0, fs.journal_size, -fs.root);
    if (i < 0) {
        tw(tr(TR_END, TrJournal, "} %d\n", i));
        return i;
    }
    fs.journal.flags = BIT_SET(fs.journal.flags, OF_READONLY);

    // commit the creation or relocation
    if (oldi != 0)
        journal_end(0);
    else {
        journal_commit(OT_FILE);
        fs.journal_pos = JOURNAL_POS_INITIAL;
    }

    tw(tr(TR_END, TrJournal, "} %d\n", i));

    return i;
}

/******************************************************************************
 * FFS Begin and End
 ******************************************************************************/

// The following two functions should surround the code of every API
// function in ffs.c (except preformat and format). The functions
// ensures that the operation about to be executed can be made without
// race-conditions or other problems.
#if (TARGET == 0)
int debug_suspend = 0;
#endif


// Check if ffs has been initialized. Suspend an erase operation.
effs_t ffs_begin(void)
{
#if (TARGET == 0)  
    if (debug_suspend > 0) {
        tw(tr(TR_FUNC, TrAll, "FATAL: Previous erase_suspend was not resumed\n"));
        return EFFS_CORRUPTED;
    }
//    tw(tr(TR_FUNC, TrHelper, "Set debug_suspend\n"));
    debug_suspend = 1;
#endif

    if (fs.initerror != EFFS_OK)
        return fs.initerror;

    // Suspend an erase in progress (only applicable if we are using a
    // multi-bank device driver)
    if (dev.state == DEV_ERASE) {
        ffsdrv.erase_suspend();
    }
    else if (dev.state == DEV_WRITE) {
        ffsdrv.write_end();
    }
        
    return EFFS_OK;
}

// Resume an erase operation that was in progress.
int ffs_end(int error)
{
#if (TARGET == 1)
    // Resume an erase in progress (only applicable if we are using a
    // multi-bank device driver)
    if (dev.state == DEV_ERASE_SUSPEND) {
        ffsdrv.erase_resume();
    }
#else
    debug_suspend = 0;
#endif

    return error;
}

/******************************************************************************
 * FFS Statistics functions
 ******************************************************************************/

// Not implemented:
int statistics_file_create(void)
{
    return 0;
}

// Not implemented:
// Rewrite the statistics file if it exists. Otherwise return error
// code. The function is called after each data and inodes reclaim (after
// writing the file that provoked the reclaim).
int statistics_write(void)
{
    return 0;
}

// Read the statistics file if it exists. Otherwise reset all statistics to
// zero and set the magic. This function is called from ffs_init().
void statistics_init(void)
{
    memset(&stats, 0, sizeof(struct ffs_stats_s));
}

void statistics_update_drec(int valid, int lost, int candidate)
{
    unsigned int old;

    switch (candidate) {
    case MOST_LOST:   stats.drec.most_lost++;   break;
    case MOST_UNUSED: stats.drec.most_unused++; break;
    case YOUNGEST:    stats.drec.youngest++;    break;
    }

    // Increment Most Significant Word if overflow is detected
    old = stats.drec.valid[0];
    stats.drec.valid[0] += valid;
    if (old > stats.drec.valid[0])
        stats.drec.valid[1]++;

    old = stats.drec.lost[0];
    stats.drec.lost[0] += lost;
    if (old > stats.drec.lost[0])
        stats.drec.lost[1]++;
}

void statistics_update_irec(int valid, int lost)
{
    stats.irec.num++;
    stats.irec.valid += valid;
    stats.irec.lost += lost;
}
