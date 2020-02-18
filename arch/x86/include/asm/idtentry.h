/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_IDTENTRY_H
#define _ASM_X86_IDTENTRY_H

/* Interrupts/Exceptions */
#include <asm/trapnr.h>

#ifndef __ASSEMBLY__

/**
 * idtentry_enter - Handle state tracking on idtentry
 * @regs:	Pointer to pt_regs of interrupted context
 *
 * Place holder for now.
 */
static __always_inline void idtentry_enter(struct pt_regs *regs)
{
}

/**
 * idtentry_exit - Prepare returning to low level ASM code
 *
 * Place holder for now.
 */
static __always_inline void idtentry_exit(struct pt_regs *regs)
{
}

/**
 * DECLARE_IDTENTRY - Declare functions for simple IDT entry points
 *		      No error code pushed by hardware
 * @vector:	Vector number (ignored for C)
 * @func:	Function name of the entry point
 *
 * Declares three functions:
 * - The ASM entry point: asm_##func
 * - The XEN PV trap entry point: xen_##func (maybe unused)
 * - The C handler called from the ASM entry point
 */
#define DECLARE_IDTENTRY(vector, func)					\
	asmlinkage void asm_##func(void);				\
	asmlinkage void xen_asm_##func(void);				\
	__visible void func(struct pt_regs *regs)

/**
 * DEFINE_IDTENTRY - Emit code for simple IDT entry points
 * @func:	Function name of the entry point
 *
 * @func is called from ASM entry code with interrupts disabled.
 *
 * The macro is written so it acts as function definition. Append the
 * body with a pair of curly brackets.
 *
 * idtentry_enter() contains common code which has to be invoked before
 * arbitrary code in the body. idtentry_exit() contains common code
 * which has to run before returning to the low level assembly code.
 */
#define DEFINE_IDTENTRY(func)						\
static __always_inline void __##func(struct pt_regs *regs);		\
									\
__visible notrace void func(struct pt_regs *regs)			\
{									\
	idtentry_enter(regs);						\
	__##func (regs);						\
	idtentry_exit(regs);						\
}									\
NOKPROBE_SYMBOL(func);							\
									\
static __always_inline void __##func(struct pt_regs *regs)

#else /* !__ASSEMBLY__ */

/* Defines for ASM code to construct the IDT entries */
#define DECLARE_IDTENTRY(vector, func)				\
	idtentry vector asm_##func func has_error_code=0

#endif /* __ASSEMBLY__ */

/* Simple exception entries: */
DECLARE_IDTENTRY(X86_TRAP_DE,		exc_divide_error);
DECLARE_IDTENTRY(X86_TRAP_BP,		exc_int3);
DECLARE_IDTENTRY(X86_TRAP_OF,		exc_overflow);
DECLARE_IDTENTRY(X86_TRAP_BR,		exc_bounds);
DECLARE_IDTENTRY(X86_TRAP_UD,		exc_invalid_op);
DECLARE_IDTENTRY(X86_TRAP_NM,		exc_device_not_available);

#endif
