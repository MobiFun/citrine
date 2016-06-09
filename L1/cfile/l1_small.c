

#include "l1sw.cfg"
#if (OP_L1_STANDALONE == 0)
  #include "debug.cfg"
  #include "rv_swe.h"
#endif
#if (OP_L1_STANDALONE == 1)
#include "general.h"
#endif
#include "l1_macro.h"
#include "l1_confg.h"

#if (CODE_VERSION == SIMULATION)
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"
  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif  
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
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "l1_varex.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  
  #include <stdio.h>
  #include "sim_cfg.h"
  #include "sim_cons.h"
  #include "sim_def.h"
  #include "sim_var.h"

#else
  #include <string.h>

  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif  
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
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "l1_varex.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include "tpudrv.h"

#endif

#if (W_A_CALYPSO_PLUS_SPR_19599 == 1)
  #include "sys_memif.h"
#endif

#if (CHIPSET == 15)
#include "sys_inth.h"
#include "bspI2c.h"
#include "clkm.h"

// This could be removed if i2c_pwr_interface is functional
extern volatile Bool bspI2c_busLocked[BSP_I2C_NUM_DEVICES] ;
#endif

/* NEW COMPILER MANAGEMENT
 * With compiler V3.00, the .text section must be explicitely
 * defined.
 * Else the following code will be put in the .cinit section.
 * The change is applied to all compilers.
 */
  asm("           .sect   \".text\" ");


UWORD8 *mode_authorized = &(l1s.pw_mgr.mode_authorized);
UWORD8 *switch_PWR_MNGT = &(l1_config.pwr_mngt);

#if (W_A_CALYPSO_BUG_01435 == 1)

  asm("SMALL_SLEEP  .equ  01h");
  asm("BIG_SLEEP    .equ  02h");
  asm("DEEP_SLEEP   .equ  03h");

  void f_arm_sleep_cmd(UWORD8 d_sleep_mode) 
  {
    asm(" LDR       R3, MPU_FREE_REG");

    // here below the C code:
    // if ((d_sleep_mode == SMALL_SLEEP) || (d_sleep_mode == BIG_SLEEP)) 
    //   * (volatile UWORD16 *) 0xfffffd00 &= 0xfffe;
    // else 
    //   if (d_sleep_mode == DEEP_SLEEP) 
    //     * (volatile UWORD16 *) 0xfffffd00 &= 0xefff;

    asm(" CMP       R0, #SMALL_SLEEP");
    asm(" BEQ       Small_or_Big_Sleep");
    asm(" CMP       R0, #BIG_SLEEP");
    asm(" BEQ       Small_or_Big_Sleep");
    asm(" CMP       R0, #DEEP_SLEEP");
    asm(" BXNE      LR");

    asm("Deep_Sleep:        ");
    asm(" LDR       R0, CLKM_CNTL_ARM_CLK_REG");
    asm(" LDRH      R12, [R0, #0]");
    asm(" AND       R1, R12, #255");
    asm(" AND       R12, R12, #61184");
    asm(" ORR       R12, R1, R12");
    asm(" STRH      R12, [R0, #0]");
    asm(" STMIA     R3!, {R4-R7}");
    asm(" B         End_Sleep");

    asm("Small_or_Big_Sleep:        ");
    asm(" LDR       R12, CLKM_CNTL_ARM_CLK_REG");
    asm(" LDRH      R0, [R12, #0]");
    asm(" MOV       R0, R0, LSL #16");
    asm(" MOV       R0, R0, LSR #17");
    asm(" MOV       R0, R0, LSL #1");
    asm(" STRH      R0, [R12, #0]");
    asm(" STMIA     R3!, {R4-R7}");

    asm("End_Sleep:        ");

  } /* f_arm_sleep_cmd() */

  asm("MPU_FREE_REG          .word  0xffffff20");
  asm("CLKM_CNTL_ARM_CLK_REG .word  0xfffffd00");
#endif

#if (CHIPSET == 15)
void init_small_sleep()
{

       /* Should not disable bridge_clk during small sleep when an I2C transaction is pending

                In Locosto without BRIDGE_CLK, I2C interrupt is not generated */

            // This could be removed if i2c_pwr_interface is functional

	Uint8 sts=0, camera_sts = 0;     

      sts=i2c_pwr_interface(0);
#ifdef RVM_CAMD_SWE
#if(LOCOSTO_LITE == 0)
#if (OP_L1_STANDALONE == 0)
	camera_sts = camera_pwr_interface(0);
#endif
#endif
#endif
	
	if (sts != 0 
#if(LOCOSTO_LITE == 0)
		|| camera_sts != 0
#endif
		)
 {
                                    sts=i2c_pwr_interface(2);//enable
                                    *((volatile UINT16 *) CLKM_CNTL_CLK) &= ~CLKM_BRIDGE_DIS;
 }
            else
                        {
                                     sts=i2c_pwr_interface(1);//disable
					*((volatile UINT16 *) CLKM_CNTL_CLK) |= CLKM_BRIDGE_DIS;
					F_INTH_ENABLE_ONE_IT(C_INTH_UART_WAKEUP_IT);
 }
}

 

void exit_small_sleep()

{
            i2c_pwr_interface(2);
            return;
}


#endif

/*-------------------------------------------------------*/ 
/* INT_Small_Sleep()                                     */
/*-------------------------------------------------------*/
/*                                                       */
/* Description: small sleep                              */
/* ------------                                          */
/* Called by TCT_Schedule main loop of Nucleus           */
/*-------------------------------------------------------*/


asm("           .def    INT_Small_Sleep ");
asm("INT_Small_Sleep  ");

/* NEW COMPILER MANAGEMENT
 * _switch_PWR_MNGT and _mode_authorized must be .def and not .ref
 * as they are both defined in this file.
 */
asm("           .def    _switch_PWR_MNGT  ");
asm("           .def    _mode_authorized  ");
asm("SMALL_SLEEP         .equ     01h ");       
asm("ALL_SLEEP           .equ     04h ");   
asm("PWR_MNGT            .equ     01h "); 
#if (OP_L1_STANDALONE == 0)
   // This code log the number of time the Small sleep 
   // function has been invoked
#if (TI_PROFILER == 1)
asm("   ldr	r0, profiler_counter "); // pick counter 
asm("	mov	r1,#0  "); 
asm("	str	r1,[r0]  ");
#endif

#if (TI_NUC_MONITOR == 1) 
// Push registers on statck because R3 and R4 must not be modified
asm("   STMFD	sp!,{r0-r5}");
asm("           .global    _ti_nuc_monitor_sleep  ");
asm("   BL  _ti_nuc_monitor_sleep");
asm("   LDMFD	sp!,{r0-r5}");  
#endif  
   // End log call
#endif // OP_L1_STANDALONE
//asm("           .ref    TCT_Schedule_Loop  ");
//asm("	B 	TCT_Schedule_Loop  ");
  
asm("	ldr	r0,Switch  ");	        // pick up sleep mode 
asm("	ldr	r0,[r0]  ");            // take the current value of the register
asm("	ldrb	r1,[r0]  ");            // take the current value of the register
asm("	cmp	r1,#PWR_MNGT  ");       // take the current value of the register
asm("	bne	End_small_sleep  ");
   
asm("	ldr	r0,Mode  ");	        // pick up sleep mode 
asm("	ldr	r0,[r0]  ");            // take the current value of the register
asm("	ldrb	r1,[r0]  ");            // take the current value of the register
asm("	cmp	r1,#SMALL_SLEEP  ");    // take the current value of the register
asm("	beq	Small_sleep_ok  ");
asm("	cmp	r1,#ALL_SLEEP  ");      // take the current value of the register
asm("	bne	End_small_sleep  ");

asm("Small_sleep_ok    ");

// *****************************************************
// CQ19599: For Calypso+ chipset, extended page mode
// shall be disabled before entering deep sleep and 
// restored at wake up
// *****************************************************
#if (W_A_CALYPSO_PLUS_SPR_19599 == 1)
   asm("   .ref _f_memif_extended_page_mode_read_bit ");
   asm("   .ref _f_memif_extended_page_mode_disable ");

   asm(" BL _f_memif_extended_page_mode_read_bit"); //read state of extended page mode
   asm(" STMFD	sp!,{r2}");                         //save r2 in stack in case it was used before
   asm(" MOV r2,r0");                             //store the state in r2
   asm(" BL _f_memif_extended_page_mode_disable");  //disable extended page mode
#endif

#if (CHIPSET == 15)
//// Disable IRQs
asm("   MRS     r1, CPSR              ");
// asm("   MOV     r2,r1"); // Copy the contents on CPSR register to r2
asm(" STMFD	sp!,{r1}"); // Push r2 in the stack
asm("   ORR     r1,r1,#00c0h   ");  // Disable IRQs
asm("   MSR     CPSR,r1              ");

asm("	.ref _init_small_sleep");
asm(" BL _init_small_sleep");
#endif

// *****************************************************
//reset the DEEP_SLEEP bit 12 of CNTL_ARM_CLK register
// (Cf BUG_1278)
asm(" ldr r0,addrCLKM  ");     // pick up CNTL_ARM_CLK register address
asm("   ldrh r1,[r0]    ");      // take the current value of the register
asm("   orr  r1,r1,#1000h    "); // reset the bit
asm("   strh r1,[r0]  ");        //store the result

asm("	ldr	r0,addrCLKM  ");	// pick up CLKM clock register address
asm("	ldrh	r1,[r0]  ");            // take the current value of the register
asm("	bic	r1,r1,#1  ");		// disable ARM clock
asm("	strh	r1,[r0]  ");
// *****************************************************

#if (CHIPSET == 15)
 asm("	.ref _exit_small_sleep");
 asm(" BL _exit_small_sleep");
//Enable IRQs
asm(" LDMFD	sp!,{r1}");   //restore r2 from stack
//asm("   MOV     r1,r2"); // Move r2 to r1
asm("   MSR     CPSR,r1              "); // Copy the contents of r1 to CPSR register
#endif

#if (W_A_CALYPSO_BUG_01435 == 1)
  asm(" MOV R0, #SMALL_SLEEP");
  asm(" BL  _f_arm_sleep_cmd");
#endif

// *****************************************************
// CQ19599: For Calypso+ chipset, restore the extended
// page mode if it was enabled before entering sleep 
// *****************************************************
#if (W_A_CALYPSO_PLUS_SPR_19599 == 1)
   asm("   .ref _f_memif_extended_page_mode_enable ");
   asm(" CMP r2,#0");                            //check if extended page mode was enabled
   asm(" BEQ extended_page_mode_restored   ");    //if not, do nothing
   asm(" BL _f_memif_extended_page_mode_enable"); //else restore it
   asm("extended_page_mode_restored    ");
   asm(" LDMFD	sp!,{r2}");   //restore r2
#endif

asm("           .ref    TCT_Schedule_Loop  ");
asm("End_small_sleep    ");
asm("	B 	TCT_Schedule_Loop  ");  // Return to TCT_Schedule main loop

asm("addrCLKM    	.word	0xfffffd00  ");//CLKM clock register address

asm("Mode               .word   _mode_authorized ");           
asm("Switch             .word   _switch_PWR_MNGT ");            
#if (OP_L1_STANDALONE == 0)
#if (TI_PROFILER == 1)
asm("   		.ref    _ti_profiler_nb_sleep_call ");
asm("profiler_counter	.word   _ti_profiler_nb_sleep_call ");
#endif
#endif // OP_L1_STANDALONE
 
