/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_messages_i.h                                            */
/*                                                                          */
/*  Purpose:  Internal messages used by DAR instance                        */
/*                                                                          */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date              Modification                                          */
/*  ------------------------------------                                    */
/*  17 october 2001   Create                                                */
/*                                                                          */
/*  Author            Stephanie Gerthoux                                    */
/*                                                                          */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#ifdef RVM_DAR_SWE
   #ifndef __DAR_MESSAGES_I_H_
      #define __DAR_MESSAGES_I_H_

      #include "dar_structs_i.h"

      #ifdef __cplusplus
         extern "C"
         {
      #endif

      /************************ Diagnose messages *****************************/
      /* Diagnose filter and no filter messages */
      #define DAR_FILTER_REQ      (0x0001 | DAR_EVENT_INTERN | DAR_EVENTS_MASK)

      /* Diagnose filter structure */
      typedef struct          
      {
         T_RV_HDR              os_hdr;
         T_DAR_MSG_PARAM       use_msg_parameter;
         T_RV_RETURN           return_path;
      } T_DAR_FILTER_START;

      /* Diagnose write messages */
      #define DAR_WRITE_REQ       (0x0003 | DAR_EVENT_INTERN | DAR_EVENTS_MASK)

      /* Diagnose write data structure */
      typedef struct          
      {
         T_RV_HDR           os_hdr;
         T_DAR_WRITE        data_write;
         T_RV_RETURN        return_path;
      } T_DAR_WRITE_START;
  
      /************************** FFS interface *******************************/
      /* FFS RAM to FLASH interface */
      /* start message */
      #define DAR_FFS_RAM_2_FLASH_START_REQ	\
				  (0x0004 | DAR_EVENT_INTERN | DAR_EVENTS_MASK)
      typedef struct
      {
         T_RV_HDR  os_hdr;
         UINT16    initial_size;
      } T_DAR_FFS_RAM_2_FLASH_START;

      /* start confirmation message */
      #define DAR_FFS_INIT_DONE   (0x0005 | DAR_EVENT_INTERN | DAR_EVENTS_MASK)

      typedef struct
      {
         T_RV_HDR  os_hdr;
      } T_DAR_FFS_INIT;

      /* stop message */
      #define DAR_FFS_STOP_REQ    (0x0006 | DAR_EVENT_INTERN | DAR_EVENTS_MASK)
      typedef struct
      {
         T_RV_HDR  os_hdr;
      } T_DAR_FFS_STOP;

      /* stop confirmation message */
      #define DAR_FFS_STOP_CON    (0x0007 | DAR_EVENT_INTERN | DAR_EVENTS_MASK)
  
    #ifdef __cplusplus  
    }
    #endif

  #endif /* __DAR_MESSAGES_I_ */

#endif /* #ifdef RVM_DAR_SWE */
