/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_emergency.h                                             */
/*                                                                          */
/*  Purpose:   This function contains the functions prototypes of the DAR   */ 
/*             entity when emergencies occured.                             */
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

#ifndef __DAR_EMERGENCY_H_
#define __DAR_EMERGENCY_H_

  #include "../../riviera/rv/rv_defined_swe.h"

  #ifdef RVM_DAR_SWE

    #include "../../riviera/rvm/rvm_gen.h"

    /* Functions prototypes */
    T_RV_RET dar_process_emergency( T_DAR_INFO    *buffer_p,
                                    T_DAR_FORMAT  format,
                                    T_RVM_USE_ID  dar_use_id,
                                    UINT32 flags);
    void dar_exception_arm_undefined(void);
    void dar_exception_arm_swi(void);
    void dar_exception_arm_abort_prefetch(void);
    void dar_exception_arm_abort_data(void);
    void dar_exception_arm_reserved(void);

  #endif /* #ifdef RVM_DAR_SWE */

  void dar_exception(int abort_type);

#endif /* __DAR_EMERGENCY_H_ */
