/*
 * ARMIO.C 
 *
 *
 * Control diagnostic bits
 *
 * Reference : GCS207
 *
 */


#include "../include/config.h"
#include "../include/sys_types.h"

#include "mem.h"
#include "iq.h"
#include "armio.h"

#if 0
/* FreeCalypso: abb.h hasn't been integrated yet */
#include "abb.h"	 // for AI_Power function : to be removed, use ABB_Power_Off in abb.c file instead !!!
#endif

#if (CHIPSET != 12)
/*
 * AI_EnableBit
 *
 * Enable ARMIO input/output bit (see CLKM module specification)
 */
void AI_EnableBit(int bit)
{
  *((volatile SYS_UWORD16 *) CLKM_IO_CNTL) |= (1<<bit); 
}

/*
 * AI_DisableBit
 *
 * Disable ARMIO input/output bit (see CLKM module specification)
 */
void AI_DisableBit(int bit)
{
  *((volatile SYS_UWORD16 *) CLKM_IO_CNTL) &= ~(1<<bit); 
}

#endif /* CHIPSET != 12 */

/*
 * AI_SetBit
 *
 * Switch-on one bit
 */
void AI_SetBit(int bit)
{
   *((volatile SYS_UWORD16 *) ARMIO_OUT) |= (1<<bit); 
}

/*
 * AI_ResetBit
 *
 * Switch-off one bit
 */
void AI_ResetBit(int bit)
{
   *((volatile SYS_UWORD16 *) ARMIO_OUT) &= ~(1<<bit); 
}

/*
 * AI_ConfigBitAsOutput
 *
 * Set this bit as an output
 */
void AI_ConfigBitAsOutput(int bit)
{
   *((volatile SYS_UWORD16 *) ARMIO_IO_CNTL) &= ~(1<<bit); 
}

/*
 * AI_ConfigBitAsInput
 *
 * Set this bit as an input
 */
void AI_ConfigBitAsInput(int bit)
{
   *((volatile SYS_UWORD16 *) ARMIO_IO_CNTL) |= (1<<bit); 
}


/*
 * AI_ReadBit
 *
 * Read value in register
 */
SYS_BOOL AI_ReadBit(int bit)
{
   if ((*((volatile SYS_UWORD16 *) ARMIO_IN)) & (1<<bit))
      return (1);
   else
      return (0);
}

/*
 * AI_Power
 *
 * Switch-on or off the board
 *
 * Parameters : SYS_UWORD8 power: 1 to power-on (maintain power)
 *                                0 to power-off
 *
 */
// #if (!OP_L1_STANDALONE)
#if 0
void AI_Power(SYS_UWORD8 power)
{
  if (power == 0) 
  {
	ABB_Power_Off();
  }
}
#endif

/*
 * AI_ResetIoConfig
 *
 * Reset all default IO configurations
 *
 */
void AI_ResetIoConfig(void)
{
   *((volatile SYS_UWORD16 *) ARMIO_IO_CNTL) = 0xFFFF; // all bits are inputs
#if (CHIPSET != 12)
   *((volatile SYS_UWORD16 *) CLKM_IO_CNTL) = 0;       // default config
#endif /* CHIPSET != 12 */
}


/*
 * AI_ClockEnable
 *
 * Enable ARMIO clock module
 *
 */
void AI_ClockEnable(void)
{
   *((volatile SYS_UWORD16 *) ARMIO_CNTL_REG) |= ARMIO_CLOCKEN;    // set to 1 bit 5
}

/*
 * The AI_InitIOConfig() function is target-specific.
 */

#ifdef CONFIG_TARGET_GTAMODEM
/* version for the Openmoko GTA0x GSM modem */
void AI_InitIOConfig(void)
{
  // reset the IOs config
  AI_ResetIoConfig();

  /*
   * The GTA0x Calypso block is a stripped-down version of the Leonardo,
   * reduced to the absolute minimum that is needed for a slave modem.
   * Almost all of the unused interface pins are left unconnected, only
   * a few are pulled externally to GND or VIO.
   *
   * We handle the unused pins the way TI's code does: configure them
   * as GPIOs, then as outputs driving a fixed value (high for GPIOs 8+,
   * low for 0-7).
   */

  /* I'll be brave and turn the unused TSPDI input into a GPIO4 output */
  AI_EnableBit(0);
  /* Don't want to do that for the IO5 pin though, as it's wired to SIM_IO
   * through a resistor like on the Leonardo. */
  AI_DisableBit(1);
  /*
   * The following 2 lines are straight from the Leonardo source: enable
   * GPIO6 and GPIO8.  GPIO6 takes the place of an ancient VEGA3(?) compat
   * pin, and GPIO8 takes the place of MCUEN1 which no Leonardo-based
   * design seems to use.
   *
   * Note that GPIO7 is not enabled in this version, so that pin retains
   * its meaning as nRESET_OUT - but it's an unused output, rather than
   * a floating input, so we are fine.
   */
  AI_EnableBit(2);
  AI_EnableBit(4);

  /*
   * The GTA0x modem has no Calypso-interfaced Bluetooth, nor any other
   * use for MCSI, aka the DSP backdoor interface.  So we turn these 4 pins
   * into GPIOs driving high output state like TI's code does in the
   * sans-BT configuration.
   */
  AI_EnableBit(5);
  AI_EnableBit(6);
  AI_EnableBit(7);
  AI_EnableBit(8);

  /* ditto for MCUEN2 turned GPIO 13 */
  AI_EnableBit(9);

  // ARMIO_OUT register configuration :
  // set IOs 8,9,10,11,12 and 13 as high
  // set IOs 0 to 7 as low
  // Falcon's note: the BP->AP interrupt line gets set low as desired
  *((volatile SYS_UWORD16 *) ARMIO_OUT) = 0x3F00;
    
  // ARMIO_CNTL_REG register configuration :
  // set IOs 0,1,6,8,9,10,11,12 and 13 as outputs.
  // Falcon's addition: set 2, 3 and 4 as outputs too,
  // as they are no-connects on the board.

  AI_ConfigBitAsOutput(0);
  AI_ConfigBitAsOutput(1);
  AI_ConfigBitAsOutput(2);
  AI_ConfigBitAsOutput(3);
  AI_ConfigBitAsOutput(4);
  AI_ConfigBitAsOutput(6);
  AI_ConfigBitAsOutput(8);
  AI_ConfigBitAsOutput(9);
  AI_ConfigBitAsOutput(10);
  AI_ConfigBitAsOutput(11);
  AI_ConfigBitAsOutput(12);
  AI_ConfigBitAsOutput(13);
}
#endif

#ifdef CONFIG_TARGET_FCDEV3B
/* version for the to-be-built FCDEV3B board */
void AI_InitIOConfig(void)
{
  // reset the IOs config
  AI_ResetIoConfig();

  AI_EnableBit(0);
  AI_DisableBit(1);
  AI_EnableBit(2);
  AI_EnableBit(4);

  /*
   * The MCSI pins are brought out to a header with the intent of
   * facilitating experimentation with this interface, so let's put them
   * into the MCSI configuration.
   */
  AI_DisableBit(5);
  AI_DisableBit(6);
  AI_DisableBit(7);
  AI_DisableBit(8);

  AI_EnableBit(9);

  // ARMIO_OUT register configuration: same as TI's code sets
  // the loudspeaker PA is disabled on boot for the time being
  *((volatile SYS_UWORD16 *) ARMIO_OUT) = 0x3F00;

  // ARMIO_CNTL_REG register configuration:
  // most of the pins are no-connects, make them outputs

  AI_ConfigBitAsOutput(0);
  AI_ConfigBitAsOutput(1);
  AI_ConfigBitAsOutput(2);
  AI_ConfigBitAsOutput(4);
  AI_ConfigBitAsOutput(6);
  AI_ConfigBitAsOutput(8);
  AI_ConfigBitAsOutput(9);
  AI_ConfigBitAsOutput(13);
}
#endif

#ifdef CONFIG_TARGET_PIRELLI
/* version for Pirelli DP-L10 */
void AI_InitIOConfig(void)
{
  // reset the IOs config
  AI_ResetIoConfig();

  /*
   * In the case of the Pirelli, our understanding of the hardware
   * is severely crippled by the lack of schematics and the difficulty of
   * reverse engineering from steve-m's PCB layer grind-down scans.
   *
   * The folllowing ARMIO configuration has been copied from OsmocomBB.
   */
  AI_EnableBit(0);
  AI_EnableBit(1);
  AI_EnableBit(2);
  AI_EnableBit(3);
  AI_EnableBit(4);
  AI_EnableBit(9);
  /* GPIO out all zeros */
  *((volatile SYS_UWORD16 *) ARMIO_OUT) = 0x0000;
  /* setup outputs like OsmocomBB does */
  AI_ConfigBitAsOutput(1);
  AI_ConfigBitAsOutput(4);
  AI_ConfigBitAsOutput(7);
}
#endif

#ifdef CONFIG_TARGET_COMPAL
/* same for all C1xx variants */
void AI_InitIOConfig(void)
{
  // reset the IOs config
  AI_ResetIoConfig();

  /*
   * I don't feel like scrutinizing every Calypso signal on the C139
   * and C155 schematics right now, so for now we'll use a GPIO
   * configuration based on OsmocomBB.
   */
  /* GPIO out all zeros */
  *((volatile SYS_UWORD16 *) ARMIO_OUT) = 0x0000;
  /* make GPIOs 1 and 3 outputs */
  AI_ConfigBitAsOutput(1);
  AI_ConfigBitAsOutput(3);
}
#endif

/*
 * AI_SelectIOForIT
 *
 * Select which IO will be used to generate an interrupt.
 * 'Edge' specifies if interrup must be detected on falling or rising edge.
 *
 * Warning: parameters are not checked.
 */
 
void AI_SelectIOForIT (SYS_UWORD16 Pin, SYS_UWORD16 Edge)
{
  #if (CHIPSET == 12)
    /*
     *  Update INTERRUPT_LEVEL_REG with Edge configuration on Pin selection
     */
    GPIO_INTERRUPT_LEVEL_REG = (Edge & 0x0001) << Pin;

    /*
     *  Update INTERRUPT_MASK_REG to enable interrupt generation on Pin selection
     */
    GPIO_INTERRUPT_MASK_REG = 1 << Pin;
  #else
    /*
     * Bit SET_GPIO_EVENT_MODE (bit 0) is set to enable the GPIO event mode.
     */
     
    *((volatile SYS_UWORD16 *) ARMIO_GPIO_EVENT_MODE) = (Pin << 1) + (Edge << 5) + 1;
  #endif
}

#if (CHIPSET != 12)
/*
 * AI_CheckITSource
 *
 * Check if the interrupt specified by 'Source' is active or not.
 *
 * Output: 0: IT is not active
 *         1: IT is active
 *
 * Warning: parameters are not checked.
 *
 * Warning: If the keypad and GPIO interrupts may occur the GPIO interrupt
 *          must be checked first because the GPIO status bit is reset when
 *          the register is read.
 */
 
int  AI_CheckITSource (SYS_UWORD16 Source)
{
    return (*((volatile SYS_UWORD16 *) ARMIO_KBD_GPIO_INT) & Source ? 1 : 0);
}

/*
 * AI_UnmaskIT
 *
 * Unmask the IT specified by 'Source' (keyboard or GPIO).
 *
 * Warning: parameters are not checked.
 */
 
void AI_UnmaskIT (SYS_UWORD16 Source)
{
    *((volatile SYS_UWORD16 *) ARMIO_KBD_GPIO_MASKIT) &= ~Source;
}

/*
 * AI_MaskIT
 *
 * Mask the IT specified by 'Source' (keyboard or GPIO).
 *
 * Warning: parameters are not checked.
 */
 
void AI_MaskIT (SYS_UWORD16 Source)
{
    *((volatile SYS_UWORD16 *) ARMIO_KBD_GPIO_MASKIT) |= Source;
}
#endif /* CHIPSET != 12 */

#if (CHIPSET == 12)

  void AI_MaskIT(SYS_UWORD16 d_io_number) {
    GPIO_INTERRUPT_MASK_REG |= (1 << d_io_number);
  } /* f_gpio_mask_it() */
  
  void AI_UnmaskIT(SYS_UWORD16 d_io_number) {
    GPIO_INTERRUPT_MASK_REG &= ~(1 << d_io_number);
  } /* f_gpio_unmask_it() */

#endif
