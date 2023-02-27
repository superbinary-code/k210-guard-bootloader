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

#ifndef __INCLUDE_GPIO_COMMON_H_
#define __INCLUDE_GPIO_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _gpio_drive_mode {
	GPIO_DM_INPUT,
	GPIO_DM_INPUT_PULL_DOWN,
	GPIO_DM_INPUT_PULL_UP,
	GPIO_DM_OUTPUT,
} gpio_drive_mode_t;

typedef enum _gpio_pin_edge {
	GPIO_PE_NONE,
	GPIO_PE_FALLING,
	GPIO_PE_RISING,
	GPIO_PE_BOTH
} gpio_pin_edge_t;

typedef enum _gpio_pin_value { GPIO_PV_LOW, GPIO_PV_HIGH } gpio_pin_value_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_GPIO_COMMON_H_ */