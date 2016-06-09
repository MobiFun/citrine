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

#ifndef __FFS_COAT_C__
#define __FFS_COAT_C__

/*==== INCLUDES ===================================================*/
#define SAP_ACI

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#include "typedefs.h"
#include "pconst.cdg"
#include "prim.h"
#include "vsi.h"
#include "../../services/ffs/ffs.h"
#include "ffs_coat.h"


/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== CONSTANTS ==================================================*/
//#define FFS_STATE_CHECK
#define FFS_PAR_CHECK
#define FFS_ERR_CHECK

/*==== EXTERNALS ==================================================*/

/*==== VARIABLES ==================================================*/

/*==== MACROS =====================================================*/
#if defined(FFS_STATE_CHECK)
#define _STATE_CHECK(ffs_ret)\
  {ffs_ret=_FFS_StateCheck (_FFS_FCT_NAME,hMMI,file,line);\
   if (ffs_ret NEQ EFFS_OK)return ffs_ret;}
#else /* FFS_PAR_CHECK */
#define _STATE_CHECK(ffs_ret)
#endif /* FFS_PAR_CHECK */

#if defined(FFS_PAR_CHECK)
#define _PAR_CHECK_NULL(parname,parnumber,ret)\
  if (!parname){\
    _FFS_ParCheck ("NULL", _FFS_FCT_NAME, parnumber, hMMI,file, line);\
    return ret;}

#define _PAR_CHECK_LESSZERO(parname,parnumber,ret)\
  if (parname < 0){\
    _FFS_ParCheck ("<0", _FFS_FCT_NAME, parnumber, hMMI,file, line);\
    return ret;}
#else /* FFS_PAR_CHECK */
#define _PAR_CHECK_NULL(parname,parnumber,ret)
#define _PAR_CHECK_LESSZERO(parname,parnumber,ret)
#endif /* FFS_PAR_CHECK */


#if defined(FFS_ERR_CHECK)
#define _ERROR_CHECK(ffs_ret)\
  if (ffs_ret < 0)_FFS_ErrorMsg(NULL, _FFS_FCT_NAME, ffs_ret, hMMI,file, line)
#else  /* FFS_ERR_CHECK */
#define _ERROR_CHECK(ffs_ret)
#endif  /* FFS_ERR_CHECK */

/*==== FUNCTIONS ==================================================*/
#if defined(FFS_COAT_ENABLED)
static  void _FFS_ParCheck (const char * const errmsg, const char * const fct,
                          int parameter,
                          T_HANDLE hMMI, const char * const file, int line);

static  void _FFS_ErrorMsg (const char * const errmsg, const char * const fct,
                          int ffs_ret,
                          T_HANDLE hMMI, const char * const file, int line);

/* FFS functions whose positive return value has a meaning */
T_FFS_FD  _FFS_open(const char *name, T_FFS_OPEN_FLAGS option,
                   T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_open"
T_FFS_FD  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_BADNAME);

  ffs_ret = ffs_open (name, option);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}

T_FFS_SIZE _FFS_write(T_FFS_FD fdi, void *src, T_FFS_SIZE amount,
                    T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_write"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (src, 2, EFFS_BADOP);

  ffs_ret = ffs_write (fdi, src, amount);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_SIZE _FFS_read(T_FFS_FD fdi, void *src, T_FFS_SIZE size,
                    T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_read"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (src, 2, EFFS_BADOP);

  ffs_ret = ffs_read (fdi, src, size);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_SIZE _FFS_seek(T_FFS_FD fdi, T_FFS_SIZE offset, T_FFS_WHENCE whence,
                    T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_seek"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_LESSZERO (offset, 2, EFFS_BADOP);

  ffs_ret = ffs_seek (fdi, offset, whence);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_SIZE _FFS_opendir(const char *name, T_FFS_DIR *dir,
                      T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_opendir"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_NOTADIR);
  _PAR_CHECK_NULL (dir, 2, EFFS_NOTADIR);

  ffs_ret = ffs_opendir (name, dir);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_SIZE _FFS_readdir (T_FFS_DIR *dir, char *name, T_FFS_SIZE size,
                      T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_readdir"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (dir, 1, EFFS_NOTADIR);
  _PAR_CHECK_NULL (name, 2, EFFS_NOTADIR);
  _PAR_CHECK_LESSZERO (size, 3, EFFS_NOTADIR);

  ffs_ret = ffs_readdir (dir, name, size);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_SIZE _FFS_readlink(const char *name, char *addr, T_FFS_SIZE size,
                      T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_readlink"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_NOTAFILE);
  _PAR_CHECK_NULL (addr, 2, EFFS_NOTAFILE);
  _PAR_CHECK_LESSZERO (size, 3, EFFS_FILETOOBIG);

  ffs_ret = ffs_readlink (name, addr, size);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_SIZE _FFS_file_read(const char *name, void *addr, T_FFS_SIZE size,
                       T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_file_read"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_NOTAFILE);
  _PAR_CHECK_NULL (addr, 2, EFFS_NOTAFILE);
  _PAR_CHECK_LESSZERO (size, 3, EFFS_FILETOOBIG);

  ffs_ret = ffs_file_read (name, addr, size);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_SIZE _FFS_fread(const char *name, void *addr, T_FFS_SIZE size,
                    T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_fread"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_NOTAFILE);
  _PAR_CHECK_NULL (addr, 2, EFFS_NOTAFILE);
  _PAR_CHECK_LESSZERO (size, 3, EFFS_FILETOOBIG);

  /* ffs_fread is deprecated, use ffs_file_read instead */
  ffs_ret = ffs_file_read (name, addr, size);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}



/* FFS functions which return a value EFFS_OK if OK */
T_FFS_RET _FFS_preformat(UINT16 magic,
                      T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_preformat"
T_FFS_SIZE  ffs_ret;

  ffs_ret = ffs_preformat (magic);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_format(const char *name, UINT16 magic,
                    T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_format"
T_FFS_SIZE  ffs_ret;

  _PAR_CHECK_NULL (name, 1, EFFS_BADNAME);

  ffs_ret = ffs_format (name, magic);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_close(T_FFS_FD fdi,
                   T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_close"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);

  ffs_ret = ffs_close (fdi);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_truncate(const char *path, T_FFS_OFFSET length,
                     T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_truncate"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (path, 1, EFFS_NOTFOUND);
  _PAR_CHECK_LESSZERO (length, 2, EFFS_NOTFOUND);

  ffs_ret = ffs_truncate (path, length);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_ftruncate(T_FFS_FD fdi, T_FFS_OFFSET length,
                      T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_ftruncate"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (fdi, 1, EFFS_BADFD);
  _PAR_CHECK_LESSZERO (length, 2, EFFS_NOTFOUND);

  ffs_ret = ffs_ftruncate (fdi, length);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_fdatasync(T_FFS_FD fdi,
                      T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_fdatasync"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (fdi, 1, EFFS_BADFD);

  ffs_ret = ffs_fdatasync (fdi);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_stat(const char *name, T_FFS_STAT *stat,
                  T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_stat"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_NOTFOUND);
  _PAR_CHECK_NULL (stat, 2, EFFS_NOTFOUND);

  ffs_ret = ffs_stat (name, stat);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_fstat(T_FFS_FD fdi, T_FFS_STAT *stat,
                   T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_fstat"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (fdi, 1, EFFS_BADFD);
  _PAR_CHECK_NULL (stat, 2, EFFS_NOTFOUND);

  ffs_ret = ffs_fstat (fdi, stat);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_lstat(const char *name, T_FFS_STAT *stat,
                   T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_lstat"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_NOTFOUND);
  _PAR_CHECK_NULL (stat, 2, EFFS_NOTFOUND);

  ffs_ret = ffs_lstat (name, stat);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_xlstat(const char *name, T_FFS_XSTAT *stat,
                    T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_xlstat"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_NOTFOUND);
  _PAR_CHECK_NULL (stat, 2, EFFS_NOTFOUND);

  ffs_ret = ffs_xlstat (name, stat);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_linkstat(const char *name, T_FFS_STAT *stat,
                     T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_linkstat"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_NOTFOUND);
  _PAR_CHECK_NULL (stat, 2, EFFS_NOTFOUND);

  /* ffs_linkstat is deprecated, use ffs_lstat instead */
  ffs_ret = ffs_lstat (name, stat);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_remove(const char *name,
                     T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_remove"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_NOTFOUND);

  ffs_ret = ffs_remove (name);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_mkdir(const char *name,
                    T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_mkdir"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_BADNAME);

  ffs_ret = ffs_mkdir (name);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_symlink(const char *name, const char *actualpath,
                     T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_symlink"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_BADNAME);
  _PAR_CHECK_NULL (actualpath, 2, EFFS_BADNAME);

  ffs_ret = ffs_symlink (name, actualpath);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_rename(const char *oldname, const char *newname,
                     T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_rename"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (oldname, 1, EFFS_BADNAME);
  _PAR_CHECK_NULL (newname, 2, EFFS_BADNAME);

  ffs_ret = ffs_rename (oldname, newname);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_file_write(const char *name, void *addr, T_FFS_SIZE size,
                      T_FFS_OPEN_FLAGS flags,
                      T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_file_write"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_BADNAME);
  _PAR_CHECK_NULL (addr, 2, EFFS_NOTAFILE);
  _PAR_CHECK_LESSZERO (size, 2, EFFS_FILETOOBIG);

  ffs_ret = ffs_file_write (name, addr, size, flags);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_fcreate(const char *name, void *addr, T_FFS_SIZE size,
                     T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_fcreate"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_BADNAME);
  _PAR_CHECK_NULL (addr, 2, EFFS_NOTAFILE);
  _PAR_CHECK_LESSZERO (size, 2, EFFS_FILETOOBIG);

  /* ffs_fcreate is deprecated, use ffs_file_write instead */
  ffs_ret = ffs_file_write (name, addr, size, FFS_O_CREATE|FFS_O_EXCL);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_fupdate(const char *name, void *addr, T_FFS_SIZE size,
                     T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_fupdate"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_BADNAME);
  _PAR_CHECK_NULL (addr, 2, EFFS_NOTAFILE);
  _PAR_CHECK_LESSZERO (size, 2, EFFS_FILETOOBIG);

  /* ffs_fupdate is deprecated, use ffs_file_write instead */
  ffs_ret = ffs_file_write (name, addr, size, FFS_O_TRUNC);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_fwrite(const char *name, void *addr, T_FFS_SIZE size,
                    T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_fwrite"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (name, 1, EFFS_BADNAME);
  _PAR_CHECK_NULL (addr, 2, EFFS_NOTAFILE);
  _PAR_CHECK_LESSZERO (size, 2, EFFS_FILETOOBIG);

  /* ffs_fwrite is deprecated, use ffs_file_write instead */
  ffs_ret = ffs_file_write (name, addr, size, FFS_O_CREATE|FFS_O_TRUNC);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_fcontrol(const char *pathname, INT8 action, int param,
                     T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_fcontrol"
T_FFS_SIZE  ffs_ret;

  _STATE_CHECK (ffs_ret);
  _PAR_CHECK_NULL (pathname, 1, EFFS_BADNAME);

  ffs_ret = ffs_fcontrol (pathname, action, param);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}


T_FFS_RET _FFS_query(INT8 query, void *p,
                   T_HANDLE hMMI, const char * const file, int line)
{
#define   _FFS_FCT_NAME "ffs_query"
T_FFS_SIZE  ffs_ret;

  _PAR_CHECK_NULL (p, 2, EFFS_BADNAME);

  ffs_ret = ffs_query (query, p);
  _ERROR_CHECK (ffs_ret);

  return ffs_ret;
#undef   _FFS_FCT_NAME
}

#if defined(FFS_PAR_CHECK)
static  void _FFS_ParCheck (const char * const  errmsg, const char * const  fct,
                          int parameter,
                          T_HANDLE hMMI, const char * const file, int line)
{
  char *f;
  int   l;

  l = strlen (file);
  if (l <= 20)
    f = (char *)file;
  else
    f = (char *)file + l - 20;

  TRACE_EVENT_P5 ("FFS PAR ERR:par %u of %s is %s (%s#%u)",
                  parameter, 
                  fct ? fct : "", 
                  errmsg ? errmsg : "", 
                  f, 
                  line);
}
#endif  /* FFS_PAR_CHECK */

char *ffs_strerror(effs_t error)
{
    switch (error) {
    case EFFS_OK: return "ok"; /* 0 */
    case EFFS_NODEVICE: return "flash device unknown"; /* -1 */
    case EFFS_CORRUPTED: return "filesystem corrupted!?"; /* -2 */
    case EFFS_NOPREFORMAT: return "ffs not preformatted"; /* -3 */
    case EFFS_NOFORMAT: return "ffs not formatted"; /* -4 */
    case EFFS_BADFORMAT: return "incompatible ffs version"; /* -5 */
    case EFFS_MAGIC: return "bad magic"; /* -6 */
    case EFFS_AGAIN: return "not ready, try again later"; /* -7 */
    case EFFS_NOSYS: return "function not implemented"; /* -8 */
    case EFFS_DRIVER: return "ffs device driver error"; /* -9 */
    case EFFS_NOSPACE: return "out of data space"; /* -10 */
    case EFFS_FSFULL: return "file system full, no free inodes"; /* -11 */
    case EFFS_BADNAME: return "bad filename"; /* -12 */
    case EFFS_NOTFOUND: return "object not found"; /* -13 */
    case EFFS_EXISTS: return "object exists"; /* -14 */
    case EFFS_ACCESS: return "access permission violation"; /* -15 */
    case EFFS_NAMETOOLONG: return "filename too long"; /* -16 */
    case EFFS_INVALID: return "invalid argument"; /* -17 */
    case EFFS_DIRNOTEMPTY: return "directory not empty"; /* -18 */
    case EFFS_NOTADIR: return "object is not a directory"; /* -19 */
    case EFFS_SPARE: return "SPARE"; /* -20 */
    case EFFS_FILETOOBIG: return "file too big"; /* -21 */
    case EFFS_NOTAFILE: return "object is not a file"; /* -22 */
    case EFFS_PATHTOODEEP: return "path too deep"; /* -23 */
    case EFFS_NUMFD: return "Max number of open files reached"; /* -24 */
    case EFFS_BADFD: return "Bad file descriptor"; /* -25 */
    case EFFS_BADOP: return "Bad operation"; /* -26 */
    case EFFS_LOCKED: return "The file is locked"; /* -27 */
    case EFFS_TOOBIG: return "too big (tmffs buffer overflow)"; /* -30 */
    case EFFS_MEMORY: return "out of memory"; /* -31 */
    case EFFS_MSGSEND: return "message send failed"; /* -32 */
    case EFFS_SIBLINGLOOP: return "directory sibling loop"; /* -40 */
    case EFFS_NOBLOCKS: return "No more blocks!?"; /* -41 */
    default: return "unknown ffs error code!";
    }
}

static  void _FFS_ErrorMsg (const char * const  errmsg, const char * const  fct,
                          int ffs_ret,
                          T_HANDLE hMMI, const char * const file, int line)
{
  char *f;
  int   l;

  l = strlen (file);
  if (l <= 20)
    f = (char *)file;
  else
    f = (char *)file + l - 20;

  TRACE_EVENT_P5 ("FFS ERR on %s: %d %s (%s#%u)",
                  fct ? fct : "", 
                  ffs_ret, 
                  ffs_strerror (ffs_ret), 
                  f, 
                  line);
}

#endif  /* FFS_COAT_ENABLED */

#if 1
GLOBAL  T_FFS_RET _FFS_StateCheck (const char * const fct,
                                T_HANDLE hMMI, const char * const file, int line)
{
  int query_result;
  int bytes_free, bytes_used, bytes_max, bytes_lost;
  T_FFS_RET ffs_ret;

  SYST_TRACE ("_FFS_StateCheck()");

  bytes_free = bytes_used = bytes_max = bytes_lost = 0;

  ffs_ret = ffs_query (Q_BYTES_FREE, &query_result);
  if (ffs_ret EQ EFFS_OK)
  {
    bytes_free = query_result;
    ffs_ret = ffs_query (Q_BYTES_USED, &query_result);
    if (ffs_ret EQ EFFS_OK)
    {
      bytes_used = query_result;
      ffs_ret = ffs_query (Q_BYTES_LOST, &query_result);
      if (ffs_ret EQ EFFS_OK)
      {
        bytes_lost = query_result;
        ffs_ret = ffs_query (Q_BYTES_MAX, &query_result);
        if (ffs_ret EQ EFFS_OK)
        {
          bytes_max = query_result;
        }
      }
    }
  }

  #if 0
    {
      char *f;
      int   l;

      PALLOC (trc_ind, ACI_TRC_IND); /* T_ACI_TRC_IND */
      trc_ind->cmd_src = 1; /* CMD_SRC_ATI_1 */

      l = strlen (file);
      if (l <= 20)
        f = (char *)file;
      else
        f = (char *)file + l - 20;

      sprintf (trc_ind->trc_buf, "STATE before %s (%s#%u)", fct?fct:"", f, line);
      trc_ind->trc_len = strlen (trc_ind->trc_buf);

      PSEND (hMMI, trc_ind);
    }
  #endif  /* 0|1 */
  {
    TRACE_EVENT_P4 ("FFS free=%5u used=%5u lost=%5u max=%6u",
                    bytes_free,
                    bytes_used, 
                    bytes_lost,
                    bytes_max);
  }

  return EFFS_OK;
}
#else /* 1|0 */
GLOBAL  T_FFS_RET _FFS_StateCheck (const char * const fct,
                                T_HANDLE hMMI, const char * const file, int line)
{
  USHORT query_result;
  int objects_free, objects_used, objects_max, objects_lost;
  T_FFS_RET ffs_ret;

  SYST_TRACE ("_FFS_StateCheck()");

  ffs_ret = ffs_query (Q_OBJECTS_FREE, &query_result);
  if (ffs_ret NEQ EFFS_OK)
    return ffs_ret;
  else
    objects_free = query_result;

  ffs_ret = ffs_query (Q_INODES_USED, &query_result);
  if (ffs_ret NEQ EFFS_OK)
    return ffs_ret;
  else
    objects_used = query_result;

  ffs_ret = ffs_query (Q_INODES_LOST, &query_result);
  if (ffs_ret NEQ EFFS_OK)
    return ffs_ret;
  else
    objects_lost = query_result;

  ffs_ret = ffs_query (Q_OBJECTS_MAX, &query_result);
  if (ffs_ret NEQ EFFS_OK)
    return ffs_ret;
  else
    objects_max = query_result;

  #if 0
    {
      char *f;
      int   l;

      PALLOC (trc_ind, ACI_TRC_IND); /* T_ACI_TRC_IND */
      trc_ind->cmd_src = 1; /* CMD_SRC_ATI_1 */

      l = strlen (file);
      if (l <= 20)
        f = (char *)file;
      else
        f = (char *)file + l - 20;

      sprintf (trc_ind->trc_buf, "STATE before %s (%s#%u)", fct?fct:"", f, line);
      trc_ind->trc_len = strlen (trc_ind->trc_buf);

      PSEND (hMMI, trc_ind);
    }
  #endif  /* 0|1 */
  {
    PALLOC (trc_ind, ACI_TRC_IND); /* T_ACI_TRC_IND */
    trc_ind->cmd_src = 1; /* CMD_SRC_ATI_1 */

    sprintf (trc_ind->trc_buf, "FFS objs: free=%3u used=%3u lost=%3u max=%3u",
      objects_free, objects_used, objects_lost, objects_max);
    trc_ind->trc_len = strlen (trc_ind->trc_buf);

    SYST_TRACE ((char *)trc_ind->trc_buf);

    PSEND (hMMI, trc_ind);
  }

  return EFFS_OK;
}
#endif /* 1|0 */

#endif
