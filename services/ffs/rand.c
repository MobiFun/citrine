/*
 * This version of rand() has been lifted from Ancient UNIX, and modified
 * to match the version used in TI's TCS211 GSM firmware, as revealed by
 * disassembly of rand.obj in the rts16le_flash.lib binary library used
 * by that semi-src package.  TI's version (most likely from their compiler
 * tools group, rather than the GSM fw group, but who knows) uses the
 * same trivial implementation of rand() as the original Ancient UNIX libc,
 * but with one change: TI's return value is right-shifted by 16 bits
 * compared to what the Ancient UNIX rand() would have returned.
 * The caller thus gets back only 15 pseudo-random bits rather than 31,
 * but then the lower bits of the original rand() return value are
 * known to be pretty bad.
 *
 * rand() is used by some FFS code in reclaim.c.  If we don't provide our
 * own version of rand() and let the linker pull the version from newlib,
 * the link fails because the latter uses malloc.  This ancient implementation
 * of rand() is quite poor, but my plan is to look into possibly adopting
 * some better PRNG after we get the basic TI GSM firmware reintegrated.
 */

static	long	randx = 1;

srand(x)
unsigned x;
{
	randx = x;
}

rand()
{
	return ((randx = randx * 1103515245 + 12345) & 0x7fffffff) >> 16;
}
