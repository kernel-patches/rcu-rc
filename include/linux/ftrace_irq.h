/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_FTRACE_IRQ_H
#define _LINUX_FTRACE_IRQ_H

#ifdef CONFIG_HWLAT_TRACER
extern bool trace_hwlat_callback_enabled;
extern void trace_hwlat_count_nmi(void);
extern void trace_hwlat_timestamp(bool enter);

static __always_inline void ftrace_count_nmi(void)
{
	if (unlikely(trace_hwlat_callback_enabled))
		trace_hwlat_count_nmi();
}

static __always_inline void ftrace_nmi_handler_enter(void)
{
	if (unlikely(trace_hwlat_callback_enabled))
		trace_hwlat_timestamp(true);
}

static __always_inline void ftrace_nmi_handler_exit(void)
{
	if (unlikely(trace_hwlat_callback_enabled))
		trace_hwlat_timestamp(false);
}
#else /* CONFIG_HWLAT_TRACER */
static inline void ftrace_count_nmi(void) {}
static inline void ftrace_nmi_handler_enter(void) {}
static inline void ftrace_nmi_handler_exit(void) {}
#endif

#endif /* _LINUX_FTRACE_IRQ_H */
