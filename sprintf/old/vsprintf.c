#include <stdarg.h>

int
vsprintf(str, fmt, ap)
	va_list ap;
	char *str, *fmt;
{
	char *strptr;
	int len;

	strptr = str;
	len = _doprnt(fmt, ap, &strptr);
	*strptr = '\0';
	return(len);
}
