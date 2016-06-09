#include <stdarg.h>

int
sprintf(char *strdest, char *fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = vsprintf(strdest, fmt, ap);
	va_end(ap);
	return(len);
}
