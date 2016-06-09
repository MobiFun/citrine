/*
 * These functions are normally defined in the DL protocol stack entity,
 * and are called by L1S.  When building L1 standalone, we need to
 * provide stubs.
 */

void *dll_read_dcch()
{
	return(0);
}

void *dll_read_sacch()
{
	return(0);
}

void dll_dcch_downlink()
{
}
