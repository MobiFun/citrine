/********************************************************************************
 * Enhanced TestMode (ETM) - Revision Controle System Header
 * @file	etm_version.h 
 *
 * @author	Kim T. Peteren (ktp@ti.com)
 * @version 0.1
 *

 *
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *  21/10/2003	Creation
 * 
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/

#ifndef _ETM_VERSION_H_
#define _ETM_VERSION_H_


/******************************************************************************
 * Enhanced TestMode version numbers 
 *****************************************************************************/

//#define ETM_VERSION      0x0100L // First Version
//#define ETM_VERSION      0x0102L // Updated ETM task state machine, removed recption of ATP events.
#define ETM_VERSION      0x0103L // Fixed issue regarding interaction with the ETM registration database 


#define ETM_API_VERSION  0x0100L // First Version


/******************************************************************************
 * Version of ETM CORE  
 *****************************************************************************/

//#define ETM_CORE_VERSION 0x0101L // First Version
//#define ETM_CORE_VERSION 0x0102L // 
#define ETM_CORE_VERSION 0x0103L // Updated core FIDs with new values and added 
                                 // support for Die-ID read.

/******************************************************************************
 * Version of ETM AUDIO  
 *****************************************************************************/

//#define ETM_AUDIO_VERSION 0x0100L // First Version
//#define ETM_AUDIO_VERSION 0x0101L // Version updated with support of new AUDIO parameters - not yet fully tested
#define ETM_AUDIO_VERSION 0x0102L // Implemented callback function instead of msg handling.
#endif // End of ETM_VERSION


/******************************************************************************
 * Version of ETM PWR  
 *****************************************************************************/

#define ETM_PWR_VERSION 0x0100L // First Version
