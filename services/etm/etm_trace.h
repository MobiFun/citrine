/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm_trace.h
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
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/


#ifndef _ETM_TRACE_H_
#define _ETM_TRACE_H_

#include "etm.h"

void tr_etm(unsigned trmask, char *format, ...);
void tr_etm_hexdump(unsigned int mask, const void *p, int size);
void hexdump_buf(char *buf, int size);
int  print_int_as_hex(unsigned int n);

#endif // End of ETM_TRACE
