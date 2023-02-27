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

#include <stdint.h>
#include <stdio.h>
#include "common.h"
#include "encoding.h"
#include "sysctl.h"
#include "uarths.h"

#include "printf.h"

/* FIXME: ideally this address should come from config string. */
volatile struct uarths_t *const uarths RODATA =
	(volatile struct uarths_t *)UARTHS_BASE_ADDR;

static inline int uart_putc(char c)
{
	while (uarths->txdata.full)
		continue;
	uarths->txdata.data = c;
	return 0;
}

int uart_getc(void)
{
	/* while not empty */
	struct uarths_rxdata_t recv;

	while (1) {
		recv = uarths->rxdata;
		if (!recv.empty)
			break;
	}

	return recv.data;
}

int uart_putchar(char c)
{
	return uart_putc(c);
}

int uart_puts(const char *s)
{
	while (*s)
		if (uart_putc(*s++) != 0)
			return -1;
	return 0;
}

int uart_init(void)
{
	uint16_t div = sysctl_clock_get_freq(SYSCTL_CLOCK_CPU) / 115200 - 1;

	/* Set UART registers */
	uarths->div.div = div;
	uarths->txctrl.txen = 1;
	uarths->rxctrl.rxen = 1;
	uarths->txctrl.txcnt = 0;
	uarths->rxctrl.rxcnt = 0;
	uarths->ip.txwm = 1;
	uarths->ip.rxwm = 1;
	uarths->ie.txwm = 0;
	uarths->ie.rxwm = 1;

	return 0;
}

int uart_tstc(void)
{
	return uarths->ip.rxwm;
}

int uart_ctrlc(void)
{
	if (uart_tstc()) {
		switch (uart_getc()) {
		case 0x03: /* ^C - Control C */
			return 1;
		default:
			break;
		}
	}

	return 0;
}
