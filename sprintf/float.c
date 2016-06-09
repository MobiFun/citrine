/*
 * Embedded [v]sprintf() implementation by Michael Spacefalcon,
 * loosely based on the 4.3BSD-Tahoe version.
 *
 * This module contains the floating point conversion functions.
 */

#include <sys/types.h>
#include <ctype.h>
#include "defs.h"

extern double modf();

extern u_char * _sprintf_field(u_char *op, int width, int flags, int sign,
			u_char *body, int size, int dprec, int fpprec);

#define	MAX_INT_DIGITS	10	/* 2^32-1 in decimal */
#define	MAXFRACT	16	/* max sensible for 64-bit double */
#define	DEFPREC		6

static char *
emit_integer_portion(unsigned number, char *endp)
{
	char *t = endp;

	do {
		*--t = tochar(number % 10);
		number /= 10;
	} while (number);
	return (t);
}

static int
f_round(double fract, char *start, char *end, int sign)
{
	double tmp;

	(void)modf(fract * 10, &tmp);
	if (tmp > 4)
		for (;; --end) {
			if (*end == '.')
				--end;
			if (++*end <= '9')
				break;
			*end = '0';
		}
	/* ``"%.3f", (double)-0.0004'' gives you a negative 0. */
	else if (sign == '-')
		for (;; --end) {
			if (*end == '.')
				--end;
			if (*end != '0')
				break;
			if (end == start)
				return(1);	/* clear the -ve */
		}
	return(0);
}

u_char *
_sprintf_percent_f(u_char *op, int width, int flags, int sign,
		double number, int prec)
{
	char buf[MAX_INT_DIGITS + 1 + MAXFRACT];
	int extra_prec = 0;
	int origsign = sign;
	double fract, tmp;
	char *start, *t;

	/* first order of business: weed out infinities and NaNs */
	if (isinf(number)) {
		if (number < 0)
			sign = '-';
		return _sprintf_field(op, width, flags, sign, "Inf", 3, 0, 0);
	}
	if (isnan(number))
		return _sprintf_field(op, width, flags, sign, "NaN", 3, 0, 0);
	/* OK, we know it's a valid real like in the good old VAX days */
	if (number < 0) {
		sign = '-';
		number = -number;
	}
	fract = modf(number, &tmp);
	if (tmp > (double) 0xFFFFFFFF)
		return _sprintf_field(op, width, flags, sign, "Toobig", 6,
					0, 0);
	start = emit_integer_portion((unsigned) tmp, buf + MAX_INT_DIGITS);
	if (prec > MAXFRACT) {
		extra_prec = prec - MAXFRACT;
		prec = MAXFRACT;
	} else if (prec == -1)
		prec = DEFPREC;
	t = buf + MAX_INT_DIGITS;
	/*
	 * if precision required or alternate flag set, add in a
	 * decimal point.
	 */
	if (prec || flags&ALT)
		*t++ = '.';
	/* if requires more precision and some fraction left */
	if (fract) {
		if (prec)
			do {
				fract = modf(fract * 10, &tmp);
				*t++ = tochar((int)tmp);
			} while (--prec && fract);
		if (fract && f_round(fract, start, t - 1, sign))
			sign = origsign;
	}
	return _sprintf_field(op, width, flags, sign, start, t - start,
				0, prec + extra_prec);
}
