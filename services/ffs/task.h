/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * ffs task
 *
 * $Id: task.h 1.23.1.17 Wed, 29 Oct 2003 12:54:27 +0100 tsj $
 *
 ******************************************************************************/

#include "ffs.h"


#define OS_FREE(addr) rvf_free_buf(addr)
#define OS_MAIL_WAIT(event) rvf_wait(event, 0)
#define OS_MAIL_READ() rvf_read_mbox(RVF_TASK_MBOX_0)
#define OS_MAIL_SEND(dest, data) rvf_send_msg(dest, data);
#define OS_DELAY(time) rvf_delay(time)
#define OS_CREATE_TASK rvf_create_task

#define OS_OK RV_OK
#define OS_TASK_MBOX_0 RVF_TASK_MBOX_0
#define OS_MAIL_EVENT_MASK RVF_TASK_MBOX_0_EVT_MASK
#define T_OS_MB_ID T_RVF_MB_ID

/******************************************************************************
 * Globals
 ******************************************************************************/

extern T_OS_MB_ID ffs_mb_id;
extern UINT16	  ffs_addr_id;
void ffs_task(void);

/******************************************************************************
 * Macros
 ******************************************************************************/

#define MSG_ALLOC(mail) if ((rvf_get_buf(ffs_mb_id, sizeof(struct ffs_req_s), \
                                         (T_RVF_BUFFER*) &mail)) == RVF_RED) { \
                                  return EFFS_MEMORY; \
                        }

#define MSG_SEND(mail)  if (rvf_send_msg(ffs_addr_id, mail)) { \
                            return EFFS_MSGSEND; \
                        } 
	                   

#define FFS_BLOCKING_CALL_BEGIN() \
    int result, error; \
    struct ffs_blocking_s fb; \
    T_RVF_MUTEX mutex; \
    if ((error = ffs_b_begin(&fb, &mutex, &result)) < 0) \
        return error;

// The result variable contains the output from the ffs_xx_b()
// function. After the requested task has been executed is the contents in
// result replaced with the output from that task
#define FFS_BLOCKING_CALL_END() \
     if ((error = ffs_b_end(&mutex, result)) < 0) \
        return error;

/******************************************************************************
 * Target Platform Abstraction Functions
 ******************************************************************************/

req_id_t request_id_get(void);
void *target_malloc(unsigned int size);


/******************************************************************************
 * Types
 ******************************************************************************/

// In and output variables to blocking functions
struct ffs_blocking_s {
    int *result;
    T_RVF_MUTEX *mutex;
};

// Structure of operation request mail sent from targetffs.c to FFS task.
struct ffs_req_s {
    T_RV_HDR   header;
    const char *path;
    char       *src;
    int        size;
    uint16     value16;
    req_id_t   request_id;
    char       cmd;
    fd_t       fdi;
    T_RV_RETURN *cp;
    struct ffs_blocking_s *fb;
};

// NOTEME: should we use the indices from tmffs.h instead?
typedef enum FFS_REQ_CMD {
    NOP = 0,
    PREFORMAT,
    FORMAT,
    FILE_WRITE,
    REMOVE,
    RENAME,
    MKDIR,
    SYMLINK,
    FCONTROL,
    OPEN,
    WRITE,
    CLOSE,
    SEEK,
    TRUNC,
    FTRUNC,
    FDATASYNC
} ffs_req_e;

effs_t task_file_write(struct ffs_req_s *req);
effs_t task_symlink(struct ffs_req_s *req);
effs_t task_format(struct ffs_req_s *req);
effs_t task_preformat(struct ffs_req_s *req);
effs_t task_mkdir(struct ffs_req_s *req);
effs_t task_remove(struct ffs_req_s *req);
effs_t task_fcontrol(struct ffs_req_s *req);
effs_t task_rename(struct ffs_req_s *p);
fd_t   task_open(struct ffs_req_s *req);
int    task_write(struct ffs_req_s *req);
effs_t task_close(struct ffs_req_s *req);
int    task_seek(struct ffs_req_s *req);
effs_t task_trunc(struct ffs_req_s *req);
effs_t task_ftrunc(struct ffs_req_s *req);
effs_t task_fdatasync(struct ffs_req_s *req);

effs_t ffs_b_begin(struct ffs_blocking_s *fb, T_RVF_MUTEX *mutex, int *result); 
effs_t ffs_b_end(T_RVF_MUTEX *mutex, int result); 
void target_free(void *buf); 

