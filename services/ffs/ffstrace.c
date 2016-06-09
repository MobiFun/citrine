/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * ffs deprecated testing
 *
 * $Id: ffstrace.c 1.32.1.10 Thu, 18 Dec 2003 10:50:52 +0100 tsj $
 *
 ******************************************************************************/

#include "ffs.h"
#include "drv.h"
#include "ffstrace.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../../riviera/rvf/rvf_api.h"
#include "../../riviera/rv/rv_general.h"
#include "../../riviera/rvm/rvm_use_id_list.h"

/******************************************************************************
 * LED control
 *****************************************************************************/

#if 0 //(TARGET == 1)

static uint8 led_state     =   0;
static uint8 led_countbits =   0; // number of counter bits
static uint8 led_mask      = 0x0; // mask containing the counter bits

// configure the number of counter bits in the leds
void led_config(unsigned char n)
{
    led_countbits = (n <= 8 ? n : 0);
    led_mask      = (n <= 8 ? (1 << n) - 1 : 0);
}

// just set the bits, no checking whatsoever
void led_set(unsigned char n)
{
    *(char *) 0x2800000 = led_state = n;
}

void led_counter(unsigned char n)
{
    *(char *) 0x2800000 = led_state = led_state & ~led_mask | (n & led_mask);
}

void led_on(unsigned char n)
{
    *(char *) 0x2800000 = led_state = led_state | (1 << (led_countbits + n));
}

void led_off(unsigned char n)
{
    *(char *) 0x2800000 = led_state = led_state & ~(1 << (led_countbits + n));
}
// FIXME
void led_toggle(unsigned char n)
{
    *(char *) 0x2700000 = led_state = led_state ^ (1 << (led_countbits + n));
}
#endif


/******************************************************************************
 * Target Tracing
 *****************************************************************************/

#if (TARGET == 1)

static unsigned int ttr_mask = TTrFatal | TTrTest;

void ttr_init(unsigned int mask)
{
    ttr_mask = mask | TTrFatal | TTrTest;
}

void ttr(unsigned int mask, char *format, ...)
{
    va_list args;
    static char buf[256];

    if (ttr_mask & mask)
    {
        // build string ala tr() then call str()
        va_start(args, format);
        vsprintf(buf, format, args);
        str(mask, buf);
        va_end(args);
    }
}

void str(unsigned mask, char *string)
{
    if (ttr_mask & mask) {
        rvf_send_trace(string, strlen(string), NULL_PARAM,
                       RV_TRACE_LEVEL_WARNING, FFS_USE_ID);
        rvf_delay(5);
    }
}


/******************************************************************************
 ** PC side Tracing and logging
 *****************************************************************************/

#else // (TARGET == 0)

static int tr_mask;    // bitmask of which modules to trace

static int tr_spaces;  // number of spaces to indent per level
static FILE *tr_fd;    // unused; file descriptor of file to write traces to


void tr_init(unsigned int mask, int spaces, char *filename)
{
    tr_mask = mask;
    tr_spaces = spaces;

    if (filename == NULL) {
        tr_fd = stdout;
    }
    else {
        if ( !(tr_fd = fopen(filename, "a+b")) ) {
            fprintf(stderr, "failed to open logfile: %s for append\n", filename);
            exit(1);
        }
    }
}

// Trace/Log the printf-like string if abs(level) > tr_level. If level is
// negative, the sematics are the same except that no indentation will occur
void tr(int type, unsigned int mask, char *format, ...)
{
    va_list args;
    int indent;
    static int indent_level = 0;
    static char buf[1024];
    const char spaces[] =
        "                                        "
                           "                                        "
                           "                                        "
                           "                                        ";
        
    if ((mask & tr_mask) == 0)
        return;

    // If tracing/debugging trace system
    if ((tr_mask & TrTrace) && (type & TR_END))
        fprintf(tr_fd, "END(%d)\n", indent_level);
    
    if (type & TR_END)
        indent_level--;

    indent = (type & TR_NULL ? 0 : indent_level);

    if (strlen(format) > 0)
    {
        va_start(args, format);
        vsprintf(buf, format, args);
        
        indent = tr_spaces * indent;
        if (indent < 0) {
            fprintf(tr_fd, "WARNING: tr() indenting too left (%d)\n",
                    indent_level);
            indent = 0;
        }
        if (indent > sizeof(spaces) - 1) {
            fprintf(tr_fd, "WARNING: tr() indenting too right (%d)\n",
                    indent_level);
            indent = sizeof(spaces) - 1;
        }
        fprintf(tr_fd, "%s%s", &spaces[sizeof(spaces) - 1 - indent], buf);
        fflush(tr_fd);
    }
    if (type & TR_BEGIN)
        indent_level++;

    // If tracing/debugging trace system
    if ((tr_mask & TrTrace) && (type & TR_BEGIN))
        fprintf(tr_fd, "BEGIN(%d)\n", indent_level);
}

#endif // (TARGET == 0)


/******************************************************************************
 ** Common Tracing and logging
 *****************************************************************************/

int tr_query(int mask)
{
#if (TARGET == 1)
    return (ttr_mask & mask);
#else
    return (tr_mask & mask);
#endif
}
