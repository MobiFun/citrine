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
/* Filename          : spi_drv.h                                              */
/*                                                                            */
/* Description       : SPI registers and bits definitions.		      */
/*                     Functions and macros to drive the SPI module.          */
/*                     The Serial Port Interface is a bidirectional 3 lines   */
/*                     interface dedicated to the transfer of data to and     */
/*                     from up to 5 external devices offering a 3 lines       */
/*                     serial interface.				      */
/*		       In this project, it is only used to connect the TI     */
/*		       Analog BaseBand (ABB).				      */
/*		       It is assumed that the ABB is connected as the SPI     */
/*                     device 0.                                              */
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
/* Version number   : 1.28                                                    */
/*                                                                            */
/* Date and time    : 07/01/03                                                */
/*                                                                            */
/* Previous delta   : Rework                                                  */
/*                                                                            */
/******************************************************************************/

#ifndef __SPI_DRV_H__
#define __SPI_DRV_H__

#include "../../include/config.h"
#include "../../include/sys_types.h"

#include "../mem.h" 

// SPI module registers definition
#define SPI_REG_SET1	(MEM_SPI + 0x00)
#define SPI_REG_SET2	(MEM_SPI + 0x02)
#define SPI_REG_CTRL	(MEM_SPI + 0x04)
#define SPI_REG_STATUS	(MEM_SPI + 0x06)
#define SPI_REG_TX_LSB	(MEM_SPI + 0x08)
#define SPI_REG_TX_MSB	(MEM_SPI + 0x0A)
#define SPI_REG_RX_LSB	(MEM_SPI + 0x0C)
#define SPI_REG_RX_MSB	(MEM_SPI + 0x0E)


// SPI module bits definition of register SPI_REG_SET1
#define SPI_CLK_OFF          0x0000   // default value 
#define SPI_CLK_ON           0x0001
#define SPI_CLOCK_DIV_1      0x0000   // default value
#define SPI_CLOCK_DIV_2      0x0002
#define SPI_CLOCK_DIV_4      0x0004
#define SPI_CLOCK_DIV_8      0x0006
#define SPI_CLOCK_DIV_16     0x0008
#if (CHIPSET == 12)
#define SPI_CLOCK_DIV_32     0x000A
#define SPI_CLOCK_DIV_64     0x000C
#define SPI_CLOCK_DIV_128    0x000E
#endif
#define SPI_IT_MASK_0        0x0010   // default value
#define SPI_IT_DEMASK_0      0x0000
#define SPI_IT_MASK_1        0x0020   // default value   
#define SPI_IT_DEMASK_1      0x0000


// SPI module bits definition of register SPI_REG_SET2
#define SPI_CLK_EDG_FALL     	  0x0000   // default value	for device 0
#define SPI_CLK_EDG_RISE     	  0x0001
#define SPI_CLK_EDG_FALL_1   	  0x0000   // default value	for device 1
#define SPI_CLK_EDG_RISE_1   	  0x0002
#define SPI_CLK_EDG_FALL_2   	  0x0000   // default value	for device 2
#define SPI_CLK_EDG_RISE_2   	  0x0004
#define SPI_CLK_EDG_FALL_3   	  0x0000   // default value	for device 3
#define SPI_CLK_EDG_RISE_3   	  0x0008
#define SPI_CLK_EDG_FALL_4   	  0x0000   // default value for device 4
#define SPI_CLK_EDG_RISE_4   	  0x0010
#define SPI_NTSPEN_NEG_LEV   	  0x0000   // default value	for device 0
#define SPI_NTSPEN_POS_LEV   	  0x0020
#define SPI_NTSPEN_NEG_LEV_1 	  0x0000   // default value	for device 1
#define SPI_NTSPEN_POS_LEV_1 	  0x0040
#define SPI_NTSPEN_NEG_LEV_2 	  0x0000   // default value	for device 2
#define SPI_NTSPEN_POS_LEV_2 	  0x0080
#define SPI_NTSPEN_NEG_LEV_3 	  0x0000   // default value	for device 3
#define SPI_NTSPEN_POS_LEV_3 	  0x0100
#define SPI_NTSPEN_NEG_LEV_4 	  0x0000   // default value for device 4
#define SPI_NTSPEN_POS_LEV_4 	  0x0200
#define SPI_NTSPEN_LEV_TRIG  	  0x0000   // default value	for device 0
#define SPI_NTSPEN_EDG_TRIG  	  0x0400
#define SPI_NTSPEN_LEV_TRIG_1	  0x0000   // default value	for device 1
#define SPI_NTSPEN_EDG_TRIG_1	  0x0800
#define SPI_NTSPEN_LEV_TRIG_2	  0x0000   // default value	for device 2
#define SPI_NTSPEN_EDG_TRIG_2	  0x1000
#define SPI_NTSPEN_LEV_TRIG_3	  0x0000   // default value	for device 3
#define SPI_NTSPEN_EDG_TRIG_3	  0x2000
#define SPI_NTSPEN_LEV_TRIG_4	  0x0000   // default value for device 4
#define SPI_NTSPEN_EDG_TRIG_4	  0x4000


// SPI module bits definition of register SPI_REG_CTRL
#define SPI_RDWR_DEACTIV   0x0000      // default value
#define SPI_RDWR_ACTIV     0x0001
#define SPI_WR_DEACTIV     0x0000      // default value
#define SPI_WR_ACTIV       0x0002
#define SPI_WNB_0          0x0000      // default value  
#define SPI_WNB_1          0x0004  
#define SPI_WNB_2          0x0008  
#define SPI_WNB_3          0x000c  
#define SPI_WNB_4          0x0010  
#define SPI_WNB_5          0x0014  
#define SPI_WNB_6          0x0018  
#define SPI_WNB_7          0x001c  
#define SPI_WNB_8          0x0020  
#define SPI_WNB_9          0x0024  
#define SPI_WNB_10         0x0028  
#define SPI_WNB_11         0x002c  
#define SPI_WNB_12         0x0030  
#define SPI_WNB_13         0x0034  
#define SPI_WNB_14         0x0038  
#define SPI_WNB_15         0x003c  
#define SPI_WNB_16         0x0040  
#define SPI_WNB_17         0x0044  
#define SPI_WNB_18         0x0048  
#define SPI_WNB_19         0x004c  
#define SPI_WNB_20         0x0050  
#define SPI_WNB_21         0x0054  
#define SPI_WNB_22         0x0058  
#define SPI_WNB_23         0x005c  
#define SPI_WNB_24         0x0060  
#define SPI_WNB_25         0x0064  
#define SPI_WNB_26         0x0068  
#define SPI_WNB_27         0x006c  
#define SPI_WNB_28         0x0070  
#define SPI_WNB_29         0x0074
#define SPI_WNB_30         0x0078  
#define SPI_WNB_31         0x007c 


// SPI possible device IDs
#define SPI_DEV0   0x0000 
#define SPI_DEV1   0x0080   
#define SPI_DEV2   0x0100   
#define SPI_DEV3   0x0180   
#define SPI_DEV4   0x0200   

// ABB should be mapped as device 0
#define ABB		SPI_DEV0


// SPI module bits definition of register SPI_REG_STATUS
#define RE_ST   0x0001   // bit 0
#define WE_ST   0x0002   // bit 1


/* The ARM emulator requires the spi clock always ON to be able to access */
/* spi registers through a window.*/ 
/* But it's better to stop the SPI clock in the GSM application to reduce the power consumption. */
/* Validate the next line to reduce power consumption */
//#define SPI_CLK_LOW_POWER



// STRUCTURES
typedef struct
{
    SYS_UWORD16 PrescVal;
    SYS_UWORD16 DataTrLength;
    SYS_UWORD16 DevAddLength;
    SYS_UWORD16 DevId;
    SYS_UWORD16 ClkEdge; 
    SYS_UWORD16 TspEnLevel; 
    SYS_UWORD16 TspEnForm; 
}T_SPI_DEV;      // T_SPI_DEV is used to define an SPI device


// MACROS
#define SPI_WRITE_TX_LSB(TxLsb) { \
* (volatile SYS_UWORD16 *) SPI_REG_TX_LSB = TxLsb;  }

#define SPI_WRITE_TX_MSB(TxMsb) { \
* (volatile SYS_UWORD16 *) SPI_REG_TX_MSB = TxMsb;  }

#define SPI_START_WRITE {* (volatile SYS_UWORD16 *) SPI_REG_CTRL |= SPI_WR_ACTIV; }

#define SPI_START_READ {* (volatile SYS_UWORD16 *) SPI_REG_CTRL |= SPI_RDWR_ACTIV; }

#define SPI_CLK_DISABLE	{ \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 &= ~SPI_CLK_ON;  }

#define SPI_CLK_ENABLE	{ \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 |= SPI_CLK_ON;  }

#define SPI_MaskIT_WR { \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 |= SPI_IT_MASK_0;  }

#define SPI_MaskIT_RD { \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 |= SPI_IT_MASK_1;  }

#define SPI_Mask_All_IT { \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 |= (SPI_IT_MASK_0 | SPI_IT_MASK_1);  }

#define SPI_UnmaskIT_WR { \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 &= ~SPI_IT_MASK_0;  }

#define SPI_UnmaskIT_RD { \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 &= ~SPI_IT_MASK_1;  }

#define SPI_Unmask_All_IT { \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 &= ~(SPI_IT_MASK_0 | SPI_IT_MASK_1);  }

#define SPI_Ready_for_WR { \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 |= (SPI_CLK_ON | SPI_IT_MASK_0); }

#define SPI_Ready_for_RD { \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 |= (SPI_CLK_ON | SPI_IT_MASK_1); }

#define SPI_Ready_for_RDWR { \
* (volatile SYS_UWORD16 *) SPI_REG_SET1 |= (SPI_CLK_ON | SPI_IT_MASK_0 | SPI_IT_MASK_1); }   



// INLINE FUNCTIONS
/*-----------------------------------------------------------------------*/
/* SPI_ReadRX_LSB()                                                      */
/*                                                                       */
/* This function returns the value of SPI_REG_RX_LSB register            */
/*                                                                       */
/*-----------------------------------------------------------------------*/
static inline SYS_UWORD16 SPI_ReadRX_LSB(void)
{
  return * (volatile SYS_UWORD16 *) SPI_REG_RX_LSB;
}


/*-----------------------------------------------------------------------*/
/* SPI_ReadRX_MSB()                                                      */
/*                                                                       */
/* This function returns the value of SPI_REG_RX_MSB register            */
/*                                                                       */
/*-----------------------------------------------------------------------*/
static inline SYS_UWORD16 SPI_ReadRX_MSB(void)
{
  return * (volatile SYS_UWORD16 *) SPI_REG_RX_MSB;
}



/*-----------------------------------------------------------------------*/
/* SPI_ReadStatus()                                                      */
/*                                                                       */
/* This function returns the value of SPI_REG_STATUS register            */
/*                                                                       */
/*-----------------------------------------------------------------------*/
static inline SYS_UWORD16 SPI_ReadStatus(void)
{
  return * (volatile SYS_UWORD16 *) SPI_REG_STATUS;
}



// PROTOTYPES
void SPI_InitDev(T_SPI_DEV *Device);

#endif   //  __SPI_DRV_H__
