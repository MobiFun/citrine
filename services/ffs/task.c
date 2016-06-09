/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * FFS task. ONLY for target!
 *
 * $Id: task.c 1.48.1.1.1.24 Thu, 18 Dec 2003 10:50:52 +0100 tsj $
 *
 ******************************************************************************/

#include "../../include/config.h"
#include "ffs.h"
#include "core.h"
#include "task.h"
#include "ffstrace.h" 
#include "intctl.h"
#include "ramffs.h"
#include "../../riviera/rvm/rvm_use_id_list.h"
#include <string.h>

/******************************************************************************
 * Globals and function prototypes
 ******************************************************************************/

extern UINT8 pcm_init(void);
void ffs_task(void);

static effs_t ffs_init_status;
req_id_t request_id_last = 0;

T_OS_MB_ID ffs_mb_id;
UINT16     ffs_addr_id;

/******************************************************************************
 * FFS Test Task
 ******************************************************************************/

// For this to work, change:
// 1. MAX_RVF_TASKS define in rvf_target.h

// Note that the FFS_TEST_TASK_ID is set one too low! We assume we are lucky
// that no other task has the same ID...

#if (WITH_TFFS == 1)

#define FFS_TEST_STACK_SIZE 1024

#define FFS_TEST_TASK_ID    (MAX_RVF_TASKS - 1 - 1)

void ffs_test_task(void);
void test_init(int keepgoing);
int  test_run(char *testname);

static uint8 ffs_test_stack[FFS_TEST_STACK_SIZE];

// This is the string of test cases to run by calling test_run(). This
// string as written to from tmffs module.
char ffs_test_string[128];

// Delay for <delay> milliseconds
void tffs_delay(int delay)
{
    delay = 14 * delay / 64; // approx. same as division by 60/13
    OS_DELAY(delay);
}

UINT32 rvf_get_tick_count(void);
// Timer functions for benchmarking
UINT32 tffs_timer_begin(void)
{
    return rvf_get_tick_count(); 
}

UINT32 tffs_timer_end(UINT32 time_begin)
{
    // return current time minus time_begin
    UINT32 ticks;

    ticks = rvf_get_tick_count();

    return (ticks - time_begin) * 60 / 13;
}

void ffs_test_task(void)
{
    effs_t error;

    OS_DELAY(217); // wait approx. 1000ms

    ttw(str(TTrTest, "ffs_test_task()" NL));

    while (1) {
        OS_DELAY(217); // wait approx. 1000ms

        // Poll to see if we have tests to run... We know that the writer of
        // ffs_test_string has a higher priority than us, so it is properly
        // written when we reach here.
        if (*ffs_test_string) {
            test_init(0);
            error = test_run(ffs_test_string);
            *ffs_test_string = 0;

            if (error == 0)
                ttw(str(TTrTest, "TEST succeeded" NL));
            else
                ttw(ttr(TTrTest, "TEST cases failed: %d" NL, error));
        }
    }
}

#endif // End of WITH_TFFS

/******************************************************************************
 * Target Platform Abstraction Functions
 ******************************************************************************/

req_id_t request_id_get(void)
{
    uint32 cpsr;

    // We disable interrupt to avoid any other tasks to get the same id.
    cpsr = int_disable();
    request_id_last++;

    if (request_id_last < 0)
        request_id_last = 0;

    int_enable(cpsr);

    return request_id_last;
}

void *target_malloc(unsigned int size)
{
    char *buf;

#if (_RVF == 1)
    if ((rvf_get_buf(ffs_mb_id, size, (T_RVF_BUFFER*) &buf)) == RVF_RED)
        return 0;
    else
        return buf;
#else
    return 0;
#endif
}

void target_free(void *buf)
{
    int error;

#if (_RVF == 1)
    if ((error = OS_FREE(buf)) != OS_OK)
        ttw(ttr(TTrFatal, "target_free() %d (FAILED)" NL, error));
#endif
}


/******************************************************************************
 * FFS Blocking Call Handling
 ******************************************************************************/

effs_t ffs_b_begin(struct ffs_blocking_s *fb, T_RVF_MUTEX *mutex, int *result)
{
    effs_t error;

    if ((error = rvf_initialize_mutex(mutex)) < 0)
    return error;

    if ((error = rvf_lock_mutex(mutex)) < 0)   // This will succeed
    return error;

    fb->result = result;
    fb->mutex = mutex;

    return EFFS_OK;
}

effs_t ffs_b_end(T_RVF_MUTEX *mutex, int result)
{
    effs_t error;

    // Do not lock the mutex if the message send operation failed
    if (result >= 0) 
        // This will block the task until the mutex has been released
        if ((error = rvf_lock_mutex(mutex)) < 0)
            return error;

    if ((error = rvf_unlock_mutex(mutex)) < 0)
        return error;

    if ((error = rvf_delete_mutex(mutex)) < 0)
        return error;

    return EFFS_OK;
}

/******************************************************************************
 * FFS Task
 ******************************************************************************/
/* The below access to the intenal Nucleus variable "TCD_Interrupt_Level" is
 * a workaround for a known Nucleus BUG (see CQ SWI-FIX-17560) */ 

void ffs_main_init() 
{
    extern int TCD_Interrupt_Level;
    int tmp_int_level;
    ttr_init(TTrTask|TTrTest|TTrTestInfo);
    //ttr_init(TTrTask|TTrTest|TTrTestInfo|TTrDrvErase|TTrDrvWrite|TTrTaskLow|TTrApi);

    tmp_int_level = TCD_Interrupt_Level;  // Backup Int level
    TCD_Interrupt_Level = 0xC0;           // The Interrups are not yet enabled..
#if FFS_IN_RAM
#if CONFIG_MOKOFFS_COPY
    bcopy_32byte_chunks(0x380000, _RAMFFS_area, 0x70000);
#else
    memset(_RAMFFS_area, 0xFF, RAMFFS_TOTAL_SIZE);
#endif
#endif
    ffs_init_status = ffs_initialize();
    TCD_Interrupt_Level = tmp_int_level;  // Restore Int level

#if 0	// moved to Application_Initialize() - FreeCalypso change
    pcm_init(); // We have to call pcm_init() before G23 starts.
#endif
}

void ffs_task_init(T_OS_MB_ID mbid, T_RVF_ADDR_ID  addr_id) 
{
    int error;

    ffs_mb_id   = mbid;
    ffs_addr_id = addr_id;

#if (WITH_TFFS == 1)
    error =
        OS_CREATE_TASK((TASKPTR) ffs_test_task, FFS_TEST_TASK_ID, "TFFS",
                       ffs_test_stack, FFS_TEST_STACK_SIZE,
                       249,                 // priority
		       4,                   // ET4_TASK type
                       0,                   // no time slicing
                       RUNNING);            // start state

    ttw(ttr(TTrTask, "os_create_task('TFFS') %d" NL, error));
#endif
}

void ffs_task()
{
    extern int etm_ffs_init(void);
    struct ffs_req_s *request;
    T_FFS_FILE_CNF *confirm_file;
    T_FFS_STREAM_CNF *confirm_stream;
    T_RVF_MUTEX *mutex_p;
    int error, free_mail, os_error = OS_OK;
    uint16 revision, manufacturer_id, device_id;
    uint32 base;
    fd_t fdi;
    char *temp_path;
    req_id_t temp_id;

#if FFS_IN_RAM && !CONFIG_MOKOFFS_COPY
    // Non formatted FFS should be formatted
    // So we don't have to use PCTM to format it
    if (fs.initerror == EFFS_NOFORMAT)
    {
        ffs_format_nb("/", 0x2BAD, 0);
    }

    /* ffs_InitRFCap(); */
#endif

    ttr(TTrTask, "ffs_init() %d" NL, ffs_init_status);

    ffs_query(Q_FFS_REVISION, &revision);
    ttr(TTrTask,"FFS revision: 0x%x" NL, revision);

    ffs_query(Q_DEV_MANUFACTURER, &manufacturer_id);
    ffs_query(Q_DEV_DEVICE,       &device_id);
    ffs_query(Q_DEV_DRIVER,       &revision);
    ttr(TTrTask,"FFS device, driver: 0x%02x, 0x%04x, %d" NL,
        manufacturer_id, device_id, revision);

    ffs_query(Q_DEV_BASE,   &base);
    ffs_query(Q_DEV_BLOCKS, &revision);
    ttr(TTrTask,"FFS base, blocks: 0x%x, %d" NL, base, revision);

    ffs_query(Q_FFS_FORMAT_READ,  &manufacturer_id);
    ffs_query(Q_FFS_FORMAT_WRITE, &device_id);
    ttr(TTrTask,"FFS format read, write: 0x%x, 0x%x" NL NL,
        manufacturer_id, device_id);

    // If some blocks has been marked for reclaim, reclaim them now...
    blocks_reclaim();

    // We can only mkdir("pcm") *after* our mailbox has been allocated
    // otherwise mkdir("pcm") will fail.
    error = ffs_mkdir_nb("/pcm", 0);

    // Register FFS to ETM database
    error = etm_ffs_init();

    while (1)
    {
        OS_MAIL_WAIT(OS_MAIL_EVENT_MASK);
        request = (struct ffs_req_s *) OS_MAIL_READ();

        ttw(ttr(TTrTaskLow, "ffs_task(%d):" NL, request->cmd));
        switch (request->cmd) {
        case WRITE: case SEEK: case CLOSE: case FTRUNC: case FDATASYNC:
            ttw(ttr(TTrTaskLow, "  fdi  = %d" NL, request->fdi));
            // Save a local copy of fdi because the task we call later can
            // modify it and we have to use the fdi if a callback is used
            fdi = (fd_t)request->fdi; 
            break;
        default:
            ttw(ttr(TTrTaskLow, "  name = %s" NL, request->path));
            break;
        }
        ttw(ttr(TTrTaskLow, "  src  = 0x%x" NL, request->src));
        ttw(ttr(TTrTaskLow, "  size = %d" NL, request->size));

        if (tr_query(TTrTaskDelays))
            OS_DELAY(5);

        switch (request->cmd) {
        case NOP:        error = EFFS_OK;                  break;
        case FILE_WRITE: error = task_file_write(request); break;
        case SYMLINK:    error = task_symlink(request);    break;
        case MKDIR:      error = task_mkdir(request);      break;
        case REMOVE:     error = task_remove(request);     break;
        case RENAME:     error = task_rename(request);     break;
        case FCONTROL:   error = task_fcontrol(request);   break;
        case PREFORMAT:  error = task_preformat(request);  break;
        case FORMAT:     error = task_format(request);     break;
        case OPEN:       error = task_open(request);       break;
        case WRITE:      error = task_write(request);      break;
        case SEEK:       error = task_seek(request);       break;
        case CLOSE:      error = task_close(request);      break;
        case TRUNC:      error = task_trunc(request);      break;
        case FTRUNC:     error = task_ftrunc(request);     break;
        case FDATASYNC:  error = task_fdatasync(request);  break;
        default:
            ttr(TTrFatal, "FFS FATAL: bad request: %d" NL, request->cmd);
            break;
        }

        ttw(ttr(TTrTaskLow, "ffs_task(%d) %d" NL, request->cmd, error));
        if (tr_query(TTrTaskDelays))
            OS_DELAY(5);

        // Call-back to caller
        mutex_p = 0;
        if (request->cp) { 
            free_mail = 0;  // don't free mail we will reuse it
            // We reuse the mail we received for our call-back. Due to
            // this reuse, we must be careful with the order of
            // assignments. If this is a stream modify function use
            // ffs_stream_cnf else use ffs_file_cnf  
            
            temp_id = request->request_id;  // Save id before we reuse the mail mem.
            switch (request->cmd) {
            case WRITE: case SEEK: case CLOSE: case FTRUNC: case FDATASYNC:
                confirm_stream = (T_FFS_STREAM_CNF *) request;
                confirm_stream->error = error;
                confirm_stream->request_id = temp_id; 
                confirm_stream->fdi = fdi; 
                confirm_stream->header.msg_id = FFS_MESSAGE_OFFSET;
            
                if (request->cp->callback_func) {
                    request->cp->callback_func((T_FFS_STREAM_CNF *) 
                                               confirm_stream);
                }
                else if (request->cp->addr_id) {
                    os_error = OS_MAIL_SEND(request->cp->addr_id, 
                                            confirm_stream);
                }
                else {
                    ttr(TTrFatal, "FFS WARNING: empty return path" NL);
                    free_mail = 1;  // free mail
                }
                break;
            default:
                temp_path = (char *) request->path;
                confirm_file = (T_FFS_FILE_CNF *) request;
                confirm_file->error = error;
                confirm_file->request_id = temp_id;
                confirm_file->path = temp_path;
                confirm_file->header.msg_id = FFS_MESSAGE_OFFSET;

                if (request->cp->callback_func) {
                    request->cp->callback_func((T_FFS_FILE_CNF *) confirm_file);
                }   

                else if (request->cp->addr_id) {
                    os_error = OS_MAIL_SEND(request->cp->addr_id, 
                                            confirm_file);
                }
                else {
                    ttr(TTrFatal, "FFS WARNING: empty return path" NL);
                    free_mail = 1;  // free mail
                }
                break;
            }
            
            if (os_error != OS_OK)
                ttr(TTrFatal, "FFS FATAL: os_send_msg() %d" NL, os_error);
        }
            
        // Blocking handling / unlocking mutex
        else if (request->fb) {   
            // Save the pointer to the mutex and the error value (allocated
            // on the stack by the macro FFS_BLOCKING_BEGIN)
            mutex_p = request->fb->mutex;
            *request->fb->result = error;  
            free_mail = 1;  // free mail
        }

        else {
            // The task must have been a non blocking without any callback
            free_mail = 1; // free mail
        }

        // Free the mail memory. Note that we always free it, except when we
        // have successfully reused it for sending a mail call-back.
        if (free_mail == 1) {
            os_error = OS_FREE(request);
            if (os_error != OS_OK)
                ttr(TTrFatal, "FFS FATAL: os_free() %d" NL, os_error);
        }

        // Blocking handling / unlocking mutex
        if (mutex_p) {   
            // Wake up the caller task
            if ((os_error = rvf_unlock_mutex(mutex_p)) < 0)
                ttr(TTrFatal, "FFS FATAL: rvf_unlock_mutex() %d" NL, os_error);
        }
        
        tr_bstat();
    }
}
