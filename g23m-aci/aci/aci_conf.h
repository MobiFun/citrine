/*
 * This header file is a FreeCalypso addition.  It serves a purpose similar to
 * fixedconf.h and condat-features.h in gsm-fw/include, but this one is local
 * to the ACI component.
 */

#define	DTI	1

#if CONFIG_UI
#define	MMI	2
#define	MFW	1
#else
#define	MMI	0
#define	ACI	1
#endif
