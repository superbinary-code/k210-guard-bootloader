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

#ifndef __INCLUDE_SPI_H_
#define __INCLUDE_SPI_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
struct spi_t {
	volatile uint32_t ctrlr0;
	/*!<SPI Control Register 0					(0x00)*/
	volatile uint32_t ctrlr1;
	/*!<SPI Control Register 1					(0x04)*/
	volatile uint32_t ssienr;
	/*!<SPI Enable Register						(0x08)*/
	volatile uint32_t mwcr;
	/*!<SPI Microwire Control Register				(0x0c)*/
	volatile uint32_t ser;
	/*!<SPI Slave Enable Register					(0x10)*/
	volatile uint32_t baudr;
	/*!<SPI Baud Rate Select					(0x14)*/
	volatile uint32_t txftlr;
	/*!<SPI Transmit FIFO Threshold Level				(0x18)*/
	volatile uint32_t rxftlr;
	/*!<SPI Receive FIFO Threshold Level				(0x1c)*/
	volatile uint32_t txflr;
	/*!<SPI Transmit FIFO Level Register				(0x20)*/
	volatile uint32_t rxflr;
	/*!<SPI Receive FIFO Level Register				(0x24)*/
	volatile uint32_t sr;
	/*!<SPI Status Register						(0x28)*/
	volatile uint32_t imr;
	/*!<SPI Interrupt Mask Register					(0x2c)*/
	volatile uint32_t isr;
	/*!<SPI Interrupt Status Register				(0x30)*/
	volatile uint32_t risr;
	/*!<SPI Raw Interrupt Status Register				(0x34)*/
	volatile uint32_t txoicr;
	/*!<SPI Transmit FIFO Overflow Interrupt Clear Register		(0x38)*/
	volatile uint32_t rxoicr;
	/*!<SPI Receive FIFO Overflow Interrupt Clear Register		(0x3c)*/
	volatile uint32_t rxuicr;
	/*!<SPI Receive FIFO Underflow Interrupt Clear Register		(0x40)*/
	volatile uint32_t msticr;
	/*!<SPI Multi-Master Interrupt Clear Register			(0x44)*/
	volatile uint32_t icr;
	/*!<SPI Interrupt Clear Register				(0x48)*/
	volatile uint32_t dmacr;
	/*!<SPI DMA Control Register					(0x4c)*/
	volatile uint32_t dmatdlr;
	/*!<SPI DMA Transmit Data Level					(0x50)*/
	volatile uint32_t dmardlr;
	/*!<SPI DMA Receive Data Level					(0x54)*/
	volatile uint32_t idr;
	/*!<SPI Identification Register					(0x58)*/
	volatile uint32_t ssic_version_id;
	/*!<SPI DWC_ssi component version				(0x5c)*/
	volatile uint32_t dr[36];
	/*!<SPI Data Register 0-36				(0x60 -- 0xec)*/
	volatile uint32_t rx_sample_delay;
	/*!<SPI RX Sample Delay Register				(0xf0)*/
	volatile uint32_t spi_ctrlr0;
	/*!<SPI SPI Control Register					(0xf4)*/
	volatile uint32_t resv;
	/*!<reserved							(0xf8)*/
	volatile uint32_t xip_mode_bits;
	/*!<SPI XIP Mode bits						(0xfc)*/
	volatile uint32_t xip_incr_inst;
	/*!<SPI XIP INCR transfer opcode				(0x100)*/
	volatile uint32_t xip_wrap_inst;
	/*!<SPI XIP WRAP transfer opcode				(0x104)*/
	volatile uint32_t xip_ctrl;
	/*!<SPI XIP Control Register					(0x108)*/
	volatile uint32_t xip_ser;
	/*!<SPI XIP Slave Enable Register				(0x10c)*/
	volatile uint32_t xrxoicr;
	/*!<SPI XIP Receive FIFO Overflow Interrupt Clear Register	(0x110)*/
	volatile uint32_t xip_cnt_time_out;
	/*!<SPI XIP time out register for continuous transfers		(0x114)*/
} __attribute__((packed, aligned(4)));
/* clang-format on */

extern volatile struct spi_t *const spi_devices[4];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_SPI_H_ */
