/*
 * This header file is a FreeCalypso addition, and it is intended to serve
 * as a workaround for TI's habit of requiring a ton of -D options to be
 * supplied on the compilation line for every single module.
 *
 * I am just now beginning to integrate the G23 protocol stack.  L1, GPF and
 * CCD have already been integrated, and thus predate the addition of this
 * header file.  However, I expect that all G23 code that uses the headers
 * under include/condat (which used to be g23m/condat/com/include) will need
 * to include this FreeCalypso header as well, starting with comlib.
 *
 * The definitions set here have been taken from TCS211 pdt_*.mak makefiles.
 * They are given as -D options when compiling every module in the group
 * just described, and are obviously constant, as in independent of any
 * possible target or feature configuration.
 */

#define	_TARGET_		1
#define	ALR			1
#define	CCDTABLES_EXTERN	1
#define	NEW_ENTITY		1
#define	NEW_FRAME		1
#define	OPTION_MULTITHREAD	1
#define	SHARED_VSI		1
