/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * ffs public API functions
 *
 * $Id: ffs.c 1.69.1.24.1.40 Thu, 08 Jan 2004 15:05:23 +0100 tsj $
 *
f ******************************************************************************/

#include "ffs.h"

#if ((TARGET == 1) || (RIV_ENV== 1))
#include "task.h"
#endif

#if (TARGET == 0) 
#include <stdlib.h>
#endif

#include <string.h>
#include <limits.h>

#include "core.h"
#include "ffstrace.h"

/******************************************************************************
 *
 ******************************************************************************/

extern struct fs_s fs; // defined in core.c

// These dummy defines and struct are only use to simulate FFS on the
// PC. The ones that is used in target are located in task.h
#if (TARGET == 0)
struct ffs_blocking_s {int x; };
#define FFS_BLOCKING_CALL_BEGIN()
    int result; \
    struct ffs_blocking_s fb; 
#define FFS_BLOCKING_CALL_END()    
#endif

/******************************************************************************
 * Create, Read and Write
 ******************************************************************************/

req_id_t ffs_file_write_b(const char *pathname, void *src, int size, 
                        ffs_options_t option, T_RV_RETURN *cp, 
                        struct ffs_blocking_s *fb)
{
    iref_t i, dir;
    char *name;
    effs_t error;
    int chunk_size, size_remaining, bytes_free;

    tw(tr(TR_FUNC, TrApi, "ffs_file_write('%s', 0x%x, %d, %d) ?\n",
          pathname, (int) src, size, option));

    ttw(ttr(TTrApi, "ffs_file_write('%s', 0x%x, %d, %d) ?" NL,
            pathname, (int) src, size, option)); 

    // TASKBEGIN effs_t FILE_WRITE(path=pathname, src=src, size=size, value16=option) iref_t i, dir; char *name; effs_t error; int chunk_size, size_remaining, bytes_free;

    if (fs.initerror)
        return fs.initerror;

    if (size < 0)
        return EFFS_INVALID;
    
    ffs_query(Q_BYTES_FREE, &bytes_free);
    if (bytes_free < size)
        return EFFS_NOSPACE;

    chunk_size = (size > fs.chunk_size_max ? fs.chunk_size_max : size);

    if ((i = object_lookup(pathname, &name, &dir)) < 0) {
        // Object not found, continue like fcreate()
        if (i != EFFS_NOTFOUND)
            return i;

        if (!is_open_option(option, FFS_O_CREATE))
            return EFFS_NOTFOUND;

        journal_begin(0);

        if ((dir = object_create(name, src, chunk_size, -dir)) < 0)
            return dir;

        journal_end(OT_FILE);
    }
    
    else {
        // Object found, continue like fupdate()
        if (is_open_option(option, (FFS_O_CREATE)) 
	    && is_open_option(option, (FFS_O_EXCL)))
            return EFFS_EXISTS;

        if (get_fdi(i) >= 0) 
            return EFFS_LOCKED;

        // Even though the ffs architecture allows to have data in
        // directory objects, we don't want to complicate matters, so we
        // return an error
        if (is_object(inode_addr(i), OT_DIR) && !(fs.flags & FS_DIR_DATA)) {
            return EFFS_NOTAFILE;
        }
    
        if ((i = is_readonly(i, pathname)) < 0)
            return i;

        // Save the segment (if any) in the global variable because this
        // global variable will be updated if the inode is going to be
        // relocated if an inode_reclaim() is triggeret by the object_create()
        fs.i_backup = segment_next(i);

        journal_begin(i);

        if ((dir = object_create(name, src, chunk_size, -dir)) < 0)
            return dir;
        
        // Do not link child - we are replacing the complete file!
        fs.link_child = 0;  
        journal_end(0);
        
        // If any other segments exist then remove them FIXME: If we get a
        // power failure here then the remaining segments wil not be removed
        // before inode_reclaim() has been executed
        if (fs.i_backup > 0) 
            if ((error = object_remove(fs.i_backup)) < 0)
                return error;
        
    }
    // Save dir in fs.i_backup because this will be updated if some of the
    // chunks below trigger a inode reclaim!
    fs.i_backup = dir;

    size_remaining = size - chunk_size;
    
    while (size_remaining > 0) {

        chunk_size = (size_remaining > fs.chunk_size_max ? 
                      fs.chunk_size_max : size_remaining);

        journal_begin(0);
        
        if ((i = segment_create((char*) src + size - size_remaining,
                                    chunk_size, fs.i_backup)) < 0)
            return i;
        
        journal_end(OT_SEGMENT);
        
        size_remaining -= chunk_size;
    }
    
    tw(tr_bstat());

    return EFFS_OK;

    // TASKEND
}

// Note: ffs_fcreate() is deprecated and should not be used. Use
// ffs_file_write(..., FFS_O_CREATE | FFS_O_EXCL) instead.
effs_t ffs_fcreate(const char *pathname, void *src, int size)
{
    FFS_BLOCKING_CALL_BEGIN();
  
    result = ffs_file_write_b(pathname, src, size, FFS_O_CREATE | FFS_O_EXCL, 
                              0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}

req_id_t ffs_fcreate_nb(const char *pathname, void *src, int size,
                      T_RV_RETURN *cp)
{
    return ffs_file_write_b(pathname, src, size, FFS_O_CREATE | FFS_O_EXCL, 
                            cp, 0);
}

// Note: ffs_fupdate() is deprecated and should not be used. Use
// ffs_file_write(...,FFS_O_TRUNC) instead.
effs_t ffs_fupdate(const char *pathname, void *src, int size)
{

    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_file_write_b(pathname, src, size, FFS_O_TRUNC, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}

req_id_t ffs_fupdate_nb(const char *pathname, void *src, int size,
                      T_RV_RETURN *cp)
{
    return ffs_file_write_b(pathname, src, size, FFS_O_TRUNC, cp, 0);
}

// Note: ffs_fwrite() is deprecated and should not be used. Use
// ffs_file_write(...,FFS_O_CREATE | FFS_O_TRUNC) instead.
effs_t ffs_fwrite(const char *pathname, void *src, int size)
{

    FFS_BLOCKING_CALL_BEGIN();
    
    result = ffs_file_write_b(pathname, src, size, 
                              FFS_O_CREATE | FFS_O_TRUNC, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}

req_id_t ffs_fwrite_nb(const char *pathname, void *src, int size,
                     T_RV_RETURN *cp)
{
    return ffs_file_write_b(pathname, src, size, 
                            FFS_O_CREATE | FFS_O_TRUNC, cp, 0);
}

effs_t ffs_file_write(const char *pathname, void *src, int size, 
                      ffs_options_t option)
{

    FFS_BLOCKING_CALL_BEGIN();
    
    result = ffs_file_write_b(pathname, src, size, option, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}

req_id_t ffs_file_write_nb(const char *pathname, void *src, int size, 
                         ffs_options_t option,
                         T_RV_RETURN *cp)
{
    return ffs_file_write_b(pathname, src, size, option, cp, 0);
}

// Note important: ffs_fread() is deprecated and should not be used. Use
// ffs_file_read() instead.
int ffs_fread(const char *name, void *addr, int size)
{
    return ffs_file_read(name, addr, size);
}


int ffs_file_read(const char *name, void *addr, int size)
{
    int error;

    tw(tr(TR_BEGIN, TrApi, "file_read('%s', 0x%x, %d) {\n",
          name, (int) addr, size));
    
    if ((error = ffs_begin()) == EFFS_OK)  
    {
        error = file_read(name, addr, size);
    }
    
    tw(tr(TR_END, TrApi, "} %d\n", error));
    
    return ffs_end(error);     // number of bytes read  
}

/******************************************************************************
 * Stat, Symlink, Remove and Rename
 ******************************************************************************/

effs_t ffs_stat(const char *name, struct stat_s *stat)
{
    iref_t i;
 
    tw(tr(TR_FUNC, TrApi, "ffs_stat('%s', ?) ?\n", name));
    ttw(ttr(TTrApi, "ffs_stat('%s', ?) ?" NL, name)); 

    if (name == NULL)
        return EFFS_BADNAME;

    if ((i = ffs_begin()) == EFFS_OK)
    {
        if ((i = object_stat(name, (struct xstat_s*) stat, 0, 0, 0)) > 0)
            i = EFFS_OK;
    }
    
    return ffs_end(i);
}

effs_t ffs_lstat(const char *name, struct stat_s *stat)
{
    iref_t i;
 
    tw(tr(TR_FUNC, TrApi, "ffs_lstat('%s', ?) ?\n", name));
    ttw(ttr(TTrApi, "ffs_lstat('%s', ?) ?" NL, name)); 

    if ((i = ffs_begin()) == EFFS_OK) {
        if ((i = object_stat(name, (struct xstat_s*)stat, 1, 0, 0)) > 0)
            i = EFFS_OK;
    }

    return ffs_end(i);
}

effs_t ffs_xlstat(const char *name, struct xstat_s *stat)
{
    iref_t i;
 
    tw(tr(TR_FUNC, TrApi, "ffs_xlstat('%s', ?) ?\n", name));
    ttw(ttr(TTrApi, "ffs_xlstat('%s', ?) ?" NL, name)); 

    if ((i = ffs_begin()) == EFFS_OK) {
        if ((i = object_stat(name, stat, 1, 0, 1)) > 0)
            i = EFFS_OK;
    }

    return ffs_end(i);
}

effs_t ffs_fstat(fd_t fdi, struct stat_s *stat)
{
    iref_t i;
 
    tw(tr(TR_FUNC, TrApi, "ffs_fstat('%d', ?) ?\n", fdi));
    ttw(ttr(TTrApi, "ffs_fstat('%d', ?) ?" NL, fdi)); 

    if ((i = ffs_begin()) == EFFS_OK) {
        if ((i = object_stat( 0, (struct xstat_s*) stat, 0, fdi, 0)) > 0)
            i = EFFS_OK;
    }

    return ffs_end(i);
}

req_id_t ffs_symlink_b(const char *pathname, const char *src,
                      T_RV_RETURN *cp, struct ffs_blocking_s *fb)
{
    iref_t i, dir;
    char *name;
    int size;

    tw(tr(TR_FUNC, TrApi, "ffs_symlink('%s', '%s') ?\n", pathname, src));
    ttw(ttr(TTrApi, "ffs_symlink('%s', '%s') ?" NL, pathname, src)); 

    // TASKBEGIN effs_t SYMLINK(path=pathname, src=src) iref_t i, dir; int size; char *name;

    if (fs.initerror)
        return fs.initerror;

    if (src == NULL)
        return EFFS_BADNAME;

    i = object_lookup(pathname, &name, &dir);
    if (i > 0)
        return EFFS_EXISTS;
    if (i != EFFS_NOTFOUND)
        return i;

    size = ffs_strlen(src) + 1;  // include null-terminator

    journal_begin(0);

    if ((i = object_create(name, src, size, -dir)) < 0)
        return i;

    journal_end(OT_LINK);

    tw(tr_bstat());

    return EFFS_OK;

    // TASKEND
}

effs_t ffs_symlink(const char *pathname, const char *actualpath)
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_symlink_b(pathname, actualpath, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}

req_id_t ffs_symlink_nb(const char *pathname, const char *src,
                      T_RV_RETURN *cp)
{
    return ffs_symlink_b(pathname, src, cp, 0);
}

int ffs_readlink(const char *name, char *addr, int size)
{
    int error;

    tw(tr(TR_FUNC, TrApi, "ffs_readlink('%s')\n", name));

    if ((error = ffs_begin()) == EFFS_OK)
    {
        error = object_read(name, addr, size, 1);
    }
    return ffs_end(error);
}

req_id_t ffs_remove_b(const char *pathname, T_RV_RETURN *cp, 
            struct ffs_blocking_s *fb)
{
    iref_t i;

    tw(tr(TR_FUNC, TrApi, "ffs_remove('%s')\n", pathname));
    ttw(ttr(TTrApi, "ffs_remove('%s') ?" NL, pathname)); 

    // TASKBEGIN effs_t REMOVE(path=pathname) iref_t i;

    if (fs.initerror)
        return fs.initerror;

    if ((i = object_lookup_once(pathname, 0, 0)) < 0)
        return i;

    if (get_fdi(i) >= 0)
        return EFFS_LOCKED;

    if ((i = is_readonly(i, pathname)) < 0)
        return i;

    if ((i = object_remove(i)) < 0)
        return i;

    tw(tr_bstat());

    return EFFS_OK;

    // TASKEND
}

effs_t ffs_remove(const char *pathname)
{
    FFS_BLOCKING_CALL_BEGIN();
    
    result = ffs_remove_b(pathname, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result; 
}

req_id_t ffs_remove_nb(const char *pathname, T_RV_RETURN *cp)
{
    return ffs_remove_b(pathname, cp, 0);
}

req_id_t ffs_fcontrol_b(const char *pathname, int8 action, int param,
              T_RV_RETURN *cp, struct ffs_blocking_s *fb)
{
    iref_t i;

    tw(tr(TR_FUNC, TrApi, "ffs_fcontrol('%s', %d, 0x%x) ?\n",
          pathname, action, param));
    ttw(ttr(TTrApi, "ffs_fcontrol('%s', %d, 0x%x) ?" NL, 
	    pathname, action, param)); 

    // TASKBEGIN effs_t FCONTROL(path=pathname, value16=action, size=param) iref_t i;

    if (fs.initerror)
        return fs.initerror;

    if (pathname == NULL)
        return EFFS_BADNAME;

    if ((i = ffs_strcmp(pathname, "/dev/ffs")) != 0)
    {
        if ((i = object_lookup_once(pathname, 0, 0)) < 0)
            return i;

        if ((i = is_readonly(i, pathname)) < 0)
            return i;
    }

    if ((i = object_control(i, action, param)) < 0)
        return i;

    tw(tr_bstat());

    return EFFS_OK;

    // TASKEND
}

effs_t ffs_fcontrol(const char *pathname, int8 action, int param)
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_fcontrol_b(pathname, action, param, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;
}

req_id_t ffs_fcontrol_nb(const char *pathname, int8 action, int param,
                       T_RV_RETURN *cp)
{
    return ffs_fcontrol_b(pathname, action, param, cp, 0);
}

req_id_t ffs_rename_b(const char *pathname, const char *newname,
                     T_RV_RETURN *cp, struct ffs_blocking_s *fb)
{
    iref_t i, oldi, dir;
    char *name;
    struct inode_s *ip;

    tw(tr(TR_FUNC, TrApi, "ffs_rename('%s', '%s') ?\n", pathname, newname));
    ttw(ttr(TTrApi, "ffs_rename('%s', '%s') ?" NL, pathname, newname)); 

    // TASKBEGIN effs_t RENAME(path=pathname, src=newname) iref_t i, oldi, dir; char *name; struct inode_s *ip;

    if (fs.initerror)
        return fs.initerror;

    // pathname MUST exist, not be open and MUST be writable
    if ((oldi = object_lookup_once(pathname, 0, 0)) < 0)
        return oldi;
    if ((oldi = is_readonly(oldi, pathname)) < 0)
        return oldi;
    if (get_fdi(oldi) >= 0)
        return EFFS_LOCKED;

    journal_begin(oldi);

    if ((i = object_lookup_once(newname, &name, &dir)) < 0) {
        if (i != EFFS_NOTFOUND)
            return i;
    }
    else {                               // newname obj exist   
        ip = inode_addr(oldi);
        if (is_object(ip, OT_FILE)) {    // is old obj a file?
            if ((i = is_readonly(i, newname)) < 0)
                return i;

            ip = inode_addr(i);
            if (!is_object(ip, OT_FILE)) // newname MUST be a file 
                return EFFS_NOTAFILE;

            fs.journal.repli = i;        
        }
        else
            return EFFS_EXISTS;         
    }
    
    if ((i = object_rename(oldi, name, -dir)) < 0)
        return i;

    journal_end(0);

    tw(tr_bstat());    

    return EFFS_OK;

    // TASKEND

}

effs_t ffs_rename(const char *pathname, const char *newname)
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_rename_b(pathname, newname, 0, &fb);


    FFS_BLOCKING_CALL_END();

    return result;
}


req_id_t ffs_rename_nb(const char *pathname, const char *newname,
                     T_RV_RETURN *cp)
{
    return ffs_rename_b(pathname, newname, cp, 0);
}

/******************************************************************************
 * Directory Operations
 ******************************************************************************/

// All directory operations are more or less similar to unix
// semantics.
req_id_t ffs_mkdir_b(const char *pathname, T_RV_RETURN *cp, 
           struct ffs_blocking_s *fb)
{
    iref_t i, dir;
    char *name;

    tw(tr(TR_FUNC, TrApi, "ffs_mkdir('%s')\n", pathname));
    ttw(ttr(TTrApi, "ffs_mkdir('%s') ?" NL, pathname)); 

    // TASKBEGIN effs_t MKDIR(path=pathname) iref_t i, dir; char *name;

    if (fs.initerror)
        return fs.initerror;

    i = object_lookup(pathname, &name, &dir);
    if (i > 0)
        return EFFS_EXISTS;
    if (i != EFFS_NOTFOUND)
        return i;

    journal_begin(0);

    if ((i = object_create(name, 0, 0, -dir)) < 0)
        return i;

    journal_end(OT_DIR);

    tw(tr_bstat());

    return EFFS_OK;

    // TASKEND
}

effs_t ffs_mkdir(const char *pathname)
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_mkdir_b(pathname, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;
}

req_id_t ffs_mkdir_nb(const char *pathname, T_RV_RETURN *cp)
{
    return ffs_mkdir_b(pathname, cp, 0);
}

int ffs_opendir(const char *name, struct dir_s *dir)
{
    int i;

    tw(tr(TR_FUNC, TrApi, "ffs_opendir('%s', ?)\n", name));
    ttw(ttr(TTrApi, "ffs_opendir('%s', ?) ?" NL, name)); 

    if (dir == NULL)
        return EFFS_INVALID;

    if ((i = ffs_begin()) == EFFS_OK)
    {
        if ((i = dir_open(name)) >= 0)
        {
            dir->this = i;
            dir->index = i;

            // Now count the number of entries in the directory
            dir_traverse(-i, (iref_t *) &i);
        }
    }
    return ffs_end(i);
}

int ffs_readdir(struct dir_s *dir, char *name, int size)
{
    iref_t i;

    tw(tr(TR_BEGIN, TrApi, "ffs_readdir(?, ?, ?) {\n"));
    ttw(ttr(TTrApi, "ffs_readdir(?, ?, ?) ?" NL)); 

    if (dir == NULL || name == NULL || size < 0) {
        tw(tr(TR_END, TrApi, "} %d\n", EFFS_INVALID));
        return EFFS_INVALID;
    }

    if ((i = ffs_begin()) == EFFS_OK)
    {
        if ((i = dir_next(dir->this, dir->index, name, size)))
            dir->index = i;
    }
    tw(tr(TR_END, TrApi, "} ('%s') %d\n", name, i));

    return ffs_end(i);
}


/******************************************************************************
 * Preformat and Format
 ******************************************************************************/

// Note that we do NOT call ffs_begin() because it will just return
// EFFS_NOFORMAT!
req_id_t ffs_format_b(const char *name, uint16 magic, T_RV_RETURN *cp,
            struct ffs_blocking_s *fb)
{
    effs_t i;

    tw(tr(TR_BEGIN, TrApi, "ffs_format('%s', 0x%x) {\n", name, magic));
    ttw(ttr(TTrApi, "ffs_format('%s', 0x%x) ?" NL, name, magic)); 

    // TASKBEGIN effs_t FORMAT(path=name, size=magic) iref_t i;

    if (magic != 0x2BAD) {
        tw(tr(TR_END, TrApi, "} %d\n", EFFS_INVALID));
        return EFFS_INVALID;
    }

    if (name == NULL) {
        name = "/ffs-5.54";
    }

    if (*name != '/') {
        tw(tr(TR_END, TrApi, "} %d\n", EFFS_BADNAME));
        return EFFS_BADNAME;
    }

    if ((i = is_formattable(1)) < 0) {
        tw(tr(TR_END, TrApi, "} %d\n", i));
        return i;
    }

    if ((i = fs_format(name)) < 0)
        return i;

    tw(tr(TR_END, TrApi, "} %d\n", i));

    tw(tr_bstat());

    return EFFS_OK;

    // TASKEND
}
 
effs_t ffs_format(const char *name, uint16 magic)
{
    FFS_BLOCKING_CALL_BEGIN();
    
    result = ffs_format_b(name, magic, 0, &fb);
    
    FFS_BLOCKING_CALL_END();

    return result;
}

req_id_t ffs_format_nb(const char *name, uint16 magic, T_RV_RETURN *cp)
{
   return ffs_format_b(name, magic, cp, 0);
}

req_id_t ffs_preformat_b(uint16 magic, T_RV_RETURN *cp, 
               struct ffs_blocking_s *fb)
{
    effs_t i;

    tw(tr(TR_BEGIN, TrApi, "ffs_preformat(0x%x) {\n", magic));
    ttw(ttr(TTrApi, "ffs_preformat(0x%x) ?" NL, magic)); 

    // TASKBEGIN effs_t PREFORMAT(path="/", size=magic) effs_t i;

    if (magic != 0xDEAD) {
        tw(tr(TR_END, TrApi, "} %d\n", EFFS_INVALID));
        return EFFS_INVALID;
    }

    if (!ffs_is_modifiable("")) {
        tw(tr(TR_END, TrApi, "} %d\n", EFFS_ACCESS));
        return EFFS_ACCESS;
    }

    if ((i = is_formattable(0)) < 0) {
        tw(tr(TR_END, TrApi, "} %d\n", i));
        return i;
    }

    if ((i = fs_preformat()) < 0)
        return i;

    tw(tr(TR_END, TrApi, "} %d\n", i));

    tw(tr_bstat());

    return EFFS_OK;

    // TASKEND
}

effs_t ffs_preformat(uint16 magic)
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_preformat_b(magic, 0, &fb);
    
    FFS_BLOCKING_CALL_END();

    return result;
}

req_id_t ffs_preformat_nb(uint16 magic, T_RV_RETURN *cp)
{
  return ffs_preformat_b(magic, cp, 0);
}

/******************************************************************************
 * Open, Read, Write, Close
 ******************************************************************************/
req_id_t ffs_open_b(const char *pathname, ffs_options_t option,
        T_RV_RETURN *cp, struct ffs_blocking_s *fb)
{
    iref_t i, dir, dummy;            
    char *name;
    fd_t other_fdi, fdi = 0;
    int error; 
    struct inode_s *ip;
    
    tw(tr(TR_FUNC, TrApi, "ffs_open('%s', 0x%x) ?\n", pathname, option));
    ttw(ttr(TTrApi, "ffs_open('%s', 0x%x) ?" NL, pathname, option)); 

    // TASKBEGIN fd_t OPEN(path=pathname, value16=option) iref_t i, dir, dummy; char *name; fd_t other_fdi, fdi = 0; int error; struct inode_s *ip;

    if (fs.initerror)
        return fs.initerror;
    
    // Minimum one of the flags RD or WR must be specifyed
    if (!is_open_option(option, FFS_O_RDONLY) && 
        !is_open_option(option, FFS_O_WRONLY))
        return EFFS_INVALID;
    
    // RDONLY must not be combined with any other options if not together
    // with WR!
    if (is_open_option(option, FFS_O_RDONLY) && 
        !is_open_option(option, FFS_O_WRONLY))     
        if (!(option == FFS_O_RDONLY))
            return EFFS_INVALID;
    
    for (fdi = 0; fdi < fs.fd_max; fdi++) {  // Find free fd
        if (fs.fd[fdi].options == 0) {
            break;
        }
    }

    if (fdi >= fs.fd_max)
        return EFFS_NUMFD;  // Too many open files in system

    i = object_lookup(pathname, &name, &dir);
    if (i < 0 && i != EFFS_NOTFOUND)
        return i;

    // Open one file several times in RD is okay but only one time in WR
    if (i != EFFS_NOTFOUND && (other_fdi = get_fdi(i)) >= 0) {
        if (is_open_option(fs.fd[other_fdi].options, FFS_O_WRONLY) || 
	    is_open_option(option, FFS_O_WRONLY))
            return EFFS_LOCKED;
    }

    // Init default values
    fs.fd[fdi].fp = fs.fd[fdi].size = fs.fd[fdi].wfp = fs.fd[fdi].dirty = 0;
         
    if (i == EFFS_NOTFOUND) {
        if (is_open_option(option, (FFS_O_CREATE | FFS_O_WRONLY))) {
            if ((error = is_filename(name)) < 0)
                return error;

            // Create segmenthead
            journal_begin(0);

            if ((i = object_create(name, 0, 0, -dir)) < 0)
                return i;

            journal_end(OT_FILE);  
            tw(tr_bstat());
            fs.fd[fdi].seghead = i;
        }
        else 
            return EFFS_NOTFOUND;    
    }
    else {
        if (is_open_option(option, FFS_O_WRONLY)) {
            if (is_open_option(option, (FFS_O_CREATE | FFS_O_EXCL)))
                return EFFS_EXISTS;   
            if ((i = is_readonly(i, pathname)) < 0)
                return i;
        }
        ip = inode_addr(i);
        
        if (is_object(ip, OT_DIR))
            return EFFS_NOTAFILE;

        if (is_open_option(option, FFS_O_TRUNC)) { 
            // Save the segment (if any) in the global variable because this
            // global variable will be updated if the inode is relocated by
            // an inode_reclaim() triggeret by object_create()
            fs.i_backup = segment_next(i);

            // Replace old seghead with a new and remove all old segments
            journal_begin(i);

            if ((i = object_create(name, 0, 0, -dir)) < 0)
                return i;

            // Do not link child
            fs.link_child = 0;  
            journal_end(0);

	    // If any further segments exist then remove them now
            if (fs.i_backup > 0) 
                if ((error = object_remove(fs.i_backup)) < 0)
                    return error;

            tw(tr_bstat());
        }

        else {
            // Get total size of the file.    
            fs.fd[fdi].size = segfile_seek(i, INT_MAX, &dummy, 0);
        }
        
        if (is_open_option(option, FFS_O_APPEND)) { 
            fs.fd[fdi].fp = fs.fd[fdi].size;
        }
    }
    
    if (is_open_option(option, FFS_O_WRONLY)) {
#if (TARGET == 1)
        if ((fs.fd[fdi].buf = (char *) target_malloc(fs.fd_buf_size)) == 0)
            return EFFS_MEMORY;
#else
        if ((fs.fd[fdi].buf = malloc(fs.fd_buf_size)) == 0)
            return EFFS_MEMORY;
#endif
    }

    // Save data in file descriptor     
    fs.fd[fdi].seghead = i;    
    fs.fd[fdi].options = option;  

    return fdi + FFS_FD_OFFSET;

    // TASKEND
}

fd_t ffs_open(const char *pathname, ffs_options_t option)
{
    FFS_BLOCKING_CALL_BEGIN();
    
    result = ffs_open_b(pathname, option, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}


req_id_t ffs_open_nb(const char *pathname, ffs_options_t option,
         T_RV_RETURN *cp)
{
    return ffs_open_b(pathname, option, cp, 0);
}

req_id_t ffs_close_b(fd_t fdi, T_RV_RETURN *cp, struct ffs_blocking_s *fb)
{
    int error;

    tw(tr(TR_FUNC, TrApi, "ffs_close(%d) ?\n", fdi));  
    ttw(ttr(TTrApi, "ffs_close(%d) ?" NL, fdi)); 

    // TASKBEGIN effs_t CLOSE(fdi=fdi) iref_t i; int error;

    if (fs.initerror)
        return fs.initerror;

    fdi -= FFS_FD_OFFSET;

    if (!is_fd_valid(fdi))
        return EFFS_BADFD;
    
    if (is_open_option(fs.fd[fdi].options, FFS_O_WRONLY )) {
        if ((error = datasync(fdi)) < 0)
            return error;

#if (TARGET == 1)
        target_free(fs.fd[fdi].buf);
#else
        free(fs.fd[fdi].buf);
#endif
    }

    // Clear all data in file descriptor
    fs.fd[fdi].seghead = 0;
    fs.fd[fdi].options = fs.fd[fdi].fp = 0;

    return EFFS_OK;

    // TASKEND
}

effs_t ffs_close(fd_t fdi)
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_close_b(fdi, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}


req_id_t ffs_close_nb(fd_t fdi, T_RV_RETURN *cp)
{
    return ffs_close_b( fdi, cp, 0);
}

req_id_t ffs_write_b(fd_t fdi, void *src, int amount, 
        T_RV_RETURN *cp, struct ffs_blocking_s *fb)
{
    effs_t error;
    iref_t i; 
    int size_remaining, fp_offset;
    int size, size_done;
    offset_t chunk_offset;

    tw(tr(TR_BEGIN, TrApi, "ffs_write_b(%d, 0x%x, %d) ?{\n", fdi, src, amount));
    ttw(ttr(TTrApi, "ffs_write_b(%d, 0x%x, %d) ?" NL, fdi, src, amount)); 

    // TASKBEGIN int WRITE(fdi=fdi, src=src, size=amount) effs_t error; iref_t i; int size_remaining, fp_offset; int size, size_done; offset_t chunk_offset; 

    if (fs.initerror)
        return fs.initerror;

    if (amount < 0 || src == NULL)
        return EFFS_INVALID;
  
    fdi -= FFS_FD_OFFSET;
    
    if (!is_fd_valid(fdi))
        return EFFS_BADFD;

    if (!is_open_option(fs.fd[fdi].options, FFS_O_WRONLY ))
        return EFFS_INVALID; // not opened with write flag

    // If FFS_O_APPEEND is specified move fp to eof
    if (is_open_option(fs.fd[fdi].options, FFS_O_APPEND ))
        fs.fd[fdi].fp = fs.fd[fdi].size;

    // If fp has been moved outside the write buf (by a read) then flush the
    // write buffer.
    if (fs.fd[fdi].fp >= (fs.fd[fdi].wfp + fs.chunk_size_max)) {
        if ((error = datasync(fdi)) < 0)
            return error;
    }

    size_done = 0;
    size_remaining = amount;

    do {       
        if (!fs.fd[fdi].dirty ) {   
            // Buffer is not dirty so find the chunk that fp points to.
            segfile_seek(fs.fd[fdi].seghead, fs.fd[fdi].fp, &i, 
                             &chunk_offset);

            if ((fs.fd[fdi].size == fs.fd[fdi].fp && 
                 chunk_offset == fs.chunk_size_max) || fs.fd[fdi].size == 0 ) {
                // End of file and last chunk is full or empty seghead.
                fs.fd[fdi].wfp = fs.fd[fdi].size;
                fs.fd[fdi].wch = 0;  // Create new chunk (not update).
            }
            else {
                // Work on this chunk and update it later by datasyns
                segment_read(i, fs.fd[fdi].buf, fs.fd_buf_size, 0);
                fs.fd[fdi].wfp = fs.fd[fdi].fp - chunk_offset;
                fs.fd[fdi].wch = i;
            }
        }
       
	fs.fd[fdi].dirty = 1;
	fp_offset = fs.fd[fdi].fp - fs.fd[fdi].wfp;

        // Fill the buffer to max or just add the rest
        size = fs.chunk_size_max - fp_offset;

	if (size_remaining <= fs.chunk_size_max - fp_offset)
                size = size_remaining;

	tw(tr(TR_FUNC, TrApi, "Copy data to buffer (size: %d)\n", size));

        memcpy(fs.fd[fdi].buf + fp_offset, (uint8*)src + size_done, 
               size);
        
        fs.fd[fdi].fp += size;
        if (fs.fd[fdi].fp > fs.fd[fdi].size)
            fs.fd[fdi].size = fs.fd[fdi].fp;

        size_done += size;         // FIXME: remove size_done or size_remaining
        size_remaining -= size;
        
        // If wrbuf is full (size = chunk_size_max) so create a chunk.
        if (fs.fd[fdi].fp >= (fs.fd[fdi].wfp + fs.chunk_size_max)) {
            if ((error = datasync(fdi)) < 0)
                return error;
        }
    } while(size_remaining > 0);
    
    tw(tr(TR_END, TrApi, "} %d\n", amount));
    return amount;         

    // TASKEND
}

int ffs_write(fd_t fdi, void *src, int amount)
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_write_b(fdi, src, amount, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}

req_id_t ffs_write_nb(fd_t fdi, void *src, int amount, T_RV_RETURN *cp)
{
    
    tw(tr(TR_FUNC, TrApi, "ffs_write_nb(%d, 0x%x, %d) ?\n", fdi, src, amount));

    return ffs_write_b(fdi, src, amount, cp, 0);      
}

int ffs_read(fd_t fdi, void *src, int size)
{
    int error;
    
    tw(tr(TR_BEGIN, TrApi, "ffs_read(%d, 0x%x, %d) {\n", fdi, src, size));
    ttw(ttr(TTrApi, "ffs_read(%d, 0x%x, %d) ?" NL, fdi, src, size)); 

    if ((error = ffs_begin()) == EFFS_OK)    
    {
        error = stream_read(fdi - FFS_FD_OFFSET, src, size);
    }

    tw(tr(TR_END, TrApi, "} %d\n", error));
    return ffs_end(error);     // number of bytes read  
}

// The seek function will not allow the file offset to be set beyond the end
// of the existing data in the file or the final offset to be negative.
req_id_t ffs_seek_b(fd_t fdi, int offset, int whence, T_RV_RETURN *cp, 
               struct ffs_blocking_s *fb)
{
    effs_t error;
    int fp_new;

    tw(tr(TR_FUNC, TrApi, "ffs_seek(%d, %d, %d) ?\n", fdi, offset, whence));
    ttw(ttr(TTrApi, "ffs_seek(%d, %d, %d) ?" NL, fdi, offset, whence)); 

    // TASKBEGIN int SEEK(fdi=fdi, size=offset, value16=whence) effs_t error; iref_t i; int fp_new, foffset;

    if (fs.initerror)
        return fs.initerror;

    fdi -= FFS_FD_OFFSET;

    if (!is_fd_valid(fdi))
        return EFFS_BADFD;
    
    switch(whence) {
    case FFS_SEEK_SET:
        if (offset < 0 || offset > fs.fd[fdi].size)
            return EFFS_INVALID;  
        fp_new = offset;
        break;
    case FFS_SEEK_CUR:
        if (fs.fd[fdi].fp + offset < 0 || 
            fs.fd[fdi].fp + offset > fs.fd[fdi].size)
            return EFFS_INVALID;  
        fp_new = fs.fd[fdi].fp + offset;
        break;
    case FFS_SEEK_END:
        if (offset > 0 || fs.fd[fdi].size < -offset)
            return EFFS_INVALID;
        fp_new = (offset + fs.fd[fdi].size);
        break;
    default:
        return EFFS_INVALID;
    }
    
    if (!is_offset_in_buf(fp_new, fdi))
        if ((error = datasync(fdi)) < 0)
            return error;
    
    return fs.fd[fdi].fp = fp_new;
    
    // TASKEND
}

int ffs_seek(fd_t fdi, int offset, int whence) 
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_seek_b(fdi, offset, whence, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}

req_id_t ffs_seek_nb(fd_t fdi, int offset, int whence, T_RV_RETURN *cp) 
{
    return ffs_seek_b(fdi, offset, whence, cp, 0);
}

req_id_t ffs_truncate_b(const char *path, offset_t length, T_RV_RETURN *cp, 
                      struct ffs_blocking_s *fb) 
{
    tw(tr(TR_FUNC, TrApi, "ffs_truncate('%s', %d) \n", path, length));
    ttw(ttr(TTrApi, "ffs_ftruncate('%s', %d) ?" NL, path, length)); 

    // TASKBEGIN effs_t TRUNC(path=path, size=length) iref_t i;

    if (fs.initerror)
        return fs.initerror;

    if (path == NULL)
        return EFFS_BADNAME;

    return object_truncate(path, -1, length);
        
    // TASKEND
}

effs_t ffs_truncate(const char *path, offset_t length) 
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_truncate_b(path, length, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}

req_id_t ffs_truncate_nb(const char *path, offset_t length, T_RV_RETURN *cp) 
{
    return ffs_truncate_b(path, length, cp, 0);
}

req_id_t ffs_ftruncate_b(fd_t fdi, offset_t length, T_RV_RETURN *cp, 
                       struct ffs_blocking_s *fb)
{
    tw(tr(TR_FUNC, TrApi, "ffs_ftruncate(%d, %d) \n", fdi, length));
    ttw(ttr(TTrApi, "ffs_ftruncate(%d, %d) ?" NL, fdi, length)); 

    // TASKBEGIN effs_t FTRUNC(fdi=fdi, size=length) iref_t i;

    if (fs.initerror)
        return fs.initerror;    
    
    return object_truncate(0, fdi - FFS_FD_OFFSET, length);
    
    // TASKEND
}

effs_t ffs_ftruncate(fd_t fdi, offset_t length)
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_ftruncate_b(fdi, length, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}

req_id_t ffs_ftruncate_nb(fd_t fdi, offset_t length, T_RV_RETURN *cp)
{
    return ffs_ftruncate_b(fdi, length, cp, 0);
}

req_id_t ffs_fdatasync_b(fd_t fdi, T_RV_RETURN *cp, struct ffs_blocking_s *fb) 
{
    tw(tr(TR_FUNC, TrApi, "ffs_fdatasync(%d) \n", fdi));
    ttw(ttr(TTrApi, "ffs_fdatasync(%d) ?" NL, fdi)); 
    
    // TASKBEGIN effs_t FDATASYNC(fdi=fdi) effs_t error;

    if (fs.initerror)
        return fs.initerror;
  
    return datasync(fdi - FFS_FD_OFFSET);
    
    // TASKEND
}

effs_t ffs_fdatasync(fd_t fdi) 
{
    FFS_BLOCKING_CALL_BEGIN();

    result = ffs_fdatasync_b(fdi, 0, &fb);

    FFS_BLOCKING_CALL_END();

    return result;             
}

req_id_t ffs_fdatasync_nb(fd_t fdi, T_RV_RETURN *cp) 
{
    return ffs_fdatasync_b(fdi, cp, 0);
}
