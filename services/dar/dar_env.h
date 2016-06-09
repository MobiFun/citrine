/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_env.h                                                   */
/*                                                                          */
/*  Purpose:  This file contains prototypes for Riviera Environment related */
/*            functions used to get info, start and stop the diagnose block.*/
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date               Modification                                         */
/*  ------------------------------------                                    */
/*  26 september 2001  Create                                               */
/*                                                                          */
/*  Author    Stephanie Gerthoux                                            */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE
   #include "dar_structs_i.h"

   #include "dar_pool_size.h"	/* Stack & Memory Bank sizes definitions */


   #ifndef __DAR_ENV_H_
      #define __DAR_ENV_H_

      /* memory bank size and watermark */
      #define DAR_MB_SIZE            DAR_MB1_SIZE
      #define DAR_MB_WATERMARK       DAR_MB_SIZE

      /* definition of RVT state*/
      typedef INT8 T_DAR_STATE;

      /* possible values */
      #define DAR_STARTED            (0)
      #define DAR_NOT_STARTED        (-1)

      /* generic functions declarations */
      T_RVM_RETURN dar_get_info (T_RVM_INFO_SWE   *infoSWEnt);

      T_RVM_RETURN dar_set_info(T_RVF_ADDR_ID addrId,
                                T_RV_RETURN    return_path[],
                                T_RVF_MB_ID    mbId[],
                                T_RVM_RETURN  (*callBackFctError) ( T_RVM_NAME SWEntName,
                                                                    T_RVM_RETURN errorCause,
                                                                    T_RVM_ERROR_TYPE errorType,
                                                                    T_RVM_STRING errorMsg) );

      T_RVM_RETURN dar_init (void);

      T_RVM_RETURN dar_stop (void);

      T_RVM_RETURN dar_kill (void);

   #endif /*__DAR_ENV_H_*/

#endif /* #ifdef RVM_DAR_SWE */
