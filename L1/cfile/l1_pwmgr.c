 /************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_PWMGR.C
 *
 *        Filename l1_pwmgr.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

// pinghua add these programe code section to put some sleep code into internal ram.
/*
 * FreeCalypso: the Leonardo binary object version puts all of l1_pwmgr
 * into the regular run-from-flash text section, so we'll do the same
 * for now.
 */
#if 0
#pragma CODE_SECTION(l1s_sleep_manager,".emifconf")
#pragma CODE_SECTION(EMIF_SetConfReg,".emifconf")
#pragma CODE_SECTION(audio_madc_sleep,".emifconf")
#pragma CODE_SECTION(Check_Peripheral_App,".emifconf")
#pragma CODE_SECTION(DBB_Configure_DS,".emifconf")
#pragma CODE_SECTION(DBB_Wakeup_DS,".emifconf")
#pragma CODE_SECTION(l1ctl_pgm_clk32,".emifconf")
#pragma CODE_SECTION(l1ctl_gauging,".emifconf")
#pragma CODE_SECTION(GAUGING_Handler,".emifconf")
#pragma CODE_SECTION(l1s_get_HWTimers_ticks,".emifconf")
#pragma CODE_SECTION(l1s_adapt_traffic_controller,".emifconf")
#pragma CODE_SECTION(l1s_wakeup,".emifconf")
#pragma CODE_SECTION(l1s_wakeup_adjust,".emifconf")
#pragma CODE_SECTION(l1s_compute_wakeup_ticks,".emifconf")
#pragma CODE_SECTION(l1s_recover_Frame,".emifconf")
#pragma CODE_SECTION(l1s_recover_HWTimers,".emifconf")
#pragma CODE_SECTION(l1s_get_next_gauging_in_Packet_Idle,".emifconf")
#pragma CODE_SECTION(l1s_gauging_decision_with_PNP,".emifconf")
#pragma CODE_SECTION(l1s_gauging_decision_with_NP,".emifconf")
#pragma CODE_SECTION(l1s_gauging_task,".emifconf")
#pragma CODE_SECTION(l1s_gauging_task_end,".emifconf")
// 2-03-2007 pinghua added end
#endif

#define  L1_PWMGR_C
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

#include "config.h"
#include "l1_confg.h"

//sajal added .....................................
  #if (CODE_VERSION == SIMULATION)
  //#include "l1_pwmgr.h"
//omaps00090550 #303 warning removal 	typedef unsigned char  UWORD_8;

//	typedef volatile unsigned short REG_UWORD16; //omaps00090550
//	#define REG16(A)    (*(REG_UWORD16*)(A))  //omaps00090550
//	typedef volatile unsigned short REGISTER_UWORD16;  //omaps00090550

	#define MAP_ULPD_REG	                0xFFFE2000	//ULPD registers start address            (CS4)
	#define ULPD_SETUP_CLK13_REG   	       (*(REGISTER_UWORD16*)((REGISTER_UWORD16 *)(MAP_ULPD_REG) + 14))
	#define ULPD_SETUP_SLICER_REG	       (*(REGISTER_UWORD16*)((REGISTER_UWORD16 *)(MAP_ULPD_REG) + 15))
	#define ULPD_SETUP_VTCXO_REG           (*(REGISTER_UWORD16*)((REGISTER_UWORD16 *)(MAP_ULPD_REG) + 16))

	#define MAP_CLKM_REG	          0xFFFFFD00	 //CLOCKM         registers start address  (CS31)
	#define CLKM_CNTL_CLK_OFFSET         0x02
	#define CLKM_CNTL_CLK_REG            REG16 (MAP_CLKM_REG + CLKM_CNTL_CLK_OFFSET)

	#define EMIF_CONFIG_PWD_POS                      0
	#define EMIF_CONFIG_PDE_POS                      1
	#define EMIF_CONFIG_PREFETCH_POS                3
	#define EMIF_CONFIG_FLUSH_PREFETCH_POS          5
	#define EMIF_CONFIG_WP_POS                      6

	#define            EMIF_CONFIG                              REG16(EMIF_CONFIG_BASE_ADDR+EMIF_CONFIG_REG_OFFSET)
	#define EMIF_CONFIG_BASE_ADDR      0xFFFFFB00      //External Memory inter registers address (CS31)   (NEW)
	#define EMIF_CONFIG_REG_OFFSET          0x02    // Emif configuration register

#endif
//sajal added till here......



#include "../../bsp/timer2.h"
#include "../../bsp/armio.h"
#include "../../serial/serialswitch.h"

#if 0 //(OP_L1_STANDALONE == 0)
  #include "sim/sim.h"
  #include "rv_swe.h"
#endif


#if (CODE_VERSION == SIMULATION)
  #include "l1_types.h"
  #include "l1_const.h"

  #if (CHIPSET == 12) || (CHIPSET == 15)
    #include "inth/sys_inth.h"
    #include "sys_dma.h"
	#include "ulpd.h"
	#include "clkm.h"

//	typedef volatile unsigned short REG_UWORD16;  //omaps00090550
	#define REG16(A)    (*(REG_UWORD16*)(A))

  #else
    #include "inth/iq.h"
  #endif

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif // TESTMODE

  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif // AUDIO_TASK

  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif

  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
  #endif

  #if (L1_MIDI == 1)
    #include "l1midi_defty.h"
  #endif
//ADDED FOR AAC
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "l1_varex.h"
  #include "l1_tabs.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include "l1_proto.h"
  #include "ulpd.h"
  #include "l1_trace.h"

  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
  #endif // L1_GPRS

  #include <stdio.h>
  #include "sim_cfg.h"
  #include "sim_cons.h"
  #include "sim_def.h"
  #include "sim_var.h"
//omaps00090550
  #include "nucleus.h"

  extern NU_TASK L1S_task;
  STATUS status;



#else // NO SIMULATION



  #include "l1_types.h"
  #include "l1_const.h"

  #include "../../bsp/abb+spi/abb.h"
  /* #include "dma/sys_dma.h" */

  #if (OP_BT == 1)
    #include "hci_ll_simul.h"
  #endif

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif // TESTMODE

  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif // AUDIO_TASK

  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif

  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
  #endif

  #if (L1_MIDI == 1)
    #include "l1midi_defty.h"
  #endif
//ADDED FOR AAC
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "l1_varex.h"
  #include "l1_tabs.h"
  #include "sys_types.h"
  #include "tpudrv.h"
  #include "../../gpf/inc/cust_os.h"
  #include "l1_msgty.h"
  #include "l1_proto.h"
  #include "l1_trace.h"
  #include "../../bsp/timer.h"

  #include "l1_pwmgr.h"

  #if (CHIPSET == 12) || (CHIPSET == 15)
    #include "timer/timer_sec.h"
    #include "inth/sys_inth.h"



  #if(CHIPSET == 15)
	#include "l1_pwmgr.h"
  #if (OP_L1_STANDALONE == 0)
        #include "lcc/lcc_api.h"
  #endif

	/* If NAND is enabled */
	#if defined(RVM_DATALIGHT_SWE) || defined(RVM_NAN_SWE)
		unsigned int temp_NAND_Reg1;
		unsigned int temp_NAND_Reg2;
		unsigned int temp_NAND_Reg3;
	#endif





	#if (OP_L1_STANDALONE == 1)

	       const 	t_peripheral_interface  Peripheral_interface [MAX_PERIPHERAL]=
		{
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			madc_outen_check,             /* MADC_AS_ID = 8 */
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
		};

		const t_application_interface  Application_interface [MAX_APPLICATIONS] =
		{
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
		};
	#else  // For integrated Build
		const t_peripheral_interface  Peripheral_interface [MAX_PERIPHERAL]=
		{
			uart_pwr_interface,
#ifdef RVM_USB_SWE
			usb_pwr_interface,
#else
            f_peripheral_interface_dummy,
#endif
			usim_pwr_interface,
			i2c_pwr_interface,
			lcd_pwr_interface,
#ifdef RVM_CAMD_SWE
#if (OP_L1_STANDALONE == 0)
				camera_pwr_interface,
#endif
#else
                f_peripheral_interface_dummy,
#endif
			backlight_pwr_interface,
			f_peripheral_interface_dummy,
			audio_madc_sleep,				/* MADC_AS_ID = 8 */
			lcc_pwr_interface,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
			f_peripheral_interface_dummy,
		};

		const t_application_interface  Application_interface [MAX_APPLICATIONS] =
		{
#ifdef BTS

			BTHAL_PM_HandleSleepManagerReq,
#else
			f_application_interface_dummy,
#endif
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
			f_application_interface_dummy,
		};


	   #endif // (OP_L1_STANDALONE == 1)

    #endif // omaps00090550 #14 -d removal (CHIPSET = 15)


  #else  //(CHIPSET == 12) || (CHIPSET == 15)
    #include "../../bsp/iq.h"
    #include "../../bsp/inth.h"
  #endif
//  #include "timer1.h"
  #include "../../bsp/ulpd.h"
  #include "../../bsp/clkm.h"
  #include "../../bsp/mem.h"
  #if L2_L3_SIMUL
    #include "hw_debug.h"
  #endif

  #if (OP_WCP == 1) && (OP_L1_STANDALONE != 1)
    #include "csmi/sleep.h"
  #endif // OP_WCP
  #if (W_A_CALYPSO_PLUS_SPR_19599 == 1)
    #include "sys_memif.h"
  #endif

#if (GSM_IDLE_RAM != 0)
#if (OP_L1_STANDALONE == 1)
#include "csmi_simul.h"
#else
#include "csmi/csmi.h"
#endif
#endif

#if (CHIPSET == 15)
  #include "drp_api.h"
#endif

#endif // NO SIMULATION

#if (CODE_VERSION != SIMULATION)
  // for PTOOL compatibility
  extern void INT_DisableIRQ(void);
  extern void INT_EnableIRQ(void);
  extern void l1dmacro_RF_sleep(void);
  extern void l1dmacro_RF_wakeup(void);
  WORD32 l1s_get_HWTimers_ticks(void);

  // file timer1.h
  SYS_UWORD16 Dtimer1_Get_cntlreg(void);
  void Dtimer1_AR(unsigned short Ar);
  void Dtimer1_PTV(unsigned short Ptv);
  void Dtimer1_Clken(unsigned short En);
  void Dtimer1_Start (unsigned short startStop);
  void Dtimer1_Init_cntl (SYS_UWORD16 St, SYS_UWORD16 Reload, SYS_UWORD16   clockScale, SYS_UWORD16 clkon);
  SYS_UWORD16 Dtimer1_WriteValue (SYS_UWORD16 value);
  SYS_UWORD16 Dtimer1_ReadValue (void);
#endif

void l1s_wakeup(void);
BOOL l1s_compute_wakeup_ticks(void);
void l1s_recover_Frame(void);
UWORD8 Cust_recover_Os(void);
void l1s_recover_HWTimers(void);
UWORD8 Cust_check_system(void);
void f_arm_sleep_cmd(UWORD8 d_sleep_mode);

//#if (TRACE_TYPE == 2) || (TRACE_TYPE == 3)
  extern void L1_trace_string(char *s);
  extern void L1_trace_char  (char s);
//#endif
extern	UWORD16  slp_debug_flag;

#if (GSM_IDLE_RAM != 0)
extern void l1s_trace_mftab(void);
#endif

#if (CODE_VERSION != SIMULATION) && (CHIPSET == 15)
extern T_DRP_REGS_STR  *drp_regs;
#endif

#if L1_GPRS
  UWORD32 l1s_get_next_gauging_in_Packet_Idle(void);
#endif
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

/************************************************************/
/* Macros for power management                              */
/************************************************************/
#define MIN(min, operand1) \
  if (operand1 <=  min) min = operand1;

// ex: RATIO T32khz/T4.33Mhz      = 132.2428385417
//                => root         = integer part of the ratio
//                                = 132
//                => frac         = fractionnal part of the ratio multiplied by 65536 rounded to make it integer
//                                = 0.2428385417 * 65536 (Cf. ULPD specification)
//                                = 0.2428385417 * 2^16
//                                = 15914.66666689 = 15914

#define RATIO(HF,LF, root, frac)          \
  root = (UWORD32)(HF/LF);                           \
  frac = (UWORD32)(((HF - (root*LF)) << 16) / LF);

// previous ratio with frac + 0.5
#define RATIO2(HF,LF, root, frac)         \
if(LF){                                       \
 root = (UWORD32)(HF/LF);                           \
  frac = (UWORD32)((((HF - (root*LF)) << 16) + 0.5*LF) / LF);}

#define HFTHEO(LF, root, frac, hftheo) \
  hftheo = root*LF + ((frac*LF) >>16);

#define SUM(HF, LF, nb, ind) \
  LF=HF=0; \
  for(ind=0; ind<nb; ind++) \
  { \
    LF = LF +l1s.pw_mgr.histo[ind][0]; \
    HF = HF +l1s.pw_mgr.histo[ind][1]; \
  }



#if (CODE_VERSION!=SIMULATION)
T_PWMGR_DEBUG l1_pwmgr_debug;
#endif // NOT SIMULATION

#if(CHIPSET == 15)

/************************************************************/
/* Configure EMIF for optimal consumption                   */
/************************************************************/


    void EMIF_SetConfReg(const UWORD8 wp,const UWORD8 flush_prefetch,const UWORD8 Prefetch_mode,const UWORD8 pde,const UWORD8 pwd_en)
	{
    UWORD16 Emif_config_Reg;
    Emif_config_Reg  = (pwd_en << EMIF_CONFIG_PWD_POS | pde << EMIF_CONFIG_PDE_POS | Prefetch_mode << EMIF_CONFIG_PREFETCH_POS | flush_prefetch << EMIF_CONFIG_FLUSH_PREFETCH_POS | wp << EMIF_CONFIG_WP_POS);
    /*p_Emifreg -> EMIF_Config =  (Emif_config_Reg & EMIF_CONFIG_REG_MASK );*/
    EMIF_CONFIG              = Emif_config_Reg;
	} // End of  EMIF_SetConfReg



#if (OP_L1_STANDALONE == 1)  // API for Audio and MADC



	T_AUDIO_OUTEN_REG audio_outen_pm;

// L1 Standalone function for Configuring Audio registers.
// Argument CLK_MASK checks if Audio path is active
// Argument SLEEP_CMD configures Audio registers for optimal consumption
// Argument WAKE_CMD reconfigure audio registers after wakeup

	Uint8 madc_outen_check(Uint8 cmd)  {
		BspTwl3029_ReturnCode returnVal = BSP_TWL3029_RETURN_CODE_FAILURE;
	   /* I2C array */
		Bsp_Twl3029_I2cTransReqArray i2cTransArray;
		Bsp_Twl3029_I2cTransReqArrayPtr i2cTransArrayPtr= &i2cTransArray;

		/* twl3029 I2C reg info struct */
		BspTwl3029_I2C_RegisterInfo regInfo[8] ;
		BspTwl3029_I2C_RegisterInfo* regInfoPtr = regInfo;
		BspTwl3029_I2C_RegData shadow_pwronstatus, ston_bit;
		Uint8 count = 0;//OMAPS90550-new


		switch( cmd )  {

			case CLK_MASK:
				  returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_PWRONSTATUS_OFFSET,
									 &shadow_pwronstatus);
				  ston_bit = (shadow_pwronstatus & (1 << BSP_TWL3029_LLIF_AUDIO_PWRONSTATUS_STON_OFFSET));

				  if (ston_bit == 1) return DCXO_CLOCK;
				  else return NO_CLOCK;
 // omaps00090550 		break;

			case SLEEP_CMD:
				/* store the output enable 1 register  */
				returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN1_OFFSET,
									 &audio_outen_pm.outen1);

				/* store the output enable 2 register  */
				returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
									 &audio_outen_pm.outen2);

				/* store the output enable 3 register  */
				returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
								 &audio_outen_pm.outen3);


				/* write default values into OUTEN1,OUTEN2 & OUTEN3 */
				returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN1_OFFSET,
	  											 BSP_TWL_3029_MAP_AUDIO_OUTEN1_DEFAULT,regInfoPtr++);
				count++;

				/* store the output enable 2 register  */
				returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
	 											 BSP_TWL_3029_MAP_AUDIO_OUTEN2_DEFAULT,regInfoPtr++);
				count++;

				/* store the output enable 3 register  */
				returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
								 BSP_TWL_3029_MAP_AUDIO_OUTEN3_DEFAULT,regInfoPtr++);
				count++;


				/* now request to I2C manager to write to Triton registers */
				if (returnVal != BSP_TWL3029_RETURN_CODE_FAILURE)
				{
							returnVal = BspTwl3029_I2c_regInfoSend(regInfo,(Uint16)count,NULL,
						   (BspI2c_TransactionRequest*)i2cTransArrayPtr);
				}

				if (returnVal == BSP_TWL3029_RETURN_CODE_SUCCESS) return SUCCESS;
				else return FAILURE;

 // omaps00090550 		break;

			case WAKE_CMD:
				/* write default values into OUTEN1,OUTEN2 & OUTEN3 */
				returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN1_OFFSET,
	  											 audio_outen_pm.outen1,regInfoPtr++);
				count++;

				/* store the output enable 2 register  */
				returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
	 											 audio_outen_pm.outen2,regInfoPtr++);
				count++;

				/* store the output enable 3 register  */
				returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
								 audio_outen_pm.outen3,regInfoPtr++);
				count++;



				/* now request to I2C manager to write to Triton registers */
				if (returnVal != BSP_TWL3029_RETURN_CODE_FAILURE)
				{
							returnVal = BspTwl3029_I2c_regInfoSend(regInfo,(Uint16)count,NULL,
						   (BspI2c_TransactionRequest*)i2cTransArrayPtr);
				}

  				if (returnVal == BSP_TWL3029_RETURN_CODE_SUCCESS) return SUCCESS;
  					else return FAILURE;
 // omaps00090550 			break;
		}
		return SUCCESS;//omaps00090550
	}
#else  // Integrated build  API for Audio and MADC

// Full PS build function for Configuring Audio registers.
// Argument CLK_MASK checks if Audio path is active
// Argument SLEEP_CMD configures Audio registers for optimal consumption
// Argument WAKE_CMD reconfigure audio registers after wakeup


	T_AUDIO_OUTEN_REG audio_outen_pm;
	BspTwl3029_I2C_RegData audio_ctrl3;

	Uint8 audio_madc_sleep(Uint8 cmd)  {
		BspTwl3029_ReturnCode returnVal = BSP_TWL3029_RETURN_CODE_FAILURE;
	   /* I2C array */
		//Bsp_Twl3029_I2cTransReqArray i2cTransArray;
		//Bsp_Twl3029_I2cTransReqArrayPtr i2cTransArrayPtr= &i2cTransArray;

		/* twl3029 I2C reg info struct */
		//BspTwl3029_I2C_RegisterInfo regInfo[8] ;
		//BspTwl3029_I2C_RegisterInfo* regInfoPtr = regInfo;
		BspTwl3029_I2C_RegData shadow_pwronstatus, ston_bit;


		Uint8 count = 0;


		switch( cmd )  {

			case CLK_MASK:
				  returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_PWRONSTATUS_OFFSET,
									 &shadow_pwronstatus);
				  ston_bit = (shadow_pwronstatus & (1 << BSP_TWL3029_LLIF_AUDIO_PWRONSTATUS_STON_OFFSET));

				  if (ston_bit == 1) return DCXO_CLOCK;
				  else return NO_CLOCK;
//omaps00090550			break;

			case SLEEP_CMD:
    #if 0
				/* store the output enable 1 register  */
				returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN1_OFFSET,
									 &audio_outen_pm.outen1);

				/* store the output enable 2 register  */
				returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
									 &audio_outen_pm.outen2);

				/* store the output enable 3 register  */
				returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
								 &audio_outen_pm.outen3);

				returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
								 &audio_ctrl3);

				if( audio_outen_pm.outen1 )
				{
					/* write default values into OUTEN1,OUTEN2 & OUTEN3 */
					returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN1_OFFSET,
	  											 BSP_TWL_3029_MAP_AUDIO_OUTEN1_DEFAULT,regInfoPtr++);
					count++;
				}

				if( audio_outen_pm.outen2 )
				{

					/* store the output enable 2 register  */
					returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
	 											 BSP_TWL_3029_MAP_AUDIO_OUTEN2_DEFAULT,regInfoPtr++);
					count++;
				}

				if( audio_outen_pm.outen3 )
				{
					/* store the output enable 3 register  */
					returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
								 BSP_TWL_3029_MAP_AUDIO_OUTEN3_DEFAULT,regInfoPtr++);
					count++;
				}

				/* Selectively checking if INMODE is set or not. Write is queued only when INMODE(0-3)
					is non-zero */
				if( audio_ctrl3 & 0xf)
				{
					/* store the output enable 3 register  */
					returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_CTRL3_OFFSET,
								 BSP_TWL_3029_MAP_AUDIO_CTRL3_DEFAULT,regInfoPtr++);
					count++;
				}

			    returnVal =  BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUX,BSP_TWL3029_MAP_AUX_REG_TOGGLE1_OFFSET,
								1 << BSP_TWL3029_LLIF_AUX_REG_TOGGLE1_MADCR_OFFSET, regInfoPtr++);
			    count++;

                //Turn off the USB leakage currrent

               // returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_PAGE0, BSP_TWL_3029_MAP_CKG_TESTUNLOCK_OFFSET,0xb6,regInfoPtr++);
               // count++;

                //returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_PAGE2, BSP_TWL3029_MAP_USB_PSM_EN_TEST_SET_OFFSET,0x80,regInfoPtr++);
                //count++;

             //   returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_PAGE2,BSP_TWL3029_MAP_USB_VBUS_EN_TEST_OFFSET,0x00,regInfoPtr++);
             //   count++;

                //returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_PAGE0, BSP_TWL_3029_MAP_CKG_TESTUNLOCK_OFFSET,0x00,regInfoPtr++);
                //count++;

				// now request to I2C manager to write to Triton registers
				//if (returnVal != BSP_TWL3029_RETURN_CODE_FAILURE && !is_i2c_bus_locked())
				if (returnVal != BSP_TWL3029_RETURN_CODE_FAILURE)
				{
					returnVal = BspTwl3029_I2c_regInfoSend(regInfo,(Uint16)count,NULL,
						   (BspI2c_TransactionRequest*)i2cTransArrayPtr);
				}

				if (returnVal == BSP_TWL3029_RETURN_CODE_SUCCESS) return SUCCESS;
				else return FAILURE;

#endif

//omaps00090550			break;

			case WAKE_CMD:
#if 0

				if( audio_outen_pm.outen1 )
				{
					/* write default values into OUTEN1,OUTEN2 & OUTEN3 */
					returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN1_OFFSET,
	  											 audio_outen_pm.outen1,regInfoPtr++);
					count++;
				}

				if( audio_outen_pm.outen2 )
				{

					/* store the output enable 2 register  */
					returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
	 											 audio_outen_pm.outen2,regInfoPtr++);
					count++;
				}

				if( audio_outen_pm.outen3 )
				{
					/* store the output enable 3 register  */
					returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
								 audio_outen_pm.outen3,regInfoPtr++);
					count++;
				}

				/* Selectively checking if INMODE is set or not. Write is queued only when INMODE(0-3)
					is non-zero */
				if( audio_ctrl3 & 0xf)
				{
					/* store the output enable 3 register  */
					returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_CTRL3_OFFSET,
								 audio_ctrl3,regInfoPtr++);
					count++;
				}


				//wake up mode: Enable MADC
				returnVal =  BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUX,BSP_TWL3029_MAP_AUX_REG_TOGGLE1_OFFSET,
								1 << BSP_TWL3029_LLIF_AUX_REG_TOGGLE1_MADCS_OFFSET, regInfoPtr++);

				count++;		//TI_SH added to set the madc on correctly

                //Enable the USB leakage current after wake up

              //  returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_PAGE0,BSP_TWL_3029_MAP_CKG_TESTUNLOCK_OFFSET,0xb6,regInfoPtr++);
               // count++;

               // returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_PAGE2,BSP_TWL3029_MAP_USB_VBUS_EN_TEST_OFFSET,0x0F,regInfoPtr++);
               // count++;

                //returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_PAGE2,BSP_TWL3029_MAP_USB_PSM_EN_TEST_CLR_OFFSET,0x80,regInfoPtr++);
                //count++;

               // returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_PAGE0,BSP_TWL_3029_MAP_CKG_TESTUNLOCK_OFFSET,0x00,regInfoPtr++);
               // count++;

								// now request to I2C manager to write to Triton registers
				//if (returnVal != BSP_TWL3029_RETURN_CODE_FAILURE && !is_i2c_bus_locked())
				if (returnVal != BSP_TWL3029_RETURN_CODE_FAILURE)
				{
							returnVal = BspTwl3029_I2c_regInfoSend(regInfo,(Uint16)count,NULL,
						   (BspI2c_TransactionRequest*)i2cTransArrayPtr);
				}

  				if (returnVal == BSP_TWL3029_RETURN_CODE_SUCCESS) return SUCCESS;
  					else return FAILURE;
#endif
	break;
		}
	return SUCCESS;
	}
#endif  // API for Audio and MADC



//Function to check status of Backlight Only Argument 0 is valid


	Uint8 backlight_pwr_interface(Uint8 cmd)
	{
		BspTwl3029_I2C_RegData regData;


		if(cmd == 0)
		{
			BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_PAGE0,PWDNSTATUS,&regData);
			if((regData) & 0x70)
			{
				return(DCXO_CLOCK);
			}
			else
			{
				return(NO_CLOCK);
			}

		}
		else
		{
			return(SUCCESS);
		}
	}

//Dummy Function for peripheral check to populate Function pointer table for unused APIs


	Uint8  f_peripheral_interface_dummy(Uint8 cmd)
	{
		if(cmd == 0)
		{
			return(NO_CLOCK);
		}
		else
		{
			return(SUCCESS);
		}

	}

//Dummy Function for Application check to populate Function pointer table for unused APIs

	Uint8 f_application_interface_dummy(Uint8 cmd)
	{
		if(cmd == 0)
		{
			return(PM_INACTIVE);
		}
		else
		{
			return(SUCCESS);
		}
	}

//Function not used as of now //OMAPS00090550
	void Update_Sleep_Status( Uint8 ID, Uint8 state)
	{
		if(state)
		{
			SLEEP_STATE |= (state << ID); //omaps00090550 ! was present before
		}
		else
		{
			SLEEP_STATE &=((Uint8)~1 <<ID);  //omaps00090550
		}
	}

//Function polls the status of the following peripherals to take
//Sleep Decision:
//UART, USB, I2C, LCD, Camera, Backlight, Audio Stereo path,
//Bluetooth and USIM.
//All peripherals either cause Deep Sleep or No Sleep.
//Only USIM can also cause Big Sleep.



	UWORD32 Check_Peripheral_App(void)
	{
 #if (CODE_VERSION!=SIMULATION)
    UWORD8 ret_value;
		/* Check Peripherals */
    ret_value = Peripheral_interface[UART_ID](CLK_MASK);
    if(ret_value)
    {
      l1_pwmgr_debug.fail_id = UART_ID;
      l1_pwmgr_debug.fail_ret_val = ret_value;
      return(DO_NOT_SLEEP);
    }
    ret_value = Peripheral_interface[USB_ID](CLK_MASK);
    if(ret_value)
    {
      l1_pwmgr_debug.fail_id = USB_ID;
      l1_pwmgr_debug.fail_ret_val = ret_value;
      return(DO_NOT_SLEEP);
    }
    ret_value = Peripheral_interface[I2C_ID](CLK_MASK);
    if(ret_value)
    {
      l1_pwmgr_debug.fail_id = I2C_ID;
      l1_pwmgr_debug.fail_ret_val = ret_value;
      return(DO_NOT_SLEEP);
    }
    ret_value = Peripheral_interface[LCD_ID](CLK_MASK);
    if(ret_value)
    {
      l1_pwmgr_debug.fail_id = LCD_ID;
      l1_pwmgr_debug.fail_ret_val = ret_value;
      return(DO_NOT_SLEEP);
    }
    ret_value = Peripheral_interface[CAMERA_ID](CLK_MASK);
    if(ret_value)
    {
      l1_pwmgr_debug.fail_id = CAMERA_ID;
      l1_pwmgr_debug.fail_ret_val = ret_value;
      return(DO_NOT_SLEEP);
    }
    ret_value = Peripheral_interface[BACKLIGHT_ID](CLK_MASK);
    if(ret_value)
    {
      l1_pwmgr_debug.fail_id = BACKLIGHT_ID;
      l1_pwmgr_debug.fail_ret_val = ret_value;
      return(DO_NOT_SLEEP);
    }
    ret_value = Peripheral_interface[MADC_AS_ID](CLK_MASK);
    if(ret_value)
    {
      l1_pwmgr_debug.fail_id = MADC_AS_ID;
      l1_pwmgr_debug.fail_ret_val = ret_value;
			return(DO_NOT_SLEEP);
		}
/* check battery charger */
	ret_value = Peripheral_interface[BCI_ID](CLK_MASK);
	if(ret_value)
	{
	      l1_pwmgr_debug.fail_id = BCI_ID;
	      l1_pwmgr_debug.fail_ret_val = ret_value;
	      return(DO_NOT_SLEEP);
	}

		/* Check Applications */
    ret_value = Application_interface[BT_Stack_ID](APP_ACTIVITY);
    if(ret_value)
    {
      // L1_APPLICATION_OFFSET is added to distinguish Application interface
      l1_pwmgr_debug.fail_id = BT_Stack_ID + (L1_PWMGR_APP_OFFSET);
      l1_pwmgr_debug.fail_ret_val = ret_value;
      return(DO_NOT_SLEEP);
    }
    ret_value = Peripheral_interface[USIM_ID](CLK_MASK);
    if(ret_value)
    {
      l1_pwmgr_debug.fail_id = USIM_ID;
      l1_pwmgr_debug.fail_ret_val = ret_value;
			l1s.pw_mgr.why_big_sleep = BIG_SLEEP_DUE_TO_SIM;
			return(FRAME_STOP);
		}
		else
		{
			return(CLOCK_STOP);
		}
   #endif //NOT SIMULATION
	}

//This function Configures  DBB for optimal Power Consumption
//during Deep Sleep



	void DBB_Configure_DS()
	{
	// FDP enabling and disabling of burst configuration in flash not required in Locosto
       // Hardware Settings as per Power Bench

		// Stop RNG oscillators
		RNG_CONFIG &= 0xF03F;


		/* Set GPIOs 19 to 22 as outputs to avoid floating pins */
		GPIO1_CNTL_REG &= ~0x0078;

		/* Set PD on VDR and VFSRX for VSP bus to avoid floating pins */
		CONF_VDR	|= 0x0008;
		CONF_VFSRX  |= 0x0008;

		/* Set HASH in auto-idle */
	    SHA_MASK = 0x0001;

		/* Set DES in auto-idle */
		DES_MASK = 0x0001;

		/* Set RNG in auto-idle */
		RNG_MASK = 0x0001;


	/*	uart_in_pull_down(); */

	#if defined(RVM_DATALIGHT_SWE) || defined(RVM_NAN_SWE)


		temp_NAND_Reg1 = COMMAND_REG;
		temp_NAND_Reg2 = CONTROL_REG;
		temp_NAND_Reg3 = STATUS_IT_REG;

		COMMAND_REG	 = 0x06;
		CONTROL_REG	 = 0x0;
		STATUS_IT_REG = 0x0;

	#endif
		// RANGA: All these bit fields should be replaced by macros
		// Set DPLL in idle mode
		// Cut C-PORT (new), IRQ, BRIDGE and TIMER clocks
		/* Set DPLL in idle mode */
		/* Cut C-PORT (new), IRQ, BRIDGE and TIMER clocks */
		CLKM_CNTL_CLK_REG &= ~0x0010 ;
		CLKM_CNTL_CLK_REG |= 0x000F ;

		CNTL_APLL_DIV_CLK &= ~0x0001;  /* Disable APLL */

         // Statements below are not required for the current hardware version.
		 // This was done to solve the problem of DCXO taking 10 frames
		 // to wake-up from Deep Sleep in older hardware versions.

		 //DCXO_THRESH_L  = 0xC040;  // Setting DCXO Thresholds
		 //DCXO_THRESH_H  = 0x051F;  // to solve Deep Sleep problem
	}

//This function Restores  DBB after wakeup from Deep Sleep


	void DBB_Wakeup_DS()
	{
// FDP re-enabling and burst re-configuration are not required if FDP is disabled
	//	during deep-sleep

	  CLKM_CNTL_CLK_REG |= 0x0010 ; // Enable CPORT Clock

	  CNTL_APLL_DIV_CLK |= 0x0001;  // Enable APLL clock

		#if defined(RVM_DATALIGHT_SWE) || defined(RVM_NAN_SWE)

		  // Restoring NAND
			COMMAND_REG  = temp_NAND_Reg1;
			CONTROL_REG  = temp_NAND_Reg2;
			STATUS_IT_REG  = temp_NAND_Reg3;
		  // Restoring NAND
		#endif


	}


//This function shuts down APC Bandgap.Cannot be used for PG 1.0 Can be used only for PG 2.0


void Disable_APC_BG()  //omaps00090550
{
	while (RHSW_ARM_CNF & DSP_PERIPH_LOCK)
    RHSW_ARM_CNF |= ARM_PERIPH_LOCK;
	APCCTRL2 &= ~BGEN;
	return;
}

//This function enables APC Bandgap.Cannot be used for PG 1.0 Can be used only for PG 2.0

void Enable_APC_BG() //omaps00090550
{
	while (RHSW_ARM_CNF & DSP_PERIPH_LOCK)
    RHSW_ARM_CNF |= ARM_PERIPH_LOCK;
	APCCTRL2 |= BGEN;
	return;
}

#endif  //CHIPSET = 15










// l1ctl_pgm_clk32()
// convert ratio in 4.33Mhz and pgm INC_FRAC,INC_SIXTEEN.

void l1ctl_pgm_clk32(UWORD32 nb_hf, UWORD32 nb_32khz)
{
#if (CODE_VERSION != SIMULATION)
  if (l1_config.pwr_mngt == PWR_MNGT)
  {
    UWORD32 inc_sixteen= 0, inc_frac=0, lf;

    // REM: nb_hf is the real value of the high frequency (ex in nbr of 65Mhz clock)
    // To compute the ratio, nb_hf must be expressed in nbr of clock 4.33 Mhz
    // that's why nb_hf is divided by 3*l1_config.dpll
    // RATIO2(nb_hf/(3*l1_config.dpll),nb_32khz,inc_sixteen,inc_frac);
    // this line above is equal to the ligne below:
    lf=(UWORD32)((UWORD32)(3*((UWORD32)((UWORD32)(l1_config.dpll)*nb_32khz))));  //OMAPS00090550
    RATIO2(nb_hf,lf,inc_sixteen,inc_frac);

    // integer part
    ULDP_INCSIXTEEN_UPDATE(inc_sixteen);

    // fractional part
    ULDP_INCFRAC_UPDATE(inc_frac);
  }
#endif
}


// l1ctl_gauging()
// Description: management of the gauging results
// At RESET state reset histogram and then go to INIT.
// At INIT state, go back to RESET on each               */
// gauging > +- 100 ppm. If NB_INIT good gauging go to ACQUIS state.
// At ACQUIS state, go back to RESET on each gauging     > (+- 20ppm +- 1us). If NB_ACQU good gauging          */
// go to UPDATE state. Allow deep sleep feature.
// At UPDATE state, count consecutive  gauging >+- 1 us.
// If MAX_BAD_GAUGING results go back to RESET.
// Otherwise re-enable deep sleep feature and reset  bad results counter.

void l1ctl_gauging ( UWORD32 nb_32khz, UWORD32 nb_hf)
{
  if (l1_config.pwr_mngt == PWR_MNGT)
  {
    enum states
    {
      RESET  = 0,
      INIT   = 1,
      ACQUIS = 2,
      UPDATE = 3
    };

    static UWORD8  bad_count;              // bad gauging values
    static UWORD8  gauging_state= RESET;   // RESET,INIT, ACQUIS, UPDATE
    static UWORD8  nb_gaug;                // number of gauging in ACQUIS
    static UWORD8  idx,i;                  // index
    static UWORD32 root, frac;             // ratio of HF and LF average
    UWORD32 sumLF=0 , sumHF=0;                  // sum of HF and LF counts
    double nbHF_theo;


    // AFC or TEMPERATURE variation

    //if ( (ABS( (WORD32)(l1s.pw_mgr.previous_afc-l1s.afc) ) > AFC_VARIATION)  ||
    //     (ABS( (WORD32)(l1s.pw_mgr.previous_temp-l1s.afc) > TEMP_VARIATION) )
    //  gauging_state = RESET;

    // reset state machine if not in IDLE mode
    #if L1_GPRS
      if ((l1a_l1s_com.l1s_en_task[NP] != TASK_ENABLED) && (l1a_l1s_com.l1s_en_task[PNP] != TASK_ENABLED))
        gauging_state = RESET;
    #else
      if ((l1a_l1s_com.l1s_en_task[NP] != TASK_ENABLED) )
        gauging_state = RESET;

    #endif

    switch (gauging_state)
    {

      case RESET:
      {
        UWORD8 i;

        // Reset Histogram
        for (i=0; i < SIZE_HIST; i++)
        {
          l1s.pw_mgr.histo[i][0] = 0;
          l1s.pw_mgr.histo[i][1] = 0;
        }
        idx = 0;
        l1s.pw_mgr.enough_gaug= FALSE;  // forbid Deep sleep
        gauging_state = INIT;
        nb_gaug = NB_INIT;              // counter for ACQUIS state
        bad_count = 0;                  // reset count of BAD gauging

        #if (TRACE_TYPE != 0)
              l1_trace_gauging_reset();
        #endif
      }


     case INIT:
      {

        // Acquire NB_INIT gauging wtw +- 100 ppm

        if (l1a_l1s_com.mode != I_MODE) return;

        // compute clocks ratio from measurements.
        RATIO(nb_hf,nb_32khz,root,frac)


        // allow [-500ppm,+100ppm] derive on 32Khz at startup.
// Commenting section below for OMAPS00148004
       /* if ( 
             (root > l1s.pw_mgr.c_clk_min || 
             (root == l1s.pw_mgr.c_clk_min &&
              frac >= l1s.pw_mgr.c_clk_init_min) ) &&
             (root < l1s.pw_mgr.c_clk_max || 
             (root == l1s.pw_mgr.c_clk_max &&
              frac <= l1s.pw_mgr.c_clk_init_max ) )
			  */
	       if (
	             (  l1s.pw_mgr.c_clk_min == l1s.pw_mgr.c_clk_max &&
	                frac >= l1s.pw_mgr.c_clk_init_min &&
	                frac <= l1s.pw_mgr.c_clk_init_max )
		             || 
		         (  l1s.pw_mgr.c_clk_min != l1s.pw_mgr.c_clk_max &&
	             ( (root == l1s.pw_mgr.c_clk_min &&
	                frac >= l1s.pw_mgr.c_clk_init_min ) ||
                 (root > l1s.pw_mgr.c_clk_min &&
                  root < l1s.pw_mgr.c_clk_max ) ||
                 (root == l1s.pw_mgr.c_clk_max &&
                  frac <= l1s.pw_mgr.c_clk_init_max ) ) )
	   )
        {
          l1s.pw_mgr.histo[idx  ][0] = nb_32khz; // init histo with the number of 32kHz
          l1s.pw_mgr.histo[idx++][1] = nb_hf; // init histo with the number of hf (13Mhz)

          #if (CODE_VERSION == SIMULATION)
            #if (TRACE_TYPE==5)
              trace_ULPD("Gauging INIT Case ", l1s.actual_time.fn);
            #endif
          #endif

        }
        else
        {
          // out of the allowed derive -> reset
          idx=0;
          #if (TRACE_TYPE != 0)
              l1_trace_gauging_reset();
          #endif
        }

        if (idx == NB_INIT)
        {
          // enough measurement -> ACQUIS state
          gauging_state   = ACQUIS;
          // compute clk ratio on count average
          SUM(sumHF,sumLF, NB_INIT,i)    // returns sumHF and sumLF
          RATIO(sumHF,sumLF,root, frac)  // returns root and frac*2E16, computed on the average
        }
      }
      break;


      case ACQUIS:
      {
        // Acquire NB_ACQU gauging at +-25ppm
        // with jitter +- 1 us
        UWORD8 n;

        // from nb_32khz "measured"
        // compute nbHF_theo
        HFTHEO(nb_32khz,root,frac,nbHF_theo)

        if ( (nb_hf >= (nbHF_theo - l1s.pw_mgr.c_delta_hf_acquis))  &&
             (nb_hf <= (nbHF_theo + l1s.pw_mgr.c_delta_hf_acquis))  )
        {
          l1s.pw_mgr.histo[idx][0]   = nb_32khz;
          l1s.pw_mgr.histo[idx++][1] = nb_hf;
          idx = idx % SIZE_HIST;

          // compute clk ratio on count average
          if(++nb_gaug >= SIZE_HIST) n=SIZE_HIST;
          else n= nb_gaug;
          SUM(sumHF,sumLF, n,i)
          RATIO(sumHF,sumLF,root, frac)

          #if (CODE_VERSION == SIMULATION)
            #if (TRACE_TYPE==5)
              trace_ULPD("Gauging ACQUIS Case ", l1s.actual_time.fn);
            #endif
          #endif

          if ( nb_gaug == (NB_INIT+NB_ACQU))     // NB_ACQU good gauging
          {
            gauging_state = UPDATE;               // UPDATE state
            l1s.pw_mgr.enough_gaug = TRUE;        // allow Deep sleep
            l1ctl_pgm_clk32(sumHF,sumLF); // clocks ratio in 4.33Mhz
          }
        }
        else
        {
          gauging_state = RESET;
        }
      }
      break;

      case UPDATE:
      {

        // Update gauging histogram
        // compute nbHF theoric for ratio_avg
        HFTHEO(nb_32khz,root,frac,nbHF_theo)

        if ( (nb_hf >= (nbHF_theo-l1s.pw_mgr.c_delta_hf_update))  &&
             (nb_hf <= (nbHF_theo+l1s.pw_mgr.c_delta_hf_update))  )
        {
          l1s.pw_mgr.histo[idx][0]   = nb_32khz;
          l1s.pw_mgr.histo[idx++][1] = nb_hf;

          // compute clk ratio on count average
          SUM(sumHF,sumLF, SIZE_HIST,i)
          l1ctl_pgm_clk32(sumHF,sumLF); // clocks ratio in 4.33Mhz

          l1s.pw_mgr.enough_gaug = TRUE;          // allow Deep sleep
          bad_count = 0;                          // reset count of BAD gauging

          #if (CODE_VERSION == SIMULATION)
            #if (TRACE_TYPE==5)
              trace_ULPD("Gauging UPDATE Case ", l1s.actual_time.fn);
            #endif
          #endif

        }
        else
        {
          bad_count ++;
          if (bad_count >= MAX_BAD_GAUGING) gauging_state = RESET;
          l1s.pw_mgr.enough_gaug= FALSE;          // forbid Deep sleep
        }
        idx = idx % SIZE_HIST;
      }
      break;
    }
  #if (TRACE_TYPE != 0) // Trace gauging
    // save parameters in the corresponding structure
    l1s.pw_mgr.state = 	gauging_state;
    l1s.pw_mgr.lf = nb_32khz ;
    // WARNING WARNING, this case gauging_state == UPDATE modify the algo.
    // In case of trace the parameter root and frac are refresh.
    // it is not the case if no trace and it seems there is  mistake
    if (gauging_state == UPDATE)
    {
      RATIO2(sumHF,sumLF,root,frac);
    }
    //End of Warning.
    l1s.pw_mgr.hf = nb_hf ;
    l1s.pw_mgr.root = root ;
    l1s.pw_mgr.frac = frac ;
  #endif // End Trace gauging
  }
}




/* GAUGING_Handler()                                     */
/* Description: update increment counter for 32Khz       */
/*  This interrupt function computes the ratio between   */
/*  HF/32Khz gauging counters and program ULPD increment */
/*  values.                                              */

void GAUGING_Handler(void)
{
#if (CODE_VERSION != SIMULATION)
  if (l1_config.pwr_mngt == PWR_MNGT)
  {
    UWORD32 nb_32khz, nb_hf;

    // Gauging task is ended
    l1s.pw_mgr.gauging_task  = INACTIVE;
    #if (CHIPSET == 12) || (CHIPSET == 15)
        F_INTH_DISABLE_ONE_IT(C_INTH_ULPD_GAUGING_IT); // Mask ULPD GAUGING int.
    #else
        INTH_DISABLEONEIT(IQ_ULPD_GAUGING);          // Mask ULPD GAUGING int.
    #endif

    // Number of 32 Khz clock at the end of the gauging
    nb_32khz =  ((*( UWORD16 *)ULDP_COUNTER_32_MSB_REG) * 65536) +
                 (*( UWORD16 *)ULDP_COUNTER_32_LSB_REG);

    // Number of high frequency clock at the end of the gauging
    // Convert it in nbr of 13 Mhz clocks (5*13=65Mhz)
    nb_hf =   ( ((*( UWORD16 *)ULDP_COUNTER_HI_FREQ_MSB_REG) * 65536) +
                 (*( UWORD16 *)ULDP_COUNTER_HI_FREQ_LSB_REG) ); // Divide by PLL ratio

    l1ctl_gauging(nb_32khz, nb_hf);
  }
#else //Simulation part

    // Gauging task is ended
    l1s.pw_mgr.gauging_task  = INACTIVE;

    l1ctl_gauging(DEFAULT_32KHZ_VALUE,DEFAULT_HFMHZ_VALUE);
#endif
}




// l1s_get_HWTimers_ticks()
// Description:
// evaluate the loading of the HW Timers for dep sleep
// BIG SLEEP: timers CLK may be stopped (user dependant)
// DEEP SLEEP:timers CLK and WTCHDOG CLK are stopped
//            CLKS are enabled after VTCX0+SLICER+13MHZ
//            setup time

WORD32 l1s_get_HWTimers_ticks(void)
{
#if (CODE_VERSION != SIMULATION)
  if (l1_config.pwr_mngt == PWR_MNGT)
  {
    WORD32  timer1,timer2,watchdog,HWTimer;
    #if (CHIPSET == 12) || (CHIPSET == 15)
      WORD32  watchdog_sec;
    #endif
    UWORD16 cntlreg;
    UWORD16 modereg;
    WORD32 old = 0;

      // read Hercules Timers & Watchdog
      //=================================================
      // Tint = Tclk * (LOAD_TIM+1) * 2^(PTV+1)
      // Tclk = 1.2308us for Fclk=13Mhz
      // PTV  = X (pre-scaler field)
      //-------------------------------------------------
      timer1 = timer2 = watchdog = HWTimer = -1;
      #if (CHIPSET == 12) || (CHIPSET == 15)
        watchdog_sec = -1;
      #endif

      cntlreg =  Dtimer1_Get_cntlreg();   // AND 0x1F
      if ( (cntlreg & D_TIMER_RUN) == D_TIMER_RUN)
      {
        cntlreg = cntlreg&0x1F;
	cntlreg >>= 2;   // take PTV
        cntlreg = 1 << (cntlreg+1);
        timer1 = (WORD32) ( ((Dtimer1_ReadValue()+1) * cntlreg * 0.0012308)  / 4.615 );
        if (timer1 <= MIN_SLEEP_TIME) return(0);
        old = Dtimer1_ReadValue();
        HWTimer = timer1;
      }

      cntlreg =  Dtimer2_Get_cntlreg();
      if ( (cntlreg & D_TIMER_RUN) == D_TIMER_RUN)
      {
        cntlreg = cntlreg&0x1F;
	cntlreg >>= 2;   // take PTV
        cntlreg = 1 << (cntlreg+1);
        timer2 = (WORD32) ( ((Dtimer2_ReadValue()+1) * cntlreg * 0.0012308)  / 4.615 );
        if (timer2 <= MIN_SLEEP_TIME) return(0);
        if (HWTimer == -1) HWTimer = timer2;
        else MIN(HWTimer,timer2)
      }

      cntlreg = TIMER_Read(0); // AND 0x0f80
      modereg = TIMER_Read(2);

      if ( (cntlreg & TIMER_ST) || (modereg & TIMER_WDOG))
      {
        // in watchdog mode PTV is forced to 7
        if ( modereg & TIMER_WDOG )
           cntlreg |= TIMER_PTV;

        cntlreg = (cntlreg & TIMER_PTV) >> 9;   // take PTV
        cntlreg = 1 << (cntlreg+1);
        watchdog = (WORD32) ( ((TIMER_ReadValue()+1) * cntlreg * 0.001078)  / 4.615 );
        if (watchdog <= MIN_SLEEP_TIME) return(0);
        if (HWTimer == -1) HWTimer = watchdog;
        else MIN(HWTimer,watchdog)
      }

      #if (CHIPSET == 12) || (CHIPSET == 15)
        /*
         *  Secure Watchdog Timer management
         */
        cntlreg = TIMER_SEC_Read(0); // AND 0x0f80
        modereg = TIMER_SEC_Read(2);
        if ( (cntlreg & TIMER_ST) || (modereg & TIMER_WDOG))
        {
          // in watchdog mode PTV is forced to 7
          if ( modereg & TIMER_WDOG )
             cntlreg |= TIMER_PTV;

          cntlreg = (cntlreg & TIMER_PTV) >> 9;   // take PTV
          cntlreg = 1 << (cntlreg+1);
          watchdog_sec = (WORD32) ( ((TIMER_SEC_ReadValue()+1) * cntlreg * 0.001078)  / 4.615 );
          if (watchdog_sec <= MIN_SLEEP_TIME) return(0);
          if (HWTimer == -1) HWTimer = watchdog_sec;
          else MIN(HWTimer,watchdog_sec)
        }

      #endif

      return (HWTimer);
  }
#else // simulation part
  return (-1);  // no HW timer in simulation
#endif
return(-1); //omaps00090550
}

#if (GSM_IDLE_RAM != 0)  // Compile only if GSM_IDLE_RAM enabled

    void l1s_adapt_traffic_controller(void)
  {
    BOOL l1s_extram;
    UWORD8 nb_bitmap;
    T_L1S_GSM_IDLE_INTRAM * gsm_idle_ram_ctl;

    gsm_idle_ram_ctl = &(l1s.gsm_idle_ram_ctl);

    l1s_extram = FALSE;

    for(nb_bitmap=0; ((nb_bitmap < SIZE_TAB_L1S_MONITOR) && (l1s_extram == FALSE)); nb_bitmap++)
    {
      if (nb_bitmap == 1)
      {
        l1s_extram |= (((INT_RAM_GSM_IDLE_L1S_PROCESSES1 ^ gsm_idle_ram_ctl->task_bitmap_idle_ram[nb_bitmap]) & gsm_idle_ram_ctl->task_bitmap_idle_ram[nb_bitmap]) != 0);
      }else
      {
        l1s_extram |= (gsm_idle_ram_ctl->task_bitmap_idle_ram[nb_bitmap] != 0);
      }
    }

    if ((l1s_extram != FALSE) && (!READ_TRAFFIC_CONT_STATE))
    {
      CSMI_TrafficControllerOn();
      #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
      {
        l1s_trace_mftab();
      }
      #endif
    }
 }
#endif


// l1s_sleep_manager()
// Description:
// evaluate the loading of the system
// - SIM, UART, LCD ....
// - Nucleus tasks, Hisrs, timers
// - Timer1, Timer2, Watchdog
// program Big or Deep sleep

void l1s_sleep_manager()
{
//UWORD8 temp=0; OMAPS00090550

  UWORD16 temp_clear_intr;

  // fn when l1s_sleep_manager function is called
  #if (CODE_VERSION != SIMULATION)
    UWORD32 sleep_time = l1s.actual_time.fn_mod42432;
  #else
    UWORD32 sleep_time = l1s.actual_time.fn;
  #endif

#if(CHIPSET == 15)
	Uint8   sleep_status;
#endif

#if (GSM_IDLE_RAM != 0)
    T_L1S_GSM_IDLE_INTRAM * gsm_idle_ram_ctl;
    BOOL flag_traffic_controller_state = 0;
    gsm_idle_ram_ctl = &(l1s.gsm_idle_ram_ctl);

    #if (AUDIO_TASK == 1)
        gsm_idle_ram_ctl->l1s_full_exec = l1s.l1_audio_it_com;
    #endif

    if (gsm_idle_ram_ctl->l1s_full_exec == TRUE)
      return;
#endif

  if (l1_config.pwr_mngt == PWR_MNGT)
  {
// Power management is enabled
    WORD32   min_time, OSload, HWtimer,wake_up_time,min_time_gauging;
    UWORD32  sleep_mode;
    #if (ANALOG != 11)
    WORD32   afc_fix;
    #endif
    UWORD32  uw32_store_next_time;
   #if (CHIPSET != 15)
    static UWORD32 previous_sleep = FRAME_STOP;
   #endif
    #if (W_A_CALYPSO_PLUS_SPR_19599 == 1)
      BOOL     extended_page_mode_state = 0;       //Store state of extended page mode
    #endif
    #if (CHIPSET != 15)
    WORD32 time_from_last_wakeup=0;
    #endif

    #if (OP_BT == 1)
      WORD32   hci_ll_status;
    #endif

    // init for trace and debug
    l1s.pw_mgr.why_big_sleep = BIG_SLEEP_DUE_TO_UNDEFINED;
    l1s.pw_mgr.wakeup_type   = WAKEUP_FOR_UNDEFINED;

#if (CHIPSET != 15)
    time_from_last_wakeup = (sleep_time - l1s.pw_mgr.wakeup_time + 42432) % 42432;
#endif


    #if (CODE_VERSION != SIMULATION)
	      //=================================================
	      // Protect System structures
	      // must be called BEFORE INT_DisableIRQ() while
	      // Nucleus does not restore IRQ/FIQ bits !!!!
	      //=================================================
	      OS_system_protect();
	      //=================================================
	      // Disable IRQ
	      //=================================================
	      INT_DisableIRQ();
    #endif // NOT SIMULATION
    //=================================================
    // check System (SIM, UART, LDC ..... )
    //=================================================
#if (CHIPSET == 15)
    #if (WCP_PROF == 0)
	sleep_mode = Check_Peripheral_App();  /* For Locosto */
    #else
	sleep_mode = DO_NOT_SLEEP; //Check_Peripheral_App();  /* For Locosto */
    #endif
#else
    sleep_mode =  Cust_check_system();
#endif

    #if (GSM_IDLE_RAM != 0)
    //=================================================
    // check System (SIM, UART, LDC ..... )
    //=================================================
    gsm_idle_ram_ctl->sleep_mode = sleep_mode;
    #endif

    if (sleep_mode == DO_NOT_SLEEP)
      {
        OS_system_Unprotect();
        // free System structure
        // Enable all IRQ
        //l1_pwmgr_irq_dis_flag = 0;
      #if (CODE_VERSION!=SIMULATION)
        INT_EnableIRQ();
        l1_trace_fail_sleep(FAIL_SLEEP_PERIPH_CHECK, l1_pwmgr_debug.fail_id, l1_pwmgr_debug.fail_ret_val);
      #endif
    #if (GSM_IDLE_RAM != 0)
    gsm_idle_ram_ctl->os_load = 0;
    gsm_idle_ram_ctl->hw_timer = 0;
    #endif // GSM_IDLE_RAM
    return;
      }


#if (OP_L1_STANDALONE == 0)
    /*GC_Sleep(); 	OMAPS00134004*/
#endif
    //=================================================
    // check OS loading
    //=================================================
    OSload = OS_get_inactivity_ticks();
  #if (CODE_VERSION!=SIMULATION)
    if ((OSload >= 0) && (OSload <= MIN_SLEEP_TIME)){
      l1_pwmgr_debug.fail_id = FAIL_SLEEP_DUE_TO_OSLOAD;
      l1_pwmgr_debug.fail_ret_val = OSload;
    }
  #endif //NOT SIMULATION

    //=================================================
    // check HW Timers loading
    //=================================================
    HWtimer= l1s_get_HWTimers_ticks();
    #if (CODE_VERSION!=SIMULATION)
    if (HWtimer == 0){
      l1_pwmgr_debug.fail_id = FAIL_SLEEP_DUE_TO_HWTIMER;
      l1_pwmgr_debug.fail_ret_val = 0;
    }
    #endif //NOT SIMULATION

    #if (GSM_IDLE_RAM != 0)
    //=================================================
    // check OS loading
    //=================================================
    gsm_idle_ram_ctl->os_load = OSload;

    //=================================================
    // check HW Timers loading
    //=================================================
    gsm_idle_ram_ctl->hw_timer = HWtimer;
    #endif // GSM_IDLE_RAM

    if ((OSload > 0) && (OSload <= MIN_SLEEP_TIME))
      OSload =0;

    //=================================================
    // check next gauging task for Packet Idle
    //=================================================
    #if L1_GPRS
      min_time_gauging = l1s_get_next_gauging_in_Packet_Idle();
    #else
      min_time_gauging = -1;   // not used
    #endif
    #if (CODE_VERSION!=SIMULATION)
    if (min_time_gauging == 0){
      l1_pwmgr_debug.fail_id = FAIL_SLEEP_DUE_TO_MINTIMEGAUGING;
      l1_pwmgr_debug.fail_ret_val = 0;
    }
    #endif // NOT SIMULATION


    #if (OP_BT == 1)
      hci_ll_status = hci_ll_ok_for_sleep();
    #endif
    // check if immediate activity planned
    // 0 means immediate activity
    // in case big sleep is choosen (sleep mode == FRAME_STOP) because of UART or SIM,
    // return and wait end of this activity (few TDMA frames) then check on next TDMA frames
    // if MS can go in deep sleep
    if (    !OSload
         || !HWtimer
         || !min_time_gauging
		#if (CHIPSET != 15)
         || ((sleep_mode != CLOCK_STOP) && ((l1s.pw_mgr.why_big_sleep == BIG_SLEEP_DUE_TO_UART) || (l1s.pw_mgr.why_big_sleep == BIG_SLEEP_DUE_TO_SIM)))
		#endif
       #if (OP_BT == 1)
         || !hci_ll_status
       #endif
         )
    {



#if (OP_L1_STANDALONE == 0)
	/*GC_Wakeup(); 	OMAPS00134004*/
#endif

#if (CODE_VERSION != SIMULATION)
        OS_system_Unprotect();
        // free System structure
        // Enable all IRQ
        INT_EnableIRQ();
        // Wake up UART
#if (GSM_IDLE_RAM != 0)
        // Traffic controller has to be enabled before calling SER_WakeUpUarts
        // as this function can access the external RAM.
        // Reset the flag that will indicates if an interrup will put the traffic
        // controller ON during that time.
        l1s.gsm_idle_ram_ctl.trff_ctrl_enable_cause_int = FALSE;
        if (!READ_TRAFFIC_CONT_STATE)
        {
          flag_traffic_controller_state = 1;
          CSMI_TrafficControllerOn();
        }
#endif

#if (CHIPSET != 15)
        SER_WakeUpUarts();  // Wake up Uarts
#else
	// To be checked if this needs a change
#endif

#if (GSM_IDLE_RAM != 0)
        // The traffic controller state shall be restored as it was before
        // calling SER_WakeUpUarts. Do not disable it if an interrup occured
        // in between and activated the traffic controller.
        if ((flag_traffic_controller_state == 1) && (l1s.gsm_idle_ram_ctl.trff_ctrl_enable_cause_int == FALSE))
        {
          CSMI_TrafficControllerOff();
        }
        flag_traffic_controller_state = 0;
#endif
#endif
      #if (CODE_VERSION!=SIMULATION)
      l1_trace_fail_sleep(FAIL_SLEEP_OSTIMERGAUGE, l1_pwmgr_debug.fail_id, l1_pwmgr_debug.fail_ret_val);
      #endif
      return;
    }
    //=================================================
    // Select sleep duration ....
    //=================================================
    // remember: -1 means no activity planned
    min_time = OSload;
    //l1a_l1s_com.time_to_next_l1s_task is UW32, min_time is W32. Max value of l1a_l1s_com.time_to_next_l1s_task will be 2p31
    //and ,min_time max value will be 2p30. If min_time > l1a_l1s_com.time_to_next_l1s_task,
    //means MSB of l1a_l1s_com.time_to_next_l1s_task is zero. so, we can use- uw32_store_next_time & 0x7FFFFFFF
    uw32_store_next_time = l1a_l1s_com.time_to_next_l1s_task;

    if (min_time == -1) min_time = (WORD32)uw32_store_next_time;
    //else                        MIN(min_time, (WORD32)l1a_l1s_com.time_to_next_l1s_task)
    else
    {
     if(min_time > l1a_l1s_com.time_to_next_l1s_task) min_time = uw32_store_next_time & 0x7FFFFFFF;
     //else min_time = min_time;
    }
    if (HWtimer != -1)          MIN(min_time, HWtimer)
    if (min_time_gauging != -1) MIN(min_time, min_time_gauging)

    #if (TRACE_TYPE !=0 ) && (TRACE_TYPE != 2) && (TRACE_TYPE != 3)
      // to trace the Wake up source
      // depending of min_time choose the wakeup_type
      l1s.pw_mgr.wakeup_type = WAKEUP_FOR_L1_TASK;
      if (min_time == l1a_l1s_com.time_to_next_l1s_task) l1s.pw_mgr.wakeup_type = WAKEUP_FOR_L1_TASK;
      if (min_time == HWtimer)                           l1s.pw_mgr.wakeup_type = WAKEUP_FOR_HW_TIMER_TASK;
      if (min_time == min_time_gauging)                  l1s.pw_mgr.wakeup_type = WAKEUP_FOR_GAUGING_TASK;
      if (min_time == OSload)                            l1s.pw_mgr.wakeup_type = WAKEUP_FOR_OS_TASK;
    #endif

    //=================================================
    // Choose DEEP or BIG SLEEP
    //=================================================
      if ( ((l1s.pw_mgr.mode_authorized == DEEP_SLEEP) && (sleep_mode == CLOCK_STOP)) ||
           ((l1s.pw_mgr.mode_authorized == ALL_SLEEP)  && (sleep_mode == CLOCK_STOP)) )
      {
         // Check now gauging histogramme or if in inactive period of cell selection
         #if (W_A_DSP_IDLE3 == 1) && (CODE_VERSION!=SIMULATION)
             if (((l1s.pw_mgr.enough_gaug == TRUE) || (l1a_l1s_com.mode == CS_MODE0)) &&
                ( l1s_dsp_com.dsp_ndb_ptr->d_dsp_state == C_DSP_IDLE3))
         #else
            #if (CHIPSET == 12) || (CHIPSET == 15)
             if (((l1s.pw_mgr.enough_gaug == TRUE) || (l1a_l1s_com.mode == CS_MODE0)) &&
                 !CLKM_READ_nIDLE3)
            #else
             if ((l1s.pw_mgr.enough_gaug == TRUE) || (l1a_l1s_com.mode == CS_MODE0))
            #endif
         #endif
           l1s.pw_mgr.sleep_performed = CLOCK_STOP;
         else
         {
           // BIG SLEEP is chosen : check the reason
           l1s.pw_mgr.sleep_performed = FRAME_STOP;
           if ((l1s.pw_mgr.enough_gaug != TRUE) && (l1a_l1s_com.mode != CS_MODE0))
             l1s.pw_mgr.why_big_sleep = BIG_SLEEP_DUE_TO_GAUGING;
           else
             l1s.pw_mgr.why_big_sleep = BIG_SLEEP_DUE_TO_DSP_TRACES;
         }
      }
      if (l1s.pw_mgr.mode_authorized == BIG_SLEEP)
        l1s.pw_mgr.why_big_sleep = BIG_SLEEP_DUE_TO_SLEEP_MODE;

      if ( ((l1s.pw_mgr.mode_authorized == BIG_SLEEP) && (sleep_mode >= FRAME_STOP)) ||
           ((l1s.pw_mgr.mode_authorized >= DEEP_SLEEP) && (sleep_mode == FRAME_STOP)) )
        l1s.pw_mgr.sleep_performed = FRAME_STOP;



#if (CHIPSET != 15)
      if ((previous_sleep == CLOCK_STOP) && (time_from_last_wakeup < 7))
      {
		#if (CODE_VERSION != SIMULATION)
				  OS_system_Unprotect();    // free System structure
				  INT_EnableIRQ();          // Enable all IRQ
			#if (GSM_IDLE_RAM != 0)
					  // Traffic controller has to be enabled before calling SER_WakeUpUarts
					  // as this function can access the external RAM.
					  // Reset the flag that will indicates if an interrup will put the traffic
					  // controller ON during that time.
					  l1s.gsm_idle_ram_ctl.trff_ctrl_enable_cause_int = FALSE;
					  if (!READ_TRAFFIC_CONT_STATE)
					  {
						flag_traffic_controller_state = 1;
						CSMI_TrafficControllerOn();
					  }
			#endif


				SER_WakeUpUarts();  // Wake up Uarts



			#if (GSM_IDLE_RAM != 0)
					  // The traffic controller state shall be restored as it was before
					  // calling SER_WakeUpUarts. Do not disable it if an interrup occured
					  // in between and activated the traffic controller.
					  if ((flag_traffic_controller_state == 1) && (l1s.gsm_idle_ram_ctl.trff_ctrl_enable_cause_int == FALSE))
					  {
						CSMI_TrafficControllerOff();
					  }
					  flag_traffic_controller_state = 0;
			#endif
		#endif // NOT SIMULATION
        return;
      }
#else   // CHIPSET == 15



	  if (l1s.pw_mgr.sleep_performed == CLOCK_STOP)
	  {

     #if (CODE_VERSION != SIMULATION)
      UWORD8 local_sleep_status;


     local_sleep_status = Peripheral_interface[UART_ID](SLEEP_CMD);
     sleep_status = local_sleep_status;
     if(local_sleep_status == 0)
     {
       l1_pwmgr_debug.fail_id = UART_ID;
       l1_pwmgr_debug.fail_ret_val = sleep_status;
     }

     OS_system_Unprotect();
     local_sleep_status = Peripheral_interface[MADC_AS_ID](SLEEP_CMD);  /* Call MADC & Stereo Sleep before I2C */
     OS_system_protect();
     sleep_status &= local_sleep_status;
     if(local_sleep_status == 0)
     {
       l1_pwmgr_debug.fail_id = MADC_AS_ID;
       l1_pwmgr_debug.fail_ret_val = sleep_status;
     }

     local_sleep_status = Peripheral_interface[USB_ID](SLEEP_CMD);
     sleep_status &= local_sleep_status;
     if(local_sleep_status == 0)
     {
       l1_pwmgr_debug.fail_id = USB_ID;
       l1_pwmgr_debug.fail_ret_val = sleep_status;
     }

     local_sleep_status = Peripheral_interface[USIM_ID](SLEEP_CMD);
     sleep_status &= local_sleep_status;
     if(local_sleep_status == 0)
     {
       l1_pwmgr_debug.fail_id = USIM_ID;
       l1_pwmgr_debug.fail_ret_val = sleep_status;
     }

     local_sleep_status = Peripheral_interface[I2C_ID](SLEEP_CMD);
     sleep_status &= local_sleep_status;
     if(local_sleep_status == 0)
     {
       l1_pwmgr_debug.fail_id = I2C_ID;
       l1_pwmgr_debug.fail_ret_val = sleep_status;
     }

     local_sleep_status = Peripheral_interface[LCD_ID](SLEEP_CMD);
     sleep_status &= local_sleep_status;
     if(local_sleep_status == 0)
     {
       l1_pwmgr_debug.fail_id = LCD_ID;
       l1_pwmgr_debug.fail_ret_val = sleep_status;
     }

     local_sleep_status = Peripheral_interface[CAMERA_ID](SLEEP_CMD);
     sleep_status &= local_sleep_status;
     if(local_sleep_status == 0)
     {
       l1_pwmgr_debug.fail_id = CAMERA_ID;
       l1_pwmgr_debug.fail_ret_val = sleep_status;
     }
local_sleep_status = Peripheral_interface[BCI_ID](SLEEP_CMD);
	 sleep_status &= local_sleep_status;
	 if(local_sleep_status == 0)
	 {
	   l1_pwmgr_debug.fail_id = BCI_ID;
	   l1_pwmgr_debug.fail_ret_val = sleep_status;
     }

    #endif // NOT SIMULATION
		 if(!sleep_status)
		 {

	 #if (OP_L1_STANDALONE == 0)
		/*GC_Wakeup(); 	OMAPS00134004*/
	 #endif

            #if (CODE_VERSION != SIMULATION)
			OS_system_Unprotect();
      l1_trace_fail_sleep(FAIL_SLEEP_PERIPH_SLEEP, l1_pwmgr_debug.fail_id, l1_pwmgr_debug.fail_ret_val);
	        #endif // NOT SIMULATION
			local_sleep_status = Peripheral_interface[UART_ID](WAKE_CMD); //OMAPS00090550
			local_sleep_status = Peripheral_interface[USB_ID](WAKE_CMD); //OMAPS00090550
			local_sleep_status = Peripheral_interface[USIM_ID](WAKE_CMD); //OMAPS00090550
			local_sleep_status = Peripheral_interface[I2C_ID](WAKE_CMD);//OMAPS00090550
			local_sleep_status = Peripheral_interface[LCD_ID](WAKE_CMD);//OMAPS00090550
			local_sleep_status = Peripheral_interface[CAMERA_ID](WAKE_CMD);//OMAPS00090550
			local_sleep_status = Peripheral_interface[MADC_AS_ID](WAKE_CMD);//OMAPS00090550
            local_sleep_status = Peripheral_interface[BCI_ID](WAKE_CMD);     //wake up for battery charger interface//OMAPS00090550
			INT_EnableIRQ();
			return;
		 }
	  }

#endif  // CHIPSET == 15

#if (CHIPSET != 15)
     // update previous sleep
       previous_sleep = l1s.pw_mgr.sleep_performed;
#endif


      #if (CODE_VERSION != SIMULATION)

        #if (CHIPSET == 12) || (CHIPSET == 15)
            F_INTH_DISABLE_ONE_IT(C_INTH_FRAME_IT); // mask Frame int.
        #else
            INTH_DISABLEONEIT(IQ_FRAME); // mask Frame int.
        #endif
      #endif

     //=====================================================
     // if CLOCK_STOP : stop RF, TPU, asleep Omega, DPLL, SPI
     // if FRAME_STOP : asleep Omega, SPI
     //=====================================================
     #if (CODE_VERSION != SIMULATION)
       if ( l1s.pw_mgr.sleep_performed == CLOCK_STOP )
       {
           // ==== STop RF and TPU..... ===================

	   //L1_trace_string("Proceeding to Deep Sleep\n");


           l1dmacro_RF_sleep();

           // (*(volatile UWORD16 *)l1s_tpu_com.reg_cmd) =TPU_CTRL_RESET |
           //      TSP_CTRL_RESET |TPU_CTRL_CLK_EN;
           // (*(volatile UWORD16 *)l1s_tpu_com.reg_cmd) =0;

           //===== SET default value for gauging =========
           // If we have come in here during the inactive period of cell
           // selection, then load the ULPD timers with default values
           // (used when the MS lost the network: in this case the deep sleep may be used)
           if (l1a_l1s_com.mode == CS_MODE0)
           {
              l1ctl_pgm_clk32(DEFAULT_HFMHZ_VALUE*8,DEFAULT_32KHZ_VALUE);
           }

			#if (CHIPSET == 15)


				/* These APIs are to be provided by BSP */
			   // Disable_APC_BG();
		        gpio_sleep();  //LCD_Floating Pin Fix
			    DBB_Configure_DS();

				//gpio_sleep();  //LCD_Floating Pin Fix

			#endif


       }
	   #if (CHIPSET == 15)
	   else
	   {
		   //DBB_Configure_BS();  // Not used
	   }
	   #endif


	     #if (CHIPSET == 15)
          // The following command writes '0' into CKM_OCPCLK register in DRP;
          // This is done before disabling DPLL
          // CKM_OCPCLK (R/W) = Address 0xFFFF040C
          //   Bit 0:    0 ?OCP clock is the DCXO clock.
          //             1 ?OCP clock is the divided DSP clock
          //   Bit 31:1  Not Used
          (drp_regs->CKM_OCPCLKL) &= (~(0x1));
          asm(" NOP");
		      asm(" NOP");
        #endif



       //==============================================
       // disable DPLL (do not provide clk to DSP & RIF (RIF))
       //==============================================
       #if ((CHIPSET ==4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15))
         // disable DPLL (do not provide clk to DSP & RIF (Bridge))
         ( * (volatile SYS_UWORD16 *) CLKM_CNTL_CLK) |= CLKM_DPLL_DIS ; /* CLKM_BRIDGE_DIS removed by Ranga*/
       #endif

       //==============================================
       // if CLOCK_STOP or FRAME-STOP : Asleep OMEGA (ABB)
       //==============================================
       #if (ANALOG != 11)
       afc_fix = ABB_sleep(l1s.pw_mgr.sleep_performed, l1s.afc);
       #else
       // Nothing to be done as it should be handled by BSP_TWL3029_Configure_DS/BS
       #endif

       #if (OP_BT == 1)
         hci_ll_go_to_sleep();
       #endif
       //=================================================
       // STop SPI .....
       //=================================================

	#if(CHIPSET != 15)
		*((volatile UWORD16 *)MEM_SPI)&=0xFFFE;  // SPI CLK DISABLED
	#endif
     #endif // NOT SIMULATION


      //=================================================
      // CQ19599: For Calypso+ chipset, extended page mode
      // shall be disabled before entering deep sleep and
      // restored at wake up
      //=================================================
      #if (W_A_CALYPSO_PLUS_SPR_19599 == 1)
         extended_page_mode_state = (BOOL) f_memif_extended_page_mode_read_bit();
         f_memif_extended_page_mode_disable();
      #endif

      //=================================================
      // Init the timer :
      //
      // a margin of 4 frames (>MIN_SLEEP_TIME) is taken
      // when evaluating system loading, because 1 frame
      // is lost for wakeup only, and because  sleep
      // duration less than 1 frame is not worth ....
      //
      //      1         2       3  4  5  6  7     8
      // SLEEP_CTRL   SLEEP   WAKEUP            TASK (RF,Timer, ...)
      //
      //=================================================
      //ULPD Timer can be loaded up to MAX_GSM_TIMER (possible in CS_MODE0)
      if ( l1s.pw_mgr.sleep_performed == CLOCK_STOP )
      {
        // DEEP SLEEP -> need time to setup afc and rf
         wake_up_time = min_time - l1_config.params.rf_wakeup_tpu_scenario_duration;
	 #if (CODE_VERSION == NOT_SIMULATION)
	// Sleep one more TDMA - this is done as part of merging init and TPU control
	      wake_up_time += 1;
	 #endif

      }
      else
        // BIG SLEEP
        wake_up_time = min_time - 1;



      #if (CODE_VERSION != SIMULATION)
        if ( wake_up_time >= MAX_GSM_TIMER)
          ULDP_TIMER_INIT(MAX_GSM_TIMER);
        else
          ULDP_TIMER_INIT(wake_up_time);

        ULDP_TIMER_LD;                                  // Load the timer

        // BUG3060. Clear pending IQ_TGSM from ULPD. This could happen in case ULPD was frozen
        // with zero into its GSM counter. In that case, the interrupt is still pending
        // and if it is not cleared, it wakes the board up just after switching the clock.
        // Clear it into the ULPD...
        // The ULDP_GSM_TIMER_IT_REG is a read only register and is cleared on
        //reading the register.
        temp_clear_intr =(* (volatile UWORD16 *) ULDP_GSM_TIMER_IT_REG) & ULPD_IT_TIMER_GSM;
        // ... and next into the INTH. (must be done in this order

        #if (CHIPSET == 12) || (CHIPSET == 15)
          F_INTH_RESET_ONE_IT(C_INTH_TGSM_IT);
          F_INTH_ENABLE_ONE_IT(C_INTH_TGSM_IT);
        #else
          INTH_RESETONEIT(IQ_TGSM); // clear TDMA IRQ
          INTH_ENABLEONEIT(IQ_TGSM);                      // Unmask ULPD GSM int.
        #endif

        #if (GSM_IDLE_RAM != 0)
         if (READ_TRAFFIC_CONT_STATE)
          {
           CSMI_TrafficControllerOff();
          }
        #endif

        ULDP_TIMER_START;                               // start count down


    #if (GSM_IDLE_RAM_DEBUG == 1)
            (*( volatile unsigned short* )(0xFFFE4802)) &= ~ (1 << 2);    // GPIO-2=0
    #endif

        if ( l1s.pw_mgr.sleep_performed == CLOCK_STOP )
        // DEEP SLEEP
        {
          #if (OP_WCP == 1) && (OP_L1_STANDALONE != 1)
            // specific sleep for WCP
            arm7_deep_sleep();
          #else // NO OP_WCP
            #if (W_A_CALYPSO_BUG_01435 == 1)
              f_arm_sleep_cmd(DEEP_SLEEP);
            #else
			  	//EMIF_SetConfReg ( 0, 0, 2 ,1 ,0);
				asm(" NOP");
				asm(" NOP");
				asm(" NOP");
				asm(" NOP");
            *((volatile UWORD16 *)CLKM_ARM_CLK) &= ~(CLKM_DEEP_SLEEP);  // set deep sleep mode
				asm(" NOP");
				asm(" NOP");
				asm(" NOP");
				asm(" NOP");
// set deep sleep mode in case it is not set back by hardware
                               *((volatile UWORD16 *)CLKM_ARM_CLK) |= (CLKM_DEEP_SLEEP);

			    //EMIF_SetConfReg ( 0, 0, 2 ,0 ,0);
            //  *((volatile UWORD16 *)CLKM_ARM_CLK) &= 0xFFFF;  // set deep sleep mode
           //   *((volatile UWORD16 *)CLKM_ARM_CLK) &= ~(CLKM_MCLK_EN);     // For Debug only


            #endif
          #endif // OP_WCP
        }
        else
        {
            // BIG SLEEP / l1s.pw_mgr.sleep_performed == FRAME_STOP

            //==========================================================
            //Shut down PERIPHERALS clocks UWIRE and ARMIO if authorized
            //==========================================================

	  #if(CHIPSET != 15)
		UWORD16  clocks_stopped; //OMAPS90550- new
            clocks_stopped = (l1s.pw_mgr.clocks & l1s.pw_mgr.modules_status);
            if((clocks_stopped & ARMIO_CLK_CUT) == ARMIO_CLK_CUT)
              *((volatile UWORD16 *)ARMIO_CNTL_REG) &= ~(ARMIO_CLOCKEN);
            if((clocks_stopped & UWIRE_CLK_CUT) == UWIRE_CLK_CUT)
              *((volatile UWORD16 *)(MEM_UWIRE + 0x8)) &= ~(0x0001);
	  #else
	     // Nothing to be done as it is taken care by Locosto_Configure_BS
	  #endif

          #if (W_A_CALYPSO_BUG_01435 == 1)
            f_arm_sleep_cmd(BIG_SLEEP);
          #else

            *((volatile UWORD16 *)CLKM_ARM_CLK) &= ~(CLKM_MCLK_EN);     // set big sleep mode
          #endif
        }
      #else // Simulation part
        l1s.pw_mgr.sleep_duration = wake_up_time;
        hw.deep_sleep_en = 1;
        status = NU_Suspend_Task(&L1S_task);
        // check status value...
        if (status)
        {
          #if (TRACE_TYPE==5)
            sprintf(errormsg,"Error somewhere in the L1S application to suspend : deep sleep\n");
            log_sim_error(ERR);
          #endif
          EXIT;
        }
      #endif // SIMULATION

      //=================================================
      // Wake-up procedure
      //=================================================
      // Restore L1 data base, Nucleus, HW Timers ....
      //=================================================

      #if (GSM_IDLE_RAM_DEBUG == 1)
        (*( volatile unsigned short* )(0xFFFE4802)) |= (1 << 2);    // GPIO-2=1
      #endif


	l1s_wakeup();

       #if (CHIPSET == 15)
         // The following command writes '1' into CKM_OCPCLK register in DRP;
         // This is done after the DPLL is up
         // CKM_OCPCLK (R/W) = Address 0xFFFF040C
         //   Bit 0:    0 ?OCP clock is the DCXO clock.
         //             1 ?OCP clock is the divided DSP clock
         //   Bit 31:1  Not Used
         (drp_regs->CKM_OCPCLKL) |= (0x1);
     	   asm(" NOP");
         asm(" NOP");
       #endif

      l1s.pw_mgr.wakeup_time = l1s.actual_time.fn_mod42432;

      if (l1s.pw_mgr.wakeup_time == sleep_time)
      // sleep duration == 0 -> wakeup in the same frame as sleep
        l1s.pw_mgr.wakeup_type = WAKEUP_ASYNCHRONOUS_SLEEP_DURATION_0;

#if (GSM_IDLE_RAM != 0)
// Update counters with sleep duration -> will be used case expiration in next wake up phase before traffic controller is enabled by msg sending
    gsm_idle_ram_ctl->os_load -= (l1s.pw_mgr.sleep_duration);
    gsm_idle_ram_ctl->hw_timer -= (l1s.pw_mgr.sleep_duration);

if (l1s.pw_mgr.wakeup_type != WAKEUP_FOR_L1_TASK)
{
        if (!READ_TRAFFIC_CONT_STATE)
        {
           CSMI_TrafficControllerOn();
        }
}
#endif
      //=================================================
      //if CLOCK_STOP : restart TPU and RF....
      //=================================================
      #if (CODE_VERSION != SIMULATION)
        if ( l1s.pw_mgr.sleep_performed == CLOCK_STOP )
        {
            // (*(volatile UWORD16 *)l1s_tpu_com.reg_cmd) = TPU_CTRL_CLK_EN;
		UWORD8 local_sleep_status;


			#if (CHIPSET == 15)

               DBB_Wakeup_DS();

			   gpio_wakeup(); //LCD_Floating Pin Fix

				/* These APIs to be provided by BSP */
				//Enable_APC_BG();
				//BT_Wakeup();
				//IRDA_Wakeup();
				local_sleep_status = Peripheral_interface[UART_ID](WAKE_CMD); //OMAPS00090550
				local_sleep_status = Peripheral_interface[USB_ID](WAKE_CMD);//OMAPS00090550
				local_sleep_status = Peripheral_interface[USIM_ID](WAKE_CMD);//OMAPS00090550
				local_sleep_status = Peripheral_interface[I2C_ID](WAKE_CMD);//OMAPS00090550
				local_sleep_status = Peripheral_interface[LCD_ID](WAKE_CMD);//OMAPS00090550
				local_sleep_status = Peripheral_interface[CAMERA_ID](WAKE_CMD);//OMAPS00090550

				OS_system_Unprotect();
				local_sleep_status = Peripheral_interface[MADC_AS_ID](WAKE_CMD);//OMAPS00090550
				local_sleep_status = Peripheral_interface[BCI_ID](WAKE_CMD);  //wake up for battery charger//OMAPS00090550
          			OS_system_protect();
 //added for OMAPS00090550 warning removal
     if(local_sleep_status == 0)
     {
       l1_pwmgr_debug.fail_ret_val = local_sleep_status;
     }
     //upto this OMAPS00090550


			#endif
            l1dmacro_RF_wakeup();

        }

        #if ((CHIPSET ==4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
          // enable DPLL (provide clk to DSP & RIF(Bridge) in small/big sleep)
          // On CALYPSO, BRIDGE clock can be cut according to the ARM sleep mode even during DMA transfer
          ( * (volatile SYS_UWORD16 *) CLKM_CNTL_CLK) &= ~(CLKM_DPLL_DIS | CLKM_BRIDGE_DIS);
        #elif (CHIPSET == 12)
          // Nothing to be done because if DSP wants clock, it will exit from IDLE3 mode, which wakes up the DPLL
	#elif (CHIPSET == 15)
	( * (volatile SYS_UWORD16 *) CLKM_CNTL_CLK) &= ~(CLKM_DPLL_DIS);
        #endif

        //=================================================
        //if CLOCK_STOP or FRAME-STOP : ReStart SPI
        //=================================================
	#if(CHIPSET != 15)
		*((volatile UWORD16 *)MEM_SPI)|=0x0001;  // SPI CLK ENABLED
	#endif

        //=================================================
        // Wake up ABB
        //=================================================
        #if (ANALOG != 11)
        ABB_wakeup(l1s.pw_mgr.sleep_performed, l1s.afc);
        #else
        // Nothing to be done here as it will be handled by BSP_TWL3029_Wakeup_DS/BS
        #endif

        #if (OP_BT == 1)
          hci_ll_wake_up();
        #endif
      #endif //CODE VERSION

      //=================================================
      // CQ19599: For Calypso+ chipset, restore the extended
      // page mode if it was enabled before entering in sleep
      //=================================================
      #if (W_A_CALYPSO_PLUS_SPR_19599 == 1)
         if ( extended_page_mode_state != 0 )
            f_memif_extended_page_mode_enable();
      #endif

      #if (OP_L1_STANDALONE == 0)
		/*GC_Wakeup(); 	OMAPS00134004*/
      #endif

      #if (CODE_VERSION != SIMULATION)
        //=================================================
        // enable IRQ
        //=================================================
        OS_system_Unprotect();
      #endif

       #if (TRACE_TYPE != 0)
         if (l1a_l1s_com.mode != CS_MODE0) // in this mode the trace prevent from going to deep sleep due to UART activity
         {
           #if (GSM_IDLE_RAM == 0)
             l1_trace_sleep(sleep_time,l1s.actual_time.fn_mod42432,l1s.pw_mgr.sleep_performed,l1s.pw_mgr.wakeup_type,l1s.pw_mgr.why_big_sleep, l1s.pw_mgr.wake_up_int_id);
           #else
             l1_trace_sleep_intram(sleep_time,l1s.actual_time.fn_mod42432,l1s.pw_mgr.sleep_performed,l1s.pw_mgr.wakeup_type,l1s.pw_mgr.why_big_sleep, l1s.pw_mgr.wake_up_int_id);
             #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
               l1s_trace_mftab();
             #endif
           #endif
         }
         l1s.pw_mgr.wake_up_int_id = 0;
       #endif

       #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
          trace_info.sleep_performed = TRUE;
       #endif

#if (CODE_VERSION != SIMULATION)

        //=================================================
        // enable IRQ
        //=================================================
        INT_EnableIRQ();

        //=================================================
        // Be careful:in case of asynchronous wake-up after sleep
        // an IT_TDMA may be unmasked and executed just after OS_system_Unprotect().
        // As we already are inside an hisr(), it implies the execution of an another hisr().
        // In order to avoid issues with the execution of an hisr() inside the hisr()
        // do not add code here after !!!
        // if possible respect this rule !
        //=================================================

        //=================================================
        // wake-up UARTs
        //this function must be call after the UART interrupt,
        //it means after the function INT_EnableIRQ()
        //=================================================
        {
#if (GSM_IDLE_RAM != 0)
          // Traffic controller has to be enabled before calling SER_WakeUpUarts
          // as this function can access the external RAM.
          // Reset the flag that will indicates if an interrup will put the traffic
          // controller ON during that time.
          l1s.gsm_idle_ram_ctl.trff_ctrl_enable_cause_int = FALSE;
          if (!READ_TRAFFIC_CONT_STATE)
          {
            flag_traffic_controller_state = 1;
            CSMI_TrafficControllerOn();
          }
#endif



#if (CHIPSET != 15)
        SER_WakeUpUarts();  // Wake up Uarts
#else
	// To be checked if this needs a change
#endif


#if (GSM_IDLE_RAM != 0)
          // The traffic controller state shall be restored as it was before
          // calling SER_WakeUpUarts. Do not disable it if an interrup occured
          // in between and activated the traffic controller.
          if ((flag_traffic_controller_state == 1) && (l1s.gsm_idle_ram_ctl.trff_ctrl_enable_cause_int == FALSE))
          {
            CSMI_TrafficControllerOff();
          }
          flag_traffic_controller_state = 0;
#endif
        }
#endif // NOT SIMULATION
  }
}

// l1s_wakeup()                                          */
// Description: wake-up of the MCU from GSM Timer it OR  unscheduled wake-up
// This function read the TPU timer and fix the :
// - system clock
// - Nucleus timers
// - L1 frame counter
// - L1 next task counter
// - Hardware timers

void l1s_wakeup(void)
{
#if (CODE_VERSION != SIMULATION)
  if (l1_config.pwr_mngt == PWR_MNGT)
  {
      // Restore interrupts ....

      #if (CHIPSET == 12) || (CHIPSET == 15)
          // mask TGSM int.
          F_INTH_DISABLE_ONE_IT(C_INTH_TGSM_IT);
      #else
          INTH_DISABLEONEIT(IQ_TGSM);   // mask TGSM int.
      #endif

      #if (CHIPSET == 12) || (CHIPSET == 15)
        l1s.pw_mgr.wake_up_int_id = ((* (SYS_UWORD16 *) C_INTH_B_IRQ_REG) & C_INTH_SRC_NUM);// For debug: Save IRQ that causes the waking up
        if ( l1s.pw_mgr.wake_up_int_id >= 256 )
          l1s.pw_mgr.wake_up_int_id = ((* (SYS_UWORD16 *) C_INTH_B_FIQ_REG) & C_INTH_SRC_NUM)+100;
      #else
        l1s.pw_mgr.wake_up_int_id = ((* (SYS_UWORD16 *) INTH_B_IRQ_REG) & INTH_SRC_NUM);// For debug: Save IRQ that causes the waking up
        if ( l1s.pw_mgr.wake_up_int_id >= 256 )
          l1s.pw_mgr.wake_up_int_id = ((* (SYS_UWORD16 *) INTH_B_FIQ_REG) & INTH_SRC_NUM)+100;
      #endif

      // clear pending IQ_FRAME it and unmask it
      #if (CHIPSET == 12) || (CHIPSET == 15)
        F_INTH_RESET_ONE_IT(C_INTH_FRAME_IT);
        F_INTH_ENABLE_ONE_IT(C_INTH_FRAME_IT);         // Unmask FRAME int.
      #else
        INTH_RESETONEIT(IQ_FRAME); // clear TDMA IRQ
        INTH_ENABLEONEIT(IQ_FRAME);         // Unmask FRAME int.
      #endif

      #if (CHIPSET == 8)
        // if deep sleep
        if ( l1s.pw_mgr.sleep_performed == CLOCK_STOP )
        {
          UWORD8 i;

          // Loop with check whether DPLL is locked: 100 us max.
          for (i=0;i<16;i++)
          {
            if (DPLL_READ_DPLL_LOCK)
              break;
          }
          wait_ARM_cycles(convert_nanosec_to_cycles(50000));  // 50us

          // Enable DPLL
          //--------------------------------------------------
          DPLL_SET_PLL_ENABLE;

          // Loop with check whether DPLL is locked: 100 us max.
          for (i=0;i<16;i++)
          {
            if (DPLL_READ_DPLL_LOCK)
              break;
          }
          wait_ARM_cycles(convert_nanosec_to_cycles(50000));  // 50us
        } // if deep sleep

      #endif // CHIPSET == 8

      //=================================================
      //Restart PERIPHERALS clocks if necessary after a big sleep period
      // WARNING: restart other clocks modules!!!
      //=================================================


	#if(CHIPSET == 15)
	  if(l1s.pw_mgr.sleep_performed == FRAME_STOP )
	  {

		  //ABB_Wakeup_BS();  //Not Used
		  //DBB_Wakeup_BS();	//Not Used
	  }
	#else
      // if big sleep
      if ( l1s.pw_mgr.sleep_performed == FRAME_STOP )
      {

        UWORD16  clocks_stopped;
        clocks_stopped = (l1s.pw_mgr.clocks & l1s.pw_mgr.modules_status);
        if((clocks_stopped & ARMIO_CLK_CUT) == ARMIO_CLK_CUT)
          *((volatile UWORD16 *)ARMIO_CNTL_REG) |= ARMIO_CLOCKEN;
        if((clocks_stopped & UWIRE_CLK_CUT) == UWIRE_CLK_CUT)
          *((volatile UWORD16 *)(MEM_UWIRE + 0x8)) |= 0x0001;

      }
	#endif


      /***************************************************/
      /* Compute effective sleeping time ....            */
      /*                                                 */
      /* sleep duration is                               */
      /* - TIMER_INIT                                    */
      /* - or TIMER_INIT - TIMER_VALUE                   */
      /*                                                 */
      /* "frame_adjust" = TRUE for unschedules wake-up   */
      /*                  FALSE for scheduled wake-up    */
      /***************************************************/
      l1s.pw_mgr.frame_adjust = l1s_compute_wakeup_ticks();

      #if (TRACE_TYPE !=0 ) && (TRACE_TYPE != 2) && (TRACE_TYPE != 3)
        if ((l1s.pw_mgr.frame_adjust == TRUE))
          l1s.pw_mgr.wakeup_type = WAKEUP_BY_ASYNC_INTERRUPT;
      #endif


      /* Fix Frame                                       */

      l1s_recover_Frame();


      /* Fix Hardware Timers                             */
      /*                                                 */
      /* GSM 1.0 : ntd - timer clock not cut             */
      /*                                                 */
      /* GSM 1.5 : deep sleep - need to fix timers       */

      if (l1s.pw_mgr.sleep_performed == CLOCK_STOP)
        l1s_recover_HWTimers();


      /* Fix Os                                          */

      if (Cust_recover_Os()) l1s.pw_mgr.Os_ticks_required = TRUE;
  }
#else  // SIMULATION part
   // update L1 timers (FN,...)
   l1s_recover_Frame();
#endif
}



/* l1s_wakeup_adjust()                                   */
/* Description: 1 frame adjust a fter unscheduled wake-up */
/* This function fix the :                               */
/* - system clock                                        */
/* - Nucleus timers                                      */
/* - L1 frame counter                                    */
/* - L1 next task counter                                */
/* - Hardware timers                                     */


void l1s_wakeup_adjust ()
{
#if (CODE_VERSION != SIMULATION)
  if (l1_config.pwr_mngt == PWR_MNGT)
  {

    UWORD32 previous_sleep_time;

    /***************************************************/
    // Freeze GSM Timer ....                           */
    /***************************************************/
    ULDP_TIMER_FREEZE;

    /***************************************************/
    // Compute effective sleeping time ....
    //
    // compute sleep duration
    // - TIMER_INIT
    // - or TIMER_INIT - TIMER_VALUE
    /***************************************************/
    // save sleep duration that was computed at "unscheduled wakeup"
    previous_sleep_time = l1s.pw_mgr.sleep_duration;

    l1s_compute_wakeup_ticks();

    // reset flag for adjustment request ....
    l1s.pw_mgr.frame_adjust = FALSE;

    // fix sleep duration
    // => compute difference with duration computed at
    //    "unscheduled wakeup"
    l1s.pw_mgr.sleep_duration -= previous_sleep_time;

    // adjust system with 1 frame IF NECESSARY ....
    if (l1s.pw_mgr.sleep_duration)
    {
      /***************************************************/
      /* Fix Frame                                       */
      /***************************************************/
      l1s_recover_Frame();

      /***************************************************/
      /* Fix Os                                          */
      /***************************************************/
      if (Cust_recover_Os()) l1s.pw_mgr.Os_ticks_required = TRUE;
    }
  }
#endif
}


/*-------------------------------------------------------*/
/* l1s_compute_wakeup_Ticks()                            */
/*-------------------------------------------------------*/
/*                                                       */
/* Description: wake-up                                  */
/* ------------                                          */
/* This function compute the sleep duration according to */
/* current value of count down counter.                  */
/* - if TIMER_VALUE = 0 it returns TIMER_INIT            */
/* - else               it returns TIMER_INIT-TIMER_VALUE*/
/*                                                       */
/*-------------------------------------------------------*/
BOOL l1s_compute_wakeup_ticks(void)
{
 UWORD16 temp_clear_intr;
#if (CODE_VERSION != SIMULATION)
  if (l1_config.pwr_mngt == PWR_MNGT)
  {
     // read current value of count down counter
     l1s.pw_mgr.sleep_duration  = READ_ULDP_TIMER_VALUE;

     // if count down=0 it's a scheduled wake-up....
     if (l1s.pw_mgr.sleep_duration == 0)
     {
        // read sleeping planned value in TPU INIT register
       l1s.pw_mgr.sleep_duration  = READ_ULDP_TIMER_INIT;
       // INTH is different from the ULPD interrupt -> aynchronous wakeup
     #if (CHIPSET == 12) || (CHIPSET == 15)
       if (l1s.pw_mgr.wake_up_int_id != C_INTH_TGSM_IT)
     #else
       if (l1s.pw_mgr.wake_up_int_id != IQ_TGSM)
     #endif
            {
         l1s.pw_mgr.wakeup_type = WAKEUP_ASYNCHRONOUS_ULPD_0;
         // RESET IT_ULPD in ULPD module
        // The ULDP_GSM_TIMER_IT_REG is a read only register and is cleared on reading the register
        temp_clear_intr =(* (volatile UWORD16 *) ULDP_GSM_TIMER_IT_REG) & ULPD_IT_TIMER_GSM;
         #if (CHIPSET == 12) || (CHIPSET == 15)
           // RESET IQ_TGSM (IT_ULPD) in IT register
           F_INTH_RESET_ONE_IT(C_INTH_TGSM_IT);
           // RESET IQ_FRAME in IT register
           F_INTH_RESET_ONE_IT(C_INTH_FRAME_IT);
           l1s.pw_mgr.wake_up_int_id = C_INTH_TGSM_IT;
         #else
           // RESET IQ_TGSM (IT_ULPD) in IT register
           INTH_RESETONEIT(IQ_TGSM);
           // RESET IQ_FRAME in IT register
           INTH_RESETONEIT(IQ_FRAME);
           l1s.pw_mgr.wake_up_int_id = IQ_TGSM;
         #endif
         return(FALSE);
       }
       else
         return(FALSE);
     }
     else // Unscheduled wakeup
     {
       // read sleeping planned value in TPU INIT register & compute time elapsed
       l1s.pw_mgr.sleep_duration  = READ_ULDP_TIMER_INIT - l1s.pw_mgr.sleep_duration;
       return(TRUE);
     }
  }
#endif
 return(FALSE);//omaps00090550
}

/*-------------------------------------------------------*/
/* l1s_recover_Frame()                                   */
/*-------------------------------------------------------*/
/*                                                       */
/* Description: adjust layer1 data from sleep duration   */
/* ------------                                          */
/*-------------------------------------------------------*/
void l1s_recover_Frame(void)
{
  if (l1_config.pwr_mngt == PWR_MNGT)
  {
    /***************************************************/
    /* Fix Frame counters .                            */
    /***************************************************/
    l1s.debug_time += l1s.pw_mgr.sleep_duration;       // used for debug and by L3 scenario.

    // Time...
    // Update "actual time".
    l1s_increment_time(&(l1s.actual_time), l1s.pw_mgr.sleep_duration);

    // Update "next time".
    l1s.next_time      = l1s.actual_time;
    l1s_increment_time(&(l1s.next_time), 1);        // Next time is actual_time + 1

    #if L1_GPRS
      // Update "next plus time".
      l1s.next_plus_time = l1s.next_time;
      l1s_increment_time(&(l1s.next_plus_time), 1); // Next_plus time is next_time + 1
    #endif

    #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
      trace_fct(CST_L1S_ADJUST_TIME, (UWORD32)(-1));
    #endif

    // Multiframe table...
    // Increment active frame % mftab size.
    l1s.afrm = (l1s.afrm + l1s.pw_mgr.sleep_duration) % MFTAB_SIZE;

    // Control function counters...
    // Increment frame count from last AFC update.
    l1s.afc_frame_count+= l1s.pw_mgr.sleep_duration;
    // reset counter to mask SNR/TOA results for 2 fr.
    #if (TOA_ALGO == 2)
      l1s.toa_var.toa_snr_mask=0;
    #else
      l1s.toa_snr_mask=0;
    #endif

    /***************************************************/
    /* Fix next L1S task counter                       */
    /***************************************************/
    // Decrement time to next L1S task.
    if((l1a_l1s_com.time_to_next_l1s_task > 0) &&
       (l1a_l1s_com.time_to_next_l1s_task < MAX_FN))
      l1a_l1s_com.time_to_next_l1s_task -= l1s.pw_mgr.sleep_duration;
  } // l1_config.pwr_mngt == PWR_MNGT
}


/*-------------------------------------------------------*/
/* l1s_recover_HWTimers()                                */
/*-------------------------------------------------------*/
/*                                                       */
/* Description: adjust hardware timers from sleep        */
/* ------------ duration                                 */
/*                                                       */
/* Timers clocks are enabled after VTCX0+SLICER+13MHZ    */
/* setup times. So sleep duration is :                   */
/* GSM TIMER - SETUP_FRAME + SETUP_SLICER + SETUP_VTCXO  */
/* + SETUP_CLK13                                         */
/*-------------------------------------------------------*/

void l1s_recover_HWTimers(void)
{
#if (CODE_VERSION != SIMULATION)

  #define SETUP_FRAME_IN_CLK32    (SETUP_FRAME*4.615*32.768)
  #if (CHIPSET == 15)
    #define DELTA_TIME              (0)
  #else
    #define DELTA_TIME              (SETUP_FRAME_IN_CLK32 -SETUP_SLICER - SETUP_VTCXO)
  #endif


  if (l1_config.pwr_mngt == PWR_MNGT)
  {
    WORD32  timer1,timer2,timer;
    #if (CHIPSET == 12) || (CHIPSET == 15)
      WORD32 timer_sec;
    #endif
    UWORD16 cntlreg;
    UWORD16 modereg;
    double duration;












    //WORD32 old;- OMAPS 90550 new

    // read Hercules Timers & Watchdog
    //=================================================
    // Tint = Tclk * (LOAD_TIM+1) * 2^(PTV+1)
    // Tclk = 1.2308us for Fclk=13Mhz
    // PTV  = 7 (pre-scaler field)
    //-------------------------------------------------

    cntlreg = Dtimer1_Get_cntlreg();
    if ( (cntlreg & D_TIMER_RUN) == D_TIMER_RUN)
    {
      cntlreg = cntlreg&0x1F;
      cntlreg >>= 2;   // take PTV
      cntlreg = 1 << (cntlreg+1);  // compute 2^(PTV+1)
      // convert sleep duration in HWTimers ticks....
      duration = (l1s.pw_mgr.sleep_duration * 4.615 - (DELTA_TIME/32.768)) / (cntlreg * 0.0012308);
      if (duration < 0.0){
        duration = 0.0; // This needs to be done for all the timers
      }
      timer1 = Dtimer1_ReadValue() - (UWORD16) duration;

      Dtimer1_Start(0);
      Dtimer1_WriteValue(timer1);
      Dtimer1_Start(1);
    }

    cntlreg = Dtimer2_Get_cntlreg();
    if ( (cntlreg & D_TIMER_RUN) == D_TIMER_RUN)
    {
      cntlreg = cntlreg&0x1F;
      cntlreg >>= 2;   // take PTV
      cntlreg = 1 << (cntlreg+1);
      // convert sleep duration in HWTimers ticks....
      duration = (l1s.pw_mgr.sleep_duration * 4.615 - (DELTA_TIME/32.768)) / (cntlreg * 0.0012308);
      if (duration < 0.0){
        duration = 0.0; // This needs to be done for all the timers
      }
      timer2 = Dtimer2_ReadValue() - (UWORD16) duration;
      Dtimer2_Start(0);
      Dtimer2_WriteValue(timer2);
      Dtimer2_Start(1);
    }

    cntlreg = TIMER_Read(0);
    modereg = TIMER_Read(2);
    if ( (cntlreg & TIMER_ST) || (modereg & TIMER_WDOG))
    {
      // in watchdog mode PTV is forced to 7
      if ( modereg & TIMER_WDOG )
      cntlreg |= TIMER_PTV;

      cntlreg = (cntlreg & TIMER_PTV) >> 9;   // take PTV
      cntlreg = 1 << (cntlreg+1);
      // convert sleep duration in HWTimers ticks....
      duration = (l1s.pw_mgr.sleep_duration * 4.615 - (DELTA_TIME/32.768)) / (cntlreg * 0.001078);

      timer = TIMER_ReadValue() - (UWORD16) duration;
      TIMER_START_STOP(0);
      TIMER_WriteValue(timer);
      TIMER_START_STOP(1);
    }

    #if (CHIPSET == 12) || (CHIPSET == 15)
      cntlreg = TIMER_SEC_Read(0);
      modereg = TIMER_SEC_Read(2);
      if ( (cntlreg & TIMER_ST) || (modereg & TIMER_WDOG))
      {
        // in watchdog mode PTV is forced to 7
        if ( modereg & TIMER_WDOG )
        cntlreg |= TIMER_PTV;

        cntlreg = (cntlreg & TIMER_PTV) >> 9;   // take PTV
        cntlreg = 1 << (cntlreg+1);
        // convert sleep duration in HWTimers ticks....
        duration = (l1s.pw_mgr.sleep_duration * 4.615 - (DELTA_TIME/32.768)) / (cntlreg * 0.001078);

        timer_sec = TIMER_SEC_ReadValue() - (UWORD16) duration;
        TIMER_SEC_START_STOP(0);
        TIMER_SEC_WriteValue(timer_sec);
        TIMER_SEC_START_STOP(1);
      }
    #endif

  }
#endif
}
/*-------------------------------------------------------*/
/* l1s_get_next_gauging_in_Packet_Idle()                 */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/*  return the nbr of frames before the next gauging     */
/*  return -1 means no activity planned                  */
/*-------------------------------------------------------*/
#if L1_GPRS
  UWORD32 l1s_get_next_gauging_in_Packet_Idle(void)
  {
    WORD32 next_gauging;

    // gauging performed with Normal Paging (we are in Idle mode)
    if (l1a_l1s_com.l1s_en_task[NP] == TASK_ENABLED)
      return ((UWORD32)(-1)); // no activity planned   //omaps00090550

    // we are not in Packet Idle Mode
    if (l1a_l1s_com.l1s_en_task[PNP] != TASK_ENABLED)
      return ((UWORD32)(-1)); // no activity planned   //omaps00090550

    next_gauging = l1s.next_gauging_scheduled_for_PNP - l1s.actual_time.fn ;
    if (next_gauging < 0)
      next_gauging+=MAX_FN;

    if (next_gauging <= MIN_SLEEP_TIME)
      return(0);

    return (next_gauging);
  }
#endif
/*-------------------------------------------------------*/
/* l1s_gauging_decision_with_PNP()                       */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/*                                                       */
/*-------------------------------------------------------*/
#if L1_GPRS
  BOOL l1s_gauging_decision_with_PNP(void)
  {
  #define TWO_SECONDS_IN_FRAME (UWORD16)(2000/4.615)
  WORD32  time_to_next_gauging=0;  //changed to WORD32- sajal
    // It's time to perform the next gauging
  time_to_next_gauging = l1s.next_gauging_scheduled_for_PNP - l1s.actual_time.fn;
  if (time_to_next_gauging < 0)
  {
    time_to_next_gauging += MAX_FN;
  }

  if( (time_to_next_gauging == 0) || (time_to_next_gauging > TWO_SECONDS_IN_FRAME))
  {

      l1s.next_gauging_scheduled_for_PNP = l1s.actual_time.fn + TWO_SECONDS_IN_FRAME;
      if (l1s.next_gauging_scheduled_for_PNP >= MAX_FN) l1s.next_gauging_scheduled_for_PNP -= MAX_FN;
      return (TRUE);
    }

    return (FALSE); // do not perform gauging
  }
#endif
/*-------------------------------------------------------*/
/* l1s_gauging_decision_with_NP()                        */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/*                                                       */
/*-------------------------------------------------------*/
BOOL l1s_gauging_decision_with_NP(void)
{

  static UWORD8 time_to_gaug;

  // a paging is scheduled or , was scheduled but discarded by a higher priority task
  if (l1s.pw_mgr.paging_scheduled == TRUE)
  {
    l1s.pw_mgr.paging_scheduled = FALSE; // reset Flag.

    // A gauging session is needed : start gauging session with this paging bloc !

        //Nina modify to save power, not forbid deep sleep, only force gauging in next paging
#if 0
	if (l1s.pw_mgr.enough_gaug != TRUE)
    time_to_gaug = 0;
#else
	  if ((l1s.pw_mgr.enough_gaug != TRUE)||(l1s.force_gauging_next_paging_due_to_CCHR == 1))
       {
       time_to_gaug = 0;
	   l1s.force_gauging_next_paging_due_to_CCHR = 0;
	  	}
#endif	  
    if (time_to_gaug > 0)
    {
      time_to_gaug--; // perform the gauging with an another paging.
    }
    else // perform the gauging with this paging
    {
      if (l1s.task_status[NP].current_status == ACTIVE )
      {
        time_to_gaug = GAUG_VS_PAGING_RATE[l1a_l1s_com.bs_pa_mfrms-2]-1;

        return (TRUE); // gauging allowed
      }
      else  // a gauging is scheduled to be perform here but the paging is missing
      {     // (paging discarded by a higher priority task ?)
         l1s.pw_mgr.enough_gaug= FALSE;    // forbid Deep sleep until next gauging
      }
    }
  }
  return (FALSE);  // gauging not allowed
}

/*************************************************************/
/* Gauging task management :                                 */
/*                                                           */
/* CALYPSO                                                   */
/*                                                           */
/*  9  8  7  6  5  4  3  2  1  0                             */
/* C0 C1 C2 C3 C4  W  R  -  -  -                             */
/*  |                       |                                */
/*  |                       |                                */
/*  |_ start gauging        |_ stop gauging                  */
/*                                                           */
/*OTHERS:                                                    */
/*                                                           */
/* 11 10  9  8  7  6  5  4  3  2  1  0                       */
/* C0 C1 C2 C3 C4  W  R  -  -  -  -  -                       */
/*  |  |  |                       |  |                       */
/*  |  |  |_ start gauging        |_ stop gauging            */
/*  |  |                          |  |                       */
/*  |  |_ (ITCOM)                 |  |(ITCOM)                */
/*  |                             |                          */
/*  |_ pgm PLL                    |_restore PLL              */
/*                                                           */
/*                                                           */
/*************************************************************/
void l1s_gauging_task(void)
{
    if (l1_config.pwr_mngt == PWR_MNGT)
    {
      /*************************************************************/
      if (l1s.pw_mgr.gauging_task == ACTIVE)
      {
        /*************************************************************/
        // COUNT = 10 ==> PLL is at 65 Mhz, start the gauging
        /*************************************************************/
        #if (CHIPSET==7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15)
          // the gauging was started with the begining of the paging
        #else
          if (l1s.pw_mgr.gaug_count == (l1s.pw_mgr.gaug_duration-1))
          {
            #if (CODE_VERSION != SIMULATION)
              ULDP_GAUGING_START; // start gauging
            #endif

            #if (TRACE_TYPE != 0)
              #if (GSM_IDLE_RAM != 0)
                l1_trace_gauging_intram();
              #else
                l1_trace_gauging();
              #endif
            #endif
          }
        #endif

        l1s.pw_mgr.gaug_count--;  // decrement counter


        // When a MISC task is enabled L1S must be ran every frame
        // to be able to enable the frame interrupt for DSP
        l1a_l1s_com.time_to_next_l1s_task = 0;
      }

      /*************************************************************/
      // REQUEST A GAUGING PROCESS ON EACH PAGING BLOCK
      // IN IDLE MODE .....
      /*************************************************************/

      else if (l1s.pw_mgr.gauging_task == INACTIVE )
      {
        BOOL decision = FALSE;

        if (l1a_l1s_com.l1s_en_task[NP] == TASK_ENABLED)
          decision = l1s_gauging_decision_with_NP();
        #if L1_GPRS
          else
            if (l1a_l1s_com.l1s_en_task[PNP] == TASK_ENABLED)
              decision = l1s_gauging_decision_with_PNP();
        #endif

        if (decision == TRUE)
        {
            // gauging duration
            l1s.pw_mgr.gaug_count = l1s.pw_mgr.gaug_duration;

            #if (CHIPSET==7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15)
              // start ULPD gauging immediately with Calypso because we needn't IT_COM.
              #if (CODE_VERSION != SIMULATION)
                ULDP_GAUGING_START;
                #if (CHIPSET == 12) || (CHIPSET == 15)
                  // Force the DPLL to be active
                  ( * (volatile SYS_UWORD16 *) CLKM_CNTL_CLK) &= ~(CLKM_DPLL_DIS);
                #endif
              #endif

              #if (TRACE_TYPE != 0)
                #if (GSM_IDLE_RAM != 0)
                  l1_trace_gauging_intram();
                #else
                  l1_trace_gauging();
                #endif
              #endif
            #endif

            // DSP programmation .......
            #if (DSP >= 33)
              #if (CHIPSET==4)
                l1s_dsp_com.dsp_ndb_ptr->d_pll_config |= B_32KHZ_CALIB;
              #endif
            #else
              l1s_dsp_com.dsp_ndb_ptr->d_pll_clkmod1 = CLKMOD2;  // IDLE1 only for DSP
            #endif

            l1s.pw_mgr.gauging_task = ACTIVE;
        }
      }
    }
}
/*-------------------------------------------------------*/
/* l1s_gauging_task_end()                               */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/*      stop the gauging activity                        */
/*-------------------------------------------------------*/
void l1s_gauging_task_end(void)
{
    if (l1_config.pwr_mngt == PWR_MNGT)
    {
      /*************************************************************/
      if (l1s.pw_mgr.gauging_task == ACTIVE)
      {
        /*************************************************************/
        // COUNT = 1 ==> stop the gauging and free DSP idle modes....
        /*************************************************************/
        if (l1s.pw_mgr.gaug_count == 1)
        {
            // wait for end of gauging interrupt ...
           l1s.pw_mgr.gauging_task = WAIT_IQ;

           // Unmask ULPD GAUGING int.
           #if (CODE_VERSION != SIMULATION)
             #if (CHIPSET == 12) || (CHIPSET == 15)
                F_INTH_ENABLE_ONE_IT(C_INTH_ULPD_GAUGING_IT);
             #else
                INTH_ENABLEONEIT(IQ_ULPD_GAUGING);
             #endif
             ULDP_GAUGING_STOP;  // stop ULPD gauging
              #if (CHIPSET == 12) || (CHIPSET == 15)
                // Allow the DPLL to be cut according to ARM sleep mode
                //( * (volatile SYS_UWORD16 *) CLKM_CNTL_CLK) |= (CLKM_DPLL_DIS);
              #endif
           #endif

           // DSP programmation : free IDLE modes...
           #if (DSP >= 33)
             #if (CHIPSET==4)
              l1s_dsp_com.dsp_ndb_ptr->d_pll_config &= ~B_32KHZ_CALIB;
             #endif
           #else
              l1s_dsp_com.dsp_ndb_ptr->d_pll_clkmod1 = CLKMOD1;
           #endif


           #if (CODE_VERSION == SIMULATION)
             // in order to simulate the Gauging interrupt
             GAUGING_Handler();

             #if (TRACE_TYPE==5)
                  trace_ULPD("Stop Gauging", l1s.actual_time.fn);
             #endif
           #endif
        }
      }
    }
}

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif



