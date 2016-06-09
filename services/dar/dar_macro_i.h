/******************************************************************************/
/*                                                                            */
/*    File Name:   dar_macro_i.h                                              */
/*                                                                            */
/*    Purpose:     This header contains the generic structures and constants  */
/*                 that are used by the Diagnose And Recovery layer.          */
/*                                                                            */
/*    Note:                                                                   */
/*        None.                                                               */
/*                                                                            */
/*    Revision History:                                                       */
/*       27 September 2001      Stephanie Gerthoux        Create              */
/*                                                                            */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved. */
/*                                                                            */
/******************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#include "dar_const_i.h"
#include "../../riviera/rvm/rvm_use_id_list.h"

#ifdef RVM_DAR_SWE
   #ifndef _DAR_MACRO_I_H
      #define _DAR_MACRO_I_H

      #ifdef __cplusplus
         extern "C"
         {
      #endif

      /* DAR trace macros */
      #define DAR_SEND_TRACE(string,type) rvf_send_trace (string,(sizeof(string)-1),NULL_PARAM,type,DAR_USE_ID)
      #define DAR_SEND_TRACE_PARAM(string,param,type) rvf_send_trace (string,(sizeof(string)-1),(UINT32)param,type,DAR_USE_ID)
      #define DAR_TRACE_WARNING(string) rvf_send_trace (string,(sizeof(string)-1),NULL_PARAM,RV_TRACE_LEVEL_WARNING,DAR_USE_ID)

      /* Macro used to calculate the value of the index */
      /* of the circular buffer */
      /* When the end of the buffer is reached, the index = 0 */
      #define DAR_PLUS_PLUS(dar_current_index) \
         { \
            dar_current_index++;\
            if (dar_current_index == DAR_MAX_BUFFER_SIZE)\
            {\
               dar_current_index = 0;\
            }\
         } \


      #ifdef __cplusplus
         }
      #endif

   #endif

#endif /* #ifdef RVM_DAR_SWE */
