
/****************************************************************************/
/*                                                                          */
/*  Name        rvm_priorities.h                                            */
/*                                                                          */
/*  Function    this file priorities defintitions for basic SWEs			*/
/*				It includes a file rvm_ext_priorities.h where are defined   */
/*				"custom" priorities.
/*                                                                          */
/*  Version		0.1															*/
/*																			*/
/* 	Date       	Modification												*/
/*  ------------------------------------									*/
/*  10/11/2000	Create														*/
/*																			*/
/*	Author		Cristian Livadiotti (c-livadiotti@ti.com)					*/
/*																			*/
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#ifndef __RVM_PRIORITIES_H_
#define __RVM_PRIORITIES_H_


#include "rvm_ext_priorities.h"

/* PRIORITIES SETTING:                        */
/* All user priority should be set under 250: */
/* Higher values are reserved                 */

/*
** Bluetooth Priorities:
** All set to same value, except ATP (not a task) and HCI (valid on PC only)
*/
#define		RVM_HCI_TASK_PRIORITY			(250)
#define		RVM_L2CAP_TASK_PRIORITY			(240)
#define		RVM_BTCTRL_TASK_PRIORITY		(240)
#define		RVM_RFCOMM_TASK_PRIORITY		(240)
#define		RVM_SPP_TASK_PRIORITY			(240)
#define		RVM_SDP_TASK_PRIORITY			(240)
#define		RVM_HSG_TASK_PRIORITY			(240)
#define		RVM_DUN_GW_TASK_PRIORITY		(240)
#define		RVM_FAX_GW_TASK_PRIORITY		(240)
#define		RVM_ATP_UART_TASK_PRIORITY		(240)
#define		RVM_ATP_TASK_PRIORITY			(255)


/*
** Drivers and Services Priorities
** Note: FFS priority should be lower than every user
*/
#define		RVM_SPI_TASK_PRIORITY			(39)
#define		RVM_TTY_TASK_PRIORITY			(238)
#define		RVM_AUDIO_TASK_PRIORITY			(245)
#define		RVM_AUDIO_BGD_TASK_PRIORITY		(246)
#define		RVM_RTC_TASK_PRIORITY			(248)
#define     RVM_FFS_TASK_PRIORITY			(250)
#define     RVM_TRACE_TASK_PRIORITY			(251) //(79)
#define     RVM_DAR_TASK_PRIORITY			(245)
#define		RVM_ETM_TASK_PRIORITY			(249)
#define		RVM_LCC_TASK_PRIORITY			(246)
#define		RVM_MKS_TASK_PRIORITY			(255)
#define		RVM_KPD_TASK_PRIORITY			(10)

#define		RVM_MPM_TASK_PRIORITY			(242)

#define		RVM_RNET_WS_TASK_PRIORITY		(240)
#define		RVM_RNET_RT_TASK_PRIORITY		(240)
#define		RVM_DCM_TASK_PRIORITY			(240)
#define		RVM_MFW_TASK_PRIORITY			(241)
#define		RVM_MDL_TASK_PRIORITY           	(241)


#define		RVM_OBIGO_TASK_PRIORITY			(242)
#define		RVM_IMG_TASK_PRIORITY			(241)
#define		RVM_MMS_TASK_PRIORITY	(247)

/*
** RV Test Menu and Dummy Tasks Priorities
*/
#define		RVM_RVTEST_MENU_TASK_PRIORITY	(240)
#define		RVM_DUMMY_TASK_PRIORITY			(80)
//#define		IDLE_TASK_PRIORITY				(80)  /* A-M-E-N-D-E-D! */
#define		RVM_INVKR_TASK_PRIORITY			(240) /* A-M-E-N-D-E-D! */
#define		RVM_TE1_TASK_PRIORITY			(240) /* A-M-E-N-D-E-D! */
#define		RVM_TE2_TASK_PRIORITY			(240) /* A-M-E-N-D-E-D! */
#define		RVM_TE3_TASK_PRIORITY			(240) /* A-M-E-N-D-E-D! */
#define		RVM_TE4_TASK_PRIORITY			(240) /* A-M-E-N-D-E-D! */
#define		RVM_TE5_TASK_PRIORITY			(240) /* A-M-E-N-D-E-D! */
#define		RVM_TE6_TASK_PRIORITY			(240) /* A-M-E-N-D-E-D! */

#define		RVM_TMS_TASK_PRIORITY			(200) /* A-M-E-N-D-E-D! */
#endif /* __RVM_PRIORITIES_H_ */
