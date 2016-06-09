/****************************************************************************/
/*                                                                          */
/*   File Name:   rtc_env.h                                                 */
/*                                                                          */
/*   Purpose:   This file contains prototypes for RV Environment related    */
/*            functions used to get info, start and stop the rtc block.     */
/*                                                                          */
/*  Version      0.1                                                        */
/*                                                                          */
/*    Date          Modification                                            */
/*  ------------------------------------                                    */
/*  03/20/1   Create                                                        */
/*                                                                          */
/*   Author      Laurent Sollier                                            */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/
#ifndef __RTC_ENV_H_
#define __RTC_ENV_H_


#include "../../riviera/rvm/rvm_gen.h"

#include "rtc_pool_size.h"	/* Stack & Memory Bank sizes definitions */


/* memory bank size and watermark */   
#define RTC_MB_PRIM_SIZE            RTC_MB1_SIZE
#define RTC_MB_PRIM_WATERMARK       (RTC_MB_PRIM_SIZE - 20)

/* generic functions declarations */
T_RVM_RETURN rtc_get_info (T_RVM_INFO_SWE  *infoSWE);

T_RVM_RETURN rtc_set_info(T_RVF_ADDR_ID   addrId,
                          T_RV_RETURN      return_path[],
                          T_RVF_MB_ID      mbId[],
                          T_RVM_RETURN   (*callBackFct) (   T_RVM_NAME SWEntName,
                                                            T_RVM_RETURN errorCause,
                                                            T_RVM_ERROR_TYPE errorType,
                                                            T_RVM_STRING errorMsg) );

T_RVM_RETURN rtc_init (void);

T_RVM_RETURN rtc_stop (void);

T_RVM_RETURN rtc_kill (void);


#endif /*__RTC_ENV_H_*/
