/*****************************************************************************/
/*                                                                           */
/*  Name       spi_process.c                                                 */
/*                                                                           */
/*  Function    this file contains the spi_process function, used to         */
/*		handle messages received in the SPI mailbox, and used to     */
/*              access the ABB. It is called by the spi task core.           */
/*                                                                           */
/*  Version    0.1                                                           */
/*                                                                           */
/*  Date       Modification                                                  */
/*  ------------------------------------                                     */
/*  20/08/2000 Create                                                        */
/*                                                                           */
/* Author                                                                    */
/*                                                                           */
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved */
/*****************************************************************************/

#include "../../riviera/rvf/rvf_api.h"
#include "spi_api.h"
#include "../../riviera/rvm/rvm_use_id_list.h"
#include "spi_process.h"
#include "../../riviera/rv/rv_defined_swe.h"	   // for RVM_PWR_SWE



//#ifndef _WINDOWS
//#include "iq.h"
//#endif

/*******************************************************************************
** Function         spi_process
**
*******************************************************************************/
UINT8 spi_process(T_RV_HDR * msg_ptr)
{
   UINT16 data ;
//   static UINT8 int_nb = 0;

   if(msg_ptr != NULL)
   {
      switch (msg_ptr->msg_id)
      {
         case ABB_EXT_IRQ_EVT:
		 {
            /* Call to the low-level driver function : interrupt status register reading */
            data = ABB_Read_Register_on_page(PAGE0, ITSTATREG);

//#ifndef _WINDOWS
            // SW workaround to avoid an ABB interrupt occuring to early after the application start.
			// The first ABB interrupt is skipped.

//            if((int_nb == 0) && (data == ADCEND_IT_STS))
//            {
//              int_nb++;

              /* Unmask keypad interrupt */
//              IQ_Unmask(IQ_EXT);
//            }
//            else
//            {

			/* Callback function */
//            if(msg_ptr->callback_func != NULL)
//            {
//               msg_ptr->callback_func(&data);
//            }
//          }
//#else

            /* Callback function */
            if(msg_ptr->callback_func != NULL)
            {
               msg_ptr->callback_func(&data);
            }
//#endif

            rvf_free_buf ((void *) msg_ptr);

		   break;
		 }

         case SPI_ABB_READ_EVT:
		 {
           rvf_send_trace("SPI_task: SPI_READ_ABB_EVT received", 35, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);

           /* Call to the low-level driver function */
           data = ABB_Read_Register_on_page(((T_SPI_READ *)msg_ptr)->page, ((T_SPI_READ *)msg_ptr)->address);

           /*  Callback function */
           if(((T_SPI_READ *)msg_ptr)->os_hdr.callback_func != NULL)
           {
              ((T_SPI_READ *)msg_ptr)->os_hdr.callback_func(&data);
           }

           rvf_free_buf ((void *) msg_ptr);

           return 0;
		 }

         case SPI_ABB_WRITE_EVT:
		 {
           rvf_send_trace("SPI_task: SPI_WRITE_ABB_EVT received", 36, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);

           /* Call to the low-level driver function */
           ABB_Write_Register_on_page(((T_SPI_WRITE *)msg_ptr)->page, ((T_SPI_WRITE *)msg_ptr)->address, ((T_SPI_WRITE *)msg_ptr)->data);

           rvf_free_buf ((void *) msg_ptr);

           return 0;
		 }

         case SPI_ABB_CONF_ADC_EVT:
		 {
           rvf_send_trace("SPI_task: SPI_ABB_CONF_ADC_EVT received", 39, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);

           /* Call to the low-level driver function */
           ABB_Conf_ADC(((T_SPI_ABB_CONF_ADC *)msg_ptr)->channels, ((T_SPI_ABB_CONF_ADC *)msg_ptr)->itval);                              

           rvf_free_buf ((void *) msg_ptr);

           return 0;
		 }

         case SPI_ABB_READ_ADC_EVT:
		 {
           rvf_send_trace("SPI_task: SPI_ABB_READ_ADC_EVT received", 39, NULL_PARAM, RV_TRACE_LEVEL_DEBUG_LOW, SPI_USE_ID);

           /* Call to the low-level driver function */
           ABB_Read_ADC(((T_SPI_ABB_READ_ADC *)msg_ptr)->Buff);

           /* Callback function */
           if(((T_SPI_ABB_READ_ADC *)msg_ptr)->callback_func != NULL)
           {
              ((T_SPI_ABB_READ_ADC *)msg_ptr)->callback_func();
           }

           rvf_free_buf ((void *) msg_ptr);

           return 0;
		 }

         default:
		 {
           /* Unknown message has been received */
           #ifdef RVM_PWR_SWE
           rvf_send_trace("SPI_task : Received an unknown or a PWR message",47, NULL_PARAM ,
                           RV_TRACE_LEVEL_DEBUG_HIGH, SPI_USE_ID);
           #else
           rvf_send_trace("SPI_task : Received an unknown message",38, NULL_PARAM ,
                           RV_TRACE_LEVEL_DEBUG_HIGH, SPI_USE_ID);

           rvf_free_buf ((void *) msg_ptr);
           #endif
		   
           return 1; 
		 }
      }	   // end of switch
   }	// end of if (msg_ptr != NULL)
   return 0;
}
