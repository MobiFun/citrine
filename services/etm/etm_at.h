/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm_at.c (Support for AT-commands)
 *
 * @author	Kim T. Peteren (ktp@ti.com)
 * @version 0.1
 *

 *
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *  16/06/2003	Creation
 *  06/11/2003  Small updates  
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/

#ifndef __ETM_AT_H_
#define __ETM_AT_H_

#include "etm_env.h" // Need because use of T_ETM_ENV_CTRL_BLK 


/** External ref "global variables" structure. */
extern T_ETM_ENV_CTRL_BLK *etm_env_ctrl_blk;


#endif //__ETM_AT_H_
