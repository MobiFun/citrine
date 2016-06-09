/*
+-----------------------------------------------------------------------------
|  Modul   :  ffs_coat
+-----------------------------------------------------------------------------
|  Copyright 2002 Texas Instruments Berlin, AG
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Texas
|                 Instruments Berlin, AG
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Texas Instruments Berlin, AG.
+-----------------------------------------------------------------------------
|  Purpose :  Encapsulate the FFS API.
+-----------------------------------------------------------------------------
*/

#ifndef FFS_COAT_H
#define FFS_COAT_H

#define FFS_COAT_ENABLED

#if defined(FFS_COAT_ENABLED)
#ifdef TI_PS_HCOMM_CHANGE
#define FFS_open(path,flags)                  _FFS_open(path,flags,_hCommMMI,__FILE__,__LINE__)
#define FFS_write(fd,p,size)                  _FFS_write(fd,p,size,_hCommMMI,__FILE__,__LINE__)
#define FFS_read(fd,p,size)                   _FFS_read(fd,p,size,_hCommMMI,__FILE__,__LINE__)
#define FFS_seek(fd,off,whence)               _FFS_seek(fd,off,whence,_hCommMMI,__FILE__,__LINE__)
#define FFS_opendir(path,dir)                 _FFS_opendir(path,dir,_hCommMMI,__FILE__,__LINE__)
#define FFS_readdir(dir,p,size)               _FFS_readdir(dir,p,size,_hCommMMI,__FILE__,__LINE__)
#define FFS_readlink(path,p,size)             _FFS_readlink(path,p,size,_hCommMMI,__FILE__,__LINE__)
#define FFS_file_read(path,p,size)            _FFS_file_read(path,p,size,_hCommMMI,__FILE__,__LINE__)
#define FFS_fread(path,p,size)                _FFS_fread(path,p,size,_hCommMMI,__FILE__,__LINE__)

/* FFS functions which return a value EFFS_OK if OK */
#define FFS_preformat(magic)                  _FFS_preformat(magic,_hCommMMI,__FILE__,__LINE__)
#define FFS_format(name,magic)                _FFS_format(name,magic,_hCommMMI,__FILE__,__LINE__)
#define FFS_close(fd)                         _FFS_close(fd,_hCommMMI,__FILE__,__LINE__)
#define FFS_truncate(path,length)             _FFS_truncate(path,length,_hCommMMI,__FILE__,__LINE__)
#define FFS_ftruncate(fd,length)              _FFS_ftruncate(fd,length,_hCommMMI,__FILE__,__LINE__)
#define FFS_fdatasync(fd)                     _FFS_fdatasync(fd,_hCommMMI,__FILE__,__LINE__)
#define FFS_stat(name,stat)                   _FFS_stat(name,stat,_hCommMMI,__FILE__,__LINE__)
#define FFS_fstat(fd,stat)                    _FFS_fstat(fd,stat,_hCommMMI,__FILE__,__LINE__)
#define FFS_lstat(name,stat)                  _FFS_lstat(name,stat,_hCommMMI,__FILE__,__LINE__)
#define FFS_linkstat(name,stat)               _FFS_linkstat(name,stat,_hCommMMI,__FILE__,__LINE__)
#define FFS_xlstat(name,stat)                 _FFS_xlstat(name,stat,_hCommMMI,__FILE__,__LINE__)
#define FFS_remove(name)                      _FFS_remove(name,_hCommMMI,__FILE__,__LINE__)
#define FFS_mkdir(path)                       _FFS_mkdir(path,_hCommMMI,__FILE__,__LINE__)
#define FFS_symlink(name,path)                _FFS_symlink(name,path,_hCommMMI,__FILE__,__LINE__)
#define FFS_rename(oldname,newname)           _FFS_rename(oldname,newname,_hCommMMI,__FILE__,__LINE__)
#define FFS_file_write(name,addr,size,flags)  _FFS_file_write(name,addr,size,flags,_hCommMMI,__FILE__,__LINE__)
#define FFS_fcreate(path,p,size)              _FFS_fcreate(path,p,size,_hCommMMI,__FILE__,__LINE__)
#define FFS_fupdate(name,addr,size)           _FFS_fupdate(name,addr,size,_hCommMMI,__FILE__,__LINE__)
#define FFS_fwrite(path,p,size)               _FFS_fwrite(path,p,size,_hCommMMI,__FILE__,__LINE__)
#define FFS_fcontrol(path,type,param)         _FFS_fcontrol(path,type,param,_hCommMMI,__FILE__,__LINE__)
#define FFS_query(query,p)                    _FFS_query(query,p,_hCommMMI,__FILE__,__LINE__)
/* FFS functions whose positive return value has a meaning */
#else
#define FFS_open(path,flags)                  _FFS_open(path,flags,hCommMMI,__FILE__,__LINE__)
#define FFS_write(fd,p,size)                  _FFS_write(fd,p,size,hCommMMI,__FILE__,__LINE__)
#define FFS_read(fd,p,size)                   _FFS_read(fd,p,size,hCommMMI,__FILE__,__LINE__)
#define FFS_seek(fd,off,whence)               _FFS_seek(fd,off,whence,hCommMMI,__FILE__,__LINE__)
#define FFS_opendir(path,dir)                 _FFS_opendir(path,dir,hCommMMI,__FILE__,__LINE__)
#define FFS_readdir(dir,p,size)               _FFS_readdir(dir,p,size,hCommMMI,__FILE__,__LINE__)
#define FFS_readlink(path,p,size)             _FFS_readlink(path,p,size,hCommMMI,__FILE__,__LINE__)
#define FFS_file_read(path,p,size)            _FFS_file_read(path,p,size,hCommMMI,__FILE__,__LINE__)
#define FFS_fread(path,p,size)                _FFS_fread(path,p,size,hCommMMI,__FILE__,__LINE__)

/* FFS functions which return a value EFFS_OK if OK */
#define FFS_preformat(magic)                  _FFS_preformat(magic,hCommMMI,__FILE__,__LINE__)
#define FFS_format(name,magic)                _FFS_format(name,magic,hCommMMI,__FILE__,__LINE__)
#define FFS_close(fd)                         _FFS_close(fd,hCommMMI,__FILE__,__LINE__)
#define FFS_truncate(path,length)             _FFS_truncate(path,length,hCommMMI,__FILE__,__LINE__)
#define FFS_ftruncate(fd,length)              _FFS_ftruncate(fd,length,hCommMMI,__FILE__,__LINE__)
#define FFS_fdatasync(fd)                     _FFS_fdatasync(fd,hCommMMI,__FILE__,__LINE__)
#define FFS_stat(name,stat)                   _FFS_stat(name,stat,hCommMMI,__FILE__,__LINE__)
#define FFS_fstat(fd,stat)                    _FFS_fstat(fd,stat,hCommMMI,__FILE__,__LINE__)
#define FFS_lstat(name,stat)                  _FFS_lstat(name,stat,hCommMMI,__FILE__,__LINE__)
#define FFS_linkstat(name,stat)               _FFS_linkstat(name,stat,hCommMMI,__FILE__,__LINE__)
#define FFS_xlstat(name,stat)                 _FFS_xlstat(name,stat,hCommMMI,__FILE__,__LINE__)
#define FFS_remove(name)                      _FFS_remove(name,hCommMMI,__FILE__,__LINE__)
#define FFS_mkdir(path)                       _FFS_mkdir(path,hCommMMI,__FILE__,__LINE__)
#define FFS_symlink(name,path)                _FFS_symlink(name,path,hCommMMI,__FILE__,__LINE__)
#define FFS_rename(oldname,newname)           _FFS_rename(oldname,newname,hCommMMI,__FILE__,__LINE__)
#define FFS_file_write(name,addr,size,flags)  _FFS_file_write(name,addr,size,flags,hCommMMI,__FILE__,__LINE__)
#define FFS_fcreate(path,p,size)              _FFS_fcreate(path,p,size,hCommMMI,__FILE__,__LINE__)
#define FFS_fupdate(name,addr,size)           _FFS_fupdate(name,addr,size,hCommMMI,__FILE__,__LINE__)
#define FFS_fwrite(path,p,size)               _FFS_fwrite(path,p,size,hCommMMI,__FILE__,__LINE__)
#define FFS_fcontrol(path,type,param)         _FFS_fcontrol(path,type,param,hCommMMI,__FILE__,__LINE__)
#define FFS_query(query,p)                    _FFS_query(query,p,hCommMMI,__FILE__,__LINE__)
#endif
/* prototypes */
T_FFS_FD  _FFS_open(const char *name, T_FFS_OPEN_FLAGS option,
                   T_HANDLE hMMI, const char * const  file, int line);
T_FFS_SIZE _FFS_write(T_FFS_FD fdi, void *src, T_FFS_SIZE amount,
                    T_HANDLE hMMI, const char * const  file, int line);
T_FFS_SIZE _FFS_read(T_FFS_FD fdi, void *src, T_FFS_SIZE size,
                    T_HANDLE hMMI, const char * const  file, int line);
T_FFS_SIZE _FFS_seek(T_FFS_FD fdi, T_FFS_SIZE offset, T_FFS_WHENCE whence,
                    T_HANDLE hMMI, const char * const  file, int line);
T_FFS_SIZE _FFS_opendir(const char *name, T_FFS_DIR *dir,
                      T_HANDLE hMMI, const char * const  file, int line);
T_FFS_SIZE _FFS_readdir (T_FFS_DIR *dir, char *name, T_FFS_SIZE size,
                      T_HANDLE hMMI, const char * const  file, int line);
T_FFS_SIZE _FFS_readlink(const char *name, char *addr, T_FFS_SIZE size,
                      T_HANDLE hMMI, const char * const  file, int line);
T_FFS_SIZE _FFS_file_read(const char *name, void *addr, T_FFS_SIZE size,
                       T_HANDLE hMMI, const char * const  file, int line);
T_FFS_SIZE _FFS_fread(const char *name, void *addr, T_FFS_SIZE size,
                    T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_preformat(UINT16 magic,
                      T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_format(const char *name, UINT16 magic,
                    T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_close(T_FFS_FD fdi,
                   T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_truncate(const char *path, T_FFS_OFFSET length,
                     T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_ftruncate(T_FFS_FD fdi, T_FFS_OFFSET length,
                      T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_fdatasync(T_FFS_FD fdi,
                       T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_stat(const char *name, T_FFS_STAT *stat,
                  T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_fstat(T_FFS_FD fdi, T_FFS_STAT *stat,
                   T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_lstat(const char *name, T_FFS_STAT *stat,
                   T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_xlstat(const char *name, T_FFS_XSTAT *stat,
                    T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_linkstat(const char *name, T_FFS_STAT *stat,
                     T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_remove(const char *name,
                     T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_mkdir(const char *name,
                    T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_symlink(const char *name, const char *actualpath,
                     T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_rename(const char *oldname, const char *newname,
                     T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_file_write(const char *name, void *addr, T_FFS_SIZE size,
                      T_FFS_OPEN_FLAGS flags,
                      T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_fcreate(const char *name, void *addr, T_FFS_SIZE size,
                     T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_fupdate(const char *name, void *addr, T_FFS_SIZE size,
                     T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_fwrite(const char *name, void *addr, T_FFS_SIZE size,
                    T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_fcontrol(const char *pathname, INT8 action, int param,
                     T_HANDLE hMMI, const char * const  file, int line);
T_FFS_RET _FFS_query(INT8 query, void *p,
                   T_HANDLE hMMI, const char * const  file, int line);
#else  /* FFS_COAT_ENABLED */
/* FFS functions whose positive return value has a meaning */
#define FFS_open(path,flags)                  ffs_open(path,flags)
#define FFS_write(fd,p,size)                  ffs_write(fd,p,size)
#define FFS_read(fd,p,size)                   ffs_read(fd,p,size)
#define FFS_seek(fd,off,whence)               ffs_seek(fd,off,whence)
#define FFS_opendir(path,dir)                 ffs_opendir(path,dir)
#define FFS_readdir(dir,p,size)               ffs_readdir(dir,p,size)
#define FFS_readlink(path,p,size)             ffs_readlink(path,p,size)
#define FFS_file_read(path,p,size)            ffs_file_read(path,p,size)
#define FFS_fread(path,p,size)                ffs_fread(path,p,size)

/* FFS functions which return a value EFFS_OK if OK */
#define FFS_preformat(magic)                  ffs_preformat(magic)
#define FFS_format(name,magic)                ffs_format(name,magic)
#define FFS_close(fd)                         ffs_close(fd)
#define FFS_truncate(path,length)             ffs_truncate(path,length)
#define FFS_ftruncate(fd,length)              ffs_ftruncate(fd,length)
#define FFS_fdatasync(fd)                     ffs_fdatasync(fd)
#define FFS_stat(name,stat)                   ffs_stat(name,stat)
#define FFS_fstat(fd,stat)                    ffs_fstat(fd,stat)
#define FFS_lstat(name,stat)                  ffs_lstat(name,stat)
#define FFS_linkstat(name,stat)               ffs_linkstat(name,stat)
#define FFS_xlstat(name,stat)                 ffs_xlstat(name,stat)
#define FFS_remove(name)                      ffs_remove(name)
#define FFS_mkdir(path)                       ffs_mkdir(path)
#define FFS_symlink(name,path)                ffs_symlink(name,path)
#define FFS_rename(oldname,newname)           ffs_rename(oldname,newname)
#define FFS_file_write(name,addr,size,flags)  ffs_file_write(name,addr,size,flags)
#define FFS_fcreate(path,p,size)              ffs_fcreate(path,p,size)
#define FFS_fupdate(name,addr,size)           ffs_fupdate(name,addr,size)
#define FFS_fwrite(path,p,size)               ffs_fwrite(path,p,size)
#define FFS_fcontrol(path,type,param)         ffs_fcontrol(path,type,param)
#define FFS_query(query,p)                    ffs_query(query,p)
#endif  /* FFS_COAT_ENABLED */

#define FFS_Check(fct)           _FFS_StateCheck(fct,hMMI,__FILE__,__LINE__)

GLOBAL  T_FFS_RET _FFS_StateCheck (const char * const fct,
                                T_HANDLE hMMI, const char * const file, int line);

#if !defined(SYST_TRACE)
#if defined(NEW_FRAME)
#define SYST_TRACE(a) vsi_o_ttrace(0, 0xFFFF,a)
#define SYST           0, 0xffff
#define SYST_TRACE_P(a) vsi_o_ttrace a
#else
#define SYST_TRACE(a) vsi_o_trace("", 0xFFFF,a)
#define SYST           "", 0xffff
#define SYST_TRACE_P(a) vsi_o_trace a
#endif
/*
 * use it as showed next line...
 * SYST_TRACE_P((SYST, "e.g. two parameter: %d %d", p1, p2));
 */
#endif  /* !SYST_TRACE */

#endif  /* !FFS_COAT_H */

