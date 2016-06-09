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
/* Filename         : abb.h                                                   */
/*                                                                            */
/* Description      : Analog BaseBand registers and bits definition.	      */
/*                    Functions to drive the ABB device.                      */
/*                    The Serial Port Interface is used to connect the TI     */
/*		      Analog BaseBand (ABB).   				      */
/*		      It is assumed that the ABB is connected as the SPI      */
/*                    device 0.                                               */
/*									      */
/* Author           : Pascal PUEL                                             */
/*                                                                            */
/* Version number   : 1.3                                                     */
/*                                                                            */
/* Date and time    : 08/22/03						      */
/*                                                                            */
/* Previous delta   : Creation                                                */
/*                                                                            */
/******************************************************************************/

#ifndef __ABB_H__
#define __ABB_H__

#include "../../include/config.h"
#include "../../include/sys_types.h"

#ifndef _WINDOWS

/*------------------------------------*/
/* SYREN PG Definition                */
/*------------------------------------*/

#if (ANALOG == 3)   // SYREN
    #define S_PG_10      1
    #define S_PG_20      2
#endif  // (ANALOG == 3)



// DEFINITIONS FOR OMEGA/NAUSICA
#if (ANALOG == 1)
  // ABB PAGE
  #define PAGE0	0x0001
  #define PAGE1	0x0002

  // ABB REGISTERS
  //=== PAGE 0 =======
  #define PAGEREG      (1  << 1)
  #define APCDEL1      (2  << 1)
  #define BULDATA1_2   (3  << 1)
  #define TOGBR1       (4  << 1)
  #define TOGBR2       (5  << 1)
  #define VBDCTRL      (6  << 1)
  #define AUXAFC1      (7  << 1)
  #define AUXAFC2      (8  << 1)
  #define AUXAPC       (9  << 1)
  #define APCRAM       (10 << 1)
  #define APCOFF       (11 << 1)
  #define AUXDAC       (12 << 1)
  #define MADCCTRL1    (13 << 1)
  #define MADCCTRL2    (14 << 1)
  #define VBATREG      (15 << 1)
  #define VCHGREG      (16 << 1)
  #define ICHGREG      (17 << 1)
  #define VBKPREG      (18 << 1)
  #define ADIN1REG     (19 << 1)
  #define ADIN2REG     (20 << 1)
  #define ADIN3REG     (21 << 1)
  #define ADIN4XREG    (22 << 1)
  #define ADIN5YREG    (23 << 1)
  #define MADCSTAT     (24 << 1)
  #define CHGREG       (25 << 1)
  #define ITMASK       (26 << 1)
  #define ITSTATREG    (27 << 1)
  #define BCICTL1      (28 << 1)
  #define BCICTL2      (29 << 1)
  #define VRPCCTL2     (30 << 1)
  #define VRPCSTS      (31 << 1)
  
  //=== PAGE 1 =======
  #define PAGEREG      (1  << 1)
  #define BULIOFF      (2  << 1)
  #define BULQOFF      (3  << 1)
  #define BULQDAC      (4  << 1)
  #define BULIDAC      (5  << 1)
  #define BBCTRL       (6  << 1)
  #define VBUCTRL      (7  << 1)
  #define VBCTRL       (8  << 1)
  #define PWDNRG       (9  << 1)
  #define TSC_TIMER    (10 << 1)
  #define VRPCCTRL3    (11 << 1)
  #define APCOUT       (12 << 1)
  #define VRPCBGT      (18 << 1)
  #define TAPCTRL      (19 << 1)
  #define TAPREG       (20 << 1)
  #define AFCCTLADD    (21 << 1)
  #define AFCOUT       (22 << 1)
  #define VRPCCTRL1    (23 << 1)
  #define VRPCCTRL4    (24 << 1)
  #define APCDEL2      (26 << 1)
  #define ITSTATREG    (27 << 1)

  // Registers bit definitions
  // ABB device bits definition of register VBCTRL
  #define VDLAUX    0x001
  #define VDLEAR    0x002
  #define VBUZ      0x004
  #define VULSWITCH 0x008
  #define MICBIAS   0x010
  #define VALOOP    0x020
  #define VCLKMODE  0x040
  #define VSYNC     0x080
  #define VBDFAUXG  0x100
  #define VFBYP     0x200

  // ABB device bits definition of register VBUCTRL
  #define DXEN      0x200

  // ABB device bits definition of register VRPCSTS
  #define ONBSTS    0x01  // ON Button push flag
  #define ONRSTS    0x02  // Remote ON flag
  #define ITWSTS    0x04  // Wake-up IT flag
  #define CHGSTS    0x08  // Charger plug flag
  #define ONREFLT   0x10  // ON Button current state
  #define ORMRFLT   0x20  // Remote ON current state
  #define CHGPRES   0x40  // Charger plug current state

  // ABB device bits definition of register ITSTATREG
  #define REMOT_IT_STS    0x02
  #define PUSHOFF_IT_STS  0x04
  #define CHARGER_IT_STS  0x08
  #define ADCEND_IT_STS   0x20

  // On Nausica, if the PWR key is pressed, the bit is set, and cleared when released
  #define PWR_OFF_KEY_PRESSED       (ONREFLT)

  // ABB ADC Interrupts
  #define EOC_INTENA  0x03DF
  #define EOC_INTMASK 0x0020

  // ABB ADC CHANNELS
  #define VBATCV      0x0001
  #define VCHGCV      0x0002
  #define ICHGCV      0x0004
  #define VBKPCV      0x0008
  #define ADIN1CV     0x0010
  #define ADIN2CV     0x0020
  #define ADIN3CV     0x0040
  #define vADIN4XCV   0x0080
  #define ADIN5XCV    0x0100
  #define ALL         0x01FF
  #define NONE        0x0000

  // ABB MODULES
  #define MADC        0x8000
  #define AFC         0x2000
  #define ADAC        0x0800
  #define DCDC        0x0080
  #define ALLOFF      0x0000

  // Definitions of OMEGA test modes for baseband windows
  #define TSPTEST1       0x001d
  #define TSPTEST2       0x001e
  #define AFCTEST        0x0010
  #define AFCNORM        0x0000


// DEFINITIONS FOR IOTA
#elif (ANALOG == 2)
  // ABB PAGE
  #define PAGE0	0x0001
  #define PAGE1	0x0002
  #define PAGE2	0x0010

  // ABB REGISTERS
  //=== PAGE 0 =======
  #define PAGEREG      (1  << 1)
  #define APCDEL1      (2  << 1)
  #define BULDATA1_2   (3  << 1)
  #define TOGBR1       (4  << 1)
  #define TOGBR2       (5  << 1)
  #define VBDCTRL      (6  << 1)
  #define AUXAFC1      (7  << 1)
  #define AUXAFC2      (8  << 1)
  #define AUXAPC       (9  << 1)
  #define APCRAM       (10 << 1)
  #define APCOFF       (11 << 1)
  #define AUXDAC       (12 << 1)
  #define MADCCTRL     (13 << 1)
  #define VBATREG      (15 << 1)
  #define VCHGREG      (16 << 1)
  #define ICHGREG      (17 << 1)
  #define VBKPREG      (18 << 1)
  #define ADIN1REG     (19 << 1)
  #define ADIN2REG     (20 << 1)
  #define ADIN3REG     (21 << 1)
  #define ADIN4REG     (22 << 1)
  #define MADCSTAT     (24 << 1)
  #define CHGREG       (25 << 1)
  #define ITMASK       (26 << 1)
  #define ITSTATREG    (27 << 1)
  #define BCICTL1      (28 << 1)
  #define BCICTL2      (29 << 1)
  #define VRPCDEV      (30 << 1)
  #define VRPCSTS      (31 << 1)
								
  //=== PAGE 1 =======		
  #define PAGEREG      (1  << 1)
  #define BULIOFF      (2  << 1)
  #define BULQOFF      (3  << 1)
  #define BULQDAC      (4  << 1)
  #define BULIDAC      (5  << 1)
  #define BBCTRL       (6  << 1)
  #define VBUCTRL      (7  << 1)
  #define VBCTRL1      (8  << 1)
  #define PWDNRG       (9  << 1)
  #define VBPOP        (10 << 1)
  #define VBCTRL2      (11 << 1)
  #define APCOUT       (12 << 1)
  #define BCICONF      (13 << 1)
  #define BULGCAL      (14 << 1)
  #define TAPCTRL      (19 << 1)
  #define TAPREG       (20 << 1)
  #define AFCCTLADD    (21 << 1)
  #define AFCOUT       (22 << 1)
  #define VRPCSIM      (23 << 1)
  #define AUXLED       (24 << 1)
  #define APCDEL2      (26 << 1)
  #define ITSTATREG    (27 << 1)
  #define VRPCMSKABB   (29 << 1)
  #define VRPCCFG      (30 << 1)
  #define VRPCMSK      (31 << 1)

  // Registers bit definitions
  // ABB device bits definition of register VBCTRL1
  #define VDLAUX    0x001
  #define VDLEAR    0x002
  #define VBUZ      0x004
  #define VULSWITCH 0x008
  #define MICBIAS   0x010
  #define VALOOP    0x020
  #define VCLKMODE  0x040
  #define VSYNC     0x080
  #define VBDFAUXG  0x100
  #define VFBYP     0x200
 
  // ABB device bits definition of register VBCTRL2
  #define MICBIASEL 0x001
  #define VDLHSO    0x002
  #define MICNAUX   0x004

  // ABB device bits definition of register VBUCTRL
  #define DXEN      0x200

  // ABB device bits definition of register VBPOP
  #define HSODIS    0x001
  #define HSOCHG    0x002
  #define HSOAUTO   0x004
  #define EARDIS    0x008
  #define EARCHG    0x010
  #define EARAUTO   0x020
  #define AUXDIS    0x040
  #define AUXCHG    0x080
  #define AUXAUTO   0x100

  // ABB device bits definition of register VRPCSTS
  #define ONBSTS    0x01  // ON Button push flag
  #define ONRSTS    0x02  // Remote ON flag
  #define ITWSTS    0x04  // Wake-up IT flag
  #define CHGSTS    0x08  // Charger plug flag
  #define ONREFLT   0x10  // ON Button current state
  #define ORMRFLT   0x20  // Remote ON current state
  #define CHGPRES   0x40  // Charger plug current state

  // ABB device bits definition of register ITSTATREG
  #define REMOT_IT_STS    0x02
  #define PUSHOFF_IT_STS  0x04
  #define CHARGER_IT_STS  0x08
  #define ADCEND_IT_STS   0x20

  // On Iota, the bit is set when the key is released and set when the key is pressed
  #define PWR_OFF_KEY_PRESSED      (0)

  // ABB ADC Interrupts
  #define EOC_INTENA  0x03DF
  #define EOC_INTMASK 0x0020

  // ABB ADC CHANNELS
  #define VBATCV      0x0001
  #define VCHGCV      0x0002
  #define ICHGCV      0x0004
  #define VBKPCV      0x0008
  #define ADIN1CV     0x0010
  #define ADIN2CV     0x0020
  #define ADIN3CV     0x0040
  #define ADIN4CV     0x0080
  #define ALL         0x00FF
  #define NONE        0x0000

  // ABB MODULES
  #define MADC        0x8000
  #define AFC         0x2000
  #define ADAC        0x0800
  #define DCDC        0x0080
  #define ALLOFF      0x0000

  // Definitions of IOTA test modes
  #define TSPTEST1      0x001d
  #define TSPTEST2      0x001e
  #define AFCTEST       0x0010
  #define AFCNORM       0x0000

  // Definition for IOTA test modes
  #define TSPEN         0x001a
  #define MADCTEST      0x0012
  #define TSPADC        0x0015
  #define TSPUP         0x0017
  #define TSPDN         0x0018

  // Definition for IOTA Power Management

  //The duration of the SLPDLY counter must be greater than the process execution time:
  //DBB deep sleep routine included the IT check = DBB sleep routine and IT check
  //+ the seven 32Khz clock cycle interval needed to ABB in order to make effective the sleep abort write access in VRPCDEV    ++  //  register -> 7*T32Khz = = ABB IBIC propagation delay 
  //+ 150us of short asynchronous wake-up time  (approximately 4*T32Khz) = ULPD short sleep where Syren/IOTA aborts sleep and 
  //  write DEVSLEEP = 0  




  #define SLPDLY                0x001F    // delay to set IOTA in sleep mode (unit: 20*T32Khz)
  #define MASK_SLEEP_MODE       0x0000    // set the regulators in low consumption in sleep mode


// DEFINITIONS FOR SYREN
#elif (ANALOG == 3)

  // ABB PAGE
  #define PAGE0	0x0001
  #define PAGE1	0x0002
  #define PAGE2	0x0010

  // ABB REGISTERS
  //=== PAGE 0 =======
  #define PAGEREG     (1  << 1)
  #define APCDEL1     (2  << 1)
  #define BULDATA1_2  (3  << 1)
  #define TOGBR1      (4  << 1)
  #define TOGBR2      (5  << 1)
  #define VBDCTRL     (6  << 1)
  #define AUXAFC1     (7  << 1)
  #define AUXAFC2     (8  << 1)
  #define AUXAPC      (9  << 1)
  #define APCRAM      (10 << 1)
  #define APCOFF      (11 << 1)
  #define AUXDAC      (12 << 1)
  #define MADCCTRL    (13 << 1)
  #define CHGIREG     (14 << 1)
  #define VBATREG     (15 << 1)
  #define VCHGREG     (16 << 1)
  #define ICHGREG     (17 << 1)
  #define VBKPREG     (18 << 1)
  #define ADIN1REG    (19 << 1)
  #define ADIN2REG    (20 << 1)
  #define ADIN3REG    (21 << 1)
  #define ADIN4REG    (22 << 1)
  #define ADIN5REG    (23 << 1)
  #define MADCSTAT    (24 << 1)
  #define CHGVREG     (25 << 1)
  #define ITMASK      (26 << 1)
  #define ITSTATREG   (27 << 1)
  #define BCICTL1     (28 << 1)
  #define BCICTL2     (29 << 1)
  #define VRPCDEV     (30 << 1)
  #define VRPCSTS     (31 << 1)

  //=== PAGE 1 =======
  #define PAGEREG     (1  << 1)
  #define BULIOFF     (2  << 1)
  #define BULQOFF     (3  << 1)
  #define BULQDAC     (4  << 1)
  #define BULIDAC     (5  << 1)
  #define BBCTRL      (6  << 1)
  #define VBUCTRL     (7  << 1)
  #define VBCTRL1     (8  << 1)
  #define PWDNRG      (9  << 1)
  #define VBPOP       (10 << 1)
  #define VBCTRL2     (11 << 1)
  #define APCOUT      (12 << 1)
  #define BCICONF     (13 << 1)
  #define BULGCAL     (14 << 1)
  #define VAUDCTRL    (15 << 1)
  #define VAUSCTRL    (16 << 1)
  #define VAUOCTRL    (17 << 1)
  #define VAUDPLL     (18 << 1)
  #define TAPCTRL     (19 << 1)
  #define TAPREG      (20 << 1)
  #define AFCCTLADD   (21 << 1)
  #define AFCOUT      (22 << 1)
  #define VRPCSIMR    (23 << 1)
  #define BCIWDOG     (24 << 1)
  #define NONE8       (25 << 1)
  #define APCDEL2     (26 << 1)
  #define ITSTATREG   (27 << 1)

#if (ANLG_PG == S_PG_20)                           // SYREN PG2.0 ON EVACONSO
  #define BBCFG       (28 << 1)
#else
  #define NONE9       (28 << 1)
#endif

  #define VRPCMSKOFF  (29 << 1)
  #define VRPCCFG     (30 << 1)
  #define VRPCMSKSLP  (31 << 1)
  
  //=== PAGE 2 =======

#if (ANLG_PG == S_PG_10)       // SYREN PG1.0 ON ESAMPLE
  #define BBCFG       (5 << 1)
#endif

  #define VRPCABBTST  (25 << 1)
  #define VRPCAUX     (30 << 1)
  #define VRPCLDO     (31 << 1)

/* INSERT HERE OTHER DEVICES REGISTERS */


  // Registers bit definitions
  /*** SYREN internal control bits ***/

  /** For reg. VBCTRL1 **/      
  #define VULSWITCH 0x008
  #define MICBIAS   0x010
  #define VALOOP    0x020
  #define VCLKMODE  0x040
  #define VSYNC     0x080
  #define VBDFAUXG  0x100
  #define VFBYP     0x200
 
  /** For reg. VBCTRL2 **/      
  #define HSMICSEL  0x001
  #define MICBIASEL 0x004
  #define SPKG      0x008
  #define HSOVMID   0x010
  #define HSDIF     0x020

  /** For reg. VBUCTRL **/      
  #define DXEN      0x200

  /** For reg. VBPOP **/      
  #define HSODIS    0x001
  #define HSOCHG    0x002
  #define HSOAUTO   0x004
  #define EARDIS    0x008
  #define EARCHG    0x010
  #define EARAUTO   0x020
  #define AUXFDIS   0x040
  #define AUXAUTO   0x080
  #define AUXFBYP   0x200

  // ABB device bits definition of register VRPCCFG
  #define PWOND     0x20  // ON Button current state
  #define CHGPRES   0x40

  // ABB device bits definition of register ITSTATREG
  #define REMOT_IT_STS    0x02
  #define PUSHOFF_IT_STS  0x04
  #define CHARGER_IT_STS  0x08
  #define ADCEND_IT_STS   0x20

  // ABB device bits definition of register VRPCSTS
  #define ITWSTS    0x10   // Wake-up IT flag
  #define PWONBSTS  0x20   // ON Button push flag
  #define CHGSTS    0x40   // Charger plug flag
  #define RPSTS     0x100  // Remote ON flag

  // ABB ADC Interrupts
  #define EOC_INTENA  0x03DF
  #define EOC_INTMASK 0x0020

  // ABB ADC CHANNELS (reg. MADCCTRL)
  #define VBATCV    0x0001
  #define VCHGCV    0x0002
  #define ICHGCV    0x0004
  #define VBKPCV    0x0008
  #define ADIN1CV   0x0010
  #define ADIN2CV   0x0020
  #define ADIN3CV   0x0040
  #define ADIN4CV   0x0080
  #define ADIN5CV   0x0100
  #define ALL       0x01FF
  #define NONE      0x0000

  // ABB MODULES
  #define MADC        0x8000
  #define AFC         0x2000
  #define ADAC        0x0800
  #define DCDC        0x0080
  #define ALLOFF      0x0000

  // Definitions of SYREN test modes
  #define TSPTEST1      0x001d
  #define TSPTEST2      0x001e
  #define AFCTEST       0x0010
  #define AFCNORM       0x0000

  #define TSPEN         0x001a
  #define MADCTEST      0x0012
  #define TSPADC        0x0015
  #define TSPUP         0x0017
  #define TSPDN         0x0018

  // Definition for SYREN Power Management

  //The duration of the SLPDLY counter must be greater than the process execution time:
  //DBB deep sleep routine included the IT check = DBB sleep routine and IT check
  //+ the seven 32Khz clock cycle interval needed to ABB in order to make effective the sleep abort write access in VRPCDEV    ++  //  register -> 7*T32Khz = = ABB IBIC propagation delay 
  //+ 150us of short asynchronous wake-up time  (approximately 4*T32Khz) = ULPD short sleep where Syren/IOTA aborts sleep and 
  //  write DEVSLEEP = 0  

  #define SLPDLY             0x001F    // delay to set SYREN in sleep mode (unit: 20*T32Khz)
  #define MASK_SLEEP_MODE    0x0000    // set the regulators in low consumption in sleep mode

  #define LOCORE_SLEEP   0x01
  #define NORMAL_SLEEP   0x00

  #define MAIN_BG        0x01,
  #define SLEEP_BG       0x00

#endif	  // ANALOG == 1,2,3


// Define the level of semaphore protection for all accesses to the ABB
// 0 for no protection
// 1 for protection low 
// 2 for protection medium 
// 3 for protection high 
#if (OP_L1_STANDALONE == 1)
#define ABB_SEMAPHORE_PROTECTION     (0)
#else
#define ABB_SEMAPHORE_PROTECTION     (1)
#endif



// PROTOTYPES
#if (ABB_SEMAPHORE_PROTECTION)   
  void ABB_Sem_Create(void);
#endif
void ABB_Wait_IBIC_Access(void);
void ABB_Write_Register_on_page(SYS_UWORD16 page, SYS_UWORD16 reg_id, SYS_UWORD16 value);
SYS_UWORD16 ABB_Read_Register_on_page(SYS_UWORD16 page, SYS_UWORD16 reg_id);
void ABB_free_13M(void);
void ABB_stop_13M(void);
SYS_UWORD16 ABB_Read_Status(void);
void ABB_Conf_ADC(SYS_UWORD16 Channels, SYS_UWORD16 ItVal);                              
void ABB_Read_ADC(SYS_UWORD16 *Buff);
void ABB_on(SYS_UWORD16 modules, SYS_UWORD8 bRecoveryFlag);
SYS_UWORD32 ABB_sleep(SYS_UWORD8 sleep_performed, SYS_WORD16 afc);
void ABB_wakeup(SYS_UWORD8 sleep_performed, SYS_WORD16 afc);
void ABB_wa_VRPC(SYS_UWORD16 value);
void ABB_Write_Uplink_Data(SYS_UWORD16 *TM_ul_data);
#if (OP_L1_STANDALONE == 0)
void ABB_Power_Off(void);
#endif
#if (ANALOG ==3)
  void Syren_Sleep_Config(SYS_UWORD16 sleep_type,SYS_UWORD16 bg_select, SYS_UWORD16 sleep_delay);
#endif

#else    // _WINDOWS
// DEFINITIONS FOR IOTA
  // ABB PAGE
  #define PAGE0	0x0001
  #define PAGE1	0x0002
  #define PAGE2	0x0010

  // ABB REGISTERS
  //=== PAGE 0 =======
  #define PAGEREG      (1  << 1)
  #define APCDEL1      (2  << 1)
  #define BULDATA1_2   (3  << 1)
  #define TOGBR1       (4  << 1)
  #define TOGBR2       (5  << 1)
  #define VBDCTRL      (6  << 1)
  #define AUXAFC1      (7  << 1)
  #define AUXAFC2      (8  << 1)
  #define AUXAPC       (9  << 1)
  #define APCRAM       (10 << 1)
  #define APCOFF       (11 << 1)
  #define AUXDAC       (12 << 1)
  #define MADCCTRL     (13 << 1)
  #define VBATREG      (15 << 1)
  #define VCHGREG      (16 << 1)
  #define ICHGREG      (17 << 1)
  #define VBKPREG      (18 << 1)
  #define ADIN1REG     (19 << 1)
  #define ADIN2REG     (20 << 1)
  #define ADIN3REG     (21 << 1)
  #define ADIN4REG     (22 << 1)
  #define MADCSTAT     (24 << 1)
  #define CHGREG       (25 << 1)
  #define ITMASK       (26 << 1)
  #define ITSTATREG    (27 << 1)
  #define BCICTL1      (28 << 1)
  #define BCICTL2      (29 << 1)
  #define VRPCDEV      (30 << 1)
  #define VRPCSTS      (31 << 1)
								
  //=== PAGE 1 =======		
  #define PAGEREG      (1  << 1)
  #define BULIOFF      (2  << 1)
  #define BULQOFF      (3  << 1)
  #define BULQDAC      (4  << 1)
  #define BULIDAC      (5  << 1)
  #define BBCTRL       (6  << 1)
  #define VBUCTRL      (7  << 1)
  #define VBCTRL1      (8  << 1)
  #define PWDNRG       (9  << 1)
  #define VBPOP        (10 << 1)
  #define VBCTRL2      (11 << 1)
  #define APCOUT       (12 << 1)
  #define BCICONF      (13 << 1)
  #define BULGCAL      (14 << 1)
  #define TAPCTRL      (19 << 1)
  #define TAPREG       (20 << 1)
  #define AFCCTLADD    (21 << 1)
  #define AFCOUT       (22 << 1)
  #define VRPCSIM      (23 << 1)
  #define AUXLED       (24 << 1)
  #define APCDEL2      (26 << 1)
  #define ITSTATREG    (27 << 1)
  #define VRPCMSKABB   (29 << 1)
  #define VRPCCFG      (30 << 1)
  #define VRPCMSK      (31 << 1)

  // ABB device bits definition of register VBUCTRL
  #define DXEN      0x200

  // ABB device bits definition of register VRPCSTS
  #define ONBSTS    0x01  // ON Button push flag
  #define ONRSTS    0x02  // Remote ON flag
  #define ITWSTS    0x04  // Wake-up IT flag
  #define CHGSTS    0x08  // Charger plug flag
  #define ONREFLT   0x10  // ON Button current state
  #define ORMRFLT   0x20  // Remote ON current state
  #define CHGPRES   0x40  // Charger plug current state

  // ABB device bits definition of register ITSTATREG
  #define REMOT_IT_STS    0x02
  #define PUSHOFF_IT_STS  0x04
  #define CHARGER_IT_STS  0x08
  #define ADCEND_IT_STS   0x20


// PROTOTYPES
void ABB_Write_Register_on_page(SYS_UWORD16 page, SYS_UWORD16 reg_id, SYS_UWORD32 value);
SYS_UWORD16 ABB_Read_Register_on_page(SYS_UWORD16 page, SYS_UWORD16 reg_id);
SYS_UWORD16 ABB_Read_Status(void);
void ABB_Conf_ADC(SYS_UWORD16 Channels, SYS_UWORD16 ItVal);                              
void ABB_Read_ADC(SYS_UWORD16 *Buff);

#endif  // _WINDOWS

#endif // __ABB_H__
