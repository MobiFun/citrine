/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_msg_ft.h                                                */
/*                                                                          */
/*  Purpose:   This function contains the functions prototypes of the DAR   */ 
/*             entity messages functions.                                   */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date               Modification                                         */
/*  ------------------------------------                                    */
/*  17 October 2001    Create                                               */
/*                                                                          */
/*  Author     Stephanie Gerthoux                                           */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE

   #ifndef __DAR_MSG_FT_H_
   #define __DAR_MSG_FT_H_

   #include "../../riviera/rvm/rvm_gen.h"

   /* Functions prototypes */
   T_RV_RET dar_filter_request (T_DAR_FILTER_START *msg_p);
   T_RV_RET dar_empty_mb_and_save_data(  T_DAR_INFO    *buffer_p);
   T_RV_RET dar_write_data_in_buffer( T_DAR_WRITE_START *msg_p);


   #endif /* __DAR_MSG_FT_H_ */

#endif /* #ifdef RVM_DAR_SWE */
