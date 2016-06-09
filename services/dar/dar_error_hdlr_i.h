/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_error_hdlr_i.h                                          */
/*                                                                          */
/*  Purpose:  This file contains routines used to report unrecoverable      */
/*            memory errors that might occur.                               */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date               Modification                                         */
/*  ----------------------------------------------------------------------  */
/*  27 September 2001  Create                                               */
/*                                                                          */
/*  Author  Stephanie Gerthoux                                              */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE
   #ifndef   _DAR_ERROR_HDLR_I_H
      #define   _DAR_ERROR_HDLR_I_H
         
      /* Id of the error trace */
      #define DAR_ENTITY_NOT_START         (0)
      #define DAR_ENTITY_NO_MEMORY         (1)
      #define DAR_ERROR_STOP_EVENT         (2)
      #define DAR_ERROR_START_EVENT        (3)
      #define DAR_ENTITY_BAD_PARAMETER     (4)
      #define DAR_ENTITY_BAD_MESSAGE       (5)
      #define DAR_ENTITY_FILE_ERROR        (7)
      #define DAR_ENTITY_FILE_NO_SAVED     (8)
      #define DAR_ENTITY_FILE_NO_CLOSE     (9)

      void dar_error_trace(UINT8 error_id);
      void dar_ffs_error_trace(UINT8 error_id);

   #endif
#endif /* #ifdef RVM_DAR_SWE */
