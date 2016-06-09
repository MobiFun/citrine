/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm_trace.c
 *
 * @author	Kim T. Peteren (ktp@ti.com) and Mads Meisner-Jensen, mmj@ti.com
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


#include "etm_trace.h"
#include "etm_env.h"

#include "../../riviera/rvf/rvf_api.h"
#include "../../riviera/rvm/rvm_use_id_list.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>


/******************************************************************************
 * Prototypes
 *****************************************************************************/

void trstr(unsigned int mask, char *string);


/******************************************************************************
 * Target Tracing
 *****************************************************************************/

static unsigned int ttr_mask = TgTrFatal; //TgTrFatal; //TgTrAll;

void tr_etm_init(unsigned int mask)
{
    ttr_mask = mask;
}


void tr_etm(unsigned int mask, char *format, ...)
{
    va_list args;
    static char buf[256];

    if (ttr_mask & mask) {
        // build string ala tr() then call str()
        va_start(args, format);
        vsprintf(buf, format, args);
        trstr(mask, buf);
        va_end(args);
    }
}


void trstr(unsigned int mask, char *string)
{
    if (ttr_mask & mask) {
        rvf_send_trace(string, strlen(string), NULL_PARAM,
                       RV_TRACE_LEVEL_WARNING, ETM_USE_ID);
        rvf_delay(10);
    }
}


void tr_etm_hexdump(unsigned int mask, const void *p, int size)
{
    unsigned int type, module;
  
    if (!(ttr_mask & mask))
        return;
    
    hexdump_buf((char*) p, size);
}


/******************************************************************************
 * Hexdumping Functions
 *****************************************************************************/

void etm_trace(char *string, int level)
{
    rvf_send_trace(string, strlen(string), NULL_PARAM, level, ETM_USE_ID);
    rvf_delay(20);
}


int sprint_int_as_hex(char *buf, unsigned int n, int width, char padding)
{
    unsigned int m = n; // MUST be unsigned because it will be right shifted
    int size = 0;
    int i;
    char digit;
    char *buf_start = buf;

    // Count number of digits in <n>
    do {
        size++;
    } while (m >>= 4);

    // Shift significant part of <n> into the top-most bits
    n <<= 4 * (8 - size);

    // Pad output buffer with <padding>
    if (0 < width && width <= 8) {
        width = (width > size ? width - size : 0);
        while (width--)
            *buf++ = padding;
    }

    // Convert <n>, outputting the hex digits
    for (i = 0; i < size; i++) {
        digit  = (n >> 28) & 0xF;
        digit += (digit < 10 ? '0' : 'A' - 10);
        *buf++ = digit;
        n <<= 4;
    }

    // Null terminate
    *buf = 0;

    return buf - buf_start;
}

int printf_int_as_hex(unsigned int n, int width, char padding)
{
    char string[8+1];
    int length;

    length = sprint_int_as_hex(string, n, width, padding);
    etm_trace(string, RV_TRACE_LEVEL_DEBUG_LOW);

    return length;
}


int print_int_as_hex(unsigned int n)
{
    return printf_int_as_hex(n, 0, 0);
}


void hexdump_buf(char *buf, int size)
{
    int n, i, multiline;
    char string[(8+1) + (16+1) + (3*16) + 1];
    char *s;
    
    multiline = (size > 16);

    while (size > 0)
    {
        s = string;
        n = (size > 16 ? 16 : size);

        // Print address
        if (multiline) {
            s += sprint_int_as_hex(s, (unsigned int) buf, 8, ' ');
            *s++ = ' ';
        }

        // Print the textual representation
        for (i = 0; i < n; i++)
            *s++ = (buf[i] >= ' ' && buf[i] < 127 ? buf[i] : '.');

        // Pad textual representation with spaces
        if (multiline)
            for (i = 0; i < 16 - n; i++)
                *s++ = ' ';

        // Print hexedecimal bytes
        for (i = 0; i < n; i++) {
            *s++ = ' ';
            s += sprint_int_as_hex(s, (unsigned int) buf[i] & 0xFF, 2, '0');
        }

        *s = 0;

        etm_trace(string, RV_TRACE_LEVEL_DEBUG_LOW);

        buf  += 16;
        size -= 16;
    }
}
