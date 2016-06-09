/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm_audio_err.h (Support for AUDIO commands)
 *
 * @author	Kim T. Peteren (ktp@ti.com)
 * @version 0.1
 *

 *
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *  30/06/2003	Creation
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/

#ifndef _ETM_AUDIO_ERR_H_
#define _ETM_AUDIO_ERR_H_

/******************************************************************************
 * ERRORS
 *****************************************************************************/

// Module private (normally target-side) errors are in the range: [ -2..-47]
enum ETM_AUDIO_ERRORS_TG {
    ETM_AUDIO_FATAL = -3   // Fatal error
};

// Module private (normally PC-side) errors are in the range:     [-90..-99]
//enum ETM_AUDIO_ERRORS_PC {
 
//};

#endif // end of _ETM_AUDIO_ERR_H_
