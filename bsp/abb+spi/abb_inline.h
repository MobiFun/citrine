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
/* Filename          : abb_inline.h                                           */
/*                                                                            */
/* Description       : inline functions to drive the ABB device.              */
/*                     The Serial Port Interface is used to connect the TI    */
/*		       Analog BaseBand (ABB).				      */
/*		       It is assumed that the ABB is connected as the SPI     */
/*                     device 0.                                              */
/*									      */
/* Author            : Pascal PUEL                                            */
/*                                                                            */
/* Version number   : 1.0                                                     */
/*                                                                            */
/* Date and time    : Dec 2002                                                */
/*                                                                            */
/* Previous delta   : Creation                                                */
/*                                                                            */
/******************************************************************************/

#ifndef __ABB_INLINE_H__
#define __ABB_INLINE_H__

#include "../../include/config.h"
#include "../../include/sys_types.h"

#include "spi_drv.h"

// MACROS
#define ABB_WRITE_REG(reg, data) { \
  SPI_WRITE_TX_MSB((data << 6) | reg) \
  SPI_START_WRITE }

#define ABB_READ_REG(reg) { \
  SPI_WRITE_TX_MSB(reg | 1) \
  SPI_START_READ }


#define ABB_SET_PAGE(page) ABB_WRITE_REG(PAGEREG, page)

#define SEVEN_CYCLES_13M_NS 539

// INLINE FUNCTIONS
/*-----------------------------------------------------------------------*/
/* ABB_SetPage()                                                         */
/*                                                                       */
/* This function sets the right page in the ABB register PAGEREG.        */
/*                                                                       */
/*-----------------------------------------------------------------------*/
static inline void ABB_SetPage(SYS_UWORD16 page)
{
  volatile SYS_UWORD16 status;

  ABB_SET_PAGE(page);
  while(((status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS) & WE_ST) == 0);

  // if IBIC is already processing another request (from the BSP) 
  // the USP request is delayed by 3 clock cycles
  // which gives a total of 7 clock cycles ( = 539 ns at 13 MHz) in the worst case
  wait_ARM_cycles(convert_nanosec_to_cycles(SEVEN_CYCLES_13M_NS));
}


/*-----------------------------------------------------------------------*/
/* ABB_WriteRegister()                                                   */
/*                                                                       */
/* This function writes "data" in the ABB register "abb_reg".            */
/*                                                                       */
/*-----------------------------------------------------------------------*/
static inline void ABB_WriteRegister(SYS_UWORD16 abb_reg, SYS_UWORD16 data)
{
  volatile SYS_UWORD16 status;

  ABB_WRITE_REG(abb_reg, data);
  while(((status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS) & WE_ST) == 0);

  // if IBIC is already processing another request (from the BSP) 
  // the USP request is delayed by 3 clock cycles
  // which gives a total of 7 clock cycles ( = 539 ns at 13 MHz) in the worst case
  wait_ARM_cycles(convert_nanosec_to_cycles(SEVEN_CYCLES_13M_NS));

}


/*-----------------------------------------------------------------------*/
/* ABB_ReadRegister()                                                    */
/*                                                                       */
/* This function reads the ABB register "abb_reg" and returns            */
/* the real register value.                                              */
/*                                                                       */
/*-----------------------------------------------------------------------*/
static inline SYS_UWORD16 ABB_ReadRegister(SYS_UWORD16 abb_reg)
{
  volatile SYS_UWORD16 status;

  // First part of read access to the ABB register
  ABB_READ_REG(abb_reg);
  while(((status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS) & RE_ST) == 0);

  // if IBIC is already processing another request (from the BSP) 
  // the USP request is delayed by 3 clock cycles
  // which gives a total of 7 clock cycles ( = 539 ns at 13 MHz) in the worst case
  wait_ARM_cycles(convert_nanosec_to_cycles(SEVEN_CYCLES_13M_NS));

  // Second part of read access to the ABB register
  ABB_READ_REG(abb_reg);
  while(((status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS) & RE_ST) == 0);

  // if IBIC is already processing another request (from the BSP) 
  // the USP request is delayed by 3 clock cycles
  // which gives a total of 7 clock cycles ( = 539 ns at 13 MHz) in the worst case
  wait_ARM_cycles(convert_nanosec_to_cycles(SEVEN_CYCLES_13M_NS));

  return ((SPI_ReadRX_LSB() >> 6) & 0x3ff);     
}


#endif  // 	__ABB_INLINE_H__
