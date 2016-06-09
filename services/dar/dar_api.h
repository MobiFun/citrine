/****************************************************************************/
/*                                                                          */
/*  File Name:  dar_api.h                                                   */
/*                                                                          */
/*  Purpose:  This file contains data structures and functions prototypes   */
/*            used to send events to the DAR SWE.                           */
/*                                                                          */
/*  Version   0.1                                                           */
/*                                                                          */
/*  Date                Modification                                        */
/*  ------------------------------------                                    */
/*  26 September 2001   Create                                              */
/*                                                                          */
/*  Author       Stephanie Gerthoux                                         */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#include "../../riviera/rvm/rvm_api.h"

/* file used in recovery case */
#include "../../include/sys_types.h" 

#include "dar_gen.h"

#ifdef RVM_DAR_SWE

   #ifndef __DAR_API_H_
      #define __DAR_API_H_

      #ifdef __cplusplus
         extern "C"
            {
      #endif

      /***** Events   *****/
      #define DAR_EVENTS_MASK                                  (0x5000)
      #define DAR_EVENT_EXTERN                                 (0x0C00)

      /***** Definitions *****/
      /*Define DAR use parameters */ 
      typedef struct
      { 
         UINT16 group_nb;
         UINT16 mask;
      }T_DAR_USE_ID;

      typedef struct
      {
         T_RV_HDR   os_hdr;
         INT8       status;
      } T_DAR_STATUS;

      /************************************************************************/
      /*                               RECOVERY                               */
      /************************************************************************/

      /***** Events   *****/
      #define DAR_RECOVERY_CONFIG                     (0x0001|DAR_EVENT_EXTERN)

      /***** Definitions ******/
      /* Define return parameters. */
      typedef UINT16 T_DAR_RECOVERY_STATUS;
      /* possible values */
      #define DAR_POWER_ON_OFF            (0x0)       /* Power ON/OFF */
      #define DAR_WATCHDOG                (0xDD11)    /* Watchdog reset */
      #define DAR_NORMAL_SCUTTLING        (0xDD22)    /* Recovery module has
							 decided to active the
							 reset */
      #define DAR_EMERGENCY_SCUTTLING     (0xDD33)    /* Emergency detection */

      /* Define Recovery configuration parameters */
      typedef struct{
         UINT16            msg_id;          /* id of the message     */
         T_DAR_BUFFER      buffer_p;        /* pointer on the buffer */
         UINT8             length;          /* buffer length         */
      } T_DAR_RECOVERY_CONFIG;

      /* Define register parameters */
      #define DAR_NAME_MAX_LEN		(15)
      typedef char T_DAR_NAME[DAR_NAME_MAX_LEN];

      /***** Prototype *****/
      /* Get and reset the status of the DAR entity */
      T_RV_RET dar_recovery_get_status(T_DAR_RECOVERY_STATUS* status);
      T_RV_RET dar_recovery_config(T_RV_RET (*dar_store_recovery_data)(
						T_DAR_BUFFER buffer_p,
						UINT16 length));
      T_RV_RET dar_get_recovery_data(T_DAR_BUFFER buffer_p,UINT16 length );

     /*************************************************************************/
     /*                               WATCHDOG                                */
     /*************************************************************************/

     /***** Prototype *****/
     T_RV_RET  dar_start_watchdog_timer(UINT16 timer);
     T_RV_RET  dar_reload_watchdog_timer(void);
     T_RV_RET  dar_stop_watchdog_timer(void);


     /*************************************************************************/
     /*                                RESET                                  */
     /*************************************************************************/

     /***** Prototype *****/
     T_RV_RET  dar_reset_system(void);


     /*************************************************************************/
     /*                              DIAGNOSE                                 */
     /*************************************************************************/

     /***** Definitions *****/

     /* DAR level messages value ( Error, Warning or debug )  */
     /*   define with 8 bits: - the first for Error level     */
     /*                       - the second for Warning level  */
     /*                       - the other bits for debug level*/

     /* Error level :    1000 0000 in binary*/
     #define DAR_ERROR                               (0x80)
     /* Warning level :  0100 0000 in binary*/
     #define DAR_WARNING                             (0x40)
     /* Debug level :    0000 0001 in binary*/
     #define DAR_DEBUG                               (0x01)
     /* None level :     0000 0000 in binary*/
     #define DAR_NO_DIAGNOSE                         (0x00)
     /* Exception level  1111 1111 in binary*/
     #define DAR_EXCEPTION                           (0xFF)
     /* Causes a reset, if set when calling dar_diagnose_write_emergency() */
     #define DAR_EMERGENCY_RESET               (0x00000001)
     /* New data is appended to last entry, if set when calling
	dar_diagnose_write_emergency() */
     #define DAR_NEW_ENTRY                     (0x00000002)

     /***** Prototype *****/
     /* Diagnose prototypes */
     T_RV_RET dar_diagnose_swe_filter (  T_RVM_USE_ID  dar_use_id, 
                                         T_DAR_LEVEL    dar_level);

     T_RV_RET dar_diagnose_write(  T_DAR_INFO    *buffer_p,
                                   T_DAR_FORMAT  format,
                                   T_DAR_LEVEL   diagnose_info_level,
                                   T_RVM_USE_ID  dar_use_id);

     T_RV_RET dar_diagnose_write_emergency(  T_DAR_INFO    *buffer_p,
                                             T_DAR_FORMAT  format,
                                             T_RVM_USE_ID  dar_use_id,
                                             UINT32 flags);

     T_RV_RET dar_diagnose_generate_emergency(  T_DAR_INFO    *buffer_p,
                                                T_DAR_FORMAT  format,
                                                T_RVM_USE_ID  dar_use_id);
  
     #ifdef __cplusplus
       } 
     #endif

   #endif /* __DAR_API_H_ */
#endif /* #ifdef RVM_DAR_SWE */
