/*
 * Embedded [v]sprintf() implementation by Michael Spacefalcon,
 * loosely based on the 4.3BSD-Tahoe version.
 *
 * This module contains the core of the vsprintf() function, which may
 * be either used directly by user code or called by the sprintf()
 * trivial wrapper.
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdarg.h>
#include "defs.h"

extern u_char * _sprintf_integer(u_char *op, int width, int flags, int sign,
					unsigned number, int base, int prec);
extern u_char * _sprintf_percent_f(u_char *op, int width, int flags, int sign,
					double number, int prec);

u_char *
_sprintf_field(u_char *op, int width, int flags, int sign,
		u_char *body, int size, int dprec, int fpprec)
{
	int fieldsz;		/* field size expanded by sign, etc */
	int realsz;		/* field size expanded by decimal precision */
	int n;			/* scratch */

	/*
	 * All reasonable formats wind up here.  At this point,
	 * `body' points to a string which (if not flags&LADJUST)
	 * should be padded out to `width' places.  If
	 * flags&ZEROPAD, it should first be prefixed by any
	 * sign or other prefix; otherwise, it should be blank
	 * padded before the prefix is emitted.  After any
	 * left-hand padding and prefixing, emit zeroes
	 * required by a decimal [diouxX] precision, then print
	 * the string proper, then emit zeroes required by any
	 * leftover floating precision; finally, if LADJUST,
	 * pad with blanks.
	 */

	/*
	 * compute actual size, so we know how much to pad
	 * fieldsz excludes decimal prec; realsz includes it
	 */
	fieldsz = size + fpprec;
	if (sign)
		fieldsz++;
	if (flags & HEXPREFIX)
		fieldsz += 2;
	realsz = dprec > fieldsz ? dprec : fieldsz;

	/* right-adjusting blank padding */
	if ((flags & (LADJUST|ZEROPAD)) == 0 && width)
		for (n = realsz; n < width; n++)
			*op++ = ' ';
	/* prefix */
	if (sign)
		*op++ = sign;
	if (flags & HEXPREFIX) {
		*op++ = '0';
		*op++ = (flags & UPPERCASE) ? 'X' : 'x';
	}
	/* right-adjusting zero padding */
	if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
		for (n = realsz; n < width; n++)
			*op++ = '0';
	/* leading zeroes from decimal precision */
	for (n = fieldsz; n < dprec; n++)
		*op++ = '0';

	/* the string or number proper */
	bcopy(body, op, size);
	op += size;
	/* trailing f.p. zeroes */
	while (--fpprec >= 0)
		*op++ = '0';
	/* left-adjusting padding (always blank) */
	if (flags & LADJUST)
		for (n = realsz; n < width; n++)
			*op++ = ' ';

	return(op);
}

int
vsprintf(buf0, fmt0, argp)
	u_char *buf0, *fmt0;
	va_list argp;
{
	u_char *op;		/* output buffer working ptr */
	u_char *fmt;		/* format string working ptr */
	int ch;			/* character from fmt */
	int n;			/* scratch integer */
	char *t;		/* scratch pointer */
	int flags;		/* flags as above */
	int prec;		/* precision from format (%.3d), or -1 */
	int width;		/* width from format (%8d), or 0 */
	char sign;		/* sign prefix (' ', '+', '-', or \0) */
	unsigned un;		/* unsigned number for conversion */

	op = buf0;
	for (fmt = fmt0; ; ++fmt) {
		for (; (ch = *fmt) && ch != '%'; ++fmt)
			*op++ = ch;
		if (!ch) {
out:			*op = '\0';
			return (op - buf0);
		}

		flags = 0; width = 0;
		prec = -1;
		sign = '\0';

rflag:		switch (*++fmt) {
		case ' ':
			/*
			 * ``If the space and + flags both appear, the space
			 * flag will be ignored.''
			 *	-- ANSI X3J11
			 */
			if (!sign)
				sign = ' ';
			goto rflag;
		case '#':
			flags |= ALT;
			goto rflag;
		case '*':
			/*
			 * ``A negative field width argument is taken as a
			 * - flag followed by a  positive field width.''
			 *	-- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			if ((width = va_arg(argp, int)) >= 0)
				goto rflag;
			width = -width;
			/* FALLTHROUGH */
		case '-':
			flags |= LADJUST;
			goto rflag;
		case '+':
			sign = '+';
			goto rflag;
		case '.':
			if (*++fmt == '*')
				n = va_arg(argp, int);
			else {
				n = 0;
				while (isascii(*fmt) && isdigit(*fmt))
					n = 10 * n + todigit(*fmt++);
				--fmt;
			}
			prec = n < 0 ? -1 : n;
			goto rflag;
		case '0':
			/*
			 * ``Note that 0 is taken as a flag, not as the
			 * beginning of a field width.''
			 *	-- ANSI X3J11
			 */
			flags |= ZEROPAD;
			goto rflag;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			n = 0;
			do {
				n = 10 * n + todigit(*fmt);
			} while (isascii(*++fmt) && isdigit(*fmt));
			width = n;
			--fmt;
			goto rflag;
		case 'L':
			flags |= LONGDBL;
			goto rflag;
		case 'h':
			flags |= SHORTINT;
			goto rflag;
		case 'l':
			flags |= LONGINT;
			goto rflag;
		case 'c':
			/*
			 * XXX reusing a variable of type char
			 * for an unrelated purpose
			 */
			sign = (char) va_arg(argp, int);
			op = _sprintf_field(op, width, flags, 0, &sign, 1,
						0, 0);
			break;
		case 'D':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'd':
		case 'i':
			n = va_arg(argp, int);
			if (n < 0) {
				un = -n;
				sign = '-';
			} else
				un = n;
			op = _sprintf_integer(op, width, flags, sign, un, 10,
						prec);
			break;
		case 'f':
			op = _sprintf_percent_f(op, width, flags, sign,
						va_arg(argp, double), prec);
			break;
		case 'n':
			n = op - buf0;
			if (flags & LONGINT)
				*va_arg(argp, long *) = n;
			else if (flags & SHORTINT)
				*va_arg(argp, short *) = n;
			else
				*va_arg(argp, int *) = n;
			break;
		case 'O':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'o':
			un = va_arg(argp, unsigned);
			op = _sprintf_integer(op, width, flags, 0, un, 8, prec);
			break;
		case 'p':
			/*
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
			/* NOSTRICT */
			un = (unsigned)va_arg(argp, void *);
			op = _sprintf_integer(op, width, flags | HEXPREFIX, 0,
						un, 16, prec);
			break;
		case 's':
			if (!(t = va_arg(argp, char *)))
				t = "(null)";
			if (prec >= 0) {
				/*
				 * can't use strlen; can only look for the
				 * NUL in the first `prec' characters, and
				 * strlen() will go further.
				 */
				char *p, *memchr();

				if (p = memchr(t, 0, prec)) {
					n = p - t;
					if (n > prec)
						n = prec;
				} else
					n = prec;
			} else
				n = strlen(t);
			op = _sprintf_field(op, width, flags, 0, t, n, 0, 0);
			break;
		case 'U':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'u':
			un = va_arg(argp, unsigned);
			op = _sprintf_integer(op, width, flags, 0, un, 10,
						prec);
			break;
		case 'X':
			flags |= UPPERCASE;
			/* FALLTHROUGH */
		case 'x':
			un = va_arg(argp, unsigned);
			/* leading 0x/X only if non-zero */
			if (flags & ALT && un != 0)
				flags |= HEXPREFIX;
			op = _sprintf_integer(op, width, flags, 0, un, 16,
						prec);
			break;
		case '\0':	/* "%?" prints ?, unless ? is NULL */
			goto out;
		default:
			*op++ = *fmt;
		}
	}
	/* NOTREACHED */
}
