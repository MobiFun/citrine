/******************************************************************************/
/*                                                                            */
/*    File Name:   rtc_i.h                                                    */
/*                                                                            */
/*    Purpose:   This file contains the structures, constants and prototypes  */
/*               dedicated to RTC.                                            */
/*                                                                            */
/*    Note:      None.                                                        */
/*                                                                            */
/*    Revision History:                                                       */
/*    03/20/01   Laurent Sollier      Create.                                 */
/*                                                                            */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved  */
/*                                                                            */
/******************************************************************************/

#ifndef _RTC_I_H_
#define _RTC_I_H_

#include "../../riviera/rvm/rvm_gen.h"
#include "../../riviera/rvf/rvf_api.h"

/******************************************************************************/
/*                                                                            */
/* Define a structure used to store all the informations related to RTC's task*/
/* & MBs identifiers.                                                         */
/*                                                                            */
/******************************************************************************/

typedef struct
{
   T_RVF_MB_ID      prim_id;
   T_RVF_ADDR_ID    addr_id;
   void*            msg_alarm_event;      
} T_RTC_ENV_CTRL_BLK;

/*******************************************************************************
 *
 *                               RTC_Initialize
 * 
 * Purpose  : Initializes the RTC driver.
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns: RVF_OK if initialisation is ok
 *            RVF_INTERNAL_ERROR else
 *
 ******************************************************************************/

T_RVF_RET RTC_Initialize(void);


/*******************************************************************************
 *
 *                               RTC_ProcessAlarmEvent
 * 
 * Purpose  : Call MMI when SW RTC module receive alarm event
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns: none
 *
 *
 ******************************************************************************/

void RTC_ProcessAlarmEvent(void);

/*******************************************************************************
 *
 *                               RTC_ItTimerHandle
 * 
 * Purpose  : Compute an average value for compensation register
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns: none
 *
 *
 ******************************************************************************/

void RTC_ItTimerHandle(void);


/*******************************************************************************
 *
 *                               RTC_ItAlarmHandle
 * 
 * Purpose  : Activate HISR which will send a message in the RTC mailbox to
 *            inform that time alarm is reached
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns: none
 *
 *
 ******************************************************************************/

void RTC_ItAlarmHandle(void);


/*******************************************************************************
 *
 *                               RTC_GaugingHandler
 * 
 * Purpose  : This function is called when a gauging is finished (started by 
 *            layer1). It save clock counter for average value computation
 *            ( done by RTC_ItTimerHandle)
 *
 * Arguments: In : none
 *            Out: none
 *
 * Returns: none
 *
 *
 ******************************************************************************/

void RTC_GaugingHandler(void);


#endif /* #ifndef _RTC_I_H_ */
