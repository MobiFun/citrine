/****************************************************************************/
/*                                                                          */
/*   File Name:   dar_const_i.h                                             */
/*                                                                          */
/*   Purpose:   Internal constants used by DAR instance                     */
/*                                                                          */
/*                                                                          */
/*   Version    0.1                                                         */
/*                                                                          */
/*   Date                 Modification                                      */
/*   ------------------------------------                                   */
/*   26 Septembre 2001    Create                                            */
/*                                                                          */
/*   Author          Stephanie Gerthoux                                     */
/*                                                                          */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"

#ifdef RVM_DAR_SWE

   #ifndef __DAR_CONST_I_H_
      #define __DAR_CONST_I_H_

      #ifdef __cplusplus
         extern "C"
            {
      #endif

      /* Define a mask used to identify the events */
      #define DAR_EVENT_EXTERN                            (0x0C00)
      #define DAR_EVENT_INTERN                            (0x0300)

      /* The DAR task run without any time out */
      #define DAR_NOT_TIME_OUT                            (0)

      /* The DAR entity processes only the messages send to the following
	 mail box */
      #define DAR_MBOX                                    (RVF_TASK_MBOX_0)

      /* The DAR entity takes into account only the following events: */
      #define DAR_TASK_MBOX_EVT_MASK                  (RVF_TASK_MBOX_0_EVT_MASK)

      /* The DAR entity waits all event type */
      #define DAR_ALL_EVENT_FLAGS                         (0xFFFF)

      /* The DAR use max group elements used */
      #define DAR_MAX_GROUP_NB                            (4)

      /* RAM max buffer size    */
      #define DAR_MAX_BUFFER_SIZE                         (3000)

      /* Recovery data max buffer size    */
      #define DAR_RECOVERY_DATA_MAX_BUFFER_SIZE           (50)

      /* Dar invalid value    */
      #define DAR_INVALID_VALUE                           (0xFFFF)
      
      /* Dar initialization */
      #define DAR_INITIALIZATION_VALUE                    (0x0000)

      /* Define the Watchdog timer register mode */
      #define WATCHDOG_TIM_MODE                           (0xFFFFF804)


      #if (CHIPSET == 7 || CHIPSET == 8 || CHIPSET == 10 || CHIPSET == 11)
          /* Define the Debug Unit register mode */
	  #define DAR_DEBUG_UNIT_REGISTER                     (0x03C00000)
      #elif (CHIPSET == 12)
	  /* Define the Debug Unit register mode */
	  #define DAR_DEBUG_UNIT_REGISTER                     (0x09F00000)
      #endif  

      /* Mask to enable the Debug Unit Module */
      #define ENABLE_DU_MASK                              (0xF7FF)

      /* Mask to disable the Debug Unit Module */
      #define DISABLE_DU_MASK                             (0x0800)

      /* Extra Control register CONF Adress */
      #define DAR_DU_EXTRA_CONTROL_REG                    (0xFFFFFB10)
     
      /* Define the size of the Debug Unit register      */
      /* This size is 64 words of 32 bits = 64*4 bytes */ 
      /* Size in bytes */
      #define DEBUG_UNIT_BYTES_SIZE                       (256)
      /* Define the size in words */
      #define DEBUG_UNIT_WORD_SIZE                        (64) 

      /* Define the size of the X_dump _buffer */
      /* This size is specified in the gsm_cs_amd4_lj3_test.cmd
	 (in "system" directory) */
      /* Its size is 38*32 bits = 38*4 bytes = 152 bytes */
      #define DAR_X_DUMP_BUFFER_SIZE                      (152)

      /* Define the exceptions */
      #define DAR_NO_ABORT_EXCEPTION                      (0)
      #define DAR_EXCEPTION_DATA_ABORT                    (1)
      #define DAR_EXCEPTION_PREFETCH_ABORT                (2)
      #define DAR_EXCEPTION_UNDEFINED                     (3)
      #define DAR_EXCEPTION_SWI                           (4)
      #define DAR_EXCEPTION_RESERVED                      (5)

      #ifdef __cplusplus
          }
      #endif
   #endif /* __DAR_CONST_I_H_ */
#endif /* #ifdef RVM_DAR_SWE */
