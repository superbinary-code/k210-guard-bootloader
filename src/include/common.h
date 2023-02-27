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

#ifndef __INCLUDE_COMMON_H_
#define __INCLUDE_COMMON_H_

#include <stdint.h>
//#include "atomic.h"
#include "encoding.h"
#include "entry.h"
#include "gpiohs.h"
#include "sleep.h"
#include "printf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#define SBI_CLEAR_IPI		3
#define SBI_CALL(which, arg0, arg1, arg2)                                      \
	({                                                                     \
		register uintptr_t a0 asm("a0") = (uintptr_t)(arg0);           \
		register uintptr_t a1 asm("a1") = (uintptr_t)(arg1);           \
		register uintptr_t a2 asm("a2") = (uintptr_t)(arg2);           \
		register uintptr_t a7 asm("a7") = (uintptr_t)(which);          \
		asm volatile("ecall"                                           \
			     : "+r"(a0)                                        \
			     : "r"(a1), "r"(a2), "r"(a7)                       \
			     : "memory");                                      \
		a0;                                                            \
	})

#ifndef EOF
#define EOF 			(-1)
#endif

/* Clear unused warnings for actually unused variables */
#define UNUSED(x)		(void)(x)

#ifdef DEBUG
#define debug_parser(...)	printk(__VA_ARGS__)
#else
#define debug_parser(...)
#endif

#define RODATA			__attribute__((section(".rodata.ram")))

#define CORE0_DUMP_ADDRESS	(0x80600000 - (18 * 1024))
#define CORE1_DUMP_ADDRESS	(0x80600000 - (17 * 1024))

// 32bytes SHA256
#define EXIT_REASON_NORMAL	(0)
#define EXIT_REASON_ABNORMAL	(255)

#define EXIT_REASON_OTPBOOT	(1)
#define EXIT_REASON_TURBOBOOT	(2)
#define EXIT_REASON_ISPBOOT	(3)

#define EXIT_REASON_SHA256OTP	(23)
#define EXIT_REASON_SHA256FLASH	(24)
#define EXIT_REASON_OVERSIZE	(233)

#define EXIT_REASON_NOFLASH	(234)
#define EXIT_REASON_OTPBYPASS	(235)

#define EXIT_REASON_EXCEPTION	(666)
/* clang-format on */

extern void __attribute__((noreturn)) safe_main(void);
extern void __attribute__((noreturn)) _boot(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__INCLUDE_COMMON_H_ */
