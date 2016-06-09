#include <stdarg.h>

int
sprintf(char *strdest, char *fmt, ...)
{
	va_list ap;
	char *strptr;
	int len;

	strptr = strdest;
	va_start(ap, fmt);
	len = _doprnt(fmt, ap, &strptr);
	va_end(ap);
	*strptr = '\0';
	return(len);
}
