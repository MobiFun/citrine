/*
 * Many commands take hex arguments.  This module contains the parse_hexarg()
 * function, which is a wrapper around strtoul that performs some additional
 * checks.
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>

lldbg_parse_hexarg(arg, maxdigits, valp)
	char *arg;
	int maxdigits;
	u_long *valp;
{
	char *cp = arg, *bp;
	int len;

	if (cp[0] == '0' && (cp[1] == 'x' || cp[1] == 'X'))
		cp += 2;
	for (bp = cp; *cp; cp++)
		if (!isxdigit(*cp))
			return(-1);
	len = cp - bp;
	if (len < 1 || len > maxdigits)
		return(-1);
	*valp = strtoul(arg, 0, 16);
	return(0);
}
