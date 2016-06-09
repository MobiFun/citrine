/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * FFS core functions (not public)
 *
 * $Id: core.c 1.156.1.13.1.1.1.50 Thu, 08 Jan 2004 15:05:23 +0100 tsj $
 *
 ******************************************************************************/

#include "ffs.h"
#include "core.h"
#include "drv.h"
#include "ffstrace.h"
#include "tmffs.h"
#include <string.h>
#include <limits.h>

/******************************************************************************
 * Globals
 ******************************************************************************/

struct fs_s         fs;
struct block_stat_s bstat[FFS_BLOCKS_MAX];
 
struct ffs_stats_s stats;

// The following line is automatically expanded by the revision control
// system to make a unique ffs revision. The revision can be retrieved by
// ffs_query().

//$Format: "static const uint16 ffs_revision = ($ProjectMajorVersion$<<12)|(0x$ProjectMinorVersion$);"$
static const uint16 ffs_revision = (5<<12)|(0x56);


/******************************************************************************
 * Main Functions
 ******************************************************************************/

// Create a new ffs object (object type is undefined)
iref_t object_create(const char *name, const char *buf, int size, iref_t dir)
{
    iref_t i;
    struct inode_s *ip;
    int realsize, namelength;
    offset_t offset;
    char *dataaddr;
    char is_journal;
    char name_copy[FFS_FILENAME_MAX + 1]; // NOTEME: make dynamic?

    ttw(ttr(TTrObj, "ocr(%s){" NL, name));
    tw(tr(TR_BEGIN, TrObject, "object_create('%s', ?, %d, %d) {\n",
       name, size, dir));

    // NOTEME: special case just for format()!?
    if (dir == 0)
        namelength = ffs_strlen(name);
    else
        namelength = is_filename(name);

    if (namelength < 0) {
        tw(tr(TR_END, TrObject, "} %d\n", namelength));
        ttw(ttr(TTrObj, "} %d" NL, namelength));
        return namelength;
    }

    is_journal = (name[0] == '.' && ffs_strcmp(name, FFS_JOURNAL_NAME) == 0);
    if (is_journal)
        tw(tr(TR_FUNC, TrObject, "Journal file creation!\n"));
    else    
        if (buf == NULL && size > 0) {
            tw(tr(TR_END, TrObject, "} %d\n", EFFS_INVALID));
            ttw(ttr(TTrObj, "} %d" NL, EFFS_INVALID));
            return EFFS_INVALID;
        }
    
    // We don't write the data null_terminator if no data exists
    realsize = namelength + 1 + size + (size > 0 ? 1 : 0);
    fs.journal.size = realsize = atomalign(realsize);

    // We save the diri in the ram journal because this will be updated if
    // chunk_alloc trigger a data_reclaim
    fs.journal.diri = dir;

    // We have to make a local copy of name because name can be destroyed if
    // it points into an object that is relocated by an ffs_data_reclaim.
    memcpy(name_copy, name, ffs_strlen(name) + 1);

    if ((i = chunk_alloc(realsize, is_journal, &offset)) < 0)
        return i;

    ip = inode_addr(i);

    // Write filename including null-terminator
    ffsdrv.write(addr2name(offset2addr(offset)), name_copy, namelength + 1);

    // Write data and null terminator.  We null-terminate the data block,
    // such that blocks_fsck() can determine the amount of used data block
    // space correctly. Note that we don't write null-terminator for objects
    // with no data, e.g. empty files and directories.
    if (size > 0) {
        dataaddr = addr2name(offset2addr(offset)) + namelength + 1;
        // Do NOT write data if we are creating the journal file --- it must
        // be created as empty!
        if (!is_journal)
            ffsdrv.write(dataaddr, buf, size);
        ffsdrv_write_byte(dataaddr + size, 0);
    }

    // Insert object in parent directory if this is not the root dir
    if (dir != 0)
        fs.journal.diri = dir_traverse(fs.journal.diri, 0);
    else
        fs.journal.diri = 0;

    tw(tr(TR_END, TrObject, "} %d\n", i));
    ttw(ttr(TTrObj, "} %d" NL,i));

    return i;
}

int file_read(const char *name, void *addr, int size)
{
    int size_read, object_size, total_read = 0;
    iref_t i, dir;
    char *not_used;
    fd_t fdi;

    if (size < 0)
        return EFFS_INVALID;

    if ((i = object_lookup(name, &not_used, &dir)) < 0)
        return i;

    if ((fdi = get_fdi(i)) >= 0) 
        if (is_open_option(fs.fd[fdi].options, FFS_O_WRONLY))
            return EFFS_LOCKED;

    object_size = object_datasize(i);

    do {
        size_read = segment_read(i, (char*)addr + total_read, 
                                     size - total_read, 0);
        total_read += size_read;
    } while ((i = segment_next(i)) != 0 && size > total_read);

    // Did we read the comlete object?
    if (object_size > size)
        return EFFS_FILETOOBIG;

    return total_read;     // number of bytes read  
}


int stream_read(fd_t fdi, void *src, int size)
{
    int offset, size_read = 0, copied = 0;
    iref_t i;
    
    if (!is_fd_valid(fdi)) 
        return EFFS_BADFD;
    
    if (!is_open_option(fs.fd[fdi].options, FFS_O_RDONLY)) 
        return EFFS_INVALID;  
   
    if (src == NULL || size < 0)
        return EFFS_INVALID;
  
    // NOTEME: do this in another way?
    // No data to read because fp is ad eof.
    if (fs.fd[fdi].fp >= fs.fd[fdi].size) { 
        tw(tr(TR_FUNC, TrObject, "eof(no data read)\n"));
        return 0;       
    }

    segfile_seek(fs.fd[fdi].seghead, fs.fd[fdi].fp, &i, &offset);

    // Read data from chunks or buffer until all data is read or eof is reach.
    do {
        if (is_offset_in_buf(fs.fd[fdi].fp, fdi)) {
            offset = fs.fd[fdi].fp - fs.fd[fdi].wfp;
            size_read = size - copied;    // requested data that is left
            // Saturate size to max left in buf or max left to eof
            if (size_read > (fs.chunk_size_max - offset))  
                size_read = fs.chunk_size_max - offset; 
            if (size_read > (fs.fd[fdi].size - fs.fd[fdi].fp))   
                size_read = fs.fd[fdi].size - fs.fd[fdi].fp;    

            memcpy((char*)src + copied, fs.fd[fdi].buf + offset, size_read);
        }
        else {
            // Data is only in the chunk
            size_read = segment_read(i, (char*) src + copied, 
                                         size - copied, offset);
        }

        offset = 0;
        fs.fd[fdi].fp += size_read;
        copied += size_read;

        if ((i = segment_next(i)) < 0)
            return i;

    } while (copied != size && fs.fd[fdi].fp < fs.fd[fdi].size);

    if (copied == size) {   
        tw(tr(TR_FUNC, TrObject, "All requested data has been read\n"));
    }
    if (fs.fd[fdi].fp >= fs.fd[fdi].size) { 
        tw(tr(TR_FUNC, TrObject, "eof\n"));
    }

    return copied;     // number of bytes read  
}


int object_read(const char *name, char *buf, int size, int linkflag)
{
    iref_t i;
    struct inode_s *ip;
    struct xstat_s stat;
    char *p;

    tw(tr(TR_BEGIN, TrObject, "object_read('%s', 0x%x, %d, %d) {\n",
       name, buf, size, linkflag));

    if (buf == NULL || size < 0) {
     tw(tr(TR_END, TrObject, "} %d\n", EFFS_INVALID));
     return EFFS_INVALID;
    }

    i = object_stat(name, &stat, linkflag, 0, 0);
    if (i < 0) {
        tw(tr(TR_END, TrObject, "} %d\n", EFFS_NOTFOUND));
        return i;
    }

    ip = inode_addr(i);
    
    if (stat.size > size) {
        tw(tr(TR_END, TrObject, "} %d\n", EFFS_FILETOOBIG));
        return EFFS_FILETOOBIG;
    }
    
    // return error if called as readlink() and object is not a link
    if (!is_object(ip, OT_LINK) && linkflag) {
        tw(tr(TR_END, TrObject, "} %d\n", EFFS_INVALID));
        return EFFS_INVALID;
    }

    // Even though the ffs architecture allows to have data in directory
    // objects, we don't want to complicate matters, so we return an error
    if (is_object(ip, OT_DIR) && !(fs.flags & FS_DIR_DATA)) {
        tw(tr(TR_END, TrObject, "} %d\n", EFFS_NOTAFILE));
        return EFFS_NOTAFILE;
    }

    p = offset2addr(location2offset(ip->location));
    size = stat.size;

    p = addr2data(p, ip);

    // Copy data. NOTEME: Should be optimized!
    while (size--)
        *buf++ = *p++;

    tw(tr(TR_END, TrObject, "} %d\n", stat.size));
    return stat.size;
}


// Convert an object data addres to pure data address
char *addr2data(const char *addr, const struct inode_s *ip)
{
    // OT_SEGMENT is pure data so it do not have any name to skip
    if (!is_object(ip, OT_SEGMENT)) {            
        while (*addr++)
            ;
    }

    return (char *) addr;
}

// Calculate exact size of file data; without filename and null terminator
// and without data null terminator and succeeding alignment padding.
// NOTEME: Does this also work for empty files and directories?
int object_datasize(iref_t i)
{
    iref_t not_used;
    return segfile_seek(i, INT_MAX, &not_used, 0);
}

iref_t object_stat(const char *name, struct xstat_s *stat,
                       int linkflag, int fdi, int extended)
{
    iref_t i;
    fd_t other_fdi;
    struct inode_s *ip;
    
    tw(tr(TR_BEGIN, TrObject, "object_stat('%s', ?, %x, %d, %d) {\n", 
          name, linkflag, fdi, extended));
    
    if (stat == NULL) {
        tw(tr(TR_END, TrObject, "} %d\n", EFFS_INVALID));
        return EFFS_INVALID;
    }
    
    if (linkflag)
        i = object_lookup_once(name, 0, 0);
    else if (name == 0) {
        fdi -= FFS_FD_OFFSET;
        if (!is_fd_valid(fdi)) {
            tw(tr(TR_END, TrObject, "} %d\n", EFFS_BADFD));
            return EFFS_BADFD;
        }
        i = fs.fd[fdi].seghead;
    }
    else
        i = object_lookup(name, 0, 0);

    if (i > 0) {
        ip = inode_addr(i);
        stat->type = ip->flags & OT_MASK;;
        stat->flags = ~ip->flags & OF_MASK;
        stat->inode = i;

        // If the file is open so get the size from the file descriptor
        if ((other_fdi = get_fdi(i)) >= 0) {
            if (i == fs.fd[other_fdi].seghead) {
                stat->size = fs.fd[other_fdi].size;
            }
        }
                
        else
            stat->size = object_datasize(i);
                
        if (extended) {
            stat->location = ip->location;
            stat->block = offset2block(location2offset(stat->location));
            stat->space = ip->size;
            while ((i = segment_next(i)) > 0) {
                ip = inode_addr(i);
                stat->space += ip->size;
            }
            stat->reserved = 0;
            stat->sequence = ip->sequence;
            stat->updates = ip->updates;
        }
    }    

    tw(tr(TR_END, TrObject, "} %d\n", i));

    return i;
}


/******************************************************************************
 * Remove and Rename
 ******************************************************************************/

// Delete a ffs object
effs_t object_remove(iref_t i)
{
    struct inode_s *ip = inode_addr(i);
    iref_t entries;

    tw(tr(TR_BEGIN, TrObject, "object_remove(%d) {\n", i));

    // if object is a dir, ensure it is empty
    if (is_object(ip, OT_DIR)) {
        dir_traverse(-i, &entries);
        if (entries) {
            tw(tr(TR_END, TrObject, "} %d\n", EFFS_DIRNOTEMPTY));
            return EFFS_DIRNOTEMPTY;
        }
    }

    // We don't actually journal deletions, this is why we call
    // journal_commit() instead of journal_end(). We have to set
    // journal.location to something else, otherwise journal_commit() will
    // not discount the number of bytes lost by this delete.
    if (is_object(ip, OT_DIR)) {
        journal_begin(i);
        fs.journal.location = 0;
        journal_commit(0);
    }
    else {
        // NOTE: This is not nice if we get a break down however the
        // remaning chunks will be removed later by a block reclaim.
        do {
            journal_begin(i);
            fs.journal.location = 0;
            journal_commit(0);
        } while ((i = segment_next(i)) != 0);
    }

    tw(tr(TR_END, TrObject, "} %d\n", EFFS_OK));

    return EFFS_OK;
}

// Rename an object. <newname> is the new name.
iref_t object_rename(iref_t oldi, const char *newname, iref_t newdir)
{
    iref_t newi;
    struct inode_s *oldip;
    char *olddata;
    int  oldsize, namelength, realsize, offset;

    tw(tr(TR_BEGIN, TrObject, "object_rename(%d, '%s', %d) {\n",
          oldi, newname, newdir));

    oldip   = inode_addr(oldi);
    oldsize = segment_datasize(oldip);

    // Make sure that there is enough space to make the rename without
    // object_create() trigger a data_reclaim() (awoid relocate oldi/data
    // source)
    namelength = is_filename(newname);
    realsize = namelength + 1 + oldsize + (oldsize > 0 ? 1 : 0);
    realsize = atomalign(realsize);

    // Save newdir in fs.xx because it will be updated if it is relocated.
    fs.i_backup  = newdir;

    if ((offset = data_prealloc(realsize)) <= 0)
        return EFFS_NOSPACE;

    // Use fs.journal.oldi because i would have been updated if
    // data_reclaim() relocate oldi
    oldip   = inode_addr(fs.journal.oldi);
    olddata = offset2addr(location2offset(oldip->location));
    olddata = addr2data(olddata, oldip);

    newi = object_create(newname, olddata, oldsize, fs.i_backup);
    
    tw(tr(TR_END, TrObject, "} %d\n", newi));
    return newi;
}


/******************************************************************************
 * Object Lookup
 ******************************************************************************/

// We can *not* use global variables, only local --- we must be re-entrant!

#if 0

// NEW CODE!

iref_t ffs_object_lookup_do(const char **path, iref_t *dir, int readlink);

iref_t ffs_object_lookup_once(const char *path, char **leaf, iref_t *dir)
{
    iref_t i, mydir;
    const char *mypath;

    tw(tr(TR_BEGIN, TrLookup, "object_lookup_once('%s', ?, ?) {\n", path));
    ttw(ttr(TTrInode, "olu(%s){" NL, path));

    mypath = path;
    mydir = 0;
    i = object_lookup_do(&mypath, &mydir, 0);
    if (leaf) *leaf = (char *) mypath;
    if (dir)  *dir  = mydir;

    tw(tr(TR_END, TrLookup, "} (%d, '%s') %d\n",
          (dir ? *dir : 0), (leaf ? *leaf : ""), i));
    ttw(ttr(TTrInode, "} %d" NL, i));

    return i;
}

// Lookup an object. Symlinks are followed.
iref_t ffs_object_lookup(const char *path, char **leaf, iref_t *dir)
{
    iref_t i, mydir;
    const char *mypath;

    tw(tr(TR_BEGIN, TrLookup, "object_lookup('%s', ?, ?) {\n", path));
    ttw(ttr(TTrInode, "olu(%s){" NL, path));

    mypath = path;
    mydir = 0;
    i = object_lookup_do(&mypath, &mydir, 1);

    if (is_object(ip, OT_LINK)) {
        // If it is a link, we unconditionally follow it
        mypath  = offset2addr(location2offset(ip->location));
        mypath += ffs_strlen(mypath) + 1; // point to data
        if (*mypath == '/') {
            mypath++;
            depth = 0;
            d = fs.root;
        }
        i = d;
        ip = inode_addr(d);
    }

    if (leaf) *leaf = (char *) mypath;
    if (dir)  *dir  = mydir;

    tw(tr(TR_END, TrLookup, "} (%d, '%s') %d\n",
          (dir ? *dir : 0), (leaf ? *leaf : ""), i));
    ttw(ttr(TTrInode, "} %d" NL, i));

    return i;
}

// NEW CODE!

// Ignore all occurrences of two successive slashes. Accept trailing slash
// in directory name.
iref_t ffs_object_lookup_do(const char **path, iref_t *dir, int followlink)
{
    // int lookup_followed;  // number of symlinks followed
    iref_t i, j, d;
    struct inode_s *ip;
    const char *p, *q, *mypath = *path;
    uint8 depth = 1;

    tw(tr(TR_FUNC, TrLookup, "object_lookup_do('%s', ?, %d) {\n",
          *path, followlink));

    d = fs.root;
    if (*mypath == '/') {
        mypath++;  // silently ignore and skip prefix slash
        // root directory is a special case
        if (*mypath == 0) {
            j = d;
            if (path) *path = mypath;
            if (dir)  *dir  = 0;
            tw(tr(TR_NULL, TrLookup, "} ('%s', %d) %d\n", mypath, 0, j));
            return j;
        }
    }
    
    // set default return value if root dir is empty (child link empty)
    j = EFFS_NOTFOUND;

    ip = inode_addr(d);

    tw(tr(TR_FUNC, TrLookup, ""));

    while ((i = ip->child) != (iref_t) IREF_NULL)
    {
        j = 0;  // default to not found
        do {
            tw(tr(TR_NULL, TrLookup, " %d", (int) i));

            p = mypath;
            ip = inode_addr(i);
            if (is_object_valid(ip) && !is_object(ip, OT_SEGMENT)) {
                q = addr2name(offset2addr(location2offset(ip->location)));
                tw(tr(TR_NULL, TrLookup, ":%s", q));
                while (*p == *q && *p != 0 && *q != 0) {
                    p++;
                    q++;
                }
                if (*q == 0 && (*p == 0 || *p == '/')) {
                    j = i;
                    break;
                }
            }
        } while ((i = ip->sibling) != (iref_t) IREF_NULL);

        if (j == 0) {
            // we did not find this component of the mypath. Let's see if this
            // was the leafname component or not...
            while (*p != 0 && *p != '/')
                p++;

            if (*p == 0)
                // The mypath component was indeed the leafname
                j = EFFS_NOTFOUND;
            else
                // The path component was not the last, so it obviously
                // contained an object that was not a directory.
                j = EFFS_NOTADIR;
            break;
        }

        if (*p == '/') {
            // if there are more path components, the object found must be a
            // directory or a symlink...
            if (is_object(ip, OT_LINK)) {
                // If it is a link, we unconditionally follow it
                mypath  = offset2addr(location2offset(ip->location));
                mypath += ffs_strlen(mypath) + 1; // point to data
                if (*mypath == '/') {
                    mypath++;
                    depth = 0;
                    d = fs.root;
                }
                i = d;
                ip = inode_addr(d);
            }
            else if (is_object(ip, OT_DIR)) {
                mypath = p + 1;
                d = i;
            }
            else {
                j = EFFS_NOTADIR;
                break;
            }
            if (++depth > fs.path_depth_max) {
                j = EFFS_PATHTOODEEP;
                break;
            }

            // if this dir inode has no children, we will leave the while
            // loop, so we preset the return error code. NOTEME: Not
            // strictly correct because if we still have a lot of the
            // pathname left, it should return the error EFFS_NOTADIR
            j = EFFS_NOTFOUND;

            tw(tr(TR_NULL, TrLookup, " /"));

        }
        else {
            // It is a fact that *p == 0. So we found the object
            if (is_object(ip, OT_LINK) && followlink) {
                // If object is a link, we conditionally follow it...
                mypath  = offset2addr(location2offset(ip->location));
                mypath += ffs_strlen(mypath) + 1; // point to data
                if (*mypath == '/') {
                    mypath++;
                    depth = 0;
                    d = fs.root;
                    i = fs.root;
                }
                else
                    i = d;
                ip = inode_addr(d);
                tw(tr(TR_NULL, TrLookup, " -%d->", d));
            }
            else {
                break; // Success, we found the object!
            }
        }
    }

    if (path) *path = (char *) mypath;
    if (dir)  *dir  = d;

    tw(tr(TR_NULL, TrLookup, "} (%d, '%s') %d\n", d, mypath, j));

    return j;
}

#else

// Lookup an object. Symlinks are followed.
iref_t object_lookup(const char *path, char **leaf, iref_t *dir)
{
    iref_t i;
    struct inode_s *ip;

    tw(tr(TR_BEGIN, TrLookup, "object_lookup('%s', ?, ?) {\n", path));
    ttw(ttr(TTrInode, "olu(%s){" NL, path));

    i = object_lookup_once(path, leaf, dir);
    ip = inode_addr(i);

    if (i > 0 && is_object(ip, OT_LINK)) {
        path = offset2addr(location2offset(ip->location));
        path += ffs_strlen(path) + 1;  // point to data portion
        i = object_lookup_once(path, leaf, dir);

        // Links may only point to regular files...
        ip = inode_addr(i);
        if (+i > 0 && !is_object(ip, OT_FILE))
            i = EFFS_NOTAFILE;
    }
    else { 
        leaf = 0;  
        dir = 0;
    }
    tw(tr(TR_END, TrLookup, "} (%d, '%s') %d\n",
          (dir ? *dir : 0), (leaf ? *leaf : ""), i));

    ttw(ttr(TTrInode, "} %d" NL, i));
    return i;
}

// Lookup an object.  If object is found: Return iref of object and
// directory of object in <dir>. If object is not found: Return
// EFFS_NOTFOUND and last directory component of path in <dir> and leafname
// of pathname in <leaf>
iref_t object_lookup_once(const char *path, char **leaf, iref_t *dir)
{
    iref_t i, j, d;
    struct inode_s *ip;
    const char *p, *q;
    uint8 depth = 1;

    tw(tr(TR_FUNC, TrLookup, "object_lookup_once('%s', ?, ?) { ", path));

    if (path == NULL) 
        return EFFS_BADNAME;

    d = fs.root;
    if (*path == '/') {
        path++;  // silently ignore and skip prefix slash
        // root directory is a special case
        if (*path == 0) {
            j = d;
            if (leaf) *leaf = (char *) path;
            if (dir) *dir = 0;
            tw(tr(TR_NULL, TrLookup, "} ('%s', %d) %d\n", path, 0, j));
            return j;
        }
    }
    else
        return EFFS_BADNAME;

    // set default return value if root dir is completely empty
    // (child link empty) 
    j = EFFS_NOTFOUND;

    ip = inode_addr(d);

    while ((i = ip->child) != (iref_t) IREF_NULL)
    {
        j = 0;  // default to not found
        do {
            tw(tr(TR_NULL, TrLookup, "i%d ", (int) i));

            p = path;
            ip = inode_addr(i);
            if (is_object_valid(ip) && !is_object(ip, OT_SEGMENT)) {
                q = addr2name(offset2addr(location2offset(ip->location)));
                tw(tr(TR_NULL, TrLookup, "%s ", q));
                while (*p == *q && *p != 0 && *q != 0) {
                    p++;
                    q++;
                }
                if (*q == 0 && (*p == 0 || *p == '/')) {
                    j = i;
                    break;
                }
            }
        } while ((i = ip->sibling) != (iref_t) IREF_NULL);


        if (j == 0) {
            // we did not find this component of the path. Let's
            // see if this was the leafname component or not...
            while (*p != 0 && *p != '/')
                p++;

            if (*p == 0)
                // The path component was indeed the leafname
                j = EFFS_NOTFOUND;
            else
                // The path component was not the last, so it
                // obviously contained an object that was not a
                // directory.
                j = EFFS_NOTADIR;
            break;
        }

        if (*p == '/') {
            // if there are more path components, the object found
            // must be a directory...
            if (!is_object(ip, OT_DIR)) {
                j = EFFS_NOTADIR;
                break;
            }
            if (++depth > fs.path_depth_max) {
                j = EFFS_PATHTOODEEP;
                break;
            }
            path = p + 1;
            d = i;

            // if this dir inode has no children, we will leave the
            // while loop, so we preset the return error code. NOTEME:
            // Not strictly correct because if we still have a lot of
            // the pathname left, it should return the error
            // EFFS_NOTADIR
            j = EFFS_NOTFOUND;

            tw(tr(TR_NULL, TrLookup, "/ "));

        }
        else {
            // It is a fact that *p == 0. So we found the object!
            break;
        }
    }

    if (leaf) *leaf = (char *) path;
    if (dir) *dir = d;

    tw(tr(TR_NULL, TrLookup, "} (%d, '%s') %d\n", d, path, j));

    return j;
}

#endif


/******************************************************************************
 * Directory Operations
 ******************************************************************************/

// Open a directory, returning the iref of the directory's inode.
iref_t dir_open(const char *name)
{
    iref_t i;
    struct inode_s *ip;

    tw(tr(TR_BEGIN, TrDirHigh, "dir_open('%s') {\n", name));

    if ((i = object_lookup(name, 0, 0)) < 0) {
        tw(tr(TR_END, TrDirHigh, "} %d\n", i));
        return i;
    }

    ip = inode_addr(i);
    if (!is_object(ip, OT_DIR))
        i = EFFS_NOTADIR;

    tw(tr(TR_END, TrDirHigh, "} %d\n", i));

    return i;
}

// Return name and iref of next entry in directory <dir>. <i> is the last
// entry we returned from this function. In case this is the first call
// after the initial call to dir_open(), <i> equals <dir>.
iref_t dir_next(iref_t dir, iref_t i, char *name, int8 size)
{
    struct inode_s *ip = inode_addr(i);
    char *p;

    tw(tr(TR_BEGIN, TrDirHigh, "dir_next(%d, %d, ?, %d) {\n", dir, i, size));

    i = (i == dir ? ip->child : ip->sibling);

    while (i != (iref_t) IREF_NULL) {
        ip = inode_addr(i);
        if (is_object_valid(ip)) {
            p = offset2addr(location2offset(ip->location));
            while (size-- && (*name++ = *p++))
                ;
            break;
        }
        i = ip->sibling;
    }
    if (i == (iref_t) IREF_NULL)
        i = 0;

    tw(tr(TR_END, TrDirHigh, "} %d\n", i));

    return i;
}

// Traverse a directory given by inode reference <i>. If <i> is negative, it
// refers to the actual directory so we start by traversing the child link.
// Otherwise if <i> is positive, it refers to an entry within the directory
// and we only traverse sibling links. Returns iref of last object in
// directory (or negative iref of directory if the child link is empty).
// <entries> is number of non-deleted objects in the dir (only valid if
// traversed from the start, eg. with negative <i>).
iref_t dir_traverse(iref_t i, iref_t *entries)
{
    iref_t j = 0, valid = 0, erased = 0, invalid = 0;
    struct inode_s *ip;

    tw(tr(TR_FUNC, TrDirLow, "dir_traverse(%d, ?) { ", i));

    if (i < 0) {
        // If directory's child is empty, this is a virgin directory and we
        // return negative iref of the directory itself.
        j = i;
        i = -i;
        ip = inode_addr(i);
        i = ip->child;
    }
    if (i != (iref_t) IREF_NULL) {
        do {
            if (j == i) {
                tw(tr(TR_NULL, TrDirLow, "LOOP! "));
                return EFFS_SIBLINGLOOP;
            }
            
            j = i;
            ip = inode_addr(j);

            tw(tr(TR_NULL, TrDirLow, "%d/%x ", j, ip->flags));

            if (is_object_valid(ip))
                valid++;
            else if (is_object(ip, OT_ERASED))
                erased++;
            else
                invalid++;

        } while ((i = ip->sibling) != (iref_t) IREF_NULL);
    }

    if (entries != 0)
        *entries = valid;

    tw(tr(TR_NULL, TrDirLow, "} (valid = %d, erased = %d, invalid = %d) %d\n",
          valid, erased, invalid, j));

    return j;
}


/******************************************************************************
 * Block, Inode and Data Allocation
 ******************************************************************************/

// Find the youngest free block. Return block index on success. If the
// argument <priority> is zero, this is a normal alloc and it will leave at
// least fs.blocks_free_min spare blocks. Otherwise, if it is non-zero, it
// is a privileged alloc (initiated by a reclaim operation) and it will not
// necessarily leave any spare blocks.
bref_t block_alloc(bref_t priority, uint16 flags)
{
    bref_t i, b, b_min, b_max, blocks_free;
    struct block_header_s *bhp;
    age_t age, age_min, age_max;

    tw(tr(TR_BEGIN, TrBlock, "block_alloc(%d, 0x%x) {\n", priority, flags));
    ttw(ttr(TTrData, "ba(%d,0x%x) {" NL, priority, flags));

    age_min = BLOCK_AGE_MAX;
    age_max = 0;
    blocks_free = 0;
    b_min = b_max = -1;

    tw(tr(TR_FUNC, TrBlock, "blocks(age): "));
    for (i = dev.numblocks - 1; i >= 0; i--)
    {
        if (is_block(i, BF_IS_FREE))
        {
	    blocks_free++;
            bhp = (struct block_header_s *) offset2addr(dev.binfo[i].offset);
	    age = bhp->age;

            tw(tr(TR_NULL, TrBlock, "%d(%d) ", i, age));

            // Remember index of block found. We use '<=' and '>=' operators
            // (instead of '<' and '>') to ensure we have both limits
            // properly set on exit from this loop.
            if (age <= age_min) {
                b_min = i;
                age_min = age;
            }
            if (age >= age_max) {
                b_max = i;
                age_max = age;
            } 
        }
    }
    tw(tr(TR_NULL, TrBlock, "\n"));

    // Handle age wrap around
    b = b_min;
    if (b_min != -1) {
        // Either age_max really is max age, so b_min is youngest block OR
        // age_max really is min age, so b_max is youngest block
        b = (age_max - age_min) < 0x8000 ? b_min : b_max;
    }
    
    // Only privileged allocs will get the last free block
    if (blocks_free <= fs.blocks_free_min - priority) {
        b = -1;
        tw(tr(TR_FUNC, TrBlock, "Only %d block(s) left, required = %d\n",
              blocks_free, fs.blocks_free_min - priority));
    }
    else {
        // Prepare/format the block for holding data/inodes
        if (flags == BF_DATA) {
            bstat[b].used = BHEADER_SIZE;
            bstat[b].lost = 0; 
            bstat[b].objects = 0;
            block_flags_write(b, BF_DATA);
        }
        else if (flags == BF_COPYING) {
            // This code is used on a fresh format and when allocating a new
            // block for reclaiming inodes
            block_flags_write(b, BF_COPYING);
            bstat[b].used = 0;
            bstat[b].lost = 0;
            bstat[b].objects = 1;  // first inode to be allocated
        }
        else {
            tw(tr(TR_FUNC, TrBlock, "FATAL: Bad input (flags = 0x%X)\n", flags));
        }
    }

    tw(tr(TR_END, TrBlock, "} (%d) %d\n", blocks_free, b));
    ttw(ttr(TTrData, "} 0x%x" NL, b));

    return b;
}

// Free and schedule a block for erase.
void block_free(bref_t b)
{
    tw(tr(TR_BEGIN, TrBlock, "block_free(%d) {\n", b));

    // mark block as invalid and schedule erasure
    block_flags_write(b, BF_LOST);
    block_reclaim(b);

    tw(tr(TR_END, TrBlock, "}\n"));
}

void block_flags_write(uint8 block, uint8 flags)
{
    struct block_header_s *bhp =
        (struct block_header_s *) offset2addr(dev.binfo[block].offset);

    tw(tr(TR_BEGIN, TrBlock, "block_flags_write(%d, 0x%x)\n", block, flags));

    bstat[block].flags = BIT_SET(bstat[block].flags, flags);
    ffsdrv.write_halfword((uint16 *) &bhp->flags, bstat[block].flags );

    tw(tr(TR_END, TrBlock, ""));
}

// Allocate an inode for a new object. We use bstat[fs.inodes].objects to
// start our scan for a free inode instead of starting from the first time
// each time.
iref_t inode_alloc(void)
{
    iref_t i;

    tw(tr(TR_BEGIN, TrInode, "inode_alloc() {\n"));
    ttw(ttr(TTrInode, "i_a() {" NL));

    if ((i = inode_alloc_try()) == 0) {
	// FIXME NO we are not always of inodes, maybe dos there exist to
	// many objects! It will not help to reclaim the inodes in that case!
        tw(tr(TR_FUNC, TrInode, "NOTE: Out of free inodes...\n"));
        inodes_reclaim();
        i = inode_alloc_try();
    }

    tw(tr(TR_END, TrInode, "} %d\n", i));
    ttw(ttr(TTrInode, "} %d" NL, i));

    return i;
}

iref_t inode_alloc_try(void)
{
    iref_t i = fs.inodes_max;
    struct inode_s *ip;

    // If we have not yet reached the maximum allowed number of objects,
    // search for next free inode...
    if (bstat[fs.inodes].used - bstat[fs.inodes].lost < fs.objects_max)
    {
        ip = inode_addr(bstat[fs.inodes].objects);
        for (i = bstat[fs.inodes].objects;
             i < fs.inodes_max - FFS_INODES_MARGIN; i++, ip++) {
            if (ip->location == FLASH_NULL32) {
                bstat[fs.inodes].objects = i;
                bstat[fs.inodes].used++;
                break;
            }
        }
    }
    if (i >= fs.inodes_max - FFS_INODES_MARGIN)
        i = 0;
        
    tw(tr(TR_FUNC, TrInode, "inode_alloc_try() %d\n", i));
    ttw(ttr(TTrInode, "i_a_t() %d" NL, i));

    return i;
}

// NOTEME: Should file data be word aligned to enable faster reads and
// writes in word quantities AND to be more compatible with the inherent
// 16-bit access width of flash memories?
offset_t data_alloc(int size)
{
    offset_t offset = 0;
    bref_t b;
  
    tw(tr(TR_BEGIN, TrData, "data_alloc(%d) {\n", size));
    ttw(ttr(TTrData, "da(%d) {" NL, size));

    offset = data_prealloc(size);

    // If we did allocate the space, we update bstat[]
    if (offset > 0) {
	b = offset2block(offset);
	bstat[b].used += size;
	stats.data_allocated += size; // STATS
    }

    tw(tr(TR_END, TrData, "} 0x%04x\n", offset));
    ttw(ttr(TTrData, "} %x" NL, offset));

    return offset;
}

offset_t data_prealloc(int realsize)
{
    int result, i, bytes_free;
    offset_t offset;

    // Is it possible to get this amount of free space and still have enough
    // reserved space?
    ffs_query(Q_BYTES_FREE_RAW, &bytes_free);
    if (realsize > (bytes_free + FFS_FILENAME_MAX + dev.atomsize))
        return 0;    // Not enough unused space

    for (i = 0; i < dev.numblocks; i++) {
        if ((offset = data_alloc_try(realsize)) > 0)
            return offset;  // Space found

        if ((result = data_reclaim(realsize)) < 0)
            return 0;  // Data reclaim failed!
    }
    
    return 0;  // No space found
}

// Find free data space of size <size>. Return zero if no space available.
// Note that we ensure that we always have space immediately available for a
// privileged data_alloc(), e.g. a data_alloc() that allocates data space
// without performing a data_reclaim(). This is important when
// re-creating/re-locating the journal file.
offset_t data_alloc_try(int size)
{
    bref_t b;
    int free;
    offset_t offset_big = 0, offset_small = 0;
    int size_big_ok = 0, size_small_ok = 0;
    int size_big, size_small;
    int reserved;

    tw(tr(TR_FUNC, TrData, "data_alloc_try(%d) { ", size));
    ttw(ttr(TTrData, "dat(%d) {" NL, size));

    // NOTE when we alloc do we only need to have reserved space for X
    // number of journal files, where X is the max number of used journals
    // per data reclaim. The only exception is when an object_relocate has
    // failed thus we set reserved_space to zero.
    reserved = RESERVED_LOW;

    if (fs.reserved_space < reserved)
	reserved = fs.reserved_space;

    // Set size_big to the grater of the sizes and size_small to the lesser.
    size_big   = (size > reserved ? size : reserved);
    size_small = (size > reserved ? reserved : size);
    tw(tr(TR_NULL, TrData, "(size_big, small = %d, %d) ", size_big, size_small));

    // First search for free space in data blocks
    tw(tr(TR_NULL, TrData, "block:free,objects: "));

    for (b = 0; b < dev.numblocks; b++) {
        if (is_block(b, BF_IS_DATA)) {
            free = dev.blocksize - bstat[b].used;
            tw(tr(TR_NULL, TrData, "%d:%d,%d ", b, free, bstat[b].objects));
            if (bstat[b].objects < fs.block_files_max - fs.block_files_reserved) {
                if (!size_big_ok && !size_small_ok && 
                    (free >= size_big + size_small)) {
                    size_big_ok = size_small_ok = 1;
                    offset_big = offset_small =
                        dev.binfo[b].offset + bstat[b].used;
                    tw(tr(TR_NULL, TrData, "big/small_ok "));
                    break;
                }
                else if (!size_big_ok && free >= size_big) {
                    size_big_ok = 1;
                    offset_big = dev.binfo[b].offset + bstat[b].used;
                    tw(tr(TR_NULL, TrData, "big_ok "));
                }
                else if (!size_small_ok && free >= size_small) {
                    size_small_ok = 1;
                    offset_small = dev.binfo[b].offset + bstat[b].used;
                    tw(tr(TR_NULL, TrData, "small_ok "));
                }
            }
        } 
        if (size_small_ok && size_big_ok)
            break;
    }

    if (size_big_ok && size_small_ok)
        offset_big = (size > reserved ? offset_big : offset_small);
    else
        offset_big = 0;

    tw(tr(TR_NULL, TrData, "} 0x%x\n", offset_big));
    ttw(ttr(TTrData, "} %x " NL, offset_big));

    return offset_big;
}

offset_t data_reserved_alloc(int size)
{
    bref_t b;
    offset_t offset = 0;
    int free;

    tw(tr(TR_BEGIN, TrData, "data_reserved_alloc(%d) {\n", size));
    ttw(ttr(TTrData, "dra(%d) {" NL, size));

    tw(tr(TR_NULL, TrData, "block:free,objects: "));
    for (b = 0; b < dev.numblocks; b++) {
        if (is_block(b, BF_IS_DATA)) {
            free = dev.blocksize - bstat[b].used;
            tw(tr(TR_NULL, TrData, "%d:%d,%d ", b, free, bstat[b].objects));
            if (free >= size) {
                offset = dev.binfo[b].offset + bstat[b].used;
                break;
            }
        } 
    }

    // If we did allocate the space, we update bstat[]
    if (offset != 0) {
        b = offset2block(offset);
        bstat[b].used += size;
        stats.data_allocated += size; // STATS
    }

    tw(tr(TR_END, TrData, "} 0x%04x\n", offset));
    ttw(ttr(TTrData, "} %x" NL, offset));

    return offset;
}


iref_t chunk_alloc(int realsize, int is_journal, offset_t *offset) 
{
    iref_t i;

    if (realsize < 0)
        return EFFS_INVALID;

    // Have we reached objects_max? We make a similar test in
    // inode_alloc_try(), however we need to do it here or else we risk to start
    // a data_reclaim we not can finish.
    if (bstat[fs.inodes].used - bstat[fs.inodes].lost >= fs.objects_max) {
	tw(tr(TR_END, TrObject, "} %d\n", EFFS_FSFULL));
        ttw(ttr(TTrObj, "} %d" NL, EFFS_FSFULL));
        return EFFS_FSFULL;
    }

    // Allocate space for the object name (and object data)
    if (is_journal)
        *offset = data_reserved_alloc(realsize);
    else
        *offset = data_alloc(realsize);

    if (*offset == 0) {
        tw(tr(TR_END, TrObject, "} %d\n", EFFS_NOSPACE));
        ttw(ttr(TTrObj, "} %d" NL, EFFS_NOSPACE));
        return EFFS_NOSPACE;
    }
    fs.journal.location = offset2location(*offset);

    // Allocate an inode for the object
    i = fs.journal.i = inode_alloc();
    if (i == 0) {
        tw(tr(TR_END, TrObject, "} %d\n", EFFS_FSFULL));
        ttw(ttr(TTrObj, "} %d" NL, EFFS_FSFULL));
        return EFFS_FSFULL;
    }

    return i;
}

/******************************************************************************
 * query and fcontrol
 ******************************************************************************/

#if 0
extern uint16 ffs_flash_device;
extern uint16 ffs_flash_manufact;
#endif

effs_t object_control(iref_t i, int8 action, int value)
{
    effs_t error = EFFS_OK;

    tw(tr(TR_BEGIN, TrOther, "object_control(%d, %d, 0x%x) {\n",
          i, action, value));
    ttw(ttr(TTrApi, "obj_control(%d,%d,0x%x)" NL, i, action, value));

    switch (action) {
    case OC_FLAGS:
        // Set/clear object flags. Attempting to modify the "/dev/ffs"
        // object (i = 0) or any non-defined flags is an invalid operation.
        if (i <= 0 || value & ~OF_ALL) {
            error = EFFS_INVALID;
        }
        else {
            // there are two cases; either we only set bits in the flags.
            // This is simple, as we just have to update the flags byte. The
            // other case is harder because we have to clear bits and for
            // this we have to copy the old inode to a new inode, setting
            // the flags appropriately. For now we always just allocate a
            // new inode and set the flags according to the <value>
            // argument.
            journal_begin(i);
            fs.journal.flags |= OF_MASK; // reset all flags
            fs.journal.flags = BIT_SET(fs.journal.flags, value);
            if ((fs.journal.i = inode_alloc()) == 0)
                error = EFFS_FSFULL;
            else {
                fs.journal.diri = dir_traverse(fs.journal.diri, 0);
                journal_end(0);
            }
        }
        break;

    case OC_FS_FLAGS:      fs.flags           = value; break;
#if 0
    case OC_DEV_MANUFACT:  ffs_flash_manufact = value; break;
    case OC_DEV_DEVICE:    ffs_flash_device   = value; break;
#endif
    case OC_FS_TESTFLAGS:  fs.testflags       = value; break;
    case OC_DEBUG_0:
    case OC_DEBUG_1:
    case OC_DEBUG_2:
    case OC_DEBUG_3:       fs.debug[action - OC_DEBUG_FIRST] = value; break;
    case OC_TRACE_INIT:
#if (TARGET == 1)
        ttr_init(value);
#endif
        break;
    default:
        error = EFFS_INVALID;
    }

    tw(tr(TR_END, TrOther, "} %d\n", error));

    return error;
}

extern int tmffs_bufsize(void); // used by ffs_query()
extern unsigned char *tmffs_bufaddr(void); // used by ffs_query()

#if (TARGET == 1)
// request_id_last is only used in TARGET not to any use on the PC side
extern req_id_t request_id_last;   // from task.c
#else
req_id_t request_id_last;
#endif

// If tmffs not is represented we define a dummy tm version
#ifndef FFS_TM_VERSION 
#define FFS_TM_VERSION   ((uint16) 0x0BAD)
#endif

effs_t ffs_query(int8 query, void *p)
{
    tw(tr(TR_FUNC, TrOther, "query(%d) (?)\n", query));

    if (p == NULL)
        return EFFS_INVALID;

    switch (query)
    {
    case Q_BYTES_FREE:
    case Q_BYTES_USED:
    case Q_BYTES_LOST:
    case Q_BYTES_MAX:
    case Q_OBJECTS_TOTAL:
    case Q_BLOCKS_FREE:
    case Q_BYTES_FREE_RAW:
    {
        bref_t b;
        bref_t blocks_free = 0;
        iref_t objects = 0;
        offset_t max, used = 0, lost = 0;
        struct block_stat_s *bp;

	// Don't count free blocks, inode block, block header and reserved space.
	max = (dev.numblocks - fs.blocks_free_min - 1) * 
            (dev.blocksize - BHEADER_SIZE) - fs.reserved_space;

        // Furthermore don't count the ovewrhead from each chunk (alignment)
        // NOTE: If we call query while FFS not is formatted there is a risk
        // of deviding with zero!
        if (fs.chunk_size_max > 0)
            max -= ((max / fs.chunk_size_max + 1) * dev.atomsize);

        for (b = 0, bp = &bstat[0]; b < dev.numblocks; b++, bp++) {
            if (is_block(b, BF_IS_FREE))
                blocks_free++;
            if (is_block(b, BF_IS_DATA)) {
                objects += bp->objects;
                used    += bp->used;
                lost    += bp->lost;
            }
        }

        switch (query) {
        case Q_BYTES_FREE:    *(uint32*)p = max - (used - lost) - FFS_FILENAME_MAX; 
            break;
        case Q_BYTES_FREE_RAW:*(uint32*)p = max - (used - lost); break;
        case Q_BYTES_USED:    *(uint32*)p = used; break;
        case Q_BYTES_LOST:    *(uint32*)p = lost; break;
        case Q_BYTES_MAX:     *(uint32*)p = max; break;
        case Q_OBJECTS_TOTAL: *(uint16*)p = objects; break;
        case Q_BLOCKS_FREE:   *(uint16*)p = blocks_free; break;
        }
        break;
    }

    case Q_TM_BUFADDR:       *(uint32*)p = (uint32) tmffs_bufaddr(); break;
    case Q_TM_BUFSIZE:       *(uint32*)p = tmffs_bufsize(); break;
    case Q_DEV_BASE:         *(uint32*)p = (uint32) dev.base; break;

	// FFS versions
    case Q_FFS_API_VERSION:  *(uint16*)p = FFS_API_VERSION; break;
    case Q_FFS_DRV_VERSION:  *(uint16*)p = FFS_DRV_VERSION; break;
    case Q_FFS_REVISION:     *(uint16*)p = ffs_revision; break;
    case Q_FFS_FORMAT_WRITE: *(uint16*)p = FFS_FORMAT_VERSION; break;
    case Q_FFS_FORMAT_READ:  *(uint16*)p = fs.format; break;
    case Q_FFS_LASTERROR:    *(int16*)p  = fs.initerror; break;
    case Q_FFS_TM_VERSION:   *(int16*)p  = FFS_TM_VERSION; break;

	// File system queries
    case Q_FILENAME_MAX:     *(uint16*)p = fs.filename_max; break;
    case Q_PATH_DEPTH_MAX:   *(uint16*)p = fs.path_depth_max; break;

    case Q_OBJECTS_FREE:     *(uint16*)p = fs.objects_max - 
                                 (bstat[fs.inodes].used -
                                  bstat[fs.inodes].lost); break;
    case Q_INODES_USED:      *(uint16*)p = bstat[fs.inodes].used; break;
    case Q_INODES_LOST:      *(uint16*)p = bstat[fs.inodes].lost; break;
    case Q_OBJECTS_MAX:      *(uint16*)p = fs.objects_max; break;

    case Q_INODES_MAX:       *(uint16*)p = fs.inodes_max; break;
    case Q_CHUNK_SIZE_MAX:   *(uint16*)p = fs.chunk_size_max; break;

	// File descriptor queris 
    case Q_FD_BUF_SIZE:      *(uint32*)p = fs.fd_buf_size; break;  
    case Q_FD_MAX:           *(uint16*)p = fs.fd_max; break;

	// device queries
    case Q_DEV_MANUFACTURER: *(uint16*)p = dev.manufact; break;
    case Q_DEV_DEVICE:       *(uint16*)p = dev.device; break;
    case Q_DEV_BLOCKS:       *(uint16*)p = dev.numblocks; break;
    case Q_DEV_ATOMSIZE:     *(uint16*)p = dev.atomsize; break;
    case Q_DEV_DRIVER:       *(uint16*)p = dev.driver; break;

	// Miscellaneous/Internal
    case Q_BLOCKS_FREE_MIN:  *(uint16*)p = fs.blocks_free_min; break;
    case Q_LOST_HIGH:        *(uint16*)p = fs.lost_threshold; break;

	// Debug queries
    case Q_FS_FLAGS:         *(uint16*)p = fs.flags; break;
    case Q_FS_INODES:        *(uint16*)p = fs.inodes; break;
    case Q_FS_ROOT:          *(uint16*)p = fs.root; break;

    case Q_STATS_DRECLAIMS:        *(uint32*)p = stats.drec.most_lost + 
                                       stats.drec.most_unused + 
                                       stats.drec.youngest; break;
    case Q_STATS_IRECLAIMS:        *(uint32*)p = stats.irec.num; break;
    case Q_STATS_DATA_RECLAIMED:   *(uint32*)p = stats.drec.valid[0] + 
                                       stats.drec.lost[0]; break;
    case Q_STATS_INODES_RECLAIMED: *(uint32*)p = stats.irec.valid + stats.irec.lost;
        break;
    case Q_STATS_DATA_ALLOCATED:   *(uint32*)p = stats.data_allocated; break;
    case Q_REQUEST_ID_LAST:        *(uint32*)p = request_id_last; break;

    default:
        if (query >= Q_BSTAT && (query - Q_BSTAT) < dev.numblocks)
        {
            struct block_header_s *bhp;
            uint32 *myp = p;
            
            query -= Q_BSTAT;
            bhp = (struct block_header_s *) offset2addr(dev.binfo[query].offset);

            *myp++ = bstat[query].used;
            *myp++ = bstat[query].lost;
            // If we are in READ mode or this block is not lost, we can
            // safely read the age. Otherwise it is maybe currently erasing
            // and thus we cannot read the age.
	    // NOTEME: Should this not have been handled by a driver function?
            if (dev.state == DEV_READ || !is_block_flag(query, BF_LOST))
                *myp++ = (bhp->age << 16) | bstat[query].flags;
            else 
                *myp++ = (  0xFFFE << 16) | bstat[query].flags;
            *myp++ = bstat[query].objects;
        }
        else if (query >= Q_DEBUG_FIRST && query < Q_DEBUG_LAST) {
            *(uint32*)p = fs.debug[query - Q_DEBUG_FIRST];
        }
        else
            return EFFS_INVALID;
    }

    return EFFS_OK;
}


/******************************************************************************
 * Miscellaneous Helper Functions
 ******************************************************************************/

// Check if an object is read-only. Note that the root inode is always
// readonly, no matter what! Returns error or original <i>.
iref_t is_readonly(iref_t i, const char *path)
{
    struct inode_s *ip = inode_addr(i);

    tw(tr(TR_FUNC, TrObject, "is_readonly(%d, '%s') ", i, path));

    if (i == fs.root || i == fs.ijournal ||
        (IS_BIT_SET(ip->flags, OF_READONLY) && !ffs_is_modifiable(path)))
        i = EFFS_ACCESS;

    tw(tr(TR_NULL, TrObject, "(0x%X) %d\n", ip->flags, i));

    return i;
}


// Check if filename is valid. Return EFFS_BADNAME if name contains
// invalid chars. Return RFFS_NAMETOOLONG if name is too
// long. Otherwise return filename length.
effs_t is_filename(const char *s)
{
    char *p = (char *) s;
    int n = 0;
    
    while ( (*s >= 'a' && *s <= 'z') ||
            (*s >= 'A' && *s <= 'Z') ||
            (*s >= '0' && *s <= '9') ||
            *s == '.' ||
            *s == ',' ||
            *s == '_' ||
            *s == '-' ||
            *s == '+' ||
            *s == '%' ||
            *s == '$' ||
            *s == '#' )
    {
        s++;
    }
    
    if (*s != 0)
        n = EFFS_BADNAME;  // invalid file name character found
    else {
        n = s - p;
        if (n > fs.filename_max)
            n = EFFS_NAMETOOLONG;
        if (n == 0)
            n = EFFS_BADNAME;
    }

    tw(tr(TR_FUNC, TrUtil, "is_filename('%s') %d\n", p, n));

    return n;
}

int ffs_strlen(const char *s)
{
    const char *p = s;

    while (*p++)
        ;

    tw(tr(TR_FUNC, TrUtil, "strlen('%s') %d\n", s, p-s-1));

    return p-s-1;
}

// Return zero if strings are equal, otherwise return non-zero.
int ffs_strcmp(const char *s, const char *p)
{
    int8 n = 1;

    tw(tr(TR_FUNC, TrUtil, "strcmp('%s', '%s') ", s, p));

    while (*s == *p && *p != 0) {
        s++;
        p++;
    }
    if (*s == *p)
        n = 0;

    tw(tr(TR_NULL, TrUtil, "(%d)\n", n));

    return n;
}

// Note: rename function? like get_fdi..
fd_t get_fdi(iref_t i)
{
    int j;

    tw(tr(TR_FUNC, TrUtil, "get_fdi(%d)\n", i));
    
    if (i > 0) {
	for (j = 0; j < fs.fd_max; j++) {
	    if (i == fs.fd[j].seghead) {
		return j;  // Return fdi without offset
	    }
	}
    }
    return -1;
}    


effs_t is_fd_valid(fd_t fdi)
{
    if (fdi >= fs.fd_max || fdi < 0 || fs.fd[fdi].options == 0)
        return 0;  // Not valid!
    return 1;
}

effs_t is_offset_in_buf(int offset, fd_t fdi)
{
    if (fs.fd[fdi].dirty == 1)
        if (offset >= fs.fd[fdi].wfp && 
            offset < fs.fd[fdi].wfp + fs.chunk_size_max)
        return 1;
    return 0;
}

/******************************************************************************
 * Chunk Operations
 ******************************************************************************/

iref_t segment_create(const char *buf, int size, iref_t dir)
{
    iref_t i;
    struct inode_s *ip;
    int realsize;
    offset_t offset;
    char *dataaddr;

    ttw(ttr(TTrObj, "segc(%d, %d){" NL, size, dir));
    tw(tr(TR_BEGIN, TrObject, "segment_create( ?, %d, %d) {\n", size, dir));

    fs.journal.size = realsize = atomalign(size + 1);

    // Init journal.diri before chunk_alloc() because it might trigger a
    // data_reclaim() which can relocate the dir inode
    fs.journal.diri = dir;

    if ((i = chunk_alloc(realsize, 0, &offset)) < 0)
        return i;

    ip = inode_addr(i);
    dataaddr = offset2addr(offset);

    // Write data and null terminator.  We null-terminate the data block,
    // such that blocks_fsck() can determine the amount of used data block
    // space correctly. 
    ffsdrv.write(dataaddr, buf, size);
    dataaddr += size;
    ffsdrv_write_byte(dataaddr, 0);
   
    // Segments is linked together by the child link(create) or by the
    // sibling link(update or relocate). A negativ dir indicate that it is a
    // update or relocate and the sign must be reversed so the journal
    // system will use the sibling link to link the inode together. 
    if (dir > 0)
        fs.journal.diri = chunk_traverse(fs.journal.diri);
    
    fs.journal.diri = -fs.journal.diri;
   
    tw(tr(TR_END, TrObject, "} %d\n", i));
    ttw(ttr(TTrObj, "} %d" NL,i));

    return i;
}


int segment_read(iref_t i, char *buf, int size, int offset)
{
    struct inode_s *ip;
    char *p;
    int chunk_size;

    tw(tr(TR_BEGIN, TrObject, "segment_read(%d, 0x%x, %d, %d) {\n",
       i, buf, offset, size));

    if (buf == NULL) {
        tw(tr(TR_END, TrObject, "} %d\n", EFFS_INVALID));
        return EFFS_INVALID;
    }

    ip = inode_addr(i);
    
    chunk_size = segment_datasize(ip);
    
    // Saturate read buffer
    if (size > chunk_size - offset)
        size = chunk_size - offset;
   
    p = offset2addr(location2offset(ip->location));    
    p = addr2data(p, ip);

    memcpy(buf, &p[offset], size);

    tw(tr(TR_END, TrObject, "} %d\n", size));
    return size;
}

// Find next valid chunk
iref_t segment_next(iref_t i)
{
    struct inode_s *ip = inode_addr(i);
    
    tw(tr(TR_BEGIN, TrDirHigh, "ffs_segment_next(%d) {\n", i));

    // Dir is not allowed to contain data
    if (is_object(ip, OT_DIR)) {
        tw(tr(TR_END, TrDirHigh, "} 0\n"));
        return 0;
    }

    // Is this the last/only segment
    if ((i = ip->child) == (iref_t) IREF_NULL) {
        tw(tr(TR_END, TrDirHigh, "} 0\n"));
        return 0;
    }

    // Get child (is valid?), search though segment by sibling link(is
    // valid?), and again..
    do {
        i = ip->child;
        ip = inode_addr(i);
        if (is_object_valid(ip)) {
            tw(tr(TR_END, TrDirHigh,"} %d\n", i));
            return i;
        }

        while (ip->sibling != (iref_t) IREF_NULL) {  
            i = ip->sibling;
            ip = inode_addr(i);
            if (is_object_valid(ip)) {
                tw(tr(TR_END, TrDirHigh,"} %d\n", i));
                return i;
            }
        }
    } while (ip->child != (iref_t) IREF_NULL); 

    // No segment found
    tw(tr(TR_END, TrDirHigh,"} %d\n", i));
    return 0;
}

// The output "inode" will be the inode that contains the requested data or
// the last inode in the segmentfile. The segmenthead will be skiped if it
// don't contains any data. inode_offset is the offset in the found inode
// pointed to by target_offset. If target_offset point past the last segment
// will inode_offset be the size of the last inode. The return value will be
// the same as target_offset but maximum the total size of the object.
int segfile_seek(iref_t seghead, int target_offset, 
                     iref_t *inode, int *inode_offset)
{
    int priv_count = 0, count_size = 0;
    iref_t i = seghead;
    struct inode_s *ip;

    tw(tr(TR_BEGIN, TrObject, "segfile_seek(%d, %d, ?, ?) {\n", 
          seghead, target_offset));
    
    if (!is_object_valid(inode_addr(seghead))) {
        tw(tr(TR_END, TrAll, "FATAL: Invalid seghead!\n"));
        return 0;
    }
    *inode = seghead;

    while (1)
    {
        ip = inode_addr(i);
        count_size += segment_datasize(ip);
        
        // Seghead will be skiped if it don't contain any data  
        if (count_size > target_offset && count_size != 0) {

            if (inode_offset != 0)
                *inode_offset = target_offset - priv_count;

            tw(tr(TR_END, TrObject, "} %d\n", target_offset));
            return target_offset;
        }

        if ((i = segment_next(i)) == 0) {
            tw(tr(TR_END, TrObject, "} (eof!?) %d\n", count_size));
            if (inode_offset != 0)
                *inode_offset = count_size - priv_count;
            // *inode = 0;
            return count_size; // No more segments
        }
        priv_count = count_size;

        *inode = i;
    }
}

// Calculate exact size of file data; without filename and null terminator
// and without data null terminator and succeeding alignment padding.
// NOTEME: Does this also work for empty files and directories?
int segment_datasize(const struct inode_s *ip)
{
    char *p, *q;
    int size;

    p = offset2addr(location2offset(ip->location));
    q = p + ip->size - 1;

    // Segments is not allowed to contain any name
    if (!is_object(ip, OT_SEGMENT)) {
        // skip filename at start of data
        while (*p)
            p++;
    }
    else 
        // If it contained a name would p pointe to the null terminator of
        // the name but because chunks don't have a name decrement we p to get
        // the size correct
        p--;
    
    // skip padding at end of data
    while (*q)
        q--;
    
    // If there are data, there is also a null-terminator. Otherwise
    // there is no null-terminator
    size = q - p;
    if (size > 0)
        size--;

    tw(tr(TR_FUNC, TrObject, "segment_datasize(0x%x) %d\n", ip, size));
    return size;
}


int object_truncate(const char *pathname, fd_t fdi, offset_t length)
{
    int segment_offset, flength, realsize, offset;
    iref_t i, dir, next_i;
    char *name = 0, *olddata;
    struct inode_s *oldip;
    effs_t error; 
    
    tw(tr(TR_FUNC, TrObject, "ffs_object_truncate('%s', %d, %d) \n", 
          pathname, fdi, length));
    if (length < 0) return EFFS_INVALID;
    
    if (pathname == 0) {
        // File descriptor must be open and it have to be in write mode
        if (!is_fd_valid(fdi)) 
            return EFFS_BADFD;;
       
        if (!is_open_option(fs.fd[fdi].options, FFS_O_WRONLY))
            return EFFS_INVALID; 
          
        // It is not possible to truncate an open file to a size less than
        // the current file pointer
        if (length < fs.fd[fdi].fp)
            return EFFS_INVALID;

        i = fs.fd[fdi].seghead;
    }
    else {
        // File must exists and not be open
        if ((i = object_lookup(pathname, &name, &dir)) < 0)
            return i;

        if (get_fdi(i) >= 0) 
            return EFFS_LOCKED;

        oldip = inode_addr(i);
        // Even though the ffs architecture allows to have data in directory
        // objects, we don't want to complicate matters, so we return an error
        if (is_object(oldip, OT_DIR) && !(fs.flags & FS_DIR_DATA)) 
            return EFFS_NOTAFILE;
        
        if ((i = is_readonly(i, pathname)) < 0) 
            return i;
    }
    // Find the segment which length points in to
    flength = segfile_seek(i, length, &i, &segment_offset);

    if (pathname == 0) {
        if (is_offset_in_buf(length, fdi) == 1) {
            fs.fd[fdi].size = (length > fs.fd[fdi].size ? 
                               fs.fd[fdi].size : length); // Truncate the buffer 
            
            if (i == fs.fd[fdi].wch) {
                next_i = segment_next(i);
                if (next_i > 0) 
                    if ((error = object_remove(next_i)) < 0)
                        return error;
            }
            return EFFS_OK;
        } 
    }
    
    if (flength < length)
        return EFFS_OK;
    
    journal_begin(i);
    
    // Realsize do not always need to include a name but we simplify it. 
    realsize = atomalign(segment_offset + 1 + fs.filename_max + 1); 

    // Make sure that there is enough space to make the rename without
    // object_create() trigger a data_reclaim() (awoid relocate oldi/data
    // source)
    if ((offset = data_prealloc(realsize)) <= 0)
        return EFFS_NOSPACE;

    // Find the next segment if any.
    next_i = segment_next(fs.journal.oldi);
    
    // Find old data source
    oldip = inode_addr(fs.journal.oldi);
    olddata = offset2addr(location2offset(oldip->location));
    name = addr2name(olddata);   // reinit name (maybe relocated)
    olddata = addr2data(olddata, oldip);
    
    if (is_object(oldip, OT_SEGMENT)) {
        if (segment_offset == 0)
            next_i = fs.journal.oldi; // Remove the found object
        else {
            if ((i = segment_create(olddata, segment_offset, 
                                        -fs.journal.oldi)) < 0)
                return i;
            
            fs.link_child = 0;  //Do not link child
            journal_end(0);
        }
    }
    else {
        if ((i = object_create(name, olddata, length, fs.journal.oldi)) < 0)
            return i;
        fs.link_child = 0;  //Do not link child
        journal_end(0);
        
        if (is_fd_valid(fdi))
            fs.fd[fdi].seghead = i;
    }

    if (is_fd_valid(fdi))
        fs.fd[fdi].size = length;

    // If any remaning segment exists then remove them 
    if (next_i > 0) 
        if ((error = object_remove(next_i)) < 0)
            return error;

    return EFFS_OK;
}


// Find the last segment valid or not valid
iref_t chunk_traverse(iref_t i)
{
    struct inode_s *ip = inode_addr(i);
    
    tw(tr(TR_BEGIN, TrDirHigh, "ffs_chunk_traverse(%d) {\n", i));
    // Is this the last/only segment?
    if (ip->child == (iref_t) IREF_NULL) {
        tw(tr(TR_END, TrDirHigh, "} %d\n", i));
        return i;
    }
        
    // Get child, find the last segment by sibling link, and again..
    do {
        i = ip->child;
        ip = inode_addr(i);

        while (ip->sibling != (iref_t) IREF_NULL) {  
            i = ip->sibling;
            ip = inode_addr(i);
        }
    } while (ip->child != (iref_t) IREF_NULL); 

    tw(tr(TR_END, TrDirHigh, "} %d\n", i));

    return i;
}

// fdi include offset now but change this so core use pure fdi.
effs_t datasync(fd_t fdi) 
{
    int chunk_size;
    iref_t i;
    struct inode_s *ip;
    char *name;

    tw(tr(TR_FUNC, TrObject, "datasync(%d) \n", fdi));
    ttw(ttr(TTrApi, "datasync(%d) {" NL, fdi)); 

    // NOTEME: is this necessary?
    if (!is_fd_valid(fdi))
        return EFFS_BADFD;

    if (fs.fd[fdi].dirty == 0) 
        return EFFS_OK;

    // If size - wfp is more than max is the complete buffer valid or else
    // is it only a part of it that consist valid data
        chunk_size = fs.fd[fdi].size - fs.fd[fdi].wfp;
        if (chunk_size > fs.chunk_size_max)
            chunk_size = fs.chunk_size_max;

      ip = inode_addr(fs.fd[fdi].wch);        

    // Create new chunk or update a old one
    if (fs.fd[fdi].wch > 0) {
        // Update existing chunk
        // Negativ dir input because it is a update (do not traverse)
        if (is_object(ip, OT_SEGMENT)) {
            journal_begin(fs.fd[fdi].wch);
    
            if ((i = segment_create(fs.fd[fdi].buf, chunk_size,
                                   -fs.fd[fdi].wch)) < 0)
                return i;
        }

        else {
            // Seghead update (like a normal file)
            ip = inode_addr(fs.fd[fdi].seghead);        
            name = addr2name(offset2addr(location2offset(ip->location)));
            journal_begin(fs.fd[fdi].seghead);

            if ((i = object_create(name, fs.fd[fdi].buf, chunk_size, 
                                      fs.fd[fdi].seghead)) < 0)
                return i;
                
            fs.fd[fdi].seghead = i;
        }
        journal_end(0);
    }        

    else {
        // Create new chunk at the end of the existing ones.
        // BTW: A seghead will always have been made before this one.
        journal_begin(0);

        if ((i = segment_create(fs.fd[fdi].buf, chunk_size, 
                                    fs.fd[fdi].seghead)) < 0)
            return i;

        journal_end(OT_SEGMENT);
    }
    fs.fd[fdi].dirty = fs.fd[fdi].wch = 0;

    ttw(ttr(TTrApi, "} 0" NL)); 
    return EFFS_OK;
}

/******************************************************************************
 * Development and Tracing
 ******************************************************************************/

#if (TARGET == 0)

void tr_bstat(void)
{
    int i, n;
    struct block_header_s *bhp;
    struct block_stat_s *bsp;

    tw(tr(TR_BEGIN, TrBstat, "bstat = {\n"));

    bsp = &bstat[0];
    tw(tr(TR_FUNC, TrBstat,
          "   bf   used   lost   free   n   age state\n"));
    for (i = 0, n = 0; i < dev.numblocks; i++, bsp++) {
        bhp = (struct block_header_s *) offset2addr(dev.binfo[i].offset);
        tw(tr(TR_FUNC, TrBstat, "%2d %02x %6d %6d %6d %3d %5d %s%s%s%s%s%s\n",
              i, bsp->flags & 0xFF,
              bsp->used, bsp->lost, 
              dev.blocksize - bsp->used,
              bsp->objects,
              bhp->age,
              (is_block(i, BF_IS_FREE)     ? "FREE " : ""),
              (is_block(i, BF_IS_DATA)     ? "DATA " : ""),
              (is_block(i, BF_IS_CLEANING) ? "CLEANING " : ""),
              (is_block(i, BF_IS_COPYING)  ? "COPYING " : ""),
              (is_block(i, BF_IS_INODES)   ? "INODES " : ""),
              (is_block_flag(i, BF_LOST)   ? "lost " : "")
            ));
        if (is_block(i, BF_IS_DATA))
            n += bsp->objects;
    }
    i = bstat[fs.inodes].used - bstat[fs.inodes].lost;
    tw(tr(TR_FUNC, TrBstat,
          "                           %3d (used-lost = %d)\n",
          n, i));

    if (n != i) {
        tw(tr(TR_FUNC, TrAll, "WARNING: sum(bstat[x].objects) != bstat[fs.inodes].used - bstat[fs.inodes].lost\n"));
    }

    tw(tr(TR_END, TrBstat, "}\n"));
}

#else // (TARGET == 1)

void tr_bstat(void)
{
    int i;
    struct block_stat_s *bsp = &bstat[0];

    for (i = 0; i < dev.numblocks; i++, bsp++) {
        ttw(ttr(TTrBstat, "%2d (%2x) u/l/f/n %6d %6d %6d %2d" NL,
           i, bsp->flags,
           bsp->used, bsp->lost, 
           dev.blocksize - bsp->used,
           bsp->objects
            ));
    }
    ttw(str(TTrBstat,"" NL));
}


void tr_fd(fd_t fdi)
{
    tw(tr(TR_BEGIN, TrHelper, "tr_fd(%d) {\n", fdi));
    tw(tr(TR_FUNC,  TrHelper, "options: 0x%x \n", fd[fdi].options));
    tw(tr(TR_FUNC,  TrHelper, "inode  : %d   \n", fd[fdi].inode_first)); 
    tw(tr(TR_FUNC,  TrHelper, "fp     : %d   \n", fd[fdi].fp)); 
    tw(tr(TR_FUNC,  TrHelper, "size   : %d   \n", fd[fdi].size));
    tw(tr(TR_FUNC,  TrHelper, "dir    : %d   \n", fd[fdi].dir)); 
    tw(tr(TR_FUNC,  TrHelper, "name   : %s   \n", fd[fdi].name));
    tw(tr(TR_END,   TrHelper, "}\n", fdi));
}

#endif // (TARGET == 0)
