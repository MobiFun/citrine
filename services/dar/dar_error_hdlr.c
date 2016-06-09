/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_error_hdlr.c                                            */
/*                                                                          */
/*  Purpose:  This file contains routines used to report unrecoverable      */
/*                 memory errors that might occur.                          */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date               Modification                                         */
/*  ------------------------------------------------------------------------*/
/*  27 September 2001  Create                                               */
/*                                                                          */
/*  Author:  Stephanie Gerthoux                                             */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE

   #include "../../riviera/rv/rv_general.h"
   #include "../../riviera/rvf/rvf_api.h"
   #include "dar_error_hdlr_i.h"
   #include "dar_macro_i.h"

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_error_trace                                     */
   /*                                                                         */
   /*    Purpose:         This function is used to report error occured during*/
   /*                     the diagnose entity execution                       */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        status       - Contains the error code to be reported.           */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*        None.                                                            */
   /*                                                                         */
   /*    Global Parameters:                                                   */
   /*        None.                                                            */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None.                                                            */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*       27 September 01           Create                                  */
   /*                                                                         */
   /***************************************************************************/
   void dar_error_trace(UINT8 error_id)
   {
      switch(error_id)
      {
         case DAR_ENTITY_NOT_START:
         {
            DAR_SEND_TRACE("DAR entity not started.",RV_TRACE_LEVEL_ERROR);
            break;
         }

         case DAR_ENTITY_NO_MEMORY:
         {
            DAR_SEND_TRACE("DAR entity has not enough memory",
				RV_TRACE_LEVEL_ERROR);
            break;
         }

         case DAR_ENTITY_BAD_PARAMETER:
         {
            DAR_SEND_TRACE("DAR entity has bad parameters",
				RV_TRACE_LEVEL_ERROR);
            break;
         }

         case DAR_ERROR_STOP_EVENT:
         {
            DAR_SEND_TRACE("DAR entity has received a stop error event",
				RV_TRACE_LEVEL_ERROR);
            break;
         }

         case DAR_ERROR_START_EVENT:
         {
             DAR_SEND_TRACE("DAR entity has received a start error event",
				RV_TRACE_LEVEL_ERROR);
             break;
         }

         case DAR_ENTITY_BAD_MESSAGE:
         {
            DAR_SEND_TRACE("DAR entity has received a bad message",
				RV_TRACE_LEVEL_ERROR);
            break;
         }  
      }
   }
   /*********************** End of dar_error_trace function *******************/

   /***************************************************************************/
   /*                                                                         */
   /*    Function Name:   dar_ffs_error_trace                                 */
   /*                                                                         */
   /*    Purpose:         This function is used to report error occured during*/
   /*                     the dar entity execution                            */
   /*                                                                         */
   /*    Input Parameters:                                                    */
   /*        status       - Contains the error code to be reported.           */
   /*                                                                         */
   /*    Output Parameters:                                                   */
   /*        None.                                                            */
   /*                                                                         */
   /*    Global Parameters:                                                   */
   /*        None.                                                            */
   /*                                                                         */
   /*    Note:                                                                */
   /*        None.                                                            */
   /*                                                                         */
   /*    Revision History:                                                    */
   /*       29 october 01           Create                                    */
   /*                                                                         */
   /***************************************************************************/
   void dar_ffs_error_trace(UINT8 error_id)
   {
      switch(error_id)
      {
         case DAR_ENTITY_NO_MEMORY:
         {
            DAR_SEND_TRACE("DAR FFS entity has not enough memory",
				RV_TRACE_LEVEL_ERROR);
            break;
         }
         case DAR_ENTITY_FILE_ERROR:
         {
            DAR_SEND_TRACE("DAR FFS entity has received a wrong file name or the flash is not formatted",RV_TRACE_LEVEL_ERROR);
            break;
         }

         case DAR_ENTITY_FILE_NO_SAVED:
         {
            DAR_SEND_TRACE("DAR FFS entity has not saved the file",
				RV_TRACE_LEVEL_ERROR);
            break;
         }

         case DAR_ENTITY_FILE_NO_CLOSE:
         {
            DAR_SEND_TRACE("DAR FFS entity has not closed the file",
				RV_TRACE_LEVEL_ERROR);
            break;
         }
      } /* switch(error_id) */
   } /* dar_ffs_error_trace */
   /******************** End of dar_ffs_error_trace function *****************/

#endif /* #ifdef RVM_DAR_SWE */
