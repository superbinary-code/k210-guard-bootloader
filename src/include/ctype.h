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
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */
/*
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#ifndef __INCLUDE_CTYPE_H_
#define __INCLUDE_CTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NOTE! This ctype does not handle EOF like the standard C
 * library is required to.
 */

/* clang-format off */
#define _U		0x01 /* upper */
#define _L		0x02 /* lower */
#define _D		0x04 /* digit */
#define _C		0x08 /* cntrl */
#define _P		0x10 /* punct */
#define _S		0x20 /* white space (space/lf/tab) */
#define _X		0x40 /* hex digit */
#define _SP		0x80 /* hard space (0x20) */
/* clang-format on */

extern const unsigned char _ctype[];

/* clang-format off */
#define __ismask(x)	(_ctype[(int)(unsigned char)(x)])

#define isalnum(c)	((__ismask(c) & (_U | _L | _D)) != 0)
#define isalpha(c)	((__ismask(c) & (_U | _L)) != 0)
#define iscntrl(c)	((__ismask(c) & (_C)) != 0)
#define isdigit(c)	((__ismask(c) & (_D)) != 0)
#define isgraph(c)	((__ismask(c) & (_P | _U | _L | _D)) != 0)
#define islower(c)	((__ismask(c) & (_L)) != 0)
#define isprint(c)	((__ismask(c) & (_P | _U | _L | _D | _SP)) != 0)
#define ispunct(c)	((__ismask(c) & (_P)) != 0)
#define isspace(c)	((__ismask(c) & (_S)) != 0)
#define isupper(c)	((__ismask(c) & (_U)) != 0)
#define isxdigit(c)	((__ismask(c) & (_D | _X)) != 0)

/*
 * Rather than doubling the size of the _ctype lookup table to hold a 'blank'
 * flag, just check for space or tab.
 */
#define isblank(c)	(c == ' ' || c == '\t')

#define isascii(c)	(((unsigned char)(c)) <= 0x7f)
#define toascii(c)	(((unsigned char)(c)) & 0x7f)
/* clang-format on */

static inline unsigned char __tolower(unsigned char c)
{
	if (isupper(c))
		c -= 'A' - 'a';
	return c;
}

static inline unsigned char __toupper(unsigned char c)
{
	if (islower(c))
		c -= 'a' - 'A';
	return c;
}

/* clang-format off */
#define tolower(c)	__tolower(c)
#define toupper(c)	__toupper(c)
/* clang-format on */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_CTYPE_H_ */
