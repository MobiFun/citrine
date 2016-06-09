#include <stdarg.h>

int
lldbg_printf(char *fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = lldbg_doprnt(fmt, ap);
	va_end(ap);
	return(len);
}
