/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_diagnose_i.h                                            */
/*                                                                          */
/*  Purpose:   This function contains the functions prototypes of the DAR   */ 
/*             entity diagnose functions.                                   */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date               Modification                                         */
/*  ------------------------------------                                    */
/*  18 October 2001    Create                                               */
/*                                                                          */
/*  Author     Stephanie Gerthoux                                           */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE

   #ifndef __DAR_DIAGNOSE_I_H_
   #define __DAR_DIAGNOSE_I_H_

   #include "../../riviera/rvm/rvm_gen.h"

   /* Search a group in the dar array*/
   T_RV_RET dar_search_group(UINT16 group, UINT8 *index_p);

   /* Add a group in the dar array*/
   T_RV_RET dar_add_group(UINT8 *index_p);

   /* Send write data prototype */
   T_RV_RET dar_send_write_data (  T_DAR_INFO    *buffer_p,
                                   T_DAR_FORMAT  format,
                                   T_DAR_LEVEL   diagnose_info_level,
                                   T_RVM_USE_ID  dar_use_id);

   /* Reset the system */
   T_RV_RET dar_reset(void);


   #endif /* _DAR_DIAGNOSE_I_H_ */

#endif /* #ifdef RVM_DAR_SWE */
