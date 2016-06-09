/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * FFS core reclaim functionality
 *
 * $Id: reclaim.c 1.4.1.28 Thu, 08 Jan 2004 15:05:23 +0100 tsj $
 *
 ******************************************************************************/

#include "ffs.h"
#include "core.h"
#include "drv.h"
#include "ffstrace.h"

extern int rand();

/******************************************************************************
 * Inodes Reclaim
 ******************************************************************************/

void inodes_recurse(iref_t i)
{
    iref_t pi;
    struct inode_s *ip, *newip;

    tw(tr(TR_BEGIN, TrReclaimLow, "inodes_recurse(%d) {\n", i));

    ip    = inode_addr(i);
    newip = (struct inode_s *) offset2addr(dev.binfo[fs.newinodes].offset) + i;
    
    // copy inode dir to new block, except child, sibling and copied
    ffsdrv.write((uint32*) &newip->location, (uint32*) &ip->location, sizeof(location_t));
    ffsdrv.write_halfword((uint16*) &newip->size,     ip->size);
    ffsdrv_write_byte    (&newip->flags,    ip->flags);
    ffsdrv.write_halfword((uint16*) &newip->sequence, ip->sequence);
    ffsdrv.write_halfword((uint16*) &newip->updates,  ip->updates);
    bstat[fs.newinodes].used++;

    // if no children of this dir, we have no more work to do
    if (ip->child == (iref_t) IREF_NULL) {
        tw(tr(TR_END, TrReclaimLow, "}\n"));
        return;
    }

    pi = -i;
    i = ip->child;
    ip = inode_addr(i);

    do {
        tw(tr(TR_FUNC, TrReclaimLow, "pi = %d, i = %d", pi, i));

        tw(tr(TR_NULL, TrReclaimLow, ", size = %d, location = 0x%x", ip->size, 
              ip->location));
        
        tw(tr(TR_NULL, TrReclaimLow, ", name_addr = 0x%x",
              addr2name(offset2addr(location2offset(ip->location)))));
        
        if (is_object(ip, OT_SEGMENT)) 
            tw(tr(TR_NULL, TrReclaimLow, ", (segment)\n"));
        
        else
            tw(tr(TR_NULL, TrReclaimLow, ", '%s'\n",
                  (ip->size ? addr2name(offset2addr(location2offset(ip->location)))
                   : "(cleaned)")));

        if (is_object_valid(ip))
        {
            if (is_object(ip, OT_DIR)) {
                tw(tr(TR_NULL, TrReclaimLow, "recursing...\n", i));
                inodes_recurse(i);
            }
            else {
                tw(tr(TR_NULL, TrReclaimLow, "copying...\n"));
                // copy inode to new block, except child, sibling and copied
                newip = (struct inode_s *)
                    offset2addr(dev.binfo[fs.newinodes].offset) + i;
                ffsdrv.write((uint32*) &newip->location, (uint32*) &ip->location, sizeof(location_t));
                ffsdrv.write_halfword((uint16*) &newip->size,     ip->size);
                ffsdrv_write_byte    (&newip->flags,    ip->flags);
                ffsdrv.write_halfword((uint16*) &newip->sequence, ip->sequence);
                ffsdrv.write_halfword((uint16*) &newip->updates,  ip->updates);
                bstat[fs.newinodes].used++;
            }

            tw(tr(TR_FUNC, TrReclaimLow, "Linking: %d->%d\n",pi, i));
            // now write the child or sibling link of previous inode
            newip = (struct inode_s *)
                offset2addr(dev.binfo[fs.newinodes].offset);
            if (pi > 0)
                ffsdrv.write_halfword((uint16*) &(newip + pi)->sibling, i);
            else
                ffsdrv.write_halfword((uint16*) &(newip + (-pi))->child, i);
            
            pi = i; // save index of previous inode
            
            if (ip->child != (iref_t) IREF_NULL && is_object(ip, OT_FILE)) {
                iref_t pis, is;
                struct inode_s *ips;
                pis = i;
                ips = ip;

                tw(tr(TR_FUNC, TrReclaimLow, "Follow segment head\n"));
                // While child is valid
                while ((is = ips->child) != (iref_t) IREF_NULL) {

                    // Get child
                    is = ips->child;
                    ips = inode_addr(is);
                    tw(tr(TR_FUNC, TrReclaimLow, "Child ok, got new child i = %d\n", is));
                    // While object not is valid
                    while (!is_object_valid(ips)) {
                        tw(tr(TR_FUNC, TrReclaimLow, "pi = %d, i = %d c(cleaned)\n", pis, is));
                        // If sibling are valid
                        if (ips->sibling != (iref_t) IREF_NULL) {  
                            // Get sibling
                            is = ips->sibling;
                            ips = inode_addr(is);
                            tw(tr(TR_FUNC, TrReclaimLow, "Sibling ok, got new sibling i = %d\n", is));
                        }
                        else {
                            tw(tr(TR_FUNC, TrReclaimLow, "Sibling = FF (%d)\n", ips->sibling));
                            break;  // Nothing more todo, child and sibling = FF
                        }
                    }
                    // If object is valid
                    if (is_object_valid(ips)) {
                        tw(tr(TR_NULL, TrReclaimLow, "copying...\n"));
                        // copy inode to new block, except child, sibling and copied
                        newip = (struct inode_s *)
                            offset2addr(dev.binfo[fs.newinodes].offset) + is;
                        ffsdrv.write((uint32*) &newip->location, (uint32*) &ips->location, sizeof(location_t));
                        ffsdrv.write_halfword((uint16*) &newip->size,     ips->size);
                        ffsdrv_write_byte    (&newip->flags,              ips->flags);
                        ffsdrv.write_halfword((uint16*) &newip->sequence, ips->sequence);
                        ffsdrv.write_halfword((uint16*) &newip->updates,  ips->updates);
                        bstat[fs.newinodes].used++;
                        
                        tw(tr(TR_FUNC, TrReclaimLow, "Linking child: %d->%d\n",pis, is));
                        // now write the child link of previous inode
                        newip = (struct inode_s *)
                            offset2addr(dev.binfo[fs.newinodes].offset);
                        ffsdrv.write_halfword((uint16*) &(newip + (pis))->child, is);
                        
                        pis = is; // save index of previous inode   
               
                    }     
                    else {
                        tw(tr(TR_FUNC, TrReclaimLow, "Sibling = FF (%d, %d)\n", 
                              ips->sibling, ips->child));
                    }

                }
            }
        }       
        else {
            tw(tr(TR_NULL, TrReclaimLow, "(ignoring)\n"));
        }
        i = ip->sibling;
        ip = inode_addr(i);
            
    } while (i != (iref_t) IREF_NULL);
    
    tw(tr(TR_END, TrReclaimLow, "}\n"));
}

// Reclaim inodes, eg. move inodes to another block and erase old one.
effs_t inodes_reclaim(void)
{
    tw(tr(TR_BEGIN, TrIReclaim, "inodes_reclaim() {\n"));
    ttw(str(TTrRec, "irec{"));

    if (fs.initerror != EFFS_OK) {
        tw(tr(TR_END, TrIReclaim, "} %d\n", fs.initerror));
        ttw(ttr(TTrRec, "} %d" NL, fs.initerror));
        return fs.initerror;
    }

    if ((fs.newinodes = block_alloc(1, BF_COPYING)) < 0) {
        tw(tr(TR_END, TrIReclaim, "} %d\n", EFFS_NOBLOCKS));
        ttw(ttr(TTrRec, "} %d" NL, EFFS_NOBLOCKS));
        return EFFS_NOBLOCKS;
    }

    statistics_update_irec(bstat[fs.inodes].used - bstat[fs.inodes].lost, 
                           bstat[fs.inodes].lost);

    // copy all inodes...
    bstat[fs.newinodes].used = 0;
    inodes_recurse(fs.root);

    block_commit();

    tw(tr(TR_END, TrIReclaim, "} 0\n"));
    ttw(str(TTrRec, "} 0" NL));

    return EFFS_OK;
}

#if (FFS_TEST == 0)
#define BLOCK_COMMIT_TEST(testcase, text)
#else
#if (TARGET == 0)
// NOTEME: We have compressed the macro code because it will NOT compile on
// Unix otherwise. So until we find out why, we use this as a work-around.
#define BLOCK_COMMIT_TEST(testcase, text) if (fs.testflags == testcase) { tw(tr(TR_FUNC, TrData, "} (" text ")\n")); return; }
#else
#define BLOCK_COMMIT_TEST(testcase, text) if (fs.testflags == testcase) { ttw(ttr(TTrData, "} (" text ")\n")); return; }
#endif
#endif

// Inode -> Lost, Copying -> Inode, Lost -> Free
void block_commit(void)
{
    int oldinodes = fs.inodes;

    tw(tr(TR_BEGIN, TrIReclaim, "block_commit(%d -> %d) {\n", 
       oldinodes, fs.newinodes));
    ttw(ttr(TTrRec, "block_commit(%d -> %d) {\n" NL, 
       oldinodes, fs.newinodes)); 

    BLOCK_COMMIT_TEST(BLOCK_COMMIT_BEFORE, "Oops before commit");

    block_flags_write(oldinodes, BF_LOST);
 
    BLOCK_COMMIT_TEST(BLOCK_COMMIT_NO_VALID, "Oops no valid inode block");

    // Validate new block as an inodes block
    block_flags_write(fs.newinodes, BF_INODES);

    bstat[fs.newinodes].lost = 0;
    bstat[fs.newinodes].objects = 1;
    inodes_set(fs.newinodes);

    // Free old inodes block
    block_free(oldinodes);

    BLOCK_COMMIT_TEST(BLOCK_COMMIT_OLD_FREE, "Oops after freeing old block");

    BLOCK_COMMIT_TEST(BLOCK_COMMIT_AFTER, "Oops after commit");

    ttw(str(TTrRec, "} 0" NL));
    tw(tr(TR_END, TrIReclaim, "}\n"));
}


/******************************************************************************
 * Data Reclaim
 ******************************************************************************/

// Important note: We must NOT perform a data reclaim when we are in the
// process of creating the journal file!

// Reclaim a data block, eg. move files to other blocks and erase old one.
// When the reclaim is done, we must completely delete the old inodes which
// are pointing into the old data sector which is going to be erased now.
iref_t data_reclaim(int space)
{
    iref_t error;

    tw(tr(TR_BEGIN, TrDReclaim, "data_reclaim(%d) {\n", space));

    if (fs.initerror != EFFS_OK) {
        tw(tr(TR_END, TrDReclaim, "} %d\n", fs.initerror));
        return fs.initerror;
    }

    error = data_reclaim_try(space);

    tw(tr(TR_END, TrDReclaim, "} (data_reclaim) %d\n", error));

    return error;
}

int dage_max_reached(int dage_blk, int agegain)
{
    int reclaim, early, log2, mask;

    tw(tr(TR_BEGIN, TrDReclaim, "young(%d, %d) {\n", dage_blk, agegain));
    
    // Simple algorithm
    reclaim = (dage_blk + agegain - 2 * FFS_DAGE_MAX >= 0);

    // Early exponential probability based reclaim
    early = FFS_DAGE_MAX - dage_blk;
    if (agegain > dage_blk - 4 && 0 < early && early <= FFS_DAGE_EARLY_WIDTH) {
        if (early < 4)
            early = 2;
        if (early < FFS_DAGE_EARLY_WIDTH) {
            // Now make an exponential probability distributon by
            // generating a bitmask of a size relative to (dage_blk
            // - DAGE_EARLY_WIDTH)
            log2 = -1;
            while (early > 0) {
                early >>= 1;
                log2++;
            }
            reclaim = log2;
            
            mask = (1 << (log2 + 1)) - 1;
            reclaim = ((rand() & mask) == 0);
        }
    }

    // Do not perform a reclaim unless we gain a certain minimum
    if (agegain < FFS_DAGE_GAIN_MIN)
        reclaim = 0;

    tw(tr(TR_END, TrDReclaim, "} (%d)\n", reclaim));
    return reclaim;
}


// Try to reclaim at least <space> bytes of data space. On success, return
// the number of bytes actually reclaimed. Otherwise, on failure, return a
// (negative) error.
int data_reclaim_try(int space)
{
    // 1. Find a suitable block to reclaim.
    //
    // 2. Relocate each valid object from old block (to another block). An
    // object relocation is similar to a normal file update, e.g. similar to
    // fupdate().
    //
    // 3. If there is not enough space to relocate a file, we must alloc a
    // new block then data_format() it.
    //
    // 4. set BF_CLEANING flag of old block.
    //
    // 5. ALL inodes (also invalid an erased ones) referring into reclaimed
    // block must now be totally wiped out.
    //
    // 6. Free (invalidate) old block.

    int result = 0, reserved_ok = 0;
    bref_t b, blocks_free;
    bref_t brc_young_b, brc_lost_b, brc_unused_b;

    blocksize_t brc_lost_lost,   brc_lost_unused;
    blocksize_t brc_unused_unused;
    blocksize_t unused, unused_total, lost, lost_total, free;

    age_t brc_young_dage, free_dage, dage;
    struct block_header_s *bhp;
    // Note gain can be negative if the free block is younger than the youngest data block
    int age_gain; 

    tw(tr(TR_BEGIN, TrDReclaim, "data_reclaim_try(%d) {\n", space));
    ttw(str(TTrRec, "drec{" NL));

    // While searching for a block to reclaim, we maintain three block
    // reclaim candidates (brc): One with the maximum number of lost bytes,
    // one with the maximum number of unused bytes and another for the
    // youngest block, e.g. the one with the largest age distance to
    // fs.age_max. The candidates are tried in the order mentioned.
    
    // This counts free blocks, so we initialize to number of blocks minus
    // one for inodes.
    blocks_free = dev.numblocks - 1;

    // Initialize Block Reclaim Candidate (brc) variables
    brc_lost_b   = -1; brc_lost_unused   = 0; brc_lost_lost   = 0;
    brc_unused_b = -1; brc_unused_unused = 0; 

    brc_young_b  = -1; brc_young_dage = 0;  free_dage  = 0;

    lost_total   = 0;
    unused_total = 0;

    tw(tr(TR_FUNC, TrDReclaim,
          "blk  unused    lost  w/age   age dist  objs\n"));
    for (b = 0; b < dev.numblocks; b++)
    {
        bhp = (struct block_header_s *) offset2addr(dev.binfo[b].offset);

        if (is_block(b, BF_IS_DATA))
        {
            // Record number of lost bytes and number of unused bytes,
            // eg. total space that would be freed if this block was
            // reclaimed
            lost   = bstat[b].lost;
            unused = dev.blocksize - (bstat[b].used - bstat[b].lost);
            free   = dev.blocksize - bstat[b].used;

            lost_total   += lost;
            unused_total += unused;

            if (free >= RESERVED_LOW) 
                reserved_ok = 1;
            if (lost > brc_lost_lost) {
                brc_lost_b = b;
                brc_lost_lost = lost;
                brc_lost_unused = unused;
            }
            if (unused > brc_unused_unused) {
                brc_unused_b = b;
                brc_unused_unused = unused;
            }

            tw(tr(TR_FUNC, TrDReclaim, "%3d %7d %7d ", b, unused, lost));

            dage = saturate_dage(fs.age_max - bhp->age);

            tw(tr(TR_NULL, TrDReclaim, "%6d %5d %4d   %3d\n",
                  lost, bhp->age, dage, bstat[b].objects));

            if (dage >= brc_young_dage) {
                brc_young_b = b;
                brc_young_dage = dage;
            }
            blocks_free--;
        }
        else if (is_block(b, BF_IS_FREE)) {
            unused_total += dev.blocksize;

            // Find youngest free block (in must cases we will only have one free b)
            dage = saturate_dage(fs.age_max - bhp->age);

            if (dage >= free_dage)
                free_dage = dage;   // Delta age of youngest free block
        }
    }
    tw(tr(TR_FUNC, TrDReclaim, "sum %7d %7d\n", unused_total, lost_total));
    tw(tr(TR_FUNC, TrDReclaim, "blocks_free = %d, fs.age_max = %d\n", blocks_free, fs.age_max));

    age_gain = brc_young_dage - free_dage; // Same as free - block age
  
    if (space > unused_total) {
        // We will never be able to reclaim this amount...
        result = 0;
    }
    else {
        // No additional blocks (apart from spare block) are free...
        tw(tr(TR_FUNC, TrDReclaim,
              "brc_young_dage = %d, brc_lost_unused = %d, brc_unused_unused = %d\n",
              brc_young_dage, brc_lost_unused, brc_unused_unused));
    
        if (reserved_ok == 0) {
            tw(tr(TR_FUNC, TrDReclaim, 
                  "No reserved, reclaim most-lost block (%d)\n", brc_unused_b));
            result = data_block_reclaim(brc_lost_b, MOST_LOST);
        }
        else if (dage_max_reached(brc_young_dage, age_gain) > 0 ) {
            tw(tr(TR_FUNC, TrDReclaim, "Reclaiming youngest block (%d)\n",
                  brc_young_b));
            result = data_block_reclaim(brc_young_b, YOUNGEST);
        }
        else if (brc_lost_unused >= space) {
            tw(tr(TR_FUNC, TrDReclaim, "Reclaiming most-lost block (%d)\n",
                  brc_lost_b));
            result = data_block_reclaim(brc_lost_b, MOST_LOST);
        }
        else if (brc_unused_unused >= space) {
            tw(tr(TR_FUNC, TrDReclaim, "Reclaiming most-unused block (%d)\n",
                  brc_unused_b));
            result = data_block_reclaim(brc_unused_b, MOST_UNUSED);
        }
        else {
            tw(tr(TR_FUNC, TrDReclaim, "Reclaiming most-lost blockx (%d)\n",
                  brc_lost_b));
            result = data_block_reclaim(brc_lost_b, MOST_LOST);
            if (result >= 0)
                result = 0;  // We reclaimed a block but we still need more space
        }

    }
    tw(tr(TR_END, TrDReclaim, "} (data_reclaim_try) %d\n", result));

    return result;
}


#if (FFS_TEST == 0)
#define BLOCK_RECLAIM_TEST(testcase, text)
#else
#if (TARGET == 0)
// NOTEME: We have compressed the macro code because it will NOT compile on
// Unix otherwise. So until we find out why, we use this as a work-around.
#define BLOCK_RECLAIM_TEST(testcase, text) if (fs.testflags == testcase) { tw(tr(TR_FUNC, TrTestHigh, "(" text ")\n")); tw(tr(TR_END, TrDReclaim, "} (Test) -100\n", result));return -100; }
#else
#define BLOCK_RECLAIM_TEST(testcase, text) if (fs.testflags == testcase) { ttw(ttr(TTrData, "} (" text ")"NL)); ttw(ttr(TTrRec, "} (Test) -100" NL));return -100; }
#endif
#endif

#if (FFS_TEST == 0)
#define BLOCK_RECOVER_TEST_INIT(testcase, text)
#define BLOCK_RECOVER_TEST(testcase, text)
#else
#if (TARGET == 0)
#define BLOCK_RECOVER_TEST_INIT(testcase, text) int rand_object; if (fs.testflags == testcase) { rand_object = rand() % bstat[b].objects; tw(tr(TR_FUNC, TrTestHigh, "Fail when object nr %d is relocated\n", rand_object)); }

#define BLOCK_RECOVER_TEST(testcase, text) if (fs.testflags == testcase) {if (rand_object == n) { tw(tr(TR_FUNC, TrTestHigh, "(" text ")\n")); tw(tr(TR_END, TrDReclaim, "} (Test) -101\n", result)); return -101; } }

#else   
#define BLOCK_RECOVER_TEST_INIT(testcase, text) int rand_object; if (fs.testflags == testcase) { rand_object = rand() % bstat[b].objects; ttw(ttr(TTrData, "Fail when object nr %d is relocated" NL, rand_object)); }
#define BLOCK_RECOVER_TEST(testcase, text) if (fs.testflags == testcase) {if (rand_object == n) { ttw(ttr(TTrData, "(" text ")" NL)); ttw(ttr(TTrRec, "} (Test) -101" NL, result)); return -101; } }
#endif
#endif

iref_t data_block_reclaim(bref_t b, int candidate)
{
    iref_t i, n, j;
    blocksize_t used_old, lost_old;
    int org_res_space, result = 0;
    iref_t org_block_files_reserved;
    offset_t lower, upper;
    struct inode_s *ip;
    static int is_reclaim_running = 0;

    tw(tr(TR_BEGIN, TrDReclaim, "data_block_reclaim(%d) {\n", b));

    // In case of no free blocks (after sudden power off) or if the file
    // system is near full we risk to be reentered (infinity recursively
    // loop) and we can not allow that, so just return.
    if (is_reclaim_running == 1) {
	tw(tr(TR_END, TrDReclaim, "} (reenteret skip reclaim) 0\n"));
	return EFFS_RECLAIMLOOP;
    }

    is_reclaim_running = 1;

    // If there are more objects in this block than there are remaining
    // free inodes, we have to make an inodes_reclaim() first.
    tw(tr(TR_FUNC, TrDReclaim,
          "block_objects, fs.inodes_max, inodes: used, free\n"));
    tw(tr(TR_FUNC, TrDReclaim,
          "%10d, %13d, %15d, %4d\n",
          bstat[b].objects,
          fs.inodes_max, bstat[fs.inodes].used,
          fs.inodes_max - (bstat[fs.inodes].used + bstat[fs.inodes].lost)));

    if (bstat[b].objects >= (fs.inodes_max - (bstat[fs.inodes].used + 
                                              bstat[fs.inodes].lost + 
                                              FFS_INODES_MARGIN))) {
        tw(tr(TR_FUNC, TrInode, "NOTE: Will run out of free inodes...\n"));
        inodes_reclaim();
    }

    // Allocate a new block. NOTE: we don't return an error because if we
    // get in the situation where we don't have any free blocks this is the
    // only way to recover.
    if ((result = block_alloc(1, BF_DATA)) < 0) {
        tw(tr(TR_FUNC, TrAll, "WARNING: block_alloc failed\n"));
    }

    BLOCK_RECLAIM_TEST(BLOCK_RECLAIM_ALLOC, "Oops after ffs_block_alloc()");

    // If there are any objects at all to reclaim...
    if (bstat[b].objects > 0)
    {
	BLOCK_RECOVER_TEST_INIT(BLOCK_RECOVER_OBJECTS, "Dummy")
            // Save the current journal state
            if (journal_push() != EFFS_OK) {
                is_reclaim_running = 0;       // NOTEME: change to goto?
                return EFFS_CORRUPTED;
            }

        // We simulate that this block is completely full, such that we
        // don't relocate files to the end of the block
        used_old = bstat[b].used;
        lost_old = bstat[b].lost;  // For statistics
        bstat[b].used = dev.blocksize - 1;


        // Compute lower (inclusive) and upper (exclusive) bounds of the
        // location of files in this block
        lower = offset2location(dev.binfo[b].offset);
        upper = offset2location(dev.binfo[b].offset + dev.blocksize);

        tw(tr(TR_FUNC, TrDReclaim, "Block addr range = 0x%X..0x%X\n",
              location2offset(lower), location2offset(upper)));

	// This is the only time we are allowed to use the reserved 
	org_block_files_reserved= fs.block_files_reserved;
	fs.block_files_reserved = 0;

	org_res_space = fs.reserved_space;
        fs.reserved_space = RESERVED_NONE;

	ip = inode_addr(1);
        for (i = 1, n = 0; i < fs.inodes_max; i++, ip++)
        {
	    BLOCK_RECOVER_TEST(BLOCK_RECOVER_OBJECTS, "Oops before relocate all objects");
            // Ensure object is valid and within the block to be reclaimed
            if (is_object_valid(ip) &&
                lower <= ip->location && ip->location < upper)
            {
                if ((result = object_relocate(i)) < 0) {
                    tw(tr(TR_FUNC, TrAll, "FATAL object_relocate failed\n"));
                    break;
                }
                
                // If we reclaim a segment head or wch that is in use we must
                // update the file descriptor as well
                for (j = 0; j < fs.fd_max; j++) {
                    if (i == fs.fd[j].seghead) {
                        tw(tr(TR_FUNC, TrDReclaim, 
                              "Updated seghead %d -> %d \n",
                              fs.fd[j].seghead, result));
                        fs.fd[j].seghead = result;
                    }
                    if (i == fs.fd[j].wch) {
                        tw(tr(TR_FUNC, TrDReclaim, 
                              "Updated wch %d -> %d \n",
                              fs.fd[j].wch, result));
                        fs.fd[j].wch = result;
                    }
                }

                // If we have just reclaimed an object which we started on
                // updating we must also update ojournal
                if (i == fs.ojournal.oldi) {
                    struct inode_s *ip = inode_addr(result);
                    tw(tr(TR_FUNC, TrDReclaim, 
                          "Updated ojournal oldi %d -> %d \n",
                          fs.ojournal.oldi, result));
                    fs.ojournal.oldi     = result;
                    fs.ojournal.location = ip->location;
                }

                if (i == fs.ojournal.diri || i == -fs.ojournal.diri) {
                    fs.ojournal.diri = (fs.ojournal.diri < 0 ? -result : result);
                    tw(tr(TR_FUNC, TrDReclaim, 
                          "Updated ojournal: diri %d -> %d \n", 
                          i, fs.ojournal.diri));
                }

                if (i == fs.ojournal.repli || i == -fs.ojournal.repli) {
                    fs.ojournal.repli = (fs.ojournal.repli < 0 ? -result : result);
                    tw(tr(TR_FUNC, TrDReclaim, 
                          "Updated ojournal: repli %d -> %d \n", 
                          i, fs.ojournal.repli));
                }
 
                if (i == fs.i_backup || i == -fs.i_backup) {
                    fs.i_backup = (fs.i_backup < 0 ? -result : result);
                    tw(tr(TR_FUNC, TrDReclaim, 
                          "Updated i_backup: %d -> %d \n", i, fs.i_backup));
                }

                n++;
            }
        }

	fs.block_files_reserved = org_block_files_reserved; // Restore
	fs.reserved_space = org_res_space;

        tw(tr(TR_FUNC, TrDReclaim, "Reclaimed %d objects\n", n));
        if (result >= 0)
            result = n; // We return number of objects relocated

        if (i < fs.inodes_max) {
            // We did not finish, so restore the old bstat[].used of the block.
            bstat[b].used = used_old;
            tw(tr(TR_FUNC, TrAll,
                  "WARNING: data_block_reclaim() not completed\n"));
            result = EFFS_DBR;
	}

        // Restore the saved journal state
        if (journal_pop() != EFFS_OK) {
	    is_reclaim_running = 0;       // NOTEME: change to goto?
	    return EFFS_CORRUPTED;
	}
    }
    BLOCK_RECLAIM_TEST(BLOCK_RECLAIM_NO_CLEAN, "Oops before clean old data block");

    if (result >= 0) {
        // Clean the block (remove all inodes that refer to this block)
        block_flags_write(b, BF_CLEANING);
        block_clean(b);

        statistics_update_drec(used_old - lost_old, lost_old, candidate); 

	BLOCK_RECLAIM_TEST(BLOCK_RECLAIM_CLEANING, "Oops before free old data block");

        // Free the old block
        block_free(b);
    }

    is_reclaim_running = 0;

    tw(tr(TR_END, TrDReclaim, "} (data_block_reclaim) %d\n", result));
    ttw(ttr(TTrRec, "} %d" NL, result));

    return result;
}

// Relocate object represented by inode reference <i>. 
iref_t object_relocate(iref_t oldi)
{
    iref_t newi;
    struct inode_s *oldip;
    char *olddata, *oldname;
    int oldsize;

    tw(tr(TR_BEGIN, TrReclaimLow, "object_relocate(%d) {\n", oldi));

    journal_begin(oldi);

    oldip = inode_addr(oldi);

    oldsize = segment_datasize(oldip);
    olddata = offset2addr(location2offset(oldip->location));
    oldname = addr2name(olddata);
    olddata = addr2data(olddata, oldip);
    
    if (is_object(oldip, OT_SEGMENT))     
        newi = segment_create(olddata, oldsize, -oldi);   
    else {
        // root inode is a special case
        if (*oldname == '/')
            newi = object_create(oldname, olddata, oldsize, 0);
        else 
            newi = object_create(oldname, olddata, oldsize, oldi);
    }

    if (newi < 0) {
        tw(tr(TR_END, TrReclaimLow, "} %d\n", newi));
        return newi;
    }

    // root inode is a special case
    if ((*oldname == '/') && !is_object(oldip, OT_SEGMENT)) {
        tw(tr(TR_FUNC, TrDReclaim, "Relocating fs.root: %d->%d\n", oldi, newi));
        fs.root = newi;
    }

    journal_end(0);

    tw(tr(TR_END, TrReclaimLow, "} %d\n", newi));

    return newi;
}

// Clean a block, eg. erase all inodes that refer to this block.
iref_t block_clean(bref_t b)
{
    iref_t i, n;
    struct inode_s *ip;
    offset_t lower, upper;

    tw(tr(TR_FUNC, TrDReclaim, "block_clean(%d) { ", b));

    // Compute lower (inclusive) and upper (exclusive) bounds of the
    // location of files in this block
    lower = offset2location(dev.binfo[b].offset);
    upper = offset2location(dev.binfo[b].offset + dev.blocksize);

    tw(tr(TR_FUNC, TrDReclaim, "offset range = 0x%X..0x%X: ", lower, upper));

    ip = inode_addr(1);
    for (i = 1, n = 0; i < fs.inodes_max; i++, ip++)
    {
        // Ensure object is within the block to be reclaimed. Note: if ffs
        // is conf. with 1MB or above will all not used inodes default have
        // the location to FFFF which will trigger a clean and make a error!
        if (lower <= ip->location && upper > ip->location)
        {
            tw(tr(TR_NULL, TrReclaimLow, "%d ", i));
            // Set the size to zero so it won't be counted in ffs_initialize()
            ffsdrv.write_halfword((uint16 *) &ip->size, 0);
            n++;
        }
    }
    tw(tr(TR_NULL, TrDReclaim, "} %d\n", n));

    return n;
}


/******************************************************************************
 * Main and block reclaim
 ******************************************************************************/

// Reclaim (erase) all blocks that are marked as invalid/reclaimable. Each
// time a block is erased, its age is incremented so as to support wear
// levelling. Also, the global age limits are updated.  FIXME: Should we
// avoid having ffs_initialize() do a block_reclaim() because it delays reboot?.
int blocks_reclaim(void)
{
    bref_t b, n, b_lost_space;
    int blocks_free = 0, lost_space;
    int free_space, b_free_space;

    tw(tr(TR_BEGIN, TrBlock, "blocks_reclaim() {\n"));
    ttw(str(TTrRec, "blocks_reclaim() {" NL));

    // Testing of fs.testflags is for the sake of testing block_commit()
    if ((fs.testflags & BLOCK_COMMIT_BASE) != 0) {
        tw(tr(TR_FUNC, TrBlock, "Bailing out because fs.testflags = 0x%X\n",
              fs.testflags));
    }
    else {
        for (b = 0, n = 0; b < dev.numblocks; b++) {
            if (is_block_flag(b, BF_LOST)) {
                block_reclaim(b);
                n++;
            }
	    if (is_block(b, BF_IS_FREE)) {
		blocks_free++;
	    }
        }
    }

    // If the number of free blocks is less than fs.blocks_free_min we
    // call data_block_reclaim(). We will reclaim the block with most lost
    // space. This should only happend if we got a sudden power off/reset
    // while we reclaimed a block.
    if (blocks_free < fs.blocks_free_min) {
	lost_space = 0;
	free_space = 0;

	// We most never reclaim the block with most free space because this
	// is the only block we can relocate the objects to.
	for (b = 0; b < dev.numblocks; b++) {
	    if (is_block_flag(b, BF_DATA)) {
		if ((dev.blocksize - bstat[b].used) > free_space) {
		    free_space = dev.blocksize - bstat[b].used;
		    b_free_space = b;
		}
	    }
	}
	tw(tr(TR_FUNC, TrBlock, "most free space: %d in block: %d \n", 
	      free_space, b_free_space));

	for (b = 0; b < dev.numblocks; b++) {
	    if (is_block_flag(b, BF_DATA) && b != b_free_space) {
		if (bstat[b].lost > lost_space) {
		    lost_space = bstat[b].lost;
		    b_lost_space = b;
		}
	    }
	}
	tw(tr(TR_FUNC, TrBlock, "most lost space: %d in block: %d \n", 
	      lost_space, b_lost_space));

	data_block_reclaim(b_lost_space, MOST_LOST);
    }
    tw(tr(TR_END, TrBlock, "} %d\n", n));
    ttw(ttr(TTrRec, "} %d" NL, n));

    return n;
}

int block_reclaim(bref_t b)
{
    age_t age;
    struct block_header_s *bhp;

    tw(tr(TR_BEGIN, TrBlock, "block_reclaim(%d) {\n", b));

    // In ffs_initialize() we set fs.initerror = EFFS_INVALID while we call
    // blocks_fsck(). We test for that condition now, in order to avoid
    // doing sector erases that will delay the whole target boot process.
    if (fs.initerror == EFFS_INVALID) {
        tw(tr(TR_END, TrBlock, "} %d\n", fs.initerror));
        return fs.initerror;
    }

    // Testing of fs.testflags is for the sake of testing block_commit()
    if ((fs.testflags & BLOCK_COMMIT_BASE) != 0 && 
	fs.testflags != BLOCK_COMMIT_OLD_FREE) {
        tw(tr(TR_FUNC, TrBlock, "Bailing out because fs.testflags = 0x%X\n",
              fs.testflags));
    }
    else {
        // We must read block's age before we erase it.
        bhp = (struct block_header_s *) offset2addr(dev.binfo[b].offset);
        age = bhp->age;
        ffsdrv.erase(b);
        block_preformat(b, age);
    }

    tw(tr(TR_END, TrBlock, "} %d\n", 0));

    return 0;
}
