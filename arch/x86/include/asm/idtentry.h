/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_IDTENTRY_H
#define _ASM_X86_IDTENTRY_H

/* Interrupts/Exceptions */
#include <asm/trapnr.h>

#ifndef __ASSEMBLY__

#ifdef CONFIG_CONTEXT_TRACKING
static __always_inline void enter_from_user_context(void)
{
	CT_WARN_ON(ct_state() != CONTEXT_USER);
	user_exit_irqoff();
}
#else
static __always_inline void enter_from_user_context(void) { }
#endif

/**
 * idtentry_enter - Handle state tracking on idtentry
 * @regs:	Pointer to pt_regs of interrupted context
 *
 * Invokes:
 *  - The hardirq tracer to keep the state consistent as low level ASM
 *    entry disabled interrupts.
 *
 *  - Context tracking if the exception hit user mode
 */
static __always_inline void idtentry_enter(struct pt_regs *regs)
{
	trace_hardirqs_off();
	if (user_mode(regs))
		enter_from_user_context();
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

/* Special case for 32bit IRET 'trap' */
#define DECLARE_IDTENTRY_SW	DECLARE_IDTENTRY
#define DEFINE_IDTENTRY_SW	DEFINE_IDTENTRY

/**
 * DECLARE_IDTENTRY_ERRORCODE - Declare functions for simple IDT entry points
 *				Error code pushed by hardware
 * @vector:	Vector number (ignored for C)
 * @func:	Function name of the entry point
 *
 * Declares three functions:
 * - The ASM entry point: asm_##func
 * - The XEN PV trap entry point: xen_##func (maybe unused)
 * - The C handler called from the ASM entry point
 */
#define DECLARE_IDTENTRY_ERRORCODE(vector, func)			\
	asmlinkage void asm_##func(void);				\
	asmlinkage void xen_asm_##func(void);				\
	__visible void func(struct pt_regs *regs, unsigned long error_code)

/**
 * DEFINE_IDTENTRY_ERRORCODE - Emit code for simple IDT entry points
 *			       Error code pushed by hardware
 * @func:	Function name of the entry point
 *
 * Same as DEFINE_IDTENTRY, but has an extra error_code argument
 */
#define DEFINE_IDTENTRY_ERRORCODE(func)					\
static __always_inline void __##func(struct pt_regs *regs,		\
				     unsigned long error_code);		\
									\
__visible notrace void func(struct pt_regs *regs,			\
			    unsigned long error_code)			\
{									\
	idtentry_enter(regs);						\
	__##func (regs, error_code);					\
	idtentry_exit(regs);						\
}									\
NOKPROBE_SYMBOL(func);							\
									\
static __always_inline void __##func(struct pt_regs *regs,		\
				     unsigned long error_code)

/**
 * DECLARE_IDTENTRY_CR2 - Declare functions for fault handling IDT entry points
 * @vector:	Vector number (ignored for C)
 * @func:	Function name of the entry point
 *
 * Declares three functions:
 * - The ASM entry point: asm_##func
 * - The XEN PV trap entry point: xen_##func (maybe unused)
 * - The C handler called from the ASM entry point
 */
#define DECLARE_IDTENTRY_CR2(vector, func)				\
	asmlinkage void asm_##func(void);				\
	asmlinkage void xen_asm_##func(void);				\
	__visible void func(struct pt_regs *regs, unsigned long error_code)

/**
 * DEFINE_IDTENTRY_CR2 - Emit code for fault handling IDT entry points
 * @func:	Function name of the entry point
 *
 * Same as IDTENTRY_ERRORCODE but reads CR2 before invoking
 * idtentry_enter() and hands the CR2 address into the function body.
 */
#define DEFINE_IDTENTRY_CR2(func)					\
static __always_inline void __##func(struct pt_regs *regs,		\
				     unsigned long error_code,		\
				     unsigned long address);		\
									\
__visible notrace void func(struct pt_regs *regs,			\
			    unsigned long error_code)			\
{									\
	unsigned long address = read_cr2();				\
									\
	idtentry_enter(regs);						\
	__##func (regs, error_code, address);				\
	idtentry_exit(regs);						\
}									\
NOKPROBE_SYMBOL(func);							\
									\
static __always_inline void __##func(struct pt_regs *regs,		\
				     unsigned long error_code,		\
				     unsigned long address)

#ifdef CONFIG_X86_64
/**
 * DECLARE_IDTENTRY_IST - Declare functions for IST handling IDT entry points
 * @vector:	Vector number (ignored for C)
 * @func:	Function name of the entry point
 *
 * Declares four functions:
 * - The ASM entry point: asm_##func
 * - The XEN PV trap entry point: xen_##func (maybe unused)
 * - The NOIST C handler called from the ASM entry point on user mode entry
 * - The C handler called from the ASM entry point
 */
#define DECLARE_IDTENTRY_IST(vector, func)				\
	asmlinkage void asm_##func(void);				\
	asmlinkage void xen_asm_##func(void);				\
	__visible void noist_##func(struct pt_regs *regs);		\
	__visible void func(struct pt_regs *regs)

/**
 * DEFINE_IDTENTRY_IST - Emit code for IST entry points
 * @func:	Function name of the entry point
 *
 * This provides two entry points:
 * - The real IST based entry
 * - The regular stack based entry invoked when coming from user mode
 *   or XEN_PV
 */
#define DEFINE_IDTENTRY_IST(func)					\
static __always_inline void __##func(struct pt_regs *regs);		\
									\
__visible notrace void func(struct pt_regs *regs)			\
{									\
	__##func (regs);						\
}									\
NOKPROBE_SYMBOL(func);							\
									\
static __always_inline void __##func(struct pt_regs *regs)

/**
 * DEFINE_IDTENTRY_NOIST - Emit code for NOIST entry points which
 *			   belong to a IST entry point (MCE, DB)
 * @func:	Function name of the entry point. Must be the same as
 *		the function name of the corresponding IST variant
 *
 * Maps to DEFINE_IDTENTRY().
 */
#define DEFINE_IDTENTRY_NOIST(func)					\
	DEFINE_IDTENTRY(noist_##func)

/**
 * DECLARE_IDTENTRY_DF - Declare functions for double fault
 * @vector:	Vector number (ignored for C)
 * @func:	Function name of the entry point
 *
 * Maps to DECLARE_IDTENTRY_ERRORCODE
 */
#define DECLARE_IDTENTRY_DF(vector, func)				\
	DECLARE_IDTENTRY_ERRORCODE(vector, func)

/**
 * DEFINE_IDTENTRY_DF - Emit code for double fault
 * @func:	Function name of the entry point
 *
 * @func is called from ASM entry code with interrupts disabled.
 */
#define DEFINE_IDTENTRY_DF(func)					\
static __always_inline void __##func(struct pt_regs *regs,		\
				     unsigned long error_code,		\
				     unsigned long address);		\
									\
__visible notrace void func(struct pt_regs *regs,			\
			    unsigned long error_code)			\
{									\
	unsigned long address = read_cr2();				\
									\
	trace_hardirqs_off();						\
	__##func (regs, error_code, address);				\
}									\
NOKPROBE_SYMBOL(func);							\
									\
static __always_inline void __##func(struct pt_regs *regs,		\
				     unsigned long error_code,		\
				     unsigned long address)

#else	/* CONFIG_X86_64 */
/* Maps to a regular IDTENTRY on 32bit for now */
# define DECLARE_IDTENTRY_IST		DECLARE_IDTENTRY
# define DEFINE_IDTENTRY_IST		DEFINE_IDTENTRY

/**
 * DECLARE_IDTENTRY_DF - Declare functions for double fault 32bit variant
 * @vector:	Vector number (ignored for C)
 * @func:	Function name of the entry point
 *
 * Declares three functions:
 * - The ASM entry point: asm_##func
 * - The C handler called from the C shim
 */
#define DECLARE_IDTENTRY_DF(vector, func)				\
	asmlinkage void asm_##func(void);				\
	__visible void func(struct pt_regs *regs,			\
			    unsigned long error_code,			\
			    unsigned long address)

/**
 * DEFINE_IDTENTRY_DF - Emit code for double fault on 32bit
 * @func:	Function name of the entry point
 *
 * This is called through the doublefault shim which already provides
 * cr2 in the address argument.
 */
#define DEFINE_IDTENTRY_DF(func)					\
static __always_inline void __##func(struct pt_regs *regs,		\
				     unsigned long error_code,		\
				     unsigned long address);		\
									\
__visible notrace void func(struct pt_regs *regs,			\
			    unsigned long error_code,			\
			    unsigned long address)			\
{									\
	__##func (regs, error_code, address);				\
}									\
NOKPROBE_SYMBOL(func);							\
									\
static __always_inline void __##func(struct pt_regs *regs,		\
				     unsigned long error_code,		\
				     unsigned long address)

#endif	/* !CONFIG_X86_64 */

/* C-Code mapping */
#define DECLARE_IDTENTRY_MCE		DECLARE_IDTENTRY_IST
#define DEFINE_IDTENTRY_MCE		DEFINE_IDTENTRY_IST
#define DEFINE_IDTENTRY_MCE_USER	DEFINE_IDTENTRY_NOIST

#define DECLARE_IDTENTRY_NMI		DECLARE_IDTENTRY_IST
#define DEFINE_IDTENTRY_NMI		DEFINE_IDTENTRY_IST

#define DECLARE_IDTENTRY_DEBUG		DECLARE_IDTENTRY_IST
#define DEFINE_IDTENTRY_DEBUG		DEFINE_IDTENTRY_IST
#define DEFINE_IDTENTRY_DEBUG_USER	DEFINE_IDTENTRY_NOIST

/**
 * DECLARE_IDTENTRY_XEN - Declare functions for XEN redirect IDT entry points
 * @vector:	Vector number (ignored for C)
 * @func:	Function name of the entry point
 *
 * Used for xennmi and xendebug redirections. No DEFINE as this is all
 * indirection magic.
 */
#define DECLARE_IDTENTRY_XEN(vector, func)			\
	asmlinkage void xen_asm_exc_xen##func(void);		\
	asmlinkage void asm_exc_xen##func(void)

#else /* !__ASSEMBLY__ */

/* Defines for ASM code to construct the IDT entries */
#define DECLARE_IDTENTRY(vector, func)				\
	idtentry vector asm_##func func has_error_code=0

#define DECLARE_IDTENTRY_ERRORCODE(vector, func)		\
	idtentry vector asm_##func func has_error_code=1

#define DECLARE_IDTENTRY_CR2(vector, func)			\
	DECLARE_IDTENTRY_ERRORCODE(vector, func)

/* Special case for 32bit IRET 'trap'. Do not emit ASM code */
#define DECLARE_IDTENTRY_SW(vector, func)

#ifdef CONFIG_X86_64
# define DECLARE_IDTENTRY_MCE(vector, func)			\
	idtentry_mce_db vector asm_##func func

# define DECLARE_IDTENTRY_DEBUG(vector, func)			\
	idtentry_mce_db vector asm_##func func

# define DECLARE_IDTENTRY_DF(vector, func)			\
	idtentry_df vector asm_##func func

#else
# define DECLARE_IDTENTRY_MCE(vector, func)			\
	DECLARE_IDTENTRY(vector, func)

# define DECLARE_IDTENTRY_DEBUG(vector, func)			\
	DECLARE_IDTENTRY(vector, func)

/* No ASM emitted for DF as this goes through a C shim */
# define DECLARE_IDTENTRY_DF(vector, func)

#endif

/* No ASM code emitted for NMI */
#define DECLARE_IDTENTRY_NMI(vector, func)

/* XEN NMI and DB wrapper */
#define DECLARE_IDTENTRY_XEN(vector, func)			\
	idtentry vector asm_exc_xen##func exc_##func has_error_code=0

#endif /* __ASSEMBLY__ */

/*
 * Dummy trap number so the low level ASM macro vector number checks do not
 * match which results in emitting plain IDTENTRY stubs without bells and
 * whistels.
 */
#define X86_TRAP_OTHER		~0uL

/* Simple exception entries: */
DECLARE_IDTENTRY(X86_TRAP_DE,		exc_divide_error);
DECLARE_IDTENTRY(X86_TRAP_BP,		exc_int3);
DECLARE_IDTENTRY(X86_TRAP_OF,		exc_overflow);
DECLARE_IDTENTRY(X86_TRAP_BR,		exc_bounds);
DECLARE_IDTENTRY(X86_TRAP_UD,		exc_invalid_op);
DECLARE_IDTENTRY(X86_TRAP_NM,		exc_device_not_available);
DECLARE_IDTENTRY(X86_TRAP_OLD_MF,	exc_coproc_segment_overrun);
DECLARE_IDTENTRY(X86_TRAP_SPURIOUS,	exc_spurious_interrupt_bug);
DECLARE_IDTENTRY(X86_TRAP_MF,		exc_coprocessor_error);
DECLARE_IDTENTRY(X86_TRAP_XF,		exc_simd_coprocessor_error);

/* 32bit software IRET trap. Do not emit ASM code */
DECLARE_IDTENTRY_SW(X86_TRAP_IRET,	exc_iret_error);

/* Simple exception entries with error code pushed by hardware */
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_TS,	exc_invalid_tss);
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_NP,	exc_segment_not_present);
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_SS,	exc_stack_segment);
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_GP,	exc_general_protection);
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_AC,	exc_alignment_check);

/* Page fault entry points */
DECLARE_IDTENTRY_CR2(X86_TRAP_PF,	exc_page_fault);
#ifdef CONFIG_KVM_GUEST
DECLARE_IDTENTRY_CR2(X86_TRAP_PF,	exc_async_page_fault);
#endif

#ifdef CONFIG_X86_MCE
/* Machine check */
DECLARE_IDTENTRY_MCE(X86_TRAP_MC,	exc_machine_check);
#endif

/* NMI */
DECLARE_IDTENTRY_NMI(X86_TRAP_NMI,	exc_nmi);
DECLARE_IDTENTRY_XEN(X86_TRAP_NMI,	nmi);

/* #DB */
DECLARE_IDTENTRY_DEBUG(X86_TRAP_DB,	exc_debug);
DECLARE_IDTENTRY_XEN(X86_TRAP_DB,	debug);

/* #DF */
#if defined(CONFIG_X86_64) || defined(CONFIG_DOUBLEFAULT)
DECLARE_IDTENTRY_DF(X86_TRAP_DF,	exc_double_fault);
#endif

#ifdef CONFIG_XEN_PV
DECLARE_IDTENTRY(X6_TRAP_OTHER,		exc_xen_hypervisor_callback);
#endif

#undef X86_TRAP_OTHER

#endif
