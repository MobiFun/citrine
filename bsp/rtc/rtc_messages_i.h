/****************************************************************************/
/*                                                                          */
/*   File Name:   rtc_messages_i.h                                          */
/*                                                                          */
/*   Purpose:   This file contains data structures and functions prototypes */
/*            used to send events to the RTC SWE.                           */
/*                                                                          */
/*  Version      0.1                                                        */
/*                                                                          */
/*    Date          Modification                                            */
/*  ------------------------------------                                    */
/*  03/20/2001   Create                                                     */
/*                                                                          */
/*   Author      Laurent Sollier                                            */
/*                                                                          */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/
#ifndef __RTC_MESSAGES_H_
#define __RTC_MESSAGES_H_


#include "../../riviera/rv/rv_general.h"
#include "../../riviera/rvf/rvf_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* the message offset must differ for each SWE in order to have unique msg_id in the system */
#define RTC_MESSAGES_OFFSET      (0x35 << 10)
#define RTC_MAIL_BOX 1




/*****************************************/
/* structures of messages send to DRV   */


/******* RTC ALARM **********/

#define RTC_ALARM_EVT               1

typedef struct
{
   T_RV_HDR os_hdr;
   
} T_RTC_ALARM;




#ifdef __cplusplus
}
#endif


#endif /* __RTC_MESSAGES_H_ */
