/*******************************************************************************
*
*	File Name: spi_api.c
*
*	Bridge functions used to send events to the SPI task 
*
*	(C) Texas Instruments, all rights reserved
*
*	Version number: 0.1		Date: 25-September-2000
*
*	History: 0.1 - Created by Candice Bazanegue
*
*	Author: 
*
*******************************************************************************/

#include "../../riviera/rvf/rvf_api.h"
#include "spi_api.h"
#include "spi_env.h"
#include "spi_process.h"
#include "../../riviera/rvm/rvm_use_id_list.h"



/******************************************************************************/
/*                                                                            */
/*    Function Name:   spi_abb_read                                           */
/*                                                                            */
/******************************************************************************/
T_RV_RET spi_abb_read(UINT16 page, UINT16 address, CALLBACK_FUNC_U16 CallBack)
{
   T_SPI_READ *msgPtr;

   /* check if the driver has been started */
   if (SPI_GBL_INFO_PTR == NULL)
   {
      rvf_send_trace("ABB read not possible. Reason: SPI task not started",51, NULL_PARAM, 
                     RV_TRACE_LEVEL_WARNING, SPI_USE_ID);
      return (RVM_NOT_READY);
   }

   if(SPI_GBL_INFO_PTR->SpiTaskReady == FALSE)
   {
      rvf_send_trace("ABB read not possible. Reason: SPI Task not ready",49, NULL_PARAM, 
                     RV_TRACE_LEVEL_WARNING, SPI_USE_ID);
      return (RVM_NOT_READY);
   }

   rvf_send_trace("SPI_ABB_READ",12, NULL_PARAM, 
                  RV_TRACE_LEVEL_WARNING, SPI_USE_ID);

   if (rvf_get_buf (SPI_GBL_INFO_PTR->prim_id, sizeof (T_SPI_READ),(void **) &msgPtr) == RVF_RED)
   {
      rvf_send_trace ("SPI ERROR: ABB read not possible. Reason: Not enough memory",
                      59,
                      NULL_PARAM,
                      RV_TRACE_LEVEL_ERROR,
                      SPI_USE_ID);

      return (RV_MEMORY_ERR);
   }

   (msgPtr->os_hdr).msg_id        = SPI_ABB_READ_EVT;
   (msgPtr->os_hdr).dest_addr_id  = SPI_GBL_INFO_PTR->addr_id;
   (msgPtr->os_hdr).callback_func = (CALLBACK_FUNC) CallBack;
   msgPtr->page = page;
   msgPtr->address = address;

   rvf_send_msg (SPI_GBL_INFO_PTR->addr_id,
                 msgPtr);

   return (RV_OK);
}




/******************************************************************************/
/*                                                                            */
/*    Function Name:   spi_abb_write                                          */
/*                                                                            */
/******************************************************************************/
T_RV_RET spi_abb_write(UINT16 page, UINT16 address, UINT16 data)
{
   T_SPI_WRITE *msgPtr;

   /* check if the driver has been started */
   if (SPI_GBL_INFO_PTR == NULL)
   {
      rvf_send_trace("ABB write not possible. Reason: SPI task not started",52, NULL_PARAM, 
                     RV_TRACE_LEVEL_WARNING, SPI_USE_ID);
      return (RVM_NOT_READY);
   }

   if(SPI_GBL_INFO_PTR->SpiTaskReady == FALSE)
   {
      rvf_send_trace("ABB write not possible. Reason: SPI Task not ready",50, NULL_PARAM, 
                     RV_TRACE_LEVEL_WARNING, SPI_USE_ID);
      return (RVM_NOT_READY);
   }

   if (rvf_get_buf (SPI_GBL_INFO_PTR->prim_id, sizeof (T_SPI_WRITE),(void **) &msgPtr) == RVF_RED)
   {
      rvf_send_trace ("SPI ERROR: ABB write not possible. Reason: Not enough memory",
                      60,
                      NULL_PARAM,
                      RV_TRACE_LEVEL_ERROR,
                      SPI_USE_ID);

      return (RV_MEMORY_ERR);
   }

   (msgPtr->os_hdr).msg_id        = SPI_ABB_WRITE_EVT;
   (msgPtr->os_hdr).dest_addr_id  = SPI_GBL_INFO_PTR->addr_id;
   (msgPtr->os_hdr).callback_func = NULL;
   msgPtr->page = page;
   msgPtr->address = address;
   msgPtr->data = data;

   rvf_send_msg (SPI_GBL_INFO_PTR->addr_id, msgPtr);

   return (RV_OK);
}




/******************************************************************************/
/*                                                                            */
/*    Function Name:   spi_abb_conf_ADC                                       */
/*                                                                            */
/******************************************************************************/
T_RV_RET spi_abb_conf_ADC(UINT16 channels, UINT16 itval)
{
   T_SPI_ABB_CONF_ADC *msgPtr;

   /* check if the driver has been started */
   if (SPI_GBL_INFO_PTR == NULL)
   {
      rvf_send_trace("ABB conf ADC not possible. Reason: SPI task not started",55, NULL_PARAM, 
                     RV_TRACE_LEVEL_WARNING, SPI_USE_ID);
      return (RVM_NOT_READY);
   }

   if(SPI_GBL_INFO_PTR->SpiTaskReady == FALSE)
   {
      rvf_send_trace("ABB conf ADC not possible. Reason: SPI Task not ready",53, NULL_PARAM, 
                     RV_TRACE_LEVEL_WARNING, SPI_USE_ID);
      return (RVM_NOT_READY);
   }

   rvf_send_trace("SPI_ABB_CONF_ADC",16, NULL_PARAM, 
                  RV_TRACE_LEVEL_WARNING, SPI_USE_ID);

   if (rvf_get_buf (SPI_GBL_INFO_PTR->prim_id, sizeof (T_SPI_ABB_CONF_ADC),(void **) &msgPtr) == RVF_RED)
   {
      rvf_send_trace ("SPI ERROR: ABB conf ADC not possible. Reason: Not enough memory",
                      63,
                      NULL_PARAM,
                      RV_TRACE_LEVEL_ERROR,
                      SPI_USE_ID);

      return (RV_MEMORY_ERR);
   }

   (msgPtr->os_hdr).msg_id        = SPI_ABB_CONF_ADC_EVT;
   (msgPtr->os_hdr).dest_addr_id  = SPI_GBL_INFO_PTR->addr_id;
   (msgPtr->os_hdr).callback_func = NULL;
   msgPtr->channels               = channels;
   msgPtr->itval                  = itval;

   rvf_send_msg (SPI_GBL_INFO_PTR->addr_id, msgPtr);

   return (RV_OK);
}



/******************************************************************************/
/*                                                                            */
/*    Function Name:   spi_abb_read_ADC                                       */
/*                                                                            */
/******************************************************************************/
T_RV_RET spi_abb_read_ADC(UINT16 *Buff, CALLBACK_FUNC_NO_PARAM CallBack)
{
   T_SPI_ABB_READ_ADC *msgPtr;

   /* check if the driver has been started */
   if (SPI_GBL_INFO_PTR == NULL)
   {
      rvf_send_trace("ABB read ADC not possible. Reason: SPI task not started",55, NULL_PARAM, 
                     RV_TRACE_LEVEL_WARNING, SPI_USE_ID);
      return (RVM_NOT_READY);
   }

   if(SPI_GBL_INFO_PTR->SpiTaskReady == FALSE)
   {
      rvf_send_trace("ABB conf ADC not possible. Reason: SPI Task not ready",53, NULL_PARAM, 
                     RV_TRACE_LEVEL_WARNING, SPI_USE_ID);
      return (RVM_NOT_READY);
   }

   rvf_send_trace("SPI_ABB_READ_ADC",16, NULL_PARAM, 
                  RV_TRACE_LEVEL_WARNING, SPI_USE_ID);

   if (rvf_get_buf (SPI_GBL_INFO_PTR->prim_id, sizeof (T_SPI_ABB_READ_ADC),(void **) &msgPtr) == RVF_RED)
   {
      rvf_send_trace ("SPI ERROR: ABB read ADC not possible. Reason: Not enough memory",
                      63,
                      NULL_PARAM,
                      RV_TRACE_LEVEL_ERROR,
                      SPI_USE_ID);

      return (RV_MEMORY_ERR);
   }

   (msgPtr->os_hdr).msg_id        = SPI_ABB_READ_ADC_EVT;
   (msgPtr->os_hdr).dest_addr_id  = SPI_GBL_INFO_PTR->addr_id;
   (msgPtr->os_hdr).callback_func = NULL;
   msgPtr->Buff                   = Buff;
   msgPtr->callback_func          = CallBack;

   rvf_send_msg (SPI_GBL_INFO_PTR->addr_id, msgPtr);

   return (RV_OK);
}
