/*****************************************************************************/
/*                                                                           */
/*  Name       spi_process.h                                                 */
/*                                                                           */
/*  Function    this file contains the spi_process function prototype,       */
/*		used to handle messages received in the SPI task mailbox.    */
/*                                                                           */
/*  Version    0.1                                                           */
/*                                                                           */
/*  Date       Modification                                                  */
/*  ------------------------------------                                     */
/*                                                                           */
/* Author      Candice Bazanegue                                             */
/*                                                                           */
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved */
/*****************************************************************************/

#ifndef __SPI_PROCESS_H__
#define __SPI_PROCESS_H__


#define SPI_ABB_READ_EVT            1
#define SPI_ABB_WRITE_EVT           2
#define SPI_ABB_CONF_ADC_EVT        3
#define SPI_ABB_READ_ADC_EVT        4
#define ABB_EXT_IRQ_EVT             5

/* Prototypes */

/*******************************************************************************
** Function         spi_process
**
** Description     It is called by the spi task core to handle the access to
**                 the ABB through the SPI in a non-preemptive way.        
**
*******************************************************************************/
UINT8 spi_process(T_RV_HDR * msg_ptr);

#endif /* __SPI_PROCESS_H__ */
