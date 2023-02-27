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

#include "common.h"
#include "platform.h"
#include "spi.h"

volatile struct spi_t *const spi_devices[4] RODATA = {
	(volatile struct spi_t *)SPI0_BASE_ADDR,
	(volatile struct spi_t *)SPI1_BASE_ADDR,
	(volatile struct spi_t *)SPI_SLAVE_BASE_ADDR,
	(volatile struct spi_t *)SPI3_BASE_ADDR
};
