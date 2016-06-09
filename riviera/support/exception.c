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

// Authorization to use this source code communicated to Christophe Favergeon
// by email

#include "../../include/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "exception.h"


/* global stack of exception context */
struct _exceptionContext_ *const _returnExceptionContext_[MAX_RVF_TASKS]=
#if (!GSMLITE)
  {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
#else
  {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
#endif

struct _exceptionContext_ *_currentExceptionContext_ [MAX_RVF_TASKS]= 
#if (!GSMLITE)
  {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
#else
  {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
#endif

/* delete protected pointers and throw exception */
void
_exceptionThrow_(int exception)
{
  struct _protectedPtr_ *p;
  struct _protectedPtr_ *oldp;

  /* no exception context saved, exit program */
  if (!_currentExceptionContext_[rvf_get_taskid()]) exit(exception); 

  /* free pointers stored on the current exception context pointers stack */
  p=_currentExceptionContext_[rvf_get_taskid()]->stack;

  while(p)
  {
	  oldp=p->previous;
	  p->func(p->ptr);
	  rvf_free_buf(p);
	  p=oldp;
  }

  _currentExceptionContext_[rvf_get_taskid()]->stack=NULL;

  /* jump to previous exception context */
  rvf_restore_context_buffer_(_currentExceptionContext_[rvf_get_taskid()]->context, exception); 
} 


// Protect a pointer when there is a try/ctahc block active
void rvf_protect_pointer(T_RVF_MB_ID mb_id,void *p,T_RVF_RELEASE_PROTECTED_POINTER func)
{

	struct _protectedPtr_ *ptr;
	struct _exceptionContext_ *context;
	T_RVF_MB_STATUS err;



	if (_currentExceptionContext_[rvf_get_taskid()])
	{

	if (p==NULL)
		throw(E_not_enough_memory);

	context=_currentExceptionContext_[rvf_get_taskid()];
	err=rvf_get_buf(mb_id,sizeof(struct _protectedPtr_),(void*)&ptr);
	

	if (err==RVF_GREEN)
	{
		ptr->next=NULL;
	    ptr->previous=NULL;
	    ptr->ptr=p;
	    ptr->func=func;

         if (context->stack==NULL)
		 {
			  context->stack=ptr;
		 }
		 else
		 {
			  ptr->previous=context->stack;
			  context->stack->next=ptr;
			  context->stack=ptr;
		 }
	 }
	 else
	 {
		 if (p!=NULL)
		   rvf_free_buf(p);
		 throw(E_not_enough_memory);
	 }
	}
}

void rvf_forget_protected_ptr()
{
   struct _protectedPtr_ *p;
   struct _protectedPtr_ *oldp;

   p=_currentExceptionContext_[rvf_get_taskid()]->stack;

   while(p)
   {
	   oldp=p->previous;
	   rvf_free_buf(p);
	   p=oldp;
   }

   _currentExceptionContext_[rvf_get_taskid()]->stack=NULL;
}
