/*****************************************************************************/
/*                                                                           */
/*  Name        spi_task.h						     */
/*                                                                           */
/*  Function    this file contains timers definitions used by spi_core,      */
/*		in case the PWR SWE is defined.				     */
/*                                                                           */
/*  Version	0.1							     */
/*  Author	Candice Bazanegue					     */
/*									     */
/*  Date       	Modification						     */
/*  ------------------------------------				     */
/*  20/08/2000	Create							     */
/*  01/09/2003	Modfication						     */
/*  Author	Pascal Puel						     */
/*									     */
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved */
/*****************************************************************************/

#ifndef _SPI_TASK_H_
#define _SPI_TASK_H_

#include "../../riviera/rv/rv_defined_swe.h"	   // for RVM_PWR_SWE

#ifdef RVM_PWR_SWE

#include "pwr/pwr_cust.h"

#define SPI_TIMER0                      (RVF_TIMER_0)
#define SPI_TIMER0_INTERVAL_1           (PWR_BAT_TEST_TIME_1)
#define SPI_TIMER0_INTERVAL_2           (PWR_BAT_TEST_TIME_2)
#define SPI_TIMER0_INTERVAL_3           (PWR_CALIBRATION_TIME_1)
#define SPI_TIMER0_INTERVAL_4           (PWR_CALIBRATION_TIME_2)
#define SPI_TIMER0_WAIT_EVENT           (RVF_TIMER_0_EVT_MASK)

#define SPI_TIMER1                      (RVF_TIMER_1)
#define SPI_TIMER1_INTERVAL             (PWR_CI_CHECKING_TIME)
#define SPI_TIMER1_WAIT_EVENT           (RVF_TIMER_1_EVT_MASK)

#define SPI_TIMER2                      (RVF_TIMER_2)
#define SPI_TIMER2_INTERVAL             (PWR_CV_CHECKING_TIME)
#define SPI_TIMER2_WAIT_EVENT           (RVF_TIMER_2_EVT_MASK)

#define SPI_TIMER3                      (RVF_TIMER_3)
#define SPI_TIMER3_INTERVAL             (PWR_DISCHARGE_CHECKING_TIME_1)
#define SPI_TIMER3_INTERVAL_BIS         (PWR_DISCHARGE_CHECKING_TIME_2)
#define SPI_TIMER3_WAIT_EVENT           (RVF_TIMER_3_EVT_MASK)

#endif  // RVM_PWR_SWE


// Prototypes
void spi_adc_on (void);
T_RV_RET spi_core(void);

#endif	// _SPI_TASK_H_
