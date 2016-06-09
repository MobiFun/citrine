/*****************************************************************************/
/*                                                                           */
/* File Name:  spi_env.h                                                     */
/*                                                                           */
/* Purpose: This file contains prototypes for RV manager related             */
/*          functions used to get info, start and stop the spi block         */
/*                                                                           */
/* Version     0.1                                                           */
/*                                                                           */
/* Date        Modification                                                  */
/*  ------------------------------------                                     */
/* 20/08/2000  Create                                                        */
/*                                                                           */
/* Author      David Lamy-Charrier (dlamy@tif.ti.com)                        */
/*                                                                           */
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved */
/*****************************************************************************/

#ifndef __SPI_ENV_H_
#define __SPI_ENV_H_

#include "../../riviera/rvm/rvm_gen.h"
#include "abb_inth.h"	 // for MADC_NUMBER_OF_MEAS

#include "spi_pool_size.h"	/* Stack & Memory Bank sizes definitions */

/* SPI mailbox */
#define SPI_MAILBOX                 RVF_TASK_MBOX_0


/* memory bank size and watermark */
#define SPI_MB_PRIM_SIZE            SPI_MB1_SIZE
#define SPI_MB_PRIM_WATERMARK       (SPI_MB_PRIM_SIZE - 56)
#define SPI_MB_PRIM_INC_SIZE        0
#define SPI_MB_PRIM_INC_WATERMARK   0



typedef struct SPI_GBL_INFO
{
   T_RVF_MB_ID    prim_id;
   T_RVF_ADDR_ID  addr_id;
   UINT16         adc_result[MADC_NUMBER_OF_MEAS];
   BOOLEAN        is_gsm_on;
   BOOLEAN        is_adc_on;
   BOOLEAN        SpiTaskReady;
} T_SPI_GBL_INFO;


/* Global variables */
extern T_SPI_GBL_INFO *SPI_GBL_INFO_PTR;


/* generic functions declarations */
T_RVM_RETURN spi_get_info (T_RVM_INFO_SWE  *infoSWE);

T_RVM_RETURN spi_set_info(T_RVF_ADDR_ID addr_id,
                          T_RV_RETURN   ReturnPath[],
                          T_RVF_MB_ID   mbId[],
                          T_RVM_RETURN (*callBackFct)(T_RVM_NAME SWEntName,
                          T_RVM_RETURN errorCause,
                          T_RVM_ERROR_TYPE errorType,
                          T_RVM_STRING errorMsg));

T_RVM_RETURN spi_init (void);

T_RVM_RETURN spi_stop (void);

T_RVM_RETURN spi_kill (void);

#endif /*__SPI_ENV_H_*/
