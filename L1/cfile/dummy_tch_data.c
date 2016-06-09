/*
 * [rt]x_tch_data() calls appear to be some old API for passing CSD payloads
 * between L1 and the higher layers.  If this API was ever used at all,
 * it is so old that even the TSM30 code does not use it; the new API that
 * took its place is dll_data_[du]l().  However, the code in l1_cmplx.c
 * still calls the old API, thus stub functions are needed.  TSM30 and
 * Leonardo versions have these stubs in the dl1_com module, but that
 * module no longer exists in our version - see l1_isr_glue.c for the
 * explanation.  The LoCosto version has a stub (in sys_dummy.c) only
 * for tx_tch_data() - the other function gets called only when IDS
 * is not enabled.
 *
 * This module provides the necessary stubs for our version.
 */
 
void rx_tch_data()
{
  return;
}
 
char *tx_tch_data()
{
  return(0);
}
