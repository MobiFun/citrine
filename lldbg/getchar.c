extern int lldbg_serial_in_poll();

int
lldbg_getchar()
{
	register int c;

	do
		c = lldbg_serial_in_poll();
	while (c < 0);
	return c;
}
