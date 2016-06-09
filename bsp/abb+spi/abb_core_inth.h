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
/* Filename          : abb_core_inth.h                                        */
/*                                                                            */
/* Description       : Functions to manage the ABB device interrupt.          */
/*                     The Serial Port Interface is used to connect the TI    */
/*		       Analog BaseBand (ABB).                                 */
/*	               It is assumed that the ABB is connected as the SPI     */
/*                     device 0, and ABB interrupt is mapped as external IT.  */
/*				                                              */
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
/*                                                                            */
/******************************************************************************/

#ifndef __ABB_CORE_INTH_H__
#define __ABB_CORE_INTH_H__

  #include "../../include/config.h"
  #include "../../include/sys_types.h"

  #ifndef _WINDOWS

    // Structure definition for ADC READING RESULT REPORTS
    // Define the maximum number of measures performed by the MADC module
    #if ((ANALOG == 1) || (ANALOG == 3))
      #define MADC_NUMBER_OF_MEAS  (9)
    #endif
    #if (ANALOG == 2)
      #define MADC_NUMBER_OF_MEAS  (8)
    #endif
  
    typedef struct
    {
      SYS_UWORD16 adc_result[MADC_NUMBER_OF_MEAS];
    } T_CST_ADC_RESULT;
  
  #else   // _WINDOWS
  
    #define MADC_NUMBER_OF_MEAS  (8)

  #endif   // _WINDOWS


  // PROTOTYPES
  #ifndef _WINDOWS
    void Create_ABB_HISR(void);
    SYS_BOOL Activate_ABB_HISR(void);
  #endif   // _WINDOWS
  
  void EXT_HisrEntry(void);


#endif  // __ABB_CORE_INTH_H__
