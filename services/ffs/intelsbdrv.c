/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * FFS AMD single bank low level flash driver RAM code
 *
 * $Id: intelsbdrv.c 1.13 Thu, 08 Jan 2004 15:05:23 +0100 tsj $
 *
 ******************************************************************************/

#include "../../include/config.h"
#include "ffs.h"
#include "drv.h"
#include "ffstrace.h"
#include "intctl.h"

#define INTEL_UNLOCK_SLOW 1

#undef  tlw
#define tlw(contents)
#undef  ttw
#define ttw(contents)

// Status bits for Intel flash memory devices
#define INTEL_STATE_MACHINE_DONE     (1<<7)
#define FLASH_READ(addr)        (*(volatile uint16 *) (addr))
#define FLASH_WRITE(addr, data) (*(volatile uint16 *) (addr)) = data

/******************************************************************************
 * INTEL Single Bank Driver Functions
 ******************************************************************************/
// Actually we should have disabled and enable the interrupts in this
// function, but when the interrupt functions are used Target don't run!
// Anyway, currently the interrupts are already disabled at this point thus
// it does not cause any problems.
int ffsdrv_ram_intel_sb_init(void)
{
    uint32 i;
    volatile uint16 *addr;

    for (i = 0; i < dev.numblocks; i++)
    {
        addr = (volatile uint16 *) block2addr(i);

        *addr = 0x60; // Intel Config Setup
        *addr = 0xD0; // Intel Unlock Block

	*addr = 0xFF; // Intel Read Array
    }

    return 0;
}

void ffsdrv_ram_intel_sb_write_halfword(volatile uint16 *addr, uint16 value)
{
    uint32 cpsr;

    ttw(ttr(TTrDrv, "wh(%x,%x)" NL, addr, value));

    if (~*addr & value) {
        ttw(ttr(TTrFatal, "wh(%x,%x->%x) fatal" NL, addr, *addr, value));
        return;
    }

    cpsr = int_disable();
    tlw(led_on(LED_WRITE));

#if (INTEL_UNLOCK_SLOW == 1)
    *addr = 0x60; // Intel Config Setup
    *addr = 0xD0; // Intel Unlock Block
#endif

    *addr = 0x50; // Intel Clear Status Register
    *addr = 0x40; // Intel program byte/word
    *addr = value;
    while ((*addr & 0x80) == 0)
        ;
    *addr = 0xFF; // Intel read array
    tlw(led_off(LED_WRITE));
    int_enable(cpsr);
}

void ffsdrv_ram_intel_sb_erase(uint8 block)
{
    volatile uint16 *addr;
    uint32 cpsr;
    uint16 poll;

    ttw(ttr(TTrDrvEra, "e(%d)" NL, block));

    addr = (volatile uint16 *) block2addr(block);

    cpsr = int_disable();
    tlw(led_on(LED_ERASE));

#if (INTEL_UNLOCK_SLOW == 1)
    *addr = 0x60; // Intel Config Setup
    *addr = 0xD0; // Intel Unlock Block
#endif

    *addr = 0x50; // Intel Clear Status Register
    *addr = 0x20; // Intel Erase Setup
    *addr = 0xD0; // Intel Erase Confirm
    *addr = 0x70; // Intel Read Status Register

    // Wait for erase to finish.
    while ((*addr & 0x80) == 0) {
        tlw(led_toggle(LED_ERASE));
        // Poll interrupts, taking interrupt mask into account.
        if (INT_REQUESTED)
        {
            // 1. suspend erase
            // 2. enable interrupts
            // .. now the interrupt code executes
            // 3. disable interrupts
            // 4. resume erase

            tlw(led_on(LED_ERASE_SUSPEND));

            *addr = 0xB0; // Intel Erase Suspend
            *addr = 0x70; // Intel Read Status Register
            while (((poll = *addr) & 0x80) == 0)
                ;
            
            // If erase is complete, exit immediately
            if ((poll & 0x40) == 0)
                break;

            *addr = 0xFF; // Intel read array

            tlw(led_off(LED_ERASE_SUSPEND));
            int_enable(cpsr);

            // Other interrupts and tasks run now...

            cpsr = int_disable();
            tlw(led_on(LED_ERASE_SUSPEND));

            *addr = 0xD0; // Intel erase resume
// The following "extra" Read Status command is required because Intel has
// changed the specification of the W30 flash! (See "1.8 Volt Intel®
// Wireless Flash Memory with 3 Volt I/O 28F6408W30, 28F640W30, 28F320W30
// Specification Update")
	    *addr = 0x70; // Intel Read Status Register

            tlw(led_off(LED_ERASE_SUSPEND));
        }
    }
    *addr = 0xFF; // Intel read array

    tlw(led_on(LED_ERASE));
    tlw(led_off(LED_ERASE));
    int_enable(cpsr);
}
