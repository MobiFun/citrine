/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 *
 *        Filename tpudrv.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

/*
 * Device addresses - GCS000
 */ 
// GSM 1.5 : TPU / TSP addresses 
//-------------------------------------

#if ((CHIPSET ==2) || (CHIPSET == 3) || (CHIPSET == 4))
  #define TPU_ADDR      0xFFFE0000l            // Hercule / Ulysse / Samson

  #define TPU_RAM       (TPU_ADDR + 0x1400)

  #define TPU_REG       (TPU_ADDR + 0x1000)
  #define TSP_REG       (TPU_ADDR + 0x0800)
  #define TPU_TIM       (TPU_ADDR + 0x2000)

#elif ((CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7)  || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define TPU_ADDR      0xFFFF0000l            // Strobe 1 address

  #define TPU_RAM       0xFFFF9000l            // TPU RAM

  #define TPU_REG       (TPU_ADDR + 0x1000)    // TPU register
  #define TSP_REG       0xFFFE0800l            // TSP register
  #define TPU_TIM       0xFFFE2000l            // ULPD register

#endif


/*
 * Macros for defining TPU instructions
 */ 
#define TPU_SLEEP             0
#define TPU_MOVE(addr,data)   (0x8000 | ((data)<<5) | (addr))
#define TPU_AT(time)          (0x2000 | (((time + 5000) % 5000)))
#define TPU_FAT(time)         (0x2000 | (time))  // Fast version without modulo
#define TPU_SYNC(time)        (0x6000 | (time))
#define TPU_WAIT(time)        (0xA000 | (time))
#define TPU_OFFSET(time)      (0x4000 | (time))
#define MOD5000(a)            (((a) + 5000) % 5000)


/*
 * TSP registers - defined in GCS004 - Time Serial Port
 */ 
/*
 *  in TPU address space
 */
// GSM 1.5 : TSP_TX_REG_1/2/3/4 instead of TSP_TX_U/M/L
//           added TSP_SPI_SET1/2/3 to ctrl up to 5 periph. 
//-----------------------------------------------------
  #define TSP_CTRL1       0x00
  #define TSP_CTRL2       0x01
  #define TSP_TX_REG_1    0x04
  #define TSP_TX_REG_2    0x03
  #define TSP_TX_REG_3    0x02
  #define TSP_TX_REG_4    0x05
  #define TSP_ACT         0x06
  #define TSP_ACTX        0x07
  #define TSP_GAUGING_EN  0x11
  #define TSP_SPI_SET1    0x09
  #define TSP_SPI_SET2    0x0A 
  #define TSP_SPI_SET3    0x0B
  #define TPU_IT_DSP_PG   0x10
  #define TSP_GAUGING_EN  0x11

/*
 *  in ARM address space - defined in HYP004
 */
  #define TSP_RX_LSB   (TSP_REG + 0x00)

  #define TSP_RX_MSB   (TSP_REG + 0x02)

  #define TSP_TX_LSB   (TSP_REG + 0x0c)

  #define TSP_TX_MSB   (TSP_REG + 0x0a)


/*
 * TSP registers bit definitions
 */ 
  #define TC1_DEVICE0   0x00
  #define TC1_DEVICE1   0x20
  #define TC1_DEVICE2   0x40
  #define TC1_DEVICE3   0x60
  #define TC1_DEVICE4   0x80
  #define TC2_RD        0x01
  #define TC2_WR        0x02
  #define TC2_EDGE_TRIG 0x40
  #define TC2_RISING    0x80
  #define TSP_CLK_RISE  0x01
  #define TSP_ENA_POS   0x02
  #define TSP_ENA_EDGE  0x04
  #define GAUGING_START 0x01
  #define GAUGING_STOP  0x00
  #define TSP_ENA_POS_MSB 0x20



/*
 * TPU registers - defined in HYP002
 */ 
// GSM 1.5 : TPU reg are 16-bit access
//---------------------------------------
#define TPU_CTRL          (TPU_REG + 0x00)
#define TPU_INT_CTRL      (TPU_REG + 0x02)
#define TPU_INT_STAT      (TPU_REG + 0x04)
#define TPU_OFFSET_REG    (TPU_REG + 0x0C)
#define TPU_SYNCHRO_REG   (TPU_REG + 0x0E)
#define TPU_DSP_PG        (TPU_REG + 0x20)



 

/*
 * TPU control register bits
 */ 
// GSM 1.5 : TPU bits changed
//---------------------------------------
#define TPU_CTRL_RESET     0x0001 
#define TSP_CTRL_RESET     0x0080
#define TPU_CTRL_T_PAGE    0x0002
#define TPU_CTRL_T_ENBL    0x0004
#define TPU_CTRL_D_ENBL    0x0010       // WARNING THIS BIT DOES NOT EXIST IN HYPERION
#define TPU_CTRL_SPI_RST   0x0080
#define TPU_CTRL_WAIT      0x0200
#define TPU_CTRL_CLK_EN    0x0400
#if (CHIPSET == 7)  || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12)
  #define TPU_CTRL_FULL_WRITE 0x0800
#endif




/* 
 * TPU interrupt control register bits
 */


/* WARNING BUG IN HYPERION. */
/* READING TPU_INT_CRTL, TPU_INT_ITP_M BIT CONTENTS AFFECTS THE TPU_INT_ITD_M VALUE. */ 

#define TPU_INT_ITF_M      0x0001
#define TPU_INT_ITP_M      0x0002         
#define TPU_INT_ITD_M      0x0004
#define TPU_INT_ITD_F      0x0008        // WARNING THIS BIT DOES NOT EXIST IN HYPERION

#define INT_FRAME   4       /* TPU frame interrupt */   
#define INT_PAGE    5       /* TPU page interrupt */        
#define INT_TSP 3       /* TSP interrupt */ 



#if ((ANLG_FAM == 1) || (ANLG_FAM == 2) || (ANLG_FAM == 3))
    // BB signals connected to serial link1
    #define BULON    0x80     // bit6
    #define BULCAL   0x40     // bit5
    #define BULENA   0x20     // bit4
    #define BDLON    0x10     // bit3
    #define BDLCAL   0x08     // bit2
    #define BDLENA   0x04     // bit1
    #define STARTADC 0x02     // bit0
#endif


/* 
 * GSM RF programming times in quarter bits
 */
/**************************************************************************/
/**************************************************************************/
/****************************** W A R N I N G !!! *************************/
/******* This values are fine tuned for LAYER 1 . DO NOT MODIFY !!! *******/
/****** FOR ANY MODIFICATION , PLEASE CONTACT Texas Instruments Inc. ******/
/**************************************************************************/
/**************************************************************************/


/**************************************/
/* TPU Macros: prototypes functions   */
/**************************************/
// TPU macros.
//------------
void l1dmacro_reset_hw        (UWORD32 servingCellOffset);
void l1dmacro_init_hw         (void);
void l1dmacro_init_hw_light   (void);
void l1dmacro_idle            (void);                       
void l1dmacro_rx_synth        (SYS_UWORD16  radio_freq);
void l1dmacro_tx_synth        (SYS_UWORD16  radio_freq);
void l1dmacro_agc             (SYS_UWORD16  radio_freq, WORD8 gain, UWORD8 lna);
void l1dmacro_afc             (SYS_UWORD16 afc_value, UWORD8 win_id);
void l1dmacro_rx_ms           (SYS_UWORD16 radio_freq);
void l1dmacro_rx_fb           (SYS_UWORD16 radio_freq);
void l1dmacro_rx_fb26         (SYS_UWORD16 radio_freq);
void l1dmacro_offset          (UWORD32     offset_value, 
                               WORD32      relative_time);
void l1dmacro_synchro         (UWORD32 when, UWORD32 value);
void l1dmacro_rx_sb           (SYS_UWORD16 radio_freq);
void l1dmacro_rx_nb           (SYS_UWORD16 radio_freq);
void l1dmacro_tx_nb           (SYS_UWORD16 radio_freq, UWORD8 txpwr, UWORD8 adc_active);
void l1dmacro_tx_ra           (SYS_UWORD16 radio_freq, UWORD8 txpwr, UWORD8 adc_active);
void l1dmacro_adc_read_tx     (UWORD32 when);
void l1dmacro_adc_read_rx     (void);
void l1dmacro_set_frame_it    (void);

void l1pdmacro_it_dsp_gen(WORD16 time);

#if TESTMODE
  void l1dmacro_rx_cont (SYS_UWORD16 radio_freq, UWORD8 txpwr);
  void l1dmacro_tx_cont (SYS_UWORD16 radio_freq, UWORD8 txpwr);
  void l1dmacro_stop_cont (void);
#endif

/*
 * TPU prototypes
 */ 
void TP_PageIntHandler  (void);
void TP_FrameIntHandler (void);
void TP_PageIntHandler  (void);
void TP_FrameIntHandler (void);
void TPU_Reset(SYS_UWORD16 on);
void TSP_Reset(SYS_UWORD16 on);
void TPU_ClkEnable(SYS_UWORD16 on);
void TP_Reset(SYS_UWORD16 on);
void TP_Enable(SYS_UWORD16 on);
BOOL TPU_check_IT_DSP(void);


/*
 * TPUDRV global variables
 */ 
#ifdef TPUDRV_C
#define TP_GLOBAL 
#else
#define TP_GLOBAL extern
#endif

TP_GLOBAL volatile UWORD32 TP_PageInt;
TP_GLOBAL volatile UWORD32 TP_FrameInt;


