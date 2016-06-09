/*
 * This module contains that part of TI's int.s (INT_Initialize) code
 * which does some entry-point initialization of a few Calypso registers.
 * The important part for us is getting rid of whatever PLL setup
 * may have been done by the BootROM-based process that got us loaded
 * and running - we need to do that before we can do our own setup.
 */

	.code 32
	.text

#define	CNTL_ARM_CLK_REG	0xFFFFFD00  // CNTL_ARM_CLK register address
#define	DPLL_CNTRL_REG		0xFFFF9800  // DPLL control register address
#define	EXTRA_CONTROL_REG	0xFFFFFB10  // Extra Control register CONF addr
#define	MPU_CTL_REG		0xFFFFFF08  // MPU_CTL register address

#define	CNTL_ARM_CLK_RST	0x1081      // Init of CNTL_ARM_CLK register

                                            // Use DPLL, Divide by 1
#define	DPLL_CONTROL_RST	0x2002      // Configure DPLL in default state
#define	DISABLE_DU_MASK		0x0800      // Mask to Disable the DU module
#define	ENABLE_DU_MASK		0xF7FF      // Mask to Enable the DU module
#define	MPU_CTL_RST		0x0000      // Reset value of MPU_CTL register
					    // - All protections disabled

	.globl	freecalypso_disable_bootrom_pll
freecalypso_disable_bootrom_pll:
@
@  Configure DPLL register with reset value
@
       ldr     r1,=DPLL_CNTRL_REG    @ Load address of DPLL register in R1
       ldrh    r2,=DPLL_CONTROL_RST  @ Load DPLL reset value in R2
       strh    r2,[r1]               @ Store DPLL reset value in DPLL register

@
@  Wait that DPLL goes in BYPASS mode
@
Wait_DPLL_Bypass:
       ldr     r2,[r1]               @ Load DPLL register
       and     r2,r2,#1              @ Perform a mask on bit 0
       cmp     r2,#1                 @ Compare DPLL lock bit
       beq     Wait_DPLL_Bypass      @ Wait Bypass mode (i.e. bit[0]='0')

@
@  Configure CNTL_ARM_CLK register with reset value: DPLL is used to
@  generate ARM clock with division factor of 1.
@
       ldr     r1,=CNTL_ARM_CLK_REG  @ Load address of CNTL_ARM_CLK register in R1
       ldrh    r2,=CNTL_ARM_CLK_RST  @ Load CNTL_ARM_CLK reset value in R2
       strh    r2,[r1]               @ Store CNTL_ARM_CLK reset value in CNTL_ARM_CLK register

@
@  Disable/Enable the DU module by setting/resetting bit 11 to '1'/'0'
@
       ldr     r1,=EXTRA_CONTROL_REG @ Load address of Extra Control register CONF
@      ldrh    r2,=DISABLE_DU_MASK   @ Load mask to write in Extra Control register CONF
       ldrh    r2,=ENABLE_DU_MASK    @ Load mask to write in Extra Control register CONF
       ldrh    r0,[r1]               @ Load Extra Control register CONF in r0
@      orr     r0,r0,r2              @ Disable DU module
       and     r0,r0,r2              @ Enable DU module
       strh    r0,[r1]               @ Store configuration in Extra Control register CONF

@
@  Disable all MPU protections
@
       ldr     r1,=MPU_CTL_REG       @ Load address of MPU_CTL register
       ldrh    r2,=MPU_CTL_RST       @ Load reset value of MPU_CTL register
       strh    r2,[r1]               @ Store reset value of MPU_CTL register

	bx	lr
