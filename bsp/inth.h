/*******************************************************************************
            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION           
                                                                             
   Property of Texas Instruments -- For  Unrestricted  Internal  Use  Only 
   Unauthorized reproduction and/or distribution is strictly prohibited.  This 
   product  is  protected  under  copyright  law  and  trade  secret law as an 
   unpublished work.  Created 1987, (C) Copyright 1997 Texas Instruments.  All 
   rights reserved.                                                            
                  
                                                           
   Filename       	: inth.h

   Description    	: Header file for the INTH module

   Project        	: drivers

   Author         	: pmonteil@tif.ti.com  Patrice Monteil.

   Version number   : 1.17

   Date             : 09/02/03

   Previous delta 	: 01/22/01 10:32:33

   SCCS file      	: /db/gsm_asp/db_ht96/dsp_0/gsw/rel_0/mcu_l1/release_gprs/RELEASE_GPRS/drivers1/common/SCCS/s.inth.h

   Sccs Id  (SID)       : '@(#) inth.h 1.10 01/30/01 10:22:23 '

 
*****************************************************************************/

#include "../include/config.h"
#include "../include/sys_types.h"

#if (CHIPSET != 12)

/* Adress of the registers */

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9)|| (CHIPSET == 10) || (CHIPSET == 11))
  #define INTH_IT_REG1    MEM_INTH_ADDR 		  /* INTH IT register 1 */
  #define INTH_IT_REG2    (MEM_INTH_ADDR + 0x02)  /* INTH IT register 2 */
  #define INTH_MASK_REG1  (MEM_INTH_ADDR + 0x08)  /* INTH mask register 1 */
  #define INTH_MASK_REG2  (MEM_INTH_ADDR + 0x0a)  /* INTH mask register 2 */
  #define INTH_B_IRQ_REG  (MEM_INTH_ADDR + 0x10)  /* INTH source binary IRQ reg. */
  #define INTH_B_FIQ_REG  (MEM_INTH_ADDR + 0x12)  /* INTH source binary FIQ reg. */
  #define INTH_CTRL_REG   (MEM_INTH_ADDR + 0x14)  /* INTH control register */
  #define INTH_EXT_REG    (MEM_INTH_ADDR + 0x20)  /* INTH 1st external int. reg. */
#else
  #define INTH_IT_REG  	  MEM_INTH_ADDR 		  /* INTH IT register */
  #define INTH_MASK_REG   (MEM_INTH_ADDR + 0x02)  /* INTH mask register */
  #define INTH_S_IRQ_REG  (MEM_INTH_ADDR + 0x04)  /* INTH source IRQ register */
  #define INTH_S_FIQ_REG  (MEM_INTH_ADDR + 0x06)  /* INTH source FIQ register */
  #define INTH_B_IRQ_REG  (MEM_INTH_ADDR + 0x08)  /* INTH source binary IRQ reg. */
  #define INTH_B_FIQ_REG  (MEM_INTH_ADDR + 0x0a)  /* INTH source binary FIQ reg. */
  #define INTH_CTRL_REG   (MEM_INTH_ADDR + 0x0c)  /* INTH control register */
  #define INTH_EXT_REG    (MEM_INTH_ADDR + 0x0e)  /* INTH 1st external int. reg. */
#endif

/* Interrupts number */

#define INTH_TIMER	0			/* number of the TIMER int. */
#define INTH_AIRQ_FIRST	1			/* first external int. number */
#define INTH_AIRQ_LAST	13			/* last external int. number */
#define INTH_DMA	14			/* number of the DMA int. */
#define INTH_LEAD	15			/* number of the LEAD int. */

/* Bit definition of INTH interrupt level registers */

#define INTH_FIQ_NIRQ	0x0001
#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
  #define INTH_PRIORITY	  0x007c
  #define INTH_EDGE_NLVL  0x0002
#else
  #define INTH_PRIORITY	0x001e
  #define INTH_EDGE_NLVL	0x0020
#endif


/* Bit definition of INTH source binary registers */

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
  #define INTH_SRC_NUM	0x001f
#else
  #define INTH_SRC_NUM	0x000f
#endif


/* Bit definition of INTH Control Register */

#define INTH_NEW_IRQ_AGR	0x0001
#define INTH_NEW_FIQ_AGR	0x0002

/* Other useful constants */

#define INTH_IRQ 0
#define INTH_FIQ 1
#define INTH_LEVEL 0
#define INTH_EDGE 1

/*
 * Macros
 */

#define INT_MASK(interrupt) (1 << (interrupt - 1))
#define PENDING_INT(pendingITs, interrupt) (pendingITs & INT_MASK(interrupt))

/*--------------------------------------------------------------*/
/*  INTH_ENABLEONEIT()						*/
/*--------------------------------------------------------------*/
/* Parameters : num of the IT to enable				*/
/* Return     :	none						*/
/* Functionality : Unmask one it				*/
/*--------------------------------------------------------------*/
#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
  #define INTH_ENABLEONEIT(it)( \
    (it < 16) ? (* (volatile unsigned short *) INTH_MASK_REG1 &= ~(1 << it)) : \
                (* (volatile unsigned short *) INTH_MASK_REG2 &= ~(1 << (it-16))) \
  )
#else
  #define INTH_ENABLEONEIT(it)(* (volatile unsigned short *) INTH_MASK_REG &= ~(1 << it))
#endif

/*--------------------------------------------------------------*/
/*  INTH_DISABLEONEIT()						*/
/*--------------------------------------------------------------*/
/* Parameters : num of the IT to disable			*/
/* Return     :	none						*/
/* Functionality : mask one it					*/
/*--------------------------------------------------------------*/
#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
  #define INTH_DISABLEONEIT(it)( \
    (it < 16) ? (* (volatile unsigned short *) INTH_MASK_REG1 |= (1 << it)) : \
                (* (volatile unsigned short *) INTH_MASK_REG2 |= (1 << (it-16))) \
  )
#else
  #define INTH_DISABLEONEIT(it)(* (volatile unsigned short *) INTH_MASK_REG |= (1 << it))
#endif

/*--------------------------------------------------------------*/
/*  INTH_ENABLEALLIT()						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality : Enable all it				*/
/*--------------------------------------------------------------*/

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
  #define INTH_ENABLEALLIT { \
    * (volatile unsigned short *) INTH_MASK_REG1 = 0x0000; \
    * (volatile unsigned short *) INTH_MASK_REG2 = 0x0000; \
  }
#else
  #define INTH_ENABLEALLIT ( * (volatile unsigned short *) INTH_MASK_REG = 0x0000)
#endif

/*--------------------------------------------------------------*/
/*  INTH_DISABLEALLIT()						*/
/*--------------------------------------------------------------*/
/* Parameters : none						*/
/* Return     :	none						*/
/* Functionality :mask all it					*/
/*--------------------------------------------------------------*/

#if (CHIPSET == 4)
  #define INTH_DISABLEALLIT { \
  * (volatile unsigned short *) INTH_MASK_REG1 = 0xffff; \
  * (volatile unsigned short *) INTH_MASK_REG2 = 0x000f; \
  }
#elif ((CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 9))
  #define INTH_DISABLEALLIT { \
  * (volatile unsigned short *) INTH_MASK_REG1 = 0xffff; \
  * (volatile unsigned short *) INTH_MASK_REG2 = 0x01ff; \
  }
#elif (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11)
  #define INTH_DISABLEALLIT { \
  * (volatile unsigned short *) INTH_MASK_REG1 = 0xffff; \
  * (volatile unsigned short *) INTH_MASK_REG2 = 0xffff; \
  }
#else
  #define INTH_DISABLEALLIT (* (volatile unsigned short *) INTH_MASK_REG = 0xffff)
#endif


/*--------------------------------------------------------------*/
/*  INTH_CLEAR()						*/
/*--------------------------------------------------------------*/
/* Parameters : value to write                		        */
/* Return     :	none						*/
/* Functionality :valid next it					*/
/*--------------------------------------------------------------*/


#define INTH_CLEAR (* (volatile SYS_UWORD16 *) INTH_CTRL_REG = 0x0003)


/*--------------------------------------------------------------*/
/*  INTH_VALIDNEXT()						*/
/*--------------------------------------------------------------*/
/* Parameters : num of the processed it				*/
/* Return     :	none						*/
/* Functionality :valid next it					*/
/*--------------------------------------------------------------*/

#define INTH_VALIDNEXT (intARM)( * (volatile SYS_UWORD16 *) INTH_CTRL_REG |= (1 << intARM))

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
  /*--------------------------------------------------------------*/
  /*  INTH_RESETALLIT()                                           */
  /*--------------------------------------------------------------*/
  /* Parameters :  None                                            */
  /* Return     :  None                                           */
  /* Functionality :Reset the inth it register                    */
  /*--------------------------------------------------------------*/

  #define INTH_RESETALLIT { \
   * (volatile unsigned short *) INTH_IT_REG1 &= 0x0000; \
   * (volatile unsigned short *) INTH_IT_REG2 &= 0x0000; \
  }  
#endif


#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
  /*-------------------------------------------------------------*/
  /*   INTH_RESETONEIT()                                         */
  /*-------------------------------------------------------------*/
  /* Parameters : Num of the IT to reset                         */
  /* Return     : None                                           */
  /* Functionality : Reset one IT of the inth IT register        */
  /*-------------------------------------------------------------*/    
  #define INTH_RESETONEIT(it) ( \
    (it<16) ? (* (volatile unsigned short *) INTH_IT_REG1 &= ~(1 << it)) : \
              (* (volatile unsigned short *) INTH_IT_REG2 &= ~(1 << (it-16))) \
			       )
#else  // CHIPSET == 2,3
  #define INTH_RESETONEIT(it) (* (volatile unsigned short *) INTH_IT_REG &= ~(1 << it))
#endif // CHIPSET

/* Prototypes */

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11))
  unsigned long INTH_GetPending (void);
  unsigned long INTH_ResetIT (void);
#else
  unsigned short INTH_GetPending (void);
  unsigned short INTH_ResetIT (void);
#endif

unsigned short INTH_Ack (int);
void INTH_InitLevel (int, int, int, int);


#endif /* endif chipset != 12 */

