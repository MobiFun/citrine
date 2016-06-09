/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_watchdog.h                                              */
/*                                                                          */
/*  Purpose:   This function contains the functions prototypes of the       */ 
/*             watchdog reset functions.                                    */
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

   #ifndef __DAR_WATCHDOG_H_
   #define __DAR_WATCHDOG_H_

      #include "../../riviera/rvm/rvm_gen.h"

      /* Reset the system when the general purpose timer expires*/
      void dar_watchdog_reset(void);

   #endif /* _DAR_WATCHDOG_H_ */

#endif /* #ifdef RVM_DAR_SWE */
