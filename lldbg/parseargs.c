/*
 * This module contains the parse_args() function, which parses the "rest"
 * part of an entered command into an argc/argv-style argument array.
 */

lldbg_parse_args(unparsed, minargs, maxargs, argv, argcp)
	char *unparsed;
	int minargs, maxargs;
	char **argv;
	int *argcp;
{
	int argc;
	char *cp;

	argc = 0;
	for (cp = unparsed; ; ) {
		while (*cp == ' ')
			cp++;
		if (!*cp)
			break;
		if (argc >= maxargs) {
			lldbg_printf("ERROR: too many arguments\n");
			return(-1);
		}
		argv[argc++] = cp;
		while (*cp && *cp != ' ')
			cp++;
		if (*cp)
			*cp++ = '\0';
	}
	if (argc < minargs) {
		lldbg_printf("ERROR: too few arguments\n");
		return(-1);
	}
	argv[argc] = 0;
	if (argcp)
		*argcp = argc;
	return(0);
}
