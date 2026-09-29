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

#ifndef struct_size_t
#define struct_size_t(type, member, count)				\
	struct_size((type *)NULL, member, count)
#endif

#endif
