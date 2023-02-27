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
 * @brief      flash driver
 */
#ifndef __INCLUDE_FLASH_H_
#define __INCLUDE_FLASH_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      flash operating status enumerate
 */
enum flash_status_t {
	FLASH_OK = 0,
	FLASH_BUSY,
};

/**
 * @brief      flash read operating enumerate
 */
enum flash_read_t {
	FLASH_STANDARD = 0,
	FLASH_STANDARD_FAST,
	FLASH_DUAL,
	FLASH_DUAL_SINGLE,
	FLASH_QUAD,
	FLASH_QUAD_SINGLE,
};

enum flash_status_t flash_init(uint8_t index);
enum flash_status_t flash_is_busy(void);
enum flash_status_t flash_chip_erase(void);
enum flash_status_t flash_enable_quad_mode(void);
enum flash_status_t flash_disable_quad_mode(void);
enum flash_status_t flash_sector_erase(uint32_t addr);
enum flash_status_t flash_32k_block_erase(uint32_t addr);
enum flash_status_t flash_64k_block_erase(uint32_t addr);
enum flash_status_t flash_read_status_reg1(uint8_t *reg_data);
enum flash_status_t flash_read_status_reg2(uint8_t *reg_data);
enum flash_status_t flash_write_status_reg(uint8_t reg1_data,
					   uint8_t reg2_data);
enum flash_status_t flash_read_id(uint8_t *manuf_id, uint8_t *device_id);
enum flash_status_t flash_write_data(uint32_t addr, uint8_t *data_buf,
				     uint32_t length);
enum flash_status_t flash_read_data(uint32_t addr, uint8_t *data_buf,
				    uint32_t length, enum flash_read_t mode);
enum flash_status_t flash_disable_protect(void);
int do_flash_erase(uint32_t offset, uint32_t length);
int do_flash_write(uint32_t offset, uint32_t length, uint8_t *ramptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_FLASH_H_ */
