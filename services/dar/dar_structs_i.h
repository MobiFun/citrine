/******************************************************************************/
/*                                                                            */
/*    File Name:   dar_structs_i.h                                            */
/*                                                                            */
/*    Purpose:     This file contains constants, data type, and data          */
/*                 structures that are used by the diagnose's task.           */
/*                                                                            */
/*    Note:                                                                   */
/*        None.                                                               */
/*                                                                            */
/*    Revision History:                                                       */
/*      26 september 01     Stephanie Gerthoux        Create                  */
/*                                                                            */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved. */
/*                                                                            */
/******************************************************************************/
#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE
   #ifndef  _DAR_STRUCTS_I_
      #define  _DAR_STRUCTS_I_

      #include "../../riviera/rvm/rvm_gen.h"
      #include "dar_gen.h"
      #include "dar_const_i.h"

      #ifdef __cplusplus
         extern "C"
            {
      #endif

      /* *****************    DAR internal structures *************************/

      /* Diagnose Use parameter with masks for warning and debug messages */
      typedef struct
      { 
         UINT16 group_nb;
         UINT16 mask_warning;
         UINT16 mask_debug;
      }T_DAR_FILTER_PARAMETER;

      /* Diagnose Use Messages parameter (used to send messages in DAR
	 mailbox) */
      typedef struct
      { 
         UINT16 group_nb;
         UINT16 mask;
         UINT8   level;
      }T_DAR_MSG_PARAM;

      /* Diagnose write data messages paremeters*/
      typedef struct
      {
         T_DAR_INFO      *char_p;
         T_DAR_FORMAT   data_format;
         T_DAR_LEVEL    level;
         T_DAR_USE_ID   use_id;
      }T_DAR_WRITE;


      /****************************** DAR'S ENVIRONMENT **********************/
      /* Define a structure used to store all information related to the     */
      /* DAR's task & memory bank identifiers.                               */

      typedef struct
      {
         /* DAR addr ID.                                                    */
         T_RVF_ADDR_ID          addrId;
         /* DAR FFS addr ID                                                 */
         T_RVF_ADDR_ID          ffs_addrId;

         /* DAR memory bank.                                                */
         T_RVF_MB_ID            mb_dar; 
         /* DAR state                                                       */
         UINT8                  state; 
         /* data format: ASCII or binary                                    */
         T_DAR_FORMAT           format;
         /* data level : Error/Warning/ Debug                               */
         T_DAR_LEVEL            diagnose_level;  
         /* DAR use id                                                      */
         T_DAR_USE_ID           dar_use_id;  
         /* return path of the function that previously start the diagnose  */
         T_RV_RETURN            return_path;  
         /* Pointer to the error function                                  */
         T_RVM_RETURN           (*callBackFctError)  (T_RVM_NAME        SWEntName,
                                                      T_RVM_RETURN      errorCause,
                                                      T_RVM_ERROR_TYPE  errorType,
                                                      T_RVM_STRING      errorMsg);
         /* Pointer to the DAR callback function */                                     
         T_RV_RET               (*entity_dar_callback)(T_DAR_BUFFER      buffer_p,
                                                       UINT16            length);
         /* Filter Array that contains the group and the masks              */
         /* (Warning, debug) in order to filter messages                    */
         T_DAR_FILTER_PARAMETER  dar_filter_array[DAR_MAX_GROUP_NB];
         /* index to indicate the group position in the dar_filter_array    */
         UINT16                  index;
         /* free index to indicate the first free group position in the     */
         /* dar_filter_array ( to add a new group in the array)             */
         UINT16                  free_index;
         /* Buffer to store diagnose data*/

      } T_DAR_ENV_CTRL_BLK;

      #ifdef __cplusplus
         }
      #endif

   #endif

#endif /* #ifdef RVM_DAR_SWE */
