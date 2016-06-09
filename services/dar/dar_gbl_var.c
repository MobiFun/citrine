/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_gbl_var.c                                               */
/*                                                                          */
/*  Purpose:    This function contains the global variables that are        */
/*              not initialized in the .bss                                 */
/*              ----------------------------                                */
/*              These variables are stored in the .bss_dar section          */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date               Modification                                         */
/*  ------------------------------------                                    */
/*  29 October 2001    Create                                               */
/*                                                                          */
/*  Author     Stephanie Gerthoux                                           */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#define	BSS_DAR	__attribute__ ((section ("ext.ram")))

#include "../../riviera/rv/general.h"
#include "../../riviera/rv/rv_general.h"
#include "../../riviera/rv/rv_defined_swe.h"

#ifdef RVM_DAR_SWE

   #include "dar_api.h"
   #include "dar_const_i.h"

   /**** Global variables ****/
   /* Buffer used to save some parameters before a reset */
   UINT8    dar_recovery_buffer[DAR_RECOVERY_DATA_MAX_BUFFER_SIZE] BSS_DAR;
  
   /* dar_current_status : to get the status of the system*/
   T_DAR_RECOVERY_STATUS dar_current_status BSS_DAR;

   /* dar_exception_status : to get the status of the exception */
   UINT8    dar_exception_status BSS_DAR;

   /* Write buffer*/
   char    dar_write_buffer[DAR_MAX_BUFFER_SIZE] BSS_DAR;

   /* Ram buffer that contains the Debug Unit register */
   UINT32  debug_RAM[DEBUG_UNIT_WORD_SIZE] BSS_DAR;

#endif /* #ifdef RVM_DAR_SWE */

/* used to be in the linker script in TI's original */
UINT32 xdump_buffer[38] BSS_DAR;
