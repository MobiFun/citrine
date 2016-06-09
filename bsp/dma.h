/******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename       	: dma.h

   Description    	: DMA 

   Project        	: drivers

   Author         	: pmonteil@tif.ti.com  Patrice Monteil.

   Version number	: 1.6

   Date and time	: 01/30/01 10:22:23

   Previous delta 	: 12/08/00 11:22:15

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/RELEASE_GPRS/drivers1/common/SCCS/s.dma.h

   Sccs Id  (SID)       : '@(#) dma.h 1.6 01/30/01 10:22:23 '

 
*****************************************************************************/

#include "../include/config.h"
 
/**** DMA configuration register ****/

#if (CHIPSET == 12)

  #define DMA_GCR   (MEM_DMA_ADDR + 0x0400)
  #define DMA_CAR   (MEM_DMA_ADDR + 0x0404)
  #define DMA_SCR   (MEM_DMA_ADDR + 0x0406)
  #define DMA_AR    (MEM_DMA_ADDR + 0x040A)

  #define DMA_CONFIG { \
      * (volatile unsigned short *) DMA_GCR = 0x0000; /* Force Autogating off */ \
      * (volatile unsigned short *) DMA_CAR |= 0x0001; /* Channel 0 allocated to DSP */ \
      * (volatile unsigned short *) DMA_SCR &= ~0x0001; /* Channel 0 is not secure */ \
      * (volatile unsigned short *) DMA_AR = 0x001C; /* Reset value */ \
    }

#else

// CONTROLLER_CONFIG register
//---------------------------
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  #define DMA_CONFIG_ADDR	MEM_DMA_ADDR
#else
  #define DMA_CONFIG_ADDR		(MEM_DMA_ADDR + 0x20)
#endif
#define DMA_CONFIG_BURST   	0x1c	/* length of burst */

// ALLOC_CONFIG register
//---------------------------
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  #define DMA_ALLOC_CONFIG_ADDR (MEM_DMA_ADDR + 0x02)
#endif
#define DMA_CONFIG_ALLOC1   	0x01	/* allocation for channel 1 */
#define DMA_CONFIG_ALLOC2   	0x02	/* allocation for channel 2 */

// DMA Channel 1 configuration
//---------------------------

// DMA1_RAD register
//---------------------------
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  #define DMA1_RAD_ADDR		(MEM_DMA_ADDR + 0x10)
#else
  #define DMA1_RAD_ADDR		MEM_DMA_ADDR
#endif
#define DMA_RHEA_ADDR   	0x07ff	/* rhea start address */
#define DMA_RHEA_CS	   	0xf800	/* rhea chip select */

// DMA1_RDPTH register
//---------------------------
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  #define DMA1_RDPTH_ADDR	(MEM_DMA_ADDR + 0x12)
#else
  #define DMA1_RDPTH_ADDR		(MEM_DMA_ADDR + 0x02)
#endif
#define DMA_RHEA_LENGTH   	0x07ff	/* rhea buffer length */

// DMA1_AAD register
//---------------------------
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  #define DMA1_AAD_ADDR		(MEM_DMA_ADDR + 0x14)
#else
  #define DMA1_AAD_ADDR		(MEM_DMA_ADDR + 0x04)
#endif
#define DMA_API_ADDR	   	0x0fff	/* API start address */

// DMA1_ALGTH register
//---------------------------
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  #define DMA1_ALGTH_ADDR	(MEM_DMA_ADDR + 0x16)
#else
  #define DMA1_ALGTH_ADDR		(MEM_DMA_ADDR + 0x06)
#endif
#define DMA_API_LENGTH	   	0x0fff	/* API page length */

// DMA1_CTRL register
//---------------------------
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  #define DMA1_CTRL_ADDR	(MEM_DMA_ADDR + 0x18)
#else
  #define DMA1_CTRL_ADDR		(MEM_DMA_ADDR + 0x08)
#endif
#define DMA_CTRL_ENABLE		0x0001	/* DMA enable */
#define DMA_CTRL_IDLE		0x0002	/* idle */
#define DMA_CTRL_ONE_SHOT	0x0004 
#define DMA_CTRL_FIFO_MODE	0x0008 
#define DMA_CTRL_CUR_PAGE	0x0010	/* current page # */
#define DMA_CTRL_MAS		0x0020 
#define DMA_CTRL_START		0x0040	/* DMA start */
#define DMA_CTRL_IRQ_MODE	0x0080	 
#define DMA_CTRL_IRQ_STATE	0x0100 
#define DMA_CTRL_RHEA_ABORT	0x0200 
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  #define DMA_CTRL_PRIORITY     0x1800  /* Number of additional reading on the bus */
#endif

// DMA1_CUR_OFFSET_API register
//---------------------------
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  #define DMA1_OFFSET_ADDR	(MEM_DMA_ADDR + 0x1A)
#else
  #define DMA1_OFFSET_ADDR	(MEM_DMA_ADDR + 0x0A)
#endif

 
// DMA Channel 2 configuration
//---------------------------
 
#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  #define DMA2_RAD_ADDR         (MEM_DMA_ADDR + 0x20)
  #define DMA2_RDPTH_ADDR       (MEM_DMA_ADDR + 0x22)
  #define DMA2_AAD_ADDR         (MEM_DMA_ADDR + 0x24)
  #define DMA2_ALGTH_ADDR       (MEM_DMA_ADDR + 0x26)
  #define DMA2_CTRL_ADDR        (MEM_DMA_ADDR + 0x28)
  #define DMA2_OFFSET_ADDR      (MEM_DMA_ADDR + 0x2A)
#else
  #define DMA2_RAD_ADDR         (MEM_DMA_ADDR + 0x10)
  #define DMA2_RDPTH_ADDR		(MEM_DMA_ADDR + 0x12)
  #define DMA2_AAD_ADDR         (MEM_DMA_ADDR + 0x14)
  #define DMA2_ALGTH_ADDR		(MEM_DMA_ADDR + 0x16)
  #define DMA2_CTRL_ADDR        (MEM_DMA_ADDR + 0x18)
  #define DMA2_OFFSET_ADDR      (MEM_DMA_ADDR + 0x1A)
#endif

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  // DMA Channel 3 configuration
  //---------------------------

  #define DMA3_RAD_ADDR     (MEM_DMA_ADDR + 0x30)
  #define DMA3_RDPTH_ADDR   (MEM_DMA_ADDR + 0x32)
  #define DMA3_AAD_ADDR     (MEM_DMA_ADDR + 0x34)
  #define DMA3_ALGTH_ADDR   (MEM_DMA_ADDR + 0x36)
  #define DMA3_CTRL_ADDR    (MEM_DMA_ADDR + 0x38)
  #define DMA3_OFFSET_ADDR  (MEM_DMA_ADDR + 0x3A)

  // DMA Channel 4 configuration
  //---------------------------

  #define DMA4_RAD_ADDR     (MEM_DMA_ADDR + 0x40)
  #define DMA4_RDPTH_ADDR   (MEM_DMA_ADDR + 0x42)
  #define DMA4_AAD_ADDR     (MEM_DMA_ADDR + 0x44)
  #define DMA4_ALGTH_ADDR   (MEM_DMA_ADDR + 0x46)
  #define DMA4_CTRL_ADDR    (MEM_DMA_ADDR + 0x48)
  #define DMA4_OFFSET_ADDR  (MEM_DMA_ADDR + 0x4A)
#endif

/*---------------------------------------------------------------/
/* DMA_ALLOCDMA()   						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : alloc DMA channel 			*/
/*--------------------------------------------------------------*/

#if ((CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
  // WARNING :
  // Only the first two channels can be configured and the last two channels are forced to be controlled by the ARM
  #define DMA_ALLOCDMA(channel0, channel1, dma_burst,priority) { \
    * (volatile unsigned short *) DMA_CONFIG_ADDR = (dma_burst << 2) | (priority << 5); \
    * (volatile unsigned short *) DMA_ALLOC_CONFIG_ADDR = channel0 | (channel1 << 1) | 0x000C; \
    }
#else
  #define DMA_ALLOCDMA(channel0, channel1,dma_burst,priority) (* (volatile unsigned short *) DMA_CONFIG_ADDR = channel0 | channel1 << 1 | dma_burst << 2 | priority << 5)
#endif

#endif /* CHIPSET != 12 */

