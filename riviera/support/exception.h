/*
 ******************************
 * 
 *
 * Initialial version: Laurent Deniau, Laurent.Deniau@cern.ch
 *
 * For more information, please see the paper:
 * http://cern.ch/Laurent.Deniau/html/oopc/exception.html
 * 
 * -----------------------------------------------------------
 *
 * Strong rework and adaption to riviera by Christophe Favergeon
 *
 ******************************
 */

#ifndef RVF_EXCEPTION_H
#define RVF_EXCEPTION_H

#include "../rvf/rvf_api.h"
#include "../rvf/rvf_target.h"

#include <string.h>



//#ifndef __STDC__
//#  error "exception.h needs ISO C compiler to work properly"
//#endif

#include <setjmp.h>

typedef enum K_RVF_EXCEPTIONS { E_invalid=1, E_not_enough_memory,E_unknown } T_RVF_EXCEPTIONS; 

typedef void (*T_RVF_RELEASE_PROTECTED_POINTER)(void *p);

/*
  choose context savings
*/

#define rvf_save_context_buffer_(context)         setjmp(context)
#define rvf_restore_context_buffer_(context, val) longjmp(context, val)

/*
  some hidden types used to handle exceptions
*/

/* type of stack of protected pointer */
struct _protectedPtr_ {
  struct _protectedPtr_ *next;
  struct _protectedPtr_ *previous;
  void *ptr;
  T_RVF_RELEASE_PROTECTED_POINTER func;
};

/* type of stack of exception */
struct _exceptionContext_ {
  struct _exceptionContext_ *next;
  struct _protectedPtr_ *stack;
  jmp_buf context;
};

extern struct _exceptionContext_ *const _returnExceptionContext_[MAX_RVF_TASKS];
extern struct _exceptionContext_ *_currentExceptionContext_[MAX_RVF_TASKS];

/* exception keywords */
#define try								 \
  do {									 \
	struct _exceptionContext_ _localExceptionContext_ ;\
	memset(&_localExceptionContext_,0,sizeof(struct _exceptionContext_));\
    _localExceptionContext_.next=_currentExceptionContext_[rvf_get_taskid()];\
    _currentExceptionContext_[rvf_get_taskid()] = &_localExceptionContext_;		 \
    do {								 \
      int const exception =						 \
              rvf_save_context_buffer_(_currentExceptionContext_[rvf_get_taskid()]->context); \
      if (!exception) {

#define catch(except)							\
      } else if ((int)(except) == exception) {				\
        _currentExceptionContext_[rvf_get_taskid()] = _currentExceptionContext_[rvf_get_taskid()]->next;

#define catch_any							\
      } else {								\
        _currentExceptionContext_[rvf_get_taskid()] = _currentExceptionContext_[rvf_get_taskid()]->next;

#define endtry								\
      }									\
    } while(0);								\
    if (_currentExceptionContext_[rvf_get_taskid()] == &_localExceptionContext_) {	\
      rvf_forget_protected_ptr();\
      _currentExceptionContext_[rvf_get_taskid()] = _currentExceptionContext_[rvf_get_taskid()]->next;	\
    }									\
  } while(0)

#define rethrow throw(exception)
#define break_try break

/*
#define return_try(...)						\
  do {								\
    _currentExceptionContext_ = _returnExceptionContext_;	\
    return __VA_ARGS__;						\
  } while(0)
*/

#define throw(except) _exceptionThrow_((int)(except))


/*
  extern declarations
*/

extern void _exceptionThrow_(int except);

extern void rvf_forget_protected_ptr();
extern void rvf_protect_pointer(T_RVF_MB_ID mb_id,void *p,T_RVF_RELEASE_PROTECTED_POINTER func);

#define RVF_PROTECT(bank,p) rvf_protect_pointer(bank,p,(T_RVF_RELEASE_PROTECTED_POINTER)rvf_free_buf)

#define THROW_IF_ERROR(err) if (err!=0) throw(E_unknown)
#define THROW_IF_NULL(p) if (p==NULL) throw(E_not_enough_memory)
#define THROW_IF_YELLOW(b,p) if ((p==NULL) || (rvf_get_mb_status(b)==RVF_YELLOW)) throw(E_not_enough_memory)
#endif
