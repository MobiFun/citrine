/******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename       	: niq.c

   Description    	: Nucleus IQ initializations

   Project        	: Drivers

   Author         	: proussel@ti.com  Patrick Roussel.

   Version number	: 1.8

   Date and time	: 01/30/01 10:22:21

   Previous delta 	: 12/19/00 14:22:24

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/RELEASE_GPRS/drivers1/common/SCCS/s.niq.c

   Sccs Id  (SID)       : '@(#) niq.c 1.8 01/30/01 10:22:21 '
  *******************************************************************************/


#include "../include/config.h"
#include "../include/sys_types.h"

#include  "mem.h"
#include  "rhea_arm.h"
#include  "inth.h"
#include  "iq.h"

extern SYS_FUNC irqHandlers[IQ_NUM_INT];

/*--------------------------------------------------------------*/
/*  	IQ_Prty[IQ_NUM_INT]					*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : IRQ priorities 0xFF means interrupt is masked*/
/*--------------------------------------------------------------*/

#if (CHIPSET == 4)
  unsigned char IQ_Prty[IQ_NUM_INT] = 
  {
  //  AIRQ0  AIRQ1  AIRQ2  AIRQ3  AIRQ4  AIRQ5  AIRQ6 AIRQ7
      0x01,  0x02,  0x02,  0xFF,  0x00,  0x04,  0x07, 0x02, 

  //  AIRQ8  AIRQ9  AIRQ10 AIRQ11 AIRQ12 AIRQ13 DMA   LEAD 
      0x01,  0x03,  0x03,  0x00,  0x08,  0x05,  0x06, 0x03,

  //  AIRQ16 AIRQ17 AIRQ18 AIRQ19
      0x07,  0xFF,  0x02,  0x03

  };
#elif ((CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 9))
  unsigned char IQ_Prty[IQ_NUM_INT] = 
  {
  //  AIRQ0  AIRQ1  AIRQ2  AIRQ3  AIRQ4  AIRQ5  AIRQ6  AIRQ7
      0x01,  0x02,  0x02,  0xFF,  0x00,  0x04,  0x07,  0x02, 

  //  AIRQ8  AIRQ9  AIRQ10 AIRQ11 AIRQ12 AIRQ13 DMA    LEAD 
      0x01,  0x03,  0x03,  0x00,  0x08,  0x05,  0x06,  0x03,

  //  AIRQ16 AIRQ17 AIRQ18 AIRQ19 AIRQ20 AIRQ21 AIRQ22 AIRQ23
      0x07,  0xFF,  0x02,  0x03,  0x0F,  0xFF,  0xFF,  0xFF,

  //  AIRQ24
      0xFF

  };
#elif (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11)
  unsigned char IQ_Prty[IQ_NUM_INT] = 
  {
  //  AIRQ0  AIRQ1  AIRQ2  AIRQ3  AIRQ4  AIRQ5  AIRQ6 AIRQ7
      0x01,  0x02,  0x02,  0xFF,  0x00,  0x04,  0x07, 0x02, 

  //  AIRQ8  AIRQ9  AIRQ10 AIRQ11 AIRQ12 AIRQ13 DMA   LEAD 
      0x01,  0x03,  0x03,  0x00,  0x08,  0x05,  0x06, 0x03,

  //  AIRQ16 AIRQ17 AIRQ18 AIRQ19 AIRQ20
      0x07,  0xFF,  0x02,  0x03,  0xFF

  };
#elif (CHIPSET == 12)
  unsigned char IQ_Prty[IQ_NUM_INT] = 
  {
  //  AIRQ0  AIRQ1  AIRQ2  AIRQ3   AIRQ4   AIRQ5   AIRQ6  AIRQ7
      0x01,  0x02,  0x02,  0xFF,   0x00,   0x04,   0x07, 0x02, 

  //  AIRQ8  AIRQ9  AIRQ10  AIRQ11  AIRQ12 AIRQ13  DMA   LEAD 
      0x01,  0x03,  0x03,  0x00,   0x08,   0x05,   0x06, 0x03,

  //  AIRQ16 AIRQ17 AIRQ18 AIRQ19 AIRQ20 AIRQ21 AIRQ22 AIRQ23
      0x07,  0xFF,  0x02,  0x03,  0xFF,  0xFF,  0xFF,  0xFF,

  //  AIRQ24 AIRQ25 AIRQ26 AIRQ27 AIRQ28 AIRQ29 AIRQ30 AIRQ31
      0xFF,  0xFF,  0xFF,  0xFF,  0x02,  0xFF,  0xFF,  0xFF

  };
#else
  unsigned char IQ_Prty[IQ_NUM_INT] = 
  {
  //  AIRQ0 AIRQ1 AIRQ2  AIRQ3  AIRQ4  AIRQ5  AIRQ6 AIRQ7
      0x01, 0x02, 0x02,  0xFF,  0x00,  0x04,  0x07, 0x02, 
      
  //  AIRQ8 AIRQ9 AIRQ10 AIRQ11 AIRQ12 AIRQ13 DMA   LEAD 
      0x01, 0x03, 0x03,  0x00,  0x08,  0x05,  0x06, 0x03 

  };
#endif

/*--------------------------------------------------------------*/
/*  	IQ_LEVEL[IQ_NUM_INT]					*/
/*--------------------------------------------------------------*/
/* Parameters : none			                    			*/
/* Return     :	none						                    */
/* Functionality : IRQ sensitivity                              */
/* 0: falling edge 1: low level (default)                       */
/*--------------------------------------------------------------*/

#if (CHIPSET == 4)
  unsigned char IQ_LEVEL[IQ_NUM_INT] = 
  {
  //  AIRQ0  AIRQ1  AIRQ2  AIRQ3  AIRQ4  AIRQ5  AIRQ6 AIRQ7
      0x01,	 0x01,  0x01,  0x01,  0x01,  0x01,  0x01, 0x00, 

  //  AIRQ8  AIRQ9  AIRQ10 AIRQ11 AIRQ12 AIRQ13 DMA   LEAD 
      0x00,  0x01,  0x00,  0x01,  0x01,  0x01,  0x00, 0x01,

  //  AIRQ16 AIRQ17 AIRQ18 AIRQ19
      0x01,  0x01,  0x00,  0x00

  };
#elif ((CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 9))
  unsigned char IQ_LEVEL[IQ_NUM_INT] = 
  {
  //  AIRQ0  AIRQ1  AIRQ2  AIRQ3  AIRQ4  AIRQ5  AIRQ6  AIRQ7
      0x01,	 0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x00, 

  //  AIRQ8  AIRQ9  AIRQ10 AIRQ11 AIRQ12 AIRQ13 DMA    LEAD 
      0x00,  0x01,  0x00,  0x01,  0x01,  0x01,  0x00,  0x01,

  //  AIRQ16 AIRQ17 AIRQ18 AIRQ19 AIRQ20 AIRQ21 AIRQ22 AIRQ23
      0x01,  0x01,  0x00,  0x00,  0x01,  0x00,  0x00,  0x00,

  //  AIRQ24
      0x00

  };
#elif (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11)
  unsigned char IQ_LEVEL[IQ_NUM_INT] = 
  {
  //  AIRQ0  AIRQ1  AIRQ2  AIRQ3  AIRQ4  AIRQ5  AIRQ6 AIRQ7
      0x01,	 0x01,  0x01,  0x01,  0x01,  0x01,  0x01, 0x00, 

  //  AIRQ8  AIRQ9  AIRQ10 AIRQ11 AIRQ12 AIRQ13 DMA   LEAD 
      0x00,  0x01,  0x00,  0x01,  0x01,  0x01,  0x00, 0x01,

  //  AIRQ16 AIRQ17 AIRQ18 AIRQ19 AIRQ20
      0x01,  0x01,  0x00,  0x00,  0x00

  };
#elif (CHIPSET == 12)
  unsigned char IQ_LEVEL[IQ_NUM_INT] = 
  {
  //  AIRQ0  AIRQ1  AIRQ2  AIRQ3   AIRQ4   AIRQ5   AIRQ6  AIRQ7
      0x01,	 0x01,  0x01,  0x01,  0x01,  0x01,  0x01, 0x00, 

  //  AIRQ8  AIRQ9  AIRQ10  AIRQ11  AIRQ12 AIRQ13  DMA   LEAD 
      0x00,  0x01,  0x00,  0x01,  0x01,  0x01,  0x00, 0x01,

  //  AIRQ16 AIRQ17 AIRQ18 AIRQ19 AIRQ20 AIRQ21 AIRQ22 AIRQ23
      0x01,  0x01,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,

  //  AIRQ24 AIRQ25 AIRQ26 AIRQ27 AIRQ28 AIRQ29 AIRQ30 AIRQ31
      0x01,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00

  };
#else
  unsigned char IQ_LEVEL[IQ_NUM_INT] = 
  {
  //  AIRQ0 AIRQ1 AIRQ2  AIRQ3  AIRQ4  AIRQ5  AIRQ6 AIRQ7
      0x01, 0x01, 0x01,  0x01,  0x01,  0x01,  0x01, 0x00, 
      
  //  AIRQ8 AIRQ9 AIRQ10 AIRQ11 AIRQ12 AIRQ13 DMA   LEAD 
      0x00, 0x01, 0x00,  0x01,  0x01,  0x01,  0x00, 0x01 

  };
#endif


/*--------------------------------------------------------------*/
/*  	IQ_FIQ_nIRQ[IQ_NUM_INT]					                */
/*--------------------------------------------------------------*/
/* Parameters : none			                    			*/
/* Return     :	none						                    */
/* Functionality : IRQ sensitivity                              */
/* 0: falling edge 1: low level (default)                       */
/*--------------------------------------------------------------*/

#if (CHIPSET == 4)
  unsigned char IQ_FIQ_nIRQ[IQ_NUM_INT] = 
  {
  //  AIRQ0     AIRQ1     AIRQ2     AIRQ3     AIRQ4     AIRQ5     AIRQ6     AIRQ7
      INTH_IRQ,	INTH_IRQ, INTH_IRQ, INTH_FIQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, 

  //  AIRQ8     AIRQ9     AIRQ10    AIRQ11    AIRQ12    AIRQ13    DMA       LEAD 
      INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ,

  //  AIRQ16    AIRQ17    AIRQ18    AIRQ19
      INTH_FIQ, INTH_FIQ, INTH_IRQ, INTH_IRQ

  };
#elif ((CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 9))
  unsigned char IQ_FIQ_nIRQ[IQ_NUM_INT] = 
  {
  //  AIRQ0     AIRQ1     AIRQ2     AIRQ3     AIRQ4     AIRQ5     AIRQ6     AIRQ7
      INTH_IRQ,	INTH_IRQ, INTH_IRQ, INTH_FIQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, 

  //  AIRQ8     AIRQ9     AIRQ10    AIRQ11    AIRQ12    AIRQ13    DMA       LEAD 
      INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ,

  //  AIRQ16    AIRQ17    AIRQ18    AIRQ19    AIRQ20    AIRQ21    AIRQ22    AIRQ23
      INTH_FIQ, INTH_FIQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ,

  //  AIRQ24
      INTH_IRQ

  };
#elif (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11)
  unsigned char IQ_FIQ_nIRQ[IQ_NUM_INT] = 
  {
  //  AIRQ0     AIRQ1     AIRQ2     AIRQ3     AIRQ4     AIRQ5     AIRQ6     AIRQ7
      INTH_IRQ,	INTH_IRQ, INTH_IRQ, INTH_FIQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, 

  //  AIRQ8     AIRQ9     AIRQ10    AIRQ11    AIRQ12    AIRQ13    DMA       LEAD 
      INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ,

  //  AIRQ16    AIRQ17    AIRQ18    AIRQ19    AIRQ20
      INTH_FIQ, INTH_FIQ, INTH_IRQ, INTH_IRQ, INTH_IRQ

  };
#elif (CHIPSET == 12)
  unsigned char IQ_FIQ_nIRQ[IQ_NUM_INT] = 
  {
  //  AIRQ0     AIRQ1     AIRQ2     AIRQ3     AIRQ4     AIRQ5     AIRQ6     AIRQ7
      INTH_IRQ,	INTH_IRQ, INTH_IRQ, INTH_FIQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, 

  //  AIRQ8     AIRQ9     AIRQ10    AIRQ11    AIRQ12    AIRQ13    DMA       LEAD 
      INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ,

  //  AIRQ16    AIRQ17    AIRQ18    AIRQ19    AIRQ20    AIRQ21    AIRQ22    AIRQ23
      INTH_FIQ, INTH_FIQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_FIQ,

  //  AIRQ24    AIRQ25    AIRQ26    AIRQ27    AIRQ28    AIRQ29    AIRQ30    AIRQ31
      INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ, INTH_IRQ

  };
#endif

/*--------------------------------------------------------------*/
/*  	IQ_GetBuild						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	Return library build number			*/
/* Functionality : 						*/
/*--------------------------------------------------------------*/

unsigned IQ_GetBuild(void)
{
   return(IQ_BUILD);
}


/*--------------------------------------------------------------*/
/*  	IQ_SetupInterrupts					*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : Set IRQ interrupt levels and unmask		*/
/*--------------------------------------------------------------*/

void IQ_SetupInterrupts(void)
{
   int i;
   char prty;

   /* Setup all interrupts to IRQ with different levels */
   for (i=INTH_TIMER;i<=IQ_NUM_INT;i++ )
   {
      prty = IQ_Prty[i];
      if (prty != 0xFF)
      {
        #if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
          INTH_InitLevel(i, IQ_FIQ_nIRQ[i], prty, IQ_LEVEL[i]);
        #else
          INTH_InitLevel(i, INTH_IRQ, prty, IQ_LEVEL[i]);
        #endif
      }     
   INTH_DISABLEONEIT(i);
   }
}

/*--------------------------------------------------------------*/
/*  	IQ_InitWaitState					*/
/*--------------------------------------------------------------*/
/* Parameters :rom,  ram,  spy,  lcd, jtag			*/
/* Return     :	none						*/
/* Functionality : Init wait states				*/
/*--------------------------------------------------------------*/

void IQ_InitWaitState(unsigned short rom, unsigned short ram, unsigned short spy, unsigned short lcd, unsigned short jtag)
{
   volatile char ws;

   ws = * (volatile SYS_UWORD16 *)  MEM_REG_nCS0;
   ws &= ~WS_MASK; 		
   ws |= rom;
   * (volatile SYS_UWORD16 *) MEM_REG_nCS0  = ws;

   ws = * (volatile SYS_UWORD16 *) MEM_REG_nCS1;
   ws &= ~WS_MASK; 		
   ws |= ram;
   * (volatile SYS_UWORD16 *) MEM_REG_nCS1 = ws;
   
   ws = * (volatile SYS_UWORD16 *) MEM_REG_nCS2;
   ws &= ~WS_MASK; 		
   ws |= spy;
   * (volatile SYS_UWORD16 *) MEM_REG_nCS2  = ws;

   ws = * (volatile SYS_UWORD16 *) MEM_REG_nCS4;
   ws &= ~WS_MASK; 		
   ws |= lcd;
   * (volatile SYS_UWORD16 *) MEM_REG_nCS4  = ws;

#if ((CHIPSET != 4) && (CHIPSET != 7) && (CHIPSET != 8) && (CHIPSET != 10) && (CHIPSET != 11))
   ws = * (volatile SYS_UWORD16 *) MEM_REG_nCS5;
   ws &= ~WS_MASK; 		
   ws |= jtag;
   * (volatile SYS_UWORD16 *) MEM_REG_nCS5  = ws;
#endif

}

void IQ_Unmask(unsigned irqNum)
{
  INTH_ENABLEONEIT(irqNum); 
}

void IQ_Mask(unsigned irqNum)
{
  INTH_DISABLEONEIT(irqNum); 
}

void IQ_MaskAll(void)
{
  INTH_DISABLEALLIT; 
}


/*
 * IQ_GetJtagId
 *
 * JTAG part identifier 
 */
SYS_UWORD16 IQ_GetJtagId(void)
{
   unsigned short v;
   
   v = *( (volatile unsigned short *) (MEM_JTAGID_PART));
   return(v);
}



 /*
 * IQ_GetDeviceVersion
 *
 * Read from CLKM module
 */
SYS_UWORD16 IQ_GetDeviceVersion(void)
{
   SYS_UWORD16 v;

   v = *( (volatile SYS_WORD16 *) (MEM_JTAGID_VER));
   return(v);
}


//// !!!! TO BE REMOVED IN NEXT CONDAT RELEASE >2.55
 /*
 * IQ_GetDeviceVersion
 *
 * Read from CLKM module
 */
SYS_UWORD16 IQ_GetPoleStarVersion(void)
{
   SYS_UWORD16 v;

   v = *( (volatile SYS_UWORD16 *) (MEM_JTAGID_VER));
   return(v);
}



/*
 * IQ_RamBasedLead
 *
 * Returns TRUE if the LEAD has RAM and needs to be downloaded
 *
 */

SYS_BOOL IQ_RamBasedLead(void)
{
   unsigned short id;
   
   id  = IQ_GetJtagId();
   
#if (CHIPSET != 7)
   return ((id == SATURN || id == HERCRAM || id == 0xB2AC) ? 1 : 0); 
#else
   // Calypso G2 rev. A and Samson share the same JTAG ID
   return (0);
#endif
 }

/*
 * IQ_Power
 *
 * Switch-on or off the board
 *
 * Parameters : BOOL power  : 1 to power-on (maintain power)
 *                            0 to power-off
 *
 * See A-Sample board specification
 */
void IQ_Power(SYS_UWORD8 power)
{
// This is only implemented for the A-Sample
}

/*
 * IQ_GetRevision
 *
 * Silicon revision - Read from JTAG version code
 */
SYS_UWORD16 IQ_GetRevision(void)
{
   unsigned short v;
   
   v = *( (volatile unsigned short *) (MEM_JTAGID_VER));
   return(v);
}

#if (TI_PROFILER == 1)

/*
 *  	IQ_InitLevel
 *
 * Parameters : interrupt, FIQ_nIRQ, priority, edge
 * Return     :	none
 * Functionality : initialize Interrupt Level Registers
 */

void IQ_InitLevel ( SYS_UWORD16 inputInt, 
                    SYS_UWORD16 FIQ_nIRQ, 
                    SYS_UWORD16 priority, 
                    SYS_UWORD16 edge )
{
   volatile SYS_UWORD16 *inthLevelReg = (SYS_UWORD16 *) INTH_EXT_REG; 

   inthLevelReg = inthLevelReg + inputInt;

   *inthLevelReg = (FIQ_nIRQ | (priority << 6) | (edge << 1));
}

#endif
