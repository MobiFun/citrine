/*
 * This header file is a FreeCalypso addition; ditto for the
 * #include "gpfconf.h" lines which had to be added near the beginning
 * of just about every original GPF header file.
 *
 * In their original form, GPF sources and headers required all of the
 * library compile-time configuration settings (i.e., those config
 * settings which affect the library build, rather than stuff set in
 * the separately-linked configuration module) to be given as -D arguments
 * on the compilation command line.  It would have been fine if these
 * -D definitions were needed only for the build of GPF libs themselves,
 * but the #ifdef logic in the header files means that these -D defs
 * were also needed for every user of these GPF headers as well!
 *
 * This bizarre quirk of the GPF headers is fully consistent with TI's
 * general approach of supplying an insanely long list of -I's and -D's
 * on the cl470 compilation command line for every single module,
 * first through BuSyB-generated makefiles, then later through SBuild
 * voodoo.  Needless to say, we wish no part of that lunacy in FreeCalypso.
 *
 * Because of the nature of the preprocessor definitions needed for GPF
 * (some are totally fixed, others may be tweaked for debugging, but
 * none are of the target/feature-dependent sort), I decided to create
 * this gpfconf.h header file instead of adding this junk to the
 * config.h mechanism.
 */

/* the following two are needed unquestionably */
#define	_TARGET_	1
#define	_NUCLEUS_	1

/*
 * GPF build configuration settings like debug and memory supervision
 * are selected here.  For now I'm setting the "official" configuration
 * to match that of the GPF libs in the Leonardo semi-src, the one that
 * runs on the GTA02 modem as leo2moko production-quality firmware.
 */
#define	NU_DEBUG	1
