/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/* Compatibility definitions for building against kernels older than 6.6 */

#ifndef __RTW89_COMPAT_H__
#define __RTW89_COMPAT_H__

#include <linux/version.h>
#include <linux/compiler.h>
#include <linux/container_of.h>

#ifndef __counted_by
#define __counted_by(member)
#endif

#ifndef container_of_const
#define container_of_const(ptr, type, member)				\
	_Generic(ptr,							\
		const typeof(*(ptr)) *: ((const type *)container_of(ptr, type, member)),\
		default: ((type *)container_of(ptr, type, member))	\
	)
#endif

#include <linux/bitfield.h>
#include <linux/overflow.h>

#ifndef __BF_CHECK_POW2
#define __BF_CHECK_POW2(n)	BUILD_BUG_ON_ZERO(((n) & ((n) - 1)) != 0)
#endif

#ifndef FIELD_PREP_CONST
#define FIELD_PREP_CONST(_mask, _val)					\
	(								\
		/* mask must be non-zero */				\
		BUILD_BUG_ON_ZERO((_mask) == 0) +			\
		/* check if value fits */				\
		BUILD_BUG_ON_ZERO(~((_mask) >> __bf_shf(_mask)) & (_val)) + \
		/* check if mask is contiguous */			\
		__BF_CHECK_POW2((_mask) + (1ULL << __bf_shf(_mask))) +	\
		/* and create the value */				\
		(((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask))	\
	)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include <linux/iopoll.h>
#include <linux/delay.h>

/*
 * Older kernels time read_poll_timeout_atomic() against ktime. The driver's
 * timeouts assume the newer behaviour, which only counts the udelay()s, so
 * over USB (where each register read is a control transfer) the ktime
 * version gives up far too early, e.g. "c2h reg timeout" during probe.
 * This is the 6.6 implementation.
 */
#undef read_poll_timeout_atomic
#define read_poll_timeout_atomic(op, val, cond, delay_us, timeout_us, \
					delay_before_read, args...) \
({ \
	u64 __timeout_us = (timeout_us); \
	s64 __left_ns = __timeout_us * NSEC_PER_USEC; \
	unsigned long __delay_us = (delay_us); \
	u64 __delay_ns = __delay_us * NSEC_PER_USEC; \
	if (delay_before_read && __delay_us) { \
		udelay(__delay_us); \
		if (__timeout_us) \
			__left_ns -= __delay_ns; \
	} \
	for (;;) { \
		(val) = op(args); \
		if (cond) \
			break; \
		if (__timeout_us && __left_ns < 0) { \
			(val) = op(args); \
			break; \
		} \
		if (__delay_us) { \
			udelay(__delay_us); \
			if (__timeout_us) \
				__left_ns -= __delay_ns; \
		} \
		cpu_relax(); \
		if (__timeout_us) \
			__left_ns--; \
	} \
	(cond) ? 0 : -ETIMEDOUT; \
})
#endif

#ifndef struct_size_t
#define struct_size_t(type, member, count)				\
	struct_size((type *)NULL, member, count)
#endif

#endif
