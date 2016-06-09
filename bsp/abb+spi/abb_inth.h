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
/* Filename          : abb_inth.h                                             */
/*                                                                            */
/* Description       : Functions to manage the ABB device interrupt.          */
/*                     The Serial Port Interface is used to connect the TI    */
/*		       Analog BaseBand (ABB).		      	              */
/*	               It is assumed that the ABB is connected as the SPI     */
/*                     device 0, and ABB interrupt is mapped as external IT.  */
/*									      */
/* Author            : Pascal PUEL                                            */
/*                                                                            */
/* Version number   : 1.0                                                     */
/*                                                                            */
/* Date and time    : Jan 2003                                                */
/*                                                                            */
/* Previous delta   : Creation                                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* 17/12/03                                                                   */
/* The original abb_inth.h has been splitted between the actual abb_inth.h    */
/* located in drv_apps directory and abb_inth_core.h located in drv_core      */
/* directory.                                                                 */
/* abb_core_inth.h must be included in abb_inth.h.                            */
/*                                                                            */
/******************************************************************************/

#ifndef __ABB_INTH_H__
#define __ABB_INTH_H__

  #include "abb_core_inth.h" 

  #ifndef _WINDOWS

    /****************************************************************/
    /* Power ON/OFF key definition.                                 */
    /****************************************************************/
    // If <ON/OFF> key is pressed more than 20 TDMAs, it's a Hook-ON
    #define SHORT_OFF_KEY_PRESSED  (20)
  
    // If <ON/OFF> key is pressed more than 160 TDMAs, it's a Hook-ON AND then Power-OFF
    #define OFF_LOOP_COUNT   (8)
    #define LONG_OFF_KEY_PRESSED   (OFF_LOOP_COUNT * SHORT_OFF_KEY_PRESSED)
  
  #endif   // _WINDOWS
  
  // PROTOTYPES
  void spi_abb_read_int_reg_callback(SYS_UWORD16 *read_value);

#endif  // __ABB_INTH_H__
