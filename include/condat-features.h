/*
 * This header file is a FreeCalypso concoction; its inclusion will be added
 * to all of the C modules comprising the core TI/Condat G23 protocol stack,
 * along with config.h and fixedconf.h.  All definitions given herein used
 * to be -D options which had to be given on the compilation line for
 * every single module.
 *
 * In terms of the staticness vs. configurability, the options given here
 * fall somewhere in the middle between config.h and fixedconf.h.  The
 * values set here have been taken from the pdt_*.mak makefiles in the
 * Leonardo semi-src from Sotovik.
 */

#define	AT_INTERPRETER		1
#define	DTI2			1
#define	FF_ATI			1
#define	FF_CPHS			1
#define	FF_EM_MODE		1
#define	FF_MULTIBAND		1
#define	FF_HOMEZONE		1
#define	FF_MMI_SMS_DYNAMIC	1
#define	HAS_FLASH_EPROM		1
#define	PHONEBOOK_EXTENSION	1
#define	SIM_TOOLKIT		1
#define	SMS_PDU_SUPPORT		1
#define	UART			1
#define	USE_L1FD_FUNC_INTERFACE	1
#define	VOCODER_FUNC_INTERFACE	1
