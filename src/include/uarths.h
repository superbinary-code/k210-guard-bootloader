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
/**
 * @file
 * @brief      Universal Asynchronous Receiver/Transmitter (UART)
 *
 *             The UART peripheral supports the following features:
 *
 *             - 8-N-1 and 8-N-2 formats: 8 data bits, no parity bit, 1 start
 *               bit, 1 or 2 stop bits
 *
 *             - 8-entry transmit and receive FIFO buffers with programmable
 *               watermark interrupts
 *
 *             - 16× Rx oversampling with 2/3 majority voting per bit
 *
 *             The UART peripheral does not support hardware flow control or
 *             other modem control signals, or synchronous serial data
 *             tranfesrs.
 *
 * @note       UART RAM Layout
 *
 * | Address   | Name     |Description                     |
 * |-----------|----------|--------------------------------|
 * | 0x000     | txdata   |Transmit data register          |
 * | 0x004     | rxdata   |Receive data register           |
 * | 0x008     | txctrl   |Transmit control register       |
 * | 0x00C     | rxctrl   |Receive control register        |
 * | 0x010     | ie       |UART interrupt enable           |
 * | 0x014     | ip       |UART Interrupt pending          |
 * | 0x018     | div      |Baud rate divisor               |
 *
 */

#ifndef __INCLUDE_UARTHS_H_
#define __INCLUDE_UARTHS_H_

#include <stdint.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
/* Register address offsets */
#define UARTHS_REG_TXFIFO (0x00)
#define UARTHS_REG_RXFIFO (0x04)
#define UARTHS_REG_TXCTRL (0x08)
#define UARTHS_REG_RXCTRL (0x0c)
#define UARTHS_REG_IE     (0x10)
#define UARTHS_REG_IP     (0x14)
#define UARTHS_REG_DIV    (0x18)

/* TXCTRL register */
#define UARTHS_TXEN       (0x01)
#define UARTHS_TXWM(x)    (((x) & 0xffff) << 16)

/* RXCTRL register */
#define UARTHS_RXEN       (0x01)
#define UARTHS_RXWM(x)    (((x) & 0xffff) << 16)

/* IP register */
#define UARTHS_IP_TXWM    (0x01)
#define UARTHS_IP_RXWM    (0x02)
/* clang-format on */

struct uarths_txdata_t {
	uint32_t data : 8; /*!< Bits [7:0] is data */
	uint32_t zero : 23; /*!< Bits [30:8] is 0 */
	uint32_t full : 1; /*!< Bit 31 is full status */
} __attribute__((packed, aligned(4)));

struct uarths_rxdata_t {
	uint32_t data : 8; /*!< Bits [7:0] is data */
	uint32_t zero : 23; /*!< Bits [30:8] is 0 */
	uint32_t empty : 1; /*!< Bit 31 is empty status */
} __attribute__((packed, aligned(4)));

struct uarths_txctrl_t {
	uint32_t txen : 1;
	/*!< Bit 0 is txen, controls whether the Tx channel is active. */
	uint32_t nstop : 1;
	/*!< Bit 1 is nstop, 0 for one stop bit and 1 for two stop bits */
	uint32_t resv0 : 14;
	/*!< Bits [15:2] is reserved */
	uint32_t txcnt : 3;
	/*!< Bits [18:16] is threshold of interrupt triggers */
	uint32_t resv1 : 13;
	/*!< Bits [31:19] is reserved */
} __attribute__((packed, aligned(4)));

struct uarths_rxctrl_t {
	uint32_t rxen : 1;
	/*!< Bit 0 is txen, controls whether the Tx channel is active. */
	uint32_t resv0 : 15;
	/*!< Bits [15:1] is reserved */
	uint32_t rxcnt : 3;
	/*!< Bits [18:16] is threshold of interrupt triggers */
	uint32_t resv1 : 13;
	/*!< Bits [31:19] is reserved */
} __attribute__((packed, aligned(4)));

struct uarths_ip_t {
	uint32_t txwm : 1;
	/*!< Bit 0 is txwm, raised less than txcnt */
	uint32_t rxwm : 1;
	/*!< Bit 1 is txwm, raised greater than rxcnt */
	uint32_t zero : 30;
	/*!< Bits [31:2] is 0 */
} __attribute__((packed, aligned(4)));

struct uarths_ie_t {
	uint32_t txwm : 1;
	/*!< Bit 0 is txwm, raised less than txcnt */
	uint32_t rxwm : 1;
	/*!< Bit 1 is txwm, raised greater than rxcnt */
	uint32_t zero : 30;
	/*!< Bits [31:2] is 0 */
} __attribute__((packed, aligned(4)));

struct uarths_div_t {
	uint32_t div : 16;
	/*!< Bits [31:2] is baud rate divisor register */
	uint32_t zero : 16;
	/*!< Bits [31:16] is 0 */
} __attribute__((packed, aligned(4)));

struct uarths_t {
	struct uarths_txdata_t txdata; /*!< Address offset 0x00 */
	struct uarths_rxdata_t rxdata; /*!< Address offset 0x04 */
	struct uarths_txctrl_t txctrl; /*!< Address offset 0x08 */
	struct uarths_rxctrl_t rxctrl; /*!< Address offset 0x0c */
	struct uarths_ie_t ie; /*!< Address offset 0x10 */
	struct uarths_ip_t ip; /*!< Address offset 0x14 */
	struct uarths_div_t div; /*!< Address offset 0x18 */
} __attribute__((packed, aligned(4)));

extern volatile struct uarths_t *const uarths;

/**
 * @brief      Initialization Core UART
 *
 * @return     result
 *     - 0     Success
 *     - Other Fail
 */
int uart_init(void);

/**
 * @brief      Put a char to UART
 *
 * @param[in]  c     The char to put
 *
 * @note       If c is '\n', a '\r' will be appended automatically
 *
 * @return     result
 *     - 0     Success
 *     - Other Fail
 */
int uart_putchar(char c);

/**
 * @brief      Send a string to UART
 *
 * @param[in]  s     The string to send
 *
 * @note       The string must ending with '\0'
 *
 * @return     result
 *     - 0     Success
 *     - Other Fail
 */
int uart_puts(const char *s);

/**
 * @brief      Get a byte from UART
 *
 * @return     byte as int type from UART, failed if return < 0
 */
int uart_getc(void);

/**
 * @brief      check if a key is available from UART
 *
 * @return     1 for available, 0 for none, failed if return < 0
 */
int uart_tstc(void);

/**
 * @brief      check if CTRL+C is pressed from UART
 *
 * @return     1 for yes, 0 for none, failed if return < 0
 */
int uart_ctrlc(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_UARTHS_H_ */
