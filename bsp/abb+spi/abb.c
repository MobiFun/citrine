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
/* Filename         : abb.c                                                   */
/*                                                                            */
/* Description      : Functions to drive the ABB device.                      */
/*                    The Serial Port Interface is used to connect the TI     */
/*                    Analog BaseBand (ABB).                                  */
/*                    It is assumed that the ABB is connected as the SPI      */
/*                    device 0.                                               */
/*                                                                            */
/* Author           : Pascal PUEL                                             */
/*                                                                            */
/* Version number   : 1.3                                                     */
/*                                                                            */
/* Date and time    : 08/22/03                                                */
/*                                                                            */
/* Previous delta   : Creation                                                */
/*                                                                            */
/******************************************************************************/

#include "../../include/config.h"
#include "../../include/sys_types.h"
#include "../../riviera/rv/general.h"
#include "../../nucleus/nucleus.h"      // for NUCLEUS functions and types

#include "../../L1/include/l1_confg.h"
#include "../../L1/include/l1_macro.h"

#include "abb.h"
#include "../clkm.h"         // for wait_ARM_cycles function
#include "abb_inline.h"
#include "../ulpd.h"        // for FRAME_STOP definition

#include "../../L1/include/l1_types.h"

#if 0	// FreeCalypso
#include "buzzer/buzzer.h"	     // for BZ_KeyBeep_OFF function
#endif

#if (VCXO_ALGO == 1)
  #include "../../L1/include/l1_ctl.h"
#endif

#if (RF_FAM == 35)
  #include "../../L1/cust0/l1_rf35.h"
#endif

#if (RF_FAM == 12)
  #include "../../L1/tpudrv/tpudrv12.h" 
  #include "../../L1/cust0/l1_rf12.h"
#endif

#if (RF_FAM == 10)
  #include "../../L1/cust0/l1_rf10.h"
#endif

#if (RF_FAM == 8)
  #include "../../L1/cust0/l1_rf8.h"
#endif

#if (RF_FAM == 2)
  #include "../../L1/cust0/l1_rf2.h"
#endif

#if (ABB_SEMAPHORE_PROTECTION)   

static NU_SEMAPHORE abb_sem;

/*-----------------------------------------------------------------------*/
/* ABB_Sem_Create()                                                      */
/*                                                                       */
/* This function creates the Nucleus semaphore to protect ABB accesses   */
/* against preemption.                                                   */
/* No check on the result.                                               */
/*                                                                       */
/*-----------------------------------------------------------------------*/
void ABB_Sem_Create(void)
{
  // create a semaphore with an initial count of 1 and with FIFO type suspension. 
  NU_Create_Semaphore(&abb_sem, "ABB_SEM", 1, NU_FIFO);
}

#endif  // ABB_SEMAPHORE_PROTECTION   

/*-----------------------------------------------------------------------*/
/* ABB_Wait_IBIC_Access()                                                */
/*                                                                       */
/* This function waits for the first IBIC access.                        */
/*                                                                       */
/*-----------------------------------------------------------------------*/
void ABB_Wait_IBIC_Access(void)
{
  #if (ANALOG ==1)
    // Wait 6 OSCAS cycles (100 KHz) for first IBIC access 
    // (i.e wait 60us + 10% security marge = 66us)
    wait_ARM_cycles(convert_nanosec_to_cycles(66000));
  #elif ((ANALOG ==2) || (ANALOG == 3))
    // Wait 6 x 32 KHz clock cycles for first IBIC access 
    // (i.e wait 187us + 10% security marge = 210us)
    wait_ARM_cycles(convert_nanosec_to_cycles(210000));
  #endif
}



/*-----------------------------------------------------------------------*/
/* ABB_Write_Register_on_page()                                          */
/*                                                                       */
/* This function manages all the spi serial transfer to write to an      */
/* ABB register on a specified page.                                     */
/*                                                                       */
/*-----------------------------------------------------------------------*/    
void ABB_Write_Register_on_page(SYS_UWORD16 page, SYS_UWORD16 reg_id, SYS_UWORD16 value)
{
  volatile SYS_UWORD16 status;

  // Start spi clock, mask IT for WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_WR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  #if ((ABB_SEMAPHORE_PROTECTION == 1) || (ABB_SEMAPHORE_PROTECTION == 2) || (ABB_SEMAPHORE_PROTECTION == 3))  

  // check if the semaphore has been correctly created and try to obtain it.
  // if the semaphore cannot be obtained, the task is suspended and then resumed 
  // as soon as the semaphore is released.
  if(&abb_sem != 0)
  {
    NU_Obtain_Semaphore(&abb_sem, NU_SUSPEND);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  // set the ABB page for register access
  ABB_SetPage(page);

  // Write value in reg_id
  ABB_WriteRegister(reg_id, value);

  // set the ABB page for register access at page 0
  ABB_SetPage(PAGE0);

  #if ((ABB_SEMAPHORE_PROTECTION == 1) || (ABB_SEMAPHORE_PROTECTION == 2) || (ABB_SEMAPHORE_PROTECTION == 3))  
  // release the semaphore only if it has correctly been created.
  if(&abb_sem != 0)
  {
    NU_Release_Semaphore(&abb_sem);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif
}


/*-----------------------------------------------------------------------*/
/* ABB_Read_Register_on_page()                                           */
/*                                                                       */    
/* This function manages all the spi serial transfer to read one         */
/* ABB register on a specified page.                                     */
/*                                                                       */ 
/* Returns the real data value of the register.                          */ 
/*                                                                       */ 
/*-----------------------------------------------------------------------*/    
SYS_UWORD16 ABB_Read_Register_on_page(SYS_UWORD16 page, SYS_UWORD16 reg_id)
{
  volatile SYS_UWORD16 status;
  SYS_UWORD16 reg_val;

  // Start spi clock, mask IT for RD and WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_RDWR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  #if ((ABB_SEMAPHORE_PROTECTION == 1) || (ABB_SEMAPHORE_PROTECTION == 2) || (ABB_SEMAPHORE_PROTECTION == 3))  

  // check if the semaphore has been correctly created and try to obtain it.
  // if the semaphore cannot be obtained, the task is suspended and then resumed 
  // as soon as the semaphore is released.
  if(&abb_sem != 0)
  {
    NU_Obtain_Semaphore(&abb_sem, NU_SUSPEND);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  /* set the ABB page for register access */
  ABB_SetPage(page);

  /* Read selected ABB register */
  reg_val = ABB_ReadRegister(reg_id);

  /* set the ABB page for register access at page 0 */
  ABB_SetPage(PAGE0);

  #if ((ABB_SEMAPHORE_PROTECTION == 1) || (ABB_SEMAPHORE_PROTECTION == 2) || (ABB_SEMAPHORE_PROTECTION == 3))  
  // release the semaphore only if it has correctly been created.
  if(&abb_sem != 0)
  {
    NU_Release_Semaphore(&abb_sem);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif

  return (reg_val);     // Return result
}

/*------------------------------------------------------------------------*/
/* ABB_free_13M()                                                         */
/*                                                                        */
/* This function sets the 13M clock working in ABB. A wait loop           */ 
/* is required to allow first slow access to ABB clock register.          */
/*                                                                        */ 
/* WARNING !! : this function must not be protected by semaphore !!       */ 
/*                                                                        */ 
/*------------------------------------------------------------------------*/ 
void ABB_free_13M(void)
{
  volatile SYS_UWORD16 status;

  // Start spi clock, mask IT for WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_WR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  ABB_SetPage(PAGE0);

  // This transmission frees the CLK13 in ABB.
  ABB_WriteRegister(TOGBR2, 0x08);

  // Wait for first IBIC access
  ABB_Wait_IBIC_Access();

  // SW Workaround : This transmission has to be done twice.
  ABB_WriteRegister(TOGBR2, 0x08);

  // Wait for first IBIC access
  ABB_Wait_IBIC_Access();

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif
}



/*------------------------------------------------------------------------*/
/* ABB_stop_13M()                                                         */
/*                                                                        */
/* This function stops the 13M clock in ABB.                              */
/*                                                                        */ 
/*------------------------------------------------------------------------*/ 
void ABB_stop_13M(void)
{
  volatile SYS_UWORD16 status;

  // Start spi clock, mask IT for WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_WR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  ABB_SetPage(PAGE0);

  // Set ACTIVMCLK = 0.
  ABB_WriteRegister(TOGBR2, 0x04);

  // Wait for first IBIC access
  ABB_Wait_IBIC_Access();

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif
}



/*------------------------------------------------------------------------*/
/* ABB_Read_Status()                                                      */
/*                                                                        */
/* This function reads and returns the value of VRPCSTS ABB register.     */
/*                                                                        */ 
/*------------------------------------------------------------------------*/ 
SYS_UWORD16 ABB_Read_Status(void)
{      
  volatile SYS_UWORD16 status;
  SYS_UWORD16 reg_val;

  // Start spi clock, mask IT for WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_WR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  #if ((ABB_SEMAPHORE_PROTECTION == 2) || (ABB_SEMAPHORE_PROTECTION == 3))  

  // check if the semaphore has been correctly created and try to obtain it.
  // if the semaphore cannot be obtained, the task is suspended and then resumed 
  // as soon as the semaphore is released.
  if(&abb_sem != 0)
  {
    NU_Obtain_Semaphore(&abb_sem, NU_SUSPEND);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  ABB_SetPage(PAGE0);

  #if (ANALOG == 1) || (ANALOG == 2)
    ABB_SetPage(PAGE0);
    reg_val = ABB_ReadRegister(VRPCSTS);
  #elif (ANALOG == 3)
    ABB_SetPage(PAGE1);
    reg_val = ABB_ReadRegister(VRPCCFG);
  #endif

  #if ((ABB_SEMAPHORE_PROTECTION == 2) || (ABB_SEMAPHORE_PROTECTION == 3))  
  // release the semaphore only if it has correctly been created.
  if(&abb_sem != 0)
  {
    NU_Release_Semaphore(&abb_sem);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif

  return (reg_val);      
}

/*------------------------------------------------------------------------*/
/* ABB_on()                                                               */
/*                                                                        */
/* This function configures ABB registers to work in ON condition         */ 
/*                                                                        */ 
/*------------------------------------------------------------------------*/ 
void ABB_on(SYS_UWORD16 modules, SYS_UWORD8 bRecoveryFlag)
{
  volatile SYS_UWORD16 status;
  #if ((ANALOG ==2) || (ANALOG == 3))
    SYS_UWORD32 reg;
  #endif
 
  // a possible cause of the recovery is that ABB is on Oscas => switch from Oscas to CLK13
  if (bRecoveryFlag)     
  {
    // RESTITUTE 13MHZ CLOCK TO ABB
    //---------------------------------------------------
    ABB_free_13M();

    // RESTITUTE 13MHZ CLOCK TO ABB AGAIN (C.F. BUG1719)
    //---------------------------------------------------
    ABB_free_13M();
  }

  // Start spi clock, mask IT for RD and WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_RDWR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  #if (ABB_SEMAPHORE_PROTECTION == 3)  

  // check if the semaphore has been correctly created and try to obtain it.
  // if the semaphore cannot be obtained, the task is suspended and then resumed 
  // as soon as the semaphore is released.
  if(&abb_sem != 0)
  {
    NU_Obtain_Semaphore(&abb_sem, NU_SUSPEND);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  ABB_SetPage(PAGE0);

  // This transmission disables MADC,AFC,VDL,VUL modules.
  ABB_WriteRegister(TOGBR1, 0x0155);  

  #if (ANALOG == 1)
    // This transmission disables Band gap fast mode Enable BB charge.
    ABB_WriteRegister(VRPCCTL2, 0x1fc);  

    /* *********** DC/DC enabling selection ************************************************************** */
    // This transmission changes the register page in OMEGA for usp to pg1.
    ABB_SetPage(PAGE1);

    /* Insert here accesses to modify DC/DC parameters. Default is a switching frequency of 240 Khz */
    {
      SYS_UWORD8 vrpcctrl3_data;

      #if (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11)
        vrpcctrl3_data = 0x007d;  // core voltage 1.4V for C035
      #else
        vrpcctrl3_data = 0x00bd;  // core voltage 1.8V for C05
      #endif

    if(modules & DCDC)    // check if the DCDC is enabled
    {  
       vrpcctrl3_data |= 0x0002; // set DCDCEN
    }  
       
    // This access disables the DCDC.
    ABB_WriteRegister(VRPCCTRL3, vrpcctrl3_data);  
    }  

    /* ************************  SELECTION OF TEST MODE FOR ABB **************************************** */
    /* This test configuration allows visibility on BULENA,BULON,BDLON,BDLENA on test pins               */
    /* ***************************************************************************************************/
    #if (BOARD==6)&& (ANALOG==1)  //BUG01967 to remove access to TAPCTRL   (EVA4 board and Nausica)                                                                 
      // This transmission enables Omega test register.
      ABB_WriteRegister(TAPCTRL, 0x01);  

      // This transmission select Omega test instruction.
      ABB_WriteRegister(TAPREG, TSPTEST1);  

      // This transmission disables Omega test register.
      ABB_WriteRegister(TAPCTRL, 0x00);  
    #endif
    /* *************************************************************************************************** */

    if (!bRecoveryFlag)    // Check recovery status from L1, prevent G23 SIM issue
    {
      // This transmission changes SIM power supply to 3 volts.
      ABB_WriteRegister(VRPCCTRL1, 0x45);  
    }

    ABB_SetPage(PAGE0);

    // This transmission enables selected OMEGA modules.
    ABB_WriteRegister(TOGBR1, (modules & ~DCDC) >> 6);  
  
    if(modules & MADC)   // check if the ADC is enabled
    {  
      // This transmission connects the resistive divider to MB and BB.
      ABB_WriteRegister(BCICTL1, 0x0005);  
    }
  #elif ((ANALOG == 2) || (ANALOG == 3)) 
    // Restore the ABB checks and debouncing if start on TESTRESETZ 

    // This transmission changes the register page in the ABB for usp to pg1.
    ABB_SetPage(PAGE1);

    // This transmission sets the AFCCK to CKIN/2.
    ABB_WriteRegister(AFCCTLADD, 0x01);  
    
    // This transmission enables the tapreg.
    ABB_WriteRegister(TAPCTRL, 0x01);  

    // This transmission enables access to page 2.
    ABB_WriteRegister(TAPREG, 0x01b);  

    // This transmission changes the register page in the ABB for usp to pg2.
    ABB_SetPage(PAGE2);

    #if (ANALOG == 2)
    // Restore push button environment
    ABB_WriteRegister(0x3C, 0x07);

    #elif (ANALOG == 3)

    // Restore push button environment
    ABB_WriteRegister(0x3C, 0xBF);

      /* ************************  SELECTION OF BBCFG CONFIG FOR ABB 3 PG1_0 *******************************/
        #if (ANLG_PG == S_PG_10)                    // SYREN PG1.0 ON ESAMPLE
          ABB_WriteRegister(BBCFG, C_BBCFG);         // Initialize transmit register
        #endif
    // This transmission enables access to page 0.
    ABB_SetPage(PAGE0);
    
    // reset bit MSKINT1 , if set by TESTRESET
    reg=ABB_ReadRegister(VRPCSTS) & 0xffe;

    ABB_WriteRegister(VRPCSTS, reg);

    ABB_SetPage(PAGE2);

    // Restore default for BG behavior in sleep mode
    ABB_WriteRegister(VRPCAUX, 0xBF);  

    // Restore default for deboucing length
    ABB_WriteRegister(VRPCLDO, 0x00F);  

    // Restore default for INT1 generation, wait time in switch on, checks in switch on
    ABB_WriteRegister(VRPCABBTST, 0x0002);  

    #endif

    // This transmission changes the register page in the ABB for usp to pg1.
    ABB_SetPage(PAGE1);

    // This transmission sets tapinst to id code.
    ABB_WriteRegister(TAPREG, 0x0001);  

    // This transmission disables TAPREG access.
    ABB_WriteRegister(TAPCTRL, 0x00);  

    // enable BB battery charge BCICONF register, enable test mode to track BDLEN and BULEN windows
    // This transmission enables BB charge and BB bridge connection for BB measurements.
    ABB_WriteRegister(BCICONF, 0x060);  

     /* ************************  SELECTION OF BBCFG CONFIG FOR ABB 3 PG2_0 *******************************/
      #if (ANALOG == 3)
        #if (ANLG_PG == S_PG_20)                     // SYREN PG2.0 ON EVACONSO
           ABB_WriteRegister(BBCFG, C_BBCFG);         // Initialize transmit register
        #endif
      #endif

    /* ************************  SELECTION OF TEST MODE FOR ABB ******************************************/
    /* This test configuration allows visibility on test pins  TAPCTRL has not to be reset                */
    /* ****************************************************************************************************/
                                    
    // This transmission enables the tapreg.
    ABB_WriteRegister(TAPCTRL, 0x01);  

    // This transmission select ABB test instruction.
    ABB_WriteRegister(TAPREG, TSPEN);  

    // This transmission changes the register page in ABB for usp to pg0.
    ABB_SetPage(PAGE0);

    // This transmission enables selected ABB modules.
    ABB_WriteRegister(TOGBR1, modules >> 6);  

    // enable MB & BB resistive bridges for measurements
    if(modules & MADC)       // check if the ADC is enabled
    {  
      // This transmission connects the resistive divider to MB and BB.
      ABB_WriteRegister(BCICTL1, 0x0001);  
    }

    /********* Sleep definition part ******************/
    // This transmission changes the register page in the ABB for usp to pg1.
    #if (ANALOG == 2)
      ABB_SetPage(PAGE1);

      // update the Delay needed by the ABB before going in deep sleep, and clear previous delay value.
      reg = ABB_ReadRegister(VRPCCFG) & 0x1e0;

      ABB_WriteRegister(VRPCCFG, (SLPDLY | reg));  

      // update the ABB mask sleep register (regulator disabled in deep sleep), and clear previous mask value.
      reg = ABB_ReadRegister(VRPCMSK) & 0x1e0;
      ABB_WriteRegister(VRPCMSK, (MASK_SLEEP_MODE | reg));  
    #elif (ANALOG == 3)
         Syren_Sleep_Config(NORMAL_SLEEP,SLEEP_BG,SLPDLY);
    #endif
    //  This transmission changes the register page in the ABB for usp to pg0.
    ABB_SetPage(PAGE0);
  #endif 

  // SW workaround for initialization of the audio parts of the ABB to avoid white noise 
  // C.f. BUG1941
  // Set VDLR and VULR bits
  // Write TOGBR1 register
  // This transmission enables selected ABB modules.
  ABB_WriteRegister(TOGBR1, 0x0A);  
  
  // wait for 1 ms
  wait_ARM_cycles(convert_nanosec_to_cycles(1000000));
  
  // Reset VDLS and VULS bits
  // Write TOGBR1 register
  // This transmission enables selected ABB modules.
  ABB_WriteRegister(TOGBR1, 0x05);  
  
  #if (ABB_SEMAPHORE_PROTECTION == 3)  
  // release the semaphore only if it has correctly been created.
  if(&abb_sem != 0)
  {
    NU_Release_Semaphore(&abb_sem);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  // Stop the SPI clock 
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif
}



/*-----------------------------------------------------------------------*/
/* ABB_Read_ADC()                                                        */
/*                                                                       */    
/* This function manages all the spi serial transfer to read all the     */
/* ABB ADC conversion channels.                                          */
/* Stores the result in Buff parameter.                                  */ 
/*                                                                       */ 
/*-----------------------------------------------------------------------*/    
void ABB_Read_ADC(SYS_UWORD16 *Buff)
{
  volatile SYS_UWORD16 status;

  // Start spi clock, mask IT for RD and WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_RDWR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  #if (ABB_SEMAPHORE_PROTECTION == 3)  

  // check if the semaphore has been correctly created and try to obtain it.
  // if the semaphore cannot be obtained, the task is suspended and then resumed
  // as soon as the semaphore is released.
  if(&abb_sem != 0)
  {
    NU_Obtain_Semaphore(&abb_sem, NU_SUSPEND);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  // This transmission changes the register page in the ABB for usp to pg0.
  ABB_SetPage(PAGE0);

  /* Read all ABB ADC registers */
  *Buff++ = ABB_ReadRegister(VBATREG);
  *Buff++ = ABB_ReadRegister(VCHGREG);
  *Buff++ = ABB_ReadRegister(ICHGREG);
  *Buff++ = ABB_ReadRegister(VBKPREG);
  *Buff++ = ABB_ReadRegister(ADIN1REG);
  *Buff++ = ABB_ReadRegister(ADIN2REG);
  *Buff++ = ABB_ReadRegister(ADIN3REG);

  #if (ANALOG ==1)
    *Buff++ = ABB_ReadRegister(ADIN4XREG);
    *Buff++ = ABB_ReadRegister(ADIN5YREG);
  #elif (ANALOG ==2)
    *Buff++ = ABB_ReadRegister(ADIN4REG);
   #elif (ANALOG == 3)
    *Buff++ = ABB_ReadRegister(ADIN4REG);
    *Buff++ = ABB_ReadRegister(ADIN5REG);
  #endif   // ANALOG

  #if (ABB_SEMAPHORE_PROTECTION == 3)  
  // release the semaphore only if it has correctly been created.
  if(&abb_sem != 0)
  {
    NU_Release_Semaphore(&abb_sem);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif
}



/*-----------------------------------------------------------------------*/
/* ABB_Conf_ADC()                                                        */
/*                                                                       */    
/* This function manages all the spi serial transfer to:                 */
/*  - select the ABB ADC channels to be converted                        */
/*  - enable/disable EOC interrupt                                       */ 
/*                                                                       */ 
/*-----------------------------------------------------------------------*/ 
void ABB_Conf_ADC(SYS_UWORD16 Channels, SYS_UWORD16 ItVal)                              
{
  volatile SYS_UWORD16 status;
  SYS_UWORD16 reg_val;

  // Start spi clock, mask IT for RD and WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_RDWR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  #if (ABB_SEMAPHORE_PROTECTION == 3)  

  // check if the semaphore has been correctly created and try to obtain it.
  // if the semaphore cannot be obtained, the task is suspended and then resumed
  // as soon as the semaphore is released.
  if(&abb_sem != 0)
  {
    NU_Obtain_Semaphore(&abb_sem, NU_SUSPEND);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  // This transmission changes the register page in the ABB for usp to pg0.
  ABB_SetPage(PAGE0);

  /* select ADC channels to be converted */
  #if (ANALOG == 1)  
    ABB_WriteRegister(MADCCTRL1, Channels);  
  #elif ((ANALOG == 2) || (ANALOG == 3))
    ABB_WriteRegister(MADCCTRL, Channels);  
  #endif

  reg_val = ABB_ReadRegister(ITMASK);

  // This transmission configure the End Of Conversion IT without modifying other bits in the same register.
  if(ItVal == EOC_INTENA)
    ABB_WriteRegister(ITMASK, reg_val & EOC_INTENA);
  else if(ItVal == EOC_INTMASK)    
    ABB_WriteRegister(ITMASK, reg_val | EOC_INTMASK);  

  #if (ABB_SEMAPHORE_PROTECTION == 3)  
  // release the semaphore only if it has correctly been created.
  if(&abb_sem != 0)
  {
    NU_Release_Semaphore(&abb_sem);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif
}




/*------------------------------------------------------------------------*/
/* ABB_sleep()                                                            */
/*                                                                        */
/* This function disables the DCDC and returns to PAGE 0. It stops then   */
/* the 13MHz clock in ABB. A wait loop s required to allow                */ 
/* first slow access to ABB clock register.                               */
/*                                                                        */ 
/* WARNING !! : this function must not be protected by semaphore !!       */ 
/*                                                                        */ 
/* Returns AFC value.                                                     */ 
/*                                                                        */ 
/*------------------------------------------------------------------------*/ 
SYS_UWORD32 ABB_sleep(SYS_UWORD8 sleep_performed, SYS_WORD16 afc)
{
  volatile SYS_UWORD16 status;
  SYS_UWORD32 afcout_index;
  volatile SYS_UWORD16 nb_it;
  SYS_UWORD16 reg_val;

  // table for AFC allowed values during Sleep mode. First 5th elements
  // are related to positive AFC values, last 5th to negative ones.
  static const SYS_UWORD32 Afcout_T[10] =
			{0x0f,0x1f,0x3f,0x7f,0xff,0x00,0x01,0x03,0x07,0x0f};

  // Start spi clock, mask IT for RD and WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_RDWR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  // COMPUTATION AND PROGRAMMING OF AFC VALUE
  //---------------------------------------------------
  if(afc & 0x1000)
    afcout_index = ((afc + 512)>>10) + 1;
  else  
    afcout_index = (afc  + 512)>>10;

  if (sleep_performed == FRAME_STOP)   // Big sleep
  {
    #if ((ANALOG == 2) || (ANALOG == 3))
      //////////// ADD HERE IOTA or SYREN CONFIGURATION FOR BIG SLEEP ////////////////////////////
    #endif

  }
  else                                  // Deep sleep 
  {
    #if(ANALOG == 1) 
      // SELECTION OF AFC TEST MODE FOR OMEGA 
      //---------------------------------------------------
      // This test configuration allows access on the AFCOUT register 
      ABB_SetPage(PAGE1);

      // This transmission enables OMEGA test register. 
      ABB_WriteRegister(TAPCTRL, 0x01);  

      // This transmission selects OMEGA test instruction.
      ABB_WriteRegister(TAPREG, AFCTEST);  

      // Set AFCOUT to 0.
      ABB_WriteRegister(AFCOUT, 0x00 >> 6);  

      ABB_SetPage(PAGE0);

    #elif (ANALOG == 2)
      // This configuration allows access on the AFCOUT register 
      ABB_SetPage(PAGE1);

      // Read AFCCTLADD value and enable USP access to AFCOUT register
      reg_val = (ABB_ReadRegister(AFCCTLADD) | 0x04);

      ABB_WriteRegister(AFCCTLADD, reg_val);  

      // Set AFCOUT to 0.
      ABB_WriteRegister(AFCOUT, 0x00);  

      // Read BCICONF value	and cut the measurement bridge of BB cut the BB charge.
      reg_val = ABB_ReadRegister(BCICONF) & 0x039f;

      ABB_WriteRegister(BCICONF, reg_val);  
    
      // Disable the ABB test mode
      ABB_WriteRegister(TAPCTRL, 0x00);  
    
      ABB_SetPage(PAGE0);

      // Read BCICTL1 value and cut the measurement bridge of MB.
      reg_val = ABB_ReadRegister(BCICTL1) & 0x03fe;

      ABB_WriteRegister(BCICTL1, reg_val);  
    #endif
     
     #if (ANALOG == 3)              // Nothing to be done as MB and BB measurement bridges are automatically disconnected 
                                              // in Syren during sleep mode. BB charge stays enabled 
       ABB_SetPage(PAGE1);                    // Initialize transmit reg_num. This transmission
                                              // change the register page in IOTA for usp to pg1

       ABB_WriteRegister(TAPCTRL, 0x00);      // Disable Syren test mode

       ABB_SetPage(PAGE0);
     #endif

    // switch off MADC, AFC, AUXDAC, VOICE.
    ABB_WriteRegister(TOGBR1, 0x155);  

    // Switch off Analog supply LDO
    //-----------------------------
    #if (ANALOG == 1)  
      ABB_SetPage(PAGE1);
  
      // Read VRPCCTL3 register value and switch off VR3.  
      reg_val = ABB_ReadRegister(VRPCCTRL3) & 0x3df;

      ABB_WriteRegister(VRPCCTRL3, reg_val);  
      
    #elif (ANALOG == 2)
      // Read VRPCSTS register value and extract status of meaningfull inputs.  
      reg_val = ABB_ReadRegister(VRPCSTS) & 0x0070;

      if (reg_val == 0x30)
      {
        // start the SLPDLY counter in order to switch the ABB in sleep mode. This transmission sets IOTA sleep bit.
        ABB_WriteRegister(VRPCDEV, 0x02);  
      }

      // Dummy transmission to clean of ABB bus. This transmission accesses IOTA address 0 in "read".
      ABB_WriteRegister(0x0000 | 0x0001, 0x0000);  

    #elif (ANALOG == 3)
       // In Syren there is no need to check for VRPCCFG as wake up prioritys are changed
       // start the SLPDLY counter in order to switch the ABB in sleep mode
       ABB_WriteRegister(VRPCDEV,0x02);     // Initialize transmit reg_num. This transmission
                                            // set Syren sleep bit
/*
      // Dummy transmission to clean of ABB bus. This transmission accesses SYREN address 0 in "read".
      ABB_WriteRegister(0x0000 | 0x0001, 0x0000);  
*/
    #endif

    // Switch to low frequency clock  
    ABB_stop_13M();
  }

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif

#if (OP_L1_STANDALONE == 1)
   #if (CHIPSET == 12)
       // GPIO_InitAllPull(ALL_ONE);  // enable all GPIO internal pull
       // workaround to set APLL_DIV_CLK( internal PU) at high level
       // by default APLL_DIV_CLK is low pulling 80uA on VRIO
//       *(SYS_UWORD16*) (0xFFFFFD90)= 0x01;//CNTL_APLL_DIV_CLK -> APLL_CLK_DIV != 0
//       *(SYS_UWORD16*) (0xFFFEF030)= 0x10;// DPLL mode
   #endif
#endif
  return(Afcout_T[afcout_index]);
}


/*------------------------------------------------------------------------*/
/* ABB_wakeup()                                                           */
/*                                                                        */
/* This function sets the 13MHz clock working in ABB. A wait loop         */    
/* is required to allow first slow access to ABB clock register.          */
/* Then it re-enables DCDC and returns to PAGE 0.                         */
/*                                                                        */ 
/* WARNING !! : this function must not be protected by semaphore !!       */ 
/*                                                                        */ 
/*------------------------------------------------------------------------*/ 
void ABB_wakeup(SYS_UWORD8 sleep_performed, SYS_WORD16 afc)
{
  volatile SYS_UWORD16 status;
  SYS_UWORD16 reg_val;

  // Start spi clock, mask IT for RD and WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_RDWR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  if (sleep_performed == FRAME_STOP)   // Big sleep
  { 
    #if ((ANALOG == 2) || (ANALOG == 3))
      //////////// ADD HERE IOTA or SYREN CONFIGURATION FOR BIG SLEEP WAKEUP ////////////////////////////
    #endif
  }
  else                                  // Deep sleep 
  {
    #if (OP_L1_STANDALONE == 1)
      #if (CHIPSET == 12)
         // restore context from     
         // workaround to set APLL_DIV_CLK( internal PU) at high level
         // by default APLL_DIV_CLK is low pulling 80uA on VRIO
//         *(SYS_UWORD16*) (0xFFFFFD90)= 0x00;//CNTL_APLL_DIV_CLK -> APLL_DIV_CLK != 0
//         *(SYS_UWORD16*) (0xFFFEF030)= 0x00;// DPLL mode
      #endif
    #endif

    // Restitutes 13MHZ Clock to ABB
    ABB_free_13M();

    // Switch ON Analog supply LDO
    #if (ANALOG == 1)   
      ABB_SetPage(PAGE1);

      // Read VRPCCTL3 register value and switch on VR3.  
      reg_val = ABB_ReadRegister(VRPCCTRL3) | 0x020;

      ABB_WriteRegister(VRPCCTRL3, reg_val);  
      ABB_SetPage(PAGE0);
    #endif

    // This transmission switches on MADC, AFC.
    ABB_WriteRegister(TOGBR1, 0x280);  

    // This transmission sets the AUXAFC2. 
    ABB_WriteRegister(AUXAFC2, ((afc>>10) & 0x7));  

    // This transmission sets the AUXAFC1. 
    ABB_WriteRegister(AUXAFC1, (afc & 0x3ff));  

    #if (ANALOG == 1)
      // Remove AFC test mode  
      ABB_SetPage(PAGE1);

      // This transmission select Omega test instruction.
      ABB_WriteRegister(TAPREG, TSPTEST1);  

      // Disable test mode selection
      // This transmission disables Omega test register. 
      ABB_WriteRegister(TAPCTRL, 0x00 >> 6);  

      ABB_SetPage(PAGE0);

    #elif (ANALOG == 2)
      ABB_SetPage(PAGE1);

      // Read AFCCTLADD register value and disable USP access to AFCOUT register.  
      reg_val = ABB_ReadRegister(AFCCTLADD) & ~0x04;

      ABB_WriteRegister(AFCCTLADD, reg_val);  

      // Read BCICONF register value and enable BB measurement bridge enable BB charge.  
      reg_val = ABB_ReadRegister(BCICONF) | 0x0060;

      ABB_WriteRegister(BCICONF, reg_val);  


      /* *************************************************************************************************** */
      // update the Delay needed by the ABB before going in deep sleep, and clear previous delay value.
      reg_val = ABB_ReadRegister(VRPCCFG) & 0x1e0;
      ABB_WriteRegister(VRPCCFG, (SLPDLY | reg_val));

      // Enable the ABB test mode
      ABB_WriteRegister(TAPCTRL, 0x01);  
      ABB_WriteRegister(TAPREG, TSPEN);
      ABB_SetPage(PAGE0);

      // Read BCICTL1 register value and enable MB measurement bridge and cut the measurement bridge of MB.  
      reg_val = ABB_ReadRegister(BCICTL1) | 0x0001;

      ABB_WriteRegister(BCICTL1, reg_val);  
    #endif   

  #if (ANALOG == 3)

    ABB_SetPage(PAGE1);

    /* *************************************************************************************************** */
    // update the Delay needed by the ABB before going in deep sleep, and clear previous delay value.
    reg_val = ABB_ReadRegister(VRPCCFG) & 0x1e0;
    ABB_WriteRegister(VRPCCFG, (SLPDLY | reg_val));
    
    /* ************************  SELECTION OF TEST MODE FOR ABB=3 *****************************************/
    /* This test configuration allows visibility on test pins  TAPCTRL has not to be reset                */
    /* ****************************************************************************************************/
                                  
    ABB_WriteRegister(TAPCTRL, 0x01);             // Initialize the transmit register  
                                                  // This transmission enables IOTA test register 

    ABB_WriteRegister(TAPREG, TSPEN);
                                                  // This transmission select IOTA test instruction 
                                                  // This transmission select IOTA test instruction 
    /**************************************************************************************************** */
        
    ABB_SetPage(PAGE0);                          // Initialize transmit reg_num. This transmission
  #endif
  } 

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif
}

/*------------------------------------------------------------------------*/
/* ABB_wa_VRPC()                                                          */
/*                                                                        */
/* This function initializes the VRPCCTRL1 or VRPCSIM register            */
/* according to the ABB used.                                             */
/*                                                                        */
/*------------------------------------------------------------------------*/ 
void ABB_wa_VRPC(SYS_UWORD16 value)
{
  volatile SYS_UWORD16 status;

  // Start spi clock, mask IT for WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_WR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  #if ((ABB_SEMAPHORE_PROTECTION == 1) || (ABB_SEMAPHORE_PROTECTION == 2) || (ABB_SEMAPHORE_PROTECTION == 3))  

  // check if the semaphore has been correctly created and try to obtain it.
  // if the semaphore cannot be obtained, the task is suspended and then resumed 
  // as soon as the semaphore is released.
  if(&abb_sem != 0)
  {
    NU_Obtain_Semaphore(&abb_sem, NU_SUSPEND);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  ABB_SetPage(PAGE1);
    
  #if (ANALOG == 1)  
    // This transmission initializes the VRPCCTL1 register.
    ABB_WriteRegister(VRPCCTRL1, value);  

  #elif (ANALOG == 2)
    // This transmission initializes the VRPCSIM register.
    ABB_WriteRegister(VRPCSIM, value);  

  #elif (ANALOG == 3)
    // This transmission initializes the VRPCSIMR register.
    ABB_WriteRegister(VRPCSIMR, value);  

  #endif

  ABB_SetPage(PAGE0);

  #if ((ABB_SEMAPHORE_PROTECTION == 1) || (ABB_SEMAPHORE_PROTECTION == 2) || (ABB_SEMAPHORE_PROTECTION == 3))  
  // release the semaphore only if it has correctly been created.
  if(&abb_sem != 0)
  {
    NU_Release_Semaphore(&abb_sem);
  }
  #endif  // ABB_SEMAPHORE_PROTECTION   

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif
}


/*-----------------------------------------------------------------------*/
/* ABB_Write_Uplink_Data()                                               */
/*                                                                       */
/* This function uses the SPI to write to ABB uplink buffer.             */
/*                                                                       */
/*-----------------------------------------------------------------------*/    
void ABB_Write_Uplink_Data(SYS_UWORD16 *TM_ul_data)
{
  SYS_UWORD8 i;
  volatile SYS_UWORD16 status;

  // Start spi clock, mask IT for WR and read SPI_REG_STATUS to reset the RE and WE flags.   
  SPI_Ready_for_WR
  status = * (volatile SYS_UWORD16 *) SPI_REG_STATUS; 

  // Select Page 0 for TOGBR2
  ABB_SetPage(PAGE0);

  // Initialize pointer of burst buffer 1 : IBUFPTR is bit 10 of TOGBR2
  ABB_WriteRegister(TOGBR2, 0x10);  

  // Clear, assuming that it works like IBUFPTR of Vega
  ABB_WriteRegister(TOGBR2, 0x0);  

  // Write the ramp data
  for (i=0;i<16;i++)
    ABB_WriteRegister(BULDATA1_2, TM_ul_data[i]>>6);  

  // Stop the SPI clock
  #ifdef SPI_CLK_LOW_POWER
    SPI_CLK_DISABLE
  #endif
}

//////////////////////// IDEV-INLO integration of sleep mode for Syren ///////////////////////////////////////

#if (ANALOG == 3)

  // Syren Sleep configuration function --------------------------
  void Syren_Sleep_Config(SYS_UWORD16 sleep_type,SYS_UWORD16 bg_select, SYS_UWORD16 sleep_delay)
  {
    volatile SYS_UWORD16 status,sl_ldo_stat;

    ABB_SetPage(PAGE1);                            // Initialize transmit register. This transmission
                                                   // change the register page in ABB for usp to pg1

    ABB_WriteRegister(VRPCCFG, sleep_delay);       // write delay value   

    sl_ldo_stat = ((sleep_type<<9|bg_select<<8) & 0x0374);

    ABB_WriteRegister(VRPCMSKSLP, sl_ldo_stat);     // write sleep ldo configuration   

    ABB_SetPage(PAGE0);                            // Initialize transmit register. This transmission
                                                   // change the register page in ABB for usp to pg0
  }
#endif


#if (OP_L1_STANDALONE == 0)
/*-----------------------------------------------------------------------*/
/* ABB_Power_Off()                                                       */
/*                                                                       */
/* This function uses the SPI to switch off the ABB.                     */
/*                                                                       */
/*-----------------------------------------------------------------------*/    
void ABB_Power_Off(void)
{
  // Wait until all necessary actions are performed (write in FFS, etc...) to power-off the board (empirical value - 30 ticks).
  NU_Sleep (30);

  // Wait also until <ON/OFF> key is released.
  // This is needed to avoid, if the power key is pressed for a long time, to switch
  // ON-switch OFF the mobile, until the power key is released.
  #if((ANALOG == 1) || (ANALOG == 2)) 
    while ((ABB_Read_Status() & ONREFLT) == PWR_OFF_KEY_PRESSED) {
  #elif(ANALOG == 3) 
    while ((ABB_Read_Register_on_page(PAGE1, VRPCCFG) & PWOND) == PWR_OFF_KEY_PRESSED) {
  #endif
  
  NU_Sleep (1); }

#if 0	// FreeCalypso
  BZ_KeyBeep_OFF();
#endif

  #if(ANALOG == 1) 
    ABB_Write_Register_on_page(PAGE0, VRPCCTL2, 0x00EE);
  #elif((ANALOG == 2) || (ANALOG == 3)) 
    ABB_Write_Register_on_page(PAGE0, VRPCDEV, 0x0001);
  #endif 
}
#endif
