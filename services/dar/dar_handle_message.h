/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_handle_message.h                                        */
/*                                                                          */
/*  Purpose:   This function contains the functions prototypes of the DAR   */ 
/*             entity handle messages.                                      */
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

   #ifndef __DAR_HANDLE_MESSAGE_H_
   #define __DAR_HANDLE_MESSAGE_H_

      #include "../../riviera/rvm/rvm_gen.h"

      /* Handle message prototype */
      T_RV_RET dar_handle_msg(T_RV_HDR	*msg_p);

   #endif /* __DAR_HANDLE_MESSAGE_H_ */

#endif /* #ifdef RVM_DAR_SWE */
