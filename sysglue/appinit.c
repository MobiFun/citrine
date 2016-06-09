/*
 * This module contains our Application_Initialize() function,
 * based on the disassembly of the binary object version in the
 * Leonardo semi-src.
 */

#include "../include/config.h"

Application_Initialize()
{
	Init_Target();
	/*
	 * The original version calls Init_Drivers() at this point,
	 * but that function is nothing more than a short sequence
	 * of calls to other functions, so I've inlined it.
	 */
	ABB_Sem_Create();
	flash_chip_init();	/* FreeCalypso addition */
	ffs_main_init();
	/*
	 * The call to pcm_init() used to be at the end of ffs_main_init(),
	 * but I factored it out. - Space Falcon
	 */
#if CONFIG_INCLUDE_PCM
	pcm_init();
#endif
	rvf_init();
	rvm_init();
	create_tasks();
#if CONFIG_INCLUDE_SIM
	SIM_Initialize();
#endif
	/* end of Init_Drivers() */
#if CONFIG_INCLUDE_L1
	Cust_Init_Layer1();
#endif
	Init_Serial_Flows();
#if CONFIG_INCLUDE_GPF
	StartFrame();
#endif
	Init_Unmask_IT();
}
