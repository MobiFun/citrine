/*
 * This module implements ASCII command entry via the serial port,
 * with normal echo and minimal editing (rubout and kill).
 *
 * The command string buffer is bss-allocated here as well.  It is
 * sized to allow a maximum-size S-record to be sent as a command,
 * as that is how we expect flash loading and XRAM chain-loading
 * to be done.
 */

#define	MAXCMD	527

char lldbg_command[MAXCMD+1];

/*
 * The command_entry() function takes no arguments, and begins by waiting
 * for serial input - hence the prompt should be printed before calling it.
 *
 * This function returns when one of the following characters is received:
 * CR - accepts the command
 * ^C or ^U - cancels the command
 *
 * The return value is non-zero if a non-empty command was accepted with CR,
 * or 0 if the user hit CR with no input or if the command was canceled
 * with ^C or ^U.  In any case a CRLF is sent out the serial port
 * to close the input echo line before this function returns.
 */
lldbg_command_entry()
{
	int inlen, ch;

	for (inlen = 0; ; ) {
		ch = lldbg_getchar();
		if (ch >= ' ' && ch <= '~') {
			if (inlen < MAXCMD) {
				lldbg_command[inlen++] = ch;
				lldbg_putchar(ch);
			} else
				/* putchar(7) */;
			continue;
		}
		switch (ch) {
		case '\r':
		case '\n':
			lldbg_command[inlen] = '\0';
			lldbg_putchar('\n');
			return(inlen);
		case '\b':	/* BS */
		case 0x7F:	/* DEL */
			if (inlen) {
				lldbg_putchar('\b');
				lldbg_putchar(' ');
				lldbg_putchar('\b');
				inlen--;
			} else
				/* putchar(7) */;
			continue;
		case 0x03:	/* ^C */
			lldbg_putchar('^');
			lldbg_putchar('C');
			lldbg_putchar('\n');
			return(0);
		case 0x15:	/* ^U */
			lldbg_putchar('^');
			lldbg_putchar('U');
			lldbg_putchar('\n');
			return(0);
		default:
			/* putchar(7) */;
		}
	}
}
