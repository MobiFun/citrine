/*
 * The int_disable() and int_enable() calls in TI's FFS code are nothing
 * more than TCT_Control_Interrupts() aka NU_Control_Interrupts().
 * TI's original code used assembly (proprietary toolchain asm syntax and
 * ABI semantics) to make the needed Thumb->ARM calls; we don't need such
 * idiocy in FreeCalypso, so we have turned int_disable() and int_enable()
 * into simple inline functions.
 */

#include "ffs.h"
#include "../../nucleus/nucleus.h"

static inline uint32 int_disable(void)
{
	return NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);
}

static inline void int_enable(uint32 tmp)
{
	NU_Control_Interrupts(tmp);
}
