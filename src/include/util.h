// SPDX-License-Identifier: Apache-2.0
/* Copyright (C) 2013-2019 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __INCLUDE_UTIL_H_
#define __INCLUDE_UTIL_H_

#include <stdint.h>
#if defined(__riscv)
#include "encoding.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

/**
 * --------------------------------------------------------------------------
 * Macros

 * Set HOST_DEBUG to 1 if you are going to compile this for a host
 * machine (ie Athena/Linux) for debug purposes and set HOST_DEBUG
 * to 0 if you are compiling with the smips-gcc toolchain.
 */

#ifndef HOST_DEBUG
#define HOST_DEBUG 0
#endif

/**
 * Set PREALLOCATE to 1 if you want to preallocate the benchmark
 * function before starting stats. If you have instruction/data
 * caches and you don't want to count the overhead of misses, then
 * you will need to use preallocation.
*/

#ifndef PREALLOCATE
#define PREALLOCATE 0
#endif

#define static_assert(cond)                                                    \
	{                                                                      \
		switch (0) {                                                   \
		case 0:                                                        \
		case !!(long)(cond):;                                          \
		}                                                              \
	}

#define stringify_1(s) #s
#define stringify(s) stringify_1(s)
#define stats(code, iter)                                                              \
	do {                                                                           \
		unsigned long _c = -read_cycle(), _i = -read_csr(minstret);            \
		code;                                                                  \
		_c += read_cycle(), _i += read_csr(minstret);                          \
		if (cid == 0)                                                          \
			printk("\n%s: %ld cycles, %ld.%ld cycles/iter, %ld.%ld CPI\n", \
			       stringify(code), _c, _c / iter,                         \
			       10 * _c / iter % 10, _c / _i,                           \
			       10 * _c / _i % 10);                                     \
	} while (0)

/**
 * Set SET_STATS to 1 if you want to carve out the piece that actually
 * does the computation.
 */

#if HOST_DEBUG
#include "printf.h"
static void setStats(int enable)
{
}
#else
extern void setStats(int enable);
#endif

static void printArray(const char name[], int n, const int arr[])
{
#if HOST_DEBUG
	int i;

	printk(" %10s :", name);
	for (i = 0; i < n; i++)
		printk(" %3d ", arr[i]);
	printk("\n");
#endif
}

static void printDoubleArray(const char name[], int n, const double arr[])
{
#if HOST_DEBUG
	int i;

	printk(" %10s :", name);
	for (i = 0; i < n; i++)
		printk(" %g ", arr[i]);
	printk("\n");
#endif
}

static int verify(int n, const volatile int *test, const int *verify)
{
	int i;
	/* Unrolled for faster verification */
	for (i = 0; i < n / 2 * 2; i += 2) {
		int t0 = test[i], t1 = test[i + 1];
		int v0 = verify[i], v1 = verify[i + 1];

		if (t0 != v0)
			return i + 1;
		if (t1 != v1)
			return i + 2;
	}
	if (n % 2 != 0 && test[n - 1] != verify[n - 1])
		return n;
	return 0;
}

static int verifyDouble(int n, const volatile double *test,
			const double *verify)
{
	int i;
	/* Unrolled for faster verification */
	for (i = 0; i < n / 2 * 2; i += 2) {
		double t0 = test[i], t1 = test[i + 1];
		double v0 = verify[i], v1 = verify[i + 1];
		int eq1 = t0 == v0, eq2 = t1 == v1;

		if (!(eq1 & eq2))
			return i + 1 + eq1;
	}
	if (n % 2 != 0 && test[n - 1] != verify[n - 1])
		return n;
	return 0;
}

static void __attribute__((noinline)) barrier(int ncores)
{
	static volatile int sense = 0;
	static volatile int count = 0;

	static __thread int threadsense;

	__sync_synchronize();

	threadsense = !threadsense;
	if (__sync_fetch_and_add(&count, 1) == ncores - 1) {
		count = 0;
		sense = threadsense;
	} else {
		while (sense != threadsense)
			;
	}

	__sync_synchronize();
}

static uint64_t lfsr(uint64_t x)
{
	uint64_t bit = (x ^ (x >> 1)) & 1;

	return (x >> 1) | (bit << 62);
}

#if defined(__GNUC__)
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-function"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_UTIL_H_ */
