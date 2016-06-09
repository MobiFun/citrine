/******************************************************************************/
/*                                                                            */
/*    File Name:   rtc_config.h                                               */
/*                                                                            */
/*    Purpose:   This file contains adresses for RTC register access.         */
/*               and defined value                                            */
/*                                                                            */
/*    Note:      None.                                                        */
/*                                                                            */
/*    Revision History:                                                       */
/*    05/31/01   Laurent Sollier      Create.                                 */
/*                                                                            */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved  */
/*                                                                            */
/******************************************************************************/

#ifndef _RTC_CONFIG_H_
#define _RTC_CONFIG_H_

#include "../../include/config.h"

#include "../mem.h" 
#include "../../riviera/rv/general.h"


/* FOR ULYSSE AND CALYPSO CHIP */
				/* Seconds register */
#define RTC_SECONDS_REG		(UINT8 *)(RTC_XIO_START)
				/* Minutes register */
#define RTC_MINUTES_REG		((UINT8 *)(RTC_XIO_START) + 0x01)
				/* Hours register */
#define RTC_HOURS_REG		((UINT8 *)(RTC_XIO_START) + 0x02)
				/* Days register */
#define RTC_DAYS_REG		((UINT8 *)(RTC_XIO_START) + 0x03)
				/* Months register */
#define RTC_MONTHS_REG		((UINT8 *)(RTC_XIO_START) + 0x04)
				/* Years register */
#define RTC_YEARS_REG		((UINT8 *)(RTC_XIO_START) + 0x05)
				/* Week register */
#define RTC_WEEK_REG		((UINT8 *)(RTC_XIO_START) + 0x06)
				/* Alarms seconds register */
#define RTC_ALARM_SECONDS_REG	((UINT8 *)(RTC_XIO_START) + 0x08)
				/* Alarms minutes register */
#define RTC_ALARM_MINUTES_REG	((UINT8 *)(RTC_XIO_START) + 0x09)
				/* Alarms hours register */
#define RTC_ALARM_HOURS_REG	((UINT8 *)(RTC_XIO_START) + 0x0A)
				/* Alarms days register */
#define RTC_ALARM_DAYS_REG	((UINT8 *)(RTC_XIO_START) + 0x0B)
				/* Alarms months register */
#define RTC_ALARM_MONTHS_REG	((UINT8 *)(RTC_XIO_START) + 0x0C)
				/* Alarms years register */
#define RTC_ALARM_YEARS_REG	((UINT8 *)(RTC_XIO_START) + 0x0D)
				/* Control register */
#define RTC_CTRL_REG		((UINT8 *)(RTC_XIO_START) + 0x10)
				/* Status register */
#define RTC_STATUS_REG		((UINT8 *)(RTC_XIO_START) + 0x11)
				/* Interrupts register */
#define RTC_INTERRUPTS_REG	((UINT8 *)(RTC_XIO_START) + 0x12)
				/* LSB compensation register */
#define RTC_COMP_LSB_REG	((UINT8 *)(RTC_XIO_START) + 0x13)
				/* MSB compensation register */
#define RTC_COMP_MSB_REG	((UINT8 *)(RTC_XIO_START) + 0x14)

/* RTC Control register description */

#define RTC_START_RTC		0x0001	/* 1 => RTC is running */
#define RTC_ROUND_30S		0x0002	/* Time rounded to the closest minute */
#define RTC_AUTO_COMP		0x0004	/* Auto compensation enabled or not */
#define RTC_MODE_12_24		0x0008	/* 12 hours mode*/
#define RTC_TEST_MODE		0x0010	/* Test mode */
#define RTC_SET_32_COUNTER	0x0020 	/* set 32 KHz counter with comp_reg */
#if ((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
    #define RTC_nDELTA_OMEGA	0x0040  /* Analog Baseband Type */
#endif


/* RTC Interrupt register description */

#define RTC_EVERY	0x0003
	/* Define period of periodic interrupt (second, minute, hour, day) */
#define RTC_IT_TIMER	0x0004		/* Enable periodic interrupt */
#define RTC_IT_ALARM	0x0008		/* Alarm interrupt enabled or not */	

/* RTC Status register description */

#define RTC_BUSY	0x0001
#define RTC_RUN		0x0002		/* RTC is running */
#define RTC_1S_EVENT	0x0004		/* One second has occured */
#define RTC_1M_EVENT	0x0008		/* One minute has occured */
#define RTC_1H_EVENT	0x0010		/* One hour has occured */
#define RTC_1D_EVENT	0x0020		/* One day has occrued */
#define RTC_ALARM	0x0040		/* Alarm interrupt has been generated */
#define RTC_POWER_UP	0x0080		/* Indicates that a reset occured */

#define RTC_EVERY_SEC	0x0000
#define RTC_EVERY_MIN  	0x0001
#define RTC_EVERY_HR	0x0002
#define RTC_EVERY_DAY	0x0003

/* 32 Khz and HF clock definition */
#define RTC_CLOCK_32K 32768.0

	/* HF clock definition */
#if ((CHIPSET == 3) || (CHIPSET == 5) || (CHIPSET == 6))
	#define RTC_CLOCK_HF 65000000.0
#elif ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8))
	#define RTC_CLOCK_HF 78000000.0
#elif ((CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
	#define RTC_CLOCK_HF 104000000.0
#endif



#endif /* #ifndef _RTC_CONFIG_H_ */
