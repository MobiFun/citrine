/******************************************************************************/
/*          TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION            */
/*                                                                            */
/* Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only    */
/* Unauthorized reproduction and/or distribution is strictly prohibited.  This*/
/* product  is  protected  under  copyright  law  and  trade  secret law as an*/
/* unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All*/
/* rights reserved.                                                           */
/*                                                                            */
/*                                                                            */
/* Filename          : spi_drv.c                                              */
/*                                                                            */
/* Description       : Functions to drive the SPI module.                     */
/*                     The Serial Port Interface is a bidirectional 3 lines   */
/*                     interface dedicated to the transfer of data to and     */
/*                     from up to 5 external devices offering a 3 lines       */
/*                     serial interface.				      */
/*		       In this project, it is only used to connect the TI     */
/*		       Analog BaseBand (ABB).				      */
/*		       It is assumed that the ABB is connected as the SPI     */
/*                     device 0.					      */
/*									      */
/*                     This interface is specified to be compatible with      */
/*                     the UMA1018M Philips, the FUJITSU MB15F02, the 	      */
/*                     SIEMENS PMB2306T synthesizers and the TI ABB.	      */
/*									      */
/*                     This serial port is based on a looped shift-register   */
/*                     thus allowing both transmit (PISO) and receive (SIPO)  */
/*                     modes.						      */
/*									      */
/*                                                                            */
/* Author            : Pascal PUEL                                            */
/*                                                                            */
/* Version number   : 1.45                                                    */
/*                                                                            */
/* Date and time    : 07/01/03                                                */
/*                                                                            */
/* Previous delta   : Rework                                                  */
/*                                                                            */
/******************************************************************************/

#include "spi_drv.h"



/*-----------------------------------------------------------------------*/
/* SPI_InitDev()                                                         */
/*                                                                       */
/* This function initializes the SPI registers for an external device    */
/* connected to the serial port.                                         */
/*                                                                       */
/*-----------------------------------------------------------------------*/    
void SPI_InitDev(T_SPI_DEV *Device)
{
  unsigned short Param,Gate,Shiftval;
  unsigned short Mask = 0x7bde;

  /* Clock enable, mask ITs and pre scale setting */
  * (volatile SYS_UWORD16 *) SPI_REG_SET1 = (SPI_CLK_ON | Device->PrescVal | SPI_IT_MASK_0 | SPI_IT_MASK_1);

  /* Building the parameter for REG_SET2 initialization */
  Shiftval = Device->DevId >> 7;
  Param = (Device->ClkEdge | Device->TspEnLevel | Device->TspEnForm)<<Shiftval ;
  Gate = Mask<<Shiftval;   
  
  * (volatile SYS_UWORD16 *) SPI_REG_SET2 = (* (volatile SYS_UWORD16 *) SPI_REG_SET2 & Gate) | Param;


  /* Configuring CTRL_REG : this writting erases the previous one */
  * (volatile SYS_UWORD16 *) SPI_REG_CTRL = (Device->DataTrLength | Device->DevId);

  /* Stop the SPI clock */
  #ifdef SPI_CLK_LOW_POWER    
    SPI_CLK_DISABLE
  #endif
}
