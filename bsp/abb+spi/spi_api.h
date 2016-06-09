/****************************************************************************/
/*                                                                          */
/* File Name:  spi_api.h                                                    */
/*                                                                          */
/* Purpose:    This file contains data structures and functions prototypes  */
/*             used to send events to the SPI SWE.                          */
/*                                                                          */
/*  Version    0.1                                                          */
/*                                                                          */
/*  Date       Modification                                                 */
/*  ------------------------------------                                    */
/*  20/08/2000 Create                                                       */
/*                                                                          */
/* Author                                                                   */
/*                                                                          */
/* (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/
#ifndef __SPI_API_H_
#define __SPI_API_H_

#include "../../riviera/rv/rv_general.h"

#include "abb.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* the message offset must differ for each SWE in order to have unique msg_id in the system */
#define SPI_MESSAGES_OFFSET      (0x34 << 10)


/* define a first msg id */
#define SPI_MESSAGE_1      (SPI_MESSAGES_OFFSET | 0x0001)


typedef void (*CALLBACK_FUNC_NO_PARAM)(void);
typedef void (*CALLBACK_FUNC_U16)(UINT16 *);



/*****************************************/
/* structures of messages send to SPI	 */
/*****************************************/

typedef struct {
	T_RV_HDR	os_hdr;
	UINT16		page;
	UINT16		address;
	UINT16		data;
} T_SPI_WRITE;


typedef struct {
	T_RV_HDR	os_hdr;
	UINT16		page;
	UINT16		address;
} T_SPI_READ;


typedef struct {
	T_RV_HDR	os_hdr;
	UINT16		channels;
	UINT16		itval;
} T_SPI_ABB_CONF_ADC;


typedef struct {
	T_RV_HDR               os_hdr;
	UINT16                 *Buff;
	CALLBACK_FUNC_NO_PARAM callback_func;
} T_SPI_ABB_READ_ADC;



#ifdef __cplusplus
}
#endif


/* Prototypes */

/******************************************************************************/
/*                                                                            */
/*    Function Name:   spi_abb_write                                          */
/*                                                                            */
/*    Purpose:     This function is used to send to the SPI mailbox a	      */
/*                 WRITE REGISTER rqst msg.                                   */
/*                                                                            */
/*    Input Parameters:							      */
/*                     - page : ABB Page where to read the register.	      */
/*                     - address : address of the ABB register to read.	      */
/*                     - data : data to write in the ABB register	      */
/*                                                                            */
/*    Return :                                                                */
/*             - RVM_NOT_READY : the SPI task is not ready                    */
/*             - RV_MEMORY_ERR : the SPI task has not enough memory           */
/*             - RV_OK : normal processing                                    */
/*                                                                            */
/*    Note:                                                                   */
/*        None.                                                               */
/*                                                                            */
/******************************************************************************/
T_RV_RET spi_abb_write(UINT16 page, UINT16 address, UINT16 data);

/******************************************************************************/
/*                                                                            */
/*    Function Name:   spi_abb_read                                           */
/*                                                                            */
/*    Purpose:     This function is used to send a READ REGISTER rqst msg     */
/*                 to the SPI mailbox.                                        */
/*                                                                            */
/*    Input Parameters:							      */
/*                     - page : ABB Page where to read the register.	      */
/*                     - address : address of the ABB register to read.	      */
/*                     - CallBack : callback function called by the spi task  */
/*                                  at the end of the read process.           */
/*                                                                            */
/*    Return :                                                                */
/*             - RVM_NOT_READY : the SPI task is not ready                    */
/*             - RV_MEMORY_ERR : the SPI task has not enough memory           */
/*             - RV_OK : normal processing                                    */
/*                                                                            */
/*    Note:                                                                   */
/*        None.                                                               */
/*                                                                            */
/******************************************************************************/
T_RV_RET spi_abb_read(UINT16 page, UINT16 address, CALLBACK_FUNC_U16 CallBack);

/******************************************************************************/
/*                                                                            */
/*    Function Name:   spi_abb_conf_ADC                                       */
/*                                                                            */
/*    Purpose:     This function is used to send to the SPI mailbox a rqst msg*/
/*                 for configuring ABB MADC for conversions.                  */
/*                                                                            */
/*    Input Parameters:							      */
/*                     - channels : ABB channels to be converted.   	      */
/*                     - itval : configure the End Of Conversion IT.	      */
/*                                                                            */
/*    Return :                                                                */
/*             - RVM_NOT_READY : the SPI task is not ready                    */
/*             - RV_MEMORY_ERR : the SPI task has not enough memory           */
/*             - RV_OK : normal processing                                    */
/*                                                                            */
/*    Note:                                                                   */
/*        None.                                                               */
/*                                                                            */
/******************************************************************************/
T_RV_RET spi_abb_conf_ADC(UINT16 channels, UINT16 itval);

/******************************************************************************/
/*                                                                            */
/*    Function Name:   spi_abb_read_ADC                                       */
/*                                                                            */
/*    Purpose:     This function is used to send to the SPI mailbox a rqst msg*/
/*                 for reading the ABB ADC results.                           */
/*                                                                            */
/*    Input Parameters:							      */
/*                     - Buff : pointer to the buffer filled with ADC results.*/
/*                     - CallBack : callback function called by the spi task  */
/*                                  at the end of the read process.           */
/*                                                                            */
/*    Return :                                                                */
/*             - RVM_NOT_READY : the SPI task is not ready                    */
/*             - RV_MEMORY_ERR : the SPI task has not enough memory           */
/*             - RV_OK : normal processing                                    */
/*                                                                            */
/*    Note:                                                                   */
/*        None.                                                               */
/*                                                                            */
/******************************************************************************/
T_RV_RET spi_abb_read_ADC(UINT16 *Buff, CALLBACK_FUNC_NO_PARAM CallBack);

#endif /* __SPI_API_H_ */
