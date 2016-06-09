/*
 * Embedded [v]sprintf() implementation by Michael Spacefalcon,
 * loosely based on the 4.3BSD-Tahoe version.
 *
 * This module contains the integer conversion functions.
 */

#include <sys/types.h>
#include <ctype.h>
#include "defs.h"

extern u_char * _sprintf_field(u_char *op, int width, int flags, int sign,
			u_char *body, int size, int dprec, int fpprec);

static const char lcdigits[] = "0123456789abcdef";
static const char ucdigits[] = "0123456789ABCDEF";

u_char *
_sprintf_integer(u_char *op, int width, int flags, int sign,
		unsigned number, int base, int prec)
{
	const char *digits;
	char buf[12];
	char *t, *endp;

	/*
	 * ``... diouXx conversions ... if a precision is
	 * specified, the 0 flag will be ignored.''
	 *	-- ANSI X3J11
	 */
	if (prec >= 0)
		flags &= ~ZEROPAD;

	if (flags & UPPERCASE)
		digits = ucdigits;
	else
		digits = lcdigits;

	/*
	 * ``The result of converting a zero value with an
	 * explicit precision of zero is no characters.''
	 *	-- ANSI X3J11
	 */
	t = endp = buf + sizeof(buf);
	if (number != 0 || prec != 0) {
		do {
			*--t = digits[number % base];
			number /= base;
		} while (number);
		if (flags & ALT && base == 8 && *t != '0')
			*--t = '0'; /* octal leading 0 */
	}
	return _sprintf_field(op, width, flags, sign, t, endp - t, prec, 0);
}
