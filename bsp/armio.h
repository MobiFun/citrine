/*
 * ARMIO.H  * * Control diagnostic bits * * Reference : GCS207 *
 *
 * FreeCalypso note: this version of armio.h originates
 * from the MV100-0.1.rar find.
 */

#ifndef _ARM_IO_H_
#define _ARM_IO_H_

#include "../include/sys_types.h"


// Duplicate address with ARMIO_IO_CNTL
// Need to investigate !!!
//#define ARMIO_CNTL      (MEM_ARMIO + 0x04) /* I/O control */

#define ARMIO_IN        (MEM_ARMIO + 0x00) /* inputs */
#define ARMIO_OUT       (MEM_ARMIO + 0x02) /* outputs */
#define ARMIO_IO_CNTL   (MEM_ARMIO + 0x04) /* I/O control */
#define ARMIO_CNTL_REG  (MEM_ARMIO + 0x06) /* control ARMIO */
#define ARMIO_LOAD_TIM  (MEM_ARMIO + 0x08) /* load TIM */
#if (CHIPSET != 12)
  #define ARMIO_KBR_IN    (MEM_ARMIO + 0x0A) /* KBR inputs (rows) */
  #define ARMIO_KBR_OUT   (MEM_ARMIO + 0x0C) /* KBR outputs (columns) */
#endif
#define ARMIO_PWM_CNTL  (MEM_ARMIO + 0x0E) /* LIGHT/BUZZER control */
#define ARMIO_LIGHT     (MEM_ARMIO + 0x10) /* light value */
#define ARMIO_BUZZER    (MEM_ARMIO + 0x12) /* buzzer value */
#define ARMIO_CLOCKEN   0x0020
#if (CHIPSET != 12)
  #define CLKM_IO_CNTL    MEM_IO_SEL         /* control IO */
#endif

#if (CHIPSET == 12)
  #define GPIO_INTERRUPT_LEVEL_REG  * (volatile SYS_UWORD16 *) (MEM_ARMIO + 0x16)
  #define GPIO_INTERRUPT_MASK_REG   * (volatile SYS_UWORD16 *) (MEM_ARMIO + 0x18)
#else
  #define ARMIO_GPIO_EVENT_MODE (MEM_ARMIO + 0x14) /* GPIO event mode */
  #define ARMIO_KBD_GPIO_INT    (MEM_ARMIO + 0x16) /* Kbd/GPIO IRQ register */
  #define ARMIO_KBD_GPIO_MASKIT (MEM_ARMIO + 0x18) /* Kbd/GPIO mask IRQ */
// CC test 0316  
  #define ARMIO_GPIO_DEBOUNCE   (MEM_ARMIO + 0x1A) /* GPIO debounceing register*/
// end  
#endif


#if (CHIPSET != 12)
  #define ARMIO_DCD (2) /* IO used for DCD on C-Sample - Output */
#endif
#define ARMIO_DTR (3) /* IO used for DTR on C-Sample - Input */

#define ARMIO_FALLING_EDGE (0)
#define ARMIO_RISING_EDGE  (1)

#if (CHIPSET != 12)
  #define ARMIO_KEYPAD_INT  (0x0001)
  #define ARMIO_KEYPDAD_INT ARMIO_KEYPAD_INT	/* TI's misspelling */
  #define ARMIO_GPIO_INT    (0x0002)

  #define ARMIO_MASKIT_KBD  (0x0001)
  #define ARMIO_MASKIT_GPIO (0x0002)

  void AI_EnableBit(int bit);
  void AI_DisableBit(int bit);
#endif
void AI_SetBit(int bit);
void AI_ResetBit(int bit);
void AI_ConfigBitAsOutput(int bit);
void AI_ConfigBitAsInput(int bit);
SYS_BOOL AI_ReadBit(int bit);
void AI_SetSimIO3V(SYS_UWORD8 StdOutput);
void AI_Power(SYS_UWORD8 power);
void AI_ResetTspIO(void);
void AI_ResetDbgReg(void);
void AI_ResetIoConfig(void);
void AI_InitIOConfig(void);
void AI_ClockEnable (void);

void AI_SelectIOForIT (SYS_UWORD16 Pin, SYS_UWORD16 Edge);
#if (CHIPSET != 12)
  int  AI_CheckITSource (SYS_UWORD16 Source);
#endif
void AI_UnmaskIT (SYS_UWORD16 Source);
void AI_MaskIT (SYS_UWORD16 Source);

#if 0

/*
 * The following stuff appears to be additions by whatever handset
 * manufacturer was behind the MV100 code - not TI's reference code.
 * It looks unclean, so let's try omitting it for FreeCalypso.
 */

/* glowing, 2004-06-08 import from M188A, defined by Robert.Chen*/
#define AI_CFGBIT_OUTPUT(bit)	(*((volatile SYS_UWORD16 *) ARMIO_IO_CNTL) &= ~(1<<bit))
#define AI_CFGBIT_INPUT(bit)	(*((volatile SYS_UWORD16 *) ARMIO_IO_CNTL) |= (1<<bit))
#define AI_SETBIT(bit)			(*((volatile SYS_UWORD16 *) ARMIO_OUT) |= (1<<bit))
#define AI_RESETBIT(bit)		(*((volatile SYS_UWORD16 *) ARMIO_OUT) &= ~(1<<bit))

#define GPIO_LEDG				0
#define GPIO_LEDB				1
#define GPIO_HALL 				2
#define GPIO_HEADSET_DETECT 	3

#define GPIO_MIDI_RESET		5

#define GPIO_LCD_RESET			7
#define GPIO_LCD_DIMM2			8
#define GPIO_MIDI_IRQ			9

#define GPIO_CAMERA_IRQ		11

#define GPIO_HEADSET_KEY		13

#endif	/* the #if 0 above */

#endif	/* file include guard */
