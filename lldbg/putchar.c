extern void lldbg_serial_out();

void
lldbg_putchar(ch)
{
	if (ch == '\n')
		lldbg_serial_out('\r');
	lldbg_serial_out(ch);
}
