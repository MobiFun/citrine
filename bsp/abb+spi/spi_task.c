/*****************************************************************************/
/*                                                                           */
/*  Name        spi_task.c						     */
/*                                                                           */
/*  Function    this file contains the main SPI function: spi_core.	     */
/*		It contains the body of the SPI task.			     */
/*		It will initialize the SPI and then wait for messages	     */
/*		or functions calls.					     */
/*                                                                           */
/*  Version	0.1							     */
/*  Author	Candice Bazanegue					     */
/*									     */
/*  Date       	Modification						     */
/*  ------------------------------------				     */
/*  20/08/2000	Create							     */
/*  01/09/2003	Modfication         					     */
/*  Author	Pascal Puel						     */
/*									     */
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved */
/*****************************************************************************/

#include "../../include/config.h"

#include "../iq.h"

#if 0	// FreeCalypso
  #include "power/power.h"   // for Switch_ON()
#endif

#include "../../riviera/rv/rv_defined_swe.h"	   // for RVM_PWR_SWE
#include "../../riviera/rvm/rvm_use_id_list.h"
#include "spi_env.h"
#include "spi_process.h"
#include "spi_task.h"

#ifdef RVM_PWR_SWE
  #include "pwr/pwr_liion_cha.h"
  #include "pwr/pwr_disch.h"
  #include "pwr/pwr_process.h"
  #include "pwr/pwr_env.h"
#endif



/*******************************************************************************
** Function         spi_core
**
** Description      Core of the spi task, which initiliazes the spi SWE and 
**		    waits for messages.
**					
*******************************************************************************/
T_RV_RET spi_core(void)
{	
    BOOLEAN error_occured = FALSE;
    T_RV_HDR * msg_ptr;

    rvf_send_trace("SPI_task: Initialization", 24, NULL_PARAM,
			RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);
    SPI_GBL_INFO_PTR->SpiTaskReady = TRUE;

#if CONFIG_INCLUDE_L1
    /* Unmask External Interrupt once the SPI task is started */
    #if (CHIPSET == 12)
      // Unmask ABB ext interrupt
      F_INTH_ENABLE_ONE_IT(C_INTH_ABB_IRQ_IT);
    #else
      // Unmask external (ABB) interrupt
      IQ_Unmask(IQ_EXT);
    #endif
#endif
#if 0	// FreeCalypso: deferring until UI integration
    // Get the switch on cause from ABB.
    Set_Switch_ON_Cause();
#endif

    /* loop to process messages */
    while (error_occured == FALSE)
    {
	/* Wait for the necessary events (infinite wait for a msg in the mailbox 0). */
	UINT16 received_event = rvf_wait (0xffff, 0);

	/* If an event related to mailbox 0 is received, then */
	if (received_event & RVF_TASK_MBOX_0_EVT_MASK) 
	{
	    /* Read the message in the driver mailbox and delegate action..*/
	    msg_ptr = (T_RV_HDR *) rvf_read_mbox(SPI_MAILBOX);

            #ifdef RVM_PWR_SWE
		if(spi_process(msg_ptr)) 
		{
			pwr_process(msg_ptr);
		}
            #else
		spi_process(msg_ptr);
	    #endif
	}

        #ifdef RVM_PWR_SWE
	/* Timers */
        if (received_event & SPI_TIMER0_WAIT_EVENT)
        {
           pwr_bat_test_timer_process();
        }

	if (received_event & SPI_TIMER1_WAIT_EVENT)
	/* timer used to detect the end of the CI charge */
        {
           pwr_CI_charge_timer_process();
        }

        if (received_event & SPI_TIMER2_WAIT_EVENT)
	/* timer used to detect the end of the CV charge */
	{
           pwr_CV_charge_timer_process();
        }
        
	if (received_event & SPI_TIMER3_WAIT_EVENT) 
	/* timer used to check the battery discharge level */
	{
           pwr_discharge_timer_process();
        }
	#endif
    }	 // end of while
    return RV_OK;	
}



/*******************************************************************************
* Function	  : spi_adc_on
*
* Description : Put the variable is_adc_on of the T_SPI_GBL_INFO structure
*		to TRUE.
*		This variable is used for the battery management. 
*               This function is called by the CST entity.
*
* Parameters  : None
*
* Return      : None
* 
*******************************************************************************/
void spi_adc_on (void)
{
   SPI_GBL_INFO_PTR->is_adc_on = TRUE;

   rvf_send_trace("SPI: ADC are on",15,
                   NULL_PARAM,
                   RV_TRACE_LEVEL_DEBUG_LOW,
                   SPI_USE_ID);
}
