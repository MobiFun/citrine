/*
 * Embedded [v]sprintf() implementation by Michael Spacefalcon,
 * loosely based on the 4.3BSD-Tahoe version.
 *
 * This header file contains some internal definitions used by
 * different pieces of what used to be one giant _doprnt()
 * function/module.
 */

#define	todigit(c)	((c) - '0')
#define	tochar(n)	((n) + '0')

#define	LONGINT		0x01		/* long integer */
#define	LONGDBL		0x02		/* long double; unimplemented */
#define	SHORTINT	0x04		/* short integer */
#define	ALT		0x08		/* alternate form */
#define	LADJUST		0x10		/* left adjustment */
#define	ZEROPAD		0x20		/* zero (as opposed to blank) pad */
#define	HEXPREFIX	0x40		/* add 0x or 0X prefix */
#define	UPPERCASE	0x80		/* uppercase forms */
