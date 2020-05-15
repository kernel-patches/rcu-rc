/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This file is designed to contain the BUILD_INTERRUPT specifications for
 * all of the extra named interrupt vectors used by the architecture.
 * Usually this is the Inter Process Interrupts (IPIs)
 */

/*
 * The following vectors are part of the Linux architecture, there
 * is no hardware IRQ pin equivalent for them, they are triggered
 * through the ICC by us (IPIs)
 */
#ifdef CONFIG_SMP
BUILD_INTERRUPT(reschedule_interrupt,RESCHEDULE_VECTOR)
#endif

#ifdef CONFIG_HAVE_KVM
BUILD_INTERRUPT(kvm_posted_intr_ipi, POSTED_INTR_VECTOR)
BUILD_INTERRUPT(kvm_posted_intr_wakeup_ipi, POSTED_INTR_WAKEUP_VECTOR)
BUILD_INTERRUPT(kvm_posted_intr_nested_ipi, POSTED_INTR_NESTED_VECTOR)
#endif

/*
 * every pentium local APIC has two 'local interrupts', with a
 * soft-definable vector attached to both interrupts, one of
 * which is a timer interrupt, the other one is error counter
 * overflow. Linux uses the local APIC timer interrupt to get
 * a much simpler SMP time architecture:
 */
#ifdef CONFIG_X86_LOCAL_APIC

#ifdef CONFIG_IRQ_WORK
BUILD_INTERRUPT(irq_work_interrupt, IRQ_WORK_VECTOR)
#endif

#ifdef CONFIG_X86_THERMAL_VECTOR
BUILD_INTERRUPT(thermal_interrupt,THERMAL_APIC_VECTOR)
#endif

#ifdef CONFIG_X86_MCE_THRESHOLD
BUILD_INTERRUPT(threshold_interrupt,THRESHOLD_APIC_VECTOR)
#endif

#ifdef CONFIG_X86_MCE_AMD
BUILD_INTERRUPT(deferred_error_interrupt, DEFERRED_ERROR_VECTOR)
#endif
#endif
